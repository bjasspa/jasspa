#!/usr/bin/env python3
#
# perf.py - Performance (Phase 8, acceptance tests P4-P6): compares a Unicode build with an 8-bit
# build (and optionally the original src, src.old) on the same text.
#
#   P4 - a large file (default 100 MB) of Latin, Cyrillic and CJK prose, as UTF-8 for the Unicode
#        build and in a code page (ISO-8859-1, cp1251) for the 8-bit builds: load, end-of-buffer,
#        searches which fail (so scan everything) and save, timed in the editor with $unix-time so
#        start-up is not counted. search/isearch are a word in magic mode (the default, so what a
#        user gets) exact and ignoring case, regex a class pattern, literal/ilitrl the word with
#        magic off. Target: Unicode within 2x.
#   P5 - a 1 MB single line file (split into 64K byte lines on load): end/beginning-of-line,
#        line moves keeping a goal column and char moves on the long lines, headless, plus a
#        redraw of the long line in a pseudo terminal. Target: responsive.
#   P6 - peak memory (VmHWM) of each P4 run. Target: Unicode within 1.5x, which UTF-8 cannot meet
#        for scripts that take 2 or 3 bytes a char in UTF-8 and 1 in a code page.
#
# Each measurement runs in its own process and the fastest of --runs is kept. Use release builds
# for numbers users see; debug builds (-O0) exaggerate differences in hot loops.
#
# Usage: python3 -I perf.py <unicode-mec> <8bit-mec> [--src <mec>] [--size MB] [--runs N]

import argparse
import os
import random
import re
import shutil
import subprocess
import sys
import tempfile
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

LATIN = ('le la les des une été déjà être français garçon über schön größe straße mañana niño año '
         'café crème naïve élève forêt hôpital île château mère dès à où çà Ärger Öl Übung '
         'the and of to in that is for it with as was on be at by this had not are but from').split()
CYRILLIC = ('и в не на я быть он с что а по это она этот к но они мы как из у который то за свой '
            'весь год от так о для ты же все тот мочь вы человек такой его сказать только или ещё '
            'бы себя один как уже до время если сам когда другой вот говорить наш мой знать стать').split()
CJK = ('的一是不了人我在有他这中大来上国个到说们为子和你地出道也时年得就那要下以生会自着去之过家学'
       '对可她里后小么心多天而能好都然没日于起还发成事只作当想看文无开手十用主行方又如前所本见经头'
       '面公同三已老从动两长知民样现分将外但身些与高意进把法此实回二理美点月明其种声全工己话儿者向')

SCRIPTS = {
    # name: (8-bit code page or None, Unicode-build pattern which fails, 8-bit pattern)
    'latin': ('iso-8859-1', 'françaisézq', 'françaisézq'),
    'cyrillic': ('cp1251', 'человекЪЪЪ', 'человекЪЪЪ'),
    'cjk': (None, '中国人民Ｚ', None),
}


def words(name, rng):
    if name == 'latin':
        return lambda: rng.choice(LATIN)
    if name == 'cyrillic':
        return lambda: rng.choice(CYRILLIC)
    return lambda: ''.join(rng.choice(CJK) for _ in range(rng.randrange(1, 5)))


def prose(name, size, seed=1):
    """about size bytes of UTF-8 prose in lines of 40-90 chars"""
    rng = random.Random(seed)
    word = words(name, rng)
    lines, total = [], 0
    sep = '' if name == 'cjk' else ' '
    while total < size:
        ll, ln = [], 0
        goal = rng.randrange(40, 90)
        while ln < goal:
            ww = word()
            ll.append(ww)
            ln += len(ww) + len(sep)
        line = sep.join(ll) + ('。' if name == 'cjk' else '.')
        lines.append(line)
        total += len(line.encode('utf-8')) + 1
    return '\n'.join(lines) + '\n'


def macroFile(path, lines, enc):
    open(path, 'w', encoding=enc).write('\n'.join(lines) + '\n')


def stampLines(steps):
    """steps: [(tag, [macro lines])], each step timed with $unix-time"""
    out = ['set-variable #l9 $unix-time']
    for tag, body in steps:
        out += body
        out += ['set-variable #l8 $unix-time',
                '-2 ml-write &cat "T:%s=" &cat #l9 &cat "," #l8' % tag,
                'set-variable #l9 #l8']
    return out


def parseStamps(text):
    res = {}
    for mm in re.finditer(r'T:(\w+)=([0-9.]+),([0-9.]+)', text):
        res[mm.group(1)] = (float(mm.group(3)) - float(mm.group(2))) * 1000.0
    return res


def runTimedRss(mec, wdir, name, steps, enc, timeout):
    """as runTimed, also returning the child's peak RSS in MB"""
    mpath = os.path.join(wdir, name + '.emf')
    macroFile(mpath, ['define-macro start-up'] + ['  ' + ll for ll in stampLines(steps)] +
              ['  4 exit-emacs 0', '!emacro'], enc)
    logf = os.path.join(wdir, name + '.log')
    with open(logf, 'wb') as lf:
        proc = subprocess.Popen([mec, '-p', '@' + mpath[:-4]], stdin=subprocess.DEVNULL, stdout=lf,
                                stderr=subprocess.STDOUT, env=dict(os.environ, MEPATH=wdir))
        # the child's own high-water mark: its ru_maxrss would include this process's size at
        # the fork, as Linux keeps the fork's mark across exec
        end = time.time() + timeout
        hwm = 0
        while time.time() < end:
            try:
                for ll in open('/proc/%d/status' % proc.pid):
                    if ll.startswith('VmHWM:'):
                        hwm = max(hwm, int(ll.split()[1]))
            except OSError:
                pass
            if os.waitpid(proc.pid, os.WNOHANG)[0]:
                break
            time.sleep(0.02)
        else:
            proc.kill()
            os.waitpid(proc.pid, 0)
    text = open(logf, 'rb').read().decode(enc, 'replace').replace('\r', '\n')
    return parseStamps(text), hwm / 1024.0


def ms(vv):
    return '%8.0f' % vv if vv is not None else '       -'


def p4Steps(path, out, pat):
    return [
        ('load', ['find-file "%s"' % path]),
        ('eob', ['end-of-buffer']),
        # magic mode is on by default, so a user's search is a regex search
        ('search', ['1 buffer-mode "magic"', 'beginning-of-buffer', '!force search-forward "%s"' % pat]),
        ('isearch', ['-1 buffer-mode "exact"', 'beginning-of-buffer', '!force search-forward "%s"' % pat,
                     '1 buffer-mode "exact"']),
        ('regex', ['beginning-of-buffer', '!force search-forward "[0-9][0-9][0-9]x"']),
        ('literal', ['-1 buffer-mode "magic"', 'beginning-of-buffer', '!force search-forward "%s"' % pat]),
        ('ilitrl', ['-1 buffer-mode "exact"', 'beginning-of-buffer', '!force search-forward "%s"' % pat,
                    '1 buffer-mode "exact"', '1 buffer-mode "magic"']),
        ('save', ['!force 0 write-buffer "%s"' % out]),
    ]


def p5Steps(path):
    return [
        ('load', ['find-file "%s"' % path]),
        ('eol', ['beginning-of-buffer', 'set-variable #l0 0', '!while &les #l0 1000', 'end-of-line',
                 'beginning-of-line', 'set-variable #l0 &add #l0 1', '!done']),
        ('goal', ['beginning-of-buffer', 'end-of-line', 'set-variable #l0 0', '!while &les #l0 200',
                  '!force forward-line', '!force backward-line', 'set-variable #l0 &add #l0 1', '!done']),
        ('chars', ['beginning-of-buffer', '!force 20000 forward-char', '!force 20000 backward-char']),
        ('col', ['beginning-of-buffer', 'set-variable #l0 0', '!while &les #l0 200',
                 '!force set-variable $window-col 15000', '!force set-variable $window-col 10',
                 'set-variable #l0 &add #l0 1', '!done']),
    ]


def p5Display(mec, wdir, path, enc, macros):
    """times redrawing the long line in a pseudo terminal: 50 x end/beginning of line with updates"""
    from screen import Mec, mecEnv
    res = os.path.join(wdir, 'disp.res')
    body = stampLines([('disp', ['set-variable #l0 0', '!while &les #l0 50', 'end-of-line', 'screen-update',
                                 'beginning-of-line', 'screen-update', 'set-variable #l0 &add #l0 1', '!done'])])
    # the stamps go to a file rather than the message line, which the redraws overwrite
    body = [ll.replace('-2 ml-write &cat "T:', 'set-variable #l7 &cat #l7 &cat "T:') for ll in body]
    body = [ll + ' "\\n"' if ll.startswith('set-variable #l7') else ll for ll in body]
    mac = ['define-macro start-up', '  find-file "%s"' % path, '  set-variable #l7 ""'] + ['  ' + ll for ll in body] + \
          ['  find-buffer "*r*"', '  insert-string #l7', '  !force 0 write-buffer "%s"' % res,
           '  4 exit-emacs 0', '!emacro']
    mpath = os.path.join(wdir, 'disp.emf')
    macroFile(mpath, mac, enc)
    if os.path.exists(res):
        os.remove(res)
    mm = Mec(mec, ['@' + mpath[:-4]], env=mecEnv(wdir, macros, 'C.UTF-8' if enc == 'utf-8' else 'C'))
    end = time.time() + 120
    while not os.path.exists(res) and time.time() < end:
        mm.settle(1.0)
    mm.close()
    if not os.path.exists(res):
        return None
    return parseStamps(open(res, 'rb').read().decode(enc, 'replace')).get('disp')


def best(results):
    out = {}
    for rr in results:
        for kk, vv in rr.items():
            out[kk] = min(out.get(kk, vv), vv)
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('umec')
    ap.add_argument('bmec')
    ap.add_argument('--src')
    ap.add_argument('--size', type=int, default=100, help='P4 file size in MB')
    ap.add_argument('--runs', type=int, default=3)
    ap.add_argument('--timeout', type=int, default=600)
    ap.add_argument('--macros', default=os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'macros'))
    args = ap.parse_args()
    builds = [('unicode', os.path.abspath(args.umec)), ('8-bit', os.path.abspath(args.bmec))]
    if args.src:
        builds.append(('src.old', os.path.abspath(args.src)))
    wdir = tempfile.mkdtemp()
    try:
        print('P4/P6: %d MB files, fastest of %d runs, times in ms, memory in MB' % (args.size, args.runs))
        cols = ['load', 'eob', 'search', 'isearch', 'regex', 'literal', 'ilitrl', 'save']
        print('%-18s %-8s' % ('file', 'build') + ''.join('%9s' % cc for cc in cols) + '%9s' % 'maxrss')
        for name, (cp, upat, bpat) in SCRIPTS.items():
            text = prose(name, args.size * 1024 * 1024)
            files = {'utf-8': os.path.join(wdir, name + '-utf8.txt')}
            open(files['utf-8'], 'wb').write(text.encode('utf-8'))
            if cp:
                files[cp] = os.path.join(wdir, name + '-8bit.txt')
                open(files[cp], 'wb').write(text.encode(cp))
            del text
            for bname, mec in builds:
                enc = 'utf-8' if bname == 'unicode' else cp
                if enc is None:
                    continue
                pat = upat if bname == 'unicode' else bpat
                runs, rss = [], []
                for rr in range(args.runs):
                    tt, mr = runTimedRss(mec, wdir, 'p4', p4Steps(files[enc], os.path.join(wdir, 'out.txt'), pat),
                                         enc, args.timeout)
                    runs.append(tt)
                    rss.append(mr)
                bb = best(runs)
                label = '%s %s' % (name, 'UTF-8' if enc == 'utf-8' else enc)
                print('%-18s %-8s' % (label, bname) + ''.join(ms(bb.get(cc)) + ' ' for cc in cols) +
                      '%8.0f' % min(rss))
                sys.stdout.flush()
            for ff in files.values():
                os.remove(ff)
            if os.path.exists(os.path.join(wdir, 'out.txt')):
                os.remove(os.path.join(wdir, 'out.txt'))

        print()
        print('P5: 1 MB single line, split into 64K byte lines on load, times in ms')
        cols = ['load', 'eol', 'goal', 'chars', 'col', 'disp']
        print('%-18s %-8s' % ('file', 'build') + ''.join('%9s' % cc for cc in cols))
        for name, (cp, upat, bpat) in SCRIPTS.items():
            text = prose(name, 1024 * 1024).replace('\n', ' ').rstrip() + '\n'
            files = {'utf-8': os.path.join(wdir, name + '-line-utf8.txt')}
            open(files['utf-8'], 'wb').write(text.encode('utf-8'))
            if cp:
                files[cp] = os.path.join(wdir, name + '-line-8bit.txt')
                open(files[cp], 'wb').write(text.encode(cp))
            for bname, mec in builds:
                enc = 'utf-8' if bname == 'unicode' else cp
                if enc is None:
                    continue
                bb = best([runTimedRss(mec, wdir, 'p5', p5Steps(files[enc]), enc, args.timeout)[0]
                           for _ in range(args.runs)])
                disp = [p5Display(mec, wdir, files[enc], enc, args.macros) for _ in range(args.runs)]
                disp = [dd for dd in disp if dd is not None]
                bb['disp'] = min(disp) if disp else None
                label = '%s %s' % (name, 'UTF-8' if enc == 'utf-8' else enc)
                print('%-18s %-8s' % (label, bname) + ''.join(ms(bb.get(cc)) + ' ' for cc in cols))
                sys.stdout.flush()
    finally:
        shutil.rmtree(wdir, ignore_errors=True)


if __name__ == '__main__':
    main()
