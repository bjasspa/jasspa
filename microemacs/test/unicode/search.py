#!/usr/bin/env python3
#
# search.py - Search and replace tests (Phase 5): runs a headless mec over scratch buffers and
# checks where searches stop and what replaces produce (acceptance tests S1-S6).
#
# Usage: python3 -I search.py <mec>

import os
import subprocess
import sys
import tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from edit import Suite, charCol  # noqa: E402


def build(ss):
    L = ss.line

    def search(tag, text, cmd, pat, modes, col, line=1, want=None):
        # col is the byte offset dot is left at, -1 for not found ($window-col is a char index)
        ss.buf(text)
        for mm in modes:
            L(mm)
        if 'backward' in cmd:
            L('end-of-buffer')
        L('!force %s "%s"' % (cmd, pat))
        ss.val(tag, '&cat &cat $status "|" &cat $window-line &cat "|" $window-col',
               '0|%d|%d' % (1 if ('backward' in cmd) and want is None else line, 0) if col < 0 else
               '1|%d|%d' % (line, charCol(text.split('\n')[line - 1], col)))

    text = 'abc 中文 déf\n'
    lit = ['-1 buffer-mode "magic"', '1 buffer-mode "exact"']
    # S1 - literal searches find non-ASCII in both directions
    ss.buf(text)
    for mm in lit:
        L(mm)
    L('search-forward "中文"')
    ss.rep('S1.fwd', 1, 10, 8, 'abc 中文 déf')
    L('search-forward "é"')
    ss.rep('S1.fwd2', 1, 14, 11, 'abc 中文 déf')
    L('search-backward "中"')
    ss.rep('S1.back', 1, 4, 4, 'abc 中文 déf')
    L('end-of-line')
    L('search-backward "c 中"')
    ss.rep('S1.back2', 1, 2, 2, 'abc 中文 déf')

    # S2 - case insensitive literal search folds Latin, Greek and Cyrillic
    nocase = ['-1 buffer-mode "magic"', '-1 buffer-mode "exact"']
    for tag, txt, pat, col in (('S2.latin', 'x école\n', 'ÉCOLE', 8),
                               ('S2.greek', 'x αθηνα\n', 'ΑΘΗΝΑ', 12),
                               ('S2.cyrillic', 'x СЛОВО\n', 'слово', 12),
                               ('S2.sigma', 'x λόγος\n', 'ΛΌΓΟΣ', 12),
                               ('S2.kelvin', 'x 5\u212a\n', '5k', 6),
                               ('S2.eszett', 'x \u1e9e\n', '\u00df', 5),
                               ('S2.finalsigma', 'x λόγος\n', 'Σ', 12)):
        ss.buf(txt)
        for mm in nocase:
            L(mm)
        L('!force search-forward "%s"' % pat)
        ss.val(tag, '&cat $status &cat "|" $window-col', '1|%d' % charCol(txt.rstrip('\n'), col))
    ss.buf('x école\n')
    for mm in lit:
        L(mm)
    L('!force search-forward "ÉCOLE"')
    ss.val('S2.exact', '$status', '0')

    # S1 - the literal scanner's filters (memchr on the first char's last byte matching case, a
    # second byte after a lead byte ignoring case) never skip a match
    for tag, txt, pat, modes, col, line in (
            ('S1.lit.cyr', 'да нет жук\n', 'жук', lit, 18, 1),
            ('S1.lit.cjk', '中中文 文\n', '文', lit, 9, 1),
            ('S1.lit.lead', 'бжук\n', 'ж', lit, 4, 1),
            ('S1.lit.nextline', 'xж\nжук\n', 'жук', lit, 6, 2),
            ('S1.lit.fold', 'да нет ЖУК\n', 'жук', nocase, 18, 1),
            ('S1.lit.foldmix', 'x ЖуК\n', 'жУк', nocase, 8, 1)):
        search(tag, txt, 'search-forward', pat, modes, col, line)

    # S3 - the regex start filter (a match's first byte, and second after a lead byte) never skips a
    # match: non-ASCII first chars exact and ignoring case, classes, optional items, alternatives
    rx = ['1 buffer-mode "magic"', '1 buffer-mode "exact"']
    rxi = ['1 buffer-mode "magic"', '-1 buffer-mode "exact"']
    for tag, txt, cmd, pat, modes, col in (
            ('S3.start.cyr', 'да нет жук\n', 'search-forward', 'жук', rx, 18),
            ('S3.start.cyrfold', 'да нет ЖУК\n', 'search-forward', 'жук', rxi, 18),
            ('S3.start.cjk', 'x 中文\n', 'search-forward', '文', rx, 8),
            ('S3.start.eszett', 'x ẞ\n', 'search-forward', 'ß', rxi, 5),
            ('S3.start.kelvin', 'x K5\n', 'search-forward', 'K5', rxi, 4),
            ('S3.start.digits', 'привет 123x\n', 'search-forward', '[0-9][0-9][0-9]x', rx, 17),
            ('S3.start.range', 'abc жук\n', 'search-forward', '[а-я]+', rx, 10),
            ('S3.start.neg', 'aaé\n', 'search-forward', '[^a]', rx, 4),
            ('S3.start.opt', 'bb жa\n', 'search-forward', 'ж?a', rx, 6),
            ('S3.start.alt', 'bb ж\n', 'search-forward', 'q\\\\|ж', rx, 5),
            ('S3.start.back', 'жук да жук\n', 'search-backward', 'жук', rx, 12)):
        search(tag, txt, cmd, pat, modes, col)

    # S6 - a byte never matches inside a char, only a raw byte
    ss.buf('é ok\n')
    for mm in lit:
        L(mm)
    L('!force search-forward "\\xa9"')
    ss.val('S6.literal', '$status', '0')
    ss.buf('é ok\n')
    for mm in ('1 buffer-mode "magic"', '1 buffer-mode "exact"'):
        L(mm)
    L('!force search-forward "\\\\xa9"')
    ss.val('S6.regex', '$status', '0')
    raw = os.path.join(ss.outDir, 'raw.in')
    open(raw, 'wb').write(b'x\xc3\xa9\xa9y\n')
    L('!force 0 delete-buffer "*t*"')
    L('find-file "%s"' % raw)
    L('1 buffer-mode "exact"')
    L('-1 buffer-mode "magic"')
    # a byte in a pattern is the char of that value (here ©), a raw byte is found by [[:raw:]]
    L('!force search-forward "\\xa9"')
    ss.val('S6.rawlit', '$status', '0')
    L('beginning-of-line')
    L('1 buffer-mode "magic"')
    L('!force search-forward "\\\\xa9"')
    ss.val('S6.rawregex', '$status', '0')
    L('!force search-forward "[[:raw:]]"')
    ss.val('S6.rawclass', '&cat $status &cat "|" $window-col', '1|3')
    L('!force 0 delete-buffer $buffer-bname')

    # \xNN is U+00NN, so the high half of a Latin-1 file's chars match [\x80-\xff]
    lat = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'iso-8859-1.txt')
    # the Latin-1 code page, which codepage.emf sets at start-up
    L('set-char-mask "c" "%s"' % ''.join('%04X' % cc for cc in range(0x80, 0x100)))
    for tag, mode, pat in (('L1.class', 1, '[\\x80-\\xff]'), ('L1.regexesc', 1, '\\\\xa3'),
                           ('L1.literal', -1, '\\xa3'), ('L1.utf8', -1, '£')):
        L('!force 0 delete-buffer "iso-8859-1.txt"')
        L('find-file "%s"' % lat)
        L('%d buffer-mode "magic"' % mode)
        L('1 buffer-mode "exact"')
        L('!force search-forward "%s"' % pat)
        ss.val(tag, '&cat &cat $status "|" &cat $window-line &cat "|" $window-col', '1|12|1')
    L('!force 0 delete-buffer "iso-8859-1.txt"')
    ss.buf('aé\n')
    for mm in ('1 buffer-mode "magic"', '1 buffer-mode "exact"'):
        L(mm)
    L('!force search-forward "[\\x80-\\xff]"')
    ss.val('L1.whole', '&cat $status &cat "|" @s0', '1|é')

    # S3 - regex '.', classes and ranges match whole chars, quantifiers apply to a char
    magic = ['1 buffer-mode "magic"', '1 buffer-mode "exact"']
    for tag, txt, pat, grp, col in (
            ('S3.dot', 'xa中cy\n', 'a.c', 'a中c', 6),
            ('S3.dot2', 'x中文y\n', 'x..y', 'x中文y', 8),
            ('S3.class', 'zzé\n', '[àé]', 'é', 4),
            ('S3.range', 'ab слово cd\n', '[а-я]+', 'слово', 13),
            ('S3.negated', 'aaé\n', '[^a]', 'é', 4),
            ('S3.star', 'xééy\n', 'xé*y', 'xééy', 6),
            ('S3.interval', 'x中中中y\n', '中\\\\{2\\\\}', '中中', 7),
            ('S3.posix', 'ab Ж\n', '[[:upper:]]', 'Ж', 5),
            ('S3.class_word', 'a- ñu\n', '[\\\\w]+', 'a', 1),
            ('S4.word', '-- naïve --\n', '\\\\w+', 'naïve', 9),
            ('S4.alpha', '12 Ωμέγα 3\n', '\\\\a+', 'Ωμέγα', 13),
            ('S4.nonword', 'Ωμ, z\n', '\\\\W+', ', ', 6),
            ('S4.bow', 'xслово слово\n', '\\\\<слово', 'слово', 22),
            ('S4.eow', 'словоx слово\n', 'слово\\\\>', 'слово', 22),
            ('S4.boundary', 'aΩ Ω\n', '\\\\bΩ\\\\b', 'Ω', 6),
            ('S4.space', 'a　b\n', 'a\\\\sb', 'a　b', 5)):
        ss.buf(txt)
        for mm in magic:
            L(mm)
        L('!force search-forward "%s"' % pat)
        ss.val(tag, '&cat &cat $status "|" &cat @s0 &cat "|" $window-col', '1|%s|%d' % (grp, charCol(txt.rstrip('\n'), col)))
    # case insensitive regex
    for tag, txt, pat, grp in (('S2.regex', 'x école\n', 'ÉCO\\\\w+', 'école'),
                               ('S2.regexclass', 'x Б\n', '[а-я]', 'Б'),
                               ('S2.regexsigma', 'x λόγος\n', 'ΓΟΣ', 'γος')):
        ss.buf(txt)
        for mm in ('1 buffer-mode "magic"', '-1 buffer-mode "exact"'):
            L(mm)
        L('!force search-forward "%s"' % pat)
        ss.val(tag, '&cat $status &cat "|" @s0', '1|%s' % grp)
    # regex backwards
    ss.buf('a中b a中b\n')
    for mm in magic:
        L(mm)
    L('end-of-line')
    L('search-backward ".中"')
    ss.rep('S3.back', 1, 6, 5, 'a中b a中b')
    L('search-backward "[中]"')
    ss.rep('S3.back2', 1, 1, 1, 'a中b a中b')

    # S5 - replace ASCII with multi-byte and back, case changing replace of non-ASCII groups
    ss.buf('banana\n')
    for mm in lit:
        L(mm)
    L('replace-string "a" "中"')
    ss.val('S5.toutf8', '@wl', 'b中n中n中')
    L('beginning-of-buffer')
    L('replace-string "中n" "é"')
    ss.val('S5.fromutf8', '@wl', 'béé中')
    ss.buf('éCOLE ωμέγα ıx\n')
    for mm in magic:
        L(mm)
    L('replace-string "\\\\(\\\\w+\\\\) \\\\(\\\\w+\\\\) \\\\(\\\\w+\\\\)" "\\\\c1 \\\\u2 \\\\u3"')
    ss.val('S5.case', '@wl', 'École ΩΜΈΓΑ IX')
    L('set-variable #l0 &xrep "straße ǆ" "\\\\(.*\\\\)" "\\\\u1"')
    ss.val('S5.xrep', '#l0', 'STRAßE Ǆ')
    ss.buf('a中b\n')
    for mm in magic:
        L(mm)
    L('replace-string "中" "[\\\\{&,=,,4\\\\}]"')
    ss.val('S5.pad', '@wl', 'a[==中]b')


def main():
    if len(sys.argv) < 2:
        sys.exit('Usage: search.py <mec>')
    mec = os.path.abspath(sys.argv[1])
    wdir = tempfile.mkdtemp()
    outDir = os.path.join(wdir, 'out')
    os.mkdir(outDir)
    ss = Suite(outDir)
    ss.mac.append('define-macro start-up')
    build(ss)
    ss.mac += ['  4 exit-emacs 0', '!emacro']
    open(os.path.join(wdir, 'se.emf'), 'w', encoding='utf-8').write('\n'.join(ss.mac) + '\n')
    try:
        res = subprocess.run([mec, '-p', '@' + os.path.join(wdir, 'se')], stdin=subprocess.DEVNULL,
                             capture_output=True, env=dict(os.environ, MEPATH=wdir), timeout=120)
    except subprocess.TimeoutExpired:
        sys.exit('FAIL mec timed out - a prompt is probably waiting for input')
    log = (res.stdout + res.stderr).decode('utf-8', 'backslashreplace').replace('\r', '\n')
    got = {}
    # a tag can follow other output on the line
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
