#!/usr/bin/env python3
"""
ANSI SGR test generator for MicroEmacs ipipe console-color support.

Targets ipipeAnsiToScheme() in src/spawn.c, which collapses raw SGR fg/bg/style
into a single semantic highlight char (see \\CC? table in macros/meth.emf):

  precedence: bg color > fg color > style-only
    bg red/green/yellow (bright or normal) -> u/O, v/N, w/M  (hlred/gdfrej, hlgreen/gdfsel, hlyellow/gdfchange)
    bg black/blue/magenta/cyan/white       -> falls through, never shown (dead)
    fg red (bold/not), green, yellow, blue, cyan -> k/R, Q, l, S, m (error/rmv, add, warn, dir, info)
    fg black/magenta/white                 -> falls through, never shown (dead)
    style bits (bold=1,italic=2,under=4), only when fg==bg==0 -> A + 1..7 (D..J in meth.emf)

  38;5;n (256-color) and 38;2;r;g;b (truecolor) are NOT parsed: the loop hits
  prmA==38/48 and does `break`, abandoning the *rest* of the SGR param list too
  -- so e.g. "\x1b[38;5;196;1m" silently drops the trailing bold(1) as well.

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

    section(w, "style resets (22=un-bold, 23=un-italic, 24=un-under)", plain)
    line(w, "bold+italic+under then un-bold(22)",
         sgr(1, 3, 4) + "BEFORE" + sgr(22) + "AFTER", plain, slow, delay)
    line(w, "bold+italic+under then un-italic(23)",
         sgr(1, 3, 4) + "BEFORE" + sgr(23) + "AFTER", plain, slow, delay)
    line(w, "bold+italic+under then un-under(24)",
         sgr(1, 3, 4) + "BEFORE" + sgr(24) + "AFTER", plain, slow, delay)


def mode_semantic(w, plain, slow, delay):
    """Exercises exactly the branches ipipeAnsiToScheme takes, both the mapped
    ones and the ones that fall through to 'dead' (default/no highlight)."""
    section(w, "reachable fg scheme mappings", plain)
    cases = [
        ("fg red, not bold -> R (rmv)", sgr(31)),
        ("fg red, bold -> k (error)", sgr(1, 31)),
        ("fg green -> Q (add)", sgr(32)),
        ("fg yellow -> l (warn)", sgr(33)),
        ("fg blue -> S (dir)", sgr(34)),
        ("fg cyan -> m (info)", sgr(36)),
    ]
    for label, seq in cases:
        line(w, label, seq + "SAMPLE", plain, slow, delay)

    section(w, "dead fg colors (no scheme match, falls to style/default)", plain)
    for idx, name in ((0, "black"), (5, "magenta"), (7, "white")):
        line(w, f"fg {name} -> falls through to 'A'/style", sgr(30 + idx) + "SAMPLE", plain, slow, delay)

    section(w, "reachable bg scheme mappings", plain)
    cases = [
        ("bg red normal -> O (gdfrej)", sgr(41)),
        ("bg red bright -> u (hlred)", sgr(101)),
        ("bg green normal -> N (gdfsel)", sgr(42)),
        ("bg green bright -> v (hlgreen)", sgr(102)),
        ("bg yellow normal -> M (gdfchange)", sgr(43)),
        ("bg yellow bright -> w (hlyellow)", sgr(103)),
    ]
    for label, seq in cases:
        line(w, label, seq + "SAMPLE", plain, slow, delay)

    section(w, "dead bg colors (no scheme match, falls to 'A')", plain)
    for idx, name in ((0, "black"), (4, "blue"), (5, "magenta"), (6, "cyan"), (7, "white")):
        line(w, f"bg {name} -> falls through to 'A'", sgr(40 + idx) + "SAMPLE", plain, slow, delay)

    section(w, "precedence: bg beats fg", plain)
    line(w, "fg green + bg red normal -> should be O, not Q", sgr(32, 41) + "SAMPLE", plain, slow, delay)
    line(w, "fg cyan + bg yellow bright -> should be w, not m", sgr(36, 103) + "SAMPLE", plain, slow, delay)

    section(w, "precedence: fg beats style when fg is mapped", plain)
    line(w, "bold + fg green -> should still be Q, not just bold", sgr(1, 32) + "SAMPLE", plain, slow, delay)


def mode_extended(w, plain, slow, delay):
    section(w, "extended color (38;5;n / 38;2;r;g;b) -- expected to be IGNORED", plain)
    line(w, "256-color fg 38;5;196 (bright red-ish)", sgr(38, 5, 196) + "SAMPLE should render as default", plain, slow, delay)
    line(w, "truecolor fg 38;2;255;0;0", sgr(38, 2, 255, 0, 0) + "SAMPLE should render as default", plain, slow, delay)
    line(w, "256-color bg 48;5;22", sgr(48, 5, 22) + "SAMPLE should render as default", plain, slow, delay)

    section(w, "BUG PROBE: attrs after an extended color get dropped too", plain)
    line(w, "38;5;196 then ;1 (bold) in same SGR -- bold expected to be LOST",
         sgr(38, 5, 196, 1) + "SAMPLE bold likely missing", plain, slow, delay)
    line(w, "control: same bold alone, no preceding extended color",
         sgr(1) + "SAMPLE bold should apply", plain, slow, delay)
    line(w, "bold BEFORE the extended color instead (order matters)",
         sgr(1, 38, 5, 196) + "SAMPLE bold applied first, then color parse eats nothing after",
         plain, slow, delay)


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
