#!/usr/bin/env python3
#
# bintools.py - The bintools.emf conversions on the byte functions, headlessly: bin-txt-to-hex,
# bin-hex-to-txt and ibin-copy-region (a hex yank inserted as text) with a 0 byte, UTF-8 runs
# longer than a conversion chunk and a long run of bytes that are not UTF-8. An 8-bit build
# (src.old or MEOPT_UNICODE 0) cannot insert a 0 byte, it is reported as lost.
#
# Usage: python3 -I bintools.py <mec> [macros-dir]

import os
import subprocess
import sys
import tempfile

TDIR = os.path.dirname(os.path.abspath(__file__))
CASES = (('small', '41c3a90a42'), ('nul', '4100420a'), ('utf8long', 'c3a9' * 300), ('raw', '80' * 600),
         ('mix', '41' * 399 + 'e282ac' * 3))

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
        sys.exit('Usage: bintools.py <mec> [macros-dir]')
    mec = os.path.abspath(sys.argv[1])
    macros = os.path.abspath(sys.argv[2] if len(sys.argv) > 2 else os.path.join(TDIR, '..', '..', 'macros'))
    wdir = tempfile.mkdtemp()
    mac = ['define-macro start-up', '  execute-file "bintools"',
           '  -2 ml-write &cat "R:uni=" &opt "uni"',
           '  bin-txt-to-hex "a\\xe9\\x1c"', '  -2 ml-write &cat "R:t2h=" $result',
           '  bin-hex-to-txt "41E9"', '  -2 ml-write &cat "R:h2t=" &bhex $result']
    for name, hx in CASES:
        mac += ['  !force 0 delete-buffer "*h*"', '  find-buffer "*h*"', '  insert-string "%s"' % hx,
                '  beginning-of-buffer', '  set-mark', '  end-of-buffer', '  copy-region',
                '  ibin-copy-region 0', '  -2 ml-write &cat "R:%s=" $result' % name,
                '  !force 0 delete-buffer "*o*"', '  find-buffer "*o*"', '  yank', '  -1 buffer-mode "cr"',
                '  1 buffer-mode "lf"', '  !force 0 write-buffer "%s"' % os.path.join(wdir, name + '.out')]
    mac += ['  4 exit-emacs 0', '!emacro']
    open(os.path.join(wdir, 'bt.emf'), 'w').write('\n'.join(mac) + '\n')
    res = subprocess.run([mec, '-p', '@' + os.path.join(wdir, 'bt')], stdin=subprocess.DEVNULL,
                         capture_output=True, env=dict(os.environ, MEPATH=wdir + ':' + macros, HOME=wdir), timeout=120)
    log = (res.stdout + res.stderr).decode('latin-1').replace('\r', '\n')
    got = dict(ll[2:].split('=', 1) for ll in log.split('\n') if ll.startswith('R:') and '=' in ll)
    uni = got.get('uni') == '1'
    check('bin-txt-to-hex', got.get('t2h') == ('61C3A91C' if uni else '61E91C'), repr(got.get('t2h')))
    check('bin-hex-to-txt', got.get('h2t') == '41e9', repr(got.get('h2t')))
    for name, hx in CASES:
        data = bytes.fromhex(hx)
        lost = 0
        if not uni:
            lost = data.count(0)
            data = data.replace(b'\0', b'')
        path = os.path.join(wdir, name + '.out')
        out = open(path, 'rb').read() if os.path.exists(path) else None
        check('ibin-copy-region %s' % name, out is not None and out.rstrip(b'\n') == data.rstrip(b'\n'),
              repr(out[:40] if out else out))
        check('ibin-copy-region %s lost' % name, got.get(name) == str(lost), repr(got.get(name)))
    print('%d passed, %d failed' % (passed, failed))
    sys.exit(1 if failed else 0)


if __name__ == '__main__':
    main()
