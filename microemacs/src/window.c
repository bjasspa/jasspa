/*****************************************************************************
*
*       Title:          %M%
*
*       Synopsis:       Window handling routines.
*
******************************************************************************
*
*       Filename:               %P%
*
*       Author:                 Danial Lawrence
*
*       Creation Date:          10/05/91 08:27          <001002.1210>
*
*       Modification date:      %G% : %U%
*
*       Current rev:            %I%
*
*       Special Comments:
*
*       Contents Description:   Window management. Some of the functions are
*                               internal, and some are attached to keys that
*                               the user actually types.
*
*****************************************************************************
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

/*---   Include defintions */

#define __WINDOWC                       /* Define filename */

/*---   Include files */

#include "emain.h"
#include "efunc.h"
#include "esearch.h"

/*---   Local macro definitions */

#define WINDOW_NEXT   0                 /* Vertical next window list */
#define WINDOW_PREV   1                 /* Vertical previous window list */
#define WINDOW_RIGHT  2                 /* Right window list */
#define WINDOW_LEFT   3                 /* Left window list */

/*---   External references */

extern LINE   *mline ;                  /* Mode line */

/*---   Local type definitions */

/*---   Local variable definitions */

/* Window Tree Traversal Functions
 * ===============================
 *
 * The following Window Traversal Functions determine window adjacency on
 * window deletion, resizing etc. establishing adjacent windows that are
 * affected by the change in window status.
 *
 * Unfortunatly, the window management is not particullary pleasant. It
 * operates by using knowledge of the position of the window in the chain
 * of windows structures relying on the fact that whenever a window is split
 * the windows are always adjacent in the window list (take heed DO NOT
 * re-order the window list for any reason).
 *
 * Consider the following example of a massively split window:-
 *
 *
 * EXAMPLE SCREEN
 * ==============
 *
 *   +---------+-------+------+---------+--+
 *   |         |       |      |    5    |  |
 *   |         |   2   |      +---------+  |
 *   |         |       |      |    6    |  |
 *   |         +-------+   4  +---------+  |
 *   |         |       |      |    7    |  |
 *   |         |   3   |      +---------+  |
 *   |         |       |      |    8    |  |
 *   |         +-----+-+---+--+--+------+  |
 *   |         |     |     |     |      |  |
 *   |         |  9  |  10 | 11  |  12  |  |
 *   |         |     |     |     |      |  |
 *   |         |     |     |     |      |  |
 *   |         +-----+-----+--+--+------+  |
 *   |    1    |              |         |20|
 *   |         |              |         |  |
 *   |         |    13        |   16    |  |
 *   |         |              |         |  |
 *   |         |              |         |  |
 *   |         |              |         |  |
 *   |         +------+-------+--+--+---+  |
 *   |         |      |       |  |  |   |  |
 *   |         |      |       |  |  |   |  |
 *   |         | 14   |  15   |17|18|19 |  |
 *   |         |      |       |  |  |   |  |
 *   |         |      |       |  |  |   |  |
 *   |         |      |       |  |  |   |  |
 *   |         |      |       |  |  |   |  |
 *   |         |      |       |  |  |   |  |
 *   +---------+------+-------+--+--+---+--+
 *
 * The numbers in the table reflect the linkage order and do not appear
 * in the WINDOW structure itself. The key to sorting window adjacency
 * is the fact that the windows are consecutive within a column (or stripe)
 * hence we use this fact with the linkage information to determine if we
 * are dealing with a vertically or horizontally stacked window. Note the
 * case of 13/16. The window order should tell you that a single vertical
 * break exists for 13+16. The bottom lines of 13 and 16 are disarticulated.
 * Where as 9,10,11 and 12 share a horizontal line.
 *
 * We also get information from the Virtual Video structure VVIDEO. This
 * contains a thread through the WINDOW structure of all of the windows that
 * are attached to it's left. This makes it a lot easier to find left/right
 * adjacency..
 *
 * I had trouble sorting out all of the cases so if you make changes ensure
 * that you understand how the ordering is derived and interpreted.
 *
 * OPERATION EFFECTS
 * =================
 *
 *   Note that 20 was not present when this table was generated so
 *   work out values as if 1-19 were on screen.
 *
 *   +-----+------------------+------------------+------------------+
 *   |     |                  |   Grow Window    |   Grow Window    |
 *   |     |  Delete Window   |    Vertically    |   Horizontally   |
 *   +-----+------------------+------------------+------------------+
 *   | 1   | 2,3,9,13,14      | None             | 2,3,9,13,14      |
 *   | 2   | 3                | 3                | 3,4              |
 *   | 3   | 2                | 4,8,9,10,11,12   | 2,4              |
 *   | 4   | 2,3              | 3,8,9,10,11,12   | 5,6,7,8          |
 *   | 5   | 6                | 6                | 4,6,7,8          |
 *   | 6   | 5                | 7                | 4,5,7,8          |
 *   | 7   | 6                | 8                | 4,5,6,8          |
 *   | 8   | 7                | 3,4,9,10,11,12   | 4,5,6,7          |
 *   | 9   | 10               | 10,11,12,13,16   | 10               |
 *   | 10  | 9                | 9,11,12,13,16    | 11               |
 *   | 11  | 10               | 9,10,12,13,16    | 12               |
 *   | 12  | 11               | 9,10,11,13,16    | 11               |
 *   | 13  | 14,15            | 14,15            | 15,16,17         |
 *   | 14  | 15               | 13,15            | 15               |
 *   | 15  | 14               | 13,14            | 13,16,17         |
 *   | 16  | 17,18,19         | 17,18,19         | 13,15,17         |
 *   | 17  | 18               | 16,18,19         | 18               |
 *   | 18  | 17               | 16,17,19         | 19               |
 *   | 19  | 18               | 16,17,18         | 18               |
 *   +-----+------------------+------------------+------------------+
 *
 * Jon Green 27th February 1997
 */

/*
 * getAdjacentWindowList - Helper function
 * This function gets a list of all of the adjacent windows that are affected
 * by re-sizing/deleting of the window "wp" (usually the current window).
 */

static WINDOW **
getAdjacentWindowList (WINDOW **wlist, int direction, WINDOW *wp)
{
    WINDOW *twp;                        /* Temporary window pointer */
    int wlen = 0;                       /* Window list length */
    int vtarget;                        /* Vertical target line */
    int htarget;                        /* Horizontal target line */

    if (direction < WINDOW_RIGHT)
    {
        /* Windows below current window required. Our target termination
         * point is the current window (end row) point */
        if (direction == WINDOW_NEXT)
        {
            vtarget = wp->firstCol + wp->numCols;
            htarget = wp->firstRow + wp->numRows;
            if (htarget >= TTnrow)
                return (NULL);

            for (twp = wp->w_wndp; twp != NULL; twp = twp->w_wndp)
            {
                /* If the window pointer is > vtarget; gone too far */
                if (wlen == 0)
                {
                    if ((twp->firstCol + twp->numCols) > vtarget)
                        vtarget = twp->firstCol + twp->numCols;
                }
                else if (twp->firstCol >= vtarget)
                    break;

                if (twp->firstRow == htarget)     /* Horizontally aligned ?? */
                    wlist [wlen++] = twp;       /* Yes - Found one !! */
            }
        }
        /* Windows above current window required. Our target termination
         * point is the current window (start row) point. */
        else
        {
            if ((htarget = wp->firstRow) == TTsrow)
                return (NULL);

            /* Search backwards */
            for (twp = wp->w_wnup; twp != NULL; twp = twp->w_wnup)
            {
                /* If the window pointer is > htarget; gone too far */
                if ((twp->firstRow + twp->numRows) > htarget)
                    break;
                if ((twp->firstRow + twp->numRows) == htarget)
                    wlist [wlen++] = twp;   /* Found one !! */
            }
        }
    }
    else
    {
        /* Windows to the right of the current window reqired. We cheat
         * here. Because of the structure of VVIDEO it is only necessary
         * to find the horizontal window aligned with the top of the
         * window. Our target termination point is the current window
         * (start row, end col) point. */
        if (direction == WINDOW_RIGHT)
        {
            htarget = wp->firstRow;
            vtarget = wp->firstCol + wp->numCols;
            if (vtarget >= TTncol)
                return (NULL);

            for (twp = wp->w_wndp; twp != NULL; twp = twp->w_wndp)
            {
                if (twp->firstCol != vtarget)
                    continue;
                if ((twp->firstRow <= htarget) &&
                    ((twp->firstRow + twp->numRows) >= htarget))
                {
                    /* pick all of the windows off the VVIDEO structure */
                    for (twp = twp->w_vvideo->window; twp != NULL;
                         twp = twp->w_lvideo)
                        wlist [wlen++] = twp;   /* Found one !! */
                    break;
                }
            }
        }
        /* Windows to the left of the current window required. Out target
         * termination point is the (start row, start col point). */
        else
        {
            int hmax;
            int hmin;

            if (wp->firstCol == 0)
                return (NULL);

            vtarget = wp->firstCol ;
            hmax = TTsrow ;
            hmin = TTnrow+1 ;

            /* Find the horizontal range of windows to left of current window */
            for (twp = wp->w_vvideo->window; twp != NULL; twp = twp->w_lvideo)
            {
                if (twp->firstRow < hmin)
                    hmin = twp->firstRow;
                if ((twp->firstRow + twp->numRows) > hmax)
                    hmax = twp->firstRow + twp->numRows;
            }

            /* Sniff out the windows */
            for (twp = wp->w_wnup; twp != NULL; twp = twp->w_wnup)
            {
                if ((twp->firstRow >= hmin) &&
                    (twp->firstRow <= hmax) &&
                    (twp->firstCol + twp->numCols == vtarget))
                    wlist [wlen++] = twp;   /* Found one !! */
            }
        }
    }

    /* Return information to caller */
    wlist [wlen] = NULL;                /* NULL terminate the list */
    if (wlen == 0)                      /* Any found ?? */
        return (NULL);                  /* No - return NULL */
    return (wlist);
}


/*
 * fixWindowScrollBox
 * Fixes the positions of the window scroll box components. 
 */
void
fixWindowScrollBox (WINDOW *wp)
{
    int row;                            /* The starting row */
    int shaftLength;                    /* Length of scroll shaft */
    int boxLength;                      /* Length of the box */
    int bufferLength;                   /* Length of the buffer */
    int shaftMovement;                  /* Permitted movement on shaft */
    int boxPosition;                    /* Start position of box */
        
    /* Has this window got a bar present ?? */
    if ((wp->w_mode & WMSCROL) == 0)
        return;                         /* No quit */
    
    /* Sort out where the different components of the bar should be */
    row = wp->w_sbpos [WCVSBUP - WCVSBSPLIT];
    shaftLength = wp->w_sbpos [WCVSBDSHAFT - WCVSBSPLIT] - row;
    
    bufferLength = wp->w_bufp->elineno + 1;
     
    if (wp->numTxtRows < bufferLength)
    {
        boxLength = (wp->numTxtRows * shaftLength) / bufferLength;
        if (boxLength == 0)
            boxLength = 1;
        shaftMovement = shaftLength - boxLength;
        if (shaftMovement > 0)
        {
            if((boxPosition = wp->topLineNo) > 0)
            {
                boxPosition = ((boxPosition * shaftMovement) /
                               (bufferLength - wp->numTxtRows));
                if (boxPosition > shaftMovement)
                    boxPosition = shaftMovement;
            }
            row += boxPosition;
        }
    }
    else
        boxLength = shaftLength;
    
    /* Test for resize of box - this is a special case since it affects
     * both up and down shafts */
    if (boxLength != (wp->w_sbpos [WCVSBBOX - WCVSBSPLIT] - 
                      wp->w_sbpos [WCVSBUSHAFT - WCVSBSPLIT]))
    {
        /* Box changed size. Change upper and lower shafts and the box */
        wp->w_flag |= WFSBUSHAFT|WFSBBOX|WFSBDSHAFT;
        wp->w_sbpos [WCVSBUSHAFT - WCVSBSPLIT] = row;
        wp->w_sbpos [WCVSBBOX - WCVSBSPLIT] = row + boxLength;
    }
    else
    {
        /* Test upper shaft end position */
        if (wp->w_sbpos [WCVSBUSHAFT - WCVSBSPLIT] != row)
        {
            if (wp->w_sbpos [WCVSBUSHAFT - WCVSBSPLIT] < row)
                wp->w_flag |= WFSBUSHAFT|WFSBBOX;
            else
                wp->w_flag |= WFSBDSHAFT|WFSBBOX;
            wp->w_sbpos [WCVSBUSHAFT - WCVSBSPLIT] = row;
        }
        
        /* Test the box itself */
        row += boxLength;
        if (wp->w_sbpos [WCVSBBOX - WCVSBSPLIT] != row)
        {
            if (wp->w_sbpos [WCVSBBOX - WCVSBSPLIT] < row)
                wp->w_flag |= WFSBUSHAFT|WFSBBOX;
            else
                wp->w_flag |= WFSBDSHAFT|WFSBBOX;
            wp->w_sbpos [WCVSBBOX - WCVSBSPLIT] = row;
        }
    }
}

/*
 * fixWindowScrollBars
 * Fixes the positions of the window scroll bar components. 
 */
void
fixWindowScrollBars(WINDOW *wp)
{
    int row;                            /* The starting row */
    int erow;                           /* The ending row */
    int shaftLength;                    /* Length of scroll shaft */
    int mode;
    
    /* Has this window got a bar present ?? */
    if (!((mode = wp->w_mode) & WMVBAR))
        return;                         /* No quit */
    
    /* Sort out where the different components of the bar should be */
    row = wp->firstRow;                   /* Start row */
    erow = wp->firstRow + wp->numRows;     /* End row */
    shaftLength = wp->numTxtRows;
    
    wp->w_sbpos [WCVSBML-WCVSBSPLIT] = erow;
    if (mode & WMBOTTM)
        erow--;
    else
        shaftLength++;
    
    /* Split */
    if ((mode & WMSPLIT) && (wp->numRows > 4))
    {
        wp->w_sbpos [WCVSBSPLIT-WCVSBSPLIT] = ++row;
        shaftLength--;
    }
    else
        wp->w_sbpos [WCVSBSPLIT-WCVSBSPLIT] = row;
    
    /* Up Arrow */
    if ((mode & WMUP) && (wp->numRows > 2))
    {
        wp->w_sbpos [WCVSBUP-WCVSBSPLIT] = ++row;
        shaftLength--;
    }
    else
        wp->w_sbpos [WCVSBUP-WCVSBSPLIT] = row;
    
    /* Down Arrow */
    wp->w_sbpos [WCVSBDOWN-WCVSBSPLIT] = erow;
    if (mode & WMDOWN)
    {
        if (shaftLength > 0)
            erow--;
    }
    
    /* Up shaft/ box / down shaft */
    wp->w_sbpos [WCVSBUSHAFT-WCVSBSPLIT] = row;
    wp->w_sbpos [WCVSBBOX-WCVSBSPLIT] = row;
    wp->w_sbpos [WCVSBDSHAFT-WCVSBSPLIT] = erow;
    
    /* If we are scrolling, fix up the modes */
    if (mode & WMSCROL)
        fixWindowScrollBox(wp);
    wp->w_flag |= WFSBAR;
}    

/*
 * fixWindowTextSize
 * Fix the size of the window text area size following a window
 * re-size. Re-assign the global window mode.
 */
void
fixWindowTextSize(WINDOW *wp)
{
    /* Fix up the text area size */
    wp->numTxtRows = wp->numRows - 1;
    
    /* Determine and assign the scrolling mode. We need do not need
     * a vertical scroll bar if the current scroll-mode is non-scrolling
     * and the window is on the right edge of the screen */
    if(gsbarmode & WMVBAR)
        wp->w_mode = gsbarmode ;
    else if ((wp->firstCol+wp->numCols) < TTncol)
        wp->w_mode = WMVBAR ;
    else
        wp->w_mode = 0 ;
    
    if (wp->w_mode & WMVBAR)
        wp->numTxtCols = wp->numCols - ((wp->w_mode & WMVWIDE) ? 2 : 1);
    else
        wp->numTxtCols = wp->numCols;
    
    /* Work out the margins. */
    wp->w_margin = (wp->numTxtCols / 10) + 1;
    wp->w_scrsiz = wp->numTxtCols - (wp->w_margin << 1);
    /* SWP 18/8/98 - need to add WFSELDRAWN to the flag list as if
     * an another window which contains a hilighted region becomes
     * part of this window, this flag is the only way to get it
     * re-evaluated. */
    wp->w_flag |= WFRESIZE|WFMOVEL|WFSELDRAWN ;
    fixWindowScrollBars(wp);
#ifdef _IPIPES
    if(meModeTest(wp->w_bufp->b_mode,MDPIPE) &&
       ((wp == curwp) || (wp->w_bufp->b_nwnd == 1)))
        ipipeSetSize(wp,wp->w_bufp) ;
#endif        
}

/* Main Window functions */

void
makeCurWind(WINDOW *wp)
{
    /* Process the exit hook of the old buffer. 
     * Force mode line / scroll bar update on old window */    
    if(curbp->ehook >= 0)
        execBufferFunc(curbp,curbp->ehook,0,1) ;
    curwp->w_flag |= WFMODE|WFSBAR;     /* Update scroll and mode lines */
    
    /* Do the swap */
    curwp = wp;
    curbp = wp->w_bufp;
    if(isWordMask != curbp->isWordMask)
    {
        isWordMask = curbp->isWordMask ;
#if MAGIC
        mereRegexClassChanged() ;
#endif
    }
    
    /* Process the entry hook of the new buffer.
     * Force mode line / scroll bar update on new window */
    if(curbp->bhook >= 0)
        execBufferFunc(curbp,curbp->bhook,0,1) ;
    curwp->w_flag |= WFMODE|WFSBAR;
#ifdef _IPIPES
    if(meModeTest(curbp->b_mode,MDPIPE))
	ipipeSetSize(curwp,curbp) ;
#endif        
    
}

/*
 * addModeToWindows
 * Scan through and add given modes to all windows
 */
void
addModeToWindows(int mode)
{
    register WINDOW *wp ;

    wp = wheadp;
    while (wp != NULL)
    {
        wp->w_flag |= mode ;
        wp = wp->w_wndp;
    }
}

/*
 * addModeToBufferWindows
 * Scan through and add given mode to all windows showing buffer.
 */
void
addModeToBufferWindows (BUFFER *bp, int mode)
{
    register WINDOW *wp;
    
    for (wp = wheadp; wp != NULL; wp = wp->w_wndp)
        if (bp == wp->w_bufp)
            wp->w_flag |= mode;
}

/*
 * Reposition dot in the current window to line "n". If the argument is
 * positive, it is that line. If it is negative it is that line from the
 * bottom. If it is 0 the window is centered (this is what the standard
 * redisplay code does). With no argument it defaults to 0.
 * Also Refreshes the screen. Bound to "C-L".
 */
int
recenter(int f, int n)
{
    if (f == FALSE)     /* default to 0 to center screen */
        n = 0;
    curwp->w_force = n ;             /* Center */
    curwp->w_flag |= WFFORCE ;

    sgarbf = TRUE;                   /* refresh */

    return (TRUE);
}


/*
 * The command make the next window (next => down the screen) the current
 * window. There are no real errors, although the command does nothing if
 * there is only 1 window on the screen. Bound to "C-X C-N".
 *
 * with an argument this command finds the <n>th window from the top
 *
 */

int
nextWindow(int f, int n)
{
    register WINDOW *wp;

    if (f)
    {
        /* If the argument is negative, it is the nth window
         * from the bottom of the screen */
        if (n < 0)
            n = numWindows + n + 1;

        /* if an argument, give them that window from the top */
        if ((n > 0) && (n <= numWindows))
        {
            wp = wheadp;
            while (--n)
                wp = wp->w_wndp;
        }
        else if ((wp = curwp->w_wndp) == NULL)
            return (FALSE);
    }
    else if ((wp = curwp->w_wndp) == NULL)
        wp = wheadp;
    if (curwp == wp)
        return TRUE ;
    makeCurWind(wp) ;
    return (TRUE);
}

/*
 * This command makes the previous window (previous => up the screen) the
 * current window. There arn't any errors, although the command does not do a
 * lot if there is 1 window.
 */
int
prevwind(int f, int n)
{
    WINDOW *wp;

    /* if we have an argument, we mean the nth window from the bottom */
    if (f)
    {
        if (n == 0)
        {
            if ((wp = curwp->w_wnup) == NULL)
                return FALSE;
        }
        else    
            return (nextWindow(f, -n));
    }
    else if ((wp = curwp->w_wnup) == NULL)
    {
        for (wp = wheadp; wp->w_wndp != NULL; wp = wp->w_wndp)
            /* NULL */;
    }
    makeCurWind (wp);
    return (TRUE);
}

/*
 * Move the current window up by "arg" lines. Recompute the new top line of
 * the window. Look to see if "." is still on the screen. If it is, you win.
 * If it isn't, then move "." to center it in the new framing of the window
 * (this command does not really move "."; it moves the frame). Bound to
 * "C-X C-P".
 */

int
scrollUp(int f, int n)
{
    register long ii ;

    if(f == FALSE)
        n = curwp->numTxtRows-1 ;
    else if(n < 0)
        return scrollDown(f, -n) ;
    
    if(curwp->topLineNo != 0)
    {
        if((curwp->topLineNo-=n) < 0)
            curwp->topLineNo = 0 ;
        ii = curwp->line_no - curwp->topLineNo - curwp->numTxtRows + 1 ;
        if(ii > 0)
            backLine(TRUE,ii) ;
        curwp->w_flag |= (WFREDRAW|WFSBOX) ;           /* Mode line is OK. */
    }
    return TRUE ;
}

/*
 * This command moves the current window down by "arg" lines. Recompute the
 * top line in the window. The move up and move down code is almost completely
 * the same; most of the work has to do with reframing the window, and picking
 * a new dot. We share the code by having "move down" just be an interface to
 * "move up". Magic. Bound to "C-X C-N".
 */

int
scrollDown(int f, int n)
{
    register long ii ;

    if(f == FALSE)
        n = curwp->numTxtRows-1 ;
    else if(n < 0)
        return scrollUp(f, -n) ;
    /* A quick if no lines case to make rest easier */
    if(curwp->topLineNo < curbp->elineno-1)
    {
        if((curwp->topLineNo+=n) >= curbp->elineno)
            curwp->topLineNo = curbp->elineno - 1 ;
    
        ii = curwp->topLineNo - curwp->line_no ;
        if(ii > 0)
            forwLine(TRUE,ii) ;
        curwp->w_flag |= (WFREDRAW|WFSBOX) ;           /* Mode line is OK. */
    }
    return TRUE ;
}

/*
 * This command moves the window right by "arg" columns. Recompute the left
 * column in the window. 
 *
 * No argument strictly implies that the whole screen is scrolled by a screen
 * width.
 */

int
scrollRight (int f, int n)
{
    if (f == FALSE)                     /* No argument ?? */
        n = curwp->numTxtCols-2;          /* Scroll a whole screen width */

    /* To scroll simply set the current windows scroll position to the
     * appropriate start column. */
    if (n != 0)
    {
        int scroll, ii, jj, doto ;
        unsigned char *off ;
        windCurLineOffsetEval(curwp) ;
        /* try to scroll the current line by n */
        jj = llength(curwp->w_dotp) ;
        if((scrollFlag & 0x0f) > 2)
            scroll = curwp->w_sscroll ;
        else if(jj == 0)
            return TRUE ;
        else
            scroll = curwp->w_scscroll ;
        if((scroll == 0) && (n < 0))
            return TRUE ;
        scroll += n ;
        ii = scroll ;
        doto = 0 ;
        off = curwp->curLineOff->l_text ;
        while((++doto) < jj)
        {
            ii -= *off++ ;
            if(ii < 0)
                break ;
        }
        if(ii > 0)
        {
            scroll -= ii ;
            jj = 0 ;
        }
        else
            jj = -ii ;
        if(curwp->w_scscroll != scroll)
        {
            curwp->w_scscroll = scroll ;
            curwp->w_flag |= WFDOT ;
            /* Check the position of the cursor, move it if it has been 
             * scrolled off the screen.
             */
            ii = curwp->numTxtCols - 1 ;
            while(doto < curwp->w_doto)
            {
                jj += *off++ ;
                if(jj >= ii)
                    break ;
                doto++ ;
            }
            if(curwp->w_doto != doto)
            {
                curwp->w_doto = doto ;
                curwp->w_flag |= WFMOVEC ;
            }
        }
        if((scrollFlag & 0x0f) == 3)
        {
            ii = curwp->w_sscroll + n ;
            if(ii < 0)
                ii = 0 ;
            else if(ii > disLineSize)
                ii = disLineSize ;
            if(ii != curwp->w_sscroll)
            {
                curwp->w_sscroll = ii ;
                curwp->w_flag |= WFREDRAW;        /* Force complete screen refresh */
            }
        }
        else if((scrollFlag & 0x0f) && (curwp->w_sscroll != curwp->w_scscroll))
        {
            curwp->w_sscroll = curwp->w_scscroll ;
            curwp->w_flag |= WFREDRAW;        /* Force complete screen refresh */
        }
    }
    return TRUE ;
}

/*
 * This command moves the window left by "arg" columns. Recompute the left
 * column in the window. 
 *
 * No argument strictly implies that the whole screen is scrolled by a screen
 * width.
 */

int
scrollLeft (int f, int n)
{
    if (f == FALSE)                     /* No argument ?? */
        n = curwp->numTxtCols-2;          /* Scroll a whole screen width */
    return scrollRight(TRUE,-n) ;
}

/* 
 * setScrollWithMouse
 * Set the scroll with the mouse. Typically bound to the scroll box to 
 * scroll the window up or down. Useless in any other context !!
 * Scrolls "curwp" window only.
 * 
 * An argument (any argument) tells us to lock to the scroll box.
 * No argument tells us to scroll from the locked position. 
 * 
 * Fails if scroll operation fails, or scroll bars are not enabled.
 */
int
setScrollWithMouse (int f, int n)
{
    long mousePos;                      /* Curent mouse position */
    long screenTopRow;                  /* Top row of the screen */
    long currentTopRow;                 /* Current buffer top row. */
    static long startLine;              /* Starting line number (top row) */
    static long startMouse;             /* Starting mouse position */
    static long mouseRatio;             /* Ratio of mouse movement to lines */
    static long maxTopRow;              /* The maximum top row */
    
    /* Has this window got a bar present ?? */
    if ((curwp->w_mode & WMSCROL) == 0)
        return FALSE;                   /* No quit and bitch about it */
    
    /*****************************************************************************
     * 
     * LOCKING
     * 
     * If we are given an agrument then LOCK onto the start position. The start
     * position becomes our reference point for all of our scrolling work
     * 
     *****************************************************************************/
    
    if (f == TRUE)
    {
        long scrollLength;               /* Length of scroll shaft */
        long bufferLength;               /* Length of the buffer */
        
        /* Get the buffer length */
        bufferLength = (long)(curwp->w_bufp->elineno) + 1;
        
        /* If the buffer is wholly contained in the window then there is nothing
         * to do - quit now. Make sure that the top line is showing */
        if (curwp->numTxtRows >= bufferLength)
        {
            mouseRatio = -1;
            scrollUp (TRUE, bufferLength + curwp->numTxtRows);
            return TRUE;
        }
        
        /* Get the line number of the top row of the buffer. This is used as
         * the reference point for all of out scroll work */
        startLine = curwp->topLineNo ;
        /* We normalise the scroll bar if there is over-hanging
         * text by supplying a scroll at the start of the scroll */
        maxTopRow = bufferLength - curwp->numTxtRows;
        if (maxTopRow < startLine)
        {
            scrollUp(TRUE, startLine - maxTopRow);
            startLine = maxTopRow;
        }
        
        /* Remeber the starting mouse position. Keep the precision up by ustilising
         * the fractional mouse components if available. Store as a fixed point 
         * 8 bit fractional integer */
        startMouse = ((long)(mouse_Y) << 8) + (long)(mouse_dY);
        
        /* Determine how long the movable scroll region is. This is a simple
           calculation of the (scroll bar length - scroll box length) */
        scrollLength = (long)((curwp->w_sbpos [WCVSBDSHAFT - WCVSBSPLIT] - 
                               curwp->w_sbpos [WCVSBUP - WCVSBSPLIT]) - 
                              (curwp->w_sbpos [WCVSBBOX - WCVSBSPLIT] - 
                               curwp->w_sbpos [WCVSBUSHAFT - WCVSBSPLIT]));
        
        /* Build a ratio factor of number of lines to the length of the scroll. 
         * Again keep the precision by storing as a 8-bit fractional integer. */
        if (scrollLength <= 0)
            mouseRatio = -1;            /* Cannot do alot with this !! */
        else
            mouseRatio = ((long)(bufferLength - curwp->numTxtRows) << 8) / scrollLength;
        
        return TRUE;                    /* Finished locking. */
    }
    
    /*****************************************************************************
     * 
     * SCROLLING 
     * 
     * No argument. Use the locking information to determine our scroll position.
     * All information is computed using the target top line of the buffer.
     * 
     *****************************************************************************/
    
    /* if the mouse ratio is -1 then nothing to do */
    if (mouseRatio == -1)               /* Nothing to do !! */
        return TRUE;
    
    /* Compute the current mouse position. Use the fractional mouse information
     * if it is available. */
    mousePos = ((long)(mouse_Y) << 8) + (long)(mouse_dY);
    mousePos -= startMouse;             /* Delta mouse position */
    
    /* Compute the required top row position in the buffer. Take care with precision
     * here. If we are dealing with a very large buffer then reduce the accuracy of
     * the calculation to ensure that the buffer does not overflow. Biggest we can 
     * cater for here is 2^23 lines - this should be sufficient !!
     *
     * Simple compute by multiplying the delta mouse position by the ratio and add
     * to our reference start line. */
    if (mouseRatio >= (1 << 16))
        screenTopRow = startLine + ((mousePos * (mouseRatio>>8)) >> 8);
    else
        screenTopRow = startLine + ((mousePos * mouseRatio) >> 16);
    if (screenTopRow < 0)
        screenTopRow = 0;               /* Normailize incase of underflow */
    else if (screenTopRow > maxTopRow)
        screenTopRow = maxTopRow;
    
    /* Work out the top row of the buffer in the window. We use the line number of
     * the current line and the offset in the window of the current line */
    currentTopRow = curwp->topLineNo ;
    if (currentTopRow == screenTopRow)  /* Same position ?? */
        return TRUE;                    /* Nothing to do - quit now. */
    
    /* Go and scroll - simple signed difference of the current top line and the
     * target top line -  done when the scroll has finished - simple !! */
    return (scrollDown (TRUE, (int)(screenTopRow - currentTopRow)));
}
    
/*
 * This command makes the current window the only window on the screen. Bound
 * to "C-X 1". Try to set the framing so that "." does not have to move on the
 * display. Some care has to be taken to keep the values of dot and mark in
 * the buffer structures right if the distruction of a window makes a buffer
 * become undisplayed.
 */

int
onlywind(int f, int n)
{
    register WINDOW *wp, *nwp ;

    bufHistNo++ ;
    wp = wheadp ;
    while (wp != NULL)
    {
        nwp = wp->w_wndp;
        vvideoDetach (wp);               /* Detach all video blocks */
        if(wp != curwp)
        {
            if(--wp->w_bufp->b_nwnd == 0)
            {
                storeWindBSet(wp->w_bufp,wp) ;
                wp->w_bufp->histNo = bufHistNo ;
            }
            meFree(wp->model);
            meFree(wp->curLineOff);
            meFree(wp);
        }
        wp = nwp ;
    }

    meAssert (vvideo.window == NULL);
    meAssert (vvideo.next == NULL);

    /* Fix up the WINDOW pointers */
    wheadp = curwp ;                    /* Point to first window */
    numWindows = 1;                     /* Only one window displayed */
    curwp->w_wndp = NULL ;              /* Reset linkage */
    curwp->w_wnup = NULL ;
    vvideoAttach (&vvideo, curwp);      /* Attach to root video bloc */
    
    if((curwp->topLineNo -= curwp->firstRow - TTsrow) < 0)
        curwp->topLineNo = 0 ;
    
    curwp->firstRow = TTsrow;
    curwp->firstCol = 0;
    curwp->numRows = TTnrow - TTsrow;
    curwp->numCols = TTncol;
    fixWindowTextSize(curwp);
    
    return (TRUE);
}

/*
 * Delete the current window, placing its space in the window above,
 * or, if it is the top window, the window below. Bound to C-X 0.
 */

/* ARGSUSED */
int
delwind(int f, int n)
{
    WINDOW*  wp;                        /* window to recieve deleted space */
    WINDOW*  pwp;                       /* Ptr to previous curwp window  */
    WINDOW*  nwp;                       /* Ptr to next curwp window  */
    WINDOW** wlp;                       /* Window list pointer */
    WINDOW*  wlist [NWINDOWS+1];        /* The list of windows */
    int      deleteType;                /* The deltion type */

    /*
     * If we are in one window mode then bell
     */
    if(wheadp->w_wndp == NULL)
    {
        TTbell() ;
        return FALSE ;
    }

    nwp = curwp->w_wndp;
    pwp = curwp->w_wnup;
    /* Check the previous window. If the previous pointer is NULL or the
     * end column of the previous is greated then the current we delete the
     * next window
     * 
     * Cases 
     * -----
     * 
     * pwp != NULL           pwp == NULL
     * 
     * |
     * +----------+          +---------+   OR  +---------+---------+
     * |pwp       |          |curwp    |       |curwp    |nwp      |
     * |          |          |         |       |         |         | 
     * +-----+----+          +---------+       +---------+---------+
     * |curwp|               |nwp
     * |     |               |
     * +-----+
     */
    if ((pwp == NULL) ||
        ((pwp->firstCol+pwp->numCols) > (curwp->firstCol+curwp->numCols)))
    {
        /* Determine if we merge the next down or right */
        if (nwp->firstCol == curwp->firstCol)
            deleteType = WINDOW_NEXT;
        else
            deleteType = WINDOW_RIGHT;
    }
    /* If the end columns are the same as the curent window and the previous
     * window then merge the current window into the previous window
     * 
     * Cases
     * -----
     * 
     * |               OR   |
     * +----------+         +----+------+
     * |pwp       |         |    | pwp  |
     * |          |         |    |      |
     * +----------+         +----+------+
     * |curwp     |         |curwp      |
     * |          |         |           |
     * +----------+         +-----------+
     * |                    |
     */
    else if ((pwp->firstCol+pwp->numCols) == (curwp->firstCol+curwp->numCols))
        deleteType = WINDOW_PREV;
    /* The end column of the previous is smaller then the current window
     * check the next start column to determine if we merge the next sibling
     * or the previous child.
     * 
     * Cases
     * -----
     * 
     * nwp == NULL          Note that there is no above condition since that
     *                      should have been detected in the previous case.
     * +----+---------+     
     * |pwp |curwp    |
     * |    |         |
     * +----+---------+
     * 
     * nwp != NULL
     *                     OR                
     * +----+------+-----+    +----+-------+
     * |pwp |curwp |nwp  |    |pwp |curwp  |
     * |    |      |     |    |    |       |
     * +----+------+-----+    +----+-------+
     *                        |nwp         |
     *                        |            |
     *                        +------------+
     */
    else if ((nwp == NULL) ||
             (nwp->firstCol < curwp->firstCol))
        deleteType = WINDOW_LEFT;
    /*
     * The next left column is greater than the current left colunm
     * 
     * Cases 
     * -----
     *  RIGHT CASE       OR  LEFT CASE
     * +--------+------+    +---------+-----+
     * |curwp   |nwp   |    |         | nwp |  
     * |        |      |    |         |     | 
     * +--------+------+    +---+-----+     |
     *                      |   |curwp|     |
     *                      +---+     |     |
     *                      |pwp|     |     |
     *                      +---+-----+-----+
     */
    else if (nwp->firstCol > curwp->firstCol)
    {
        if (nwp->firstRow  < curwp->firstRow)
            deleteType = WINDOW_LEFT;
        else
            deleteType = WINDOW_RIGHT;
    }
    /*
     * Default to the next window.
     * 
     * Cases
     * -----
     * 
     * +-----+---------------+
     * |pwp  | curwp         |
     * |     |               |
     * |     +------+--------+
     * |     |nwp   |        |
     * |     |      |        |
     * +-----+------+--------+
     */
    else
        deleteType = WINDOW_NEXT;

    /* Perform the deletion window movement. */
    wlp = getAdjacentWindowList (wlist, deleteType, curwp);
    meAssert (wlp != NULL);

    for  (wp = *wlp; wp != NULL; wp = *++wlp)
    {
        switch (deleteType)
        {
        case WINDOW_NEXT:           /* Window vertically down */
            wp->firstRow = curwp->firstRow ;
            wp->numRows += curwp->numRows ;
            break;
        case WINDOW_RIGHT:          /* Window horizontally to right */
            wp->firstCol  = curwp->firstCol;
            wp->numCols += curwp->numCols;
            
            /* Move virtual video frames */
            vvideoDetach (wp);
            vvideoAttach (curwp->w_vvideo, wp);
            break;
        case WINDOW_PREV:           /* Window vertically up */
            wp->numRows += curwp->numRows;
            break;
        case WINDOW_LEFT:           /* Window horizontally to left */
            wp->numCols += curwp->numCols;
            break;
        }

        /* Set window flags for update - note we always set the
         * mode line. This is not always strictly necessary but
         * it is not worth working out what should and should not
         * be strictly set */
        fixWindowTextSize(wp);         /* Fix text window size */
    }

    /* Fix up the curwp pointers and get ready to delete by unchaining
     * from the window list */
    if (pwp == NULL)
        wheadp = nwp;
    else
        pwp->w_wndp = nwp;
    if (nwp != NULL)
        nwp->w_wnup = pwp;
    
    wp = curwp ;
    {
        /* Get rid of the current window */
        BUFFER *bp=wp->w_bufp ;
        if(--bp->b_nwnd == 0)
            storeWindBSet(bp,wp) ;
    }

    /* Delete the current window */
    vvideoDetach(wp);                   /* Detach from the video block */
    numWindows--;                       /* One less window displayed */

    /* Determine the next window to make the current window */
    if ((deleteType == WINDOW_LEFT) || (deleteType == WINDOW_PREV))
        makeCurWind(pwp);               /* Previous window is IT !! */
    else
        makeCurWind(nwp);               /* Next window is IT !! */
    /* Must free AFTER makeCurWind as this dicks with curwp */
    meFree(wp->model);                  /* Free off the old mode line */
    meFree(wp->curLineOff);
    meFree(wp);                         /* and finally the window itself */
    return TRUE ;
}

/*
   Split the current window.  A window smaller than 3 lines cannot be
   split.  An argument of 1 forces the cursor into the upper window, an
   argument of two forces the cursor to the lower window.  The only other
   error that is possible is a "malloc" failure allocating the structure
   for the new window.  Bound to "C-X 2".
 */
int
splitWindVert(int f, int n)
{
    register WINDOW *wp;
    register LINE   *lp, *off;
    register int    ntru, stru;
    register int    ntrl, strl;
    register int    ntrd;

    if (curwp->numTxtRows < 3)
        return mlwrite(MWABORT,(uint8 *)"Cannot split a %d line window",curwp->numTxtRows);
    if (numWindows == NWINDOWS)
        return mlwrite(MWABORT,(uint8 *)"Cannot create more than %d windows",numWindows);
    if(((wp = (WINDOW *) meMalloc(sizeof(WINDOW))) == NULL) || 
       ((lp=lalloc(TTmcol)) == NULL) || ((off=lalloc(TTmcol)) == NULL))
    {
        meNullFree (wp);                /* Destruct created window */
        return (FALSE);                 /* Fail */
    }
    memcpy(wp,curwp,sizeof(WINDOW)) ;
    
    wp->model = lp ;
    off->l_fp = NULL ;
    wp->curLineOff = off ;
    (curbp->b_nwnd)++;                  /* Displayed once more.  */
    numWindows++;                       /* One more window displayed */

    vvideoAttach (curwp->w_vvideo, wp); /* Attach to vvideo block */

    ntru = (curwp->numTxtRows-1) / 2;     /* Upper size           */
    ntrl = (curwp->numTxtRows-1) - ntru;  /* Lower size           */
    stru = (curwp->numRows)/2;           /* Upper window size    */
    strl = (curwp->numRows) - stru;      /* Lower window size    */
    ntrd = curwp->line_no - curwp->topLineNo ;
    if (((f == FALSE) && (ntru >= ntrd)) || ((f == TRUE) && (n == 1)))
    {
        /* Old is upper window. */
        if(ntrd == ntru)           /* Hit mode line.       */
            curwp->topLineNo++ ;
        curwp->numTxtRows = ntru;
        curwp->numRows = stru;

        /* Maintain the window prevous/next linkage. */
        if ((wp->w_wndp = curwp->w_wndp) != NULL)
            wp->w_wndp->w_wnup = wp;
        curwp->w_wndp = wp;
        wp->w_wnup = curwp;

        wp->firstRow = curwp->firstRow+ntru+1;
        wp->numTxtRows = ntrl;
        wp->numRows   = strl;
    }
    else
    {
        /* Old is lower window  */
        wp->firstRow = curwp->firstRow;
        wp->numTxtRows = ntru;
        wp->numRows   = stru;
        
        ++ntru;                         /* Mode line.           */
        curwp->firstRow += ntru;
        curwp->numTxtRows = ntrl;
        curwp->numRows = strl;
        curwp->topLineNo += ntru ;

        /* Maintain the window prevous/next linkage. */
        if ((wp->w_wnup = curwp->w_wnup) != NULL)
            wp->w_wnup->w_wndp = wp;
        else
            wheadp = wp;
        curwp->w_wnup = wp;
        wp->w_wndp = curwp;
    }
    
    wp->topLineNo = curwp->topLineNo ;
    fixWindowTextSize (curwp);              /* Fix text window size */
    fixWindowTextSize (wp);                 /* Fix text window size */
    return TRUE ;
}

/*
 * Split the current window horizontally.  A window smaller than 7 columns
 * cannot be split.
 *
 * An argument of 1 forces the cursor into the left window, an
 * argument of two forces the cursor to the right window.  The only other
 * error that is possible is a "malloc" failure allocating the structure
 * for the new window.
 *
 */
int
splitWindHorz(int f, int n)
{
    register WINDOW *wp;
    register LINE   *lp, *off;

    /* Out minimum split value horizontally is 7 i.e. |$c$|$c$| */
    if (curwp->numTxtCols < ((gsbarmode & WMVWIDE) ? 8 : 7))
        return mlwrite(MWABORT,(uint8 *)"Cannot split a %d column window", curwp->numTxtCols);
    if (numWindows == NWINDOWS)
        return mlwrite(MWABORT,(uint8 *)"Cannot create more than %d windows",numWindows);
    if(((wp = (WINDOW *) meMalloc(sizeof(WINDOW))) == NULL) ||
       ((lp=lalloc(TTmcol)) == NULL) || ((off=lalloc(TTmcol)) == NULL))
    {
        meNullFree (wp);                /* Destruct window */
        return (FALSE);                 /* and Fail */
    }
    memcpy(wp,curwp,sizeof(WINDOW)) ;
    if (vvideoAttach (NULL, wp) == FALSE)
    {
        meFree (lp);                    /* Destruct line */
        meFree (off);                   /* Destruct line */
        meFree (wp);                    /* Destruct window */
        return (FALSE);                 /* Failed */
    }

    /* All storage allocated. Build the new window */
    numWindows++;                       /* One more window displayed */
    wp->model = lp ;                    /* Attach mode line */
    off->l_fp = NULL ;
    wp->curLineOff = off ;              /* Attach current line offset buffer */
    (curbp->b_nwnd)++;                  /* Displayed once more.  */

    /* Maintain the window prevous/next linkage. */
    if ((wp->w_wndp = curwp->w_wndp) != NULL)
        wp->w_wndp->w_wnup = wp;
    curwp->w_wndp = wp;
    wp->w_wnup = curwp;

    /* Compute the new size of the windows and reset the width counters and
     * the margins */
    curwp->numCols /= 2;
    wp->numCols -= curwp->numCols;
    wp->firstCol  = curwp->firstCol + curwp->numCols;
    fixWindowTextSize(wp);
    fixWindowTextSize(curwp);

    if ((f != FALSE) && (n == 2))
        makeCurWind(wp);               /* Next window is IT !! */
    return (TRUE);
}

/*
 * Enlarge the current window - adds n lines to the current window.
 *
 * Find the window that loses space. Make sure it is big enough. If so,
 * hack the window descriptions, and ask redisplay to do all the hard work.
 * You don't just set "force reframe" because dot would move.
 * Bound to "C-X Z".
 */

int
growWindVert(int f, int n)
{
    WINDOW *wp;                         /* Temporary window pointer */
    WINDOW **twlp;                      /* Temporary window list pointer */
    WINDOW *bwlist [NWINDOWS+1];        /* Bottom window list */
    WINDOW *twlist [NWINDOWS+1];        /* Top window list */
    int     ii;                         /* Local loop counter */
    int     jj;                         /* Used as minimum window size */

    /* Get the resize lists - these are the windows that under-go change,
     * made a little more complicated by the presence of horizontal split
     * windows. In this case we have to consider other windows adjacent to
     * "curwp" that may undergo a change is size because of the split */
    if (getAdjacentWindowList (bwlist, WINDOW_NEXT, curwp) == NULL)
    {
        n = 0 - n;
        if (getAdjacentWindowList (twlist, WINDOW_PREV, curwp) == NULL)
            return mlwrite(MWCLEXEC|MWABORT,(uint8 *)"Only one vertical window");
        getAdjacentWindowList (bwlist, WINDOW_NEXT, twlist[0]);
    }
    else
        getAdjacentWindowList (twlist, WINDOW_PREV, bwlist[0]);

    meAssert (twlist[0] != NULL);
    meAssert (bwlist[0] != NULL);

    /* Compute the minumum size of the windows */
    if (n < 0)
    {
        twlp = twlist ;
        jj = -n ;
    }
    else
    {
        twlp = bwlist ;
        jj = n ;
    }
    for (ii = 0; (wp = twlp [ii]) != NULL; ii++)
        if (wp->numTxtRows <= jj)
            return mlwrite(MWCLEXEC|MWABORT,(uint8 *)"Impossible change");

    /* Repostion the top line in the bottom windows */
    for (jj = 0; (wp = bwlist[jj]) != NULL; jj++)
        if((wp->topLineNo+=n) < 0)
            wp->topLineNo = 0 ;

    /* Resize all of the windows BELOW curwp */
    for (ii = 0; (wp = twlist[ii]) != NULL; ii++)
    {
        wp->numRows += n;
        fixWindowTextSize (wp);
    }

    /* Resize all of the windows */
    for (ii = 0; (wp = bwlist[ii]) != NULL; ii++)
    {
        wp->firstRow += n;
        wp->numRows -= n;
        fixWindowTextSize(wp);
    }
    return (TRUE);

}

/*
 * Shrink the current window. Find the window that gains space. Hack at the
 * window descriptions. Ask the redisplay to do all the hard work. Bound to
 * "C-X C-Z".
 */
int
shrinkWindVert(int f, int n)
{
    return growWindVert(f,0-n) ;
}


/*      Resize the current window to the requested size */

int
resizeWndVert(int f, int n)
{
    int clines;                         /* current # of lines in window */

    if (f == FALSE)                     /* Must have a non-default argument */
        return(TRUE);                   /* else ignore call */
    clines = curwp->numTxtRows;           /* Find out what to do */
    if (clines == n)                    /* Already the right size? */
        return(TRUE);                   /* Yes - quit */

    return(growWindVert(TRUE, n - clines));
}

/*
 * Enlarge the current window - adds n columns to the current window.
 *
 * Find the windows that lose space. Make sure it is big enough. If so,
 * hack the window descriptions, and ask redisplay to do all the hard work.
 * You don't just set "force reframe" because dot would move.
 * Bound to "C-X Z".
 */

int
growWindHorz(int f, int n)
{
    WINDOW *wp;                         /* Temporary window pointer */
    WINDOW **twlp;                      /* Temporary window list pointer */
    WINDOW *rwlist [NWINDOWS+1];        /* Right set of windows */
    WINDOW *lwlist [NWINDOWS+1];        /* Left set of windows */
    int     ii;                         /* Local loop counter */
    int     jj;                         /* Used as minimum window size */

    /* Quick check - we may be the only window */
    if (wheadp == NULL)
        goto one_window;

    /* Get the resize lists - these are the windows that under-go change,
     * made a little more complicated by the presence of horizontal split
     * windows. In this case we have to consider other windows adjacent to
     * "curwp" that may undergo a change is size because of the split */
    if (getAdjacentWindowList (rwlist, WINDOW_RIGHT, curwp) == NULL)
    {
        n = 0 - n;
        if (getAdjacentWindowList (lwlist, WINDOW_LEFT, curwp) == NULL)
            goto one_window;
        getAdjacentWindowList (rwlist, WINDOW_RIGHT, lwlist[0]);
    }
    else
        getAdjacentWindowList (lwlist, WINDOW_LEFT, rwlist[0]);

    meAssert (rwlist[0] != NULL);
    meAssert (lwlist[0] != NULL);

    /* Compute the minumum size of the windows */
    jj = TTncol+1;
    twlp = (n < 0) ? lwlist : rwlist;
    for (ii = 0; (wp = twlp [ii]) != NULL; ii++)
        if (wp->numTxtCols < jj)
            jj = wp->numTxtCols;
    jj -= (gsbarmode & WMVWIDE) ? 3:2;   /* Minimum of 3 columns !! */

    /* Do the resizing of the windows */
    if (n < 0)
    {
        if (jj <= -n)
            goto enlarge_imp;
    }
    else if (jj <= n)
        goto enlarge_imp;

    /* Resize all of the windows LEFT curwp */
    for (ii = 0; (wp = lwlist[ii]) != NULL; ii++)
    {
        wp->numCols += n;
        fixWindowTextSize(wp);
    }

    /* Resize all of the windows to the RIGHT of curwp */
    for (ii = 0; (wp = rwlist[ii]) != NULL; ii++)
    {
        wp->firstCol += n;
        wp->numCols -= n;
        fixWindowTextSize(wp);
    }
    return (TRUE);

    /* Handle any errors with a report */
enlarge_imp:
    return mlwrite(MWCLEXEC|MWABORT,(uint8 *)"Impossible change");
one_window:
    return mlwrite(MWCLEXEC|MWABORT,(uint8 *)"Only one horizontal window");
}

/* Shrink the current window - resize by columns
 * Find the window that gains space. Hack at the window descriptions.
 * Ask the redisplay to do all the hard work.
 * Bound to "C-X C-Z". */
int
shrinkWindHorz(int f, int n)
{
    return growWindHorz (f, 0-n);    /* Get enlarge to do work */
}

/* Resize the current window - size to requested columns */
int
resizeWndHorz(int f, int n)
{
    int ccols;                           /* current # of columns in window */

    if (f == FALSE)                     /* Must have a non-default arg .. */
        return(TRUE);                   /* .. otherwise ignore */
    ccols = curwp->numTxtCols;            /* find out what to do */
    if (ccols == n)                     /* already the right size? */
        return(TRUE);                   /* Yes - finish */

    return (growWindHorz (TRUE, n - ccols));
}

/*
 * Pick a window for a pop-up. Split the screen if there is only one window.
 * Pick the uppermost window that isn't the current window. An LRU algorithm
 * might be better. Return a pointer, or NULL on error.
 */

WINDOW  *
wpopup(uint8 *name, int flags)
{
    WINDOW *wp=NULL;
    BUFFER *bp;

    if(name != NULL)
    {
        if((bp=bfind(name,(flags & 0xff))) == NULL)
            return NULL ;
        if(bp == curbp)
        {
            /* It is already the current buffer so return the current window
             * One complication is that this is often used with BFND_CLEAR
             * so the current buffer is NACT and the fhook needs to be
             * executed etc. The best way is to "swap" the buffer
             */
            if(meModeTest(bp->b_mode,MDNACT))
                swbuffer(curwp,curbp);
            return curwp ;
        }
        if(bp->b_nwnd > 0)
            for(wp=wheadp ; wp->w_bufp != bp ; wp=wp->w_wndp)
                ;
        else if(flags & WPOP_EXIST)
            return NULL ;
    }
    if((wp == NULL) && (flags & WPOP_USESTR))
    {
        wp = curwp ;
        for(;;)
        {
            if((wp = wp->w_wndp) == NULL)
                wp = wheadp ;
            if(wp == curwp)
            {
                wp = NULL ;
                break ;
            }
            if(wp->w_bufp->b_bname[0] == '*')
                break ;
        }
    }
    if(wp == NULL)
    {
        if(wheadp->w_wndp == NULL)
            splitWindVert(FALSE, 0) ;
        if((wp = curwp->w_wndp) == NULL)
            wp = wheadp ;
    }
    if(name != NULL)
        swbuffer(wp,bp);
    if(flags & WPOP_MKCURR)
        makeCurWind(wp) ;

    return wp ;
}

int
popupWindow(int f, int n)
{
    uint8 bufn[MAXBUF], *nn ;
    int s ;

    if((s = getBufferName((uint8 *)"Popup buffer", 0, 2, bufn)) != TRUE)
        return s ;
    if(bufn[0] == '\0')
        nn = NULL ;
    else
        nn = bufn ;
    if(n == 0)
        n = WPOP_EXIST|WPOP_MKCURR ;
    else
    {
        n = BFND_CREAT|WPOP_MKCURR ;
        if(f)
            n |= WPOP_USESTR ;
    }
    if(wpopup(nn,n) == NULL)
        return FALSE ;
    return TRUE ;
}

int
scrollNextUp(int f, int n)          /* scroll the next window up (back) a page */
{
    nextWindow(FALSE, 1);
    scrollUp(f, n);
    return(prevwind(FALSE, 1));
}

int
scrollNextDown(int f, int n)          /* scroll the next window down (forward) a page */
{
    nextWindow(FALSE, 1);
    scrollDown(f, n);
    return (prevwind(FALSE, 1));
}

/*
 * Re-arrange the windows on the screen automatically.
 */

int
resizeAllWnd (int f, int n)
{
    WINDOW *wp;                         /* Current window pointer */
    WINDOW *pwp;                        /* Previous window pointer */
    WINDOW *twp;                        /* Temporary window pointer */
    int row [NWINDOWS];                 /* Keep the start row number */
    int col [NWINDOWS];                 /* Keep the start col number */
    int maxrow = 0;                     /* Maximum number of rows */
    int maxcol = 0;                     /* Maximum number of columns */
    int colwidth;                       /* Width of a colunm */
    int rowdepth;                       /* Depth of a row */
    int wid;                            /* Window indenty */
    int twid;                           /* Temporary window identity */
    int ii, jj;                         /* Temporary loop counters */

    pwp = wheadp;                       /* Previous window is head of list */
    wp = pwp->w_wndp;                   /* Current window is the next */

    row [0] = 0;                        /* First window must be (0,0) */
    col [0] = 0;
    wid = 1;

    /* Iterate down the list of windows determining the starting row and
     * column numbers of each */
    while (wp != NULL)
    {
        /*
         * Top rows are the same - must be horizontally stacked. e.g.
         *
         *  |
         *  +-------+-------+-------+-------+--
         *  |       |       |       |       |
         *  |       |  pwp  |  wp   |       |
         *  |       |       |       |       |
         *  +-------+-------+-------+-------+--
         *  |
         */
        if (pwp->firstRow == wp->firstRow)
        {
            row [wid] = row [wid-1];
            col [wid] = col [wid-1] + 1;
        }
        /*
         * Left columns are the same - must be vertically stacked e.g.
         *
         * |               |
         * +---------------+
         * |               |
         * +---------------+
         * |      pwp      |
         * +---------------+
         * |      wp       |
         * +---------------+
         * |               |
         */
        else if (pwp->firstCol == wp->firstCol)
        {
            row [wid] = row [wid-1] + 1;
            col [wid] = col [wid-1];
        }
        /*
         * Previous Left column is greater than the current left column - must
         * be return from a horizontal complex column e.g.
         *
         * |
         * +----------+--------------+
         * |          |              |
         * |          +--------------+
         * |          |              |
         * +----------+--------------+
         * | (*)      |     pwp      |
         * +----------+--------------+
         * | wp
         * |
         * +--
         *
         * Find the last occurence of a window on the current column (*) and
         * propogate the maximum row count to the current window. Propogate
         * the column depth of (*)
         */
        else if (pwp->firstCol > wp->firstCol)
        {
            ii = 0;                     /* Maximum row count */
            twid = wid;                 /* Temporary window identity */
            twp = wp;                   /* Point to current block */

            do
            {
                twp = twp->w_wnup;      /* Previous block */
                twid--;                 /* Previous identity */
                if (row [twid] > ii)    /* This row greater than current */
                    ii = row [twid];    /* Yes - save it !! */
            } while (twp->firstCol != wp->firstCol);

            row [wid] = ii+1;           /* wp must be on the next row */
            col [wid] = col [twid];     /* Propogate column depth */
        }
        /*
         * Current top row is smaller than previous top row - must be entry
         * into a horizontal complex column e.g.
         *
         * |
         * +----------+--------------
         * | (*)      | wp
         * +----+-----+
         * |    |     |
         * +----+-----+
         * |   pwp    |
         * +----------+-------------
         * |
         * Find the last occurence of the window on the same row (*) and
         * find the deepest column on route. Store the deepest column and
         * row depth of (*).
         */
        else if (pwp->firstRow > wp->firstRow)
        {
            ii = 0;                     /* Maximum column count */
            twid = wid;                 /* Temporary window indentity */
            twp = wp;                   /* Point to current block */

            do
            {
                twp = twp->w_wnup;      /* Previous block */
                twid--;                 /* Previous identity */
                if (col [twid] > ii)    /* This column greater than current */
                    ii = col [twid];    /* Yes - save it !! */
            } while (twp->firstRow != wp->firstRow);

            row [wid] = row [twid];     /* Propogate row depth */
            col [wid] = ii + 1;         /* wp must be the next column */
        }
        else
            return mlwrite(MWABORT,(uint8 *)"Cannot get into this state !!");

        /* Increment the counters, move onto the next row and accumulate
         * the row and column maximums */
        if (row [wid] > maxrow)
            maxrow = row [wid];         /* Save maximum row */
        if (col [wid] > maxcol)
            maxcol = col [wid];         /* Save maximum column */

        wid++;                          /* Next window */
        pwp = wp;                       /* Advance pointers */
        wp = wp->w_wndp;
    }

    /* Resize the windows */
    maxrow++;                           /* The maximum number of rows */
    maxcol++;                           /* The maximum number of colunms */

    colwidth = TTncol / maxcol;         /* Unit width of a colunm */
    rowdepth = (TTnrow-TTsrow)/maxrow;  /* Unit width of a row */

    /* Make sure that the windows can be re-sized. If they fall below the
     * minimum then delete all windows with the exception of 1 */
    if ((colwidth < 6) || (rowdepth < 2))
        return onlywind(0,0);

    /* Iterate down the list of windows determining the ending row and
     * column numbers of each */

    wp = wheadp;
    for (wid = 0; wid < numWindows; wid++)
    {
        /* Find the end colunm. Search forward until we find another window
         * with the same top row. */
        for (twid = wid + 1; twid < numWindows; twid++)
        {
            ii = row [wid];             /* Get current start row */
            if (row [twid] <= ii)       /* Same starting row ?? */
            {
                ii = col [twid];        /* Save the end column */
                break;                  /* Quit - located end colunm */
            }
        }
        if (twid == numWindows)         /* Got to the end ?? */
            ii = maxcol;                /* Yes - must be the maximum column */

        /* Find the end row. Search forward until we find another window
         * with the same starting colunm */
        for (twid = wid + 1; twid < numWindows; twid++)
        {
            jj = col [wid];             /* Get current start colunm */
            if (col [twid] <= jj)       /* Lower/equal starting colunm ?? */
            {
                jj = row [twid];        /* Save the end row */
                break;
            }
        }
        if (twid == numWindows)
            jj = maxrow;

        /* Size Vertically - Assign the new row indicies */
        if ((f == FALSE) || (n > 0))
        {
            jj = (jj == maxrow) ? TTnrow : (jj * rowdepth) + TTsrow;
            wp->firstRow = (row [wid] * rowdepth) + TTsrow;
            wp->numRows = jj - wp->firstRow;
        }

        /* Size Horizontally - Assign the colunm indicies */
        if ((f == FALSE) || (n < 0))
        {
            ii = (ii == maxcol) ? TTncol : (ii * colwidth);
            wp->firstCol = col [wid] * colwidth;
            wp->numCols = ii - wp->firstCol;
        }
        
        /* Reframe - Update all of the windows to reflect the new sizes */
        fixWindowTextSize(wp);
        wp = wp->w_wndp;                /* Move onto the next */
    }
    sgarbf = TRUE;                      /* Garbage the screen */
/*    mlwrite (MWABORT, "There are %d rows and %d cols !!", maxrow, maxcol);*/
    return TRUE;

}


/*
 * changeScreenDepth  - Change the depth of the screen.
 * Resize the screen, re-writing the screen
 */

int
changeScreenDepth(int f, int n)
{
    /* if the command defaults fail. */
    if (f == FALSE)                     /* No argument ?? */
        return mlwrite(MWABORT,(uint8 *)"[Argument expected]");
    if ((n < 4) || (n > 400))           /* Argument in range ?? */
        return mlwrite(MWABORT,(uint8 *)"[Screen depth %d out of range]", n);
    if (n == TTnrow+1)
        return (TRUE);                  /* Already the right size */
    
    /* Only process if the window size is different from the current
     * window size. If we got here that is true. 
     * Go and get some more screen space */
    if (n > TTmrow)
    {
        if (vvideoEnlarge (n) == FALSE)
depth_error:                            /* Safe exit point for failures */
            n = TTmrow;
        else
        {
            FRAMELINE *flp;             /* Temporary frame line */
            int ii, jj;                 /* Local loop counter */
            
            /* Grow the Frame store depthwise do this safely so that 
             * we do not cause a crash at the video end. 
             *
             * Grow the frame store first. Reallocate and then
             * copy across the old information. 
             */
            if ((flp = (FRAMELINE *) meMalloc (sizeof (FRAMELINE) * n)) == NULL)
                goto depth_error;       /* Fail safe */
            memcpy (flp, frameStore, sizeof (FRAMELINE) * TTmrow);
            meFree (frameStore);        /* Free off old store */
            frameStore = flp;           /* Re-assign */
            
            /* Allocate a new set of lines for the remainder of the space */
            for (flp += TTmrow, ii = TTmrow; ii < n; ii++, flp++)
            {
                if ((flp->scheme = meMalloc(TTmcol*(sizeof(uint8)+sizeof(meSCHEME)))) == NULL)
                {
                    n = ii;             /* Stop here */
                    break;              /* Quit */
                }
                flp->text = (uint8 *) (flp->scheme+TTmcol) ;
                /* Initialise the data to something valid */
                jj = TTmcol ;
                while(--jj >= 0)
                {
                    flp->text[jj] = ' ' ;
                    flp->scheme[jj] = globScheme ;
                }
            }
            TTmrow = n;                 /* Set max rows to new value */
        }
    }

    /* Fix up the message line by binding to the new video frame */
    vvideo.video [TTnrow].flag = 0 ;    /* Decouple the old one */
    vvideo.video [TTnrow].line = NULL ;
    TTnrow = --n ;                      /* Set up global number of rows */
    vvideo.video [n].flag = VFMESSL;    /* Bind in new message line */
    vvideo.video [n].line = mline ;
    vvideo.video [n].endp = TTncol;
    
    if (TTsrow > 0)
        vvideo.video [0].line = menuLine;
    
    /* Fix up the windows */
    resizeAllWnd (TRUE, 1);             /* Resize windows vertically */

#ifdef _WINDOW
    TTdepth(TTnrow+1) ;                 /* Change the depth of the screen */
#endif
    return TRUE ;
}

/*
 * changeScreenWidth - Change the width of the screen.
 * Resize the screen, re-writing the screen
 */

int
changeScreenWidth(int f, int n)
{
    /* if the command defaults, fail */
    if (f == FALSE)                     /* No argument ?? */
        return mlwrite(MWABORT,(uint8 *)"Argument expected");
    if ((n < 8) || (n > 400))           /* In range ?? */
        return mlwrite(MWABORT,(uint8 *)"Screen width %d out of range", n);
    if (n == TTncol)                    /* Already this size ?? */
        return TRUE;
    
    /* Only process if the window size is different from the current
     * window size. If we got here that is true, */
    if(n > TTmcol)
    {
        /* Must extend the length of mline, mlStore, and all window
         * mode lines */
        LINE *ml ;
        uint8 *mls ;

        if(((ml = meMalloc(sizeof(LINE)+n)) == NULL) ||
           ((mls = meMalloc(n+1)) == NULL))
width_error:            
            n = TTmcol ;
        else
        {
            WINDOW *wp ;                /* Temporary window pointer */
            FRAMELINE *flp;             /* Frame line pointer */
            FRAMELINE fl;               /* Temporary frame line */
            int ii, jj;                 /* Local loop counters */
            
            /* Fix up the frame store by growing the lines. Do a safe
             * grow where by we can recover if a malloc fails. */
            for (flp = frameStore, ii = 0; ii < TTmrow; ii++, flp++)
            {
                if ((fl.scheme = meMalloc(n*(sizeof(uint8)+sizeof(meSTYLE)))) == NULL)
                    goto width_error;   /* Goto safe fail point */
                fl.text = (uint8 *) (fl.scheme+n) ;
                
                /* Data structures allocated. Copy accross the new screen
                 * information and pad endings with valid data. Strictly we
                 * do not need to do this for all platforms, however if it
                 * is safer if we make sure the data is valid. Resize is an
                 * infrequent operation and time is not critical here */
                memcpy (fl.text, flp->text, sizeof(uint8) * TTmcol);
                memcpy (fl.scheme, flp->scheme, sizeof(meSCHEME) * TTmcol);
                jj = n ;
                while(--jj >= TTmcol)
                {
                    fl.text[jj] = ' ' ;
                    fl.scheme[jj] = globScheme ;
                }                
                /* Free off old data and copy in new */
                meFree (flp->scheme);
                flp->text = fl.text;
                flp->scheme = fl.scheme;
            }
            /* Fix up the window structures */
            memcpy(ml,mline,sizeof(LINE)+mline->l_used) ;
            if(mlStatus & MLSTATUS_KEEP)
            {
                meStrcpy(mls,mline->l_text);
                mlStoreCol = mlCol ;
                mlStatus = (mlStatus & ~MLSTATUS_KEEP) | MLSTATUS_RESTORE ;
            }
            else if(mlStatus & MLSTATUS_RESTORE)
                meStrcpy(mls,mlStore) ;

            free(mline) ;
            free(mlStore) ;
            vvideo.video [TTnrow].line = (mline = ml) ;
            ml->l_size = n;
            mlStore = mls ;

            wp = wheadp ;
            while(wp != NULL)
            {
                if((ml = lalloc(n)) == NULL)
                {
                    n = TTmcol ;
                    break ;
                }
                memcpy(ml,wp->model,sizeof(LINE)+wp->model->l_used) ;
                free(wp->model) ;
                wp->model = ml ;
                wp = wp->w_wndp ;
            }
            TTmcol = n ;
        }
    }

    TTncol = n ;
    
    /* just re-width it (no big deal) */
    resizeAllWnd (TRUE, -1);            /* Resize windows horizontally */
#ifdef _WINDOW
    TTwidth(n) ;                        /* Change the width of the screen */
#endif
    return(TRUE);
}
    
int
setPosition(int f, int n)		/* save ptr to current window */
{
    register mePOS *pos ;
    uint16 mark ;                       /* Position alpha mark name*/
    int cc ;
    
    if((cc = mlCharReply((uint8 *)"Set position: ",mlCR_QUIT_ON_USER,NULL,NULL)) == -2)
        cc = mlCharReply((uint8 *)"Set position: ",mlCR_ALPHANUM_CHAR,NULL,NULL) ;
    
    if(cc < 0)
        return ctrlg(FALSE,1) ;
    
    pos = mePosition ;
    mark = meAM_FRSTPOS ;
    while((pos != NULL) && (cc != (int) pos->name))
    {
        pos = pos->next ;
        mark++ ;
    }
    if(pos == NULL)
    {
        pos = meMalloc(sizeof(mePOS)) ;
        if(pos == NULL)
            return ABORT ;
        pos->next = mePosition ;
        mePosition = pos ;
        pos->name = cc ;
        pos->line_amark = mark ;
    }
    if(f == FALSE)
        n = mePOS_DEFAULT ;
    pos->flags = n ;
    
    if(n & mePOS_WINDOW)
        pos->window = curwp ;
    if(n & mePOS_BUFFER)
        pos->buffer = curbp ;
    if(n & mePOS_LINEMRK)
    {
        if(alphaMarkSet(curbp,pos->line_amark,curwp->w_dotp,0,1) != TRUE)
        {
            pos->flags = 0 ;
            return ABORT ;
        }
    }
    if(n & mePOS_LINENO)
        pos->line_no = curwp->line_no ;
    if(n & mePOS_LINEOFF)
        pos->w_doto = curwp->w_doto ;
    if(n & mePOS_WINYSCRL)
        pos->topLineNo = curwp->topLineNo ;
    if(n & mePOS_WINXCSCRL)
        pos->w_scscroll = curwp->w_scscroll ;
    if(n & mePOS_WINXSCRL)
        pos->w_sscroll = curwp->w_scscroll ;
    
    return TRUE ;
}

int
gotoPosition(int f, int n)		/* restore the saved screen */
{
    register mePOS *pos ;
    int ret=TRUE, sflg ;
    int cc ;
    
    if(n == -1)
    {
        while(mePosition != NULL)
        {
            pos = mePosition ;
            mePosition = pos->next ;
            free(pos) ;
        }
        return TRUE ;
    }
    
    if((cc = mlCharReply((uint8 *)"Goto position: ",mlCR_QUIT_ON_USER,NULL,NULL)) == -2)
        cc = mlCharReply((uint8 *)"Goto position: ",mlCR_ALPHANUM_CHAR,NULL,NULL) ;
    if(cc < 0)
        return ctrlg(FALSE,1) ;
    
    pos = mePosition ;
    while((pos != NULL) && (cc != (int) pos->name))
        pos = pos->next ;
    
    if(pos == NULL)
    {
        uint8    allpos[256]; 	/* record of the positions	*/
        int      ii = 0;
        
        pos = mePosition ;
        while(pos != NULL)
        {
            if(pos->name < 128)
            {
                allpos[ii++] = ' ' ;
                allpos[ii++] = (uint8) pos->name;
            }
            pos = pos->next;
        }
        if(ii == 0)
            return mlwrite(MWABORT|MWCLEXEC,(uint8 *)"[No positions set]");
        allpos[ii] = '\0';
        return mlwrite(MWABORT|MWCLEXEC,(uint8 *)"[Valid positions:%s]", allpos);
    }
    
    if(f == FALSE)
        n = pos->flags ;
    else
        n &= pos->flags ;
    
    if(ret && (n & mePOS_WINDOW))
    {
        /* find the window */
        register WINDOW *wp;
        wp = wheadp;
        while (wp != NULL)
        {
            if(wp == pos->window)
            {
                makeCurWind(wp) ;
                break ;
            }
            wp = wp->w_wndp;
        }
        if(wp == NULL)
            ret = FALSE ;
    }
    if(ret && (n & mePOS_BUFFER))
    {
        /* find the buffer */
        register BUFFER *bp;
        bp = bheadp;
        while (bp != NULL)
        {
            if(bp == pos->buffer)
            {
                swbuffer(curwp,bp) ;
                break ;
            }
            bp = bp->b_bufp;
        }
        if(bp == NULL)
            ret = FALSE ;
    }
    if(ret && (n & mePOS_LINEMRK))
    {
        if((ret = alphaMarkGet(curbp,pos->line_amark)) == TRUE)
        {
            curwp->w_dotp = curbp->b_dotp ;
            curwp->line_no = curbp->line_no ;
            curwp->w_doto = 0 ;
            curwp->w_flag |= WFMOVEL ;
        }
    }
    else if(ret && (n & mePOS_LINENO))
        ret = gotoLine(1,pos->line_no+1) ;
    
    if(ret && (n & mePOS_LINEOFF))
    {
        if(pos->w_doto > llength(curwp->w_dotp))
        {
            curwp->w_doto = llength(curwp->w_dotp) ;
            ret = FALSE ;
        }
        else
            curwp->w_doto = pos->w_doto ;
    }
    
    sflg = 0 ;
    if(n & mePOS_WINYSCRL)
    {
        if(curbp->elineno && (pos->topLineNo >= curbp->elineno))
            pos->topLineNo = curbp->elineno-1 ;
        if(curwp->topLineNo != pos->topLineNo)
        {
            curwp->topLineNo = pos->topLineNo ;
            curwp->w_flag |= WFMAIN|WFSBOX|WFLOOKBK ;
            sflg = 1 ;
        }
    }
    if(n & mePOS_WINXCSCRL)
    {
        if(pos->w_scscroll >= llength(curwp->w_dotp))
            pos->w_scscroll = llength(curwp->w_dotp)-1 ;
        if(curwp->w_scscroll != pos->w_scscroll)
        {
            curwp->w_scscroll = pos->w_scscroll ;
            curwp->w_flag |= WFREDRAW ;        /* Force a screen update */
            sflg = 1 ;
        }
    }
    if(n & mePOS_WINXSCRL)
    {
        if(curwp->w_sscroll != pos->w_sscroll)
        {
            curwp->w_sscroll = pos->w_sscroll ;
            curwp->w_flag |= WFREDRAW ;        /* Force a screen update */
            sflg = 1 ;
        }
    }
    if(sflg)
        updCursor(curwp) ;
    
    if(!ret)
        return mlwrite(MWCLEXEC|MWABORT,(uint8 *)"[Failed to restore position]");
    return TRUE ;
}

/*
 * menuWindow.
 * Change the state of the window line.
 * Shuffle windows for allow the insertion of a menu line
 * 
 * Return TRUE if the menu line has changed visibility state.
 */
int
menuWindow (int newTTsrow)
{
    WINDOW *wp;
    
    /* Make some / remove some space for the menu line */
    if (newTTsrow != TTsrow)
    {
        TTsrow = newTTsrow;
        
        /* REMOVE - existing menu line */
        if (newTTsrow == 0)
        {
            for (wp = wheadp; wp != NULL; wp = wp->w_wndp)
            {
                if (wp->firstRow == 1)
                {
                    wp->firstRow = 0;
                    wp->numRows += 1;
                    fixWindowTextSize(wp);
                }
            }
        }
        
        /* INSERT - new menu */
        else
        {
            /* Try to insert the menu line by squashing the 
             * existing buffers down. */
            for (wp = wheadp; wp != NULL; wp = wp->w_wndp)
            {
                if (wp->firstRow == 0)
                {
                    /* Failed. Reduce to the current window which will
                     * give us enough roo,. */
                    if (wp->numRows <= 2)
                    {
                        onlywind(0,0);
                        break;
                    }
                    else
                    {
                        wp->numRows -= 1;
                        wp->firstRow = TTsrow;
                        fixWindowTextSize(wp);
                    }
                }
            }
        }
    }
    else
        return (FALSE);
    
/*    sgarbf = TRUE; */                 /* Garbage the screen */
    return (TRUE);                      /* Changed status     */
}


