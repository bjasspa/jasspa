#!/usr/bin/env python3
#
# inputdiff.py - Sends the same ASCII key sequences to several mec builds in a pseudo terminal
# and reports where the saved file or message line differ, to check the key changes leave ASCII
# input as the original handles it.
#
# Usage: python3 -I inputdiff.py <mec> <mec> [<mec>...] [--macros dir]

import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from input import Run, ESC, CX  # noqa: E402

TDIR = os.path.dirname(os.path.abspath(__file__))

CASES = [
    ('typing', b'one two\n', [b'hello ', b'world\r', b'\x7f\x7fX']),
    ('prompt edit', b'x\n', [ESC + b'xinsert-string\r', b'abc def ghi', b'\x02\x02\x04', b'\x01', ESC + b'f',
                             ESC + b'u', ESC + b'c', b'\x14', b'\x05', b'\x7f', b'\x0b', b'\x19', b'\r']),
    ('kbd macro', b'x\n', [CX + b'(', b'ab', b'\x02', b'c', CX + b')', CX + b'e', b'\x15' + b'3' + CX + b'e']),
    ('isearch', b'the cat sat on the mat\n', [b'\x13', b'at', b'\x13', b'\x7f', b'\x7fs', b'\r', b'Z']),
    ('quote', b'x\n', [b'\x11\x01', b'\x11\x7f', b'\x15' + b'2\x11a']),
    ('bind', b'abc\n', [ESC + b'xglobal-bind-key\r', b'forward-char\r', b'q', b'qqY']),
    ('arrows', b'abc\ndef\n', [b'\x1b[B', b'\x1b[C', b'\x1b[C', b'P', b'\x1b[A', b'Q', b'\x1b[H', b'R']),
    ('esc prefix', b'one two three\n', [ESC + b'f', ESC + b'd', ESC + b'b', ESC + b'u', b'\x05', ESC + b'\x7f']),
    ('completion', b'x\n', [ESC + b'xinsert-str\t', b'\rok\r']),
    ('overwrite', b'abcdef\n', [ESC + b'xbuffer-mode\rover\r', b'XY\t', b'\x05Z']),
]


def runAll(mec, macros):
    out = []
    for name, content, keys in CASES:
        rr = Run(mec, macros, content)
        rr.keys(*keys)
        ml = ''.join(rr.mm.screen.cells[rr.mm.screen.rows - 1]).rstrip()
        out.append((name, rr.saved(), ml))
        rr.finish()
    return out


def main():
    args = sys.argv[1:]
    macros = os.path.join(TDIR, '..', '..', 'macros')
    if '--macros' in args:
        ii = args.index('--macros')
        macros = args[ii + 1]
        del args[ii:ii + 2]
    if len(args) < 2:
        sys.exit('Usage: inputdiff.py <mec> <mec> [<mec>...] [--macros dir]')
    macros = os.path.abspath(macros)
    res = [runAll(os.path.abspath(mm), macros) for mm in args]
    diff = 0
    for jj in range(1, len(res)):
        for aa, bb in zip(res[0], res[jj]):
            if aa != bb:
                diff += 1
                print('DIFF %s %s:\n  ref %r\n  got %r' % (args[jj], aa[0], aa[1:], bb[1:]))
    print('%d cases compared, %d differ' % (len(CASES), diff))
    sys.exit(1 if diff else 0)


if __name__ == '__main__':
    main()
