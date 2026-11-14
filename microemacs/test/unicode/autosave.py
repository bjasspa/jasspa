#!/usr/bin/env python3
#
# autosave.py - Auto-save, recovery and backup files keep the encoding (acceptance test F12):
# loads files in every encoding and with mixed line endings, no final newline, NULs and invalid
# bytes, inserts a char, then
#   - exits with 12 exit-emacs, which auto-saves changed buffers: each file# must hold the edited
#     text in the file's encoding (BOM, endings and odd bytes kept);
#   - opens each file again in a pseudo terminal, answers y to "Recover newer file#" and saves:
#     the recovered buffer must have the file's encoding and save the same bytes as the auto-save;
#   - saves each edited file with backup mode: the ~ file must be the original bytes and the file
#     the edited text.
#
# Usage: python3 -I autosave.py <mec> [macros-dir]   (a Unicode build, an 8-bit one has no UTF-16/32 or NULs)

import os
import shutil
import subprocess
import sys
import tempfile
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from convert import LATIN1_TABLE  # noqa: E402
from screen import Mec, mecEnv  # noqa: E402

TEXT = 'Héllo wörld\nsecond line 中文\nthird line\n' * 3

# name: (file bytes, $buffer-encoding it loads as, bytes after a 'Z' is inserted at the start)
def cases():
    out = {}
    def add(name, enc, data, bom=b''):
        out[name] = (bom + data(TEXT), enc, bom + data('Z' + TEXT))
    add('utf8.txt', 'utf8', lambda tt: tt.encode('utf-8'))
    add('utf8bom.txt', 'utf8-bom', lambda tt: tt.encode('utf-8'), b'\xef\xbb\xbf')
    add('utf16le.txt', 'utf16le-bom', lambda tt: tt.encode('utf-16-le'), b'\xff\xfe')
    add('utf16be.txt', 'utf16be-bom', lambda tt: tt.encode('utf-16-be'), b'\xfe\xff')
    add('utf16nobom.txt', 'utf16le', lambda tt: tt.encode('utf-16-le'))
    add('utf32le.txt', 'utf32le-bom', lambda tt: tt.encode('utf-32-le'), b'\xff\xfe\x00\x00')
    latin = 'Héllo wörld\nsecond line\nthird line\n' * 3
    out['latin1.txt'] = (latin.encode('latin-1'), 'C', ('Z' + latin).encode('latin-1'))
    out['mixed.txt'] = (b'a1\r\nb2\nc3\rd4\r\n', 'utf8', b'Za1\r\nb2\nc3\rd4\r\n')
    out['noeol.txt'] = (b'one\ntwo', 'utf8', b'Zone\ntwo')
    out['nul_ascii.txt'] = (b'a\x00b\n\x00\n', 'utf8', b'Za\x00b\n\x00\n')
    # a stray byte means a code page file, its bytes kept either way
    out['invalid.txt'] = (b'caf\xc3\xa9 caf\xe9\n', 'C', b'Zcaf\xc3\xa9 caf\xe9\n')
    return out


def autoName(fdir, name):
    """the auto-save file name, name# on Unix, #name# elsewhere"""
    for ff in (name + '#', '#' + name + '#'):
        if os.path.exists(os.path.join(fdir, ff)):
            return os.path.join(fdir, ff)
    return os.path.join(fdir, name + '#')


def runHeadless(mec, wdir, lines):
    path = os.path.join(wdir, 'as.emf')
    open(path, 'w', encoding='utf-8').write('\n'.join(['define-macro start-up'] + ['  ' + ll for ll in lines] +
                                                      ['!emacro']) + '\n')
    res = subprocess.run(['timeout', '-s', 'KILL', '120', mec, '-p', '@' + path[:-4]], stdin=subprocess.DEVNULL,
                         capture_output=True, env=dict(os.environ, MEPATH=wdir))
    return (res.stdout + res.stderr).decode('utf-8', 'replace').replace('\r', '\n')


def main():
    if len(sys.argv) < 2:
        sys.exit('Usage: autosave.py <mec> [macros-dir]')
    mec = os.path.abspath(sys.argv[1])
    macros = os.path.abspath(sys.argv[2] if len(sys.argv) > 2 else os.path.join(TDIR, '..', '..', 'macros'))
    wdir = tempfile.mkdtemp()
    fdir = os.path.join(wdir, 'files')
    os.mkdir(fdir)
    passed = failed = 0

    def check(name, cond, detail=''):
        nonlocal passed, failed
        if cond:
            passed += 1
        else:
            failed += 1
            print('FAIL %s %s' % (name, detail))

    cs = cases()
    old = time.time() - 3600
    for name, (data, enc, edited) in cs.items():
        path = os.path.join(fdir, name)
        open(path, 'wb').write(data)
        os.utime(path, (old, old))

    # 1 - the auto-saves written on exit
    lines = ['set-char-mask "c" "%s"' % LATIN1_TABLE]
    for name, (data, enc, edited) in cs.items():
        lines += ['find-file "%s"' % os.path.join(fdir, name), '-2 ml-write &cat "enc:%s=" $buffer-encoding' % name,
                  'beginning-of-buffer', 'insert-string "Z"']
    lines += ['12 exit-emacs 0']
    log = runHeadless(mec, wdir, lines)
    for name, (data, enc, edited) in cs.items():
        check('load %s' % name, ('enc:%s=%s' % (name, enc)) in log, 'loaded as another encoding')
        asv = autoName(fdir, name)
        got = open(asv, 'rb').read() if os.path.exists(asv) else None
        check('auto-save %s' % name, got == edited, repr(got))

    # 2 - recovery through the prompt, in a pseudo terminal with the user's set up
    for name, (data, enc, edited) in cs.items():
        asv = autoName(fdir, name)
        if not os.path.exists(asv):
            check('recover %s' % name, False, 'no auto-save')
            continue
        out = os.path.join(wdir, 'rec-' + name)
        mpath = os.path.join(wdir, 'rec.emf')
        open(mpath, 'w', encoding='utf-8').write('\n'.join([
            'define-macro start-up',
            '  set-char-mask "c" "%s"' % LATIN1_TABLE,
            '  find-file "%s"' % os.path.join(fdir, name),
            '  set-variable #l0 $buffer-encoding',
            '  !force 0 write-buffer "%s"' % out,
            '  find-buffer "*r*"',
            '  insert-string #l0',
            '  !force 0 write-buffer "%s.enc"' % out,
            '  4 exit-emacs 0',
            '!emacro']) + '\n')
        mm = Mec(mec, ['@' + mpath[:-4]], env=mecEnv(wdir, macros))
        if b'Recover' in mm.raw:
            mm.keys(b'y')
        end = time.time() + 30
        while (time.time() < end) and not os.waitpid(mm.pid, os.WNOHANG)[0]:
            mm.settle(0.5)
        try:
            os.close(mm.fd)
        except OSError:
            pass
        got = open(out, 'rb').read() if os.path.exists(out) else None
        genc = open(out + '.enc').read().strip() if os.path.exists(out + '.enc') else None
        check('recover %s' % name, got == edited, repr(got))
        check('recover encoding %s' % name, genc == enc, 'got %r expected %s' % (genc, enc))

    # 3 - backup mode keeps the original bytes in the ~ file
    bdir = os.path.join(wdir, 'backup')
    os.mkdir(bdir)
    lines = ['set-char-mask "c" "%s"' % LATIN1_TABLE]
    for name, (data, enc, edited) in cs.items():
        path = os.path.join(bdir, name)
        open(path, 'wb').write(data)
        lines += ['find-file "%s"' % path, '1 buffer-mode "backup"', 'beginning-of-buffer', 'insert-string "Z"',
                  '!force 0 save-buffer', '!force 0 delete-buffer $buffer-bname']
    lines += ['4 exit-emacs 0']
    runHeadless(mec, wdir, lines)
    for name, (data, enc, edited) in cs.items():
        path = os.path.join(bdir, name)
        bak = path + '~'
        check('backup %s' % name, os.path.exists(bak) and open(bak, 'rb').read() == data,
              repr(open(bak, 'rb').read() if os.path.exists(bak) else None))
        check('backup save %s' % name, open(path, 'rb').read() == edited, repr(open(path, 'rb').read()))

    shutil.rmtree(wdir, ignore_errors=True)
    print('%d passed, %d failed' % (passed, failed))
    sys.exit(1 if failed else 0)


TDIR = os.path.dirname(os.path.abspath(__file__))

if __name__ == '__main__':
    main()
