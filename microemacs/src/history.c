/* -*- C -*- ****************************************************************
 *
 *  System        : MicroEmacs Jasspa Distribution
 *  Module        : history.c
 *  Synopsis      : ME histroy saving and re-loading routines
 *  Created By    : Steven Phillips
 *  Created       : 1995
 *  Last Modified : <000221.0749>
 *
 *  Description
 *     Saves the main registry configuration, with a history of currently
 *     loaded files (for use with -c option), last search string, buffer names
 *     etc.
 *
 *  Notes
 *     The history id string must be changed whenever some internals change,
 *     most notably the history file format and any change to the buffer
 *     modes (nasty side-effects with -c). This stops the use of wrong verson
 *     histories.
 *
 ****************************************************************************
 * 
 * Copyright (c) 1995-2000 Steven Phillips    
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

#define	__HISTORYC			/* Define the name of the file */

#include "emain.h"

static uint8 histVerId[] = "4" ;      /* History version */
uint8 *defHistFile=NULL ;

/* Constants */
static uint8 *regNames[5] = {
    (uint8 *)"string",(uint8 *)"buffer",(uint8 *)"command",(uint8 *)"file",(uint8 *)"search"
};
static uint8 *numList[5]=
{
    &numStrHist,
    &numBuffHist,
    &numCommHist,
    &numFileHist,
    &numSrchHist
};
    

void
initHistory(void)
{
    /* Must malloc the 20 history slots for the 5 history types */
    if((strHist = (uint8 **) meMalloc(sizeof(uint8 *) * 5 * MLHISTSIZE)) == NULL)
        meExit(1) ;
    /* Initialise the array to NULLS so we know they don't point to a history
     * string
     */
    memset(strHist,0,sizeof(uint8 *) * 5 * MLHISTSIZE) ;
    buffHist = strHist + MLHISTSIZE ;
    commHist = buffHist + MLHISTSIZE ;
    fileHist = commHist + MLHISTSIZE ;
    srchHist = fileHist + MLHISTSIZE ;
}

int
setupHistory(int option, uint8 **numPtr, uint8 ***list)
{
    if(option & MLBUFFER)
    {
        *numPtr = &numBuffHist ;
        *list = buffHist ;
    }
    else if(option & MLCOMMAND)
    {
        *numPtr = &numCommHist ;
        *list = commHist ;
    }
    else if(option & MLFILE)
    {
        *numPtr = &numFileHist ;
        *list = fileHist ;
    }
    else if(option & MLSEARCH)
    {
        *numPtr = &numSrchHist ;
        *list = srchHist ;
    }
    else
    {
        *numPtr = &numStrHist ;
        *list = strHist ;
    }
    return **numPtr ;
}


void
addHistory(int option, uint8 *str)
{
    uint8   *numPtr, numHist,i ;
    uint8  **history, *buf ;

    numHist = setupHistory(option, &numPtr, &history) ;

    if((numHist > 0) && !meStrcmp(str,history[0]))
        return ;
    if((buf = meMalloc(meStrlen(str)+1)) == NULL)
        return ;
    if(numHist == MLHISTSIZE)
        meFree(history[numHist-1]) ;
    else
    {
        numHist++ ;
        (*numPtr)++ ;
    }
    for(i=numHist-1 ; i>0 ; i--)
        history[i] = history[i-1] ;
    meStrcpy(buf,str) ;
    history[0] = buf ;
}

/*
 * Read in the history.
 */

int
readHistory(int f, int n)
{
    BUFFER *bp ;                        /* Buffer pointer */
    REGHANDLE regRoot;                  /* Root of the registry */
    REGHANDLE reg;                      /* temp registry pointer */
    uint8 filename[FILEBUF] ;	        /* Filename */
    uint8 *fname;	                /* Resulting file name to execute */
    int ii;                             /* Loop counter */
    
    if(n == 0)
    {
        if(defHistFile == NULL)
            return mlwrite(MWABORT|MWCLEXEC,(uint8 *)"[No default history file]") ;
        fname = defHistFile ;
    }
    else
    {
        if (mlreply((uint8 *)"Read history", MLFILECASE,0,filename,FILEBUF) != TRUE)
            return ABORT ;
        fname = filename ;
    }
    
    /* Write the strings into the registry */
    if ((regRoot = regRead((uint8 *)"history", fname, REGMODE_RELOAD|REGMODE_BACKUP)) == NULL)
        return ABORT ;
    
    if((f == TRUE) && (n != 0))
    {
        /* Store the default history file name */
        meNullFree(defHistFile) ;
        defHistFile = meStrdup(regRoot->value) ;
    }
    if ((regRoot = regFind(regRoot,(uint8 *)"history")) != NULL)
    {
        uint8 *ss;                       /* temp string pointer */
        
        /* Load in the history */
        for (ii = 0; ii < 5; ii++)
        {
            int numHistory = 0;
            
            if((reg = regFind (regRoot, regNames[ii])) != NULL)
               reg = regGetChild(reg) ;
            while (reg != NULL)
            {
                meNullFree(strHist[(ii*MLHISTSIZE)+numHistory]) ;
                if((strHist[(ii*MLHISTSIZE)+numHistory] = reg->value) != NULL)
                {
                    reg->value = NULL;
                    numHistory++;
                }
                reg = regGetNext(reg) ;
            }
            /* Assign the number of history items */
            *(numList[ii]) = numHistory;
            while(numHistory < MLHISTSIZE)
            {
                if(strHist[(ii*MLHISTSIZE)+numHistory] != NULL)
                {
                    free(strHist[(ii*MLHISTSIZE)+numHistory]) ;
                    strHist[(ii*MLHISTSIZE)+numHistory] = NULL ;
                }
                numHistory++ ;
            }
        }
        /*
         * Read in the buffer history provided that the history file state is 
         * defined correctly and the versions are not different
         */
        if(HistNoFilesLoaded < 0)
        {
            /* Get the version information */
            if (((reg = regFind (regRoot,(uint8 *)"version")) != NULL) &&
                ((ss=regGetString(reg,NULL)) != NULL) && !meStrcmp(ss,histVerId))
            {
                /* Set up to extract the 'active' configuration */
                /* Reset the files loaded counter */
                ii = 0;
                if((reg = regFind (regRoot,(uint8 *)"active")) != NULL)
                    reg = regGetChild(reg) ;
                while (reg != NULL)
                {
                    meSTAT stats ;
                    uint8  fnbuf[FILEBUF] ;
                    int    flags ;
                    
                    /* Get the filename */
                    /* If file is nasty, or it doesnt exist, ignore it */
                    /* NOTE: must correct the file name as duel systems such as window & dos
                     * have different naming conventions */
                    if(((fname = reg->value) != NULL) &&
                       ((flags = getFileStats(fname,0,&stats,NULL)) != meFILETYPE_NASTY) && (flags != meFILETYPE_NOTEXIST) &&
                       (fileNameCorrect(fname,fnbuf,NULL),((fname=meStrdup(fnbuf)) != NULL)))
                    {
                        int    histNo, lineNo, m0, m1, m2, m3, m4 ;
                        meMODE mode ;
                        
                        /* now extract the buffer info */
                        sscanf((char *)reg->name,"%d,%d,%d,%d,%d,%d,%d,%d",
                               &flags,&histNo,&lineNo,&m0,&m1,&m2,&m3,&m4) ;
                        mode[0] = (uint8) m0 ;
                        mode[1] = (uint8) m1 ;
                        mode[2] = (uint8) m2 ;
                        mode[3] = (uint8) m3 ;
                        mode[4] = (uint8) m4 ;
                        
                        /* set this to inactive */
                        bp = bfind(fname,(BFND_CREAT|BFND_MKNAM|
                                          (meModeTest(mode,MDBINRY) ? BFND_BINARY:0)));
                        bp->intFlag |= BIFFILE ;
                        bp->b_fname = fname ;
                        bp->line_no = lineNo ;
                        bp->histNo = histNo ;
                        
                        meModeCopy(bp->b_mode,mode) ;
                        meModeClear(bp->b_mode,MDEDIT) ;
                        meModeClear(bp->b_mode,MDNRRW) ;
                        meModeSet(bp->b_mode,MDNACT) ;
                        /* copy across the stat info - really important for unix
                         * to avoid the posibility of load the file in twice
                         */
                        memcpy(&(bp->stats),&stats,sizeof(meSTAT)) ;
                        bufHistNo = 2 ;
                        if(histNo)
                            ii++ ;
                    }
                    reg = regGetNext(reg) ;
                }
                /* Fix up the history */
                if(!ii && (bufHistNo == 2))
                    ii++ ;
                HistNoFilesLoaded = ii ;
            }
            else
                /* MUST reset the HistNoFilesLoaded value or a call to save-history
                 * followed by a read-history will really screw things up! */
                HistNoFilesLoaded = 0 ;
        }
        /* Delete the history from memory - we have finished with it */
        regDelete (regRoot);
        mlwrite(MWCLEXEC,(uint8 *)"[History loaded]") ;
    }
    return TRUE ;
}

/*
 * saveHistory
 * Save the history back to the registry and to file.
 */
int
saveHistory(int f, int n)
{
    int ii, jj;                         /* Local loop counters */
    BUFFER *bp ;                        /* Pointer to buffers */
    REGHANDLE regRoot;                  /* Root of the registry */
    REGHANDLE regHistory;               /* Handle into the registry */
    uint8 name[2] = "0";                /* Automatic name seed */
    uint8 filename[FILEBUF] ;           /* Filename */
    uint8 *fname;                       /* Resulting file name to execute */

    if(n == 0)
    {
        if((fname = defHistFile) == NULL)
            return mlwrite(MWABORT|MWCLEXEC,(uint8 *)"[No default history file]") ;
    }
    else
    {
        uint8 tmp[FILEBUF] ;	/* Filename */
        if(mlreply((uint8 *)"Save history",MLFILECASE,0,tmp,FILEBUF) != TRUE)
            return FALSE ;
        if(!fileLookup(tmp,(uint8 *)".erf",meFL_CHECKDOT|meFL_USESRCHPATH,filename))
            meStrcpy(filename,tmp) ;
        fname = filename ;
    }
    /* Write the strings into the registry */
    if ((regRoot = regRead((uint8 *)"history", NULL, REGMODE_BACKUP)) == NULL)
        return mlwrite (MWABORT|MWPAUSE,(uint8 *)"Cannot write history");
    
    /* Delete any other history information */
    if((regHistory = regFind (regRoot,(uint8 *)"history")) != NULL)
        regDelete (regHistory);
    
    /* Construct a new history entry */
    if ((regHistory = regSet (regRoot,(uint8 *)"history",(uint8 *)"")) != 0)
    {
        REGHANDLE regSection;
        
        /* hide from this node down as its junk */
        regHistory->mode |= REGMODE_HIDDEN ;
        
        /* Write the version information in */
        regSet (regHistory,(uint8 *)"version", histVerId);
        
        for (ii = 0; ii < 5; ii++)
        {
            /* Construct the new node container */
            if ((regSection = regSet(regHistory, regNames [ii],(uint8 *)"")) != NULL)
            {
                int numHistory = *(numList[ii]);
                
                for (jj = 0; jj < numHistory; jj++)
                {
                    name[0] = 'A' + jj ;
                    regSet (regSection, name,
                            strHist[(ii*MLHISTSIZE)+jj]);
                }
            }
        }
        
                        
        /* Construct the buffer history list container and make it
         * current. */
        if ((regHistory = regSet (regHistory,(uint8 *)"active",(uint8 *)"")) != NULL)
        {
            int bufno=0 ;
            /* Write the buffer information */
            for (bp = bheadp; bp != NULL; bp = bp->b_bufp)
            {
                if((bp->b_bname[0] != '*') && (bp->b_fname != NULL))
                {
                    uint8 buff[40] ;
                    int32 lineNo ;
                    int histno ;
                    
                    if(bp->b_nwnd)
                    {
                        if(bp == curbp)
                        {
                            histno = 2 ;
                            lineNo = curwp->line_no+1;
                        }
                        else
                        {
                            WINDOW *wp ;
                            histno = 1 ;
                            wp = wheadp ;
                            while(wp->w_bufp != bp)
                                wp = wp->w_wndp ;
                            lineNo = wp->line_no+1;
                        }
                    }
                    else
                    {
                        histno = 0 ;
                        lineNo = bp->line_no ;
                    }
                    sprintf((char *)buff,"%d,%d,%ld,%d,%d,%d,%d,%d",bufno++,histno,lineNo,
                            bp->b_mode[0],bp->b_mode[1],bp->b_mode[2],
                            bp->b_mode[3],bp->b_mode[4]) ;
                    /* Write file name = state info. */
                    regSet (regHistory, buff, bp->b_fname) ;
                }
            }
        }
    }
    
    /* Save the history to file. */
    if (regSave (regRoot, fname))
    {
        mlwrite(MWCLEXEC,(uint8 *)"[History written in %s]", fname) ;
        return TRUE ;
    }
    return FALSE;
}
