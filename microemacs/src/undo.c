/* -*- C -*- ****************************************************************
 *
 *  System        : MicroEmacs Jasspa Distribution
 *  Module        : undo.c
 *  Synopsis      : ME undo support routines
 *  Created By    : Steven Phillips
 *  Created       : 1996
 *  Last Modified : <000221.0749>
 *
 *  Description
 *
 *  Notes
 *
 ****************************************************************************
 *
 * Copyright (c) 1996-2000 Steven Phillips
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

/*---	Include defintions */

#define	__UNDOC	1			/* Define the filename */

/*---	Include files */

#include "emain.h"

#if MEUNDO

UNDOND *
meUndoCreateNode(size_t size)
{
    UNDOND *nn ;

    if((nn = meMalloc(size)) != NULL)
    {
        nn->type = 0 ;
        nn->next = curbp->fUndo ;
        curbp->fUndo = nn ;
        nn->udata.dotp = curwp->line_no ;
        nn->doto  = curwp->w_doto ;
        nn->str[0] = '\0' ;
        if(curbp->undoContFlag == undoContFlag)
            nn->type |= MEUNDO_CONT ;
        else
            curbp->undoContFlag = undoContFlag ;
    }
    return nn ;
}


void
meUndoAddInsChar(void)
{
    if(meModeTest(curbp->b_mode,MDUNDO))
    {
        uint8 type=MEUNDO_SING|MEUNDO_MINS ;
        UNDOND *nn ;

        if(curbp->undoContFlag == undoContFlag)
            type |= MEUNDO_CONT ;

        if(((nn = curbp->fUndo) != NULL) && (nn->type == type) &&
           (nn->udata.dotp == curwp->line_no) &&
           (nn->doto == curwp->w_doto-1))
        {
            nn->doto++ ;
            nn->count++ ;
            curbp->undoContFlag = undoContFlag ;
        }
        else if((nn = meUndoCreateNode(sizeof(UNDOND))) != NULL)
        {
            nn->type |= MEUNDO_SING|MEUNDO_MINS ;
            nn->count = 1 ;
        }
    }
}

void
meUndoAddDelChar(void)
{
    if(meModeTest(curbp->b_mode,MDUNDO))
    {
        uint8 type=MEUNDO_SING|MEUNDO_MDEL ;
        UNDOND *nn ;
        uint8   cc ;

        if((cc = curwp->w_dotp->l_text[curwp->w_doto]) == '\0')
        {
            if((curwp->line_no == curbp->elineno-1) && (curwp->w_doto))
                /* This is trying to just remove the end line when the line
                 * before is not empty, this will fail so don't store it
                 */
                return ;
            cc = meNLCHAR ;
        }
        if(curbp->undoContFlag == undoContFlag)
            type |= MEUNDO_CONT ;

        nn = curbp->fUndo ;
        if((nn != NULL) && ((nn->type & ~MEUNDO_REVS) == type) &&
           (nn->udata.dotp == curwp->line_no))
        {
            if(!(nn->type & MEUNDO_REVS) && (nn->doto == curwp->w_doto))
                ;
            else if(((nn->doto-nn->count) == curwp->w_doto) &&
                    (((nn->type & MEUNDO_REVS) != 0) || (nn->count == 1)))
                nn->type |= MEUNDO_REVS ;
            else
                goto meUndoAddDelCharNew ;
            if(!(nn->count & 0x0f) &&
               ((nn = meRealloc(nn,sizeof(UNDOND)+nn->count+18)) == NULL))
                return ;
            curbp->fUndo = nn ;
            nn->str[nn->count++] = cc ;
            nn->str[nn->count]   = '\0' ;
            curbp->undoContFlag = undoContFlag ;
        }
        else
        {
meUndoAddDelCharNew:
            if((nn = meUndoCreateNode(sizeof(UNDOND)+18)) != NULL)
            {
                nn->type |= MEUNDO_MDEL|MEUNDO_SING ;
                nn->count = 1 ;
                nn->str[0] = cc ;
                nn->str[1] = '\0' ;
            }
        }
    }
}

void
meUndoAddRepChar(void)
{
    if(meModeTest(curbp->b_mode,MDUNDO))
    {
        uint8 type=MEUNDO_SING|MEUNDO_MINS|MEUNDO_MDEL ;
        UNDOND *nn ;
        uint8   cc ;

        if(curbp->undoContFlag == undoContFlag)
            type |= MEUNDO_CONT ;
        if((cc = curwp->w_dotp->l_text[curwp->w_doto]) == '\0')
            cc = meNLCHAR ;
        nn = curbp->fUndo ;
        if((nn != NULL) && (nn->type == type) &&
           (nn->udata.dotp == curwp->line_no) &&
           (nn->doto == curwp->w_doto-1))
        {
            if(!(nn->count & 0x0f) &&
               ((nn = meRealloc(nn,sizeof(UNDOND)+nn->count+18)) == NULL))
                return ;
            curbp->fUndo = nn ;
            nn->doto++ ;
            nn->str[nn->count++] = cc ;
            nn->str[nn->count]   = '\0' ;
            curbp->undoContFlag = undoContFlag ;
        }
        else if((nn = meUndoCreateNode(sizeof(UNDOND)+18)) != NULL)
        {
            nn->type |= MEUNDO_MDEL|MEUNDO_SING|MEUNDO_MINS ;
            nn->count = 1 ;
            nn->str[0] = cc ;
            nn->str[1] = '\0' ;
        }
    }
}

void
meUndoAddInsChars(int32 numChars)
{
    UNDOND *nn ;

    if(meModeTest(curbp->b_mode,MDUNDO) &&
       ((nn = meUndoCreateNode(sizeof(UNDOND))) != NULL))
    {
        nn->type |= MEUNDO_MINS|MEUNDO_REVS ;
        nn->count = numChars ;
    }
}

void
meUndoAddDelChars(int32 numChars)
{
    UNDOND *nn ;

    if(numChars < 0)
        numChars = 0 ;
    if(meModeTest(curbp->b_mode,MDUNDO) &&
       ((nn = meUndoCreateNode(sizeof(UNDOND)+numChars)) != NULL))
    {
        LINE   *ll = curwp->w_dotp ;
        int     len ;
        uint8  *dd=nn->str, *ss=ll->l_text+curwp->w_doto ;

        nn->type |= MEUNDO_MDEL ;
        nn->count = 0;
        if((len = llength(ll) - curwp->w_doto) < numChars)
        {
            for(;;)
            {
                numChars-=len+1 ;
                for(; len ; len--)
                    *dd++ = *ss++ ;
                ll = lforw(ll) ;
                ss = ll->l_text ;
                /* A bit of a bodge here to cope with the bogus last line
                 * If the last but 1 line's '\n' is about to be removed
                 * don't actually store the '\n' as this is automatically
                 * added by the system
                 */
                if((numChars == 0) && (ll == curbp->b_linep))
                    break ;
                *dd++ = meNLCHAR ;
                if(numChars <= (len=llength(ll)))
                    break ;
            }
        }
        for(; numChars ; numChars--)
            *dd++ = *ss++ ;
        *dd = '\0' ;
    }
}

void
meUndoAddReplaceBgn(LINE *elinep, uint16 elineo)
{
    if(meModeTest(curbp->b_mode,MDUNDO))
    {
        UNDOND *nn ;
        int     len ;

        if(elinep == curbp->b_linep)
        {
            elinep = lback(elinep) ;
            elineo = llength(elinep) ;
        }
        if(elinep == curwp->w_dotp)
            len = elineo - curwp->w_doto ;
        else
        {
            LINE *ll = curwp->w_dotp ;
            len = llength(ll) - curwp->w_doto + elineo+1 ;
            while((ll = lforw(ll)) != elinep)
                len += llength(ll)+1 ;
        }
        if((nn = meUndoCreateNode(sizeof(UNDOND)+len)) != NULL)
        {
            LINE  *ll = curwp->w_dotp ;
            uint8 *dd=nn->str, *ss=ll->l_text+curwp->w_doto ;

            nn->type |= MEUNDO_MDEL ;
            /* This should be zero because added on the end call. */
            nn->count = 0;
            if(elinep == curwp->w_dotp)
            {
                for(; len ; len--)
                    *dd++ = *ss++ ;
            }
            else
            {
                len = llength(ll) - curwp->w_doto ;
                for(; len ; len--)
                    *dd++ = *ss++ ;
                *dd++ = meNLCHAR ;
                while((ll = lforw(ll)) != elinep)
                {
                    len = llength(ll) ;
                    ss = ll->l_text ;
                    for(; len ; len--)
                        *dd++ = *ss++ ;
                    *dd++ = meNLCHAR ;
                }
                ss = ll->l_text ;
                for(; elineo ; elineo--)
                    *dd++ = *ss++ ;
            }
            *dd = '\0' ;
        }
    }
}

void
meUndoAddReplaceEnd(int32 numChars)
{
    if(meModeTest(curbp->b_mode,MDUNDO))
    {
        curbp->fUndo->count = numChars ;
        curbp->fUndo->type |= MEUNDO_MINS ;
    }
}

void
meUndoAddReplace(uint8 *dstr, int32 count)
{
    if(meModeTest(curbp->b_mode,MDUNDO))
    {
        UNDOCOORD *co ;
        UNDOND    *nn ;
        int        doto, contFlag ;

        contFlag = (curbp->undoContFlag == undoContFlag) ;

        nn = curbp->fUndo ;
        if((nn == NULL) ||
           !(nn->type & MEUNDO_REPL) ||
           (nn->count != count) ||
           (nn->doto == 0xffff) ||
           meStrcmp(nn->str,dstr))
        {
            uint8 *dd ;
            if((nn = meUndoCreateNode(sizeof(UNDOND)+meStrlen(dstr))) == NULL)
                return ;
            nn->type |= MEUNDO_MDEL|MEUNDO_MINS|MEUNDO_REPL|MEUNDO_REVS ;
            nn->udata.pos = NULL ;
            nn->doto = 0 ;
            nn->count = count ;
            dd = nn->str ;
            while((*dd++ = *dstr++))
                ;
        }
        /* replace is the same as last time */
        if(!(nn->doto & 0x0f) &&
           ((nn->udata.pos = meRealloc(nn->udata.pos,sizeof(UNDOCOORD)*
                                       (nn->doto+16))) == NULL))
            return ;
        co = nn->udata.pos ;
        doto = curwp->w_doto ;
        if(contFlag)
            /* sub 1 of so 0 becomes less than 0 */
            doto = -1 - doto ;
        co[nn->doto][0]   = curwp->line_no ;
        co[nn->doto++][1] = doto ;
        curbp->undoContFlag = undoContFlag ;
    }
}

#if NARROW
/* NOTE: Narrow undos are only added if there is already an
 * undo. This is so the state of the buffer will be correct
 * for the previous undo
 */
void
meUndoAddNarrow(int32 sln, uint16 name)
{
    UNDOND *nn ;

    if((curbp->fUndo != NULL) &&
       ((nn = meUndoCreateNode(sizeof(UNDOND))) != NULL))
    {
        nn->type |= MEUNDO_NRRW ;
        nn->udata.dotp = sln ;
        nn->doto = name ;
        nn->count = -1 ;
    }
}
void
meUndoAddUnnarrow(int32 sln, int32 eln, uint16 name)
{
    UNDOND *nn ;

    if((curbp->fUndo != NULL) &&
       ((nn = meUndoCreateNode(sizeof(UNDOND))) != NULL))
    {
        nn->type |= MEUNDO_NRRW ;
        nn->udata.dotp = sln ;
        nn->doto = name ;
        nn->count = eln ;
    }
}
#endif

void
meUndoRemove(BUFFER *bp)
{
    UNDOND *nn ;

    while((nn=bp->fUndo) != NULL)
    {
        bp->fUndo = nn->next ;
        if(nn->type & MEUNDO_REPL)
            free(nn->udata.pos) ;
        free(nn) ;
    }
}

int
meUndo(int f, int n)
{
    if(n < -1)
    {
#ifndef NDEBUG
        if(n == -4)
        {
            UNDOND *nn ;
            nn=curbp->fUndo ;
            while(nn != NULL)
            {
                printf("Undo %x %x %ld (%ld,%d) [%s]\n",nn->type,(int) nn->next,nn->count,
                       nn->udata.dotp,nn->doto,nn->str) ;
                if(nn->type & MEUNDO_REPL)
                {
                    int ii ;
                    for(ii=0 ; ii<nn->doto ; ii++)
                        printf("(%d,%d) ",nn->udata.pos[ii][0],nn->udata.pos[ii][1]) ;
                    printf("\n") ;
                }
                nn = nn->next ;
            }
        }
        else
#endif
            undoContFlag++ ;
    }
    else if(!meModeTest(curbp->b_mode,MDUNDO))
        return ctrlg(FALSE,1) ;
    else if(n < 0)
        meUndoRemove(curbp) ;
    else
    {
        static UNDOND *cun ;
        static int32  ccount ;
        static uint16 cdoto ;

        if((lastflag != CFUNDO) && ((cun = curbp->fUndo) != NULL))
        {
            cdoto = cun->doto ;
            ccount = cun->count ;
        }
        for(;;)
        {
            int count, cont ;
            if((cun == NULL) || ((n <= 0) && !meModeTest(curbp->b_mode,MDEDIT)))
                break ;
            if(bchange() != TRUE)               /* Check we can change the buffer */
                return ABORT ;
            cont=0 ;
#if NARROW
            if(cun->type & MEUNDO_NRRW)
            {
                if(ccount < 0)
                {
                    meNARROW *nrrw ;
                    nrrw = curbp->narrow ;
                    while(nrrw->name != cdoto)
                        nrrw = nrrw->next ;
                    gotoLine(TRUE,cun->udata.dotp+1) ;
                    curbp->b_dotp = curwp->w_dotp ;
                    curbp->line_no = curwp->line_no ;
                    curbp->b_doto = 0 ;
                    removeNarrow(curbp,nrrw,1) ;
                }
                else
                {
                    LINE *slp ;
                    gotoLine(TRUE,cun->udata.dotp+1) ;
                    slp = curwp->w_dotp ;
                    gotoLine(TRUE,ccount+1) ;
                    createNarrow(curbp,slp,curwp->w_dotp,cun->udata.dotp,ccount,cdoto) ;
                }
                if(cun->type & MEUNDO_CONT)
                    cont=1 ;
                goto meUndoNext ;
            }
#endif
            if(cun->type & MEUNDO_REPL)
            {
                gotoLine(TRUE,cun->udata.pos[cdoto-1][0]+1) ;
                count = cun->udata.pos[cdoto-1][1] ;
                if(count < 0)
                {
                    cont = 1 ;
                    count = -1 - count ;
                }
                curwp->w_doto = count ;
            }
            else
            {
                if(cun->type & MEUNDO_CONT)
                    cont = 1 ;
                if(cun->type & MEUNDO_FRST)
                {
                    if(!(cun->type & MEUNDO_MDEL))
                    {
                        autowriteremove(curbp) ;
                        meModeClear(curbp->b_mode,MDEDIT) ;
                        curwp->w_flag |= WFMODE ;
                    }
                    goto meUndoNext ;
                }
                gotoLine(TRUE,cun->udata.dotp+1) ;
                curwp->w_doto = cdoto ;
            }
            if(cun->type & MEUNDO_SING)
            {
                ccount-- ;
                count = 1 ;
            }
            else
                count = ccount ;
            if(cun->type & MEUNDO_REVS)
                WbackChar(curwp,ccount) ;
            if(cun->type & MEUNDO_MINS)
                ldelete(count,2) ;
            if(cun->type & MEUNDO_MDEL)
            {
                /* When dealing with long lines this loop becomes infinitly
                 * long because of the number of times that the line is
                 * re-allocated - pre-allocate the line length first. In order
                 * to reduce the processing overhead then we find the longest
                 * strings and then add them back in in one go, this ensures
                 * that we only ever re-allocate the line once. 
                 * Jon - 99/12/12.
                 */
                uint8 *ss, cc ;
                ss = cun->str ;
                /* Deal with a single character undo */
                if(cun->type & MEUNDO_SING)
                {
                    meUndoAddInsChar() ;
                    ss += ccount ;
                    if((cc = *ss++) == meNLCHAR)
                        lnewline();
                    else if (cc != '\0')
                        linsert(1, cc);
                }
                else
                {
                    /* Deal with a multiple character undo. */
                    count = 0;
                    while (*ss != '\0')
                    {
                        int ii;
                        
                        /* Find string length */
                        for (ii = 0; (((cc = ss[ii]) != '\0') && (cc != meNLCHAR)); ii++)
                            /* NULL */;
                        /* Insert the string if there is one */
                        if (ii != 0)
                            lsinsert (ii, ss);
                        /* Insert the new line if there is one */
                        if (ss[ii] == meNLCHAR)
                        {
                            lnewline();
                            ii++;
                        }
                        /* Advance */
                        ss += ii;
                        count += ii;
                    }
                    if(count)
                        meUndoAddInsChars(count) ;
                }
            }
            if((cun->type & MEUNDO_SING) && (ccount > 0))
            {
                if(cun->type & MEUNDO_MINS)
                    cdoto-- ;
            }
            else if(cun->type & MEUNDO_REPL)
            {
                cdoto-- ;
                if(!cdoto)
                    goto meUndoNext ;
            }
            else
            {
meUndoNext:
                if((cun = cun->next) != NULL)
                {
                    cdoto = cun->doto ;
                    ccount = cun->count ;
                }
            }
            if(!cont && (--n == 0))
                break ;
        }
        thisflag = CFUNDO ;
    }
    return TRUE ;
}
#endif
