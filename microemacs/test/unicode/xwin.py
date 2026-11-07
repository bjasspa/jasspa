#!/usr/bin/env python3
#
# xwin.py - X11 window build tests: runs a window build (mew) on its own Xvfb server (or a given
# display), types with XTest and captures the window with XGetImage, all through ctypes.
#  - Input: typed non-ASCII and non-BMP chars, Return and the saved bytes, with the locale unset,
#    C and C.UTF-8.
#  - Title: a non-ASCII file name in _NET_WM_NAME (UTF8_STRING) and a WM_NAME that is not UTF-8
#    bytes in a Latin-1 STRING.
#  - Clipboard: a region copied in ME is offered as UTF8_STRING, a UTF8_STRING selection owned by
#    another client is yanked into ME.
#  - Rendering, by comparing captures of the window: lines of wide chars and complex scripts, each
#    padded so a '|' ends it in the same column, must end at the same pixel; two different chars of
#    a script must be drawn differently, or both as ME's box with a diagonal if the font has no
#    glyph for them (as the Windows build, there is no font fallback);
#    walking the cursor over every char must only change the cursor's cells, and moving it back
#    must restore the line exactly.
# With --shots the window is captured to PNG files to be inspected, and the captures of a failed
# check are kept.
#
# Usage: python3 -I xwin.py <mew> [macros-dir] [--display <display>] [--shots <dir>]

import ctypes
import ctypes.util
import os
import shutil
import struct
import subprocess
import sys
import tempfile
import threading
import time
import unicodedata
import zlib

TDIR = os.path.dirname(os.path.abspath(__file__))

c_int, c_uint, c_long, c_ulong, c_char_p, c_void_p = ctypes.c_int, ctypes.c_uint, ctypes.c_long, ctypes.c_ulong, ctypes.c_char_p, ctypes.c_void_p
Window = Atom = KeySym = c_ulong

passed = failed = 0


def check(name, cond, detail=''):
    global passed, failed
    if cond:
        passed += 1
    else:
        failed += 1
        print('FAIL %s %s' % (name, detail))


def loadLib(name, major):
    for pp in (ctypes.util.find_library(name), '/opt/X11/lib/lib%s.dylib' % name, '/usr/X11R6/lib/lib%s.dylib' % name,
               'lib%s.so.%d' % (name, major)):
        if pp:
            try:
                return ctypes.CDLL(pp)
            except OSError:
                pass
    sys.exit('Cannot load lib%s' % name)


x11 = loadLib('X11', 6)
xtst = loadLib('Xtst', 6)


class XImage(ctypes.Structure):
    pass


XImage._fields_ = [('width', c_int), ('height', c_int), ('xoffset', c_int), ('format', c_int), ('data', c_void_p),
                   ('byte_order', c_int), ('bitmap_unit', c_int), ('bitmap_bit_order', c_int), ('bitmap_pad', c_int),
                   ('depth', c_int), ('bytes_per_line', c_int), ('bits_per_pixel', c_int),
                   ('red_mask', c_ulong), ('green_mask', c_ulong), ('blue_mask', c_ulong), ('obdata', c_void_p),
                   ('create_image', c_void_p), ('destroy_image', ctypes.CFUNCTYPE(c_int, ctypes.POINTER(XImage))),
                   ('get_pixel', c_void_p), ('put_pixel', c_void_p), ('sub_image', c_void_p), ('add_pixel', c_void_p)]


class XSelectionRequestEvent(ctypes.Structure):
    _fields_ = [('type', c_int), ('serial', c_ulong), ('send_event', c_int), ('display', c_void_p), ('owner', Window),
                ('requestor', Window), ('selection', Atom), ('target', Atom), ('property', Atom), ('time', c_ulong)]


class XSelectionEvent(ctypes.Structure):
    _fields_ = [('type', c_int), ('serial', c_ulong), ('send_event', c_int), ('display', c_void_p), ('requestor', Window),
                ('selection', Atom), ('target', Atom), ('property', Atom), ('time', c_ulong)]


class XEvent(ctypes.Union):
    _fields_ = [('type', c_int), ('xselectionrequest', XSelectionRequestEvent), ('xselection', XSelectionEvent),
                ('pad', c_long * 24)]


SelectionRequest, SelectionNotify = 30, 31
ZPixmap, AnyPropertyType, PropModeReplace = 2, 0, 0
XK_Return, XK_Escape, XK_Shift_L, XK_Control_L = 0xff0d, 0xff1b, 0xffe1, 0xffe3

for fn, res, args in (
        ('XInitThreads', c_int, []),
        ('XOpenDisplay', c_void_p, [c_char_p]),
        ('XCloseDisplay', c_int, [c_void_p]),
        ('XDefaultRootWindow', Window, [c_void_p]),
        ('XSetErrorHandler', c_void_p, [c_void_p]),
        ('XSync', c_int, [c_void_p, c_int]),
        ('XFlush', c_int, [c_void_p]),
        ('XFree', c_int, [c_void_p]),
        ('XInternAtom', Atom, [c_void_p, c_char_p, c_int]),
        ('XGetAtomName', c_void_p, [c_void_p, Atom]),
        ('XQueryTree', c_int, [c_void_p, Window, ctypes.POINTER(Window), ctypes.POINTER(Window),
                               ctypes.POINTER(ctypes.POINTER(Window)), ctypes.POINTER(c_uint)]),
        ('XGetGeometry', c_int, [c_void_p, Window, ctypes.POINTER(Window), ctypes.POINTER(c_int), ctypes.POINTER(c_int),
                                 ctypes.POINTER(c_uint), ctypes.POINTER(c_uint), ctypes.POINTER(c_uint), ctypes.POINTER(c_uint)]),
        ('XGetWindowProperty', c_int, [c_void_p, Window, Atom, c_long, c_long, c_int, Atom, ctypes.POINTER(Atom),
                                       ctypes.POINTER(c_int), ctypes.POINTER(c_ulong), ctypes.POINTER(c_ulong),
                                       ctypes.POINTER(ctypes.POINTER(ctypes.c_ubyte))]),
        ('XChangeProperty', c_int, [c_void_p, Window, Atom, Atom, c_int, c_int, c_void_p, c_int]),
        ('XGetImage', ctypes.POINTER(XImage), [c_void_p, Window, c_int, c_int, c_uint, c_uint, c_ulong, c_int]),
        ('XSetInputFocus', c_int, [c_void_p, Window, c_int, c_ulong]),
        ('XDisplayKeycodes', c_int, [c_void_p, ctypes.POINTER(c_int), ctypes.POINTER(c_int)]),
        ('XGetKeyboardMapping', ctypes.POINTER(KeySym), [c_void_p, ctypes.c_ubyte, c_int, ctypes.POINTER(c_int)]),
        ('XChangeKeyboardMapping', c_int, [c_void_p, c_int, c_int, ctypes.POINTER(KeySym), c_int]),
        ('XCreateSimpleWindow', Window, [c_void_p, Window, c_int, c_int, c_uint, c_uint, c_uint, c_ulong, c_ulong]),
        ('XDestroyWindow', c_int, [c_void_p, Window]),
        ('XSetSelectionOwner', c_int, [c_void_p, Atom, Window, c_ulong]),
        ('XGetSelectionOwner', Window, [c_void_p, Atom]),
        ('XConvertSelection', c_int, [c_void_p, Atom, Atom, Atom, Window, c_ulong]),
        ('XPending', c_int, [c_void_p]),
        ('XNextEvent', c_int, [c_void_p, ctypes.POINTER(XEvent)]),
        ('XSendEvent', c_int, [c_void_p, Window, c_int, c_long, ctypes.POINTER(XEvent)])):
    ff = getattr(x11, fn)
    ff.restype, ff.argtypes = res, args
xtst.XTestFakeKeyEvent.restype, xtst.XTestFakeKeyEvent.argtypes = c_int, [c_void_p, c_uint, c_int, c_ulong]

# an X error (e.g. capturing a window not yet mapped) must not exit the test
xerrors = []
XERRORHANDLER = ctypes.CFUNCTYPE(c_int, c_void_p, c_void_p)(lambda dd, ee: xerrors.append(1) or 0)
x11.XInitThreads()
x11.XSetErrorHandler(ctypes.cast(XERRORHANDLER, c_void_p))


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


def keysymOf(ch):
    cp = ord(ch)
    return cp if (0x20 <= cp <= 0x7e) or (0xa0 <= cp <= 0xff) else 0x1000000 + cp


class Display:
    def __init__(self, name):
        self.name = name
        self.dpy = x11.XOpenDisplay(name.encode())
        if not self.dpy:
            raise RuntimeError('cannot open display %s' % name)
        self.root = x11.XDefaultRootWindow(self.dpy)
        self.atoms = {}

    def atom(self, name):
        if name not in self.atoms:
            self.atoms[name] = x11.XInternAtom(self.dpy, name.encode(), 0)
        return self.atoms[name]

    def atomName(self, aa):
        if not aa:
            return None
        pp = x11.XGetAtomName(self.dpy, aa)
        ss = ctypes.string_at(pp).decode('latin-1')
        x11.XFree(pp)
        return ss

    def children(self, win):
        rr, pp, cc, nn = Window(), Window(), ctypes.POINTER(Window)(), c_uint()
        if not x11.XQueryTree(self.dpy, win, ctypes.byref(rr), ctypes.byref(pp), ctypes.byref(cc), ctypes.byref(nn)):
            return []
        ret = [cc[ii] for ii in range(nn.value)]
        if cc:
            x11.XFree(cc)
        return ret

    def prop(self, win, name):
        # (type name, bytes or a list of longs for format 32) of a window property, None if not set
        at, af, ni, ba = Atom(), c_int(), c_ulong(), c_ulong()
        pp = ctypes.POINTER(ctypes.c_ubyte)()
        if x11.XGetWindowProperty(self.dpy, win, self.atom(name), 0, 65536, 0, AnyPropertyType, ctypes.byref(at),
                                  ctypes.byref(af), ctypes.byref(ni), ctypes.byref(ba), ctypes.byref(pp)) != 0:
            return None
        try:
            if not at.value:
                return None
            if af.value == 32:
                data = [ctypes.cast(pp, ctypes.POINTER(c_ulong))[ii] for ii in range(ni.value)]
            else:
                data = ctypes.string_at(pp, ni.value * (af.value // 8))
            return (self.atomName(at.value), data)
        finally:
            if pp:
                x11.XFree(pp)

    def geometry(self, win):
        rr, xx, yy, ww, hh, bw, dd = Window(), c_int(), c_int(), c_uint(), c_uint(), c_uint(), c_uint()
        if not x11.XGetGeometry(self.dpy, win, ctypes.byref(rr), ctypes.byref(xx), ctypes.byref(yy), ctypes.byref(ww),
                                ctypes.byref(hh), ctypes.byref(bw), ctypes.byref(dd)):
            return None
        return (ww.value, hh.value)

    def close(self):
        x11.XCloseDisplay(self.dpy)


class Keyboard:
    """Types chars with XTest. ASCII uses the server's keys (with shift if needed); any other char
    is mapped to a spare keycode before ME starts, as ME does not reload a changed mapping"""

    def __init__(self, disp):
        self.disp = disp
        dpy = disp.dpy
        mn, mx = c_int(), c_int()
        x11.XDisplayKeycodes(dpy, ctypes.byref(mn), ctypes.byref(mx))
        per = c_int()
        nn = mx.value - mn.value + 1
        kmap = x11.XGetKeyboardMapping(dpy, mn.value, nn, ctypes.byref(per))
        self.per = per.value
        self.table = {}
        self.spare = []
        self.changed = []
        for ii in range(nn):
            syms = [kmap[ii * self.per + jj] for jj in range(self.per)]
            if not any(syms):
                self.spare.append(mn.value + ii)
                continue
            for lvl in range(min(2, self.per)):
                if syms[lvl] and (syms[lvl] not in self.table):
                    self.table[syms[lvl]] = (mn.value + ii, lvl)
        x11.XFree(kmap)

    def addChars(self, text):
        for ch in text:
            ks = keysymOf(ch)
            if (ord(ch) >= 0x20) and (ks not in self.table):
                kc = self.spare.pop()
                arr = (KeySym * self.per)(*([ks] * self.per))
                x11.XChangeKeyboardMapping(self.disp.dpy, kc, self.per, arr, 1)
                self.table[ks] = (kc, 0)
                self.changed.append(kc)
        x11.XSync(self.disp.dpy, 0)

    def restore(self):
        for kc in self.changed:
            arr = (KeySym * self.per)()
            x11.XChangeKeyboardMapping(self.disp.dpy, kc, self.per, arr, 1)
        x11.XSync(self.disp.dpy, 0)

    def tap(self, ks, mods=()):
        dpy = self.disp.dpy
        kc, lvl = self.table[ks]
        mods = list(mods) + ([XK_Shift_L] if lvl else [])
        for mm in mods:
            xtst.XTestFakeKeyEvent(dpy, self.table[mm][0], 1, 0)
        xtst.XTestFakeKeyEvent(dpy, kc, 1, 0)
        xtst.XTestFakeKeyEvent(dpy, kc, 0, 0)
        for mm in reversed(mods):
            xtst.XTestFakeKeyEvent(dpy, self.table[mm][0], 0, 0)
        x11.XSync(dpy, 0)
        time.sleep(0.01)

    def char(self, ch):
        cp = ord(ch)
        if ch == '\r':
            self.tap(XK_Return)
        elif ch == '\x1b':
            self.tap(XK_Escape)
        elif cp < 0x20:
            self.tap(cp + 0x60, [XK_Control_L])
        else:
            self.tap(keysymOf(ch))

    def text(self, ss):
        for ch in ss:
            self.char(ch)


class Image:
    """A capture of the window as packed RGB rows"""

    def __init__(self, disp, win):
        self.raw = None
        geo = disp.geometry(win)
        if geo is None:
            return
        self.ww, self.hh = geo
        im = x11.XGetImage(disp.dpy, win, 0, 0, self.ww, self.hh, 0xffffffff, ZPixmap)
        if not im:
            return
        ii = im.contents
        try:
            if ii.bits_per_pixel != 32:
                raise RuntimeError('only 32 bit visuals are supported, not %d' % ii.bits_per_pixel)
            data = ctypes.string_at(ii.data, ii.bytes_per_line * ii.height)
            if ii.bytes_per_line != self.ww * 4:
                data = b''.join(data[yy * ii.bytes_per_line:yy * ii.bytes_per_line + self.ww * 4] for yy in range(self.hh))
            rgb = bytearray(self.ww * self.hh * 3)
            ro, go, bo = (2, 1, 0) if ii.byte_order == 0 else (1, 2, 3)
            rgb[0::3], rgb[1::3], rgb[2::3] = data[ro::4], data[go::4], data[bo::4]
            self.raw = bytes(rgb)
        finally:
            ii.destroy_image(im)
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

    def diff(self, other, ymax, ymin=0):
        # {y: [x...]} of the differing pixels in each row from ymin to ymax
        dd = {}
        for yy in range(ymin, min(ymax, self.hh)):
            ra, rb = self.row(yy), other.row(yy)
            if ra != rb:
                dd[yy] = [xx for xx in range(self.ww) if ra[xx * 3:xx * 3 + 3] != rb[xx * 3:xx * 3 + 3]]
        return dd


class Mew:
    def __init__(self, disp, kbd, mec, macros, lines, name='uni.txt', lang=None):
        self.disp, self.kbd = disp, kbd
        self.home = tempfile.mkdtemp()
        self.name = name
        self.tfile = os.path.join(self.home, name)
        with open(self.tfile, 'w', encoding='utf-8', newline='\n') as ff:
            ff.write('\n'.join(lines) + '\n')
        env = dict(os.environ)
        for vv in ('LANG', 'LC_ALL', 'LC_CTYPE'):
            env.pop(vv, None)
        if lang is not None:
            env['LANG'] = lang
        env.update(DISPLAY=disp.name, HOME=self.home, MEUSERPATH=self.home, MEPATH=macros, MENAME='test')
        self.proc = subprocess.Popen([mec, self.tfile], env=env, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        self.win = None
        for _ in range(60):
            time.sleep(0.25)
            self.win = self.findWindow(disp.root)
            if (self.win is not None) and (Image(disp, self.win).raw is not None):
                break
            self.win = None
        self.last = None
        if self.win is not None:
            x11.XSetInputFocus(disp.dpy, self.win, 1, 0)
            x11.XSync(disp.dpy, 0)
            self.image(False)

    def findWindow(self, win):
        # ME's window has the file's name in its title, under a window manager it is reparented
        for cc in self.disp.children(win):
            pp = self.disp.prop(cc, '_NET_WM_NAME') or self.disp.prop(cc, 'WM_NAME')
            if pp and pp[1].startswith(b'MicroEmacs') and (self.name.encode('utf-8') in pp[1]):
                return cc
            ff = self.findWindow(cc)
            if ff is not None:
                return ff
        return None

    def text(self, ss):
        self.kbd.text(ss)

    def char(self, ch):
        self.kbd.char(ch)

    def command(self, line):
        self.text('\x1bxexecute-line\r')
        self.text(line + '\r')

    def capture(self):
        # the menu line is never black
        for _ in range(100):
            img = Image(self.disp, self.win)
            if (img.raw is not None) and img.row(2).strip(b'\0'):
                return img
            time.sleep(0.05)
        raise RuntimeError('window capture is blank')

    def image(self, changed=True, until=None, timeout=5.0):
        # ME's drawing is complete when three captures in a row are the same, the window has
        # changed from the last capture if it should and shows what until expects
        time.sleep(0.05)
        prev, same = self.capture(), 0
        end = time.time() + timeout
        img = prev
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


class SelectionOwner(threading.Thread):
    """Owns CLIPBOARD on its own connection and answers TARGETS, UTF8_STRING and STRING"""

    def __init__(self, dname, text):
        threading.Thread.__init__(self, daemon=True)
        self.disp = Display(dname)
        self.text = text
        self.requests = []
        self.stop = False
        dd = self.disp
        self.win = x11.XCreateSimpleWindow(dd.dpy, dd.root, 0, 0, 1, 1, 0, 0, 0)
        for nn in ('CLIPBOARD', 'TARGETS', 'UTF8_STRING', 'STRING'):
            dd.atom(nn)
        x11.XSetSelectionOwner(dd.dpy, dd.atom('CLIPBOARD'), self.win, 0)
        x11.XSync(dd.dpy, 0)
        self.owner = (x11.XGetSelectionOwner(dd.dpy, dd.atom('CLIPBOARD')) == self.win)

    def run(self):
        dd = self.disp
        ev = XEvent()
        while not self.stop:
            if not x11.XPending(dd.dpy):
                time.sleep(0.01)
                continue
            x11.XNextEvent(dd.dpy, ctypes.byref(ev))
            if ev.type != SelectionRequest:
                continue
            rq = ev.xselectionrequest
            target = dd.atomName(rq.target)
            self.requests.append(target)
            prop = rq.property or rq.target
            if target == 'TARGETS':
                arr = (c_ulong * 3)(dd.atom('TARGETS'), dd.atom('UTF8_STRING'), dd.atom('STRING'))
                x11.XChangeProperty(dd.dpy, rq.requestor, prop, dd.atom('ATOM'), 32, PropModeReplace, arr, 3)
            elif target in ('UTF8_STRING', 'STRING'):
                data = self.text.encode('utf-8' if target == 'UTF8_STRING' else 'latin-1', 'replace')
                x11.XChangeProperty(dd.dpy, rq.requestor, prop, rq.target, 8, PropModeReplace, data, len(data))
            else:
                prop = 0
            re = XEvent()
            re.xselection.type = SelectionNotify
            re.xselection.requestor = rq.requestor
            re.xselection.selection = rq.selection
            re.xselection.target = rq.target
            re.xselection.property = prop
            re.xselection.time = rq.time
            x11.XSendEvent(dd.dpy, rq.requestor, 0, 0, ctypes.byref(re))
            x11.XFlush(dd.dpy)

    def close(self):
        self.stop = True
        self.join(2)
        x11.XDestroyWindow(self.disp.dpy, self.win)
        self.disp.close()


def readSelection(disp, selection, target, timeout=5.0):
    # (type, data) of the selection converted to target, None if refused or no reply
    dd = disp
    win = x11.XCreateSimpleWindow(dd.dpy, dd.root, 0, 0, 1, 1, 0, 0, 0)
    prop = dd.atom('XWIN_SEL')
    x11.XConvertSelection(dd.dpy, dd.atom(selection), dd.atom(target), prop, win, 0)
    x11.XFlush(dd.dpy)
    ev = XEvent()
    ret = None
    end = time.time() + timeout
    while time.time() < end:
        if not x11.XPending(dd.dpy):
            time.sleep(0.01)
            continue
        x11.XNextEvent(dd.dpy, ctypes.byref(ev))
        if (ev.type == SelectionNotify) and (ev.xselection.requestor == win):
            if ev.xselection.property:
                ret = dd.prop(win, 'XWIN_SEL')
            break
    x11.XDestroyWindow(dd.dpy, win)
    x11.XSync(dd.dpy, 0)
    return ret


INPUT_TYPED = 'éß中'
INPUT_TYPED2 = '\U0001F600z'
INPUT_LINES = ['Hello wörld café', 'Αβγ да', '中文字 x |', 'a\U0001F600b |', 'end']
INPUT_NAME = 'größe中.txt'
CLIP_ME = 'é中\U0001F600x'
CLIP_X = 'Ωβ中\U0001F601z'


def testInput(disp, kbd, mec, macros, shots, lang):
    tag = 'LANG=%s' % ('unset' if lang is None else lang)
    mm = Mew(disp, kbd, mec, macros, INPUT_LINES, INPUT_NAME, lang)
    try:
        check('%s window found' % tag, mm.win is not None)
        if mm.win is None:
            return
        title = 'MicroEmacs: ' + INPUT_NAME
        nn = disp.prop(mm.win, '_NET_WM_NAME')
        check('%s _NET_WM_NAME' % tag, (nn is not None) and (nn[0] == 'UTF8_STRING') and (nn[1].decode('utf-8', 'replace') == title),
              ascii(nn))
        wn = disp.prop(mm.win, 'WM_NAME')
        # a STRING is Latin-1 (chars it lacks may be replaced), not the UTF-8 bytes
        check('%s WM_NAME not UTF-8 in a STRING' % tag, (wn is not None) and
              not ((wn[0] == 'STRING') and (title.encode('utf-8') in wn[1])), ascii(wn))
        if shots:
            mm.image(False).save(os.path.join(shots, 'xwin_start.png'))
        mm.text(INPUT_TYPED + '\r' + INPUT_TYPED2)
        if shots:
            mm.image().save(os.path.join(shots, 'xwin_typed.png'))
        # C-x C-s
        mm.text('\x18\x13')
        mm.image()
        data = open(mm.tfile, 'rb').read()
        exp = (INPUT_TYPED + '\n' + INPUT_TYPED2 + '\n'.join(INPUT_LINES) + '\n').encode('utf-8')
        check('%s saved bytes' % tag, data == exp, ascii(data[:40]))
    finally:
        mm.close()


def testClipboard(disp, kbd, mec, macros, shots):
    mm = Mew(disp, kbd, mec, macros, [CLIP_ME, 'end'], 'clip.txt', 'C.UTF-8')
    try:
        if mm.win is None:
            check('clipboard window found', False)
            return
        # copy the first line in ME, another client gets it as UTF8_STRING
        mm.command('set-mark')
        mm.command('end-of-line')
        mm.command('copy-region')
        mm.image()
        tt = readSelection(disp, 'CLIPBOARD', 'TARGETS')
        names = [disp.atomName(aa) for aa in tt[1]] if tt and isinstance(tt[1], list) else []
        check('clipboard TARGETS has UTF8_STRING', 'UTF8_STRING' in names, ascii(names))
        uu = readSelection(disp, 'CLIPBOARD', 'UTF8_STRING')
        check('clipboard UTF8_STRING from ME', (uu is not None) and (uu[1] == CLIP_ME.encode('utf-8')), ascii(uu))
        # another client owns the clipboard, yanking in ME inserts its text
        own = SelectionOwner(disp.name, CLIP_X)
        own.start()
        try:
            check('clipboard taken', own.owner)
            mm.command('end-of-buffer')
            mm.text('\x19\x18\x13')
            mm.image()
            data = open(mm.tfile, 'rb').read()
            # ME ends the last line when saving
            exp = (CLIP_ME + '\nend\n' + CLIP_X + '\n').encode('utf-8')
            check('clipboard yanked into ME', data == exp, '%s requests %s' % (ascii(data[-30:]), own.requests))
        finally:
            own.close()
    finally:
        mm.close()


RENDER_TEXTS = ['Tamil: யாமறிந்த மொழி',
                'Deva: नमस्ते कि',
                'Kannada: ಕನ್ನಡ',
                'Arabic: مرحبا بالعالم',
                'Greek: Αβγ é',
                'CJK: 中文字 abc',
                'Emoji: a\U0001F600b']
# two different chars of a script, a font lacking them draws the same box for both
RENDER_PAIRS = [('Greek', 'βγ'), ('Cyrillic', 'дж'), ('Box', '┌┐'), ('CJK', '中文'),
                ('Hiragana', 'あい'), ('Hangul', '한글'), ('Emoji', '\U0001F600\U0001F680'),
                ('Tamil', 'கந'), ('Deva', 'नम'), ('Arabic', 'مر'), ('Thai', 'กข')]


def testRendering(disp, kbd, mec, macros, shots):
    endCol = 30
    lines = [tt + ' ' * (endCol - sum(meCells(ch) for ch in tt)) + '|' for tt in RENDER_TEXTS]
    pairLines = ['P: ' + pp for _, pp in RENDER_PAIRS]
    # an ASCII first line for the geometry, drawn the same however the rest is drawn
    mm = Mew(disp, kbd, mec, macros, ['ASCII: 0123456789 abc'] + lines + pairLines + ['end'], 'render.txt', 'C.UTF-8')
    try:
        if mm.win is None:
            check('render window found', False)
            return
        # a steady cursor and a mode line without the time so captures can be compared; setting
        # $cursor-blink to 0 leaves the cursor hidden if it is off, a long blink first shows it
        mm.command('set-variable $cursor-blink 65535')
        mm.command('set-variable $cursor-blink 0')
        mm.command('set-variable $buffer-mode-line "static"')
        mm.image(False)
        # the geometry from the cursor's positions: the cursor at the start of line 1 then line 3
        # gives the row height, then at column 10 of the ASCII line 1 the cell width
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
                im.save(os.path.join(shots, 'xwin_%s.png' % nm))
        if cw <= 2:
            return
        # ignore the mode and message lines and the scroll bar
        ymax = img1.hh - 3 * ch
        xmax = x0 + (endCol + 5) * cw
        if shots:
            img1.save(os.path.join(shots, 'xwin_render.png'))

        # column layout: every line ends with its '|' in column endCol
        for ll, tt in enumerate(RENDER_TEXTS):
            yy = yTop + ll * ch
            xe = x0 + (endCol + 3) * cw
            bg = img1.pixel(xe, yy + ch // 2)
            ink = [xx for ry in range(yy, yy + ch) for xx in range(x0, xe) if img1.pixel(xx, ry) != bg]
            right = max(ink) if ink else -1
            check('column layout %s' % tt.split(':')[0], x0 + endCol * cw <= right < x0 + (endCol + 1) * cw,
                  'last ink at x %d, column %d is x %d' % (right, endCol, x0 + endCol * cw))

        # glyphs: the two chars of a pair are drawn differently by the font, or both as ME's box
        # with a diagonal when the font has no glyph for them
        def isBox(xx, yy, ww):
            bg = img1.pixel(xx, yy)
            return all(img1.pixel(px, py) != bg for px, py in
                       ((xx + 1, yy + ch // 2), (xx + ww - 2, yy + ch // 2), (xx + ww // 2, yy + 1), (xx + ww // 2, yy + ch - 2)))
        noGlyph = []
        for pp, (nm, pair) in enumerate(RENDER_PAIRS):
            yy = yTop + (len(lines) + pp) * ch
            ww = meCells(pair[0])
            xa, xb = x0 + 3 * cw, x0 + (3 + ww) * cw
            aa = img1.block((xa, xb), (yy, yy + ch))
            bb = img1.block((xb, xb + ww * cw), (yy, yy + ch))
            boxes = (aa == bb) and isBox(xa, yy, ww * cw)
            if boxes:
                noGlyph.append(nm)
            check('glyphs %s' % nm, (aa != bb) or boxes, 'both chars drawn the same but not as the unavailable char box')
        if noGlyph:
            print('INFO the font has no glyphs for %s' % ' '.join(noGlyph))

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
                mm.char('\x06')
                img = mm.image(until=lambda im: cellsDiffer(ref, im, spans[ii], (yy, yy + ch)))
                for ry, xs in ref.diff(img, ymax, yTop).items():
                    xs = [xx for xx in xs if xx < xmax]
                    out = [xx for xx in xs if not ((spans[0][0] <= xx < spans[0][1]) or (spans[ii][0] <= xx < spans[ii][1]))]
                    if xs and (not (yy <= ry < yy + ch) or out):
                        bad = 'after char %d %s: row %d x %s' % (ii, ascii(tt[ii - 1]), ry, (out or xs)[:4])
                        break
                if bad:
                    if shots:
                        ref.save(os.path.join(shots, 'xwin_fail_ref.png'))
                        img.save(os.path.join(shots, 'xwin_fail_img.png'))
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


def startXvfb():
    xvfb = shutil.which('Xvfb') or ('/opt/X11/bin/Xvfb' if os.path.exists('/opt/X11/bin/Xvfb') else None)
    if xvfb is None:
        sys.exit('Xvfb not found, give a display with --display')
    nn = 90
    while os.path.exists('/tmp/.X11-unix/X%d' % nn) or os.path.exists('/tmp/.X%d-lock' % nn):
        nn += 1
    proc = subprocess.Popen([xvfb, ':%d' % nn, '-screen', '0', '1280x1024x24', '-nolisten', 'tcp'],
                            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    for _ in range(50):
        time.sleep(0.1)
        try:
            return proc, Display(':%d' % nn)
        except RuntimeError:
            pass
    proc.kill()
    sys.exit('Xvfb did not start')


def main():
    args = sys.argv[1:]
    shots = dname = None
    for opt in ('--shots', '--display'):
        if opt in args:
            ii = args.index(opt)
            if opt == '--shots':
                shots = os.path.abspath(args[ii + 1])
            else:
                dname = args[ii + 1]
            del args[ii:ii + 2]
    if len(args) < 1:
        sys.exit('Usage: xwin.py <mew> [macros-dir] [--display <display>] [--shots <dir>]')
    mec = os.path.abspath(args[0])
    macros = os.path.abspath(args[1] if len(args) > 1 else os.path.join(TDIR, '..', '..', 'macros'))
    if shots:
        os.makedirs(shots, exist_ok=True)
    xproc = None
    if dname is None:
        xproc, disp = startXvfb()
    else:
        disp = Display(dname)
    kbd = Keyboard(disp)
    chars = ''.join(INPUT_TYPED + INPUT_TYPED2) + 'abcdefghijklmnopqrstuvwxyz'
    kbd.addChars(chars)
    try:
        for lang in (None, 'C', 'C.UTF-8'):
            testInput(disp, kbd, mec, macros, shots, lang)
        testClipboard(disp, kbd, mec, macros, shots)
        testRendering(disp, kbd, mec, macros, shots)
    finally:
        kbd.restore()
        disp.close()
        if xproc is not None:
            xproc.kill()
            xproc.wait()
    print('%d passed, %d failed' % (passed, failed))
    sys.exit(1 if failed else 0)


if __name__ == '__main__':
    main()
