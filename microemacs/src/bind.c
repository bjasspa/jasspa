/*****************************************************************************
*
*	Title:		%M%
*
*	Synopsis:	Binding functions.
*
******************************************************************************
*
*	Filename:		%P%
*
*	Author:			Daniel Lawrence
*
*	Creation Date:		11/02/86 12:37		<010718.1250>
*
*	Modification date:	%G% : %U%
*
*	Current rev:		%I%
*
*	Special Comments:	
*
*	Contents Description:	
*
*	This file is for functions having to do with key bindings,
*	descriptions, help commands and startup file.
*
*****************************************************************************
* 
* (C)opyright 1987 by Daniel M. Lawrence
* MicroEMACS 3.8 can be copied and distributed freely for any
* non-commercial purposes. MicroEMACS 3.8 can only be incorporated
* into commercial software with the permission of the current author.
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

/*---	Include defintions */

#define	__BINDC				/* Define file name */

/*---	Include files */

#include "emain.h"
#ifdef _UNIX
#include <sys/stat.h>
#endif
#include "efunc.h"
#include "esearch.h"
#include "eskeys.h"

uint16
meGetKeyFromString(uint8 **tp)
{
    uint8 *ss=*tp, cc, dd ;
    uint16 ii=0 ;
    
    for(;;)
    {
        if(((cc=*ss++) == '\0') ||
           ((dd=*ss++) != '-'))
            break ;
        if     (cc == 'C')
            ii |= ME_CONTROL ;
        else if(cc == 'S')
            ii |= ME_SHIFT ;
        else if(cc == 'A')
            ii |= ME_ALT ;
    }
    if(cc != '\0')
    {
        if(!isSpace(dd))
        {
            uint8 *s1=ss ;
            int skey ;
            
            while(((dd=*s1) != '\0') && (dd != ' '))
                s1++ ;
            *s1 = '\0' ;
            skey = biChopFindString(ss-2,13,specKeyNames,SKEY_MAX) ;
            *s1 = dd ;
            
            if(skey >= 0)
            {
                if(skey == SKEY_space)
                    ii |= 32 ;
                else
                    ii |= (skey|ME_SPECIAL) ;
                *tp = s1 ;
                return ii ;
            }
        }
        if(ii)
        {
            cc = toUpper(cc) ;
            if((ii == ME_CONTROL) && (cc >= 'A') && (cc <= '_'))
                ii = cc - '@' ;
            else
            {
                cc = toLower(cc) ;
                ii |= cc ;
            }
        }
        else
            ii |= cc ;
    }
    *tp = ss-1 ;
    return ii ;
}

/* meGetKey
 * 
 * get a key sequence from either the current macro execute line, a keyboard
 * macro or the keyboard
 * 
 *     flag - meGETKEY flags
 */
uint16
meGetKey(int flag)
{
    register uint16 cc ;	/* character fetched */
    
    /* check to see if we are executing a command line */
    if (clexec)
    {
        uint8 *tp;		/* pointer into the token */
        uint8 tok[MAXBUF];	/* command incoming */
        
        macarg(tok);	        /* get the next token */
        
        tp = tok ;
        cc = meGetKeyFromString(&tp) ;
        
        if(!(flag & meGETKEY_SINGLE))
        {
            int ii=ME_PREFIX_NUM+1 ;
            
            while(--ii > 0)
                if(cc == prefixc[ii])
                {
                    cc = ii << ME_PREFIX_BIT ;
                    break ;
                }
            if(cc & ME_PREFIX_MASK)
            {
                uint16 ee ;	/* character fetched */
                uint8  dd ;
                while(((dd=*tp) != '\0') && isSpace(dd))
                    tp++ ;
                ee = meGetKeyFromString(&tp) ;
                if((ee >= 'A') && (ee <= 'Z'))
                    /* with a prefix make a letter lower case */
                    ee ^= 0x20 ;
                cc |= ee ;
            }
        }
    }
    else
        /* or the normal way */
        cc = meGetKeyFromUser(FALSE,0,flag) ;
    return cc ;
}

int
meGetStringFromChar(uint16 cc, uint8 *d)
{
    register int  doff=0, spec ;
    register uint8 *ss ;
    
    if(cc & ME_ALT)
    {
        d[doff++] = 'A' ;
        d[doff++] = '-' ;
    }        
    if(cc & ME_CONTROL)
    {
        d[doff++] = 'C' ;
        d[doff++] = '-' ;
    }        
    if(cc & ME_SHIFT)
    {
        d[doff++] = 'S' ;
        d[doff++] = '-' ;
    }
    spec = (cc & ME_SPECIAL) ;
    cc &= 0x0ff ;
    
    if(spec)
    {
        register uint8 ch ;
        
        ss = specKeyNames[cc] ;
name_copy:
        while((ch = *ss++) != '\0')
            d[doff++] = ch ;
        return doff ;
    }
    
    if((cc > 0) && (cc <= 0x20))	/* control character */
    {
        if(cc == 0x20)
        {
            ss = specKeyNames[SKEY_space] ;
            goto name_copy ;
        }
        cc |= 0x40 ;
        cc = toLower(cc) ;
        d[doff++] = 'C' ;
        d[doff++] = '-' ;
        d[doff++] = (uint8) cc ;
    }
    else if((cc == 0) || (cc >= 0x80))
    {
        d[doff++] = '\\';
        d[doff++] = 'x';
        d[doff++] = hexdigits[cc >> 4] ;
        d[doff++] = hexdigits[cc & 0x0f] ;
    }
    else if((cc == '\\') || (cc == '"'))
    {
        d[doff++] = '\\';
        d[doff++] = (uint8) cc ;
    }
    else			/* any other character */
        d[doff++] = (uint8) cc ;
    
    return doff ;
}

/* change a key command to a string we can print out */
/* c   - 	sequence to translate */
/* seq - 	destination string for sequence */
void
meGetStringFromKey(uint16 cc, register uint8 * seq)
{
    /* apply prefix sequence if needed */
    if(cc & ME_PREFIX_MASK)
    {
        seq += meGetStringFromChar(prefixc[(cc&ME_PREFIX_MASK)>>ME_PREFIX_BIT],seq);
        *seq++ = ' ' ;
    }
    seq += meGetStringFromChar(cc,seq);
    *seq = 0;	/* terminate the string */
}

int
descKey(int f, int n)	/* describe the command for a certain key */
{
    static   uint8  notBound[] = "Not Bound" ;
    static   uint8  dskyPrompt[]="Show binding: " ;
    register uint16 cc;		/* command character to describe */
    register uint8 *ptr; 	/* string pointer to scan output strings */
    register int    found;	/* matched command flag */
    uint32          arg ;	/* argument */
    uint8           outseq[40];	/* output buffer for command sequence */
    uint8           argStr[20];	/* argument string */
    
    /* prompt the user to type us a key to describe */
    mlwrite(MWCURSOR,dskyPrompt);
    
    /* get the command sequence to describe */
    cc = meGetKey(meGETKEY_SILENT);	/* get a silent command sequence */
    
    /* change it to something we can print as well */
    meGetStringFromKey(cc, outseq);
    
    /* find the right function */
    if ((found = decode_key(cc,&arg)) < 0)
    {
        ptr = notBound ;
        argStr[0] = 0 ;
    }
    else
    {
        ptr = getCommandName(found) ;
        if(arg)
            sprintf((char *) argStr,"%d ",(int) (arg + 0x80000000)) ;
        else
            argStr[0] = 0 ;
    }

    /* output the resultant string */
    mlwrite(MWCURSOR,(uint8 *)"%s\"%s\" %s%s",dskyPrompt,outseq,argStr,ptr);
    
    return TRUE ;
}


uint32
cmdHashFunc (register uint8 *cmdName)
{
    uint32 ii = 0;
    uint32 jj;
    uint8 cc;
    
    while ((cc = *cmdName++) != '\0')
    {
        ii = (ii << 4) + cc;
        if ((jj = ii & 0xf0000000) != 0)
        {
            ii ^= (jj >> 24);
            ii ^= jj;
        }
    }
    return ii % cmdHashSize ;
}
     

/* match fname to a function in the names table
   and return index into table or -1 if none	
   fname;	name to attempt to match */
int
decode_fncname(register uint8 *fname, int silent)
{
    uint32 key ;
    meCMD *cmd ;
    
    key = cmdHashFunc(fname) ;
    for (cmd=cmdHash[key] ; cmd != NULL; cmd = cmd->hnext)
        if (meStrcmp (fname, cmd->name) == 0)
            return cmd->id ;
    
    if(!silent)
        mlwrite(MWABORT,(uint8 *)"[No such command \"%s\"]", fname);
    
    return -1 ;				/* Not found - error. */
}

void
charMaskTblInit(void)
{
    int ii ;
    for(ii=0 ; ii<128 ; ii++)
    {
        charLatinUserTbl[ii] = ii ;
        charUserLatinTbl[ii] = ii ;
        charMaskTbl2[ii] &= ~(CHRMSK_LOWER|CHRMSK_UPPER|CHRMSK_HEXDIGIT|CHRMSK_SPLLEXT) ;
        charCaseTbl[ii] = ii ;
    }
    for( ; ii<256 ; ii++)
    {
        charLatinUserTbl[ii] = ii ;
        charUserLatinTbl[ii] = ii ;
        charMaskTbl2[ii] = 0x00 ;
        charCaseTbl[ii] = ii ;
    }
#ifdef _UNIX
    /* 0xA0 is the No-Break SPace char - nothing is drawn! */
    for(ii=128 ; ii<161 ; ii++)
        charMaskTbl1[ii] = 0x0A ;
#else
    for(ii=128 ; ii<160 ; ii++)
        charMaskTbl1[ii] = 0x3A ;
#endif
    for( ; ii<256 ; ii++)
        charMaskTbl1[ii] = 0x7A ;
    for(ii='0' ; ii<='9' ; ii++)
        charMaskTbl2[ii] |= CHRMSK_HEXDIGIT ;
    for(ii='A' ; ii<='F' ; ii++)
    {
        charMaskTbl2[ii] |= (CHRMSK_UPPER|CHRMSK_HEXDIGIT) ;
        charCaseTbl[ii] = (ii^0x20) ;
    }
    for( ; ii<='Z' ; ii++)
    {
        charMaskTbl2[ii] |= CHRMSK_UPPER ;
        charCaseTbl[ii] = (ii^0x20) ;
    }
    for(ii='a' ; ii<='f' ; ii++)
    {
        charMaskTbl2[ii] |= (CHRMSK_LOWER|CHRMSK_HEXDIGIT) ; ;
        charCaseTbl[ii] = (ii^0x20) ;
    }
    for( ; ii<='z' ; ii++)
    {
        charMaskTbl2[ii] |= CHRMSK_LOWER ;
        charCaseTbl[ii] = (ii^0x20) ;
    }
    charMaskTbl2[0x27] |= CHRMSK_SPLLEXT ;
    charMaskTbl2[0x2d] |= CHRMSK_SPLLEXT ;
    charMaskTbl2[0x2e] |= CHRMSK_SPLLEXT ;
    charMaskTbl2[0x5f] |= CHRMSK_USER1 ;
}


int
setCharMask(int f, int n)
{
    static uint8 tnkyPrompt[]="set-char-mask chars" ;
    uint8 buf1[MAXBUF], *ss, *dd ;
    int	  c1, c2, flags, ii, jj ;
    uint8 mask1, mask2 ;
    
    meStrcpy(tnkyPrompt+14,"flags") ;
    if(meGetString(tnkyPrompt,0,0,buf1,20) != TRUE)
        return FALSE ;
    /* setup the masks */
    flags = 0 ;
    ss = buf1 ;
    while((c1 = *ss++) != '\0')
        if((dd = (uint8 *) strchr((char *) charMaskFlags,c1)) != NULL)
            flags |= 1 << (((int) dd)- ((int) charMaskFlags)) ;
    mask2 = (flags & 0x00ff) ;
    mask1 = (flags & 0x0f00) >> 4 ;
    
    if(n == 0)
    {
        ss = (uint8 *) resultStr ;
        if(flags & 0x1c000)
        {
            if(flags & 0x4000)
                flags |= 0x18000 ;
            for(c1=0 ; c1<256 ; c1++)
            {
                c2 = charLatinUserTbl[c1] ;
                if(c1 != c2)
                {
                    if(flags & 0x8000)
                    {
                        if(c1 == 0)
                        {
                            *ss++ = 255 ;
                            c1 = 1 ;
                        }
                        else if(c1 == 255)
                            *ss++ = 255 ;
                        *ss++ = c1 ;
                    }
                    if(flags & 0x10000)
                    {
                        if(c2 == 0)
                        {
                            *ss++ = 255 ;
                            c2 = 1 ;
                        }
                        else if(c2 == 255)
                            *ss++ = 255 ;
                        *ss++ = c2 ;
                    }
                }
            }
        }
        else
        {
            if(flags & 0x1000)
                mask2 |= CHRMSK_LOWER ;
            if(flags & 0x2000)
                mask2 |= (CHRMSK_LOWER|CHRMSK_HEXDIGIT) ;
            for(c1=0 ; c1<256 ; c1++)
                if((charMaskTbl1[c1] & mask1) ||
                   (charMaskTbl2[c1] & mask2))
                {
                    if((flags & 0x3000) &&
                       ((c2 = toggleCase(c1)) != c1))
                    {
                        if(c2 == 0)
                        {
                            *ss++ = 255 ;
                            c2 = 1 ;
                        }
                        else if(c2 == 255)
                            *ss++ = 255 ;
                        *ss++ = c2 ;
                    }
                    if(c1 == 0)
                    {
                        *ss++ = 255 ;
                        c1 = 1 ;
                    }
                    else if(c1 == 255)
                        *ss++ = 255 ;
                    *ss++ = c1 ;
                }
        }
        *ss = '\0' ;
    }
    else
    {
#if	MAGIC
        /* reset the last magic search string to force a recompile as char groups may change */
        mereRegexInvalidate() ;
#endif
        if(flags & 0x1A003)
            return mlwrite(MWABORT,(uint8 *)"[Cannot set flags u, l, A, L or U]");
        meStrcpy(tnkyPrompt+14,"chars") ;
        if(meGetString(tnkyPrompt,MLFFZERO,0,buf1,MAXBUF) != TRUE)
            return FALSE ;
        if(flags & 0x4000)
        {
            /* reset the tables */
            charMaskTblInit() ;
            ss = buf1 ;
            while((c1=*ss++) != '\0')
            {
                if((c1 == 255) && ((c1=*ss++) == 1))
                    c1 = 0 ;
                if((c2=*ss++) == '\0')
                    break ;
                if((c2 == 255) && ((c2=*ss++) == 1))
                    c2 = 0 ;
                charLatinUserTbl[c1] = c2 ;
                charUserLatinTbl[c2] = c1 ;
            }
            return TRUE ;
        }
        /* convert the string to user font and take out any 255 padding, 0's are now 0's so need int count */
        ss = buf1 ;
        dd = buf1 ;
        ii = 0 ;
        while((c1=*ss++) != '\0')
        {
            if((c1 == 255) && ((c1=*ss++) == 1))
                c1 = 0 ;
            *dd++ = toUserFont(c1) ;
            ii++ ;
        }
        if(n < 0)
        {
            if(flags & 0x1000)
                mask2 |= CHRMSK_ALPHA ;
            if(mask1 != 0)
            {
                mask1 ^= 0xff ;
                for(jj=0 ; jj<ii ; jj++)
                    charMaskTbl1[buf1[jj]] &= mask1 ;
            }
            if(mask2 != 0)
            {
                mask2 ^= 0xff ;
                for(jj=0 ; jj<ii ; jj++)
                    charMaskTbl2[buf1[jj]] &= mask2 ;
            }
        }
        else
        {
            if(flags & 0x1000)
            {
                for(jj=0 ; jj<ii ; )
                {
                    c1 = buf1[jj++] ;
                    c2 = buf1[jj++] ;
                    charMaskTbl2[c1] |= CHRMSK_UPPER ;
                    charMaskTbl2[c2] |= CHRMSK_LOWER ;
                    charCaseTbl[c1] = c2 ;
                    charCaseTbl[c2] = c1 ;
                }
            }
            if(mask1 != 0)
            {
                for(jj=0 ; jj<ii ; jj++)
                    charMaskTbl1[buf1[jj]] |= mask1 ;
            }
            if(mask2 != 0)
            {
                for(jj=0 ; jj<ii ; jj++)
                    charMaskTbl2[buf1[jj]] |= mask2 ;
            }
        }
    }
    return (TRUE);
}

/* bindtokey:	add a new key to the key binding table
*/

int
bindkey(uint8 *prom, int f, int n, uint16 *lclNoBinds, KEYTAB **lclBinds)
{
    register int	ss;
    register uint16 cc;	/* command key to bind */
    register Fintii	kfunc;	/* ptr to the requexted function to bind to */
    register KEYTAB    *ktp;	/* pointer into the command table */
    uint8    outseq[40];        /* output buffer for keystroke sequence */
    int	     namidx;		/* Name index */
    uint8    buf[MAXBUF] ;
    uint32 arg ;
    uint16 ssc ;
    
    /*---	Get the function name to bind it to */
    
    if(meGetString(prom, MLCOMMAND, 0, buf, MAXBUF) != TRUE)
        return FALSE ;
    if((namidx = decode_fncname(buf,0)) < 0)
        return FALSE ;
    kfunc = getCommandFunc(namidx) ;

    /*---	Prompt the user to type in a key to bind */
    /*---	Get the command sequence to bind */
    
    if((ss=(kfunc==(Fintii) prefixFunc)))
    {
        if((n < 1) || (n > ME_PREFIX_NUM))
            return mlwrite(MWABORT,(uint8 *)"[Prefix number is out of range]") ;
        ssc = prefixc[n] ;
    }
    else if((ss=(kfunc==(Fintii) ctrlg)))
        ssc = breakc ;
    else if((ss=(kfunc==(Fintii) uniArgument)))
        ssc = reptc ;

#if LCLBIND
    if((lclNoBinds != NULL) && ss)
        return mlwrite(MWABORT,(uint8 *)"Can not locally bind [%s]",buf) ;
#endif              
    mlwrite(MWCURSOR|MWCLEXEC,(uint8 *)"%s [%s] to: ",prom,buf) ;
    cc = meGetKey((ss) ? meGETKEY_SILENT|meGETKEY_SINGLE:meGETKEY_SILENT) ;
    
    /*---   Change it to something we can print as well */
    
    if (!clexec)
    {
        meGetStringFromKey(cc,outseq);
        mlwrite(0,(uint8 *)"%s [%s] to: %s",prom,buf,outseq);
    }
    
    /*---   Calculate the argument */
    if(f)
        arg = (uint32) (n+0x80000000) ;
    else
        arg = 0 ;
    
    /*---	If the function is a prefix key */
    
    if(ss)
    {
	/*---	Search for an existing binding for the prefix key */
	if(ssc != ME_INVALID_KEY)
            delete_key(ssc);
        
	/*---	Reset the appropriate global prefix variable */
        if(kfunc == (Fintii) prefixFunc)
            prefixc[n] = cc ;
        else if(kfunc ==(Fintii) ctrlg)
            breakc = cc ;
        else
            reptc = cc ;
    }
    
    /*---	Search the table to see if it exists */
#if LCLBIND
    if(lclNoBinds != NULL)
    {
        register KEYTAB *kk=NULL ;
        
        ktp = *lclBinds ;
        for (ss=0 ; ss < *lclNoBinds ; ss++)
        {
            if(ktp[ss].code == cc)
            {
                ktp[ss].index = (uint16) namidx;
                ktp[ss].arg   = arg ;
                return TRUE ;
            }
            if(ktp[ss].code == 0)
                kk = ktp+ss ;
        }
        if(kk != NULL)
        {
            kk->code  = cc ;
            kk->index = (uint16) namidx;
            kk->arg   = arg ;
        }
        else
        {
            if((ktp = *lclBinds = 
                meRealloc(ktp,(++(*lclNoBinds))*sizeof(KEYTAB))) == NULL)
                return FALSE ;
            ktp[ss].code  = cc ;
            ktp[ss].index = (uint16) namidx ;
            ktp[ss].arg   = arg ;
        }
    }
    else
#endif
    {
        for (ktp = &keytab[0]; ktp->code != ME_INVALID_KEY; ktp++)
            if (ktp->code == cc)
            {
                ktp->index = (uint16) namidx;
                ktp->arg   = arg ;
                return TRUE ;
            }
        
        /*---	Otherwise we  need to  add it  to the  end, if  we run  out of
           binding room, bitch */
        
        if(insert_key(cc, (uint16) namidx, arg) != TRUE)
            return mlwrite(MWABORT,(uint8 *)"Binding table FULL!");
    }
    return(TRUE);
}


int
globalBind(int f, int n)
{
    return bindkey((uint8 *)"global bind", f, n, NULL, NULL) ;
}

#if LCLBIND
int
bufferBind(int f, int n)
{
    return bindkey((uint8 *)"local bind", f, n, &(curbp->nobbinds), &(curbp->bbinds)) ;
}
int
mlBind(int f, int n)
{
    return bindkey((uint8 *)"ml bind", f, n, &mlNoBinds, &mlBinds) ;
}
#endif

/* unbindkey:	delete a key from the key binding table */

int
unbindkey(uint8 *prom, int n, uint16 *lclNoBinds, KEYTAB **lclBinds)
{
    register uint16 cc;	/* command key to unbind */
    uint8 outseq[40];	/* output buffer for keystroke sequence */
    int  rr ;
    
    if(n < 0)
    {
        sprintf((char *)outseq,"Remove all %s binds",prom);
        if(mlyesno(outseq) != TRUE)
            return ctrlg(FALSE,1) ;
        
#if LCLBIND
        if(lclNoBinds != NULL)
        {
            if(meNullFree(*lclBinds))
            {
                *lclBinds = NULL ;
                *lclNoBinds = 0 ;
            }
        }
        else
#endif
        {
            keyTableSize = 0 ;
            keytab[0].code = ME_INVALID_KEY ;
            
            /* remove all the prefixes */
            rr = ME_PREFIX_NUM+1;
            while(--rr > 0)
                prefixc[rr] = ME_INVALID_KEY ;
            breakc = ME_INVALID_KEY ;
            reptc = ME_INVALID_KEY ;
#if LCLBIND
            {
                BUFFER *bp ;
                
                /* loop through removing all buffer bindings */
                bp = bheadp ;
                while(bp != NULL)
                {
                    if(meNullFree(bp->bbinds))
                    {
                        bp->bbinds = NULL ;
                        bp->nobbinds = 0 ;
                    }
                    bp = bp->b_bufp ;
                }
                /* remove all ml bindings */
                if(meNullFree(mlBinds))
                {
                    mlBinds = NULL ;
                    mlNoBinds = 0 ;
                }
            }
#endif
        }
        return TRUE ;
    }
        
    /*---	Prompt the user to type in a key to unbind */
    
    /* if interactive then print prompt */
    mlwrite(MWCURSOR|MWCLEXEC,(uint8 *)"%s unbind: ",prom);
    
    /*---	Get the command sequence to unbind */
    
    /* get a command sequence */
    cc = meGetKey((n==0) ? meGETKEY_SILENT|meGETKEY_SINGLE:meGETKEY_SILENT);
    
    /* If interactive then there is no really need to print the string because
     * you will probably not see it !! The TTputc() is dangerous and squirts
     * characters over the screen if the previous mlwrite() determined that
     * we were in a macro and has not moved the cursor to the message line.
     * Hence, safer bet is to re-do the mlwrite(); my preference would be
     * to remove the keyboard echo !! - Jon Green 13/03/97 */
    meGetStringFromKey(cc, outseq);		/* change to something printable */
    mlwrite(MWCURSOR|MWCLEXEC,(uint8 *)"%s unbind: %s", prom, outseq);
#if LCLBIND
    if(lclNoBinds != NULL)
    {
        register KEYTAB *ktp ;
        register int	 ss;
        
        rr = FALSE ;
        ktp = *lclBinds ;
        ss = *lclNoBinds ;
        while(--ss >= 0)
        {
            if(ktp[ss].code == cc)
            {
                ktp[ss].code = ME_INVALID_KEY ;
                rr = TRUE ;
            }
        }
    }
    else
#endif
    {
        if((rr = delete_key(cc)) == TRUE)
        {
            if(n == 0)
            {
                register uint16 mask=ME_PREFIX_NUM+1;
                
                while(--mask > 0)
                    if(cc == prefixc[mask])
                        break ;
                if(mask)
                {
                    int ii= (int) keyTableSize ;
                    
                    prefixc[mask] = ME_INVALID_KEY ;
                    mask <<= ME_PREFIX_BIT ;
                    while(--ii>=0)
                        if((keytab[ii].code & ME_PREFIX_MASK) == mask)
                            delete_key(keytab[ii].code) ;
#if LCLBIND
                    {
                        register int ss ;
                        BUFFER *bp ;
                        
                        /* loop through all buffer bindings using prefix */
                        bp = bheadp ;
                        while(bp != NULL)
                        {
                            ss = bp->nobbinds ;
                            while(--ss >= 0)
                                if((bp->bbinds[ss].code & ME_PREFIX_MASK) == mask)
                                    bp->bbinds[ss].code = ME_INVALID_KEY ;
                            bp = bp->b_bufp ;
                        }
                        /* loop through all ml bindings using prefix */
                        ss = mlNoBinds ;
                        while(--ss >= 0)
                            if((mlBinds[ss].code & ME_PREFIX_MASK) == mask)
                                mlBinds[ss].code = ME_INVALID_KEY ;
                    }
#endif
                }
            }
            if (breakc == cc)
                breakc = ME_INVALID_KEY ;
            if (reptc == cc)
                reptc = ME_INVALID_KEY ;
        }
    }
    if(rr == FALSE)	/* if it isn't bound, bitch */
        return mlwrite(MWABORT|MWCLEXEC,(uint8 *)"[%s Key \"%s\" not bound]", prom,outseq);
    return TRUE ;
}

int
globalUnbind(int f, int n)
{
    return unbindkey((uint8 *)"Global",n, NULL, NULL) ;
}

#if LCLBIND
int
bufferUnbind(int f, int n)
{
    return unbindkey((uint8 *)"Local",n, &(curbp->nobbinds), &(curbp->bbinds)) ;
}
int
mlUnbind(int f, int n)
{
    return unbindkey((uint8 *)"ML",n, &mlNoBinds, &mlBinds) ;
}
#endif


/* build a binding list (limited or full) */
/* type  	true = full list,   false = partial list */
/* mstring	match string if a partial list */
static int
buildlist(int type, int n, uint8 *mstring)
{
    meCMD    *cmd ;
    KEYTAB   *ktp;	        /* pointer into the command table */
    WINDOW   *wp;
    BUFFER   *bp;	        /* buffer to put binding list into */
    int       cpos;   		/* current position to use in outseq */
    uint8     _outseq[MAXBUF]; 	/* output buffer for keystroke sequence */
    uint8    *outseq=_outseq+4;	/* output buffer for keystroke sequence */
    int       curidx;           /* Name integer index. */
    char      op_char;		/* Output character */
    
    /* and get a buffer for it */
    if((wp = wpopup(BcommandsN,(BFND_CREAT|BFND_CLEAR|WPOP_USESTR))) == NULL)
        return mlwrite(MWABORT,(uint8 *)"[Failed to create list]") ;
    bp = wp->w_bufp ;
    
    _outseq[0] = ' ' ;
    _outseq[1] = ' ' ;
    _outseq[2] = ' ' ;
    _outseq[3] = ' ' ;
    wp->w_markp = NULL;
    wp->w_marko = 0;
    
    /* build the contents of this window, inserting it line by line */
    for(cmd=cmdHead ; cmd != NULL ; cmd=cmd->anext)
    {
        /*---	If we are  executing an  apropos command  and current string
        doesn't include the search string */
        if ((type == FALSE) && !regexStrCmp(cmd->name,mstring,1))
            continue;		/* Do next loop */
        /*---	Add in the command name */
        meStrcpy(outseq, cmd->name);
        curidx = cmd->id ;
        cpos = meStrlen(outseq);
        
        /*---	Search down any keys bound to this */
        
        for (ktp = &keytab[0] ; ktp->code != ME_INVALID_KEY ; ktp++)
        {
            if (ktp->index == curidx)
            {
                /*---	Padd out some  spaces.  If we  are on a line
                with a  string then  pad  with '.'  else use
                space's */
                op_char = ((cpos) ? '.' : ' ');
                outseq[cpos++] = ' ';
                while (cpos < 31)
                    outseq[cpos++] = op_char;
                outseq[cpos++] = ' ';
                outseq[cpos++] = '"';
                /*---	Add in the command sequence */
                meGetStringFromKey(ktp->code, &outseq[cpos]);
                meStrcat(outseq+cpos,"\"");
                
                /* and add it as a line into the buffer */
                addLineToEob(bp,_outseq) ;
                cpos = 0;	/* and clear the line */
            }
        }
        
        /* if no key was bound, we need to dump it anyway */
        if (cpos > 0)
            addLineToEob(bp,_outseq) ;
    }
    meModeClear(bp->b_mode,MDEDIT) ;    /* don't flag this as a change */
    meModeSet(bp->b_mode,MDVIEW) ;      /* put this buffer view mode */
    wp->w_dotp = lforw(bp->b_linep);    /* back to the beginning */
    wp->w_doto = 0;
    mlerase(MWCLEXEC);          	/* clear the mode line */
    return(TRUE);
}


/* list-functions
 * bring up a fake buffer and list the key bindings into it with view mode
 */
int
listCommands(int f, int n)
{
    return (buildlist(TRUE, n, (uint8 *) ""));
}

int
commandApropos(int f, int n)	/* Apropos (List functions that match a substring) */
{
    uint8 mstring[NSTRING];	/* string to match cmd names to */
    int	  status;		/* status return */

    if ((status = meGetString((uint8 *)"Apropos string", MLCOMMAND, 0, 
                          mstring+2, NSTRING-4)) != TRUE)
        return status ;
    mstring[0] = '.' ;
    mstring[1] = '*' ;
    meStrcat(mstring,".*") ;
    return buildlist(FALSE, n, mstring);
}


/*
 * listbindings
 * List ALL of the bindings held in the system.
 */
static void
showBinding(BUFFER *bp, KEYTAB *ktp)
{
    uint8 *ss, buf[MAXBUF], outseq[40];
    int len;
    
    /* If the keyboard code is zero then the command has been unbound.
     * Situation occurs when the buffer or ml binding list is traversed.
     * We test at point of print rather than at the caller so that we do
     * not repeat the test. Speed is not a crital issue here. */
    if (ktp->code == ME_INVALID_KEY)
        return;
    
    /* The key command is non-zero therefore it is legal - print it */
    meGetStringFromKey(ktp->code, outseq);
    len = sprintf((char *)buf, "    \"%s\" ", outseq);
    while (len < 35)
        buf[len++] = '.';
    ss = buf+len ;
    *ss++ = ' ';
    if(ktp->arg)
        sprintf((char *)ss,"%d ",(int) (ktp->arg + 0x80000000)) ;
    else
        *ss = 0 ;
    meStrcat(ss,getCommandName(ktp->index)) ;
    addLineToEob(bp,buf) ;
}

int
descBindings (int f, int n)
{
    WINDOW *wp ;
    BUFFER *bp ;
    uint8   buf[MAXBUF] ;
    int     ii ;
    KEYTAB *ktp;
    
    if((wp = wpopup(BbindingsN,(BFND_CREAT|BFND_CLEAR|WPOP_USESTR))) == NULL)
        return FALSE ;
    bp = wp->w_bufp ;
    
#if LCLBIND
    sprintf((char *)buf,"Buffer [%s] bindings:", curbp->b_bname);
    addLineToEob(bp,buf);
    addLineToEob(bp,(uint8 *)"") ;
    
    ii  = curbp->nobbinds ;
    ktp = curbp->bbinds ;
    while(ii--)
        showBinding(bp,ktp++) ;
    addLineToEob(bp,(uint8 *)"") ;
    
    sprintf((char *)buf,"Ml bindings:") ;
    addLineToEob(bp,buf);
    addLineToEob(bp,(uint8 *)"") ;
    
    ii  = mlNoBinds ;
    ktp = mlBinds ;
    while(ii--)
        showBinding(bp,ktp++) ;
    addLineToEob(bp,(uint8 *)"") ;
#endif    
    
    addLineToEob(bp,(uint8 *)"Global bindings:");
    addLineToEob(bp,(uint8 *)"") ;
    
    ii  = keyTableSize ;
    ktp = keytab ;
    while(ii--)
        showBinding(bp,ktp++) ;
    
    bp->b_dotp = lforw(bp->b_linep);
    bp->b_doto = 0 ;
    bp->line_no = 0 ;
    
    meModeClear(bp->b_mode,MDEDIT) ;    /* don't flag this as a change */
    meModeSet(bp->b_mode,MDVIEW) ;      /* put this buffer view mode */
    resetBufferWindows(bp) ;            /* Update the window */
    mlerase(MWCLEXEC);	                /* clear the mode line */
    return TRUE ;
}
     

