/*****************************************************************************
*
*	Title:		%M%
*
*	Synopsis:	Display routines.
*
******************************************************************************
*
*	Filename:		%P%
*
*	Author:			Danial Lawrence
*
*	Creation Date:		29/04/91 09:13
*
*	Modification date:	%G% : %U%	<000723.2038>
*
*	Current rev:		%I%
*
*	Special Comments:
*
*	Contents Description:
*
*
* The functions in this file handle redisplay. There are two halves, the
* ones that update the virtual display screen, and the ones that make the
* physical display screen the same as the virtual display screen. These
* functions use hints that are left in the windows by the commands.
*
******************************************************************************
*
* (C)opyright 1987 by Daniel M. Lawrence
* MicroEMACS 3.8 can be copied and distributed freely for any
* non-commercial purposes. MicroEMACS 3.8 can only be incorporated
* into commercial software with the permission of the current author.
*
* Modifications to the original file by Jasspa.
*
* Copyright (C) 1988 - 1999, JASSPA
* The MicroEmacs Jasspa distribution can be copied and distributed freely for
* any non-commercial purposes. The MicroEmacs Jasspa Distribution can only be
* incorporated into commercial software with the expressed permission of
* JASSPA.
*
****************************************************************************/

/*---	Include defintions */

#define	__DISPLAYC				/* Name file */

/*---	Include files */

#include "emain.h"
#include "eskeys.h"		/* External Defintions */

#ifdef _STDARG
#include <stdarg.h>		/* Variable Arguments */
#endif

#if (defined _UNIX) || (defined _DOS) || (defined _WIN32)
#include <sys/types.h>
#include <time.h>
#endif

#ifdef _DOS
#include <pc.h>
#endif

/*---	Local macro definitions */

/*---	External references */

/*---	Local type definitions */

/*
 * Set the virtual cursor to the specified row and column on the virtual
 * screen. There is no checking for nonsense values; this might be a good
 * idea during the early stages.
 */
/*---	Local variable definitions */

VVIDEO        vvideo;                   /* Head of the virtual video list */
LINE          *mline;                   /* message line. */
uint16 TTsrow = 0;              /* Terminal starting row */

/* Poke screen flags
 * Poke screen maintains the rectangular extents of the region of the
 * screen that has been damaged. These screen areas are fixed on the next
 * invocation to update(). Note that we only initialise the flag, the other
 * fields should be automatically initialised since we start in a garbaged
 * state.
 *
 * Note that all poke extents are 'inclusive' values.
 */
static void pokeUpdate (void);          /* The poke screen update function */
static char poke_flag = 0;              /* Boolean TRUE/FALSE flag. TRUE
                                         * when a poke operation has been
                                         * performed. */
static int poke_xmin;                   /* Minimum column extent of poke   */
static int poke_xmax;                   /* Maximum column extent of poke   */
static int poke_ymin;                   /* Minimum row extent of poke      */
static int poke_ymax;                   /* Maximum row extent of poke      */

/*
 * Initialize the data structures used by the display code. The edge vectors
 * used to access the screens are set up. The operating system's terminal I/O
 * channel is set up. All the other things get initialized at compile time.
 * The original window has "WFCHG" set, so that it will get completely
 * redrawn on the first call to "update".
 */
static meSTYLE defScheme [meSCHEME_STYLES*2] =
{
    meSTYLE_NDEFAULT,meSTYLE_RDEFAULT,            /* Normal style */
    meSTYLE_NDEFAULT,meSTYLE_RDEFAULT,            /* Current style */
    meSTYLE_NDEFAULT,meSTYLE_RDEFAULT,            /* Region style */
    meSTYLE_NDEFAULT,meSTYLE_RDEFAULT,            /* Current region style */
    meSTYLE_RDEFAULT,meSTYLE_NDEFAULT,            /* Normal style */
    meSTYLE_RDEFAULT,meSTYLE_NDEFAULT,            /* Current style */
    meSTYLE_RDEFAULT,meSTYLE_NDEFAULT,            /* Region style */
    meSTYLE_RDEFAULT,meSTYLE_NDEFAULT,            /* Current region style */
};

void
vtinit(void)
{
    FRAMELINE *flp;                     /* Frame store line pointer */
    int ii, jj ;                        /* Local loop counters */

    if (TTstart() == FALSE)             /* Started ?? */
        meExit (1);                     /* No quit */
    
    /* add 2 to hilBlockS to allow for a double trunc-scheme change */
    if(((styleTable = malloc(2*meSCHEME_STYLES)) == NULL) ||
       ((vvideo.video = malloc((TTmrow)*sizeof(VIDEO))) == NULL) ||
       ((hilBlock = malloc((hilBlockS+2)*sizeof(HILBLOCK))) == NULL) ||
       ((disLineBuff = malloc((disLineSize+32)*sizeof(uint8))) == NULL) ||
       ((mline   = malloc(sizeof(LINE)+TTmcol)) == NULL) ||
       ((mlStore = malloc(TTmcol+1)) == NULL))
        meExit(1);

    memcpy(styleTable,defScheme,2*meSCHEME_STYLES);

    /* Initialise the virtual video structure. Allocate and reset to
     * zero. */
    memset(vvideo.video,0,TTmrow*sizeof(VIDEO)) ;
    memset(mline,0,sizeof(LINE)) ;
    vvideo.video [TTnrow].flag = VFMESSL ;
    vvideo.video [TTnrow].line = mline ;
    /* Reset the video list block */
    vvideo.next = NULL;                 /* No next block */
    vvideo.window = NULL;               /* No windows attached */

    /* Set the fore and back colours */
    TTaddColor(meCOLOR_FDEFAULT,255,255,255) ;
    TTaddColor(meCOLOR_BDEFAULT,0,0,0) ;

    /* Set up the size of the line */
    mline->l_size = TTncol;             /* Set the length of the line */
    mlStore [0] = '\0';                 /* Reset the length of the store */

    /* Frame Store storage
     * The frame store hold's 'n' lines of video information.
     * The frameStore is an array of points to FRAMELINE structures
     * which hold the physical line contents.
     *
     * Allocate lines in the frame store to hold the video information.
     */
    if ((frameStore = (FRAMELINE *) meMalloc (sizeof (FRAMELINE) * TTmrow)) == NULL)
        meExit (1);
    for (flp = frameStore, ii = 0; ii < TTmrow; ii++, flp++)
    {
        if ((flp->scheme = meMalloc(TTmcol*(sizeof(uint8)+sizeof(meSTYLE)))) == NULL)
            meExit (1);
        flp->text = (uint8 *) (flp->scheme+TTmcol) ;
        /* Fill with data */
        jj=TTmcol ;
        while(--jj >= 0)
        {
            flp->text[jj] = ' ' ;
            flp->scheme[jj] = meSCHEME_NDEFAULT ;
        }
    }

    /* Some windowing systems require a 2 stage start. Kick off the second
     * stage of the start up process after the display memory has been
     * initialised */
    TTstartStage2();
}


/*
 * convertSchemeToStyle
 * Validate a hilighting colour.
 */
int
convertUserScheme(int n, int defaultStyle)
{
    if ((n < 0) || (n >= styleTableSize))
    {
        mlwrite (MWABORT|MWPAUSE,(uint8 *)"[Invalid color-scheme index %d]", n);
        return defaultStyle ;
    }
    return (n*meSCHEME_STYLES) ;
}

/*
 * Apply selection hilighting to the window
 */
static void
shilightWindow(WINDOW *wp)
{
    int32 lineNo;           /* Toprow line number */

    /* Check to see if any selection hilighting is to be removed. */
    if(((selhilight.flags & SELHIL_ACTIVE) == 0) ||
       (wp->w_bufp != selhilight.bp))
        goto remove_hilight;

    if((wp->w_flag & (WFSELHIL|WFREDRAW)) == 0)
        return ;

    /* Hilight selection has been modified. Work out the start and
     * end points
     */
    if (selhilight.flags & SELHIL_CHANGED)
    {
        selhilight.flags &= ~(SELHIL_CHANGED|SELHIL_SAME);

        /* dot ahead of mark line */
        if (selhilight.dlineno > selhilight.mlineno)
        {
            selhilight.sline = selhilight.mlineno;
            selhilight.eline = selhilight.dlineno;
            selhilight.soff = selhilight.mlineoff;
            selhilight.eoff = selhilight.dlineoff;
        }
        /* dot behind mark line */
        else if (selhilight.dlineno < selhilight.mlineno)
        {
            selhilight.sline = selhilight.dlineno;
            selhilight.eline = selhilight.mlineno;
            selhilight.soff = selhilight.dlineoff;
            selhilight.eoff = selhilight.mlineoff;
        }
        /* dot and mark on same line */
        else
        {
            selhilight.sline = selhilight.mlineno;
            selhilight.eline = selhilight.mlineno;
            /* dot ahead of mark */
            if (selhilight.mlineoff > selhilight.dlineoff)
            {
                selhilight.soff = selhilight.dlineoff;
                selhilight.eoff = selhilight.mlineoff;
            }
            /* dot behind mark line */
            else if (selhilight.mlineoff < selhilight.dlineoff)
            {
                selhilight.soff = selhilight.mlineoff;
                selhilight.eoff = selhilight.dlineoff;
            }
            /* dot and mark on same offset */
            else
                selhilight.flags |= SELHIL_SAME;
        }
    }

    lineNo = wp->topLineNo ;
    if ((selhilight.flags & SELHIL_SAME) ||
        (lineNo > selhilight.eline) ||
        (lineNo+wp->numTxtRows-1 < selhilight.sline))
    {
remove_hilight:
        if(wp->w_flag & WFSELDRAWN)
        {
            VIDEO  *vptr;                     /* Pointer to the video block */
            uint16  sline, eline ;            /* physical screen line to update */

            /* Determine the video line position and determine the video block that
             * is being used. */
            vptr = wp->w_vvideo->video;       /* Video block */
            sline = wp->firstRow;             /* Start line */
            eline = sline + wp->numTxtRows ;  /* End line */

            while (sline < eline)
            {
                uint16 vflag;

                if ((vflag = vptr[sline].flag) & VFSHMSK)
                    vptr[sline].flag = (vflag & ~VFSHMSK)|VFCHNGD;
                sline++;
            }
            wp->w_flag = (wp->w_flag & ~WFSELDRAWN) | WFMAIN ;
        }
    }
    else
    {
        VIDEO          *vptr;             /* Pointer to the video block */
        uint16  sline, eline ;    /* physical screen line to update */

        /* Determine the video line position and determine the video block that
         * is being used. */
        vptr = wp->w_vvideo->video;       /* Video block */
        sline = wp->firstRow;             /* Start line */
        eline = sline + wp->numTxtRows ;  /* End line */

        /* Mark all of the video lines with the update. */
        while (sline < eline)
        {
            uint16 vflag;

            vflag = vptr[sline].flag;

            if ((lineNo < selhilight.sline) || (lineNo > selhilight.eline))
            {
                if (vflag & VFSHMSK)
                    vptr[sline].flag = (vflag & ~VFSHMSK)|VFCHNGD;
            }
            else
            {
                if (lineNo == selhilight.sline)
                {
                    if (lineNo == selhilight.eline)
                        vptr[sline].flag = (vflag & ~VFSHMSK)|(VFSHBEG|VFSHEND|VFCHNGD);
                    else
                        vptr[sline].flag = (vflag & ~VFSHMSK)|(VFSHBEG|VFCHNGD);
                }
                else if (lineNo == selhilight.eline)
                    vptr[sline].flag = (vflag & ~VFSHMSK)|(VFSHEND|VFCHNGD);
                else if ((vflag & VFSHALL) == 0)
                    vptr[sline].flag = (vflag & ~VFSHMSK)|(VFSHALL|VFCHNGD);
            }
            /* on to the next one */
            sline++;                        /* Next video line */
            lineNo++;                       /* Next line number */
        }
        wp->w_flag |= (WFSELDRAWN|WFMAIN) ;
    }
}


int
showCursor(int f, int n)
{
    int ii ;

    ii = cursorState ;
    if(f)
        cursorState += n ;
    else
        cursorState = 0 ;
    if((cursorState >= 0) && (ii < 0))
    {
        if(cursorBlink)
            /* kick off the timer */
            TThandleBlink(1) ;
        else
            TTshowCur() ;
    }
    else if((cursorState < 0) && (ii >= 0))
        TThideCur() ;
    return TRUE ;
}

int
showRegion(int f, int n)
{
    int absn ;

    absn = (n < 0) ? -n:n ;

    if((absn == 1) || (absn == 2))
    {
        if(!(selhilight.flags & SELHIL_FIXED))
            return mlwrite(MWABORT|MWCLEXEC,(uint8 *)"[No current region]") ;
        if(absn == 2)
        {
            if(selhilight.bp != curbp)
                return mlwrite(MWABORT|MWCLEXEC,(uint8 *)"[Current region not in this buffer]") ;
            curwp->w_flag |= WFMOVEL ;
            if(n == 2)
            {
                n = gotoLine(TRUE,selhilight.dlineno+1) ;
                f = selhilight.dlineoff ;
            }
            else
            {
                n = gotoLine(TRUE,selhilight.mlineno+1) ;
                f = selhilight.mlineoff ;
            }
            if((n == TRUE) && (f <= llength(curwp->w_dotp)))
                curwp->w_doto = (uint16) f ;
            return n ;
        }
    }
    switch(n)
    {
    case -3:
        selhilight.bp = curbp;                  /* Select the current buffer */
        selhilight.flags = SELHIL_ACTIVE|SELHIL_CHANGED ;
        selhilight.mlineoff = curwp->w_doto;   /* Current mark offset */
        selhilight.mlineno = curwp->line_no;    /* Current mark line number */
        selhilight.dlineoff = curwp->w_doto;   /* Current mark offset */
        selhilight.dlineno = curwp->line_no;    /* Current mark line number */
        break ;
    case -1:
        selhilight.flags &= ~SELHIL_ACTIVE ;
        break ;
    case 0:        /* Show status of hilighting */
        n = 0 ;
        if(selhilight.flags & SELHIL_ACTIVE)
            n |= 1 ;
        if(selhilight.flags & SELHIL_FIXED)
            n |= 2 ;
        if(selhilight.bp == curbp)
        {
            n |= 4 ;
            /* check for d >= dot > m */
            if(((selhilight.dlineno < curwp->line_no) ||
                ((selhilight.dlineno == curwp->line_no) && (selhilight.dlineoff <= curwp->w_doto))) &&
               ((selhilight.mlineno > curwp->line_no) ||
                ((selhilight.mlineno == curwp->line_no) && (selhilight.mlineoff >  curwp->w_doto))))
                n |= 8 ;
            /* check for m >= dot > d */
            else if(((selhilight.mlineno < curwp->line_no) ||
                     ((selhilight.mlineno == curwp->line_no) && (selhilight.mlineoff <= curwp->w_doto))) &&
                    ((selhilight.dlineno > curwp->line_no) ||
                     ((selhilight.dlineno == curwp->line_no) && (selhilight.dlineoff >  curwp->w_doto))))
                n |= 8 ;
        }
        sprintf((char *)resultStr,"%d",n) ;
        return TRUE ;

    case  1:
        selhilight.flags |= SELHIL_ACTIVE;
        if(f || (selhilight.uFlags & SELHILU_KEEP))
            selhilight.flags |= SELHIL_KEEP ;
        break ;
    case 3:
        if(((selhilight.flags & (SELHIL_FIXED|SELHIL_ACTIVE)) == 0) ||
           (selhilight.bp != curbp))
        {
            selhilight.bp = curbp ;
            selhilight.mlineoff = curwp->w_doto;   /* Current mark offset */
            selhilight.mlineno = curwp->line_no;    /* Current mark line number */
        }
        selhilight.flags |= SELHIL_FIXED|SELHIL_CHANGED|SELHIL_ACTIVE ;
        if(selhilight.uFlags & SELHILU_KEEP)
            selhilight.flags |= SELHIL_KEEP ;
        selhilight.dlineoff = curwp->w_doto;   /* Current mark offset */
        selhilight.dlineno = curwp->line_no;    /* Current mark line number */
        break ;
    default:
        return ABORT ;
    }
    addModeToBufferWindows(selhilight.bp, WFSELHIL);
    return TRUE ;
}

void
windCurLineOffsetEval(WINDOW *wp)
{
    if((wp->curLineOff->l_fp == wp->w_dotp) &&
       !(wp->w_dotp->l_flag & LNCHNG))
        return ;
    if(wp->curLineOff->l_size < wp->w_dotp->l_size)
    {
        meFree(wp->curLineOff) ;
        wp->curLineOff = lalloc(wp->w_dotp->l_size) ;
    }
    /* store dotp as the last line done */
    wp->curLineOff->l_fp = wp->w_dotp ;
#if HILIGHT
    if((wp->w_bufp->hiLight != 0) &&
       (hilights[wp->w_bufp->hiLight]->type & HFRPLCDIFF))
        hilightCurLineOffsetEval(wp) ;
    else
#endif
    {
        register uint8 cc, *ss, *off ;
        register int pos, ii ;

        ss = wp->w_dotp->l_text ;
        off = wp->curLineOff->l_text ;
        pos = 0 ;
#ifndef NDEBUG
        if(wp->w_dotp->l_text[wp->w_dotp->l_used] != '\0')
        {
            _meAssert(__FILE__,__LINE__) ;
            wp->w_dotp->l_text[wp->w_dotp->l_used] = '\0' ;
        }
#endif
        while((cc=*ss++) != 0)
        {
            if(isDisplayable(cc))
                ii = 1 ;
            else if(cc == TAB)
                ii = get_tab_pos(pos) + 1;
            else if (cc < 0x20)
                ii = 2 ;
            else
                ii = 4 ;
            *off++ = (uint8) ii ;
            pos += ii ;
        }
        *off = 0 ;
    }
}

void
updCursor(register WINDOW *wp)
{
    register uint32 ii, jj ;
    int leftMargin;                     /* Base left margin position (scrolled) */

    if(wp->w_flag & (WFREDRAW|WFRESIZE))
        /* reset the curLineOff to force a recalc when needed */
        wp->curLineOff->l_fp = NULL ;

    /* set ii to cursors screen col & jj to the require screen scroll */
    if(wp->w_doto == 0)
    {
        /* simple express case when offset is zero */
        ii = 0 ;
        jj = 0 ;
        /* must reset the curLineOff if its changed cos the flag will be lost
         * for next time! Nasty bug!! Must check the line is still dotp if we
         * are to keep it cos it could have been freed.
         */
        if((wp->curLineOff->l_fp != wp->w_dotp) || (wp->w_dotp->l_flag & LNCHNG))
            wp->curLineOff->l_fp = NULL ;
    }
    else
    {
        register uint8 *off ;

        windCurLineOffsetEval(wp) ;
        jj = 0 ;
        ii = wp->w_doto ;
        off = wp->curLineOff->l_text ;
        while(ii--)
            jj += *off++ ;

        /* jj - is the character offset of the cursor on the screen */
        if((scrollFlag & 0x0f) == 3)
            leftMargin = wp->w_sscroll ;
        else
            leftMargin = wp->w_scscroll;

        if(((int) jj) <= leftMargin)
        {
            /* Cursor behind the current scroll position */
            ii = wp->numTxtCols - (((leftMargin - jj) % wp->w_scrsiz) + wp->w_margin);
            if (ii > jj)                   /* Make sure we have enough info to show */
                ii = jj;                   /* No - shoe the start of the line */
            jj -= ii;                      /* Correct the screen offset position */
        }
        else if (((int) jj) >= leftMargin + (wp->numTxtCols-1))
        {
            /* Cursor ahead of the scroll position and off screen */
            /* Move onto the next screen */
            ii = ((jj - leftMargin - wp->numTxtCols) % wp->w_scrsiz) + wp->w_margin;
            jj -= ii;                      /* Screen offset */
        }
        else
        {
            /* Cursor ahead of scroll position and on screen */
            ii = jj - leftMargin ;
            jj = leftMargin ;
        }
    }

    mwRow = wp->firstRow + (wp->line_no - wp->topLineNo) ;
    mwCol = wp->firstCol + ii ;
    if(wp->w_scscroll != (int) jj)         /* Screen scroll correct ?? */
    {
        wp->w_scscroll = (uint16) jj;      /* Scrolled line offset */
        wp->w_flag |= WFDOT ;
    }
    switch(scrollFlag & 0x0f)
    {
    case 0:
        if(wp->w_sscroll)
        {
            /* Reset scroll column */
            wp->w_sscroll = 0 ;            /* Set scroll column to base position */
            wp->w_flag |= WFREDRAW;        /* Force a screen update */
        }
        break ;
    case 1:
        if((wp->w_sscroll != (int) jj) && wp->w_sscroll)
        {
            /* Reset scroll column */
            wp->w_sscroll = 0 ;            /* Set scroll column to base position */
            wp->w_flag |= WFREDRAW;        /* Force a screen update */
        }
        break ;
    case 2:
        if(wp->w_sscroll != (int) jj)
        {
            /* Reset scroll column */
            wp->w_sscroll = (uint16) jj ;  /* Set scroll column to base position */
            wp->w_flag |= WFREDRAW;        /* Force a screen update */
        }
        break ;
        /* case 3 leaves the scroll alone */
    }
}

#define DEBUGGING 0
#if DEBUGGING
static char drawno = 'A' ;
#endif

/*
 * renderLine
 * This function renders a non-hilighted text line.
 */
int
renderLine (uint8 *s1, int len, int wid)
{
    register uint8 cc;
    register uint8 *s2 ;

    s2 = disLineBuff + wid;
    while(len-- > 0)
    {
        /* the largest character size is a tab which is user definable */
        if (wid >= disLineSize)
        {
            disLineSize += 512 ;
            disLineBuff = realloc(disLineBuff,disLineSize+32) ;
            s2 = disLineBuff + wid ;
        }
        cc = *s1++ ;
        if(isDisplayable(cc))
        {
            wid++ ;
            if(cc == ' ')
                *s2++ = displaySpace ;
            else if(cc == TAB)
                *s2++ = displayTab ;
            else
                *s2++ = cc ;
        }
        else if(cc == TAB)
        {
            int ii=get_tab_pos(wid) ;

            wid += ii+1 ;
            *s2++ = displayTab ;
            while(--ii >= 0)
                *s2++ = ' ' ;
        }
        else if(cc < 0x20)
        {
            wid += 2 ;
            *s2++ = '^' ;
            *s2++ = cc ^ 0x40 ;
        }
        else
        {
            /* Its a nasty character */
            wid += 4 ;
            *s2++ = '\\' ;
            *s2++ = 'x' ;
            *s2++ = hexdigits[cc/0x10] ;
            *s2++ = hexdigits[cc%0x10] ;
        }
    }
    return wid;
}

/* row of screen to update, virtual screen image */
static int
updateline(register int row, register VIDEO *vp1, WINDOW *window)
{
    register uint8 *s1;         /* Text line pointer */
    register uint16 flag ;      /* Video line flag */
    HILBLOCK *blkp;             /* Style change list */
    meSCHEME *fssp;             /* Frame store - colour pointer */
    uint8    *fstp;             /* Frame store - text pointer */
    uint16 noColChng;           /* Number of colour changes */
    uint16 scol;                /* Lines starting column */
    uint16 ncol;                /* Lines number of columns */

    /* Get column size/offset */
    s1 = vp1->line->l_text;
    if(window != NULL)
    {
        scol = window->firstCol ;
        ncol = window->numTxtCols ;
    }
    else
    {
        scol = 0 ;
        ncol = TTncol ;
    }

    if((flag = vp1->flag) & VFMAINL)
    {
        meSCHEME scheme;
        if(vp1->line->l_flag & LNSMASK)
        {
            scheme = window->w_bufp->lscheme[vp1->line->l_flag & LNSMASK] ;
#if HILIGHT
            /* We have to assume this line is an exception and the hilno & bracket
             * for the next line should be what this line would have been */
            if(window->w_bufp->hiLight &&
               ((vp1[1].hilno != vp1[0].hilno) || (vp1[1].bracket != vp1[0].bracket)))
            {
                vp1[1].flag |= VFCHNGD ;
                vp1[1].hilno = vp1[0].hilno ;
                vp1[1].bracket = vp1[0].bracket ;
            }
#endif
            goto hideLineJump ;
        }
#if HILIGHT
        else if(window->w_bufp->hiLight)
        {
            uint8 tempIsWordMask;

            /* The highlighting relies on the correct setting of the
             * isWordMask to find word boundaries. Context save the existing
             * word mask and restore from the buffer. Do the hilighting
             * and then restore the previous setting. */
            tempIsWordMask = isWordMask;
            isWordMask = window->w_bufp->isWordMask;
            noColChng = hilightLine(vp1) ;
            isWordMask = tempIsWordMask;

            /* Export the information from the higlighting request */
            blkp = hilBlock + 1 ;
            scheme = (meSCHEME) ((size_t) hilights[vp1->hilno]->close) ;
        }
#endif
        else
        {
            uint16 lineLen;

            scheme = window->w_bufp->scheme;
hideLineJump:
            blkp = hilBlock + 1 ;
            lineLen = vp1->line->l_used;

            if (flag & VFCURRL)
                scheme += meSCHEME_CURRENT;

            /* Determine if there is any selection hilighting on the line. */
            if (flag & VFSHMSK)
            {
                /* Check for whole line hilighted */
                if (flag & VFSHALL)
                {
                    if(lineLen > 0)
                        blkp[0].column = renderLine(s1,lineLen,0) ;
                    else
                        blkp[0].column = 0 ;
                    blkp[0].scheme = scheme + meSCHEME_SELECT;
                    noColChng = 1 ;
                }
                else
                {
                    register uint16 wid, len ;

                    /* Region hilight commences in this line */
                    if((flag & VFSHBEG) && (selhilight.soff > 0))
                    {
                        len = selhilight.soff ;
                        wid = renderLine (s1, len, 0) ;
                        blkp[0].scheme = scheme ;
                        blkp[0].column = wid ;
                        noColChng = 1 ;
                    }
                    else
                    {
                        wid = 0 ;
                        len = 0 ;
                        noColChng = 0 ;
                    }

                    /* Region hilight terminates in this line */
                    if (flag & VFSHEND)
                    {
                        if (selhilight.eoff > len)
                        {
                            /* Set up the colour change */
                            wid = renderLine(s1+len,selhilight.eoff-len,wid) ;
                            blkp[noColChng].scheme = scheme + meSCHEME_SELECT;
                            blkp[noColChng++].column = wid ;
                            len = selhilight.eoff ;
                        }
                        blkp[noColChng].scheme = scheme ;
                    }
                    else
                        /* Set up the colour change */
                        blkp[noColChng].scheme = scheme + meSCHEME_SELECT;

                    /* Render the rest of the line in the standard colour */
                    if (lineLen > len)
                        wid = renderLine(s1+len, lineLen-len,wid) ;
                    blkp[noColChng].column = wid ;
                    noColChng += 1 ;
                }
            }
            else
            {
                /* Render the rest of the line in the standard colour */
                if (lineLen > 0)
                    blkp[0].column = renderLine(s1,lineLen,0) ;
                else
                    blkp[0].column = 0 ;
                blkp[0].scheme = scheme ;
                noColChng = 1 ;
            }
            scheme = trncScheme ;
        }

        s1 = disLineBuff ;
        {
            register uint16 scroll ;

            if (flag & VFCURRL)
                scroll = window->w_scscroll ;   /* Current line scroll */
            else
                scroll = window->w_sscroll ;    /* Hard window scroll */
            if(scroll != 0)
            {
                register uint16 ii ;

                /* Line is scrolled. The effect we want is if any text at all
                 * is on the line we place a dollar at the start of the line
                 * in the last highlighting colour. If the line is empty then
                 * we do not want the dollar.
                 *
                 * Only process the line if it is not empty, this ensures that
                 * we do not insert a dollar where not required. */
                if (blkp->column > 0)
                {
                    s1 += scroll ;
                    while((blkp->column <= scroll))
                    {
                        if(noColChng == 1)
                        {
                            blkp->column = scroll+1 ;
                            break ;
                        }
                        blkp++ ;
                        noColChng-- ;
                    }

                    for(ii=0 ; ii<noColChng ; ii++)
                        blkp[ii].column -= scroll ;

                    /* set the first char to the truncate '$' and set the scheme */
                    *s1 = '$' ;
                    blkp-- ;
                    blkp[0].column = 1 ;
                    blkp[0].scheme = scheme | (blkp[1].scheme & (meSCHEME_CURRENT|meSCHEME_SELECT)) ;
                    noColChng++ ;
                }
            }
        }
        if(blkp[noColChng-1].column >= ncol)
        {
            while((noColChng > 1) && (blkp[noColChng-2].column >= (ncol-1)))
                noColChng-- ;
            blkp[noColChng-1].column = ncol-1 ;
            /* set the last char to the truncate '$' and set the scheme */
            if(meSchemeTestStyleHasFont(scheme))
            {
                /* remove the fonts as these can effect the next char which will probably be the scroll bar */
                scheme = meSchemeSetNoFont(scheme) ;
            }
            s1[ncol-1] = '$' ;
            blkp[noColChng].column = ncol ;
            blkp[noColChng].scheme = scheme | (blkp[noColChng-1].scheme & (meSCHEME_CURRENT|meSCHEME_SELECT)) ;
            noColChng++ ;
        }
        else
        {
            /* An extra space is added to the last column so that
             * the cursor will be the right colour */
            if(vp1->line != window->w_bufp->b_linep)
                s1[blkp[noColChng-1].column] = displayNewLine ;
            else
                s1[blkp[noColChng-1].column] = ' ' ;
            if(meSchemeTestStyleHasFont(blkp[noColChng-1].scheme))
            {
                /* remove the fonts as these can effect the next char which will probably be the scroll bar */
                if(((noColChng == 1) && (blkp[0].column == 0)) ||
                   ((noColChng > 1) && (blkp[noColChng-1].column == blkp[noColChng-2].column)))
                {
                    blkp[noColChng-1].column++ ;
                    blkp[noColChng-1].scheme = meSchemeSetNoFont(blkp[noColChng-1].scheme) ;
                }
                else
                {
                    blkp[noColChng].column = blkp[noColChng-1].column+1 ;
                    blkp[noColChng].scheme = meSchemeSetNoFont(blkp[noColChng-1].scheme) ;
                    noColChng++ ;
                }
            }
            else
            {
                blkp[noColChng-1].column++ ;
            }
        }
    }
    else
    {
        blkp = hilBlock ;
        blkp->column = vp1->line->l_used ;
        noColChng = 1 ;
        if(flag & VFMODEL)
            blkp->scheme = mdLnScheme + ((flag & VFCURRL) ? meSCHEME_CURRENT:meSCHEME_NORMAL);
        else if (flag & VFMENUL)        /* Menu line */
            blkp->scheme = osdScheme;
        else
            blkp->scheme = mlScheme;
    }

    /* Get the frame store colour and text pointers */
    fssp = &frameStore[row].scheme[scol] ;
    fstp = &frameStore[row].text[scol] ;

    /************************************************************************
     *
     * X-WINDOWS
     *
     ***********************************************************************/
#ifdef _XTERM
    if(!(meSystemCfg & meSYSTEM_CONSOLE))
    {
        register int ii, len, col, cno ;
        register meSCHEME scheme ;

        row = rowToClient(row) ;
        col = cno = 0 ;                 /* Virtual column start */

        if (meSystemCfg & meSYSTEM_FONTFIX)
        {
            uint8 cc, *sfstp=fstp;
            int spFlag, ccol ;
            do {
                scheme = blkp->scheme ;
                XTERMschemeSet(scheme) ;
                ii = blkp->column ;
                ii -= col;
                len = ii ;
                blkp++ ;
                ccol = col ;
                spFlag = 0 ;
                /* Maintain the frame store and copy the string into
                 * the frame store with the colour information
                 * copy a space in place of special chars, they are
                 * drawn separately after the XDraw, the spaces are
                 * replaced with the correct chars */
                while (--len >= 0)
                {
                    *fssp++ = scheme;
                    if(((cc=s1[col++]) & 0xe0) == 0)
                    {
                        cc = ' ' ;
                        spFlag++ ;
                    }
                    *fstp++ = cc ;
                }
                XTERMstringDraw(colToClient(scol+ccol),row,(char *)sfstp+ccol,ii);
                while(--spFlag >= 0)
                {
                    while (((cc=s1[ccol]) & 0xe0) != 0)
                        ccol++ ;
                    sfstp[ccol] = cc ;
                    XTERMSpecialChar(colToClient(scol+ccol),row-mecm.ascent,cc) ;
                    ccol++ ;
                }
            } while(++cno < noColChng) ;
        }
        else
        {
            do {
                scheme = blkp->scheme ;
                XTERMschemeSet(scheme) ;
                ii = blkp->column ;
                len = ii-col;
                XTERMstringDraw(colToClient(scol+col),row,(char *)s1+col,len);
                blkp++ ;

                /* Maintain the frame store and copy the string into
                 * the frame store with the colour information */
                while (--len >= 0)
                {
                    *fssp++ = scheme;
                    *fstp++ = s1[col++];
                }
            } while(++cno < noColChng) ;
        }
        if (meStyleCmpBColor(meSchemeGetStyle(vp1->eolScheme),meSchemeGetStyle(scheme)))
        {
            ii = ncol ;
            vp1->eolScheme = scheme;
        }
        else
            ii = vp1->endp ;
        vp1->endp = col ;

        if(ii > col)
        {
            char buff[MAXBUF] ;

            ii -= col ;
            cno = ii ;
            s1 = (uint8 *) buff ;
            do
            {
                *s1++ = ' ' ;           /* End fill with spaces */
                *fstp++ = ' ';         /* Frame store space fill */
                *fssp++ = scheme;      /* Frame store colour fill */
            }
            while(--cno) ;
            XTERMstringDraw(colToClient(scol+col),row,(char *)buff,ii);
        }
#if DEBUGGING
        else
            ii = 0 ;
        XTERMstringDraw(colToClient(scol+col+ii-1),row,&drawno,1);
#endif
    }
    else
#endif
    /************************************************************************
     *
     * TERMCAP
     *
     ***********************************************************************/
#ifdef _TCAP
    {
        meSCHEME scheme ;
        register int ii, col, cno ;

        TCAPmove(row,scol);	/* Go to start of line. */

        col = 0 ;
        for(cno=0 ; cno<noColChng ; cno++)
        {
            scheme = blkp->scheme ;
            TCAPschemeSet(scheme) ;

            /* Output the character in the specified colour.
             * Maintain the frame store */
            for(ii = blkp->column ; col<ii ; col++)
            {
                *fssp++ = scheme;
                *fstp++ = *s1;
                TCAPputc(*s1++) ;
            }
            blkp++;
        }
        if (meStyleCmpBColor(meSchemeGetStyle(vp1->eolScheme),meSchemeGetStyle(scheme)))
        {
            ii = ncol ;
            vp1->eolScheme = scheme ;
        }
        else
            ii = vp1->endp ;
        vp1->endp = col ;
        if((ii -= col) > 0)
        {
            /* Output the end of the line - maintain the frame store */
            while(--ii >= 0)
            {
                *fssp++ = scheme ;
                *fstp++ = ' ' ;
                TCAPputc(' ') ;
            }
        }
#if DEBUGGING
        TCAPputc(drawno) ;
#endif
        TCAPschemeReset() ;
    }

#endif /* _TCAP */
    /************************************************************************
     *
     * MS-DOS
     *
     ***********************************************************************/
#ifdef _DOS
    {
        register meSCHEME scheme ;
        register uint16 ii, len ;
        register uint8  cc ;

        ii = 0 ;
        do {
            scheme = blkp->scheme ;
            cc = TTschemeSet(scheme) ;

            /* Update the screen.
             * Maintain the frame store and copy the sting into the
             * frame store with the colour information */
            len = blkp->column ;
            for(; ii < len; ii++)
            {
                *fssp++ = scheme ;
                *fstp++ = *s1 ;
                ScreenPutChar(*s1++, cc, scol++, row);
            }
            blkp++;
        } while(--noColChng) ;

        if (meStyleCmpBColor(meSchemeGetStyle(vp1->eolScheme),meSchemeGetStyle(scheme)))
        {
            len = ncol ;
            vp1->eolScheme = scheme;
        }
        else
            len = vp1->endp ;
        vp1->endp = ii ;
        /* Update the screen for end of the line. Maintain the
         * frame store copy. */
        for( ; ii<len ; ii++)
        {
            *fssp++ = scheme;
            *fstp++ = ' ';
            ScreenPutChar(' ', cc, scol++, row);
        }
#if DEBUGGING
        ScreenPutChar(drawno,TTcolorSet(1,7),scol-1,row) ;
#endif
    }
#endif /* _DOS */
    /*************************************************************************
     *
     * MS-WINDOWS
     *
     ************************************************************************/
#ifdef _WIN32
#ifdef _WINCON
    if (meSystemCfg & meSYSTEM_CONSOLE)
    {
        register meSCHEME scheme ;
        register uint16 ii, ccol ;
        register WORD  cc ;
        register int ll ;

        ccol = 0 ;
        do {
            scheme = blkp->scheme ;
            cc = TTschemeSet(scheme) ;

            /* Update the screen.
             * Maintain the frame store and copy the sting into the
             * frame store with the colour information */
            ii = blkp->column;
            ll = ii - ccol ;
            ConsoleDrawString (s1, cc, scol+ccol, row, ll);
            ccol = ii ;
            while(--ll >= 0)
            {
                *fssp++ = scheme ;
                *fstp++ = *s1++;
            }
            blkp++;
        } while(--noColChng) ;

        if (meStyleCmpBColor(meSchemeGetStyle(vp1->eolScheme),meSchemeGetStyle(scheme)))
        {
            ii = ncol ;
            vp1->eolScheme = scheme;
        }
        else
            ii = vp1->endp ;
        vp1->endp = ccol ;
        /* Update the screen for end of the line. Maintain the
         * frame store copy. */
        if(ccol < ii)
        {
            s1 = fstp ;
            ll = ii - ccol ;
            while(--ll >= 0)
            {
                *fssp++ = scheme;
                *fstp++ = ' ';
            }
            ll = ii - ccol ;
            ConsoleDrawString (s1, cc, scol+ccol, row, ll);
#if DEBUGGING
            ccol = ii ;
#endif
        }
#if DEBUGGING
        ConsoleDrawString (&drawno, cc, scol+ccol, row, 1);
#endif
/*        TTflush() ;*/
    }
    else
#endif
    {
        register meSCHEME scheme ;
        int offset;                     /* Offset into the line */
        int len;                        /* Local line column */

        /* Iterate through the colour changes */
        len = 0;
        do {
            /* Set up the colour maps */
            offset = blkp->column - len;   /* Get width of coloured block */
            scheme = blkp->scheme;
            blkp++;                     /* Next colour pair */
            len += offset;              /* Increase line length */
            while (--offset >= 0)
            {
                *fssp++ = scheme;
                *fstp++ = *s1++;
            }
        } while (--noColChng > 0);

        /* Clear to the end of the line */
        if (meStyleCmpBColor(meSchemeGetStyle(vp1->eolScheme),meSchemeGetStyle(scheme)))
        {
            offset = ncol - len;
            vp1->eolScheme = scheme;
        }
        else if (vp1->endp < len)
            offset = 0;
        else
            offset = vp1->endp - len;
        vp1->endp = len;
        len += offset;
        
        if (offset != 0)
        {
            /* Line terminates before right margin. */
            /* Fill in the frame store */
            do
            {
                *fssp++ = scheme;
                *fstp++ = ' ';
            }
            while (--offset > 0);
        }
#if DEBUGGING
        fstp[-1] = drawno ;
#endif
        /* Invalidate the screen */
        if(flag & VFMAINL)
            /* in the main window just add the area */
            TTaddArea(row,scol,1,len) ;
        else
            /* other areas we want to set and apply */
            TTputs(row,scol,len) ;
    }
#endif

    return TRUE ;
}

/*
 * updateWindow() - update all the lines in a window on the virtual screen
 * The screen now consists of TTnrow array of VIDEO's which point to the
 * actual LINE's (dummy ones if its a modeline) the flags indicate if the
 * LINE is mess or mode or main line and if its current
 */
static void
updateWindow(WINDOW *wp)
{
    BUFFER *bp = wp->w_bufp ;
    VIDEO   *vptr;                    /* Pointer to the video block */
    register LINE *lp ;               /* Line to update */
    register int   row, nrows ;       /* physical screen line to update */
    register uint8 force ;

    force = (uint8) (wp->w_flag & (WFREDRAW|WFRESIZE)) ;
    /* Determine the video line position and determine the video block that
     * is being used. */
    row   = wp->firstRow ;
    vptr  = wp->w_vvideo->video + row ;  /* Video block */
    nrows = wp->numTxtRows ;

    /* Search down the lines, updating them */
    {
        int ii ;
        ii = (wp->line_no - wp->topLineNo) ;
        lp = wp->w_dotp ;
        while(--ii >= 0)
            lp = lback(lp) ;
    }
#if HILIGHT
    if(bp->hiLight)
    {
        if(force)
        {
            vptr->hilno   = bp->hiLight ;
            vptr->bracket = NULL ;
            wp->w_flag |= WFLOOKBK ;
        }
        if((wp->w_flag & WFLOOKBK) && !TTahead())
        {
            if(hilights[bp->hiLight]->ignore)
                hilightLookBack(wp) ;
            wp->w_flag &= ~WFLOOKBK ;
        }
    }
#endif
#ifdef _WIN32
#ifdef _WINCON
    if (!(meSystemCfg & meSYSTEM_CONSOLE))
#endif
        TTinitArea() ;
#endif
    while(--nrows >= 0)
    {
        register uint8 update=force|(vptr->flag & VFCHNGD) ;

        /*---	 and update the virtual line */
        if(lp == wp->w_dotp)
        {
            if(((vptr->flag & VFTPMSK) != (VFMAINL|VFCURRL)) ||
               (wp->w_flag & WFDOT))
                update = 1 ;
            vptr->flag = (vptr->flag & ~VFTPMSK) | VFMAINL | VFCURRL ;
        }
        else
        {
            if((vptr->flag & VFTPMSK) != VFMAINL)
                update = 1 ;
            vptr->flag = (vptr->flag & ~VFTPMSK) | VFMAINL ;
        }
        if(force)
        {
            if(force & WFRESIZE)
                vptr->endp = wp->numTxtCols ;
            vptr->wind = wp ;
            vptr->line = lp ;
#if HILIGHT
            vptr[1].hilno = bp->hiLight ;
            vptr[1].bracket = NULL ;
#endif
        }
        else if((lp->l_flag & LNCHNG) ||
                (vptr->line != lp))
        {
            vptr->line = lp ;
            update = 1 ;
        }
        if(update)
            updateline(row,vptr,wp);
        row++ ;
        vptr++ ;
        if(lp == bp->b_linep)
            break ;
        lp=lforw(lp) ;
    }

    if(nrows > 0)
    {
        /* if we are at the end */
        meSCHEME scheme = vptr[-1].eolScheme ;
        uint8 l_flag ;
        /* store the b_linep l_flag and set to 0 as it may have a $line-scheme */
        l_flag=lp->l_flag ;
        lp->l_flag = 0 ;
        for( ; (--nrows >= 0) ; row++,vptr++)
        {
            vptr->line = lp ;
            if(force || (vptr->endp > 1) || (vptr->flag & (VFCHNGD|VFCURRL)) ||
               (vptr->eolScheme != scheme))
            {
                vptr->flag = VFMAINL ;
                vptr->wind = wp ;
                if(force & WFRESIZE)
                    vptr->endp = wp->numTxtCols ;
                updateline(row,vptr,wp);
            }
        }
        lp->l_flag = l_flag ;
    }
#ifdef _WIN32
#ifdef _WINCON
    if (!(meSystemCfg & meSYSTEM_CONSOLE))
#endif
        TTapplyArea() ;
#endif
}

uint8
assessModeLine(uint8 *ml)
{
    uint8 flags=WFMODE|WFRESIZE ;
    uint8 cc ;

    while((cc = *ml++) != '\0')
    {
        if(cc == '%')
        {
            switch((cc = *ml++))
            {
            case 'l':
                flags |= WFMOVEL ;
                break ;
            case 'c':
                /* The column often changes with the line,
                 * so add that too
                 */
                flags |= WFMOVEC|WFMOVEL ;
                break ;
            }
        }
    }
    return flags ;
}

/*
 * updateModeLine()
 * Redisplay the mode line for the window pointed to by the "wp". This is the
 * only routine that has any idea of how the modeline is formatted. You can
 * change the modeline format by hacking at this routine. Called by "update"
 * any time there is a dirty window.
 */
static void
updateModeLine(WINDOW *wp)
{
    time_t          clock;		    /* Time in machine format. */
    struct tm	   *time_ptr;	            /* Pointer to time frame. */
    register uint8 *ml, *cp, *ss ;
    register uint8  cc, lchar ;
    register BUFFER *bp ;
    register int    ii ;	            /* loop index */
    register int    lineLen ;               /* Max length of the line */

    /* See if there's anything to do first */
    bp = wp->w_bufp;
    if((ml = bp->modeLineStr) == NULL)
    {
        if((wp->w_flag & modeLineFlags) == 0)
            return ;
        ml = modeLineStr ;
    }
    else if((wp->w_flag & bp->modeLineFlags) == 0)
        return ;

    lineLen = wp->numTxtCols ;              /* Max length of line. Only need to
                                             * evaluate this many characters */
    wp->model->l_used = lineLen ;

    if (wp == curwp)                        /* mark the current buffer */
        lchar = windowChars [WCMLCWSEP];    /* Typically '=' */
    else
        lchar = windowChars [WCMLIWSEP];    /* Typically '-' */

    clock = time(0);	                    /* Get system time */
    time_ptr = (struct tm *) localtime (&clock);	/* Get time frame */
    cp = wp->model->l_text ;
    while((lineLen > 0) && ((cc = *ml++) != '\0'))
    {
        if(cc == '%')
        {
            switch((cc = *ml++))
            {
            case 's':
                /* Horizontal split line character. Only displayed if split
                 * is enabled in the scroll bars */
                if (gsbarmode & WMSPLIT)
                {
                    *cp++ = windowChars [WCHSBSPLIT];
                    lineLen-- ;
                }
                break;
            case 'r':
                /* root user - unix only */
#ifdef _UNIX
                /*
                 * The idea of this nastiness is that the mode line is
                 * changed to give some visual indication when running as root.
                 */
                if(meUid == 0)
                    *cp++ = '#' ;
                else
#endif
                    *cp++ = lchar ;
                lineLen-- ;
                break ;

            case 'u':
                /* buffer changed */
                if(meModeTest(bp->b_mode,MDEDIT))        /* "*" if changed. */
                    *cp++ = windowChars [WCMLBCHNG];     /* Typically '*' */
                else if (meModeTest(bp->b_mode,MDVIEW))  /* "%" if view. */
                    *cp++ = windowChars [WCMLBVIEW];     /* Typically '%' */
                else
                    *cp++ = lchar ;
                lineLen--;
                break ;

            case 'e':
                /* add in the mode flags */
                for (ii = 0; ii < MDNUMMODES; ii++)
                    if(meModeTest(modeLineDraw,ii) &&
                       meModeTest(bp->b_mode,ii))
                    {
                        *cp++ = modeCode[ii] ;
                        if (--lineLen == 0)
                            break;
                    }
                break ;

            case 'k':
                if (kbdmode == RECORD)           /* if playing macro */
                {
                    ss = (uint8 *) "REC" ;
                    goto model_copys ;
                }
                break ;

            case 'l':
                ss = meItoa(wp->line_no+1);
                goto model_copys ;

            case 'n':
                ss = meItoa(wp->w_bufp->elineno+1);
                goto model_copys ;

            case 'c':
                ss = meItoa(wp->w_doto) ;
                goto model_copys ;

            case 'b':
                ss = bp->b_bname ;
                goto model_copys ;

            case 'f':
                if((bp->b_bname[0] == '*') || (bp->b_fname == NULL))
                    break ;
                ss = bp->b_fname ;
                if((ii=meStrlen(ss)) > lineLen)
                {
                    *cp++ = '$' ;
                    if(--lineLen == 0)
                        break ;
                    ss += ii-lineLen ;
                }
                goto model_copys ;

            case 'D':
                ss = meItoa(time_ptr->tm_mday);
                goto model_copys ;

            case 'M':
                ss = meItoa(time_ptr->tm_mon+1);
                goto model_copys ;

            case 'y':
                ss = meItoa(1900+time_ptr->tm_year);
                goto model_copys ;

            case 'Y':
                ss = meItoa(time_ptr->tm_year%100);
                goto model_2n_copy ;

            case 'h':
                ss = meItoa(time_ptr->tm_hour);
                goto model_2n_copy ;

            case 'm':
                ss = meItoa(time_ptr->tm_min);
model_2n_copy:
                if(ss[1] == '\0')
                {
                    *cp++ = '0' ;
                    if (--lineLen == 0)
                        break;
                }
model_copys:
                while((cc = *ss++) != '\0')
                {
                    *cp++ = cc ;
                    if (--lineLen == 0)
                        break;
                }
                break ;
            default:
                *cp++ = cc ;
                lineLen--;
            }
        }
        else
        {
            if(cc == windowChars [WCMLIWSEP])  /* Is it a '-' ?? */
                *cp++ = lchar ;
            else
                *cp++ = cc ;
            lineLen--;
        }
    }
    while(lineLen-- > 0)              /* Pad to full width. */
        *cp++ = lchar ;

    {
        register VIDEO *vptr ;        /* Pointer to the video block */

        /* Determine the video line position and determine the video block that
         * is being used. */
        ii = wp->firstRow + wp->numTxtRows ;
        vptr = wp->w_vvideo->video + ii ;        /* Video block */

        if(wp->w_flag & WFRESIZE)
            vptr->endp = wp->numTxtCols ;
        if(curwp == wp)
            vptr->flag = VFMODEL|VFCURRL ;
        else
            vptr->flag = VFMODEL ;
        vptr->line = wp->model ;
        updateline(ii,vptr,wp) ;
    }
}

/*	reframe: check to see if the cursor is on in the window
        and re-frame it if needed or wanted		*/

void
reframe(WINDOW *wp)
{
    register long  ii ;

    /* See if the selection hilighting is enabled for the buffer */
    if ((selhilight.flags & SELHIL_ACTIVE) &&
        (selhilight.bp == wp->w_bufp))
        wp->w_flag |= WFSELHIL;

#ifdef _IPIPES
    if(meModeTest(wp->w_bufp->b_mode,MDLOCK) &&
       (wp->w_dotp->l_flag & LNMARK))
    {
        meAMARK *ap=wp->w_bufp->b_amark ;

        /* Are we at the input line? */
        while((ap != NULL) && (ap->name != 'I'))
            ap = ap->next ;

        if((ap != NULL) &&
           (ap->line == wp->w_dotp) &&
           (ap->offs == wp->w_doto))
        {
            IPIPEBUF *ipipe ;

            /* Yes we are at the right place, find the ipipe node */
            ipipe = ipipes ;
            while(ipipe->bp != wp->w_bufp)
                ipipe = ipipe->next ;

            if((ii = ipipe->curRow) >= wp->numTxtRows)
                ii = wp->numTxtRows-1 ;
            if((ii = wp->line_no-ii) < 0)
                ii = 0 ;
            if((wp->w_flag & WFFORCE) || (ii != wp->topLineNo))
            {
                wp->topLineNo = ii ;
                /* Force the scroll box to be updated if present. */
                wp->w_flag |= WFSBOX ;
            }
            return ;
        }
    }
#endif
    /* if not a requested reframe, check for a needed one */
    if(!(wp->w_flag & WFFORCE))
    {
        ii = wp->line_no - wp->topLineNo ;
        if((ii >= 0) && (ii < wp->numTxtRows))
            return ;
    }
    /* reaching here, we need a window refresh */
    ii = wp->w_force ;

    /* how far back to reframe? */
    if(ii > 0)
    {	/* only one screen worth of lines max */
        if (--ii >= wp->numTxtRows)
            ii = wp->numTxtRows - 1;
    }
    else if(ii < 0)
    {	/* negative update???? */
        ii += wp->numTxtRows;
        if(ii < 0)
            ii = 0;
    }
    else
        ii = wp->numTxtRows / 2;

    ii = wp->line_no-ii ;
    if(ii != wp->topLineNo)
    {
        if(ii <= 0)
            wp->topLineNo = 0 ;
        else
            wp->topLineNo = ii ;
        /* Force the scroll box and lookBack to be updated if present. */
        wp->w_flag |= WFSBOX|WFLOOKBK ;
    }
}


/*
 * updateSrollBar
 * Update the scroll bar
 */
static void
updateScrollBar (WINDOW *wp)
{
    int ii;                             /* index into split string */
    int col;                            /* The column */
    int row;                            /* Current row */
    int endrow;                         /* Terminating row */
    meSCHEME scheme;                    /* Region scheme */
    uint8 *wbase;                       /* Base window character */
    int  len;                           /* Length of bar */
    int  flipBox;                       /* Flip colours for the box */

    /* Has this window got a bar present ?? */
    if (!(wp->w_mode & WMVBAR))
        return;                         /* No quit */

    /* Sort out the colours we should be using */
    scheme = sbarScheme + ((curwp == wp) ? meSCHEME_CURRENT:meSCHEME_NORMAL);

    /* Get the extents of the bar line */
    row = wp->firstRow;                 /* Start of row */
    col = wp->firstCol + wp->numTxtCols;  /* Start column */

    if (wp->w_mode & WMVWIDE)
    {
        len = 2;
        wbase = &windowChars[WCVSBSPLIT1];
    }
    else
    {
        len = 1;
        wbase = &windowChars[WCVSBSPLIT];
    }

    /* Flipbox is provided where we have not got a good reverse video character
     * for the scroll box (UNIX !!). When WMRVBOX is defined we may use a <SPACE>
     * character for the scroll box and paint in reverse video, this then makes a
     * better scroll box as required.
     *
     * Simply get the status and record the offset of the box, we will flip fcol
     * and bcol when we get there. If it is not defined then set to the end of
     * scroll bar + 1.
     */
    if (wp->w_mode & WMRVBOX)
        flipBox = 1 << (WCVSBBOX-WCVSBSPLIT) ;
    else
        flipBox = 0 ;

#ifdef _WIN32
#ifdef _WINCON
    if (!(meSystemCfg & meSYSTEM_CONSOLE))
#endif
        /* Initialize the invalid screen area */
        TTinitArea() ;
#endif /* _WIN32 */

    /* Iterate down the length of the split line */
    for (ii = 0; ii <= (WCVSBML-WCVSBSPLIT); ii++, wbase += len)
    {
        scheme = (scheme & ~1) ^ (flipBox & 1) ;  /* Select component scheme */
        endrow = wp->w_sbpos [ii] ;
        flipBox >>= 1;

        /* See if there is anything to do. */
        if ((wp->w_flag & (WFSBSPLIT << ii)) == 0)
        {
            row = endrow ;
            continue ;
        }

        /* Perform the update */
#ifdef _WIN32
#ifdef _WINCON
        if (!(meSystemCfg & meSYSTEM_CONSOLE))
#endif
            /* Invalidate the screen */
            TTaddArea (row, col, endrow-row, len);
#endif /* _WIN32 */

        for (; row < endrow; row++)
        {
            /* Update the frame store with the character and scheme
             * information - this is nearly the same for all platforms
             */
            uint8    *fstp ;      /* Frame store text pointer */
            meSCHEME *fssp ;      /* Frame store scheme pointer */

            fstp = frameStore[row].text + col ;
            fssp = frameStore[row].scheme + col ;

#ifdef _WIN32
            /***************************************************************
             *
             * Windows
             *
             **************************************************************/

#ifdef _WINCON
            if (meSystemCfg & meSYSTEM_CONSOLE)
            {
                WORD cc ;
                cc = TTschemeSet(scheme) ;
                ConsoleDrawString(wbase,cc,col,row,len) ;
            }
#endif
            fssp[0] = scheme;         /* Assign the scheme */
            fstp[0] = wbase[0];       /* Assign the text */
            if(len > 1)
            {
                fssp[1] = scheme;     /* Assign the scheme */
                fstp[1] = wbase[1];   /* Assign the text */
            }
#endif /* _WIN32 */

#ifdef _XTERM
            /***************************************************************
             *
             * X-WINDOWS
             *
             **************************************************************/
            if(!(meSystemCfg & meSYSTEM_CONSOLE))
            {
                XTERMschemeSet(scheme) ;

                if (meSystemCfg & meSYSTEM_FONTFIX)
                {
                    uint8 cc;
                    fssp[0] = scheme;  /* Assign the colour */
                    if(!((cc = *wbase) & 0xe0))
                        cc = ' ' ;    /* Assign the text */
                    fstp[0] = cc ;    /* Assign the text */

                    if(len > 1)
                    {
                        fssp[1] = scheme;  /* Assign the colour */
                        if(!((cc = wbase[1]) & 0xe0))
                            cc = ' ' ;    /* Assign the text */
                        fstp[1] = cc ;    /* Assign the text */
                    }
                    XTERMstringDraw(colToClient(col),rowToClient(row),fstp,len);
                    if(!((cc = *wbase) & 0xe0))
                    {
                        XTERMSpecialChar(colToClient(col),rowToClientTop(row),cc) ;
                        fstp[0] = cc ;   /* Assign the text */
                    }
                    if((len > 1) && !((cc = *wbase) & 0xe0))
                    {
                        XTERMSpecialChar(colToClient(col+1),rowToClientTop(row),cc) ;
                        fstp[1] = cc ;   /* Assign the text */
                    }
                }
                else
                {
                    fssp[0] = scheme ;   /* Assign the colour */
                    fstp[0] = *wbase ;   /* Assign the text */
                    if(len > 1)
                    {
                        fssp[1] = scheme ;  /* Assign the colour */
                        fstp[1] = wbase[1] ;
                    }
                    XTERMstringDraw(colToClient(col),rowToClient(row),wbase,len);
                }
            }
            else
#endif
#ifdef _TCAP
            /***************************************************************
             *
             * TERMCAP
             *
             **************************************************************/
            {
                uint8 cc;

                TCAPmove(row, col);    /* Go to correct place. */
                TCAPschemeSet(scheme) ;

                fssp[0] = scheme;      /* Assign the colour */
                cc = *wbase;
                fstp[0] = cc ;           /* Assign the text */
                TCAPputc(cc) ;
                if(len > 1)
                {
                    fssp[1] = scheme; /* Assign the colour */
                    cc = wbase[1] ;
                    fstp[1] = cc ;    /* Assign the text */
                    TCAPputc(cc) ;
                }
                TCAPschemeReset() ;
            }
#endif /* _TCAP */
#ifdef _DOS
            /***************************************************************
             *
             * MS-DOS
             *
             **************************************************************/
            {
                uint8 ss ;
                int cc ;

                cc = TTschemeSet(scheme) ;

                fssp[0] = scheme ;  /* Assign the colour */
                ss = *wbase ;
                fstp[0] = ss ;      /* Assign the text */
                ScreenPutChar(ss,cc,col,row);
                if(len > 1)
                {
                    fssp[1] = scheme ;  /* Assign the colour */
                    ss = wbase[1] ;
                    fstp[1] = ss ;      /* Assign the text */
                    ScreenPutChar(ss,cc,col+1,row);
                }
            }
#endif /* _DOS */
        }
    }
#ifdef _WIN32
#ifdef _WINCON
    if (!(meSystemCfg & meSYSTEM_CONSOLE))
#endif
        /* Apply the invalid area */
        TTapplyArea () ;
#endif /* _WIN32 */
}


/*
 * pokeUpdate()
 * Update all of the areas of the screen that have been modified.
 * This is a simple bounding box test against the extents of the
 * window. If any part of the poke screen box lies within the window
 * then the lines in the virtual video structure must be marked as
 * changed in order to refresh them.
 */
static void
pokeUpdate (void)
{
    WINDOW *wp;                         /* Window pointer */
    VIDEO  *vp1;                        /* Video pointer */

    /* Fix up the message line - special case */
    if (poke_ymax == TTnrow)
    {
        vp1 = vvideo.video + TTnrow;
        vp1->flag |= VFCHNGD;
        vp1->endp = TTncol;             /* Force update to EOL        */
    }

    /* Fix up the menu line - spacial case */
    if (poke_ymin < TTsrow)
    {
        vvideo.video->flag |= VFCHNGD;
        vvideo.video->endp = TTncol;
    }

    /* Fix up the windows */
    for (wp = wheadp; wp != NULL; wp = wp->w_wndp)
    {
        int ii, jj;                      /* Offset/Length counter */

        /* Bounding box test agains the window extents */
        if (wp->firstCol > poke_xmax)
            continue;
        if (wp->firstRow > poke_ymax)
            continue;
        if ((wp->firstCol + wp->numCols) <= poke_xmin)
            continue;
        if ((wp->firstRow + wp->numRows) <= poke_ymin)
            continue;

        /* Got here - therefore overlaps the window.
         * Determine vertical starting position on the frame.
         *
         * ii initially contains the startOffset */
        if (wp->firstRow > poke_ymin)
            ii = wp->firstRow;            /* Start at top of window */
        else
            ii = poke_ymin;             /* Start at top of poke */

        /* Get starting point og the Video frame pointer */
        vp1 = (VIDEO *)(wp->w_vvideo->video) + ii;

        /* Determine the end position on the frame and work out
         * the number of lines that need to be updated.
         *
         * ii will contain the (endOffset - startOffset) i.e the
         * number of video rows to be updated */
        if ((jj=wp->firstRow+wp->numRows-1) <= poke_ymax)
        {
            ii = jj - ii - 1 ;
            wp->w_flag |= WFMODE ;
        }
        else
            ii = poke_ymax - ii;        /* End at poke max */

        /* Iterate down the video frame until we have exhausted our
         * row count. Note that we only mark the underlying video structure
         * as changed and NOT the window contents. This ensures that the
         * video is re-drawn without checking the window flags
         */
        do
        {
            vp1->flag |= VFCHNGD;       /* Mark video line as changed */
            vp1->endp = wp->numTxtCols; /* Force update to EOL        */
            vp1++;                      /* Next line                  */
        }
        while (ii-- > 0);               /* Until exhaused line count  */

        /* flag the window as needing attension */
        wp->w_flag |= WFMAIN ;

        /* Fix up the vertical scroll bar if we have invaded it's space,
         * note that we do not modify the underlying separator bar in the
         * VVIDOE structure since it has not been re-sized. It is only
         * necessary to mark the scroll bar itself as changed */
        if ((poke_xmax >= (wp->firstCol + wp->numTxtCols)) &&
            (wp->w_mode & WMVBAR))
            wp->w_flag |= WFSBAR;
    }

    /* Reset the poke flags for next time. Set to their maximal limits
     * that will force a fail on the next poke screen operation */
    poke_xmin = TTncol;
    poke_xmax = 0;
    poke_ymin = TTnrow;
    poke_ymax = 0;
    poke_flag = 0;
}

/*
 * Make sure that the display is right. This is a three part process. First,
 * scan through all of the windows looking for dirty ones. Check the framing,
 * and refresh the screen. Second, make sure that "currow" and "curcol" are
 * correct for the current window. Third, make the virtual and physical
 * screens the same.
 */
/* screenUpdate:	user routine to force a screen update
		always finishes complete update		*/

int
screenUpdate(int f, int n)
{
    register WINDOW *wp;

#if DEBUGGING
    if(drawno++ == 'Z')
        drawno = 'A' ;
#endif
    if(n)
    {
        /* if screen is garbage, re-plot it all */
        if (TTsrow > 0)
        {
            /* Mark menu as changed */
            vvideo.video[0].flag = VFMENUL|VFCHNGD;
            vvideo.video[0].endp = TTncol;
        }
        /* Mark message line as changed */
        vvideo.video[TTnrow].flag |= VFCHNGD ;
        vvideo.video[TTnrow].endp = TTncol;
        /* Reset the poke flags */
        poke_xmin = TTncol;
        poke_xmax = 0;
        poke_ymin = TTnrow;
        poke_ymax = 0;
        poke_flag = 0;
    }
    else
    {
#if MEOSD
        if(osdDisplayHd != NULL)
            /* else we must store any osd dialogs */
            osdStoreAll() ;
#endif
        
        /* if the screen has been poked then fix it */
        if (poke_flag)                      /* Screen been poked ??           */
            pokeUpdate();                   /* Yes - determine screen changes */
    }

    /* Does the title bar need updating? */
#ifdef _WINDOW
    {
        static uint8 *lastBName=NULL ;

        if(n || (curbp->b_bname != lastBName))
        {
            lastBName = curbp->b_bname ;
            TTtitleText(curbp->b_bname) ;
        }
    }
#endif
#if MEOSD
    /* Does the menu line need updating? if so do it! */
    if((TTsrow > 0) && (vvideo.video[0].flag & VFCHNGD))
        osdMainMenuUpdate(n) ;
#endif
    /* update any windows that need refreshing */
    for (wp = wheadp; wp != NULL; wp = wp->w_wndp)
    {
        if(n)
        {
            wp->l_bufp = wp->w_bufp ;
            wp->w_flag |= WFUPGAR ;
        }
        else if(wp->l_bufp != wp->w_bufp)
        {
            wp->l_bufp = wp->w_bufp ;
            wp->w_flag |= WFMODE|WFREDRAW|WFMOVEL|WFSBOX ;
        }
        /* if top of window is the last line and there's more than
         * one, force refame and draw */
        if((wp->topLineNo == wp->w_bufp->elineno) && wp->topLineNo)
            wp->w_flag |= WFFORCE ;

        /* if the window has changed, service it */
        if(wp->w_flag & (WFMOVEL|WFFORCE))
            reframe(wp) ;	        /* check the framing */

        if(wp->w_flag & (WFREDRAW|WFRESIZE|WFSELHIL|WFSELDRAWN))
            shilightWindow(wp);         /* Update selection hilight */

        /* check the horizontal scroll and cursor position */
        if(wp == curwp)
            updCursor(wp) ;

        if(wp->w_flag & ~(WFSELDRAWN|WFLOOKBK))
        {
            /* if the window has changed, service it */
            if(wp->w_flag & WFSBOX)
                fixWindowScrollBox(wp);     /* Fix the scroll bars */
            if(wp->w_flag & ~(WFMODE|WFSBAR|WFLOOKBK))
                updateWindow(wp) ;          /* Update the window */
            updateModeLine(wp);	    /* Update mode line */
            if(wp->w_flag & WFSBAR)
                updateScrollBar(wp);        /* Update scroll bar  */
            wp->w_flag &= WFSELDRAWN ;
        }
        wp->w_force = 0;
    }

    /* If forced then sort out the message-line as well */
    if(vvideo.video[TTnrow].flag & VFCHNGD)
    {
        updateline(TTnrow,vvideo.video+TTnrow,NULL) ;
        vvideo.video[TTnrow].flag &= ~VFCHNGD ;
    }
#if MEOSD
    /* If we are in osd then update the osd menus */
    if(osdDisplayHd != NULL)
        osdRestoreAll(n) ;
#endif
    /* update the cursor and flush the buffers */
    resetCursor() ;

    TTflush();

    /* Now rest all the window line LNCHNG flags */
    for (wp = wheadp; wp != NULL; wp = wp->w_wndp)
    {
        register LINE *flp, *blp ;
        register int   ii, jj ;
        register uint8 flag ;

        ii = (wp->line_no - wp->topLineNo) ;
        jj = (wp->numTxtRows - ii) ;
        blp = flp = wp->w_dotp ;
        while(--ii >= 0)
        {
            blp = lback(blp) ;
            if((flag=blp->l_flag) & LNCHNG)
                blp->l_flag = (flag & ~LNCHNG) ;
        }
        while(--jj >= 0)
        {
            if((flag=flp->l_flag) & LNCHNG)
                flp->l_flag = (flag & ~LNCHNG) ;
            flp = lforw(flp) ;
        }
    }
    return(TRUE);
}

int
update(int flag)    /* force=TRUE update past type ahead? */
{
    register int index;
    uint32 arg ;

    if((alarmState & meALARM_PIPED) ||
       (!(flag & 0x01) && ((kbdmode == PLAY) || clexec || TTahead())))
        return TRUE ;

    if((index = decode_key(ME_SPECIAL|SKEY_redraw,&arg)) >= 0)
        execFuncHidden(ME_SPECIAL|SKEY_redraw,index,0x80000000+sgarbf) ;
    else
        screenUpdate(1,sgarbf) ;
    /* reset garbled status */
    sgarbf = FALSE ;

    return TRUE ;
}

/*
 * pokeScreen.
 * Poke some text on the screen.
 *
 * flags are a bit pattern with the following meanings
 *
 * 0x01 POKE_NOMARK - Dont remember (or mark) the poke so that at the next
 *                    update this may not be erased (depending on what upates
 *                    are required.
 * 0x02 POKE_NOFLUSH- Don't flush the output terminal, great for splatting a
 *                    lot of pokes up at once (only the last one calls a flush)
 *                    This has limited success on Xterms as they seem to flush
 *                    anyway.
 * 0x04 POKE_COLORS - If not set then fbuf and bbuf point to a single fore and
 *                    back-ground color to be used for the whole poke. If set
 *                    then a fore & back color is given per poke char.
 */
void
pokeScreen(int flags, int row, int col, uint8 *scheme,
           uint8 *str)
{
    uint8  cc, *ss ;
    meSCHEME *fssp;               /* Frame store scheme pointer */
    int len, schm, off ;

    /* Normalise to the screen rows */
    if((row < 0) || (row > TTnrow))     /* Off the screen in Y ?? */
        return ;                        /* Yes - quit */

    /* Normalise to the screen columns. Chop the string up if necessary
     * and change any bad chars to '.'s
     */
    len = 0 ;
    ss = str ;
    while((cc = *ss) != '\0')
    {
        len++ ;
        if(!isPokable(cc))
            *ss = '.' ;
        ss++ ;
    }
    if(col < 0)
    {
        if((len += col) <= 0)
            return ;
        str -= col ;
        if(flags & POKE_COLORS)
            scheme -= col ;
        col = 0 ;
    }
    else if(col >= TTncol)
        return ;
    if((col+len) >= TTncol)
        len = TTncol - col ;

    /* Sort out the poke flags. Just keep the minimal and maximal extents
     * of the change. We will sort out what needs to be updated on the
     * next update() call. */
    if((flags & POKE_NOMARK) == 0)
    {
        poke_flag |= 1;
        if (row < poke_ymin)
            poke_ymin = row;
        if (row > poke_ymax)
            poke_ymax = row;
        if (col < poke_xmin)
            poke_xmin = col;
        if ((col+len) > poke_xmax)
            poke_xmax = col+len;
    }

    /* Write the text to the frame store. Note that the colour still
     * needs to be updated. */
    memcpy(frameStore[row].text+col,str,len) ;      /* Write text in */
    fssp = frameStore[row].scheme + col ;           /* Get the scheme pointer */
    off  = (flags >> 4) & 0x07 ;

#ifdef _WIN32
#ifdef _WINCON
    if (!(meSystemCfg & meSYSTEM_CONSOLE))
#endif
        TTputs(row,col,len) ;
#endif

    /* Write to the screen */
    if(flags & POKE_COLORS)
    {
        /************************************************************************
         *
         * X-WINDOWS
         *
         ***********************************************************************/
#ifdef _XTERM
        if(!(meSystemCfg & meSYSTEM_CONSOLE))
        {
            col = colToClient(col) ;
            row = rowToClient(row) ;

            if (meSystemCfg & meSYSTEM_FONTFIX)
            {
                uint8 cc ;
                str-- ;
                while(len--)
                {
                    schm = *scheme++ ;
                    if((schm == 0xff) && ((schm = *scheme++) == 1))
                        schm = 0 ;
                    else
                        schm = meSchemeCheck(schm)*meSCHEME_STYLES ;
                    schm += off ;
                    /* Update the frame store colour */
                    *fssp++ = schm ;
                    XTERMschemeSet(schm) ;
                    if((cc=*++str) & 0xe0)
                        XTERMstringDraw(col,row,str,1);
                    else
                    {
                        static char ss[1]={' '} ;
                        XTERMstringDraw(col,row,ss,1);
                        XTERMSpecialChar(col,row-mecm.ascent,cc) ;
                    }
                    col += mecm.fwidth ;
                }
            }
            else
            {
                while(len--)
                {
                    schm = *scheme++ ;
                    if((schm == 0xff) && ((schm = *scheme++) == 1))
                        schm = 0 ;
                    else
                        schm = meSchemeCheck(schm)*meSCHEME_STYLES ;
                    schm += off ;
                    /* Update the frame store colour */
                    *fssp++ = schm ;
                    XTERMschemeSet(schm) ;
                    XTERMstringDraw(col,row,str,1);
                    str++ ;
                    col += mecm.fwidth ;
                }
            }
        }
        else
#endif
            /************************************************************************
             *
             * TERMCAP
             *
             ***********************************************************************/
#ifdef _TCAP
        {
            TCAPmove(row, col);	/* Go to correct place. */
            while(len--)
            {
                schm = *scheme++ ;
                if((schm == 0xff) && ((schm = *scheme++) == 1))
                    schm = 0 ;
                else
                    schm = meSchemeCheck(schm)*meSCHEME_STYLES ;
                schm += off ;
                *fssp++ = schm ;
                TCAPschemeSet(schm) ;
                cc = *str++ ;
                TCAPputc(cc) ;

            }
            TCAPschemeReset() ;
        }

#endif /* _TCAP */
        /************************************************************************
         *
         * MS-DOS
         *
         ***********************************************************************/
#ifdef _DOS
        {
            while(len--)
            {
                schm = *scheme++ ;
                if((schm == 0xff) && ((schm = *scheme++) == 1))
                    schm = 0 ;
                else
                    schm = meSchemeCheck(schm)*meSCHEME_STYLES ;
                schm += off ;
                *fssp++ = schm ;
                cc = TTschemeSet(schm) ;
                ScreenPutChar(*str++, cc, col++, row);
            }
        }
#endif /* _DOS */
        /************************************************************************
         *
         * MS-WINDOWS
         *
         ***********************************************************************/
#ifdef _WIN32
        {
            while(len--)
            {
                schm = *scheme++ ;
                if((schm == 0xff) && ((schm = *scheme++) == 1))
                    schm = 0 ;
                else
                    schm = meSchemeCheck(schm)*meSCHEME_STYLES ;
                schm += off ;
                /* Update the frame store colour */
                *fssp++ = schm ;
#ifdef _WINCON
                if (meSystemCfg & meSYSTEM_CONSOLE)
                {
                    WORD att ;
                    att = TTschemeSet(schm) ;
                    ConsoleDrawString(str++, att, col++, row, 1);
                }
#endif
            }
        }
#endif /* _WIN32 */
    }
    else
    {
        schm = *scheme ;
        if((schm == 0xff) && ((schm = *scheme++) == 1))
            schm = 0 ;
        else
            schm = meSchemeCheck(schm)*meSCHEME_STYLES ;
        schm += off ;

        /************************************************************************
         *
         * X-WINDOWS
         *
         ***********************************************************************/
#ifdef _XTERM
        if(!(meSystemCfg & meSYSTEM_CONSOLE))
        {
            XTERMschemeSet(schm) ;
            if (meSystemCfg & meSYSTEM_FONTFIX)
            {
                uint8 cc, *sfstp, *fstp ;
                int ii, spFlag=0 ;

                ii = len ;
                sfstp = fstp = &(frameStore[row].text[col]) ;
                row = rowToClient(row) ;
                while (--len >= 0)
                {
                    *fssp++ = schm ;
                    if(((cc=*fstp++) & 0xe0) == 0)
                    {
                        spFlag++ ;
                        fstp[-1] = ' ' ;
                    }
                }
                XTERMstringDraw(colToClient(col),row,sfstp,ii);
                ii = 0 ;
                while(--spFlag >= 0)
                {
                    while (((cc=str[ii]) & 0xe0) != 0)
                        ii++ ;
                    sfstp[ii] = cc ;
                    XTERMSpecialChar(colToClient(col+ii),row-mecm.ascent,cc) ;
                    ii++ ;
                }
            }
            else
            {
                XTERMstringDraw(colToClient(col),rowToClient(row),str,len);
                /* Update the frame store colour */
                while (--len >= 0)
                    *fssp++ = schm ;
            }
        }
        else
#endif
            /************************************************************************
             *
             * TERMCAP
             *
             ***********************************************************************/
#ifdef _TCAP
        {
            TCAPmove(row, col);	/* Go to correct place. */
            TCAPschemeSet(schm) ;

            while(len--)
            {
                cc = *str++ ;
                TCAPputc(cc) ;
                /* Update the frame store colour */
                *fssp++ = schm ;
            }
            TCAPschemeReset() ;
            /* restore cursor position */
            if((cursorState >= 0) && blinkState)
                TTshowCur() ;
            else
                TThideCur() ;
        }

#endif /* _TCAP */
        /************************************************************************
         *
         * MS-DOS
         *
         ***********************************************************************/
#ifdef _DOS
        {
            cc = TTschemeSet(schm) ;
            while(len--)
            {
                ScreenPutChar(*str++, cc, col++, row);
                /* Update the frame store colour */
                *fssp++ = schm;
            }
        }
#endif /* _DOS */
        /************************************************************************
         *
         * MS-WINDOWS
         *
         ***********************************************************************/
#ifdef _WIN32
        {
#ifdef _WINCON
            if (meSystemCfg & meSYSTEM_CONSOLE)
            {
                WORD att ;
                att = TTschemeSet(schm) ;
                ConsoleDrawString (str, att, col, row, len);
            }
#endif
            /* Update the frame store colours */
            while (--len >= 0)
                *fssp++ = schm ;
        }
#endif /* _WIN32 */
    }
    if((flags & POKE_NOFLUSH) == 0)
        TTflush() ;                         /* Force update of screen */
}

/*
 * screenPoke.
 * The macro interface to the pokeScreen function. This accepts a
 * numeric argument which is a bit map of the form passed into pokeScreen.
 * As the default argument is 1 this will set just the POKE_NOMARK flag
 * which means that the poke will be visible after an ecs x invocation of
 * screen-poke.
 */
int
screenPoke(int f, int n)
{
    int row, col ;
    uint8 fbuf[MAXBUF] ;
    uint8 sbuf[MAXBUF] ;

    if((mlreply((uint8 *)"row",0,0,fbuf,MAXBUF) != TRUE) ||
       ((row = meAtoi(fbuf)),(mlreply((uint8 *)"col",0,0,fbuf,MAXBUF) != TRUE)) ||
       ((col = meAtoi(fbuf)),(mlreply((uint8 *)"scheme",0,0,fbuf,MAXBUF) != TRUE)) ||
       (mlreply((uint8 *)"string",0,0,sbuf,MAXBUF) != TRUE))
        return FALSE ;
    if((n & POKE_COLORS) == 0)
        fbuf[0] = (uint8) meAtoi(fbuf) ;
    pokeScreen(n,row,col,fbuf,sbuf) ;
    return TRUE ;
}

/*
 * Write out a long integer, in the specified radix. Update the physical cursor
 * position.
 *
 * As usual, if the last argument is not NULL, write characters to it and
 * return the number of characters written.
 */
static	int
mlputi(long i, int r, uint8 *buf)
{
    uint8	    temp_buf [22];		/* Temporary buffer */
    register uint8 *temp = temp_buf;	/* Temporaty buffer pointer */
    register int    count=0, neg=0 ;

    *temp++ = '\0' ;
    if (i < 0)
    {	/* Number is negetive */
        i = -i;
        neg = 1 ;
    }
    do
    {
        count++;
        *temp++ = hexdigits [i%r];
    } while ((i = i/r) > 0) ;

    if(neg)
    {
        count++ ;
        *temp++ = '-';
    }

    /*---	Write the information out */

    while ((*buf++ = *--temp) != 0)
        ;
    return(count);
}

/*
 * Write a message into the message line. Keep track of the physical cursor
 * position. A small class of printf like format items is handled. Assumes the
 * stack grows down; this assumption is made by the "++" in the argument scan
 * loop. Set the "message line" flag TRUE.
 */
#ifdef _STDARG
int
mlwrite(int flags, uint8 *fmt, ...)
#else
int
mlwrite(int flags, uint8 *fmt, int arg)
#endif
{
#ifdef _STDARG
    va_list	 ap;
#else
    register char *ap;
#endif
    register uint8 c;
    register VIDEO *vp1;
    register int offset ;	/* offset into ???		*/
    uint8  mlw[MAXBUF];		/* what we want to be there	*/
    uint8 *mlwant;		/* what we want to be there	*/
    uint8 *s1 ;

    if((alarmState & meALARM_PIPED) ||
       (clexec && (flags & MWCLEXEC)))
        goto mlwrite_exit ;

    if(mlStatus & MLSTATUS_KEEP)
    {
        uint8 *_ss, *_dd ;
        _ss = mline->l_text ;
        _dd = mlStore ;
        while((*_dd++ = *_ss++))
            ;
        mlStoreCol = mlCol ;
        mlStatus = (mlStatus & ~MLSTATUS_KEEP) | MLSTATUS_RESTORE ;
    }
    else
        mlStatus |= MLSTATUS_CLEAR ;

    if(flags & MWSPEC)
    {
        mlwant = (uint8 *) fmt ;
        offset = meStrlen(mlwant) ;
    }
    else
    {
#ifdef _STDARG
        va_start(ap,fmt);
#else
        ap = (char *) &arg;
#endif
        mlwant = mlw ;
        while ((c = *fmt++) != '\0')
        {
            if(c != '%')
                *mlwant++ = c;
            else
            {
                c = *fmt++;
                switch (c)
                {
                case 'c':
#ifdef _STDARG
                    *mlwant++ = va_arg(ap, int) ;
#else
                    *mlwant++ = *(uint8 *)ap ;
                    ap += sizeof(int);
#endif
                    break;

                case 'd':
#ifdef _STDARG
                    mlwant += mlputi((long)(va_arg(ap, int)),10,mlwant);
#else
                    mlwant += mlputi((long)(*(int *)ap), 10,mlwant);
                    ap += sizeof(int);
#endif
                    break;

                case 'o':
#ifdef _STDARG
                    mlwant += mlputi((long)(va_arg(ap, int)),8,mlwant);
#else
                    mlwant += mlputi((long)(*(int *)ap),8,mlwant);
                    ap += sizeof(int);
#endif
                    break;

                case 'x':
#ifdef _STDARG
                    mlwant += mlputi((long)(va_arg(ap, int)),16,mlwant);
#else
                    mlwant += mlputi((long)(*(int *)ap),16,mlwant);
                    ap += sizeof(int);
#endif
                    break;

                case 'D':
#ifdef _STDARG
                    mlwant += mlputi(va_arg(ap, long), 10,mlwant) ;
#else
                    mlwant += mlputi(*(long *)ap,10,mlwant) ;
                    ap += sizeof(long);
#endif
                    break;

                case 's':
#ifdef _STDARG
                    s1 = va_arg(ap, uint8 *) ;
#else
                    s1 = *(uint8 **)ap ;
                    ap += sizeof(uint8 *);
#endif
                    /* catch a string which is too long */
                    offset = meStrlen(s1) ;
                    if(offset > TTncol)
                    {
                        mlwant = mlw ;
                        s1 += offset - TTncol ;
                        offset = TTncol ;
                    }
                    meStrcpy(mlwant,s1) ;
                    mlwant += offset ;
                    break;

                default:
                    *mlwant++ = c;
                    break;
                } /* switch */
            } /* if */
        } /* while */
#ifdef _STDARG
        va_end (ap);
#endif
        *mlwant= '\0' ;
        offset = mlwant - mlw ;
        mlwant = mlw ;
    }
    vp1 = vvideo.video + TTnrow ;
    /* JDG End */
    /* SWP - changed the mlwrite when the string is to long for the ml line
     * so that it always displays the last part. This is for two reasons:
     * 1) If this is an MWSPEC then it must not touch the string, so can't
     *    null terminate.
     * 2) The latter part is usually the more important, i.e. where the
     *    prompt is
     */
    if(offset > TTncol)
    {
        mlwant += offset-TTncol ;
        offset = TTncol ;
        if(!(flags & MWUSEMLCOL))
            mlCol = TTncol-1 ;
    }
    else if(!(flags & MWUSEMLCOL))
        mlCol = offset ;
    s1 = mline->l_text ;
    while((*s1++ = *mlwant++))
        ;
    mline->l_used=offset ;
    /* JDG 02/03/97 - This line was removed, however when it is then the
     * bottom line is not reset properly.
     * SWP - I've commented it out again - this only alleviates another bug
     * which should be/should have been fixed.
     */
    /* vp1->endp = TTncol ;*/

    updateline(TTnrow,vp1,NULL);
    mlResetCursor() ;
    TTflush();

    if(flags & MWABORT)
        TTbell() ;
    if(flags & MWPAUSE)
    {
        /* Change the value of mlStatus to MLSTATUS_KEEP cos we want to keep
         * the string for the length of the sleep
         */
        uint8 oldMlStatus = mlStatus ;
        mlStatus = MLSTATUS_KEEP ;
        TTsleep(2000,1)  ;
        mlStatus = oldMlStatus ;
    }
    else if((flags & MWWAIT) || (clexec && (flags & MWCLWAIT)))
    {
        /* Change the value of mlStatus to MLSTATUS_KEEP cos we want to keep
         * the string till we get a key
         */
        uint8 scheme=(globScheme/meSCHEME_STYLES), oldMlStatus = mlStatus ;
        pokeScreen(POKE_NOMARK+0x10,TTnrow,TTncol-9,&scheme,
                   (uint8 *) "<ANY KEY>") ;
        vp1->endp = TTncol ;
        mlStatus = MLSTATUS_KEEP ;
        TTgetc() ;
        mlStatus = oldMlStatus ;
        mlerase(0) ;
    }
    if(!(flags & MWCURSOR))
        resetCursor();
    TTflush();

mlwrite_exit:
    if(!(flags & MWABORT))
        return TRUE ;

    if(macbug < -2)
        macbug = 1 ;
    return ABORT ;
}


/* mlerase relegated to here due to its use of mlwrite */
/*
 * Erase the message line. This is a special routine because the message line
 * is not considered to be part of the virtual screen. It always works
 * immediately; the terminal buffer is flushed via a call to the flusher.
 */
void
mlerase(int flag)
{
    if(clexec && (flag & MWCLEXEC))
        return ;
    if((mlStatus & MLSTATUS_RESTORE) && !(flag & MWERASE))
    {
        mlCol = mlStoreCol ;
        mlwrite(((mlStatus & MLSTATUS_POSML) ? MWSPEC|MWUSEMLCOL|MWCURSOR:MWSPEC),mlStore) ;
        mlStatus = (mlStatus & ~MLSTATUS_RESTORE) | MLSTATUS_KEEP ;
    }
    else if(!(mlStatus & MLSTATUS_KEEP))
    {
        mlwrite(MWSPEC|(flag&MWCURSOR),(uint8 *)"") ;
        mlStatus &= ~MLSTATUS_CLEAR ;
    }
}


/*
 * addColor
 * Add a new colour palett entry, or modify existing colour
 * palette entry.
 *
 * add-colour <index> <red> <green> <blue>
 *
 * Modify the colour  palette  values. The default is black (0) and white (1).
 * Any number of colours may be specified. Colors are specified as rgb values
 * in the range 0-255 where 0 is the black level; 255 is bright.
 *
 * The  <index>  need not  necesserily  commence  from zero and may exceed 256
 * entries. The support  offered by the system is dependent  upon the graphics
 * sub-system.
 */
int
addColor(int f, int n)
{
    meCOLOR index ;
    uint8 r, g, b ;
    uint8 buff[MAXBUF] ;

    if((mlreply((uint8 *)"Index",0,0,buff,MAXBUF) == ABORT) ||
       ((index = (meCOLOR) meAtoi(buff)) == meCOLOR_INVALID) ||
       (mlreply((uint8 *)"Red",0,0,buff,MAXBUF) == ABORT) ||
       ((r = (uint8) meAtoi(buff)),
        (mlreply((uint8 *)"Green",0,0,buff,MAXBUF) == ABORT)) ||
       ((g = (uint8) meAtoi(buff)),
        (mlreply((uint8 *)"Blue",0,0,buff,MAXBUF) == ABORT)))
        return FALSE ;
    b = (uint8) meAtoi(buff) ;

    n = TTaddColor(index,r,g,b) ;
    if(index == (meCOLOR) meStyleGetBColor(meSchemeGetStyle(globScheme)))
        TTsetBgcol() ;
    return n ;
}


/*
 * addColorScheme
 * Get the hilight colour definition.
 */
int
addColorScheme(int f, int n)
{
    /* Brought out as add-font also uses the same prompts */
    static uint8 *schmPromptM[4] = {(uint8 *)"Normal",(uint8 *)"Current",(uint8 *)"Select",(uint8 *)"Cur-Sel" } ;
    static uint8 *schmPromptP[4] = {(uint8 *)" fcol",(uint8 *)" bcol",(uint8 *)" nfont",(uint8 *)"rfont" } ;
    meSTYLE *hcolors ;
    meSTYLE *dcolors ;
    meCOLOR  scheme[4][4] ;
    uint8  fcol, bcol, font ;
    uint8  prompt[20] ;
    uint8  buf[MAXBUF] ;
    int index, ii, jj, col ;

    /* Get the hilight colour index */
    if ((mlreply((uint8 *)"Color scheme index",0,0,buf,MAXBUF) != TRUE) ||
        ((index = meAtoi(buf)) < 0))
        return mlwrite(MWABORT|MWPAUSE,(uint8 *)"Invalid scheme index number");

    /* See if we are replicating a color and validate the existing index */
    if ((f) && ((n > styleTableSize) || (n < 0)))
        return mlwrite (MWABORT|MWPAUSE,(uint8 *)"Cannot duplicate undefined scheme %d.", n);
    else
        dcolors = styleTable + (n*meSCHEME_STYLES);

    /* Allocate a new table if required */
    if (index >= styleTableSize)
    {
        /* Allocate a new table */
        if((styleTable = meRealloc(styleTable,(index+1)*meSCHEME_STYLES*sizeof(meSTYLE))) == NULL)
            return mlwrite (MWABORT|MWPAUSE,(uint8 *)"Cannot allocate scheme table");
        for(ii=styleTableSize ; ii<=index ; ii++)
            memcpy(styleTable+(ii*meSCHEME_STYLES),defScheme,meSCHEME_STYLES*sizeof(meSTYLE)) ;
        styleTableSize = index+1 ;
    }
    hcolors = styleTable+(index*meSCHEME_STYLES) ;

    if (f)
    {
        memcpy(hcolors,dcolors,meSCHEME_STYLES*sizeof(meSTYLE));
        return TRUE ;
    }

    /* Get the parameters from the user. */
    for (ii = 0; ii < 4; ii++)
    {
        for (jj = 0; jj < 2; jj++)
        {
            meStrcpy(prompt,schmPromptM[ii]) ;
            meStrcat(prompt,schmPromptP[jj]) ;
            if((mlreply(prompt,0,0,buf,MAXBUF) != TRUE) ||
               ((col=meAtoi(buf)) < 0) || (col >= noColors))
                return mlwrite(MWABORT|MWPAUSE,(uint8 *)"Invalid scheme entry");
            scheme[ii][jj] = col;
        }
        scheme[ii][2] = 0 ;
        scheme[ii][3] = meFONT_REVERSE ;
    }
    for (ii = 0; ii < 4; ii++)
    {
        for (jj = 2; jj < 4; jj++)
        {
            meStrcpy(prompt,schmPromptM[ii]) ;
            meStrcat(prompt,schmPromptP[jj]) ;
            if(mlreply(prompt,0,0,buf,MAXBUF) != TRUE)
                break;
            scheme[ii][jj] = ((uint8) meAtoi(buf)) & meFONT_MASK ;
        }
    }
    /* now copy the scheme into the table */
    for (ii = 0; ii < meSCHEME_STYLES; ii++)
    {
        font = scheme[ii>>1][2+(ii&0x1)] ;
        if(font & meFONT_REVERSE)
        {
            fcol = scheme[ii>>1][1] ;
            bcol = scheme[ii>>1][0] ;
        }
        else
        {
            fcol = scheme[ii>>1][0] ;
            bcol = scheme[ii>>1][1] ;
        }
        if(!(meSystemCfg & meSYSTEM_FONTS))
            /* fonts not being used */
            font = 0 ;
        /* if color is used then reverse font is done using color
         * so remove this font. If not using color then set the
         * color to the defaults */
        if(meSystemCfg &  (meSYSTEM_RGBCOLOR|meSYSTEM_ANSICOLOR|meSYSTEM_XANSICOLOR))
            font &= ~meFONT_REVERSE ;
        else
        {
            fcol = meCOLOR_FDEFAULT ;
            bcol = meCOLOR_BDEFAULT ;
        }
        meStyleSet(hcolors[ii],fcol,bcol,font) ;
    }
    if(index == globScheme)
        TTsetBgcol() ;
    return TRUE;
}


/************************** VIRTUAL VIDEO INTERFACES **************************
 *
 * vvideoDetach
 * Detach the window from the virtual video block. If the virtual video
 * block has no more windows attached then destruct the block itself.
 */

void
vvideoDetach (WINDOW *wp)
{
    VVIDEO *vvptr;
    WINDOW *twp;

    vvptr = wp->w_vvideo;               /* Get windows virtual video block */
    meAssert (vvptr != NULL);

    if ((twp = vvptr->window) == wp)    /* First window in the chain ?? */
    {
        vvptr->window = wp->w_lvideo;   /* Detach from the head */
        wp->w_vvideo = NULL;
    }
    else                                /* Not first in chain - chase block */
    {
        while (twp != NULL)
        {
            if (twp->w_lvideo == wp)
            {
                twp->w_lvideo = wp->w_lvideo;
                wp->w_vvideo = NULL;
                break;
            }
            else
                twp = twp->w_lvideo;
        }
    }

    meAssert (twp != NULL);

    /* Clean up the Virtual video frame if it is empty. Note
     * that vvptr == vvideo is handled correctly and is not
     * deleted. */
    if (vvptr->window == NULL)
    {
        VVIDEO *vv;

        for (vv = &vvideo; vv != NULL; vv = vv->next)
        {
            if (vv->next == vvptr)
            {
                vv->next = vvptr->next;
                meNullFree (vvptr->video);
                meFree (vvptr);
                break;
            }
        }
    }
}

/*
 * vvideoAttach
 * Attach a window to the virtual video block. If no virtual video block
 * is specified then construct a new block and link it into the existing
 * structure.
 */

int
vvideoAttach (VVIDEO *vvptr, WINDOW *wp)
{
    /* Construct a new virtual video block if required */
    if (vvptr == NULL)
    {
        VIDEO  *vs;                     /* New video structure */

        /* Allocate a new video structure */
        if (((vvptr = (VVIDEO *) meMalloc (sizeof (VVIDEO))) == NULL) ||
            ((vs = (VIDEO *) meMalloc(TTmrow*sizeof(VIDEO))) == NULL))
        {
            meNullFree (vvptr);
            return (FALSE);
        }

        /* Reset the strucure and chain to the existing vvideo list */
        memset (vs, 0, TTmrow * sizeof(VIDEO));
        memset (vvptr, 0, sizeof (VVIDEO));
        vvptr->video = vs;              /* Attach the video structure */
        vvptr->next = vvideo.next;      /* Chain into vvideo list */
        vvideo.next = vvptr;

        /* Reset information in the Window block */
        wp->w_lvideo = NULL;
    }

    /* Link the window to the vvideo block */
    wp->w_vvideo = vvptr;
    wp->w_lvideo = vvptr->window;
    vvptr->window = wp;
    return (TRUE);
}


/*
 * vvideoEnlarge
 * Enlarge the video frames attached to the virtual video structures.
 * Usually follows an enlargement of the screen. */
int
vvideoEnlarge (int rows)
{
    VVIDEO *vvptr;                      /* Pointer to the video blocks */
    VIDEO  *vs;                         /* Video store line */

    for (vvptr = &vvideo; vvptr != NULL; vvptr = vvptr->next)
    {
        /* Try and allocate new video frame */
        if ((vs = (VIDEO *) meMalloc (rows * sizeof (VIDEO))) == NULL)
            return (FALSE);

        /* Allocation successful. Reset the contents to zero and swap
           for the existing one */
        memset (vs, 0, rows * sizeof (VIDEO));  /* Reset to zero */
        meFree (vvptr->video);                  /* Dispose of old one */
        vvptr->video = vs;                      /* Swap in new one */
    }

    return (TRUE);
}
