/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * window.c - Window handling routines.
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
 * Synopsis:    Window handling routines.
 * Authors:     Unknown, Jon Green & Steven Phillips
 * Description:
 *     Window management. Some of the functions are internal, and some are
 *     attached to keys that the user actually types.
 */

#define __WINDOWC                       /* Define filename */

#include "emain.h"
#include "efunc.h"
#include "esearch.h"

#if MEOPT_HSPLIT

#define WINDOW_NEXT   0                 /* Vertical next window list */
#define WINDOW_PREV   1                 /* Vertical previous window list */
#define WINDOW_RIGHT  2                 /* Right window list */
#define WINDOW_LEFT   3                 /* Left window list */

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
 * in the meWindow structure itself. The key to sorting window adjacency
 * is the fact that the windows are consecutive within a column (or stripe)
 * hence we use this fact with the linkage information to determine if we
 * are dealing with a vertically or horizontally stacked window. Note the
 * case of 13/16. The window order should tell you that a single vertical
 * break exists for 13+16. The bottom lines of 13 and 16 are disarticulated.
 * Where as 9,10,11 and 12 share a horizontal line.
 *
 * We also get information from the Virtual Video structure meVideo. This
 * contains a thread through the meWindow structure of all of the windows that
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
 * meWindowGetAdjacentList - Helper function
 * This function gets a list of all of the adjacent windows that are affected
 * by re-sizing/deleting of the window "wp" (usually the current window).
 */

static meWindow **
meWindowGetAdjacentList (meWindow **wlist, int direction, meWindow *wp)
{
    meWindow *twp;                        /* Temporary window pointer */
    int wlen = 0;                       /* Window list length */
    int vtarget;                        /* Vertical target line */
    int htarget;                        /* Horizontal target line */

    if (direction < WINDOW_RIGHT)
    {
        /* Windows below current window required. Our target termination
         * point is the current window (end row) point */
        if (direction == WINDOW_NEXT)
        {
            vtarget = wp->frameColumn + wp->width;
            htarget = wp->frameRow + wp->depth;
            if (htarget >= frameCur->depth)
                return (NULL);

            for (twp = wp->next; twp != NULL; twp = twp->next)
            {
                /* If the window pointer is > vtarget; gone too far */
                if (wlen == 0)
                {
                    if ((twp->frameColumn + twp->width) > vtarget)
                        vtarget = twp->frameColumn + twp->width;
                }
                else if (twp->frameColumn >= vtarget)
                    break;

                if (twp->frameRow == htarget)     /* Horizontally aligned ?? */
                    wlist [wlen++] = twp;       /* Yes - Found one !! */
            }
        }
        /* Windows above current window required. Our target termination
         * point is the current window (start row) point. */
        else
        {
            if ((htarget = wp->frameRow) == meFrameGetMenuDepth(frameCur))
                return (NULL);

            /* Search backwards */
            for (twp = wp->prev; twp != NULL; twp = twp->prev)
            {
                /* If the window pointer is > htarget; gone too far */
                if ((twp->frameRow + twp->depth) > htarget)
                    break;
                if ((twp->frameRow + twp->depth) == htarget)
                    wlist [wlen++] = twp;   /* Found one !! */
            }
        }
    }
    else
    {
        /* Windows to the right of the current window reqired. We cheat
         * here. Because of the structure of meVideo it is only necessary
         * to find the horizontal window aligned with the top of the
         * window. Our target termination point is the current window
         * (start row, end col) point. */
        if (direction == WINDOW_RIGHT)
        {
            htarget = wp->frameRow;
            vtarget = wp->frameColumn + wp->width;
            if (vtarget >= frameCur->width)
                return (NULL);

            for (twp = wp->next; twp != NULL; twp = twp->next)
            {
                if (twp->frameColumn != vtarget)
                    continue;
                if ((twp->frameRow <= htarget) &&
                    ((twp->frameRow + twp->depth) >= htarget))
                {
                    /* pick all of the windows off the meVideo structure */
                    for (twp = twp->video->window; twp != NULL;
                         twp = twp->videoNext)
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

            if (wp->frameColumn == 0)
                return (NULL);

            vtarget = wp->frameColumn ;
            hmax = meFrameGetMenuDepth(frameCur) ;
            hmin = frameCur->depth+1 ;

            /* Find the horizontal range of windows to left of current window */
            for (twp = wp->video->window; twp != NULL; twp = twp->videoNext)
            {
                if (twp->frameRow < hmin)
                    hmin = twp->frameRow;
                if ((twp->frameRow + twp->depth) > hmax)
                    hmax = twp->frameRow + twp->depth;
            }

            /* Sniff out the windows */
            for (twp = wp->prev; twp != NULL; twp = twp->prev)
            {
                if ((twp->frameRow >= hmin) &&
                    (twp->frameRow <= hmax) &&
                    (twp->frameColumn + twp->width == vtarget))
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
#endif

#if MEOPT_SCROLL
/*
 * meWindowFixScrollBox
 * Fixes the positions of the window scroll box components. 
 */
void
meWindowFixScrollBox (meWindow *wp)
{
    int row;                            /* The starting row */
    int shaftLength;                    /* Length of scroll shaft */
    int boxLength;                      /* Length of the box */
    int bufferLength;                   /* Length of the buffer */
    int shaftMovement;                  /* Permitted movement on shaft */
    int boxPosition;                    /* Start position of box */
        
    /* Has this window got a bar present ?? */
    if ((wp->vertScrollBarMode & WMSCROL) == 0)
        return;                         /* No quit */
    
    /* Sort out where the different components of the bar should be */
    row = wp->vertScrollBarPos [WCVSBUP - WCVSBSPLIT];
    shaftLength = wp->vertScrollBarPos [WCVSBDSHAFT - WCVSBSPLIT] - row;
    
    bufferLength = wp->buffer->lineCount + 1;
     
    if (wp->textDepth < bufferLength)
    {
        boxLength = (wp->textDepth * shaftLength) / bufferLength;
        if (boxLength == 0)
            boxLength = 1;
        shaftMovement = shaftLength - boxLength;
        if (shaftMovement > 0)
        {
            if((boxPosition = wp->vertScroll) > 0)
            {
                boxPosition = ((boxPosition * shaftMovement) /
                               (bufferLength - wp->textDepth));
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
    if (boxLength != (wp->vertScrollBarPos [WCVSBBOX - WCVSBSPLIT] - 
                      wp->vertScrollBarPos [WCVSBUSHAFT - WCVSBSPLIT]))
    {
        /* Box changed size. Change upper and lower shafts and the box */
        wp->updateFlags |= WFSBUSHAFT|WFSBBOX|WFSBDSHAFT;
        wp->vertScrollBarPos [WCVSBUSHAFT - WCVSBSPLIT] = row;
        wp->vertScrollBarPos [WCVSBBOX - WCVSBSPLIT] = row + boxLength;
    }
    else
    {
        /* Test upper shaft end position */
        if (wp->vertScrollBarPos [WCVSBUSHAFT - WCVSBSPLIT] != row)
        {
            if (wp->vertScrollBarPos [WCVSBUSHAFT - WCVSBSPLIT] < row)
                wp->updateFlags |= WFSBUSHAFT|WFSBBOX;
            else
                wp->updateFlags |= WFSBDSHAFT|WFSBBOX;
            wp->vertScrollBarPos [WCVSBUSHAFT - WCVSBSPLIT] = row;
        }
        
        /* Test the box itself */
        row += boxLength;
        if (wp->vertScrollBarPos [WCVSBBOX - WCVSBSPLIT] != row)
        {
            if (wp->vertScrollBarPos [WCVSBBOX - WCVSBSPLIT] < row)
                wp->updateFlags |= WFSBUSHAFT|WFSBBOX;
            else
                wp->updateFlags |= WFSBDSHAFT|WFSBBOX;
            wp->vertScrollBarPos [WCVSBBOX - WCVSBSPLIT] = row;
        }
    }
}

/*
 * meWindowFixScrollBars
 * Fixes the positions of the window scroll bar components. 
 */
void
meWindowFixScrollBars(meWindow *wp)
{
    int row;                            /* The starting row */
    int erow;                           /* The ending row */
    int shaftLength;                    /* Length of scroll shaft */
    int mode;
    
    /* Has this window got a bar present ?? */
    if (!((mode = wp->vertScrollBarMode) & WMVBAR))
        return;                         /* No quit */
    
    /* Sort out where the different components of the bar should be */
    row = wp->frameRow;                   /* Start row */
    erow = wp->frameRow + wp->depth;     /* End row */
    shaftLength = wp->textDepth;
    
    wp->vertScrollBarPos [WCVSBML-WCVSBSPLIT] = erow;
    if (mode & WMBOTTM)
        erow--;
    else
        shaftLength++;
    
    /* Split */
    if ((mode & WMSPLIT) && (wp->depth > 4))
    {
        wp->vertScrollBarPos [WCVSBSPLIT-WCVSBSPLIT] = ++row;
        shaftLength--;
    }
    else
        wp->vertScrollBarPos [WCVSBSPLIT-WCVSBSPLIT] = row;
    
    /* Up Arrow */
    if ((mode & WMUP) && (wp->depth > 2))
    {
        wp->vertScrollBarPos [WCVSBUP-WCVSBSPLIT] = ++row;
        shaftLength--;
    }
    else
        wp->vertScrollBarPos [WCVSBUP-WCVSBSPLIT] = row;
    
    /* Down Arrow */
    wp->vertScrollBarPos [WCVSBDOWN-WCVSBSPLIT] = erow;
    if (mode & WMDOWN)
    {
        if (shaftLength > 0)
            erow--;
    }
    
    /* Up shaft/ box / down shaft */
    wp->vertScrollBarPos [WCVSBUSHAFT-WCVSBSPLIT] = row;
    wp->vertScrollBarPos [WCVSBBOX-WCVSBSPLIT] = row;
    wp->vertScrollBarPos [WCVSBDSHAFT-WCVSBSPLIT] = erow;
    
    /* If we are scrolling, fix up the modes */
    if (mode & WMSCROL)
        meWindowFixScrollBox(wp);
    wp->updateFlags |= WFSBAR;
}    
#endif /* MEOPT_SCROLL */

/*
 * meWindowFixTextSize
 * Fix the size of the window text area size following a window
 * re-size. Re-assign the global window mode.
 */
void
meWindowFixTextSize(meWindow *wp)
{
    /* Fix up the text area size */
    wp->textDepth = wp->depth - 1;
    
#if MEOPT_SCROLL
    /* Determine and assign the scrolling mode. We need do not need
     * a vertical scroll bar if the current scroll-mode is non-scrolling
     * and the window is on the right edge of the screen */
    if(gsbarmode & WMVBAR)
        wp->vertScrollBarMode = gsbarmode ;
    else if ((wp->frameColumn+wp->width) < frameCur->width)
        wp->vertScrollBarMode = WMVBAR ;
    else
        wp->vertScrollBarMode = 0 ;
    
    if (wp->vertScrollBarMode & WMVBAR)
        wp->textWidth = wp->width - ((wp->vertScrollBarMode & WMVWIDE) ? 2 : 1);
    else
#endif
        wp->textWidth = wp->width;

    
    /* Work out the margin. */
    wp->marginWidth = (wp->textWidth / 10) + 1;
    /* SWP 18/8/98 - need to add WFSELDRAWN to the flag list as if
     * an another window which contains a hilighted region becomes
     * part of this window, this flag is the only way to get it
     * re-evaluated. */
    wp->updateFlags |= WFRESIZE|WFMOVEL|WFSELDRAWN ;
#if MEOPT_SCROLL
    meWindowFixScrollBars(wp);
#endif
#if MEOPT_IPIPES
    if(meModeTest(wp->buffer->mode,MDPIPE) &&
       ((wp == frameCur->windowCur) || (wp->buffer->windowCount == 1)))
        ipipeSetSize(wp,wp->buffer) ;
#endif        
}

/* Main Window functions */

void
meWindowMakeCurrent(meWindow *wp)
{
#if MEOPT_FILEHOOK
    /* Process the exit hook of the old buffer. 
     * Force mode line / scroll bar update on old window */    
    if(frameCur->bufferCur->ehook >= 0)
        execBufferFunc(frameCur->bufferCur,frameCur->bufferCur->ehook,0,1) ;
#endif
    frameCur->windowCur->updateFlags |= WFMODE|WFSBAR;     /* Update scroll and mode lines */
    
    /* Do the swap */
    frameCur->windowCur = wp;
    frameCur->bufferCur = wp->buffer;
#if MEOPT_EXTENDED
    if(isWordMask != frameCur->bufferCur->isWordMask)
    {
        isWordMask = frameCur->bufferCur->isWordMask ;
#if MEOPT_MAGIC
        mereRegexClassChanged() ;
#endif
    }
#endif    
#if MEOPT_FILEHOOK
    /* Process the entry hook of the new buffer.
     * Force mode line / scroll bar update on new window */
    if(frameCur->bufferCur->bhook >= 0)
        execBufferFunc(frameCur->bufferCur,frameCur->bufferCur->bhook,0,1) ;
#endif
    frameCur->windowCur->updateFlags |= WFMODE|WFSBAR;
#if MEOPT_IPIPES
    if(meModeTest(frameCur->bufferCur->mode,MDPIPE))
	ipipeSetSize(frameCur->windowCur,frameCur->bufferCur) ;
#endif        
}

/*
 * frameAddModeToWindows
 * Scan through and add given modes to all windows
 */
void
frameAddModeToWindows(int mode)
{
    register meWindow *wp ;
    
    /* ZZZZ - all ??? */
    wp = frameCur->windowList;
    while (wp != NULL)
    {
        wp->updateFlags |= mode ;
        wp = wp->next;
    }
}

/*
 * meBufferAddModeToWindows
 * Scan through and add given mode to all windows showing buffer.
 */
void
meBufferAddModeToWindows (meBuffer *bp, int mode)
{
    register meWindow *wp;
    
    meFrameLoopBegin() ;
    for (wp = loopFrame->windowList; wp != NULL; wp = wp->next)
    {
        if (bp == wp->buffer)
            wp->updateFlags |= mode;
    }
    meFrameLoopEnd() ;
}

/*
 * Reposition dot in the current window to line "n". If the argument is
 * positive, it is that line. If it is negative it is that line from the
 * bottom. If it is 0 the window is centered (this is what the standard
 * redisplay code does). With no argument it defaults to 0.
 * Also Refreshes the screen. Bound to "C-L".
 */
int
windowRecenter(int f, int n)
{
    if (f == meFALSE)     /* default to 0 to center screen */
        n = 0;
    frameCur->windowCur->windowRecenter = n ;             /* Center */
    frameCur->windowCur->updateFlags |= WFFORCE ;

    sgarbf = meTRUE;                   /* refresh */

    return (meTRUE);
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
windowGotoNext(int f, int n)
{
    meWindow *wp, *ewp ;
    
    if(n & 0x04)
        ewp = NULL ;
    else
        ewp = wp = frameCur->windowCur ;
        
    for(;;)
    {
        if(n & 0x04)
        {
            wp = frameCur->windowList ;
            /* switch off bit 1 so it won't loop and bit 4 so we don't re-init */
            n &= ~0x05 ;
        }
        else if((wp = wp->next) == NULL)
        {
            if((n & 0x01) == 0)
                return meFALSE ;
            wp = frameCur->windowList ;
        }
        if(wp == ewp)
        {
#if MEOPT_EXTENDED
            if((wp->flags & meWINDOW_NO_NEXT) && ((n & 0x02) == 0))
                return meFALSE ;
#endif
            return meTRUE ;
        }
#if MEOPT_EXTENDED
        if(((wp->flags & meWINDOW_NO_NEXT) == 0) || (n & 0x02))
#endif
        {
            meWindowMakeCurrent(wp) ;
            return meTRUE ;
        }
    }
}

/*
 * This command makes the previous window (previous => up the screen) the
 * current window. There arn't any errors, although the command does not do a
 * lot if there is 1 window.
 */
int
windowGotoPrevious(int f, int n)
{
    meWindow *wp, *ewp ;
    
    if(n & 0x04)
        ewp = NULL ;
    else
        ewp = wp = frameCur->windowCur ;
        
    wp = frameCur->windowCur ;
    for(;;)
    {
        if(n & 0x04)
        {
            for (wp = frameCur->windowList; wp->next != NULL; wp = wp->next)
                /* NULL */ ;
            /* switch off bit 1 so it won't loop and bit 4 so we don't re-init */
            n &= ~0x05 ;
        }
        else if((wp = wp->prev) == NULL)
        {
            if((n & 0x01) == 0)
                return meFALSE ;
            for (wp = frameCur->windowList; wp->next != NULL; wp = wp->next)
                /* NULL */ ;
        }
        if(wp == ewp)
        {
#if MEOPT_EXTENDED
            if((wp->flags & meWINDOW_NO_NEXT) && ((n & 0x02) == 0))
                return meFALSE ;
#endif
            return meTRUE ;
        }
#if MEOPT_EXTENDED
        if(((wp->flags & meWINDOW_NO_NEXT) == 0) || (n & 0x02))
#endif
        {
            meWindowMakeCurrent(wp) ;
            return meTRUE ;
        }
    }
}

/*
 * Move the current window up by "arg" lines. Recompute the new top line of
 * the window. Look to see if "." is still on the screen. If it is, you win.
 * If it isn't, then move "." to center it in the new framing of the window
 * (this command does not really move "."; it moves the frame). Bound to
 * "C-X C-P".
 */

int
windowScrollUp(int f, int n)
{
    register long ii ;

    if(f == meFALSE)
        n = frameCur->windowCur->textDepth-1 ;
    else if(n < 0)
        return windowScrollDown(f, -n) ;
    
    if(frameCur->windowCur->vertScroll == 0)
        return meErrorBob() ;

    if((frameCur->windowCur->vertScroll-=n) < 0)
        frameCur->windowCur->vertScroll = 0 ;
    ii = frameCur->windowCur->dotLineNo - frameCur->windowCur->vertScroll - frameCur->windowCur->textDepth + 1 ;
    if(ii > 0)
        windowBackwardLine(meTRUE,ii) ;
    frameCur->windowCur->updateFlags |= (WFREDRAW|WFSBOX) ;           /* Mode line is OK. */
    return meTRUE ;
}

/*
 * This command moves the current window down by "arg" lines. Recompute the
 * top line in the window. The move up and move down code is almost completely
 * the same; most of the work has to do with reframing the window, and picking
 * a new dot. We share the code by having "move down" just be an interface to
 * "move up". Magic. Bound to "C-X C-N".
 */

int
windowScrollDown(int f, int n)
{
    register long ii ;

    if(f == meFALSE)
        n = frameCur->windowCur->textDepth-1 ;
    else if(n < 0)
        return windowScrollUp(f, -n) ;
    /* A quick if no lines case to make rest easier */
    if(frameCur->windowCur->vertScroll >= frameCur->bufferCur->lineCount-1)
        return meErrorEob() ;
    
    if((frameCur->windowCur->vertScroll+=n) >= frameCur->bufferCur->lineCount)
        frameCur->windowCur->vertScroll = frameCur->bufferCur->lineCount - 1 ;
    
    ii = frameCur->windowCur->vertScroll - frameCur->windowCur->dotLineNo ;
    if(ii > 0)
        windowForwardLine(meTRUE,ii) ;
    frameCur->windowCur->updateFlags |= (WFREDRAW|WFSBOX) ;           /* Mode line is OK. */
    return meTRUE ;
}

/*
 * This command moves the window right by "arg" columns. Recompute the left
 * column in the window. 
 *
 * No argument strictly implies that the whole screen is scrolled by a screen
 * width.
 */

int
windowScrollRight (int f, int n)
{
    if (f == meFALSE)                     /* No argument ?? */
        n = frameCur->windowCur->textWidth-2;          /* Scroll a whole screen width */

    /* To scroll simply set the current windows scroll position to the
     * appropriate start column. */
    if (n != 0)
    {
        int scroll, ii, jj, doto ;
        unsigned char *off ;
        windCurLineOffsetEval(frameCur->windowCur) ;
        /* try to scroll the current line by n */
        jj = meLineGetLength(frameCur->windowCur->dotLine) ;
        if((scrollFlag & 0x0f) > 2)
            scroll = frameCur->windowCur->horzScrollRest ;
        else if(jj == 0)
            return meTRUE ;
        else
            scroll = frameCur->windowCur->horzScroll ;
        if((scroll == 0) && (n < 0))
            return meTRUE ;
        scroll += n ;
        ii = scroll ;
        doto = 0 ;
        off = frameCur->windowCur->dotCharOffset->text ;
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
        if(frameCur->windowCur->horzScroll != scroll)
        {
            frameCur->windowCur->horzScroll = scroll ;
            frameCur->windowCur->updateFlags |= WFDOT ;
            /* Check the position of the cursor, move it if it has been 
             * scrolled off the screen.
             */
            ii = frameCur->windowCur->textWidth - 1 ;
            while(doto < frameCur->windowCur->dotOffset)
            {
                jj += *off++ ;
                if(jj >= ii)
                    break ;
                doto++ ;
            }
            if(frameCur->windowCur->dotOffset != doto)
            {
                frameCur->windowCur->dotOffset = doto ;
                frameCur->windowCur->updateFlags |= WFMOVEC ;
            }
        }
        if((scrollFlag & 0x0f) == 3)
        {
            ii = frameCur->windowCur->horzScrollRest + n ;
            if(ii < 0)
                ii = 0 ;
            else if(ii > disLineSize)
                ii = disLineSize ;
            if(ii != frameCur->windowCur->horzScrollRest)
            {
                frameCur->windowCur->horzScrollRest = ii ;
                frameCur->windowCur->updateFlags |= WFREDRAW;        /* Force complete screen refresh */
            }
        }
        else if((scrollFlag & 0x0f) && (frameCur->windowCur->horzScrollRest != frameCur->windowCur->horzScroll))
        {
            frameCur->windowCur->horzScrollRest = frameCur->windowCur->horzScroll ;
            frameCur->windowCur->updateFlags |= WFREDRAW;        /* Force complete screen refresh */
        }
    }
    return meTRUE ;
}

/*
 * This command moves the window left by "arg" columns. Recompute the left
 * column in the window. 
 *
 * No argument strictly implies that the whole screen is scrolled by a screen
 * width.
 */

int
windowScrollLeft (int f, int n)
{
    if (f == meFALSE)                     /* No argument ?? */
        n = frameCur->windowCur->textWidth-2;          /* Scroll a whole screen width */
    return windowScrollRight(meTRUE,-n) ;
}

#if MEOPT_MOUSE
/* 
 * windowSetScrollWithMouse
 * Set the scroll with the mouse. Typically bound to the scroll box to 
 * scroll the window up or down. Useless in any other context !!
 * Scrolls "frameCur->windowCur" window only.
 * 
 * An argument (any argument) tells us to lock to the scroll box.
 * No argument tells us to scroll from the locked position. 
 * 
 * Fails if scroll operation fails, or scroll bars are not enabled.
 */
int
windowSetScrollWithMouse (int f, int n)
{
    long mousePos;                      /* Curent mouse position */
    long screenTopRow;                  /* Top row of the screen */
    long currentTopRow;                 /* Current buffer top row. */
    static long startLine;              /* Starting line number (top row) */
    static long startMouse;             /* Starting mouse position */
    static long mouseRatio;             /* Ratio of mouse movement to lines */
    static long maxTopRow;              /* The maximum top row */
    
    /* Has this window got a bar present ?? */
    if ((frameCur->windowCur->vertScrollBarMode & WMSCROL) == 0)
        return meFALSE;                   /* No quit and bitch about it */
    
    /*****************************************************************************
     * 
     * LOCKING
     * 
     * If we are given an agrument then LOCK onto the start position. The start
     * position becomes our reference point for all of our scrolling work
     * 
     *****************************************************************************/
    
    if (f == meTRUE)
    {
        long scrollLength;               /* Length of scroll shaft */
        long bufferLength;               /* Length of the buffer */
        
        /* Get the buffer length */
        bufferLength = (long)(frameCur->windowCur->buffer->lineCount) + 1;
        
        /* If the buffer is wholly contained in the window then there is nothing
         * to do - quit now. Make sure that the top line is showing */
        if (frameCur->windowCur->textDepth >= bufferLength)
        {
            mouseRatio = -1;
            windowScrollUp (meTRUE, bufferLength + frameCur->windowCur->textDepth);
            return meTRUE;
        }
        
        /* Get the line number of the top row of the buffer. This is used as
         * the reference point for all of out scroll work */
        startLine = frameCur->windowCur->vertScroll ;
        /* We normalise the scroll bar if there is over-hanging
         * text by supplying a scroll at the start of the scroll */
        maxTopRow = bufferLength - frameCur->windowCur->textDepth;
        if (maxTopRow < startLine)
        {
            windowScrollUp(meTRUE, startLine - maxTopRow);
            startLine = maxTopRow;
        }
        
        /* Remeber the starting mouse position. Keep the precision up by ustilising
         * the fractional mouse components if available. Store as a fixed point 
         * 8 bit fractional integer */
        startMouse = ((long)(mouse_Y) << 8) + (long)(mouse_dY);
        
        /* Determine how long the movable scroll region is. This is a simple
           calculation of the (scroll bar length - scroll box length) */
        scrollLength = (long)((frameCur->windowCur->vertScrollBarPos [WCVSBDSHAFT - WCVSBSPLIT] - 
                               frameCur->windowCur->vertScrollBarPos [WCVSBUP - WCVSBSPLIT]) - 
                              (frameCur->windowCur->vertScrollBarPos [WCVSBBOX - WCVSBSPLIT] - 
                               frameCur->windowCur->vertScrollBarPos [WCVSBUSHAFT - WCVSBSPLIT]));
        
        /* Build a ratio factor of number of lines to the length of the scroll. 
         * Again keep the precision by storing as a 8-bit fractional integer. */
        if (scrollLength <= 0)
            mouseRatio = -1;            /* Cannot do alot with this !! */
        else
            mouseRatio = ((long)(bufferLength - frameCur->windowCur->textDepth) << 8) / scrollLength;
        
        return meTRUE;                    /* Finished locking. */
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
        return meTRUE;
    
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
    currentTopRow = frameCur->windowCur->vertScroll ;
    if (currentTopRow == screenTopRow)  /* Same position ?? */
        return meTRUE;                    /* Nothing to do - quit now. */
    
    /* Go and scroll - simple signed difference of the current top line and the
     * target top line -  done when the scroll has finished - simple !! */
    return (windowScrollDown (meTRUE, (int)(screenTopRow - currentTopRow)));
}
#endif

/*
 * This command makes the current window the only window on the screen. Bound
 * to "C-X 1". Try to set the framing so that "." does not have to move on the
 * display. Some care has to be taken to keep the values of dot and mark in
 * the buffer structures right if the distruction of a window makes a buffer
 * become undisplayed.
 */

int
windowDeleteOthers(int f, int n)
{
    register meWindow *wp, *nwp ;

#if MEOPT_EXTENDED
    if((n & 2) == 0)
    {
        wp = frameCur->windowList ;
        while (wp != NULL)
        {
            if((wp->flags & meWINDOW_NO_DELETE) && (wp != frameCur->windowCur))
                break ;
            wp = wp->next;
        }
        if(wp != NULL)
        {
            meWindow *cwp ;
            cwp = frameCur->windowCur ;
            wp = frameCur->windowList ;
            while (wp != NULL)
            {
                nwp = wp->next;
                if(((wp->flags & meWINDOW_NO_DELETE) == 0) && (wp != cwp))
                {
                    meWindowMakeCurrent(wp) ;
                    windowDelete(meFALSE,1) ;
                }
                wp = nwp ;
            }
            meWindowMakeCurrent(cwp) ;
            return meTRUE ;
        }
    }
#endif
    bufHistNo++ ;
    wp = frameCur->windowList ;
    while (wp != NULL)
    {
        nwp = wp->next;
        meVideoDetach(wp);               /* Detach all video blocks */
        if(wp != frameCur->windowCur)
        {
            if(--wp->buffer->windowCount == 0)
            {
                storeWindBSet(wp->buffer,wp) ;
                wp->buffer->histNo = bufHistNo ;
            }
            meFree(wp->modeLine);
            meFree(wp->dotCharOffset);
            meFree(wp);
        }
        wp = nwp ;
    }

    meAssert (frameCur->video.window == NULL);
    meAssert (frameCur->video.next == NULL);

    /* Fix up the meWindow pointers */
    frameCur->windowList = frameCur->windowCur ;                    /* Point to first window */
    frameCur->windowCount = 1;                     /* Only one window displayed */
    frameCur->windowCur->next = NULL ;              /* Reset linkage */
    frameCur->windowCur->prev = NULL ;
    meVideoAttach (&frameCur->video, frameCur->windowCur);      /* Attach to root video bloc */
    
    if((frameCur->windowCur->vertScroll -= frameCur->windowCur->frameRow - meFrameGetMenuDepth(frameCur)) < 0)
        frameCur->windowCur->vertScroll = 0 ;
    
    frameCur->windowCur->frameRow = meFrameGetMenuDepth(frameCur);
    frameCur->windowCur->frameColumn = 0;
    frameCur->windowCur->depth = frameCur->depth - meFrameGetMenuDepth(frameCur);
    frameCur->windowCur->width = frameCur->width;
    meWindowFixTextSize(frameCur->windowCur);
    
    return meTRUE ;
}

/*
 * Delete the current window, placing its space in the window above,
 * or, if it is the top window, the window below. Bound to C-X 0.
 */

/* ARGSUSED */
int
windowDelete(int f, int n)
{
    meWindow  *wp;                        /* window to recieve deleted space */
    meWindow  *pwp;                       /* Ptr to previous window */
    meWindow  *cwp;                       /* Ptr to current window */
    meWindow  *nwp;                       /* Ptr to next window */
#if MEOPT_HSPLIT
    meWindow **wlp;                       /* Window list pointer */
    meWindow  *wlist [meWINDOW_MAX+1];    /* The list of windows */
    int        deleteType;                /* The deltion type */
#endif

    /*
     * If we are in one window mode then bell
     */
    if(frameCur->windowList->next == NULL)
    {
        TTbell() ;
        return meFALSE ;
    }
    
    cwp = frameCur->windowCur ;
    nwp = cwp->next;
    pwp = cwp->prev;

#if MEOPT_HSPLIT
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
     * |pwp       |          |curWin   |       |curWin   |nwp      |
     * |          |          |         |       |         |         | 
     * +-----+----+          +---------+       +---------+---------+
     * |curWn|               |nwp
     * |     |               |
     * +-----+
     */
    if ((pwp == NULL) ||
        ((pwp->frameColumn+pwp->width) > (cwp->frameColumn+cwp->width)))
    {
        /* Determine if we merge the next down or right */
        if (nwp->frameColumn == cwp->frameColumn)
            deleteType = WINDOW_NEXT;
        else
            deleteType = WINDOW_RIGHT;
    }
    /* The end column of the previous is smaller then the current window
     * check the next start column to determine if we merge the next sibling
     * or the previous child.
     * 
     * Cases
     * -----
     * 
     * nwp == NULL                    |
     *                             pwp|
     * +----+---------+  OR  +--------+
     * |pwp |curWin   |      |curWin  |
     * |    |         |      |        |
     * +----+---------+      +--------+
     * 
     * nwp != NULL
     *                 
     * +----+-------+
     * |pwp |curWin |
     * |    |       |
     * +----+-------+
     * |nwp         |
     * |            |
     * +------------+
     */
    else if ((nwp == NULL) ||
             (nwp->frameColumn < cwp->frameColumn))
    {
        /* Determine if we merge the prev up or left */
        if ((pwp->frameColumn+pwp->width) == (cwp->frameColumn+cwp->width))
            deleteType = WINDOW_PREV;
        else
            deleteType = WINDOW_LEFT;
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
     * |curWin    |         |curWin     |
     * |          |         |           |
     * +----------+         +-----------+
     * |                    |
     */
    else if ((pwp->frameColumn+pwp->width) == (cwp->frameColumn+cwp->width))
    {
#if MEOPT_EXTENDED
        /* try to respect the prevoius windows depth lock */
        if((pwp->flags & meWINDOW_LOCK_DEPTH) &&
           (nwp->frameColumn == cwp->frameColumn))
            deleteType = WINDOW_NEXT;
        else
#endif
            deleteType = WINDOW_PREV;
    }
    /*
     * The next left column is the same as than the current left colunm
     * 
     * Cases
     * -----
     * 
     * +-----+---------------+
     * |pwp  | curWin        |
     * |     |               |
     * |     +------+--------+
     * |     |nwp   |        |
     * |     |      |        |
     * +-----+------+--------+
     */
    else if (nwp->frameColumn == cwp->frameColumn)
        deleteType = WINDOW_NEXT;
    /* The previous and next windows must both be in different columns */
    /*
     * The next top row is less than the current top row
     * 
     * Cases 
     * -----
     * 
     * +---------+-----+
     * |         | nwp |  
     * |         |     | 
     * +---+-----+     |
     * |   |curWn|     |
     * +---+     |     |
     * |pwp|     |     |
     * +---+-----+-----+
     */
    else if (nwp->frameRow < cwp->frameRow)
        deleteType = WINDOW_LEFT;
    /*
     * The prev end row is greater than the current end row
     * 
     * Cases 
     * -----
     * 
     * +----+-----+----+
     * | pwp|curWn| nwp|
     * |    |     |    |
     * |    +-----+----+
     * |    |          |
     * |    |          |
     * |    |          |
     * +----+----------+
     */
    else if ((pwp->frameRow+pwp->depth) > (cwp->frameRow+cwp->depth))
        deleteType = WINDOW_RIGHT;
    /*
     * The default case
     * 
     * Cases 
     * -----
     *  
     *     +--------+----
     *     |curWin  |nwp
     *  pwp|        |
     * ----+--------+
     */
#if MEOPT_EXTENDED
    /* try to respect the next windows width lock */
    else if(nwp->flags & meWINDOW_LOCK_WIDTH)
       deleteType = WINDOW_LEFT;
    else
#endif
       deleteType = WINDOW_RIGHT;

    /* Perform the deletion window movement. */
    wlp = meWindowGetAdjacentList (wlist, deleteType, cwp);
    meAssert (wlp != NULL);

    for  (wp = *wlp; wp != NULL; wp = *++wlp)
    {
        switch (deleteType)
        {
        case WINDOW_NEXT:           /* Window vertically down */
            wp->frameRow = cwp->frameRow ;
            wp->depth += cwp->depth ;
            break;
        case WINDOW_RIGHT:          /* Window horizontally to right */
            wp->frameColumn  = cwp->frameColumn;
            wp->width += cwp->width;
            
            /* Move virtual video frames */
            meVideoDetach (wp);
            meVideoAttach (cwp->video, wp);
            break;
        case WINDOW_PREV:           /* Window vertically up */
            wp->depth += cwp->depth;
            break;
        case WINDOW_LEFT:           /* Window horizontally to left */
            wp->width += cwp->width;
            break;
        }

        /* Set window flags for update - note we always set the
         * mode line. This is not always strictly necessary but
         * it is not worth working out what should and should not
         * be strictly set */
        meWindowFixTextSize(wp);         /* Fix text window size */
    }
#else
    if (nwp != NULL)
    {
        nwp->frameRow = cwp->frameRow ;
        nwp->depth += cwp->depth ;
        cwp = nwp ;
    }
    else
    {
        pwp->depth += cwp->depth;
        cwp = pwp ;
    }

    meWindowFixTextSize(cwp);         /* Fix text window size */
#endif
    
    /* Fix up the current Window pointers and get ready to delete by
     * unchaining from the window list */
    if (pwp == NULL)
        frameCur->windowList = nwp;
    else
        pwp->next = nwp;
    if (nwp != NULL)
        nwp->prev = pwp;
    
    wp = frameCur->windowCur ;
    {
        /* Get rid of the current window */
        meBuffer *bp=wp->buffer ;
        if(--bp->windowCount == 0)
            storeWindBSet(bp,wp) ;
    }

    /* Delete the current window */
    meVideoDetach(wp);                   /* Detach from the video block */
    frameCur->windowCount--;                       /* One less window displayed */

    /* Determine the next window to make the current window */
#if MEOPT_HSPLIT
    if ((deleteType == WINDOW_LEFT) || (deleteType == WINDOW_PREV))
        meWindowMakeCurrent(pwp);               /* Previous window is IT !! */
    else
        meWindowMakeCurrent(nwp);               /* Next window is IT !! */
#else
    meWindowMakeCurrent(cwp);
#endif
    /* Must free AFTER meWindowMakeCurrent as this dicks with frameCur->windowCur */
    meFree(wp->modeLine);                  /* Free off the old mode line */
    meFree(wp->dotCharOffset);
    meFree(wp);                         /* and finally the window itself */
    return meTRUE ;
}

/*
   Split the current window.  A window smaller than 3 lines cannot be
   split.  An argument of 1 forces the cursor into the upper window, an
   argument of two forces the cursor to the lower window.  The only other
   error that is possible is a "malloc" failure allocating the structure
   for the new window.  Bound to "C-X 2".
 */
int
windowSplitDepth(int f, int n)
{
    register meWindow *wp;
    register meLine   *lp, *off;
    register int    ntru, stru;
    register int    ntrl, strl;
    register int    ntrd;

    if (frameCur->windowCur->textDepth < 3)
        return mlwrite(MWABORT,(meUByte *)"Cannot split a %d line window",frameCur->windowCur->textDepth);
    if (frameCur->windowCount == meWINDOW_MAX)
        return mlwrite(MWABORT,(meUByte *)"Cannot create more than %d windows",frameCur->windowCount);
    if(((wp = (meWindow *) meMalloc(sizeof(meWindow))) == NULL) || 
       ((lp=lalloc(frameCur->widthMax)) == NULL) || ((off=lalloc(frameCur->widthMax)) == NULL))
    {
        meNullFree (wp);                /* Destruct created window */
        return (meFALSE);                 /* Fail */
    }
    /* reset the window flags for both as these tend to cause more trouble
     * than they are worth */
#if MEOPT_EXTENDED
    frameCur->windowCur->flags = 0 ;
#endif
    memcpy(wp,frameCur->windowCur,sizeof(meWindow)) ;
    
    wp->modeLine = lp ;
    off->next = NULL ;
    wp->dotCharOffset = off ;
    (frameCur->bufferCur->windowCount)++;                  /* Displayed once more.  */
    frameCur->windowCount++;                       /* One more window displayed */

    meVideoAttach (frameCur->windowCur->video, wp); /* Attach to frameCur->vvideo block */

    ntru = (frameCur->windowCur->textDepth-1) / 2;     /* Upper size           */
    ntrl = (frameCur->windowCur->textDepth-1) - ntru;  /* Lower size           */
    stru = (frameCur->windowCur->depth)/2;           /* Upper window size    */
    strl = (frameCur->windowCur->depth) - stru;      /* Lower window size    */
    ntrd = frameCur->windowCur->dotLineNo - frameCur->windowCur->vertScroll ;
    if (((f == meFALSE) && (ntru >= ntrd)) || ((f == meTRUE) && (n == 1)))
    {
        /* Old is upper window. */
        if(ntrd == ntru)           /* Hit mode line.       */
            frameCur->windowCur->vertScroll++ ;
        frameCur->windowCur->textDepth = ntru;
        frameCur->windowCur->depth = stru;

        /* Maintain the window prevous/next linkage. */
        if ((wp->next = frameCur->windowCur->next) != NULL)
            wp->next->prev = wp;
        frameCur->windowCur->next = wp;
        wp->prev = frameCur->windowCur;

        wp->frameRow = frameCur->windowCur->frameRow+ntru+1;
        wp->textDepth = ntrl;
        wp->depth   = strl;
    }
    else
    {
        /* Old is lower window  */
        wp->frameRow = frameCur->windowCur->frameRow;
        wp->textDepth = ntru;
        wp->depth   = stru;
        
        ++ntru;                         /* Mode line.           */
        frameCur->windowCur->frameRow += ntru;
        frameCur->windowCur->textDepth = ntrl;
        frameCur->windowCur->depth = strl;
        frameCur->windowCur->vertScroll += ntru ;

        /* Maintain the window prevous/next linkage. */
        if ((wp->prev = frameCur->windowCur->prev) != NULL)
            wp->prev->next = wp;
        else
            frameCur->windowList = wp;
        frameCur->windowCur->prev = wp;
        wp->next = frameCur->windowCur;
    }
    
    wp->vertScroll = frameCur->windowCur->vertScroll ;
    meWindowFixTextSize (frameCur->windowCur);              /* Fix text window size */
    meWindowFixTextSize (wp);                 /* Fix text window size */
    return meTRUE ;
}

#if MEOPT_HSPLIT
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
windowSplitWidth(int f, int n)
{
    register meWindow *wp;
    register meLine   *lp, *off;

    /* Out minimum split value horizontally is 7 i.e. |$c$|$c$| */
    if (frameCur->windowCur->textWidth < ((gsbarmode & WMVWIDE) ? 8 : 7))
        return mlwrite(MWABORT,(meUByte *)"Cannot split a %d column window", frameCur->windowCur->textWidth);
    if (frameCur->windowCount == meWINDOW_MAX)
        return mlwrite(MWABORT,(meUByte *)"Cannot create more than %d windows",frameCur->windowCount);
    if(((wp = (meWindow *) meMalloc(sizeof(meWindow))) == NULL) ||
       ((lp=lalloc(frameCur->widthMax)) == NULL) || ((off=lalloc(frameCur->widthMax)) == NULL))
    {
        meNullFree (wp);                /* Destruct window */
        return (meFALSE);                 /* and Fail */
    }
    /* reset the window flags for both as these tend to cause more trouble
     * than they are worth */
#if MEOPT_EXTENDED
    frameCur->windowCur->flags = 0 ;
#endif
    memcpy(wp,frameCur->windowCur,sizeof(meWindow)) ;
    if (meVideoAttach (NULL, wp) == meFALSE)
    {
        meFree (lp);                       /* Destruct line */
        meFree (off);                      /* Destruct line */
        meFree (wp);                       /* Destruct window */
        return (meFALSE);                    /* Failed */
    }

    /* All storage allocated. Build the new window */
    frameCur->windowCount++;               /* One more window displayed */
    wp->modeLine = lp ;                    /* Attach mode line */
    off->next = NULL ;
    wp->dotCharOffset = off ;              /* Attach current line offset buffer */
    (frameCur->bufferCur->windowCount)++;  /* Displayed once more.  */

    /* Maintain the window prevous/next linkage. */
    if ((wp->next = frameCur->windowCur->next) != NULL)
        wp->next->prev = wp;
    frameCur->windowCur->next = wp;
    wp->prev = frameCur->windowCur;

    /* Compute the new size of the windows and reset the width counters and
     * the margins */
    frameCur->windowCur->width /= 2;
    wp->width -= frameCur->windowCur->width;
    wp->frameColumn  = frameCur->windowCur->frameColumn + frameCur->windowCur->width;
    meWindowFixTextSize(wp);
    meWindowFixTextSize(frameCur->windowCur);

    if ((f != meFALSE) && (n == 2))
        meWindowMakeCurrent(wp);               /* Next window is IT !! */
    return (meTRUE);
}
#endif

/*
 * Enlarge the current window - adds n lines to the current window.
 *
 * Find the window that loses space. Make sure it is big enough. If so,
 * hack the window descriptions, and ask redisplay to do all the hard work.
 * You don't just set "force reframe" because dot would move.
 * Bound to "C-X Z".
 */

int
windowGrowDepth(int f, int n)
{
    meWindow *wp;                         /* Temporary window pointer */
    meWindow **twlp;                      /* Temporary window list pointer */
    meWindow *bwlist [meWINDOW_MAX+1];        /* Bottom window list */
    meWindow *twlist [meWINDOW_MAX+1];        /* Top window list */
    int     ii;                         /* Local loop counter */
    int     jj;                         /* Used as minimum window size */

#if MEOPT_HSPLIT
    /* Get the resize lists - these are the windows that under-go change,
     * made a little more complicated by the presence of horizontal split
     * windows. In this case we have to consider other windows adjacent to
     * "frameCur->windowCur" that may undergo a change is size because of the split */
    if (meWindowGetAdjacentList (bwlist, WINDOW_NEXT, frameCur->windowCur) == NULL)
    {
        n = 0 - n;
        if (meWindowGetAdjacentList (twlist, WINDOW_PREV, frameCur->windowCur) == NULL)
            return mlwrite(MWCLEXEC|MWABORT,(meUByte *)"[Only one vertical window]");
        meWindowGetAdjacentList (bwlist, WINDOW_NEXT, twlist[0]);
    }
    else
        meWindowGetAdjacentList (twlist, WINDOW_PREV, bwlist[0]);
#else
    if (frameCur->windowList->next == NULL)
        return mlwrite(MWCLEXEC|MWABORT,(meUByte *)"[Only one window]");
    if ((bwlist[0]=frameCur->windowCur->next) == NULL)
    {
        n   = 0 - n ;
        bwlist[0] = frameCur->windowCur ;
        twlist[0] = frameCur->windowList;
        while(twlist[0]->next != frameCur->windowCur)
            twlist[0] = twlist[0]->next;
    }
    else
        twlist[0] = frameCur->windowCur ;
    bwlist[1] = twlist[1] = NULL ;
#endif

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
        if (wp->textDepth <= jj)
            return mlwrite(MWCLEXEC|MWABORT,(meUByte *)"[Impossible change]");

    /* Repostion the top line in the bottom windows */
    for (jj = 0; (wp = bwlist[jj]) != NULL; jj++)
        if((wp->vertScroll+=n) < 0)
            wp->vertScroll = 0 ;

    /* Resize all of the windows BELOW frameCur->windowCur */
    for (ii = 0; (wp = twlist[ii]) != NULL; ii++)
    {
        wp->depth += n;
        meWindowFixTextSize (wp);
    }

    /* Resize all of the windows */
    for (ii = 0; (wp = bwlist[ii]) != NULL; ii++)
    {
        wp->frameRow += n;
        wp->depth -= n;
        meWindowFixTextSize(wp);
    }
    return (meTRUE);

}

/*
 * Shrink the current window. Find the window that gains space. Hack at the
 * window descriptions. Ask the redisplay to do all the hard work. Bound to
 * "C-X C-Z".
 */
int
windowShrinkDepth(int f, int n)
{
    return windowGrowDepth(f,0-n) ;
}


/*      Resize the current window to the requested size */

int
windowResizeDepth(int f, int n)
{
    int clines;                         /* current # of lines in window */

    if (f == meFALSE)                     /* Must have a non-default argument */
        return(meTRUE);                   /* else ignore call */
    clines = frameCur->windowCur->textDepth;           /* Find out what to do */
    if (clines == n)                    /* Already the right size? */
        return(meTRUE);                   /* Yes - quit */

    return(windowGrowDepth(meTRUE, n - clines));
}

#if MEOPT_HSPLIT
/*
 * Enlarge the current window - adds n columns to the current window.
 *
 * Find the windows that lose space. Make sure it is big enough. If so,
 * hack the window descriptions, and ask redisplay to do all the hard work.
 * You don't just set "force reframe" because dot would move.
 * Bound to "C-X Z".
 */

int
windowGrowWidth(int f, int n)
{
    meWindow *wp;                         /* Temporary window pointer */
    meWindow **twlp;                      /* Temporary window list pointer */
    meWindow *rwlist [meWINDOW_MAX+1];        /* Right set of windows */
    meWindow *lwlist [meWINDOW_MAX+1];        /* Left set of windows */
    int     ii;                         /* Local loop counter */
    int     jj;                         /* Used as minimum window size */

    /* Quick check - we may be the only window */
    if (frameCur->windowList == NULL)
        goto one_window;

    /* Get the resize lists - these are the windows that under-go change,
     * made a little more complicated by the presence of horizontal split
     * windows. In this case we have to consider other windows adjacent to
     * "frameCur->windowCur" that may undergo a change is size because of the split */
    if (meWindowGetAdjacentList (rwlist, WINDOW_RIGHT, frameCur->windowCur) == NULL)
    {
        n = 0 - n;
        if (meWindowGetAdjacentList (lwlist, WINDOW_LEFT, frameCur->windowCur) == NULL)
            goto one_window;
        meWindowGetAdjacentList (rwlist, WINDOW_RIGHT, lwlist[0]);
    }
    else
        meWindowGetAdjacentList (lwlist, WINDOW_LEFT, rwlist[0]);

    meAssert (rwlist[0] != NULL);
    meAssert (lwlist[0] != NULL);

    /* Compute the minumum size of the windows */
    jj = frameCur->width+1;
    twlp = (n < 0) ? lwlist : rwlist;
    for (ii = 0; (wp = twlp [ii]) != NULL; ii++)
        if (wp->textWidth < jj)
            jj = wp->textWidth;
    jj -= (gsbarmode & WMVWIDE) ? 3:2;   /* Minimum of 3 columns !! */

    /* Do the resizing of the windows */
    if (n < 0)
    {
        if (jj <= -n)
            goto enlarge_imp;
    }
    else if (jj <= n)
        goto enlarge_imp;

    /* Resize all of the windows LEFT frameCur->windowCur */
    for (ii = 0; (wp = lwlist[ii]) != NULL; ii++)
    {
        wp->width += n;
        meWindowFixTextSize(wp);
    }

    /* Resize all of the windows to the RIGHT of frameCur->windowCur */
    for (ii = 0; (wp = rwlist[ii]) != NULL; ii++)
    {
        wp->frameColumn += n;
        wp->width -= n;
        meWindowFixTextSize(wp);
    }
    return (meTRUE);

    /* Handle any errors with a report */
enlarge_imp:
    return mlwrite(MWCLEXEC|MWABORT,(meUByte *)"[Impossible change]");
one_window:
    return mlwrite(MWCLEXEC|MWABORT,(meUByte *)"[Only one horizontal window]");
}

/* Shrink the current window - resize by columns
 * Find the window that gains space. Hack at the window descriptions.
 * Ask the redisplay to do all the hard work.
 * Bound to "C-X C-Z". */
int
windowShrinkWidth(int f, int n)
{
    return windowGrowWidth (f, 0-n);    /* Get enlarge to do work */
}

/* Resize the current window - size to requested columns */
int
windowResizeWidth(int f, int n)
{
    int ccols;                           /* current # of columns in window */

    if (f == meFALSE)                     /* Must have a non-default arg .. */
        return(meTRUE);                   /* .. otherwise ignore */
    ccols = frameCur->windowCur->textWidth;            /* find out what to do */
    if (ccols == n)                     /* already the right size? */
        return(meTRUE);                   /* Yes - finish */

    return (windowGrowWidth (meTRUE, n - ccols));
}
#endif

/*
 * Pick a window for a pop-up. Split the screen if there is only one window.
 * Pick the uppermost window that isn't the current window. An LRU algorithm
 * might be better. Return a pointer, or NULL on error.
 */

meWindow  *
meWindowPopup(meUByte *name, int flags, meBuffer **bufferReplaced)
{
    meWindow *wp=NULL;
    meBuffer *bp;
    
    if(bufferReplaced != NULL)
        *bufferReplaced = NULL ;
    if(name != NULL)
    {
        if((bp=bfind(name,(flags & 0xff))) == NULL)
            return NULL ;
        if(bp == frameCur->bufferCur)
        {
            /* It is already the current buffer so return the current window
             * One complication is that this is often used with BFND_CLEAR
             * so the current buffer is NACT and the fhook needs to be
             * executed etc. The best way is to "swap" the buffer
             */
            if(meModeTest(bp->mode,MDNACT))
                swbuffer(frameCur->windowCur,frameCur->bufferCur);
            return frameCur->windowCur ;
        }
        /* ZZZZ - pop-up on this frame */
        if(bp->windowCount > 0)
        {
            for(wp=frameCur->windowList ; (wp != NULL) && (wp->buffer != bp) ; wp=wp->next)
                ;
        }
        if((wp == NULL) && (flags & WPOP_EXIST))
            return NULL ;
    }
    if((wp == NULL) && (flags & WPOP_USESTR))
    {
        wp = frameCur->windowCur ;
        for(;;)
        {
            if((wp = wp->next) == NULL)
                wp = frameCur->windowList ;
            if(wp == frameCur->windowCur)
            {
                wp = NULL ;
                break ;
            }
            if((wp->buffer->name[0] == '*')
#if MEOPT_EXTENDED
               && ((wp->flags & meWINDOW_LOCK_BUFFER) == 0)
#endif
               )
                break ;
        }
    }
    if(wp == NULL)
    {
#if MEOPT_EXTENDED
        wp = frameCur->windowCur ;
        for(;;)
        {
            if((wp = wp->next) == NULL)
                wp = frameCur->windowList ;
            if(wp == frameCur->windowCur)
            {
                wp = NULL ;
                break ;
            }
            if((wp->flags & meWINDOW_LOCK_BUFFER) == 0)
                    break ;
        }
        if(wp == NULL)
        {
            meUShort flags ;
            /* maintain the current windows flags */
            flags = frameCur->windowCur->flags ;
            wp = frameCur->windowCur->next ;
            windowSplitDepth(meFALSE, 0) ;
            frameCur->windowCur->flags = flags ;
            if(frameCur->windowCur->next == wp)
            {
                /* next is the same so we want the previous */
                wp = frameCur->windowList ;
                while(wp->next != frameCur->windowCur)
                    wp = wp->next ;
            }
            else
                wp = frameCur->windowCur->next ;
            /* splitting a window so there is no replacement */
            bufferReplaced = NULL ;
        }
#else
        if(frameCur->windowList->next == NULL)
        {
            if(windowSplitDepth(meFALSE, 0) != meTRUE)
                return NULL ;
            /* splitting a window so there is no replacement */
            bufferReplaced = NULL ;
        }
        if((wp = frameCur->windowCur->next) == NULL)
            wp = frameCur->windowList ;
#endif
    }
    if(name != NULL)
    {
        if(bufferReplaced != NULL)
            *bufferReplaced = wp->buffer ;
        swbuffer(wp,bp);
    }
    if(flags & WPOP_MKCURR)
        meWindowMakeCurrent(wp) ;

    return wp ;
}

int
windowPopup(int f, int n)
{
    meUByte bufn[meBUF_SIZE_MAX], *nn ;
    int s ;
    
    /* temporary arg 2 to popup frame */
    if(n & 2)
    {
        meFrameTermMakeCur(frameCur) ;
        return meTRUE ;
    }
    if((s = getBufferName((meUByte *)"Popup buffer", 0, 2, bufn)) != meTRUE)
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
    if(meWindowPopup(nn,n,NULL) == NULL)
        return meFALSE ;
    return meTRUE ;
}

#if MEOPT_EXTENDED
int
windowScrollNextUp(int f, int n)          /* scroll the next window up (back) a page */
{
    windowGotoNext(meFALSE, 1);
    windowScrollUp(f, n);
    return(windowGotoPrevious(meFALSE, 1));
}

int
windowScrollNextDown(int f, int n)          /* scroll the next window down (forward) a page */
{
    windowGotoNext(meFALSE, 1);
    windowScrollDown(f, n);
    return (windowGotoPrevious(meFALSE, 1));
}
#endif

/*
 * Re-arrange the windows on the screen automatically.
 */

int
frameResizeWindows (int f, int n)
{
    meWindow *wp;                       /* Current window pointer */
    meWindow *pwp;                      /* Previous window pointer */
    int row [meWINDOW_MAX];             /* Keep the start row number */
    int col [meWINDOW_MAX];             /* Keep the start col number */
    int maxrow = 0;                     /* Maximum number of rows */
    int maxcol = 0;                     /* Maximum number of columns */
    int colwidth;                       /* Width of a colunm */
    int rowdepth;                       /* Depth of a row */
    int wid;                            /* Window indenty */
    int twid;                           /* Temporary window identity */
    int ii, jj;                         /* Temporary loop counters */

    pwp = frameCur->windowList;                       /* Previous window is head of list */
    wp = pwp->next;                   /* Current window is the next */

    row [0] = 0;                        /* First window must be (0,0) */
    col [0] = 0;
    wid = 1;

    /* Iterate down the list of windows determining the starting row and
     * column numbers of each */
    while (wp != NULL)
    {
#if MEOPT_HSPLIT
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
        if (pwp->frameRow == wp->frameRow)
        {
            row [wid] = row [wid-1];
            col [wid] = col [wid-1] + 1;
        }
        else
#endif
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
        if (pwp->frameColumn == wp->frameColumn)
        {
            row [wid] = row [wid-1] + 1;
            col [wid] = col [wid-1];
        }
#if MEOPT_HSPLIT
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
        else if (pwp->frameColumn > wp->frameColumn)
        {
            meWindow *twp;                /* Temporary window pointer */
            ii = 0;                     /* Maximum row count */
            twid = wid;                 /* Temporary window identity */
            twp = wp;                   /* Point to current block */

            do
            {
                twp = twp->prev;      /* Previous block */
                twid--;                 /* Previous identity */
                if (row [twid] > ii)    /* This row greater than current */
                    ii = row [twid];    /* Yes - save it !! */
            } while (twp->frameColumn != wp->frameColumn);

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
        else if (pwp->frameRow > wp->frameRow)
        {
            meWindow *twp;                /* Temporary window pointer */
            ii = 0;                     /* Maximum column count */
            twid = wid;                 /* Temporary window indentity */
            twp = wp;                   /* Point to current block */

            do
            {
                twp = twp->prev;      /* Previous block */
                twid--;                 /* Previous identity */
                if (col [twid] > ii)    /* This column greater than current */
                    ii = col [twid];    /* Yes - save it !! */
            } while (twp->frameRow != wp->frameRow);

            row [wid] = row [twid];     /* Propogate row depth */
            col [wid] = ii + 1;         /* wp must be the next column */
        }
        else
            return mlwrite(MWABORT,(meUByte *)"Cannot get into this state !!");
#endif
        /* Increment the counters, move onto the next row and accumulate
         * the row and column maximums */
        if (row [wid] > maxrow)
            maxrow = row [wid];         /* Save maximum row */
        if (col [wid] > maxcol)
            maxcol = col [wid];         /* Save maximum column */

        wid++;                          /* Next window */
        pwp = wp;                       /* Advance pointers */
        wp = wp->next;
    }

    /* Resize the windows */
    maxrow++;                           /* The maximum number of rows */
    maxcol++;                           /* The maximum number of colunms */

    colwidth = frameCur->width / maxcol;    /* Unit width of a colunm */
#if MEOPT_OSD
    rowdepth = (frameCur->depth-meFrameGetMenuDepth(frameCur))/maxrow;  /* Unit width of a row */
#else
    rowdepth = frameCur->depth / maxrow;    /* Unit width of a row */
#endif
    /* Make sure that the windows can be re-sized. If they fall below the
     * minimum then delete all windows with the exception of 1 */
    if ((colwidth < 6) || (rowdepth < 2))
        return windowDeleteOthers(0,0);

    /* Iterate down the list of windows determining the ending row and
     * column numbers of each */

    wp = frameCur->windowList;
    for (wid = 0; wid < frameCur->windowCount; wid++)
    {
        /* Find the end colunm. Search forward until we find another window
         * with the same top row. */
        for (twid = wid + 1; twid < frameCur->windowCount; twid++)
        {
            ii = row [wid];             /* Get current start row */
            if (row [twid] <= ii)       /* Same starting row ?? */
            {
                ii = col [twid];        /* Save the end column */
                break;                  /* Quit - located end colunm */
            }
        }
        if (twid == frameCur->windowCount)         /* Got to the end ?? */
            ii = maxcol;                /* Yes - must be the maximum column */

        /* Find the end row. Search forward until we find another window
         * with the same starting colunm */
        for (twid = wid + 1; twid < frameCur->windowCount; twid++)
        {
            jj = col [wid];             /* Get current start colunm */
            if (col [twid] <= jj)       /* Lower/equal starting colunm ?? */
            {
                jj = row [twid];        /* Save the end row */
                break;
            }
        }
        if (twid == frameCur->windowCount)
            jj = maxrow;

        /* Size Vertically - Assign the new row indicies */
        if ((f == meFALSE) || (n > 0))
        {
            jj = (jj == maxrow) ? frameCur->depth : (jj * rowdepth) + meFrameGetMenuDepth(frameCur);
            wp->frameRow = (row [wid] * rowdepth) + meFrameGetMenuDepth(frameCur);
            wp->depth = jj - wp->frameRow;
        }

        /* Size Horizontally - Assign the colunm indicies */
        if ((f == meFALSE) || (n < 0))
        {
            ii = (ii == maxcol) ? frameCur->width : (ii * colwidth);
            wp->frameColumn = col [wid] * colwidth;
            wp->width = ii - wp->frameColumn;
        }
        
        /* Reframe - Update all of the windows to reflect the new sizes */
        meWindowFixTextSize(wp);
        wp = wp->next;                /* Move onto the next */
    }
    sgarbf = meTRUE;                      /* Garbage the screen */
/*    mlwrite (MWABORT, "There are %d rows and %d cols !!", maxrow, maxcol);*/
    return meTRUE;

}


#if MEOPT_POSITION
int
positionSet(int f, int n)		/* save ptr to current window */
{
    register mePosition *pos ;
    meUShort mark ;                     /* Position alpha mark name*/
    int cc ;
    
    if((cc = mlCharReply((meUByte *)"Set position: ",mlCR_QUIT_ON_USER,NULL,NULL)) == -2)
        cc = mlCharReply((meUByte *)"Set position: ",mlCR_ALPHANUM_CHAR,NULL,NULL) ;
    
    if(cc < 0)
        return ctrlg(meFALSE,1) ;
    
    pos = position ;
    mark = meAM_FRSTPOS ;
    while((pos != NULL) && (cc != (int) pos->name))
    {
        pos = pos->next ;
        mark += 2 ;
    }
    if(pos == NULL)
    {
        pos = meMalloc(sizeof(mePosition)) ;
        if(pos == NULL)
            return meABORT ;
        pos->next = position ;
        position = pos ;
        pos->name = cc ;
        pos->line_amark = mark ;
    }
    if(f == meFALSE)
        n = mePOS_DEFAULT ;
    if(frameCur->windowCur->markLine == NULL)
        n &= ~(mePOS_MLINEMRK|mePOS_MLINENO|mePOS_MLINEOFF) ;
    
    pos->flags = n ;
    
    if(n & mePOS_WINDOW)
    {
        pos->window = frameCur->windowCur ;
        /* store window dimentions so we can pick the best */
        pos->winMinRow = frameCur->windowCur->frameRow ;
        pos->winMinCol = frameCur->windowCur->frameColumn ;
        pos->winMaxRow = frameCur->windowCur->frameRow + frameCur->windowCur->width - 1 ;
        pos->winMaxCol = frameCur->windowCur->frameColumn + frameCur->windowCur->depth - 1 ;
    }
    if(n & mePOS_BUFFER)
        pos->buffer = frameCur->bufferCur ;
    if(n & mePOS_LINEMRK)
    {
        if(alphaMarkSet(frameCur->bufferCur,pos->line_amark,frameCur->windowCur->dotLine,0,1) != meTRUE)
        {
            pos->flags = 0 ;
            return meABORT ;
        }
    }
    if(n & mePOS_LINENO)
        pos->dotLineNo = frameCur->windowCur->dotLineNo ;
    if(n & mePOS_LINEOFF)
        pos->dotOffset = frameCur->windowCur->dotOffset ;
    if(n & mePOS_MLINEMRK)
    {
        if(alphaMarkSet(frameCur->bufferCur,(meUShort) (pos->line_amark+1),frameCur->windowCur->markLine,0,1) != meTRUE)
        {
            pos->flags = 0 ;
            return meABORT ;
        }
    }
    if(n & mePOS_MLINENO)
        pos->markLineNo = frameCur->windowCur->markLineNo ;
    if(n & mePOS_MLINEOFF)
        pos->markOffset = frameCur->windowCur->markOffset ;
    if(n & mePOS_WINYSCRL)
        pos->vertScroll = frameCur->windowCur->vertScroll ;
    if(n & mePOS_WINXCSCRL)
        pos->horzScroll = frameCur->windowCur->horzScroll ;
    if(n & mePOS_WINXSCRL)
        pos->horzScrollRest = frameCur->windowCur->horzScroll ;
    
    return meTRUE ;
}

int
positionGoto(int f, int n)		/* restore the saved screen */
{
    register mePosition *pos ;
    int ret=meTRUE, sflg ;
    int cc ;
    
    if(n == -1)
    {
        while(position != NULL)
        {
            pos = position ;
            position = pos->next ;
            free(pos) ;
        }
        return meTRUE ;
    }
    
    if((cc = mlCharReply((meUByte *)"Goto position: ",mlCR_QUIT_ON_USER,NULL,NULL)) == -2)
        cc = mlCharReply((meUByte *)"Goto position: ",mlCR_ALPHANUM_CHAR,NULL,NULL) ;
    if(cc < 0)
        return ctrlg(meFALSE,1) ;
    
    pos = position ;
    while((pos != NULL) && (cc != (int) pos->name))
        pos = pos->next ;
    
    if(pos == NULL)
    {
        meUByte    allpos[256]; 	/* record of the positions	*/
        int      ii = 0;
        
        pos = position ;
        while(pos != NULL)
        {
            if(pos->name < 128)
            {
                allpos[ii++] = ' ' ;
                allpos[ii++] = (meUByte) pos->name;
            }
            pos = pos->next;
        }
        if(ii == 0)
            return mlwrite(MWABORT|MWCLEXEC,(meUByte *)"[No positions set]");
        allpos[ii] = '\0';
        return mlwrite(MWABORT|MWCLEXEC,(meUByte *)"[Valid positions:%s]", allpos);
    }
    
    if(f == meFALSE)
        n = pos->flags ;
    else
        n &= pos->flags ;
    
    if(n & mePOS_WINDOW)
    {
        /* find the window */
        register meWindow *wp, *bwp=NULL;
        int off, boff, ww, dd ;
        
        if(frameCur->windowList->next == NULL)
            bwp = frameCur->windowList ;
        if(bwp == NULL)
        {
            wp = frameCur->windowList;
            while (wp != NULL)
            {
                if(wp == pos->window)
                {
                    bwp = wp ;
                    break ;
                }
                wp = wp->next;
            }
        }
        if(bwp == NULL)
        {
            wp = frameCur->windowList;
            while (wp != NULL)
            {
                /* calculate how close this window matches the original -
                 * calculate the amount this window covers of the old window */
                ww = (pos->winMaxRow < (wp->frameRow + wp->width - 1)) ? pos->winMaxRow : (wp->frameRow + wp->width - 1) ;
                ww = ww - ((pos->winMinRow > wp->frameRow) ? pos->winMinRow : wp->frameRow) ;
                dd = (pos->winMaxCol < (wp->frameColumn + wp->width - 1)) ? pos->winMaxCol : (wp->frameColumn + wp->width - 1) ;
                dd = ww - ((pos->winMinCol > wp->frameColumn) ? pos->winMinCol : wp->frameColumn) ;
                off = ww * dd ;
                if((ww < 0) && (dd < 0))
                    off = -off ;
                if((bwp == NULL) || (off > boff))
                {
                    bwp = wp ;
                    boff = off ;
                }
                wp = wp->next;
            }
        }
        if(bwp != frameCur->windowCur)
            meWindowMakeCurrent(bwp) ;
    }
    if(n & mePOS_BUFFER)
    {
        /* find the buffer */
        register meBuffer *bp;
        bp = bheadp;
        while (bp != NULL)
        {
            if(bp == pos->buffer)
            {
                if(bp != frameCur->bufferCur)
                    swbuffer(frameCur->windowCur,bp) ;
                break ;
            }
            bp = bp->next;
        }
        if(bp == NULL)
        {
            /* print the warning and ensure no other restoring takes place */
            ret = meFALSE ;
            n &= mePOS_WINDOW ;
        }
    }
    /* restore the mark first */
    if(n & mePOS_MLINEMRK)
    {
        if((ret = alphaMarkGet(frameCur->bufferCur,(meUShort) (pos->line_amark+1))) == meTRUE)
        {
            frameCur->windowCur->markLine = frameCur->bufferCur->dotLine ;
            frameCur->windowCur->markLineNo = frameCur->bufferCur->dotLineNo ;
            frameCur->windowCur->markOffset = 0 ;
        }
    }
    else if(n & mePOS_MLINENO)
    {
        meLine *dotp=frameCur->windowCur->dotLine ;
        meInt dotLineNo=frameCur->windowCur->dotLineNo ;
        meUShort doto=frameCur->windowCur->dotOffset ;
        
        if((ret = windowGotoLine(1,pos->markLineNo+1)) == meTRUE)
        {
            frameCur->windowCur->markLine = frameCur->windowCur->dotLine ;
            frameCur->windowCur->markLineNo = frameCur->windowCur->dotLineNo ;
            frameCur->windowCur->markOffset = 0 ;
        }
        frameCur->windowCur->dotLine = dotp ;
        frameCur->windowCur->dotLineNo = dotLineNo ;
        frameCur->windowCur->dotOffset = doto ;
    }
    if(n & mePOS_MLINEOFF)
    {
        if(frameCur->windowCur->markLine == NULL)
            ret = meFALSE ;
        else if(pos->markOffset > meLineGetLength(frameCur->windowCur->markLine))
        {
            frameCur->windowCur->markOffset = meLineGetLength(frameCur->windowCur->markLine) ;
            ret = meFALSE ;
        }
        else
            frameCur->windowCur->markOffset = pos->markOffset ;
    }
    
    if(n & mePOS_LINEMRK)
    {
        if((ret = alphaMarkGet(frameCur->bufferCur,pos->line_amark)) == meTRUE)
        {
            frameCur->windowCur->dotLine = frameCur->bufferCur->dotLine ;
            frameCur->windowCur->dotLineNo = frameCur->bufferCur->dotLineNo ;
            frameCur->windowCur->dotOffset = 0 ;
            frameCur->windowCur->updateFlags |= WFMOVEL ;
        }
        else
            /* don't restore the offset or scrolls */
            n &= (mePOS_WINDOW|mePOS_BUFFER) ;
    }
    else if((n & mePOS_LINENO) && ((ret = windowGotoLine(1,pos->dotLineNo+1)) != meTRUE))
        /* don't restore the offset or scrolls */
        n &= (mePOS_WINDOW|mePOS_BUFFER) ;
    
    if(n & mePOS_LINEOFF)
    {
        if(pos->dotOffset > meLineGetLength(frameCur->windowCur->dotLine))
        {
            frameCur->windowCur->dotOffset = meLineGetLength(frameCur->windowCur->dotLine) ;
            ret = meFALSE ;
            /* don't restore the x scrolls */
            n &= (mePOS_WINDOW|mePOS_WINYSCRL|mePOS_BUFFER|mePOS_LINEMRK|mePOS_LINENO) ;
        }
        else
            frameCur->windowCur->dotOffset = pos->dotOffset ;
    }
    
    sflg = 0 ;
    if(n & mePOS_WINYSCRL)
    {
        if(frameCur->bufferCur->lineCount && (pos->vertScroll >= frameCur->bufferCur->lineCount))
            pos->vertScroll = frameCur->bufferCur->lineCount-1 ;
        if(frameCur->windowCur->vertScroll != pos->vertScroll)
        {
            frameCur->windowCur->vertScroll = pos->vertScroll ;
            frameCur->windowCur->updateFlags |= WFMAIN|WFSBOX|WFLOOKBK ;
            sflg = 1 ;
        }
    }
    if(n & mePOS_WINXCSCRL)
    {
        if(pos->horzScroll >= meLineGetLength(frameCur->windowCur->dotLine))
            pos->horzScroll = meLineGetLength(frameCur->windowCur->dotLine)-1 ;
        if(frameCur->windowCur->horzScroll != pos->horzScroll)
        {
            frameCur->windowCur->horzScroll = pos->horzScroll ;
            frameCur->windowCur->updateFlags |= WFREDRAW ;        /* Force a screen update */
            sflg = 1 ;
        }
    }
    if(n & mePOS_WINXSCRL)
    {
        if(frameCur->windowCur->horzScrollRest != pos->horzScrollRest)
        {
            frameCur->windowCur->horzScrollRest = pos->horzScrollRest ;
            frameCur->windowCur->updateFlags |= WFREDRAW ;        /* Force a screen update */
            sflg = 1 ;
        }
    }
    if(sflg)
        updCursor(frameCur->windowCur) ;
    
    if(!ret)
        return mlwrite(MWCLEXEC|MWABORT,(meUByte *)"[Failed to restore %sposition]",
                       (n == 0) ? "":"part of ");
    return meTRUE ;
}
#endif

#if MEOPT_OSD
/*
 * frameSetupMenuLine.
 * Change the state of the window line.
 * Shuffle windows for allow the insertion of a menu line
 * 
 * Return meTRUE if the menu line has changed visibility state.
 */
int
frameSetupMenuLine (int newTTsrow)
{
    meWindow *wp;
    
    /* Make some / remove some space for the menu line */
    if (newTTsrow != frameCur->menuDepth)
    {
        frameCur->menuDepth = newTTsrow;
        
        /* REMOVE - existing menu line */
        if (newTTsrow == 0)
        {
            for (wp = frameCur->windowList; wp != NULL; wp = wp->next)
            {
                if (wp->frameRow == 1)
                {
                    wp->frameRow = 0;
                    wp->depth += 1;
                    meWindowFixTextSize(wp);
                }
            }
        }
        
        /* INSERT - new menu */
        else
        {
            /* Try to insert the menu line by squashing the 
             * existing buffers down. */
            for (wp = frameCur->windowList; wp != NULL; wp = wp->next)
            {
                if (wp->frameRow == 0)
                {
                    /* Failed. Reduce to the current window which will
                     * give us enough roo,. */
                    if (wp->depth <= 2)
                    {
                        windowDeleteOthers(0,0);
                        break;
                    }
                    else
                    {
                        wp->depth -= 1;
                        wp->frameRow = frameCur->menuDepth;
                        meWindowFixTextSize(wp);
                    }
                }
            }
        }
    }
    else
        return (meFALSE);
    
/*    sgarbf = meTRUE; */                 /* Garbage the screen */
    return (meTRUE);                      /* Changed status     */
}
#endif



