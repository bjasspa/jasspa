/* -*- C -*- ****************************************************************
 *
 *  System        : MicroEmacs Jasspa Distribution
 *  Module        : termio.c
 *  Synopsis      : Generic terminal support routines
 *  Created By    : Steven Phillips
 *  Created       : 1993
 *  Last Modified : <001006.1813>
 *
 *  Description
 *     Many generic routines to support timers, key input and some output.
 *
 *  Notes
 * 
 ****************************************************************************
 * 
 * Copyright (c) 1993-2000 Steven Phillips    
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
#include "emain.h"
#include "eskeys.h"
#ifndef _WIN32
#include <time.h>
#include <sys/time.h>
#endif

#ifdef _STDARG
#include <stdarg.h>		/* Variable Arguments */
#endif

#if (defined _WIN32) || (defined _DOS)
/* executable extension list, in reverse order of dos priority
 * included 4dos's btm files for completeness */
uint8 *execExtensions[noExecExtensions] = {".btm", ".bat", ".exe", ".com"} ;
#endif

/* SpecialChars; This is an array of special characters, below 32 used for
 * rendering boxes, lines etc. The following table provides a conversion
 * between those characters */
uint8 ttSpeChars [TTSPECCHARS] =
{
    ' ', /*  0/0x00 - Undefined */
    ' ', /*  1/0x01 - Undefined */
    ' ', /*  2/0x02 - Undefined */
    'H', /*  3/0x03 - Graphic / Playing card Heart */
    'D', /*  4/0x04 - Graphic / Playing card Diamond */
    'C', /*  5/0x05 - Graphic / Playing card Club */
    'S', /*  6/0x06 - Graphic / Playing card Space */
    ' ', /*  7/0x07 - Visible space */
    ' ', /*  8/0x08 - Undefined */
    '\t', /*  9/0x09 - Tab character */
    ' ', /* 10/0x0a - New line character */
    '+', /* 11/0x0b - Line Drawing / Bottom right _| */
    '+', /* 12/0x0c - Line Drawing / Top right */
    '+', /* 13/0x0d - Line Drawing / Top left */
    '+', /* 14/0x0e - Line Drawing / Bottom left |_ */
    '+', /* 15/0x0f - Line Drawing / Centre cross + */
    '>', /* 16/0x10 - Cursor Arrows / Right */
    '<', /* 17/0x11 - Cursor Arrows / Left */
    '-', /* 18/0x12 - Line Drawing / Horizontal line - */
    ' ', /* 19/0x13 - Undefined */
    ' ', /* 20/0x14 - Undefined */
    '+', /* 21/0x15 - Line Drawing / Left Tee |- */
    '+', /* 22/0x16 - Line Drawing / Right Tee -| */
    '+', /* 23/0x17 - Line Drawing / Bottom Tee _|_ */
    '+', /* 24/0x18 - Line Drawing / Top Tee -|- */
    '|', /* 25/0x19 - Line Drawing / Vertical Line | */
    ' ', /* 26/0x1a - Graphic  / Filled Gray Box */
    ' ', /* 27/0x1b - undefined */
    ' ', /* 28/0x1c - undefined */
    ' ', /* 28/0x1d - undefined */
    '^', /* 29/0x1e - Cursor Arrows / Up */
    'v'  /* 31/0x1f - Cursor Arrows / Down */
};

void
TTdoBell(int n)
{  
    register int index;
    uint32 arg ;
    
    if(macbug < -1)
        macbug = 1 ;
    if((index = decode_key(ME_SPECIAL|SKEY_bell,&arg)) >= 0)
        execFuncHidden(ME_SPECIAL|SKEY_bell,index,arg) ;
    else if((n == 0) || ((n == 1) && meModeTest(globMode,MDQUIET)))
    {
        uint8 scheme=(globScheme/meSCHEME_STYLES) ;
        pokeScreen(POKE_NOMARK+0x10,TTnrow,TTncol-6,&scheme,
                   (uint8 *) "[BELL]") ;
        vvideo.video[TTnrow].endp = TTncol ;
        resetCursor() ;
        mlStatus |= MLSTATUS_CLEAR ;
    }
    else
    {
        TTNbell() ;
    }
}
void
TTbell(void)
{  
    if(kbdmode == PLAY)
    {
        kbdmode = STOP ;
        if(meRegCurr->force < 2)
            /* a double !force says \CG is okay so don't complain */
            mlwrite(0,(uint8 *)"Macro terminated by ringing the bell") ;
    }
    TTinflush() ;
    TTdoBell(1) ;
}

uint8 meTimerState[NUM_TIMERS]={0} ;/* State of the timers, set or expired */
int32 meTimerTime[NUM_TIMERS]={0} ; /* Absolute time of the timers */

#ifdef _MULTI_TIMER

#ifdef _WIN32
static INT meTimerId[NUM_TIMERS]={0} ;      /* Windows timer handles          */
#define meSetMultiTimer(id,tim)  (meTimerId[id]=SetTimer(ttHwnd,id,tim,(TIMERPROC)(LPVOID)NULL))
#define meKillMultiTimer(id)                                                 \
do {                                                                         \
    if(meTimerId[id] != 0)                                                   \
    {                                                                        \
        KillTimer(ttHwnd,meTimerId[id]) ;                                    \
        meTimerId[id] = 0 ;                                                  \
    }                                                                        \
} while(0)

#endif

#endif

#ifdef _SINGLE_TIMER

#ifdef _UNIX
static struct itimerval itv={{0},{0}} ;           /* itimer interval                */
#define meSetSingleTimer(tim)                                                \
((itv.it_value.tv_sec=tim/1000),(itv.it_value.tv_usec=(tim%1000)*1000),setitimer(ITIMER_REAL,&itv,0))
#define meKillSingleTimer()                                                  \
(itv.it_value.tv_sec=0,itv.it_value.tv_usec=0,setitimer(ITIMER_REAL,&itv,0))
#endif

#endif


#ifdef _MULTI_TIMER
#ifdef _MULTI_NOID_TIMER
void
timerAlarm(int id)
{
    /* the id is the timer id stored in meTimeId not the timer number, convert */
    int ii=NUM_TIMERS ;
    
    /* must kill the timer or it will come back!!! */
    KillTimer(NULL,id) ;
    while(--ii >= 0)
        if(meTimerId[ii] == id)
        {
            meTimerId[ii] = 0 ;
            meTimerState[ii] = (meTimerState[ii] & ~TIMER_SET) | TIMER_EXPIRED;
        }
}
#endif
#endif
    

#if (defined _CONST_TIMER) || (defined _SINGLE_TIMER)

static TIMERBLOCK tblock[NUM_TIMERS]={      /* Timer block storage            */
    { NULL, 0, AUTOS_TIMER_ID },
    { NULL, 0, SLEEP_TIMER_ID },
    { NULL, 0, CALLB_TIMER_ID },
    { NULL, 0, IDLE_TIMER_ID },
    { NULL, 0, CURSOR_TIMER_ID }
#if MOUSE
   ,{ NULL, 0, MOUSE_TIMER_ID }
#endif
#ifdef _URLSUPP
   ,{ NULL, 0, SOCKET_TIMER_ID }
#endif
} ;
TIMERBLOCK *timers = NULL ;                 /* Head of timer list             */

/*FILE *fplog=NULL ;*/

/*
 * timerCheck
 * Check the status of the timers. Invokes the timer callback functions
 * if any of the timers have expired.
 *
 * timerCheck is invoked after a timer has changed status, it schedules
 * the timer fornext time, taking into account the order of the timers
 * on the list.
 *
 * tim may be specified as zero, in which case the current time is
 * obtained.
 */

void
timerCheck(long tim)
{
    TIMERBLOCK *tbp;                    /* Current timer block */
    TIMERBLOCK *ntbp;                   /* Next timer block */

    /* Make sure that there is something to do */
    if((tbp = timers) == NULL)          /* Timers scheduled ?? */
        return ;

    /* Get the current time if not supplied */
    if(tim == 0)
    {
        struct meTimeval tp ;             /* Time interval for current time */

        gettimeofday(&tp,NULL) ;
        tim = ((tp.tv_sec-startTime)*1000) + (tp.tv_usec/1000);
    }

    /* Iterate down the list of timers looking for any that have
     * expired. 
     */
    do
    {
        ntbp = tbp->next ;              /* Save the next timer in list */
        /* Timer expired ?? */
        if (tim+TIMER_MIN >= tbp->abstime)
        {
/*            fprintf(fplog,"Timer %d %d Expired\n",tbp->id,tbp->abstime) ;*/
            /* Report it as expired */
            meTimerState[tbp->id] = (meTimerState[tbp->id] & ~TIMER_SET) | TIMER_EXPIRED ;
            /* Now remove the timer - we know we're first in the list */
            timers = tbp->next ;
        }
        else
        {
#ifdef _SINGLE_TIMER
            /* Set up the timer for next time */
            tim = tbp->abstime - tim;        /* New offset time */
            /*            fprintf(fplog,"Timer %d %d Reset %d\n",tbp->id,tbp->abstime,tim) ;*/
            /* Set a new timer interval */
            meSetSingleTimer(tim) ;
#endif
            break ;
        }
    }
    while((tbp = ntbp) != NULL);
}

#endif


/*
 * timerSet
 * Start a timer, reset or continue the timer. In all eventualities
 * the timer is re-started and will run.
 *
 * id     - handle of the timer.
 * tim    - absolute time of alarm time.
 *          if (tim == 0) Absolute time is unknown but value
 *              is needed in meTimerTime.
 *          if (tim < 0)  Absolute time is unknown and value
                is not needed.
 * offset - offset from the current time to alarm.
 */
void
timerSet(int id, int32 tim, int32 offset)
{
#if (defined _CONST_TIMER) || (defined _SINGLE_TIMER)
    TIMERBLOCK *tbp;
    TIMERBLOCK *p;
    TIMERBLOCK *q;    
#ifdef _SINGLE_TIMER
    int mustSet = 0 ;
#endif
#endif
#ifdef _UNIX
    /* On unix, must hold back the signals else we can get in a right state */
    meSigHold() ;
#endif
    
    if(offset < TIMER_MIN)
    {
        /* must increase the time to the min */
        if(tim > 0)
            tim += TIMER_MIN - offset ;
        offset = TIMER_MIN ;
    }
#ifdef _MULTI_TIMER
    {
#ifdef _MULTI_NOID_TIMER
        /* each timer does not keep the id, so kill the current timer if set */
#ifdef _WINCON
        if (meSystemCfg & meSYSTEM_CONSOLE)
#endif
            meKillMultiTimer(id) ;
#endif
        meSetMultiTimer(id,offset) ;
        if(tim == 0)
        {
            struct meTimeval tp ;
            /* Get the absolute time */
            gettimeofday(&tp,NULL) ;
            tim = ((tp.tv_sec-startTime)*1000) + (tp.tv_usec/1000) + offset;
        }
    }
#endif
#if (defined _CONST_TIMER) || (defined _SINGLE_TIMER)
    {
        tbp = &tblock [id];
        /* Kill off the old timer, and unlink from list */
        if(meTimerState[id] & TIMER_SET)
        {
            if (tbp == timers)
            {
                timers = tbp->next;
#ifdef _SINGLE_TIMER
                mustSet = 1 ;
#endif
            }
            else
            {
                /* Search the list for the timer block */
                p = timers ;
                while(p != NULL)
                {
                    if(p->next == tbp)
                    {
                        p->next = tbp->next;
                        break ;
                    }
                    p = p->next ;
                }
            }
        }
        
        if(tim <= 0)
        {
            struct meTimeval tp ;
            /* Get the absolute time */
            gettimeofday(&tp,NULL) ;
            tim = ((tp.tv_sec-startTime)*1000) + (tp.tv_usec/1000) + offset ;
        }
        tbp->abstime = tim ;
        /*    fprintf(fplog,"Timer %d %d Set\n",tbp->id,tbp->abstime) ;*/
        /* Insertion sort the timer block into the list. */
        for (q = NULL, p = timers; p != NULL; p = p->next)
        {
            if (p->abstime > tbp->abstime)
                break;
            q = p;
        }
        
        if (q == NULL)
        {
            timers = tbp;                   /* Link to the head of the list */
#ifdef _SINGLE_TIMER
            /* Set a new timer interval */
            meSetSingleTimer(offset) ;
            mustSet = 0 ;
#endif
        }
        else
            q->next = tbp;                  /* Link into middle of list */
        tbp->next = p;                      /* Fix the next pointer */
    }
#endif
        
    meTimerState[id] = (meTimerState[id] & ~TIMER_EXPIRED) | TIMER_SET ;
    meTimerTime[id] = tim ;
        
#ifdef _SINGLE_TIMER
    if(mustSet)
        /* Call timerCheck to schedule the timer */
        timerCheck(tim-offset) ;
#endif
#ifdef _UNIX
    meSigRelease() ;
#endif
}

/*
 * timerKill
 * Kills a timer if it is currently running. The timer is stopped
 * and removed from the timer list.
 *
 * A macro wrap checks if the timer is running, if it is then
 * it is descheduled, otherwise there is no work to be done.
 */


int
_timerKill (int id)
{
    /* Kill off the old timer */
    if(isTimerSet(id))
    {
#if (defined _CONST_TIMER) || (defined _SINGLE_TIMER)
        TIMERBLOCK *tbp;
        TIMERBLOCK *p;
#endif
        
#ifdef _MULTI_TIMER
        {
            meKillMultiTimer(id) ;
        }
#endif
#if (defined _CONST_TIMER) || (defined _SINGLE_TIMER)
        {
            tbp = &tblock [id];
            
            if (tbp == timers)
            {
                timers = tbp->next;
#ifdef _SINGLE_TIMER
                if(timers == NULL)
                {
                    /* No other timer - kill the timer */
                    meKillSingleTimer() ;
                }
                else if(timers->abstime > tbp->abstime)
                    /* Must change the timer to the next timer which is
                     * a different time
                     */
                    timerCheck(0) ;
#endif
            }
            else
            {
                for (p = timers; p != NULL; p = p->next)
                {
                    if (p->next == tbp)
                    {
                        p->next = tbp->next;
                        break;
                    }
                }
            }
            tbp->next = NULL;
        }
#endif            
    }
    return (meTimerState[id] = meTimerState[id] & ~(TIMER_SET|TIMER_EXPIRED)) ;
}

/* translate-key support code */
#define ME_DELETE_KEY 0xffff           /* an invalid key number used to flag the key is deleted */
meTRANSKEY TTtransKey={0,250,0,ME_INVALID_KEY,NULL} ;

/* Keyboard buffer */
uint16 TTkeyBuf [KEYBUFSIZ]={0};
uint8  TTnextKeyIdx = KEYBUFSIZ;
uint8  TTlastKeyIdx = KEYBUFSIZ;
uint8  TTnoKeys = 0;
uint8  TTbreakFlag = 0;
uint8  TTbreakCnt = TTBREAKCNT;
uint8  TTallKeys = 0;           /* Report all keys */

int TTcurr =  0;                 /* Windows current row. */
int TTcurc =  0;                 /* Windows current column */
int TTfocus = 1;                 /* Is the window the current focus */


void
TThandleBlink(int initFlag)
{
    if((cursorState < 0) || !TTfocus)
        timerClearExpired(CURSOR_TIMER_ID) ;
    else
    {
        int ii ;
        
        if(initFlag && blinkState)
        {
            if(initFlag & 2)
            {
                /* The cursor must be redrawn */
                TTshowCur() ;
                TTflush() ;
            }
            ii = cursorBlink & 0xffff ;
        }
        else
        {
            blinkState ^= 1 ;
            if(blinkState)
            {
                TTshowCur() ;
                ii = cursorBlink & 0xffff ;
            }
            else
            {
                TThideCur() ;
                if((ii = cursorBlink >> 16) == 0)
                    ii = cursorBlink ;
            }
            TTflush() ;
        }
        timerSet (CURSOR_TIMER_ID,-1,ii) ;
    }
}

void
TTmove(int row, int col)
{
    if ((row != TTcurr) || (col != TTcurc))
    {
        if((cursorState >= 0) && blinkState)
            TThideCur() ;
        TTcurr = row ;
        TTcurc = col ;
    }
    if((cursorState >= 0) && blinkState)
        TTshowCur() ;
}


#ifdef FREE_ALL_MEMORY

static void
TTfreeTranslateKeyChild(meTRANSKEY *tcapKeys)
{
    int ii ;
    for(ii=0 ; ii<tcapKeys->count ; ii++)
        TTfreeTranslateKeyChild(tcapKeys->child+ii) ;
    meNullFree(tcapKeys->child) ;
}

void
TTfreeTranslateKey(void)
{
    TTfreeTranslateKeyChild(&TTtransKey) ;
}
   
#endif

void
translateKeyAdd(meTRANSKEY *tcapKeys, int count,int time, uint16 *key, uint16 map)
{
    if(--count < 0)
    {
        tcapKeys->map = map ;
        tcapKeys->time = time ;
    }
    else
    {
        uint16 cc=*key++ ;
        int ii ;
        
        for(ii=0 ; ii<tcapKeys->count ; ii++)
            if(tcapKeys->child[ii].key == cc)
                break ;
        if(ii == tcapKeys->count)
        {
            tcapKeys->child = realloc(tcapKeys->child,(ii+1)*sizeof(meTRANSKEY)) ;
            (tcapKeys->count)++ ;
            tcapKeys->child[ii].key   = cc ;
            tcapKeys->child[ii].count = 0 ;
            tcapKeys->child[ii].time  = time ;
            tcapKeys->child[ii].map   = ME_INVALID_KEY ;
            tcapKeys->child[ii].child = NULL ;
        }
        else if(tcapKeys->child[ii].time < time)
            tcapKeys->child[ii].time = time ;
        translateKeyAdd(tcapKeys->child+ii,count,time,key,map) ;
    }
}

int
keyListToShorts(uint16 *sl, uint8 *kl)
{
    uint8 dd ;
    int ii=0 ;
    
    for(;;)
    {
        for(;;kl++)
        {
            if((dd=*kl) == '\0')
                return ii ;
            if(!isSpace(dd))
                break ;
        }
        sl[ii++] = getskey(&kl) ;
    }
}

int
charListToShorts(uint16 *sl, uint8 *cl)
{
    uint8 cc ;
    int ii=0 ;
    
    while((cc=*cl++) != 0)
        sl[ii++] = (uint16) cc ;
    return ii ;
}

static void
translateKeyShow(meTRANSKEY *tcapKeys, BUFFER *bp, uint8 *key, int depth)
{
    int ii, nd ;
    
    if(tcapKeys->map != ME_INVALID_KEY)
    {
        uint8 buf[MAXBUF] ;
        
        meStrcpy(buf,"    \"") ;
        ii = 5 ;
        meStrncpy(buf+ii,key,depth) ;
        ii += depth ;
        buf[ii++] = '"' ;
        buf[ii++] = ' ' ;
        if(ii < 35)
        {
            memset(buf+ii,'.',35-ii) ;
            ii = 35 ;
        }
        buf[ii++] = ' ' ;
        buf[ii++] = '"' ;
        if(tcapKeys->map != ME_DELETE_KEY)
            ii += cmdchar(tcapKeys->map,buf+ii);
        buf[ii++] = '"' ;
        buf[ii] = '\0' ;
        addLineToEob(bp,buf) ;
    }
    if(depth)
        key[depth++] = ' ' ;
    for(ii=0 ; ii<tcapKeys->count ; ii++)
    {
        nd = depth + cmdchar(tcapKeys->child[ii].key,key+depth);
        translateKeyShow(tcapKeys->child+ii,bp,key,nd) ;
    }
}

int
translateKey(int f, int n)
{
    static uint8 tnkyPrompt[]="translate-tcap-key code" ;
    uint16 c_from[20], c_to[20] ;
    uint8 buf[128] ;
    int ii ;
    
    if(n == 0)
    {
        WINDOW *wp ;
        BUFFER *bp ;
    
        if((wp = wpopup(BtranskeyN,(BFND_CREAT|BFND_CLEAR|WPOP_USESTR))) == NULL)
            return FALSE ;
        bp = wp->w_bufp ;
        
        translateKeyShow(&TTtransKey,bp,buf,0) ;
        bp->b_dotp = lforw(bp->b_linep);
        bp->b_doto = 0 ;
        bp->line_no = 0 ;
        meModeClear(bp->b_mode,MDEDIT) ;    /* don't flag this as a change */
        meModeSet(bp->b_mode,MDVIEW) ;      /* put this buffer view mode */
        resetBufferWindows(bp) ;            /* Update the window */
        mlerase(MWCLEXEC);	                /* clear the mode line */
    }
    else
    {
        meStrcpy(tnkyPrompt+19,"code") ;
        if((mlreply(tnkyPrompt,0,0,buf,128) != TRUE) ||
           ((ii=keyListToShorts(c_from,buf)) <= 0))
            return FALSE ;

        if(n == -1)
            c_to[0] = ME_INVALID_KEY ;
        else
        {
            if(f == FALSE)
                n = TTtransKey.time ;
            meStrcpy(tnkyPrompt+19,"to") ;
            if((mlreply(tnkyPrompt,0,0,buf,128) != TRUE) ||
               ((f=keyListToShorts(c_to,buf)) < 0))
                return FALSE ;
            if(f == 0)
                c_to[0] = ME_DELETE_KEY ;
        }
        translateKeyAdd(&TTtransKey,ii,n,c_from,*c_to) ;
    }
    return TRUE ;
}

uint16
TTgetc(void)
{
    uint16 cc ;
    
    do {
        if(!TTahead())
            TTwaitForChar() ;
        
        if(TTtransKey.count)
        {
            meTRANSKEY *tt ;
            int         ii, nextKeyIdx, keyNo ;
            
            tt = &TTtransKey ;
            nextKeyIdx = TTnextKeyIdx ;
            keyNo = 0 ;
            
            while(tt->count != 0)
            {
                if(keyNo == TTnoKeys)
                {
                    /* Wait for a quarter a second to see if any more keys are available */
                    /* Only gocha here is that preferably you want an interuptable wait
                     * therefore we must set the TTnoKeys to 0 so we only interrupt on
                     * a receipt of a new key. Must remember to reset it as well.
                     */
                    TTnoKeys = 0 ;
                    TTsleep(tt->time,1) ;
                    TTnoKeys += keyNo ;
                    if(keyNo == TTnoKeys)
                        break ;
                }
                keyNo++ ;
                if(!nextKeyIdx)
                    nextKeyIdx = KEYBUFSIZ ;
                cc = TTkeyBuf[--nextKeyIdx] ;
                for(ii=0 ; ii<tt->count ; ii++)
                {
                    if(tt->child[ii].key == cc)
                    {
                        tt = tt->child+ii ;
                        if(tt->map == ME_DELETE_KEY)
                        {
                            TTnextKeyIdx = nextKeyIdx ;
                            TTnoKeys -= keyNo ;
                            keyNo = 0 ;
                        }
                        else if(tt->map != ME_INVALID_KEY)
                        {
                            TTnextKeyIdx = nextKeyIdx + 1 ;
                            TTnoKeys -= keyNo-1 ;
                            TTkeyBuf[nextKeyIdx] = tt->map ;
                            keyNo = 1 ;
                        }
                        ii = 0 ;
                        break ;
                    }
                }
                if(ii == tt->count)
                    break ;
            }
        }
    } while(!TTnoKeys) ;
    
    if(!TTnextKeyIdx)
        TTnextKeyIdx = KEYBUFSIZ ;
    cc = TTkeyBuf[--TTnextKeyIdx] ;
    TTnoKeys-- ;
    /* remove the abort flag if we are waiting for the key */
    if((cc == 0x07) && TTbreakFlag)
        TTbreakFlag = 0 ;
    return cc ;
}

/*
 * doIdlePickEvent
 * Handle the idle event.
 */
void
doIdlePickEvent(void)
{
    register int index;
    uint32 arg;

    /* See if there is any binding on the idle event */
    if((index=decode_key(ME_SPECIAL|SKEY_idle_pick,&arg)) != -1)
    {
        /* Execute the idle-pick key */
        execFuncHidden(ME_SPECIAL|SKEY_idle_pick,index,arg) ;
        
        /* Now see if theres a idle-time event, if so start the timer */
        if(decode_key(ME_SPECIAL|SKEY_idle_time, &arg) != -1)
            timerSet(IDLE_TIMER_ID,-1,idletime);
        else if(decode_key(ME_SPECIAL|SKEY_idle_drop,&arg) != -1)
            meTimerState[IDLE_TIMER_ID] = IDLE_STATE_DROP ;
    }
}

/*
 * doIdleDropEvent
 * Handle the end of an idle event.
 */
static void
doIdleDropEvent(void)
{
    register int index;
    uint32 arg;

    if(isTimerActive(IDLE_TIMER_ID))
        /* Kill off the timer */
        timerKill(IDLE_TIMER_ID) ;
    
    /* do the idle-drop if bound */
    if((index=decode_key(ME_SPECIAL|SKEY_idle_drop,&arg)) != -1)
        /* Execute the idle-pick key */
        execFuncHidden(ME_SPECIAL|SKEY_idle_drop,index,arg) ;
    meTimerState[IDLE_TIMER_ID] = 0;
}

/*
 * addKeyToBuffer
 * Add a new keyboard character to the keyboard buffer.
 */
void
addKeyToBuffer(uint16 cc)
{
#if MOUSE
    /* Reset the mouse context if mouse interaction is outstanding. 
     * This should only be timer keys
     * Kill off the timer */
    timerKill(MOUSE_TIMER_ID);
#endif
    /* Flag the end of the idle if necessary */
    if(meTimerState[IDLE_TIMER_ID] != 0)
        doIdleDropEvent() ;
    
    /* If cursor is blinking, ensure the cursor is visible */
    if((cursorState >= 0) && cursorBlink)
        TThandleBlink(1) ;
	
    /* Add the key to the buffer */
    if (TTnoKeys < KEYBUFSIZ)
    {
        if (!TTlastKeyIdx)
            TTlastKeyIdx = KEYBUFSIZ;
        TTkeyBuf [--TTlastKeyIdx] = cc;
        TTnoKeys++;
    }
    else
        TTNbell() ;
}

#ifdef _STDARG
char *
meTParm(char *str, ...)
#else
char *
meTParm(char *str, int32 arg)
#endif
{
    static char *tbuff=NULL ;
    static int32 tbuffLen=0 ;
    int32 stack[10], ii, stackDep=0, len=0 ;
    char cc, *ss ;
    
    for(;;)
    {
        if(len >= tbuffLen)
        {
            tbuff = realloc(tbuff,tbuffLen+32) ;
            if(tbuff == NULL)
            {
                tbuffLen = 0 ;
                return (char *) "" ;
            }
            tbuffLen += 16 ;
        }
        if((cc=*str++) == '\0')
            break ;
        else if(cc == '%')
        {
            switch((cc=*str++))
            {
                /* miscellaneous codes */
            case '\0':
                str-- ;
                break ;
            case '%':
                tbuff[len++] = '%' ;
                break ;
                
                /* codes which push values to the stack */
            case '\'':
                if((cc=*str++) != '\0')
                {
                    stack[stackDep++] = (int32) cc ;
                    if(*str++ != '\'')
                        str-- ;
                }
                else
                    str-- ;
                break ;
            case '{':
                ii = 0 ;
                while((cc=*str++),isDigit(cc))
                    ii = (ii * 10) + (cc - '0') ;
                /* minimal error handling */
                if(cc != '}')
                    str-- ;
                stack[stackDep++] = ii ;
                break ;
            case 'p':
                /* get the nth parameter */
                {
#ifdef _STDARG
                    va_list ap;
                    int pno ;
                    va_start(ap,str);
                    pno = *str++ - '0' ;
                    do {
                        ii = va_arg(ap, int32) ;
                    } while(--pno > 0) ;
                    va_end (ap);
#else
                    register int32 *ap;
                    ap = &arg;
                    ii = ap[*str++ - '1'] ;
#endif
                    stack[stackDep++] = ii ;
                    break ;
                }
                
                /* codes which operate on values in the stack */
            case '+':
                if(stackDep > 1)
                {
                    stackDep-- ;
                    stack[stackDep-1] = stack[stackDep-1] + stack[stackDep] ;
                }
                break ;
            case '-':
                if(stackDep > 1)
                {
                    stackDep-- ;
                    stack[stackDep-1] = stack[stackDep-1] - stack[stackDep] ;
                }
                break ;
            case '*':
                if(stackDep > 1)
                {
                    stackDep-- ;
                    stack[stackDep-1] = stack[stackDep-1] * stack[stackDep] ;
                }
                break ;
            case '/':
                if(stackDep > 1)
                {
                    stackDep-- ;
                    stack[stackDep-1] = stack[stackDep-1] / stack[stackDep] ;
                }
                break ;
            case 'm':
                if(stackDep > 1)
                {
                    stackDep-- ;
                    stack[stackDep-1] = stack[stackDep-1] % stack[stackDep] ;
                }
                break ;
            case '&':
                if(stackDep > 1)
                {
                    stackDep-- ;
                    stack[stackDep-1] = stack[stackDep-1] & stack[stackDep] ;
                }
                break ;
            case '|':
                if(stackDep > 1)
                {
                    stackDep-- ;
                    stack[stackDep-1] = stack[stackDep-1] | stack[stackDep] ;
                }
                break ;
            case '^':
                if(stackDep > 1)
                {
                    stackDep-- ;
                    stack[stackDep-1] = stack[stackDep-1] ^ stack[stackDep] ;
                }
                break ;
            case '=':
                if(stackDep > 1)
                {
                    stackDep-- ;
                    stack[stackDep-1] = stack[stackDep-1] == stack[stackDep] ;
                }
                break ;
            case '>':
                if(stackDep > 1)
                {
                    stackDep-- ;
                    stack[stackDep-1] = stack[stackDep-1] > stack[stackDep] ;
                }
                break ;
            case '<':
                if(stackDep > 1)
                {
                    stackDep-- ;
                    stack[stackDep-1] = stack[stackDep-1] < stack[stackDep] ;
                }
                break ;
            case 'A':
                if(stackDep > 1)
                {
                    stackDep-- ;
                    stack[stackDep-1] = stack[stackDep-1] && stack[stackDep] ;
                }
                break ;
            case 'O':
                if(stackDep > 1)
                {
                    stackDep-- ;
                    stack[stackDep-1] = stack[stackDep-1] || stack[stackDep] ;
                }
                break ;
            case '!':
                if(stackDep > 0)
                    stack[stackDep-1] = !stack[stackDep-1] ;
                break ;
            case '~':
                if(stackDep > 0)
                    stack[stackDep-1] = ~stack[stackDep-1] ;
                break ;
            
                /* codes which pop a value from the stackand add it to the output */
            case 'c':
                if(stackDep > 0)
                {
                    stackDep-- ;
                    tbuff[len++] = (char) stack[stackDep] ;
                }
                break ;
            case ':': case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
                ss = str-2 ;
                do
                    cc = *str++ ;
                while((cc != '\0') && (cc != 'd') && (cc != 'o') && (cc != 's') &&
                      (cc != 'x') && (cc != 'X')) ;
                if(cc == '\0')
                    break ;
                goto arg_print ;
            case 'd':
            case 'o':
            case 's':
            case 'x':
            case 'X':
                ss = str-2 ;
arg_print:
                if(cc == 's')
                    /* don't handle string args so change to 'd' */
                    str[-1] = 'd' ;
                cc = *str ;
                *str = '\0' ;
                stackDep-- ;
                sprintf(tbuff+len,ss,stack[stackDep]) ;
                len += strlen(tbuff+len) ;
                *str = cc ;
                break ;
            }
        }
        else
            tbuff[len++] = cc ;
    }
    tbuff[len] = '\0' ;
    return tbuff ;
}

