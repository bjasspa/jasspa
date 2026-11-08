#!/usr/bin/env python3
#
# fetch.py - Files fetched over http keep their text (acceptance test F15): serves UTF-8 (with and
# without a BOM, a long line), UTF-16 with a BOM, Latin-1, mixed line endings, no final newline and
# invalid bytes from a local http server, opens each with find-file "http://127.0.0.1:<port>/<file>"
# in a headless mec and reports the buffer's encoding and each line's bytes (&bhex) and ending
# ($line-flags), which must match the file. (A buffer read from a URL cannot be written to a local
# file, in src.old too, so the text is checked in the buffer.) ftp is not covered (no local ftp server).
#
# Usage: python3 -I fetch.py <mec>

import functools
import http.server
import os
import shutil
import subprocess
import sys
import tempfile
import threading

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from convert import LATIN1_TABLE, encId  # noqa: E402

TEXT = 'Héllo wörld 中文 😀\nsecond line\n'
# name: (file bytes, $buffer-encoding, the lines' internal UTF-8 and their endings)
FILES = {
    'utf8.txt': (TEXT.encode('utf-8'), 1, [('Héllo wörld 中文 😀', '\n'), ('second line', '\n')]),
    'utf8bom.txt': (b'\xef\xbb\xbf' + TEXT.encode('utf-8'), 9, [('Héllo wörld 中文 😀', '\n'), ('second line', '\n')]),
    'utf16.txt': (b'\xff\xfe' + TEXT.encode('utf-16-le'), 10, [('Héllo wörld 中文 😀', '\n'), ('second line', '\n')]),
    'latin1.txt': ('Héllo wörld\nsecond line\n'.encode('latin-1'), 0, [('Héllo wörld', '\n'), ('second line', '\n')]),
    'mixed.txt': ('a\r\nbé\nc\rd\r\n'.encode('utf-8'), 1, [('a', '\r\n'), ('bé', '\n'), ('c', '\r'), ('d', '\r\n')]),
    'noeol.txt': ('one\ntwó'.encode('utf-8'), 1, [('one', '\n'), ('twó', '')]),
    'long.txt': (('é' * 30000 + '\nend\n').encode('utf-8'), 1, [('é' * 30000, '\n'), ('end', '\n')]),
}
# one invalid byte means not UTF-8, so a stray byte makes the file read with the code page
# (Latin-1 here), its UTF-8 'é' then being two chars
INVALID = (b'caf\xc3\xa9 caf\xe9\n', 0, '636166c383c2a920636166c3a9')


class Quiet(http.server.SimpleHTTPRequestHandler):
    # keep-alive: ME's http client loses every second request to a server which closes the
    # connection after each reply (HTTP/1.0), in src.old too
    protocol_version = 'HTTP/1.1'

    def log_message(self, *args):
        pass


def main():
    if len(sys.argv) < 2:
        sys.exit("Usage: fetch.py <mec>")
    mec = os.path.abspath(sys.argv[1])
    wdir = tempfile.mkdtemp()
    sdir = os.path.join(wdir, 'srv')
    out = os.path.join(wdir, 'out')
    os.mkdir(sdir)
    os.mkdir(out)
    for name, (data, enc, lines) in FILES.items():
        open(os.path.join(sdir, name), 'wb').write(data)
    open(os.path.join(sdir, 'invalid.txt'), 'wb').write(INVALID[0])
    server = http.server.ThreadingHTTPServer(('127.0.0.1', 0), functools.partial(Quiet, directory=sdir))
    threading.Thread(target=server.serve_forever, daemon=True).start()
    port = server.server_address[1]
    mac = ['define-macro start-up', '  set-char-mask "c" "%s"' % LATIN1_TABLE]
    for name in list(FILES) + ['invalid.txt']:
        mac += ['  !force find-file "http://127.0.0.1:%d/%s"' % (port, name),
                '  -2 ml-write &cat "R:%s:enc=" $buffer-encoding' % name,
                '  -2 ml-write &cat "R:%s:style=" &cat &bmod "cr" &bmod "lf"' % name,
                '  beginning-of-buffer',
                '  set-variable #l0 0',
                '  !repeat',
                '    end-of-line',
                '    -2 ml-write &cat "R:%s:" &cat #l0 &cat "=" &cat $window-col &cat "|" &cat &bhex &bmid @wl 0 60 &cat "|" $line-flags' % name,
                '    set-variable #l0 &add #l0 1',
                '    !force forward-line',
                '  !until &not $status',
                '  !force 0 delete-buffer $buffer-bname']
    mac += ['  4 exit-emacs 0', '!emacro']
    mpath = os.path.join(wdir, 'ht.emf')
    open(mpath, 'w', encoding='utf-8').write('\n'.join(mac) + '\n')
    res = subprocess.run(['timeout', '-s', 'KILL', '120', mec, '-p', '@' + mpath[:-4]], stdin=subprocess.DEVNULL,
                         capture_output=True, env=dict(os.environ, MEPATH=wdir, HOME=wdir))
    server.shutdown()
    log = (res.stdout + res.stderr).decode('utf-8', 'replace').replace('\r', '\n')
    got = dict(ll[2:].split('=', 1) for ll in log.split('\n') if ll.startswith('R:') and '=' in ll)
    passed = failed = 0

    def check(tag, cond, detail):
        nonlocal passed, failed
        if cond:
            passed += 1
        else:
            failed += 1
            print('FAIL %s %s' % (tag, detail))

    for name, (data, enc, lines) in FILES.items():
        check(name + ' encoding', got.get(name + ':enc') == encId(enc), repr(got.get(name + ':enc')))
        style = got.get(name + ':style', '01')
        for ii, (text, eol) in enumerate(lines):
            line = got.get('%s:%d' % (name, ii), '0||0').split('|')
            if len(line) != 3:
                line = ['0', '', '0']
            # the length in chars ($window-col at the end, @wl is cut at a macro string's size)
            nc, hx, flags = line
            tb = text.encode('utf-8')
            check('%s line %d' % (name, ii), (nc == str(len(text))) and (hx == tb[:60].hex()), '%s %s' % (nc, hx[:60]))
            # the line's ending: the buffer style unless $line-flags says the CR or LF differs
            cr, lf = style[0] == '1', style[1] == '1'
            flags = int(flags or 0)
            cr ^= bool(flags & 0x80)
            lf ^= bool(flags & 0x100)
            if flags & 0x10:
                cr = lf = False
            have = ('\r' if cr else '') + ('\n' if lf else '')
            check('%s line %d ending' % (name, ii), have == eol, repr(have))
    check('invalid.txt encoding', got.get('invalid.txt:enc') == encId(INVALID[1]), repr(got.get('invalid.txt:enc')))
    check('invalid.txt line', got.get('invalid.txt:0', '||').split('|')[1] == INVALID[2],
          repr(got.get('invalid.txt:0')))
    shutil.rmtree(wdir, ignore_errors=True)
    print('%d passed, %d failed' % (passed, failed))
    sys.exit(1 if failed else 0)


if __name__ == '__main__':
    main()
