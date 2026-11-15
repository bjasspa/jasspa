#!/usr/bin/env python3
#
# normalize.py - accents-compose-region (NFC) and accents-decompose-region (NFD) with a headless
# mec: the whole buffer both ways against Python's unicodedata (decomposed and precomposed
# accents, marks out of order, Hangul, the Angstrom and Ohm signs), a region starting and ending
# within lines, a raw byte left as it is, one undo restoring the text, and a line which would grow
# past the line length limit left unchanged. The full conformance test (NormalizationTest.txt) is
# in unitest.c, run by run_unitest.sh. A build without Unicode support is only checked to refuse them.
#
# Usage: python3 -I normalize.py <mec>

import os
import subprocess
import sys
import tempfile
import unicodedata

C = chr
TEXT = '\n'.join(['cafe' + C(0x301) + ' na' + C(0x131) + 've e' + C(0x301) + C(0x308) + 'x',
                  'o' + C(0x323) + C(0x302) + ' ' + C(0x1ed9) + ' a' + C(0x302) + C(0x323),
                  C(0xd55c) + C(0xad6d) + ' ' + C(0x1112) + C(0x1161) + C(0x11ab),
                  C(0x212b) + ' ' + C(0x2126) + ' plain ascii',
                  'keep ' + C(0xe9)]) + '\n'

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
        sys.exit('Usage: normalize.py <mec>')
    mec = os.path.abspath(sys.argv[1])
    wdir = tempfile.mkdtemp()

    def path(name):
        return os.path.join(wdir, name)

    open(path('in.txt'), 'wb').write(TEXT.encode('utf-8'))
    open(path('raw.txt'), 'wb').write(b'x\xe9 e\xcc\x81\n')
    open(path('long.txt'), 'wb').write(('a' + C(0xe9) * 30000 + '\n').encode('utf-8'))
    whole = ['beginning-of-buffer', 'set-mark', 'end-of-buffer']
    # a build without Unicode support has the commands but they fail
    mac = ['define-macro start-up', '-2 ml-write &cat "R:uni=" &opt "uni"', '!if &not &opt "uni"',
           '  !force accents-compose-region', '  -2 ml-write &cat "R:na=" $status', '  4 exit-emacs 0', '!endif',
           '1 global-mode "undo"']

    def run(name, src, lines, out, pre=()):
        mac.append('find-file "%s"' % path(src))
        mac.extend(pre)
        mac.extend(lines)
        mac.append('-2 ml-write &cat "R:%s=" $status' % name)
        mac.append('!force 0 write-buffer "%s"' % path(out))
        mac.extend(['-1 buffer-mode "edit"', '!force 0 delete-buffer $buffer-bname'])

    run('nfc', 'in.txt', whole + ['accents-compose-region'], 'nfc.out')
    run('nfd', 'in.txt', whole + ['accents-decompose-region'], 'nfd.out')
    run('undo', 'in.txt', whole + ['accents-compose-region', 'undo'], 'undo.out')
    # from after "cafe" on line 1 to after "o" and its dot below on line 2
    run('part', 'in.txt', ['beginning-of-buffer', '4 forward-char', 'set-mark', 'forward-line',
                           'beginning-of-line', '2 forward-char', 'accents-compose-region'], 'part.out')
    # a stray byte makes it a code page file, so UTF-8 is forced to have a raw byte in a UTF-8 buffer
    run('raw', 'raw.txt', whole + ['accents-compose-region'], 'raw.out',
        pre=['set-variable $buffer-encoding "utf8"', '-1 buffer-mode "edit"', '1 read-file $buffer-fname'])
    run('long', 'long.txt', whole + ['!force accents-decompose-region'], 'long.out')
    mac += ['4 exit-emacs 0', '!emacro']
    open(path('nm.emf'), 'w', encoding='utf-8').write('\n'.join(mac) + '\n')
    try:
        res = subprocess.run([mec, '-n', '-p', '@' + path('nm')], stdin=subprocess.DEVNULL, capture_output=True,
                             env=dict(os.environ, MEPATH=wdir), timeout=120)
    except subprocess.TimeoutExpired:
        sys.exit('FAIL mec timed out')
    log = (res.stdout + res.stderr).decode('utf-8', 'replace').replace('\r', '\n')
    got = dict(ll[2:].split('=', 1) for ll in log.split('\n') if ll.startswith('R:') and '=' in ll)

    if got.get('uni') != '1':
        check('not available without Unicode', got.get('na') == '0', repr(got.get('na')))
        print('%d passed, %d failed (no Unicode support)' % (passed, failed))
        sys.exit(1 if failed else 0)

    def text(name):
        return open(path(name), 'rb').read().decode('utf-8', 'surrogateescape') if os.path.exists(path(name)) else None

    check('compose status', got.get('nfc') == '1', repr(got.get('nfc')))
    check('compose', text('nfc.out') == unicodedata.normalize('NFC', TEXT), repr(text('nfc.out')))
    check('decompose', text('nfd.out') == unicodedata.normalize('NFD', TEXT), repr(text('nfd.out')))
    check('undo restores', text('undo.out') == TEXT, repr(text('undo.out')))
    eol = TEXT.index('\n')
    check('region within lines', text('part.out') == TEXT[:4] + unicodedata.normalize('NFC', TEXT[4:eol + 3]) +
          TEXT[eol + 3:], repr(text('part.out')))
    check('raw byte kept', open(path('raw.out'), 'rb').read() == b'x\xe9 \xc3\xa9\n', repr(open(path('raw.out'), 'rb').read()))
    check('too long fails', got.get('long') == '0', repr(got.get('long')))
    check('too long unchanged', open(path('long.out'), 'rb').read() == open(path('long.txt'), 'rb').read())
    print('%d passed, %d failed' % (passed, failed))
    sys.exit(1 if failed else 0)


if __name__ == '__main__':
    main()
