/*****************************************************************************
 *
 *  Title:      %M%
 *
 *  Synopsis:   Expresion evaluation functions for MicroEMACS
 *
 ******************************************************************************
 *
 *  Filename:       %P%
 *
 *  Author:         Danial Lawrence
 *
 *  Creation Date:      14/05/86 12:37      <011026.2209>
 *
 *  Modification date:  %G% : %U%
 *
 *  Current rev:        %I%
 *
 *  Special Comments:   
 *
 *  Contents Description:   
 *
 *  Ammendments :   Jon Green of Akebia Ltd.
 *
 *          Token evaluation not performed correctly. Trying
 *          to read from the keyboard, line not terminated by
 *          a carridge return. Invoked from 'getval()' on
 *          TKARG condition.
 *
 * Jon Green    17-05-91
 * Added suffix modes so that the suffix defintions may be defined in the 
 * start up file.
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

/*---   Include defintions */

#define __EVALC 1       /* Define program name */

/*---   Include files */

#include "emain.h"
#include "evar.h"
#include "efunc.h"
#include "eskeys.h"
#include "esearch.h"
#include "evers.h"

#if (defined _UNIX) || (defined _DOS) || (defined _WIN32)
#include <sys/types.h>
#include <time.h>
#ifdef _UNIX
#include <sys/stat.h>
#endif
#ifndef _WIN32
#include <sys/time.h>
#endif
#endif

uint8 evalResult[TOKENBUF];    /* resulting string */
static uint8 machineName[]=meSYSTEM_NAME;    /* resulting string */

/*---   Local macro definitions */

/*---   External references */

#if TIMSTMP
extern uint8 time_stamp[];   /* Time stamp string */
#endif

/*---   Local type definitions */

/*---   Local variable definitions */
static time_t timeOffset=0 ;            /* Time offset in seconds */


#ifdef _INSENSE_CASE
meNAMESVAR buffNames={0,0} ;
meDIRLIST  fileNames={0,0} ;
#else
meNAMESVAR buffNames={1,0} ;
meDIRLIST  fileNames={1,0} ;
#endif
meNAMESVAR commNames={1,0} ;
meNAMESVAR modeNames={0,MDNUMMODES,modeName,0} ;
meNAMESVAR varbNames={1,0} ;

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
static meREGISTERS *gmaLocalRegPtr=NULL ;

#if MAGIC
/* Quick and easy interface to regex compare
 * 
 * return -1 on error, 0 on no match 1 if matched
 */
int
regexStrCmp(uint8 *str, uint8 *reg, int exact)
{
    static meRegex regex={0} ;
    int ii ;
    
    if(meRegexComp(&regex,reg,(exact) ? 0:meREGEX_ICASE) != meREGEX_OKAY)
        return -1 ;
    
    ii = meStrlen(str) ;
    return meRegexMatch(&regex,str,ii,0,ii,(meREGEX_BEGBUFF|meREGEX_ENDBUFF|meREGEX_MATCHWHOLE)) ;
}
#else
/* Cheesy regular expression comparer
 * 
 * reg is the regular expersion which is matched against str
 * reg's special chars are * ? [
 * in a [, a ^ can follow meaning not, and ranges are allowed
 * the exact flag indicates a case sensitive comparision
 */
int
regexStrCmp(uint8 *str, uint8 *reg, int exact)
{
    char *starStr, *starReg ;
    int   starState=0 ;
    char  rc, sc, nrc ;
    
    for(;;)
    {
        rc = *reg++ ;
        if(rc == '*')
        {
            /* A wild card, if its the last char in the match then we have
             * a match, else flag we got one and where (incase we 
             * have to go back) and move on
             */
            if(*reg == '\0')
                return 1 ;
            starState = 1 ;
            starReg = reg ;
        }
        else
        {
            if(!exact)
                rc = toLower(rc) ;
            for(;;)
            {
                sc = *str++ ;
                /* is it the end of str */
                if(sc == '\0')
                {
                    /* if its the end of reg break as if a char match, otherwise
                     * we have failed
                     */
                    if(rc != '\0')
                        return 0 ;
                    break ;
                }
                if(!exact)
                    sc = toLower(sc) ;
                /* if rc is a wild char then as long as sc isn't '\0'
                 * then its a match
                 */
                if(rc == '?')
                    break ;
                /* Test for a bracketed range */
                if(rc == '[')
                {
                    int not ;
                    /* do we inverse the result ? */
                    if(*reg == '^')
                    {
                        not = 1 ;
                        reg++ ;
                    }
                    else
                        not = 0 ;
                    while((rc=*reg++) != ']')
                    {
                        if(rc == '\0')
                            /* failed to find closing ']' - abort */
                            return -1 ;
                        nrc = *reg ;
                        if(nrc == '-')
                        {
                            /* range, note that if we're done a case insensitive compare
                             * we must test the two cases of sc and not change any of the
                             * range charaters
                             */
                            reg++ ;
                            nrc = *reg++ ;
                            if((sc >= rc) && (sc <= nrc))
                                break ;
                            if(!exact)
                            {
                                char tc=toggleCase(sc) ;
                                if((tc >= rc) && (tc <= nrc))
                                    break ;
                            }
                        }
                        else
                        {
                            /* single char compare */
                            if(!exact)
                                rc = toLower(rc) ;
                            if(sc == rc)
                                break ;
                        }
                    }
                    if(rc != ']')
                    {
                        /* found match, move to the ']' and continue */
                        while((rc=*reg++) != ']')
                            if(rc == '\0')
                                return -1 ;
                        /* X-or to allow for the not */
                        not ^= 1 ;
                    }
                    /* Did we find a match ? */
                    if(not)
                        break ;
                }
                /* If the letters are the same, match. */
                else if(sc == rc)
                    break ;
                /* Character match has failed - if there's been no wild
                 * card we've failed
                 */
                if(starState == 0)
                    return 0 ;
                if(starState < 0)
                {
                    /* There has been, but we've matched chars after, so
                     * move back to the char after the first match and try again
                     * This is to cope with matching *.cpp with test.c.cpp
                     */
                    starState = 1 ;
                    str = starStr ;
                    reg = starReg ;
                    rc = *reg++ ;
                    if(!exact)
                        rc = toLower(rc) ;
                }
            }
            /* we matched the char, if it was the null terminator then we've
             * got a match
             */
            if(sc == '\0')
                return 1 ;
            /* if the previous char was a'*' then store this location incase
             * we fail later on and we can come back to here and try again
             */
            if(starState > 0)
            {
                starState = -1 ;
                starStr = str ;
            }
        }
    }
}
#endif

/* meItoa
 * integer to ascii string..........
 * This is too inconsistant to use the system's */
#define INTWIDTH (sizeof(int)*4+2)
uint8 *
meItoa(int i)
{
    register uint8 *sp;      /* pointer into result */
    register int sign;      /* sign of resulting number */
    
    if(i == 0)          /* eliminate the trivial 0  */
        return meLtoa(0) ;
    else if(i == 1)     /* and trivial 1            */
        return meLtoa(1) ;
    
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
    
    return sp ;
}


meVARIABLE *
SetUsrLclCmdVar(uint8 *vname, uint8 *vvalue, register meVARLIST *varList)
{
    /* set a user variable */
    register meVARIABLE *vptr, *vp ;
    register int ii, jj ;
    
    vptr = (meVARIABLE *) varList ;
    ii = varList->count ;
    /* scan the list looking for the user var name */
    while(ii)
    {
        uint8 *s1, *s2, cc ;
        
        jj = (ii>>1)+1 ;
        vp = vptr ;
        while(--jj >= 0)
            vp = vp->next ;
        
        s1 = vp->name ;
        s2 = vname ;
        for( ; ((cc=*s1++) == *s2) ; s2++)
            if(cc == 0)
            {
                /* found it, replace value */
                meNullFree(vp->value);
                vp->value = meStrdup(vvalue) ;
                return vp ;
            }
        if(cc > *s2)
            ii = ii>>1 ;
        else
        {
            ii -= (ii>>1) + 1 ;
            vptr = vp ;
        }
    }
    
    /* Not found so create a new one */
    if((vp = (meVARIABLE *) meMalloc(sizeof(meVARIABLE)+meStrlen(vname))) != NULL)
    {
        meStrcpy(vp->name,vname) ;
        vp->next = vptr->next ;
        vptr->next = vp ;
        varList->count++ ;
        vp->value = meStrdup(vvalue) ;
    }
    return vp ;
}

int
setVar(uint8 *vname, uint8 *vvalue, meREGISTERS *regs)
{
    register int status ;         /* status return */
    uint8 *nn ;
    
    /* check the legality and find the var */
    nn = vname+1 ;
    switch(getMacroTypeS(vname))
    {
    case TKREG:
        {
            uint8 cc ;
            if((cc=*nn) == 'g')
                regs = meRegHead ;
            else if(cc == 'p')
                regs = regs->prev ;
            cc = nn[1] - '0' ;
            if(cc >= ('0'+meNUMREG))
                return mlwrite(MWABORT,(uint8 *)"[No such register %s]",vname);
            meStrcpy(regs->reg[cc],vvalue) ;
            break ;
        }
    case TKVAR:
        if(SetUsrLclCmdVar(nn,vvalue,&usrVarList) == NULL)
            return FALSE ;
        break ;
    case TKLVR:
        {
            BUFFER *bp ;
            uint8 *ss ;
            if((ss=meStrrchr(nn,':')) != NULL)
            {
                *ss = '\0' ;
                bp = bfind(nn,0) ;
                *ss++ = ':' ;
                if(bp == NULL)
                    /* not a buffer - abort */
                    return mlwrite(MWABORT,(uint8 *)"[No such variable]");
                nn = ss ;
            }
            else
                bp = curbp ;
            if(SetUsrLclCmdVar(nn,vvalue,&(bp->varList)) == NULL)
                return FALSE ;
            break ;
        }
    case TKCVR:
        {
            meVARLIST *varList ;
            uint8 *ss ;
            if((ss=meStrrchr(nn,'.')) != NULL)
            {
                int ii ;
                
                *ss = '\0' ;
                ii = decode_fncname(nn,1) ;
                *ss++ = '.' ;
                if(ii < 0)
                    /* not a command - abort */
                    return mlwrite(MWABORT,(uint8 *)"[No such variable]");
                varList = &(cmdTable[ii]->varList) ;
                nn = ss ;
            }
            else if((varList = regs->varList) == NULL)
                return mlwrite(MWABORT,(uint8 *)"[No such variable]");
            if(SetUsrLclCmdVar(nn,vvalue,varList) == NULL)
                return FALSE ;
            break ;
        }
    
    case TKARG:
        if(*nn == 'w')
        {
            if((status=bchange()) != TRUE)
                return status ;
            if(nn[1] == 'l')
            {
                int ll=0 ;
                uint8 cc ;
                
                if (curwp->w_dotp >= curbp->b_linep)
                    return mlwrite(MWABORT,(uint8 *)"[Cannot set @wl here]");
                
                curwp->w_doto = 0 ;
#if MEUNDO
                meUndoAddDelChars(llength(curwp->w_dotp)) ;
#endif
                mldelete(llength(curwp->w_dotp),NULL) ;
                while((cc=*vvalue++))
                {
                    if (cc == 0x0a)
                        status = lnewline();
                    else
                        status = linsert(1,cc);
                    if (status != TRUE)
                        return status ;
                    ll++ ;
                }
#if MEUNDO
                meUndoAddReplaceEnd(ll) ;
#endif
            }
            else
            {
                if (curwp->w_doto >= curwp->w_dotp->l_used)
                    return mlwrite(MWABORT,(uint8 *)"[Cannot set @wc here]");
                
                lchange(WFMAIN);
#if MEUNDO
                meUndoAddRepChar() ;
#endif
                lputc(curwp->w_dotp,curwp->w_doto,vvalue[0]) ;
            }
        }
        else if(*nn == 'c')
        {
            int ii ;
            if(nn[2] == 'k')
            {
                uint8 *saves, savcle ;
                          
                /* use the macro string to key evaluator to get the value,
                 * must set this up carefully and restore state */
                savcle = clexec;        /* save execution mode */
                clexec = TRUE;      /* get the argument */
                saves = execstr ;
                execstr = vvalue ;
                ii = meGetKey(meGETKEY_SILENT) ;
                execstr = saves ;
                clexec = savcle;        /* restore execution mode */
                if(nn[1] == 'c')
                    thisCommand = ii ;
                else
                    lastCommand = ii ;
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
                        thisflag = CFCPCN;
                        break ;
                    case CK_DELBAK:
                    case CK_DELFOR:
                        if(!meModeTest(curbp->b_mode,MDLETTR))
                            break ;
                    case CK_DELWBAK:
                    case CK_CPYREG:
                    case CK_DELFWRD:
                    case CK_KILEOL:
                    case CK_KILPAR:
                    case CK_KILREG:
                        thisflag = CFKILL;
                        break ;
                    case CK_REYANK:
                        thisflag = CFRYANK ;
                        break ;
                    case CK_YANK:
                        thisflag = CFYANK ;
                        break ;
                    case CK_UNDO:
                        thisflag = CFUNDO ;
                        break ;
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
        else
            return mlwrite(MWABORT,(uint8 *)"[Cannot set variable %s]",vname);
        break ;
    
    case TKENV:
    /* set an environment variable */
#ifdef _MACRO_COMP
        if((status = *nn) >= 128)
            status -= 128 ;
        else
#endif
            status = biChopFindString(nn,14,envars,NEVARS) ;
        switch(status)
        {
        case EVAUTOTIME:
            autotime = meAtoi(vvalue);
            break;
        case EVKEPTVERS:
            if(!(meSystemCfg & meSYSTEM_DOSFNAMES) && 
               ((keptVersions = meAtoi(vvalue)) < 0))
                keptVersions = 0 ;
            break;
        case EVBOXCHRS:
            {
                uint8 cc ;
                int len;
                len = meStrlen(vvalue);
                if (len > BCLEN)
                    len = BCLEN;
                while(--len >= 0)
                {
                    cc = vvalue[len] ;
                    if(!isPokable(cc))
                        cc = '.' ;
                    boxChars[len] = cc ;
                }
            }
            break;
        case EVIDLETIME:
            idletime = (uint32) meAtoi(vvalue);
            if (idletime < 10)
                idletime = 10;
            break;
        case EVDELAYTIME:
            delaytime = (uint32) meAtoi(vvalue);
            if (delaytime < 10)
                delaytime = 10;
            break;
        case EVREPEATTIME:
            repeattime = (uint32) meAtoi(vvalue);
            if (repeattime < 10)
                repeattime = 10;
            break;
        case EVMODELINE:
            if(modeLineStr != orgModeLineStr)
                meFree(modeLineStr) ;
            modeLineStr = meStrdup(vvalue) ;
            modeLineFlags = assessModeLine(vvalue) ;
            addModeToWindows(WFMODE) ;
            break ;
        case EVBMDLINE:
            meNullFree(curbp->modeLineStr) ;
            curbp->modeLineStr = meStrdup(vvalue) ;
            curbp->modeLineFlags = assessModeLine(vvalue) ;
            addModeToWindows(WFMODE) ;
            break ;
        case EVMOUSE:
            meMouseCfg = meAtoi(vvalue) ;
            TTinitMouse() ;
            break;
        case EVSYSTEM:
            {
                meSystemCfg = (meSystemCfg & ~meSYSTEM_MASK) | 
                          (meAtoi(vvalue)&meSYSTEM_MASK) ;
                if(meSystemCfg & meSYSTEM_DOSFNAMES)
                    keptVersions = 0 ;
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
#ifdef _XTERM
                /* on unix, if using x-window then can't set ANSI || XANSI bits */
                if(!(meSystemCfg & meSYSTEM_CONSOLE))
                    meSystemCfg &= ~(meSYSTEM_ANSICOLOR|meSYSTEM_XANSICOLOR) ;
#endif
#ifdef _CLIENTSERVER
                /* open or close the client server files */
                if(meSystemCfg & meSYSTEM_CLNTSRVR)
                    TTopenClientServer ();
                else
                    TTkillClientServer ();
#endif
                /* may changes may case a redraw so always redraw next time */
                sgarbf = TRUE ;
                break;
            }
        case EVFILLBULLET:
            meStrncpy(fillbullet,vvalue,15);
            fillbullet[15] = '\0' ;
            break;
        case EVFILLBULLETLEN:
            fillbulletlen = (int16) meAtoi (vvalue);
            break;
        case EVFILLCOL:
            fillcol = (int16) meAtoi(vvalue);
            break;
        case EVFILLEOS:
            meStrncpy(filleos,vvalue,15);
            filleos[15] = '\0' ;
            break;
        case EVFILLEOSLEN:
            filleoslen = (int16) meAtoi (vvalue);
            break;
        case EVFILLIGNORE:
            meStrncpy(fillignore,vvalue,15);
            fillignore[15] = '\0' ;
            break;
        case EVFILLMODE:
            fillmode = *vvalue ;
            break;
        case EVTIME:
            timeOffset = meAtoi(vvalue) ;
            break;
        case EVMATCHLEN:
            matchlen = (int16) meAtoi (vvalue);
            break;
        case EVPAGELEN:
            return changeScreenDepth(TRUE, meAtoi(vvalue));
        case EVRANDOM:
            srand(time(NULL)) ;
            break ;
        case EVABSCOL:
            return setcwcol(meAtoi(vvalue));
        case EVABSLINE:
            return gotoAbsLine(meAtoi(vvalue));
        case EVCURCOL:
            if((status=meAtoi(vvalue)) < 0)
            {
                curwp->w_doto = 0 ;
                return FALSE ;
            }
            else if(status > llength(curwp->w_dotp))
            {
                curwp->w_doto = llength(curwp->w_dotp) ;
                return FALSE ;
            }
            curwp->w_doto = status ;
            return TRUE ;
        case EVCURLINE:
            return gotoLine(TRUE, meAtoi(vvalue));
        case EVWINCHRS:
            {
                uint8 cc ;
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
        case EVWXSCROLL:
            if((status=meAtoi(vvalue)) < 0)
                status = 0 ;
            if(curwp->w_sscroll != status)
            {
                curwp->w_sscroll = status ;
                curwp->w_flag |= WFREDRAW ;        /* Force a screen update */
                updCursor(curwp) ;
            }
            return TRUE ;
        case EVWXCLSCROLL:
            if((status=meAtoi(vvalue)) < 0)
                status = 0 ;
            else if(status >= llength(curwp->w_dotp))
                status = llength(curwp->w_dotp)-1 ;
            if(curwp->w_scscroll != status)
            {
                curwp->w_scscroll = status ;
                curwp->w_flag |= WFREDRAW ;        /* Force a screen update */
                updCursor(curwp) ;
            }
            return TRUE ;
        case EVWYSCROLL:
            if((status=meAtoi(vvalue)) < 0)
                status = 0 ;
            else if(curbp->elineno && (status >= curbp->elineno))
                status = curbp->elineno-1 ;
            if(curwp->topLineNo != status)
            {
                curwp->topLineNo = status ;
                curwp->w_flag |= WFMAIN|WFSBOX|WFLOOKBK ;
                reframe(curwp) ;
            }
            return TRUE ;
        case EVCURWIDTH:
            return changeScreenWidth(TRUE, meAtoi(vvalue));
        case EVCBUFNAME:
            unlinkBuffer(curbp) ;
            if((vvalue[0] == '\0') || (bfind(vvalue,0) != NULL))
            {
                /* if the name is used */
                linkBuffer(curbp) ;
                return ABORT ;
            }
            meFree(curbp->b_bname) ;
            curbp->b_bname = meStrdup(vvalue);
            addModeToWindows(WFMODE) ;
            linkBuffer(curbp) ;
            break;
        case EVBUFFMOD:
            curbp->stats.stmode = (uint16) meAtoi(vvalue) ;
            break ;
        case EVGLOBFMOD:
            meUmask = (uint16) meAtoi(vvalue) ;
            break ;
        case EVCFNAME:
            meNullFree(curbp->b_fname) ;
            curbp->b_fname = meStrdup(vvalue);
            addModeToWindows(WFMODE) ;
            break;
        case EVDEBUG:
            macbug = (int8) meAtoi(vvalue);
            break;
#if TIMSTMP
        case EVTIMSTMP:
            meStrncpy(&time_stamp[0], vvalue, TSTMPLEN-1);
            time_stamp[TSTMPLEN-1] = '\0';
            break;
#endif
        case EVTABSIZE:
            tabsize = (uint16) meAtoi(vvalue);
            break;
        case EVTABWIDTH:
            tabwidth = (uint16) meAtoi(vvalue);
            addModeToWindows(WFRESIZE) ;
            break;
        case EVSRCHPATH:
            meNullFree(searchPath) ;
            searchPath = meStrdup(vvalue) ;
            break;
        case EVHOMEDIR:
            meNullFree(homedir) ;
            homedir = meStrdup(vvalue) ;
            break;
#if CFENCE
        case EVCBRACE:
            braceIndent = (int16) meAtoi (vvalue);
            break;
        case EVCCASE:
            caseIndent = (int16) meAtoi (vvalue);
            break;
        case EVCCONTCOMM:
            if(commentCont != commentContOrg)
                meFree(commentCont) ;
            commentCont = meStrdup(vvalue) ;
            break ;
        case EVCCONTINUE:
            continueIndent = (int16) meAtoi (vvalue);
            break;
        case EVCCONTMAX:
            continueMax = (int16) meAtoi (vvalue);
            break;
        case EVCMARGIN:
            commentMargin = (int16) meAtoi(vvalue);
            break;
        case EVCSTATEMENT:
            statementIndent = (int16) meAtoi (vvalue);
            break;
        case EVCSWITCH:
            switchIndent = (int16) meAtoi (vvalue);
            break;
#endif
        case EVSHWMDS:
            {
                uint8 cc ;
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
                addModeToWindows(WFMODE) ;
                break ;
            }
        case EVSHWRGN:
            selhilight.uFlags = (uint16) meAtoi(vvalue);
            break;
        case EVCURSORBLK:
            timerKill(CURSOR_TIMER_ID) ;
            if((cursorBlink = meAtoi(vvalue)) > 0)
                /* kick off the timer */
                TThandleBlink(1) ;
            break ;
#if COLOR
        case EVCURSORCOL:
            if((cursorColor = (meCOLOR) meAtoi(vvalue)) >= noColors)
                cursorColor = meCOLOR_FDEFAULT ;
            break ;
        case EVOSDSCHM:
            osdScheme = convertUserScheme(meAtoi(vvalue),osdScheme);
            break ;
        case EVMLSCHM:
            mlScheme = convertUserScheme(meAtoi(vvalue),mlScheme);
            break ;
        case EVMDLNSCHM:
            mdLnScheme = convertUserScheme(meAtoi(vvalue),mdLnScheme);
            addModeToWindows(WFMODE) ;
            break ;
        case EVGLOBSCHM:
            globScheme = convertUserScheme(meAtoi(vvalue),globScheme);
            vvideo.video [TTnrow].endp = TTncol ;
            sgarbf = TRUE ;
            TTsetBgcol() ;
            break ;
        case EVTRNCSCHM:
            trncScheme = convertUserScheme(meAtoi(vvalue),trncScheme);
            sgarbf = TRUE ;
            break ;
        case EVBUFSCHM:
            curbp->scheme = convertUserScheme(meAtoi(vvalue),curbp->scheme);
            addModeToBufferWindows(curbp,WFRESIZE) ;
            break ;
        case EVLINESCHM:
            {
                register int ii ;
                meSCHEME schm ;
                
                ii = meAtoi(vvalue) ;
                if(ii >= 0)
                {
                    schm = convertUserScheme(ii,curbp->scheme);
                    for(ii=1 ; ii<LNNOSCHM ; ii++)
                        if(curbp->lscheme[ii] == schm)
                            break ;
                    if(ii == LNNOSCHM)
                    {
                        if((ii=curbp->lschemeNext+1) == LNNOSCHM)
                            ii = 1 ;
                        curbp->lscheme[ii] = schm ;
                        curbp->lschemeNext = (ii+1) & LNSMASK ;
                    }
                    curwp->w_dotp->l_flag = (curwp->w_dotp->l_flag & ~LNSMASK) | ii ;
                }
                else if(curwp->w_dotp->l_flag & LNSMASK)
                    curwp->w_dotp->l_flag = (curwp->w_dotp->l_flag & ~LNSMASK) ;
                lchange(WFMAIN) ;
                break ;
            }
        case EVSBAR:    
            gsbarmode = meAtoi(vvalue) & WMUSER;
            resizeAllWnd (TRUE, 0);         /* Force window update */
            break;
        case EVSBARSCHM:
            sbarScheme = convertUserScheme(meAtoi(vvalue),sbarScheme);
            addModeToWindows(WFRESIZE|WFSBAR) ;
            break;
#endif
#if _IPIPES
        case EVBUFIPIPE:
            curbp->ipipeFunc = decode_fncname(vvalue,1) ;
            break ;
#endif
        case EVBUFINP:
            curbp->inputFunc = decode_fncname(vvalue,1) ;
            break ;
        case EVBUFBHK:
            curbp->bhook = decode_fncname(vvalue,1) ;
            break ;
        case EVBUFDHK:
            curbp->dhook = decode_fncname(vvalue,1) ;
            break ;
        case EVBUFEHK:
            curbp->ehook = decode_fncname(vvalue,1) ;
            break ;
        case EVBUFFHK:    
            curbp->fhook = decode_fncname(vvalue,1) ;
            break ;
#if HILIGHT
        case EVBUFHIL:
            if(((curbp->hiLight = (uint8) meAtoi(vvalue)) >= noHilights) ||
               (hilights[curbp->hiLight] == NULL))
                curbp->hiLight = 0 ;
            addModeToBufferWindows(curbp,WFRESIZE) ;
            break ;
        case EVBUFIND:
            if(((curbp->indent = (uint8) meAtoi(vvalue)) >= noIndents) ||
               (indents[curbp->indent] == NULL))
                curbp->indent = 0 ;
            break ;
#endif
        case EVBUFMASK:
            {
                int ii ;
                isWordMask = 0 ;
                for(ii=0 ; ii<8 ; ii++)
                    if(meStrchr(vvalue,charMaskFlags[ii]) != NULL)
                        isWordMask |= 1<<ii ;
                if(curbp->isWordMask != isWordMask)
                {
                    curbp->isWordMask = isWordMask ;
#if MAGIC
                    mereRegexClassChanged() ;
#endif
                }
                break ;
            }
        case EVFILEIGNORE:
            meNullFree(fileIgnore) ;
            fileIgnore = meStrdup(vvalue) ;
            break ;
#if FLNEXT
        case EVFILETEMP:
            meNullFree(flNextFileTemp) ;
            flNextFileTemp = meStrdup(vvalue) ;
            break ;
        case EVLINETEMP:
            meNullFree(flNextLineTemp) ;
            flNextLineTemp = meStrdup(vvalue) ;
            break ;
#endif
#if DORCS
        case EVRCSFILE:
            meNullFree(rcsFile) ;
            rcsFile = meStrdup(vvalue) ;
            break ;
        case EVRCSCICOM:
            meNullFree(rcsCiStr) ;
            rcsCiStr = meStrdup(vvalue) ;
            break ;
        case EVRCSCIFCOM:
            meNullFree(rcsCiFStr) ;
            rcsCiFStr = meStrdup(vvalue) ;
            break ;
        case EVRCSCOCOM:
            meNullFree(rcsCoStr) ;
            rcsCoStr = meStrdup(vvalue) ;
            break ;
        case EVRCSCOUCOM:
            meNullFree(rcsCoUStr) ;
            rcsCoUStr = meStrdup(vvalue) ;
            break ;
        case EVRCSUECOM:
            meNullFree(rcsUeStr) ;
            rcsUeStr = meStrdup(vvalue) ;
            break ;
#endif
        case EVSCROLL:
            scrollFlag = (uint8) meAtoi(vvalue) ;
            break ;
        case EVBNAMES:
            meNullFree(buffNames.list) ;
            meNullFree(buffNames.mask) ;
            if((buffNames.mask = meStrdup(vvalue)) != NULL)
                buffNames.size = createBuffList(&buffNames.list,0) ;
            buffNames.curr = 0 ;
            break ;
        case EVCNAMES:
            meNullFree(commNames.list) ;
            meNullFree(commNames.mask) ;
            if((commNames.mask = meStrdup(vvalue)) != NULL)
            {
                commNames.size = createCommList(&commNames.list,1) ;
                sortStrings(commNames.size,commNames.list,0,strcmp) ;
            }
            commNames.curr = 0 ;
            break ;
        case EVFNAMES:
            {
                uint8 *mm, *ss, cc ;
                
                meNullFree(fileNames.mask) ;
                ss = mm = vvalue ;
                while(((cc=*ss++) != '\0') && (cc != '[') && (*ss != '\0'))
                {
#ifdef _DRV_CHAR
                    if((cc == '/') || ((cc == _DRV_CHAR) && (*ss != '/')))
#else
                    if(cc == '/')
#endif
                        mm = ss ;
                }
                if((fileNames.mask = meStrdup(mm)) != NULL)
                {
                    uint8 buff[FILEBUF] ;
#ifdef _DRV_CHAR
                    if((isAlpha(mm[0]) || (mm[0] == '.' )) && (mm[1] == _DRV_CHAR))
                    {
                        resultStr[0] = '\0' ;
                        meStrcpy(buff,mm) ;
                        getDirectoryList(resultStr,&fileNames) ;
                    }
                    else
#endif
                    {
                        *mm = '\0' ;
                        getFilePath(curbp->b_fname,buff) ;
                        meStrcat(buff,vvalue) ;
                        pathNameCorrect(buff,PATHNAME_COMPLETE,resultStr,NULL) ;
                        getDirectoryList(resultStr,&fileNames) ;
                    }
                }
                fileNames.curr = 0 ;
                break ;
            }
        case EVMNAMES:
            meNullFree(modeNames.mask) ;
            modeNames.mask = meStrdup(vvalue) ;
            modeNames.curr = 0 ;
            break ;
        case EVVNAMES:
            if(varbNames.list != NULL)
                freeFileList(varbNames.size,varbNames.list) ;
            meNullFree(varbNames.mask) ;
            if((varbNames.mask = meStrdup(vvalue)) != NULL)
                varbNames.size = createVarList(&varbNames.list) ;
            varbNames.curr = 0 ;
            break ;
#if SPELL
        case EVFINDWORDS:
            findWordsInit(vvalue) ;
            break ;
#endif
        case EVRESULT:                      /* Result string - assignable */
            meStrcpy (resultStr, vvalue);
            break;
        case -1:
            {
                /* unknown so set an environment variable */
                char buff[MAXBUF+NSTRING] ;
                sprintf(buff,"%s=%s",nn,vvalue) ;
                mePutenv(meStrdup(buff)) ;
                break ;
            }
        default:
            /* default includes EVCBUFBACKUP EVMOUSEX EVMOUSEY EVSTATUS EVMACHINE 
             * EVSYSRET EVUSEX, EVVERSION, EVWMDLINE or system dependant vars
             * where this isn't the system (e.g. use-x) which cant be set
             */
            return FALSE ;
        }
        break ;
    default:
        /* else its not legal....bitch */
        return mlwrite(MWABORT,(uint8 *)"[No such variable]");
    }
    return TRUE ;
}

static uint8 *
gtenv(uint8 *vname)   /* vname   name of environment variable to retrieve */
{
    int ii ;
    register uint8 *ret ;
    register meNAMESVAR *mv ;
    
    /* scan the list, looking for the referenced name */
#ifdef _MACRO_COMP
    if((ii = *vname) >= 128)
        ii -= 128 ;
    else
#endif
        ii = biChopFindString(vname,14,envars,NEVARS) ;
    switch(ii)
    {
        /* Fetch the appropriate value */
    case EVAUTOTIME:    return (meItoa(autotime));
    case EVKEPTVERS:    return (meItoa(keptVersions));
    case EVBOXCHRS:     return boxChars;
    case EVDELAYTIME:   return (meItoa(delaytime));
    case EVIDLETIME:    return (meItoa(idletime));
    case EVREPEATTIME:  return (meItoa(repeattime));
    case EVMODELINE:    return modeLineStr ;
    case EVMACHINE:     return machineName ;
    case EVPROGNM:      return progName ;
    case EVCURSORX:     return (meItoa(mwCol));
    case EVCURSORY:     return (meItoa(mwRow));
    case EVMOUSE:       return (meItoa(meMouseCfg));
    case EVMOUSEPOS:    return (meItoa(mouse_pos));
    case EVMOUSEX:      return (meItoa(mouse_X));
    case EVMOUSEY:      return (meItoa(mouse_Y));
    case EVSYSTEM:      return (meItoa(meSystemCfg));
    case EVFILLBULLET:  return fillbullet;
    case EVFILLBULLETLEN:return (meItoa(fillbulletlen));
    case EVFILLCOL:     return (meItoa(fillcol));
    case EVFILLEOS:     return filleos;
    case EVFILLEOSLEN:  return (meItoa(filleoslen));
    case EVFILLIGNORE:  return fillignore;
    case EVFILLMODE:
        evalResult[0] = fillmode ;
        evalResult[1] = '\0' ;
        return evalResult ;
    case EVBNAMES:
        mv = &buffNames ;
        goto handle_namesvar ;
    case EVCNAMES:
        mv = &commNames ;
        goto handle_namesvar ;
    case EVFNAMES:
        mv = (meNAMESVAR *) &fileNames ;
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
            uint8 *ss = mv->list[(mv->curr)++] ;
            if(regexStrCmp(ss,mv->mask,mv->exact))
                return ss ;
        }
        return emptym ;
    case EVVERSION:
        return meVERSION_CODE ;
    case EVTEMPNAME:
        mkTempName (evalResult, NULL,NULL);
        return evalResult ;
#if SPELL
    case EVFINDWORDS:   return findWordsNext() ;
#endif
    case EVRECENTKEYS:
        {
            int ii, jj, kk ;
            uint16 cc ;
            for(ii=100,jj=TTnextKeyIdx,kk=0 ; --ii>0 ; )
            {
                if(((cc=TTkeyBuf[jj++]) == 0) ||
                   ((kk+=meGetStringFromChar(cc,evalResult+kk)) > MAXBUF-20))
                    break ;
                evalResult[kk++] = ' ' ;
                if(jj == KEYBUFSIZ)
                    jj = 0 ;
            }
            evalResult[kk] = '\0' ;
            return evalResult;
        }
    case EVRESULT:      return resultStr;
    case EVTIME:
        {
            struct meTimeval tp ;           /* Time interval for current time */
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
            gettimeofday(&tp,NULL) ;
            clock = tp.tv_sec + timeOffset ;              /* Get system time */
            time_ptr = (struct tm *) localtime (&clock);  /* Get time frame */
            /* SWP - Took out the 0 padding because of macro use.
             *       ME interprets numbers of the form 0## as octagon based numbers
             *       so in september the number is 09 which is illegal etc.
             */
            sprintf ((char *)evalResult, "%4d%3d%2d%2d%1d%2d%2d%2d%3d",
                     time_ptr->tm_year+1900,/* Year - 2000 should be ok !! */
                     time_ptr->tm_yday,     /* Year day */
                     time_ptr->tm_mon+1,    /* Month */
                     time_ptr->tm_mday,     /* Day of the month */
                     time_ptr->tm_wday,     /* Day of the week */
                     time_ptr->tm_hour,     /* Hours */
                     time_ptr->tm_min,      /* Minutes */
                     time_ptr->tm_sec,      /* Seconds */
                     (int) (tp.tv_usec/1000));/* Milliseconds */
            return evalResult;
        }
    case EVMATCHLEN:    return (meItoa(matchlen));
    case EVPAGELEN:     return (meItoa(TTnrow + 1));
    case EVRANDOM:      return (meItoa(rand()));
    case EVABSCOL:      return (meItoa(getcwcol()));
    case EVABSLINE:     return (meItoa(gotoAbsLine(-1)+1));
    case EVCURCOL:      return (meItoa(curwp->w_doto));
    case EVCURLINE:     return (meItoa(curwp->line_no+1));
    case EVWINCHRS:     return windowChars;
    case EVWXSCROLL:    return (meItoa(curwp->w_sscroll));
    case EVWXCLSCROLL:  return (meItoa(curwp->w_scscroll));
    case EVWYSCROLL:    return (meItoa(curwp->topLineNo));
    case EVBMDLINE:
        if(curbp->modeLineStr == NULL)
            return modeLineStr ;
        return curbp->modeLineStr ;
    case EVWMDLINE:     return (meItoa(curwp->firstRow+curwp->numTxtRows));
    case EVWSBAR:       return (meItoa(curwp->firstCol+curwp->numTxtCols));
    case EVWDEPTH:      return (meItoa(curwp->numTxtRows));
    case EVWWIDTH:      return (meItoa(curwp->numTxtCols));
    case EVCURWIDTH:    return (meItoa(TTncol));
    case EVCBUFBACKUP:
        if((curbp->b_fname == NULL) || createBackupName(evalResult,curbp->b_fname,'~',0))
            return (uint8 *) "" ;
#ifndef _DOS
        if(!(meSystemCfg & meSYSTEM_DOSFNAMES) && (keptVersions > 0))
            meStrcpy(evalResult+meStrlen(evalResult)-1,".~0~") ;
#endif
        return evalResult ;
    case EVCBUFNAME:    return (curbp->b_bname);
#ifdef _UNIX
    case EVBUFFMOD:
        sprintf((char *)evalResult,"0%o",(int) curbp->stats.stmode);
        return evalResult ;
    case EVGLOBFMOD:
        sprintf((char *)evalResult,"0%o",(int) meUmask);
        return evalResult ;
#else
    case EVBUFFMOD:     return (meItoa(curbp->stats.stmode));
    case EVGLOBFMOD:    return (meItoa(meUmask));
#endif
    case EVCFNAME:      return ((curbp->b_fname == NULL) ? (uint8 *)"":curbp->b_fname);
    case EVDEBUG:       return (meItoa(macbug));
    case EVSTATUS:      return (meLtoa(cmdstatus));
#if TIMSTMP
    case EVTIMSTMP:     return time_stamp ;
#endif
    case EVTABSIZE:     return (meItoa(tabsize));
    case EVTABWIDTH:    return (meItoa(tabwidth));
    case EVSRCHPATH:    return searchPath ;
    case EVHOMEDIR:     return homedir ;
#if CFENCE
    case EVCBRACE:      return meItoa(braceIndent) ;
    case EVCCASE:       return meItoa(caseIndent) ;
    case EVCCONTCOMM:   return commentCont ;
    case EVCCONTINUE:   return meItoa(continueIndent) ;
    case EVCCONTMAX:    return meItoa(continueMax) ;
    case EVCMARGIN:     return meItoa(commentMargin) ;
    case EVCSTATEMENT:  return meItoa(statementIndent) ;
    case EVCSWITCH:     return meItoa(switchIndent) ;
#endif
    case EVSHWMDS:
        {
            ret = evalResult;
            for (ii=0; ii < MDNUMMODES ; ii++) 
                *ret++ = (meModeTest(modeLineDraw,ii)) ? '1':'0' ;
            *ret = '\0' ;
            return evalResult;
        }
    case EVSHWRGN:      return meItoa(selhilight.uFlags);
    case EVCURSORBLK:   return meItoa(cursorBlink);
#if COLOR
    case EVCURSORCOL:   return meItoa(cursorColor);
    case EVOSDSCHM:     return meItoa(osdScheme/meSCHEME_STYLES);
    case EVMLSCHM:      return meItoa(mlScheme/meSCHEME_STYLES);
    case EVMDLNSCHM:    return meItoa(mdLnScheme/meSCHEME_STYLES);
    case EVSBARSCHM:    return meItoa(sbarScheme/meSCHEME_STYLES);
    case EVGLOBSCHM:    return meItoa(globScheme/meSCHEME_STYLES);
    case EVTRNCSCHM:    return meItoa(trncScheme/meSCHEME_STYLES);
    case EVBUFSCHM:     return meItoa(curbp->scheme/meSCHEME_STYLES);
    case EVLINESCHM:    return meItoa((curwp->w_dotp->l_flag & LNSMASK) ? 
                                      (curbp->lscheme[curwp->w_dotp->l_flag & LNSMASK]/meSCHEME_STYLES):-1) ;
    case EVSBAR:        return meItoa(gsbarmode);
#endif  /* COLOR  */
    case EVBUFIPIPE:
#if _IPIPES
        ii = curbp->ipipeFunc ;
#else
        ii = -1 ;
#endif
        goto hook_jump ;
    case EVBUFINP:
        ii = curbp->inputFunc ;
        goto hook_jump ;
    case EVBUFBHK:
        ii = curbp->bhook ;
        goto hook_jump ;
    case EVBUFDHK:
        ii = curbp->dhook ;
        goto hook_jump ;
    case EVBUFEHK:
        ii = curbp->ehook ;
        goto hook_jump ;
    case EVBUFFHK:    
        ii = curbp->fhook ;
hook_jump:
        if(ii < 0)
            evalResult[0] = '\0';
        else
            meStrcpy(evalResult,getCommandName(ii)) ;
        return evalResult ;

#if HILIGHT
    case EVBUFHIL:      return meItoa(curbp->hiLight);
    case EVBUFIND:      return meItoa(curbp->indent);
#endif
    case EVBUFMASK:
        {
            uint8 *ss=evalResult ;
            int   ii ;
            for(ii=0 ; ii<8 ; ii++)
                if(isWordMask & (1<<ii))
                    *ss++ = charMaskFlags[ii] ;
            *ss = '\0' ;
            return evalResult ;
        }
    case EVFILEIGNORE:  return fileIgnore ;
#if FLNEXT
    case EVFILETEMP:    return flNextFileTemp ;
    case EVLINETEMP:    return flNextLineTemp ;
#endif
#if DORCS
    case EVRCSFILE:     return rcsFile ;
    case EVRCSCOCOM:    return rcsCoStr ;
    case EVRCSCOUCOM:   return rcsCoUStr ;
    case EVRCSCICOM:    return rcsCiStr ;
    case EVRCSCIFCOM:   return rcsCiFStr ;
    case EVRCSUECOM:    return rcsUeStr ;
#endif
    case EVSCROLL:      return meItoa(scrollFlag) ;
    }
    if((ret = meGetenv(vname)) == NULL)
        ret = errorm ; /* errorm on bad reference */
    
    return ret ;
}

/* look up a user var's value */
/* vname - name of user variable to fetch */
uint8 *
getUsrLclCmdVar(uint8 *vname, register meVARLIST *varList)
{
    register meVARIABLE *vptr, *vp ;
    register int ii, jj ;
    
    vptr = varList->head ;
    ii = varList->count ;
    /* scan the list looking for the user var name */
    while(ii)
    {
        int8 *s1, *s2, ss ;
        
        jj = ii>>1 ;
        vp = vptr ;
        while(--jj >= 0)
            vp = vp->next ;
        
        s1 = (int8 *) vp->name ;
        s2 = (int8 *) vname ;
        for( ; ((ss=*s1++) == *s2) ; s2++)
            if(ss == 0)
                return(vp->value);
        if(ss > *s2)
            ii = ii>>1 ;
        else
        {
            ii -= (ii>>1) + 1 ;
            vptr = vp->next ;
        }
    }
    return errorm ;     /* return errorm on a bad reference */
}


static uint8 *
getMacroArg(int index)
{
    meREGISTERS *crp ;
    uint8 *oldestr, *ss ;
    
    /* move the register pointer to the parent as any # reference
     * will be w.r.t the parent
     */
    crp = meRegCurr ;
    if((ss=crp->execstr) == NULL)
        return NULL ;
    oldestr = execstr ;
    execstr = ss ;
    meRegCurr = crp->prev ;
    if(alarmState & meALARM_VARIABLE)
        gmaLocalRegPtr = meRegCurr ;
    for( ; index>0 ; index--)
    {
        execstr = token(execstr, evalResult) ;
        ss = getval(evalResult) ;
    }
    execstr   = oldestr ;
    meRegCurr = crp ;
    return ss ;
}


uint8 *
getval(uint8 *tkn)   /* find the value of a token */
{
    switch (getMacroTypeS(tkn)) 
    {
    case TKARG:
        if(tkn[1] == 'w')
        {
            if(alarmState & meALARM_VARIABLE)
                return tkn ;
            if(tkn[2] == 'l')
            {
                register int blen ;
                uint8 *ss, *dd ;
                /* Current buffer line fetch */
                blen = curwp->w_dotp->l_used;
                if (blen >= MAXBUF)
                    blen = MAXBUF-1 ;
                ss = curwp->w_dotp->l_text ;
                dd = evalResult ;
                while(--blen >= 0)
                    *dd++ = *ss++ ;
                *dd = '\0' ;
            }
            else
            {
                /* Current Buffer character fetch */
                evalResult[0] = getCurChar(curwp) ;
                evalResult[1] = '\0';
            }
        }
        else if(tkn[1] == '#')
        {
            if(alarmState & meALARM_VARIABLE)
                return tkn ;
            return meItoa(meRegCurr->n) ;
        }
        else if(tkn[1] == '?')
        {
            if(alarmState & meALARM_VARIABLE)
                return tkn ;
            return meItoa(meRegCurr->f) ;
        }
        else if(isDigit(tkn[1]))
        {
            int index ;
            uint8 *ss ;
            
            if((index = meAtoi(tkn+1)) == 0)
            {
                ss = meRegCurr->commandName ;
                if(ss == NULL)
                    return "" ;
            }
            else if((ss = getMacroArg(index)) == NULL)
                return abortm ;
            return ss ;
        }
        else if((tkn[1] == 's') && isDigit(tkn[2]))
        {
            int ii ;
            
            if(srchLastState != TRUE)
                return abortm ;
            if((ii=tkn[2]-'0') == 0)
                return srchLastMatch ;
#if MAGIC
            if(srchLastMagic && (ii <= mereRegexGroupNo()))
            {
                int jj ;
                if((jj=mereRegexGroupStart(ii)) >= 0)
                {
                    ii=mereRegexGroupEnd(ii) - jj ;
                    if(ii >= MAXBUF)
                        ii = MAXBUF-1 ;
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
        else if(tkn[1] == 'y')
        {
            /* Don't use the X or windows clipboard in this case */
            register uint8 *ss, *dd, cc ;
            register int   ii=MAXBUF-1 ;
            KILL          *killp;
            
#ifdef _CLIPBRD
            if(clipState & CLIP_TRY_GET)
                TTgetClipboard() ;
#endif
            if(klhead == (KLIST*) NULL)
                return abortm ;
            dd = evalResult ;
            killp = klhead->kill;
            while(ii && (killp != NULL))
            {
                ss = killp->data ;
                while(((cc=*ss++) != '\0') && (--ii))
                    *dd++ = cc ;
                killp = killp->next;
            }
            *dd = '\0' ;
        }
        else if(tkn[1] == 'c')
        {
            uint16 key ;
            int kk, index ;
                        
            kk = (tkn[3] == 'k') ;
            if(tkn[2] == 'c')
            {
                if(alarmState & meALARM_VARIABLE)
                    return tkn ;
                key = (uint16) thisCommand ;
                index = thisIndex ;
            }
            else if(tkn[2] == 'l')
            {
                if(alarmState & meALARM_VARIABLE)
                    return tkn ;
                key = (uint16) lastCommand ;
                index = lastIndex ;
            }
            else
            {
                if(tkn[2] == 'q')
                {
                    /* intercative single char read which will be quoted */
                    int cc ;
                    key = meGetKeyFromUser(FALSE,0,meGETKEY_SILENT|meGETKEY_SINGLE) ;
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
                    key = meGetKeyFromUser(FALSE, 1, 0) ;
                if(!kk)
                {
                    uint32 arg ;
                    index = decode_key(key,&arg) ;
                }
            }
            if(kk)
                meGetStringFromKey(key,evalResult) ;
            else if(index < 0)
            {
                if(key > 0xff)
                    return errorm ;
                evalResult[0] = (uint8) key ;
                evalResult[1] = '\0';
            }
            else
                meStrcpy(evalResult,getCommandName(index)) ;
        }
        else if(tkn[1] == 'm')
        {
            if(tkn[2] == 'l')
            {
                /* interactive argument */
                /* note: the result buffer cant be used directly as it is used
                 *       in modeline which is is by update which can be used
                 *       by meGetStringFromUser
                 */
                static uint8 **strList=NULL ;
                static int    strListSize=0 ;
                uint8 cc, *ss, buff[TOKENBUF] ;
                uint8 divChr ;
                uint8 prompt[MAXBUF] ;
                int  option=0 ;
                int  flag, defH ;
                
                if((flag=tkn[3]) != '\0')
                {
                    flag = hexToNum(flag) ;
                    if(((cc=tkn[4]) > '0') && (cc<='9'))
                        cc -= '0' ;
                    else if(cc == 'a')
                        cc = 10 ;
                    else
                        cc = 11 ;
                    if(cc == 1)
                        option = MLFILE ;
                    else
                    {
                        if(cc == 7)
                        {
                            mlgsStrList = modeName ;
                            mlgsStrListSize = MDNUMMODES ;
                        }
                        else if(cc == 9)
                        {
                            flag |= 0x100 ;
                            cc = 7 ;
                        }
                        else if(cc == 10)
                        {
                            flag |= 0x300 ;
                            cc = 7 ;
                        }
                        if(cc < 11)
                            option = 1 << (cc-2) ;
                    }
#if (MLFILECASE != MLFILE)
                    if(option & MLFILE)
                        option |= MLFILECASE ;
#if (MLBUFFERCASE != MLBUFFER)
                    else if(option & MLBUFFER)
                        option |= MLBUFFERCASE ;
#endif
#endif
                }
                else
                    cc = 0 ;
                
                /* Get and evaluate the next argument - this is the prompt */
                execstr = token(execstr, buff) ;
                if((ss = getval(buff)) == abortm)
                    return abortm ;
                meStrcpy(prompt,ss) ;
                if(flag & 0x01)
                {
                    /* Get and evaluate the next argument - this is the default
                     * value
                     */
                    execstr = token(execstr, buff) ;
                    if((ss = getval(buff)) == abortm)
                        return abortm ;
                    addHistory(option,ss) ;
                    defH = 1 ;
                }
                else
                    defH = 0 ;
                if(flag & 0x02)
                {
                    /* Get and evaluate the next argument - this is the ml
                     * line init value
                     */
                    execstr = token(execstr, buff) ;
                    if((ss = getval(buff)) == abortm)
                        return abortm ;
                    if(ss != buff)
                        meStrcpy(buff,ss) ;
                    option |= MLNORESET ;
                }
                if(flag & 0x04)
                    /* set the first key to 'tab' to expand the input according 
                     * to the completion list */
                    mlfirst = ME_SPECIAL|SKEY_tab ;
                if(flag & 0x08)
                    option |= MLHIDEVAL ;
                
                if(flag & 0x100)
                {
                    /* Get and evaluate the next argument - this is the
                     * completion list
                     */
                    uint8 comp[TOKENBUF], *tt ;
                    execstr = token(execstr,comp) ;
                    if((ss = getval(comp)) == abortm)
                        return abortm ;
                    if(flag & 0x200)
                    {
                        BUFFER *bp ;
                        LINE *lp ;
                        flag &= ~0x100 ;
                        if((bp = bfind(ss,0)) == NULL)
                            return abortm ;
                        if(bp->elineno > strListSize)
                        {
                            strListSize = bp->elineno ;
                            if((strList = meRealloc(strList,strListSize*sizeof(uint8 *))) == NULL)
                            {
                                strListSize = 0 ;
                                return abortm ;
                            }
                        }
                        lp = bp->b_linep ;
                        mlgsStrListSize = 0 ;
                        while((lp=lforw(lp)) != bp->b_linep)
                            strList[mlgsStrListSize++] = lp->l_text ;
                    }
                    else
                    {
                        /* if the resultant string is going to the same
                         * location as we are getting the competion list
                         * then we MUST not restore the list after, so remove
                         * the flag
                         */
                        if((ss >= evalResult) && (ss < evalResult+MAXBUF))
                            flag &= ~0x100 ;
                        divChr = *ss++ ;
                        mlgsStrListSize = 0 ;
                        while((tt = meStrchr(ss,divChr)) != NULL)
                        {
                            *tt++ = '\0' ;
                            if(mlgsStrListSize == strListSize)
                            {
                                if((strList = meRealloc(strList,(strListSize+8)*sizeof(uint8 *))) == NULL)
                                {
                                    strListSize = 0 ;
                                    return abortm ;
                                }
                                strListSize += 8 ;
                            }
                            strList[mlgsStrListSize++] = ss ;
                            ss = tt ;
                        }
                    }
                    mlgsStrList = strList ;
                }
                if(cc == 1)
                {
                    if(!(option & MLNORESET))
                        getFilePath(curbp->b_fname,buff) ;
                    option &= MLHIDEVAL ;
                    option |= MLFILECASE|MLNORESET|MLMACNORT ;

                    clexec = FALSE ;
                    if((cc = meGetString(prompt,option,0,buff,MAXBUF)) == TRUE)
                        fileNameCorrect(buff,evalResult,NULL) ;
                    clexec = TRUE ;
                }
                else
                {
                    /* Note that buffer result cannot be used directly as in the process
                     * of updating the screen the result buffer can be used by meItoa
                     * so use a temp buffer and copy across. note that inputFileName
                     * above doesn't suffer from this as the function uses a temp buffer
                     */
                    cc = meGetStringFromUser(prompt,option,defH,buff,MAXBUF) ;
                    meStrncpy(evalResult,buff,MAXBUF) ;
                    evalResult[MAXBUF-1] = '\0' ;
                }
                if(flag & 0x100)
                {
                    /* glue the completion string back together */
                    while(--mlgsStrListSize >= 0)
                    {
                        uint8 *s1 = mlgsStrList[mlgsStrListSize] ;
                        s1[meStrlen(s1)] = divChr ;
                    }
                }
                if(cc == ABORT)
                    return abortm ;
            }
            else if(tkn[2] == 'c')
            {
                uint8 *ss, buff[TOKENBUF] ;
                uint8 prompt[MAXBUF] ;
                int ret ;
                
                if((ret=tkn[3]) != '\0')
                    ret -= '0' ;
                
                /* Get and evaluate the next argument - this is the prompt */
                execstr = token(execstr,buff) ;
                if((ss = getval(buff)) == abortm)
                    return abortm ;
                meStrcpy(prompt,ss) ;
                if(ret & 0x01)
                {
                    /* Get and evaluate the next argument - this is valid
                     * values list
                     */
                    execstr = token(execstr, buff) ;
                    if((ss = getval(buff)) == abortm)
                        return abortm ;
                    if((ss != buff) || (ss != buff+1))
                    {
                        meStrcpy(buff,ss) ;
                        ss = buff ;
                    }
                }
                else
                    ss = NULL ;
                
                clexec = FALSE ;
                ret = mlCharReply(prompt,mlCR_QUOTE_CHAR,ss,NULL) ;
                clexec = TRUE ;
                if(ret < 0)
                    return abortm ;
                evalResult[0] = ret ;
                evalResult[1] = 0 ;
            }
            else
                return abortm ;
                    
        }
        else if(tkn[1] == 'p')
        {
            /* parent command name */
            if(meRegCurr->prev->commandName == NULL)
                return "" ;
            return meRegCurr->prev->commandName ;
        }
        else if((tkn[1] == 'f') && (tkn[2] == 's'))
        {
            /* frame store @fs <row> <col> */
            uint8 *ss, buff[TOKENBUF] ;
            int row, col ;
            
            /* Get and evaluate the arguments */
            execstr = token(execstr,buff) ;
            if((ss = getval(buff)) == abortm)
                return abortm ;
            row = meAtoi(ss) ;
            execstr = token(execstr,buff) ;
            if((ss = getval(buff)) == abortm)
                return abortm ;
            col = meAtoi(ss) ;
            /* Off the screen ?? */
            if((row < 0) || (row > TTnrow) ||
               (col < 0) || (col >= TTncol))
                evalResult[0] = 0 ;
            else
            {
                evalResult[0] = frameStore[row].text[col] ;
                evalResult[1] = 0 ;
            }
        }
        else
        {
            mlwrite(MWABORT|MWWAIT,(uint8 *)"[Unknown argument %s]",tkn);
            return abortm ;
        }
        return evalResult ;
        
    case TKREG:
        if(alarmState & meALARM_VARIABLE)
            return tkn ;
        {
            meREGISTERS *rp ;
            uint8 cc ;
            
            if((cc=tkn[1]) == 'l')
                rp = meRegCurr ;
            else if(cc == 'p')
                rp = meRegCurr->prev ;
            else
                rp = meRegHead ;
            cc = tkn[2] - '0' ;
            if(cc < ('0'+meNUMREG))
                return rp->reg[cc] ;
            break ;
        }
    
    case TKVAR:
        if(alarmState & meALARM_VARIABLE)
            return tkn ;
        return getUsrVar(tkn+1) ;
    
    case TKLVR:
        {
            uint8 *ss, *tt ;
            BUFFER *bp ;
            if(alarmState & meALARM_VARIABLE)
                return tkn ;
            tt = tkn+1 ;
            if((ss=meStrrchr(tt,':')) != NULL)
            {
                *ss = '\0' ;
                bp = bfind(tt,0) ;
                *ss++ = ':' ;
                if(bp == NULL)
                    /* not a buffer - abort */
                    return errorm ;
                tt = ss ;
            }
            else
                bp = curbp ;
            return getUsrLclCmdVar(tt,&(bp->varList)) ;
        }
        
    case TKCVR:
        {
            uint8 *ss, *tt ;
            if(alarmState & meALARM_VARIABLE)
                return tkn ;
            tt = tkn+1 ;
            if((ss=meStrrchr(tt,'.')) != NULL)
            {
                int ii ;
                *ss = '\0' ;
                ii = decode_fncname(tt,1) ;
                *ss++ = '.' ;
                if(ii < 0)
                    /* not a command - error */
                    return errorm ;
                return getUsrLclCmdVar(ss,&(cmdTable[ii]->varList)) ;
            }
            if(meRegCurr->varList == NULL)
                return abortm ;
            return getUsrLclCmdVar(tt,meRegCurr->varList) ;
        }
    case TKENV:
        if(alarmState & meALARM_VARIABLE)
            return tkn ;
        return gtenv(tkn+1) ;
    
    case TKFUN:
        return gtfun(tkn+1) ;
    
    case TKSTR:
        tkn++ ;
    case TKCMD:
    case TKLIT:
        return tkn ;
    }
    /* If here then it is either TKDIR, TKLBL or TKNUL. Doesn't matter which
     * cause something has gone wrong and we return abortm
     */
    return abortm ;
}

static uint8 *
queryMode(uint8 *name, uint8 *mode)
{
    int ii ;
    
    for (ii=0; ii < MDNUMMODES ; ii++) 
        if(meStrcmp(name,modeName[ii]) == 0) 
            return meLtoa(meModeTest(mode,ii)) ;
    return abortm ;
}

uint8 *
gtfun(uint8 *fname)  /* evaluate a function given name of function */
{
    register int fnum;      /* index to function to eval */
    meREGISTERS *regs ;     /* pointer to relevant regs if setting var */
    uint8 arg1[MAXBUF];      /* value of first argument */
    uint8 arg2[MAXBUF];      /* value of second argument */
    uint8 arg3[MAXBUF];      /* value of third argument */
    uint8 *varVal ;
    
    /* look the function up in the function table */
#ifdef _MACRO_COMP
    if((fnum = *fname) >= 128)
        fnum -= 128 ;
    else
#endif
        if((fnum = biChopFindString(fname,3,funcNames,NFUNCS)) < 0)
    {
        mlwrite(MWABORT|MWWAIT,(uint8 *)"[Unknown function &%s]",fname);
        return abortm ;
    }
    if((fnum == UFCBIND) || (fnum == UFNBIND))
    {
        uint32 arg ;
        uint16 ii ;
        int jj ;
        
        ii = meGetKey(meGETKEY_SILENT) ;
        if((jj = decode_key(ii,&arg)) < 0)
            return errorm ;
        if(fnum == UFCBIND)
            meStrcpy(evalResult,getCommandName(jj)) ;
        else if(arg == 0)
            evalResult[0] = '\0' ;
        else
            return meItoa((int) (arg + 0x80000000)) ;
        return evalResult ;
    }
    /* retrieve the arguments */
    {
        uint8 alarmStateStr=alarmState ;
        int ii ;
        
        if(funcTypes[fnum] & FUN_SETVAR)
        {
            /* horrid global variable, see notes at definition */
            gmaLocalRegPtr = meRegCurr ;
            alarmState |= meALARM_VARIABLE ;
            ii = macarg(arg1) ;
            alarmState &= ~meALARM_VARIABLE ;
            regs = gmaLocalRegPtr ;
            if((ii == TRUE) && (funcTypes[fnum] & FUN_GETVAR) &&
               ((varVal = getval(arg1)) == abortm))
                ii = FALSE ;
        }
        else
        {
            alarmState &= ~meALARM_VARIABLE ;
            ii = macarg(arg1) ;
        }
        if((ii != TRUE) ||
           ((funcTypes[fnum] & FUN_ARG2) &&
            ((macarg(arg2) != TRUE) ||
             ((funcTypes[fnum] & FUN_ARG3) &&
              (macarg(arg3) != TRUE)))))
        {
            mlwrite(MWABORT|MWWAIT,(uint8 *)"[Failed to get argument for function &%s]",fname);
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
    case UFEXIST:
        switch (getMacroTypeS(arg1)) 
        {
        case TKARG:
        case TKREG:
        case TKVAR:
        case TKLVR:
        case TKCVR:
        case TKENV:
            /* all variable types */
            varVal=getval(arg1) ;
            return (meLtoa((varVal != abortm) && (varVal != errorm))) ;
            
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
            uint32 narg ;
            int ii, idx ;
            uint16 code=ME_INVALID_KEY ;
            
            if((idx = decode_fncname(arg2,0)) < 0)
                return abortm ;
            if(arg1[0] == '\0')
                narg = 0 ;
            else
                narg = (uint32) (0x80000000+meAtoi(arg1)) ;
#if LCLBIND
            for(ii=0 ; ii<curbp->nobbinds ; ii++)
            {
                if((curbp->bbinds[ii].index == idx) &&
                   (curbp->bbinds[ii].arg == narg))
                {
                    code = curbp->bbinds[ii].code ;
                    break ;
                }
            }
            if(code == ME_INVALID_KEY)
#endif
            {
                for(ii=0 ; ii<keyTableSize ; ii++)
                    if((keytab[ii].index == idx) &&
                       (keytab[ii].arg == narg))
                    {
                        code = keytab[ii].code ;
                        break ;
                    }
            }
            if(code != ME_INVALID_KEY)
                meGetStringFromKey(code,evalResult) ;
            else
                evalResult[0] = '\0' ;
            return evalResult ;
        }
    case UFABS:        return meItoa(abs(meAtoi(arg1)));
    case UFADD:        return meItoa(meAtoi(arg1) + meAtoi(arg2));
    case UFSUB:        return meItoa(meAtoi(arg1) - meAtoi(arg2));
    case UFMUL:        return meItoa(meAtoi(arg1) * meAtoi(arg2));
    case UFDIV:
        {
            int ii, jj ;
            
            /* Check for divide by zero, this causes an exception if
             * encountered so we must abort if detected. */
            ii  = meAtoi(arg1) ;
            if ((jj = meAtoi(arg2)) == 0)
            {
                mlwrite(MWABORT|MWWAIT,(uint8 *)"[Divide by zero &%s]",fname) ;
                return abortm ;
            }
            ii /= jj ;
            return meItoa(ii) ;
        }
     case UFMOD:
        {
            int ii, jj;
            
            /* Check for divide by zero, this causes an exception if
             * encountered so we must abort if detected */
            ii  = meAtoi(arg1) ;
            if ((jj = meAtoi(arg2)) == 0)
            {
                mlwrite(MWABORT|MWWAIT,(uint8 *)"[Modulus of zero &%s]",fname) ;
                return abortm ;
            }
            ii %= jj ;
            return meItoa(ii);
        }
    case UFNEG:        return meItoa(0-meAtoi(arg1));
    case UFATOI:       return meItoa(arg1[0]) ;
    case UFITOA:
        {
            int ii = meAtoi(arg1) ;
            evalResult[0] = (uint8) ii ;
            if(ii == 0xff)
            {
                evalResult[1] = 0xff ;
                evalResult[2] = '\0' ;
            }
            else
                evalResult[1] = '\0' ;
            return evalResult ;
        }
    case UFCAT:
        {
            uint8 *dd, *ss ;
            int ii = MAXBUF-1 ;
            
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
            uint8 *dd, *ss, cc ;
            int ii=meAtoi(arg2) ;
            ss = arg1 ;
            dd = evalResult ;
            while(--ii >= 0)
            {
                if((cc=*ss++) == 0xff)
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
            uint8 *dd, *ss, cc ;
            int ii=meAtoi(arg2) ;
            ss = arg1 ;
            while(--ii >= 0)
            {
                if((cc=*ss++) == 0xff)
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
            uint8 *dd, *ss, cc ;
            int ii=meAtoi(arg2) ;
            ss = arg1 ;
            while(--ii >= 0)
            {
                if((cc=*ss++) == 0xff)
                    cc = *ss++ ;
                if(cc == '\0')
                {
                    ss-- ;
                    break ;
                }
            }
            ii = meAtoi(arg3) ;
            dd = evalResult ;
            while(--ii >= 0)
            {
                if((cc=*ss++) == 0xff)
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
            uint8 *ss, cc ;
            int ii=0 ;
            ss = arg1 ;
            for(;;)
            {
                if((cc=*ss++) == 0xff)
                    cc=*ss++ ;
                if(cc == '\0')
                    break ;
                ii++ ;
            }
            return meItoa(ii) ;
        }
    case UFSLOWER:
        {
            uint8 cc, *dd, *ss ;
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
            uint8 cc, *dd, *ss ;
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
            Fintssi cmpIFunc ;
            uint8 cc, *ss=arg2, *lss=NULL ;
            int len, off=0 ;
            len = meStrlen(arg1) ;
            if((fnum == UFSIN) || (fnum == UFRSIN))
                cmpIFunc = strncmp ;
            else
                cmpIFunc = strnicmp ;
            
            do
            {
                if(!cmpIFunc((char *)arg1,(char *)ss,len))
                {
                    lss = ss ;
                    if((fnum == UFSIN) || (fnum == UFISIN))
                        break ;
                }
                if((cc=*ss++) == 0xff)
                {
                    cc = *ss++ ;
                    off++ ;
                }
            } while(cc != '\0') ;
            
            if(lss != NULL)
                return meItoa(lss-arg2-off+1);
            return meLtoa(0) ;
        }
    case UFNBMODE:
        {
            BUFFER *bp ;
            if((bp = bfind(arg1,0)) == NULL)
                return abortm ;
            return queryMode(arg2,bp->b_mode) ;
        }
    case UFBMODE:      return queryMode(arg1,curbp->b_mode) ;
    case UFGMODE:      return queryMode(arg1,globMode) ;
    case UFBAND:       return(meItoa(meAtoi(arg1) & meAtoi(arg2)));
    case UFBNOT:       return(meItoa(~meAtoi(arg1))) ;
    case UFBOR:        return(meItoa(meAtoi(arg1) |  meAtoi(arg2)));
    case UFBXOR:       return(meItoa(meAtoi(arg1) ^  meAtoi(arg2)));
    case UFNOT:        return(meLtoa(meAtol(arg1) == FALSE));
    case UFAND:        return(meLtoa(meAtol(arg1) && meAtol(arg2)));
    case UFOR:         return(meLtoa(meAtol(arg1) || meAtol(arg2)));
    case UFEQUAL:      return(meLtoa(meAtoi(arg1) == meAtoi(arg2)));
    case UFLESS:       return(meLtoa(meAtoi(arg1) <  meAtoi(arg2)));
    case UFGREATER:    return(meLtoa(meAtoi(arg1) >  meAtoi(arg2)));
    case UFSEQUAL:     return(meLtoa(meStrcmp(arg1,arg2) == 0));
    case UFSLESS:      return(meLtoa(meStrcmp(arg1,arg2) < 0));
    case UFSGREAT:     return(meLtoa(meStrcmp(arg1,arg2) > 0));
    case UFINWORD:     return(meLtoa(isWord(arg1[0])));
    case UFISEQUAL:    return(meLtoa(meStricmp(arg1,arg2) == 0));
    case UFXSEQ:       return(meLtoa(regexStrCmp(arg1,arg2,1) == 1));
    case UFXISEQ:      return(meLtoa(regexStrCmp(arg1,arg2,0) == 1));
    case UFLDEL:
        {
            int  index=meAtoi(arg2) ;
            uint8 cc, *s1, *s2 ;
            if(index > 0)
            {
                s2 = arg1 ;
                cc = *s2 ;
                do {
                    s1 = s2+1 ;
                    if((s2 = meStrchr(s1,cc)) == NULL)
                        break ;
                } while(--index > 0) ;
            }
            else
                s2 = NULL ;
            if(s2 == NULL)
                meStrcpy(evalResult,arg1) ;
            else
            {
                index = (int) (s1 - arg1) ;
                meStrncpy(evalResult,arg1,index) ;
                meStrcpy(evalResult+index,s2+1) ;
            }
            return evalResult ;
        }
    case UFLFIND:
        {
            int  index ;
            uint8 cc, *s1, *s2 ;
            s2 = arg1 ;
            cc = *s2 ;
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
            int  index ;
            uint8 cc, *s1, *s2 ;
            if((index=meAtoi(arg2)) <= 0)
                return emptym ;
            s2 = arg1 ;
            cc = *s2 ;
            do {
                s1 = s2+1 ;
                if((s2 = meStrchr(s1,cc)) == NULL)
                    return emptym ;
            } while(--index > 0) ;
            index = (int) (s2 - s1) ;
            meStrncpy(evalResult,s1,index) ;
            evalResult[index] = '\0' ;
            return evalResult ;
        }
    case UFLINS:
    case UFLSET:
        {
            int  index=meAtoi(arg2), ii ;
            uint8 cc, *s1, *s2 ;
            s2 = arg1 ;
            cc = *s2 ;
            if(index <= 0)
            {
                if(index < 0)
                    s2 += meStrlen(s2)-1 ;
                s1 = s2+1 ;
            }
            else
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
            index = (int) (s1 - arg1) ;
            meStrncpy(evalResult,arg1,index) ;
            ii = meStrlen(arg3) ;
            if(ii+index < MAXBUF)
            {
                meStrncpy(evalResult+index,arg3,ii) ;
                index += ii ;
                if(fnum == UFLINS)
                    s2 = s1-1 ;
                ii = meStrlen(s2) ;
                if(ii+index < MAXBUF)
                {
                    meStrcpy(evalResult+index,s2) ;
                    index += ii ;
                }
            }
            evalResult[index] = '\0' ;
            return evalResult ;
        }
    case UFFIND:
        {
            if(!fileLookup(arg1,arg2,meFL_CHECKDOT|meFL_USESRCHPATH,evalResult))
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
            uint8 *ss;
            uint8 cc;
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
            uint8 *ss;
            uint8 cc;
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
                uint8 cc;
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
    case UFSET:
        {
            if(setVar(arg1,arg2,regs) != TRUE)
                return abortm ;
            meStrcpy (evalResult, arg2);
            return evalResult;
        }
    case UFDEC:
        {
            varVal = meItoa(meAtoi(varVal) - meAtoi(arg2));
            if(setVar(arg1,varVal,regs) != TRUE)
                return abortm ;
            return varVal ;
        }
    case UFINC:
        {
            varVal = meItoa(meAtoi(varVal) + meAtoi(arg2));
            if(setVar(arg1,varVal,regs) != TRUE)
                return abortm ;
            return varVal ;
        }
    case UFPDEC:
        {
            /* cannot copy into evalResult here cos meItoa uses it */
            meStrcpy(arg3,varVal) ;
            varVal = meItoa(meAtoi(varVal) - meAtoi(arg2));
            if(setVar(arg1,varVal,regs) != TRUE)
                return abortm ;
            meStrcpy(evalResult,arg3) ;
            return evalResult ;
        }
    case UFPINC:
        {
            /* cannot copy into evalResult here cos meItoa uses it */
            meStrcpy(arg3,varVal) ;
            varVal = meItoa(meAtoi(varVal) + meAtoi(arg2));
            if(setVar(arg1,varVal,regs) != TRUE)
                return abortm ;
            meStrcpy(evalResult,arg3) ;
            return evalResult ;
        }
#if REGSTRY
    case UFREGISTRY:
        {
            REGHANDLE reg;
            uint8 *p = arg3;
            /*
             * Read a value from the registry.
             * 
             * Arg1 is the path,
             * Arg2 is the subkey
             * Arg3 is the default value
             * 
             * If the node cannot be found then return the result.
             */
            if (((reg = regFind (NULL, arg1)) != NULL) &&
                ((reg = regFind (reg,  arg2)) != NULL))
                p = regGetString (reg, p) ;
            /* Set up the result string */
            if (p == NULL)
                evalResult [0] = '\0';
            else
                meStrcpy (evalResult, p);
            return evalResult;
        }
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
            
            uint8 *p;                            /* Temp char pointer */
            uint8  c;                            /* Current char */
            uint8 *s = arg1;                     /* String pointer */
            int   count, nn, index=0 ;          /* Count of repeat */
            
            while ((c = *s++) != '\0') 
            {
                if (c == '%')                   /* Check for control */
                {
                    p = s-1 ;
                    count = 1 ;
get_flag:
                    switch((c = *s++))
                    {
                    case 'n':                   /* Replicate string */
                        if (macarg (arg2) != TRUE) 
                            return abortm;
                        count *= meAtoi(arg2);
                    case 's':                   /* String */
                        if (macarg (arg2) != TRUE)
                            return abortm;
                        nn = meStrlen(arg2) ;
                        while(--count >= 0)
                        {
                            if(index+nn >= MAXBUF)
                                /* break if we dont have enough space */
                                break ;
                            meStrcpy(arg3+index,arg2) ;
                            index += nn ;
                        }
                        break;
                    case 'X':                   /* Hexadecimal */
                    case 'x':                   /* Hexadecimal */
                    case 'd':                   /* Decimal */
                    case 'o':                   /* Octal */
                        if (macarg (arg2) != TRUE) 
                            return abortm;
                        nn = meAtoi(arg2) ;
                        c = *s ;
                        *s = '\0' ;
                        if(index < MAXBUF-16)
                            /* Only do it if we have enough space */
                            index += sprintf((char *)arg3+index,(char *)p,nn) ;
                        *s = c ;
                        break;
                    default:
                        if(isDigit(c))
                        {
                            count = c - '0' ;
                            while((c = *s++),isDigit(c)) 
                                count = (count*10) + (c-'0') ;
                            s-- ;
                            goto get_flag ;
                        }
                        else if(index >= MAXBUF)
                            break ;
                        else
                            arg3[index++] = c;  /* Just a literal char - pass in */
                    }
                }
                else if(index >= MAXBUF)
                    break ;
                else
                    arg3[index++] = c;          /* Just a literal char - pass in */
            }
            arg3[index] = '\0';                 /* make sure sting terminated */
            meStrcpy(evalResult,arg3) ;
            return evalResult ;
        }
    case UFSTAT:
        {
            static uint8 typeRet[] = "NRDXHF" ;
            meSTAT stats ;
            int    ftype ;
            /*
             * 0 -> N - If file is nasty
             * 1 -> R - If file is regular
             * 2 -> D - If file is a directory
             * 3 -> X - If file doesn't exist
             * 4 -> H - If file is a HTTP url
             * 5 -> F - If file is a FTP url
             */
            ftype = getFileStats(arg2,1,&stats,evalResult) ;
            switch(arg1[0])
            {
            case 'a':
                /* absolute name - check for symbolic link */
                if(*evalResult == '\0')
                    fileNameCorrect(arg2,evalResult,NULL) ;
                /* if its a directory check there's an ending '/' */
                if(ftype == meFILETYPE_DIRECTORY)
                {
                    uint8 *ss=evalResult+meStrlen(evalResult) ;
                    if(ss[-1] != DIR_CHAR)
                    {
                        ss[0] = DIR_CHAR ;
                        ss[1] = '\0' ;
                    }
                }
                return evalResult ;
                
            case 'd':
                /* file date stamp */
                ftype = stats.stmtime ;
                break ;
                
            case 'r':
                /* File read permission */
                /* If its a nasty or it doesn't exist - no */
                if((ftype == meFILETYPE_NASTY) || (ftype == meFILETYPE_NOTEXIST))
                    return meLtoa(0) ;
                /* If its a url then do we support url ? */
#if URLAWAR
                if((ftype == meFILETYPE_HTTP) || (ftype == meFILETYPE_FTP))
#ifdef _URLSUPP
                    return meLtoa(1) ;
#else
                    return meLtoa(0) ;
#endif
#endif
                /* If its a DOS/WIN directory - yes */
#if (defined _DOS) || (defined _WIN32)
                if(ftype == meFILETYPE_DIRECTORY)
                    return meLtoa(1) ;
#endif
                /* normal test */
                return meLtoa(meStatTestRead(stats)) ;
                
            case 's':
                /* File size - return -1 if not a regular file */
                if(ftype == meFILETYPE_REGULAR)
                    ftype = stats.stsize ;
                else
                    ftype = -1 ;
                break ;
            case 't':
                /* File type - use look up table, see first comment */
                if((ftype == meFILETYPE_DIRECTORY) &&
#ifdef _UNIX
                   (stats.stmtime == -1))
#else
                   meTestDir(arg2))
#endif
                    ftype = meFILETYPE_NOTEXIST ;
                evalResult[0] = typeRet[ftype] ;
                evalResult[1] = '\0' ;
                return evalResult ;
            
            case 'w':
                /* File write permission */
                /* If nasty or url - no */
                if((ftype == meFILETYPE_NASTY)
#if URLAWAR
                   || (ftype == meFILETYPE_HTTP) || (ftype == meFILETYPE_FTP)
#endif
                   )
                    return meLtoa(0) ;
                /* if it doesnt exist or its an DOS/WIN directory - yes */
                if((ftype == meFILETYPE_NOTEXIST)
#if (defined _DOS) || (defined _WIN32)
                   || (ftype == meFILETYPE_DIRECTORY)
#endif
                   )
                    return meLtoa(1) ;
                return meLtoa(meStatTestWrite(stats)) ;
            
            case 'x':
                /* File execute permission */
                /* If nasty or doesnt exist, or url then no */
                if((ftype == meFILETYPE_NASTY) || (ftype == meFILETYPE_NOTEXIST) 
#if URLAWAR
                   || (ftype == meFILETYPE_HTTP) || (ftype == meFILETYPE_FTP)
#endif
                   )
                    return meLtoa(0) ;
#if (defined _DOS) || (defined _WIN32)
                /* If directory, simulate unix style execute flag and return yes */
                if(ftype == meFILETYPE_DIRECTORY)
                    return meLtoa(1) ;
                /* Must test for .exe, .com, .bat, .btm extensions etc */
                return meLtoa(meTestExecutable(arg2)) ;
#else
                /* Test the unix execute flag */
                return meLtoa((((stats.stuid == meUid) && (stats.stmode & S_IXUSR)) ||
                               ((stats.stgid == meGid) && (stats.stmode & S_IXGRP)) ||
                               (stats.stmode & S_IXOTH))) ;
#endif
            default:
                return abortm ;
            }                
            return meItoa(ftype) ;
        }
    }
    return abortm ;
}


/* numeric arg can overide prompted value */
int
unsetVariable(int f, int n)     /* Delete a variable */
{
    register meVARLIST  *varList ;      /* User variable pointer */
    register meVARIABLE *vptr, *prev;   /* User variable pointer */
    register int   vnum;                /* ordinal number of var refrenced */
    meREGISTERS *regs ;
    uint8 var[NSTRING] ;                 /* name of variable to fetch */
    uint8 *vv ;
    
    /* First get the variable to set.. */
    /* set this flag to indicate that the variable name is required, NOT
     * its value */
    alarmState |= meALARM_VARIABLE ;
    /* horrid global variable, see notes at definition */
    gmaLocalRegPtr = meRegCurr ;
    vnum = meGetString((uint8 *)"Unset variable", MLVARBL, 0, var, NSTRING) ;
    regs = gmaLocalRegPtr ;
    alarmState &= ~meALARM_VARIABLE ;
    if(vnum != TRUE)
        return vnum ;
    
    /* Check the legality and find the var */
    vv = var+1 ;
    vnum = getMacroTypeS(var) ; 
    if(vnum == TKVAR) 
        varList = &usrVarList ;
    else if(vnum == TKLVR)
    {        
        uint8 *ss ;
        BUFFER *bp ;
        
        if((ss=meStrrchr(vv,':')) != NULL)
        {
            *ss = '\0' ;
            bp = bfind(vv,0) ;
            *ss++ = ':' ;
            if(bp == NULL)
                /* not a command - abort */
                return mlwrite(MWABORT,(uint8 *)"[No such variable]");
            vv = ss ;
        }
        else
            bp = curbp ;
        varList = &(bp->varList) ;
    }
    else if(vnum == TKCVR)
    {
        uint8 *ss ;
        if((ss=meStrrchr(vv,'.')) != NULL)
        {
            int idx ;
            
            *ss = '\0' ;
            idx = decode_fncname(var+1,0) ;
            *ss++ = '.' ;
            if(idx < 0)
                return mlwrite(MWABORT,(uint8 *)"[No such variable]");
            varList = &(cmdTable[idx]->varList) ;
            vv = ss ;
        }
        else if((varList = regs->varList) == NULL)
            return mlwrite(MWABORT,(uint8 *)"[No such variable]");
    }
    else
        return mlwrite(MWABORT,(uint8 *)"[User variable required]");
    
    /*---   Check for existing legal user variable */
    prev = NULL ;
    vptr = varList->head ;
    while(vptr != NULL)
    {
        if(!(vnum=meStrcmp(vptr->name,vv)))
        {
            /* link out the variable to unset and free it */
            if(prev == NULL)
                varList->head = vptr->next ;
            else
                prev->next = vptr->next ;
            varList->count-- ;
            meNullFree(vptr->value) ;
            meFree(vptr) ;
            return TRUE ;           /* True exit */
        }
        if(vnum > 0)
            break ;
        prev = vptr ;
        vptr = vptr->next ;
    }
    
    /* If its not legal....bitch */
    return mlwrite(MWABORT|MWCLEXEC,(uint8 *)"[No such variable]") ;
}

int
descVariable(int f, int n)      /* describe a variable */
{
    uint8  var[NSTRING]; /* name of variable to fetch */
    uint8 *ss ;
    int    status ;
    
    /* first get the variable to describe */
    if((status = meGetString((uint8 *)"Show variable",MLVARBL,0,var,NSTRING)) != TRUE)
        return(status);
    if((ss = getval(var)) == NULL)
        return mlwrite(MWABORT,(uint8 *)"Unknown variable type") ;
    mlwrite(0,(uint8 *)"Current setting is \"%s\"", ss) ;
    return TRUE ;
} 


int
setVariable(int f, int n)       /* set a variable */
{
    register int   status ;         /* status return */
    uint8 var[NSTRING] ;            /* name of variable to fetch */
    uint8 value[MAXBUF] ;           /* value to set variable to */
    meREGISTERS *regs ;
    
    /* horrid global variable, see notes at definition */
    gmaLocalRegPtr = meRegCurr ;
    /* set this flag to indicate that the variable name is required, NOT
     * its value */
    alarmState |= meALARM_VARIABLE ;
    status = meGetString((uint8 *)"Set variable", MLVARBL, 0, var,NSTRING) ;
    alarmState &= ~meALARM_VARIABLE ;
    regs = gmaLocalRegPtr ;
    if(status != TRUE)
        return status ;
    /* get the value for that variable */
    if (f == TRUE)
        meStrcpy(value, meItoa(n));
    else if((status = meGetString((uint8 *)"Value", MLFFZERO, 0, value,MAXBUF)) != TRUE)
        return status ;
    
    return setVar(var,value,regs) ;
}

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
showVariable (BUFFER *bp, uint8 prefix, uint8 *name, uint8 *value)
{
    uint8 buf[MAXBUF+NSTRING+16] ;
    int len;
    
    if (value == NULL)
        value = errorm ;
    
    len = sprintf ((char *) buf, "    %c%s ", prefix, name);
    while (len < 35)
        buf[len++] = '.';
    buf[len++] = ' ';
    buf[len++] = '"';
    len = expandexp(-1,value,MAXBUF+NSTRING+16-2,len,buf,-1,NULL,meEXPAND_BACKSLASH|meEXPAND_FFZERO) ;
    buf[len++] = '"';
    buf[len] = '\0';
    addLineToEob(bp,buf);
}

/*
 * listVariables
 * List ALL of the variables held in the system.
 */

int
listVariables (int f, int n)
{
    meVARIABLE *tv ;
    WINDOW *wp ;
    BUFFER *bp ;
    uint8   buf[MAXBUF] ;
    int     ii ;
    
    if((wp = wpopup(BvariablesN,(BFND_CREAT|BFND_CLEAR|WPOP_USESTR))) == NULL)
        return FALSE ;
    bp = wp->w_bufp ;
    
    addLineToEob(bp,(uint8 *)"Register variables:");
    addLineToEob(bp,(uint8 *)"") ;
    
    buf[0] = 'g';
    buf[2] = '\0';
    for(ii = 0; ii<meNUMREG ; ii++)
    {
        buf[1] = (int)('0') + ii;
        showVariable (bp, '#', buf,meRegHead->reg[ii]);
    }
    addLineToEob(bp,(uint8 *)"") ;
    sprintf((char *)buf,"Buffer [%s] variables:", curbp->b_bname);
    addLineToEob(bp,buf);
    addLineToEob(bp,(uint8 *)"") ;
    tv = curbp->varList.head ;
    while(tv != NULL)
    {
        showVariable (bp, ':', tv->name,tv->value);
        tv = tv->next ;
    }
    
    addLineToEob(bp,(uint8 *)"") ;
    addLineToEob(bp,(uint8 *)"System variables:");
    addLineToEob(bp,(uint8 *)"") ;
    for (ii = 0; ii < NEVARS; ii++)
        showVariable (bp, '$', envars [ii],gtenv(envars [ii]));
    
    addLineToEob(bp,(uint8 *)"") ;
    addLineToEob(bp,(uint8 *)"Global variables:");
    addLineToEob(bp,(uint8 *)"") ;
    for (tv=usrVarList.head ; tv != NULL ; tv = tv->next)
        showVariable (bp, '%', tv->name,tv->value);
    
    bp->b_dotp = lforw(bp->b_linep);
    bp->b_doto = 0 ;
    bp->line_no = 0 ;
    meModeClear(bp->b_mode,MDEDIT) ;    /* don't flag this as a change */
    meModeSet(bp->b_mode,MDVIEW) ;      /* put this buffer view mode */
    resetBufferWindows(bp) ;            /* Update the window */
    mlerase(MWCLEXEC);                  /* clear the mode line */
    return TRUE ;
}
