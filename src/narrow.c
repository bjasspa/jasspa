/* -*- C -*- ****************************************************************
 *
 *  System        : MicroEmacs Jasspa Distribution
 *  Module        : narrow.c
 *  Synopsis      : Narrow out regions of a buffer
 *  Created By    : Steven Phillips
 *  Created       : 01/01/99
 *  Last Modified : <010915.2102>
 *
 *  Description
 *     Narrow can narrow out or narrow to a region of text, a buffer can have
 *     multiple narrows which can be removed one at a time or all at once.
 *     goto-line can go to the absolute line (i.e. ignore narrows) or with
 *     the narrowed out lines.
 *
 *  Notes
 *     A narrow drops an alpha mark on the next line, this line can be moved
 *     around and the alpha mark may or may not move with it depending on the
 *     may its moved. This can lead to confusion and a mixed up file ordering.
 * 
 ****************************************************************************
 * 
 * Copyright (c) 1999-2000 Steven Phillips    
 *    
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the  authors be held liable for any damages  arising  from
 * the use of this software.
 *     
 * This software was generated as part of the MicroEmacs JASSPA  distribution,
 * (http://www.jasspa.com) but is excluded from those licensing restrictions.
 *
 * Permission  is  granted  to anyone to use this  software  for any  purpose,
 * including  commercial  applications,  and to alter it and  redistribute  it
 * freely, subject to the following restrictions:
 *
 *  1. The origin of this software must not be misrepresented; you must not
 *     claim that you wrote the original software. If you use this software
 *     in a product, an acknowledgment in the product documentation would be
 *     appreciated but is not required.
 *  2. Altered source versions must be plainly marked as such, and must not be
 *     misrepresented as being the original software.
 *  3. This notice may not be removed or altered from any source distribution.
 *
 * Steven Phillips         bill@jasspa.com
 *
 ****************************************************************************/

#define	__NARROWC				/* Define filename */

/*---	Include files */
#include "emain.h"

#if NARROW

void
createNarrow(BUFFER *bp, LINE *slp, LINE *elp, int32 sln, int32 eln, uint16 name)
{
    WINDOW   *wp ;
    meNARROW *nrrw, *cn, *pn ;
    int32     nln ;
    
    if((nrrw = meMalloc(sizeof(meNARROW))) == NULL)
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
    if(alphaMarkSet(bp,name,elp,0,1) != TRUE)
    {
        meFree(nrrw) ;
        return ;
    }
    nln = eln - sln ;
    nrrw->slp = slp ;
    nrrw->elp = lback(elp) ;
    nrrw->sln = sln ;
    nrrw->noLines = nln ;
    slp->l_bp->l_fp = elp ;
    elp->l_bp = slp->l_bp ;
    bp->elineno -= nln ;
    meModeSet(bp->b_mode,MDNRRW) ;
#if MEUNDO
    meUndoAddNarrow(sln,name) ;
#endif
    for (wp=wheadp; wp!=NULL; wp=wp->w_wndp)
        if (wp->w_bufp == bp)
        {
            if(wp->line_no >= sln)
            {
                if(wp->line_no >= eln)
                {
                    if ((wp->topLineNo -= nln) < 0)
                        wp->topLineNo = 0;
                    wp->line_no -= nln ;
                }
                else
                {
                    wp->w_dotp  = elp ;
                    wp->w_doto  = 0 ;
                    wp->line_no = sln ;
                }
            }
            if(wp->mlineno >= sln)
            {
                if(wp->mlineno >= eln)
                    wp->mlineno -= nln ;
                else
                {
                    wp->w_markp = elp ;
                    wp->w_marko = 0 ;
                    wp->mlineno = sln ;
                }
            }
            wp->w_flag |= WFMAIN|WFMOVEL|WFSBOX ;
        }
}


static void
singleUnnarrow(BUFFER *bp, register meNARROW *nrrw, int useDot)
{
    meNARROW *nw ;
    WINDOW   *wp ;
    LINE *lp1, *lp2 ;
    
    if(!useDot)
        alphaMarkGet(bp,nrrw->name) ;

    /* Note that bp->line_no is used outside this function
     * to setup the undo
     */
    lp2 = bp->b_dotp ;
    lp1 = lback(lp2) ;
    nrrw->slp->l_bp = lp1 ;
    lp1->l_fp = nrrw->slp ;
    nrrw->elp->l_fp = lp2 ;
    lp2->l_bp = nrrw->elp ;
    bp->elineno += nrrw->noLines ;
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
        meAMARK *p = bp->b_amark;
        while(p->name != nw->name)
            p = p->next ;
        if((p->line == lp2) &&
           (nw->sln < nrrw->sln))
        {
            /* Got one, set the narrow line to be the first line in this narrow */
            p->line = nrrw->slp ;
            p->line->l_flag |= LNMARK ;		/* mark the line as marked */
        }
        nw = nw->next ;
    }        
    for (wp=wheadp; wp!=NULL; wp=wp->w_wndp)
        if (wp->w_bufp == bp)
        {
            if(wp->line_no >= bp->line_no)
            {
                wp->topLineNo += nrrw->noLines ;
                wp->line_no += nrrw->noLines ;
            }
            if(wp->mlineno >= bp->line_no)
                wp->mlineno += nrrw->noLines ;
            wp->w_flag |= WFMAIN|WFMOVEL|WFSBOX ;
        }
}

void
removeNarrow(BUFFER *bp, register meNARROW *nrrw, int useDot)
{
    meNARROW *nn ;
    
    singleUnnarrow(bp,nrrw,useDot) ;
#if MEUNDO
    meUndoAddUnnarrow(bp->line_no,bp->line_no+nrrw->noLines,nrrw->name) ;
#endif
    nn = curbp->narrow ;
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
        meModeClear(bp->b_mode,MDNRRW) ;
    meFree(nrrw) ;
}

void
unnarrowBuffer(BUFFER *bp)
{
    meNARROW *nrrw ;
    
    nrrw = bp->narrow ;
    while(nrrw != NULL)
    {
        singleUnnarrow(bp,nrrw,0) ;
        nrrw = nrrw->next ;
    }
}

void
delSingleNarrow(BUFFER *bp, int useDot)
{
    meNARROW *nrrw, *nn ;
    
    if((nrrw = bp->narrow) != NULL)
    {
        singleUnnarrow(bp,nrrw,useDot) ;
        if((nn = nrrw->next) == NULL)
        {
            bp->narrow = NULL ;
            meModeClear(bp->b_mode,MDNRRW) ;
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
redoNarrowInfo(BUFFER *bp)
{
    meNARROW *nrrw ;
    WINDOW   *wp ;
    
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
        nrrw->slp->l_bp->l_fp = nrrw->elp->l_fp ;
        nrrw->elp->l_fp->l_bp = nrrw->slp->l_bp ;
        bp->elineno -= nrrw->noLines ;
        for (wp=wheadp; wp!=NULL; wp=wp->w_wndp)
            if (wp->w_bufp == bp)
            {
                if(wp->line_no >= bp->line_no)
                {
                    wp->topLineNo -= nrrw->noLines ;
                    wp->line_no -= nrrw->noLines ;
                }
                if(wp->mlineno >= bp->line_no)
                    wp->mlineno -= nrrw->noLines ;
                wp->w_flag |= WFMAIN|WFMOVEL ;
            }
        nrrw = nrrw->prev ;
    } while(nrrw != NULL) ;
}

int
narrowBuffer(int f, int n)
{
    if(n == 1)
    {
        meNARROW *nrrw, *nn ;
        
        if((nrrw=curbp->narrow) == NULL)
            return mlwrite(MWABORT|MWCLEXEC,(uint8 *)"[Current buffer not narrowed]") ;
    
        while(nrrw != NULL)
        {
            nn = nrrw->next ;
            singleUnnarrow(curbp,nrrw,0) ;
#if MEUNDO
            meUndoAddUnnarrow(curbp->line_no,curbp->line_no+nrrw->noLines,nrrw->name) ;
#endif
            meFree(nrrw) ;
            nrrw = nn ;
        }
        meModeClear(curbp->b_mode,MDNRRW) ;
        curbp->narrow = NULL ;
    }
    else if(n == 2)
    {
        if(curwp->w_dotp->l_flag & LNMARK)
        {
            meNARROW *nrrw ;
            meAMARK  *am ;
            
            nrrw = curbp->narrow ;
            while(nrrw != NULL)
            {
                am = curbp->b_amark;
                while(am != NULL)
                {
                    if((nrrw->name == am->name) &&
                       (am->line == curwp->w_dotp))
                    {
                        removeNarrow(curbp,nrrw,0) ;
                        return TRUE ;
                    }
                    am = am->next;
                }
                nrrw = nrrw->next ;
            }
        }
        return mlwrite(MWABORT|MWCLEXEC,(uint8 *)"[No narrow on current line]") ;
    }
    else
    {
        LINE   *slp, *elp ;
        int32   sln,  eln ;
        if(curwp->w_markp == NULL)
            return noMarkSet() ;
        if(curwp->line_no == curwp->mlineno)
            return mlwrite(MWABORT,(uint8 *)"[Illegal narrow]") ;
        if(curwp->line_no < curwp->mlineno)
        {
            slp = curwp->w_dotp ;
            elp = curwp->w_markp ;
            sln = curwp->line_no ;
            eln = curwp->mlineno ;
        }
        else
        {
            slp = curwp->w_markp ;
            elp = curwp->w_dotp ;
            sln = curwp->mlineno ;
            eln = curwp->line_no ;
        }
        if(n == 4)
            createNarrow(curbp,slp,elp,sln,eln,0) ;
        else if(n == 3)
        {
            if(elp != curbp->b_linep)
                createNarrow(curbp,elp,curbp->b_linep,eln,curbp->elineno,0) ;
            if(sln != 0)
                createNarrow(curbp,lforw(curbp->b_linep),slp,0,sln,0) ;
        }
        else
            return mlwrite(MWABORT,(uint8 *)"[Illegal narrow argument]") ;
    }
    return TRUE ;
}
#endif
