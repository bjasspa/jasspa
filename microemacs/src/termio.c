/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * termio.c - Generic terminal support routines.
 *
 * Copyright (C) 1993-2001 Steven Phillips
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
 * Created:     1993
 * Synopsis:    Generic terminal support routines.
 * Authors:     Steven Phillips
 * Description:
 *     Many generic routines to support timers, key input and some output.
 * 
 * Notes:
 *     This is not related to the original termio.c in the MicroEMACS 3.8
 *     release, the functions in that file have either been moved or removed.
 */
#include "emain.h"
#include "eskeys.h"
#include "efunc.h"

#ifdef _STDARG
#include <stdarg.h>		/* Variable Arguments */
#endif

/* SpecialChars; This is an array of special characters, below 32 used for
 * rendering boxes, lines etc. The following table provides a conversion
 * between those characters */
meUByte ttSpeChars[TTSPECCHARS] =
{
    ' ', /*  0/0x00 - Undefined */
    'u', /*  1/0x01 - Unicode tag - 3 byte encode */
    'U', /*  2/0x02 - Unicode tag - 5 byte encode */
    '[', /*  3/0x03 - Checkbox left side */
    ' ', /*  4/0x04 - Checkbox center not selected */
    'X', /*  5/0x05 - Checkbox center selected */
    ']', /*  6/0x06 - Checkbox right side */
    '?', /*  7/0x07 - unsupported/undefined char meCHAR_UNDEF (i.e. \uFFFD) */
    ' ', /*  8/0x08 - Visible backspace char */
    '\t',/*  9/0x09 - Tab character */
    ' ', /* 10/0x0a - Visible new line character */
    '+', /* 11/0x0b - Line Drawing / Bottom right _| */
    '+', /* 12/0x0c - Line Drawing / Top right */
    '+', /* 13/0x0d - Line Drawing / Top left */
    '+', /* 14/0x0e - Line Drawing / Bottom left |_ */
    '+', /* 15/0x0f - Line Drawing / Centre cross + */
    '>', /* 16/0x10 - Cursor Arrows / Right */
    '<', /* 17/0x11 - Cursor Arrows / Left */
    '-', /* 18/0x12 - Line Drawing / Horizontal line - */
    ' ', /* 19/0x13 - Cross box empty [ ] */
    'X', /* 20/0x14 - Cross box [X] */
    '+', /* 21/0x15 - Line Drawing / Left Tee |- */
    '+', /* 22/0x16 - Line Drawing / Right Tee -| */
    '+', /* 23/0x17 - Line Drawing / Bottom Tee _|_ */
    '+', /* 24/0x18 - Line Drawing / Top Tee -|- */
    '|', /* 25/0x19 - Line Drawing / Vertical Line | */
    '*', /* 26/0x1a - Line Drawing / Bottom right _| with resize */
    ' ', /* 27/0x1b - Scroll box - vertical */
    ' ', /* 28/0x1c - Not used */
    ' ', /* 29/0x1d - Scroll box - horizontal */
    '^', /* 30/0x1e - Cursor Arrows / Up */
    'v'  /* 31/0x1f - Cursor Arrows / Down */
};
#ifdef _UNIX
#ifdef _ME_CONSOLE
meUShort ttSpeUChars[TTSPECCHARS] =
{
    ' ', /*  0/0x00 - Undefined */
    0x25c7, /*  1/0x01 - Unicode tag - 3 byte encode */
    0x25c6, /*  2/0x02 - Unicode tag - 5 byte encode */
    ' ',    /*  3/0x03 - Checkbox left side */
    0x25A1, /*  4/0x04 - Checkbox center not selected */
    0x25A3, /*  5/0x05 - Checkbox center selected */
    ' ',    /*  6/0x06 - Checkbox right side */
    0xfffd, /*  7/0x07 - unsupported/undefined char meCHAR_UNDEF (i.e. \uFFFD) */
    0x00b7, /*  8/0x08 - backspace char, used for visible space */
    0x2192, /*  9/0x09 - Tab character */
    0x2b90, /* 10/0x0a - Visible new line character, could use 0x2b92 */
    0x2518, /* 11/0x0b - Line Drawing / Bottom right _| */
    0x2510, /* 12/0x0c - Line Drawing / Top right */
    0x250c, /* 13/0x0d - Line Drawing / Top left */
    0x2514, /* 14/0x0e - Line Drawing / Bottom left |_ */
    0x253c, /* 15/0x0f - Line Drawing / Centre cross + */
    '>',    /* 16/0x10 - Cursor Arrows / Right */
    '<',    /* 17/0x11 - Cursor Arrows / Left */
    0x2500, /* 18/0x12 - Line Drawing / Horizontal line - */
    0x2610, /* 19/0x13 - Cross box empty [ ] */
    0x2612, /* 20/0x14 - Cross box [X] */
    0x251c, /* 21/0x15 - Line Drawing / Left Tee |- */
    0x2524, /* 22/0x16 - Line Drawing / Right Tee -| */
    0x2534, /* 23/0x17 - Line Drawing / Bottom Tee _|_ */
    0x252c, /* 24/0x18 - Line Drawing / Top Tee -|- */
    0x2502, /* 25/0x19 - Line Drawing / Vertical Line | */
    0x002a, /* 26/0x1a - Line Drawing / Bottom right _| with resize */
    0x2592, /* 27/0x1b - Scroll box - vertical */
    0x0020, /* 28/0x1c - Not used */
    0x2592, /* 29/0x1d - Scroll box - horizontal */
    '^',    /* 30/0x1e - Cursor Arrows / Up */
    'v'     /* 31/0x1f - Cursor Arrows / Down */
};
#endif
#endif

void
TTdoBell(int n)
{  
#if MEOPT_CALLBACK
    register int index;
    meUInt arg ;
#endif
    
#if MEOPT_DEBUGM
    if(macbug & 0x20)
        macbug |= 0x82 ;
#endif
#if MEOPT_CALLBACK
    if((index = decode_key(ME_SPECIAL|SKEY_bell,&arg)) >= 0)
        execFuncHidden(ME_SPECIAL|SKEY_bell,index,arg) ;
    else
#endif
        if((n == 0) || ((n == 1) && quietMode))
    {
        meUByte scheme=(globScheme/meSCHEME_STYLES) ;
        pokeScreen(POKE_NOMARK+0x10,frameCur->depth,frameCur->width-6,&scheme,
                   (meUByte *) "[BELL]") ;
        frameCur->video.lineArray[frameCur->depth].endp = frameCur->width ;
        resetCursor() ;
        frameCur->mlStatus |= MLSTATUS_CLEAR ;
    }
    else
    {
        TTNbell() ;
    }
}
void
TTbell(void)
{  
    if(kbdmode == mePLAY)
    {
        kbdmode = meSTOP ;
        if(meRegCurr->force < 2)
            /* a double !force says \CG is okay so don't complain */
            mlwrite(0,(meUByte *)"Macro terminated by ringing the bell") ;
    }
    TTinflush() ;
    TTdoBell(1) ;
}

meUByte meTimerState[NUM_TIMERS]={0}; /* State of the timers, set or expired */
meTime meTimerTime[NUM_TIMERS]={0};   /* Absolute time of the timers */

#ifdef _MULTI_TIMER

#ifdef _WIN32
static INT meTimerId[NUM_TIMERS]={0}; /* Windows timer handles          */
#define meSetMultiTimer(id,tim)  (meTimerId[id]=SetTimer(baseHwnd,id,tim,(TIMERPROC)(LPVOID)NULL))
#define meKillMultiTimer(id)                                                 \
do {                                                                         \
    if(meTimerId[id] != 0)                                                   \
    {                                                                        \
        KillTimer(baseHwnd,meTimerId[id]);                                   \
        meTimerId[id] = 0;                                                   \
    }                                                                        \
} while(0)

#endif

#endif

#ifdef _SINGLE_TIMER

#ifdef _UNIX
static struct itimerval itv ;             /* itimer interval                */
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
    { NULL, 0, CURSOR_TIMER_ID}
#if MEOPT_CALLBACK
   ,{ NULL, 0, CALLB_TIMER_ID }
   ,{ NULL, 0, IDLE_TIMER_ID  }
#endif
#if MEOPT_MOUSE
   ,{ NULL, 0, MOUSE_TIMER_ID }
#endif
#if MEOPT_SOCKET
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
timerCheck(meTime tim)
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
        if (((meUInt) tim+TIMER_MIN) >= tbp->abstime)
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
            break;
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
 *              is not needed.
 * offset - offset from the current time to alarm.
 */
void
timerSet(int id, meTime tim, meInt offset)
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
            tim += TIMER_MIN - offset;
        offset = TIMER_MIN;
    }
#ifdef _MULTI_TIMER
    {
#ifdef _MULTI_NOID_TIMER
        /* each timer does not keep the id, so kill the current timer if set */
#ifdef _WIN32
#ifdef _ME_WINDOW
        if (meSystemCfg & meSYSTEM_CONSOLE)
#endif
#endif
            meKillMultiTimer(id);
#endif
        meSetMultiTimer(id,offset);
        if(tim == 0)
        {
            struct meTimeval tp;
            /* Get the absolute time */
            gettimeofday(&tp,NULL);
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
                mustSet = 1;
#endif
            }
            else
            {
                /* Search the list for the timer block */
                p = timers;
                while(p != NULL)
                {
                    if(p->next == tbp)
                    {
                        p->next = tbp->next;
                        break;
                    }
                    p = p->next;
                }
            }
        }
        
        if(tim <= 0)
        {
            struct meTimeval tp;
            /* Get the absolute time */
            gettimeofday(&tp,NULL);
            tim = ((tp.tv_sec-startTime)*1000) + (tp.tv_usec/1000) + offset;
        }
        tbp->abstime = tim;
        /*    fprintf(fplog,"Timer %d %d Set\n",tbp->id,tbp->abstime) ;*/
        /* Insertion sort the timer block into the list. */
        for(q = NULL, p = timers; p != NULL; p = p->next)
        {
            if(p->abstime > tbp->abstime)
                break;
            q = p;
        }
        
        if(q == NULL)
        {
            timers = tbp;                   /* Link to the head of the list */
#ifdef _SINGLE_TIMER
            /* Set a new timer interval */
            meSetSingleTimer(offset);
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
_timerKill(int id)
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
            meKillMultiTimer(id);
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
                    meKillSingleTimer();
                }
                else if(timers->abstime > tbp->abstime)
                    /* Must change the timer to the next timer which is a different time */
                    timerCheck(0);
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
    return (meTimerState[id] = meTimerState[id] & ~(TIMER_SET|TIMER_EXPIRED));
}

void
handleTimerExpired(void)
{
#if MEOPT_MWFRAME
    extern int commandDepth;
    
    /* if the user has changed the window focus using the OS
     * and this is the top level then change frames */
    if((frameFocus != NULL) && (frameFocus != frameCur) && (commandDepth == 0))
    {
        if(frameCur->mlLine->length > 0)
        {
            /* erase the ml line of old current frame */
            frameCur->mlLine->text[0] = '\0';
            frameCur->mlLine->length = 0;
            frameCur->mlLine->flag |= meLINE_CHANGED;
        }
        mlerase(MWCLEXEC);
        frameCur = frameFocus;
        frameFocus = NULL;
    }
#endif
    
    if(isTimerExpired(AUTOS_TIMER_ID))  /* Alarm expired ?? */
        autoSaveHandler();              /* Initiate an auto save */
#if MEOPT_CALLBACK
    if(isTimerExpired(CALLB_TIMER_ID))  /* Alarm expired ?? */
        callBackHandler();              /* Initiate any callbacks */
#endif
    if(isTimerExpired(CURSOR_TIMER_ID)) /* Alarm expired ?? */
        TThandleBlink(0);               /* Initiate a cursor blink */
#if MEOPT_SOCKET
    if(isTimerExpired(SOCKET_TIMER_ID)) /* socket connection time-out */
        ffFileOp(NULL,NULL,meRWFLAG_SOCKCLOSE|meRWFLAG_SILENT,-1);
#endif
}

void
adjustStartTime(meTime tim)
{
    meBuffer *bp ;
    meTime mstim=tim*1000 ;
    meInt ii;
#if MEOPT_CALLBACK
    meMacro *mac ;
#endif
#if (defined _CONST_TIMER) || (defined _SINGLE_TIMER)
    TIMERBLOCK *tbp ;
    tbp = timers ;
    
    while(tbp != NULL)
    {
        if(mstim >= tbp->abstime)
            tbp->abstime = 0 ;
        else
            tbp->abstime -= mstim ;
        tbp = tbp->next ;
    }
#endif
    bp  = bheadp;
    while(bp != NULL)
    {
        if(bp->autoTime >= 0)
        {
            if(mstim >= bp->autoTime)
                bp->autoTime = 0 ;
            else
                bp->autoTime -= mstim ;
        }
        bp = bp->next ;
    }
#if MEOPT_CALLBACK
    for(ii=CK_MAX ; ii<cmdTableSize ; ii++)
    {
        mac = getMacro(ii) ;
        if(mac->callback >= 0)
        {
            if(mstim >= mac->callback)
                mac->callback = 0 ;
            else
                mac->callback -= mstim ;
        }
    }
#endif
    ii = NUM_TIMERS-1 ;
    do
        if(isTimerSet(ii))
            meTimerTime[ii] -= mstim ;
    while(--ii >= 0) ;
    startTime += tim ;
}

/* translate-key support code */
#define ME_DELETE_KEY 0xffff           /* an invalid key number used to flag the key is deleted */
meTRANSKEY TTtransKey={0,250,0,ME_INVALID_KEY,NULL} ;

/* Keyboard buffer */
meUShort TTkeyBuf [KEYBUFSIZ]={0};
meUByte  TTnextKeyIdx = KEYBUFSIZ;
meUByte  TTlastKeyIdx = KEYBUFSIZ;
meUByte  TTnoKeys = 0;
meUByte  TTbreakFlag = 0;
meUByte  TTbreakCnt = TTBREAKCNT;
meUByte  TTallKeys = 0;           /* Report all keys */

meUShort TTwidthDefault=0 ;      /* Default no. of cols per frame*/
meUShort TTdepthDefault=0 ;      /* Default no. of rows per frame*/


void
TThandleBlink(int initFlag)
{
    if((cursorState < 0) || (frameCur->flags & meFRAME_NOT_FOCUS))
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
    if ((row != frameCur->cursorRow) || (col != frameCur->cursorColumn))
    {
        if((cursorState >= 0) && blinkState)
            TThideCur() ;
        frameCur->cursorRow = row ;
        frameCur->cursorColumn = col ;
    }
    if((cursorState >= 0) && blinkState)
        TTshowCur() ;
}


#ifdef _ME_FREE_ALL_MEMORY

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
translateKeyAdd(meTRANSKEY *tcapKeys, int count,int time, meUShort *key, meUShort map)
{
    if(--count < 0)
    {
        tcapKeys->map = map ;
        tcapKeys->time = time ;
    }
    else
    {
        meUShort cc=*key++ ;
        int ii ;
        
        for(ii=0 ; ii<tcapKeys->count ; ii++)
            if(tcapKeys->child[ii].key == cc)
                break;
        if(ii == tcapKeys->count)
        {
            tcapKeys->child = meRealloc(tcapKeys->child,(ii+1)*sizeof(meTRANSKEY)) ;
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
keyListToShorts(meUShort *sl, meUByte *kl)
{
    meUByte dd ;
    int ii=0 ;
    
    for(;;)
    {
        for(;;kl++)
        {
            if((dd=*kl) == '\0')
                return ii ;
            if(!isSpace(dd))
                break;
        }
        if((sl[ii++] = meGetKeyFromString(&kl))==0)
            return meABORT ;
    }
}

int
charListToShorts(meUShort *sl, meUByte *cl)
{
    meUByte cc ;
    int ii=0 ;
    
    while((cc=*cl++) != 0)
        sl[ii++] = (meUShort) cc ;
    return ii ;
}

#if MEOPT_EXTENDED
static void
translateKeyShow(meTRANSKEY *tcapKeys, meBuffer *bp, meUByte *key, int depth)
{
    int ii, nd ;
    
    if(tcapKeys->map != ME_INVALID_KEY)
    {
        meUByte buf[meBUF_SIZE_MAX] ;
        
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
            ii += meGetStringFromChar(tcapKeys->map,buf+ii);
        buf[ii++] = '"' ;
        buf[ii] = '\0' ;
        addLineToEob(bp,buf) ;
    }
    if(depth)
        key[depth++] = ' ' ;
    for(ii=0 ; ii<tcapKeys->count ; ii++)
    {
        nd = depth + meGetStringFromChar(tcapKeys->child[ii].key,key+depth);
        translateKeyShow(tcapKeys->child+ii,bp,key,nd) ;
    }
}
#endif

int
translateKey(int f, int n)
{
    static meUByte tnkyPrompt[]="translate-tcap-key code" ;
    meUShort c_from[20], c_to[20];
    meUByte buf[128];
    int ii;
    
#if MEOPT_EXTENDED
    if(n == 0)
    {
        meWindow *wp;
        meBuffer *bp;
    
        if((wp = meWindowPopup(NULL,BtranskeyN,BFND_CREAT|BFND_CLEAR|WPOP_USESTR,NULL)) == NULL)
            return meFALSE;
        bp = wp->buffer;
        
        translateKeyShow(&TTtransKey,bp,buf,0);
        bp->dotLine = meLineGetNext(bp->baseLine);
        bp->dotOffset = 0;
        bp->dotLineNo = 0;
        meModeClear(bp->mode,MDEDIT);    /* don't flag this as a change */
        meModeSet(bp->mode,MDVIEW);      /* put this buffer view mode */
        resetBufferWindows(bp);          /* Update the window */
        mlerase(MWCLEXEC);	         /* clear the mode line */
    }
    else
#endif
    {
        meStrcpy(tnkyPrompt+19,"code");
        if((meGetString(tnkyPrompt,0,0,buf,128) <= 0) ||
           ((ii=keyListToShorts(c_from,buf)) <= 0))
            return meFALSE;

        if(n == -1)
            c_to[0] = ME_INVALID_KEY;
        else
        {
            if(f == meFALSE)
                n = TTtransKey.time;
            meStrcpy(tnkyPrompt+19,"to");
            if((meGetString(tnkyPrompt,0,0,buf,128) <= 0) ||
               ((f=keyListToShorts(c_to,buf)) < 0))
                return meFALSE;
            if(f == 0)
                c_to[0] = ME_DELETE_KEY;
        }
        translateKeyAdd(&TTtransKey,ii,n,c_from,*c_to);
    }
    return meTRUE;
}

meUShort
TTgetc(void)
{
    meTRANSKEY *tt, *tk;
    int ci, ki, ll, kk, kl;
#ifdef _USE_NCURSES
    int bp=0;
#endif
    meUShort cc;
    
    do {
        if(!TTahead())
            TTwaitForChar();
        
        tk = NULL;
        if((ci = TTtransKey.count) > 0)
        {
            tt = &TTtransKey;
            ki = TTnextKeyIdx;
            ll = 0;
            for(;;)
            {
                if(!ki)
                    ki = KEYBUFSIZ;
                cc = TTkeyBuf[--ki];
                while((--ci >= 0) && (tt->child[ci].key != cc))
                    ;
                if(ci < 0)
                    break;
                tt = tt->child+ci;
                ll++;
                if(tt->map != ME_INVALID_KEY)
                {
                    tk = tt;
                    kk = ki;
                    kl = ll;
                }
                if((ci = tt->count) == 0)
                    break;
                if(ll == TTnoKeys)
                {
                    /* Wait for a quarter a second to see if any more keys are available */
                    /* Only gocha here is that preferably you want an interuptable wait
                     * therefore we must set the TTnoKeys to 0 so we only interrupt on
                     * a receipt of a new key. Must remember to reset it as well.
                     */
                    TTnoKeys = 0;
                    TTsleep(tt->time,1,NULL);
                    if((TTnoKeys += ll) == ll)
                        break;
                }
            }
            if(tk != NULL)
            {
                ll = TTnoKeys - kl;
                TTnoKeys = ll;
                ki = TTnextKeyIdx;
#ifdef _USE_NCURSES
                if(tt->map == (ME_SPECIAL|SKEY_bpe))
                {
                    if(bp)
                    {
                        meUByte *dd;
                        bp--;
                        if((dd = killAddNode(bp)) != NULL)
                        {
                            memcpy(dd,ffbuf+1,bp);
                            dd[bp] = '\0';
                        }
                        thisflag = meCFKILL;
                        bp = 0;
                        if(!ki)
                            ki = KEYBUFSIZ;
                        TTkeyBuf[--ki] = tk->map;
                        TTnoKeys++;
                    }
                }
                else if(bp)
                {
                    /* in the middle of a bracketed paste, these 'keys' are part of the paste so don't tanslate */
                    if((kl == 1) && (tt->map == (ME_SPECIAL|SKEY_return)))
                    {
                        ffbuf[bp++] = meCHAR_NL;
                        TTnextKeyIdx--;
                    }
                    else
                    {
                        do
                            ffbuf[bp++] = TTkeyBuf[--TTnextKeyIdx];
                        while(--kl);
                    }
                    continue;
                }
                else if(tt->map == (ME_SPECIAL|SKEY_bps))
                {
                    /* this is the start of a paste, all the following text, until the paste end,
                     * should be treated as part of the 'clipboard', so no translation */
                    bp = 1;
                    TTnextKeyIdx -= kl;
                    /* always new kill, don't want to glue 'em together */
                    killInit(0);
                    continue;
                }
                else if((tt->map == (ME_SPECIAL|SKEY_mouse_move))
#ifdef _ME_WINDOW
                   && (meSystemCfg & meSYSTEM_CONSOLE)
#endif
                   )
                {
                    int iv[3], ix=2;
                    if(cc == '<')
                    {
                        do {
                            iv[ix]=0;
                            while(--ll >= 0)
                            {
                                if(!kk)
                                    kk = KEYBUFSIZ;
                                if(((cc = TTkeyBuf[--kk]) < '0') || (cc > '9'))
                                    break;
                                iv[ix] = (iv[ix]*10) + cc-'0';
                            }
                        } while((--ix >= 0) && (ll >= 0) && (cc == ';'));
                        if(ix < 0)
                        {
                            if((cc & 0xdf) != 'M')
                                ix = 0;
                            /* fprintf(fplog,"MOUSE-SGR: %d %d %d %d\n",iv[2],iv[1],iv[0],mouseState);*/
                        }
                        else if(ll < 0)
                            ll = 0;
                    }
                    else if((ll -= 3) < 0)
                        ll = 0;
                    else
                    {
                        do {
                            if(!kk)
                                kk = KEYBUFSIZ;
                            iv[ix] = TTkeyBuf[--kk]-32;
                        } while(--ix >= 0);
                        /* fprintf(fplog,"MOUSE-X10: %d %d %d %d\n",iv[2],iv[1],iv[0],mouseState);*/
                        if(((iv[2] & 3) == 3) && mouseState)
                        {
                            cc = 'm';
                            iv[2] = (iv[2] & ~0x03) | ((mouseState == 4) ? 2:((mouseState&3)-1));
                        }
                        else
                            cc = 'M';
                    }
                    TTnoKeys = ll;
                    if(ix < 0)
                    {
                        mouse_X = iv[1]-1;
                        mouse_Y = iv[0]-1;
                        
                        if(iv[2] & 192)
                        {
                            if(iv[2] & 190)
                                cc = 0;
                            else
                            {
                                cc = ME_SPECIAL | ((iv[2] & 1) ? SKEY_mouse_wdown:SKEY_mouse_wup);
                                ci = 1;
                            }
                        }
                        else
                        {
                            ix = (iv[2]+1) & 3;
                            if(iv[2] & 32)
                            {
                                if(ix)
                                {
                                    cc = ME_SPECIAL | (SKEY_mouse_move+ix);
                                    mouseState |= 1 << (ix-1);
                                }
                                else
                                {
                                    cc = ME_SPECIAL | SKEY_mouse_move;
                                    mouseState = 0;
                                }
                                ci = (TTallKeys & 0x1);
                            }
                            else if(ix)
                            {
                                ix--;
                                if(cc == 'm')
                                {
                                    mouseState &= ~(1 << ix);
                                    cc = ME_SPECIAL | (SKEY_mouse_drop_1+ix);
                                }
                                else
                                {
                                    mouseState |= 1 << ix;
                                    cc = ME_SPECIAL | (SKEY_mouse_pick_1+ix);
                                }
                                ci = 1;
                            }
                        }
                        if(iv[2] & 4)
                            cc |= ME_SHIFT;
                        if(iv[2] & 8)
                            cc |= ME_ALT;
                        if(iv[2] & 16)
                            cc |= ME_CONTROL;
                        
                        /* Are we after all movements or mouse-move bound ?? */
                        if((cc & ME_SPECIAL) && (ci || (!TTallKeys && (decode_key(cc,(unsigned int *) iv) != -1))))
                        {
                            if(!ki)
                                ki = KEYBUFSIZ;
                            TTkeyBuf[--ki] = cc;
                            mouseKeyState = cc & (ME_SHIFT|ME_CONTROL|ME_ALT);
                            TTnoKeys++;
                        }
                    }
                }
                else
#endif
                    if(tk->map != ME_DELETE_KEY)
                {
                    if(!ki)
                        ki = KEYBUFSIZ;
                    TTkeyBuf[--ki] = tk->map;
                    TTnoKeys++;
                }
                /* if ki == TTnextKeyIdx then no key has been added yet (i.e. unbound mouse-move) and any remaining keys still need translating */
                if(ki != TTnextKeyIdx)
                    tk = NULL;
                while(--ll >= 0)
                {
                    if(!ki)
                        ki = KEYBUFSIZ;
                    if(!kk)
                        kk = KEYBUFSIZ;
                    TTkeyBuf[--ki] = TTkeyBuf[--kk];
                }
                TTlastKeyIdx = ki;
            }
#ifdef _USE_NCURSES
            else if(bp)
            {
                /* In the middle of a bracketed paste - one gotcha, a new-line is sent as a 0x0d (i.e. 'return' key)
                 * rather than 0x0a which is what ME uses as a new-line char (i.e. meCHAR_NL) */
                meUByte cc;
                if(!TTnextKeyIdx)
                    TTnextKeyIdx = KEYBUFSIZ;
                if((cc = TTkeyBuf[--TTnextKeyIdx]) == 0x0d)
                    cc = meCHAR_NL;
                ffbuf[bp] = cc;
                if(bp > (meFIOBUFSIZ>>1))
                {
                    /* Unlikely to happen but if very large paste then avoid ffbuf overrun */
                    meUByte *dd;
                    if((dd = killAddNode(bp)) != NULL)
                    {
                        memcpy(dd,ffbuf+1,bp);
                        dd[bp] = '\0';
                    }
                    thisflag = meCFKILL;
                    bp = 0;
                }
                bp++;
                TTnoKeys--;
            }
#endif
        }
    } while(!TTnoKeys ||
#ifdef _USE_NCURSES
            bp ||
#endif
            (tk != NULL));
    
    if(!TTnextKeyIdx)
        TTnextKeyIdx = KEYBUFSIZ;
    cc = TTkeyBuf[--TTnextKeyIdx];
    TTnoKeys--;
    /* remove the abort flag if we are waiting for the key */
    if((cc == breakc) && TTbreakFlag)
        TTbreakFlag = 0;
    return cc ;
}

#if MEOPT_CALLBACK
/*
 * doIdlePickEvent
 * Handle the idle event.
 */
void
doIdlePickEvent(void)
{
    register int index;
    meUInt arg;

    /* See if there is any binding on the idle event */
    if((index=decode_key(ME_SPECIAL|SKEY_idle_pick,&arg)) != -1)
    {
        /* Execute the idle-pick key */
        execFuncHidden(ME_SPECIAL|SKEY_idle_pick,index,arg) ;
        
        /* Now see if theres a idle-time event, if so start the timer */
        if(decode_key(ME_SPECIAL|SKEY_idle_time, &arg) != -1)
            timerSet(IDLE_TIMER_ID,-1,idleTime);
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
    meUInt arg;

    if(isTimerActive(IDLE_TIMER_ID))
        /* Kill off the timer */
        timerKill(IDLE_TIMER_ID) ;
    
    /* do the idle-drop if bound */
    if((index=decode_key(ME_SPECIAL|SKEY_idle_drop,&arg)) != -1)
        /* Execute the idle-pick key */
        execFuncHidden(ME_SPECIAL|SKEY_idle_drop,index,arg) ;
    meTimerState[IDLE_TIMER_ID] = 0;
}
#endif

/*
 * addKeyToBuffer
 * Add a new keyboard character to the keyboard buffer.
 */
void
addKeyToBuffer(meUShort cc)
{
    if(cc == breakc)
    {
#if MEOPT_DEBUGM
        if(macbug & 0x10)
        {
            macbug |= 0x82 ;
            return ;
        }
#endif
        TTbreakFlag = 1 ;
    }
#if MEOPT_MOUSE
    /* Reset the mouse context if mouse interaction is outstanding. 
     * This should only be timer keys
     * Kill off the timer */
    timerKill(MOUSE_TIMER_ID);
#endif
#if MEOPT_CALLBACK
    /* Flag the end of the idle if necessary */
    if(meTimerState[IDLE_TIMER_ID] != 0)
        doIdleDropEvent() ;
#endif
    
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

/*
 * Add the key to the buffer if it is not already in the buffer. We only check
 * the last key event. This is typically used for the mouse events which may
 * be many and we only need a single mouse move event.
 */
void 
addKeyToBufferOnce (meUShort cc)
{
    /* Add the key to the buffer if the buffer is empty or the key code is
     * different from the last key code. */
    if ((TTnoKeys == 0) || (TTkeyBuf [TTlastKeyIdx] != cc))
        addKeyToBuffer (cc);
}

#if MEOPT_MOUSE
/* when TTallKeys is set to 1 all mouse move events are added to the key stack
 * even if the key is not bound. Due to the speed at which windows generates
 * mouse move events, after exiting an OSD dialog mouse move keys can still be
 * in the buffer and this will generate problems as the keys are not bound (e.g.
 * breaks the main menus repeat undo function) - remove the extras.
 */
void
TTallKeysFlush(void)
{
    meUByte nidx ;
    meInt cc ;
    
    while(TTnoKeys > 0)
    {
        if(!TTnextKeyIdx)
            nidx = KEYBUFSIZ ;
        else
            nidx = TTnextKeyIdx - 1 ;
        cc = (TTkeyBuf[nidx] & (ME_SPECIAL|0x0ff)) ;
        if((cc < (ME_SPECIAL|SKEY_mouse_move)) || (cc > (ME_SPECIAL|SKEY_mouse_move_5)))
            break;
        TTnextKeyIdx = nidx ;
        TTnoKeys-- ;
    }
}
#endif

#ifdef _ME_CONSOLE
#ifdef _TCAP
#ifndef _USETPARM
#define MENEED_TPARM 1
#endif /* _USETPARM */
#endif /* _TCAP */
#endif /* _ME_CONSOLE */
#ifndef MENEED_TPARM
#define MENEED_TPARM MEOPT_PRINT
#endif /* MENEED_TPARM */

#if MENEED_TPARM
#ifdef _STDARG
char *
meTParm(char *str, ...)
#else
char *
meTParm(char *str, meInt arg)
#endif
{
    static char *tbuff=NULL;
    static meInt tbuffLen=0;
    meInt stack[10], pInc=0, ii, stackDep=0, len=0;
    char cc, *ss;
    
    /* See man terminfo for docs on Parameterized Strings format.
     * Currently does not support: %l (strlen(pop) - stack only supports ints)
     *   %P? & %g? set and get static and dynamic vars 
     *   %i is incomplete, inc's all parameters
     */
    for(;;)
    {
        if(len >= tbuffLen)
        {
            tbuff = meRealloc(tbuff,tbuffLen+32);
            if(tbuff == NULL)
            {
                tbuffLen = 0;
                return (char *) "";
            }
            tbuffLen += 16;
        }
        if((cc=*str++) == '\0')
            break;
        else if(cc == '%')
        {
            switch((cc=*str++))
            {
                /* miscellaneous codes */
            case '\0':
                str--;
                break;
            case '%':
                tbuff[len++] = '%';
                break;
                
                /* codes which push values to the stack */
            case '\'':
                if((cc=*str++) != '\0')
                {
                    stack[stackDep++] = (meInt) cc;
                    if(*str++ != '\'')
                        str--;
                }
                else
                    str--;
                break;
            case '{':
                ii = 0;
                while((cc=*str++),isDigit(cc))
                    ii = (ii * 10) + (cc - '0');
                /* minimal error handling */
                if(cc != '}')
                    str--;
                stack[stackDep++] = ii;
                break;
            case 'p':
                /* get the nth parameter */
                {
#ifdef _STDARG
                    va_list ap;
                    int pno;
                    va_start(ap,str);
                    pno = *str++ - '0';
                    do {
                        ii = va_arg(ap,meInt);
                    } while(--pno > 0);
                    va_end (ap);
#else
                    register meInt *ap;
                    ap = &arg;
                    ii = ap[*str++ - '1'];
#endif
                    /* Note - docs state %i should only effect the first 2 parameters, but not seen it used by anything with more than 2 */ 
                    stack[stackDep++] = ii+pInc;
                    break;
                }
                
                /* codes which operate on values in the stack */
            case '+':
                if(stackDep > 1)
                {
                    stackDep--;
                    stack[stackDep-1] = stack[stackDep-1] + stack[stackDep];
                }
                break;
            case '-':
                if(stackDep > 1)
                {
                    stackDep--;
                    stack[stackDep-1] = stack[stackDep-1] - stack[stackDep];
                }
                break;
            case '*':
                if(stackDep > 1)
                {
                    stackDep--;
                    stack[stackDep-1] = stack[stackDep-1] * stack[stackDep];
                }
                break;
            case '/':
                if(stackDep > 1)
                {
                    stackDep--;
                    stack[stackDep-1] = stack[stackDep-1] / stack[stackDep];
                }
                break;
            case 'm':
                if(stackDep > 1)
                {
                    stackDep--;
                    stack[stackDep-1] = stack[stackDep-1] % stack[stackDep];
                }
                break;
            case '&':
                if(stackDep > 1)
                {
                    stackDep--;
                    stack[stackDep-1] = stack[stackDep-1] & stack[stackDep];
                }
                break;
            case '|':
                if(stackDep > 1)
                {
                    stackDep--;
                    stack[stackDep-1] = stack[stackDep-1] | stack[stackDep];
                }
                break;
            case '^':
                if(stackDep > 1)
                {
                    stackDep--;
                    stack[stackDep-1] = stack[stackDep-1] ^ stack[stackDep];
                }
                break;
            case '=':
                if(stackDep > 1)
                {
                    stackDep--;
                    stack[stackDep-1] = stack[stackDep-1] == stack[stackDep];
                }
                break;
            case '>':
                if(stackDep > 1)
                {
                    stackDep--;
                    stack[stackDep-1] = stack[stackDep-1] > stack[stackDep];
                }
                break;
            case '<':
                if(stackDep > 1)
                {
                    stackDep--;
                    stack[stackDep-1] = stack[stackDep-1] < stack[stackDep];
                }
                break;
            case 'A':
                if(stackDep > 1)
                {
                    stackDep--;
                    stack[stackDep-1] = stack[stackDep-1] && stack[stackDep];
                }
                break;
            case 'O':
                if(stackDep > 1)
                {
                    stackDep--;
                    stack[stackDep-1] = stack[stackDep-1] || stack[stackDep];
                }
                break;
            case '!':
                if(stackDep > 0)
                    stack[stackDep-1] = !stack[stackDep-1];
                break;
            case '~':
                if(stackDep > 0)
                    stack[stackDep-1] = ~stack[stackDep-1];
                break;
            case 'i':
                pInc++;
                break;
            
            case '?':
                /* Start of IF statement %?<test>%t<then>%e<else>%; */
                /* Currently parsing so ignore this here */
                break;
            case 't':
                /* Then of IF - Currently parsing so run test and just to else if false */
                if(stackDep > 0)
                {
                    stackDep--;
                    if(!stack[stackDep])
                    {
                        /* test false - skip to matching %e or %; */
                        ii = 0;
                        while((cc=*str++) != 0) 
                        {
                            if(cc == '%')
                            {
                                if((cc=*str++) == 0) 
                                    break;
                                if((cc == 'e') || (cc == ';'))
                                {
                                    if(--ii < 0)
                                        break;
                                }
                                else if(cc == '?')
                                    ii++;
                            }
                        }
                    }
                }
                break;
            case 'e':
                /* Else of IF Currently parsing so %t was true - skip else */
                ii = 0;
                while((cc=*str++) != 0) 
                {
                    if(cc == '%')
                    {
                        if((cc=*str++) == 0) 
                            break;
                        if(cc == ';')
                        {
                            if(--ii < 0)
                                break;
                        }
                        else if(cc == '?')
                            ii++;
                    }
                }
                break;
            case ';':
                /* End of IF - Currently parsing so ignore this here */
                break;

                /* codes which pop a value from the stack and add it to the output */
            case 'c':
                if(stackDep > 0)
                {
                    stackDep--;
                    tbuff[len++] = (char) stack[stackDep];
                }
                break;
            case ':': case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
                ss = str-2;
                do
                    cc = *str++;
                while((cc != '\0') && (cc != 'd') && (cc != 'o') && (cc != 's') &&
                      (cc != 'x') && (cc != 'X'));
                if(cc == '\0')
                    break;
                goto arg_print;
            case 'd':
            case 'o':
            case 's':
            case 'x':
            case 'X':
                ss = str-2;
arg_print:
                if(cc == 's')
                    /* don't handle string args so change to 'd' */
                    str[-1] = 'd';
                cc = *str;
                *str = '\0';
                if(!stackDep)
                {
                    /* an argument has not been loaded into the stack yet -
                     * occurs with cygwin color terminal. assume the first arg
                     * is required */
#ifdef _STDARG
                    va_list ap;
                    va_start(ap,str);
                    stack[0] = va_arg(ap, meInt);
                    va_end (ap);
#else
                    stack[0] = ((meInt *)&arg)[0];
#endif
                }
                else
                    stackDep--;
                sprintf(tbuff+len,ss,stack[stackDep]);
                len += strlen(tbuff+len);
                *str = cc;
                break;
            }
        }
        else
            tbuff[len++] = cc;
    }
    tbuff[len] = '\0';
    return tbuff;
}
#endif
