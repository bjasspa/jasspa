/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * region.c - Region (dot to cursor) routines.
 *
 * Copyright (C) 1988-2002 JASSPA (www.jasspa.com)
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
 * fills in the fields of the "REGION" structure pointed
 * to by "rp". Because the dot and mark are usually very
 * close together, we scan outward from dot looking for
 * mark. This should save time. Return a standard code.
 * Callers of this routine should be prepared to get
 * an "ABORT" status; we might make this have the
 * conform thing later.
 */
int
getregion(register REGION *rp)
{
    LINE *lp, *elp;
    long  size;
    
    if (curwp->w_markp == NULL)
        return noMarkSet() ;
    if(curwp->w_dotp == curwp->w_markp)
    {
        rp->linep = curwp->w_dotp;
        if (curwp->w_doto < curwp->w_marko) 
        {
            rp->offset = curwp->w_doto;
            rp->size = (long)(curwp->w_marko-curwp->w_doto);
        }
        else
        {
            rp->offset = curwp->w_marko;
            rp->size = (long)(curwp->w_doto-curwp->w_marko);
        }
        rp->line_no = curwp->line_no;
        return TRUE ;
    }
    if(curwp->line_no < curwp->mlineno)
    {
        elp = curwp->w_markp ;
        lp = curwp->w_dotp ;
        rp->line_no = curwp->line_no ;
        rp->offset = curwp->w_doto ;
        size = curwp->w_doto ;
        size = curwp->w_marko + llength(lp) - size -
                  curwp->line_no + curwp->mlineno ;
    }
    else
    {
        elp = curwp->w_dotp ;
        lp = curwp->w_markp ;
        rp->line_no = curwp->mlineno ;
        rp->offset = curwp->w_marko ;
        size = curwp->w_marko ;
        size = curwp->w_doto + llength(lp) - size +
                  curwp->line_no - curwp->mlineno ;
    }
    rp->linep = lp ;
    while((lp=lforw(lp)) != elp)
        size += llength(lp) ;
    rp->size = size ;
    return TRUE ;
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
    REGION region;
    
    if(n == 0)
        return TRUE ;
    if(getregion(&region) != TRUE)
        return FALSE ;
    if(bchange() != TRUE)               /* Check we can change the buffer */
        return FALSE ;
    curwp->w_dotp   = region.linep;
    curwp->w_doto   = region.offset;   
    curwp->line_no  = region.line_no;
    
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
    LINE   *linep;
    int     left, ii ;
    meUByte  *ss, *dd ;
    REGION  region ;
    
    if((ii=getregion(&region)) != TRUE)
        return ii ;
    if(lastflag != CFKILL)                /* Kill type command.   */
        ksave();
    if((left=region.size) == 0)
    {
        thisflag = CFKILL ;
        return TRUE ;
    }
    
    if((dd = kaddblock(left)) == NULL)
        return FALSE ;
    
    linep = region.linep;                 /* Current line.        */
    if((ii = region.offset) == llength(linep))
        goto add_newline ;                 /* Current offset.      */
    ss = linep->l_text+ii ;
    ii = llength(linep) - ii ;
    goto copy_middle ;
    while(left)
    {
        ss = linep->l_text ;
        ii = llength(linep) ;
copy_middle:
        /* Middle of line.      */
        if(ii > left)
        {
            ii = left ;
            left = 0 ;
        }
        else
            left -= ii ;
        while(ii--)
            *dd++ = *ss++ ;
add_newline:
        if(left != 0)
        {
            *dd++ = meNLCHAR ;
            linep = lforw(linep);
            left-- ;
        }
    }
    thisflag = CFKILL ;
    return TRUE ;
}

/*
 * Lower case region. Zap all of the upper
 * case characters in the region to lower case. Use
 * the region code to set the limits. Scan the buffer,
 * doing the changes. Call "lchange" to ensure that
 * redisplay is done in all buffers. Bound to
 * "C-X C-L".
 */
int
lowerRegion(int f, int n)
{
    LINE *linep ;
    int   loffs ;
    meInt lline ;
    register meUByte c;
    register int   s;
    REGION  region;
    
    if((s=getregion(&region)) != TRUE)
        return (s);
    if((s=bchange()) != TRUE)               /* Check we can change the buffer */
        return s ;
    linep = curwp->w_dotp ;
    loffs = curwp->w_doto ;
    lline = curwp->line_no ;
    curwp->w_dotp = region.linep ;
    curwp->w_doto = region.offset ;
    curwp->line_no = region.line_no ;
    while (region.size--)
    {
        if((c = lgetc(curwp->w_dotp, curwp->w_doto)) == '\0')
        {
            curwp->w_dotp = lforw(curwp->w_dotp);
            curwp->w_doto = 0;
            curwp->line_no++ ;
        }
        else
        {
            if(isUpper(c))
            {
                lchange(WFMAIN);
#if MEUNDO
                meUndoAddRepChar() ;
#endif
                c = toggleCase(c) ;
                lputc(curwp->w_dotp, curwp->w_doto, c);
            }
            (curwp->w_doto)++ ;
        }
    }
    curwp->w_dotp = linep ;
    curwp->w_doto = loffs ;
    curwp->line_no = lline ;
    return TRUE ;
}

/*
 * Upper case region. Zap all of the lower
 * case characters in the region to upper case. Use
 * the region code to set the limits. Scan the buffer,
 * doing the changes. Call "lchange" to ensure that
 * redisplay is done in all buffers. Bound to
 * "C-X C-U".
 */
int
upperRegion(int f, int n)
{
    LINE *linep ;
    int   loffs ;
    long  lline ;
    register char  c;
    register int   s;
    REGION         region;
    
    if((s=getregion(&region)) != TRUE)
        return (s);
    if((s=bchange()) != TRUE)               /* Check we can change the buffer */
        return s ;
    linep = curwp->w_dotp ;
    loffs = curwp->w_doto ;
    lline = curwp->line_no ;
    curwp->w_dotp = region.linep ;
    curwp->w_doto = region.offset ;
    curwp->line_no = region.line_no ;
    while (region.size--)
    {
        if((c = lgetc(curwp->w_dotp, curwp->w_doto)) == '\0')
        {
            curwp->w_dotp = lforw(curwp->w_dotp);
            curwp->w_doto = 0;
            curwp->line_no++ ;
        }
        else
        {
            if(isLower(c))
            {
                lchange(WFMAIN);
#if MEUNDO
                meUndoAddRepChar() ;
#endif
                c = toggleCase(c) ;
                lputc(curwp->w_dotp, curwp->w_doto, c);
            }
            (curwp->w_doto)++ ;
        }
    }
    curwp->w_dotp = linep ;
    curwp->w_doto = loffs ;
    curwp->line_no = lline ;
    return TRUE ;
}

int
killRectangle(int f, int n)
{
    meUByte *kstr ;
    meInt slno, elno, size ;
    int soff, eoff, coff, llen, dotPos ;
    
    if (curwp->w_markp == NULL)
        return noMarkSet() ;
    if(curwp->w_dotp == curwp->w_markp)
        return killRegion(f,n) ;
    if(bchange() != TRUE)               /* Check we can change the buffer */
        return FALSE ;
    
    eoff = getcwcol() ;
    /* remember we have swapped */
    swapmark(0,1) ;
    soff = getcwcol() ;
    if(soff > eoff)
    {
        llen = eoff ;
        eoff = soff ;
        soff = llen ;
    }
    llen = eoff - soff ;
    if((dotPos=curwp->line_no > curwp->mlineno))
        swapmark(0,1) ;
    slno = curwp->line_no ;
    elno = curwp->mlineno ;
    /* calculate the maximum length */
    size = (elno - slno + 1) * (llen + 1) ;
    
    /* sort out the kill buffer */
    if((lastflag != CFKILL) && (thisflag != CFKILL))
        ksave();
    if((kstr = kaddblock(size)) == NULL)
        return FALSE ;
    thisflag = CFKILL;
    for(;;)
    {
        meUByte *off, cc ;
        int lspc=0, ii, jj, kk, ll ;
        
        windCurLineOffsetEval(curwp) ;
        off = curwp->curLineOff->l_text ;
        coff = 0 ;
        ii = 0 ;
        kk = curwp->w_dotp->l_used ;
        while(ii < kk)
        {
            if(coff == soff)
                break ;
            if((coff+off[ii]) > soff)
            {
                lspc = soff - coff ;
                /* as we can convert tabs to spaces, adjust the offset */
                if(lgetc(curwp->w_dotp,ii) == TAB)
                    off[ii] -= lspc ;
                coff = soff ;
                break ;
            }
            coff += off[ii++] ;
        }
        jj = ii ;
        if(coff < soff)
        {
            /* line too short to even get to the start point */
            lspc = soff - coff ;
            memset(kstr,' ',llen) ;
            kstr += llen ;
        }
        else
        {
            while(jj < kk)
            {
                if(coff == eoff)
                    break ;
                ll = off[jj] ;
                cc = lgetc(curwp->w_dotp,jj) ;
                if((coff+ll) > eoff)
                {
                    /* as we can convert tabs to spaces, delete and convert */
                    if(cc == TAB)
                    {
                        lspc += coff + ll - eoff ;
                        jj++ ;
                    }
                    break ;
                }
                coff += ll ;
                if(cc == TAB)
                {
                    /* convert tabs to spaces for better column support */
                    do
                        *kstr++ = ' ' ;
                    while(--ll > 0) ;
                }
                else
                    *kstr++ = cc ;
                jj++ ;
            }
            if(coff < eoff)
            {
                /* line too short to get to the end point, fill with spaces */
                coff = eoff - coff ;
                memset(kstr,' ',coff) ;
                kstr += coff ;
            }
            
        }
        *kstr++ = '\n' ;
        curwp->w_doto = ii ;
        if((jj -= ii) > 0)
        {
#if MEUNDO
            meUndoAddDelChars(jj) ;
#endif
            mldelete(jj,NULL) ;
        }
        if(lspc)
        {
            if(linsert(lspc,' ') != TRUE)
            {
                *kstr = '\0' ;
                return FALSE ;
            }
#if MEUNDO
            if(jj > 0)
                meUndoAddReplaceEnd(lspc) ;
            else
                meUndoAddInsChars(lspc) ;
#endif
        }
        if(curwp->line_no == elno)
            break ;
        /* move on to the next line */
        curwp->line_no++ ;
        curwp->w_dotp  = lforw(curwp->w_dotp);
        curwp->w_doto  = 0;
    }
    *kstr = '\0' ;
    if(dotPos)
        swapmark(0,1) ;
    return TRUE ;
}

static int
yankRectangleKill(struct KLIST *pklist, int soff, int notLast)
{
    meUByte *off, *ss, *tt, *dd=NULL, cc ;
    int ii, jj, kk, lsspc, lespc, ldel, linsc, coff ;
    KILL *killp ;
    
    killp = pklist->kill ;
    while (killp != NULL)
    {
        ss = killp->data ;
        while(*ss != '\0')
        {
            tt = ss ;
            while(((cc = *ss) != '\0') && (cc != meNLCHAR))
                ss++ ;
            ii = (size_t) ss - (size_t) tt ;
            windCurLineOffsetEval(curwp) ;
            off = curwp->curLineOff->l_text ;
            ldel = lsspc = lespc = 0 ;
            coff = 0 ;
            jj = 0 ;
            kk = curwp->w_dotp->l_used ;
            while(jj < kk)
            {
                if(coff == soff)
                    break ;
                if((coff+off[jj]) > soff)
                {
                    /* if its a tab we can remove the tab and replace with spaces */
                    if(lgetc(curwp->w_dotp,jj) == TAB)
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
            curwp->w_doto = jj ;
            /* Make space for the characters */
            if((dd = lmakespace(linsc)) == NULL)
                return ABORT ;
            lchange(WFMOVEC|WFMAIN) ;
#if MEUNDO
            if(ldel > 0)
            {
                *dd = TAB ;
                curwp->w_doto = jj ;
                meUndoAddDelChars(ldel) ;
            }
#endif
            /* add the starting spaces */
            while(--lsspc >= 0)
                *dd++ = ' ' ;
            while(--ii >= 0)
                *dd++ = *tt++ ;
            /* add the ending spaces (only if we've deleted a TAB) */
            while(--lespc >= 0)
                *dd++ = ' ' ;
#if MEUNDO
            if(ldel > 0)
                meUndoAddReplaceEnd(linsc+ldel) ;
            else
                meUndoAddInsChars(linsc) ;
#endif
            if(*ss == meNLCHAR)
                ss++ ;
            curwp->line_no++ ;
            curwp->w_dotp  = lforw(curwp->w_dotp);
            curwp->w_doto  = 0;
        }
        killp = killp->next;
    }
    if((dd != NULL) && !notLast)
    {
        curwp->line_no-- ;
        curwp->w_dotp = lback(curwp->w_dotp);
        curwp->w_doto = ((size_t) dd - (size_t) ltext(curwp->w_dotp)) ;
    }
    return TRUE ;
}

int
yankRectangle(int f, int n)
{
    int col ;
#ifdef _CLIPBRD
    if(clipState & CLIP_TRY_GET)
        TTgetClipboard() ;
#endif
    /* make sure there is something to yank */
    if(klhead == NULL)
        return mlwrite(MWABORT,(meUByte *)"[nothing to yank]");
    /* Check we can change the buffer */
    if(bchange() != TRUE)
        return ABORT ;
    
    /* get the current column */
    col = getcwcol() ;
    
    /* place the mark on the current line */
    setMark(FALSE, FALSE);
    
    /* for each time.... */
    while(--n >= 0)
        if(yankRectangleKill(klhead,col,n) != TRUE)
            return ABORT ;
    return TRUE ;
}
