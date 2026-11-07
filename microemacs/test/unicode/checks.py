#!/usr/bin/env python3
#
# checks.py - Tests the save file checks (unicode_design.md section 9) with a headless mec:
# each prompt answer, $file-checks, remembered answers and undo of a text fix, check 5 (chars a
# code page file cannot hold: asked first, never remembered, not asked by a 0 write-buffer)
# (acceptance tests F20, F21, F23, F25, F26, F28), the binary flag, and the line endings
# insert-file gives.
#
# Usage: python3 -I checks.py <mec>
#
# The prompts read their answer from the macro command line, e.g. write-buffer "file" "s".

import os
import subprocess
import sys
import tempfile

LATIN1_TABLE = ''.join('%04X' % cc for cc in range(0x80, 0x100))
MIXED = b'one\ntwo\r\nthree\nfour\r\n'
# a UTF-8 file (it has a valid sequence) with a NUL, mixed endings, no final newline and an invalid byte
ALL = b'NUL[\x00] \xc3\xa9t\xc3\xa9 caf\xe9\r\nline two\nend'
# Greek alpha and beta as ME string escapes, which the Latin-1 code page does not have
BS = chr(92)
GREEK = BS + 'u03B1' + BS + 'u03B2'
GREEK8 = b'\xce\xb1\xce\xb2'


def main():
    if len(sys.argv) < 2:
        sys.exit('Usage: checks.py <mec>')
    mec = os.path.abspath(sys.argv[1])
    wdir = tempfile.mkdtemp()
    inp = os.path.join(wdir, 'in')
    out = os.path.join(wdir, 'out')
    os.mkdir(inp)
    os.mkdir(out)
    files = {
        'nul_ascii.txt': b'a[\x00]b[\x00\x00]\n',
        'mixed.txt': MIXED,
        'noeol.txt': b'first\nlast',
        'invalid.txt': b'caf\xc3\xa9 and caf\xe9\n',
        'all.txt': ALL,
        'dos.txt': b'l1\r\nl2\r\n',
        'unix.txt': b't1\nt2\n',
        'binu.txt': b'n1\nn\x00y\n',
        'eols.txt': b'a\r\nb\nc\rd',
        'latin.txt': b'caf\xe9\n',
        'latinmix.txt': b'caf\xe9\r\nx\n',
        'utf8.txt': b'caf\xc3\xa9\n',
    }
    for name, data in files.items():
        open(os.path.join(inp, name), 'wb').write(data)

    mac = ['define-macro start-up', '  set-char-mask "c" "%s"' % LATIN1_TABLE, '  1 global-mode "undo"']
    expected = {}
    fmods = {}
    infos = {}
    encs = {}

    def case(tag, name, answers, want, pre=None, post=None):
        mac.append('  !force find-file "%s"' % os.path.join(inp, name))
        for ll in (pre or []):
            mac.append('  ' + ll)
        args = ''.join(' "%s"' % aa for aa in answers)
        mac.append('  !force write-buffer "%s"%s' % (os.path.join(out, tag), args))
        mac.append('  -2 ml-write &cat "status:%s=" $status' % tag)
        for ll in (post or []):
            mac.append('  ' + ll)
        mac.append('  !force 0 delete-buffer $buffer-bname')
        expected[tag] = want

    def insert(tag, name, ins, want, n=1, args='', fmod=None, pos='  forward-line'):
        # inserts after the first line and saves without the checks
        mac.append('  !force find-file "%s"' % os.path.join(inp, name))
        mac.extend(['  beginning-of-buffer', pos])
        mac.append('  %d insert-file "%s"%s' % (n, os.path.join(inp, ins), args))
        mac.append('  !force 0 write-buffer "%s"' % os.path.join(out, tag))
        mac.append('  -2 ml-write &cat "status:%s=" $status' % tag)
        if fmod is not None:
            fmods[tag] = fmod
            mac.append('  -2 ml-write &cat "fmod:%s=" &band $buffer-fmod 0x20000' % tag)
        mac.extend(['  -1 buffer-mode "edit"', '  !force 0 delete-buffer $buffer-bname'])
        expected[tag] = want

    # check 1 - NULs
    case('nul.p', 'nul_ascii.txt', ['p'], files['nul_ascii.txt'])
    case('nul.s', 'nul_ascii.txt', ['s'], b'a[]b[]\n')
    # check 2 - line endings
    case('eol.p', 'mixed.txt', ['p'], MIXED)
    case('eol.u', 'mixed.txt', ['u'], MIXED.replace(b'\r\n', b'\n'))
    case('eol.w', 'mixed.txt', ['w'], MIXED.replace(b'\r\n', b'\n').replace(b'\n', b'\r\n'))
    # check 3 - final newline
    case('noeol.p', 'noeol.txt', ['p'], b'first\nlast')
    case('noeol.a', 'noeol.txt', ['a'], b'first\nlast\n')
    # check 4 - invalid bytes in a UTF-8 file
    case('raw.p', 'invalid.txt', ['p'], files['invalid.txt'])
    case('raw.c', 'invalid.txt', ['c'], b'caf\xc3\xa9 and caf\xc3\xa9\n')
    case('raw.r', 'invalid.txt', ['r'], b'caf\xc3\xa9 and caf\xef\xbf\xbd\n')
    # all four in order: NULs, line endings, final newline, invalid bytes
    case('all.fix', 'all.txt', ['s', 'u', 'a', 'c'], b'NUL[] \xc3\xa9t\xc3\xa9 caf\xc3\xa9\nline two\nend\n')
    case('all.keep', 'all.txt', ['p', 'p', 'p', 'p'], ALL)
    # $file-checks - 1 always preserve, 2 always fix, no prompts so no answers given
    case('fc.keep', 'all.txt', [], ALL, pre=['set-variable $file-checks 0x55'], post=['set-variable $file-checks 0'])
    # one CRLF and one LF ending - a tie gives LF as the buffer style
    case('fc.fix', 'all.txt', [], b'NUL[] \xc3\xa9t\xc3\xa9 caf\xc3\xa9\nline two\nend\n',
         pre=['set-variable $file-checks 0xaa'], post=['set-variable $file-checks 0'])
    # a preserve answer is remembered, so a second save does not prompt
    case('remember', 'all.txt', ['p', 'p', 'p', 'p'], ALL,
         post=['!force write-buffer "%s"' % os.path.join(out, 'remember2'),
               '-2 ml-write &cat "status:remember2=" $status'])
    expected['remember2'] = ALL
    # check 5 - chars a code page file's page does not have, asked first, never remembered
    greek = ['end-of-line', 'insert-string "%s"' % GREEK]
    def enc(tag, want):
        encs[tag] = want
        return '-2 ml-write &cat "enc:%s=" $buffer-encoding' % tag
    case('lack.u', 'latin.txt', ['u'], b'caf\xc3\xa9' + GREEK8 + b'\n', pre=greek, post=[enc('lack.u', 'utf8')])
    case('lack.r', 'latin.txt', ['r'], b'caf\xe9??\n', pre=greek, post=[enc('lack.r', 'C')])
    # asked before the line endings, so 'r' answers it and 'u' the line endings
    case('lack.order', 'latinmix.txt', ['r', 'u'], b'caf\xe9??\nx\n', pre=greek)
    case('lack.fc1', 'latin.txt', [], b'caf\xe9??\n', pre=['set-variable $file-checks 0x100'] + greek,
         post=['set-variable $file-checks 0'])
    case('lack.fc2', 'latin.txt', [], b'caf\xc3\xa9' + GREEK8 + b'\n', pre=['set-variable $file-checks 0x200'] + greek,
         post=['set-variable $file-checks 0'])
    # a replace answer is not remembered, the second save asks again
    case('lack.again', 'latin.txt', ['r'], b'caf\xe9??\n', pre=greek,
         post=['!force write-buffer "%s" "u"' % os.path.join(out, 'lack.again2'),
               '-2 ml-write &cat "status:lack.again2=" $status'])
    expected['lack.again2'] = b'caf\xc3\xa9' + GREEK8 + b'\n'
    # no prompt when the page has every char or the buffer is UTF-8
    case('lack.none', 'latin.txt', [], b'caf\xe9\xe9\n', pre=['end-of-line', 'insert-string "%su00E9"' % BS])
    case('lack.utf8', 'utf8.txt', [], b'caf\xc3\xa9' + GREEK8 + b'\n', pre=greek)
    # a save without the checks writes '?' without asking
    mac += ['  !force find-file "%s"' % os.path.join(inp, 'latin.txt')] + ['  ' + ll for ll in greek]
    mac += ['  !force 0 write-buffer "%s"' % os.path.join(out, 'lack.skip'),
            '  -2 ml-write &cat "status:lack.skip=" $status', '  -1 buffer-mode "edit"',
            '  !force 0 delete-buffer $buffer-bname']
    expected['lack.skip'] = b'caf\xe9??\n'
    # stripping NULs is undoable, after a save that needs $system's keep undo bit (0x8000)
    case('undo', 'nul_ascii.txt', ['s'], b'a[]b[]\n', pre=['set-variable $system &bor $system 0x8000'],
         post=['undo', '!force 0 write-buffer "%s"' % os.path.join(out, 'undo2')])
    expected['undo2'] = files['nul_ascii.txt']

    # a NUL makes the buffer binary ($buffer-fmod 0x20000) until they are stripped
    mac += ['  !force find-file "%s"' % os.path.join(inp, 'nul_ascii.txt'),
            '  -2 ml-write &cat "fmod:nul.load=" &band $buffer-fmod 0x20000',
            '  !force write-buffer "%s" "s"' % os.path.join(out, 'nul.s2'),
            '  -2 ml-write &cat "fmod:nul.strip=" &band $buffer-fmod 0x20000',
            '  !force 0 delete-buffer $buffer-bname']
    expected['nul.s2'] = b'a[]b[]\n'
    fmods.update({'nul.load': '131072', 'nul.strip': '0'})
    # buffer-info shows each line end and its chars
    mac.append('  !force find-file "%s"' % os.path.join(inp, 'eols.txt'))
    for tag, want in (('crlf', 'char 13, 10 (0D, 0A - CRLF)'), ('lf', 'char 10 (0A - LF)'),
                      ('cr', 'char 13 (0D - CR)'), ('noeol', 'no char (no line end)')):
        mac += ['  end-of-line', '  1 buffer-info', '  -2 ml-write &cat "info:eol.%s=" $result' % tag, '  forward-line']
        infos['eol.' + tag] = want
    mac += ['  end-of-buffer', '  1 buffer-info', '  -2 ml-write &cat "info:eol.eob=" $result',
            '  !force 0 delete-buffer $buffer-bname']
    infos['eol.eob'] = 'no char (end of buffer)'
    # insert-file - a plain insert takes the buffer's line ending style (here CRLF)
    insert('ins.unix', 'dos.txt', 'unix.txt', b'l1\r\nt1\r\nt2\r\nl2\r\n')
    insert('ins.mixed', 'dos.txt', 'mixed.txt', b'l1\r\none\r\ntwo\r\nthree\r\nfour\r\nl2\r\n')
    # the buffer's own odd endings are kept, mixed.txt is a tie so LF
    insert('ins.own', 'mixed.txt', 'dos.txt', b'one\nl1\nl2\ntwo\r\nthree\nfour\r\n')
    # a binary buffer or file keeps the inserted endings, the file makes the buffer binary
    insert('ins.bin', 'nul_ascii.txt', 'dos.txt', files['nul_ascii.txt'] + files['dos.txt'], fmod='131072')
    insert('ins.binfile', 'dos.txt', 'binu.txt', b'l1\r\nn1\nn\x00y\nl2\r\n', fmod='131072')
    insert('ins.text', 'dos.txt', 'unix.txt', b'l1\r\nt1\r\nt2\r\nl2\r\n', fmod='0')
    # an inserted last line with no newline gets one unless it is the buffer's last line or binary
    insert('ins.noeol', 'dos.txt', 'noeol.txt', b'l1\r\nfirst\r\nlast\r\nl2\r\n')
    insert('ins.noeolend', 'dos.txt', 'noeol.txt', b'l1\r\nl2\r\nfirst\r\nlast', pos='  end-of-buffer')
    insert('ins.noeolbin', 'binu.txt', 'noeol.txt', b'n1\nfirst\nlastn\x00y\n')
    # 2 takes the file's modes and 4 is a partial read, both keep the endings
    insert('ins.modes', 'dos.txt', 'unix.txt', b'l1\r\nt1\nt2\nl2\r\n', n=2)
    insert('ins.part', 'dos.txt', 'unix.txt', b'l1\r\nt1\nt2\nl2\r\n', n=4, args=' "0" "0" "6"')

    mac += ['  4 exit-emacs 0', '!emacro']
    open(os.path.join(wdir, 'ck.emf'), 'w').write('\n'.join(mac) + '\n')
    try:
        res = subprocess.run([mec, '-p', '@' + os.path.join(wdir, 'ck')], stdin=subprocess.DEVNULL,
                             capture_output=True, env=dict(os.environ, MEPATH=wdir), timeout=120)
    except subprocess.TimeoutExpired:
        sys.exit('FAIL mec timed out - a prompt is probably waiting for input')
    log = (res.stdout + res.stderr).decode('latin-1').replace('\r', '\n')
    status = {}
    for line in log.split('\n'):
        if line.startswith('status:') and '=' in line:
            key, val = line[7:].split('=', 1)
            status[key] = val.strip()
        elif line.startswith('info:') and '=' in line:
            key, val = line[5:].split('=', 1)
            if not val.strip().endswith(' ' + infos.get(key, '?')):
                print('FAIL %s: buffer-info gave %r expected it to end %r' % (key, val.strip(), infos.get(key)))
                infos[key] = 'failed'
            else:
                infos[key] = 'ok'
        elif line.startswith('enc:') and '=' in line:
            key, val = line[4:].split('=', 1)
            if encs.get(key) != val.strip():
                print('FAIL %s: $buffer-encoding gave %s expected %s' % (key, val.strip(), encs.get(key)))
                encs[key] = 'failed'
            else:
                encs[key] = 'ok'
        elif line.startswith('fmod:') and '=' in line:
            key, val = line[5:].split('=', 1)
            got = val.strip()
            if fmods.get(key) != got:
                print('FAIL %s: $buffer-fmod 0x20000 gave %s expected %s' % (key, got, fmods.get(key)))
                fmods[key] = 'failed'
            else:
                fmods[key] = 'ok'

    passed = failed = 0
    for tag, want in expected.items():
        path = os.path.join(out, tag)
        if (tag in status) and (status[tag] != '1'):
            failed += 1
            print('FAIL %s: write-buffer status %s' % (tag, status[tag]))
        elif not os.path.exists(path):
            failed += 1
            print('FAIL %s not written' % tag)
        elif open(path, 'rb').read() != want:
            failed += 1
            print('FAIL %s: got %r expected %r' % (tag, open(path, 'rb').read(), want))
        else:
            passed += 1
    for tag, val in infos.items():
        if val == 'ok':
            passed += 1
        else:
            failed += 1
            if val != 'failed':
                print('FAIL %s: no buffer-info result' % tag)
    for tag, val in encs.items():
        if val == 'ok':
            passed += 1
        else:
            failed += 1
            if val != 'failed':
                print('FAIL %s: no $buffer-encoding result' % tag)
    for tag, val in fmods.items():
        if val == 'ok':
            passed += 1
        else:
            failed += 1
            if val != 'failed':
                print('FAIL %s: no $buffer-fmod result' % tag)
    print('%d passed, %d failed' % (passed, failed))
    sys.exit(1 if failed else 0)


if __name__ == '__main__':
    main()
