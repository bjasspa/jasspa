#!/usr/bin/env python3
#
# hilight.py - Hilighting tests (X2, X5): opens a C file holding non-ASCII text in mec in a
# pseudo terminal and compares the schemes drawn (@fss) for keywords, identifiers, strings,
# comments and char literals.
#
# Usage: python3 -I hilight.py <mec> [macros-dir]

import os
import shutil
import sys
import tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from screen import Mec, mecEnv  # noqa: E402

TDIR = os.path.dirname(os.path.abspath(__file__))

TEXT = ['if (x) { y = "héllo"; }',
        'ifé = 1;',
        'éif = 2;',
        '/* é if */',
        'naïve if x',
        "x = 'é';",
        "z = 'a';",
        'if中 = 3;',
        'r = "中" if']

# (name, row, col, reference row, reference col) - the cell must have the reference's scheme
SAME = [
    ('keyword', 1, 0, 1, 0),
    ('string non-ASCII', 1, 15, 1, 13),
    ('keyword followed by a letter is an identifier', 2, 0, 1, 9),
    ('keyword after a letter is an identifier', 3, 1, 1, 9),
    ('comment', 4, 5, 4, 0),
    ('keyword after a non-ASCII word', 5, 6, 1, 0),
    ('identifier with a non-ASCII letter', 5, 0, 1, 9),
    ('char literal of a non-ASCII char', 6, 4, 7, 4),
    ('keyword followed by a CJK letter', 8, 0, 1, 9),
    ('keyword after a string holding a wide char', 9, 10, 1, 0),
]
# (name, row, col, reference row, reference col) - must differ
DIFF = [
    ('keyword is not an identifier', 1, 0, 1, 9),
]

passed = failed = 0


def check(name, cond, detail=''):
    global passed, failed
    if cond:
        passed += 1
    else:
        failed += 1
        print('FAIL %s %s' % (name, detail))


def main():
    if len(sys.argv) < 2:
        sys.exit('Usage: hilight.py <mec> [macros-dir]')
    mec = os.path.abspath(sys.argv[1])
    macros = os.path.abspath(sys.argv[2] if len(sys.argv) > 2 else os.path.join(TDIR, '..', '..', 'macros'))
    home = tempfile.mkdtemp()
    src = os.path.join(home, 't.c')
    open(src, 'w', encoding='utf-8').write('\n'.join(TEXT) + '\n')
    out = os.path.join(home, 'out.txt')
    cells = sorted(set((r, c) for _, r, c, rr, rc in SAME + DIFF for r, c in ((r, c), (rr, rc))))
    mm = Mec(mec, [src], env=mecEnv(home, macros))
    mm.keys(b'\x07', 1.0)
    expr = '&spr "%s"' % ('|'.join('%d' for _ in cells)) + ''.join(' @fss %d %d' % rc for rc in cells)
    for ll in ('screen-update', 'set-variable %%hs %s' % expr, 'find-buffer "*hs*"',
               'insert-string %hs', '!force 0 write-buffer "%s"' % out):
        mm.keys(b'\x1bxexecute-line\r', 1.0)
        mm.keys(ll.encode() + b'\r', 1.5)
    mm.close()
    got = open(out).read().strip().split('|') if os.path.exists(out) else []
    shutil.rmtree(home, ignore_errors=True)
    if len(got) != len(cells):
        sys.exit('FAIL no schemes read: %r' % got)
    sch = dict(zip(cells, got))
    for name, r, c, rr, rc in SAME:
        check(name, sch[(r, c)] == sch[(rr, rc)], 'scheme %s, reference %s' % (sch[(r, c)], sch[(rr, rc)]))
    for name, r, c, rr, rc in DIFF:
        check(name, sch[(r, c)] != sch[(rr, rc)], 'both scheme %s' % sch[(r, c)])
    print('%d passed, %d failed' % (passed, failed))
    sys.exit(1 if failed else 0)


if __name__ == '__main__':
    main()
