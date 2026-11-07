#!/usr/bin/env python3
#
# undo.py - Undo keeps each line's ending (acceptance tests F10, F21, F25, E8): loads files with
# mixed line endings, no final newline, a final empty line and a line over the 64K limit, makes
# an edit which deletes or inserts line ends, undoes it and checks the file saves byte identical;
# also checks inserting whole lines at the start of a line leaves its ending with it. In a pseudo
# terminal, chars typed in over mode are undone one per undo (they share a node, bug 8 in unicode_src_differences.md), and
# a case change or transpose straight after is undone in one; with a Unicode build also over
# multi-byte chars. Runs in both builds (the line endings are not Unicode only).
#
# Usage: python3 -I undo.py <mec> [macros-dir]

import os
import shutil
import subprocess
import sys
import tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from screen import Mec, mecEnv  # noqa: E402

TDIR = os.path.dirname(os.path.abspath(__file__))
UNDO = b'\x18u'

FILES = {
    'mixed': b'a1\r\nb2\nc3\rd4\r\n',
    'mixed2': b'a1\r\nb2\nc3\n',
    'noeol': b'abc\ndef',
    'ctrlz': b'abc\ndef\x1a',
    'lastcr': b'x\n\r',
    'lastlf': b'x\n\n',
    'nl': b'\n',
    'long': b'a' * 0xff00 + b'bcd\nend\n',
}

EDITS = {
    'kill all': ['beginning-of-buffer', 'set-mark', 'end-of-buffer', 'kill-region'],
    'kill middle': ['beginning-of-buffer', 'forward-char', 'set-mark', 'end-of-buffer', 'backward-char',
                    'kill-region'],
    'kill lines': ['beginning-of-buffer', '!force 2 kill-line'],
    'join': ['beginning-of-buffer', 'end-of-line', '!force forward-delete-char'],
    'backspace join': ['beginning-of-buffer', '!force forward-line', '!force backward-delete-char'],
    'insert lines': ['beginning-of-buffer', 'insert-string "Z\\nY\\n"'],
    'insert split': ['beginning-of-buffer', 'forward-char', 'insert-string "Z\\nY"'],
    'yank': ['beginning-of-buffer', 'set-mark', 'end-of-buffer', 'copy-region', 'beginning-of-buffer', 'yank'],
}

# inserting whole lines at the start of a line: the line's ending stays with its text
INSERTS = {
    # new lines take the buffer's style, CRLF being the most common here
    'mixed': b'Z\r\nY\r\na1\r\nb2\nc3\rd4\r\n',
    'noeol': b'Z\nY\nabc\ndef',
    'long': b'Z\nY\n' + FILES['long'],
}


# typed in over mode from the start of a line, then undo one at a time: the line after each
TYPED = [
    ('over', 'abcdef', [b'x', b'y', b'z'], [], ['xyzdef', 'xycdef', 'xbcdef', 'abcdef']),
    ('upper after over', 'abcdef ghi', [b'x', b'\x1bu'], [], ['xBCDEF ghi', 'xbcdef ghi', 'abcdef ghi']),
    ('transpose after over', 'abcdef', [b'x', b'y', b'\x14'], [], ['xcydef', 'xycdef', 'xbcdef', 'abcdef']),
]
UTYPED = [
    ('over multi-byte', 'üöäß', ['é', 'ñ', 'ç'], [], ['éñçß', 'éñäß', 'éöäß', 'üöäß']),
    ('over mixed widths', 'aüb', ['é', 'x'], [], ['éxb', 'éüb', 'aüb']),
]


def textRow(mm, start):
    for rr in range(mm.screen.rows - 1):
        if mm.screen.text(rr).rstrip() == start:
            return rr
    return None


def typed(mec, macros, unicode):
    passed = failed = 0
    for name, text, keys, _, want in TYPED + (UTYPED if unicode else []):
        home = tempfile.mkdtemp()
        fname = os.path.join(home, 'in.txt')
        open(fname, 'w', encoding='utf-8').write(text + '\n')
        mm = Mec(mec, [fname], env=mecEnv(home, macros))
        try:
            mm.keys(b'\x1bxbuffer-mode\rover\r')
            row = textRow(mm, text)
            for kk in keys:
                mm.keys(kk if isinstance(kk, bytes) else kk.encode('utf-8'))
            got = []
            for ii in range(len(want)):
                if ii:
                    mm.keys(UNDO)
                got.append(None if row is None else mm.screen.text(row).rstrip())
        finally:
            mm.close()
            shutil.rmtree(home, ignore_errors=True)
        if got == want:
            passed += 1
        else:
            failed += 1
            print('FAIL typed %s: %r' % (name, got))
    return passed, failed


def main():
    if len(sys.argv) < 2:
        sys.exit('Usage: undo.py <mec> [macros-dir]')
    mec = os.path.abspath(sys.argv[1])
    wdir = tempfile.mkdtemp()
    mac = ['define-macro start-up']
    cases = []
    for fname, data in FILES.items():
        inp = os.path.join(wdir, fname + '.txt')
        open(inp, 'wb').write(data)
        for ename, edit in EDITS.items():
            out = os.path.join(wdir, '%s-%d.out' % (fname, len(cases)))
            cases.append((fname, ename, out, data))
            mac += ['  !force find-file "%s"' % inp] + ['  ' + ee for ee in edit]
            mac += ['  -2 undo', '  undo', '  !force 0 write-buffer "%s"' % out, '  !force 0 delete-buffer $buffer-bname']
        if fname in INSERTS:
            out = os.path.join(wdir, '%s-ins.out' % fname)
            cases.append((fname, 'insert lines kept', out, INSERTS[fname]))
            mac += ['  !force find-file "%s"' % inp, '  beginning-of-buffer', '  insert-string "Z\\nY\\n"',
                    '  !force 0 write-buffer "%s"' % out, '  !force 0 delete-buffer $buffer-bname']
    mac += ['  4 exit-emacs 0', '!emacro']
    open(os.path.join(wdir, 'ud.emf'), 'w', encoding='utf-8').write('\n'.join(mac) + '\n')
    subprocess.run(['timeout', '-s', 'KILL', '300', mec, '-p', '@' + os.path.join(wdir, 'ud')],
                   stdin=subprocess.DEVNULL, capture_output=True, env=dict(os.environ, MEPATH=wdir))
    macros = os.path.abspath(sys.argv[2] if len(sys.argv) > 2 else os.path.join(TDIR, '..', '..', 'macros'))
    probe = os.path.join(wdir, 'u.emf')
    open(probe, 'w').write('define-macro start-up\n  -2 ml-write &cat "R:uni=" &opt "uni"\n  4 exit-emacs 0\n!emacro\n')
    res = subprocess.run([mec, '-p', '@' + probe[:-4]], stdin=subprocess.DEVNULL, capture_output=True, timeout=30,
                         env=dict(os.environ, MEPATH=wdir))
    passed, failed = typed(mec, macros, b'R:uni=1' in res.stdout + res.stderr)
    for fname, ename, out, want in cases:
        got = open(out, 'rb').read() if os.path.exists(out) else None
        if got == want:
            passed += 1
        else:
            failed += 1
            print('FAIL %s %s: %r' % (fname, ename, got if (got is None) or (len(got) < 60) else got[-40:]))
    shutil.rmtree(wdir, ignore_errors=True)
    print('%d passed, %d failed' % (passed, failed))
    sys.exit(1 if failed else 0)


if __name__ == '__main__':
    main()
