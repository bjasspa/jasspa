/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * macro.c - Macro Handling routines.
 *
 * Copyright (C) 1995-2002 JASSPA (www.jasspa.com)
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
 * Created:     1995
 * Synopsis:    Macro Handling routines.
 * Authors:     Steve Philips
 */

#include "emain.h"
#include "efunc.h"

/*
 * Begin a keyboard macro.
 * Error if not at the top level in keyboard processing. Set up variables and
 * return.
 */
int
startKbdMacro(int f, int n)
{
    if (kbdmode != STOP)
    {
        mlwrite(0,(meUByte *)"Macro already active");
        return FALSE ;
    }
    mlwrite(0,(meUByte *)"[Start macro]");
    kbdptr = &lkbdptr[0];
    kbdlen = 0 ;
    kbdmode = RECORD;
    addModeToWindows(WFMODE) ;  /* and update ALL mode lines */
    return TRUE ;
}

/*
 * End keyboard macro. Check for the same limit conditions as the above
 * routine. Set up the variables and return to the caller.
 */
int
endKbdMacro(int f, int n)
{
    if (kbdmode == PLAY)
        return TRUE ;
    if (kbdmode == RECORD)
    {
        addModeToWindows(WFMODE) ;  /* and update ALL mode lines */
        mlwrite(0,(meUByte *)"[End macro]");
        kbdmode = STOP;

        lkbdlen = kbdlen ;
        return TRUE ;
    }
    return mlwrite(MWABORT,(meUByte *)"Macro not active");
}

/*
 * Execute a macro.
 * The command argument is the number of times to loop. Quit as soon as a
 * command gets an error. Return TRUE if all ok, else FALSE.
 */
int
execKbdMacro(int f, int n)
{
    if (kbdmode != STOP)
        return mlwrite(MWABORT,(meUByte *)"Macro already active!");
    if (n <= 0)
        return TRUE ;
    kbdptr = lkbdptr ;
    kbdlen = lkbdlen ;
    kbdoff = 0 ;
    kbdrep = n;         /* remember how many times to execute */
    kbdmode = PLAY;     /* start us in play mode */
    return TRUE ;
}

int
stringExec(int f, int n, meUByte *macro)
{
    meUByte *okbdptr;
    meUByte  oldcle ;      /* old contents of clexec flag */
    meUByte *oldestr ;	 /* original exec string */
    int  okbdlen, okbdoff, okbdmode, okbdrep ;
    int  oldexec, ii ;
    
    okbdptr = kbdptr ;
    okbdoff = kbdoff ;
    okbdlen = kbdlen ;
    okbdrep = kbdrep ;
    okbdmode = kbdmode ;
    oldcle = clexec;			/* save old clexec flag */
    clexec = FALSE;			/* in cline execution */
    oldestr = execstr;	/* save last ptr to string to execute */
    execstr = NULL ;
    oldexec = execlevel ;
    execlevel = 0;			/* Reset execution level */
    
    kbdptr = macro ;
    kbdoff = 0 ;
    kbdlen = meStrlen(macro) ;
    kbdrep = n ;
    kbdmode = PLAY;     /* start us in play mode */
    ii = TRUE ;
    while((kbdrep > 1) || (kbdoff < kbdlen))
    {
        if(TTbreakFlag || (kbdmode != PLAY))
        {
            ii = ABORT ;
            break ;
        }
        doOneKey() ;
    }
    kbdptr = okbdptr ;
    kbdoff = okbdoff ;
    kbdlen = okbdlen ;
    kbdrep = okbdrep ;
    kbdmode = okbdmode ;
    execlevel = oldexec ;
    clexec = oldcle;			/* restore clexec flag */
    execstr = oldestr;
    
    return ii ;
}

int
execString(int f, int n)
{
    meUByte sbuf[MAXBUF] ;
    
    if(meGetString((meUByte *)"Enter string", MLFFZERO, 0, sbuf, MAXBUF) != TRUE)
        return 0 ;
    return stringExec(f,n,sbuf) ;
}

/*
 * Asks the user if they wish to continue macro execution.
 * does nothing when recording, must STOP the macro execution so 
 * mlyesno goes to the keyboard.
 */
int
macroQuery(int f, int n)
{
    if(kbdmode == PLAY)
    {
        int   rr ;
        
        /* force a screen update */
        update(TRUE);
        kbdmode = STOP ;
        if((rr = mlyesno((meUByte *)"Continue macro")) == ABORT)
            return ABORT ;
        if(rr == FALSE)
        {
            kbdrep-- ;
            kbdoff = 0 ;
        }
        if(kbdrep)
            kbdmode = PLAY ;
        else
            return FALSE ;
    }
    else if(kbdmode != RECORD)
        return mlwrite(MWABORT,(meUByte *)"Not defining macro") ;
    return TRUE ;
}
    

static meMACRO *
createMacro(meUByte *name)
{
    meUByte buff[MAXBUF] ;
    register meMACRO *mac ;
    register LINE *hlp ;
    register int idx ;
    
    /* If the macro name has not been give then try and get one */
    if((name == NULL) && 
       (meGetString((meUByte *)"Enter macro name", MLCOMMAND, 0, buff, MAXBUF) == TRUE) && (buff[0] != '\0'))
        name = buff ;
    
    if((name == NULL) || ((hlp = lalloc(0)) == NULL))
        return NULL ;
    
    /* if it already exists */
    if((idx = decode_fncname(name,1)) >= 0)
    {
        /* if function return error, else clear the buffer */ 
        if(idx < CK_MAX)
        {
            mlwrite(MWABORT|MWPAUSE,(meUByte *)"Error! can't re-define a base function") ;
            meFree(hlp) ;
            return NULL ;
        }
        mac = getMacro(idx) ;
        if(!(mac->hlp->l_flag & MACEXEC))
        {
            if(mac->hlp->l_flag & MACFILE)
            {
                if(meNullFree(mac->fname))
                    mac->fname = NULL ;
            }
            freeLineLoop(mac->hlp,1) ;
        }
    }
    else
    {
        if(!(cmdTableSize & 0x1f))
        {
            /* run out of room in hte command table, malloc more */
            meCMD **nt ;
            if((nt = (meCMD **) meMalloc((cmdTableSize+0x20)*sizeof(meCMD *))) == NULL)
                return NULL ;
            memcpy(nt,cmdTable,cmdTableSize*sizeof(meCMD *)) ;
            if(cmdTable != __cmdTable)
                free(cmdTable) ;
            cmdTable = nt ;
        }
        if(((mac = (meMACRO *) meMalloc(sizeof(meMACRO))) == NULL) ||
           ((mac->name = meStrdup(name)) == NULL))
        {
            meFree(hlp) ;
            return NULL ;
        }
        mac->varList.head = NULL ;
        mac->varList.count = 0 ;
        mac->id = cmdTableSize ;
        mac->fname = NULL ;
        mac->callback = -1 ;
        cmdTable[cmdTableSize++] = (meCMD *) mac ;
        mac->hnext = NULL ;
        /* insert macro into the hash table and alphabetic list */
        {
            meUInt key ;
            meCMD **cmd ;
            
            key = cmdHashFunc(name) ;
            cmd = &(cmdHash[key]) ;
            while(*cmd != NULL)
                cmd = &((*cmd)->hnext) ;
            *cmd = (meCMD *) mac ;
            cmd = &(cmdHead) ;
            while((*cmd != NULL) && (meStrcmp((*cmd)->name,name) < 0))
                cmd = &((*cmd)->anext) ;
            mac->anext = *cmd ;
            *cmd = (meCMD *) mac ;
        }
    }
    mac->hlp = hlp ;
    hlp->l_fp = hlp ;
    hlp->l_bp = hlp ;
    return mac ;
}
/* macroDefine:	Set up a macro buffer and flag to store all
		executed command lines there			*/

int
macroDefine(int f, int n)
{
    register meMACRO *mac ;	/* pointer to macro */
    
    if((mac=createMacro(NULL)) == NULL)
        return FALSE ;
    if(n == 0)
        mac->hlp->l_flag |= MACHIDE ;
    
    /* and set the macro store pointers to it */
    mcStore = 1 ;
    lpStore = mac->hlp ;
    return TRUE ;
}

static meUByte helpFileName[] = "me.ehf" ;

static BUFFER *
helpBufferFind(void)
{
    BUFFER *hbp ;
    if((hbp=bfind(BolhelpN,BFND_CREAT)) == NULL)
        return NULL ;
    meModeClear(hbp->b_mode,MDUNDO) ;
    meModeClear(hbp->b_mode,MDCRYPT) ;
    meModeClear(hbp->b_mode,MDBINRY) ;
    meModeClear(hbp->b_mode,MDRBIN) ;
    meModeClear(hbp->b_mode,MDNACT) ;
    meModeSet(hbp->b_mode,MDHIDE) ;
    return hbp ;
}

void 
helpBufferReset(BUFFER *bp)
{
    WINDOW *wp ;
    meModeClear(bp->b_mode,MDEDIT) ;
    meModeSet(bp->b_mode,MDVIEW) ;
    bp->b_dotp = lforw(bp->b_linep) ;
    bp->b_doto = 0 ;
    bp->line_no = 0 ;
    bp->topLineNo = 0 ;
    bp->b_markp = NULL ;
    for (wp=wheadp; wp!=NULL; wp=wp->w_wndp)
        if (wp->w_bufp == bp)
        {
            restoreWindBSet(wp,bp) ;
        }
}

static int
helpBufferLoad(BUFFER *hbp)
{
    if(!meModeTest(hbp->b_mode,MDUSR1))
    {
        meUByte fname[FILEBUF] ;
    
        meModeSet(hbp->b_mode,MDUSR1) ;
        if(!fileLookup(helpFileName,NULL,meFL_CHECKDOT|meFL_USESRCHPATH,fname))
            return mlwrite(MWABORT,(meUByte *)"[Help file \"%s\" is not on-line]",helpFileName);
        /* and read the stuff in */
        meModeClear(hbp->b_mode,MDVIEW) ;
        ffReadFile(fname,meRWFLAG_SILENT,hbp,hbp->b_linep) ;
        helpBufferReset(hbp) ;
    }
    return TRUE ;
}

static int
findHelpItem(meUByte *item, int silent)
{
    WINDOW *wp ;
    BUFFER *bp, *hbp ;
    LINE   *lp, *elp ;
    int     sectLen, itemLen, ii ;
    meUByte  *ss, cc, sect[5] ;
    
    itemLen = meStrlen(item) ;
    if((item[itemLen-1] == ')') &&
       ((item[(ii=itemLen-3)] == '(') ||
        (item[(ii=itemLen-4)] == '(') ))
    {
        sectLen = itemLen-ii-2 ;
        meStrcpy(sect,item+ii) ;
        itemLen = ii ;
        item[itemLen] = '\0' ;
    }
    else
    {
        sectLen = 0 ;
        sect[0] = '\0' ;
        sect[1] = '\0' ;
    }
	
    if((hbp=helpBufferFind()) == NULL)
        return ABORT ;
    elp = hbp->b_linep ;
try_again:
    lp = lforw(elp) ;
    while(lp != elp)
    {
        if((lp->l_text[0] == '!') &&
           (!sectLen || 
            ((sect[1] == lp->l_text[2]) &&
             (((sectLen == 1) && (lp->l_text[3] == ' ')) || 
              (sect[2] == lp->l_text[3])))))
        {
            if((cc=lp->l_text[1]) == ' ')
            {
                ii = llength(lp) - 4 ;
                if(ii != itemLen)
                    ii = -1 ;
            }
            else
            {
                ii = cc - '0' ;
                if(ii > itemLen)
                    ii = -1 ;
            }
            if((ii > 0) && !meStrncmp(item,lp->l_text+4,ii))
                break ;
        }
        lp = lforw(lp) ;
    }
        
    if(lp == elp)
    {
        meMACRO *mac ;
        if(!meModeTest(hbp->b_mode,MDUSR1))
        {
            if(helpBufferLoad(hbp) == ABORT)
                return ABORT ;
            goto try_again ;
        }
        if((getMacroTypeS(item) == TKCMD) &&
           ((ii = decode_fncname(item,1)) >= CK_MAX) &&
           ((mac = getMacro(ii)) != NULL) &&
           (mac->hlp->l_flag & MACFILE))
        {
            meModeClear(hbp->b_mode,MDVIEW) ;
            if(mac->fname != NULL)
                dofile(mac->fname,0,1) ;
            else
                dofile(mac->name,0,1) ;
            helpBufferReset(hbp) ;
            if(!(mac->hlp->l_flag & MACFILE))
                goto try_again ;
        }
        if(!silent)
            mlwrite(MWABORT,(meUByte *)"[Can't find help on %s%s]",item,sect);
        return ABORT ;
    }
    if((wp = wpopup(BhelpN,(BFND_CREAT|BFND_CLEAR|WPOP_USESTR))) == NULL)
        return FALSE ;
    if((sectLen == 0) && (lp->l_text[2] != ' '))
    {
        ss = sect ;
        *ss++ = '(' ;
        *ss++ = lp->l_text[2] ;
        if(lp->l_text[3] != ' ')
            *ss++ = lp->l_text[3] ;
        *ss++ = ')' ;
        *ss = '\0' ;
    }
    
    bp = wp->w_bufp ;
    /* Add the header */
    {
        meUByte buff[MAXBUF] ;
        sprintf((char *)buff,"\033cD%s%s\033cA",lp->l_text+4,sect) ;
        addLineToEob(bp,buff) ;
        addLineToEob(bp,(meUByte *)"") ;
        addLineToEob(bp,(meUByte *)"\033lsMicroEmacs\033lm[Home]\033le \033lsCommand G\033lm[Commands]\033le \033lsVariable \033lm[Variables]\033le \033lsMacro Lan\033lm[Macro-Dev]\033le \033lsGlobal G\033lm[Glossary]\033le") ;
        memset(buff,boxChars[BCEW],78) ;
        buff[78] = '\0' ;
        addLineToEob(bp,buff) ;
        addLineToEob(bp,(meUByte *)"") ;
    }
    while(((lp=lforw(lp)) != elp) && (lp->l_text[0] == '!'))
        ;
    while((lp != elp) && ((cc=lp->l_text[0]) != '!'))
    {
        if(cc == '|')
        {
            if(meStrcmp(item,lp->l_text+1))
                lp = lforw(lp) ;
        }
        else if(cc == '$')
        {
            if(lp->l_text[1] == 'a')
            {
                if(sect[1] == '5')
                {
                    meUByte line[MAXBUF], *ss ;
                    if((ss = getval(item)) != NULL)
                    {
                        addLineToEob(bp,(meUByte *)"") ;
                        addLineToEob(bp,(meUByte *)"") ;
                        addLineToEob(bp,(meUByte *)"\033cEVALUE\033cA") ;
                        addLineToEob(bp,(meUByte *)"") ;
                        meStrcpy(line,"    \"") ;
                        meStrncpy(line+5,ss,MAXBUF-13) ;
                        line[MAXBUF-2] = '\0' ;
                        meStrcat(line,"\"") ;
                        addLineToEob(bp,line) ;
                    }
                }
                if(sect[1] == '2')
                {
                    if((ii = decode_fncname(item,1)) >= 0)
                    {
                        KEYTAB *ktp ;
                        meUByte line[MAXBUF], *ss ;
                        addLineToEob(bp,(meUByte *)"") ;
                        addLineToEob(bp,(meUByte *)"") ;
                        addLineToEob(bp,(meUByte *)"\033cEBINDINGS\033cA") ;
                        addLineToEob(bp,(meUByte *)"") ;
                        meStrcpy(line,"    ") ;
                        ss = line+4 ;
                        for(ktp = &keytab[0] ; ktp->code != ME_INVALID_KEY ; ktp++)
                        {
                            if(ktp->index == ii)
                            {
                                *ss++ = '"' ;
                                meGetStringFromKey(ktp->code,ss);
                                ss += meStrlen(ss) ;
                                *ss++ = '"' ;
                                *ss++ = ' ' ;
                            }
                        }
                        if(ss == line+4)
                            meStrcpy(ss,"none") ;
                        else
                            *ss = '\0' ;
                        addLineToEob(bp,line) ;
                    }
                }
            }
        }
        else
            addLineToEob(bp,lp->l_text) ;
        lp = lforw(lp) ;
    }
    /* Add the footer */
    {
        meUByte buff[MAXBUF] ;
        addLineToEob(bp,(meUByte *)"") ;
        memset(buff,boxChars[BCEW],78) ;
        buff[78] = '\0' ;
        addLineToEob(bp,buff) ;
        addLineToEob(bp,(meUByte *)"") ;
        sprintf((char *)buff,"\033lsCopyright\033lm%s\033le",meCopyright) ;
        addLineToEob(bp,buff) ;
    }
    bp->b_dotp = lforw(bp->b_linep);
    bp->b_doto = 0 ;
    bp->line_no = 0 ;
    meModeClear(bp->b_mode,MDEDIT) ;    /* don't flag this as a change */
    meModeSet(bp->b_mode,MDVIEW) ;      /* put this buffer view mode */
    resetBufferWindows(bp) ;            /* Update the window */
    mlerase(MWCLEXEC);	                /* clear the mode line */
    return TRUE ;
}

int
help(int f, int n)
{
    if(n == 0)
    {
        BUFFER *hbp ;
        if(((hbp=helpBufferFind()) == NULL) ||
           (helpBufferLoad(hbp) != TRUE))
            return ABORT ;
        return swbuffer(curwp,hbp);
    }
    return findHelpItem((meUByte *)"MicroEmacs",0) ;
}

int	
helpItem(int f, int n)
{
    meUByte buf[MAXBUF] ;
    
    if(meGetString((meUByte *)"Help on item", 0, 0, buf, MAXBUF-10) != TRUE)
        return FALSE ;
    return findHelpItem(buf,0) ;
}

int	
helpCommand(int f, int n)
{
    meUByte *ss, buf[MAXBUF] ;
              
    if(meGetString((meUByte *)"Help on command", MLCOMMAND, 0, buf, MAXBUF-10) != TRUE)
        return FALSE ;
    ss = buf + meStrlen(buf) ;
    meStrcpy(ss,"(2)") ;
    if(findHelpItem(buf,1) == TRUE)
        return TRUE ;
    meStrcpy(ss,"(3)") ;
    return findHelpItem(buf,0) ;
}

int	
helpVariable(int f, int n)
{
    meUByte buf[MAXBUF] ;

    if(meGetString((meUByte *)"Help on variable", MLVARBL, 0, buf, MAXBUF-10) != TRUE)
        return FALSE ;
    meStrcat(buf,"(5)") ;
    return findHelpItem(buf,0) ;
}

/* macroHelpDefine:
 * Set up a macro help definition
 */
int
macroHelpDefine(int f, int n)
{
    meUByte name[MAXBUF] ;
    meUByte sect[20] ;
    
    if((lpStoreBp=helpBufferFind()) == NULL)
        return ABORT ;
    if(meGetString((meUByte *)"Enter name", MLCOMMAND, 0, name+4, MAXBUF-4) != TRUE)
        return FALSE ;
    sect[0] = '\0' ;
    if(meGetString((meUByte *)"Enter section", 0, 0, sect, 20) == ABORT)
        return FALSE ;
    /* and set the macro store pointers to it */
    lpStore = lforw(lpStoreBp->b_linep) ;
    name[0] = '!' ;
    name[1] = (f) ? '0'+n:' ' ;
    if(sect[0] == '\0')
    {
        name[2] = ' ' ; 
        name[3] = ' ' ; 
    }
    else
    {
        name[2] = sect[0] ; 
        name[3] = (sect[1] == '\0') ? ' ':sect[1] ;
    }
    addLine(lpStore,name) ;
    lpStoreBp->elineno++ ;
    mcStore = 2 ;
    return TRUE ;
}


/* macroFileDefine:
 * Set up a macro buffer and flag to store all
 * executed command lines there			*/

int
macroFileDefine(int f, int n)
{
    register meMACRO *mac ;	/* pointer to macro */
    meUByte fname[MAXBUF] ;
    int ii=0 ;
    
    if(meGetString((meUByte *)"Enter file", MLCOMMAND, 0, fname, MAXBUF) != TRUE)
        return FALSE ;
    while((mac=createMacro(NULL)) != NULL)
    {
        if(n == 0)
            mac->hlp->l_flag |= MACHIDE ;
        mac->hlp->l_flag |= MACFILE ;
        mac->fname = meStrdup(fname) ;
        ii++ ;
    }
    if(ii == 0)
    {
        if((mac=createMacro(fname)) == NULL)
            return FALSE ;
        if(n == 0)
            mac->hlp->l_flag |= MACHIDE ;
        mac->hlp->l_flag |= MACFILE ;
    }
    return TRUE ;
}


int
nameKbdMacro(int f, int n)
{
    meUByte buf[MAXBUF] ;
    int ss ;
    
    if (kbdmode != STOP)
        return mlwrite(MWABORT,(meUByte *)"Macro already active!");
    if(lkbdlen <= 0)
        return mlwrite(MWABORT,(meUByte *)"No macro defined!") ;
    if((ss=macroDefine(FALSE, TRUE)) == TRUE)
    {
        meStrcpy(buf,"execute-string \"") ;
        n = expandexp(lkbdlen,lkbdptr,MAXBUF-2,16,buf,-1,NULL,meEXPAND_BACKSLASH|meEXPAND_FFZERO|meEXPAND_PRINTABLE) ;
        meStrcpy(buf+n,"\"") ;
        addLine(lpStore,buf) ;
    }
    mcStore = 0 ;
    lpStore = NULL ;
    return ss ;
}

meMACRO *
userGetMacro(meUByte *buf, int len)
{
    register int idx ;
    
    if(meGetString((meUByte *)"Enter macro name ", MLCOMMAND,2,buf,len) == TRUE)
    {        
        if((idx = decode_fncname(buf,0)) < 0)
            mlwrite(MWABORT,(meUByte *)"%s not defined",buf) ;
        else if(idx < CK_MAX)
            mlwrite(MWABORT,(meUByte *)"%s is a command",buf) ;
        else
            return getMacro(idx) ;
    }
    return NULL ;
}

int
insMacro(int f, int n)
{
    WINDOW          *wp ;
    register LINE   *lp, *slp ;
    meMACRO         *mac ;
    meUByte            buf[MAXBUF] ;
    meInt            nol, lineNo, len ;
    int              ii ;
    
    meStrcpy(buf,"define-macro ") ;
    if((mac=userGetMacro(buf+13, MAXBUF-13)) == NULL)
        return FALSE ;
    
    if((ii=bchange()) != TRUE)               /* Check we can change the buffer */
        return ii ;
    curwp->w_doto = 0 ;
    slp = curwp->w_dotp ;
    lineNo = curwp->line_no ;
    nol = addLine(slp,buf) ;
    len = meStrlen(buf) + 9 ;
    lp = lforw(mac->hlp);            /* First line.          */
    while (lp != mac->hlp)
    {
        nol += addLine(slp,lp->l_text) ;
        len += llength(lp) + 1 ;
        lp = lforw(lp);
    }
    nol += addLine(slp,(meUByte *)"!emacro") ;
    curbp->elineno += nol ;
    for (wp=wheadp; wp!=NULL; wp=wp->w_wndp)
        if (wp->w_bufp == curbp)
        {
            if(wp->line_no >= lineNo)
                wp->line_no += nol ;
            if(wp->mlineno >= lineNo)
                wp->mlineno += nol ;
            wp->w_flag |= WFMAIN|WFMOVEL ;
        }
#if MEUNDO
    if(meModeTest(curbp->b_mode,MDUNDO))
        meUndoAddInsChars(len) ;
#endif
    return TRUE ;
}
  

