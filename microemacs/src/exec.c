/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * exec.c - Command execution.
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
 * Synopsis:    Command execution.
 * Authors:     Daniel Lawrence, Jon Green & Steven Phillips
 * Description:
 *     This file is for functions dealing with execution of
 *     commands, command lines, buffers, files and startup files
 */

#define __EXECC                 /* Define program name */

#include "emain.h"
#include "efunc.h"
#include "eskeys.h"
#include "evar.h"
#include "evers.h"

int relJumpTo;

/* token:       chop a token off a string
 * return a pointer past the token
 *
 * src - source string,
 * tok - destination token string (must be meTOKENBUF_SIZE_MAX in size, 
 * returning a string no bigger than meBUF_SIZE_MAX with the \0)
 */
meUByte *
token(meUByte *src, meUByte *tok)
{
    register meUByte cc, *ss, *dd, *tokEnd ;
    meUShort key;
    
    dd = tok;
    ss = src;
    cc = *ss++;
#if MEOPT_BYTECOMP
    if(cc & BCLEAD)
    {
        if((cc & BCSTRF) == 0)
        {
            /* 2 byte code 0xFTII - F = 0x80; T = TKENV,TKFUN,TKDIR; 0xII = index  */
            *dd++ = cc;
            *dd++ = *ss++;
            *dd = '\0';
        }
        else
        {
            /* lengthed-str, 0xFLSSSS... 0xFLLLSSSS... F = 0xC0; L = 0 to 0x1F length; or F = 0xE0; L = 0x20 to 0x0FFF;  SSS.. = String */ 
            int len = (cc & BCSLLM);
            if(cc & BCSTRT)
            {
                if(((cc=*ss++) < '0') || (cc > 0xaf))
                {
                    *dd = 0;
                    return ss;
                }
                len = (len << 7) | (cc - '0');
            }
            memcpy(dd,ss,len);
            dd[len] = '\0';
            ss += len;
        }
        return ss;
    }
#endif    
    if((cc == ' ') || (cc == '\t'))
    {
        /*---       First scan past any whitespace in the source string */
        while(((cc = *ss++) == ' ') || (cc == '\t'))
            ;
    }
    /* note tokEnd is set to tok+bSize-5 to leave room for a special key and a terminating \0 */
    tokEnd = dd + meTOKENBUF_SIZE_MAX-5 ;
    
    /*---       Scan through the source string */
    if(cc == '"')
    {
        *dd++ = '"' ;
        while((cc=*ss++) != '"')
        {
            if(cc == '\0') 
            {
                ss-- ;
                break ;
            }
            else if(cc == '\\')
            {
                /* Process special characters */
                switch ((cc = *ss++))
                {
                case '\0':
                    /* this probably should be an error */
                    *dd = 0;
                    ss-- ;
                    return ss ;
                    
                case 'B':
                    /* backspace key - replace with backspace */
                    key = ME_SPECIAL|SKEY_backspace ;
                    goto quote_spec_key1 ;
                case 'C':
                    /* Control key - \C? */
                    if((*dd++ = *ss++ - '@') == meCHAR_LEADER)
                        *dd++ = meCHAR_TRAIL_LEADER ;
                    break;
                case 'D':
                    /* Delete key - replace with delete */
                    key = ME_SPECIAL|SKEY_delete ;
                    goto quote_spec_key1 ;
                case 'E':
                    /* Escape key - replace with esc */
                    key = ME_SPECIAL|SKEY_esc ;
                    goto quote_spec_key1 ;
                case 'H':
                    /* OSD hotkey */
                    *dd++ = meCHAR_LEADER ;
                    *dd++ = meCHAR_TRAIL_HOTKEY ;  
                    break;
                case 'I':
                    /* backward-delete-tab - replace with S-tab */
                    key = ME_SPECIAL|ME_SHIFT|SKEY_tab ;
                    goto quote_spec_key1 ;
                case 'L':
                    /* Exec-line special - replace with x-line key */
                    key = ME_SPECIAL|SKEY_x_line ;
                    goto quote_spec_key1 ;
                case 'N':
                    /* Return key - replace with return */
                    key = ME_SPECIAL|SKEY_return ;
                    goto quote_spec_key1 ;
                case 'P':
                    /* Go to set position, defined by \p - replace with \CXAP */
                    {
                        int ll = meStrlen(__cmdArray[CK_GOAMRK].name) ;
                        if((size_t) (dd+ll+9) <= (size_t) tokEnd)
                        {
                            *dd++ = meCHAR_LEADER ;
                            *dd++ = meCHAR_TRAIL_SPECIAL ;
                            *dd++ = (ME_SPECIAL|SKEY_x_command) >> 8 ;
                            *dd++ = (ME_SPECIAL|SKEY_x_command) & 0xff ;
                            meStrcpy(dd,__cmdArray[CK_GOAMRK].name) ;
                            dd += ll ;
                            *dd++ = meCHAR_LEADER ;
                            *dd++ = meCHAR_TRAIL_SPECIAL ;
                            *dd++ = (ME_SPECIAL|SKEY_return) >> 8 ;
                            *dd++ = (ME_SPECIAL|SKEY_return) & 0xff ;
                            *dd++ = meANCHOR_EXSTRPOS ;
                        }
                        break;
                    }
                case 'T':
                    /* Tab key - replace with tab */
                    key = ME_SPECIAL|SKEY_tab ;
                    goto quote_spec_key1 ;
                case 'X':
                    /* Exec-command special - replace with x-command key */
                    key = ME_SPECIAL|SKEY_x_command ;
quote_spec_key1:
                    *dd++ = meCHAR_LEADER ;
                    *dd++ = meCHAR_TRAIL_SPECIAL ;
                    *dd++ = key >> 8 ;
                    *dd++ = key & 0xff ;
                    break;
                case 'b':   *dd++ = 0x08; break;
                case 'd':   *dd++ = 0x7f; break;
                case 'e':   *dd++ = 0x1b; break;
                case 'f':   *dd++ = 0x0c; break;
                case 'g':   *dd++ = 0x07; break;
                case 'i':
                    *dd++ = meCHAR_LEADER ;
                    *dd++ = meCHAR_TRAIL_SPECIAL ;  
                    *dd++ = '@';
                    *dd++ = 'I' - '@';
                    break;
                case 'n':   *dd++ = 0x0a; break;
                case 'p':
                    {
                        int ll = meStrlen(__cmdArray[CK_SETAMRK].name) ;
                        if((size_t) (dd+ll+9) <= (size_t) tokEnd)
                        {
                            *dd++ = meCHAR_LEADER ;
                            *dd++ = meCHAR_TRAIL_SPECIAL ;
                            *dd++ = (ME_SPECIAL|SKEY_x_command) >> 8 ;
                            *dd++ = (ME_SPECIAL|SKEY_x_command) & 0xff ;
                            meStrcpy(dd,__cmdArray[CK_SETAMRK].name) ;
                            dd += ll ;
                            *dd++ = meCHAR_LEADER ;
                            *dd++ = meCHAR_TRAIL_SPECIAL ;
                            *dd++ = (ME_SPECIAL|SKEY_return) >> 8 ;
                            *dd++ = (ME_SPECIAL|SKEY_return) & 0xff ;
                            *dd++ = meANCHOR_EXSTRPOS ;
                        }
                        break;
                    }
                case 'r':   *dd++ = 0x0d; break;
                case 's':   
                    *dd++ = meCHAR_LEADER ;
                    *dd++ = meCHAR_TRAIL_SPECIAL ;  
                    break;
                case 't':   *dd++ = 0x09; break;
                case 'v':   *dd++ = 0x0b; break;
                case 'x':
                    cc = *ss ;
                    if(isXDigit(cc))
                    {
                        register meUByte c1 ;
                        c1 = *++ss ;
                        if(isXDigit(c1))
                        {
                            cc = (hexToNum(cc) << 4) | hexToNum(c1) ;
                            ss++ ;
                        }
                        else
                            cc = hexToNum(cc) ;
                        if(cc == 0)
                        {
                            *dd++ = meCHAR_LEADER ;
                            *dd++ = 0x01 ;
                        }
                        else if(cc == meCHAR_LEADER)
                        {
                            *dd++ = meCHAR_LEADER ;
                            *dd++ = meCHAR_TRAIL_LEADER ;
                        }
                        else
                            *dd++ = cc ;
                    }
                    break;
                case '{':
                    /* OSD start hilight */
                    *dd++ = meCHAR_LEADER ;
                    *dd++ = meCHAR_TRAIL_HILSTART ;  
                    break;
                case '}':
                    /* OSD stop hilight */
                    *dd++ = meCHAR_LEADER ;
                    *dd++ = meCHAR_TRAIL_HILSTOP ;  
                    break;
                    
                default:
                    *dd++ = cc;
                }
            }
            else if(cc == meCHAR_LEADER)
            {
                *dd++ = meCHAR_LEADER ;
                *dd++ = meCHAR_TRAIL_LEADER ;
            }
            else
                *dd++ = cc;
            if((size_t) dd > (size_t) tokEnd)
                /* reset dd into safe area and keep going cos we must
                 * finish parsing this token */
                dd = tokEnd ;
        }
    }
    else if((cc == '\0') || (cc == ';'))
        ss-- ;
    else
    {
        *dd++ = cc;
        while(!isSpace(cc=*ss++))
            *dd++ = cc;
        if(cc == '\0')
            ss-- ;
    }
    *dd = 0 ;
    return ss ;
}

/* meGetString
 * Get a string input, either from the macro line or from the user
 * prompt  - prompt to use if we must be interactive
 * option  - options for getstring
 * defnum  - Default history number
 * buffer  - buffer to put token into
 * size    - size of the buffer */
int
meGetString(meUByte *prompt, int option, int defnum, meUByte *buffer, int size)
{
    /* if we are not interactive, go get it! */
    if(clexec == meTRUE)
    {
        meUByte buff[meTOKENBUF_SIZE_MAX], *res, *ss, cc;
        
        /* grab token and advance past */
        ss = execstr ;
#if MEOPT_BYTECOMP
        if((cc=*ss) & BCLEAD)
        {
            res = ss+1;
            if(cc & BCSTRF)
            {
                /* lengthed-str, 0xFLSSSS... 0xFLLLSSSS... F = 0xC0; L = 0 to 0x1F length; or F = 0xE0; L = 0x20 to 0x0FFF;  SSS.. = String */ 
                int len = (cc & BCSLLM);
                if(cc & BCSTRT)
                    len = (len << 7) | (*res++ - '0');
                execstr = res+len;
                switch(getMacroType(*res))
                {
                case TKSTR:
                    len--;
                    res++;
                case TKCMD:
                case TKLIT:
                    ss = buffer ;
                    if(option & MLMACNORT)
                    {
                        while((*ss))
                        {
                            ss++ ;
                            size-- ;
                        }
                    }
                    if(len >= size)
                        len = size-1;
                    memcpy(ss,res,len);
                    ss[len] = '\0';
                    if(!(option & MLFFZERO))
                    {
                        while((cc=*ss++) != '\0')
                        {
                            if(cc == meCHAR_LEADER)
                            {
                                res = ss;
                                if(*ss++ != meCHAR_TRAIL_LEADER)
                                    res[-1] = '\0';
                                else
                                {
                                    while((cc=*ss++) != '\0')
                                    {
                                        if((cc == meCHAR_LEADER) && (*ss++ != meCHAR_TRAIL_LEADER))
                                            break;
                                        *res++ = cc;
                                    }
                                    *res = '\0' ;
                                }
                                break;
                            }
                        }
                    }
                    return meTRUE ;
                }
                memcpy(buff,res,len);
                buff[len] = '\0';
            }
            else
            {
                execstr += 2;
                if(cc == BCFUN)
                {
                    res = gtfun((*res)-'0',NULL);
                    goto chk_cpy_res;
                }
                buff[0] = cc;
                buff[1] = *res;
                buff[2] = '\0';
            }
        }
        else
#endif
            execstr = token(ss,buff);
        
        if((buff[0] == '@') && (buff[1] == 'm') && (buff[2] == 'x'))
        {
            cc = buff[3] ;
            execstr = token(execstr, buff);
            if(cc == 'a')
                execstr = ss;
            if(prompt == NULL)
                resultStr[0] = '\0';
            else
                meStrcpy(resultStr,prompt) ;
            if(lineExec(0, 1, buff) <= 0)
                return meABORT ;
            meStrncpy(buffer,resultStr,size-1) ;
            buffer[size-1] = '\0' ;
            return meTRUE ;
        }
        else if((buff[0] != '@') || (buff[1] != 'm') || (buff[2] != 'n'))
        {
            /* evaluate it */
            res = getval(buff) ;
#if MEOPT_BYTECOMP
chk_cpy_res:
#endif
            if(res != (meUByte *) abortm)
            {
                ss = buffer ;
                if(option & MLMACNORT)
                {
                    while((*ss))
                    {
                        ss++ ;
                        size-- ;
                    }
                }
                if(option & MLFFZERO)
                {
                    while((--size > 0) && ((cc = *res++) != '\0'))
                        *ss++ = cc ;
                }
                else
                {
                    for(; ((--size) > 0) && ((cc = *res++) != '\0') ; )
                    {
                        if((cc == meCHAR_LEADER) && ((cc = *res++) != meCHAR_TRAIL_LEADER))
                            break ;
                        *ss++ = cc ;
                    }
                }
                *ss = '\0' ;
                return meTRUE ;
            }
            if(((buff[0] != '\0') && (buff[0] != ';')) || (option & MLEXECNOUSER))
            {
                *buffer = '\0' ;
                return meFALSE ;
            }
        }
        /* if @mna (get all input from user) then rewind the execstr */
        else if(buff[3] == 'a')
            execstr = ss ;
    }
    if(prompt == NULL)
    {
        *buffer = '\0' ;
        return ctrlg(meFALSE,1) ;
    }
    return meGetStringFromUser(prompt, option, defnum, buffer, size) ;
}

int
macarg(meUByte *tok)               /* get a macro line argument */
{
    int savcle;                 /* buffer to store original clexec */
    int status;
    
    savcle = clexec;            /* save execution mode */
    clexec = meTRUE;              /* get the argument */
    status = meGetString(NULL,MLNOHIST|MLFFZERO,0,tok,meBUF_SIZE_MAX) ;
    clexec = savcle;            /* restore execution mode */
    
    return status ;
}

#if KEY_TEST
/*---   Little function to test the  alphabetic state of the table.  Invoked
   via "!test" primative.  */

void
fnctest(meBuffer *bp)
{
    meBind *ktp;                       /* Keyboard character array */
    meCommand *cmd;                    /* Names pointer */
#if MEOPT_CMDHASH
    meUInt key;
#endif
    meUByte outseq[12];
    int count=0,ix,ii,jj,fn;                 /* Counter of errors */
    
    addLineToEob(bp,(meUByte *) "");
    ii = sizeof(size_t);
#ifdef _64BIT
    if(ii != 8)
    {
        sprintf((char *) resultStr,"Compile Error: Pointer size is %d, should be 8",ii);
#else
    if(ii != 4)
    {
        sprintf((char *) resultStr,"Compile Error: Pointer size is %d, should be 4",ii);
#endif
        addLineToEob(bp,resultStr);
        count++;
    }
#if MEOPT_CMDHASH
    /* test the command hash table */
    for (ii=0; ii < CK_MAX; ii++)
    {
        meStringHash(getCommandName(ii),key);
        cmd = cmdHash[key&(cmdHashSize-1)];
        while((cmd != NULL) && (cmd != cmdTable[ii]))
            cmd = cmd->hnext;
        if(cmd == NULL)
        {
            count++;
            sprintf((char *) resultStr,"cmdTbl Error: [%s] should be in position %d",getCommandName(ii),(key&(cmdHashSize-1)));
            addLineToEob(bp,resultStr);
        }
        else if(cmd->hash != key)
        {
            count++;
            sprintf((char *) resultStr,"cmdHash Error: [%s] hash should be 0x%08x",getCommandName(ii),key);
            addLineToEob(bp,resultStr);
        }
    }
#endif
    
    /* test the command alphabetically ordered list */
    cmd = cmdHead ;
    while(cmd->anext != NULL)
    {
        if(meStrcmp(cmd->anext->name,cmd->name) < 0)
        {
            count++;
            sprintf((char *) resultStr,"cmdHead Error: [%s] should be before [%s]",cmd->anext->name,cmd->name) ;
            addLineToEob(bp,resultStr);
        }
        cmd = cmd->anext;
    }
    /* Scan through the key table looking for the character code.  If found
     * then return the index into the names table.  */
    
    for (ktp = &keytab[0]; ktp[1].code != 0; ktp++)
    {
        if (ktp->code > ktp[1].code)
        {
            count++;
            meGetStringFromKey(ktp->code,outseq);
            sprintf((char *) resultStr,"[%s] key out of place",outseq);
            addLineToEob(bp,resultStr);
        }
    }
    /* now check the user funcs */
    ix = 0;
    while(funcNames[++ix] != NULL)
    {
        ii = funcNames[ix][0] - 'a';
        jj = (((int) funcNames[ix][0]) << 16);
        if((ii = funcOffst[ii]) == 0)
        {
            count++;
            sprintf((char *) resultStr,"funcLookup Error: Offset for function [%s] is zero",funcNames[ix]);
            addLineToEob(bp,resultStr);
        }
        else if((funcHashs[ii] & 0x0ff0000) != jj)
        {
            count++;
            sprintf((char *) resultStr,"funcLookup Error: Offset for function [%s] is wrong, points to %s",funcNames[ix],funcNames[ii]);
            addLineToEob(bp,resultStr);
        }
        else
        {
            jj |= (((int) funcNames[ix][1]) << 8) | ((int) funcNames[ix][2]);
            if((fn=funcHashs[ii]) != jj)
            {
                if(fn < jj)
                {
                    while((fn=funcHashs[++ii]) < jj)
                        ;
                    if(fn != jj)
                    {
                        count++;
                        sprintf((char *) resultStr,"funcLookup Error: Failed to ffind function [%s], hash 0x%x",funcNames[ix],jj);
                        addLineToEob(bp,resultStr);
                    }
                }
                else
                {
                    while((fn=funcHashs[--ii]) > jj)
                        ;
                    if(fn != jj)
                    {
                        count++;
                        sprintf((char *) resultStr,"funcLookup Error: Failed to bfind function [%s], hash 0x%x",funcNames[ix],jj);
                        addLineToEob(bp,resultStr);
                    }
                }
            }
        }
    }
    sprintf((char *) resultStr,"\nTest result: %d Error(s) Detected",count);
    addLineToEob(bp,resultStr);
}
#endif

static int
domstore(meUByte *cline)
{
    register meUByte *ss,cc;
    
    if(mcStore == 2)
    {
#if MEOPT_EXTENDED
        /* check to see if this line ends definition */
        if(meStrncmp(cline, "!ehe",4) == 0)
        {
            helpBufferReset(lpStoreBp) ;
            mcStore = 0 ;
            lpStore = NULL;
            return meTRUE ;
        }
#endif
        if(addLine(lpStore,cline) <= 0)
            return meFALSE ;
        lpStoreBp->lineCount++ ;
        return meTRUE ;
    }
    cc = *cline;
#if MEOPT_BYTECOMP
    if(cc & BCLEAD)
    {
        if((cc == BCDIR) && (cline[1] == ('0'+DREMACRO)) && !execlevel--)
        {
            mcStore = meFALSE;
            lpStore = NULL;
            execlevel = 0 ;
            return meTRUE ;
        }
        ss = cline+1;
        if((cc == (BCLEAD|BCSTRF|1)) && isDigit(*ss))
        {
            cc = *++ss;
            ss++;
        }
        if((cc == (BCLEAD|BCSTRF|12)) && !memcmp(ss,getCommandName(CK_DEFMAC),12))
            execlevel++ ;
        return addLine(lpStore,cline) ;
    }
#endif
    /* eat leading spaces */
    while((cc == ' ') || (cc == '\t'))
        cc = *++cline ;
    
    /* dump comments and empties here */
    if((cc == ';') || (cc == '\0'))
        return meTRUE ;
    /* check to see if this line turns macro storage off */
    if((cc == '!') && !meStrncmp(cline+1, "ema", 3) && !execlevel--)
    {
        mcStore = meFALSE;
        lpStore = NULL;
        execlevel = 0 ;
        return meTRUE ;
    }
    ss = cline ;
    if(isDigit(cc))
    {
        do
            cc = *++ss ;
        while(isDigit(cc)) ;
        while((cc == ' ') || (cc == '\t'))
            cc = *++ss ;
    }
    if((cc == 'd') && !meStrncmp(ss,getCommandName(CK_DEFMAC),12) &&
       (((cc=ss[12]) == ' ') || (cc == '\t') || (cc == ';') || (cc == '\0')))
        execlevel++ ;
    return addLine(lpStore,cline) ;
}


/*
   docmd - take a passed string as a command line and translate
   it to be executed as a command. This function will be
   used by execute-line and by all source and
   startup files.
   
   format of the command line is:
   
   {# arg} <command-name> {<argument string(s)>}
   
   Directives start with a "!" and include:
   
   !endm           End a macro
   !if (cond)      conditional execution
   !else
   !endif
   !return         Return (terminating current macro)
   !goto <label>   Jump to a label in the current macro
   
   Line Labels begin with a "*" in column 1, like:
   
 *LBL01
 */
/* cline        command line to execute */
static int
docmd(meUByte *cline, register meUByte *tkn)
{
    register int ii;   /* index to function to execute */
    int f;             /* default argument flag */
    int n;             /* numeric repeat value */
    meUByte cc;        /* Character */
    int  status;       /* return status of function */
    int  nmacro;       /* run as it not a macro? */
    
    nmacro = meFALSE;
    execstr = cline;    /* and set this one as current */
    meRegCurr->force = 0 ;
        
    cc = *execstr;
try_again:
#if MEOPT_BYTECOMP
    if(cc & BCLEAD)
    {
        meUByte dd;
        /* byte compiled line */
        execstr++;
        if(cc == BCDIR)
        {
            int dirType;
            status = (*execstr++)-'0';
            dirType = dirTypes[status];
            if(execlevel == 0)
            {
bcelif_jump:
                if(dirType & DRFLAG_TEST)
                {
                    if(macarg(tkn) <= 0)
                        return meFALSE;
                    if(!meAtol(tkn))
                    {
                        /* if DRTGOTO or DRTJUMP and the test failed, we dont
                         * need the argument and if DRIIF we don't want the switch */
                        dirType &= ~(DRFLAG_ARG|DRFLAG_SWITCH);
                        execlevel += (dirType & DRFLAG_AMSKEXECLVL);
                        status |= DRTESTFAIL;
                    }
                }
                else if(dirType & DRFLAG_ASGLEXECLVL)
                    /* DRELIF DRELSE */
                    execlevel += 2 ;
                if(dirType & DRFLAG_ARG)
                {
                    if(meGetString(NULL,MLNOHIST|MLFFZERO|MLEXECNOUSER,0,tkn,meBUF_SIZE_MAX) <= 0)
                    {
                        if(!(dirType & DRFLAG_OPTARG))
                            return meFALSE;
                        f = meFALSE;
                        n = 1;
                    }
                    else if(dirType & DRFLAG_NARG)
                    {
                        f = meTRUE;
                        n = meAtoi(tkn);
                        if(dirType & DRFLAG_JUMP)
                            relJumpTo = n;
                    }
                }
                if(!(dirType & DRFLAG_SWITCH))
                    return status;
                
                /* DRFORCE, DRNMACRO, DRABORT, DRBELL, DRIIF, DRRETURN */
                switch(status)
                {
                case DRABORT:
                    if(f)
                        TTdoBell(n);
                    return meFALSE;
                case DRBELL:
                    TTdoBell(n);
                    return meTRUE;
                case DREMACRO:
                    return mlwrite(MWABORT|MWWAIT,(meUByte *)"[Unexpected !emacro]");
                case DRFORCE:
                    (meRegCurr->force)++;
                    cc = *execstr;
                    goto try_again;
                case DRIIF:
                    cc = *execstr;
                    goto try_again;
                case DRNMACRO:
                    nmacro = meTRUE;
                    cc = *execstr;
                    goto try_again;
                case DRRETURN:
                    /* Stop the debugger kicking in on a !return, a macro doing !return 0 is okay */
                    meRegCurr->force = 1;
                    return (n) ? DRRETURN:meFALSE;
                }
            }
            else if(status == DRBREAK)
            {
                /* Stop DRBREAK effecting exec if execlevel != 0 */
                status = DRABORT;
                dirType = dirTypes[DRABORT];
            }
            if(dirType & DRFLAG_SMSKEXECLVL)
            {
                if(execlevel == 1)
                {
                    execlevel = 0 ;
                    if(status == DRELIF)
                    {
                        dirType = DRFLAG_ASGLEXECLVL|DRFLAG_TEST;
                        goto bcelif_jump;
                    }
                }
                else if(dirType & DRFLAG_SDBLEXECLVL)
                    execlevel -= 2;
            }
            else if(dirType & DRFLAG_AMSKEXECLVL)
                execlevel += 2;
            return meTRUE;
        }
        /* if not a directive and execlevel > 0 ignore it */
        if(execlevel)
            return meTRUE ;
        
        /* This must now be either be a. &func ... <cmd>; b. ## <cmd>; c. <var> <cmd>; d. <cmd> - the &func, ## & <cmd> will all be compiled (1st byte > 0x7f) */
        f = meFALSE;
        n = 1;
        if(cc == BCFUN)
        {
            meUByte *ns;
            if((ns = gtfun((*execstr++)-'0',NULL)) == abortm)
                return meFALSE;
            n = meAtoi(ns);
            f = meTRUE;
            cc = *execstr++;
            if((cc & (BCLEAD|BCSTRF)) == (BCLEAD|BCSTRF))
            {
                ii = (cc & BCSLLM);
                if(cc & BCSTRT)
                    ii = (ii << 7) | (*execstr++ - '0');
                dd = getMacroType(execstr[0]);
            }
        }
        else if((cc & (BCLEAD|BCSTRF)) == (BCLEAD|BCSTRF))
        {
            ii = (cc & BCSLLM);
            if(cc & BCSTRT)
                ii = (ii << 7) | (*execstr++ - '0');
            if((dd = getMacroType(execstr[0])) != TKCMD)
            {
                meUByte *vv;
                memcpy(tkn,execstr,ii);
                tkn[ii] = '\0';
                execstr += ii;
                if(dd == TKLIT)
                    n = meAtoi(tkn);
                else if((vv = getval(tkn)) == abortm)
                    return meFALSE;
                else
                    n = meAtoi(vv);
                f = meTRUE;
                cc = *execstr++;
                if((cc & (BCLEAD|BCSTRF)) == (BCLEAD|BCSTRF))
                {
                    ii = (cc & BCSLLM);
                    if(cc & BCSTRT)
                        ii = (ii << 7) | (*execstr++ - '0');
                    dd = getMacroType(execstr[0]);
                }
            }
        }
        if(((cc & (BCLEAD|BCSTRF)) != (BCLEAD|BCSTRF)) || (dd != TKCMD))
            /* not a command */
            return meFALSE;
        memcpy(tkn,execstr,ii);
        tkn[ii] = '\0';
        execstr += ii;
    }
    else
#endif
    {
        /* eat leading spaces */
        while((cc == ' ') || (cc == '\t'))
            cc = *++execstr;
        
        /* dump comments, empties and labels here */
        if((cc == ';') || (cc == '\0') || (cc == '*'))
            return meTRUE ;
        
        /* process argument */
        if((status=getMacroType(cc)) == TKDIR)
        {
            /* SWP 2003-12-22 Used to use the biChopFindString to look the
             * derivative up in a name table, but after profiling found this to be
             * the second biggest consumer of cpu and the function was 3rd (token
             * was the worst at 42%, the call to look-up the derivative was second
             * taking over 9% and docmd took 7%).
             * 
             * By changing this to an optimised set of if statements embedded in
             * docmd the time usage in my tests dropped from 4.52 sec to 2.73, i.e.
             * 40% time saving.
             * 
             * So yes this does look horrid but its fast!
             */
            int dirType ;
            meUByte c1, c2, c3;
            
            if(((c1 = *++execstr) <= ' ') || ((c2 = *++execstr) <= ' ') || ((c3 = *++execstr) <= ' '))
            {
                if((c1 != 'i') || (c2 != 'f') || (c3 > ' '))
                    return mlwrite(MWABORT|MWWAIT,(meUByte *)"[Unknown directive %s]",cline);
                status = DRIF ;
            }
            else
            {
                while((cc=*++execstr) > ' ')
                    ;
                status = -1 ;
                if(c1 > 'e')
                {
                    if(c1 < 'r')
                    {
                        if(c1 < 'i')
                        {
                            if(c1 == 'f')
                            {
                                if((c2 == 'o') && (c3 == 'r'))
                                    status = DRFORCE ;
                            }
                            else if(c1 == 'g')
                            {
                                if((c2 == 'o') && (c3 == 't'))
                                    status = DRGOTO ;
                            }
                        }
                        else if(c1 == 'i')
                        {
                            if((c2 == 'i') && (c3 == 'f'))
                                status = DRIIF ;
                        }
                        else if(c1 == 'j')
                        {
                            if((c2 == 'u') && (c3 == 'm'))
                                status = DRJUMP ;
                        }
                        else if(c1 == 'n')
                        {
                            if((c2 == 'm') && (c3 == 'a'))
                                status = DRNMACRO ;
                        }
                    }
                    else if(c1 == 'r')
                    {
                        if(c2 == 'e')
                        {
                            if(c3 == 'p')
                                status = DRREPEAT ;
                            else if(c3 == 't')
                                status = DRRETURN ;
                        }
                    }
                    else if(c1 > 't')
                    {
                        if(c1 == 'u')
                        {
                            if((c2 == 'n') && (c3 == 't'))
                                status = DRUNTIL ;
                        }
                        else if(c1 == 'w')
                        {
                            if((c2 == 'h') && (c3 == 'i'))
                                status = DRWHILE ;
                        }
                    }
                    else if(c1 == 't')
                    {
                        if((c2 == 'g') && (c3 == 'o'))
                            status = DRTGOTO ;
                        if((c2 == 'j') && (c3 == 'u'))
                            status = DRTJUMP ;
                    }
                }
                else if(c1 == 'e')
                {
                    if(c2 > 'l')
                    {
                        if((c2 == 'n') && (c3 == 'd'))
                            status = DRENDIF ;
                        else if((c2 == 'm') && (c3 == 'a'))
                            status = DREMACRO ;
                    }
                    else if(c2 == 'l')
                    {
                        if(c3 == 'i')
                            status = DRELIF ;
                        else if(c3 == 's')
                            status = DRELSE ;
                    }
                }
                else if(c1 > 'b')
                {
                    if((c2 == 'o') && (c3 == 'n'))
                        status = (c1 == 'c') ? DRCONTIN:DRDONE;
                }
                else if(c1 == 'b')
                {
                    if((c2 == 'r') && (c3 == 'e'))
                        /* Stop DRBREAK effecting exec if execlevel != 0 */
                        status = (execlevel == 0) ? DRBREAK:DRABORT;
                    else if((c2 == 'e') && (c3 == 'l'))
                        status = DRBELL;
                }
                else if((c1 == 'a') && (c2 == 'b') && (c3 == 'o'))
                    status = DRABORT;
                
                if(status < 0)
                    return mlwrite(MWABORT|MWWAIT,(meUByte *)"[Unknown directive %s]",cline);
            }        
            dirType = dirTypes[status] ;
            
            if(execlevel == 0)
            {
elif_jump:
                if(dirType & DRFLAG_TEST)
                {
                    if(macarg(tkn) <= 0)
                        return meFALSE ;
                    if(!meAtol(tkn))
                    {
                        /* if DRTGOTO or DRTJUMP and the test failed, we dont
                         * need the argument and if DRIIF we don't want the switch */
                        dirType &= ~(DRFLAG_ARG|DRFLAG_SWITCH) ;
                        execlevel += (dirType & DRFLAG_AMSKEXECLVL) ;
                        status |= DRTESTFAIL ;
                    }
                }
                else if(dirType & DRFLAG_ASGLEXECLVL)
                    /* DRELIF DRELSE */
                    execlevel += 2 ;
                if(dirType & DRFLAG_ARG)
                {
                    if(meGetString(NULL,MLNOHIST|MLFFZERO|MLEXECNOUSER,0,tkn,meBUF_SIZE_MAX) <= 0)
                    {
                        if(!(dirType & DRFLAG_OPTARG))
                            return meFALSE ;
                        f = meFALSE;
                        n = 1 ;
                    }
                    else if(dirType & DRFLAG_NARG)
                    {
                        f = meTRUE;
                        n = meAtoi(tkn) ;
                        if(dirType & DRFLAG_JUMP)
                            relJumpTo = n ;
                    }
                }
                if(!(dirType & DRFLAG_SWITCH))
                    return status ;
                
                /* DRFORCE, DRNMACRO, DRABORT, DRBELL, DRIIF, DRRETURN */
                switch(status)
                {
                case DRABORT:
                    if(f)
                        TTdoBell(n) ;
                    return meFALSE ;
                case DRBELL:
                    TTdoBell(n) ;
                    return meTRUE ;
                case DREMACRO:
                    return mlwrite(MWABORT|MWWAIT,(meUByte *)"[Unexpected !emacro]");
                case DRFORCE:
                    (meRegCurr->force)++ ;
                    while((cc == ' ') || (cc == '\t'))
                        cc = *++execstr;
                    goto try_again;
                case DRIIF:
                    while(((cc=*execstr) == ' ') || (cc == '\t'))
                        execstr++;
                    goto try_again;
                case DRNMACRO:
                    nmacro = meTRUE;
                    while((cc == ' ') || (cc == '\t'))
                        cc = *++execstr;
                    goto try_again;
                case DRRETURN:
                    /* Stop the debugger kicking in on a !return, a macro doing !return 0 is okay */
                    meRegCurr->force = 1 ;
                    return (n) ? DRRETURN:meFALSE ;
                }
            }
            
            if(dirType & DRFLAG_SMSKEXECLVL)
            {
                if(execlevel == 1)
                {
                    execlevel = 0 ;
                    if(status == DRELIF)
                    {
                        dirType = DRFLAG_ASGLEXECLVL|DRFLAG_TEST ;
                        goto elif_jump ;
                    }
                }
                else if(dirType & DRFLAG_SDBLEXECLVL)
                    execlevel -= 2 ;
            }
            else if(dirType & DRFLAG_AMSKEXECLVL)
                execlevel += 2 ;
            
            return meTRUE ;
        }
        /* if not a directive and execlevel > 0 ignore it */
        if(execlevel)
            return meTRUE ;
        
        /* get the current token and if not a CMD then treat as numeric arg n */
        execstr = token(execstr,tkn);
        if(status != TKCMD)
        {
            meUByte *tmp ;
            if((tmp = getval(tkn)) == abortm)
                return meFALSE;
            n = meAtoi(tmp) ;
            f = meTRUE;
            execstr = token(execstr,tkn);
            if(getMacroType(tkn[0]) != TKCMD)
                return meFALSE;
        }
        else
        {
            f = meFALSE;
            n = 1;
        }
    }
    
    /* and match the token to see if it exists */
    if ((ii = decode_fncname(tkn,0)) < 0)
        return meFALSE ;
    if(nmacro)
        clexec = meFALSE ;
    cmdstatus = (execFunc(ii,f,n) > 0) ;       /* call the function */
    clexec = meTRUE ;
    if(cmdstatus)
        status = meTRUE ;
    else if(TTbreakFlag)
    {
        if(meRegCurr->force > 1)
        {
            /* A double !force - macro says \CG is okay, 
             * remove all input (must remove the \CG) and rest flag
             */
            TTinflush() ;
            TTbreakFlag = 0 ;
            status = meTRUE ;
        }
        else
            status = meFALSE ;
    }
    else if(meRegCurr->force)
        status = meTRUE ;
    else
        status = meFALSE ;
    return status ;
}

/*      dobuf:  execute the contents of the buffer pointed to
   by the passed BP                                */
/* bp - buffer to execute */
static meLine *errorLine=NULL ;

static long
macroPrintError(meInt type, meLine *hlp, meUByte *macName)
{
    meLine *lp ;
    long  ln ;
    
    if(errorLine == NULL)
        /* Quit if there is no error line */
        return -1 ;
    
    lp=meLineGetNext(hlp) ;
    ln=0 ;
    while(lp != errorLine)
    {
        lp = meLineGetNext(lp) ;
        ln++ ;
    }
    mlwrite(MWABORT|MWWAIT,(meUByte *)"[%s:: %s executing %s, line %d]",errorLine->text,(type) ? "Halt":"Error",macName,ln+1) ;
    return ln ;
}

static int
dobuf(meLine *hlp)
{
#if MEOPT_DEBUGM
    static meUByte __dobufStr1[]="(C)ontinue, (S)tep, (V)ariable, <any>=next, (^G)Abort, (^L)redraw ? " ;
    meByte debug=0;
#endif
    meUByte *tline;               /* Temp line */
    int status;                   /* status return */
    meLine *lp;                   /* pointer to line to execute */
    meLine *lpFStk[DRLOOPMAX];    /* pointers to first line of loop */
    meLine *lpLStk[DRLOOPMAX];    /* pointers to last line of loop */
    meUByte lpCnt=0;
    
    clexec = meTRUE;              /* in cline execution */
    execstr = NULL ;
    execlevel = 0;                /* Reset execution level */
    lpFStk[0] = NULL;
    
    /* starting at the beginning of the buffer */
    lp = hlp->next;
    while (lp != hlp)
    {
        tline = lp->text;
#if MEOPT_DEBUGM
        /* if $debug == meTRUE, every line to execute
           gets echoed and a key needs to be pressed to continue
           ^G will abort the command */
        
        if((macbug & 0x0a) && (!execlevel || (macbug & 0x04) || ((execlevel == 1) && !meStrncmp(tline,"!el",3))))
        {
            meUByte dd, outline[meBUF_SIZE_MAX];   /* string to hold debug line text */
            meLine *tlp=hlp ;
            meUShort cc ;
            int lno=0 ;
            
            dd = macbug & ~0x80 ;
            /* force debugging off while we are getting input from the user,
             * if we don't and any macro is executed (idle-pick macro) then
             * ME will spin and crash!! */
            macbug = 0 ;
            
            /*---   Generate the debugging line for the 'programmer' !! */
            do 
                lno++ ;
            while ((tlp=meLineGetNext(tlp)) != lp)
                ;
            sprintf((char *)outline,"%s:%d:%d [%s] ? ",meRegCurr->commandName,lno,execlevel,tline) ;
loop_round2:
            mlwrite(MWSPEC,outline);        /* Write out the debug line */
            /* Cannot do update as if this calls a macro then
             * we will end up in an infinite loop, the best we can do is
             * call the screenUpdate function
             */
            screenUpdate(meTRUE,2-sgarbf) ;
            /* reset garbled status */
            sgarbf = meFALSE ;
            if((dd & 0x08) == 0)
            {
loop_round:
                /* and get the keystroke */
                if(((cc = meGetKeyFromUser(meFALSE,0,meGETKEY_SILENT|meGETKEY_SINGLE)) & 0xff00) == 0)
                    cc = toLower(cc) ;
                switch(cc)
                {
                case '?':
                    mlwrite(MWSPEC,__dobufStr1) ;       /* Write out the debug line */
                    goto loop_round ;
                case 'L'-'@':
                    goto loop_round2 ;
                case 'v':
                    {
                        meUByte mlStatusStore ;
                        mlStatusStore = frameCur->mlStatus ;
                        frameCur->mlStatus = 0 ;
                        clexec = meFALSE ;
                        descVariable(meFALSE,1) ;
                        clexec = meTRUE ;
                        frameCur->mlStatus = mlStatusStore ;
                        goto loop_round ;
                    }
                default:
                    if(cc == breakc)
                    {
                        /* Abort - Must exit the usual way so that macro
                         * storing and execlevel are corrected */
                        ctrlg(meFALSE,1) ;
                        status = meABORT ;
                        errorLine = lp ;
                        macbug = dd ;
                        goto dobuf_exit ;
                    }
                    debug = dd ;
                case 'c':
                    dd &= ~0x02 ;
                case 's':
                    break;
                }
            }
            macbug = dd ;
        }
#endif
        if(TTbreakTest(0))
        {
            mcStore = meFALSE;
            lpStore = NULL;
            execlevel = 0;
            status = ctrlg(meFALSE,1);
        }
        else
        {
            meUByte tkn[meTOKENBUF_SIZE_MAX] ;
            
            if(mcStore)
                status = domstore(tline) ;
            else
                status = docmd(tline,tkn) ;
#if     MEOPT_DEBUGM
            if(debug)
            {
                macbug = debug ;
                debug = 0 ;
            }
#endif
            switch(status)
            {
            case meABORT:
            case meFALSE:
            case meTRUE:
                break ;
                
            case DRGOTO:
            case DRTGOTO:
                {
                    register int linlen;        /* length of line to execute */
                    register meLine *glp;         /* line to goto */
                    
                    linlen = meStrlen(tkn) ;
                    glp = hlp->next;
                    while(glp != hlp)
                    {
                        if(*glp->text == '*' && (meStrncmp(glp->text+1,tkn,linlen) == 0) &&
                           isSpace(glp->text[linlen+1]))
                        {
                            lp = glp;
                            status = meTRUE;
                            break ;
                        }
                        glp = glp->next;
                    }
                    if(status != meTRUE)
                        status = mlwrite(MWABORT|MWWAIT,(meUByte *)"[No such label]");
                    break;
                }
            case DRREPEAT:                      /* REPEAT */
                if(lpCnt >= DRLOOPMAX)
                    status = mlwrite(MWABORT|MWWAIT,(meUByte *)"[Too many nested loops]");
                else
                {
                    if(lpFStk[lpCnt] != lp)
                    {
                        /* Save first line and reset end line */
                        lpFStk[lpCnt] = lp;
                        lpLStk[lpCnt++] = NULL;
                        lpFStk[lpCnt] = NULL;
                    }
                    else
                        lpCnt++;
                    status = meTRUE;
                }
                break;
            case DRUNTIL:                       /* meTRUE UNTIL */
                if(lpCnt)
                {
                    lpCnt--;
                    status = meTRUE;
                }
                else
                    status = mlwrite(MWABORT|MWWAIT,(meUByte *)"[Start of loop missing]");
                break;  
            case DRUNTILF:                      /* meFALSE UNTIL */
                if(lpCnt)
                {
                    lpCnt--;
                    /* looping, save last line */
                    lpLStk[lpCnt] = lp;
                    lp = lpFStk[lpCnt];
                    continue;
                }
                else
                    status = mlwrite(MWABORT|MWWAIT,(meUByte *)"[Start of loop missing]");
                break; 
            case DRCONTIN:
                if(lpCnt)
                {
                    lpCnt--;
                    lp = lpFStk[lpCnt];
                    continue;
                }
                status = mlwrite(MWABORT|MWWAIT,(meUByte *)"[Start of loop missing]");
                break;
                
            case DRBREAK:
                if(lpCnt)
                {
                    lpCnt--;
                    if(lpLStk[lpCnt] != NULL)
                        lp = lpLStk[lpCnt];
                    else
                    {
                        /* Nested if blocks make it difficult to find the loop end, use !cont code to get to start then treat as a failed !while */
                        lp = lpFStk[lpCnt];
                        execlevel += 2;
                    }
                    status = meTRUE;
                }
                else
                    status = mlwrite(MWABORT|MWWAIT,(meUByte *)"[Start of loop missing]");
                break;
            
            case DRDONE:        
                if(lpCnt)
                {
                    lpCnt--;
                    /* looping, save last line */
                    lpLStk[lpCnt] = lp;
                    lp = lpFStk[lpCnt];
                    continue;
                }
                status = mlwrite(MWABORT|MWWAIT,(meUByte *)"[Start of loop missing]");
                break;
                
            case DRWHILE:                       
                if(lpCnt >= DRLOOPMAX)
                    status = mlwrite(MWABORT|MWWAIT,(meUByte *)"[Too many nested loops]");
                else
                {
                    if(lpFStk[lpCnt] != lp)
                    {
                        /* Save first line and reset end line */
                        lpFStk[lpCnt] = lp;
                        lpLStk[lpCnt++] = NULL;
                        lpFStk[lpCnt] = NULL;
                    }
                    else
                        lpCnt++;
                    status = meTRUE;
                }
                break;
            
            case DRWHILEF:
                if((lpLStk[lpCnt] != NULL) && (lpFStk[lpCnt] == lp)) 
                {
                    lp = lpLStk[lpCnt];
                    execlevel -= 2;
                }
                status = meTRUE;
                break;
                
            case DRJUMP:
            case DRTJUMP:
                if(relJumpTo < 0)
                    for( ; relJumpTo < 0 ; relJumpTo++)
                        lp = lp->prev ;
                else
                    for( ; relJumpTo > 0 ; relJumpTo--)
                        lp = lp->next ;
                continue;
                
            case DRRETURN:      /* if it is a !RETURN directive...do so */
                status = meTRUE;
                goto dobuf_exit;
            default:
                status = meTRUE;
                
            }
        }
        /* check for a command error */
        if(status <= 0)
        {
            /* in any case set the buffer . */
            errorLine = lp;
#if     MEOPT_DEBUGM
            if(macbug & 0x01)
            {
                /* check if the failure is handled by a !force */
                meRegister *rr ;
                rr = meRegCurr ;
                for(;; rr = rr->prev)
                {
                    if(rr->force)
                        goto dobuf_exit ;
                    if(rr == meRegHead) 
                        break ;
                }
                macroPrintError(0,hlp,meRegCurr->commandName) ;
            }
#endif
            goto dobuf_exit ;
        }
#if     MEOPT_DEBUGM
        if(macbug & 0x80)
        {
            errorLine = lp;
            macroPrintError(1,hlp,meRegCurr->commandName) ;
            macbug &= ~0x80 ;
        }
#endif
        
        /* on to the next line */
        lp = lp->next;
    }
    if(mcStore)
        mlwrite(MWABORT|MWWAIT,(meUByte *)"[Missing !e%s termination]",(mcStore == 2) ? "help":"macro");
    else
        status = meTRUE ;
    
    /* exit the current function */
dobuf_exit:
    
    if(mcStore)
    {
        mcStore = 0 ;
        lpStore = NULL ;
        execlevel = 0 ;
        errorLine = hlp;
        status = meFALSE ;
    }
    
    return status ;
}


static int
donbuf(meLine *hlp, meVariable **varList, meUByte *commandName, int f, int n)
{
    register meRegister *rp;
    meUByte oldcle;
    int oldexec, status;
    
    /* Always make sure there is another child so functions calling execBufferFunc can use it */
    if((rp=meRegCurr->next)->next == NULL)
    {
        if(rp->depth > meMACRO_DEPTH_MAX)
        {
            /* macro recursion gone too deep, bail out.
             * attempt to force the break out by pretending C-g was pressed */
            TTbreakFlag = 1 ;
            return mlwrite(MWABORT|MWWAIT,(meUByte *)"[Macro recursion gone too deep]") ;
        }
        if((rp->next = meMalloc(sizeof(meRegister))) == NULL)
            return meABORT;
        rp->next->depth = rp->depth + 1;
        rp->next->prev = rp;
        rp->next->next = NULL;
    }
    /* save the arguments */
    oldcle = clexec;
    oldexec = execlevel;
    /* Setup these argument and move the register stack */
    rp->commandName = commandName;
    rp->execstr = execstr;
#if MEOPT_EXTENDED
    rp->varList = varList;
#endif
    rp->f = f;
    rp->n = n;
    rp->nextArg = 0xff;
    meRegCurr = rp;
    
    status = dobuf(hlp) ;
    
    /* restore old arguments & stack */
    meRegCurr = rp->prev ;
    execstr = rp->execstr ;
    execlevel = oldexec ;
    clexec = oldcle;
    
    cmdstatus = status ;
    return status ;
}

int
execFunc(register int index, int f, int n)
{
    register int status;
    
    if(index < CK_MAX)
    {
        thisflag = 0 ;
        status = cmdTable[index]->func(f,n) ;
        if(isComIgnore(index))
        {
            thisflag = lastflag ;
            return status ;
        }
        lastflag = thisflag;
        if(selhilight.uFlags)
        {
            if((status > 0) || isComSelIgnFail(index))
            {
                if(isComSelKill(index))
                {
                    selhilight.bp = NULL ;
                    selhilight.flags = 0 ;
                }
                else
                {
                    meWindow *cwp=frameCur->windowCur;
                    if(isComSelStart(index))
                    {
                        selhilight.bp = cwp->buffer;                  /* Select the current buffer */
                        selhilight.flags = SELHIL_ACTIVE|SELHIL_CHANGED ;
                    }
                    if(selhilight.flags & SELHIL_FIXED)
                    {
                        if(!(selhilight.flags & SELHIL_KEEP) && isComSelDelFix(index))
                            selhilight.flags &= ~SELHIL_ACTIVE ;
                    }
                    else if(selhilight.flags & SELHIL_ACTIVE)
                    {
                        if(isComSelSetMark(index) &&
                           ((selhilight.markOffset != cwp->markOffset) ||
                            (selhilight.markLineNo != cwp->markLineNo)))
                        {
                            selhilight.flags |= SELHIL_CHANGED ;
                            selhilight.markOffset = cwp->markOffset;
                            selhilight.markLineNo = cwp->markLineNo;
                        }
                        if(isComSelSetDot(index) &&
                           ((selhilight.dotOffset != cwp->dotOffset) ||
                            (selhilight.dotLineNo != cwp->dotLineNo)))
                        {
                            selhilight.flags |= SELHIL_CHANGED ;
                            selhilight.dotOffset = cwp->dotOffset;
                            selhilight.dotLineNo = cwp->dotLineNo;
                        }
                        if(isComSelSetFix(index))
                        {
                            selhilight.flags |= SELHIL_FIXED ;
                            if(selhilight.uFlags & SELHILU_KEEP)
                                selhilight.flags |= SELHIL_KEEP ;
                        }
                        if(isComSelStop(index))
                            selhilight.flags &= ~SELHIL_ACTIVE ;
                        if(selhilight.flags & SELHIL_CHANGED)
                        {
                            if(selhilight.bp->windowCount > 1)
                                meBufferAddModeToWindows(selhilight.bp, WFSELHIL);
                            else
                                cwp->updateFlags |= WFSELHIL;
                        }
                    }
                }
            }
            else if(!meRegCurr->force && ((selhilight.flags & (SELHIL_ACTIVE|SELHIL_KEEP)) == SELHIL_ACTIVE))
                selhilight.flags &= ~SELHIL_ACTIVE ;
        }
    }
    else
    {
        register meMacro *mac = getMacro(index) ;
#if MEOPT_EXTENDED
        register meLine *hlp ;
        
        hlp = mac->hlp ;
        if(hlp->flag & meMACRO_EXEC)
            status = donbuf(hlp,&mac->varList,mac->name,f,n) ;
        else
        {
            if(hlp->flag & meMACRO_FILE)
            {
                if(mac->fname != NULL)
                    execFile(mac->fname,0,1) ;
                else
                    execFile(mac->name,0,1) ;
                hlp = mac->hlp ;
                if(hlp->flag & meMACRO_FILE)
                    return mlwrite(MWABORT,(meUByte *)"[file-macro %s not defined]",mac->name);
            }
            hlp->flag |= meMACRO_EXEC;
            status = donbuf(hlp,&mac->varList,mac->name,f,n);
            if(mac->hlp != hlp)
                meLineLoopFree(hlp,1) ;
            else
                hlp->flag &= ~meMACRO_EXEC;
        }
#else
        status = donbuf(mac->hlp,NULL,mac->name,f,n) ;
#endif
    }
    
    return status ;
}

/* Execute the given command, but in a hidden way, i.e.
 * the 'key' is not recorded
 */
int
execFuncHidden(int keyCode, int index, meUInt arg)
{
    meUByte tf, lf, cs;
    int f, n, tc, ti, lc, li, sv;
#if MEOPT_EXTENDED
    int ii;
#endif
    
    cs = cmdstatus;
    lc = lastCommand;
    li = lastIndex;
    lf = lastflag;
    tc = thisCommand;
    ti = thisIndex;
    tf = thisflag;
    if((sv=(alarmState & meALARM_VARIABLE)))
        alarmState &= ~meALARM_VARIABLE;
    lastCommand = tc;
    lastIndex = ti;
    thisCommand = keyCode;
    thisIndex = index;
    if(arg != 0)
    {
        f = 1;
        n = (int) (arg + 0x80000000);
    }
    else
    {
        f = 0;
        n = 1;
    }
#if MEOPT_EXTENDED
    if(keyCode && ((ii=frameCur->windowCur->buffer->inputFunc) >= 0))
    {
        meUByte *ss, ff;
        /* set a force value for the execution as the macro is allowed to
         * fail and we don't want to kick in the macro debugging */
        ff = meRegHead->force;
        meRegHead->force = 1;
        if((execFunc(ii,f,n) > 0) ||
           ((ss=getUsrLclCmdVar((meUByte *)"status",cmdTable[ii]->varList)) == errorm) || meAtoi(ss))
            index = -1;
        meRegHead->force = ff;
    }
    if(index >= 0)
#endif
        index = execFunc(index,f,n);
    if(sv)
        alarmState |= meALARM_VARIABLE;
    thisflag = tf;
    thisIndex = ti;
    thisCommand = tc;
    lastflag = lf;
    lastIndex = li;
    lastCommand = lc;
    cmdstatus = cs;
    return index;
}

int
execBufferFunc(meBuffer *bp, int index, int flags, int n)
{
    meWindow *cwp=frameCur->windowCur;
    meBuffer *cbp=cwp->buffer;
    meSelection sh;
    meUByte tf, lf, cs;
    int cg, tc, ti, ret;
    
    cs = cmdstatus;
    tc = thisCommand;
    ti = thisIndex;
    tf = thisflag;
    lf = lastflag;
    thisIndex = index;
    if(flags & meEBF_USE_B_DOT)
    {
        cg = curgoal;    
        memcpy(&sh,&selhilight,sizeof(meSelection));
        if(bp == cbp)
        {
            /* drop an anchor and return back to it after */
            meAnchorSet(bp,meANCHOR_EXEC_BUFFER,cwp->dotLine,cwp->dotLineNo,cwp->dotOffset,1);
            cwp->dotLine = bp->dotLine;
            cwp->dotOffset = bp->dotOffset;
            cwp->dotLineNo = bp->dotLineNo;
        }
    }
    if(bp != cbp)
    {
        /* swap this buffer into the current window, execute the function and then swap back out */
        meUInt flag;
        meInt vertScroll;
        
        storeWindBSet(cbp,cwp);
        flag = cwp->updateFlags;
        vertScroll = cwp->vertScroll;
        
        cbp->windowCount--;
        cwp->buffer = bp;
        restoreWindBSet(cwp,bp);
#if MEOPT_EXTENDED
        isWordMask = bp->isWordMask;
#endif
        bp->windowCount++;
        
        ret = execFunc(index,(flags & meEBF_ARG_GIVEN),n);
        
        if((cwp = frameCur->windowCur)->buffer != cbp)
        {
            /* the func could have done some really whacky things like delete the buffer that was being displayed.
             * If it has avoid crashing by checking the buffer is still a buffer! */
            register meBuffer *tbp = bheadp;
            do {
                if(tbp == cbp)
                {
                    tbp = cwp->buffer;
                    tbp->windowCount--;
                    storeWindBSet(tbp,cwp);
                    cwp->buffer = cbp;
                    cbp->windowCount++;
                    cwp->vertScroll = vertScroll;
                    /* force a check on update, just incase the Func has done something behind our back */
                    cwp->updateFlags |= (flag|WFMOVEL|WFMAIN);
                    restoreWindBSet(cwp,cbp);
#if MEOPT_EXTENDED
                    isWordMask = cbp->isWordMask;
#endif
                    break;
                }
            } while((tbp=tbp->next) != NULL);
        }
    }
    else
    {
        ret = execFunc(index,(flags & meEBF_ARG_GIVEN),n);
        cwp = frameCur->windowCur;
    }
    if(flags & meEBF_USE_B_DOT)
    {
        if(bp == cwp->buffer)
        {
            /* return back to the anchor */
            if(meAnchorGet(bp,meANCHOR_EXEC_BUFFER) > 0)
            {
                cwp->dotLine = bp->dotLine;
                cwp->dotOffset = bp->dotOffset;
                cwp->dotLineNo = bp->dotLineNo;
                cwp->updateFlags |= WFMOVEL;
            }
            meAnchorDelete(bp,meANCHOR_EXEC_BUFFER);
        }
        selhilight.bp = NULL;
        selhilight.flags = 0;
        if(sh.flags != 0)
        {
            meBuffer *bp = bheadp;
            while((bp != NULL) && (bp != sh.bp))
                bp = bp->next;
            if((bp != NULL) && (sh.dotLineNo < bp->lineCount) && (sh.markLineNo < bp->lineCount))
                memcpy(&selhilight,&sh,sizeof(meSelection));
        }
        curgoal = cg;    
    }
    thisflag = tf;
    lastflag = lf;
    thisIndex = ti;
    thisCommand = tc;
    cmdstatus = cs;
    return ret;
}

/* executeBuffer:     Execute the contents of a buffer of commands    */
int
executeBuffer(int f, int n)
{
    register meBuffer *bp;                /* ptr to buffer to execute */
    register int s;                     /* status return */
    meVariable *varList=NULL;
    meUByte  bufn[meBUF_SIZE_MAX];                /* name of buffer to execute */
    meInt  ln ;
    
    /* find out what buffer the user wants to execute */
    if((s = getBufferName((meUByte *)"Execute buffer", 0, 1, bufn)) <= 0)
        return s ;
    
    /* find the pointer to that buffer */
    if ((bp=bfind(bufn, 0)) == NULL)
        return mlwrite(MWABORT,(meUByte *)"No such buffer");
    
    /* and now execute it as asked */
    if(((s = donbuf(bp->baseLine,&varList,bp->name,f,n)) <= 0) &&
       ((ln = macroPrintError(0,bp->baseLine,bufn)) >= 0))
    {
        /* the execution failed lets go to the line that caused the grief */
        bp->dotLine  = errorLine ;
        bp->dotLineNo = ln ;
        bp->dotOffset  = 0 ;
        /* look if buffer is showing */
        if(bp->windowCount)
        {
            register meWindow *wp;        /* ptr to windows to scan */
            
            meFrameLoopBegin() ;
            wp = loopFrame->windowList;
            while (wp != NULL)
            {
                if (wp->buffer == bp)
                {
                    /* and point it */
                    wp->dotLine = errorLine ;
                    wp->dotOffset = 0 ;
                    wp->dotLineNo = ln ;
                    wp->updateFlags |= WFMOVEL ;
                }
                wp = wp->next;
            }
            meFrameLoopEnd() ;
        }
    }
    /* free off any command variables */
    while(varList != NULL)
    {
        meVariable *nv=varList->next;
        meNullFree(varList->value);
        free(varList) ;
        varList = nv ;
    }
    return s ;
}

/* execFile: yank a file into a buffer and execute it
   if there are no errors, delete the buffer on exit */
/* Note for Dynamic file names
 * This ensures that nothing is done to fname
 */
int
execFile(meUByte *fname, int f, int n)
{
    meVariable   *varList=NULL;
    meUByte       fn[meBUF_SIZE_MAX] ;
    meLine        hlp ;
    register int  status=0 ;
    
    if((fname == NULL) || (fname[0] == '\0'))
    {
        fname = (meUByte *) ME_SHORTNAME ;
#ifdef _NANOEMACS
        /* don't print-out the '[Failed to load file ne.emf]' error for ne - who cares */ 
        status = 1 ;
#endif
    }
    hlp.next = &hlp ;
    hlp.prev = &hlp ;
    /* use a new buffer to ensure it doesn't mess with any loaded files */
    if(!fileLookup(fname,extMacroCnt,extMacroLst,meFL_CHECKPATH|meFL_USESRCHPATH,fn) ||
       (ffReadFile(&meior,fn,meRWFLAG_READ|meRWFLAG_SILENT,NULL,&hlp,0,0,0) == meABORT))
    {
        if(status)
            return meABORT ;
        return mlwrite(MWABORT|MWCLEXEC,(meUByte *)"[Failed to load file %s]", fname);
    }
    /* go execute it! */
    if((status = donbuf(&hlp,&varList,fn,f,n)) <= 0)
        /* the execution failed lets go to the line that caused the grief */
        macroPrintError(0,&hlp,fn) ;
    meLineLoopFree(&hlp,0) ;
    /* free off any command variables */
    while(varList != NULL)
    {
        meVariable *nv=varList->next;
        meNullFree(varList->value);
        free(varList) ;
        varList = nv ;
    }
    
    return status ;
}

int
executeFile(int f, int n)  /* execute a series of commands in a file */
{
    meUByte filename[meBUF_SIZE_MAX];      /* Filename */
    int status;
    
    if ((status=meGetString((meUByte *)"Execute File",MLFILECASE, 0, filename, 
                            meBUF_SIZE_MAX)) <= 0)
        return status ;
    
    /* otherwise, execute it */
    return execFile(filename,f,n) ;
}

/* executeNamedCommand: execute a named command even if it is not bound */
int
executeNamedCommand(int f, int n)
{
    register int idx ;       /* index to the requested function */
    meUByte buf[meBUF_SIZE_MAX];       /* buffer to hold tentative command name */
    meUByte prm[30] ;
    
    /* setup prompt */
    if(f == meTRUE)
        sprintf((char *)prm,"Arg %d: Command",n) ;
    else
        meStrcpy(prm,"Command") ;
    
    /* if we are executing a command line get the next arg and match it */
    if((idx = meGetString(prm, MLCOMMAND, 1, buf, meBUF_SIZE_MAX)) <= 0)
        return idx ;
    
    /* decode the function name, if -ve then duff */
    if((idx = decode_fncname(buf,0)) < 0)
        return(meFALSE);
    
    /* and then execute the command */
    return execFunc(idx,f,n) ;
}


int
lineExec (int f, int n, meUByte *cmdstr)
{
    register int status;                /* status return */
    meUByte *oldestr;                     /* original exec string */
    meUByte  oldcle ;                     /* old contents of clexec flag */
    meUByte  tkn[meTOKENBUF_SIZE_MAX] ;
    int oldexec, oldF, oldN, oldForce, prevForce ;
    
    /* save the arguments */
    oldcle = clexec;
    oldexec = execlevel;
    oldestr = execstr;
    oldF = meRegCurr->f;
    oldN = meRegCurr->n;
    oldForce = meRegCurr->force;
    /* store the execute-line's !force count on the prev registry if greater, the docmd resets the
     * force count so a !force !force execute-line "..." effectively loses the !force's during the
     * execution which can allow the debugger to kick in unexpectedly */
    if((prevForce=meRegCurr->prev->force) < oldForce)
        meRegCurr->prev->force = oldForce;
    
    clexec = meTRUE;                      /* in cline execution */
    execstr = NULL;
    execlevel = 0;                      /* Reset execution level */
    meRegCurr->f = f;
    meRegCurr->n = n;
    
    status = docmd(cmdstr,tkn) ;
    if((n & 0x2) && (status == meTRUE) && (execstr != NULL) && (execstr[0] != '\0'))
    {
        meUByte cc ;
        /* eat leading spaces */
        while(((cc=*execstr) == ' ') || (cc == '\t'))
            execstr++;
        /* dump comments, empties and labels here */
        if(cc != '\0')
            status = mlwrite(MWABORT|MWWAIT,(meUByte *)"[Extra text at end of line: %s]",execstr);
    }
    /* restore old arguments */
    execlevel = oldexec ;
    clexec = oldcle;
    execstr = oldestr ;
    meRegCurr->f = oldF ;
    meRegCurr->n = oldN ;
    meRegCurr->force = oldForce ;
    meRegCurr->prev->force = prevForce ;
    
    return status ;
}    


/* executeLine: Execute a command line command to be typed in by the user */
int
executeLine(int f, int n)
{
    register int status;                /* status return */
    meUByte cmdstr[meBUF_SIZE_MAX];               /* string holding command to execute */
    
    /* get the line wanted */
    if ((status = meGetString((meUByte *)":", 0, 0, cmdstr, meBUF_SIZE_MAX)) <= 0)
        return status ;
    
    return lineExec(f,n,cmdstr) ;
}

