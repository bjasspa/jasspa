/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * narrow.c - Narrow out regions of a buffer.
 *
 * Copyright (C) 1999-2001 Steven Phillips
 * Copyright (C) 2002 JASSPA (www.jasspa.com)
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
 * Created:     01/01/99
 * Synopsis:    Narrow out regions of a buffer.
 * Authors:     Steven Phillips
 * Description:
 *     Narrow can narrow out or narrow to a region of text, a buffer can have
 *     multiple narrows which can be removed one at a time or all at once.
 *     goto-line can go to the absolute line (i.e. ignore narrows) or with
 *     the narrowed out lines.
 *
 * Notes:
 *     A narrow drops an alpha mark on the next line, this line can be moved
 *     around and the alpha mark may or may not move with it depending on the
 *     may its moved. This can lead to confusion and a mixed up file ordering.
 */

#define	__NARROWC				/* Define filename */

/*---	Include files */
#include "emain.h"

#if MEOPT_NARROW

void
createNarrow(meBuffer *bp, meLine *slp, meLine *elp, meInt sln, meInt eln, meUShort name)
{
    meWindow   *wp ;
    meNarrow *nrrw, *cn, *pn ;
    meInt     nln ;
    
    if((nrrw = meMalloc(sizeof(meNarrow))) == NULL)
        return ;
    
    pn = NULL ;
    cn = bp->narrow ;
    if(!name)
    {
        if(cn != NULL)
            name = cn->name+1 ;
        else
            name = meAM_FRSTNRRW ;
    }
    while((cn != NULL) && (cn->name > name))
    {
        pn = cn ;
        cn = cn->next ;
    }
    meAssert((cn == NULL) || (cn->name != name)) ;
    nrrw->name = name ;
    if((nrrw->next = cn) != NULL)
        cn->prev = nrrw ;
    if((nrrw->prev = pn) != NULL)
        pn->next = nrrw ;
    else
        bp->narrow = nrrw ;
    if(alphaMarkSet(bp,name,elp,0,1) != meTRUE)
    {
        meFree(nrrw) ;
        return ;
    }
    nln = eln - sln ;
    nrrw->slp = slp ;
    nrrw->elp = meLineGetPrev(elp) ;
    nrrw->sln = sln ;
    nrrw->noLines = nln ;
    slp->prev->next = elp ;
    elp->prev = slp->prev ;
    bp->lineCount -= nln ;
    meModeSet(bp->mode,MDNRRW) ;
#if MEOPT_UNDO
    meUndoAddNarrow(sln,name) ;
#endif
    meFrameLoopBegin() ;
    for (wp=loopFrame->windowList; wp!=NULL; wp=wp->next)
    {
        if (wp->buffer == bp)
        {
            if(wp->dotLineNo >= sln)
            {
                if(wp->dotLineNo >= eln)
                {
                    if ((wp->vertScroll -= nln) < 0)
                        wp->vertScroll = 0;
                    wp->dotLineNo -= nln ;
                }
                else
                {
                    wp->dotLine  = elp ;
                    wp->dotOffset  = 0 ;
                    wp->dotLineNo = sln ;
                }
            }
            if(wp->markLineNo >= sln)
            {
                if(wp->markLineNo >= eln)
                    wp->markLineNo -= nln ;
                else
                {
                    wp->markLine = elp ;
                    wp->markOffset = 0 ;
                    wp->markLineNo = sln ;
                }
            }
            wp->flag |= WFMAIN|WFMOVEL|WFSBOX ;
        }
    }
    meFrameLoopEnd() ;
}


static void
singleUnnarrow(meBuffer *bp, register meNarrow *nrrw, int useDot)
{
    meNarrow *nw ;
    meWindow   *wp ;
    meLine *lp1, *lp2 ;
    
    if(!useDot)
        alphaMarkGet(bp,nrrw->name) ;

    /* Note that bp->dotLineNo is used outside this function
     * to setup the undo
     */
    lp2 = bp->dotLine ;
    lp1 = meLineGetPrev(lp2) ;
    nrrw->slp->prev = lp1 ;
    lp1->next = nrrw->slp ;
    nrrw->elp->next = lp2 ;
    lp2->prev = nrrw->elp ;
    bp->lineCount += nrrw->noLines ;
    /* The next bit is a bit grotty, a fudge and doesn't work properly
     * in everycase, but is better than nothing.
     * 
     * If there are other narrows check to see if they are on the same
     * line, if so, use the start-line (sln) to decide if they should
     * go before or after this narrow, and do the right thing
     */
    nw = nrrw->next ;
    while(nw != NULL)
    {
        meAMark *p = bp->amarkList;
        while(p->name != nw->name)
            p = p->next ;
        if((p->line == lp2) &&
           (nw->sln < nrrw->sln))
        {
            /* Got one, set the narrow line to be the first line in this narrow */
            p->line = nrrw->slp ;
            p->line->flag |= meLINE_AMARK ;		/* mark the line as marked */
        }
        nw = nw->next ;
    }        
    meFrameLoopBegin() ;
    for (wp=loopFrame->windowList; wp!=NULL; wp=wp->next)
    {
        if (wp->buffer == bp)
        {
            if(wp->dotLineNo >= bp->dotLineNo)
            {
                wp->vertScroll += nrrw->noLines ;
                wp->dotLineNo += nrrw->noLines ;
            }
            if(wp->markLineNo >= bp->dotLineNo)
                wp->markLineNo += nrrw->noLines ;
            wp->flag |= WFMAIN|WFMOVEL|WFSBOX ;
        }
    }
    meFrameLoopEnd() ;
}

void
removeNarrow(meBuffer *bp, register meNarrow *nrrw, int useDot)
{
    meNarrow *nn ;
    
    singleUnnarrow(bp,nrrw,useDot) ;
#if MEOPT_UNDO
    meUndoAddUnnarrow(bp->dotLineNo,bp->dotLineNo+nrrw->noLines,nrrw->name) ;
#endif
    nn = frameCur->bufferCur->narrow ;
    while(nn != nrrw)
    {
        if(nn->sln > nrrw->sln)
            nn->sln += nrrw->noLines ;
        nn = nn->next ;
    }
    if(nrrw->next != NULL)
        nrrw->next->prev = nrrw->prev ;
    if(nrrw->prev != NULL)
        nrrw->prev->next = nrrw->next ;
    else if((bp->narrow = nrrw->next) == NULL)
        meModeClear(bp->mode,MDNRRW) ;
    meFree(nrrw) ;
}

void
unnarrowBuffer(meBuffer *bp)
{
    meNarrow *nrrw ;
    
    nrrw = bp->narrow ;
    while(nrrw != NULL)
    {
        singleUnnarrow(bp,nrrw,0) ;
        nrrw = nrrw->next ;
    }
}

void
delSingleNarrow(meBuffer *bp, int useDot)
{
    meNarrow *nrrw, *nn ;
    
    if((nrrw = bp->narrow) != NULL)
    {
        singleUnnarrow(bp,nrrw,useDot) ;
        if((nn = nrrw->next) == NULL)
        {
            bp->narrow = NULL ;
            meModeClear(bp->mode,MDNRRW) ;
        }
        else
        {
            bp->narrow = nn ;
            nn->prev = NULL ;
        }
        meFree(nrrw) ;
    }
}

void
redoNarrowInfo(meBuffer *bp)
{
    meNarrow *nrrw ;
    meWindow   *wp ;
    
    /* redoing the narrows must be done in the right order,
     * i.e. last in the list first! So go to the last and
     * then use the prev pointers to move back.
     */
    if((nrrw = bp->narrow) == NULL)
        return ;
    
    while(nrrw->next != NULL)
        nrrw = nrrw->next ;
    do
    {
        alphaMarkGet(bp,nrrw->name) ;
        nrrw->slp->prev->next = nrrw->elp->next ;
        nrrw->elp->next->prev = nrrw->slp->prev ;
        bp->lineCount -= nrrw->noLines ;
        meFrameLoopBegin() ;
        for (wp=loopFrame->windowList; wp!=NULL; wp=wp->next)
        {
            if (wp->buffer == bp)
            {
                if(wp->dotLineNo >= bp->dotLineNo)
                {
                    wp->vertScroll -= nrrw->noLines ;
                    wp->dotLineNo -= nrrw->noLines ;
                }
                if(wp->markLineNo >= bp->dotLineNo)
                    wp->markLineNo -= nrrw->noLines ;
                wp->flag |= WFMAIN|WFMOVEL ;
            }
        }
        meFrameLoopEnd() ;
        nrrw = nrrw->prev ;
    } while(nrrw != NULL) ;
}

int
narrowBuffer(int f, int n)
{
    if(n == 1)
    {
        meNarrow *nrrw, *nn ;
        
        if((nrrw=frameCur->bufferCur->narrow) == NULL)
            return mlwrite(MWABORT|MWCLEXEC,(meUByte *)"[Current buffer not narrowed]") ;
    
        while(nrrw != NULL)
        {
            nn = nrrw->next ;
            singleUnnarrow(frameCur->bufferCur,nrrw,0) ;
#if MEOPT_UNDO
            meUndoAddUnnarrow(frameCur->bufferCur->dotLineNo,frameCur->bufferCur->dotLineNo+nrrw->noLines,nrrw->name) ;
#endif
            meFree(nrrw) ;
            nrrw = nn ;
        }
        meModeClear(frameCur->bufferCur->mode,MDNRRW) ;
        frameCur->bufferCur->narrow = NULL ;
    }
    else if(n == 2)
    {
        if(frameCur->windowCur->dotLine->flag & meLINE_AMARK)
        {
            meNarrow *nrrw ;
            meAMark  *am ;
            
            nrrw = frameCur->bufferCur->narrow ;
            while(nrrw != NULL)
            {
                am = frameCur->bufferCur->amarkList;
                while(am != NULL)
                {
                    if((nrrw->name == am->name) &&
                       (am->line == frameCur->windowCur->dotLine))
                    {
                        removeNarrow(frameCur->bufferCur,nrrw,0) ;
                        return meTRUE ;
                    }
                    am = am->next;
                }
                nrrw = nrrw->next ;
            }
        }
        return mlwrite(MWABORT|MWCLEXEC,(meUByte *)"[No narrow on current line]") ;
    }
    else
    {
        meLine   *slp, *elp ;
        meInt   sln,  eln ;
        if(frameCur->windowCur->markLine == NULL)
            return noMarkSet() ;
        if(frameCur->windowCur->dotLineNo == frameCur->windowCur->markLineNo)
            return mlwrite(MWABORT,(meUByte *)"[Illegal narrow]") ;
        if(frameCur->windowCur->dotLineNo < frameCur->windowCur->markLineNo)
        {
            slp = frameCur->windowCur->dotLine ;
            elp = frameCur->windowCur->markLine ;
            sln = frameCur->windowCur->dotLineNo ;
            eln = frameCur->windowCur->markLineNo ;
        }
        else
        {
            slp = frameCur->windowCur->markLine ;
            elp = frameCur->windowCur->dotLine ;
            sln = frameCur->windowCur->markLineNo ;
            eln = frameCur->windowCur->dotLineNo ;
        }
        if(n == 4)
            createNarrow(frameCur->bufferCur,slp,elp,sln,eln,0) ;
        else if(n == 3)
        {
            if(elp != frameCur->bufferCur->baseLine)
                createNarrow(frameCur->bufferCur,elp,frameCur->bufferCur->baseLine,eln,frameCur->bufferCur->lineCount,0) ;
            if(sln != 0)
                createNarrow(frameCur->bufferCur,meLineGetNext(frameCur->bufferCur->baseLine),slp,0,sln,0) ;
        }
        else
            return mlwrite(MWABORT,(meUByte *)"[Illegal narrow argument]") ;
    }
    return meTRUE ;
}
#endif
