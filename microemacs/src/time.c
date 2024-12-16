/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * time.c - File time stamping routines
 *
 * Copyright (C) 1991-2001 Jon Green
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
 * Created:     10/05/1991
 * Synopsis:    File time stamping routines
 * Authors:     Jon Green
 * Description:
 *     Set auto file time stamping. Only defined if running some sort of
 *     UNIX/DOS/WIN32.
 */

#define	__TIMEC			/* Define filename */

#include "emain.h"
#include "esearch.h"

#if (defined _UNIX) || (defined _DOS) || (defined _WIN32)
#include <sys/types.h>
#endif


#if MEOPT_TIMSTMP

meUByte time_stamp[meTIME_STAMP_SIZE_MAX] = "<%Y%M%D\\.%h%m>";	/* Time stamp string */

#define TSNUMFIELD 6
static meUByte TSFIELDS[]="YMDhms" ;

static void
createTimeStampSrch(meUByte *buf, meUByte *pos)
{
    meUByte curPos=0 ;
    meUByte cc, *ss, *tt ;
    int ii ;
    memset(pos,0,TSNUMFIELD) ;
    
    ss = time_stamp ;
    while((cc=*ss++) != '\0')
    {
        if((cc == '%') && ((cc=*ss++) != '\0') &&
           ((tt = meStrchr(TSFIELDS,cc)) != NULL))
        {
            ii = tt-TSFIELDS ;
            pos[ii] = ++curPos ;
            if(ii == 0)
            {
                /* for the year flags cater for an optional century (4 digits) */
                meStrcpy(buf,"\\(\\([0-9][0-9]\\)?[0-9][0-9]\\)") ;
                buf += 29 ;
                curPos++ ;
            }
            else
            {
                meStrcpy(buf,"\\([0-9][0-9]\\)") ;
                buf += 14 ;
            }
        }
        else
            *buf++ = cc ;
    }
    *buf = '\0' ;
}


int
set_timestamp(meBuffer *bp)
{
    meWindow *owp, win;
    meUByte pos[TSNUMFIELD];	/* pos of fields in search */
    meUByte patt[512];    	/* Search pattern */
    int ii, jj;
    
    /*---	Determine if time stamping is to be performed. */
    
    if(!meModeTest(bp->mode,MDTIME) ||	/* Time stamping mode on ?? */
       (time_stamp[0] == '\0'))			/* No time stamp defined */
        return meTRUE;				/* No - exit */
    
    mlwrite(MWCURSOR,(meUByte *)"[Time stamping file]");
    
    /*---	Save current position in buffer and go to the start of the buffer. */
    
    owp = frameCur->windowCur;
    frameCur->windowCur = &win;
    win.buffer = bp;
    win.dotLine = meLineGetNext(bp->baseLine) ;	/* Save position */
    win.dotOffset = 0;
    win.dotLineNo = 0;    
    win.updateFlags = 0;
    
    /* create magic search string */
    createTimeStampSrch(patt,pos);
    
    /*---	Search for the time stamp string. */
    if(iscanner(patt,100,ISCANNER_PTBEG|ISCANNER_MAGIC|ISCANNER_EXACT,NULL) == meTRUE)
    {
        int values[TSNUMFIELD];
        struct tm  *time_ptr;		/* Pointer to time frame. */
        time_t clock;			/* Time in machine format. */
        meUShort soff;

        /* Found it, so fill in the slots */
        
        clock = time(NULL);				/* Get system time */
        time_ptr = (struct tm *) localtime(&clock);	/* Get time frame */
        
        values[0] = time_ptr->tm_year+1900 ;
        values[1] = time_ptr->tm_mon+1 ;
        values[2] = time_ptr->tm_mday ;
        values[3] = time_ptr->tm_hour ;
        values[4] = time_ptr->tm_min ;
        values[5] = time_ptr->tm_sec ;
        
        soff = win.dotOffset ;
        for(ii=0 ; ii<TSNUMFIELD ; ii++)
            if((jj=pos[ii]) != 0)
            {
                if(!ii && ((mereRegexGroupEnd(jj)-mereRegexGroupStart(jj)) == 4))
                {
                    win.dotLine->text[soff+mereRegexGroupEnd(jj)-4] = '0' + ((values[0]/1000)%10) ;
                    win.dotLine->text[soff+mereRegexGroupEnd(jj)-3] = '0' + ((values[0]/100)%10) ;
                }
                win.dotLine->text[soff+mereRegexGroupEnd(jj)-2] = '0' + ((values[ii]/10)%10) ;
                win.dotLine->text[soff+mereRegexGroupEnd(jj)-1] = '0' + (values[ii]%10) ;
            }
        win.dotLine->flag |= meLINE_CHANGED ;
    }
    else
        ii = 0 ;
    frameCur->windowCur = owp;
    if(ii)
        /* if found flag any window displaying it to update */
        meBufferAddModeToWindows(bp,WFMAIN) ;

    return meTRUE ;
}

#endif  /* MEOPT_TIMSTMP */

