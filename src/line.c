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
 *     editing must be being displayed, which means that "b_nwnd" is non zero,
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

static KILL **currkill=NULL ;	/* current kill buffer */
static KLIST *reyankLastYank=NULL;

void
lunmarkBuffer(BUFFER *bp, LINE *lp, LINE *nlp)
{
    /*
     * The line "lp" is marked by an alphabetic mark. Look through
     * the linked list of marks for the one that references this
     * line and update to the new line
     */
    register meAMARK* p=bp->b_amark;

    while(p != NULL)
    {
        if(p->line == lp)
            p->line = nlp ;
        p = p->next;
    }
    /* also swap the flag */
    nlp->l_flag |= LNMARK ;
    lp->l_flag &= ~LNMARK ;
}


/****************************************************************************
 * This routine allocates a block of memory large enough to hold a LINE
 * containing "used" characters. The block is always rounded up a bit. Return
 * a pointer to the new block, or NULL if there isn't any memory left. Print a
 * message in the message line if no space.
 ******************************************************************************/

LINE *
lalloc(register int used)
{
    register LINE   *lp;
    register int    size;

    size = (used+NBLOCK-1) & ~(NBLOCK-1);
    if (size == 0)                          /* Assume that an empty */
        size = NBLOCK;                      /* line is for type-in. */
    if ((lp = (LINE *) meMalloc(sizeof(LINE)+size)) == NULL)
        return (NULL);
    lp->l_size = size;
    lp->l_used = used;
    lp->l_text[used] = '\0' ;
    lp->l_flag = LNCHNG ;		    /* line is not marked.	*/

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
    if(meModeTest(curbp->b_mode,MDVIEW))
        return rdonly() ;
    if(!meModeTest(curbp->b_mode,MDEDIT))      /* First change, so     */
    {
        if((curbp->b_bname[0] != '*') &&
           (bufferOutOfDate(curbp)) &&
           (mlyesno((uint8 *)"File changed on disk, really edit") != TRUE))
            return ctrlg(TRUE,1) ;
        meModeSet(curbp->b_mode,MDEDIT);
        addModeToWindows(WFMODE) ;
#if MEUNDO
        if(meModeTest(curbp->b_mode,MDUNDO))
        {
            UNDOND        *uu ;

            uu = meUndoCreateNode(sizeof(UNDOND)) ;
            uu->type |= MEUNDO_FRST ;
            /* Add and narrows, must get the right order */
        }
#endif
    }
    if((autotime > 0) && (curbp->autotime < 0) &&
       meModeTest(curbp->b_mode,MDATSV) &&
       (curbp->b_bname[0] != '*'))
    {
        struct   meTimeval tp ;

        gettimeofday(&tp,NULL) ;
        curbp->autotime = ((tp.tv_sec-startTime+autotime)*1000) +
                  (tp.tv_usec/1000) ;
        if(!isTimerExpired(AUTOS_TIMER_ID) &&
           (!isTimerSet(AUTOS_TIMER_ID) || 
            (meTimerTime[AUTOS_TIMER_ID] > curbp->autotime)))
            timerSet(AUTOS_TIMER_ID,curbp->autotime,((long)autotime) * 1000L) ;
    }
    return TRUE ;
}

void
lchange(register int flag)
{
    curwp->w_dotp->l_flag |= LNCHNG ;

    if (curbp->b_nwnd != 1)             /* If more than 1 window, update rest */
    {
        register WINDOW *wp;

        wp = wheadp;
        while (wp != NULL)
        {
            if(wp->w_bufp == curbp)
                wp->w_flag |= flag;
            wp = wp->w_wndp;
        }
    }
    else
        curwp->w_flag |= flag;              /* update current window */
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
uint8 *
lmakespace(int n)
{
    uint8  *cp1;
    WINDOW *wp;
    LINE   *lp_new;
    LINE   *lp_old;
    uint16  doto ;
    int     newl ;

    lp_old = curwp->w_dotp;			/* Current line         */
    if((newl=(n+((int) llength(lp_old)))) > 0x0fff0)
    {
        mlwrite(MWABORT|MWPAUSE,(uint8 *)"[Line too long!]") ;
        return NULL ;
    }
    if (lp_old == curbp->b_linep)		/* At the end: special  */
    {
        /*---	Allocate a new line */

        if ((lp_new=lalloc(n)) == NULL)    /* Allocate new line    */
            return (NULL);

        /*---	Link in with the previous line */

#if MEUNDO
        meUndoAddInsChar() ;
#endif
        curbp->elineno++ ;
        lp_new->l_fp = lp_old;
        lp_new->l_bp = lp_old->l_bp;
        lp_new->l_flag = lp_old->l_flag|LNCHNG ;
        /* only the new line should have any $line-scheme */
        lp_old->l_flag &= ~LNSMASK ;
        lp_old->l_bp->l_fp = lp_new;
        lp_old->l_bp = lp_new;

        /* keep alpha marks on the top line - important for narrows */
        if(lp_old->l_flag & LNMARK)
            lunmarkBuffer(curbp,lp_old,lp_new) ;
        cp1 = lp_new->l_text ;	/* Return pointer */
        doto = 0 ;
    }
    else
    {
        doto = curwp->w_doto;           /* Save for later.      */
        if (newl > lp_old->l_size)      /* Hard: reallocate     */
        {
            uint8 *cp2 ;
            uint16 ii ;

            /*---	Allocate new line of correct length */

            if ((lp_new = lalloc (newl)) == NULL)
                return (NULL);

            /*---	Copy line */

            cp1 = &lp_old->l_text[0];
            cp2 = &lp_new->l_text[0];
            for (ii=doto ; ii>0 ; ii--)
                *cp2++ = *cp1++ ;
            cp2 += n;
            while((*cp2++ = *cp1++))
                ;

            /*---	Re-link in the new line */

            lp_new->l_bp = lp_old->l_bp;
            lp_new->l_fp = lp_old->l_fp;
            /* preserve the $line-scheme */
            lp_new->l_flag = lp_old->l_flag|LNCHNG ;
            lp_old->l_bp->l_fp = lp_new;
            lp_old->l_fp->l_bp = lp_new;
            if(lp_old->l_flag & LNMARK)
                lunmarkBuffer(curbp,lp_old,lp_new);
            meFree(lp_old);
            cp1 = lp_new->l_text + doto ;		/* Save position */
        }
        else                                /* Easy: in place       */
        {
            uint8 *cp2 ;
            uint16 ii ;

            lp_new = lp_old;                      /* Pretend new line     */
            lp_new->l_used = newl ;
            lp_new->l_flag |= LNCHNG ;
            cp2 = lp_old->l_text + lp_old->l_used ;
            cp1 = cp2 - n ;
            ii  = cp1 - lp_old->l_text - doto ;
            do
                *cp2-- = *cp1-- ;
            while(ii--) ;
            cp1++ ;
        }
    }

    wp = wheadp;                    /* Update windows       */
    while (wp != NULL)
    {
        if(wp->w_bufp == curbp)
        {
            if (wp->w_dotp == lp_old)
            {
                wp->w_dotp = lp_new;
                if (wp==curwp || wp->w_doto>doto)
                    wp->w_doto += n;
            }
            if (wp->w_markp == lp_old)
            {
                wp->w_markp = lp_new;
                if (wp->w_marko > doto)
                    wp->w_marko += n;
            }
        }
        wp = wp->w_wndp;
    }
    return cp1 ;
}

/*
 * Insert "n" copies of the character "c" at the current location of dot. In
 * the easy case all that happens is the text is stored in the line. In the
 * hard case, the line has to be reallocated. When the window list is updated,
 * take special care; I screwed it up once. You always update dot in the
 * current window. You update mark, and a dot in another window, if it is
 * greater than the place where you did the insert. Return TRUE if all is
 * well, and FALSE on errors.
 */
int
linsert(int n, int c)
{
    uint8 *cp;

    lchange(WFMOVEC|WFMAIN);		/* Declare editied buffer */
    if ((cp = lmakespace(n))==NULL)     /* Make space for the characters */
        return (FALSE);		        /* Error !!! */
    while (n-- > 0)                     /* Copy in the characters */
        *cp++ = c;
    return (TRUE);
}

/*---	As insrt char, insert string */
int
lsinsert(register int n, register uint8 *cp)
{
    uint8 *lp ;

    if(n == 0)
    {
        for( ; cp[n] != 0 ; n++)
            ;
        if(n == 0)
            return TRUE ;
    }
    lchange(WFMAIN) ;
    if ((lp = lmakespace(n))==NULL)	/* Make space for the characters */
        return (FALSE);		/* Error !!! */
    while(n-- > 0)			/* Copy in the characters */
        *lp++ = *cp++;
    return TRUE ;
}

int
insSpace(int f, int n)	/* insert spaces forward into text */
{
    register int s ;
    
    if(n <= 0)
        return TRUE ;
    if((s=bchange()) != TRUE)               /* Check we can change the buffer */
        return s ;
    s = linsert(n, ' ');
#if MEUNDO
    meUndoAddInsChars(n) ;
#endif
    return s ;
}

int
insTab(int f, int n)	/* insert spaces forward into text */
{
    register int s ;

    if(n <= 0)
        return TRUE ;
    if((s=bchange()) != TRUE)               /* Check we can change the buffer */
        return s ;
    s = linsert(n, '\t');
#if MEUNDO
    meUndoAddInsChars(n) ;
#endif
    return s ;
}

/*
 * Insert a newline into the buffer at the current location of dot in the
 * current window. The funny ass-backwards way it does things is not a botch;
 * it just makes the last line in the file not a special case. Return TRUE if
 * everything works out and FALSE on error (memory allocation failure). The
 * update of dot and mark is a bit easier then in the above case, because the
 * split forces more updating.
 */
int
lnewline(void)
{
    register LINE   *lp1;
    register LINE   *lp2;
    register uint16  doto, llen ;
    register WINDOW *wp;
    register int32   lineno ;

    lchange(WFMOVEL|WFMAIN|WFSBOX);
    lp1  = curwp->w_dotp;                   /* Get the address and  */
    doto = curwp->w_doto;                   /* offset of "."        */
    llen = llength(lp1)-doto ;
    if((doto == 0) || (llen > doto+32))
    {
        /* the length of the rest of the line is greater than doto+32
         * so use the current line for the next line and create a new this line
         */
        if ((lp2=lalloc(doto)) == NULL) /* New second line      */
            return FALSE ;
        lp2->l_bp = lp1->l_bp;
        lp1->l_bp->l_fp = lp2;
        lp2->l_fp = lp1 ;
        lp1->l_bp = lp2 ;
        if(doto)
        {
            /* Shuffle text around  */
            register uint8 *cp1, *cp2 ;
            
            llen = doto ;
            cp1 = lp1->l_text ;
            cp2 = lp2->l_text ;
            while(llen--)
                *cp2++ = *cp1++ ;
            cp2 = lp1->l_text ;
            while((*cp2++ = *cp1++))
                ;
            *cp2 = '\0' ;
            lp1->l_used -= doto ;
        }
        /* keep alpha marks on the top line - important for narrows */
        if(lp1->l_flag & LNMARK)
            lunmarkBuffer(curbp,lp1,lp2) ;
        /* also true for any $line-schemes */
        if(lp1->l_flag & LNSMASK)
        {
            lp2->l_flag |= lp1->l_flag & LNSMASK ;
            lp1->l_flag &= ~LNSMASK ;
        }
        lineno = curwp->line_no ;
        wp = wheadp;                            /* Windows              */
        while(wp != NULL)
        {
            if(wp->w_bufp == curbp)
            {
                if(wp->topLineNo > lineno)
                    wp->topLineNo++ ;
                if(wp->line_no == lineno)
                {
                    if (wp->w_doto >= doto)
                    {
                        wp->line_no++ ;
                        wp->w_doto -= doto;
                    }
                    else
                        wp->w_dotp = lp2;
                }
                else if(wp->line_no > lineno)
                    wp->line_no++ ;
                if(wp->mlineno == lineno)
                {
                    if (wp->w_marko > doto)
                    {
                        wp->mlineno++ ;
                        wp->w_marko -= doto;
                    }
                    else
                        wp->w_markp = lp2;
                }
                else if(wp->mlineno > lineno)
                    wp->mlineno++ ;
            }
            wp = wp->w_wndp;
        }
    }
    else
    {
        register uint8 *cp1, *cp2, *cp3 ;
        
        if ((lp2=lalloc(llen)) == NULL) /* New second line      */
            return FALSE ;
        cp2 = cp1 = lp1->l_text+doto ;                /* Shuffle text around  */
        cp3 = lp2->l_text ;
        while((*cp3++=*cp2++))
            ;
        *cp1 = '\0' ;
        lp1->l_used = doto ;
        lp2->l_fp = lp1->l_fp;
        lp1->l_fp->l_bp = lp2;
        lp2->l_bp = lp1 ;
        lp1->l_fp = lp2 ;
        
        lineno = curwp->line_no ;
        wp = wheadp;                            /* Windows              */
        while(wp != NULL)
        {
            if(wp->w_bufp == curbp)
            {
                if(wp->topLineNo > lineno)
                    wp->topLineNo++ ;
                if(wp->line_no == lineno)
                {
                    if (wp->w_doto >= doto)
                    {
                        wp->w_dotp = lp2;
                        wp->line_no++ ;
                        wp->w_doto -= doto;
                    }
                }
                else if(wp->line_no > lineno)
                    wp->line_no++ ;
                if(wp->mlineno == lineno)
                {
                    if (wp->w_marko > doto)
                    {
                        wp->w_markp = lp2;
                        wp->mlineno++ ;
                        wp->w_marko -= doto;
                    }
                }
                else if(wp->mlineno > lineno)
                    wp->mlineno++ ;
            }
            wp = wp->w_wndp;
        }
    }
    curbp->elineno++ ;
    return TRUE ;
}

/*
 * Save the current kill context. This involves making a new klist element and
 * putting it at the head of the klists (ie make khead point at it).
 *
 * The next kill will go into khead->kill, the previous kill can be accessed
 * via khead->k_next->kill
 *
 * The routine is made slightly more complicated because the number of elements
 * in the klist is kept to NKILL (defined in edef.h). Every time we enter this
 * routine we have to count the number of elements in the list and throw away
 * (ie free) the excess ones.
 *
 * Return TRUE, except when we cannot malloc enough space for the new klist
 * element.
 */
int
ksave(void)
{
    int16 count=0 ;
    KLIST *thiskl, *lastkl ;/* pointer to last klist element */

    /*
     * see if klhead points to anything, if not then our job is easy,
     * we setup klhead and make currkill point to the kill buffer that
     * klhead points to.
     */
    thiskl = klhead ;
    /*
     * There is at least one element in the klist. Count the total number
     * of elements and if over NKILL, delete the oldest kill.
     */
    while(thiskl != NULL)
    {
        if(count++ == NKILL)
            break ;
        lastkl = thiskl;
        thiskl = thiskl->next;
    }
    if(thiskl != NULL)
    {
        KILL *next, *kill ;
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
        lastkl->next = (KLIST*) NULL;
        reyankLastYank = NULL ;
    }

    /*
     * There was a klhead. malloc a new klist element and wire it in to
     * the head of the list.
     */
    if((thiskl = (KLIST *) meMalloc(sizeof(KLIST))) == NULL)
        return FALSE ;
    thiskl->next = klhead ;
    klhead = thiskl ;
    thiskl->kill = NULL ;
    currkill = &(thiskl->kill) ;

#ifdef _CLIPBRD
    if(clipState & (CLIP_TRY_SET|CLIP_STALE))
        TTsetClipboard() ;
#endif

    return TRUE ;
}

/* Add a new kill block of size count to the current kill.
 * returns NULL on failure, pointer to the 1st byte of
 * the block otherwise.
 */
uint8 *
kaddblock(int32 count)
{
    KILL *nbl ;

    if((nbl = (KILL*) meMalloc(sizeof(KILL)+count)) == NULL)
        return NULL ;
    nbl->next = NULL ;
    nbl->data[count] = '\0' ;
    *currkill = nbl ;
    currkill  = &(nbl->next) ;
    return nbl->data ;
}

/*
 * This function deletes "n" bytes, starting at dot. It understands how do deal
 * with end of lines, etc. It returns TRUE if all of the characters were
 * deleted, and FALSE if they were not (because dot ran into the end of the
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
mldelete(int32 noChrs, uint8 *kstring)
{
    LINE   *slp, *elp, *fslp, *felp ;
    int32   slno, elno, nn=noChrs, ii, soff, eoff ;
    uint8  *ks, *ss ;
    WINDOW *wp ;
    
    ks = kstring ;
    if(noChrs == 0)
    {
        if(ks != NULL)
            *ks = '\0' ;
        return 0 ;
    }
    slp  = curwp->w_dotp ;
    soff = curwp->w_doto ;
    ii = llength(slp) - soff ;
    if(nn <= ii)
    {
        /* this can be treated as a special quick case because
         * it is simply removing a few chars from the middle of
         * the line - removal as window position updates are
         * special case and very easy
         */
        uint8 *s1, *s2 ;
        
        s1 = slp->l_text+soff ;
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
        slp->l_used -= (uint16) nn ;
        slp->l_flag |= LNCHNG ;
        
        /* Last thing to do is update the windows */
        wp = wheadp ;
        while (wp != NULL)
        {
            if(wp->w_bufp == curbp)
            {
                if((wp->w_dotp == slp) && (((int32) wp->w_doto) > soff))
                {
                    if((ii=wp->w_doto - nn) < soff)
                        wp->w_doto = (uint16) soff ;
                    else
                        wp->w_doto = (uint16) ii ;
                }
                if((wp->w_markp == slp) && (((int32) wp->w_marko) > soff))
                {
                    if((ii=wp->w_marko - nn) < soff)
                        wp->w_marko = (uint16) soff ;
                    else
                        wp->w_marko = (uint16) ii ;
                }
                wp->w_flag |= WFMAIN ;
            }
            wp = wp->w_wndp;
        }
        /* Can't fail, must have remove all chars */
        return 0 ;
    }
    slno = curwp->line_no ;
    elp = lforw(slp) ;
    elno = slno+1 ;
    eoff = 0 ;
    nn -= ii + 1 ;
    if(ks != NULL)
    {
        ss = slp->l_text+soff ;
        while(ii--)
            *ks++ = *ss++ ;
        *ks++ = meNLCHAR ;
    }
    /* The deletion involves more than one line, lets find the end point */
    while((nn != 0) && (elp != curbp->b_linep))
    {
        ii = llength(elp) ;
        if(nn <= ii)
        {
            /* Ends somewhere in this line, set the offset and break */
            eoff = (uint16) nn ;
            if(ks != NULL)
            {
                ss = elp->l_text ;
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
            ss = elp->l_text ;
            for( ; ii ; ii--)
                *ks++ = *ss++ ;
            *ks++ = meNLCHAR ;
        }
        elp = lforw(elp) ;
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
            uint8 *s1, *s2 ;
            
            s1 = elp->l_text ;
            s2 = s1+eoff ;
            while((*s1++ = *s2++) != '\0')
                ;
            elp->l_used -= (uint16) eoff ;
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
        ii = llength(elp) - eoff ;
        newl = ii + soff ;
        if(newl > 0x0fff0)
            /* this deletion will leave the joined line too long, abort */
            return noChrs ;
        if(slp->l_size >= newl)
        {
            /* here we have got to test for one special case, if the elp is
             * the buffer b_linep then eoff must be 0, we can't remove this
             * line so can't remove last '\n' and increment nn or but don't
             * consider this an error? Not sure yet
             */
            if(elp == curbp->b_linep)
            {
                slp->l_text[soff] = '\0' ;
                slp->l_used = (uint16) soff ;
                felp = elp ;
                /* increment the no-lines cause we're only pretending we've
                 * removed the last line
                 */
                curbp->elineno++ ;
            }
            else
            {
                uint8 *s1, *s2 ;
            
                slp->l_used = newl ;
                s1 = slp->l_text+soff ;
                s2 = elp->l_text+eoff ;
                while((*s1++ = *s2++) != '\0')
                    ;
                felp = lforw(elp) ;
            }
            fslp = lforw(slp) ;
        }
        else if(elp->l_size >= newl)
        {
            uint8 *s1, *s2 ;
            
            if(soff > eoff)
            {
                s1 = elp->l_text+newl ;
                s2 = elp->l_text+elp->l_used ;
                do
                    *s1-- = *s2-- ;
                while(ii--) ;
            }
            else if(soff < eoff)
            {
                s1 = elp->l_text+soff ;
                s2 = elp->l_text+eoff ;
                while((*s1++ = *s2++) != '\0')
                    ;
            }
            elp->l_used = (uint16) newl ;
            s1 = elp->l_text ;
            s2 = slp->l_text ;
            ii = soff ;
            while(ii--)
                *s1++ = *s2++ ;
            fslp = slp ;
            felp = elp ;
            slp = elp ;
        }
        else
        {
            uint8 *s1, *s2 ;
            fslp = slp ;
            if((slp=lalloc(newl)) == NULL)
                return noChrs ;
            s1 = slp->l_text ;
            s2 = fslp->l_text ;
            ii = soff ;
            while(ii--)
                *s1++ = *s2++ ;
            s2 = elp->l_text+eoff ;
            while((*s1++ = *s2++) != '\0')
                ;
            slp->l_fp = elp->l_fp ;
            elp->l_fp->l_bp = slp ;
            slp->l_bp = elp ;
            elp->l_fp = slp ;
            felp = slp ;
        }
    }
    
    /* due to the last '\n' & long line special cases above, it is possible
     * that we aren't actually removing any lines, check this
     */
    if(fslp != felp)
    {
        /* link out the unwanted section fslp -> lback(felp) and free */
        felp->l_bp->l_fp = NULL ;
        fslp->l_bp->l_fp = felp ;
        felp->l_bp = fslp->l_bp ;
        
        while(fslp != NULL)
        {
            if(fslp->l_flag & LNMARK)
            {
                lunmarkBuffer(curbp,fslp,slp);
                /* bit of a fudge, give the $line-scheme of a line which also has an alpha
                 * mark than any other, often used with narrows etc */
                if(fslp->l_flag & LNSMASK)
                    slp->l_flag &= ~LNSMASK ;
            }
            if((fslp->l_flag & LNSMASK) && !(slp->l_flag & LNSMASK))
                slp->l_flag |= fslp->l_flag & LNSMASK ;
            felp = lforw(fslp) ;
            meFree(fslp) ;
            fslp = felp ;
        }
    }
    /* correct the buffers number of lines */
    curbp->elineno -= elno-slno ;
    
    /* Last thing to do is update the windows */
    slp->l_flag |= LNCHNG ;
    wp = wheadp ;
    while (wp != NULL)
    {
        if(wp->w_bufp == curbp)
        {
            if(wp->topLineNo > slno)
            {
                if(wp->topLineNo > elno)
                    wp->topLineNo -= elno-slno ;
                else
                    wp->topLineNo = slno ;
            }
            if(wp->line_no >= slno)
            {
                if(wp->line_no > elno)
                    wp->line_no -= elno-slno ;
                else
                {
                    if((wp->line_no == elno) && (wp->w_doto > eoff))
                        wp->w_doto = (uint16) (soff + wp->w_doto - eoff) ;
                    else if((wp->line_no != slno) || (wp->w_doto > soff))
                        wp->w_doto = (uint16) soff ;
                    wp->w_dotp = slp ;
                    wp->line_no = slno ;
                }
            }
            if(wp->mlineno >= slno)
            {
                if(wp->mlineno > elno)
                    wp->mlineno -= elno-slno ;
                else
                {
                    if((wp->mlineno == elno) && (wp->w_marko > eoff))
                        wp->w_marko = (uint16) (soff + wp->w_marko - eoff) ;
                    else if((wp->mlineno != slno) || (wp->w_marko > soff))
                        wp->w_marko = (uint16) soff ;
                    wp->w_markp = slp ;
                    wp->mlineno = slno ;
                }
            }
            wp->w_flag |= WFMOVEL|WFMAIN|WFSBOX ;
        }
        wp = wp->w_wndp;
    }
    /* return the number of chars left to delete, usually 0 */
    return nn ;
}

/*
 * This function deletes "n" bytes, starting at dot. It understands how do deal
 * with end of lines, etc. It returns TRUE if all of the characters were
 * deleted, and FALSE if they were not (because dot ran into the end of the
 * buffer. The "kflag" is TRUE if the text should be put in the kill buffer.
 */
/* n     -  # of chars to delete */
/* kflag -  put killed text in kill buffer flag */
int
ldelete(int32 nn, int kflag)
{
    LINE *ll ;
    uint8 *kstring ;
    long ret ;
    
    /* A quick test to make failure handling easier */
    ll = curwp->w_dotp ;
    if((ll == curbp->b_linep) && nn)
        return FALSE ;
    
    /* Must get the # chars we can delete */
    ret = nn + curwp->w_doto - llength(ll) ;
    while(ret > 0)
    {
        ll = lforw(ll) ;
        if(ll == curbp->b_linep)
        {
            /* The last but ones line can only be removed if the current 
             * offset is 0
             */
            if(curwp->w_doto == 0)
                ret -- ;
            break ;
        }
        ret -= llength(ll)+1 ;
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
            ret = FALSE ;
        else
            ret = TRUE ;
    }
    else if((((long) curwp->w_doto) - ret) > 0xfff0)
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
        nn -= llength(ll)+ret+1 ;
        mlwrite(MWABORT|MWPAUSE,(uint8 *)"[Line too long!]") ;
        ret = FALSE ;
    }        
    else
        ret = TRUE ;
    if(kflag & 1)
    {   /* Kill?                */
        if((lastflag != CFKILL) && (thisflag != CFKILL))
            ksave();
        if((kstring = kaddblock(nn)) == NULL)
            return FALSE ;
        thisflag = CFKILL;
    }
    else
        kstring = NULL ;
#if MEUNDO
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
 * Return TRUE if all goes well, FALSE if the characters from the kill buffer
 * cannot be inserted into the text.
 */

int
yankfrom(struct KLIST *pklist)
{
    int ii, cutLen ;
    int len=0 ;
    uint8 *ss, *dd, *tt, cc ;		/* pointer into string to insert */
    KILL *killp ;

    /* if pasting to the end of a very long line, the combined length could
     * be too lone (> 0xfff0), try to cope with this. */
    cutLen = 0xfff0 - llength(curwp->w_dotp) ;
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
                    len += ii ;
                    if((dd = lmakespace(ii))==NULL)	/* Make space for the characters */
                        return -1 ;		/* Error !!! */
                    lchange(WFMAIN) ;	        /* Declare editied buffer */
                    for( ; ii ; ii--)
                        *dd++ = *tt++ ;
                }
                if(cc == '\0')
                    break ;
                if(lnewline() != TRUE)
                    return -1 ;
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
        setMark(FALSE, FALSE);
        /* remember that this was a yank command */
        thisflag = CFYANK ;
        return TRUE ;
    }
    if(n < 0)
    {
        KLIST *kl ;
        
        while((++n <= 0) && ((kl = klhead) != NULL))
        {
            KILL *next, *kill ;
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
        return TRUE ;
    }
        
#ifdef _CLIPBRD
    if(clipState & CLIP_TRY_GET)
        TTgetClipboard() ;
#endif
    /* make sure there is something to yank */
    if(klhead == NULL)
        return mlwrite(MWABORT,(uint8 *)"[nothing to yank]");
    /* Check we can change the buffer */
    if((ret=bchange()) != TRUE)
        return ret ;

    /* place the mark on the current line */
    setMark(FALSE, FALSE);

    /* for each time.... */
    while(n--)
    {
        if((ret = yankfrom(klhead)) < 0)
            break;
        len += ret ;
    }

    if(ret >= 0)
    {
#if MEUNDO
        meUndoAddInsChars(len) ;
#endif
        /* remember that this was a yank command */
        thisflag = CFYANK ;
        return TRUE ;
    }
    return FALSE ;
}

int
reyank(int f, int n)
{
    register int  ret, len=0;		/* return value of things	*/
    REGION        region;		/* dot to mark region		*/

    if(lastflag == CFYANK)
        /* Last command was yank. Set reyankLastYank to the most recent
         * delete, so we reyank the second most recent */
        reyankLastYank = klhead;
    else if(lastflag != CFRYANK)
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
        return mlwrite(MWABORT,(uint8 *)"[reyank must IMMEDIATELY follow a yank or reyank]");

    reyankLastYank = reyankLastYank->next;
    /*
     * Delete the current region.
     */
    if((ret = getregion(&region)) != TRUE)
        return(ret);

    curwp->w_dotp  = region.linep;
    curwp->w_doto  = region.offset;
    curwp->line_no = region.line_no;
    ldelete(region.size,6);

    /*
     * Set the mark here so that we can delete the region in the next
     * reyank command.
     */
    setMark(FALSE, FALSE);

    /*
     * If we've fallen off the end of the klist and there are no more
     * elements, wrap around to the most recent delete. This makes it
     * appear as if the linked list of kill buffers is actually
     * implemented as the GNU style "kill ring".
     *
     * Put out a warning as well.
     */
    if(reyankLastYank == (KLIST*) NULL)
    {
        mlwrite(MWABORT,(uint8 *)"[start of kill-ring]");
        reyankLastYank = klhead;
    }

    while(n--)
    {
        if((ret = yankfrom(reyankLastYank)) < 0)
            return FALSE ;
        len += ret ;
    }
#if MEUNDO
    meUndoAddReplaceEnd(len) ;
#endif
    /* Remember that this is a reyank command for next time. */
    thisflag = CFRYANK;

    return(TRUE);
}

void
freeLineLoop(LINE *lp, int flag)
{
    LINE *clp, *nlp ;
    clp = lforw(lp) ;
    while(clp != lp)
    {
        nlp = lforw(clp) ;
        meFree(clp) ;
        clp = nlp ;
    }
    if(flag)
        meFree(clp) ;
    else
        clp->l_fp = clp->l_bp = clp ;
}
