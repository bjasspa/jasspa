/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * exec.c - Command execution.
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
 * Created:     1986
 * Synopsis:    Command execution.
 * Authors:     Daniel Lawrence, Jon Green & Steven Phillips
 * Description:
 *     This file is for functions dealing with execution of
 *     commands, command lines, buffers, files and startup files
 */

#define __EXECC                 /* Define program name */

#define KEY_TEST 0

#include "emain.h"
#include "efunc.h"
#include "eskeys.h"
#include "evar.h"


#define DRTESTFAIL   0x80

#define DRUNTILF     (DRUNTIL|DRTESTFAIL)

#define DRRCONTIN    0x60
#define DRRDONE      0x61
#define DRRELIF      0xc8
#define DRRELSE      0xc9
#define DRRENDIF     0x08
#define DRRGOTO      0x62
#define DRRIF        0x38
#define DRRREPEAT    0x20
#define DRRRETURN    0x05
#define DRRUNTIL     0x31
#define DRRUNTILF    0xb3
#define DRRWHILE     0x37
#define DRRWHILEF    0xb9

#define DRRJUMP      0x80

int relJumpTo ;

int
biChopFindString(register meUByte *ss, register int len, register meUByte **tbl,
                 register int size)
{
    register int hi, low, mid, ll ;
    
    hi  = size-1;               /* Set hi water to end of table */
    low = 0;                    /* Set low water to start of table */
    
    do
    {
        meUByte *s1, *s2, cc ;
        mid = (low + hi) >> 1;          /* Get mid value. */
        
        ll = len ;
        s1 = tbl[mid] ;
        s2 = ss ;
        for( ; ((cc=*s1++) == *s2) ; s2++)
            if(!--ll || (cc == 0))
                return mid ;
        if(cc > *s2)
            hi = mid - 1;               /* Discard bottom half */
        else
            low = mid + 1;              /* Discard top half */
    } while(low <= hi);                 /* Until converges */
    
    return -1 ;
}

/* token:       chop a token off a string
 * return a pointer past the token
 *
 * src - source string,
 * tok - destination token string (must be TOKENBUF in size, 
 * returning a string no bigger than MAXBUF with the \0)
 */
meUByte *
token(meUByte *src, meUByte *tok)
{
    register int quotef ;
    register meUByte cc, *ss, *dd, *tokEnd ;
    meUShort key ;
    
    ss = src ;
    dd = tok ;
    /* note tokEnd is set to tok+MAXBUF-1 to leave room for the terminating \0 */
    tokEnd = dd + MAXBUF-1 ;
    
    /*---       First scan past any whitespace in the source string */
    
    while(((cc = *ss) == ' ') || (cc == '\t'))
        ss++ ;
    
    /*---       Scan through the source string */
    
    for(quotef=0 ; ; cc=*++ss)
    {
        switch(cc)
        {
        case '"':
            if(quotef)
            {
                /* Null Terminate the token and exit */
                ss++;
                *dd = 0;
                return ss ;
            }
            quotef = 1 ;
            *dd++ = '"' ;
            break ;
        case '\t':
        case ' ':
        case ';':
            if(quotef)
            {
                *dd++ = cc;
                break ;
            }
            /* no break - this is the end of the token */
        case '\0':
            /* Null Terminate the token and exit */
            *dd = 0;
            return ss ;
            
        case '\\':
            /* Process special characters */
            switch ((cc = *++ss))
            {
            case '\0':
                /* this probably should be an error */
                *dd = 0;
                return ss ;
                
            case 'C':
                /* Control key - \C? */
                *dd++ = *++ss - '@'; 
                break;
            case 'E':
                /* Escape key - replace with esc */
                key = ME_SPECIAL|SKEY_esc ;
                goto quote_spec_key ;
            case 'I':
                /* backward-delete-tab - replace with S-tab */
                key = ME_SPECIAL|ME_SHIFT|SKEY_tab ;
                goto quote_spec_key ;
            case 'N':
                /* Return key - replace with return */
                key = ME_SPECIAL|SKEY_return ;
                goto quote_spec_key ;
            case 'P':
                /* Go to set position, defined by \p - replace with \CXAP */
                *dd++ = 'X' - '@';
                *dd++ = 'A' ;
                *dd++ = 'P';
                break;
            case 'T':
                /* Tab key - replace with tab */
                key = ME_SPECIAL|SKEY_tab ;
quote_spec_key:
                *dd++ = 0xff ;
                *dd++ = 2 ;
                *dd++ = key >> 8 ;
                *dd++ = key & 0xff ;
                break;
            case 'a':   *dd++ = 0x07; break;
            case 'b':   *dd++ = 0x08; break;
            case 'd':   *dd++ = 0x7f; break;
            case 'e':   *dd++ = 0x1b; break;
            case 'f':   *dd++ = 0x0c; break;
            case 'i':
                *dd++ = 0xff;
                *dd++ = 2;  
                *dd++ = '@';
                *dd++ = 'I' - '@';
                break;
            case 'n':   *dd++ = 0x0a; break;
            case 'p':
                *dd++ = 'X' - '@';
                *dd++ = 'A' - '@';
                *dd++ = 'P';
                break;
            case 'r':   *dd++ = 0x0d; break;
            case 's':   
                *dd++ = 0xff;
                *dd++ = 0x02;  
                break;
            case 't':   *dd++ = 0x09; break;
            case 'v':   *dd++ = 0x0b; break;
            case 'x':
                cc = ss[1] ;
                if(isXDigit(cc))
                {
                    register meUByte c1=(++ss)[1] ;
                    
                    if(isXDigit(c1))
                    {
                        cc = (hexToNum(cc) << 4) | hexToNum(c1) ;
                        ss++ ;
                    }
                    else
                        cc = hexToNum(cc) ;
                    if(cc == 0)
                    {
                        *dd++ = 0xff ;
                        *dd++ = 0x01 ;
                    }
                    else if(cc == 0xff)
                    {
                        *dd++ = 0xff ;
                        *dd++ = 0xff ;
                    }
                    else
                        *dd++ = cc ;
                }
                break;
            default:
                *dd++ = cc;
            }
            break ;
            
        case 255:
            *dd++ = 0xff;
            /* no break - must store 2 0xff's */
        default:
            /* record the character */
            *dd++ = cc;
        }
        if((size_t) dd > (size_t) tokEnd)
        {
            /* reset dd into safe area and keep going cos we must
			 * finish parsing this token */
            dd = tokEnd ;
        }
    }
    
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
    if (clexec == TRUE)
    {
        meUByte buff[TOKENBUF], *res, *ss, cc ;
        
        /* grab token and advance past */
        ss = execstr ;
        execstr = token(ss, buff);
        
        if((buff[0] == '@') && (buff[1] == 'm') && (buff[2] == 'x'))
        {
            cc = buff[3] ;
            execstr = token(execstr, buff);
            if(cc == 'a')
                execstr = ss ;
            meStrcpy(resultStr,prompt) ;
            if(lineExec (0, 1, buff) != TRUE)
                return ABORT ;
            meStrncpy(buffer,resultStr,size-1) ;
            buffer[size-1] = '\0' ;
            return TRUE ;
        }
        else if((buff[0] != '@') || (buff[1] != 'm') || (buff[2] != 'n'))
        {
            /* evaluate it */
            res = getval(buff) ;
            if(res == (meUByte *) abortm)
            {
                *buffer = '\0' ;
                return FALSE ;
            }
            ss = buffer ;
            if(option & MLMACNORT)
            {
                while((*ss))
                {
                    ss++ ;
                    size-- ;
                }
            }
            for(; ((--size) > 0) && ((cc = *res++) != '\0') ; )
            {
                if((cc == 0xff) && !(option & MLFFZERO) &&
                   (((cc = *res++) == 0x0) || (cc == 0x01)))
                    break ;
                *ss++ = cc ;
            }
            *ss = '\0' ;
            return TRUE ;
        }
        /* if @mna (get all input from user) then rewind the execstr */
        if(buff[3] == 'a')
            execstr = ss ;
    }
    return meGetStringFromUser(prompt, option, defnum, buffer, size) ;
}

int
macarg(meUByte *tok)               /* get a macro line argument */
{
    int savcle;                 /* buffer to store original clexec */
    int status;
    
    savcle = clexec;            /* save execution mode */
    clexec = TRUE;              /* get the argument */
    status = meGetString((meUByte *)"", MLNOHIST|MLFFZERO, 0, tok, MAXBUF) ;
    clexec = savcle;            /* restore execution mode */
    
    return(status);
}

#if KEY_TEST
/*---   Little function to test the  alphabetic state of the table.  Invoked
   via "!test" primative.  */

int
fnctest(void)
{
    register KEYTAB *ktp;                       /* Keyboard character array */
    register meCMD *cmd ;                       /* Names pointer */
    meUInt key ;
    meUByte outseq[12];
    int count=0, ii;                            /* Counter of errors */
    
    /* test the command hash table */
    for (ii=0; ii < CK_MAX; ii++)
    {
        key = cmdHashFunc(getCommandName(ii)) ;
        cmd = cmdHash[key] ;
        while((cmd != NULL) && (cmd != cmdTable[ii]))
            cmd = cmd->hnext ;
        if(cmd == NULL)
        {
            count++;
            mlwrite(MWWAIT,"cmdHash Error: [%s] should be in position %d", 
                   getCommandName(ii),key) ;
        }
    }
    
    /* test the command alphabetically ordered list */
    cmd = cmdHead ;
    while(cmd->anext != NULL)
    {
        if(meStrcmp(cmd->anext->name,cmd->name) < 0)
        {
            count++;
            mlwrite(MWWAIT,"cmdHead Error: [%s] should be before [%s]", 
                   cmd->anext->name,cmd->name) ;
        }
        cmd = cmd->anext ;
    }
    /* Scan through the key table looking for the character code.  If found
     * then return the index into the names table.  */
    
    for (ktp = &keytab[0]; ktp[1].code != 0; ktp++)
        if (ktp->code > ktp[1].code)
        {
            count++;
            meGetStringFromKey(ktp->code, outseq);
            mlwrite(MWWAIT,"[%s] key out of place",outseq);
        }
    
    if(count)
        mlwrite(MWWAIT,"!test: %d Error(s) Detected",count);
    return (count);
}
#endif

static int
domstore(meUByte *cline)
{
    register meUByte cc ;          /* Character */
    
    if(mcStore == 2)
    {
        /* check to see if this line ends definition */
        if(meStrncmp(cline, "!ehe",4) == 0)
        {
            helpBufferReset(lpStoreBp) ;
            mcStore = 0 ;
            lpStore = NULL;
            return TRUE ;
        }
        if(addLine(lpStore,cline) == 0)
            return FALSE ;
        lpStoreBp->elineno++ ;
        return TRUE ;
    }
    cc = *cline ;
    /* eat leading spaces */
    while((cc == ' ') || (cc == '\t'))
        cc = *++cline ;
    
    /* dump comments and empties here */
    if((cc == ';') || (cc == '\0'))
        return TRUE ;
#ifdef _MACRO_COMP
    /* check to see if this line turns macro storage off */
    if((cc == '!') &&
       (((cline[1]) == DREMACRO+128) ||
        (meStrncmp(cline+1, "ema", 3) == 0)) && 
       !execlevel--)
    {
        mcStore = FALSE;
        lpStore = NULL;
        execlevel = 0 ;
        return TRUE ;
    }
    if(macbug == -3)
    {
        LINE           *lp, *ilp ;
        meUByte  *ss ;
        register int    status ;
        int             len=0 ;
        
        ss = (meUByte *) cline ;
        while((*ss++))
            len++ ;
        if ((lp=lalloc(len)) == NULL)
            return FALSE ;
        ilp = lpStore ;
        ilp->l_bp->l_fp = lp;
        lp->l_bp = ilp->l_bp;
        ilp->l_bp = lp;
        lp->l_fp = ilp ;
        ss = lp->l_text ;
        for(;;)
        {
            meUByte dd, *s1 ;
            int           idx ;
            
            while(((dd = (meUByte) *cline++) == ' ') || (dd == '\t'))
                ;
            if((dd == '\0') || (dd == ';'))
            {
                *--ss = '\0' ;
                lp->l_used = ss-lp->l_text ;
                return TRUE ;
            }
            s1 = cline ;
            cc = dd ;
            idx = 0 ;
            for(;;)
            {
                if(cc == '"')
                    idx ^= 1 ;
                else if((cc == '\\') && (*cline != '\0'))
                    cline++ ;
                if(!idx && ((cc == ' ') || (cc == '\t') || (cc == '\0')))
                    break ;
                cc = *cline++ ;
            }
            *--cline = '\0' ;
            switch(dd)
            {
            case '!':
                if((*s1 >= 128) ||
                   ((status = biChopFindString(s1,3,derNames,NDERIV)) < 0))
                    goto cpy_str ;
                *ss++ = '!' ;
                *ss++ = status + 128 ;
                break ;
            case '$':
                if((*s1 >= 128) ||
                   ((status = biChopFindString(s1,14,envars,NEVARS)) < 0))
                    goto cpy_str ;
                *ss++ = '$' ;
                *ss++ = status + 128 ;
                break ;
            case '&':
                if((*s1 >= 128) ||
                   ((status = biChopFindString(s1,3,funcNames,NFUNCS)) < 0))
                    goto cpy_str ;
                *ss++ = '&' ;
                *ss++ = status + 128 ;
                break ;
            default:
                if(!isAlpha(dd) ||
                   ((idx = __decode_fncname(s1-1,1)) < 0) ||
                   (idx >= CK_MAX))
                    goto cpy_str ;
                if(idx == CK_DEFMAC)
                    execlevel++ ;
                *ss++ = '^' ;
                if(((++idx) == '\n') || (idx == '\r') || (idx == 26))
                    ss += expandchar(idx,ss,meEXPAND_BACKSLASH) ;
                else
                {
                    if((idx == ' ') || (idx == '\\') || (idx == '\t'))
                        *ss++ = '\\' ;
                    *ss++ = idx ;
                }
                break ;
                
cpy_str:
                if((dd == '^') &&
                   (*s1 == CK_DEFMAC+1))
                    execlevel++ ;
                if((dd == '"') && (cline[-1] == '"') && 
                   ((long) cline != (long) s1+1))
                {
                    meUByte *s2, ee ;
                    
                    if((ee=*s1) == '\\')
                        ee = s1[1] ;
                    if(getMacroType(ee) == TKCMD)
                    {
                        s2 = s1 ;
                        while(((ee = *s2++) != '\0') && (ee != ' ') && (ee != '\t'))
                            ;
                        if(ee == '\0')
                        {
                            while((dd = *s1++))
                                *ss++ = dd ;
                            ss-- ;
                            break ;
                        }
                    }   
                }
                do {
                    *ss++ = dd ;
                } while((dd = *s1++)) ;
                break ;
            }
            *cline = cc ;
            *ss++ = ' ' ;
        }
    }
    else
    {
        if(((cc == '^') && (cline[1] == CK_DEFMAC+1)) ||
           ((cc == 'd') && (strncmp(cline, names[CK_DEFMAC].name,12) == 0)))
            execlevel++ ;
        return addLine(lpStore,cline) ;
    }
#else
    /* check to see if this line turns macro storage off */
    if((cc == '!') && !meStrncmp(cline+1, "ema", 3) && !execlevel--)
    {
        mcStore = FALSE;
        lpStore = NULL;
        execlevel = 0 ;
        return TRUE ;
    }
    else
    {
        meUByte *ss = cline ;
        if(isDigit(cc))
        {
            do
                cc = *++ss ;
            while(isDigit(cc)) ;
            while((cc == ' ') || (cc == '\t'))
                cc = *++ss ;
        }
        if((cc == 'd') && !meStrncmp(ss, getCommandName(CK_DEFMAC),12) &&
           (((cc=ss[12]) == ' ') || (cc == '\t') || (cc == ';') || (cc == '\0')))
            execlevel++ ;
    }
    return addLine(lpStore,cline) ;
#endif
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
    register int  f ;           /* default argument flag */
    register int  n ;           /* numeric repeat value */
    register meUByte cc ;         /* Character */
    register int  status ;      /* return status of function */
    register int  nmacro ;      /* run as it not a macro? */
    
    cc = *cline ;
    /* eat leading spaces */
    while((cc == ' ') || (cc == '\t'))
        cc = *++cline ;
    
    /* dump comments, empties and labels here */
    if((cc == ';') || (cc == '\0') || (cc == '*'))
        return TRUE ;
    
    nmacro = FALSE;
    execstr = cline;    /* and set this one as current */
    meRegCurr->force = 0 ;
    
    /* process argument */
try_again:
    execstr = token(execstr, tkn);
    if((status=getMacroTypeS(tkn)) == TKDIR)
    {
        register int dirType ;
        /* look the derivative up in the name table */
#ifdef _MACRO_COMP
        if((status = tkn[1]) >= 128)
            status -= 128 ;
        else
#endif
            if((status = biChopFindString(tkn+1,3,derNames,NDERIV)) < 0)
            {
                mlwrite(MWABORT|MWWAIT,(meUByte *)"[Unknown directive %s]",tkn);
                return FALSE ;
            }
        dirType = dirTypes[status] ;
        
        if(execlevel == 0)
        {
elif_jump:
            if(dirType & DRFLAG_TEST)
            {
                if(macarg(tkn) != TRUE)
                    return FALSE ;
                if(!meAtol(tkn))
                {
                    /* if DRTGOTO or DRTJUMP and the test failed, we dont
                     * need the argument */
                    dirType &= ~DRFLAG_ARG ;
                    execlevel += (dirType & DRFLAG_AMSKEXECLVL) ;
                    status |= DRTESTFAIL ;
                }
            }
            else if(dirType & DRFLAG_ASGLEXECLVL)
                /* DRELIF DRELSE */
                execlevel += 2 ;
            if(dirType & DRFLAG_ARG)
            {
                if(macarg(tkn) != TRUE)
                {
                    if(!(dirType & DRFLAG_OPTARG))
                        return FALSE ;
                    f = FALSE;
                    n = 1 ;
                }
                else if(dirType & DRFLAG_NARG)
                {
                    f = TRUE;
                    n = meAtoi(tkn) ;
                    if(dirType & DRFLAG_JUMP)
                        relJumpTo = n ;
                }
            }
            if(!(dirType & DRFLAG_SWITCH))
                return status ;
            
            
            /* DRTEST, DRFORCE, DRNMACRO, DRABORT, DRBELL, DRRETURN */
            switch(status)
            {
#if KEY_TEST
            case DRTEST:
                /* Test directive. */
                return ((fnctest() == 0));
#endif
            case DREMACRO:
                mlwrite(MWABORT|MWWAIT,(meUByte *)"[Unexpected !emacro]");
                return FALSE ;
            case DRFORCE:
                (meRegCurr->force)++ ;
                goto try_again;
            case DRNMACRO:
                nmacro = TRUE;
                goto try_again;
            case DRABORT:
                if(f)
                    TTdoBell(n) ;
                return FALSE ;
            case DRBELL:
                TTdoBell(n) ;
                return TRUE ;
            case DRRETURN:
                return (n) ? DRRETURN:FALSE ;
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
        
        return TRUE ;
    }
    /* if not a directive and execlevel > 0 ignore it */
    if(execlevel)
        return TRUE ;
    
    /* first set up the default command values */
    f = FALSE;
    n = 1;
    
    if(status != TKCMD)
    {
        meUByte *tmp ;
        
        if((tmp = getval(tkn)) == abortm)
            return FALSE ;
        n = meAtoi(tmp) ;
        f = TRUE;
        
        execstr = token(execstr, tkn);
        if(getMacroTypeS(tkn) != TKCMD)
            return FALSE ;
    }
    
    {
        register int idx ; /* index to function to execute */
        
        if(execlevel)
            return TRUE ;
        
        /* and match the token to see if it exists */
        if ((idx = decode_fncname(tkn,0)) < 0)
            return(FALSE);
        if(nmacro)
            clexec = FALSE ;
        cmdstatus = (execFunc(idx,f,n) == TRUE) ;       /* call the function */
        clexec = TRUE ;
        if(cmdstatus == TRUE)
            status = TRUE ;
        else if(TTbreakFlag)
        {
            if(meRegCurr->force > 1)
            {
                /* A double !force - macro says \CG is okay, 
                 * remove all input (must remove the \CG) and rest flag
                 */
                TTinflush() ;
                TTbreakFlag = 0 ;
                status = TRUE ;
            }
            else
                status = FALSE ;
        }
        else if(meRegCurr->force)
            status = TRUE ;
        else
            status = FALSE ;
        return status ;
    }
}

/*      dobuf:  execute the contents of the buffer pointed to
   by the passed BP                                */
/* bp - buffer to execute */
static meUByte __dobufStr1[]="(^G)Abort, (^L)redraw, (V)ariable, (!)Continue, (S)tep, <any>=next ?" ;
static LINE *errorLine=NULL ;

static int
dobuf(LINE *hlp)
{
#if     DEBUGM
    meByte debug=0;
#endif
    meUByte *tline;               /* Temp line */
    int status;                 /* status return */
    LINE *lp;                   /* pointer to line to execute */
    LINE *wlp;                  /* line to while */
    LINE *rlp;                  /* line to repeat */
    
    clexec = TRUE;                      /* in cline execution */
    execstr = NULL ;
    execlevel = 0;                      /* Reset execution level */
    
    /* starting at the beginning of the buffer */
    wlp = NULL;
    rlp = NULL;
    lp = hlp->l_fp;
    while (lp != hlp)
    {
        tline = lp->l_text;
#if     DEBUGM
        /* if $debug == TRUE, every line to execute
           gets echoed and a key needs to be pressed to continue
           ^G will abort the command */
        
        if(macbug > 0)
        {
            if((macbug > 1) || (macbug && !execlevel))
            {
                meUByte dd=macbug, outline[MAXBUF];   /* string to hold debug line text */
                LINE *tlp=hlp ;
                meUShort cc ;
                int lno=0 ;
                
                /* force debugging off while we are getting input from the user,
                 * if we don't and any macro is executed (idle-pick macro) then
                 * ME will spin and crash!! */
                macbug = 0 ;
                
                /*---   Generate the debugging line for the 'programmer' !! */
                do 
                    lno++ ;
                while ((tlp=lforw(tlp)) != lp)
                    ;
                sprintf((char *)outline,"%s:%d:%d [%s] ?",meRegCurr->commandName,lno,execlevel,tline) ;
loop_round2:
                mlwrite(MWSPEC,outline);        /* Write out the debug line */
                /* Cannot do update as if this calls a macro then
                 * we will end up in an infinite loop, the best we can do is
                 * call the screenUpdate function
                 */
                screenUpdate(TRUE,2-sgarbf) ;
                /* reset garbled status */
                sgarbf = FALSE ;
                if(dd <= 2)
                {
loop_round:
                    /* and get the keystroke */
                    cc = meGetKeyFromUser(FALSE,0,meGETKEY_SILENT|meGETKEY_SINGLE) ;
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
                            mlStatusStore = mlStatus ;
                            mlStatus = 0 ;
                            clexec = FALSE ;
                            descVariable(FALSE,1) ;
                            clexec = TRUE ;
                            mlStatus = mlStatusStore ;
                            goto loop_round ;
                        }
                    default:
                        if(cc == breakc)
                        {
                            /* Abort - Must exit the usual way so that macro
                             * storing and execlevel are corrected */
                            ctrlg(FALSE,1) ;
                            status = ABORT ;
                            errorLine = lp ;
                            macbug = dd ;
                            goto dobuf_exit ;
                        }
                        debug = dd ;
                    case '!':
                        dd = 0 ;
                    case 's':
                        break;
                    }
                }
                macbug = dd ;
            }
        }
#endif
        if(TTbreakTest(0))
        {
            mcStore = FALSE;
            lpStore = NULL;
            execlevel = 0 ;
            status = ctrlg(FALSE,1) ;
        }
        else
        {
            meUByte tkn[TOKENBUF] ;
            
            if(mcStore)
                status = domstore(tline) ;
            else
                status = docmd(tline,tkn) ;
#if     DEBUGM
            if(debug)
            {
                macbug = debug ;
                debug = 0 ;
            }
#endif
            switch(status)
            {
            case FALSE:
            case TRUE:
                break ;
                
            case DRGOTO:
            case DRTGOTO:
                {
                    register int linlen;        /* length of line to execute */
                    register LINE *glp;         /* line to goto */
                    
                    linlen = meStrlen(tkn) ;
                    glp = hlp->l_fp;
                    while (glp != hlp)
                    {
                        if (*glp->l_text == '*' &&
                            (meStrncmp(glp->l_text+1,tkn,linlen) == 0) &&
                            isSpace(glp->l_text[linlen+1]))
                        {
                            lp = glp;
                            status = TRUE;
                            break ;
                        }
                        glp = glp->l_fp;
                    }
                    
                    if (status == DRRGOTO)
                    {
                        mlwrite(MWABORT|MWWAIT,(meUByte *)"No such label");
                        status = FALSE;
                    }
                    break;
                }
            case DRREPEAT:                      /* REPEAT */
                if (rlp != NULL)
                {
                    mlwrite(MWABORT|MWWAIT,(meUByte *)"Nested Repeat");
                    status = FALSE;
                    break;
                }
                rlp = lp;               /* Save line */
                status = TRUE;
                break;
            case DRUNTIL:                       /* TRUE UNTIL */
                if (rlp == NULL)
                {
                    mlwrite(MWABORT|MWWAIT,(meUByte *)"No repeat set");
                    status = FALSE;
                    break;
                }
                rlp = NULL;
                status = TRUE;
                break;  
            case DRUNTILF:                      /* FALSE UNTIL */
                if (rlp == NULL)
                {
                    mlwrite(MWABORT|MWWAIT,(meUByte *)"No repeat set");
                    status = FALSE;
                    break;
                }
                lp = rlp;               /* Back to 'repeat' */
                status = TRUE;
                break; 
            case DRCONTIN:
                if (wlp == NULL)
                {
                    mlwrite(MWABORT|MWWAIT,(meUByte *)"No while");
                    status = FALSE;
                    break;
                }
                lp = wlp;
                continue;
                
            case DRDONE:        
                if (wlp == NULL)
                {
                    mlwrite(MWABORT|MWWAIT,(meUByte *)"No while");
                    status = FALSE;
                    break;
                }
                lp = wlp;
                wlp = NULL;
                continue;
                
            case DRWHILE:                       
                wlp = lp;
                status = TRUE;
                break;
                
            case DRJUMP:
            case DRTJUMP:
                if(relJumpTo < 0)
                    for( ; relJumpTo < 0 ; relJumpTo++)
                        lp = lp->l_bp ;
                else
                    for( ; relJumpTo > 0 ; relJumpTo--)
                        lp = lp->l_fp ;
                continue;
                
            case DRRETURN:      /* if it is a !RETURN directive...do so */
                status = TRUE ;
                goto dobuf_exit ;
            default:
                status = TRUE ;
                
            }
        }
        /* check for a command error */
        if (status != TRUE)
        {
            /* in any case set the buffer . */
            errorLine = lp;
            goto dobuf_exit ;
        }
        
        /* on to the next line */
        lp = lp->l_fp;
    }
    if(mcStore)
        mlwrite(MWABORT|MWWAIT,(meUByte *)"[Missing !emacro termination]");
    else
        status = TRUE ;
    
    /* exit the current function */
dobuf_exit:
    
    if(mcStore)
    {
        mcStore = 0 ;
        lpStore = NULL ;
        execlevel = 0 ;
        errorLine = hlp;
        status = FALSE ;
    }
    
    return status ;
}


static int
donbuf(LINE *hlp, meVARLIST *varList, meUByte *commandName, int f, int n)
{
    meREGISTERS  rp ;
    meUByte oldcle ;
    int oldexec, status ;
    
    /* save the arguments */
    oldcle = clexec;
    oldexec = execlevel ;
    /* Setup these argument and move the register stack */
    rp.prev = meRegCurr ;
    rp.commandName = commandName ;
    rp.execstr = execstr ;
    rp.varList = varList ;
    rp.f = f ;
    rp.n = n ;
    meRegCurr = &rp ;
    
    status = dobuf(hlp) ;
    
    /* restore old arguments & stack */
    meRegCurr = rp.prev ;
    execstr = rp.execstr ;
    execlevel = oldexec ;
    clexec = oldcle;
    
    cmdstatus = status ;
    return status ;
}

int
execFunc(int index, int f, int n)
{
    register int status;
    
    if(index < CK_MAX)
    {
        thisflag = 0 ;
        status = cmdTable[index]->func(f,n) ;
        lastflag = thisflag;
        if(selhilight.uFlags)
        {
            if((status == TRUE) || isComSelIgnFail(index))
            {
                if(isComSelKill(index))
                {
                    selhilight.bp = NULL ;
                    selhilight.flags = 0 ;
                }
                else
                {
                    if(isComSelStart(index))
                    {
                        selhilight.bp = curbp;                  /* Select the current buffer */
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
                           ((selhilight.mlineoff != curwp->w_marko) ||
                            (selhilight.mlineno != curwp->mlineno)))
                        {
                            selhilight.flags |= SELHIL_CHANGED ;
                            selhilight.mlineoff = curwp->w_marko;   /* Current mark offset */
                            selhilight.mlineno = curwp->mlineno;    /* Current mark line number */
                        }
                        if(isComSelSetDot(index) &&
                           ((selhilight.dlineoff != curwp->w_doto) ||
                            (selhilight.dlineno != curwp->line_no)))
                        {
                            selhilight.flags |= SELHIL_CHANGED ;
                            selhilight.dlineoff = curwp->w_doto;   /* Current mark offset */
                            selhilight.dlineno = curwp->line_no;    /* Current mark line number */
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
                            addModeToBufferWindows(selhilight.bp, WFSELHIL);
                    }
                }
            }
            else if(!meRegCurr->force && ((selhilight.flags & (SELHIL_ACTIVE|SELHIL_KEEP)) == SELHIL_ACTIVE))
                selhilight.flags &= ~SELHIL_ACTIVE ;
        }
    }
    else
    {
        meMACRO *mac ;
        meUByte firstExec ;                    /* set if this is first */
        LINE *hlp ;
        
        mac = getMacro(index) ;
        hlp = mac->hlp ;
        if(!(hlp->l_flag & MACEXEC))
        {
            if(hlp->l_flag & MACFILE)
            {
                if(mac->fname != NULL)
                    dofile(mac->fname,0,1) ;
                else
                    dofile(mac->name,0,1) ;
                hlp = mac->hlp ;
                if(hlp->l_flag & MACFILE)
                    return mlwrite(MWABORT,(meUByte *)"[file-macro %s not defined]",mac->name);
            }
            hlp->l_flag |= MACEXEC ;
            firstExec = 1 ;
        }
        else
            firstExec = 0 ;
        
        status = donbuf(hlp,&mac->varList,mac->name,f,n) ;
        
        if(firstExec)
        {
            if(mac->hlp != hlp)
                freeLineLoop(hlp,1) ;
            else
                hlp->l_flag &= ~MACEXEC ;
        }
    }
    
    return status ;
}

/* Execute the given command, but in a hidden way, i.e.
 * the 'key' is not recorded
 */
void
execFuncHidden(int keyCode, int index, meUInt arg)
{
    meUByte tf, lf, cs;
    int tc, ti, lc, li, ii ;
    int f, n ;
    
    cs = cmdstatus ;
    lc = lastCommand ;
    li = lastIndex ;
    lf = lastflag ;
    tc = thisCommand ;
    ti = thisIndex ;
    tf = thisflag ;
    lastCommand = tc ;
    lastIndex = ti ;
    thisCommand = keyCode ;
    thisIndex = index ;
    if(arg != 0)
    {
        f = 1 ;
        n = (int) (arg + 0x80000000) ;
    }
    else
    {
        f = 0 ;
        n = 1 ;
    }
    if((ii=curbp->inputFunc) >= 0)
    {
        meUByte *ss ;
        if((execFunc(ii,f,n) == TRUE) ||
           ((ss=getUsrLclCmdVar((meUByte *)"status",&(cmdTable[ii]->varList))) == errorm) || meAtoi(ss))
            index = -1 ;
    }
    if(index >= 0)
        execFunc(index,f,n);
    thisflag = tf;
    thisIndex = ti ;
    thisCommand = tc ;
    lastflag = lf;
    lastIndex = li ;
    lastCommand = lc ;
    cmdstatus = cs ;
}

void
execBufferFunc(BUFFER *bp, int index, int flags, int n)
{
    int movePos = ((flags & meEBF_HIDDEN) && (bp == curbp)) ; 
    meUByte tf, lf, cs;
    int tc, ti ;
    
    cs = cmdstatus ;
    tc = thisCommand ;
    ti = thisIndex ;
    tf = thisflag ;
    lf = lastflag ;
    thisIndex = index ;
    if(movePos)
    {
        /* drop an alpha mark and return back to it after */
        alphaMarkSet(curbp,meAM_EXECBUFF,curwp->w_dotp,curwp->w_doto,1) ;
        curwp->w_dotp = curbp->b_dotp ;
        curwp->w_doto = curbp->b_doto ;
        curwp->line_no = curbp->line_no ;
    }
    if(bp != curbp)
    {
        /* swap this buffer into the current window, execute the function
         * and then swap back out */
        register BUFFER *tbp = curbp ;
        meUInt w_flag;
        meInt topLineNo ;
        
        storeWindBSet(tbp,curwp) ;
        w_flag = curwp->w_flag ;
        topLineNo = curwp->topLineNo ;
        
        tbp->b_nwnd-- ;
        curbp = curwp->w_bufp = bp ;
        restoreWindBSet(curwp,bp) ;
        isWordMask = bp->isWordMask ;
        bp->b_nwnd++ ;
        
        execFunc(index,(flags & meEBF_ARG_GIVEN),n) ;
        
        curbp->b_nwnd-- ;
        storeWindBSet(curbp,curwp) ;
        curbp = curwp->w_bufp = tbp ;
        tbp->b_nwnd++ ;
        curwp->topLineNo = topLineNo ;
        /* force a check on update, just incase the Func has done something behind our back */
        curwp->w_flag |= (w_flag|WFMOVEL|WFMAIN) ;
        restoreWindBSet(curwp,tbp) ;
        isWordMask = tbp->isWordMask ;
    }
    else
        execFunc(index,(flags & meEBF_ARG_GIVEN),n) ;
    if(movePos)
    {
        /* return back to the alpha mark */
        alphaMarkGet(curbp,meAM_EXECBUFF) ;
        curwp->w_dotp = curbp->b_dotp ;
        curwp->w_doto = curbp->b_doto ;
        curwp->line_no = curbp->line_no ;
        curwp->w_flag |= WFMOVEL ;
    }
    thisflag = tf;
    lastflag = lf;
    thisIndex = ti ;
    thisCommand = tc ;
    cmdstatus = cs ;
}

static long
macroPrintError(LINE *hlp, meUByte *macName)
{
    LINE *lp ;
    long  ln ;
    
    if(errorLine == NULL)
        /* Quit if there is no error line */
        return -1 ;
    
    lp=lforw(hlp) ;
    ln=0 ;
    while(lp != errorLine)
    {
        lp = lforw(lp) ;
        ln++ ;
    }
    mlwrite(MWABORT|MWWAIT,(meUByte *)"[Error executing %s, line %d]",macName,ln+1) ;
    return ln ;
}

/*      execBuffer:     Execute the contents of a buffer of commands    */
int
execBuffer(int f, int n)
{
    register BUFFER *bp;                /* ptr to buffer to execute */
    register int s;                     /* status return */
    meVARLIST varList={NULL,0} ;
    meUByte  bufn[MAXBUF];                /* name of buffer to execute */
    meInt  ln ;
    
    /* find out what buffer the user wants to execute */
    if((s = getBufferName((meUByte *)"Execute buffer", 0, 1, bufn)) != TRUE)
        return s ;
    
    /* find the pointer to that buffer */
    if ((bp=bfind(bufn, 0)) == NULL)
        return mlwrite(MWABORT,(meUByte *)"No such buffer");
    
    /* and now execute it as asked */
    if(((s = donbuf(bp->b_linep,&varList,bp->b_bname,f,n)) != TRUE) &&
       ((ln = macroPrintError(bp->b_linep,bufn)) >= 0))
    {
        /* the execution failed lets go to the line that caused the grief */
        bp->b_dotp  = errorLine ;
        bp->line_no = ln ;
        bp->b_doto  = 0 ;
        /* look if buffer is showing */
        if(bp->b_nwnd)
        {
            register WINDOW *wp;        /* ptr to windows to scan */
            
            wp = wheadp;
            while (wp != NULL)
            {
                if (wp->w_bufp == bp)
                {
                    /* and point it */
                    wp->w_dotp = errorLine ;
                    wp->w_doto = 0 ;
                    wp->line_no = ln ;
                    wp->w_flag |= WFMOVEL ;
                }
                wp = wp->w_wndp;
            }
        }
    }
    /* free off any command variables */
    if(varList.head != NULL)
    {
        meVARIABLE *cv=varList.head, *ncv ;
        while(cv != NULL)
        {
            ncv = cv->next ;
            meNullFree(cv->value) ;
            free(cv) ;
            cv = ncv ;
        }
    }
    return s ;
}

/*      dofile: yank a file into a buffer and execute it
   if there are no errors, delete the buffer on exit */
/* Note for Dynamic file names
 * This ensures that nothing is done to fname
 */
int
dofile(meUByte *fname, int f, int n)
{
    meVARLIST     varList={NULL,0} ;
    meUByte         fn[FILEBUF] ;
    LINE          hlp ;
    register int  status ;      /* results of various calls */
    
    hlp.l_fp = &hlp ;
    hlp.l_bp = &hlp ;
    /* use a new buffer to ensure it doesn't mess with any loaded files */
    if(!fileLookup(fname,(meUByte *)".emf",meFL_CHECKDOT|meFL_USESRCHPATH,fn) ||
       (ffReadFile(fn,meRWFLAG_SILENT,NULL,&hlp) == ABORT))
        return mlwrite(MWABORT|MWCLEXEC,(meUByte *)"[Failed to load file %s]", fname);
    
    /* go execute it! */
    if((status = donbuf(&hlp,&varList,fn,f,n)) != TRUE)
        /* the execution failed lets go to the line that caused the grief */
        macroPrintError(&hlp,fn) ;
    freeLineLoop(&hlp,0) ;
    /* free off any command variables */
    if(varList.head != NULL)
    {
        meVARIABLE *cv=varList.head, *ncv ;
        while(cv != NULL)
        {
            ncv = cv->next ;
            meNullFree(cv->value) ;
            free(cv) ;
            cv = ncv ;
        }
    }
    
    return status ;
}

int
execFile(int f, int n)  /* execute a series of commands in a file */
{
    meUByte filename[MAXBUF];      /* Filename */
    int status;
    
    if ((status=meGetString((meUByte *)"Execute File",MLFILECASE, 0, filename, 
                        MAXBUF)) != TRUE)
        return status ;
    
    /* otherwise, execute it */
    return dofile(filename,f,n) ;
}

/* execute the startup file */
/* sfname       name of startup file (null if default) */

int
startup(meUByte *sfname)
{
    /* look up the startup file */
    if(sfname == NULL)
        sfname = (meUByte *)"me" ;
    
    return dofile(sfname,0,1) ;
}


/* execCommand: execute a named command even if it is not bound */

int
execCommand(int f, int n)
{
    register int idx ;       /* index to the requested function */
    meUByte buf[MAXBUF];       /* buffer to hold tentative command name */
    meUByte prm[30] ;
    
    /* setup prompt */
    if(f==TRUE)
        sprintf((char *)prm,"Arg %d: Command",n) ;
    else
        meStrcpy(prm,"Command") ;
    
    /* if we are executing a command line get the next arg and match it */
    if((idx = meGetString(prm, MLCOMMAND, 1, buf, MAXBUF)) != TRUE)
        return idx ;
    
    /* decode the function name, if -ve then duff */
    if((idx = decode_fncname(buf,0)) < 0)
        return(FALSE);
    
    /* and then execute the command */
    return execFunc(idx,f,n) ;
}


/*      execLine:       Execute a command line command to be typed in
   by the user                                     */

/*ARGSUSED*/
int
lineExec (int f, int n, meUByte *cmdstr)
{
    register int status;                /* status return */
    meUByte *oldestr;                     /* original exec string */
    meUByte  oldcle ;                     /* old contents of clexec flag */
    meUByte  tkn[TOKENBUF] ;
    int oldexec, oldF, oldN, oldForce ;
    
    /* save the arguments */
    oldcle = clexec;
    oldexec = execlevel ;
    oldestr = execstr;
    oldF = meRegCurr->f ;
    oldN = meRegCurr->n ;
    oldForce = meRegCurr->force ;
    
    clexec = TRUE;                      /* in cline execution */
    execstr = NULL ;
    execlevel = 0;                      /* Reset execution level */
    meRegCurr->f = f ;
    meRegCurr->n = n ;
    
    status = docmd(cmdstr,tkn) ;
    
    /* restore old arguments */
    execlevel = oldexec ;
    clexec = oldcle;
    execstr = oldestr ;
    meRegCurr->f = oldF ;
    meRegCurr->n = oldN ;
    meRegCurr->force = oldForce ;
    
    return status ;
}    


int
execLine(int f, int n)
{
    register int status;                /* status return */
    meUByte cmdstr[MAXBUF];               /* string holding command to execute */
    
    /* get the line wanted */
    if ((status = meGetString((meUByte *)":", 0, 0, cmdstr, MAXBUF)) != TRUE)
        return status ;
    
    return lineExec(f,n,cmdstr) ;
}

