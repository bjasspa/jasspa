#!/usr/bin/env python3
#
# wingui.py - Windows GUI tests: runs a window build (mew) and types with PostMessageW.
#  - Input: a Unicode window, the title, typed non-ASCII and surrogate pair chars, Return and the
#    saved bytes.
#  - Rendering, by comparing captures of the window: lines of complex scripts (Tamil, Devanagari,
#    Kannada, Arabic) and wide chars, each padded so a '|' ends it in the same column, must end at
#    the same pixel; walking the cursor over every char must only change the cursor's cells, and
#    moving it back must restore the line exactly - a char must be drawn the same whatever is
#    drawn with it.
# The captures need the display on and unlocked. With --shots the window is captured to BMP files
# to be inspected, and the captures of a failed check are kept.
#
# Usage: python wingui.py <mew> [macros-dir] [--shots <dir>]

import ctypes
import os
import shutil
import struct
import subprocess
import sys
import tempfile
import time
import unicodedata
from ctypes import wintypes

TDIR = os.path.dirname(os.path.abspath(__file__))

u32 = ctypes.WinDLL("user32", use_last_error=True)
g32 = ctypes.WinDLL("gdi32", use_last_error=True)
u32.PostMessageW.argtypes = [wintypes.HWND, wintypes.UINT, wintypes.WPARAM, wintypes.LPARAM]
u32.GetDC.restype = wintypes.HDC
u32.GetDC.argtypes = [wintypes.HWND]
u32.ReleaseDC.argtypes = [wintypes.HWND, wintypes.HDC]
u32.PrintWindow.argtypes = [wintypes.HWND, wintypes.HDC, wintypes.UINT]
g32.CreateCompatibleDC.restype = wintypes.HDC
g32.CreateCompatibleDC.argtypes = [wintypes.HDC]
g32.CreateCompatibleBitmap.restype = wintypes.HANDLE
g32.CreateCompatibleBitmap.argtypes = [wintypes.HDC, ctypes.c_int, ctypes.c_int]
g32.SelectObject.restype = wintypes.HANDLE
g32.SelectObject.argtypes = [wintypes.HDC, wintypes.HANDLE]
g32.GetDIBits.argtypes = [wintypes.HDC, wintypes.HANDLE, wintypes.UINT, wintypes.UINT, ctypes.c_void_p, ctypes.c_void_p, wintypes.UINT]
g32.DeleteObject.argtypes = [wintypes.HANDLE]
g32.DeleteDC.argtypes = [wintypes.HDC]
WM_KEYDOWN, WM_KEYUP, WM_CHAR = 0x100, 0x101, 0x102

passed = failed = 0


def check(name, cond, detail=''):
    global passed, failed
    if cond:
        passed += 1
    else:
        failed += 1
        print('FAIL %s %s' % (name, detail))


def clusters(vals):
    # [start, end] of each run of consecutive values
    runs = []
    for vv in sorted(set(vals)):
        if runs and (vv == runs[-1][1] + 1):
            runs[-1][1] = vv
        else:
            runs.append([vv, vv])
    return runs


def cellsDiffer(aa, bb, xs, ys):
    # whether any pixel of the cells x xs[0] to xs[1], y ys[0] to ys[1] differs
    return any(aa.row(yy)[xs[0] * 3:xs[1] * 3] != bb.row(yy)[xs[0] * 3:xs[1] * 3] for yy in range(ys[0], ys[1]))


def meCells(ch):
    # cells ME gives a char: a zero width char is shown as a \uXXXX escape
    if unicodedata.category(ch) in ('Mn', 'Me', 'Cf'):
        return 6
    return 2 if unicodedata.east_asian_width(ch) in ('W', 'F') else 1


class BITMAPINFOHEADER(ctypes.Structure):
    _fields_ = [("biSize", wintypes.DWORD), ("biWidth", wintypes.LONG), ("biHeight", wintypes.LONG),
                ("biPlanes", wintypes.WORD), ("biBitCount", wintypes.WORD), ("biCompression", wintypes.DWORD),
                ("biSizeImage", wintypes.DWORD), ("biXPelsPerMeter", wintypes.LONG), ("biYPelsPerMeter", wintypes.LONG),
                ("biClrUsed", wintypes.DWORD), ("biClrImportant", wintypes.DWORD)]


class Image:
    """A top-down 24 bit capture of the window's client area"""
    def __init__(self, hwnd):
        rc = wintypes.RECT()
        u32.GetClientRect(hwnd, ctypes.byref(rc))
        self.ww, self.hh = rc.right, rc.bottom
        hdc = u32.GetDC(hwnd)
        mdc = g32.CreateCompatibleDC(hdc)
        bmp = g32.CreateCompatibleBitmap(hdc, self.ww, self.hh)
        g32.SelectObject(mdc, bmp)
        u32.PrintWindow(hwnd, mdc, 3)          # PW_CLIENTONLY|PW_RENDERFULLCONTENT
        self.bi = BITMAPINFOHEADER(ctypes.sizeof(BITMAPINFOHEADER), self.ww, -self.hh, 1, 24, 0, 0, 0, 0, 0, 0)
        self.stride = ((self.ww * 3 + 3) // 4) * 4
        buf = ctypes.create_string_buffer(self.stride * self.hh)
        g32.GetDIBits(mdc, bmp, 0, self.hh, buf, ctypes.byref(self.bi), 0)
        self.raw = buf.raw
        g32.DeleteObject(bmp)
        g32.DeleteDC(mdc)
        u32.ReleaseDC(hwnd, hdc)

    def row(self, yy):
        return self.raw[yy * self.stride:yy * self.stride + self.ww * 3]

    def pixel(self, xx, yy):
        oo = yy * self.stride + xx * 3
        return self.raw[oo:oo + 3]

    def save(self, fname):
        with open(fname, 'wb') as ff:
            ff.write(b'BM' + struct.pack('<IHHI', 54 + len(self.raw), 0, 0, 54) + bytes(self.bi) + self.raw)

    def diff(self, other, ymax, ymin=0):
        # {y: [x...]} of the differing pixels in each row from ymin to ymax
        dd = {}
        for yy in range(ymin, min(ymax, self.hh)):
            ra, rb = self.row(yy), other.row(yy)
            if ra != rb:
                dd[yy] = [xx for xx in range(self.ww) if ra[xx * 3:xx * 3 + 3] != rb[xx * 3:xx * 3 + 3]]
        return dd


class Mew:
    def __init__(self, mec, macros, lines, name='uni.txt'):
        self.home = tempfile.mkdtemp()
        self.tfile = os.path.join(self.home, name)
        with open(self.tfile, 'w', encoding='utf-8', newline='\n') as ff:
            ff.write('\n'.join(lines) + '\n')
        env = dict(os.environ)
        env.update(HOME=self.home, MEUSERPATH=self.home, MEPATH=macros, MENAME='test')
        self.proc = subprocess.Popen([mec, self.tfile], env=env)
        self.hwnd = None
        for _ in range(40):
            time.sleep(0.25)
            self.hwnd = self.findWindow()
            if self.hwnd is not None:
                break
        self.last = None
        if self.hwnd is not None:
            self.image(False)

    def findWindow(self):
        found = []

        @ctypes.WINFUNCTYPE(wintypes.BOOL, wintypes.HWND, wintypes.LPARAM)
        def cb(hwnd, lp):
            wp = wintypes.DWORD()
            u32.GetWindowThreadProcessId(hwnd, ctypes.byref(wp))
            if (wp.value == self.proc.pid) and u32.IsWindowVisible(hwnd):
                found.append(hwnd)
            return True
        u32.EnumWindows(cb, 0)
        return found[0] if found else None

    def char(self, cc):
        if cc > 0xffff:
            cc -= 0x10000
            u32.PostMessageW(self.hwnd, WM_CHAR, 0xd800 + (cc >> 10), 1)
            u32.PostMessageW(self.hwnd, WM_CHAR, 0xdc00 + (cc & 0x3ff), 1)
        else:
            u32.PostMessageW(self.hwnd, WM_CHAR, cc, 1)
        time.sleep(0.02)

    def text(self, ss):
        for ch in ss:
            self.char(ord(ch))

    def ret(self):
        # Return as the keyboard sends it, the key down is translated to a WM_CHAR
        u32.PostMessageW(self.hwnd, WM_KEYDOWN, 0x0d, 1 | (0x1c << 16))
        u32.PostMessageW(self.hwnd, WM_KEYUP, 0x0d, 1 | (0x1c << 16) | (3 << 30))
        time.sleep(0.02)

    def command(self, line):
        self.text('\x1bxexecute-line')
        self.ret()
        self.text(line)
        self.ret()

    def capture(self):
        # PrintWindow returns a blank image while the display is locked or asleep, and
        # occasionally otherwise; the menu line is never black
        for _ in range(100):
            img = Image(self.hwnd)
            if img.row(2).strip(b'\0'):
                return img
            time.sleep(0.05)
        raise RuntimeError('window capture is blank - the display must be on and unlocked')

    def image(self, changed=True, until=None, timeout=5.0):
        # ME's drawing is complete when three captures in a row are the same, the window has
        # changed from the last capture if it should and shows what until expects - ME does not
        # draw while it has keys to process so a pause is not enough
        time.sleep(0.05)
        prev, same = self.capture(), 0
        end = time.time() + timeout
        while time.time() < end:
            time.sleep(0.05)
            img = self.capture()
            same = (same + 1) if (img.raw == prev.raw) else 0
            if (same >= 2) and not (changed and (self.last is not None) and (img.raw == self.last.raw)) and \
               ((until is None) or until(img)):
                break
            prev = img
        self.last = img
        return img

    def close(self):
        self.proc.kill()
        self.proc.wait()
        shutil.rmtree(self.home, ignore_errors=True)


def testInput(mec, macros, shots):
    lines = ['Hello w\u00f6rld caf\u00e9', '\u0391\u03b2\u03b3 \u0434\u0430', '\u4e2d\u6587\u5b57 x |', 'a\U0001F600b |', 'end']
    mm = Mew(mec, macros, lines)
    try:
        check('window found', mm.hwnd is not None)
        if mm.hwnd is None:
            return
        check('window is unicode', bool(u32.IsWindowUnicode(mm.hwnd)))
        tb = ctypes.create_unicode_buffer(512)
        u32.GetWindowTextW(mm.hwnd, tb, 512)
        check('title', tb.value.startswith('uni.txt'), ascii(tb.value))
        if shots:
            mm.image(False).save(os.path.join(shots, 'wingui_start.bmp'))
        mm.text('\u00e9\u00df\u4e2d')
        mm.ret()
        mm.text('\U0001F600z')
        if shots:
            mm.image().save(os.path.join(shots, 'wingui_typed.bmp'))
        # C-x C-s
        mm.text('\x18\x13')
        mm.image()
        data = open(mm.tfile, 'rb').read()
        exp = ('\u00e9\u00df\u4e2d\n\U0001F600z' + '\n'.join(lines) + '\n').encode('utf-8')
        check('saved bytes', data == exp, ascii(data[:40]))
    finally:
        mm.close()


def testRendering(mec, macros, shots):
    texts = ['Tamil: \u0baf\u0bbe\u0bae\u0bb1\u0bbf\u0ba8\u0bcd\u0ba4 \u0bae\u0bca\u0bb4\u0bbf',
             'Deva: \u0928\u092e\u0938\u094d\u0924\u0947 \u0915\u093f',
             'Kannada: \u0c95\u0ca8\u0ccd\u0ca8\u0ca1',
             'Arabic: \u0645\u0631\u062d\u0628\u0627 \u0628\u0627\u0644\u0639\u0627\u0644\u0645',
             'Greek: \u0391\u03b2\u03b3 \u00e9',
             'CJK: \u4e2d\u6587\u5b57 abc',
             'Emoji: a\U0001F600b']
    endCol = 30
    lines = [tt + ' ' * (endCol - sum(meCells(ch) for ch in tt)) + '|' for tt in texts]
    # an ASCII first line for the geometry, drawn the same however the rest is drawn
    mm = Mew(mec, macros, ['ASCII: 0123456789 abc'] + lines + ['end'], 'render.txt')
    try:
        if mm.hwnd is None:
            check('render window found', False)
            return
        # a steady cursor and a mode line without the time so captures can be compared; setting
        # $cursor-blink to 0 leaves the cursor hidden if it is off, a long blink first shows it
        mm.command('set-variable $cursor-blink 65535')
        mm.command('set-variable $cursor-blink 0')
        mm.command('set-variable $buffer-mode-line "static"')
        mm.image(False)
        # the geometry from the cursor's positions, not its size which depends on the focus: the
        # cursor at the start of line 1 then line 3 gives the row height, then at column 10 of the
        # ASCII line 1 the cell width
        mm.text('\x0e\x1b<')
        img1 = mm.image()
        mm.text('\x0e\x0e')
        img2 = mm.image(until=lambda im: len(clusters(img1.diff(im, im.hh))) >= 2)
        rows = clusters(img1.diff(img2, img2.hh))
        check('row height found', (len(rows) >= 2) and (rows[1][0] - rows[0][0]) % 2 == 0, ascii(rows))
        if len(rows) < 2:
            return
        y1, ch = rows[0][0], (rows[1][0] - rows[0][0]) // 2
        yTop = y1 + ch
        mm.text('\x1b<' + '\x06' * 10)

        def xStarts(im):
            return [rr[0] for rr in clusters(xx for xs in img1.diff(im, y1 + ch, y1).values() for xx in xs)]
        img3 = mm.image(until=lambda im: len(xStarts(im)) >= 2)
        starts = xStarts(img3)
        x0 = starts[0] if starts else 0
        cw = (starts[-1] - x0) // 10 if len(starts) >= 2 else 0
        check('cell width found', (cw > 2) and (starts[-1] - x0 == 10 * cw), ascii(starts))
        if shots and not ((cw > 2) and (starts[-1] - x0 == 10 * cw)):
            for nm, im in (('cal1', img1), ('cal2', img2), ('cal3', img3)):
                im.save(os.path.join(shots, 'wingui_%s.bmp' % nm))
        # ignore the mode and message lines and the scroll bar
        ymax = img1.hh - 3 * ch
        xmax = x0 + (endCol + 5) * cw
        if shots:
            img1.save(os.path.join(shots, 'wingui_render.bmp'))

        # column layout: every line ends with its '|' in column endCol
        for ll, tt in enumerate(texts):
            yy = yTop + ll * ch
            xe = x0 + (endCol + 3) * cw
            bg = img1.pixel(xe, yy + ch // 2)
            ink = [xx for ry in range(yy, yy + ch) for xx in range(x0, xe) if img1.pixel(xx, ry) != bg]
            right = max(ink) if ink else -1
            check('column layout %s' % tt.split(':')[0], x0 + endCol * cw <= right < x0 + (endCol + 1) * cw,
                  'last ink at x %d, column %d is x %d' % (right, endCol, x0 + endCol * cw))

        # walk the cursor over each line: only the cursor's cells may change, returning restores it
        for ll, tt in enumerate(lines):
            yy = yTop + ll * ch
            # the cursor's cells at each char, a wide char's cursor covers both
            spans, col = [], 0
            for ch_ in tt + ' ':
                spans.append((x0 + col * cw, x0 + (col + (2 if meCells(ch_) == 2 else 1)) * cw))
                col += meCells(ch_)
            last = mm.last
            mm.text('\x1b<' + '\x0e' * (ll + 1))
            # the cursor is on the line's first cell
            ref = mm.image(until=lambda im: cellsDiffer(last, im, spans[0], (yy, yy + ch)))
            bad = ''
            for ii in range(1, len(spans)):
                mm.char(0x06)
                img = mm.image(until=lambda im: cellsDiffer(ref, im, spans[ii], (yy, yy + ch)))
                for ry, xs in ref.diff(img, ymax, yTop).items():
                    xs = [xx for xx in xs if xx < xmax]
                    out = [xx for xx in xs if not ((spans[0][0] <= xx < spans[0][1]) or (spans[ii][0] <= xx < spans[ii][1]))]
                    if xs and (not (yy <= ry < yy + ch) or out):
                        bad = 'after char %d %s: row %d x %s' % (ii, ascii(tt[ii - 1]), ry, (out or xs)[:4])
                        break
                if bad:
                    if shots:
                        ref.save(os.path.join(shots, 'wingui_fail_ref.bmp'))
                        img.save(os.path.join(shots, 'wingui_fail_img.bmp'))
                    break
            check('cursor walk %s' % tt.split(':')[0], not bad, bad)
            last = mm.last
            mm.text('\x01')
            img = mm.image(until=lambda im: cellsDiffer(last, im, spans[-1], (yy, yy + ch)))
            # the cursor's own cells may differ if the window's focus changed
            dd = [ry for ry, xs in ref.diff(img, ymax, yTop).items()
                  if [xx for xx in xs if (xx < xmax) and (not (yy <= ry < yy + ch) or not (spans[0][0] <= xx < spans[0][1]))]]
            check('cursor return %s' % tt.split(':')[0], not dd, ascii(dd[:3]))
    finally:
        mm.close()


def main():
    args = sys.argv[1:]
    shots = None
    if '--shots' in args:
        ii = args.index('--shots')
        shots = os.path.abspath(args[ii + 1])
        del args[ii:ii + 2]
    if len(args) < 1:
        sys.exit('Usage: wingui.py <mew> [macros-dir] [--shots <dir>]')
    mec = os.path.abspath(args[0])
    macros = os.path.abspath(args[1] if len(args) > 1 else os.path.join(TDIR, '..', '..', 'macros'))
    testInput(mec, macros, shots)
    testRendering(mec, macros, shots)
    print('%d passed, %d failed' % (passed, failed))
    sys.exit(1 if failed else 0)


if __name__ == '__main__':
    main()
