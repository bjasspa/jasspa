#!/usr/bin/env python3
#
# hash.py - Tests generate-hash with a headless mec. A text buffer is hashed as its text in UTF-8
# with a \n line end, whatever the encoding, line ending style or ctrlz mode, so the hash is the
# same on every platform; a line with no line end (the last line, or a long line split on load)
# adds none. binary and rbin buffers hash the file's bytes. Key and data strings are hashed as
# their bytes, so a NUL or a byte from &hexb is hashed as that byte.
#
# Usage: python3 -I hash.py <mec>
#
# Works with an 8-bit or Unicode build, the Unicode only cases (NULs in a buffer, a code page
# file decoded to UTF-8) are skipped by an 8-bit build.

import hashlib
import hmac
import os
import subprocess
import sys
import tempfile

LATIN1_TABLE = ''.join('%04X' % cc for cc in range(0x80, 0x100))
LONG = b'x' * 70000 + b'\n'


def sha(data):
    return hashlib.sha256(data).hexdigest().upper()


def mac(key, data):
    return hmac.new(key, data, hashlib.sha256).hexdigest().upper()


def main():
    if len(sys.argv) < 2:
        sys.exit('Usage: hash.py <mec>')
    mec = os.path.abspath(sys.argv[1])
    wdir = tempfile.mkdtemp()
    files = {
        'unix.txt': b'a\nb\n',
        'dos.txt': b'a\r\nb\r\n',
        'mac.txt': b'a\rb\r',
        'mixed.txt': b'a\r\nb\n',
        'noeol.txt': b'a\nb',
        'ctrlz.txt': b'a\r\nb\r\n\x1a',
        'utf8.txt': 'café €\n'.encode(),
        'invalid.txt': b'caf\xc3\xa9 \xff\n',
        'latin1.txt': b'caf\xe9\n',
        'nul.txt': b'a\x00b\n',
        'long.txt': LONG,
        'region.txt': b'abc\r\ndef\r\nghi\r\n',
        'rbin.txt': b'\x00\x01\xff\r\nz',
    }
    for name, data in files.items():
        open(os.path.join(wdir, name), 'wb').write(data)

    lines = ['define-macro start-up', '  set-char-mask "c" "%s"' % LATIN1_TABLE,
             '  -2 ml-write &cat "uni:" &opt "uni"']
    want = {}
    uniOnly = set()

    def string(tag, cmd, value):
        lines.extend(['  %s' % cmd, '  -2 ml-write &cat "hash:%s=" $result' % tag])
        want[tag] = value

    def buffer(tag, name, value, n=1, ff=1, pre=(), uni=False):
        lines.append('  !force %d find-file "%s"' % (ff, os.path.join(wdir, name)))
        lines.extend('  ' + ll for ll in pre)
        lines.extend(['  %d generate-hash' % n, '  -2 ml-write &cat "hash:%s=" $result' % tag,
                      '  !force 0 delete-buffer $buffer-bname'])
        want[tag] = value
        if uni:
            uniOnly.add(tag)

    # strings
    string('s.plain', '2 generate-hash "Hello world"', sha(b'Hello world'))
    string('s.doc', '6 generate-hash "TheKey" "Hello world"',
           '1804C1869D6DD76A9A8D00819E6F62E970D981F01FE805F927CF4E3BDF4AD95C')
    string('s.hexb', '6 generate-hash &hex "41424344454647" &hex "41424344454647"', mac(b'ABCDEFG', b'ABCDEFG'))
    string('s.high', '6 generate-hash "ABCDEFG" &hex "c0c1c2"', mac(b'ABCDEFG', b'\xc0\xc1\xc2'))
    string('s.nul', '6 generate-hash "ABCDEFG" &cat "A" &hex "00"', mac(b'ABCDEFG', b'A\x00'))
    # a key is padded with 0s, so a NUL in a key must have a byte after it to count
    string('s.nulkey', '6 generate-hash &cat &cat "K" &hex "00" "Z" "data"', mac(b'K\x00Z', b'data'))
    string('s.hexdata', '3 generate-hash "c0c1c2"', sha(b'\xc0\xc1\xc2'))
    string('s.hexkey', '14 generate-hash "4b005a" "data"', mac(b'K\x00Z', b'data'))
    string('s.utf8', '2 generate-hash "café"', sha('café'.encode()))
    # buffers - the text with \n line ends
    buffer('b.unix', 'unix.txt', sha(b'a\nb\n'))
    buffer('b.dos', 'dos.txt', sha(b'a\nb\n'))
    buffer('b.mac', 'mac.txt', sha(b'a\nb\n'))
    buffer('b.mixed', 'mixed.txt', sha(b'a\nb\n'))
    buffer('b.noeol', 'noeol.txt', sha(b'a\nb'))
    buffer('b.ctrlz', 'ctrlz.txt', sha(b'a\nb\n'))
    buffer('b.utf8', 'utf8.txt', sha(files['utf8.txt']))
    buffer('b.invalid', 'invalid.txt', sha(files['invalid.txt']))
    buffer('b.long', 'long.txt', sha(LONG))
    buffer('b.latin1', 'latin1.txt', sha('café\n'.encode()), uni=True)
    buffer('b.nul', 'nul.txt', sha(files['nul.txt']), uni=True)
    # regions, from line 1 col 1 to line 3 col 2, and within line 2
    buffer('r.lines', 'region.txt', sha(b'bc\ndef\ngh'), n=0,
           pre=('beginning-of-buffer', 'forward-char', 'set-mark', '3 goto-line', '2 forward-char'))
    buffer('r.line', 'region.txt', sha(b'de'), n=0,
           pre=('2 goto-line', 'set-mark', '2 forward-char'))
    # rbin is byte accurate
    buffer('b.rbin', 'rbin.txt', sha(files['rbin.txt']), ff=9)
    lines += ['  4 exit-emacs 0', '!emacro']
    open(os.path.join(wdir, 'hs.emf'), 'w', encoding='utf-8').write('\n'.join(lines) + '\n')
    try:
        res = subprocess.run([mec, '-p', '@' + os.path.join(wdir, 'hs')], stdin=subprocess.DEVNULL,
                             capture_output=True, env=dict(os.environ, MEPATH=wdir), timeout=120)
    except subprocess.TimeoutExpired:
        sys.exit('FAIL mec timed out')
    out = (res.stdout + res.stderr).decode('utf-8', 'replace').replace('\r', '\n').split('\n')
    uni = any(ll.startswith('uni:1') for ll in out)
    got = {}
    for ll in out:
        if ll.startswith('hash:') and '=' in ll:
            tag, val = ll[5:].split('=', 1)
            got[tag] = val.strip()
    passed = failed = 0
    for tag, value in want.items():
        if (tag in uniOnly) and not uni:
            continue
        if got.get(tag) == value:
            passed += 1
        else:
            failed += 1
            print('FAIL %s got %s, expected %s' % (tag, got.get(tag), value))
    print('%d passed, %d failed' % (passed, failed))
    sys.exit(1 if failed else 0)


if __name__ == '__main__':
    main()
