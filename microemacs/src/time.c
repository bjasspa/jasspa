/* -*- C -*- ****************************************************************
 *
 *  System        : MicroEmacs Jasspa Distribution
 *  Module        : time.c
 *  Synopsis      : File time stamping routines
 *  Created By    : Jon Green
 *  Created       : 10/05/1991
 *  Last Modified : <000221.0748>
 *
 *  Description
 *     Set auto file time stamping. Only defined if running some sort of
 *     UNIX/DOS/WIN32.
 *
 *  Notes
 *
 ****************************************************************************
 * 
 * Copyright (c) 1991-2000 Jon Green
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
 * Jon Green         jon@jasspa.com
 *
 ****************************************************************************/

/*---	Include defintions */

#define	__TIMEC			/* Define filename */

/*---	Include files */

#include "emain.h"
#include "esearch.h"

#if (defined _UNIX) || (defined _DOS) || (defined _WIN32)
#include <sys/types.h>
#include <time.h>
#endif


#if	TIMSTMP


/*---	Local macro definitions */

/*---	External references */

/*---	Local type definitions */

/*---	Local variable definitions */

uint8 time_stamp[TSTMPLEN] = "<%Y%M%D.%h%m>";	/* Time stamp string */

#define TSNUMFIELD 6
static uint8 TSFIELDS[]="YMDhms" ;

static void
createTimeStampSrch(uint8 *buf, uint8 *pos)
{
    uint8 curPos=0 ;
    uint8 cc, *ss, *tt ;
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
set_timestamp(BUFFER *bp)
{
    WINDOW *owp, win ;
    BUFFER *obp ;
    uint8   pos[TSNUMFIELD];	/* pos of fields in search */
    uint8   patt[1024];    	/* Search pattern */
    int     ii, jj ;
    
    /*---	Determine if time stamping is to be performed. */
    
    if(!meModeTest(bp->b_mode,MDTIME) ||	/* Time stamping mode on ?? */
       (time_stamp[0] == '\0'))			/* No time stamp defined */
        return TRUE ;				/* No - exit */
    
    mlwrite(MWCURSOR,(uint8 *)"[Time stamping File]");
    
    /*---	Save current position in buffer and go to the start of the buffer. */
    
    obp = curbp ;
    owp = curwp ;
    curwp = &win ;
    curbp = bp ;
    win.w_bufp = curbp ;
    win.w_dotp = lforw(curbp->b_linep) ;	/* Save position */
    win.w_doto = 0 ;
    win.w_flag = 0 ;
    win.line_no = 0 ;    
    
    /* create magic search string */
    createTimeStampSrch(patt,pos) ;
    
    /*---	Search for the time stamp string. */
    if(iscanner(patt,100,ISCANNER_PTBEG|ISCANNER_MAGIC|ISCANNER_EXACT,NULL) == TRUE)
    {
        int values[TSNUMFIELD] ;
        struct tm  *time_ptr;		/* Pointer to time frame. */
        time_t clock;			/* Time in machine format. */
        uint16 soff ;

        /* Found it, so fill in the slots */
        
        clock = time(0);				/* Get system time */
        time_ptr = (struct tm *) localtime (&clock);	/* Get time frame */
        
        values[0] = time_ptr->tm_year+1900 ;
        values[1] = time_ptr->tm_mon+1 ;
        values[2] = time_ptr->tm_mday ;
        values[3] = time_ptr->tm_hour ;
        values[4] = time_ptr->tm_min ;
        values[5] = time_ptr->tm_sec ;
        
        soff = curwp->w_doto ;
        for(ii=0 ; ii<TSNUMFIELD ; ii++)
            if((jj=pos[ii]) != 0)
            {
                if(!ii && ((mereRegexGroupEnd(jj)-mereRegexGroupStart(jj)) == 4))
                {
                    curwp->w_dotp->l_text[soff+mereRegexGroupEnd(jj)-4] = '0' + ((values[0]/1000)%10) ;
                    curwp->w_dotp->l_text[soff+mereRegexGroupEnd(jj)-3] = '0' + ((values[0]/100)%10) ;
                }
                curwp->w_dotp->l_text[soff+mereRegexGroupEnd(jj)-2] = '0' + ((values[ii]/10)%10) ;
                curwp->w_dotp->l_text[soff+mereRegexGroupEnd(jj)-1] = '0' + (values[ii]%10) ;
            }
        curwp->w_dotp->l_flag |= LNCHNG ;
    }
    else
        ii = 0 ;
    curbp = obp ;
    curwp = owp ;
    if(ii)
        /* if found flag any window displaying it to update */
        addModeToBufferWindows(bp,WFMAIN) ;

    return TRUE ;
}

#endif  /* TIMSTMP */

