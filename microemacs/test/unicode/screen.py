#!/usr/bin/env python3
#
# screen.py - Runs mec in a pseudo terminal and emulates the screen it draws, so display tests
# can check what is in each cell. Only the sequences mec uses are emulated.

import os
import pty
import re
import select
import signal
import struct
import termios
import fcntl
import time
import unicodedata

CSI_RE = re.compile(rb'\x1b\[([0-9;?>]*)([@-~])')


def charWidth(ch):
    if unicodedata.combining(ch) or unicodedata.category(ch) in ('Mn', 'Me', 'Cf'):
        return 0
    return 2 if unicodedata.east_asian_width(ch) in ('W', 'F') else 1


class Screen:
    def __init__(self, rows, cols):
        self.rows = rows
        self.cols = cols
        self.cells = [[' '] * cols for _ in range(rows)]
        self.row = self.col = 0
        self.top, self.bot = 0, rows - 1
        self.pend = b''
        self.encoding = 'utf-8'

    def put(self, ch):
        ww = charWidth(ch)
        if ww == 0:
            # combining - attach to the previous cell
            cc = max(self.col - 1, 0)
            self.cells[self.row][cc] += ch
            return
        if self.col + ww > self.cols:
            self.col = self.cols - ww
        line = self.cells[self.row]
        # overwriting half of a wide char clears the other half
        if line[self.col] == '' and self.col > 0:
            line[self.col - 1] = ' '
        if self.col + ww < self.cols and line[self.col + ww] == '':
            line[self.col + ww] = ' '
        line[self.col] = ch
        if ww == 2:
            line[self.col + 1] = ''
        self.col = min(self.col + ww, self.cols - 1) if self.col + ww >= self.cols else self.col + ww

    def scroll(self, up):
        if up:
            del self.cells[self.top]
            self.cells.insert(self.bot, [' '] * self.cols)
        else:
            del self.cells[self.bot]
            self.cells.insert(self.top, [' '] * self.cols)

    def csi(self, params, final):
        ps = [int(pp) if pp.isdigit() else 0 for pp in params.lstrip('?>').split(';')] if params else []
        p1 = ps[0] if ps else 0
        if params.startswith(('?', '>')):
            return
        if final in 'Hf':
            self.row = min(max((ps[0] if ps and ps[0] else 1) - 1, 0), self.rows - 1)
            self.col = min(max((ps[1] if len(ps) > 1 and ps[1] else 1) - 1, 0), self.cols - 1)
        elif final == 'K':
            line = self.cells[self.row]
            rng = range(self.col, self.cols) if p1 == 0 else range(0, self.col + 1) if p1 == 1 else range(self.cols)
            for cc in rng:
                line[cc] = ' '
        elif final == 'J':
            if p1 == 2:
                self.cells = [[' '] * self.cols for _ in range(self.rows)]
            else:
                for cc in range(self.col, self.cols):
                    self.cells[self.row][cc] = ' '
                for rr in range(self.row + 1, self.rows):
                    self.cells[rr] = [' '] * self.cols
        elif final == 'A':
            self.row = max(self.row - max(p1, 1), 0)
        elif final == 'B':
            self.row = min(self.row + max(p1, 1), self.rows - 1)
        elif final == 'C':
            self.col = min(self.col + max(p1, 1), self.cols - 1)
        elif final == 'D':
            self.col = max(self.col - max(p1, 1), 0)
        elif final == 'G':
            self.col = min(max(p1 - 1, 0), self.cols - 1)
        elif final == 'd':
            self.row = min(max(p1 - 1, 0), self.rows - 1)
        elif final == 'r':
            self.top = (ps[0] - 1) if ps and ps[0] else 0
            self.bot = (ps[1] - 1) if len(ps) > 1 and ps[1] else self.rows - 1
        elif final == 'X':
            for cc in range(self.col, min(self.col + max(p1, 1), self.cols)):
                self.cells[self.row][cc] = ' '

    def feed(self, data):
        data = self.pend + data
        self.pend = b''
        ii = 0
        nn = len(data)
        while ii < nn:
            bb = data[ii]
            if bb == 0x1b:
                if ii + 1 >= nn:
                    self.pend = data[ii:]
                    return
                nx = data[ii + 1]
                if nx == 0x5b:
                    mm = CSI_RE.match(data, ii)
                    if not mm:
                        self.pend = data[ii:]
                        return
                    self.csi(mm.group(1).decode('ascii'), chr(mm.group(2)[0]))
                    ii = mm.end()
                elif nx == 0x5d:
                    end = data.find(b'\x07', ii)
                    end2 = data.find(b'\x1b\\', ii)
                    ends = [ee for ee in (end, end2) if ee >= 0]
                    if not ends:
                        self.pend = data[ii:]
                        return
                    ii = min(ends) + (1 if min(ends) == end else 2)
                elif nx in (0x28, 0x29):
                    ii += 3
                elif nx == 0x4d:
                    if self.row == self.top:
                        self.scroll(False)
                    else:
                        self.row = max(self.row - 1, 0)
                    ii += 2
                elif nx == 0x44:
                    if self.row == self.bot:
                        self.scroll(True)
                    else:
                        self.row += 1
                    ii += 2
                else:
                    ii += 2
            elif bb == 0x0d:
                self.col = 0
                ii += 1
            elif bb == 0x0a:
                if self.row == self.bot:
                    self.scroll(True)
                else:
                    self.row = min(self.row + 1, self.rows - 1)
                ii += 1
            elif bb == 0x08:
                self.col = max(self.col - 1, 0)
                ii += 1
            elif bb < 0x20:
                ii += 1
            elif self.encoding != 'utf-8':
                self.put(bytes([bb]).decode(self.encoding))
                ii += 1
            else:
                ll = 1 if bb < 0x80 else 2 if bb < 0xe0 else 3 if bb < 0xf0 else 4
                if ii + ll > nn:
                    self.pend = data[ii:]
                    return
                try:
                    ch = data[ii:ii + ll].decode('utf-8')
                except UnicodeDecodeError:
                    ch, ll = '�', 1
                self.put(ch)
                ii += ll

    def text(self, row):
        return ''.join(self.cells[row])

    def cellsAt(self, row):
        return self.cells[row]


class Mec:
    """mec running in a pty, keys() sends input and waits for the screen to settle"""

    def __init__(self, mec, args, rows=24, cols=80, env=None, home=None):
        self.screen = Screen(rows, cols)
        self.raw = b''
        pid, fd = pty.fork()
        if pid == 0:
            os.execve(mec, [mec] + list(args), env)
        self.pid, self.fd = pid, fd
        fcntl.ioctl(fd, termios.TIOCSWINSZ, struct.pack('HHHH', rows, cols, 0, 0))
        os.kill(pid, signal.SIGWINCH)
        self.settle(5.0)

    def settle(self, maxWait=3.0, quiet=0.4):
        end = time.time() + maxWait
        got = False
        while time.time() < end:
            rr, _, _ = select.select([self.fd], [], [], quiet)
            if rr:
                try:
                    data = os.read(self.fd, 65536)
                except OSError:
                    return
                if not data:
                    return
                self.raw += data
                self.screen.feed(data)
                got = True
            elif got:
                return

    def keys(self, data, wait=2.0):
        os.write(self.fd, data)
        self.settle(wait)

    def close(self):
        try:
            os.write(self.fd, b'\x07\x07\x18\x03')
            time.sleep(0.2)
        except OSError:
            pass
        # macOS does not finish a process's exit until its tty output is drained, so keep reading
        # while waiting or both sides wait for ever
        try:
            os.kill(self.pid, signal.SIGKILL)
            end = time.time() + 10
            while (os.waitpid(self.pid, os.WNOHANG)[0] == 0) and (time.time() < end):
                if select.select([self.fd], [], [], 0.1)[0]:
                    os.read(self.fd, 65536)
        except OSError:
            pass
        os.close(self.fd)

    @property
    def cursor(self):
        return (self.screen.row, self.screen.col)


def mecEnv(home, macros, lang='C.UTF-8'):
    # home is first on the path so it is the user path (session files are kept out of macros) and
    # its newuser.erf is used, without the scroll bars some platforms default to (e.g. macOS
    # console) so every platform draws the same columns
    erf = os.path.join(home, 'newuser.erf')
    src = [os.path.join(dd, 'newuser.erf') for dd in macros.split(':')]
    src = [ff for ff in src if os.path.isfile(ff)]
    if src and not os.path.exists(erf):
        text = open(src[0], 'rb').read()
        open(erf, 'wb').write(re.sub(rb'(?m)^[ \t]*"scroll-bar" = .*\n', b'', text))
    env = dict(os.environ)
    env.update(TERM='xterm', HOME=home, MEPATH=home + ':' + macros, LANG=lang, LC_ALL=lang)
    return env
