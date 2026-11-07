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
#if MEOPT_UNICODE
/* set while reading an old registry file, \xNN from 0x80 is the byte */
meUByte tokenRawEscape=0;
#endif

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
    meKey key;
    
    dd = tok;
    ss = src;
    cc = *ss++;
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
                        int ll = (int) meStrlen(__cmdArray[CK_GOAMRK].name) ;
                        if((size_t) (dd+ll+9) <= (size_t) tokEnd)
                        {
                            dd += meKeyToStr(dd,ME_SPECIAL|SKEY_x_command) ;
                            meStrcpy(dd,__cmdArray[CK_GOAMRK].name) ;
                            dd += ll ;
                            dd += meKeyToStr(dd,ME_SPECIAL|SKEY_return) ;
                            dd += meMacroPutChar(dd,meANCHOR_EXSTRPOS);
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
                    dd += meKeyToStr(dd,key) ;
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
                        int ll = (int) meStrlen(__cmdArray[CK_SETAMRK].name) ;
                        if((size_t) (dd+ll+9) <= (size_t) tokEnd)
                        {
                            dd += meKeyToStr(dd,ME_SPECIAL|SKEY_x_command) ;
                            meStrcpy(dd,__cmdArray[CK_SETAMRK].name) ;
                            dd += ll ;
                            dd += meKeyToStr(dd,ME_SPECIAL|SKEY_return) ;
                            dd += meMacroPutChar(dd,meANCHOR_EXSTRPOS);
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
                        /* in a Unicode build \xNN is the char U+00NN */
#if MEOPT_UNICODE
                        if(tokenRawEscape && (cc >= 0x80))
                            *dd++ = cc;
                        else
#endif
                            dd += meMacroPutChar(dd,cc);
                    }
                    break;
                case 'u':
                case 'U':
                    {
                        /* \uXXXX or \UXXXXXX code point, up to 4 or 6 hex digits */
                        meUInt uc=0;
                        int ii=(cc == 'u') ? 4:6;
                        while((--ii >= 0) && isXDigit(*ss))
                        {
                            uc = (uc << 4) | hexToNum(*ss);
                            ss++;
                        }
                        if(ii == ((cc == 'u') ? 3:5))
                        {
                            /* no digits, as before it is a plain u */
                            *dd++ = cc;
                            break;
                        }
#if !MEOPT_UNICODE
                        if(uc >= 0x80)
                        {
                            /* the code page char, ? if it has none */
                            ii = -1;
#if MEOPT_EXTENDED
                            ii = 127;
                            while((ii >= 0) && (charToUnicode[ii] != uc))
                                ii--;
#endif
                            uc = (ii < 0) ? '?':(ii+128);
                        }
#endif
                        dd += meMacroPutChar(dd,uc);
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
/* meGetStringExec - an @mx or @mxa argument, the next token is executed and its
 * result is the argument; ss is the @mx token */
static int
meGetStringExec(meUByte *prompt, meUByte cc, meUByte *ss, meUByte *buffer, int size)
{
    /* malloced so meGetString, if this is inlined, has no array */
    meUByte *buff;
    int ret;
    
    if((buff = meMalloc(meTOKENBUF_SIZE_MAX)) == NULL)
        return meABORT;
    execstr = macroNextToken(execstr,buff);
    if(cc == 'a')
        execstr = ss;
    if(prompt == NULL)
        resultStr[0] = '\0';
    else
        meStrcpy(resultStr,prompt);
    if((ret = lineExec(0, 1, buff)) > 0)
    {
        meStrncpy(buffer,resultStr,size);
        buffer[size-1] = '\0';
        ret = meTRUE;
    }
    else
        ret = meABORT;
    free(buff);
    return ret;
}

int
meGetString(meUByte *prompt, int option, int defnum, meUByte *buffer, int size)
{
    /* if we are not interactive, go get it! */
    if(clexec == meTRUE)
    {
        meUByte *res, *ss, *tok, cc;
        int fn=0;
        
        /* take the next token of the compiled line */
        ss = execstr;
        if(*ss == meCL_END)
            tok = (meUByte *) "";
        else
        {
            tok = meCLGetTok(ss);
            if(*ss == meCL_TOK)
                fn = meCLGetValue(ss);
            execstr = meCLNext(ss);
        }
        
        if((tok[0] == '@') && (tok[1] == 'm') && (tok[2] == 'x'))
            return meGetStringExec(prompt,tok[3],ss,buffer,size);
        else if((tok[0] != '@') || (tok[1] != 'm') || (tok[2] != 'n'))
        {
            /* evaluate it, a function known when compiled by its index */
            res = (fn) ? gtfun(fn-1,NULL):getval(tok);
            if(res != (meUByte *) abortm)
            {
                ss = buffer;
                if(option & MLMACNORT)
                {
                    while((*ss))
                    {
                        ss++;
                        size--;
                    }
                }
                if(size > 1)
                {
                    /* copy up to any special char in one go */
                    size_t ll=meStrlen(res);
                    meUByte *ee;
                    if(!(option & MLFFZERO) && ((ee=memchr(res,meCHAR_LEADER,ll)) != NULL))
                        ll = ee - res;
                    if(ll >= (size_t) size)
                        ll = size-1;
                    memmove(ss,res,ll);
                    ss += ll;
                    res += ll;
                    size -= (int) ll;
                }
                /* all copied unless a special char was found */
                if(!(option & MLFFZERO))
                {
                    for( ; ((--size) > 0) && ((cc = *res++) != '\0') ; )
                    {
                        if((cc == meCHAR_LEADER) && ((cc = *res++) != meCHAR_TRAIL_LEADER))
                            break;
                        *ss++ = cc;
                    }
                }
                *ss = '\0';
                return meTRUE;
            }
            if(((tok[0] != '\0') && (tok[0] != ';')) || (option & MLEXECNOUSER))
            {
                *buffer = '\0';
                return meFALSE;
            }
        }
        /* if @mna (get all input from user) then rewind the execstr */
        else if(tok[3] == 'a')
            execstr = ss;
    }
    if(prompt == NULL)
    {
        *buffer = '\0';
        return ctrlg(meFALSE,1);
    }
    return meGetStringFromUser(prompt,option,defnum,buffer,size);
}

int
macarg(meUByte *tok)               /* get a macro line argument */
{
    int savcle;                 /* buffer to store original clexec */
    int status;
    
    savcle = clexec;            /* save execution mode */
    clexec = meTRUE;              /* get the argument */
    status = meGetString(NULL,MLNOHIST|MLFFZERO,0,tok,meBUF_SIZE_MAX);
    clexec = savcle;            /* restore execution mode */
    
    return status;
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
    cmd = cmdHead;
    while(cmd->anext != NULL)
    {
        if(meStrcmp(cmd->anext->name,cmd->name) < 0)
        {
            count++;
            sprintf((char *) resultStr,"cmdHead Error: [%s] should be before [%s]",cmd->anext->name,cmd->name);
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

/* macroDirective - the directive named at ss (the char after the '!'), or -1 if
 * unknown, *end is set to the char after the name.
 *
 * SWP 2003-12-22 Used to use the biChopFindString to look the
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
static int
macroDirective(meUByte *ss, meUByte **end)
{
    int status=-1;
    meUByte c1, c2=0, c3=0;

    if(((c1 = *ss) <= ' ') || ((c2 = *++ss) <= ' ') || ((c3 = *++ss) <= ' '))
    {
        *end = ss;
        return ((c1 == 'i') && (c2 == 'f')) ? DRIF:-1;
    }
    while(*++ss > ' ')
        ;
    *end = ss;
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
            status = DRBREAK;
        else if((c2 == 'e') && (c3 == 'l'))
            status = DRBELL;
    }
    else if((c1 == 'a') && (c2 == 'b') && (c3 == 'o'))
        status = DRABORT;
    return status;
}

/* macroSkipEnd - when a false test (or a reached !elif or !else) sets execlevel to 1 or 2
 * after line lp, the lines are skipped to the first line whose directive takes the level
 * below that; returns that line, which docmd then runs at the same level, or NULL if not
 * found or a directive is unknown (the lines are then walked and docmd reports it). These
 * are docmd's execlevel rules for skipped lines. */
static meLine *
macroSkipEnd(meLine *lp, meLine *hlp, int level, int compiled)
{
    meUByte *ss, cc;
    int status, dirType;

    while((lp = meLineGetNext(lp)) != hlp)
    {
        ss = lp->text;
        if(compiled)
        {
            if(*ss != meCL_DIR)
                continue;
            if((status = meCLGetValue(ss)) == meCL_UNKNOWN)
                return NULL;
        }
        else
        {
            cc = *ss;
            while((cc == ' ') || (cc == '\t'))
                cc = *++ss;
            if(getMacroType(cc) != TKDIR)
                continue;
            if((status = macroDirective(ss+1,&ss)) < 0)
                return NULL;
        }
        if(status == DRBREAK)
            /* a skipped !break is an !abort */
            continue;
        dirType = dirTypes[status];
        if(dirType & DRFLAG_SMSKEXECLVL)
        {
            if((level == 1) || ((dirType & DRFLAG_SDBLEXECLVL) && ((level -= 2) == 0)))
                return lp;
        }
        else if(dirType & DRFLAG_AMSKEXECLVL)
            level += 2;
    }
    return NULL;
}

/* After a stored macro line's compiled text and a '\0' come, for a directive line, the skip
 * ends of execlevels 1 and 2 (NULL until found), then if the macro keeps its source
 * (meMACRO_SOURCE) the line's number in its file and its text */
#define macroLineSkipEnd(lp,ll) ((lp)->text+(lp)->length+1+(((ll)-1)*sizeof(meLine *)))
#define macroLineExtra(lp) ((lp)->text+(lp)->length+1+(((lp)->text[0] == meCL_DIR) ? 2*sizeof(meLine *):0))

meUByte *
macroLineSource(meLine *lp)
{
    return macroLineExtra(lp)+sizeof(meInt);
}

static meInt
macroLineNumber(meLine *lp)
{
    meInt no;
    memcpy(&no,macroLineExtra(lp),sizeof(meInt));
    return no;
}

static meUByte *macroSrcFile=NULL;      /* the file of the lines being run, NULL if unknown */
#if MEOPT_EXTENDED
static meUByte macroHelpText=0;         /* storing a macro's define-help text */
#endif
static meUByte macroNoSource=0;         /* lines being run have no source */

/* macroSetSource - called as macro mac is defined, it then keeps its source; its head line
 * holds the file of the lines being run */
int
macroSetSource(meMacro *mac)
{
    meLine *hlp, *ohlp=mac->hlp;
    meUByte *fn=(macroSrcFile == NULL) ? (meUByte *) "":macroSrcFile;
    int ll=(int) meStrlen(fn);
    
    if(macroNoSource)
        /* defined by lines with no source */
        return meTRUE;
    if((hlp=meLineMalloc(ll,0)) == NULL)
        return meFALSE;
    memcpy(hlp->text,fn,ll);
    hlp->flag = ohlp->flag|meMACRO_SOURCE;
    hlp->next = hlp;
    hlp->prev = hlp;
    mac->hlp = hlp;
    meFree(ohlp);
    return meTRUE;
}

/* macroStoreLine - adds compiled line cl (len bytes, its end included) to the macro being
 * defined, with its source src (NULL if none) and its line number when the macro keeps it */
static int
macroStoreLine(meUByte *cl, int len, meUByte *src, meInt srcLine)
{
    meLine *lp, *plp;
    meUByte *ss;
    int extra=0, sl=0;
    
    if(len > 0xffff)
        return mlwrite(MWABORT|MWWAIT,(meUByte *)"[Macro line too long]");
    if(cl[0] == meCL_DIR)
        extra = 2*sizeof(meLine *);
    if(lpStore->flag & meMACRO_SOURCE)
    {
        if(src == NULL)
            src = (meUByte *) "";
        while((*src == ' ') || (*src == '\t'))
            src++;
        sl = (int) meStrlen(src)+1;
        extra += sizeof(meInt)+sl;
    }
    if((lp=meLineMalloc(len+extra,0)) == NULL)
        return meFALSE;
    lp->length = len;
    memcpy(lp->text,cl,len);
    lp->text[len] = '\0';
    ss = lp->text+len+1;
    if(cl[0] == meCL_DIR)
    {
        memset(ss,0,2*sizeof(meLine *));
        ss += 2*sizeof(meLine *);
    }
    if(sl)
    {
        memcpy(ss,&srcLine,sizeof(meInt));
        memcpy(ss+sizeof(meInt),src,sl);
    }
    plp = lpStore->prev;
    plp->next = lp;
    lp->prev = plp;
    lp->next = lpStore;
    lpStore->prev = lp;
    return meTRUE;
}

/* macroCompileLine - compiles the macro line ss into buf (bufSz bytes) or, when too small, a
 * malloced buffer the caller frees; returns the one used (NULL on a malloc failure) and sets
 * *len to the compiled length, its end included. Only token() reads the text. */
meUByte *
macroCompileLine(meUByte *ss, meUByte *buf, int bufSz, int *len)
{
    meUByte tkn[meTOKENBUF_SIZE_MAX], *dd=buf, *tt, cc;
    int ll, used=0, kind, value;
    
    while(((cc=*ss) == ' ') || (cc == '\t'))
        ss++;
    /* a label keeps its text */
    kind = (cc == '*') ? -1:0;
    for(;;)
    {
        if(kind < 0)
        {
            ll = (int) meStrlen(ss)+1;
            tt = ss;
        }
        else if((cc == '\0') || (cc == ';'))
        {
            ll = 0;
            kind = meCL_END;
        }
        else
        {
            ss = token(ss,tkn);
            ll = (int) meStrlen(tkn);
            tt = tkn;
            switch(getMacroType(tkn[0]))
            {
            case TKDIR:
                kind = meCL_DIR;
                if((value = macroDirective(tkn+1,&tt)) < 0)
                    value = meCL_UNKNOWN;
                tt = tkn;
                break;
            case TKCMD:
                kind = meCL_CMD;
                value = meCL_UNKNOWN;
                break;
            case TKFUN:
                kind = meCL_TOK;
                value = funcLookup(tkn+1)+1;
                break;
            default:
                kind = meCL_TOK;
                value = 0;
            }
        }
        if((used+meCL_HDR+ll+2) > bufSz)
        {
            meUByte *nn;
            bufSz = (bufSz*2)+meCL_HDR+ll+2;
            if((nn = meMalloc(bufSz)) == NULL)
            {
                if(dd != buf)
                    free(dd);
                return NULL;
            }
            memcpy(nn,dd,used);
            if(dd != buf)
                free(dd);
            dd = nn;
        }
        if(kind < 0)
        {
            memcpy(dd,tt,ll);
            used = ll;
            break;
        }
        if(kind == meCL_END)
        {
            dd[used++] = meCL_END;
            break;
        }
        dd[used] = kind;
        meCLSetValue(dd+used,value);
        dd[used+3] = (meUByte) ll;
        dd[used+4] = (meUByte) (ll >> 8);
        memcpy(dd+used+meCL_HDR,tt,ll+1);
        used += meCL_HDR+ll+1;
        while(((cc=*ss) == ' ') || (cc == '\t'))
            ss++;
    }
    *len = used;
    return dd;
}

/* macroNextToken - copies the next token at ss of a compiled line to tkn (empty at the end)
 * and returns the one after, as token() does with text */
meUByte *
macroNextToken(meUByte *ss, meUByte *tkn)
{
    if(*ss == meCL_END)
    {
        *tkn = '\0';
        return ss;
    }
    memcpy(tkn,meCLGetTok(ss),meCLGetLen(ss)+1);
    return meCLNext(ss);
}

/* macroStoreText - compiles and adds the text line to the macro being defined */
int
macroStoreText(meUByte *text)
{
    meUByte buf[meBUF_SIZE_MAX], *cl;
    int len, ret;
    
    if((cl=macroCompileLine(text,buf,meBUF_SIZE_MAX,&len)) == NULL)
        return meFALSE;
    ret = macroStoreLine(cl,len,text,1);
    if(cl != buf)
        free(cl);
    return ret;
}

static int
domstoreHelp(meUByte *cline)
{
#if MEOPT_EXTENDED
    /* check to see if this line ends definition */
    if(meStrncmp(cline, "!ehe",4) == 0)
    {
        helpBufferReset(lpStoreBp);
        mcStore = 0;
        lpStore = NULL;
        return meTRUE;
    }
#endif
    if(addLine(lpStore,cline) <= 0)
        return meFALSE;
    lpStoreBp->lineCount++;
    return meTRUE;
}

/* domstore - stores compiled line cl (len bytes) in the macro being defined, src is its text
 * (NULL if none) and srcLine its line number */
static int
domstore(meUByte *cl, int len, meUByte *src, meInt srcLine)
{
    meUByte *ss, cc;
#if MEOPT_EXTENDED
    int help=0;
    
    if(macroHelpText)
    {
        /* define-help text is kept as text */
        meUByte buf[meBUF_SIZE_MAX], *tt;
        int ll, ret;
        
        if(cl[0] == meCL_TEXT)
            ss = cl+1;
        else if((ss = src) == NULL)
            ss = (meUByte *) "";
        /* stored without its indent, as every macro line */
        while((*ss == ' ') || (*ss == '\t'))
            ss++;
        if(meStrncmp(ss,"!ema",4))
        {
            if(meStrncmp(ss,"!ehe",4) == 0)
                macroHelpText = 0;
            ll = (int) meStrlen(ss)+2;
            if((tt = (ll <= meBUF_SIZE_MAX) ? buf:meMalloc(ll)) == NULL)
                return meFALSE;
            tt[0] = meCL_TEXT;
            memcpy(tt+1,ss,ll-1);
            ret = macroStoreLine(tt,ll,src,srcLine);
            if(tt != buf)
                free(tt);
            return ret;
        }
        macroHelpText = 0;
    }
#endif
    /* dump comments and empties here */
    if((cc = cl[0]) == meCL_END)
        return meTRUE;
    /* check to see if this line turns macro storage off */
    if((cc == meCL_DIR) && (meCLGetValue(cl) == DREMACRO) && !execlevel--)
    {
        mcStore = meFALSE;
        lpStore = NULL;
        execlevel = 0;
        return meTRUE;
    }
    ss = cl;
    if((cc == meCL_TOK) && isDigit(*meCLGetTok(ss)))
        cc = *(ss = meCLNext(ss));
    if(cc == meCL_CMD)
    {
        if(!meStrcmp(meCLGetTok(ss),getCommandName(CK_DEFMAC)))
            execlevel++;
#if MEOPT_EXTENDED
        else if(!meStrcmp(meCLGetTok(ss),getCommandName(CK_DEFHELP)))
            help = 1;
#endif
    }
    if(macroStoreLine(cl,len,src,srcLine) <= 0)
        return meFALSE;
#if MEOPT_EXTENDED
    if(help)
        macroHelpText = 1;
#endif
    return meTRUE;
}


/*
   docmd - execute a compiled command line (see macroCompileLine)
   
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
        
try_again:
    if((cc = *execstr) == meCL_DIR)
    {
        int dirType ;
        
        if((status = meCLGetValue(execstr)) == meCL_UNKNOWN)
            return mlwrite(MWABORT|MWWAIT,(meUByte *)"[Unknown directive %s]",meCLGetTok(execstr));
        execstr = meCLNext(execstr);
        if((status == DRBREAK) && execlevel)
            /* Stop DRBREAK effecting exec if execlevel != 0 */
            status = DRABORT;
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
                goto try_again;
            case DRIIF:
                goto try_again;
            case DRNMACRO:
                nmacro = meTRUE;
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
    /* dump comments, empties, labels and help text here, and if not a directive and
     * execlevel > 0 ignore it */
    if((cc == meCL_END) || (cc == '*') || (cc == meCL_TEXT) || execlevel)
        return meTRUE ;
    
    /* if not a CMD then treat as numeric arg n */
    if(cc != meCL_CMD)
    {
        meUByte *tmp ;
        int fn ;
        tmp = meCLGetTok(execstr);
        fn = (cc == meCL_TOK) ? meCLGetValue(execstr):0;
        execstr = meCLNext(execstr);
        if((tmp = (fn) ? gtfun(fn-1,NULL):getval(tmp)) == abortm)
            return meFALSE;
        n = meAtoi(tmp) ;
        f = meTRUE;
        if(*execstr != meCL_CMD)
            return meFALSE;
    }
    else
    {
        f = meFALSE;
        n = 1;
    }
    
    /* and match the token to see if it exists, keeping the index found */
    if((ii = meCLGetValue(execstr)) == meCL_UNKNOWN)
    {
        if((ii = decode_fncname(meCLGetTok(execstr),0)) < 0)
            return meFALSE ;
        if(ii < meCL_UNKNOWN)
            meCLSetValue(execstr,ii);
    }
    execstr = meCLNext(execstr);
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
/* bp - buffer to execute, compiled - its lines are a stored macro's (see macroStoreLine), else
 * text compiled as each line runs */
static meLine *errorLine=NULL ;

static long
macroPrintError(meInt type, meLine *hlp, meUByte *macName, int compiled)
{
    meLine *lp ;
    long  ln ;
    
    if(errorLine == NULL)
        /* Quit if there is no error line */
        return -1 ;
    
    if(compiled)
    {
        if((errorLine != hlp) && (hlp->flag & meMACRO_SOURCE))
        {
            if(hlp->text[0] != '\0')
                mlwrite(MWABORT|MWWAIT,(meUByte *)"[%s:: %s executing %s, %s line %d]",macroLineSource(errorLine),
                        (type) ? "Halt":"Error",macName,hlp->text,macroLineNumber(errorLine)) ;
            else
                mlwrite(MWABORT|MWWAIT,(meUByte *)"[%s:: %s executing %s, line %d]",macroLineSource(errorLine),
                        (type) ? "Halt":"Error",macName,macroLineNumber(errorLine)) ;
        }
        else
            mlwrite(MWABORT|MWWAIT,(meUByte *)"[%s executing %s (compiled)]",(type) ? "Halt":"Error",macName) ;
        return 0 ;
    }
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
dobuf(meLine *hlp, int compiled)
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
    meUByte skipWalk=0;           /* the skip end was not found, walk the lines */
    meUByte *src;                 /* the line's source, NULL if none */
    meUByte hasSrc;               /* the lines have their source */
    meUByte *oldSrcFile=macroSrcFile, oldNoSource=macroNoSource;
    meLine *srcLp=hlp;            /* the last text line numbered and its number */
    meInt srcNo=0;
    
    hasSrc = (!compiled || (hlp->flag & meMACRO_SOURCE));
    macroSrcFile = (!compiled) ? meRegCurr->commandName:(hasSrc) ? hlp->text:NULL;
    macroNoSource = !hasSrc;
    clexec = meTRUE;              /* in cline execution */
    execstr = NULL ;
    execlevel = 0;                /* Reset execution level */
    lpFStk[0] = NULL;
#if MEOPT_DEBUGM
    meRegCurr->execHlp = hlp;
    meRegCurr->execLp = &lp;
    meRegCurr->execCompiled = (meUByte) compiled;
#endif

    /* starting at the beginning of the buffer */
    lp = hlp->next;
    while (lp != hlp)
    {
        tline = lp->text;
        src = (!compiled) ? tline:(hasSrc) ? macroLineSource(lp):NULL;
#if MEOPT_DEBUGM
        /* if $debug == meTRUE, every line to execute
           gets echoed and a key needs to be pressed to continue
           ^G will abort the command; a line with no source is only shown once
           after a break, to say it is compiled */
        
        if((macbug & 0x0a) &&
           ((src == NULL) ? (macbug & 0x80):(!execlevel || (macbug & 0x04) || ((execlevel == 1) && !meStrncmp(src,"!el",3)))))
        {
            meRegister *rr;
            meUByte dd, *dsrc, outline[meBUF_SIZE_MAX];   /* string to hold debug line text */
            meLine *tlp, *dlp;
            meKey cc;
            int ii, lno, dlvl, dpth=0;
            
            dd = macbug & ~0x80;
            /* force debugging off while we are getting input from the user,
             * if we don't and any macro is executed (idle-pick macro) then
             * ME will spin and crash!! */
            macbug = 0;
            
            /*---   Generate the debugging line for the 'programmer' !! */
loop_round3:
            rr = meRegCurr;
            ii = dpth;
            while(ii--)
            {
                if(rr->prev == meRegHead) 
                {
                    dpth -= ii+1;
                    break;
                }
                rr = rr->prev;
            }
            if(dpth == 0)
            {
                dlp = lp;
                dsrc = src;
                dlvl = execlevel;
            }
            else if(rr->execLp != NULL)
            {
                /* a parent is always running the line that called its child */
                dlp = *rr->execLp;
                dsrc = (!rr->execCompiled) ? dlp->text:(rr->execHlp->flag & meMACRO_SOURCE) ? macroLineSource(dlp):NULL;
                dlvl = 0;
            }
            else
                dsrc = NULL;
            if(dsrc == NULL)
                snprintf((char *)outline,meBUF_SIZE_MAX,"%d:%s: compiled ? ",dpth,rr->commandName) ;
            else
            {
                if(rr->execCompiled)
                    lno = macroLineNumber(dlp) ;
                else
                {
                    lno = 0;
                    tlp = rr->execHlp;
                    do
                        lno++ ;
                    while ((tlp=meLineGetNext(tlp)) != dlp)
                        ;
                }
                snprintf((char *)outline,meBUF_SIZE_MAX,"%d:%s:%d:%d [%s] ? ",dpth,rr->commandName,lno,dlvl,dsrc) ;
            }
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
                if(meKeyIsChar(cc = meGetKeyFromUser(meFALSE,0,meGETKEY_SILENT|meGETKEY_SINGLE)))
                    cc = meUniCharToLower(cc) ;
                switch(cc)
                {
                case '?':
                    mlwrite(MWSPEC,__dobufStr1);       /* Write out the debug line */
                    goto loop_round;
                case 'L'-'@':
                    goto loop_round2;
                case 'd':
                    if(!dpth)
                        goto loop_round;
                    dpth--;
                    goto loop_round3;
                case 'u':
                    dpth++;
                    goto loop_round3;
                case 'v':
                    {
                        meUShort mlStatusStore ;
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
            
            if(mcStore == 2)
                status = domstoreHelp((!compiled) ? tline:(tline[0] == meCL_TEXT) ? tline+1:(src != NULL) ? src:(meUByte *) "") ;
            else if(compiled)
            {
                if(mcStore)
                    status = domstore(tline,lp->length,src,(src != NULL) ? macroLineNumber(lp):0) ;
                else
                    status = docmd(tline,tkn) ;
            }
            else
            {
                /* compile the text line as it runs */
                meUByte cbuf[meBUF_SIZE_MAX], *cl;
                int clen;
                
                if((cl = macroCompileLine(tline,cbuf,meBUF_SIZE_MAX,&clen)) == NULL)
                    status = meABORT ;
                else
                {
                    if(mcStore)
                    {
                        if(lpStore->flag & meMACRO_SOURCE)
                        {
                            /* number the line for its source */
                            if(meLineGetNext(srcLp) != lp)
                            {
                                srcNo = 0;
                                for(srcLp=hlp ; meLineGetNext(srcLp) != lp ; srcLp=meLineGetNext(srcLp))
                                    srcNo++;
                            }
                            srcLp = lp;
                            srcNo++;
                        }
                        status = domstore(cl,clen,tline,srcNo) ;
                    }
                    else
                        status = docmd(cl,tkn) ;
                    if(cl != cbuf)
                        free(cl);
                }
            }
#if MEOPT_DEBUGM
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
                    meLine *glp;                  /* line to goto */
                    
                    linlen = (int) meStrlen(tkn) ;
                    /* a stored macro's goto line keeps the label line last found, its first
                     * skip end is not used, the label may be a variable so is checked */
                    if(compiled && (lp->text[0] == meCL_DIR))
                    {
                        memcpy(&glp,macroLineSkipEnd(lp,1),sizeof(meLine *));
                        if((glp != NULL) && (meStrncmp(glp->text+1,tkn,linlen) == 0) && isSpace(glp->text[linlen+1]))
                        {
                            lp = glp;
                            status = meTRUE;
                            break ;
                        }
                    }
                    glp = hlp->next;
                    while(glp != hlp)
                    {
                        if(*glp->text == '*' && (meStrncmp(glp->text+1,tkn,linlen) == 0) &&
                           isSpace(glp->text[linlen+1]))
                        {
                            if(compiled && (lp->text[0] == meCL_DIR))
                                memcpy(macroLineSkipEnd(lp,1),&glp,sizeof(meLine *));
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
                if((lpFStk[lpCnt] == lp) && (lpLStk[lpCnt] != NULL)) 
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
#if MEOPT_DEBUGM
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
                macroPrintError(0,hlp,meRegCurr->commandName,compiled) ;
            }
#endif
            goto dobuf_exit ;
        }
#if MEOPT_DEBUGM
        if((macbug & 0x80) && hasSrc)
        {
            errorLine = lp;
            macroPrintError(1,hlp,meRegCurr->commandName,compiled) ;
            macbug &= ~0x80 ;
        }
#endif
        
        if(execlevel == 0)
            skipWalk = 0;
        /* domstore uses execlevel to count nested define-macros */
        else if(((execlevel == 1) || (execlevel == 2)) && !skipWalk && !mcStore
#if MEOPT_DEBUGM
           && (!(macbug & 0x0a) || !hasSrc)
#endif
           )
        {
            /* skip straight to the line which ends the skip */
            meLine *elp;
            if(compiled && (lp->text[0] == meCL_DIR))
            {
                memcpy(&elp,macroLineSkipEnd(lp,execlevel),sizeof(meLine *));
                if((elp == NULL) && ((elp = macroSkipEnd(lp,hlp,execlevel,compiled)) != NULL))
                    memcpy(macroLineSkipEnd(lp,execlevel),&elp,sizeof(meLine *));
            }
            else
                elp = macroSkipEnd(lp,hlp,execlevel,compiled);
            if(elp != NULL)
            {
                lp = elp;
                continue;
            }
            skipWalk = 1;
        }
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
#if MEOPT_DEBUGM
    meRegCurr->execLp = NULL;
#endif
    macroSrcFile = oldSrcFile;
    macroNoSource = oldNoSource;

    return status ;
}


static int
donbuf(meLine *hlp, meVariable **varList, meUByte *commandName, int f, int n, int compiled)
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
    
    status = dobuf(hlp,compiled) ;
    
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
        thisflag = 0;
        status = cmdTable[index]->func(f,n);
        if(isComIgnore(index))
        {
            thisflag = lastflag;
            return status;
        }
        lastflag = thisflag;
        if(selhilight.uFlags)
        {
            if((status > 0) || isComSelIgnFail(index))
            {
                if(isComSelKill(index))
                {
                    selhilight.bp = NULL;
                    selhilight.flags = 0;
                }
                else
                {
                    meWindow *cwp=frameCur->windowCur;
                    if(isComSelStart(index))
                    {
                        selhilight.bp = cwp->buffer;                  /* Select the current buffer */
                        selhilight.flags = SELHIL_ACTIVE|SELHIL_CHANGED;
                    }
                    if(selhilight.flags & SELHIL_FIXED)
                    {
                        if(!(selhilight.flags & SELHIL_KEEP) && isComSelDelFix(index))
                            selhilight.flags &= ~SELHIL_ACTIVE;
                    }
                    else if(selhilight.flags & SELHIL_ACTIVE)
                    {
                        if(isComSelSetMark(index) &&
                           ((selhilight.markOffset != cwp->markOffset) ||
                            (selhilight.markLineNo != cwp->markLineNo)))
                        {
                            selhilight.flags |= SELHIL_CHANGED;
                            selhilight.markOffset = cwp->markOffset;
                            selhilight.markLineNo = cwp->markLineNo;
                        }
                        if(isComSelSetDot(index) &&
                           ((selhilight.dotOffset != cwp->dotOffset) ||
                            (selhilight.dotLineNo != cwp->dotLineNo)))
                        {
                            selhilight.flags |= SELHIL_CHANGED;
                            selhilight.dotOffset = cwp->dotOffset;
                            selhilight.dotLineNo = cwp->dotLineNo;
                        }
                        if(isComSelSetFix(index))
                        {
                            selhilight.flags |= SELHIL_FIXED;
                            if(selhilight.uFlags & SELHILU_KEEP)
                                selhilight.flags |= SELHIL_KEEP;
                        }
                        if(isComSelStop(index))
                            selhilight.flags &= ~SELHIL_ACTIVE;
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
                selhilight.flags &= ~SELHIL_ACTIVE;
        }
    }
    else
    {
        register meMacro *mac = getMacro(index);
#if MEOPT_EXTENDED
        register meLine *hlp;
        
        hlp = mac->hlp;
        if(hlp->flag & meMACRO_EXEC)
            status = donbuf(hlp,&mac->varList,mac->name,f,n,1);
        else
        {
            if(hlp->flag & meMACRO_FILE)
            {
                if(mac->fname != NULL)
                    execFile(mac->fname,0,1);
                else
                    execFile(mac->name,0,1);
                hlp = mac->hlp;
                if(hlp->flag & meMACRO_FILE)
                    return mlwrite(MWABORT,(meUByte *)"[file-macro %s not defined]",mac->name);
            }
            hlp->flag |= meMACRO_EXEC;
            status = donbuf(hlp,&mac->varList,mac->name,f,n,1);
            if(mac->hlp != hlp)
                meLineLoopFree(hlp,1);
            else
                hlp->flag &= ~meMACRO_EXEC;
        }
#else
        status = donbuf(mac->hlp,NULL,mac->name,f,n,1);
#endif
    }
    
    return status;
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
    if(((s = donbuf(bp->baseLine,&varList,bp->name,f,n,0)) <= 0) &&
       ((ln = macroPrintError(0,bp->baseLine,bufn,0)) >= 0))
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
static int
execFileRead(meUByte *fname, int f, int n, meUInt rflags)
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
       (ffReadFile(&meior,fn,rflags|meRWFLAG_READ|meRWFLAG_SILENT,NULL,&hlp,0,0,0) == meABORT))
    {
        if(status)
            return meABORT ;
        return mlwrite(MWABORT|MWCLEXEC,(meUByte *)"[Failed to load file %s]", fname);
    }
    /* go execute it! */
    if((status = donbuf(&hlp,&varList,fn,f,n,0)) <= 0)
        /* the execution failed lets go to the line that caused the grief */
        macroPrintError(0,&hlp,fn,0) ;
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
execFile(meUByte *fname, int f, int n)
{
    return execFileRead(fname,f,n,0);
}

#if MEOPT_UNICODE
/* Executes macro file fname read as single byte in code page cp, never as UTF-8 */
int
execFileCodePage(meUByte *fname, meCodePage *cp)
{
    meior.codePage = cp;
    return execFileRead(fname,meFALSE,1,meRWFLAG_CODEPAGE);
}
#endif

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
    
    /* set @cc to this command, but @cck remains esc-X */
    thisIndex = idx;
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
    meUByte  cbuf[meBUF_SIZE_MAX], *cl ;
    int oldexec, oldF, oldN, oldForce, prevForce, clen ;
    
    if((cl = macroCompileLine(cmdstr,cbuf,meBUF_SIZE_MAX,&clen)) == NULL)
        return meABORT ;
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
    
    status = docmd(cl,tkn) ;
    if((n & 0x2) && (status == meTRUE) && (execstr != NULL) && (execstr[0] != meCL_END))
        status = mlwrite(MWABORT|MWWAIT,(meUByte *)"[Extra text at end of line: %s]",
                         (execstr[0] == '*') ? execstr:meCLGetTok(execstr));
    /* restore old arguments */
    execlevel = oldexec ;
    clexec = oldcle;
    execstr = oldestr ;
    meRegCurr->f = oldF ;
    meRegCurr->n = oldN ;
    meRegCurr->force = oldForce ;
    meRegCurr->prev->force = prevForce ;
    if(cl != cbuf)
        free(cl);
    
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

