/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * next.c - Next file/line goto routines.
 *
 * Copyright (C) 1994-2001 Steven Phillips
 * Copyright (C) 2002-2024 JASSPA (www.jasspa.com)
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
 * Created:     01/06/1994
 * Synopsis:    Next file/line goto routines.
 * Authors:     Steven Phillips
 */

/*---	Include defintions */

#define	__NEXTC	1			/* Define the filename */

/*---	Include files */

#include "emain.h"
#include "esearch.h"

#if MEOPT_FILENEXT

static int
flNextFind(meUByte *str, meUByte **curFilePtr, meInt *curLine)
{
    meUByte pat[meBUF_SIZE_MAX], *ss, cc, *n=NULL;
    int buffRpt=-1, fileRpt=-1, lineRpt=-1;
    int soff, len;
    
    ss = str+1;
    soff = 0;
    while((cc=*ss++) != '\0')
    {
        if(cc == '%')
        {
            switch((cc=*ss++))
            {
            case 'b':
                buffRpt = soff;
                n = (meUByte *) ".*";
                break;
            case 'f':
                fileRpt = soff;
                n = flNextFileTemp;
                break;
            case 'l':
                lineRpt = soff;
                n = flNextLineTemp;
                break;
            }
        }
        if(n != NULL)
        {
            pat[soff++] = '\\';
            pat[soff++] = '(';
            meStrcpy(pat+soff,n);
            soff += meStrlen(n);
            pat[soff++] = '\\';
            pat[soff++] = ')';
            n = NULL;
        }
        else
            pat[soff++] = cc;
        if(soff > (meBUF_SIZE_MAX-128))
            return mlwrite(MWABORT,(meUByte *)"[Search string to long]");
    }
    pat[soff] = '\0';

    if(iscanner(pat,1,ISCANNER_PTBEG|ISCANNER_MAGIC|ISCANNER_EXACT,NULL) <= 0)
        return 0;

    /* Found it, is it an ignore line? if so return 2 else fill in the slots and return 1. */
    if(*str == '0')
        return 2;
    
    soff = frameCur->windowCur->dotOffset;
    if(fileRpt >= 0)
    {
        len = mereRegexGroupNo();
        do {
            if(mereRegexGroupRegexStart(len) == fileRpt)
                break;
        } while(--len);
        if(len == 0)
            return mlwrite(MWABORT,(meUByte *)"[Failed to locate file group]");
        fileRpt = mereRegexGroupStart(len);
        len = mereRegexGroupEnd(len) - fileRpt;
        meNullFree(*curFilePtr);
        if((*curFilePtr = meMalloc(len+1)) == NULL)
            return meFALSE;
        memcpy(*curFilePtr,frameCur->windowCur->dotLine->text+soff+fileRpt,len);
        (*curFilePtr)[len] = '\0';
    }
    else if(buffRpt >= 0)
    {
        len = mereRegexGroupNo();
        do {
            if(mereRegexGroupRegexStart(len) == buffRpt)
                break;
        } while(--len);
        if(len == 0)
            return mlwrite(MWABORT,(meUByte *)"[Failed to locate buffer group]");
        buffRpt = mereRegexGroupStart(len);
        len = mereRegexGroupEnd(len) - buffRpt;
        meNullFree(*curFilePtr);
        if((*curFilePtr = meMalloc(len+3)) == NULL)
            return meFALSE;
        (*curFilePtr)[0] = '*';
        memcpy((*curFilePtr)+1,frameCur->windowCur->dotLine->text+soff+buffRpt,len);
        (*curFilePtr)[len+1] = '*';
        (*curFilePtr)[len+2] = '\0';
    }
    if(lineRpt >= 0)
    {
        len = mereRegexGroupNo();
        do {
            if(mereRegexGroupRegexStart(len) == lineRpt)
                break;
        } while(--len);
        if(len == 0)
            return mlwrite(MWABORT,(meUByte *)"[Failed to locate line group]");
        *curLine = meAtoi(frameCur->windowCur->dotLine->text+soff+mereRegexGroupStart(len));
    }
    return 1;
}

int
getNextLine(int f,int n)
{
    int     noNextLines, ii, ll, no, rr;
    meUByte **nextLines;
    meUByte  *nextFile=NULL;
    meInt   curLine=-1;
    meBuffer *bp=NULL, *tbp;
    meWindow *cwp=frameCur->windowCur;
    
    tbp = cwp->buffer;
    ii = noNextLine;
    while(--ii >= 0)
    {
        ll = meStrlen(nextName[ii]);
#ifdef _INSENSE_CASE
        if(meStrnicmp(tbp->name,nextName[ii],ll) == 0)
#else
        if(meStrncmp(tbp->name,nextName[ii],ll) == 0)
#endif
        {
            bp = tbp;
            no = ii;
            break;
        }
    }
    if(bp == NULL)
    {    
        if((n & 0x02) == 0)
        {
            ii = noNextLine;
            while(--ii >= 0)
            {
                ll = meStrlen(nextName[ii]);
                tbp = bheadp;
                while(tbp != NULL)
                {
#ifdef _INSENSE_CASE
                    if((rr=meStrnicmp(tbp->name,nextName[ii],ll)) == 0)
#else
                    if((rr=meStrncmp(tbp->name,nextName[ii],ll)) == 0)
#endif
                    {
                        if((bp == NULL) || (bp->windowCount < tbp->windowCount) ||
                           ((bp->windowCount == tbp->windowCount) && (bp->histNo < tbp->histNo)))
                        {
                            bp = tbp;
                            no = ii;
                        }
                    }
                    else if(rr > 0)
                        break;
                    tbp = tbp->next;
                }
            }
        }
        if(bp == NULL)
            return mlwrite(MWABORT|MWCLEXEC,(meUByte *)"[No next buffer found]");
        if(n & 0x01)
            cwp = meWindowPopup(bp,NULL,WPOP_MKCURR,NULL);
        else if(swbuffer(cwp,bp) <= 0)
            return meFALSE;
    }
    if(n & 0x08)
        return meTRUE;
    if((noNextLines = nextLineCnt[no]) == 0)
        return mlwrite(MWABORT|MWCLEXEC,(meUByte *)"[No lines for next buffer %s]",bp->name);
    nextLines = nextLineStr[no];

    if((cwp->dotLine == bp->baseLine) ||
       ((cwp->dotLine == meLineGetPrev(bp->baseLine)) &&
        (cwp->dotOffset == meLineGetLength(cwp->dotLine))))
    {
        cwp->dotLine = meLineGetNext(bp->baseLine);
        cwp->dotOffset = 0;
        cwp->dotLineNo = 0;
    }
    while((cwp->dotLineNo)++,
          (cwp->dotLine = meLineGetNext(cwp->dotLine)) != bp->baseLine)
    {
        ii = noNextLines;
        while(--ii >= 0)
        {
            cwp->dotOffset = 0;
            if((rr=flNextFind(nextLines[ii],&nextFile,&curLine)) == meABORT)
            {
                cwp->dotOffset = 0;
                cwp->updateFlags |= WFMOVEL;
                return meABORT;
            }
            if(rr == 2)
                /* ignore the line */
                ii = 0;
            else if(rr == 1)
            {
                /* matched a line */
                cwp->dotOffset = 0;
                cwp->updateFlags |= WFMOVEL;
                if(nextFile != NULL)
                {
                    meUByte fname[meBUF_SIZE_MAX], *value;
                    meVariable *var;
                    
                    ii = meStrlen(nextFile);
                    if((nextFile[0] != '*') || (nextFile[ii-1] != '*'))
                    {
                        if(bp->fileName != NULL)
                        {
                            meUByte tmp[meBUF_SIZE_MAX];
                            getFilePath(bp->fileName,tmp);
                            meStrcat(tmp,nextFile);
                            fileNameCorrect(tmp,fname,NULL);
                        }
                        else
                            fileNameCorrect(nextFile,fname,NULL);
                        value = fname;
                    }
                    else
                        value = nextFile;
                    
                    var = SetUsrLclCmdVar((meUByte *)"next-file",value,&(bp->varList));
                    meFree(nextFile);
                    if(var == NULL)
                        return meFALSE;
                    nextFile = var->value;
                }
                else if((nextFile = getUsrLclCmdVar((meUByte *)"next-file",bp->varList)) == errorm)
                    return mlwrite(MWABORT,(meUByte *)"[No File name]");
                else
                    ii = meStrlen(nextFile);
                SetUsrLclCmdVar((meUByte *)"next-line",meItoa(curLine),&(bp->varList));
                
                if(n & 0x04)
                    return meTRUE;
                    
                if((n & 0x01) && ((cwp=meWindowPopup(NULL,NULL,WPOP_MKCURR,NULL)) == NULL))
                    return meFALSE;
                if((nextFile[0] == '*') && (nextFile[ii-1] == '*'))
                {
                    meBuffer *bp;
                    nextFile[ii-1] = '\0';
                    if((bp = bfind(nextFile+1,0)) == NULL)
                    {
                        mlwrite(MWABORT,(meUByte *)"[Buffer \"%s\" no longer exists]",nextFile+1);
                        nextFile[ii-1] = '*';
                        return meFALSE;
                    }
                    nextFile[ii-1] = '*';
                    if(swbuffer(cwp,bp) <= 0)
                        return meFALSE;
                }
                else if(findSwapFileList(nextFile,(BFND_CREAT|BFND_MKNAM),0,0) <= 0)
                    return meFALSE;
                if(curLine <= 0)
                    return mlwrite(MWABORT,(meUByte *)"[No line number]");
                /* if for some strange reason the file wasn't found, but the directory
                 * was and its read only, the findSwapFileList will succeed (new buffer)
                 * BUT the file name will be null! catch this and just use the buffer name */ 
                mlwrite(0,(meUByte *)"File %s, line %d",
                        (cwp->buffer->fileName == NULL) ? cwp->buffer->name:cwp->buffer->fileName,curLine);
#if MEOPT_NARROW
                if(cwp->buffer->narrow != NULL)
                    return meWindowGotoAbsLine(cwp,curLine);
#endif
                return meWindowGotoLine(cwp,curLine);
            }
        }
    }
    cwp->dotOffset = 0;
    cwp->updateFlags |= WFMOVEL;
    return mlwrite(MWABORT|MWCLEXEC,(meUByte *)"[No more lines found]");
}

int
addNextLine(int f, int n)
{
    meUByte name[meBUF_SIZE_MAX], line[meBUF_SIZE_MAX];
    int no, cnt;
    
    if(meGetString((meUByte *)"next name",0,0,name,meBUF_SIZE_MAX) <= 0)
        return meFALSE;
    for(no=0 ; no<noNextLine ; no++)
        if(!meStrcmp(nextName[no],name))
            break;
    if(n == 0)
    {
        if(no != noNextLine)
        {
            cnt = nextLineCnt[no];
            while(cnt--)
                meFree(nextLineStr[no][cnt]);
            nextLineCnt[no] = 0;
        }
        return meTRUE;
    }
    line[0] = (n < 0) ? '0':'1';
    if(meGetString((meUByte *)"next line",0,0,line+1,meBUF_SIZE_MAX) <= 0)
        return meFALSE;
    if(no == noNextLine)
    {
        noNextLine++;
        if(((nextName=meRealloc(nextName,noNextLine*sizeof(meUByte *))) == NULL) ||
           ((nextName[no]=meStrdup(name)) == NULL) ||
           ((nextLineCnt=meRealloc(nextLineCnt,noNextLine*sizeof(meUByte))) == NULL) ||
           ((nextLineStr=meRealloc(nextLineStr,noNextLine*sizeof(meUByte **))) == NULL))
        {
            noNextLine = 0;
            return meFALSE;
        }
        nextLineCnt[no] = 0;
        nextLineStr[no] = NULL;
    }
    cnt = nextLineCnt[no];

    if(((nextLineStr[no]=meRealloc(nextLineStr[no],(cnt+1)*sizeof(char *))) == NULL) ||
       ((nextLineStr[no][cnt]=meStrdup(line)) == NULL))
    {
        nextLineCnt[no] = 0;
        return meFALSE;
    }
    nextLineCnt[no]++;
    return meTRUE;
}

#endif

#if MEOPT_RCS

int
rcsFilePresent(meUByte *fname)
{
    meUByte tname[meBUF_SIZE_MAX];
    register meUByte *fn, *ld, *tn=tname;

    if(rcsFile == NULL)
        return meFALSE;

    fn = fname;
    if((ld = meStrrchr(fname,DIR_CHAR)) != NULL)
    {
        do
            *tn++ = *fn;
        while(fn++ != ld);
    }
    ld = rcsFile;
    while(*ld != '\0')
    {
        if((*ld == '%') &&
           (*++ld == 'f'))
        {
            meStrcpy(tn,fn);
            tn += meStrlen(fn);
        }
        else
            *tn++ = *ld;
        ld++;
    }
    *tn = '\0';
    return !meTestExist(tname);
}

int
doRcsCommand(meUByte *fname, register meUByte *comStr)
{
    meUByte  pat[meBUF_SIZE_MAX+1], cc;
    meUByte  path[meBUF_SIZE_MAX];
    meUByte *bname;
    register int ii, len;
    
    fileNameCorrect(fname,path,&bname);
    ii = 0;
    while((cc=*comStr++) != '\0')
    {
        if((cc == '%') &&
           (((cc=*comStr++) == '\0') || (cc == 'f') || (cc == 'm')))
        {
			if(cc == '\0')
				break;
            if(cc == 'f')
            {
                len = meStrlen(bname);
                if((ii + len) > meBUF_SIZE_MAX)
                    break;
                meStrcpy(pat+ii,bname);
                ii += len;
            }
            else
            {
                if(meGetString((meUByte *)"Enter message",0,0,pat+ii,meBUF_SIZE_MAX-ii) <= 0)
                    return meFALSE;
                ii = meStrlen(pat);
            }
        }
        else
        {
            if(ii >= meBUF_SIZE_MAX)
                break;
            pat[ii++] = cc;
        }
    }
    if(cc != '\0')
        return mlwrite(MWABORT,(meUByte *)"[command to long]");
    pat[ii] = '\0';
    *bname = '\0';
    /* must do a pipe not an ipipe as its sequential */
    ii = doPipeCommand(pat,path,(meUByte *)"*rcs*",-1,LAUNCH_SILENT,NULL);

    return ii;
}

int
rcsCiCoFile(int f, int n)
{
    meWindow *cwp=frameCur->windowCur;
    meBuffer *cbp=cwp->buffer;
    meUByte *str;
    int lineno, curcol, ss;

    if(cbp->fileName == NULL)
        return mlwrite(MWABORT,(meUByte *)"No file name!");
    ss = rcsFilePresent(cbp->fileName);
    if(meModeTest(cbp->mode,MDVIEW))	/* if read-only then unlock */
    {
        if(n < 0)
            /* already read-only, no changes to undo, return */
            return meTRUE;
            
        if(ss <= 0)
        {
#ifdef _DOS
            cbp->stats.stmode &= ~0x01;
#endif
#ifdef _WIN32
            cbp->stats.stmode &= ~FILE_ATTRIBUTE_READONLY;/* Write permission for owner */
#endif
#ifdef _UNIX
            cbp->stats.stmode |= 00200;
#endif
            meModeClear(cbp->mode,MDVIEW);
            cwp->updateFlags |= WFMODE;
            return meTRUE;
        }
        if((str = rcsCoUStr) == NULL)
            return mlwrite(MWABORT,(meUByte *)"[rcs cou command not set]");
    }
    else
    {
        if(n < 0)
        {
            /* unedit changes */
            if(ss <= 0)
                return mlwrite(MWABORT,(meUByte *)"[rcs file not found - cannot unedit]");
            str = rcsUeStr;
        }
        else
        {
            if(ss > 0)
                str = rcsCiStr;
            else
                str = rcsCiFStr;
        }
        if(str == NULL)
            return mlwrite(MWABORT,(meUByte *)"[rcs ci or cif command not set]");
        if((n >= 0) && meModeTest(cbp->mode,MDEDIT) &&
           ((ss=saveBuffer(meTRUE,meTRUE)) <= 0))
            return ss;
    }
    lineno = cwp->dotLineNo;
    curcol = cwp->dotOffset;
    /* must execute the command and then reload, reload taken from swbuffer */
    if((doRcsCommand(cbp->fileName,str) <= 0) ||
       (bclear(cbp) <= 0) ||
       ((cbp->intFlag |= BIFFILE),(cbp->dotLineNo = lineno+1),
        (swbuffer(cwp,cbp) <= 0)))
        return meFALSE;
    if(curcol > meLineGetLength(cwp->dotLine))
        cwp->dotOffset = meLineGetLength(cwp->dotLine);
    else
        cwp->dotOffset = curcol;

    return meTRUE;
}

#endif
