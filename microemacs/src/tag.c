/*****************************************************************************
*
*	Title:		%M%
*
*	Synopsis:	Find tag positions
*
******************************************************************************
*
*	Filename:		%P%
*
*	Author:			Mike Rendell of ROOT Computers Ltd.
*
*	Creation Date:		10/05/91 08:27		<010305.0749>
*
*	Modification date:	%G% : %U%
*
*	Current rev:		%I%
*
*	Special Comments:	
*
*	Contents Description:	Find tags from a tag file. Only defined if
*				running some sort of UNIX.
*
*****************************************************************************
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

#define	__TAGC			/* Define filename */

/*---	Include files */

#include "emain.h"
#include "esearch.h"

#define TAG_MIN_LINE 6
static uint8 *tagLastFile=NULL ;
static int32  tagLastPos=-1 ;

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
findTagInFile(uint8 *file, uint8 *baseName, uint8 *tagTemp,
              int tagOption, uint8 *key, uint8 *patt)
{
    FILE  *fp ;
    uint8  function[MAXBUF] ;
    int32  opos, pos, start, end, found=-1;
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
            while((pos <= end) && (fgetc(fp) != meNLCHAR)) ;
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
            else if (tmp < 0)	/* back */
                end = opos;
            else			/* foward */
                start = ftell(fp) - 1 ;
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
 * Note file must be an array MAXBUF big
 */

static int
findTagSearch(uint8 *key, uint8 *file, uint8 *patt)
{
    uint8 *tagFile, *tagTemp, *baseName, *tt ;
    int ii, tagOption;
    
    /* Get the tag environment variables */
    if(((tagFile = getUsrVar((uint8 *)"tag-file")) == errorm) ||
       ((tagTemp = getUsrVar((uint8 *)"tag-template")) == errorm))
        return mlwrite(MWABORT,(uint8 *)"[tags not setup]");
    
    /* Process the "%tag-option" currently only the following options are supported:
     * 
     *   'r' == recursive ascent.
     *   'c' == continue (multi-tag processing)
     */
    tagOption = 0 ;
    if ((tt = getUsrVar((uint8 *)"tag-option")) != errorm)
    {
        /* 'c' automatically adds 'r' */
        if(meStrchr(tt,'c') != NULL)
            tagOption |= 3 ;
        else if(meStrchr(tt,'r') != NULL)
            tagOption |= 1 ;
        if(meStrchr(tt,'m') != NULL)
            tagOption |= 4 ;
    }
    
    /* Get the current directory location of our file and use this to locate
     * the tag file. */
    getFilePath(curbp->b_fname,file) ;
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
                    tagLastFile = meMalloc(FILEBUF) ;
                if(tagLastFile != NULL)
                {
                    ii = (size_t) baseName - (size_t) file ;
                    meStrncpy(tagLastFile,file,ii) ;
                    meStrcpy(tagLastFile+ii,tagFile) ;
                }
                else
                    tagLastPos = -1 ;
            }
            return TRUE ;
        }
        if(ii == 1)
        {
            if(!(tagOption & 2))
            {
                meStrcpy(baseName,tagFile) ;
                /* no continue after fail to find it a tag file - return */
                return mlwrite(MWABORT|MWCLEXEC,(uint8 *)"[Can't find tag [%s] in tag file : %s]",
                               key, file);
            }
        }
        else if(!(tagOption & 1))
            /* if no recursion */
            return mlwrite(MWABORT|MWCLEXEC,(uint8 *)"[no tag file : %s]",file);
        
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
    return mlwrite(MWABORT|MWCLEXEC,(uint8 *)"[Failed to find tag : %s]",key);
}

static int
findTagSearchNext(uint8 *key, uint8 *file, uint8 *patt)
{
    FILE *fp ;
    uint8 function[MAXBUF], *baseName, *tagTemp ;
    int32 pos ;
    
    if((tagLastPos < 0) ||
       ((tagTemp = getUsrVar((uint8 *)"tag-template")) == errorm))
        return mlwrite(MWABORT,(uint8 *)"[No last tag]") ;
    
    if (((fp = fopen((char *)tagLastFile, "rb")) == NULL) ||
        (fseek(fp, tagLastPos, SEEK_SET) != 0))
        return mlwrite(MWABORT,(uint8 *)"[Failed to reload tag]") ;
    
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
        return mlwrite(MWABORT,(uint8 *)"[No more \"%s\" tags]",key) ;
    tagLastPos = pos ;
    return TRUE ;
}

static	int
findTagExec(int nn, uint8 tag[])
{
    uint8 file[FILEBUF];	/* File name */
    uint8 fpatt[MAXBUF];	/* tag file pattern */
    uint8 mpatt[MAXBUF];	/* magic pattern */
    int   flags ;
    
    if(nn & 0x04)
    {
        if(findTagSearchNext(tag, file, fpatt) != TRUE)
            return FALSE ;
    }
    else if(findTagSearch(tag, file, fpatt) != TRUE)
        return FALSE ;
    
    if((nn & 0x01) && (wpopup(NULL,WPOP_MKCURR) == NULL))
        return FALSE ;
    if(findSwapFileList(file,(BFND_CREAT|BFND_MKNAM),0) != TRUE)
        return mlwrite(MWABORT,(uint8 *)"[Can't find %s]", file);
    
    /* now convert the tag file search pattern into a magic search string */
    {
        uint8 cc, *ss, *dd, ee ;
        ss = fpatt ;
        
        /* if the first char is a '/' then search forwards, '?' for backwards */
        ee = *ss++ ;
        if(ee == '?')
        {
            flags = ISCANNER_BACKW|ISCANNER_PTBEG|ISCANNER_MAGIC|ISCANNER_EXACT ;
            gotoeob(TRUE, 0);
        }            
        else
        {
            flags = ISCANNER_PTBEG|ISCANNER_MAGIC|ISCANNER_EXACT ;
            gotobob(TRUE, 0);
            if(ee != '/')
            {
                ee = '\0' ;
                ss-- ;
            }
        }
        if(ee != '\0')
        {
            /* look for the end '/' or '?' - start at the end and look backwards */
            dd = ss + meStrlen(ss) ;
            while(dd != ss)
                if(*--dd == ee)
                {
                    *dd = '\0' ;
                    break ;
                }
        }
            
        dd = mpatt ;
        if(*ss == '^')
        {
            *dd++ = '^' ;
            ss++ ;
        }
        
        while((cc=*ss++) != '\0')
        {
            if(cc == '\\')
            {
                *dd++ = '\\' ;
                *dd++ = *ss++ ;
            }
            else
            {
                if((cc == '[') || (cc == '*') || (cc == '+') ||
                   (cc == '.') || (cc == '?') || (cc == '$'))
                    *dd++ = '\\' ;
                *dd++ = cc ;
            }
        }
        if(dd[-1] == '$')
        {
            dd[-2] = '$' ;
            dd[-1] = '\0' ;
        }
        else
            *dd = '\0' ;
    }
    
    if(iscanner(mpatt,0,flags,NULL) != TRUE)
        return mlwrite(MWABORT,(uint8 *)"[Can't find %s]",fpatt) ;
    
    return TRUE ;
}

/*ARGSUSED*/

int
findTag(int f, int n)
{
    uint8  tag [MAXBUF];
    LINE  *startp;
    int16  starto, i;
    
    /* Determine if we are in a word. If not then get a word from the 
       user. */
    
    if(n & 0x04)
        ;
    else if((n & 0x02) || (inWord() == FALSE))
    {
	/*---	Get user word. */
        if((meGetString((uint8 *)"Enter Tag", MLNOSPACE, 0, tag,MAXBUF) != TRUE) || (tag[0] == '\0'))
            return ctrlg(FALSE,1) ;
    }
    else
    {
	/*---	Save the current cursor postion. */
        
        startp = curwp->w_dotp;		
        starto = curwp->w_doto;
        
	/*---	Go to the start of the word. */

        while(WbackChar(curwp,1) != FALSE)	/* Back up */
            if(inWord() == FALSE)
            {
                WforwChar(curwp, 1);		/* Back into the word */
                break ;
            }
        
	/*---	Get the word required. */
        
        i = 0;
        while (inWord() != FALSE)
        {
            tag[i++] = lgetc(curwp->w_dotp, curwp->w_doto);
            if (i >= MAXBUF-1)
            {
                i = 0;	/* Too big !! */
                break;	/* Break out */
            }
            WforwChar(curwp, 1);
        }
        tag[i] = 0;			/* End of string */
        
	/*---	Restore old cursor position, find tag type and find the string */
        
        curwp->w_dotp = startp;
        curwp->w_doto = starto;

    }	/* End of 'if' */
    
    /*---	Call the tag find. */

    return findTagExec(n,tag);

}


