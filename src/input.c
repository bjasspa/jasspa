/*
 *	SCCS:		%W%		%G%		%U%
 *
 *	Last Modified :	<010305.1315>
 *
 *	INPUT:	Various input routines for MicroEMACS 3.7
 *		written by Daniel Lawrence
 *		5/9/86
 *
 *	BUG:	meGetString() responds to backChar as back word. Backspace 
 *		does not delete previous character. Fix 05/11/90 JDG. 
 *
 *	BUG	mldisp() Core dumps if the search/replace string exceeds 
 *		50 characters. The 'space' variable has to be set up to
 *		PROMSIZ - 1 to allow for the '\0'. Hence changed variable
 *		to be -1 to stop it core dumping.
 *		Fix 16/07/91 JDG. - It's taken me 6 months to find this
 *		bastard since SDB on core gave me a differnt address !!!
 *
 **************************************************************************
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

#define	__INPUTC			/* Define the name of the file */

#include "emain.h"
#include "efunc.h"
#include "evar.h"
#include "eskeys.h"

/*
 * Ask a yes or no question in the message line. Return either TRUE, FALSE, or
 * ABORT. The ABORT status is returned if the user bumps out of the question
 * with a ^G. Used any time a confirmation is required.
 */
int
mlCharReply(uint8 *prompt, int mask, uint8 *validList, uint8 *helpStr)
{
    int   inpType=0, cc ;
    uint8 buff[TOKENBUF] ;
    uint8 *pp=prompt ;
    
    for(;;)
    {
        if((clexec == FALSE) || inpType)
        {
            if((kbdmode != PLAY) || (kbdoff >= kbdlen))
            {
                /* We are going to get a key of the user!! */
                if(mask & mlCR_QUIT_ON_USER)
                    return -2 ;
                if(mask & mlCR_UPDATE_ON_USER)
                    update(TRUE) ;
                
                if(mlStatus & MLSTATUS_POSOSD)
                {
                    mlStatus = MLSTATUS_POSOSD ;
                    mlResetCursor() ;
                }
                else
                {
                    /* switch off the status cause we are replacing it */
                    mlStatus = 0 ;
                    mlwrite(((mask & mlCR_CURSOR_IN_MAIN) ? 0:MWCURSOR)|MWSPEC,pp) ;
                    pp = prompt ;
                    /* switch on the status so we save it */
                    mlStatus = (mask & mlCR_CURSOR_IN_MAIN) ? MLSTATUS_KEEP:(MLSTATUS_KEEP|MLSTATUS_POSML) ;
                    inpType = 2 ;
                }
            }
            cc = meGetKeyFromUser(FALSE,0,meGETKEY_SILENT|meGETKEY_SINGLE) ;
            mlStatus &= ~(MLSTATUS_KEEP|MLSTATUS_RESTORE|MLSTATUS_POSML) ;
            if(cc == breakc)
                return -1 ;
        }
        else
        {
            uint8 *ss = execstr ;
            execstr = token(execstr,buff) ;
            if((buff[0] == '@') && (buff[1] == 'm') && (buff[2] == 'x'))
            {
                cc = buff[3] ;
                execstr = token(execstr, buff);
                if(cc == 'a')
                    execstr = ss ;
                meStrcpy(resultStr,prompt) ;
                if(lineExec (0, 1, buff) != TRUE)
                    return -1 ;
                cc = resultStr[0] ;
            }
            else if((buff[0] == '@') && (buff[1] == 'm') && (buff[2] == 'n'))
            {
                /* if @mna (get all input from user) then rewind the execstr */
                if(buff[3] == 'a')
                    execstr = ss ;
                inpType = 1 ;
                continue ;
            }
            else
            {
                /* evaluate it */
                uint8 *res = getval(buff) ;
                if(res == abortm)
                    return -1 ;
                cc = *res ;
            }
        }
        
        if((mask & mlCR_QUOTE_CHAR) &&
           ((cc = quoteKeyToChar((uint16) cc)) < 0))
        {
            if(validList == NULL)
                return -1 ;
        }
        else
        {
            if(mask & mlCR_LOWER_CASE)
                cc = toLower(cc) ;
            
            if((cc == '?') && (helpStr != NULL))
                pp = helpStr ;
            else if((cc != '\0') &&
                    (!(mask & mlCR_ALPHANUM_CHAR) || isAlphaNum(cc)) &&
                    ((validList == NULL) || (meStrchr(validList,cc) != NULL)))
            {
                if (inpType == 2)
                {
                    meGetStringFromKey((uint16) cc,buff) ;
                    mlwrite(MWCURSOR,(uint8 *)"%s%s",prompt,buff) ;
                }
                return cc ;
            }
        }
        inpType = 1 ;
    }
}

int
mlyesno(uint8 *prompt)
{
    uint8 buf[MAXBUF] ;    /* prompt to user */
    int ret ;
    
    /* build and prompt the user */
    meStrcpy(buf,prompt) ;
    meStrcat(buf," [y/n]? ") ;

    ret = mlCharReply(buf,mlCR_LOWER_CASE,(uint8 *)"yn",NULL) ;
    
    if(ret == -1)
        return ctrlg(FALSE,1) ;
    else if(ret == 'n')
        return FALSE ;
    return TRUE ;
}

/*
 * These are very simple character inserting, character deleting, move
 * over or erase spaces forwards or backwards routines. They are used
 * by the meGetString routine to edit the command line.
 */

static int
cins(uint8 cc, uint8 *buf, int *pos, int *len, int max)
{
    /*
     * Insert a character "c" into the buffer pointed to by "buf" at
     * position pos (which we update), if the length len (which we
     * also update) is less than the maximum length "max".
     *
     * Return:  TRUE    if the character will fit
     *          FALSE   otherwise
     */
    int ll ;
    ll = meStrlen(buf);
    if(ll + 1 >= max)
        return FALSE ;
    
    if(*pos == ll)
    {
        /* The character goes on the end of the buffer. */
        buf[ll++] = cc;
        buf[ll] = '\0';
        *len = ll;
        *pos = ll;
    }
    else
    {
        /* The character goes in the middle of the buffer. */
        uint8 *ss, c1, c2 ;
        
        ss = buf + *pos ;
        c2 = cc ;
        
        do {
            c1 = c2 ;
            c2 = *ss ;
            *ss++ = c1 ;
        } while(c1 != '\0') ;
        
        /* update the pos & length */
        *len = ll+1;
        (*pos)++ ;
    }
    return TRUE ;
}

static void
cdel(uint8 *buf, int pos, int *len)
{
    /*
     * Delete the character at position 'pos' in the buffer, update len.
     */
    meStrcpy(&buf[pos], &buf[pos + 1]);
    buf[--*len] = '\0';
}

#define ERASE (-1)      /* erase whatever we move over	*/
#define LEAVE 1         /* leave whatever we move over	*/

/* buf		pointer to buffer 		*/
/* pos		position in above buffer	*/
/* len		length of buffer (ie no. of chars in it) */
/* state	either ERASE or LEAVE		*/
static void
fspace(uint8 *buf, int *pos, int *len, int state)
{
    /*
     * Move over or erase spaces in the buffer (depending on the value
     * of "state") pointed to by "buf", after the current cursor position,
     * given by "pos", updating the length (ie the number of characters
     * in the buffer).
     */
    register uint8 c;
    
    for(;;)
    {
        if(*pos >= *len)
            return;
        
        if((c = buf[*pos]) != ' ' && c!= '\t' && c != DIR_CHAR)
            return;
        
        if(state == ERASE)
            cdel(buf, *pos, len);
        else
            ++*pos;
    }
}

/* buf		pointer to buffer 		*/
/* pos		position in above buffer	*/
/* len		length of buffer (ie no. of chars in it) */
/* state	either ERASE or LEAVE		*/
static void
bspace(uint8 *buf, int *pos, int *len, int state)
{
    /*
     * Identical to fspace() except that we go backwards instead of
     * forwards.
     */
    register uint8 c;
    
    for(;;)
    {
        if(*pos == 0)
            return;
        
        if((c = buf[*pos - 1]) != ' ' && c != '\t' && c != DIR_CHAR)
            return;
        
        if(state == ERASE)
            cdel(buf, *pos - 1, len);
        
        --*pos;
    }
}

/* buf		pointer to buffer 		*/
/* pos		position in above buffer	*/
/* len		length of buffer (ie no. of chars in it) */
/* state	either ERASE or LEAVE		*/
static void
fword(uint8 *buf, int *pos, int *len, int state)
{
    /*
     * Move forward over a "word", erasing it if "state" is ERASE.
     * The defintion of "word" is very rudimentary. It is considered
     * to be delimited by a space or by the dir char "/" character.
     */
    register uint8 c;
    
    for(;;)
    {
        if(*pos >= *len)
            return ;
        
        if((c = buf[*pos]) == ' ' || c == '\t' || c == DIR_CHAR)
            return;
        
        if(state == ERASE)
            cdel(buf, *pos, len);
        else
            ++*pos;
    }
}

/* buf		pointer to buffer 		*/
/* pos		position in above buffer	*/
/* len		length of buffer (ie no. of chars in it) */
/* state	either ERASE or LEAVE		*/
static void
bword(uint8 *buf, int *pos, int *len, int state)
{
    /*
     * As for fword() but go backwards to the start, instead of forwards.
     */
    register uint8 c;
    
    for(;;)
    {
        if(*pos == 0)
            return;
        
        if((c = buf[*pos - 1]) == ' ' || c == '\t' || c == DIR_CHAR)
            return;
        
        if(state == ERASE)
            cdel(buf, *pos - 1, len);
        
        --*pos;
    }
}

/*
 * mldisp() - expand control characters in "prompt" and "buf" (ie \t -> <TAB>,
 * \n -> <NL> etc etc). The third argument, "cpos" causes mldisp() to return
 * the column position of the "cpos" character in the "buf" string, ie if
 * mldisp is called as:
 *
 *        mldisp("hello", "\n\nthere", 3);
 *
 * Then mldisp will return:
 *
 *        5 + 4 + 4 + 1 = 10
 *        ^   ^   ^   ^
 *        |   |   |   |
 *    lenght of   |   |   length of "t"
 *       prompt   |   |   ^
 *      "hello"   lenght  |
 *            of <NL> |
 *            ^        |
 *            |-------|
 *            |
 *            3 characters into "buf"
 *
 * If "cpos" is < 0 then the the length of the expanded prompt + buf
 * combination, or term.t_ncol is returned (the routine displays only
 * term.t_ncol characters of the expanded buffer.
 * Added continue string which is printed after buf.
 */

#define    RESTSIZ        200    /* length of buf expand buffer      */

/* prompt         prompt to be displayed      */
/* buf            buffer to expand controls from */
/* cont           continuing str to be displayed */
/* cpos           offset into "buf"          */
/* expChr         0=dont expand unprintables, use ?, 1=expand ?'s */
void
mlDisp(uint8 *prompt, uint8 *buf, uint8 *cont, int cpos)
{
    int    len ;               /* offset into current buffer   */
    char   expbuf[RESTSIZ+1];    /* expanded buf                 */
    int    start, col, maxCol ;
    int    promsiz ;
    
    col = -1 ;
    len = expandexp(-1,buf,RESTSIZ,0,(uint8 *)expbuf,cpos,&col,0) ;
    if(col < 0)
        col = len ;
    if(cont != NULL)
    {
        meStrncpy(expbuf+len,cont,RESTSIZ-len);
        expbuf[RESTSIZ] = '\0' ;
        len += strlen(expbuf+len) ;
    }
    /* switch off the status cause we are replacing it */
    mlStatus = 0 ;
    maxCol = TTncol ;
    promsiz = meStrlen(prompt) ;
    col += promsiz ;
    len += promsiz ;
    /*
     * Having expanded it, display it.
     *
     * "ret" which is the actual column number of the "cpos" character
     * is used as an indication of how to display the line. If it is
     * too long for the terminal to display then:
     *    if "ret" is between 0 and max-column-number
     *    then
     *        display with $ in right hand column
     *    else
     *        display with $ after prompt
     *
     * Be careful about putting characters in the lower left hand corner
     * of the screen as this may cause scrolling on some terminals.
     */
    if(col > maxCol - 1)
    {
        int div = maxCol >> 1 ;
        
        start = (((col - (div >> 1)) / div) * div) ;
        len -= start ;
        col -= start-1 ;
        maxCol-- ;
    }
    else
        start = 0 ;
    if(len > maxCol)
    {
        expbuf[start+maxCol-promsiz-1] = '$';
        expbuf[start+maxCol-promsiz] = '\0';
    }
    mlCol = col ;
    if(start >= promsiz)
        mlwrite(MWUSEMLCOL|MWCURSOR,(uint8 *)"%s%s",(start) ? "$":"",expbuf+start-promsiz) ;
    else
        mlwrite(MWUSEMLCOL|MWCURSOR,(uint8 *)"%s%s%s",(start) ? "$":"",prompt+start,expbuf) ;
    
    /* switch on the status so we save it */
    mlStatus = MLSTATUS_KEEP|MLSTATUS_POSML ;
}



/*    tgetc:    Get a key from the terminal driver, resolve any keyboard
 *              macro action
 */
static uint16
tgetc(void)
{
    uint16 cc ;    /* fetched character */

    /* if we are playing a keyboard macro back, */
    if (kbdmode == PLAY)
    {
kbd_rep:
        /* if there is some left... */
        if(kbdoff < kbdlen)
        {
            cc = (uint16) kbdptr[kbdoff++] ;
            if(cc == 0xff)
            {
                cc = kbdptr[kbdoff++] ;
                if(cc == 0x02)
                {
                    uint8 dd ;    /* fetched character */
                    cc = ((uint16) kbdptr[kbdoff++]) << 8 ;
                    if(((dd = kbdptr[kbdoff++]) != 0xff) ||
                       ((dd = kbdptr[kbdoff++]) != 0x01))
                        cc |= dd ;
                }
                else if(cc == 0x01)
                    cc = 0 ;
            }
            return cc ;
        }
        /* at the end of last repitition? */
        if (--kbdrep > 0)
        {
            /* reset the macro to the begining for the next rep */
            kbdoff = 0 ;
            goto kbd_rep ;
        }
        kbdmode = STOP;
#if MEUNDO
        undoContFlag++ ;
#endif
        /* force a screen update after all is done */
        update(FALSE);
    }

    if(kbdmode == RECORD)
    {
        /* get and save a key */
        do {
            /* fetch a character from the terminal driver */
            cc = TTgetc();
            /* if this is a mouse key, ignored while recording a macro - get another */
        } while(((cc & (ME_SPECIAL|0x00ff)) >= (ME_SPECIAL|SKEY_mouse_drop_1))  &&
                ((cc & (ME_SPECIAL|0x00ff)) <= (ME_SPECIAL|SKEY_mouse_time_3))) ;
        /* Each 'key' could take 5 chars to store - if we haven't got room
         * stop so we don't overrun the buffer */
        if(kbdlen > NKBDM - 5)
        {
            kbdmode = STOP;
            TTbell();
        }
        else
        {
            uint8 dd ;
            /* must store 0xaabb as ff,2,aa,bb
             * also must store 0x00ff as ff,ff & 0x0000 as 0xff01
             * Also 0x??00 stored as ff,2,??,ff,01
             */
            if(cc > 0xff)
            {
                kbdptr[kbdlen++] = 0xff ;
                kbdptr[kbdlen++] = 2 ;
                kbdptr[kbdlen++] = cc >> 8 ;
            }
            dd = (cc & 0xff) ;
            if(dd == 0xff)
            {
                kbdptr[kbdlen++] = 0xff ;
                kbdptr[kbdlen++] = 0xff ;
            }
            else if(dd == 0x0)
            {
                kbdptr[kbdlen++] = 0xff ;
                kbdptr[kbdlen++] = 0x01 ;
            }
            else
                kbdptr[kbdlen++] = dd ;
        }
    }
    else
        cc = TTgetc();

    /* and finally give the char back */
    return cc ;
}


static int
getprefixchar(int f, int n, int ctlc, int flag)
{
    uint8 buf[20] ;
    int c ;

    if(!(flag & meGETKEY_SILENT))
    {
        buf[meGetStringFromChar((uint16) ctlc,buf)] = '\0' ;
        if(f==TRUE)
            mlwrite(MWCURSOR,(uint8 *)"Arg %d: %s", n, buf);
        else
            mlwrite(MWCURSOR,(uint8 *)"%s", buf);
    }
    c = tgetc();
    if(!(c & (ME_SHIFT|ME_CONTROL|ME_ALT|ME_SPECIAL)))
        c = toLower(c) ;
    return c ;
}

/*    GETCMD:    Get a command from the keyboard. Process all applicable
        prefix keys
 */
uint16
meGetKeyFromUser(int f, int n, int flag)
{
    uint16 cc ;        /* fetched keystroke */
    int ii ;
    
    if(kbdmode == PLAY)
    {
        if(TTbreakTest(0))
        {
            ctrlg(FALSE,1) ;
#if MEUNDO
            undoContFlag++ ;
#endif
            /* force a screen update */
            update(0);
        }
        else
            flag |= meGETKEY_SILENT ;
    }
    if(f && !(flag & meGETKEY_SILENT))
        mlwrite(MWCURSOR,(uint8 *)"Arg %d:",n);

    /* get initial character */
    cc = tgetc();
    
    /* If we are in idle mode then disable now */
    if (kbdmode == KBD_IDLE)            /* In the idle state */
        kbdmode = STOP;                 /* Restore old state */

    if(!(flag & meGETKEY_SINGLE))
    {
        /* process prefixes */
        ii = ME_PREFIX_NUM+1 ;
        while(--ii > 0)
            if(cc == prefixc[ii])
            {
                cc = ((ii << ME_PREFIX_BIT) | getprefixchar(f,n,cc,flag)) ;
                break ;
            }
        if(flag & meGETKEY_COMMAND)
            thiskey = cc ;
    }
    /* return the key */
    return cc ;
}

static Fintssi curCmpIFunc ;
static Fintss  curCmpFunc ;

static int
getFirstLastPos(int noStr,uint8 **strs, uint8 *str, int option,
                int *fstPos, int *lstPos)
{
    register int nn ;
    
    if((nn = meStrlen(str)) == 0)
    {
        *fstPos = 0 ;
        *lstPos = noStr - 1 ;
        if(noStr <= 0)
            return 0 ;
    }
    else
    {
        register int ii, rr ;
        
        for(ii=0 ; ii<noStr ; ii++)
            if(!(rr = curCmpIFunc((char *)str,(char *)strs[ii],nn)))
                break ;
            else if(rr < 0)
                return 0 ;
        
        if(ii == noStr)
            return 0 ;
        
        *fstPos = ii ;
        for(++ii ; ii<noStr ; ii++)
            if(curCmpIFunc((char *)str,(char *)strs[ii],nn))
                break ;
        *lstPos = ii - 1 ;
    }
    if((option & MLFILE) && (fileIgnore != NULL))
    {
        /* try to eat away at the number of possible completions
         * until there is only one. If successful the return as
         * just one. Can remove fileIngore type files which are 
         * usually . .. and backup files (*~) etc.
         */
        register int ff, ll, ii=1, len, fil ;
        register uint8 *ss, *fi, cc ;
        ff = *fstPos ;
        ll = *lstPos ;
        
        while((ff <= ll) && (ii >= 0))
        {
            if(ii)
                ss = strs[ll] ;
            else
                ss = strs[ff] ;
            len = meStrlen(ss) ;
            fi = fileIgnore ;
            for(;;)
            {
                fil=1 ;
                while(((cc=fi[fil]) != ' ') && (cc != '\0'))
                    fil++ ;
                if((fil <= len) &&
                   !curCmpIFunc((char *)ss+len-fil,(char *)fi,fil))
                {
                    if(ii)
                        ll-- ;
                    else
                        ff++ ;
                    break ;
                }
                fi += fil ;
                if(*fi++ == '\0')
                {
                    ii-- ;
                    break ;
                }
            }
        }
        if(ff == ll)
            *fstPos = *lstPos = ff ;
    }
    return 1 ;
}

int
createBuffList(uint8 ***listPtr, int noHidden)
{
    register BUFFER *bp = bheadp ;    /* index buffer pointer    */
    register int     i, n ;
    register uint8 **list ;

    n = 0 ;
    while(bp != NULL)
    {
        if(!noHidden || !meModeTest(bp->b_mode,MDHIDE))
            n++;
        bp = bp->b_bufp ;
    }
    if((list = (uint8 **) meMalloc(sizeof(uint8 *) * n)) == NULL)
        return 0 ;
    bp = bheadp ;
    for(i=0 ; i<n ; )
    {
        if(!noHidden || !meModeTest(bp->b_mode,MDHIDE))
            list[i++] = bp->b_bname ;
        bp = bp->b_bufp ;
    }
    *listPtr = list ;
    return i ;
}
    
int
createVarList(uint8 ***listPtr)
{
    meVARIABLE *vptr;     	/* User variable pointer */
    int     ii, nn ;
    uint8  **list ;

    nn = NEVARS + usrVarList.count + curbp->varList.count ;
    
    if((list = (uint8 **) meMalloc(sizeof(uint8 *) * nn)) == NULL)
        return 0 ;
    *listPtr = list ;
    
    for(ii=0 ; ii<NEVARS; ii++)
    {
        if((list[ii] = meMalloc(meStrlen(envars[ii])+2)) == NULL)
            return 0 ;
        list[ii][0] = '$' ;
        meStrcpy(list[ii]+1,envars[ii]) ;
    }
    for(vptr=usrVarList.head ; vptr != NULL ; vptr = vptr->next,ii++)
    {
        if((list[ii] = meMalloc(meStrlen(vptr->name)+2)) == NULL)
            return 0 ;
        list[ii][0] = '%' ;
        meStrcpy(list[ii]+1,vptr->name) ;
    }
    for(vptr=curbp->varList.head ; vptr != NULL ; vptr = vptr->next,ii++)
    {
        if((list[ii] = meMalloc(meStrlen(vptr->name)+2)) == NULL)
            return 0 ;
        list[ii][0] = ':' ;
        meStrcpy(list[ii]+1,vptr->name) ;
    }
    return ii ;
}
    
int
createCommList(uint8 ***listPtr, int noHidden)
{
    meCMD *cmd ;
    register int ii ;
    register uint8 **list ;
    
    if((list = meMalloc(sizeof(uint8 *) * (cmdTableSize))) == NULL)
        return 0 ;
    ii = 0 ;
    cmd = cmdHead ;
    while(cmd != NULL)
    {
        if(!noHidden || (cmd->id < CK_MAX) ||
           !(((meMACRO *) cmd)->hlp->l_flag & MACHIDE))
            list[ii++] = cmd->name ;
        cmd = cmd->anext ;
    }
    *listPtr = list ;
    return ii ;
}


#if LCLBIND
uint8 oldUseMlBinds ;
#endif
uint8 **mlgsStrList ;
int    mlgsStrListSize ;
static WINDOW *mlgsOldCwp=NULL ;
static int32   mlgsSingWind=0 ;
static int     mlgsCursorState=0 ;
static uint8  *mlgsStoreBuf=NULL ;

static void
mlfreeList(int option, int noStrs, uint8 **strList)
{
    mlStatus = MLSTATUS_CLEAR ;
#if LCLBIND
    useMlBinds = oldUseMlBinds ;
#endif
    if(mlgsStoreBuf != NULL)
    {
        meFree(mlgsStoreBuf) ;
        mlgsStoreBuf = NULL ;
    }
    if(strList != NULL)
    {
        if(option & MLVARBL)
            freeFileList(noStrs,strList) ;
        else if(option & (MLBUFFER|MLCOMMAND))
            meFree(strList) ;
    }
    if(mlgsOldCwp != NULL)
    {
        BUFFER *bp=curbp ;
        if(mlgsSingWind)
        {
            delwind(FALSE,FALSE) ;
            curwp->topLineNo = mlgsSingWind-1 ;
        }
        else
        {
            BUFFER *rbp=NULL, *tbp=bheadp ;
            while(tbp != NULL)
            {
                if((tbp != bp) && ((rbp == NULL) || (rbp->histNo < tbp->histNo)))
                    rbp = tbp ;
                tbp = tbp->b_bufp ;
            }
            swbuffer(curwp,rbp) ;
            makeCurWind(mlgsOldCwp) ;
        }
        zotbuf(bp,1) ;
        mlgsOldCwp = NULL ;
    }
    if(mlgsCursorState < 0)
        showCursor(TRUE,mlgsCursorState) ;
}

static int
mlHandleMouse(uint8 *inpBuf, int inpBufSz, int compOff)
{
    int row, col ;
    
    if((mlgsOldCwp != NULL) &&
       ((row=mouse_Y-curwp->firstRow) >= 0) && (row < curwp->numRows-1) &&
       ((col=mouse_X-curwp->firstCol) >= 0) && (col < curwp->numCols))
    {
        if (col >= curwp->numTxtCols)
        {
            /* only do scroll bar if on pick and bars are enabled */
            if((inpBuf == NULL) && (curwp->w_mode & WMSCROL))
            {
                int ii ;
                for (ii = 0; ii <= (WCVSBML-WCVSBSPLIT); ii++)
                    if (mouse_Y < (int16) curwp->w_sbpos[ii])
                        break;
                if(ii == (WCVSBUP-WCVSBSPLIT))
                    scrollUp(1,1) ;
                else if(ii == (WCVSBUSHAFT-WCVSBSPLIT))
                    scrollUp(0,1) ;
                else if(ii == (WCVSBDOWN-WCVSBSPLIT))
                    scrollDown(1,1) ;
                else if(ii == (WCVSBDSHAFT-WCVSBSPLIT))
                    scrollDown(0,1) ;
                update(TRUE) ;
            }
        }
        else
        {
            LINE *lp ;
            int ii, jj, lineNo ;
            
            row += curwp->topLineNo ;
            lineNo = row ;
            lp = curbp->b_linep->l_fp ;
            while(--row >= 0)
                lp = lforw(lp) ;
            if((lp->l_flag & LNNEOL) && (col >= curwp->w_marko))
                ii = curwp->w_marko ;
            else
                ii = 0 ;
            if((ii == 0) && (lp->l_flag & LNNEOL))
            {
                jj = 1 ;
                while((lgetc(lp,jj) != ' ') || (lgetc(lp,jj+1) != ' '))
                    jj++ ;
            }
            else
                jj = llength(lp) ;
            if(jj > col)
            {
                curwp->w_dotp = lp ;
                curwp->w_doto = ii ;
                curwp->line_no = lineNo ;
                setShowRegion(curbp,lineNo,ii,lineNo,jj) ;
                curwp->w_flag |= WFMOVEL|WFSELHIL ;
                if(inpBuf != NULL)
                {
                    if((jj -= ii) >= (inpBufSz-compOff))
                        jj = inpBufSz-compOff-1 ;
                    /* if we already have this, assume the user has double clicked
                     * to 'select', so return 2 to exit */
                    if((inpBuf[compOff+jj] == '\0') && 
                       !meStrncmp(inpBuf+compOff,ltext(lp)+ii,jj))
                        return 2 ;
                    meStrncpy(inpBuf+compOff,ltext(lp)+ii,jj) ;
                    inpBuf[compOff+jj] = '\0' ;
                }
                update(TRUE) ;
                return 1 ;
            }
        }
    }
    return 0 ;
}

uint8 *compSole     = (uint8 *)" [Sole completion]" ;
uint8 *compNoMch    = (uint8 *)" [No match]" ;
uint8 *compNoExp    = (uint8 *)" [No expansion]" ;
uint8 *compFailComp = (uint8 *)" [Failed to create]" ;

#if MEOSD
#define mlgsDisp(prom,buf,contstr,ipos) \
((mlStatus & MLSTATUS_POSOSD) ? osdDisp(buf,contstr,ipos):mlDisp(prom,buf,contstr,ipos))
#else
#define mlgsDisp(prom,buf,contstr,ipos) \
(mlDisp(prom,buf,contstr,ipos))
#endif
/* prompt - prompt associated with what we're typing         */
/* option - bit field which modifies our behaviour slightly  */
/* defnum - the default no. in history (0 = no default)      */
/* buf    - where it all goes at the end of the day          */
/* nbuf   - amount of space in buffer                        */
int
meGetStringFromUser(uint8 *prompt, int option, int defnum, uint8 *buf, int nbuf)
{
    register int cc ;
    int     ii ;
    int     ipos ;                      /* input position in buffer */
    int     ilen ;                      /* number of chars in buffer */
    int     cont_flag ;                 /* Continue flag */
    uint8 **history ;
    uint8   onHist, numHist, *numPtr ;
    uint8  *defaultStr ;
    uint8   prom[MAXBUF] ;
    uint8   ch, **strList ;
    uint8  *contstr=NULL ;
    int     gotPos=1, fstPos, lstPos, curPos, noStrs ;
    int     changed=1, compOff=0 ;
    
    if((mlgsStoreBuf = meMalloc(nbuf)) == NULL)
       return ABORT ;

    mlerase(0);            /* Blank line to force update */

    if(option & MLINSENSCASE)
    {
        curCmpFunc  = stridif ;
        curCmpIFunc = strnicmp ;
    }
    else
    {
        curCmpFunc  = strcmp ;
        curCmpIFunc = strncmp ;
    }
    numHist = setupHistory(option, &numPtr, &history) ;
    if(option & MLBUFFER)
        noStrs = createBuffList(&strList,1) ;
    else if(option & MLVARBL)
        noStrs = createVarList(&strList) ;
    else if(option & MLCOMMAND)
        noStrs = createCommList(&strList,1) ;
    else if(option & MLUSER)
    {
        strList = mlgsStrList ;
        noStrs = mlgsStrListSize ;
    }
    else
    {
        strList = NULL ;
        noStrs = 0 ;
    }
    if(strList != NULL)
        sortStrings(noStrs,strList,0,curCmpFunc) ;

    if(option & MLNOHIST)
        onHist = ~0 ;
    else
        onHist = numHist+1 ;
    meStrcpy(prom,prompt) ;
    ii = meStrlen(prom) ;
    if((defnum > 0) && (defnum <= numHist))
    {
        defaultStr = history[defnum-1] ;
        meStrncpy(prom+ii," (default [",11) ;
        ii = expandexp(-1,defaultStr,MAXBUF-5,ii+11,prom,-1,NULL,0) ;
        meStrcpy(prom+ii,"]): ") ;
    }
    else
    {
        defaultStr = NULL ;
        meStrcpy(prom+ii,": ") ;
    }

    if(option & MLNORESET)
    {
        ilen = meStrlen(buf) ;
        if(mlStatus & MLSTATUS_OSDPOS)
        {
            uint8 *s1, *s2 ;
            s1 = buf ;
            while((--osdRow >= 0) && ((s2 = meStrchr(s1,meNLCHAR)) != NULL))
                s1 = s2+1 ;
            if(osdRow >= 0)
                ipos = ilen ;
            else
            {
                ipos = (int) s1 - (int) buf + osdCol ;
                if(ipos > ilen)
                    ipos = ilen ;
                if(((s2 = meStrchr(s1,meNLCHAR)) != NULL) &&
                    (((int) s2 - (int) buf) < ipos))
                    ipos = (int) s2 - (int) buf ;
            }
        }
        else
            ipos = ilen ;
    }
    else
    {
        ipos = ilen = 0 ;
        buf[0] = '\0' ;
    }
#if LCLBIND
    oldUseMlBinds = useMlBinds ;
    useMlBinds = 1 ;
#endif
    if((mlgsCursorState=cursorState) < 0)
        showCursor(FALSE,1) ;
    for (cont_flag = 1; cont_flag != 0;)
    {
        uint32 arg ;
        int idx ;
        int ff ;
        
        mlgsDisp(prom, buf, contstr, ipos) ;
        contstr = NULL ;

        /*
        ** mlfirst  may be set by meShell. If both are unset, get a 
        ** character from the user. 
        */

        if(mlfirst != -1)
        {
            cc = mlfirst ;
            mlfirst = -1;
        }
        else
            cc = meGetKeyFromUser(FALSE,0,meGETKEY_SILENT) ;
        
        idx = decode_key((uint16) cc,&arg) ;
        if(arg)
        {
            ff = 1 ;
            ii = (int) (arg + 0x80000000) ;
        }
        else
        {
            ff = 0 ;
            ii = 1 ;
        }
        switch(idx)
        {
        case CK_GOBOL:          /* ^A : Move to start of buffer */
            if(mlStatus & MLSTATUS_NINPUT)
            {
                while(ipos && (buf[ipos-1] != meNLCHAR))
                    ipos-- ;
            }
            else
                ipos = 0;
            break;
            
        case CK_BAKCHR:    /* ^B : Backwards char */
            while(ipos && ii--)
                ipos--;
            break;
            
        case CK_DELFOR:    /* ^D : Delete the character under the cursor */
            while((ipos < ilen) && ii--)
            {
                cdel(buf, ipos, &ilen);
                /*                if(ipos > ilen)*/
                /*                    ipos = ilen;*/
                changed=1 ;
            }
            break;
            
        case CK_GOEOL:    /* ^E : Move to end of buffer */
            if(mlStatus & MLSTATUS_NINPUT)
            {
                while((ipos < ilen) && (buf[ipos] != meNLCHAR))
                    ipos++ ;
            }
            else
                ipos = ilen ;
            break;
            
        case CK_FORCHR:    /* ^F : Move forward one character */
            while((ipos < ilen) && ii--)
                ipos++;
            break;
            
        case CK_ABTCMD: /* ^G : Abort input and return */
            mlfreeList(option,noStrs,strList) ;
            return ctrlg(FALSE,1);
            
        case CK_DELBAK:    /* ^H : backwards delete. */
            while(ipos && ii--)
            {
                cdel(buf, ipos-1, &ilen);
                ipos--;
                changed=1 ;
            }
            break;
            
        case CK_DOTAB:    /* ^I : Tab character */
            if(mlStatus & MLSTATUS_NINPUT)
            {
                cont_flag = 0;
                break;
            }
            cc = '\t' ;    /* ^I for search strings */
            if(!(option & (MLCOMMAND | MLFILE | MLBUFFER | MLVARBL | MLUSER)))
                goto input_addexpand ;
input_expand:
            if(option & MLFILE)
            {
                uint8 fname[FILEBUF], *base ;
                
                pathNameCorrect(buf,PATHNAME_PARTIAL,fname,&base) ;
                meStrcpy(buf,fname) ;
                ipos = ilen = meStrlen(buf) ;
                /* if current buff is xxx/yyy/ then pathNameCorrect will
                 * return the path as xxx/ and base as yyy/ but for completion
                 * we want to list yyy/ so move the base for the case */
                if(buf[ilen-1] != DIR_CHAR)
                    *base = '\0' ;
                compOff = meStrlen(fname) ;
                getDirectoryList(fname,&curDirList) ;
                noStrs = curDirList.size ;
                strList = curDirList.list ;
            }
            if(strList == NULL)
            {
                contstr = compNoExp ;
                TTbell();
                break ;
            }
            if(changed)
            {
                changed = 0 ;
                if((gotPos = getFirstLastPos
                    (noStrs,strList,buf+compOff,option,&fstPos,&lstPos)) == 0)
                {
                    contstr = compNoMch ;
                    TTbell();
                    break ;
                }
                curPos = fstPos-1 ;
                if(fstPos == lstPos)
                {
                    meStrcpy(buf+compOff,(strList[lstPos])) ;
                    ilen += meStrlen(buf+ilen) ;
                    if((option & MLFILE) && (buf[ilen-1] == DIR_CHAR))
                        /* if we're entering a file name and we've just
                         * completed to the a directory name then the next
                         * 'TAB' completion should be for inside the new directory,
                         * so effectively we have changed the input point */
                        changed = 1 ;
                }
                else
                {
                    for(ii=fstPos ; ii<=lstPos ; ii++)
                        if(strList[ii][ilen-compOff] == cc)
                            goto input_addexpand ;
                    for(;;)
                    {
                        ch = strList[lstPos][ilen-compOff] ;
                        for(ii=fstPos ; ii<lstPos ; ii++)
                            if((strList[ii][ilen-compOff] != ch) &&
                               (((option & MLINSENSCASE) == 0) ||
                                ((uint8) toLower(strList[ii][ilen-compOff]) != (uint8) toLower(ch))))
                                break ;
                        if(ii != lstPos)
                            break ;
                        if(ch == '\0')
                            break ;
                        buf[ilen++] = ch ;
                    }
                    buf[ilen] = '\0' ;
                }
                ipos = ilen ;
            }
            else if(!gotPos)
                contstr = compNoMch ;
            else if(fstPos == lstPos)
                contstr = compSole ;
            else
            {
                uint8 line[150] ;
                int len, lwidth ;
                
                for(ii=fstPos ; ii<=lstPos ; ii++)
                    if(strList[ii][ilen-compOff] == cc)
                        goto input_addexpand ;
                if(mlgsOldCwp == NULL)
                {
                    if(wheadp->w_wndp == NULL)
                        mlgsSingWind = curwp->topLineNo+1 ;
                    else
                        mlgsSingWind = 0 ;
                    mlgsOldCwp = curwp ;
                }
                
                if(wpopup(BcompleteN,BFND_CREAT|BFND_CLEAR|WPOP_MKCURR) == NULL)
                {
                    contstr = compFailComp ;
                    break ;
                }
                /* remove any completion list selection hilighting */
                if(selhilight.bp == curbp)
                    selhilight.flags &= ~SELHIL_ACTIVE ;
                
                /* Compute the widths available from the window width */
                lwidth = curwp->numTxtCols >> 1;
                if (lwidth > 75)
                    lwidth = 75 ;
                curwp->w_marko = lwidth ;
                for(ii=fstPos ; ii<=lstPos ; ii++)
                {
                    uint8 flag ;
                    
                    if(((len= meStrlen(strList[ii])) < lwidth) && (ii<lstPos) &&
                       (((int) meStrlen(strList[ii+1])) < lwidth-1))
                    {
                        meStrcpy(line,strList[ii++]) ;
                        memset(line+len,' ',lwidth-len) ;
                        meStrcpy(line+lwidth,strList[ii]) ;
                        flag = LNNEOL ;
                    }
                    else
                    {
                        meStrncpy(line,strList[ii],148) ;
                        line[149] = '\0' ;
                        flag = 0 ;
                    }
                    addLineToEob(curbp,line) ;
                    curbp->b_linep->l_bp->l_flag |= flag ;
                }
                gotobob(FALSE,FALSE) ;
                update(TRUE) ;
            }
            break ;
            
        case CK_MOVUWND:
            if(mlgsOldCwp != NULL)
            {
                scrollUp(ff,ii) ;
                update(TRUE) ;
            }
            break ;
            
        case CK_MOVDWND:
            if(mlgsOldCwp != NULL)
            {
                scrollDown(ff,ii) ;
                update(TRUE) ;
            }
            break ;
            
        case CK_BAKLIN: /* M-P - previous match in complete list */
            if(mlStatus & MLSTATUS_NINPUT)
            {
                ii = ipos ;
                while((--ii >= 0) && (buf[ii] != meNLCHAR))
                    ;
                if(ii >= 0)
                {
                    int jj ;
                    jj = ipos - ii ;
                    ipos = ii ;
                    /* work out the new line offset */
                    while((--ii >= 0) && (buf[ii] != meNLCHAR))
                        ;
                    if((ii+jj) < ipos)
                        ipos = ii+jj ;
                }
            }
            else
            {
                if(ff || !(option & (MLCOMMAND | MLFILE | MLBUFFER | MLVARBL | MLUSER)))
                    goto mlgs_prevhist ;
                if((strList != NULL) && !changed && gotPos)
                {
                    if(fstPos == lstPos)
                        contstr = compSole ;
                    else
                    {
                        if(--curPos < fstPos)
                            curPos = lstPos ;
                        meStrncpy(buf+compOff,strList[curPos],nbuf-compOff) ;
                        buf[nbuf-1] = '\0' ;
                        ipos = ilen = meStrlen(buf) ;
                    }
                }
            }
            break ;
            
        case CK_FORLIN: /* ^N - next match in complete list */
            if(mlStatus & MLSTATUS_NINPUT)
            {
                ii = ipos ;
                while((ii < ilen) && (buf[ii] != meNLCHAR))
                    ii++ ;
                if(ii < ilen)
                {
                    int jj ;
                    jj = ipos ;
                    while(jj-- && (buf[jj] != meNLCHAR))
                        ;
                    jj = ipos-jj-1 ;
                    ipos = ii+1 ;
                    while((--jj >= 0) && (ipos < ilen) && (buf[ipos] != meNLCHAR))
                        ipos++ ;
                }
            }
            else
            {
                if(ff || !(option & (MLCOMMAND | MLFILE | MLBUFFER | MLVARBL | MLUSER)))
                    goto mlgs_nexthist ;
                if((strList != NULL) && !changed && gotPos)
                {
                    if(fstPos == lstPos)
                        contstr = compSole ;
                    else
                    {
                        if(++curPos > lstPos)
                            curPos = fstPos ;
                        meStrncpy(buf+compOff,strList[curPos],nbuf-compOff) ;
                        buf[nbuf-1] = '\0' ;
                        ipos = ilen = meStrlen(buf) ;
                    }
                }
            }
            break ;
            
        case CK_KILEOL:    /* ^K : Kill to end of line */
            if(ipos < ilen)
            {
                if(mlStatus & MLSTATUS_NINPUT)
                {
                    ii = ilen ;
                    while((ipos < ilen) && (buf[ipos] != meNLCHAR))
                        cdel(buf, ipos, &ilen);
                    if((ii==ilen) ||
                       (!meModeTest(globMode,MDLINE) &&         /* if line kill mode    */
                        (ipos < ilen) &&                        /* some chars left      */
                        (!ipos || (buf[ipos-1] == meNLCHAR))))  /* whole line           */
                        cdel(buf, ipos, &ilen);
                }
                else
                {
                    buf[ipos] = '\0';
                    ilen = ipos;
                }
                changed=1 ;
            }
            break;
            
        case CK_RECENT:  /* ^L : Redraw the screen */
            sgarbf = TRUE;
            update(TRUE) ;
            mlerase(0);
            break;
            
        case CK_NEWLIN:  /* ^J : New line. Finish processing */
            if(mlStatus & MLSTATUS_NINPUT)
            {
                cc = meNLCHAR ;
                goto input_addexpand ;
            }
            cont_flag = 0;
            break;
            
        case CK_GOEOP:    /* M-N : Got to next in history list */
            /* Note the history list is reversed, ie 0 most recent,
            ** (numHist-1) the oldest. However if numHist then its
            ** the current one (wierd but easiest to implement
            */
mlgs_nexthist:
            if(!(option & MLNOHIST) && (numHist > 0))
            {
                if(onHist > numHist)
                    /* if on current then save */
                    meStrcpy(mlgsStoreBuf,buf) ;
                if(onHist == 0)
                {
                    onHist = numHist+1 ;
                    meStrcpy(buf,mlgsStoreBuf) ;
                }
                else if(onHist > numHist)
                {
                    meStrcpy(prom+meStrlen(prompt),": ") ;
                    defaultStr = NULL ;
                    onHist = numHist ;
                    buf[0] = '\0' ;
                }
                else
                {
                    meStrncpy(buf,history[--onHist],nbuf) ;
                    buf[nbuf-1] = '\0' ;
                }                    
                ipos = ilen = meStrlen(buf) ;
                changed=1 ;
            }
            break;
            
        case CK_GOBOP:    /* M-P : Got to previous in history list */
mlgs_prevhist:
            if(!(option & MLNOHIST) && (numHist > 0))
            {
                if(onHist > numHist)
                {
                    /* if on current then save */
                    meStrcpy(mlgsStoreBuf,buf) ;
                    onHist = 0 ;
                }
                else
                    ++onHist ;
                if(onHist > numHist)
                    meStrcpy(buf,mlgsStoreBuf) ;
                else if(onHist == numHist)
                {
                    meStrcpy(prom+meStrlen(prompt),": ") ;
                    defaultStr = NULL ;
                    buf[0] = '\0' ;
                }
                else
                {
                    meStrncpy(buf,history[onHist],nbuf) ;
                    buf[nbuf-1] = '\0' ;
                }
                ipos = ilen = meStrlen(buf) ;
                changed=1 ;
            }
            break;
            
        case CK_TRNCHR:
            /* ^T : Transpose the character before the cursor
             *      with the one under the cursor.
             */
            if(ipos && ipos < ilen)
            {
                cc = buf[ipos - 1];
                buf[ipos - 1] = buf[ipos];
                buf[ipos] = cc;
                changed=1 ;
            }
            break;
            
        case CK_KILREG:    /* ^W : Kill the whole line */
            ipos = 0 ;
            ilen = 0 ;
            buf[ipos] = '\0';
            changed=1 ;
            break;
        case CK_QUOTE:    /* ^Q - quote the next character */
            if((cc = quoteKeyToChar(tgetc())) < 0)
            {
                TTbell() ;
                break ;
            }
            while(ii--)
                if(cins((uint8) cc, buf, &ipos, &ilen, nbuf) == FALSE)
                    TTbell();
            changed=1 ;
            break;
            
        case CK_OPNLIN:    /* ^O : Insert current line into buffer */
            {
                register uint8 *p = curwp->w_dotp->l_text;
                register int count = curwp->w_dotp->l_used;
                
                while(*p && count--)
                    cins(*p++, buf, &ipos, &ilen, nbuf);
                changed=1 ;
                break ;
            }
        case CK_YANK:    /* ^Y : insert yank buffer */
            {
                register uint8 *pp, cy ;
                KILL *killp;
                
#ifdef _CLIPBRD
                if((clipState & CLIP_TRY_GET) || (kbdmode != PLAY))
                    TTgetClipboard() ;
#endif
                if(klhead == (KLIST*) NULL)
                {
                    TTbell() ;
                    break ;
                }
                killp = klhead->kill;
                
                while(killp != NULL)
                {
                    pp = killp->data ;
                    while((cy=*pp++))
                        if(cins(cy, buf, &ipos, &ilen, nbuf) == FALSE)
                        {
                            TTbell() ;
                            break ;
                        }
                    killp = killp->next;
                }
                changed=1 ;
                break;
            }
            
        case CK_INSFLNM:    /* insert file name */
            {
                register uint8 ch, *p = curbp->b_fname;
                
                if(p != NULL)
                    while(ii--)
                        while((ch=*p++) && cins(ch, buf, &ipos, &ilen, nbuf))
                            ;
                break ;
            }
            
            /*
             * Handle a limited set of Meta sequences. Note that
             * there is no conflict when the end-of-line char is
             * escape, since we will never reach here if it is.
             */
        case CK_BAKWRD:    /* M-B : Move to start of previous word */
            while(ipos && ii--)
            {
                bspace(buf, &ipos, &ilen, LEAVE); /* move over space after word */
                bword(buf, &ipos, &ilen, LEAVE);  /* move over word */
            }
            break;
            
        case CK_CAPWRD:    /* M-C : Capitalise next word and move past it. */
            while(ilen && (ipos != ilen) && ii--)
            {
                fspace(buf, &ipos, &ilen, LEAVE); /* Move over spaces before word */
                /* capitalise if lower-case letter */
                buf[ipos] = toUpper(buf[ipos]) ;
                for(ipos++ ; (ipos<ilen) && isWord(buf[ipos]) ; ipos++)
                    buf[ipos] = toLower(buf[ipos]) ;
                changed=1 ;
            }
            break;
            
        case CK_HIWRD:    /* M-U : Capitalise the whole of the next
                           *      word and move past it.
                           */
            while(ii--)
            {
                /* move over spaces before word */
                fspace(buf, &ipos, &ilen, LEAVE);
                
                /* capitalise if lower-case letter */
                for(; (ipos<ilen) && isWord(buf[ipos]) ; ipos++)
                    buf[ipos] = toUpper(buf[ipos]) ;
                changed=1 ;
            }
            break;
            
        case CK_LOWWRD:   /* M-L : Converts to lower case the whole of the next
                           *      word and move past it.
                           */
            while(ii--)
            {
                /* move over spaces before word */
                fspace(buf, &ipos, &ilen, LEAVE);
                
                /* to lower-case letter */
                for(; (ipos<ilen) && isWord(buf[ipos]) ; ipos++)
                    buf[ipos] = toLower(buf[ipos]) ;
                changed=1 ;
            }
            break;
            
        case CK_DELFWRD: /* Delete word forwards. */
            while(ilen && ii--)
            {
                fspace(buf, &ipos, &ilen, ERASE);    /* eat spaces before word */
                fword(buf, &ipos, &ilen, ERASE);    /* eat word */
                changed=1 ;
            }
            break;
            
        case CK_FORWRD:    /* M-F : Move forward to start of next word. */
            while((ipos != ilen) && ii--)
            {
                /* move over spaces before word */
                fspace(buf, &ipos, &ilen, LEAVE);
                fword(buf, &ipos, &ilen, LEAVE);    /* move over word */
            }
            break;
            
        case CK_SETMRK:
            /*
             * Do nothing - one day this will set the
             * mark at the current cursor position.
             */
            TTbell();
            break;
            
        case CK_DELWBAK:
            /* Delete word backwards. If spaces before
             * the input position then delete those too.
             */
            while(ipos && ii--)
            {
                bspace(buf, &ipos, &ilen, ERASE);    /* eat spaces after word */
                bword(buf, &ipos, &ilen, ERASE);    /* eat word */
                changed=1 ;
            }
            break;
            
        case CK_REYANK:    /* M-Y or M-^Y : Yank the current buffername. */
            {
                register uint8 *p = curbp->b_bname;
                
                while(*p && cins(*p++, buf, &ipos, &ilen, nbuf))
                    ;
                changed=1 ;
                break;
            }
            
        case CK_FISRCH:
        case CK_FORSRCH:
        case CK_BISRCH:
        case CK_BAKSRCH:
            if(option & MLISEARCH)
            {
                mlfirst = cc ;
                mlfreeList(option,noStrs,strList) ;
                return TRUE ;
            }
            TTbell() ;
            break ;
#if MOUSE            
        case CK_CTOMOUSE:
            /* a binding to set-cursor-to-mouse is used to handle mouse
             * events, needs to handle osd pick and normal completion lists */
            /* is it a pick event */
            if((cc == (ME_SPECIAL|SKEY_mouse_pick_1)) ||
               (cc == (ME_SPECIAL|SKEY_mouse_pick_2)) ||
               (cc == (ME_SPECIAL|SKEY_mouse_pick_3)) )
            {
#if MEOSD
                if(mlStatus & MLSTATUS_POSOSD)
                {
                    if(osdMouseContextChange(1))
                        cont_flag = 0;
                }
                else
#endif
                    mlHandleMouse(NULL,0,0) ;
            }
            /* a drop event */
            else if(!(mlStatus & MLSTATUS_POSOSD) && ((cc=mlHandleMouse(buf,nbuf,compOff)) != 0))
            {
                ipos = ilen = meStrlen(buf) ;
                if(cc == 2)
                    cont_flag = 0;
                else
                    changed = 1 ;
            }
            break ;
#endif            
        case CK_VOIDFUNC:
            break ;
            
        case -1:
            if(cc & 0xff00)    /* if control, meta or prefix then scrap */
            {
				TTbell();
                break ;
            }
            
            if (cc == ' ')    /* space */
            {
                if(option & (MLCOMMAND | MLFILE | MLBUFFER | MLVARBL | MLUSER))
                    goto input_expand ;
                if(option & MLNOSPACE)
                {
                    TTbell() ;
                    break ;
                }
            }
input_addexpand:
            /* Else ordinary letter to be stored */
            
            if(option&MLUPPER && cc >= 'a' && cc <= 'z')
                cc -= 'a' - 'A';
            else if(option&MLLOWER && cc >= 'A' && cc <= 'Z')
                cc += 'a' - 'A';
            /*
             * And insert it ....
             */
            if(cins((uint8) cc, buf, &ipos, &ilen, nbuf) == FALSE)
                TTbell();
            changed=1 ;
            break;
            
        default:
            TTbell();
        }
    }

    /*---    Terminate the input. */
    if(ilen == 0)
    {
        if(defaultStr != NULL)
        {
            meStrncpy(buf,defaultStr,nbuf) ;
            buf[nbuf-1] = '\0' ;
        }
    }
    
    /* Store the history if it is not disabled. */
    if((option & (MLNOHIST|MLNOSTORE)) == 0)
    {
        uint8 *ss=mlgsStoreBuf ;
        
        /* If the number of history buffers is at it's maximum 
         * then swap out the last history buffer (mlgsStoreBuf is
         * deleted soon), otherwise the slot will be NULL and
         * nothing will get freed
         */
        mlgsStoreBuf = history[MLHISTSIZE-1] ;
        meStrcpy(ss,buf) ;
        if(numHist < MLHISTSIZE)
        {
            numHist++ ;
            (*numPtr)++ ;
        }
        for(ii=numHist-1 ; ii>0 ; ii--)
            history[ii] = history[ii-1] ;
        history[0] = ss ;
    }
    mlfreeList(option,noStrs,strList) ;

    return TRUE;
}

