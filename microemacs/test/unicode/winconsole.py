#!/usr/bin/env python3
#
# winconsole.py - Windows console tests: runs a console build (mec) in its own console, reads the
# cells back with ReadConsoleOutputW and types with WriteConsoleInputW. Checks the text, wide chars
# in the console's leading/trailing cell form, a char above U+FFFF as a wide U+FFFD, typed
# non-ASCII and surrogate pair keys, Return, the saved bytes and the cursor on a wide char; and
# non-ASCII file names (UTF-8 process code page) from the command line, find-file, write-file and
# completion of a directory name.
# Windows equivalent of the display.py and input.py basics.
#
# Usage: python winconsole.py <mec> [macros-dir]

import ctypes
import os
import shutil
import subprocess
import sys
import tempfile
import time
from ctypes import wintypes

TDIR = os.path.dirname(os.path.abspath(__file__))


class COORD(ctypes.Structure):
    _fields_ = [("X", ctypes.c_short), ("Y", ctypes.c_short)]


class SMALL_RECT(ctypes.Structure):
    _fields_ = [("Left", ctypes.c_short), ("Top", ctypes.c_short), ("Right", ctypes.c_short), ("Bottom", ctypes.c_short)]


class CHAR_INFO(ctypes.Structure):
    _fields_ = [("Char", ctypes.c_wchar), ("Attributes", wintypes.WORD)]


class CSBI(ctypes.Structure):
    _fields_ = [("dwSize", COORD), ("dwCursorPosition", COORD), ("wAttributes", wintypes.WORD),
                ("srWindow", SMALL_RECT), ("dwMaximumWindowSize", COORD)]


class KEY_EVENT_RECORD(ctypes.Structure):
    _fields_ = [("bKeyDown", wintypes.BOOL), ("wRepeatCount", wintypes.WORD), ("wVirtualKeyCode", wintypes.WORD),
                ("wVirtualScanCode", wintypes.WORD), ("UnicodeChar", ctypes.c_wchar), ("dwControlKeyState", wintypes.DWORD)]


class EVENT_UNION(ctypes.Union):
    _fields_ = [("KeyEvent", KEY_EVENT_RECORD), ("pad", ctypes.c_byte * 16)]


class INPUT_RECORD(ctypes.Structure):
    _fields_ = [("EventType", wintypes.WORD), ("Event", EVENT_UNION)]


k32 = ctypes.WinDLL("kernel32", use_last_error=True)
u32 = ctypes.WinDLL("user32", use_last_error=True)
CTRL = 0x0008
LEADING, TRAILING = 0x100, 0x200

passed = failed = 0


def check(name, cond, detail=''):
    global passed, failed
    if cond:
        passed += 1
    else:
        failed += 1
        print('FAIL %s %s' % (name, detail))


class Console:
    def __init__(self, pid):
        k32.FreeConsole()
        for _ in range(20):
            if k32.AttachConsole(pid):
                break
            time.sleep(0.25)
        self.hout = k32.CreateFileW("CONOUT$", 0xC0000000, 3, None, 3, 0, None)
        self.hin = k32.CreateFileW("CONIN$", 0xC0000000, 3, None, 3, 0, None)

    def cells(self):
        info = CSBI()
        k32.GetConsoleScreenBufferInfo(self.hout, ctypes.byref(info))
        ww = info.srWindow.Right - info.srWindow.Left + 1
        hh = info.srWindow.Bottom - info.srWindow.Top + 1
        buf = (CHAR_INFO * (ww * hh))()
        rect = SMALL_RECT(info.srWindow.Left, info.srWindow.Top, info.srWindow.Right, info.srWindow.Bottom)
        k32.ReadConsoleOutputW(self.hout, buf, COORD(ww, hh), COORD(0, 0), ctypes.byref(rect))
        return [[(buf[r * ww + c].Char, buf[r * ww + c].Attributes) for c in range(ww)] for r in range(hh)]

    def key(self, ch, vk=0, scan=0, state=0):
        recs = (INPUT_RECORD * 2)()
        for ii, down in enumerate((1, 0)):
            recs[ii].EventType = 1
            ke = recs[ii].Event.KeyEvent
            ke.bKeyDown, ke.wRepeatCount, ke.wVirtualKeyCode, ke.wVirtualScanCode = down, 1, vk, scan
            ke.UnicodeChar, ke.dwControlKeyState = ch, state
        nn = wintypes.DWORD()
        k32.WriteConsoleInputW(self.hin, recs, 2, ctypes.byref(nn))
        time.sleep(0.05)

    def ctrl(self, letter):
        # the char of C-<letter> with its virtual key and scan code
        self.key(chr(ord(letter) & 0x1f), vk=ord(letter), scan=u32.MapVirtualKeyW(ord(letter), 0), state=CTRL)

    def type(self, text):
        for ch in text:
            if ord(ch) > 0xffff:
                cc = ord(ch) - 0x10000
                self.key(chr(0xd800 + (cc >> 10)))
                self.key(chr(0xdc00 + (cc & 0x3ff)))
            else:
                self.key(ch)


def text(row):
    # the trailing cell of a wide char repeats the char, the last column is the scroll bar
    return ''.join(ch for ch, at in row[:-1] if not (at & TRAILING)).rstrip()


def main():
    if len(sys.argv) < 2:
        sys.exit('Usage: winconsole.py <mec> [macros-dir]')
    mec = os.path.abspath(sys.argv[1])
    macros = os.path.abspath(sys.argv[2] if len(sys.argv) > 2 else os.path.join(TDIR, '..', '..', 'macros'))
    # attaching to mec's console detaches this process from its own, so the tests run in a child
    # whose output is a pipe
    rr = subprocess.run([sys.executable, os.path.abspath(__file__), '--child', mec, macros],
                        stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    print(rr.stdout, end='')
    sys.exit(rr.returncode)


def child(mec, macros):
    home = tempfile.mkdtemp()
    tfile = os.path.join(home, 'uni.txt')
    lines = ['Hello w\u00f6rld caf\u00e9', '\u0391\u03b2\u03b3 \u0434\u0430', '\u4e2d\u6587\u5b57 x', 'a\U0001F600b', 'end']
    with open(tfile, 'w', encoding='utf-8', newline='\n') as ff:
        ff.write('\n'.join(lines) + '\n')
    env = dict(os.environ)
    env.update(HOME=home, MEUSERPATH=home, MEPATH=macros, MENAME='test')
    proc = subprocess.Popen([mec, tfile], env=env, creationflags=subprocess.CREATE_NEW_CONSOLE)
    try:
        time.sleep(1.0)
        con = Console(proc.pid)
        # row 0 is the menu line
        for _ in range(20):
            scr = con.cells()
            if text(scr[1]).startswith('Hello'):
                break
            time.sleep(0.25)
        for ii, ll in enumerate(lines):
            exp = ll.replace('\U0001F600', '\ufffd')
            check('row %d text' % ii, text(scr[ii + 1]) == exp, ascii(text(scr[ii + 1])))
        row = scr[3]
        check('wide leading/trailing', row[0][0] == '\u4e2d' and (row[0][1] & LEADING) and row[1][0] == '\u4e2d' and
              (row[1][1] & TRAILING) and row[2][0] == '\u6587' and (row[2][1] & LEADING), ascii(row[:7]))
        row = scr[4]
        check('above U+FFFF is a wide U+FFFD', row[1][0] == '\ufffd' and (row[1][1] & LEADING) and
              (row[2][1] & TRAILING) and row[3][0] == 'b', ascii(row[:5]))

        # typed non-ASCII and wide chars, Return splits the line, a surrogate pair is one char
        con.type('\u00e9\u00df\u4e2d')
        con.key('\r', vk=0x0d, scan=0x1c)
        con.type('\U0001F600z')
        time.sleep(0.5)
        scr = con.cells()
        check('typed', text(scr[1]) == '\u00e9\u00df\u4e2d', ascii(text(scr[1])))
        check('return', text(scr[2]) == '\ufffdzHello w\u00f6rld caf\u00e9', ascii(text(scr[2])))

        con.ctrl('X')
        con.ctrl('S')
        time.sleep(1.0)
        data = open(tfile, 'rb').read()
        exp = ('\u00e9\u00df\u4e2d\n\U0001F600z' + '\n'.join(lines) + '\n').encode('utf-8')
        check('saved bytes', data == exp, ascii(data[:40]))

        # the cursor on a wide char colours both its cells
        con.ctrl('N')
        con.ctrl('N')
        con.ctrl('A')
        time.sleep(0.5)
        row = con.cells()[4]
        check('cursor on wide char', text(row) == '\u4e2d\u6587\u5b57 x' and (row[0][1] & 0xff) == (row[1][1] & 0xff) and
              (row[0][1] & 0xff) != (row[2][1] & 0xff), ascii(row[:4]))
        con.ctrl('F')
        time.sleep(0.5)
        row = con.cells()[4]
        check('cursor moved to next wide char', (row[0][1] & 0xff) == (row[1][1] & 0xff) and
              (row[2][1] & 0xff) == (row[3][1] & 0xff) and (row[0][1] & 0xff) != (row[2][1] & 0xff), ascii(row[:4]))
    finally:
        proc.kill()
        proc.wait()
        shutil.rmtree(home, ignore_errors=True)

    childFiles(mec, macros)
    print('%d passed, %d failed' % (passed, failed))
    sys.exit(1 if failed else 0)


def childFiles(mec, macros):
    # non-ASCII file names: opened from the command line, find-file, write-file and completion
    home = tempfile.mkdtemp()
    fdir = tempfile.mkdtemp(prefix='me\u00e9')
    names = {'caf\u00e9.txt': 'cafe content\n', '\u4e2d\u6587.txt': 'cjk content\n'}
    for nn, cc in names.items():
        with open(os.path.join(fdir, nn), 'w', encoding='utf-8', newline='\n') as ff:
            ff.write(cc)
    os.mkdir(os.path.join(fdir, 'dir \u00e9'))
    with open(os.path.join(fdir, 'dir \u00e9', 'x.txt'), 'w', newline='\n') as ff:
        ff.write('x content\n')
    env = dict(os.environ)
    env.update(HOME=home, MEUSERPATH=home, MEPATH=macros, MENAME='test')
    proc = subprocess.Popen([mec, os.path.join(fdir, 'caf\u00e9.txt')], env=env, creationflags=subprocess.CREATE_NEW_CONSOLE)
    try:
        time.sleep(1.0)
        con = Console(proc.pid)
        for _ in range(20):
            scr = con.cells()
            if text(scr[1]):
                break
            time.sleep(0.25)
        # the mode line is the row before the message line
        check('file arg loaded', text(scr[1]) == 'cafe content', ascii(text(scr[1])))
        check('file arg mode line', 'caf\u00e9.txt' in text(scr[-2]), ascii(text(scr[-2])))

        con.type('X')
        con.ctrl('X')
        con.ctrl('S')
        time.sleep(1.0)
        data = open(os.path.join(fdir, 'caf\u00e9.txt'), 'rb').read()
        check('file arg saved', data == b'Xcafe content\n', ascii(data))

        con.ctrl('X')
        con.ctrl('F')
        time.sleep(0.5)
        con.type(os.path.join(fdir, '\u4e2d\u6587.txt'))
        con.key('\r', vk=0x0d, scan=0x1c)
        time.sleep(1.0)
        scr = con.cells()
        check('find-file', text(scr[1]) == 'cjk content', ascii(text(scr[1])))
        check('find-file mode line', '\u4e2d\u6587.txt' in text(scr[-2]), ascii(text(scr[-2])))

        con.ctrl('X')
        con.ctrl('W')
        time.sleep(0.5)
        con.type(os.path.join(fdir, '\u043d\u043e\u0432\u044b\u0439.txt'))
        con.key('\r', vk=0x0d, scan=0x1c)
        time.sleep(1.0)
        nfile = os.path.join(fdir, '\u043d\u043e\u0432\u044b\u0439.txt')
        data = open(nfile, 'rb').read() if os.path.exists(nfile) else None
        check('write-file', data == b'cjk content\n', ascii(data))
        check('no mangled names', all(any(ord(ch) > 0x7f for ch in nn) or nn.endswith('~') or nn == 'x.txt' for nn in os.listdir(fdir)),
              ascii(os.listdir(fdir)))

        # completion lists the directory: "dir" Tab completes to the non-ASCII directory name
        con.ctrl('X')
        con.ctrl('F')
        time.sleep(0.5)
        con.type(os.path.join(fdir, 'dir'))
        con.key('\t', vk=0x09, scan=0x0f)
        time.sleep(0.5)
        con.type('x.txt')
        con.key('\r', vk=0x0d, scan=0x1c)
        time.sleep(1.0)
        scr = con.cells()
        check('completion', text(scr[1]) == 'x content', ascii(text(scr[1])) + ' ' + ascii(text(scr[-1])))
    finally:
        proc.kill()
        proc.wait()
        shutil.rmtree(home, ignore_errors=True)
        shutil.rmtree(fdir, ignore_errors=True)


if __name__ == '__main__':
    if (len(sys.argv) == 4) and (sys.argv[1] == '--child'):
        child(sys.argv[2], sys.argv[3])
    else:
        main()
