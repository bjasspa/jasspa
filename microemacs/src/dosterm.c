/* -*- C -*- ****************************************************************
 *
 *  System        : MicroEmacs Jasspa Distribution
 *  Module        : dosterm.c
 *  Synopsis      : Dos terminal support routines
 *  Created By    : Steven Phillips
 *  Created       : 1994
 *  Last Modified : <000221.0749>
 *
 *  Description
 *
 *  Notes
 * 
 ****************************************************************************
 * 
 * Copyright (c) 1994-2000 Steven Phillips    
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
#include "eskeys.h"

#include <dos.h>
#include <sys/time.h>
#include <pc.h>

#if MOUSE

/* Local definitions for mouse handling code */
/* mouseState
 * A integer interpreted as a bit mask that holds the current state of
 * the mouse interaction. */
#define MOUSE_STATE_LEFT         0x0001 /* Left mouse button is pressed */
#define MOUSE_STATE_MIDDLE       0x0002 /* Middle mouse button is pressed */
#define MOUSE_STATE_RIGHT        0x0004 /* Right mouse button is pressed */
#define MOUSE_STATE_VISIBLE      0x0200 /* Mouse is currently visible */
#define MOUSE_STATE_SHOW         0x0400 /* Mouse active, show next time */
#define MOUSE_STATE_BUTTONS      (MOUSE_STATE_LEFT|MOUSE_STATE_MIDDLE|MOUSE_STATE_RIGHT)

static int  mouseState = 0;             /* State of the mouse. */
/* bit button lookup - [0] = no keys, [1] = left, [2] = right, [4]=middle */
static uint16 mouseKeys[8] = { 0, 1, 3, 0, 2, 0, 0, 0 } ;

#endif /* MOUSE */

uint8  Cattr = 0x07 ;
uint16 TTmargin	= 8  ;			/* size of minimim margin and	*/
uint16 TTscrsiz	= 64 ;			/* scroll size for extended lines */
uint16 TTmrow, TTmcol,TTnrow, TTncol ;

static int16 dosOrgMode ;
static int16 dosOrgDepth ;
static int16 dosResMode=-1 ;
static int16 dosResSpecial ;

#define ME_EXT_FLAG 0x80
uint8 TTextkey_lut [256] =
{
 0, SKEY_esc,  0,0xc0,  0,  0,  0,  0,  0,  0,
 0,  0,  0,  0,SKEY_backspace,SKEY_tab,0xf1,0xf7,0xe5,0xf2,
 0xf4,0xf9,0xf5,0xe9,0xef,0xf0,0xdb,0xdd,SKEY_return,  0,
 0xe1,0xf3,0xe4,0xe6,0xe7,0xe8,0xea,0xeb,0xec,0xbb,
 0xa7,0xe0,   0,0xa3,0xfa,0xf8,0xe3,0xf6,0xe2,0xee,
 0xed,0xac,0xae,0xaf,  0,ME_EXT_FLAG|'*',  0,  0,  0,SKEY_f1,
 SKEY_f2,SKEY_f3,SKEY_f4,SKEY_f5,SKEY_f6,SKEY_f7,SKEY_f8,SKEY_f9,SKEY_f10,  0,
 0,SKEY_home,SKEY_up,SKEY_page_up,ME_EXT_FLAG|'-',SKEY_left,SKEY_kp_begin,SKEY_right,ME_EXT_FLAG|'+',SKEY_end,
 SKEY_down,SKEY_page_down,SKEY_insert,SKEY_delete,SKEY_f1,SKEY_f2,SKEY_f3,SKEY_f4,SKEY_f5,SKEY_f6,
 SKEY_f7,SKEY_f8,SKEY_f9,SKEY_f10,SKEY_f1,SKEY_f2,SKEY_f3,SKEY_f4,SKEY_f5,SKEY_f6,
 SKEY_f7,SKEY_f8,SKEY_f9,SKEY_f10,SKEY_f1,SKEY_f2,SKEY_f3,SKEY_f4,SKEY_f5,SKEY_f6,
 SKEY_f7,SKEY_f8,SKEY_f9,SKEY_f10,0xf2,SKEY_left,SKEY_right,SKEY_end,SKEY_page_down,SKEY_home,
 ME_EXT_FLAG|'1',ME_EXT_FLAG|'2',ME_EXT_FLAG|'3',ME_EXT_FLAG|'4',ME_EXT_FLAG|'5',ME_EXT_FLAG|'6',ME_EXT_FLAG|'7',ME_EXT_FLAG|'8',ME_EXT_FLAG|'9',ME_EXT_FLAG|'0',
 0xad,0xbd,SKEY_page_up,SKEY_f11,SKEY_f12,SKEY_f11,SKEY_f12,SKEY_f11,SKEY_f12,SKEY_f11,
 SKEY_f12,SKEY_up,ME_EXT_FLAG|'-',SKEY_kp_begin,ME_EXT_FLAG|'+',SKEY_down,SKEY_insert,SKEY_delete,SKEY_tab,ME_EXT_FLAG|'/',
 ME_EXT_FLAG|'*',SKEY_home,SKEY_up,SKEY_page_up,0,SKEY_left,0,SKEY_right,0,SKEY_end,
 SKEY_down,SKEY_page_down,SKEY_insert,SKEY_delete,ME_EXT_FLAG|'/',SKEY_tab,SKEY_return
};	/* Extended Keyboard lookup table */


uint16 TTmodif=0 ;

/* Color code */
uint8 *colTable=NULL ;

#define dosNumColors 16
static uint8 dosColors[dosNumColors*3] =
{
    0,0,0,    0,0,200, 0,200,0, 0,200,200, 200,0,0, 200,0,200, 200,200,0, 200,200,200, 
    75,75,75, 0,0,255, 0,255,0, 0,255,255, 255,0,0, 255,0,255, 255,255,0, 255,255,255
} ;
    

void
TTdump(BUFFER *bp)
{
    uint16 r1, r2, c1, c2 ;
    uint8 buff[TTmrow*TTmcol*2], *ss ;
    uint8 line[TTmcol] ;

    ScreenRetrieve(buff) ;
    r1 = ScreenRows() ;
    c1 = ScreenCols() ;
    ss = buff ;
    for(r2=0 ; r2<r1 ; r2++)
    {
        for(c2=0 ; c2<c1 ; c2++)
        {
            line[c2] = *ss++ ;
            ss++ ;
        }
        line[c2] = '\0' ;
        addLineToEob(bp,line) ;
    }
}

/*
** test file exists and return attributes
*/
int
meGetFileAttributes(uint8 *fn)
{
    union REGS reg ;		/* cpu register for use of DOS calls */

    reg.x.ax = 0x4300 ;
    reg.x.dx = ((unsigned long) fn) ;
    intdos(&reg, &reg);

    if(reg.x.cflag)
        return -1 ;
    return reg.x.ax ;
}

void
_meChmod(uint8 *fn, uint16 attr)
{
    union REGS reg ;		/* cpu register for use of DOS calls */

    reg.x.ax = 0x4301 ;
    reg.x.cx = attr ;
    reg.x.dx = ((unsigned long) fn) ;
    intdos(&reg, &reg);
}

#if 0
#define _DEBUG_KEYS
static FILE *dklog=NULL ;
#endif

void
TTgetkeyc(void)
{
    union REGS rg ;
    uint16 ii ;
    uint8  cc ;

    /* call the dos to get a char */
    rg.h.ah = 7;		/* dos Direct Console Input call */
    intdos(&rg, &rg);
    cc = rg.h.al;		/* grab the char */
    rg.h.ah = 0x02;             /* get shift status */
    int86(0x16,&rg,&rg) ;
    TTmodif = ((rg.h.al & 0x01) << 8) | ((rg.h.al & 0x0E) << 7) ;

#ifdef _DEBUG_KEYS
    if(dklog==NULL)
        dklog = fopen("log","w+") ;
#endif
    
    if(cc == 0)
    {
        rg.h.ah = 0x07 ;		/* dos Direct Console Input call */
        intdos(&rg, &rg) ;
        cc = TTextkey_lut[rg.h.al] ;

#ifdef _DEBUG_KEYS
        fprintf(dklog,"special key, mode 0x%04x - cc %d al = %d\n",TTmodif,rg.h.al,cc) ;
#endif

        if(cc & ME_EXT_FLAG)
            ii = (cc & 0x7f) | TTmodif ;
        else
return_spec:
	    ii = ME_SPECIAL | cc | TTmodif ;

    }
    else
    {
#ifdef _DEBUG_KEYS
        fprintf(dklog,"normal  key, mode 0x%04x - cc %d\n",TTmodif,cc) ;
#endif
        if(cc == 0x7f)
        {
            if(TTmodif)
                cc = SKEY_backspace ;
            else
                cc = SKEY_delete ;
            goto return_spec ;
        }
        if((TTmodif & ME_CONTROL) == 0)
        {
            if(cc == 0x08)
            {
                cc = SKEY_backspace ;
                goto return_spec ;
            }
            if(cc == 0x09)
            {
                cc = SKEY_tab ;
                goto return_spec ;
            }
            if(cc == 0x0d)
            {
                cc = SKEY_return ;
                goto return_spec ;
            }
            if(cc == 0x1b)
            {
                cc = SKEY_esc ;
                goto return_spec ;
            }
        }
        if(cc == 0x20)
            ii = cc | (TTmodif & (ME_CONTROL|ME_ALT)) ;
        else
            ii = cc ;
    }
    if(ii == 0x07)
    {
        if(macbug < 0)
        {
            macbug = 1 ;
            return ;
        }
        TTbreakFlag = 1 ;
    }
    addKeyToBuffer(ii) ;
#if MOUSE
    /* If the mouse is active and flagged to show, as the user has pressed a
     * key, we should now flag to not automatically show the mouse.
     */
    mouseState &= ~MOUSE_STATE_SHOW ;
#endif
    
}

/*
 * TTgetc
 * Get a character from the keyboard. 
 * 
 * Within the get loop we service the following:-
 * 
 * Keyboard Input - via TTahead() which fills the character buffer. 
 * Mouse Input    - Directly read the mouse state and look for changes.
 *                  The mouse information is serviced by doMouseEvent 
 *                  which determines the mouse button state, performing
 *                  timed mouse events.
 * Idle Input     - Report the presence of idle time. This occurs when 
 *                  all other requests have been serviced and there are
 *                  no outstanding inputs which are not reported. The
 *                  doIdleEvent() detemines the idle state and may 
 *                  generate a special idle key.
 * Timer Handling - The timers are checked for expiration in TTahead(),
 *                  triggering events via doMouseEvent(), doIdleEvent
 *                  which in turn may add keys to the keyboard buffer.
 *                  A side effect is that an Autosave operation may be
 *                  scheduled via the 'alarmState' variable, the control
 *                  loop instigates an Autosave() operation.
 *                 
 * The mouse deserves a special mention here. The mouse is only EVER visible
 * in TTgetc(). The function is always exited with the mouse hidden. It is 
 * important that the mouse enabled/disable visibility are kept in strict 
 * pairs otherwise there is a danger that the mouse cursor is perminantly
 * disabled. The 'MouseOkay' state holds the visibility of the mouse with the
 * bit MOUSE_VISIBLE. Outside of TTgetc() the MOUSE_VISIBLE bit may be set
 * but the mouse cursor is infact off because we are NOT in TTgetc.
 * 
 * If TTgetc() is entered with MOUSE_VISIBLE, then the mouse cursor is 
 * always turned on on entry to the control loop (provided that there are
 * no keys' on the input queue). If it is entered with the mouse invisible
 * then the visibility of the mouse is only enabled when activity on the 
 * mouse has been detected.
 * 
 * We exit TTgetc with MOUSE_VISIBLE set ONLY if we are dealing with a 
 * mouse key, or the mouse is visible and an idle/redraw event is detected.
 * Conversly, if a user strikes a key then we exit with MOUSE_INVISIBLE i.e.
 * the mouse is not showing and will only be reshown if the user subsequently
 * fiddles with the mouse.
 * 
 * The priority of idle events is:-
 * 
 *   Mouse idle events
 *   Redraw idle events
 *   Idle idle events (if this make any sense !!)
 * 
 * Observe the above, especially the mouse, when tinkering in here. This is
 * the 20th attempt to get this correct in all scenarios. 
 * 
 * Jon Green 29/06/97
 */

void
TTwaitForChar(void)
{
    union REGS rg ;
    uint16 cc;                  /* Local character code */

#if MOUSE
    if(meMouseCfg & meMOUSE_ENBLE)
    {
        uint16 mc ;
        uint32 arg ;
        /* If mouse state flags that the mouse should be visible then we
         * must make it visible. We had to make it invisible to get the
         * screen-updates correct
         */
        if(mouseState & MOUSE_STATE_SHOW)
        {
            mouseState |= MOUSE_STATE_VISIBLE ;
            rg.x.ax = 0x0001 ;
            int86(0x33, &rg, &rg) ;
        }
        
        /* If no keys left then if theres currently no mouse timer and
         * theres a button press (No mouse-time key) then check for a
         * time-mouse-? key, if found add a timer start a mouse checking
         */
        if(!isTimerActive(MOUSE_TIMER_ID) &&
           ((mc=(mouseState & MOUSE_STATE_BUTTONS)) != 0))
        {
            mc = ME_SPECIAL | TTmodif | (SKEY_mouse_time+mouseKeys[mc]) ;
            if((!TTallKeys && (decode_key(mc,&arg) != -1)) || (TTallKeys & 0x2))
            {
                /* Start a timer and move to timed state 1 */
                /* Start a new timer to clock in at 'delay' intervals */
                /* printf("Setting mouse timer for delay  %1x %1x %d\n",TTallKeys,meTimerState[MOUSE_TIMER_ID],delaytime) ;*/
                timerSet(MOUSE_TIMER_ID,-1,delaytime);
            }
        }
    }
#endif
    /* IDLE TIME: Check the idle time events */        
    if(kbdmode == KBD_IDLE)
        /* Check the idle event */
        doIdlePickEvent() ;
    
    for(;;)
    {
        if(isTimerExpired(AUTOS_TIMER_ID))  /* Alarm expired ?? */
            autoSaveHandler();              /* Initiate an auto save */
        if(isTimerExpired(CALLB_TIMER_ID))  /* Alarm expired ?? */
            callBackHandler();              /* Initiate any callbacks */
        if(isTimerExpired(CURSOR_TIMER_ID)) /* Alarm expired ?? */
            TThandleBlink(0);               /* Initiate a cursor blink */
        if(TTahead())
            break ;
#if MOUSE
        if(meMouseCfg & meMOUSE_ENBLE)
        {
            int16 row, col, but ;
            
            /* Get new mouse status. It appears that the fractional bits of
             * the mouse change across reads. ONLY Compare the non-fractional
             * components.
             */
            rg.x.ax = 0x0003 ;
            int86(0x33, &rg, &rg) ;
            but = rg.x.bx & MOUSE_STATE_BUTTONS ;
            col = rg.x.cx>>3 ;
            row = rg.x.dx>>3 ;
            
            /* Check for mouse state changes */
            if(((mouseState ^ but) & MOUSE_STATE_BUTTONS) || 
               (mouse_X != col) || (mouse_Y != row))
            {
                int ii ;
                
                /* Get shift status */
                rg.h.ah = 0x02;
                int86(0x16,&rg,&rg) ;
                TTmodif = ((rg.h.al & 0x01) << 8) | ((rg.h.al & 0x0E) << 7) ;
                
                /* Record the new mouse positions */
                mouse_X = col ;
                mouse_Y = row ;
                
                /* Check for button changes, these always create keys */
                if((mouseState ^ but) & MOUSE_STATE_BUTTONS)
                {
                    /* Iterate down the bit pattern looking for changes
                     * of state */
                    for (ii=1 ; ii<8 ; ii<<=1)
                    {
                        /* Test each key and add to the stack */
                        if((mouseState ^ but) & ii)
                        {
                            /* Up or down press ?? */
                            if(but & ii)
                                cc = ME_SPECIAL+SKEY_mouse_pick_1-1 ;     /* Down  */
                            else
                                cc = ME_SPECIAL+SKEY_mouse_drop_1-1 ;     /* Up !! */
                            cc = TTmodif | (cc + mouseKeys[ii]) ;
                            addKeyToBuffer(cc) ;
                        }
                    }
                    /* Correct the mouse state */
                    mouseState = (mouseState & ~MOUSE_STATE_BUTTONS) | but ;
                }
                else
                {
                    uint32 arg;                 /* Decode key argument */
                    /* Check for a mouse-move key */
                    cc = (ME_SPECIAL+SKEY_mouse_move+mouseKeys[but]) | TTmodif ;
                    /* Are we after all movements or mouse-move bound ?? */
                    if((!TTallKeys && (decode_key(cc,&arg) != -1)) || (TTallKeys & 0x1))
                        addKeyToBuffer(cc) ;        /* Add key to keyboard buffer */
                    else
                    {
                        /* Mouse has been moved so we must make it visible */
                        if(!(mouseState & MOUSE_STATE_VISIBLE))
                        {
                            mouseState |= MOUSE_STATE_VISIBLE ;
                            rg.x.ax = 0x0001 ;
                            int86(0x33, &rg, &rg) ;
                        }
                        continue ;
                    }
                }
                /* As we are adding a mouse key, the mouse is active
                 * so add the 'show' flag, so on entry next time
                 * the mouse will automatically become visible.
                 */
                mouseState |= MOUSE_STATE_SHOW ;
                break ;
            }
        }
#endif
    } /* ' for' */
    
#if MOUSE
    if(mouseState & MOUSE_STATE_VISIBLE)
    {
        /* Turn mouse off - NOTE that we dont change the 'show' state, If
         * the 'show' is flagged then on entry next time the mouse will
         * automatically become visible.
         * 
         * If we pressed a key TTgetkeyc will remove the 'show'.
         */
        mouseState &= ~MOUSE_STATE_VISIBLE ;
        rg.x.ax = 0x0002 ;
        int86(0x33, &rg, &rg) ;
    }
#endif
}

void
TThideCur(void)
{
    if((TTcurr <= TTnrow) && (TTcurc < TTncol))
    {
        FRAMELINE *flp;                     /* Frame store line pointer */
        meSCHEME schm;                      /* Current colour */
        uint8 cc ;                          /* Current cchar  */
        uint8 dcol;

        flp  = frameStore + TTcurr ;
        cc   = flp->text[TTcurc] ;          /* Get char under cursor */
        schm = flp->scheme[TTcurc] ;        /* Get colour under cursor */

        dcol = TTschemeSet(schm) ;
        
        ScreenPutChar(cc, dcol, TTcurc, TTcurr);
    }
}

void
TTshowCur(void)
{
    if((TTcurr <= TTnrow) && (TTcurc < TTncol))
    {
        FRAMELINE *flp;                     /* Frame store line pointer */
        meSCHEME schm;                      /* Current colour */
        uint8 cc ;                          /* Current cchar  */
        uint8 dcol;

        flp  = frameStore + TTcurr ;
        cc   = flp->text[TTcurc] ;          /* Get char under cursor */
        schm = flp->scheme[TTcurc] ;        /* Get colour under cursor */

        dcol = TTcolorSet(colTable[meStyleGetBColor(meSchemeGetStyle(schm))],
                          colTable[cursorColor]) ;

        ScreenPutChar(cc, dcol, TTcurc, TTcurr);
    }
}


#if MOUSE
void
TTinitMouse(void)
{
    if(meMouseCfg & meMOUSE_ENBLE)
    {
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
        mouseKeys[2] = b3 ;
        mouseKeys[4] = b2 ;
    }
}
#endif

void
ibmChangeSRes(void)
{
    union REGS rg;

    if(dosResMode >= 0)
    {
        int row, col ;
        
        /* and put the beast into given mode */
        rg.x.ax = dosResMode ;
        int86(0x10, &rg, &rg);

        if(dosResSpecial & 0x01)
        {
            rg.h.bl = 0;
            rg.x.ax = 0x1112;
            int86(0x10, &rg, &rg);
        }
        row = ScreenRows();
        col = ScreenCols();
#if 0
        /* move the cursor to the bottom left */
        rg.h.ah = 2 ;		/* set cursor position function code */
        rg.h.dl = 0 ;
        rg.h.dh = row-1 ;
        rg.h.bh = 0 ;		/* set screen page number */
        int86(0x10, &rg, &rg);
#endif
        changeScreenDepth(TRUE,row);
        changeScreenWidth(TRUE,col);
    }
    /* Stop that horrible blinking */
    rg.x.ax = 0x1003;		/* blink state dos call */
    rg.x.bx = 0x0000;		/* set the current state */
    int86(0x10, &rg, &rg);
    /* hide the cursor as we do our own.
     * NOTE: changing screen modes switches it back on */
    rg.x.ax = 0x0100;		/* Cursor type dos call */
    rg.x.cx = 0x2607;		/* set invisible */
    int86(0x10, &rg, &rg);

#if MOUSE
    if(meMouseCfg & meMOUSE_ENBLE)
    {
        /* initialise the mouse and flag if okay */
        rg.x.ax = 0x0000 ;
        int86(0x33, &rg, &rg) ;
        /* initialise the mouse screen range */
        /* set mouse hor range */
        rg.x.ax = 0x0007 ;
        rg.x.cx = 0x0000 ;
        rg.x.dx = (TTncol-1) << 3 ;
        int86(0x33, &rg, &rg) ;
        /* set mouse vert range */
        rg.x.ax = 0x0008 ;
        rg.x.cx = 0x0000 ;
        rg.x.dx = (TTnrow) << 3 ;
        int86(0x33, &rg, &rg) ;
        /* get the current status */
        rg.x.ax = 0x0003 ;
        int86(0x33, &rg, &rg) ;
        mouse_X = rg.x.cx>>3 ;
        mouse_Y = rg.x.dx>>3 ;
        mouseState |= (rg.x.bx & MOUSE_STATE_BUTTONS) ;
    }
#endif
    sgarbf = TRUE ;
}

int
TTstart(void)
{
    union REGS rg;
    
#ifdef __DJGPP2__
    /* must call this to disable DJ's C-c signal handling */
    extern void __djgpp_exception_toggle(void);
    __djgpp_exception_toggle() ;
#endif
    dosOrgMode = ScreenMode() ;
    dosOrgDepth = ScreenRows() ;

    TTmrow = TTnrow = dosOrgDepth ;
    TTmcol = TTncol = ScreenCols() ;
    TTnrow-- ;

    /* vga cursor emulation */
    rg.h.bl = 0x34;
    rg.x.ax = 0x1200;
    int86(0x10, &rg, &rg);
    
    /* initialise the mouse and flag if okay */
    rg.x.ax = 0x0000 ;
    int86(0x33, &rg, &rg) ;
    if(rg.x.ax != 0)
    {
        meMouseCfg |= meMOUSE_ENBLE ;
        /* value of 0xffff also means 2 buttons */
        if(rg.x.bx == 3)
        {
            /* 3 buttons, so change middle from right to middle */
            meMouseCfg |= 3 ;
        }
        else
            meMouseCfg |= 2 ;
        TTinitMouse() ;
    }
    else
        meMouseCfg &= ~meMOUSE_ENBLE ;

    return TTopen() ;
}

int
TTopen(void)
{
    union REGS rg;

    /* kill the ctrl-break interupt */
    rg.h.ah = 0x33;		/* control-break check dos call */
    rg.h.al = 1;		/* set the current state */
    rg.h.dl = 0;		/* set it OFF */
    intdos(&rg, &rg);	        /* go for it! */

    ibmChangeSRes() ;

    return TRUE ;
}

int
TTclose(void)
{
    union REGS rg;

    mlerase(MWERASE|MWCURSOR);
    if(dosResMode >= 0)
    {
        /* screen changed, restore the mode */
        rg.h.ah = 0x00 ;
        rg.h.al = dosOrgMode ;
        int86(0x10, &rg, &rg);

        if(dosOrgDepth > ScreenRows())
        {
            /* if there are less rows than when we started,
             * try dicking with it */
            rg.h.bl = 0;
            rg.x.ax = 0x1112;
            int86(0x10, &rg, &rg);
        }
#if 0
        /* move the cursor to the bottom left */
        rg.h.ah = 2 ;		/* set cursor position function code */
        rg.h.dl = 0 ;
        rg.h.dh = ScreenRows()-1 ;
        rg.h.bh = 0 ;		/* set screen page number */
        int86(0x10, &rg, &rg);
#endif
    }
    /* show the dos text cursor again */
    rg.x.ax = 0x0100;		/* Cursor type dos call */
    rg.x.cx = 0x0607;		/* set normal & size    */
    int86(0x10, &rg, &rg);
    /* restore the ctrl-break interupt */
    rg.h.ah = 0x33;		/* control-break check dos call */
    rg.h.al = 1;		/* set the current state */
    rg.h.dl = 1;		/* set it ON */
    return intdos(&rg, &rg);	/* go for it! */
}


int
changeFont(int f, int n)
{
    char buf[NSTRING] ;
    int  mode ;

    if(mlreply("Res mode",0,0,buf,NBUFN) != TRUE)
        return FALSE ;
    mode = meAtoi(buf) ;
    if(mlreply("Res special",0,0,buf,NBUFN) != TRUE)
        return FALSE ;
    dosResMode = mode ;
    dosResSpecial = meAtoi(buf) ;
    ibmChangeSRes() ;
    return TRUE ;
}

int
TTaddColor(meCOLOR index, uint8 r, uint8 g, uint8 b)
{
    uint8 *ss ;
    int ii, jj, idif, jdif ;

    jdif = 256+256+256 ;
    ss = dosColors ;
    for(ii=0 ; ii<dosNumColors ; ii++)
    {
        idif = abs(r - *ss++) + abs(g - *ss++) + abs(b - *ss++) ;
        if(idif < jdif)
        {
            jdif = idif ;
            jj = ii ;
        }
    }
                 
    if(noColors <= index)
    {
        colTable = realloc(colTable, (index+1)*sizeof(uint8)) ;
        memset(colTable+noColors,0,(index-noColors+1)*sizeof(uint8)) ;
        noColors = index+1 ;
    }
    colTable[index] = jj ;
    
    return TRUE ;
}

void
TTsleep(int msec, int intable)
{
    if(intable && ((kbdmode == PLAY) || (clexec == TRUE)))
        return ;

    timerSet(SLEEP_TIMER_ID,-1,msec);
    do
    {
        if(isTimerExpired(AUTOS_TIMER_ID))  /* Alarm expired ?? */
            autoSaveHandler();              /* Initiate an auto save */
        if(isTimerExpired(CALLB_TIMER_ID))  /* Alarm expired ?? */
            callBackHandler();              /* Initiate any callbacks */
        if(isTimerExpired(CURSOR_TIMER_ID)) /* Alarm expired ?? */
            TThandleBlink(0);               /* Initiate a cursor blink */
        if(TTahead() && intable)                    /* Interruptable ?? */
            break ;
    } while(!isTimerExpired(SLEEP_TIMER_ID)) ;
    timerKill(SLEEP_TIMER_ID) ;             /* Kill off the timer */
}


#if	TYPEAH
/* TTahead:	Check to see if any characters are already in the
		keyboard buffer
*/

int
TTahead(void)
{
    extern int kbhit APRAM((void)) ;
    while(kbhit() != 0)
        TTgetkeyc() ;
    
    /* don't process the timers if we have a key waiting!
     * This is because the timers can generate a lot of timer
     * keys, filling up the input buffer - these are not wanted.
     * By not processing, we leave them there until we need them.
     */
    if(TTnoKeys)
        return TTnoKeys ;

    timerCheck(0) ;
#if MOUSE
    /* If an alarm hCheck the mouse */
    if(isTimerExpired(MOUSE_TIMER_ID))
    {
        uint16 mc ;
        union REGS rg ;
        
        /* printf("Clering mouse timer for repeat %1x %1x %d\n",TTallKeys,meTimerState[MOUSE_TIMER_ID],repeattime) ;*/
        timerClearExpired(MOUSE_TIMER_ID) ;
        /* must check that the same mouse button is still pressed  */
        rg.x.ax = 0x0003 ;
        int86(0x33, &rg, &rg) ;
        mc = (mouseState & MOUSE_STATE_BUTTONS) ;
        if((rg.x.bx & MOUSE_STATE_BUTTONS) == mc)
        {
            uint32 arg ;
            mc = ME_SPECIAL | TTmodif | (SKEY_mouse_time+mouseKeys[mc]) ;
            /* mouse-move bound ?? */
            if((!TTallKeys && (decode_key(mc,&arg) != -1)) || (TTallKeys & 0x2))
            {
                /* Timer has expired and timer still bound. Report the key. */
                /* Push the generated keycode into the buffer */
                addKeyToBuffer(mc) ;
                /* Set the new timer and state */
                /* Start a new timer to clock in at 'repeat' intervals */
                /* printf("Setting mouse timer for repeat %1x %1x %d\n",TTallKeys,meTimerState[MOUSE_TIMER_ID],repeattime) ;*/
                timerSet(MOUSE_TIMER_ID,-1,repeattime);
            }
        }
    }
#endif
    if(isTimerExpired(IDLE_TIMER_ID))
    {
        uint32 arg;           /* Decode key argument */
        int index;
        
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
#endif



