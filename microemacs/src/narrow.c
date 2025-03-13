/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * narrow.c - Narrow out regions of a buffer.
 *
 * Copyright (C) 1999-2001 Steven Phillips
 * Copyright (C) 2002-2024 JASSPA (www.jasspa.com)
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

meNarrow *
meBufferCreateNarrow(meBuffer *bp, meLine *slp, meLine *elp, meInt sln, meInt eln, 
                     meInt name, meScheme scheme, meUByte *markupLine, meInt markupCmd, meInt undoCall)
{
    meWindow *wp;
    meNarrow *nrrw, *cn, *pn;
    meInt nln;
    
    pn = NULL;
    cn = bp->narrow;
    if((name & meANCHOR_NARROW) == 0)
    {
        if(cn != NULL)
            name |= (cn->name & meNARROW_NUMBER_MASK) + 1;
        name |= meANCHOR_NARROW;
    }
    if((nrrw = meMalloc(sizeof(meNarrow))) == NULL)
        return NULL;
    
    while((cn != NULL) && (cn->name > name))
    {
        pn = cn;
        cn = cn->next;
    }
    meAssert((cn == NULL) || (cn->name != name));
    if((nrrw->next = cn) != NULL)
        cn->prev = nrrw;
    if((nrrw->prev = pn) != NULL)
        pn->next = nrrw;
    else
        bp->narrow = nrrw;
    nln = eln - sln;
    nrrw->name = name;
    nrrw->noLines = nln;
    if(markupCmd && (!undoCall || (markupLine != NULL)))
    {
        bp->dotLine = slp;
        bp->dotOffset = 0;
        bp->dotLineNo = sln;
        if(markupLine == NULL)
        {
            execBufferFunc(bp,markupCmd,(meEBF_ARG_GIVEN|meEBF_USE_B_DOT|meEBF_HIDDEN),
                           meNarrowNameGetMarkupArg(name));
            markupLine = resultStr;
        }
        if(addLine(elp,markupLine) > 0)
        {
            elp = elp->prev;
            elp->flag |= meLINE_MARKUP|meLINE_PROTECT;
            nln--;
        }
        else
            markupCmd = 0;
    }
    nrrw->markupCmd = markupCmd;
    if(meAnchorSet(bp,name,elp,sln,(meUShort) (((name & meNARROW_TYPE_MASK) == meNARROW_TYPE_TO_BOTTOM) ? 1:0),1) <= 0)
    {
        meFree(nrrw);
        return NULL;
    }
    if(scheme != meSCHEME_INVALID)
    {
        register int ii;
                
        for(ii=1 ; ii<meLINE_SCHEME_MAX ; ii++)
            if(bp->lscheme[ii] == scheme)
                break;
        if(ii == meLINE_SCHEME_MAX)
        {
            if((ii=bp->lschemeNext+1) == meLINE_SCHEME_MAX)
                ii = 1;
            bp->lscheme[ii] = scheme;
            bp->lschemeNext = ii;
        }
        elp->flag = (elp->flag & ~meLINE_SCHEME_MASK) | (ii << meLINE_SCHEME_SHIFT) | meLINE_CHANGED;
        nrrw->scheme = ii;
    }
    else
        nrrw->scheme = 0;
    nrrw->slp = slp;
    nrrw->elp = meLineGetPrev(elp);
    nrrw->sln = sln;
    nrrw->expanded = meFALSE;
    slp->prev->next = elp;
    elp->prev = slp->prev;
    bp->lineCount -= nln;
    meModeSet(bp->mode,MDNARROW);
#if MEOPT_UNDO
    meUndoAddNarrow(sln,name,markupCmd,nrrw->slp);
#endif
    meFrameLoopBegin();
    for (wp=loopFrame->windowList; wp!=NULL; wp=wp->next)
    {
        if (wp->buffer == bp)
        {
            if(wp->dotLineNo >= sln)
            {
                if(wp->dotLineNo >= eln)
                {
                    if((wp->vertScroll -= nln) < 0)
                        wp->vertScroll = 0;
                    wp->dotLineNo -= nln;
                }
                else
                {
                    wp->dotLine = elp;
                    wp->dotOffset = 0;
                    wp->dotLineNo = sln;
                }
            }
            if(wp->markLineNo >= sln)
            {
                if(wp->markLineNo >= eln)
                    wp->markLineNo -= nln;
                else
                {
                    wp->markLine = elp;
                    wp->markOffset = 0;
                    wp->markLineNo = sln;
                }
            }
            wp->updateFlags |= WFMAIN|WFMOVEL|WFSBOX|WFLOOKBK;
        }
    }
    meFrameLoopEnd();
    return nrrw;
}


static void
meBufferExpandNarrow(meBuffer *bp, register meNarrow *nrrw, meUByte *firstLine, meInt undoCall)
{
    meWindow *wp;
    meLine *lp1, *lp2;
    meInt noLines, markup;
    
    meAssert(nrrw->expanded == meFALSE);

    if(!undoCall)
        meAnchorGet(bp,nrrw->name);

    /* Note that bp->dotLineNo is used outside this function to setup the undo */
    noLines = nrrw->noLines;
    lp2 = bp->dotLine;
    lp1 = meLineGetPrev(lp2);
    /* if the narrow had a markup line then remove it */
    markup = (nrrw->markupCmd) && (lp2 != bp->baseLine) && (undoCall || (lp2->flag & meLINE_MARKUP));
    if(markup)
    {
        nrrw->markupLine = lp2;
        lp2 = meLineGetNext(lp2);
        noLines--;
    }
    else
        nrrw->markupLine = NULL;
    
    nrrw->slp->prev = lp1;
    lp1->next = nrrw->slp;
    nrrw->elp->next = lp2;
    lp2->prev = nrrw->elp;
    bp->lineCount += noLines;
    nrrw->expanded = meTRUE;
    
    meFrameLoopBegin();
    for (wp=loopFrame->windowList; wp!=NULL; wp=wp->next)
    {
        if (wp->buffer == bp)
        {
            if(wp->dotLineNo >= bp->dotLineNo)
            {
                if(markup && (wp->dotLineNo == bp->dotLineNo))
                {
                    /* the markup line has been removed, move to the next line */
                    wp->dotLine = lp2;
                    wp->dotOffset = 0;
                    wp->dotLineNo++;
                }
                wp->dotLineNo += noLines;
            }
            if(wp->markLineNo >= bp->dotLineNo)
            {
                if(markup && (wp->markLineNo == bp->dotLineNo))
                {
                    wp->markLine = lp2;
                    wp->markOffset = 0;
                    wp->markLineNo++;
                }
                wp->markLineNo += noLines;
            }
            wp->updateFlags |= WFMAIN|WFMOVEL|WFSBOX|WFLOOKBK;
        }
    }
    meFrameLoopEnd();
    if(markup)
    {
        if((firstLine == NULL) && (nrrw->markupCmd > 0))
        {
            meStrcpy(resultStr,meLineGetText(nrrw->markupLine));
            bp->dotLine = nrrw->slp;
            noLines = bp->dotLineNo;
            if(execBufferFunc(bp,nrrw->markupCmd,(meEBF_ARG_GIVEN|meEBF_USE_B_DOT|meEBF_HIDDEN),
                              0-meNarrowNameGetMarkupArg(nrrw->name)) > 0)
                firstLine = resultStr;
            else
                firstLine = NULL;
            bp->dotLine = nrrw->slp;
            bp->dotLineNo = noLines;
        }
        if(firstLine != NULL)
        {
            int len;
            lp1 = nrrw->slp;
            len = meStrlen(firstLine);
            if((len > meLineGetMaxLength(lp1)) &&
               (addLine(lp1,firstLine) > 0))
            {
                /* existing line was to short, so created a replacement line.
                 * must now remove the old line. Anchors are updated later and
                 * no window can be using this line as a dot or mark */
                lp2 = meLineGetPrev(lp1);
                lp2->next = meLineGetNext(lp1);
                meLineGetNext(lp1)->prev = lp2;
                nrrw->slp = lp2;
            }
            else
            {
                meStrcpy(meLineGetText(lp1),firstLine);
                lp1->length = len;
                lp1->unused = meLineGetMaxLength(lp1) - len;
                lp2 = lp1;
            }
            if(lp1->flag & meLINE_ANCHOR)
                /* rest anchors to the start of this new line */
                meLineResetAnchors(meLINEANCHOR_ALWAYS|meLINEANCHOR_FIXED,bp,lp1,lp2,0,0);
            if(lp1 != lp2)
                meFree(lp1);
        }
    }
}

void
meBufferRemoveNarrow(meBuffer *bp, register meNarrow *nrrw, meUByte *firstLine, meInt undoCall)
{
    meNarrow *nn;
    meLine *lp;
    
    meBufferExpandNarrow(bp,nrrw,firstLine,undoCall);
#if MEOPT_UNDO
    {
        meScheme scheme;
        if(nrrw->scheme)
            scheme = bp->lscheme[nrrw->scheme];
        else
            scheme = meSCHEME_INVALID;
        meUndoAddUnnarrow(bp->dotLineNo,bp->dotLineNo+nrrw->noLines,
                          nrrw->name,scheme,nrrw->markupCmd,nrrw->markupLine);
    }
#endif
    if(nrrw->next != NULL)
        nrrw->next->prev = nrrw->prev;
    if(nrrw->prev != NULL)
        nrrw->prev->next = nrrw->next;
    else if((bp->narrow = nrrw->next) == NULL)
        meModeClear(bp->mode,MDNARROW);
    
    /* delete the narrow anchor */
    meAnchorDelete(bp,nrrw->name);
    
    if(nrrw->markupLine != NULL)
    {
        if(nrrw->markupLine->flag & meLINE_ANCHOR)
            /* move any anchors on the markup line to the start of the first narrowed line */
            meLineResetAnchors(meLINEANCHOR_ALWAYS|meLINEANCHOR_FIXED,bp,nrrw->markupLine,
                               meLineGetNext(meLineGetPrev(nrrw->markupLine)),0,0);
        meFree(nrrw->markupLine);
    }
    else if(nrrw->scheme && 
            (((lp = bp->dotLine)->flag & meLINE_SCHEME_MASK) == (nrrw->scheme << meLINE_SCHEME_SHIFT)))
    {
        /* the narrow had a scheme, remove it */
        lp->flag &= ~meLINE_SCHEME_MASK;
        lp->flag |= meLINE_CHANGED;
        if(lp->flag & meLINE_ANCHOR_NARROW)
        {
            /* there is another narrow on this line, find it */
            meAnchor *aa=bp->anchorList;
            while(aa != NULL)
            {
                if((meAnchorGetLine(aa) == lp) && (meAnchorGetType(aa) == meANCHOR_NARROW))
                {
                    /* found the narrow anchor, now find the narrow */
                    nn = bp->narrow;
                    while(nn != NULL)
                    {
                        if(nn->name != aa->name)
                        {
                            /* got the narrow, has this got a scheme? */
                            if(nn->scheme)
                                /* use this narrows scheme */
                                lp->flag |= nn->scheme << meLINE_SCHEME_SHIFT;
                            break;
                        }
                        nn = nn->next;
                    }
                    break;
                }
                aa = meAnchorGetNext(aa);
            }
        }
    }
    if(!undoCall && ((nrrw->name & meNARROW_TYPE_MASK) != meNARROW_TYPE_OUT))
    {
        nn = frameCur->windowCur->buffer->narrow;
        while(nn != NULL)
        {
            if((nn->name & meNARROW_NUMBER_MASK) == (nrrw->name & meNARROW_NUMBER_MASK))
            {
                meBufferRemoveNarrow(bp,nn,NULL,0);
                break;
            }
            nn = nn->next;
        }
    }
    meFree(nrrw);
}

static meNarrow *
meLineGetNarrow(meBuffer *bp, meLine *lp)
{
    meBuffer *cbp=frameCur->windowCur->buffer;
    meNarrow *nrrw;
    meAnchor *aa;
    
    nrrw = cbp->narrow;
    while(nrrw != NULL)
    {
        if(!nrrw->expanded)
        {
            aa = cbp->anchorList;
            while(aa != NULL)
            {
                if((nrrw->name == aa->name) && (aa->line == lp))
                    return nrrw;
                aa = aa->next;
            }
        }
        nrrw = nrrw->next;
    }
    return NULL;
}

void
meBufferExpandNarrowAll(meBuffer *bp)
{
    meNarrow *nrrw;
    
    nrrw = bp->narrow;
    while(nrrw != NULL)
    {
        meBufferExpandNarrow(bp,nrrw,NULL,0);
        nrrw = nrrw->next;
    }
}

void
meBufferCollapseNarrowAll(meBuffer *bp)
{
    meNarrow *nrrw;
    meWindow *wp;
    meInt noLines;
    
    /* redoing the narrows must be done in the right order,
     * i.e. last in the list first! So go to the last and
     * then use the prev pointers to move back.
     */
    if((nrrw = bp->narrow) == NULL)
        return;
    
    while(nrrw->next != NULL)
        nrrw = nrrw->next;
    
    do
    {
        if(nrrw->expanded)
        {
            noLines = nrrw->noLines;
            if(nrrw->markupLine != NULL)
            {
                meAssert(meLineGetPrev(nrrw->markupLine) == nrrw->slp->prev);
                meAssert(meLineGetNext(nrrw->markupLine) == nrrw->elp->next);
                meLineGetNext(meLineGetPrev(nrrw->markupLine)) = nrrw->markupLine;
                meLineGetPrev(meLineGetNext(nrrw->markupLine)) = nrrw->markupLine;
                noLines--;
            }
            else
            {
                nrrw->slp->prev->next = nrrw->elp->next;
                nrrw->elp->next->prev = nrrw->slp->prev;
            }
            bp->lineCount -= noLines;
            nrrw->expanded = meFALSE;
            meAnchorGet(bp,nrrw->name);
            meFrameLoopBegin();
            for (wp=loopFrame->windowList; wp!=NULL; wp=wp->next)
            {
                if (wp->buffer == bp)
                {
                    if(wp->dotLineNo >= bp->dotLineNo)
                        wp->dotLineNo -= noLines;
                    if(wp->markLineNo >= bp->dotLineNo)
                        wp->markLineNo -= noLines;
                    wp->updateFlags |= WFMAIN|WFMOVEL;
                }
            }
            meFrameLoopEnd();
        }
        nrrw = nrrw->prev;
    } while(nrrw != NULL);
}

meInt
meBufferRegionExpandNarrow(meBuffer *bp, meLine **startLine, meUShort soffset, meLine *endLine, meUShort eoffset, meInt remove)
{
    /* the kill probably includes one or more narrows, find them, expand then and
     * add them to the size of the kill */
    meLine *slp, *plp, *clp, *nlp, *elp;
    meNarrow *nrrw;
    meInt len, adjLen=0;
    
    clp = *startLine;
    if(clp == endLine)
        return 0;
    slp = meLineGetPrev(clp);
    
    /* if at the start of the line and this is a narrow markup line then this narrow should be included */
    if((soffset == 0) && (clp != bp->baseLine) &&
       ((meLineGetFlag(clp) & (meLINE_ANCHOR_NARROW|meLINE_MARKUP|meLINE_PROTECT)) == (meLINE_ANCHOR_NARROW|meLINE_MARKUP|meLINE_PROTECT)) &&
       ((nrrw = meLineGetNarrow(bp,clp)) != NULL) && nrrw->markupCmd)
    {
        plp = slp;
        elp = meLineGetNext(clp);
        adjLen -= meLineGetLength(clp) + 1;
        if(remove)
            meBufferRemoveNarrow(bp,nrrw,NULL,0);
        else
            meBufferExpandNarrow(bp,nrrw,NULL,0);
        meAssert(meLineGetPrev(elp) != clp) ; /* check it was a markup */
        nlp = meLineGetNext(plp);
    }
    else
    {
        plp = clp;
        nlp = meLineGetNext(clp);
        elp = NULL;
    }
    if(eoffset)
        endLine = meLineGetNext(endLine);
    while((clp = nlp) != endLine)
    {
        if(elp == clp)
            /* reached the end of the narrowed out section */
            elp = NULL;
        nlp = meLineGetNext(clp);
        len = meLineGetLength(clp);
        if((meLineGetFlag(clp) & meLINE_ANCHOR_NARROW) &&
           ((nrrw = meLineGetNarrow(bp,clp)) != NULL))
        {
            if(remove)
                meBufferRemoveNarrow(bp,nrrw,NULL,0);
            else
                meBufferExpandNarrow(bp,nrrw,NULL,0);
            if(meLineGetPrev(nlp) == clp)
                /* the current line was not  a narrow markup line */
                nlp = clp;
            else if(elp == NULL)
                /* the current line was a narrow markup line that was
                 * counted as part of the original kill, subtract the
                 * length of this from the kill */
                adjLen -= len + 1;
            if(elp == NULL)
                elp = nlp;
            nlp = meLineGetNext(plp);
        }
        else
        {
            if(elp != NULL)
                /* currently trundling down a narrowed out section add to adjLen */
                adjLen += len + 1;
            plp = clp;
        }
    }
    *startLine = meLineGetNext(slp);
    return adjLen;
}

int
narrowBuffer(int f, int n)
{
    meWindow *cwp=frameCur->windowCur;
    meBuffer *cbp=cwp->buffer;
    meUByte buf[meBUF_SIZE_MAX];
    meUByte *ml1, ml1b[meBUF_SIZE_MAX], *ml2, ml2b[meBUF_SIZE_MAX];
    meInt markupCmd, sln, eln;
    meLine *slp, *elp;
    meScheme scheme;
    meNarrow *nrrw;
    
    f = n & 0xfff0;
    n &= 0xf;
    
    if(n == 1)
    {
        if(cbp->narrow == NULL)
            return mlwrite(MWABORT|MWCLEXEC,(meUByte *)"[Current buffer not narrowed]");
    
        while(cbp->narrow != NULL)
            meBufferRemoveNarrow(cbp,cbp->narrow,NULL,0);
        return meTRUE;
    }
    else if(n == 2)
    {
        if((cwp->dotLine->flag & meLINE_ANCHOR_NARROW) &&
           ((nrrw=meLineGetNarrow(cbp,cwp->dotLine)) != NULL))
        {
            meBufferRemoveNarrow(cbp,nrrw,NULL,0);
            return meTRUE;
        }
        return mlwrite(MWABORT|MWCLEXEC,(meUByte *)"[No narrow on current line]");
    }
    else if((n != 3) && (n != 4))
        return mlwrite(MWABORT,(meUByte *)"[Illegal narrow argument]");
    
    if(cwp->markLine == NULL)
        return noMarkSet();
    if(cwp->dotLineNo == cwp->markLineNo)
        return mlwrite(MWABORT,(meUByte *)"[Illegal narrow]");
    
    if(f & 0x10)
    {
        if(meGetString((meUByte *)"Line Scheme",0,0,buf,meBUF_SIZE_MAX) <= 0)
            return meABORT;
        scheme = convertUserScheme(meAtoi(buf),cbp->scheme);
    }
    else
        scheme = meSCHEME_INVALID;
    
    if(f & 0x20)
    {
        if(meGetString((meUByte *)"Markup Line",0,0,ml1b,meBUF_SIZE_MAX) <= 0)
            return meABORT;
        if((n == 3) &&
           (meGetString((meUByte *)"Markup Line",0,0,ml2b,meBUF_SIZE_MAX) <= 0))
            return meABORT;
        ml1 = ml1b;
        ml2 = ml2b;
        markupCmd = -1;
    }
    else
    {
        ml1 = NULL;
        ml2 = NULL;
        markupCmd = 0;
    }        
    if(f & 0x40)
    {
        if((meGetString((meUByte *)"Markup Command", MLCOMMAND,2,buf,meBUF_SIZE_MAX) <= 0) ||
           ((markupCmd = decode_fncname(buf,0)) < 0))
            return meABORT;
    }
    if(cwp->dotLineNo < cwp->markLineNo)
    {
        slp = cwp->dotLine;
        elp = cwp->markLine;
        sln = cwp->dotLineNo;
        eln = cwp->markLineNo;
    }
    else
    {
        slp = cwp->markLine;
        elp = cwp->dotLine;
        sln = cwp->markLineNo;
        eln = cwp->dotLineNo;
    }
    if(n == 3)
    {
        /* meNARROW_TYPE_TO_TOP must be greater than meNARROW_TYPE_TO_BOTTOM
         * to ensure the top narrow is first in the list en therefore
         * expanded first if all lines in between a delete. This ensures the
         * top stays at the top */
        n = meNARROW_TYPE_TO_TOP;
        if(elp != cbp->baseLine)
        {
            if((nrrw=meBufferCreateNarrow(cbp,elp,cbp->baseLine,eln,
                                          cbp->lineCount,meNARROW_TYPE_TO_BOTTOM,scheme,ml2,markupCmd,0)) == NULL)
                return meFALSE;
            n = meANCHOR_NARROW | meNARROW_TYPE_TO_TOP | (nrrw->name & meNARROW_NUMBER_MASK);
        }
        else
            nrrw = NULL;
        if(sln != 0)
            nrrw = meBufferCreateNarrow(cbp,meLineGetNext(cbp->baseLine),
                                        slp,0,sln,n,scheme,ml1,markupCmd,0);
    }
    else
        nrrw = meBufferCreateNarrow(cbp,slp,elp,sln,eln,meNARROW_TYPE_OUT,scheme,ml1,markupCmd,0);
    return (nrrw == NULL) ? meFALSE:meTRUE;
}
#endif
