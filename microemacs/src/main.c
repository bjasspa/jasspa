/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * main.c - Main entry point.
 *
 * Originally written by Dave G. Conroy for MicroEmacs
 * Copyright (C) 1987 by Daniel M. Lawrence
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
 * Synopsis:    Main entry point.
 * Authors:     Dave G. Conroy, Steve Wilhite, George Jones,
 *              Daniel M. Lawrence,
 *              Jon Green & Steven Phillips (JASSPA)
 */

#define	__MAINC			/* Define program name */

#define	INC_MODE_DEF

#include "emain.h"

#define	maindef		/* Make main defintions - cannot define this at the top
                         * because all the main defs are needed to init edef's vars */

#include "efunc.h"	/* function declarations and name table	*/
#include "eskeys.h"     /* Special key names - Must include before edef.h & ebind.h */
#include "edef.h"	/* The main global variables		*/
#include "ebind.h"	/* default bindings			*/
#include "evar.h"	/* env. vars and user funcs		*/
#include "evers.h"	/* Variables */
#include "esearch.h"	/* defines PTBEG PTEND			*/

#if (defined _UNIX) || (defined _DOS) || (defined _WIN32)
#include <sys/types.h>
#include <time.h>
#ifndef _WIN32
#include <sys/time.h>
#endif
#ifdef _UNIX
#include <fcntl.h>
#include <sys/wait.h>
#endif
#endif

static char meHelpPage[]=
"usage     : me [options] [files]\n\n"
"where options can be:-\n"
"  @<file> : Setup using <file>[.emf], default is me.emf\n"
"  -b      : Load next file as a binary file\n"
"  -c      : Continuation mode (last edit, must have history setup)\n"
"  -d      : Debug mode (for macro files)\n"
"  -h      : For this help page\n"
#ifdef _DOS
"  -i      : Insert the current screen into the *scratch* buffer\n"
#endif
"  -k[key] : Load next file as a crypted file optionally giving the <key>\n"
"  +<n> or\n"
"  -l <n>  : Go to line <n> in the next given file\n"
#ifdef _CLIENTSERVER
"  -m <msg>: Post message <msg> to MicroEmacs server\n"
#endif
#ifdef _WINCON
"  -n      : For not MS window, uses console instead\n"
#endif
#ifdef _XTERM
"  -n      : For not X window, uses termcap instead\n"
#endif
#ifdef _CLIENTSERVER
"  -o      : One MicroEmacs, use ME server if available\n"
#endif
"  -p      : Pipe stdin into *stdin*, when saved output to stdout\n"
"  -r      : Read-only, all buffers will be in view mode\n"
"  -s <s>  : Search for string <s> in the next given file\n"
"  -u <n>  : Set user name to <n> (sets $MENAME)\n"
"  -v <v=s>: Set variable <v> to string <s>\n"
#ifdef _UNIX
"  -x      : Don't catch signals\n"
#endif
"  -y      : Load next file as a reduced binary file\n"
"\n"
;

#ifdef _WIN32
#define mePrintHelpMessage(mm) MessageBox(NULL,(char *) mm,"MicroEmacs '" meVERSION,MB_OK);
#else
#define mePrintHelpMessage(mm) write(2,mm,meStrlen(mm))
#endif

/*
 * Initialize all of the buffers and windows. The buffer name is passed down
 * as an argument, because the main routine may have been told to read in a
 * file by default, and we want the buffer name to be right.
 */
static void
edinit(uint8 *bname)
{
    register BUFFER *bp;
    register WINDOW *wp;
    register LINE   *lp, *off;

    if(((bp = bfind(bname,BFND_CREAT)) == NULL) ||
       ((wp = (WINDOW *) meMalloc(sizeof(WINDOW))) == NULL) ||
       ((lp =lalloc(TTmcol)) == NULL) ||
       ((off=lalloc(TTmcol)) == NULL))
        meExit(1);
    curbp  = bp;                        /* Make this current    */
    wheadp = wp;
    curwp  = wp;
    numWindows   = 1;                   /* Number of windows */
    memset(wp,0,sizeof(WINDOW)) ;
    wp->numCols  = TTncol;              /* Window # columns     */
    wp->numRows  = TTnrow;              /* Window # rows        */
    wp->model    = lp ;
    off->l_fp    = NULL ;
    wp->curLineOff= off ;
    wp->w_bufp   = bp;
    wp->w_dotp   = bp->b_linep;
    wp->w_flag   = WFMODE|WFRESIZE|WFSBAR;  /* Full + resize        */
    bp->b_nwnd   = 1;                   /* Displayed.           */
    fixWindowTextSize (wp);             /* Fix the text window  */
    vvideoAttach (&vvideo, wp);         /* Attach to video      */
}

/*
 * insertChar
 *
 * Insert n * 'c' chars into the current buffer
 * Copes with insert and undo modes correctly
 */
int
insertChar(register int c, register int n)
{
    /*---    If we are  in overwrite mode,  not at eol,  and next char is
       not a tab or we are at a tab stop, delete a char forword */

    if(meModeTest(curbp->b_mode,MDOVER))
    {
        int index, ii=n ;
        for(index=0,n=0 ; (ii>0) && (curwp->w_doto < curwp->w_dotp->l_used) ; ii--)
        {
            if((lgetc(curwp->w_dotp, curwp->w_doto) != TAB) ||
               (at_tab_pos(getccol()+index+1) == 0))
            {
                lchange(WFMAIN);
#if MEUNDO
                meUndoAddRepChar() ;
#endif
                lputc(curwp->w_dotp,curwp->w_doto++,c) ;
                index = 0 ;
            }
            else
            {
                index++ ;
                n++ ;
            }
        }
        n += ii ;
    }
    /*---    Do the appropriate insertion */
    if(n > 0)
    {
#if MEUNDO
        if(n == 1)
        {
            meUndoAddInsChar() ;
            if(linsert(1,c) != TRUE)
                return FALSE ;
        }
        else
        {
            if(linsert(n, c) != TRUE)
                return FALSE ;
            meUndoAddInsChars(n) ;
        }
#else
        if(linsert(n, c) != TRUE)
            return FALSE ;
#endif
    }
    return TRUE ;
}

/*
 * This is the general command execution routine. It handles the fake binding
 * of all the keys to "self-insert". It also clears out the "thisflag" word,
 * and arranges to move it to the "lastflag", so that the next command can
 * look at it. Return the status of command.
 */
int
execute(register int c, register int f, register int n)
{
    register int index;
    uint32 arg ;
    int ii ;

    {
        /* If any of the modifiers have changed since the last key we executed
         * then execute any commands bound to the M-pick or M-drop pseudo keys,
         * e.g. the S-pick key will be generated between the following 2 keys:
         *      right S-right. 
         * These keys only get 'pressed' if a command is bound to them. The
         * commands are executed in a hidden way, so they are not recorded in
         * kbd macros or in the $recent-keys. This could led to a macro failing
         * but alternatives have similar if not worse problems.
         */
        static uint16 lastcc=0 ;
        uint32 arg ;
        uint16 mask, kk, ccx=lastcc ^ c ;
        
        lastcc = c ;
        for(mask=ME_SHIFT ; mask != ME_SPECIAL ; mask<<=1)
        {
            if(ccx & mask)
            {
                if(c & mask)
                    kk = ME_SPECIAL|SKEY_pick|mask ;
                else
                    kk = ME_SPECIAL|SKEY_drop|mask ;
                if((index=decode_key(kk,&arg)) != -1)
                    execFuncHidden(kk,index,arg) ;
            }
        }
    }
    
    if (kbdmode != PLAY)
    {
#ifdef _CLIPBRD
        clipState |= CLIP_TRY_GET|CLIP_TRY_SET ;
#endif
#if MEUNDO
        undoContFlag++ ;
#endif
    }
    lastCommand = thisCommand ;
    lastIndex = thisIndex ;
    thisCommand = c ;

    if(((index = decode_key((uint16) c,&arg)) >= 0) && (arg != 0))
    {
        f = 1 ;
        n *= (int) (arg + 0x80000000) ;
    }
    thisIndex = index ;
    meRegCurr->f = f ;
    meRegCurr->n = n ;
    /* SWP - 27/2/99 - to enable input commands to not have to jump through
     * so many hoops I've changed to input support so the macros can fail
     * in such a way as to indicate that they have not handled this input,
     * this allows them to pick and choose.
     * The method chosen was to check the command variable status if it aborted, if
     * defined to "0" then its not handled the input
     */
    if((ii=curbp->inputFunc) >= 0)
    {
        uint8 *ss ;
        if(((cmdstatus = (execFunc(ii,f,n) == TRUE))) ||
           ((ss=getUsrLclCmdVar((uint8 *)"status",&(cmdTable[ii]->varList))) == errorm) || meAtoi(ss))
            return cmdstatus ;
    }
    if(index >= 0)
        return (cmdstatus = (execFunc(index,f,n) == TRUE)) ;
    if(selhilight.flags)
    {
        selhilight.bp = NULL;
        selhilight.flags = 0;
    }
    if(c < 0x20 || c > 0xff)    /* If not an insertable char */
    {
        uint8 outseq[40];	/* output buffer for keystroke sequence */
        meGetStringFromKey((uint16) c,outseq);	/* change to something printable */
        lastflag = 0;                       /* Fake last flags.     */
        cmdstatus = 0 ;
        /* don't complain about mouse_move* or mouse_time* as these are
         * special keys that are only added if they are bound and due
         * to their frequence they can be in the input buffer before they're
         * unbound leading to this error */
        c &= ~(ME_SHIFT|ME_CONTROL|ME_ALT) ;
        if(((c >= (ME_SPECIAL|SKEY_mouse_move)) && (c <= (ME_SPECIAL|SKEY_mouse_move_3))) ||
           ((c >= (ME_SPECIAL|SKEY_mouse_time)) && (c <= (ME_SPECIAL|SKEY_mouse_time_3))) )
            return TRUE ;
        return mlwrite(MWABORT,(uint8 *)"[Key not bound \"%s\"]", outseq); /* complain */
    }

    if (n <= 0)            /* Fenceposts.          */
    {
        lastflag = 0;
        return (cmdstatus = ((n<0) ? FALSE : TRUE));
    }
    thisflag = 0;          /* For the future.      */

    if(bchange() != TRUE)               /* Check we can change the buffer */
        return (cmdstatus = FALSE) ;

    /* If a space was  typed, fill column is  defined, the argument is non-
     * negative, wrap mode is enabled, and we are now past fill column, and
     * we are not read-only, perform word wrap. */

    if(meModeTest(curbp->b_mode,MDWRAP) &&
       ((c == ' ') || (meStrchr(filleos,c) != NULL)) &&
       (fillcol > 0) && (n >= 0) && (getccol() > fillcol) &&
       !meModeTest(curbp->b_mode,MDVIEW))
        wrapWord(FALSE, 1);

    /* insert the required number of chars */
    if(insertChar(c,n) != TRUE)
        return (cmdstatus = FALSE) ;

#if    CFENCE
    if(meModeTest(curbp->b_mode,MDCMOD))
    {
        if((c == '}') || (c == '#'))
        {
            ii = curwp->w_doto ;
            curwp->w_doto = 0 ;
            if(gotoFrstNonWhite() == c)
            {
                curwp->w_doto = ii ;
                doCindent(&ii) ;
            }
            else
                curwp->w_doto = ii ;
        }
        else if((commentMargin >= 0) &&
                ((c == '*') || (c == '/')) && (curwp->w_doto > 2) &&
                (lgetc(curwp->w_dotp, curwp->w_doto-2) == '/') &&
                (lgetc(curwp->w_dotp, curwp->w_doto-3) != '/'))
        {
            ii = curwp->w_doto - 2 ;
            if((gotoFrstNonWhite() == 0) &&
               (curwp->w_doto=0,(gotoFrstNonWhite() != 0)) &&
               (curwp->w_doto < ii))
            {
                curwp->w_doto = ii ;
                if(ii > commentMargin)
                    ii = (tabsize-1) - ((ii-1)%tabsize) ;
                else
                    ii = commentMargin - ii ;
                linsert(ii,' ') ;
#if MEUNDO
                meUndoAddInsChars(ii) ;
#endif
            }
            else
                curwp->w_doto = ii ;
            curwp->w_doto += 2 ;
        }
    }
    /* check for fence matching */
    if(meModeTest(curbp->b_mode,MDFENCE) && ((c == '}') || (c == ')') || (c == ']')))
    {
        curwp->w_doto-- ;
        /* flag for delay move and only bell in cmode */
        gotoFence(TRUE,(meModeTest(curbp->b_mode,MDCMOD)) ? 3:2) ;
        curwp->w_doto++ ;
    }
#endif
    lastflag = thisflag;
    return (cmdstatus = TRUE) ;
}


/*
** Me info
** Creates a *info* buffer and populates it with info on the current state of things.
*/

static void
addModesList(BUFFER *bp, register uint8 *buf, uint8 *name, meMODE mode,
             int res)
{
    register int ii, nlen, len, ll ;

    meStrcpy(buf,name) ;
    len = nlen = meStrlen(name) ;
    for (ii = 0; ii < MDNUMMODES; ii++)	/* add in the mode flags */
        if((meModeTest(mode,ii) != 0) == res)
        {
            ll = meStrlen(modeName[ii]);
            if(len+ll >= 78)
            {
                buf[len] = '\0' ;
                addLineToEob(bp,buf) ;
                len = nlen ;
                memset(buf,' ',len) ;
            }
            buf[len++] = ' ' ;
            meStrcpy(buf+len, modeName[ii]);
            len += ll ;
        }
    buf[len] = '\0' ;
    addLineToEob(bp,buf) ;
}

static void
addModesLists(BUFFER *bp, register uint8 *buf, meMODE mode)
{
    addLineToEob(bp,(uint8 *)"") ;
    addModesList(bp,buf,(uint8 *)"  Modes on  :",mode,1) ;
    addModesList(bp,buf,(uint8 *)"  Modes off :",mode,0) ;
    addLineToEob(bp,(uint8 *)"") ;
}

uint8 meCopyright[]="Copyright (C) 1988-" meCENTURY meYEAR " JASSPA (www.jasspa.com)" ;
int
meAbout(int f, int n)
{
    WINDOW *wp ;
    BUFFER *bp, *tbp ;
    int32   numchars ;		/* # of chars in file */
    int32   numlines ;		/* # of lines in file */
    int32   predchars ;		/* # chars preceding point */
    int32   predlines ;		/* # lines preceding point */
    uint8   buf[MAXBUF] ;
    int     ii ;

    if((wp = wpopup(BaboutN,(BFND_CREAT|BFND_CLEAR|WPOP_USESTR))) == NULL)
        return FALSE ;
    bp = wp->w_bufp ;

     /* definitions in evers.h */
    addLineToEob(bp,(uint8 *)ENAME " " meVERSION " - Date " meDATE " - " meSYSTEM_NAME) ;
    addLineToEob(bp,(uint8 *)"") ;
    addLineToEob(bp,(uint8 *)"Global Status:") ;
    tbp = bheadp ;
    ii = 0 ;
    while(tbp != NULL)
    {
        ii++ ;
        tbp = tbp->b_bufp ;
    }
    sprintf((char *)buf,"  # buffers : %d", ii) ;
    addLineToEob(bp,buf) ;
    addModesLists(bp,buf,globMode) ;
    addLineToEob(bp,(uint8 *)"Current Buffer Status:") ;
    sprintf((char *)buf,"  Buffer    : %s", curbp->b_bname) ;
    addLineToEob(bp,(uint8 *)buf) ;
    sprintf((char *)buf,"  File name : %s",
            (curbp->b_fname == NULL) ? (uint8 *)"":curbp->b_fname) ;
    addLineToEob(bp,buf) ;
    addLineToEob(bp,(uint8 *)"") ;

    getBufferInfo(&numlines,&predlines,&numchars,&predchars) ;
    sprintf((char *)buf,"  Lines     : Total %6ld, Current %6ld",numlines,predlines) ;
    addLineToEob(bp,buf) ;
    sprintf((char *)buf,"  Characters: Total %6ld, Current %6ld",numchars,predchars) ;
    addLineToEob(bp,buf) ;

    addModesLists(bp,buf,curbp->b_mode) ;
    addLineToEob(bp,meCopyright) ;
    bp->b_dotp = lforw(bp->b_linep);
    bp->b_doto = 0 ;
    bp->line_no = 0 ;
    meModeClear(bp->b_mode,MDEDIT);     /* don't flag this as a change */
    meModeSet(bp->b_mode,MDVIEW);       /* put this buffer view mode */
    resetBufferWindows(bp) ;
    return TRUE ;
}

/*
 * Quit command. If an argument, always quit. Otherwise confirm if a buffer
 * has been changed and not written out. Normally bound to "C-X C-C".
 */
int
exitEmacs(int f, int n)
{
    int s=TRUE ;

    /* Argument forces it.  */
    if(f == FALSE)
    {
        char buff[128] ;
        
        if(anyChangedBuffer())
            strcpy(buff,"Modified buffer") ;
        else
            buff[0] = '\0' ;
#if SPELL
        if(anyChangedDictionary())
        {
            strcat(buff,(buff[0] == '\0') ? "Modified":",") ;
            strcat(buff," dictionary") ;
        }
#endif
#if REGSTRY
        if(anyChangedRegistry())
        {
            strcat(buff,(buff[0] == '\0') ? "Modified":",") ;
            strcat(buff," registry") ;
        }
#endif
#ifdef _IPIPES
        if(anyActiveIpipe())
        {
            strcat(buff,(buff[0] == '\0') ? "A":" and a") ;
            strcat(buff,"ctive process") ;
        }
#endif
        if(buff[0] != '\0')
        {
            /* somethings changed - check the user is happy */
            strcat(buff," exists, leave anyway") ;
            s = mlyesno((uint8 *)buff) ;
        }
    }

    if(s == TRUE)
    {
        BUFFER *bp, *nbp ;
        int func ;

        /* User says it's OK.   */
#ifdef _CLIENTSERVER
        /* the client-server is a psuedo ipipe but it needs
         * to be stopped properly, therefore stop it first as
         * the following ipipeRemove only does half a job and
         * TTkillClientServer will be very confused if called after
         */
        TTkillClientServer() ;
#endif
        /* go round removing all the active ipipes,
         * execute any dhooks set on active buffers
         * and remove all auto-write files
         */
#ifdef _IPIPES
        while(ipipes != NULL)
            ipipeRemove(ipipes) ;
#endif
        bp = bheadp ;
        while(bp != NULL)
        {
            nbp = bp->b_bufp ;
            autowriteremove(bp) ;
            if(!meModeTest(bp->b_mode,MDNACT) && (bp->dhook >= 0))
                execBufferFunc(bp,bp->dhook,0,1) ;     /* Execute the delete hook */
            bp = nbp ;
        }
        saveHistory(TRUE,0) ;
        /* call the shut-down command if its defined */
        func = decode_fncname((uint8 *)"shut-down",1) ;
        if(func >= 0)
            execFunc(func,FALSE,1) ;
#ifdef _URLSUPP
        ffFileOp(NULL,NULL,meRWFLAG_FTPCLOSE|meRWFLAG_NOCONSOLE) ;
#endif
        TTend();
#ifdef _TCAP
        if(!(alarmState & meALARM_PIPED)
#ifdef _XTERM
           && (meSystemCfg & meSYSTEM_CONSOLE)
#endif
           )
            TCAPputc('\n');
#endif
#ifdef _ME_FREE_ALL_MEMORY
        {
            /* free of everything we can */
            extern void freeToken(HILNODEPTR root) ;
            extern void printFreeMemory(void) ;
            extern void osdFreeMemory(void) ;
            extern void regFreeMemory(void) ;
            extern void srchFreeMemory(void) ;
            extern void TTfreeTranslateKey(void) ;
            extern uint8 *ffbuf ;
            extern LINE  *mline ;
            extern uint8 *defHistFile ;
            extern uint32 *colTable ;
            FRAMELINE    *flp;                   /* Frame store line pointer */
            WINDOW       *wp ;
            meMACRO      *mac ;
            meVARIABLE   *nuv, *cuv ;
            KLIST        *thiskl ;
            KILL         *next, *kill ;
            meABBREV     *abrev ;
            VVIDEO       *vvptr;
            int           ii, jj ;

            while(bheadp->b_bufp != NULL)
            {
                bheadp->b_bufp->intFlag |= BIFBLOW ;
                zotbuf(bheadp->b_bufp,1) ;
            }
            /* ensure the buffer we're left with is a simple *scratch* */
            bheadp->intFlag |= BIFBLOW ;
            zotbuf(bheadp,1) ;
            bclear(bheadp) ;
            meFree(bheadp->b_linep);             /* Release header line. */
            meNullFree(bheadp->b_fname) ;
            meNullFree(bheadp->b_bname) ;
            meNullFree(bheadp->modeLineStr) ;
            meFree(bheadp);                             /* Release buffer block */
            meNullFree(mlBinds) ;

            while (wheadp != NULL)
            {
                wp = wheadp ;
                wheadp = wp->w_wndp ;
                meFree(wp->model) ;
                meFree(wp->curLineOff) ;
                meFree(wp);
            }
            /* Destruct all virtual video blocks */
            meFree (vvideo.video);
            for (vvptr = vvideo.next; vvptr != NULL; vvptr = vvideo.next)
            {
                vvideo.next = vvptr->next;
                meFree (vvptr->video);
                meFree (vvptr);
            }
            meFree(hilBlock) ;
            meFree(disLineBuff) ;
            meFree(mline) ;
            meFree(mlStore) ;
            meNullFree(defHistFile) ;
#ifdef _XTERM
            meNullFree(colTable) ;
#endif
            meNullFree(searchPath) ;
            meNullFree(homedir) ;
            meNullFree(curdir) ;

            for(ii=0 ; ii<numStrHist ; ii++)
                free(strHist[ii]) ;
            for(ii=0 ; ii<numBuffHist ; ii++)
                free(buffHist[ii]) ;
            for(ii=0 ; ii<numCommHist ; ii++)
                free(commHist[ii]) ;
            for(ii=0 ; ii<numFileHist ; ii++)
                free(fileHist[ii]) ;
            for(ii=0 ; ii<numSrchHist ; ii++)
                free(srchHist[ii]) ;
            free(strHist) ;

            while(klhead != NULL)
            {
                thiskl = klhead ;
                klhead = klhead->next ;
                kill = thiskl->kill ;
                while(kill != NULL)
                {
                    next = kill->next;
                    free(kill);
                    kill = next;
                }
                free(thiskl) ;
            }
            for(ii=0 ; ii<CK_MAX ; ii++)
            {
                cuv = cmdTable[ii]->varList.head ;
                while(cuv != NULL)
                {
                    nuv = cuv->next ;
                    meNullFree(cuv->value) ;
                    free(cuv) ;
                    cuv = nuv ;
                }
            }
            for( ; ii<cmdTableSize ; ii++)
            {
                mac = getMacro(ii) ;
                meFree(mac->name) ;
                meNullFree(mac->fname) ;
                freeLineLoop(mac->hlp,1) ;
                cuv = mac->varList.head ;
                while(cuv != NULL)
                {
                    nuv = cuv->next ;
                    meNullFree(cuv->value) ;
                    free(cuv) ;
                    cuv = nuv ;
                }
                free(mac) ;
            }
            if(cmdTable != __cmdTable)
                free(cmdTable) ;
            cuv = usrVarList.head ;
            while(cuv != NULL)
            {
                nuv = cuv->next ;
                meNullFree(cuv->value) ;
                free(cuv) ;
                cuv = nuv ;
            }
            meFree(meRegHead) ;
            meFree(ffbuf) ;
            meFree(progName) ;
            if(modeLineStr != orgModeLineStr)
                meNullFree(modeLineStr) ;
            meNullFree(flNextFileTemp) ;
            meNullFree(flNextLineTemp) ;
            if(commentCont != commentContOrg)
                meFree(commentCont) ;

            meNullFree(rcsFile) ;
            meNullFree(rcsCiStr) ;
            meNullFree(rcsCiFStr) ;
            meNullFree(rcsCoStr) ;
            meNullFree(rcsCoUStr) ;
            meNullFree(rcsUeStr) ;

            {
                extern meNAMESVAR buffNames ;
                extern meDIRLIST  fileNames ;
                extern meNAMESVAR commNames ;
                extern meNAMESVAR modeNames ;
                extern meNAMESVAR varbNames ;

                meNullFree(curDirList.path) ;
                freeFileList(curDirList.size,curDirList.list) ;
                meNullFree(fileNames.mask) ;
                meNullFree(fileNames.path) ;
                freeFileList(fileNames.size,fileNames.list) ;
                meNullFree(modeNames.mask) ;
                meNullFree(commNames.list) ;
                meNullFree(commNames.mask) ;
                meNullFree(buffNames.list) ;
                meNullFree(buffNames.mask) ;
                if(varbNames.list != NULL)
                    freeFileList(varbNames.size,varbNames.list) ;
                meNullFree(varbNames.mask) ;
            }

            if(noNextLine > 0)
            {
                for(ii=0 ; ii<noNextLine ; ii++)
                {
                    meFree(nextName[ii]) ;
                    for(jj=0 ; jj<nextLineCnt[ii] ; jj++)
                        meFree(nextLineStr[ii][jj]) ;
                    meNullFree(nextLineStr[ii]) ;
                }
                meFree(nextLineCnt) ;
                meFree(nextLineStr) ;
                meFree(nextName) ;
            }
            addFileHook(TRUE,0) ;

            deleteDict(1,6) ;
            addSpellRule(1,0) ;
            printFreeMemory() ;
            osdFreeMemory() ;
            regFreeMemory() ;
            srchFreeMemory() ;
            TTfreeTranslateKey() ;
#if HILIGHT
            if(noHilights > 0)
            {
                uint8 hilno ;

                for(hilno=0 ; hilno < noHilights ; hilno++)
                {
                    if(hilights[hilno] != NULL)
                    {
                        hilights[hilno]->close = NULL ;
                        hilights[hilno]->rtoken = NULL ;
                        freeToken(hilights[hilno]) ;
                    }
                }
                meFree(hilights) ;
            }
#endif
            meNullFree (fileIgnore) ;

            while(aheadp != NULL)
            {
                abrev = aheadp ;
                aheadp = abrev->next ;
                freeLineLoop(&(abrev->hlp),0) ;
                meFree(abrev) ;
            }
            meFree(styleTable) ;
            for(flp=frameStore,ii=0; ii<TTmrow; ii++, flp++)
                meFree(flp->scheme) ;
            meFree(frameStore) ;
        }
#endif
        meExit(0);
    }
    mlerase(MWCLEXEC);

    return s ;
}


/*
 * Better quit command, query saves buffers and then queries the exit
 * if modified buffers still exist
 */
int
saveExitEmacs(int f, int n)
{
    if((saveSomeBuffers(f,(n & 0x01)) == TRUE)
#if SPELL
       && (saveDict(f,2|(n & 0x01)) != FALSE)
#endif
#if REGSTRY
       && (saveRegistry(f,2|(n & 0x01)) != FALSE)
#endif
       )
        return exitEmacs(f, n) ;            /* conditionally quit   */
    return FALSE ;
}

/*
 * Fancy quit command, as implemented by Norm. If the any buffer has
 * changed do a write on that buffer and exit emacs, otherwise simply exit.
 */
int
quickExit(int f, int n)
{
    return saveExitEmacs(1,0) ;
}

/*
 * Abort.
 * Beep the beeper. Kill off any keyboard macro, etc., that is in progress.
 * Sometimes called as a routine, to do general aborting of stuff.
 */
int
ctrlg(int f, int n)
{
    /* stop recording - execution will stop at this point so there's no
     * point continuing, this provides an easy get out if the user forgets
     * the macro end record key */
    if(kbdmode == RECORD)
    {
        kbdmode = STOP ;
        addModeToWindows(WFMODE) ;  /* update ALL mode lines */
    }
    if(n)
        mlwrite(MWABORT,(uint8 *)"[Aborted]");
    return ABORT ;
}

/*
** function to report a commands not available (i.e. compiled out!) */
int
notAvailable(int f, int n)
{
    return mlwrite(MWABORT,(uint8 *)"[Command not available]");
}

/*
** function to report a no mark set error */
int
noMarkSet(void)
{
    return mlwrite(MWABORT,(uint8 *)"No mark set in this window");
}

/* tell the user that this command is illegal while we are in
   VIEW (read-only) mode                */
int
rdonly(void)
{
    return mlwrite(MWABORT,(uint8 *)"[Command illegal in VIEW mode]");
}

/* void function, does nothing but return false if an argument of zero past,
 * else return true. Used to get rid of unwanted unbounded-key bells
 */
int
voidFunc(int f, int n)
{
    return ((n) ? TRUE:FALSE) ;
}

void
prefixFunc(void) /* dummy prefix function */
{
}
void
uniArgument(void)         /* dummy function for binding to universal-argument */
{
}

#ifdef _UNIX
/* sigchild; Handle a SIGCHILD action. Note for the older types of signal it
 * is not necessary to reset the signal handler because they "reappear" as
 * soon as the signal handler is reset - hence we do it at the bottom */
static void
sigchild(SIGNAL_PROTOTYPE)
{
    meWAIT_STATUS status ;              /* Status return from waitpid() */

#ifdef _IPIPES
    if(noIpipes != 0)
    {
        meIPIPE *ipipe ;

        ipipe = ipipes ;
        while(ipipe != NULL)
        {
            if((ipipe->pid > 0) &&
               (meWaitpid(ipipe->pid,&status,WNOHANG) == ipipe->pid))
            {
                if(WIFEXITED(status))
                    ipipe->pid = -4 ;
                else if(WIFSIGNALED(status))
                    ipipe->pid = -3 ;
#ifdef WCOREDUMP
                else if(WCOREDUMP(status))
                    ipipe->pid = -2 ;
#endif
#if 0
                else
                {
                    if(WIFSTOPPED(status))
                        mlwrite(0,"Process %s has STOPPED",ipipe->buf->b_bname) ;
#ifdef _IRIX
                    else if(WIFCONTINUED(status))
                        mlwrite(0,"Process %s has CONTINUED",ipipe->buf->b_bname) ;
#endif
                    else
                        mlwrite(0,"Process %s sent an UNKNOWN SIGNAL",ipipe->buf->b_bname) ;
                }
#endif
            }
            ipipe = ipipe->next ;
        }
    }
#endif
    /* clear up any zoombie children */
    meWaitpid(-1,&status,WNOHANG) ;
    
    /* Reload the signal handler. Note that the child is a special case where
     * the signal is reset at the end of the handler as opposed to the head of
     * the handler. For the POSIX or BSD signals this is not required. */
#if !((defined _POSIX_SIGNALS) || (defined _BSD_SIGNALS))
    signal (SIGCHLD, sigchild);
#endif
}
#endif

#if (defined _UNIX) || (defined _WIN32)
int
meDie(void)
{
    register BUFFER *bp;    /* scanning pointer to buffers */
#ifdef _XTERM
    int      usedX = 0 ;
#endif
    /*
     * To get here we have received a signal and am about to die!
     * Warn the user.
     */
    /* we know about the signal and we're dealing with it */
    alarmState = 0 ;
#ifdef _XTERM
    /* As the X window will always go, always output to the console
     * or terminal, so if using X, do some grotty stuff.
     * This is really important if the X-term window is destroyed,
     * see waitForEvent in unixterm.c. This is because the windows
     * already gone, so we can't use it!
     */
    if(!(meSystemCfg & meSYSTEM_CONSOLE))
    {
        usedX = 1 ;
        meSystemCfg |= meSYSTEM_CONSOLE ;
    }
#endif
    /* Make a noisy BELL */
    meModeClear(globMode,MDQUIET) ;
    mlwrite(MWCURSOR|MWABORT,(uint8 *)"*** Emergency quit ***");
#ifdef _TCAP
    TCAPputc('\n');
#endif

    bp = bheadp;
    while (bp != NULL)
    {
        if(bufferNeedSaving(bp))
        {
            autowriteout(bp) ;
#ifdef _TCAP
            TCAPputc('\n');
#endif
        }
        bp = bp->b_bufp;            /* on to the next buffer */
    }
    saveHistory(1,0) ;
#if REGSTRY
    saveRegistry(1,2) ;
#endif
#ifdef _IPIPES
    /* try to kill any child processes */
    while(ipipes != NULL)
        ipipeRemove(ipipes) ;
#endif
#ifdef _TCAP
    TCAPputc('\n');
#endif
#ifdef _XTERM
    /* Don't bother with the TTend if this was an xterm, to dangerous */
    if(!usedX)
#endif
        TTend();

    meExit(1) ;
    return 0;
}
#endif

void
autoSaveHandler(void)
{
    struct meTimeval tp ;
    register BUFFER *bp ;
    register int32 tim, next=0x7fffffff ;

    gettimeofday(&tp,NULL) ;
    tim = ((tp.tv_sec-startTime)*1000) + (tp.tv_usec/1000) ;
    bp  = bheadp;
    while (bp != NULL)
    {
        if(bp->autotime >= 0)
        {
            if(bp->autotime <= tim)
                autowriteout(bp) ;
            else if(bp->autotime < next)
                next = bp->autotime ;
        }
        bp = bp->b_bufp ;
    }
    if(next != 0x7fffffff)
        timerSet(AUTOS_TIMER_ID,next,next-tim) ;
    else
        timerClearExpired(AUTOS_TIMER_ID) ;
}

void
callBackHandler(void)
{
    struct meTimeval tp ;
    register meMACRO *mac ;
    register int32 tim, next=0x7fffffff ;
    register int ii ;

    gettimeofday(&tp,NULL) ;
    tim = ((tp.tv_sec-startTime)*1000) + (tp.tv_usec/1000) ;

    /* Loop through all the macros executing them first */
    for(ii=CK_MAX ; ii<cmdTableSize ; ii++)
    {
        mac = getMacro(ii) ;
        if((mac->callback >= 0) && (mac->callback < tim))
        {
             mac->callback = -1 ;
            /* If the current buffer has an input handler, that macro will
             * receive the key "callback", this must be bound to this macro,
             * otherwise the input handler will not know what macro to call
             */
            if(curbp->inputFunc >= 0)
            {
                /* binary chop through the key table looking for the character code.
                ** If found then return the index into the names table.
                */
                register KEYTAB *ktp;			/* Keyboard character array */
                register int     low;			/* Lowest index in table. */
                register int     hi;			/* Hightest index in table. */
                register int     mid;			/* Mid value. */
                register int     status;		/* Status of comparison. */
                ktp = keytab ;
                hi  = keyTableSize;			/* Set hi water to end of table */
                low = 0;				/* Set low water to start of table */
                do
                {
                    mid = (low + hi) >> 1;		/* Get mid value. */
                    if ((status=(ME_SPECIAL|SKEY_callback)-ktp[mid].code) == 0)
                    {
                        /* Found - return index */
                        ktp[mid].index = ii ;
                        break ;
                    }
                    else if (status < 0)
                        hi = mid - 1;		/* Discard bottom half */
                    else
                        low = mid + 1;		/* Discard top half */
                } while (low <= hi);		/* Until converges */
            }
            execFuncHidden(ME_SPECIAL|SKEY_callback,ii,0) ;
        }
    }
    /* Loop through all the macros to set the next */
    for(ii=CK_MAX ; ii<cmdTableSize ; ii++)
    {
        mac = getMacro(ii) ;
        if((mac->callback >= 0) && (mac->callback < next))
            next = mac->callback ;
    }
    if(next != 0x7fffffff)
        timerSet(CALLB_TIMER_ID,next,next-tim) ;
    else
        timerClearExpired(CALLB_TIMER_ID) ;
}

#ifdef _UNIX
static void
sigeat(SIGNAL_PROTOTYPE)
{
    /* Reload the signal handler. For the POSIX or BSD signals this is not
     * required. */
#if !((defined _POSIX_SIGNALS) || (defined _BSD_SIGNALS))
    signal (sig, sigeat);
#endif
    /*
     * Trap nasty signals under UNIX. Write files and quit.
     * Can't just do that cos god knows what me is doing (broken links etc).
     * So can only flag request here and hope its seen.
     */
    alarmState |= meALARM_DIE ;
}
#endif    /* _UNIX */

void
doOneKey(void)
{
    register int    c;
    register int    f;
    register int    n;
    register int    mflag;
    int     basec;              /* c stripped of meta character   */

    update(FALSE);                          /* Fix up the screen    */

    /*
     * If we are not playing or recording a macro. This is the ONLY place
     * where we may get an idle key back from the input stream. Set the kdb
     * mode to IDLE to indicate that we will accept an idle key.
     *
     * kdbmode is probably not the best variable to use. However it is global
     * and it is not currntly being used - hence it is available for abuse !!
     */
    if (kbdmode == STOP)
        kbdmode = KBD_IDLE;             /* In an idle state  */
    
    c = meGetKeyFromUser(FALSE, 1, meGETKEY_COMMAND);     /* Get a key */

    if (mlStatus & MLSTATUS_CLEAR)
        mlerase(MWCLEXEC) ;

    f = FALSE;
    n = 1;

    /* do ME_PREFIX1-# processing if needed */
    basec = c & ~ME_PREFIX1 ;        /* strip meta char off if there */
    if((c & ME_PREFIX1) && ((basec >= '0' && basec <= '9') || basec == '-'))
    {
        f = TRUE;
        if(basec == '-')
        {
            mflag = -1 ;
            n = 0;
        }
        else
        {
            mflag = 1 ;
            n = basec - '0' ;
        }
        while(((c=meGetKeyFromUser(TRUE,(n * mflag),meGETKEY_COMMAND)) >= '0') && (c <= '9'))
            n = n * 10 + (c - '0');
        n *= mflag;    /* figure in the sign */
    }

    /* do ^U repeat argument processing */
    if(c == reptc)
    {                           /* ^U, start argument   */
        f = TRUE;               /* In case not set */
        mflag = 1;              /* current minus flag */
        for(;;c = meGetKeyFromUser(f,n,meGETKEY_COMMAND))
        {
            switch(c)
            {
            case 21 :                   /* \U (^U) */
                n *= 4;
                continue;    /* Loop for more */
            case '-':
                /* If '-', and argument is +ve, make arg -ve */
                mflag = -1;
                if(n < 0)
                    break;
                n *= -1;
                continue;    /* Loop for more */
            }
            /* Not ^U or first '-' .. drop out */
            break;
        }
        /* handle "^U n" to give "n" (GNU Emacs does this) */
        if ((c >= '0' && c <= '9'))
        {
            for(n=c-'0' ;;)
            {
                /*
                 * get the next key, if a digit, update the
                 * count note that we do not handle "-" here
                 */
                c = meGetKeyFromUser(TRUE,(mflag*n),meGETKEY_COMMAND);
                if(c >= '0' && c <= '9')
                    n = n * 10 + (c - '0');
                else
                    break;
            }
            n *= mflag;    /* figure in the sign */
        }
    }

    if(f == TRUE)        /* Zap count from cmd line ? */
        mlerase(MWCLEXEC);
    
    execute(c, f, n) ;   /* Do it. */
}


void
mesetup(int argc, char *argv[])
{
    extern uint8 *ffbuf ;
    BUFFER *bp, *mainbp ;
    int     carg,rarg;          /* current arg to scan            */
    int     noFiles=0 ;
    uint8  *file=NULL ;
#ifdef _UNIX
    int     sigcatch=1 ;        /* Dont catch signals             */
#endif
#ifdef _DOS
    int     dumpScreen=0 ;
#endif
#ifdef _CLIENTSERVER
    uint8  *clientMessage=NULL ;
    int     userClientServer=0 ;
#endif
    startTime = time(NULL) ;

#ifdef _UNIX
    /* Get the usr id and group id for mode-line and file permissions */
    meXUmask = umask(0);
    umask(meXUmask);
    meXUmask = (meXUmask & (S_IROTH|S_IWOTH|S_IXOTH|S_IRGRP|S_IWGRP|S_IXGRP|S_IRUSR|S_IWUSR|S_IXUSR)) ^ 
              (S_IROTH|S_IWOTH|S_IXOTH|S_IRGRP|S_IWGRP|S_IXGRP|S_IRUSR|S_IWUSR|S_IXUSR) ; /* 00666 */
    meUmask = meXUmask & (S_IROTH|S_IWOTH|S_IRGRP|S_IWGRP|S_IRUSR|S_IWUSR) ;
    meUid = getuid();
    meGid = getgid();
    /* get a list of groups the user belongs to */
    meGidSize = getgroups(0,NULL) ;
    if((meGidSize > 1) &&
       ((meGidList = malloc(meGidSize*sizeof(gid_t))) != NULL))
        meGidSize = getgroups(meGidSize,meGidList) ;
    else
        meGidSize = 0 ;

    /* Set the required alarms first so we always have them */
    /* setup alarm process for timers */
#ifdef _POSIX_SIGNALS
    {
        struct sigaction sa ;

        sigemptyset(&meSigHoldMask) ;
        sigaddset(&meSigHoldMask,SIGCHLD);
        sigaddset(&meSigHoldMask,SIGALRM);
        meSigRelease() ;

        sigemptyset(&sa.sa_mask) ;
        sa.sa_flags=0;
        sa.sa_handler=sigAlarm ;
        sigaction(SIGALRM,&sa,NULL) ;
        /* sigset(SIGALRM,sigAlarm);*/
        /* setup child process exit capture for ipipe */
        sa.sa_handler=sigchild ;
        sigaction(SIGCHLD,&sa,NULL) ;
        /* sigset(SIGCHLD,sigchild);*/
    }
#else /* _POSIX_SIGNALS */
#ifdef _BSD_SIGNALS
    /* Set up the hold mask */
    meSigHoldMask = sigmask (SIGCHLD) | sigmask (SIGALRM);
#endif /* _BSD_SIGNALS */
    signal (SIGCHLD, sigchild);
    signal (SIGALRM, sigAlarm);
#endif /* _POSIX_SIGNALS */
#endif /* _UNIX */
    count_key_table() ;

    if(((meRegHead = meMalloc(sizeof(meREGISTERS))) == NULL) ||
       ((ffbuf = meMalloc(FIOBUFSIZ+1)) == NULL))
        exit(1) ;
    /* Make the head registers point back to themselves so that
     * accessing #p? gets #g? and not a core-dump
     */
    meRegHead->prev = meRegHead ;
    meRegHead->commandName = NULL ;
    meRegHead->execstr = NULL ;
    meRegHead->varList = NULL ;
    meRegHead->force = 0 ;
    meRegCurr = meRegHead ;
    /* Initialise the head as this is dumped in list-variables */
    for(carg = 0 ; carg<meNUMREG ; carg++)
        meRegHead->reg[carg][0] = '\0' ;

    /* initialize the editor and process the command line arguments */
    initHistory() ;           /* allocate history space */
    set_dirs();               /* setup directories */

    {
        /* setup the $progname make it an absolute path. */
        if(executableLookup((uint8 *)argv[0],evalResult))
            progName = meStrdup(evalResult) ;
        else
#ifdef _ME_FREE_ALL_MEMORY
            /* stops problems on exit */
            progName = meStrdup(argv[0]) ;
#else
            progName = (uint8 *)argv[0] ;
#endif
    }
    /* scan through the command line and get all global options */
    carg = rarg = 1 ;
    for( ; carg < argc; ++carg)
    {
        /* if its a switch, process it */
        if (argv[carg][0] == '-')
        {
            switch (argv[carg][1])
            {
            case 'b':    /* -b bin flag */
            case 'k':    /* -k crypt flag */
            case 'y':    /* -y rbin flag */
                /* don't want these options yet as they apply to the
                 * next file on the command line
                 */
                argv[rarg++] = argv[carg] ;
                break ;

            case 'c':
                HistNoFilesLoaded = -1 ;
                break ;

            case 'd':    /* -d debug */
                macbug = TRUE;
                break;

            case 'h':
                mePrintHelpMessage(meHelpPage) ;
                meExit(0) ;
#ifdef _DOS
            case 'i':
                dumpScreen = 1 ;
                break ;
#endif
            case 'l':    /* -l for initial goto line */
                /* don't want these options yet as they apply to the
                 * next file on the command line
                 */
                argv[rarg++] = argv[carg] ;
                if (argv[carg][2] == 0)
                {
                    if (carg == argc-1)
                    {
missing_arg:
                        sprintf((char *)evalResult,"%s Error: Argument expected with option %s\nOption -h gives further help\n",
                                argv[0],argv[carg]);
                        mePrintHelpMessage(evalResult) ;
                        meExit(1);
                    }
                    argv[rarg++] = argv[++carg] ;
                }
                break ;

#ifdef _CLIENTSERVER
            case 'm':    /* -m send a client server message */
                userClientServer = 1 ;
                if(argv[carg][2] == '\0')
                {
                    if (carg == argc-1)
                        goto missing_arg ;
                    clientMessage = (uint8 *)argv[++carg] ;
                }
                else
                    clientMessage = (uint8 *)argv[carg]+2 ;
                break;
#endif
#if (defined _XTERM) || (defined _WINCON)
            case 'n':
                meSystemCfg |= meSYSTEM_CONSOLE ;
                break ;
#endif
#ifdef _CLIENTSERVER
            case 'o':
                userClientServer=1 ;
                break ;
#endif
            case 'p':
                alarmState |= meALARM_PIPED ;
#if (defined _XTERM) || (defined _WINCON)
                meSystemCfg |= meSYSTEM_CONSOLE ;
#endif
                break ;

            case 'r':
                meModeSet(globMode,MDVIEW) ;
                break ;

            case 's':    /* -s for initial search string */
                argv[rarg++] = argv[carg] ;
                if(argv[carg][2] == '\0')
                {
                    if (carg == argc-1)
                        goto missing_arg ;
                    argv[rarg++] = argv[++carg] ;
                }
                break;

            case 'u':    /* -u for setting the user name */
                {
                    char *un ;
                    if (argv[carg][2] == 0)
                    {
                        if (carg == argc-1)
                            goto missing_arg ;
                        un = argv[++carg] ;
                    }
                    else
                        un = argv[carg] + 2 ;
                    meStrcpy(evalResult,"MENAME=") ;
                    meStrcpy(evalResult+7,un) ;
                    mePutenv(meStrdup(evalResult)) ;
                    break;
                }

            case 'v':
                {
                    char *ss, *tt, cc ;
                    if (argv[carg][2] == 0)
                    {
                        if (carg == argc-1)
                            goto missing_arg ;
                        ss = argv[++carg] ;
                    }
                    else
                        ss = argv[carg] + 2 ;
                    if((tt = strchr(ss,'=')) == NULL)
                    {
                        sprintf((char *)evalResult,"%s Error: Mal-formed -v option\n",argv[0]) ;
                        mePrintHelpMessage(evalResult) ;
                        meExit(1) ;
                    }
                    
                    if(((cc=getMacroTypeS(ss)) != TKREG) && (cc != TKVAR) &&
                       (cc != TKCVR) && (cc != TKENV))
                    {
                        *tt = '\0' ;
                        sprintf((char *)evalResult,"%s Error: Cannot set variable [%s] from the command-line\n",argv[0],ss) ;
                        mePrintHelpMessage(evalResult) ;
                        meExit(1) ;
                    }
                    if((tt = strchr(ss,'=')) != NULL)
                    {
                        *tt++ = '\0' ;
                        if(setVar((uint8 *)ss,(uint8 *)tt,meRegCurr) != TRUE)  /* set a variable */
                        {
                            sprintf((char *)evalResult,"%s Error: Unable to set variable [%s]\n",argv[0],ss) ;
                            mePrintHelpMessage(evalResult) ;
                            meExit(1) ;
                        }
                        execstr = NULL ;
                        clexec = FALSE ;
                    }
                    break ;
                }
#ifdef _UNIX
            case 'x':
                sigcatch = 0 ;
                break;
#endif

            default:
                {
                    sprintf((char *)evalResult,"%s Error: Unknown option %s\nOption -h gives further help\n",argv[0],argv[carg]) ;
                    mePrintHelpMessage(evalResult) ;
                    meExit(1) ;
                }
            }
        }
        else if(argv[carg][0] == '@')
            file = (uint8 *) argv[carg] + 1 ;
        else if((argv[carg][0] == '+') && (argv[carg][1] == '\0'))
            goto missing_arg ;
        else
            argv[rarg++] = argv[carg] ;
    }
#ifdef _CLIENTSERVER
    if(userClientServer && TTconnectClientServer())
    {
        int     binflag=0 ;         /* load next file as a binary file*/
        int     gline = 0 ;         /* what line? (-g option)         */

        for(carg=1 ; carg < rarg ; carg++)
        {
            /* if its a switch, process it */
            if(argv[carg][0] == '-')
            {
                switch(argv[carg][1])
                {
                case 'l':    /* -l for initial goto line */
                    if (argv[carg][2] == 0)
                        gline = meAtoi(argv[++carg]);
                    else
                        gline = meAtoi((argv[carg])+2);
                    break;
                case 'b':    /* -b bin flag */
                    binflag |= BFND_BINARY ;
                    break;
                case 'k':    /* -k crypt flag */
                    binflag |= BFND_CRYPT ;
                    break;
                case 's':
                    /* -s not supported across client-server */
                    if (argv[carg][2] == 0)
                        carg++ ;
                    break ;
                case 'y':    /* -y rbin flag */
                    binflag |= BFND_RBIN ;
                    break;
                }
            }
            else if (argv[carg][0] == '+')
                gline = meAtoi((argv[carg])+1);
            else
            {
                /* set up a buffer for this file */
                noFiles += findFileList((uint8 *)argv[carg],(BFND_CREAT|BFND_MKNAM|binflag),gline) ;
                gline = 0 ;
                binflag = 0 ;
            }
        }
        if(clientMessage != NULL)
            TTsendClientServer(clientMessage) ;
        bp = bheadp ;
        while(bp != NULL)
        {
            if(bp->b_fname != NULL)
            {
                uint8 buff[MAXBUF+32] ;
                int nn=1 ;
                if(meModeTest(bp->b_mode,MDBINRY))
                    nn |= BFND_BINARY ;
                if(meModeTest(bp->b_mode,MDCRYPT))
                    nn |= BFND_CRYPT ;
                if(meModeTest(bp->b_mode,MDRBIN))
                    nn |= BFND_RBIN ;
                sprintf((char *)buff,"C:ME:%d find-file \"%s\"\n",nn,bp->b_fname) ;
                TTsendClientServer(buff) ;
                if(bp->line_no != 0)
                {
                    sprintf((char *)buff,"C:ME:goto-line %d\n",(int) bp->line_no) ;
                    TTsendClientServer(buff) ;
                }
            }
            bp = bp->b_bufp ;
        }
        meExit(0) ;
    }
    else if(userClientServer && clientMessage)
    {
        sprintf((char *)evalResult,"%s Error: Unable to connect to server\n",argv[0]) ;
        mePrintHelpMessage(evalResult) ;
        meExit(1) ;
    }
#endif

    vtinit();                 /* Displays.            */
    edinit(BmainN);           /* Buffers, windows.    */

#ifdef _DOS
    if(dumpScreen)
    {
        extern void TTdump(BUFFER *) ;
        TTdump(curbp) ;
        gotobob(FALSE,1) ;
        carg++ ;
    }
#endif
    if(alarmState & meALARM_PIPED)
    {
#ifdef _WIN32
        ffrp = GetStdHandle(STD_INPUT_HANDLE) ;
#else
        ffrp = stdin ;
#endif
        bp = bfind(BstdinN,BFND_CREAT);
        readin(bp,NULL) ;
    }

    mlerase(0);                /* Delete command line */
    /* run me.emf unless an @... arg was given in which case run that */
    startup(file) ;

    /* initalize *scratch* colors and modes to global defaults */
    if((mainbp=bfind(BmainN,0)) != NULL)
    {
        meModeCopy(mainbp->b_mode,globMode) ;
#if COLOR
        mainbp->scheme = globScheme;
#endif
        /* make *scratch*'s history number very low so any other
         * buffer is preferable */
        mainbp->histNo = -1 ;
    }
#if COLOR
#ifdef _CLIENTSERVER
    /* also initialize the client server color scheme */
    if((ipipes != NULL) && (ipipes->pid == 0))
        ipipes->bp->scheme = globScheme;
#endif
#endif
    
    {
        uint8  *searchStr=NULL, *cryptStr=NULL ;
        int     binflag=0 ;         /* load next file as a binary file*/
        int     gline = 0 ;         /* what line? (-g option)         */
        int     obufHistNo ;
        
        obufHistNo = bufHistNo ;

        /* scan through the command line and get the files to edit */
        for(carg=1 ; carg < rarg ; carg++)
        {
            /* if its a switch, process it */
            if(argv[carg][0] == '-')
            {
                switch(argv[carg][1])
                {
                case 'l':    /* -l for initial goto line */
                    if (argv[carg][2] == 0)
                        gline = meAtoi(argv[++carg]);
                    else
                        gline = meAtoi((argv[carg])+2);
                    break;
                case 'b':    /* -b bin flag */
                    binflag |= BFND_BINARY ;
                    break;
                case 'k':    /* -k crypt flag */
                    binflag |= BFND_CRYPT ;
                    if (argv[carg][2] != 0)
                        cryptStr = (uint8 *) argv[carg] + 2 ;
                    break;
                case 's':
                    /* -s not supported across client-server */
                    if (argv[carg][2] == 0)
                        searchStr = (uint8 *) argv[++carg] ;
                    else
                        searchStr = (uint8 *) argv[carg]+2 ;
                    break ;
                case 'y':    /* -y rbin flag */
                    binflag |= BFND_RBIN ;
                    break;
                }
            }
            else if (argv[carg][0] == '+')
                gline = meAtoi((argv[carg])+1);
            else
            {
                /* set up a buffer for this file - force the history so the first file
                 * on the command-line has the highest bufHistNo so is shown first */
                bufHistNo = obufHistNo + rarg - carg ;
                noFiles += findFileList((uint8 *)argv[carg],(BFND_CREAT|BFND_MKNAM|binflag),gline) ;
                if((cryptStr != NULL) || (searchStr != NULL))
                {
                    /* Deal with -k<key> and -s <search> */
                    bp = bheadp ;
                    while(bp != NULL)
                    {
                        if(bp->histNo == bufHistNo)
                        {
#if CRYPT
                            if(cryptStr != NULL)
                                setBufferCryptKey(bp,cryptStr) ;
#endif
                            if(searchStr != NULL)
                            {
                                BUFFER *cbp ;
                                int histNo, flags ;
                                cbp = curbp ;
                                histNo = cbp->histNo ;
                                swbuffer(curwp,bp) ;
                                flags = ISCANNER_QUIET ;
                                if(meModeTest(bp->b_mode,MDMAGIC))
                                    flags |= ISCANNER_MAGIC ;
                                if(meModeTest(bp->b_mode,MDEXACT))
                                    flags |= ISCANNER_EXACT ;
                                /* what can we do if we fail to find it? */
                                iscanner(searchStr,0,flags,NULL) ;
                                swbuffer(curwp,cbp) ;
                                cbp->histNo = histNo ;
                                bp->histNo = bufHistNo ;
                            }
                        }
                        bp = bp->b_bufp ;
                    }
                    cryptStr = NULL ;
                    searchStr = NULL ;
                }
                gline = 0 ;
                binflag = 0 ;
            }
        }
        bufHistNo = obufHistNo + rarg ;
    }

    /* load-up the com-line or -c first and second files */
    if(!noFiles)
        noFiles = HistNoFilesLoaded ;
    else
        HistNoFilesLoaded = 0 ;
    
    if(noFiles > 0)
    {
        if((curbp == mainbp) && ((bp = replacebuffer(NULL)) != mainbp) &&
           (bp->b_fname != NULL))
        {
            if(HistNoFilesLoaded && isUrlLink(bp->b_fname))
            {
                uint8 prompt[FILEBUF+16] ;
                meStrcpy(prompt,"Reload file ") ;
                meStrcat(prompt,bp->b_fname) ;
                if(mlyesno(prompt) != TRUE)
                {
                    bp = NULL ;
                    noFiles = 0 ;
                }
            }
	}
        else
            bp = NULL ;
        if(bp != NULL)
        {
            swbuffer(curwp,bp) ;
	    mainbp->histNo = -1 ;
        }
        if((noFiles > 1) && ((bp = replacebuffer(NULL)) != mainbp) &&
           (bp->b_fname != NULL))
        {
            if(!HistNoFilesLoaded && !isUrlLink(bp->b_fname))
            {
                splitWindVert(TRUE,2) ;
                swbuffer(curwp,replacebuffer(NULL)) ;
                prevwind(FALSE,1) ;
            }
        }
    }
#ifdef _UNIX
    /*
     * Catch nasty signals when running under UNIX unless the -x
     * option appeared on the command line.
     *
     * These can't be trapped as caused by ME failure and will cause loop
     *
     * signal(SIGILL, sigeat);
     * signal(SIGTRAP, sigeat);
     * signal(SIGBUS, sigeat);
     * signal(SIGSEGV, sigeat);
     * signal(SIGSYS, sigeat);
     * signal(SIGIOT, sigeat);
     * signal(SIGEMT, sigeat);
     */
    if(sigcatch)
    {
#ifdef _POSIX_SIGNALS
        struct sigaction sa ;

        sigemptyset(&sa.sa_mask) ;
        sa.sa_flags=0;
        sa.sa_handler=sigeat ;
        sigaction(SIGHUP,&sa,NULL) ;
        sigaction(SIGINT,&sa,NULL) ;
        sigaction(SIGQUIT,&sa,NULL) ;
        sigaction(SIGTERM,&sa,NULL) ;
        sigaction(SIGUSR1,&sa,NULL) ;
#else /* _POSIX_SIGNALS */
        signal(SIGHUP,sigeat) ;
        signal(SIGINT,sigeat) ;
        signal(SIGQUIT,sigeat) ;
        signal(SIGTERM,sigeat) ;
        signal(SIGUSR1,sigeat) ;
#endif /* _POSIX_SIGNALS */
    }
#endif /* _UNIX */

    /* setup to process commands */
    lastflag = 0;                       /* Fake last flags.     */

    /* Set the screen for a redraw at this point cos its after the
     * startup which can screw things up
     */
    sgarbf = TRUE;			 /* Erase-page clears */

    carg = decode_fncname((uint8 *)"start-up",1) ;
    if(carg >= 0)
        execFunc(carg,FALSE,1) ;
}

#ifndef NDEBUG

/* _meAssert
 * The receiving end of the macro meAssert(<boolean>). This keeps microEMACS
 * up allowing the debugger to be attached. This is essential under Windows
 * since Microsofts assert() is the pits and invariably just exits the
 * program without invoking the debugger.
 */

void
_meAssert (char *file, int line)
{
    uint8 buf[MAXBUF];                  /* String buffer */
    uint8 cc;                           /* Character input */

    /* Put out the string */
    sprintf ((char *) buf,
             "!! ASSERT FAILED %s:%d !! - <S>ave. <Q>uit. <C>ontinue",
             file, line);
    mlwrite (MWABORT, buf);

    TTinflush ();                       /* Flush the input buffer */
    for (;;)
    {
        cc = (uint8) TTgetc() ;         /* Get character from keyboard */
        /* Try and perform an emergency save for the user - no guarantees
         * here I am afraid - we may be totally screwed at this point in
         * time. */
        if (cc == 'S')
        {
            register BUFFER *bp;        /* scanning pointer to buffers */

            bp = bheadp;
            while (bp != NULL)
            {
                if(bufferNeedSaving(bp))
                    autowriteout(bp) ;
                bp = bp->b_bufp;        /* on to the next buffer */
            }
            saveHistory(TRUE,0) ;
        }
        else if (cc == 'Q')
            meExit (1);                 /* Die the death */
        else if (cc == 'C')
            break;                      /* Let the sucker go !! */
    }
}
#endif

int
commandWait(int f, int n)
{
    meVARLIST *varList=NULL ;
    uint8 clexecSv ;
    int execlevelSv ;
    uint8 *ss ;
    
    if(f)
    {
        f = clexec ;
        clexec = FALSE ;
        if(n < 0)
            TTsleep(0-n,1) ;
        else
            TTsleep(n,0) ;
        clexec = f ;
        return TRUE ;
    }
    if((meRegCurr->commandName != NULL) &&
       ((f=decode_fncname(meRegCurr->commandName,1)) >= 0))
        varList = &(cmdTable[f]->varList) ;
    
    clexecSv = clexec;
    execlevelSv = execlevel ;
    clexec = FALSE ;
    execlevel = 0 ;
    do
    {
        doOneKey() ;
        if(TTbreakFlag)
        {
            TTinflush() ;
            if((selhilight.flags & (SELHIL_ACTIVE|SELHIL_KEEP)) == SELHIL_ACTIVE)
                selhilight.flags &= ~SELHIL_ACTIVE ;
            TTbreakFlag = 0 ;
        }
    } while((varList != NULL) && 
            ((ss=getUsrLclCmdVar((uint8 *)"wait",varList)) != errorm) && meAtoi(ss)) ;
    clexec = clexecSv ;
    execlevel = execlevelSv ;
    return TRUE ;
}

#ifndef _WIN32
void
main(int argc, char *argv[])
{
    mesetup(argc,argv) ;
    while(1)
    {
        doOneKey() ;
        if(TTbreakFlag)
        {
            TTinflush() ;
            if((selhilight.flags & (SELHIL_ACTIVE|SELHIL_KEEP)) == SELHIL_ACTIVE)
                selhilight.flags &= ~SELHIL_ACTIVE ;
            TTbreakFlag = 0 ;
        }
    }
}
#endif
