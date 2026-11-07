/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * input.c - Various input routines.
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
 * Created:     5/9/86
 * Synopsis:    Various input routines.
 * Authors:     Daniel Lawrence, Jon Green & Steven Phillips
 */

#define	__INPUTC			/* Define the name of the file */

#include "emain.h"
#include "efunc.h"
#include "evar.h"
#include "eskeys.h"

meUByte **mlgsStrList;
int    mlgsStrListSize;

/*
 * Ask a yes or no question in the message line. Return either meTRUE, meFALSE, or
 * meABORT. The meABORT status is returned if the user bumps out of the question
 * with a ^G. Used any time a confirmation is required.
 */
int
mlCharReply(meUByte *prompt, int mask, meUByte *validList, meUByte *helpStr)
{
    int   inpType=0, cc;
    meUByte buff[meTOKENBUF_SIZE_MAX];
    meUByte *pp=prompt, *lp=NULL;
    
    for(;;)
    {
        if((clexec == meFALSE) || inpType)
        {
            if((kbdmode != mePLAY) || (kbdoff >= kbdlen))
            {
                /* We are going to get a key of the user!! */
                if(mask & mlCR_QUIT_ON_USER)
                    return -2;
                if(mask & mlCR_UPDATE_ON_USER)
                    update(meTRUE);
                
                if(frameCur->mlStatus & MLSTATUS_POSOSD)
                {
                    frameCur->mlStatus = (frameCur->mlStatus & MLSTATUS_CURMASK)|MLSTATUS_POSOSD;
                    mlResetCursor();
                }
                else
                {
                    /* switch off the status cause we are replacing it */
                    frameCur->mlStatus &= MLSTATUS_CURMASK;
                    mlwrite(((mask & mlCR_CURSOR_IN_MAIN) ? 0:MWCURSOR)|MWSPEC,pp);
                    lp = pp;
                    pp = prompt;
                    /* switch on the status so we save it */
                    frameCur->mlStatus = (frameCur->mlStatus & MLSTATUS_CURMASK)|((mask & mlCR_CURSOR_IN_MAIN) ? MLSTATUS_KEEP:(MLSTATUS_KEEP|MLSTATUS_POSML));
                    inpType = 2;
                }
            }
            cc = meGetKeyFromUser(meFALSE,0,meGETKEY_SILENT|meGETKEY_SINGLE);
            frameCur->mlStatus &= ~(MLSTATUS_KEEP|MLSTATUS_RESTORE|MLSTATUS_POSML);
            if((cc == breakc) && !(mask & mlCR_QUOTE_CHAR))
                return -1;
        }
        else
        {
            meUByte *ss = execstr;
            execstr = macroNextToken(execstr,buff);
            if((buff[0] == '@') && (buff[1] == 'm') && (buff[2] == 'x'))
            {
                cc = buff[3];
                execstr = macroNextToken(execstr,buff);
                if(cc == 'a')
                    execstr = ss;
                meStrcpy(resultStr,prompt);
                if(lineExec (0, 1, buff) <= 0)
                    return -1;
                cc = resultStr[0];
            }
            else if((buff[0] == '@') && (buff[1] == 'm') && (buff[2] == 'n'))
            {
                /* if @mna (get all input from user) then rewind the execstr */
                if(buff[3] == 'a')
                    execstr = ss;
                inpType = 1;
                continue;
            }
            else
            {
                /* evaluate it */
                meUByte *res = getval(buff);
                if(res == abortm)
                {
                    if((buff[0] != '\0') && (buff[0] != ';'))
                        return -1;
                    inpType = 1;
                    continue;
                }
#if MEOPT_UNICODE
                {
                    meUInt uc;
                    meUtf8Decode(res,meUTF8_LEN_MAX,&uc);
                    cc = (int) uc;
                }
#else
                cc = *res;
#endif
            }
        }
        
        if((mask & mlCR_QUOTE_CHAR) &&
           ((cc = quoteKeyToChar((meKey) cc)) < 0))
        {
            if(validList == NULL)
                return -1;
        }
        else
        {
            if((mask & mlCR_LOWER_CASE) && meKeyIsChar(cc))
                cc = meUniCharToLower(cc);
#if MEOPT_EXTENDED
            /* a non-Latin keyboard's key which is not a valid answer gives its Latin key's char */
            if((validList != NULL) && (charKeyboardMap != NULL) && meKeyIsChar(cc) &&
#if MEOPT_UNICODE
               ((cc >= 0x80) || (meStrchr(validList,cc) == NULL)))
#else
               (meStrchr(validList,cc) == NULL))
#endif
            {
                int mc;
                if((mc = meKeyboardMapChar(cc)) != 0)
                    cc = mc;
            }
#endif
            if((cc == '?') && (helpStr != NULL))
                pp = (lp == helpStr) ? prompt:helpStr;
            else if(((cc != '\0') || (mask & mlCR_QUOTE_CHAR)) &&
#if MEOPT_UNICODE
                    /* the valid list is ASCII */
                    (!(mask & mlCR_ALPHANUM_CHAR) || ((cc < 0x80) && isAlphaNum(cc))) &&
                    ((validList == NULL) || ((cc < 0x80) && (meStrchr(validList,cc) != NULL))))
#else
                    (!(mask & mlCR_ALPHANUM_CHAR) || isAlphaNum(cc)) &&
                    ((validList == NULL) || (((cc & 0xff00) == 0) && (meStrchr(validList,cc) != NULL))))
#endif
            {
                if (inpType == 2)
                {
                    meGetStringFromKey((meKey) cc,buff);
                    mlwrite(MWCURSOR,(meUByte *)"%s%s",prompt,buff);
                }
                return cc;
            }
        }
        inpType = 1;
    }
}

int
mlyesno(meUByte *prompt)
{
    meUByte buf[meBUF_SIZE_MAX] ;    /* prompt to user */
    int ret;
    
    /* build and prompt the user */
    meStrcpy(buf,prompt);
    meStrcat(buf," (?/y/n) ? ");

    ret = mlCharReply(buf,mlCR_LOWER_CASE,(meUByte *)"yn",(meUByte *)"(Y)es, (N)o, (C-g)Abort ? ");
    
    if(ret < 0)
        return ctrlg(meFALSE,1);
    else if(ret == 'n')
        return meFALSE;
    return meTRUE;
}

/* Writes key cc into a key string (keyboard macros, execute-string), returns its length, at
 * most meKEYSTR_LEN_MAX. A char key is the char itself, a key with modifiers, a prefix or a
 * special key is the leader, the special trail, the modifier & prefix byte and the char or
 * special key code. Nul and the leader char are escaped */
int
meKeyToStr(meUByte *dd, meKey cc)
{
    meUInt ch=cc & ME_KEY_CHAR;
    int ll=0;
    
    if(!meKeyIsChar(cc))
    {
        dd[ll++] = meCHAR_LEADER;
        dd[ll++] = meCHAR_TRAIL_SPECIAL;
        dd[ll++] = (meUByte) (cc >> ME_KEY_HIGH_SHIFT);
    }
#if MEOPT_UNICODE
    if((ch >= 0x80) && !(cc & ME_SPECIAL))
        return ll + meUtf8Encode(dd+ll,ch);
#endif
    if(ch == meCHAR_LEADER)
    {
        dd[ll++] = meCHAR_LEADER;
        dd[ll++] = meCHAR_TRAIL_LEADER;
    }
    else if(ch == 0)
    {
        dd[ll++] = meCHAR_LEADER;
        dd[ll++] = meCHAR_TRAIL_NULL;
    }
    else
        dd[ll++] = (meUByte) ch;
    return ll;
}

/* Returns the length of the key following a leader & special trail in a key string */
int
meKeyStrSpecialLen(meUByte *ss)
{
#if MEOPT_UNICODE
    meUInt uc;
    if((ss[1] >= 0x80) && !(ss[0] & (ME_SPECIAL >> ME_KEY_HIGH_SHIFT)))
        return 1 + meUtf8Decode(ss+1,meUTF8_LEN_MAX,&uc);
#endif
    return (ss[1] == meCHAR_LEADER) ? 3:2;
}

/*    tgetc:    Get a key from the terminal driver, resolve any keyboard
 *              macro action
 */
static meKey
tgetc(void)
{
    meKey cc ;    /* fetched character */

    /* if we are playing a keyboard macro back, */
    if (kbdmode == mePLAY)
    {
kbd_rep:
        /* if there is some left... */
        if(kbdoff < kbdlen)
        {
            cc = (meKey) kbdptr[kbdoff++];
            if(cc == meCHAR_LEADER)
            {
                cc = kbdptr[kbdoff++];
                if(cc == meCHAR_TRAIL_SPECIAL)
                {
                    meUByte dd ;    /* fetched character */
                    cc = ((meKey) kbdptr[kbdoff++]) << ME_KEY_HIGH_SHIFT;
#if MEOPT_UNICODE
                    if((kbdptr[kbdoff] >= 0x80) && !(cc & ME_SPECIAL))
                    {
                        meUInt uc;
                        kbdoff += meUtf8Decode(kbdptr+kbdoff,kbdlen-kbdoff,&uc);
                        cc |= uc;
                    }
                    else
#endif
                    if(((dd = kbdptr[kbdoff++]) != meCHAR_LEADER) ||
                       ((dd = kbdptr[kbdoff++]) != meCHAR_TRAIL_NULL))
                        cc |= dd;
                }
                else if(cc == meCHAR_TRAIL_NULL)
                    cc = 0;
                else if(cc != meCHAR_TRAIL_LEADER)
                    /* special '\?' key (e.g. OSD hot key) - ignore, a leader pair is the leader char */
                    goto kbd_rep;
            }
#if MEOPT_UNICODE
            else if(cc >= 0x80)
            {
                meUInt uc;
                kbdoff += meUtf8Decode(kbdptr+kbdoff-1,kbdlen-kbdoff+1,&uc) - 1;
                cc = uc;
            }
#endif
            return cc;
        }
        /* at the end of last repitition? */
        if (--kbdrep > 0)
        {
            /* reset the macro to the begining for the next rep */
            kbdoff = 0;
            goto kbd_rep;
        }
        kbdmode = meSTOP;
#if MEOPT_UNDO
        undoContFlag++;
#endif
        /* force a screen update after all is done */
        update(meFALSE);
    }

    if(kbdmode == meRECORD)
    {
#if MEOPT_MOUSE
        /* get and save a key */
        /* ignore mouse keys while recording a macro - get another */
        do {
            /* fetch a character from the terminal driver */
            cc = TTgetc();
        } while(((cc & (ME_SPECIAL|ME_KEY_CHAR)) >= (ME_SPECIAL|SKEY_mouse_drop_1))  &&
                ((cc & (ME_SPECIAL|ME_KEY_CHAR)) <= (ME_SPECIAL|SKEY_mouse_time_3)));
#else
        cc = TTgetc();
#endif
        /* if we haven't got room for the key stop so we don't overrun the buffer */
        if(kbdlen > meBUF_SIZE_MAX - meKEYSTR_LEN_MAX)
        {
            kbdmode = meSTOP;
            TTbell();
        }
        else
            kbdlen += meKeyToStr(kbdptr+kbdlen,cc);
    }
    else
        cc = TTgetc();

    /* and finally give the char back */
    return cc;
}


static int
getprefixchar(int f, int n, int ctlc, int flag)
{
    meUByte buf[20];
    int c;

    if(!(flag & meGETKEY_SILENT))
    {
        buf[meGetStringFromChar((meKey) ctlc,buf)] = '\0';
        if(f == meTRUE)
            mlwrite(MWCURSOR,(meUByte *)"Arg %d: %s", n, buf);
        else
            mlwrite(MWCURSOR,(meUByte *)"%s", buf);
    }
    c = tgetc();
    if(!(c & (ME_SHIFT|ME_CONTROL|ME_ALT|ME_SPECIAL)))
        c = toLower(c);
    return c;
}

/* meGetKeyFromUser
 *   Get a command from the keyboard. Process all applicable prefix keys
 */
meKey
meGetKeyFromUser(int f, int n, int flag)
{
    meKey cc ;        /* fetched keystroke */
    int ii;
    
    /* Get a key or mouse event, if meGetKeyFirst is not < 0 then we already
     * have one to process (i.e. tab out of an OSD entry, pre-loaded a TAB
     * to auto complete on @ml etc). Next check if we are playing a keyboard
     * macro, otherwise wait for an event from the user */
    if(meGetKeyFirst >= 0)
    {
        cc = (meKey) meGetKeyFirst;
        meGetKeyFirst = -1;
        return cc;
    }
    if(kbdmode == mePLAY)
    {
        if(TTbreakTest(0))
        {
            ctrlg(meFALSE,1);
#if MEOPT_UNDO
            undoContFlag++;
#endif
            /* force a screen update */
            update(0);
        }
        else
            flag |= meGETKEY_SILENT;
    }
    if(f && !(flag & meGETKEY_SILENT))
        mlwrite(MWCURSOR,(meUByte *)"Arg %d:",n);

    /* get initial character */
    cc = tgetc();
    
    /* If we are in idle mode then disable now */
    if (kbdmode == meIDLE)            /* In the idle state */
        kbdmode = meSTOP;                 /* Restore old state */

    if(!(flag & meGETKEY_SINGLE))
    {
        /* process prefixes */
        ii = ME_PREFIX_NUM+1;
        while(--ii > 0)
            if(cc == prefixc[ii])
            {
                cc = ((ii << ME_PREFIX_BIT) | getprefixchar(f,n,cc,flag));
                break;
            }
        if(flag & meGETKEY_COMMAND)
            thiskey = cc;
    }
    /* return the key */
    return cc;
}

#define mlINPUT_LAST_KILL   0x01
#define mlINPUT_LAST_YANK   0x02
#define mlINPUT_LAST_REYANK 0x04
#define mlINPUT_THIS_KILL   0x10
#define mlINPUT_THIS_YANK   0x20
#define mlINPUT_THIS_REYANK 0x40
static int mlInputFlags;

static meIFuncSSI curCmpIFunc;
static meIFuncSS  curCmpFunc;

#if MEOPT_UNICODE
/* the input buffer holds internal UTF-8, positions step whole chars */
#define mlNextChar(buf,pos,len) meUtf8Next((buf),(pos),(len))
#define mlPrevChar(buf,pos)     meUtf8Prev((buf),(pos))

static int
mlIsAlphaNum(meUByte *buf, int pos)
{
    meUInt uc=buf[pos];
    if(uc < 0x80)
        return isAlphaNum(uc);
    meUtf8Decode(buf+pos,meUTF8_LEN_MAX,&uc);
    return meUniPropIsWord(meUniProps(uc));
}

#define mlForwardToNextSpace(buff,curPos,curLen) \
for(; (curPos < curLen) && mlIsAlphaNum(buff,curPos) ; curPos=meUtf8Next(buff,curPos,curLen))
#define mlBackwardToNextSpace(buff,curPos) \
for(; (curPos > 0) && mlIsAlphaNum(buff,meUtf8Prev(buff,curPos)) ; curPos=meUtf8Prev(buff,curPos))

#define mlForwardToNextWord(buff,curPos,curLen) \
for(; (curPos < curLen) && !mlIsAlphaNum(buff,curPos) ; curPos=meUtf8Next(buff,curPos,curLen))
#define mlBackwardToNextWord(buff,curPos) \
for(; (curPos > 0) && !mlIsAlphaNum(buff,meUtf8Prev(buff,curPos)) ; curPos=meUtf8Prev(buff,curPos))
#else
#define mlNextChar(buf,pos,len) ((pos)+1)
#define mlPrevChar(buf,pos)     ((pos)-1)
#define mlIsAlphaNum(buf,pos)   isAlphaNum((buf)[pos])

#define mlForwardToNextSpace(buff,curPos,curLen) \
for(; (curPos < curLen) && isAlphaNum(buff[curPos]) ; curPos++)
#define mlBackwardToNextSpace(buff,curPos) \
for(; (curPos > 0) && isAlphaNum(buff[curPos-1]) ; curPos--)

#define mlForwardToNextWord(buff,curPos,curLen) \
for(; (curPos < curLen) && !isAlphaNum(buff[curPos]) ; curPos++)
#define mlBackwardToNextWord(buff,curPos) \
for(; (curPos > 0) && !isAlphaNum(buff[curPos-1]) ; curPos--)
#endif

static int
mlInsertChar(meUByte cc, meUByte *buf, int *pos, int *len, int max)
{
    /*
     * Insert a character "c" into the buffer pointed to by "buf" at
     * position pos (which we update), if the length len (which we
     * also update) is less than the maximum length "max".
     *
     * Return:  meTRUE    if the character will fit
     *          meFALSE   otherwise
     */
    int ll;
    ll = (int) meStrlen(buf);
    if(ll + 1 >= max)
        return meFALSE;
    
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
        meUByte *ss, c1, c2;
        
        ss = buf + *pos;
        c2 = cc;
        
        do {
            c1 = c2;
            c2 = *ss;
            *ss++ = c1;
        } while(c1 != '\0');
        
        /* update the pos & length */
        *len = ll+1;
        (*pos)++;
    }
    return meTRUE;
}

/* Inserts the char of key cc */
static int
mlInsertKeyChar(int cc, meUByte *buf, int *pos, int *len, int max)
{
#if MEOPT_UNICODE
    /* a NUL is inserted in its internal form, a 0 byte would end the string */
    if((cc >= 0x80) || (cc == 0))
    {
        meUByte bb[meUTF8_LEN_MAX];
        int ii, ll=meUtf8Encode(bb,(meUInt) cc);
        if(((int) meStrlen(buf) + ll) >= max)
            return meFALSE;
        for(ii=0 ; ii<ll ; ii++)
            mlInsertChar(bb[ii],buf,pos,len,max);
        return meTRUE;
    }
#endif
    return mlInsertChar((meUByte) cc,buf,pos,len,max);
}

/* Changes the case of the char at pos, to upper if up, returns the position after it */
static int
mlCharCase(meUByte *buf, int pos, int *len, int max, int up)
{
#if MEOPT_UNICODE
    meUByte bb[meUTF8_LEN_MAX];
    meUInt uc, nc;
    int ol, nl;
    
    ol = meUtf8Decode(buf+pos,*len-pos,&uc);
    nc = (up) ? meUniCharToUpper(uc):meUniCharToLower(uc);
    if(nc == uc)
        return pos+ol;
    nl = meUtf8Encode(bb,nc);
    if(nl != ol)
    {
        /* the char changes length */
        if((*len + nl - ol) >= max)
            return pos+ol;
        memmove(buf+pos+nl,buf+pos+ol,*len-pos-ol+1);
        *len += nl - ol;
    }
    memcpy(buf+pos,bb,nl);
    return pos+nl;
#else
    buf[pos] = (up) ? toUpper(buf[pos]):toLower(buf[pos]);
    return pos+1;
#endif
}

static int
mlForwardDelete(meUByte *buf,int curPos, int curLen, int delLen, int flags)
{
    meUByte *ss, *dd;
    if((curPos+delLen) > curLen)
        delLen = curLen - curPos;
    if(delLen > 0)
    {
        dd = buf + curPos;
        if(flags & 2)
        {
            killInit(mlInputFlags & mlINPUT_LAST_KILL);
            if((ss = killAddNode(delLen)) != NULL)
            {
                memcpy(ss,dd,delLen);
                ss[delLen] = '\0';
                mlInputFlags |= mlINPUT_THIS_KILL;
            }
        }
        if(flags & 1)
        {
            curLen -= delLen;
            ss = dd + delLen;
            while((*dd++ = *ss++) != '\0')
                ;
        }
    }
    return curLen;
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

/* prompt         prompt to be displayed      */
/* buf            buffer to expand controls from */
/* cont           continuing str to be displayed */
/* cpos           offset into "buf"          */
/* expChr         0=dont expand unprintables, use ?, 1=expand ?'s */
void
mlDisp(meUByte *prompt, meUByte *buf, meUByte *cont, int cpos)
{
    char   expbuf[meMLDISP_SIZE_MAX];
    int    len, start, col, maxCol;
    int    promsiz;
    
    col = -1;
    len = expandexp(-1,buf,meMLDISP_SIZE_MAX-1,0,(meUByte *)expbuf,cpos,&col,0);
    if(col < 0)
        col = len;
    if(cont != NULL)
    {
        meStrncpy(expbuf+len,cont,meMLDISP_SIZE_MAX-len);
        expbuf[meMLDISP_SIZE_MAX-1] = '\0';
        len += (int) strlen(expbuf+len);
    }
    /* switch off the status cause we are replacing it */
    frameCur->mlStatus &= MLSTATUS_CURMASK;
    maxCol = frameCur->width;
#if MEOPT_UNICODE
    {
        /* the same layout in cells, wide chars cut by the start or end become spaces */
        meUByte full[meMLDISP_SIZE_MAX+meBUF_SIZE_MAX], out[meMLDISP_SIZE_MAX+meBUF_SIZE_MAX], *ss, *dd;
        int flen, ll, ww, cc, lim;
        
        promsiz = (int) meStrlen(prompt);
        if(promsiz > meBUF_SIZE_MAX-1)
            promsiz = meBUF_SIZE_MAX-1;
        memcpy(full,prompt,promsiz);
        memcpy(full+promsiz,expbuf,len+1);
        flen = promsiz + len;
        cc = meStrCells(prompt,promsiz);
        col = cc + meStrCells((meUByte *) expbuf,col);
        len = cc + meStrCells((meUByte *) expbuf,len);
        if(col > maxCol - 1)
        {
            int div = maxCol >> 1;
            
            start = (((col - (div >> 1)) / div) * div);
            len -= start;
            col -= start-1;
            maxCol--;
        }
        else
            start = 0;
        dd = out;
        if(start)
            *dd++ = '$';
        lim = (len > maxCol) ? maxCol-1:maxCol;
        ss = full;
        for(cc=0 ; flen > 0 ; ss+=ll, flen-=ll, cc+=ww)
        {
            ww = meStrCharCells(ss,flen,&ll);
            if((cc+ww) <= start)
                continue;
            if((cc+ww-start) > lim)
                break;
            if(cc < start)
                *dd++ = ' ';
            else
            {
                memcpy(dd,ss,ll);
                dd += ll;
            }
        }
        if(len > maxCol)
        {
            for(cc=meStrCells(out,(int) (dd-out))-((start) ? 1:0) ; cc<lim ; cc++)
                *dd++ = ' ';
            *dd++ = '$';
        }
        *dd = '\0';
        frameCur->mlColumn = col;
        mlwrite(MWUSEMLCOL|MWCURSOR|MWSPEC,out);
    }
#else
    promsiz = (int) meStrlen(prompt);
    col += promsiz;
    len += promsiz;
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
        int div = maxCol >> 1;
        
        start = (((col - (div >> 1)) / div) * div);
        len -= start;
        col -= start-1;
        maxCol--;
    }
    else
        start = 0;
    if(len > maxCol)
    {
        expbuf[start+maxCol-promsiz-1] = '$';
        expbuf[start+maxCol-promsiz] = '\0';
    }
    frameCur->mlColumn = col;
    if(start >= promsiz)
        mlwrite(MWUSEMLCOL|MWCURSOR,(meUByte *)"%s%s",(start) ? "$":"",expbuf+start-promsiz);
    else
        mlwrite(MWUSEMLCOL|MWCURSOR,(meUByte *)"%s%s%s",(start) ? "$":"",prompt+start,expbuf);
#endif
    
    /* switch on the status so we save it */
    frameCur->mlStatus = (frameCur->mlStatus & MLSTATUS_CURMASK)|MLSTATUS_KEEP|MLSTATUS_POSML;
}


#if MEOPT_EXTENDED
int
isFileIgnored(meUByte *fileName)
{
    meUByte *fi, cc;
    int len, fil;
    
    if((fi = fileIgnore) != NULL)
    {
        len = (int) meStrlen(fileName);
        for(;;)
        {
            fil=1;
            while(((cc=fi[fil]) != ' ') && (cc != '\0'))
                fil++;
            if((fil <= len) &&
#ifdef _FILE_CASE_INSENSE
               !meStrnicmp(fileName+len-fil,fi,fil)
#else
               !meStrncmp(fileName+len-fil,fi,fil)
#endif
               )
                return meTRUE;
            fi += fil;
            if(*fi++ == '\0')
                break;
        }
    }
    return meFALSE;
}
#endif

#if MEOPT_UNICODE
/* the display width of a completion */
#define mlCompCells(ss) meStrCells((ss),(int) meStrlen(ss))
#else
#define mlCompCells(ss) ((int) meStrlen(ss))
#endif

#if MEOPT_UNICODE
#define mlFoldChar(cc) (((cc) < 0x80) ? (meUInt) toLower(cc):meUniFold(cc))

/* Compares the nn bytes of str1 with the start of str2 a char at a time ignoring case, as
 * meStridif orders the completion list; the matching start of str2 can be a different length */
static int
mlStrnifold(const meUByte *str1, const meUByte *str2, size_t nn)
{
    const meUByte *ee=str1+nn;
    meUInt cc, dd, fc, fd;

    while(str1 < ee)
    {
        str1 += meUtf8Decode(str1,(int) (ee-str1),&cc);
        str2 += meUtf8Decode(str2,meUTF8_LEN_MAX,&dd);
        if(cc != dd)
        {
            fc = mlFoldChar(cc);
            fd = mlFoldChar(dd);
            if(fc != fd)
                return (fc < fd) ? -1:1;
        }
        if((dd == 0) && (str2[-1] == 0))
            return 1;
    }
    return 0;
}

/* Returns the byte offset of the char after the first nc chars of ss */
static int
mlCharsToBytes(const meUByte *ss, int nc)
{
    meUInt uc;
    int off=0;

    while((--nc >= 0) && (ss[off] != '\0'))
        off += meUtf8Decode(ss+off,meUTF8_LEN_MAX,&uc);
    return off;
}

/* Returns the number of chars in the len bytes of ss */
static int
mlBytesToChars(const meUByte *ss, int len)
{
    meUInt uc;
    int off=0, nc=0;

    while(off < len)
    {
        off += meUtf8Decode(ss+off,len-off,&uc);
        nc++;
    }
    return nc;
}

/* Tests whether any of the completions strs[fst..lst] has the ASCII char cc after the input's
 * nc chars */
static int
mlCompleteHasChar(meUByte **strs, int fst, int lst, int nc, int cc)
{
    for( ; fst<=lst ; fst++)
        if(strs[fst][mlCharsToBytes(strs[fst],nc)] == cc)
            return 1;
    return 0;
}

/* Extends the input buf (len bytes, nc chars of it the completion's) with the chars every
 * completion strs[fst..lst] has next, taken from strs[lst]. Ignoring case the completions can
 * hold the same chars in different lengths, so each keeps its own offset. Returns the new length */
static int
mlCompleteCommon(meUByte *buf, int len, int nbuf, meUByte **strs, int fst, int lst, int nc,
                 int insens)
{
    meUByte *ss;
    meUInt ch, dd;
    int *offs, ii, rl, nn=lst-fst+1;

    if((offs = (int *) meMalloc(nn*sizeof(int))) == NULL)
        return len;
    for(ii=0 ; ii<nn ; ii++)
        offs[ii] = mlCharsToBytes(strs[fst+ii],nc);
    for(;;)
    {
        ss = strs[lst]+offs[nn-1];
        if(*ss == '\0')
            break;
        rl = meUtf8Decode(ss,meUTF8_LEN_MAX,&ch);
        if((len+rl) >= nbuf)
            break;
        for(ii=0 ; ii<nn-1 ; ii++)
        {
            if(strs[fst+ii][offs[ii]] == '\0')
                break;
            offs[ii] += meUtf8Decode(strs[fst+ii]+offs[ii],meUTF8_LEN_MAX,&dd);
            if((dd != ch) && (!insens || (mlFoldChar(dd) != mlFoldChar(ch))))
                break;
        }
        if(ii != nn-1)
            break;
        memcpy(buf+len,ss,rl);
        len += rl;
        offs[nn-1] += rl;
    }
    meFree(offs);
    return len;
}
#endif

static int
getFirstLastPos(int noStr,meUByte **strs, meUByte *str, int option,
                int *fstPos, int *lstPos)
{
    register int nn;
    
    if((nn = (int) meStrlen(str)) == 0)
    {
        *fstPos = 0;
        *lstPos = noStr - 1;
        if(noStr <= 0)
            return 0;
    }
    else
    {
        register int ii, rr;
        
        for(ii=0 ; ii<noStr ; ii++)
            if(!(rr = curCmpIFunc(str,strs[ii],nn)))
                break;
            else if(rr < 0)
                return 0;
        
        if(ii == noStr)
            return 0;
        
        *fstPos = ii;
        for(++ii ; ii<noStr ; ii++)
            if(curCmpIFunc(str,strs[ii],nn))
                break;
        *lstPos = ii - 1;
    }
#if MEOPT_EXTENDED
    if((option & MLFILE) && (fileIgnore != NULL))
    {
        /* try to eat away at the number of possible completions
         * until there is only one. If successful the return as
         * just one. Can remove fileIngore type files which are 
         * usually . .. and backup files (*~) etc.
         */
        register int ff, ll, ii=1;
        register meUByte *ss;
        ff = *fstPos;
        ll = *lstPos;
        
        while((ff <= ll) && (ii >= 0))
        {
            if(ii)
                ss = strs[ll];
            else
                ss = strs[ff];
            if(!isFileIgnored(ss))
                ii--;
            else if(ii)
                ll--;
            else
                ff++;
        }
        if(ff == ll)
            *fstPos = *lstPos = ff;
    }
#endif
    return 1;
}

int
createBuffList(meUByte ***listPtr, int noHidden)
{
    register meBuffer *bp = bheadp ;    /* index buffer pointer    */
    register int     i, n;
    register meUByte **list;

    n = 0;
    while(bp != NULL)
    {
        if(!noHidden || !meModeTest(bp->mode,MDHIDE))
            n++;
        bp = bp->next;
    }
    if((list = (meUByte **) meMalloc(sizeof(meUByte *) * n)) == NULL)
        return 0;
    bp = bheadp;
    for(i=0 ; i<n ; )
    {
        if(!noHidden || !meModeTest(bp->mode,MDHIDE))
            list[i++] = bp->name;
        bp = bp->next;
    }
    *listPtr = list;
    return i;
}

#if MEOPT_MAJORMODE
int
createMajorModeList(meUByte ***listPtr)
{
    meMajorMode *mp=majorModeHead;
    meUByte **list;
    int ii;

    ii = 0;
    while(mp != NULL)
    {
        ii++;
        mp = mp->next;
    }
    if((list = (meUByte **) meMalloc(ii*sizeof(meUByte *))) == NULL)
        return 0;
    mp = majorModeHead;
    ii = 0;
    while(mp != NULL)
    {
        list[ii++] = mp->id;
        mp = mp->next;
    }
    *listPtr = list;
    return ii;
}
#endif

int
createVarList(meUByte ***listPtr)
{
    int ii;
    meUByte **list;
#if MEOPT_EXTENDED
    meVariable *vp;     	/* User variable pointer */
#endif
    
    ii = NEVARS;
#if MEOPT_EXTENDED
    vp = usrVarList;
    while(vp != NULL)
    {
        ii++;
        vp = vp->next;
    }
    vp = frameCur->windowCur->buffer->varList;
    while(vp != NULL)
    {
        ii++;
        vp = vp->next;
    }
#endif    
    if((list = (meUByte **) meMalloc(sizeof(meUByte *) * ii)) == NULL)
        return 0;
    *listPtr = list;
    
    for(ii=0 ; ii<NEVARS; ii++)
    {
        if((list[ii] = meMalloc(meStrlen(envars[ii])+2)) == NULL)
            return 0;
        list[ii][0] = '$';
        meStrcpy(list[ii]+1,envars[ii]);
    }
#if MEOPT_EXTENDED
    vp = usrVarList;
    while(vp != NULL)
    {
        if((list[ii] = meMalloc(meStrlen(vp->name)+2)) == NULL)
            return 0;
        list[ii][0] = '%';
        meStrcpy(list[ii]+1,vp->name);
        ii++;
        vp = vp->next;
    }
    vp = frameCur->windowCur->buffer->varList;
    while(vp != NULL)
    {
        if((list[ii] = meMalloc(meStrlen(vp->name)+2)) == NULL)
            return 0;
        list[ii][0] = ':';
        meStrcpy(list[ii]+1,vp->name);
        ii++;
        vp = vp->next;
    }
#endif
    return ii;
}
    
int
createCommList(meUByte ***listPtr, int noHidden)
{
    meCommand *cmd;
    register int ii;
    register meUByte **list;
    
    if((list = meMalloc(sizeof(meUByte *) * (cmdTableSize))) == NULL)
        return 0;
    ii = 0;
    cmd = cmdHead;
    while(cmd != NULL)
    {
        if(!noHidden || (cmd->id < CK_MAX) ||
           !(((meMacro *) cmd)->hlp->flag & meMACRO_HIDE))
            list[ii++] = cmd->name;
        cmd = cmd->anext;
    }
    *listPtr = list;
    return ii;
}



#if MEOPT_MOUSE
static int
mlHandleMouse(meUByte *inpBuf, int inpBufSz, int compOff)
{
    meWindow *cwp=frameCur->windowCur;
    int row, col;
    
    if(((row=mouse_Y-cwp->frameRow) >= 0) && (row < cwp->depth-1) &&
       ((col=mouse_X-cwp->frameColumn) >= 0) && (col < cwp->width))
    {
        if (col >= cwp->textWidth)
        {
            /* only do scroll bar if on pick and bars are enabled */
            if((inpBuf == NULL) && (cwp->vertScrollBarMode & WMSCROL))
            {
                int ii;
                for (ii = 0; ii <= (WCVSBML-WCVSBSPLIT); ii++)
                    if (mouse_Y < (meShort) cwp->vertScrollBarPos[ii])
                        break;
                if(ii == (WCVSBUP-WCVSBSPLIT))
                    windowScrollUp(1,1);
                else if(ii == (WCVSBUSHAFT-WCVSBSPLIT))
                    windowScrollUp(0,1);
                else if(ii == (WCVSBDOWN-WCVSBSPLIT))
                    windowScrollDown(1,1);
                else if(ii == (WCVSBDSHAFT-WCVSBSPLIT))
                    windowScrollDown(0,1);
                update(meTRUE);
            }
        }
        else
        {
            meLine *lp;
            int ii, jj, lineNo, sc, ss, ll;
            
            row += cwp->vertScroll;
            lineNo = row;
            lp = cwp->buffer->baseLine->next;
            while(--row >= 0)
                lp = meLineGetNext(lp);
            /* a line of 2 completions has the second at cell markOffset, after the first's padding */
            ii = sc = 0;
            jj = meLineGetLength(lp);
            if(lp->flag & meLINE_NOEOL)
            {
                for(ss=0 ; (ss < cwp->markOffset) && (ii < jj) ; ii+=ll)
#if MEOPT_UNICODE
                    ss += meStrCharCells(meLineGetText(lp)+ii,jj-ii,&ll);
#else
                    ss++, ll=1;
#endif
                if(col >= cwp->markOffset)
                    sc = cwp->markOffset;
                else
                {
                    jj = ii;
                    while((jj > 0) && (meLineGetChar(lp,jj-1) == ' '))
                        jj--;
                    ii = 0;
                }
            }
#if MEOPT_UNICODE
            if((sc + meStrCells(meLineGetText(lp)+ii,jj-ii)) > col)
#else
            if((sc + jj - ii) > col)
#endif
            {
                cwp->dotLine = lp;
                cwp->dotOffset = ii;
                cwp->dotLineNo = lineNo;
                setShowRegion(cwp->buffer,lineNo,ii,lineNo,jj);
                cwp->updateFlags |= WFMOVEL|WFSELHIL;
                if(inpBuf != NULL)
                {
                    if((jj -= ii) >= (inpBufSz-compOff))
                        jj = inpBufSz-compOff-1;
                    /* if we already have this, assume the user has double clicked
                     * to 'select', so return 2 to exit */
                    if((inpBuf[compOff+jj] == '\0') && 
                       !meStrncmp(inpBuf+compOff,meLineGetText(lp)+ii,jj))
                        return 2;
                    meStrncpy(inpBuf+compOff,meLineGetText(lp)+ii,jj);
                    inpBuf[compOff+jj] = '\0';
                }
                update(meTRUE);
                return 1;
            }
        }
    }
    return 0;
}
#endif

meUByte *compSole     = (meUByte *)" [Sole completion]";
meUByte *compNoMch    = (meUByte *)" [No match]";
meUByte *compNoExp    = (meUByte *)" [No expansion]";
meUByte *compFailComp = (meUByte *)" [Failed to create]";
#if MEOPT_SOCKET
meUByte *compFtpComp  = (meUByte *)" [FTP completion?]";
#endif

#if MEOPT_OSD
#define mlgsDisp(prom,buf,contstr,ipos) \
((frameCur->mlStatus & MLSTATUS_POSOSD) ? osdDisp(buf,contstr,ipos):mlDisp(prom,buf,contstr,ipos))
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
meGetStringFromUser(meUByte *prompt, int option, int defnum, meUByte *buf, int nbuf)
{
#if MEOPT_LOCALBIND
    meUByte  oldUseMlBinds;
#endif
    meWindow *mlgsOldCwp=NULL;
    meBuffer *mlgsOldWBp=NULL;
    meInt    mlgsSingWind=0;
#if MEOPT_EXTENDED
    int     curPos;
#endif
    int     cc;
    int     ii;
    int     ipos ;                      /* input position in buffer */
    int     ilen ;                      /* number of chars in buffer */
    int     cont_flag ;                 /* Continue flag */
    meKill  *lastYank;
    meUByte **history;
    meUByte onHist, numHist, *numPtr;
    meUByte *defaultStr;
    meUByte prom[meBUF_SIZE_MAX], storeBuf[meBUF_SIZE_MAX];
    meUByte **strList;
#if !MEOPT_UNICODE
    meUByte ch;
#endif
    meUByte *contstr=NULL;
    int     gotPos=1, fstPos, lstPos, mrkPos=0, noStrs;
    int     changed=1, compOff=0;
    
    /* Blank line to force update
     * Don't do this if in an osd dialog as the osd cursor position is not
     * calculated until the first call to osdDisp, osdCol is likely to be -1
     * which will blow ME up */
    if(!(frameCur->mlStatus & MLSTATUS_POSOSD))
        mlerase(0);

    if(option & MLINSENSCASE)
    {
        curCmpFunc  = meStridif;
#if MEOPT_UNICODE
        curCmpIFunc = (meIFuncSSI) mlStrnifold;
#else
        curCmpIFunc = meStrnicmp;
#endif
    }
    else
    {
        curCmpFunc  = (meIFuncSS) strcmp;
        curCmpIFunc = (meIFuncSSI) strncmp;
    }
    numHist = setupHistory(option, &numPtr, &history);
    if(option & MLBUFFER)
        noStrs = createBuffList(&strList,1);
    else if(option & MLVARBL)
        noStrs = createVarList(&strList);
    else if(option & MLCOMMAND)
        noStrs = createCommList(&strList,1);
    else if(option & MLMODE)
    {
        strList = mlgsStrList;
        noStrs = mlgsStrListSize;
    }
#if MEOPT_MAJORMODE
    else if(option & MLMAJORMODE)
        noStrs = createMajorModeList(&strList);
#endif /* MEOPT_MAJORMODE */
    else
    {
        strList = NULL;
        noStrs = 0;
    }
    if(strList != NULL)
        sortStrings(noStrs,strList,0,curCmpFunc);

    if(option & MLNOHIST)
        onHist = ~0;
    else
        onHist = numHist+1;
    mlInputFlags = 0;
    meStrcpy(prom,prompt);
    ii = (int) meStrlen(prom);
    if((defnum > 0) && (defnum <= numHist))
    {
        defaultStr = history[defnum-1];
        memcpy(prom+ii," (default [",11);
        ii = expandexp(-1,defaultStr,meBUF_SIZE_MAX-5,ii+11,prom,-1,NULL,0);
        meStrcpy(prom+ii,"]): ");
    }
    else
    {
        defaultStr = NULL;
        meStrcpy(prom+ii,": ");
    }

    if(option & MLNORESET)
    {
        ilen = (int) meStrlen(buf);
#if MEOPT_OSD
        if(frameCur->mlStatus & MLSTATUS_OSDPOS)
        {
            meUByte *s1, *s2;
            s1 = buf;
            while((--osdRow >= 0) && ((s2 = meStrchr(s1,meCHAR_NL)) != NULL))
                s1 = s2+1;
            if(osdRow >= 0)
                ipos = ilen;
            else
            {
                ipos = ((int) (s1 - buf)) + osdCol;
                if(ipos > ilen)
                    ipos = ilen;
                if(((s2 = meStrchr(s1,meCHAR_NL)) != NULL) &&
                    (((int)(s2 - buf)) < ipos))
                    ipos = (int) (s2 - buf);
            }
        }
        else
#endif
            ipos = ilen;
    }
    else
    {
        ipos = ilen = 0;
        buf[0] = '\0';
    }
#if MEOPT_LOCALBIND
    oldUseMlBinds = useMlBinds;
    useMlBinds = 1;
#endif
    /* the user must be able to see what they are typing */
    frameCur->mlStatus |= MLSTATUS_CURSHOW;
    meCursorUpdate();
    for(cont_flag=0 ; cont_flag == 0 ;)
    {
        meUInt arg;
        int idx;
        int ff;
        
        if(option & MLHIDEVAL)
        {
            meUByte hbuf[meBUF_SIZE_MAX];
            int hpos=ipos;
#if MEOPT_UNICODE
            /* a star for each char */
            for(ff=0, ii=0, hpos=0 ; buf[ii] != '\0' ; ff++, ii=meUtf8Next(buf,ii,meBUF_SIZE_MAX))
                if(ii < ipos)
                    hpos++;
#else
            ff = (int) meStrlen(buf);
#endif
            meAssert(ff < meBUF_SIZE_MAX);
            memset(hbuf,'*',ff);
            hbuf[ff] = '\0';
            mlgsDisp(prom,hbuf,contstr,hpos);
        }
        else
            mlgsDisp(prom,buf,contstr,ipos);
        contstr = NULL;
        mlInputFlags >>= 4;

        cc = meGetKeyFromUser(meFALSE,0,meGETKEY_SILENT);
        idx = decode_key((meKey) cc,&arg);
        if(arg)
        {
            ff = 1;
            ii = (int) (arg + 0x80000000);
        }
        else
        {
            ff = 0;
            ii = 1;
        }
        switch(idx)
        {
        case CK_GOBOF:          /* Home : Move to start of buffer */
            ipos = 0;
            break;
        
        case CK_GOBOL:          /* ^A : Move to start of line */
            if(frameCur->mlStatus & MLSTATUS_NINPUT)
            {
                while(ipos && (buf[ipos-1] != meCHAR_NL))
                    ipos--;
            }
            else
                ipos = 0;
            break;
            
        case CK_BAKCHR:    /* ^B : Backwards char */
            while(ipos && ii--)
                ipos = mlPrevChar(buf,ipos);
            break;
            
        case CK_DELFOR:    /* ^D : Delete the character under the cursor */
            if((ipos < ilen) & ii)
            {
                ff = ipos;
                while((ff < ilen) && (ii-- > 0))
                    ff = mlNextChar(buf,ff,ilen);
                ilen = mlForwardDelete(buf,ipos,ilen,ff-ipos,1);
                changed=1;
            }
            break;
            
        case CK_GOEOF:    /* End : Move to end of buffer */
            ipos = ilen;
            break;
        
        case CK_GOEOL:    /* ^E : Move to end of line */
            if(frameCur->mlStatus & MLSTATUS_NINPUT)
            {
                while((ipos < ilen) && (buf[ipos] != meCHAR_NL))
                    ipos++;
            }
            else
                ipos = ilen;
            break;
            
        case CK_FORCHR:    /* ^F : Move forward one character */
            while((ipos < ilen) && ii--)
                ipos = mlNextChar(buf,ipos,ilen);
            break;
            
        case CK_ABTCMD: /* ^G : Abort input and return */
            cont_flag = 5;
            break;
            
        case CK_DELBAK:    /* ^H : backwards delete. */
            if(ipos && ii)
            {
                ff = ipos;
                while((ipos > 0) && (ii-- > 0))
                    ipos = mlPrevChar(buf,ipos);
                ilen = mlForwardDelete(buf,ipos,ilen,ff-ipos,1);
                changed=1;
            }
            break;
#if MEOPT_EXTENDED
        case CK_DELTAB:
#if MEOPT_OSD
            if(frameCur->mlStatus & MLSTATUS_POSOSD)
            {
                meGetKeyFirst = cc;
                cont_flag = 3;
            }
            else
#endif
                TTbell();
            break;
#endif        
        case CK_DOTAB:    /* tab : Tab character */
#if MEOPT_OSD
            if(frameCur->mlStatus & MLSTATUS_POSOSD)
            {
                meGetKeyFirst = cc;
                cont_flag = 3;
                break;
            }
#endif
        case CK_INSTAB:
            cc = '\t' ;    /* ^I for search strings */
            if(!(option & (MLCOMMAND | MLFILE | MLBUFFER | MLVARBL | MLMODE | MLMAJORMODE)))
                goto input_addexpand;
input_expand:
            if(option & MLFILE)
            {
                meUByte ft, fname[meBUF_SIZE_MAX], *base;
                
                pathNameCorrect(buf,PATHNAME_PARTIAL,fname,&base);
                ipos = (int) meStrlen(fname);
                if((fname[0] == DIR_CHAR) && (fname[1] == DIR_CHAR) && (base == fname+2) && (buf[ilen-1] != '/') && (buf[ilen-1] != '\\'))
                {
                    /* don't try to expand '//xxx' as we can't get a list of remote servers */
                    if(fname[ipos-1] == DIR_CHAR)
                        fname[--ipos] = '\0';
                    meStrcpy(buf,fname);
                    ilen = ipos;
                    contstr = compNoExp;
                    TTbell();
                    break;
                }
                meStrcpy(buf,fname);
                ilen = ipos;
                /* if current buff is xxx/yyy/ then pathNameCorrect will
                 * return the path as xxx/ and base as yyy/ but for completion
                 * we want to list yyy/ so move the base for the case */
                if(buf[ilen-1] != DIR_CHAR)
                    *base = '\0';
                ft = ffUrlGetType(fname);
                if(ffUrlTypeIsHttpDict(ft))
                    strList = NULL;
                else
                {
#if MEOPT_SOCKET
                    if(ffUrlTypeIsFtp(ft) && ((curDirList.path == NULL) || meStrcmp(curDirList.path,fname)))
                    {
                        changed ^= 1;
                        if(!changed)
                        {
                            contstr = compFtpComp;
                            break;
                        }
                    }
#endif
                    compOff = (int) meStrlen(fname);
                    getDirectoryList(fname,&curDirList);
                    if(noStrs != curDirList.size)
                    {
                        changed = 1;
                        noStrs = curDirList.size;
                    }
                    strList = curDirList.list;
                }
            }
            if(strList == NULL)
            {
                contstr = compNoExp;
                TTbell();
                break;
            }
            if(changed)
            {
                changed = 0;
                if((gotPos = getFirstLastPos(noStrs,strList,buf+compOff,option,&fstPos,&lstPos)) == 0)
                {
                    contstr = compNoMch;
                    TTbell();
                    break;
                }
#if MEOPT_EXTENDED
                curPos = fstPos-1;
#endif
                if(fstPos == lstPos)
                {
                    meStrcpy(buf+compOff,(strList[lstPos]));
                    ilen += (int) meStrlen(buf+ilen);
                    if((option & MLFILE) && (buf[ilen-1] == DIR_CHAR))
                    {
                        /* if we're entering a file name and we've just
                         * completed to the a directory name then the next
                         * 'TAB' completion should be for inside the new directory,
                         * so effectively we have changed the input point */
                        changed = 1;
                        if((ilen >= 4) && (buf[ilen-4] == DIR_CHAR) && 
                           (buf[ilen-3] == '.') && (buf[ilen-2] == '.'))
                            goto input_expand;
                    }
                }
                else
                {
#if MEOPT_UNICODE
                    ii = mlBytesToChars(buf+compOff,ilen-compOff);
                    if(mlCompleteHasChar(strList,fstPos,lstPos,ii,cc))
                        goto input_addexpand;
                    ilen = mlCompleteCommon(buf,ilen,nbuf,strList,fstPos,lstPos,ii,option & MLINSENSCASE);
#else
                    for(ii=fstPos ; ii<=lstPos ; ii++)
                        if(strList[ii][ilen-compOff] == cc)
                            goto input_addexpand;
                    for(;;)
                    {
                        ch = strList[lstPos][ilen-compOff];
                        for(ii=fstPos ; ii<lstPos ; ii++)
                            if((strList[ii][ilen-compOff] != ch) &&
                               (((option & MLINSENSCASE) == 0) ||
                                ((meUByte) toLower(strList[ii][ilen-compOff]) != (meUByte) toLower(ch))))
                                break;
                        if(ii != lstPos)
                            break;
                        if(ch == '\0')
                            break;
                        buf[ilen++] = ch;
                    }
#endif
                    buf[ilen] = '\0';
                }
                ipos = ilen;
            }
            else if(!gotPos)
                contstr = compNoMch;
            else if(fstPos == lstPos)
                contstr = compSole;
            else
            {
                meWindow *cwp;
                meBuffer *cbp, **br;
#if MEOPT_UNICODE
                /* a column is up to 75 cells and a cell can take 4 bytes */
                meUByte line[4*150+1];
#else
                meUByte line[150];
#endif
                int jj, last, len, lwidth, ww;
                
                for(ii=fstPos ; ii<=lstPos ; ii++)
                    if(strList[ii][ilen-compOff] == cc)
                        goto input_addexpand;
                /* if a new window is created to display the completion list
                 * then store the information we need to delete it once we
                 * have finished with it */
                if(mlgsOldCwp == NULL)
                {
                    mlgsOldCwp = frameCur->windowCur;
                    mlgsSingWind = mlgsOldCwp->vertScroll+1;
                    br = &mlgsOldWBp;
                }
                else
                    br = NULL;
                if(meWindowPopup(NULL,BcompleteN,BFND_CREAT|BFND_CLEAR|WPOP_MKCURR,br) == NULL)
                {
                    contstr = compFailComp;
                    break;
                }
                cwp = frameCur->windowCur;
                cbp = cwp->buffer;
                /* remove any completion list selection hilighting */
                if(selhilight.bp == cbp)
                    selhilight.flags &= ~SELHIL_ACTIVE;
                
                /* Compute the widths available from the window width */
                lwidth = cwp->textWidth >> 1;
                if (lwidth > 75)
                    lwidth = 75;
                cwp->markOffset = lwidth;
                jj = (option & MLFILE) ? 1:0;
                last = -1;
                do {
                    for(ii=fstPos ; ii<=lstPos ; ii++)
                    {
#if MEOPT_EXTENDED
                        if(!jj || !isFileIgnored(strList[ii]))
#endif
                        {
                            if(last >= 0)
                            {
                                if(((ww=mlCompCells(strList[last])) < lwidth) && 
                                   (mlCompCells(strList[ii]) < lwidth))
                                {
                                    len = (int) meStrlen(strList[last]);
                                    meStrcpy(line,strList[last]);
                                    memset(line+len,' ',lwidth-ww);
                                    meStrcpy(line+len+lwidth-ww,strList[ii]);
                                    addLineToEob(cbp,line);
                                    cbp->baseLine->prev->flag |= meLINE_NOEOL;
                                    last = -1;
                                }
                                else
                                {
                                    addLineToEob(cbp,strList[last]);
                                    last = ii;
                                }
                            }
                            else if(mlCompCells(strList[ii]) < lwidth)
                            {
                                line[0] = '\0';
                                last = ii;
                            }
                            else
                                addLineToEob(cbp,strList[ii]);
                        }
                    }
                    if(last >= 0)
                        addLineToEob(cbp,strList[last]);
                } while((cbp->lineCount == 0) && (--jj >= 0));
                windowGotoBob(meFALSE,meFALSE);
                update(meTRUE);
            }
            break;
            
        case CK_MOVUWND:
            if(mlgsOldCwp != NULL)
            {
                windowScrollUp(ff,ii);
                update(meTRUE);
            }
            break;
            
        case CK_MOVDWND:
            if(mlgsOldCwp != NULL)
            {
                windowScrollDown(ff,ii);
                update(meTRUE);
            }
            break;
            
        case CK_BAKLIN: /* M-P - previous match in complete list */
            if(frameCur->mlStatus & MLSTATUS_NINPUT)
            {
                ii = ipos;
                while((--ii >= 0) && (buf[ii] != meCHAR_NL))
                    ;
                if(ii >= 0)
                {
                    int jj;
                    jj = ipos - ii;
                    ipos = ii;
                    /* work out the new line offset */
                    while((--ii >= 0) && (buf[ii] != meCHAR_NL))
                        ;
                    if((ii+jj) < ipos)
                        ipos = ii+jj;
                }
                break;
            }
#if MEOPT_OSD
            if(frameCur->mlStatus & MLSTATUS_POSOSD)
            {
                meGetKeyFirst = cc;
                cont_flag = 3;
                break;
            }
#endif
            /* no break - if not multi-line input then backward-line also cycles history */
#if MEOPT_WORDPRO
        case CK_GOBOP:    /* M-P : Got to previous in history list */
#endif
            if(!(option & MLNOHIST) && (numHist > 0))
            {
                if(onHist > numHist)
                {
                    /* if on current then save */
                    meStrcpy(storeBuf,buf);
                    onHist = 0;
                }
                else
                    ++onHist;
                if(onHist > numHist)
                    meStrcpy(buf,storeBuf);
                else if(onHist == numHist)
                {
                    meStrcpy(prom+meStrlen(prompt),": ");
                    defaultStr = NULL;
                    buf[0] = '\0';
                }
                else
                {
                    meStrncpy(buf,history[onHist],nbuf);
                    buf[nbuf-1] = '\0';
                }
                ipos = ilen = (int) meStrlen(buf);
                changed=1;
            }
            break;
            
        case CK_FORLIN: /* ^N - next match in complete list */
            if(frameCur->mlStatus & MLSTATUS_NINPUT)
            {
                ii = ipos;
                while((ii < ilen) && (buf[ii] != meCHAR_NL))
                    ii++;
                if(ii < ilen)
                {
                    int jj;
                    jj = ipos;
                    while(jj-- && (buf[jj] != meCHAR_NL))
                        ;
                    jj = ipos-jj-1;
                    ipos = ii+1;
                    while((--jj >= 0) && (ipos < ilen) && (buf[ipos] != meCHAR_NL))
                        ipos++;
                }
                break;
            }
#if MEOPT_OSD
            if(frameCur->mlStatus & MLSTATUS_POSOSD)
            {
                meGetKeyFirst = cc;
                cont_flag = 3;
                break;
            }
#endif
            /* no break - if not multi-line input then forward-line also cycles history */
#if MEOPT_WORDPRO
        case CK_GOEOP:    /* M-N : Got to next in history list */
#endif
            /* Note the history list is reversed, ie 0 most recent,
            ** (numHist-1) the oldest. However if numHist then its
            ** the current one (wierd but easiest to implement
            */
            if(!(option & MLNOHIST) && (numHist > 0))
            {
                if(onHist > numHist)
                    /* if on current then save */
                    meStrcpy(storeBuf,buf);
                if(onHist == 0)
                {
                    onHist = numHist+1;
                    meStrcpy(buf,storeBuf);
                }
                else if(onHist > numHist)
                {
                    meStrcpy(prom+meStrlen(prompt),": ");
                    defaultStr = NULL;
                    onHist = numHist;
                    buf[0] = '\0';
                }
                else
                {
                    meStrncpy(buf,history[--onHist],nbuf);
                    buf[nbuf-1] = '\0';
                }                    
                ipos = ilen = (int) meStrlen(buf);
                changed=1;
            }
            break;
            
        case CK_KILEOL:    /* ^K : Kill to end of line */
            ff = ipos;
            if(ii > 0)
            {
                if(frameCur->mlStatus & MLSTATUS_NINPUT)
                {
                    while((ff < ilen) && (buf[ff] != meCHAR_NL))
                        ff++;
                    /* kill \n as if we were at the end of the line or arg
                     * given (as per kill-line) */
                    if((ff == ipos) ||
                       (ff && (ff < ilen) &&
                        ((ipos == 0) || (buf[ipos-1] == meCHAR_NL))))
                        ff++;
                }
                else
                    ff = ilen;
            }
            else if((ii < 0) && ipos)
            {
                if(frameCur->mlStatus & MLSTATUS_NINPUT)
                {
                    ipos--;
                    while((ipos > 0) && (buf[ipos-1] != meCHAR_NL))
                        ipos--;
                }
                else
                {
                    ipos = 0;
                }
            }
            if((ff -= ipos) > 0)
            {
                ilen = mlForwardDelete(buf,ipos,ilen,ff,3);
                changed=1;
            }
            break;
            
        case CK_RECENT:  /* ^L : Redraw the screen */
            sgarbf = meTRUE;
            update(meTRUE);
            mlerase(0);
            break;
            
        case CK_NEWLIN:  /* ^J : New line. Finish processing */
            if(frameCur->mlStatus & MLSTATUS_NINPUT)
            {
                cc = meCHAR_NL;
                goto input_addexpand;
            }
#if MEOPT_OSD
            if(frameCur->mlStatus & MLSTATUS_POSOSD)
                meGetKeyFirst = cc;
#endif
            cont_flag = 3;
            break;
            
#if MEOPT_EXTENDED
        case CK_SCLNXT:    /* M-N : Got to next in completion list */
            if((option & (MLCOMMAND | MLFILE | MLBUFFER | MLVARBL | MLMODE | MLMAJORMODE)) &&
               (strList != NULL) && !changed && gotPos)
            {
                if(fstPos == lstPos)
                    contstr = compSole;
                else
                {
                    if(++curPos > lstPos)
                        curPos = fstPos;
                    meStrncpy(buf+compOff,strList[curPos],nbuf-compOff);
                    buf[nbuf-1] = '\0';
                    ipos = ilen = (int) meStrlen(buf);
                }
            }
            break;
            
        case CK_SCLPRV:    /* M-N : Got to prev in completion list */
            if((option & (MLCOMMAND | MLFILE | MLBUFFER | MLVARBL | MLMODE | MLMAJORMODE)) &&
               (strList != NULL) && !changed && gotPos)
            {
                if(fstPos == lstPos)
                    contstr = compSole;
                else
                {
                    if(--curPos < fstPos)
                        curPos = lstPos;
                    meStrncpy(buf+compOff,strList[curPos],nbuf-compOff);
                    buf[nbuf-1] = '\0';
                    ipos = ilen = (int) meStrlen(buf);
                }
            }
            break;
#endif
            
        case CK_TRNCHR:
            /* ^T : Transpose the character before the cursor
             *      with the one under the cursor.
             */
            if(ipos && ipos < ilen)
            {
#if MEOPT_UNICODE
                meUByte bb[meUTF8_LEN_MAX*2];
                int lo=meUtf8Prev(buf,ipos), eo=meUtf8Next(buf,ipos,ilen);
                memcpy(bb,buf+ipos,eo-ipos);
                memcpy(bb+eo-ipos,buf+lo,ipos-lo);
                memcpy(buf+lo,bb,eo-lo);
                ipos = lo+eo-ipos;
#else
                cc = buf[ipos - 1];
                buf[ipos - 1] = buf[ipos];
                buf[ipos] = cc;
#endif
                changed=1;
            }
            break;
            
        case CK_SWPMRK:
            ii = mrkPos;
            mrkPos = ipos;
            if((ipos=ii) > ilen)
                ipos = ilen;
            break;
        
        case CK_SETMRK:
            mrkPos = ipos;
            break;
            
        case CK_CPYREG:    /* M-w : Copy region */
            ii = ipos;
            if(mrkPos < ipos)
            {
                ff = ipos;
                ipos = mrkPos;
            }
            else
                ff = mrkPos;
            if((ff -= ipos) > 0)
                ilen = mlForwardDelete(buf,ipos,ilen,ff,2);
            ipos = ii;
            break;
        
        case CK_KILREG:    /* C-w : Kill region */
            if(mrkPos < ipos)
            {
                ff = ipos;
                ipos = mrkPos;
            }
            else
                ff = mrkPos;
            if((ff -= ipos) > 0)
            {
                ilen = mlForwardDelete(buf,ipos,ilen,ff,3);
                changed=1;
            }
            break;
        
        case CK_QUOTE:    /* ^Q - quote the next character */
            if(ii < 0)
            {
                /* bound with a -ve argument, the char is given by its code and inserted -n times */
                meUByte cb[meCHAR_LEN_MAX];
                int cl, jj;
                if((cl = meGetCharCode(NULL,cb)) < 0)
                {
                    cont_flag = 5;
                    break;
                }
                for(ii=-ii ; (cl > 0) && (ii > 0) ; ii--)
                {
                    if(((int) meStrlen(buf) + cl) >= nbuf)
                    {
                        TTbell();
                        break;
                    }
                    for(jj=0 ; jj<cl ; jj++)
                        mlInsertChar(cb[jj],buf,&ipos,&ilen,nbuf);
                }
                changed=1;
                break;
            }
            if((cc = quoteKeyToChar(tgetc())) < 0)
            {
                TTbell();
                break;
            }
            while(ii--)
                if(mlInsertKeyChar(cc, buf, &ipos, &ilen, nbuf) == meFALSE)
                    TTbell();
            changed=1;
            break;
            
        case CK_OPNLIN:    /* ^O : Insert current line into buffer */
            {
                register meUByte cc, *pp=frameCur->windowCur->dotLine->text;
                
                while((cc=*pp++) != '\0')
                    mlInsertChar(cc,buf,&ipos,&ilen,nbuf);
                changed=1;
                break;
            }
        case CK_YANK:    /* ^Y : insert yank buffer */
            {
                register meUByte *pp, cy;
                meKillNode *killp;
#ifdef _CLIPBRD
                TTgetClipboard((kbdmode == mePLAY) ? 0:1);
#endif
                if((lastYank=klhead) == (meKill*) NULL)
                {
                    TTbell();
                    break;
                }
ml_yank:
                mrkPos = ipos;
                killp = lastYank->kill;
                while(killp != NULL)
                {
                    pp = killp->data;
                    while((cy=*pp++))
                        if(mlInsertChar(cy, buf, &ipos, &ilen, nbuf) == meFALSE)
                        {
                            TTbell();
                            break;
                        }
                    killp = killp->next;
                }
                mlInputFlags |= mlINPUT_THIS_YANK;
                changed=1;
                break;
            }
            
        case CK_REYANK:    /* ^Y : reyank kill buffer */
            if(mlInputFlags & mlINPUT_LAST_YANK)
            {
                ff = ipos;
                ipos = mrkPos;
                if((ff -= ipos) > 0)
                    ilen = mlForwardDelete(buf,ipos,ilen,ff,1);
                if((lastYank = lastYank->next) == NULL)
                    lastYank = klhead;
                mlInputFlags |= mlINPUT_THIS_REYANK;
                goto ml_yank;
            }
            TTbell();
            break;
            
        case CK_INSFLNM:    /* insert file name */
            {
                register meUByte ch, *p;
                meBuffer *cbp=frameCur->windowCur->buffer;
                p = (ii < 0) ? cbp->name:cbp->fileName;
                if(p != NULL)
                    while(((ch=*p++) != '\0') && mlInsertChar(ch, buf, &ipos, &ilen, nbuf))
                        ;
                break;
            }
            
            /*
             * Handle a limited set of Meta sequences. Note that
             * there is no conflict when the end-of-line char is
             * escape, since we will never reach here if it is.
             */
        case CK_BAKWRD:    /* M-B : Move to start of previous word */
            while(ipos && ii--)
            {
                mlBackwardToNextWord(buf,ipos);
                mlBackwardToNextSpace(buf,ipos);
            }
            break;
            
        case CK_FORWRD:    /* M-F : Move forward to start of next word. */
            while((ipos != ilen) && ii--)
            {
                mlForwardToNextWord(buf,ipos,ilen);
                mlForwardToNextSpace(buf,ipos,ilen);
            }
            break;
            
        case CK_CAPWRD:    /* M-C : Capitalise next word and move past it. */
            while(ilen && (ipos != ilen) && ii--)
            {
                mlForwardToNextWord(buf,ipos,ilen);
                if(ipos < ilen)
                    ipos = mlCharCase(buf,ipos,&ilen,nbuf,1);
                while((ipos<ilen) && mlIsAlphaNum(buf,ipos))
                    ipos = mlCharCase(buf,ipos,&ilen,nbuf,0);
                changed=1;
            }
            break;
            
        case CK_HIWRD:    /* M-U : Capitalise the whole of the next
                           *      word and move past it. */
            while(ii--)
            {
                mlForwardToNextWord(buf,ipos,ilen);
                while((ipos<ilen) && mlIsAlphaNum(buf,ipos))
                    ipos = mlCharCase(buf,ipos,&ilen,nbuf,1);
                changed=1;
            }
            break;
            
        case CK_LOWWRD:   /* M-L : Converts to lower case the whole of the next
                           *      word and move past it. */
            while(ii--)
            {
                mlForwardToNextWord(buf,ipos,ilen);
                while((ipos<ilen) && mlIsAlphaNum(buf,ipos))
                    ipos = mlCharCase(buf,ipos,&ilen,nbuf,0);
                changed=1;
            }
            break;
            
        case CK_DELFWRD: /* Delete word forwards. */
            ff = ipos;
            while((ff < ilen) && ii--)
            {
                mlForwardToNextWord(buf,ff,ilen);
                mlForwardToNextSpace(buf,ff,ilen);
            }
            if((ff -= ipos) > 0)
            {
                ilen = mlForwardDelete(buf,ipos,ilen,ff,3);
                changed=1;
            }
            break;
            
        case CK_DELWBAK:
            ff = ipos;
            while((ipos > 0) && ii--)
            {
                mlBackwardToNextWord(buf,ipos);
                mlBackwardToNextSpace(buf,ipos);
            }
            if((ff -= ipos) > 0)
            {
                ilen = mlForwardDelete(buf,ipos,ilen,ff,3);
                changed=1;
            }
            break;
            
#if MEOPT_ISEARCH
        case CK_BISRCH:
        case CK_FISRCH:
        case CK_BAKSRCH:
        case CK_FORSRCH:
            if(option & MLISEARCH)
            {
                meGetKeyFirst = cc;
                cont_flag = 1;
            }
            else
                TTbell();
            break;
#endif
#if MEOPT_MOUSE
        case CK_CTOMOUSE:
            /* a binding to set-cursor-to-mouse is used to handle mouse
             * events, needs to handle osd pick and normal completion lists */
            /* is it a pick event */
            if((cc == (ME_SPECIAL|SKEY_mouse_pick_1)) ||
               (cc == (ME_SPECIAL|SKEY_mouse_pick_2)) ||
               (cc == (ME_SPECIAL|SKEY_mouse_pick_3)) )
            {
#if MEOPT_OSD
                if(frameCur->mlStatus & MLSTATUS_POSOSD)
                {
                    if(osdDisplayMouseLocate(1) > 0)
                    {
                        meGetKeyFirst = cc;
                        cont_flag = 3;
                    }
                }
                else if(mlgsOldCwp != NULL)
#endif
                    mlHandleMouse(NULL,0,0);
            }
            /* a drop event */
            else if((mlgsOldCwp != NULL) &&
#if MEOPT_OSD
                    ((frameCur->mlStatus & MLSTATUS_POSOSD) == 0) && 
#endif            
                    ((cc=mlHandleMouse(buf,nbuf,compOff)) != 0))
            {
                ipos = ilen = (int) meStrlen(buf);
                if(cc == 2)
                    cont_flag = 3;
                else
                    changed = 1;
            }
            break;
#endif            
        case CK_VOIDFUNC:
            break;
            
        case -1:
            if(!meKeyIsChar(cc))    /* if control, meta or prefix then scrap */
            {
#if MEOPT_MOUSE
                /* ignore mouse move events - these are sometimes generated
                 * when executing a command from osd */
                if((cc & ME_KEY_CHAR) != SKEY_mouse_move)
#endif                
                    TTbell();
                break;
            }
            
            if (cc == ' ')    /* space */
            {
                if(option & (MLCOMMAND | MLFILE | MLBUFFER | MLVARBL | MLMODE | MLMAJORMODE))
                    goto input_expand;
                if(option & MLNOSPACE)
                {
                    TTbell();
                    break;
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
            if(mlInsertKeyChar(cc, buf, &ipos, &ilen, nbuf) == meFALSE)
                TTbell();
            changed=1;
            break;
            
        default:
            TTbell();
        }
    }
    
    if(cont_flag & 0x02)
    {
        /* if no string has been given and there's a default then return this */
        if((ilen == 0) && (defaultStr != NULL))
        {
            meStrncpy(buf,defaultStr,nbuf);
            buf[nbuf-1] = '\0';
        }
        /* Store the history if it is not disabled. */
        if((option & (MLNOHIST|MLNOSTORE)) == 0)
        {
            if((option & MLFILE) && ((meStrchr(buf,DIR_CHAR) != NULL) 
#ifdef _CONVDIR_CHAR
                                     || (meStrchr(buf,_CONVDIR_CHAR) != NULL)
#endif
                                     ))
            {
                fileNameCorrect(buf,storeBuf,NULL);
                defaultStr = storeBuf;
            }
            else
                defaultStr = buf;
            addHistory(option,defaultStr,meFALSE);
        }
    }
    
    
    frameCur->mlStatus = MLSTATUS_CLEAR;
#if MEOPT_LOCALBIND
    useMlBinds = oldUseMlBinds;
#endif
    if(strList != NULL)
    {
        if(option & MLVARBL)
            freeFileList(noStrs,strList);
        else if(option & (MLBUFFER|MLCOMMAND))
            meFree(strList);
    }
    if(mlgsOldCwp != NULL)
    {
        meBuffer *cbp=frameCur->windowCur->buffer;
        if(mlgsOldWBp == NULL)
        {
            /* was no replacement so the window was split */
            windowDelete(meFALSE,1);
            frameCur->windowCur->vertScroll = mlgsSingWind-1;
        }
        else
        {
            swbuffer(frameCur->windowCur,mlgsOldWBp);
            meWindowMakeCurrent(mlgsOldCwp);
        }
        zotbuf(cbp,1);
    }
    /* MLSTATUS_CLEAR above dropped MLSTATUS_CURSHOW, update once the current
     * window and buffer have been restored */
    meCursorUpdate();
    
    if(cont_flag & 0x04)
        return ctrlg(meFALSE,1);

    return meTRUE;
}

