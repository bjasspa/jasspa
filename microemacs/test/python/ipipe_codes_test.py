#!/usr/bin/env python3
# ipipe_codes_test.py - test terminal control-code handling implemented in
# ipipeRead() (src/spawn.c).
#
# Windows/cmd-prompt port of ipipe_codes_test.sh. Use this version when
# testing from cmd.exe or PowerShell, since the .sh script relies on bash
# builtins (printf, read -n1 -s -r, [ ]) that are not available there.
# Run it both inside ME's ipipe and in a plain console window, and compare.
#
# See wrap_test.sh for the auto-wrap (\E[?7h / \E[?7l) specific test.
#
# Each section prints the escape sequences under test, followed by the
# "--- expect: ... ---" line describing what a correct implementation
# should show. Compare the actual rendered lines against that line.
#
# By default the script pauses after each section so the result can be
# read before it scrolls away. Pass --noprompt to run straight through.
#
# Tests that rely on absolute cursor positioning (\E[row;colH) first clear
# and home the screen (\E[2J\E[H) so "row N" means row N of our own output,
# not row N of whatever the terminal's viewport happens to be scrolled to.
# Those tests are ordered after the \E[2J test so that erase-screen is
# itself verified before later tests rely on it.

import sys

NOPROMPT = "--noprompt" in sys.argv[1:]
if any(a in ("-h", "--help") for a in sys.argv[1:]):
    print("usage: %s [--noprompt]" % sys.argv[0])
    raise SystemExit(0)


def w(s):
    sys.stdout.write(s)


def enable_vt_on_windows():
    """Turn on ENABLE_VIRTUAL_TERMINAL_PROCESSING so a plain cmd.exe window
    (not Windows Terminal) renders the escape codes instead of showing them
    literally. No-op everywhere else - in particular this is not needed (and
    not touched) when running under ME's ipipe, which already understands
    VT via ConPTY."""
    if sys.platform != "win32":
        return
    try:
        import ctypes
        kernel32 = ctypes.windll.kernel32
        STD_OUTPUT_HANDLE = -11
        ENABLE_VIRTUAL_TERMINAL_PROCESSING = 0x0004
        h = kernel32.GetStdHandle(STD_OUTPUT_HANDLE)
        mode = ctypes.c_uint32()
        if kernel32.GetConsoleMode(h, ctypes.byref(mode)):
            kernel32.SetConsoleMode(h, mode.value | ENABLE_VIRTUAL_TERMINAL_PROCESSING)
    except Exception:
        pass


def getch():
    """Read and discard a single keypress without echoing, cross-platform."""
    if sys.platform == "win32":
        import msvcrt
        msvcrt.getch()
        return
    try:
        import termios
        import tty
        fd = sys.stdin.fileno()
        old = termios.tcgetattr(fd)
        try:
            tty.setraw(fd)
            sys.stdin.read(1)
        finally:
            termios.tcsetattr(fd, termios.TCSADRAIN, old)
    except Exception:
        sys.stdin.readline()


def pause():
    if NOPROMPT:
        return
    w("--- press any key to continue ---")
    sys.stdout.flush()
    getch()
    w("\r\x1b[K")
    sys.stdout.flush()


enable_vt_on_windows()

w("=== 1. Backspace (BS, ^H) ===\n")
w("ABCDE")
w("\x08\x08")
w("XY\n")
w("--- expect: ABCXY ---\n")
sys.stdout.flush()
pause()
w("\n")

w("=== 2. Carriage return without newline (CR, \\r) ===\n")
w("Hello World")
w("\r")
w("Bye\n")
w("--- expect: Byelo World ---\n")
sys.stdout.flush()
pause()
w("\n")

w("=== 3. Cursor forward/back (CUF \\E[nC / CUB \\E[nD) ===\n")
w("AAAAAAAAAA")
w("\x1b[5D")
w("XXXXX\n")
w("--- expect: AAAAAXXXXX ---\n")
sys.stdout.flush()
pause()
w("\n")

w("=== 4. Cursor up/down (CUU \\E[nA / CUD \\E[nB), column preserved ===\n")
w("1111111111\n")
w("2222222222\n")
w("3333333333\n")
w("\x1b[3A")   # up to row 1, col 0
w("\x1b[4C")   # right to col 4 (0-based)
w("X")         # overwrite col 4, cursor now at col 5
w("\x1b[2B")   # down to row 3, col preserved (5)
w("Y\n")       # overwrite col 5
w("--- expect:\n")
w("    1111X11111\n")
w("    2222222222\n")
w("    33333Y3333\n")
w("---\n")
sys.stdout.flush()
pause()
w("\n")

w("=== 5. Cursor horizontal absolute (CHA \\E[nG) ===\n")
w("ABCDEFGHIJ")
w("\x1b[6G")   # column 6 (1-based) = index 5
w("*\n")
w("--- expect: ABCDE*GHIJ ---\n")
sys.stdout.flush()
pause()
w("\n")

w("=== 6. Erase to end of line (EL \\E[K) ===\n")
w("ABCDEFGHIJ")
w("\x1b[5D")
w("\x1b[K\n")
w("--- expect: ABCDE (rest erased) ---\n")
sys.stdout.flush()
pause()
w("\n")

w("=== 7. Erase display, cursor to end of screen (ED \\E[0J) ===\n")
w("LINE1\n")
w("LINE2\n")
w("LINE3\n")
w("\x1b[2A")    # up to LINE2, col 0
w("\x1b[0J")
w("\n\n")       # ED doesn't move the cursor; two newlines clear both blanked rows
w("--- expect: LINE1 remains, LINE2 and LINE3 blanked ---\n")
sys.stdout.flush()
pause()
w("\n")

w("=== 8. Erase whole screen (ED \\E[2J) ===\n")
w("BEFORE-CLEAR\n")
w("\x1b[2J")
w("after-clear\n")
w("--- expect: BEFORE-CLEAR line blanked, only 'after-clear' visible ---\n")
sys.stdout.flush()
pause()
w("\n")

w("=== 9. Cursor position (CUP \\E[row;colH) ===\n")
# Note: \E[2J only erases the current viewport, not scrollback history, so
# this header (and anything above it) may still appear if you scroll back
# or copy-paste the whole session. Judge this test by the live screen: the
# line below should be the first/topmost visible line once it runs.
w("\x1b[2J\x1b[H")  # clear and home first: row/col below are then unambiguous
w("AAAAA\n")
w("BBBBB\n")
w("CCCCC\n")
w("\x1b[2;2H")  # row 2, col 2 (1-based) -> the B row
w("*")
w("\x1b[3;1H\n")  # move past the C row (CUP doesn't advance a line) before continuing
w("--- expect (ignore any header/blank leftovers above from scrollback):\n")
w("    AAAAA\n")
w("    B*BBB\n")
w("    CCCCC\n")
w("---\n")
sys.stdout.flush()
pause()
w("\n")

w("=== 10. Save/restore cursor (DECSC \\E7 / DECRC \\E8) ===\n")
w("\x1b[2J\x1b[H")  # clear and home first, same reasoning as test 9
w("XXXXXXXXXX\n")
w("YYYYYYYYYY\n")
w("\x1b[1;1H")
w("\x1b7")     # save cursor at row1,col1
w("\x1b[2;5H")
w("*")         # mark row2,col5
w("\x1b8")     # restore to row1,col1
w("#")
w("\x1b[2;1H\n")  # move past the Y row (row 2) before continuing
w("--- expect (ignore any header/blank leftovers above from scrollback):\n")
w("    #XXXXXXXXX\n")
w("    YYYY*YYYYY\n")
w("---\n")
sys.stdout.flush()
pause()
w("\n")

w("=== 11. Insert character (ICH \\E[n@) ===\n")
w("ABCDEFGHIJ")
w("\x1b[5D")
w("\x1b[3@")
w("\n")
w("--- expect: ABCDE   FGHIJ (3 blanks inserted before F) ---\n")
sys.stdout.flush()
pause()
w("\n")

w("=== 12. Delete character (DCH \\E[nP) ===\n")
w("ABCDEFGHIJ")
w("\x1b[5D")
w("\x1b[3P")
w("\n")
w("--- expect: ABCDEIJ (F,G,H deleted, IJ shift left) ---\n")
sys.stdout.flush()
pause()
w("\n")

w("=== 13. SGR colour/style (\\E[...m) ===\n")
w("Normal ")
w("\x1b[1m")
w("Bold ")
w("\x1b[31m")
w("Red ")
w("\x1b[0m")
w("Normal\n")
w("--- expect: text reads 'Normal Bold Red Normal'; styling only visible if the buffer has ANSICOLOR mode enabled ---\n")
sys.stdout.flush()
pause()
w("\n")

w("=== 14. Bell (BEL, ^G) ===\n")
w("ring\x07\n")
w("--- expect: audible bell / bell indication, text 'ring' unaffected ---\n")
sys.stdout.flush()
pause()
w("\n")

w("--- done ---\n")
sys.stdout.flush()
