#!/bin/bash
# ipipe_codes_test.sh - test terminal control-code handling implemented in
# ipipeRead() (src/spawn.c).
#
# Run in both ME ishell and a real terminal to compare behaviour.
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

NOPROMPT=0
for arg in "$@"; do
    case "$arg" in
    --noprompt) NOPROMPT=1 ;;
    -h|--help)
        echo "usage: $0 [--noprompt]"
        exit 0
        ;;
    esac
done

pause()
{
    [ "$NOPROMPT" = 1 ] && return
    printf -- '--- press any key to continue ---'
    read -n1 -s -r
    printf '\r\033[K'
}

echo "=== 1. Backspace (BS, ^H) ==="
printf 'ABCDE'
printf '\010\010'
printf 'XY\n'
echo "--- expect: ABCXY ---"
pause
echo ""

echo "=== 2. Carriage return without newline (CR, \\r) ==="
printf 'Hello World'
printf '\r'
printf 'Bye\n'
echo "--- expect: Byelo World ---"
pause
echo ""

echo "=== 3. Cursor forward/back (CUF \\E[nC / CUB \\E[nD) ==="
printf 'AAAAAAAAAA'
printf '\033[5D'
printf 'XXXXX\n'
echo "--- expect: AAAAAXXXXX ---"
pause
echo ""

echo "=== 4. Cursor up/down (CUU \\E[nA / CUD \\E[nB), column preserved ==="
printf '%s\n' '1111111111'
printf '%s\n' '2222222222'
printf '%s\n' '3333333333'
printf '\033[3A'   # up to row 1, col 0
printf '\033[4C'   # right to col 4 (0-based)
printf 'X'         # overwrite col 4, cursor now at col 5
printf '\033[2B'   # down to row 3, col preserved (5)
printf 'Y\n'        # overwrite col 5
echo "--- expect:"
echo "    1111X11111"
echo "    2222222222"
echo "    33333Y3333"
echo "---"
pause
echo ""

echo "=== 5. Cursor horizontal absolute (CHA \\E[nG) ==="
printf 'ABCDEFGHIJ'
printf '\033[6G'   # column 6 (1-based) = index 5
printf '*\n'
echo "--- expect: ABCDE*GHIJ ---"
pause
echo ""

echo "=== 6. Erase to end of line (EL \\E[K) ==="
printf 'ABCDEFGHIJ'
printf '\033[5D'
printf '\033[K\n'
echo "--- expect: ABCDE (rest erased) ---"
pause
echo ""

echo "=== 7. Erase display, cursor to end of screen (ED \\E[0J) ==="
printf '%s\n' 'LINE1'
printf '%s\n' 'LINE2'
printf '%s\n' 'LINE3'
printf '\033[2A'    # up to LINE2, col 0
printf '\033[0J'
printf '\n\n'        # ED doesn't move the cursor; two newlines clear both blanked rows
echo "--- expect: LINE1 remains, LINE2 and LINE3 blanked ---"
pause
echo ""

echo "=== 8. Erase whole screen (ED \\E[2J) ==="
printf '%s\n' 'BEFORE-CLEAR'
printf '\033[2J'
printf 'after-clear\n'
echo "--- expect: BEFORE-CLEAR line blanked, only 'after-clear' visible ---"
pause
echo ""

echo "=== 9. Cursor position (CUP \\E[row;colH) ==="
# Note: \E[2J only erases the current viewport, not scrollback history, so
# this header (and anything above it) may still appear if you scroll back
# or copy-paste the whole session. Judge this test by the live screen: the
# line below should be the first/topmost visible line once it runs.
printf '\033[2J\033[H'  # clear and home first: row/col below are then unambiguous
printf '%s\n' 'AAAAA'
printf '%s\n' 'BBBBB'
printf '%s\n' 'CCCCC'
printf '\033[2;2H'  # row 2, col 2 (1-based) -> the B row
printf '*'
printf '\033[3;1H\n'  # move past the C row (CUP doesn't advance a line) before continuing
echo "--- expect (ignore any header/blank leftovers above from scrollback):"
echo "    AAAAA"
echo "    B*BBB"
echo "    CCCCC"
echo "---"
pause
echo ""

echo "=== 10. Save/restore cursor (DECSC \\E7 / DECRC \\E8) ==="
printf '\033[2J\033[H'  # clear and home first, same reasoning as test 9
printf '%s\n' 'XXXXXXXXXX'
printf '%s\n' 'YYYYYYYYYY'
printf '\033[1;1H'
printf '\0337'      # save cursor at row1,col1
printf '\033[2;5H'
printf '*'          # mark row2,col5
printf '\0338'      # restore to row1,col1
printf '#'
printf '\033[2;1H\n'  # move past the Y row (row 2) before continuing
echo "--- expect (ignore any header/blank leftovers above from scrollback):"
echo "    #XXXXXXXXX"
echo "    YYYY*YYYYY"
echo "---"
pause
echo ""

echo "=== 11. Insert character (ICH \\E[n@) ==="
printf 'ABCDEFGHIJ'
printf '\033[5D'
printf '\033[3@'
printf '\n'
echo "--- expect: ABCDE   FGHIJ (3 blanks inserted before F) ---"
pause
echo ""

echo "=== 12. Delete character (DCH \\E[nP) ==="
printf 'ABCDEFGHIJ'
printf '\033[5D'
printf '\033[3P'
printf '\n'
echo "--- expect: ABCDEIJ (F,G,H deleted, IJ shift left) ---"
pause
echo ""

echo "=== 13. SGR colour/style (\\E[...m) ==="
printf 'Normal '
printf '\033[1m'
printf 'Bold '
printf '\033[31m'
printf 'Red '
printf '\033[0m'
printf 'Normal\n'
echo "--- expect: text reads 'Normal Bold Red Normal'; styling only visible if the buffer has ANSICOLOR mode enabled ---"
pause
echo ""

echo "=== 14. Bell (BEL, ^G) ==="
printf 'ring\007\n'
echo "--- expect: audible bell / bell indication, text 'ring' unaffected ---"
pause
echo ""

echo "--- done ---"
