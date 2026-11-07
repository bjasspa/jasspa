#!/usr/bin/env python3
#
# input.py - Input tests (Phase 4): runs mec in a pseudo terminal, sends key bytes as a UTF-8 (or
# code page) terminal would and checks the saved file, the screen and the cursor
# (acceptance tests I1, I3, I5, I6, I8, I9 and E4), plus key names and key strings checked headlessly.
#
# Usage: python3 -I input.py <mec> [macros-dir]

import os
import shutil
import sys
import tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from screen import Mec, mecEnv, charWidth  # noqa: E402

TDIR = os.path.dirname(os.path.abspath(__file__))
ESC = b'\x1b'
CX = b'\x18'

passed = failed = 0


def check(name, cond, detail=''):
    global passed, failed
    if cond:
        passed += 1
    else:
        failed += 1
        print('FAIL %s %s' % (name, detail))


def width(text):
    return sum(charWidth(ch) for ch in text)


def u8(text):
    return text.encode('utf-8')


class Run:
    """mec editing a scratch file with the given content"""

    def __init__(self, mec, macros, content=b'x\n', lang='C.UTF-8'):
        self.home = tempfile.mkdtemp()
        self.fname = os.path.join(self.home, 'in.txt')
        open(self.fname, 'wb').write(content)
        self.mm = Mec(mec, [self.fname], env=mecEnv(self.home, macros, lang))

    def keys(self, *kk):
        for key in kk:
            self.mm.keys(key)

    def command(self, name):
        self.keys(ESC + b'x' + name.encode() + b'\r')

    def saved(self):
        self.keys(CX + b'\x13')
        return open(self.fname, 'rb').read()

    def finish(self):
        self.mm.close()
        shutil.rmtree(self.home, ignore_errors=True)


def headless(mec):
    """key names, bindings and key strings checked by a headless macro"""
    import subprocess
    wdir = tempfile.mkdtemp()
    mac = ['define-macro start-up',
           '  find-buffer "*t*"',
           '  insert-string "abcd"',
           '  beginning-of-line',
           # I8 - a non-ASCII key bound and described
           '  global-bind-key "forward-char" "é"',
           '  0 describe-key "é"',
           '  -2 ml-write &cat "R:desc=" $result',
           '  execute-string "é"',
           '  -2 ml-write &cat "R:boundcol=" $window-col',
           # an upper case non-ASCII key is looked up as lower case, as letters are
           '  execute-string "É"',
           '  -2 ml-write &cat "R:uppercol=" $window-col',
           # a modifier on a non-ASCII char in a key string: leader, special, alt byte, char
           '  global-bind-key "backward-char" "A-ж"',
           '  0 describe-key "A-ж"',
           '  -2 ml-write &cat "R:altdesc=" $result',
           '  execute-string "\\s\\x04ж"',
           '  -2 ml-write &cat "R:altcol=" $window-col',
           '  4 exit-emacs 0', '!emacro']
    open(os.path.join(wdir, 'k.emf'), 'w', encoding='utf-8').write('\n'.join(mac) + '\n')
    res = subprocess.run([mec, '-p', '@' + os.path.join(wdir, 'k')], stdin=subprocess.DEVNULL,
                         capture_output=True, env=dict(os.environ, MEPATH=wdir), timeout=60)
    log = (res.stdout + res.stderr).decode('utf-8', 'backslashreplace').replace('\r', '\n')
    got = dict(ll[2:].split('=', 1) for ll in log.split('\n') if ll.startswith('R:') and '=' in ll)
    shutil.rmtree(wdir, ignore_errors=True)
    for key, want in (('desc', 'forward-char'), ('boundcol', '1'), ('uppercol', '2'),
                      ('altdesc', 'backward-char'), ('altcol', '1')):
        check('headless %s' % key, got.get(key) == want, 'got %r' % got.get(key))


def main():
    if len(sys.argv) < 2:
        sys.exit('Usage: input.py <mec> [macros-dir]')
    mec = os.path.abspath(sys.argv[1])
    macros = os.path.abspath(sys.argv[2] if len(sys.argv) > 2 else os.path.join(TDIR, '..', '..', 'macros'))
    headless(mec)

    # I1 - typed UTF-8 chars of every length are inserted, a key a char
    rr = Run(mec, macros)
    rr.keys(u8('aé中😀'), b'\r')
    check('I1 typed text', rr.saved() == u8('aé中😀\nx\n'), repr(open(rr.fname, 'rb').read()))
    rr.finish()

    # typed chars are undone a char at a time
    rr = Run(mec, macros)
    rr.keys(u8('é'), u8('中'), b'\x1f')
    check('I1 undo typed char', rr.saved() == u8('éx\n'), repr(open(rr.fname, 'rb').read()))
    rr.finish()

    # E4 - overwrite with typed multi-byte chars
    rr = Run(mec, macros, b'abcd\n')
    rr.command('buffer-mode')
    rr.keys(b'over\r', u8('中😀'))
    check('E4 overwrite typed', rr.saved() == u8('中😀cd\n'), repr(open(rr.fname, 'rb').read()))
    rr.finish()

    # I3 - prompt editing a char at a time, the cursor column in cells
    rr = Run(mec, macros)
    rr.command('insert-string')
    rr.keys(u8('aé中文'))
    rows = rr.mm.screen.rows
    check('I3 prompt cursor', rr.mm.cursor == (rows - 1, len('String: ') + width('aé中文')),
          'cursor %r' % (rr.mm.cursor,))
    rr.keys(b'\x7f', u8('😀b'), b'\x02', b'\x02', b'\x04', b'\x01', b'\x06', b'\x14', b'\r')
    check('I3 prompt edit', rr.saved() == u8('éa中bx\n'), repr(open(rr.fname, 'rb').read()))
    rr.finish()

    # I3 - prompt word case change with a char which grows
    rr = Run(mec, macros)
    rr.command('insert-string')
    rr.keys(u8('straße ǆx Ⱥ'), b'\x01', ESC + b'u', ESC + b'u', ESC + b'l', b'\r')
    check('I3 prompt case', rr.saved() == u8('STRAßE ǄX ⱥx\n'), repr(open(rr.fname, 'rb').read()))
    rr.finish()

    # keyboard macros record and replay whole chars
    rr = Run(mec, macros)
    rr.keys(CX + b'(', u8('é中😀'), CX + b')', CX + b'e')
    check('keyboard macro', rr.saved() == u8('é中😀é中😀x\n'), repr(open(rr.fname, 'rb').read()))
    rr.finish()

    # I5 - a -ve quote-char inserts by code point, a +ve n still quotes the key (3 u's)
    rr = Run(mec, macros)
    rr.keys(ESC + b'-1\x11', b'e9\r', ESC + b'-1\x11', b'U+1F600\r', b'\x11', u8('中'), ESC + b'3\x11', b'u')
    check('I5 quote code point', rr.saved() == u8('é😀中uuux\n'), repr(open(rr.fname, 'rb').read()))
    rr.finish()

    # I5 - esc C-q (-1 quote-char) gives a rejected key's reason, finds a char by its code in isearch
    # (a NUL and a raw byte too, C-g aborting the search) and inserts it in a prompt
    rr = Run(mec, macros)
    rr.keys(ESC + b'\x11', b'2g')
    mline = rr.mm.screen.text(rr.mm.screen.rows - 1).rstrip()
    check('I5 code not hex', mline.endswith(': 2 [not hex]'), repr(mline))
    rr.keys(ESC, ESC + b'\x11', b'x123')
    mline = rr.mm.screen.text(rr.mm.screen.rows - 1).rstrip()
    check('I5 code too large', mline.endswith(': x12 [too large, max FF]'), repr(mline))
    rr.keys(ESC)
    rr.finish()
    text = b'cafe\xc3\xa9 nul:a\x00b raw:\xff end\n'
    for tag, code, want in (('char', b'e9', text.replace(b'\xa9 ', b'\xa9X ')), ('NUL', b'0', text.replace(b'\x00', b'\x00X')),
                            ('raw byte', b'rff', text.replace(b'\xff', b'\xffX'))):
        rr = Run(mec, macros, text)
        rr.keys(ESC + b'xexecute-line\rset-variable $file-checks 0x55\r')
        rr.keys(b'\x13', ESC + b'\x11', code + b'\r', b'\r', b'X')
        check('I5 isearch by code %s' % tag, rr.saved() == want, repr(open(rr.fname, 'rb').read()))
        rr.finish()
    rr = Run(mec, macros, text)
    rr.keys(ESC + b'xexecute-line\rset-variable $file-checks 0x55\r')
    rr.keys(b'\x05', b'\x13e', ESC + b'\x11', b'\x07', b'Y')
    check('I5 isearch code abort', rr.saved() == text.replace(b' end', b' endY'), repr(open(rr.fname, 'rb').read()))
    rr.finish()
    rr = Run(mec, macros)
    rr.keys(ESC + b'xexecute-line\rset-variable $file-checks 0x55\r')
    rr.command('execute-line')
    rr.keys(b'insert-string "', ESC + b'\x11', b'263a\r', ESC + b'\x11', b'0\r', b'"\r')
    check('I5 prompt by code', rr.saved() == u8('\u263a') + b'\x00x\n', repr(open(rr.fname, 'rb').read()))
    rr.finish()
    # C-q C-@ (a terminal sends C-space) quotes a NUL, in the buffer and in isearch
    rr = Run(mec, macros, b'ab\x00cd\n')
    rr.keys(ESC + b'xexecute-line\rset-variable $file-checks 0x55\r')
    rr.keys(b'\x11\x00', b'\x13\x11\x00', b'\r', b'X')
    check('quote NUL', rr.saved() == b'\x00ab\x00Xcd\n', repr(open(rr.fname, 'rb').read()))
    rr.finish()

    # isearch adds a typed char as one step, backspace removes it
    rr = Run(mec, macros, u8('abc 中文 def\n'))
    rr.keys(b'\x13', u8('中文'))
    check('isearch found', rr.mm.cursor[1] == width('abc 中文'), 'cursor %r' % (rr.mm.cursor,))
    rr.keys(b'\x7f')
    check('isearch backspace', rr.mm.cursor[1] == width('abc 中'), 'cursor %r' % (rr.mm.cursor,))
    rr.keys(b'\r', b'X')
    check('isearch end', rr.saved() == u8('abc 中X文 def\n'), repr(open(rr.fname, 'rb').read()))
    rr.finish()

    # I6 - bracketed paste of UTF-8 text
    rr = Run(mec, macros)
    if b'\x1b[?2004h' in rr.mm.raw:
        rr.keys(b'\x1b[200~' + u8('é中\r😀') + b'\x1b[201~')
        check('I6 bracketed paste', rr.saved() == u8('é中\n😀x\n'), repr(open(rr.fname, 'rb').read()))
    else:
        check('I6 bracketed paste enabled', False, 'mec did not enable bracketed paste')
    rr.finish()

    # I8 - binding a non-ASCII key
    rr = Run(mec, macros, b'abc\n')
    rr.command('global-bind-key')
    rr.keys(b'forward-char\r', u8('é'), u8('é'), b'X')
    check('I8 bind non-ASCII key', rr.saved() == b'aXbc\n', repr(open(rr.fname, 'rb').read()))
    rr.finish()

    # I9 - completion ignoring case folds non-ASCII chars (buffer names ignore case)
    def bufferPrompt(names, typed, tabs=1):
        rr = Run(mec, macros)
        for nn in names:
            rr.command('find-buffer')
            rr.keys(u8(nn) + b'\r')
        rr.keys(CX + b'b', u8(typed))
        for _ in range(tabs):
            rr.keys(b'\t')
        return rr

    rr = bufferPrompt(['Ébène'], 'é')
    ml = rr.mm.screen.text(rr.mm.screen.rows - 1).rstrip()
    check('I9 fold sole', ml.endswith('Ébène'), repr(ml))
    rr.finish()

    # the common start is taken a char at a time, ß and ẞ fold the same but differ in length
    rr = bufferPrompt(['straße-1', 'STRAẞE-2'], 'str')
    ml = rr.mm.screen.text(rr.mm.screen.rows - 1).rstrip()
    check('I9 fold common', ml.endswith('strAẞE-'), repr(ml))
    rr.finish()

    # the completion list's second column starts at the same cell for wide names as for ASCII
    def secondColumn(names, typed):
        rr = bufferPrompt(names, typed, 2)
        col = None
        for row in range(rr.mm.screen.rows - 1):
            cells = rr.mm.screen.cellsAt(row)
            if rr.mm.screen.text(row).startswith(names[0]):
                first = width(names[0])
                col = next((ii for ii in range(first, len(cells)) if cells[ii] not in (' ', '')), None)
                break
        rr.finish()
        return col

    asc = secondColumn(['ab-1', 'ab-2'], 'ab')
    wide = secondColumn(['中中-1', '中中-2'], '中')
    check('I9 list columns', (asc is not None) and (wide == asc), 'ascii %r wide %r' % (asc, wide))

    # a code page terminal's byte is mapped through the code page
    rr = Run(mec, macros, lang='C')
    rr.keys(b'\xe9')
    check('code page terminal key', rr.saved() == u8('éx\n'), repr(open(rr.fname, 'rb').read()))
    rr.finish()

    print('%d passed, %d failed' % (passed, failed))
    sys.exit(1 if failed else 0)


if __name__ == '__main__':
    main()
