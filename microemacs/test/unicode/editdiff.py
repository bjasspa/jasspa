#!/usr/bin/env python3
#
# editdiff.py - Runs the same ASCII editing script on several mec builds and reports where they
# differ, to check the Unicode and 8-bit builds still edit ASCII text as the original does.
#
# Usage: python3 -I editdiff.py <mec> <mec> [<mec>...]
#
# The first mec is the reference. Fields only a Unicode build adds (buffer-info and count-words
# char/byte counts) are cut before comparing.

import os
import subprocess
import sys
import tempfile

TEXT = ('The quick brown fox, jumps over\tthe lazy dog.\n'
        '\tIndented line_with under_scores and CAPS\n'
        '  two spaces then text    trailing   \n'
        'short\n'
        '\n'
        'Another paragraph of words that should be filled to the fill column by fill paragraph.\n'
        'x\ty\tz\n')

OPS = [
    'forward-char', '5 forward-char', 'backward-char', 'forward-word', '3 forward-word', 'backward-word',
    'forward-line', 'set-variable $window-acol 6', 'forward-line', 'backward-line', 'end-of-line',
    'forward-char', 'set-variable $window-acol 3', 'set-variable $window-acol 9',
    'upper-case-word', 'lower-case-word', 'capitalize-word', '-2 undo', 'transpose-chars', 'end-of-line',
    'transpose-chars', 'beginning-of-line', 'forward-kill-word', 'backward-kill-word', 'yank',
    '2 forward-delete-char', 'backward-delete-char', '-2 forward-delete-char', '-2 undo', 'undo',
    'beginning-of-buffer', 'set-mark', '2 forward-line', '4 forward-char', 'upper-case-region',
    'lower-case-region', 'exchange-point-and-mark', 'kill-region', 'yank', 'beginning-of-buffer',
    'set-variable $window-acol 4', 'set-mark', '3 forward-line', 'set-variable $window-acol 12',
    'kill-rectangle', 'beginning-of-buffer', 'set-variable $window-acol 2', 'yank-rectangle',
    '1 buffer-mode "over"', 'execute-string "XYZ\t"', '-1 buffer-mode "over"', 'tab',
    '1 buffer-mode "tab"', 'tab', 'backward-delete-tab', '-1 buffer-mode "tab"',
    'set-variable $buffer-fill-col 30', 'set-variable $buffer-fill-mode "l"', '-1 buffer-mode "justify"',
    '5 forward-line', 'fill-paragraph', 'beginning-of-buffer', '5 forward-line',
    '1 buffer-mode "justify"', 'set-variable $buffer-fill-mode "b"', 'fill-paragraph',
    'beginning-of-buffer', 'set-variable $buffer-fill-mode "c"', 'fill-paragraph',
    'beginning-of-buffer', 'set-mark', 'end-of-buffer', '-1 buffer-mode "exact"', 'sort-lines',
    'beginning-of-buffer', '2 count-words', 'R$result', '0 buffer-info', 'R$result', 'end-of-buffer',
    'backward-word', '0 buffer-info', 'R$result', '-2 undo', '3 undo', 'transpose-lines',
]


def run(mec, wdir, tag):
    lines = ['define-macro start-up', '  1 global-mode "undo"', '  find-buffer "*t*"',
             '  insert-string "%s"' % TEXT.replace('\\', '\\\\').replace('"', '\\"').replace('\t', '\\t').replace('\n', '\\n'),
             '  beginning-of-buffer']
    for ii, op in enumerate(OPS):
        if op.startswith('R'):
            lines.append('  -2 ml-write &spr "R%%d=%%s" %d %s' % (ii, op[1:]))
            continue
        # each command its own undo step, as when typed
        lines.append('  -2 undo')
        lines.append('  !force %s' % op)
        lines.append('  -2 ml-write &spr "R%%d=%%d|%%d|%%d|%%s" %d $window-line $window-col $window-acol @wl' % ii)
    lines += ['  !force 0 write-buffer "%s"' % os.path.join(wdir, tag + '.out'), '  4 exit-emacs 0', '!emacro']
    open(os.path.join(wdir, tag + '.emf'), 'w').write('\n'.join(lines) + '\n')
    res = subprocess.run([mec, '-p', '@' + os.path.join(wdir, tag)], stdin=subprocess.DEVNULL,
                         capture_output=True, env=dict(os.environ, MEPATH=wdir), timeout=60)
    log = (res.stdout + res.stderr).decode('latin-1').replace('\r', '\n')
    out = [ll for ll in log.split('\n') if ll.startswith('R')]
    try:
        out.append('FILE=' + open(os.path.join(wdir, tag + '.out'), 'rb').read().decode('latin-1'))
    except OSError:
        out.append('FILE missing')
    return out


def trim(line):
    # buffer-info has 12 fields in the original, count-words 4
    if line.startswith('R') and '=|' in line:
        key, val = line.split('=', 1)
        ff = val.split('|')
        if len(ff) > 14:
            val = '|'.join(ff[:13]) + '|'
        elif len(ff) == 7:
            val = '|'.join(ff[:5]) + '|'
        return key + '=' + val
    return line


def main():
    if len(sys.argv) < 3:
        sys.exit('Usage: editdiff.py <mec> <mec> [<mec>...]')
    wdir = tempfile.mkdtemp()
    outs = [[trim(ll) for ll in run(os.path.abspath(mm), wdir, 't%d' % ii)] for ii, mm in enumerate(sys.argv[1:])]
    failed = 0
    for jj in range(1, len(outs)):
        for ii in range(max(len(outs[0]), len(outs[jj]))):
            aa = outs[0][ii] if ii < len(outs[0]) else None
            bb = outs[jj][ii] if ii < len(outs[jj]) else None
            if aa != bb:
                failed += 1
                print('DIFF %s step %d:\n  ref %r\n  got %r' % (sys.argv[jj + 1], ii, aa, bb))
                break
    print('%d steps compared, %d builds differ' % (len(outs[0]), failed))
    sys.exit(1 if failed else 0)


if __name__ == '__main__':
    main()
