/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * history.c - histroy saving and re-loading routines.
 *
 * Copyright (C) 1995-2001 Steven Phillips
 * Copyright (C) 2002 JASSPA (www.jasspa.com)
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
 * Synopsis:    histroy saving and re-loading routines.
 * Authors:     Steven Phillips
 * Description:
 *     Saves the main registry configuration, with a history of currently
 *     loaded files (for use with -c option), last search string, buffer names
 *     etc.
 *
 * Notes:
 *     The history id string must be changed whenever some internals change,
 *     most notably the history file format and any change to the buffer
 *     modes (nasty side-effects with -c). This stops the use of wrong verson
 *     histories.
 */

#define	__HISTORYC		      /* Define the name of the file */

#include "emain.h"

static meUByte histVerId[] = "6" ;      /* History version */
meUByte *defHistFile=NULL ;

/* Constants */
static meUByte *regNames[5] = {
    (meUByte *)"string",(meUByte *)"buffer",(meUByte *)"command",(meUByte *)"file",(meUByte *)"search"
};
static meUByte *numList[5]=
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
    if((strHist = (meUByte **) meMalloc(sizeof(meUByte *) * 5 * MLHISTSIZE)) == NULL)
        meExit(1) ;
    /* Initialise the array to NULLS so we know they don't point to a history
     * string
     */
    memset(strHist,0,sizeof(meUByte *) * 5 * MLHISTSIZE) ;
    buffHist = strHist + MLHISTSIZE ;
    commHist = buffHist + MLHISTSIZE ;
    fileHist = commHist + MLHISTSIZE ;
    srchHist = fileHist + MLHISTSIZE ;
}

int
setupHistory(int option, meUByte **numPtr, meUByte ***list)
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
addHistory(int option, meUByte *str)
{
    meUByte   *numPtr, numHist,i ;
    meUByte  **history, *buf ;

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

#if REGSTRY

/*
 * Read in the history.
 */

int
readHistory(int f, int n)
{
    BUFFER *bp ;                        /* Buffer pointer */
    REGHANDLE regRoot;                  /* Root of the registry */
    REGHANDLE reg;                      /* temp registry pointer */
    meUByte filename[FILEBUF] ;	        /* Filename */
    meUByte *fname;	                /* Resulting file name to execute */
    int ii;                             /* Loop counter */
    
    if(n == 0)
    {
        if(defHistFile == NULL)
            return mlwrite(MWABORT|MWCLEXEC,(meUByte *)"[No default history file]") ;
        fname = defHistFile ;
    }
    else
    {
        if (meGetString((meUByte *)"Read history", MLFILECASE,0,filename,FILEBUF) != TRUE)
            return ABORT ;
        fname = filename ;
    }
    
    /* Write the strings into the registry */
    if ((regRoot = regRead((meUByte *)"history", fname, REGMODE_RELOAD|REGMODE_BACKUP)) == NULL)
        return ABORT ;
    
    if((f == TRUE) && (n != 0))
    {
        /* Store the default history file name */
        meNullFree(defHistFile) ;
        defHistFile = meStrdup(regRoot->value) ;
    }
    if ((regRoot = regFind(regRoot,(meUByte *)"history")) != NULL)
    {
        meUByte *ss;                       /* temp string pointer */
        
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
            if (((reg = regFind (regRoot,(meUByte *)"version")) != NULL) &&
                ((ss=regGetString(reg,NULL)) != NULL) && !meStrcmp(ss,histVerId))
            {
                /* Set up to extract the 'active' configuration */
                /* Reset the files loaded counter */
                ii = 0;
                if((reg = regFind (regRoot,(meUByte *)"active")) != NULL)
                    reg = regGetChild(reg) ;
                while (reg != NULL)
                {
                    meSTAT stats ;
                    meUByte  fnbuf[FILEBUF] ;
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
                        mode[0] = (meUByte) m0 ;
                        mode[1] = (meUByte) m1 ;
                        mode[2] = (meUByte) m2 ;
                        mode[3] = (meUByte) m3 ;
                        mode[4] = (meUByte) m4 ;
                        
                        /* don't worry about the binary, crypt and rbin
                         * modes here, we copy them across next */
                        bp = bfind(fname,BFND_CREAT|BFND_MKNAM);
                        bp->intFlag |= BIFFILE ;
                        bp->b_fname = fname ;
                        bp->line_no = lineNo ;
                        bp->histNo = histNo ;
                        
                        meModeCopy(bp->b_mode,mode) ;
                        meModeClear(bp->b_mode,MDEDIT) ;
                        meModeClear(bp->b_mode,MDNRRW) ;
                        /* set this to inactive */
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
        mlwrite(MWCLEXEC,(meUByte *)"[History loaded]") ;
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
    meUByte name[2] = "0";                /* Automatic name seed */
    meUByte filename[FILEBUF] ;           /* Filename */
    meUByte *fname;                       /* Resulting file name to execute */

    if(n == 0)
    {
        if((fname = defHistFile) == NULL)
            return mlwrite(MWABORT|MWCLEXEC,(meUByte *)"[No default history file]") ;
    }
    else
    {
        meUByte tmp[FILEBUF] ;	/* Filename */
        if(meGetString((meUByte *)"Save history",MLFILECASE,0,tmp,FILEBUF) != TRUE)
            return FALSE ;
        if(!fileLookup(tmp,(meUByte *)".erf",meFL_CHECKDOT|meFL_USESRCHPATH,filename))
            meStrcpy(filename,tmp) ;
        fname = filename ;
    }
    /* Write the strings into the registry */
    if ((regRoot = regRead((meUByte *)"history", NULL, REGMODE_BACKUP)) == NULL)
        return mlwrite (MWABORT|MWPAUSE,(meUByte *)"Cannot write history");
    
    /* Delete any other history information */
    if((regHistory = regFind (regRoot,(meUByte *)"history")) != NULL)
        regDelete (regHistory);
    
    /* Construct a new history entry */
    if ((regHistory = regSet (regRoot,(meUByte *)"history",(meUByte *)"")) != 0)
    {
        REGHANDLE regSection;
        
        /* hide from this node down as its junk */
        regHistory->mode |= REGMODE_HIDDEN ;
        
        /* Write the version information in */
        regSet (regHistory,(meUByte *)"version", histVerId);
        
        for (ii = 0; ii < 5; ii++)
        {
            /* Construct the new node container */
            if ((regSection = regSet(regHistory, regNames [ii],(meUByte *)"")) != NULL)
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
        if ((regHistory = regSet (regHistory,(meUByte *)"active",(meUByte *)"")) != NULL)
        {
            int bufno=0 ;
            /* Write the buffer information */
            for (bp = bheadp; bp != NULL; bp = bp->b_bufp)
            {
                if((bp->b_bname[0] != '*') && (bp->b_fname != NULL))
                {
                    meUByte buff[40] ;
                    meInt lineNo ;
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
        mlwrite(MWCLEXEC,(meUByte *)"[History written in %s]", fname) ;
        return TRUE ;
    }
    return FALSE;
}
#endif
