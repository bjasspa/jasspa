#!/usr/bin/env python3
#
# macro.py - Macro language tests (Phase 6): runs a headless mec and checks the string
# functions, escapes, byte functions and $window-col work a char at a time (acceptance tests
# M1-M3, M5 and M6).
#
# Usage: python3 -I macro.py <mec>

import os
import subprocess
import sys
import tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from edit import Suite  # noqa: E402


def build(ss):
    L = ss.line
    V = ss.val
    # the Latin-1 code page, which codepage.emf sets at start-up
    L('set-char-mask "c" "%s"' % ''.join('%04X' % cc for cc in range(0x80, 0x100)))

    # M1 - string functions count chars, negative counts too
    s4 = '"aé中😀"'
    for tag, expr, want in (
            ('M1.len', '&len ' + s4, '4'),
            ('M1.lef', '&lef %s 2' % s4, 'aé'),
            ('M1.lefneg', '&lef %s -1' % s4, 'aé中'),
            ('M1.rig', '&rig %s 2' % s4, '中😀'),
            ('M1.rigneg', '&rig %s -1' % s4, '😀'),
            ('M1.mid', '&mid %s 1 2' % s4, 'é中'),
            ('M1.midneg', '&mid %s -2 1' % s4, '中'),
            ('M1.midneglen', '&mid %s 1 -1' % s4, 'é中'),
            ('M1.sin', '&sin "中" ' + s4, '3'),
            ('M1.sinnone', '&sin "x" ' + s4, '0'),
            ('M1.rsin', '&rsin "é" "aéaé"', '4'),
            ('M1.isin', '&isin "É" "xxé"', '3'),
            ('M1.risin', '&risin "σ" "ΣxΣ"', '3'),
            ('M1.ise', '&ise "ÉCOLE" "école"', '1'),
            ('M1.isediff', '&ise "école" "ecole"', '0'),
            ('M1.rep', '&rep "aéaé" "é" "中"', 'a中a中'),
            ('M1.irep', '&irep "aÉaé" "é" "x"', 'axax'),
            ('M1.irepkelvin', '&irep "5K!" "k" "x"', '5x!'),
            ('M1.repempty', '&rep "é中" "" "-"', '-é-中-'),
            # a leader pair is one char, the negative count bug in src.old used bytes
            ('M1.leader', '&len "a\\x00b"', '3'),
            ('M1.leaderneg', '&len &lef "a\\x00bc" -1', '3'),
            ('M1.inw', '&cat &cat &inw "é" &inw "中" &inw "·"', '110'),
            ('M1.trb', '&cat &cat "[" &trb " é中 " "]"', '[é中]')):
        V(tag, expr, want)

    # M2 - &atoi and &itoa use code points
    for tag, expr, want in (
            ('M2.atoi', '&cat &cat &cat &cat &atoi "é" "," &atoi "中" "," &atoi "😀"', '233,20013,128512'),
            ('M2.atoinul', '&atoi "\\x00"', '0'),
            ('M2.itoa', '&cat &cat &itoa 233 &itoa 20013 &itoa 128512', 'é中😀'),
            ('M2.itoanul', '&cat &len &itoa 0 &bhex &itoa 0', '100'),
            ('M2.roundtrip', '&itoa &atoi "ж"', 'ж')):
        V(tag, expr, want)
    ss.buf('a中')
    L('forward-char')
    V('M2.wc', '&cat @wc &atoi @wc', '中20013')

    # M3 - case mapping, a char can change length
    for tag, expr, want in (
            ('M3.slo', '&slo "ÉCOLE ΑΘΗΝΑ ЖУК"', 'école αθηνα жук'),
            ('M3.sup', '&sup "straße ǆ ⱥ"', 'STRAßE Ǆ Ⱥ')):
        V(tag, expr, want)

    # escapes - \xNN is the char U+00NN, \u and \U take a code point
    for tag, expr, want in (
            ('Esc.x', '&cat &seq "\\xe9" "é" &atoi "\\xa3"', '1163'),
            ('Esc.u', '"\\u4e2d\\u00e9x"', '中éx'),
            ('Esc.U', '"\\U01F600"', '😀'),
            ('Esc.ushort', '"\\u41-"', 'A-'),
            ('Esc.nodigit', '"\\uZ\\U"', 'uZU'),
            ('Esc.len', '&len "\\xe9\\u4e2d\\U01F600"', '3')):
        V(tag, expr, want)

    # byte functions, a raw byte is its byte
    for tag, expr, want in (
            ('B.blen', '&blen "aé中"', '6'),
            ('B.bhex', '&bhex "aé"', '61c3a9'),
            ('B.hexb', '&hexb "61c3a9e4b8ad"', 'aé中'),
            ('B.hexbraw', '&cat &len &hexb "e9" &bhex &hexb "e9"', '1e9'),
            ('B.hexbnul', '&cat &len &hexb "610062" &bhex &hexb "610062"', '3610062'),
            ('B.batoi', '&cat &cat &cat &batoi "é" "," &batoi &bmid "aé" -1 1 &cat "," &batoi ""', '195,169,0'),
            ('B.bmid', '&bmid "aé中" 1 2', 'é'),
            ('B.bmidsplit', '&cat &len &bmid "aé" 1 1 &bhex &bmid "aé" 1 1', '1c3'),
            ('B.bitoa', '&cat &bitoa 65 &bhex &bitoa 233', 'Ae9'),
            ('B.rawlen', '&cat &blen &bitoa 233 &len &cat &bitoa 195 &bitoa 169', '12'),
            ('B.bmiddecode', '&bmid &cat &bitoa 195 &bitoa 169 0 9', 'é')):
        V(tag, expr, want)
    L('!force set-variable #l0 &hexb "zz"')
    V('B.hexbbad', '$status', '0')
    # any char including a newline, [\x01-\xff] stopped at U+00FF
    V('R.anychar', '&xseq "a\\n中\\x1c" "[\\\\s\\\\S]*"', '1')

    # list functions take a multi-byte separator (\xa0 is now a 2 byte char)
    for tag, expr, want in (
            ('L.lget', '&lget "\\xa0a\\xa0b\\xa0" 2', 'b'),
            ('L.llen', '&lle "·a·b·"', '2'),
            ('L.lfind', '&lfind "·a·b·" "b"', '2'),
            ('L.lset', '&lset "·a·b·" 1 "x"', '·x·b·'),
            ('L.lins', '&lins "·a·b·" 2 "x"', '·a·x·b·'),
            ('L.linsend', '&lins "·a·b·" -1 "z"', '·a·b·z·'),
            ('L.linssort', '&lins "|a|é|z|" -3 "É"', '|a|É|é|z|'),
            ('L.ldel', '&lde "·a·b·" 1', '·b·')):
        V(tag, expr, want)

    # &spr widths are in cells, never splitting a char
    for tag, expr, want in (
            ('Spr.left', '&spr "[%-4s]" "中"', '[中  ]'),
            ('Spr.right', '&spr "[%4s]" "é"', '[   é]'),
            ('Spr.prec', '&spr "[%.3s]" "中文字"', '[中]'),
            ('Spr.both', '&spr "[%-5.3s]" "中文字"', '[中   ]')):
        V(tag, expr, want)

    # $window-col and $window-mark-col are char indexes
    ss.buf('a中b')
    L('set-variable $window-col 2')
    V('Col.set', '&cat @wc $window-col', 'b2')
    V('Col.lef', '&lef @wl $window-col', 'a中')
    # M6 - the byte offset is the byte length of the chars before
    V('M6.bytes', '&blen &lef @wl $window-col', '4')
    L('set-mark')
    L('beginning-of-line')
    V('Col.mark', '$window-mark-col', '2')
    L('!force set-variable $window-col 5')
    V('Col.past', '&cat $status $window-col', '03')
    ss.buf('xé')
    L('set-mark')
    L('set-variable $window-mark-col 1')
    L('exchange-point-and-mark')
    V('Col.setmark', '&cat @wc $window-col', 'é1')

    # @wc is only "" at the end of the buffer, a NUL char is not empty
    nul = os.path.join(ss.outDir, 'nul.in')
    open(nul, 'wb').write(b'a\x00b\n\x00\x00\nc\xe9\x00\n')
    L('find-file "%s"' % nul)
    L('set-variable #l1 0')
    L('!while &not &seq @wc ""')
    L('  set-variable #l1 &add #l1 1')
    L('  forward-char')
    L('!done')
    V('NUL.walk', '&cat #l1 $window-line', '114')
    L('beginning-of-buffer')
    L('forward-char')
    V('NUL.wc', '&cat &cat &len @wc &bhex @wc &atoi @wc', '1000')
    L('!force 0 delete-buffer $buffer-bname')

    # Unicode info functions, the utf8 field holds the bytes as raw bytes
    for tag, expr, want in (
            ('U.uni', '&cat &cat &lget &uni 233 3 &lget &uni 233 4 &bhex &lget &uni 233 5', 'é2c3a9'),
            ('U.uniascii', '&lget &uni 65 3', 'A'),
            ('U.ufi', '&lget &ufi "é" 2', '0x0000e9'),
            ('U.ufiraw', '&cat &lget &ufi &cat &bitoa 195 &bitoa 169 3 &lget &ufi &cat &bitoa 195 &bitoa 169 4', 'é2'),
            ('U.ufibad', '&lget &ufi &bitoa 233 1', '0'),
            ('U.uciraw', '&lget &uci &bitoa 233 2', '0x0000e9'),
            ('U.uci', '&lget &uci "中" 2', '0x004e2d')):
        V(tag, expr, want)

    # M5 - UTF-8 registry values are saved and read back
    reg = os.path.join(ss.outDir, 'test.erf')
    L('set-registry "/utest" ""')
    L('set-registry "/utest/val" "é中😀"')
    L('save-registry "/utest" "%s"' % reg)
    L('delete-registry "/utest"')
    L('read-registry "/utest" "%s" ""' % reg)
    V('M5.registry', '&reg "/utest/val" "none"', 'é中😀')


def main():
    if len(sys.argv) < 2:
        sys.exit('Usage: macro.py <mec>')
    mec = os.path.abspath(sys.argv[1])
    wdir = tempfile.mkdtemp()
    outDir = os.path.join(wdir, 'out')
    os.mkdir(outDir)
    ss = Suite(outDir)
    ss.mac.append('define-macro start-up')
    build(ss)
    ss.mac += ['  4 exit-emacs 0', '!emacro']
    open(os.path.join(wdir, 'ma.emf'), 'w', encoding='utf-8').write('\n'.join(ss.mac) + '\n')
    try:
        res = subprocess.run([mec, '-p', '@' + os.path.join(wdir, 'ma')], stdin=subprocess.DEVNULL,
                             capture_output=True, env=dict(os.environ, MEPATH=wdir), timeout=120)
    except subprocess.TimeoutExpired:
        sys.exit('FAIL mec timed out - a prompt is probably waiting for input')
    log = (res.stdout + res.stderr).decode('utf-8', 'backslashreplace').replace('\r', '\n')
    got = {}
    for line in log.split('\n'):
        if 'R:' in line and '=' in line:
            key, val = line[line.index('R:')+2:].split('=', 1)
            got[key] = val
    passed = failed = 0
    for tag, want in ss.expect.items():
        if got.get(tag) == want:
            passed += 1
        else:
            failed += 1
            print('FAIL %s: got %r expected %r' % (tag, got.get(tag), want))
    print('%d passed, %d failed' % (passed, failed))
    sys.exit(1 if failed else 0)


if __name__ == '__main__':
    main()
