/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * eval.c - Expresion evaluation functions.
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
 * Created:     1986
 * Synopsis:    Expresion evaluation functions.
 * Authors:     Daniel Lawrence, Jon Green & Steven Phillips
 */ 

#define __EVALC 1       /* Define program name */

#include "emain.h"
#include "evar.h"
#include "efunc.h"
#include "eskeys.h"
#include "esearch.h"
#include "evers.h"
#include "eopt.h"

#if (defined _UNIX) || (defined _DOS) || (defined _WIN32)
#include <sys/types.h>
#ifdef _UNIX
#include <sys/stat.h>
#endif
#endif

meUByte evalResult[meTOKENBUF_SIZE_MAX];    /* result string */
static meUByte machineName[]=meSYSTEM_NAME; /* platform & build strings */
static meUByte buildStr[]=" " meSYSTEM_NAME " " DEF_TO_STR(_PLATFORM_VER) " " DEF_TO_STR(_ARCHITEC) " " 
#ifdef _64BIT
"64"
#else
"32"
#endif
" " DEF_TO_STR(_TOOLKIT) " " DEF_TO_STR(_TOOLKIT_VER) " ";

#if MEOPT_TIMSTMP
extern meUByte time_stamp[];   /* Time stamp string */
#endif

#if MEOPT_EXTENDED
#include <math.h>
#define meFtoa(dd) (sprintf((char *) evalResult,"%.16e",(dd)),evalResult)
#define meAtof(ss) (atof((const char *) ss))
static time_t timeOffset=0 ;            /* Time offset in seconds */

#ifdef _INSENSE_CASE
meNamesList buffNames={0,0,NULL,NULL,0} ;
meDirList  fileNames={0,0,NULL,NULL,0,NULL} ;
#else
meNamesList buffNames={1,0,NULL,NULL,0} ;
meDirList  fileNames={1,0,NULL,NULL,0,NULL,0} ;
#endif
meNamesList commNames={1,0,NULL,NULL,0} ;
meNamesList modeNames={0,MDNUMMODES,modeName,NULL,0} ;
meNamesList varbNames={1,0,NULL,NULL,0} ;

#endif

/* The following horrid global variable solves a horrid problem, consider:
   define-macro l3
   set-variable @1 @2
   !emacro
   define-macro l2
   l3 @1 @2
   !emacro
   define-macro l1
   l2 @1 @2
   !emacro
   define-macro Test
   l1 #l0 @#
   ml-write #l0
   !emacro
   500 Test
 * The above example should write out "500".
 * its not good enough for @1 in l3 to return #l0, it must return Test's #l0
 * and thats horrid, hence the variable. Similarly it should be set to Test's
 * @#, not l3's which will be 1.
 * The hopefully final saga to this is that the process of setting a variable
 * may not be instantaneous, e.g. when doing "set-variable #l1 @ml "get value""
 * it waits for the user to enter a value. During this time a callback macro
 * may occur which could (probably will) change the value of gmaLocalRegPtr
 * so the value must be cached as soon as the variable to be set is obtained.
 */
meRegister *gmaLocalRegPtr=NULL ;

#if MEOPT_MAGIC
meRegex meRegexStrCmp ;
/* Quick and easy interface to regex compare
 * 
 * return -1 on error, 0 on no match 1 if matched
 */
int
regexStrCmp(meUByte *str, meUByte *rgx, int flags)
{
    meRegex *rg ;
    int ii, rr ;
    
    if(flags & meRSTRCMP_USEMAIN)
    {
        rg = &mereRegex ;
        srchLastState = meFALSE ;
    }
    else
        rg = &meRegexStrCmp ;
    
    if(meRegexComp(rg,rgx,(flags & meRSTRCMP_ICASE)) != meREGEX_OKAY)
        return -1 ;
    
    ii = meStrlen(str) ;
    rr = meRegexMatch(rg,str,ii,0,ii,(flags & meRSTRCMP_WHOLE)) ;
    if((flags & meRSTRCMP_USEMAIN) && (rr > 0))
    {
        if(ii >= mereNewlBufSz)
        {
            if((mereNewlBuf = meRealloc(mereNewlBuf,ii+128)) == NULL)
            {
                mereNewlBufSz = 0 ;
                return meFALSE ;
            }
            mereNewlBufSz = ii+127 ;
        }
        meStrcpy(mereNewlBuf,str) ;
        srchLastMatch = mereNewlBuf ;
        srchLastState = meTRUE ;
        /* or in the STRCMP bit so that hunt-forw/backward can still determine
         * whether the last main search was magic or not & @s1 also work correctly */ 
        srchLastMagic |= meSEARCH_LAST_MAGIC_STRCMP ;
    }
    return rr ;
}
#endif

/* meItoa
 * integer to ascii string..........
 * This is too inconsistant to use the system's */
#define INTWIDTH (sizeof(int)*3+2)
meUByte *
meItoa(int i)
{
    register meUByte *sp;      /* pointer into result */
    register int sign;      /* sign of resulting number */
    
    if(i == 0)          /* eliminate the trivial 0  */
        return meLtoa(0);
    else if(i == 1)     /* and trivial 1            */
        return meLtoa(1);
    
    if((sign = (i < 0)))    /* record the sign...*/
        i = -i;
    
    /* and build the string (backwards!) */
    sp = &(evalResult[INTWIDTH]);
    *sp = 0;
    do
        *(--sp) = '0' + (i % 10);   /* Install the new digit */
    while (i /= 10);            /* Remove digit and test for 0 */
    
    if (sign != 0)          /* and fix the sign */
        *(--sp) = '-';          /* and install the minus sign */
    
    return sp;
}


meVariable *
SetUsrLclCmdVar(meUByte *vname, meUByte *vvalue, meVariable **varList)
{
    /* varList is a pointer to the head var address, type cast to a variable - works as first member is next */
    register meVariable *vp, *vptr = (meVariable *) varList;
    register int ii;
#if MEOPT_CMDHASH
    register meUInt hash;
    
    meStringHash(vname,hash);
    /* scan the list looking for the hash & then the var name */
    while(((vp = vptr->next) != NULL) && (vp->hash <= hash))
    {
        if(vp->hash == hash)
        {
            if((ii = meStrcmp(vname,vp->name)) == 0)
            {
                /* found it, replace value */
                meStrrep(&vp->value,vvalue) ;
                return vp ;
            }
            if(ii < 0)
                break;
        }
        vptr = vp;
    }
#else
    while((vp = vptr->next) != NULL)
    {
        if((ii = meStrcmp(vname,vp->name)) == 0)
        {
            /* found it, replace value */
            meStrrep(&vp->value,vvalue) ;
            return vp ;
        }
        if(ii < 0)
            break;
        vptr = vp;
    }
#endif
    
    /* Not found so create a new one */
    if((vp = (meVariable *) meMalloc(sizeof(meVariable)+meStrlen(vname))) != NULL)
    {
        meStrcpy(vp->name,vname) ;
#if MEOPT_CMDHASH
        vp->hash = hash;
#endif
        vp->next = vptr->next;
        vp->value = meStrdup(vvalue);
        vptr->next = vp;
    }
    return vp ;
}

int
setVar(meUByte *vname, meUByte *vvalue, meRegister *regs)
{
    register int status;         /* status return */
    meUByte *nn;
    
    /* check the legality and find the var */
    nn = vname+1;
    switch(getMacroTypeS(vname))
    {
    case TKREG:
        {
            meUByte cc ;
            if((cc=*nn) != 'l')
            {
                if(cc == 'g')
                    regs = meRegHead ;
                else if(cc == 'p')
                    regs = regs->prev ;
                else
                    return mlwrite(MWABORT,(meUByte *)"[No such register %s]",vname);
            }
            cc = nn[1] - '0' ;
            if(cc >= meREGISTER_MAX)
                return mlwrite(MWABORT,(meUByte *)"[No such register %s]",vname);
            meStrcpy(regs->val[cc],vvalue) ;
            break ;
        }
#if MEOPT_EXTENDED
    case TKVAR:
        if(SetUsrLclCmdVar(nn,vvalue,&usrVarList) == NULL)
            return meFALSE ;
        break ;
    case TKLVR:
        {
            meBuffer *bp ;
            meUByte *ss ;
            if((ss=meStrrchr(nn,':')) != NULL)
            {
                *ss = '\0' ;
                bp = bfind(nn,0) ;
                *ss++ = ':' ;
                if(bp == NULL)
                    /* not a buffer - abort */
                    return mlwrite(MWABORT,(meUByte *)"[No such variable]");
                nn = ss ;
            }
            else
                bp = frameCur->windowCur->buffer;
            if(SetUsrLclCmdVar(nn,vvalue,&(bp->varList)) == NULL)
                return meFALSE ;
            break ;
        }
    case TKCVR:
        {
            meVariable **varList ;
            meUByte *ss ;
            if((ss=meStrrchr(nn,'.')) != NULL)
            {
                int ii ;
                
                *ss = '\0' ;
                ii = decode_fncname(nn,1) ;
                *ss++ = '.' ;
                if(ii < 0)
                    /* not a command - abort */
                    return mlwrite(MWABORT,(meUByte *)"[No such variable]");
                varList = &(cmdTable[ii]->varList) ;
                nn = ss ;
            }
            else if((varList = regs->varList) == NULL)
                return mlwrite(MWABORT,(meUByte *)"[No such variable]");
            if(SetUsrLclCmdVar(nn,vvalue,varList) == NULL)
                return meFALSE ;
            break ;
        }
        
    case TKARG:
        if(*nn == 'w')
        {
            meWindow *cwp;
            if((status=bufferSetEdit()) <= 0)
                return status;
            cwp = frameCur->windowCur;
            if(nn[1] == 'l')
            {
                int ll=0 ;
                
                if((cwp->dotLineNo == cwp->buffer->lineCount) ||
                   (meStrchr(vvalue,meCHAR_NL) != NULL))
                    return ctrlg(0,1) ;
                
                cwp->dotOffset = 0 ;
                if((status = ldelete(meLineGetLength(cwp->dotLine),6)) > 0)
                {
                    ll = meStrlen(vvalue) ;
                    status = lineInsertString(ll,vvalue) ;
#if MEOPT_UNDO
                    if(status >= 0)
                        meUndoAddReplaceEnd(ll) ;
#endif
                }
                return status ;
            }
            else
            {
                if (cwp->dotOffset >= cwp->dotLine->length)
                    return mlwrite(MWABORT,(meUByte *)"[Cannot set @wc here]");
                
                lineSetChanged(WFMAIN);
#if MEOPT_UNDO
                meUndoAddRepChar() ;
#endif
                meLineSetChar(cwp->dotLine,cwp->dotOffset,vvalue[0]) ;
            }
        }
        else if(*nn == 'c')
        {
            int ii ;
            if(nn[2] == 'k')
            {
                meUByte *saves, savcle ;
                
                /* use the macro string to key evaluator to get the value,
                 * must set this up carefully and restore state */
                savcle = clexec;        /* save execution mode */
                clexec = meTRUE;      /* get the argument */
                saves = execstr ;
                execstr = vvalue ;
                ii = meGetKey(meGETKEY_SILENT) ;
                execstr = saves ;
                clexec = savcle;        /* restore execution mode */
                if(ii != 0)
                {
                    if(nn[1] == 'c')
                        thisCommand = ii ;
                    else
                        lastCommand = ii ;
                }
            }
            else
            {
                ii = decode_fncname(vvalue,1) ;
                if(nn[1] == 'l')
                {
                    switch(ii)
                    {
                    case CK_BAKLIN:
                    case CK_FORLIN:
                        thisflag = meCFCPCN;
                        break ;
                    case CK_DELWBAK:
                    case CK_CPYREG:
                    case CK_DELFWRD:
                    case CK_KILEOL:
#if MEOPT_WORDPRO
                    case CK_KILPAR:
#endif
                    case CK_KILREG:
                        thisflag = meCFKILL;
                        break ;
                    case CK_REYANK:
                        thisflag = meCFRYANK ;
                        break ;
                    case CK_YANK:
                        thisflag = meCFYANK ;
                        break ;
#if MEOPT_UNDO
                    case CK_UNDO:
                        thisflag = meCFUNDO ;
                        break ;
#endif
                    }
                    lastIndex = ii ;
                }
                else
                    thisIndex = ii ;
            }
        }
        else if(*nn == '?')
            meRegCurr->f = meAtoi(vvalue) ;
        else if(*nn == '#')
            meRegCurr->n = meAtoi(vvalue) ;
        else if(*nn == 'y')
        {
            int len = meStrlen(vvalue);
            killInit(0);
            if((nn = killAddNode(len)) == NULL)
                return meABORT ;
            memcpy(nn,vvalue,len) ;
            nn[len] = '\0' ;
        }
        else if(*nn == 'h')
        {
            int option;
            if((nn[1] > '0') && (nn[1] <= '4'))
                option = 1 << (nn[1] - '1');
            else
                option = 0 ;
            addHistory(option,vvalue,(nn[2] == '-'));
        }
        else
            return mlwrite(MWABORT,(meUByte *)"[Cannot set variable %s]",vname);
        break ;
#endif
        
    case TKENV:
        /* set an environment variable */
        biChopFindString(nn,envars,NEVARS,status);
        switch(status)
        {
        case EVAUTOTIME:
            autoTime = meAtoi(vvalue);
            break;
#if MEOPT_CALLBACK
        case EVIDLETIME:
            idleTime = (meUInt) meAtoi(vvalue);
            if (idleTime < 10)
                idleTime = 10;
            break;
#endif
#if MEOPT_MOUSE
        case EVDELAYTIME:
            delayTime = (meUInt) meAtoi(vvalue);
            if (delayTime < 10)
                delayTime = 10;
            break;
        case EVREPEATTIME:
            repeatTime = (meUInt) meAtoi(vvalue);
            if (repeatTime < 10)
                repeatTime = 10;
            break;
#endif
#if MEOPT_EXTENDED
            /* always allow the user to set the mouse position as termcap
             * does not support the mouse but context menus need to be positioned sensibly */
        case EVMOUSE:
#if MEOPT_MOUSE
            TTinitMouse(meAtoi(vvalue));
#endif
            break;
        case EVMOUSEPOS:
            mouse_pos = (meUByte) meAtoi(vvalue);
            break;
        case EVMOUSEX:
            mouse_X = (meShort) meAtoi(vvalue);
            break;
        case EVMOUSEY:
            mouse_Y = (meShort) meAtoi(vvalue);
            break;
        case EVPAUSETIME:
            pauseTime = (meShort) meAtoi(vvalue);
            break;
        case EVFSPROMPT:
            fileSizePrompt = (meUInt) meAtoi(vvalue);
            break;
        case EVKEPTVERS:
            if(!(meSystemCfg & meSYSTEM_DOSFNAMES) && 
               ((keptVersions = meAtoi(vvalue)) < 0))
                keptVersions = 0;
            break;
#endif
        case EVBOXCHRS:
            {
                meUByte cc;
                int len;
                len = meStrlen(vvalue);
                if (len > BCLEN)
                    len = BCLEN;
                while(--len >= 0)
                {
                    cc = vvalue[len];
                    if(!isPokable(cc))
                        cc = '.';
                    boxChars[len] = cc;
                }
            }
            break;
        case EVMODELINE:
            if(modeLineStr != orgModeLineStr)
                meFree(modeLineStr);
            modeLineStr = meStrdup(vvalue);
            modeLineFlags = assessModeLine(vvalue);
            frameAddModeToWindows(WFMODE);
            break;
#if MEOPT_EXTENDED
        case EVBMDLINE:
            meStrrep(&frameCur->windowCur->buffer->modeLineStr,vvalue);
            frameCur->windowCur->buffer->modeLineFlags = assessModeLine(vvalue);
            frameAddModeToWindows(WFMODE);
            break;
#endif
        case EVSYSTEM:
            {
                meSystemCfg = (meSystemCfg & ~meSYSTEM_MASK) | 
                      (meAtoi(vvalue)&meSYSTEM_MASK) ;
#if MEOPT_EXTENDED
                if(meSystemCfg & meSYSTEM_DOSFNAMES)
                    keptVersions = 0 ;
#endif
                if(meSystemCfg & meSYSTEM_SHOWWHITE)
                {
                    displayTab = windowChars[WCDISPTAB];
                    displayNewLine = windowChars[WCDISPCR];
                    displaySpace = windowChars[WCDISPSPACE];
                }
                else
                {
                    displayTab = ' ';
                    displayNewLine = ' ';
                    displaySpace = ' ';    
                }
#ifdef _UNIX
#ifdef _ME_WINDOW
#ifdef _ME_CONSOLE
                /* on unix, if using x-window then can't set ANSI || XANSI bits, and update clipboard settings */
                if(!(meSystemCfg & meSYSTEM_CONSOLE))
#endif
                {
                    meSystemCfg &= ~(meSYSTEM_ANSICOLOR|meSYSTEM_XANSICOLOR);
                    TTinitClipboard();
                }
#ifdef _ME_CONSOLE
                else
#endif
#endif
#ifdef _ME_CONSOLE
                    TCAPinitDrawChar();
#endif
#endif
#if MEOPT_CLIENTSERVER
                /* open or close the client server files */
                if(meSystemCfg & meSYSTEM_CLNTSRVR)
                    TTopenClientServer ();
                else
                    TTkillClientServer ();
#endif
                /* may changes may case a redraw so always redraw next time */
                sgarbf = meTRUE ;
                break;
            }
#if MEOPT_WORDPRO
        case EVBUFFILLCOL:
            {
                meInt col=meAtoi(vvalue);
                if(col <= 0)
                    frameCur->windowCur->buffer->fillcol = 0 ;
                else
                    frameCur->windowCur->buffer->fillcol = (meUShort) col ;
                break;
            }
        case EVBUFFILLMODE:
            frameCur->windowCur->buffer->fillmode = *vvalue ;
            break;
        case EVFILLBULLET:
            meStrncpy(fillbullet,vvalue,15);
            fillbullet[15] = '\0' ;
            break;
        case EVFILLBULLETLEN:
            fillbulletlen = (meUByte) meAtoi (vvalue);
            break;
        case EVFILLCOL:
            {
                meInt col=meAtoi(vvalue);
                if(col <= 0)
                    fillcol = 0 ;
                else
                    fillcol = (meUShort) col ;
                break;
            }
            break;
        case EVFILLEOS:
            meStrncpy(filleos,vvalue,15);
            filleos[15] = '\0' ;
            break;
        case EVFILLEOSLEN:
            filleoslen = (meUByte) meAtoi (vvalue);
            break;
        case EVFILLIGNORE:
            meStrncpy(fillignore,vvalue,15);
            fillignore[15] = '\0' ;
            break;
        case EVFILLMODE:
            fillmode = *vvalue ;
            break;
#endif
#if MEOPT_EXTENDED
        case EVTIME:
            timeOffset = meAtoi(vvalue);
            break;
#endif
        case EVFRMDPTH:
            return frameChangeDepth(meTRUE,meAtoi(vvalue)-(frameCur->depth+1));
#if MEOPT_EXTENDED
        case EVFRMID:
            frameCur->id = meAtoi(vvalue) ;
            break;
        case EVFRMTITLE:
            if(frameTitle != NULL)
                meFree(frameTitle) ;
            frameTitle = meStrdup(vvalue) ;
            /* force a redraw */
            sgarbf = meTRUE ;
            break;
#endif
        case EVFRMWDTH:
            return frameChangeWidth(meTRUE,meAtoi(vvalue)-frameCur->width);
        case EVABSCOL:
            return setcwcol(frameCur->windowCur,meAtoi(vvalue));
#if MEOPT_EXTENDED
        case EVABSLINE:
            return meWindowGotoAbsLine(frameCur->windowCur,meAtoi(vvalue));
        case EVWMRKCOL:
            if((status=meAtoi(vvalue)) < 0)
            {
                frameCur->windowCur->markOffset = 0 ;
                return meFALSE ;
            }
            else if(status && ((frameCur->windowCur->buffer->intFlag & BIFNACT) == 0) &&
                    (frameCur->windowCur->markLine != NULL) &&
                    (status > meLineGetLength(frameCur->windowCur->markLine)))
            {
                frameCur->windowCur->markOffset = meLineGetLength(frameCur->windowCur->markLine) ;
                return meFALSE ;
            }
            frameCur->windowCur->markOffset = status ;
            return meTRUE ;
        case EVWMRKLINE:
            if((status=meAtoi(vvalue)) < 0)
                return meFALSE ;
            frameCur->windowCur->markOffset = 0 ;
            if(status == 0)
            {
                frameCur->windowCur->markLine = NULL ;
                frameCur->windowCur->markLineNo = 0 ;
            }
            else if(frameCur->windowCur->buffer->intFlag & BIFNACT)
                frameCur->windowCur->markLineNo = status ;
            else
            {
                meLine   *odotp ;
                meUShort  odoto ;
                meInt     lineno ;
                
                odotp = frameCur->windowCur->dotLine ;
                lineno = frameCur->windowCur->dotLineNo ;
                odoto = frameCur->windowCur->dotOffset ;
                if((status = meWindowGotoLine(frameCur->windowCur,status)) > 0)
                {
                    frameCur->windowCur->markLine = frameCur->windowCur->dotLine ;
                    frameCur->windowCur->markLineNo = frameCur->windowCur->dotLineNo ;
                }
                frameCur->windowCur->dotLine = odotp ;
                frameCur->windowCur->dotLineNo = lineno ;
                frameCur->windowCur->dotOffset = odoto ;
                return status ;
            }
            return meTRUE ;
#endif
        case EVCURCOL:
            if((status=meAtoi(vvalue)) < 0)
            {
                frameCur->windowCur->dotOffset = 0 ;
                return meFALSE ;
            }
            else if(status && ((frameCur->windowCur->buffer->intFlag & BIFNACT) == 0) &&
                    (status > meLineGetLength(frameCur->windowCur->dotLine)))
            {
                frameCur->windowCur->dotOffset = meLineGetLength(frameCur->windowCur->dotLine) ;
                return meFALSE ;
            }
            frameCur->windowCur->dotOffset = status ;
            return meTRUE ;
        case EVCURLINE:
            if((status=meAtoi(vvalue)) <= 0)
                return meFALSE ;
            if(frameCur->windowCur->buffer->intFlag & BIFNACT)
            {
                frameCur->windowCur->dotLineNo = status ;
                return meTRUE ;
            }
            return meWindowGotoLine(frameCur->windowCur,status) ;
        case EVWINCHRS:
            {
                meUByte cc ;
                int len;
                len = meStrlen(vvalue);
                if (len > WCLEN)
                    len = WCLEN;
                while(--len >= 0)
                {
                    cc = vvalue[len] ;
                    if(!isPokable(cc))
                        cc = '.' ;
                    windowChars[len] = cc ;
                }
                if(meSystemCfg & meSYSTEM_SHOWWHITE)
                {
                    displayTab = windowChars[WCDISPTAB];
                    displayNewLine = windowChars[WCDISPCR];
                    displaySpace = windowChars[WCDISPSPACE];
                }
            }
            break;
#if MEOPT_EXTENDED
        case EVWFLAGS:
            frameCur->windowCur->flags = (meUShort) meAtoi(vvalue) ;
            break;
        case EVWID:
            frameCur->windowCur->id = meAtoi(vvalue) ;
            break;
        case EVWXSCROLL:
            if((status=meAtoi(vvalue)) < 0)
                status = 0 ;
            if(frameCur->windowCur->horzScrollRest != status)
            {
                frameCur->windowCur->horzScrollRest = status ;
                frameCur->windowCur->updateFlags |= WFREDRAW ;        /* Force a screen update */
                updCursor(frameCur->windowCur) ;
            }
            return meTRUE ;
        case EVWXCLSCROLL:
            if((status=meAtoi(vvalue)) < 0)
                status = 0 ;
            else if(status >= meLineGetLength(frameCur->windowCur->dotLine))
                status = meLineGetLength(frameCur->windowCur->dotLine)-1 ;
            if(frameCur->windowCur->horzScroll != status)
            {
                frameCur->windowCur->horzScroll = status ;
                frameCur->windowCur->updateFlags |= WFREDRAW ;        /* Force a screen update */
                updCursor(frameCur->windowCur) ;
            }
            return meTRUE ;
        case EVWYSCROLL:
            if((status=meAtoi(vvalue)) < 0)
                status = 0 ;
            else if(frameCur->windowCur->buffer->lineCount && (status >= frameCur->windowCur->buffer->lineCount))
                status = frameCur->windowCur->buffer->lineCount-1 ;
            if(frameCur->windowCur->vertScroll != status)
            {
                frameCur->windowCur->vertScroll = status ;
                frameCur->windowCur->updateFlags |= WFMAIN|WFSBOX|WFLOOKBK ;
                reframe(frameCur->windowCur) ;
            }
            return meTRUE ;
#endif
        case EVCBUFNAME:
            {
                meBuffer *cbp=frameCur->windowCur->buffer;
                unlinkBuffer(cbp);
                if((vvalue[0] == '\0') || (bfind(vvalue,0) != NULL) ||
                   ((nn = meStrdup(vvalue)) == NULL))
                {
                    /* if the name is used */
                    linkBuffer(cbp);
                    return meABORT;
                }
                meFree(cbp->name);
                cbp->name = nn;
                frameAddModeToWindows(WFMODE);
                linkBuffer(cbp);
                break;
            }
        case EVBUFFMOD:
            frameCur->windowCur->buffer->stats.stmode = (meUShort) meAtoi(vvalue) ;
            break ;
        case EVGLOBFMOD:
            meUmask = (meUShort) meAtoi(vvalue) ;
            break ;
        case EVCFNAME:
            meStrrep(&frameCur->windowCur->buffer->fileName,vvalue);
            frameAddModeToWindows(WFMODE) ;
            break;
#if MEOPT_DEBUGM
        case EVDEBUG:
            macbug = (meByte) meAtoi(vvalue);
            break;
#endif
#if MEOPT_TIMSTMP
        case EVTIMSTMP:
            meStrncpy(&time_stamp[0], vvalue, meTIME_STAMP_SIZE_MAX-1);
            time_stamp[meTIME_STAMP_SIZE_MAX-1] = '\0';
            break;
#endif
        case EVINDWIDTH:
            if((indentWidth = (meUByte) meAtoi(vvalue)) <= 0)
                indentWidth = 1 ;
            break;
        case EVBUFINDWIDTH:
            if((frameCur->windowCur->buffer->indentWidth = (meUByte) meAtoi(vvalue)) <= 0)
                frameCur->windowCur->buffer->indentWidth = 1 ;
            break;
        case EVTABWIDTH:
            if((tabWidth = (meUByte) meAtoi(vvalue)) <= 0)
                tabWidth = 1 ;
            frameAddModeToWindows(WFRESIZE) ;
            break;
        case EVBUFTABWIDTH:
            if((frameCur->windowCur->buffer->tabWidth = (meUByte) meAtoi(vvalue)) <= 0)
                frameCur->windowCur->buffer->tabWidth = 1 ;
            frameAddModeToWindows(WFRESIZE) ;
            break;
        case EVSRCHPATH:
            meStrrep(&searchPath,vvalue) ;
            break;
        case EVHOMEDIR:
            fileNameSetHome(vvalue) ;
            break;
#if MEOPT_EXTENDED
        case EVBUFXLATE:
            frameCur->windowCur->buffer->xlateFlag = (meByte) meAtoi(vvalue);
            break;
        case EVSHWMDS:
            {
                meUByte cc ;
                int nn ;
                for(nn=0; nn < MDNUMMODES ; nn++) 
                {
                    if((cc=*vvalue++) == '\0')
                        break ;
                    else if(cc == '0')
                        meModeClear(modeLineDraw,nn) ;
                    else
                        meModeSet(modeLineDraw,nn) ;
                }
                frameAddModeToWindows(WFMODE) ;
                break ;
            }
        case EVSHWRGN:
            selhilight.uFlags = (meUShort) meAtoi(vvalue);
            break;
#endif
        case EVCURSORBLK:
            timerKill(CURSOR_TIMER_ID) ;
            if((cursorBlink = meAtoi(vvalue)) > 0)
                /* kick off the timer */
                TThandleBlink(1) ;
            break ;
#if MEOPT_COLOR
        case EVCURSORCOL:
            if((cursorColor = (meColor) meAtoi(vvalue)) >= noColors)
                cursorColor = meCOLOR_FDEFAULT ;
            break ;
        case EVMLSCHM:
            mlScheme = convertUserScheme(meAtoi(vvalue),mlScheme);
            break ;
        case EVMDLNSCHM:
            mdLnScheme = convertUserScheme(meAtoi(vvalue),mdLnScheme);
            frameAddModeToWindows(WFMODE) ;
            break ;
        case EVGLOBSCHM:
            globScheme = convertUserScheme(meAtoi(vvalue),globScheme);
            frameCur->video.lineArray[frameCur->depth].endp = frameCur->width ;
            sgarbf = meTRUE ;
            TTsetBgcol() ;
            break ;
        case EVTRNCSCHM:
            trncScheme = convertUserScheme(meAtoi(vvalue),trncScheme);
            sgarbf = meTRUE ;
            break ;
        case EVBUFSCHM:
            frameCur->windowCur->buffer->scheme = convertUserScheme(meAtoi(vvalue),frameCur->windowCur->buffer->scheme);
            meBufferAddModeToWindows(frameCur->windowCur->buffer,WFRESIZE) ;
            break ;
        case EVSBARSCHM:
            sbarScheme = convertUserScheme(meAtoi(vvalue),sbarScheme);
            frameAddModeToWindows(WFRESIZE|WFSBAR) ;
            break;
#if MEOPT_OSD
        case EVOSDSCHM:
            osdScheme = convertUserScheme(meAtoi(vvalue),osdScheme);
            break ;
#endif
#endif
        case EVSBAR:    
            gsbarmode = meAtoi(vvalue) & WMUSER;
            meFrameResizeWindows(frameCur,0);
            break;
#if MEOPT_IPIPES
        case EVBUFIPIPE:
            frameCur->windowCur->buffer->ipipeFunc = decode_fncname(vvalue,1) ;
            break ;
#endif
#if MEOPT_FILEHOOK
        case EVBUFBHK:
            frameCur->windowCur->buffer->bhook = decode_fncname(vvalue,1) ;
            break ;
        case EVBUFDHK:
            frameCur->windowCur->buffer->dhook = decode_fncname(vvalue,1) ;
            break ;
        case EVBUFEHK:
            frameCur->windowCur->buffer->ehook = decode_fncname(vvalue,1) ;
            break ;
        case EVBUFFHK:    
            frameCur->windowCur->buffer->fhook = decode_fncname(vvalue,1) ;
            break ;
#endif
#if MEOPT_EXTENDED
        case EVBUFINP:
            frameCur->windowCur->buffer->inputFunc = decode_fncname(vvalue,1) ;
            break ;
        case EVLINEFLAGS:
            frameCur->windowCur->dotLine->flag = ((meLineFlag) (meAtoi(vvalue) & meLINE_SET_MASK)) |
                  (frameCur->windowCur->dotLine->flag & ~meLINE_SET_MASK) ;
            break ;
#endif
#if MEOPT_HILIGHT
        case EVLINESCHM:
            {
                meWindow *cwp=frameCur->windowCur;
                register int ii ;
                meScheme schm ;
                
                ii = meAtoi(vvalue) ;
                if(ii >= 0)
                {
                    schm = convertUserScheme(ii,cwp->buffer->scheme);
                    for(ii=1 ; ii<meLINE_SCHEME_MAX ; ii++)
                        if(cwp->buffer->lscheme[ii] == schm)
                            break ;
                    if(ii == meLINE_SCHEME_MAX)
                    {
                        if((ii=cwp->buffer->lschemeNext+1) == meLINE_SCHEME_MAX)
                            ii = 1 ;
                        cwp->buffer->lscheme[ii] = schm ;
                        cwp->buffer->lschemeNext = ii ;
                    }
                    cwp->dotLine->flag = (cwp->dotLine->flag & ~meLINE_SCHEME_MASK) | (ii << meLINE_SCHEME_SHIFT) ;
                }
                else if(meLineIsSchemeSet(cwp->dotLine))
                    cwp->dotLine->flag = (cwp->dotLine->flag & ~meLINE_SCHEME_MASK) ;
                lineSetChanged(WFMAIN) ;
                break ;
            }
        case EVBUFHIL:
            {
                meBuffer *cbp=frameCur->windowCur->buffer;
                if(((cbp->hilight = (meUByte) meAtoi(vvalue)) >= noHilights) ||
                   (hilights[cbp->hilight] == NULL))
                    cbp->hilight = 0;
                meBufferAddModeToWindows(cbp,WFRESIZE);
                break;
            }
        case EVBUFIND:
            {
                meBuffer *cbp=frameCur->windowCur->buffer;
                if(((cbp->indent = (meUByte) meAtoi(vvalue)) >= noIndents) ||
                   (indents[cbp->indent] == NULL))
                    cbp->indent = 0;
                break;
            }
#endif
#if MEOPT_EXTENDED
        case EVBUFMASK:
            {
                int ii ;
                isWordMask = 0 ;
                for(ii=0 ; ii<8 ; ii++)
                    if(meStrchr(vvalue,charMaskFlags[ii]) != NULL)
                        isWordMask |= 1<<ii ;
                if(frameCur->windowCur->buffer->isWordMask != isWordMask)
                {
                    frameCur->windowCur->buffer->isWordMask = isWordMask ;
#if MEOPT_MAGIC
                    mereRegexClassChanged() ;
#endif
                }
                break ;
            }
#endif
#if MEOPT_FILENEXT
        case EVFILETEMP:
            meStrrep(&flNextFileTemp,vvalue) ;
            break ;
        case EVLINETEMP:
            meStrrep(&flNextLineTemp,vvalue) ;
            break ;
#endif
#if MEOPT_RCS
        case EVRCSFILE:
            meStrrep(&rcsFile,vvalue) ;
            break ;
        case EVRCSCICOM:
            meStrrep(&rcsCiStr,vvalue) ;
            break ;
        case EVRCSCIFCOM:
            meStrrep(&rcsCiFStr,vvalue) ;
            break ;
        case EVRCSCOCOM:
            meStrrep(&rcsCoStr,vvalue) ;
            break ;
        case EVRCSCOUCOM:
            meStrrep(&rcsCoUStr,vvalue) ;
            break ;
        case EVRCSUECOM:
            meStrrep(&rcsUeStr,vvalue) ;
            break ;
#endif
        case EVUSERPATH:
            meStrrep(&meUserPath,vvalue);
            break;
        case EVSCROLL:
            scrollFlag = (meUByte) meAtoi(vvalue) ;
            break ;
        case EVQUIET:
            quietMode = (meUByte) meAtoi(vvalue) ;
            break;
#if MEOPT_EXTENDED
        case EVFILEIGNORE:
            meStrrep(&fileIgnore,vvalue) ;
            break ;
        case EVBNAMES:
            meNullFree(buffNames.list);
            buffNames.list = NULL;
            buffNames.curr = 0;
            if(meRegexComp(&meRegexStrCmp,vvalue,(buffNames.exact) ? meRSTRCMP_WHOLE:meRSTRCMP_ICASE|meRSTRCMP_WHOLE) != meREGEX_OKAY)
                return mlwrite(MWABORT,(meUByte *)"[bad regex]");
            meStrrep(&buffNames.mask,vvalue);
            if(buffNames.mask != NULL)
                buffNames.size = createBuffList(&buffNames.list,0) ;
            break ;
        case EVCNAMES:
            meNullFree(commNames.list);
            commNames.list = NULL;
            commNames.curr = 0;
            if(meRegexComp(&meRegexStrCmp,vvalue,(commNames.exact) ? meRSTRCMP_WHOLE:meRSTRCMP_ICASE|meRSTRCMP_WHOLE) != meREGEX_OKAY)
                return mlwrite(MWABORT,(meUByte *)"[bad regex]");
            meStrrep(&commNames.mask,vvalue);
            if(commNames.mask != NULL)
            {
                commNames.size = createCommList(&commNames.list,1) ;
                sortStrings(commNames.size,commNames.list,0,(meIFuncSS) strcmp) ;
            }
            break ;
        case EVFNAMES:
            {
                meUByte *mm, *ss, cc, dd ;
                
                ss = mm = vvalue ;
                while(((cc=*ss++) != '\0') && (cc != '[') && ((dd=*ss) != '\0') &&
                      ((cc != '\\') || ((dd != '(') && (dd != '|') && (dd != '<'))))
                {
#ifdef _DRV_CHAR
                    if((cc == '/') || ((cc == _DRV_CHAR) && (*ss != '/')))
#else
                    if(cc == '/')
#endif
                        mm = ss ;
                }
                fileNames.curr = 0 ;
                if(meRegexComp(&meRegexStrCmp,mm,(fileNames.exact) ? meRSTRCMP_WHOLE:meRSTRCMP_ICASE|meRSTRCMP_WHOLE) != meREGEX_OKAY)
                    return mlwrite(MWABORT,(meUByte *)"[bad regex]");
                meStrrep(&fileNames.mask,mm);
                if(fileNames.mask != NULL)
                {
                    meUByte buff[meBUF_SIZE_MAX], path[meBUF_SIZE_MAX] ;
#ifdef _DRV_CHAR
                    if((isAlpha(mm[0]) || (mm[0] == '.' )) && (mm[1] == _DRV_CHAR))
                        path[0] = '\0' ;
                    else
#endif
                    {
                        *mm = '\0' ;
                        getFilePath(frameCur->windowCur->buffer->fileName,buff) ;
                        meStrcat(buff,vvalue);
                        pathNameCorrect(buff,PATHNAME_COMPLETE,path,NULL);
                    }
                    getDirectoryList(path,&fileNames);
                    meStrcpy(resultStr,path);
                }
                break;
            }
        case EVMNAMES:
            modeNames.curr = 0 ;
            if(meRegexComp(&meRegexStrCmp,vvalue,(modeNames.exact) ? meRSTRCMP_WHOLE:meRSTRCMP_ICASE|meRSTRCMP_WHOLE) != meREGEX_OKAY)
                return mlwrite(MWABORT,(meUByte *)"[bad regex]");
            meStrrep(&modeNames.mask,vvalue);
            break;
        case EVVNAMES:
            freeFileList(varbNames.size,varbNames.list);
            varbNames.list = NULL;
            varbNames.curr = 0;
            if(meRegexComp(&meRegexStrCmp,vvalue,(varbNames.exact) ? meRSTRCMP_WHOLE:meRSTRCMP_ICASE|meRSTRCMP_WHOLE) != meREGEX_OKAY)
                return mlwrite(MWABORT,(meUByte *)"[bad regex]");
            meStrrep(&varbNames.mask,vvalue);
            if(varbNames.mask != NULL)
                varbNames.size = createVarList(&varbNames.list) ;
            break;
#endif
#if MEOPT_SPELL
        case EVFINDWORDS:
            return findWordsInit(vvalue);
#endif
#if MEOPT_EXTENDED
        case EVRECENTKEYS:
            if(vvalue[0] != '\0')
                return meFALSE ;
            TTkeyBuf[TTnextKeyIdx] = 0 ;
            break ;
#endif
        case EVRESULT:                      /* Result string - assignable */
            meStrcpy (resultStr, vvalue);
            break;
        case -1:
            {
                /* unknown so set an environment variable */
                meUByte buff[meBUF_SIZE_MAX+meSBUF_SIZE_MAX], *ss ;
                sprintf((char *) buff,"%s=%s",nn,vvalue) ;
                if((ss=meStrdup(buff)) != NULL)
                    mePutenv(ss) ;
                break ;
            }
        default:
            /* default includes EVUSERNAME EVCBUFBACKUP EVSTATUS EVBUILD EVPLATFORM 
             * EVSYSRET EVUSEX, EVVERSION, EVWMDLINE, EVEOBLINE or system dependant vars
             * where this isn't the system (e.g. use-x) which cant be set
             */
            return meFALSE ;
        }
        break ;
    default:
        /* else its not legal....bitch */
        return mlwrite(MWABORT,(meUByte *)"[No such variable]");
    }
    return meTRUE ;
}

static meUByte *
gtenv(meUByte *vname)   /* vname   name of environment variable to retrieve */
{
    int ii;
    register meUByte *ret;
#if MEOPT_EXTENDED
    register meNamesList *mv;
#endif
    
    biChopFindString(vname,envars,NEVARS,ii);
    switch(ii)
    {
        /* Fetch the appropriate value */
    case EVAUTOTIME:    return meItoa(autoTime);
#if MEOPT_MOUSE
    case EVDELAYTIME:   return meItoa(delayTime);
    case EVREPEATTIME:  return meItoa(repeatTime);
#endif
#if MEOPT_CALLBACK
    case EVIDLETIME:    return meItoa(idleTime);
#endif
    case EVBOXCHRS:     return boxChars;
    case EVBUILD:       return buildStr;
    case EVMODELINE:    return modeLineStr;
    case EVPLATFORM:    return machineName;
    case EVPROGDT:
#ifdef _WIN32
        return mePtos(meProgData);
#else
        if(homedir == NULL)
            return emptym;
        meStrcpy(evalResult,homedir);
#ifdef _DOS
        meStrcat(evalResult,"jasspa/");
#else
        meStrcat(evalResult,".config/jasspa/");
#endif
        return evalResult;
#endif
    case EVPROGNM:      return meProgName;
    case EVCURSORX:     return meItoa(frameCur->mainColumn);
    case EVCURSORY:     return meItoa(frameCur->mainRow);
    case EVSYSTEM:      return meItoa(meSystemCfg);
#if MEOPT_WORDPRO
    case EVBUFFILLCOL:  return meItoa(frameCur->windowCur->buffer->fillcol);
    case EVBUFFILLMODE:
        evalResult[0] = frameCur->windowCur->buffer->fillmode;
        evalResult[1] = '\0';
        return evalResult;
    case EVFILLBULLET:  return fillbullet;
    case EVFILLBULLETLEN:return meItoa(fillbulletlen);
    case EVFILLCOL:     return meItoa(fillcol);
    case EVFILLEOS:     return filleos;
    case EVFILLEOSLEN:  return meItoa(filleoslen);
    case EVFILLIGNORE:  return fillignore;
    case EVFILLMODE:
        evalResult[0] = fillmode ;
        evalResult[1] = '\0' ;
        return evalResult ;
#endif
#if MEOPT_EXTENDED
    case EVPAUSETIME:   return meItoa(pauseTime);
    case EVFSPROMPT:    return meItoa(fileSizePrompt);
    case EVKEPTVERS:    return meItoa(keptVersions);
    case EVMOUSE:       return meItoa(meMouseCfg);
    case EVMOUSEPOS:    return meItoa(mouse_pos);
    case EVMOUSEX:      return meItoa(mouse_X);
    case EVMOUSEY:      return meItoa(mouse_Y);
    case EVBNAMES:
        mv = &buffNames ;
        goto handle_namesvar ;
    case EVCNAMES:
        mv = &commNames ;
        goto handle_namesvar ;
    case EVFNAMES:
        mv = (meNamesList *) &fileNames ;
        goto handle_namesvar ;
    case EVMNAMES:
        mv = &modeNames ;
        goto handle_namesvar ;
    case EVVNAMES:
        mv = &varbNames ;
handle_namesvar:
        if(mv->mask == NULL)
            return abortm ;
        while(mv->curr < mv->size)
        {
            meUByte *ss = mv->list[(mv->curr)++] ;
            if(regexStrCmp(ss,mv->mask,(mv->exact) ? meRSTRCMP_WHOLE:meRSTRCMP_ICASE|meRSTRCMP_WHOLE))
                return ss ;
        }
        return emptym;
    case EVTEMPNAME:
        mkTempName(evalResult,NULL,NULL);
        return evalResult;
#endif
    case EVVERSION:
        return (meUByte *) meVERSION_CODE;
#if MEOPT_SPELL
    case EVFINDWORDS:   return findWordsNext();
#endif
#if MEOPT_EXTENDED
    case EVRECENTKEYS:
        {
            int ii, jj, kk ;
            meUShort cc ;
            for(ii=100,jj=TTnextKeyIdx,kk=0 ; --ii>0 ; )
            {
                if(((cc=TTkeyBuf[jj++]) == 0) ||
                   ((kk+=meGetStringFromChar(cc,evalResult+kk)) > meBUF_SIZE_MAX-20))
                    break;
                evalResult[kk++] = ' ';
                if(jj == KEYBUFSIZ)
                    jj = 0;
            }
            evalResult[kk] = '\0';
            return evalResult;
        }
#endif
    case EVRESULT:      return resultStr;
#if MEOPT_EXTENDED
    case EVTIME:
        {
            struct meTimeval tp;            /* Time interval for current time */
            struct tm *time_ptr;            /* Pointer to time frame. */
            time_t clock;                   /* Time in machine format. */
            
            /* Format the result. Format is:-
             * 
             *    YYYYCCCMMDDWhhmmssSSS
             * 
             * Where:
             *     YYYY - Year i.e. 1997
             *     CCC  - Day of the week since January 1st (0-365).
             *     MM   - Month of year 1-12
             *     DD   - Day of the month 1-31
             *     W    - Weekday since sunday 0-6 (sunday == 0)
             *     hh   - Hours 0-23
             *     mm   - Minutes 0-59
             *     ss   - Seconds 0-59
             *     SSS  - Milliseconds 0-999
             */
            gettimeofday(&tp,NULL);
            clock = tp.tv_sec + timeOffset;               /* Get system time */
            time_ptr = (struct tm *) localtime (&clock);  /* Get time frame */
            /* SWP - Took out the 0 padding because of macro use.
             *       ME interprets numbers of the form 0## as octagon based numbers
             *       so in september the number is 09 which is illegal etc.
             */
            sprintf((char *) evalResult,"%4d%3d%2d%2d%1d%2d%2d%2d%3d",
                    time_ptr->tm_year+1900,  /* Year - 2000 should be ok !! */
                    time_ptr->tm_yday,       /* Year day */
                    time_ptr->tm_mon+1,      /* Month */
                    time_ptr->tm_mday,       /* Day of the month */
                    time_ptr->tm_wday,       /* Day of the week */
                    time_ptr->tm_hour,       /* Hours */
                    time_ptr->tm_min,        /* Minutes */
                    time_ptr->tm_sec,        /* Seconds */
                    (int) (tp.tv_usec/1000));/* Milliseconds */
            return evalResult;
        }
    case EVUNIXTIME:
        sprintf((char *) evalResult,"%d",(unsigned int) time(NULL));
        return evalResult;
        
    case EVRANDOM:
        if(meRndSeed[0] == 0)
            meXoshiro128Seed();
        return meItoa((meXoshiro128Next() >> 8));
#endif
    case EVFRMDPTH:     return meItoa(frameCur->depth + 1);
#if MEOPT_EXTENDED
    case EVFRMID:       return meItoa(frameCur->id);
#endif
    case EVFRMWDTH:     return meItoa(frameCur->width);
    case EVABSCOL:      return meItoa(getcwcol(frameCur->windowCur));
    case EVCURCOL:      return meItoa(frameCur->windowCur->dotOffset);
    case EVCURLINE:     return meItoa(frameCur->windowCur->dotLineNo+1);
#if MEOPT_EXTENDED
    case EVABSLINE:     return meItoa(meWindowGotoAbsLine(frameCur->windowCur,-1)+1);
    case EVEOBLINE:     return meItoa(frameCur->windowCur->buffer->lineCount+1);
    case EVWMRKCOL:     return meItoa(frameCur->windowCur->markOffset);
    case EVWMRKLINE:    return meItoa((frameCur->windowCur->markLine == NULL) ? 0:frameCur->windowCur->markLineNo+1);
    case EVBMDLINE:
        if((ret=frameCur->windowCur->buffer->modeLineStr) != NULL)
            return ret;
        return modeLineStr;
    case EVWXSCROLL:    return meItoa(frameCur->windowCur->horzScrollRest);
    case EVWXCLSCROLL:  return meItoa(frameCur->windowCur->horzScroll);
    case EVWYSCROLL:    return meItoa(frameCur->windowCur->vertScroll);
    case EVWMDLINE:     return meItoa(frameCur->windowCur->frameRow+frameCur->windowCur->textDepth);
    case EVWSBAR:       return meItoa(frameCur->windowCur->frameColumn+frameCur->windowCur->textWidth);
    case EVWFLAGS:      return meItoa(frameCur->windowCur->flags);
    case EVWID:         return meItoa(frameCur->windowCur->id);
    case EVWDEPTH:      return meItoa(frameCur->windowCur->textDepth);
    case EVWWIDTH:      return meItoa(frameCur->windowCur->textWidth);
#endif
    case EVWINCHRS:     return windowChars;
    case EVCBUFBACKUP:
        if(((ret=frameCur->windowCur->buffer->fileName) == NULL) || createBackupName(evalResult,ret,'~',0))
            return emptym ;
#if MEOPT_EXTENDED
#ifndef _DOS
        if(!(meSystemCfg & meSYSTEM_DOSFNAMES) && (keptVersions > 0))
            meStrcpy(evalResult+meStrlen(evalResult)-1,".~0~") ;
#endif
#endif
        return evalResult ;
    case EVCBUFNAME:    return frameCur->windowCur->buffer->name ;
    case EVBUFFMOD:     return meItoa((((meInt) frameCur->windowCur->buffer->fileFlag) << 16) | ((meInt) frameCur->windowCur->buffer->stats.stmode));
    case EVGLOBFMOD:    return meItoa(meUmask);
    case EVCFNAME:      return mePtos(frameCur->windowCur->buffer->fileName);
#if MEOPT_DEBUGM
    case EVDEBUG:       return meItoa(macbug);
#endif
    case EVSTATUS:      return meLtoa(cmdstatus);
#if MEOPT_TIMSTMP
    case EVTIMSTMP:     return time_stamp;
#endif
    case EVINDWIDTH:    return meItoa(indentWidth);
    case EVBUFINDWIDTH: return meItoa(frameCur->windowCur->buffer->indentWidth);
    case EVTABWIDTH:    return meItoa(tabWidth);
    case EVBUFTABWIDTH: return meItoa(frameCur->windowCur->buffer->tabWidth);
    case EVSRCHPATH:    return mePtos(searchPath);
    case EVHOMEDIR:     return mePtos(homedir);
#if MEOPT_EXTENDED
    case EVBUFXLATE:    return meItoa(frameCur->windowCur->buffer->xlateFlag);
    case EVMODECHRS:    return modeCode;
    case EVSHWMDS:
        {
            ret = evalResult;
            for (ii=0; ii < MDNUMMODES ; ii++) 
                *ret++ = (meModeTest(modeLineDraw,ii)) ? '1':'0' ;
            *ret = '\0' ;
            return evalResult;
        }
    case EVSHWRGN:      return meItoa(selhilight.uFlags);
#endif
    case EVCURSORBLK:   return meItoa(cursorBlink);
#if MEOPT_COLOR
    case EVCURSORCOL:   return meItoa(cursorColor);
    case EVMLSCHM:      return meItoa(mlScheme/meSCHEME_STYLES);
    case EVMDLNSCHM:    return meItoa(mdLnScheme/meSCHEME_STYLES);
    case EVSBARSCHM:    return meItoa(sbarScheme/meSCHEME_STYLES);
    case EVGLOBSCHM:    return meItoa(globScheme/meSCHEME_STYLES);
    case EVTRNCSCHM:    return meItoa(trncScheme/meSCHEME_STYLES);
    case EVBUFSCHM:     return meItoa(frameCur->windowCur->buffer->scheme/meSCHEME_STYLES);
    case EVSBAR:        return meItoa(gsbarmode);
#if MEOPT_OSD
    case EVOSDSCHM:     return meItoa(osdScheme/meSCHEME_STYLES);
#endif
#endif
#if MEOPT_IPIPES
    case EVBUFIPIPE:
        ii = frameCur->windowCur->buffer->ipipeFunc ;
        goto hook_jump ;
#endif
#if MEOPT_FILEHOOK
    case EVBUFBHK:
        ii = frameCur->windowCur->buffer->bhook ;
        goto hook_jump ;
    case EVBUFDHK:
        ii = frameCur->windowCur->buffer->dhook ;
        goto hook_jump ;
    case EVBUFEHK:
        ii = frameCur->windowCur->buffer->ehook ;
        goto hook_jump ;
    case EVBUFFHK:    
        ii = frameCur->windowCur->buffer->fhook ;
        goto hook_jump ;
#endif
#if MEOPT_EXTENDED
    case EVBUFINP:
        ii = frameCur->windowCur->buffer->inputFunc ;
hook_jump:
        if(ii < 0)
            evalResult[0] = '\0';
        else
            meStrcpy(evalResult,getCommandName(ii)) ;
        return evalResult ;
    case EVLINEFLAGS:   return meItoa(frameCur->windowCur->dotLine->flag) ;
    case EVBUFMASK:
        {
            meUByte *ss=evalResult ;
            int   ii ;
            for(ii=0 ; ii<8 ; ii++)
                if(isWordMask & (1<<ii))
                    *ss++ = charMaskFlags[ii] ;
            *ss = '\0' ;
            return evalResult ;
        }
    case EVFILEIGNORE:  return mePtos(fileIgnore) ;
#endif
#if MEOPT_HILIGHT
    case EVLINESCHM:    return meItoa(meLineIsSchemeSet(frameCur->windowCur->dotLine) ? 
                                      (frameCur->windowCur->buffer->lscheme[meLineGetSchemeIndex(frameCur->windowCur->dotLine)]/meSCHEME_STYLES):-1) ;
    case EVBUFHIL:      return meItoa(frameCur->windowCur->buffer->hilight);
    case EVBUFIND:      return meItoa(frameCur->windowCur->buffer->indent);
#endif
#if MEOPT_FILENEXT
    case EVFILETEMP:    return mePtos(flNextFileTemp) ;
    case EVLINETEMP:    return mePtos(flNextLineTemp) ;
#endif
#if MEOPT_RCS
    case EVRCSFILE:     return mePtos(rcsFile) ;
    case EVRCSCOCOM:    return mePtos(rcsCoStr) ;
    case EVRCSCOUCOM:   return mePtos(rcsCoUStr) ;
    case EVRCSCICOM:    return mePtos(rcsCiStr) ;
    case EVRCSCIFCOM:   return mePtos(rcsCiFStr) ;
    case EVRCSUECOM:    return mePtos(rcsUeStr) ;
#endif
    case EVUSERNAME:    return mePtos(meUserName) ;
    case EVUSERPATH:    return mePtos(meUserPath) ;
    case EVSCROLL:      return meItoa(scrollFlag) ;
    case EVQUIET:       return meItoa(quietMode);
    }
    if((ret = meGetenv(vname)) == NULL)
        ret = errorm ; /* errorm on bad reference */
    
    return ret ;
}

/* look up a variable in the given list and return a pointer to the variable */
meVariable *
getUsrLclCmdVarP(meUByte *vname, register meVariable *vp)
{
    register int ii;
#if MEOPT_CMDHASH
    meUInt hash;
    
    meStringHash(vname,hash);
    while(vp != NULL)
    {
        if(vp->hash == hash)
        {
            if((ii=meStrcmp(vname,vp->name)) == 0)
                return vp;
            if(ii < 0)
                break;
        }
        else if(vp->hash > hash)
            break;
        vp = vp->next ;
    }
#else
    /* scan the list looking for the user var name */
    while(vp != NULL)
    {
        if((ii=meStrcmp(vname,vp->name)) == 0)
            return vp;
        if(ii < 0)
            break;
        vp = vp->next ;
    }
#endif
    return NULL;
}
/* look up a variable in the given list and return the value */
meUByte *
getUsrLclCmdVar(meUByte *vname, register meVariable *vp)
{
    register int ii;
#if MEOPT_CMDHASH
    meUInt hash;
    
    meStringHash(vname,hash);
    while(vp != NULL)
    {
        if(vp->hash == hash)
        {
            if((ii=meStrcmp(vname,vp->name)) == 0)
                return vp->value;
            if(ii < 0)
                break;
        }
        else if(vp->hash > hash)
            break;
        vp = vp->next ;
    }
#else
    /* scan the list looking for the user var name */
    while(vp != NULL)
    {
        if((ii=meStrcmp(vname,vp->name)) == 0)
            return vp->value;
        if(ii < 0)
            break;
        vp = vp->next ;
    }
#endif
    return errorm ;     /* return errorm on a bad reference */
}


static meUByte *
getMacroArg(int index)
{
    meRegister *crp;
    meUByte *oldestr, *ss;
    
    /* move the register pointer to the parent as any # reference
     * will be w.r.t the parent
     */
    crp = meRegCurr;
    if((ss=crp->execstr) == NULL)
        return NULL;
    oldestr = execstr ;
    meRegCurr = crp->prev ;
    if(alarmState & meALARM_VARIABLE)
        gmaLocalRegPtr = meRegCurr ;
    if(index == 1)
    {
        execstr = token(ss,evalResult);
        ss = getval(evalResult);
        crp->nextArg = 1;
    }
    else
    {
        if(crp->nextArg < index)
        {
            index -= crp->nextArg;
            crp->nextArg += index;
            ss = crp->nextArgStr;
        }
        execstr = ss ;
        do
        {
            execstr = token(execstr,evalResult);
            ss = getval(evalResult);
        } while(--index > 0);
    }
    crp->nextArgStr = execstr;
    execstr = oldestr;
    meRegCurr = crp;
    return ss;
}


static meUByte *
gtarg(meUByte *tkn)
{
    meUByte buff[meBUF_SIZE_MAX], prompt[meBUF_SIZE_MAX], buff2[meBUF_SIZE_MAX];
    
    switch(tkn[1])
    {
    case '#':
        if(alarmState & meALARM_VARIABLE)
            return tkn ;
        return meItoa(meRegCurr->n) ;
        
    case '?':
        if(alarmState & meALARM_VARIABLE)
            return tkn ;
        return meItoa(meRegCurr->f) ;
        
    case 'c':
        {
            meUShort key ;
            int kk, index ;
            
            kk = (tkn[3] == 'k') ;
            if(tkn[2] == 'c')
            {
                if(alarmState & meALARM_VARIABLE)
                    return tkn ;
                key = (meUShort) thisCommand ;
                index = thisIndex ;
            }
            else if(tkn[2] == 'l')
            {
                if(alarmState & meALARM_VARIABLE)
                    return tkn ;
                key = (meUShort) lastCommand ;
                index = lastIndex ;
            }
            else
            {
                if(tkn[2] == 'q')
                {
                    /* intercative single char read which will be quoted */
                    int cc ;
                    key = meGetKeyFromUser(meFALSE,0,meGETKEY_SILENT|meGETKEY_SINGLE) ;
                    if((cc=quoteKeyToChar(key)) > 0)
                    {
                        if(tkn[3] == 'k')
                        {
                            evalResult[0] = cc ;
                            evalResult[1] = '\0';
                            return evalResult ;
                        }
                        key = cc ;
                    }
                }
                else
                    key = meGetKeyFromUser(meFALSE, 1, 0) ;
                if(!kk)
                {
                    meUInt arg ;
                    index = decode_key(key,&arg) ;
                }
            }
            if(kk)
                meGetStringFromKey(key,evalResult) ;
            else if(index < 0)
            {
                if(key > 0xff)
                    return errorm ;
                evalResult[0] = (meUByte) key ;
                evalResult[1] = '\0';
            }
            else
                meStrcpy(evalResult,getCommandName(index)) ;
            return evalResult ;
        }
    case 'f':
        if(tkn[2] == 's')
        {
            /* frame store @fs <row> <col> */
            int row, col ;
            
            /* Get and evaluate the arguments */
            if((meGetString(NULL,0,0,buff,meBUF_SIZE_MAX) <= 0) ||
               ((row = meAtoi(buff)) < 0) || (row > frameCur->depth) ||
               (meGetString(NULL,0,0,buff,meBUF_SIZE_MAX) <= 0) ||
               ((col = meAtoi(buff)) < 0) || (col >= frameCur->width))
                evalResult[0] = 0 ;
            else if(tkn[3] == 's')
                return meItoa((frameCur->store[row].scheme[col] & meSCHEME_STYLE)/meSCHEME_STYLES) ;
            else
            {
                evalResult[0] = frameCur->store[row].text[col] ;
                evalResult[1] = 0 ;
            }
            return evalResult ;
        }
        break ;
        
    case 'h':
        {
            int ht, hn ;
            
            if(alarmState & meALARM_VARIABLE)
                return tkn ;
            
            /* get the history type */
            if((tkn[2] > '0') && (tkn[2] <= '4'))
                ht = tkn[2]-'0' ;
            else
                ht = 0 ;
            
            /* Get and evaluate the next argument - this is the history number */
            if(meGetString(NULL,0,0,buff,meBUF_SIZE_MAX) <= 0)
                return abortm ;
            hn = meAtoi(buff) ;
            
            return mePtos(strHist[(ht*meHISTORY_SIZE)+hn]) ;
        }
        
    case 'm':
        if(tkn[2] == 'l')
        {
            /* interactive argument */
            /* note: the result buffer cant be used directly as it is used
             *       in modeline which is is by update which can be used
             *       by meGetStringFromUser
             */
            static meUByte **strList=NULL ;
            static int strListSize=0 ;
            meUByte cc, *ss ;
            meUByte *tt, divChr ;
            int option=0 ;
            int ret, flag, defH ;
            
            if((flag=tkn[3]) != '\0')
            {
                flag = hexToNum(flag);
                if(((cc=tkn[4]) > '0') && (cc<='9'))
                    cc -= '0';
                else if(cc == 'a')
                    cc = 10;
                else if(cc == 'b')
                    cc = 11;
                else
                    cc = 12;
                if(cc == 1)
                    option = MLFILE;
                else if(cc < 11)
                {
                    if(cc == 7)
                    {
                        mlgsStrList = modeName;
                        mlgsStrListSize = MDNUMMODES;
                    }
                    else if(cc == 9)
                    {
                        flag |= 0x100;
                        cc = 7;
                    }
                    else if(cc == 10)
                    {
                        flag |= 0x300;
                        cc = 7;
                    }
                    option = 1 << (cc-2);
                }
                else if(cc == 11)
                    option = MLHISTLIST|MLNOSTORE;
#if (MLFILECASE != MLFILE)
                if(option & MLFILE)
                    option |= MLFILECASE;
#if (MLBUFFERCASE != MLBUFFER)
                else if(option & MLBUFFER)
                    option |= MLBUFFERCASE;
#endif
#endif
            }
            else
                cc = 0;
            
            /* Get and evaluate the next argument - this is the prompt */
            if(meGetString(NULL,0,0,prompt,meBUF_SIZE_MAX) <= 0)
                return abortm;
            if(flag & 0x01)
            {
                if(!(option & MLNOSTORE))
                {
                    /* Get and evaluate the next argument - this is the default value */
                    if(meGetString(NULL,0,0,buff,meBUF_SIZE_MAX) <= 0)
                        return abortm;
                    addHistory(option,buff,meFALSE);
                }
                defH = 1;
            }
            else
                defH = 0;
            if(flag & 0x02)
            {
                /* Get and evaluate the next argument - this is the ml line init value */
                if(meGetString(NULL,0,0,buff,meBUF_SIZE_MAX) <= 0)
                    return abortm;
                option |= MLNORESET;
            }
            if(flag & 0x04)
                /* set the first key to 'tab' to expand the input according to the completion list */
                meGetKeyFirst = ME_SPECIAL|SKEY_tab;
            if(flag & 0x08)
                option |= MLNOHIST|MLHIDEVAL;
            
            if((option & MLHISTLIST) || (flag & 0x100))
            {
                /* Get and evaluate the next argument - this is the completion list */
                if(meGetString(NULL,0,0,buff2,meBUF_SIZE_MAX) <= 0)
                    return abortm;
                if(option & MLHISTLIST)
                {
                    meUByte *numPtr, numHist;
                    meUByte **history;
                    
                    setupHistory(option,&numPtr,&history);
                    ss = buff2;
                    numHist = 0;
                    if((divChr = *ss++) != '\0')
                    {
                        while((tt = meStrchr(ss,divChr)) != NULL)
                        {
                            *tt++ = '\0';
                            if(numHist == meHISTORY_SIZE)
                                break;
                            history[numHist++] = ss;
                            ss = tt;
                        }
                    }
                    *numPtr = numHist;
                }
                else if(flag & 0x200)
                {
                    meBuffer *bp;
                    meLine *lp;
                    flag &= ~0x100;
                    if((bp = bfind(buff2,0)) == NULL)
                        return abortm ;
                    if(bp->lineCount > strListSize)
                    {
                        strListSize = bp->lineCount ;
                        if((strList = meRealloc(strList,strListSize*sizeof(meUByte *))) == NULL)
                        {
                            strListSize = 0;
                            return abortm;
                        }
                    }
                    lp = bp->baseLine;
                    mlgsStrListSize = 0;
                    while((lp=meLineGetNext(lp)) != bp->baseLine)
                        strList[mlgsStrListSize++] = lp->text ;
                    mlgsStrList = strList;
                }
                else
                {
                    ss = buff2;
                    mlgsStrListSize = 0;
                    if((divChr = *ss++) != '\0')
                    {
                        while((tt = meStrchr(ss,divChr)) != NULL)
                        {
                            *tt++ = '\0';
                            if(mlgsStrListSize == strListSize)
                            {
                                if((strList = meRealloc(strList,(strListSize+8)*sizeof(meUByte *))) == NULL)
                                {
                                    strListSize = 0;
                                    return abortm;
                                }
                                strListSize += 8;
                            }
                            strList[mlgsStrListSize++] = ss;
                            ss = tt;
                        }
                    }
                    mlgsStrList = strList;
                }
            }
            if(cc == 1)
            {
                if(!(option & MLNORESET))
                    getFilePath(frameCur->windowCur->buffer->fileName,buff);
                option &= MLHIDEVAL;
                option |= MLFILECASE|MLNORESET|MLMACNORT;
                
                if((ret = meGetStringFromUser(prompt,option,0,buff,meBUF_SIZE_MAX)) > 0)
                    fileNameCorrect(buff,evalResult,NULL);
            }
            else
            {
                /* Note that buffer result cannot be used directly as in the process
                 * of updating the screen the result buffer can be used by meItoa
                 * so use a temp buffer and copy across. note that inputFileName
                 * above doesn't suffer from this as the function uses a temp buffer
                 */
                ret = meGetStringFromUser(prompt,option,defH,buff,meBUF_SIZE_MAX) ;
                meStrncpy(evalResult,buff,meBUF_SIZE_MAX) ;
                evalResult[meBUF_SIZE_MAX-1] = '\0' ;
            }
            if(ret >= 0)
                return evalResult ;
        }
        else if(tkn[2] == 'c')
        {
            meUByte *ss, *helpStr ;
            int ret ;
            
            if((ret=tkn[3]) != '\0')
                ret -= '0' ;
            
            /* Get and evaluate the next argument - this is the prompt */
            if(meGetString(NULL,0,0,prompt,meBUF_SIZE_MAX) <= 0)
                return abortm ;
            if(ret & 0x01)
            {
                /* Get and evaluate the next argument - this is valid
                 * values list */
                if(meGetString(NULL,0,0,buff,meBUF_SIZE_MAX) <= 0)
                    return abortm ;
                ss = buff ;
            }
            else
                ss = NULL ;
            if(ret & 0x04)
            {
                /* Get and evaluate the next argument - this is the help string */
                if(meGetString(NULL,0,0,buff2,meBUF_SIZE_MAX) <= 0)
                    return abortm ;
                helpStr = buff2 ;
            }
            else
                helpStr = NULL ;
            ret = (ret & 0x02) ? mlCR_QUOTE_CHAR:0 ;
            
            clexec = meFALSE ;
            ret = mlCharReply(prompt,ret,ss,helpStr) ;
            clexec = meTRUE ;
            if(ret >= 0)
            {
                evalResult[0] = ret ;
                evalResult[1] = 0 ;
                return evalResult ;
            }
        }
        return abortm ;
        
    case 'p':
        /* parent command name */
        if(meRegCurr->prev->commandName == NULL)
            return (meUByte *) "" ;
        return meRegCurr->prev->commandName ;
        
#if MEOPT_EXTENDED
    case 's':
        if(isDigit(tkn[2]))
        {
            int ii ;
            
            if(srchLastState != meTRUE)
                return abortm ;
            if((ii=tkn[2]-'0') == 0)
                return srchLastMatch ;
#if MEOPT_MAGIC
            if(srchLastMagic && (ii <= mereRegexGroupNo()))
            {
                int jj ;
                if((jj=mereRegexGroupStart(ii)) >= 0)
                {
                    ii=mereRegexGroupEnd(ii) - jj ;
                    if(ii >= meBUF_SIZE_MAX)
                        ii = meBUF_SIZE_MAX-1 ;
                    memcpy(evalResult,srchLastMatch+jj,ii) ;
                }
                else
                    ii = 0 ;
                evalResult[ii] = '\0';
                return evalResult ;
            }
#endif
            return abortm ;
        }
        break ;
#endif
        
    case 'w':
        if(alarmState & meALARM_VARIABLE)
            return tkn ;
        if(tkn[2] == 'l')
        {
            register int blen ;
            meUByte *ss, *dd ;
            /* Current buffer line fetch */
            blen = frameCur->windowCur->dotLine->length;
            if (blen >= meBUF_SIZE_MAX)
                blen = meBUF_SIZE_MAX-1 ;
            ss = frameCur->windowCur->dotLine->text ;
            dd = evalResult ;
            while(--blen >= 0)
                *dd++ = *ss++ ;
            *dd = '\0' ;
            return evalResult ;
        }
        else
        {
            /* Current Buffer character fetch */
            evalResult[0] = meWindowGetChar(frameCur->windowCur) ;
            evalResult[1] = '\0';
            return evalResult ;
        }
        break ;
        
    case 'y':
        {
            meUByte *ss, *dd, cc ;
            meKillNode *killp;
            meKill *kl ;
            int ii ;
            
            if(alarmState & meALARM_VARIABLE)
                return tkn ;
            
            if(isDigit(tkn[2]))
            {
                /* Don't use the X or windows clipboard in this case */
                ii = meAtoi(tkn+2) ;
                kl = klhead ;
                while((kl != NULL) && (--ii >= 0))
                    kl = kl->next ;
            }
            else
            {
#ifdef _CLIPBRD
                TTgetClipboard(0);
#endif
                kl = klhead ;
            }
            if(kl == (meKill*) NULL)
                return abortm ;
            dd = evalResult ;
            killp = kl->kill ;
            ii = meBUF_SIZE_MAX-1 ;
            while(ii && (killp != NULL))
            {
                ss = killp->data ;
                while(((cc=*ss++) != '\0') && (--ii >= 0))
                    *dd++ = cc ;
                killp = killp->next;
            }
            *dd = '\0' ;
            return evalResult ;
        }
        
    default:
        if(isDigit(tkn[1]))
        {
            int index ;
            meUByte *ss ;
            
            if((index = meAtoi(tkn+1)) == 0)
            {
                ss = meRegCurr->commandName ;
                if(ss == NULL)
                    return (meUByte *) "" ;
            }
            else if((ss = getMacroArg(index)) == NULL)
                return abortm ;
            return ss ;
        }
    }
    mlwrite(MWABORT|MWWAIT,(meUByte *)"[Unknown argument %s]",tkn);
    return abortm ;
}

meUByte *
getval(meUByte *tkn)   /* find the value of a token */
{
    meUByte cc=*tkn;
#if MEOPT_BYTECOMP
    if(cc & BCLEAD)
    {
        if(cc == BCFUN)
            return gtfun((tkn[1])-'0',NULL);
        return abortm ;
    }
#endif
    switch (getMacroType(cc)) 
    {
    case TKARG:
        return gtarg(tkn) ;
        
    case TKREG:
        if(alarmState & meALARM_VARIABLE)
            return tkn;
        {
            meRegister *rp ;
            meUByte cc ;
            
            if((cc=tkn[1]) == 'l')
                rp = meRegCurr ;
            else if(cc == 'p')
                rp = meRegCurr->prev ;
            else if(cc == 'g')
                rp = meRegHead ;
            else
                break;
            cc = tkn[2] - '0' ;
            if(cc < meREGISTER_MAX)
                return rp->val[cc] ;
            return abortm ;
        }
        
#if MEOPT_EXTENDED
    case TKVAR:
        if(alarmState & meALARM_VARIABLE)
            return tkn ;
        return getUsrVar(tkn+1) ;
#endif
    case TKENV:
        if(alarmState & meALARM_VARIABLE)
            return tkn;
        return gtenv(tkn+1);

    case TKFUN:
        return gtfun(-1,tkn+1);
    
    case TKSTR:
        tkn++ ;
    case TKCMD:
    case TKLIT:
        return tkn ;
        
#if MEOPT_EXTENDED
    case TKLVR:
        if(alarmState & meALARM_VARIABLE)
            return tkn ;
        {
            meUByte *ss, *tt ;
            meBuffer *bp ;
            tt = tkn+1 ;
            if((ss=meStrrchr(tt,':')) != NULL)
            {
                *ss = '\0' ;
                bp = bfind(tt,0) ;
                *ss++ = ':' ;
                if(bp == NULL)
                    /* not a buffer - abort */
                    return errorm;
                tt = ss;
            }
            else
                bp = frameCur->windowCur->buffer;
            return getUsrLclCmdVar(tt,bp->varList);
        }
        
    case TKCVR:
        if(alarmState & meALARM_VARIABLE)
            return tkn;
        {
            register meUByte *ss, *tt ;
            tt = tkn+1 ;
            if((ss=meStrrchr(tt,'.')) != NULL)
            {
                int ii;
                *ss = '\0';
                ii = decode_fncname(tt,1);
                *ss++ = '.';
                if(ii >= 0)
                    return getUsrLclCmdVar(ss,cmdTable[ii]->varList);
                /* not a command - error */
                return errorm;
            }
            if(meRegCurr->varList != NULL)
                return getUsrLclCmdVar(tt,*(meRegCurr->varList));
            return abortm;
        }
#endif
    }
    /* If here then it is either TKDIR, TKLBL or TKNUL. Doesn't matter which
     * cause something has gone wrong and we return abortm
     */
    return abortm ;
}

static meUByte *
queryMode(meUByte *name, meMode mode)
{
    int ii ;
    
    for (ii=0; ii < MDNUMMODES ; ii++) 
        if(meStrcmp(name,modeName[ii]) == 0) 
            return meLtoa(meModeTest(mode,ii));
    return abortm;
}

#if (defined _DOS) || (defined _WIN32)
meInt
meGetDayOfYear(meInt year, meInt month, meInt day)
{
    static int dom[11]={31,28,31,30,31,30,31,31,30,31,30};
    
    if(month > 2)
    {
        if(((year & 0x03) == 0) &&
           (((year % 100) != 0) || ((year % 400) == 0)))
            day++;
    }
    month--;
    while(--month >= 0)
        day += dom[month];
    return day;
}
#endif

meUByte *
gtfun(register int fnum, meUByte *fname)  /* evaluate a function given name of function */
{
#if MEOPT_EXTENDED
    meRegister *regs ;     /* pointer to relevant regs if setting var */
#endif
    meUByte arg1[meBUF_SIZE_MAX];      /* value of first argument */
    meUByte arg2[meBUF_SIZE_MAX];      /* value of second argument */
    meUByte arg3[meBUF_SIZE_MAX];      /* value of third argument */
    meUByte *varVal;
    int ii;
    if(fnum < 0)
    {
        if(((ii = fname[0] - 'a') >= 0) && (ii < 26) && ((ii = funcOffst[ii]) != 0))
        {
            int fn, fnam = (((int) fname[0]) << 16) | (((int) fname[1]) << 8) | ((int) fname[2]) ;
            if((fn=funcHashs[ii]) == fnam)
                fnum = ii;
            else if(fn < fnam)
            {
                while((fn=funcHashs[++ii]) < fnam)
                    ;
                if(fn == fnam)
                    fnum = ii;
            }
            else
            {
                while((fn=funcHashs[--ii]) > fnam)
                    ;
                if(fn == fnam)
                    fnum = ii;
            }
        }
        if(fnum < 0)
        {
            mlwrite(MWABORT|MWWAIT,(meUByte *)"[Unknown function &%s]",fname);
            return abortm;
        }
    }
#if MEOPT_EXTENDED
    if((fnum == UFCBIND) || (fnum == UFNBIND))
    {
        meUInt arg ;
        
        if((ii = decode_key(meGetKey(meGETKEY_SILENT),&arg)) < 0)
            return errorm ;
        if(fnum == UFCBIND)
            meStrcpy(evalResult,getCommandName(ii)) ;
        else if(arg == 0)
            evalResult[0] = '\0' ;
        else
            return meItoa((int) (arg + 0x80000000)) ;
        return evalResult ;
    }
#endif
    /* retrieve the arguments */
    {
        meUByte alarmStateStr=alarmState;
        
#if MEOPT_EXTENDED
        if(funcTypes[fnum] & FUN_SETVAR)
        {
            /* horrid global variable, see notes at definition */
            gmaLocalRegPtr = meRegCurr;
            alarmState |= meALARM_VARIABLE;
            ii = macarg(arg1);
            alarmState &= ~meALARM_VARIABLE;
            regs = gmaLocalRegPtr;
            if((ii > 0) && (funcTypes[fnum] & FUN_GETVAR) &&
               ((varVal = getval(arg1)) == abortm))
                ii = meFALSE ;
        }
        else
#endif
        {
            alarmState &= ~meALARM_VARIABLE ;
            ii = macarg(arg1) ;
        }
        if((ii <= 0) ||
           ((funcTypes[fnum] & FUN_ARG2) &&
            ((macarg(arg2) <= 0) ||
             ((funcTypes[fnum] & FUN_ARG3) && (macarg(arg3) <= 0)))))
        {
            if(meRegCurr->commandName != NULL)
                sprintf((char *) arg3," in macro %s",meRegCurr->commandName);
            else
                arg3[0] = '\0';
            mlwrite(MWABORT|MWWAIT,(meUByte *)"[Failed to get argument for function &%s%s]",funcNames[fnum],arg3);
            return abortm ;
        }
        alarmState = alarmStateStr ;
    }
    
    /* and now evaluate it! */
    switch(fnum)
    {
    case UFIND:
        /* one gotcha here is that if we are getting a variable name
         * getval can return arg1! in this case we must copy arg1 to
         * evalResult */
        if((varVal=getval(arg1)) != arg1)
            return varVal ;
        meStrcpy (evalResult, arg1);
        return evalResult;
#if MEOPT_EXTENDED
    case UFEXIST:
        switch (getMacroTypeS(arg1))
        {
        case TKLVR:
        case TKCVR:
            if(((ii=meStrlen(arg1)) > 2) && (arg1[--ii] == arg1[0]))
            {
                /* simple buffer exists */
                arg1[ii] = '\0';
                if(arg1[0] == ':')
                    return (meLtoa(bfind(arg1+1,0) != NULL));
                return (meLtoa(decode_fncname(arg1+1,1) >= 0));
            }
        case TKARG:
        case TKREG:
        case TKVAR:
        case TKENV:
            /* all variable types */
            varVal = getval(arg1);
            return (meLtoa((varVal != abortm) && (varVal != errorm)));
            
        case TKCMD:
            return (meLtoa(decode_fncname(arg1,1) >= 0)) ;
            
        default:
            /* If here then it is either TKARG, TKFUN, TKDIR, TKSTR, TKLIT, TKLBL or TKNUL.
             * Doesn't matter which cause &exist doesn't work on these, return NO
             */
            return meLtoa(0) ;
        }
    case UFKBIND:
        {
            meUInt narg ;
            int ii, idx ;
            meUShort code=ME_INVALID_KEY ;
            
            if((idx = decode_fncname(arg2,0)) < 0)
                return abortm ;
            if(arg1[0] == '\0')
                narg = 0 ;
            else
                narg = (meUInt) (0x80000000+meAtoi(arg1)) ;
#if MEOPT_LOCALBIND
            {
                register meBuffer *cbp=frameCur->windowCur->buffer;
                for(ii=0 ; ii<cbp->bindCount ; ii++)
                {
                    if((cbp->bindList[ii].index == idx) &&
                       (cbp->bindList[ii].arg == narg))
                    {
                        code = cbp->bindList[ii].code ;
                        break ;
                    }
                }
            }
            if(code == ME_INVALID_KEY)
#endif
            {
                for(ii=0 ; ii<keyTableSize ; ii++)
                {
                    if((keytab[ii].index == idx) && (keytab[ii].arg == narg))
                    {
                        code = keytab[ii].code ;
                        break ;
                    }
                }
            }
            if(code != ME_INVALID_KEY)
                meGetStringFromKey(code,evalResult) ;
            else
                evalResult[0] = '\0' ;
            return evalResult ;
        }
#endif
    case UFABS:        return meItoa(abs((int) meAtoi(arg1)));
    case UFADD:        return meItoa(meAtoi(arg1) + meAtoi(arg2));
    case UFSUB:        return meItoa(meAtoi(arg1) - meAtoi(arg2));
    case UFMUL:        return meItoa(meAtoi(arg1) * meAtoi(arg2));
    case UFDIV:
    case UFMOD:
        {
            int ii, jj ;
            
            /* Check for divide/mod by zero, this causes an exception if
             * encountered so we must abort if detected. */
            if((jj = meAtoi(arg2)) == 0)
            {
                mlwrite(MWABORT|MWWAIT,(meUByte *) 
                        ((fnum == UFDIV) ? "[Divide by zero &div]":"[Modulus of zero &mod]")) ;
                return abortm ;
            }
            ii = meAtoi(arg1) ;
            if(fnum == UFDIV)
                ii /= jj ;
            else
                ii %= jj ;
            return meItoa(ii) ;
        }
    case UFNEG:        return meItoa(0-meAtoi(arg1));
    case UFOPT:
        arg1[3] = '\0' ;
        return meLtoa(meStrstr(meOptionList,arg1) != NULL) ;
        
#if MEOPT_EXTENDED
    case UFATOI:       return meItoa(arg1[0]) ;
    case UFITOA:
        {
            int ii = meAtoi(arg1) ;
            evalResult[0] = (meUByte) ii ;
            if(ii == meCHAR_LEADER)
            {
                evalResult[1] = meCHAR_TRAIL_LEADER ;
                evalResult[2] = '\0' ;
            }
            else
                evalResult[1] = '\0' ;
            return evalResult ;
        }
    case UFFABS:        return meFtoa(fabs(meAtof(arg1)));
    case UFFADD:        return meFtoa(meAtof(arg1) + meAtof(arg2));
    case UFFDIV:        return meFtoa(meAtof(arg1) / meAtof(arg2));
    case UFFMUL:        return meFtoa(meAtof(arg1) * meAtof(arg2));
    case UFFSQRT:       return meFtoa(sqrt(meAtof(arg1)));
    case UFFSUB:        return meFtoa(meAtof(arg1) - meAtof(arg2));
#endif
    case UFCAT:
        {
            meUByte *dd, *ss ;
            int ii = meBUF_SIZE_MAX-1 ;
            
            /* first string can be copied, second we must check the left */
            dd = evalResult ;
            ss = arg1 ;
            while((*dd++ = *ss++))
                ii-- ;
            dd-- ;
            ss = arg2 ;
            do {
                if(--ii <= 0)
                {
                    *dd = '\0' ;
                    break ;
                }
            } while((*dd++ = *ss++)) ;
            return evalResult ;
        }
    case UFLEFT:
        {
            meUByte *dd, *ss, cc ;
            int ii=meAtoi(arg2) ;
            ss = arg1 ;
            if(ii < 0)
                ii = meStrlen(ss) + ii ;
            dd = evalResult ;
            while(--ii >= 0)
            {
                if((cc=*ss++) == meCHAR_LEADER)
                {
                    *dd++ = cc ;
                    cc = *ss++ ;
                }
                if(cc == '\0')
                    break ;
                *dd++ = cc ;
            }
            *dd = '\0' ;
            return evalResult ;
        }
    case UFRIGHT:
        {
            meUByte *dd, *ss, cc ;
            int ii=meAtoi(arg2) ;
            ss = arg1 ;
            if(ii < 0)
                ii = meStrlen(ss) + ii ;
            while(--ii >= 0)
            {
                if((cc=*ss++) == meCHAR_LEADER)
                    cc = *ss++ ;
                if(cc == '\0')
                {
                    ss-- ;
                    break ;
                }
            }
            dd = evalResult ;
            while((*dd++ = *ss++) != '\0')
                ;
            return evalResult ;
        }
    case UFMID:
        {
            meUByte *dd, *ss, cc ;
            int ii, ll ;
            ss = arg1 ;
            
            ll = meAtoi(arg3) ;
            ii = meAtoi(arg2) ;
            if(ii < 0)
                ii = meStrlen(ss) + ii ;
            if(ii > 0)
            {
                do
                {
                    if((cc=*ss++) == meCHAR_LEADER)
                        cc = *ss++ ;
                    if(cc == '\0')
                    {
                        ss-- ;
                        break ;
                    }
                } while(--ii > 0) ;
            }
            else if(ll >= 0)
                ll += ii ;
            if(ll < 0)
                ll = meStrlen(ss) + ll ;
            dd = evalResult ;
            while(--ll >= 0)
            {
                if((cc=*ss++) == meCHAR_LEADER)
                {
                    *dd++ = cc ;
                    cc = *ss++ ;
                }
                if(cc == '\0')
                    break ;
                *dd++ = cc ;
            }
            *dd = '\0' ;
            return evalResult ;
        }
    case UFLEN:
        {
            meUByte *ss, cc ;
            int ii=0 ;
            ss = arg1 ;
            for(;;)
            {
                if((cc=*ss++) == meCHAR_LEADER)
                    cc=*ss++ ;
                if(cc == '\0')
                    break ;
                ii++ ;
            }
            return meItoa(ii) ;
        }
#if MEOPT_EXTENDED
    case UFSLOWER:
        {
            meUByte cc, *dd, *ss ;
            dd = evalResult ;
            ss = arg1 ;
            do {
                cc = *ss++ ;
                *dd++ = toLower(cc) ;
            } while(cc) ;
            return evalResult ;
        }
    case UFSUPPER:
        {
            meUByte cc, *dd, *ss ;
            dd = evalResult ;
            ss = arg1 ;
            do {
                cc = *ss++ ;
                *dd++ = toUpper(cc) ;
            } while(cc) ;
            return evalResult ;
        }
    case UFSIN:
    case UFRSIN:
    case UFISIN:
    case UFRISIN:
        {
            meIFuncSSI cmpIFunc ;
            meUByte cc, *ss=arg2, *lss=NULL ;
            int len, off=0 ;
            len = meStrlen(arg1) ;
            if((fnum == UFSIN) || (fnum == UFRSIN))
                cmpIFunc = (meIFuncSSI) strncmp ;
            else
                cmpIFunc = meStrnicmp ;
            
            do
            {
                if(!cmpIFunc(arg1,ss,len))
                {
                    lss = ss ;
                    if((fnum == UFSIN) || (fnum == UFISIN))
                        break ;
                }
                if((cc=*ss++) == meCHAR_LEADER)
                {
                    cc = *ss++ ;
                    off++ ;
                }
            } while(cc != '\0') ;
            
            if(lss != NULL)
                return meItoa(lss-arg2-off+1);
            return meLtoa(0) ;
        }
    case UFREP:
    case UFIREP:
        {
            meIFuncSSI cmpIFunc ;
            meUByte cc, *ss=arg1 ;
            int mlen, dlen=0, rlen, ii ;
            mlen = meStrlen(arg2) ;
            rlen = meStrlen(arg3) ;
            if(fnum == UFREP)
                cmpIFunc = (meIFuncSSI) strncmp ;
            else
                cmpIFunc = meStrnicmp ;
            
            for(;;)
            {
                ii = cmpIFunc(arg2,ss,mlen) ;
                if(!ii)
                {
                    if((dlen+rlen) >= meBUF_SIZE_MAX)
                        break ;
                    meStrcpy(evalResult+dlen,arg3) ;
                    dlen += rlen ;
                    ss += mlen ;
                }
                if((cc=*ss) == '\0')
                    break ;
                if(ii || (mlen == 0))
                {
                    if(dlen >= meBUF_SIZE_MAX-2)
                        break ;
                    ss++ ;
                    if(cc == meCHAR_LEADER)
                    {
                        evalResult[dlen++] = meCHAR_LEADER ;
                        cc = *ss++ ;
                    }
                    evalResult[dlen++] = cc ;
                }
            }
            evalResult[dlen] = '\0' ;
            return evalResult ;
        }
    case UFXREP:
    case UFXIREP:
        {
            meUByte cc, *rr ;
            int slen, soff=0, mlen, dlen=0, ii ;
            
            if(meRegexComp(&meRegexStrCmp,arg2,(fnum == UFXREP) ? 0:meREGEX_ICASE) != meREGEX_OKAY)
                return abortm ;
            
            slen = meStrlen(arg1) ;
            for(;;)
            {
                ii = meRegexMatch(&meRegexStrCmp,arg1,slen,soff,slen,(meREGEX_BEGBUFF|meREGEX_ENDBUFF)) ;
                if(ii)
                {
                    mlen = meRegexStrCmp.group[0].start - soff ;
                    if(dlen+mlen >= meBUF_SIZE_MAX)
                        break ;
                    meStrncpy(evalResult+dlen,arg1+soff,mlen) ;
                    dlen += mlen ;
                    rr = arg3 ;
                    while((cc=*rr++) != '\0')
                    {
                        if(dlen >= meBUF_SIZE_MAX-1)
                            break ;
                        if((cc == '\\') && (*rr != '\0'))
                        {
                            int changeCase=0 ;
                            cc = *rr++ ;
                            if(((cc == 'l') || (cc == 'u') || (cc == 'c')) && (*rr != '\0'))
                            {
                                changeCase = (cc == 'l') ? -1:cc ;
                                cc = *rr++ ;
                            }
                            if((cc == '&') || ((cc >= '0') && (cc <= '9') && ((((int) cc) - '0') <= meRegexStrCmp.groupNo)))
                            {
                                cc = (cc == '&') ? 0:(cc-'0') ;
                                /* if start offset is < 0, it was a ? auto repeat which was not found,
                                   therefore replace str == "" */ 
                                if((soff=meRegexStrCmp.group[cc].start) >= 0)
                                {
                                    mlen = meRegexStrCmp.group[cc].end - soff ;
                                    if(dlen+mlen >= meBUF_SIZE_MAX)
                                        break ;
                                    if(cc)
                                        soff += meRegexStrCmp.group[0].start ;
                                    if(changeCase)
                                    {
                                        while(--mlen >= 0)
                                        {
                                            cc = arg1[soff++] ;
                                            if(changeCase > 0)
                                            {
                                                evalResult[dlen++] = toUpper(cc) ;
                                                if(changeCase == 'c')
                                                    changeCase = -1 ;
                                            }
                                            else
                                                evalResult[dlen++] = toLower(cc) ;
                                        } 
                                    }
                                    else
                                    {
                                        meStrncpy(evalResult+dlen,arg1+soff,mlen) ;
                                        dlen += mlen ;
                                    }
                                }
                            }
                            else
                            {
                                if(cc == 'n')
                                    cc = '\n' ;
                                else if(cc == 'r')
                                    cc = '\r' ;
                                else if(cc == 't')
                                    cc = '\t' ;
                                evalResult[dlen++] = cc ;
                            }
                        }
                        else
                            evalResult[dlen++] = cc ;
                    }
                    mlen = meRegexStrCmp.group[0].end - meRegexStrCmp.group[0].start ;
                    soff = meRegexStrCmp.group[0].end ;
                }
                if((cc=arg1[soff]) == '\0')
                    break ;
                if(!ii || (mlen == 0))
                {
                    if(dlen >= meBUF_SIZE_MAX-2)
                        break ;
                    soff++ ;
                    if(cc == meCHAR_LEADER)
                    {
                        evalResult[dlen++] = meCHAR_LEADER ;
                        cc = arg1[soff++] ;
                    }
                    evalResult[dlen++] = cc ;
                }
            }
            evalResult[dlen] = '\0' ;
            return evalResult ;
        }
    case UFNBMODE:
        {
            meBuffer *bp ;
            if((bp = bfind(arg1,0)) == NULL)
                return abortm ;
            return queryMode(arg2,bp->mode) ;
        }
    case UFINWORD:     return(meLtoa(isWord(arg1[0])));
#endif
    case UFBMODE:      return queryMode(arg1,frameCur->windowCur->buffer->mode);
    case UFGMODE:      return queryMode(arg1,globMode);
    case UFBAND:       return meItoa(meAtoi(arg1) & meAtoi(arg2));
    case UFBNOT:       return meItoa(~meAtoi(arg1));
    case UFBOR:        return meItoa(meAtoi(arg1) |  meAtoi(arg2));
    case UFBXOR:       return meItoa(meAtoi(arg1) ^  meAtoi(arg2));
    case UFNOT:        return meLtoa(meAtol(arg1) == meFALSE);
    case UFAND:        return meLtoa(meAtol(arg1) && meAtol(arg2));
    case UFOR:         return meLtoa(meAtol(arg1) || meAtol(arg2));
    case UFEQUAL:      return meLtoa(meAtoi(arg1) == meAtoi(arg2));
    case UFLESS:       return meLtoa(meAtoi(arg1) <  meAtoi(arg2));
    case UFGREATER:    return meLtoa(meAtoi(arg1) >  meAtoi(arg2));
    case UFSEQUAL:     return meLtoa(meStrcmp(arg1,arg2) == 0);
    case UFSLESS:      return meLtoa(meStrcmp(arg1,arg2) < 0);
    case UFSGREAT:     return meLtoa(meStrcmp(arg1,arg2) > 0);
    case UFISEQUAL:    return meLtoa(meStricmp(arg1,arg2) == 0);
#if MEOPT_EXTENDED
    case UFXSEQ:       return meLtoa(regexStrCmp(arg1,arg2,meRSTRCMP_WHOLE|meRSTRCMP_USEMAIN) == 1);
    case UFXISEQ:      return meLtoa(regexStrCmp(arg1,arg2,meRSTRCMP_ICASE|meRSTRCMP_WHOLE|meRSTRCMP_USEMAIN) == 1);
    case UFLDEL:
        {
            int  index=meAtoi(arg2);
            meUByte cc, *s1, *s2=arg1;
            if((cc = *s2) == '\0')
                return abortm;
            if(index > 0)
            {
                do {
                    s1 = s2+1 ;
                    if((s2 = meStrchr(s1,cc)) == NULL)
                        break;
                } while(--index > 0);
            }
            else
                s2 = NULL;
            if(s2 == NULL)
                meStrcpy(evalResult,arg1);
            else
            {
                index = (int) (s1 - arg1);
                meStrncpy(evalResult,arg1,index);
                meStrcpy(evalResult+index,s2+1);
            }
            return evalResult;
        }
    case UFLFIND:
        {
            int  index;
            meUByte cc, *s1, *s2;
            s2 = arg1;
            if((cc = *s2) == '\0')
                return abortm;
            for(index=1 ; ; index++)
            {
                s1 = s2+1 ;
                if((s2 = meStrchr(s1,cc)) == NULL)
                    return meLtoa(0) ;
                *s2 = '\0' ;
                if(!meStrcmp(s1,arg2))
                    return meItoa(index) ;
            }
        }
    case UFLGET:
        {
            int  index;
            meUByte cc, *s1, *s2;
            s2 = arg1;
            if((cc = *s2) == '\0')
                return abortm;
            if((index=meAtoi(arg2)) <= 0)
                return emptym;
            do {
                s1 = s2+1 ;
                if((s2 = meStrchr(s1,cc)) == NULL)
                    return emptym;
            } while(--index > 0);
            index = (int) (s2 - s1);
            meStrncpy(evalResult,s1,index);
            evalResult[index] = '\0';
            return evalResult;
        }
    case UFLLEN:
        {
            int ll=0;
            meUByte cc, *s1;
            s1 = arg1;
            if((cc=*s1) == '\0')
                return abortm;
            while((s1 = meStrchr(s1+1,cc)) != NULL)
                ll++;
            return meItoa(ll);
        }
    case UFLINS:
    case UFLSET:
        {
            int  index, ii;
            meUByte cc, *s1, *s2;
            s2 = arg1;
            if((cc = *s2) == '\0')
                return abortm;
            if((index=meAtoi(arg2)) == 0)
                s1 = s2+1;
            else if(index > 0)
            {
                do {
                    s1 = s2+1 ;
                    if((s2 = meStrchr(s1,cc)) == NULL)
                    {
                        s2 = s1-1 ;
                        break ;
                    }
                } while(--index > 0) ;
            }
            else if((fnum == UFLINS) && (index < -1))
            {
                do
                {
                    s1 = s2+1 ;
                    if((s2 = meStrchr(s1,cc)) == NULL)
                    {
                        s2 = s1-1 ;
                        break ;
                    }
                    *s2 = '\0' ;
                    if(index == -2)
                        ii = (meStrcmp(s1,arg3) < 0);
                    else
                        ii = (meStricmp(s1,arg3) < 0);
                    *s2 = cc;
                } while(ii);
                s2 = s1-1;
            }
            else
            {
                ii = meStrlen(s2);
                s1 = s2+ii;
                s2 += ii-1;
            }
            index = (int) (s1 - arg1);
            memcpy(evalResult,arg1,index);
            ii = meStrlen(arg3);
            if(ii+index < meBUF_SIZE_MAX)
            {
                memcpy(evalResult+index,arg3,ii);
                index += ii;
                if(fnum == UFLINS)
                    s2 = s1-1;
                ii = meStrlen(s2);
                if(ii+index < meBUF_SIZE_MAX)
                {
                    memcpy(evalResult+index,s2,ii);
                    index += ii;
                }
            }
            evalResult[index] = '\0';
            return evalResult;
        }
    case UFFIND:
        {
            meUByte *el[10];
            int ec;
            if(arg2[0] == '\0')
                ec = 0;
            else if(arg2[0] == '|')
            {
                meUByte *e1=arg2+1,*e2;
                ec = 0;
                while((e2 = meStrchr(e1,'|')) != NULL)
                {
                    el[ec++] = e1;
                    *e2++ = '\0';
                    e1 = e2;
                }
            }
            else
            {
                ec = 1;
                el[0] = arg2;
            }
            if(!fileLookup(arg1,ec,el,meFL_CHECKPATH|meFL_USESRCHPATH,evalResult))
                return errorm ;
            return evalResult ;
        }
    case UFWHICH:
        {
            if(!executableLookup(arg1,evalResult))
                return errorm ;
            return evalResult ;
        }
    case UFCOND:
        {
            if(meAtol(arg1))
                meStrcpy(evalResult,arg2) ;
            else
                meStrcpy(evalResult,arg3) ;
            return evalResult ;
        }
    case UFTRIMB:
        {
            /* Trim both */
            meUByte *ss;
            meUByte cc;
            int ii;
            
            /* Trim left */
            for (ss = arg1; (((cc = *ss) != '\0') && isSpace(cc)) ; ss++)
                ;
            /* Trim right */
            ii = meStrlen (ss);
            while (ii > 0)
            {
                cc = ss[ii-1];
                if (isSpace(cc))
                    ii--;
                else
                    break;
            }
            meStrncpy(evalResult, ss, ii);
            evalResult[ii] = '\0';
            return evalResult;
        }
    case UFTRIML:
        {
            /* Trim left */
            meUByte *ss;
            meUByte cc;
            for (ss = arg1; (((cc = *ss) != '\0') && isSpace(cc)) ; ss++)
                ;
            meStrcpy(evalResult, ss);
            return evalResult;
        }
    case UFTRIMR:
        {
            /* Trim right */
            int ii;
            ii = meStrlen (arg1);
            while (ii > 0)
            {
                meUByte cc;
                cc = arg1[ii-1];
                if (isSpace(cc))
                    ii--;
                else
                    break;
            }
            meStrncpy (evalResult, arg1, ii);
            evalResult[ii] = '\0';
            return evalResult;
        }
    case UFUCINFO:
        {
            meUByte cc=arg1[0];
            if(cc == 0)
                meStrcpy(evalResult,"|3|0x000000||1||");
            else if(cc < 128)
                sprintf((char *) evalResult,"\xa0" "3\xa0" "0x%06x\xa0%c\xa0" "1\xa0%c\xa0",(int) cc,cc,cc);
            else if((ii = charToUnicode[cc-128]) == 0)
                sprintf((char *) evalResult,"|2|0x000000|%c|3|\xef\xbf\xbd|",cc);
            else
            {
                varVal = evalResult + sprintf((char *) evalResult,"|3|0x%06x|%c|",ii,cc);
                if(ii & 0x0f800)
                {
                    *varVal++ = '3';
                    *varVal++ = '|';
                    *varVal++ = 0xe0 | (ii >> 12);
                    *varVal++ = 0x80 | ((ii >> 6) & 0x3f);
                    *varVal++ = 0x80 | (ii & 0x3f);
                }
                else if(ii & 0x00780)
                {
                    *varVal++ = '2';
                    *varVal++ = '|';
                    *varVal++ = 0xc0 | (ii >> 6);
                    *varVal++ = 0x80 | (ii & 0x3f);
                }
                else
                {
                    *varVal++ = '1';
                    *varVal++ = '|';
                    *varVal++ = ii;
                }
                *varVal++ = '|';
                *varVal = '\0';
            }
            return evalResult;
        }
    case UFUFINFO:
        {
            meUByte cc=arg1[0], c1, bc, bm, ll=1;
            varVal = evalResult + 16;
            if(cc < 0x80)
            {
                if((ii = cc) != 0)
                    *varVal++ = cc;
                c1 = '\xa0';
                bc = 3;
            }
            else if(cc > 0xf4)
            {
                *varVal++ = cc;
                c1 = '|';
                bc = 0;
                ii = 0;
                cc = meCHAR_UNDEF;
            }
            else
            {
                *varVal++ = cc;
                bc = 0;
                ii = 0;
                bm = 0x40;
                while(cc & bm)
                {
                    c1 = arg1[ll++];
                    if((c1 & 0xc0) != 0x80)
                    {
                        ll--;
                        bc = 0;
                        break;
                    }
                    *varVal++ = c1;
                    ii = (ii << 6) | (c1 & 0x3f);
                    bm >>= 1;
                    bc += 6;
                }
                if(bc == 0)
                {
                    ii = 0;
                    cc = meCHAR_UNDEF;
                }
                else if(((ii |= (cc & (bm - 1)) << bc) < 256) && ((ii < 128) || (charToUnicode[ii-128] == ii)))
                {
                    cc = ii;
                    bc = 3;
                }
                else if(ii > 0x10ffff)
                {
                    bc = 0;
                    ii = 0;
                    cc = meCHAR_UNDEF;
                }
                else
                {
                    int jj = 127;
                    while((charToUnicode[jj] != ii) && (--jj >= 0))
                        ;
                    if(jj >= 0)
                    {
                        cc = jj+128;
                        bc = 3;
                    }
                    else
                    {
                        cc = meCHAR_UNDEF;
                        bc = 1;
                    }
                }
                c1 = '|';
            }
            *varVal++ = c1;
            *varVal = '\0';
            sprintf((char *) evalResult,"%c%c%c0x%06x%c%c%c%c",c1,'0'+bc,c1,ii,c1,cc,c1,'0'+ll);
            evalResult[15] = c1;
            if(cc == '\0')
            {
                varVal = evalResult + 12;
                while((*varVal = varVal[1]) != '\0')
                    varVal++;
            }
            return evalResult;
        }
    case UFUNINFO:
        {
            if((ii = meAtoi(arg1)) == 0)
                meStrcpy(evalResult,"|3|0x000000||1||");
            else if((ii < 0) || (ii > 0x10ffff))
                sprintf((char *) evalResult,"|0|0x000000|%c|3|\xef\xbf\xbd|",meCHAR_UNDEF);
            else if(ii < 128)
                sprintf((char *) evalResult,"\xa0" "3\xa0" "0x%06x\xa0%c\xa0" "1\xa0%c\xa0",ii,(meUByte) ii,(meUByte) ii);
            else
            {
                int jj = 127;
                while((charToUnicode[jj] != ii) && (--jj >= 0))
                    ;
                if(jj < 0)
                    varVal = evalResult + sprintf((char *) evalResult,"|1|0x%06x|%c|",ii,meCHAR_UNDEF);
                else
                    varVal = evalResult + sprintf((char *) evalResult,"|3|0x%06x|%c|",ii,(meUByte) jj+128);
                if(ii & 0x1f0000)
                {
                    *varVal++ = '4';
                    *varVal++ = '|';
                    *varVal++ = 0xf0 | (ii >> 18);
                    *varVal++ = 0x80 | ((ii >> 12) & 0x3f);
                    *varVal++ = 0x80 | ((ii >> 6) & 0x3f);
                    *varVal++ = 0x80 | (ii & 0x3f);
                }
                else if(ii & 0x00f800)
                {
                    *varVal++ = '3';
                    *varVal++ = '|';
                    *varVal++ = 0xe0 | (ii >> 12);
                    *varVal++ = 0x80 | ((ii >> 6) & 0x3f);
                    *varVal++ = 0x80 | (ii & 0x3f);
                }
                else if(ii & 0x000780)
                {
                    *varVal++ = '2';
                    *varVal++ = '|';
                    *varVal++ = 0xc0 | (ii >> 6);
                    *varVal++ = 0x80 | (ii & 0x3f);
                }
                else
                {
                    *varVal++ = '1';
                    *varVal++ = '|';
                    *varVal++ = ii;
                }
                *varVal++ = '|';
                *varVal = '\0';
            }
            return evalResult;
        }
    case UFSET:
        if(setVar(arg1,arg2,regs) <= 0)
            return abortm ;
        meStrcpy (evalResult, arg2);
        return evalResult;
    
    case UFDEC:
        varVal = meItoa(meAtoi(varVal) - meAtoi(arg2));
        if(setVar(arg1,varVal,regs) <= 0)
            return abortm;
        return varVal;
    
    case UFINC:
        varVal = meItoa(meAtoi(varVal) + meAtoi(arg2));
        if(setVar(arg1,varVal,regs) <= 0)
            return abortm;
        return varVal;
    
    case UFPDEC:
        /* cannot copy into evalResult here cos meItoa uses it */
        meStrcpy(arg3,varVal) ;
        varVal = meItoa(meAtoi(varVal) - meAtoi(arg2));
        if(setVar(arg1,varVal,regs) <= 0)
            return abortm;
        meStrcpy(evalResult,arg3);
        return evalResult;
    
    case UFPINC:
        /* cannot copy into evalResult here cos meItoa uses it */
        meStrcpy(arg3,varVal);
        varVal = meItoa(meAtoi(varVal) + meAtoi(arg2));
        if(setVar(arg1,varVal,regs) <= 0)
            return abortm;
        meStrcpy(evalResult,arg3) ;
        return evalResult;
    
    case UFREGISTRY:
#if MEOPT_REGISTRY
        {
            meRegNode *rgy;
            meUByte *p = arg2;
            /*
             * Read a value from the registry.
             * 
             * Arg1 is the path,
             * Arg2 is the subkey
             * Arg3 is the default value
             * 
             * If the node cannot be found then return the result.
             */
            if((rgy = regFind (NULL, arg1)) != NULL)
                p = regGetString (rgy, p) ;
            /* Set up the result string */
            if (p == NULL)
                evalResult [0] = '\0';
            else
                meStrcpy (evalResult, p);
            return evalResult;
        }
#else
        break;
#endif
    case UFSPRINT:                         /* Monamic function !! */
        {
            /*
             * Do a sprintf type of function. 
             * &sprintf "This is a %s %s." "arg1" "arg2"
             *
             * Note that we build the result in a buffer (arg3) on the stack
             * which allows us to effectivelly do recursive invocations 
             * of &sprintf.
             * 
             * Note that this is a monamic function, the first argument is the
             * format string. The rest of the arguments are dynamic and are
             * feteched as required.
             * 
             * Note - need to sort the '"' character. Not sure how macro process
             * it when specified in string.
             * 
             * Syntax defined as follows:-
             * 
             * sprintf "%s" arg
             * sprintf "%n" count arg
             * sprintf "%d" 10
             * 
             * e.g.
             *     sprintf "Foo [%s%s]" "a" "b"
             * generates:-
             *     "Foo [ab]"
             * 
             *     sprintf "Foo [%n%s]" 10 "a" "b"
             * generates:-
             *     "Foo [aaaaaaaaaab]"
             */
            
            meUByte *p;                            /* Temp char pointer */
            meUByte  c;                            /* Current char */
            meUByte *s = arg1;                     /* String pointer */
            int   count, nn, index=0 ;             /* Count of repeat */
            meUByte alarmStateStr=alarmState ;
            
            alarmState &= ~meALARM_VARIABLE ;
            
            while((c = *s++) != '\0') 
            {
                if(c == '%')                       /* Check for control */
                {
                    p = s-1 ;
                    count = 1 ;
get_flag:
                    switch((c = *s++))
                    {
                    case 'n':                   /* Replicate string */
                        if (macarg (arg2) <= 0) 
                            return abortm;
                        count *= meAtoi(arg2);
                    case 's':                   /* String */
                        if (macarg (arg2) <= 0)
                            return abortm;
                        nn = meStrlen(arg2) ;
                        while(--count >= 0)
                        {
                            if(index+nn >= meBUF_SIZE_MAX)
                                /* only copy amount we have space for */
                                nn = meBUF_SIZE_MAX - 1 - index ;
                            memcpy(arg3+index,arg2,nn) ;
                            index += nn ;
                        }
                        break;
                    case 'X':                   /* Hexadecimal */
                    case 'x':                   /* Hexadecimal */
                    case 'd':                   /* Decimal */
                    case 'o':                   /* Octal */
                        if (macarg (arg2) <= 0) 
                            return abortm;
                        nn = meAtoi(arg2) ;
                        c = *s ;
                        *s = '\0' ;
                        if(index < meBUF_SIZE_MAX-16)
                            /* Only do it if we have enough space */
                            index += sprintf((char *)arg3+index,(char *)p,nn) ;
                        *s = c ;
                        break;
                    case 'e':                   /* Float (double precision) */
                    case 'f':                   /* Float (double precision) */
                    case 'g':                   /* Float (double precision) */
                        {
                            double dd;
                            if (macarg (arg2) <= 0) 
                                return abortm;
                            dd = meAtof(arg2) ;
                            c = *s ;
                            *s = '\0' ;
                            if(index < meBUF_SIZE_MAX-16)
                                /* Only do it if we have enough space */
                                index += sprintf((char *)arg3+index,(char *)p,dd) ;
                            *s = c ;
                            break;
                        }
                    default:
                        if(isDigit(c) || (c == '.'))
                        {
                            count = c - '0' ;
                            while(((c = *s++) == '.') || isDigit(c)) 
                                count = (count*10) + (c-'0') ;
                            s-- ;
                            goto get_flag ;
                        }
                        else if(index >= meBUF_SIZE_MAX)
                            break ;
                        else
                            arg3[index++] = c;  /* Just a literal char - pass in */
                    }
                }
                else if(index >= meBUF_SIZE_MAX-1)
                    break ;
                else
                    arg3[index++] = c;          /* Just a literal char - pass in */
            }
            arg3[index] = '\0';                 /* make sure sting terminated */
            meStrcpy(evalResult,arg3) ;
            alarmState = alarmStateStr ;
            return evalResult ;
        }
    case UFSTAT:
        {
            meStat stats;
            int ftype;
            fileNameCorrect(arg2,arg3,NULL);
            ftype = getFileStats(arg3,gfsERRON_ILLEGAL_NAME|gfsERRON_BAD_FILE,
                                     (arg1[0] == 't') ? NULL:&stats,(arg1[0] == 't') ? NULL:evalResult);
            switch(arg1[0])
            {
            case 'a':
                /* absolute name - check for symbolic link */
                if(evalResult[0] == '\0')
                    meStrcpy(evalResult,arg3);
                /* if its a directory check there's an ending '/' */
                if(ftype & meIOTYPE_DIRECTORY)
                {
                    meUByte *ss=evalResult+meStrlen(evalResult);
                    if(ss[-1] != DIR_CHAR)
                    {
                        ss[0] = DIR_CHAR;
                        ss[1] = '\0';
                    }
                }
                return evalResult;
                
            case 'd':
                /* file date stamp to int */
                if(meFiletimeIsSet(stats.stmtime))
                {
#ifdef _WIN32
                    /* Convert the file time into standard unix epoch time */
                    ULARGE_INTEGER uli;
                    uli.LowPart = stats.stmtime.dwLowDateTime;
                    uli.HighPart = stats.stmtime.dwHighDateTime;
                    sprintf((char *)evalResult,"%llu",((uli.QuadPart / 10000000) - 11644473600));
#else
#ifdef _UNIX
                    sprintf((char *)evalResult,"%ld",stats.stmtime);
#else
                    sprintf((char *)evalResult,"%u",stats.stmtime);
#endif
#endif
                    return evalResult;
                }
                ftype = -1;
                break ;
                
            case 'i':
                {
                    /* file information in list form as follows:
                     *  |<url-type>|<file-type>|<link-file-type>|<attrib>|<size-upper>|<size-lower>|
                     * Note:
                     *   - this version differs from 't' as links have a file
                     *     type of 'L' and ftp files return the file type not 'F'
                     *   - The fourth value is the file attributes (-1 == not available)
                     *   - The size has an upper int and lower int value to handle
                     *     large files
                     *   - The file modification time will be the 7th value (n.y.i.)
                     */ 
                    meUByte v1, v2, v3='\0', v5=0, *dd, v7[32];
                    meUInt v51, v52;
                    int v4=-1;
                    
                    v7[0] = '\0';
                    if(ftype & meIOTYPE_HTTP)
                        v1 = v2 = 'H';
                    else if(ftype & (meIOTYPE_FTP|meIOTYPE_FTPE))
                    {
                        v1 = 'F';
                        if(ffFileOp(arg3,NULL,meRWFLAG_STAT|meRWFLAG_SILENT,-1) > 0)
                        {
                            v2 = evalResult[0];
                            dd = evalResult+1;
                            if(*dd != '|')
                            {
                                v5 = 1;
                                v51 = 0;
                                v52 = meAtoi(dd);
                                dd = meStrchr(dd,'|');
                            }
                            dd++ ;
                            if(*dd != '\0')
                            {
                                meStrncpy(v7,dd,15) ;
                                v7[15] = '\0' ;
                            }
                        }
                        else
                            v2 = 'N';
                    }
                    else
                    {
#ifdef MEOPT_TFS
                        v1 = (ftype & meIOTYPE_TFS) ? 'T':'L';
#else
                        v1 = 'L';
#endif
                        if(ftype & meIOTYPE_NOTEXIST)
                            v2 = 'X';
                        else
                        {
#ifdef _DOS
                            if((stats.stmtime & 0x0ffff) != 0x7fff)
                                sprintf((char *)v7,"%4d%2d%2d%2d%2d%2d",
                                        (int) ((stats.stmtime >> 25) & 0x007f)+1980,
                                        (int) ((stats.stmtime >> 21) & 0x000f),
                                        (int) ((stats.stmtime >> 16) & 0x001f),
                                        (int) ((stats.stmtime >> 11) & 0x001f),
                                        (int) ((stats.stmtime >>  5) & 0x003f),
                                        (int) ((stats.stmtime & 0x001f)  << 1));
#endif
#ifdef _WIN32
                            SYSTEMTIME tmp;
                            FILETIME ftmp;
                            
                            if(FileTimeToLocalFileTime(&stats.stmtime,&ftmp) && FileTimeToSystemTime(&ftmp,&tmp))
                                sprintf((char *)v7,"%4d%2d%2d%2d%2d%2d",
                                        tmp.wYear,tmp.wMonth,tmp.wDay,
                                        tmp.wHour,tmp.wMinute,tmp.wSecond);
#endif
#ifdef _UNIX
                            struct tm *tmp;            /* Pointer to time frame. */
                            if((tmp = localtime(&stats.stmtime)) != NULL)
                                sprintf((char *)v7, "%4d%2d%2d%2d%2d%2d",
                                        tmp->tm_year+1900,tmp->tm_mon+1,tmp->tm_mday,
                                        tmp->tm_hour,tmp->tm_min,tmp->tm_sec);
#endif
                            if(ftype & meIOTYPE_NASTY)
                                v2 = 'N';
                            else
                            {
                                v2 = (ftype & meIOTYPE_DIRECTORY) ? 'D':'R';
                                v4 = meFileGetAttributes(arg3);
                                v5 = 1;
                                v51 = stats.stsizeHigh;
                                v52 = stats.stsizeLow;
                            }
                        }
#ifdef _UNIX
                        if(evalResult[0] != '\0')
                        {
                            v3 = v2;
                            v2 = 'L';
                        }
#endif
                    }
                    dd = evalResult ;
                    *dd++ = '\b' ;
                    *dd++ = v1 ;
                    *dd++ = '\b' ;
                    *dd++ = v2 ;
                    *dd++ = '\b' ;
                    if(v3 != '\0')
                        *dd++ = v3 ;
                    *dd++ = '\b' ;
                    if(v4 >= 0) 
                        dd += sprintf((char *)dd,"%d",v4) ;
                    *dd++ = '\b' ;
                    if(v5)
                        dd += sprintf((char *)dd,"0x%x\b0x%x",v51,v52) ;
                    else
                        *dd++ = '\b' ;
                    *dd++ = '\b' ;
                    if(v7[0] != '\0')
                    {
                        meStrcpy(dd,v7) ;
                        dd += meStrlen(dd) ;
                    }
                    *dd++ = '\b' ;
                    *dd = '\0' ;
                    return evalResult;
                }
            case 'm':
                {
                    /* file modified date stamp in a useable form */
                    /* Format same as $time variable */
#ifdef _UNIX
                    struct tm *tmp;            /* Pointer to time frame. */
                    if ((tmp = localtime(&stats.stmtime)) != NULL)
                        sprintf((char *)evalResult, "%4d%3d%2d%2d%1d%2d%2d%2d  -",
                                tmp->tm_year+1900,tmp->tm_yday,tmp->tm_mon+1,tmp->tm_mday,
                                tmp->tm_wday,tmp->tm_hour,tmp->tm_min,tmp->tm_sec);
                    else
#endif
#ifdef _DOS
                        if((stats.stmtime & 0x0ffff) != 0x7fff)
                    {
                        meInt year, month, day, doy ;
                        
                        year = ((stats.stmtime >> 25) & 0x007f)+1980 ;
                        month = ((stats.stmtime >> 21) & 0x000f) ;
                        day = ((stats.stmtime >> 16) & 0x001f) ;
                        doy = meGetDayOfYear(year,month,day) ;
                        sprintf((char *)evalResult,"%4d%3d%2d%2d-%2d%2d%2d  -",
                                (int) year,(int) doy,(int) month,(int) day,
                                (int) ((stats.stmtime >> 11) & 0x001f),
                                (int) ((stats.stmtime >>  5) & 0x003f),
                                (int) ((stats.stmtime & 0x001f)  << 1)) ;
                    }
                    else
#endif
#ifdef _WIN32
                        SYSTEMTIME tmp;
                    FILETIME ftmp;
                    meInt doy ;
                    
                    if(FileTimeToLocalFileTime(&stats.stmtime,&ftmp) && FileTimeToSystemTime(&ftmp,&tmp))
                    {
                        doy = meGetDayOfYear(tmp.wYear,tmp.wMonth,tmp.wDay) ;
                        sprintf((char *)evalResult, "%4d%3d%2d%2d%1d%2d%2d%2d%3d",
                                tmp.wYear,(int) doy,tmp.wMonth,tmp.wDay,tmp.wDayOfWeek,tmp.wHour,
                                tmp.wMinute,tmp.wSecond,tmp.wMilliseconds) ;
                    }
                    else
#endif
                        sprintf((char *)evalResult, "   -  - - -- - - -  -") ;
                    return evalResult;
                }
                
            case 'r':
                /* File read permission */
                /* If its a nasty or it doesn't exist - no */
                if(ftype & (meIOTYPE_NASTY|meIOTYPE_NOTEXIST))
                    return meLtoa(0);
                /* If its a url then do we support url ? */
                if(ftype & (meIOTYPE_HTTP|meIOTYPE_FTP|meIOTYPE_FTPE))
#if MEOPT_SOCKET
                    return meLtoa(1);
#else
                    return meLtoa(0);
#endif
                /* If its a DOS/WIN directory - yes */
#if (defined _DOS) || (defined _WIN32)
                if(ftype & meIOTYPE_DIRECTORY)
                    return meLtoa(1);
#endif
                /* normal test */
                return meLtoa(meStatTestRead(stats));
                
            case 's':
                /* File size - return -1 if not a regular file */
                if((ftype & meIOTYPE_REGULAR) == 0)
                    ftype = -1 ;
                else if((stats.stsizeHigh > 0) || (stats.stsizeLow & 0x80000000))
                    ftype = 0x7fffffff ;
                else
                    ftype = stats.stsizeLow ;
                break ;
            case 't':
                if(ftype & meIOTYPE_NOTEXIST)
                    evalResult[0] = 'X';
                else if(ftype & (meIOTYPE_REGULAR|meIOTYPE_DIRECTORY))
                    evalResult[0] = (ftype & meIOTYPE_REGULAR) ? 'R':'D';
                else if(ftype & (meIOTYPE_HTTP|meIOTYPE_FTP|meIOTYPE_FTPE))
                    evalResult[0] = (ftype & meIOTYPE_HTTP) ? 'H':'F';
                else
                    evalResult[0] = 'N';
                evalResult[1] = '\0';
                return evalResult;
                
            case 'w':
                /* File write permission - If nasty or url - no */
                if(ftype & (meIOTYPE_NASTY|meIOTYPE_TFS|meIOTYPE_HTTP|meIOTYPE_FTP|meIOTYPE_FTPE))
                    return meLtoa(0);
                
                /* if it doesnt exist or its an DOS/WIN directory - yes */
#if (defined _DOS) || (defined _WIN32)
                if(ftype & (meIOTYPE_NOTEXIST|meIOTYPE_DIRECTORY))
#else
                if(ftype & meIOTYPE_NOTEXIST)
#endif
                    return meLtoa(1);
                return meLtoa(meStatTestWrite(stats));
                
            case 'x':
                /* File execute permission */
                /* If nasty or doesnt exist, or url then no */
                if(ftype & (meIOTYPE_NASTY|meIOTYPE_NOTEXIST|meIOTYPE_HTTP|meIOTYPE_FTP|meIOTYPE_FTPE))
                    return meLtoa(0);
#if (defined _DOS) || (defined _WIN32)
                /* If directory, simulate unix style execute flag and return yes */
                if(ftype & meIOTYPE_DIRECTORY)
                    return meLtoa(1);
                /* Must test for .exe, .com, .bat, .btm extensions etc */
                return meLtoa(meTestExecutable(arg3));
#else
                /* Test the unix execute flag */
                return meLtoa((((stats.stuid == meUid) && (stats.stmode & S_IXUSR)) ||
                               ((stats.stgid == meGid) && (stats.stmode & S_IXGRP)) ||
                               (stats.stmode & S_IXOTH)));
#endif
            default:
                return abortm;
            }                
            return meItoa(ftype);
        }
    case UFBSTAT:
        {
            int ret ;
            switch(arg1[0])
            {
            case 'o':
                ret = (bufferOutOfDate(frameCur->windowCur->buffer) == 2);
                break ;
            default:
                return abortm ;
            }                
            return meItoa(ret) ;
        }
#endif
    }
    return abortm ;
}

#if MEOPT_EXTENDED
/* numeric arg can overide prompted value */
int
unsetVariable(int f, int n)     /* Delete a variable */
{
    register meVariable *vptr, *vp;   /* User variable pointers */
    register int vnum;                /* ordinal number of var refrenced */
#if MEOPT_CMDHASH
    meUInt hash;
#endif
    meRegister *regs ;
    meUByte var[meSBUF_SIZE_MAX] ;                 /* name of variable to fetch */
    meUByte *vv ;
    
    /* First get the variable to set.. */
    /* set this flag to indicate that the variable name is required, NOT
     * its value */
    alarmState |= meALARM_VARIABLE ;
    /* horrid global variable, see notes at definition */
    gmaLocalRegPtr = meRegCurr ;
    vnum = meGetString((meUByte *)"Unset variable", MLVARBL, 0, var, meSBUF_SIZE_MAX) ;
    regs = gmaLocalRegPtr ;
    alarmState &= ~meALARM_VARIABLE ;
    if(vnum <= 0)
        return vnum;
    
    /* Check the legality and find the var */
    vv = var+1 ;
    vnum = getMacroTypeS(var); 
    if(vnum == TKVAR) 
        vptr = (meVariable *) &usrVarList;
    else if(vnum == TKLVR)
    {        
        meUByte *ss;
        meBuffer *bp;
        
        if((ss=meStrrchr(vv,':')) != NULL)
        {
            *ss = '\0' ;
            bp = bfind(vv,0) ;
            *ss++ = ':' ;
            if(bp == NULL)
                /* not a command - abort */
                return mlwrite(MWABORT,(meUByte *)"[No such variable]");
            vv = ss ;
        }
        else
            bp = frameCur->windowCur->buffer ;
        vptr = (meVariable *) &(bp->varList) ;
    }
    else if(vnum == TKCVR)
    {
        meUByte *ss ;
        if((ss=meStrrchr(vv,'.')) != NULL)
        {
            int idx ;
            
            *ss = '\0' ;
            idx = decode_fncname(var+1,0) ;
            *ss++ = '.' ;
            if(idx < 0)
                return mlwrite(MWABORT,(meUByte *)"[No such variable]");
            vptr = (meVariable *) &(cmdTable[idx]->varList) ;
            vv = ss ;
        }
        else if((vptr = (meVariable *) regs->varList) == NULL)
            return mlwrite(MWABORT,(meUByte *)"[No such variable]");
    }
    else
        return mlwrite(MWABORT,(meUByte *)"[User variable required]");
    
#if MEOPT_CMDHASH
    meStringHash(vv,hash);
    while(((vp = vptr->next) != NULL) && (vp->hash <= hash))
    {
        if(vp->hash == hash)
        {
            if((vnum = meStrcmp(vv,vp->name)) == 0)
            {
                /* found it, link out the variable and free it */
                vptr->next = vp->next;
                meNullFree(vp->value);
                meFree(vp);
                return meTRUE;
            }
            if(vnum < 0)
                break;
        }
        vptr = vp ;
        vp = vp->next ;
    }
#else
    while((vp = vptr->next) != NULL)
    {
        if((vnum = meStrcmp(vv,vp->name)) == 0)
        {
            /* found it, link out the variable and free it */
            vptr->next = vp->next;
            meNullFree(vp->value);
            meFree(vp);
            return meTRUE;
        }
        if(vnum < 0)
            break;
        vptr = vp ;
        vp = vp->next ;
    }
#endif
    /* If its not legal....bitch */
    return mlwrite(MWABORT|MWCLEXEC,(meUByte *)"[No such variable]") ;
}
#endif

int
setVariable(int f, int n)       /* set a variable */
{
    register int   status ;         /* status return */
    meUByte var[meSBUF_SIZE_MAX] ;            /* name of variable to fetch */
    meUByte value[meBUF_SIZE_MAX] ;           /* value to set variable to */
    meRegister *regs ;
    
    /* horrid global variable, see notes at definition */
    gmaLocalRegPtr = meRegCurr ;
    /* set this flag to indicate that the variable name is required, NOT
     * its value */
    alarmState |= meALARM_VARIABLE ;
    status = meGetString((meUByte *)"Set variable", MLVARBL, 0, var,meSBUF_SIZE_MAX) ;
    alarmState &= ~meALARM_VARIABLE ;
    regs = gmaLocalRegPtr ;
    if(status <= 0)
        return status ;
    /* get the value for that variable */
    if(f == meTRUE)
        meStrcpy(value, meItoa(n));
    else if((status = meGetString((meUByte *)"Value", MLFFZERO, 0, value,meBUF_SIZE_MAX)) <= 0)
        return status ;
    
    return setVar(var,value,regs) ;
}

int
descVariable(int f, int n)      /* describe a variable */
{
    meUByte  var[meSBUF_SIZE_MAX]; /* name of variable to fetch */
    meUByte *ss ;
    int ii ;
    
    /* first get the variable to describe */
    if((ii = meGetString((meUByte *)"Show variable",MLVARBL,0,var,meSBUF_SIZE_MAX)) <= 0)
        return ii ;
    if((((ii=getMacroTypeS(var)) != TKREG) && (ii != TKVAR) && (ii != TKENV) && (ii != TKLVR) && (ii != TKCVR)) ||
       ((ss = getval(var)) == NULL))
        return mlwrite(MWABORT,(meUByte *)"Unknown variable type") ;
    mlwrite(0,(meUByte *)"Current setting is \"%s\"", ss) ;
    return meTRUE ;
} 

#if MEOPT_EXTENDED
/*
 * showVariable
 * Format a variable for display. This is a helper function for 
 * "listVariables()" used to format and display each of the values.
 * 
 * Arguments:-
 *    bp     - The buffer to be rendered into.
 *    prefix - The prefix character(s) of the variable
 *    name   - The name of the variable.
 *    value  - The value assigned to variable.
 * 
 * Produce a line in the form:-
 * 
 *    name......... "value"
 */
static void
showVariable(meBuffer *bp, meUByte prefix, meUByte *name, meUByte *value)
{
    meUByte buf[meBUF_SIZE_MAX+meSBUF_SIZE_MAX+16] ;
    int len;
    
    len = sprintf ((char *) buf, "    %c%s ", prefix, name);
    while (len < 35)
        buf[len++] = '.';
    buf[len++] = ' ';
    buf[len++] = '"';
    len = expandexp(-1,value,meBUF_SIZE_MAX+meSBUF_SIZE_MAX+16-2,len,buf,-1,NULL,meEXPAND_BACKSLASH|meEXPAND_FFZERO) ;
    buf[len++] = '"';
    buf[len] = '\0';
    addLineToEob(bp,buf);
}

static void
showVarList(meBuffer *bp, meUByte prefix, meVariable *varList)
{
    meVariable *tv, *nv;
    meUByte *vl=NULL, *vn;
    
    /* bit inefficient varList ordering but doesn't matter as this is a seldom used interactive function */
    for(;;)
    {
        vn = NULL;
        tv = varList;
        while(tv != NULL)
        {
            if(((vn == NULL) || (meStrcmp(tv->name,vn) < 0)) &&
               ((vl == NULL) || (meStrcmp(vl,tv->name) < 0)))
            {
                nv = tv;
                vn = tv->name;
            }
            tv = tv->next;
        }
        if(vn == NULL)
            return;
        showVariable(bp,prefix,nv->name,nv->value);
        vl = nv->name;
    }
}

/*
 * listVariables
 * List ALL of the variables held in the system.
 */

int
listVariables(int f, int n)
{
    meVariable *tv;
    meWindow *wp;
    meBuffer *bp;
    meUByte   buf[meBUF_SIZE_MAX];
    int       ii;
    
    /* don't execute hooks here as they can change variable values */
    if((wp = meWindowPopup(NULL,BvariablesN,(BFND_CREAT|BFND_CLEAR|BFND_NOHOOK|WPOP_USESTR),NULL)) == NULL)
        return meFALSE;
    bp = wp->buffer;
    
    addLineToEob(bp,(meUByte *)"Register variables:\n");
    
    buf[0] = 'g';
    buf[2] = '\0';
    for(ii = 0; ii<meREGISTER_MAX ; ii++)
    {
        buf[1] = (int)('0') + ii;
        showVariable(bp,'#',buf,meRegHead->val[ii]);
    }
    if(n & 1)
    {
        addLineToEob(bp,(meUByte *)"\nBuffer variables:\n");
        if((tv = frameCur->windowCur->buffer->varList) != NULL)
            showVarList(bp,':',tv);
    }
    else
    {
        meBuffer *bb = bheadp;
        meCommand *cc = cmdHead;
        while(bb != NULL)
        {
            if((tv = bb->varList) != NULL)
            {
                sprintf((char *)buf,"\nBuffer variables: %s\n",bb->name);
                addLineToEob(bp,buf);
                showVarList(bp,':',tv);
            }
            bb = bb->next;
        }
        while(cc != NULL)
        {
            if((tv = cc->varList) != NULL)
            {
                sprintf((char *)buf,"\nCommand variables: %s\n",cc->name);
                addLineToEob(bp,buf);
                showVarList(bp,'.',tv);
            }
            cc = cc->anext;
        }
    }
    
    addLineToEob(bp,(meUByte *)"\nSystem variables:\n");
    for (ii = 0; ii < NEVARS; ii++)
        showVariable(bp,'$',envars[ii],gtenv(envars[ii]));
    
    addLineToEob(bp,(meUByte *)"\nGlobal variables:\n");
    if((tv = usrVarList) != NULL)
        showVarList(bp,'%',tv);
    
    bp->dotLine = meLineGetNext(bp->baseLine);
    bp->dotOffset = 0;
    bp->dotLineNo = 0;
    meModeClear(bp->mode,MDEDIT);     /* don't flag this as a change */
    meModeSet(bp->mode,MDVIEW);       /* put this buffer view mode */
    resetBufferWindows(bp);           /* Update the window */
    setBufferContext(bp);             /* execute hook now (if any) */
    mlerase(MWCLEXEC);                /* clear the mode line */
    return meTRUE;
}
#endif
