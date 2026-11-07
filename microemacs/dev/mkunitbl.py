#!/usr/bin/env python3
#
# JASSPA MicroEmacs - www.jasspa.com
# mkunitbl.py - Generate src/eunitbl.h from the Unicode Character Database.
#
# Usage: python3 -I dev/mkunitbl.py [--dump <file>] <ucd-dir> [eunitbl.h]
#
# The tables are written to ../src/eunitbl.h, next to this script, unless another file is given
#
# --dump writes the per code point properties (0x110000 bytes) followed by the lower, upper and
# fold mappings as text lines of "<from> <to>" in hex, used by test/unicode/unitest.c
#
# <ucd-dir> must contain UnicodeData.txt, EastAsianWidth.txt, CaseFolding.txt
# and PropList.txt, from https://www.unicode.org/Public/UCD/latest/ucd/
#
# Copyright (C) 2026 JASSPA (www.jasspa.com)
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option)
# any later version.

import argparse
import os
import re
import sys

UNI_MAX = 0x10ffff

# Order must match the meUNIGC_ defines in eunicode.h
GC_LIST = ['Cn', 'Lu', 'Ll', 'Lt', 'Lm', 'Lo', 'Mn', 'Mc', 'Me', 'Nd', 'Nl', 'No',
           'Pc', 'Pd', 'Ps', 'Pe', 'Pi', 'Pf', 'Po', 'Sm', 'Sc', 'Sk', 'So',
           'Zs', 'Zl', 'Zp', 'Cc', 'Cf', 'Cs', 'Co']
GC_IDX = {gc: ii for ii, gc in enumerate(GC_LIST)}

WIDTH_ZERO = 0
WIDTH_NARROW = 1
WIDTH_WIDE = 2
WIDTH_AMBIG = 3
PROP_SPACE = 0x04
PROP_GC_SHIFT = 3

CASE_COUNT_MAX = 0xffff


def ucdLines(path):
    with open(path, encoding='utf-8') as fp:
        for line in fp:
            line = line.split('#', 1)[0].strip()
            if line:
                yield [ff.strip() for ff in line.split(';')]


def parseRange(ss):
    if '..' in ss:
        lo, hi = ss.split('..')
        return int(lo, 16), int(hi, 16)
    cp = int(ss, 16)
    return cp, cp


def readUnicodeData(ucd):
    gc = bytearray(UNI_MAX + 1)          # 0 == Cn
    upper = {}
    lower = {}
    first = None
    for ff in ucdLines(os.path.join(ucd, 'UnicodeData.txt')):
        cp = int(ff[0], 16)
        gg = GC_IDX[ff[2]]
        if ff[1].endswith(', First>'):
            first = cp
            continue
        if ff[1].endswith(', Last>'):
            for cc in range(first, cp + 1):
                gc[cc] = gg
            first = None
            continue
        gc[cp] = gg
        if ff[12]:
            upper[cp] = int(ff[12], 16)
        if ff[13]:
            lower[cp] = int(ff[13], 16)
    return gc, upper, lower


def readEastAsianWidth(ucd):
    path = os.path.join(ucd, 'EastAsianWidth.txt')
    eaw = ['N'] * (UNI_MAX + 1)
    version = None
    missing = re.compile(r'#\s*@missing:\s*([0-9A-Fa-f.]+)\s*;\s*(\w+)')
    with open(path, encoding='utf-8') as fp:
        for line in fp:
            if version is None:
                mm = re.match(r'#\s*EastAsianWidth-([0-9.]+)\.txt', line)
                if mm:
                    version = mm.group(1)
            mm = missing.match(line)
            if mm:
                lo, hi = parseRange(mm.group(1))
                for cc in range(lo, hi + 1):
                    eaw[cc] = mm.group(2)
    for ff in ucdLines(path):
        lo, hi = parseRange(ff[0])
        for cc in range(lo, hi + 1):
            eaw[cc] = ff[1]
    return eaw, version


def readPropList(ucd, prop):
    cps = set()
    for ff in ucdLines(os.path.join(ucd, 'PropList.txt')):
        if ff[1] == prop:
            lo, hi = parseRange(ff[0])
            cps.update(range(lo, hi + 1))
    return cps


def readCaseFolding(ucd):
    fold = {}
    for ff in ucdLines(os.path.join(ucd, 'CaseFolding.txt')):
        if ff[1] in ('C', 'S'):
            fold[int(ff[0], 16)] = int(ff[2], 16)
    return fold


def width(cp, gcName, eaw, pcm):
    # Zero width follows the Kuhn wcwidth rules: non-spacing & enclosing marks, format characters
    # except the soft hyphen and prepended concatenation marks, and the Hangul medial vowels
    # and final consonants which join the preceding leading consonant
    if gcName in ('Mn', 'Me', 'Cc'):
        return WIDTH_ZERO
    if gcName == 'Cf' and cp != 0x00ad and cp not in pcm:
        return WIDTH_ZERO
    if (0x1160 <= cp <= 0x11ff) or (0xd7b0 <= cp <= 0xd7ff):
        return WIDTH_ZERO
    if eaw in ('W', 'F'):
        return WIDTH_WIDE
    if eaw == 'A':
        return WIDTH_AMBIG
    return WIDTH_NARROW


def caseRuns(mapping):
    runs = []
    for src in sorted(mapping):
        delta = mapping[src] - src
        if runs:
            run = runs[-1]
            last = run[0] + (run[1] - 1) * run[2]
            if (run[3] == delta) and (run[1] < CASE_COUNT_MAX):
                if (run[1] == 1) and ((src - last) in (1, 2)):
                    run[1] = 2
                    run[2] = src - last
                    continue
                if (run[1] > 1) and (src - last == run[2]):
                    run[1] += 1
                    continue
        runs.append([src, 1, 1, delta])
    return runs


def checkCaseRuns(runs, mapping):
    got = {}
    for start, count, stride, delta in runs:
        for ii in range(count):
            got[start + ii * stride] = start + ii * stride + delta
    assert got == mapping, 'case run compression failed'


def writeCaseRuns(out, name, runs):
    out.append('static const meUniCaseRun %s[%d] =\n{' % (name, len(runs)))
    for ii in range(0, len(runs), 3):
        out.append('    ' + ' '.join('{0x%05x,%6d,%5d,%d},' % (rr[0], rr[3], rr[1], rr[2])
                                     for rr in runs[ii:ii + 3]))
    out.append('};\n')


# The BMP also has two level tables, so its lookups need no search: a page index per block of
# chars and the distinct blocks
PROP_BLOCK_SHIFT = 6
CASE_BLOCK_SHIFT = 5
BMP_END = 0x10000


def pageTables(values, shift):
    """splits values (one per BMP char) into blocks of 1 << shift, returns (index, blocks)"""
    size = 1 << shift
    index, blocks, seen = [], [], {}
    for ii in range(0, BMP_END, size):
        blk = tuple(values[ii:ii + size])
        if blk not in seen:
            seen[blk] = len(blocks)
            blocks.append(blk)
        index.append(seen[blk])
    assert len(blocks) <= 256, 'page index no longer fits a byte'
    return index, blocks


def writeIndex(out, name, index):
    out.append('static const meUByte %s[%d] =\n{' % (name, len(index)))
    for ii in range(0, len(index), 16):
        out.append('    ' + ','.join('%3d' % vv for vv in index[ii:ii + 16]) + ',')
    out.append('};\n')


def main():
    ap = argparse.ArgumentParser(description='Generate eunitbl.h from the Unicode Character Database')
    ap.add_argument('--dump', help='also write the expanded tables for unitest')
    ap.add_argument('ucd', help='directory containing the UCD files')
    ap.add_argument('out', nargs='?', default=os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'src', 'eunitbl.h'))
    args = ap.parse_args()
    ucd = args.ucd
    outPath = args.out

    gc, upper, lower = readUnicodeData(ucd)
    eaw, version = readEastAsianWidth(ucd)
    space = readPropList(ucd, 'White_Space')
    pcm = readPropList(ucd, 'Prepended_Concatenation_Mark')
    fold = readCaseFolding(ucd)
    if version is None:
        sys.exit('Failed to find the Unicode version in EastAsianWidth.txt')

    props = bytearray(UNI_MAX + 1)
    for cp in range(UNI_MAX + 1):
        gg = gc[cp]
        pp = (gg << PROP_GC_SHIFT) | width(cp, GC_LIST[gg], eaw[cp], pcm)
        if cp in space:
            pp |= PROP_SPACE
        props[cp] = pp

    propRuns = [0]
    for cp in range(1, UNI_MAX + 1):
        if props[cp] != props[cp - 1]:
            propRuns.append(cp)

    lowerRuns = caseRuns(lower)
    upperRuns = caseRuns(upper)
    foldRuns = caseRuns(fold)
    checkCaseRuns(lowerRuns, lower)
    checkCaseRuns(upperRuns, upper)
    checkCaseRuns(foldRuns, fold)

    out = ['/* Generated by mkunitbl.py from the Unicode %s Character Database - DO NOT EDIT */\n' % version,
           '#define meUNICODE_VERSION "%s"\n' % version]
    out.append('/* (start << 8) | properties, one per run of identical properties */')
    out.append('static const meUInt meUniPropRuns[%d] =\n{' % len(propRuns))
    for ii in range(0, len(propRuns), 8):
        out.append('    ' + ','.join('0x%08x' % ((cp << 8) | props[cp]) for cp in propRuns[ii:ii + 8]) + ',')
    out.append('};\n')
    out.append('/* BMP properties: meUniPropBlocks[(meUniPropIndex[uc >> %d] << %d) | (uc & 0x%x)] */'
               % (PROP_BLOCK_SHIFT, PROP_BLOCK_SHIFT, (1 << PROP_BLOCK_SHIFT) - 1))
    index, blocks = pageTables(props[:BMP_END], PROP_BLOCK_SHIFT)
    writeIndex(out, 'meUniPropIndex', index)
    out.append('static const meUByte meUniPropBlocks[%d] =\n{' % (len(blocks) << PROP_BLOCK_SHIFT))
    for blk in blocks:
        for ii in range(0, len(blk), 16):
            out.append('    ' + ','.join('0x%02x' % vv for vv in blk[ii:ii + 16]) + ',')
    out.append('};\n')
    out.append('/* BMP case mappings, {lower, upper, fold} deltas: meUniCaseBlocks[(meUniCaseIndex[uc >> %d] << %d) | (uc & 0x%x)] */'
               % (CASE_BLOCK_SHIFT, CASE_BLOCK_SHIFT, (1 << CASE_BLOCK_SHIFT) - 1))
    deltas = [(lower.get(cp, cp) - cp, upper.get(cp, cp) - cp, fold.get(cp, cp) - cp) for cp in range(BMP_END)]
    index, blocks = pageTables(deltas, CASE_BLOCK_SHIFT)
    writeIndex(out, 'meUniCaseIndex', index)
    out.append('static const meInt meUniCaseBlocks[%d][3] =\n{' % (len(blocks) << CASE_BLOCK_SHIFT))
    for blk in blocks:
        for ii in range(0, len(blk), 4):
            out.append('    ' + ' '.join('{%d,%d,%d},' % dd for dd in blk[ii:ii + 4]))
    out.append('};\n')
    out.append('/* {start, delta, count, stride} */')
    writeCaseRuns(out, 'meUniLowerRuns', lowerRuns)
    writeCaseRuns(out, 'meUniUpperRuns', upperRuns)
    writeCaseRuns(out, 'meUniFoldRuns', foldRuns)

    with open(outPath, 'w', encoding='ascii', newline='\n') as fp:
        fp.write('\n'.join(out))
    if args.dump:
        with open(args.dump, 'wb') as fp:
            fp.write(bytes(props))
            for name, mapping in (('lower', lower), ('upper', upper), ('fold', fold)):
                fp.write(('%s %d\n' % (name, len(mapping))).encode('ascii'))
                for src in sorted(mapping):
                    fp.write(('%x %x\n' % (src, mapping[src])).encode('ascii'))
    sys.stderr.write('Unicode %s: %d property runs, %d lower, %d upper, %d fold case runs\n'
                     % (version, len(propRuns), len(lowerRuns), len(upperRuns), len(foldRuns)))


if __name__ == '__main__':
    main()
