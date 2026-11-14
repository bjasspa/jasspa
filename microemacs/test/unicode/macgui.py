#!/usr/bin/env python3
#
# macgui.py - native macOS window build tests: runs the app (meunw) with the client-server
# enabled, sends it commands through the server socket and captures its window with
# screencapture, so no keys are sent (only the Screen Recording permission is needed).
#  - Column layout: lines of complex scripts, wide and unavailable chars, each padded so a '|'
#    ends it in the same column, must end at the same pixel.
#  - Glyphs: two different chars of each script must be drawn differently (a font fallback or
#    the font itself has them) or both as the unavailable char box.
#  - Stability: walking the cursor over each line of a file (by default the render lines, then
#    the non-ASCII lines of Kermit_UTF-8_Sampler.txt) must only change the cursor's old and new
#    cells, and moving it back must restore the line exactly - a char must be drawn the same
#    whatever is drawn with it. The cursor is found by its colour, so the cell widths are ME's.
# With --shots the captures of a failed check are kept as PNG files. ME_GLYPH_FALLBACK=0 in the
# environment tests the unavailable char box instead of the font fallback.
#
# Usage: python3 -I macgui.py <app-binary> [macros-dir] [--file <file>] [--max <chars>] [--shots <dir>]
#   <app-binary> is .../MicroEmacs.app/Contents/MacOS/MicroEmacs of a native build

import ctypes
import os
import re
import shutil
import struct
import subprocess
import sys
import tempfile
import time
import unicodedata
import zlib

TDIR = os.path.dirname(os.path.abspath(__file__))
passed = failed = 0


def check(name, cond, detail=''):
    global passed, failed
    if cond:
        passed += 1
    else:
        failed += 1
        print('FAIL %s %s' % (name, detail))


def meCells(ch):
    # cells ME gives a char: a zero width char is shown as a \uXXXX escape
    if unicodedata.category(ch) in ('Mn', 'Me', 'Cf'):
        return 6
    return 2 if unicodedata.east_asian_width(ch) in ('W', 'F') else 1


# ---- the window list, through CoreGraphics -------------------------------------------------
cf = ctypes.CDLL('/System/Library/Frameworks/CoreFoundation.framework/CoreFoundation')
cg = ctypes.CDLL('/System/Library/Frameworks/CoreGraphics.framework/CoreGraphics')
cf.CFArrayGetCount.restype, cf.CFArrayGetCount.argtypes = ctypes.c_long, [ctypes.c_void_p]
cf.CFArrayGetValueAtIndex.restype, cf.CFArrayGetValueAtIndex.argtypes = ctypes.c_void_p, [ctypes.c_void_p, ctypes.c_long]
cf.CFDictionaryGetValue.restype, cf.CFDictionaryGetValue.argtypes = ctypes.c_void_p, [ctypes.c_void_p, ctypes.c_void_p]
cf.CFStringCreateWithCString.restype = ctypes.c_void_p
cf.CFStringCreateWithCString.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_uint32]
cf.CFNumberGetValue.restype, cf.CFNumberGetValue.argtypes = ctypes.c_bool, [ctypes.c_void_p, ctypes.c_int, ctypes.c_void_p]
cf.CFRelease.argtypes = [ctypes.c_void_p]
cg.CGWindowListCopyWindowInfo.restype, cg.CGWindowListCopyWindowInfo.argtypes = ctypes.c_void_p, [ctypes.c_uint32, ctypes.c_uint32]


def cfKey(name):
    return cf.CFStringCreateWithCString(None, name.encode(), 0x08000100)


KEY_PID, KEY_NUM, KEY_LAYER = cfKey('kCGWindowOwnerPID'), cfKey('kCGWindowNumber'), cfKey('kCGWindowLayer')
KEY_BOUNDS, KEY_W, KEY_H = cfKey('kCGWindowBounds'), cfKey('Width'), cfKey('Height')


def cfInt(dd, key):
    vv = cf.CFDictionaryGetValue(dd, key)
    nn = ctypes.c_int64(0)
    if vv:
        cf.CFNumberGetValue(vv, 4, ctypes.byref(nn))
    return nn.value


def windowOf(pid):
    # the number of pid's largest normal (layer 0) window, None if it has none yet; while it
    # starts the app also has a small blank window
    best, area = None, 0
    arr = cg.CGWindowListCopyWindowInfo(0, 0)
    try:
        for ii in range(cf.CFArrayGetCount(arr)):
            dd = cf.CFArrayGetValueAtIndex(arr, ii)
            if (cfInt(dd, KEY_PID) == pid) and (cfInt(dd, KEY_LAYER) == 0):
                bb = cf.CFDictionaryGetValue(dd, KEY_BOUNDS)
                aa = (cfInt(bb, KEY_W) * cfInt(bb, KEY_H)) if bb else 0
                if aa > area:
                    best, area = cfInt(dd, KEY_NUM), aa
    finally:
        cf.CFRelease(arr)
    return best


# ---- captures -----------------------------------------------------------------------------
class Image:
    """A capture of a window as top-down packed RGB rows"""

    def __init__(self, fname):
        dd = open(fname, 'rb').read()
        off = struct.unpack('<I', dd[10:14])[0]
        self.ww, hh, _, bpp = struct.unpack('<iiHH', dd[18:30])
        self.hh = abs(hh)
        bpr = ((self.ww * bpp // 8) + 3) & ~3
        rows = [dd[off + yy * bpr:off + yy * bpr + self.ww * bpp // 8] for yy in range(self.hh)]
        if hh > 0:
            rows.reverse()
        data = b''.join(rows)
        step = bpp // 8
        rgb = bytearray(self.ww * self.hh * 3)
        rgb[0::3], rgb[1::3], rgb[2::3] = data[2::step], data[1::step], data[0::step]
        self.raw = bytes(rgb)
        self.stride = self.ww * 3

    def row(self, yy):
        return self.raw[yy * self.stride:(yy + 1) * self.stride]

    def pixel(self, xx, yy):
        oo = yy * self.stride + xx * 3
        return self.raw[oo:oo + 3]

    def block(self, xs, ys):
        return b''.join(self.row(yy)[xs[0] * 3:xs[1] * 3] for yy in range(ys[0], ys[1]))

    def save(self, fname):
        def chunk(tt, bb):
            return struct.pack('>I', len(bb)) + tt + bb + struct.pack('>I', zlib.crc32(tt + bb) & 0xffffffff)
        rows = b''.join(b'\0' + self.row(yy) for yy in range(self.hh))
        with open(fname, 'wb') as ff:
            ff.write(b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>IIBBBBB', self.ww, self.hh, 8, 2, 0, 0, 0)) +
                     chunk(b'IDAT', zlib.compress(rows)) + chunk(b'IEND', b''))

    def diff(self, other, ymin, ymax):
        # {y: [x...]} of the differing pixels in each row from ymin to ymax
        dd = {}
        for yy in range(ymin, min(ymax, self.hh)):
            ra, rb = self.row(yy), other.row(yy)
            if ra != rb:
                dd[yy] = [xx for xx in range(self.ww) if ra[xx * 3:xx * 3 + 3] != rb[xx * 3:xx * 3 + 3]]
        return dd


class App:
    """Runs the app with a start-up macro (instead of me.emf) that loads fname and then runs each
    step: its commands, a screen update, then it makes the marker directory and waits until it
    is removed. steps() yields each step's capture, taken when the marker appears. No keys, timers
    or client-server are used. To wait in a macro use command-wait (ME has no sleep command)"""

    def __init__(self, app, macros, fname, steps):
        self.home = tempfile.mkdtemp()
        self.marker = os.path.join(self.home, 'step')
        self.cap = os.path.join(self.home, 'cap.bmp')
        self.nsteps = len(steps)
        self.last = None
        def q(ss):
            return '"%s"' % ss.replace('\\', '\\\\').replace('"', '\\"')
        mac = ['define-macro start-up',
               '  !force 0x20 file-op %s' % q(self.marker),
               '  find-file %s' % q(fname),
               # a steady cursor and a mode line without the time so captures can be compared;
               # setting $cursor-blink to 0 leaves the cursor hidden if it is off, a long blink first shows it
               '  set-variable $cursor-blink 65535',
               '  set-variable $cursor-blink 0',
               '  set-variable $buffer-mode-line "static"',
               # a cursor colour no text has, so the cursor can be found in a capture
               '  add-color 250 255 192 0',
               '  set-variable $cursor-color 250']
        for st in steps:
            mac += ['  ' + cc for cc in st]
            mac += ['  screen-update',
                    '  0x100 file-op %s' % q(self.marker),
                    '  !while &not &seq &stat "t" %s "X"' % q(self.marker),
                    '  !done']
        mac += ['  4 exit-emacs 0', '!emacro']
        sfile = os.path.join(self.home, 'macgui.emf')
        with open(sfile, 'w', encoding='utf-8') as ff:
            ff.write('\n'.join(mac) + '\n')
        env = dict(os.environ)
        env.update(HOME=self.home, MEPATH=self.home + ':' + macros, MENAME='test')
        self.proc = subprocess.Popen([app, '@' + sfile], env=env, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        self.win = None

    def steps(self):
        for ii in range(self.nsteps):
            end = time.time() + 20
            while not os.path.isdir(self.marker):
                if (time.time() > end) or (self.proc.poll() is not None):
                    raise RuntimeError('step %d not reached' % ii)
                time.sleep(0.01)
            if self.win is None:
                self.win = windowOf(self.proc.pid)
            img = self.image()
            os.rmdir(self.marker)
            yield img

    def capture(self):
        for _ in range(20):
            if (self.win is not None) and \
               (subprocess.call(['screencapture', '-x', '-o', '-t', 'bmp', '-l', str(self.win), self.cap],
                                stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL) == 0):
                return Image(self.cap)
            time.sleep(0.1)
            self.win = windowOf(self.proc.pid)
        raise RuntimeError('window capture failed - is Screen Recording allowed for this terminal?')

    def image(self, timeout=5.0):
        # drawing is complete when three captures in a row are the same (the cursor is drawn
        # after the text)
        prev, same = self.capture(), 0
        img = prev
        end = time.time() + timeout
        while (time.time() < end) and (same < 2):
            time.sleep(0.03)
            img = self.capture()
            same = (same + 1) if (img.raw == prev.raw) else 0
            prev = img
        return img

    def close(self):
        if self.proc.poll() is None:
            end = time.time() + 5
            while (self.proc.poll() is None) and (time.time() < end):
                time.sleep(0.1)
            if self.proc.poll() is None:
                self.proc.kill()
        self.proc.wait()
        shutil.rmtree(self.home, ignore_errors=True)


def cursorBox(img, color, ymin, ymax):
    # (x0, y0, x1, y1) of the pixels of the cursor's colour in rows ymin to ymax, None if none
    pp = color
    xs, ys = [], []
    for yy in range(ymin, ymax):
        rr = img.row(yy)
        oo = rr.find(pp)
        while oo >= 0:
            if oo % 3 == 0:
                xs.append(oo // 3)
                ys.append(yy)
            oo = rr.find(pp, oo + 1)
    if not xs:
        return None
    return (min(xs), min(ys), max(xs) + 1, max(ys) + 1)


# the first steps move the cursor away and back as the window's first drawing differs slightly
# (glyph edge pixels) from its later drawing
GEOMETRY_STEPS = [['forward-line'], ['backward-line'], ['beginning-of-buffer'], ['forward-line'], ['beginning-of-buffer']]
CURSOR_COLOR = b'\xff\xc0\x00'       # add-color 250 255 192 0 in the start-up macro


class Geometry:
    """The text area from the GEOMETRY_STEPS captures: the cursor (CURSOR_COLOR) at the start of
    the first line (an ASCII char) then one line down"""

    def __init__(self, it):
        self.ok = False
        self.color = CURSOR_COLOR
        next(it), next(it)
        img, img2, self.home = next(it), next(it), next(it)
        box = cursorBox(img, self.color, 0, img.hh)
        box2 = cursorBox(img2, self.color, 0, img.hh)
        if (box is not None) and (box2 is not None) and (box2[1] > box[1]):
            self.ok = True
            self.x0, self.y0 = box[0], box[1]
            self.cw, self.ch = box[2] - box[0], box2[1] - box[1]
            self.cols = (img.ww - self.x0) // self.cw
            # the text rows, not the mode or message line
            self.ymax = self.y0 + ((img.hh - self.y0) // self.ch - 2) * self.ch


def walkSteps(lineNo, text, maxChars):
    # the steps walking the cursor over line lineNo and back to its start
    nn = min(len(text), maxChars)
    return [['goto-line %d' % lineNo, 'beginning-of-line']] + [['forward-char']] * nn + [['beginning-of-line']]


def walkCheck(it, geo, text, maxChars, shots, tag):
    # checks the captures of walkSteps, returns '' or the first problem. Each char may only change
    # the cursor's old and new cells (its boxes as drawn, as a cell may be a pixel taller on a
    # scaled display, plus a pixel), returning must restore the line
    nn = min(len(text), maxChars)
    ref = next(it)
    imgs = [next(it) for _ in range(nn)]
    back = next(it)
    rbox = cursorBox(ref, geo.color, geo.y0, geo.ymax)
    if rbox is None:
        return 'cursor not found'

    def inside(bb, xx, yy):
        return (bb[0] - 1 <= xx <= bb[2]) and (bb[1] - 1 <= yy <= bb[3])
    col = 0
    for ii, img in enumerate(imgs):
        # stop before the line scrolls horizontally (the cursor reaches the last columns)
        col += meCells(text[ii])
        if col >= geo.cols - 3:
            break
        box = cursorBox(img, geo.color, geo.y0, geo.ymax)
        if (box is None) or (abs(box[1] - rbox[1]) > geo.ch // 2):
            break               # the cursor left the line
        for ry, xs in ref.diff(img, geo.y0, geo.ymax).items():
            out = [xx for xx in xs if not (inside(rbox, xx, ry) or inside(box, xx, ry))]
            if out:
                if shots:
                    ref.save(os.path.join(shots, 'macgui_%s_ref.png' % tag))
                    img.save(os.path.join(shots, 'macgui_%s_fail.png' % tag))
                return 'after char %d %s: row %d x %s' % (ii + 1, ascii(text[ii]), ry, out[:4])
    dd = ref.diff(back, geo.y0, geo.ymax)
    if dd:
        if shots:
            ref.save(os.path.join(shots, 'macgui_%s_ref.png' % tag))
            back.save(os.path.join(shots, 'macgui_%s_return.png' % tag))
        return 'returning changed rows %s' % sorted(dd)[:3]
    return ''


RENDER_TEXTS = ['Tamil: யாமறிந்த மொழி',
                'Deva: नमस्ते कि',
                'Kannada: ಕನ್ನಡ',
                'Arabic: مرحبا بالعالم',
                'Greek: Αβγ é',
                'CJK: 中文字 abc',
                'Emoji: a\U0001F600b']
RENDER_PAIRS = [('Greek', 'βγ'), ('Cyrillic', 'дж'), ('Box', '┌┐'), ('CJK', '中文'),
                ('Hiragana', 'あい'), ('Hangul', '한글'), ('Emoji', '\U0001F600\U0001F680'),
                ('Tamil', 'கந'), ('Deva', 'नम'), ('Arabic', 'مر'), ('Thai', 'กข'),
                ('Unassigned', '\U000E0F00\U000E0F01')]


def testRender(appPath, macros, maxChars, shots):
    endCol = 30
    lines = [tt + ' ' * (endCol - sum(meCells(ch) for ch in tt)) + '|' for tt in RENDER_TEXTS]
    pairLines = ['P: ' + pp for _, pp in RENDER_PAIRS]
    allLines = ['ASCII: 0123456789 abc'] + lines + pairLines + ['end']
    walks = [(ll + 1, tt) for ll, tt in enumerate(allLines) if any(ord(cc) >= 0x80 for cc in tt)]
    wdir = tempfile.mkdtemp()
    fname = os.path.join(wdir, 'render.txt')
    with open(fname, 'w', encoding='utf-8', newline='\n') as ff:
        ff.write('\n'.join(allLines) + '\n')
    steps = list(GEOMETRY_STEPS)
    for lineNo, tt in walks:
        steps += walkSteps(lineNo, tt, maxChars)
    app = App(appPath, macros, fname, steps)
    try:
        it = app.steps()
        geo = Geometry(it)
        check('geometry found', geo.ok)
        if not geo.ok:
            return
        img = geo.home
        if shots:
            img.save(os.path.join(shots, 'macgui_render.png'))
        # column layout: every padded line ends with its '|' in column endCol
        for ll, tt in enumerate(RENDER_TEXTS):
            yy = geo.y0 + (ll + 1) * geo.ch
            xe = geo.x0 + (endCol + 3) * geo.cw
            bg = img.pixel(xe, yy + geo.ch // 2)
            ink = [xx for ry in range(yy, yy + geo.ch) for xx in range(geo.x0, xe) if img.pixel(xx, ry) != bg]
            right = max(ink) if ink else -1
            check('column layout %s' % tt.split(':')[0], geo.x0 + endCol * geo.cw <= right < geo.x0 + (endCol + 1) * geo.cw,
                  'last ink at x %d, column %d is x %d' % (right, endCol, geo.x0 + endCol * geo.cw))
        # glyphs: the two chars of a pair are drawn differently, or both as the box
        noGlyph = []
        for pp, (nm, pair) in enumerate(RENDER_PAIRS):
            yy = geo.y0 + (len(lines) + 1 + pp) * geo.ch
            ww = meCells(pair[0]) * geo.cw
            xa, xb = geo.x0 + 3 * geo.cw, geo.x0 + 3 * geo.cw + ww
            aa = img.block((xa, xb), (yy, yy + geo.ch))
            bb = img.block((xb, xb + ww), (yy, yy + geo.ch))
            if aa == bb:
                noGlyph.append(nm)
            check('glyphs %s' % nm, (aa != bb) or (nm == 'Unassigned') or (os.environ.get('ME_GLYPH_FALLBACK') == '0'),
                  'both chars drawn the same')
        if noGlyph:
            print('INFO drawn as the unavailable char box: %s' % ' '.join(noGlyph))
        # stability
        for lineNo, tt in walks:
            bad = walkCheck(it, geo, tt, maxChars, shots, 'render%d' % lineNo)
            check('cursor walk render line %d %s' % (lineNo, tt.split(':')[0]), not bad, bad)
    finally:
        app.close()
        shutil.rmtree(wdir, ignore_errors=True)


def testFile(appPath, macros, fname, maxChars, shots):
    lines = open(fname, encoding='utf-8', errors='replace').read().split('\n')
    # one line per script: a line with non-ASCII letters whose first such letter's script is new
    todo, seen = [], set()
    for ll, tt in enumerate(lines):
        if '\t' in tt:
            continue
        for cc in tt:
            if (ord(cc) >= 0x80) and unicodedata.category(cc).startswith('L'):
                script = unicodedata.name(cc, '?').split(' ')[0]
                if script not in seen:
                    seen.add(script)
                    todo.append((ll + 1, tt, script))
                break
    steps = list(GEOMETRY_STEPS)
    for lineNo, tt, _ in todo:
        steps += walkSteps(lineNo, tt, maxChars)
    app = App(appPath, macros, fname, steps)
    try:
        it = app.steps()
        geo = Geometry(it)
        check('%s geometry found' % os.path.basename(fname), geo.ok)
        if not geo.ok:
            return
        for lineNo, tt, script in todo:
            bad = walkCheck(it, geo, tt, maxChars, shots, 'line%d' % lineNo)
            check('cursor walk line %d %s' % (lineNo, script), not bad, bad)
    finally:
        app.close()


def main():
    args = sys.argv[1:]
    opts = {'--file': os.path.join(TDIR, 'Kermit_UTF-8_Sampler.txt'), '--max': '40', '--shots': None}
    for opt in list(opts):
        if opt in args:
            ii = args.index(opt)
            opts[opt] = args[ii + 1]
            del args[ii:ii + 2]
    if len(args) < 1:
        sys.exit('Usage: macgui.py <app-binary> [macros-dir] [--file <file>] [--max <chars>] [--shots <dir>]')
    appPath = os.path.abspath(args[0])
    macros = os.path.abspath(args[1] if len(args) > 1 else os.path.join(TDIR, '..', '..', 'macros'))
    shots = os.path.abspath(opts['--shots']) if opts['--shots'] else None
    if shots:
        os.makedirs(shots, exist_ok=True)
    maxChars = int(opts['--max'])
    testRender(appPath, macros, maxChars, shots)
    if opts['--file'] != '-':
        testFile(appPath, macros, opts['--file'], maxChars, shots)
    print('%d passed, %d failed' % (passed, failed))
    sys.exit(1 if failed else 0)


if __name__ == '__main__':
    main()
