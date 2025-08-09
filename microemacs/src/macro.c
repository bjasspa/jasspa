/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * macro.c - Macro Handling routines.
 *
 * Copyright (C) 1995-2024 JASSPA (www.jasspa.com)
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
    if(kbdmode != meSTOP)
    {
        mlwrite(0,(meUByte *)"Macro already active");
        return meFALSE;
    }
    mlwrite(0,(meUByte *)"[Start macro]");
    kbdptr = &lkbdptr[0];
    kbdlen = 0;
    kbdmode = meRECORD;
    frameAddModeToWindows(WFMODE);  /* and update ALL mode lines */
    return meTRUE;
}

/*
 * End keyboard macro. Check for the same limit conditions as the above
 * routine. Set up the variables and return to the caller.
 */
int
endKbdMacro(int f, int n)
{
    if(kbdmode == mePLAY)
        return meTRUE;
    if(kbdmode == meRECORD)
    {
        frameAddModeToWindows(WFMODE);  /* and update ALL mode lines */
        mlwrite(0,(meUByte *)"[End macro]");
        kbdmode = meSTOP;

        lkbdlen = kbdlen;
        return meTRUE;
    }
    return mlwrite(MWABORT,(meUByte *)"Macro not active");
}

/*
 * Execute a macro.
 * The command argument is the number of times to loop. Quit as soon as a
 * command gets an error. Return meTRUE if all ok, else meFALSE.
 */
int
executeKbdMacro(int f, int n)
{
    if(kbdmode != meSTOP)
        return mlwrite(MWABORT,(meUByte *)"Macro already active!");
    if(n <= 0)
        return meTRUE;
    kbdptr = lkbdptr;
    kbdlen = lkbdlen;
    kbdoff = 0;
    kbdrep = n;         /* remember how many times to execute */
    kbdmode = mePLAY;     /* start us in play mode */
    return meTRUE;
}

int
stringExec(int f, int n, meUByte *macro)
{
    meUByte *okbdptr;
    meUByte  oldcle;      /* old contents of clexec flag */
    meUByte *oldestr;	 /* original exec string */
    int  okbdlen, okbdoff, okbdmode, okbdrep;
    int  oldexec, ii;
    
    okbdptr = kbdptr;
    okbdoff = kbdoff;
    okbdlen = kbdlen;
    okbdrep = kbdrep;
    okbdmode = kbdmode;
    oldcle = clexec;			/* save old clexec flag */
    clexec = meFALSE;			/* in cline execution */
    oldestr = execstr;	/* save last ptr to string to execute */
    execstr = NULL;
    oldexec = execlevel;
    execlevel = 0;			/* Reset execution level */
    
    kbdptr = macro;
    kbdoff = 0;
    kbdlen = meStrlen(macro);
    kbdrep = n;
    kbdmode = mePLAY;     /* start us in play mode */
    ii = meTRUE;
    while((kbdrep > 1) || (kbdoff < kbdlen))
    {
        if(TTbreakFlag || (kbdmode != mePLAY))
        {
            ii = meABORT;
            break;
        }
        doOneKey();
    }
    kbdptr = okbdptr;
    kbdoff = okbdoff;
    kbdlen = okbdlen;
    kbdrep = okbdrep;
    kbdmode = okbdmode;
    execlevel = oldexec;
    clexec = oldcle;			/* restore clexec flag */
    execstr = oldestr;
    
    return ii;
}

int
executeString(int f, int n)
{
    meUByte sbuf[meBUF_SIZE_MAX];
    
    if(meGetString((meUByte *)"Enter string", MLFFZERO, 0, sbuf, meBUF_SIZE_MAX) <= 0)
        return 0;
    return stringExec(f,n,sbuf);
}

meMacro *
createMacro(meUByte *name)
{
    meUByte buff[meBUF_SIZE_MAX];
    register meMacro *mac;
    register meLine *hlp;
    register int idx;
    
    /* If the macro name has not been give then try and get one */
    if((name == NULL) && 
       (meGetString((meUByte *)"Macro name",MLCOMMAND|MLEXECNOUSER, 0, buff, meBUF_SIZE_MAX) > 0) && (buff[0] != '\0'))
        name = buff;
    
    if((name == NULL) || ((hlp = meLineMalloc(0,0)) == NULL))
        return NULL;
    
    /* if it already exists */
    if((idx = decode_fncname(name,1)) >= 0)
    {
        /* if function return error, else clear the buffer */ 
        if(idx < CK_MAX)
        {
            mlwrite(MWABORT|MWPAUSE,(meUByte *)"Error! can't re-define a base function");
            meFree(hlp);
            return NULL;
        }
        mac = getMacro(idx);
        if(!(mac->hlp->flag & meMACRO_EXEC))
        {
#if MEOPT_EXTENDED
            if(mac->hlp->flag & meMACRO_FILE)
            {
                if(meNullFree(mac->fname))
                    mac->fname = NULL;
            }
#endif
            meLineLoopFree(mac->hlp,1);
        }
    }
    else
    {
        meCommand **cmd;
        if((cmdTableSize & 0x1f) == 0)
        {
            /* run out of room in the command table, malloc more */
            meCommand **nt;
            if((nt = (meCommand **) meMalloc((cmdTableSize+0x20)*sizeof(meCommand *))) == NULL)
                return NULL;
            memcpy(nt,cmdTable,cmdTableSize*sizeof(meCommand *));
            if(cmdTable != __cmdTable)
                free(cmdTable);
            cmdTable = nt;
        }
        if(((mac = (meMacro *) meMalloc(sizeof(meMacro))) == NULL) ||
           ((mac->name = meStrdup(name)) == NULL))
        {
            meFree(hlp);
            return NULL;
        }
        mac->id = cmdTableSize;
#if MEOPT_EXTENDED
        mac->varList = NULL;
        mac->fname = NULL;
        mac->callback = -1;
#endif
        cmdTable[cmdTableSize++] = (meCommand *) mac;
        /* insert macro into the alphabetic list */
        cmd = &(cmdHead);
        while((*cmd != NULL) && (meStrcmp((*cmd)->name,name) < 0))
            cmd = &((*cmd)->anext);
        mac->anext = *cmd;
        *cmd = (meCommand *) mac;
#if MEOPT_CMDHASH
        /* insert macro into the hash table */
        {
            meUInt key;
            
            mac->hnext = NULL;
            meStringHash(name,key);
            mac->hash = key;
            cmd = &(cmdHash[key&(cmdHashSize-1)]);
            while(*cmd != NULL)
                cmd = &((*cmd)->hnext);
            *cmd = (meCommand *) mac;
        }
#endif
    }
    mac->hlp = hlp;
    hlp->next = hlp;
    hlp->prev = hlp;
    return mac;
}
/* macroDefine:	Set up a macro buffer and flag to store all
		executed command lines there			*/

int
macroDefine(int f, int n)
{
    register meMacro *mac;	/* pointer to macro */
    
    if((mac=createMacro(NULL)) == NULL)
        return meFALSE;
    if(n == 0)
        mac->hlp->flag |= meMACRO_HIDE;
    
    /* and set the macro store pointers to it */
    mcStore = 1;
    lpStore = mac->hlp;
    return meTRUE;
}

#if MEOPT_EXTENDED

/* macroFileDefine:
 * Set up a macro buffer and flag to store all
 * executed command lines there			*/
int
macroFileDefine(int f, int n)
{
    register meMacro *mac;	/* pointer to macro */
    meUByte fname[meBUF_SIZE_MAX];
    int ii=0;
    
    if(meGetString((meUByte *)"Enter file", MLCOMMAND, 0, fname, meBUF_SIZE_MAX) <= 0)
        return meFALSE;
    while((mac=createMacro(NULL)) != NULL)
    {
        if(n == 0)
            mac->hlp->flag |= meMACRO_HIDE;
        mac->hlp->flag |= meMACRO_FILE;
        mac->fname = meStrdup(fname);
        ii++;
    }
    if(ii == 0)
    {
        if((mac=createMacro(fname)) == NULL)
            return meFALSE;
        if(n == 0)
            mac->hlp->flag |= meMACRO_HIDE;
        mac->hlp->flag |= meMACRO_FILE;
    }
    return meTRUE;
}


/*
 * Asks the user if they wish to continue macro execution.
 * does nothing when recording, must meSTOP the macro execution so 
 * mlyesno goes to the keyboard.
 */
int
macroQuery(int f, int n)
{
    if(kbdmode == mePLAY)
    {
        int   rr;
        
        /* force a screen update */
        update(meTRUE);
        kbdmode = meSTOP;
        if((rr = mlyesno((meUByte *)"Continue macro")) == meABORT)
            return meABORT;
        if(rr == meFALSE)
        {
            kbdrep--;
            kbdoff = 0;
        }
        if(kbdrep)
            kbdmode = mePLAY;
        else
            return meFALSE;
    }
    else if(kbdmode != meRECORD)
        return mlwrite(MWABORT,(meUByte *)"Not defining macro");
    return meTRUE;
}
    
static meUByte helpFileName[] = "me.ehf";

static meBuffer *
helpBufferFind(void)
{
    meBuffer *hbp;
    if((hbp=bfind(BolhelpN,BFND_CREAT)) == NULL)
        return NULL;
    meModeClear(hbp->mode,MDUNDO);
    meModeClear(hbp->mode,MDCRYPT);
    meModeClear(hbp->mode,MDBINARY);
    meModeClear(hbp->mode,MDRBIN);
    meModeClear(hbp->mode,MDNACT);
    meModeSet(hbp->mode,MDHIDE);
    return hbp;
}

void 
helpBufferReset(meBuffer *bp)
{
    meModeClear(bp->mode,MDEDIT);
    meModeSet(bp->mode,MDVIEW);
    bp->dotLine = meLineGetNext(bp->baseLine);
    bp->dotOffset = 0;
    bp->dotLineNo = 0;
    bp->vertScroll = 0;
    bp->markLine = NULL;
    resetBufferWindows(bp);
}

static int
helpBufferLoad(meBuffer *hbp)
{
    if(!meModeTest(hbp->mode,MDLOCK))
    {
        meUByte fname[meBUF_SIZE_MAX];
    
        meModeSet(hbp->mode,MDLOCK);
        if(!fileLookup(helpFileName,0,NULL,meFL_USESRCHPATH|meFL_CALLBACK,fname))
            return mlwrite(MWABORT,(meUByte *)"[Help file \"%s\" is not on-line]",helpFileName);
        /* and read the stuff in */
        meModeClear(hbp->mode,MDVIEW);
        ffReadFile(&meior,fname,meRWFLAG_READ|meRWFLAG_SILENT,hbp,hbp->baseLine,0,0,0);
        helpBufferReset(hbp);
    }
    return meTRUE;
}

static int
findHelpItem(meUByte *item, int silent)
{
    meBuffer *bp, *hbp;
    meLine   *lp, *elp;
    int     sectLen, itemLen, ii;
    meUByte  *ss, cc, sect[5];
    
    itemLen = meStrlen(item);
    if((item[itemLen-1] == ')') &&
       ((item[(ii=itemLen-3)] == '(') ||
        (item[(ii=itemLen-4)] == '(') ))
    {
        sectLen = itemLen-ii-2;
        meStrcpy(sect,item+ii);
        itemLen = ii;
        item[itemLen] = '\0';
    }
    else
    {
        sectLen = 0;
        sect[0] = '\0';
        sect[1] = '\0';
    }
	
    if((hbp=helpBufferFind()) == NULL)
        return meABORT;
    elp = hbp->baseLine;
try_again:
    lp = meLineGetNext(elp);
    while(lp != elp)
    {
        if((lp->text[0] == '!') &&
           (!sectLen || 
            ((sect[1] == lp->text[2]) &&
             (((sectLen == 1) && (lp->text[3] == ' ')) || 
              (sect[2] == lp->text[3])))))
        {
            if((cc=lp->text[1]) == ' ')
            {
                ii = meLineGetLength(lp) - 4;
                if(ii != itemLen)
                    ii = -1;
            }
            else
            {
                ii = cc - '0';
                if(ii > itemLen)
                    ii = -1;
            }
            if((ii > 0) && !meStrncmp(item,lp->text+4,ii))
                break;
        }
        lp = meLineGetNext(lp);
    }
        
    if(lp == elp)
    {
        meMacro *mac;
        if(!meModeTest(hbp->mode,MDLOCK))
        {
            if(helpBufferLoad(hbp) == meABORT)
                return meABORT;
            goto try_again;
        }
        if((getMacroTypeS(item) == TKCMD) &&
           ((ii = decode_fncname(item,1)) >= CK_MAX) &&
           ((mac = getMacro(ii)) != NULL) &&
           (mac->hlp->flag & meMACRO_FILE))
        {
            meModeClear(hbp->mode,MDVIEW);
            if(mac->fname != NULL)
                execFile(mac->fname,0,1);
            else
                execFile(mac->name,0,1);
            helpBufferReset(hbp);
            if(!(mac->hlp->flag & meMACRO_FILE))
                goto try_again;
        }
        if(!silent)
            mlwrite(MWABORT,(meUByte *)"[Can't find help on %s%s]",item,sect);
        return meABORT;
    }
    if((bp=bfind(BhelpN,BFND_CREAT|BFND_CLEAR)) == NULL)
        return meFALSE;
    if((sectLen == 0) && (lp->text[2] != ' '))
    {
        ss = sect;
        *ss++ = '(';
        *ss++ = lp->text[2];
        if(lp->text[3] != ' ')
            *ss++ = lp->text[3];
        *ss++ = ')';
        *ss = '\0';
    }
    
    /* Add the header */
    {
        meUByte buff[meBUF_SIZE_MAX];
        sprintf((char *)buff,"\003D%s%s\003A",lp->text+4,sect);
        addLineToEob(bp,buff);
        addLineToEob(bp,(meUByte *)"\n\005sMicroEmacs\005m[Home]\005e \005sCommand G\005m[Commands]\005e \005sVariable \005m[Variables]\005e \005sMacro Lan\005m[Macro-Dev]\005e \005sGlobal G\005m[Glossary]\005e");
        memset(buff,boxChars[BCEW],78);
        buff[78] = '\n';
        buff[79] = '\0';
        addLineToEob(bp,buff);
    }
    while(((lp=meLineGetNext(lp)) != elp) && (lp->text[0] == '!'))
        ;
    while((lp != elp) && ((cc=lp->text[0]) != '!'))
    {
        if(cc == '|')
        {
            if(meStrcmp(item,lp->text+1))
                lp = meLineGetNext(lp);
        }
        else if(cc == '$')
        {
            if(lp->text[1] == 'a')
            {
                if(sect[1] == '5')
                {
                    meUByte line[meBUF_SIZE_MAX], *ss;
                    if((ss = getval(item)) != NULL)
                    {
                        addLineToEob(bp,(meUByte *)"\n\n\003BVALUE\003A\n");
                        meStrcpy(line,"    \"");
                        meStrncpy(line+5,ss,meBUF_SIZE_MAX-13);
                        line[meBUF_SIZE_MAX-2] = '\0';
                        meStrcat(line,"\"");
                        addLineToEob(bp,line);
                    }
                }
                if(((sect[1] == '2') || (sect[1] == '3')) && ((ii = decode_fncname(item,1)) >= 0))
                {
                    meBind *ktp;
                    meUByte line[meBUF_SIZE_MAX], *ss;
                    addLineToEob(bp,(meUByte *)"\n\n\003BBINDINGS\003A\n");
                    meStrcpy(line,"    ");
                    ss = line+4;
                    for(ktp = &keytab[0] ; ktp->code != ME_INVALID_KEY ; ktp++)
                    {
                        if(ktp->index == ii)
                        {
                            *ss++ = '"';
                            meGetStringFromKey(ktp->code,ss);
                            ss += meStrlen(ss);
                            *ss++ = '"';
                            *ss++ = ' ';
                        }
                    }
                    if(ss == line+4)
                        meStrcpy(ss,"none");
                    else
                        *ss = '\0';
                    addLineToEob(bp,line);
                }
            }
        }
        else
            addLineToEob(bp,lp->text);
        lp = meLineGetNext(lp);
    }
    /* Add the footer */
    {
        meUByte buff[meBUF_SIZE_MAX];
        buff[0] = '\n';
        memset(buff+1,boxChars[BCEW],78);
        sprintf((char *)buff+79,"\n\005sCopyright\005m%s\005e",meCopyright);
        addLineToEob(bp,buff);
    }
    bp->dotLine = meLineGetNext(bp->baseLine);
    bp->dotOffset = 0;
    bp->dotLineNo = 0;
    meModeClear(bp->mode,MDEDIT);    /* don't flag this as a change */
    meModeSet(bp->mode,MDVIEW);      /* put this buffer view mode */
    resetBufferWindows(bp);            /* Update the window */
    meWindowPopup(bp,NULL,WPOP_USESTR,NULL);
    return meTRUE;
}

int
help(int f, int n)
{
    if(n == 0)
    {
        meBuffer *hbp;
        if(((hbp=helpBufferFind()) == NULL) ||
           (helpBufferLoad(hbp) <= 0))
            return meABORT;
        return swbuffer(frameCur->windowCur,hbp);
    }
    return findHelpItem((meUByte *)"MicroEmacs",0);
}

int	
helpItem(int f, int n)
{
    meUByte buf[meBUF_SIZE_MAX];
    
    if(meGetString((meUByte *)"Help on item", 0, 0, buf, meBUF_SIZE_MAX-10) <= 0)
        return meFALSE;
    return findHelpItem(buf,0);
}

int	
helpCommand(int f, int n)
{
    meUByte *ss, buf[meBUF_SIZE_MAX];
              
    if(meGetString((meUByte *)"Help on command", MLCOMMAND, 0, buf, meBUF_SIZE_MAX-10) <= 0)
        return meFALSE;
    ss = buf + meStrlen(buf);
    meStrcpy(ss,"(2)");
    if(findHelpItem(buf,1) > 0)
        return meTRUE;
    meStrcpy(ss,"(3)");
    return findHelpItem(buf,0);
}

int	
helpVariable(int f, int n)
{
    meUByte buf[meBUF_SIZE_MAX];

    if(meGetString((meUByte *)"Help on variable", MLVARBL, 0, buf, meBUF_SIZE_MAX-10) <= 0)
        return meFALSE;
    meStrcat(buf,"(5)");
    return findHelpItem(buf,0);
}

/* macroHelpDefine:
 * Set up a macro help definition
 */
int
macroHelpDefine(int f, int n)
{
    meUByte name[meBUF_SIZE_MAX];
    meUByte sect[20];
    
    if((lpStoreBp=helpBufferFind()) == NULL)
        return meABORT;
    if(meGetString((meUByte *)"Enter name", MLCOMMAND, 0, name+4, meBUF_SIZE_MAX-4) <= 0)
        return meFALSE;
    sect[0] = '\0';
    if(meGetString((meUByte *)"Enter section", 0, 0, sect, 20) == meABORT)
        return meFALSE;
    /* and set the macro store pointers to it */
    lpStore = meLineGetNext(lpStoreBp->baseLine);
    name[0] = '!';
    name[1] = (f) ? '0'+n:' ';
    if(sect[0] == '\0')
    {
        name[2] = ' '; 
        name[3] = ' '; 
    }
    else
    {
        name[2] = sect[0]; 
        name[3] = (sect[1] == '\0') ? ' ':sect[1];
    }
    addLine(lpStore,name);
    lpStoreBp->lineCount++;
    mcStore = 2;
    return meTRUE;
}

int
nameKbdMacro(int f, int n)
{
    meUByte buf[meBUF_SIZE_MAX];
    int ss;
    
    if(kbdmode != meSTOP)
        return mlwrite(MWABORT,(meUByte *)"Macro already active!");
    if(lkbdlen <= 0)
        return mlwrite(MWABORT,(meUByte *)"No macro defined!");
    if((ss=macroDefine(meFALSE, meTRUE)) > 0)
    {
        meStrcpy(buf,"execute-string \"");
        n = expandexp(lkbdlen,lkbdptr,meBUF_SIZE_MAX-2,16,buf,-1,NULL,meEXPAND_BACKSLASH|meEXPAND_FFZERO|meEXPAND_PRINTABLE);
        meStrcpy(buf+n,"\"");
        addLine(lpStore,buf);
    }
    mcStore = 0;
    lpStore = NULL;
    return ss;
}

meMacro *
userGetMacro(meUByte *buf, int len)
{
    register int idx;
    
    if(meGetString((meUByte *)"Enter macro name ", MLCOMMAND,2,buf,len) > 0)
    {        
        if((idx = decode_fncname(buf,0)) < 0)
            mlwrite(MWABORT,(meUByte *)"%s not defined",buf);
        else if(idx < CK_MAX)
            mlwrite(MWABORT,(meUByte *)"%s is a command",buf);
        else
            return getMacro(idx);
    }
    return NULL;
}

int
insMacro(int f, int n)
{
    register meLine *lp, *slp;
    meWindow *wp, *cwp;
    meMacro *mac;
    meUByte buf[meBUF_SIZE_MAX];
    meInt nol, lineNo, len;
    int ii;
    
    meStrcpy(buf,"define-macro ");
    if((mac=userGetMacro(buf+13, meBUF_SIZE_MAX-13)) == NULL)
        return meFALSE;
    
    if((ii=bufferSetEdit()) <= 0)               /* Check we can change the buffer */
        return ii;
    cwp = frameCur->windowCur;
    cwp->dotOffset = 0;
    slp = cwp->dotLine;
    lineNo = cwp->dotLineNo;
    nol = addLine(slp,buf);
    len = meStrlen(buf) + 9;
    lp = meLineGetNext(mac->hlp);            /* First line.          */
    while (lp != mac->hlp)
    {
        nol += addLine(slp,lp->text);
        len += meLineGetLength(lp) + 1;
        lp = meLineGetNext(lp);
    }
    nol += addLine(slp,(meUByte *)"!emacro");
    cwp->buffer->lineCount += nol;
    meFrameLoopBegin();
    for(wp=loopFrame->windowList; wp!=NULL; wp=wp->next)
    {
        if(wp->buffer == cwp->buffer)
        {
            if(wp->dotLineNo >= lineNo)
                wp->dotLineNo += nol;
            if(wp->markLineNo >= lineNo)
                wp->markLineNo += nol;
            wp->updateFlags |= WFMAIN|WFMOVEL;
        }
    }
    meFrameLoopEnd();
#if MEOPT_UNDO
    meUndoAddInsChars(len);
#endif
    return meTRUE;
}
#endif
  

