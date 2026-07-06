#!/bin/bash
# wrap_test.sh - test terminal auto-wrap handling
# Run in both ME ishell and a real terminal to compare behaviour.
#
# Expected in a real terminal:
#   wrap OFF: long line stops/overwrites at right margin, no continuation
#   wrap ON:  long line continues onto next line
#
# Expected in ME ishell if wrap is not implemented:
#   both sections look identical

WIDTH=$(tput cols 2>/dev/null || echo 80)
echo "Terminal width reported: $WIDTH"
echo ""

# Build a line: (WIDTH-1) A's + pipe marker at column WIDTH + 40 B's beyond
LONG=$(printf '%*s' $((WIDTH - 1)) '' | tr ' ' 'A')'|'$(printf '%*s' 40 '' | tr ' ' 'B')
echo "Line length: ${#LONG} chars (margin at col $WIDTH)"
echo ""

echo "=== Auto-wrap DISABLED (\E[?7l) ==="
printf '\033[?7l'
printf '%s\n' "$LONG"
printf '\033[?7h'    # restore wrap immediately after

echo ""
echo "=== Auto-wrap ENABLED (\E[?7h) ==="
printf '\033[?7h'
printf '%s\n' "$LONG"

echo ""
echo "--- done ---"
echo "Real terminal : OFF=truncate/overwrite at col $WIDTH, ON=wrap to next line"
echo "ME ishell     : both identical if wrap mode is not implemented"
