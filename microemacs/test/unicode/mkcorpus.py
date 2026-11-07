#!/usr/bin/env python3
#
# mkcorpus.py - Generate the Unicode acceptance test corpus.
#
# Usage: python3 -I mkcorpus.py [out-dir]       (default ./corpus)
#
# File names give the expected encoding; *.txt files are expected to load and save byte
# identical. The ids in brackets refer to unicode_acceptance_tests.md.

import os
import sys

LATIN = 'Café naïve façade – “quoted” Ærøskøbing Straße £5 ±1°\n'
GREEK = 'Γειά σου Κόσμε – ΣΊΣΥΦΟΣ σίσυφος\n'
CYRILLIC = 'Привет, мир! Москва ЁЖИК ёжик\n'
CJK = '中文测试 日本語のテキスト かな カナ 한국어 텍스트\n'
EMOJI = 'Emoji 😀 👍🏽 👨‍👩‍👧 🇬🇧 ❤️ ☺︎\n'
COMBINING = 'NFD: é ä ộ NFC: é ä ộ Jamo: 각\n'
WIDTHS = 'A\tTab\tWide中\tAmbig αβγ ±\tFull ＡＢＣ\tZW[​]\n'
MULTI = LATIN + GREEK + CYRILLIC + CJK + EMOJI + COMBINING + WIDTHS

JAPANESE = 'これは日本語のテストです。カタカナ、ひらがな、漢字。\n半角ｶﾀｶﾅ\n'
CHINESE_S = '这是简体中文测试。\n'
CHINESE_T = '這是繁體中文測試。\n'
KOREAN = '이것은 한국어 테스트입니다.\n'

# Each sequence is invalid on its own; they are joined with ASCII markers so the position of
# any corruption is easy to see
MALFORMED = [
    b'stray-cont[\x80]', b'stray-cont[\xbf]', b'overlong-slash[\xc0\xaf]', b'overlong-nul[\xc0\x80]',
    b'escape-lookalike[\xc1\xbf]', b'overlong3[\xe0\x80\xaf]', b'overlong4[\xf0\x80\x80\xaf]',
    b'surrogate[\xed\xa0\x80]', b'surrogate-pair[\xed\xa0\xbd\xed\xb8\x80]', b'too-big[\xf4\x90\x80\x80]',
    b'f5[\xf5\x80\x80\x80]', b'fe[\xfe]', b'ff[\xff]', b'cut2[\xc3]x', b'cut3[\xe2\x82]x',
    b'cut4[\xf0\x9f\x98]x', b'valid[\xc3\xa9]', b'latin1[\xe9t\xe9]',
]


def write(outDir, name, data):
    with open(os.path.join(outDir, name), 'wb') as fp:
        fp.write(data)


def main():
    outDir = sys.argv[1] if len(sys.argv) > 1 else 'corpus'
    os.makedirs(outDir, exist_ok=True)

    asciiText = ''.join(chr(cc) for cc in range(0x20, 0x7f)) + '\n' + 'Plain ASCII line\n\tTabbed\n'
    write(outDir, 'ascii.txt', asciiText.encode('ascii'))                                # F3
    write(outDir, 'latin1.txt', (LATIN.replace('–', '-').replace('“', '"').replace('”', '"') +
                                 ''.join(chr(cc) for cc in range(0xa0, 0x100)) + '\n').encode('latin-1'))  # F2
    write(outDir, 'cp1252.txt', ('€ ‚ƒ„…†‡ˆ‰Š‹Œ Ž ‘’“”•–—˜™š›œ žŸ\n' + LATIN).encode('cp1252'))  # F2
    write(outDir, 'cp1252_undef.txt', b'cp1252 undefined bytes [\x81\x8d\x8f\x90\x9d]\n')            # F2
    write(outDir, 'utf8.txt', MULTI.encode('utf-8'))                                     # F1, D1
    write(outDir, 'utf8_bom.txt', b'\xef\xbb\xbf' + MULTI.encode('utf-8'))               # F5
    write(outDir, 'utf8_cjk.txt', (JAPANESE + CHINESE_S + CHINESE_T + KOREAN).encode('utf-8'))  # D2
    write(outDir, 'utf8_crlf.txt', MULTI.replace('\n', '\r\n').encode('utf-8'))          # F10
    write(outDir, 'utf8_cr.txt', MULTI.replace('\n', '\r').encode('utf-8'))              # F10
    write(outDir, 'utf8_noeol.txt', MULTI.rstrip('\n').encode('utf-8'))
    write(outDir, 'utf8_late_invalid.txt', (MULTI * 200).encode('utf-8') + b'late[\xe9]\n')  # F6
    write(outDir, 'latin1_late.txt', (asciiText * 2000).encode('ascii') + 'late é\n'.encode('latin-1'))  # F6
    write(outDir, 'utf8_malformed.txt', b'\n'.join(MALFORMED) + b'\n')                   # F4
    write(outDir, 'utf8_malformed_eof.txt', b'truncated at EOF [\xf0\x9f\x98')           # F4

    # A line longer than the 0xff00 byte load limit with 3 byte characters straddling it
    longLine = ('x' + '中' * 30000 + '\n').encode('utf-8')
    write(outDir, 'utf8_long_line.txt', longLine)

    for name, codec, bom in (('utf16le', 'utf-16-le', b'\xff\xfe'), ('utf16be', 'utf-16-be', b'\xfe\xff'),
                             ('utf32le', 'utf-32-le', b'\xff\xfe\x00\x00'), ('utf32be', 'utf-32-be', b'\x00\x00\xfe\xff')):
        write(outDir, name + '_bom.txt', bom + MULTI.encode(codec))                       # F8, F16
        write(outDir, name + '.txt', MULTI.encode(codec))
    # Lone surrogate - not encodable by Python's codecs so built by hand
    write(outDir, 'utf16le_lone_surrogate.txt',
          b'\xff\xfe' + 'lone['.encode('utf-16-le') + b'\x00\xd8' + ']\n'.encode('utf-16-le'))

    # NUL bytes (F20) and the control characters translate-buffer had to escape
    write(outDir, 'nul_ascii.txt', b'NUL[\x00] two[\x00\x00]\n\x00leading\ntrailing\x00\n')
    write(outDir, 'nul_utf8.txt', MULTI.replace(' ', '\x00', 3).encode('utf-8'))
    write(outDir, 'control_chars.txt', b'SOH[\x01] STX[\x02] BEL[\x07] ESC[\x1b] DEL[\x7f]\n')

    # Line endings (F21) - mixed terminators & bare CRs inside lines
    lines = [ll + '\n' for ll in MULTI.split('\n')[:-1]]
    write(outDir, 'eol_mixed_lf.txt', ''.join(lines[:3] + [ll.replace('\n', '\r\n') for ll in lines[3:5]] + lines[5:]).encode('utf-8'))
    write(outDir, 'eol_mixed_crlf.txt', ''.join([ll.replace('\n', '\r\n') for ll in lines[:4]] + lines[4:5] +
                                                [ll.replace('\n', '\r\n') for ll in lines[5:]]).encode('utf-8'))
    write(outDir, 'eol_mixed_cr.txt', b'old mac\rstyle\rcrlf line\r\nlf line\nlast\r')
    write(outDir, 'eol_bare_cr.txt', b'progress 10%\r50%\r100%\ndone\nCRLF after bare CR\r\r\n')
    write(outDir, 'eol_ctrlz.txt', b'DOS text\r\nwith ctrl-z\r\n\x1a')

    # Legacy multi-byte encodings - unsupported, must still round trip (F17)
    write(outDir, 'sjis.txt', JAPANESE.encode('shift_jis'))
    write(outDir, 'eucjp.txt', JAPANESE.encode('euc_jp'))
    write(outDir, 'gbk.txt', CHINESE_S.encode('gbk'))
    write(outDir, 'big5.txt', CHINESE_T.encode('big5'))
    write(outDir, 'euckr.txt', KOREAN.encode('euc_kr'))

    sys.stderr.write('Corpus written to %s\n' % outDir)


if __name__ == '__main__':
    main()
