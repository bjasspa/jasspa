/*
 * Last Modified : <011025.2328>
 * 
 * The functions in this file implement commands that perform incremental
 * searches in the forward and backward directions.  This "ISearch" command
 * is intended to emulate the same command from the original EMACS 
 * implementation (ITS).  Contains references to routines internal to
 * SEARCH.C.
 *
 * REVISION HISTORY:
 *
 *      D. R. Banks 9-May-86
 *      - added ITS EMACSlike ISearch
 *
 *      John M. Gamble 5-Oct-86
 *      - Made iterative search use search.c's scanner() routine.
 *        This allowed the elimination of bakscan().
 *      - Put isearch constants into esearch.h
 *      - Eliminated the passing of 'status' to scanmore() and
 *        checknext(), since there were no circumstances where
 *        it ever equalled FALSE.
 *
 *  Steven W. Phillips 29-Apr-94
 *  - Made it work!
 *  - It now exits search on all other commands except ^R,^S,^Q
 *  - uses TTbell like the rest
 *  - doesnt bell all the time, only on first failure
 *  - searching again (^R,^S) when already fails sets the starting
 *    position to the top or bottom.
 *  - the first backspace after failed search resets the search
 *    string to its last successful string.
 *  - created ^W function which takes the next word in as a search string 
 * 
 *****************************************************************************
 * Modifications to the original file by Jasspa. 
 * 
 * Copyright (C) 1988 - 1999, JASSPA 
 * The MicroEmacs Jasspa distribution can be copied and distributed freely for
 * any non-commercial purposes. The MicroEmacs Jasspa Distribution can only be
 * incorporated into commercial software with the expressed permission of
 * JASSPA.
 * 
 ****************************************************************************/


#define __ISEARCHC                   /* Define the filename */

#include "emain.h"
#include "efunc.h"
#include "eskeys.h"
#include "esearch.h"

#if ISRCH

static uint8 isHeadStr[12]="f-isearch: " ;
static uint8 *statusStr[5]={
    (uint8 *)" [LOST]",(uint8 *)" [FOUND]",(uint8 *)" [INCOMPLETE]",(uint8 *)" [REGEX ERROR]",(uint8 *)" [SEARCHING]"
} ;

static int
isearchGotoEnd(uint8 *patrn, int flags, int histNo, int16 *histLen, SCANNERPOS *histPos)
{
    if(histLen[histNo] == 0)
        return TRUE ;
#ifdef _IPIPES
    /* due to the dynamic nature of an active ipipe buffer, things have to
     * be done differently */
    if(flags & ISEARCH_PIPE)
    {
        LINE *tmpline;
        int   tmpoff;
        int32 tmplno;
        int8 cc ;
        tmpline = curwp->w_dotp ;       /* Store the current position*/
        tmpoff  = curwp->w_doto ;       /* incase its not found      */
        tmplno  = curwp->line_no ;
        if(flags & ISCANNER_BACKW)
            WforwChar(curwp, 1) ;
        cc = patrn[histLen[histNo]] ;
        patrn[histLen[histNo]] = '\0' ;
        if(iscanner(patrn, 0,flags,histPos+histNo+1) != TRUE)
        {
            patrn[histLen[histNo]] = cc ;
            curwp->w_dotp  = tmpline;       /* Reset the position */
            curwp->w_doto  = tmpoff;
            curwp->line_no = tmplno;
            return FALSE ;
        }
        patrn[histLen[histNo]] = cc ;
    }
    else
#endif
    {
        curwp->w_dotp  = histPos[histNo].endline ;
        curwp->w_doto  = histPos[histNo].endoffset ;
        curwp->line_no = histPos[histNo].endline_no ;
    }
    return TRUE ;
}
static int
isearchGotoStart(uint8 *patrn, int flags, int histNo, int16 *histLen, SCANNERPOS *histPos)
{
    if(histLen[histNo] == 0)
        return TRUE ;
#ifdef _IPIPES
    /* due to the dynamic nature of an active ipipe buffer, things have to
     * be done differently */
    if(flags & ISEARCH_PIPE)
    {
        LINE *tmpline;
        int   tmpoff;
        long  tmplno;
        uint8 cc ;
        tmpline = curwp->w_dotp ;       /* Store the current position*/
        tmpoff  = curwp->w_doto ;       /* incase its not found      */
        tmplno  = curwp->line_no ;
        if(!(flags & ISCANNER_BACKW))
            WforwChar(curwp, 1) ;
        cc = patrn[histLen[histNo]] ;
        patrn[histLen[histNo]] = '\0' ;
        if(iscanner(patrn,0,(flags ^ (ISCANNER_BACKW|ISCANNER_PTBEG)),histPos+histNo+1) != TRUE)
        {
            patrn[histLen[histNo]] = cc ;
            curwp->w_dotp  = tmpline;       /* Reset the position */
            curwp->w_doto  = tmpoff;
            curwp->line_no = tmplno;
            return FALSE ;
        }
        patrn[histLen[histNo]] = cc ;
    }
    else
#endif
    {
        curwp->w_dotp  = histPos[histNo].startline ;
        curwp->w_doto  = histPos[histNo].startoff ;
        curwp->line_no = histPos[histNo].startline_no ;
    }
    return TRUE ;
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
static int
scanmore(uint8 *patrn, int flags, int histNo, int16 *histLen, SCANNERPOS *histPos)
{
    int sts;                            /* search status */
    
    /* Must flag as seaching, even if current status is ABORT
     * cos if the char is a ']' then the search may continue */
    mlDisp(isHeadStr,patrn,statusStr[4],-1) ;
    if(flags & ISEARCH_SCANMR)
    {
        isearchGotoStart(patrn,flags,histNo,histLen,histPos) ;
        if(flags & ISCANNER_BACKW)      /* reverse search? */
            WforwChar(curwp, 1) ;
    }
    sts = iscanner(patrn, 0,flags,histPos+histNo+1) ;
    
    if(sts != TRUE)
    {
        memcpy(histPos+histNo+1,histPos+histNo,sizeof(SCANNERPOS)) ;
        if(sts < 0)
            sts += 4 ;
        
        if(flags & ISEARCH_SCANMR)
            isearchGotoEnd(patrn,flags,histNo,histLen,histPos) ;
        
        if(!TTbreakFlag && (sts == FALSE))
            sts |= 0x80 ;
    }
    return sts ;                  /* else, don't even try */
}

#define mlisDisp(pat,status)                                                 \
do {                                                                         \
    mlDisp(isHeadStr,pat,statusStr[(status & 0x3)],-1) ;                   \
    mlStatus=MLSTATUS_KEEP ;                                                 \
    if(status & 0x80)                                                        \
    {                                                                        \
        TTbell() ; /* Feep if search fails       */                          \
        if(kbmd == PLAY)                                                     \
        {                                                                    \
            mlStatus = MLSTATUS_CLEAR ;                                      \
            useMlBinds = oldUseMlBinds ;                                     \
            mlerase(MWCLEXEC) ;                                              \
            return FALSE ;                                                   \
        }                                                                    \
        status &= 0x7f ;                                                     \
    }                                                                        \
    mwResetCursor() ;                                                        \
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
    uint8         status;       /* Search status */
    uint32        arg;          /* argument */
    int           cpos;         /* character number in search string */
    int           c;            /* current input character */
    LINE         *tmpline;      /* Tempory storage for wrap searching*/
    int           tmpoff;
    int32         tmplno;
    SCANNERPOS    histPos[HISTBUFSIZ] ;
    int16         histLen[HISTBUFSIZ] ;
    uint8         histStt[HISTBUFSIZ], kbmd=kbdmode ;
    int16         histNo=0 ;
    uint8         oldUseMlBinds = useMlBinds ;
    
    /* Initialize starting conditions */
    useMlBinds = 1 ;                 /* Use the ml-bind-key's             */
    
    /* Use search's srchPat array so that the hunt functions will work */
    srchPat[0]  = '\0' ;
    histPos[0].startline = 
              histPos[0].endline = curwp->w_dotp;       /* Save the current line pointer     */
    histPos[0].startoff  = 
              histPos[0].endoffset = curwp->w_doto;     /* Save the current line pointer     */
    histPos[0].startline_no = 
              histPos[0].endline_no = curwp->line_no;   /* Save the current line pointer     */
    histLen[0]  = 0 ;                                   /* Save the current offset */
    histStt[0]  = TRUE ;
    
    /* ask the user for the text of a pattern */
    isHeadStr[9] = '\0' ;
    mlwrite(MWCURSOR,(uint8 *)"%s (default [%s]): ",isHeadStr,
            (numSrchHist==0) ? (uint8 *)"":srchHist[0]) ;
    /* switch on the status so we save it */
    mlStatus = MLSTATUS_KEEP|MLSTATUS_POSML ;
    if(meModeTest(curbp->b_mode,MDMAGIC))
        flags |= ISCANNER_MAGIC ;
    if(meModeTest(curbp->b_mode,MDEXACT))
        flags |= ISCANNER_EXACT ;
#ifdef _IPIPES
    /* due to the dynamic nature of an active ipipe buffer, things have to
     * be done differently */
    if(meModeTest(curbp->b_mode,MDPIPE))
        flags |= ISEARCH_PIPE ;
#endif    
        
    /*
     * Get the first character in the pattern. If we get an initial C-s or
     * C-r, re-use the old search string and find the first occurrence
     */

get_another_key:
    /* Get the first character   */
    c = meGetKeyFromUser (FALSE, 0, meGETKEY_SILENT|meGETKEY_COMMAND);
    switch(decode_key((uint16) c,&arg))
    {
    case CK_NEWLIN:                     /* ^M : New line. do other search */
        useMlBinds = oldUseMlBinds ;
        if(flags & ISCANNER_BACKW)
            return searchBack(FALSE, 1) ;
        else
            return searchForw(FALSE, 1) ;

    case CK_GOEOP:    /* M-n - Got to next in history list */
    case CK_FORLIN:   /* C-n - next match in complete list */
    case CK_GOBOP:    /* M-p - Got to previous in history list */
    case CK_BAKLIN:   /* C-p - previous match in complete list */
    case CK_YANK:     /* C-y - yank the contents of the kill buffer */
        mlfirst = c ;
        if ((status = meGetString(isHeadStr, MLSEARCH|MLISEARCH, 0, srchPat, NPAT)) != TRUE)
        {
            useMlBinds = oldUseMlBinds ;
            return status ;
        }
        isHeadStr[9] = ':' ;
        if(mlfirst < 0)
        {
            status = scanmore(srchPat,flags,0,histLen,histPos) ;  /* do 1 search     */
            mlisDisp(srchPat,status) ;
            mlStatus = MLSTATUS_CLEAR ;
            useMlBinds = oldUseMlBinds ;
            return (status == TRUE) ;
        }
        c=mlfirst ;
        mlfirst = -1 ;
        break ;

    case CK_FISRCH:
    case CK_FORSRCH:
    case CK_BISRCH:
    case CK_BAKSRCH:
        if(numSrchHist)
            meStrcpy(srchPat,srchHist[0]) ;
        break ;
    case CK_VOIDFUNC:
        /* could be a pick on a mouse and we want the drop - so try again */
        goto get_another_key ;
    }
    
    cpos = meStrlen(srchPat)  ;
    isHeadStr[9] = ':' ;
    status = TRUE;                        /* Assume everything's cool   */
    mlisDisp(srchPat,status) ;
    /* Top of the per character loop */

    for(;;)
    {
        switch(decode_key((uint16) c,&arg))
        {
        case CK_ABTCMD: /* ^G : Abort input and return */
#ifdef _IPIPES
            if(!(flags & ISEARCH_PIPE))
#endif    
            {
                curwp->w_dotp  = histPos[0].endline ;
                curwp->w_doto  = histPos[0].endoffset ;
                curwp->line_no = histPos[0].endline_no ;
                curwp->w_flag |= WFMOVEL;       /* Say that we've moved      */
            }
            goto bad_finish ;                   /* Finish processing */

        case CK_BISRCH:
        case CK_BAKSRCH:                        /* If backward search        */
            tmpoff = ISCANNER_BACKW ;           /*  go forward               */
            isHeadStr[0] = 'b' ;
            goto find_next ;

        case CK_FISRCH:
        case CK_FORSRCH:                        /* If forward search         */
            isHeadStr[0] = 'f' ;
            tmpoff = 0;                         /* go forward                */
find_next:
            if(status >= 2)
            {
                TTbell() ;
                break ;
            }
            if((flags & ISCANNER_BACKW) != tmpoff)
            {
                flags ^= ISCANNER_BACKW|ISCANNER_PTBEG ;
                if(status == FALSE)
                    status = TRUE ;
            }
            tmpline = curwp->w_dotp ;       /* Store the current position*/
            tmpoff  = curwp->w_doto ;       /* incase its not found      */
            tmplno  = curwp->line_no ;
            /* The status cannot be ABORT, only FALSE or TRUE */
            if(!status)                         /* if already lost goto start*/
            {
                if(flags & ISCANNER_BACKW)      /* if backward               */
                    c = gotoeob(0,1) ;          /* and move to the bottom    */
                else
                    c = gotobob(0,1) ;          /* and move to the top       */
            }
            else if((histPos[histNo].startoff == histPos[histNo].endoffset) &&
                    (histPos[histNo].startline_no == histPos[histNo].endline_no))
            {
                /* the search string has matched a zero length string, e.g. "^"
                 * so move the cursor position one character */ 
                if(flags & ISCANNER_BACKW)      /* if backward               */
                    c = WbackChar(curwp,1) ;
                else
                    c = WforwChar(curwp,1) ;
            }
            else
                c = TRUE ;
            if((c == TRUE) &&
               (c=scanmore(srchPat,flags,histNo,histLen,histPos)) != TRUE) /* Start the search again    */
            {
                if(TTbreakFlag)
                {
                    c = 0 ;
                    goto bad_finish ;           /* Finish processing */
                }
                curwp->w_dotp  = tmpline;   /* Reset the position        */
                curwp->w_doto  = tmpoff;
                curwp->line_no = tmplno;
            }
            status = c ;
            histNo++ ;
            histLen[histNo] = cpos ;            /* Save the current srchPat len */
            histStt[histNo] = (status & 0x7f);  /* Save the current status   */
            break ;                             /* Go continue with the srch */

        case CK_QUOTE:                          /* ^Q - quote next character */
            if((c = quoteKeyToChar(meGetKeyFromUser(FALSE,0,meGETKEY_SILENT|meGETKEY_SINGLE))) < 0)
            {
                TTbell() ;
                break ;
            }
            goto isAddChar ;

        case CK_NEWLIN:                         /* ^M : New line. */
            c = 0 ;
            goto good_finish ;                  /* Finish processing */

        case CK_KILREG:                         /* ^W : Kill the whole line? */
            if(status == TRUE)
            {
                int tt ;
                
                if(flags & ISCANNER_BACKW)
                    isearchGotoEnd(srchPat,flags,histNo,histLen,histPos) ;
                
                /* use function from word.c  */
                tt = (inWord()==0) ;
                while(((inWord()==0)==tt) && (cpos < NPAT))    
                {
                    if(llength(curwp->w_dotp) == curwp->w_doto)
                        c = meNLCHAR ;
                    else
                        c = lgetc(curwp->w_dotp, curwp->w_doto);
                    if(meModeTest(curbp->b_mode,MDMAGIC) &&
                       ((c == '[') || (c == '.') || (c == '+') || (c == '*') || (c == '^') || (c == '$') || (c == '?') || (c == '\\')))
                        srchPat[cpos++] = '\\' ;
                    srchPat[cpos++] = c ;
                    if(WforwChar(curwp, 1) == FALSE)
                        break ;
                }
                srchPat[cpos] = 0;                      /* null terminate the buffer */
                memcpy(histPos+histNo+1,histPos+histNo,sizeof(SCANNERPOS)) ;
                histNo++ ;
                histLen[histNo]  = cpos ;               /* Save the current srchPat len */
                histPos[histNo].endline = curwp->w_dotp ;
                histPos[histNo].endoffset = curwp->w_doto ;
                histPos[histNo].endline_no = curwp->line_no ;
                
                if(flags & ISCANNER_BACKW)
                    isearchGotoStart(srchPat,flags,histNo,histLen,histPos) ;
                goto is_redraw ;
            }
            break ;
        case CK_RECENT:                         /* ^L : Redraw the screen    */
            sgarbf = TRUE;
            update(TRUE) ;
            mlerase(0);
            goto is_redraw ;
            
        case CK_DELBAK:                         /* ^H : backwards delete.    */
            if((--histNo) < 0)
            {
                c = 0 ;
                goto bad_finish ;               /* Finish processing */
            }
            cpos           = histLen[histNo] ;
            status         = histStt[histNo] ;
#ifdef _IPIPES
            /* due to the dynamic nature of an active ipipe buffer, things have to
             * be done differently */
            if(flags & ISEARCH_PIPE)
            {
                if(histLen[histNo] == histLen[histNo+1])
                {
                    /* was it a swap around */
                    if((histPos[histNo].endline == histPos[histNo+1].startline) &&
                       (histPos[histNo].endoffset == histPos[histNo+1].startoff))
                    {
                        flags ^= ISCANNER_BACKW|ISCANNER_PTBEG ;
                        isearchGotoEnd(srchPat,flags,histNo,histLen,histPos) ;
                    }
                    else
                    {
                        if(histStt[histNo+1] == TRUE)
                        {
                            isearchGotoStart(srchPat,flags,histNo,histLen,histPos+1) ;
                            if(histStt[histNo] == TRUE)
                                iscanner(srchPat,0,(flags ^ (ISCANNER_BACKW|ISCANNER_PTBEG)),&(histPos[histNo])) ;
                            isearchGotoEnd(srchPat,flags,histNo,histLen,histPos) ;
                        }
                    }
                        
                }
                else
                {
                    if(histStt[histNo+1] == TRUE)
                    {
                        isearchGotoStart(srchPat,flags,histNo,histLen,histPos+1) ;
                        srchPat[cpos] = 0 ;
                        if(((curwp->w_dotp != histPos[histNo].startline) ||
                            (curwp->w_doto != histPos[histNo].startoff)) && (histStt[histNo] == TRUE))
                            iscanner(srchPat,0,(flags ^ (ISCANNER_BACKW|ISCANNER_PTBEG)),&(histPos[histNo])) ;
                        isearchGotoEnd(srchPat,flags,histNo,histLen,histPos) ;
                    }
                }
            }
            else
#endif
            {
                curwp->w_dotp  = histPos[histNo].endline ;
                curwp->w_doto  = histPos[histNo].endoffset ;
                curwp->line_no = histPos[histNo].endline_no ;
            }
            srchPat[cpos] = 0 ;
            curwp->w_flag |= WFMOVEL;           /* Say that we've moved      */
is_redraw:
#ifdef _IPIPES
            /* due to the dynamic nature of an active ipipe buffer, things have to
             * be done differently */
            if(flags & ISEARCH_PIPE)
            {
                isearchGotoStart(srchPat,flags,histNo,histLen,histPos) ;
                histPos[histNo].startline = curwp->w_dotp ;
                histPos[histNo].startoff = curwp->w_doto ;
                histPos[histNo].startline_no = curwp->line_no ;
                isearchGotoEnd(srchPat,flags,histNo,histLen,histPos) ;
                histPos[histNo].endline = curwp->w_dotp ;
                histPos[histNo].endoffset = curwp->w_doto ;
                histPos[histNo].endline_no = curwp->line_no ;
            }
#endif
            setShowRegion(curbp,
                          histPos[histNo].startline_no,histPos[histNo].startoff,
                          histPos[histNo].endline_no,histPos[histNo].endoffset) ;
            addModeToBufferWindows(curbp, WFSELHIL);
            break ;
            
        default:                                /* All other chars           */
            if((c < 0x20) || (c > 0xff))        /* Is it printable?          */
            {
                /* must ignore [SCA]-pick and [SCA]-drop */
                if(((c & ~(ME_SHIFT|ME_CONTROL|ME_ALT)) == (ME_SPECIAL|SKEY_pick)) ||
                   ((c & ~(ME_SHIFT|ME_CONTROL|ME_ALT)) == (ME_SPECIAL|SKEY_drop)))
                    goto input_cont ;           /* Ignore this one, get another */
                goto good_finish ;              /* Nope. - Finish processing */
            }
isAddChar:
            srchPat[cpos++] = c;                /* put the char in the buffer*/
            if(cpos >= NPAT)                    /* too many chars in string? */
            {                                   /* Yup.  Complain about it   */
                addHistory(MLSEARCH, srchPat) ;
                mlwrite(MWABORT,(uint8 *)"[Search string too long!]");
                goto quit_finish ;
            }
            srchPat[cpos] = 0;                  /* null terminate the buffer */
            if(status != FALSE)                 /* If not lost last time     */
            {
                /*  or find the next match   */
                status = scanmore(srchPat,flags|ISEARCH_SCANMR,histNo,histLen,histPos) ;
                if(TTbreakFlag)
                    goto quit_finish ;          /* Finish processing */
            }
            else
                memcpy(histPos+histNo+1,histPos+histNo,sizeof(SCANNERPOS)) ;
            histNo++ ;
            histLen[histNo] = cpos ;            /* Save the current srchPat len */
            histStt[histNo] = (status&0x7f) ;   /* Save the current status   */
            break ;
        }  /* Switch */
        if(histNo == HISTBUFSIZ-1)
        {
            /* history is full to bust, best thing to do is ditch the first
             * half and thus make it half full.
             */
            memcpy(histPos,histPos+(HISTBUFSIZ-(HISTBUFSIZ/2)),(HISTBUFSIZ/2)*sizeof(SCANNERPOS)) ;
            memcpy(histLen,histLen+(HISTBUFSIZ-(HISTBUFSIZ/2)),(HISTBUFSIZ/2)*sizeof(short)) ;
            memcpy(histStt,histStt+(HISTBUFSIZ-(HISTBUFSIZ/2)),(HISTBUFSIZ/2)*sizeof(unsigned char)) ;
            histNo = (HISTBUFSIZ/2) - 1 ;
        }
        update(FALSE) ;
        mlisDisp(srchPat,status) ;
input_cont:
        c = meGetKeyFromUser(FALSE, 0, meGETKEY_SILENT|meGETKEY_COMMAND) ;
    }
    
good_finish:
    if(srchPat[0] != '\0')
        addHistory(MLSEARCH,srchPat) ;
    lastReplace = 0 ;
    
bad_finish:
    mlStatus = MLSTATUS_CLEAR ;
    useMlBinds = oldUseMlBinds ;
    mlerase(MWCLEXEC) ;
    if(c == 0)
        return TRUE ;
    lastflag = 0;                               /* Fake last flags.     */
    return execute(c,FALSE,1) ;                 /* Execute last command. */

quit_finish:
    mlStatus = MLSTATUS_CLEAR ;
    useMlBinds = oldUseMlBinds ;
    return FALSE ;
}


/*
 * Subroutine to do incremental reverse search.  It actually uses the
 * same code as the normal incremental search, as both can go both ways.
 */
int
isearchBack(int f, int n)
{
    isHeadStr[0] = 'b' ;
    /* Call ISearch backwards */
    return isearch(ISCANNER_QUIET|ISCANNER_BACKW|ISCANNER_PTBEG) ;
}

/* Again, but for the forward direction */

int
isearchForw(int f, int n)
{
    isHeadStr[0] = 'f' ;
    /* Call ISearch forwards */
    return isearch(ISCANNER_QUIET) ;
}

#endif

