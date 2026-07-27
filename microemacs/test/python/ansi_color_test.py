#!/usr/bin/env python3
"""
ANSI SGR test generator for MicroEmacs ipipe console-color support.

Targets ipipeAnsiToScheme() in src/spawn.c, which collapses raw SGR fg/bg/style
into a single semantic highlight char (see \\CC? table in macros/meth.emf):

  precedence: bg color > fg color > dim > style-only
    bg red/green/yellow        -> O, N, M (gdfrej, gdfsel, gdfchange), or
                                  u, v, w (hlred, hlgreen, hlyellow) when the
                                  bright form (\\E[10xm) is used, which is taken
                                  as a request to simply highlight the text
    bg blue/magenta/cyan/white -> x, y, z, s (hlblue, hlmagenta, hlcyan, hlwhite)
    bg black                   -> A, assumed to be the terminal's own background
    fg red (bold/not), green, yellow, blue, magenta, cyan
                               -> k/R, Q, l, S, S, m (error/rmv, add, warn, dir, dir, info)
    fg black or bright black (\\E[90m), and dim (\\E[2m)
                               -> h (comment), i.e. de-emphasized text
    fg white                   -> falls through to the style
    style bits (bold=1,italic=2,under=4), only when no color applies -> A + 1..7 (D..J in meth.emf)

  38;5;n (256-color) and 38;2;r;g;b (truecolor) are reduced to one of the 8 base
  colors plus a shade and then mapped as above, so e.g. any rgb red background
  is a diff rejection and any grey darker than 0xa0 is de-emphasized text.
  ipipe_scheme_test.py covers that reduction in detail and derives its expected
  schemes from a mirror of the C; this script stays hand-labelled and sticks to
  the 16 color terminal SGRs.

Usage:
  python ansi_color_test.py                # everything, normal speed
  python ansi_color_test.py --mode grid     # just the 16-color fg/bg grid
  python ansi_color_test.py --slow          # byte-at-a-time, stresses split-CSI reads
  python ansi_color_test.py --plain > out   # no section banners, just raw escapes+labels

Run this piped into / opened via the ipipe (e.g. as the command an ipipe buffer
runs), not just eyeballed in a normal terminal -- the point is to check what
MicroEmacs's ipipe decodes it as, not how your shell renders it.
"""
import argparse
import sys
import time

ESC = "\x1b["
RESET = ESC + "0m"

# color index -> name, matches ipipeAnsiToScheme's (fg|bg & 0x07) switch
COLORS = [
    (0, "black"), (1, "red"), (2, "green"), (3, "yellow"),
    (4, "blue"), (5, "magenta"), (6, "cyan"), (7, "white"),
]

STYLE_BITS = [(0x01, "bold"), (0x02, "italic"), (0x04, "under")]


def sgr(*codes):
    return ESC + ";".join(str(c) for c in codes) + "m"


def out(w, s, end="\n"):
    w.write(s)
    if end:
        w.write(end)


def emit(w, text, slow, delay):
    """Write text either whole, or one char at a time (to fragment CSI sequences
    across separate pty reads, like a slow/chunked real process would)."""
    if not slow:
        w.write(text)
        return
    for ch in text:
        w.write(ch)
        w.flush()
        time.sleep(delay)


def section(w, title, plain):
    if not plain:
        out(w, "")
        out(w, f"== {title} ==")


def line(w, label, seq_text, plain, slow, delay):
    """label: plain description; seq_text: the raw ANSI-wrapped sample."""
    if plain:
        emit(w, seq_text + RESET, slow, delay)
        out(w, "")
    else:
        pad = " " * max(1, 42 - len(label))
        out(w, label + pad, end="")
        emit(w, seq_text + RESET, slow, delay)
        out(w, "")


def mode_grid(w, plain, slow, delay):
    section(w, "16 fg colors (30-37, 90-97)", plain)
    for idx, name in COLORS:
        line(w, f"fg {idx} {name} (normal)", sgr(30 + idx) + f"SAMPLE fg={name}", plain, slow, delay)
    for idx, name in COLORS:
        line(w, f"fg {idx} {name} (bright/90s)", sgr(90 + idx) + f"SAMPLE fg={name} bright", plain, slow, delay)

    section(w, "16 bg colors (40-47, 100-107)", plain)
    for idx, name in COLORS:
        line(w, f"bg {idx} {name} (normal)", sgr(40 + idx) + f"SAMPLE bg={name}", plain, slow, delay)
    for idx, name in COLORS:
        line(w, f"bg {idx} {name} (bright/100s)", sgr(100 + idx) + f"SAMPLE bg={name} bright", plain, slow, delay)


def mode_style(w, plain, slow, delay):
    section(w, "style bit combinations (bold/italic/under)", plain)
    for bits in range(8):
        codes = []
        if bits & 0x01:
            codes.append(1)
        if bits & 0x02:
            codes.append(3)
        if bits & 0x04:
            codes.append(4)
        names = "+".join(n for m, n in STYLE_BITS if bits & m) or "none"
        seq = ESC + ";".join(str(c) for c in codes) + "m" if codes else ""
        line(w, f"style {bits:03b} ({names})", seq + f"SAMPLE style={names}", plain, slow, delay)

    section(w, "dim (2) is an intensity, not a style bit", plain)
    line(w, "dim then un-bold(22) clears the dim too",
         sgr(2) + "BEFORE" + sgr(22) + "AFTER", plain, slow, delay)
    line(w, "dim + bold -> still h (comment)", sgr(2, 1) + "SAMPLE", plain, slow, delay)

    section(w, "style resets (22=un-bold/un-dim, 23=un-italic, 24=un-under)", plain)
    line(w, "bold+italic+under then un-bold(22)",
         sgr(1, 3, 4) + "BEFORE" + sgr(22) + "AFTER", plain, slow, delay)
    line(w, "bold+italic+under then un-italic(23)",
         sgr(1, 3, 4) + "BEFORE" + sgr(23) + "AFTER", plain, slow, delay)
    line(w, "bold+italic+under then un-under(24)",
         sgr(1, 3, 4) + "BEFORE" + sgr(24) + "AFTER", plain, slow, delay)


def mode_semantic(w, plain, slow, delay):
    """Exercises exactly the branches ipipeAnsiToScheme takes, both the mapped
    ones and the ones that fall through to the style or default."""
    section(w, "fg scheme mappings", plain)
    cases = [
        ("fg red, not bold -> R (rmv)", sgr(31)),
        ("fg red, bold -> k (error)", sgr(1, 31)),
        ("fg green -> Q (add)", sgr(32)),
        ("fg yellow -> l (warn)", sgr(33)),
        ("fg blue -> S (dir)", sgr(34)),
        ("fg magenta -> S (dir)", sgr(35)),
        ("fg cyan -> m (info)", sgr(36)),
        ("fg black -> h (comment)", sgr(30)),
        ("fg bright black/grey -> h (comment)", sgr(90)),
        ("dim, no color -> h (comment)", sgr(2)),
    ]
    for label, seq in cases:
        line(w, label, seq + "SAMPLE", plain, slow, delay)

    section(w, "fg colors with no meaning of their own", plain)
    for code, name in ((37, "white"), (97, "bright white")):
        line(w, f"fg {name} -> falls through to 'A'/style", sgr(code) + "SAMPLE", plain, slow, delay)

    section(w, "bg scheme mappings", plain)
    cases = [
        ("bg red normal -> O (gdfrej)", sgr(41)),
        ("bg red bright -> u (hlred)", sgr(101)),
        ("bg green normal -> N (gdfsel)", sgr(42)),
        ("bg green bright -> v (hlgreen)", sgr(102)),
        ("bg yellow normal -> M (gdfchange)", sgr(43)),
        ("bg yellow bright -> w (hlyellow)", sgr(103)),
        ("bg blue -> x (hlblue)", sgr(44)),
        ("bg magenta -> y (hlmagenta)", sgr(45)),
        ("bg cyan -> z (hlcyan)", sgr(46)),
        ("bg white -> s (hlwhite)", sgr(47)),
    ]
    for label, seq in cases:
        line(w, label, seq + "SAMPLE", plain, slow, delay)

    section(w, "bg black, taken as the terminal's own background", plain)
    for code, name in ((40, "black"), (100, "bright black")):
        line(w, f"bg {name} -> falls through to 'A'", sgr(code) + "SAMPLE", plain, slow, delay)

    section(w, "precedence: bg beats fg", plain)
    line(w, "fg green + bg red normal -> should be O, not Q", sgr(32, 41) + "SAMPLE", plain, slow, delay)
    line(w, "fg cyan + bg yellow bright -> should be w, not m", sgr(36, 103) + "SAMPLE", plain, slow, delay)

    section(w, "precedence: fg beats style when fg is mapped", plain)
    line(w, "bold + fg green -> should still be Q, not just bold", sgr(1, 32) + "SAMPLE", plain, slow, delay)


def mode_extended(w, plain, slow, delay):
    """The extended colors are reduced to a base color + shade, see
    ipipe_scheme_test.py for full coverage of that reduction."""
    section(w, "256 palette (38;5;n / 48;5;n)", plain)
    line(w, "fg 38;5;196 (red) -> R (rmv)", sgr(38, 5, 196) + "SAMPLE", plain, slow, delay)
    line(w, "fg 38;5;46 (green) -> Q (add)", sgr(38, 5, 46) + "SAMPLE", plain, slow, delay)
    line(w, "fg 38;5;244 (mid grey) -> h (comment)", sgr(38, 5, 244) + "SAMPLE", plain, slow, delay)
    line(w, "bg 48;5;22 (dark green) -> N (gdfsel)", sgr(48, 5, 22) + "SAMPLE", plain, slow, delay)
    line(w, "bg 48;5;52 (dark red) -> O (gdfrej)", sgr(48, 5, 52) + "SAMPLE", plain, slow, delay)

    section(w, "truecolor (38;2;r;g;b / 48;2;r;g;b)", plain)
    line(w, "fg 38;2;255;0;0 -> R (rmv)", sgr(38, 2, 255, 0, 0) + "SAMPLE", plain, slow, delay)
    line(w, "fg 38;2;136;136;136 (grey) -> h (comment)", sgr(38, 2, 136, 136, 136) + "SAMPLE", plain, slow, delay)
    line(w, "bg 48;2;204;238;204 (pale green) -> N (gdfsel)", sgr(48, 2, 204, 238, 204) + "SAMPLE", plain, slow, delay)
    line(w, "bg 48;2;255;204;204 (pale red) -> O (gdfrej)", sgr(48, 2, 255, 204, 204) + "SAMPLE", plain, slow, delay)

    section(w, "REGRESSION: attrs after an extended color must survive", plain)
    line(w, "38;5;196 then ;1 (bold) in same SGR -> k (error), not R",
         sgr(38, 5, 196, 1) + "SAMPLE bold must apply", plain, slow, delay)
    line(w, "control: same bold alone, no preceding extended color",
         sgr(1) + "SAMPLE bold should apply", plain, slow, delay)
    line(w, "bold BEFORE the extended color instead (order must not matter)",
         sgr(1, 38, 2, 255, 0, 0) + "SAMPLE also k (error)", plain, slow, delay)
    line(w, "rgb fg and bg in one SGR -> N (gdfsel), bg wins",
         sgr(0, 38, 2, 0, 0, 0, 48, 2, 105, 219, 124) + "SAMPLE", plain, slow, delay)

    section(w, "malformed extended color, rest of the sequence is abandoned", plain)
    line(w, "38;2 with too few channels -> A (default)",
         sgr(38, 2, 255, 0) + "SAMPLE", plain, slow, delay)


def mode_reset(w, plain, slow, delay):
    section(w, "reset behavior", plain)
    line(w, "0 = full reset", sgr(31, 1) + "COLORED" + sgr(0) + "PLAIN", plain, slow, delay)
    line(w, "39 = fg reset only (bg should persist)", sgr(31, 42) + "BOTH" + sgr(39) + "BG_ONLY", plain, slow, delay)
    line(w, "49 = bg reset only (fg should persist)", sgr(31, 42) + "BOTH" + sgr(49) + "FG_ONLY", plain, slow, delay)
    line(w, "bare ESC[m = same as 0", ESC + "m" + "SHOULD_BE_PLAIN", plain, slow, delay)


MODES = {
    "grid": mode_grid,
    "style": mode_style,
    "semantic": mode_semantic,
    "extended": mode_extended,
    "reset": mode_reset,
}


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--mode", choices=list(MODES) + ["all"], default="all")
    ap.add_argument("--slow", action="store_true", help="emit one char at a time, to fragment escape sequences across pty reads")
    ap.add_argument("--delay", type=float, default=0.01, help="seconds between chars when --slow (default 0.01)")
    ap.add_argument("--plain", action="store_true", help="drop section banners/labels, just the raw sequences (one per line)")
    args = ap.parse_args()

    w = sys.stdout
    modes = list(MODES) if args.mode == "all" else [args.mode]
    for m in modes:
        MODES[m](w, args.plain, args.slow, args.delay)
    if not args.slow:
        w.flush()


if __name__ == "__main__":
    main()
