#!/usr/bin/env python3
#
# edit.py - Editing tests (Phase 3): runs a headless mec over scratch buffers and checks the
# text, the char index ($window-col) and display column ($window-acol) after each command
# (acceptance tests E1-E10, E13-E18, E21).
#
# Usage: python3 -I edit.py <mec> [--ascii]
#
# --ascii runs only the ASCII cases, for comparing against an 8-bit build.

import os
import subprocess
import sys
import tempfile


def charCol(text, col):
    """the char index of byte offset col in text"""
    return len(text.encode('utf-8')[:col].decode('utf-8'))


class Suite:
    def __init__(self, outDir):
        self.mac = []
        self.expect = {}
        self.files = {}
        self.outDir = outDir
        self.ascii = []

    def line(self, ll):
        self.mac.append('  ' + ll)

    def buf(self, text, ascii=False):
        # a fresh buffer holding text with dot at its start
        self.line('!force 0 delete-buffer "*t*"')
        self.line('find-buffer "*t*"')
        self.line('insert-string "%s"' % text.replace('\\', '\\\\').replace('"', '\\"').replace('\n', '\\n'))
        self.line('beginning-of-buffer')

    def rep(self, tag, line, col, acol, text):
        # line is 1 based, col the byte offset in text's UTF-8 (reported as $window-col, a char
        # index), acol the display column
        self.line('-2 ml-write &spr "R:%%s=%%d|%%d|%%d|%%s" "%s" $window-line $window-col $window-acol @wl' % tag)
        self.expect[tag] = '%d|%d|%d|%s' % (line, charCol(text, col), acol, text)

    def val(self, tag, expr, want):
        self.line('-2 ml-write &spr "R:%%s=%%s" "%s" %s' % (tag, expr))
        self.expect[tag] = want

    def save(self, tag, want):
        self.line('!force 0 write-buffer "%s"' % os.path.join(self.outDir, tag))
        self.files[tag] = want

    def sep(self):
        # separates undo steps, as each key press would
        self.line('-2 undo')


def build(ss, asciiOnly):
    L = ss.line

    # E1 forward/backward char
    if not asciiOnly:
        ss.buf('aé中😀b')
        for ii, (col, acol) in enumerate(((1, 1), (3, 2), (6, 4), (10, 6), (11, 7))):
            L('forward-char')
            ss.rep('E1.fwd%d' % ii, 1, col, acol, 'aé中😀b')
        for ii, (col, acol) in enumerate(((10, 6), (6, 4), (3, 2), (1, 1), (0, 0))):
            L('backward-char')
            ss.rep('E1.bwd%d' % ii, 1, col, acol, 'aé中😀b')
        L('3 forward-char')
        ss.rep('E1.arg', 1, 6, 4, 'aé中😀b')
        ss.buf('é\nx')
        L('forward-char')
        L('forward-char')
        ss.rep('E1.nl', 2, 0, 0, 'x')
        L('backward-char')
        ss.rep('E1.nlback', 1, 2, 1, 'é')
    ss.buf('abc\nd')
    L('forward-char')
    L('forward-char')
    L('forward-char')
    L('forward-char')
    ss.rep('E1.ascii', 2, 0, 0, 'd')
    ss.ascii.append('E1.ascii')

    # E2 goal column across wide chars
    if not asciiOnly:
        ss.buf('abcdef\n中文字\nabcdef')
        L('set-variable $window-col 4')
        L('forward-line')
        ss.rep('E2.down1', 2, 6, 4, '中文字')
        L('forward-line')
        ss.rep('E2.down2', 3, 4, 4, 'abcdef')
        # a report between line moves resets the goal column, so test the goal without one
        L('beginning-of-buffer')
        L('set-variable $window-col 3')
        L('forward-line')
        ss.rep('E2.mid', 2, 6, 4, '中文字')
        L('beginning-of-buffer')
        L('set-variable $window-col 3')
        L('forward-line')
        L('forward-line')
        ss.rep('E2.goal', 3, 3, 3, 'abcdef')
        # $window-acol uses the same column to offset code as the mouse (E11)
        L('backward-line')
        L('set-variable $window-acol 2')
        ss.rep('E2.acol', 2, 3, 2, '中文字')

    # E3 delete whole chars
    if not asciiOnly:
        ss.buf('aé中😀b')
        L('forward-char')
        L('forward-delete-char')
        ss.rep('E3.fwd', 1, 1, 1, 'a中😀b')
        L('2 forward-delete-char')
        ss.rep('E3.fwd2', 1, 1, 1, 'ab')
        ss.buf('aé中b')
        L('end-of-line')
        L('backward-char')
        L('backward-delete-char')
        ss.rep('E3.back', 1, 3, 2, 'aéb')
        L('backward-delete-char')
        ss.rep('E3.back2', 1, 1, 1, 'ab')
        L('-1 forward-delete-char')
        ss.rep('E3.neg', 1, 0, 0, 'b')

    # E4 overwrite, 1 byte typed over multi-byte chars and @wc replacing with longer chars
    if not asciiOnly:
        ss.buf('é中😀b')
        L('1 buffer-mode "over"')
        L('execute-string "xyz"')
        ss.rep('E4.over', 1, 3, 3, 'xyzb')
        L('-1 buffer-mode "over"')
        L('beginning-of-line')
        L('set-variable @wc "😀"')
        ss.rep('E4.wc4', 1, 0, 0, '😀yzb')
        L('set-variable @wc "é"')
        ss.rep('E4.wc2', 1, 0, 0, 'éyzb')
        L('forward-char')
        ss.val('E4.wcget', '@wc', 'y')
        L('backward-char')
        ss.val('E4.wcget2', '@wc', 'é')

    # E5 words with accented, Greek, Cyrillic and CJK letters
    if not asciiOnly:
        words = 'café naïve Ωμέγα слово 中文 x'
        ss.buf(words)
        for ii, col in enumerate((5, 12, 23, 34, 41)):
            L('forward-word')
            ss.rep('E5.fwd%d' % ii, 1, col, len(words[:[4, 10, 16, 22, 25][ii]]) + (2 if ii == 4 else 0), words)
        L('backward-word')
        ss.rep('E5.back1', 1, 35, 23, words)
        L('backward-word')
        ss.rep('E5.back2', 1, 24, 17, words)
        ss.buf(words)
        L('forward-kill-word')
        ss.rep('E5.kill', 1, 0, 0, words[4:])
        L('end-of-line')
        L('3 backward-kill-word')
        ss.rep('E5.bkill', 1, 19, 13, ' naïve Ωμέγα ')

    # E6 case conversion, including chars which change length
    if not asciiOnly:
        ss.buf('straße ωμέγα слово ǆemal')
        L('4 upper-case-word')
        ss.rep('E6.upper', 1, 36, 24, 'STRAßE ΩΜΈΓΑ СЛОВО ǄEMAL')
        ss.buf('ÉCOLE ΑΘΗΝΑ')
        L('2 lower-case-word')
        ss.rep('E6.lower', 1, 17, 11, 'école αθηνα')
        ss.buf('élan ωμέγα')
        L('2 capitalize-word')
        ss.rep('E6.cap', 1, 16, 10, 'Élan Ωμέγα')
        # dotless i and long s shrink to 1 byte, the Kelvin sign is upper case already
        ss.buf('\u0131\u017f\u212a x')
        L('upper-case-word')
        ss.rep('E6.shrink', 1, 5, 3, 'IS\u212a x')
        # the Kelvin sign shrinks to k, A with stroke grows to 3 bytes
        ss.buf('\u212a\u023a x')
        L('lower-case-word')
        ss.rep('E6.grow', 1, 4, 2, 'k\u2c65 x')
        # region: mark at the start, dot at the end, then the other way round
        orig = 'ab \u0131\u017f\u023a \u212a cd'

        def regionCase(report):
            ss.buf(orig)
            L('3 forward-char')
            L('set-mark')
            L('5 forward-char')
            ss.sep()
            L('upper-case-region')
            if report:
                ss.rep('E6.rup', 1, 11, 8, 'ab IS\u023a \u212a cd')
            ss.sep()
            L('lower-case-region')
            if report:
                ss.rep('E6.rlow', 1, 10, 8, 'ab is\u2c65 k cd')
            L('exchange-point-and-mark')
            if report:
                ss.rep('E6.rmark', 1, 3, 3, 'ab is\u2c65 k cd')
            ss.sep()
            L('upper-case-region')
            if report:
                ss.rep('E6.rdot', 1, 3, 3, 'ab IS\u023a K cd')
                L('exchange-point-and-mark')
                ss.rep('E6.rdot2', 1, 9, 8, 'ab IS\u023a K cd')
        regionCase(True)
        # undo restores the original bytes a command at a time, a report between undos would
        # break the undo chain so each count starts again
        for ii, text in enumerate(('ab is\u2c65 k cd', 'ab IS\u023a \u212a cd', orig)):
            regionCase(False)
            ss.sep()
            L('%d undo' % (ii + 1))
            ss.val('E6.undo%d' % ii, '@wl', text)

    # E8 undo of single char deletes is a char per step
    if not asciiOnly:
        for ii, text in enumerate(('a', 'aé', 'aé中', 'aé中😀', 'aé中😀b')):
            ss.buf('aé中😀b')
            L('end-of-line')
            for jj in range(4):
                ss.sep()
                L('backward-delete-char')
            if ii:
                ss.sep()
                L('%d undo' % ii)
            ss.val('E8.del%d' % ii, '@wl', text)
        for ii, text in enumerate(('ab', 'a😀b', 'a中😀b', 'aé中😀b')):
            ss.buf('aé中😀b')
            L('forward-char')
            for jj in range(3):
                ss.sep()
                L('forward-delete-char')
            if ii:
                ss.sep()
                L('%d undo' % ii)
            ss.val('E8.fdel%d' % ii, '@wl', text)
        # redo, the steps of one undo command are redone together as in the 8-bit build
        ss.sep()
        L('undo')
        ss.val('E8.redo', '@wl', 'ab')
        # typed bytes forming a char, undone in one step
        for ii, text in enumerate(('xé中', 'xé', 'x')):
            ss.buf('x')
            L('end-of-line')
            ss.sep()
            L('execute-string "é"')
            ss.sep()
            L('execute-string "中"')
            if ii:
                ss.sep()
                L('%d undo' % ii)
            ss.val('E8.typed%d' % ii, '@wl', text)
        # and redone
        for ii, nn in enumerate((1, 2)):
            ss.buf('x')
            L('end-of-line')
            ss.sep()
            L('execute-string "é"')
            ss.sep()
            L('execute-string "中"')
            ss.sep()
            L('%d undo' % nn)
            ss.sep()
            L('undo')
            ss.val('E8.tredo%d' % ii, '@wl', 'xé中')

    # E9 kill and yank are byte exact
    if not asciiOnly:
        ss.buf('x中文é😀\nsecond ñ\ny')
        L('forward-char')
        L('set-mark')
        L('forward-line')
        L('end-of-line')
        L('kill-region')
        ss.val('E9.kill', '@wl', 'x')
        L('yank')
        L('yank')
        ss.save('E9.yank', 'x中文é😀\nsecond ñ中文é😀\nsecond ñ\ny\n')

    # E10 marks stay on the same char when a char before them changes length
    if not asciiOnly:
        ss.buf('ıab')
        L('forward-char')
        L('forward-char')
        L('set-mark')
        L('beginning-of-line')
        L('set-variable @wc "I"')
        L('exchange-point-and-mark')
        ss.rep('E10.after', 1, 2, 2, 'Iab')
        ss.buf('ıab')
        L('forward-char')
        L('set-mark')
        L('beginning-of-line')
        L('set-variable @wc "😀"')
        L('exchange-point-and-mark')
        ss.rep('E10.next', 1, 4, 2, '😀ab')

    # E13 transpose-chars
    if not asciiOnly:
        ss.buf('aé中')
        L('3 forward-char')
        L('backward-char')
        ss.sep()
        L('transpose-chars')
        ss.rep('E13.mid', 1, 4, 3, 'a中é')
        L('end-of-line')
        ss.sep()
        L('transpose-chars')
        ss.rep('E13.eol', 1, 6, 4, 'aé中')
        ss.sep()
        L('undo')
        ss.val('E13.undo', '@wl', 'a中é')

    # E14 rectangles use display columns
    if not asciiOnly:
        ss.buf('ab中cd\nabcdef\n中文字')
        L('set-variable $window-acol 2')
        L('set-mark')
        L('2 forward-line')
        L('set-variable $window-acol 4')
        L('kill-rectangle')
        ss.save('E14.kill', 'abcd\nabef\n中字\n')
        L('beginning-of-buffer')
        L('set-variable $window-acol 2')
        L('yank-rectangle')
        ss.save('E14.yank', 'ab中cd\nabcdef\n中文字\n')

    # E15 fill-paragraph measures display width
    if not asciiOnly:
        ss.buf('éé éé éé éé éé\n')
        L('set-variable $buffer-fill-col 10')
        L('set-variable $buffer-fill-mode "l"')
        L('-1 buffer-mode "justify"')
        L('fill-paragraph')
        ss.save('E15.fill', 'éé éé éé\néé éé\n')

    # E16 indentation with multi-byte text before point
    if not asciiOnly:
        ss.buf('中x')
        L('forward-char')
        L('1 buffer-mode "tab"')
        L('set-variable $buffer-indent-width 4')
        L('tab')
        ss.rep('E16.tab', 1, 5, 4, '中  x')
        ss.buf('中x')
        L('forward-char')
        L('-1 buffer-mode "tab"')
        L('set-variable $buffer-tab-width 8')
        L('tab')
        ss.rep('E16.tabchar', 1, 4, 8, '中\tx')

    # E17 count-words and buffer-info report chars and bytes
    if not asciiOnly:
        ss.buf('café 中文 😀\nx')
        L('2 count-words')
        ss.val('E17.count', '$result', '|3|2.3|12|3|20|')
        L('forward-word')
        L('0 buffer-info')
        ss.val('E17.info', '&lget $result 3', '5')
        ss.val('E17.infoc', '&lget $result 13', '4')
        ss.val('E17.infob', '&lget $result 16', '12')

    # E18 sort-lines folds case for non-ASCII
    if not asciiOnly:
        ss.buf('zebra\néclair\nÉclair\napple\nÀ la\nZulu\nzoo\n')
        L('-1 buffer-mode "exact"')
        L('set-mark')
        L('end-of-buffer')
        L('sort-lines')
        ss.save('E18.fold', 'apple\nzebra\nzoo\nZulu\nÀ la\nÉclair\néclair\n')
        L('1 buffer-mode "exact"')
        L('beginning-of-buffer')
        L('set-mark')
        L('end-of-buffer')
        L('sort-lines')
        ss.save('E18.exact', 'Zulu\napple\nzebra\nzoo\nÀ la\nÉclair\néclair\n')
    ss.buf('b\nA\nc\n')
    L('-1 buffer-mode "exact"')
    L('set-mark')
    L('end-of-buffer')
    L('sort-lines')
    ss.save('E18.ascii', 'A\nb\nc\n')
    ss.ascii.append('E18.ascii')

    # E21 sort-lines from a column counts chars; lines shorter than the column go first, sorted
    # from column 0 (backwards for a negative argument)
    if not asciiOnly:
        ss.buf('éa3\nab1\n中x2\nzy\nq\n')
        L('1 buffer-mode "exact"')
        L('set-mark')
        L('end-of-buffer')
        L('2 sort-lines')
        ss.save('E21.col', 'q\nzy\nab1\n中x2\néa3\n')
        ss.buf('éa3\nb\nab1\n中x2\nzy\nq\n')
        L('1 buffer-mode "exact"')
        L('set-mark')
        L('end-of-buffer')
        L('-3 sort-lines')
        ss.save('E21.back', 'q\nb\néa3\n中x2\nab1\nzy\n')
    ss.buf('ab3\nb\ncd1\nef2\nzy\nq\n')
    L('1 buffer-mode "exact"')
    L('set-mark')
    L('end-of-buffer')
    L('2 sort-lines')
    ss.save('E21.ascii', 'b\nq\nzy\ncd1\nef2\nab3\n')
    ss.ascii.append('E21.ascii')
    # the short lines differ in length, as the old reverse sort compared them from the byte before
    # their text (the line's spare space)
    ss.buf('abc3\nb\nabc1\nabc2\nzy\nqq\n')
    L('1 buffer-mode "exact"')
    L('set-mark')
    L('end-of-buffer')
    L('-4 sort-lines')
    ss.save('E21.asciiback', 'zy\nqq\nb\nabc3\nabc2\nabc1\n')
    ss.ascii.append('E21.asciiback')

    # $window-col is a char index
    if not asciiOnly:
        ss.buf('a中b')
        L('set-variable $window-col 2')
        ss.rep('Col.char', 1, 4, 3, 'a中b')
        L('!force set-variable $window-col 4')
        ss.rep('Col.past', 1, 5, 4, 'a中b')

    # compare-windows compares chars, stopping at the start of the first that differs
    if not asciiOnly:
        L('!force 0 delete-buffer "*t2*"')
        L('find-buffer "*t2*"')
        L('insert-string "x中é1"')
        L('beginning-of-buffer')
        ss.buf('x中è1')
        L('split-window-vertically')
        L('next-window')
        L('find-buffer "*t2*"')
        L('beginning-of-buffer')
        L('next-window')
        L('!force 0 compare-windows')
        ss.val('Cmp.status', '$status', '0')
        ss.rep('Cmp.pos', 1, 4, 3, 'x中è1')
        L('delete-other-windows')

    # auto wrap while typing measures display columns
    if not asciiOnly:
        ss.buf('')
        L('1 buffer-mode "wrap"')
        L('set-variable $buffer-fill-col 10')
        L('execute-string "éééé éééé éééé x"')
        L('-1 buffer-mode "wrap"')
        # the same layout as ASCII, which also keeps the space before the break
        ss.save('Wrap', 'éééé éééé \néééé x\n')

    # raw bytes and NUL step as single chars
    if not asciiOnly:
        raw = os.path.join(ss.outDir, 'raw.in')
        open(raw, 'wb').write(b'x\xc3\xa9\xe9y\x00z\n')
        L('!force 0 delete-buffer "*t*"')
        L('find-file "%s"' % raw)
        for ii, (col, acol) in enumerate(((1, 1), (2, 2), (3, 6), (4, 7), (5, 9), (6, 10))):
            L('forward-char')
            ss.line('-2 ml-write &spr "R:%%s=%%d|%%d" "raw%d" $window-col $window-acol' % ii)
            ss.expect['raw%d' % ii] = '%d|%d' % (col, acol)
        L('2 backward-delete-char')
        L('!force 0 write-buffer "%s"' % os.path.join(ss.outDir, 'raw.del'))
        ss.files['raw.del'] = None
        ss.rawWant = b'x\xc3\xa9\xe9y\n'


def main():
    if len(sys.argv) < 2:
        sys.exit('Usage: edit.py <mec> [--ascii]')
    mec = os.path.abspath(sys.argv[1])
    asciiOnly = '--ascii' in sys.argv
    wdir = tempfile.mkdtemp()
    outDir = os.path.join(wdir, 'out')
    os.mkdir(outDir)
    ss = Suite(outDir)
    ss.rawWant = None
    ss.mac.append('define-macro start-up')
    ss.line('1 global-mode "undo"')
    build(ss, asciiOnly)
    ss.mac += ['  4 exit-emacs 0', '!emacro']
    open(os.path.join(wdir, 'ed.emf'), 'w', encoding='utf-8').write('\n'.join(ss.mac) + '\n')
    try:
        res = subprocess.run([mec, '-p', '@' + os.path.join(wdir, 'ed')], stdin=subprocess.DEVNULL,
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
        if asciiOnly and tag not in ss.ascii:
            continue
        if got.get(tag) == want:
            passed += 1
        else:
            failed += 1
            print('FAIL %s: got %r expected %r' % (tag, got.get(tag), want))
    for tag, want in ss.files.items():
        if asciiOnly and tag not in ss.ascii:
            continue
        path = os.path.join(outDir, tag)
        if want is None:
            want = ss.rawWant
        else:
            want = want.encode('utf-8')
        data = open(path, 'rb').read() if os.path.exists(path) else None
        if data == want:
            passed += 1
        else:
            failed += 1
            print('FAIL %s: got %r expected %r' % (tag, data, want))
    print('%d passed, %d failed' % (passed, failed))
    sys.exit(1 if failed else 0)


if __name__ == '__main__':
    main()
