#!/bin/sh
# Loads and saves every corpus/*.txt file with a headless mec and reports any file which does
# not save byte identical (acceptance tests F1-F5, F8, F10, F16, F17).
# Usage: roundtrip.sh <mec> [corpus-dir]
# Only the start-up file below is run, not me.emf, so no file hooks or translate-buffer apply.
if [ $# -lt 1 ] ; then
    echo "Usage: $0 <mec> [corpus-dir]"
    exit 1
fi
MEC=$1
TDIR=$(cd "$(dirname "$0")" && pwd)
CORPUS=$(cd "${2:-$TDIR/corpus}" && pwd)
WDIR=$(mktemp -d)
trap 'rm -rf "$WDIR"' EXIT
mkdir "$WDIR/out"

{
    echo 'define-macro start-up'
    for ff in "$CORPUS"/*.txt ; do
        bb=$(basename "$ff")
        echo "  !force find-file \"$ff\""
        echo "  !if \$status"
        echo "    !force 0 write-buffer \"$WDIR/out/$bb\""
        echo "  !endif"
        echo "  !force delete-buffer \$buffer-bname"
    done
    echo '  4 exit-emacs 0'
    echo '!emacro'
} > "$WDIR/rt.emf"

# timeout is not part of macOS, perl's alarm kills mec the same way
if command -v timeout > /dev/null 2>&1 ; then
    TIMEOUT="timeout -s KILL 120"
else
    TIMEOUT="perl -e 'alarm 120; exec @ARGV'"
fi
MEPATH="$WDIR" eval $TIMEOUT '"$MEC"' -p '"@$WDIR/rt"' < /dev/null > "$WDIR/mec.log" 2>&1

pass=0
fail=0
for ff in "$CORPUS"/*.txt ; do
    bb=$(basename "$ff")
    if [ ! -f "$WDIR/out/$bb" ] ; then
        echo "FAIL $bb - not written"
        fail=$((fail+1))
    elif cmp -s "$ff" "$WDIR/out/$bb" ; then
        pass=$((pass+1))
    else
        echo "FAIL $bb - $(cmp "$ff" "$WDIR/out/$bb" 2>&1 | head -1 | sed 's/.*differ: //'), size $(wc -c < "$ff") -> $(wc -c < "$WDIR/out/$bb")"
        fail=$((fail+1))
    fi
done
echo "$pass passed, $fail failed"
[ $fail -eq 0 ]
