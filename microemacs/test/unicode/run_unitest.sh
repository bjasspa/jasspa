#!/bin/sh
# Regenerates the Unicode tables with a dump, builds and runs the unicode.c unit tests.
# Usage: run_unitest.sh <ucd-dir> [src-dir]
# <ucd-dir> holds UnicodeData.txt, EastAsianWidth.txt, CaseFolding.txt & PropList.txt
set -e
if [ $# -lt 1 ] ; then
    echo "Usage: $0 <ucd-dir> [src-dir]"
    exit 1
fi
UCD=$1
TDIR=$(cd "$(dirname "$0")" && pwd)
SDIR=$(cd "${2:-$TDIR/../../src}" && pwd)
WDIR=$(mktemp -d)
trap 'rm -rf "$WDIR"' EXIT

python3 -I "$TDIR/../../dev/mkunitbl.py" --dump "$WDIR/unitbl.dump" "$UCD" "$WDIR/eunitbl.h"
if ! cmp -s "$WDIR/eunitbl.h" "$SDIR/eunitbl.h" ; then
    echo "WARNING: $SDIR/eunitbl.h differs from one generated from $UCD"
fi
gcc -O2 -Wall -Wextra -I "$WDIR" -I "$SDIR" -o "$WDIR/unitest" "$TDIR/unitest.c"
"$WDIR/unitest" "$WDIR/unitbl.dump"
