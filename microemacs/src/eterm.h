/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * eterm.h - Terminal interface definitions.
 *
 * Copyright (C) 1994-2002 JASSPA (www.jasspa.com)
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
 * Created:     1994
 * Synopsis:    Terminal interface definitions.
 * Authors:     Steven Phillips & Jon Green
 * Notes:
 *     every system must provide the following variables / constants
 *
 *        TTmrow    - maximum number of rows
 *        TTnrow    - current number of rows
 *        TTmcol    - maximum number of columns
 *        TTncol    - current number of columns
 *        TTmargin  - minimum margin for extended lines
 *        TTscrsiz  - size of scroll region
 *        TTpause   - # times thru update to pause
 *
 *     every system must provide the following functions
 *
 *        TTstart   - Creates the terminal (called once at start)
 *        TTend     - Destroyes the terminal (called once at end)
 *        TTopen    - opens the terminal called after any shell call.
 *        TTclose   - closes the terminal called before any shell call.
 *        TTwaitForChar - gets a character
 *        TTflush   - flushes the output terminal
 *        TTmove    - most the cursor to a given position
 *        TThideCur - hide the cursor
 *        TTshowCur - show the cursor
 *        TTNbell   - Do a noisy beep
 *
 *     The following must be provided if TYPEAH=TRUE
 *
 *        TTahead   - any characters waiting ?
 *
 *     TTbreakTest(n) - Test for a break from the keyboard. The numeric argument
 *     'n' is set to 1 if the break test may include 'display update' messages,
 *     or 0 if it is only allowed to collect user input. Note that not all platforms
 *     support this argument.
 */
#ifndef __TERMIOH
#define __TERMIOH

/**************************************************************************
* Common variables, constants and defines                                 *
**************************************************************************/
/* Standard special keys */
#define	BELL	0x07			/* a bell character		 */
#define	TAB	0x09			/* a tab character		 */

/* Standard key input definitions, found in termio.c */
typedef struct meTRANSKEY {
    int      count ;
    int      time ;
    meUShort key ;
    meUShort map ;
    struct meTRANSKEY *child ;
} meTRANSKEY ;

#define KEYBUFSIZ      128              /* Length of keyboard buffer    */
#define TTBREAKCNT     100              /* Initial ttbreakCnt value     */
#define TTSPECCHARS    32               /* End of the special chars     */

extern meTRANSKEY TTtransKey ;          /* A translated key             */
extern int        TTcurr;               /* Windows current row.         */
extern int        TTcurc;               /* Windows current column       */
extern int        TTfocus;              /* Is window current focus ?    */
extern meUShort   TTkeyBuf[KEYBUFSIZ] ; /* Key beuffer/pending keys     */
extern meUByte ttSpeChars [TTSPECCHARS];/* Special characters           */
extern meUByte    TTnextKeyIdx ;        /* Circular buffer index        */
extern meUByte    TTlastKeyIdx ;        /* Key buffer - last index.     */
extern meUByte    TTnoKeys ;            /* Number of keys in buffer     */
extern meUByte    TTbreakFlag ;         /* Break outstanding on input   */
extern meUByte    TTbreakCnt ;          /* Number breaks outsanding     */
extern meUByte    TTallKeys;            /* Report all keys              */

extern meUShort   TTgetc APRAM((void)) ;
extern void       TThandleBlink APRAM((int initFlag)) ;
extern void       TTmove APRAM((int r, int c)) ;
#define TTinflush()   (TTahead(),TTlastKeyIdx=TTnextKeyIdx,TTnoKeys=0)
extern void       addKeyToBuffer APRAM((meUShort cc)) ;
extern void       doIdlePickEvent APRAM((void)) ;
extern void       setAlarm APRAM((meInt absTime, meInt offTime)) ;


#ifdef _UNIX

/* Signal definitions */
#ifdef _POSIX_SIGNALS
#define meSigHold()    ((++meSigLevel == 0) ? sigprocmask(SIG_BLOCK, &meSigHoldMask, NULL):0)
#define meSigRelease() ((--meSigLevel  < 0) ? sigprocmask(SIG_UNBLOCK, &meSigHoldMask, NULL):0)
/* The prototype of a signal in POSIX */
#define SIGNAL_PROTOTYPE  int sig
#else
/* BSD Signals */
#ifdef _BSD_SIGNALS
#define meSigHold()    ((++meSigLevel == 0) ? sigsetmask(meSigHoldMask):0)
#define meSigRelease() ((--meSigLevel  < 0) ? sigsetmask(0):0)
/* The prototype of a signal call in BSD */
#define SIGNAL_PROTOTYPE  int sig, int code, struct sigcontext *context

/* Remove any signal definition */
#ifdef signal
#undef signal
#endif /* signal */

/* Redefined for the safe BSD signal functions. We should be able to use a
 * regular signal() however is is easier to just assume all signals are broken
 * and use the safe ones !! We hope that the compiler will optimise structure
 * re-use when we have a series of these, but we are not too worried either
 * way !! */
#ifdef _BSD_42
#define signal(x,y)        \
do {                       \
    struct sigvec sa;      \
    sa.sv_mask = 0;        \
    sa.sv_onstack = 0;     \
    sa.sv_handler = y;     \
    sigvec (x, &sa, NULL); \
} while (0)
#endif /* _BSD_42 */
#ifdef _BSD_43
#define signal(x,y)        \
do {                       \
    struct sigvec sa ;     \
    sa.sv_mask = 0 ;       \
    sa.sv_flags = 0 ;      \
    sa.sv_handler = y ;    \
    sigvec (x, &sa, NULL); \
} while (0)
#endif /* _BSD_43 */
#else /* ! (_BSD_SIGNALS || _POSIX_SIGNALS) */
/* No hold mechanism */
#define meSigHold()    /* */
#define meSigRelease() /* */
/* The prototype of any other signal */
#define SIGNAL_PROTOTYPE  int sig
#endif /* _BSD_SIGNALS */
#endif /* _POSIX_SIGNALS */

/* Timer definitions */
extern void sigAlarm(SIGNAL_PROTOTYPE) ;
#define	_SINGLE_TIMER 1		/* Only one itimer available  */
#define	TIMER_MIN     10	/* Only one itimer available  */

/* Additional UNIX externals */
extern meUShort TTmrow, TTnrow, TTsrow, TTmcol, TTncol, TTmargin, TTscrsiz ;
extern char  *CM, *CL ;

#ifndef _CYGWIN
/* Following are functions used by termcap & an xterm */
extern	int   tputs(char *, int, int (*)(int)) ;
extern	char *tgoto(char *, int, int ) ;
#endif

#define TTNbell()      (putchar(BELL),fflush(stdout))
#define TTdieTest()    if(alarmState & meALARM_DIE) meDie()
#define TTbreakTest(x) ((--TTbreakCnt == 0) &&                         \
                       (((alarmState & meALARM_DIE) && meDie()) ||     \
                        (TTbreakCnt=TTBREAKCNT,TTahead(),TTbreakFlag)))
extern void TTwaitForChar(void) ;
extern void TTsleep(int msec, int intable) ;
#if TYPEAH
extern int  TTahead(void) ;
#endif

/* Following are termcap function */
extern int TCAPstart(void) ;
extern int TCAPopen(void) ;
extern int TCAPclose(void) ;
extern void TCAPmove(int row, int col);
#define TCAPputc(c)    putchar(c)
#define TCAPflush()    fflush(stdout)
extern void TCAPhideCur(void) ;
extern void TCAPshowCur(void) ;
extern void TCAPhandleBlink(void) ;

#if COLOR
extern int  TCAPaddColor(meCOLOR index, meUByte r, meUByte g, meUByte b) ;
extern void TCAPschemeSet(meSCHEME scheme) ;
extern void TCAPschemeReset(void) ;
#endif

#ifdef _XTERM
/* Display information */

typedef struct
{
    Display  *xdisplay ;                /* the x display */
    Window    xwindow ;                 /* the x window */
    GC        xgc ;                     /* the x draw style */
    XGCValues xgcv;                     /* X colour values */
    int       fwidth ;                  /* Font width in pixels */
    int       fdepth ;                  /* Font depth in pixels */
    int       fhwidth ;                 /* Font half width in pixels */
    int       fhdepth ;                 /* Font half depth in pixels */
    int       fadepth ;                 /* Font up-arrow depth in pixels */
    int       ascent ;                  /* Font ascent */
    int       descent ;                 /* Font descent */
    int       underline ;               /* The underline position */
    meUByte  *fontName;                 /* The current Font name */
    Font      fontId;                   /* Font X id */
    Font      fontTbl[meFONT_MAX];      /* table of font X ids for diff styles */
    meUByte  *fontPart[meFONT_MAX];     /* pointers to parts in fontname */
    meUByte   fontFlag[meFONT_MAX];     /* Font loaded ? */
    meUByte   fcol;                     /* Foreground color */
    meUByte   bcol;                     /* Background color */
    meUByte   font;                     /* Font style */
} meCellMetrics;                        /* The character cell metrics */

extern meCellMetrics mecm ;

/* Set of macros to interchange pixel and character spaces coordinates */
#define colToClient(x)    (mecm.fwidth*(x))                /* Convert column char => pixel */
#define rowToClient(y)    ((mecm.fdepth*(y))+mecm.ascent)  /* Convert row char => pixel (for text drawing) */
#define rowToClientTop(y) (mecm.fdepth*(y))                /* Convert row char => pixel (top of row) */
#define clientToRow(y)    ((y)/mecm.fdepth)                /* Convert row pixel => char */
#define clientToCol(x)    ((x)/mecm.fwidth)                /* Convert column pixel => char */
#define XTERMstringDraw(col,row,str,len)                                           \
do {                                                                               \
    XDrawImageString(mecm.xdisplay,mecm.xwindow,mecm.xgc,(col),(row),(char *)(str),(len)); \
    if(mecm.font & meFONT_UNDERLINE)                                               \
        XDrawLine(mecm.xdisplay,mecm.xwindow,mecm.xgc,(col),(row)+mecm.underline,  \
                  (col)+colToClient(len)-1,(row)+mecm.underline);                  \
} while(0)

extern void XTERMschemeSet(meSCHEME scheme) ;
extern int  XTERMstartStage2(void);
extern void XTERMinitMouse(void);
extern void XTERMmove(int r, int c) ;
extern void XTERMclear(void) ;
extern void XTERMhideCur(void) ;
extern void XTERMshowCur(void) ;
extern int  XTERMaddColor(meCOLOR index, meUByte r, meUByte g, meUByte b) ;
extern void XTERMSpecialChar(int x, int y, meUByte cc) ;
extern void XTERMPaint(int srow, int scol, int erow, int ecol) ;

extern void TTsetBgcol(void) ;
extern int  TTstart(void) ;
#define TTstartStage2() ((!(meSystemCfg & meSYSTEM_CONSOLE)) ? XTERMstartStage2():0)

#define TTend()     ((meSystemCfg & meSYSTEM_CONSOLE) ? TCAPclose():0)
#define TTopen()    ((meSystemCfg & meSYSTEM_CONSOLE) ? TCAPopen():0)
#define TTclose()   ((meSystemCfg & meSYSTEM_CONSOLE) ? TCAPclose():0)
#define TTflush()   ((meSystemCfg & meSYSTEM_CONSOLE) ? TCAPflush():(XFlush(mecm.xdisplay),1))
#define TThideCur() ((meSystemCfg & meSYSTEM_CONSOLE) ? TCAPhideCur():XTERMhideCur())
#define TTshowCur() ((meSystemCfg & meSYSTEM_CONSOLE) ? TCAPshowCur():XTERMshowCur())
#define TTaddColor(i,r,g,b) ((meSystemCfg & meSYSTEM_CONSOLE) ? TCAPaddColor(i,r,g,b):XTERMaddColor(i,r,g,b))

/* Some extra function, only available to xterms */
extern void TTtitleText (meUByte *str) ;
extern void TTdepth(int y) ;
extern void TTwidth(int x) ;

#ifdef _CLIPBRD
extern void TTgetClipboard(void);
extern void TTsetClipboard(void);
#endif

#else
/* If no xterm then just use the termcap ones */
#define TTstart    TCAPstart
#define TTstartStage2()                 /* No stage 2 startup */
#define TTopen     TCAPopen
#define TTclose    TCAPclose
#define TTend      TCAPclose
#define TTflush    TCAPflush
#define TThideCur  TCAPhideCur
#define TTshowCur  TCAPshowCur
#define TTaddColor TCAPaddColor
#define TTsetBgcol()

#endif /* _XTERM */

#ifdef _CLIENTSERVER
extern void TTopenClientServer(void) ;
extern void TTkillClientServer(void) ;
extern int  TTconnectClientServer(void) ;
extern void TTsendClientServer(meUByte *) ;
#endif

#endif /* _UNIX */


#ifdef _WIN32

#define	_MULTI_TIMER 1		     /* Multiple timers can be set at once */
#ifdef _WINCON
/* strange but true, when running as a console app, you can't assign ids
 * so effectively when a timer pops you have to work out which one has
 * if any. Bummer is you can kill them either! Thank you Bill */
#define	_MULTI_NOID_TIMER 1	     /* Multi timer but no ids */
#endif
#define	TIMER_MIN    1		     /* The minimum timer allowed */
/* executable extension list, in reverse order of dos priority - Must have the end NULL
 * included 4dos's btm files for completeness */
#define noExecExtensions 4
extern meUByte *execExtensions[noExecExtensions] ;

extern HWND ttHwnd ;                 /* This is the window handle */
extern RECT ttRect ;                 /* Area of screen to update */
extern int fdepth, fwidth ;
extern int ascent ;

extern meUShort TTmrow, TTnrow, TTsrow, TTmcol, TTncol, TTmargin, TTscrsiz ;

extern meUInt *colTable ;

extern int TTstart(void) ;
extern int TTstartStage2(void);
extern int  TTopen(void) ;
#define TTclose()

#ifdef _WINCON
extern BOOL ConsolePaint(void) ;
extern void ConsoleDrawString(meUByte *s, WORD wAttribute, int x, int y, int len) ;
#define TTcolorSet(f,b) ((f) | ((b) << 4))
#define TTschemeSet(scheme) \
TTcolorSet(colTable[meStyleGetFColor(meSchemeGetStyle(scheme))], \
           colTable[meStyleGetBColor(meSchemeGetStyle(scheme))])


extern int TTend(void) ;
#define TTflush()   ((meSystemCfg & meSYSTEM_CONSOLE) ? ConsolePaint () : UpdateWindow (ttHwnd))
#else
#define TTend()
/*
 * TTflush()
 * Because of the delayed nature of the windows display perform the
 * flush by processing any outstanding PAINT messages ONLY. This
 * will update the display leaving the remaining messages on the
 * input queue.
 */
#define TTflush()   UpdateWindow (ttHwnd)
#endif

extern void TTtitleText (meUByte *str) ;

extern void TTwaitForChar(void) ;
extern void TThideCur(void) ;
extern void TTshowCur(void) ;
#define TTNbell()  MessageBeep(0xffffffff)
extern void TTdepth(int y) ;
extern void TTwidth(int x) ;

#ifdef _CLIPBRD
extern void TTgetClipboard(void);
extern void TTsetClipboard(void);
#endif

#ifdef _CLIENTSERVER
extern int  ttServerToRead;
extern void TTopenClientServer(void) ;
extern void TTkillClientServer(void) ;
extern int  TTcheckClientServer(void) ;
extern int  TTconnectClientServer(void) ;
extern void TTsendClientServer(meUByte *) ;
#endif


extern void TTsleep(int msec, int intable) ;
#if TYPEAH
extern int TTahead(void) ;
#endif
extern int TTaheadFlush(void);
#define TTdieTest()
#define TTbreakTest(x) ((TTbreakCnt-- ==  0) ? (TTbreakCnt=TTBREAKCNT,((x==0)?TTahead():TTaheadFlush()),TTbreakFlag):(TTbreakFlag))

extern int  TTaddColor(meCOLOR index, meUByte r, meUByte g, meUByte b);
extern void TTsetBgcol(void);
extern void TTcolour (int fg, int bg);

extern void TTputs(int row, int col, int len) ;

/* init the rect to invalid big areas, they are LONGS so this is okay */
#define TTinitArea()        (ttRect.left=0xfffff,ttRect.right=-1,ttRect.top=0xfffff,ttRect.bottom=-1)

#define TTsetArea(r,c,h,w)                                                   \
(ttRect.left=(c),ttRect.right=(c)+(w),ttRect.top=(r),ttRect.bottom=(r)+(h))

#define TTaddArea(r,c,h,w)                                                   \
do {                                                                         \
    if(ttRect.left > (c))                                                    \
        ttRect.left = (c) ;                                                  \
    if(ttRect.right < (c)+(w))                                               \
        ttRect.right = (c)+(w) ;                                             \
    if(ttRect.top > (r))                                                     \
        ttRect.top = (r) ;                                                   \
    if(ttRect.bottom < (r)+(h))                                              \
        ttRect.bottom = (r)+(h) ;                                            \
} while(0)

extern void TTapplyArea (void) ;
extern int  WinMouseMode (int buttonMask, int highlight, int cursorShape);
extern int  WinLaunchProgram (meUByte *cmd, int flags, meUByte *inFile, meUByte *outFile,
#ifdef _IPIPES
                              meIPIPE *ipipe,
#endif
                              int *sysRet);

#endif /* _WIN32 */

#ifdef _DOS

#define	_CONST_TIMER 1		     /* Constantly checked timer */
#define	TIMER_MIN    0		     /* The minimum timer allowed */
/* executable extension list, in reverse order of dos priority - Must have the end NULL
 * included 4dos's btm files for completeness */
#define noExecExtensions 4
extern meUByte *execExtensions[noExecExtensions] ;

extern meUShort TTmrow, TTmcol, TTnrow, TTsrow, TTncol, TTmargin, TTscrsiz ;

extern meUByte *colTable ;

extern int  TTstart(void) ;
#define TTstartStage2()                 /* No stage 2 startup */
#define TTend()   TTclose()
extern int  TTopen(void) ;
extern int  TTclose(void) ;

extern void TTwaitForChar(void) ;
#define TTflush()
extern void DOShideCur(void) ;
extern void DOSshowCur(void) ;
extern void TThideCur(void) ;
extern void TTshowCur(void) ;
extern int  bdos(int func, unsigned dx, unsigned al);
#define TTNbell()   bdos(6, BELL, 0);

extern void TTsleep(int msec, int intable) ;
#if TYPEAH
extern int TTahead(void) ;
#endif
#define TTdieTest()
#define TTbreakTest(x) ((TTbreakCnt-- ==  0) && (TTbreakCnt=TTBREAKCNT,TTahead(),TTbreakFlag))

extern meUByte Cattr ;

extern int TTaddColor(meCOLOR index, meUByte r, meUByte g, meUByte b);
#define TTsetBgcol()
#define TTcolorSet(f,b)     ((f) | ((b) << 4))
#define TTschemeSet(scheme) \
TTcolorSet(colTable[meStyleGetFColor(meSchemeGetStyle(scheme))], \
           colTable[meStyleGetBColor(meSchemeGetStyle(scheme))])

#endif /* _DOS */

#if MOUSE
void TTinitMouse(void) ;
#define meCURSOR_DEFAULT   0
#define meCURSOR_ARROW     1
#define meCURSOR_IBEAM     2
#define meCURSOR_CROSSHAIR 3
#define meCURSOR_GRAB      4
#define meCURSOR_WAIT      5
#define meCURSOR_STOP      6
#define meCURSOR_COUNT     7
#else
#define TTinitMouse()
#endif

/* timer definitions */

enum
{
    AUTOS_TIMER_ID=0,          /* Auto save timer handle         */
    SLEEP_TIMER_ID,            /* Sleep timer                    */
    CALLB_TIMER_ID,            /* Macro callback timer           */
    IDLE_TIMER_ID,             /* Idle timer handle              */
    CURSOR_TIMER_ID,           /* Cursor blink timer handle      */
#if MOUSE
    MOUSE_TIMER_ID,            /* Mouse timer handle             */
#endif
#ifdef _URLSUPP
    SOCKET_TIMER_ID,           /* socket connection time-out     */
#endif
    NUM_TIMERS                 /* Number of timers               */
} ;

#define TIMER_SET      0x01             /* Timer is active                */
#define TIMER_EXPIRED  0x02             /* Timer has expired              */

/*
 * Timer block structure.
 */
typedef struct TIMERBLOCK
{
    struct TIMERBLOCK *next;            /* Next block to be scheduled */
    meUInt   abstime;                   /* Absolute time */
    meUByte  id;                        /* Identity of timer */
} TIMERBLOCK;

/*
 * isTimerActive
 * Returns a boolean state (0==FALSE) depending on if the timer
 * is outstanding or not.
 */
#define isTimerActive(id)  (meTimerState[id] & (TIMER_SET|TIMER_EXPIRED))
#define isTimerSet(id)     (meTimerState[id] & TIMER_SET)
#define isTimerExpired(id) (meTimerState[id] & TIMER_EXPIRED)

#define timerClearExpired(id) (meTimerState[(id)] = meTimerState[(id)] & ~TIMER_EXPIRED)

extern meUByte     meTimerState[] ;/* State of the timers, set or expired */
extern meInt       meTimerTime[] ; /* Absolute time of the timers */
extern TIMERBLOCK *timers ;        /* Head of timer list             */

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
extern void timerSet (int id, meInt tim, meInt offset) ;
extern int  _timerKill (int id) ;
#define timerKill(id) (isTimerSet(id)?_timerKill(id):timerClearExpired(id))
extern void handleTimerExpired(void) ;

#ifdef _MULTI_NOID_TIMER
extern void timerAlarm(int id) ;
#endif

#if (defined _CONST_TIMER) || (defined _SINGLE_TIMER)
extern void timerCheck(meInt tim) ;
#endif

/* flag used when idle-drop needs to be called */
#define IDLE_STATE_DROP         0x04  /* Idle timer is running */

#endif /* __TERMIOH */
