#!/usr/bin/env python3
#
# fnames.py - Non-ASCII file names on Unix (acceptance test F14, winconsole.py covers Windows):
# files named in Latin, CJK, Greek and emoji, an NFD name next to its NFC twin (different files,
# never normalised) and a name which is not valid UTF-8 (a stray \xff byte, kept as a raw byte):
#   - headless: find-file opens each with its own content, the buffer name is the file's,
#     write-file creates a non-ASCII name, and the directory listing holds every name;
#   - in a pseudo terminal: a typed name opens, Tab completes a non-ASCII and the invalid name,
#     and saving the invalid one writes back to that same file.
#
# Usage: python3 -I fnames.py <mec> [macros-dir]

import os
import shutil
import subprocess
import sys
import tempfile
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from screen import Mec, mecEnv  # noqa: E402

TDIR = os.path.dirname(os.path.abspath(__file__))
CX = b'\x18'

NAMES = {
    'café.txt': 'nfc content',
    'café.txt': 'nfd content',
    '中文.txt': 'cjk content',
    'Ελληνικά.txt': 'greek content',
    '\U0001f600.txt': 'emoji content',
}
BAD = b'bad\xff.txt'

passed = failed = 0


def check(name, cond, detail=''):
    global passed, failed
    if cond:
        passed += 1
    else:
        failed += 1
        print('FAIL %s %s' % (name, detail))


def mstr(ss):
    return ss.replace('\\', '\\\\').replace('"', '\\"')


def headless(mec, fdir, wdir):
    out = os.path.join(wdir, 'out')
    os.mkdir(out)
    mac = ['define-macro start-up']
    for ii, (nn, cc) in enumerate(NAMES.items()):
        mac += ['  !force find-file "%s"' % mstr(os.path.join(fdir, nn)),
                '  -2 ml-write &cat "R:text%d=" @wl' % ii,
                '  -2 ml-write &cat "R:bname%d=" $buffer-bname' % ii]
    newn = 'новый.txt'
    mac += ['  find-buffer "*n*"', '  insert-string "new content"',
            '  !force 0 write-buffer "%s"' % mstr(os.path.join(fdir, newn)),
            '  !force find-file "%s/"' % mstr(fdir),
            '  !force 0 write-buffer "%s"' % mstr(os.path.join(out, 'listing')),
            '  4 exit-emacs 0', '!emacro']
    mpath = os.path.join(wdir, 'fn.emf')
    open(mpath, 'w', encoding='utf-8').write('\n'.join(mac) + '\n')
    res = subprocess.run(['timeout', '-s', 'KILL', '60', mec, '-p', '@' + mpath[:-4]], stdin=subprocess.DEVNULL,
                         capture_output=True, env=dict(os.environ, MEPATH=wdir))
    log = (res.stdout + res.stderr).decode('utf-8', 'replace').replace('\r', '\n')
    got = dict(ll[2:].split('=', 1) for ll in log.split('\n') if ll.startswith('R:') and '=' in ll)
    for ii, (nn, cc) in enumerate(NAMES.items()):
        check('find-file %s' % ascii(nn), got.get('text%d' % ii) == cc, ascii(got.get('text%d' % ii)))
        check('buffer name %s' % ascii(nn), got.get('bname%d' % ii) == nn, ascii(got.get('bname%d' % ii)))
    path = os.path.join(fdir, newn)
    check('write-file', os.path.exists(path) and open(path, 'rb').read() == b'new content\n',
          ascii(os.listdir(fdir)))
    lpath = os.path.join(out, 'listing')
    listing = open(lpath, 'rb').read() if os.path.exists(lpath) else b''
    for nn in list(NAMES) + [newn]:
        check('listing %s' % ascii(nn), nn.encode('utf-8') in listing, ascii(listing[:200]))
    check('listing raw name', BAD in listing, ascii(listing[:200]))


def keys(mm, *kk):
    for key in kk:
        mm.keys(key)


def pty(mec, macros, fdir, wdir):
    home = os.path.join(wdir, 'home')
    os.mkdir(home)
    mm = Mec(mec, [os.path.join(fdir, 'café.txt')], env=mecEnv(home, macros))
    try:
        rows = mm.screen.rows
        check('pty file arg', mm.screen.text(0).startswith('nfc content') or mm.screen.text(1).startswith('nfc content'),
              ascii(mm.screen.text(0)))
        # a typed name
        keys(mm, CX + b'\x06', os.path.join(fdir, '中文.txt').encode('utf-8') + b'\r')
        check('pty typed name', any(mm.screen.text(rr).startswith('cjk content') for rr in range(3)),
              ascii(mm.screen.text(1)))
        check('pty mode line', any('中文.txt' in mm.screen.text(rr) for rr in range(rows)), '')
        # completion of a non-ASCII name: Greek is the only name starting with E-with-a-tonos-less capital
        keys(mm, CX + b'\x06', os.path.join(fdir, 'Ε').encode('utf-8'), b'\t')
        ml = mm.screen.text(rows - 1).rstrip()
        check('pty complete', ml.endswith('Ελληνικά.txt'), ascii(ml))
        keys(mm, b'\r')
        check('pty complete open', any(mm.screen.text(rr).startswith('greek content') for rr in range(3)),
              ascii(mm.screen.text(1)))
        # the invalid name completes, opens and saves back to the same file
        keys(mm, CX + b'\x06', os.path.join(fdir, 'bad').encode('utf-8'), b'\t', b'\r')
        check('pty raw name open', any(mm.screen.text(rr).startswith('bad content') for rr in range(3)),
              ascii(mm.screen.text(1)) + ' ' + ascii(mm.screen.text(rows - 1)))
        keys(mm, b'X', CX + b'\x13')
        bad = os.path.join(fdir.encode(), BAD)
        check('pty raw name save', open(bad, 'rb').read() == b'Xbad content\n', ascii(open(bad, 'rb').read()))
        # every name is a test file or a backup of one, the raw byte never became U+00FF's UTF-8
        known = [nn.encode('utf-8') for nn in NAMES] + [BAD, '\u043d\u043e\u0432\u044b\u0439.txt'.encode('utf-8')]
        names = os.listdir(fdir.encode())
        check('pty names kept', all(any(nn == kk or nn.startswith(kk + b'.~') or nn == kk + b'~' for kk in known)
                                    for nn in names) and not any(b'\xc3\xbf' in nn for nn in names), ascii(names))
    finally:
        mm.close()


def main():
    if len(sys.argv) < 2:
        sys.exit('Usage: fnames.py <mec> [macros-dir]')
    mec = os.path.abspath(sys.argv[1])
    macros = os.path.abspath(sys.argv[2] if len(sys.argv) > 2 else os.path.join(TDIR, '..', '..', 'macros'))
    wdir = tempfile.mkdtemp()
    fdir = os.path.join(wdir, 'files')
    os.mkdir(fdir)
    for nn, cc in NAMES.items():
        open(os.path.join(fdir, nn), 'wb').write(cc.encode('utf-8') + b'\n')
    open(os.path.join(fdir.encode(), BAD), 'wb').write(b'bad content\n')
    headless(mec, fdir, wdir)
    pty(mec, macros, fdir, wdir)
    shutil.rmtree(wdir, ignore_errors=True)
    print('%d passed, %d failed' % (passed, failed))
    sys.exit(1 if failed else 0)


if __name__ == '__main__':
    main()
