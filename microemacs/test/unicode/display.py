#!/usr/bin/env python3
#
# display.py - Display tests: runs mec in a pseudo terminal and checks the emulated screen
# (acceptance tests D1-D7, D11, E7 and F27).
#
# Usage: python3 -I display.py <mec> [macros-dir]

import os
import re
import shutil
import sys
import tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from screen import Mec, mecEnv, charWidth  # noqa: E402

TDIR = os.path.dirname(os.path.abspath(__file__))
CORPUS = os.path.join(TDIR, 'corpus')

passed = failed = 0


def check(name, cond, detail=''):
    global passed, failed
    if cond:
        passed += 1
    else:
        failed += 1
        print('FAIL %s %s' % (name, detail))


def width(text):
    return sum(charWidth(ch) for ch in text)


def rowText(mm, row):
    # continuation cells are '' so joining gives the characters as typed
    return ''.join(mm.screen.cells[row]).rstrip()


def run(mec, macros, args, lang='C.UTF-8', keys=()):
    home = tempfile.mkdtemp()
    mm = Mec(mec, args, env=mecEnv(home, macros, lang))
    for kk in keys:
        mm.keys(kk)
    return mm, home


def finish(mm, home):
    mm.close()
    shutil.rmtree(home, ignore_errors=True)


def main():
    if len(sys.argv) < 2:
        sys.exit('Usage: display.py <mec> [macros-dir]')
    mec = os.path.abspath(sys.argv[1])
    macros = os.path.abspath(sys.argv[2] if len(sys.argv) > 2 else os.path.join(TDIR, '..', '..', 'macros'))
    utf8 = open(os.path.join(CORPUS, 'utf8.txt'), encoding='utf-8').read().split('\n')

    # D1/D2/D4 - text, wide chars and tabs on a UTF-8 terminal
    mm, home = run(mec, macros, [os.path.join(CORPUS, 'utf8.txt')])
    for ii in range(3):
        check('D1 row %d' % (ii + 1), rowText(mm, ii + 1) == utf8[ii], repr(rowText(mm, ii + 1)))
    cjk = utf8[3]
    check('D2 cjk text', rowText(mm, 4) == cjk, repr(rowText(mm, 4)))
    cells = mm.screen.cells[4]
    check('D2 wide cells', cells[0] == '中' and cells[1] == '' and cells[2] == '文', repr(cells[:4]))
    tabline = mm.screen.cells[7]
    col = ''.join(cc if cc else '\0' for cc in tabline).find('Ambig')
    check('D4 tab after wide char', col == 24, 'Ambig at %d' % col)
    check('D11 zero width shown', '\\u200B' in rowText(mm, 7), repr(rowText(mm, 7)))
    check('D11 combining shown', 'e\\u0301' in rowText(mm, 6), repr(rowText(mm, 6)))
    # cursor columns - end of the CJK line, then back over the wide chars with the column count
    mm.keys(b'\x0e\x0e\x0e\x05')
    check('D2 cursor at end of wide line', mm.cursor == (4, width(cjk)), 'cursor %r expected %r' % (mm.cursor, (4, width(cjk))))
    finish(mm, home)

    # E7 - the mode line column %c counts chars
    mm, home = run(mec, macros, [os.path.join(CORPUS, 'utf8.txt')])
    mm.keys(b'\x1bxset-variable\r$mode-line\rMLCOL=%c=\r')
    mm.keys(b'\x0e\x0e\x0e\x05')
    mline = [rowText(mm, rr) for rr in range(mm.screen.rows) if rowText(mm, rr).startswith('MLCOL=')]
    check('E7 mode line char column', mline and mline[0].startswith('MLCOL=%d=' % len(cjk)), repr(mline))
    finish(mm, home)

    # D3 - wide chars at the right edge '$' and with a horizontal scroll
    tdir = tempfile.mkdtemp()
    fname = os.path.join(tdir, 'wide.txt')
    open(fname, 'w', encoding='utf-8').write('中' * 50 + '\n' + 'x' + '中' * 50 + '\n')
    mm, home = run(mec, macros, [fname])
    cols = mm.screen.cols
    even, odd = mm.screen.cells[1], mm.screen.cells[2]
    # a wide char cut by the '$' has its other half shown as a '$' too
    check('D3 even wide line', ''.join(even) == '中' * ((cols - 2) // 2) + '$$', repr(''.join(even)[-6:]))
    check('D3 odd wide line', ''.join(odd) == 'x' + '中' * ((cols - 2) // 2) + '$', repr(''.join(odd)[-6:]))
    mm.keys(b'\x05')
    row = mm.screen.cells[1]
    aligned = all((row[ii] != '') or (ii > 0 and charWidth(row[ii - 1] or ' ') == 2) for ii in range(cols))
    last = max(ii for ii in range(cols) if row[ii] not in (' ',))
    check('D3 scrolled line', row[0] == '$' and aligned, repr(''.join(cc if cc else '.' for cc in row)))
    # a wide char cut by the left '$' shows a second '$', never a blank
    check('D3 scrolled left edge', row[1] != ' ', repr(''.join(cc if cc else '.' for cc in row[:4])))
    check('D3 scrolled cursor', mm.cursor == (1, last + 1), 'cursor %r last %d' % (mm.cursor, last))
    finish(mm, home)
    shutil.rmtree(tdir, ignore_errors=True)

    # D6 - NUL, control chars and invalid bytes
    mm, home = run(mec, macros, [os.path.join(CORPUS, 'nul.txt')])
    check('D6 NUL shown as ^@', rowText(mm, 1).startswith('NUL[^@] two[^@^@]'), repr(rowText(mm, 1)))
    finish(mm, home)
    mm, home = run(mec, macros, [os.path.join(CORPUS, 'control_chars.txt')])
    check('D6 control chars', rowText(mm, 1) == 'SOH[^A] STX[^B] BEL[^G] ESC[^[] DEL[\\x7F]', repr(rowText(mm, 1)))
    finish(mm, home)
    mm, home = run(mec, macros, [os.path.join(CORPUS, 'utf8_late_invalid.txt')], keys=[b'\x1b>'])
    late = [rr for rr in range(1, 22) if 'late[' in rowText(mm, rr)]
    check('D6 invalid byte shown as \\xE9', late and rowText(mm, late[0]) == 'late[\\xE9]',
          repr(rowText(mm, late[0])) if late else 'line not found')
    finish(mm, home)

    # mode line with a UTF-8 file name
    tdir = tempfile.mkdtemp()
    fname = os.path.join(tdir, 'größe中.txt')
    open(fname, 'w', encoding='utf-8').write('x\n')
    mm, home = run(mec, macros, [fname])
    mline = [rr for rr in range(mm.screen.rows) if rowText(mm, rr).startswith('==')]
    check('mode line UTF-8 name', mline and ('größe中.txt' in rowText(mm, mline[0])),
          repr(rowText(mm, mline[0])) if mline else 'no mode line')
    # the long path is cut to its end, which must exactly fill the line
    check('mode line fills the width', mline and (width(''.join(mm.screen.cells[mline[0]])) == mm.screen.cols)
          and ''.join(mm.screen.cells[mline[0]]).endswith('größe中.txt'),
          repr(''.join(mm.screen.cells[mline[0]])) if mline else '')
    finish(mm, home)
    shutil.rmtree(tdir, ignore_errors=True)

    # %e - the encoding on the mode line, its $buffer-encoding name, after the modes as "%O %- %e"
    for name, enc in (('utf16le_bom.txt', ' - utf16le-bom)'), ('latin1.txt', ' - iso8859-1)'), ('utf8.txt', ' - utf8)')):
        mm, home = run(mec, macros, [os.path.join(CORPUS, name)])
        mline = [rowText(mm, rr) for rr in range(mm.screen.rows) if rowText(mm, rr).startswith('==')]
        check('mode line encoding %s' % name, mline and (enc in mline[0]), repr(mline[0]) if mline else 'no mode line')
        finish(mm, home)

    # D5 - UTF-8 in a hilighted file, hilighting must not split chars
    tdir = tempfile.mkdtemp()
    fname = os.path.join(tdir, 'hil.c')
    src = ['/* café 中文 naïve */', 'char *s = "größe 日本語";', 'int x; /* end ☺ */']
    open(fname, 'w', encoding='utf-8').write('\n'.join(src) + '\n')
    mm, home = run(mec, macros, [fname])
    for ii, line in enumerate(src):
        check('D5 hilighted row %d' % (ii + 1), rowText(mm, ii + 1) == line, repr(rowText(mm, ii + 1)))
    mm.keys(b'\x0e\x05')
    check('D5 hilighted cursor', mm.cursor == (2, width(src[1])), 'cursor %r expected %r' % (mm.cursor, (2, width(src[1]))))
    finish(mm, home)
    shutil.rmtree(tdir, ignore_errors=True)

    # D7 - code page terminal: Latin-1 chars arrive as code page bytes, others as '?'
    mm, home = run(mec, macros, [os.path.join(CORPUS, 'utf8.txt')], lang='C')
    mm.screen.encoding = 'latin-1'
    # redraw with the emulator decoding Latin-1
    mm.screen.__init__(mm.screen.rows, mm.screen.cols)
    mm.screen.encoding = 'latin-1'
    mm.keys(b'\x0c')
    check('D7 latin-1 row', rowText(mm, 1) == utf8[0].replace('–', '?').replace('“', '?').replace('”', '?'),
          repr(rowText(mm, 1)))
    check('D7 wide char padded', rowText(mm, 4).startswith('? ? ? ? '), repr(rowText(mm, 4)))
    finish(mm, home)

    # F27 - the load message lists the conditions found
    expect = {
        'nul.txt': r'\[Read \d+ lines? - %d NULs\]' % open(os.path.join(CORPUS, 'nul.txt'), 'rb').read().count(b'\0'),
        'eol_mixed_lf.txt': r'\[Read \d+ lines - mixed line endings\]',
        'utf8_noeol.txt': r'\[Read \d+ lines - no final newline\]',
        'utf8_late_invalid.txt': r'\[Read \d+ lines - 1 invalid byte\]',
        'utf8_long_line.txt': r'\[Read \d+ lines? - 1 long line split\]',
        'utf16le_bom.txt': r'\[Read \d+ lines - utf16le-bom\]',
        'latin1.txt': r'\[Read \d+ lines - iso8859-1\]',
        'utf8_bom.txt': r'\[Read \d+ lines - utf8-bom\]',
        'utf8.txt': r'\[Read \d+ lines\]',
    }
    for name, pat in expect.items():
        mm, home = run(mec, macros, [os.path.join(CORPUS, name)])
        msgs = re.findall(rb'\[Read [^\]]*\]', mm.raw)
        msg = msgs[0].decode('utf-8', 'replace') if msgs else ''
        check('F27 %s' % name, re.fullmatch(pat, msg) is not None, repr(msg))
        finish(mm, home)

    # screen-poke text is UTF-8, a wide char takes 2 cells and is a space when cut by an edge;
    # with per char schemes (4) the scheme string has a char per char
    mm, home = run(mec, macros, [os.path.join(CORPUS, 'utf8.txt')])
    for line in ('screen-poke 10 2 0 "é中x"', '5 screen-poke 11 0 "\\x01\\x02\\x01" "中éy"',
                 'screen-poke 12 78 0 "z中"', 'screen-poke 13 -1 0 "中x"'):
        mm.keys(b'\x1bxexecute-line\r' + line.encode('utf-8') + b'\r')
    check('poke text', mm.screen.cells[10][:6] == [' ', ' ', 'é', '中', '', 'x'], repr(mm.screen.cells[10][:8]))
    check('poke schemes', mm.screen.cells[11][:4] == ['中', '', 'é', 'y'], repr(mm.screen.cells[11][:6]))
    check('poke right edge', mm.screen.cells[12][-2:] == ['z', ' '], repr(mm.screen.cells[12][-4:]))
    check('poke left edge', mm.screen.cells[13][:2] == [' ', 'x'], repr(mm.screen.cells[13][:4]))
    finish(mm, home)

    # a poke over half of a wide char on screen blanks its other half, on the screen and in the
    # frame store (read by @fs, and by the window builds to repaint and draw the cursor), and the
    # line is drawn back whole when next redrawn
    tdir = tempfile.mkdtemp()
    wide = os.path.join(tdir, 'wide.txt')
    open(wide, 'wb').write('中中中中\n'.encode('utf-8'))
    for name, line, cells in (('start', 'screen-poke 1 1 0 "x"', [' ', 'x', '中', '']),
                              ('end', 'screen-poke 1 2 0 "x"', ['中', '', 'x', ' '])):
        mm, home = run(mec, macros, [wide])
        mm.keys(b'\x1bxexecute-line\r' + line.encode('utf-8') + b'\r')
        check('poke wide %s screen' % name, mm.screen.cells[1][:4] == cells, repr(mm.screen.cells[1][:6]))
        mm.keys(b'\x1bxexecute-line\r' + 'ml-write &cat "fs=[" &cat @fs 1 0 &cat @fs 1 1 &cat @fs 1 2 &cat @fs 1 3 "]"'.encode('utf-8') + b'\r')
        ml = mm.screen.text(mm.screen.rows - 1)
        want = 'fs=[%s]' % ''.join(cells)
        check('poke wide %s store' % name, want in ml, repr(ml.rstrip()))
        mm.keys(b'\x05z')
        check('poke wide %s redraw' % name, mm.screen.cells[1][:9] == ['中', ''] * 4 + ['z'], repr(mm.screen.cells[1][:10]))
        finish(mm, home)
    shutil.rmtree(tdir, ignore_errors=True)

    print('%d passed, %d failed' % (passed, failed))
    sys.exit(1 if failed else 0)


if __name__ == '__main__':
    main()
