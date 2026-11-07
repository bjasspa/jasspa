#!/usr/bin/env python3
#
# macrodiff.py - Evaluates the same ASCII macro string expressions on several mec builds and
# reports where they differ, to check the char based string functions leave ASCII as it was.
#
# Usage: python3 -I macrodiff.py <src.old-mec> <mec> [<mec>...]
#
# The original src (src.old) lacks the byte functions and the &atoi and negative count fixes, so when the
# first build is it they are only compared across the other builds.

import os
import subprocess
import sys
import tempfile

# expressions every build has
OLD = [
    '&len "hello world"', '&lef "hello" 3', '&lef "hello" -2', '&lef "hello" 0', '&lef "hello" 10',
    '&lef "hello" -10', '&rig "hello" 2', '&rig "hello" -2', '&rig "hello" 10', '&rig "hello" -10',
    '&mid "hello" 1 3', '&mid "hello" -3 2', '&mid "hello" 1 -1', '&mid "hello" -10 3', '&mid "hello" 2 10',
    '&mid "hello" 0 -2', '&mid "hello" 10 2', '&mid "hello" -2 -1',
    '&sin "l" "hello"', '&sin "" "hello"', '&rsin "l" "hello"', '&rsin "" "hello"', '&isin "LL" "hello"',
    '&risin "L" "hello"', '&sin "x" "hello"', '&sin "lo" "hello"',
    '&rep "hello" "l" "L"', '&rep "hello" "" "-"', '&irep "HeLLo" "l" "x"', '&rep "aaa" "aa" "b"',
    '&irep "abc" "" "."', '&slo "HeLLo 123"', '&sup "HeLLo 123"',
    '&atoi "A"', '&atoi ""', '&itoa 65', '&len &itoa 28', '&atoi &itoa 28', '&inw "a"', '&inw "-"',
    '&cat &cat "[" &trb "  x  " "]"', '&cat &cat "[" &trl "  x  " "]"', '&cat &cat "[" &trr "  x  " "]"',
    '&lget "|a|b|c|" 2', '&lle "|a|b|c|"', '&lfind "|a|b|c|" "c"', '&lfind "|a|b|c|" "d"',
    '&lset "|a|b|c|" 2 "X"', '&lset "|a|b|c|" -1 "X"', '&lins "|a|b|c|" 1 "X"', '&lins "|a|b|c|" -1 "X"',
    '&lins "|b|d|" -2 "c"', '&lins "|B|d|" -3 "c"', '&lins "|a|b|" 0 "X"', '&lde "|a|b|c|" 2',
    '&lget "|a|b|c|" 5', '&lget ":a:b:" 1',
    '&spr "[%5s]" "ab"', '&spr "[%-5s]" "ab"', '&spr "[%.1s]" "ab"', '&spr "[%-4.2s]" "abc"',
    '&spr "[%3s]" "abcdef"', '&spr "%d-%x-%s" 10 255 "z"', '&spr "%n" 3 "ab"', '&spr "[%+5s]" "ab"',
    '&ise "ABC" "abc"', '&ise "abc" "abd"', '&seq "abc" "abc"', '&sle "abc" "abd"',
    '&len "a\\x00b"', '&len "a\\sb"', '&mid "a\\x1cb" 1 1', '&len "\\x41\\x42"', '&cat "\\x41" "\\t"',
    '&xrep "hello" "l+" "L"',
]
# the byte functions and the src.old bugs fixed
NEW = [
    '&blen "abc"', '&bhex "AB~"', '&hexb "414243"', '&batoi "AB"', '&batoi &bmid "AB" -1 1', '&batoi ""',
    '&bmid "hello" 1 3', '&bmid "hello" -2 5', '&bitoa 65', '&blen "a\\x00b"', '&bhex "a\\x00b"',
    '&bhex &bitoa 0', '&bhex &bitoa 28', '&bhex &hexb "61001c62"', '&atoi "\\x00"', '&lef "a\\x00bc" -1',
    '&rig "a\\x00bc" -1', '&mid "a\\x00bc" -2 1',
]


def run(mec, wdir, tag):
    mac = ['define-macro start-up', '  !force 0 delete-buffer "*t*"', '  find-buffer "*t*"',
           '  insert-string "hello"', '  beginning-of-line', '  3 forward-char',
           '  -2 ml-write &cat "R0=" $window-col', '  !force set-variable $window-col 10',
           '  -2 ml-write &cat &cat "R1=" $status $window-col', '  set-variable $window-col 2',
           '  -2 ml-write &cat "R2=" @wc']
    for ii, expr in enumerate(OLD + NEW):
        mac.append('  !force set-variable #l0 %s' % expr)
        mac.append('  -2 ml-write &cat &cat "X%d=" $status &cat "|" #l0' % ii)
    mac += ['  4 exit-emacs 0', '!emacro']
    open(os.path.join(wdir, tag + '.emf'), 'w').write('\n'.join(mac) + '\n')
    res = subprocess.run([mec, '-p', '@' + os.path.join(wdir, tag)], stdin=subprocess.DEVNULL,
                         capture_output=True, env=dict(os.environ, MEPATH=wdir), timeout=120)
    log = (res.stdout + res.stderr).decode('latin-1').replace('\r', '\n')
    out = {}
    for ll in log.split('\n'):
        if '=' in ll and ll[:1] in 'RX' and ll[1:ll.index('=')].isdigit():
            out[ll[:ll.index('=')]] = ll[ll.index('=') + 1:]
    return out


def main():
    if len(sys.argv) < 3:
        sys.exit('Usage: macrodiff.py <src.old-mec> <mec> [<mec>...]')
    wdir = tempfile.mkdtemp()
    outs = [run(os.path.abspath(mm), wdir, 't%d' % ii) for ii, mm in enumerate(sys.argv[1:])]
    keys = ['R0', 'R1', 'R2'] + ['X%d' % ii for ii in range(len(OLD))]
    newKeys = ['X%d' % ii for ii in range(len(OLD), len(OLD) + len(NEW))]
    exprs = dict(('X%d' % ii, ee) for ii, ee in enumerate(OLD + NEW))
    # the original src (src.old) has no &blen
    orig = outs[0].get(newKeys[0], '')[:1] != '1'
    failed = compared = 0
    for kk in keys + newKeys:
        ref = 1 if (kk in newKeys) and orig else 0
        if ref >= len(outs) - 1:
            continue
        compared += 1
        if outs[ref].get(kk) is None:
            failed += 1
            print('MISSING %s %s' % (kk, exprs.get(kk, '')))
            continue
        for jj in range(ref + 1, len(outs)):
            if outs[ref].get(kk) != outs[jj].get(kk):
                failed += 1
                print('DIFF %s %s:\n  ref %r\n  got %r' % (sys.argv[jj + 1], exprs.get(kk, kk),
                                                         outs[ref].get(kk), outs[jj].get(kk)))
    print('%d results compared, %d differ' % (compared, failed))
    sys.exit(1 if failed else 0)


if __name__ == '__main__':
    main()
