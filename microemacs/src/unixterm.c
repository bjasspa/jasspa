/* -*- C -*- ****************************************************************
 *
 *  System        : MicroEmacs Jasspa Distribution
 *  Module        : unixterm.c
 *  Synopsis      : Unix X-Term and Termcap support routines
 *  Created By    : Steven Phillips
 *  Created       : 1993
 *  Last Modified : <001011.1802>
 *
 *  Description
 *    This implementation of unix support currently only supports Unix v5 (_USG),
 *    there is no current support for V7 or VMS.
 *    Implemented XTerm support.
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
#include "efunc.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#ifdef _CYGWIN
#include <sys/socket.h>
#endif

#ifdef _USG                     /* System V */
/* We need this stuff to do the pipes properly. */
#ifdef _TERMIOS
#include <termios.h>
struct  termios otermio;        /* original terminal characteristics */
struct  termios ntermio;        /* charactoristics to use inside */
#else
#include <termio.h>
struct  termio  otermio;        /* original terminal characteristics */
struct  termio  ntermio;        /* charactoristics to use inside */
#endif /* _TERMIOS */
#endif /* _USG */

#ifdef _BSD                             /* BSD */
#include <sgtty.h>                      /* For stty/gtty functions */
struct sgttyb  osgttyb;                 /* Old tty context */
struct sgttyb  nsgttyb;                 /* New tty context */
struct tchars  otchars;                 /* Old terminal context */
struct tchars  ntchars;                 /* New terminal context */
struct ltchars oltchars;                /* Old line discipline context */
struct ltchars nltchars;                /* New line discipline context */
int    olwmode;                         /* Old local word mode */
int    nlwmode;                         /* New local word mode */
/* Terminal output buffer */
#define TERMOUT_BUFSIZ   256            /* Size of the terminal o/p buffer */
static char termOutBuf [TERMOUT_BUFSIZ];
#endif /* _BSD */

/**************************************************************************
* TERMCAP Definitions                                                     *
**************************************************************************/
/* Termcap prototyes - really we should include the appropriate header */ 
extern int      tgetnum(char *);
extern int      tgetent(char *,char *);
extern char *   tgetstr(char *,char **);
extern int      tputs(char *,int, int (*)(int));
extern int      tgetflag(char *);
#ifdef _USETPARM
extern char *   tparm(char *, int p1);
#endif

#define putpad(s)  tputs(s, 1, putchar)
#define TCAPSLEN 512

#define TERMCAP_END          0          /* End of the termcap table */
#define TERMCAP_BOOLN        1          /* Boolean termcap value */
#define TERMCAP_DIGIT        2          /* Integer termcap value */
#define TERMCAP_STRING       3          /* String termcap value */

/* TCAPkey; The termcap code table for the information that we extract from
 * termcap */
struct TCAPkey
{
    char capKey[3];                     /* Termcap key (really only need 2) */
    char type;                          /* Type of variable */
    union
    {
        int value;                      /* Integer value back */
        char *str;                      /* String value back */
    } code;                             /* Termcap code */
};

/* Define the TERMCAP entry table from the definition file */
struct TCAPkey tcaptab [] =
{
#define TCAP_BOOLN(var,c1,c2,c3)  {{c1,c2,c3},TERMCAP_BOOLN,{0}},
#define TCAP_DIGIT(var,c1,c2,c3)  {{c1,c2,c3},TERMCAP_DIGIT,{0}},
#define TCAP_STRING(var,c1,c2,c3) {{c1,c2,c3},TERMCAP_STRING,{0}},
#include "etermcap.def"
#undef TCAP_BOOLN
#undef TCAP_DIGIT
#undef TCAP_STRING
    {"", TERMCAP_END, {0}}
};

/* Define the TERMCAP index entries */
enum
{
#define TCAP_BOOLN(var,c1,c2,c3)  var,
#define TCAP_DIGIT(var,c1,c2,c3)  var,
#define TCAP_STRING(var,c1,c2,c3) var,
#include "etermcap.def"
#undef TCAP_BOOLN
#undef TCAP_DIGIT
#undef TCAP_STRING
    TCAPmax
};

/* Define the local termcap constants that we require */
unsigned  short  TTmrow=0, TTnrow=49, TTmcol=0, TTncol=80 ;
unsigned  short  TTnewWid, TTnewHig  ;
unsigned  short  TTmargin=8  ;
unsigned  short  TTscrsiz=64 ;
static    int    TTcursorVisible = 1;   /* Cursor is visible */
static    int    TTaMarginsDisabled = 0;/* Automatic margins disabled */
static    char   tcapbuf[TCAPSLEN];

/**************************************************************************
* Special termcal color definitions                                       *
**************************************************************************/
#if COLOR
uint32 *colTable=NULL ;

#define tcapNumColors 8
static uint8 tcapColors[tcapNumColors*3] =
{
    0,    0,  0,                        /* Black */
    200,  0,  0,                        /* Red */
    0,  200,  0,                        /* Green */
    200,200,  0,                        /* Yellow */
    0,    0,200,                        /* Blue */
    200,  0,200,                        /* Magenta */
    0,  200,200,                        /* Cyan */
    200,200,200,                        /* White */
} ;

/* Set up the default ANSI colors. Many Termcap entries omit these when they
 * should not - we provide our own in the hope that they may be used. */

/* exit_attribute_mode/srg0/me/Turn off all attributes */
static char RCOLSTR[] =
{ 27, 91, 48,     109, 0 } ;   /* me - ESC[0m */
/* set_a_foreground/setaf/AF/Set foreground color using ANSI escape */
static char FCOLSTR[] = 
{ 27, 91, 51, 65, 109, 0 } ;   /* AF - ESC[3Am */
/* set_a_background/setab/AB/Set background color using ANSI escape */
static char BCOLSTR[] = 
{ 27, 91, 52, 65, 109, 0 } ;   /* AB - ESC[4Am */
#endif /* COLOR */

/**************************************************************************
* Keyboard                                                                *
**************************************************************************/
#include	"eskeys.h"

#define	DEFSKEY(s,i,d,t) i,

static char *tcapSpecKeyStrs[]=
{
#include	"eskeys.def"
};
#undef	DEFSKEY

#define	DEFSKEY(s,i,d,t) (uint8 *) d,

static uint8 *tcapSpecKeyDefs[]=
{
#include	"eskeys.def"
};
#undef	DEFSKEY

/**************************************************************************
* X-Windows                                                               *
**************************************************************************/
#ifdef _XTERM

#include <X11/Xatom.h>
#include <X11/cursorfont.h>

/* Setup the xterm variables and things */
meCellMetrics mecm={0} ;


static int disableResize = 0;           /* Flag to disable screen resize */

Colormap xcmap ;
int      xscreen ;
XSizeHints sizeHints ;
int meStdin ;
#define meATOM_WM_DELETE_WINDOW 0
#define meATOM_WM_SAVE_YOURSELF 1
#define meATOM_WM_PROTOCOLS     2
#define meATOM_COPY_TEXT        3
#define meATOM_INCR             4
#define meATOM_MULTIPLE         5
#define meATOM_TARGETS          6
#define meATOM_STRING           7
static Atom meAtoms[8]={0};
char *meName="MicroEmacs" ;

#define iconWidth  48
#define iconHeight 48
static uint8 iconBits[] = {
 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f,
 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x2a,
 0x54, 0x55, 0x55, 0x55, 0x55, 0x15,
 0x54, 0x55, 0x55, 0x55, 0x55, 0x0d,
 0xf8, 0xff, 0xff, 0xff, 0xff, 0x0f,
 0xf8, 0xff, 0xff, 0xff, 0xff, 0x07,
 0xf0, 0xff, 0xff, 0xff, 0xff, 0x07,
 0xf0, 0xff, 0x07, 0xf0, 0xff, 0x03,
 0xe0, 0xff, 0x01, 0xc0, 0xff, 0x03,
 0xe0, 0xff, 0x00, 0x80, 0xff, 0x01,
 0xc0, 0x7f, 0x00, 0x00, 0xff, 0x01,
 0xc0, 0x3f, 0xe0, 0x01, 0xfe, 0x00,
 0x80, 0x1f, 0xf8, 0x07, 0xfe, 0x00,
 0x80, 0x0f, 0xfc, 0x0f, 0x7c, 0x00,
 0x00, 0x0f, 0xfc, 0x1f, 0x7c, 0x00,
 0x00, 0x0f, 0xfc, 0x1f, 0x38, 0x00,
 0x00, 0x0e, 0xfc, 0x1f, 0x18, 0x00,
 0x00, 0x0e, 0x00, 0x00, 0x18, 0x00,
 0x00, 0x0c, 0x00, 0x00, 0x08, 0x00,
 0x00, 0x0c, 0x00, 0x00, 0x08, 0x00,
 0x00, 0x08, 0x00, 0x00, 0x08, 0x00,
 0x00, 0x08, 0xfc, 0xff, 0x07, 0x00,
 0x00, 0x08, 0xfc, 0xff, 0x07, 0x00,
 0x00, 0x08, 0xfc, 0xff, 0x03, 0x00,
 0x00, 0x10, 0xf8, 0xff, 0x01, 0x00,
 0x00, 0x10, 0xf0, 0x8f, 0x03, 0x00,
 0x00, 0x20, 0xe0, 0x07, 0x07, 0x00,
 0x00, 0x40, 0x00, 0x00, 0x04, 0x00,
 0x00, 0x40, 0x00, 0x00, 0x04, 0x00,
 0x00, 0x80, 0x00, 0x00, 0x02, 0x00,
 0x00, 0x00, 0x07, 0xe0, 0x01, 0x00,
 0x00, 0x00, 0xff, 0x3f, 0x00, 0x00,
 0x00, 0x00, 0xaa, 0x2a, 0x00, 0x00,
 0x00, 0x00, 0x54, 0x15, 0x00, 0x00,
 0x00, 0x00, 0x54, 0x15, 0x00, 0x00,
 0x00, 0x00, 0xa8, 0x0a, 0x00, 0x00,
 0x00, 0x00, 0xa8, 0x0a, 0x00, 0x00,
 0x00, 0x00, 0x50, 0x05, 0x00, 0x00,
 0x00, 0x00, 0x50, 0x05, 0x00, 0x00,
 0x00, 0x00, 0xa0, 0x02, 0x00, 0x00,
 0x00, 0x00, 0xa0, 0x02, 0x00, 0x00,
 0x00, 0x00, 0x40, 0x01, 0x00, 0x00,
 0x00, 0x00, 0x40, 0x01, 0x00, 0x00,
 0x00, 0x00, 0x80, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

#define ME_KP_COUNT 16
#define ME_KP_FLAG  0x80
#define ME_KP_MASK  0x7f
uint8 TTextkey_lut [256] =
{
  0,  0,  0,  0,  0,  0,  0,  0,  SKEY_backspace,  SKEY_tab,  0,  0,  0, SKEY_return,  0,  0,
  SKEY_f11,SKEY_f12,  0,  0,  0,  0,  0,  0,  0,  0,  0, SKEY_esc,  0,  0,  0,  0,
  SKEY_tab,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  SKEY_home,SKEY_left,SKEY_up,SKEY_right,SKEY_down,SKEY_page_up,SKEY_page_down,SKEY_end,0,0,0,0,0,0,0,0,
  0,  0,  0,SKEY_insert,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,SKEY_tab,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  ME_KP_FLAG|5,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, ME_KP_FLAG|15,  0,  0,
  0,  0,  0,  0,  0,ME_KP_FLAG|7,ME_KP_FLAG|4,ME_KP_FLAG|8,ME_KP_FLAG|6,ME_KP_FLAG|2,ME_KP_FLAG|9,ME_KP_FLAG|3,ME_KP_FLAG|1,ME_KP_FLAG|5,ME_KP_FLAG|0,ME_KP_FLAG|10,
  0,0,0,0,0,0,0,0,0,0,ME_KP_FLAG|12,ME_KP_FLAG|14,0,ME_KP_FLAG|13,ME_KP_FLAG|10, ME_KP_FLAG|11,
  ME_KP_FLAG|0,ME_KP_FLAG|1,ME_KP_FLAG|2,ME_KP_FLAG|3,ME_KP_FLAG|4,ME_KP_FLAG|5,ME_KP_FLAG|6,ME_KP_FLAG|7,ME_KP_FLAG|8,ME_KP_FLAG|9,0,0,0,0,SKEY_f1,SKEY_f2,
  SKEY_f3,SKEY_f4,SKEY_f5,SKEY_f6,SKEY_f7,SKEY_f8,SKEY_f9,SKEY_f10,SKEY_f11,SKEY_f12,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,ME_KP_FLAG|13,ME_KP_FLAG|11,ME_KP_FLAG|12,ME_KP_FLAG|7,0,ME_KP_FLAG|9,0,0,0,ME_KP_FLAG|1,  0,
  ME_KP_FLAG|3,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,SKEY_delete
};	/* Extended Keyboard lookup table */

uint16 NumLockLookUpOn[ME_KP_COUNT]=
{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '.', '/', '*', '-', '+', ME_SPECIAL|SKEY_return } ;
uint16 NumLockLookUpOff[ME_KP_COUNT]=
{   ME_SPECIAL|SKEY_kp_insert, ME_SPECIAL|SKEY_kp_end, ME_SPECIAL|SKEY_kp_down, ME_SPECIAL|SKEY_kp_page_down,
    ME_SPECIAL|SKEY_kp_left, ME_SPECIAL|SKEY_kp_begin, ME_SPECIAL|SKEY_kp_right, ME_SPECIAL|SKEY_kp_home,
    ME_SPECIAL|SKEY_kp_up, ME_SPECIAL|SKEY_kp_page_up, ME_SPECIAL|SKEY_kp_delete,
    '/', '*', '-', '+', ME_SPECIAL|SKEY_return
} ;
#else  /* _XTERM */
#define meStdin 0
#endif /* _XTERM */

/**************************************************************************
* Mouse                                                                   *
**************************************************************************/
#if MOUSE
/* Local definitions for mouse handling code */
/* mouseState
 * A integer interpreted as a bit mask that holds the current state of
 * the mouse interaction. */
#define MOUSE_STATE_LEFT         0x0001 /* Left mouse button is pressed */
#define MOUSE_STATE_MIDDLE       0x0002 /* Middle mouse button is pressed */
#define MOUSE_STATE_RIGHT        0x0004 /* Right mouse button is pressed */
#define MOUSE_STATE_BUT4         0x0008 /* Button 4 (can be used by wheel) */
#define MOUSE_STATE_BUT5         0x0010 /* Button 5 (can be used by wheel) */
#define MOUSE_STATE_BUTTONS      (MOUSE_STATE_LEFT|MOUSE_STATE_MIDDLE|MOUSE_STATE_RIGHT|MOUSE_STATE_BUT4|MOUSE_STATE_BUT5)

int  mouseState = 0;             /* State of the mouse. */

/* mouseKeyState
 * The state of the control keys when the mouse was pressed. */
uint16 mouseKeyState;     /* State of keyboard control */

static uint16 mouseKeys[8] = { 0, 1, 2, 3, 4, 5 } ;
#define mouseButtonPick(bb) (mouseState |=  (1<<((bb)-1)))
#define mouseButtonDrop(bb) (mouseState &= ~(1<<((bb)-1)))

#define mouseButtonGetPick()                                                   \
((mouseState == 0)                 ? 0:                                      \
 (mouseState & MOUSE_STATE_LEFT)   ? 1:                                      \
 (mouseState & MOUSE_STATE_MIDDLE) ? 2:                                      \
 (mouseState & MOUSE_STATE_RIGHT)  ? 3:                                      \
 (mouseState & MOUSE_STATE_BUT4)   ? 4:                                      \
 (mouseState & MOUSE_STATE_BUT5)   ? 5:0)

#endif


void
sigAlarm(SIGNAL_PROTOTYPE)
{
    /* Trap Alarm signals under UNIX. Used to do auto saving. */
#if !((defined (_POSIX_SIGNALS)) || (defined _BSD_SIGNALS))
    signal (SIGALRM, sigAlarm);
#endif
    /* Note that we use the first timers expire time here and not the actual time.
     * sometimes the itimers fire a little bit early i.e. +-5 msec, to
     * ensure that we do not hang then we force the timer to expire.
     * There is no other way for the timer to expire (that I am aware of) */
    if (timers != NULL)
        timerCheck(timers->abstime);           /* Timer has expired */
}

int
meGidInGidList(gid_t gid)
{
    int ii=meGidSize ;
    while(--ii>=0)
        if(meGidList[ii] == gid)
            return 1 ;
    return 0 ;
}

static void
TCAPgetWinSize(void)
{
#ifdef TIOCGWINSZ
    /* BSD-style.  */
    struct winsize size;

    if((ioctl(meStdin,TIOCGWINSZ,&size) == -1) ||
       ((TTnewWid = size.ws_col) <= 0) ||
       ((TTnewHig = size.ws_row) <= 0))
#else
#ifdef TIOCGSIZE
    /* SunOS - style.  */
    struct ttysize size;

    if((ioctl(meStdin,TIOCGSIZE,&size) == -1) ||
       ((TTnewWid = size.ts_col) <= 0) ||
       ((TTnewHig = size.ts_lines) <= 0))
#endif
#endif
    {
        int  ii ;

        /* Get the number of lines on the screen */
        if((ii=tgetnum(tcaptab[TCAPlines].capKey)) != -1)
        {
            /* If automatic margins are active and there is a new line glitch
             * then reduce the terminal height by 1 line. This prevents the
             * screen from scrolling up */
            if ((tgetnum(tcaptab[TCAPam].capKey) == 1) &&
                (tgetnum(tcaptab[TCAPxenl].capKey) == 0))
                ii--;
            TTnewHig = ii ;
        }            
        else
            TTnewHig = TTnrow + 1 ;
        
        /* Get the number of columns on the screen */
        if((ii=tgetnum(tcaptab[TCAPcols].capKey)) != -1)
            TTnewWid = ii ;
        else
            TTnewWid = TTncol ;
    }
}

static void
sigSize(SIGNAL_PROTOTYPE)
{
#if !((defined (_POSIX_SIGNALS)) || (defined _BSD_SIGNALS))
    /* Reset the signal handler */
    signal (SIGWINCH, sigSize) ;
#endif
    TCAPgetWinSize() ;

    if((TTnewWid != TTncol) || (TTnewHig != TTnrow+1))
        alarmState |= meALARM_WINSIZE ;
}


#ifdef _XTERM

static Font
__XTERMfontGetId(uint8 font)
{
    uint8 fontNU ;
    
    /* remove the underline as this is not a different font,
     * the underline is drawn on after */
    fontNU = (font & ~meFONT_UNDERLINE) ;
    if(mecm.fontTbl[fontNU] == BadName)
    {
        XFontStruct *fontQ ;
        char *weight;
        char *slant;
        char buf [MAXBUF];              /* Local name buffer */
        
        /* Process the bold field. If the X font specifier (weight) is
         * medium or undefined and this is a bold field identifier then
         * specify a bold font */
        if(fontNU & meFONT_BOLD)
            weight = "bold";
        else if(fontNU & meFONT_LIGHT)
            weight = "light";
        else
            weight = (char *)mecm.fontPart [2];
        
        /* Process the italic field. If the X font specified (slant) is r
         * or undefined and this is a italic field identifier then specify
         * an italic font. */
        if(fontNU & meFONT_ITALIC)
            slant = "o";
        else
            slant = (char *)mecm.fontPart [3];
        
        /* Construct the new font name */
        sprintf (buf, "-%s-%s-%s-%s-%s-%s-%s-%s-%s-%s-%s-%s-%s-%s",
                 mecm.fontPart [0],  mecm.fontPart [1],  weight,
                 slant,              mecm.fontPart [4],  mecm.fontPart [5],
                 mecm.fontPart [6],  mecm.fontPart [7],  mecm.fontPart [8],
                 mecm.fontPart [9],  mecm.fontPart [10], mecm.fontPart [11],
                 mecm.fontPart [12], mecm.fontPart [13]);
        
        if((fontQ=XLoadQueryFont(mecm.xdisplay,buf)) == NULL)
            mecm.fontTbl[fontNU] = mecm.fontTbl[0] ;
        else
        {
            if(((fontQ->ascent + fontQ->descent) != mecm.fdepth) ||
               (fontQ->max_bounds.width != mecm.fwidth))
                mecm.fontTbl[fontNU] = mecm.fontTbl[0] ;
            else
            {
                mecm.fontTbl[fontNU] = fontQ->fid ;
                mecm.fontFlag[fontNU] = 1 ;
            }
            XFreeFontInfo(NULL,fontQ,1) ;
        }
    }
    mecm.fontTbl[font] = mecm.fontTbl[fontNU] ;
    return mecm.fontTbl[font] ;
}

#define XTERMfontGetId(font) \
((mecm.fontTbl[font] == BadName) ? __XTERMfontGetId(font):mecm.fontTbl[font])


void
XTERMschemeSet(meSCHEME scheme)
{
    uint32 valueMask = 0 ;
    uint8 cc ;

    cc = meStyleGetFColor(meSchemeGetStyle(scheme)) ;
    if(mecm.fcol != cc)
    {
        mecm.fcol = cc ;
        mecm.xgcv.foreground = colTable[cc] ;
        valueMask |= GCForeground ;
    }
    cc = meStyleGetBColor(meSchemeGetStyle(scheme)) ;
    if(mecm.bcol != cc)
    {
        mecm.bcol = cc ;
        mecm.xgcv.background = colTable[cc] ;
        valueMask |= GCBackground ;
    }
    if(mecm.fontName != NULL)
    {
        cc = meStyleGetFont(meSchemeGetStyle(scheme)) ;
        if(meSchemeTestNoFont(scheme))
            cc &= ~(meFONT_BOLD|meFONT_ITALIC|meFONT_UNDERLINE) ;
        if(mecm.font != cc)
        {
            mecm.font = cc ;
            mecm.fontId = XTERMfontGetId(cc) ;
            if(mecm.xgcv.font != mecm.fontId)
            {
                mecm.xgcv.font = mecm.fontId ;
                valueMask |= GCFont ;
            }
        }
    }
    if(valueMask)
        XChangeGC(mecm.xdisplay,mecm.xgc,valueMask,&mecm.xgcv) ;
}

/* XTERMSpecialChar; Draw a special character to the screen. x is the lefthand
 * edge of the character, y is the top of the character. xgc is used for the
 * colors.
 */
void
XTERMSpecialChar (int x, int y, uint8 cc)
{
    XPoint points[3] ;
    int ii ;
    /* Fill in the character */
    switch (cc)
    {
    case 0x07:          /* Line space '.' */
        XDrawLine(mecm.xdisplay, mecm.xwindow, mecm.xgc, x+mecm.fhwidth, y+mecm.fhdepth, x+mecm.fhwidth+1, y+mecm.fhdepth);
        break;

    case 0x09:          /* Tab character -> */
        ii=(mecm.fhdepth+1) >> 1 ;
        XDrawLine(mecm.xdisplay, mecm.xwindow, mecm.xgc, x+1, y+mecm.fhdepth, x+mecm.fhwidth-1, y+mecm.fhdepth);
        points[0].x = x+mecm.fhwidth-1 ;
        points[0].y = y+ii ;
        points[1].x = x+mecm.fhwidth-1 ;
        points[1].y = y+mecm.fdepth-ii-1;
        points[2].x = x+mecm.fwidth-2 ;
        points[2].y = y+mecm.fhdepth ;
        XFillPolygon(mecm.xdisplay, mecm.xwindow, mecm.xgc, points, 3, Convex, CoordModeOrigin) ;
        break;

    case 0x0a:          /* CR character / <-| */
        ii=(mecm.fhdepth+1) >> 1 ;
        XDrawLine(mecm.xdisplay, mecm.xwindow, mecm.xgc, x+mecm.fhwidth,  y+mecm.fhdepth, x+mecm.fwidth-2, y+mecm.fhdepth);
        XDrawLine(mecm.xdisplay, mecm.xwindow, mecm.xgc, x+mecm.fwidth-2, y+mecm.fhdepth, x+mecm.fwidth-2, y+ii-1);
        points[0].x = x+mecm.fhwidth ;
        points[0].y = y+ii ;
        points[1].x = x+mecm.fhwidth ;
        points[1].y = y+mecm.fdepth-ii-1;
        points[2].x = x+1 ;
        points[2].y = y+mecm.fhdepth ;
        XFillPolygon(mecm.xdisplay, mecm.xwindow, mecm.xgc, points, 3, Convex, CoordModeOrigin) ;
        break;

    case 0x0b:          /* Line Drawing / Bottom right _| */
        XDrawLine(mecm.xdisplay, mecm.xwindow, mecm.xgc, x, y + mecm.fhdepth, x + mecm.fhwidth, y + mecm.fhdepth);
        XDrawLine(mecm.xdisplay, mecm.xwindow, mecm.xgc, x + mecm.fhwidth, y + mecm.fhdepth, x + mecm.fhwidth, y);
        break;

    case 0x0c:          /* Line Drawing / Top right */
        XDrawLine(mecm.xdisplay, mecm.xwindow, mecm.xgc, x, y + mecm.fhdepth, x + mecm.fhwidth, y + mecm.fhdepth);
        XDrawLine(mecm.xdisplay, mecm.xwindow, mecm.xgc, x + mecm.fhwidth, y + mecm.fhdepth, x + mecm.fhwidth, y + mecm.fdepth - 1);
        break;

    case 0x0d:          /* Line Drawing / Top left */
        XDrawLine(mecm.xdisplay, mecm.xwindow, mecm.xgc, x + mecm.fwidth - 1, y + mecm.fhdepth, x + mecm.fhwidth, y + mecm.fhdepth);
        XDrawLine(mecm.xdisplay, mecm.xwindow, mecm.xgc, x + mecm.fhwidth, y + mecm.fhdepth, x + mecm.fhwidth, y + mecm.fdepth - 1);
        break;

    case 0x0e:          /* Line Drawing / Bottom left |_ */
        XDrawLine(mecm.xdisplay, mecm.xwindow, mecm.xgc, x + mecm.fhwidth, y, x + mecm.fhwidth, y + mecm.fhdepth);
        XDrawLine(mecm.xdisplay, mecm.xwindow, mecm.xgc, x + mecm.fhwidth, y + mecm.fhdepth, x + mecm.fwidth - 1, y + mecm.fhdepth);
        break;

    case 0x0f:          /* Line Drawing / Centre cross + */
        XDrawLine(mecm.xdisplay, mecm.xwindow, mecm.xgc, x, y + mecm.fhdepth, x + mecm.fwidth - 1, y + mecm.fhdepth);
        XDrawLine(mecm.xdisplay, mecm.xwindow, mecm.xgc, x + mecm.fhwidth, y, x + mecm.fhwidth, y + mecm.fdepth - 1);
        break;

    case 0x10:          /* Cursor Arrows / Right */
        points[0].x = x + 1 ;
        points[0].y = y ;
        points[1].x = x + 1 ;
        points[1].y = y + mecm.fdepth - 1;
        points[2].x = x + mecm.fwidth ;
        points[2].y = y + mecm.fhdepth ;
        XFillPolygon(mecm.xdisplay, mecm.xwindow, mecm.xgc, points, 3, Convex, CoordModeOrigin) ;
        break;

    case 0x11:          /* Cursor Arrows / Left */
        points [0].x = x + mecm.fwidth - 1 ;
        points [0].y = y ;
        points [1].x = x + mecm.fwidth - 1 ;
        points [1].y = y + mecm.fdepth - 1 ;
        points [2].x = x ;
        points [2].y = y + mecm.fhdepth ;
        XFillPolygon(mecm.xdisplay, mecm.xwindow, mecm.xgc, points, 3, Convex, CoordModeOrigin) ;
        break;

    case 0x12:          /* Line Drawing / Horizontal line - */
        XDrawLine(mecm.xdisplay, mecm.xwindow, mecm.xgc, x, y + mecm.fhdepth, x + mecm.fwidth - 1, y + mecm.fhdepth);
        break;

    case 0x15:          /* Line Drawing / Left Tee |- */
        XDrawLine(mecm.xdisplay, mecm.xwindow, mecm.xgc, x + mecm.fhwidth, y, x + mecm.fhwidth, y + mecm.fdepth - 1);
        XDrawLine(mecm.xdisplay, mecm.xwindow, mecm.xgc, x + mecm.fhwidth, y + mecm.fhdepth, x + mecm.fwidth - 1, y + mecm.fhdepth);
        break;

    case 0x16:          /* Line Drawing / Right Tee -| */
        XDrawLine(mecm.xdisplay, mecm.xwindow, mecm.xgc, x + mecm.fhwidth, y, x + mecm.fhwidth, y + mecm.fdepth - 1);
        XDrawLine(mecm.xdisplay, mecm.xwindow, mecm.xgc, x, y + mecm.fhdepth, x + mecm.fhwidth, y + mecm.fhdepth);
        break;

    case 0x17:          /* Line Drawing / Bottom Tee _|_ */
        XDrawLine(mecm.xdisplay, mecm.xwindow, mecm.xgc, x, y + mecm.fhdepth, x + mecm.fwidth - 1, y + mecm.fhdepth);
        XDrawLine(mecm.xdisplay, mecm.xwindow, mecm.xgc, x + mecm.fhwidth, y, x + mecm.fhwidth, y + mecm.fhdepth);
        break;

    case 0x18:          /* Line Drawing / Top Tee -|- */
        XDrawLine(mecm.xdisplay, mecm.xwindow, mecm.xgc, x, y + mecm.fhdepth, x + mecm.fwidth - 1, y + mecm.fhdepth);
        XDrawLine(mecm.xdisplay, mecm.xwindow, mecm.xgc, x + mecm.fhwidth, y + mecm.fdepth - 1, x + mecm.fhwidth, y + mecm.fhdepth);
        break;

    case 0x19:          /* Line Drawing / Vertical Line | */
        XDrawLine(mecm.xdisplay, mecm.xwindow, mecm.xgc, x + mecm.fhwidth, y, x + mecm.fhwidth, y + mecm.fdepth - 1);
        break;

    case 0x1e:          /* Cursor Arrows / Up */
        points [0].x = x - 1 ;
        points [0].y = y + mecm.fdepth - 1 ;
        points [1].x = mecm.fhwidth + (mecm.fwidth & 0x01) ;
        points [1].y = -mecm.fadepth ;
        points [2].x = mecm.fhwidth + 1 ;
        points [2].y = mecm.fadepth ;
        XFillPolygon(mecm.xdisplay, mecm.xwindow, mecm.xgc, points, 3, Convex, CoordModePrevious) ;
        break;

    case 0x1f:          /* Cursor Arrows / Down */
        points [0].x = x - 1 ;
        points [0].y = y + 1 ;
        points [1].x = mecm.fhwidth + (mecm.fwidth & 0x01) ;
        points [1].y = mecm.fadepth ;
        points [2].x = mecm.fhwidth + 1 ;
        points [2].y = -mecm.fadepth ;
        XFillPolygon(mecm.xdisplay, mecm.xwindow, mecm.xgc, points, 3, Convex, CoordModePrevious) ;
        break;
    }
}

/*
 * XTERMPaint
 * Repaint the screen for the specified region from the frame store.
 * We use the colour and text information from the frame store and
 * re-paint it on the canvas, thereby refreshing the screen.
 *
 * srow - Start row
 * erow - End row
 * scol - Start column
 * ecol - End column
 */
void
XTERMPaint (int srow, int scol, int erow, int ecol)
{
    FRAMELINE *flp;                     /* Frame store line pointer */
    meSCHEME  *fssp;                    /* Frame store colour pointer */
    uint8     *fstp;                    /* Frame store text pointer */
    meSCHEME   schm;                    /* Current colour */
    int col;                            /* Current column position */
    int row;                            /* Current row screen position */
    int tcol;                           /* Text column start */
    int length;                         /* Length of string */

    /* Process each row in turn until we reach the end of the line */
    if (meSystemCfg & meSYSTEM_FONTFIX)
    {
        uint8 cc, *sfstp, buff[MAXBUF] ;
        int spFlag ;

        for (flp = frameStore + srow; srow < erow; srow++, flp++)
        {
            length = 0;                     /* Initialise the string length */
            col = scol;                     /* Current column becomes start column */
            tcol = col;                     /* Start of the text column */
            row = rowToClient(srow) ;

            /* Get pointers aligned into the frame store */
            sfstp = flp->text ;
            fstp = sfstp + scol ;           /* Point to text block */
            fssp = flp->scheme + scol ;     /* Point to colour block */
            schm = *fssp;                   /* Get the initial scheme */
            spFlag = 0 ;
            while (col < ecol)
            {
                if (*fssp++ != schm)        /* Change in colour ?? */
                {
                    /* Output the current text item */
                    XTERMschemeSet(schm) ;
                    XTERMstringDraw(colToClient(tcol),row,buff,length);
                    while(--spFlag >= 0)
                    {
                        while (((cc=sfstp[tcol]) & 0xe0) != 0)
                            tcol++ ;
                        XTERMSpecialChar(tcol*mecm.fwidth,row-mecm.ascent,cc) ;
                        tcol++ ;
                    }
                    spFlag = 0 ;
                    tcol = col;             /* Move the text position */
                    length = 0;             /* Reset the length */
                    schm = fssp[-1];        /* Get the next colour */
                }
                if(((cc=*fstp++) & 0xe0) == 0)
                {
                    spFlag++ ;
                    cc = ' ' ;
                }
                buff[length++]=cc;          /* Set & Increment the string length */
                col++;                      /* Next colunm */
            }

            /* Output the remaining text item */
            if (length > 0)
            {
                XTERMschemeSet(schm) ;
                XTERMstringDraw(colToClient(tcol),row,buff,length);
                while(--spFlag >= 0)
                {
                    while (((cc=sfstp[tcol]) & 0xe0) != 0)
                        tcol++ ;
                    XTERMSpecialChar(tcol*mecm.fwidth,row-mecm.ascent,cc) ;
                    tcol++ ;
                }
            }
        }
    }
    else
    {
        for (flp = frameStore + srow; srow < erow; srow++, flp++)
        {
            length = 0;                     /* Initialise the string length */
            col = scol;                     /* Current column becomes start column */
            tcol = col;                     /* Start of the text column */

            /* Get pointers aligned into the frame store */
            fstp = flp->text + scol ;       /* Point to text block */
            fssp = flp->scheme + scol ;     /* Point to colour block */
            schm = *fssp;                   /* Get the initial scheme */

            while (col < ecol)
            {
                if (*fssp++ != schm)        /* Change in colour ?? */
                {
                    /* Output the current text item */
                    XTERMschemeSet(schm) ;
                    XTERMstringDraw(colToClient(tcol),rowToClient(srow),fstp,length);
                    fstp += length;         /* Move the text pointer */
                    tcol = col;             /* Move the text position */
                    length = 0;             /* Reset the length */
                    schm = fssp[-1];        /* Get the next colour */
                }
                length++;                   /* Increment the string length */
                col++;                      /* Next colunm */
            }

            /* Output the remaining text item */
            if (length > 0)
            {
                XTERMschemeSet(schm) ;
                XTERMstringDraw(colToClient(tcol),rowToClient(srow),fstp,length);
            }
        }
    }
}

/* following are used for incremental Selection getting */
static uint8 *meClipBuff=NULL ;
static int meClipSize=0 ;

/* The xterm XEvent handler */
static void
meXEventHandler(void)
{
    XEvent event ;

    /* Get the next event - wait if necesary */
    XNextEvent(mecm.xdisplay,&event);
/*    if(XFilterEvent(&event,mecm.xwindow) == True)*/
/*    {*/
/*        printf("Filtered event %d\n",event.type) ;*/
/*        return ;*/
/*    }*/

    /* printf("Got event %d\n",event.type) ; */
    switch(event.type)
    {
    case ConfigureNotify:
        /* window has been resized, change width and height to send to
         * draw_text and draw_graphics */
        {
            /* Get the width and heigth back and setup the TTmrow etc */
            int ww, hh ;

            sizeHints.height = event.xconfigure.height ;
            sizeHints.width  = event.xconfigure.width ;
            hh = event.xconfigure.height / mecm.fdepth ;
            ww = event.xconfigure.width / mecm.fwidth ;
            /*            printf("Got event2 Config %d %d\n",hh,ww) ;*/
            /*        printf("Got configure event! (%d,%d)\n",TTncol,TTnrow) ;*/
            /* Disable the window resize until BOTH width and height changes
             * have been established. Both LINUX and SUN suffer from severe
             * beating between the client and server when the resize requests
             * are split. Allow the new sizes to be computed and then inform
             * the server in one invocation. */
            disableResize = 1;          /* Disable X reconfiguration */
            changeScreenWidth(TRUE,ww); /* Change width */
            changeScreenDepth(TRUE,hh); /* Change depth */
            disableResize = 0;          /* Re-enable the resize */

            /* Change the size of the window now that both sizes have been
             * established. Only perform the resize if there is something to
             * change */
            if ((hh != TTnrow+1) || (ww != TTncol))
            {
                sizeHints.height = mecm.fdepth*(TTnrow+1) ;
                sizeHints.width  = mecm.fwidth*TTncol ;
                XResizeWindow(mecm.xdisplay,mecm.xwindow,sizeHints.width,sizeHints.height) ;
            }
        }
        break;
    case Expose:
        /*
         * All, or part of the window has been exposed. probably caused by
         * another window passing ontop of the MicroEmacs canvas, or we have
         * just been exposed.
         *
         * Update the canvas from the frame store, there is no need to
         * regenerate the screen, simply repaint those canval pixels which
         * have been corrupted by X over-painting another window.
         *
         * Note that we use the 'count' in the XExposeEvent. This tells us
         * if another Expose event is coming on the input queue when non-zero.
         * In this case we hold off on the paint and remember the largest
         * bounding box that contains the region to be painted. Only when it
         * reaches zero do we actually perform the paint. This allows us to
         * optimise the painting otherwise we may over paint the same region
         * multiple times. Who knows what the user is doing !!
         *
         * Note that keeping a single bounding box may mean that we update
         * too much of the screen, however in most cases multiple paints
         * are in adjacent regions.
         *
         * Jon Green 28/06/97
         */
        {
            static int sscol= -1;       /* History - Start column */
            static int ssrow;           /* History - Start row */
            static int secol;           /* History - End column */
            static int serow;           /* History - End row */
            int scol;                   /* Current - Start column */
            int srow;                   /* Current - Start row */
            int ecol;                   /* Current - End column */
            int erow;                   /* Current - End row */

            /* Compute the start and end column widths. For the end
             * column then a rounding operation is performed to ensure
             * that the whole character is covered */
            scol = clientToCol(event.xexpose.x);
            ecol = clientToCol(event.xexpose.x+event.xexpose.width+mecm.fwidth-1);
            if (ecol > TTncol)
                ecol = TTncol;
            srow = clientToRow(event.xexpose.y);
            erow = clientToRow(event.xexpose.y+event.xexpose.height+mecm.fdepth-1);
            if (erow > (TTnrow+1))
                erow = (TTnrow+1);

            /*
             * Determine if the paint should be held off, other paints
             * are on their way. If the paint is to be held off then
             * remeber the extents of the bounding box, otherwise
             * do the paint.
             */
            if (sscol == -1)            /* No historical bounding information */
            {
                if (event.xexpose.count == 0)   /* Last of the expose events ?? */
                    XTERMPaint (srow, scol, erow, ecol);
                else                    /* More to come - save bounding box */
                {
                    sscol = scol;
                    secol = ecol;
                    ssrow = srow;
                    serow = erow;
                }
            }
            /* A historical bounding box exists. */
            else
            {
                /* Update the historical bounding box infomation by using the
                 * maximum and minimum values of the existing and current
                 * Exposure boxes. */
                if (scol < sscol)
                    sscol = scol;
                if (ecol > secol)
                    secol = ecol;
                if (srow < ssrow)
                    ssrow = srow;
                if (erow > serow)
                    serow = erow;
                /* If this is the last of the Expose events then initiate
                 * a paint to redraw the screen. */
                if (event.xexpose.count == 0)
                {
                    XTERMPaint (ssrow, sscol, serow, secol);
                    sscol = -1;         /* Reset the history to none */
                }
            }
            if((cursorState >= 0) && blinkState)
                XTERMshowCur() ;
            XFlush(mecm.xdisplay) ;
        }
        break;

    case MotionNotify:                  /* Mouse has moved. */
        if(meMouseCfg & meMOUSE_ENBLE)
        {
            /* Collect the position of the mouse. Require the row/column mouse
             * position. Also require the fractional bits incase we are scrolling
             *
             * Really we should re-evaluate the buttons here. However X-Windows
             * does not seemed to have aquired an incorrect state (i.e. button
             * state mis-represented). I will leave it for the time being (JDG)
             */
            uint16 cc;                  /* Character code */
            uint32 arg;                 /* Decode key argument */
            mouse_X = event.xmotion.x / mecm.fwidth ;
            mouse_Y = event.xmotion.y / mecm.fdepth ;
            mouse_dX = ((event.xmotion.x - (mouse_X * mecm.fwidth)) << 8) / mecm.fwidth;
            mouse_dY = ((event.xmotion.y - (mouse_Y * mecm.fdepth)) << 8) / mecm.fdepth;
            /* Check for a mouse-move key */
            cc = (ME_SPECIAL | mouseKeyState |
                  (SKEY_mouse_move+mouseKeys[mouseButtonGetPick()])) ;
            /* Are we after all movements or mouse-move bound ?? */
            if((TTallKeys & 0x1) || (!TTallKeys && (decode_key(cc,&arg) != -1)))
                addKeyToBuffer(cc) ;        /* Add key to keyboard buffer */
        }
        break;

    case EnterNotify:
        /* Some window managers do not send the FocusIn and FocusOut events correctly
         * To fix this we have to track down Window Enter and Leave events and check
         * for a focus change ourselves */
        if(!TTfocus)
        {
            Window fwin ;
            int rtr ;
            XGetInputFocus(mecm.xdisplay,&fwin,&rtr) ;
            if(fwin != mecm.xwindow)
                break ;
        }
    case FocusIn:
        if(!TTfocus)
	{
            TTfocus = 1 ;
            if((cursorState >= 0) && blinkState)
            {
                if(cursorBlink)
                    TThandleBlink(2) ;
                else
                    XTERMshowCur() ;
                XFlush(mecm.xdisplay) ;
            }
	}
        break ;

    case LeaveNotify:
        /* Only get this event if using the -f option */
        if(TTfocus)
        {
            Window fwin ;
            int rtr ;
            XGetInputFocus(mecm.xdisplay,&fwin,&rtr) ;
            if(fwin == mecm.xwindow)
                break ;
        }
    case FocusOut:
        if(TTfocus)
        {
            TTfocus = 0 ;
            if(cursorState >= 0)
            {
                /* because the cursor is a part of the solid cursor we must
                 * remove the old one first and then redraw
                 */
                if(blinkState)
                    XTERMhideCur() ;
                blinkState = 1 ;
                XTERMshowCur() ;
                XFlush(mecm.xdisplay) ;
            }
        }
        break ;

    case ButtonPress:
        if(meMouseCfg & meMOUSE_ENBLE)
        {
            unsigned int   bb ;
            uint16 ss ;
            if(!TTfocus)
                /* if we haven't currently got the input focus, grab it now */
                XSetInputFocus(mecm.xdisplay,mecm.xwindow,RevertToPointerRoot,CurrentTime) ;

            /* Collect the position of the mouse. Require the row/column mouse
             * position. Also require the fractional bits incase we are scrolling */
            mouse_X = event.xbutton.x / mecm.fwidth ;
            mouse_Y = event.xbutton.y / mecm.fdepth ;
            mouse_dX = ((event.xbutton.x - (mouse_X * mecm.fwidth)) << 8) / mecm.fwidth;
            mouse_dY = ((event.xbutton.y - (mouse_Y * mecm.fdepth)) << 8) / mecm.fdepth;
            bb = event.xbutton.button ;

            /* Add the presense of the button to the mouseState. */
            mouseButtonPick(bb) ;

            /* Generate keycode and shunt on the key queue */
            ss = event.xbutton.state ;
            mouseKeyState = ((ss & 0x01) << 8) | ((ss & 0x0C) << 7);
            ss = (ME_SPECIAL | (SKEY_mouse_pick_1+mouseKeys[bb]-1) | mouseKeyState) ;
            addKeyToBuffer(ss) ;
        }
        break ;
    case ButtonRelease:
        if(meMouseCfg & meMOUSE_ENBLE)
        {
            unsigned int   bb ;
            uint16 ss ;
            /* Collect the position of the mouse. Require the row/column mouse
             * position. Also require the fractional bits incase we are scrolling */
            mouse_X = event.xbutton.x / mecm.fwidth ;
            mouse_Y = event.xbutton.y / mecm.fdepth ;
            mouse_dX = ((event.xbutton.x - (mouse_X * mecm.fwidth)) << 8) / mecm.fwidth;
            mouse_dY = ((event.xbutton.y - (mouse_Y * mecm.fdepth)) << 8) / mecm.fdepth;
            bb = event.xbutton.button ;

            /* Remove the presense of the button from the mouseState. */
            mouseButtonDrop(bb) ;

            /* Generate keycode and shunt on the key queue */
            ss = event.xbutton.state ;
            mouseKeyState = ((ss & 0x01) << 8) | ((ss & 0x0C) << 7);
            ss = (ME_SPECIAL | (SKEY_mouse_drop_1+mouseKeys[bb]-1) | mouseKeyState) ;
            addKeyToBuffer(ss) ;
        }
        break ;
    case KeyPress:
        {
            uint16 ii, ss ;
            KeySym keySym ;
            char   keyStr[20];
            int    keyLen ;

            ss = event.xkey.state ;
            keyLen = XLookupString(&event.xkey,keyStr,20,&keySym,NULL);
            /* Map Mod2 which is ALT-GR onto A-C */
#if (defined _SUNOS_X86)
            /* SunOS mapped to modifier key 2 */
            if (ss & 0x10)
                ss = (ss & ~0x10) | 0xc;
#else
#if (defined _LINUX)
            /* Linux mapped to modifier key 3 */
            if (ss & 0x20)
                ss = (ss & ~0x20) | 0xc;
#endif
#endif
#if 0
            keyStr[keyLen] = '\0' ;
            printf("got key %x, ss=%x [%s]\n",(unsigned int) keySym, ss, keyStr) ;
#endif
            if(keySym > 0xff)
            {
                keySym = TTextkey_lut[keySym & 0xff] ;
                if(keySym == 0)
                    break ;
                /* Cope with the num lock state on the key pad */
                if(keySym & ME_KP_FLAG)
                {
                    if(ss & 0x10)
                        ii = (NumLockLookUpOff[keySym & ME_KP_MASK] | ((ss & 0x0C) << 7)) ;
                    else
                        ii = (NumLockLookUpOn[keySym & ME_KP_MASK] | ((ss & 0x0C) << 7)) ;
                    /* only add the shift modifier if its a special key */
                    if(ii & ME_SPECIAL)
                        ii |= ((ss & 0x01) << 8) ;
                }
                else
                    ii = (ME_SPECIAL | keySym | ((ss & 0x01) << 8) | ((ss & 0x0C) << 7)) ;
            }
            else
            {
                keySym &= 0xff ;
                if(ss & 0x0C)
                {
                    keySym = toUpper(keySym) ;
                    if(((ss & 0x0C) != 4) ||
                       ((keySym < 'A') || (keySym > '_')))
                    {
                        keySym = toLower(keySym) ;
                        ii = keySym | ((ss & 0x0C) << 7) ;
                    }
                    else
                        ii = keySym-'@' ;
                }
                else
                    ii = keySym ;
            }
            if(ii == 0x07)
            {
                if(macbug < 0)
                {
                    macbug = 1 ;
                    break ;
                }
                TTbreakFlag = 1 ;
            }
            addKeyToBuffer(ii) ;
            break ;
        }
#ifdef _CLIPBRD
    case SelectionClear:
        if((event.xselectionclear.window == mecm.xwindow) &&
           (event.xselection.selection == XA_PRIMARY))
            clipState &= ~CLIP_OWNER ;
        break ;

    case SelectionRequest:
        {
            XSelectionEvent reply;

            /* printf("Got SelectionRequest %d, Target = %d %s, string %d\n",*/
            /*        event.xselectionrequest.requestor, event.xselectionrequest.target,*/
            /*        XGetAtomName(mecm.xdisplay, event.xselectionrequest.target),XA_STRING) ;*/
            reply.type = SelectionNotify;
            reply.serial = 1;
            reply.send_event = 1;
            reply.display = mecm.xdisplay;
            reply.requestor = event.xselectionrequest.requestor ;
            reply.selection = event.xselectionrequest.selection ;
            reply.target = event.xselectionrequest.target ;
            reply.property = None ;
            reply.time = event.xselectionrequest.time ;

            if(event.xselectionrequest.selection == XA_PRIMARY)
            {
                if((event.xselectionrequest.target == XA_STRING) && (klhead != NULL))
                {
                    static uint8 *data=NULL ;
                    static int dataLen=0 ;
                    uint8 *ss, *dd, cc ;
                    KILL *killp ;
                    int   len ;

                    len = 0 ;
                    killp = klhead->kill;
                    while(killp != NULL)
                    {
                        len += meStrlen(killp->data) ;
                        killp = killp->next;
                    }
                    if((dataLen <= len) &&
                       ((ss = malloc(len+1)) != NULL))
                    {
                        meNullFree(data) ;
                        data = ss ;
                        dataLen = len+1 ;
                    }
                    if(dataLen > len)
                    {
                        ss = data ;
                        killp = klhead->kill;
                        while(killp != NULL)
                        {
                            dd = killp->data ;
                            while((cc = *dd++))
                                *ss++ = cc ;
                            killp = killp->next ;
                        }
                        *ss = '\0' ;
                        reply.property = event.xselectionrequest.property ;
                        XChangeProperty(mecm.xdisplay,reply.requestor,reply.property,reply.target,
                                        8,PropModeReplace,data,len);
                    }
                }
                else if(event.xselectionrequest.target == meAtoms[meATOM_TARGETS])
                {
                    reply.property = event.xselectionrequest.property ;
                    XChangeProperty(mecm.xdisplay,reply.requestor,reply.property,reply.target,
                                    32,PropModeReplace,(unsigned char *) (meAtoms+meATOM_TARGETS),2);
                }
            }
            XSendEvent(mecm.xdisplay,reply.requestor,False,0,(XEvent *) &reply) ;
            break;
        }
    case SelectionNotify:
        {
            uint32 nitems, left;
            uint8 *buff, *dd ;
            Atom type ;
            int  fmt ;

            if((event.xselection.selection == XA_PRIMARY) &&
               (event.xselection.property == meAtoms[meATOM_COPY_TEXT]) &&
               (XGetWindowProperty(mecm.xdisplay,mecm.xwindow,meAtoms[meATOM_COPY_TEXT],0,0x1fffffffL,False,AnyPropertyType,
                                   &type, &fmt, &nitems, &left, &buff) == Success))
            {
                if(type == meAtoms[meATOM_MULTIPLE])
#ifndef NDEBUG
                    printf("Currently don't support multiple\n")
#endif
                          ;
                else if((type == meAtoms[meATOM_INCR]) && (fmt == 32) && (nitems > 0))
                {
                    nitems = ((long *) buff)[0] ;
                    XFree(buff) ;
                    /* always ksave, don't want to glue 'em together */
                    ksave();
                    if((dd = kaddblock(nitems)) != NULL)
                    {
                        dd[0] = '\0' ;
                        thisflag = CFKILL ;
                        meClipSize = nitems ;
                        meClipBuff = dd ;
                        /* Start the text transfer */
                        XDeleteProperty(mecm.xdisplay,mecm.xwindow,meAtoms[meATOM_COPY_TEXT]);
                        XFlush(mecm.xdisplay);
                        /* don't flag that we've got the clipboard yet cos we haven't */
                        break ;
                    }
                }
                else if((type == XA_STRING) && (fmt == 8) && (nitems > 0))
                {
                    if((klhead == NULL) || (klhead->kill == NULL) ||
                       (klhead->kill->next != NULL) ||
                       meStrncmp(klhead->kill->data,buff,nitems) ||
                       (klhead->kill->data[nitems] != '\0'))
                    {
                        /* always ksave, don't want to glue 'em together */
                        ksave();
                        if((dd = kaddblock(nitems)) != NULL)
                        {
                            meStrncpy(dd,buff,nitems) ;
                            dd[nitems] = '\0' ;
                        }
                        thisflag = CFKILL;
                    }
                }
                XFree(buff) ;
            }
            /* Always flag that we got the event */
            clipState |= CLIP_RECVED ;
            break ;
        }

    case PropertyNotify:
        if ((meClipSize > 0) && (event.xproperty.state == PropertyNewValue) &&
            (event.xproperty.atom == meAtoms[meATOM_COPY_TEXT]))
        {
            uint32 nitems, left;
            uint8 *buff ;
            Atom type ;
            int  fmt, ret ;

            ret = XGetWindowProperty(mecm.xdisplay,mecm.xwindow,meAtoms[meATOM_COPY_TEXT],
                                     0L,0x1fffffffL,True,AnyPropertyType,
                                     &type, &fmt, &nitems, &left, &buff) ;

            if((ret == Success) && (type == XA_STRING) && (fmt == 8))
            {
                if(nitems == 0)
                {
                    /* got all we're going to */
                    meClipSize = 0 ;
                    clipState |= CLIP_RECVED ;
                }
                else
                {
                    if(nitems >= meClipSize)
                    {
                        /* flag that we have the clipboard */
                        nitems = meClipSize ;
                        clipState |= CLIP_RECVED ;
                    }
                    memcpy(meClipBuff,buff,nitems) ;
                    meClipSize -= nitems ;
                    meClipBuff += nitems ;
                    *meClipBuff = '\0' ;
                    XFree(buff) ;
                }
            }
        }
        break ;
#endif
    case ClientMessage:
        if((event.xclient.message_type == meAtoms[meATOM_WM_PROTOCOLS]) &&
           (event.xclient.format == 32))
        {
            if(event.xclient.data.l[0] == meAtoms[meATOM_WM_DELETE_WINDOW])
            {
                /* Use the normal command to save buffers and exit
                 * if it doesn't exit then carry on as normal
                 * Must ensure we ask the user, not a macro
                 */
                int savcle ;
                savcle = clexec ;
                clexec = FALSE ;
                saveExitEmacs(0,1) ;
                clexec = savcle ;
            }
            else if(event.xclient.data.l[0] == meAtoms[meATOM_WM_SAVE_YOURSELF])
            {
                /* printf("Got a SAVE_YOURSELF\n") ;*/
            }
        }
        break ;
    default:
        /* all events selected by StructureNotifyMask
         * except ConfigureNotify are thrown away here,
         * since nothing is done with them */
        break;
    } /* end switch */
}


#endif



/*****************************************************************************
 *
 * TCAP FUNCTIONS
 *
 *****************************************************************************/



int
TCAPstart(void)
{
    char *p ;
    char tcbuf[1024];
    char *tv_stype;
    char err_str[72];
    int  ii ;

#ifdef _USG
#ifndef _XTERM
#ifdef _TERMIOS
    tcgetattr (0, &otermio);
#else
    ioctl(0, TCGETA, &otermio) ;
#endif
#endif
#endif
    /* default to use fonts - usually supports reverse */
    meSystemCfg &= ~meSYSTEM_RGBCOLOR ;
    meSystemCfg |= (meSYSTEM_CONSOLE|meSYSTEM_FONTS) ;
    if((tv_stype = meGetenv("TERM")) == NULL)
    {
        puts("Environment variable TERM not defined");
        meExit(1);
    }

    if((tgetent(tcbuf, tv_stype)) != 1)
    {
        sprintf(err_str, "Unknown terminal type [%s]", tv_stype);
        puts(err_str);
        meExit(1);
    }
    TCAPgetWinSize() ;
    TTmrow = TTnewHig ;
    TTmcol = TTnewWid ;

    TTnrow = TTmrow - 1 ;
    TTncol = TTmcol;

    /* Initialise the termcap strings */
    p = tcapbuf;
    for (ii = 0; ii < TCAPmax; ii++)
    {
        if (tcaptab[ii].type == TERMCAP_BOOLN)
            tcaptab[ii].code.value = tgetflag (tcaptab[ii].capKey);
        else if (tcaptab[ii].type == TERMCAP_DIGIT)
            tcaptab[ii].code.value = tgetnum (tcaptab[ii].capKey);
        else
        {
            tcaptab[ii].code.str = tgetstr(tcaptab[ii].capKey, &p);
            if ((tcaptab[ii].code.str == NULL) ||
                (tcaptab[ii].code.str[0] == '\0'))
                tcaptab[ii].code.str = NULL;
        }
    }
    
    /* Make sure that there was sufficient buffer space to process the strings */
    if (p >= &tcapbuf[TCAPSLEN])
    {
        sprintf(err_str, "%s termcap longer than %d characters",
                tv_stype, TCAPSLEN);
        puts(err_str);
        meExit(1);
    }
    
    /* Sort out the visibility flags. We must have both set or both NULL */
    if (tcaptab[TCAPcivis].code.str == NULL)
        tcaptab[TCAPcnorm].code.str = NULL;
    else if (tcaptab[TCAPcnorm].code.str == NULL)
        tcaptab[TCAPcivis].code.str = NULL;
    
    /* Determine if there is a mechanism to enable and disable the automatic
     * margins. If there is then disable them now */
    if (tcaptab[TCAPam].code.value == 1)
    {
        /* Try to disable the margins */
        if (tcaptab[TCAPsmam].code.str != NULL)
        {
            putpad (tcaptab[TCAPsmam].code.str);
            tcaptab[TCAPam].code.value = tgetnum (tcaptab[TCAPam].capKey);
            TTaMarginsDisabled = 1;
        }
    }
    
    /* If there is a new line glitch and we have automargins then it is
     * dangerous for us to use the last line as we cause a scroll that we
     * cannot easily correct. If this is the case then reduce the number of
     * lines by 1. */
    if ((tcaptab[TCAPam].code.value != 0) &&
        (tcaptab[TCAPxenl].code.value == 0) &&
        (TTaMarginsDisabled == 0))
    {
        TTnewHig-- ;
        TTmrow = TTnewHig ;
        TTnrow = TTmrow - 1 ;
    }
    
    /* We rely on Clear and Move <x><y>, make sure that these exist */
    if ((tcaptab[TCAPclear].code.str == NULL) ||
        (tcaptab[TCAPcup].code.str == NULL))
    {
        sprintf(err_str, "%s termcap lacks %s or %s entry",
                tv_stype,tcaptab[TCAPclear].capKey, tcaptab[TCAPcup].capKey);
        puts(err_str);
        meExit(1);
    }
    
    /* Try and setup some of the standard keys like the cursor keys */
    {
        char buf[20] ;
        uint16 ii, sl[20] ;

        for(ii=0 ; ii<SKEY_MAX ; ii++)
        {
            if(tcapSpecKeyDefs[ii] != NULL)
                translateKeyAdd(&TTtransKey,charListToShorts(sl,tcapSpecKeyDefs[ii]),
                                TTtransKey.time,sl,ME_SPECIAL|ii) ;
        }
        for(ii=0 ; ii<SKEY_MAX ; ii++)
        {
            p = buf ;
            if((tcapSpecKeyStrs[ii] != NULL) && ((p=tgetstr(tcapSpecKeyStrs[ii], &p)) != NULL))
                translateKeyAdd(&TTtransKey,charListToShorts(sl,(uint8 *) p),
                                TTtransKey.time,sl,(ii|ME_SPECIAL)) ;
            else
                tcapSpecKeyStrs[ii] = NULL ;
        }
        /* KEY_PPAGE, sent by previous-page key */
        if((tcapSpecKeyStrs[SKEY_page_up] == NULL) && ((p=tgetstr("kP", &p)) != NULL))
        {
            tcapSpecKeyStrs[SKEY_page_up] = "kP" ;
            translateKeyAdd(&TTtransKey,charListToShorts(sl,(uint8 *) p),
                            TTtransKey.time,sl,(SKEY_page_up|ME_SPECIAL)) ;
        }
        /* KEY_NPAGE, sent by next-page key */
        if((tcapSpecKeyStrs[SKEY_page_down] == NULL) && ((p=tgetstr("kN", &p)) != NULL))
        {
            tcapSpecKeyStrs[SKEY_page_down] = "kN" ;
            translateKeyAdd(&TTtransKey,charListToShorts(sl,(uint8 *) p),
                            TTtransKey.time,sl,(SKEY_page_down|ME_SPECIAL)) ;
        }
    }
    /* Switch off the vertical window scroll bar */
    gsbarmode &= ~WMVBAR ;
    {
#ifdef _POSIX_SIGNALS
        struct sigaction sa ;
        sigemptyset(&sa.sa_mask) ;
        sa.sa_flags=0 ;
        sa.sa_handler=sigSize ;
        sigaction(SIGWINCH,&sa,NULL) ;
#else
        signal (SIGWINCH, sigSize);
#endif
    }
    return TCAPopen() ;
}

#ifdef _XTERM

/*
 * TTinitMouse
 * Sort out what to do with the mouse buttons.
 */
void
TTinitMouse(void)
{
    if(meSystemCfg & meSYSTEM_CONSOLE)
        meMouseCfg &= ~meMOUSE_ENBLE ;
    else if(meMouseCfg & meMOUSE_ENBLE)
    {
        static uint8  meCurCursor=0 ;
        static Cursor meCursors[meCURSOR_COUNT]={0,0,0,0,0,0,0} ;
        static uint8  meCursorChar[meCURSOR_COUNT]={
            0,XC_arrow,XC_xterm,XC_crosshair,XC_hand2,XC_watch,XC_pirate} ;
        uint8 cc ;
        int b1, b2, b3 ;

        if(meMouseCfg & meMOUSE_SWAPBUTTONS)
            b1 = 3, b3 = 1 ;
        else
            b1 = 1, b3 = 3 ;
        if((meMouseCfg & meMOUSE_NOBUTTONS) > 2)
            b2 = 2 ;
        else
            b2 = b3 ;
        mouseKeys[1] = b1 ;
        mouseKeys[2] = b2 ;
        mouseKeys[3] = b3 ;

        cc = (uint8) ((meMouseCfg & meMOUSE_ICON) >> 16) ;
        if(cc >= meCURSOR_COUNT)
            cc = 0 ;
        if(cc != meCurCursor)
        {
            if(meCurCursor)
            {
                XUndefineCursor(mecm.xdisplay,mecm.xwindow) ;
                meCurCursor = 0 ;
            }
            if(cc && ((meCursors[cc] != 0) ||
                      ((meCursors[cc] = XCreateFontCursor(mecm.xdisplay,meCursorChar[cc])) != 0)))
            {
                XDefineCursor(mecm.xdisplay,mecm.xwindow,meCursors[cc]) ;
                meCurCursor = cc ;
            }
        }
    }
}

/* XTERMsetFont; Determine the font that we shall use. Use the base
 * string provided by the user (or system) and determine if there
 * are any derivatives of the font */
static int
XTERMsetFont (char *fontName)
{
    int ii;
    XFontStruct *font ;

    if (fontName == NULL)
        fontName = "-*-fixed-medium-r-normal--15-*-*-*-c-90-iso8859-1";

    /* Load the basic font into the server */
    if((font=XLoadQueryFont(mecm.xdisplay,fontName)) == NULL)
        return FALSE ;
    mecm.ascent    = font->ascent ;
    mecm.descent   = font->descent ;
    mecm.fdepth    = mecm.ascent + font->descent ;
    mecm.fwidth    = font->max_bounds.width ;
    mecm.fhdepth   = mecm.fdepth >> 1 ;
    mecm.fhwidth   = mecm.fwidth >> 1 ;
    mecm.underline = mecm.fdepth - mecm.ascent - 1;

    if((mecm.fadepth = mecm.fwidth+1) > mecm.fdepth)
        mecm.fadepth = mecm.fdepth ;

    sizeHints.height_inc = mecm.fdepth ;
    sizeHints.width_inc  = mecm.fwidth ;
    sizeHints.min_height = mecm.fdepth*4 ;
    sizeHints.min_width  = mecm.fwidth*10 ;
    sizeHints.base_height = mecm.fdepth ;
    sizeHints.base_width  = mecm.fwidth ;

    /* Clean up the font table for the existing font. Unload all of the
     * previously loaded fonts */
    for (ii = 0; ii < meFONT_MAX ; ii++)
    {
        if(mecm.fontFlag[ii])
            XUnloadFont (mecm.xdisplay, mecm.fontTbl[ii]);
        mecm.fontTbl[ii] = BadName;
        mecm.fontFlag[ii] = 0 ;
    }

    /* Assign the base font */
    mecm.fontTbl[0] = font->fid ;
    mecm.fontFlag[0] = 1;
    mecm.font = 0 ;
    mecm.fontId = font->fid ;

    XFreeFontInfo(NULL,font,1) ;

    if(mecm.fontName != NULL)
        free(mecm.fontName) ;

    if((meSystemCfg & meSYSTEM_FONTS) &&
       ((mecm.fontName=meStrdup(fontName)) != NULL))
    {
        uint8 cc, *p = mecm.fontName ;

        /* Break up the font. This must be the full X-Window font definition, we
         * do not understand the X-Windows wildcards and aliases. Lifes too short
         * to sit down and address these now !! */
        for (ii = 0 ; (ii < 14) ; ii++)
        {
            if (*p != '-')
                break;
            *p++ = '\0' ;
            mecm.fontPart[ii] = p ;
            while (((cc = *p) != '\0') && (cc != '-'))
                p++;
        }
        if ((ii != 14) || (*p != '\0'))
        {
            free(mecm.fontName) ;
            mecm.fontName = NULL ;
        }
    }
    else
        mecm.fontName = NULL ;

    return TRUE;
}

/* Heres the xterm equivalent, note that its done in two stages */
static int
XTERMstart(void)
{
    XrmDatabase  rdb ;
    XrmValue     retVal ;
    Pixmap       iconPixmap=0 ;
    char        *retType ;
    char        *xdefs ;
    int          xx, yy ;
    unsigned int ww, hh  ;
    char        *ss ;

    XSetLocaleModifiers ("");
    {
        char *disp ;
        if((disp=meGetenv("DISPLAY")) == NULL)
            disp = ":0.0" ;
        if((mecm.xdisplay = XOpenDisplay(disp)) == NULL)
        {
            fprintf(stderr,"MicroEmacs: Failed to open DISPLAY \"%s\"\n",disp) ;
            exit(1) ;
        }
    }
    meStdin = ConnectionNumber(mecm.xdisplay);
    xscreen = DefaultScreen(mecm.xdisplay) ;
    xcmap   = DefaultColormap(mecm.xdisplay,xscreen) ;

    sizeHints.flags = PSize | PResizeInc | PMinSize | PBaseSize ;
    sizeHints.max_height = DisplayHeight(mecm.xdisplay,xscreen) ;
    sizeHints.max_width  = DisplayWidth(mecm.xdisplay,xscreen) ;

    XrmInitialize ();
    xdefs = XResourceManagerString(mecm.xdisplay);
    if (xdefs != NULL)
        rdb = XrmGetStringDatabase (xdefs);
    else
    {
        char buff[1048] ;
        meStrcpy(buff,(homedir != NULL) ? homedir:(uint8 *)".") ;
        meStrcat(buff,"/.Xdefaults") ;
        rdb = XrmGetFileDatabase(buff);
    }
    if(XrmGetResource(rdb,"MicroEmacs.font","MicroEmacs.Font",&retType,&retVal) &&
       !strcmp(retType,"String"))
        ss = retVal.addr ;
    else
        ss = NULL ;

    /* Load the font into the system */
    if (XTERMsetFont (ss) == FALSE)
        return FALSE ;

    /* Set the default geometry, then look for an override */
    ww = 80 ;
    hh = 50 ;
    xx = 0 ;
    yy = 0 ;
    if(XrmGetResource(rdb,"MicroEmacs.geometry","MicroEmacs.Geometry",&retType,&retVal) &&
       !strcmp(retType,"String"))
    {
        sizeHints.flags = USSize | PResizeInc | PMinSize | PBaseSize ;
        if(sscanf(retVal.addr,"%dx%d%d%d",&ww,&hh,&xx,&yy) > 2)
            sizeHints.flags |= USPosition ;
    }
    else
        sizeHints.flags = PSize | PResizeInc | PMinSize | PBaseSize ;
    
    ww *= mecm.fwidth ;
    hh *= mecm.fdepth ;
    if(hh > sizeHints.max_height)
        hh = (sizeHints.max_height/mecm.fdepth) * mecm.fdepth ;
    if(ww > sizeHints.max_width)
        ww = (sizeHints.max_width/mecm.fwidth) * mecm.fwidth ;
    if(xx < 0)
        xx = sizeHints.max_width + xx - ww ;
    if(yy < 0)
        yy = sizeHints.max_height + yy - hh ;
    sizeHints.x = xx ;
    sizeHints.y = yy ;
    sizeHints.height = hh ;
    sizeHints.width  = ww ;
    TTnrow = TTmrow = hh / mecm.fdepth ;
    TTncol = TTmcol = ww / mecm.fwidth ;
    TTnrow-- ;
    mecm.xwindow = XCreateSimpleWindow(mecm.xdisplay,RootWindow(mecm.xdisplay,xscreen),xx,yy,ww,hh,0,
                                 WhitePixel(mecm.xdisplay,xscreen),BlackPixel(mecm.xdisplay,xscreen)) ;

    XSetWMNormalHints(mecm.xdisplay,mecm.xwindow,&sizeHints);
    /* Map the window a.s.a.p. cos officially we can't draw to the window until we
     * get an Expose event. As this is not checked for by mapping now there is
     * less chance of this being a problem.
     */
    XMapWindow(mecm.xdisplay,mecm.xwindow) ;

    {
        static char* meAtomNames[7] = {
            "WM_DELETE_WINDOW",
            "WM_SAVE_YOURSELF",
            "WM_PROTOCOLS",
            "__COPY_TEXT",
            "INCR",
            "MULTIPLE",
            "TARGETS"
        } ;
        int ii ;
        for(ii=0 ; ii<7 ; ii++)
            meAtoms[ii] = XInternAtom(mecm.xdisplay,meAtomNames[ii], FALSE);
        meAtoms[ii] = XA_STRING ;
    }
    XSetWMProtocols(mecm.xdisplay,mecm.xwindow,meAtoms,2) ;

    if(XrmGetResource(rdb,"MicroEmacs.name","MicroEmacs.Name",&retType,&retVal) &&
       !strcmp(retType,"String") &&
       ((ss = meStrdup(retVal.addr)) != NULL))
        meName = ss ;
    TTtitleText(NULL) ;

    if(XrmGetResource(rdb,"MicroEmacs.iconname","MicroEmacs.IconName",&retType,&retVal) &&
       !strcmp(retType,"String"))
        ss = retVal.addr ;
    else
        ss = meName ;
    XSetIconName(mecm.xdisplay,mecm.xwindow,ss);

    if(XrmGetResource(rdb,"MicroEmacs.icon","MicroEmacs.Icon",&retType,&retVal) &&
       !strcmp(retType,"String"))
    {
        unsigned int i1,i2 ;
        int          i3,i4 ;

        switch(XReadBitmapFile(mecm.xdisplay,mecm.xwindow,retVal.addr,&i1,&i2,
                               &iconPixmap,&i3,&i4))
        {
        case BitmapOpenFailed:
            fprintf(stderr,"Failed to open bitmap %s\n",retVal.addr) ;
            break ;
        case BitmapFileInvalid:
            fprintf(stderr,"Invalid bitmap file %s\n",retVal.addr) ;
            break ;
        }
    }
    else
        iconPixmap = XCreatePixmapFromBitmapData
              (mecm.xdisplay,mecm.xwindow,(char *) iconBits,iconWidth,iconHeight,
               WhitePixel(mecm.xdisplay,xscreen),BlackPixel(mecm.xdisplay,xscreen),1);

    {
        XWMHints wmHints ;

        wmHints.flags = (InputHint|StateHint) ;
        wmHints.input = True ;
        wmHints.initial_state = NormalState ;
        if(iconPixmap != 0)
        {
            wmHints.flags |= IconPixmapHint ;
            wmHints.icon_pixmap = iconPixmap ;
        }
        XSetWMHints(mecm.xdisplay,mecm.xwindow,&wmHints) ;
    }

    mecm.xgcv.font = mecm.fontId ;
    mecm.xgc = XCreateGC(mecm.xdisplay,mecm.xwindow,GCFont,&mecm.xgcv) ;
    /* To get the mouse positional information then we register for
     * "PointerMotionMask" events. */
    /* olwm on sunos has a Focus bug, it does not sent the FocusIn
     * and FocusOut events properly, but as it does send EnterNotify
     * and LeaveNotify events. use this to check the Focus state
     * as well - SWP 17/8/99
     */
    XSelectInput(mecm.xdisplay,mecm.xwindow,
                 (ExposureMask|StructureNotifyMask|KeyPressMask|ButtonPressMask|
                  ButtonReleaseMask|PointerMotionMask|FocusChangeMask|
                  PropertyChangeMask|LeaveWindowMask|EnterWindowMask)) ;

    /* Free off the resource database */
    XrmDestroyDatabase(rdb) ;
	
    /* When the xterm version of ME has to die using meDie (using killing it
     * etc) it cannot write to the X-Window as it may have gone. Therefore the
     * die messages "*** Emergency..." are written to the terminal. This is
     * done by setting the meSystem variable to a value which make it use
     * TCAP. But TCAP has not been initialise and mlwrite will use TCAPmove so
     * we must initialise the TCAPcup (cursor move string) to something to
     * stop it core-dumping.
     */
    tcaptab[TCAPcup].code.str = "" ;
    
    return TRUE ;
}

int
XTERMstartStage2(void)
{
    Window fwin ;
    int rtr ;

    /* see if we are the current focus */
    XGetInputFocus(mecm.xdisplay,&fwin,&rtr) ;
    TTfocus = (fwin == mecm.xwindow) ;
    /* Create the cursor */
    XTERMshowCur();                       /* Show the cursor */
    return TRUE ;
}


/* Function only needed if we have xterm and termcap, its purpose
 * is to work out where to call XTERMstart or TCAPstart
 */
int
TTstart(void)
{
    char *tt ;

#ifdef _USG
#ifdef _TERMIOS
    tcgetattr (0, &otermio);
#else
    ioctl(0, TCGETA, &otermio) ;
#endif
#endif

    if((meSystemCfg & meSYSTEM_CONSOLE) ||
       (meGetenv("DISPLAY") == NULL) ||
       (((tt = meGetenv("TERM")) != NULL) &&
        (tt[0] == 'v') && (tt[1] == 't')))
        return TCAPstart() ;
    return XTERMstart() ;
}
#endif

int
TCAPopen(void)
{
#ifdef _USG
    ntermio = otermio;

/*    ntermio.c_lflag &= ~(ISIG|ICANON|ECHO);*/
    ntermio.c_lflag &= ~(ISIG|ICANON|ECHO|IEXTEN);
/*    ntermio.c_iflag &= ~(IXON|ICRNL);*/
    ntermio.c_iflag &= ~(IXON|INLCR|INPCK|ICRNL|ISTRIP);
#ifndef _TERMIOS
    /* Additional SVR4 settings for termio */
    ntermio.c_iflag &= ~(IXANY);
#endif
#ifdef TAB3
    ntermio.c_oflag &= ~(ONLCR|TAB3);
#else
    ntermio.c_oflag &= ~ONLCR ;
#endif
    ntermio.c_iflag |= IGNBRK ;
    ntermio.c_cc[VMIN] = 1 ;
    ntermio.c_cc[VTIME] = 0 ;
/*    ntermio.c_cc[VINTR] = 7 ;*/
#ifdef _TERMIOS
    tcsetattr (meStdin, TCSAFLUSH, &ntermio);
#else
    ioctl(0, TCSETA, &ntermio); /* and activate them */
#endif
#endif /* _USG */

#ifdef _BSD
    /* Set up the TTY states */
    gtty (0, &osgttyb);
    memcpy (&nsgttyb,  &osgttyb,  sizeof (struct sgttyb)) ;
#ifdef _BSD_CBREAK
    /* Use CBREAK rather than raw mode */
    nsgttyb.sg_flags &= ~(ECHO|CRMOD) ;
    nsgttyb.sg_flags |= CBREAK ;
#else
    nsgttyb.sg_flags &= ~(ECHO|CRMOD) ;
    nsgttyb.sg_flags |= RAW ;
#endif
    stty (0, &nsgttyb);

    /* Set up the line disciplines */
#ifdef _BSD_CBREAK
    ioctl (0, TIOCGETC, &otchars) ;
    memcpy (&ntchars,  &otchars,  sizeof (struct tchars)) ;
    ntchars.t_intrc  = -1 /*07*/;       /* Interrupt character ^G */
    ntchars.t_quitc  = -1 ;             /* Quit - disabled - disabled */
    ntchars.t_startc = -1 /*021*/;      /* Start character ^Q - disabled */
    ntchars.t_stopc  = -1 /*023*/;      /* Stop character ^S - disabled */
    ntchars.t_eofc   = -1 ;             /* End of file ^D - disabled */
    ntchars.t_brkc   = -1 ;             /* End of line - diabled */
    ioctl (0, TIOCSETC, &ntchars) ;
#endif

    /* Set up the local line disciplines special */
    ioctl (0, TIOCGLTC, &oltchars) ;
    memcpy (&nltchars, &oltchars, sizeof (struct ltchars)) ;
    nltchars.t_suspc  = -1 ;            /* Disable the lot */
    nltchars.t_dsuspc = -1 ;
    nltchars.t_rprntc = -1 ;
    nltchars.t_flushc = -1 ;
    nltchars.t_werasc = -1 ;
    nltchars.t_lnextc = -1 ;
    ioctl (0, TIOCSLTC, &nltchars) ;

    /* Set up the type ahead buffer. There are some inconsistancies here on
     * various systems so it is better simply to define a default output
     * buffer size. */
    setbuffer (stdout, termOutBuf, sizeof (termOutBuf));
#endif /* _BSD */

#if (defined(NTTYDISC) && defined(TIOCSETD))
     /* Set the line discipline to the new Berkley standard */
     {
         int ldisc = NTTYDISC;
         ioctl (0, TIOCSETD, &ldisc);
     }
#endif /*  NTTYDISC/TIOCSETD */
    
    /* If automatic margins are enabled then try to disable them */
    if ((tcaptab[TCAPam].code.value) &&
        (tcaptab[TCAPrmam].code.str != NULL) &&
        (TTaMarginsDisabled == 0))
    {
        /* Automatic margins are enabled - disable */
        tputs(tcaptab[TCAPrmam].code.str,1,putchar) ;
        tcaptab[TCAPam].code.value = tgetnum(tcaptab[TCAPam].capKey);
        TTaMarginsDisabled = 1;
    }
    
    /* Success */
    return TRUE ;
}

/*
 * This function gets called just before we go back home to the command
 * interpreter. On VMS it puts the terminal back in a reasonable state.
 * Another no-operation on CPM.
 */
int
TCAPclose(void)
{
    mlerase(MWERASE|MWCURSOR);
    TCAPschemeReset() ;
    
#if COLOR
    /* Restore the original colors */
    if (tcaptab[TCAPoc].code.str != NULL)/* Restore colors */
        putpad(tcaptab[TCAPoc].code.str) ;
    if (tcaptab[TCAPop].code.str != NULL)/* Restore pairs */
        putpad(tcaptab[TCAPop].code.str) ;
#endif
    
    /* If automatic margins are disabled then try to enable them again */
    if (TTaMarginsDisabled != 0)
    {
        if (tcaptab[TCAPsmam].code.str != NULL)
        {
            putpad(tcaptab[TCAPsmam].code.str);
            tcaptab[TCAPam].code.value = tgetnum (tcaptab[TCAPam].capKey);
        }
        TTaMarginsDisabled = 0;
    }
    
    /* Restore the terminal modes */
#ifdef _USG
#ifdef _TERMIOS
    return tcsetattr (meStdin, TCSAFLUSH, &otermio);
#else
    return ioctl(0, TCSETA, &otermio);  /* restore terminal settings */
#endif
#endif /* _USG */
#ifdef _BSD
    /* Restore the terminal settings */
    stty (0, &osgttyb);
#ifdef _BSD_CBREAK
    ioctl (0, TIOCSETC, &otchars) ;
#endif
    ioctl (0, TIOCSLTC, &oltchars) ;
    return TRUE;
#endif
}


static void
waitForEvent(void)
{
    static int lost=0 ;
#ifdef _IPIPES
    meIPIPE *ipipe, *pp ;
#endif
    fd_set select_set;
    int pfd ;

    /* While no input keys and ipipe or alarm signals. */
    for(;;)
    {
        if(TTahead() ||
#ifdef _CLIPBRD
           (clipState & CLIP_RECVED) ||
#endif
           isTimerExpired(AUTOS_TIMER_ID) ||
           isTimerExpired(CALLB_TIMER_ID) ||
           isTimerExpired(CURSOR_TIMER_ID) ||
#ifdef _URLSUPP
           isTimerExpired(SOCKET_TIMER_ID) ||
#endif
           (sgarbf == TRUE))
            break ;
        TTdieTest() ;
        FD_ZERO(&select_set);
        FD_SET(meStdin, &select_set);
        pfd = meStdin ;
#ifdef _IPIPES
        ipipe = ipipes ;
        while(ipipe != NULL)
        {
            pp = ipipe->next ;
            if(ipipe->pid < 0)
                ipipeRead(ipipe) ;
            else
            {
                FD_SET(ipipe->rfd, &select_set);
                if(ipipe->rfd > pfd)
                    pfd = ipipe->rfd ;
            }
            ipipe = pp ;
        }
#endif
        pfd++ ;
        if(select(pfd,&select_set,NULL,NULL,NULL) >= 0)
        {
#ifdef _IPIPES
            ipipe = ipipes ;
            while(ipipe != NULL)
            {
                pp = ipipe->next ;
                /* See if theres anything to read, if so read it */
                if(FD_ISSET(ipipe->rfd,&select_set) || (ipipe->pid < 0))
                {
                    /* reset the lost */
                    lost = 0 ;
                    ipipeRead(ipipe) ;
                }
                ipipe = pp ;
            }
#endif
            if(FD_ISSET(meStdin,&select_set))
            {
                /* this is a check to see if we've lost stdin,
                 * if we have there's nothing we can do other than die.
                 * This is common when the X-window is destroyed using
                 * the window manager.
                 */
#ifdef FIONREAD
                int x;      /* holds # of pending chars */
#endif
                if(!
#ifdef FIONREAD
                   /* if ioctl fails don't die! */
                   ((ioctl(meStdin,FIONREAD,&x) < 0) ? 1 : x)
#else
                   ioctl(meStdin, FIORDCHK,0)
#endif
                   )
                {
                    /* Found this test can go wrong in some strange case, which
                     * is a real bummer as me now dies! as a fix, and I know its
                     * a bit of a bodge, only die is this happens 3 times on the
                     * trot
                     */
                    if(lost++ == 2)
                        /* If there's nothing to read, yet select flags meStdin as
                         * triggered then we've lost the input - Die...
                         * Unfortunately we must be very careful on what we do, we
                         * cannot write out to the x-window cause its gone, but
                         * meDie takes care of this
                         */
                        meDie() ;
                }
                else
                    lost = 0 ;
            }
        }
        else
            /* Premeture exit - probably a signal for an alarm */
            break;
    }
}

void
TTwaitForChar(void)
{
#if MOUSE
    /* If no keys left then if theres currently no mouse timer and
     * theres a button press (No mouse-time key) then check for a
     * time-mouse-? key, if found add a timer start a mouse checking
     */
    if(!isTimerSet(MOUSE_TIMER_ID) && (mouseState != 0))
    {
        uint16 mc ;
        uint32 arg ;
        mc = ME_SPECIAL | mouseKeyState | (SKEY_mouse_time+mouseKeys[mouseButtonGetPick()]) ;
        /* mouse-time bound ?? */
        if((!TTallKeys && (decode_key(mc,&arg) != -1)) || (TTallKeys & 0x2))
        {
            /* Start a timer and move to timed state 1 */
            /* Start a new timer to clock in at 'delay' intervals */
            /* printf("Setting mouse timer for delay %d\n",delaytime) ;*/
            timerSet(MOUSE_TIMER_ID,-1,delaytime);
        }
    }
#endif
    /* IDLE TIME: Check the idle time events */
    if(kbdmode == KBD_IDLE)
        /* Check the idle event */
        doIdlePickEvent() ;
    for(;;)
    {
#if (defined _XTERM) && (defined _LINUX)
        if(!(meSystemCfg & meSYSTEM_CONSOLE))
        {
            /* Make a call to XSync(). This ensures that the X-Server has
             * caught up with the client (us !!). On operations such as
             * scrolling etc. then the X-Server tends to lag behind giving
             * a very 'rubbery' feel to the scroll bars. This reduces that
             * effect, allowing the server to process all of our outstanding
             * events (typically mecm.xdisplay requests). */
            XSync (mecm.xdisplay, FALSE);
        }
#endif
        /* Must only do one at a time else if you get A key while autosaving
         * the key can be proccessed by TTahead and then you sit and wait
         * in TCAPwait
         */
        if(isTimerExpired(AUTOS_TIMER_ID))  /* Alarm expired ?? */
            autoSaveHandler();              /* Initiate an auto save */
        if(isTimerExpired(CALLB_TIMER_ID))  /* Alarm expired ?? */
            callBackHandler();              /* Initiate any callbacks */
        if(isTimerExpired(CURSOR_TIMER_ID)) /* Alarm expired ?? */
            TThandleBlink(0);               /* Initiate a cursor blink */
#ifdef _URLSUPP
        if(isTimerExpired(SOCKET_TIMER_ID)) /* socket connection time-out */
            ffCloseSockets(1) ;
#endif
        if(TTahead())
            break ;
        if(sgarbf == TRUE)
        {
            update(FALSE) ;
            mlerase(MWCLEXEC) ;
        }
        waitForEvent() ;
    }
}


void
TCAPhideCur(void)
{
    /* If we have a cursor hide capability then use it */
    if (tcaptab[TCAPcivis].code.str != NULL)
    {
        if (TTcursorVisible != 0)
        {
            putpad (tcaptab[TCAPcivis].code.str);
            TTcursorVisible = 0;
        }
    }
    else
    {
        /* Otherwise, move the cursor off screen (bottom right) */
        TCAPmove(TTnrow-1,TTncol-1) ;
    }
}

void
TCAPshowCur(void)
{
    if((TTcurr <= TTnrow) && (TTcurc < TTncol))
    {
        /* Make sure that the cursor is visible */
        if ((TTcursorVisible == 0) && (tcaptab[TCAPcnorm].code.str != NULL))
        {
            putpad (tcaptab[TCAPcnorm].code.str);
            TTcursorVisible = 1;
        }
        TCAPmove(TTcurr,TTcurc) ;
    }
    /* Cursor is off the screen - hide it */
    else if (tcaptab[TCAPcivis].code.str != NULL)
    {
        if (TTcursorVisible != 0)
        {
            putpad (tcaptab[TCAPcivis].code.str);
            TTcursorVisible = 0;
        }
    }
    else
    {
        /* move the cursor off screen (bottom right) */
        TCAPmove(TTnrow-1,TTncol-1) ;
    }
}

#if COLOR

int
TCAPaddColor(uint8 index, uint8 r, uint8 g, uint8 b)
{
    uint8 *ss ;
    int ii, jj, idif, jdif ;

    jdif = 256*256*3 ;                  /* Smallest least squares. */
    ss = tcapColors ;
    
    /* To find the nearest color then use a least squares method. This
     * produces a better approximation than a straight forward color
     * differencing algorithm as it takes into account the variance. */
    for(ii=0 ; ii<tcapNumColors ; ii++)
    {
        int delta;
        
        delta = r - *ss++;
        idif  = (delta * delta) ;
        delta = g - *ss++;
        idif += (delta * delta) ;
        delta = b - *ss++ ;
        idif += (delta * delta) ;
        if(idif < jdif)
        {
            jdif = idif ;
            jj = ii ;
        }
    }

    if(noColors <= index)
    {
        colTable = (uint32 *) realloc(colTable,(index+1)*sizeof(uint32)) ;
        memset(colTable+noColors,0, (index-noColors+1)*sizeof(uint32)) ;
        noColors = index+1 ;
    }
    colTable[index] = jj ;

    return TRUE ;
}

#endif

static uint8 oschemeFcol=meCOLOR_INVALID ;
static uint8 oschemeBcol=meCOLOR_INVALID ;
static uint8 oschemeFont=0 ;
static uint8 oschemeFntr=0 ;

void
TCAPschemeSet(meSCHEME scheme)
{
    meSTYLE nstyle ;

    nstyle = meSchemeGetStyle(scheme) ;
    
    /* Termcap fonts */
#ifdef _TCAPFONT
    if(meSystemCfg & meSYSTEM_FONTS)
    {
        uint8 font ;
        
        font = meStyleGetFont(nstyle) ;
        if(meSchemeTestNoFont(scheme))
            font &= ~(meFONT_BOLD|meFONT_ITALIC|meFONT_UNDERLINE) ;

        if(oschemeFont != font)
        {
            if(oschemeFntr)
            {
                /* Remove the old font attributes. We cannot guarantee the
                 * state of some attributes so it is best to turn all off and
                 * start again, */
                if ((oschemeFont & meFONT_ITALIC) && (tcaptab[TCAPritm].code.str != NULL))
                    putpad (tcaptab[TCAPritm].code.str);
                if ((oschemeFont & meFONT_UNDERLINE) && (tcaptab[TCAPrmul].code.str != NULL))
                    putpad (tcaptab[TCAPrmul].code.str);
                if ((oschemeFont & (meFONT_BOLD|meFONT_REVERSE|meFONT_LIGHT)) &&
                    (tcaptab[TCAPsgr0].code.str != NULL))
                    putpad (tcaptab[TCAPsgr0].code.str);
                oschemeFcol = meCOLOR_INVALID ;
                oschemeBcol = meCOLOR_INVALID ;
                oschemeFntr = 0 ;
            }
            oschemeFont = font;
            if (font & meFONT_MASK)
            {
                oschemeFntr = 1 ;
                /* Apply the modes */
                if ((font & meFONT_BOLD) && (tcaptab[TCAPbold].code.str != NULL))
                    putpad(tcaptab[TCAPbold].code.str);
                if ((font & meFONT_ITALIC) && (tcaptab[TCAPsitm].code.str != NULL))
                    putpad (tcaptab[TCAPsitm].code.str);
                if ((font & meFONT_LIGHT) && (tcaptab[TCAPdim].code.str != NULL))
                    putpad (tcaptab[TCAPdim].code.str);
                if ((font & meFONT_REVERSE) && (tcaptab[TCAPrev].code.str != NULL))
                    putpad (tcaptab[TCAPrev].code.str);
                if ((font & meFONT_UNDERLINE) && (tcaptab[TCAPsmul].code.str != NULL))
                    putpad (tcaptab[TCAPsmul].code.str);
            }
        }
    }
#endif /* _TCAPFONT */
    
    /* Termcap coloring */
#if COLOR    
    if(meSystemCfg & meSYSTEM_ANSICOLOR)
    {
        uint8 col ;
        
        /* Foreground color */
        col = colTable[meStyleGetFColor(nstyle)] ;
        if(oschemeFcol != col)
        {
            if (tcaptab[TCAPsetaf].code.str != NULL)
            {
                /* Have a termcap entry for color ?? */
#ifdef _USETPARM
                putpad (tparm (tcaptab[TCAPsetaf].code.str, col));
#else
                putpad (meTParm (tcaptab[TCAPsetaf].code.str, col));
#endif /* _USETPARM */
            }
            else
            {
                /* Try our ANSI color */
                FCOLSTR[3]= (col & 0x07) + 48;
                putpad (FCOLSTR);
            }
            oschemeFcol = col ;
        }
        
        /* Background color */
        col = colTable[meStyleGetBColor(nstyle)] ;
        if(oschemeBcol != col)
        {
            if (tcaptab[TCAPsetab].code.str != NULL)
            {
                /* Have a termcap entry for color ?? */
#ifdef _USETPARM
                putpad (tparm (tcaptab[TCAPsetab].code.str, col));
#else
                putpad (meTParm (tcaptab[TCAPsetab].code.str, col));
#endif /* _USETPARM */
            }
            else
            {
                BCOLSTR[3]=(col & 0x07) + 48 ;
                putpad (BCOLSTR);
            }
            oschemeBcol = col ;
        }
    }
#endif /* COLOR */
}

void
TCAPschemeReset(void)
{
#ifdef _TCAPFONT
    if(oschemeFntr)
    {
        /* Remove the old font attributes. We cannot guarantee the state of
         * some attributes so it is best to turn all off and start again, */
        if ((oschemeFont & meFONT_ITALIC) && (tcaptab[TCAPritm].code.str != NULL))
            putpad (tcaptab[TCAPritm].code.str);
        if ((oschemeFont & meFONT_UNDERLINE) && (tcaptab[TCAPrmul].code.str != NULL))
            putpad (tcaptab[TCAPrmul].code.str);
        if ((oschemeFont & (meFONT_BOLD|meFONT_REVERSE|meFONT_LIGHT)) &&
            (tcaptab[TCAPsgr0].code.str != NULL))
            putpad (tcaptab[TCAPsgr0].code.str);
        oschemeFntr = 0 ;
    }
#endif
    
#if COLOR
    if(meSystemCfg & meSYSTEM_ANSICOLOR)
    {
        if ((oschemeFcol != meCOLOR_INVALID) || (oschemeBcol != meCOLOR_INVALID))
        {
            /* Disable the color mode */
            if (tcaptab[TCAPsgr0].code.str != NULL)
                putpad (tcaptab[TCAPsgr0].code.str);
            else 
                putpad (RCOLSTR);
        }
    }
#endif
    /* Reset the fonts regardless of the mode */
    oschemeFont = 0 ;
    oschemeFcol = meCOLOR_INVALID ;
    oschemeBcol = meCOLOR_INVALID ;
}

void
TCAPmove(int row, int col)
{
    /* We really need to perform this operation without cup, hence the
     * movement to a function from a macro. In addition we should really be
     * checking if we can move with attributes enabled, if this is not the
     * case then we need to reset out modes, move and then re-apply. The API
     * would be better served with a TCAPmoveScheme() where we could set the
     * next scheme at the same time. Keep this for a rainy day !! */ 
    tputs(tgoto(tcaptab[TCAPcup].code.str,col,row),1,putchar);
}

void
TTsleep(int  msec, int  intable)
{
    if(intable && ((kbdmode == PLAY) || (clexec == TRUE)))
        return ;

    /* Don't actually need the abs time as this will remain the next alarm */
    timerSet(SLEEP_TIMER_ID,-1,msec);
    do
    {
        if(isTimerExpired(AUTOS_TIMER_ID))  /* Alarm expired ?? */
            autoSaveHandler();              /* Initiate an auto save */
        if(isTimerExpired(CALLB_TIMER_ID))  /* Alarm expired ?? */
            callBackHandler();              /* Initiate any callbacks */
        if(isTimerExpired(CURSOR_TIMER_ID)) /* Alarm expired ?? */
            TThandleBlink(0);               /* Initiate a cursor blink */
#ifdef _URLSUPP
        if(isTimerExpired(SOCKET_TIMER_ID)) /* socket connection time-out */
            ffCloseSockets(1) ;
#endif
        if(intable && TTahead())
            break ;
        waitForEvent() ;
    } while(!isTimerExpired(SLEEP_TIMER_ID)) ;
    timerKill(SLEEP_TIMER_ID) ;             /* Kill off the timer */
}

#if TYPEAH
int
TTahead(void)
{
#ifdef _XTERM
    if(!(meSystemCfg & meSYSTEM_CONSOLE))
    {
        int ii ;
        while((ii=XPending(mecm.xdisplay)))
        {
            do
                meXEventHandler() ;
            while(--ii > 0) ;
        }
        /* don't process the timers if we have a key waiting!
         * This is because the timers can generate a lot of timer
         * keys, filling up the input buffer - these are not wanted.
         * By not processing, we leave them there until we need them.
         */
        if(TTnoKeys)
            return TTnoKeys ;

#if MOUSE
        /* If an alarm hCheck the mouse */
        if(isTimerExpired(MOUSE_TIMER_ID))
        {
            uint16 mc ;
            uint32 arg ;

            timerClearExpired(MOUSE_TIMER_ID) ;
            mc = ME_SPECIAL | mouseKeyState |
                      (SKEY_mouse_time+mouseKeys[mouseButtonGetPick()]) ;
            /* mouse-time bound ?? */
            if((!TTallKeys && (decode_key(mc,&arg) != -1)) || (TTallKeys & 0x2))
            {
                /* Timer has expired and timer still bound. Report the key. */
                /* Push the generated keycode into the buffer */
                addKeyToBuffer(mc) ;
                /* Set the new timer and state */
                /* Start a new timer to clock in at 'repeat' intervals */
                /* printf("Setting mouse timer for repeat %d\n",repeattime) ;*/
                timerSet(MOUSE_TIMER_ID,-1,repeattime);
            }
        }
#endif /* MOUSE */
    }
    else
#endif /* _XTERM */
    {
        uint8 cc ;
#ifdef FIONREAD
        int x;      /* holds # of pending chars */
#endif /* FIONREAD */
        /* Pasting in termcap results in lots of keys all at once
         * rather than lose them, keep them there until there enough
         * room in the input key buffer to store them
         */
        while((TTnoKeys != KEYBUFSIZ) &&
#ifdef FIONREAD
              ((ioctl(meStdin,FIONREAD,&x) < 0) ? 0 : x)
#else
              (ioctl(meStdin, FIORDCHK,0))
#endif /* FIONREAD */
              )
            if(read(meStdin,&cc,1) > 0)
                addKeyToBuffer(cc) ;
        if(alarmState & meALARM_WINSIZE)
        {
            changeScreenWidth(TRUE,TTnewWid); /* Change width */
            changeScreenDepth(TRUE,TTnewHig); /* Change depth */
        }
        if(TTnoKeys)
            return TTnoKeys ;
    }
    if(isTimerExpired(IDLE_TIMER_ID))
    {
        register int index;
        uint32 arg;           /* Decode key argument */
        if((index=decode_key(ME_SPECIAL|SKEY_idle_time,&arg)) != -1)
        {
            /* Execute the idle-time key */
            execFuncHidden(ME_SPECIAL|SKEY_idle_time,index,arg) ;

            /* Now set the timer for the next */
            timerSet(IDLE_TIMER_ID,-1,idletime);
        }
        else if(decode_key(ME_SPECIAL|SKEY_idle_drop,&arg) != -1)
            meTimerState[IDLE_TIMER_ID] = IDLE_STATE_DROP ;
        else
            meTimerState[IDLE_TIMER_ID] = 0 ;
    }
    return TTnoKeys ;
}
#endif /* TYPEAH */

#ifdef _XTERM

/*
 * TThideCur - hide the cursor
 */
void
XTERMhideCur(void)
{
    if((TTcurr <= TTnrow) && (TTcurc < TTncol))
    {
        FRAMELINE *flp;                     /* Frame store line pointer */
        uint8     *cc ;                     /* Current cchar  */
        meSCHEME   schm;                    /* Current colour */

        flp  = frameStore + TTcurr ;
        cc   = flp->text+TTcurc ;           /* Get char under cursor */
        schm = flp->scheme[TTcurc] ;        /* Get scheme under cursor */

        XTERMschemeSet(schm) ;
        if ((meSystemCfg & meSYSTEM_FONTFIX) && !((*cc) & 0xe0))
        {
            static char ss[1]={' '} ;
            XTERMstringDraw(colToClient(TTcurc),rowToClient(TTcurr),ss,1);
            XTERMSpecialChar(colToClient(TTcurc),rowToClientTop(TTcurr),*cc) ;
        }
        else
            XTERMstringDraw(colToClient(TTcurc),rowToClient(TTcurr),cc,1);
    }
}

/*
 * TTshowCur - show the cursor
 */
void
XTERMshowCur(void)
{
    if((TTcurr <= TTnrow) && (TTcurc < TTncol))
    {
        FRAMELINE *flp;                     /* Frame store line pointer */
        uint8     *cc ;                     /* Current cchar  */
        meSCHEME   schm;                    /* Current colour */

        flp  = frameStore + TTcurr ;
        cc   = flp->text+TTcurc ;           /* Get char under cursor */
        schm = flp->scheme[TTcurc] ;        /* Get scheme under cursor */


        if(TTfocus)
        {
            uint32 valueMask=0 ;
            uint8 ff ;
            ff = meStyleGetBColor(meSchemeGetStyle(schm)) ;
            if(mecm.fcol != ff)
            {
                mecm.fcol = ff ;
                mecm.xgcv.foreground = colTable[ff] ;
                valueMask |= GCForeground ;
            }
            if(mecm.bcol != cursorColor)
            {
                mecm.bcol = cursorColor ;
                mecm.xgcv.background = colTable[cursorColor] ;
                valueMask |= GCBackground ;
            }
            if(mecm.fontName != NULL)
            {
                ff = meStyleGetFont(meSchemeGetStyle(schm)) ;
                if(meSchemeTestNoFont(schm))
                    ff &= ~(meFONT_BOLD|meFONT_ITALIC|meFONT_UNDERLINE) ;
                if(mecm.font != ff)
                {
                    mecm.font = ff ;
                    mecm.fontId = XTERMfontGetId(ff) ;
                    if(mecm.xgcv.font != mecm.fontId)
                    {
                        mecm.xgcv.font = mecm.fontId ;
                        valueMask |= GCFont ;
                    }
                }
            }
            if(valueMask)
                XChangeGC(mecm.xdisplay,mecm.xgc,valueMask,&mecm.xgcv) ;
            if ((meSystemCfg & meSYSTEM_FONTFIX) && !((*cc) & 0xe0))
            {
                static char ss[1]={' '} ;
                XTERMstringDraw(colToClient(TTcurc),rowToClient(TTcurr),ss,1);
                XTERMSpecialChar(colToClient(TTcurc),rowToClientTop(TTcurr),*cc) ;
            }
            else
                XTERMstringDraw(colToClient(TTcurc),rowToClient(TTcurr),cc,1);
        }
        else
        {
            if(mecm.fcol != cursorColor)
            {
                mecm.fcol = cursorColor ;
                mecm.xgcv.foreground = colTable[cursorColor] ;
                XChangeGC(mecm.xdisplay,mecm.xgc,GCForeground,&mecm.xgcv) ;
            }
            XDrawRectangle(mecm.xdisplay,mecm.xwindow,mecm.xgc,colToClient(TTcurc),
                           rowToClientTop(TTcurr),mecm.fwidth-1,mecm.fdepth-1) ;
        }
    }
}


/*
 * TTdepth
 * Resize the depth of the window. If the resize has been disabled
 * (within the configureNotify event) then do not modify the
 * window size - this operation is performed elsewhere.
 */
void
TTdepth(int y)
{
    if(!(meSystemCfg & meSYSTEM_CONSOLE) && (disableResize == 0))
    {
        sizeHints.height = mecm.fdepth*y ;
        XResizeWindow(mecm.xdisplay,mecm.xwindow,sizeHints.width,sizeHints.height) ;
    }
}

/*
 * TTwidth
 * Resize the width of the window. If the resize has been disabled
 * (within the configureNotify event) then do not modify the
 * window size - this operation is performed elsewhere.
 */
void
TTwidth(int x)
{
    if(!(meSystemCfg & meSYSTEM_CONSOLE) && (disableResize == 0))
    {
        sizeHints.width  = mecm.fwidth*x ;
        XResizeWindow(mecm.xdisplay,mecm.xwindow,sizeHints.width,sizeHints.height) ;
    }
}

int
changeFont(int f, int n)
{
    uint8        buff[MAXBUF] ;            /* Input buffer */
	
    if(meSystemCfg & meSYSTEM_CONSOLE)
        /* change-font not supported on termcap */
        return notAvailable(f,n) ;
    
    /* Get the name of the font. If it is specified as default then
     * do not collect the remaining arguments */
    if(mlreply((uint8 *)"Font Name", 0, 0, buff, MAXBUF) == ABORT)
        return FALSE ;

    /* Change the font */
    if (XTERMsetFont ((char *)buff) == FALSE)
        return FALSE ;

    /* Set up the arguments for a resize operation. Because the
     * font has changed then we need to define the new window
     * base size, minimum size and increment size.
     */
    sizeHints.height = mecm.fdepth*(TTnrow+1) ;
    sizeHints.width  = mecm.fwidth*TTncol ;
    XSetWMNormalHints(mecm.xdisplay,mecm.xwindow,&sizeHints);
    XResizeWindow(mecm.xdisplay,mecm.xwindow,sizeHints.width,sizeHints.height) ;

    /* Make the current font invalid and force a complete redraw */
    mecm.xgcv.font = mecm.fontId ;
    XChangeGC(mecm.xdisplay,mecm.xgc,GCFont,&mecm.xgcv) ;
    sgarbf = TRUE;

    return TRUE ;
}

int
XTERMaddColor(meCOLOR index, uint8 r, uint8 g, uint8 b)
{
    XColor col ;

    if(noColors <= index)
    {
        colTable = (uint32 *) realloc(colTable,
                                             (index+1)*sizeof(uint32)) ;
        memset(colTable+noColors,0,
               (index-noColors+1)*sizeof(uint32)) ;
        noColors = index+1 ;
    }
    col.red   = ((uint16) r) << 8 ;
    col.green = ((uint16) g) << 8 ;
    col.blue  = ((uint16) b) << 8 ;
    if(!XAllocColor(mecm.xdisplay,xcmap,&col))
    {
        static int noCells=-1 ;
        static uint8 *cells=NULL ;
        uint8 *ss ;
        int ii, diff;
        int bDiff= 256*256*3 ;          /* Smallest least squares. */

        if(noCells < 0)
        {
            printf("Warning: Failed to allocate colors, looking up closest\n") ;
            if(((noCells = DisplayCells(mecm.xdisplay,xscreen)) > 0) &&
               ((cells = malloc(noCells*3*sizeof(uint8))) != NULL))
            {
                ii = noCells ;
                ss = cells ;
                for(ii=0 ; ii<noCells ; ii++)
                {
                    col.pixel = ii ;
                    XQueryColor(mecm.xdisplay,xcmap,&col) ;
                    *ss++ = (col.red   >> 8) ;
                    *ss++ = (col.green >> 8) ;
                    *ss++ = (col.blue  >> 8) ;
                }
            }
        }
        col.pixel = BlackPixel(mecm.xdisplay,xscreen) ;
        /* To find the nearest color then use a least squares method. This
         * produces a better approximation than a straight forward color
         * differencing algorithm as it takes into account the variance. */
        if(cells != NULL)
        {
            ii = noCells ;
            ss = cells ;
            for(ii=0 ; ii<noCells ; ii++)
            {
                int delta;
                
                delta = ((int) *ss++) - ((int) r) ;
                diff  = (delta * delta) ;
                delta = ((int) *ss++) - ((int) g) ;
                diff += (delta * delta) ;
                delta = ((int) *ss++) - ((int) b) ;
                diff += (delta * delta) ;
                if(diff < bDiff)
                {
                    bDiff = diff ;
                    col.pixel = ii ;
                }
            }
        }
        XQueryColor(mecm.xdisplay,xcmap,&col) ;
        if(!XAllocColor(mecm.xdisplay,xcmap,&col))
            printf("Warning: Failed to allocate closest color\n") ;

    }
    colTable[index] = col.pixel ;
    if(mecm.fcol == index)
        mecm.fcol = meCOLOR_INVALID ;
    if(mecm.bcol == index)
        mecm.bcol = meCOLOR_INVALID ;
    return TRUE ;
}

void TTsetBgcol(void)
{
    /* change the background */
    if(!(meSystemCfg & meSYSTEM_CONSOLE))
        XSetWindowBackground(mecm.xdisplay,mecm.xwindow,colTable[meStyleGetBColor(meSchemeGetStyle(globScheme))]);
}

#ifdef _CLIPBRD
void
TTsetClipboard(void)
{
    clipState &= ~CLIP_TRY_SET ;
    if(!(meSystemCfg & meSYSTEM_CONSOLE) && !(clipState & CLIP_OWNER))
    {
        XSetSelectionOwner(mecm.xdisplay,XA_PRIMARY,mecm.xwindow,CurrentTime) ;
        if(XGetSelectionOwner(mecm.xdisplay,XA_PRIMARY) == mecm.xwindow)
            clipState |= CLIP_OWNER ;
    }
}

void
TTgetClipboard(void)
{
    clipState &= ~CLIP_TRY_GET ;

    if(!(meSystemCfg & meSYSTEM_CONSOLE) && !(clipState & CLIP_OWNER))
    {
        /* Remove the 'Try to set selection' flag. This is really important if increment
         * copy texts are being used. If they are and this flag is set after receiving
         * the initial size, the ksave then take ownership of the block and so we never
         * get the copy text. Take ownership at the end */
        clipState &= ~(CLIP_TRY_SET|CLIP_RECVED) ;
        /* Request for the current Primary string owner to send a
         * SelectionNotify event to us giving the current string
         */
        XConvertSelection(mecm.xdisplay,XA_PRIMARY,XA_STRING,meAtoms[meATOM_COPY_TEXT],
                          mecm.xwindow,CurrentTime) ;
        /* Must do a flush to ensure the request has gone */
        XFlush(mecm.xdisplay) ;
        /* Wait for the returned value, alarmState bit will be set */
        while(!TTahead() && !(clipState & CLIP_RECVED))
            waitForEvent() ;
        clipState &= ~CLIP_RECVED ;
        /* reset the increment clip size to zero (just incase there was an interuption) */
        meClipSize=0 ;
        TTsetClipboard() ;
    }
}
#endif

/*
 * TTtitle. Put the name of the buffer into the window frame
 */

void
TTtitleText (uint8 *str)
{
    if(!(meSystemCfg & meSYSTEM_CONSOLE))
    {
        char buf[MAXBUF], *ss ;

        meStrcpy(buf,meName);
        if (str != NULL)
        {
            meStrcat(buf,": ") ;
            meStrcat(buf,str) ;
        }
        ss = buf ;
        XStoreName(mecm.xdisplay,mecm.xwindow,ss);
    }
}

#endif	/* _XTERM */

#ifdef _CLIENTSERVER

#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <netinet/in.h>

int meCSSock=-1 ;
int meCCSSock=-1 ;
void
TTopenClientServer (void)
{
    /* If the server has not been created then create it now */
    if(meCSSock == -1)
    {
        struct sockaddr_un cssa ;
        meIPIPE *ipipe ;
        BUFFER *bp ;
        meMODE sglobMode ;
        int ii ;

        if((meCSSock=socket(AF_UNIX,SOCK_STREAM, 0)) < 0)
        {
            meSystemCfg &= ~meSYSTEM_CLNTSRVR ;
            return ;
        }
        ii = sprintf(cssa.sun_path,"/tmp/mesrv%d",(int) meUid);
        ii += sizeof(cssa.sun_family);
        cssa.sun_family = AF_UNIX;

        if(meTestExist((char *) cssa.sun_family) &&
           (connect(meCSSock,(struct sockaddr *)&cssa,ii) >= 0))
        {
            /* theres another me acting as a server, quit! */
            meSystemCfg &= ~meSYSTEM_CLNTSRVR ;
            close(meCSSock) ;
            meCSSock = -1 ;
            return ;
        }
        meUnlink(cssa.sun_path) ;
        if((bind(meCSSock,(struct sockaddr *)&cssa,ii) < 0) ||
           (listen(meCSSock,20) < 0))
        {
            meSystemCfg &= ~meSYSTEM_CLNTSRVR ;
            close(meCSSock) ;
            meCSSock = -1 ;
            return ;
        }
        /* Change socket permissions so only the user can send commands */
        meChmod((char *) cssa.sun_path,0700);
        /* Create the ipipe buffer */
        meModeCopy(sglobMode,globMode) ;
        meModeSet(globMode,MDPIPE) ;
        meModeSet(globMode,MDLOCK) ;
        meModeSet(globMode,MDHIDE) ;
        meModeClear(globMode,MDWRAP) ;
        meModeClear(globMode,MDUNDO) ;
        if(((bp=bfind(BserverN,BFND_CREAT)) == NULL) ||
           ((ipipe = meMalloc(sizeof(meIPIPE))) == NULL))
        {
            meSystemCfg &= ~meSYSTEM_CLNTSRVR ;
            close(meCSSock) ;
            meCSSock = -1 ;
            return ;
        }
        meModeCopy(globMode,sglobMode) ;
        bp->intFlag |= BIFNODEL ;
        ipipe->next = ipipes ;
        ipipe->pid = 0 ;
        ipipe->rfd = meCSSock ;
        ipipe->outWfd = meCSSock ;
        ipipes = ipipe ;
        noIpipes++ ;
        ipipe->bp = bp ;
        /* setup the buffer */
        {
            uint8 buff[MAXBUF] ;
            sprintf((char *)buff,"Client Server: /tmp/mesrv%d",(int) meUid) ;
            addLineToEob(bp,buff) ;              /* Add string */
            addLineToEob(bp,(uint8 *)"") ;       /* Add string */
            addLineToEob(bp,(uint8 *)"") ;       /* Add string */
            bp->b_dotp = lback(bp->b_linep) ;
            bp->b_doto = 0 ;
            bp->line_no = bp->elineno-1 ;
            alphaMarkSet(bp,'I',bp->b_dotp,bp->b_doto,1) ;
        }
        /* Set up the window dimensions - default to having auto wrap */
        ipipe->flag = 0 ;
        ipipe->strRow = 0 ;
        ipipe->strCol = 0 ;
        ipipe->noRows = 0 ;
        ipipe->noCols = 0 ;
        ipipe->curRow = 0 ;
        ipipe->curRow = bp->line_no ;
        /* get a popup window for the command output */
        ipipeSetSize(curwp,bp) ;
    }
}


void
TTkillClientServer(void)
{
    /* Close & delete the client file */
    if(meCSSock != -1)
    {
        meIPIPE *ipipe ;
        char fname[MAXBUF] ;

        /* get the ipipe node */
        ipipe = ipipes ;
        while(ipipe != NULL)
        {
            if(ipipe->pid == 0)
                break ;
            ipipe = ipipe->next ;
        }
        if(ipipe != NULL)
        {
            ipipe->bp->intFlag |= BIFBLOW ;
            zotbuf(ipipe->bp,1) ;
        }
        else
            close(meCSSock) ;
        sprintf(fname,"/tmp/mesrv%d",(int) meUid) ;
        meUnlink(fname) ;
        meSystemCfg &= ~meSYSTEM_CLNTSRVR ;
        meCSSock = -1 ;
    }
    if(meCCSSock != -1)
    {
        close(meCSSock) ;
        meCCSSock = -1 ;
    }
}

int
TTconnectClientServer (void)
{
    /* If the server has not been connected then create it now */
    if(meCCSSock == -1)
    {
        struct sockaddr_un cssa ;
        int ii ;

        if((meCCSSock=socket(AF_UNIX,SOCK_STREAM, 0)) < 0)
            return 0 ;

        ii = sizeof(cssa.sun_family);
        ii += sprintf(cssa.sun_path,"/tmp/mesrv%d",(int) meUid);
        cssa.sun_family = AF_UNIX;

        if(connect(meCCSSock,(struct sockaddr *)&cssa,ii) < 0)
        {
            close(meCCSSock) ;
            meCCSSock = -1 ;
            return 0 ;
        }
    }
    return 1 ;
}
void
TTsendClientServer(uint8 *line)
{
    if(meCCSSock >= 0)
    {
        int ii, ll=meStrlen(line) ;
        while(ll)
        {
            if((ii=write(meCCSSock,line,ll)) < 0)
                return ;
            ll -= ii ;
            line += ii ;
        }
    }
}
#endif /* _CLIENTSERVER */

/**************************************************************************
* MISCELEANEOUS FUNCTIONS                                                 *
**************************************************************************/
#ifdef _NOSTRDUP
/* strdup; String duplication is not supported on some earlier platforms
 * provide one if it does not exits. Typically absent on earlier BSD4.2
 * platforms. */
char *
strdup (const char *s)
{
    char *p;

    if ((p = (char *) meMalloc ((meStrlen (s)) + 1)) != NULL)
        meStrcpy (p, s);
    return (p);
}

#endif

#ifdef _NOPUTENV
int
putenv (const char *string)
{
    char **p;                           /* Temp environment ptr */
    int len;                            /* Length of the string */
    int ii;                             /* Loop counter */
    
    /* Test the string for a valid value */
    if ((string == NULL) || (*string == '\0'))
        return -1;
    
    /* If the local environment space has not been set up then do so now */
    if (meEnviron == NULL)
    {
        int jj;
        
        /* Count the number of entries */
        for (ii = 0, p = environ; *p != NULL; p++)
            ii++;
        
        /* Allocate space */
        if ((meEnviron = meMalloc (sizeof (char *) * (ii + 1))) == NULL)
            return -1;
        
        /* Initialise to NULL */
        jj = ii;
        do
            meEnviron[jj] = NULL;
        while (--jj >= 0);
        
        /* Copy across the environment */
        for (jj = 0; jj < ii; jj++)
        {
            if ((meEnviron[jj] = meStrdup (environ[jj])) == NULL)
            {
                while (--jj > 0)
                    meFree (meEnviron[jj]);
                meFree (meEnviron);
                return -1;
            }
        }
    }
    
    /* Now try and find an existing entry with the same name */
    for (len = 0; string[len] != '\0'; len++)
        if (string [len] == '=')
            break;
    
    /* Test for valid putenv value */
    if (string[len] == '\0')
        return -1;                      /* Invalid string */
    
    /* Try and locate the environment value in the existing environment */
    for (ii = 0, p = meEnviron; (*p != NULL); ii++, p++)
    {
        if ((strnicmp (*p, string, len) == 0) && ((*p)[len] == '='))
        {
            /* Same entry. push the new value */
            meFree (*p);
            *p = string;
            return (0);
        }
    }
    
    /* The variable value does not exist in the environment, make a new entry.
     * First re-size the environment. */
    if ((p = realloc (meEnviron, (ii+2) * sizeof (char *))) == NULL)
        return -1;
    
    /* Inherit the new environment and add the new variable. */
    meEnviron = p;
    meEnviron [ii] = string;
    meEnviron [ii+1] = NULL;
    return 0;
}

char *
megetenv (const char *string)
{
    if (meEnviron != NULL)
    {
        char **p;
        int len;
        
        /* Test the string for a valid value */
        if ((string == NULL) || (*string == '\0'))
            return NULL;
        
        /* Get the length of the string for the search */
        len = strlen (string);
        if ((p = meEnviron) != NULL)
        {
            while (*p != NULL)
            {
                if ((strnicmp (*p, string, len) == 0) && ((*p)[len] == '='))
                    return (&((*p)[len+1]));
                p++;
            }
        }
        /* Not found */
        return NULL;
    }
    return (getenv(string));
}
#endif
