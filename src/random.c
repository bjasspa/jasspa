/*****************************************************************************
 *
 *	Title:		%M%
 *
 *	Synopsis:	Random Routines.
 *
 ******************************************************************************
 *
 *	Filename:		%P%
 *
 *	Author:			Danial Lawrence.
 *
 *	Creation Date:		07/05/91 08:19		<000720.1645>
 *
 *	Modification date:	%G% : %U%
 *
 *	Current rev:		%I%
 *
 *	Special Comments:	
 *
 *	Contents Description:	
 *
 * 	This file contains the command processing functions for a number of
 *	random commands. There is no functional grouping here, for sure.
 *
 *	Jon Green	05/12/90
 *	Added "backtab". If we handle pseudo tab positions forward with spaces
 *	then we must do the same backwards to delete them. 
 *
 *	Jon Green	17/05/91
 *	Fixed fence match bug.
 *
 *	Steven Phillips	93-99
 *	Rewritten all the C-indentation and fence match code. Added sortlines,
 *  cmpWindows, createCallback and mouse support routines.
 *
 ******************************************************************************
 * 
 * Modifications to the original file by Jasspa. 
 * 
 * Copyright (C) 1988 - 1999, JASSPA 
 *
 * The MicroEmacs Jasspa distribution can be copied and distributed freely for
 * any non-commercial purposes. The MicroEmacs Jasspa Distribution can only be
 * incorporated into commercial software with the expressed permission of
 * JASSPA.
 *  
 ****************************************************************************/

/*---	Include defintions */

#define	__RANDOMC		/* Define filename */

/*---	Include files */

#include "emain.h"
#include "eskeys.h"
#ifdef _STDARG
#include <stdarg.h>		/* Variable Arguments */
#endif

#if (defined _UNIX) || (defined _DOS)
#include <sys/types.h>
#include <time.h>
#ifndef _WIN32
#include <sys/time.h>
#endif
#endif

void *
meMalloc(size_t s)
{
    register void *r ;
    if((r = malloc(s)) == NULL)
        mlwrite(MWCURSOR|MWABORT|MWWAIT,(uint8 *)"Warning! Malloc failure") ;
    return r ;
}

void *
meRealloc(void *r, size_t s)
{
    if((r = realloc(r,s)) == NULL)
        mlwrite(MWCURSOR|MWABORT|MWWAIT,(uint8 *)"Warning! Realloc failure") ;
    return r ;
}


/* case insensitive str compare
 * done by simply turning all letters to lower case
 */
int
strnicmp(const char *str1, const char *str2, size_t n)
{
    register char cc, dd ;
    for(; n>0 ; n--)
    {
        cc = *str1++ ;
        dd = *str2++ ;
        if((cc != dd) && (((char) toggleCase(cc)) != dd))
            return 1 ;
        if(cc == 0)
            break ;
    }
    return 0 ;
}

/* case insensitive str compare
 * done by simply turning all letters to lower case
 */
int
stricmp(const char *str1, const char *str2)
{
    register char  cc, dd ;
    int ii ;
    
    do {
        cc = *str1++ ;
        dd = *str2++ ;
        if((cc != dd) &&
           ((ii=((char) toLower(cc)) - ((char) toLower(dd))) != 0))
            return ii ;
    } while(cc != 0) ;
    return 0 ;
}

/* case insensitive str compare/diff
 * This is slightly different to the above as if the 
 * strings are the same when ignore the case, it then returns
 * the case sensitive result.
 */
int
stridif(const char *str1, const char *str2)
{
    register char cc, dd ;
    register int  rr=0, ss, tt ;
    
    do {
        cc = *str1++ ;
        dd = *str2++ ;
        if((ss = cc - dd) != 0)
        {
            if((tt = ((char) toLower(cc)) - ((char) toLower(dd))) != 0)
                return tt ;
            if(!rr)
                rr = ss ;
        }
    } while(cc != 0) ;
    return rr ;
}

/* 
** sort the given list of strings out on the nth plus character.
** Can be used on lines by adding (sizeof(LINE)-1) to the offset.
*/
#ifdef _NOQSORT
void
sortStrings(int noStr, uint8 **strs, int offset, Fintss cmpFunc)
{
    int chng, i ;
    uint8 *tmp ;
	
    if(offset < 0)
    {
        offset = -1-offset ;
        do
        {
            chng=0 ;
            for(i=0 ; i<noStr-1 ; i++)
                if(cmpFunc((const char *) strs[i]+offset,(const char *) strs[i+1]+offset) < 0)
                {
                    tmp = strs[i] ;
                    strs[i] = strs[i+1] ;
                    strs[i+1] = tmp ;
                    chng=1 ;
                }
        } while(chng) ;
    }
    else
    {
        do
        {
            chng=0 ;
            for(i=0 ; i<noStr-1 ; i++)
                if(cmpFunc((const char *) strs[i]+offset,(const char *) strs[i+1]+offset) > 0)
                {
                    tmp = strs[i] ;
                    strs[i] = strs[i+1] ;
                    strs[i+1] = tmp ;
                    chng=1 ;
                }
        } while(chng) ;
    }
}

#else

Fintss sortStringsCmpFunc ;
int sortStringsOffset ;
int sortStringsBackward ;

static int
sortStringsCmp(const void *v1, const void *v2)
{
    int ii ;
    
    ii = sortStringsCmpFunc(*((const char **) v1)+sortStringsOffset,*((const char **) v2)+sortStringsOffset) ;
    if(sortStringsBackward)
        ii = 0-ii ;
    return ii ;
}

void
sortStrings(int noStr, uint8 **strs, int offset, Fintss cmpFunc)
{
    if((sortStringsBackward=(offset < 0)))
        offset = -1-offset ;
    sortStringsOffset = offset ;
    sortStringsCmpFunc = cmpFunc ;
    qsort(strs, noStr, sizeof(char *), sortStringsCmp);
}

#endif

int
sortLines(int f, int n)
{
    Fintss    cmpFunc ;
    LINE     *sL, *eL ;
    register  LINE *l, **list ;
    int32     sln, noL, ii, jj, len ;
    int       offs ;
    
    if (curwp->w_markp == NULL)
        return noMarkSet() ;
    
    if((noL=curwp->line_no-curwp->mlineno) == 0)
        return TRUE ;
    if((ii=bchange()) != TRUE)               /* Check we can change the buffer */
        return ii ;
    if(noL < 0)
    {
        noL = 0 - noL ;
        sln = curwp->line_no ;
        sL = lback(curwp->w_dotp) ;
        eL = curwp->w_markp ;
    }
    else
    {
        sln = curwp->mlineno ;
        sL = lback(curwp->w_markp) ;
        eL = curwp->w_dotp ;
        curwp->w_dotp  = curwp->w_markp ;
        curwp->line_no = sln ;
    }
    if(f == FALSE)
        n = 0 ;
    offs = (int) ((size_t) sL->l_text) - ((size_t) sL) ;
    if(n < 0)
    {
        offs = 0-offs ;
        f = -1-n ;
    }
    else
        f = n ;
    if((list = (LINE **) meMalloc(noL*sizeof(LINE *))) == NULL)
        return FALSE ;
    for(ii=0, len=0, l=lforw(sL) ; ii<noL ; l=lforw(l),ii++)
    {
        list[ii] = l ;
        len += llength(l)+1 ;
    }
    if(l != eL)
        return ctrlg(FALSE,1);
    curwp->w_doto = 0 ;
#if MEUNDO
    meUndoAddDelChars(len) ;
    meUndoAddReplaceEnd(len) ;
#endif
    
    if(meModeTest(curbp->b_mode,MDEXACT))
        cmpFunc = strcmp ;
    else
        cmpFunc = stridif ;
    if(f != 0)
    {
        for(ii=0,jj=0 ; ii<noL ; ii++)
        {
            if(llength(list[ii]) < f)
            {
                l = list[jj] ;
                list[jj] = list[ii] ;
                list[ii] = l ;
                jj++ ;
            }
        }
        sortStrings(jj,(uint8 **)list, offs,cmpFunc) ;
    }
    else
        jj = 0 ;
    sortStrings(noL-jj,(uint8 **)(list+jj), n+offs,cmpFunc) ;
    
    for(ii=0, l=sL ; ii<noL ; l=lforw(l),ii++)
    {
        l->l_fp = list[ii] ;
        list[ii]->l_bp = l ;
    }
    l->l_fp = eL ;
    eL->l_bp = l ;
    
    curwp->w_markp = *list ;
    curwp->w_marko = 0 ;
    curwp->mlineno = sln ;
    curwp->w_dotp  = eL ;
    curwp->line_no = sln+noL ;
    
    meFree(list) ;
    curwp->w_flag |= WFMOVEL|WFMAIN ;
    
    return TRUE ;
}


int
getBufferInfo(int32 *numlines, int32 *predlines,
              int32 *numchars, int32 *predchars)
{
    register LINE   *lp;		/* current line */
    register int    curchar = 0;	/* character under cursor */
    
    /* starting at the beginning of the buffer */
    lp = lforw(curbp->b_linep);
    
    /* start counting chars and lines */
    *numchars = *numlines = 0;
    while (lp != curbp->b_linep)
    {
        if(lp->l_text[llength(lp)] != '\0')
        {
            lp->l_text[llength(lp)] = '\0' ;
            fprintf(stderr,"We've got problems on line [%s]\n",lp->l_text) ;
        }
        /* if we are on the current line, record it */
        if (lp == curwp->w_dotp)
        {
            *predlines = *numlines;
            *predchars = *numchars + curwp->w_doto;
            if((curchar = lgetc(lp, curwp->w_doto)) == '\0')
                curchar = meNLCHAR;
        }
        /* on to the next line */
        (*numlines)++ ;
        *numchars += llength(lp) + 1;
        lp = lforw(lp);
    }
    
    /* if at end of file, record it */
    if (curwp->w_dotp == curbp->b_linep)
    {
        *predlines = *numlines;
        *predchars = *numchars;
        curchar = meNLCHAR;
    }
    return curchar ; 
}


/*
 * Display the current position of the cursor, in origin 1 X-Y coordinates,
 * the character that is under the cursor (in hex), and the fraction of the
 * text that is before the cursor. The displayed column is not the current
 * column, but the column that would be used on an infinite width display.
 * Normally this is bound to "C-X =".
 */
int
bufferInfo(int f, int n)
{
    int32 numchars ;			/* # of chars in file */
    int32 numlines ;			/* # of lines in file */
    int32 predchars ;			/* # chars preceding point */
    int32 predlines ;			/* # lines preceding point */
    int   curchar ;			/* character under cursor */
    int   ratio;
    int   col;
    int   savepos;			/* temp save for current offset */
    int   ecol;				/* column pos/end of current line */
    
    curchar = getBufferInfo(&numlines,&predlines,&numchars,&predchars) ;
    
    /* Get real column and end-of-line column. */
    col = getccol();
    savepos = curwp->w_doto;
    curwp->w_doto = llength(curwp->w_dotp);
    ecol = getccol();
    curwp->w_doto = savepos;
    
    ratio = 0;              /* Ratio before dot. */
    if (numchars != 0)
        ratio = (100L*predchars) / numchars;
    
    /* summarize and report the info */
    sprintf((char *)resultStr,"Line %ld/%ld Col %d/%d Char %ld/%ld (%d%%) Win Line %d/%d Col %d/%d char = 0x%x",
            predlines+1, numlines+1, savepos, llength(curwp->w_dotp),predchars, numchars, ratio, 
            (int) (curwp->line_no-curwp->topLineNo), curwp->numTxtRows-1,col, ecol, curchar);
    return mlwrite(MWSPEC,resultStr) ;
}

int
getcline(WINDOW *wp)	/* get the current line number */
{
    register LINE	*lp;		/* current line */
    register int	numlines;	/* # of lines before point */
    
    /* starting at the beginning of the buffer */
    lp = lforw(wp->w_bufp->b_linep);
    
    /* start counting lines */
    numlines = 0 ;
    while (lp != wp->w_bufp->b_linep) 
    {
        /* if we are on the current line, record it */
        if (lp == wp->w_dotp)
            break;
        ++numlines;
        lp = lforw(lp);
    }
    
    /* and return the resulting count */
    return numlines ;
}

int
getcol(uint8 *ss, int off)
{
    register int c, col=0 ;
    
    while(off--)
    {
        c = *ss++ ;
        if(isDisplayable(c))
            col++ ;
        else if(c == TAB)
            col += get_tab_pos(col) + 1 ;
        else if (c  < 0x20)
            col += 2 ;
        else
            col += 4 ;
    }
    return col ;
}

/*
 * Set current column.
 */
int
setccol(int pos)
{
    register int c; 	/* character being scanned */
    register int i; 	/* index into current line */
    register int col;	/* current cursor column   */
    register int llen;	/* length of line in bytes */
    
    col = 0;
    llen = llength(curwp->w_dotp);
    
    /* scan the line until we are at or past the target column */
    for (i = 0; i < llen; ++i)
    {
        /* advance one character */
        c = lgetc(curwp->w_dotp, i);
        if(isDisplayable(c))
            col++ ;
        else if(c == TAB)
            col += get_tab_pos(col) + 1 ;
        else if (c  < 0x20)
            col += 2 ;
        else
            col += 4 ;
        /* upon reaching the target, drop out */
        if (col > pos)
            break;
        
    }
    /* set the new position */
    curwp->w_doto = i;
    /* if not long enough return an error */
    return ((col < pos) ? FALSE:TRUE) ;
}

int
getcwcol(void)
{
    uint8 *off ;
    int ii, col=0 ;
    
    windCurLineOffsetEval(curwp) ;
    
    off = curwp->curLineOff->l_text ;
    ii = curwp->w_doto ;
    while(--ii >= 0)
        col += *off++ ;
    return col ;
}
int
setcwcol(int col)
{
    uint8 *off ;
    int ii, jj ;
    
    windCurLineOffsetEval(curwp) ;
    
    off = curwp->curLineOff->l_text ;
    ii = 0 ;
    jj = curwp->w_dotp->l_used ;
    while((ii < jj) && (col > 0))
        col -= off[ii++] ;
    curwp->w_doto = ii ;
    return ((col) ? FALSE:TRUE) ;
}

/*
 * Twiddle the two characters on either side of dot. If dot is at the end of
 * the line transChars the two characters before it. Return with an error if dot
 * is at the beginning of line; it seems to be a bit pointless to make this
 * work. This fixes up a very common typo with a single stroke. Normally bound
 * to "C-T". This always works within a line, so "WFMAIN" is good enough.
 */
int
transChars(int f, int n)
{
    LINE         *dotp;
    int           doto;
    uint8 cl, cr;
    
    dotp = curwp->w_dotp;
    doto = curwp->w_doto;
    if((doto==llength(dotp)) && (doto > 0))
        curwp->w_doto-- ;
    if(curwp->w_doto == 0)
    {
        curwp->w_doto = doto ;
        return FALSE ;
    }
    cr = lgetc(dotp, curwp->w_doto) ;
    curwp->w_doto-- ;
    if(bchange() != TRUE)               /* Check we can change the buffer */
        return ABORT ;
    cl = lgetc(dotp, curwp->w_doto);
    lchange(WFMAIN);
#if MEUNDO
    meUndoAddRepChar() ;
    lputc(dotp, curwp->w_doto, cr);
    curwp->w_doto++ ;
    meUndoAddRepChar() ;
    lputc(dotp, curwp->w_doto, cl);
#else
    lputc(dotp, doto+0, cr);
    lputc(dotp, doto+1, cl);
#endif
    curwp->w_doto = doto ;
    return (TRUE);
}

/*
 * Twiddle the line before with dot nad move down (unless at the end where 
 * there is no advance. If dot is the top the move down 1 to start.
 * Normally bound ^X^T.
 */
int
transLines(int f, int n)
{
    register LINE   *ln1, *ln2 ;
    register int    i, len ;
    
    if((n<0) && (backLine(TRUE, 1) != TRUE))
        return FALSE ;
    if((i=bchange()) != TRUE)               /* Check we can change the buffer */
        return i ;
    lchange(WFMAIN|WFMOVEL);
    curwp->w_doto = 0 ;
    for(i=0 ; i<abs(n) ; i++)
    {
        if(((ln1 = curwp->w_dotp) == lback(curbp->b_linep)) ||
           ((ln2 = lforw(ln1)) == curbp->b_linep))
            return FALSE ;
#if MEUNDO
        len = llength(ln1) + llength(ln2) + 2 ;
        meUndoAddDelChars(len) ;
        meUndoAddReplaceEnd(len) ;
#endif
        lback(lforw(ln2)) = ln1 ;
        lforw(lback(ln1)) = ln2 ;
        lback(ln2) = lback(ln1) ;
        lforw(ln1) = lforw(ln2) ;
        lback(ln1) = ln2 ;
        lforw(ln2) = ln1 ;
        curwp->line_no++ ;
        if((n<0) && (backLine(TRUE, 2) != TRUE))  
            /* move back over one swapped aswell */
            return FALSE ;
    }
    if(n<0) 
        forwLine(TRUE, 1) ;
    update(FALSE) ;
    return (TRUE);
}

int
quoteKeyToChar(uint16 cc)
{
    if(cc > 255)
    {
        switch(cc)
        {
        case ME_SPECIAL|SKEY_backspace:
            cc = 0x08 ;
            break ;
        case ME_SPECIAL|SKEY_delete:
            cc = 0x7f ;
            break ;
        case ME_SPECIAL|SKEY_esc:
            cc = 0x1b ;
            break ;
        case ME_SPECIAL|SKEY_return:
            cc = 0x0d ;
            break ;
        case ME_SPECIAL|SKEY_tab:
            cc = 0x09 ;
            break ;
        case ME_SPECIAL|SKEY_up:
            cc = 'P' - '@' ;
            break ;
        case ME_SPECIAL|SKEY_down:
            cc = 'N' - '@' ;
            break ;
        case ME_SPECIAL|SKEY_left:
            cc = 'B' - '@' ;
            break ;
        case ME_SPECIAL|SKEY_right:
            cc = 'F' - '@' ;
            break ;
        default:
            return -1 ;
        }
    }
    return cc ;
}


/*
 * Quote the next character, and insert it into the buffer. All the characters
 * are taken literally, with the exception of the newline, which always has
 * its line splitting meaning. The character is always read, even if it is
 * inserted 0 times, for regularity. Bound to "C-Q"
 */
int
quote(int f, int n)
{
    register int    s;
    register int    c;
    
    if (n < 0)
        return (FALSE);
    if (n == 0)
        return (TRUE);
    if((c = mlCharReply((uint8 *)"Quote: ",mlCR_QUOTE_CHAR,NULL, NULL)) < 0)
        return mlwrite(MWABORT,(uint8 *)"[Cannot quote this key]") ;
    if((s=bchange()) != TRUE)               /* Check we can change the buffer */
        return s ;
    if(c == meNLCHAR)
    {
        do 
            s = lnewline();
        while((s==TRUE) && --n);
#if MEUNDO
        meUndoAddInsChars(n) ;
#endif
    }
    else
    {
        /* insert the required number of chars */
        s = insertChar(c,n) ;
    }
    return s ;
}


/*
 * Set tab size if given non-default argument (n <> 1).  Otherwise, insert a
 * tab into file.  If given argument, n, of zero, change to true tabs.
 * If n > 1, simulate tab stop every n-characters using spaces. This has to be
 * done in this slightly funny way because the tab (in ASCII) has been turned
 * into "C-I" (in 10 bit code) already. Bound to "C-I".
 */
int
meTab(int f, int n)
{
    int ii ;
    
    if(n<=0)
        return TRUE ;
    
    if((meSystemCfg & meSYSTEM_TABINDALW) || (curwp->w_doto == 0))
    {
        if(curbp->indent)
            return indentLine() ;
        if(meModeTest(curbp->b_mode,MDCMOD))
            return doCindent(&ii) ;
    }
    if((ii=bchange()) != TRUE)               /* Check we can change the buffer */
        return ii ;
    
    /* If a newline was typed, fill column is defined, the argument is non-
     * negative, wrap mode is enabled, and we are now past fill column,
     * and we are not read-only, perform word wrap.
     */
    if(meModeTest(curwp->w_bufp->b_mode,MDWRAP) && (fillcol > 0) &&
       (getccol() > fillcol))
        wrapWord(FALSE, 1);
    if(!meModeTest(curbp->b_mode,MDTAB))
    {
        /* insert the required number of TABs */
        ii = insertChar(TAB,n) ;
    }
    else
    {
        int ss = (tabsize*(n-1)) + (tabsize - (getccol()%tabsize)) - n ;
        /* insert the required number of TABs as spaces first - this handles over mode
         * The extra spaces required are inserted next */
        if(((ii = insertChar(' ',n)) == TRUE) && ss &&
           ((ii = linsert(ss,' ')) == TRUE))
#if MEUNDO
            meUndoAddInsChars(ss) ;
#endif
    }
    return ii ;
}

/*
 * Delete back a tab-stop. If the previous character was a <TAB> then delete.
 * If the pseudo tabs are in operation then delete spaces to the next tab stop
 * position. 
 * backtab is bound to 'ESC-I'
 */

int
meBacktab(int f, int n)
{
    uint8 cc;		/* Character buffer */
    int tabspace;	/* Count of the size of the TAB space */
    int delspace;	/* Count of the size of the delete space */
    int doto;		/* Current position in line - 1 */
    
    /*---	If we are at the beginning of the line or the line has no length then
       ignore the request */
    
    if ((llength(curwp->w_dotp) == 0) ||	/* Zero line length ?? */
        ((doto = curwp->w_doto-1) < 0))	/* At start of line ?? */
        return (FALSE);			/* Yes - quit out */
    
    if(bchange() != TRUE)               /* Check we can change the buffer */
        return ABORT ;
    
    /* Delete the tabular space. */
    
    if(meModeTest(curbp->b_mode,MDTAB))
    {
        /*---	Forced tab spacing is in operation.
           Get the tabular spacing from our current position */
        
        if ((tabspace = (getccol() % tabsize)) == 0)
            tabspace = tabsize;
        
        /*---	Scan back through the characters in the line and determine the 
           number of characters to remove */	
        
        for (delspace = 0; delspace < tabspace; delspace++)
        {
            if (doto < 0)		/* At start of line ?? */
                break;		/* Yes - quit out */
            
            /*---	Get the character. If it is a <TAB> or any other
               character other than space then exit */
            
            cc = lgetc (curwp->w_dotp, doto);
            if (cc != ' ')
            {
                if (cc == TAB)	
                    delspace++;	/* Remove <TAB> char */
                break;			/* Quit */
            }
            else
                doto--;		/* Approaching start of line */
        }
        
        /*---	If there are characters to delete then adjust 'dot' and then
           remove in one go. */
        
        if (delspace)
        {
            curwp->w_doto -= delspace;
            return (ldelete((int32) delspace, 2));
        }
    }
    else
    {
        /*---	Natural tab spacing. If the previous character is a <TAB> then
           delete the <TAB> character from the line */
        
        if (lgetc(curwp->w_dotp, doto) == TAB) 
        {   /* Tab char ?? */
            curwp->w_doto = doto;		/* Yes - back up */
            return(ldelete(1L,2));	/* and delete */
        }
    }
    return (FALSE);		/* Nothing to remove !!! */
}

/*
 * Open up some blank space. The basic plan is to insert a bunch of newlines,
 * and then back up over them. Everything is done by the subcommand
 * procerssors. They even handle the looping. Normally this is bound to "C-O".
 */
int
insLine(int f, int n)
{
    register int    i;
    register int    s;
    
    if (n <= 0)
        return TRUE ;
    if((s=bchange()) != TRUE)               /* Check we can change the buffer */
        return s ;
    i = n;                                  /* Insert newlines.     */
    while((s==TRUE) && i--)
        s = lnewline();
    if (s == TRUE)
#if MEUNDO
        meUndoAddInsChars(n) ;
#endif
    return (s);
}

int
winsert(void)	/* insert a newline and indentation for Wrap indent mode */
{
    /*---	This is nasty if the user is not using tab because we expect 
       to drop a tab position when  we come back. Hence on spaced tab we need
       to consume to the previous tab stop.  */
    
    register uint8 *cptr;	/* string pointer into text to copy */
    register int tptr;	        /* index to scan into line */
    register int i;
    uint8 ichar[NSTRING];	/* buffer to hold indent of last line */
    
    /* grab a pointer to text to copy indentation from */
    cptr = &curwp->w_dotp->l_text[0];
    
    /* check for a brace */
    tptr = curwp->w_doto ;
    
    /* save the indent of the previous line */
    i = 0;
    while ((i != tptr) && (cptr[i] == ' ' || cptr[i] == '\t')
           && (i < NSTRING - 1))
    {
        ichar[i] = cptr[i] ;
        i++ ;
    }
    /* put in the newline */
    if (lnewline() == FALSE)
        return(FALSE);
    
    if(i && (i != tptr))
    {
        /* and the saved indentation */
        lsinsert(i, ichar) ;
#if MEUNDO
        meUndoAddInsChars(i+1) ;
#endif
        /*        if(meModeTest(curbp->b_mode,MDTAB) && cptr[tptr] == ' ')*/
        /*            meBacktab(FALSE, 1);*/
    }
#if MEUNDO
    else
        meUndoAddInsChars(1) ;
#endif
    return(TRUE);
}

static int
indentInsert(void)
/* insert a newline and indentation for C */
{
    indentLine() ;
    /* put in the newline */
#if MEUNDO
    meUndoAddInsChar() ;
#endif
    if (lnewline() == FALSE)
        return FALSE ;
    return indentLine() ;
}

int
meLineSetIndent(int curInd, int newInd)
{
    register int ss, curOff ;
    if((ss=bchange()) != TRUE)               /* Check we can change the buffer */
        return ss ;
    curOff = curwp->w_doto ;
    curwp->w_doto = 0 ;
    curOff -= curInd ;
    ldelete(curInd,6) ;
    if(meModeTest(curbp->b_mode,MDTAB))
        ss = 0 ;
    else
    {
        ss = newInd / tabwidth ;
        newInd -= ss * tabwidth ;
        linsert(ss,'\t') ;
    }
    linsert(newInd,' ') ;
    ss += newInd ;
    curOff += ss ;
#if MEUNDO
    if(meModeTest(curbp->b_mode,MDUNDO))
    {
        meUndoAddReplaceEnd(ss) ;
        curbp->fUndo->doto = ss ;
        curbp->fUndo->type |= MEUNDO_REVS ;
    }
#endif
    curwp->w_doto = curOff ;
    return TRUE ;
}

int
cinsert(void)
/* insert a newline and indentation for C */
{
    int inComment, doto ;
    uint8 *str ;
    
    doCindent(&inComment) ;
    
    /* put in the newline */
#if MEUNDO
    meUndoAddInsChar() ;
#endif
    if (lnewline() == FALSE)
        return FALSE ;
    doCindent(&inComment) ;
    if(inComment && (commentCont != NULL))
    {    
        doto = curwp->w_doto ;
        if(gotoFrstNonWhite() == 0)
        {
            int newInd ;
            curwp->w_doto = doto ;
            if((newInd = getccol() - 3) < 0)
                newInd = 0 ;
            if(meLineSetIndent(doto,newInd) != TRUE)
                return FALSE ;
            str = commentCont ;
            while(*str != '\0')
                linsert(1, *str++) ;
            if((doto=((size_t) str) - ((size_t) commentCont)) > 0)
#if MEUNDO
                meUndoAddInsChars(doto) ;
#endif
        }
        else
            curwp->w_doto = doto ;
    }
    return TRUE ;
}

/*
 * Insert a newline. Bound to "C-M". If we are in CMODE, do automatic
 * indentation as specified.
 */
int
meNewline(int f, int n)
{
    register int    s;
    
    if (n == 0)
        return TRUE ;
    if (n < 0)
        return FALSE ;
    if((s=bchange()) != TRUE)               /* Check we can change the buffer */
        return s ;
    
    /* If a newline was typed, fill column is defined, the argument is non-
     * negative, wrap mode is enabled, and we are now past fill column,
     * and we are not read-only, perform word wrap.
     */
    if(meModeTest(curwp->w_bufp->b_mode,MDWRAP) && (fillcol > 0) &&
       (getccol() > fillcol))
        wrapWord(FALSE, 1);
    
    if(meModeTest(curwp->w_bufp->b_mode,MDJUST) &&
       ((fillmode == 'c') || (fillmode == 'r')))
        f = 1 ;
    else
        f = 0 ;
    while (n--)
    {
        if(curbp->indent)
            s = indentInsert() ;
        else if(meModeTest(curbp->b_mode,MDCMOD))
            s = cinsert() ;
        else if(meModeTest(curbp->b_mode,MDINDEN))
            s = winsert() ;
        else
        {
#if MEUNDO
            meUndoAddInsChar() ;
#endif
            s = lnewline() ;
        }
        if (s != TRUE)
            return s ;
        if(f)
        {
            f = 0 ;
            justify(-1) ;
        }
    }
    
    return TRUE ;
}

/*
 * Delete backwards. This is quite easy too, because it's all done with other
 * functions. Just move the cursor back, and delete forwards. Like delete
 * forward, this actually does a kill if presented with an argument. Bound to
 * both "RUBOUT" and "C-H".
 */
int
backDelChar(int f, int n)
{
    register int    s;
    int keep ;
    
    if(n == 0)
        return TRUE ;
    if (n < 0)
        return (forwDelChar(f, -n));
    if((s=bchange()) != TRUE)               /* Check we can change the buffer */
        return s ;
    
    if((f != FALSE) || meModeTest(curbp->b_mode,MDLETTR))
        keep = 3 ;                      /* Save in kill */
    else
        keep = 2 ;
    
    if((s = WbackChar(curwp, n)) == TRUE)
        /*
         * Always make ldelete save the deleted stuff in a kill buffer
         * unless only one character and in letter kill mode.
         */
        s = ldelete((int32)n, keep);
    
    return s ;
}


/*
 * Delete blank lines around dot. What this command does depends if dot is
 * sitting on a blank line. If dot is sitting on a blank line, this command
 * deletes all the blank lines above and below the current line. If it is
 * sitting on a non blank line then it deletes all of the blank lines after
 * the line. Normally this command is bound to "C-X C-O". Any argument is
 * ignored.
 */
int
deblank(int f, int n)
{
    register LINE   *lp1;
    register LINE   *lp2;
    long     nld, lld=0 ;
    
    lp1 = curwp->w_dotp;
    while (llength(lp1)==0 && (lp2=lback(lp1))!=curbp->b_linep)
    {
        lp1 = lp2;
        lld++ ;
    }
    lp2 = lp1;
    nld = 0;
    while ((lp2=lforw(lp2))!=curbp->b_linep && llength(lp2)==0)
        ++nld;
    if (nld == 0)
        return (TRUE);
    if(bchange() != TRUE)               /* Check we can change the buffer */
        return ABORT ;
    curwp->w_dotp = lforw(lp1);
    curwp->w_doto = 0;
    curwp->line_no -= lld-1 ;
    return (ldelete(nld,2));
}


/*
 * Delete forward. This is real easy, because the basic delete routine does
 * all of the work. Watches for negative arguments, and does the right thing.
 * If any argument is present, it kills rather than deletes, to prevent loss
 * of text if typed with a big argument. Normally bound to "C-D".
 */
int
forwDelChar(int f, int n)
{
    register int    s;
    int keep ;
    
    if(n == 0)
        return TRUE ;
    if (n < 0)
        return (backDelChar(f, -n));
    if((s=bchange()) != TRUE)               /* Check we can change the buffer */
        return s ;
    
    if((f != FALSE) || meModeTest(curbp->b_mode,MDLETTR))
        keep = 3 ;                      /* Save in kill */
    else
        keep = 2 ;
    
    /*
     * Always make ldelete save the deleted stuff in a kill buffer
     * unless only one character and in letter kill mode.
     */
    return (ldelete((int32)n, keep));
}


/*
 * Kill text. If called without an argument, it kills from dot to the end of
 * the line, unless it is at the end of the line, when it kills the newline.
 * If called with an argument of 0, it kills from the start of the line to dot.
 * If called with a positive argument, it kills from dot forward over that
 * number of newlines. If called with a negative argument it kills backwards
 * that number of newlines. Normally bound to "C-K".
 */
int
killLine(int f, int n)
{
    register LINE   *nextp;
    register int     s ;
    long chunk;
    
    if((s=bchange()) != TRUE)               /* Check we can change the buffer */
        return s ;
    if(curwp->w_dotp == curbp->b_linep)
        chunk = 0 ;
    else if(f == FALSE)
    {
        chunk = llength(curwp->w_dotp)-curwp->w_doto;
        if (chunk == 0)
            chunk = 1;
        else if(!meModeTest(curbp->b_mode,MDLINE) &&   /* if line kill mode    */
                (curwp->w_doto == 0) )                 /* whole line           */
            chunk++ ;
    }
    else if (n == 0)
    {
        chunk = curwp->w_doto;
        curwp->w_doto = 0;
    }
    else if (n > 0) 
    {
        chunk = llength(curwp->w_dotp)-curwp->w_doto+1;
        nextp = lforw(curwp->w_dotp);
        while (--n)
        {
            if (nextp == curbp->b_linep)
                return (FALSE);
            chunk += llength(nextp)+1;
            nextp = lforw(nextp);
        }
    }
    else
        return mlwrite(MWABORT,(uint8 *)"neg kill");
    
    return(ldelete(chunk,3));
}



/* ARGSUSED */

/*	This function simply clears the message line,
   mainly for macro usage			*/

int
mlClear(int f, int n)
{
    mlerase(0);
    return(TRUE);
}

/*	This function writes a string on the message line
   mainly for macro usage			*/

int
mlWrite(int f, int n)
{
    register int status;
    uint8 buf[MAXBUF];	/* buffer to recieve message into */
    
    if ((status = mlreply((uint8 *)"Message", 0, 0, buf, MAXBUF)) != TRUE)
        return(status);
    
    mlwrite(MWSPEC,buf);
    if((f==TRUE) && (n>0))
        TTsleep(n,0);
    return(TRUE);
}

#if	CFENCE

/* List of fense id chars, close (or move backward) first then open */
uint8 fenceString[] = "##/*)(}{][" ; /* */

uint8
gotoFrstNonWhite(void)
{
    register uint8 ch ;
    
    while(((ch = lgetc(curwp->w_dotp,curwp->w_doto)) != '\0') &&
          ((ch == ' ') || (ch == '\t')))
        curwp->w_doto++ ;
    return ch ;
}

static int
findfence(uint8 ch, uint8 forwFlag) ;

/* move forward of backward the the next non-white char
 * skipping '#' lines and comments
 * 
 * Flags passed in are
 * 
 * 0x01 - dont skip # lines
 * 0x02 - currently in a #define with '\'s
 * 0x04 - in a C comment, so dont skip other comments
 */
#define MTNW_INCOM     0x01
#define MTNW_KNOWN     0x02
#define MTNW_NOTINHASH 0x04
#define MTNW_SKIPHASH  0x08
#define MTNW_SIMPLE    0x10

static int
moveToNonWhite(uint8 forwFlag, uint8 *flags)
{
    register uint8 ch ;
    register LINE *lp ;
    uint8 *ss, lc ;
    int inq ;
    
    for(;;)
    {
        if(forwFlag)
        {
            for(;;)
            {
                if(curwp->w_doto >= llength(curwp->w_dotp))
                {
                    /* Go back lines until we find a line which
                     * 1) Doesn't start with a '#'
                     * 2) Has some text on it
                     * Move cursor to end or if we find a c++ "//" comment
                     * then to the char before that
                     */
                    if(!((*flags) & MTNW_KNOWN))
                        *flags |= MTNW_KNOWN|MTNW_NOTINHASH ;
                    if((lp = lforw(curwp->w_dotp)) == curbp->b_linep)
                        return 0 ;
                    curwp->w_dotp = lp ;
                    curwp->line_no++ ;
                    ss=lp->l_text ;
                    while((ch = *ss++) != '\0')
                        if((ch != ' ') && (ch != '\t'))
                            break ;
                    if(ch == '\0')
                        continue ;
                    if((ch == '#') && ((*flags) & MTNW_NOTINHASH))
                    {
                        if((ss[0] == 'e') && (ss[1] == 'l'))
                        {
                            /* Moving forwards, we want to skip #else & #elifs,
                             * but findfence moves to the next #e? if it was an #elif
                             * so we must check it is an endif
                             */
                            curwp->w_doto = ((size_t) ss - (size_t) lp->l_text) - 1 ;
                            do {
                                if(findfence('#',forwFlag) != TRUE)
                                    return 0 ;
                            } while(lgetc(curwp->w_dotp,curwp->w_doto+2) != 'n') ;
                        }
                        /* skip the hash line itself, and if line has a \ then next etc */
                        inq = 0 ;
                        lc = ' ' ;
                        do
                        {
                            if((ch == '"') && (lc != '\\') && !(inq & 0x2))
                                inq ^= 1 ;
                            else if((ch == '*') && (lc == '/') && !inq)
                                inq = 2 ;
                            else if((ch == '/') && (lc == '*') && (inq == 2))
                                inq = 0 ;
                            else if(!inq)
                            {
                                /* Exit if we're at a c++ comment */
                                if((ch == '/') && (lc == '/'))
                                    ss = lp->l_text + llength(lp) ;
                                /* Exit if we're in a hash define and we're at a '\' */
                                else if((ch == '\\') && (lc != '\\') && (lc != '\''))
                                {
                                    if((lp = lforw(curwp->w_dotp)) == curbp->b_linep)
                                        return 0 ;
                                    curwp->w_dotp = lp ;
                                    curwp->line_no++ ;
                                    ss=lp->l_text ;
                                    ch = ' ' ;
                                }
                            }
                            lc = ch ;
                        } while((ch = *ss++) != '\0') ;
                        curwp->w_doto = llength(curwp->w_dotp) ;
                        continue ;
                    }
                    curwp->w_doto = ((size_t) ss - (size_t) lp->l_text) - 1 ;
                    ch = lgetc(curwp->w_dotp,curwp->w_doto) ;
                    break ;
                }
                else
                {
                    curwp->w_doto++ ;
                    ch = lgetc(curwp->w_dotp,curwp->w_doto) ;
                    if(ch == '\\')
                    {
                        if(!((*flags) & MTNW_KNOWN))
                            *flags |= MTNW_KNOWN ;
                        if(!((*flags) & MTNW_NOTINHASH))
                            continue ;
                    }
                    break ;
                }
            }
            
        }
        else
        {
            if(curwp->w_doto == 0)
            {
                for(;;)
                {
                    int comCont ;
                    uint8 *fss ;
newline_skip:
                    /* Go back lines until we find a line which
                     * 1) Doesn't start with a '#'
                     * 2) Has some text on it
                     * Move cursor to end or if we find a c++ "//" comment
                     * then to the char before that
                     */
                    if((lp = lback(curwp->w_dotp)) == curbp->b_linep)
                        return 0 ;
                    curwp->w_dotp = lp ;
                    curwp->line_no-- ;
                    ss=lp->l_text ;
                    while((ch = *ss++) != '\0')
                        if((ch != ' ') && (ch != '\t'))
                            break ;
                    fss = ss ;
                    comCont = 0 ;
                    if(ch == '\0')
                        ss = lp->l_text ;
                    else if((*flags) & MTNW_SIMPLE)
                        ss = lp->l_text + llength(lp) + 1 ;
                    else
                    {
                        if(((*flags) & MTNW_NOTINHASH) && (ch == '#') && !((*flags) & MTNW_SKIPHASH))
                        {
hash_skip:
                            /* Moving backwards, we want to skip #else & #elifs
                             * find fence does this nicely for us
                             */
                            if((ss[0] == 'e') && (ss[1] == 'l'))
                            {
                                curwp->w_doto = ((size_t) ss - (size_t) lp->l_text) - 1 ;
                                if(findfence('#',forwFlag) != TRUE)
                                    return 0 ;
                            }
                            /* skip the hash line itself */
                            continue ;
                        }
                        inq = 0 ;
                        lc = ' ' ;
                        do
                        {
                            if((ch == '"') && (lc != '\\') && (comCont <= 0))
                                inq ^= 1 ;
                            else if((ch == '*') && (lc == '/') && !inq)
                            {
                                /* change ch from '*' to another bogus char, this stops / * / / /
                                 * being misinterpreted */
                                comCont++ ;
                                ch = 0x01 ;
                            }
                            else if((ch == '/') && (lc == '*') && !inq)
                            {
                                /* change ch from '/' to another bogus char, this stops the double '/'
                                 * created at a stop start c comment being interpreted as a c++ comment */
                                comCont-- ;
                                ch = 0x01 ;
                            }
                            else if(!inq && (comCont <= 0))
                            {
                                /* Exit if we're at a c++ comment */
                                if((ch == '/') && (lc == '/'))
                                {
                                    ss-- ;
                                    break ;
                                }
                                /* Exit if we're in a hash define and we're at a '\' */
                                if((ch == '\\') && (lc != '\\') && (lc != '\''))
                                {
                                    /* must do a quick check to see if we're not in a comment */
                                    uint8 *tss=ss ;
                                    lc = ch ;
                                    while((ch = *tss++) != '\0')
                                    {
                                        if((ch == '/') && (lc == '*'))
                                        {
                                            comCont-- ;
                                            ch = 0x01 ;
                                            break ;
                                        }
                                        else if((lc == '/') && ((ch == '*') || (ch == '/')))
                                            break ;
                                        lc = ch ;
                                    }
                                    if(ch != 0x01)
                                    {
                                        if(!((*flags) & MTNW_KNOWN))
                                            *flags |= MTNW_KNOWN ;
                                        else if((*flags) & MTNW_NOTINHASH)
                                        {
                                            /* part of a #define \ line, skip the line */ 
                                            *flags &= ~MTNW_SKIPHASH ;
                                            goto newline_skip ;
                                        }
                                        break ;
                                    }
                                    ss = tss ;
                                }
                            }
                            lc = ch ;
                        } while((ch = *ss++) != '\0') ;
                    }
                    
                    if(!((*flags) & MTNW_KNOWN))
                    {
                        *flags |= MTNW_KNOWN|MTNW_NOTINHASH ;
                        if(fss[-1] == '#')
                        {
                            ss = fss ;
                            goto hash_skip ;
                        }
                    }
                    if((*flags == (MTNW_NOTINHASH|MTNW_KNOWN)) && (comCont >= 0))
                    {
                        
                        /* If not in a hash and the current line has no
                         * mismatched close comment, we must check the
                         * previous line to see if it has a trailing '\'. If
                         * so then this line is part of a hash define, so we
                         * must skip to the line before the # */
                        uint8 tflags ;
                        long lineNo=curwp->line_no ;
                        
                        curwp->w_doto = 0 ;
                        tflags = *flags | MTNW_SKIPHASH ;
                        if(((ch=moveToNonWhite(forwFlag,&tflags)) != 0) &&
                           !(tflags & MTNW_SKIPHASH))
                            return ch ;
                        curwp->w_dotp = lp ;
                        curwp->line_no = lineNo ;
                    }
                    if((inq = ((size_t) ss - (size_t) lp->l_text) - 2) >= 0)
                    {
                        curwp->w_doto = inq ;
                        break ;
                    }
                }
            }
            else
                curwp->w_doto-- ;
            ch = lgetc(curwp->w_dotp,curwp->w_doto) ;
        }
        if(!isSpace(ch))
        {
            if(!((*flags) & (MTNW_INCOM|MTNW_SIMPLE)) && (ch == '/'))
            {
                if(forwFlag)
                {
                    register uint8 c2 = lgetc(curwp->w_dotp, curwp->w_doto+1) ;
                    if(c2 == '*')
                    {   /* c comment, go to the end of it */
                        if(findfence('*',forwFlag) != TRUE)
                            return 0 ;
                        curwp->w_doto++ ;
                        continue ;
                    }
                    else if(c2 == '/')
                    {   /* c++ comment, go to the end of the line */
                        curwp->w_doto = llength(curwp->w_dotp) ;
                        continue ;
                    }
                }
                else
                {
                    /* are we at the start of a comment?? if so skip it.
                     * One gotcha is a comment of the form / * / / / / / / / */
                    if((curwp->w_doto > 0) &&
                       (lgetc(curwp->w_dotp, curwp->w_doto-1) == '*') &&
                       ((curwp->w_doto == 1) ||
                        (lgetc(curwp->w_dotp, curwp->w_doto-2) != '/')))
                    {
                        /* must move back one into the comment */
                        curwp->w_doto-- ;
                        if(findfence('/',forwFlag) != TRUE)
                            return 0 ;
                        /* ZZZZ - note that at this point we should check for
                         * a # at the start of the line and do the right
                         * stuff
                         */
                        continue ;
                    }
                }
            }
            return ch ;
        }
    }
}

/* finds the matching given fence ch.
** changes the current position to it and returns true. 
** if fails then leaves the position in an undefined place and returns FALSE
*/

/* ch - fence type to find is " or ', where a \" must be ignored */
static int
findQuoteFence(uint8 qtype, uint8 forwFlag)
{
    register uint8 c ;  	/* current character in scan */
    LINE *comlp;	        /* comment line pointer */
    uint16 comoff;	        /* and offset */
    int32 comlno;               /* and line-no */
    int comStt=0;               /* and status */
    
    /* scan until we find it, or reach the end of file */
    for(;;)
    {
        /* backChar & forwChar goes 1 character past the end of the
         * line, therfore want to omit this character in the search.
         * Check for end of line, and go back another character if true.
         */
        if(forwFlag)
        {
            if(curwp->w_doto >= llength(curwp->w_dotp))
            {
                register LINE *lp ;
                register int32 ii=0 ;
                lp = curwp->w_dotp ;
                do {
                    if((lp = lforw(lp)) == curbp->b_linep)
                        return FALSE ;
                    ii++ ;
                } while(llength(lp) == 0) ;
                curwp->w_dotp = lp ;
                curwp->w_doto = 0 ;
                curwp->line_no += ii ;
            }
            else
                (curwp->w_doto)++ ;
        }
        else
        {
            if(curwp->w_doto == 0)
            {
                register LINE *lp ;
                register int32 ii=0 ;
                if(comStt > 0)
                    /* if we have found an open comment it is
                     * more likely that the quote is part of a comment
                     * than the open comment being part of a multi-
                     * line string, quit
                     */
                    break ;
                lp = curwp->w_dotp ;
                do {
                    if((lp = lback(lp)) == curbp->b_linep)
                        return FALSE ;
                    ii++ ;
                } while(llength(lp) == 0) ;
                curwp->w_dotp = lp ;
                curwp->w_doto = llength(lp) - 1 ;
                curwp->line_no -= ii ;
            }
            else
                (curwp->w_doto)-- ;
        }
        c=lgetc(curwp->w_dotp, curwp->w_doto) ;
        if(c == qtype)
        {
            /* must count the '\'s before the character to
             * check whether its the close or just quoted in
             * a string, e.g. "\\\"" etc.
             */
            register int32 ii=curwp->w_doto ;
            while((--ii > 0) && (lgetc(curwp->w_dotp,ii) == '\\'))
                ;
            /* if its an odd number then its a quote (note that ii is over decr)  */
            if((((int) curwp->w_doto) - ii) & 0x01)
                return TRUE ;
        }
        if(c == '*')
        {
            if(!comStt && !forwFlag && (lgetc(curwp->w_dotp, curwp->w_doto+1) == '/'))
                comStt = -1 ;
        }
        else if(c == '/')
        {
            if(!comStt && !forwFlag && (lgetc(curwp->w_dotp, curwp->w_doto+1) == '*'))
            {
                /* save the comment cursor position */
                comlp  = curwp->w_dotp ;
                comoff = curwp->w_doto+2 ;
                comlno = curwp->line_no ;
                comStt = 1 ;
            }
        }
        else if((c == '{') && (curwp->w_doto == 0))
            break ;
    }
    if(comStt <= 0)
        return FALSE ;
    curwp->w_dotp = comlp ;
    curwp->w_doto = comoff ;
    curwp->line_no = comlno ;
    return -1 ;
}

/* find the matching bracket, brace or comment:
 * Has 4 end points, leave cursor at end point and returns 1 of the following
 * -1    There is no matching bracket as this is inside quotes
 *  0    Failed. There is no match, error.
 *  1    Success.
 *  2    There is no matching bracket as this is inside comments
 */
/* ch - fence type to find ('*' indicates a comment start, '/' an end
 * movenext is the function used to move, sets direction.
 */
static int
findfence(uint8 ch, uint8 forwFlag)
{
    uint8 inCom ;
    register uint8 cc ;
    register int  inAps ;
    
    /* Separate hash case as we can really optimise this */
    if(ch == '#')
    {
        register LINE *lp ;
        register int32 ii=0 ;
        uint8 *ss ;
        lp = curwp->w_dotp ;
        if(forwFlag)
        {
            for(;;)
            {
                if((lp = lforw(lp)) == curbp->b_linep)
                    return FALSE ;
                ii++ ;
                ss=lp->l_text ;
                while(((cc = *ss++) != '\0') &&
                      ((cc == ' ') || (cc == '\t')))
                    ;
                if(cc == '#')
                {
                    curwp->w_dotp = lp ;
                    curwp->w_doto = ((size_t) ss - (size_t) lp->l_text) - 1 ;
                    curwp->line_no += ii ;
                    if((ss[0] == 'e') && ((ss[1] == 'l') || (ss[1] == 'n')))
                        break ;
                    else if((ss[0] == 'i') && (ss[1] == 'f'))
                    {
                        do {
                            if(findfence('#',forwFlag) != TRUE)
                                return FALSE ;
                            /* findfence can only succeed with a line containing
                             * #e[nl]
                             */
                            ss = curwp->w_dotp->l_text ;
                            while((cc = *ss++) != 'e')
                                ;
                        } while(*ss != 'n') ;
                    }
                    lp = curwp->w_dotp ;
                    ii = 0 ;
                }
            }
        }
        else
        {
            /* Moving backwards we only want to stop at the #if so we can
             * cycly round a #if, #else, #endif
             */
            for(;;)
            {
                if((lp = lback(lp)) == curbp->b_linep)
                    return FALSE ;
                ii++ ;
                ss=lp->l_text ;
                while(((cc = *ss++) != '\0') &&
                      ((cc == ' ') || (cc == '\t')))
                    ;
                if(cc == '#')
                {
                    curwp->w_dotp = lp ;
                    curwp->w_doto = ((size_t) ss - (size_t) lp->l_text) - 1 ;
                    curwp->line_no -= ii ;
                    if((ss[0] == 'i') && (ss[1] == 'f'))
                        break ;
                    else if((ss[0] == 'e') && (ss[1] == 'n') &&
                            (findfence('#',forwFlag) != TRUE))
                        return FALSE ;
                    lp = curwp->w_dotp ;
                    ii = 0 ;
                }
            }
        }
        return TRUE ;
    }
    /* scan until we find it, or reach the end of file */
    /* check the previous char, if its a ' then we could be in a quote */
    if((curwp->w_doto > 0) && (lgetc(curwp->w_dotp, curwp->w_doto-1) == '\''))
        inAps = -1 ;
    else
        inAps = 0 ;        
    if((ch == '*') || (ch == '/'))
    {
        inCom = MTNW_INCOM|MTNW_KNOWN ;
        if(lgetc(curwp->w_dotp, curwp->w_doto) == '/')
            curwp->w_doto += (forwFlag) ? 1:-1 ;
    }
    else if(!meModeTest(curbp->b_mode,MDCMOD))
        inCom = MTNW_SIMPLE|MTNW_KNOWN ;
    else
        inCom = 0 ;
    
    for(;;)
    {
        if((cc=moveToNonWhite(forwFlag,&inCom)) == 0)
            break ;
        if(cc == '/')
        {
            cc = lgetc(curwp->w_dotp, curwp->w_doto+1) ;
            if(cc != '*')
                continue ;
            if(ch == '/')
                return TRUE ;
            if(!forwFlag)
                /* we've found a double comment, or we're in a comment,
                 * assume the latter.
                 */
                return 2 ;
            if(findfence('*',forwFlag) != TRUE)
                break ;
        }
        else if(cc == '*')
        {
            if(lgetc(curwp->w_dotp, curwp->w_doto+1) != '/')
                continue ;
            if(ch == '*')
                return TRUE ;
            /* one possible gotcha here is a / * / / / / * / comment - check for it */
            if((ch == '/') && curwp->w_doto && (lgetc(curwp->w_dotp, curwp->w_doto-1) == '/'))
                continue ;
            if(forwFlag)
                /* we've found a double comment, try to report the error */
                break ;
            if(findfence('/',forwFlag) != TRUE)
                break ;
        }
        else if(ch == cc)
            return TRUE ;
        else if(!(inCom & MTNW_INCOM))
        {
            uint8 *ss ;
            
            /* If we're looking for a bracket then check for a bracket,
             * i.e. a ",',(,),[,],{,}
             */
            if((cc == '"') || (cc == '\''))
            {
                if(findQuoteFence(cc,forwFlag) == FALSE)
                    return -1 ;
            }
            else if((ss = meStrchr(fenceString+4,cc)) != NULL)
            {
                int ii=ss-fenceString ;
                
                /* check for an oposite direction bracket - i.e. a ')' when going forward
                 * If so return a failure
                 */
                if((ii & 1) != forwFlag)
                    return 0 ;
                /* This is a same direction bracket - i.e. a '(' when going forward
                 * If so find the other side of the bracket (using findfence)
                 */
                if((ii=findfence(fenceString[ii^1],forwFlag)) != TRUE)
                {
                    if(!inAps)
                        inAps = ii ;
                    break ;
                }
            }
        }
    }
    return inAps ;
}


/* move cursor to a matching fence */
int
gotoFence(int f, int n)
{
    register LINE  *oldlp;	/* original line pointer */
    register uint16 oldoff;	/* and offset */
    register long   oldlno;	/* and line-no */
    register long   oldtln;	/* The window top line-no */
    register int    ret;	/* return value */
    register uint8  ch; 	/* open fence */
    register uint8 *ss;         /* fenceSting pointer */
    
    /* save the original cursor position */
    oldlp  = curwp->w_dotp ;
    oldoff = curwp->w_doto ;
    oldlno = curwp->line_no ;
    oldtln = curwp->topLineNo ;
    ch = lgetc(oldlp, oldoff) ;
    
    /* Check the current char is a valid fence char, if not do nothing */
    if((ch != '\0') && ((ss=meStrchr(fenceString,ch)) != NULL))
    { 
        register uint8 forwFlag=1;  /* movement direction */
        
        if(ch == '*')
        {
            /* if its a comment fence, default is forward, but check for reverse */
            if(lgetc(oldlp, oldoff+1) == '/')
            {
                forwFlag = 0 ;
                ch = '/' ;
            }
        }
        else if(ch == '/')
        {
            if(oldoff && lgetc(oldlp, oldoff-1) == '*')
            {
                /* must move back into the comment as well, or we will end up in the wrong place */
                forwFlag = 0 ;
                curwp->w_doto-- ;
            }
            else
                ch = '*' ;
        }
        else if(ch == '#')
        {
            /* this is a #if, #elif, #else, #endif fence
             * workout which and the direction
             */
            if(lgetc(oldlp, oldoff+1) == 'e')
            {
                if(lgetc(oldlp, oldoff+2) == 'n')
                    /* a #endif, go backwards */
                    forwFlag = 0 ;
                else if(lgetc(oldlp, oldoff+2) != 'l')
                    goto invalid_fence ;
            }
            else if((lgetc(oldlp, oldoff+1) != 'i') ||
                    (lgetc(oldlp, oldoff+2) != 'f'))
                goto invalid_fence ;
        }
        else
        {
            /* this is a simple bracket fence, i.e. (,),{,},[,]
             * the direction and close ch can be calculated
             */
            forwFlag = ss-fenceString ;
            ch = fenceString[forwFlag^1] ;
            forwFlag &= 1 ;
        }
        if((ret = findfence(ch,forwFlag)) == TRUE)
        {
            curwp->w_flag |= WFMOVEL;
            /* if 2nd bit not set then we want to stay here so simply
             * return at theis point
             */
            if((n & 2) == 0)
                return TRUE ;
            /* We just want to show the location so, update, delay
             * then move back
             */
            update(FALSE);
            TTsleep(matchlen,1) ;
            curwp->w_flag |= WFMOVEL;
        }
        /* restore the current position */
        curwp->w_dotp  = oldlp;
        curwp->w_doto  = oldoff;
        curwp->line_no = oldlno;
        curwp->topLineNo = oldtln ;
    }
    else
invalid_fence:
    ret = FALSE ;
    if((ret == FALSE) && (n & 1))
        TTbell();
    return (ret == TRUE) ;
}


static int
prevCToken(uint8 *token, int size)
{
    register int offset ;
    
    if(((offset = curwp->w_doto - size + 1) < 0) ||
       ((offset > 0) && !isSpace(curwp->w_dotp->l_text[offset-1])))
        return 0 ;
    return !meStrncmp(curwp->w_dotp->l_text+offset,token,size) ;
}

static int
getCoffset(int onBrace, int *inComment)
{
    LINE *oldlp;    	        /* original line pointer */
    uint8 cc ;
    uint8 mtnwFlag=0 ;
    int   normCont=1, brakCont=0, indent=0, gotsome=0 ;
    
    *inComment = 0 ;
    oldlp = curwp->w_dotp;
    curwp->w_doto = 0 ;
    /* scan until we find it, or reach the end of file */
    while(!indent)
    {
        switch((cc=moveToNonWhite(0,&mtnwFlag)))
        {
        case 0:
            if(brakCont < 0)
                return -brakCont ;
            else if(onBrace < 0)
                return statementIndent ;
            return 0 ;
            
        case ')':
            cc = '(' ;
            goto find_bracket_fence ;
        case ']':
            cc = '[' ;
            goto find_bracket_fence ;
        case '}':
            cc = '{' ;
find_bracket_fence:
            {
                int ret ;
                
                if((ret=findfence(cc,0)) <= 0)
                    return 0 ;
                if(ret == 2)
                {
                    /* in a comment */
                    *inComment = 1 ;
                    return getccol() ;
                }
                if(cc == '{')
                {
                    ret = getccol() ;
                    if(ret <= statementIndent+braceIndent)
                    {
                        if(brakCont < 0)
                            return -brakCont ;
                        else if(onBrace < 0)
                            return statementIndent ;
                        return 0 ;
                    }
                    if(!gotsome)
                        normCont = 0 ;
                    /*            indent = curwp->w_doto ; */
                }
                break ;
            }
        case '{':
            {
                uint16 off ;
                long lno ;
                LINE *lp ;
                int ii ;
                
                lp = curwp->w_dotp ;
                off = curwp->w_doto ;
                lno = curwp->line_no ;
                if((cc=moveToNonWhite(0,&mtnwFlag)) != 0)
                {
                    if(cc == ')')
                    {
			    int foundFence=-999 ;
			    if(curwp->w_dotp == lp)
			    {
				    if((foundFence=findfence('(',0)) == 1)
				    {
					    lp  = curwp->w_dotp ;
					    off = curwp->w_doto ;
					    lno = curwp->line_no ;
				    }
			    }
			    if(!brakCont)
			    {
				    /* have we found a switch? */
				    if(foundFence == -999) 
					    foundFence = findfence('(',0) ;
				    if((foundFence == 1) &&
				       (moveToNonWhite(0,&mtnwFlag) != 0) &&
				       prevCToken((uint8 *)"switch",6))
					    indent = -1 ;
			    }
                            curwp->w_dotp = lp ;
                            curwp->line_no = lno ;
                    }
                    else if(cc == '=')
                    {
                        normCont = 0 ;
                        curwp->w_dotp = lp ;
                        curwp->line_no = lno ;
                    }
                    else
                    {
                        curwp->w_doto = 0 ;
                        if((cc = gotoFrstNonWhite()) == 'e')
                        {
                            if(!meStrncmp(curwp->w_dotp->l_text+curwp->w_doto,"extern \"C\"",10))
                                indent = 2 ;
                            else if(!meStrncmp(curwp->w_dotp->l_text+curwp->w_doto,"enum",4))
                            {
                                indent = 3 ;
                                normCont = 0 ;
                            }
                        }
                        else if((cc == 't') &&
                                !meStrncmp(curwp->w_dotp->l_text+curwp->w_doto,"typedef",7) &&
                                ((curwp->w_doto+=7),((cc = gotoFrstNonWhite()) == 'e')) &&
                                !meStrncmp(curwp->w_dotp->l_text+curwp->w_doto,"enum",4))
                        {
                            indent = 3 ;
                            normCont = 0 ;
                        }
                        if(indent)
                            off = curwp->w_doto ;
                        else
                        {
                            curwp->w_dotp = lp ;
                            curwp->line_no = lno ;
                        }
                    }
                }
	        else
		{
                    curwp->w_dotp = lp ;
                    curwp->line_no = lno ;
		}
                curwp->w_doto = 0 ;
                cc = gotoFrstNonWhite() ;
                ii = getccol() ;
                if(!gotsome)
                    normCont = 0 ;
                
                if(!brakCont)
                {
                    if(indent == 2)
                        brakCont = ii ;
		    else
		    {
                        if((cc == '{') || (cc == '}'))
                            brakCont = ii-braceIndent ;
                        else
                            brakCont = ii+statementIndent ;
                        if(!onBrace && indent == -1)
                            brakCont += switchIndent ;
                        
                        if(brakCont < 0)
                            brakCont = 0 ;
		    }
                }
                curwp->w_doto = off ;
                if(ii <= statementIndent+braceIndent)
                    indent = 1 ;
		else if(indent < 0)
                    indent = 0 ;
                break ;
            }
        case '#':
            if(mtnwFlag == MTNW_KNOWN)
                indent = 1 ;
            break ;
        case '/':
            if(lgetc(curwp->w_dotp, curwp->w_doto+1) == '*')
            {
                /* in a comment */
                *inComment = 1 ;
                return getccol() ;
            }
            break ;
        case '(':
        case '[':
            if(brakCont >= 0)
            {
                uint8 ch ;
                /* ignore tabs here cos they will screw up anyway */
                while(((ch=lgetc(curwp->w_dotp,++(curwp->w_doto))) == ' ') || (ch == '\t'))
                    ;
		brakCont = getccol() ;
                if((continueMax > 0) && (brakCont > continueMax))
		{
                    uint16 off ;
                    int ii ;

                    off = curwp->w_doto ;
                    curwp->w_doto = 0 ;
                    gotoFrstNonWhite() ;
                    ii = getccol() + continueMax ;
                    if(ii < brakCont)
                        brakCont = ii ;
                    curwp->w_doto = off ;
		}
                brakCont = 0-brakCont ;
            }
            break ;
        case ';' :
        case ':' :
            if(!gotsome)
                normCont = 0 ;
            break ;
        case '\'':
        case '"' :
            /* only return if findQuoteFence returns FALSE, returns
             * -1 if we could be in a comment */
            if(findQuoteFence(cc,0) == FALSE)
                return 0 ;
            break ;
        default:
            if(onBrace > 1)
            {
                if(cc == 'e')
                {
                    if(((curwp->w_doto == 0) || isSpace(lgetc(curwp->w_dotp, curwp->w_doto-1))) &&
                       !meStrncmp(curwp->w_dotp->l_text+curwp->w_doto,"else",4) &&
                       (isSpace(lgetc(curwp->w_dotp, curwp->w_doto+4)) ||
                        (lgetc(curwp->w_dotp, curwp->w_doto+2) == '{')))
                        onBrace++ ;
                }
                else if(cc == 'i')
                {
                    if(((curwp->w_doto == 0) || isSpace(lgetc(curwp->w_dotp, curwp->w_doto-1))) &&
                       !meStrncmp(curwp->w_dotp->l_text+curwp->w_doto,"if",2) &&
                       (isSpace(lgetc(curwp->w_dotp, curwp->w_doto+2)) ||
                        (lgetc(curwp->w_dotp, curwp->w_doto+2) == '(')) &&
                       (--onBrace == 1))
                    {
                        uint16 odoto=curwp->w_doto ;
                        curwp->w_doto = 0 ;
                        gotoFrstNonWhite() ;
                        brakCont = getccol() ;
			    curwp->w_doto = odoto ;
			    onBrace = 0 ;
                    }
                }
            }
        }
        if(!gotsome)
        {
            if((cc == '=') && onBrace < 0)
                normCont = 0 ;
            gotsome = 1 ;
        }
    }
    if(brakCont < 0)
        return -brakCont ;
    if(normCont)
    {
        curwp->w_dotp =	oldlp ;
        curwp->w_doto = 0 ;
        if(moveToNonWhite(0,&mtnwFlag) == ')')
        {
            if(findfence('(',0) <= 0)
                return 0 ;
            moveToNonWhite(0,&mtnwFlag) ;
            if(prevCToken((uint8 *)"if",2) || prevCToken((uint8 *)"for",3) ||
               prevCToken((uint8 *)"while",5) || prevCToken((uint8 *)"switch",6))
            {
                curwp->w_doto = 0 ;
                gotoFrstNonWhite() ;
                brakCont = getccol()+statementIndent ;
            }
            else
            {
                if(onBrace < 0)
                    brakCont += statementIndent ;
                else if(brakCont)
                    brakCont += continueIndent ;
            }
        }
        else if(!prevCToken((uint8 *)"do",2) && !prevCToken((uint8 *)"else",4))
        {
            if(onBrace < 0)
                brakCont += statementIndent ;
            else if(brakCont)
                brakCont += continueIndent ;
        }
        else
        {
            curwp->w_doto = 0 ;
            gotoFrstNonWhite() ;
            brakCont = getccol()+statementIndent ;
        }
    }
    else if(onBrace < 0)
        brakCont += statementIndent ;
    return brakCont ;
}


/*
** do C indent, returns -ve if failed (god knows how)
** the indent on success.
*/
int
doCindent(int *inComment)
{
    LINE *oldlp;    	        /* original line pointer */
    int   ind=1, curOff, curInd, curPos, cc ; 
    int   addInd=0, comInd=3, onBrace=0 ;
    long  lineno ;
    
    /* save the original cursor position */
    oldlp = curwp->w_dotp ;
    lineno = curwp->line_no ;
    curPos = curwp->w_doto ;
    curwp->w_doto = 0 ;
    cc = gotoFrstNonWhite() ;
    curOff = curwp->w_doto ;
    curInd = getccol() ;
    /* change the current position to the indent position if to the left */
    if(curPos < curOff)
        curPos = curOff ;
    if(isAlpha(cc))
    {
        if((cc == 'c') &&
           !meStrncmp(curwp->w_dotp->l_text+curwp->w_doto,"case",4) &&
           !isAlpha(curwp->w_dotp->l_text[curwp->w_doto+4]))
            addInd += caseIndent ;
        else if((cc == 'e') &&
                !meStrncmp(curwp->w_dotp->l_text+curwp->w_doto,"else",4) &&
                (!isAlpha(curwp->w_dotp->l_text[curwp->w_doto+4]) ||
                 (curwp->w_dotp->l_text[curwp->w_doto+4] == '{')))
            onBrace = 2 ;
        else if((cc == 'd') &&
                !meStrncmp(curwp->w_dotp->l_text+curwp->w_doto,"default",7) &&
                !isAlpha(curwp->w_dotp->l_text[curwp->w_doto+7]))
            addInd += caseIndent ;
        else
        {
            /* check to see if its a lable */
            int ii ;
            ii = curOff ;
            do
                cc = lgetc(curwp->w_dotp,++ii) ;
            while(isAlphaNum(cc) || (cc == '_')) ;
            
            while((cc == ' ') || (cc == '\t'))
                cc = lgetc(curwp->w_dotp,++ii) ;
            
            if(cc == ':')
            {
                cc = lgetc(curwp->w_dotp,++ii) ;
                if(isSpace(cc))
                    ind = 0 ;
            }
        }
    }
    else if(cc == '#')
    {
        *inComment = 0 ;
        ind = 0 ;
    }
    else if(cc == '*')
    {
        comInd = 1 ;
        if((cc=lgetc(curwp->w_dotp,curwp->w_doto+1)) == '*')
        {
            if(lgetc(curwp->w_dotp,curwp->w_doto+2) == '*')
                goto use_prev_line ;
            comInd = 0 ;
        }
        else if(cc == '/')
        {
use_prev_line:
            curwp->w_dotp = lback(curwp->w_dotp) ;
            curwp->w_doto = 0 ;
            if(((cc=gotoFrstNonWhite()) == '*') &&
               (lgetc(curwp->w_dotp,curwp->w_doto+1) == '*'))
            {
                if(lgetc(curwp->w_dotp,curwp->w_doto+2) == '*')
                    goto use_contcomm ;
                comInd = 0 ;
            }
            else if(cc == '/')
            {
use_contcomm:
                if((lgetc(curwp->w_dotp,curwp->w_doto+1) == '*') &&
                   (commentCont != NULL) &&
                   (commentCont[0] == '*') &&
                   (commentCont[1] == '*'))
                    comInd = 0 ;
            }
            curwp->w_dotp = oldlp ;
            curwp->w_doto = curOff ;
        }
    }
    else if(cc == '{')
    {
        addInd += braceIndent ;
        onBrace = -1 ;
    }
    else if(cc == '}')
    {
	    addInd += braceIndent ;
	    onBrace = 1 ;
    }
    if(ind)
    {
        ind = getCoffset(onBrace,inComment) ;
        if(*inComment)
            ind += comInd ;
        else if((ind += addInd) < 0)
            ind = 0 ;
    }
    
    curwp->w_dotp = oldlp ;
    curwp->line_no = lineno ;
    curwp->w_doto = curPos ;
    if(curInd == ind)
        return TRUE ;
    return meLineSetIndent(curOff,ind) ;
}

#endif /* CFENCE */

int
insString(int f, int n)	/* ask for and insert a string into the current
                           buffer at the current point */
{
    register uint8 cc, *tp;	        /* pointer into string to add */
    uint8 tstring[MAXBUF];              /* string to add */
    register int status;		/* status return code */
    register int count=0;		/* char insert count */
    
    /* ask for string to insert */
    if((status=mlreply((uint8 *)"String", 0, 0,tstring, MAXBUF)) != TRUE)
        return status ;
    if((status=bchange()) != TRUE)               /* Check we can change the buffer */
        return status ;
    
    /* insert it */
    for(; n>0 ; n--)
    {
        tp = &tstring[0];
        while ((cc=*tp++) != '\0')
        {
            if(cc == 0x0a)
                status = lnewline();
            else
                status = linsert(1,cc);
            if(status != TRUE)
                return status ;
            count++ ;
        }
    }
#if MEUNDO
    meUndoAddInsChars(count) ;
#endif
    return TRUE ;
}


int
alphaMarkGet(BUFFER *bp, uint16 name)
{
    meAMARK *p = bp->b_amark;
    
    while(p != NULL)
    {
        if(p->name == name)
        {
            /* found the mark - do the buisness */
            LINE *lp ;
try_again:
            lp = bp->b_linep ;
            bp->b_dotp = p->line ;
            if((bp->b_doto = p->offs) > llength(p->line))
                bp->b_doto = llength(p->line) ;
            bp->line_no = bp->elineno ;
            do {
                if(lp == bp->b_dotp)
                    return TRUE ;
                lp = lback(lp) ;
            } while ((--bp->line_no) >= 0) ;
            
            /* Okay, we failed to find the mark so only chance now is
             * the mark is in a narrow
             */
#if NARROW
            {
                meNARROW *nrrw ;
                
                nrrw=curbp->narrow ;
                while(nrrw != NULL)
                {
                    lp = nrrw->slp ;
                    for (;;)
                    {
                        if(lp == bp->b_dotp)
                        {
                            /* We've found the mark in this narrow, remove the
                             * narrow and try again */
                            removeNarrow(bp,nrrw,0) ;
                            goto try_again ;
                        }
                        if(lp == nrrw->elp)
                            break ;
                        lp = lforw(lp) ;
                    }
                    nrrw = nrrw->next ;
                }
            }
#endif
        }
        p = p->next;
    }
    /* Failed to find mark - set to the end of file */
    bp->line_no = bp->elineno ;
    bp->b_dotp = bp->b_linep ;
    bp->b_doto = 0 ;
    return FALSE ;
}

int
alphaMarkSet(BUFFER *bp, uint16 name, LINE *lp,
             uint16 off, int silent)
{
    meAMARK *p = bp->b_amark;
    
    while((p != NULL) && (p->name != name))
        p = p->next;
    
    if(p == NULL)
    {
        if((p = (meAMARK*) meMalloc(sizeof(meAMARK))) == NULL)
            return FALSE ;
        p->next = bp->b_amark ;
        bp->b_amark = p ;
    }
    else if(!silent)
        mlwrite(MWCLEXEC,(uint8 *)"[overwriting existing mark]");
    
    p->name = name ;
    p->line = lp ;
    p->offs = off ;
    lp->l_flag |= LNMARK ;		/* mark the line as marked */
    return TRUE ;
}

int
setAlphaMark(int f, int n)
{
    /*
     * Place a mark at the current offset within the buffer. This involves
     * searching down the mark list and tacking a new mark list element on
     * the end.
     */
    int   cc ;
    
    if((cc = mlCharReply((uint8 *)"Place mark: ",mlCR_QUIT_ON_USER,NULL,NULL)) == -2)
        cc = mlCharReply((uint8 *)"Place mark: ",mlCR_ALPHANUM_CHAR,NULL,NULL) ;
    
    if(cc < 0)
        return ctrlg(FALSE,1) ;
    
    return alphaMarkSet(curbp,(uint16) cc,curwp->w_dotp,curwp->w_doto,0) ;
}

int
gotoAlphaMark(void)
{
    /*
     * Return to an alphabetic mark (ie make the current line and the
     * current offset equal to the line and offset stored in the mark
     * element with the required name).
     */
    int	   cc ;
    
    if((cc = mlCharReply((uint8 *)"Goto mark: ",mlCR_QUIT_ON_USER,NULL,NULL)) == -2)
        cc = mlCharReply((uint8 *)"Goto mark: ",mlCR_ALPHANUM_CHAR,NULL,NULL) ;
    
    if(cc < 0)
        return ctrlg(FALSE,1) ;
    
    if(alphaMarkGet(curbp,(uint16) cc) != TRUE)
    {
        meAMARK *p = curbp->b_amark;
        uint8    allmarks[54];		/* record of the marks	*/
        int      ii = 0;
        
        if(p == NULL)
            return mlwrite(MWABORT|MWCLEXEC,(uint8 *)"[No marks in buffer]");
        
        while(p != NULL)
        {
            if(p->name < 128)
            {
                allmarks[ii++] = (uint8) p->name;
                allmarks[ii++] = ' ' ;
            }
            p = p->next;
        }
        allmarks[ii] = '\0';
        
        return mlwrite(MWABORT|MWCLEXEC,(uint8 *)"[Marks in buffer: %s]", allmarks);
    }
    
    /* do the buisness */
    curwp->w_dotp = curbp->b_dotp ;
    curwp->w_doto = curbp->b_doto ;
    curwp->line_no = curbp->line_no ;
    curwp->w_flag |= WFMOVEL ;
    
    return TRUE ;
}


#ifdef _SINGLE_CASE
/* make string lower case
** What can I say - its simple.
*/
void
makestrlow(uint8 *str)
{
    register uint8 cc, *p=str ;
    
    while((cc=*p) != '\0')
        *p++ = toLower(cc) ;
}
#endif

int
insFileName(int f, int n)
{
    register uint8 *p, cc ;
    register int s, count=0 ;
    
    if((n <= 0) || (curbp->b_fname == NULL))
        return TRUE ;
    if((s=bchange()) == TRUE)               /* Check we can change the buffer */
    {
        while(n--)
        {
            p=curbp->b_fname ;
            while(((cc = *p++) != 0) && ((s = linsert(1,cc)) == TRUE))
                count++ ;
        }
#if MEUNDO
        meUndoAddInsChars(count) ;
#endif
    }
    return s ;
}

/* cmpBuffers; Compare buffers against each other. In no argument is specified
 * then perform a white space insensitive comparison */
int
cmpBuffers(int f, int n)
{
    register WINDOW *wp ;
    register uint8 cc;
    if (n == 0)
    {
        /* Exact match - white space is matched. */
        for(;;)
        {
            wp = wheadp ;
            cc = getCurChar(wp) ;
            while((wp = wp->w_wndp) != NULL)
                if(getCurChar(wp) != cc)
                    return FALSE ;
            wp = wheadp ;
            cc = WforwChar(wp,1) ;
            while((wp = wp->w_wndp) != NULL)
                if(WforwChar(wp,1) != cc)
                    return FALSE ;
            if(cc == FALSE)
                break ;
        }
    }
    else
    {
        /* Ignore white space */
        for(;;)
        {
            uint8 moreData;
            uint8 winData;
            uint8 tmpc;
            
            wp = wheadp ;
            cc = getCurChar(wp);
            if (isSpace (cc))
                cc = ' ';
            
            /* Check the current character */
            while((wp = wp->w_wndp) != NULL)
            {
                if((tmpc = getCurChar(wp)) != cc)
                {
                    if (!isSpace(tmpc) && (tmpc != ' '))
                        return FALSE ;
                }
            }
            
            /* Back to the start of the buffers. Advance to the next character
             * in the buffer. if the current character is a space then advance
             * to the next non-white character in the buffer. */
            wp = wheadp ;
            if (((moreData = WforwChar(wp,1)) == TRUE) && (cc == ' '))
            {
                do
                {
                    tmpc = getCurChar(wp);
                    if (!isSpace (tmpc) && (tmpc != ' '))
                        break;
                }
                while ((moreData = WforwChar(wp,1)) == TRUE);
            }
            
            /* Advance the rest of the windows. */
            while((wp = wp->w_wndp) != NULL)
            {
                if (((winData = WforwChar(wp,1)) == TRUE) && (cc == ' '))
                {
                    do
                    {
                        tmpc = getCurChar(wp);
                        if (!isSpace (tmpc) && (tmpc != ' '))
                            break;
                    }
                    while ((winData = WforwChar(wp,1)) == TRUE);
                }
                
                if(winData != moreData)
                    return FALSE ;
            }
            if(moreData == FALSE)
                break ;
        }
    }
    return TRUE ;
}

int
createCallback(int f, int n)
{
    struct meTimeval tp ;
    meMACRO *mac ;
    uint8 buf[MAXBUF] ;
    
    if((mac=userGetMacro(buf,MAXBUF)) == NULL)
        return FALSE ;
    if(n < 0)
        mac->callback = -1 ;
    else
    {
        gettimeofday(&tp,NULL) ;
        mac->callback = ((tp.tv_sec-startTime)*1000) + (tp.tv_usec/1000) + n ;
        if(!isTimerExpired(CALLB_TIMER_ID) &&
           (!isTimerSet(CALLB_TIMER_ID) || 
            (meTimerTime[CALLB_TIMER_ID] > mac->callback)))
            timerSet(CALLB_TIMER_ID,mac->callback,n) ;
    }
    return TRUE ;
}

#if MOUSE
/*
 * Mouse intersection values.
 * 
 * We set external mouse_pos in here, after working out the intersection
 * with the windows. 
 */

#define MITEXT      0                   /* Intersection in the text area */
#define MIMESSAGE   1                   /* Intersection on message line  */
#define MIMODE      2                   /* Intersection on the mode line */
#define MIDIVIDER   3                   /* Intersection on the divider   */
#define MISBSPLIT   4                   /* Intersection with the split   */
#define MISBUP      5                   /* Intersection with up arrow    */
#define MISBUSHAFT  6                   /* Intersection with upper shaft */
#define MISBBOX     7                   /* Intersection with the box     */
#define MISBDSHAFT  8                   /* Intersection with lower shaft */
#define MISBDARROW  9                   /* Intersection with lower arrow */
#define MISBML      10                  /* Intersection on window corner */
#define MIMENU      11                  /* Intersection with the menu    */
#define MICOLUNM2   16                  /* Intersection with 2nd column  */
#define MIREGION    32                  /* Intersection with region      */
#define MIERROR     255                 /* Error value                   */

int
setCursorToMouse(int f, int n)
{
    register WINDOW *wp ;
    register LINE   *ln ;
    int      row, col, ii ;
    int      odoto;                     /* Old doto */
    LINE    *odotp;                     /* Old dotp */ 
    
    /* Handle the message line */
    row = mouse_Y ;
    if(row >= TTnrow)
    {
        mouse_pos = MIMESSAGE;          /* On the message line */
        return TRUE ;
    }
    
    /* Handle the menu line */
    if(row < TTsrow)
    {
        mouse_pos = MIMENU;
        return TRUE ;
    }
    
    /* Locate the window associated with the mouse position. */
    col = mouse_X ;
    for (wp = wheadp; wp != NULL; wp = wp->w_wndp)
    {
        if (wp->firstRow > row)
            continue;                   /* Above window */
        if ((wp->firstRow + wp->numRows) <= row)
            continue;                   /* Below window */
        if (wp->firstCol > col)
            continue;                   /* Left of window */
        if ((wp->firstCol + wp->numCols) > col)
            break;                      /* In the window !! */
    }
    if (wp == NULL)
    {
        mouse_pos = MIERROR;            /* Set bad status */
        return (FALSE);                 /* Fail */
    }
    
    if(wp != curwp)                     /* Current window ?? */
    {
        if (f == TRUE)
        {
            mouse_pos = MIERROR;        /* Set bad status */
            return (TRUE);
        }
        makeCurWind(wp) ;               /* No - make it so */
    }
    
    /* Handle the divider */
    col -= wp->firstCol ;                 /* Normalise the column count */
    if (col >= wp->numTxtCols)
    {
        /* Iterate down the scroll positions */
        for (ii = 0; ii <= (WCVSBML-WCVSBSPLIT); ii++)
            if (row < wp->w_sbpos[ii])
                break;
        
        if (col > wp->numTxtCols) /* Report if in the second column */
            mouse_pos = MICOLUNM2;
        else
            mouse_pos = 0;
        /* Translate to return code */
        if (wp->w_mode & WMSCROL)       /* Scroll bar enabled ?? */
            mouse_pos |= MISBSPLIT+ii;  /* Yes - report scroll bar component */
        else if (ii>=(WCVSBML-WCVSBSPLIT))/* On the corner ?? */
            mouse_pos |= MISBML;        /* Yes - report corner */
        else
            mouse_pos |= MIDIVIDER;     /* Report divider */
        return (TRUE);                  /* Done */
    }
    
    /* Handle the mode line */
    row -= wp->firstRow ;                 /* Normalise the row count */
    if (row >= wp->numTxtRows)
    {
        mouse_pos = MIMODE;             /* On the mode line */
        return (TRUE);                  /* And done */
    }
    
    /* Must be in the text area !!
     * Work out the new postion of the cursor in the buffer.
     * Save the old position so that we can later check if we
     * have moved. */
    odoto = wp->w_doto;
    odotp = wp->w_dotp;
    
    mouse_pos = MITEXT;
    ln = wp->w_dotp ;
    ii = row - (wp->line_no - wp->topLineNo) ;
    if(ii > 0)
    {
        if(wp->line_no+ii > wp->w_bufp->elineno)
            ii = wp->w_bufp->elineno - wp->line_no ;
        wp->line_no += ii ;
        while(ii--)
            ln = lforw(ln) ;
    }
    else if(ii < 0)
    {
        wp->line_no += ii ;
        while(ii++)
            ln = lback(ln) ;
    }
    
    /* Take into account the fact that the buffer might be horizontally
     * scrolled. Change the base column position to take this into
     * account. Note that this is not perfect since if you click on the
     * dollars you get hyperspaed backwards or forwards - but I am not
     * going to try and work this one out now !! - Jon Green 01/03/97 */
    if (wp->w_dotp != ln)
        wp->w_scscroll = wp->w_sscroll ;      /* Set the line scroll to the window offset */
    
    /* Compute the new position */
    wp->w_dotp = ln ;
    col += wp->w_scscroll ;
    setcwcol(col) ;
    
    /* If we have moved then set Window flag to indicate that we have
     * changed cursor position.
     * Check the line first as WFMOVEL does the column as well
     */
    if(wp->w_dotp != odotp)
        wp->w_flag |= WFMOVEL ;               /* Moved from line to line */
    else if(wp->w_doto != odoto)
        wp->w_flag |= WFMOVEC ;               /* Moved from col to col   */
    
    /* Determine if we are in the selection region. If so then set
     * $mouse_pos to reflect the fact that we are in a region. */
    if ((curbp == wp->w_bufp) && (selhilight.flags & SELHIL_ACTIVE))
    {
        if ((selhilight.dlineno > selhilight.mlineno) ||
            ((selhilight.dlineno == selhilight.mlineno) && 
             (selhilight.dlineoff>=selhilight.mlineoff)))
        {
            /* Point is below mark */
            if (((wp->line_no > selhilight.mlineno) ||
                 ((wp->line_no == selhilight.mlineno) &&
                  (wp->w_doto >= selhilight.mlineoff))) &&
                ((wp->line_no < selhilight.dlineno) ||
                 ((wp->line_no == selhilight.dlineno) &&
                  (wp->w_doto <= selhilight.dlineoff))))
                mouse_pos |= MIREGION;
        }
        else
        {
            /* Point is above mark */
            if (((wp->line_no > selhilight.dlineno) ||
                 ((wp->line_no == selhilight.dlineno) &&
                  (wp->w_doto >= selhilight.dlineoff))) &&
                ((wp->line_no < selhilight.mlineno) ||
                 ((wp->line_no == selhilight.mlineno) &&
                  (wp->w_doto <= selhilight.mlineoff))))
                mouse_pos |= MIREGION;
        }
    }
    return TRUE ;
}
#else
int
setCursorToMouse(int f, int n)
{
	return ABORT ;
}
#endif
