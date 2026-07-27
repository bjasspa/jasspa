#!/usr/bin/env python3
"""
ANSI SGR -> semantic scheme test generator for the MicroEmacs ipipe.

Targets ipipeAnsiToScheme() and friends in src/spawn.c, which reduce the SGR
fg/bg/style state to a single meth scheme tag char (the \\CC? table at the top
of macros/meth.emf).  24 bit rgb (38;2;r;g;b) and 256 palette (38;5;n) colours
are reduced to one of the 8 base colours plus a shade, and it is that reduction
which picks the scheme, e.g. any rgb red background is a diff rejection.

The mapping is mirrored in Python below, so every sample states the scheme it
is expected to be displayed in -- if the text ".scheme.add" is not drawn using
.scheme.add then that case has failed.

Usage:
  python ipipe_scheme_test.py                 # every mode
  python ipipe_scheme_test.py --mode rgb      # just the 24 bit rgb colours
  python ipipe_scheme_test.py --list          # list the modes and exit
  python ipipe_scheme_test.py --coverage      # report scheme coverage on stderr
  python ipipe_scheme_test.py --slow          # byte at a time, splits the CSIs
  python ipipe_scheme_test.py --plain         # no labels, just the samples

Run it as the command of an ipipe buffer, e.g.

    ipipe-shell-command "python3 test/python/ipipe_scheme_test.py"

not in a plain terminal -- the point is what MicroEmacs makes of it, not what
the terminal makes of it.
"""
import argparse
import sys
import time

ESC = "\x1b["
RESET = ESC + "0m"

# meIPIPE_COL_* from src/estruct.h
COL_MASK = 0x07
COL_SET = 0x08
COL_BRIGHT = 0x10
COL_DARK = 0x20
COL_GREY = 0x40
COL_RGB = 0x80

# meIPIPE_STY_* from src/estruct.h
STY_BOLD = 0x01
STY_ITALIC = 0x02
STY_UNDER = 0x04
STY_DIM = 0x08

# scheme tag char -> scheme name, from the \CC? hilight table in macros/meth.emf
SCHEME = {
    "A": "default", "D": "bold", "E": "italic", "F": "bold-italic",
    "G": "under", "H": "bold-under", "I": "italic-under",
    "J": "bold-italic-under", "M": "gdfchange", "N": "gdfsel", "O": "gdfrej",
    "Q": "add", "R": "rmv", "S": "dir", "h": "comment", "k": "error",
    "l": "warn", "m": "info", "s": "hlwhite", "u": "hlred", "v": "hlgreen",
    "w": "hlyellow", "x": "hlblue", "y": "hlmagenta", "z": "hlcyan",
}

# every scheme ipipeAnsiToScheme() can return, used by --coverage
REACHABLE = set(SCHEME)

CUBE_LVL = (0x00, 0x5f, 0x87, 0xaf, 0xd7, 0xff)

# meIPIPE_PRM_MAX from src/spawn.c, the CSI parameter limit
PRM_MAX = 16


#
# Mirror of the src/spawn.c colour reduction, keep the two in step.
#
def base_to_color(idx):
    """ipipeBaseToColor() - one of the 16 standard terminal colours."""
    cc = (idx & COL_MASK) | COL_SET
    if idx & 0x08:
        cc |= COL_BRIGHT
    if (idx & COL_MASK) in (0, 7):
        cc |= COL_GREY
    return cc


def rgb_to_color(r, g, b):
    """ipipeRgbToColor() - reduce a 24 bit rgb colour to a base colour + shade."""
    mx, mn = max(r, g, b), min(r, g, b)
    if (mx < 0x20) or (((mx - mn) << 4) < mx):
        if mx < 0x40:
            cc = 0 | COL_DARK
        elif mx < 0xa0:
            cc = 0 | COL_BRIGHT
        elif mx < 0xd8:
            cc = 7 | COL_DARK
        else:
            cc = 7 | COL_BRIGHT
        return cc | COL_SET | COL_GREY | COL_RGB
    mid = (mx + mn) >> 1
    cc = (0x01 if r > mid else 0) | (0x02 if g > mid else 0) | (0x04 if b > mid else 0)
    if mx >= 0xc0:
        cc |= COL_BRIGHT
    elif mx < 0x80:
        cc |= COL_DARK
    return cc | COL_SET | COL_RGB


def pal_to_color(idx):
    """ipipePalToColor() - a 256 colour palette index."""
    if idx < 0:
        return 0
    if idx < 16:
        return base_to_color(idx)
    if idx < 232:
        idx -= 16
        return rgb_to_color(CUBE_LVL[(idx // 36) % 6], CUBE_LVL[(idx // 6) % 6], CUBE_LVL[idx % 6])
    if idx < 256:
        idx = 8 + ((idx - 232) * 10)
        return rgb_to_color(idx, idx, idx)
    return 0


def parse_params(spec):
    """The CSI parameter collection of ipipeRead(), spec being the parameter
    text of the sequence, e.g. "1;38:2::255:0:0".  Returns the parameter values
    and, for each, whether it was introduced by the ':' sub-parameter
    separator.  Both are truncated to meIPIPE_PRM_MAX entries."""
    vals, subs, sub, val = [], [], 0, 0
    for ch in spec:
        if ch.isdigit():
            val = (val * 10) + int(ch)
        elif ch in ";:":
            vals.append(val)
            subs.append(sub)
            sub, val = 1 if ch == ":" else 0, 0
    vals.append(val)
    subs.append(sub)
    return vals[:PRM_MAX], subs[:PRM_MAX]


def ext_color(prm, sub):
    """ipipeAnsiExtColor() - the parameters following a 38 or 48, returns
    (colour, number of parameters consumed), consumed 0 if not understood."""
    cnt, itu = len(prm), False
    if cnt > 1:
        if sub[0]:
            # the colon separator gives the exact argument count, the ITU-T
            # form has an extra colour space id before the rgb values
            nn = 1
            while (nn < cnt) and sub[nn]:
                nn += 1
            cnt = nn
            itu = cnt > 4
        if prm[0] == 5:
            return pal_to_color(prm[1]), 2
        if prm[0] == 2:
            nn = 2 if itu else 1
            if cnt > (nn + 2):
                return rgb_to_color(prm[nn], prm[nn + 1], prm[nn + 2]), nn + 3
    return 0, 0


def apply_sgr(codes, subs, fg=0, bg=0, st=0):
    """The SGR parameter loop of ipipeRead()'s 'm' case."""
    ii = 0
    while ii < len(codes):
        prm = codes[ii]
        if prm == 0:
            fg = bg = st = 0
        elif prm < 30:
            if prm < 5:
                if prm > 2:
                    st |= 1 << (prm - 2)
                elif prm == 2:
                    st |= STY_DIM
                else:
                    st |= STY_BOLD
            elif prm == 22:
                st &= ~(STY_BOLD | STY_DIM)
            elif 22 < prm < 25:
                st &= ~(1 << (prm - 22))
        elif prm < 40:
            if prm < 38:
                fg = base_to_color(prm - 30)
            elif prm == 38:
                fg, nn = ext_color(codes[ii + 1:], subs[ii + 1:])
                if nn == 0:
                    break
                ii += nn
            else:
                fg = 0
        elif prm < 50:
            if prm < 48:
                bg = base_to_color(prm - 40)
            elif prm == 48:
                bg, nn = ext_color(codes[ii + 1:], subs[ii + 1:])
                if nn == 0:
                    break
                ii += nn
            else:
                bg = 0
        elif 90 <= prm < 98:
            fg = base_to_color(prm - 90 + 8)
        elif 100 <= prm < 108:
            bg = base_to_color(prm - 100 + 8)
        ii += 1
    return fg, bg, st


def ansi_to_scheme(fg, bg, st):
    """ipipeAnsiToScheme() - the colour & style state as a meth scheme tag."""
    if bg & COL_SET:
        hl = (bg & (COL_BRIGHT | COL_RGB)) == COL_BRIGHT
        if bg & COL_GREY:
            return "s" if (bg & COL_MASK) else "A"
        base = bg & COL_MASK
        if base == 1:
            return "u" if hl else "O"
        if base == 2:
            return "v" if hl else "N"
        if base == 3:
            return "w" if hl else "M"
        return {4: "x", 5: "y", 6: "z", 7: "s"}.get(base, "A")
    if fg & COL_SET:
        if fg & COL_GREY:
            if not (fg & COL_MASK):
                return "h"
        else:
            base = fg & COL_MASK
            if base == 1:
                return "k" if (st & STY_BOLD) else "R"
            hue = {2: "Q", 3: "l", 4: "S", 5: "S", 6: "m"}.get(base)
            if hue is not None:
                return hue
    if st & STY_DIM:
        return "h"
    return chr(ord("C") + (st & 0x07)) if (st & 0x07) else "A"


def expect(spec):
    """The scheme tag char a fresh SGR of this parameter text should produce."""
    return ansi_to_scheme(*apply_sgr(*parse_params(spec)))


#
# Output
#
class Out:
    """Emits the samples and tracks which schemes have been exercised."""

    def __init__(self, w, opts):
        self.w = w
        self.plain = opts.plain
        self.slow = opts.slow
        self.delay = opts.delay
        self.seen = {}

    def raw(self, text, end="\n"):
        if self.slow:
            for ch in text:
                self.w.write(ch)
                self.w.flush()
                time.sleep(self.delay)
        else:
            self.w.write(text)
        if end:
            self.w.write(end)

    def section(self, title):
        if not self.plain:
            self.raw("")
            self.raw("== " + title + " ==")

    def sample(self, label, seq, ch, text=None):
        """label describes the SGR, seq is the escape sequence, ch the expected
        scheme tag - the sample text names the scheme so a mis-mapping shows up
        as text drawn in the wrong scheme."""
        self.seen.setdefault(ch, label)
        if text is None:
            text = "sample -> ." + ("scheme." + SCHEME[ch] if ch in SCHEME else "unknown(%s)" % ch)
        if self.plain:
            self.raw(seq + text + RESET)
        else:
            self.raw("%-44s" % label, end="")
            self.raw(seq + text + RESET)

    def spec(self, label, spec, text=None):
        """Emit a sample for the SGR with this parameter text."""
        self.sample(label, ESC + spec + "m", expect(spec), text)

    def sgr(self, label, *codes, **kw):
        """Emit a sample for an SGR built from these ';' separated parameters."""
        self.spec(label, ";".join(str(c) for c in codes), kw.get("text"))


#
# Modes
#
def mode_base(o):
    """the 16 standard fg & bg colours (30-37, 90-97, 40-47, 100-107)"""
    names = ("black", "red", "green", "yellow", "blue", "magenta", "cyan", "white")
    o.section("standard fg colours")
    for i, nm in enumerate(names):
        o.sgr("fg %d %s" % (30 + i, nm), 30 + i)
    for i, nm in enumerate(names):
        o.sgr("fg %d bright %s" % (90 + i, nm), 90 + i)
    o.section("standard fg colours, bold")
    for i, nm in enumerate(names):
        o.sgr("fg %d %s + bold" % (30 + i, nm), 1, 30 + i)
    o.section("standard bg colours")
    for i, nm in enumerate(names):
        o.sgr("bg %d %s" % (40 + i, nm), 40 + i)
    for i, nm in enumerate(names):
        o.sgr("bg %d bright %s" % (100 + i, nm), 100 + i)


def mode_rgb(o):
    """24 bit rgb fg & bg colours (38;2;r;g;b / 48;2;r;g;b)"""
    hues = [
        ("pure red", (255, 0, 0)), ("dark red", (128, 32, 32)),
        ("pure green", (0, 255, 0)), ("dark green", (32, 96, 48)),
        ("pure yellow", (255, 255, 0)), ("orange", (255, 135, 0)),
        ("amber", (215, 175, 0)), ("pure blue", (0, 0, 255)),
        ("steel blue", (70, 130, 180)), ("pure magenta", (255, 0, 255)),
        ("purple", (150, 90, 200)), ("pure cyan", (0, 255, 255)),
        ("teal", (0, 128, 128)),
    ]
    o.section("24 bit rgb fg")
    for nm, (r, g, b) in hues:
        o.sgr("fg rgb %-12s %3d,%3d,%3d" % (nm, r, g, b), 38, 2, r, g, b)
    o.section("24 bit rgb bg")
    for nm, (r, g, b) in hues:
        o.sgr("bg rgb %-12s %3d,%3d,%3d" % (nm, r, g, b), 48, 2, r, g, b)
    o.section("rgb hue boundaries (channel split about the mid point)")
    for r, g, b in ((255, 128, 0), (255, 127, 0), (200, 200, 60), (60, 200, 200),
                    (200, 60, 200), (100, 100, 60), (255, 220, 220)):
        o.sgr("fg rgb %3d,%3d,%3d" % (r, g, b), 38, 2, r, g, b)
    o.section("rgb fg and bg in a single SGR")
    o.sgr("black fg on pastel green bg", 0, 38, 2, 0, 0, 0, 48, 2, 105, 219, 124)
    o.sgr("white fg on dark red bg", 0, 38, 2, 255, 255, 255, 48, 2, 90, 30, 30)
    o.section("rgb with a trailing attribute (must not be swallowed)")
    o.sgr("rgb red fg then bold", 38, 2, 255, 0, 0, 1)
    o.sgr("bold then rgb red fg", 1, 38, 2, 255, 0, 0)
    o.sgr("rgb grey fg then underline", 38, 2, 136, 136, 136, 4)


def mode_pal(o):
    """256 colour palette (38;5;n / 48;5;n)"""
    o.section("palette 0-15, the standard colours")
    for n in (0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15):
        o.sgr("fg 38;5;%d" % n, 38, 5, n)
    o.section("palette 16-231, the 6x6x6 colour cube")
    for n in (16, 21, 46, 51, 196, 201, 226, 231, 28, 88, 130, 166, 202, 208, 214, 63, 99):
        o.sgr("fg 38;5;%d" % n, 38, 5, n)
    o.section("palette 232-255, the grey ramp")
    for n in (232, 236, 240, 244, 248, 252, 255):
        o.sgr("fg 38;5;%d" % n, 38, 5, n)
    o.section("palette backgrounds")
    for n in (22, 52, 58, 236, 252, 196, 46):
        o.sgr("bg 48;5;%d" % n, 48, 5, n)
    o.section("out of range palette index")
    o.sgr("fg 38;5;300 (ignored)", 38, 5, 300)


def mode_grey(o):
    """greys and dim text, i.e. de-emphasized placeholder text"""
    o.section("dim (\\E[2m) and its reset")
    o.sgr("dim", 2)
    o.sgr("dim + bold", 2, 1)
    o.sgr("dim then 22 (both intensities off)", 2, 22)
    o.sgr("dim + underline", 2, 4)
    o.sgr("dim + rgb red fg (colour wins)", 2, 38, 2, 255, 0, 0)
    o.section("grey foregrounds, darker than mid grey is de-emphasized")
    for lvl in (0x20, 0x40, 0x60, 0x80, 0x9f, 0xa0, 0xc0, 0xe0, 0xff):
        o.sgr("fg rgb grey 0x%02x" % lvl, 38, 2, lvl, lvl, lvl)
    o.section("near greys, the hue counts once above about 6% saturation")
    for r, g, b in ((136, 136, 140), (136, 136, 150), (100, 104, 100),
                    (100, 120, 110), (200, 196, 200), (200, 190, 210)):
        o.sgr("fg rgb %3d,%3d,%3d" % (r, g, b), 38, 2, r, g, b)
    o.section("very dark colours, too dark for the hue to count")
    for r, g, b in ((10, 20, 15), (16, 16, 16), (0, 31, 0), (0, 33, 0)):
        o.sgr("fg rgb %3d,%3d,%3d" % (r, g, b), 38, 2, r, g, b)
    o.section("grey via the standard colours")
    o.sgr("fg 90 bright black, i.e. grey", 90)
    o.sgr("fg 37 white", 37)
    o.sgr("fg 97 bright white", 97)
    o.section("grey backgrounds")
    for lvl in (0x1e, 0x30, 0x50, 0xb0, 0xf0):
        o.sgr("bg rgb grey 0x%02x" % lvl, 48, 2, lvl, lvl, lvl)


def mode_diff(o):
    """diff style backgrounds, the main use of rgb backgrounds"""
    o.section("diff blocks, light theme")
    o.sgr("added line", 48, 2, 204, 238, 204, text="+   printf(\"hello\\n\");")
    o.sgr("removed line", 48, 2, 255, 204, 204, text="-   puts(\"hello\");")
    o.sgr("changed line", 48, 2, 250, 240, 180, text="~   fputs(\"hello\",stdout);")
    o.section("diff blocks, dark theme")
    o.sgr("added line", 48, 2, 31, 58, 40, text="+   printf(\"hello\\n\");")
    o.sgr("removed line", 48, 2, 74, 31, 31, text="-   puts(\"hello\");")
    o.section("diff blocks, standard colours")
    o.sgr("bg 42 green", 42, text="+   added")
    o.sgr("bg 41 red", 41, text="-   removed")
    o.sgr("bg 43 yellow", 43, text="~   changed")
    o.section("bright backgrounds are a highlight, not a diff")
    o.sgr("bg 102 bright green", 102)
    o.sgr("bg 101 bright red", 101)
    o.sgr("bg 103 bright yellow", 103)
    o.section("remaining highlight backgrounds")
    o.sgr("bg 44 blue", 44)
    o.sgr("bg 45 magenta", 45)
    o.sgr("bg 46 cyan", 46)
    o.sgr("bg 47 white", 47)
    o.sgr("bg 40 black, the terminal's own", 40)


def mode_style(o):
    """the bold / italic / underline combinations"""
    o.section("style bits, no colour set")
    for bits in range(8):
        codes = [c for c, m in ((1, 0x01), (3, 0x02), (4, 0x04)) if bits & m]
        if not codes:
            o.sample("style none", "", "A")
        else:
            o.sgr("style %s" % "+".join(str(c) for c in codes), *codes)
    o.section("style resets")
    for reset, what in ((22, "un-bold"), (23, "un-italic"), (24, "un-under")):
        o.sample("bold+italic+under then %d %s" % (reset, what), ESC + "1;3;4m",
                 expect("1;3;4;%d" % reset), "BEFORE" + ESC + "%dm" % reset + "AFTER")
    o.section("colour beats style")
    o.sgr("bold + green fg", 1, 32)
    o.sgr("bold + red fg (bold red is an error)", 1, 31)
    o.sgr("underline + rgb bg green (bg beats all)", 4, 48, 2, 60, 160, 90)


def mode_colon(o):
    """the ':' separated sub-parameter forms"""
    o.section("colon separated, no colour space id")
    for spec in ("38:2:255:0:0", "38:5:196", "48:2:60:160:90"):
        o.spec(spec, spec)
    o.section("colon separated, ITU-T form with the (usually empty) colour space id")
    for spec in ("38:2::255:0:0", "38:2:0:255:0:0", "48:2::60:160:90"):
        o.spec(spec, spec)
    o.section("colon separated colour mixed with ';' separated parameters")
    for spec in ("1;38:2::255:0:0", "38:2::255:0:0;1", "38:5:196;4",
                 "38:2::136:136:136;48:2::60:160:90"):
        o.spec(spec, spec)


def mode_reset(o):
    """resets and truncated or malformed sequences"""
    o.section("resets")
    o.sample("0 full reset", ESC + "1;38;2;255;0;0m", "A", "COLOURED" + RESET + "plain")
    both = "38;2;255;0;0;48;2;60;160;90"
    o.sample("39 fg reset, bg persists", ESC + both + "m",
             expect(both + ";39"), "BOTH" + ESC + "39m" + "bg only")
    o.sample("49 bg reset, fg persists", ESC + both + "m",
             expect(both + ";49"), "BOTH" + ESC + "49m" + "fg only")
    o.sample("bare ESC[m is a full reset", ESC + "31m", "A", "RED" + ESC + "m" + "plain")
    o.section("malformed extended colours, the sequence is abandoned")
    o.sgr("38 with nothing following", 38)
    o.sgr("38;2 with too few channels", 38, 2, 255, 0)
    o.sgr("38;9 unknown colour type", 38, 9, 1)
    o.sgr("38;5 with no index", 38, 5)
    o.section("more parameters than the %d parameter limit, the excess is dropped" % PRM_MAX)
    o.spec("rgb red fg pushed past the limit", ";".join(["0"] * PRM_MAX) + ";38;2;255;0;0")


def mode_claude(o):
    """a sample of the sort of output the claude cli produces"""
    grey = ESC + "38;2;136;136;136m"
    orange = ESC + "38;2;215;119;0m"
    green = ESC + "38;2;60;160;90m"
    addbg = ESC + "48;2;204;238;204m"
    rmvbg = ESC + "48;2;255;204;204m"
    o.section("claude cli style output")
    for ch, lbl in (("h", "grey placeholder"), ("l", "orange status"), ("Q", "green tick"),
                    ("N", "diff added"), ("O", "diff removed")):
        o.seen.setdefault(ch, "claude " + lbl)
    o.raw(grey + "  Try \"fix the build error\"" + RESET)
    o.raw(orange + "* Compiling..." + RESET + grey + " (esc to interrupt)" + RESET)
    o.raw(green + "  + Read " + RESET + "spawn.c" + grey + " (1908 lines)" + RESET)
    o.raw("  Updated " + ESC + "1m" + "spawn.c" + RESET + grey + " with 2 additions" + RESET)
    o.raw(addbg + "   946 + static meUByte ipipeRgbToColor(int r,int g,int b)" + RESET)
    o.raw(rmvbg + "   945 - static meUByte ipipeAnsiToScheme(void)" + RESET)
    o.raw(grey + "  ? for shortcuts" + RESET)


MODES = {
    "base": mode_base,
    "rgb": mode_rgb,
    "pal": mode_pal,
    "grey": mode_grey,
    "diff": mode_diff,
    "style": mode_style,
    "colon": mode_colon,
    "reset": mode_reset,
    "claude": mode_claude,
}


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--mode", action="append", choices=list(MODES) + ["all"],
                    help="modes to run, may be repeated (default all)")
    ap.add_argument("--list", action="store_true", help="list the modes and exit")
    ap.add_argument("--coverage", action="store_true",
                    help="report which schemes were exercised on stderr")
    ap.add_argument("--slow", action="store_true",
                    help="emit one char at a time, fragmenting the escape sequences across reads")
    ap.add_argument("--delay", type=float, default=0.01,
                    help="seconds between chars when --slow (default 0.01)")
    ap.add_argument("--plain", action="store_true",
                    help="drop the section banners and labels, just the samples")
    args = ap.parse_args()

    if args.list:
        for nm in MODES:
            print("%-8s %s" % (nm, MODES[nm].__doc__))
        return 0

    modes = list(MODES) if (not args.mode or "all" in args.mode) else args.mode
    o = Out(sys.stdout, args)
    for nm in modes:
        MODES[nm](o)
    sys.stdout.flush()

    if args.coverage:
        missing = REACHABLE - set(o.seen)
        sys.stderr.write("\nschemes exercised: %d of %d\n" % (len(REACHABLE) - len(missing), len(REACHABLE)))
        for ch in sorted(SCHEME, key=lambda c: SCHEME[c]):
            if ch in o.seen:
                sys.stderr.write("  %s .scheme.%-18s %s\n" % (ch, SCHEME[ch], o.seen[ch]))
        for ch in sorted(missing, key=lambda c: SCHEME[c]):
            sys.stderr.write("  %s .scheme.%-18s NOT EXERCISED\n" % (ch, SCHEME[ch]))
        return 1 if missing else 0
    return 0


if __name__ == "__main__":
    sys.exit(main())
