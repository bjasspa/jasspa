#!/usr/bin/env python3
#
# keyboard.py - The non-Latin keyboard map (set-char-mask "k", keyboard.emf): each key is a 4 hex
# digit code point and the Latin char on the same key, in every build.
#   - headless: keyboard.emf's Russian and Greek maps are read back by 0 set-char-mask "k", a map
#     with a bad length, a non-hex code point or an ASCII key is rejected, "" removes it, and an
#     unbound A-<Cyrillic key> runs the binding of A-<its Latin key>;
#   - in a pseudo terminal (Unicode build): the Cyrillic key on Y answers a (?/y/n) prompt.
# An 8-bit build is run with the cp1251 code page, which holds the Cyrillic keys (Greek's are not
# in it, so are skipped).
#
# Usage: python3 -I keyboard.py <mec> [macros-dir]

import os
import shutil
import subprocess
import sys
import tempfile
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from screen import Mec, mecEnv  # noqa: E402

TDIR = os.path.dirname(os.path.abspath(__file__))
RUSSIAN = dict(zip('abcdefghijklmnopqrstuvwxyz', 'фисвуапршолдьтщзйкыегмцчня'))
GREEK = dict(zip('abcdefghijklmnoprstuvwxyz', 'αβψδεφγηιξκλμνοπρστθωςχυζ'))

passed = failed = 0


def check(name, cond, detail=''):
    global passed, failed
    if cond:
        passed += 1
    else:
        failed += 1
        print('FAIL %s %s' % (name, detail))


def mapOf(layout):
    """the expected map: each key and its capital (when it has one no other key gives)"""
    out = {}
    for lat, ch in layout.items():
        out[ch] = lat
        up = ch.upper()
        if (up != ch) and (len(up) == 1) and (up not in out):
            out[up] = lat.upper()
    return out


def parseMap(ss):
    if (ss is None) or (len(ss) % 5):
        return None
    return {chr(int(ss[ii:ii + 4], 16)): ss[ii + 4] for ii in range(0, len(ss), 5)}


def headless(mec, macros, unicode):
    wdir = tempfile.mkdtemp()
    mac = ['define-macro start-up']
    if not unicode:
        mac += ['  execute-file "codepage"', '  code-page-define "cp1251"', '  set-char-mask "C" "cp1251"']
    for kb in ('Russian', 'Greek'):
        mac += ['  set-registry "/history/keyboard" "%s"' % kb, '  execute-file "keyboard"',
                '  0 set-char-mask "k"', '  -2 ml-write &cat "R:%s=" $result' % kb]
    # bad maps are rejected and leave no map, "" removes it
    for tag, bad in (('len', '044'), ('hex', '04x4a'), ('ascii', '0061b'), ('ctrl', '0444\\x01')):
        mac += ['  !force set-char-mask "k" "%s"' % bad, '  -2 ml-write &cat "R:bad%s=" $status' % tag]
    mac += ['  set-char-mask "k" "0444a"', '  set-char-mask "k" ""',
            '  0 set-char-mask "k"', '  -2 ml-write &cat "R:cleared=[" &cat $result "]"']
    # a key given twice keeps its last
    mac += ['  set-char-mask "k" "0444a0444b"', '  0 set-char-mask "k"', '  -2 ml-write &cat "R:twice=" $result']
    # an unbound A-<Cyrillic f key> runs A-a's binding
    fkey = 'ф' if unicode else '\\xf4'
    mac += ['  set-registry "/history/keyboard" "Russian"', '  execute-file "keyboard"',
            '  find-buffer "*t*"', '  insert-string "abcdef"', '  beginning-of-line',
            '  global-bind-key "end-of-line" "A-a"', '  execute-string "\\s\\x04%s"' % fkey,
            '  -2 ml-write &cat "R:binding=" $window-col', '  4 exit-emacs 0', '!emacro']
    mpath = os.path.join(wdir, 'kb.emf')
    open(mpath, 'w', encoding='utf-8').write('\n'.join(mac) + '\n')
    res = subprocess.run(['timeout', '-s', 'KILL', '60', mec, '-p', '@' + mpath[:-4]], stdin=subprocess.DEVNULL,
                         capture_output=True, env=dict(os.environ, MEPATH=wdir + ':' + macros, HOME=wdir))
    log = (res.stdout + res.stderr).decode('utf-8', 'replace').replace('\r', '\n')
    got = dict(ll[ll.index('R:') + 2:].split('=', 1) for ll in log.split('\n') if 'R:' in ll and '=' in ll)
    shutil.rmtree(wdir, ignore_errors=True)
    check('russian map', parseMap(got.get('Russian')) == mapOf(RUSSIAN), repr(got.get('Russian')))
    # an 8-bit build's code page (cp1251) has no Greek, so its map is empty
    want = mapOf(GREEK) if unicode else {}
    check('greek map', parseMap(got.get('Greek')) == want, repr(got.get('Greek')))
    for tag in ('len', 'hex', 'ascii', 'ctrl'):
        check('bad map %s' % tag, got.get('bad' + tag) == '0', repr(got.get('bad' + tag)))
    check('map removed', got.get('cleared') == '[]', repr(got.get('cleared')))
    check('key twice', got.get('twice') == '0444b', repr(got.get('twice')))
    check('binding fallback', got.get('binding') == '6', repr(got.get('binding')))


def prompt(mec, macros):
    """the Cyrillic key on Y answers the exit's "Save file (?/y/n/a/o/g)" prompt with y"""
    home = tempfile.mkdtemp()
    fname = os.path.join(home, 'in.txt')
    open(fname, 'w').write('x\n')
    env = mecEnv(home, macros)
    mm = Mec(mec, [fname], env=env)
    try:
        mm.keys(b'\x1bxexecute-line\rset-registry "/history/keyboard" "Russian"\r')
        mm.keys(b'\x1bxexecute-file\rkeyboard\r')
        mm.keys(b'Z')
        mm.keys(b'\x18\x03')
        asked = '(?/y/n' in mm.screen.text(mm.screen.rows - 1)
        mm.keys('н'.encode('utf-8'), 2.0)
        end = time.time() + 5
        exited = False
        while time.time() < end:
            if os.waitpid(mm.pid, os.WNOHANG)[0]:
                exited = True
                break
            mm.settle(0.3)
        check('prompt asked', asked, repr(mm.screen.text(mm.screen.rows - 1)))
        check('prompt answered by keyboard map', exited and (open(fname, 'rb').read() == b'Zx\n'),
              repr(mm.screen.text(mm.screen.rows - 1)) + ' ' + repr(open(fname, 'rb').read()))
    finally:
        mm.close()
        shutil.rmtree(home, ignore_errors=True)


def main():
    if len(sys.argv) < 2:
        sys.exit('Usage: keyboard.py <mec> [macros-dir]')
    mec = os.path.abspath(sys.argv[1])
    macros = os.path.abspath(sys.argv[2] if len(sys.argv) > 2 else os.path.join(TDIR, '..', '..', 'macros'))
    # a Unicode build has the uni option
    probe = tempfile.mkdtemp()
    open(os.path.join(probe, 'u.emf'), 'w').write('define-macro start-up\n  -2 ml-write &cat "R:uni=" &opt "uni"\n'
                                                  '  4 exit-emacs 0\n!emacro\n')
    res = subprocess.run([mec, '-p', '@' + os.path.join(probe, 'u')], stdin=subprocess.DEVNULL, capture_output=True,
                         timeout=30, env=dict(os.environ, MEPATH=probe))
    shutil.rmtree(probe, ignore_errors=True)
    unicode = b'R:uni=1' in res.stdout + res.stderr
    headless(mec, macros, unicode)
    if unicode:
        prompt(mec, macros)
    print('%d passed, %d failed' % (passed, failed))
    sys.exit(1 if failed else 0)


if __name__ == '__main__':
    main()
