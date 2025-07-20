/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * random.c - Random Routines.
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
 * Synopsis:    Random Routines.
 * Authors:     Unknown, Jon Green & Steven Phillips
 */

#define	__RANDOMC		/* Define filename */

#include "emain.h"
#include "efunc.h"
#include "eskeys.h"

#if (defined _UNIX) || (defined _DOS)
#include <sys/types.h>
#endif

#ifndef _ME_USE_STD_MALLOC
void *
meMalloc(size_t s)
{
    void *r;
    if((r = malloc(s)) == NULL)
        mlwrite(MWCURSOR|MWABORT|MWWAIT,(meUByte *)"Warning! Malloc failure");
    return r;
}

void *
meRealloc(void *r, size_t s)
{
    if((r = realloc(r,s)) == NULL)
        mlwrite(MWCURSOR|MWABORT|MWWAIT,(meUByte *)"Warning! Malloc failure");
    return r;
}

void *
meStrdup(const meUByte *ss)
{
    size_t ll = strlen((char *) ss) + 1;
    void *rr;
    if((rr = malloc(ll)) == NULL)
        mlwrite(MWCURSOR|MWABORT|MWWAIT,(meUByte *)"Warning! Malloc failure");
    else
        memcpy(rr,ss,ll);
    return rr;
}
#endif

void
meStrrep(meUByte **dd, const meUByte *ss)
{
    size_t ll;
    void *rr;
    if(*dd != NULL)
        free(*dd);
    ll = strlen((char *) ss) + 1;
    if((rr = malloc(ll)) == NULL)
        mlwrite(MWCURSOR|MWABORT|MWWAIT,(meUByte *)"Warning! Malloc failure");
    else
        memcpy(rr,ss,ll);
    *dd = (meUByte *) rr;
}

/* case insensitive str compare
 * done by simply turning all letters to lower case
 */
int
meStrnicmp(const meUByte *str1, const meUByte *str2, size_t nn)
{
    meUByte cc, dd;
    int ii;
    nn++;
    while(--nn > 0)
    {
        cc = *str1++;
        dd = *str2++;
        if((cc != dd) &&
           ((ii=((int) toLower(cc)) - ((int) toLower(dd))) != 0))
            return ii;
        if(cc == 0)
            break;
    }
    return 0;
}

/* case insensitive str compare
 * done by simply turning all letters to lower case
 */
int
meStricmp(const meUByte *str1, const meUByte *str2)
{
    meUByte cc, dd;
    int ii;

    do {
        cc = *str1++;
        dd = *str2++;
        if((cc != dd) &&
           ((ii=((int) toLower(cc)) - ((int) toLower(dd))) != 0))
            return ii;
    } while(cc != 0);
    return 0;
}

/* case insensitive str compare/diff
 * This is slightly different to the above as if the
 * strings are the same when ignore the case, it then returns
 * the case sensitive result.
 */
int
meStridif(const meUByte *str1, const meUByte *str2)
{
    int cc, dd, rr=0, ss, tt;

    do {
        cc = *str1++;
        dd = *str2++;
        if((ss = cc - dd) != 0)
        {
            if((tt = ((int) toLower(cc)) - ((int) toLower(dd))) != 0)
                return tt;
            if(!rr)
                rr = ss;
        }
    } while(cc != 0);
    return rr;
}

/* sort the given list of strings out on the nth plus character. Can be used
 * on lines by adding (sizeof(meLine)-1) to the offset. */
#ifdef _NOQSORT
void
sortStrings(int noStr, meUByte **strs, int offset, meIFuncSS cmpFunc)
{
    int chng, i;
    meUByte *tmp;

    if(offset < 0)
    {
        offset = -1-offset;
        do
        {
            chng=0;
            for(i=0 ; i<noStr-1 ; i++)
                if(cmpFunc(strs[i]+offset,strs[i+1]+offset) < 0)
                {
                    tmp = strs[i];
                    strs[i] = strs[i+1];
                    strs[i+1] = tmp;
                    chng=1;
                }
        } while(chng);
    }
    else
    {
        do
        {
            chng=0;
            for(i=0 ; i<noStr-1 ; i++)
                if(cmpFunc(strs[i]+offset,strs[i+1]+offset) > 0)
                {
                    tmp = strs[i];
                    strs[i] = strs[i+1];
                    strs[i+1] = tmp;
                    chng=1;
                }
        } while(chng);
    }
}

#else

meIFuncSS sortStringsCmpFunc;
int sortStringsOffset;
int sortStringsBackward;

static int
sortStringsCmp(const void *v1, const void *v2)
{
    int ii;

    ii = sortStringsCmpFunc(*((const meUByte **) v1)+sortStringsOffset,*((const meUByte **) v2)+sortStringsOffset);
    if(sortStringsBackward)
        ii = 0-ii;
    return ii;
}

void
sortStrings(int noStr, meUByte **strs, int offset, meIFuncSS cmpFunc)
{
    if((sortStringsBackward=(offset < 0)))
        offset = -1-offset;
    sortStringsOffset = offset;
    sortStringsCmpFunc = cmpFunc;
    qsort(strs, noStr, sizeof(char *), sortStringsCmp);
}

#endif

int
sortLines(int f, int n)
{
    meWindow *cwp=frameCur->windowCur;
    meIFuncSS cmpFunc;
    meLine *sL, *eL, *l, **list;
    meInt sln, noL, ii, jj, offs;
#if MEOPT_UNDO
    meInt *undoInfo;
#endif

    if(cwp->markLine == NULL)
        return noMarkSet();

    if((noL=cwp->dotLineNo-cwp->markLineNo) == 0)
        return meTRUE;
    if((ii=bufferSetEdit()) <= 0)               /* Check we can change the buffer */
        return ii;
    if(noL < 0)
    {
        noL = 0 - noL;
        sln = cwp->dotLineNo;
        sL = meLineGetPrev(cwp->dotLine);
        eL = cwp->markLine;
    }
    else
    {
        sln = cwp->markLineNo;
        sL = meLineGetPrev(cwp->markLine);
        eL = cwp->dotLine;
        cwp->dotLine  = cwp->markLine;
        cwp->dotLineNo = sln;
    }
    if(f == meFALSE)
        n = 0;
    offs = (int) ((size_t) sL->text) - ((size_t) sL);
    if(n < 0)
    {
        offs = 0-offs;
        f = -1-n;
    }
    else
        f = n;
    if((list = (meLine **) meMalloc(noL*sizeof(meLine *))) == NULL)
        return meFALSE;
    for(ii=0, l=meLineGetNext(sL) ; ii<noL ; l=meLineGetNext(l),ii++)
    {
        list[ii] = l;
#if MEOPT_UNDO
        l->prev = (meLine *) mePtrFromInt(ii);
#endif
    }
    meAssert(l == eL);
    cwp->dotOffset = 0;
#if MEOPT_UNDO
    undoInfo = meUndoAddLineSort(noL);
#endif
    cwp->dotLine = eL;
    cwp->dotLineNo = sln+noL;

    if(meModeTest(cwp->buffer->mode,MDEXACT))
        cmpFunc = (meIFuncSS) strcmp;
    else
        cmpFunc = meStridif;
    if(f != 0)
    {
        for(ii=0,jj=0 ; ii<noL ; ii++)
        {
            if(meLineGetLength(list[ii]) < f)
            {
                l = list[jj];
                list[jj] = list[ii];
                list[ii] = l;
                jj++;
            }
        }
        sortStrings(jj,(meUByte **)list, offs,cmpFunc);
    }
    else
        jj = 0;
    sortStrings(noL-jj,(meUByte **)(list+jj), n+offs,cmpFunc);

    for(ii=0, l=sL ; ii<noL ; l=meLineGetNext(l),ii++)
    {
#if MEOPT_UNDO
        if(undoInfo != NULL)
            *undoInfo++ = meIntFromPtr(list[ii]->prev);
#endif
        l->next = list[ii];
        list[ii]->prev = l;
    }
    l->next = eL;
    eL->prev = l;

    cwp->markLine = *list;
    cwp->markOffset = 0;
    cwp->markLineNo = sln;

    meFree(list);
    lineSetChanged(WFMOVEL|WFMAIN);

    return meTRUE;
}


int
getBufferInfo(meInt *numlines, meInt *predlines,
              meInt *numchars, meInt *predchars)
{
    meWindow *cwp=frameCur->windowCur;
    register meLine *lp;	/* current line */
    register int curchar = 0;	/* character under cursor */

    /* starting at the beginning of the buffer */
    lp = meLineGetNext(cwp->buffer->baseLine);

    /* start counting chars and lines */
    *numchars = *numlines = 0;
    while (lp != cwp->buffer->baseLine)
    {
        if(lp->text[meLineGetLength(lp)] != '\0')
        {
            lp->text[meLineGetLength(lp)] = '\0';
            fprintf(stderr,"We've got problems on line [%s]\n",lp->text);
        }
        /* if we are on the current line, record it */
        if (lp == cwp->dotLine)
        {
            *predlines = *numlines;
            *predchars = *numchars + cwp->dotOffset;
            if((curchar = meLineGetChar(lp, cwp->dotOffset)) == '\0')
                curchar = meCHAR_NL;
        }
        /* on to the next line */
        (*numlines)++;
        *numchars += meLineGetLength(lp) + 1;
        lp = meLineGetNext(lp);
    }

    /* if at end of file, record it */
    if (cwp->dotLine == cwp->buffer->baseLine)
    {
        *predlines = *numlines;
        *predchars = *numchars;
        curchar = meCHAR_NL;
    }
    return curchar ;
}


/* buffer-info. Display the current position of the cursor, in origin 1 X-Y
 * coordinates, the character that is under the cursor (in hex), and the
 * fraction of the text that is before the cursor. The displayed column is not
 * the current column, but the column that would be used on an infinite width
 * display. Normally bound to C-x = */
int
bufferInfo(int f, int n)
{
    meWindow *cwp;
    meInt numchars;			/* # of chars in file */
    meInt numlines;			/* # of lines in file */
    meInt predchars;			/* # chars preceding point */
    meInt predlines;			/* # lines preceding point */
    int   curchar;			/* character under cursor */
    int   ratio;
    int   coff;
    int   ccol;
    int   eoff;
    int   ecol;

    curchar = getBufferInfo(&numlines,&predlines,&numchars,&predchars);
    cwp = frameCur->windowCur;

    /* Get real column and end-of-line column. */
    coff = cwp->dotOffset;
    ccol = getiwcol(cwp,coff);
    eoff = meLineGetLength(cwp->dotLine);
    ecol = (eoff == coff) ? ccol:getiwcol(cwp,eoff);

    ratio = 0;              /* Ratio before dot. */
    if (numchars != 0)
        ratio = (100L*predchars) / numchars;

    if(n == 0)
    {
        /* macro call - put info into $result only and in a more usable form */
        sprintf((char *)resultStr,"|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|0x%02x|",
                predlines+1,numlines+1,coff,eoff,predchars,numchars,ratio,
                (int) (cwp->dotLineNo-cwp->vertScroll),cwp->textDepth-1,ccol,ecol,curchar);
        return meTRUE;
    }
    /* summarize and report the info */
    sprintf((char *)resultStr,"Line %d/%d Col %d/%d Char %d/%d (%d%%) Win Line %d/%d ACol %d/%d char %d (0x%02x)",
            predlines+1,numlines+1,coff,eoff,predchars,numchars,ratio,
            (int) (cwp->dotLineNo-cwp->vertScroll),cwp->textDepth-1,ccol,ecol,curchar,curchar);
    return mlwrite(MWSPEC,resultStr);
}

int
getcline(meWindow *wp)	/* get the current line number */
{
    register meLine	*lp;		/* current line */
    register int	numlines;	/* # of lines before point */

    /* starting at the beginning of the buffer */
    lp = meLineGetNext(wp->buffer->baseLine);

    /* start counting lines */
    numlines = 0;
    while (lp != wp->buffer->baseLine)
    {
        /* if we are on the current line, record it */
        if (lp == wp->dotLine)
            break;
        ++numlines;
        lp = meLineGetNext(lp);
    }

    /* and return the resulting count */
    return numlines;
}

int
getcol(meUByte *ss,register int off, int tabWidth)
{
    register int c, col=0;

    while(off--)
    {
        c = *ss++;
        if(isDisplayable(c))
            col++;
        else if(c == meCHAR_TAB)
            col += get_tab_pos(col,tabWidth) + 1;
        else if (c  < 0x20)
            col += 2;
        else
            col += 4;
    }
    return col;
}

int
setccol(meWindow *wp,register int pos)
{
    register int c;	/* character being scanned */
    register int i;	/* index into current line */
    register int col;	/* current cursor column   */
    register int llen;	/* length of line in bytes */

    col = 0;
    llen = meLineGetLength(wp->dotLine);

    /* scan the line until we are at or past the target column */
    for(i = 0; i < llen; ++i)
    {
        /* advance one character */
        c = meLineGetChar(wp->dotLine,i);
        if(isDisplayable(c))
            col++;
        else if(c == meCHAR_TAB)
            col += get_tab_pos(col,wp->buffer->tabWidth) + 1;
        else if (c  < 0x20)
            col += 2;
        else
            col += 4;
        /* upon reaching the target, drop out */
        if (col > pos)
            break;

    }
    /* set the new position */
    wp->dotOffset = i;
    /* if not long enough return an error */
    return ((col < pos) ? meFALSE:meTRUE);
}

int
getiwcol(meWindow *wp, register int off)
{
    int col=0;
    if(off > 0)
    {
        register meUByte *cOff = windCurLineOffsetEval(wp);
        do {
            col += *cOff++;
        } while(--off > 0);
    }
    return col;
}
int
setcwcol(meWindow *wp,register int col)
{
    int ii=0, jj;

    if((col > 0) && ((jj = wp->dotLine->length) > 0))
    {
        register meUByte *off = windCurLineOffsetEval(wp);
        do {
            col -= off[ii++];
        } while((ii < jj) && (col > 0));
    }
    wp->dotOffset = ii;
    return ((col) ? meFALSE:meTRUE);
}

/* transpose-character. Twiddle the two characters on either side of dot. If
 * dot is at the end of the line transChars the two characters before it.
 * Return with an error if dot is at the beginning of line; it seems to be a
 * bit pointless to make this work. This fixes up a very common typo with a
 * single stroke. Normally bound to C-t. */
int
transChars(int f, int n)
{
    meWindow *cwp=frameCur->windowCur;
    meLine *dotp;
    int doto;
    meUByte cl, cr;

    dotp = cwp->dotLine;
    doto = cwp->dotOffset;
    if((doto==meLineGetLength(dotp)) && (doto > 0))
        cwp->dotOffset--;
    if(cwp->dotOffset == 0)
    {
        cwp->dotOffset = doto;
        return meFALSE;
    }
    cr = meLineGetChar(dotp, cwp->dotOffset);
    cwp->dotOffset--;
    if(bufferSetEdit() <= 0)               /* Check we can change the buffer */
        return meABORT;
    cl = meLineGetChar(dotp, cwp->dotOffset);
    lineSetChanged(WFMAIN);
#if MEOPT_UNDO
    meUndoAddRepChar();
    meLineSetChar(dotp, cwp->dotOffset, cr);
    cwp->dotOffset++;
    meUndoAddRepChar();
    meLineSetChar(dotp, cwp->dotOffset, cl);
#else
    meLineSetChar(dotp, doto+0, cr);
    meLineSetChar(dotp, doto+1, cl);
#endif
    cwp->dotOffset = doto;
    return (meTRUE);
}

/* transpose-lines. Swap the line before with dot and move down (unless at the
 * end where there is no advance. If dot is the top the move down 1 to start.
 * Normally bound C-x C-t. */
int
transLines(int f, int n)
{
    meWindow *cwp;
    meLine *ln1, *ln2;
    meInt i, j, ret;

    if(n == 0)
        return meTRUE;
    cwp = frameCur->windowCur;
    if(n < 0)
    {
        if(windowBackwardLine(meTRUE,1) <= 0)
            return meFALSE;
    }
    else if(cwp->dotLineNo+1 >= cwp->buffer->lineCount)
        return meErrorEob();
    if((i=bufferSetEdit()) <= 0)               /* Check we can change the buffer */
        return i;
    lineSetChanged(WFMAIN|WFMOVEL);
    cwp->dotOffset = 0;

    for(i=0,j=abs(n) ; ; )
    {
        if(((ln1 = cwp->dotLine) == cwp->buffer->baseLine) ||
           ((ln2 = meLineGetNext(ln1)) == cwp->buffer->baseLine))
            break;
        meLineGetPrev(meLineGetNext(ln2)) = ln1;
        meLineGetNext(meLineGetPrev(ln1)) = ln2;
        meLineGetPrev(ln2) = meLineGetPrev(ln1);
        meLineGetNext(ln1) = meLineGetNext(ln2);
        meLineGetPrev(ln1) = ln2;
        meLineGetNext(ln2) = ln1;
        cwp->dotLineNo++;
        if(++i == j)
        {
            if(n < 0)
                windowBackwardLine(meTRUE, 1);
            break;
        }
        if((n<0) && (windowBackwardLine(meTRUE, 2) <= 0))
            /* move back over one swapped aswell */
            break;
    }
    ret = (i == j);
#if MEOPT_UNDO
    if(i > 0)
    {
        meInt *undoInfo;
        if((undoInfo = meUndoAddLineSort(i+1)) != NULL)
        {
            if(n < 0)
            {
                j=0;
                *undoInfo++ = i--;
            }
            else
            {
                j=1;
                /* meUndoAddLineSort returned array[-1] == start line no, as we are at the end, ajust */
                undoInfo[-1] -= i;
                undoInfo[i] = 0;
            }
            do
                *undoInfo++ = j++;
            while(j<=i);
        }
    }
#endif
    lineSetChanged(WFMOVEL|WFMAIN);
    return ret;
}

int
quoteKeyToChar(meUShort cc)
{
    if(cc > 255)
    {
        switch(cc)
        {
        case ME_SPECIAL|SKEY_backspace:
            cc = 0x08;
            break;
        case ME_SPECIAL|SKEY_delete:
            cc = 0x7f;
            break;
        case ME_SPECIAL|SKEY_esc:
            cc = 0x1b;
            break;
        case ME_SPECIAL|SKEY_return:
            cc = 0x0d;
            break;
        case ME_SPECIAL|SKEY_tab:
            cc = 0x09;
            break;
        case ME_SPECIAL|SKEY_up:
            cc = 'P' - '@';
            break;
        case ME_SPECIAL|SKEY_down:
            cc = 'N' - '@';
            break;
        case ME_SPECIAL|SKEY_left:
            cc = 'B' - '@';
            break;
        case ME_SPECIAL|SKEY_right:
            cc = 'F' - '@';
            break;
        default:
            return -1;
        }
    }
    return cc;
}


/* quote-char. Quote the next character, and insert it into the buffer. All
 * the characters are taken literally, with the exception of the C-j,
 * which always has its line splitting meaning. The character is always read,
 * even if it is inserted 0 times. Normally bound to C-q
 */
int
quote(int f, int n)
{
    register int s;
    register int c;

    if(n < 0)
        return meFALSE;
    if(n == 0)
        return meTRUE;
    if((c = mlCharReply((meUByte *)"Quote: ",mlCR_QUOTE_CHAR,NULL, NULL)) < 0)
        return mlwrite(MWABORT,(meUByte *)"[Cannot quote this key]");
    if((s=bufferSetEdit()) <= 0)               /* Check we can change the buffer */
        return s;
    if(c == meCHAR_NL)
    {
        f = n;
        do
            s = lineInsertNewline(0);
        while((s > 0) && --n);
#if MEOPT_UNDO
        meUndoAddInsChars(f-n);
#endif
    }
    else
    {
        /* insert the required number of chars */
        s = insertChar(c,n);
    }
    return s;
}


/* tab. Insert n tabs or pseudo tabs. Normally bound to tab.
 */
int
meTab(int f, int n)
{
    meWindow *cwp=frameCur->windowCur;
    meBuffer *cbp=cwp->buffer;
    int ii;

    if(n<=0)
        return meTRUE;

#if MEOPT_HILIGHT
    if((cbp->indent) &&
       ((meSystemCfg & meSYSTEM_TABINDANY) ||
        ((meSystemCfg & meSYSTEM_TABINDFST) && (cwp->dotOffset == 0))))
        return indentLine(&ii);
#endif

    if((ii=bufferSetEdit()) <= 0)               /* Check we can change the buffer */
        return ii;

#if MEOPT_WORDPRO
    /* If a newline was typed, fill column is defined, the argument is non-
     * negative, wrap mode is enabled, and we are now past fill column,
     * and we are not read-only, perform word wrap.
     */
    if(meModeTest(cbp->mode,MDWRAP) &&
       (cbp->fillcol > 0) &&
       (getwcol(cwp) > cbp->fillcol))
        wrapWord(meFALSE, 1);
#endif
    if(!meModeTest(cbp->mode,MDTAB))
    {
        /* insert the required number of TABs */
        ii = insertChar(meCHAR_TAB,n);
    }
    else
    {
        int bufIndentWidth = (int)(cbp->indentWidth);
        int ss = (bufIndentWidth*(n-1)) + (bufIndentWidth - (getwcol(cwp)%bufIndentWidth)) - n;
        /* insert the required number of TABs as spaces first - this handles over mode
         * The extra spaces required are inserted next */
        if(((ii = insertChar(' ',n)) > 0) && ss &&
           ((ii = lineInsertChar(ss,' ')) > 0))
        {
#if MEOPT_UNDO
            meUndoAddInsChars(ss);
#endif
        }
    }
    return ii;
}

#if MEOPT_EXTENDED
/* backward-delete-tab. If the previous character was a <TAB> then delete. If
 * the pseudo tabs are in operation then delete spaces to the next tab stop
 * position. Normally bound to S-tab */
int
meBacktab(int f, int n)
{
    meWindow *cwp=frameCur->windowCur;
    meUByte cc;		/* Character buffer */
    int tabspace;	/* Count of the size of the TAB space */
    int delspace;	/* Count of the size of the delete space */
    int doto;		/* Current position in line - 1 */

    /*---	If we are at the beginning of the line or the line has no length then
       ignore the request */

    if ((meLineGetLength(cwp->dotLine) == 0) ||	/* Zero line length ?? */
        ((doto = cwp->dotOffset-1) < 0))	/* At start of line ?? */
        return (meFALSE);			/* Yes - quit out */

    if(bufferSetEdit() <= 0)               /* Check we can change the buffer */
        return meABORT;

    /* Delete the tabular space. */

    if(meModeTest(cwp->buffer->mode,MDTAB))
    {
        /*---	Forced tab spacing is in operation.
           Get the tabular spacing from our current position */
        int bufIndentWidth = (int) cwp->buffer->indentWidth;
        if ((tabspace = (getwcol(cwp) % bufIndentWidth)) == 0)
            tabspace = bufIndentWidth;

        /*---	Scan back through the characters in the line and determine the
           number of characters to remove */

        for (delspace = 0; delspace < tabspace; delspace++)
        {
            if (doto < 0)		/* At start of line ?? */
                break;		/* Yes - quit out */

            /*---	Get the character. If it is a <TAB> or any other
               character other than space then exit */

            cc = meLineGetChar (cwp->dotLine, doto);
            if (cc != ' ')
            {
                if (cc == meCHAR_TAB)
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
            cwp->dotOffset -= delspace;
            return (ldelete((meInt) delspace, 2));
        }
    }
    else
    {
        /*---	Natural tab spacing. If the previous character is a <TAB> then
           delete the <TAB> character from the line */

        if (meLineGetChar(cwp->dotLine, doto) == meCHAR_TAB)
        {   /* Tab char ?? */
            cwp->dotOffset = doto;		/* Yes - back up */
            return(ldelete(1L,2));	/* and delete */
        }
    }
    return (meFALSE);		/* Nothing to remove !!! */
}

/* delete-blank-lines. What this command does depends if dot is sitting on a
 * blank line. If dot is sitting on a blank line, this command deletes all the
 * blank lines above and below the current line. If it is sitting on a non
 * blank line then it deletes all of the blank lines after the line. Normally
 * this command is bound to C-x C-o. */
int
windowDeleteBlankLines(int f, int n)
{
    meWindow *cwp=frameCur->windowCur;
    register meLine *lp1;
    register meLine *lp2;
    long nld, lld=0;

    lp1 = cwp->dotLine;
    while((meLineGetLength(lp1) == 0) && ((lp2=meLineGetPrev(lp1)) != cwp->buffer->baseLine))
    {
        lp1 = lp2;
        lld++;
    }
    lp2 = lp1;
    nld = 0;
    while(((lp2=meLineGetNext(lp2)) != cwp->buffer->baseLine) && (meLineGetLength(lp2) == 0))
        ++nld;
    if (nld == 0)
        return (meTRUE);
    if(bufferSetEdit() <= 0)               /* Check we can change the buffer */
        return meABORT;
    cwp->dotLine = meLineGetNext(lp1);
    cwp->dotOffset = 0;
    cwp->dotLineNo -= lld-1;
    return (ldelete(nld,2));
}

#endif

#if MEOPT_WORDPRO
int
winsert(void)	/* insert a newline and indentation for Wrap indent mode */
{
    /*---	This is nasty if the user is not using tab because we expect
       to drop a tab position when  we come back. Hence on spaced tab we need
       to consume to the previous tab stop.  */

    register meUByte *cptr;	/* string pointer into text to copy */
    register int tptr;	        /* index to scan into line */
    register int i;
    meUByte ichar[meSBUF_SIZE_MAX];	/* buffer to hold indent of last line */

    /* grab a pointer to text to copy indentation from */
    cptr = &frameCur->windowCur->dotLine->text[0];

    /* check for a brace */
    tptr = frameCur->windowCur->dotOffset;

    /* save the indent of the previous line */
    i = 0;
    while ((i != tptr) && (cptr[i] == ' ' || cptr[i] == '\t')
           && (i < meSBUF_SIZE_MAX - 1))
    {
        ichar[i] = cptr[i];
        i++;
    }
    /* put in the newline */
    if (lineInsertNewline(0) <= 0)
        return meFALSE;

    if(i && (i != tptr))
    {
        /* and the saved indentation */
        if(lineInsertString(i, ichar) <= 0)
            return meFALSE;
#if MEOPT_UNDO
        meUndoAddInsChars(i+1);
#endif
    }
#if MEOPT_UNDO
    else
        meUndoAddInsChars(1);
#endif
    return(meTRUE);
}
#endif

#if MEOPT_HILIGHT
/* insert a newline and indentation for current buffers indent scheme */
int
indentInsert(void)
{
    meWindow *cwp;
    int inComment, doto;
    meUByte *str, *comCont;

    indentLine(&inComment);

    /* put in the newline */
    if(lineInsertNewline(0) <= 0)
        return meFALSE;
#if MEOPT_UNDO
    meUndoAddInsChar();
#endif
    if(indentLine(&inComment) <= 0)
        return meFALSE;

    cwp = frameCur->windowCur;
    if(inComment && ((comCont=meIndentGetCommentContinue(indents[cwp->buffer->indent])) != NULL))
    {
        doto = cwp->dotOffset;
        if(gotoFrstNonWhite() == 0)
        {
            int newInd;
            cwp->dotOffset = doto;
            if((newInd = getwcol(cwp) - 3) < 0)
                newInd = 0;
            if(meLineSetIndent(doto,newInd,1) <= 0)
                return meFALSE;
            str = comCont;
            while(*str != '\0')
                lineInsertChar(1, *str++);
            if((doto=((size_t) str) - ((size_t) comCont)) > 0)
            {
#if MEOPT_UNDO
                meUndoAddInsChars(doto);
#endif
            }
        }
        else
            cwp->dotOffset = doto;
    }
    return meTRUE;
}
#endif

int
meLineSetIndent(int curInd, int newInd, int undo)
{
    meWindow *cwp;
    meBuffer *cbp;
    register int ss, curOff;
    if((ss=bufferSetEdit()) <= 0)               /* Check we can change the buffer */
        return ss;
    cwp = frameCur->windowCur;
    curOff = cwp->dotOffset;
    cwp->dotOffset = 0;
    curOff -= curInd;
    ldelete(curInd,(undo) ? 6:0);
    cbp = cwp->buffer;
    if(meModeTest(cbp->mode,MDTAB))
        ss = 0;
    else
    {
        ss = newInd / cbp->tabWidth;
        newInd -= ss * cbp->tabWidth;
        lineInsertChar(ss,'\t');
    }
    lineInsertChar(newInd,' ');
    ss += newInd;
    curOff += ss;
#if MEOPT_UNDO
    if(undo && meModeTest(cbp->mode,MDUNDO))
    {
        meUndoAddReplaceEnd(ss);
        cbp->undoHead->doto = ss;
    }
#endif
    cwp->dotOffset = curOff;
    return meTRUE;
}

/* newline. If we are in CMODE or have an indent rule, do automatic
 * indentation as specified. Normally bound to C-m */
int
meNewline(int f, int n)
{
    meBuffer *cbp;
    register int s;

    if (n == 0)
        return meTRUE;
    if (n < 0)
        return meFALSE;
    if((s=bufferSetEdit()) <= 0)               /* Check we can change the buffer */
        return s;
    cbp = frameCur->windowCur->buffer;

#if MEOPT_WORDPRO
    /* If a newline was typed, fill column is defined, the argument is non-
     * negative, wrap mode is enabled, and we are now past fill column,
     * and we are not read-only, perform word wrap.
     */
    if(meModeTest(cbp->mode,MDWRAP) &&
       (cbp->fillcol > 0) &&
       (getwcol(frameCur->windowCur) > cbp->fillcol))
        wrapWord(meFALSE, 1);

    if(meModeTest(cbp->mode,MDJUST) &&
       ((cbp->fillmode == 'c') || (cbp->fillmode == 'r')))
        f = 1;
    else
        f = 0;
#endif
    while (n--)
    {
#if MEOPT_HILIGHT
        if(cbp->indent)
            s = indentInsert();
        else
#endif
#if MEOPT_WORDPRO
            if(meModeTest(cbp->mode,MDINDENT))
            s = winsert();
        else
#endif
        {
            s = lineInsertNewline(0);
#if MEOPT_UNDO
            if (s > 0)
                meUndoAddInsChar();
#endif
        }
        if (s <= 0)
            return s;
#if MEOPT_WORDPRO
        if(f)
        {
            f = 0;
            justify(-1,-1,cbp->fillmode);
        }
#endif
    }

    return meTRUE;
}

/* forward-delete-char. This is real easy, because the basic delete routine
 * does all of the work. Watches for negative arguments, and does the right
 * thing. If any argument is present, it kills rather than deletes, to prevent
 * loss of text if typed with a big argument. Normally bound to C-d. */
int
forwDelChar(int f, int n)
{
    int s, keep;

    if(n == 0)
        return meTRUE;
    if(n < 0)
    {
        n = -n;
        if((s = meWindowBackwardChar(frameCur->windowCur, n)) <= 0)
            return s;
    }
    if((s = bufferSetEdit()) <= 0)             /* Check we can change the buffer */
        return s;

    if(f != meFALSE)
        keep = 3;                      /* Save in kill */
    else
        keep = 2;

    /* Always make ldelete save the deleted stuff in a kill buffer
     * unless only one character and not in letter kill mode. */
    return ldelete(n,keep);
}

/* backward-delete-char. Normally bound to C-h */
int
backDelChar(int f, int n)
{
    return forwDelChar(f,-n);
}


/* kill-line. Kill or delete n lines. Normally bound to C-k */
int
killLine(int f, int n)
{
    meWindow *cwp;
    meBuffer *cbp;
    meLine *nextp;
    meInt chunk;
    int s;

    if(n == 0)
        return meTRUE;
    if((s=bufferSetEdit()) <= 0)               /* Check we can change the buffer */
        return s;
    cwp = frameCur->windowCur;
    cbp = cwp->buffer;

    if(n < 0)
    {
        nextp = meLineGetPrev(cwp->dotLine);
        if((chunk = cwp->dotOffset) == 0)
        {
            if(nextp == cbp->baseLine)
                meErrorBob();
            n--;
        }
        cwp->dotOffset = 0;
        while((++n < 0) && (nextp != cbp->baseLine))
        {
            chunk += meLineGetLength(nextp)+1;
            cwp->dotLine = nextp;
            cwp->dotLineNo--;
            nextp = meLineGetPrev(cwp->dotLine);
        }
    }
    else if(cwp->dotLine == cbp->baseLine)
        return meErrorEob();
    else if(f == meFALSE)
    {
        chunk = meLineGetLength(cwp->dotLine)-cwp->dotOffset;
        if (chunk == 0)
            chunk = 1;
    }
    else
    {
        if((chunk = cwp->dotOffset) == 0)
            chunk = meLineGetLength(cwp->dotLine) + 1;
        else if((chunk = meLineGetLength(cwp->dotLine) - chunk) == 0)
            chunk = 1;
        nextp = meLineGetNext(cwp->dotLine);
        while(--n && (nextp != cbp->baseLine))
        {
            chunk += meLineGetLength(nextp)+1;
            nextp = meLineGetNext(nextp);
        }
    }

    return ldelete(chunk,3);
}


/* ml-write. This function writes a string on the message line mainly for macro usage */
int
mlWrite(int f, int n)
{
    register int status;
    meUByte buf[meBUF_SIZE_MAX];	/* buffer to recieve message into */

    if(n == 0)
    {
        mlerase(0);
        return meTRUE;
    }
    if((status = meGetString((meUByte *)"Message", 0, 0, buf, meBUF_SIZE_MAX)) > 0)
    {
        status = MWSPEC;
        if(f == meFALSE)
            n = 0;
        else if(n < 0)
        {
            f = 0-n;
            if((f & 0x03) && ((alarmState & meALARM_PIPED) || (f & 0x04)))
                status |= (f & 0x01) ? MWSTDOUTWRT:MWSTDERRWRT;
            if(f & 0x08)
                status |= MWABORT;
            if(f & 0x10)
                status |= MWWAIT;
        }
        status = mlwrite(status,buf);
        if(n > 0)
            TTsleep(n,0,NULL);
    }
    return status;
}

#if MEOPT_FENCE

/* List of fense id chars, close (or move backward) first then open */
meUByte fenceString[] = "##/*><)(}{][\"'" ; /* */
#define meFENCE_BRACKET_OFFSET 6

meUByte
gotoFrstNonWhite(void)
{
    meWindow *cwp=frameCur->windowCur;
    register meUByte ch;

    while(((ch = meLineGetChar(cwp->dotLine,cwp->dotOffset)) != '\0') &&
          ((ch == ' ') || (ch == '\t')))
        cwp->dotOffset++;
    return ch;
}

static int
findfence(meUByte ch, meUByte forwFlag, meInt depth);

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
moveToNonWhite(meUByte forwFlag, meUByte *flags, meInt depth)
{
    register meWindow *cwp=frameCur->windowCur;
    register meUByte ch;
    register meLine *lp;
    meUByte *ss, lc;
    int inq;

    for(;;)
    {
        if(forwFlag)
        {
            for(;;)
            {
                if(cwp->dotOffset >= meLineGetLength(cwp->dotLine))
                {
                    /* Go back lines until we find a line which
                     * 1) Doesn't start with a '#'
                     * 2) Has some text on it
                     * Move cursor to end or if we find a c++ "//" comment
                     * then to the char before that
                     */
                    if(!((*flags) & MTNW_KNOWN))
                        *flags |= MTNW_KNOWN|MTNW_NOTINHASH;
                    if((lp = meLineGetNext(cwp->dotLine)) == cwp->buffer->baseLine)
                        return 0;
                    cwp->dotLine = lp;
                    cwp->dotLineNo++;
                    ss=lp->text;
                    while((ch = *ss++) != '\0')
                        if((ch != ' ') && (ch != '\t'))
                            break;
                    if(ch == '\0')
                        continue;
                    if((ch == '#') && ((*flags) & MTNW_NOTINHASH) && !((*flags) & MTNW_SIMPLE))
                    {
                        if((ss[0] == 'e') && (ss[1] == 'l'))
                        {
                            /* Moving forwards, we want to skip #else & #elifs,
                             * but findfence moves to the next #e? if it was an #elif
                             * so we must check it is an endif
                             */
                            cwp->dotOffset = (meUShort) ((size_t) ss - (size_t) lp->text - 1);
                            do {
                                if(findfence('#',forwFlag,depth) <= 0)
                                    return 0;
                            } while(meLineGetChar(cwp->dotLine,cwp->dotOffset+2) != 'n');
                        }
                        /* skip the hash line itself, and if line has a \ then next etc */
                        inq = 0;
                        lc = ' ';
                        do
                        {
                            if((ch == '"') && (lc != '\\') && !(inq & 0x2))
                                inq ^= 1;
                            else if((ch == '*') && (lc == '/') && !inq)
                                inq = 2;
                            else if((ch == '/') && (lc == '*') && (inq == 2))
                                inq = 0;
                            else if(!inq)
                            {
                                /* Exit if we're at a c++ comment */
                                if((ch == '/') && (lc == '/'))
                                    ss = lp->text + meLineGetLength(lp);
                                /* Exit if we're in a hash define and we're at a '\' */
                                else if((ch == '\\') && (lc != '\\') && (lc != '\''))
                                {
                                    if((lp = meLineGetNext(cwp->dotLine)) == cwp->buffer->baseLine)
                                        return 0;
                                    cwp->dotLine = lp;
                                    cwp->dotLineNo++;
                                    ss=lp->text;
                                    ch = ' ';
                                }
                            }
                            lc = ch;
                        } while((ch = *ss++) != '\0');
                        cwp->dotOffset = meLineGetLength(cwp->dotLine);
                        continue;
                    }
                    cwp->dotOffset = (meUShort) ((size_t) ss - (size_t) lp->text - 1);
                    ch = meLineGetChar(cwp->dotLine,cwp->dotOffset);
                    break;
                }
                else
                {
                    cwp->dotOffset++;
                    ch = meLineGetChar(cwp->dotLine,cwp->dotOffset);
                    if(ch == '\\')
                    {
                        if(!((*flags) & MTNW_KNOWN))
                            *flags |= MTNW_KNOWN;
                        if(!((*flags) & MTNW_NOTINHASH))
                            continue;
                    }
                    break;
                }
            }

        }
        else
        {
            if(cwp->dotOffset == 0)
            {
                for(;;)
                {
                    int comCont;
                    meUByte *fss, lns;
newline_skip:
                    /* Go back lines until we find a line which
                     * 1) Doesn't start with a '#'
                     * 2) Has some text on it
                     * Move cursor to end or if we find a c++ "//" comment
                     * then to the char before that
                     */
                    if((lp = meLineGetPrev(cwp->dotLine)) == cwp->buffer->baseLine)
                        return 0;
                    cwp->dotLine = lp;
                    cwp->dotLineNo--;
                    ss=lp->text;
                    while((ch = *ss++) != '\0')
                        if((ch != ' ') && (ch != '\t'))
                            break;
                    fss = ss;
                    comCont = 0;
                    if(ch == '\0')
                        ss = lp->text;
                    else if((*flags) & MTNW_SIMPLE)
                        ss = lp->text + meLineGetLength(lp) + 1;
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
                                cwp->dotOffset = (meUShort) ((size_t) ss - (size_t) lp->text - 1);
                                if(findfence('#',forwFlag,depth) <= 0)
                                    return 0;
                            }
                            /* skip the hash line itself */
                            continue;
                        }
                        inq = 0;
                        lc = ' ';
                        do
                        {
                            if((ch != ' ') && (ch != '\t'))
                            {
                                lns = ch;
                                if((ch == '"') && (lc != '\\') && (comCont <= 0))
                                    inq ^= 1;
                                else if((ch == '*') && (lc == '/') && !inq)
                                {
                                    /* change ch from '*' to another bogus char, this stops / * / / /
                                     * being misinterpreted */
                                    comCont++;
                                    ch = 0x01;
                                }
                                else if(!inq && (ch == '/'))
                                {
                                    if(lc == '*')
                                    {
                                        /* change ch from '/' to another bogus char, this stops the double '/'
                                         * created at a stop start c comment being interpreted as a c++ comment */
                                        comCont--;
                                        ch = 0x01;
                                    }
                                    else if((lc == '/') && (comCont <= 0))
                                    {
                                        /* Exit if we're at a c++ comment */
                                        ss--;
                                        break;
                                    }
                                }
                            }
                            lc = ch;
                        } while((ch = *ss++) != '\0');

                        if(lns == '\\')
                        {
                            /* For the line to end with a '\' we must be in a #define - flag we're in a HASH and move to the char before '\' */
                            if(!((*flags) & MTNW_KNOWN))
                                *flags |= MTNW_KNOWN;
                            else if((*flags) & MTNW_NOTINHASH)
                            {
                                /* part of a #define \ line, skip the line */
                                *flags &= ~MTNW_SKIPHASH;
                                goto newline_skip;
                            }
                            while(*--ss != '\\')
                                ;
                            ss++;
                        }
                    }

                    if(!((*flags) & MTNW_KNOWN))
                    {
                        *flags |= MTNW_KNOWN|MTNW_NOTINHASH;
                        if(fss[-1] == '#')
                        {
                            ss = fss;
                            goto hash_skip;
                        }
                    }
                    if((*flags == (MTNW_NOTINHASH|MTNW_KNOWN)) && (comCont >= 0))
                    {

                        /* If not in a hash and the current line has no
                         * mismatched close comment, we must check the
                         * previous line to see if it has a trailing '\'. If
                         * so then this line is part of a hash define, so we
                         * must skip to the line before the # */
                        meUByte tflags;
                        long lineNo=cwp->dotLineNo;

                        cwp->dotOffset = 0;
                        tflags = *flags | MTNW_SKIPHASH;
                        if(((ch=moveToNonWhite(forwFlag,&tflags,depth)) != 0) &&
                           !(tflags & MTNW_SKIPHASH))
                            return ch;
                        cwp->dotLine = lp;
                        cwp->dotLineNo = lineNo;
                    }
                    if((inq = ((size_t) ss - (size_t) lp->text) - 2) >= 0)
                    {
                        cwp->dotOffset = inq;
                        break;
                    }
                }
            }
            else
                cwp->dotOffset--;
            ch = meLineGetChar(cwp->dotLine,cwp->dotOffset);
        }
        if(!isSpace(ch))
        {
            if(!((*flags) & (MTNW_INCOM|MTNW_SIMPLE)) && (ch == '/'))
            {
                if(forwFlag)
                {
                    register meUByte c2 = meLineGetChar(cwp->dotLine, cwp->dotOffset+1);
                    if(c2 == '*')
                    {   /* c comment, go to the end of it */
                        if(findfence('*',forwFlag,depth) <= 0)
                            return 0;
                        cwp->dotOffset++;
                        continue;
                    }
                    else if(c2 == '/')
                    {   /* c++ comment, go to the end of the line */
                        cwp->dotOffset = meLineGetLength(cwp->dotLine);
                        continue;
                    }
                }
                else
                {
                    /* are we at the start of a comment?? if so skip it.
                     * One gotcha is a comment of the form / * / / / / / / / */
                    if((cwp->dotOffset > 0) &&
                       (meLineGetChar(cwp->dotLine, cwp->dotOffset-1) == '*') &&
                       ((cwp->dotOffset == 1) ||
                        (meLineGetChar(cwp->dotLine, cwp->dotOffset-2) != '/')))
                    {
                        /* must move back one into the comment */
                        cwp->dotOffset--;
                        if(findfence('/',forwFlag,depth) <= 0)
                            return 0;
                        /* ZZZZ - note that at this point we should check for
                         * a # at the start of the line and do the right
                         * stuff
                         */
                        continue;
                    }
                }
            }
            return ch;
        }
    }
}

static int
findQuoteFence(meUByte qtype, meUByte flags, meUByte forwFlag)
{
    meWindow *cwp=frameCur->windowCur;
    meUByte c;			/* current character in scan */
    meLine *comlp;	        /* comment line pointer */
    meUShort comoff;	        /* and offset */
    meInt comlno;               /* and line-no */
    int comStt=0;               /* and status */

    /* scan until we find it, or reach the end of file */
    for(;;)
    {
        /* windowBackwardChar & windowForwardChar goes 1 character past the end of the
         * line, therfore want to omit this character in the search.
         * Check for end of line, and go back another character if true.
         */
        if(forwFlag)
        {
            if(cwp->dotOffset < meLineGetLength(cwp->dotLine))
                (cwp->dotOffset)++;
            else if(flags & MTNW_SIMPLE)
                return meFALSE;
            else
            {
                register meLine *lp;
                register meInt ii=0;
                lp = cwp->dotLine;
                do {
                    if((lp = meLineGetNext(lp)) == cwp->buffer->baseLine)
                        return meFALSE;
                    ii++;
                } while(meLineGetLength(lp) == 0);
                cwp->dotLine = lp;
                cwp->dotOffset = 0;
                cwp->dotLineNo += ii;
            }
        }
        else
        {
            if(cwp->dotOffset > 0)
                (cwp->dotOffset)--;
            else if(flags & MTNW_SIMPLE)
                return meFALSE;
            else
            {
                register meLine *lp;
                register meInt ii=0;
                if(comStt > 0)
                    /* if we have found an open comment it is
                     * more likely that the quote is part of a comment
                     * than the open comment being part of a multi-
                     * line string, quit
                     */
                    break;
                lp = cwp->dotLine;
                do {
                    if((lp = meLineGetPrev(lp)) == cwp->buffer->baseLine)
                        return meFALSE;
                    ii++;
                } while(meLineGetLength(lp) == 0);
                cwp->dotLine = lp;
                cwp->dotOffset = meLineGetLength(lp) - 1;
                cwp->dotLineNo -= ii;
            }
        }
        c=meLineGetChar(cwp->dotLine,cwp->dotOffset);
        if(c == qtype)
        {
            /* must count the '\'s before the character to
             * check whether its the close or just quoted in
             * a string, e.g. "\\\"" etc.
             */
            register meInt ii=cwp->dotOffset;
            while((--ii > 0) && (meLineGetChar(cwp->dotLine,ii) == '\\'))
                ;
            /* if its an odd number then its a quote (note that ii is over decr)  */
            if((((int) cwp->dotOffset) - ii) & 0x01)
                return meTRUE;
        }
        if(c == '*')
        {
            if(!comStt && !forwFlag && (meLineGetChar(cwp->dotLine, cwp->dotOffset+1) == '/'))
                comStt = -1;
        }
        else if(c == '/')
        {
            if(!comStt && !forwFlag && (meLineGetChar(cwp->dotLine, cwp->dotOffset+1) == '*'))
            {
                /* save the comment cursor position */
                comlp  = cwp->dotLine;
                comoff = cwp->dotOffset+2;
                comlno = cwp->dotLineNo;
                comStt = 1;
            }
        }
        else if((c == '{') && (cwp->dotOffset == 0))
            break;
    }
    if(comStt <= 0)
        return meFALSE;
    cwp->dotLine = comlp;
    cwp->dotOffset = comoff;
    cwp->dotLineNo = comlno;
    return -1;
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
findfence(meUByte ch, meUByte forwFlag, meInt depth)
{
    meWindow *cwp;
    meUByte inCom, cc;
    int inAps;

    /* safe-guard ME crash caused by too many fences creating recursion nightmare */
    if(++depth > 255)
        return mlwrite(MWABORT,(meUByte *)"[Too many nested fences]");
    /* Separate ', ' & # cases as we can really optimise these */
    if((ch == '"') || (ch == '\''))
        return findQuoteFence(ch,0,forwFlag);
    cwp = frameCur->windowCur;
    if(ch == '#')
    {
        register meLine *lp;
        register meInt ii=0;
        meUByte *ss;
        lp = cwp->dotLine;
        if(forwFlag)
        {
            for(;;)
            {
                if((lp = meLineGetNext(lp)) == cwp->buffer->baseLine)
                    return 0;
                ii++;
                ss=lp->text;
                while(((cc = *ss++) != '\0') && ((cc == ' ') || (cc == '\t')))
                    ;
                if(cc == '#')
                {
                    cwp->dotLine = lp;
                    cwp->dotOffset = (meUShort) ((size_t) ss - (size_t) lp->text - 1);
                    cwp->dotLineNo += ii;
                    if((ss[0] == 'e') && ((ss[1] == 'l') || (ss[1] == 'n')))
                        break;
                    else if((ss[0] == 'i') && (ss[1] == 'f'))
                    {
                        do {
                            if(findfence('#',forwFlag,depth) <= 0)
                                return 0;
                            /* findfence can only succeed with a line containing
                             * #e[nl]
                             */
                            ss = cwp->dotLine->text;
                            while((cc = *ss++) != 'e')
                                ;
                        } while(*ss != 'n');
                    }
                    lp = cwp->dotLine;
                    ii = 0;
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
                if((lp = meLineGetPrev(lp)) == cwp->buffer->baseLine)
                    return 0;
                ii++;
                ss=lp->text;
                while(((cc = *ss++) != '\0') &&
                      ((cc == ' ') || (cc == '\t')))
                    ;
                if(cc == '#')
                {
                    cwp->dotLine = lp;
                    cwp->dotOffset = (meUShort) ((size_t) ss - (size_t) lp->text - 1);
                    cwp->dotLineNo -= ii;
                    if((ss[0] == 'i') && (ss[1] == 'f'))
                        break;
                    else if((ss[0] == 'e') && (ss[1] == 'n') &&
                            (findfence('#',forwFlag,depth) <= 0))
                        return 0;
                    lp = cwp->dotLine;
                    ii = 0;
                }
            }
        }
        return 1;
    }
    /* scan until we find it, or reach the end of file */
    /* check the previous char, if its a ' then we could be in a quote */
    if((cwp->dotOffset > 0) && (meLineGetChar(cwp->dotLine, cwp->dotOffset-1) == '\''))
        inAps = -1;
    else
        inAps = 0;
    if((ch == '*') || (ch == '/'))
    {
        inCom = MTNW_INCOM|MTNW_KNOWN;
        if(meLineGetChar(cwp->dotLine, cwp->dotOffset) == '/')
            cwp->dotOffset += (forwFlag) ? 1:-1;
    }
#if MEOPT_HILIGHT
    else if(cwp->buffer->indent && (meHilightGetFlags(indents[cwp->buffer->indent]) & HICMODE))
        inCom = 0;
#endif
    else
        inCom = MTNW_SIMPLE|MTNW_KNOWN;

    for(;;)
    {
        if((cc=moveToNonWhite(forwFlag,&inCom,depth)) == 0)
            break;
        if(cc == '/')
        {
            cc = meLineGetChar(cwp->dotLine, cwp->dotOffset+1);
            if(cc != '*')
                continue;
            if(ch == '/')
                return 1;
            if(!forwFlag)
                /* we've found a double comment, or we're in a comment,
                 * assume the latter.
                 */
                return 2;
            if(findfence('*',forwFlag,depth) <= 0)
                break;
        }
        else if(cc == '*')
        {
            if(meLineGetChar(cwp->dotLine, cwp->dotOffset+1) != '/')
                continue;
            if(ch == '*')
                return 1;
            /* one possible gotcha here is a / * / / / / * / comment - check for it */
            if((ch == '/') && cwp->dotOffset && (meLineGetChar(cwp->dotLine, cwp->dotOffset-1) == '/'))
                continue;
            if(forwFlag)
                /* we've found a double comment, try to report the error */
                break;
            if(findfence('/',forwFlag,depth) <= 0)
                break;
        }
        else if(ch == cc)
            return 1;
        else if(!(inCom & MTNW_INCOM))
        {
            meUByte *ss;

            /* If we're looking for a bracket then check for a bracket,
             * i.e. a ",',(,),[,],{,}
             */
            if((cc == '"') || (cc == '\''))
            {
                meLine *qlp;
                meUShort qoff;
                meInt qlno;

                qlp  = cwp->dotLine;
                qoff = cwp->dotOffset;
                qlno = cwp->dotLineNo;
                if(findQuoteFence(cc,inCom,forwFlag) == meFALSE)
                {

                    if(!(inCom & MTNW_SIMPLE))
                        return -1;
                    /* in a simple buffer (not C) if we failed to find the
                     * end quote on hte same line then it is more likely this
                     * was a single quote, i.e it's like that. Handle them by
                     * ignoring them */
                    cwp->dotLine = qlp;
                    cwp->dotOffset = qoff;
                    cwp->dotLineNo = qlno;
                }
            }
            else if((ss = meStrchr(fenceString+meFENCE_BRACKET_OFFSET,cc)) != NULL)
            {
                int ii=ss-fenceString;

                /* check for an oposite direction bracket - i.e. a ')' when going forward
                 * If so return a failure
                 */
                if((ii & 1) != forwFlag)
                    return 0;
                /* This is a same direction bracket - i.e. a '(' when going forward
                 * If so find the other side of the bracket (using findfence)
                 */
                if((ii=findfence(fenceString[ii^1],forwFlag,depth)) <= 0)
                {
                    if(!inAps)
                        inAps = ii;
                    break;
                }
                else if((ii == 2) && (ch != '/'))
                    /* we're in a comment going backwards - return 2 unless we are currently looking for this end */
                    return 2;
            }
        }
    }
    return inAps;
}


/* goto-matching-fence. move cursor to a matching fence. Normally bound to
 * esc C-i */
int
gotoFence(int f, int n)
{
    meWindow *cwp=frameCur->windowCur;
    meLine  *oldlp;	/* original line pointer */
    meUShort oldoff;	/* and offset */
    long oldlno;	/* and line-no */
    long oldtln;	/* The window top line-no */
    int ret=meFALSE;	/* return value */
    meUByte ch;	/* open fence */
    meUByte *ss;        /* fenceSting pointer */

    /* save the original cursor position */
    oldlp  = cwp->dotLine;
    oldoff = cwp->dotOffset;
    oldlno = cwp->dotLineNo;
    oldtln = cwp->vertScroll;
    ch = meLineGetChar(oldlp, oldoff);

    /* Check the current char is a valid fence char, if not do nothing */
    if((ch != '\0') && ((ss=meStrchr(fenceString,ch)) != NULL))
    {
        register meUByte forwFlag=1;  /* movement direction */

        if(ch == '*')
        {
            /* if its a comment fence, default is forward, but check for reverse */
            if(meLineGetChar(oldlp, oldoff+1) == '/')
            {
                forwFlag = 0;
                ch = '/';
            }
        }
        else if(ch == '/')
        {
            if(oldoff && meLineGetChar(oldlp, oldoff-1) == '*')
            {
                /* must move back into the comment as well, or we will end up in the wrong place */
                forwFlag = 0;
                cwp->dotOffset--;
            }
            else
                ch = '*';
        }
        else if(ch == '#')
        {
            /* this is a #if, #elif, #else, #endif fence
             * workout which and the direction
             */
            if(meLineGetChar(oldlp, oldoff+1) == 'e')
            {
                if(meLineGetChar(oldlp, oldoff+2) == 'n')
                    /* a #endif, go backwards */
                    forwFlag = 0;
                else if(meLineGetChar(oldlp, oldoff+2) != 'l')
                    goto exit_fence;
            }
            else if((meLineGetChar(oldlp, oldoff+1) != 'i') ||
                    (meLineGetChar(oldlp, oldoff+2) != 'f'))
                goto exit_fence;
        }
        else if((ch == '"') || (ch == '\''))
        {
            /* rely on the numeric arg to determine direction */
            if(n & 4)
                forwFlag = 0;
        }
        else
        {
            /* this is a simple bracket fence, i.e. (,),{,},[,]
             * the direction and close ch can be calculated
             */
            forwFlag = ss-fenceString;
            ch = fenceString[forwFlag^1];
            forwFlag &= 1;
        }
        /* return must be 1 for match, 2 == in comment */
        if(findfence(ch,forwFlag,0) == 1)
        {
            cwp->updateFlags |= WFMOVEL;
            /* if 2nd bit not set then we want to stay here so simply
             * return at theis point
             */
            if((n & 2) == 0)
                return meTRUE;
            /* We just want to show the location so, update, delay
             * then move back
             */
            update(meFALSE);
            TTsleep(pauseTime,1,NULL);
            cwp->updateFlags |= WFMOVEL;
            if(cwp->vertScroll != oldtln)
                /* the redraw has changed the top line - must do a major update */
                cwp->updateFlags |= (WFREDRAW|WFSBOX);
            ret = meTRUE;
        }
        /* restore the current position */
        cwp->dotLine  = oldlp;
        cwp->dotOffset  = oldoff;
        cwp->dotLineNo = oldlno;
        cwp->vertScroll = oldtln;
    }

exit_fence:
    if((ret == meFALSE) && (n & 1))
        TTbell();
    return ret;
}

#endif /* MEOPT_FENCE */

#if MEOPT_HILIGHT

static int
prevCToken(meWindow *cwp,meUByte *token, int size)
{
    register int offset;

    if(((offset = cwp->dotOffset - size + 1) < 0) ||
       ((offset > 0) && !isSpace(cwp->dotLine->text[offset-1])))
        return 0;
    return !meStrncmp(cwp->dotLine->text+offset,token,size);
}

static int
getCoffset(meHilight *indentDef, int onBrace, int *inComment)
{
    meWindow *cwp=frameCur->windowCur;
    meLine *oldlp;
    meLine *lp, *blp=NULL;
    long lno;
    int ii, normCont=1, brakCont=0, indent=0, gotsome=0;
    meUShort off;
    meUByte cc, mtnwFlag=0;

    *inComment = 0;
    oldlp = cwp->dotLine;
    cwp->dotOffset = 0;
    /* scan until we find it, or reach the end of file */
    while(!indent)
    {
        switch((cc=moveToNonWhite(0,&mtnwFlag,0)))
        {
        case 0:
            if(!(meIndentGetFlags(indentDef)&HICINDTPLV))
            {
                if(brakCont < 0)
                    return -brakCont;
                return 0;
            }
            if(!gotsome)
                normCont &= ~1;
            indent = 1;
            break;

        case ')':
            cc = '(';
            goto find_bracket_fence;
        case ']':
            cc = '[';
            goto find_bracket_fence;
        case '}':
            cc = '{';
find_bracket_fence:
            if((ii=findfence(cc,0,0)) <= 0)
                return -1;
            if(ii == 2)
            {
                /* in a comment */
                *inComment = 1;
                return getwcol(cwp);
            }
            if(cc == '{')
            {
                ii = getwcol(cwp);
                if((ii == 0) || (ii < meIndentGetBraceIndent(indentDef,cwp->buffer)))
                {
                    if(brakCont < 0)
                        return -brakCont;
                    return 0;
                }
                if(!gotsome)
                    normCont &= ~1;
            }
            break;
        case '{':
            lp = cwp->dotLine;
            lno = cwp->dotLineNo;
            off = cwp->dotOffset;
            if((cc=moveToNonWhite(0,&mtnwFlag,0)) != 0)
            {
                if(cc == ')')
                {
                    meLine *tlp=cwp->dotLine;
                    if((ii=findfence('(',0,0)) <= 0)
                        return -1;
                    if(ii == 2)
                    {
                        /* in a comment */
                        *inComment = 1;
                        return getwcol(cwp);
                    }
                    if(tlp == lp)
                    {
                        lp  = cwp->dotLine;
                        lno = cwp->dotLineNo;
                        off = cwp->dotOffset;
                    }
                    if(moveToNonWhite(0,&mtnwFlag,0) != 0)
                    {
                        if(!brakCont && prevCToken(cwp,(meUByte *)"switch",6))
                            /* we found a switch */
                            indent = -1;
                        else if(normCont && prevCToken(cwp,(meUByte *)"function",8))
                            normCont = 3;
                    }
                }
                else if(cc == '=')
                    normCont &= ~1;
                else if((cc == 'm') && (cwp->dotOffset >= 3) &&
                        !meStrncmp(cwp->dotLine->text+cwp->dotOffset-3,"enum",4))
                {
                    indent = 3;
                    normCont &= ~1;
                }
                else
                {
                    cwp->dotOffset = 0;
                    if((cc = gotoFrstNonWhite()) == 'e')
                    {
                        if(!meStrncmp(cwp->dotLine->text+cwp->dotOffset,"extern \"C\"",10))
                            indent = 2;
                    }
                    else if((cc == 't') &&
                            !meStrncmp(cwp->dotLine->text+cwp->dotOffset,"typedef",7) &&
                            ((cwp->dotOffset+=7),((cc = gotoFrstNonWhite()) == 'e')) &&
                            !meStrncmp(cwp->dotLine->text+cwp->dotOffset,"enum",4))
                    {
                        indent = 3;
                        normCont &= ~1;
                    }
                    else if((cc == 'n') &&
                            !meStrncmp(cwp->dotLine->text+cwp->dotOffset,"namespace",9))
                        indent = 2;
                    else if(!brakCont)
                        blp = cwp->dotLine;
                    if(indent)
                        off = cwp->dotOffset;
                }
            }
            cwp->dotLine = lp;
            cwp->dotLineNo = lno;
            cwp->dotOffset = 0;
            cc = gotoFrstNonWhite();
            ii = getwcol(cwp);
            if(!gotsome && (normCont & 1))
                normCont = 0;
            if(!brakCont)
            {
                if(indent == 2)
                    brakCont = ii;
                else
                {
                    /* special cases, if the '{' is on left hand side
                     * indent lines by statement unless we are indenting
                     * the closing '}' in which case return 0 */
                    if(ii != 0)
                        brakCont = ii + meIndentGetBraceStatementIndent(indentDef,cwp->buffer);
                    else if(onBrace > 0)
                        brakCont = 0;
                    else
                    {
                        brakCont = meIndentGetStatementIndent(indentDef,cwp->buffer);
                        if(!onBrace && (blp != NULL))
                        {
                            meUByte *ss = blp->text;
                            while(((ss = meStrstr(ss,"class")) != NULL) &&
                                  (!isSpace(ss[5]) || ((ss != blp->text) && !isSpace(ss[-1]))))
                                ss += 5;
                            if(ss != NULL)
                            {
                                brakCont = 0-brakCont;
                                indent = 1;
                                break;
                            }
                        }
                    }
                    if(!onBrace && indent == -1)
                        brakCont += meIndentGetSwitchIndent(indentDef,cwp->buffer);

                    if(brakCont < 0)
                        brakCont = 0;
                }
                blp = cwp->dotLine;
            }
            else if((blp == lp) && (brakCont > 0))
                brakCont += meIndentGetStatementIndent(indentDef,cwp->buffer);
            cwp->dotOffset = off;
            if(ii <= meIndentGetBraceIndent(indentDef,cwp->buffer))
                indent = 1;
            else if(indent < 0)
                indent = 0;
            break;
        case '#':
            if((mtnwFlag == MTNW_KNOWN) && (cwp->dotOffset == 0))
                indent = 1;
            break;
        case '/':
            if(meLineGetChar(cwp->dotLine,cwp->dotOffset+1) == '*')
            {
                /* in a comment */
                *inComment = 1;
                return getwcol(cwp);
            }
            break;
        case '(':
        case '[':
            if(brakCont == 0)
            {
                /* ignore tabs here cos they will screw up anyway */
                off = cwp->dotOffset;
                while(((cc=meLineGetChar(cwp->dotLine,++(cwp->dotOffset))) == ' ') || (cc == '\t'))
                    ;
		brakCont = getwcol(cwp);
                if((meIndentGetContinueMax(indentDef,cwp->buffer) > 0) && (brakCont > meIndentGetContinueMax(indentDef,cwp->buffer)))
                {
                    cwp->dotOffset = 0;
                    gotoFrstNonWhite();
                    ii = getwcol(cwp) + meIndentGetContinueMax(indentDef,cwp->buffer);
                    if(ii < brakCont)
                        brakCont = ii;
                }
                brakCont = 0-brakCont;
                cwp->dotOffset = off;
            }
            else if((cwp->dotLine == blp) && (brakCont > 0))
            {
                off = cwp->dotOffset;
                while(((cc=meLineGetChar(cwp->dotLine,++(cwp->dotOffset))) == ' ') || (cc == '\t'))
                    ;
		ii = getwcol(cwp);
                cwp->dotOffset = 0;
                gotoFrstNonWhite();
                ii -= getwcol(cwp);
                if((meIndentGetContinueMax(indentDef,cwp->buffer) > 0) && (ii > meIndentGetContinueMax(indentDef,cwp->buffer)))
                    ii = meIndentGetContinueMax(indentDef,cwp->buffer);
                brakCont += ii;
                cwp->dotOffset = off;
            }
            break;
        case ';' :
        case ':' :
            if(!gotsome)
                normCont &= ~1;
            break;
        case '\'':
        case '"' :
            /* only return if findQuoteFence returns meFALSE, returns
             * -1 if we could be in a comment */
            if(findQuoteFence(cc,mtnwFlag,0) == meFALSE)
                return -1;
            break;
        default:
            if(onBrace > 1)
            {
                if(cc == 'e')
                {
                    if(((cwp->dotOffset == 0) || isSpace(meLineGetChar(cwp->dotLine, cwp->dotOffset-1))) &&
                       !meStrncmp(cwp->dotLine->text+cwp->dotOffset,"else",4) &&
                       (isSpace(meLineGetChar(cwp->dotLine, cwp->dotOffset+4)) ||
                        (meLineGetChar(cwp->dotLine, cwp->dotOffset+2) == '{')))
                        onBrace++;
                }
                else if(cc == 'i')
                {
                    if(((cwp->dotOffset == 0) || isSpace(meLineGetChar(cwp->dotLine,cwp->dotOffset-1))) &&
                       (cwp->dotLine->text[cwp->dotOffset+1] == 'f') &&
                       (isSpace(meLineGetChar(cwp->dotLine,cwp->dotOffset+2)) ||
                        (meLineGetChar(cwp->dotLine,cwp->dotOffset+2) == '(')) &&
                       (--onBrace == 1))
                    {
                        lp = cwp->dotLine;
                        lno = cwp->dotLineNo;
                        off = cwp->dotOffset;
                        if(((cc=moveToNonWhite(0,&mtnwFlag,0)) != 'e') || !prevCToken(cwp,(meUByte *)"else",4))
                        {
                            cwp->dotLine = lp;
                            cwp->dotLineNo = lno;
                        }
                        else
                            off = cwp->dotOffset-3;
                        cwp->dotOffset = 0;
                        gotoFrstNonWhite();
                        brakCont = getwcol(cwp);
                        cwp->dotOffset = off;
                        onBrace = 0;
                    }
                }
            }
        }
        if(!gotsome)
        {
            if((cc == '=') && onBrace < 0)
                normCont &= ~1;
            gotsome = 1;
        }
    }

    if(brakCont < 0)
        return -brakCont;
    if(normCont)
    {
        cwp->dotLine =	oldlp;
        cwp->dotOffset = 0;
        if(moveToNonWhite(0,&mtnwFlag,0) == ')')
        {
            if(findfence('(',0,0) <= 0)
                return -1;
            moveToNonWhite(0,&mtnwFlag,0);
            if(prevCToken(cwp,(meUByte *)"if",2))
            {
                lp = cwp->dotLine;
                lno = cwp->dotLineNo;
                cwp->dotOffset--;
                if(((cc=moveToNonWhite(0,&mtnwFlag,0)) != 'e') || !prevCToken(cwp,(meUByte *)"else",4))
                {
                    cwp->dotLine = lp;
                    cwp->dotLineNo = lno;
                }
                cwp->dotOffset = 0;
                gotoFrstNonWhite();
                brakCont = getwcol(cwp)+meIndentGetStatementIndent(indentDef,cwp->buffer);
            }
            else if(prevCToken(cwp,(meUByte *)"for",3) || prevCToken(cwp,(meUByte *)"while",5) || prevCToken(cwp,(meUByte *)"switch",6))
            {
                cwp->dotOffset = 0;
                gotoFrstNonWhite();
                brakCont = getwcol(cwp)+meIndentGetStatementIndent(indentDef,cwp->buffer);
            }
            else
            {
                if(onBrace < 0)
                    brakCont += meIndentGetStatementIndent(indentDef,cwp->buffer);
                else if(brakCont)
                    brakCont += meIndentGetContinueIndent(indentDef,cwp->buffer);
            }
        }
        else if(!prevCToken(cwp,(meUByte *)"do",2) && !prevCToken(cwp,(meUByte *)"else",4))
        {
            if(onBrace < 0)
                brakCont += meIndentGetStatementIndent(indentDef,cwp->buffer);
            else if(brakCont || (meIndentGetFlags(indentDef)&HICINDTPLV))
                brakCont += meIndentGetContinueIndent(indentDef,cwp->buffer);
        }
        else
        {
            cwp->dotOffset = 0;
            gotoFrstNonWhite();
            brakCont = getwcol(cwp)+meIndentGetStatementIndent(indentDef,cwp->buffer);
        }
    }
    else if(onBrace < 0)
        /* this is a new context brase (i.e. one not following an if or else
         * etc. used to define a new variable in mid function. The next line
         * should be indented by a Statement not (Brace + BraceStatement) */
        brakCont += (meIndentGetStatementIndent(indentDef,cwp->buffer) << 1) -
          meIndentGetBraceIndent(indentDef,cwp->buffer) - meIndentGetBraceStatementIndent(indentDef,cwp->buffer);
    return brakCont;
}


/* do C indent, returns -ve if failed (god knows how) the indent on success. */
int
doCindent(meHilight *indentDef, int *inComment)
{
    meWindow *cwp=frameCur->windowCur;
    meLine *oldlp;
    int   ind=1, curOff, curInd, curPos, cc, ii;
    int   addInd=0, comInd=3, onBrace=0;
    long  lineno;

    /* save the original cursor position */
    oldlp = cwp->dotLine;
    lineno = cwp->dotLineNo;
    curPos = cwp->dotOffset;
    cwp->dotOffset = 0;
    cc = gotoFrstNonWhite();
    curOff = cwp->dotOffset;
    curInd = getwcol(cwp);
    /* change the current position to the indent position if to the left */
    if(curPos < curOff)
        curPos = curOff;
    if(isAlpha(cc))
    {
        if((cc == 'c') &&
           !meStrncmp(cwp->dotLine->text+cwp->dotOffset,"case",4) &&
           !isAlpha(cwp->dotLine->text[cwp->dotOffset+4]))
            addInd += meIndentGetCaseIndent(indentDef,cwp->buffer);
        else if((cc == 'e') &&
                !meStrncmp(cwp->dotLine->text+cwp->dotOffset,"else",4) &&
                (!isAlpha(cwp->dotLine->text[cwp->dotOffset+4]) ||
                 (cwp->dotLine->text[cwp->dotOffset+4] == '{')))
            onBrace = 2;
        else if((cc == 'd') &&
                !meStrncmp(cwp->dotLine->text+cwp->dotOffset,"default",7) &&
                !isWord(cwp->dotLine->text[cwp->dotOffset+7]))
            addInd += meIndentGetCaseIndent(indentDef,cwp->buffer);
        else if((cc == 'p') &&
                ((ii=7,!meStrncmp(cwp->dotLine->text+cwp->dotOffset,"private",ii)) ||
                 (ii=6,!meStrncmp(cwp->dotLine->text+cwp->dotOffset,"public",ii)) ||
                 (ii=9,!meStrncmp(cwp->dotLine->text+cwp->dotOffset,"protected",ii))) &&
                !isAlpha(cwp->dotLine->text[cwp->dotOffset+ii]))
        {
            ii += cwp->dotOffset;
            while(((cc = meLineGetChar(cwp->dotLine,ii++)) == ' ') || (cc == '\t'))
                ;
            /* Special test for private/public/protected with a trailing
             * keyword such as "public slots:" as used in Qt */
            if (isWord (cc))
            {
                /* Consume the word */
                while(isWord (cc))
                    cc = meLineGetChar(cwp->dotLine,ii++);
                /* Consume any trailing white space */
                while ((cc == ' ') || (cc == '\t'))
                    cc = meLineGetChar(cwp->dotLine,ii++);
            }
            if(cc == ':')
                addInd -= meIndentGetStatementIndent(indentDef,cwp->buffer);
        }
        else
        {
            /* check to see if its a lable */
            ii = curOff;
            do
                cc = meLineGetChar(cwp->dotLine,++ii);
            while(isAlphaNum(cc) || (cc == '_'));

            while((cc == ' ') || (cc == '\t'))
                cc = meLineGetChar(cwp->dotLine,++ii);

            if(cc == ':')
            {
                cc = meLineGetChar(cwp->dotLine,++ii);
                if(isSpace(cc) && ((addInd = meIndentGetLabelIndent(indentDef,cwp->buffer)) < -65535))
                    ind = 0;
            }
        }
    }
    else if(cc == '#')
    {
        *inComment = 0;
        ind = 0;
    }
    else if(cc == '*')
    {
        comInd = 1;
        if((cc=meLineGetChar(cwp->dotLine,cwp->dotOffset+1)) == '*')
        {
            if(meLineGetChar(cwp->dotLine,cwp->dotOffset+2) == '*')
                goto use_prev_line;
            comInd = 0;
        }
        else if(cc == '/')
        {
use_prev_line:
            cwp->dotLine = meLineGetPrev(cwp->dotLine);
            cwp->dotOffset = 0;
            if(((cc=gotoFrstNonWhite()) == '*') &&
               (meLineGetChar(cwp->dotLine,cwp->dotOffset+1) == '*'))
            {
                if(meLineGetChar(cwp->dotLine,cwp->dotOffset+2) == '*')
                    goto use_contcomm;
                comInd = 0;
            }
            else if(cc == '/')
            {
use_contcomm:
                if((meLineGetChar(cwp->dotLine,cwp->dotOffset+1) == '*') &&
                   (meIndentGetCommentContinue(indentDef) != NULL) &&
                   (meIndentGetCommentContinue(indentDef)[0] == '*') &&
                   (meIndentGetCommentContinue(indentDef)[1] == '*'))
                    comInd = 0;
            }
            cwp->dotLine = oldlp;
            cwp->dotOffset = curOff;
        }
    }
    /* This case is a little strange. The comment restyle for C++ comments
     * aligns comments on-top other in a column when placed at the end of a
     * statement line. i.e.
     *
     * >    statement;                   // blah blah blah blah
     * >                                 // more blah blah blah
     *
     * We want to preserve the alignment of the C++ comment if we find them
     * stacked like this. If this lines start position exactly matches the
     * previous line then we say that the comment position is correct. If the
     * comment is in a different column then we assume it is supposed to be
     * aligned with the statement. i.e
     *
     * >    statement;                   // blah blah blah blah
     * >                                 // more blah blah blah
     * >    // Statement aligned comment.
     *
     * So restyling the above should not change the position of the comments.
     */
    else if ((cc == '/') &&
             (((ii=meLineGetChar(cwp->dotLine,cwp->dotOffset+1)) == '/') ||
              (ii == '*')))
    {
        /* We are in a C++ comment, check the previous line to see if this
         * comments is aligned with the previous line, if it is then we must
         * keep the same alignment. */
        cwp->dotLine = meLineGetPrev(cwp->dotLine);
        cwp->dotOffset = 0;
        if(setccol(cwp,curInd))             /* Move to position on prev line */
        {
            /* Checkout the character. Note that we check the previous
             * character to ensure that this is not some sort of block
             * comment which tend to mess things up. */
            if (((cwp->dotOffset == 0) ||
                 (meLineGetChar(cwp->dotLine,cwp->dotOffset-1) != '/')) &&
                (meLineGetChar(cwp->dotLine,cwp->dotOffset+0) == '/') &&
                (meLineGetChar(cwp->dotLine,cwp->dotOffset+1) == ii))
            {
                /* Matched our position with the current position, use this as the indent. */
                if((ii = getCoffset(indentDef,onBrace,inComment)) < 0)
                    ii = 0;
                addInd = curInd - ii;
                if (addInd < 0)
                    /* Correct bad return ! */
                    addInd = 0;
            }
        }
        cwp->dotLine = oldlp;
        cwp->dotOffset = curOff;
    }
    else if(cc == '{')
    {
        addInd += meIndentGetBraceIndent(indentDef,cwp->buffer) - meIndentGetStatementIndent(indentDef,cwp->buffer);
        onBrace = -1;
    }
    else if(cc == '}')
    {
        addInd -= meIndentGetBraceStatementIndent(indentDef,cwp->buffer);
        onBrace = 1;
    }
    else if((cc == ':') &&
            (meLineGetChar(cwp->dotLine,cwp->dotOffset+1) != ':'))
    {
        /* C++ ':' starting the next line. Indent the current line
         * temporarily. Found in statements such as:-
         *
         * @ AnalogClock::AnalogClock( QWidget *parent, const char *name )
         *       : QWidget( parent, name )
         *   {
         *       ...
         *   }
         */
        addInd += meIndentGetContinueIndent(indentDef,cwp->buffer);
    }
    if(ind)
    {
        ii = getCoffset(indentDef,onBrace,inComment);
        ind = (ii < 0) ? 0:ii;
        if(*inComment)
            ind += comInd;
        else if(((onBrace < 0) && (ii < 0)) || ((ind += addInd) < 0))
            ind = 0;
    }

    cwp->dotLine = oldlp;
    cwp->dotLineNo = lineno;
    cwp->dotOffset = curPos;
    if(curInd == ind)
        return meTRUE;
#if MEOPT_EXTENDED
    if(((ii=meIndentGetChangeFunc(indentDef)) > 0) && (execFuncHidden(0,ii,ind-0x80000000) <= 0))
    {
        meUByte *vv;
        if(((vv=getUsrLclCmdVar((meUByte *)"indent",cmdTable[ii]->varList)) != errorm) &&
           (((ind=meAtoi(vv)) < 0) || (ind == curInd)))
            return meTRUE;
    }
#endif
    return meLineSetIndent(curOff,ind,1);
}

#endif /* MEOPT_HILIGHT */

int
meAnchorDelete(meBuffer *bp, meInt name)
{
    meAnchor *pp=NULL, *aa=bp->anchorList;
    meLineFlag flag;

    while((aa != NULL) && (meAnchorGetName(aa) != name))
    {
        pp = aa;
        aa = meAnchorGetNext(aa);
    }
    if(aa == NULL)
        return meFALSE;
    /* Remove aa from the anchor list */
    if(pp == NULL)
        bp->anchorList = meAnchorGetNext(aa);
    else
        pp->next = meAnchorGetNext(aa);

    flag = meAnchorGetLineFlag(aa);
    meAssert(aa->line->flag & flag);

    /* see if we can remove the anchor flag from the old line */
    pp = bp->anchorList;
    while(pp != NULL)
    {
        if((pp->line == aa->line) && (meAnchorGetLineFlag(pp) == flag))
            break;
        pp = meAnchorGetNext(pp);
    }
    if(pp == NULL)
        aa->line->flag ^= flag;
    meFree(aa);
    return meTRUE;
}

int
meAnchorGet(meBuffer *bp, meInt name)
{
    meAnchor *p = bp->anchorList;

    while(p != NULL)
    {
        if(p->name == name)
        {
            /* found the mark - do the buisness */
            register meLine *plp=p->line, *blp;
            register meInt bln;
#if MEOPT_NARROW
try_again:
#endif
            blp = bp->baseLine;
            if((bp->dotOffset = p->offs) > meLineGetLength(plp))
                bp->dotOffset = meLineGetLength(plp);
            bln = bp->lineCount;
            if(p->frwd)
            {
                do {
                    if((blp = meLineGetNext(blp)) == plp)
                    {
                        bp->dotLine = blp;
                        bp->dotLineNo = bp->lineCount-bln;
                        p->frwd = (bln > bp->dotLineNo);
                        return meTRUE;
                    }
                } while(--bln >= 0);
            }
            else
            {
                do {
                    if(blp == plp)
                    {
                        bp->dotLine = blp;
                        bp->dotLineNo = bln;
                        p->frwd = (bln < (bp->lineCount>>1));
                        return meTRUE;
                    }
                    blp = meLineGetPrev(blp);
                } while(--bln >= 0);
            }
            /* Okay, we failed to find the mark so only chance now is
             * the mark is in a narrow
             */
#if MEOPT_NARROW
            {
                meNarrow *nrrw=bp->narrow;

                while(nrrw != NULL)
                {
                    blp = nrrw->slp;
                    for (;;)
                    {
                        if(blp == plp)
                        {
                            /* We've found the mark in this narrow, remove the
                             * narrow and try again */
                            meBufferRemoveNarrow(bp,nrrw,NULL,0);
                            goto try_again;
                        }
                        if(blp == nrrw->elp)
                            break;
                        blp = meLineGetNext(blp);
                    }
                    nrrw = nrrw->next;
                }
            }
#endif
            break;
        }
        p = p->next;
    }
    /* Failed to find mark - set to the end of file */
    bp->dotLineNo = bp->lineCount;
    bp->dotLine = bp->baseLine;
    bp->dotOffset = 0;
    return meFALSE;
}

int
meAnchorSet(meBuffer *bp, meInt name, meLine *lp, meInt lineNo, meUShort off, int silent)
{
    meAnchor *p = bp->anchorList;

    while((p != NULL) && (p->name != name))
        p = p->next;

    if(p == NULL)
    {
        if((p = (meAnchor*) meMalloc(sizeof(meAnchor))) == NULL)
            return meFALSE;
        p->next = bp->anchorList;
        bp->anchorList = p;
    }
    else
    {
        /* see if we can remove the anchor flag from the old line */
        if(p->line != lp)
        {
            meAnchor *q = bp->anchorList;
            meLineFlag flag = meAnchorGetLineFlag(p);

            meAssert(p->line->flag & flag);
            while(q != NULL)
            {
                if((q != p) && (q->line == p->line) && (meAnchorGetLineFlag(q) == flag))
                    break;
                q = q->next;
            }
            if(q == NULL)
                p->line->flag ^= flag;
        }
        if(!silent)
            mlwrite(MWCLEXEC,(meUByte *)"[overwriting existing mark]");
    }
    p->name = name;
    p->line = lp;
    p->offs = off;
    p->frwd = (lineNo < (bp->lineCount>>1));
    /* mark the line as having an anchor */
    lp->flag |= meAnchorGetLineFlag(p);
    return meTRUE;
}

int
setAlphaMark(int f, int n)
{
    /*
     * Place a mark at the current offset within the buffer. This involves
     * searching down the mark list and tacking a new mark list element on
     * the end.
     */
    meWindow *cwp;
    meInt cc;

    if((cc = mlCharReply((meUByte *)"Place mark: ",mlCR_QUIT_ON_USER,NULL,NULL)) == -2)
        cc = mlCharReply((meUByte *)"Place mark: ",mlCR_ALPHANUM_CHAR,NULL,NULL);

    if(cc < 0)
        return ctrlg(meFALSE,1);

    cwp = frameCur->windowCur;
    return meAnchorSet(cwp->buffer,cc,cwp->dotLine,cwp->dotLineNo,cwp->dotOffset,0);
}

int
gotoAlphaMark(int f, int n)
{
    /*
     * Return to an alphabetic mark (ie make the current line and the
     * current offset equal to the line and offset stored in the mark
     * element with the required name).
     */
    meWindow *cwp=frameCur->windowCur;
    meInt cc;

    if(n == 0)
    {
        meAnchor *p = cwp->buffer->anchorList;
        meUByte  *s = resultStr;

        while(p != NULL)
        {
            if(p->name < 128)
                *s++ = (meUByte) p->name;
            p = p->next;
        }
        *s = '\0';
        return meTRUE;
    }

    if((cc = mlCharReply((meUByte *)"Goto mark: ",mlCR_QUIT_ON_USER,NULL,NULL)) == -2)
        cc = mlCharReply((meUByte *)"Goto mark: ",mlCR_ALPHANUM_CHAR,NULL,NULL);

    if(cc < 0)
        return ctrlg(meFALSE,1);

    if(meAnchorGet(cwp->buffer,cc) <= 0)
    {
        meAnchor *p = cwp->buffer->anchorList;
        meUByte  allmarks[256];	/* record of the marks	*/
        int      ii = 0;

        while(p != NULL)
        {
            if(p->name < 128)
            {
                allmarks[ii++] = ' ';
                allmarks[ii++] = (meUByte) p->name;
            }
            p = p->next;
        }
        if(ii == 0)
            return mlwrite(MWABORT|MWCLEXEC,(meUByte *)"[No marks in buffer]");
        allmarks[ii] = '\0';
        return mlwrite(MWABORT|MWCLEXEC,(meUByte *)"[Marks in buffer:%s]", allmarks);
    }

    /* do the buisness */
    cwp->dotLine = cwp->buffer->dotLine;
    cwp->dotOffset = cwp->buffer->dotOffset;
    cwp->dotLineNo = cwp->buffer->dotLineNo;
    cwp->updateFlags |= WFMOVEL;

    return meTRUE;
}


#ifdef _SINGLE_CASE
/* make string lower case */
void
makestrlow(meUByte *str)
{
    register meUByte cc, *p=str;

    while((cc=*p) != '\0')
        *p++ = toLower(cc);
}
#endif

int
insFileName(int f, int n)
{
    meUByte *name, *p, cc;
    int s, count=0;

    if(n < 0)
    {
        name = frameCur->windowCur->buffer->name;
        n = 0 - n;
    }
    else if(n > 0)
        name = frameCur->windowCur->buffer->fileName;
    else
        return meTRUE;
    if(name == NULL)
        return ctrlg(meFALSE,1);

    if((s=bufferSetEdit()) > 0)               /* Check we can change the buffer */
    {
        do
        {
            p = name;
            while(((cc = *p++) != 0) && ((s = lineInsertChar(1,cc)) > 0))
                count++;
        } while((s > 0) && (--n > 0));
#if MEOPT_UNDO
        meUndoAddInsChars(count);
#endif
    }
    return s;
}

#if MEOPT_EXTENDED
/* compare-windows. Compare buffers against each other. In no argument is
 * specified then perform a white space insensitive comparison */
int
cmpBuffers(int f, int n)
{
    register meWindow *swp, *wp;
    register meUByte cc;

    swp = NULL;
    wp = frameCur->windowList;
    do {
        if((wp->flags & meWINDOW_NO_CMP) == 0)
        {
            if(swp != NULL)
                break;
            swp = wp;
        }
    } while((wp = wp->next) != NULL);
    if(wp == NULL)
        return mlwrite(MWABORT,(meUByte *)"[Not enough windows to compare]");

    if(n == 0)
    {
        /* Exact match - white space is matched. */
        for(;;)
        {
            wp = swp;
            cc = meWindowGetChar(wp);
            while((wp = wp->next) != NULL)
                if(((wp->flags & meWINDOW_NO_CMP) == 0) &&
                   (meWindowGetChar(wp) != cc))
                    return meFALSE;
            wp = swp;
            cc = meWindowForwardChar(wp,1);
            while((wp = wp->next) != NULL)
                if(((wp->flags & meWINDOW_NO_CMP) == 0) &&
                   (meWindowForwardChar(wp,1) != cc))
                    return meFALSE;
            if(cc == meFALSE)
                break;
        }
    }
    else
    {
        /* Ignore white space */
        for(;;)
        {
            meUByte moreData;
            meUByte winData;
            meUByte tmpc;

            wp = swp;
            cc = meWindowGetChar(wp);
            if((n & 1) && isSpace(cc))
                cc = ' ';

            /* Check the current character */
            while((wp = wp->next) != NULL)
            {
                if(((wp->flags & meWINDOW_NO_CMP) == 0) && ((tmpc = meWindowGetChar(wp)) != cc))
                {
                    if(cc == ' ')
                    {
                        if(((n & 1) == 0) || !isSpace(tmpc))
                            return meFALSE;
                    }
                    else if(((n & 2) == 0) || (toLower(cc) != toLower(tmpc)))
                        return meFALSE;
                }
            }

            /* Back to the start of the buffers. Advance to the next character
             * in the buffer. if the current character is a space then advance
             * to the next non-white character in the buffer. */
            wp = swp;
            if(((moreData = meWindowForwardChar(wp,1)) > 0) && (cc == ' ') && (n & 1))
            {
                do
                {
                    tmpc = meWindowGetChar(wp);
                    if(!isSpace(tmpc))
                        break;
                }
                while ((moreData = meWindowForwardChar(wp,1)) > 0);
            }

            /* Advance the rest of the windows. */
            while((wp = wp->next) != NULL)
            {
                if((wp->flags & meWINDOW_NO_CMP) == 0)
                {
                    if(((winData = meWindowForwardChar(wp,1)) > 0) && (cc == ' ') && (n & 1))
                    {
                        do
                        {
                            tmpc = meWindowGetChar(wp);
                            if(!isSpace(tmpc))
                                break;
                        }
                        while ((winData = meWindowForwardChar(wp,1)) > 0);
                    }
                    if(winData != moreData)
                        return meFALSE;
                }
            }
            if(moreData == meFALSE)
                break;
        }
    }
    return meTRUE;
}
#endif

#if MEOPT_CALLBACK
int
createCallback(int f, int n)
{
    struct meTimeval tp;
    meMacro *mac;
    meUByte buf[meBUF_SIZE_MAX];

    if((mac=userGetMacro(buf,meBUF_SIZE_MAX)) == NULL)
        return meFALSE;
    if(n < 0)
        mac->callback = -1;
    else
    {
        gettimeofday(&tp,NULL);
        mac->callback = ((tp.tv_sec-startTime)*1000) + (tp.tv_usec/1000) + n;
        if(!isTimerExpired(CALLB_TIMER_ID) &&
           (!isTimerSet(CALLB_TIMER_ID) ||
            (meTimerTime[CALLB_TIMER_ID] > mac->callback)))
            timerSet(CALLB_TIMER_ID,mac->callback,n);
    }
    return meTRUE;
}

void
callBackHandler(void)
{
    struct meTimeval tp;
    register meMacro *mac;
    register time_t tim, next;
    register int ii;

    do {
        gettimeofday(&tp,NULL);
        tim = ((tp.tv_sec-startTime)*1000) + (tp.tv_usec/1000);
        next = 0x7fffffff;

        /* Loop through all the macros executing them or getting the time of the next */
        for(ii=CK_MAX ; ii<cmdTableSize ; ii++)
        {
            mac = getMacro(ii);
            if((mac->callback >= 0) && (mac->callback < next))
            {
                if(mac->callback <= tim)
                {
                    mac->callback = -1;
                    /* If the current buffer has an input handler, that macro will
                     * receive the key "callback", this must be bound to this macro,
                     * otherwise the input handler will not know what macro to call
                     */
                    if(frameCur->windowCur->buffer->inputFunc >= 0)
                    {
                        /* binary chop through the key table looking for the character code.
                        ** If found then return the index into the names table.
                        */
                        register meBind *ktp;			/* Keyboard character array */
                        register int     low;			/* Lowest index in table. */
                        register int     hi;			/* Hightest index in table. */
                        register int     mid;			/* Mid value. */
                        register int     status;		/* Status of comparison. */
                        ktp = keytab;
                        hi  = keyTableSize;			/* Set hi water to end of table */
                        low = 0;				/* Set low water to start of table */
                        do
                        {
                            mid = (low + hi) >> 1;		/* Get mid value. */
                            if ((status=(ME_SPECIAL|SKEY_callback)-ktp[mid].code) == 0)
                            {
                                /* Found - return index */
                                ktp[mid].index = ii;
                                break;
                            }
                            else if (status < 0)
                                hi = mid - 1;		/* Discard bottom half */
                            else
                                low = mid + 1;		/* Discard top half */
                        } while (low <= hi);		/* Until converges */
                    }
                    execFuncHidden(ME_SPECIAL|SKEY_callback,ii,0);
                    /* macro could have taken a measurable amount of time to run, so we need to update the current tim and loop through again */
                    next = 0;
                    break;
                }
                next = mac->callback;
            }
        }
    } while(!next);

    if(next != 0x7fffffff)
        timerSet(CALLB_TIMER_ID,next,(meInt) (next-tim));
    else
        timerClearExpired(CALLB_TIMER_ID);
    if(tim & 0x40000000)
        adjustStartTime(tp.tv_sec-startTime);
}
#endif

#if MEOPT_MOUSE
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
    register meWindow *wp;
    register meLine   *ln;
    int      row, col, ii;
    int      odoto;                     /* Old doto */
    meLine    *odotp;                     /* Old dotp */

    if(!f)
        n = 3;

    /* Handle the message line */
    row = mouse_Y;
    if(row >= frameCur->depth)
    {
        mouse_pos = MIMESSAGE;          /* On the message line */
        return meTRUE;
    }

    /* Handle the menu line */
    if(row < frameCur->menuDepth)
    {
        mouse_pos = MIMENU;
        return meTRUE;
    }

    /* Locate the window associated with the mouse position. */
    col = mouse_X;
    for (wp = frameCur->windowList; wp != NULL; wp = wp->next)
    {
        if (wp->frameRow > row)
            continue;                   /* Above window */
        if ((wp->frameRow + wp->depth) <= row)
            continue;                   /* Below window */
        if (wp->frameColumn > col)
            continue;                   /* Left of window */
        if ((wp->frameColumn + wp->width) > col)
            break;                      /* In the window !! */
    }
    if(wp == NULL)
    {
        mouse_pos = MIERROR;            /* Set bad status */
        return meFALSE;                 /* Fail */
    }

    if(wp != frameCur->windowCur)       /* Current window ?? */
    {
        if((n & 0x01) == 0)
        {
            mouse_pos = MIERROR;        /* Set bad status */
            return meTRUE;
        }
        meWindowMakeCurrent(wp);        /* No - make it so */
    }

    /* Handle the divider */
    col -= wp->frameColumn;             /* Normalise the column count */
    if(col >= wp->textWidth)
    {
        /* Iterate down the scroll positions */
        for (ii = 0; ii <= (WCVSBML-WCVSBSPLIT); ii++)
            if (row < wp->vertScrollBarPos[ii])
                break;

        if (col > wp->textWidth) /* Report if in the second column */
            mouse_pos = MICOLUNM2;
        else
            mouse_pos = 0;
        /* Translate to return code */
        if (wp->vertScrollBarMode & WMSCROL)       /* Scroll bar enabled ?? */
            mouse_pos |= MISBSPLIT+ii;  /* Yes - report scroll bar component */
        else if (ii>=(WCVSBML-WCVSBSPLIT))/* On the corner ?? */
            mouse_pos |= MISBML;        /* Yes - report corner */
        else
            mouse_pos |= MIDIVIDER;     /* Report divider */
        return (meTRUE);                  /* Done */
    }

    /* Handle the mode line */
    row -= wp->frameRow;                 /* Normalise the row count */
    if (row >= wp->textDepth)
    {
        mouse_pos = MIMODE;             /* On the mode line */
        return (meTRUE);                  /* And done */
    }

    /* Must be in the text area !! should we move the cursor */
    mouse_pos = MITEXT;
    if((n & 2) == 0)
        return meTRUE;

    /* Work out the new postion of the cursor in the buffer.
     * Save the old position so that we can later check if we
     * have moved. */
    odoto = wp->dotOffset;
    odotp = wp->dotLine;

    ln = wp->dotLine;
    ii = row - (wp->dotLineNo - wp->vertScroll);
    if(ii > 0)
    {
        if(wp->dotLineNo+ii > wp->buffer->lineCount)
            ii = wp->buffer->lineCount - wp->dotLineNo;
        wp->dotLineNo += ii;
        while(ii--)
            ln = meLineGetNext(ln);
    }
    else if(ii < 0)
    {
        wp->dotLineNo += ii;
        while(ii++)
            ln = meLineGetPrev(ln);
    }

    /* Take into account the fact that the buffer might be horizontally
     * scrolled. Change the base column position to take this into
     * account. Note that this is not perfect since if you click on the
     * dollars you get hyperspaed backwards or forwards - but I am not
     * going to try and work this one out now !! - Jon Green 01/03/97 */
    if (wp->dotLine != ln)
        wp->horzScroll = wp->horzScrollRest;      /* Set the line scroll to the window offset */

    /* Compute the new position */
    wp->dotLine = ln;
    col += wp->horzScroll;
    setcwcol(wp,col);

    /* If we have moved then set Window flag to indicate that we have
     * changed cursor position.
     * Check the line first as WFMOVEL does the column as well
     */
    if(wp->dotLine != odotp)
        wp->updateFlags |= WFMOVEL;               /* Moved from line to line */
    else if(wp->dotOffset != odoto)
        wp->updateFlags |= WFMOVEC;               /* Moved from col to col   */

    /* Determine if we are in the selection region. If so then set
     * $mouse_pos to reflect the fact that we are in a region. */
    if((frameCur->windowCur->buffer == wp->buffer) && (selhilight.flags & SELHIL_ACTIVE))
    {
        if ((selhilight.dotLineNo > selhilight.markLineNo) ||
            ((selhilight.dotLineNo == selhilight.markLineNo) &&
             (selhilight.dotOffset>=selhilight.markOffset)))
        {
            /* Point is below mark */
            if (((wp->dotLineNo > selhilight.markLineNo) ||
                 ((wp->dotLineNo == selhilight.markLineNo) &&
                  (wp->dotOffset >= selhilight.markOffset))) &&
                ((wp->dotLineNo < selhilight.dotLineNo) ||
                 ((wp->dotLineNo == selhilight.dotLineNo) &&
                  (wp->dotOffset <= selhilight.dotOffset))))
                mouse_pos |= MIREGION;
        }
        else
        {
            /* Point is above mark */
            if (((wp->dotLineNo > selhilight.dotLineNo) ||
                 ((wp->dotLineNo == selhilight.dotLineNo) &&
                  (wp->dotOffset >= selhilight.dotOffset))) &&
                ((wp->dotLineNo < selhilight.markLineNo) ||
                 ((wp->dotLineNo == selhilight.markLineNo) &&
                  (wp->dotOffset <= selhilight.markOffset))))
                mouse_pos |= MIREGION;
        }
    }
    return meTRUE;
}
#endif
