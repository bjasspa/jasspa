#!/usr/bin/env python3
#
# fuzz.py - Robustness fuzzing (Phase 8, acceptance test P2): generates files of random bytes,
# malformed UTF-8, mutated UTF-16/32, the byte patterns of ME's internal escapes, mixed line
# endings, NULs and lines around the 64K limit, and runs a headless mec over them, by default
# under valgrind. For each file it checks:
#   - it loads and saves byte identical (0 write-buffer skips the save checks);
#   - after movement, searches, string functions on its lines, an insert, both case changes and
#     a kill and yank of the whole buffer, undoing the 5 edits saves it byte identical again;
#   - saving it as UTF-16LE, UTF-16BE BOM, UTF-32LE and a code page does not fail, and when the
#     file is valid UTF-8 the UTF-16/32 saves hold the same text;
# and that valgrind reports no errors. A run which crashes or hangs is reported with the files of
# its batch, which are kept with -k.
#
# Usage: python3 -I fuzz.py <mec> [-n files] [-s seed] [-b batch] [-k dir] [--no-valgrind]

import argparse
import os
import random
import re
import shutil
import subprocess
import sys
import tempfile

SAMPLE = ('ASCII text, Ünïcödé ß ẞ ǆ Ǆ İ ı ΣΑΣ σας Кириллица, 中文字符 日本語 한국어, '
          'emoji 😀👍🏽 ‍ é ​‮﻿ tab\tend')

# byte patterns ME uses internally, or which are invalid in ways a decoder can get wrong
SPECIAL = [b'\xc0\x80', b'\xc1\xbf', b'\xc0\xaf', b'\xe0\x80\x80', b'\xe0\x81\x80', b'\xe0\x82\xbf',
           b'\xe0\x83\x80', b'\xe0\x80\xaf', b'\xed\xa0\x80', b'\xed\xbf\xbf', b'\xed\xa0\xbd\xed\xb8\x80',
           b'\xf4\x90\x80\x80', b'\xf5\x80\x80\x80', b'\xf8\x88\x80\x80\x80', b'\xfc\x84\x80\x80\x80\x80',
           b'\xfe', b'\xff', b'\x80', b'\xbf', b'\xc3', b'\xe4\xb8', b'\xf0\x9f\x98', b'\x00', b'\r',
           b'\n', b'\r\n', b'\r\r\n', b'\x1a', b'\x01', b'\x07', b'\x7f', b'\x85', b'\xa0',
           b'\xef\xbb\xbf', b'\xfe\xff', b'\xff\xfe', b'\x00\x00\xfe\xff', b'\xff\xfe\x00\x00']


def encId(enc):
    """the $buffer-encoding of the old numeric encoding bits, a single byte file being in code page C"""
    if not enc & 3:
        return 'C'
    ss = ('', 'utf8', 'utf16', 'utf32')[enc & 3] + (('be' if enc & 4 else 'le') if enc & 3 > 1 else '')
    return ss + ('-bom' if enc & 8 else '')

def mutate(rng, data, count):
    data = bytearray(data)
    for _ in range(count):
        op = rng.randrange(5)
        pos = rng.randrange(len(data) + 1)
        if op == 0 and data:
            data[min(pos, len(data) - 1)] ^= 1 << rng.randrange(8)
        elif op == 1:
            data[pos:pos] = bytes([rng.randrange(0x80, 0x100)])
        elif op == 2 and data:
            del data[min(pos, len(data) - 1)]
        elif op == 3:
            data[pos:pos] = rng.choice(SPECIAL)
        else:
            data = data[:pos] if rng.random() < 0.2 else data
    return bytes(data)


def text(rng, size):
    out = []
    while sum(len(ss) for ss in out) < size:
        ss = SAMPLE[rng.randrange(len(SAMPLE)):]
        out.append(ss[:rng.randrange(1, 40)])
        out.append(rng.choice(['\n', '\n', '\r\n', '\r', ' ']))
    return ''.join(out)


def generate(rng):
    """returns (kind, bytes)"""
    kind = rng.choice(['random', 'utf8', 'utf8', 'special', 'utf16', 'utf32', 'endings', 'long'])
    if kind == 'random':
        return kind, bytes(rng.randrange(256) for _ in range(rng.randrange(1, 3000)))
    if kind == 'utf8':
        return kind, mutate(rng, text(rng, rng.randrange(10, 3000)).encode('utf-8'), rng.randrange(0, 30))
    if kind == 'special':
        return kind, b''.join(rng.choice(SPECIAL + [b'a', b'\xc3\xa9', b'\xe4\xb8\xad', b'\xf0\x9f\x98\x80'])
                              for _ in range(rng.randrange(1, 800)))
    if kind in ('utf16', 'utf32'):
        enc = rng.choice(['le', 'be'])
        bom = rng.random() < 0.7
        unit = 2 if kind == 'utf16' else 4
        tt = text(rng, rng.randrange(10, 1500))
        data = ('﻿' if bom else '').encode('%s-%s' % (kind.replace('utf', 'utf-'), enc),
                                                'surrogatepass') + tt.encode('%s-%s' % (kind.replace('utf', 'utf-'), enc))
        data = bytearray(data)
        for _ in range(rng.randrange(0, 6)):
            # lone surrogates, zero units, odd trailing bytes
            pos = rng.randrange(len(data) // unit + 1) * unit
            sur = rng.choice([0xd800, 0xdc00, 0xdbff, 0xdfff, 0, 0x110000 if unit == 4 else 0xfffe])
            data[pos:pos] = sur.to_bytes(unit, 'little' if enc == 'le' else 'big')
        if rng.random() < 0.3:
            data += bytes(rng.randrange(256) for _ in range(rng.randrange(1, unit)))
        return kind, bytes(data)
    if kind == 'endings':
        parts = []
        for _ in range(rng.randrange(1, 300)):
            parts.append(rng.choice([b'abc', b'\xc3\xa9', b'', b'\x00', b'\xe4\xb8\xad\xe6\x96\x87', b'\xff']))
            parts.append(rng.choice([b'\n', b'\r', b'\r\n', b'\r\r\n', b'\n\r', b'']))
        if rng.random() < 0.3:
            parts.append(b'\x1a')
        return kind, b''.join(parts)
    # long lines near the 64K byte limit, a multi-byte char or raw byte straddling the split
    pre = rng.choice([0xff00, 0xfff0, 0x10000]) - rng.randrange(0, 8)
    fill = rng.choice([b'a', b'\xc3\xa9', b'\xe4\xb8\xad', b'\xf0\x9f\x98\x80'])
    line = (fill * (pre // len(fill) + 1))[:pre]
    tail = rng.choice([b'\xe4\xb8\xad', b'\xf0\x9f\x98\x80', b'\xff', b'\x00', b'\xc3', b'xyz'])
    return kind, line + tail + b'more\n' + rng.choice([b'', b'second line\n'])


def macroString(path):
    return path.replace('\\', '\\\\').replace('"', '\\"')


def patterns(rng, data):
    """search strings taken from the file, so some match"""
    out = []
    for _ in range(3):
        if len(data) > 4:
            pos = rng.randrange(len(data) - 2)
            ss = data[pos:pos + rng.randrange(1, 6)].decode('utf-8', 'ignore')
            ss = ''.join(ch for ch in ss if ch.isprintable() and ch not in '\\"')
            if ss:
                out.append(ss)
    return out or ['é']


def batchMacro(rng, files, wdir):
    mm = ['define-macro start-up']
    L = mm.append
    for ii, (inp, data) in enumerate(files):
        base = os.path.join(wdir, 'o%d' % ii)
        L('  !force find-file "%s"' % macroString(inp))
        L('  !if $status')
        L('    !force 0 write-buffer "%s.rt"' % macroString(base))
        # movement over the buffer, by chars, words and lines with a goal column
        L('    beginning-of-buffer')
        L('    !force 30000 forward-char')
        L('    !force 30000 backward-char')
        L('    !force 2000 forward-word')
        L('    !force 2000 backward-word')
        L('    !force set-variable $window-col 7')
        L('    !force 300 forward-line')
        L('    !force set-variable $window-col 3')
        L('    !force 300 backward-line')
        # string functions on the first lines
        L('    beginning-of-buffer')
        L('    set-variable #l0 0')
        L('    !while &les #l0 40')
        L('      !force set-variable #l1 &cat &cat &len @wl &mid @wl 2 5 &cat &slo @wl &sup @wl')
        L('      !force set-variable #l1 &cat &cat &blen @wl &bhex &lef @wl 6 &cat &atoi @wl &rig @wl 3')
        L('      !force set-variable #l1 &cat &uni &lef @wl 1 &ufi &mid @wl 1 2')
        L('      !force set-variable #l1 &cat &sin @wl "é" &isin @wl "Ä"')
        L('      !force set-variable #l1 &spr "%-10.4s|%5s" @wl @wc')
        L('      !force forward-line')
        L('      !if &not $status')
        L('        set-variable #l0 40')
        L('      !endif')
        L('      set-variable #l0 &add #l0 1')
        L('    !done')
        # searches, literal both ways ignoring case and not, and regex
        for pp in patterns(rng, data):
            for cmd in ('search-forward', 'search-backward'):
                L('    beginning-of-buffer' if cmd == 'search-forward' else '    end-of-buffer')
                L('    !force %s "%s"' % (cmd, pp))
        L('    -1 buffer-mode "exact"')
        L('    1 buffer-mode "magic"')
        for pp in ('[^a-z]+', '\\\\w+\\\\>', '.\\\\{3\\\\}', '[[:alpha:]]', '[^\\\\x00-\\\\x7f]', '[[:raw:]]'):
            L('    beginning-of-buffer')
            L('    !force search-forward "%s"' % pp)
            L('    end-of-buffer')
            L('    !force search-backward "%s"' % pp)
        L('    -1 buffer-mode "magic"')
        L('    1 buffer-mode "exact"')
        # 5 edits, each certain to change the buffer, undone together
        L('    beginning-of-buffer')
        L('    insert-string "aé中\\n"')
        L('    -2 undo')
        for cmd in ('upper-case-region', 'lower-case-region'):
            L('    beginning-of-buffer')
            L('    set-mark')
            L('    end-of-buffer')
            L('    %s' % cmd)
            L('    -2 undo')
        L('    beginning-of-buffer')
        L('    set-mark')
        L('    end-of-buffer')
        L('    kill-region')
        L('    -2 undo')
        L('    yank')
        L('    -2 undo')
        L('    5 undo')
        L('    !force 0 write-buffer "%s.ud"' % macroString(base))
        # other encodings
        for enc in (2, 14, 3, 0):
            L('    !force set-variable $buffer-encoding "%s"' % encId(enc))
            L('    !force 0 write-buffer "%s.e%d"' % (macroString(base), enc))
        L('    !force 0 delete-buffer $buffer-bname')
        L('  !endif')
    L('  4 exit-emacs 0')
    L('!emacro')
    return '\n'.join(mm) + '\n'


def decodeAs(data, enc):
    try:
        return data.decode(enc, 'strict')
    except UnicodeDecodeError:
        return None


def runBatch(args, rng, batch, wdir):
    """returns a list of failure strings"""
    files = []
    for ii, (kind, data) in enumerate(batch):
        inp = os.path.join(wdir, 'in%d.txt' % ii)
        open(inp, 'wb').write(data)
        files.append((inp, data))
    open(os.path.join(wdir, 'fz.emf'), 'w', encoding='utf-8').write(batchMacro(rng, files, wdir))
    cmd = [args.mec, '-p', '@' + os.path.join(wdir, 'fz')]
    vlog = os.path.join(wdir, 'vg.log')
    if not args.no_valgrind:
        cmd = ['valgrind', '--leak-check=full', '--error-exitcode=99', '--log-file=' + vlog] + cmd
    try:
        res = subprocess.run(cmd, stdin=subprocess.DEVNULL, capture_output=True, timeout=args.timeout,
                             env=dict(os.environ, MEPATH=wdir))
        rc = res.returncode
    except subprocess.TimeoutExpired:
        rc = 'timeout'
    fails = []
    if rc != 0:
        detail = ''
        if os.path.exists(vlog):
            log = open(vlog, encoding='utf-8', errors='replace').read()
            mm = re.search(r'ERROR SUMMARY: (\d+) errors', log)
            detail = ' (valgrind: %s errors)' % (mm.group(1) if mm else '?')
        fails.append('run exit %s%s' % (rc, detail))
    for ii, (kind, data) in enumerate(batch):
        base = os.path.join(wdir, 'o%d' % ii)
        tag = 'file %d (%s, %d bytes)' % (ii, kind, len(data))
        for ext, what in (('rt', 'load/save'), ('ud', 'edits undone')):
            path = base + '.' + ext
            if not os.path.exists(path):
                fails.append('%s: %s not written' % (tag, what))
            elif open(path, 'rb').read() != data:
                got = open(path, 'rb').read()
                pos = next((jj for jj in range(min(len(got), len(data))) if got[jj] != data[jj]),
                           min(len(got), len(data)))
                fails.append('%s: %s differs at byte %d, size %d -> %d' % (tag, what, pos, len(data), len(got)))
        for enc in (2, 14, 3, 0):
            if not os.path.exists('%s.e%d' % (base, enc)):
                fails.append('%s: save as encoding %d not written' % (tag, enc))
        # a valid UTF-8 file without a BOM must keep its text in UTF-16/32
        tt = decodeAs(data, 'utf-8')
        if (tt is not None) and not data.startswith(b'\xef\xbb\xbf') and os.path.exists(base + '.e2'):
            for enc, codec in ((2, 'utf-16-le'), (14, 'utf-16-be'), (3, 'utf-32-le')):
                got = decodeAs(open('%s.e%d' % (base, enc), 'rb').read(), codec)
                if got is not None and got.startswith('﻿'):
                    got = got[1:]
                if got != tt:
                    fails.append('%s: UTF-8 text not kept saving as encoding %d' % (tag, enc))
    return fails


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('mec')
    ap.add_argument('-n', type=int, default=200, help='number of files')
    ap.add_argument('-s', type=int, default=1, help='random seed')
    ap.add_argument('-b', type=int, default=20, help='files per run')
    ap.add_argument('-k', help='keep the files of failing batches in this directory')
    ap.add_argument('--timeout', type=int, default=1800)
    ap.add_argument('--no-valgrind', action='store_true')
    args = ap.parse_args()
    args.mec = os.path.abspath(args.mec)
    rng = random.Random(args.s)
    total = bad = 0
    for start in range(0, args.n, args.b):
        batch = [generate(rng) for _ in range(min(args.b, args.n - start))]
        wdir = tempfile.mkdtemp()
        fails = runBatch(args, rng, batch, wdir)
        total += len(batch)
        if fails:
            bad += 1
            print('batch at file %d (seed %d):' % (start, args.s))
            for ff in fails:
                print('  FAIL ' + ff)
            if args.k:
                keep = os.path.join(args.k, 'batch%d' % start)
                shutil.copytree(wdir, keep, dirs_exist_ok=True)
                # the kept macro names the kept files, so the batch can be run again
                kmac = os.path.join(keep, 'fz.emf')
                text = open(kmac, encoding='utf-8').read().replace(wdir, keep)
                open(kmac, 'w', encoding='utf-8').write(text)
                print('  kept in ' + keep)
        shutil.rmtree(wdir, ignore_errors=True)
        sys.stdout.flush()
    print('%d files in %d runs, %d runs failed' % (total, (args.n + args.b - 1) // args.b, bad))
    sys.exit(1 if bad else 0)


if __name__ == '__main__':
    main()
