/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * basic.c - Basic Movement Routines.
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
gotobol(int f, int n)
{
    frameCur->windowCur->dotOffset  = 0;
    frameCur->windowCur->flag |= WFMOVEC ;
    return (meTRUE);
}
/*
 * Move the cursor to the end of the current line. Trivial. No errors.
 */
int
gotoeol(int f, int n)
{
    frameCur->windowCur->dotOffset  = meLineGetLength(frameCur->windowCur->dotLine);
    frameCur->windowCur->flag |= WFMOVEC ;
    return (meTRUE);
}

int
eobError(void)
{
    return mlwrite(MWABORT|MWCLEXEC,(meUByte *)"[end of buffer]");
}

int
bobError(void)
{
    return mlwrite(MWABORT|MWCLEXEC,(meUByte *)"[top of buffer]");
}

int
WbackChar(register meWindow *wp, register int n)
{
    register meLine   *lp;
    
    while (n--)
    {
        if(wp->dotOffset == 0)
        {
            if ((lp=meLineGetPrev(wp->dotLine)) == wp->buffer->baseLine)
                return meFALSE ;
            wp->dotLineNo-- ;
            wp->dotLine  = lp;
            wp->dotOffset  = meLineGetLength(lp);
            wp->flag |= WFMOVEL ;
        }
        else
        {
            wp->dotOffset-- ;
            wp->flag |= WFMOVEC ;
        }
    }
    return meTRUE ;
}

int
WforwChar(register meWindow *wp, register int n)
{
    while (n--)
    {
        if(wp->dotOffset == meLineGetLength(wp->dotLine))
        {
            if (wp->dotLine == wp->buffer->baseLine)
                return (meFALSE);
            wp->dotLineNo++ ;
            wp->dotLine  = meLineGetNext(wp->dotLine);
            wp->dotOffset  = 0;
            wp->flag |= WFMOVEL ;
        } 
        else
        {
            wp->dotOffset++;
            wp->flag |= WFMOVEC ;
        }
    }
    return meTRUE ;
}

/*
 * Move the cursor backwards by "n" characters. If "n" is less than zero call
 * "forwChar" to actually do the move. Otherwise compute the new cursor
 * location. Error if you try and move out of the buffer. Set the flag if the
 * line pointer for dot changes.
 */
int
backChar(int f, register int n)
{
    if (n < 0)
        return (forwChar(f, -n));
    if(WbackChar(frameCur->windowCur,n) != meTRUE)
        return bobError() ;
    return meTRUE ;
}

/*
 * Move the cursor forwwards by "n" characters. If "n" is less than zero call
 * "backChar" to actually do the move. Otherwise compute the new cursor
 * location, and move ".". Error if you try and move off the end of the
 * buffer. Set the flag if the line pointer for dot changes.
 */
int
forwChar(int f, register int n)
{
    if (n < 0)
        return (backChar(f, -n));
    if(WforwChar(frameCur->windowCur,n) != meTRUE)
        return eobError() ;
    return meTRUE ;
}


meUByte
getCurChar(meWindow *wp)
{
    register meUByte cc ;
    
    if (wp->buffer->baseLine == wp->dotLine)
        cc = '\0';
    else if (wp->dotOffset >= wp->dotLine->length)
        cc = meNLCHAR ;
    else
        cc = wp->dotLine->text[wp->dotOffset];
    return cc ;
}


/*
 * Move forward by full lines. If the number of lines to move is less than
 * zero, call the backward line function to actually do it. The last command
 * controls how the goal column is set. Bound to "C-N". No errors are
 * possible.
 */
int
forwLine(int f, int n)
{
    register meLine *dlp;
    register long  dln;
    
    if (n < 0)
        return (backLine(f, -n));

    /* if the last command was not a line move,
       reset the goal column */
    if ((lastflag&meCFCPCN) == 0)
        curgoal = getcwcol();
    
    /* and move the point up */
    dln = frameCur->windowCur->dotLineNo ;
    if(dln+n > frameCur->bufferCur->lineCount)
    {
        if(dln != frameCur->bufferCur->lineCount)
        {
            frameCur->windowCur->dotLine = frameCur->bufferCur->baseLine ;
            frameCur->windowCur->dotLineNo= frameCur->bufferCur->lineCount ;
            frameCur->windowCur->dotOffset = 0 ;
            frameCur->windowCur->flag |= WFMOVEL;
        }
        return eobError() ;
    }
    /* reseting the current position */
    frameCur->windowCur->dotLineNo = dln + n ;
    dlp = frameCur->windowCur->dotLine ;
    while(n--)
        dlp = meLineGetNext(dlp);
    frameCur->windowCur->dotLine = dlp ;
    setcwcol(curgoal) ;
    frameCur->windowCur->flag |= WFMOVEL ;
    if((scrollFlag & 0x10) && (frameCur->windowCur->vertScroll <= (frameCur->windowCur->dotLineNo-frameCur->windowCur->textDepth)))
    {
        /* do the smooth scroll here */
        frameCur->windowCur->vertScroll = frameCur->windowCur->dotLineNo - frameCur->windowCur->textDepth + 1 ;
        frameCur->windowCur->flag |= (WFREDRAW|WFSBOX) ;
    }
    /* flag this command as a line move */
    thisflag = meCFCPCN;
    return meTRUE ;
}

/*
 * This function is like "forwLine", but goes backwards. The scheme is exactly
 * the same. Check for arguments that are less than zero and call your
 * alternate. Figure out the new line and call "movedot" to perform the
 * motion. No errors are possible. Bound to "C-P".
 */
int
backLine(int f, int n)
{
    register meLine *dlp;
    register long  dln;

    if (n < 0)
        return (forwLine(f, -n));
    
    /* if the last command was not a line move,
       reset the goal column */
    if ((lastflag&meCFCPCN) == 0)
        curgoal = getcwcol();

    /* and move the point up */
    dln = frameCur->windowCur->dotLineNo;
    if(n > dln)
    {
        if(dln)
        {
            dlp = meLineGetNext(frameCur->bufferCur->baseLine) ;
            frameCur->windowCur->dotLine = dlp ;
            frameCur->windowCur->dotLineNo= 0 ;
            setcwcol(curgoal) ;
            frameCur->windowCur->flag |= WFMOVEL ;
        }
        return bobError() ;
    }
    /* reseting the current position */
    frameCur->windowCur->dotLineNo = dln - n ;
    dlp = frameCur->windowCur->dotLine;
    while(n--)
        dlp = meLineGetPrev(dlp);
    frameCur->windowCur->dotLine = dlp ;
    setcwcol(curgoal) ;
    frameCur->windowCur->flag |= WFMOVEL ;
    if((scrollFlag & 0x10) && (frameCur->windowCur->vertScroll > frameCur->windowCur->dotLineNo))
    {
        /* do the smooth scroll here */
        frameCur->windowCur->vertScroll = frameCur->windowCur->dotLineNo ;
        frameCur->windowCur->flag |= (WFREDRAW|WFSBOX) ;
    }
    /* flag this command as a line move */
    thisflag = meCFCPCN;
    return meTRUE ;
}

/* move to a particular line. argument (n) must be a positive integer for this
 * to actually do anything
 */
int	
gotoLine(int f, int n)
{
    register int status;	/* status return */
    meUByte arg[meSBUF_SIZE_MAX];	        /* buffer to hold argument */
    meInt nlno ;
    
    /* get an argument if one doesnt exist */
    if ((f == meFALSE) || (n == 0))
    {
        if ((status = meGetString((meUByte *)"Goto line", 0, 0, arg, meSBUF_SIZE_MAX)) != meTRUE) 
            return(status);
        
        /*---	Skip white space */
        
        for (n = 0; (arg[n] == '\t' || arg[n] == ' ') && arg[n] != '\0'; n++)
            /* NULL */ ;
        
        /*---	Record if it is a displacement */
        
        status = (arg[n]=='+' || arg[n]=='-');	/* Displacement ?? */
        nlno = meAtoi(arg);				/* Get value */
        
        /* SWP moved these two lines into this bracket as if arguement is given
         * the default must be absolution (previously undefined as status was
         * left uninitialised.
         */
        if (status)			/* Displacement ?? */
        {
#if MEOPT_NARROW
            if (f != meFALSE)
                nlno += gotoAbsLine(-1) ;
            else
#endif
                nlno += frameCur->windowCur->dotLineNo ;
        }
        if(nlno <= 0)
        {
            gotobob(meFALSE,1) ;
            return bobError() ;
        }
#if MEOPT_NARROW
        if (f != meFALSE)
            return gotoAbsLine(nlno) ;
#endif
    }
    else
        nlno = n ;
    
    /*--- This is an absolute jump. Go to zero & move */

    if (nlno < 1)		/* if a bogus argument...then leave */
        return(meFALSE);
    
    /* always reset offset to the left hand edge */
    curgoal = 0 ;
    frameCur->windowCur->dotOffset = 0 ;
    if(nlno < (frameCur->windowCur->dotLineNo-nlno))
    {
        /* first, we go to the start of the buffer */
        frameCur->windowCur->dotLine  = meLineGetNext(frameCur->bufferCur->baseLine);
        frameCur->windowCur->dotLineNo = 0;
        frameCur->windowCur->flag |= WFMOVEL ;
    }
    else if((frameCur->bufferCur->lineCount-nlno) < (nlno-frameCur->windowCur->dotLineNo))
    {
        /* first, we go to the end of the buffer */
        frameCur->windowCur->dotLine  = frameCur->bufferCur->baseLine ;
        frameCur->windowCur->dotLineNo = frameCur->bufferCur->lineCount ;
        frameCur->windowCur->flag |= WFMOVEL ;
    }
    frameCur->windowCur->dotOffset  = 0 ;
    nlno -= frameCur->windowCur->dotLineNo ;
    /* force the $scroll variable to no smooth so if the line is off the screen
     * the reframe will center the line */
    {
        meUByte sscrollFlag ; 
        
        sscrollFlag = scrollFlag ;
        scrollFlag = 0 ;
        n =  forwLine(f, nlno-1) ;
        scrollFlag = sscrollFlag ;
    }
    return n ;
}

#if MEOPT_NARROW
/* gotoAbsLine
 * 
 * Goto absolute line is similar to gotoLine except it considers narrowed out
 * text as well */
int	
gotoAbsLine(meInt line)
{
    int rr ;
    
    if(frameCur->bufferCur->narrow != NULL)
        /* There are narrows - first unnarrow buffer */
        unnarrowBuffer(frameCur->bufferCur) ;
    if(line < 0)
    {
        rr = frameCur->windowCur->dotLineNo ;
        if(frameCur->bufferCur->narrow != NULL)
            redoNarrowInfo(frameCur->bufferCur) ;
        return rr ;
    }
    /* now move to the required line */
    rr = gotoLine(meTRUE,line) ;
    
    if(frameCur->bufferCur->narrow != NULL)
    {
        /* To ensure we get back to the right line (could be
         * currently narrowed out), drop an alpha mark.
         */
        if(rr == meTRUE)
            rr = alphaMarkSet(frameCur->bufferCur,meAM_ABSLINE,frameCur->windowCur->dotLine,0,1) ;
        redoNarrowInfo(frameCur->bufferCur) ;
        if((rr == meTRUE) &&
           ((rr = alphaMarkGet(frameCur->bufferCur,meAM_ABSLINE)) == meTRUE))
        {
            /* do the buisness */
            frameCur->windowCur->dotLine = frameCur->bufferCur->dotLine ;
            frameCur->windowCur->dotOffset = frameCur->bufferCur->dotOffset ;
            frameCur->windowCur->dotLineNo = frameCur->bufferCur->dotLineNo ;
            frameCur->windowCur->flag |= WFMOVEL ;
        }
    }
    return rr ;
}
#endif


/*
 * Goto the beginning of the buffer. Massive adjustment of dot. This is
 * considered to be hard motion; it really isn't if the original value of dot
 * is the same as the new value of dot. Normally bound to "M-<".
 */
int
gotobob(int f, int n)
{
    frameCur->windowCur->dotLine  = meLineGetNext(frameCur->bufferCur->baseLine);
    frameCur->windowCur->dotOffset  = 0;
    frameCur->windowCur->dotLineNo = 0;
    frameCur->windowCur->flag |= WFMOVEL ;
    return (meTRUE);
}

/*
 * Move to the end of the buffer. Dot is always put at the end of the file
 * (ZJ). The standard screen code does most of the hard parts of update.
 * Bound to "M->".
 */
int
gotoeob(int f, int n)
{
    frameCur->windowCur->dotLine  = frameCur->bufferCur->baseLine;
    frameCur->windowCur->dotOffset  = 0;
    frameCur->windowCur->dotLineNo = frameCur->bufferCur->lineCount;
    frameCur->windowCur->flag |= WFMOVEL ;
    return (meTRUE);
}

#if	MEOPT_WORDPRO

/* go back to the beginning of the current paragraph here we look for a
 * <NL><NL> or <NL><TAB> or <NL><SPACE> combination to delimit the beginning
 * of a paragraph
 */
int
backPara(int f, int n)
{
    register int suc;   	/* success of last backChar */
    int	         i;		/* Local counter. */
    int	         line_len;	/* Length of the line */
    char	 c;		/* Temporary character */
    
    if (n < 0)	/* the other way...*/
        return(forwPara(f, -n));
    
    while (n-- > 0)
    {	/* for each one asked for */
        
        /* first scan back until we are in a word */
        do
            if(WbackChar(frameCur->windowCur, 1) != meTRUE)
                return bobError() ;
        while(!inPWord()) ;
        
        frameCur->windowCur->dotOffset = 0;	/* and go to the B-O-Line */
        
        /* and scan back until we hit a <NL><NL> or <NL><TAB>
           or a <NL><SPACE>					*/
        
        do {
            suc = meTRUE;	/* Reset sucess flag */
            if (meLineGetPrev(frameCur->windowCur->dotLine) == frameCur->bufferCur->baseLine)	/* SOF  ?? */
                break;		/* Exit */
            if ((line_len = meLineGetLength (frameCur->windowCur->dotLine)) == 0)
                break;		/* Exit */
            for (i = 0; i < line_len ; i++)
            {
                c = meLineGetChar (frameCur->windowCur->dotLine, i);	/* Get character */
                if (c != TAB && c != ' ')	/* Character on line ?? */
                {
                    /*---	Yes - prvious line. Exit loop */
                    
                    frameCur->windowCur->dotLine = meLineGetPrev(frameCur->windowCur->dotLine);
                    frameCur->windowCur->dotLineNo-- ;
                    suc = meFALSE;
                    break;
                }
            }	/* End of 'for' */
        }
        while (suc != meTRUE);
        
        /* and then forward until we are in a word */
        while (suc && !inPWord())
            suc = WforwChar(frameCur->windowCur, 1);
    }
    frameCur->windowCur->flag |= WFMOVEL ;	/* force screen update */
    return meTRUE ;
}

/* go forword to the end of the current paragraph here we look for a <NL><NL>
 * or <NL><TAB> or <NL><SPACE> combination to delimit the beginning of a
 * paragraph
 */
int
forwPara(int f, int n)	
{
    register int suc=meTRUE;   	/* success of last backChar */
    int	         i;		/* Local counter. */
    int	         line_len;	/* Length of the line */
    char	 c;		/* Temporary character */
    
    if (n < 0)	/* the other way...*/
        return (backPara(f, -n));

    frameCur->windowCur->flag |= WFMOVEL ;	/* force screen update */
    while (n-- > 0) 
    {	/* for each one asked for */

        /*---	First scan forward until we are in a word */
        while(!inPWord())
            if(WforwChar(frameCur->windowCur, 1) != meTRUE)
                return eobError() ;
        
        frameCur->windowCur->dotOffset = 0;	/* and go to the B-O-Line */
        if (suc)		/* of next line if not at EOF */
        {
            frameCur->windowCur->dotLine = meLineGetNext(frameCur->windowCur->dotLine);
            frameCur->windowCur->dotLineNo++ ;
        }
        /* and scan forword until we hit a <NL><NL> or <NL><TAB>
           or a <NL><SPACE>					*/
        do {
            suc = meTRUE;	/* Reset sucess flag */
            if (frameCur->windowCur->dotLine == frameCur->bufferCur->baseLine)	/* EOF  ?? */
                break;		/* Exit */
            if ((line_len = meLineGetLength (frameCur->windowCur->dotLine)) == 0)
                break;		/* Exit */
            for (i = 0; i < line_len ; i++)
            {
                c = meLineGetChar (frameCur->windowCur->dotLine, i);	/* Get character */
                if (c != TAB && c != ' ')	/* Character on line ?? */
                {
                    /*---	Goto the next line. Break ot of loop. */
                    frameCur->windowCur->dotLine = meLineGetNext(frameCur->windowCur->dotLine);
                    frameCur->windowCur->dotLineNo++ ;
                    suc = meFALSE;
                    break;
                }
            }	/* End of 'for' */
        } while (suc != meTRUE);
		
	/*---	Then backward until we are in a word */

        do
            suc = WbackChar(frameCur->windowCur, 1);
        while (suc && !inPWord());
        frameCur->windowCur->dotOffset = meLineGetLength(frameCur->windowCur->dotLine);	/* and to the EOL */
    }
    return(meTRUE);
}
#endif

/*
 * Set the mark in the current window to the value of "." in the window. No
 * errors are possible. Bound to "M-.".
 */
int
setMark(int f, int n)
{
    frameCur->windowCur->markLine = frameCur->windowCur->dotLine;
    frameCur->windowCur->markOffset = frameCur->windowCur->dotOffset;
    frameCur->windowCur->markLineNo = frameCur->windowCur->dotLineNo;
    return mlwrite(MWCLEXEC,(meUByte *)"[Mark set]");
}

/*
 * Swap the values of "." and "mark" in the current window. This is pretty
 * easy, bacause all of the hard work gets done by the standard routine
 * that moves the mark about. The only possible error is "no mark". Bound to
 * "C-X C-X".
 */
int
swapmark(int f, int n)
{
    register meLine  *odotp ;
    register int    odoto ;
    register long   lineno ;

    if (frameCur->windowCur->markLine == NULL)
        return noMarkSet() ;
    odotp = frameCur->windowCur->dotLine;
    odoto = frameCur->windowCur->dotOffset;
    lineno = frameCur->windowCur->dotLineNo ;
    frameCur->windowCur->dotLine  = frameCur->windowCur->markLine;
    frameCur->windowCur->dotOffset  = frameCur->windowCur->markOffset;
    frameCur->windowCur->dotLineNo = frameCur->windowCur->markLineNo;
    frameCur->windowCur->markLine = odotp;
    frameCur->windowCur->markOffset = odoto;
    frameCur->windowCur->markLineNo = lineno;
    frameCur->windowCur->flag |= WFMOVEL ;
    return (meTRUE);
}

static meLine  *orgLpp, *orgLnp ;
static meUShort orgLo ;
static meInt  orgLn ;

void
bufferPosStore(meLine *lp, meUShort lo, meInt ln)
{
    orgLpp = meLineGetPrev(lp) ;
    orgLnp = meLineGetNext(lp) ;
    orgLo = lo ;
    orgLn=ln ;
}

void
bufferPosUpdate(meBuffer *bp, meUInt noLines, meUShort newOff)
{
    meWindow *wp ;
    
    meFrameLoopBegin() ;
    wp = loopFrame->windowList;
    while(wp != NULL)
    {
        if(wp->buffer == bp)
        {
            if(meModeTest(wp->buffer->mode,MDLOCK) &&
               (wp->buffer->intFlag & BIFLOCK))
            {
                wp->dotLine= bp->dotLine ;
                wp->dotOffset= bp->dotOffset ;
                wp->dotLineNo = bp->dotLineNo ;
            }
            else if(wp->dotLineNo >= orgLn)
            {
                if(wp->dotLineNo == orgLn)
                {
                    if(wp->dotOffset < orgLo)
                        wp->dotLine = meLineGetNext(orgLpp) ;
                    else
                    {
                        wp->dotLine = meLineGetPrev(orgLnp) ;
                        wp->dotOffset = wp->dotOffset-orgLo+newOff ;
                        wp->dotLineNo += noLines ;
                    }
                    if(wp->dotOffset > meLineGetLength(wp->dotLine))
                       wp->dotOffset = meLineGetLength(wp->dotLine) ;
                }
                else
                    wp->dotLineNo += noLines ;
            }
            wp->flag |= WFMOVEL|WFMAIN|WFSBOX ;
            if(wp->markLineNo == orgLn)
            {
                if(wp->markOffset < orgLo)
                    wp->markLine = meLineGetNext(orgLpp) ;
                else
                {
                    wp->markLine = meLineGetPrev(orgLnp) ;
                    wp->markOffset = wp->markOffset-orgLo+newOff ;
                    wp->markLineNo += noLines ;
                }
                if(wp->markOffset > meLineGetLength(wp->markLine))
                    wp->markOffset = meLineGetLength(wp->markLine) ;
            }
            else if(wp->markLineNo > orgLn)
                wp->markLineNo += noLines ;
        }
        wp = wp->next;
    }
    meFrameLoopEnd() ;
}


