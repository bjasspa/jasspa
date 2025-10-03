/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * dosterm.c - Dos terminal support routines.
 *
 * Copyright (C) 1994-2001 Steven Phillips
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
 * Created:     1994
 * Synopsis:    Dos terminal support routines.
 * Authors:     Steven Phillips
 */

#include "emain.h"
#include "efunc.h"
#include "eskeys.h"

#include <dos.h>
#include <io.h>
#include <pc.h>

#if MEOPT_MOUSE

/* Local definitions for mouse handling code */
/* mouseState - An integer interpreted as a bit mask that holds the current state of the mouse interaction. */
#define MOUSE_STATE_LEFT         0x0001 /* Left mouse button is pressed */
#define MOUSE_STATE_MIDDLE       0x0002 /* Middle mouse button is pressed */
#define MOUSE_STATE_RIGHT        0x0004 /* Right mouse button is pressed */
#define MOUSE_STATE_VISIBLE      0x0200 /* Mouse is currently visible */
#define MOUSE_STATE_SHOW         0x0400 /* Mouse active, show next time */
#define MOUSE_STATE_BUTTONS      (MOUSE_STATE_LEFT|MOUSE_STATE_MIDDLE|MOUSE_STATE_RIGHT)

/* bit button lookup - [0] = no keys, [1] = left, [2] = right, [4]=middle */
static meUShort mouseKeys[8] = { 0, 1, 3, 0, 2, 0, 0, 0 } ;

#endif /* MEOPT_MOUSE */

meUByte  Cattr = 0x07 ;

static meShort dosOrgMode ;
static meShort dosOrgDepth ;
static meShort dosResMode=-1 ;
static meShort dosResSpecial ;

#define ME_EXT_FLAG 0x80
meUByte TTextkey_lut [256] =
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


meUShort TTmodif=0 ;

/* Color code */
meUByte *colTable=NULL ;

#define dosNumColors 16
static meUByte dosColors[dosNumColors*3] =
{
    0,0,0,    0,0,200, 0,200,0, 0,200,200, 200,0,0, 200,0,200, 200,200,0, 200,200,200, 
    75,75,75, 0,0,255, 0,255,0, 0,255,255, 255,0,0, 255,0,255, 255,255,0, 255,255,255
} ;
    

void
meSetupProgname(char *progname)
{
    curdir = gwd(0) ;
    if(curdir == NULL)
        /* not yet initialised so mlwrite will exit */
        mlwrite(MWCURSOR|MWABORT|MWWAIT,(meUByte *)"Failed to get cwd\n") ;
    
    /* setup the $progname make it an absolute path. */
    if(executableLookup((meUByte *) progname,evalResult))
        meProgName = meStrdup(evalResult) ;
    else
    {
#ifdef _ME_FREE_ALL_MEMORY
        /* stops problems on exit */
        meProgName = meStrdup(progname) ;
#else
        meProgName = (meUByte *)progname ;
#endif
    }
}

void
meSetupPathsAndUser(void)
{
    meUByte *ss, buff[meBUF_SIZE_MAX] ;
    int ii, ll, gotPaths;
    
    if(meUserName == NULL)
    {
        if(((ss = meGetenv("MENAME")) == NULL) || (ss[0] == '\0'))
            ss = (meUByte *) "user";
        meUserName = meStrdup(ss);
    }
    
    /* get the users home directory, user path and search path */
    if(((ss = meGetenv("HOME")) == NULL) || (ss[0] == '\0'))
        ss = (meUByte *) "c:/";
    fileNameSetHome(ss);

    /* meUserPath & searchPath may not be null due to -v command-line option */
    if((((ss = meUserPath) != NULL) && (ss[0] != '\0')) ||
       (((ss = meGetenv("MEUSERPATH")) != NULL) && (ss[0] != '\0')))
    {
        ll = meStrlen(ss);
        if((ss[ll-1] == DIR_CHAR) || (ss[ll-1] == _CONVDIR_CHAR))
            ll--;
        meUserPath = meMalloc(ll+2);
        memcpy(meUserPath,ss,ll);
        meUserPath[ll++] = DIR_CHAR;
        meUserPath[ll] = '\0';
        fileNameConvertDirChar(meUserPath);
    }
    if((searchPath == NULL) &&
       ((ss = meGetenv("MEPATH")) != NULL) && (ss[0] != '\0'))
        searchPath = meStrdup(ss);
        
    if(searchPath != NULL)
    {
        /* explicit path set by the user, don't need to look at anything else */
        /* we just need to add the $user-path to the front */
        fileNameConvertDirChar(searchPath);
        if((meUserPath != NULL) &&
           (memcmp(searchPath,meUserPath,ll-1) ||
            ((searchPath[ll-1] != mePATH_CHAR) &&
             ((searchPath[ll-1] != DIR_CHAR) || (searchPath[ll] != mePATH_CHAR)))))
        {
            /* meMalloc will exit if it fails as ME has not finished initialising */
            ss = meMalloc(ll + meStrlen(searchPath) + 2);
            memcpy(ss,meUserPath,ll);
            ss[ll] = mePATH_CHAR;
            meStrcpy(ss+ll+1,searchPath);
            meFree(searchPath);
            searchPath = ss;
        }
        /* we have to assume that if meUserPath was null then the first path will be the user path */
        gotPaths = 8;
    }
    else
    {
        /* construct the search-path, put the $user-path first */
        gotPaths = (meUserPath != NULL) ? 8:0;
        if(gotPaths)
            meStrcpy(evalResult,meUserPath);
        {
            evalResult[0] = '\0';
            ll = 0;
        }
        /* Check for setting of $MEINSTALLPATH first, if set, check for $user-path and standard sub-dirs */
        if(((ss = meGetenv ("MEINSTALLPATH")) != NULL) && (ss[0] != '\0'))
        {
            meStrcpy(buff,ss) ;
            ll = mePathAddSearchPath(ll,evalResult,buff,6,&gotPaths);
        }
        else if(homedir != NULL)
        {
            /* look for the user's area in ~/jasspa directory - an exception here, if we find macros or spelling here
             * still look for it in the program area as this may just contain downloaded help and spelling packages */
            ii = gotPaths;
            meStrcpy(buff,homedir);
            meStrcat(buff,"jasspa");
            ll = mePathAddSearchPath(ll,evalResult,buff,6,&ii);
            gotPaths |= (ii & 0x0c);
        }
        
        /* also check for directories in the same location as the binary */
        if((ss=meStrrchr(meProgName,DIR_CHAR)) != NULL)
        {
            ii = (((size_t) ss) - ((size_t) meProgName));
            meStrncpy(buff,meProgName,ii);
            buff[ii] = '\0';
            ll = mePathAddSearchPath(ll,evalResult,buff,9,&gotPaths);
        }
        if(ll > 0)
        {
            searchPath = meStrdup(evalResult);
            fileNameConvertDirChar(searchPath);
        }
    }
    if(meUserPath == NULL)
    {
        if((gotPaths & 0x08) && (searchPath != NULL))
        {
            /* the first path in the search-path is to be used as the user-path */
            if((ss = meStrchr(searchPath,mePATH_CHAR)) != NULL)
                ll = ss-searchPath;
            else
                ll = meStrlen(searchPath);
            if(searchPath[ll-1] == DIR_CHAR)
                ll--;
            meUserPath = meMalloc(ll+2);
            memcpy(meUserPath,searchPath,ll);
            meUserPath[ll++] = DIR_CHAR;
            meUserPath[ll] = '\0';
        }
        else
            meUserPath = meStrdup((meUByte *) "tfs://new-user/");
    }
}

void
TTdump(meBuffer *bp)
{
    meUShort r1, r2, c1, c2 ;
    meUByte buff[TTdepthDefault*TTdepthDefault*2], *ss ;
    meUByte line[TTwidthDefault] ;

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
meInt
meFileGetAttributes(meUByte *fname)
{
#ifdef __DJGPP2__
    return _chmod((char *) fname,0,0) ;
#else
    union REGS reg ;		/* cpu register for use of DOS calls */
    meUByte fn[meBUF_SIZE_MAX] ;
    meInt len ;
    
    reg.x.ax = 0x4300 ;
    len = strlen(fname) ;
    if(fname[len-1] == DIR_CHAR)
    {
        strcpy(fn,fname) ;
        fn[len] = '.' ;
        fn[len+1] = '\0' ;
        reg.x.dx = ((unsigned long) fn) ;
    }
    else
        reg.x.dx = ((unsigned long) fname) ;

    intdos(&reg, &reg);

    if(reg.x.cflag)
        return -1 ;
    return reg.x.ax ;
#endif
}

void
_meFileSetAttributes(meUByte *fn, meUShort attr)
{
#ifdef __DJGPP2__
    _chmod((char *) fn,1,attr) ;
#else
    union REGS reg ;		/* cpu register for use of DOS calls */

    reg.x.ax = 0x4301 ;
    reg.x.cx = attr ;
    reg.x.dx = ((unsigned long) fn) ;
    intdos(&reg, &reg);
#endif
}

#if 0
#define _DEBUG_KEYS
static FILE *dklog=NULL ;
#endif

void
TTgetkeyc(void)
{
    union REGS rg ;
    meUShort ii ;
    meUByte  cc ;

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
    addKeyToBuffer(ii) ;
#if MEOPT_MOUSE
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
#if MEOPT_MOUSE
    union REGS rg ;

    if(meMouseCfg & meMOUSE_ENBLE)
    {
        meUShort mc ;
        meUInt arg ;
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
                /* printf("Setting mouse timer for delay  %1x %1x %d\n",TTallKeys,meTimerState[MOUSE_TIMER_ID],delayTime) ;*/
                timerSet(MOUSE_TIMER_ID,-1,delayTime);
            }
        }
    }
#endif
#if MEOPT_CALLBACK
    /* IDLE TIME: Check the idle time events */        
    if(kbdmode == meIDLE)
        /* Check the idle event */
        doIdlePickEvent() ;
#endif
    
    for(;;)
    {
        handleTimerExpired() ;
        if(TTahead())
            break ;
#if MEOPT_MOUSE
        if(meMouseCfg & meMOUSE_ENBLE)
        {
            meShort row, col, but ;
            meUShort cc ;
            
            /* Get new mouse status. It appears that the fractional bits of
             * the mouse change across reads. ONLY Compare the non-fractional
             * components.
             */
            rg.x.ax = 0x0003 ;
            int86(0x33, &rg, &rg) ;
            but = rg.x.bx & MOUSE_STATE_BUTTONS ;
            col = (rg.x.cx & 0x0ffff)>>3 ;
            row = (rg.x.dx & 0x0ffff)>>3 ;
            
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
                    meUInt arg;                 /* Decode key argument */
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
    
#if MEOPT_MOUSE
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
    if((frameCur->cursorRow <= frameCur->depth) && (frameCur->cursorColumn < frameCur->width))
    {
        meFrameLine *flp;                     /* Frame store line pointer */
        meScheme schm;                      /* Current colour */
        meUByte cc ;                          /* Current cchar  */
        meUByte dcol;

        flp  = frameCur->store + frameCur->cursorRow ;
        cc   = flp->text[frameCur->cursorColumn] ;          /* Get char under cursor */
        schm = flp->scheme[frameCur->cursorColumn] ;        /* Get colour under cursor */

        dcol = TTschemeSet(schm) ;
        
        ScreenPutChar(cc, dcol, frameCur->cursorColumn, frameCur->cursorRow);
    }
}

void
TTshowCur(void)
{
    if((frameCur->cursorRow <= frameCur->depth) && (frameCur->cursorColumn < frameCur->width))
    {
        meFrameLine *flp;                     /* Frame store line pointer */
        meScheme schm;                        /* Current colour */
        meUByte cc;                           /* Current cchar  */
        meUByte dcol;

        flp  = frameCur->store + frameCur->cursorRow;
        cc   = flp->text[frameCur->cursorColumn];          /* Get char under cursor */
        schm = flp->scheme[frameCur->cursorColumn];        /* Get colour under cursor */

        dcol = TTcolorSet(colTable[meStyleGetBColor(meSchemeGetStyle(schm))],
                          colTable[cursorColor[(meModeTest(frameCur->windowCur->buffer->mode,MDOVER)) ? 1:0]]);

        ScreenPutChar(cc, dcol, frameCur->cursorColumn, frameCur->cursorRow);
    }
}

#if MEOPT_MOUSE
void
TTinitMouse(meInt nCfg)
{
    if((nCfg & meMOUSE_NOBUTTONS) != 0)
        meMouseCfg = nCfg & (meMOUSE_ENBLE|meMOUSE_SWAPBUTTONS|meMOUSE_NOBUTTONS);
    else
        meMouseCfg = (meMouseCfg & meMOUSE_NOBUTTONS) | (nCfg & (meMOUSE_ENBLE|meMOUSE_SWAPBUTTONS));
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
        frameChangeDepth(meTRUE,row-(frameCur->depth+1));
        frameChangeWidth(meTRUE,col-frameCur->width);
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

#if MEOPT_MOUSE
    if(meMouseCfg & meMOUSE_ENBLE)
    {
        /* initialise the mouse and flag if okay */
        rg.x.ax = 0x0000 ;
        int86(0x33, &rg, &rg) ;
        /* initialise the mouse screen range */
        /* set mouse hor range */
        rg.x.ax = 0x0007 ;
        rg.x.cx = 0x0000 ;
        rg.x.dx = (((frameCur == NULL) ? TTwidthDefault:frameCur->width)-1) << 3 ;
        int86(0x33, &rg, &rg) ;
        /* set mouse vert range */
        rg.x.ax = 0x0008 ;
        rg.x.cx = 0x0000 ;
        rg.x.dx = ((frameCur == NULL) ? TTdepthDefault:frameCur->depth) << 3 ;
        int86(0x33, &rg, &rg) ;
        /* get the current status */
        rg.x.ax = 0x0003 ;
        int86(0x33, &rg, &rg) ;
        mouse_X = (rg.x.cx & 0x0ffff)>>3 ;
        mouse_Y = (rg.x.dx & 0x0ffff)>>3 ;
        mouseState |= (rg.x.bx & MOUSE_STATE_BUTTONS) ;
    }
#endif
    sgarbf = meTRUE ;
}

int
TTstart(void)
{
    union REGS rg;
#if MEOPT_MOUSE
    int nCfg=0;
#endif
    
#ifdef __DJGPP2__
    /* must call this to disable DJ's C-c signal handling */
    extern void __djgpp_exception_toggle(void);
    __djgpp_exception_toggle() ;
#endif
    dosOrgMode = ScreenMode() ;
    dosOrgDepth = ScreenRows() ;

    TTdepthDefault = dosOrgDepth ;
    TTwidthDefault = ScreenCols() ;

    /* vga cursor emulation */
    rg.h.bl = 0x34;
    rg.x.ax = 0x1200;
    int86(0x10, &rg, &rg);
    
#if MEOPT_MOUSE
    /* initialise the mouse and flag if okay */
    rg.x.ax = 0x0000;
    int86(0x33, &rg, &rg);
    if(rg.x.ax != 0)
    {
        /* value of 0xffff also means 2 buttons */
        if(rg.x.bx == 3)
            /* 3 buttons, so change middle from right to middle */
            nCfg = 3|meMOUSE_ENBLE;
        else
            nCfg = 2|meMOUSE_ENBLE;
    }
    TTinitMouse(nCfg);
#endif
    
    return TTopen();
}

int
TTopen(void)
{
    union REGS rg;

    /* kill the ctrl-break interupt */
    rg.h.ah = 0x33;		/* control-break check dos call */
    rg.h.al = 1;		/* set the current state */
    rg.h.dl = 0;		/* set it OFF */
    intdos(&rg,&rg);	        /* go for it! */

    ibmChangeSRes();

    return meTRUE;
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
    meUByte buf[meSBUF_SIZE_MAX] ;
    int  mode ;

    if(meGetString((meUByte *) "Res mode",0,0,buf,meSBUF_SIZE_MAX) <= 0)
        return meFALSE ;
    mode = meAtoi(buf) ;
    if(meGetString((meUByte *) "Res special",0,0,buf,meSBUF_SIZE_MAX) <= 0)
        return meFALSE ;
    dosResMode = mode ;
    dosResSpecial = meAtoi(buf) ;
    ibmChangeSRes() ;
    return meTRUE ;
}

int
TTaddColor(meColor index, meUByte r, meUByte g, meUByte b)
{
    meUByte *ss ;
    int ii, jj, idif, jdif ;

    jdif = 256+256+256 ;
    ss = dosColors ;
    for(ii=0 ; ii<dosNumColors ; ii++)
    {
        idif  = abs(r - *ss++) ;
        idif += abs(g - *ss++) ;
        idif += abs(b - *ss++) ;
        if(idif < jdif)
        {
            jdif = idif ;
            jj = ii ;
        }
    }
                 
    if(noColors <= index)
    {
        colTable = meRealloc(colTable,(index+1)*sizeof(meUByte)) ;
        memset(colTable+noColors,0,(index-noColors+1)*sizeof(meUByte)) ;
        noColors = index+1 ;
    }
    colTable[index] = jj ;
    
    return meTRUE ;
}

void
TTsleep(int msec, int intable, meVariable **waitVarList)
{
    meUByte *ss ;
    
    if(intable && ((kbdmode == mePLAY) || (clexec == meTRUE)))
        return ;

    if(msec >= 0)
        /* Don't actually need the abs time as this will remain the next alarm */
        timerSet(SLEEP_TIMER_ID,-1,msec);
    else if(waitVarList != NULL)
        timerKill(SLEEP_TIMER_ID) ;             /* Kill off the timer */
    else
        return ;
    
    do
    {
        handleTimerExpired() ;
        if(TTahead() && intable)                    /* Interruptable ?? */
            break ;
        if((waitVarList != NULL) &&
           (((ss=getUsrLclCmdVar((meUByte *)"wait",*waitVarList)) == errorm) || !meAtoi(ss)))
            break ;
    } while(!isTimerExpired(SLEEP_TIMER_ID)) ;
    timerKill(SLEEP_TIMER_ID) ;             /* Kill off the timer */
}


#if MEOPT_TYPEAH
/* TTahead:	Check to see if any characters are already in the
		keyboard buffer
*/

int
TTahead(void)
{
    extern int kbhit(void);
    
    while(kbhit() != 0)
        TTgetkeyc();
    
    /* Check the timers but don't process them if we have a key waiting!
     * This is because the timers can generate a lot of timer
     * keys, filling up the input buffer - these are not wanted.
     * By not processing, we leave them there until we need them.
     */
    timerCheck(0);
    
    if(TTnoKeys)
        return TTnoKeys;

#if MEOPT_MOUSE
    /* If an alarm hCheck the mouse */
    if(isTimerExpired(MOUSE_TIMER_ID))
    {
        meUShort mc;
        union REGS rg;
        
        /* printf("Clering mouse timer for repeat %1x %1x %d\n",TTallKeys,meTimerState[MOUSE_TIMER_ID],repeatTime) ;*/
        timerClearExpired(MOUSE_TIMER_ID);
        /* must check that the same mouse button is still pressed  */
        rg.x.ax = 0x0003;
        int86(0x33, &rg, &rg);
        mc = (mouseState & MOUSE_STATE_BUTTONS);
        if((rg.x.bx & MOUSE_STATE_BUTTONS) == mc)
        {
            meUInt arg ;
            mc = ME_SPECIAL | TTmodif | (SKEY_mouse_time+mouseKeys[mc]) ;
            /* mouse-move bound ?? */
            if((!TTallKeys && (decode_key(mc,&arg) != -1)) || (TTallKeys & 0x2))
            {
                /* Timer has expired and timer still bound. Report the key. */
                /* Push the generated keycode into the buffer */
                addKeyToBuffer(mc) ;
                /* Set the new timer and state */
                /* Start a new timer to clock in at 'repeat' intervals */
                /* printf("Setting mouse timer for repeat %1x %1x %d\n",TTallKeys,meTimerState[MOUSE_TIMER_ID],repeatTime) ;*/
                timerSet(MOUSE_TIMER_ID,-1,repeatTime);
            }
        }
    }
#endif
#if MEOPT_CALLBACK
    if(isTimerExpired(IDLE_TIMER_ID))
    {
        meUInt arg;           /* Decode key argument */
        int index;
        
        if((index=decode_key(ME_SPECIAL|SKEY_idle_time,&arg)) != -1)
        {
            /* Execute the idle-time key */
            execFuncHidden(ME_SPECIAL|SKEY_idle_time,index,arg);
            
            /* Now set the timer for the next */
            timerSet(IDLE_TIMER_ID,-1,idleTime);
        }
        else if(decode_key(ME_SPECIAL|SKEY_idle_drop,&arg) != -1)
            meTimerState[IDLE_TIMER_ID] = IDLE_STATE_DROP;
        else
            meTimerState[IDLE_TIMER_ID] = 0;
    }
#endif
    return TTnoKeys;
}
#endif
