/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * unicode.c - Unicode character support.
 *
 * Copyright (C) 2026 JASSPA (www.jasspa.com)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 675 Mass Ave, Cambridge, MA 02139, USA.
 */
/*
 * Created:     Tue Oct 6 2026
 * Synopsis:    Unicode character support.
 * Authors:     Steven Phillips
 * Description:
 *     UTF-8 encoding, decoding & navigation of the internal text format (see
 *     eunicode.h) and the character property & case tables generated into
 *     eunitbl.h by dev/mkunitbl.py.
 */

#define __UNICODEC

#ifndef _ME_UNITEST
#include "emain.h"
#endif

#if MEOPT_UNICODE

typedef struct {
    meUInt   start;
    meInt    delta;
    meUShort count;
    meUShort stride;
} meUniCaseRun;

#include "eunitbl.h"

#define meUniTblSize(tbl) ((int) (sizeof(tbl)/sizeof(tbl[0])))

/* Strictly validates the UTF-8 sequence at ss (Unicode Table 3-7), returns its length, 0 if
 * invalid or -1 if ss is a valid prefix cut short by len */
int
meUtf8SeqLen(const meUByte *ss, int len)
{
    meUByte cc=ss[0], lo=0x80, hi=0xbf;
    int ll, ii;

    if(cc < 0x80)
        return 1;
    if(cc < 0xc2)
        return 0;
    if(cc < 0xe0)
        ll = 2;
    else if(cc < 0xf0)
    {
        ll = 3;
        if(cc == 0xe0)
            lo = 0xa0;
        else if(cc == 0xed)
            hi = 0x9f;
    }
    else if(cc < 0xf5)
    {
        ll = 4;
        if(cc == 0xf0)
            lo = 0x90;
        else if(cc == 0xf4)
            hi = 0x8f;
    }
    else
        return 0;
    for(ii=1 ; ii<ll ; ii++)
    {
        if(ii >= len)
            return -1;
        cc = ss[ii];
        if((cc < lo) || (cc > hi))
            return 0;
        lo = 0x80;
        hi = 0xbf;
    }
    return ll;
}

/* Decodes one character of internal text, never fails - any byte which is not part of a
 * valid sequence is returned as a raw byte */
int
meUtf8Decode(const meUByte *ss, int len, meUInt *uc)
{
    meUByte cc=ss[0];
    int ll;

    if(cc < 0x80)
    {
        *uc = cc;
        return 1;
    }
    /* the common case of a valid 2 byte sequence */
    if((cc >= 0xc2) && (cc < 0xe0) && (len > 1) && ((ss[1] & 0xc0) == 0x80))
    {
        *uc = (((meUInt) (cc & 0x1f)) << 6) | (ss[1] & 0x3f);
        return 2;
    }
    if((ll = meUtf8SeqLen(ss,len)) > 0)
    {
        if(ll == 2)
            *uc = (((meUInt) (cc & 0x1f)) << 6) | (ss[1] & 0x3f);
        else if(ll == 3)
            *uc = (((meUInt) (cc & 0x0f)) << 12) | (((meUInt) (ss[1] & 0x3f)) << 6) | (ss[2] & 0x3f);
        else
            *uc = (((meUInt) (cc & 0x07)) << 18) | (((meUInt) (ss[1] & 0x3f)) << 12) |
                  (((meUInt) (ss[2] & 0x3f)) << 6) | (ss[3] & 0x3f);
        return ll;
    }
    if((len > 1) && ((ss[1] & 0xc0) == 0x80))
    {
        if((cc == 0xe0) && (len > 2) && ((ss[2] & 0xc0) == 0x80))
        {
            if((ss[1] == 0x80) && (ss[2] == 0x80))
            {
                *uc = 0;
                return 3;
            }
            if((ss[1] == 0x81) || (ss[1] == 0x82))
            {
                *uc = meUNI_RAW_BASE + ((ss[1] - 0x81) << 6) + (ss[2] & 0x3f);
                return 3;
            }
        }
        if((cc & 0xfe) == 0xc0)
        {
            *uc = meUNI_RAW_BASE + (0x80 | ((cc & 0x01) << 6) | (ss[1] & 0x3f));
            return 2;
        }
        if((cc == 0xed) && (ss[1] >= 0xa0) && (len > 2) && ((ss[2] & 0xc0) == 0x80))
        {
            *uc = 0xd000 | (((meUInt) (ss[1] & 0x3f)) << 6) | (ss[2] & 0x3f);
            return 3;
        }
    }
    *uc = meUNI_RAW_BASE + cc;
    return 1;
}

/* Decodes a search pattern char, as meUtf8Decode but a stray byte (not an internal raw byte
 * escape) is the char of that value, as \xNN is */
int
meUtf8DecodePat(const meUByte *ss, int len, meUInt *uc)
{
    int ll=meUtf8Decode(ss,len,uc);
    if((ll == 1) && meUniIsRaw(*uc))
        *uc = meUniRawByte(*uc);
    return ll;
}

int
meUtf8Encode(meUByte *dd, meUInt uc)
{
    if(uc == 0)
    {
        dd[0] = 0xe0;
        dd[1] = 0x80;
        dd[2] = 0x80;
        return 3;
    }
    if(uc < 0x80)
    {
        dd[0] = (meUByte) uc;
        return 1;
    }
    if(uc < 0x800)
    {
        dd[0] = (meUByte) (0xc0 | (uc >> 6));
        dd[1] = (meUByte) (0x80 | (uc & 0x3f));
        return 2;
    }
    if(uc < 0x10000)
    {
        dd[0] = (meUByte) (0xe0 | (uc >> 12));
        dd[1] = (meUByte) (0x80 | ((uc >> 6) & 0x3f));
        dd[2] = (meUByte) (0x80 | (uc & 0x3f));
        return 3;
    }
    if(uc <= meUNI_MAX)
    {
        dd[0] = (meUByte) (0xf0 | (uc >> 18));
        dd[1] = (meUByte) (0x80 | ((uc >> 12) & 0x3f));
        dd[2] = (meUByte) (0x80 | ((uc >> 6) & 0x3f));
        dd[3] = (meUByte) (0x80 | (uc & 0x3f));
        return 4;
    }
    if((uc -= meUNI_RAW_BASE) < 0x80)
    {
        dd[0] = 0xe0;
        dd[1] = (meUByte) (0x81 + (uc >> 6));
        dd[2] = (meUByte) (0x80 | (uc & 0x3f));
        return 3;
    }
    if(uc < 0x100)
    {
        dd[0] = (meUByte) (0xc0 | ((uc >> 6) & 0x01));
        dd[1] = (meUByte) (0x80 | (uc & 0x3f));
        return 2;
    }
    return meUtf8Encode(dd,meUNI_REPLACE);
}

int
meUtf8Next(const meUByte *text, int off, int len)
{
    meUInt uc;
    if(off >= len)
        return len;
    if(text[off] < 0x80)
        return off+1;
    return off + meUtf8Decode(text+off,len-off,&uc);
}

/* off must be on a character boundary */
int
meUtf8Prev(const meUByte *text, int off)
{
    meUInt uc;
    int ii, stop;

    if(off <= 0)
        return 0;
    ii = off - 1;
    if(text[ii] < 0x80)
        return ii;
    stop = (off > meUTF8_LEN_MAX) ? off - meUTF8_LEN_MAX:0;
    while((ii > stop) && ((text[ii] & 0xc0) == 0x80))
        ii--;
    if((ii + meUtf8Decode(text+ii,off-ii,&uc)) == off)
        return ii;
    return off - 1;
}

/* Raw bytes and invalid values return 0, i.e. unassigned and zero width */
meUByte
meUniProps(meUInt uc)
{
    int lo, hi, mid;

    if(uc < 0x10000)
        return meUniPropBlocks[(meUniPropIndex[uc >> 6] << 6) | (uc & 0x3f)];
    if(uc > meUNI_MAX)
        return 0;
    lo = 0;
    hi = meUniTblSize(meUniPropRuns) - 1;
    while(lo < hi)
    {
        mid = (lo + hi + 1) >> 1;
        if((meUniPropRuns[mid] >> 8) <= uc)
            lo = mid;
        else
            hi = mid - 1;
    }
    return (meUByte) meUniPropRuns[lo];
}

/* Returns the spacing form a combining mark is drawn as in its own cell, 0 if it has none */
meUInt
meUniSpacingMark(meUInt uc)
{
    int lo=0, hi=meUniTblSize(meUniSpacingMarks)-1, mid;

    if((uc < meUniSpacingMarks[0][0]) || (uc > meUniSpacingMarks[hi][0]))
        return 0;
    while(lo <= hi)
    {
        mid = (lo + hi) >> 1;
        if(meUniSpacingMarks[mid][0] == uc)
            return meUniSpacingMarks[mid][1];
        if(meUniSpacingMarks[mid][0] < uc)
            lo = mid + 1;
        else
            hi = mid - 1;
    }
    return 0;
}

/* Returns the stand-in kind (meUNIIGN_*) of a default ignorable char, 0 if not one or a bidi
 * control */
int
meUniIgnorable(meUInt uc)
{
    int lo=0, hi=meUniTblSize(meUniIgnorables)-1, mid;

    if((uc < meUniIgnorables[0][0]) || (uc > (meUniIgnorables[hi][1] & 0xffffff)))
        return 0;
    while(lo <= hi)
    {
        mid = (lo + hi) >> 1;
        if(uc < meUniIgnorables[mid][0])
            hi = mid - 1;
        else if(uc > (meUniIgnorables[mid][1] & 0xffffff))
            lo = mid + 1;
        else
            return (int) (meUniIgnorables[mid][1] >> 24);
    }
    return 0;
}

int
meUniWidth(meUInt uc, int ambigWide)
{
    int ww;
    if((uc >= 0x20) && (uc < 0x7f))
        return 1;
    ww = meUniPropWidth(meUniProps(uc));
    if(ww == meUNIWIDTH_AMBIG)
        return (ambigWide) ? 2:1;
    return ww;
}

static meUInt
meUniCaseMap(const meUniCaseRun *runs, int count, meUInt uc)
{
    int lo=0, hi=count-1, mid;
    meUInt off;

    if(uc < runs[0].start)
        return uc;
    while(lo < hi)
    {
        mid = (lo + hi + 1) >> 1;
        if(runs[mid].start <= uc)
            lo = mid;
        else
            hi = mid - 1;
    }
    off = uc - runs[lo].start;
    if((off <= ((meUInt) (runs[lo].count - 1)) * runs[lo].stride) && ((off % runs[lo].stride) == 0))
        return uc + runs[lo].delta;
    return uc;
}

/* the BMP's lower (0), upper (1) or fold (2) mapping of uc */
#define meUniCaseBmp(uc,ii) ((uc) + meUniCaseBlocks[(meUniCaseIndex[(uc) >> 5] << 5) | ((uc) & 0x1f)][ii])

meUInt
meUniToLower(meUInt uc)
{
    if(uc < 0x80)
        return ((uc >= 'A') && (uc <= 'Z')) ? (uc + 0x20):uc;
    if(uc < 0x10000)
        return meUniCaseBmp(uc,0);
    return meUniCaseMap(meUniLowerRuns,meUniTblSize(meUniLowerRuns),uc);
}

meUInt
meUniToUpper(meUInt uc)
{
    if(uc < 0x80)
        return ((uc >= 'a') && (uc <= 'z')) ? (uc - 0x20):uc;
    if(uc < 0x10000)
        return meUniCaseBmp(uc,1);
    return meUniCaseMap(meUniUpperRuns,meUniTblSize(meUniUpperRuns),uc);
}

meUInt
meUniFold(meUInt uc)
{
    if(uc < 0x80)
        return ((uc >= 'A') && (uc <= 'Z')) ? (uc + 0x20):uc;
    if(uc < 0x10000)
        return meUniCaseBmp(uc,2);
    return meUniCaseMap(meUniFoldRuns,meUniTblSize(meUniFoldRuns),uc);
}

/* Sets cs to the chars which case fold to fc, fc included if it folds to itself, returns the
 * number (at most max) */
int
meUniFoldSources(meUInt fc, meUInt *cs, int max)
{
    const meUniCaseRun *rr=meUniFoldRuns;
    meUInt sc, off;
    int ii, nn=0;
    
    if(meUniFold(fc) == fc)
        cs[nn++] = fc;
    for(ii=meUniTblSize(meUniFoldRuns) ; (ii > 0) && (nn < max) ; ii--, rr++)
    {
        sc = fc - rr->delta;
        if((sc >= rr->start) && ((off = sc - rr->start) <= ((meUInt) (rr->count - 1)) * rr->stride) &&
           ((off % rr->stride) == 0))
            cs[nn++] = sc;
    }
    return nn;
}

const char *
meUniVersion(void)
{
    return meUNICODE_VERSION;
}

#endif /* MEOPT_UNICODE */
