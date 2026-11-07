#!/usr/bin/env python3
#
# compiled.py - Compiled macros (unicode_design.md section 17), what is new by design so not
# compared with src.old (macroflow.py does that): a macro defined while $debug is 0 keeps no source,
# so insert-macro fails, while one defined with $debug set gives its lines, and an error in it
# names its file and line; name-kbd-macro's macro keeps its source whatever $debug is; in a
# pseudo terminal, stepping ($debug 2) runs a compiled macro as one step, and C-g ($debug 0x10)
# breaks into a compiled loop at once, saying it is compiled. Either build.
#
# Usage: python3 -I compiled.py <mec> [macros-dir]

import os
import shutil
import subprocess
import sys
import tempfile
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from screen import Mec, mecEnv  # noqa: E402

TDIR = os.path.dirname(os.path.abspath(__file__))

passed = failed = 0


def check(name, cond, detail=''):
    global passed, failed
    if cond:
        passed += 1
    else:
        failed += 1
        print('FAIL %s %s' % (name, detail))


DEFS = r'''define-macro nosrc
  insert-string "n"
!emacro
set-variable $debug 1
define-macro withsrc
  insert-string "w"
    ; a comment
  !if 0
    bad-command-xyz
  !endif
!emacro
define-macro failing
  insert-string "f"
  bad-command-xyz
!emacro
set-variable $debug 0
'''


def headless(mec, wdir):
    open(os.path.join(wdir, 'cdefs.emf'), 'w').write(DEFS)
    mac = ['define-macro start-up', '  execute-file "cdefs"', '  find-buffer "*t*"',
           '  !force insert-macro "nosrc"', '  -2 ml-write &cat "R:nosrc=" $status',
           '  !force insert-macro "withsrc"', '  -2 ml-write &cat "R:withsrc=" $status',
           '  beginning-of-buffer', '  set-variable #l1 ""', '  !repeat',
           '    set-variable #l1 &cat #l1 &cat @wl "|"', '    !force forward-line', '  !until &not $status',
           '  -2 ml-write &cat "R:text=" #l1',
           # an error with $debug's 0x01 bit names the file and line; not forced, so it is printed
           '  set-variable $debug 1', '  -2 ml-write "R:go=1"', '  failing', '  4 exit-emacs 0', '!emacro']
    open(os.path.join(wdir, 'ch.emf'), 'w').write('\n'.join(mac) + '\n')
    # failing aborts start-up, so the run is killed after a while
    res = subprocess.run(['timeout', '-s', 'KILL', '15', mec, '-n', '-p', '@' + os.path.join(wdir, 'ch')],
                         stdin=subprocess.DEVNULL, capture_output=True, env=dict(os.environ, MEPATH=wdir))
    log = (res.stdout + res.stderr).decode('utf-8', 'replace').replace('\r', '\n')
    got = dict(ll[ll.index('R:') + 2:].split('=', 1) for ll in log.split('\n') if 'R:' in ll and '=' in ll)
    check('insert-macro no source', got.get('nosrc') == '0', repr(got.get('nosrc')))
    check('insert-macro source', got.get('withsrc') == '1', repr(got.get('withsrc')))
    want = 'define-macro withsrc|insert-string "w"|!if 0|bad-command-xyz|!endif|!emacro||'
    check('insert-macro text', got.get('text') == want, repr(got.get('text')))
    want = '[bad-command-xyz:: Error executing failing, %s line 14]' % os.path.join(wdir, 'cdefs.emf')
    check('error location', want in log, repr([ll for ll in log.split('\n') if 'executing' in ll]))


def waitFor(mm, text, secs=5):
    end = time.time() + secs
    while time.time() < end:
        if text in mm.screen.text(mm.screen.rows - 1):
            return True
        mm.settle(0.3)
    return False


def pty(mec, macros, wdir):
    home = os.path.join(wdir, 'home')
    os.mkdir(home)
    open(os.path.join(home, 'pdefs.emf'), 'w').write(r'''define-macro comp
  insert-string "c"
  insert-string "c"
!emacro
define-macro spin
  !repeat
    forward-char
    backward-char
  !until 0
!emacro
set-variable $debug 1
define-macro dbg
  comp
  insert-string "d"
!emacro
set-variable $debug 0
''')
    fname = os.path.join(wdir, 'in.txt')
    open(fname, 'w').write('abc\n')
    mm = Mec(mec, [fname], env=mecEnv(home, macros))
    try:
        rows = mm.screen.rows
        mm.keys(b'\x1bxexecute-line\rexecute-file "pdefs"\r')
        # name-kbd-macro keeps the source with $debug 0
        mm.keys(b'\x18(xyz\x18)')
        mm.keys(b'\x1bxname-kbd-macro\rkk\r')
        mm.keys(b'\x1bxfind-buffer\r*ins*\r')
        mm.keys(b'\x1bxinsert-macro\rkk\r')
        screen = '\n'.join(mm.screen.text(rr) for rr in range(rows))
        check('name-kbd-macro insert', 'execute-string "xyz' in screen, repr(screen[:300]))
        # stepping: dbg's lines are shown, comp runs as one step
        mm.keys(b'\x1bxexecute-line\rset-variable $debug 2\r')
        mm.keys(b'\x1bxdbg\r')
        first = mm.screen.text(rows - 1)
        check('step shows source', 'dbg:' in first and '[comp]' in first, repr(first))
        mm.keys(b' ')
        second = mm.screen.text(rows - 1)
        check('step over compiled', 'dbg:' in second and 'insert-string "d"' in second, repr(second))
        mm.keys(b'c')
        mm.keys(b'\x1bxexecute-line\rset-variable $debug 0\r')
        # C-g in a compiled loop breaks at once
        mm.keys(b'\x1bxexecute-line\rset-variable $debug 0x10\r')
        mm.keys(b'\x1bxspin\r', 1.0)
        mm.keys(b'\x07', 1.0)
        broke = waitFor(mm, 'spin: compiled ?')
        check('break in compiled', broke, repr(mm.screen.text(rows - 1)))
        mm.keys(b'\x07', 1.0)
        mm.keys(b'\x1bxexecute-line\rset-variable $debug 0\r')
        mm.keys(b'\x1bxexecute-line\rml-write "alive"\r')
        check('aborted', 'alive' in mm.screen.text(rows - 1), repr(mm.screen.text(rows - 1)))
    finally:
        mm.close()


def main():
    if len(sys.argv) < 2:
        sys.exit('Usage: compiled.py <mec> [macros-dir]')
    mec = os.path.abspath(sys.argv[1])
    macros = os.path.abspath(sys.argv[2] if len(sys.argv) > 2 else os.path.join(TDIR, '..', '..', 'macros'))
    wdir = tempfile.mkdtemp()
    headless(mec, wdir)
    pty(mec, macros, wdir)
    shutil.rmtree(wdir, ignore_errors=True)
    print('%d passed, %d failed' % (passed, failed))
    sys.exit(1 if failed else 0)


if __name__ == '__main__':
    main()
