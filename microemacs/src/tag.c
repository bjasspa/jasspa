/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * tag.c - Find tag positions.
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
 * Created:     Unknown
 * Synopsis:    Find tag positions.
 * Authors:     Mike Rendell of ROOT Computers Ltd, Jon Green & Steven Phillips.
 * Description:
 *     Find tags from a tag file. Requires a 'tags' file listing all defined
 *     tgas in the form:
 *         tag<tab>file<tab>search-pattern
 */

#define	__TAGC			/* Define filename */

#include "emain.h"
#include "esearch.h"

#if MEOPT_TAGS

#define TAG_MIN_LINE 6
static meUByte *tagLastFile=NULL ;
static meInt  tagLastPos=-1 ;

/* findTagInFile
 * 
 * Search for a tag in the give tag filename
 * 
 * returns
 *    0 - Found tag
 *    1 - Found tag file but not tag
 *    2 - Found nothing
 */
static int
findTagInFile(meUByte *file, meUByte *baseName, meUByte *tagTemp,
              int tagOption, meUByte *key, meUByte *patt)
{
    FILE  *fp ;
    meUByte  function[meBUF_SIZE_MAX] ;
    meInt  opos, pos, start, end, found=-1;
    int    tmp ;
    
    if ((fp = fopen((char *)file, "rb")) == (FILE *) NULL)
        return 2 ;
    
    /* Read in the tags file */
    fseek(fp, 0L, 2);
    start = 0;
    end = ftell(fp) - TAG_MIN_LINE;
    pos = end >> 1;
    for(;;)
    {
        opos = pos ;
        fseek(fp, pos, 0);
        if(pos)
        {
            do
                pos++ ;
            while((pos <= end) && (fgetc(fp) != meCHAR_NL)) ;
        }
        if(pos > end)
        {
            if(opos == start)
                break ;
            end = opos ;
            pos = start ;
        }   
        else
        {
            /* Get line of info */
            fscanf(fp,(char *)tagTemp,function,baseName,patt) ;
            
            if (!(tmp = meStrcmp(key, function)))
            {
                /* found */
                if(((found = pos) == 0) || !(tagOption & 0x04))
                    break ;
                end = opos ;
            }
            else if (tmp > 0)
                /* forward */
                start = ftell(fp) - 1 ;
            else if((end=opos) == 0)
                break ;
            pos = ((start+end) >> 1) ;
        }
    }
    if((found >= 0) && (tagOption & 0x04))
    {
        tagLastPos = found ;
        fseek(fp, found, SEEK_SET);
        fscanf(fp,(char *)tagTemp,function,baseName,patt) ;
    }
    fclose(fp);
    return (found < 0) ;
}


/*
 * Find the function in the tag file.  From Mike Rendell.
 * Return the line of the tags file, and let do_tag handle it from here.
 * Note file must be an array meBUF_SIZE_MAX big
 */

static int
findTagSearch(meUByte *key, meUByte *file, meUByte *patt)
{
    meUByte *tagFile, *tagTemp, *baseName, *tt ;
    int ii, tagOption;
    
    /* Get the tag environment variables */
    if(((tagFile = getUsrVar((meUByte *)"tag-file")) == errorm) ||
       ((tagTemp = getUsrVar((meUByte *)"tag-template")) == errorm))
        return mlwrite(MWABORT,(meUByte *)"[tags not setup]");
    
    /* Process the "%tag-option" currently only the following options are supported:
     * 
     *   'r' == recursive ascent.
     *   'c' == continue (multi-tag processing)
     */
    if((tt = getUsrVar((meUByte *)"tag-option")) != errorm)
    {
        tagOption = (meStrchr(tt,'m') != NULL) ? 4:0;
        /* 'c' automatically adds 'r' */
        if(meStrchr(tt,'c') != NULL)
            tagOption |= 3;
        else if(meStrchr(tt,'r') != NULL)
            tagOption |= 1;
    }
    else
        tagOption = 7;
    
    /* Get the current directory location of our file and use this to locate
     * the tag file. */
    getFilePath(frameCur->windowCur->buffer->fileName,file) ;
    baseName = file + meStrlen(file) ;
    for(;;)
    {
        /* Construct the new tags file base */
        meStrcpy(baseName,tagFile) ;
        
        /* Attempt to find the tag in this tags file. */
        if((ii = findTagInFile(file,baseName,tagTemp,tagOption,key,patt)) == 0)
        {
            if(tagOption & 4)
            {
                if(tagLastFile == NULL)
                    tagLastFile = meMalloc(meBUF_SIZE_MAX) ;
                if(tagLastFile != NULL)
                {
                    ii = (size_t) baseName - (size_t) file ;
                    meStrncpy(tagLastFile,file,ii) ;
                    meStrcpy(tagLastFile+ii,tagFile) ;
                }
                else
                    tagLastPos = -1 ;
            }
            return meTRUE ;
        }
        if(ii == 1)
        {
            if(!(tagOption & 2))
            {
                meStrcpy(baseName,tagFile) ;
                /* no continue after fail to find it a tag file - return */
                return mlwrite(MWABORT|MWCLEXEC,(meUByte *)"[Can't find tag [%s] in tag file : %s]",
                               key, file);
            }
        }
        else if(!(tagOption & 1))
            /* if no recursion */
            return mlwrite(MWABORT|MWCLEXEC,(meUByte *)"[no tag file : %s]",file);
        
        /* continue search. Ascend the tree by geting the directory path
         * component of our current path position */
        if(baseName == file)
            break ;
        baseName[-1] = '\0';    /* Knock the trailing '/' */
        if((baseName=meStrrchr(file,DIR_CHAR)) == NULL)
            break ;
        baseName++ ;    /* New basename to try */
    }
    tagLastPos = -1 ;
    return mlwrite(MWABORT|MWCLEXEC,(meUByte *)"[Failed to find tag : %s]",key);
}

static int
findTagSearchNext(meUByte *key, meUByte *file, meUByte *patt)
{
    FILE *fp ;
    meUByte function[meBUF_SIZE_MAX], *baseName, *tagTemp ;
    meInt pos ;
    
    if((tagLastPos < 0) ||
       ((tagTemp = getUsrVar((meUByte *)"tag-template")) == errorm))
        return mlwrite(MWABORT,(meUByte *)"[No last tag]") ;
    
    if (((fp = fopen((char *)tagLastFile, "rb")) == NULL) ||
        (fseek(fp, tagLastPos, SEEK_SET) != 0))
        return mlwrite(MWABORT,(meUByte *)"[Failed to reload tag]") ;
    
    fscanf(fp,(char *)tagTemp,key,file,patt) ;
    
    meStrcpy(file,tagLastFile) ;
    if((baseName=meStrrchr(file,DIR_CHAR)) != NULL)
        baseName++ ;
    else
        baseName = file ;
    
    pos = ftell(fp) ;
    fscanf(fp,(char *)tagTemp,function,baseName,patt) ;
    fclose(fp) ;
    
    if (meStrcmp(key,function))
        /* no more found */
        return mlwrite(MWABORT,(meUByte *)"[No more \"%s\" tags]",key) ;
    tagLastPos = pos ;
    return meTRUE ;
}

static int
findTagExec(int nn, meUByte tag[])
{
    meUByte file[meBUF_SIZE_MAX];	/* File name */
    meUByte fpatt[meBUF_SIZE_MAX];	/* tag file pattern */
    meUByte mpatt[meBUF_SIZE_MAX];	/* magic pattern */
    int   flags ;
    
    if(nn & 0x04)
    {
        if(findTagSearchNext(tag,file,fpatt) <= 0)
            return meFALSE;
    }
    else if(findTagSearch(tag,file,fpatt) <= 0)
        return meFALSE;
    
    if((nn & 0x01) && (meWindowPopup(NULL,NULL,WPOP_MKCURR,NULL) == NULL))
        return meFALSE;
    if(findSwapFileList(file,(BFND_CREAT|BFND_MKNAM),0,0) <= 0)
        return mlwrite(MWABORT,(meUByte *)"[Can't find %s]",file);
    
    /* now convert the tag file search pattern into a magic search string */
    {
        meUByte cc, *ss, *dd, ee;
        ss = fpatt;
        
        /* if the first char is a '/' then search forwards, '?' for backwards */
        ee = *ss++ ;
        if(ee == '?')
        {
            flags = ISCANNER_BACKW|ISCANNER_PTBEG|ISCANNER_MAGIC|ISCANNER_EXACT;
            windowGotoEob(meTRUE,0);
        }            
        else
        {
            flags = ISCANNER_PTBEG|ISCANNER_MAGIC|ISCANNER_EXACT;
            windowGotoBob(meTRUE,0);
            if(ee != '/')
            {
                ee = '\0';
                ss--;
            }
        }
        if(ee != '\0')
        {
            /* look for the end '/' or '?' - start at the end and look backwards */
            dd = ss + meStrlen(ss);
            while(dd != ss)
                if(*--dd == ee)
                {
                    *dd = '\0';
                    break;
                }
        }
            
        dd = mpatt;
        if(*ss == '^')
        {
            *dd++ = '^';
            ss++;
        }
        
        while((cc=*ss++) != '\0')
        {
            if(cc == '\\')
            {
                *dd++ = '\\';
                *dd++ = *ss++;
            }
            else
            {
                if((cc == '[') || (cc == '*') || (cc == '+') ||
                   (cc == '.') || (cc == '?') || (cc == '$'))
                    *dd++ = '\\';
                *dd++ = cc;
            }
        }
        if(dd[-1] == '$')
        {
            dd[-2] = '$';
            dd[-1] = '\0';
        }
        else
            *dd = '\0';
    }
    
    if(iscanner(mpatt,0,flags,NULL) > 0)
        return meTRUE;
    
    windowGotoBob(meTRUE,0);
    iscanner(tag,0,ISCANNER_PTBEG|ISCANNER_EXACT,NULL);
    return mlwrite(MWABORT,(meUByte *)"[Can't find %s]",fpatt);
}

/*ARGSUSED*/

int
findTag(int f, int n)
{
    register meWindow *cwp=frameCur->windowCur;
    meUByte  tag[meBUF_SIZE_MAX], *ss;
    int      offs, len;
    
    /* Determine if we are in a word. If not then get a word from the user. */
    if(n & 0x04)
        ;
    else if((n & 0x02) || !meWindowInWord(cwp))
    {
	/*---	Get user word. */
        if((meGetString((meUByte *)"Enter Tag",MLNOSPACE,0,tag,meBUF_SIZE_MAX) <= 0) || (tag[0] == '\0'))
            return ctrlg(meFALSE,1);
    }
    else
    {
        ss = cwp->dotLine->text;
        offs = cwp->dotOffset;
        
	/* Go to the start of the word. */
        while((--offs >= 0) && isWord(ss[offs]))
            ;
        offs++;
        
	/* Get the word required. */
        len = 0;
        do
            tag[len++] = ss[offs++];
        while((len < meBUF_SIZE_MAX-1) && isWord(ss[offs]));
        tag[len] = 0;
    }
    
    /*---	Call the tag find. */
    return findTagExec(n,tag);
}

#endif
