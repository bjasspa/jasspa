/*****************************************************************************
 *
 *	Title:		%M%
 *
 *	Synopsis:	Word processing routines
 *
 ******************************************************************************
 *
 *	Filename:		%P%
 *
 *	Author:			Danial Lawrence
 *
 *	Creation Date:		10/05/91 08:27		<010807.0804>
 *
 *	Modification date:	%G% : %U%
 *
 *	Current rev:		%I%
 *
 *	Special Comments:	
 *
 *	Contents Description:	The routines in this file implement commands
 *				that work word or a paragraph at a time.  There
 *				are all sorts of word mode commands.  If I
 *				do any sentence mode commands, they are likely
 *				to be put in this file. 
 *
 * Jon Green	13th July 1990.
 * Allowed CMODE & INDENT with WRAP. In 'C' mode will perform the auto
 * indentation when wrap is performed.
 * 
 * 
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

/**************************************************************************
* NOTICE; Jon; 99/07/09                                                   *
*                                                                         *
* I have just added the new Anchor routines in here to preserve the       *
* character position of point and mark on fill paragraph. This was a      *
* fairly quick modification that disturbed as little of the code as       *
* possible, thereby maintaining the integrity of the release.             *
*                                                                         *
* The anchor routines are written ready for export because I think that   *
* they might be useful, although they have not actually been exported yet *
* because with misuse they will crash editor. So I'm in two minds about   *
* this.                                                                   *
*                                                                         *
* The fill paragraph is now in need of a severe overhaul to reduce the    *
* amount of work that it performs. It currently performs a destructive    *
* justification process which is deeply unpleasant. In addition it must   *
* scan through the source copying it multiple times - very un-nice.       *
*                                                                         *
* I have started to prototype a replacement which is not ready yet. This  *
* will overcome all of the above. In addition I want to add a new Flow    *
* mode, this will allow MicroEmacs to act more like a word processor and  *
* throw words during the editing process onto the next line, and then     *
* automatically re-format the remaining part of the paragraph. Obviously  *
* handling left/right/centre correctly.                                   *
*                                                                         *
* On top of all of that. I want to change the paragraph recognition       *
* markers to be a buffer dependent regexp, thereby allowing the different *
* paragraph types to be recognised and handled correctly. And finally,    *
* allow for text leaders (i.e. typically programming language comment     *
* delimiters) and correctly handle text wrapping with leader insertion.   *
*                                                                         *
* One could argue that some of the above belongs in ifill-paragraph       *
* (macro wrapper around fill-paragraph). However for smatter flow control *
* I do not think that this is the case and would like to pursue a generic *
* method that handles a multitude of cases, given the minimal amount of   *
* setup information in the buffer                                         *
*                                                                         *
* All of the above is possible, it just takes a little time for some of   *
* these things to come together - a commodity I've not got much of at the *
* moment.                                                                 *
**************************************************************************/

/*---	Include definitions */

#define	__WORDC			/* Define filename */

/*---	Include files */

#include "emain.h"

/*---	Local macro definitions */

/*
 * Move the cursor backward by "n" words. All of the details of motion are
 * performed by the "backChar" and "forwChar" routines. Error if you try to
 * move beyond the buffers.
 */
int
backWord(int f, int n)
{
    if (n < 0)
        return (forwWord(f, -n));
    if (WbackChar(curwp, 1) == FALSE)
        return (FALSE);
    while (n--)
    {
        while (inWord() == FALSE)
        {
            if (WbackChar(curwp, 1) == FALSE)
                return (FALSE);
        }
        while (inWord() != FALSE)
        {
            if (WbackChar(curwp, 1) == FALSE)
                /* We can't move back any more cos we're at the start,
                 * BUT as we have moved and we are in the buffers first word, 
                 * we should succeed */
                return TRUE ;
        }
    }
    return (WforwChar(curwp, 1));
}

/*
 * Move the cursor forward by the specified number of words. All of the motion
 * is done by "forwChar". Error if you try and move beyond the buffer's end.
 */
int
forwWord(int f, int n)
{
    if (n < 0)
        return (backWord(f, -n));
    while (n--)
    {
        while (inWord() == FALSE)
        {
            if (WforwChar(curwp, 1) == FALSE)
                return (FALSE);
        }
        while (inWord() != FALSE)
        {
            if (WforwChar(curwp, 1) == FALSE)
                return (FALSE);
        }
    }
    return(TRUE);
}

/*
 * Move the cursor forward by the specified number of words. As you move,
 * convert any characters to upper case. Error if you try and move beyond the
 * end of the buffer. Bound to "M-U".
 */
int
upperWord(int f, int n)
{
    register int    c;
    
    if (n < 0)
        return (FALSE);
    if((c=bchange()) != TRUE)               /* Check we can change the buffer */
        return c ;
    while (n--)
    {
        while (inWord() == FALSE) 
        {
            if(WforwChar(curwp, 1) == FALSE)
                return (FALSE);
        }
        while (inWord() != FALSE)
        {
            c = lgetc(curwp->w_dotp, curwp->w_doto);
            if (isLower(c))
            {
                lchange(WFMAIN);
#if MEUNDO
                meUndoAddRepChar() ;
#endif
                c = toggleCase(c) ;
                lputc(curwp->w_dotp, curwp->w_doto, c);
            }
            if (WforwChar(curwp, 1) == FALSE)
                return (FALSE);
        }
    }
    return (TRUE);
}

/*
 * Move the cursor forward by the specified number of words. As you move
 * convert characters to lower case. Error if you try and move over the end of
 * the buffer. Bound to "M-L".
 */
int
lowerWord(int f, int n)
{
    register int    c;
    
    if (n < 0)
        return (FALSE);
    if((c=bchange()) != TRUE)               /* Check we can change the buffer */
        return c ;
    while (n--)
    {
        while (inWord() == FALSE) 
        {
            if (WforwChar(curwp, 1) == FALSE)
                return (FALSE);
        }
        while (inWord() != FALSE)
        {
            c = lgetc(curwp->w_dotp, curwp->w_doto);
            if (isUpper(c))
            {
                lchange(WFMAIN);
#if MEUNDO
                meUndoAddRepChar() ;
#endif
                c = toggleCase(c) ;
                lputc(curwp->w_dotp, curwp->w_doto, c);
            }
            if (WforwChar(curwp, 1) == FALSE)
                return (FALSE);
        }
    }
    return (TRUE);
}

/*
 * Move the cursor forward by the specified number of words. As you move
 * convert the first character of the word to upper case, and subsequent
 * characters to lower case. Error if you try and move past the end of the
 * buffer. Bound to "M-C".
 */
int
capWord(int f, int n)
{
    register int    c;
    
    if(n < 0)
        return (FALSE);
    if((c=bchange()) != TRUE)               /* Check we can change the buffer */
        return c ;
    while (n--)
    {
        while (inWord() == FALSE)
        {
            if (WforwChar(curwp, 1) == FALSE)
                return (FALSE);
        }
        if (inWord() != FALSE) {
            c = lgetc(curwp->w_dotp, curwp->w_doto);
            if (isLower(c))
            {
                lchange(WFMAIN);
#if MEUNDO
                meUndoAddRepChar() ;
#endif
                c = toggleCase(c) ;
                lputc(curwp->w_dotp, curwp->w_doto,c);
            }
            if (WforwChar(curwp, 1) == FALSE)
                return (FALSE);
            while (inWord() != FALSE)
            {
                c = lgetc(curwp->w_dotp, curwp->w_doto);
                if (isUpper(c))
                {
                    lchange(WFMAIN);
#if MEUNDO
                    meUndoAddRepChar() ;
#endif
                    c = toggleCase(c) ;
                    lputc(curwp->w_dotp, curwp->w_doto,c);
                }
                if (WforwChar(curwp, 1) == FALSE)
                    return (FALSE);
            }
        }
    }
    return (TRUE);
}

/*
 * Kill forward by "n" words. Remember the location of dot. Move forward by
 * the right number of words. Put dot back where it was and issue the kill
 * command for the right number of characters. Bound to "M-D".
 */
int
forwDelWord(int f, int n)
{
    register LINE   *dotp;
    register int    doto, delType ;
    int32           size, lineno ;
    
    if(n == 0)
        return TRUE ;
    if((f=(n < 0)))
        n = -n ;
    
    if(bchange() != TRUE)               /* Check we can change the buffer */
        return ABORT ;
    dotp   = curwp->w_dotp;
    doto   = curwp->w_doto;
    lineno = curwp->line_no ;
    size = 0;
    while (n--)
    {
        /* inWord returns 0 if not in word - BUT if in word its return
         * value is only defined as non-zero, so must test if 0
         */
        delType = (inWord() == 0) ;
        while ((inWord() == 0) == delType) 
        {
            if (WforwChar(curwp, 1) == FALSE)
                break ;
            ++size;
        }
     }
     curwp->w_dotp = dotp;
     curwp->w_doto = doto;
     curwp->line_no = lineno ;
     return (ldelete(size,(f) ? 2:3));
}

/*
 * Kill backwards by "n" words. Move backwards by the desired number of words,
 * counting the characters. When dot is finally moved to its resting place,
 * fire off the kill command. Bound to "M-Rubout" and to "M-Backspace".
 */
int
backDelWord(int f, int n)
{
    uint8 delType ;
    int32 size;
    int moveForw=TRUE ;
    
    if(n == 0)
        return TRUE ;
    if((f=(n < 0)))
        n = -n ;
    
    if(bchange() != TRUE)               /* Check we can change the buffer */
        return ABORT ;
    if (WbackChar(curwp, 1) == FALSE)
        return (FALSE);
    size = 0;
    while (n--)
    {
        /* inWord returns 0 if not in word - BUT if in word its return
         * value is only defined as non-zero, so must test if 0
         */
        delType = (inWord() == 0) ;
        while ((inWord() == 0) == delType) 
        {
            ++size;
            if ((moveForw=WbackChar(curwp, 1)) == FALSE)
                break ;
        }
    }
    if (moveForw && (WforwChar(curwp, 1) == FALSE))
        return (FALSE);
    return (ldelete(size,(f) ? 2:3));
}

#if WORDPRO
/* Definitions for the anchor types */
#define ANCHOR_CLEAR      0x00          /* Clear anchors */
#define ANCHOR_POINT      0x01          /* Remember point */
#define ANCHOR_MARK       0x02          /* Remember mark */
#define ANCHOR_START      0x04          /* Set start of region */
#define ANCHOR_END        0x08          /* Send end of region */
#define ANCHOR_COMPUTE    0x10          /* Compute position independant anchor */

/* Anchor flags */
#define ANCHFLG_INSPACE   0x00          /* Anchor is in a space */
#define ANCHFLG_INWORD    0x01          /* Anchor is in a word */
#define ANCHFLG_VALID     0x02          /* Anchor is valid */

/*---   Local Type definitions */
struct PosAnchor
{
    LINE *dotp;                         /* Line position */
    long words;                         /* Number of words */
    long offset;                        /* Sub-position in word */
    long dotl;                          /* Line number */
    unsigned short doto;                /* Line offset */
    unsigned char flags;                /* 0 = in space; 1 = in word; 2 = valid */
};

typedef struct SAnchor
{
    struct PosAnchor point;             /* Location of point */
    struct PosAnchor mark;              /* Location of mark */
    LINE *startp;                       /* Starting Line */
    unsigned short starto;              /* Starting Offset */
    long startl;                        /* Starting Line number */
    LINE *endp;                         /* Ending Line */
    unsigned short endo;                /* Ending Line */
    long endl;                          /* Ending Line number */
} Anchor;

static Anchor anchor;                   /* Anchor position */

/* setAnchor; Set the anchor position. The arguments are as follows:-
 * 
 * f=true;  n=ANCHOR_POINT    - Remember point
 * f=true;  n=ANCHOR_MARK     - Remember mark
 * f=true;  n=ANCHOR_START    - Set start
 * f=true;  n=ANCHOR_END      - Set end 
 * f=true;  n=ANCHOR_COMPUTE  - Compute anchor positions
 * f=false; n=ANCHOR_CLEAR    - Clear anchor positions 
 */
static int
setAnchor(int f, int n)
{
    /* By default remember point */
    if ((f == FALSE) || (n == ANCHOR_CLEAR))
    {
        anchor.point.dotp = NULL;
        anchor.mark.dotp = NULL;
        anchor.endp = NULL;
        anchor.startp = NULL;
        return TRUE;
    }
    
    /* Determine what we are doing */
    /* Save point */
    if (n & ANCHOR_POINT)
    {
        anchor.point.dotp = curwp->w_dotp;
        anchor.point.doto = curwp->w_doto;
        anchor.point.dotl = curwp->line_no;
    }
    /* Save mark */
    if (n & ANCHOR_MARK)
    {
        anchor.mark.dotp = curwp->w_markp;
        anchor.mark.doto = curwp->w_marko;
        anchor.mark.dotl = curwp->mlineno;
    }
    /* Save anchor start */
    if (n & ANCHOR_START)
    {
        anchor.startp = lback(curwp->w_dotp);
        anchor.starto = 0; /* curwp->w_doto */
        anchor.startl = curwp->line_no;
    }
    /* Save anchor end */
    if (n & ANCHOR_END)
    {
        anchor.endp = lforw(curwp->w_dotp);
        anchor.endo = 0; /*curwp->w_doto;*/
        anchor.endl = curwp->line_no;
    }
    /* Compute anchor offsets */
    if (n & ANCHOR_COMPUTE)
    {
        LINE *inp;
        int inword;
        int mm = ANCHOR_POINT|ANCHOR_MARK;
        int ninword;
        int offset;
        int wordcnt;
        uint8 cc;
        uint16 ino;
        
        /* Make sure that we have all of the information that we require in
         * order to be able to compute the postion independent postions for
         * out marker information. */
        if (anchor.startp == NULL)
            return mlwrite(MWABORT,(uint8 *)"No start position set");
        if (anchor.endp == NULL)
            return mlwrite(MWABORT,(uint8 *)"No end position set");
        if (anchor.point.dotp == NULL)
            mm &= ~ANCHOR_POINT;        /* No point set */
        if (anchor.mark.dotp == NULL)
            mm &= ~ANCHOR_MARK;         /* No mark set */
        if (mm == 0)
            /* nothing to do */
            return TRUE ;
        /* Save the line information */
        inp = lforw (anchor.startp);
        ino = anchor.starto;
        anchor.point.flags = ANCHFLG_INSPACE;
        anchor.mark.flags = ANCHFLG_INSPACE;
        
        offset = 0;
        inword = 0;
        wordcnt = 0;
        while (mm != 0)
        {
            /* Get the next character */
            if (llength (inp) <= ino)
                cc = '\0';
            else
                cc = lgetc(inp, ino);
            
            /* Determine the new 'in-word' setting of the character */
            if ((cc == ' ')|| (cc == TAB) || (cc == '\0'))
                ninword = ANCHFLG_INSPACE;
            else
                ninword = ANCHFLG_INWORD;
            
            /* Advance the counters in accordance with the word type */
            if (ninword ^ inword)
            {
                offset = 0;
                if ((inword = ninword) != ANCHFLG_INSPACE)
                    wordcnt++;
            }
            else
                offset++;
            
            /* Check for match on point */
            if ((anchor.point.dotp != 0) && 
                (anchor.point.dotp == inp) &&
                (anchor.point.doto == ino))
            {
                anchor.point.words = wordcnt; /* Number of words */
                anchor.point.offset = offset; /* Sub-position in word */
                anchor.point.flags = inword|ANCHFLG_VALID;
                mm &= ~ANCHOR_POINT;
            }
            
            /* Check for match on mark */
            if ((anchor.mark.dotp != 0) && 
                (anchor.mark.dotp == inp) &&
                (anchor.mark.doto == ino))
            {
                anchor.mark.words = wordcnt; /* Number of words */
                anchor.mark.offset = offset; /* Sub-position in word */
                anchor.mark.flags = inword|ANCHFLG_VALID;
                mm &= ~ANCHOR_MARK;
            }
            
            /* Check for the end marker */
            if ((anchor.endp == inp) && 
                (anchor.endo >= ino))
                break;
            
            /* Check for end of line */
            if (cc == '\0')
            {
                /* Advance line; emergency check on end of buffer */
                inp = lforw (inp);
                if (inp == curbp->b_linep)
                    break;
                ino = 0;   
            }               
            else
                ino++;
        }
    }
    return TRUE;
}

/* gotoAnchor; Retore the position of dot and mark using the anchor
 * information.
 * f=true;  n=ANCHOR_POINT;  - Restore point from remembered point.
 * f=true;  n=ANCHOR_MARK;   - Restore mark from remembered mark
 */
static int
gotoAnchor (int f, int n)
{
    /* Default is to restore point and mark */
    if (!f)
        n = ANCHOR_POINT|ANCHOR_MARK;
    
    /* Point and mark */
    if (n & (ANCHOR_POINT|ANCHOR_MARK))
    {
        LINE *linp, *inp;               /* Last + current line pointer */
        uint16 lino, ino;               /* Last + current line offset */
        int32 linl, inl;                /* Last + current line length */
        int inword = ANCHFLG_INSPACE;   /* Word status flag */
        int offset = 0;                 /* Offset into the word */
        int wordcnt = 0;                /* Number of words encountered */
        uint8 cc;                       /* Temporary working character */
        int mm = n & 3;                 /* Required operation */ 
        
        /* Make sure that the start positions are defined */
        if (anchor.startp == NULL)
            return mlwrite(MWABORT,(uint8 *)"No start position set");
        if (anchor.endp == NULL)
            return mlwrite(MWABORT,(uint8 *)"No end position set");
        
        /* Save the line information */
        inp = lforw(anchor.startp);
        ino = anchor.starto;
        inl = anchor.startl;
        linp = inp;
        lino = ino;
        linl = inl;
        
        /* Make sure that the point and mark are valid */
        if ((anchor.point.dotp == NULL) || ((anchor.point.flags & ANCHFLG_VALID) == 0))
            mm &= ~ANCHOR_POINT;
        if ((anchor.mark.dotp == NULL) || ((anchor.mark.flags & ANCHFLG_VALID) == 0))
            mm &= ~ANCHOR_MARK;
        
        /* Find the new paragraph position, provided that we have something
         * to do. */
        while (mm != 0)
        {
            int ninword;                /* Loop in word */
            
            /* Get the next character */
            if (llength (inp) <= ino)
                cc = '\0';
            else
                cc = lgetc(inp, ino);
        
            /* Determine if we are in a new word */
            if ((cc == ' ') || (cc == TAB) || (cc == '\0'))
                ninword = ANCHFLG_INSPACE;
            else
                ninword = ANCHFLG_INWORD;
            
            /* Match on end point */
            if (ninword ^ inword)
            {
                /* Fix up the pointers */
                if ((inword = ninword) != ANCHFLG_INSPACE)
                    wordcnt++;
                offset = 0;
                
                /* Match on point */
                if ((mm & ANCHOR_POINT) &&
                    (anchor.point.words == wordcnt-1))
                {
                    /* Fix up point */
                    lchange(WFMOVEC|WFMAIN);/* Old line has changed */
                    curwp->w_dotp = linp;
                    curwp->w_doto = lino;
                    curwp->line_no = linl;
                    lchange(WFMOVEC|WFMAIN);/* New line has changed */
                    curwp->w_flag |= WFMOVEL ;
                    if ((mm &= ~ANCHOR_POINT) == 0)
                        break;
                }
                
                /* Match on mark */
                if ((mm & ANCHOR_MARK) &&
                    (anchor.mark.words == wordcnt-1))
                {
                    curwp->w_markp = linp;
                    curwp->w_marko = lino;
                    curwp->mlineno = linl;
                    if ((mm &= ~ANCHOR_MARK) == 0)
                        break;
                }
            }
            else
                offset++;
            
            /* Match on point */
            if ((mm & ANCHOR_POINT) &&
                (anchor.point.words == wordcnt) &&
                (anchor.point.offset <= offset) &&
                ((anchor.point.flags & ANCHFLG_INWORD) == ninword))
            {
                /* Fix up point */
                lchange(WFMOVEC|WFMAIN);/* Old line has changed */
                curwp->w_dotp = inp;
                curwp->w_doto = ino;
                curwp->line_no = inl;
                curwp->w_flag |= WFMOVEL ;
                lchange(WFMOVEC|WFMAIN);/* New line has changed */
                curwp->w_flag |= WFMOVEL ;
                if ((mm &= ~ANCHOR_POINT) == 0)
                    break;
            }
            
            /* Match on mark */
            if ((mm & ANCHOR_MARK) &&
                (anchor.mark.words == wordcnt) &&
                (anchor.mark.offset <= offset) &&
                ((anchor.mark.flags & ANCHFLG_INWORD) == ninword))
            {
                curwp->w_markp = inp;
                curwp->w_marko = ino;
                curwp->mlineno = inl;
                if ((mm &= ~ANCHOR_MARK) == 0)
                    break;
            }
            
            /* End ?? */
            if ((inp == anchor.endp) && (ino >= anchor.endo))
                break;
            
            /* Save the last pointers */
            linp = inp;
            lino = ino;
            linl = inl;
            
            /* Check for the end of line */
            if (cc == '\0')
            {
                /* Are we going past the end anchor ?? */
                if (inp == anchor.endp)
                    break;
                inl++;
                
                /* Next line - check for end of buffer */
                inp = lforw (inp);
                if (inp == curbp->b_linep)
                    break;
                ino = 0;            /* Start of line */
            }
            else
                ino++;
        }
    }
    return TRUE;
}         

/* Word wrap on n-spaces. Back-over whatever precedes the point on the current
 * line and stop on the first word-break or the beginning of the line. If we
 * reach the beginning of the line, jump back to the end of the word and start
 * a new line.  Otherwise, break the line at the word-break, eat it, and jump
 * back to the end of the word.
 * Returns TRUE on success, FALSE on errors.
 */

int
wrapWord(int f, int n)
{
    register uint16 cnt, off ;		/* size of word wrapped to next line */
    register uint8 c, last=2 ;		/* charector temporary */
    
    if(bchange() != TRUE)               /* Check we can change the buffer */
        return ABORT ;
    
    /* Make sure we are not in no fill mode */
    if (toLower (fillmode) == 'n')
        return TRUE;
    
    /* back up until we aren't in a word,
       make sure there is a break in the line */
    cnt = 0 ;
    off = curwp->w_doto ;
    do
    {
        c = lgetc(curwp->w_dotp,curwp->w_doto) ;
        if((c == ' ') || (c == TAB))
        {
            if(last == 1)
                cnt = curwp->w_doto+1 ;
            last = 0 ;
        }
        else if(!last && (getccol() < fillcol))
            break ;
        else
            last = 1 ;
    } while((curwp->w_doto)-- > 0) ;
    
    if(cnt != 0)
        curwp->w_doto = cnt ;
    else
    {
        curwp->w_doto = off ;
        if(((c = lgetc(curwp->w_dotp,off)) != ' ') && (c != TAB))
            return TRUE ;
    }
    off = llength(curwp->w_dotp) - off ;
#if CFENCE
    if(meModeTest(curwp->w_bufp->b_mode,MDCMOD))
        cnt = cinsert() ;
    else
#endif
    if(meModeTest(curwp->w_bufp->b_mode,MDINDEN))
        cnt = winsert() ;
    else
    {
        cnt = lnewline() ;
#if MEUNDO
        if(cnt == TRUE)
            meUndoAddInsChars(n) ;
#endif
    }    
    if(meModeTest(curwp->w_bufp->b_mode,MDJUST))
        justify(-1,-1) ;
    
    curwp->w_doto = llength(curwp->w_dotp) - off ;
    return cnt ;
}


#if 0
/* 
 * removeTabs
 * This is a helper function for fillPara. It strips out the tabs in the 
 * line leading up to doto and replaces them with tabs. Additionally collects
 * the continuation line leader, returning a count of the leader depth to 
 * the caller. 
 */

static int
removeTabs (uint8 *iptr, int logUndo)
{
    int	ilength;                        /* length of the leader. */
    int	ii;                             /* Local loop counter */
    
    ilength = 0;
    for (ii = 0; ii < curwp->w_doto; ii++)
    {
        /* Expand TAB characters to spaces. */
        if (TAB == lgetc(curwp->w_dotp, ii))
        {
            int temp_doto;
            int jj;
            
            /* Get expansion width of TAB */ 
            jj = next_tab_pos (ilength);
            temp_doto = curwp->w_doto + jj - 1;
            
            /* Replace the TAB with equivelent SPACE's */
            curwp->w_doto = ii;
            ldelete (1L, logUndo);
            linsert (jj, ' ');
#if MEUNDO
            if (logUndo)
                meUndoAddInsChars(jj) ;
#endif
            curwp->w_doto = temp_doto;
        }
        
        /* Hive away the blanking for line continuation */
        ilength++;
        if (iptr != NULL)
            *iptr++ = ' ';
    }
    if (iptr != NULL)
        *iptr = '\0';                   /* Terminate the string */
    return (ilength);                   /* Return the string length */
}
#endif

static int
countGaps(void)
{
    unsigned char last;	/* Last character read. */
    unsigned char c;	/* Current character */
    int	count=0 ;	/* Number of gaps encountered */

    last = '\0';				/* Reset word */
    while((c=lgetc(curwp->w_dotp,curwp->w_doto)) != '\0')
    {
        if((last == ' ') && (c != ' '))
           ++count ;
        last = c;		/* Save last character */
        curwp->w_doto++;	/* Next character position */
    }
    return count ;		/* Record word */
    
}	/* End of "goto_gap" */

static char gapDir = 0;                 /* Gap direction */

static void
getBestGap(void)
{
    unsigned char last;	/* Last character read. */
    unsigned char c;	/* Current character */
    int	score=0, bestscore=0, currscore=0 ;	/* Number of gaps encountered */
    unsigned short bestoff, curroff, off ;
    
    last = 'z';				/* Reset word */
    off = curwp->w_doto ;
    for(;;)
    {
        if(((c=lgetc(curwp->w_dotp,off)) == '\0') || (c == ' '))
        {
            if(last != ' ')
            {
                if(!isWord(last))
                    score -= 10 ;
                currscore += score ;
/*                currscore += off & 0x7 ;*/
                currscore += gapDir ;
                if(currscore > bestscore)
                {
                    bestscore = currscore ;
                    bestoff = curroff ;
                }
                currscore = score+0x0fff0000 ;
                curroff = off ;
            }
            else
                currscore -= 0x10000 ;
            if(c == '\0')
                break ;
        }
        else
        {
            if(last == ' ')
            {
                if(!isWord(c))
                    score = 10 ;
                else
                    score = 20 ;
            }
            else
                score += 20 ;
        }
        last = c;		/* Save last character */
        off++;	                /* Next character position */
    }
    curwp->w_doto = bestoff ;		/* Record word */
    
}	/* End of "goto_gap" */

/* justify	
 * This routine will justify a block of text. The start of the text is at
 * doto, and the scaller specifies the length of the text to be justified, as
 * well as the new required length. This routine assumes the caller has
 * already removed tabs as justification is performed on spaces only.
 * Additionally the caller has already formatted the line to the correct
 * length. Justification is performed on the right margin only. (The left
 * margin is assumed to be done).
 * 
 * leftMargin is a instruction from fillPara() to inform justify where it's
 * left margin is. justify() determines it's own left margin and tabMode if
 * passed a -ve values. 
 * 
 * This is required for the bulleted "* " or "a) " and "ii -  " type
 * indentation where a leading space must not be altered.
 * 
 * Note that on first inspection justify appears to be doing too much work
 * in cleaning up the line. This is true for a fillPara() invocation but
 * it is not true for any other invocation. 
 */
int
justify(int leftMargin, int leftDoto)
{
    unsigned short  len, doto, ss ;
    unsigned short  odoto;              /* Starting doto backup */
    unsigned char   cc ;
    char            jmode;              /* Justification mode */
    int             undoMode;           /* Undo operation */
    int             col ;               /* left indent with tab expansion */
    
    /* Save old context */
    odoto = curwp->w_doto;
    curwp->w_dotp = lback(curwp->w_dotp) ;
    (curwp->line_no)-- ;
    undoMode = (leftMargin < 0) ? 2:0;  /* Compute the undo mode */ 
    
    /* We are always filling the previous line */
    jmode = toLower (fillmode);         /* Get justification mode */
    len = llength(curwp->w_dotp) ; 
    ss = len ;
    
    /* Strip any spaces from the right of the string */
    while(len > 0)
    {
        len-- ;
        if(((cc = lgetc(curwp->w_dotp,len)) != ' ') && (cc != TAB))
        {
            len++ ;
            break ;
        }
    }
    if((ss = ss - len) != 0)
    {
        curwp->w_doto = len ;
        ldelete (ss, undoMode) ;
    }

    /* If line is empty or left justification is required then do nothing */
    if((len == 0) || (strchr ("bcr", jmode) == NULL))
        goto finished;
    
    if(leftDoto < 0)
    {
	/* move to the left hand margin */
	curwp->w_doto = 0 ;
	while(((cc = lgetc(curwp->w_dotp,curwp->w_doto)) != '\0') &&
	      ((cc == ' ') || (cc == '\t')))
	    curwp->w_doto++ ;
	doto = curwp->w_doto ;
	col = getccol() ;
	if(leftMargin < 0)
	    leftMargin = col ;
    }
    else
    {
	doto = leftDoto ;
	curwp->w_doto = doto ;
	col = leftMargin ;
    }
    /* check for and trash any TABs on the rest of the line */
    while((cc=lgetc(curwp->w_dotp,curwp->w_doto)) != '\0')
    {
	if (cc == TAB)
	{
	    /* Get expansion width of TAB */ 
	    int jj = next_tab_pos(getccol());
	    /* Replace the TAB with equivelent SPACE's */
	    ldelete (1L,undoMode);
	    linsert (jj, ' ');
#if MEUNDO
	    if (undoMode)
		meUndoAddInsChars(jj) ;
#endif
	}
	curwp->w_doto++ ;
    }
    len = llength(curwp->w_dotp) - doto + leftMargin ;
    
    /* If there is leading white space and we are justifying centre
     * or right then delete the white space. */
    if(jmode != 'b')
    {
        /* Centre and right justification are pretty similar. For
         * right justification we insert "fillcol-linelen" spaces.
         * For centre justification we insert "fillcol-linelen/2"
         * spaces. */
        if(len > fillcol)
        {
            /* line is already too long - this can happen when filling
             * a paragraph to the right and the first line is not the
             * longest - pull back minimum to stay within the fillcol */
            len -= fillcol ;
            if(leftMargin < len)
                leftMargin = 0 ;
            else
                leftMargin -= len ;
        }
        else
        {
            len = fillcol - len ;
            if(jmode == 'c')                /* Centre ?? */
                leftMargin = (len + leftMargin) >> 1 ;
            else
                leftMargin += len ;
        }
    }
    /* Both left and right margin justification. */
    else if(len < (unsigned short) fillcol)
    {
        int gaps ;			/* The number of gaps in the line */
        
        ss = fillcol - len ;
        curwp->w_doto = doto ;	        /* Save doto position */
        gaps = countGaps() ;	        /* Get number of gaps */
        
        if(gaps == 0)			/* No gaps ?? */
        {
            linsert(ss, ' ') ;		/* Fill at end */
#if MEUNDO
            if (undoMode)
                meUndoAddInsChars(ss) ;
#endif
        }
        else                            /* Start filling the gaps with spaces */
        {
            int incGaps ;
            
            gapDir ^= 1 ;               /* Direction of gaps */
            do
            {
                curwp->w_doto = doto ;	/* Save doto position */
                getBestGap() ;
                
                incGaps = (ss+gaps-1)/gaps ;
                linsert(incGaps, ' ') ;
#if MEUNDO
                if (undoMode)
                    meUndoAddInsChars(incGaps) ;
#endif
                ss -= incGaps ;
                gaps-- ;
            } while(ss) ;
        }
    }

    /* Set the left margin for center and right modes,
     * and change the left margin back to tabs */
    if(col != leftMargin)
	meLineSetIndent(doto,leftMargin,undoMode) ;
finished:
    
    /* Restore context - return the new line length */
    ss = llength (curwp->w_dotp);
    curwp->w_dotp = lforw(curwp->w_dotp) ;
    curwp->w_doto = odoto ;
    (curwp->line_no)++ ;
    return ss;
}

/*
 * fillLookahead
 * Helper for fillPara(). Search forward in the paragraph for any magic
 * paragraph characters which are used for auto indentation. If found 
 * prompt the user that they have been found and ask for the left-hand
 * fill column. 
 * 
 * A change of lefthand fill column modifies the buffers doto position.
 * 
 * form is a bit of a hang over which I must come back and sort (Jon Green).
 * TABS are also bogus here !!
 */

static int
lookahead(int autoDetect)
{
    int	ii;                             /* Loop counter */
    int	limit;		                /* Limit */
    char c;                             /* Current character */
    char last_c;                        /* Last character */
    
    /* Nothing to do if there are no paragraph markers */
    if (fillbullet == NULL)
        return (0);
    limit = curwp->w_doto+fillbulletlen;/* Define the limit of search */
    if (limit > llength(curwp->w_dotp))
        limit = llength (curwp->w_dotp);
    
    /* Scan the line for a paragraph starting point. Start from the beginning
       of the line */
    for (ii = 0, last_c = (char) 1; ii < limit; ii++)
    {
        c = lgetc (curwp->w_dotp, ii);
        if ((meStrchr (fillbullet, last_c) != NULL) && (c == ' ' || c == TAB))
        {
            /* Go to the end of the spaces. */
            while (++ii < limit)
            {
                c = lgetc(curwp->w_dotp, ii);
                if ((c != ' ') && (c != TAB))
                    break;
            }
            
            /* If we are automatically detecting then try to make a 
             * sensible decision on the paragraph indentation without
             * troubling the user */
            if (autoDetect != 0)
            {
                LINE *temp_dotp;        /* Save our current line */
                
                /* Move to the next line and scan it */
                autoDetect = 0;         /* Assume a fail status */
                temp_dotp = curwp->w_dotp;  
                curwp->w_dotp = lforw (curwp->w_dotp);
                if (curwp->w_dotp != curbp->b_linep)	/* EOF  ?? */
                {
                    int jj;
                    int jlen;
                    
                    /* Build up a profile of the spaces on the next line */
                    jlen = llength (curwp->w_dotp);
                    for (jj = 0; jj < jlen; jj++)
                    {
                        c = lgetc(curwp->w_dotp, jj);
                        if ((c != ' ') && (c != TAB))
                            break;
                    }
                
                    /* Only prompt the user if the indent on the next line
                     * does not match the current line expectation. Or the
                     * current line is liable to spill onto the next line
                     * which is empty */
                    if ((jj == ii) ||       /* Next line indented to 'ii' */
                        ((jj == jlen) && (llength(temp_dotp) <= (unsigned short) fillcol)))
                    {
                        curwp->w_doto = ii; /* Assume position 'i' */
                        autoDetect = 1;     /* Disable prompt */
                    }
                }
                /* Restore previous position */
                curwp->w_dotp = temp_dotp;
            }
            
            /* If auto detect is disabled then manually prompt the user
             * for the indented position. */
            if (autoDetect == 0)
            {
                short temp_doto;        /* Temporary character position */
                int   status;           /* Status of command. */
                
                temp_doto = curwp->w_doto;
                curwp->w_doto = ii;
                
                if((status = mlCharReply((uint8 *)"Indent to <<<<<<<<<< (yn)? ",mlCR_QUIT_ON_USER|mlCR_LOWER_CASE,(uint8 *)"yn",NULL)) == -2)
                {
                    uint8 scheme=(curbp->scheme/meSCHEME_STYLES) ;
            
                    /* Force an update of the screen to to ensure that the user
                     * can see the information in the correct location */
                    update (TRUE);
                    pokeScreen(0x10,mwRow,mwCol,&scheme,(uint8 *)"<<<<<<<<<<") ;
                    status = mlCharReply((uint8 *)"Indent to <<<<<<<<<< (yn)? ",mlCR_LOWER_CASE,(uint8 *)"yn",NULL) ;
                    mlerase(0);
                }
                if(status == 'y')
                    curwp->w_doto = ii;
                else
                {
                    curwp->w_doto = temp_doto;	/* Restore */
                    if (status == -1)
                        return -1 ;
                }
            }
            return (autoDetect);        /* Automatically detected */
        }
        else
            last_c = c;
    }
    return (0);
}


/*
 * Try to autodetect the fill mode from the formation of the first line
 * in the paragraph. This will assume that the first line. is correctly
 * formatted.
 */
static char 
fillAutoDetect (char mode)
{
    int doto;                           /* Doto position */
    int len;                            /* lenght of the line */
    int textLen;                        /* Length of the text */
    
    doto = curwp->w_doto;               /* Save current position */
    len = llength (curwp->w_dotp);      /* Length of the line */
    textLen = (len - doto);             /* Text length */
    mode = toLower (mode);              /* Align mode to regular operation */
    
    /* Test for centre */
    if (len < fillcol )                 /* On the right margin */
    {
        int sdiff;                      /* Start difference */
        int ediff;                      /* End difference */
        
        /* Check for balanced line. We expect the start and end of the
         * line to be approximatly balanced. Allow a margin of error of
         * 6 characters */
        sdiff = (fillcol >> 1) - (doto + (textLen >> 1));
        if (sdiff < 0)                  /* Quick abs !! */
            sdiff = -sdiff;
        ediff = (fillcol >> 1) - (len  - (textLen >> 1));
        if (ediff < 0)                  /* Quick abs !! */
            ediff = -ediff;
        sdiff += ediff;
        
        if ((sdiff == 0) ||             /* We are certain !! */
            ((sdiff < 2) && (textLen < fillcol/2)))
            return ('c');               /* We are centred */
    }
    
    /* Test for right aligned text */
    if ((len >= fillcol) && (doto > (fillcol/2)))
        return ('r');                   /* We are right */
    
    /* Check for text on the left-hand edge of line. If it meets our
     * criteria for not filling the whole paragraph then apply 'n' which
     * formats each line separately. */
    if ((doto == 0) && (len <= (fillcol/2)))
        return ('n');                   /* No justification for left */
    
    return (mode);                      /* Returned modified mode */
}              
    
/* Fill the current paragraph according to the current fill column */
int
fillPara(int f, int n)	
{
#define FILL_LEFT    0x0001             /* Left fill                    */
#define FILL_RIGHT   0x0002             /* Right fill                   */
#define FILL_BOTH    0x0004             /* Both fill                    */
#define FILL_CENTRE  0x0008             /* Center fill                  */
#define FILL_NONE    0x0010             /* None fill                    */
#define FILL_TMASK   0x001f             /* Type mask for fill           */
#define FILL_INDENT  0x0020             /* Do Indented fill             */
#define FILL_FORCE   0x0040             /* Forced line output           */
#define FILL_DOT     0x0080             /* Last character was a period. */
#define FILL_EOP     0x0100             /* End of paragraph detected    */
#define FILL_FIRST   0x0200             /* First word ? (needs no space */
#define FILL_JUSTIFY 0x0400             /* Status of the justify flag   */
#define FILL_AUTO    0x0800             /* Automatic fill               */
#define FILL_LINE    0x1000             /* Fill to a line               */
#define FILL_MARGIN  0x2000             /* Fill to margin (AUTO+LINE)   */
    
    LINE *eopline;		        /* ptr to line just past EOP	*/
    int32 eoplno;		        /* line no of line just past EOP*/
    uint8 ofillmode;                    /* Old justification mode       */
    uint8 wbuf[NSTRING];	        /* buffer for current word	*/
    int c;			        /* current char durring scan	*/
    int clength;		        /* position on line during fill	*/
    int ccol;				/* position on line during fill	*/
    int newcol;			        /* tentative new line length	*/
    int wordlen;		        /* length of current word	*/
    int ilength;			/* Initial line length          */
    int icol;				/* Initial line columns         */
    register int fillState;             /* State of the fill            */
    int fdoto;                          /* The left doto for 1st line   */

#if MEUNDO
    UNDOND *undoNd ;
    int paralen ;
#endif
    
    if (fillcol == 0)                   /* Fill column set ??*/
        return mlwrite(MWABORT,(uint8 *)"No fill column set");
    if ((c=bchange()) != TRUE)           /* Check we can change the buffer */
        return c ;
    
    /* A -ve cont indicates that we do not want to be prompted for indentation
     * We also do not prompt if we are right or centre justifying */
    if (n < 0)
    {
        fillState = 0 ;                 /* Disable indented fill */
        n = 0 - n ;
    }
    else
        fillState = FILL_INDENT;        /* Enable indented fill */
    
    /* Sort out the justification mode that we are running in.
     * An upper case justification mode implies an automatic mode
     * where we detect the current state of the line and apply the 
     * appropriate formatting.
     *
     * If the One Line mode (o) is operational then fill compleate 
     * paragraphs to a single line. Indentation is disabled */
    ofillmode = fillmode;
    if (isUpper (fillmode))
    {
        fillState |= FILL_AUTO;         /* Auto paragraph type detection */
        fillmode = toLower (fillmode);
        if (fillmode == 'o')
            fillState = (fillState | FILL_LINE | FILL_MARGIN) & ~FILL_INDENT; 

    }
    else if (fillmode == 'o')
        fillState = (fillState | FILL_LINE) & ~FILL_INDENT; 
    
    /* In none mode then we do not touch the paragraph. */
    if (fillmode == 'n') 
    {
        /* If there are no arguments then do nothing, otherwise advance the
         * paragraph. */
        if (!f)
            return TRUE;
        return forwPara(FALSE, n) ;
    }
    
    /* Record if justify mode is enabled. Record in our local context
     * Also knock off indent if disabled. We do not want to be prompting
     * the user. */
    if(meModeTest(curbp->b_mode,MDJUST)) /* Justify enabled ?? */
        fillState |= FILL_JUSTIFY;       /* Yes - Set justification state */
    else
    {
        /* Justification is disabled. If centre, right or both is enabled
         * then reduce to left. */
        fillmode = 'l';
        fillState &= ~FILL_INDENT;       /* No - Knock indent off */
    }
    
    /* If fill paragraph is called with no arguments then we must retain the
     * position of dot so that we can preserve the users position after we
     * have filled the paragraph. */
    if (!f)
        setAnchor (TRUE, ANCHOR_POINT); /* Save current 'point' */
    
    /* Fill 'n' paragraphs */
    while (--n >= 0)
    {
        setAnchor (TRUE, ANCHOR_MARK);  /* Save mark */
                  
        /* go to the beginning of the line and use forward-paragraph
         * to go to the end of the current or next paragraph, if this
         * fails we are at the end of buffer */
        curwp->w_doto = 0 ;             /* Got start of current line */
        if (forwPara(FALSE, 1) != TRUE)
            break;
        
	/* record the pointer to the line just past the EOP 
         * and then back top the beginning of the paragraph.
         * doto is at the beginning of the first word */
        eopline = lforw(curwp->w_dotp);
        eoplno = curwp->line_no + 1 ;
        setAnchor (TRUE, ANCHOR_END);   /* End of paragraph */
	backPara(FALSE, 1);
        
        /* Skip non-formatting paragraphs */
        if ((fillignore != NULL) &&
            (meStrchr (fillignore, lgetc (curwp->w_dotp, curwp->w_doto)) != NULL))
        {
            curwp->w_dotp = eopline;    /* Goto the end of the paragraph */
            curwp->line_no = eoplno ;
            curwp->w_doto = 0;
            continue;			/* Next one */
	}
        setAnchor (TRUE, ANCHOR_START); /* Start of paragraph */
				
        /* Quick auto test to determine what mode the current paragraph is.
         * Set up the modes in the fill status mask */
        if (fillState & FILL_AUTO)
            fillmode = fillAutoDetect (ofillmode);
        fillState &= ~FILL_TMASK;
        switch (fillmode)
        {
        case 'b':                       /* Both mode */
            fillState |= FILL_BOTH;
            break;
        case 'r':                       /* Right mode */
            if (fillState & FILL_MARGIN)
                goto noIndent;          /* Reduce to 'none' */
            fillState |= FILL_RIGHT;
            break;
        case 'c':                       /* Center mode */
            if (fillState & FILL_MARGIN)
                goto noIndent;          /* Reduce to 'none' */
            fillState |= FILL_CENTRE;
            break;
noIndent:            
        case 'g':                       /* Guffer mode */
            fillmode = 'n';
            /* Drop through */
        case 'n':                       /* None mode */
            fillState |= FILL_NONE;
            break;
        case 'o':                       /* One line justification */
            fillmode = 'l';             /* Force to left */
            /* Drop through */
        default:                        /* Left justification */
            fillState |= FILL_LEFT;
            break;
        }
        
        setAnchor (TRUE, ANCHOR_COMPUTE);/* Remember paragraph positions */
        
	fdoto = curwp->w_doto ;
        /* Get the initial string from the start of the paragraph.
         * lookahead() modifies the current doto value. */
        if (((fillState & (FILL_INDENT|FILL_BOTH|FILL_LEFT)) > FILL_INDENT) &&
            (lookahead(fillState & FILL_AUTO) == -1))
        {
            fillmode = ofillmode;
            return (ABORT);
        }
	/* if lookahead has found a new left indent position, this position
	 * must be passed to justify so justify ignores the text to the left
	 * of this indent point */
	if(fdoto == curwp->w_doto)
	    fdoto = -1 ;
	else
	    fdoto = curwp->w_doto ;
        
        ilength = curwp->w_doto;
#if MEUNDO
        curwp->w_doto = 0;
        meUndoAddReplaceBgn(eopline,0) ;
        undoNd = curbp->fUndo ;
        curwp->w_doto = (uint16) ilength;
        paralen = 0 ;
#endif
        /* Get the indentation column, this is the real left indentation position */
	icol = getccol() ;
        
	/* Reset to initial settings.
         * Modes are:-
         * 
         *     ~EOP    - Not at end of paragraph
         *     ~FORCE  - Not a forced paragraph line
         *     ~DOT    - Not a period present
         *      FIRST  - This is the first word
         */
        clength = ilength;              /* Character length is leader length */
        ccol = icol;
        wordlen = 0;                    /* No word is present */
 
        fillState = ((fillState & ~(FILL_EOP|FILL_FORCE|FILL_DOT)) |
                     (FILL_FIRST));
        
        /* scan through lines, filling words */
        while ((fillState & FILL_EOP) == 0)
        {
            /* get the next character in the paragraph */
            if((c = lgetc(curwp->w_dotp, curwp->w_doto)) == '\0')
            {
                c = ' ';
                if (lforw(curwp->w_dotp) == eopline)
                    fillState |= FILL_EOP;  /* End of paragraph */
                
                if (fillState & (FILL_CENTRE|FILL_NONE|FILL_RIGHT))
                    fillState |=FILL_FORCE; /* Force paragraph output */
            }
            
            /* and then delete it */
            ldelete(1L,0);
            
            /* if not a separator, just add it in */
            if (c != ' ' && c != TAB)
            {
                if (wordlen < NSTRING - 1)
                    wbuf[wordlen++] = c;
            }
            else if (wordlen)
            {
                /* at a word break with a word waiting */
                /* calculate tantitive new length with word added */
                newcol = ccol + 1 + wordlen;
                if (fillState & FILL_DOT)
                    newcol += filleoslen-1 ;
                if ((newcol <= fillcol) || (fillState & FILL_LINE))
                {
                    /* add word to current line */
                    if ((fillState & FILL_FIRST) == 0)
                    {
                        if (fillState & FILL_DOT)
                        {
                            linsert(filleoslen, ' ') ; /* the space */
                            clength += filleoslen ;
                            ccol += filleoslen ;
                        }
                        else
                        {
                            linsert(1, ' '); /* the space */
                            ++clength;
			    ++ccol;
                        }
                    }
                    fillState &= ~(FILL_FIRST|FILL_DOT);
                }
                else
                {
                    if (fillState & FILL_JUSTIFY)
                    {
                        lnewline();
                        clength = justify (icol,fdoto);
			/* reset the indent offset as following lines will not
                         * have the bullet text to the left of the indent
                         * column */
			fdoto = -1 ;
                    }
                    else
                    {
                        clength = curwp->w_doto;
                        lnewline();
                    }
#if MEUNDO
                    paralen += clength + 1;
#endif
		    meLineSetIndent(0,icol,0) ;
                    clength = (int) ilength;
                    ccol = (int) icol;
                    fillState &= ~FILL_DOT;
                }
                
                /* and add the word in either case */
                lsinsert(wordlen, wbuf);
                clength += wordlen;
                ccol += wordlen;
                if (meStrchr(filleos,wbuf[wordlen-1]) != NULL)
                    fillState |= FILL_DOT;
                wordlen = 0;
            }
                
            if (fillState & FILL_FORCE)
            {
                if (fillState & FILL_JUSTIFY)
                {
                    lnewline();
                    clength = justify (icol,fdoto);
		    fdoto = -1 ;
                }
                else
                {
                    clength = curwp->w_doto;
                    lnewline ();
                }
#if MEUNDO
                paralen += clength + 1;
#endif
                /* Turn off dot marker and forced line */
                ccol = 0;
                clength = 0;
                if ((fillState & FILL_EOP) == 0)
                    fillState = ((fillState & ~(FILL_DOT|FILL_FORCE)) |
                                 FILL_FIRST);
            }
        }

        if ((fillState & FILL_FORCE) == 0)
        {
#if MEUNDO
            paralen += curwp->w_doto + 1;
#endif
            lnewline();
        }
#if MEUNDO
        if(undoNd != NULL)
        {
            undoNd->count = paralen ;
            undoNd->type |= MEUNDO_MINS ;
        }
#if 0
        meUndoAddReplaceEnd (paralen);
#endif
#endif
        /* Fix up the mark */
        if (f)
            gotoAnchor (TRUE, ANCHOR_MARK); /* Restore mark */
    }
    fillmode = ofillmode;
    
    /* Restore point and mark */
    if (!f)
    {
        gotoAnchor (TRUE, ANCHOR_POINT);/* Restore point and mark */
        setAnchor (FALSE, ANCHOR_CLEAR);/* Clear store */
    }
    return(TRUE);
}

int	
killPara(int f, int n)	/* delete n paragraphs starting with the current one */
{
    int ss ;
    
    if(n == 0)
        return TRUE ;
    if((f=(n < 0)))
        n = -n ;
    
    if((forwPara(FALSE, 1) != TRUE) ||
       (backPara(FALSE, 1) != TRUE) )
        return FALSE ;
    curwp->w_doto = 0 ;
    /* set the mark here */
    curwp->w_markp = curwp->w_dotp ;
    curwp->mlineno = curwp->line_no ;
    curwp->w_marko = curwp->w_doto ;
    
    if(bchange() != TRUE)               /* Check we can change the buffer */
        return ABORT ;
        
    ss = forwPara(TRUE,n) ;
    
    curwp->line_no++ ;
    curwp->w_dotp = lforw(curwp->w_dotp);
    curwp->w_doto = 0 ;
    
    /* and delete it */
    if(killRegion(TRUE,(f) ? -1:1) != TRUE)
        return FALSE ;
    return ss ;
}


/*	countWords:	count the # of words in the marked region,
			along with average word sizes, # of chars, etc,
			and report on them.			*/

int	
countWords(int f, int n)
{
    register LINE *lp;		/* current line to scan */
    register int offset;	/* current char to scan */
    long size;			/* size of region left to count */
    register int ch;		/* current character to scan */
    register int wordflag;	/* are we in a word now? */
    register int lastword;	/* were we just in a word? */
    long nwords;		/* total # of words */
    long nchars;		/* total number of chars */
    int nlines;			/* total number of lines in region */
    int status;			/* status return code */
    REGION region;		/* region to look at */

    /* make sure we have a region to count */
    if((status = getregion(&region)) != TRUE)
        return status ;
    lp = region.linep;
    offset = region.offset;
    size = region.size;
    
    /* count up things */
    lastword = FALSE;
    nchars = 0L;
    nwords = 0L;
    nlines = 0;
    while(size--) 
    {
        /* get the current character */
        if((ch = lgetc(lp, offset)) == '\0')
        {   /* end of line */
            ch = meNLCHAR;
            lp = lforw(lp);
            offset = 0;
            nlines++ ;
        } 
        else
            offset++ ;
        
        /* and tabulate it */
        wordflag = isWord(ch) ;
        if((wordflag != FALSE) && (lastword == FALSE))
            ++nwords;
        lastword = wordflag;
        ++nchars;
    }

    /* and report on the info */
    sprintf((char *)resultStr,"%ld Words, %ld Chars, %d Lines",
            nwords,nchars,nlines+1);
    return mlwrite(MWSPEC,resultStr) ;
}
#endif
