#!/usr/bin/env python3
#
# spell.py - Spelling tests (X3): checks words, guesses, a buffer scan and derivatives with the
# 8-bit dictionaries in spelling/ (German, Polish, French, Russian), each with a display code page
# that differs from the dictionary's (the spelling code page set by set-char-mask "S", the rules
# read in it by add-spell-rule "@"), headlessly. A rules file whose bytes happen to be valid UTF-8
# must still be read as single byte. The ignore dictionary of each spelling code page (ASCII
# words, so this part runs in any build, the rest only in a Unicode build).
#
# Usage: python3 -I spell.py <mec> [spelling-dir]

import os
import subprocess
import sys
import tempfile

TDIR = os.path.dirname(os.path.abspath(__file__))


def table(enc):
    out = ''
    for bb in range(0x80, 0x100):
        ch = bytes([bb]).decode(enc, 'replace')
        out += '%04x' % (bb if ch == '�' else ord(ch))
    return out


# language, display code page, dictionary code page, [(word, ok)], (guess for, expected in it)
LANGS = [
    ('dede', 'latin_1', 'cp1252',
     [('schön', 1), ('Grüße', 1), ('Straße', 1), ('Äpfel', 1), ('ÜBER', 1), ('Übermäßig', 1),
      ('Mädchn', 0), ('Strasse', 0)],
     ('Mädchn', 'Mädchen')),
    ('plpl', 'iso8859_2', 'cp1250',
     [('świat', 1), ('źle', 1), ('zaś', 1), ('gęś', 1), ('Łódź', 1), ('ŚWIAT', 1), ('swiat', 0)],
     ('swiat', 'świat')),
    # French rules use flag bytes from 0x80, so flags are converted through the code page too
    ('frfr', 'latin_1', 'cp1252',
     [('reçoivent', 1), ('aperçu', 1), ('sait', 1), ('Reçu', 1), ('reçoivant', 0)],
     ('recevior', 'recevoir')),
    ('ruru', 'koi8_r', 'cp1251',
     [('мир', 1), ('слово', 1), ('Москва', 1), ('МОСКВА', 1), ('слова', 1), ('славo', 0)],
     ('слвоо', 'слово')),
]

passed = failed = 0


def check(name, cond, detail=''):
    global passed, failed
    if cond:
        passed += 1
    else:
        failed += 1
        print('FAIL %s %s' % (name, detail))


def run(mec, wdir, sdir, mac):
    open(os.path.join(wdir, 'sp.emf'), 'w', encoding='utf-8').write('\n'.join(mac) + '\n')
    res = subprocess.run([mec, '-p', '@' + os.path.join(wdir, 'sp')], stdin=subprocess.DEVNULL,
                         capture_output=True, env=dict(os.environ, MEPATH=wdir + ':' + sdir), timeout=120)
    log = (res.stdout + res.stderr).decode('utf-8', 'backslashreplace').replace('\r', '\n')
    return dict(ll[2:].split('=', 1) for ll in log.split('\n') if ll.startswith('R:') and '=' in ll)


IGNORE = [('pa', 'Fehlr', 'E'), ('pa', '+Fehlr', 'O'), ('pb', 'Fehlr', 'E'), ('pb', '+Wrng', 'O'),
          ('pa', 'Fehlr', 'O'), ('pa', 'Wrng', 'E'), ('pb', 'Wrng', 'O'), ('pa', '-', ''),
          ('pa', 'Fehlr', 'E'), ('pb', 'Wrng', 'E')]


def ignore(mec, wdir, sdir):
    # pa and pb are empty code pages, a word starting + is added to the ignore dictionary first
    # and - is 4 delete-dictionary
    mac = ['define-macro start-up', '  -2 ml-write &cat "R:uni=" &opt "uni"',
           '  set-char-mask "C" "pa"', '  set-char-mask "C" "pb"', '  set-char-mask "C" "C"',
           '  set-char-mask "S" "pa"', '  0 set-char-mask "S"', '  -2 ml-write &cat "R:S=" $result',
           '  !force add-spell-rule "@" "lsrnosuch"', '  -2 ml-write &cat "R:nofile=" $status',
           '  add-spell-rule "@" "lsrenus"', '  add-dictionary "lsmenus"']
    for ii, (cp, ww, _) in enumerate(IGNORE):
        mac.append('  set-char-mask "S" "%s"' % cp)
        if ww == '-':
            mac.append('  4 delete-dictionary')
            continue
        if ww[0] == '+':
            ww = ww[1:]
            mac.append('  0x0d spell "%s" ""' % ww)
        mac += ['  !force 1 spell "%s"' % ww, '  -2 ml-write &cat "R:i%d=" $result' % ii]
    mac += ['  set-char-mask "S" ""', '  !force 1 spell "Fehlr"', '  -2 ml-write &cat "R:nopage=" $status',
            '  !force add-spell-rule "@" "lsrenus"', '  -2 ml-write &cat "R:noprules=" $status',
            '  4 exit-emacs 0', '!emacro']
    got = run(mec, wdir, sdir, mac)
    uni = got.get('uni') == '1'
    check('S current', got.get('S') == 'pa', repr(got.get('S')))
    check('@ missing file', got.get('nofile') == '0', repr(got.get('nofile')))
    for ii, (cp, ww, want) in enumerate(IGNORE):
        if ww != '-':
            ww = ww.lstrip('+')
            check('ignore %d %s %s' % (ii, cp, ww), got.get('i%d' % ii) == want + ww, repr(got.get('i%d' % ii)))
    # only the Unicode build needs a spelling code page to spell, every build to run a rules file
    check('no spelling code page', got.get('nopage') == ('0' if uni else '1'), repr(got.get('nopage')))
    check('no spelling code page @', got.get('noprules') == '0', repr(got.get('noprules')))
    return uni


def main():
    if len(sys.argv) < 2:
        sys.exit('Usage: spell.py <mec> [spelling-dir]')
    mec = os.path.abspath(sys.argv[1])
    sdir = os.path.abspath(sys.argv[2] if len(sys.argv) > 2 else os.path.join(TDIR, '..', '..', 'spelling'))
    wdir = tempfile.mkdtemp()
    if not ignore(mec, wdir, sdir):
        print('%d passed, %d failed (8-bit build, the code page conversions are not tested)' % (passed, failed))
        sys.exit(1 if failed else 0)
    # q, A diaeresis, S caron in cp1252 is also UTF-8 for U+010A
    fluke = b'add-spell-rule "*" "q\xc4\x8a+"\n'
    fluke.decode('utf-8')
    open(os.path.join(wdir, 'lsrfluke.emf'), 'wb').write(fluke)
    for lang, disp, dcp, words, (gw, gwant) in LANGS:
        if not os.path.exists(os.path.join(sdir, 'lsm%s.edf' % lang)):
            print('SKIP %s - no dictionary in %s' % (lang, sdir))
            continue
        mac = ['define-macro start-up',
               '  set-char-mask "C" "dict"',
               '  set-char-mask "c" "%s"' % table(dcp),
               '  set-char-mask "S" "dict"',
               '  set-char-mask "C" "C"',
               '  set-char-mask "c" "%s"' % table(disp),
               '  add-spell-rule "@" "lsr%s"' % lang,
               '  add-dictionary "lsm%s"' % lang]
        for ii, (ww, _) in enumerate(words):
            mac += ['  !force 1 spell "%s"' % ww, '  -2 ml-write &cat "R:c%d=" $result' % ii]
        mac += ['  !force 33 spell "%s"' % gw, '  -2 ml-write &cat "R:guess=" $result']
        if lang == 'dede':
            mac += ['  add-spell-rule "@" "lsrfluke"', '  !force 1 spell "qÄŠŠ"', '  -2 ml-write &cat "R:fluke=" $result']
            # the first misspelt word in a buffer and its position, then derivatives as UTF-8
            mac += ['  find-buffer "*t*"',
                    '  insert-string "Das Mädchen ist schön und grün. Fehlr Grüße."',
                    '  beginning-of-buffer',
                    '  !force 2 spell',
                    '  -2 ml-write &cat &cat "R:scan=" $result &cat "|" $window-col',
                    '  end-of-buffer',
                    '  insert-newline',
                    '  !force 0x141 spell "Mädchen"',
                    '  beginning-of-buffer',
                    '  !force search-forward "^Mädchens$"',
                    '  -2 ml-write &cat "R:deriv=" $status']
        mac += ['  4 exit-emacs 0', '!emacro']
        got = run(mec, wdir, sdir, mac)
        for ii, (ww, ok) in enumerate(words):
            check('%s %s' % (lang, ww), got.get('c%d' % ii) == ('O' if ok else 'E') + ww, repr(got.get('c%d' % ii)))
        check('%s guess %s' % (lang, gw), ('|%s|' % gwant) in got.get('guess', ''), repr(got.get('guess', '')[:80]))
        if lang == 'dede':
            check('dede rules file valid as UTF-8 read as single byte', got.get('fluke') == 'OqÄŠŠ', repr(got.get('fluke')))
            check('dede buffer scan', got.get('scan') == 'EFehlr|37', repr(got.get('scan')))
            check('dede derivatives', got.get('deriv') == '1', repr(got.get('deriv')))
    print('%d passed, %d failed' % (passed, failed))
    sys.exit(1 if failed else 0)


if __name__ == '__main__':
    main()
