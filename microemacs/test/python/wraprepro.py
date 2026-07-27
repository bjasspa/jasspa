import sys
import time

# Deterministic repro v4 for the ME ipipe row-drift bug.
#
# Root cause theory (derived from exact node-identity tracing of the
# real claude /diff log): when a line auto-wraps (spawn.c's default
# split branch, ~1676-1697), it inserts new content for the wrap's
# row WITHOUT retiring the pre-existing old line at that row -- lp_old
# is left unmoved, pointing at whatever was there BEFORE the wrap
# started. The following NL commit then retires *that* node (the old
# row-N content), not the old row-(N+1) content the wrap's second row
# is actually replacing. The old row-(N+1) line is never targeted by
# any retire and is silently stranded in the list -- surfacing later,
# one row down from everything that follows.
#
# This test sets up two rows of pre-existing, uniquely-identifiable
# content, then overwrites exactly those two rows via a SINGLE write
# that force-wraps mid-stream with only one trailing \n (mirroring
# the real "rule + header" sequence, which has no \n between the
# wrapped rule and the header text that follows it on the same row).
#
# Run with the ConPTY sized to 80 columns x 24 rows, same as v1-v3.
#
# Expected correct output on a real terminal:
#   rows 0-5   : LINE 01..LINE 06
#   rows 6-7   : 90 Z's, wrapped, ending with "AFTERWRAP-newrow7"
#   rows 8-10  : LINE 08, LINE 09, LINE 10  (NOT the old OLDROW7 marker)
#   rows 11-15 : LINE 11..LINE 15

out = sys.stdout

for n in range(1, 7):
    out.write("LINE %02d\n" % n)          # rows 0-5

# Pre-existing content at rows 6-7 that the wrap below will overwrite.
out.write("OLDROW6-marker-text-here\n")   # row 6
out.write("OLDROW7-marker-text-here\n")   # row 7

for n in range(8, 11):
    out.write("LINE %02d\n" % n)          # rows 8-10
out.flush()
time.sleep(0.3)

# Jump back to row 6 (1-based row 7) and overwrite rows 6-7 in place
# via one continuous wrap -- no \n until both rows are filled.
out.write("\x1b[7;1H")
out.write("Z" * 90 + "AFTERWRAP-newrow7\n")
out.flush()
time.sleep(0.1)

for n in range(11, 16):
    out.write("LINE %02d\n" % n)
out.flush()
