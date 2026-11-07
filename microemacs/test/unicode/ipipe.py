#!/usr/bin/env python3
#
# ipipe.py - ipipe tests (X1): runs mec in a pseudo terminal with an ipipe child writing UTF-8
# and terminal sequences, then checks the saved buffer - a cell per terminal column, wide
# chars, overwriting half a wide char, combining marks, invalid bytes, and UTF-8 sent to the
# child. test/ipipe/ipipewrap.emf covers the row counts.
#
# Usage: python3 -I ipipe.py <mec> [macros-dir]

import os
import shutil
import sys
import tempfile
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from screen import Mec, mecEnv  # noqa: E402

TDIR = os.path.dirname(os.path.abspath(__file__))
ESC = '\033'

# (tag, shell printf argument, expected line) - the child prints each followed by CR LF
CASES = [
    ('T1', 'a\\303\\251\\344\\270\\255\\360\\237\\230\\200b', 'aé中😀b'),
    # CR then overwrite the first wide char
    ('T2', '\\344\\270\\255\\346\\226\\207\\345\\255\\227\\rT2:ab', 'ab文字'),
    # CHA onto the second half of a wide char, the first half becomes a space
    ('T3', '\\344\\270\\255\\346\\226\\207\\033[5Gx', ' x文'),
    # a combining mark joins the char before it, so CR and 2 chars overwrite both
    ('T4', 'e\\314\\201x\\rT4:AB', 'AB'),
    ('T5', 'e\\314\\201x', 'éx'),
    # BS onto the second half of a wide char
    ('T6', '\\344\\270\\255\\bX', ' X'),
    # DCH of the first half of a wide char leaves the second half as a space
    ('T7', '\\344\\270\\255\\346\\226\\207\\033[4G\\033[1P', ' 文'),
    # ICH inside a run of wide chars
    ('T8', '\\344\\270\\255\\346\\226\\207\\033[6G\\033[2@', '中  文'),
    # EL from the second half of a wide char leaves the first half as a space
    ('T9', 'ab\\344\\270\\255cd\\033[7G\\033[K', 'ab '),
]

passed = failed = 0


def check(name, cond, detail=''):
    global passed, failed
    if cond:
        passed += 1
    else:
        failed += 1
        print('FAIL %s %s' % (name, detail))


def run(mec, macros, script, flags, keys=()):
    home = tempfile.mkdtemp()
    sfile = os.path.join(home, 'child.sh')
    open(sfile, 'w').write(script)
    out = os.path.join(home, 'out.txt')
    mm = Mec(mec, [], env=mecEnv(home, macros))
    mm.keys(b'\x07', 1.0)

    def line(ll, wait=2.0):
        mm.keys(b'\x1bxexecute-line\r', 1.0)
        mm.keys(ll.encode('utf-8') + b'\r', wait)
    line('%s ipipe-shell-command "sh %s" "*ut*"' % (flags, sfile), 2.0)
    for kk in keys:
        line(kk, 1.0)
    end = time.time() + 10
    while (time.time() < end) and (b'[EXIT' not in mm.raw):
        mm.settle(1.0)
    line('find-buffer "*ut*"')
    line('!force 0 write-buffer "%s"' % out)
    mm.close()
    data = open(out, 'rb').read() if os.path.exists(out) else b''
    shutil.rmtree(home, ignore_errors=True)
    return data


def main():
    if len(sys.argv) < 2:
        sys.exit('Usage: ipipe.py <mec> [macros-dir]')
    mec = os.path.abspath(sys.argv[1])
    macros = os.path.abspath(sys.argv[2] if len(sys.argv) > 2 else os.path.join(TDIR, '..', '..', 'macros'))
    script = ''.join("printf '%s:%s\\r\\n'\n" % (tag, arg) for tag, arg, _ in CASES)
    # an invalid byte is kept as a raw byte and saved unchanged
    script += "printf 'R1:\\377z\\r\\n'\n"
    for flags in ('0x08', '0x1008'):
        data = run(mec, macros, script, flags)
        lines = dict((ll.split(b':', 1)[0].decode('latin-1'), ll.split(b':', 1)[1].rstrip(b'\r'))
                     for ll in data.split(b'\n') if b':' in ll)
        for tag, _, want in CASES:
            got = lines.get(tag, b'').decode('utf-8', 'backslashreplace')
            check('%s %s' % (flags, tag), got == want, 'got %r expected %r' % (got, want))
        check('%s R1' % flags, lines.get('R1', b'').rstrip() == b'\xffz', repr(lines.get('R1')))
    # text sent to the child is UTF-8
    script = 'read l\nprintf "%s" "$l" | od -An -tx1\n'
    data = run(mec, macros, script, '0x08', ['find-buffer "*ut*"', 'ipipe-write "é中\\CM"'])
    # BSD od (macOS) spaces the bytes wider than GNU od
    check('ipipe-write UTF-8', b'c3 a9 e4 b8 ad' in b' '.join(data.split()), repr(data[-80:]))
    print('%d passed, %d failed' % (passed, failed))
    sys.exit(1 if failed else 0)


if __name__ == '__main__':
    main()
