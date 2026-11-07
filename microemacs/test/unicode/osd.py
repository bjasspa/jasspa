#!/usr/bin/env python3
#
# osd.py - OSD tests (X8): runs mec in a pseudo terminal and checks a dialog and an entry with
# non-ASCII text are laid out in cells, a non-ASCII hot key works and typing into an entry
# keeps the cursor on the right cell, and insert-symbol lists the code page's chars aligned.
#
# Usage: python3 -I osd.py <mec> [macros-dir]

import os
import shutil
import sys
import tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from screen import Mec, mecEnv  # noqa: E402

TDIR = os.path.dirname(os.path.abspath(__file__))

passed = failed = 0


def check(name, cond, detail=''):
    global passed, failed
    if cond:
        passed += 1
    else:
        failed += 1
        print('FAIL %s %s' % (name, detail))


def rowText(mm, row):
    return ''.join(mm.screen.cells[row]).rstrip()


def run(mm, line):
    mm.keys(b'\x1bxexecute-line\r', 1.0)
    mm.keys(line.encode('utf-8') + b'\r', 2.0)


def main():
    if len(sys.argv) < 2:
        sys.exit('Usage: osd.py <mec> [macros-dir]')
    mec = os.path.abspath(sys.argv[1])
    macros = os.path.abspath(sys.argv[2] if len(sys.argv) > 2 else os.path.join(TDIR, '..', '..', 'macros'))
    home = tempfile.mkdtemp()
    mm = Mec(mec, [], env=mecEnv(home, macros))
    mm.keys(b'\x07', 1.0)

    run(mm, 'osd-dialog "Größe 中文" "  Der Bär läuft 中文字  " "  \\HÖk  "')
    rows = [rowText(mm, rr) for rr in range(8)]
    check('dialog title', 'Größe 中文' in rows[0], repr(rows[0]))
    text = [rr for rr in range(8) if 'Der Bär' in rows[rr]]
    check('dialog text', text and rows[text[0]].strip() == '|  Der Bär läuft 中文字  |', repr(rows[text[0]] if text else rows))
    # the right border lines up with the border rows, so the text was measured in cells
    if text:
        line = mm.screen.cells[text[0]]
        border = mm.screen.cells[text[0] - 1]
        check('dialog border aligned', line.index('|', 1) == border.index('|', 1),
              '%d %d' % (line.index('|', 1), border.index('|', 1)))
    check('dialog hot key drawn', any('Ök' in rr for rr in rows), repr(rows))
    mm.keys('ö'.encode(), 1.5)
    check('non-ASCII hot key closes the dialog', not any('Der Bär' in rowText(mm, rr) for rr in range(8)),
          repr([rowText(mm, rr) for rr in range(8)]))

    # tab moves to the next button and return runs it, a hot key can be any char
    run(mm, 'osd-dialog "T" "  Msg  " "  \\HYes  " "  \\HNo  "')
    mm.keys(b'\t', 1.0)
    check('tab moves the focus', any('>  No  <' in rowText(mm, rr) for rr in range(8)),
          repr([rowText(mm, rr) for rr in range(8)]))
    mm.keys(b'\r', 1.0)
    check('return closes the dialog', not any('Msg' in rowText(mm, rr) for rr in range(8)),
          repr([rowText(mm, rr) for rr in range(8)]))
    run(mm, 'insert-string &cat "R=" $result')
    check('return ran the focused button', any('R=2' in rowText(mm, rr) for rr in range(1, 22)),
          repr([rowText(mm, rr) for rr in range(1, 4)]))
    run(mm, 'osd-dialog "T" "  Msg  " "  \\H中  " "  \\HNo  "')
    mm.keys('中'.encode(), 1.5)
    check('wide hot key closes the dialog', not any('Msg' in rowText(mm, rr) for rr in range(8)),
          repr([rowText(mm, rr) for rr in range(8)]))

    run(mm, 'osd-entry "Titel" "Wert: " "é中xyz"')
    rows = [rowText(mm, rr) for rr in range(8)]
    ent = [rr for rr in range(8) if 'Wert:' in rows[rr]]
    check('entry text', ent and 'Wert: é中xyz' in rows[ent[0]], repr(rows))
    if ent:
        start = ''.join(cc if cc else '\0' for cc in mm.screen.cells[ent[0]]).index('é')
        check('entry cursor', mm.cursor == (ent[0], start + 6), 'cursor %r start %d' % (mm.cursor, start))
        mm.keys('ä文'.encode(), 1.5)
        check('entry typed', 'Wert: é中xyzä文' in rowText(mm, ent[0]), repr(rowText(mm, ent[0])))
        check('entry cursor after typing', mm.cursor == (ent[0], start + 9), 'cursor %r' % (mm.cursor,))
    mm.close()

    # insert-symbol - the buffer's code page, a char with no glyph is a '.', columns aligned
    open(os.path.join(home, 'ru.txt'), 'wb').write('Привет\n'.encode('cp1251'))
    mm = Mec(mec, [os.path.join(home, 'ru.txt')], rows=45, cols=100, env=mecEnv(home, macros))
    mm.keys(b'\x07', 1.0)
    run(mm, 'code-page-define "cp1251"')
    run(mm, 'set-variable $buffer-encoding "cp1251"')
    mm.keys(b'\x1bxexecute-line\r', 1.0)
    mm.keys(b'insert-symbol\r', 3.0)
    rows = [rowText(mm, rr) for rr in range(45)]
    sym = [rr for rr in rows if ('  0  0 ' in rr) or (' 152 98 ' in rr) or (' 255 ff ' in rr)]
    check('symbol no glyph', any('0  0 .' in rr for rr in rows) and any('152 98 .' in rr for rr in rows), repr(sym))
    check('symbol code page char', any('233 e9 й' in rr for rr in rows) and any('255 ff я' in rr for rr in rows),
          repr(sym))
    body = [rr for rr in rows if rr.strip().startswith('|') and rr.strip().endswith('|')]
    check('symbol columns aligned', len(body) > 32 and len(set(len(rr) for rr in body)) == 1,
          repr(set(len(rr) for rr in body)))
    mm.keys(b'\x07', 1.0)
    mm.close()
    shutil.rmtree(home, ignore_errors=True)
    print('%d passed, %d failed' % (passed, failed))
    sys.exit(1 if failed else 0)


if __name__ == '__main__':
    main()
