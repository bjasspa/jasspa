#!/usr/bin/env python3
#
# encoding.py - The translate-buffer replacements (X12) in mec in a pseudo terminal with the full
# macros: change-buffer-encoding re-decoding a buffer from how it was read and converting to a
# code page or UTF-8 (keeping, changing or removing chars the code page lacks), buffer-is-utf8,
# the Change Buffer Encoding dialog's defaults
# and ispell-convert building a cp1251 dictionary from UTF-8 and ISO-8859-5 hunspell files.
#
# Usage: python3 -I encoding.py <mec> [macros-dir]

import os
import shutil
import sys
import tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from screen import Mec, mecEnv  # noqa: E402

TDIR = os.path.dirname(os.path.abspath(__file__))
RU = 'Привет мир\nСъешь ещё\n'
U8 = 'é€ “quote” — ж\n'

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
        sys.exit('Usage: encoding.py <mec> [macros-dir]')
    mec = os.path.abspath(sys.argv[1])
    macros = os.path.abspath(sys.argv[2] if len(sys.argv) > 2 else os.path.join(TDIR, '..', '..', 'macros'))
    home = tempfile.mkdtemp()

    def path(name):
        return os.path.join(home, name)

    def write(name, data):
        open(path(name), 'wb').write(data)

    def read(name):
        return open(path(name), 'rb').read() if os.path.exists(path(name)) else None

    write('ru.txt', RU.encode('cp1251'))
    write('u8.txt', U8.encode('utf-8'))
    write('edge.txt', 'ééé\nxé'.encode('utf-8'))
    write('raw.txt', 'aé b\xff c\n'.encode('utf-8').replace(b'\xc3\xbf', b'\xff'))
    write('ascii.txt', b'plain\n')
    write('u16.txt', b'\xff\xfe' + 'abé\n'.encode('utf-16-le'))
    write('mostly.txt', 'é'.encode('utf-8') + b' \xe9\xe8\xe0 \xf9\n')

    mm = Mec(mec, [], env=mecEnv(home, macros))
    mm.keys(b'\x07', 2.0)

    def run(line, wait=2.0):
        mm.keys(b'\x1bxexecute-line\r', 1.0)
        mm.keys(line.encode('utf-8') + b'\r', wait)
        return ''.join(cc or '' for cc in mm.screen.cells[-1]).rstrip()

    def get(expr):
        line = run('ml-write &cat "R=" %s' % expr)
        return line[2:] if line.startswith('R=') else None

    def first():
        run('beginning-of-buffer')
        return get('&cat $buffer-encoding &cat "|" @wl')

    def close():
        run('!force 0 delete-buffer $buffer-bname')

    # a cp1251 file read with cp1252 is re-decoded, then converted to UTF-8
    run('code-page-define "cp1252"')
    run('find-file "%s"' % path('ru.txt'))
    check('read with the wrong page', first() == 'cp1252|' + RU.split('\n')[0].encode('cp1251').decode('cp1252'))
    msg = run('1 change-buffer-encoding "cp1251" "cp1251"', 4)
    check('re-decode message', 'converted 17 chars (13)' in msg, repr(msg))
    check('re-decoded', first() == 'cp1251|' + RU.split('\n')[0])
    run('!force 0 write-buffer "%s"' % path('ru.out'))
    check('re-decoded saves the same bytes', read('ru.out') == RU.encode('cp1251'), repr(read('ru.out')))
    run('change-buffer-encoding "cp1251" "utf8"', 4)
    check('converted to UTF-8', first() == 'utf8|' + RU.split('\n')[0])
    run('!force 0 write-buffer "%s"' % path('ru8.out'))
    check('UTF-8 save', read('ru8.out') == RU.encode('utf-8'), repr(read('ru8.out')))
    close()

    # chars the code page lacks: kept (no 4 bit), changed to \CG (4), removed (6), approximated (12)
    for flags, to, want in ((1, 'cp1251', U8.strip()), (3, 'cp1251', U8.strip()),
                            (5, 'cp1251', '\x07' + U8.strip()[1:]), (7, 'cp1251', U8.strip()[1:]),
                            (13, 'iso8859-1', 'é\x07 "quote" - \x07')):
        run('find-file "%s"' % path('u8.txt'))
        run('%d change-buffer-encoding "utf8" "%s"' % (flags, to), 4)
        run('beginning-of-buffer')
        got = get('&cat $buffer-encoding &cat "|" &bhex @wl')
        check('lacking chars %d %s' % (flags, to), got == to + '|' + want.encode('utf-8').hex(), repr(got))
        close()

    # UTF-8 text re-decoded as cp1252 bytes: repeated chars, no final newline, the same bytes saved
    run('find-file "%s"' % path('edge.txt'))
    run('change-buffer-encoding "cp1252" "cp1252"', 4)
    check('repeated chars', first() == 'cp1252|' + 'ééé'.encode('utf-8').decode('cp1252'))
    run('goto-line 2')
    check('last line', get('@wl') == 'xé'.encode('utf-8').decode('cp1252'), repr(get('@wl')))
    run('!force 0 write-buffer "%s"' % path('edge.out'))
    check('cp1252 save of the UTF-8 bytes', read('edge.out') == 'ééé\nxé'.encode('utf-8'), repr(read('edge.out')))
    run('change-buffer-encoding "utf8" "utf8"', 4)
    check('back to UTF-8', first() == 'utf8|ééé')
    close()

    # a UTF-16 buffer re-decoded to UTF-8 stays UTF-16
    run('find-file "%s"' % path('u16.txt'))
    run('change-buffer-encoding "cp1252" "utf8"', 4)
    text = 'ab' + 'é'.encode('utf-8').decode('cp1252')
    check('UTF-16 kept', first() == 'utf16le-bom|' + text, repr(first()))
    run('!force 0 write-buffer "%s"' % path('u16.out'))
    check('UTF-16 save', read('u16.out') == b'\xff\xfe' + (text + '\n').encode('utf-16-le'), repr(read('u16.out')))
    close()

    # an invalid byte is a byte of the code page
    run('find-file "%s"' % path('raw.txt'))
    run('change-buffer-encoding "cp1252" "cp1252"', 4)
    check('raw byte re-decoded', first() == 'cp1252|' + 'aé b'.encode('utf-8').decode('cp1252') + 'ÿ c')
    run('!force 0 write-buffer "%s"' % path('raw.out'))
    check('raw byte saved', read('raw.out') == read('raw.txt'), repr(read('raw.out')))
    close()

    # buffer-is-utf8
    for name, want in (('u8.txt', '1'), ('ascii.txt', '0'), ('ru.txt', '-1'), ('raw.txt', '-1')):
        run('find-file "%s"' % path(name))
        run('buffer-is-utf8 #l1')
        run('set-variable %isu #l1')
        check('buffer-is-utf8 %s' % name, get('%isu') == want, repr(get('%isu')))
        close()

    # a mostly invalid file is read as single byte
    run('find-file "%s"' % path('mostly.txt'))
    check('mostly invalid is single byte', get('$buffer-encoding') == 'cp1252')
    close()

    # the dialog's From and To default to how the buffer was read
    run('find-file "%s"' % path('ru.txt'))
    run('set-variable $buffer-encoding "cp1251"')
    mm.keys(b'\x1bxchange-buffer-encoding\r', 3.0)
    rows = [''.join(cc or '' for cc in mm.screen.cells[rr]) for rr in range(mm.screen.rows)]
    frm = [rr for rr in rows if 'From:' in rr]
    to = [rr for rr in rows if ' To: ' in rr]
    check('dialog From', frm and '1251' in frm[0], repr(frm))
    check('dialog To', to and '1251' in to[0], repr(to))
    mm.keys(b'\x07', 1.0)
    mm.close()

    # ispell-convert: the rules file is written in the dictionary's code page and the words
    # derive through the rules
    for enc, setname in (('utf-8', 'UTF-8'), ('iso8859_5', 'ISO8859-5')):
        hdir = path('h' + enc)
        os.mkdir(hdir)
        open(os.path.join(hdir, 'ru.aff'), 'wb').write(
            ('SET %s\n\nSFX A Y 2\nSFX A   о   а   о\nSFX A   0   ы   [^о]\n\nPFX B Y 1\nPFX B   0   не  .\n'
             % setname).encode(enc))
        open(os.path.join(hdir, 'ru.dic'), 'wb').write('3\nслово/A\nмир/AB\nдом\n'.encode(enc))
        mm = Mec(mec, [], env=mecEnv(home, macros + ':' + hdir))
        mm.keys(b'\x07', 2.0)
        run('execute-file "spellutl"', 3)
        line = run('1 ispell-convert "Russian" "%s"' % os.path.join(hdir, 'ru.aff'), 15)
        while 'Save dictionary' in line:
            mm.keys(b'y', 3.0)
            line = ''.join(cc or '' for cc in mm.screen.cells[-1]).rstrip()
        check('ispell-convert %s' % enc, 'All Done' in line, repr(line))
        lsr = open(os.path.join(hdir, 'lsrruru.emf'), 'rb').read() if os.path.exists(
            os.path.join(hdir, 'lsrruru.emf')) else b''
        check('ispell-convert %s rules in cp1251' % enc,
              '2 add-spell-rule "A" "о" "о" "а" ""'.encode('cp1251') in lsr, repr(lsr[-120:]))
        run('set-registry "/history/language" "Russian"')
        run('execute-file "language"')
        run('!force spell-rules-init', 5)
        for word, ok in (('слово', 1), ('слова', 1), ('миры', 1), ('немир', 1), ('дом', 1), ('дома', 0)):
            run('!force 1 spell "%s"' % word)
            check('ispell-convert %s %s' % (enc, word), get('$result') == ('O' if ok else 'E') + word,
                  repr(get('$result')))
        mm.close()
    shutil.rmtree(home, ignore_errors=True)
    print('%d passed, %d failed' % (passed, failed))
    sys.exit(1 if failed else 0)


if __name__ == '__main__':
    main()
