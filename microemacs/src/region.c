/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * region.c - Region (dot to cursor) routines.
 *
 * Copyright (C) 1988-2024 JASSPA (www.jasspa.com)
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
 * Created:     Unknown
 * Authors:     Unknown, Jon Green & Steven Phillips
 * Description:	
 *     The routines in this file deal with the region, that magic space
 *     between "." and mark. Some functions are commands. Some functions are
 *     just for internal use.
 */

#define	__REGIONC			/* Define filename */

#include "emain.h"

/*
 * This routine figures out the
 * bounds of the region in the current window, and
 * fills in the fields of the "meRegion" structure pointed
 * to by "rp". Because the dot and mark are usually very
 * close together, we scan outward from dot looking for
 * mark. This should save time. Return a standard code.
 * Callers of this routine should be prepared to get
 * an "meABORT" status; we might make this have the
 * conform thing later.
 */
int
getregion(register meRegion *rp)
{
    meWindow *cwp=frameCur->windowCur;
    meLine *lp, *elp;
    long  size;
    
    if (cwp->markLine == NULL)
        return noMarkSet();
    if(cwp->dotLine == cwp->markLine)
    {
        rp->line = cwp->dotLine;
        if (cwp->dotOffset < cwp->markOffset) 
        {
            rp->offset = cwp->dotOffset;
            rp->size = (long)(cwp->markOffset-cwp->dotOffset);
        }
        else
        {
            rp->offset = cwp->markOffset;
            rp->size = (long)(cwp->dotOffset-cwp->markOffset);
        }
        rp->lineNo = cwp->dotLineNo;
        return meTRUE ;
    }
    if(cwp->dotLineNo < cwp->markLineNo)
    {
        elp = cwp->markLine ;
        lp = cwp->dotLine ;
        rp->lineNo = cwp->dotLineNo ;
        rp->offset = cwp->dotOffset ;
        size = cwp->dotOffset ;
        size = cwp->markOffset + meLineGetLength(lp) - size -
                  cwp->dotLineNo + cwp->markLineNo ;
    }
    else
    {
        elp = cwp->dotLine ;
        lp = cwp->markLine ;
        rp->lineNo = cwp->markLineNo ;
        rp->offset = cwp->markOffset ;
        size = cwp->markOffset ;
        size = cwp->dotOffset + meLineGetLength(lp) - size +
                  cwp->dotLineNo - cwp->markLineNo ;
    }
    rp->line = lp ;
    while((lp=meLineGetNext(lp)) != elp)
        size += meLineGetLength(lp) ;
    rp->size = size ;
    return meTRUE ;
}

/*
 * Kill the region. Ask "getregion"
 * to figure out the bounds of the region.
 * Move "." to the start, and kill the characters.
 * Bound to "C-W".
 */
int
killRegion(int f, int n)
{
    meRegion region;
    
    if(n == 0)
        return meTRUE ;
    if(getregion(&region) <= 0)
        return meFALSE ;
    if(bufferSetEdit() <= 0)               /* Check we can change the buffer */
        return meFALSE ;
    frameCur->windowCur->dotLine = region.line ;
    frameCur->windowCur->dotLineNo = region.lineNo ;
    frameCur->windowCur->dotOffset = region.offset ;   
    
    return ldelete(region.size,(n > 0) ? 3:2);
}

/*
 * Copy all of the characters in the
 * region to the kill buffer. Don't move dot
 * at all. This is a bit like a kill region followed
 * by a yank. Bound to "M-W".
 */
int	
copyRegion(int f, int n)
{
    meRegion region;
    meLine *line;
    meUByte  *ss, *dd;
    meInt left, ii, ret;
#if MEOPT_NARROW
    meWindow *cwp;
    meUShort eoffset;
    meLine *markLine, *dotLine, *elp;
    meInt markLineNo, dotLineNo, expandNarrows=meFALSE;
    meUShort markOffset, dotOffset;
#endif
    
    if(n < 0)
    {
#ifdef _CLIPBRD
        TTsetClipboard(1);        
#endif
        return meTRUE;
    }
    if(getregion(&region) <= 0)
        return meFALSE;
    left = region.size;
    line = region.line;
#if MEOPT_NARROW
    cwp = frameCur->windowCur;
    if((n & 0x01) && (cwp->buffer->narrow != NULL) &&
       (cwp->dotLine != cwp->markLine))
    {
        /* expand narrows that are within the region */
        expandNarrows = meTRUE;
        dotLine = cwp->dotLine;
        dotLineNo = cwp->dotLineNo;
        dotOffset = cwp->dotOffset;
        markLine = cwp->markLine;
        markLineNo = cwp->markLineNo;
        markOffset = cwp->markOffset;
        if(line == dotLine)
        {
            elp = markLine;
            eoffset = markOffset;
        }
        else
        {
            elp = dotLine;
            eoffset = dotOffset;
        }
        left += meBufferRegionExpandNarrow(cwp->buffer,&line,region.offset,elp,eoffset,0) ;
        if(((cwp->dotLine != dotLine) && dotOffset) ||
           ((cwp->markLine != markLine) && markOffset))
        {
            ret = mlwrite(MWABORT,(meUByte *)"[Bad region definition]");
            goto copy_region_exit;
        }
    }
#endif
    killInit(lastflag == meCFKILL);
    if(left == 0)
    {
        thisflag = meCFKILL;
        ret =  meTRUE;
        goto copy_region_exit;
    }
    
    if((dd = killAddNode(left)) == NULL)
    {
        ret = meFALSE;
        goto copy_region_exit;
    }
    if((ii = region.offset) == meLineGetLength(line))
        goto add_newline;                 /* Current offset.      */
    ss = line->text+ii;
    ii = meLineGetLength(line) - ii;
    goto copy_middle;
    while(left)
    {
        ss = line->text;
        ii = meLineGetLength(line);
copy_middle:
        /* Middle of line.      */
        if(ii > left)
        {
            ii = left;
            left = 0;
        }
        else
            left -= ii;
        while(ii--)
            *dd++ = *ss++;
add_newline:
        if(left != 0)
        {
            *dd++ = meCHAR_NL;
            line = meLineGetNext(line);
            left--;
        }
    }
    thisflag = meCFKILL;
    ret = meTRUE;

copy_region_exit:

#if MEOPT_NARROW
    if(expandNarrows)
    {
        meBufferCollapseNarrowAll(cwp->buffer);
        cwp->dotLine = dotLine;
        cwp->dotLineNo = dotLineNo;
        cwp->dotOffset = dotOffset;
        cwp->markLine = markLine;
        cwp->markLineNo = markLineNo;
        cwp->markOffset = markOffset;
    }
#endif
    return ret;
}

/*
 * Lower case region. Zap all of the upper
 * case characters in the region to lower case. Use
 * the region code to set the limits. Scan the buffer,
 * doing the changes. Call "lineSetChanged" to ensure that
 * redisplay is done in all buffers. Bound to
 * "C-X C-L".
 */
int
lowerRegion(int f, int n)
{
    meWindow *cwp;
    meLine *line;
    int loffs;
    meInt lline;
    register meUByte c;
    register int s;
    meRegion  region;
    
    if((s=getregion(&region)) <= 0)
        return (s);
    if((s=bufferSetEdit()) <= 0)               /* Check we can change the buffer */
        return s;
    cwp = frameCur->windowCur;
    line = cwp->dotLine;
    loffs = cwp->dotOffset;
    lline = cwp->dotLineNo;
    cwp->dotLine = region.line;
    cwp->dotOffset = region.offset;
    cwp->dotLineNo = region.lineNo;
    while (region.size--)
    {
        if((c = meLineGetChar(cwp->dotLine, cwp->dotOffset)) == '\0')
        {
            cwp->dotLine = meLineGetNext(cwp->dotLine);
            cwp->dotOffset = 0;
            cwp->dotLineNo++ ;
        }
        else
        {
            if(isUpper(c))
            {
                lineSetChanged(WFMAIN);
#if MEOPT_UNDO
                meUndoAddRepChar() ;
#endif
                c = toggleCase(c) ;
                meLineSetChar(cwp->dotLine, cwp->dotOffset, c);
            }
            (cwp->dotOffset)++ ;
        }
    }
    cwp->dotLine = line ;
    cwp->dotOffset = loffs ;
    cwp->dotLineNo = lline ;
    return meTRUE ;
}

/*
 * Upper case region. Zap all of the lower
 * case characters in the region to upper case. Use
 * the region code to set the limits. Scan the buffer,
 * doing the changes. Call "lineSetChanged" to ensure that
 * redisplay is done in all buffers. Bound to
 * "C-X C-U".
 */
int
upperRegion(int f, int n)
{
    meWindow *cwp;
    meLine *line;
    int loffs;
    long lline;
    register char c;
    register int s;
    meRegion region;
    
    if((s=getregion(&region)) <= 0)
        return (s);
    if((s=bufferSetEdit()) <= 0)               /* Check we can change the buffer */
        return s;
    cwp = frameCur->windowCur;
    line = cwp->dotLine ;
    loffs = cwp->dotOffset ;
    lline = cwp->dotLineNo ;
    cwp->dotLine = region.line ;
    cwp->dotOffset = region.offset ;
    cwp->dotLineNo = region.lineNo ;
    while (region.size--)
    {
        if((c = meLineGetChar(cwp->dotLine, cwp->dotOffset)) == '\0')
        {
            cwp->dotLine = meLineGetNext(cwp->dotLine);
            cwp->dotOffset = 0;
            cwp->dotLineNo++ ;
        }
        else
        {
            if(isLower(c))
            {
                lineSetChanged(WFMAIN);
#if MEOPT_UNDO
                meUndoAddRepChar() ;
#endif
                c = toggleCase(c) ;
                meLineSetChar(cwp->dotLine, cwp->dotOffset, c);
            }
            (cwp->dotOffset)++ ;
        }
    }
    cwp->dotLine = line ;
    cwp->dotOffset = loffs ;
    cwp->dotLineNo = lline ;
    return meTRUE ;
}

#if MEOPT_EXTENDED
int
killRectangle(int f, int n)
{
    meWindow *cwp=frameCur->windowCur;
    meUByte *kstr;
    meInt slno, elno, size;
    int soff, eoff, coff, llen, dotPos;
    
    if(cwp->markLine == NULL)
        return noMarkSet();
    if(cwp->dotLine == cwp->markLine)
        return killRegion(f,n);
    if(bufferSetEdit() <= 0)               /* Check we can change the buffer */
        return meFALSE;
    
    eoff = getcwcol(cwp);
    /* remember we have swapped */
    windowSwapDotAndMark(0,1);
    soff = getcwcol(cwp);
    if(soff > eoff)
    {
        llen = eoff;
        eoff = soff;
        soff = llen;
    }
    llen = eoff - soff;
    dotPos = (cwp->dotLineNo > cwp->markLineNo) ? 1:0;    
    if(dotPos)
        windowSwapDotAndMark(0,1);
    
    if(meAnchorSet(cwp->buffer,meANCHOR_ABS_LINE,cwp->dotLine,cwp->dotLineNo,cwp->dotOffset,1))
        dotPos |= 2;
            
    slno = cwp->dotLineNo;
    elno = cwp->markLineNo;
    /* calculate the maximum length */
    size = (elno - slno + 1) * (llen + 1);
    
    /* sort out the kill buffer */
    killInit((lastflag == meCFKILL) || (thisflag == meCFKILL));
    if((kstr = killAddNode(size)) == NULL)
        return meFALSE;
    thisflag = meCFKILL;
    for(;;)
    {
        meUByte *off, cc;
        int lspc=0, ii, jj, kk, ll;
        
        off = windCurLineOffsetEval(cwp);
        coff = 0;
        ii = 0;
        kk = cwp->dotLine->length;
        while(ii < kk)
        {
            if(coff == soff)
                break;
            if((coff+off[ii]) > soff)
            {
                lspc = soff - coff;
                /* as we can convert tabs to spaces, adjust the offset */
                if(meLineGetChar(cwp->dotLine,ii) == meCHAR_TAB)
                    off[ii] -= lspc;
                coff = soff;
                break;
            }
            coff += off[ii++];
        }
        jj = ii;
        if(coff < soff)
        {
            /* line too short to even get to the start point */
            lspc = soff - coff;
            memset(kstr,' ',llen);
            kstr += llen;
        }
        else
        {
            while(jj < kk)
            {
                if(coff == eoff)
                    break;
                if((ll = off[jj]) != 0)
                {
                    cc = meLineGetChar(cwp->dotLine,jj);
                    if((coff+ll) > eoff)
                    {
                        /* as we can convert tabs to spaces, delete and convert */
                        if(cc == meCHAR_TAB)
                        {
                            lspc += coff + ll - eoff;
                            jj++;
                        }
                        break;
                    }
                    coff += ll;
                    if(cc == meCHAR_TAB)
                    {
                        /* convert tabs to spaces for better column support */
                        do
                            *kstr++ = ' ';
                        while(--ll > 0);
                    }
                    else
                        *kstr++ = cc;
                }
                jj++;
            }
            if(coff < eoff)
            {
                /* line too short to get to the end point, fill with spaces */
                coff = eoff - coff;
                memset(kstr,' ',coff);
                kstr += coff;
            }
            
        }
        *kstr++ = '\n';
        cwp->dotOffset = ii;
        if((jj -= ii) > 0)
        {
#if MEOPT_UNDO
            meUndoAddDelChars(jj);
#endif
            mldelete(jj,NULL);
        }
        if(lspc)
        {
            if(lineInsertChar(lspc,' ') <= 0)
            {
                *kstr = '\0';
                return meFALSE;
            }
#if MEOPT_UNDO
            if(jj > 0)
                meUndoAddReplaceEnd(lspc);
            else
                meUndoAddInsChars(lspc);
#endif
        }
        if(cwp->dotLineNo == elno)
            break;
        /* move on to the next line */
        cwp->dotLineNo++;
        cwp->dotLine = meLineGetNext(cwp->dotLine);
        cwp->dotOffset  = 0;
    }
    *kstr = '\0';
    if(dotPos & 2)
    {
        meAnchor *p = cwp->buffer->anchorList;
        
        while(p != NULL)
        {
            if(p->name == meANCHOR_ABS_LINE)
            {
                if(dotPos & 1)
                {
                    cwp->dotLine = p->line;
                    cwp->dotOffset = p->offs;
                    cwp->dotLineNo = slno;
                }
                else
                {
                    cwp->markLine = p->line;
                    cwp->markOffset = p->offs;
                    cwp->markLineNo = slno;
                }
                break;
            }
            p = p->next;
        }
        meAnchorDelete(cwp->buffer,meANCHOR_ABS_LINE);
    }
    return meTRUE;
}

static int
yankRectangleKill(struct meKill *pklist, int soff, int notLast)
{
    meWindow *cwp=frameCur->windowCur;
    meUByte *off, *ss, *tt, *dd=NULL, cc ;
    int ii, jj, kk, lsspc, lespc, ldel, linsc, coff ;
    meKillNode *killp ;
    
    killp = pklist->kill;
    while(killp != NULL)
    {
        ss = killp->data;
        while(*ss != '\0')
        {
            tt = ss ;
            while(((cc = *ss) != '\0') && (cc != meCHAR_NL))
                ss++ ;
            ii = (size_t) ss - (size_t) tt ;
            off = windCurLineOffsetEval(cwp);
            ldel = lsspc = lespc = 0 ;
            coff = 0 ;
            jj = 0 ;
            kk = cwp->dotLine->length;
            while(jj < kk)
            {
                if(coff == soff)
                    break ;
                if((coff+off[jj]) > soff)
                {
                    /* if its a tab we can remove the tab and replace with spaces */
                    if(meLineGetChar(cwp->dotLine,jj) == meCHAR_TAB)
                    {
                        ldel = 1 ;
                        lespc = off[jj] - soff + coff ;
                    }
                    break ;
                }
                coff += off[jj++] ;
            }
            lsspc = soff - coff ;
            linsc = ii + lsspc + lespc - ldel ;
            cwp->dotOffset = jj ;
            /* Make space for the characters */
            if((dd = lineMakeSpace(linsc)) == NULL)
                return meABORT ;
            lineSetChanged(WFMOVEC|WFMAIN) ;
#if MEOPT_UNDO
            if(ldel > 0)
            {
                *dd = meCHAR_TAB ;
                cwp->dotOffset = jj ;
                meUndoAddDelChars(ldel) ;
            }
#endif
            /* add the starting spaces */
            while(--lsspc >= 0)
                *dd++ = ' ' ;
            while(--ii >= 0)
                *dd++ = *tt++ ;
            /* add the ending spaces (only if we've deleted a TAB),
             * preserve dd so the end point is correct */
            tt = dd ;
            while(--lespc >= 0)
                *tt++ = ' ' ;
#if MEOPT_UNDO
            if(ldel > 0)
            {
                cwp->dotOffset += linsc+ldel ;
                meUndoAddReplaceEnd(linsc+ldel) ;
            }
            else
                meUndoAddInsChars(linsc) ;
#endif
            if(*ss == meCHAR_NL)
                ss++ ;
            cwp->dotLineNo++ ;
            cwp->dotLine  = meLineGetNext(cwp->dotLine);
            cwp->dotOffset  = 0;
        }
        killp = killp->next;
    }
    if((dd != NULL) && !notLast)
    {
        cwp->dotLineNo--;
        cwp->dotLine = meLineGetPrev(cwp->dotLine);
        cwp->dotOffset = (meUShort) ((size_t) dd - (size_t) meLineGetText(cwp->dotLine));
    }
    return meTRUE ;
}

int
yankRectangle(int f, int n)
{
    int col;
#ifdef _CLIPBRD
    TTgetClipboard(0);
#endif
    /* make sure there is something to yank */
    if(klhead == NULL)
        return mlwrite(MWABORT,(meUByte *)"[nothing to yank]");
    /* Check we can change the buffer */
    if(bufferSetEdit() <= 0)
        return meABORT;
    
    /* get the current column */
    col = getcwcol(frameCur->windowCur);
    
    /* place the mark on the current line */
    windowSetMark(meFALSE, meFALSE);
    
    /* for each time.... */
    while(--n >= 0)
        if(yankRectangleKill(klhead,col,n) <= 0)
            return meABORT;
    /* update the current window's line etc, other windows seem to be updated okay as this does not change their current line */
    frameCur->windowCur->updateFlags |= WFMOVEL|WFMAIN|WFSBOX;
    return meTRUE;
}
#endif

