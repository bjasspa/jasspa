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
    curwp->w_doto  = 0;
    curwp->w_flag |= WFMOVEC ;
    return (TRUE);
}
/*
 * Move the cursor to the end of the current line. Trivial. No errors.
 */
int
gotoeol(int f, int n)
{
    curwp->w_doto  = llength(curwp->w_dotp);
    curwp->w_flag |= WFMOVEC ;
    return (TRUE);
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
WbackChar(register WINDOW *wp, register int n)
{
    register LINE   *lp;
    
    while (n--)
    {
        if(wp->w_doto == 0)
        {
            if ((lp=lback(wp->w_dotp)) == wp->w_bufp->b_linep)
                return FALSE ;
            wp->line_no-- ;
            wp->w_dotp  = lp;
            wp->w_doto  = llength(lp);
            wp->w_flag |= WFMOVEL ;
        }
        else
        {
            wp->w_doto-- ;
            wp->w_flag |= WFMOVEC ;
        }
    }
    return TRUE ;
}

int
WforwChar(register WINDOW *wp, register int n)
{
    while (n--)
    {
        if(wp->w_doto == llength(wp->w_dotp))
        {
            if (wp->w_dotp == wp->w_bufp->b_linep)
                return (FALSE);
            wp->line_no++ ;
            wp->w_dotp  = lforw(wp->w_dotp);
            wp->w_doto  = 0;
            wp->w_flag |= WFMOVEL ;
        } 
        else
        {
            wp->w_doto++;
            wp->w_flag |= WFMOVEC ;
        }
    }
    return TRUE ;
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
    if(WbackChar(curwp,n) != TRUE)
        return bobError() ;
    return TRUE ;
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
    if(WforwChar(curwp,n) != TRUE)
        return eobError() ;
    return TRUE ;
}


meUByte
getCurChar(WINDOW *wp)
{
    register meUByte cc ;
    
    if (wp->w_bufp->b_linep == wp->w_dotp)
        cc = '\0';
    else if (wp->w_doto >= wp->w_dotp->l_used)
        cc = meNLCHAR ;
    else
        cc = wp->w_dotp->l_text[wp->w_doto];
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
    register LINE *dlp;
    register long  dln;
    
    if (n < 0)
        return (backLine(f, -n));

    /* if the last command was not a line move,
       reset the goal column */
    if ((lastflag&CFCPCN) == 0)
        curgoal = getcwcol();
    
    /* and move the point up */
    dln = curwp->line_no ;
    if(dln+n > curbp->elineno)
    {
        if(dln != curbp->elineno)
        {
            curwp->w_dotp = curbp->b_linep ;
            curwp->line_no= curbp->elineno ;
            curwp->w_doto = 0 ;
            curwp->w_flag |= WFMOVEL;
        }
        return eobError() ;
    }
    /* reseting the current position */
    curwp->line_no = dln + n ;
    dlp = curwp->w_dotp ;
    while(n--)
        dlp = lforw(dlp);
    curwp->w_dotp = dlp ;
    setcwcol(curgoal) ;
    curwp->w_flag |= WFMOVEL ;
    if((scrollFlag & 0x10) && (curwp->topLineNo <= (curwp->line_no-curwp->numTxtRows)))
    {
        /* do the smooth scroll here */
        curwp->topLineNo = curwp->line_no - curwp->numTxtRows + 1 ;
        curwp->w_flag |= (WFREDRAW|WFSBOX) ;
    }
    /* flag this command as a line move */
    thisflag = CFCPCN;
    return TRUE ;
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
    register LINE *dlp;
    register long  dln;

    if (n < 0)
        return (forwLine(f, -n));
    
    /* if the last command was not a line move,
       reset the goal column */
    if ((lastflag&CFCPCN) == 0)
        curgoal = getcwcol();

    /* and move the point up */
    dln = curwp->line_no;
    if(n > dln)
    {
        if(dln)
        {
            dlp = lforw(curbp->b_linep) ;
            curwp->w_dotp = dlp ;
            curwp->line_no= 0 ;
            setcwcol(curgoal) ;
            curwp->w_flag |= WFMOVEL ;
        }
        return bobError() ;
    }
    /* reseting the current position */
    curwp->line_no = dln - n ;
    dlp = curwp->w_dotp;
    while(n--)
        dlp = lback(dlp);
    curwp->w_dotp = dlp ;
    setcwcol(curgoal) ;
    curwp->w_flag |= WFMOVEL ;
    if((scrollFlag & 0x10) && (curwp->topLineNo > curwp->line_no))
    {
        /* do the smooth scroll here */
        curwp->topLineNo = curwp->line_no ;
        curwp->w_flag |= (WFREDRAW|WFSBOX) ;
    }
    /* flag this command as a line move */
    thisflag = CFCPCN;
    return TRUE ;
}

/* move to a particular line. argument (n) must be a positive integer for this
 * to actually do anything
 */
int	
gotoLine(int f, int n)
{
    register int status;	/* status return */
    meUByte arg[NSTRING];	        /* buffer to hold argument */
    meInt nlno ;
    
    /* get an argument if one doesnt exist */
    if ((f == FALSE) || (n == 0))
    {
        if ((status = meGetString((meUByte *)"Goto line", 0, 0, arg, NSTRING)) != TRUE) 
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
#if NARROW
            if (f != FALSE)
                nlno += gotoAbsLine(-1) ;
            else
#endif
                nlno += curwp->line_no ;
        }
        if(nlno <= 0)
        {
            gotobob(FALSE,1) ;
            return bobError() ;
        }
#if NARROW
        if (f != FALSE)
            return gotoAbsLine(nlno) ;
#endif
    }
    else
        nlno = n ;
    
    /*--- This is an absolute jump. Go to zero & move */

    if (nlno < 1)		/* if a bogus argument...then leave */
        return(FALSE);
    
    /* always reset offset to the left hand edge */
    curgoal = 0 ;
    curwp->w_doto = 0 ;
    if(nlno < (curwp->line_no-nlno))
    {
        /* first, we go to the start of the buffer */
        curwp->w_dotp  = lforw(curbp->b_linep);
        curwp->line_no = 0;
        curwp->w_flag |= WFMOVEL ;
    }
    else if((curbp->elineno-nlno) < (nlno-curwp->line_no))
    {
        /* first, we go to the end of the buffer */
        curwp->w_dotp  = curbp->b_linep ;
        curwp->line_no = curbp->elineno ;
        curwp->w_flag |= WFMOVEL ;
    }
    curwp->w_doto  = 0 ;
    nlno -= curwp->line_no ;
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

#if NARROW
/* gotoAbsLine
 * 
 * Goto absolute line is similar to gotoLine except it considers narrowed out
 * text as well */
int	
gotoAbsLine(meInt line)
{
    int rr ;
    
    if(curbp->narrow != NULL)
        /* There are narrows - first unnarrow buffer */
        unnarrowBuffer(curbp) ;
    if(line < 0)
    {
        rr = curwp->line_no ;
        if(curbp->narrow != NULL)
            redoNarrowInfo(curbp) ;
        return rr ;
    }
    /* now move to the required line */
    rr = gotoLine(TRUE,line) ;
    
    if(curbp->narrow != NULL)
    {
        /* To ensure we get back to the right line (could be
         * currently narrowed out), drop an alpha mark.
         */
        if(rr == TRUE)
            rr = alphaMarkSet(curbp,meAM_ABSLINE,curwp->w_dotp,0,1) ;
        redoNarrowInfo(curbp) ;
        if((rr == TRUE) &&
           ((rr = alphaMarkGet(curbp,meAM_ABSLINE)) == TRUE))
        {
            /* do the buisness */
            curwp->w_dotp = curbp->b_dotp ;
            curwp->w_doto = curbp->b_doto ;
            curwp->line_no = curbp->line_no ;
            curwp->w_flag |= WFMOVEL ;
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
    curwp->w_dotp  = lforw(curbp->b_linep);
    curwp->w_doto  = 0;
    curwp->line_no = 0;
    curwp->w_flag |= WFMOVEL ;
    return (TRUE);
}

/*
 * Move to the end of the buffer. Dot is always put at the end of the file
 * (ZJ). The standard screen code does most of the hard parts of update.
 * Bound to "M->".
 */
int
gotoeob(int f, int n)
{
    curwp->w_dotp  = curbp->b_linep;
    curwp->w_doto  = 0;
    curwp->line_no = curbp->elineno;
    curwp->w_flag |= WFMOVEL ;
    return (TRUE);
}

#if	WORDPRO

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
            if(WbackChar(curwp, 1) != TRUE)
                return bobError() ;
        while(!inPWord()) ;
        
        curwp->w_doto = 0;	/* and go to the B-O-Line */
        
        /* and scan back until we hit a <NL><NL> or <NL><TAB>
           or a <NL><SPACE>					*/
        
        do {
            suc = TRUE;	/* Reset sucess flag */
            if (lback(curwp->w_dotp) == curbp->b_linep)	/* SOF  ?? */
                break;		/* Exit */
            if ((line_len = llength (curwp->w_dotp)) == 0)
                break;		/* Exit */
            for (i = 0; i < line_len ; i++)
            {
                c = lgetc (curwp->w_dotp, i);	/* Get character */
                if (c != TAB && c != ' ')	/* Character on line ?? */
                {
                    /*---	Yes - prvious line. Exit loop */
                    
                    curwp->w_dotp = lback(curwp->w_dotp);
                    curwp->line_no-- ;
                    suc = FALSE;
                    break;
                }
            }	/* End of 'for' */
        }
        while (suc != TRUE);
        
        /* and then forward until we are in a word */
        while (suc && !inPWord())
            suc = WforwChar(curwp, 1);
    }
    curwp->w_flag |= WFMOVEL ;	/* force screen update */
    return TRUE ;
}

/* go forword to the end of the current paragraph here we look for a <NL><NL>
 * or <NL><TAB> or <NL><SPACE> combination to delimit the beginning of a
 * paragraph
 */
int
forwPara(int f, int n)	
{
    register int suc=TRUE;   	/* success of last backChar */
    int	         i;		/* Local counter. */
    int	         line_len;	/* Length of the line */
    char	 c;		/* Temporary character */
    
    if (n < 0)	/* the other way...*/
        return (backPara(f, -n));

    curwp->w_flag |= WFMOVEL ;	/* force screen update */
    while (n-- > 0) 
    {	/* for each one asked for */

        /*---	First scan forward until we are in a word */
        while(!inPWord())
            if(WforwChar(curwp, 1) != TRUE)
                return eobError() ;
        
        curwp->w_doto = 0;	/* and go to the B-O-Line */
        if (suc)		/* of next line if not at EOF */
        {
            curwp->w_dotp = lforw(curwp->w_dotp);
            curwp->line_no++ ;
        }
        /* and scan forword until we hit a <NL><NL> or <NL><TAB>
           or a <NL><SPACE>					*/
        do {
            suc = TRUE;	/* Reset sucess flag */
            if (curwp->w_dotp == curbp->b_linep)	/* EOF  ?? */
                break;		/* Exit */
            if ((line_len = llength (curwp->w_dotp)) == 0)
                break;		/* Exit */
            for (i = 0; i < line_len ; i++)
            {
                c = lgetc (curwp->w_dotp, i);	/* Get character */
                if (c != TAB && c != ' ')	/* Character on line ?? */
                {
                    /*---	Goto the next line. Break ot of loop. */
                    curwp->w_dotp = lforw(curwp->w_dotp);
                    curwp->line_no++ ;
                    suc = FALSE;
                    break;
                }
            }	/* End of 'for' */
        } while (suc != TRUE);
		
	/*---	Then backward until we are in a word */

        do
            suc = WbackChar(curwp, 1);
        while (suc && !inPWord());
        curwp->w_doto = llength(curwp->w_dotp);	/* and to the EOL */
    }
    return(TRUE);
}
#endif

/*
 * Set the mark in the current window to the value of "." in the window. No
 * errors are possible. Bound to "M-.".
 */
int
setMark(int f, int n)
{
    curwp->w_markp = curwp->w_dotp;
    curwp->w_marko = curwp->w_doto;
    curwp->mlineno = curwp->line_no;
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
    register LINE  *odotp ;
    register int    odoto ;
    register long   lineno ;

    if (curwp->w_markp == NULL)
        return noMarkSet() ;
    odotp = curwp->w_dotp;
    odoto = curwp->w_doto;
    lineno = curwp->line_no ;
    curwp->w_dotp  = curwp->w_markp;
    curwp->w_doto  = curwp->w_marko;
    curwp->line_no = curwp->mlineno;
    curwp->w_markp = odotp;
    curwp->w_marko = odoto;
    curwp->mlineno = lineno;
    curwp->w_flag |= WFMOVEL ;
    return (TRUE);
}

static LINE  *orgLpp, *orgLnp ;
static meUShort orgLo ;
static meInt  orgLn ;

void
bufferPosStore(LINE *lp, meUShort lo, meInt ln)
{
    orgLpp = lback(lp) ;
    orgLnp = lforw(lp) ;
    orgLo = lo ;
    orgLn=ln ;
}

void
bufferPosUpdate(BUFFER *bp, meUInt noLines, meUShort newOff)
{
    WINDOW *wp = wheadp;
    
    while(wp != NULL)
    {
        if(wp->w_bufp == bp)
        {
            if(meModeTest(wp->w_bufp->b_mode,MDLOCK) &&
               (wp->w_bufp->intFlag & BIFLOCK))
            {
                wp->w_dotp= bp->b_dotp ;
                wp->w_doto= bp->b_doto ;
                wp->line_no = bp->line_no ;
            }
            else if(wp->line_no >= orgLn)
            {
                if(wp->line_no == orgLn)
                {
                    if(wp->w_doto < orgLo)
                        wp->w_dotp = lforw(orgLpp) ;
                    else
                    {
                        wp->w_dotp = lback(orgLnp) ;
                        wp->w_doto = wp->w_doto-orgLo+newOff ;
                        wp->line_no += noLines ;
                    }
                    if(wp->w_doto > llength(wp->w_dotp))
                       wp->w_doto = llength(wp->w_dotp) ;
                }
                else
                    wp->line_no += noLines ;
            }
            wp->w_flag |= WFMOVEL|WFMAIN|WFSBOX ;
            if(wp->mlineno == orgLn)
            {
                if(wp->w_marko < orgLo)
                    wp->w_markp = lforw(orgLpp) ;
                else
                {
                    wp->w_markp = lback(orgLnp) ;
                    wp->w_marko = wp->w_marko-orgLo+newOff ;
                    wp->mlineno += noLines ;
                }
                if(wp->w_marko > llength(wp->w_markp))
                    wp->w_marko = llength(wp->w_markp) ;
            }
            else if(wp->mlineno > orgLn)
                wp->mlineno += noLines ;
        }
        wp = wp->w_wndp;
    }
}


