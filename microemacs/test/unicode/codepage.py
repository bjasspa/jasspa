#!/usr/bin/env python3
#
# codepage.py - Named code pages: set-char-mask "C"/"S", code-page-define, $buffer-encoding naming
# a code page, a legacy file keeping the code page it was loaded with, converting between code
# pages, a forced code page kept by read-file, inserting into a legacy buffer and changing a code
# page's table in place, headlessly. An 8-bit build (src.old or MEOPT_UNICODE 0) only has the code page
# commands checked.
#
# Usage: python3 -I codepage.py <mec> [macros-dir]

import os
import subprocess
import sys
import tempfile

TDIR = os.path.dirname(os.path.abspath(__file__))
TEXT = 'Привет, мир!\nСъешь же ещё этих мягких французских булок.\n'
INS = 'Вставка\n'

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
        sys.exit('Usage: codepage.py <mec> [macros-dir]')
    mec = os.path.abspath(sys.argv[1])
    macros = os.path.abspath(sys.argv[2] if len(sys.argv) > 2 else os.path.join(TDIR, '..', '..', 'macros'))
    wdir = tempfile.mkdtemp()
    src = os.path.join(wdir, 'ru.txt')
    open(src, 'wb').write(TEXT.encode('iso8859_5'))
    open(os.path.join(wdir, 'ins.txt'), 'wb').write(INS.encode('iso8859_5'))

    def out(name):
        return os.path.join(wdir, name)

    mac = ['define-macro start-up']

    def report(tag, expr):
        mac.append('  -2 ml-write &cat "R:%s=" %s' % (tag, expr))

    def line(tag):
        mac.append('  beginning-of-buffer')
        report(tag, '&cat &cat $buffer-encoding "|" @wl')

    report('uni', '&opt "uni"')
    mac.append('  0 set-char-mask "C"')
    report('list0', '$result')
    mac.append('  0 set-char-mask "S"')
    report('spell0', '$result')
    # defines code-page-define and the display code page, iso8859-1 when not set by user-setup
    mac.append('  execute-file "codepage"')
    mac += ['  code-page-define "iso8859-5"', '  0 code-page-define "cp1252"', '  0 set-char-mask "C"']
    report('list1', '$result')
    # -1 only tests a code page exists
    mac += ['  !force -1 set-char-mask "C" "cp1252"']
    report('test1', '$status')
    mac += ['  !force -1 set-char-mask "C" "nosuch"']
    report('test0', '$status')
    mac += ['  !force -1 set-char-mask "C" "nosuch"']
    report('test2', '$status')
    mac += ['  0 set-char-mask "C"']
    report('test3', '$result')
    # utf* names are $buffer-encoding's UTF forms, not code pages
    mac += ['  !force set-char-mask "C" "utf8"']
    report('utf0', '$status')
    mac += ['  !force set-char-mask "C" "UTF-16"']
    report('utf1', '$status')
    mac += ['  !force -1 set-char-mask "C" "utf8"']
    report('utf2', '$status')
    mac += ['  code-page-define "cp1252"', '  0 set-char-mask "C"']
    report('select', '$result')
    mac += ['  0 code-page-define "iso8859-5"', '  0 set-char-mask "C"']
    report('select0', '$result')
    mac += ['  set-char-mask "C" "iso8859-5"']
    mac += ['  set-char-mask "S" "cp1252"', '  0 set-char-mask "S"']
    report('spell1', '$result')
    mac.append('  !force set-char-mask "S" "nosuch"')
    report('spellbad', '$status')
    mac += ['  set-char-mask "S" ""', '  0 set-char-mask "S"']
    report('spell2', '$result')

    mac.append('  find-file "%s"' % src)
    line('load')
    # buffer-info gives a single byte file's byte first
    mac.append('  1 buffer-info')
    report('info', '$result')
    mac.append('  0 buffer-info')
    report('bytes', '&lget $result 6')
    # the current code page changes, the buffer keeps its own
    mac += ['  code-page-define "cp1252"', '  !force 0 write-buffer "%s"' % out('keep.txt')]
    mac += ['  end-of-buffer', '  insert-file "%s"' % os.path.join(wdir, 'ins.txt')]
    mac += ['  end-of-buffer', '  backward-line']
    report('insert', '&cat &cat $buffer-encoding "|" @wl')
    mac += ['  0 code-page-define "cp1251"', '  set-variable $buffer-encoding "cp1251"',
            '  !force 0 write-buffer "%s"' % out('cp1251.txt')]
    mac += ['  !force set-variable $buffer-encoding "utf8"']
    report('utf8set', '$status')
    report('utf8enc', '$buffer-encoding')
    mac += ['  !force 0 write-buffer "%s"' % out('utf8.txt')]
    mac.append('  !force set-variable $buffer-encoding "nosuch"')
    report('bad', '$status')
    mac += ['  !force 0 delete-buffer $buffer-bname']

    # loaded with the wrong code page, forced to the right one and re-read
    mac.append('  find-file "%s"' % src)
    line('wrong')
    mac += ['  set-variable $buffer-encoding "iso8859-5"', '  -1 buffer-mode "edit"',
            '  1 read-file $buffer-fname']
    line('reread')

    # changing a code page's table translates a buffer using it on save
    mac += ['  set-char-mask "C" "iso8859-5"',
            '  set-char-mask "c" "%s"' % ''.join('%04x' % ord(bytes([bb]).decode('koi8_r'))
                                                for bb in range(0x80, 0x100)),
            '  !force 0 write-buffer "%s"' % out('koi8.txt')]
    line('retable')
    mac += ['  end-of-buffer', '  insert-string "\\u20AC"', '  backward-char', '  1 buffer-info']
    report('infonot', '$result')
    mac.append('  -1 buffer-mode "edit"')
    mac += ['  4 exit-emacs 0', '!emacro']
    open(os.path.join(wdir, 'cp.emf'), 'w', encoding='utf-8').write('\n'.join(mac) + '\n')

    res = subprocess.run([mec, '-p', '@' + os.path.join(wdir, 'cp')], stdin=subprocess.DEVNULL,
                         capture_output=True, env=dict(os.environ, MEPATH=wdir + ':' + macros), timeout=60)
    log = (res.stdout + res.stderr).decode('utf-8', 'backslashreplace').replace('\r', '\n')
    got = dict(ll[2:].split('=', 1) for ll in log.split('\n') if ll.startswith('R:') and '=' in ll)

    def file(name):
        return open(out(name), 'rb').read() if os.path.exists(out(name)) else None

    first = TEXT.split('\n')[0]
    fb = first[0].encode('iso8859_5')[0]
    check('the default page', got.get('list0') == 'C', repr(got.get('list0')))
    check('no spell page', got.get('spell0') == '', repr(got.get('spell0')))
    check('define and select, define only', got.get('list1') == 'iso8859-5', repr(got.get('list1')))
    check('-1 tests a defined page', got.get('test1') == '1', repr(got.get('test1')))
    check('-1 tests an undefined page', got.get('test0') == '0', repr(got.get('test0')))
    check('-1 does not create', got.get('test2') == '0', repr(got.get('test2')))
    check('-1 does not select', got.get('test3') == 'iso8859-5', repr(got.get('test3')))
    check('no utf code page', got.get('utf0') == '0' and got.get('utf1') == '0' and got.get('utf2') == '0',
          repr((got.get('utf0'), got.get('utf1'), got.get('utf2'))))
    check('selecting a defined page', got.get('select') == 'cp1252', repr(got.get('select')))
    check('0 leaves the current page', got.get('select0') == 'cp1252', repr(got.get('select0')))
    check('unknown spell page fails', got.get('spellbad') == '0', repr(got.get('spellbad')))
    if got.get('uni') != '1':
        # an 8-bit build records the current page, S is only checked
        check('8-bit records the page', got.get('load', '').startswith('iso8859-5|'), repr(got.get('load')))
        check('8-bit buffer page', got.get('bad') == '0', repr(got.get('bad')))
        check('8-bit has no UTF-8', got.get('utf8set') == '0', repr(got.get('utf8set')))
        check('8-bit file byte count', got.get('bytes') == str(len(TEXT)), repr(got.get('bytes')))
        # with its Unicode value through the buffer's page
        check('8-bit buffer-info byte', got.get('info', '').endswith(' char %d (0x%02x U+%04X)' % (fb, fb, ord(first[0]))),
              repr(got.get('info')))
        print('%d passed, %d failed' % (passed, failed))
        sys.exit(1 if failed else 0)
    check('set the spell page', got.get('spell1') == 'cp1252', repr(got.get('spell1')))
    check('clear the spell page', got.get('spell2') == '', repr(got.get('spell2')))
    check('load with the current page', got.get('load') == 'iso8859-5|' + first, repr(got.get('load')))
    check('save keeps the buffer page', file('keep.txt') == TEXT.encode('iso8859_5'), repr(file('keep.txt')))
    check('insert uses the buffer page', got.get('insert') == 'iso8859-5|' + INS.strip(), repr(got.get('insert')))
    check('convert to cp1251', file('cp1251.txt') == (TEXT + INS).encode('cp1251'), repr(file('cp1251.txt')))
    check('set to UTF-8', got.get('utf8set') == '1' and got.get('utf8enc') == 'utf8', repr(got.get('utf8enc')))
    check('convert to UTF-8', file('utf8.txt') == (TEXT + INS).encode('utf-8'), repr(file('utf8.txt')))
    check('unknown buffer page fails', got.get('bad') == '0', repr(got.get('bad')))
    check('load with a wrong page', got.get('wrong') == 'cp1252|' + first.encode('iso8859_5').decode('cp1252'),
          repr(got.get('wrong')))
    check('read-file keeps a forced page', got.get('reread') == 'iso8859-5|' + first, repr(got.get('reread')))
    check('changed table translates on save', file('koi8.txt') == TEXT.encode('koi8_r'), repr(file('koi8.txt')))
    check('buffer-info byte and char', got.get('info', '').endswith(' char %d (%02X, U+%04X)' % (fb, fb, ord(first[0]))),
          repr(got.get('info')))
    check('file byte count is the char count', got.get('bytes') == str(len(TEXT)), repr(got.get('bytes')))
    check('buffer-info char not in page', got.get('infonot', '').endswith(' char U+20AC (not in iso8859-5)'),
          repr(got.get('infonot')))
    check('changed table leaves the text', got.get('retable') == 'iso8859-5|' + first, repr(got.get('retable')))
    print('%d passed, %d failed' % (passed, failed))
    sys.exit(1 if failed else 0)


if __name__ == '__main__':
    main()
