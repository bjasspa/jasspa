/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * history.c - histroy saving and re-loading routines.
 *
 * Copyright (C) 1995-2001 Steven Phillips
 * Copyright (C) 2002-2009 JASSPA (www.jasspa.com)
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

void
initHistory(void)
{
    /* Must malloc the 20 history slots for the 5 history types */
    if((strHist = (meUByte **) meMalloc(sizeof(meUByte *) * meHISTORY_COUNT * meHISTORY_SIZE)) == NULL)
        meExit(1);
    /* Initialise the array to NULLS so we know they don't point to a history
     * string
     */
    memset(strHist,0,sizeof(meUByte *) * meHISTORY_COUNT * meHISTORY_SIZE);
    buffHist = strHist + meHISTORY_SIZE;
    commHist = buffHist + meHISTORY_SIZE;
    fileHist = commHist + meHISTORY_SIZE;
    srchHist = fileHist + meHISTORY_SIZE;
    listHist = srchHist + meHISTORY_SIZE;
}

int
setupHistory(int option, meUByte **numPtr, meUByte ***list)
{
    if(option & MLBUFFER)
    {
        *numPtr = &numBuffHist;
        *list = buffHist;
    }
    else if(option & MLCOMMAND)
    {
        *numPtr = &numCommHist;
        *list = commHist;
    }
    else if(option & MLFILE)
    {
        *numPtr = &numFileHist;
        *list = fileHist;
    }
    else if(option & MLSEARCH)
    {
        *numPtr = &numSrchHist;
        *list = srchHist;
    }
    else if(option & MLHISTLIST)
    {
        *numPtr = &numListHist;
        *list = listHist;
    }
    else
    {
        *numPtr = &numStrHist;
        *list = strHist;
    }
    return **numPtr;
}


void
addHistory(int option, meUByte *str, int rmv)
{
    meUByte   *numPtr, numHist;
    meUByte  **history, *buf ;
    meInt      ii;

    numHist = setupHistory(option, &numPtr, &history) ;
    
    for(ii=0 ; ii<numHist ; ii++)
    {
        if(!meStrcmp(str,history[ii]))
        {
            if(rmv)
            {
                meFree(history[ii]);
                numHist--;
                *numPtr = numHist;
                while(ii < numHist)
                {
                    history[ii] = history[ii+1] ;
                    ii++;
                }
                history[ii] = NULL;
            }
            else
            {
                buf = history[ii] ;
                while(--ii >= 0)
                    history[ii+1] = history[ii] ;
                history[0] = buf ;
            }
            return;
        }
    }
    if(!rmv && ((buf = meMalloc(meStrlen(str)+1)) != NULL))
    {
        meStrcpy(buf,str) ;
        if(numHist == meHISTORY_SIZE)
            meFree(history[numHist-1]) ;
        else
        {
            numHist++ ;
            (*numPtr)++ ;
        }
        for(ii=numHist-1 ; ii>0 ; ii--)
            history[ii] = history[ii-1] ;
        history[0] = buf;
    }
}

