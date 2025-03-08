/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * isearch.c - incremental search routines.
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
 * Created:     9-May-86
 * Synopsis:    incremental search routines.
 * Authors:     D. R. Banks, John M. Gamble & Steven Phillips
 * Description:
 *     The functions in this file implement commands that perform incremental
 *     searches in the forward and backward directions.  This "ISearch" command
 *     is intended to emulate the same command from the original EMACS 
 *     implementation (ITS).  Contains references to routines internal to
 *     search.c.
 */


#define __ISEARCHC                   /* Define the filename */

#include "emain.h"
#include "efunc.h"
#include "eskeys.h"
#include "esearch.h"

#if MEOPT_ISEARCH

static meUByte isHeadStr[12]="f-isearch: ";
static meUByte *statusStr[5]={
    (meUByte *)" [LOST]",(meUByte *)" [FOUND]",(meUByte *)" [INCOMPLETE]",(meUByte *)" [REGEX ERROR]",(meUByte *)" [SEARCHING]"
};

static meInt
isearchGotoEnd(meUByte *patrn, int flags, int histNo, meShort *histLen, SCANNERPOS *histPos)
{
    meWindow *cwp;
    if(histLen[histNo] == 0)
        return meTRUE;
    cwp = frameCur->windowCur;
#if MEOPT_IPIPES
    /* due to the dynamic nature of an active ipipe buffer, things have to
     * be done differently */
    if(flags & ISCANNER_PIPE)
    {
        meLine *tmpline;
        int   tmpoff;
        meInt tmplno;
        meByte cc;
        tmpline = cwp->dotLine;      /* Store the current position*/
        tmpoff = cwp->dotOffset;     /* incase its not found      */
        tmplno = cwp->dotLineNo;
        if(flags & ISCANNER_BACKW)
            meWindowForwardChar(cwp, 1);
        cc = patrn[histLen[histNo]];
        patrn[histLen[histNo]] = '\0';
        if(iscanner(patrn, 0,flags,histPos+histNo+1) <= 0)
        {
            patrn[histLen[histNo]] = cc;
            cwp->dotLine = tmpline;       /* Reset the position */
            cwp->dotOffset = tmpoff;
            cwp->dotLineNo = tmplno;
            return meFALSE;
        }
        patrn[histLen[histNo]] = cc;
    }
    else
#endif
    {
        cwp->dotLine = histPos[histNo].endline;
        cwp->dotOffset = histPos[histNo].endoffset;
        cwp->dotLineNo = histPos[histNo].endline_no;
    }
    return meTRUE;
}
static meInt
isearchGotoStart(meUByte *patrn, int flags, int histNo, meShort *histLen, SCANNERPOS *histPos)
{
    meWindow *cwp;
    if(histLen[histNo] == 0)
        return meTRUE;
    cwp = frameCur->windowCur;
#if MEOPT_IPIPES
    /* due to the dynamic nature of an active ipipe buffer, things have to
     * be done differently */
    if(flags & ISCANNER_PIPE)
    {
        meLine *tmpline;
        int   tmpoff;
        long  tmplno;
        meUByte cc;
        tmpline = cwp->dotLine;      /* Store the current position*/
        tmpoff = cwp->dotOffset;     /* incase its not found      */
        tmplno = cwp->dotLineNo;
        if(!(flags & ISCANNER_BACKW))
            meWindowForwardChar(cwp, 1);
        cc = patrn[histLen[histNo]];
        patrn[histLen[histNo]] = '\0';
        if(iscanner(patrn,0,(flags ^ (ISCANNER_BACKW|ISCANNER_PTBEG)),histPos+histNo+1) <= 0)
        {
            patrn[histLen[histNo]] = cc;
            cwp->dotLine = tmpline;       /* Reset the position */
            cwp->dotOffset = tmpoff;
            cwp->dotLineNo = tmplno;
            return meFALSE;
        }
        patrn[histLen[histNo]] = cc;
    }
    else
#endif
    {
        cwp->dotLine = histPos[histNo].startline;
        cwp->dotOffset = histPos[histNo].startoff;
        cwp->dotLineNo = histPos[histNo].startline_no;
    }
    return meTRUE;
}

/*
 * This hack will search for the next occurrence of <pat> in the buffer, either
 * forward or backward.  It is called with the status of the prior search
 * attempt, so that it knows not to bother if it didn't work last time.  If
 * we can't find any more matches, "point" is left where it was before.  If
 * we do find a match, "point" will be at the end of the matched string for
 * forward searches and at the beginning of the matched string for reverse
 * searches.
 */
static meInt
scanmore(meUByte *patrn, int flags, int histNo, meShort *histLen, SCANNERPOS *histPos)
{
    int sts;                            /* search status */
    
    /* Must flag as seaching, even if current status is meABORT
     * cos if the char is a ']' then the search may continue */
    mlDisp(isHeadStr,patrn,statusStr[4],-1);
    if(flags & ISCANNER_SCANMR)
    {
        isearchGotoStart(patrn,flags,histNo,histLen,histPos);
        if(flags & ISCANNER_BACKW)      /* reverse search? */
            meWindowForwardChar(frameCur->windowCur,1);
    }
    /* iscanner can return:
     *  -2 - regex class error
     *  -1 - regex error
     *   0 - Failed to find
     *   1 - found */
    sts = iscanner(patrn,0,flags,histPos+histNo+1);
    
    if(sts <= 0)
    {
        memcpy(histPos+histNo+1,histPos+histNo,sizeof(SCANNERPOS));
        if(sts < 0)
            sts += 4;
        
        if(flags & ISCANNER_SCANMR)
            isearchGotoEnd(patrn,flags,histNo,histLen,histPos);
        
        if(!TTbreakFlag && (sts == meFALSE))
            sts |= 0x10;
    }
    return sts;
}

#if MEOPT_LOCALBIND
#define restoreUseMlBinds()  (useMlBinds = oldUseMlBinds)
#else
#define restoreUseMlBinds()
#endif

#define mlisDisp(pat,status)                                                 \
do {                                                                         \
    mlDisp(isHeadStr,pat,statusStr[(status & 0x3)],-1);                      \
    frameCur->mlStatus=MLSTATUS_KEEP;                                        \
    if(status & 0x10)                                                        \
    {                                                                        \
        TTbell();  /* Feep if search fails       */                          \
        if(kbmd == mePLAY)                                                   \
        {                                                                    \
            frameCur->mlStatus = MLSTATUS_CLEAR;                             \
            restoreUseMlBinds();                                             \
            return meFALSE;                                                  \
        }                                                                    \
        status &= 0x0f;                                                      \
    }                                                                        \
    mwResetCursor();                                                         \
    TTflush();                                                               \
} while(0)
/*
 * Subroutine to do an incremental search.  In general, this works similarly
 * to the older micro-emacs search function, except that the search happens
 * as each character is typed, with the screen and cursor updated with each
 * new search character.
 *
 * While searching forward, each successive character will leave the cursor
 * at the end of the entire matched string.  Typing a Control-S or Control-X
 * will cause the next occurrence of the string to be searched for (where the
 * next occurrence does NOT overlap the current occurrence).  A Control-R will
 * change to a backwards search, META will terminate the search and Control-G
 * will abort the search.  Rubout will back up to the previous match of the
 * string, or if the starting point is reached first, it will delete the
 * last character from the search string.
 *
 * While searching backward, each successive character will leave the cursor
 * at the beginning of the matched string.  Typing a Control-R will search
 * backward for the next occurrence of the string.  Control-S or Control-X
 * will revert the search to the forward direction.  In general, the reverse
 * incremental search is just like the forward incremental search inverted.
 *
 * In all cases, if the search fails, the user will be feeped, and the search
 * will stall until the pattern string is edited back into something that
 * exists (or until the search is aborted).
 */
 
static int
isearch(int flags)
{
    meWindow     *cwp=frameCur->windowCur;
    meInt         status;       /* Search status */
    meUInt        arg;          /* argument */
    int           cpos;         /* character number in search string */
    int           c;            /* current input character */
    meLine       *tmpline;      /* Tempory storage for wrap searching*/
    int           tmpoff;
    meInt         tmplno;
    SCANNERPOS    histPos[HISTBUFSIZ];
    meShort       histLen[HISTBUFSIZ];
    meUByte       histStt[HISTBUFSIZ], kbmd=kbdmode;
    meShort       histNo=0;
#if MEOPT_LOCALBIND
    /* Initialize starting conditions */
    meUByte       oldUseMlBinds = useMlBinds;
    useMlBinds = 1;             /* Use the ml-bind-key's             */
#endif    
    
    /* Use search's srchPat array so that the hunt functions will work */
    srchPat[0] = '\0';
    histPos[0].startline = histPos[0].endline = cwp->dotLine;       /* Save the current line pointer     */
    histPos[0].startoff = histPos[0].endoffset = cwp->dotOffset;     /* Save the current line pointer     */
    histPos[0].startline_no = histPos[0].endline_no = cwp->dotLineNo;   /* Save the current line pointer     */
    histLen[0] = 0;                                   /* Save the current offset */
    histStt[0] = meTRUE;
    
    /* ask the user for the text of a pattern */
    isHeadStr[9] = '\0';
    mlwrite(MWCURSOR,(meUByte *)"%s (default [%s]): ",isHeadStr,(numSrchHist==0) ? (meUByte *)"":srchHist[0]);
    /* switch on the status so we save it */
    frameCur->mlStatus = MLSTATUS_KEEP|MLSTATUS_POSML;
    if(meModeTest((arg=cwp->buffer->mode),MDMAGIC))
        flags |= ISCANNER_MAGIC;
    if(meModeTest(arg,MDEXACT))
        flags |= ISCANNER_EXACT;
#if MEOPT_IPIPES
    /* due to the dynamic nature of an active ipipe buffer, things have to
     * be done differently */
    if(meModeTest(arg,MDPIPE))
        flags |= ISCANNER_PIPE;
#endif    
        
    /*
     * Get the first character in the pattern. If we get an initial C-s or
     * C-r, re-use the old search string and find the first occurrence
     */

get_another_key:
    /* Get the first character   */
    c = meGetKey(meGETKEY_SILENT|meGETKEY_COMMAND);
    switch(decode_key((meUShort) c,&arg))
    {
    case CK_NEWLIN:                     /* ^M : New line. do other search */
        restoreUseMlBinds();
        if(flags & ISCANNER_BACKW)
            return searchBack(meFALSE,1);
        else
            return searchForw(meFALSE,1);

    case CK_FORLIN:   /* C-n - next match in complete list */
    case CK_BAKLIN:   /* C-p - previous match in complete list */
#if MEOPT_WORDPRO
    case CK_GOEOP:    /* M-n - Got to next in history list */
    case CK_GOBOP:    /* M-p - Got to previous in history list */
#else
    case CK_GOEOF:
    case CK_GOBOF:
#endif
    case CK_YANK:     /* C-y - yank the contents of the kill buffer */
        meGetKeyFirst = c;
        if((status = meGetString(isHeadStr,MLSEARCH|MLISEARCH,0,srchPat,meBUF_SIZE_MAX)) <= 0)
        {
            restoreUseMlBinds();
            return status;
        }
        isHeadStr[9] = ':';
        if(meGetKeyFirst < 0)
        {
            status = scanmore(srchPat,flags,0,histLen,histPos);  /* do 1 search     */
            mlisDisp(srchPat,status);
            frameCur->mlStatus = MLSTATUS_CLEAR;
            restoreUseMlBinds();
            return (status == meTRUE);
        }
        c = meGetKeyFirst;
        meGetKeyFirst = -1;
        break;

    case CK_FISRCH:
    case CK_FORSRCH:
    case CK_BISRCH:
    case CK_BAKSRCH:
        if(numSrchHist)
            meStrcpy(srchPat,srchHist[0]);
        break;
    case CK_VOIDFUNC:
        /* could be a pick on a mouse and we want the drop - so try again */
        goto get_another_key;
    }
    
    cpos = meStrlen(srchPat);
    isHeadStr[9] = ':';
    status = meTRUE;                        /* Assume everything's cool   */
    mlisDisp(srchPat,status);
    /* Top of the per character loop */

    for(;;)
    {
        switch(decode_key((meUShort) c,&arg))
        {
        case CK_ABTCMD: /* ^G : Abort input and return */
#if MEOPT_IPIPES
            if(!(flags & ISCANNER_PIPE))
#endif    
            {
                cwp->dotLine = histPos[0].endline;
                cwp->dotOffset = histPos[0].endoffset;
                cwp->dotLineNo = histPos[0].endline_no;
                cwp->updateFlags |= WFMOVEL;       /* Say that we've moved      */
            }
            goto bad_finish;                    /* Finish processing */

        case CK_BISRCH:
        case CK_BAKSRCH:                        /* If backward search        */
            tmpoff = ISCANNER_BACKW;            /*  go forward               */
            isHeadStr[0] = 'b';
            goto find_next;

        case CK_FISRCH:
        case CK_FORSRCH:                        /* If forward search         */
            isHeadStr[0] = 'f';
            tmpoff = 0;                         /* go forward                */
find_next:
            if(status >= 2)
            {
                TTbell();
                break;
            }
            if((flags & ISCANNER_BACKW) != tmpoff)
            {
                flags ^= ISCANNER_BACKW|ISCANNER_PTBEG;
                if(status == meFALSE)
                    status = meTRUE;
            }
            tmpline = cwp->dotLine;       /* Store the current position*/
            tmpoff = cwp->dotOffset;      /* incase its not found      */
            tmplno = cwp->dotLineNo;
            /* The status cannot be meABORT, only meFALSE or meTRUE */
            if(!status)                         /* if already lost goto start*/
            {
                if(flags & ISCANNER_BACKW)      /* if backward               */
                    c = windowGotoEob(0,1);          /* and move to the bottom    */
                else
                    c = windowGotoBob(0,1);          /* and move to the top       */
            }
            else if(histNo && (histPos[histNo].startoff == histPos[histNo].endoffset) &&
                    (histPos[histNo].startline_no == histPos[histNo].endline_no))
            {
                /* the search string has matched a zero length string, e.g. "^"
                 * so move the cursor position one character */ 
                if(flags & ISCANNER_BACKW)      /* if backward               */
                    c = meWindowBackwardChar(cwp,1);
                else
                    c = meWindowForwardChar(cwp,1);
            }
            else
                c = meTRUE;
            if(c <= 0)
                c = (TTbreakFlag) ? 0:0x10;
            else if((c=scanmore(srchPat,flags,histNo,histLen,histPos)) != meTRUE) /* Start the search again    */
            {
                if(TTbreakFlag)
                {
                    c = 0;
                    goto bad_finish;           /* Finish processing */
                }
                cwp->dotLine = tmpline;   /* Reset the position        */
                cwp->dotOffset = tmpoff;
                cwp->dotLineNo = tmplno;
            }
            status = c;
            histNo++;
            histLen[histNo] = cpos;             /* Save the current srchPat len */
            histStt[histNo] = (meUByte) status; /* Save the current status   */
            break;                              /* Go continue with the srch */

        case CK_QUOTE:                          /* ^Q - quote next character */
            if((c = quoteKeyToChar(meGetKey(meGETKEY_SILENT|meGETKEY_SINGLE))) < 0)
            {
                TTbell();
                break;
            }
            goto isAddChar;

        case CK_NEWLIN:                         /* ^M : New line. */
            c = 0;
            goto good_finish;                   /* Finish processing */

        case CK_KILREG:                         /* ^W : Kill the whole line? */
            if(status == meTRUE)
            {
                int tt;
                
                if(flags & ISCANNER_BACKW)
                    isearchGotoEnd(srchPat,flags,histNo,histLen,histPos);
                
                /* use function from word.c  */
                tt = (meWindowInWord(cwp)==0);
                while(((meWindowInWord(cwp)==0)==tt) && (cpos < meBUF_SIZE_MAX))    
                {
                    if(meLineGetLength(cwp->dotLine) == cwp->dotOffset)
                        c = meCHAR_NL;
                    else
                        c = meLineGetChar(cwp->dotLine, cwp->dotOffset);
                    if((flags & ISCANNER_MAGIC) &&
                       ((c == '[') || (c == '.') || (c == '+') || (c == '*') || (c == '^') || (c == '$') || (c == '?') || (c == '\\')))
                        srchPat[cpos++] = '\\';
                    srchPat[cpos++] = c;
                    if(meWindowForwardChar(cwp, 1) == meFALSE)
                        break;
                }
                srchPat[cpos] = 0;              /* null terminate the buffer */
                memcpy(histPos+histNo+1,histPos+histNo,sizeof(SCANNERPOS));
                histNo++;
                histStt[histNo] = meTRUE;
                histLen[histNo] = cpos;         /* Save the current srchPat len */
                histPos[histNo].endline = cwp->dotLine;
                histPos[histNo].endoffset = cwp->dotOffset;
                histPos[histNo].endline_no = cwp->dotLineNo;
                
                if(flags & ISCANNER_BACKW)
                    isearchGotoStart(srchPat,flags,histNo,histLen,histPos);
                goto is_redraw;
            }
            break;
        case CK_RECENT:                         /* ^L : Redraw the screen    */
            sgarbf = meTRUE;
            update(meTRUE);
            mlerase(0);
            goto is_redraw;
            
        case CK_DELBAK:                         /* ^H : backwards delete.    */
            if((--histNo) < 0)
            {
                c = 0;
                goto bad_finish;                /* Finish processing */
            }
            cpos = histLen[histNo];
            status = histStt[histNo];
#if MEOPT_IPIPES
            /* due to the dynamic nature of an active ipipe buffer, things have to
             * be done differently */
            if(flags & ISCANNER_PIPE)
            {
                if(histLen[histNo] == histLen[histNo+1])
                {
                    /* was it a swap around */
                    if((histPos[histNo].endline == histPos[histNo+1].startline) &&
                       (histPos[histNo].endoffset == histPos[histNo+1].startoff))
                    {
                        flags ^= ISCANNER_BACKW|ISCANNER_PTBEG;
                        isearchGotoEnd(srchPat,flags,histNo,histLen,histPos);
                    }
                    else
                    {
                        if(histStt[histNo+1] == meTRUE)
                        {
                            isearchGotoStart(srchPat,flags,histNo,histLen,histPos+1);
                            if(histStt[histNo] == meTRUE)
                                iscanner(srchPat,0,(flags ^ (ISCANNER_BACKW|ISCANNER_PTBEG)),&(histPos[histNo]));
                            isearchGotoEnd(srchPat,flags,histNo,histLen,histPos);
                        }
                    }
                        
                }
                else
                {
                    if(histStt[histNo+1] == meTRUE)
                    {
                        isearchGotoStart(srchPat,flags,histNo,histLen,histPos+1);
                        srchPat[cpos] = 0;
                        if(((cwp->dotLine != histPos[histNo].startline) ||
                            (cwp->dotOffset != histPos[histNo].startoff)) && (histStt[histNo] == meTRUE))
                            iscanner(srchPat,0,(flags ^ (ISCANNER_BACKW|ISCANNER_PTBEG)),&(histPos[histNo]));
                        isearchGotoEnd(srchPat,flags,histNo,histLen,histPos);
                    }
                }
            }
            else
#endif
            {
                cwp->dotLine = histPos[histNo].endline;
                cwp->dotOffset = histPos[histNo].endoffset;
                cwp->dotLineNo = histPos[histNo].endline_no;
            }
            srchPat[cpos] = 0;
            cwp->updateFlags |= WFMOVEL;           /* Say that we've moved      */
is_redraw:
#if MEOPT_IPIPES
            /* due to the dynamic nature of an active ipipe buffer, things have to
             * be done differently */
            if(flags & ISCANNER_PIPE)
            {
                isearchGotoStart(srchPat,flags,histNo,histLen,histPos);
                histPos[histNo].startline = cwp->dotLine;
                histPos[histNo].startoff = cwp->dotOffset;
                histPos[histNo].startline_no = cwp->dotLineNo;
                isearchGotoEnd(srchPat,flags,histNo,histLen,histPos);
                histPos[histNo].endline = cwp->dotLine;
                histPos[histNo].endoffset = cwp->dotOffset;
                histPos[histNo].endline_no = cwp->dotLineNo;
            }
#endif
            setShowRegion(cwp->buffer,
                          histPos[histNo].startline_no,histPos[histNo].startoff,
                          histPos[histNo].endline_no,histPos[histNo].endoffset);
            meBufferAddModeToWindows(cwp->buffer,WFSELHIL);
            break;
            
        default:                                /* All other chars           */
            if((c < 0x20) || (c > 0xff))        /* Is it printable?          */
            {
#if MEOPT_CALLBACK
                /* must ignore [SCA]-pick and [SCA]-drop */
                if(((c & ~(ME_SHIFT|ME_CONTROL|ME_ALT)) == (ME_SPECIAL|SKEY_pick)) ||
                   ((c & ~(ME_SHIFT|ME_CONTROL|ME_ALT)) == (ME_SPECIAL|SKEY_drop)))
                    goto input_cont;            /* Ignore this one, get another */
#endif
                goto good_finish;               /* Nope. - Finish processing */
            }
isAddChar:
            srchPat[cpos++] = c;                /* put the char in the buffer*/
            if(cpos >= meBUF_SIZE_MAX)                    /* too many chars in string? */
            {                                   /* Yup.  Complain about it   */
                addHistory(MLSEARCH,srchPat,meFALSE);
                mlwrite(MWABORT,(meUByte *)"[Search string too long!]");
                goto quit_finish;
            }
            srchPat[cpos] = 0;                  /* null terminate the buffer */
            if(status != meFALSE)               /* If not lost last time     */
            {
                /*  or find the next match   */
                status = scanmore(srchPat,flags|ISCANNER_SCANMR,histNo,histLen,histPos);
                if(TTbreakFlag)
                    goto quit_finish;           /* Finish processing */
            }
            else
                memcpy(histPos+histNo+1,histPos+histNo,sizeof(SCANNERPOS));
            histNo++;
            histLen[histNo] = cpos;             /* Save the current srchPat len */
            histStt[histNo] = (meUByte) status; /* Save the current status   */
            break;
        }  /* Switch */
        if(histNo == HISTBUFSIZ-1)
        {
            /* history is full to bust, best thing to do is ditch the first
             * half and thus make it half full.
             */
            memcpy(histPos,histPos+(HISTBUFSIZ-(HISTBUFSIZ/2)),(HISTBUFSIZ/2)*sizeof(SCANNERPOS));
            memcpy(histLen,histLen+(HISTBUFSIZ-(HISTBUFSIZ/2)),(HISTBUFSIZ/2)*sizeof(meShort));
            memcpy(histStt,histStt+(HISTBUFSIZ-(HISTBUFSIZ/2)),(HISTBUFSIZ/2)*sizeof(meUByte));
            histNo = (HISTBUFSIZ/2) - 1;
        }
        update(meFALSE);
        mlisDisp(srchPat,status);
#if MEOPT_CALLBACK
input_cont:
#endif
        if((status == meFALSE) && (flags & ISCANNER_EOBQUIT))
        {
            if(flags & ISCANNER_BACKW)      /* if backward               */
                c = windowGotoBob(0,1);          /* and move to the top       */
            else
                c = windowGotoEob(0,1);          /* and move to the bottom    */
            c = 0;
            break;
        }
        c = meGetKey(meGETKEY_SILENT|meGETKEY_COMMAND);
    }
    
good_finish:
    if(srchPat[0] != '\0')
        addHistory(MLSEARCH,srchPat,meFALSE);
    lastReplace = 0;
    
bad_finish:
    frameCur->mlStatus = MLSTATUS_CLEAR;
    restoreUseMlBinds();
    mlerase(MWCLEXEC);
    if(c == 0)
        return meTRUE;
#if MEOPT_MWFRAME
    /* Check the focus has not changed before we execute the extra command, if
     * it has then change focus */
    {
        extern int commandDepth;
    
        /* if the user has changed the window focus using the OS
         * and this is the top level then change frames */
        if((frameFocus != NULL) && (frameFocus != frameCur) && (commandDepth == 1))
        {
            frameCur = frameFocus;
            frameFocus = NULL;
        }
    }
#endif
    lastflag = 0;                                 /* Fake last flags.     */
    return execute(c,meFALSE,1);                 /* Execute last command. */

quit_finish:
    frameCur->mlStatus = MLSTATUS_CLEAR;
    restoreUseMlBinds();
    return meFALSE;
}


/*
 * Subroutine to do incremental reverse search.  It actually uses the
 * same code as the normal incremental search, as both can go both ways.
 */
int
isearchBack(int f, int n)
{
    isHeadStr[0] = 'b';
    /* Call ISearch backwards */
    f = ISCANNER_QUIET|ISCANNER_BACKW|ISCANNER_PTBEG;
    if(n == 0)
        f |= ISCANNER_EOBQUIT;
    return isearch(f);
}

/* Again, but for the forward direction */

int
isearchForw(int f, int n)
{
    isHeadStr[0] = 'f';
    /* Call ISearch forwards */
    f = ISCANNER_QUIET;
    if(n == 0)
        f |= ISCANNER_EOBQUIT;
    return isearch(f);
}

#endif
