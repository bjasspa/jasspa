/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * line.c - Line handling operations.
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
 * Synopsis:    Line handling operations.
 * Authors:     Unknown, Jon Green & Steven Phillips
 * Description:
 *     The functions in this file are a general set of line management
 *     utilities. They are the only routines that touch the text. They also
 *     touch the buffer and window structures, to make sure that the
 *     necessary updating gets done. There are routines in this file that
 *     handle the kill buffer too. It isn't here for any good reason.
 *
 * Notes:
 *     This code only updates the dot and mark values in the window list.
 *     Since all the code acts on the current window, the buffer that we are
 *     editing must be being displayed, which means that "windowCount" is non zero,
 *     which means that the dot and mark values in the buffer headers are
 *     nonsense.
 */

#define	__LINEC				/* Define filename */

#include "emain.h"
#include "efunc.h"

#if (defined _UNIX) || (defined _DOS) || (defined _WIN32)
#include <sys/types.h>
#include <time.h>
#ifndef _WIN32
#include <sys/time.h>
#endif
#endif

static meKillNode **currkill=NULL ;	/* current kill buffer */
static meKill *reyankLastYank=NULL;

void
lunmarkBuffer(meBuffer *bp, meLine *lp, meLine *nlp)
{
    /*
     * The line "lp" is marked by an alphabetic mark. Look through
     * the linked list of marks for the one that references this
     * line and update to the new line
     */
    register meAMark* p=bp->amarkList;

    while(p != NULL)
    {
        if(p->line == lp)
            p->line = nlp ;
        p = p->next;
    }
    /* also swap the flag */
    nlp->flag |= meLINE_AMARK ;
    lp->flag &= ~meLINE_AMARK ;
}


/****************************************************************************
 * This routine allocates a block of memory large enough to hold a meLine
 * containing "used" characters. The block is always rounded up a bit. Return
 * a pointer to the new block, or NULL if there isn't any memory left. Print a
 * message in the message line if no space.
 ******************************************************************************/

meLine *
lalloc(register int used)
{
    register meLine   *lp;
    register int    size;

    size = (used+meLINE_BLOCK_SIZE-1) & ~(meLINE_BLOCK_SIZE-1);
    if (size == 0)                          /* Assume that an empty */
        size = meLINE_BLOCK_SIZE;           /* line is for type-in. */
    if ((lp = (meLine *) meMalloc(sizeof(meLine)+size)) == NULL)
        return (NULL);
    lp->size = size;
    lp->length = used;
    lp->text[used] = '\0' ;
    lp->flag = meLINE_CHANGED ;		    /* line is not marked.	*/

    return (lp);
}

/*
 * This routine gets called when a character is changed in place in the current
 * buffer. It updates all of the required flags in the buffer and window
 * system. The flag used is passed as an argument; if the buffer is being
 * displayed in more than 1 window we change EDIT t HARD. Set MODE if the
 * mode line needs to be updated (the "*" has to be set).
 */
int
bchange(void)
{
    if(meModeTest(frameCur->bufferCur->mode,MDVIEW))
        return rdonly() ;
    if(!meModeTest(frameCur->bufferCur->mode,MDEDIT))      /* First change, so     */
    {
        if((frameCur->bufferCur->name[0] != '*') &&
           (bufferOutOfDate(frameCur->bufferCur)) &&
           (mlyesno((meUByte *)"File changed on disk, really edit") != meTRUE))
            return ctrlg(meTRUE,1) ;
        meModeSet(frameCur->bufferCur->mode,MDEDIT);
        frameAddModeToWindows(WFMODE) ;
#if MEOPT_UNDO
        if(meModeTest(frameCur->bufferCur->mode,MDUNDO))
        {
            meUndoNode        *uu ;

            uu = meUndoCreateNode(sizeof(meUndoNode)) ;
            uu->type |= meUNDO_FRST ;
            /* Add and narrows, must get the right order */
        }
#endif
    }
    if((autotime > 0) && (frameCur->bufferCur->autotime < 0) &&
       meModeTest(frameCur->bufferCur->mode,MDATSV) &&
       (frameCur->bufferCur->name[0] != '*'))
    {
        struct   meTimeval tp ;

        gettimeofday(&tp,NULL) ;
        frameCur->bufferCur->autotime = ((tp.tv_sec-startTime+autotime)*1000) +
                  (tp.tv_usec/1000) ;
        if(!isTimerExpired(AUTOS_TIMER_ID) &&
           (!isTimerSet(AUTOS_TIMER_ID) || 
            (meTimerTime[AUTOS_TIMER_ID] > frameCur->bufferCur->autotime)))
            timerSet(AUTOS_TIMER_ID,frameCur->bufferCur->autotime,((long)autotime) * 1000L) ;
    }
    return meTRUE ;
}

void
lchange(register int flag)
{
    frameCur->windowCur->dotLine->flag |= meLINE_CHANGED ;

    if (frameCur->bufferCur->windowCount != 1)             /* If more than 1 window, update rest */
    {
        register meWindow *wp;

        meFrameLoopBegin() ;
        wp = loopFrame->windowList;
        while (wp != NULL)
        {
            if(wp->buffer == frameCur->bufferCur)
                wp->updateFlags |= flag;
            wp = wp->next;
        }
        meFrameLoopEnd() ;
    }
    else
        frameCur->windowCur->updateFlags |= flag;              /* update current window */
}

/*
 * Insert "n" characters (not written) at the current location of dot. In
 * the easy case all that happens is the text is stored in the line. In the
 * hard case, the line has to be reallocated. When the window list is updated,
 * take special care; I screwed it up once. You always update dot in the
 * current window. You update mark, and a dot in another window, if it is
 * greater than the place where you did the insert. Return line pointer if all
 * is well, and NULL on errors.
 */
meUByte *
lmakespace(int n)
{
    meUByte  *cp1;
    meWindow *wp;
    meLine   *lp_new;
    meLine   *lp_old;
    meUShort  doto ;
    int     newl ;

    lp_old = frameCur->windowCur->dotLine;			/* Current line         */
    if((newl=(n+((int) meLineGetLength(lp_old)))) > 0x0fff0)
    {
        mlwrite(MWABORT|MWPAUSE,(meUByte *)"[Line too long!]") ;
        return NULL ;
    }
    if (lp_old == frameCur->bufferCur->baseLine)		/* At the end: special  */
    {
        /*---	Allocate a new line */

        if ((lp_new=lalloc(n)) == NULL)    /* Allocate new line    */
            return (NULL);

        /*---	Link in with the previous line */

#if MEOPT_UNDO
        meUndoAddInsChar() ;
#endif
        frameCur->bufferCur->lineCount++ ;
        lp_new->next = lp_old;
        lp_new->prev = lp_old->prev;
        lp_new->flag = lp_old->flag|meLINE_CHANGED ;
        /* only the new line should have any $line-scheme */
        lp_old->flag &= ~meLINE_SCHEME_MASK ;
        lp_old->prev->next = lp_new;
        lp_old->prev = lp_new;

        /* keep alpha marks on the top line - important for narrows */
        if(lp_old->flag & meLINE_AMARK)
            lunmarkBuffer(frameCur->bufferCur,lp_old,lp_new) ;
        cp1 = lp_new->text ;	/* Return pointer */
        doto = 0 ;
    }
    else
    {
        doto = frameCur->windowCur->dotOffset;           /* Save for later.      */
        if (newl > lp_old->size)      /* Hard: reallocate     */
        {
            meUByte *cp2 ;
            meUShort ii ;

            /*---	Allocate new line of correct length */

            if ((lp_new = lalloc (newl)) == NULL)
                return (NULL);

            /*---	Copy line */

            cp1 = &lp_old->text[0];
            cp2 = &lp_new->text[0];
            for (ii=doto ; ii>0 ; ii--)
                *cp2++ = *cp1++ ;
            cp2 += n;
            while((*cp2++ = *cp1++))
                ;

            /*---	Re-link in the new line */

            lp_new->prev = lp_old->prev;
            lp_new->next = lp_old->next;
            /* preserve the $line-scheme */
            lp_new->flag = lp_old->flag|meLINE_CHANGED ;
            lp_old->prev->next = lp_new;
            lp_old->next->prev = lp_new;
            if(lp_old->flag & meLINE_AMARK)
                lunmarkBuffer(frameCur->bufferCur,lp_old,lp_new);
            meFree(lp_old);
            cp1 = lp_new->text + doto ;		/* Save position */
        }
        else                                /* Easy: in place       */
        {
            meUByte *cp2 ;
            meUShort ii ;

            lp_new = lp_old;                      /* Pretend new line     */
            lp_new->length = newl ;
            lp_new->flag |= meLINE_CHANGED ;
            cp2 = lp_old->text + lp_old->length ;
            cp1 = cp2 - n ;
            ii  = cp1 - lp_old->text - doto ;
            do
                *cp2-- = *cp1-- ;
            while(ii--) ;
            cp1++ ;
        }
    }

    meFrameLoopBegin() ;
    wp = loopFrame->windowList;                    /* Update windows       */
    while (wp != NULL)
    {
        if(wp->buffer == frameCur->bufferCur)
        {
            if (wp->dotLine == lp_old)
            {
                wp->dotLine = lp_new;
                if((wp == frameCur->windowCur) || (wp->dotOffset > doto))
                    wp->dotOffset += n;
            }
            if (wp->markLine == lp_old)
            {
                wp->markLine = lp_new;
                if (wp->markOffset > doto)
                    wp->markOffset += n;
            }
        }
        wp = wp->next;
    }
    meFrameLoopEnd() ;
    return cp1 ;
}

/*
 * Insert "n" copies of the character "c" at the current location of dot. In
 * the easy case all that happens is the text is stored in the line. In the
 * hard case, the line has to be reallocated. When the window list is updated,
 * take special care; I screwed it up once. You always update dot in the
 * current window. You update mark, and a dot in another window, if it is
 * greater than the place where you did the insert. Return meTRUE if all is
 * well, and meFALSE on errors.
 */
int
linsert(int n, int c)
{
    meUByte *cp;

    lchange(WFMOVEC|WFMAIN);		/* Declare editied buffer */
    if ((cp = lmakespace(n))==NULL)     /* Make space for the characters */
        return (meFALSE);		        /* Error !!! */
    while (n-- > 0)                     /* Copy in the characters */
        *cp++ = c;
    return (meTRUE);
}

/*---	As insrt char, insert string */
int
lsinsert(register int n, register meUByte *cp)
{
    meUByte *lp ;

    if(n == 0)
    {
        for( ; cp[n] != 0 ; n++)
            ;
        if(n == 0)
            return meTRUE ;
    }
    lchange(WFMAIN) ;
    if ((lp = lmakespace(n))==NULL)	/* Make space for the characters */
        return (meFALSE);		/* Error !!! */
    while(n-- > 0)			/* Copy in the characters */
        *lp++ = *cp++;
    return meTRUE ;
}

int
insSpace(int f, int n)	/* insert spaces forward into text */
{
    register int s ;
    
    if(n <= 0)
        return meTRUE ;
    if((s=bchange()) != meTRUE)               /* Check we can change the buffer */
        return s ;
    s = linsert(n, ' ');
#if MEOPT_UNDO
    meUndoAddInsChars(n) ;
#endif
    return s ;
}

int
insTab(int f, int n)	/* insert spaces forward into text */
{
    register int s ;

    if(n <= 0)
        return meTRUE ;
    if((s=bchange()) != meTRUE)               /* Check we can change the buffer */
        return s ;
    s = linsert(n, '\t');
#if MEOPT_UNDO
    meUndoAddInsChars(n) ;
#endif
    return s ;
}

/*
 * Insert a newline into the buffer at the current location of dot in the
 * current window. The funny ass-backwards way it does things is not a botch;
 * it just makes the last line in the file not a special case. Return meTRUE if
 * everything works out and meFALSE on error (memory allocation failure). The
 * update of dot and mark is a bit easier then in the above case, because the
 * split forces more updating.
 */
int
lnewline(void)
{
    register meLine   *lp1;
    register meLine   *lp2;
    register meUShort  doto, llen ;
    register meWindow *wp;
    register meInt   lineno ;

    lchange(WFMOVEL|WFMAIN|WFSBOX);
    lp1  = frameCur->windowCur->dotLine;                   /* Get the address and  */
    doto = frameCur->windowCur->dotOffset;                   /* offset of "."        */
    llen = meLineGetLength(lp1)-doto ;
    if((doto == 0) || (llen > doto+32))
    {
        /* the length of the rest of the line is greater than doto+32
         * so use the current line for the next line and create a new this line
         */
        if ((lp2=lalloc(doto)) == NULL) /* New second line      */
            return meFALSE ;
        lp2->prev = lp1->prev;
        lp1->prev->next = lp2;
        lp2->next = lp1 ;
        lp1->prev = lp2 ;
        if(doto)
        {
            /* Shuffle text around  */
            register meUByte *cp1, *cp2 ;
            
            llen = doto ;
            cp1 = lp1->text ;
            cp2 = lp2->text ;
            while(llen--)
                *cp2++ = *cp1++ ;
            cp2 = lp1->text ;
            while((*cp2++ = *cp1++))
                ;
            *cp2 = '\0' ;
            lp1->length -= doto ;
        }
        /* keep alpha marks on the top line - important for narrows */
        if(lp1->flag & meLINE_AMARK)
            lunmarkBuffer(frameCur->bufferCur,lp1,lp2) ;
        /* also true for any $line-schemes */
        if(lp1->flag & meLINE_SCHEME_MASK)
        {
            lp2->flag |= lp1->flag & meLINE_SCHEME_MASK ;
            lp1->flag &= ~meLINE_SCHEME_MASK ;
        }
        lineno = frameCur->windowCur->dotLineNo ;
        meFrameLoopBegin() ;
        wp = loopFrame->windowList ;
        while(wp != NULL)
        {
            if(wp->buffer == frameCur->bufferCur)
            {
                if(wp->vertScroll > lineno)
                    wp->vertScroll++ ;
                if(wp->dotLineNo == lineno)
                {
                    if (wp->dotOffset >= doto)
                    {
                        wp->dotLineNo++ ;
                        wp->dotOffset -= doto;
                    }
                    else
                        wp->dotLine = lp2;
                }
                else if(wp->dotLineNo > lineno)
                    wp->dotLineNo++ ;
                if(wp->markLineNo == lineno)
                {
                    if (wp->markOffset > doto)
                    {
                        wp->markLineNo++ ;
                        wp->markOffset -= doto;
                    }
                    else
                        wp->markLine = lp2;
                }
                else if(wp->markLineNo > lineno)
                    wp->markLineNo++ ;
            }
            wp = wp->next;
        }
        meFrameLoopEnd() ;
    }
    else
    {
        register meUByte *cp1, *cp2, *cp3 ;
        
        if ((lp2=lalloc(llen)) == NULL) /* New second line      */
            return meFALSE ;
        cp2 = cp1 = lp1->text+doto ;                /* Shuffle text around  */
        cp3 = lp2->text ;
        while((*cp3++=*cp2++))
            ;
        *cp1 = '\0' ;
        lp1->length = doto ;
        lp2->next = lp1->next;
        lp1->next->prev = lp2;
        lp2->prev = lp1 ;
        lp1->next = lp2 ;
        
        lineno = frameCur->windowCur->dotLineNo ;
        meFrameLoopBegin() ;
        wp = loopFrame->windowList;                            /* Windows              */
        while(wp != NULL)
        {
            if(wp->buffer == frameCur->bufferCur)
            {
                if(wp->vertScroll > lineno)
                    wp->vertScroll++ ;
                if(wp->dotLineNo == lineno)
                {
                    if (wp->dotOffset >= doto)
                    {
                        wp->dotLine = lp2;
                        wp->dotLineNo++ ;
                        wp->dotOffset -= doto;
                    }
                }
                else if(wp->dotLineNo > lineno)
                    wp->dotLineNo++ ;
                if(wp->markLineNo == lineno)
                {
                    if (wp->markOffset > doto)
                    {
                        wp->markLine = lp2;
                        wp->markLineNo++ ;
                        wp->markOffset -= doto;
                    }
                }
                else if(wp->markLineNo > lineno)
                    wp->markLineNo++ ;
            }
            wp = wp->next;
        }
        meFrameLoopEnd() ;
    }
    frameCur->bufferCur->lineCount++ ;
    return meTRUE ;
}

/*
 * Save the current kill context. This involves making a new klist element and
 * putting it at the head of the klists (ie make khead point at it).
 *
 * The next kill will go into khead->kill, the previous kill can be accessed
 * via khead->k_next->kill
 *
 * The routine is made slightly more complicated because the number of elements
 * in the klist is kept to meKILL_MAX (defined in edef.h). Every time we enter this
 * routine we have to count the number of elements in the list and throw away
 * (ie free) the excess ones.
 *
 * Return meTRUE, except when we cannot malloc enough space for the new klist
 * element.
 */
int
ksave(void)
{
    meShort count=0 ;
    meKill *thiskl, *lastkl ;/* pointer to last klist element */

    /*
     * see if klhead points to anything, if not then our job is easy,
     * we setup klhead and make currkill point to the kill buffer that
     * klhead points to.
     */
    thiskl = klhead ;
    /*
     * There is at least one element in the klist. Count the total number
     * of elements and if over meKILL_MAX, delete the oldest kill.
     */
    while(thiskl != NULL)
    {
        if(count++ == meKILL_MAX)
            break ;
        lastkl = thiskl;
        thiskl = thiskl->next;
    }
    if(thiskl != NULL)
    {
        meKillNode *next, *kill ;
        /*
         * One element too many. Free the kill buffer(s) associated
         * with this klist, and finally the klist itself.
         */
        kill = thiskl->kill ;
        while(kill != NULL)
        {
            next = kill->next;
            meFree(kill);
            kill = next;
        }
        meFree(thiskl) ;
        /*
         * Now make sure that the previous element in the list does
         * not point to the thing that we have freed.
         */
        lastkl->next = (meKill*) NULL;
        reyankLastYank = NULL ;
    }

    /*
     * There was a klhead. malloc a new klist element and wire it in to
     * the head of the list.
     */
    if((thiskl = (meKill *) meMalloc(sizeof(meKill))) == NULL)
        return meFALSE ;
    thiskl->next = klhead ;
    klhead = thiskl ;
    thiskl->kill = NULL ;
    currkill = &(thiskl->kill) ;

#ifdef _CLIPBRD
    if(clipState & (CLIP_TRY_SET|CLIP_STALE))
        TTsetClipboard() ;
#endif

    return meTRUE ;
}

/* Add a new kill block of size count to the current kill.
 * returns NULL on failure, pointer to the 1st byte of
 * the block otherwise.
 */
meUByte *
kaddblock(meInt count)
{
    meKillNode *nbl ;

    if((nbl = (meKillNode*) meMalloc(sizeof(meKillNode)+count)) == NULL)
        return NULL ;
    nbl->next = NULL ;
    nbl->data[count] = '\0' ;
    *currkill = nbl ;
    currkill  = &(nbl->next) ;
    return nbl->data ;
}

/*
 * This function deletes "n" bytes, starting at dot. It understands how do deal
 * with end of lines, etc. It returns meTRUE if all of the characters were
 * deleted, and meFALSE if they were not (because dot ran into the end of the
 * buffer. The characters deleted are returned in the string. It is assumed
 * that the kill buffer is not in use.
 * SWP - changed because
 * a. Incredibly slow and inefficient
 * b. The mark line wasn't being undated properly
 * c. Needs to return the number of chars not deleted so that the undo can
 *    be corrected
 *  nn      - # of chars to delete
 *  kstring - put killed text into kill string
 */
int
mldelete(meInt noChrs, meUByte *kstring)
{
    meLine   *slp, *elp, *fslp, *felp ;
    meInt   slno, elno, nn=noChrs, ii, soff, eoff ;
    meUByte  *ks, *ss ;
    meWindow *wp ;
    
    ks = kstring ;
    if(noChrs == 0)
    {
        if(ks != NULL)
            *ks = '\0' ;
        return 0 ;
    }
    slp  = frameCur->windowCur->dotLine ;
    soff = frameCur->windowCur->dotOffset ;
    ii = meLineGetLength(slp) - soff ;
    if(nn <= ii)
    {
        /* this can be treated as a special quick case because
         * it is simply removing a few chars from the middle of
         * the line - removal as window position updates are
         * special case and very easy
         */
        meUByte *s1, *s2 ;
        
        s1 = slp->text+soff ;
        if(ks != NULL)
        {
            s2 = s1 ;
            ii = nn ;
            while(ii--)
                *ks++ = *s2++ ;
            *ks = '\0' ;
        }
        s2 = s1+nn ;
        while((*s1++ = *s2++) != '\0')
            ;
        slp->length -= (meUShort) nn ;
        slp->flag |= meLINE_CHANGED ;
        
        /* Last thing to do is update the windows */
        meFrameLoopBegin() ;
        wp = loopFrame->windowList ;
        while (wp != NULL)
        {
            if(wp->buffer == frameCur->bufferCur)
            {
                if((wp->dotLine == slp) && (((meInt) wp->dotOffset) > soff))
                {
                    if((ii=wp->dotOffset - nn) < soff)
                        wp->dotOffset = (meUShort) soff ;
                    else
                        wp->dotOffset = (meUShort) ii ;
                }
                if((wp->markLine == slp) && (((meInt) wp->markOffset) > soff))
                {
                    if((ii=wp->markOffset - nn) < soff)
                        wp->markOffset = (meUShort) soff ;
                    else
                        wp->markOffset = (meUShort) ii ;
                }
                wp->updateFlags |= WFMAIN ;
            }
            wp = wp->next;
        }
        meFrameLoopEnd() ;
        /* Can't fail, must have remove all chars */
        return 0 ;
    }
    slno = frameCur->windowCur->dotLineNo ;
    elp = meLineGetNext(slp) ;
    elno = slno+1 ;
    eoff = 0 ;
    nn -= ii + 1 ;
    if(ks != NULL)
    {
        ss = slp->text+soff ;
        while(ii--)
            *ks++ = *ss++ ;
        *ks++ = meNLCHAR ;
    }
    /* The deletion involves more than one line, lets find the end point */
    while((nn != 0) && (elp != frameCur->bufferCur->baseLine))
    {
        ii = meLineGetLength(elp) ;
        if(nn <= ii)
        {
            /* Ends somewhere in this line, set the offset and break */
            eoff = (meUShort) nn ;
            if(ks != NULL)
            {
                ss = elp->text ;
                for( ; nn ; nn--)
                    *ks++ = *ss++ ;
            }
            else
                nn = 0 ;
            break ;
        }
        /* move to the next line, decrementing the counter */
        nn -= ii+1 ;
        if(ks != NULL)
        {
            ss = elp->text ;
            for( ; ii ; ii--)
                *ks++ = *ss++ ;
            *ks++ = meNLCHAR ;
        }
        elp = meLineGetNext(elp) ;
        elno++ ;
    }
    if(ks != NULL)
        *ks = '\0' ;
    /* quick test first for speed, see if we can simply use the last line */
    if(soff == 0)
    {
        /* here we can simply use the end line */
        if(eoff != 0)
        {
            meUByte *s1, *s2 ;
            
            s1 = elp->text ;
            s2 = s1+eoff ;
            while((*s1++ = *s2++) != '\0')
                ;
            elp->length -= (meUShort) eoff ;
        }
        fslp = slp ;
        felp = elp ;
        slp = elp ;
    }
    else
    {
        /* construct the new joined start and end line, 3 posibilities
         * a. use the start line
         * b. use the end line
         * c. create a new line
         * 
         * NOTE There is a 4th possibility, the two joined lines could be
         * too long (>0xfff0) for a single line. This is tested for in mldelete
         * line and avoid so if this function is used else where, beware!
         */
        int newl ;
        ii = meLineGetLength(elp) - eoff ;
        newl = ii + soff ;
        if(newl > 0x0fff0)
            /* this deletion will leave the joined line too long, abort */
            return noChrs ;
        if(slp->size >= newl)
        {
            /* here we have got to test for one special case, if the elp is
             * the buffer baseLine then eoff must be 0, we can't remove this
             * line so can't remove last '\n' and increment nn or but don't
             * consider this an error? Not sure yet
             */
            if(elp == frameCur->bufferCur->baseLine)
            {
                slp->text[soff] = '\0' ;
                slp->length = (meUShort) soff ;
                felp = elp ;
                /* increment the no-lines cause we're only pretending we've
                 * removed the last line
                 */
                frameCur->bufferCur->lineCount++ ;
            }
            else
            {
                meUByte *s1, *s2 ;
            
                slp->length = newl ;
                s1 = slp->text+soff ;
                s2 = elp->text+eoff ;
                while((*s1++ = *s2++) != '\0')
                    ;
                felp = meLineGetNext(elp) ;
            }
            fslp = meLineGetNext(slp) ;
        }
        else if(elp->size >= newl)
        {
            meUByte *s1, *s2 ;
            
            if(soff > eoff)
            {
                s1 = elp->text+newl ;
                s2 = elp->text+elp->length ;
                do
                    *s1-- = *s2-- ;
                while(ii--) ;
            }
            else if(soff < eoff)
            {
                s1 = elp->text+soff ;
                s2 = elp->text+eoff ;
                while((*s1++ = *s2++) != '\0')
                    ;
            }
            elp->length = (meUShort) newl ;
            s1 = elp->text ;
            s2 = slp->text ;
            ii = soff ;
            while(ii--)
                *s1++ = *s2++ ;
            fslp = slp ;
            felp = elp ;
            slp = elp ;
        }
        else
        {
            meUByte *s1, *s2 ;
            fslp = slp ;
            if((slp=lalloc(newl)) == NULL)
                return noChrs ;
            s1 = slp->text ;
            s2 = fslp->text ;
            ii = soff ;
            while(ii--)
                *s1++ = *s2++ ;
            s2 = elp->text+eoff ;
            while((*s1++ = *s2++) != '\0')
                ;
            slp->next = elp->next ;
            elp->next->prev = slp ;
            slp->prev = elp ;
            elp->next = slp ;
            felp = slp ;
        }
    }
    
    /* due to the last '\n' & long line special cases above, it is possible
     * that we aren't actually removing any lines, check this
     */
    if(fslp != felp)
    {
        /* link out the unwanted section fslp -> meLineGetPrev(felp) and free */
        felp->prev->next = NULL ;
        fslp->prev->next = felp ;
        felp->prev = fslp->prev ;
        
        while(fslp != NULL)
        {
            if(fslp->flag & meLINE_AMARK)
            {
                lunmarkBuffer(frameCur->bufferCur,fslp,slp);
                /* bit of a fudge, give the $line-scheme of a line which also has an alpha
                 * mark than any other, often used with narrows etc */
                if(fslp->flag & meLINE_SCHEME_MASK)
                    slp->flag &= ~meLINE_SCHEME_MASK ;
            }
            if((fslp->flag & meLINE_SCHEME_MASK) && !(slp->flag & meLINE_SCHEME_MASK))
                slp->flag |= fslp->flag & meLINE_SCHEME_MASK ;
            felp = meLineGetNext(fslp) ;
            meFree(fslp) ;
            fslp = felp ;
        }
    }
    slp->flag |= meLINE_CHANGED ;
    
    /* correct the buffers number of lines */
    frameCur->bufferCur->lineCount -= elno-slno ;
    
    /* Last thing to do is update the windows */
    meFrameLoopBegin() ;
    wp = loopFrame->windowList;
    while (wp != NULL)
    {
        if(wp->buffer == frameCur->bufferCur)
        {
            if(wp->vertScroll > slno)
            {
                if(wp->vertScroll > elno)
                    wp->vertScroll -= elno-slno ;
                else
                    wp->vertScroll = slno ;
            }
            if(wp->dotLineNo >= slno)
            {
                if(wp->dotLineNo > elno)
                    wp->dotLineNo -= elno-slno ;
                else
                {
                    if((wp->dotLineNo == elno) && (wp->dotOffset > eoff))
                        wp->dotOffset = (meUShort) (soff + wp->dotOffset - eoff) ;
                    else if((wp->dotLineNo != slno) || (wp->dotOffset > soff))
                        wp->dotOffset = (meUShort) soff ;
                    wp->dotLine = slp ;
                    wp->dotLineNo = slno ;
                }
            }
            if(wp->markLineNo >= slno)
            {
                if(wp->markLineNo > elno)
                    wp->markLineNo -= elno-slno ;
                else
                {
                    if((wp->markLineNo == elno) && (wp->markOffset > eoff))
                        wp->markOffset = (meUShort) (soff + wp->markOffset - eoff) ;
                    else if((wp->markLineNo != slno) || (wp->markOffset > soff))
                        wp->markOffset = (meUShort) soff ;
                    wp->markLine = slp ;
                    wp->markLineNo = slno ;
                }
            }
            wp->updateFlags |= WFMOVEL|WFMAIN|WFSBOX ;
        }
        wp = wp->next;
    }
    meFrameLoopEnd() ;
    
    /* return the number of chars left to delete, usually 0 */
    return nn ;
}

/*
 * This function deletes "n" bytes, starting at dot. It understands how do deal
 * with end of lines, etc. It returns meTRUE if all of the characters were
 * deleted, and meFALSE if they were not (because dot ran into the end of the
 * buffer. The "kflag" is meTRUE if the text should be put in the kill buffer.
 */
/* n     -  # of chars to delete */
/* kflag -  put killed text in kill buffer flag */
int
ldelete(meInt nn, int kflag)
{
    meLine *ll ;
    meUByte *kstring ;
    long ret ;
    
    /* A quick test to make failure handling easier */
    ll = frameCur->windowCur->dotLine ;
    if((ll == frameCur->bufferCur->baseLine) && nn)
        return meFALSE ;
    
    /* Must get the # chars we can delete */
    ret = nn + frameCur->windowCur->dotOffset - meLineGetLength(ll) ;
    while(ret > 0)
    {
        ll = meLineGetNext(ll) ;
        if(ll == frameCur->bufferCur->baseLine)
        {
            /* The last but ones line can only be removed if the current 
             * offset is 0
             */
            if(frameCur->windowCur->dotOffset == 0)
                ret -- ;
            break ;
        }
        ret -= meLineGetLength(ll)+1 ;
    }
    if(ret > 0)
    {
        nn -= ret ;
        /* This is a bit grotty, we cannot remove the last \n, so the count has
         * been adjusted appropriately, but neither should we generate an error.
         * So only generate an error if the number of chars we want to delete but
         * can't is greater that 1
         */
        if(ret > 1)
            ret = meFALSE ;
        else
            ret = meTRUE ;
    }
    else if((((long) frameCur->windowCur->dotOffset) - ret) > 0xfff0)
    {
        /* The last line will be too long - dont remove the last \n and the offset
         * on the last line, i.e. if G is a wanted char and D is a char that should
         * be deleted, we start with:
         * 
         * GGGGGGDDDDDDDDDD
         * DDDDDDDDDDDDDDDD
         * DDDDDDDDDDDDDDDD
         * DDDDDDDGGGGGGGG
         * 
         * We will try to get it to
         * 
         * GGGGGG
         * DDDDDDDGGGGGGGG
         * 
         * All we have to do is reduce the no chars appropriately
         */
        nn -= meLineGetLength(ll)+ret+1 ;
        mlwrite(MWABORT|MWPAUSE,(meUByte *)"[Line too long!]") ;
        ret = meFALSE ;
    }        
    else
        ret = meTRUE ;
    if(kflag & 1)
    {   /* Kill?                */
        if((lastflag != meCFKILL) && (thisflag != meCFKILL))
            ksave();
        if((kstring = kaddblock(nn)) == NULL)
            return meFALSE ;
        thisflag = meCFKILL;
    }
    else
        kstring = NULL ;
#if MEOPT_UNDO
    if(kflag & 2)
    {
        if(!(kflag & 4) && (nn == 1))
            meUndoAddDelChar() ;
        else
            meUndoAddDelChars(nn) ;
    }
#endif
    mldelete(nn,kstring) ;
    return ret ;
}

/*
 * yank text back via a klist.
 *
 * Return meTRUE if all goes well, meFALSE if the characters from the kill buffer
 * cannot be inserted into the text.
 */

int
yankfrom(struct meKill *pklist)
{
    int ii, cutLen ;
    int len=0 ;
    meUByte *ss, *dd, *tt, cc ;		/* pointer into string to insert */
    meKillNode *killp ;

    /* if pasting to the end of a very long line, the combined length could
     * be too lone (> 0xfff0), try to cope with this. */
    cutLen = 0xfff0 - meLineGetLength(frameCur->windowCur->dotLine) ;
    killp = pklist->kill;
    while (killp != NULL)
    {
        ii = 0 ;
        tt = ss = killp->data ;
        for(;;)
        {
            if(((cc = *ss++) == '\0') || (cc == meNLCHAR) || (ii >= cutLen))
            {
                if(ii)
                {
                    meAssert(meLineGetChar(frameCur->windowCur->dotLine,meLineGetLength(frameCur->windowCur->dotLine)) == '\0') ;
                    len += ii ;
                    if((dd = lmakespace(ii))==NULL)	/* Make space for the characters */
                        return -1 ;		/* Error !!! */
                    meAssert(meLineGetChar(frameCur->windowCur->dotLine,meLineGetLength(frameCur->windowCur->dotLine)) == '\0') ;
                    lchange(WFMAIN) ;	        /* Declare editied buffer */
                    for( ; ii ; ii--)
                        *dd++ = *tt++ ;
                }
                if(cc == '\0')
                    break ;
                    meAssert(meLineGetChar(frameCur->windowCur->dotLine,meLineGetLength(frameCur->windowCur->dotLine)) == '\0') ;
                if(lnewline() != meTRUE)
                    return -1 ;
                    meAssert(meLineGetChar(frameCur->windowCur->dotLine,meLineGetLength(frameCur->windowCur->dotLine)) == '\0') ;
                if(cc == meNLCHAR)
                    tt++ ;
                cutLen = 0xfff0 ;
                len++ ;
            }
            else
                ii++ ;
        }
        killp = killp->next;
    }
    return len ;
}

/*
 * Yank text back from the kill buffer. This is really easy. All of the work
 * is done by the standard insert routines. All you do is run the loop, and
 * check for errors. Bound to "C-Y".
 */
int
yank(int f, int n)
{
    register int ret ;		/* return from "yankfrom()" */
    register int len = 0 ;	/* return from "yankfrom()" */

    commandFlag[CK_YANK] = (comSelStart|comSelSetDot|comSelSetMark|comSelSetFix) ;
    if(n == 0)
    {
        /* place the mark on the current line */
        windowSetMark(meFALSE, meFALSE);
        /* remember that this was a yank command */
        thisflag = meCFYANK ;
        return meTRUE ;
    }
    if(n < 0)
    {
        meKill *kl ;
        
        while((++n <= 0) && ((kl = klhead) != NULL))
        {
            meKillNode *next, *kill ;
            kill = kl->kill ;
            while(kill != NULL)
            {
                next = kill->next;
                meFree(kill);
                kill = next;
            }
            klhead = kl->next ;
            meFree(kl) ;
        }
        reyankLastYank = NULL ;
        commandFlag[CK_YANK] = comSelKill ;
        return meTRUE ;
    }
        
#ifdef _CLIPBRD
    if(clipState & CLIP_TRY_GET)
        TTgetClipboard() ;
#endif
    /* make sure there is something to yank */
    if(klhead == NULL)
        return mlwrite(MWABORT,(meUByte *)"[nothing to yank]");
    /* Check we can change the buffer */
    if((ret=bchange()) != meTRUE)
        return ret ;

    /* place the mark on the current line */
    windowSetMark(meFALSE, meFALSE);

    /* for each time.... */
    while(n--)
    {
        if((ret = yankfrom(klhead)) < 0)
            break;
        len += ret ;
    }

    if(ret >= 0)
    {
#if MEOPT_UNDO
        meUndoAddInsChars(len) ;
#endif
        /* remember that this was a yank command */
        thisflag = meCFYANK ;
        return meTRUE ;
    }
    return meFALSE ;
}

int
reyank(int f, int n)
{
    register int  ret, len=0;		/* return value of things	*/
    meRegion        region;		/* dot to mark region		*/

    if(lastflag == meCFYANK)
        /* Last command was yank. Set reyankLastYank to the most recent
         * delete, so we reyank the second most recent */
        reyankLastYank = klhead;
    else if(lastflag != meCFRYANK)
        /* Last command was not a reyank or yank, set reyankLastYank to
         * NULL so we bitch */
        reyankLastYank = NULL ;
    
    if(reyankLastYank == NULL)
        /*
         * Last command was not a yank or reyank (or someone was
         * messing with @cc and got it wrong). Complain and return
         * immediately, this is because the first thing we do is to
         * delete the current region without saving it. If we've just
         * done a yank, or reyank, the mark and cursor will surround
         * the last yank so this will be ok, but otherwise the user
         * would loose text with no hope of recovering it.
         */
        return mlwrite(MWABORT,(meUByte *)"[reyank must IMMEDIATELY follow a yank or reyank]");

    reyankLastYank = reyankLastYank->next;
    /*
     * Delete the current region.
     */
    if((ret = getregion(&region)) != meTRUE)
        return(ret);

    frameCur->windowCur->dotLine  = region.line;
    frameCur->windowCur->dotOffset  = region.offset;
    frameCur->windowCur->dotLineNo = region.lineNo;
    ldelete(region.size,6);

    /*
     * Set the mark here so that we can delete the region in the next
     * reyank command.
     */
    windowSetMark(meFALSE, meFALSE);

    /*
     * If we've fallen off the end of the klist and there are no more
     * elements, wrap around to the most recent delete. This makes it
     * appear as if the linked list of kill buffers is actually
     * implemented as the GNU style "kill ring".
     *
     * Put out a warning as well.
     */
    if(reyankLastYank == (meKill*) NULL)
    {
        mlwrite(MWABORT,(meUByte *)"[start of kill-ring]");
        reyankLastYank = klhead;
    }

    while(n--)
    {
        if((ret = yankfrom(reyankLastYank)) < 0)
            return meFALSE ;
        len += ret ;
    }
#if MEOPT_UNDO
    meUndoAddReplaceEnd(len) ;
#endif
    /* Remember that this is a reyank command for next time. */
    thisflag = meCFRYANK;

    return(meTRUE);
}

void
freeLineLoop(meLine *lp, int flag)
{
    meLine *clp, *nlp ;
    clp = meLineGetNext(lp) ;
    while(clp != lp)
    {
        nlp = meLineGetNext(clp) ;
        meFree(clp) ;
        clp = nlp ;
    }
    if(flag)
        meFree(clp) ;
    else
        clp->next = clp->prev = clp ;
}
