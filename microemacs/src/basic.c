/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * basic.c - Basic Movement Routines.
 *
 * Copyright (C) 1988-2024 JASSPA (www.jasspa.com)
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
 * Synopsis:    Basic Movement Routines.
 * Authors:     Unknown, Jon Green & Steven Phillips
 * Description:
 *     The routines in this file move the cursor around on the screen. They
 *     compute a new value for the cursor, then adjust ".". The display code
 *     always updates the cursor location, so only moves between lines, or
 *     functions that adjust the top line in the window and invalidate the
 *     framing, are hard.
 */

#define	__BASICC			/* Define program */ 

#include "emain.h"

/*
 * Move the cursor to the beginning of the current line. Trivial.
 */
int
windowGotoBol(int f, int n)
{
    frameCur->windowCur->dotOffset = 0;
    frameCur->windowCur->updateFlags |= WFMOVEC;
    return meTRUE;
}
/*
 * Move the cursor to the end of the current line. Trivial. No errors.
 */
int
windowGotoEol(int f, int n)
{
    frameCur->windowCur->dotOffset = meLineGetLength(frameCur->windowCur->dotLine);
    frameCur->windowCur->updateFlags |= WFMOVEC;
    return meTRUE;
}

int
meErrorEob(void)
{
    return mlwrite(MWABORT|MWCLEXEC,(meUByte *)"[end of buffer]");
}

int
meErrorBob(void)
{
    return mlwrite(MWABORT|MWCLEXEC,(meUByte *)"[top of buffer]");
}

int
meWindowBackwardChar(register meWindow *wp, register int n)
{
    register meLine   *lp;
    
    while(n--)
    {
        if(wp->dotOffset == 0)
        {
            if((lp=meLineGetPrev(wp->dotLine)) == wp->buffer->baseLine)
                return meFALSE;
            wp->dotLineNo--;
            wp->dotLine = lp;
            wp->dotOffset = meLineGetLength(lp);
            wp->updateFlags |= WFMOVEL;
        }
        else
        {
            wp->dotOffset--;
            wp->updateFlags |= WFMOVEC;
        }
    }
    return meTRUE;
}

int
meWindowForwardChar(register meWindow *wp, register int n)
{
    while(n--)
    {
        if(wp->dotOffset == meLineGetLength(wp->dotLine))
        {
            if(wp->dotLine == wp->buffer->baseLine)
                return meFALSE;
            wp->dotLineNo++;
            wp->dotLine = meLineGetNext(wp->dotLine);
            wp->dotOffset = 0;
            wp->updateFlags |= WFMOVEL;
        } 
        else
        {
            wp->dotOffset++;
            wp->updateFlags |= WFMOVEC;
        }
    }
    return meTRUE;
}

/*
 * Move the cursor backwards by "n" characters. If "n" is less than zero call
 * "windowForwardChar" to actually do the move. Otherwise compute the new cursor
 * location. Error if you try and move out of the buffer. Set the flag if the
 * line pointer for dot changes.
 */
int
windowBackwardChar(int f, register int n)
{
    if(n < 0)
        return windowForwardChar(f,-n);
    if(meWindowBackwardChar(frameCur->windowCur,n) <= 0)
        return meErrorBob();
    return meTRUE;
}

/*
 * Move the cursor forwwards by "n" characters. If "n" is less than zero call
 * "windowBackwardChar" to actually do the move. Otherwise compute the new cursor
 * location, and move ".". Error if you try and move off the end of the
 * buffer. Set the flag if the line pointer for dot changes.
 */
int
windowForwardChar(int f, register int n)
{
    if(n < 0)
        return windowBackwardChar(f,-n);
    if(meWindowForwardChar(frameCur->windowCur,n) <= 0)
        return meErrorEob();
    return meTRUE;
}


meUByte
meWindowGetChar(register meWindow *wp)
{
    register meUByte cc;
    
    if(wp->buffer->baseLine == wp->dotLine)
        cc = '\0';
    else if(wp->dotOffset >= wp->dotLine->length)
        cc = meCHAR_NL;
    else
        cc = wp->dotLine->text[wp->dotOffset];
    return cc;
}


/*
 * Move forward by full lines. If the number of lines to move is less than
 * zero, call the backward line function to actually do it. The last command
 * controls how the goal column is set. Bound to "C-N". No errors are
 * possible.
 */
int
windowForwardLine(int f, int n)
{
    register meWindow *cwp;
    register meLine *dlp;
    register long dln;
    
    if(n < 0)
        return windowBackwardLine(f,-n);
    cwp = frameCur->windowCur;

    /* if the last command was not a line move, reset the goal column */
    if((lastflag & meCFCPCN) == 0)
        curgoal = getcwcol(cwp);
    
    /* and move the point up */
    dln = cwp->dotLineNo;
    if(dln+n > cwp->buffer->lineCount)
    {
        if(dln != cwp->buffer->lineCount)
        {
            cwp->dotLine = cwp->buffer->baseLine;
            cwp->dotLineNo = cwp->buffer->lineCount;
            cwp->dotOffset = 0;
            cwp->updateFlags |= WFMOVEL;
        }
        return meErrorEob();
    }
    /* reseting the current position */
    cwp->dotLineNo = dln + n;
    dlp = cwp->dotLine;
    while(n--)
        dlp = meLineGetNext(dlp);
    cwp->dotLine = dlp;
    setcwcol(cwp,curgoal);
    cwp->updateFlags |= WFMOVEL;
    if((scrollFlag & 0x10) && (cwp->vertScroll <= (cwp->dotLineNo-cwp->textDepth)))
    {
        /* do the smooth scroll here */
        cwp->vertScroll = cwp->dotLineNo - cwp->textDepth + 1;
        cwp->updateFlags |= (WFREDRAW|WFSBOX);
    }
    /* flag this command as a line move */
    thisflag = meCFCPCN;
    return meTRUE;
}

/*
 * This function is like "windowForwardLine", but goes backwards. The scheme is exactly
 * the same. Check for arguments that are less than zero and call your
 * alternate. Figure out the new line and call "movedot" to perform the
 * motion. No errors are possible. Bound to "C-P".
 */
int
windowBackwardLine(int f, int n)
{
    register meWindow *cwp;
    register meLine *dlp;
    register long dln;

    if(n < 0)
        return windowForwardLine(f,-n);
    cwp = frameCur->windowCur;
    
    /* if the last command was not a line move,
       reset the goal column */
    if((lastflag&meCFCPCN) == 0)
        curgoal = getcwcol(cwp);

    /* and move the point up */
    dln = cwp->dotLineNo;
    if(n > dln)
    {
        if(dln)
        {
            dlp = meLineGetNext(cwp->buffer->baseLine);
            cwp->dotLine = dlp;
            cwp->dotLineNo = 0;
            setcwcol(cwp,curgoal);
            cwp->updateFlags |= WFMOVEL;
        }
        return meErrorBob();
    }
    /* reseting the current position */
    cwp->dotLineNo = dln - n;
    dlp = cwp->dotLine;
    while(n--)
        dlp = meLineGetPrev(dlp);
    cwp->dotLine = dlp;
    setcwcol(cwp,curgoal);
    cwp->updateFlags |= WFMOVEL;
    if((scrollFlag & 0x10) && (cwp->vertScroll > cwp->dotLineNo))
    {
        /* do the smooth scroll here */
        cwp->vertScroll = cwp->dotLineNo;
        cwp->updateFlags |= (WFREDRAW|WFSBOX);
    }
    /* flag this command as a line move */
    thisflag = meCFCPCN;
    return meTRUE;
}

int	
meWindowGotoLine(register meWindow *wp, meInt nlno)
{
    register meLine *lp;
    meInt nn;
    /* if a bogus argument...then leave */
    if(--nlno < 0)
        return meFALSE;
    
    /* always reset offset to the left hand edge */
    wp->dotOffset = 0;
    if(wp == frameCur->windowCur)
        curgoal = 0;
    
    if((nn=wp->dotLineNo-nlno) == 0)
        return meTRUE;
    wp->updateFlags |= WFMOVEL;
    wp->dotLineNo = nlno;
    if(nlno < nn)
    {
        /* first, we go to the start of the buffer */
        lp = meLineGetNext(wp->buffer->baseLine);
        if(!nlno)
        {
            wp->dotLine = lp;
            return meTRUE;
        }
    }
    else if((nlno=nlno - wp->buffer->lineCount) >= nn)
    {
        /* first, we go to the end of the buffer */
        lp = wp->buffer->baseLine;
        if(nlno >= 0)
        {
            wp->dotLine = lp;
            if(nlno == 0)
                return meTRUE;
            wp->dotLineNo = wp->buffer->lineCount;
            return meErrorEob();
        }
    }
    else
    {
        lp = wp->dotLine;
        nlno = 0-nn;
    }
    if(nlno < 0)
    {
        do
            lp = meLineGetPrev(lp);
        while(++nlno);
    }
    else
    {
        do
            lp = meLineGetNext(lp);
        while(--nlno);
    }
    wp->dotLine = lp;
    return meTRUE;
}

#if MEOPT_EXTENDED || MEOPT_NARROW
/* windowGotoAbsLine
 * 
 * Goto absolute line is similar to windowGotoLine except it considers narrowed out
 * text as well */
int	
meWindowGotoAbsLine(register meWindow *wp, meInt line)
{
    meBuffer *bp=wp->buffer;
    meLine *lp;
    int rr;
    
    if(line == 0)		/* if a bogus argument...then leave */
        return meFALSE;
#if MEOPT_NARROW
    if(bp->narrow != NULL)
        /* There are narrows - first unnarrow buffer */
        meBufferExpandNarrowAll(bp);
#endif
    if(line < 0)
    {
        rr = wp->dotLineNo;
#if MEOPT_EXTENDED
        if(wp->dotLine->flag & meLINE_MARKUP)
            rr = -1;
        else
        {
            lp = meLineGetNext(bp->baseLine);
            while(lp != wp->dotLine)
            {
                if(lp->flag & meLINE_MARKUP)
                    rr--;
                lp = meLineGetNext(lp);
            }
        }
#endif
#if MEOPT_NARROW
        if(bp->narrow != NULL)
            meBufferCollapseNarrowAll(bp);
#endif
        return rr;
    }
    /* now move to the required line */
    if(line > bp->lineCount)
    {
        wp->dotLine = bp->baseLine;
        wp->dotLineNo = bp->lineCount;
        rr = (line == bp->lineCount);
    }
    else
    {
        wp->dotLineNo = 0;
        lp = meLineGetNext(bp->baseLine);
        while(((lp->flag & meLINE_MARKUP) || (--line > 0)) &&
              (lp != bp->baseLine))
        {
            lp = meLineGetNext(lp);
            wp->dotLineNo++;
        }
        wp->dotLine = lp;
        rr = (line == 0);
    }
    /* set column to left hand edge and reset the goal column */
    wp->dotOffset = 0;
    wp->updateFlags |= WFMOVEL;
    if(wp == frameCur->windowCur)
        curgoal = 0;
    
#if MEOPT_NARROW
    if(bp->narrow != NULL)
    {
        /* To ensure we get back to the right line (could be
         * currently narrowed out), drop an alpha mark.
         */
        if(rr > 0)
            rr = meAnchorSet(bp,meANCHOR_ABS_LINE,wp->dotLine,wp->dotLineNo,0,1);
        meBufferCollapseNarrowAll(bp);
        if(rr > 0) 
        {
            if((rr = meAnchorGet(bp,meANCHOR_ABS_LINE)) > 0)
            {
                /* do the buisness */
                wp->dotLine = bp->dotLine;
                wp->dotOffset = bp->dotOffset;
                wp->dotLineNo = bp->dotLineNo;
                wp->updateFlags |= WFMOVEL;
            }
            meAnchorDelete(bp,meANCHOR_ABS_LINE);
        }
    }
#endif
    return rr;
}
#endif

/* move to a particular line. argument (n) must be a positive integer for this
 * to actually do anything
 */
int	
windowGotoLine(int f, int n)
{
    register int status;	/* status return */
    meUByte arg[meSBUF_SIZE_MAX];	        /* buffer to hold argument */
    meInt nlno;
    
    /* get an argument if one doesnt exist */
    if((f == meFALSE) || (n == 0))
    {
        if((status = meGetString((meUByte *)"Goto line", 0, 0, arg, meSBUF_SIZE_MAX)) <= 0) 
            return status;
        
        /*---	Skip white space */
        
        for(n = 0; ((arg[n] == '\t') || (arg[n] == ' ')) && (arg[n] != '\0'); n++)
            /* NULL */;
        
        /*---	Record if it is a displacement */
        
        status = (arg[n]=='+' || arg[n]=='-');	/* Displacement ?? */
        nlno = meAtoi(arg);			/* Get value */
        
        /* SWP moved these two lines into this bracket as if arguement is given the default must be
         * absolution (previously undefined as status was left uninitialised.
         */
        if(status)			/* Displacement ?? */
        {
#if MEOPT_NARROW
            if(f != meFALSE)
                nlno += meWindowGotoAbsLine(frameCur->windowCur,-1);
            else
#endif
                nlno += frameCur->windowCur->dotLineNo;
            nlno++;
        }
        if(nlno <= 0)
        {
            windowGotoBob(meFALSE,1);
            return meErrorBob();
        }
#if MEOPT_NARROW
        if(f != meFALSE)
            return meWindowGotoAbsLine(frameCur->windowCur,nlno);
#endif
    }
    else
        nlno = n;
    
    return meWindowGotoLine(frameCur->windowCur,nlno);
}

/*
 * Goto the beginning of the buffer. Massive adjustment of dot. This is
 * considered to be hard motion; it really isn't if the original value of dot
 * is the same as the new value of dot. Normally bound to "M-<".
 */
int
windowGotoBob(int f, int n)
{
    register meWindow *cwp=frameCur->windowCur;
    cwp->dotLine = meLineGetNext(cwp->buffer->baseLine);
    cwp->dotOffset = 0;
    cwp->dotLineNo = 0;
    cwp->updateFlags |= WFMOVEL;
    return meTRUE;
}

/*
 * Move to the end of the buffer. Dot is always put at the end of the file
 * (ZJ). The standard screen code does most of the hard parts of update.
 * Bound to "M->".
 */
int
windowGotoEob(int f, int n)
{
    register meWindow *cwp=frameCur->windowCur;
    cwp->dotLine = cwp->buffer->baseLine;
    cwp->dotOffset = 0;
    cwp->dotLineNo = cwp->buffer->lineCount;
    cwp->updateFlags |= WFMOVEL;
    return meTRUE;
}

#if MEOPT_WORDPRO

/* go back to the beginning of the current paragraph here we look for a
 * <NL><NL> or <NL><TAB> or <NL><SPACE> combination to delimit the beginning
 * of a paragraph
 */
int
windowBackwardParagraph(int f, int n)
{
    register meWindow *cwp;
    register int suc;   	/* success of last windowBackwardChar */
    int	         i;		/* Local counter. */
    int	         line_len;	/* Length of the line */
    char	 c;		/* Temporary character */
    
    if(n < 0)	/* the other way...*/
        return windowForwardParagraph(f,-n);
    
    cwp = frameCur->windowCur;
    while(n-- > 0)
    {	/* for each one asked for */
        
        /* first scan back until we are in a word */
        do
            if(meWindowBackwardChar(cwp, 1) <= 0)
                return meErrorBob();
        while(!meWindowInPWord(cwp));
        
        cwp->dotOffset = 0;	/* and go to the B-O-Line */
        
        /* and scan back until we hit a <NL><NL> or <NL><TAB>
           or a <NL><SPACE>					*/
        
        do {
            suc = meTRUE;	/* Reset sucess flag */
            if(meLineGetPrev(cwp->dotLine) == cwp->buffer->baseLine)	/* SOF  ?? */
                break;		/* Exit */
            if((line_len = meLineGetLength(cwp->dotLine)) == 0)
                break;		/* Exit */
            for(i = 0; i < line_len ; i++)
            {
                c = meLineGetChar(cwp->dotLine,i);	/* Get character */
                if((c != meCHAR_TAB) && (c != ' '))	/* Character on line ?? */
                {
                    /*---	Yes - prvious line. Exit loop */
                    
                    cwp->dotLine = meLineGetPrev(cwp->dotLine);
                    cwp->dotLineNo--;
                    suc = meFALSE;
                    break;
                }
            }	/* End of 'for' */
        }
        while(suc <= 0);
        
        /* and then leave the cursor at the start of the line. */
        cwp->dotOffset = 0;        
    }
    cwp->updateFlags |= WFMOVEL;	/* force screen update */
    return meTRUE;
}

/* go forword to the end of the current paragraph here we look for a <NL><NL>
 * or <NL><TAB> or <NL><SPACE> combination to delimit the beginning of a
 * paragraph
 */
int
windowForwardParagraph(int f, int n)	
{
    register meWindow *cwp;
    register int suc=meTRUE;   	/* success of last windowBackwardChar */
    int	         i;		/* Local counter. */
    int	         line_len;	/* Length of the line */
    char	 c;		/* Temporary character */
    
    if(n < 0)	/* the other way...*/
        return windowBackwardParagraph(f,-n);

    cwp = frameCur->windowCur;
    cwp->updateFlags |= WFMOVEL;	/* force screen update */
    while(n-- > 0) 
    {	/* for each one asked for */

        /*---	First scan forward until we are in a word */
        while(!meWindowInPWord(cwp))
            if(meWindowForwardChar(cwp,1) <= 0)
                return meErrorEob();
        
        cwp->dotOffset = 0;	/* and go to the B-O-Line */
        if(suc)		/* of next line if not at EOF */
        {
            cwp->dotLine = meLineGetNext(cwp->dotLine);
            cwp->dotLineNo++;
        }
        /* and scan forword until we hit a <NL><NL> or <NL><TAB>
           or a <NL><SPACE>					*/
        do {
            suc = meTRUE;	/* Reset sucess flag */
            if(cwp->dotLine == cwp->buffer->baseLine)	/* EOF  ?? */
                break;		/* Exit */
            if((line_len = meLineGetLength(cwp->dotLine)) == 0)
                break;		/* Exit */
            for(i = 0; i < line_len ; i++)
            {
                c = meLineGetChar(cwp->dotLine,i);	/* Get character */
                if((c != meCHAR_TAB) && (c != ' '))	/* Character on line ?? */
                {
                    /*---	Goto the next line. Break ot of loop. */
                    cwp->dotLine = meLineGetNext(cwp->dotLine);
                    cwp->dotLineNo++;
                    suc = meFALSE;
                    break;
                }
            }	/* End of 'for' */
        } while(suc <= 0);
        
        /* Leave the cursor at the start of the line. */
        cwp->dotOffset = 0;
    }
    return meTRUE;
}
#endif

/*
 * Set the mark in the current window to the value of "." in the window. No
 * errors are possible. Bound to "M-.".
 */
int
windowSetMark(int f, int n)
{
    register meWindow *cwp=frameCur->windowCur;
    cwp->markLine = cwp->dotLine;
    cwp->markOffset = cwp->dotOffset;
    cwp->markLineNo = cwp->dotLineNo;
    return mlwrite(MWCLEXEC,(meUByte *)"[Mark set]");
}

/*
 * Swap the values of "." and "mark" in the current window. This is pretty
 * easy, bacause all of the hard work gets done by the standard routine
 * that moves the mark about. The only possible error is "no mark". Bound to
 * "C-X C-X".
 */
int
windowSwapDotAndMark(int f, int n)
{
    meWindow *cwp;
    meLine   *odotp;
    meUShort  odoto;
    meInt     lineno;

    if((cwp=frameCur->windowCur)->markLine == NULL)
        return noMarkSet();
    odotp = cwp->dotLine;
    odoto = cwp->dotOffset;
    lineno = cwp->dotLineNo;
    cwp->dotLine = cwp->markLine;
    cwp->dotOffset = cwp->markOffset;
    cwp->dotLineNo = cwp->markLineNo;
    cwp->markLine = odotp;
    cwp->markOffset = odoto;
    cwp->markLineNo = lineno;
    cwp->updateFlags |= WFMOVEL;
    return meTRUE;
}

#if MEOPT_IPIPES || MEOPT_SOCKET

static meLine  *orgLpp, *orgLnp;
static meUShort orgLo;
static meInt  orgLn;

void
meBufferStoreLocation(meLine *lp, meUShort lo, meInt ln)
{
    if((orgLpp = meLineGetPrev(lp)) != lp)
    {
        orgLnp = meLineGetNext(lp);
        orgLo = lo;
        orgLn = ln;
    }
    else
        orgLpp = NULL;
}

void
meBufferUpdateLocation(meBuffer *bp, meUInt noLines, meUShort newOff)
{
    meWindow *wp;
    
    meFrameLoopBegin();
    wp = loopFrame->windowList;
    while(wp != NULL)
    {
        if(wp->buffer == bp)
        {
            if(orgLpp == NULL)
            {
                wp->dotLine = bp->dotLine;
                wp->dotOffset = bp->dotOffset;
                wp->dotLineNo = bp->dotLineNo;
                if(wp->markLine != NULL)
                {
                    wp->markLine = meLineGetNext(bp->baseLine);
                    wp->markOffset = 0;
                    wp->markLineNo = 0;
                }
            }
            else
            {
                if(meModeTest(wp->buffer->mode,MDLOCK) && (wp->buffer->intFlag & BIFLOCK))
                {
                    wp->dotLine = bp->dotLine;
                    wp->dotOffset = bp->dotOffset;
                    wp->dotLineNo = bp->dotLineNo;
                }
                else if(wp->dotLineNo >= orgLn)
                {
                    if(wp->dotLineNo == orgLn)
                    {
                        if(wp->dotOffset < orgLo)
                            wp->dotLine = meLineGetNext(orgLpp);
                        else
                        {
                            wp->dotLine = meLineGetPrev(orgLnp);
                            wp->dotOffset = wp->dotOffset-orgLo+newOff;
                            wp->dotLineNo += noLines;
                        }
                        if(wp->dotOffset > meLineGetLength(wp->dotLine))
                            wp->dotOffset = meLineGetLength(wp->dotLine);
                    }
                    else
                        wp->dotLineNo += noLines;
                }
                if(wp->markLineNo == orgLn)
                {
                    if(wp->markOffset < orgLo)
                        wp->markLine = meLineGetNext(orgLpp);
                    else
                    {
                        wp->markLine = meLineGetPrev(orgLnp);
                        wp->markOffset = wp->markOffset-orgLo+newOff;
                        wp->markLineNo += noLines;
                    }
                    if(wp->markOffset > meLineGetLength(wp->markLine))
                        wp->markOffset = meLineGetLength(wp->markLine);
                }
                else if(wp->markLineNo > orgLn)
                    wp->markLineNo += noLines;
            }
            wp->updateFlags |= WFMOVEL|WFMAIN|WFSBOX;
        }
        wp = wp->next;
    }
    meFrameLoopEnd();
}

#endif
