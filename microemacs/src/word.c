/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * word.c - Word processing routines.
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
 * Synopsis:    Word processing routines.
 * Authors:     Unknown, Jon Green & Steven Phillips
 * Description:
 *     The routines in this file implement commands that work word or a
 *     paragraph at a time. There are all sorts of word mode commands.
 *     If I do any sentence mode commands, they are likely to be put in
 *     this file. 
 *
 * Notes:
 * 
 *     Jon: 99/07/09
 *
 *     I have just added the new Anchor routines in here to preserve the
 *     character position of point and mark on fill paragraph. This was a
 *     fairly quick modification that disturbed as little of the code as
 *     possible, thereby maintaining the integrity of the release.
 *    
 *     The anchor routines are written ready for export because I think that
 *     they might be useful, although they have not actually been exported yet
 *     because with misuse they will crash editor. So I'm in two minds about
 *     this.
 *    
 *     The fill paragraph is now in need of a severe overhaul to reduce the
 *     amount of work that it performs. It currently performs a destructive
 *     justification process which is deeply unpleasant. In addition it must
 *     scan through the source copying it multiple times - very un-nice.
 *    
 *     I have started to prototype a replacement which is not ready yet. This
 *     will overcome all of the above. In addition I want to add a new Flow
 *     mode, this will allow MicroEmacs to act more like a word processor and
 *     throw words during the editing process onto the next line, and then
 *     automatically re-format the remaining part of the paragraph. Obviously
 *     handling left/right/centre correctly.
 *    
 *     On top of all of that. I want to change the paragraph recognition
 *     markers to be a buffer dependent regexp, thereby allowing the different
 *     paragraph types to be recognised and handled correctly. And finally,
 *     allow for text leaders (i.e. typically programming language comment
 *     delimiters) and correctly handle text wrapping with leader insertion.
 *    
 *     One could argue that some of the above belongs in ifill-paragraph
 *     (macro wrapper around fill-paragraph). However for smatter flow control
 *     I do not think that this is the case and would like to pursue a generic
 *     method that handles a multitude of cases, given the minimal amount of
 *     setup information in the buffer
 *    
 *     All of the above is possible, it just takes a little time for some of
 *     these things to come together - a commodity I've not got much of at the
 *     moment.
 */

#define	__WORDC			/* Define filename */

#include "emain.h"

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
    if (WbackChar(frameCur->windowCur, 1) == meFALSE)
        return (meFALSE);
    while (n--)
    {
        while (inWord() == meFALSE)
        {
            if (WbackChar(frameCur->windowCur, 1) == meFALSE)
                return (meFALSE);
        }
        while (inWord() != meFALSE)
        {
            if (WbackChar(frameCur->windowCur, 1) == meFALSE)
                /* We can't move back any more cos we're at the start,
                 * BUT as we have moved and we are in the buffers first word, 
                 * we should succeed */
                return meTRUE ;
        }
    }
    return (WforwChar(frameCur->windowCur, 1));
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
        while (inWord() == meFALSE)
        {
            if (WforwChar(frameCur->windowCur, 1) == meFALSE)
                return (meFALSE);
        }
        while (inWord() != meFALSE)
        {
            if (WforwChar(frameCur->windowCur, 1) == meFALSE)
                return (meFALSE);
        }
    }
    return(meTRUE);
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
        return (meFALSE);
    if((c=bchange()) != meTRUE)               /* Check we can change the buffer */
        return c ;
    while (n--)
    {
        while (inWord() == meFALSE) 
        {
            if(WforwChar(frameCur->windowCur, 1) == meFALSE)
                return (meFALSE);
        }
        while (inWord() != meFALSE)
        {
            c = meLineGetChar(frameCur->windowCur->dotLine, frameCur->windowCur->dotOffset);
            if (isLower(c))
            {
                lchange(WFMAIN);
#if MEOPT_UNDO
                meUndoAddRepChar() ;
#endif
                c = toggleCase(c) ;
                meLineSetChar(frameCur->windowCur->dotLine, frameCur->windowCur->dotOffset, c);
            }
            if (WforwChar(frameCur->windowCur, 1) == meFALSE)
                return (meFALSE);
        }
    }
    return (meTRUE);
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
        return (meFALSE);
    if((c=bchange()) != meTRUE)               /* Check we can change the buffer */
        return c ;
    while (n--)
    {
        while (inWord() == meFALSE) 
        {
            if (WforwChar(frameCur->windowCur, 1) == meFALSE)
                return (meFALSE);
        }
        while (inWord() != meFALSE)
        {
            c = meLineGetChar(frameCur->windowCur->dotLine, frameCur->windowCur->dotOffset);
            if (isUpper(c))
            {
                lchange(WFMAIN);
#if MEOPT_UNDO
                meUndoAddRepChar() ;
#endif
                c = toggleCase(c) ;
                meLineSetChar(frameCur->windowCur->dotLine, frameCur->windowCur->dotOffset, c);
            }
            if (WforwChar(frameCur->windowCur, 1) == meFALSE)
                return (meFALSE);
        }
    }
    return (meTRUE);
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
        return (meFALSE);
    if((c=bchange()) != meTRUE)               /* Check we can change the buffer */
        return c ;
    while (n--)
    {
        while (inWord() == meFALSE)
        {
            if (WforwChar(frameCur->windowCur, 1) == meFALSE)
                return (meFALSE);
        }
        if (inWord() != meFALSE) {
            c = meLineGetChar(frameCur->windowCur->dotLine, frameCur->windowCur->dotOffset);
            if (isLower(c))
            {
                lchange(WFMAIN);
#if MEOPT_UNDO
                meUndoAddRepChar() ;
#endif
                c = toggleCase(c) ;
                meLineSetChar(frameCur->windowCur->dotLine, frameCur->windowCur->dotOffset,c);
            }
            if (WforwChar(frameCur->windowCur, 1) == meFALSE)
                return (meFALSE);
            while (inWord() != meFALSE)
            {
                c = meLineGetChar(frameCur->windowCur->dotLine, frameCur->windowCur->dotOffset);
                if (isUpper(c))
                {
                    lchange(WFMAIN);
#if MEOPT_UNDO
                    meUndoAddRepChar() ;
#endif
                    c = toggleCase(c) ;
                    meLineSetChar(frameCur->windowCur->dotLine, frameCur->windowCur->dotOffset,c);
                }
                if (WforwChar(frameCur->windowCur, 1) == meFALSE)
                    return (meFALSE);
            }
        }
    }
    return (meTRUE);
}

/*
 * Kill forward by "n" words. Remember the location of dot. Move forward by
 * the right number of words. Put dot back where it was and issue the kill
 * command for the right number of characters. Bound to "M-D".
 */
int
forwDelWord(int f, int n)
{
    register meLine   *dotp;
    register int    doto, delType ;
    meInt           size, lineno ;
    
    if(n == 0)
        return meTRUE ;
    if((f=(n < 0)))
        n = -n ;
    
    if(bchange() != meTRUE)               /* Check we can change the buffer */
        return meABORT ;
    dotp   = frameCur->windowCur->dotLine;
    doto   = frameCur->windowCur->dotOffset;
    lineno = frameCur->windowCur->dotLineNo ;
    size = 0;
    while (n--)
    {
        /* inWord returns 0 if not in word - BUT if in word its return
         * value is only defined as non-zero, so must test if 0
         */
        delType = (inWord() == 0) ;
        while ((inWord() == 0) == delType) 
        {
            if (WforwChar(frameCur->windowCur, 1) == meFALSE)
                break ;
            ++size;
        }
     }
     frameCur->windowCur->dotLine = dotp;
     frameCur->windowCur->dotOffset = doto;
     frameCur->windowCur->dotLineNo = lineno ;
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
    meUByte delType ;
    meInt size;
    int moveForw=meTRUE ;
    
    if(n == 0)
        return meTRUE ;
    if((f=(n < 0)))
        n = -n ;
    
    if(bchange() != meTRUE)               /* Check we can change the buffer */
        return meABORT ;
    if (WbackChar(frameCur->windowCur, 1) == meFALSE)
        return (meFALSE);
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
            if ((moveForw=WbackChar(frameCur->windowCur, 1)) == meFALSE)
                break ;
        }
    }
    if (moveForw && (WforwChar(frameCur->windowCur, 1) == meFALSE))
        return (meFALSE);
    return (ldelete(size,(f) ? 2:3));
}

#if MEOPT_WORDPRO
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
    meLine    *dotp;                      /* Line position */
    long     words;                     /* Number of words */
    long     offset;                    /* Sub-position in word */
    meInt    dotl;                      /* Line number */
    meUShort doto;                      /* Line offset */
    meUByte  flags;                     /* 0 = in space; 1 = in word; 2 = valid */
};

typedef struct SAnchor
{
    struct PosAnchor point;             /* Location of point */
    struct PosAnchor mark;              /* Location of mark */
    meLine    *startp;                    /* Starting Line */
    meUShort starto;                    /* Starting Offset */
    meInt    startl;                    /* Starting Line number */
    meLine    *endp;                      /* Ending Line */
    meUShort endo;                      /* Ending Line */
    meInt    endl;                      /* Ending Line number */
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
    if ((f == meFALSE) || (n == ANCHOR_CLEAR))
    {
        anchor.point.dotp = NULL;
        anchor.mark.dotp = NULL;
        anchor.endp = NULL;
        anchor.startp = NULL;
        return meTRUE;
    }
    
    /* Determine what we are doing */
    /* Save point */
    if (n & ANCHOR_POINT)
    {
        anchor.point.dotp = frameCur->windowCur->dotLine;
        anchor.point.doto = frameCur->windowCur->dotOffset;
        anchor.point.dotl = frameCur->windowCur->dotLineNo;
    }
    /* Save mark */
    if (n & ANCHOR_MARK)
    {
        anchor.mark.dotp = frameCur->windowCur->markLine;
        anchor.mark.doto = frameCur->windowCur->markOffset;
        anchor.mark.dotl = frameCur->windowCur->markLineNo;
    }
    /* Save anchor start */
    if (n & ANCHOR_START)
    {
        anchor.startp = meLineGetPrev(frameCur->windowCur->dotLine);
        anchor.starto = 0; /* frameCur->windowCur->dotOffset */
        anchor.startl = frameCur->windowCur->dotLineNo;
    }
    /* Save anchor end */
    if (n & ANCHOR_END)
    {
        anchor.endp = meLineGetNext(frameCur->windowCur->dotLine);
        anchor.endo = 0; /*frameCur->windowCur->dotOffset;*/
        anchor.endl = frameCur->windowCur->dotLineNo;
    }
    /* Compute anchor offsets */
    if (n & ANCHOR_COMPUTE)
    {
        meLine *inp;
        int inword;
        int mm = ANCHOR_POINT|ANCHOR_MARK;
        int ninword;
        int offset;
        int wordcnt;
        meUByte cc;
        meUShort ino;
        
        /* Make sure that we have all of the information that we require in
         * order to be able to compute the postion independent postions for
         * out marker information. */
        if (anchor.startp == NULL)
            return mlwrite(MWABORT,(meUByte *)"No start position set");
        if (anchor.endp == NULL)
            return mlwrite(MWABORT,(meUByte *)"No end position set");
        if (anchor.point.dotp == NULL)
            mm &= ~ANCHOR_POINT;        /* No point set */
        if (anchor.mark.dotp == NULL)
            mm &= ~ANCHOR_MARK;         /* No mark set */
        if (mm == 0)
            /* nothing to do */
            return meTRUE ;
        /* Save the line information */
        inp = meLineGetNext (anchor.startp);
        ino = anchor.starto;
        anchor.point.flags = ANCHFLG_INSPACE;
        anchor.mark.flags = ANCHFLG_INSPACE;
        
        offset = 0;
        inword = 0;
        wordcnt = 0;
        while (mm != 0)
        {
            /* Get the next character */
            if (meLineGetLength (inp) <= ino)
                cc = '\0';
            else
                cc = meLineGetChar(inp, ino);
            
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
                inp = meLineGetNext (inp);
                if (inp == frameCur->bufferCur->baseLine)
                    break;
                ino = 0;   
            }               
            else
                ino++;
        }
    }
    return meTRUE;
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
        meLine *linp, *inp;               /* Last + current line pointer */
        meUShort lino, ino;             /* Last + current line offset */
        meInt linl, inl;                /* Last + current line length */
        int inword = ANCHFLG_INSPACE;   /* Word status flag */
        int offset = 0;                 /* Offset into the word */
        int wordcnt = 0;                /* Number of words encountered */
        meUByte cc;                     /* Temporary working character */
        int mm = n & 3;                 /* Required operation */ 
        
        /* Make sure that the start positions are defined */
        if (anchor.startp == NULL)
            return mlwrite(MWABORT,(meUByte *)"No start position set");
        if (anchor.endp == NULL)
            return mlwrite(MWABORT,(meUByte *)"No end position set");
        
        /* Save the line information */
        inp = meLineGetNext(anchor.startp);
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
            if (meLineGetLength (inp) <= ino)
                cc = '\0';
            else
                cc = meLineGetChar(inp, ino);
        
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
                    frameCur->windowCur->dotLine = linp;
                    frameCur->windowCur->dotOffset = lino;
                    frameCur->windowCur->dotLineNo = linl;
                    lchange(WFMOVEC|WFMAIN);/* New line has changed */
                    frameCur->windowCur->flag |= WFMOVEL ;
                    if ((mm &= ~ANCHOR_POINT) == 0)
                        break;
                }
                
                /* Match on mark */
                if ((mm & ANCHOR_MARK) &&
                    (anchor.mark.words == wordcnt-1))
                {
                    frameCur->windowCur->markLine = linp;
                    frameCur->windowCur->markOffset = lino;
                    frameCur->windowCur->markLineNo = linl;
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
                frameCur->windowCur->dotLine = inp;
                frameCur->windowCur->dotOffset = ino;
                frameCur->windowCur->dotLineNo = inl;
                frameCur->windowCur->flag |= WFMOVEL ;
                lchange(WFMOVEC|WFMAIN);/* New line has changed */
                frameCur->windowCur->flag |= WFMOVEL ;
                if ((mm &= ~ANCHOR_POINT) == 0)
                    break;
            }
            
            /* Match on mark */
            if ((mm & ANCHOR_MARK) &&
                (anchor.mark.words == wordcnt) &&
                (anchor.mark.offset <= offset) &&
                ((anchor.mark.flags & ANCHFLG_INWORD) == ninword))
            {
                frameCur->windowCur->markLine = inp;
                frameCur->windowCur->markOffset = ino;
                frameCur->windowCur->markLineNo = inl;
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
                inp = meLineGetNext (inp);
                if (inp == frameCur->bufferCur->baseLine)
                    break;
                ino = 0;            /* Start of line */
            }
            else
                ino++;
        }
    }
    return meTRUE;
}         

/* Word wrap on n-spaces. Back-over whatever precedes the point on the current
 * line and stop on the first word-break or the beginning of the line. If we
 * reach the beginning of the line, jump back to the end of the word and start
 * a new line.  Otherwise, break the line at the word-break, eat it, and jump
 * back to the end of the word.
 * Returns meTRUE on success, meFALSE on errors.
 */

int
wrapWord(int f, int n)
{
    register meUShort cnt, off ;		/* size of word wrapped to next line */
    register meUByte c, last=2 ;		/* charector temporary */
    
    if(bchange() != meTRUE)               /* Check we can change the buffer */
        return meABORT ;
    
    /* Make sure we are not in no fill mode */
    if (toLower (fillmode) == 'n')
        return meTRUE;
    
    /* back up until we aren't in a word,
       make sure there is a break in the line */
    cnt = 0 ;
    off = frameCur->windowCur->dotOffset ;
    do
    {
        c = meLineGetChar(frameCur->windowCur->dotLine,frameCur->windowCur->dotOffset) ;
        if((c == ' ') || (c == TAB))
        {
            if(last == 1)
                cnt = frameCur->windowCur->dotOffset+1 ;
            last = 0 ;
        }
        else if(!last && (getccol() < fillcol))
            break ;
        else
            last = 1 ;
    } while((frameCur->windowCur->dotOffset)-- > 0) ;
    
    if(cnt != 0)
        frameCur->windowCur->dotOffset = cnt ;
    else
    {
        frameCur->windowCur->dotOffset = off ;
        if(((c = meLineGetChar(frameCur->windowCur->dotLine,off)) != ' ') && (c != TAB))
            return meTRUE ;
    }
    off = meLineGetLength(frameCur->windowCur->dotLine) - off ;
#if MEOPT_CFENCE
    if(meModeTest(frameCur->windowCur->buffer->mode,MDCMOD))
        cnt = cinsert() ;
    else
#endif
    if(meModeTest(frameCur->windowCur->buffer->mode,MDINDEN))
        cnt = winsert() ;
    else
    {
        cnt = lnewline() ;
#if MEOPT_UNDO
        if(cnt == meTRUE)
            meUndoAddInsChars(n) ;
#endif
    }    
    if(meModeTest(frameCur->windowCur->buffer->mode,MDJUST))
        justify(-1,-1) ;
    
    frameCur->windowCur->dotOffset = meLineGetLength(frameCur->windowCur->dotLine) - off ;
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
removeTabs (meUByte *iptr, int logUndo)
{
    int	ilength;                        /* length of the leader. */
    int	ii;                             /* Local loop counter */
    
    ilength = 0;
    for (ii = 0; ii < frameCur->windowCur->dotOffset; ii++)
    {
        /* Expand TAB characters to spaces. */
        if (TAB == meLineGetChar(frameCur->windowCur->dotLine, ii))
        {
            int temp_doto;
            int jj;
            
            /* Get expansion width of TAB */ 
            jj = next_tab_pos (ilength);
            temp_doto = frameCur->windowCur->dotOffset + jj - 1;
            
            /* Replace the TAB with equivelent SPACE's */
            frameCur->windowCur->dotOffset = ii;
            ldelete (1L, logUndo);
            linsert (jj, ' ');
#if MEOPT_UNDO
            if (logUndo)
                meUndoAddInsChars(jj) ;
#endif
            frameCur->windowCur->dotOffset = temp_doto;
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
    meUByte last;       /* Last character read. */
    meUByte c;          /* Current character */
    int	count=0 ;	/* Number of gaps encountered */

    last = '\0';				/* Reset word */
    while((c=meLineGetChar(frameCur->windowCur->dotLine,frameCur->windowCur->dotOffset)) != '\0')
    {
        if((last == ' ') && (c != ' '))
           ++count ;
        last = c;		/* Save last character */
        frameCur->windowCur->dotOffset++;	/* Next character position */
    }
    return count ;		/* Record word */
    
}	/* End of "goto_gap" */

static char gapDir = 0;                 /* Gap direction */

static void
getBestGap(void)
{
    meUByte last;       /* Last character read. */
    meUByte c;          /* Current character */
    int	score=0, bestscore=0, currscore=0 ;	/* Number of gaps encountered */
    meUShort bestoff, curroff, off ;
    
    last = 'z';				/* Reset word */
    off = frameCur->windowCur->dotOffset ;
    for(;;)
    {
        if(((c=meLineGetChar(frameCur->windowCur->dotLine,off)) == '\0') || (c == ' '))
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
    frameCur->windowCur->dotOffset = bestoff ;		/* Record word */
    
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
    meUShort  len, doto, ss ;
    meUShort  odoto;              /* Starting doto backup */
    meUByte   cc ;
    char      jmode;              /* Justification mode */
    int       undoMode;           /* Undo operation */
    int       col ;               /* left indent with tab expansion */
    
    /* Save old context */
    odoto = frameCur->windowCur->dotOffset;
    frameCur->windowCur->dotLine = meLineGetPrev(frameCur->windowCur->dotLine) ;
    (frameCur->windowCur->dotLineNo)-- ;
    undoMode = (leftMargin < 0) ? 2:0;  /* Compute the undo mode */ 
    
    /* We are always filling the previous line */
    jmode = toLower (fillmode);         /* Get justification mode */
    len = meLineGetLength(frameCur->windowCur->dotLine) ; 
    ss = len ;
    
    /* Strip any spaces from the right of the string */
    while(len > 0)
    {
        len-- ;
        if(((cc = meLineGetChar(frameCur->windowCur->dotLine,len)) != ' ') && (cc != TAB))
        {
            len++ ;
            break ;
        }
    }
    if((ss = ss - len) != 0)
    {
        frameCur->windowCur->dotOffset = len ;
        ldelete (ss, undoMode) ;
    }

    /* If line is empty or left justification is required then do nothing */
    if((len == 0) || (strchr ("bcr", jmode) == NULL))
        goto finished;
    
    if(leftDoto < 0)
    {
	/* move to the left hand margin */
	frameCur->windowCur->dotOffset = 0 ;
	while(((cc = meLineGetChar(frameCur->windowCur->dotLine,frameCur->windowCur->dotOffset)) != '\0') &&
	      ((cc == ' ') || (cc == '\t')))
	    frameCur->windowCur->dotOffset++ ;
	doto = frameCur->windowCur->dotOffset ;
	col = getccol() ;
	if(leftMargin < 0)
	    leftMargin = col ;
    }
    else
    {
	doto = leftDoto ;
	frameCur->windowCur->dotOffset = doto ;
	col = leftMargin ;
    }
    /* check for and trash any TABs on the rest of the line */
    while((cc=meLineGetChar(frameCur->windowCur->dotLine,frameCur->windowCur->dotOffset)) != '\0')
    {
	if (cc == TAB)
	{
	    /* Get expansion width of TAB */ 
	    int jj = next_tab_pos(getccol());
	    /* Replace the TAB with equivelent SPACE's */
	    ldelete (1L,undoMode);
	    linsert (jj, ' ');
#if MEOPT_UNDO
	    if (undoMode)
		meUndoAddInsChars(jj) ;
#endif
	}
	frameCur->windowCur->dotOffset++ ;
    }
    len = meLineGetLength(frameCur->windowCur->dotLine) - doto + leftMargin ;
    
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
    else if(len < (meUShort) fillcol)
    {
        int gaps ;			/* The number of gaps in the line */
        
        ss = fillcol - len ;
        frameCur->windowCur->dotOffset = doto ;	        /* Save doto position */
        gaps = countGaps() ;	        /* Get number of gaps */
        
        if(gaps == 0)			/* No gaps ?? */
        {
            linsert(ss, ' ') ;		/* Fill at end */
#if MEOPT_UNDO
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
                frameCur->windowCur->dotOffset = doto ;	/* Save doto position */
                getBestGap() ;
                
                incGaps = (ss+gaps-1)/gaps ;
                linsert(incGaps, ' ') ;
#if MEOPT_UNDO
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
    ss = meLineGetLength (frameCur->windowCur->dotLine);
    frameCur->windowCur->dotLine = meLineGetNext(frameCur->windowCur->dotLine) ;
    frameCur->windowCur->dotOffset = odoto ;
    (frameCur->windowCur->dotLineNo)++ ;
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
    limit = frameCur->windowCur->dotOffset+fillbulletlen;/* Define the limit of search */
    if (limit > meLineGetLength(frameCur->windowCur->dotLine))
        limit = meLineGetLength (frameCur->windowCur->dotLine);
    
    /* Scan the line for a paragraph starting point. Start from the beginning
       of the line */
    for (ii = 0, last_c = (char) 1; ii < limit; ii++)
    {
        c = meLineGetChar (frameCur->windowCur->dotLine, ii);
        if ((meStrchr (fillbullet, last_c) != NULL) && (c == ' ' || c == TAB))
        {
            /* Go to the end of the spaces. */
            while (++ii < limit)
            {
                c = meLineGetChar(frameCur->windowCur->dotLine, ii);
                if ((c != ' ') && (c != TAB))
                    break;
            }
            
            /* If we are automatically detecting then try to make a 
             * sensible decision on the paragraph indentation without
             * troubling the user */
            if (autoDetect != 0)
            {
                meLine *temp_dotp;        /* Save our current line */
                
                /* Move to the next line and scan it */
                autoDetect = 0;         /* Assume a fail status */
                temp_dotp = frameCur->windowCur->dotLine;  
                frameCur->windowCur->dotLine = meLineGetNext (frameCur->windowCur->dotLine);
                if (frameCur->windowCur->dotLine != frameCur->bufferCur->baseLine)	/* EOF  ?? */
                {
                    int jj;
                    int jlen;
                    
                    /* Build up a profile of the spaces on the next line */
                    jlen = meLineGetLength (frameCur->windowCur->dotLine);
                    for (jj = 0; jj < jlen; jj++)
                    {
                        c = meLineGetChar(frameCur->windowCur->dotLine, jj);
                        if ((c != ' ') && (c != TAB))
                            break;
                    }
                
                    /* Only prompt the user if the indent on the next line
                     * does not match the current line expectation. Or the
                     * current line is liable to spill onto the next line
                     * which is empty */
                    if ((jj == ii) ||       /* Next line indented to 'ii' */
                        ((jj == jlen) && (meLineGetLength(temp_dotp) <= (meUShort) fillcol)))
                    {
                        frameCur->windowCur->dotOffset = ii; /* Assume position 'i' */
                        autoDetect = 1;     /* Disable prompt */
                    }
                }
                /* Restore previous position */
                frameCur->windowCur->dotLine = temp_dotp;
            }
            
            /* If auto detect is disabled then manually prompt the user
             * for the indented position. */
            if (autoDetect == 0)
            {
                short temp_doto;        /* Temporary character position */
                int   status;           /* Status of command. */
                
                temp_doto = frameCur->windowCur->dotOffset;
                frameCur->windowCur->dotOffset = ii;
                
                if((status = mlCharReply((meUByte *)"Indent to <<<<<<<<<< (yn)? ",mlCR_QUIT_ON_USER|mlCR_LOWER_CASE,(meUByte *)"yn",NULL)) == -2)
                {
                    meUByte scheme=(frameCur->bufferCur->scheme/meSCHEME_STYLES) ;
            
                    /* Force an update of the screen to to ensure that the user
                     * can see the information in the correct location */
                    update (meTRUE);
                    pokeScreen(0x10,frameCur->mainRow,frameCur->mainColumn,&scheme,(meUByte *)"<<<<<<<<<<") ;
                    status = mlCharReply((meUByte *)"Indent to <<<<<<<<<< (yn)? ",mlCR_LOWER_CASE,(meUByte *)"yn",NULL) ;
                    mlerase(0);
                }
                if(status == 'y')
                    frameCur->windowCur->dotOffset = ii;
                else
                {
                    frameCur->windowCur->dotOffset = temp_doto;	/* Restore */
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
    
    doto = frameCur->windowCur->dotOffset;               /* Save current position */
    len = meLineGetLength (frameCur->windowCur->dotLine);      /* Length of the line */
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
#define FILL_MARGIN  0x2000             /* Fill to margin (AUTO+meLine)   */
    
    meLine *eopline;		        /* ptr to line just past EOP	*/
    meInt eoplno;		        /* line no of line just past EOP*/
    meUByte ofillmode;                    /* Old justification mode       */
    meUByte wbuf[meSBUF_SIZE_MAX];	        /* buffer for current word	*/
    int c;			        /* current char durring scan	*/
    int clength;		        /* position on line during fill	*/
    int ccol;				/* position on line during fill	*/
    int newcol;			        /* tentative new line length	*/
    int wordlen;		        /* length of current word	*/
    int ilength;			/* Initial line length          */
    int icol;				/* Initial line columns         */
    register int fillState;             /* State of the fill            */
    int fdoto;                          /* The left doto for 1st line   */

#if MEOPT_UNDO
    meUndoNode *undoNd ;
    int paralen ;
#endif
    
    if (fillcol == 0)                   /* Fill column set ??*/
        return mlwrite(MWABORT,(meUByte *)"No fill column set");
    if ((c=bchange()) != meTRUE)           /* Check we can change the buffer */
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
            return meTRUE;
        return forwPara(meFALSE, n) ;
    }
    
    /* Record if justify mode is enabled. Record in our local context
     * Also knock off indent if disabled. We do not want to be prompting
     * the user. */
    if(meModeTest(frameCur->bufferCur->mode,MDJUST)) /* Justify enabled ?? */
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
        setAnchor (meTRUE, ANCHOR_POINT); /* Save current 'point' */
    
    /* Fill 'n' paragraphs */
    while (--n >= 0)
    {
        setAnchor (meTRUE, ANCHOR_MARK);  /* Save mark */
                  
        /* go to the beginning of the line and use forward-paragraph
         * to go to the end of the current or next paragraph, if this
         * fails we are at the end of buffer */
        frameCur->windowCur->dotOffset = 0 ;             /* Got start of current line */
        if (forwPara(meFALSE, 1) != meTRUE)
            break;
        
	/* record the pointer to the line just past the EOP 
         * and then back top the beginning of the paragraph.
         * doto is at the beginning of the first word */
        eopline = meLineGetNext(frameCur->windowCur->dotLine);
        eoplno = frameCur->windowCur->dotLineNo + 1 ;
        setAnchor (meTRUE, ANCHOR_END);   /* End of paragraph */
	backPara(meFALSE, 1);
        
        /* Skip non-formatting paragraphs */
        if ((fillignore != NULL) &&
            (meStrchr (fillignore, meLineGetChar (frameCur->windowCur->dotLine, frameCur->windowCur->dotOffset)) != NULL))
        {
            frameCur->windowCur->dotLine = eopline;    /* Goto the end of the paragraph */
            frameCur->windowCur->dotLineNo = eoplno ;
            frameCur->windowCur->dotOffset = 0;
            continue;			/* Next one */
	}
        setAnchor (meTRUE, ANCHOR_START); /* Start of paragraph */
				
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
        
        setAnchor (meTRUE, ANCHOR_COMPUTE);/* Remember paragraph positions */
        
	fdoto = frameCur->windowCur->dotOffset ;
        /* Get the initial string from the start of the paragraph.
         * lookahead() modifies the current doto value. */
        if (((fillState & (FILL_INDENT|FILL_BOTH|FILL_LEFT)) > FILL_INDENT) &&
            (lookahead(fillState & FILL_AUTO) == -1))
        {
            fillmode = ofillmode;
            return (meABORT);
        }
	/* if lookahead has found a new left indent position, this position
	 * must be passed to justify so justify ignores the text to the left
	 * of this indent point */
	if(fdoto == frameCur->windowCur->dotOffset)
	    fdoto = -1 ;
	else
	    fdoto = frameCur->windowCur->dotOffset ;
        
        ilength = frameCur->windowCur->dotOffset;
#if MEOPT_UNDO
        frameCur->windowCur->dotOffset = 0;
        meUndoAddReplaceBgn(eopline,0) ;
        undoNd = frameCur->bufferCur->undoHead ;
        frameCur->windowCur->dotOffset = (meUShort) ilength;
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
            if((c = meLineGetChar(frameCur->windowCur->dotLine, frameCur->windowCur->dotOffset)) == '\0')
            {
                c = ' ';
                if (meLineGetNext(frameCur->windowCur->dotLine) == eopline)
                    fillState |= FILL_EOP;  /* End of paragraph */
                
                if (fillState & (FILL_CENTRE|FILL_NONE|FILL_RIGHT))
                    fillState |=FILL_FORCE; /* Force paragraph output */
            }
            
            /* and then delete it */
            ldelete(1L,0);
            
            /* if not a separator, just add it in */
            if (c != ' ' && c != TAB)
            {
                if (wordlen < meSBUF_SIZE_MAX - 1)
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
                        clength = frameCur->windowCur->dotOffset;
                        lnewline();
                    }
#if MEOPT_UNDO
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
                    clength = frameCur->windowCur->dotOffset;
                    lnewline ();
                }
#if MEOPT_UNDO
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
#if MEOPT_UNDO
            paralen += frameCur->windowCur->dotOffset + 1;
#endif
            lnewline();
        }
#if MEOPT_UNDO
        if(undoNd != NULL)
        {
            undoNd->count = paralen ;
            undoNd->type |= meUNDO_MINS ;
        }
#if 0
        meUndoAddReplaceEnd (paralen);
#endif
#endif
        /* Fix up the mark */
        if (f)
            gotoAnchor (meTRUE, ANCHOR_MARK); /* Restore mark */
    }
    fillmode = ofillmode;
    
    /* Restore point and mark */
    if (!f)
    {
        gotoAnchor (meTRUE, ANCHOR_POINT);/* Restore point and mark */
        setAnchor (meFALSE, ANCHOR_CLEAR);/* Clear store */
    }
    return(meTRUE);
}

int	
killPara(int f, int n)	/* delete n paragraphs starting with the current one */
{
    int ss ;
    
    if(n == 0)
        return meTRUE ;
    if((f=(n < 0)))
        n = -n ;
    
    if((forwPara(meFALSE, 1) != meTRUE) ||
       (backPara(meFALSE, 1) != meTRUE) )
        return meFALSE ;
    frameCur->windowCur->dotOffset = 0 ;
    /* set the mark here */
    frameCur->windowCur->markLine = frameCur->windowCur->dotLine ;
    frameCur->windowCur->markLineNo = frameCur->windowCur->dotLineNo ;
    frameCur->windowCur->markOffset = frameCur->windowCur->dotOffset ;
    
    if(bchange() != meTRUE)               /* Check we can change the buffer */
        return meABORT ;
        
    ss = forwPara(meTRUE,n) ;
    
    frameCur->windowCur->dotLineNo++ ;
    frameCur->windowCur->dotLine = meLineGetNext(frameCur->windowCur->dotLine);
    frameCur->windowCur->dotOffset = 0 ;
    
    /* and delete it */
    if(killRegion(meTRUE,(f) ? -1:1) != meTRUE)
        return meFALSE ;
    return ss ;
}


/*	countWords:	count the # of words in the marked region,
			along with average word sizes, # of chars, etc,
			and report on them.			*/

int	
countWords(int f, int n)
{
    register meLine *lp;		/* current line to scan */
    register int offset;	/* current char to scan */
    long size;			/* size of region left to count */
    register int ch;		/* current character to scan */
    register int wordflag;	/* are we in a word now? */
    register int lastword;	/* were we just in a word? */
    long nwords;		/* total # of words */
    long nchars;		/* total number of chars */
    int nlines;			/* total number of lines in region */
    int status;			/* status return code */
    meRegion region;		/* region to look at */

    /* make sure we have a region to count */
    if((status = getregion(&region)) != meTRUE)
        return status ;
    lp = region.line ;
    offset = region.offset ;
    size = region.size ;
    
    /* count up things */
    lastword = meFALSE;
    nchars = 0L;
    nwords = 0L;
    nlines = 0;
    while(size--) 
    {
        /* get the current character */
        if((ch = meLineGetChar(lp, offset)) == '\0')
        {   /* end of line */
            ch = meNLCHAR;
            lp = meLineGetNext(lp);
            offset = 0;
            nlines++ ;
        } 
        else
            offset++ ;
        
        /* and tabulate it */
        wordflag = isWord(ch) ;
        if((wordflag != meFALSE) && (lastword == meFALSE))
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
