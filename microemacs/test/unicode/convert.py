#!/usr/bin/env python3
#
# convert.py - Checks file encoding detection, conversion between encodings and line ending
# handling with a headless mec (acceptance tests F1-F8, F16, F20, F21, F23).
#
# Usage: python3 -I convert.py <mec> [corpus-dir]
#
# Only the generated start-up macro runs, not me.emf. A Latin-1 code page is installed with
# set-char-mask "c" so legacy files decode to real characters.

import os
import subprocess
import sys
import tempfile

LATIN1_TABLE = ''.join('%04X' % cc for cc in range(0x80, 0x100))

DETECT = {
    'ascii.txt': 1, 'control_chars.txt': 1, 'nul.txt': 1, 'nul_utf8.txt': 1,
    'eol_bare_cr.txt': 1, 'eol_ctrlz.txt': 1, 'eol_mixed_cr.txt': 1, 'eol_mixed_crlf.txt': 1,
    'eol_mixed_lf.txt': 1, 'utf8.txt': 1, 'utf8_bom.txt': 9, 'utf8_cjk.txt': 1, 'utf8_cr.txt': 1,
    'utf8_crlf.txt': 1, 'utf8_late_invalid.txt': 1, 'utf8_long_line.txt': 1, 'utf8_noeol.txt': 1,
    'latin1.txt': 0, 'latin1_late.txt': 0, 'cp1252.txt': 0, 'cp1252_undef.txt': 0,
    'sjis.txt': 0, 'eucjp.txt': 0, 'gbk.txt': 0, 'big5.txt': 0, 'euckr.txt': 0,
    'utf16le.txt': 2, 'utf16be.txt': 6, 'utf16le_bom.txt': 10, 'utf16be_bom.txt': 14,
    'utf16le_lone_surrogate.txt': 10,
    'utf32le.txt': 3, 'utf32be.txt': 7, 'utf32le_bom.txt': 11, 'utf32be_bom.txt': 15,
}

# line ending style chosen on load - (cr mode, lf mode)
EOL_MODES = {
    'eol_mixed_lf.txt': (0, 1), 'eol_mixed_crlf.txt': (1, 1), 'eol_mixed_cr.txt': (1, 0),
    'utf8_cr.txt': (1, 0), 'utf8_crlf.txt': (1, 1), 'utf8.txt': (0, 1),
}

ENC_CODEC = {1: 'utf-8', 9: 'utf-8-sig', 2: 'utf-16-le', 6: 'utf-16-be', 3: 'utf-32-le', 7: 'utf-32-be',
             0: 'latin-1'}
BOMS = {10: (b'\xff\xfe', 'utf-16-le'), 14: (b'\xfe\xff', 'utf-16-be'),
        11: (b'\xff\xfe\x00\x00', 'utf-32-le'), 15: (b'\x00\x00\xfe\xff', 'utf-32-be')}


def encode(text, enc):
    if enc in BOMS:
        bom, codec = BOMS[enc]
        return bom + text.encode(codec)
    return text.encode(ENC_CODEC[enc])


def encId(enc):
    """the $buffer-encoding of the old numeric encoding bits, a single byte file being in code page C"""
    if not enc & 3:
        return 'C'
    ss = ('', 'utf8', 'utf16', 'utf32')[enc & 3] + (('be' if enc & 4 else 'le') if enc & 3 > 1 else '')
    return ss + ('-bom' if enc & 8 else '')


def main():
    if len(sys.argv) < 2:
        sys.exit('Usage: convert.py <mec> [corpus-dir]')
    mec = os.path.abspath(sys.argv[1])
    corpus = os.path.abspath(sys.argv[2] if len(sys.argv) > 2 else
                             os.path.join(os.path.dirname(os.path.abspath(__file__)), 'corpus'))
    wdir = tempfile.mkdtemp()
    out = os.path.join(wdir, 'out')
    os.mkdir(out)
    utf8 = open(os.path.join(corpus, 'utf8.txt'), 'rb').read().decode('utf-8')
    latin1 = open(os.path.join(corpus, 'latin1.txt'), 'rb').read().decode('latin-1')

    mac = ['define-macro start-up', '  set-char-mask "c" "%s"' % LATIN1_TABLE]

    def load(name):
        mac.append('  !force find-file "%s"' % os.path.join(corpus, name))

    def save(name, enc=None):
        if enc is not None:
            mac.append('  set-variable $buffer-encoding "%s"' % encId(enc))
        mac.append('  !force 0 write-buffer "%s"' % os.path.join(out, name))
        mac.append('  !force 0 delete-buffer $buffer-bname')

    def report(tag, expr):
        mac.append('  -2 ml-write &cat "%s=" %s' % (tag, expr))

    for name in sorted(DETECT):
        load(name)
        report('enc:' + name, '$buffer-encoding')
        report('eol:' + name, '&cat &bmod "cr" &bmod "lf"')
        mac.append('  !force 0 delete-buffer $buffer-bname')

    expected = {}
    # every UTF encoding of the same text converts to plain UTF-8 and back
    for src in ('utf8_bom.txt', 'utf16le.txt', 'utf16be.txt', 'utf16le_bom.txt', 'utf16be_bom.txt',
                'utf32le.txt', 'utf32be.txt', 'utf32le_bom.txt', 'utf32be_bom.txt'):
        load(src)
        save('to8.' + src, 1)
        expected['to8.' + src] = utf8.encode('utf-8')
    for enc in (9, 2, 6, 3, 7, 10, 14, 11, 15):
        load('utf8.txt')
        save('from8.%d' % enc, enc)
        expected['from8.%d' % enc] = encode(utf8, enc)
    # legacy Latin-1 <-> UTF-8
    load('latin1.txt')
    save('latin1.to8', 1)
    expected['latin1.to8'] = latin1.encode('utf-8')
    mac.append('  !force find-file "%s"' % os.path.join(out, 'latin1.to8'))
    save('latin1.back', 0)
    expected['latin1.back'] = latin1.encode('latin-1')
    # characters with no Latin-1 code are written as '?'
    load('utf8.txt')
    save('utf8.tolatin1', 0)
    expected['utf8.tolatin1'] = utf8.encode('latin-1', errors='replace')
    # NUL survives conversion
    nul = open(os.path.join(corpus, 'nul.txt'), 'rb').read().decode('utf-8')
    load('nul.txt')
    save('nul.to16', 2)
    expected['nul.to16'] = nul.encode('utf-16-le')
    # a NUL before a line's first non-ASCII byte does not stop the UTF-8 detection
    nulfirst = os.path.join(wdir, 'nulfirst.txt')
    open(nulfirst, 'wb').write('\0é\n\r'.encode('utf-8'))
    mac.append('  !force find-file "%s"' % nulfirst)
    save('nulfirst.to16', 2)
    expected['nulfirst.to16'] = '\0é\n\r'.encode('utf-16-le')
    # changing the line ending style normalises every line (F23)
    for name, mode, target in (('eol_mixed_lf.txt', 'cr', '\n'), ('eol_mixed_crlf.txt', 'cr', '\r\n')):
        load(name)
        mac.append('  -1 buffer-mode "%s"' % mode)
        mac.append('  1 buffer-mode "%s"' % mode)
        if target == '\n':
            mac.append('  -1 buffer-mode "cr"')
        save('norm.' + name)
        text = open(os.path.join(corpus, name), 'rb').read().decode('utf-8')
        expected['norm.' + name] = text.replace('\r\n', '\n').replace('\n', target).encode('utf-8')
    # splitting a line without a terminator keeps the missing terminator on the second part
    noeol = os.path.join(wdir, 'noeol.txt')
    open(noeol, 'wb').write(b'abc')
    mac.append('  !force find-file "%s"' % noeol)
    mac.append('  forward-char')
    mac.append('  forward-char')
    mac.append('  newline')
    save('split.noeol')
    expected['split.noeol'] = b'ab\nc'
    # splitting a CRLF line in an LF buffer - the second part keeps the CRLF
    mixed = os.path.join(wdir, 'mixed.txt')
    open(mixed, 'wb').write(b'one\ntwo\r\nthree\n')
    mac.append('  !force find-file "%s"' % mixed)
    mac.append('  forward-line')
    mac.append('  forward-char')
    mac.append('  newline')
    save('split.crlf')
    expected['split.crlf'] = b'one\nt\nwo\r\nthree\n'

    # encrypted file - detection and the BOM work on the decrypted data (F11)
    crypt = os.path.join(out, 'crypt.bin')
    load('utf16le_bom.txt')
    mac.append('  set-encryption-key "secret"')
    mac.append('  !force 0 write-buffer "%s"' % crypt)
    mac.append('  !force 0 delete-buffer $buffer-bname')
    mac.append('  !force 4 find-file "%s" "secret"' % crypt)
    report('enc:crypt', '$buffer-encoding')
    mac.append('  set-encryption-key ""')
    save('crypt.plain')
    expected['crypt.plain'] = open(os.path.join(corpus, 'utf16le_bom.txt'), 'rb').read()
    DETECT['crypt'] = 10

    mac += ['  4 exit-emacs 0', '!emacro']
    open(os.path.join(wdir, 'cv.emf'), 'w').write('\n'.join(mac) + '\n')
    env = dict(os.environ, MEPATH=wdir)
    try:
        res = subprocess.run([mec, '-p', '@' + os.path.join(wdir, 'cv')], stdin=subprocess.DEVNULL,
                             capture_output=True, env=env, timeout=120)
        log = (res.stdout + res.stderr).decode('latin-1').replace('\r', '\n')
    except subprocess.TimeoutExpired:
        sys.exit('FAIL mec timed out - a prompt is probably waiting for input')

    got = {}
    for line in log.split('\n'):
        if '=' in line and line.split('=', 1)[0][:4] in ('enc:', 'eol:'):
            key, val = line.split('=', 1)
            got[key] = val.strip()
    passed = failed = 0
    for name, enc in sorted(DETECT.items()):
        if got.get('enc:' + name) == encId(enc):
            passed += 1
        else:
            failed += 1
            print('FAIL detect %s: $buffer-encoding %s expected %s' % (name, got.get('enc:' + name), encId(enc)))
    for name, (cr, lf) in sorted(EOL_MODES.items()):
        want = '%d%d' % (cr, lf)
        if got.get('eol:' + name) == want:
            passed += 1
        else:
            failed += 1
            print('FAIL eol style %s: cr/lf %s expected %s' % (name, got.get('eol:' + name), want))
    for name, data in sorted(expected.items()):
        path = os.path.join(out, name)
        if not os.path.exists(path):
            failed += 1
            print('FAIL %s not written' % name)
            continue
        have = open(path, 'rb').read()
        if have == data:
            passed += 1
        else:
            failed += 1
            ii = next((ii for ii in range(min(len(have), len(data))) if have[ii] != data[ii]), min(len(have), len(data)))
            print('FAIL %s differs at byte %d (size %d expected %d): got %r expected %r' %
                  (name, ii, len(have), len(data), have[max(0, ii-8):ii+8], data[max(0, ii-8):ii+8]))
    print('%d passed, %d failed' % (passed, failed))
    sys.exit(1 if failed else 0)


if __name__ == '__main__':
    main()
