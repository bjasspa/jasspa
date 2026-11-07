#!/usr/bin/env python3
#
# macroflow.py - Runs the same macros on several mec builds and reports where their traces
# differ, to check the macro speed-ups change nothing: skipping a false branch straight to the
# line which ends it (if/elif/else chains with nested blocks, loops with !break and !continue,
# !goto and !jump into blocks, unknown directives and a missing !endif in skipped lines, a
# define-macro in a skipped block, execute-file and execute-buffer lines), number parsing, the
# regex cache (patterns alternated with @s groups, case flags, &xrep, search-buffer), copying
# long values, and for compiled macros a goto label in a variable, numeric arguments from
# functions, !iif, execute-line, @mx, key strings, define-help text in a macro and insert-macro
# of a macro defined with $debug set. The first build is the reference, e.g. src.old's mec.
#
# Usage: python3 -I macroflow.py <ref-mec> <mec> [<mec>...]

import os
import shutil
import subprocess
import sys
import tempfile

MACROS = r'''
0 define-macro t
!emacro
define-macro tlog
  set-variable .t.log &cat .t.log &cat @1 ","
!emacro

; if/elif/else chains, each branch holding nested blocks which must be skipped as a whole
define-macro chain
  set-variable #l0 @1
  !if &equ #l0 0
    tlog "a0"
    !if &equ #l0 1
      tlog "bad1"
    !elif 1
      tlog "a0n"
    !else
      tlog "bad2"
    !endif
  !elif &equ #l0 1
    tlog "a1"
    !while 0
      tlog "bad3"
      !if 1
        tlog "bad4"
      !endif
    !done
    !repeat
      tlog "a1r"
    !until 1
  !elif &equ #l0 2
    !if 0
      !if 1
        tlog "bad5"
      !elif 1
        tlog "bad6"
      !else
        tlog "bad7"
      !endif
    !elif 0
      tlog "bad8"
    !else
      tlog "a2e"
      !iif 0  tlog "bad9"
      !iif 1  tlog "a2i"
    !endif
  !else
    tlog "aelse"
    !if 0
      !while 1
        !break
      !done
      !abort
    !endif
  !endif
  tlog "end"
!emacro

; loops with break and continue inside nested ifs
define-macro loops
  set-variable #l0 0
  !while &les #l0 6
    set-variable #l0 &add #l0 1
    !if &equ #l0 2
      !continue
    !elif &equ #l0 5
      !if 1
        !break
      !endif
      tlog "bad"
    !endif
    set-variable #l1 0
    !repeat
      set-variable #l1 &add #l1 1
      !if &equ #l1 2
        !if 0
          !break
        !else
          tlog &cat "r" #l0
        !endif
      !endif
    !until &gre #l1 2
    tlog &cat "w" #l0
  !done
  ; a while which never runs, twice so its end is known the second time
  set-variable #l2 0
  !while &les #l2 2
    set-variable #l2 &add #l2 1
    !while 0
      !if 1
        tlog "bad"
      !endif
    !done
    tlog &cat "x" #l2
  !done
!emacro

; jumps into blocks: an !else or !elif reached by a jump skips to the !endif
define-macro jumps
  !goto into
  !if 0
    tlog "bad1"
*into
    tlog "in"
  !elif 1
    tlog "bad2"
  !else
    tlog "bad3"
  !endif
  !jump 3
  !if 1
    tlog "bad4"
    tlog "j"
  !else
    tlog "bad5"
  !endif
  !tjump 1 2
  !if 0
  !elif 1
    tlog "tj"
  !else
    tlog "bad6"
  !endif
  set-variable #l0 0
*back
  set-variable #l0 &add #l0 1
  !if &les #l0 3
    tlog &cat "g" #l0
    !goto back
  !endif
!emacro

; recursion through skipped branches
define-macro recur
  !if &les @1 1
    tlog "base"
  !elif &equ &mod @1 2 0
    tlog &cat "even" @1
    recur &sub @1 1
  !else
    tlog &cat "odd" @1
    recur &sub @1 1
  !endif
!emacro

; a skipped block holding an unknown directive is an error
define-macro baddir
  tlog "b1"
  !if 0
    !frobnicate
  !endif
  tlog "b2"
!emacro

; no !endif
define-macro noend
  tlog "n1"
  !if 0
    tlog "bad"
  tlog "n2"
!emacro

; a define-macro, with if blocks, run inside a branch, and one in a skipped branch
define-macro defs
  !if 1
    define-macro inner
      !if &equ @1 1
        tlog "i1"
        define-macro inner2
          !if 1
            tlog "i2"
          !endif
        !emacro
      !else
        tlog "ie"
      !endif
    !emacro
  !else
    tlog "bad"
  !endif
  inner 1
  inner 0
  inner2
!emacro

define-macro numbers
  !force set-variable #l0 &add "-0" 0
  tlog #l0
  set-variable #l1 0
  !while &les #l1 &lle .t.nums
    set-variable #l1 &add #l1 1
    tlog &add &lget .t.nums #l1 0
  !done
  set-variable #l1 0
  !while &les #l1 &lle .t.nums
    set-variable #l1 &add #l1 1
    tlog &mul &lget .t.nums #l1 1
  !done
!emacro
set-variable .t.nums "|0|7|-5|007|08|0x1F|-0x10| 5|+5|123abc|999999999|1234567890|-1234567890|2147483647|2147483648|-2147483648|99999999999|1e5|-||-x|0.5|"

define-macro regex
  set-variable #l0 0
  !while &les #l0 3
    set-variable #l0 &add #l0 1
    !if &xseq "abc123" "\\([a-z]+\\)\\([0-9]+\\)"
      tlog &cat @s1 &cat ":" @s2
    !endif
    !if &xseq "x-y" "\\(.\\)-\\(.\\)"
      tlog &cat @s2 @s1
    !endif
    !if &xiseq "ABC123" "\\([a-z]+\\)\\([0-9]+\\)"
      tlog &cat "i" @s2
    !endif
    !if &xseq "ABC123" "\\([a-z]+\\)\\([0-9]+\\)"
      tlog "bad"
    !endif
    tlog &xrep "a1b2c3" "\\([a-z]\\)\\([0-9]\\)" "\\2\\1"
    tlog &xirep "A1b2" "[a-z]" "."
    tlog &xseq "zzz" "z*"
  !done
  ; @s after a failed match, then an earlier pattern again
  set-variable #l1 &xseq "q" "\\(w\\)"
  !if &xseq "abc123" "\\([a-z]+\\)\\([0-9]+\\)"
    tlog @s2
  !endif
  ; many patterns, more than any cache holds
  set-variable #l0 0
  !while &les #l0 40
    set-variable #l0 &add #l0 1
    !if &xseq &cat "k" #l0 &cat "k\\(" &cat #l0 "\\)"
      set-variable #l2 @s1
    !endif
  !done
  tlog #l2
  ; search-buffer and &xseq share the main regex
  find-buffer "*rx*"
  insert-string "one two three\ntwo four\n"
  beginning-of-buffer
  set-variable #l0 0
  !while &les #l0 3
    set-variable #l0 &add #l0 1
    !force search-buffer "me" "t\\(w\\|h\\)\\(o\\|r\\)"
    !if $status
      tlog &cat "s" @s1
      !if &xseq "t1" "t\\(\\d\\)"
        tlog @s1
      !endif
    !endif
  !done
!emacro

define-macro values
  set-variable #l0 "abcdefghij"
  set-variable #l1 0
  !while &les #l1 8
    set-variable #l1 &add #l1 1
    set-variable #l0 &cat #l0 #l0
  !done
  tlog &len #l0
  set-variable #l2 #l0
  tlog &len #l2
  tlog &len &cat #l0 "x"
  set-variable #l3 "a\sb\\c"
  tlog &len #l3
  tlog &seq #l3 &cat "a\sb" "\\c"
  insert-string #l3
!emacro

; a goto whose label is a variable, alternating, so a kept label line must be checked
define-macro vgoto
  set-variable #l0 0
  set-variable #l2 "la"
*top
  set-variable #l0 &add #l0 1
  !iif &gre #l0 6  !return
  !goto #l2
*la
  tlog &cat "a" #l0
  set-variable #l2 "lb"
  !goto top
*lb
  tlog &cat "b" #l0
  set-variable #l2 "la"
  !goto top
!emacro

; numeric arguments from functions and variables, !iif with a function test, !force !force,
; execute-line and @mx
define-macro args
  find-buffer "*args*"
  insert-string "abcdefghij"
  beginning-of-buffer
  &add 1 2 forward-char
  tlog $window-col
  set-variable #l0 2
  #l0 forward-char
  tlog $window-col
  !iif &not &equ #l0 2  tlog "bad"
  !iif &equ &add #l0 1 3  tlog "iif"
  !force !force 1000 forward-line
  tlog $status
  execute-line "3 backward-char"
  tlog $window-col
  !force 2 execute-line "forward-char 1 extra"
  tlog $status
  set-variable #l1 @mx "set-variable $result \"mx\""
  tlog #l1
  set-variable @cl "C-a"
  tlog @cl
!emacro

; define-help text in a macro body stays text
define-macro mhelp
  define-help "zzflowhelp" "3"
help text line one
  indented help line
!ehelp
  tlog "after"
!emacro

define-macro helps
  mhelp
  !force help-item "zzflowhelp"
  tlog $status
  !force find-buffer "*help*"
  beginning-of-buffer
  !force search-buffer "me" "indented"
  tlog $status
  tlog @wl
!emacro

; insert-macro of a macro defined with $debug set gives its lines
set-variable $debug 1
define-macro withsrc
  set-variable #l0 1
    ; a comment
  !if #l0
    insert-string "y"
  !endif
*lab
  !jump -1
!emacro
set-variable $debug 0
define-macro inserts
  find-buffer "*ins*"
  insert-macro "withsrc"
  beginning-of-buffer
  set-variable #l1 0
  !repeat
    tlog @wl
    !force forward-line
  !until &not $status
!emacro

define-macro start-up
  set-variable #l9 "|chain 0|chain 1|chain 2|chain 3|loops|jumps|recur 5|baddir|noend|defs|numbers|regex|values|ef|eb|vgoto|args|helps|inserts|"
  set-variable #l8 0
  !while &les #l8 &lle #l9
    set-variable #l8 &add #l8 1
    set-variable .t.log ""
    set-variable #l7 &lget #l9 #l8
    !if &seq #l7 "ef"
      !force execute-file "flowtop"
    !elif &seq #l7 "eb"
      find-buffer "*eb*"
      insert-string "!if 0\n  tlog \"bad\"\n!elif 1\n  !if 0\n    tlog \"bad\"\n  !else\n    tlog \"eb\"\n  !endif\n!endif\n"
      !force execute-buffer "*eb*"
    !else
      !force execute-line #l7
    !endif
    -2 ml-write &cat &cat "R:" #l7 &cat "=" &cat $status &cat ":" .t.log
  !done
  4 exit-emacs 0
!emacro
'''

TOP = r'''
!if 0
  tlog "bad1"
  define-macro skipped
    tlog "bad2"
  !emacro
!elif 0
  tlog "bad3"
!else
  tlog "top"
!endif
set-variable #l0 0
!while &les #l0 3
  set-variable #l0 &add #l0 1
  !if &equ #l0 2
    tlog "two"
  !else
    tlog "other"
  !endif
!done
!iif &not &exi skipped  tlog "noskipped"
'''


def run(mec, wdir):
    res = subprocess.run(['timeout', '-s', 'KILL', '60', mec, '-n', '-p', '@' + os.path.join(wdir, 'flow')],
                         stdin=subprocess.DEVNULL, capture_output=True, env=dict(os.environ, MEPATH=wdir))
    log = (res.stdout + res.stderr).decode('utf-8', 'replace').replace('\r', '\n')
    return dict(ll[ll.index('R:') + 2:].split('=', 1) for ll in log.split('\n') if 'R:' in ll and '=' in ll)


def main():
    if len(sys.argv) < 3:
        sys.exit('Usage: macroflow.py <ref-mec> <mec> [<mec>...]')
    wdir = tempfile.mkdtemp()
    open(os.path.join(wdir, 'flow.emf'), 'w').write(MACROS)
    open(os.path.join(wdir, 'flowtop.emf'), 'w').write(TOP)
    mecs = [os.path.abspath(mm) for mm in sys.argv[1:]]
    ref = run(mecs[0], wdir)
    passed = failed = 0
    if len(ref) < 19:
        print('FAIL reference run gave %d results' % len(ref))
        failed += 1
    for mec in mecs[1:]:
        got = run(mec, wdir)
        for kk in sorted(set(ref) | set(got)):
            if ref.get(kk) == got.get(kk):
                passed += 1
            else:
                failed += 1
                print('FAIL %s %s\n  ref: %r\n  got: %r' % (os.path.basename(mec), kk, ref.get(kk), got.get(kk)))
    shutil.rmtree(wdir, ignore_errors=True)
    print('%d passed, %d failed' % (passed, failed))
    sys.exit(1 if failed else 0)


if __name__ == '__main__':
    main()
