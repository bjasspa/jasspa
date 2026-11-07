#!/usr/bin/env python3
#
# fuzzdisp.py - Display fuzzing (Phase 8, acceptance test P2, the half fuzz.py cannot reach as a
# headless mec never draws): runs mec in a pseudo terminal under valgrind with me.emf loaded, so
# file hooks set up hilighting, and a start-up macro which opens fuzz.py's generated files under
# source file names (.c, .py, .html, ...) and redraws after each move through them: by pages, to
# the line ends, scrolled sideways and to the end, some with show white space ($system 0x80000)
# set. Batches alternate between a UTF-8 terminal and a code page terminal (the C locale). Each
# run must exit normally with no valgrind errors or leaks.
#
# Usage: python3 -I fuzzdisp.py <mec> [-n files] [-s seed] [-b batch] [-k dir] [macros-dir]

import argparse
import os
import random
import re
import shutil
import sys
import tempfile
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fuzz import generate, macroString  # noqa: E402
from screen import Mec, mecEnv  # noqa: E402

EXTS = ['.c', '.py', '.html', '.md', '.txt', '.sh', '.emf', '.java', '.xml', '.tex']


def batchMacro(rng, files):
    mm = ['execute-file "me"', 'define-macro start-up']
    L = mm.append
    for path in files:
        white = rng.random() < 0.3
        L('  !force find-file "%s"' % macroString(path))
        L('  !if $status')
        if white:
            L('    set-variable $system &bor $system 0x80000')
        L('    beginning-of-buffer')
        L('    screen-update')
        L('    set-variable #l0 0')
        L('    !while &les #l0 12')
        L('      !force 20 forward-line')
        L('      screen-update')
        L('      end-of-line')
        L('      screen-update')
        L('      !force set-variable $window-col 120')
        L('      screen-update')
        L('      beginning-of-line')
        L('      set-variable #l0 &add #l0 1')
        # stop once the end is reached, a file of a few 64K lines is otherwise redrawn 36 times
        L('      !force forward-line')
        L('      !if &not $status')
        L('        set-variable #l0 12')
        L('      !endif')
        L('    !done')
        L('    end-of-buffer')
        L('    screen-update')
        L('    !force 10 backward-line')
        L('    !force 3 forward-word')
        L('    screen-update')
        if white:
            L('    set-variable $system &band $system &bnot 0x80000')
        L('    !force 0 delete-buffer $buffer-bname')
        L('  !endif')
    L('  4 exit-emacs 0')
    L('!emacro')
    return '\n'.join(mm) + '\n'


def runBatch(args, rng, batch, wdir, lang):
    files = []
    for ii, (kind, data) in enumerate(batch):
        path = os.path.join(wdir, 'f%d%s' % (ii, rng.choice(EXTS)))
        open(path, 'wb').write(data)
        files.append(path)
    open(os.path.join(wdir, 'fd.emf'), 'w', encoding='utf-8').write(batchMacro(rng, files))
    vlog = os.path.join(wdir, 'vg.log')
    env = mecEnv(wdir, args.macros, lang)
    mm = Mec('/usr/bin/valgrind', ['--leak-check=full', '--error-exitcode=99', '--log-file=' + vlog,
                                   args.mec, '@' + os.path.join(wdir, 'fd')], env=env)
    end = time.time() + args.timeout
    status = 'timeout'
    while time.time() < end:
        mm.settle(1.0)
        pid, st = os.waitpid(mm.pid, os.WNOHANG)
        if pid:
            status = os.waitstatus_to_exitcode(st)
            break
    if status == 'timeout':
        mm.close()
    else:
        os.close(mm.fd)
    fails = []
    log = open(vlog, encoding='utf-8', errors='replace').read() if os.path.exists(vlog) else ''
    mm = re.search(r'ERROR SUMMARY: (\d+) errors', log)
    errs = mm.group(1) if mm else '?'
    if status != 0 or errs != '0':
        fails.append('exit %s, valgrind %s errors (%s terminal)' % (status, errs, lang))
    return fails


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('mec')
    ap.add_argument('macros', nargs='?',
                    default=os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'macros'))
    ap.add_argument('-n', type=int, default=60, help='number of files')
    ap.add_argument('-s', type=int, default=1, help='random seed')
    ap.add_argument('-b', type=int, default=10, help='files per run')
    ap.add_argument('-k', help='keep the files of failing batches in this directory')
    ap.add_argument('--timeout', type=int, default=1800)
    args = ap.parse_args()
    args.mec = os.path.abspath(args.mec)
    args.macros = os.path.abspath(args.macros)
    rng = random.Random(args.s)
    bad = runs = 0
    for start in range(0, args.n, args.b):
        batch = [generate(rng) for _ in range(min(args.b, args.n - start))]
        lang = 'C.UTF-8' if runs % 2 == 0 else 'C'
        runs += 1
        wdir = tempfile.mkdtemp()
        fails = runBatch(args, rng, batch, wdir, lang)
        if fails:
            bad += 1
            print('batch at file %d (seed %d):' % (start, args.s))
            for ff in fails:
                print('  FAIL ' + ff)
            if args.k:
                keep = os.path.join(args.k, 'batch%d' % start)
                shutil.copytree(wdir, keep, dirs_exist_ok=True)
                # the kept macro names the kept files, so the batch can be run again
                kmac = os.path.join(keep, 'fd.emf')
                text = open(kmac, encoding='utf-8').read().replace(wdir, keep)
                open(kmac, 'w', encoding='utf-8').write(text)
                print('  kept in ' + keep)
        shutil.rmtree(wdir, ignore_errors=True)
        sys.stdout.flush()
    print('%d files in %d runs, %d runs failed' % (args.n, runs, bad))
    sys.exit(1 if bad else 0)


if __name__ == '__main__':
    main()
