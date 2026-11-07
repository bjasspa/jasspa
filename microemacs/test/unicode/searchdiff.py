#!/usr/bin/env python3
#
# searchdiff.py - Runs the same ASCII searches and replaces on several mec builds and reports where
# they differ, to check the char based scanner and regex leave ASCII searching as it was.
#
# Usage: python3 -I searchdiff.py <mec> <mec> [<mec>...]
#
# [[:upper:]] and [[:lower:]] are left out, the original swaps them (fixed in the Unicode build).

import os
import subprocess
import sys
import tempfile

TEXT = ('The quick brown Fox jumps over the lazy dog.\n'
        'foo_bar = foo-bar + FOO(bar) ; 0x1F 42\n'
        'aaa abab abba Abba ABBA\n'
        '\ttabbed\tline  with   spaces\n'
        'end of the text\n')

# (magic, exact, command, pattern[, replacement])
CASES = [
    (0, 1, 'search-forward', 'the'), (0, 0, 'search-forward', 'the'), (0, 0, 'search-forward', 'FOX'),
    (0, 1, 'search-backward', 'ab'), (0, 0, 'search-backward', 'aBb'), (0, 1, 'search-forward', 'bar\\n'),
    (0, 1, 'search-forward', 'no such'), (0, 0, 'search-backward', 'THE'),
    (1, 1, 'search-forward', 'b.r'), (1, 1, 'search-forward', '[A-Z][a-z]+'), (1, 0, 'search-forward', 'abba'),
    (1, 1, 'search-forward', '\\\\<ab+a\\\\>'), (1, 1, 'search-forward', '\\\\w+_\\\\w+'),
    (1, 1, 'search-forward', '[^a-z ]+'), (1, 1, 'search-forward', '0x\\\\h+'), (1, 1, 'search-forward', '\\\\d\\\\{2\\\\}'),
    (1, 1, 'search-forward', '\\\\(ab\\\\)\\\\1'), (1, 1, 'search-forward', '\\\\s+with'), (1, 1, 'search-forward', '[[:digit:]]+'),
    (1, 1, 'search-forward', '[[:punct:]]+'), (1, 1, 'search-forward', 'a*b?a+'), (1, 1, 'search-forward', 'x\\\\|y\\\\|z'),
    (1, 1, 'search-forward', '\\\\bover\\\\b'), (1, 1, 'search-forward', '\\\\Bver'), (1, 1, 'search-forward', 'dog\\\\.$'),
    (1, 1, 'search-forward', '^\\\\t'), (1, 1, 'search-forward', 'ba.*?r'), (1, 1, 'search-forward', '\\\\W+'),
    (1, 1, 'search-backward', 'b\\\\w'), (1, 0, 'search-backward', '[a]bba'), (1, 1, 'search-backward', '\\\\<'),
    (1, 1, 'search-forward', 'text\\\\n\\\\\''),
    (0, 1, 'replace-string', 'foo', 'XyZ'), (0, 0, 'replace-string', 'ABBA', 'q'),
    (1, 1, 'replace-string', '\\\\(\\\\w+\\\\)-\\\\(\\\\w+\\\\)', '\\\\u2.\\\\c1'),
    (1, 1, 'replace-string', '\\\\<\\\\(a\\\\w*\\\\)', '[\\\\{1,<,>,8\\\\}]'),
    (1, 0, 'replace-string', 'the', '\\\\c&!'),
]


def run(mec, wdir, tag):
    mac = ['define-macro start-up']
    for ii, case in enumerate(CASES):
        magic, exact, cmd, pat = case[:4]
        mac += ['  !force 0 delete-buffer "*t*"', '  find-buffer "*t*"',
                '  insert-string "%s"' % TEXT.replace('\t', '\\t').replace('\n', '\\n'),
                '  %d buffer-mode "magic"' % (1 if magic else -1), '  %d buffer-mode "exact"' % (1 if exact else -1)]
        if cmd == 'search-forward':
            mac.append('  beginning-of-buffer')
            mac.append('  forward-line')
        else:
            mac.append('  end-of-buffer')
            mac.append('  backward-line')
        if cmd == 'replace-string':
            mac.append('  beginning-of-buffer')
            mac.append('  !force %s "%s" "%s"' % (cmd, pat, case[4]))
            mac.append('  -2 ml-write &spr "R%d=%%d|%%s" $status &lget $result 2' % ii)
            mac.append('  !force 0 write-buffer "%s"' % os.path.join(wdir, '%s.%d' % (tag, ii)))
        else:
            mac.append('  !force %s "%s"' % (cmd, pat))
            # @s0 is an error after a failed search
            mac += ['  !if $status',
                    '    -2 ml-write &spr "R%d=1|%%d|%%d|%%s" $window-line $window-col @s0' % ii,
                    '  !else',
                    '    -2 ml-write &spr "R%d=0|%%d|%%d" $window-line $window-col' % ii,
                    '  !endif']
    mac += ['  4 exit-emacs 0', '!emacro']
    open(os.path.join(wdir, tag + '.emf'), 'w').write('\n'.join(mac) + '\n')
    res = subprocess.run([mec, '-p', '@' + os.path.join(wdir, tag)], stdin=subprocess.DEVNULL,
                         capture_output=True, env=dict(os.environ, MEPATH=wdir), timeout=120)
    log = (res.stdout + res.stderr).decode('latin-1').replace('\r', '\n')
    out = [ll[ll.index('R'):] for ll in log.split('\n') if 'R' in ll and '=' in ll and ll[ll.index('R') + 1:ll.index('=')].isdigit()]
    for ii, case in enumerate(CASES):
        if case[2] == 'replace-string':
            path = os.path.join(wdir, '%s.%d' % (tag, ii))
            out.append('F%d=%r' % (ii, open(path, 'rb').read() if os.path.exists(path) else None))
    return out


def main():
    if len(sys.argv) < 3:
        sys.exit('Usage: searchdiff.py <mec> <mec> [<mec>...]')
    wdir = tempfile.mkdtemp()
    outs = [run(os.path.abspath(mm), wdir, 't%d' % ii) for ii, mm in enumerate(sys.argv[1:])]
    failed = 0
    for jj in range(1, len(outs)):
        for aa, bb in zip(outs[0], outs[jj]):
            if aa != bb:
                failed += 1
                print('DIFF %s:\n  ref %r\n  got %r' % (sys.argv[jj + 1], aa, bb))
        if len(outs[0]) != len(outs[jj]):
            failed += 1
            print('DIFF %s: %d results, ref %d' % (sys.argv[jj + 1], len(outs[jj]), len(outs[0])))
    print('%d results compared, %d differ' % (len(outs[0]), failed))
    sys.exit(1 if failed else 0)


if __name__ == '__main__':
    main()
