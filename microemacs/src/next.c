/* -*- C -*- ****************************************************************
 *
 *  System        : MicroEmacs Jasspa Distribution
 *  Module        : next.c
 *  Synopsis      : Next file/line goto routines
 *  Created By    : Steven Phillips
 *  Created       : 01/06/1994
 *  Last Modified : <000627.1024>
 *
 *  Description
 *
 *  Notes
 * 
 ****************************************************************************
 * 
 * Copyright (c) 1994-2000 Steven Phillips    
 *    
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the  authors be held liable for any damages  arising  from
 * the use of this software.
 *     
 * This software was generated as part of the MicroEmacs JASSPA  distribution,
 * (http://www.jasspa.com) but is excluded from those licensing restrictions.
 *
 * Permission  is  granted  to anyone to use this  software  for any  purpose,
 * including  commercial  applications,  and to alter it and  redistribute  it
 * freely, subject to the following restrictions:
 *
 *  1. The origin of this software must not be misrepresented; you must not
 *     claim that you wrote the original software. If you use this software
 *     in a product, an acknowledgment in the product documentation would be
 *     appreciated but is not required.
 *  2. Altered source versions must be plainly marked as such, and must not be
 *     misrepresented as being the original software.
 *  3. This notice may not be removed or altered from any source distribution.
 *
 * Steven Phillips         bill@jasspa.com
 *
 ****************************************************************************/

/*---	Include defintions */

#define	__NEXTC	1			/* Define the filename */

/*---	Include files */

#include "emain.h"
#include "esearch.h"

#if FLNEXT

static int
flNextFind(uint8 *str, uint8 **curFilePtr, int32 *curLine)
{
    uint8   pat[MAXBUF] ;
    uint8  *n=NULL ;
    int     fileRpt=-1, lineRpt=-1, onRpt=0 ;
    int     soff, len ;

    soff = 0 ;
    while((soff < NPAT) && (*str != '\0'))
    {
        if(*str == '%')
        {
            str++ ;
            switch(*str)
            {
            case 'f':
                fileRpt = onRpt++ ;
                n = flNextFileTemp ;
                break ;
            case 'l':
                lineRpt = onRpt++ ;
                n = flNextLineTemp ;
                break ;
            }
        }
        if(n != NULL)
        {
            pat[soff++] = '\\' ;
            pat[soff++] = '(' ;
            meStrcpy(pat+soff,n) ;
            soff += meStrlen(n) ;
            pat[soff++] = '\\' ;
            pat[soff++] = ')' ;
            str++ ;
            n = NULL ;
        }
        else
            pat[soff++] = *str++ ;
    }
    if(soff >= NPAT)
        return mlwrite(MWABORT,(uint8 *)"[Search string to long]") ;
    pat[soff] = '\0' ;

    if(iscanner(pat,1,ISCANNER_PTBEG|ISCANNER_MAGIC|ISCANNER_EXACT,NULL) != TRUE)
        return FALSE;

    /*
     * Found it, so fill in the slots.
     */

    soff = curwp->w_doto ;
    if(fileRpt >= 0)
    {
        len = mereRegexGroupEnd(fileRpt+1) - mereRegexGroupStart(fileRpt+1) ;
        meNullFree(*curFilePtr) ;
        if((*curFilePtr = meMalloc(len+1)) == NULL)
            return FALSE ;
        meStrncpy(*curFilePtr,curwp->w_dotp->l_text+soff+mereRegexGroupStart(fileRpt+1),len) ;
        (*curFilePtr)[len] = '\0' ;
    }
    if(lineRpt >= 0)
        *curLine = meAtoi(curwp->w_dotp->l_text+soff+mereRegexGroupStart(lineRpt+1)) ;

    return TRUE ;
}

int
getNextLine(int f,int n)
{
    uint8   noNextLines, ii, no ;
    uint8 **nextLines ;
    uint8  *newName=NULL ;
    int32   curLine=-1 ;
    BUFFER *bp=NULL, *tbp ;
    
    for(ii=0 ; ii<noNextLine ; ii++)
    {
        if((tbp = bfind(nextName[ii],0)) != NULL)
        {
            if(tbp == curbp)
            {
                bp = tbp ;
                no = ii ;
                break ;
            }
            if((bp == NULL) || (bp->b_nwnd < tbp->b_nwnd))
            {
                bp = tbp ;
                no = ii ;
            }
        }
    }
    if(bp == NULL)
        return mlwrite(MWABORT,(uint8 *)"[No next buffer found]") ;
    if((noNextLines = nextLineCnt[no]) == 0)
        return mlwrite(MWABORT,(uint8 *)"[No lines for next buffer %s]",bp->b_bname) ;
    nextLines = nextLineStr[no] ;
    
    wpopup(bp->b_bname,WPOP_MKCURR) ;

    if((curwp->w_dotp == bp->b_linep) ||
       ((curwp->w_dotp == lback(bp->b_linep)) &&
        (curwp->w_doto == llength(curwp->w_dotp))))
    {
        curwp->w_dotp = lforw(bp->b_linep) ;
        curwp->w_doto = 0 ;
        curwp->line_no = 0 ;
    }
    while((curwp->line_no)++,
          (curwp->w_dotp = lforw(curwp->w_dotp)) != bp->b_linep)
    {
        for(ii=0 ; ii<noNextLines ; ii++)
        {
            curwp->w_doto = 0 ;
            if(flNextFind(nextLines[ii],&newName,&curLine) == TRUE)
            {
                if(newName != NULL)
                {
                    uint8 fname[FILEBUF] ;
                    
                    meNullFree(bp->nextFile) ;
                    if(bp->b_fname != NULL)
                    {
                        uint8 tmp[FILEBUF] ;
                        getFilePath(bp->b_fname,tmp) ;
                        meStrcat(tmp,newName) ;
                        fileNameCorrect(tmp,fname,NULL) ;
                    }
                    else
                        fileNameCorrect(newName,fname,NULL) ;
                    bp->nextFile = meStrdup(fname) ;
                    meFree(newName) ;
                }
                curwp->w_doto = 0 ;
                curwp->w_flag |= WFMOVEL ;
                if(bp->nextFile == NULL)
                    return mlwrite(MWABORT,(uint8 *)"[No File name]") ;
                if(wpopup(NULL,WPOP_MKCURR) == NULL)
                    return FALSE ;
                if(findSwapFileList(bp->nextFile,(BFND_CREAT|BFND_MKNAM),0) != TRUE)
                    return FALSE ;
                if(curLine < 0)
                    return mlwrite(MWABORT,(uint8 *)"[No Line number]") ;
                /* if for some strange reason the file wasn't found, but the directory
                 * was and its read only, the findSwapFileList will succeed (new buffer)
                 * BUT the file name will be null! catch this and just use the buffer name */ 
                mlwrite(0,(uint8 *)"File %s, line %d",
                        (curbp->b_fname == NULL) ? curbp->b_bname:curbp->b_fname,curLine) ;
#if NARROW
                return gotoAbsLine(curLine) ;
#else
                return gotoLine(curLine) ;
#endif
            }
        }
    }
    curwp->w_doto = 0 ;
    curwp->w_flag |= WFMOVEL ;
#if 0
    /* swp 24/12/97 - I don't think this is needed */
    addModeToWindows(WFMODE) ;  /* and update ALL mode lines */
#endif
    return mlwrite(MWABORT,(uint8 *)"[No more lines found]") ;
}

int
addNextLine(int f, int n)
{
    uint8 name[MAXBUF], line[MAXBUF] ;
    int no, cnt ;
    
    if((mlreply((uint8 *)"next name",0,0,name,MAXBUF) != TRUE) ||
       (mlreply((uint8 *)"next line",0,0,line,MAXBUF) != TRUE))
        return FALSE ;
    for(no=0 ; no<noNextLine ; no++)
        if(!meStrcmp(nextName[no],name))
            break ;
    if(no == noNextLine)
    {
        noNextLine++ ;
        if(((nextName=meRealloc(nextName,noNextLine*sizeof(uint8 *))) == NULL) ||
           ((nextName[no]=meStrdup(name)) == NULL) ||
           ((nextLineCnt=meRealloc(nextLineCnt,noNextLine*sizeof(uint8))) == NULL) ||
           ((nextLineStr=meRealloc(nextLineStr,noNextLine*sizeof(uint8 **))) == NULL))
        {
            noNextLine = 0 ;
            return FALSE ;
        }
        nextLineCnt[no] = 0 ;
        nextLineStr[no] = NULL ;
    }
    cnt = nextLineCnt[no] ;
    if(line[0] == '\0')
    {
        while(cnt--)
            meFree(nextLineStr[no][cnt]) ;
        nextLineCnt[no] = 0 ;
    }
    else
    {
        if(((nextLineStr[no]=meRealloc(nextLineStr[no],(cnt+1)*sizeof(char *))) == NULL) ||
           ((nextLineStr[no][cnt]=meStrdup(line)) == NULL))
        {
            noNextLine = 0 ;
            return FALSE ;
        }
        nextLineCnt[no]++ ;
    }
    return TRUE ;
}

#endif

int
addFileHook(int f, int n)
{
    if(n == 0)
    {
        /* arg of 0 frees all hooks off */
        if(fileHookCount)
        {
            n = fileHookCount ;
            while(--n >= 0)
            {
                free(fileHookExt[n]) ;
                free(fileHookFunc[n]) ;
            }
            free(fileHookExt) ;
            free(fileHookFunc) ;
            free(fileHookArg) ;
            fileHookCount=0 ;
            fileHookExt=NULL ;
            fileHookFunc=NULL ;
            fileHookArg=NULL ;
        }
    }
    else
    {
        uint8 buff[MAXBUF] ;
    
        if(((fileHookExt = meRealloc(fileHookExt,(fileHookCount+1)*sizeof(char *))) == NULL) ||
           ((fileHookFunc = meRealloc(fileHookFunc,(fileHookCount+1)*sizeof(char *))) == NULL) ||
           ((fileHookArg = meRealloc(fileHookArg,(fileHookCount+1)*sizeof(short))) == NULL))
        {
            fileHookCount=0 ;
            return FALSE ;
        }
        if((mlreply((uint8 *)"File id",0,0,buff,MAXBUF) != TRUE) ||
           ((fileHookExt[fileHookCount] = meStrdup(buff)) == NULL) ||
           (mlreply((uint8 *)"Hook func",0,0,buff,MAXBUF) != TRUE) ||
           ((fileHookFunc[fileHookCount] = meStrdup(buff)) == NULL))
            return FALSE ;
        if(f == FALSE)
            fileHookArg[fileHookCount] = 0 ;
        else
            fileHookArg[fileHookCount] = n ;
        fileHookCount++ ;
    }
    return TRUE ;
}



#if DORCS

int
rcsFilePresent(uint8 *fname)
{
    uint8 tname[MAXBUF] ;
    register uint8 *fn, *ld, *tn=tname ;

    if(rcsFile == NULL)
        return FALSE ;

    fn = fname ;
    if((ld = meStrrchr(fname,DIR_CHAR)) != NULL)
    {
        do
            *tn++ = *fn ;
        while(fn++ != ld) ;
    }
    ld = rcsFile ;
    while(*ld != '\0')
    {
        if((*ld == '%') &&
           (*++ld == 'f'))
        {
            meStrcpy(tn,fn) ;
            tn += meStrlen(fn) ;
        }
        else
            *tn++ = *ld ;
        ld++ ;
    }
    *tn = '\0' ;
    return !meTestExist(tname) ;
}

int
doRcsCommand(uint8 *fname, register uint8 *comStr)
{
    uint8  pat[MAXBUF+1], cc ;
    uint8  path[FILEBUF] ;
    uint8 *bname ;
    register int ii, len ;
    
    fileNameCorrect(fname,path,&bname) ;
    ii = 0 ;
    while((cc=*comStr++) != '\0')
    {
        if((cc == '%') &&
           (((cc=*comStr++) == '\0') || (cc == 'f') || (cc == 'm')))
        {
			if(cc == '\0')
				break ;
            if(cc == 'f')
            {
                len = meStrlen(bname) ;
                if((ii + len) > MAXBUF)
                    break ;
                meStrcpy(pat+ii,bname) ;
                ii += len ;
            }
            else
            {
                if(mlreply((uint8 *)"Enter message",0,0,pat+ii,MAXBUF-ii) != TRUE)
                    return FALSE ;
                ii = meStrlen(pat) ;
            }
        }
        else
        {
            if(ii >= MAXBUF)
                break ;
            pat[ii++] = cc ;
        }
    }
    if(cc != '\0')
        return mlwrite(MWABORT,(uint8 *)"[command to long]") ;
    pat[ii] = '\0' ;
    *bname = '\0' ;
    /* must do a pipe not an ipipe as its sequential */
    ii = doPipeCommand(pat,path,(uint8 *)"*rcs*",LAUNCH_SILENT) ;

    return ii ;
}

int
rcsCiCoFile(int f, int n)
{
    register uint8 *str ;
    register int lineno, curcol, ss ;

    if(curbp->b_fname == NULL)
        return mlwrite(MWABORT,(uint8 *)"No file name!") ;
    ss = rcsFilePresent(curbp->b_fname) ;
    if(meModeTest(curbp->b_mode,MDVIEW))	/* if read-only then unlock */
    {
        if(n < 0)
            /* already read-only, no changes to undo, return */
            return TRUE ;
            
        if(ss != TRUE)
        {
#ifdef _DOS
            curbp->stats.stmode &= ~0x01 ;
#endif
#ifdef _WIN32
            curbp->stats.stmode &= ~FILE_ATTRIBUTE_READONLY;/* Write permission for owner */
#endif
#ifdef _UNIX
            curbp->stats.stmode |= 00200 ;
#endif
            meModeClear(curbp->b_mode,MDVIEW) ;
            curwp->w_flag |= WFMODE ;
            return TRUE ;
        }
        if((str = rcsCoUStr) == NULL)
            return mlwrite(MWABORT,(uint8 *)"[rcs cou command not set]") ;
    }
    else
    {
        if(n < 0)
        {
            /* unedit changes */
            if(ss != TRUE)
                return mlwrite(MWABORT,(uint8 *)"[rcs file not found - cannot unedit]") ;
            str = rcsUeStr ;
        }
        else
        {
            if(ss == TRUE)
                str = rcsCiStr ;
            else
                str = rcsCiFStr ;
        }
        if(str == NULL)
            return mlwrite(MWABORT,(uint8 *)"[rcs ci or cif command not set]") ;
        if((n >= 0) && meModeTest(curbp->b_mode,MDEDIT) &&
           ((ss=saveBuffer(TRUE,TRUE)) != TRUE))
            return ss ;
    }
    lineno = curwp->line_no ;
    curcol = curwp->w_doto ;
    /* must execute the command and then reload, reload taken from swbuffer */
    if((doRcsCommand(curbp->b_fname,str) != TRUE) ||
       (bclear(curbp) != TRUE) ||
       ((curbp->intFlag |= BIFFILE),(curbp->line_no = lineno+1),
        (swbuffer(curwp,curbp) != TRUE)))
        return FALSE ;
    if(curcol > llength(curwp->w_dotp))
        curwp->w_doto = llength(curwp->w_dotp) ;
    else
        curwp->w_doto = curcol ;

    return TRUE ;
}

#endif
