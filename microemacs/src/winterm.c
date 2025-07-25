/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * winterm.c - Win32 platform support.
 *
 * Copyright (C) 1996-2001 Jon Green
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
 * Created:     21/12/1996
 * Synopsis:    Win32 platform support.
 * Authors:     Jon Green, Matthew Robinson & Steven Phillips
 * Description:
 *     This is the windows terminal driver for Microsoft windows environments.
 *     This file contains the display and terminal input functions for
 *     collecting and displaying data.
 *
 * Notes:
 *
 * SCREEN
 * ======
 * This differs from most of the other EMACS screen drivers in that a memory
 * representation of the screen is constructed. All writes to the screen update
 * the intermal memory buffer only and invalidate a region of the screen.
 * Subsequent messages from the Windows Message dispatcher will inform us that
 * a portion of the screen requires re-painting and at that point the information
 * is written to the screen.
 *
 * Note that it is possible to update the screen immediatly, however this does
 * seem to cause some problems with windows itself - the documentation does
 * point out that this is not advisable.
 *
 * The painting takes place on a WM_PAINT message. This message may occur while
 * the program is active AND inactive. The active case is obvious, the inactive
 * case arises when another window has passed over the Emacs window, at which
 * point windows requests that we re-draw that part of the screen which has
 * become corrupt. Hence we need the physical srceen buffer to allow us to
 * repaint the window.
 *
 * INPUT
 * =====
 * The input has been bound to the same mappings as the IBM-PC version of EMACS
 * hence the same escape codes etc are received.
 *
 * MOUSE
 * =====
 * The mouse is pretty standard. However if you are using a 3 button mouse
 * ensure that the middle button is not bound to "double-click" or emacs
 * will not be able to see the middle button.
 *
 * Notes
 *
 * !!!! WARNING WARNING WARNING !!!!
 *
 * Unless you know what you are doing DO NOT MESS with the event mechanism.
 * It might work but it may not be correct.
 */

/* Emacs standard headers */
#include "emain.h"
#include "commdlg.h"                    /* Common dialogs */
#include "cderr.h"                      /* Common dialoge errors */
#include "evers.h"                      /* Version information */
#include "eskeys.h"                     /* Emacs special keys */
#include "winterm.h"                    /* Windows terminal definitions */
#include "wintermr.h"                   /* Windows resource file */

#include <process.h>
#include <shellapi.h>
#include <shlobj.h>

#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL (WM_MOUSELAST+1)  // message that will be supported by the OS
#endif

#define _WIN_DEBUG_KEY   0
#define _WIN_DEBUG_MOUSE 0
#define _WIN_DEBUG_MSG   0
#define _WIN_DEBUG_POS   0

#if _WIN_DEBUG_KEY || _WIN_DEBUG_MOUSE || _WIN_DEBUG_MSG || _WIN_DEBUG_POS
FILE *logfp=NULL;
#endif

/* For the Win32s then we have to perform a thunking operation in order to get
 * a synchronous spawn to operate correctly. In addition to the fact we
 * actually have to process using a BAT file because CreateProcess() and
 * WinExec() are both f**ked !! CreateProcess() does not allow re-direction.
 * WinExec() does not allow the parent environment to be inherited, therefore
 * we can never get the current directory correct !!. To find all of this out
 * you need to look in the MSDM developer database - even then it is not spelt
 * out. */
#ifdef _WIN32s
#define SYNCHSPAWN 1                    /* Our command to spawn */
#define W32SUT_32  1                    /* Tell "w32sut.h" that we are 32-bit complier */
#include "win32s/w32sut.h"              /* Local to the win32s directory */
#endif

HWND baseHwnd=meHWndNull;               /* Handle to base hidden window */

#ifdef _ME_WINDOW
/* When we get a message that we can't completely handle we must dispatch the
 * message and let windows handle it. Used a define for easy searching */
#define meMessageHandler(msg) DispatchMessage(msg)

#define meFrameGetWinData(ff)               ((meFrameData *) (ff)->termData)
#define meFrameDataGetWinHandle(ff)         ((ff)->hwnd)
#define meFrameDataGetWinCanvas(ff)         ((ff)->canvas)
#define meFrameDataGetWinMaximized(ff)      ((ff)->maximized)
#define meFrameDataGetWinPaintAll(ff)       ((ff)->paintAll)
#define meFrameDataGetWinPaintDepth(ff)     ((ff)->paintDepth)
#define meFrameDataGetWinPaintStartCol(ff)  ((ff)->paintStartCol)
#define meFrameDataGetWinPaintEndCol(ff)    ((ff)->paintEndCol)

/* Macros to move into and out of screen <=> character space */
#define rowToClient(y) ((y) * eCellMetrics.cell.sizeY)
#define colToClient(x) ((x) * eCellMetrics.cell.sizeX)
#define clientToRow(y) ((y) / eCellMetrics.cell.sizeY)
#define clientToCol(x) ((x) / eCellMetrics.cell.sizeX)

/* CellMetrics
 * This structure defines the character font items used to address the
 * screen. Thise details the size of the font in screen space and
 * also contains the windows definition of the font.
 *
 * The cellRowPos/cellColPos is a look up table for screen to client
 * coordinates. This is the text position.
 *
 * The LUT's provide a faster and more convienient method
 * of moving from screen coordinate space to client coordinate
 * space. The 2 pixel offset from the edge of the screen
 * makes testing for the edges a little cumbersome and
 * is best done via the LUT since it is unconditional and
 * therefore faster - at the expense of 1/2K's worth of memory.
 */

typedef struct
{
    COLORREF cpixel;                    /* Color of the pixel */
    COLORREF rgb;                       /* Requested RGB colour */
} PaletteItem;                          /* Item in the palette */

typedef struct
{
    HPALETTE hPal;                      /* Windows palette table */
    int hPalSize;                       /* Size of the palette */
    int *hPalRefCount;                  /* Reference count for palette */
    PaletteItem *cPal;                  /* Emacs colour palette item */
} PaletteInfo;                          /* Color palette information */

typedef struct
{
    CharMetrics cell;                   /* Metrics of a character cell */
    HFONT fontdef[meFONT_MAX];          /* Definition of the font. */
    PaletteInfo pInfo;                  /* Emacs colour palette tables */
    meShort *cellRowPos;                /* Character cell position in Y */
    meShort *cellColPos;                /* Character cell position in X */
    meUByte *cellColTmpPos;             /* Temporary X position */
    INT     *cellSpacing;               /* Spacing of cells */
    meShort  cellDepthCount;            /* Screen cell number of rows */
    meShort  cellWidthCount;            /* Screen cell number of columns */
    int      maxDepth;                  /* Window maximum depth */
    int      maxPosTop;                 /* Maximized window top pos */
    int      maxPosLeft;                /* Maximized window left pos */
    int      borderDepth;               /* Window border depth */
    int      borderWidth;               /* Window border width */
    RECT     workArea;                  /* primary monitor working area */
} CellMetrics;

CellMetrics eCellMetrics;               /* Cell metrics */
RECT   ttRect;                          /* Area of screen to update */
static int ttshowState;                 /* Show state of the window */
static HBRUSH ttBrush = NULL;           /* Current background brush */
#endif /* _ME_WINDOW */

#if MEOPT_CLIENTSERVER
HANDLE *clientHandle = INVALID_HANDLE_VALUE;
HANDLE *serverHandle = INVALID_HANDLE_VALUE;
HANDLE *connectHandle= INVALID_HANDLE_VALUE;
int ttServerSize = 0;
int ttServerToRead = 0;
meUByte ttServerCheck = 0;
#endif

LRESULT APIENTRY
MainWndProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);
/* The following stores raw key of potential AltGr special key */
static meUShort keyDownChr=0;

/***************************************************************************/
#ifdef _ME_CONSOLE
/* Windows NT Console screen buffer handle.  All console I/O is done through
 * this handle.
 */
static HANDLE hInput, hOutput;			/* Handles to console I/O */
static char chConsoleTitle[256];		/* Preserve the title of the console. */
static DWORD ConsoleMode, OldConsoleMode;	/* Current and old console modes */
static SMALL_RECT consolePaintArea={0};		/* Update area for console */
static int ciScreenSize=0;			/* Size of screen buffer memory */
static CHAR_INFO *ciScreenBuffer=NULL;		/* Copy of screen buffer memory */
static COORD OldConsoleSize;
meUInt *colTable=NULL;				/* Currently allocated colour table */
#define consoleNumColors 16			/* Number of colours in console window */
static meUByte consoleColors[consoleNumColors*3] =
{
    0,0,0,    0,0,200, 0,200,0, 0,200,200, 200,0,0, 200,0,200, 200,200,0, 200,200,200,
    75,75,75, 0,0,255, 0,255,0, 0,255,255, 255,0,0, 255,0,255, 255,255,0, 255,255,255
};

/* Function prototypes */
static HANDLE
WinKillToClipboard(void);
#endif
/***************************************************************************/

/* Local variables holding context state */
#define TIMER_INACTIVE      0           /* Inactive value for the mouse */

static int ttfcol = meCOLOR_FDEFAULT;   /* Foregound colour */
static int ttbcol = meCOLOR_BDEFAULT;   /* Background colour */
static meUShort ttmodif=0;              /* Modified keyboard state */
HANDLE ttInstance;                      /* Instance of the application */
static DWORD ttThreadId=0;              /* Current thread identity */

/* Font type settings */
LOGFONT ttlogfont={0};                  /* Current logical font */
#ifdef _ME_WINDOW
static meUByte ttPitchFam=0;
#endif

#if MEOPT_MOUSE
/* Local definitions for mouse handling code */
/* mouseState - An integer interpreted as a bit mask that holds the current state of the mouse interaction. */
#define MOUSE_STATE_LEFT         0x0001 /* Left mouse button is pressed  */
#define MOUSE_STATE_MIDDLE       0x0002 /* Middle mouse button is pressed*/
#define MOUSE_STATE_RIGHT        0x0004 /* Right mouse button is pressed */
#define MOUSE_STATE_VISIBLE      0x0400 /* Mouse is currently visible    */
#define MOUSE_STATE_BUTTONS      (MOUSE_STATE_LEFT|MOUSE_STATE_MIDDLE|MOUSE_STATE_RIGHT)
#define MOUSE_STATE_LOCKED       0x0800 /* Mouse is locked in */

static WPARAM mouseButs=0;              /* State of the mouse buttons. */
/* bit button lookup - [0] = no keys, [1] = left, [2]=middle, [4] = right */
static meUShort mouseKeys[8] = { 0, 1, 2, 0, 3, 0, 0, 0 };
static meUByte mouseInFrame=0;

#ifdef _ME_WINDOW
#define mouseHide() ((mouseState & MOUSE_STATE_VISIBLE) ? (SetCursor(NULL),(mouseState &= ~MOUSE_STATE_VISIBLE)):0)
#define mouseShow() ((mouseState & MOUSE_STATE_VISIBLE) ? 0:(SetCursor(meCursors[meCurCursor]),(mouseState |= MOUSE_STATE_VISIBLE)))

/* Cursor definitions  */
static meUByte meCurCursor=0;
static HCURSOR meCursors[meCURSOR_COUNT]={NULL,NULL,NULL,NULL,NULL,NULL,NULL};
static LPCTSTR meCursorName[meCURSOR_COUNT]=
{
    IDC_ARROW,
    IDC_ARROW,
    IDC_IBEAM,
    IDC_CROSS,
    IDC_UPARROW,
    IDC_WAIT,
    IDC_NO
};
#else
#define mouseHide()
#define mouseShow()
#endif /* _ME_WINDOW */

/* Convert the mouse coordinates to cell space. Compute the fractional bits
 * which are 1/128ths. Because we lock the mouse into the window then cater
 * for the -ve case by ANDing with 0x8000.
 * Added console support
 */
#define __winMousePosUpdate(lpos)                                        \
mouseInFrame = 1;                                                        \
if(LOWORD(lpos) & 0x8000)         /* Dirty check for -ve */              \
    mouse_X = 0, mouse_dX = mouseInFrame = 0;                            \
else                                                                     \
{                                                                        \
    mouse_X = clientToCol(LOWORD(lpos));                                 \
    mouse_dX = ((((LOWORD(lpos)) - colToClient(mouse_X)) << 8) /         \
                eCellMetrics.cell.sizeX);                                \
    if(mouse_X > frameCur->width)                                        \
    {                                                                    \
        mouse_X = frameCur->width;                                       \
        mouseInFrame = 0;                                                \
    }                                                                    \
}                                                                        \
if(HIWORD(lpos) & 0x8000)         /* Dirty check for -ve */              \
mouse_Y = 0, mouse_dY = mouseInFrame=0;                                  \
else                                                                     \
{                                                                        \
    mouse_Y = clientToRow(HIWORD(lpos));                                 \
    mouse_dY = ((((HIWORD(lpos)) - rowToClient(mouse_Y)) << 8) /         \
                eCellMetrics.cell.sizeY);                                \
    if(mouse_Y > frameCur->depth)                                        \
    {                                                                    \
        mouse_Y = frameCur->depth;                                       \
        mouseInFrame = 0;                                                \
    }                                                                    \
}

#ifdef _ME_CONSOLE
#ifdef _ME_WINDOW
#define mousePosUpdate(lpos)                                             \
do {                                                                     \
    if(meSystemCfg & meSYSTEM_CONSOLE)                                   \
    {                                                                    \
	mouse_X = LOWORD(lpos);                                          \
	mouse_Y = HIWORD(lpos);                                          \
    }                                                                    \
    else                                                                 \
    {                                                                    \
        __winMousePosUpdate(lpos)                                        \
    }                                                                    \
} while(0)

#else /* _ME_WINDOW */

#define mousePosUpdate(lpos)                                             \
do {                                                                     \
    mouse_X = LOWORD(lpos);                                              \
    mouse_Y = HIWORD(lpos);                                              \
} while(0)

#endif /* _ME_WINDOW */

#else /* _ME_CONSOLE - window only */

#define mousePosUpdate(lpos)                                             \
do {                                                                     \
    __winMousePosUpdate(lpos)                                            \
} while(0)
#endif /* _ME_CONSOLE */

#else /* MEOPT_MOUSE */
/* if the mouse is disabled we must still do some work or it will look very
 * neglected - do the bare minumum */

#ifdef _ME_WINDOW
#define MOUSE_STATE_VISIBLE      0x0400 /* Mouse is currently visible    */

static int  mouseState=0;               /* State of the mouse. */

#define meCURSOR_DEFAULT 0
#define meCURSOR_ARROW   0
#define meCurCursor      0
#define mouseHide() ((mouseState & MOUSE_STATE_VISIBLE) ? (SetCursor(NULL),(mouseState &= ~MOUSE_STATE_VISIBLE)):0)
#define mouseShow() ((mouseState & MOUSE_STATE_VISIBLE) ? 0:(SetCursor(meCursors[meCurCursor]),(mouseState |= MOUSE_STATE_VISIBLE)))

static HCURSOR meCursors[1]={NULL};
#else
#define mouseHide()
#define mouseShow()
#endif /* _ME_WINDOW */

#endif /* MEOPT_MOUSE */

#if MEOPT_MWFRAME

static meFrame *
meMessageGetFrame(HWND hwnd)
{
#ifdef _ME_CONSOLE
    if(meSystemCfg & meSYSTEM_CONSOLE)
        return frameCur;
#endif /* _ME_CONSOLE */
    
    meFrameLoopBegin();
    
    if(((loopFrame->flags & meFRAME_HIDDEN) == 0) && (meFrameGetWinHandle(loopFrame) == hwnd))
        return loopFrame;
    
    meFrameLoopEnd();
    
    return NULL;
}

#else

#define meMessageGetFrame(event) frameCur

#endif

#ifndef _WIN32_WINNT
int platformId;                         /* Running under NT, 95, or Win32s? */
#endif

/****************************************************************************
 *
 * PORTING FUNCTIONS
 *
 ****************************************************************************/

#ifdef _WIN32s
/**************************************************************************
 * Function: DWORD SynchSpawn(LPTSTR, UINT)                                *
 *                                                                         *
 * Purpose: Thunk to 16-bit code. This allows a synchronous process spawn  *
 * i.e. it only returns when the new process has been created.             *
 **************************************************************************/
static DWORD
SynchSpawn( LPCSTR lpszCmdLine, UINT nCmdShow )
{
    static int doneOnce = 0;            /* Have loaded DLL once */
    UT32PROC pfnUTProc = NULL;
    DWORD dwVersion;
    BOOL fWin32s;
    DWORD Args[2];
    PVOID Translist[2];
    DWORD status;
    
    /* Find out if we're running on Win32s */
    dwVersion = GetVersion();
    fWin32s = (BOOL) (!(dwVersion < 0x80000000)) && (LOBYTE(LOWORD(dwVersion)) < 4);
    if(!fWin32s)
        return 0;                       /* Not win32s */
    
    /* Register the 16bit DLL. We do this when we are called. This saves
       problems with a win32s 32-bit DLL under Win 3.1 with win32s installed. */
again:
    if(UTRegister(ttInstance,           /* 'me's module handle */
                    "methnk16.dll",     /* 16-bit thunk dll */
                    NULL,               /* Nothing to do */
                    "UTProc",           /* 16-bit dispatch routine */
                    &pfnUTProc,         /* Receives thunk address */
                    NULL,               /* No callback function */
                    NULL) == meFALSE)     /* no shared memroy */
    {
        
        /* This fails the first time !! */
        if(doneOnce == 0)
        {
            doneOnce = 1;
            goto again;
        }
        return 0;
    }
    
    /* Build the argument list to the 16 bit side */
    Args[0] = (DWORD) lpszCmdLine;
    Args[1] = (DWORD) nCmdShow;
    
    Translist[0] = &Args[0];
    Translist[1] = NULL;
    
    status = (* pfnUTProc)(Args, SYNCHSPAWN, Translist);
    
    /* Unregister the DLL */
    UTUnRegister(ttInstance);
    return status;
}
#endif

/* gettimeofday - Retreives the time of day to millisecond resolution */
void
gettimeofday(struct meTimeval *tp, struct meTimezone *tz)
{
    SYSTEMTIME stime;
    UNREFERENCED_PARAMETER(tz);
    
    /* Get the second resolution time */
    tp->tv_sec = time(NULL);
    
    /* Get the microsecond time */
    GetLocalTime(&stime);
    tp->tv_usec = (long)(stime.wMilliseconds * 1000);
}

/**************************************************************************
 *
 * Client/Server Functions
 *
 **************************************************************************/
#if MEOPT_CLIENTSERVER
void
TTopenClientServer(void)
{
    /* If the server has not been created then create it now */
    if(serverHandle == INVALID_HANDLE_VALUE)
    {
        meIPipe *ipipe;
        meBuffer *bp;
        meMode sglobMode;
        meUByte fname[meBUF_SIZE_MAX];
        meInt ii;
        
        /* create the response file name */
        mkTempName(fname,meUserName,(meUByte *) ".rsp");
        
        /* Open the response file for read/write, if this fails we are not the server, another ME is */
        if((clientHandle = CreateFile((LPCSTR) fname,GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,
                                      FILE_ATTRIBUTE_NORMAL,NULL)) == INVALID_HANDLE_VALUE)
        {
            meSystemCfg &= ~meSYSTEM_CLNTSRVR;
            return;
        }
        /* Open command file for read/write */
        mkTempName(fname,meUserName,(meUByte *) ".cmd");
        if((serverHandle = CreateFile((LPCSTR) fname,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,
                                      CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL)) == INVALID_HANDLE_VALUE)
        {
            CloseHandle(clientHandle);
            clientHandle = INVALID_HANDLE_VALUE;
            meSystemCfg &= ~meSYSTEM_CLNTSRVR;
            return;
        }
        /* Create the ipipe buffer */
        meModeCopy(sglobMode,globMode);
        meModeSet(globMode,MDPIPE);
        meModeSet(globMode,MDLOCK);
        meModeSet(globMode,MDHIDE);
        meModeClear(globMode,MDWRAP);
        meModeClear(globMode,MDUNDO);
        if(((bp=bfind(BserverN,BFND_CREAT)) == NULL) ||
           ((ipipe = meMalloc(sizeof(meIPipe))) == NULL))
        {
            CloseHandle(clientHandle);
            CloseHandle(serverHandle);
            clientHandle = INVALID_HANDLE_VALUE;
            serverHandle = INVALID_HANDLE_VALUE;
            meSystemCfg &= ~meSYSTEM_CLNTSRVR;
            return;
        }
        meModeCopy(globMode,sglobMode);
        bp->intFlag |= BIFNODEL;
        ipipe->next = ipipes;
        ipipe->pid = 0;
        ipipe->rfd = serverHandle;
        ipipe->outWfd = clientHandle;
        ipipe->process = 0;
        ipipe->processId = 0;
        ipipe->thread = NULL;
        ipipe->childActive = NULL;
        ipipe->threadContinue = NULL;
        ipipes = ipipe;
        noIpipes++;
        ipipe->bp = bp;
        /* setup the response file and server buffer */
        {
            meUByte buff[meBUF_SIZE_MAX+20];
            
#ifdef _64BIT
            ii = sprintf((char *)buff,"%llu\n",(size_t) baseHwnd);
#else
            ii = sprintf((char *)buff,"%d\n",(int) baseHwnd);
#endif
            WriteFile(clientHandle,buff,ii,(DWORD *)&ii,NULL);
            
            sprintf((char *)buff,"Client Server: %s\n\n",fname);
            addLineToEob(bp,buff);     /* Add string */
            bp->dotLine = meLineGetPrev(bp->baseLine);
            bp->dotOffset = 0;
            bp->dotLineNo = bp->lineCount-1;
            meAnchorSet(bp,'I',bp->dotLine,bp->dotLineNo,bp->dotOffset,1);
        }
        /* Set up the window dimensions - default to having auto wrap */
        ipipe->flag = 0;
        ipipe->strRow = 0;
        ipipe->strCol = 0;
        ipipe->noRows = 0;
        ipipe->noCols = 0;
        ipipe->curRow = (meShort) bp->dotLineNo;
        /* get a popup window for the command output */
        ipipeSetSize(frameCur->windowCur,bp);
    }
}

int
TTcheckClientServer(void)
{
    int ii;
    
    if(serverHandle == INVALID_HANDLE_VALUE)
        return 0;
    /* The handle exists. If the file is non-NULL then there is something to
     * do */
    ii = GetFileSize(serverHandle, NULL);
    if(ii == ttServerSize)
        return 0;
    ttServerToRead += ii - ttServerSize;
    ttServerSize = ii;
    ttServerCheck = 0;
    return 1;
}

void
TTkillClientServer(void)
{
    if(serverHandle != INVALID_HANDLE_VALUE)
    {
        meIPipe *ipipe;
        meUByte fname[meBUF_SIZE_MAX];
        
        /* get the ipipe node */
        ipipe = ipipes;
        while(ipipe != NULL)
        {
            if(ipipe->pid == 0)
                break;
            ipipe = ipipe->next;
        }
        /* if found (should be) delete buffer (this will close the file handles */
        if(ipipe != NULL)
        {
            ipipe->bp->intFlag |= BIFBLOW;
            zotbuf(ipipe->bp,1);
        }
        else
        {
            /* Close and delete the server files */
            CloseHandle(serverHandle);
            CloseHandle(clientHandle);
        }
        clientHandle = INVALID_HANDLE_VALUE;
        serverHandle = INVALID_HANDLE_VALUE;
        meSystemCfg &= ~meSYSTEM_CLNTSRVR;
        
        /* remove the command and response files */
        mkTempName(fname,meUserName,(meUByte *) ".cmd");
        DeleteFile((char *) fname);
        mkTempName(fname,meUserName,(meUByte *) ".rsp");
        DeleteFile((char *) fname);
    }
    if(connectHandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(connectHandle);
        connectHandle = INVALID_HANDLE_VALUE;
    }
}

int
TTconnectClientServer(void)
{
    /* If the server has not been connected then do it now */
    if(connectHandle == INVALID_HANDLE_VALUE)
    {
        HANDLE hndl;
        meUByte fname[meBUF_SIZE_MAX];
        DWORD ii;
        
        /* Create the file name, if the file exists, or deleting it
         * succeeds then there is no server, fail */
        mkTempName(fname,meUserName,(meUByte *) ".cmd");
        if(meTestExist(fname) || DeleteFile((LPCSTR) fname))
            return 0;
        if((connectHandle = CreateFile((LPCSTR) fname, GENERIC_WRITE, FILE_SHARE_READ, NULL,
                                       OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,NULL)) == INVALID_HANDLE_VALUE)
            return 0;
        /* Goto the end of the file */
        SetFilePointer(connectHandle,0,NULL,FILE_END);
        
        /* Try opening the response file and get the base window handle value */
        mkTempName(fname,meUserName,(meUByte *) ".rsp");
        if((hndl = CreateFile((LPCSTR) fname, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL,
                              OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) != INVALID_HANDLE_VALUE)
        {
            if((ReadFile(hndl,&fname,20,&ii,NULL) != 0) && (ii > 0))
                baseHwnd = (HWND) mePtrFromStr(fname);
            CloseHandle(hndl);
        }
    }
    return 1;
}
void
TTsendClientServer(meUByte *line)
{
    if(connectHandle != INVALID_HANDLE_VALUE)
    {
        int ll=meStrlen(line);
        DWORD ww;
        WriteFile(connectHandle,line,ll,&ww,NULL);
    }
}

#endif

#ifdef _ME_CONSOLE
/****************************************************************************
 *
 * WINDOWS CONSOLE FUNCTIONS
 *
 ****************************************************************************/

/*
 * ConsolePaint
 * Paint to the console window the updated region of text from the virtual
 * screen store. Note that we are only painting the region of the
 * screen that has changed. (This is bound to TTflush())
 *
 * However...this is not done very efficiently at the moment.  It aparently
 * takes a long time to redraw the console window, so the more optimisation
 * the better.  Unfortunately, the current code marks the smallest rectangle
 * that encompases all changes as the update region.  This means if you have
 * something on the mode line that changes (ie. current line) and the cursor
 * is moving at the top of the screen, then almost the whole screen is
 * re-drawn, which can cause flicker while moving the cursor.  It may be
 * better to have two update regions so that the current line and mode line
 * can be updated without redrawing the whole screen.  This problem is really
 * only apparent when moving the cursor.  Other operations do not usually
 * repeat at such a rate as to cause flicker.
 *
 */
BOOL
ConsolePaint(void)
{
    
    /* Work out region to update */
    if(consolePaintArea.Right > 0)
    {
        COORD coordUpdateBegin, coordBufferSize;
        
        /* Set size of screen buffer */
        coordBufferSize.X = frameCur->width;
        coordBufferSize.Y = frameCur->depth+1;
        
        consolePaintArea.Right--;
        /* top left cord of buffer to write from */
        coordUpdateBegin.X = consolePaintArea.Left;
        coordUpdateBegin.Y = consolePaintArea.Top;
        
        /* Write to console */
        WriteConsoleOutput(hOutput, ciScreenBuffer, coordBufferSize,
                           coordUpdateBegin, &consolePaintArea);
        
        /* Remove the region, as we just updated it */
        consolePaintArea.Right = consolePaintArea.Bottom = 0;
        consolePaintArea.Left = consolePaintArea.Top = (SHORT) 0x7fff;
    }
    return 1;
}

/* Draw string to console buffer */
void
ConsoleDrawString(meUByte *ss, WORD wAttribute, int x, int y, int len)
{
    CHAR_INFO *pCI;     /* Pointer to current screen buffer location */
    BOOL bAny = meFALSE;  /* Anything to refresh? */
    meUByte cc;
    int r=x+len;
    
    /* Get pointer to correct location in screen buffer */
    pCI = &ciScreenBuffer[(y * frameCur->width) + x];
    
    /* Copy the string to the screen buffer memory, and flag any changes */
    while(--len >= 0)
    {
        if(((cc=*ss++) != pCI->Char.AsciiChar) || (wAttribute != pCI->Attributes))
        {
            bAny = meTRUE;
            pCI->Char.AsciiChar = cc;
            pCI->Attributes = wAttribute;
        }
        pCI++;
    }
    
    /* Adjust the current update region */
    if(bAny)
    {
        if(y < consolePaintArea.Top)
            consolePaintArea.Top = y;
        if(y > consolePaintArea.Bottom)
            consolePaintArea.Bottom = y;
        if(x < consolePaintArea.Left)
            consolePaintArea.Left = x;
        if(r > consolePaintArea.Right)
            consolePaintArea.Right = r;
    }
}

/* signal handler routine for console app */
BOOL WINAPI
ConsoleHandlerRoutine(DWORD dwCtrlType)
{
    /* Trap user's click on 'X' window button, and exit cleanly */
    switch(dwCtrlType)
    {
    case CTRL_BREAK_EVENT:
    case CTRL_CLOSE_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
        meDie();
    }
    return meFALSE;
}

/*
 * TTend
 * Close the terminal down
 */
int
TTend(void)
{
#ifdef _ME_WINDOW
    if(meSystemCfg & meSYSTEM_CONSOLE)
#endif /* _ME_WINDOW */
    {
        CONSOLE_CURSOR_INFO CursorInfo;
        COORD dwCursorPosition;
        
        /* Restore the console mode and title */
        if(hInput != INVALID_HANDLE_VALUE)
            SetConsoleMode(hInput, OldConsoleMode);
        
        /* Show the cursor */
        GetConsoleCursorInfo(hOutput, &CursorInfo);
        CursorInfo.bVisible = meTRUE;
        SetConsoleCursorInfo(hOutput, &CursorInfo);
#if MEOPT_EXTENDED
        if((alarmState & meALARM_PIPED) == 0)
#endif
        {
            /* restore the console buffer size */
            SetConsoleScreenBufferSize(hOutput, OldConsoleSize);
            
            /* Set cursor to bottom of screen, and print a newline */
            dwCursorPosition.X = 0;
            dwCursorPosition.Y = TTdepthDefault - 1;
            SetConsoleCursorPosition(hOutput, dwCursorPosition);
            putchar('\n');
        }
        SetConsoleTitle(chConsoleTitle);
    }
    return meTRUE;
}

/* Get a console message and format as a standard windows message */
static int
meGetConsoleMessage(MSG *msg, int mode)
{
    INPUT_RECORD ir;
    DWORD dwCount;
    
    /* If in console mode, check whether we must render the clipboard */
    if((clipState & CLIP_OWNER) && OpenClipboard(NULL))
    {
        HANDLE hmem;
        
        hmem = WinKillToClipboard();
        EmptyClipboard();
        SetClipboardData(CF_UNICODETEXT,hmem);
        CloseClipboard();
        
        clipState &= ~CLIP_OWNER;
    }
    
    /* Get the next keyboard/mouse/resize event */
    if(ReadConsoleInput(hInput, &ir, 1, &dwCount) == 0)
    {
        /* if ReadConsoleInput fails we have lost the console input
         * and as the user is waiting on this bomb out */
        if(!mode)
            meDie();
        hInput = INVALID_HANDLE_VALUE;
        return meFALSE;
    }
    /* Let the proper event handler field this event */
    if(ir.EventType == KEY_EVENT)
    {
        KEY_EVENT_RECORD *kr = &ir.Event.KeyEvent;
#if _WIN_DEBUG_KEY
        if(logfp != NULL)
        {
            fprintf(logfp,"ConsoleKeyMsg   %d,%d,%d,%d\n",kr->bKeyDown,kr->wVirtualKeyCode,kr->dwControlKeyState,kr->uChar.AsciiChar);
            fflush(logfp);
        }
#endif
        
        /* Make message a la windows GUI */
        msg->lParam = 0;
        /* SWP - 8/5/99 another odd one from bill, the cursor keys on win9? seem
         * to come through with an AsciiChar value of -32 or 224 instead of 0... Why? */
        if((kr->uChar.AsciiChar == 0) || (((meUByte) kr->uChar.AsciiChar) == 224))
        {
            /* WM_KEYDOWN of WM_KEYUP */
            if(kr->bKeyDown)
            {
                /* on win98, for some reason that only bill can explain, when a cursor
                 * key is pressed a shift keydown is generated and when released, just
                 * before a shift keyup is generated. to stop this, only allow shift
                 * keydown events if the the shift bit is set */
                if((kr->wVirtualKeyCode != VK_SHIFT) || (kr->dwControlKeyState & SHIFT_PRESSED))
                    msg->message = WM_KEYDOWN;
                else
                    msg->message = 0;
            }
            else
                msg->message = WM_KEYUP;
            msg->wParam = kr->wVirtualKeyCode;
        }
        else if(kr->bKeyDown)
        {
            msg->message = WM_CHAR;
            msg->wParam = (meUByte) kr->uChar.AsciiChar;
        }
        else
            msg->message = 0;
        
        /* if we filled in a message, then success! */
        if(msg->message != 0)
        {
            /* Set up the modifier key state */
            ttmodif = 0;
            if(kr->dwControlKeyState & (LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED))
            {
                msg->lParam |= 1<<29;
                ttmodif |= ME_ALT;
            }
            if(kr->dwControlKeyState & SHIFT_PRESSED)
                ttmodif |= ME_SHIFT;
            if(kr->dwControlKeyState & (LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED))
                ttmodif |= ME_CONTROL;
            if(kr->dwControlKeyState & ENHANCED_KEY)
                msg->lParam |= 0x01000000;
            return meTRUE;
        }
    }
    else if(ir.EventType == MOUSE_EVENT)
    {
        MOUSE_EVENT_RECORD *mr = &ir.Event.MouseEvent;
        static DWORD dwLastButtonState = 0;
        DWORD dwButtonState;
        
        dwButtonState = dwLastButtonState ^ mr->dwButtonState;
        
        /* Has the state of the mouse buttons changed? */
        if(dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)
        {
            if(dwLastButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)
                msg->message = WM_LBUTTONUP;
            else
                msg->message = WM_LBUTTONDOWN;
            msg->wParam |= MK_LBUTTON;
        }
        else if(dwButtonState & FROM_LEFT_2ND_BUTTON_PRESSED)
        {
            if(dwLastButtonState & FROM_LEFT_2ND_BUTTON_PRESSED)
                msg->message = WM_MBUTTONUP;
            else
                msg->message = WM_MBUTTONDOWN;
            msg->wParam |= MK_MBUTTON;
        }
        else if(dwButtonState & RIGHTMOST_BUTTON_PRESSED)
        {
            if(dwLastButtonState & RIGHTMOST_BUTTON_PRESSED)
                msg->message = WM_RBUTTONUP;
            else
                msg->message = WM_RBUTTONDOWN;
            msg->wParam |= MK_RBUTTON;
        }
#ifdef WM_MOUSEWHEEL
#ifdef MOUSE_WHEELED
        else if(mr->dwEventFlags & MOUSE_WHEELED)
        {
            msg->message = WM_MOUSEWHEEL;
            /* Haven't got NT5 so cant see it working and as usual the MS docs are crap
             * so I don't know whether this was a wheel up or down event */
            msg->wParam  = (1) ? 0x0:0x80000000;
        }
#endif
#endif
        /* Mouse moved? */
        else if(mr->dwEventFlags & MOUSE_MOVED)
            msg->message = WM_MOUSEMOVE;
        else
            msg->message = 0;
        
        /* Remember state for next time */
        dwLastButtonState = mr->dwButtonState;
        
        /* got anything to send? */
        if(msg->message != 0)
        {
            /* Build the wParam with mouse buttons and key states */
#ifdef WM_MOUSEWHEEL
            if(msg->message != WM_MOUSEWHEEL)
#endif
            {
                msg->wParam = 0;
                if(mr->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)
                    msg->wParam |= MK_LBUTTON;
                if(mr->dwButtonState & FROM_LEFT_2ND_BUTTON_PRESSED)
                    msg->wParam |= MK_MBUTTON;
                if(mr->dwButtonState & RIGHTMOST_BUTTON_PRESSED)
                    msg->wParam |= MK_RBUTTON;
            }
            if(mr->dwControlKeyState & (LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED))
                ttmodif |= ME_ALT;
            if(mr->dwControlKeyState & SHIFT_PRESSED)
                ttmodif |= ME_SHIFT;
            if(mr->dwControlKeyState & (LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED))
                ttmodif |= ME_CONTROL;
            /* Set up mouse position */
            msg->lParam = ((mr->dwMousePosition.Y) << 16) | mr->dwMousePosition.X;
            
            return meTRUE;
        }
    }
    else if(ir.EventType == WINDOW_BUFFER_SIZE_EVENT)
    {
        /* in true MS style the console resize stuff is a complete pain! The
         * user can change the buffer size and we get the change, but this is
         * no use to us as we want window size changes. Instead we have to get
         * the current window size and change the screen size to that.
         */
        CONSOLE_SCREEN_BUFFER_INFO Console;
        COORD size;
        
        GetConsoleScreenBufferInfo(hOutput, &Console);
        /* this should be the window size, not the buffer size
         * as this needs the scroll-bar to use */
        size.X = Console.srWindow.Right-Console.srWindow.Left+1;
        size.Y = Console.srWindow.Bottom-Console.srWindow.Top+1;
        
#if MEOPT_EXTENDED
        if((alarmState & meALARM_PIPED) == 0)
#endif
            SetConsoleScreenBufferSize(hOutput,size);
        meFrameChangeWidth(frameCur,size.X);
        meFrameChangeDepth(frameCur,size.Y);
        meFrameSetWindowSize(frameCur);
    }
    else if(ir.EventType == FOCUS_EVENT)
    {
        if(ir.Event.FocusEvent.bSetFocus)
        {
            /* Record the fact we have focus */
            frameCur->flags &= ~meFRAME_NOT_FOCUS;
            
            /* Kick of the blinker - as default value for cursorBlink
             * is 0 this will not happen until after the window is created */
            if((cursorState >= 0) && cursorBlink)
                TThandleBlink(2);
        }
        else
        {
            frameCur->flags |= meFRAME_NOT_FOCUS;
            if(cursorState >= 0)
            {
                /* because the cursor is a part of the solid cursor we must
                 * remove the old one first and then redraw
                 */
                if(blinkState)
                    meFrameHideCursor(frameCur);
                blinkState = 1;
                meFrameShowCursor(frameCur);
            }
        }
    }
    else
    {
        /* Menu or focus events, so do nowt */
        /* printf("Got unknown console input %d\n",ir.EventType);*/
    }
    return meFALSE;
}

#endif /* _ME_CONSOLE */

#ifdef _ME_WINDOW

/****************************************************************************
 *
 * WINDOWS SCREEN FUNCTIONS
 *
 ****************************************************************************/

/* WinSpecialChar; Draw a special character to the screen. x is the lefthand
 * edge of the character, y is the top of the character. The background is
 * assumed to be filled. A foreground pen is expected to be selected. */
static void
WinSpecialChar(HDC hdc, CharMetrics *cm, int x, int y, meUByte cc, COLORREF fcol)
{
    POINT points[4];                   /* Triangular points */
    int ii;
    
    /* Fill in the character */
    /* Note: LineTo does not draw the last pixel, i.e. a line x1,y1 -> x2,y2 will be drawn as x1,y1 -> x2-1,y2-1 */
    switch(cc)
    {
    case 0x01:          /* unicode tag - 3 byte encode [u] */
        MoveToEx(hdc,x,y,NULL);
        LineTo(hdc,x+cm->sizeX-1,y);
        LineTo(hdc,x+cm->sizeX-1,y+cm->sizeY-1);
        LineTo(hdc,x,y+cm->sizeY-1);
        LineTo(hdc,x,y);
        MoveToEx(hdc,x+2,y+4,NULL);
        LineTo(hdc,x+2,y+cm->sizeY-3);
        MoveToEx(hdc,x+3,y+cm->sizeY-3,NULL);
        LineTo(hdc,x+cm->sizeX-3,y+cm->sizeY-3);
        MoveToEx(hdc,x+cm->sizeX-3,y+4,NULL);
        LineTo(hdc,x+cm->sizeX-3,y+cm->sizeY-3);
        break;
        
    case 0x02:          /* unicode tag - 5 byte encode [U] */
        MoveToEx(hdc,x,y,NULL);
        LineTo(hdc,x+cm->sizeX-1,y);
        LineTo(hdc,x+cm->sizeX-1,y+cm->sizeY-1);
        LineTo(hdc,x,y+cm->sizeY-1);
        LineTo(hdc,x,y);
        MoveToEx(hdc,x+2,y+2,NULL);
        LineTo(hdc,x+2,y+cm->sizeY-3);
        MoveToEx(hdc,x+3,y+cm->sizeY-3,NULL);
        LineTo(hdc,x+cm->sizeX-3,y+cm->sizeY-3);
        MoveToEx(hdc,x+cm->sizeX-3,y+2,NULL);
        LineTo(hdc,x+cm->sizeX-3,y+cm->sizeY-3);
        break;
    
    case 0x03:          /* checkbox left side ([) */
        MoveToEx (hdc, x + cm->sizeX - 1, y + 1, NULL);
        LineTo(hdc, x + cm->sizeX - 2, y + 1);
        LineTo(hdc, x + cm->sizeX - 2, y + cm->sizeY - 2);
        LineTo(hdc, x + cm->sizeX, y + cm->sizeY - 2);
        break;
        
    case 0x04:          /* checkbox center not selected */
        MoveToEx (hdc, x, y + 1, NULL);
        LineTo(hdc, x + cm->sizeX, y + 1);
        MoveToEx (hdc, x, y + cm->sizeY - 2, NULL);
        LineTo(hdc, x + cm->sizeX, y + cm->sizeY - 2);
        break;
        
    case 0x05:          /* checkbox center selected */
        {
            HBRUSH obrush;
            HBRUSH fbrush;
            
            MoveToEx (hdc, x, y + 1, NULL);
            LineTo(hdc, x + cm->sizeX, y + 1);
            MoveToEx(hdc, x, y + cm->sizeY - 2, NULL);
            LineTo(hdc, x + cm->sizeX, y + cm->sizeY - 2);
            
            points[0].x = x;
            points[0].y = y + 3;
            points[1].x = x;
            points[1].y = y + cm->sizeY - 4;
            points[2].x = x + cm->sizeX - 1;
            points[2].y = y + cm->sizeY - 4;
            points[3].x = x + cm->sizeX - 1;
            points[3].y = y + 3;
            
            fbrush = CreateSolidBrush(fcol);
            obrush = (HBRUSH) SelectObject(hdc, fbrush);
            SetPolyFillMode(hdc, WINDING);
            Polygon(hdc, points, 4);
            SelectObject(hdc, obrush);
            DeleteObject(fbrush);
        }
        break;
        
    case 0x06:          /* checkbox right side (]) */
        MoveToEx(hdc, x, y + 1, NULL);
        LineTo(hdc, x + 1, y + 1);
        LineTo(hdc, x + 1, y + cm->sizeY - 2);
        LineTo(hdc, x - 1, y + cm->sizeY - 2);
        break;
        
    case 0x07:          /* meCHAR_UNDEF, undefined char - can be generated during X11 clipboard get or charset change <+> */
        MoveToEx(hdc, x, y + cm->midY, NULL);
        LineTo(hdc, x + cm->midX,  y);
        LineTo(hdc, x + cm->sizeX, y + cm->midY);
        LineTo(hdc, x + cm->midX,  y + cm->sizeY);
        LineTo(hdc, x, y + cm->midY);
        LineTo(hdc, x + cm->sizeX, y + cm->midY);
        MoveToEx(hdc, x + cm->midX,  y, NULL);
        LineTo(hdc, x + cm->midX,  y + cm->sizeY);
        break;

    case 0x08:          /* Line space '.' */
        MoveToEx(hdc, x + cm->midX, y + cm->midY, NULL);
        LineTo(hdc, x + cm->midX + 1, y + cm->midY);
        break;
        
    case 0x09:          /* Line & Poly / Tab -> */
        ii = cm->midY >> 1;
        MoveToEx(hdc, x, y + cm->midY, NULL);
        LineTo(hdc, x + cm->sizeX - cm->midX - 1, y + cm->midY);
        points[0].x = x + cm->sizeX - cm->midX - 1;
        points[0].y = y + cm->midY - ii;
        points[1].x = x + cm->sizeX - 2;
        points[1].y = y + cm->midY;
        points[2].x = x + cm->sizeX - cm->midX - 1;
        points[2].y = y + cm->midY + ii;
        goto makePoly;
        
    case 0x0a:          /* Line & Poly / CR <-| */
        ii = cm->midY >> 1;
        MoveToEx(hdc, x + cm->midX,      y + cm->midY, NULL);
        LineTo(hdc, x + cm->sizeX - 2, y + cm->midY);
        LineTo(hdc, x + cm->sizeX - 2, y + cm->midY - ii - 2);
        points[0].x = x + cm->midX;
        points[0].y = y + cm->midY - ii;
        points[1].x = x + cm->midX;
        points[1].y = y + cm->midY + ii;
        points[2].x = x + 1;
        points[2].y = y + cm->midY;
        goto makePoly;
        
    case 0x0b:          /* Line Drawing / Bottom right _| */
        MoveToEx(hdc, x, y + cm->midY, NULL);
        LineTo(hdc, x + cm->midX, y + cm->midY);
        LineTo(hdc, x + cm->midX, y - 1);
        break;
        
    case 0x0c:          /* Line Drawing / Top right */
        MoveToEx(hdc, x, y + cm->midY, NULL);
        LineTo(hdc, x + cm->midX, y + cm->midY);
        LineTo(hdc, x + cm->midX, y + cm->sizeY + 1);
        break;
        
    case 0x0d:          /* Line Drawing / Top left */
        MoveToEx(hdc, x + cm->midX, y + cm->sizeY + 1, NULL);
        LineTo(hdc, x + cm->midX, y + cm->midY);
        LineTo(hdc, x + cm->sizeX, y + cm->midY);
        break;
        
    case 0x0e:          /* Line Drawing / Bottom left |_ */
        MoveToEx(hdc, x + cm->midX, y, NULL);
        LineTo(hdc, x + cm->midX, y + cm->midY);
        LineTo(hdc, x + cm->sizeX, y + cm->midY);
        break;
        
    case 0x0f:          /* Line Drawing / Centre cross + */
        MoveToEx(hdc, x, y + cm->midY, NULL);
        LineTo(hdc, x + cm->sizeX, y + cm->midY);
        MoveToEx(hdc, x + cm->midX, y, NULL);
        LineTo(hdc, x + cm->midX, y + cm->sizeY + 1);
        break;
        
    case 0x10:          /* Cursor Arrows / Right */
        ii = cm->sizeX - 2;
        if(ii > cm->midY)
            ii = cm->midY;
        points[0].x = x + 1;
        points[0].y = y + cm->midY - ii;
        points[1].x = x + 1;
        points[1].y = y + cm->midY + ii;
        points[2].x = x + ii + 1;
        points[2].y = y + cm->midY;
        goto makePoly;
        
    case 0x11:          /* Cursor Arrows / Left */
        ii = cm->sizeX - 2;
        if(ii > cm->midY)
            ii = cm->midY;
        points[0].x = x + cm->sizeX - 2;
        points[0].y = y + cm->midY + ii;
        points[1].x = x + cm->sizeX - 2;
        points[1].y = y + cm->midY - ii;
        points[2].x = x + cm->sizeX - 2 - ii;
        points[2].y = y + cm->midY;
        goto makePoly;
        
    case 0x12:          /* Line Drawing / Horizontal line - */
        MoveToEx(hdc, x, y + cm->midY, NULL);
        LineTo(hdc, x + cm->sizeX, y + cm->midY);
        break;
        
    case 0x13:          /* cross box empty ([ ]) */
        MoveToEx(hdc, x, y + cm->midY - cm->midX, NULL);
        LineTo(hdc, x + cm->sizeX - 1, y + cm->midY - cm->midX);
        LineTo(hdc, x + cm->sizeX - 1, y + cm->midY + cm->sizeX - cm->midX);
        LineTo(hdc, x, y + cm->midY + cm->sizeX - cm->midX);
        LineTo(hdc, x, y + cm->midY - cm->midX);
        break;
        
    case 0x14:          /* cross box ([X]) */
        MoveToEx(hdc, x, y + cm->midY - cm->midX, NULL);
        LineTo(hdc, x + cm->sizeX - 1, y + cm->midY - cm->midX);
        LineTo(hdc, x + cm->sizeX - 1, y + cm->midY + cm->sizeX - cm->midX);
        LineTo(hdc, x, y + cm->midY + cm->sizeX - cm->midX);
        LineTo(hdc, x, y + cm->midY - cm->midX);
        LineTo(hdc, x + cm->sizeX - 1, y + cm->midY + cm->sizeX - cm->midX);
        MoveToEx(hdc, x + cm->sizeX - 1, y + cm->midY - cm->midX,NULL);
        LineTo(hdc, x, y + cm->midY + cm->sizeX - cm->midX);
        break;
        
    case 0x15:          /* Line Drawing / Left Tee |- */
        MoveToEx(hdc, x + cm->midX, y, NULL);
        LineTo(hdc, x + cm->midX, y + cm->sizeY + 1);
        MoveToEx(hdc, x + cm->midX, y + cm->midY, NULL);
        LineTo(hdc, x + cm->sizeX, y + cm->midY);
        break;
        
    case 0x16:          /* Line Drawing / Right Tee -| */
        MoveToEx(hdc, x + cm->midX, y, NULL);
        LineTo(hdc, x + cm->midX, y + cm->sizeY + 1);
        MoveToEx(hdc, x, y + cm->midY, NULL);
        LineTo(hdc, x + cm->midX, y + cm->midY);
        break;
        
    case 0x17:          /* Line Drawing / Bottom Tee _|_ */
        MoveToEx (hdc, x,  y + cm->midY, NULL);
        LineTo(hdc, x + cm->sizeX, y + cm->midY);
        MoveToEx (hdc, x + cm->midX, y, NULL);
        LineTo(hdc, x + cm->midX, y + cm->midY);
        break;
        
    case 0x18:          /* Line Drawing / Top Tee -|- */
        MoveToEx(hdc, x,  y + cm->midY, NULL);
        LineTo(hdc, x + cm->sizeX, y + cm->midY);
        MoveToEx(hdc, x + cm->midX, y + cm->sizeY, NULL);
        LineTo(hdc, x + cm->midX, y + cm->midY);
        break;
        
    case 0x19:          /* Line Drawing / Vertical Line | */
        MoveToEx(hdc, x + cm->midX, y, NULL);
        LineTo(hdc, x + cm->midX, y + cm->sizeY + 1);
        break;
        
    case 0x1a:          /* Line Drawing / Bottom right _| with resize */
        MoveToEx(hdc, x, y + cm->midY, NULL);
        LineTo(hdc, x + cm->midX, y + cm->midY);
        LineTo(hdc, x + cm->midX, y - 1);
        
        MoveToEx(hdc, x, y + cm->sizeY, NULL);
        LineTo(hdc, x + cm->sizeX, y + cm->sizeY - cm->sizeX);
        MoveToEx(hdc, x + 2, y + cm->sizeY, NULL);
        LineTo(hdc, x + cm->sizeX, y + cm->sizeY - cm->sizeX + 2);
        MoveToEx(hdc, x + 4, y + cm->sizeY, NULL);
        LineTo(hdc, x + cm->sizeX, y + cm->sizeY - cm->sizeX + 4);
        break;
        
    case 0x1b:          /* Scroll box - vertical */
        for(ii = (y+1) & ~1; ii < y + cm->sizeY; ii += 2)
        {
            MoveToEx (hdc, x, ii, NULL);
            LineTo(hdc, x + cm->sizeX, ii);
        }
        break;
        
        /* 0x1c is difficult to use as its the meCHAR_LEADER char - only use if value not used in variables such as $window-chars */ 
        
    case 0x1d:          /* Scroll box - horizontal */
        for(ii = (x+1) & ~1; ii < x + cm->sizeX; ii += 2)
        {
            MoveToEx(hdc, ii, y, NULL);
            LineTo(hdc, ii, y + cm->sizeY + 1);
        }
        break;
        
    case 0x1e:          /* Cursor Arrows / Up */
        {
            int hh;
            if((hh = cm->sizeY-cm->sizeX-2) < 0)
                hh = 0;
            
            points[0].x = x + cm->midX;
            points[0].y = y + hh;
            points[1].x = x + cm->sizeX - 1;
            points[1].y = y + cm->sizeY - 2;
            points[2].x = x;
            points[2].y = y + cm->sizeY - 2;
            goto makePoly;
        }
        
    case 0x1f:          /* Cursor Arrows / Down */
        {
            int hh;
            if((hh = cm->sizeX+1) >= cm->sizeY)
                hh = cm->sizeY-1;
            
            points[0].x = x;
            points[0].y = y + 1;
            points[1].x = x + cm->sizeX - 1;
            points[1].y = y + 1;
            points[2].x = x + cm->midX;
            points[2].y = y + hh;
        }
        /* Construct the polygon */
makePoly:
        {
            HBRUSH obrush;
            HBRUSH fbrush;
            
            fbrush = CreateSolidBrush(fcol);
            obrush = (HBRUSH) SelectObject(hdc, fbrush);
            SetPolyFillMode(hdc, WINDING);
            Polygon(hdc, points, 3);
            SelectObject(hdc, obrush);
            DeleteObject(fbrush);
        }
        break;
    }
}

/* meFrameDrawCursor; Draw the cursor on the screen. We use the information from
 * the frame store to determine what character to render */
void
meFrameDrawCursor(meFrame *frame, HDC hdc)
{
    RECT rline;                         /* Rectangle of line */
    int clientRow;                      /* Text region row start - client units */
    int clientCol;                      /* Text region col start - client units */
    meFrameLine *flp;                   /* Frame store line pointer */
    meStyle style;                      /* Current style */
    meUByte cc;                         /* Current char  */
    
    /* Set up the drawing borders. */
    rline.top    = eCellMetrics.cellRowPos[frame->cursorRow];
    rline.bottom = eCellMetrics.cellRowPos[frame->cursorRow+1];
    rline.left   = eCellMetrics.cellColPos[frame->cursorColumn];
    rline.right  = eCellMetrics.cellColPos[frame->cursorColumn+1];
    /* Set up text start position */
    clientRow = eCellMetrics.cellRowPos[frame->cursorRow];
    clientCol = eCellMetrics.cellColPos[frame->cursorColumn];
    
    flp = frame->store + frame->cursorRow;
    cc = flp->text[frame->cursorColumn];                          /* Get char under cursor */
    style = meSchemeGetStyle(flp->scheme[frame->cursorColumn]);   /* Get style under cursor */
    
    /* Check the current character set and select the special font or convert
     * the character for the current font. Bring the appropriate font into the
     * device context. */
    if(((cc & 0xe0) == 0) && (meSystemCfg & meSYSTEM_FONTFIX))
    {
	HBRUSH brush;
	HPEN pen;                   /* Foreground pen */
	HPEN oldpen;                /* first pen */
	COLORREF cref;              /* Background color */
        
	if(frame->flags & meFRAME_NOT_FOCUS)
	    cref = eCellMetrics.pInfo.cPal[meStyleGetBColor(style)].cpixel;
	else
	    cref = eCellMetrics.pInfo.cPal[cursorColor].cpixel;
        
	/* Fill in the background */
	brush = CreateSolidBrush(cref);
	FillRect(hdc, &rline, brush);
	DeleteObject(brush);
        
	/* Create a pen to draw the character */
	pen = CreatePen(PS_SOLID, 0, eCellMetrics.pInfo.cPal[meStyleGetFColor(style)].cpixel);
	oldpen = SelectObject(hdc, pen);
        
	/* This is a special character, render the character to the
	 * screen. We need to create a pen to handle the object */
	WinSpecialChar(hdc, &eCellMetrics.cell, rline.left, rline.top, cc,
                       eCellMetrics.pInfo.cPal[meStyleGetFColor(style)].cpixel);
        
	if(frame->flags & meFRAME_NOT_FOCUS)
	{
	    /* If there is no focus then put a rectangle around the
	     * object; bring in the backgound color */
	    pen = CreatePen(PS_SOLID, 0, eCellMetrics.pInfo.cPal[cursorColor].cpixel);
	    pen = SelectObject(hdc, pen);
	    DeleteObject(pen);
            
	    /* Draw the rectangle */
	    MoveToEx(hdc, rline.top - 1, rline.left - 1, NULL);
	    LineTo(hdc, rline.top + eCellMetrics.cell.sizeY + 1, rline.left - 1);
	    LineTo(hdc, rline.top + eCellMetrics.cell.sizeY + 1, rline.left + eCellMetrics.cell.sizeX + 1);
	    LineTo(hdc, rline.top - 1, rline.left + eCellMetrics.cell.sizeX + 1);
	    LineTo(hdc, rline.top - 1, rline.left - 1);
	}
	pen = SelectObject(hdc, oldpen);
	DeleteObject(pen);
        
	/* Finished !! */
	return;
    }
    
    /* Set up the font */
    SetTextColor(hdc, eCellMetrics.pInfo.cPal[meStyleGetBColor(style)].cpixel);
    SetBkColor(hdc, eCellMetrics.pInfo.cPal[cursorColor].cpixel);
    SelectObject(hdc, eCellMetrics.fontdef[meStyleGetFont(style)]);
    
    /* Output the character */
    ExtTextOut(hdc,
               clientCol,                /* Text start position */
               clientRow,
               ETO_OPAQUE|ETO_CLIPPED,   /* Fill background */
               &rline,                   /* Background area */
               (LPCSTR) &cc,             /* Text string */
               1,                        /* Length of string */
               eCellMetrics.cellSpacing);
    
    if(frame->flags & meFRAME_NOT_FOCUS)
    {
        /* on top draw the normal character but smaller, this creates the
         * rectangle effect */
        SetTextColor(hdc, eCellMetrics.pInfo.cPal[meStyleGetFColor(style)].cpixel);
        SetBkColor(hdc, eCellMetrics.pInfo.cPal[meStyleGetBColor(style)].cpixel);
        
        rline.top++;
        rline.bottom--;
        rline.left++;
        rline.right--;
        
        ExtTextOut(hdc,
                   clientCol,      /* Text start position */
                   clientRow,
                   ETO_CLIPPED,    /* Clip char to smaller rectangle */
                   &rline,         /* Background area */
                   (LPCSTR) &cc,   /* Text string */
                   1,              /* Length of string */
                   eCellMetrics.cellSpacing);
    }
}

/* WinLoadFont
 * load the specified font variant, i.e. italic/bold etc
 */
void
WinLoadFont(int font)
{
    if(eCellMetrics.fontdef[font] == NULL)
    {
        /* Construct the new font from the logical font information */
        if(font & meFONT_BOLD)
            ttlogfont.lfWeight = FW_BOLD;
        else if(font & meFONT_LIGHT)
            ttlogfont.lfWeight = FW_EXTRALIGHT;
        else
            ttlogfont.lfWeight = FW_NORMAL;
        ttlogfont.lfItalic    = (font & meFONT_ITALIC)    ? meTRUE:meFALSE;
        ttlogfont.lfUnderline = (font & meFONT_UNDERLINE) ? meTRUE:meFALSE;
        
        /* Create the font - use the existing font if it exists */
        if((eCellMetrics.fontdef[font] = CreateFontIndirect(&ttlogfont)) == NULL)
            eCellMetrics.fontdef[font] = eCellMetrics.fontdef[0];
    }
}

/*
 * WinPaint Paint to the screen the updated region of text from the virtual
 * screen store. Note that we are only painting the regions of the screen that
 * have changed.
 *
 * ONLY call this function from a WM_PAINT message. If it is called from
 * elsewhere then hide and show the caret.
 *
 * Jon: 00/03/17; Now that we have moved to rendering italic characters then
 * we render right to left. Windows italic characters are wider than their
 * fixed font counter parts and typically spill into the next character
 * position by 1 or 2 pixels (unlike X-Windows Oblique font). This is very
 * noticable for charactes like d/W/X where the top right part of the
 * character is clipped. The side effect is that part of the next character
 * might be over-written. In most cases this artifact is far less noticable
 * than truncating the character.
 */

static void
meFrameDraw(meFrame *frame)
{
    PAINTSTRUCT ps;                     /* Paint structure */
    RECT rline;                         /* Rectangle of line */
    int col;                            /* Current column position */
    meStyle schm;                       /* Current style */
    meColor font;                       /* Text font type */
    meColor fcol;                       /* Foreground colour */
    meColor bcol;                       /* Background colour */
    meColor pfcol;                      /* Pen foreground colour */
    meColor pbcol;                      /* Pen background colour */
    int srow;                           /* Start row */
    int erow;                           /* End row */
    int scol;                           /* Start column */
    int ecol;                           /* End column */
    int clientRow;                      /* Text region row start - client units */
    meFrameData *fd;                    /* Pointer to Frame data */
    meFrameLine *flp;                   /* Frame store line pointer */
    int drawCursor;                     /* draw cursor flag */
    HBRUSH bbrush = NULL;               /* Background brush */
    HPEN pen;                           /* Foreground pen */
    HPEN oldpen = NULL;                 /* fist pen */
    UINT etoOpt;                        /* ExtTextOut options */

    fd = meFrameGetWinData(frame);
#define DEBUG_BG 0
#if DEBUG_BG
    static meUByte bgcol=0;
    if(++bgcol >= noColors)
        bgcol=0;
#endif
    BeginPaint(meFrameDataGetWinHandle(fd), &ps);
    
    /* I'm not sure if I need these here or not ?? */
    SetMapMode(ps.hdc, MM_TEXT);        /* Text mode */
    SetMapperFlags(ps.hdc, 1);          /* Allow interpolation */
    
    /* quick check to see if theres anything to do */
    if((ps.rcPaint.top == ps.rcPaint.bottom) ||
        (ps.rcPaint.left == ps.rcPaint.right))
    {
        EndPaint(meFrameDataGetWinHandle(fd), &ps);
        return;
    }
    
    if(eCellMetrics.pInfo.hPal != NULL)
    {
        SelectPalette(ps.hdc, eCellMetrics.pInfo.hPal, meFALSE);
        RealizePalette(ps.hdc);
    }
    
    /* Initialise some variables now. */
    schm = meSCHEME_INVALID;
    font = meCOLOR_INVALID;
    fcol = meCOLOR_INVALID;
    bcol = meCOLOR_INVALID;
    pfcol = meCOLOR_INVALID;
    pbcol = meCOLOR_INVALID;
    
    /* Convert the paint region from client coordinates to screen coordinates.
     * Note that the end row/column is rounded down at the sub-pixel level. */
    srow = clientToRow(ps.rcPaint.top);
    erow = clientToRow(ps.rcPaint.bottom + eCellMetrics.cell.sizeY - 1);
    if(erow > frame->depth)
        erow = frame->depth;
    
    scol = clientToCol(ps.rcPaint.left);
    
    /* As we  draw  in  character  space  then  make  sure we are  within  the
     * character  canvas, the only  special case is the left hand edge when we
     * are running with an offset. */
    if(ps.rcPaint.right > 0)
        ecol = clientToCol(ps.rcPaint.right + eCellMetrics.cell.sizeX - 1);
    else
        ecol = 1;                       /* Render 1st column */
    if(ecol > frame->width)
        ecol = frame->width;
    
    /* Redraw the cursor if we have zapped it */
    if((cursorState >= 0) && blinkState)
        drawCursor = ((srow <= frame->cursorRow) && (erow >= frame->cursorRow) &&
                      (scol <= frame->cursorColumn) && (ecol > frame->cursorColumn));
    else
        drawCursor = 0;
    
    /* Process each row in turn until we reach the end of the line */
    for(flp = frame->store + srow; srow <= erow; srow++, flp++)
    {
        meScheme *fschm;
        meUByte *tbp, cc;
        meUByte *ftext;
        int length;
        int tcol, spFlag;
        
        /* Determine the boundaries we are painting around */
        if(meFrameDataGetWinPaintAll(fd))
            col = ecol;
        else if((col = meFrameDataGetWinPaintEndCol(fd)[srow]) > 0)
            scol = meFrameDataGetWinPaintStartCol(fd)[srow];
        else
            continue;
        
        /* Reset the paint extremities - we set these for optimisation
         * purposes. */
        meFrameDataGetWinPaintStartCol(fd)[srow] = frame->width;
        meFrameDataGetWinPaintEndCol(fd)[srow] = 0;
        tbp = eCellMetrics.cellColTmpPos;
        
        /* Set up the drawing borders. */
        rline.top    = eCellMetrics.cellRowPos[srow];
        rline.bottom = eCellMetrics.cellRowPos[srow+1];
        rline.right  = eCellMetrics.cellColPos[col];
        
        /* Set up text start position */
        clientRow = eCellMetrics.cellRowPos[srow];
        
        /* As we render right to left then we start with the end character - 1 */
        col--;
        
        ftext = flp->text;              /* Point to appropriate text block */
        fschm = flp->scheme + col;      /* Point to appropriate colour block  */
        
        for(;;)
        {
            if(schm != *fschm)
            {
                /* Set up the colour change */
                meStyle style;
                meUByte ff;
                
                schm = *fschm;
                style = meSchemeGetStyle(schm);
                ff = (meUByte) meStyleGetFColor(style);
                if(fcol != ff)
                {
                    fcol = ff;
                    SetTextColor(ps.hdc, eCellMetrics.pInfo.cPal[fcol].cpixel);
                }
                ff = (meUByte) meStyleGetBColor(style);
#if DEBUG_BG
                ff = bgcol;
#endif
                if(bcol != ff)
                {
                    bcol = ff;
                    SetBkColor(ps.hdc, eCellMetrics.pInfo.cPal[bcol].cpixel);
                }
                
                ff = (meUByte) meStyleGetFont(style);
                /* If there is a modification on the font then apply it now.
                 * Note that the following looks a little cumbersome and
                 * unecessary, however the compiler will reduce the first pair
                 * of expressions into a single test so we only enter the
                 * conditional block when we need to. Both of the following
                 * are applied at the end of the line and occur infrequently. */
                if(meSchemeTestNoFont(schm))
                {
                    ff &= ~(meFONT_BOLD|meFONT_ITALIC|meFONT_UNDERLINE);
                }
                
                if(font != ff)
                {
                    font = ff;
                    if(eCellMetrics.fontdef[font] == NULL)
                        WinLoadFont(font);
                    /* Change font */
                    SelectObject(ps.hdc, eCellMetrics.fontdef[font]);
                    /* if this is a different font it could overhang the next char, common if italic (which is why we draw right to left)
                     * in which case we must not clip. However, if this is the base font then clip the draw to the rectangle to avoid pixel
                     * overruns caused by windows ClearType */
                    etoOpt = (font && (eCellMetrics.fontdef[font] != eCellMetrics.fontdef[0])) ? ETO_OPAQUE : ETO_OPAQUE|ETO_CLIPPED;
                        
                }
            }
            
            /* how many characters can we draw until we get a color change or
             * reach the end */
            tcol = col;
            spFlag = 0;
            do
            {
                cc = ftext[col];
                if((meSystemCfg & meSYSTEM_FONTFIX) && ((cc & 0xe0) == 0))
                {
                    spFlag++;
                    cc = ' ';
                }
                tbp[col] = cc;
            } while((--col >= scol) && (*--fschm == schm));
            
            /* Output the current text item. Set up the current left margin
             * and determine the length of text that we have to output. */
            length = tcol - col;
            col++;                      /* Move to current position */
            rline.left = eCellMetrics.cellColPos[col];
            
            /* Output regular text */
            ExtTextOut(ps.hdc,
                       eCellMetrics.cellColPos[col], /* Text start position */
                       clientRow,
                       etoOpt,         /* Fill background & may be clip */
                       &rline,         /* Background area */
                       (LPCSTR)tbp+col,/* Text string */
                       length,         /* Length of string */
                       eCellMetrics.cellSpacing);
            col--;                     /* Restore position */
            
            /* Special characters */
            if(spFlag != 0)
            {
                /* Correct the pen colours if required. We do this here
                 * because we expect less special characters. We do not want
                 * to create and delete new pens unless we really are going to
                 * use them. */
                if(pfcol != fcol)
                {
                    HPEN p;
                    pfcol = fcol;
                    /* Set up for line drawing */
                    pen = CreatePen(PS_SOLID, 0, eCellMetrics.pInfo.cPal[fcol].cpixel);
                    p = SelectObject(ps.hdc, pen);
                    if(oldpen == NULL)
                        oldpen = p;
                    else
                        DeleteObject(p);
                }
                if(pbcol != bcol)
                {
                    pbcol = bcol;
                    /* Set up for line drawing */
                    if(bbrush != NULL)
                        DeleteObject(bbrush);
                    bbrush = CreateSolidBrush(eCellMetrics.pInfo.cPal[bcol].cpixel);
                }
                /* Render the special characters */
                do
                {
                    while(((cc=ftext[tcol]) & 0xe0) != 0)
                        tcol--;
                    
                    WinSpecialChar(ps.hdc,&eCellMetrics.cell,eCellMetrics.cellColPos[tcol],
                                   rline.top,ftext[tcol],eCellMetrics.pInfo.cPal[fcol].cpixel);
                    tcol--;
                }
                while(--spFlag > 0);
            }
            
            /* are we at the end?? */
            if(scol > col)
                break;
            
            rline.right = rline.left;
        }
    }
    if(drawCursor)
        meFrameDrawCursor(frame,ps.hdc);
    
    meFrameDataGetWinPaintAll(fd) = 0;
    
    /* Relinquish the resources */
    if(oldpen != NULL)
    {
        oldpen = SelectObject(ps.hdc,oldpen);
        DeleteObject(oldpen);
    }
    
    if(bbrush)
        DeleteObject(bbrush);
    
    EndPaint(meFrameDataGetWinHandle(fd),&ps);
}

LRESULT CALLBACK
WinQuitExit(HWND hwndDlg,     /* window handle of dialog box     */
            UINT message,     /* type of message                 */
            WPARAM wParam,    /* message-specific information    */
            LPARAM lParam)    /* message-specific information    */
{
    switch(message)
    {
    case WM_INITDIALOG:  /* message: initialize dialog box  */
        return meTRUE;
        
    case WM_COMMAND:     /* message: received a command */
        /* User pressed "Cancel" button--stop print job. */
        if((LOWORD(wParam)) == IDOK)
            EndDialog(hwndDlg,meTRUE);
        else if((LOWORD(wParam)) == IDCANCEL)
            EndDialog(hwndDlg, meFALSE);
        else
            return meFALSE;
        return meTRUE;
    default:
        return meFALSE;     /* didn't process a message   */
        
    }
}

static void
meModifierUpdate(void)
{
    BYTE keyBuf[256];          /* Keyboard buffer */
    
    GetKeyboardState(keyBuf);
    
    ttmodif = 0;
    if(keyBuf[VK_SHIFT] & 0x80)
        ttmodif |= ME_SHIFT;
    if(keyBuf[VK_MENU] & 0x80)
        ttmodif |= ME_ALT;
    if(keyBuf[VK_CONTROL] & 0x80)
        ttmodif |= ME_CONTROL;
}

#endif /* _ME_WINDOW */

/* returns 0 for not translation, -1 for dead key, -2 for key>255 else char */
int
WinKeyTranslate(UINT key, meUShort modif)
{
    BYTE keyBuf[256];
    WCHAR buf[5];
    int rr;
    memset(keyBuf,0,256);
    if(modif & ME_SHIFT)
        keyBuf[VK_SHIFT] = 0xff;
    if(modif & ME_ALT)
        keyBuf[VK_MENU] = 0xff;
    if(modif & ME_CONTROL)
        keyBuf[VK_CONTROL] = 0xff;
    
    if((rr = ToUnicode(key,0,keyBuf,buf,5,4)) < 0)
        rr = -1;
    else if((rr > 0) && ((rr = buf[0]) > 0x0ff))
        rr = -2;
    return rr;
}


/*
 * WinKillToClipboard
 * Copy the data into the clipboard from the kill buffer.
 */
static HANDLE
WinKillToClipboard(void)
{
    HANDLE hmem;                        /* Windows global memory handle */
    WCHAR uc, *bufp;                    /* Windows global memory pointer */
    meKillNode *killp;                  /* Pointer to the kill data */
    meUByte cc;                         /* Local character pointer */
    meUByte *dd;                        /* Pointer to the kill data */
    int killSize = 0;                   /* Number of bytes in kill buffer */
    int noEmpty;
    
    /* Determine the size of the data in the kill buffer.
     * Make sure that \r\n are appended to the end of each line. */
    if(klhead != NULL)
    {
        for(killp = klhead->kill; killp != NULL; killp = killp->next)
        {
            for(dd = killp->data; (cc = *dd++) != '\0'; killSize++)
                if(cc == meCHAR_NL)
                    killSize++; /* Add 1 for the '\r' */
        }
    }
    if((noEmpty = ((meSystemCfg & meSYSTEM_NOEMPTYANK) && (killSize == 0))) != 0)
        killSize++;
    
    /* Create global buffer for the data */
    if((hmem = GlobalAlloc(GMEM_MOVEABLE,(killSize + 1)*sizeof(WCHAR))) != NULL)
    {
        bufp = GlobalLock(hmem);
        
        /* Copy the data into the buffer */
        if(noEmpty)
            *bufp++ = ' ';
        else if(klhead != NULL)
        {
            for(killp = klhead->kill; killp != NULL; killp = killp->next)
            {
                dd = killp->data;
                while((uc = (WCHAR) *dd++))
                {
                    /* Convert the end of line to CR/LF */
                    if(uc == meCHAR_NL)
                        *bufp++ = '\r';
#if MEOPT_EXTENDED
                    /* Convert any special characters */
                    else if(uc >= 0x80)
                        uc = charToUnicode[uc-128];
                    else if((meSystemCfg & meSYSTEM_FONTFIX) && (uc < TTSPECCHARS))
                        uc = ttSpeChars[uc];
#endif
                    /* Copy in the character */
                    *bufp++ = uc;
                }
            }
        }
        
        /* NULL terminate the buffer and unlock */
        *bufp = '\0';                      /* Null terminate string */
        GlobalUnlock(hmem);                /* Unlock the memory region */
    }
    else
        hmem = GlobalAlloc(GHND,1);
    
    return hmem;
}

/*
 * TTsetClipboard
 * Make the information available to other applications.
 * Do not copy the data (unless cpData != 0) simply mark the clipboard signalling that data is available.
 */
void
TTsetClipboard(int cpData)
{
    HANDLE hmem=NULL;
    /* We aquire the clipboard and flush it under the following conditions;
     * "We do NOT own it" or "Clipboard is stale", we have a kill buffer and we
     * have current windows focus. The clipboard becomes stale
     * when we own it but another application has aquired our clipboard data.
     * In this case we need to reset the clipboard so that the application may
     * aquire our next data block that has changed. */
    if((!(clipState & CLIP_OWNER) || (clipState & CLIP_STALE) || cpData) &&
       !(clipState & CLIP_DISABLED) && !(meSystemCfg & meSYSTEM_NOCLIPBRD) &&
       ((kbdmode != mePLAY) || cpData) && !(frameCur->flags & meFRAME_NOT_FOCUS) &&
       ((cpData == 0) || ((hmem = WinKillToClipboard()) != NULL)) && OpenClipboard(baseHwnd))
    {
        if(clipState & CLIP_OWNER)
            /* if we are currently the owner of the clipboard, the call to EmptyClipboard
             * will generate a WM_DESTROYCLIPBOARD to this window, ignore it! */
            clipState |= CLIP_IGNORE_DC;
        EmptyClipboard();
        SetClipboardData(CF_UNICODETEXT,hmem);
        CloseClipboard();
        clipState |= CLIP_OWNER;
        clipState &= ~CLIP_STALE;
    }
}

/*
 * TTgetClipboard.
 * Pop the contents of the clipboard into the kill buffer ready for
 * a yank. */
void
TTgetClipboard(int flag)
{
    HANDLE hmem;                          /* Windows clipboard memory handle */
    
    /* Check the standard clipboard status, if owner or not got focus or it has
     * been disabled then there's nothing to do */
    if((clipState & (CLIP_OWNER|CLIP_DISABLED)) || (meSystemCfg & meSYSTEM_NOCLIPBRD) || 
       (((flag & 1) == 0) && ((kbdmode == mePLAY) || (frameCur->flags & meFRAME_NOT_FOCUS))) ||
       !OpenClipboard(baseHwnd))
        return;
    
    /* Get the data from the clipboard */
    if((hmem = GetClipboardData(CF_UNICODETEXT)) != NULL)
    {
        meKillNode *kn;
        WCHAR *cb, *ss, uc;
        meUByte *dd;
        int ll, len;
        
        cb = GlobalLock(hmem);        /* Lock global buffer */
        ss = cb;
        len = ll = 0;
        while((uc = *ss++) != '\0')
        {
            if(uc == '\n')
            {
                /* If line is longer than 32KB it must be split, the (ll>>15) adds the extra chars required */
                len += ll+(ll>>15)+1;
                ll = 0;
            }
#if MEOPT_EXTENDED
            else if((uc >= 0x80) && (flag & 2))
                ll += (uc & 0x0f800) ? 3:2;
#endif
            else if((uc != '\r') || (*ss != '\n'))
                ll++;
        }
        len += ll+(ll>>15);
        
        if((kn = (meKillNode *) meMalloc(sizeof(meKillNode)+len)) == NULL)
            goto do_unlock;             /* Failed memory allocation */
        
        ss = cb;                        /* Start of clipboard data */
        dd = kn->data;                  /* Start of the kill buffer */
        ll = 0;
        while((uc = *ss++) != '\0')
        {
            if((uc == '\r') && (*ss == '\n'))
            {
                ss++;
                *dd++ = '\n';
                ll = 0;
            }
            else if(uc == '\n')
            {
                *dd++ = '\n';
                ll = 0;
            }
            else
            {
                if(ll == meLINE_ELEN_MAX)
                {
                    *dd++ = '\n';
                    ll = 0;
                }
#if MEOPT_EXTENDED
                if(uc >= 0x80)
                {
                    if(flag & 2)
                    {
                        if(uc & 0x0f800)
                        {
                            *dd++ = 0xe0 | (uc >> 12);
                            *dd++ = 0x80 | ((uc >> 6) & 0x3f);
                        }
                        else
                            *dd++ = 0xc0 | (uc >> 6);
                        *dd++ = 0x80 | (uc & 0x3f);
                    }
                    else if((uc > 0xff) || (charToUnicode[uc-128] != uc))
                    {
                        int ii = 127;
                        while((charToUnicode[ii] != uc) && (--ii >= 0))
                            ;
                        if(ii >= 0)
                            *dd++ = (meUByte) ii+128;
                        else
                            *dd++ = meCHAR_UNDEF;
                    }
                    else
                        *dd++ = (meUByte) uc;
                }
                else
#endif
                    *dd++ = (meUByte) uc;
                ll++;
            }
        }
        *dd = '\0';
        
        /* Make sure that it is not the same as the current save buffer head */
        if((len == 0) || (klhead == NULL) || (klhead->kill == NULL) ||
           (klhead->kill->next != NULL) || (meStrcmp(klhead->kill->data,kn->data)))
        {
            /* Always new kill, don't want to glue them together */
            killInit(0);
            killInsertNode(kn);
        }
        else
            meFree(kn);                /* Relinquish temp buffer */
do_unlock:
        GlobalUnlock(cb);              /* Unlock clipboard data */
    }
    CloseClipboard();
}

#if MEOPT_IPIPES
#ifdef USE_BEGINTHREAD
void
childActiveThread(void *lpParam)
#else
DWORD WINAPI
childActiveThread(LPVOID lpParam)
#endif
{
    meIPipe *ipipe=(meIPipe *) lpParam;
    DWORD bytesRead;
    meUByte buff[4];
    
    do {
        /* wait for child process activity */
        if((ReadFile(ipipe->rfd,buff,1,&bytesRead,NULL) != 0) && (bytesRead > 0))
        {
            ipipe->nextChar = buff[0];
            ipipe->flag |= meIPIPE_NEXT_CHAR;
        }
        else
            ipipe->flag |= meIPIPE_CHILD_EXIT;
        
        /* flag the child is active! */
        if(!SetEvent(ipipe->childActive))
            break;
        
        /* if there was a problem, the pipe is dead - exit */
        if(ipipe->flag & meIPIPE_CHILD_EXIT)
            break;
        
        /* wait for the main thread to read all available output and
         * flag for us to start waiting again */
    } while((WaitForSingleObject(ipipe->threadContinue,INFINITE) == WAIT_OBJECT_0) &&
            !(ipipe->flag & meIPIPE_CHILD_EXIT));
#ifndef USE_BEGINTHREAD
    return 0;
#endif
}

#endif

#if MEOPT_SPAWN
/*
 * WinLaunchProgram
 * Launches an external program using the DOS shell.
 *
 * Returns meTRUE if all went well, meFALSE if wait cancelled and FAILED if
 * failed to launch.
 *
 * Cmd is the command string to launch.
 *
 * DOSApp is meTRUE if the external program is a DOS program to be run
 * under a DOS shell. If DOSApp is meFALSE, the program is launched
 * directly as a Windows application. In that case, the InFile parameter
 * is ignored, and the value of the OutFile parameter is used only to
 * determine if the program should be monitored. the text of the string
 * referenced by OutFile is irrelevant.
 *
 * InFile is the name of the file to pipe into stdin (if NULL, nothing
 * is piped in)
 *
 * OutFile is the name of the file where stdout is expected to be
 * redirected. If it is NULL or an empty string, stdout is not redirected
 *
 * If Outfile is NULL, LaunchPrg returns immediately after starting the
 * DOS box.
 *
 * If OutFile is not NULL, the external program is monitored.
 * LaunchPrg returns only when the external program has terminated or
 * the user has cancelled the wait (in which case LaunchPrg returns
 * meFALSE).
 *
 * NOTE: Jon 14/05/97:
 *
 * Encountering problems with utilies such as 'grep' locking up the system,
 * this occurs when the command line has been goofed up by the user (me in
 * this case) and forgot to add some files as arguments. grep then takes it's
 * input from 'stdin'. If stdin is NULL (as was the case) then grep hangs
 * since there is no 'stdin', even worse we seem to kill off a windows .dll
 * from which we can never recover and grep never works again.
 *
 * To get round this problem create a empty file as the stdin input file,
 * utilities such as grep then have a source of stdin, which will immediatly
 * terminate safely. The stdin file is deleted once the command has been
 * executed.
 */
int
WinLaunchProgram(meUByte *cmd, int flags, meUByte *inFile, meUByte *outFile,
#if MEOPT_IPIPES
                  meIPipe *ipipe,
#endif
                  meInt *sysRet)
{
    static meUByte *compSpecName=NULL;
    static int compSpecLen;
    PROCESS_INFORMATION mePInfo;
    STARTUPINFO meSuInfo;
    meUByte  *cmdLine=NULL, *cp;                /* Buffer for the command line */
    meUByte  dummyInFile[meBUF_SIZE_MAX];       /* Dummy input file */
    meUByte  pipeOutFile[meBUF_SIZE_MAX];       /* Pipe output file */
    int    status;
#ifdef _WIN32s
    static int pipeStderr=0;                    /* Remember the stderr state */
#else
    HANDLE inHdlTmp, inHdl, outHdlTmp, outHdl, dumHdl;
#endif
    
    /* Get the comspec */
    if(((flags & LAUNCH_NOCOMSPEC) == 0) && (compSpecName == NULL))
    {
        if(((compSpecName = meGetenv("COMSPEC")) == NULL) ||
           ((compSpecName = meStrdup(compSpecName)) == NULL))
        {
#ifndef _WIN32_WINNT
            /* If no COMSPEC setup the default */
            if(platformId != VER_PLATFORM_WIN32_NT)
                compSpecName = (meUByte *) "command.com";
            else
#endif
                compSpecName = (meUByte *) "cmd.exe";
        }
        compSpecLen = meStrlen(compSpecName);
    }
    
#ifdef _WIN32s
    /* If the pipe to stderr is not defined then get the value. Note that this
     * is a once off get and we only look on the first run */
    if(pipeStderr == 0)
        pipeStderr = ((meGetenv("ME_PIPE_STDERR") != NULL) ? 1 : -1);
#endif
    
    /* set the startup window size */
    memset(&meSuInfo,0,sizeof(STARTUPINFO));
    meSuInfo.cb = sizeof(STARTUPINFO);
    
    /* If there is an output file then we need to synchronise */
    if(flags & LAUNCH_SHELL)
    {
        if(flags & LAUNCH_NOCOMSPEC)
            cp = cmd;                   /* No comspec - use the command */
        else
            cp = compSpecName;         /* Use the comspec */
    }
    else
    {
        /* Create the command line */
        meUByte c1, c2, *dd, *ss;
        
        ss = cmd;
        while(((c1 = *ss) == ' ') || (c1 == '\t'))
            ss++;
        if(c1 == '\0')
            return meFALSE;
        
        if((outFile == NULL) && ((flags & (LAUNCH_SYSTEM|LAUNCH_FILTER|LAUNCH_IPIPE)) == 0))
        {
            /* Create the output file */
            mkTempName(pipeOutFile,NULL,NULL);
            outFile = pipeOutFile;
        }
#ifdef _WIN32s
        status = strlen(ss) + strlen(outFile) + 16;
        if(flags & LAUNCH_FILTER)
            status += strlen(inFile) + 4;
#else
        status = meStrlen(ss) + compSpecLen + 16;
#endif
        if((cmdLine = meMalloc(status)) == NULL)
            return meFALSE;
        cp = dd = cmdLine;
        
#ifndef _WIN32s
        /* If there is no command spec then skip */
        if((flags & LAUNCH_NOCOMSPEC) == 0)
        {
            meStrncpy(dd,compSpecName,compSpecLen);
            dd += compSpecLen;
            /* copy in the <shell> /c */
            meStrncpy(dd," /c ",4);
            dd += 4;
#ifndef _WIN32_WINNT
            if(platformId == VER_PLATFORM_WIN32_NT)
#endif
                *dd++ = '"';
        }
#endif
        
        if((flags & LAUNCH_LEAVENAMES) == 0)
        {
            if(c1 != '"')
                c1 = ' ';
            while(((c2=*ss++)) && (c2 != c1))
            {
                if(c2 == '/')
                    c2 = '\\';
                *dd++ = c2;
            }
            ss--;
        }
        meStrcpy(dd,ss);
        
#ifdef _WIN32_WINNT
        if((flags & LAUNCH_NOCOMSPEC) == 0)
#else
        if((platformId == VER_PLATFORM_WIN32_NT) && ((flags & LAUNCH_NOCOMSPEC) == 0))
#endif
            meStrcat(dd,"\"");
        
        /*        fprintf(fp,"Running [%s]\n",cp);*/
        /*        fflush(fp);*/
        
        if((flags & LAUNCH_SHOWWINDOW) == 0)
        {
            meSuInfo.wShowWindow = SW_HIDE;
            meSuInfo.dwFlags |= STARTF_USESHOWWINDOW;
        }
        
        /* Only a shell needs to be visible, so hide the rest */
        meSuInfo.lpTitle = (char *) cmd;
        meSuInfo.hStdInput  = INVALID_HANDLE_VALUE;
        meSuInfo.hStdOutput = INVALID_HANDLE_VALUE;
        meSuInfo.hStdError  = INVALID_HANDLE_VALUE;
        
        /* If its a system call then we don't care about input or output */
        if((flags & LAUNCH_SYSTEM) == 0)
        {
            SECURITY_ATTRIBUTES sbuts;
            
            sbuts.nLength = sizeof(SECURITY_ATTRIBUTES);
            sbuts.lpSecurityDescriptor = NULL;
            sbuts.bInheritHandle = meTRUE;
            
            if(flags & LAUNCH_FILTER)
            {
#ifdef _WIN32s
                strcat(cp, " <");
                strcat(cp, inFile);
                strcat(cp, " >");
                strcat(cp, outFile);
#else
                HANDLE h;
                if((meSuInfo.hStdInput=CreateFile((const char *) inFile,GENERIC_READ,FILE_SHARE_READ,&sbuts,
                                                  OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL)) == INVALID_HANDLE_VALUE)
                {
                    meFree(cmdLine);
                    return meFALSE;
                }
                if((meSuInfo.hStdOutput=CreateFile((const char *) outFile,GENERIC_WRITE,FILE_SHARE_READ,&sbuts,
                                                   OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL)) == INVALID_HANDLE_VALUE)
                {
                    CloseHandle(meSuInfo.hStdInput);
                    meFree(cmdLine);
                    return meFALSE;
                }
                if(CreatePipe(&meSuInfo.hStdError,&h,&sbuts,0) != 0)
                    CloseHandle(h);
#endif
            }
#if MEOPT_IPIPES
            else if(flags & LAUNCH_IPIPE)
            {
                /* Its an IPIPE so create the pipes */
                if(CreatePipe(&meSuInfo.hStdInput,&inHdlTmp,&sbuts,0) == 0)
                {
                    meFree(cmdLine);
                    return meFALSE;
                }
                if(CreatePipe(&outHdlTmp,&meSuInfo.hStdOutput,&sbuts,0) == 0)
                {
                    CloseHandle(meSuInfo.hStdInput);
                    CloseHandle(inHdlTmp);
                    meFree(cmdLine);
                    return meFALSE;
                }
                // Create new output read handle and the input write handles. Set
                // the Properties to FALSE. Otherwise, the child inherits the
                // properties and, as a result, non-closeable handles to the pipes
                // are created.
                if(!DuplicateHandle(GetCurrentProcess(),inHdlTmp,
                                    GetCurrentProcess(),&inHdl,
                                    0,meFALSE,DUPLICATE_SAME_ACCESS))
                {
                    CloseHandle(meSuInfo.hStdInput);
                    CloseHandle(inHdl);
                    CloseHandle(outHdlTmp);
                    CloseHandle(meSuInfo.hStdOutput);
                    meFree(cmdLine);
                    return meFALSE;
                }
                CloseHandle(inHdlTmp);
                
                if(!DuplicateHandle(GetCurrentProcess(),outHdlTmp,
                                    GetCurrentProcess(),&outHdl,
                                    0,meFALSE,DUPLICATE_SAME_ACCESS))
                {
                    CloseHandle(meSuInfo.hStdInput);
                    CloseHandle(inHdlTmp);
                    CloseHandle(outHdlTmp);
                    CloseHandle(meSuInfo.hStdOutput);
                    meFree(cmdLine);
                    return meFALSE;
                }
                CloseHandle(outHdlTmp);
                
                /* Duplicate stdout => stderr, don't really care if this fails */
                DuplicateHandle(GetCurrentProcess(),meSuInfo.hStdOutput,
                                GetCurrentProcess(),&meSuInfo.hStdError,
                                0,meTRUE,DUPLICATE_SAME_ACCESS);
                
            }
#endif
            else
            {
                /* the output file name will be set as either one was given or we have created one */
#ifdef _WIN32s
                if(pipeStderr > 0)
                    strcat(cp, " >& ");
                else
                    strcat(cp, " > ");
                strcat(cp, outFile);
#else
                meSuInfo.hStdOutput=CreateFile((const char *) outFile,GENERIC_WRITE,FILE_SHARE_WRITE,
                                               &sbuts,CREATE_ALWAYS,FILE_ATTRIBUTE_TEMPORARY,NULL);
                if(meSuInfo.hStdOutput == INVALID_HANDLE_VALUE)
                {
                    meFree(cmdLine);
                    return meFALSE;
                }
                /* Under Windows 95 (and I assume win32s) if there is no
                 * standard input then create an empty file as stdin. This
                 * allows commands such as Grep to not lock up when they
                 * have been mis-typed on the command line with no file.
                 * Otherwise the grep command hangs on an input steam
                 * that does not exist.
                 *
                 * I added this before which sorted the problem - the
                 * code has since been re-arranged and omitted.
                 *
                 * Jon 17/11/97
                 *
                 * SWP - 20/7/99
                 *
                 * Found that some launched programs return before sub programs
                 * have finished, this leaves the "stdin.~~~" file locked.
                 * This is not a problem in this case because the file will be
                 * empty and we should still be able to open it, so don't fail
                 * if we fail to create the file, only fail if we fail to open it.
                 */
                /* Create a dummy input file to stop the process from locking up.
                 *
                 * Construct the dummy input file */
                mkTempName(dummyInFile,NULL,NULL);
                
                if((dumHdl = CreateFile((const char *) dummyInFile,GENERIC_WRITE,FILE_SHARE_WRITE,NULL,
                                         CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL)) != INVALID_HANDLE_VALUE)
                    CloseHandle(dumHdl);
                
                /* Re-open the file for reading */
                if((dumHdl = CreateFile((const char *) dummyInFile,GENERIC_READ,FILE_SHARE_READ,&sbuts,
                                         OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL)) == INVALID_HANDLE_VALUE)
                {
                    DeleteFile((const char *) dummyInFile);
                    CloseHandle(meSuInfo.hStdOutput);
                    meFree(cmdLine);
                    return meFALSE;
                }
                meSuInfo.hStdInput = dumHdl;
                /* Duplicate stdout => stderr, don't really care if this fails */
                DuplicateHandle(GetCurrentProcess(),meSuInfo.hStdOutput,
                                GetCurrentProcess(),&meSuInfo.hStdError,0,meTRUE,
                                DUPLICATE_SAME_ACCESS);
#endif
            }
#ifndef _WIN32s
            meSuInfo.dwFlags |= STARTF_USESTDHANDLES;
#endif
        }
#ifndef _WIN32_WINNT
#ifndef _WIN32s
        else if(platformId == VER_PLATFORM_WIN32_WINDOWS)
        {
            /* For some reason Win98 shell-command start-up path is incorrect unless a dummy input
             * file is used, no idea why but doing the following (taken from above) works! */
            mkTempName(dummyInFile,NULL,NULL);
            
            if((dumHdl = CreateFile((const char *) dummyInFile,GENERIC_WRITE,FILE_SHARE_WRITE,NULL,
                                    CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL)) != INVALID_HANDLE_VALUE)
                CloseHandle(dumHdl);
            
            /* Re-open the file for reading */
            if((dumHdl = CreateFile((const char *) dummyInFile,GENERIC_READ,FILE_SHARE_READ,NULL,
                                     OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL)) == INVALID_HANDLE_VALUE)
            {
                DeleteFile((const char *) dummyInFile);
                CloseHandle(meSuInfo.hStdOutput);
                meFree(cmdLine);
                return meFALSE;
            }
            meSuInfo.hStdInput = dumHdl;
            meSuInfo.dwFlags |= STARTF_USESTDHANDLES;
        }
#endif
#endif
    }
#ifdef _WIN32s
    if((cmdLine == NULL) || (flags & LAUNCH_NOCOMSPEC))
        status = SynchSpawn(cp,/*SW_HIDE*/SW_SHOWNORMAL);
    else
    {
        meUByte buff[1024];
        FILE *fp;
        
        /* Get the current directory in the 32-bit world */
        _getcwd(buff,1024);
        
        /* Create a BAT file to hold the command */
        mkTempName(dummyInFile,NULL,".bat");
        
        if((fp = fopen(dummyInFile, "w")) != NULL)
        {
            /* Change drive */
            fprintf(fp,"%c:\n",buff[0]);
            /* Change directory */
            fprintf(fp,"cd %s\n",buff);
            /* Do the command */
            fprintf(fp,"%s\n",cp);
            fclose(fp);
            
            strcpy(buff,compSpecName);
            strcat(buff," /c ");
            strcat(buff,dummyInFile);
            
            status = SynchSpawn(buff,/*SW_HIDE*/SW_SHOWNORMAL);
        }
        else
            status = 0;
    }
    /* Correct the status frpom the call */
    if(status != 1)
        status = meFALSE;
#else /* ! _WIN32s */
    /* start the process and get a handle on it */
    if(CreateProcess(NULL,(char *) cp,NULL,NULL,
                     ((flags & (LAUNCH_SHELL|LAUNCH_NOWAIT)) ? meFALSE:meTRUE),
                     ((flags & LAUNCH_DETACHED) ? DETACHED_PROCESS : CREATE_NEW_CONSOLE),
                     NULL,NULL,&meSuInfo,&mePInfo))
    {
        status = meTRUE;
        CloseHandle(mePInfo.hThread);
        /* Ipipes need the process handle and we dont wait for it */
        if((flags & LAUNCH_IPIPE) == 0)
        {
            /* For shells we must close the process handle but we dont wait for it */
            if((flags & (LAUNCH_SHELL|LAUNCH_NOWAIT)) == 0)
            {
                /* Wait for filter, system and pipe process to end */
                for(;;)
                {
                    DWORD procStatus;
                    
                    procStatus = WaitForSingleObject(mePInfo.hProcess,200);
                    if(procStatus == WAIT_TIMEOUT)
                    {
                        if(TTahead() && (TTbreakFlag != 0))
                            status = meTRUE;
                        else
                            continue;
                    }
                    else if(procStatus == WAIT_FAILED)
                        status = meFALSE;
                    else
                        status = meTRUE;
                    /* If we're interested in the result, get it */
                    if(sysRet != NULL)
                        GetExitCodeProcess(mePInfo.hProcess,(LPDWORD) sysRet);
                    break;
                }
            }
            /* Close the process */
            CloseHandle(mePInfo.hProcess);
        }
    }
    else
    {
        mlwrite(0,(meUByte *)"[Failed to run \"%s\"]",cp);
        status = meFALSE;
    }
    /* Close the file handles */
    if(meSuInfo.hStdInput != INVALID_HANDLE_VALUE)
        CloseHandle(meSuInfo.hStdInput);
    if(meSuInfo.hStdOutput != INVALID_HANDLE_VALUE)
        CloseHandle(meSuInfo.hStdOutput);
    if(meSuInfo.hStdError != INVALID_HANDLE_VALUE)
        CloseHandle(meSuInfo.hStdError);
#endif /* WIN32s */
    
#if MEOPT_IPIPES
    if(flags & LAUNCH_IPIPE)
    {
        if(status == meFALSE)
        {
            CloseHandle(inHdl);
            CloseHandle(outHdl);
        }
        else
        {
            ipipe->pid = 1;
            ipipe->rfd = outHdl;
            ipipe->outWfd = inHdl;
            ipipe->process = mePInfo.hProcess;
            ipipe->processId = mePInfo.dwProcessId;
            
            /* attempt to create a new thread to wait for activity,
             * this is because windows pipes are crap and doing a Wait on
             * them fails. so us poor programmers have to jump through lots
             * of hoops just to make Bils crap usable, and what really hurts
             * is that after doing so every non-programmer thinks Bills stuff
             * is wonderful! Sometimes the world sucks.
             * Set the childActive event to manual so we can do the global
             * MsgWait and then after its Set do a SingleWait on each reset
             * those that are set.
             */
            ipipe->childActive = NULL;
            ipipe->threadContinue = NULL;
#ifdef USE_BEGINTHREAD
            {
                unsigned long thread;
                if(((ipipe->childActive=CreateEvent(NULL, meTRUE, meFALSE, NULL)) != 0) &&
                   ((ipipe->threadContinue=CreateEvent(NULL, meFALSE, meFALSE, NULL)) != 0) &&
                   ((thread=_beginthread(childActiveThread,0,ipipe)) != -1))
                    ipipe->thread = (HANDLE) thread;
                else
                    ipipe->thread = NULL;
            }
#else
            if(((ipipe->childActive=CreateEvent(NULL, meTRUE, meFALSE, NULL)) != 0) &&
               ((ipipe->threadContinue=CreateEvent(NULL, meFALSE, meFALSE, NULL)) != 0))
                ipipe->thread = CreateThread(NULL,0,childActiveThread,ipipe,0,&(ipipe->threadId));
            else
                ipipe->thread = NULL;
#endif
        }
    }
    else
#endif
        if(flags & LAUNCH_PIPE)
    {
        /* Delete the dummy stdin file if there is one. */
        DeleteFile((const char *) dummyInFile);
    }
    meNullFree(cmdLine);
    return status;
}
#endif /* MEOPT_SPAWN */

/*
 * meFrameResizeWindow()
 * Note that this is a bit of a nasty one.
 * Once frameCur->widthMax and frameCur->depthMax are defined we may not alter their value
 * until EMACS has initialised (that includes calling frameChangeWidth/depth.
 * We rely on the ttinitialised state to ensure that we set these
 * values only once.
 *
 * Once initialised we immediately call meFrameResizeWindow() to change the
 * screen size. This resizes the screen if the font has been changed
 * during the initialisation phase. The resize is then coped with
 * properly within EMACS.
 */
void
meFrameResizeWindow(meFrame *frame)
{
    int nrow;
    int ncol;
    
#ifdef _ME_CONSOLE
#ifdef _ME_WINDOW
    /* If in console mode... */
    if(meSystemCfg & meSYSTEM_CONSOLE)
#endif /* _ME_CONSOLE */
    {
        /* nothing to do for console version */
        ncol = TTwidthDefault;
        nrow = TTdepthDefault;
    }
#ifdef _ME_WINDOW
    else
#endif /* _ME_WINDOW */
#endif /* _ME_CONSOLE */
#ifdef _ME_WINDOW
    {
        /* Get the new canvas and invalidate the whole screen. */
        GetClientRect(meFrameGetWinHandle(frame), &meFrameGetWinCanvas(frame)); /* Get the new canvas size */
        
        nrow = meFrameGetWinCanvas(frame).bottom / eCellMetrics.cell.sizeY;
        if(nrow < 1)
            nrow = 1;
        
        ncol = meFrameGetWinCanvas(frame).right / eCellMetrics.cell.sizeX;
        if(ncol < 1)
            ncol = 1;
        
    }
#endif /* _ME_WINDOW */
    
    /* Inform Emacs window S/W that the window has changed size */
    meFrameChangeWidth(frame,ncol);
    meFrameChangeDepth(frame,nrow);
    meFrameSetWindowSize(frame);
}

/*
 * WinExit
 * User/program has called exit. We use this instead of the converntional
 * exit(2) invocation. This ensures that the process is shut down properly
 * and Windows receives the correct termination messages on the message
 * queue.
 */
void
WinExit(int status)
{
#ifdef _ME_WINDOW
#ifdef _ME_CONSOLE
    /* If not in console mode... */
    if(!(meSystemCfg & meSYSTEM_CONSOLE))
#endif /* _ME_CONSOLE */
        DestroyWindow(meFrameGetWinHandle(frameCur));
#endif /* _ME_WINDOW */
    exit(status);
}

/* WinShutdown; Shutdown the system. It is important that we relinquish some
 * of the resources that we may have aquired during the session. The most
 * important resource to relinquish is the font resource. */
void
WinShutdown(void)
{
    /* ZZZZ - should this have console support */
#if MEOPT_CLIENTSERVER
    /* Close & delete the client file */
    TTkillClientServer();
#endif
#ifdef _ME_WINDOW
#ifdef _ME_CONSOLE
    /* If not in console mode... */
    if(!(meSystemCfg & meSYSTEM_CONSOLE))
#endif /* _ME_CONSOLE */
    {
        /* Destroy the palette information */
        if(eCellMetrics.pInfo.hPal != NULL)  /* Release the palette */
        {
            DeleteObject(eCellMetrics.pInfo.hPal);
            eCellMetrics.pInfo.hPal = NULL;
        }
        if(eCellMetrics.pInfo.cPal != NULL)
        {
            meFree(eCellMetrics.pInfo.cPal);
            eCellMetrics.pInfo.cPal = NULL;
        }
        if(eCellMetrics.pInfo.hPalRefCount != NULL)
        {
            meFree(eCellMetrics.pInfo.hPalRefCount);
            eCellMetrics.pInfo.hPalRefCount = NULL;
        }
        eCellMetrics.pInfo.hPalSize = 0;
        noColors = 0;
        
        /* Free off the background brush */
        if(ttBrush != NULL)
        {
            DeleteObject(ttBrush);
            ttBrush = NULL;
        }
    }
#endif /* _ME_WINDOW */
    
    /* Save any buffers as an emergency quit */
#ifdef WE_SHOULD_NOT_NEED_THIS_HERE
    /* Save any buffers as an emergency quit */
    {
        register meBuffer *bp;    /* scanning pointer to buffers */
        
        bp = bheadp;
        while(bp != NULL)
        {
            if(bufferNeedSaving(bp))
                autowriteout(bp);
            bp = bp->next;            /* on to the next buffer */
        }
        saveHistory(meTRUE,0);
    }
#endif
}

#if MEOPT_MOUSE
/*
 * TTinitMouse
 * Sort out what to do with the mouse buttons.
 */
void
TTinitMouse(meInt nCfg)
{
    meMouseCfg = nCfg;
    if(nCfg & meMOUSE_ENBLE)
    {
        int b1, b2, b3;
        
        if(nCfg & meMOUSE_SWAPBUTTONS)
            b1 = 3, b3 = 1;
        else
            b1 = 1, b3 = 3;
        if((nCfg & meMOUSE_NOBUTTONS) > 2)
            b2 = 2;
        else
            b2 = b3;
        mouseKeys[1] = b1;
        mouseKeys[2] = b2;
        mouseKeys[4] = b3;
        
#ifdef _ME_WINDOW
#ifdef _ME_CONSOLE
        if(!(meSystemCfg & meSYSTEM_CONSOLE))
#endif
        {
            meUByte cc = (meUByte) ((nCfg & meMOUSE_ICON) >> 16);
            if(cc >= meCURSOR_COUNT)
                cc = 0;
            if(cc != meCurCursor)
            {
                if(meCursors[cc] == NULL)
                    meCursors[cc] = LoadCursor(NULL,meCursorName[cc]);
                if(meCursors[cc] != NULL)
                {
                    if(mouseState & MOUSE_STATE_VISIBLE)
                        SetCursor(meCursors[cc]);
                    meCurCursor = cc;
                }
            }
        }
#endif
    }
}

/*
 * WinMouse
 * Handle mouse events from the message queues.
 * Returning meTRUE if the event is handled; otherwise meFALSE.
 */
int
WinMouse(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    meFrame *frame;
    
    if((frame = meMessageGetFrame(hwnd)) == NULL)
        return meFALSE;
    
    if(!(meMouseCfg & meMOUSE_ENBLE))
    {
        switch(message)
        {
        case WM_MOUSEMOVE:
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP:
            return meTRUE;
        }
        return meFALSE;
    }
#ifdef _ME_WINDOW
#ifdef _ME_CONSOLE
    if(!(meSystemCfg & meSYSTEM_CONSOLE))
#endif /* _ME_CONSOLE */
        meModifierUpdate();
#endif /* _ME_WINDOW */
    
    switch(message)
    {
    case WM_MOUSEMOVE:
        /* If a button was pressed/release outside the window we don't
         * see it!!! The first chance we get to see it is when the user
         * moves the mouse back into the window - so check the mouse
         * button state is the same, if not treat this event as a button
         * event
         */
        if(((wParam ^ mouseButs) & (MK_LBUTTON|MK_RBUTTON|MK_MBUTTON)) == 0)
        {
            static LPARAM lastPos=-1;
            meUInt arg;           /* Decode key argument */
            meUShort cc;
            
            /* Mouse buttons are the same - handle as a move mouse */
            
#if _WIN_DEBUG_MOUSE
            if(logfp != NULL)
            {
                fprintf(logfp,"Mouse button %x %08x %08x\n",message, wParam, lParam);
                fflush(logfp);
            }
#endif
            /* Convert the mouse coordinates to cell space. Compute
             * the fractional bits which are 1/128ths */
            mousePosUpdate(lParam);
            /* if the mouse is not in the client area and no button is
             * currently pressed then we don't handle this event, the default
             * handler should to pass on NC events  */
            if(!mouseInFrame && !mouseButs)
                return meFALSE;
            
            /* Found that on NT ME continually gets MOUSEMOVE messages even
             * though the mouse hasn't moved! I wonder if thats why it runs
             * so slow? To stop the mouse cursor reappearing & generating
             * continuous mouse_move 'keys' store the last position and only
             * do something if the mouse really has moved. */
            if(lParam != lastPos)
            {
                /* Check for a mouse-move key */
                cc = ME_SPECIAL | ttmodif | (SKEY_mouse_move+mouseKeys[mouseState&MOUSE_STATE_BUTTONS]);
                /* Are we after all movements or mouse-move bound ?? */
                if((!TTallKeys && (decode_key(cc,&arg) != -1)) || (TTallKeys & 0x1))
                    addKeyToBufferOnce(cc);        /* Add key to keyboard buffer */
                mouseShow();
                lastPos = lParam;
            }
            break;
        }
        /* Jon 00/02/12: Fault report by "Dave E" that the screen was
         * always selected under Windows '95 when double clicking from an
         * icon. A trace from his machine revealed that the mouse was
         * showing the left button pressed on a mouse move. The left mouse
         * was not in fact pressed.
         *
         * As a result of the above we now consider it dangerous to infer
         * button presses from mouse movements. Hence we no longer drop
         * through and process the button state. Simply ignore the button
         * state  and ignore the mouse move !! A click of the mouse is
         * required to correct this state.
         *
         * I have not been able to re-create the above. The fix has
         * not yet shown any adverse effects - although I would not
         * expect it to. */
        return meFALSE;
        
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
        {
            int mouseCode, ii;
            
            mousePosUpdate(lParam);
            /* if the mouse is not in the client area and no other button is
             * currently pressed then we don't handle this event, the default
             * handler should to pass on NC events  */
            if(!mouseInFrame && !mouseButs)
                return meFALSE;
            
            mouseButs = wParam;
            
#if _WIN_DEBUG_MOUSE
            if(logfp != NULL)
            {
                fprintf(logfp,"Mouse button %x %08x %08x - %d %d\n",message,wParam,lParam,mouse_X,mouse_Y);
                fflush(logfp);
            }
#endif
            mouseCode = 0;
            if(wParam & MK_LBUTTON)
                mouseCode |= MOUSE_STATE_LEFT;
            if(wParam & MK_MBUTTON)
                mouseCode |= MOUSE_STATE_MIDDLE;
            if(wParam & MK_RBUTTON)
                mouseCode |= MOUSE_STATE_RIGHT;
            
            for(ii=1 ; ii < 8 ; ii <<= 1)
            {
                if(((mouseCode ^ mouseState) & ii) != 0)
                {
                    meUShort cc;
                    if(mouseCode & ii)             /* Release ?? */
                        cc = SKEY_mouse_pick_1;    /* Get mouse pick key binding */
                    else
                        cc = SKEY_mouse_drop_1;    /* Get mouse drop key binding */
                    
                    /* Generate the key code and report */
                    cc = (ME_SPECIAL|ttmodif)|(cc+mouseKeys[ii]-1);
                    
                    addKeyToBuffer(cc);
                }
            }
            mouseState = (mouseState & ~MOUSE_STATE_BUTTONS)|mouseCode;
            mouseShow();
#ifdef _ME_WINDOW
#ifdef _ME_CONSOLE
            if(!(meSystemCfg & meSYSTEM_CONSOLE))
#endif /* _ME_CONSOLE */
            {
                if(mouseState & MOUSE_STATE_BUTTONS)
                {
                    if((mouseState & MOUSE_STATE_LOCKED) == 0)
                    {
                        SetCapture(meFrameGetWinHandle(frame));    /* Lock the mouse */
                        mouseState |= MOUSE_STATE_LOCKED;
                    }
                }
                else if(mouseState & MOUSE_STATE_LOCKED)
                {
                    ReleaseCapture();         /* Relinquish the mouse */
                    mouseState &= ~MOUSE_STATE_LOCKED;
                }
            }
#endif /* _ME_WINDOW */
            break;
        }
#ifdef WM_MOUSEWHEEL
    case WM_MOUSEWHEEL:
        {
            meUShort cc;
            
#if _WIN_DEBUG_MOUSE
            if(logfp != NULL)
            {
                fprintf(logfp,"Mouse wheel  %x %08x %08x\n",message, wParam, lParam);
                fflush(logfp);
            }
#endif
            /* unlike mouse move or button events the lParam mouse position is
             * absolute, not the position within the window, we should therefore
             * subtract the top left point of the window from the position to get
             * an ME mouse position, but that can lead to negative numbers which is
             * probably why windows is inconsistent in this area so let the other
             * mouse events set the position
               mousePosUpdate(lParam);*/
            if(wParam & 0x80000000)
                cc = ME_SPECIAL|SKEY_mouse_wdown;
            else
                cc = ME_SPECIAL|SKEY_mouse_wup;
            cc |= ttmodif;
            addKeyToBuffer(cc);
            break;
        }
#endif
    default:
        return meFALSE;
    }
    
    return meTRUE;
}
#endif

/*
 * WinKeyboard
 * Handle keyboard message types.
 * Returning meTRUE if the event is handled; otherwise meFALSE.
 */
int
WinKeyboard(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    meFrame *frame;
    meUShort cc;                  /* Local keyboard character */
    
    if((frame = meMessageGetFrame(hwnd)) == NULL)
        return meFALSE;
    
#ifdef _ME_WINDOW
#ifdef _ME_CONSOLE
    if(!(meSystemCfg & meSYSTEM_CONSOLE))
#endif /* _ME_CONSOLE */
        meModifierUpdate();
#endif /* _ME_WINDOW */
#if _WIN_DEBUG_KEY
    if(logfp != NULL)
    {
        char *name;
        
        switch(message)
        {
        case WM_SYSKEYDOWN:
            name = "WM_SYSKEYDOWN";
            break;
        case WM_KEYDOWN:
            name = "WM_KEYDOWN";
            break;
        case WM_SYSKEYUP:
            name = "WM_SYSKEYUP";
            break;
        case WM_KEYUP:
            name = "WM_KEYUP";
            break;
        case WM_SYSCHAR:
            name = "WM_SYSCHAR";
            break;
        case WM_CHAR:
            name = "WM_CHAR";
            break;
        case WM_DEADCHAR:
            name = "WM_DEADCHAR";
            break;
        default:
            name = "?WM_UNKNOWN?";
            break;
        }
        
        fprintf(logfp,"%s::%d(0x%08x). wParam = %d(%04x) lParam = %d(%08x) modif %x, %x\n",
                name,message,message,wParam,wParam,lParam,lParam,ttmodif,keyDownChr);
        fflush(logfp);
    }
#endif
    switch(message)
    {
    case WM_SYSKEYDOWN:
        /* For some unknown reason, bill has decided that an F10 key
         * press will generate a SYSKEYDOWN, cope with this oddity */
        if(wParam != VK_F10)
        {
#ifdef _ME_WINDOW
#ifdef _ME_CONSOLE
            if(!(meSystemCfg & meSYSTEM_CONSOLE))
#endif /* _ME_CONSOLE */
            {
                if((lParam & (1<<29)) == 0)    /* Windows got no input focus ? */
                {
                    SetFocus(meFrameGetWinHandle(frame));          /* No - aquire focus */
                    return meFALSE;
                }
            }
#endif /* _ME_WINDOW */
        }
    case WM_KEYDOWN:
        keyDownChr = ttmodif;
        /* Determine the special keyboard character we are handling */
        switch(wParam)
        {
        case VK_SHIFT:
        case VK_CONTROL:
        case VK_MENU:
            break;
        case VK_LEFT:
            /* Distinguish cursor/number-pad keys */
            cc = ((lParam & 0x01000000) ? SKEY_left : SKEY_kp_left);
            goto do_keydown;
        case VK_RIGHT:
            /* Distinguish cursor/number-pad keys */
            cc = ((lParam & 0x01000000) ? SKEY_right : SKEY_kp_right);
            goto do_keydown;
        case VK_UP:
            /* Distinguish cursor/number-pad keys */
            cc = ((lParam & 0x01000000) ? SKEY_up : SKEY_kp_up);
            goto do_keydown;
        case VK_DOWN:
            /* Distinguish cursor/number-pad keys */
            cc = ((lParam & 0x01000000) ? SKEY_down : SKEY_kp_down);
            goto do_keydown;
        case VK_INSERT:
            /* Distinguish control-pad/number-pad keys */
            cc = ((lParam & 0x01000000) ? SKEY_insert : SKEY_kp_insert);
            goto do_keydown;
        case VK_DELETE:
            /* Distinguish control-pad/number-pad keys */
            cc = ((lParam & 0x01000000) ? SKEY_delete : SKEY_kp_delete);
            goto do_keydown;
        case VK_PRIOR:
            /* Distinguish control-pad/number-pad keys */
            cc = ((lParam & 0x01000000) ? SKEY_page_up : SKEY_kp_page_up);
            goto do_keydown;
        case VK_NEXT:
            /* Distinguish control-pad/number-pad keys */
            cc = ((lParam & 0x01000000) ? SKEY_page_down : SKEY_kp_page_down);
            goto do_keydown;
        case VK_HOME:
            /* Distinguish control-pad/number-pad keys */
            cc = ((lParam & 0x01000000) ? SKEY_home : SKEY_kp_home);
            goto do_keydown;
        case VK_END:
            /* Distinguish control-pad/number-pad keys */
            cc = ((lParam & 0x01000000) ? SKEY_end : SKEY_kp_end);
            goto do_keydown;
        case VK_CLEAR:
            /* This comes from the number pad 5 with numlock off. Mapped to
             * kp-begin (UNIX derivation). */
            cc = SKEY_kp_begin;
            goto do_keydown;
        case VK_APPS:
            /* 105-Key keyboard, the Apps button mapped to "menu" */
            cc = SKEY_menu;
            goto do_keydown;
        case VK_PAUSE:
            cc = SKEY_pause;
            goto do_keydown;
            
            /* the following lock keys must be bound to generate a key event */
        case VK_CAPITAL:
            cc = SKEY_caps_lock;
            goto test_do_keydown;
        case VK_NUMLOCK:
            cc = SKEY_num_lock;
            goto test_do_keydown;
        case VK_SCROLL:
            cc = SKEY_scroll_lock;
test_do_keydown:
            {
                meUInt arg;
                if(decode_key((meUShort) (ME_SPECIAL|ttmodif|cc),&arg) != -1)  /* Key bound ?? */
                    goto do_keydown;
            }
            break;
        case VK_F1:
            cc = SKEY_f1;
            goto do_keydown;
        case VK_F2:
            cc = SKEY_f2;
            goto do_keydown;
        case VK_F3:
        case VK_F4:
        case VK_F5:
        case VK_F6:
        case VK_F7:
        case VK_F8:
        case VK_F9:
            cc = SKEY_f3 + ((meUShort) (wParam - VK_F3));
            goto do_keydown;
        case VK_F10:
        case VK_F11:
        case VK_F12:
            cc = SKEY_f10 + ((meUShort) (wParam - VK_F10));
do_keydown:
            cc = (ME_SPECIAL | ttmodif | cc);
            /* Add the character to the typeahead buffer.
             * Note that we do not process (lParam & 0xff) which is the
             * auto-repeat count - this always appears to be 1. */
#if _WIN_DEBUG_KEY
            if(logfp != NULL)
            {
                fprintf(logfp,"addKeyToBuffer1    %c - %d(0x%04x)\n",cc & 0xff, cc, cc);
                fflush(logfp);
            }
#endif
            addKeyToBuffer(cc);
            /* hide the mouse cursor
             * This must be done whether MEOPT_MOUSE is enabled or not */
            mouseHide();
            break;
            
        default:
            if(message != WM_KEYDOWN)
                return meFALSE;           /* WM_SYSKEYDOWN NOT PROCESSED - return a false state */
            
            /* Generally, Windows sends a WM_KEYDOWN for all key presses, we then call
             * TranslateMessage which uses the ToUnicode function (see WinKeyTranslate) to convert
             * the Virtual-key into a char and then generate a WM_CHAR message. For a simple util
             * the KEYDOWN msgs can be largely ignored as the CHAR msgs are all they need, but not
             * ME. We want all key input, including some very unusual key combinations like C-#, and
             * this creates problems.
             * 
             * First issue is that Windows simply grabs some key input so ME never sees it, e.g.
             * C-S-0 ('C-)' on UK keyboard), nothing we can do about that unless we create a lower
             * level input driver (e.g. AutoHotkey can get the key) however this type of interface
             * can create security and permission issues and these keys are not deemed important
             * enough to warrent that risk and complexity at this stage.
             * 
             * Second and the bigger problem, when many keys are translated they don't produce a
             * 'valid' char, e.g. C-=, and Windows simply drops these, as a result ME will get a
             * KEYDOWN message but not a CHAR msg. So our first task is to identify KEYDOWN messages
             * that ME would want to capture but will not generate a WM_CHAR msg (e.g. the KEYDOWN
             * of '=' when Ctrl is also already pressed, but not for example the pressing of the
             * Ctrl on its own). These keys must be added to the Key buffer here.
             * 
             * Care needs to be taken w.r.t. DEADCHARs (accent chars like accute or umlaut), these
             * produce no key on the its input but will on the second key, so we must also ignore
             * the dead key input.
             * 
             * Steve 2025-06-20
             */
            {
                int ci;
                ci = WinKeyTranslate(wParam,ttmodif);
                /* if the initial key translates to anything, including ones above 255 & deadchars, return */
                if(ci != 0)
                {
                    keyDownChr = (meUShort) (ci & 0xff);
                    return meFALSE;
                }
                /* Get the base key, we must pass in the Shift state as this will change the key and
                 * we must use Windows API as this is affected by different keyboard layouts etc. */
                if((ci = WinKeyTranslate(wParam,(ttmodif & ME_SHIFT))) > 0)
                    cc = ci | (ttmodif & ~ME_SHIFT);
                else if((ttmodif & ME_SHIFT) && ((ci = WinKeyTranslate(wParam,0)) > 0))
                    cc = ci | ttmodif;
                else
                    return meFALSE;
                
                if((ttmodif & ME_CONTROL) && (ci > 0x40) && (ci < 0x7b))
                    cc &= 0x1f|ME_ALT|ME_SHIFT;
#if _WIN_DEBUG_KEY
                if(logfp != NULL)
                {
                    fprintf(logfp,"addKeyToBuffer2    %c - %d(0x%04x) 0x%04x\n",cc & 0xff,cc,cc,ci);
                    fflush(logfp);
                }
#endif
                keyDownChr = cc;
                addKeyToBuffer(cc);
                mouseHide();
                
            }
        }
        break;
        
    case WM_SYSKEYUP:
    case WM_KEYUP:
        switch(wParam)
        {
        case VK_MENU:
        case VK_SHIFT:
        case VK_CONTROL:
            break;
        default:
            return meFALSE;
        }
        break;
        
    case WM_SYSCHAR:
    case WM_CHAR:
        /* Handle any special keys here */
        switch(wParam)
        {
        case VK_RETURN:
#ifdef _ME_CONSOLE
#ifdef _ME_WINDOW
            if(meSystemCfg & meSYSTEM_CONSOLE)
#endif /* _ME_WINDOW */
            {
                /* For the console check the control flag. A control flag will
                 * indicate that the control C-m has been pressed as opposed
                 * to <RETURN> */
                if((ttmodif & ME_CONTROL) == 0)
                {
                    cc = SKEY_return;
                    goto return_spec;
                }
                break;                    /* This is C-m */
            }
#endif
#ifdef _ME_WINDOW
            /* Look at the scan code to determine if this is a C-m or a
             * <return>. The scan code for 'M' is 0x32; the scan code for
             * <RETURN> is 0x1c. If this is a C-m then simply drop through,
             * wParam should be 0x1d which is C-m. */
        case 0x0a:
            /* Found on Win10 C-return now uses C-j (0x0a) as the wParam
             * (which is a reserved value); the scan code for return is still
             * given as 0x1c where as a C-j has a code of 0x24. */
            if((lParam & 0xff0000) == (0x1c<<16))
            {
                cc = SKEY_return; /* Return */
                goto return_spec;
            }
#endif
        }
        cc = (meUShort) wParam;
        if(cc > 0x20)
        {
            if(cc == 0x7f)
            {
                /* Special case of the C-backspace key */
                cc = SKEY_backspace;
                goto return_spec;
            }
        }
        else if(cc == 0x20)
        {
            if(ttmodif & ME_CONTROL)
                cc |= ME_CONTROL;
            else if((ttmodif == ME_ALT) && !(meSystemCfg & meSYSTEM_CTCHASPC))
                return meFALSE;          /* NOT PROCESSED - return a false state */
        }
        else if((ttmodif & ME_CONTROL) == 0)
        {
            if(cc == 0x09)
            {
                cc = SKEY_tab;
                goto return_spec;
            }
            if(cc == 0x1b)
            {
                cc = SKEY_esc;
                goto return_spec;
            }
            if(cc == 0x08)
            {
                cc = SKEY_backspace;
                goto return_spec;
            }
        }
        if(ttmodif & ME_ALT)
        {
            /* If the ALT is pressed and the CONTROL isn't this is an A-? or A-S-? key. If ALT &
             * CONTROL are pressed then we must be careful because this could be an AltGr extended
             * character, e.g. A-C-e for Euro.
             * 
             * When not running In a console we can use the KEYDOWN msg to work this out, keyDownChr stores
             * the best translation of the key which will not have ALT or CONTROL set if its an
             * AltGr char.
             * 
             * When running in a console ME does not get KEYDOWN msgs for each CHAR and the CHAR msg
             * doesn't have the scancode or the virtual-key, so the best we can do is only enable
             * ALT if CONTROL is not set
             */
            if(meSystemCfg & meSYSTEM_CONSOLE)
            {
                if(!(ttmodif & ME_CONTROL))
                    cc = ME_ALT | toLower(cc);
            }
            else if(keyDownChr & ME_ALT)
                cc = ME_ALT | toLower(cc);
        }
        else if((ttmodif & ME_CONTROL) && (cc > 0x1a) && !(meSystemCfg & meSYSTEM_CONSOLE))
        {
            /* For reasons unknown, Windows sometimes miss-translates the base key of C-'?' when the
             * control is pressed and not the alt, it doesn't seem to happen when SHIFT or ALT are
             * also pressed. On UK keyboard C-# gets converted to C-\\ by the time we process the
             * WM_CHAR. On DEU keyboard C-# also goes to C-\\, but also C-:u (u-umlaut) -> C-[,
             * C-+ -> C-] and C-& -> C-^
             * 
             * Fix this by translating the base scancode of the key, without the CONTROL set, and
             * then bor in the CONTROL flag and store this as the key.
             * 
             * This cannot be fixed in console mode because ME does not get the original scancode!
             * 
             * Steve 2025-06-20
             */
            UINT cv;
            int ci;
            if(((cv=MapVirtualKey(((lParam>>16) & 0x0ff),MAPVK_VSC_TO_VK)) > 0) && 
               ((ci = WinKeyTranslate(cv,(ttmodif & ME_SHIFT))) > 0))
            {
                if((ci > 0x40) && (ci < 0x60))
                    cc = (ci & 0x1f);
                else
                    cc = ci | ME_CONTROL;
#if _WIN_DEBUG_KEY
                if(logfp != NULL)
                {
                    fprintf(logfp,"  Special handler for C-<key>:  %02x -> %02x -> %02x\n",cv,ci,cc);
                    fflush(logfp);
                }
#endif
            }
        }
        /* Add the character to the typeahead buffer.
         * Note that we do no process (lParam & 0xff) which is the
         * auto-repeat count. - this always appears to be 1 */
#if _WIN_DEBUG_KEY
        if(logfp != NULL)
        {
            fprintf(logfp,"addKeyToBuffer3    %c - %d(0x%04x)\n",cc & 0xff, cc, cc);
            fflush(logfp);
        }
#endif
        addKeyToBuffer(cc);
        /* hide the mouse cursor
         * This must be done whether MEOPT_MOUSE is enabled or not */
        mouseHide();
        break;
return_spec:
        cc = (ME_SPECIAL | ttmodif | cc);
#if _WIN_DEBUG_KEY
        if(logfp != NULL)
        {
            fprintf(logfp,"addKeyToBuffer4    %c - %d(0x%04x)\n",cc & 0xff, cc, cc);
            fflush(logfp);
        }
#endif
        addKeyToBuffer(cc);
        /* hide the mouse cursor
         * This must be done whether MEOPT_MOUSE is enabled or not */
        mouseHide();
        break;
    default:
        return meFALSE;
    }
    return meTRUE;
}

/****************************************************************************
 *
 * STANDARD EMACS TT FUNCTIONS
 *
 ****************************************************************************/

/*
 * TTaddColor
 * Add a new colour definition to the palette
 * Note that we try to minimize palette usage by sharing colour entries wherever
 * possible. This makes windows more efficient. Also there is NOT a 1:1 correspondence
 * between the palette entries and the colour indexes requested by EMACS. New palette
 * entries are created when we have not got an existing colour in the palette. If an
 * index changes colour, then the existing colour is removed from the palette table
 * if it's refrence count drops to zero. The palette table is resized, the last colour
 * in the palette table moves to the vacant position.
 */
int
TTaddColor(meColor index, meUByte r, meUByte g, meUByte b)
{
#ifdef _ME_CONSOLE
#ifdef _ME_WINDOW
    if(meSystemCfg & meSYSTEM_CONSOLE)
#endif /* _ME_WINDOW */
    {
        meUByte *ss;
        int ii, jj, idif, jdif;
        
        jdif = 256+256+256;
        ss = consoleColors;
        for(ii=0 ; ii<consoleNumColors ; ii++)
        {
            idif  = abs(r - *ss++);
            idif += abs(g - *ss++);
            idif += abs(b - *ss++);
            if(idif < jdif)
            {
                jdif = idif;
                jj = ii;
            }
        }
        
        if(noColors <= index)
        {
            colTable = meRealloc(colTable,(index+1)*sizeof(meUInt));
            memset(colTable+noColors,0,(index-noColors+1)*sizeof(meUInt));
            noColors = index+1;
        }
        colTable[index] = jj;
    }
#ifdef _ME_WINDOW
    else
#endif /* _ME_WINDOW */
#endif /* _ME_CONSOLE */
#ifdef _ME_WINDOW
    {
        COLORREF cReq;                      /* Color requested */
        COLORREF cAsg;                      /* Color assigned */
        HDC      hDC;                       /* Device context */
        
        cReq = RGB(r,g,b);                  /* Construct the new colour reference */
        hDC = GetDC(NULL);                  /* Get the device context */
        
        /* Determine if we are adding to the device context or the palette */
        if(GetDeviceCaps(hDC,RASTERCAPS) & RC_PALETTE)
        {
            int maxPaletteSize;             /* Maximum size of the palette */
            int closeIndex;                 /* A close existing palette index */
            PALETTEENTRY closeEntry;        /* A close palette entry */
            COLORREF closeColor;           /* A close existing palette colour */
            
            maxPaletteSize = GetDeviceCaps(hDC, SIZEPALETTE);
            
            /* Delete the old palette entry if it existed previously */
            if((index < noColors) && (eCellMetrics.pInfo.cPal[index].rgb != 0))
            {
                closeIndex = GetNearestPaletteIndex(eCellMetrics.pInfo.hPal,
                                                    eCellMetrics.pInfo.cPal[index].rgb);
                
                /* Reference count has reached zero. Remove the palette entry */
                if(--eCellMetrics.pInfo.hPalRefCount[closeIndex] <= 0)
                {
                    if(--eCellMetrics.pInfo.hPalSize == 0)
                    {
                        /* Destruct the palette */
                        meFree(eCellMetrics.pInfo.hPalRefCount);
                        eCellMetrics.pInfo.hPalRefCount = NULL;
                        
                        DeleteObject(eCellMetrics.pInfo.hPal);
                        eCellMetrics.pInfo.hPal = NULL;
                    }
                    else
                    {
                        if(closeIndex <  eCellMetrics.pInfo.hPalSize)
                        {
                            /* Reduce the size of the palette by re-shuffling */
                            GetPaletteEntries(eCellMetrics.pInfo.hPal,
                                              eCellMetrics.pInfo.hPalSize, 1, &closeEntry);
                            SetPaletteEntries(eCellMetrics.pInfo.hPal,
                                              closeIndex, 1, &closeEntry);
                            
                            /* Move the reference count entry - do not bother to resize the
                             * reference count table */
                            eCellMetrics.pInfo.hPalRefCount[closeIndex] =
                                  eCellMetrics.pInfo.hPalRefCount[eCellMetrics.pInfo.hPalSize];
                        }
                        /* Resize the palette */
                        ResizePalette(eCellMetrics.pInfo.hPal, eCellMetrics.pInfo.hPalSize);
                    }
                }
            }
            
            /* Create a new entry for the colour
             * Check the current palette for the colour. If we have it already we do
             * not need to create a new palette entry. */
            if(eCellMetrics.pInfo.hPalSize > 0)
            {
                closeIndex = GetNearestPaletteIndex(eCellMetrics.pInfo.hPal, cReq);
                if(closeIndex != CLR_INVALID)
                {
                    GetPaletteEntries(eCellMetrics.pInfo.hPal, closeIndex, 1, &closeEntry);
                    closeColor = RGB(closeEntry.peRed, closeEntry.peGreen, closeEntry.peBlue);
                }
                else
                    closeColor = 0xffffffff;
                
                /* Check for duplicates */
                if(cReq == closeColor)
                {
                    /* Already have it in the palette */
                    eCellMetrics.pInfo.hPalRefCount[closeIndex] += 1;
                }
                else
                {
                    /* Fail if the colour map is full */
                    if(eCellMetrics.pInfo.hPalSize == maxPaletteSize)
                    {
                        /* ERROR - not enough colours */
                        ReleaseDC(NULL, hDC);
                        return meFALSE;
                    }
                    else
                    {
                        /* A  a new entry to the palette */
                        closeIndex = eCellMetrics.pInfo.hPalSize;
                        eCellMetrics.pInfo.hPalSize++;
                        ResizePalette(eCellMetrics.pInfo.hPal, eCellMetrics.pInfo.hPalSize);
                        
                        closeEntry.peRed = r;
                        closeEntry.peGreen = g;
                        closeEntry.peBlue = b;
                        closeEntry.peFlags = 0;
                        SetPaletteEntries(eCellMetrics.pInfo.hPal, closeIndex, 1, &closeEntry);
                        
                        /* Resize the pallete reference table */
                        eCellMetrics.pInfo.hPalRefCount = (int *) meRealloc(eCellMetrics.pInfo.hPalRefCount,
                                                                            (sizeof(int) * eCellMetrics.pInfo.hPalSize));
                        eCellMetrics.pInfo.hPalRefCount[closeIndex] = 1;
                    }
                }
            }
            else
            {
                LPLOGPALETTE lPal;          /* Logical palette */
                
                /* No palette is defined. Allocate a new palette */
                lPal = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
                                  sizeof(LOGPALETTE) + sizeof(PALETTEENTRY));
                lPal->palVersion = 0x300;   /* Windows magic number !! */
                lPal->palNumEntries = 1;    /* 2 entries in the palette */
                
                lPal->palPalEntry[0]. peRed = r;
                lPal->palPalEntry[0]. peGreen = g;
                lPal->palPalEntry[0]. peBlue = b;
                lPal->palPalEntry[0]. peFlags = 0;
                
                /* Create the palette */
                eCellMetrics.pInfo.hPal = CreatePalette(lPal);
                HeapFree(GetProcessHeap(), 0, lPal);  /* Dispose of the local palette */
                
                eCellMetrics.pInfo.hPalSize = 1;
                
                /* Create the reference count */
                eCellMetrics.pInfo.hPalRefCount = meMalloc(sizeof(int));
                eCellMetrics.pInfo.hPalRefCount[0] = 1;
            }
            
            /* Save the assigned colour */
            cAsg = PALETTERGB (r, g, b);
        }
        else
        {
            /* Determine what colour will actually be used on non-colour mapped systems */
            cAsg = GetNearestColor(hDC, PALETTERGB(r, g, b));
        }
        
        ReleaseDC (NULL, hDC);
        
        /* Grow the colour table if required */
        if(index >= noColors)
        {
            int ii;
            
            eCellMetrics.pInfo.cPal = meRealloc((void *) eCellMetrics.pInfo.cPal,
                                                sizeof(PaletteItem) * (index+1));
            /* Set to black */
            for(ii=noColors ; ii <= index ; ii++)
            {
                eCellMetrics.pInfo.cPal[ii].rgb = 0;
                eCellMetrics.pInfo.cPal[ii].cpixel = PALETTERGB(0, 0, 0);
            }
            noColors = index + 1;
        }
        
        /* Assign the allocated colour */
        eCellMetrics.pInfo.cPal[index].cpixel = cAsg;
        eCellMetrics.pInfo.cPal[index].rgb = cReq;
    }
#endif /* _ME_WINDOW */
    return meTRUE;
}

#ifdef _ME_WINDOW
/*
 * TTchangeFont
 * 
 * Change the current font setting. Re-compute the cell metrics for the client area and change to
 * the new font. If the font cannot be found then default to the default OEM font. 
 * Arg n is a bitwise flag defined as:
 *  01  Input new font details
 *  02  Don't apply font
 *  04  Don't set $result
 *  08  Use dialog to select font
 *  10  List all fonts in dialog, not just fixed width
 * $result format = "|Pitch&Tech|CharSet|Weight|SizeX|SizeY|ReqX|ReqY|FontName|"
 */
static int
TTchangeFont(int n, int fontType, meUByte *fontName, int fontWeight, int fontHeight, int fontWidth)
{
    HDC hDC;                            /* Device context */
    HFONT oldFont=NULL, newFont=NULL;   /* The new font */
    TEXTMETRIC textmetric;              /* Text metrics */
    LOGFONT logfont;                    /* Logical font */
    int mode, szX, szY;
    
    hDC = GetDC(baseHwnd);
    SetMapMode(hDC,MM_TEXT);
    SetMapperFlags(hDC,1);              /* Allow interpolation */
    
    if(n & 0x01)
    {
        /* Reset the default logical font */
        memset(&logfont,0,sizeof(LOGFONT));
        logfont.lfCharSet = fontType;
        logfont.lfWeight = FW_DONTCARE;
        logfont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
        logfont.lfQuality = DRAFT_QUALITY;
        logfont.lfPitchAndFamily = FIXED_PITCH|FF_DONTCARE;
    
        if(n & 0x08)
        {
            CHOOSEFONT chooseFont;
            
            memset(&chooseFont,0,sizeof(CHOOSEFONT));
            chooseFont.lStructSize = sizeof(CHOOSEFONT);
            chooseFont.hwndOwner = baseHwnd;
            chooseFont.lpLogFont = &logfont;
            chooseFont.Flags = (n & 0x10) ? CF_SCREENFONTS:(CF_FIXEDPITCHONLY|CF_SCREENFONTS);
            
            /* This stupid Microsoft system, why is the CHOOSEFONT structure
             * size not sizeof (CHOOSEFONT). Typical Microsoft !! */
            mode = ChooseFont(&chooseFont);
            /* give current frame the focus again */
            SetFocus(meFrameGetWinHandle(frameCur));
            if(mode == 0)
                /* SWP - if the user cancelled just return false leaving the font alone */
                return meFALSE;
            /* We never want italic as the main font */
            logfont.lfItalic = 0;
            mode = 2;
        }
        else if((fontName != NULL) && (fontName[0] != '\0'))
        {
            logfont.lfWeight = fontWeight * 100;
            logfont.lfHeight = fontHeight;
            logfont.lfWidth = fontWidth;
            meStrncpy(logfont.lfFaceName,fontName,sizeof(logfont.lfFaceName));
            mode = 1;
        }
        else
        {
            /* set to default font for fontType */
            logfont.lfHeight = 0;
            logfont.lfWidth = 0;
            mode = 0;
        }
        /* Create the new font */
        if(!mode)
            newFont = GetStockObject((fontType == OEM_CHARSET) ? OEM_FIXED_FONT:ANSI_FIXED_FONT);
        else if((newFont = CreateFontIndirect(&logfont)) == NULL)
            return meFALSE;
        
        oldFont = (HFONT) SelectObject(hDC,newFont);
        GetTextMetrics(hDC,&textmetric);
        /* Build up the cell metrics */
        /* The tmMaxCharWidth can be larger than tmAveCharWidth even for fixed width fonts due to Windows ClearType,
         * however we are using ETO_CLIPPED to avoid overrun so ignore the extra width
         * 
         * Note: WRT. ME's 'Fonts' (use of bold & italic etc) it may seem reasonable to add in the textmetric.tmOverhang
         * value if meSCHEME_NOFONT is not set, however this value seems to be always 0 on the base font (reasonable)
         * and windows fonts seem to fit into one of 2 groups:
         * a. Properly designed forms for bold & italic etc: For good fixed width fonts the width is unchanged so no
         *    addition is required, ME simply works.
         * b. No font form built into the font: For these fonts windows generates them by overstriking and shearing the
         *    base font, for these auto-generated forms the tmOverhang can be as much as the tmAveCharWidth which would
         *    make the text unreadable anyway - these fonts must either be used with meSCHEME_NOFONT set or avoided.
         */
        szX = (textmetric.tmAveCharWidth <= 0) ? 1:textmetric.tmAveCharWidth;
        if(textmetric.tmPitchAndFamily & TMPF_FIXED_PITCH)
        {
            /* however, if this is a proportial font we need to do more if we can. Using the tmMaxCharWidth value is not
             * a good solution because this can include all the chiness characters which can be twice as wide as we need
             * as we only use chars 0 to 255, the average can also be more then width of all 0 - 255. So calc the width
             * of a 'W' and use that instead, this will most likely look awful but should stop most/all character clipping */
            SIZE sz;
            GetTextExtentPoint32(hDC,"W",1,&sz);
            if((sz.cx > 0) && (sz.cx <= textmetric.tmMaxCharWidth))
                szX = sz.cx;
        }
        /* Note: We may need to add on textmetric.tmExternalLeading otherwise the following line might be unreadably
         * close, however so far the value has always been 0 for usable fonts (i.e. fixed fonts) */
        szY = (textmetric.tmHeight <= 0) ? 1 : textmetric.tmHeight;
    }
    else
    {
        /* cannot get here first time round due to init in TTstart so ttlogfont is initialised */
        memcpy(&logfont,&ttlogfont,sizeof(LOGFONT));
        szX = eCellMetrics.cell.sizeX;
        szY = eCellMetrics.cell.sizeY;
        textmetric.tmPitchAndFamily = ttPitchFam;
        mode = -1;
    }
    
    if((n & 0x04) == 0)
    {
        /* Save the values in $result */
        sprintf((char *) resultStr,"|%c|%d|%d|%d|%d|%d|%d|%s|",'0'+(textmetric.tmPitchAndFamily&7),logfont.lfCharSet,
                (int) (logfont.lfWeight/100),szX,szY,(int) logfont.lfWidth,(int) logfont.lfHeight,logfont.lfFaceName);
    }
    if((n & 0x02) || (mode < 0))
    {
        /* Don't apply the font so restore old font and get out */
        if(oldFont != NULL)
            SelectObject(hDC,oldFont);
        return meTRUE;
    }
    
    /* Delete the exisiting font */
    if(eCellMetrics.fontdef[0] != NULL)
    {
        int ii;
        /* Iterate over the font face table and locate duplicated fonts */
        for(ii = 1; ii < meFONT_MAX; ii++)
        {
            if((eCellMetrics.fontdef[ii] != NULL) && (eCellMetrics.fontdef[ii] != eCellMetrics.fontdef[0]))
                /* Delete the font container */
                DeleteObject(eCellMetrics.fontdef[ii]);
            eCellMetrics.fontdef[ii] = NULL;
        }
        DeleteObject(eCellMetrics.fontdef[0]);
    }
    
    ttPitchFam = textmetric.tmPitchAndFamily;
    eCellMetrics.fontdef[0] = newFont;
    
    /* Build up the cell metrics */
    eCellMetrics.cell.sizeX = szX;
    eCellMetrics.cell.sizeY = szY;
    eCellMetrics.cell.midX = szX / 2;
    eCellMetrics.cell.midY = szY / 2;
    
    /* Store logfont into the ttlogfont for font style and language char set changes  */
    memcpy(&ttlogfont,&logfont,sizeof(LOGFONT));
    GetTextFace(hDC,sizeof(ttlogfont.lfFaceName),ttlogfont.lfFaceName);
    if(ttlogfont.lfHeight <= 0)
    {
        ttlogfont.lfHeight = eCellMetrics.cell.sizeY;
        ttlogfont.lfWidth = eCellMetrics.cell.sizeX;
    }
    /* Release the window */
    ReleaseDC(baseHwnd,hDC);
    
    meFrameLoopBegin();
    
    meFrameLoopContinue(loopFrame->flags & meFRAME_HIDDEN);
    
#if (MEOPT_FRAME == 0)
    if(loopFrame != NULL)
#endif
        meFrameSetWindowSize(loopFrame);
    
    meFrameLoopEnd();
    
    return meTRUE;
}
#endif /* _ME_WINDOW */

/* changeFont
 * Change the size of the font.
 *
 * change-font <charSet> <name> <weight> <width> <height>
 *
 *
 * <charSet> - The type of character set required.
 *
 *             0 = OEM (or bitmapped)
 *             1 = ANSI (True Type etc).
 *
 * <font>    - The name of the font. Max of 32  characters.  Select Fixed mono
 *             fonts only. Proportional  fonts may be specified but the cursor
 *             will not align with the characters on the screen.
 *
 *             "default" may be specified  and the sytem  default OEM font. No
 *             other arguments are required when specified.
 *
 *             Note that  "Courier  New" is not  actually a fixed mono font as
 *             might be expected.
 *
 * <weight>  - The weight of the font. The values are defined as:-
 *
 *             0 = Don't care (Automatically selected).
 *             1 = Thin
 *             2 = Extra Light
 *             3 = Light
 *             4 = Normal
 *             5 = Medium
 *             6 = Semi-Bold
 *             7 = Bold
 *             8 = Extra-Bold
 *             9 = Heavy
 *
 *             Note  that you may  request  a weight  and it is not  honoured.
 *             Typically 4 and 7 are honoured by most font  definitions.  4 is
 *             typically used.
 *
 * <width>   - The width of the font.
 *
 *             Specifies the average width, in logical units, of characters in
 *             the  requested  font. If this  value is zero,  the font  mapper
 *             chooses a "closest  match" value. The "closest  match" value is
 *             determined by comparing the absolute  values of the  difference
 *             between the current  device's  aspect  ratio and the  digitized
 *             aspect ratio of available fonts.
 *
 *             Note  that if the width is  specified  as zero then the  height
 *             should  be  specified  and  the  width  will  be  automatically
 *             selected.
 *
 * <height>  - The height of the font.
 *
 *             Specifies  the  desired   height,  in  logical  units,  of  the
 *             requested  font's  character  cell or character. (The character
 *             height  value is the  character  cell  height  value  minus the
 *             internal-leading  value.) If this  value is greater  than zero,
 *             the font mapper  matches it against  available  character  cell
 *             height  values; if this value is zero, the font  mapper  uses a
 *             default  height  value  when it  searches  for a match; if this
 *             value is less than zero, the font  mapper  matches  it  against
 *             available character height values.
 *
 *             Note: as with  the  weight  the  width  and  height  may not be
 *             honoured if the font cannot support the specified  width/height
 *             in which  case the  closest  matching  height is  automatically
 *             selected
 */

#define FONTBUFSIZ 33                   /* Size of buffer is 33
Max font name is 32 chars */
int
changeFont(int f, int n)
{
    int  fontType=0;                      /* Type of font 0=ANSI,255=OEM etc */
#ifdef _ME_WINDOW
    meUByte fontName[FONTBUFSIZ];       /* Input font name buffer */
    int  fontWeight=0;                  /* Weight of font (0-9) */
    int  fontHeight=0;                  /* Height of font */
    int  fontWidth=0;                   /* Width of font */
    
    if(!f)
        n = 0x09;
#endif
#ifdef _ME_CONSOLE
#ifdef _ME_WINDOW
    if(meSystemCfg & meSYSTEM_CONSOLE)
#endif /* _ME_WINDOW */
    {
        if(n & 0x01)
        {
            meUByte buff[FONTBUFSIZ];            /* Input buffer */
            if((meGetString((meUByte *) "Font Type [ANSI=0,OEM=255]", 0, 0, buff, FONTBUFSIZ) <= 0) ||
               ((fontType = meAtoi(buff)) < 0) || (fontType > 255))
                return meFALSE;
            if((n & 0x02) == 0)
                ttlogfont.lfCharSet = (meUByte) fontType;
        }
        else
            fontType = ttlogfont.lfCharSet;
        if((n & 0x04) == 0)
            sprintf((char *) resultStr,"||%d|||||||",fontType);
        return meTRUE;
    }
#endif
        
#ifdef _ME_WINDOW
    fontName[0] = '\0';
    if((n & 0x09) == 1)
    {
        meUByte buff[FONTBUFSIZ];
        
        if((meGetString((meUByte *) "Font Type [ANSI=0,OEM=255]", 0, 0, buff, FONTBUFSIZ) <= 0) ||
           ((fontType = meAtoi(buff)) < 0) || (fontType > 255) ||
           (meGetString((meUByte *) "Font Name ['' for default]", 0, 0, fontName, FONTBUFSIZ) == meABORT) ||
           ((fontName[0] != '\0') &&
            ((meGetString((meUByte *) "Font Weight [1-9; 0=don't care]", 0, 0, buff, FONTBUFSIZ) <= 0) ||
             ((fontWeight = meAtoi(buff)) < 0) || (fontWeight > 9) ||
             (meGetString((meUByte *) "Font Width", 0, 0, buff, FONTBUFSIZ) <= 0) ||
             ((fontWidth = meAtoi(buff)) < 0) ||
             (meGetString((meUByte *) "Font Height", 0, 0, buff, FONTBUFSIZ) <= 0))))
            return meFALSE;
        fontHeight = meAtoi(buff);
    }
    return TTchangeFont(n,fontType,fontName,fontWeight,fontHeight,fontWidth);
#endif /* _ME_WINDOW */
}
#undef FONTBUFSIZ

/*
 * meFrameHideCursor - hide the cursor
 */
void
meFrameHideCursor(meFrame *frame)
{
    if((frame->cursorRow <= frame->depth) && (frame->cursorColumn < frame->width))
    {
#ifdef _ME_CONSOLE
#ifdef _ME_WINDOW
        if(meSystemCfg & meSYSTEM_CONSOLE)
#endif /* _ME_WINDOW */
        {
            meFrameLine *flp;                    /* Frame store line pointer */
            meScheme schm;                       /* Current colour */
            meUByte cc;                          /* Current cchar  */
            WORD dcol;
            
            flp  = frame->store + frame->cursorRow;
            cc   = flp->text[frame->cursorColumn];          /* Get char under cursor */
            schm = flp->scheme[frame->cursorColumn];        /* Get colour under cursor */
            
            dcol = (WORD) TTschemeSet(schm);
            ConsoleDrawString(&cc, dcol, frame->cursorColumn, frame->cursorRow, 1);
        }
#ifdef _ME_WINDOW
        else if(!meFrameGetWinPaintAll(frame))
#endif /* _ME_WINDOW */
#else
        if(!meFrameGetWinPaintAll(frame))
#endif /* _ME_CONSOLE */
#ifdef _ME_WINDOW
        {
            RECT rect;                  /* Area of screen to update */
            int startCol = frame->cursorColumn;
            meFrameLine *flp;
            
            /* If the cursor preceeeds an italic character then we need to fix
             * the previous character. This is done by ME drawing the previous
             * char but not invalidating the area so that only the overhang into
             * the next char is drawn.
             */
            if((startCol > 0) &&
               (meSchemeGetStyle((flp = frame->store+frame->cursorRow)->scheme[startCol-1]) & meSTYLE_ITALIC) &&
               (flp->text[startCol-1] != ' '))
            {
                /* Back up, the previous character will be corrupted. */
                startCol--;
            }
            
            if(meFrameGetWinPaintStartCol(frame)[frame->cursorRow] > startCol)
                meFrameGetWinPaintStartCol(frame)[frame->cursorRow] = startCol;
            if(meFrameGetWinPaintEndCol(frame)[frame->cursorRow] <= frame->cursorColumn)
                meFrameGetWinPaintEndCol(frame)[frame->cursorRow] = frame->cursorColumn+1;
            /* Set up the area on the client window to be modified
               and signal that the client is about to be updated */
            /* SWP 20040108 - XP ClearType smooth edge font rendering seems
             * to spill the bg out onto the next character by one pixel so
             * to avoid dropping vertical lines add 1 to the right edge */
            rect.left   = eCellMetrics.cellColPos[frame->cursorColumn];
            rect.right  = eCellMetrics.cellColPos[frame->cursorColumn+1] + 1;
            rect.top    = eCellMetrics.cellRowPos[frame->cursorRow];
            rect.bottom = eCellMetrics.cellRowPos[frame->cursorRow+1];
            InvalidateRect(meFrameGetWinHandle(frame), &rect, meFALSE);
        }
#endif /* _ME_WINDOW */
    }
}

/*
 * meFrameShowCursor - show the cursor
 */
void
meFrameShowCursor(meFrame *frame)
{
    if((frame->cursorRow <= frame->depth) && (frame->cursorColumn < frame->width))
    {
#ifdef _ME_CONSOLE
#ifdef _ME_WINDOW
        if(meSystemCfg & meSYSTEM_CONSOLE)
#endif /* _ME_WINDOW */
        {
            meFrameLine *flp;                    /* Frame store line pointer */
            meScheme schm;                       /* Current colour */
            meUByte cc;                          /* Current cchar  */
            WORD dcol;
            
            flp  = frame->store + frame->cursorRow;
            cc   = flp->text[frame->cursorColumn];          /* Get char under cursor */
            schm = flp->scheme[frame->cursorColumn];        /* Get colour under cursor */
            
            dcol = (WORD) TTcolorSet(colTable[meStyleGetBColor(meSchemeGetStyle(schm))],
                                     colTable[cursorColor]);
            
            ConsoleDrawString(&cc, dcol, frame->cursorColumn, frame->cursorRow, 1);
        }
#ifdef _ME_WINDOW
        else if(!meFrameGetWinPaintAll(frame))
#endif /* _ME_WINDOW */
#else
        if(!meFrameGetWinPaintAll(frame))
#endif /* _ME_CONSOLE */
#ifdef _ME_WINDOW
        {
            RECT rect;                  /* Area of screen to update */
            int startCol = frame->cursorColumn;
            meFrameLine *flp;
            
            /* If the cursor preceeeds an italic character then we need to fix
             * the previous character. This is done by ME drawing the previous
             * char but not invalidating the area so that only the overhang into
             * the next char is drawn.
             */
            if((startCol > 0) && 
               (meSchemeGetStyle((flp = frame->store+frame->cursorRow)->scheme[startCol-1]) & meSTYLE_ITALIC) &&
               (flp->text[startCol-1] != ' '))
            {
                /* Back up, the previous character will be corrupted. */
                startCol--;
            }
            
            if(meFrameGetWinPaintStartCol(frame)[frame->cursorRow] > startCol)
                meFrameGetWinPaintStartCol(frame)[frame->cursorRow] = startCol;
            if(meFrameGetWinPaintEndCol(frame)[frame->cursorRow] <= frame->cursorColumn)
                meFrameGetWinPaintEndCol(frame)[frame->cursorRow] = frame->cursorColumn+1;
            /* Set up the area on the client window to be modified
               and signal that the client is about to be updated */
            /* SWP 20040108 - XP ClearType smooth edge font fix - see first comment */
            rect.left   = eCellMetrics.cellColPos[frame->cursorColumn];
            rect.right  = eCellMetrics.cellColPos[frame->cursorColumn+1] + 1;
            rect.top    = eCellMetrics.cellRowPos[frame->cursorRow];
            rect.bottom = eCellMetrics.cellRowPos[frame->cursorRow+1];
            InvalidateRect(meFrameGetWinHandle(frame), &rect, meFALSE);
        }
#endif /* _ME_WINDOW */
    }
}

/*
 * TTcolour
 * Change the foreground and background colours.
 */
void
TTcolour(int fcol, int bcol)
{
    if(ttfcol != fcol)
        ttfcol = meFColorCheck(fcol);
    if(ttbcol != bcol)
        ttbcol = meBColorCheck(bcol);
}

/*
 * meGetMessage
 * Get a new message from the windows message queue. Handle
 * any streams immediatly.
 */
static int
meGetMessage(MSG *msg, int mode)
{
#if MEOPT_IPIPES || (defined _ME_CONSOLE)
    if(
#ifdef _ME_CONSOLE
#ifdef _ME_WINDOW
        (meSystemCfg & meSYSTEM_CONSOLE)
#else
        1
#endif /* _ME_WINDOW */
#if MEOPT_IPIPES
        ||
#endif
#endif /* _ME_CONSOLE */
#if MEOPT_IPIPES
        noIpipes
#endif
        )
    {
        static HANDLE *hTable;
        static int hTableSize=0;
        int ii, jj;
#if MEOPT_IPIPES
        meIPipe *ipipe, *pp;
        ii = noIpipes;
#else
        ii = 0;
#endif
#ifdef _ME_CONSOLE
#ifdef _ME_WINDOW
        if(meSystemCfg & meSYSTEM_CONSOLE)
#endif /* _ME_WINDOW */
            ii++;
#endif
        if(ii > hTableSize)
        {
            hTableSize = ii+1;
            meNullFree(hTable);
            if((hTable = meMalloc(hTableSize*sizeof(HANDLE))) == NULL)
            {
                /* if this fails we really are in big trouble, time to get out! */
                meDie();
            }
        }
        for(;;)
        {
#if MEOPT_IPIPES
            /* Loop round first time to read anything available and close
             * those processes that have finished
             */
            jj = 0;
            ipipe = ipipes;
            while(ipipe != NULL)
            {
                DWORD doRead;
                pp = ipipe->next;
#if MEOPT_CLIENTSERVER
                if(ipipe->pid == 0)
                {
                    if(ttServerCheck && TTcheckClientServer())
                    {
                        ipipeRead(ipipe);
                        jj = 1;
                    }
                }
                else
#endif
                {
                    if(ipipe->pid > 0)
                    {
                        if(ipipe->thread != NULL)
                        {
                            if((doRead = (WaitForSingleObject(ipipe->childActive,0) == WAIT_OBJECT_0)))
                                ResetEvent(ipipe->childActive);
                        }
                        else if(PeekNamedPipe(ipipe->rfd, (LPVOID) NULL, (DWORD) 0,
                                              (LPDWORD) NULL, &doRead, (LPDWORD) NULL) == meFALSE)
                        {
                            /* If peek failed, wipe our hands of it. Close the process */
                            GetExitCodeProcess(ipipe->process,(LPDWORD) &(ipipe->exitCode));
                            CloseHandle(ipipe->process);
                            ipipe->pid = -4;
                        }
                    }
                    if((ipipe->pid < 0) || doRead)
                    {
                        ipipeRead(ipipe);
                        jj = 1;
                    }
#ifndef _WIN32_WINNT
                    else if((platformId != VER_PLATFORM_WIN32_NT) &&
                            /* ipipe->bp->windowCount &&*/
                            (!GetExitCodeProcess(ipipe->process,&doRead) || (doRead != STILL_ACTIVE)))
                    {
                        /* Win95 fails to spot the exit state some times, this fixes it */
                        GetExitCodeProcess(ipipe->process,(LPDWORD) &ipipe->exitCode);
                        CloseHandle(ipipe->process);
                        ipipe->pid = -4;
                        pp = ipipe;
                    }
#endif
                }
                ipipe = pp;
            }
#endif
            if(mode && jj)
                return meFALSE;
            
            /* Now simply loop through creating a wait object list of all remaining
             * processes & console.
             */
            ii = 0;
#ifdef _ME_CONSOLE
#ifdef _ME_WINDOW
            if(meSystemCfg & meSYSTEM_CONSOLE)
#endif /* _ME_WINDOW */
            {
                if(hInput != INVALID_HANDLE_VALUE)
                    hTable[ii++] = hInput;
                else if(!mode)
                    /* stdin has gone and we're only interested in user input - exit */
                    meDie();
            }
#endif
#if MEOPT_IPIPES
            ipipe = ipipes;
            while(ipipe != NULL)
            {
#if MEOPT_CLIENTSERVER
                if(ipipe->pid > 0)
#endif
                {
                    if(ipipe->thread != NULL)
                        hTable[ii++] = ipipe->childActive;
                    else
                        hTable[ii++] = ipipe->rfd;
                }
                ipipe = ipipe->next;
            }
#endif
            /* Wait for either user or process activity */
            jj = MsgWaitForMultipleObjects(ii,hTable,meFALSE,INFINITE,QS_ALLINPUT) - WAIT_OBJECT_0;
#ifdef _ME_CONSOLE
#ifdef _ME_WINDOW
            if(meSystemCfg & meSYSTEM_CONSOLE)
#endif /* _ME_WINDOW */
            {
                if(PeekMessage(msg, meHWndNull, WM_TIMER, WM_TIMER, PM_REMOVE) != meFALSE)
                    return meTRUE;
                if(((jj < 0) || (jj >= ii) || ((jj == 0) && (hInput != INVALID_HANDLE_VALUE))) &&
                   meGetConsoleMessage(msg,mode))
                    return meTRUE;
            }
#ifdef _ME_WINDOW
            else
#endif /* _ME_WINDOW */
#endif
#ifdef _ME_WINDOW
                if((jj < 0) || (jj >= ii))
                /* User activity, go get it! */
                break;
#endif /* _ME_WINDOW */
        }
    }
#endif /* MEOPT_IPIPES & _ME_CONSOLE */
    /* Note: console versions cannot get here */
    if(GetMessage(msg,              /* address of structure with message */
                  meHWndNull,       /* handle of window */
                  0,                /* first message */
                  0) <= 0)          /* last message */
        meDie();
    return meTRUE;
}

/*
 * TTgetc
 * Wait for a character. If a character does not arrive then
 * suspend awaiting a character from the keyboard.
 */
void
TTwaitForChar(void)
{
#if MEOPT_MOUSE
    meUShort mc;
    meUInt arg;
    /* If no keys left then if theres currently no mouse timer and
     * theres a button press (No mouse-time key) then check for a
     * time-mouse-? key, if found add a timer start a mouse checking
     */
    if(!isTimerSet(MOUSE_TIMER_ID) &&
       ((mc=(mouseState & MOUSE_STATE_BUTTONS)) != 0))
    {
        mc = ME_SPECIAL | ttmodif | (SKEY_mouse_time + mouseKeys[mc]);
        /* mouse-time bound ?? */
        if((!TTallKeys && (decode_key(mc,&arg) != -1)) || (TTallKeys & 0x2))
        {
            /* Start a timer and move to timed state 1 */
            /* Start a new timer to clock in at 'delay' intervals */
            /* printf("Setting mouse timer for delay %d\n",delayTime);*/
            timerSet(MOUSE_TIMER_ID,-1,delayTime);
        }
    }
#endif
#ifdef _ME_WIN32_FULL_DEBUG
    /* Do heap walk before getting next char */
    _CrtCheckMemory();
#endif
#if MEOPT_CALLBACK
    /* IDLE TIME: Check the idle time events */
    if(kbdmode == meIDLE)
        doIdlePickEvent();         /* Check the idle event */
#endif
    
    /* Pend for messages */
    for(;;)
    {
        MSG msg;                            /* Message buffer */
        
        handleTimerExpired();
        if(TTahead())
            break;
        /* TTahead can process the timers so we need to recheck the timers
         * before we wait for the next message */
        handleTimerExpired();
        
#if MEOPT_MWFRAME
        /* if the user has changed the window focus using the OS
         * but ME can swap to this frame because there is an active frame
         * then give a warning */
        if((frameFocus != NULL) && (frameFocus != frameCur))
        {
            meUByte scheme=(globScheme/meSCHEME_STYLES);
            meFrame *fc=frameCur;
            frameCur = frameFocus;
            pokeScreen(0x10,frameCur->depth,(frameCur->width >> 1)-5,&scheme,
                       (meUByte *) "[NOT FOCUS]");
            frameCur = fc;
        }
#endif
        
        if(sgarbf == meTRUE)
        {
            update(meFALSE);
            mlerase(MWCLEXEC);
        }
        
        /* Suspend until there is another message to process. */
        TTflush();                 /* Make sure the screen is up-to-date */
        meGetMessage(&msg,0);      /* Suspend for a message */
        
        /* Closing down the system */
        if(msg.message == WM_CLOSE)
            WinExit(0);
        /* Timer - handle short and sweet here */
        else if(msg.message == WM_TIMER)
        {
#ifdef _ME_CONSOLE
#ifdef _ME_WINDOW
            if(meSystemCfg & meSYSTEM_CONSOLE)
#endif /* _ME_WINDOW */
                timerAlarm(msg.wParam);
#ifdef _ME_WINDOW
            else
#endif /* _ME_WINDOW */
#endif
#ifdef _ME_WINDOW
                if(msg.wParam < NUM_TIMERS)
            {
                meTimerState[msg.wParam] = (meTimerState[msg.wParam] & ~TIMER_SET) | TIMER_EXPIRED;
                continue;
            }
#endif /* _ME_WINDOW */
        }
#if MEOPT_MOUSE
        /* Mouse movement or button press */
        else if((msg.message >= WM_MOUSEFIRST) && (msg.message <= WM_MOUSELAST))
        {
            if(WinMouse(msg.hwnd, msg.message, msg.wParam, msg.lParam))
                continue;
        }
#endif
        /* Keyboard message */
        else if((msg.message >= WM_KEYFIRST) && (msg.message <= WM_KEYLAST))
        {
            if(!(meSystemCfg & meSYSTEM_CONSOLE))
                TranslateMessage(&msg);    /* Translate keyboard characters */
            if(WinKeyboard(msg.hwnd, msg.message, msg.wParam, msg.lParam))
                continue;
        }
        
#ifdef _ME_WINDOW
        /* Only get here if we have not handled the message
         * post to the dispatcher if not a console. */
#ifdef _ME_CONSOLE
        if(!(meSystemCfg & meSYSTEM_CONSOLE))
#endif
            meMessageHandler(&msg);
#endif /* _ME_WINDOW */
    }
}

#ifdef _ME_WINDOW
/*
 * TTputs
 * Put a string to the screen simply by invalidating the region. The
 * display functions should have updated the frameCur->store which is what
 * is used for rendering so it is only necessary to invalidate the
 * screen region to invoke a paint operation.
 */
void
TTputs(int row, int col, int len)
{
    if(!meFrameGetWinPaintAll(frameCur))
    {
        RECT rect;                          /* Area of screen to update */
        
        if(meFrameGetWinPaintStartCol(frameCur)[row] > col)
            meFrameGetWinPaintStartCol(frameCur)[row] = col;
        if(meFrameGetWinPaintEndCol(frameCur)[row] < (col+len))
            meFrameGetWinPaintEndCol(frameCur)[row] = col+len;
        
        /* Set up the area on the client window to be modified
           and signal that the client is about to be updated */
        /* SWP 20040108 - XP ClearType smooth edge font fix - see first comment */
        rect.left   = eCellMetrics.cellColPos[col];
        rect.right  = eCellMetrics.cellColPos[col+len] + 1;
        rect.top    = eCellMetrics.cellRowPos[row];
        rect.bottom = eCellMetrics.cellRowPos[row+1];
        InvalidateRect(meFrameGetWinHandle(frameCur), &rect, meFALSE);
    }
}

/*
 * TTapplyArea
 * Invalidate an area of the screen that has been accumulated by
 * the display system.
 */
void
TTapplyArea(void)
{
    if((ttRect.right >= 0) && !meFrameGetWinPaintAll(frameCur))
    {
        int row;
        for(row=ttRect.top ; row < ttRect.bottom ; row++)
        {
            if(meFrameGetWinPaintStartCol(frameCur)[row] > (meShort) ttRect.left)
                meFrameGetWinPaintStartCol(frameCur)[row] = (meShort) ttRect.left;
            if(meFrameGetWinPaintEndCol(frameCur)[row] < (meShort) ttRect.right)
                meFrameGetWinPaintEndCol(frameCur)[row] = (meShort) ttRect.right;
        }
        /* SWP 20040108 - XP ClearType smooth edge font fix - see first comment */
        ttRect.left   = eCellMetrics.cellColPos[ttRect.left];
        ttRect.right  = eCellMetrics.cellColPos[ttRect.right] + 1;
        ttRect.top    = eCellMetrics.cellRowPos[ttRect.top];
        ttRect.bottom = eCellMetrics.cellRowPos[ttRect.bottom];
        InvalidateRect(meFrameGetWinHandle(frameCur), &ttRect, meFALSE);
    }
}

void
meFrameTermMakeCur(meFrame *frame)
{
#ifdef _ME_CONSOLE
    if(!(meSystemCfg & meSYSTEM_CONSOLE))
#endif /* _ME_CONSOLE */
    {
        DWORD ThreadID1, ThreadID2;
        HWND cwHwnd;
        if(IsIconic(meFrameGetWinHandle(frame)))
            ShowWindow(meFrameGetWinHandle(frame), SW_SHOWNORMAL);
        else if((cwHwnd = GetForegroundWindow()) != meFrameGetWinHandle(frame))
        {
            /* To get the require permissions to set the foreground
             * window ME needs to attach to the current thread */
            if((cwHwnd != NULL) &&
               ((ThreadID1 = GetWindowThreadProcessId(cwHwnd, NULL)),
                (ThreadID2 = GetWindowThreadProcessId(meFrameGetWinHandle(frame), NULL)),
                (ThreadID1 != ThreadID2)))
            {
                AttachThreadInput(ThreadID1, ThreadID2, TRUE);
                SetForegroundWindow(meFrameGetWinHandle(frame));
                AttachThreadInput(ThreadID1, ThreadID2, FALSE);
            }
            else
                SetForegroundWindow(meFrameGetWinHandle(frame));
        }
    }
}

void
meFrameTermFree(meFrame *frame, meFrame *sibling)
{
#ifdef _ME_CONSOLE
    if(!(meSystemCfg & meSYSTEM_CONSOLE))
#endif /* _ME_CONSOLE */
    {
        /* if another internal frame exists then theres nothing to do */
        if(sibling == NULL)
        {
            DestroyWindow(meFrameGetWinHandle(frame));
            free(meFrameGetWinPaintStartCol(frame));
            free(frame->termData);
        }
    }
}

#endif /* _ME_WINDOW */

int
meFrameTermInit(meFrame *frame, meFrame *sibling)
{
    if(sibling == NULL)
    {
#ifdef _ME_CONSOLE
#ifdef _ME_WINDOW
        if(meSystemCfg & meSYSTEM_CONSOLE)
#endif /* _ME_WINDOW */
            meFrameSetWindowSize(frame);
#ifdef _ME_WINDOW
        else
#endif /* _ME_WINDOW */
#endif /* _ME_CONSOLE */
#ifdef _ME_WINDOW
        {
            meFrameData *frameData;
            
            if((frameData = meMalloc(sizeof(meFrameData))) == NULL)
                return meFALSE;
            memset(frameData,0,sizeof(meFrameData));
            frame->termData = frameData;
            frameData->hwnd = CreateWindow("MicroEmacsClass",
                                           ME_FULLNAME " " meVERSION,
                                           WS_OVERLAPPEDWINDOW,  /* No scroll bars */
                                           CW_USEDEFAULT,
                                           CW_USEDEFAULT,
                                           CW_USEDEFAULT,
                                           CW_USEDEFAULT,
                                           NULL,
                                           NULL,
                                           ttInstance,
                                           NULL);
            meFrameSetWindowSize(frame);
            ShowWindow(frameData->hwnd,ttshowState);     /* Create the window */
            UpdateWindow(frameData->hwnd);               /* Show it off - ready for errors */
        }
#endif /* _ME_WINDOW */
    }
#ifdef _ME_WINDOW
    else
        /* internal frame, just copy the window handler */
        frame->termData = sibling->termData;
#endif /* _ME_WINDOW */
    return meTRUE;
}

/*
 * TTstart
 * Start the EMACS window environment. This is called once at the start
 * when it creates the Window and determines the initial text metrics.
 */
int
TTstart(void)
{
#ifdef _ME_CONSOLE
#ifdef _ME_WINDOW
    if(meSystemCfg & meSYSTEM_CONSOLE)
#else
    meSystemCfg |= meSYSTEM_CONSOLE;
#endif /* _ME_WINDOW */
    {
        CONSOLE_SCREEN_BUFFER_INFO Console;
        COORD coord;
        
        /* console can't support fonts and only has XANSI */
        meSYSTEM_MASK &= ~meSYSTEM_FONTS;
        meSystemCfg = (meSystemCfg & ~(meSYSTEM_FONTS|meSYSTEM_RGBCOLOR)) | (meSYSTEM_ANSICOLOR|meSYSTEM_XANSICOLOR);
        
        /* This will allocate a console if started from
         * the windows NT program manager. */
        AllocConsole();
        
        /* Save the titlebar of the window so we can
         * restore it when we leave. */
        GetConsoleTitle(chConsoleTitle, sizeof(chConsoleTitle));
        
        /* Set Window Title to MicroEMACS */
        /*        SetConsoleTitle();*/
        
        /* Get our standard handles */
        hInput = GetStdHandle(STD_INPUT_HANDLE);
        hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
        
        /* get a ptr to the output screen buffer */
        if(GetConsoleScreenBufferInfo(hOutput,&Console))
        {
            OldConsoleSize.X = Console.dwSize.X;
            OldConsoleSize.Y = Console.dwSize.Y;
            
            /* let MicroEMACS know our starting screen size */
            /* this should be the window size, not the buffer size
             * as this needs the scroll-bar to use */
            TTwidthDefault = Console.srWindow.Right-Console.srWindow.Left+1;
            TTdepthDefault = Console.srWindow.Bottom-Console.srWindow.Top+1;
        }
#if MEOPT_EXTENDED
        else if(alarmState & meALARM_PIPED)
        {
            /* can't get the console so can't get the size but as running in -p piped mode set to a default 80x50 */
            TTwidthDefault = OldConsoleSize.X = 80;
            TTdepthDefault = OldConsoleSize.Y = 50;
        }
#endif
        else
            return meFALSE;
        
        if(TTwidthDefault < 8)
            TTwidthDefault = 8;
        if(TTdepthDefault < 4)
            TTdepthDefault = 4;
        
#if MEOPT_EXTENDED
        if((alarmState & meALARM_PIPED) == 0)
#endif
        {
            /* now fix the window buffer size to this window size to
             * get rid of the horrid scroll bars! */
            coord.X = TTwidthDefault;
            coord.Y = TTdepthDefault;
            SetConsoleScreenBufferSize(hOutput,coord);
        }
        consolePaintArea.Right = consolePaintArea.Bottom = 0;
        consolePaintArea.Left = consolePaintArea.Top = (SHORT) 0x7fff;
        
#if MEOPT_MOUSE
        /* we always have a mouse under NT */
        /*        mexist = GetNumberOfConsoleMouseButtons(&nbuttons);*/
        /* the mouse is always in the frame when we get a mouse event */
        mouseInFrame = 1;
#endif
        
        /* save the original console mode to restore on exit */
        GetConsoleMode(hInput,&OldConsoleMode);
        OldConsoleMode |= ENABLE_EXTENDED_FLAGS;
        
        /* and reset this to what MicroEMACS needs, note no ENABLE_QUICK_EDIT_MODE which requires ENABLE_EXTENDED_FLAGS to set/clear */
        ConsoleMode = ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS;
        SetConsoleMode(hInput,ConsoleMode);
        
        /* Set emergency quit handler routine */
        SetConsoleCtrlHandler(ConsoleHandlerRoutine, meTRUE);
        
#if MEOPT_EXTENDED
        if(!(alarmState & meALARM_PIPED))
#endif
        {
            /* so move the cursor to a fixed less annoying position */
            coord.X = TTwidthDefault-1;
            coord.Y = 0;
            SetConsoleCursorPosition(hOutput,coord);
        }
        /* make the default charSet OEM for cut-n-paste to clipboard */
        ttlogfont.lfCharSet = 255;
    }
#ifdef _ME_WINDOW
    else
#endif /* _ME_WINDOW */
#endif /* _ME_CONSOLE */
#ifdef _ME_WINDOW
    {
#ifdef _ME_CONSOLE
        /* If in window mode, then bin the console we were given */
        FreeConsole();
#endif /* _ME_CONSOLE */
        baseHwnd = CreateWindow("MicroEmacsClass",
                                ME_FULLNAME " " meVERSION,
                                WS_DISABLED,
                                CW_USEDEFAULT,CW_USEDEFAULT,
                                CW_USEDEFAULT,CW_USEDEFAULT,
                                NULL,
                                NULL,
                                ttInstance,
                                NULL);
        
        if(!baseHwnd)
            return meFALSE;
        TTchangeFont(5,0,NULL,0,0,0);
    }
#endif /* _ME_WINDOW */

    /* Create the default colours */
    /* Construct the palette. Add two colours; black and white. */
    TTaddColor(meCOLOR_FDEFAULT,255, 255, 255);   /* White */
    TTaddColor(meCOLOR_BDEFAULT,  0,   0,   0);   /* Black */
    TTcolour(meCOLOR_FDEFAULT,meCOLOR_BDEFAULT);  /* Default colours - none created yet */

    /* To be continued in meFrameTermInit after the display memory
     * has been initialised */
    return meTRUE;
}

/*
 * TTahead()
 * Typeahead. Search for any keyboard or mouse messages on the input
 * queue. Process them and return the type-ahead buffer length.
 */

int
TTahead(void)
{
    MSG msg;                            /* Local message buffer */
    
    /* Peek all Keyboard and Mouse messages until there are no another
       message to process. */
#ifdef _ME_CONSOLE
#ifdef _ME_WINDOW
    if(meSystemCfg & meSYSTEM_CONSOLE)
#endif /* _ME_WINDOW */
    {
        DWORD dwCount;
        INPUT_RECORD ir;
        /* If TTnoKeys == KEYBUFSIZ then one of 2 things has happend,
         * 1) The user is running a big command (execute macro 3 billion times) and
         *    has carried on typing 128+ chars (without pressing the abort key)
         * 2) Pasting in a dos window using the window paste results in lots of keys
         *    all at once, the clipboard must contain 128+ chars.
         * (1) is unlikely compared to the (2) so rather than lose the extra chars,
         * keep them there until there enough room in the input key buffer to store
         * them. */
        while(TTnoKeys != KEYBUFSIZ)
        {
            if((hInput != INVALID_HANDLE_VALUE) &&
               (PeekConsoleInput(hInput, &ir, 1, &dwCount) != meFALSE) && (dwCount > 0))
            {
                if(meGetConsoleMessage(&msg,0))
                {
                    /* Keyboard message */
                    if((msg.message >= WM_KEYFIRST) && (msg.message <= WM_KEYLAST))
                        WinKeyboard(msg.hwnd, msg.message, msg.wParam, msg.lParam);
#if MEOPT_MOUSE
                    else if((msg.message >= WM_MOUSEFIRST) && (msg.message <= WM_MOUSELAST))
                        WinMouse(msg.hwnd, msg.message, msg.wParam, msg.lParam);
#endif
                }
            }
            else if(PeekMessage(&msg, meHWndNull, WM_TIMER, WM_TIMER, PM_REMOVE) != meFALSE)
                timerAlarm(msg.wParam);
            else
                break;
        }
    }
#ifdef _ME_WINDOW
    else
#endif /* _ME_WINDOW */
#endif /* _ME_CONSOLE */
#ifdef _ME_WINDOW
    {
        for(;;)
        {
            if(PeekMessage(&msg, meHWndNull, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE) != meFALSE)
            {
                TranslateMessage(&msg);
                if(!WinKeyboard(msg.hwnd, msg.message, msg.wParam, msg.lParam))
                    meMessageHandler(&msg);
            }
#if MEOPT_MOUSE
            /* Check out the mouse. */
            else if(PeekMessage(&msg, meHWndNull, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE) != meFALSE)
            {
                if(WinMouse(msg.hwnd, msg.message, msg.wParam, msg.lParam) == meFALSE)
                    meMessageHandler(&msg);
            }
#endif
            else if(PeekMessage(&msg, meHWndNull, WM_TIMER, WM_TIMER, PM_REMOVE) != meFALSE)
            {
                /* Check out the timers */
                if(msg.wParam < NUM_TIMERS)
                    meTimerState[msg.wParam] = (meTimerState[msg.wParam] & ~TIMER_SET) | TIMER_EXPIRED;
                else
                    meMessageHandler(&msg);
            }
            else
                break;
        }
    }
#endif /* _ME_WINDOW */
    
    /* don't process the timers if we have a key waiting!
     * This is because the timers can generate a lot of timer
     * keys, filling up the input buffer - these are not wanted.
     * By not processing, we leave them there until we need them.
     */
    if(TTnoKeys)
        return TTnoKeys;
#if MEOPT_MOUSE
    /* If the mouse alarm has returned Check the mouse */
    if(isTimerExpired(MOUSE_TIMER_ID))
    {
        meUShort mc;
        meUInt arg;
        
        timerClearExpired(MOUSE_TIMER_ID);
        mc = ME_SPECIAL | ttmodif | (SKEY_mouse_time + mouseKeys[mouseState&MOUSE_STATE_BUTTONS]);
        /* mouse-move bound ?? */
        if((!TTallKeys && (decode_key(mc,&arg) != -1)) || (TTallKeys & 0x2))
        {
            /* Timer expired and still bound. Report the key. */
            addKeyToBufferOnce(mc);
            /* Set the new timer and state */
            /* Start a new timer to clock in at 'repeat' intervals */
            /* printf("Setting mouse timer for repeat %d\n",repeatTime);*/
            timerSet(MOUSE_TIMER_ID,-1,repeatTime);
        }
    }
#endif
    
#if MEOPT_CALLBACK
    /* If the idle timer has gone then deal with it */
    if(isTimerExpired(IDLE_TIMER_ID))
    {
        register int index;
        meUInt arg;           /* Decode key argument */
        
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

/*
 * TTaheadFlush(), Typeahead with Message flush. Handle all messages on the
 * message queue to keep the screen and palette up-to-date.
 *
 * This is slightly different from TTahead in that we collect all input from
 * the message queues to ensure that we keep the number of windows resources
 * at their minimum. This is typically called from TTbreakTest() which is
 * invoked where there are signficant processing loops. It is essential that
 * we handle the message queue correctly in this instance to keep the screen
 * refreshed.
 */

int
TTaheadFlush(void)
{
    MSG msg;                            /* Local message buffer */
    
#ifdef _ME_CONSOLE
#ifdef _ME_WINDOW
    if(meSystemCfg & meSYSTEM_CONSOLE)
#endif /* _ME_WINDOW */
    {
        DWORD dwCount;
        INPUT_RECORD ir;
        for(;;)
        {
            if((hInput != INVALID_HANDLE_VALUE) &&
               (PeekConsoleInput(hInput, &ir, 1, &dwCount) != meFALSE) && (dwCount > 0))
            {
                if(meGetConsoleMessage(&msg,0))
                {
                    /* Keyboard message */
                    if((msg.message >= WM_KEYFIRST) && (msg.message <= WM_KEYLAST))
                        WinKeyboard(msg.hwnd, msg.message, msg.wParam, msg.lParam);
#if MEOPT_MOUSE
                    else if((msg.message >= WM_MOUSEFIRST) && (msg.message <= WM_MOUSELAST))
                        WinMouse(msg.hwnd, msg.message, msg.wParam, msg.lParam);
#endif
                }
            }
            else if(PeekMessage(&msg, meHWndNull, 0, 0, PM_REMOVE) != meFALSE)
            {
                if(msg.message == WM_TIMER)
                    /* Timer has expired */
                    timerAlarm(msg.wParam);
            }
            else
                break;
        }
    }
#ifdef _ME_WINDOW
    else
#endif /* _ME_WINDOW */
#endif /* _ME_CONSOLE */
#ifdef _ME_WINDOW
    {
        /* Peek all Keyboard and Mouse messages until there are no another
           message to process. */
        for(;;)
        {
            if(PeekMessage(&msg, meHWndNull, 0, 0, PM_REMOVE) != meFALSE)
            {
                /* Check out the keyboard. */
                if((msg.message >=  WM_KEYFIRST) && (msg.message <= WM_KEYLAST))
                {
                    TranslateMessage(&msg);
                    if(!WinKeyboard(msg.hwnd, msg.message, msg.wParam, msg.lParam))
                        meMessageHandler(&msg);
                }
#if MEOPT_MOUSE
                /* Check out the mouse. */
                else if((msg.message >= WM_MOUSEFIRST) && (msg.message <= WM_MOUSELAST))
                {
                    if(WinMouse(msg.hwnd, msg.message, msg.wParam, msg.lParam) == meFALSE)
                        meMessageHandler(&msg);
                }
#endif
                else if(msg.message == WM_TIMER)
                {
                    /* Check out the timers */
                    if(msg.wParam < NUM_TIMERS)
                        meTimerState[msg.wParam] = (meTimerState[msg.wParam] & ~TIMER_SET) | TIMER_EXPIRED;
                    else
                        meMessageHandler(&msg);
                }
                else
                    meMessageHandler(&msg);
            }
            else
                break;
        }
    }
#endif /* _ME_WINDOW */
#if MEOPT_MOUSE
    /* If the mouse alarm has returned Check the mouse */
    if(isTimerExpired(MOUSE_TIMER_ID))
    {
        meUShort mc;
        meUInt arg;
        
        timerClearExpired(MOUSE_TIMER_ID);
        mc = ME_SPECIAL | ttmodif | (SKEY_mouse_time + mouseKeys[mouseState&MOUSE_STATE_BUTTONS]);
        /* mouse-move bound ?? */
        if((!TTallKeys && (decode_key(mc,&arg) != -1)) || (TTallKeys & 0x2))
        {
            /* Timer expired and still bound. Report the key. */
            addKeyToBufferOnce(mc);
            /* Set the new timer and state */
            /* Start a new timer to clock in at 'repeat' intervals */
            /* printf("Setting mouse timer for repeat %d\n",repeatTime);*/
            timerSet(MOUSE_TIMER_ID,-1,repeatTime);
        }
    }
#endif
    
#if MEOPT_CALLBACK
    /* If the idle timer has gone then deal with it */
    if(isTimerExpired(IDLE_TIMER_ID))
    {
        register int index;
        meUInt arg;           /* Decode key argument */
        
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

/*
 * TTsleep()
 * Sleep for the specified time period. Only sleep while there
 * is no type-ahead data.
 */
void
TTsleep(int msec, int intable, meVariable **waitVarList)
{
    meUByte *ss;
    
    if(intable && ((kbdmode == mePLAY) || (clexec == meTRUE)))
        return;
    
    /* Do not actually need the absolute time as this will
     * remain the next alarm. Ensure that the value is sane */
    if(msec >= 0)
    {
        if(msec < 10)
            msec = 10;
        timerSet(SLEEP_TIMER_ID,-1,msec);
    }
    else if(waitVarList != NULL)
        timerKill(SLEEP_TIMER_ID);              /* Kill off the timer */
    else
        return;
    
    do
    {
        MSG msg;                            /* Message buffer */
        
        handleTimerExpired();
        
        /* Call TTahead first to get the input */
        if(intable && TTahead())
            break;
        /* TTahead can process the timers so we need to recheck the timers
         * before we wait for the next message */
        handleTimerExpired();
        
        if((waitVarList != NULL) &&
           (((ss=getUsrLclCmdVar((meUByte *)"wait",*waitVarList)) == errorm) || !meAtoi(ss)))
            break;
        
        /* Suspend until there is another message to process. */
        meGetMessage(&msg,1);
#ifdef _ME_CONSOLE
#ifdef _ME_WINDOW
        if(meSystemCfg & meSYSTEM_CONSOLE)
#endif /* _ME_WINDOW */
        {
            /* Keyboard message */
            if((msg.message >= WM_KEYFIRST) && (msg.message <= WM_KEYLAST))
                WinKeyboard(msg.hwnd, msg.message, msg.wParam, msg.lParam);
#if MEOPT_MOUSE
            else if((msg.message >= WM_MOUSEFIRST) && (msg.message <= WM_MOUSELAST))
                WinMouse(msg.hwnd, msg.message, msg.wParam, msg.lParam);
#endif
            else if(msg.message == WM_TIMER)
                /* Timer has expired */
                timerAlarm(msg.wParam);
        }
#ifdef _ME_WINDOW
        else
#endif /* _ME_WINDOW */
#endif /* _ME_CONSOLE */
#ifdef _ME_WINDOW
        {
            TranslateMessage(&msg);
            meMessageHandler(&msg);
        }
#endif /* _ME_WINDOW */
    } while((msec < 0) || !isTimerExpired(SLEEP_TIMER_ID));
    
    timerKill(SLEEP_TIMER_ID);              /* Kill off the timer */
}

#ifdef _ME_WINDOW
static void
meFrameSetWindowSizeInternal(meFrame *frame)
{
    int ii, width, depth;
    
    /* get the new canvas area and invalidate it. */
    GetClientRect(meFrameGetWinHandle(frame), &meFrameGetWinCanvas(frame)); /* Get the new canvas size */
    InvalidateRect(meFrameGetWinHandle(frame), &meFrameGetWinCanvas(frame), meFALSE);
    meFrameGetWinPaintAll(frame) = 1;
    
    /* Set up the frame store */
    width = frame->width;
    if(width > eCellMetrics.cellWidthCount)
    {
        /* Set up the column cell LUT positions. Note allocate a single
         * array and split into two for re-use */
        eCellMetrics.cellColPos = meRealloc(eCellMetrics.cellColPos,sizeof(meShort) * (width + 1));
        /* Construct the cell spacing. This is the  width of the characters. */
        eCellMetrics.cellSpacing = meRealloc(eCellMetrics.cellSpacing,sizeof(INT) * (width + 1));
        /* Construct the temporary rendering buffer */
        eCellMetrics.cellColTmpPos = meRealloc(eCellMetrics.cellColTmpPos, width+1);
        eCellMetrics.cellWidthCount = width;
    }
    /* Initialise the column cell LUT tables - this must always be done as only the font may have changed. */
    for(ii = 0; ii <= width; ii++)
    {
        eCellMetrics.cellColPos[ii] = colToClient(ii);
        eCellMetrics.cellSpacing[ii] = eCellMetrics.cell.sizeX;
    }
    
    /* Grow the existing rows */
    depth = frame->depth+1;
    if(depth > eCellMetrics.cellDepthCount)
    {
        /* Set up the row cell LUT positions. Note allocate a single
         * array and split into 4 for re-use. */
        eCellMetrics.cellRowPos = meRealloc(eCellMetrics.cellRowPos,sizeof(meShort) * 2 * (depth + 1));
        eCellMetrics.cellDepthCount = depth;
    }
    /* Initialise the row cell LUT tables - this must always be done as only the font may have changed. */
    for(ii = 0; ii <= depth; ii++)
        eCellMetrics.cellRowPos[ii] = rowToClient(ii);
    
    /* resize the frame specific data */
    if(depth > meFrameGetWinPaintDepth(frame))
    {
        meFrameGetWinPaintStartCol(frame) = meRealloc(meFrameGetWinPaintStartCol(frame), sizeof(meShort) * 2 * (depth + 1));
        meFrameGetWinPaintEndCol(frame)   = &(meFrameGetWinPaintStartCol(frame)[depth+1]);
        meFrameGetWinPaintDepth(frame)    = depth;
    }
    if(!screenUpdateDisabledCount)
        screenUpdate(meTRUE,2-sgarbf);
}
#endif /* _ME_WINDOW */

/*
 * meFrameSetWindowSize
 *
 * Changes the width and depth of the screen.
 *
 * This function is initiated from window.c in frameChangeDepth ()
 * [Emacs command change-screen-depth]. This presents a problem in that
 * meFrameResizeWindow() also calls frameChangeDepth (). This causes a recursive loop !!
 * So to make sure that we do not get stuck meFrameResizeWindow() ONLY invokes
 * frameChangeDepth () when the current frameCur->depth is not equal to it's new depth.
 *
 * As it turns out this recursive loop is useful. The user might request a
 * screen depth of 1000 columns frameChangeDepth() will allocate these and call
 * meFrameSetWindowSize() when it has finished. meFrameSetWindowSize() invokes WinTermCellResize()
 * which will try to establish the new width which will fail and truncate to
 * the largest width it can allocate. On invocation of meFrameResizeWindow() a
 * new number of rows is computed from the window size (now the truncated)
 * and frameChangeDepth() is invoked. frameChangeDepth() will fix it's rows to match the
 * screen size and invokes meFrameSetWindowSize() again. meFrameSetWindowSize() drops out immediatly
 * since the CellMetrics should now match the screen depth, hence we recurse
 * out and everybody is happy !!.
 */
void
meFrameSetWindowSize(meFrame *frame)
{
    int width;
    int depth;
    
    width = frame->width;
    depth = frame->depth+1;
    
#ifdef _ME_CONSOLE
#ifdef _ME_WINDOW
    /* If in console mode... */
    if(meSystemCfg & meSYSTEM_CONSOLE)
#endif /* _ME_CONSOLE */
    {
        CONSOLE_CURSOR_INFO CursorInfo;
        if((width*depth) > ciScreenSize)
        {
            if(ciScreenBuffer != NULL)
                free(ciScreenBuffer);
            ciScreenBuffer = (CHAR_INFO *) meMalloc(sizeof(CHAR_INFO) * width * depth);
            memset((void *)ciScreenBuffer, 0, sizeof(CHAR_INFO) * width * depth);
        }
        /* Hide/rehide the cursor (resize window seems to make it visible again) */
        GetConsoleCursorInfo(hOutput,&CursorInfo);
        CursorInfo.bVisible = meFALSE;
        SetConsoleCursorInfo(hOutput,&CursorInfo);
    }
#ifdef _ME_WINDOW
    else
#endif /* _ME_WINDOW */
#endif /* _ME_CONSOLE */
#ifdef _ME_WINDOW
    {
        RECT wRect;                         /* Window rectangle */
        RECT cRect;                         /* Client rectangle */
        int dBorder;                        /* Depth of the border */
        int wBorder;                        /* Width of the boarder */
        int ndepth;                         /* The new depth */
        int nwidth;                         /* The new width */
        
        /* Get the current screen widths */
        GetWindowRect(meFrameGetWinHandle(frame), &wRect);
        GetClientRect(meFrameGetWinHandle(frame), &cRect);
        
        /* calc the boarder sizes by differencing the window and the client area */
        wBorder = (wRect.right - wRect.left) - cRect.right;
        dBorder = (wRect.bottom - wRect.top) - cRect.bottom;
        
        /* Compute the new window widths in terms of pixels */
        /* Resize the x axis if requested.
         * Compute the desired window width, no horizontal size restriction */
        nwidth = (width * eCellMetrics.cell.sizeX) + wBorder;
        
        /* Resize the y axis if requested. If the requested size is greater
         * than the screen depth then refuse, maximum depth is the screen depth */
        ndepth = (depth * eCellMetrics.cell.sizeY) + dBorder;
        if((eCellMetrics.maxDepth > 0) && (ndepth > eCellMetrics.maxDepth))
        {
            ndepth = (eCellMetrics.maxDepth - dBorder) / eCellMetrics.cell.sizeY;
            meFrameChangeDepth(frame,ndepth);
            ndepth = (ndepth * eCellMetrics.cell.sizeY) + dBorder;
        }
        
        if((nwidth != (wRect.right - wRect.left)) || (ndepth != (wRect.bottom - wRect.top)))
        {
#if _WIN_DEBUG_POS
            if(logfp != NULL)
            {
                fprintf(logfp,"SetWindowPos - %d %d -> %d %d (%d)\n",width,depth,nwidth,ndepth,eCellMetrics.maxDepth);
                fflush(logfp);
            }
#endif
            SetWindowPos(meFrameGetWinHandle(frame),NULL,0,0,nwidth,ndepth,
                         SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
            meFrameSetWindowSizeInternal(frame);
        }
        
    }
#endif /* _ME_WINDOW */
}

#if MEOPT_EXTENDED
void
meFrameRepositionWindow(meFrame *frame, int resize)
{
#ifdef _ME_WINDOW
    if(
#ifdef _ME_CONSOLE
       /* If in console mode... */
       !(meSystemCfg & meSYSTEM_CONSOLE) &&
#endif /* _ME_CONSOLE */
       !meFrameGetWinMaximized(frame))
    {
        RECT wRect;
        int left, top, width, depth;
        
        /* Always reposition so the top left is visible and as much of the window */
        GetWindowRect(meFrameGetWinHandle(frame),&wRect);
        
        left = wRect.left;
        width = wRect.right - left;
        if(wRect.right > eCellMetrics.workArea.right)
            left = eCellMetrics.workArea.right - width;
        if(left < eCellMetrics.workArea.left)
            left = eCellMetrics.workArea.left;
        
        top = wRect.top;
        depth = wRect.bottom - top;
        if(wRect.bottom > eCellMetrics.workArea.bottom)
            top = eCellMetrics.workArea.bottom - depth;
        if(top < eCellMetrics.workArea.top)
            top = eCellMetrics.workArea.top;
        
        if(resize && (((left + width) > eCellMetrics.workArea.right) || ((top + depth) > eCellMetrics.workArea.bottom)))
        {
            if((left + width) > eCellMetrics.workArea.right)
                width = eCellMetrics.workArea.right - left;
            if((top + depth) > eCellMetrics.workArea.bottom)
                depth = eCellMetrics.workArea.bottom - top;
            SetWindowPos(meFrameGetWinHandle(frame),NULL,left,top,width,depth,
                         SWP_NOZORDER | SWP_NOACTIVATE);
        }
        else if((left != wRect.left) || (top != wRect.top))
            SetWindowPos(meFrameGetWinHandle(frame),NULL,left,top,0,0,SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
    }
#endif /* _ME_WINDOW */
}
#endif

static void
meSetupUserName(void)
{
    char *nn, buff[128];
    DWORD ii;
    
    /* Decide on a name. */
    if(((nn = meGetenv("MENAME")) == NULL) || (nn[0] == '\0'))
    {
        ii = 128;
        if((GetUserName(buff,&ii) == meTRUE) && (buff[0] != '\0'))
            nn = buff;
        else if(((nn = meGetenv("LOGNAME")) != NULL) && (nn[0] == '\0'))
            nn = NULL;
    }
    if(nn == NULL)
        nn = "user";
    meUserName = meStrdup((meUByte *) nn);
    meUid = (IsUserAnAdmin()) ? 0:1;
}

void
meSetupProgname(char *progname)
{
    char buff[meBUF_SIZE_MAX];
    
    curdir = gwd(0);
    if(curdir == NULL)
        /* not yet initialised so mlwrite will exit */
        mlwrite(MWCURSOR|MWABORT|MWWAIT,(meUByte *)"Failed to get cwd\n");
    
    /* Setup the $progname make it an absolute path. On MS-Windows use the
     * Windows specific system call to determine the full executable filename
     * and then perform an executableLookup() in order to correct the
     * filename. - Thanks to Petro 2009-11-09. */
    if(((GetModuleFileName(0, buff, sizeof(buff)) > 0) && executableLookup((meUByte *) buff,evalResult)) ||
       executableLookup((meUByte *) progname,evalResult))
    {
        meProgName = meStrdup(evalResult);
    }
    else
    {
#ifdef _ME_FREE_ALL_MEMORY
        /* stops problems on exit */
        meProgName = meStrdup(progname);
#else
        meProgName = (meUByte *) progname;
#endif
    }
}

/* meSetupPathsAndUser
 *
 * On windows the user name has already been setup - required for ini file reading
 * The ini file may also have found an mepath or meinstallpath setting */
void
meSetupPathsAndUser(void)
{
    char *ss, buff[meBUF_SIZE_MAX], pdBuf[meBUF_SIZE_MAX];
    int pdLen, ii, ll, gotPaths;
    
#if (defined CSIDL_APPDATA)
#if meBUF_SIZE_MAX < MAX_PATH
#error meBUF_SIZE_MAX too small, must be at least MAX_PATH
#endif
    if((SHGetFolderPath(NULL,CSIDL_APPDATA|CSIDL_FLAG_CREATE,NULL,0,pdBuf) != S_OK) || (pdBuf[0] != '\0'))
#endif
    {
        /* get the windows user application data path */
        if(((ss = meGetenv("APPDATA")) != NULL) && (ss[0] != '\0'))
            strcpy(pdBuf,ss);
        else
            pdBuf[0] = '\0';
    }
    if(((pdLen=strlen(pdBuf)) > 0) && ((meProgData=meMalloc(pdLen+9)) != NULL))
    {
        fileNameConvertDirChar(pdBuf);
        if(pdBuf[pdLen-1] != DIR_CHAR)
            pdBuf[pdLen++] = DIR_CHAR;
        memcpy(pdBuf+pdLen,"jasspa/",8);
        memcpy(meProgData,pdBuf,pdLen+8);
    }
    
    /* meUserPath & searchPath may not be null due to -v command-line option */ 
    if((((ss = (char *) meUserPath) != NULL) && (ss[0] != '\0')) ||
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
        searchPath = meStrdup((meUByte *) ss);
    
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
            searchPath = (meUByte *) ss;
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
        else
        {
            evalResult[0] = '\0';
            ll = 0;
        }
        /* Check for setting of $MEINSTALLPATH first, if set, check for $user-path and standard sub-dirs */
        if(((ss = meGetenv("MEINSTALLPATH")) != NULL) && (ss[0] != '\0'))
        {
            strcpy(buff,ss);
            ll = mePathAddSearchPath(ll,evalResult,(meUByte *) buff,6,&gotPaths);
        }
        if((meProgData != NULL) && (gotPaths != 0x0f))
        {
            /* look for the user's area in $APPDATA/jasspa directory - an exception here, if we find macros or spelling here
             * still look for it in the program area as this may just contain downloaded help and spelling packages */
            ii = gotPaths;
            ll = mePathAddSearchPath(ll,evalResult,(meUByte *) pdBuf,6,&ii);
            gotPaths |= (ii & 0x0c);
        }
        
        /* also check for directories in the same location as the binary */
        if((ss=meStrrchr(meProgName,DIR_CHAR)) != NULL)
        {
            ii = (((size_t) ss) - ((size_t) meProgName));
            meStrncpy(buff,meProgName,ii);
            buff[ii] = '\0';
            ll = mePathAddSearchPath(ll,evalResult,(meUByte *) buff,9,&gotPaths);
        }
#if MEOPT_TFS
        /* also check for the built-in file system */
        if((tfsdev != NULL) && (gotPaths != 0x0f))
            ll = mePathAddSearchPath(ll,evalResult,(meUByte *) "tfs://",1,&gotPaths);
#endif
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
                ll = ((size_t) ss) - ((size_t) searchPath);
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
    
    if((((ss = meGetenv("HOME")) != NULL) && (ss[0] != '\0')) ||
#if (defined CSIDL_PROFILE)
       ((SHGetFolderPath(NULL,CSIDL_PROFILE|CSIDL_FLAG_CREATE,NULL,0,buff) == S_OK) && ((ss=buff)[0] != '\0')) ||
#endif
       ((ss = meProgData) != NULL))
        fileNameSetHome((meUByte *) ss);
}

#ifdef _ME_CONSOLE
/****************************************************************************
   
FUNCTION: main(int, char *[])
   
PURPOSE: calls initialization function, processes message loop
   
 ****************************************************************************/

int
main(int argc, char *argv[])
{
#else
/****************************************************************************
   
FUNCTION: WinMain(HANDLE, HANDLE, LPSTR, int)
   
PURPOSE: calls initialization function, processes message loop
   
 ****************************************************************************/

int APIENTRY
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    char **argv;                        /* Argument string table */
    int  argc;                          /* Argument count */
#endif
    
#ifdef _ME_WIN32_FULL_DEBUG
    freopen("std.err","w",stderr);
    _CrtSetReportMode(_CRT_WARN,_CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN,_CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ERROR,_CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ERROR,_CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ASSERT,_CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT,_CRTDBG_FILE_STDERR);
    /* Enable heap checking on each allocate and free */
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) |_CRTDBG_ALLOC_MEM_DF|_CRTDBG_LEAK_CHECK_DF|_CRTDBG_DELAY_FREE_MEM_DF);
#endif
    
#if _WIN_DEBUG_KEY || _WIN_DEBUG_MOUSE || _WIN_DEBUG_MSG || _WIN_DEBUG_POS
    if(logfp == NULL)
        logfp = fopen("medebug.log","w+");
#endif
    
#ifdef _ME_WINDOW
#ifdef _ME_CONSOLE
    /* Can we do something more safe here for Console+Window version? */
    {
        HMODULE hInstance = GetModuleHandle(NULL);
#else
    if(!hPrevInstance)
    {
#endif
        /* Initialise the window data and register window class */
        WNDCLASS  wc;
        
        wc.style = 0;
        wc.lpfnWndProc = (WNDPROC) MainWndProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = hInstance;
        wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
        wc.hCursor = NULL;
        wc.hbrBackground = GetStockObject(BLACK_BRUSH);
        wc.lpszMenuName =  "InputMenu";
        wc.lpszClassName = "MicroEmacsClass";
        
        if(!RegisterClass(&wc))
            return meFALSE;
        
        ttInstance =  hInstance;
#ifdef _ME_CONSOLE
        ttshowState = 1;
#else
        ttshowState = nCmdShow;
#endif
    }
#endif /* _ME_WINDOW */
#ifdef _WIN32_WINNT
#if MEOPT_IPIPES
    meSYSTEM_MASK |= meSYSTEM_DOSFNAMES|meSYSTEM_IPIPES;
    meSystemCfg |= meSYSTEM_IPIPES;
#else
    meSYSTEM_MASK |= meSYSTEM_DOSFNAMES;
#endif
#else
    {
        OSVERSIONINFO os;
        
        os.dwOSVersionInfoSize = sizeof(os);
        GetVersionEx(&os);
        platformId = os.dwPlatformId;
        if(platformId != VER_PLATFORM_WIN32s)
        {
#if MEOPT_IPIPES
            meSYSTEM_MASK |= meSYSTEM_DOSFNAMES|meSYSTEM_IPIPES;
            meSystemCfg |= meSYSTEM_IPIPES;
#else
            meSYSTEM_MASK |= meSYSTEM_DOSFNAMES;
#endif
        }
    }
#endif
    TTwidthDefault=80;
    TTdepthDefault=50;
    meSetupUserName();
    ttThreadId = GetCurrentThreadId();
    
#ifndef _ME_CONSOLE
    /* Transpose the argument string into a standard argc, argv line since
     * Windows abandons all standard 'C' practices and does it's own thing
     * (bunch of M***** F*****'s !!). */
    argv = (char **) meMalloc(sizeof(char *) * 2);
    argv[1] = NULL;
    argc = 1;
    
    /* Get the process name out. This is horrible, the command Line name
     * is returned as UNI-Code characters. In addition we have to cope
     * for spaces in the name, so we check for the ".exe" extension */
    {
        LPTSTR pp;                      /* Command line string pointer */
        /* Find the executable name */
        if((pp = GetCommandLine()) == NULL)
        {
            argv[0] = "me";             /* Use a sensible default */
        }
        else
        {
            char *qq, cc, ec;
            char nambuf[meBUF_SIZE_MAX];
            
            /* ingore the first quote, if 2 quotes then the progname was in
             * quotes so don't stop at a space */
            if(*pp == '"')
            {
                ec = '"';
                pp++;
            }
            else
                ec = ' ';
            qq = nambuf;
            while(((cc=*pp++) != '\0') && (cc != ec))
            {
                if(cc == '\\')
                    cc = '/';
                *qq++ = cc;
            }
            /* Copy the command name */
            *qq = '\0';
            argv[0] = meStrdup((meUByte *) nambuf);
        }
    }
    
    /* Get the rest of the parameters from the passed in command line */
    if((lpCmdLine != NULL) && (*lpCmdLine != '\0'))
    {
        extern int mesetupInsertResourceString(char *ss, int argi, int oargc, char **oargv[]);
        argc = mesetupInsertResourceString(meStrdup((meUByte *) lpCmdLine),1,argc,&argv);
    }
#endif /* !_ME_CONSOLE */
    
    /* Call EMACS with the command line that we have just constructed.
     * Note that we cannot delete the string that we have allocated since
     * EMACS may retain parts of the argument list. */
    mesetup(argc, argv);
    
    /* Just incase the window has been resized during start up go and check */
    meFrameResizeWindow(frameCur);                    /* Resize the screen */
    
    /* Sint in a continual loop and process messages. */
    while(1)
    {
        doOneKey();
        if(TTbreakFlag)
        {
            TTinflush();
            if((selhilight.flags & (SELHIL_ACTIVE|SELHIL_KEEP)) == SELHIL_ACTIVE)
                selhilight.flags &= ~SELHIL_ACTIVE;
            TTbreakFlag = 0;
        }
#ifdef _ME_WINDOW
#ifdef _DRAGNDROP
        /* if drag and drop is enabled then process the drag and drop
         * files now. Note we make the invocation here as we
         * know we have returned to a base state; any macro's would have
         * been aborted. However it is better that macro debugging
         * is disabled so explicitly disable it. */
        if(dadHead != NULL)
        {
            struct s_DragAndDrop *dadp; /* Drag and drop pointer */
            
            /* Disable the cursor to allow the mouse position to be
             * artificially moved */
#if MEOPT_DEBUGM
            macbug = 0;                 /* Force macro debugging off */
#endif
            /* hide the mouse cursor
             * This must be done whether MEOPT_MOUSE is enabled or not */
            mouseHide();                /* Hide the cursor */
            /* Iterate down the list and get the files. */
            while((dadp = dadHead) != NULL)
            {
#if MEOPT_MOUSE
                /* Re-position the mouse */
                mouse_X = (meShort) clientToCol(dadp->mousePos.x);
                mouse_Y = (meShort) clientToRow(dadp->mousePos.y);
                mouse_dX = (meShort) (((dadp->mousePos.x - colToClient(mouse_X)) << 8) / eCellMetrics.cell.sizeX);
                mouse_dY = (meShort) (((dadp->mousePos.y - rowToClient(mouse_Y)) << 8) / eCellMetrics.cell.sizeY);
                if(mouse_X > frameCur->width)
                    mouse_X = frameCur->width;
                if(mouse_Y > frameCur->depth)
                    mouse_Y = frameCur->depth;
                
                /* Find the window with the mouse */
                setCursorToMouse(meFALSE,0);
#endif
#if MEOPT_EXTENDED
                /* if the current window is locked to a buffer find another */
                if(frameCur->windowCur->flags & meWINDOW_LOCK_BUFFER)
                    meWindowPopup(NULL,NULL,WPOP_MKCURR|WPOP_USESTR,NULL);
#endif
                
                /* Find the file into buffer */
                findSwapFileList(dadp->fname,BFND_CREAT|BFND_MKNAM,0,0);
                
                /* Destruct the list */
                dadHead = dadp->next;
                meFree(dadp);
            }
            /* Display a message indicating last trasaction */
            mlwrite(0,(meUByte *) "Drag and Drop transaction completed");
        }
#endif /* _DRAGNDROP */
#endif /* _ME_WINDOW */
    }
    return 0;
}

#ifdef _ME_WINDOW

static void
meFrameGainFocus(meFrame *frame)
{
    /* have we not got the focus? */
    if(frame->flags & meFRAME_NOT_FOCUS)
    {
        /* Record the fact we have focus */
        frame->flags &= ~meFRAME_NOT_FOCUS;
#if MEOPT_MWFRAME
        if(frameCur != frame)
            frameFocus = frame;
#endif
        
        /* Mark the screen as invalid */
        InvalidateRect(meFrameGetWinHandle(frame), NULL, meFALSE);
        meFrameGetWinPaintAll(frame) = 1;
        
        /* We have been swapped out. Therefore we potentially do not
         * own the clipboard contents*/
        clipState &= ~CLIP_OWNER;
        
        /* Make sure the cursor is ok
         * This must be done whether MEOPT_MOUSE is enabled or not */
        SetCursor(meCursors[meCurCursor]);
        mouseState |= MOUSE_STATE_VISIBLE;
    }
}

static void
meFrameKillFocus(meFrame *frame)
{
    /* have we got the focus to loose it? */
    if(!(frame->flags & meFRAME_NOT_FOCUS))
    {
        frame->flags |= meFRAME_NOT_FOCUS;
#if MEOPT_MWFRAME
        if(frameFocus == frame)
            frameFocus = NULL;
#endif
        
        if(cursorState >= 0)
        {
            /* because the cursor is a part of the solid cursor we must
             * remove the old one first and then redraw
             */
            if(blinkState)
                meFrameHideCursor(frame);
            blinkState = 1;
            meFrameShowCursor(frame);
        }
#if MEOPT_MOUSE
        /* Relinquish control of the mouse */
        if(mouseState & MOUSE_STATE_LOCKED)
        {
            ReleaseCapture();          /* Relinquish the mouse */
            mouseState &= ~MOUSE_STATE_LOCKED;
        }
#endif
    }
}

/****************************************************************************
   
FUNCTION: MainWndProc(HWND, UINT, UINT, LPARAM)
   
PURPOSE:  Processes messages
   
MESSAGES:
   
   WM_COMMAND    - application menu (About dialog box)
   WM_CREATE     - create window
   WM_MOUSEMOVE  - mouse movement
   WM_LBUTTONDOWN - left mouse button pressed
   WM_LBUTTONUP  - left mouse button released
   WM_LBUTTONDBLCLK - left mouse button double clicked
   WM_KEYDOWN    - key pressed
   WM_KEYUP      - key released
   WM_CHAR       - ASCII character received
   WM_TIMER      - timer has elapsed
   WM_PAINT      - update window, draw objects
   WM_DESTROY    - destroy window
   
COMMENTS:
   
   This demonstrates how input messages are received, and what the
   additional information is that comes with the message.
   
 ****************************************************************************/

LRESULT APIENTRY
MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static LPARAM setCursorLastLParam;
    meFrame *frame;
    
#if _WIN_DEBUG_MSG
    if(logfp != NULL)
    {
        static int msgCount=0;
        fprintf(logfp,"%05d Got message %x %x %x\n",msgCount++,message, wParam, lParam);
        fflush(logfp);
    }
#endif
    
    switch(message)
    {
    case WM_SETFOCUS:
        if((frame = meMessageGetFrame(hWnd)) != NULL)
            meFrameGainFocus(frame);
        break;
        
    case WM_KILLFOCUS:
        if((frame = meMessageGetFrame(hWnd)) != NULL)
            meFrameKillFocus(frame);
        break;
        
    case WM_SETCURSOR:
        /* If we have not got the focus or this is not the main window cursor
         * then we dont handle it, use the default Proc handle
         * This must be done whether MEOPT_MOUSE is enabled or not */
        if((lParam & 0xffff) != 0x01)
        {
            setCursorLastLParam = -1;
            goto unhandled_message;
        }
        if(lParam != setCursorLastLParam)
        {
            setCursorLastLParam = lParam;
            SetCursor(meCursors[meCurCursor]);
            mouseState |= MOUSE_STATE_VISIBLE;
        }
        break;
        
    case WM_QUERYNEWPALETTE:
        /* About to get focus, realise our palette */
        if(eCellMetrics.pInfo.hPal != NULL)
        {
            /* we have a palette simply return TRUE as the WM_PALETTECHANGED
             * message will handle the actual ins and outs of palette swapping */
            return meTRUE;
        }
        break;
        
    case WM_PALETTECHANGED:
        /* Another application has changed the palette */
        /* we only want to do this once so only do it if this is to the baseHwnd.
         * Note that it could be us changing the palette. */
        if((eCellMetrics.pInfo.hPal != NULL) && (hWnd == baseHwnd))
        {
            HDC hDC = GetDC(baseHwnd);
            
            SelectPalette(hDC, eCellMetrics.pInfo.hPal, meTRUE);
            if(RealizePalette(hDC) != GDI_ERROR)
            {
                UpdateColors(hDC);
                meFrameLoopBegin();
                
                meFrameLoopContinue(loopFrame->flags & meFRAME_HIDDEN);
                
                InvalidateRect(meFrameGetWinHandle(loopFrame), NULL, meFALSE);
                meFrameGetWinPaintAll(loopFrame) = 1;
                
                meFrameLoopEnd();
            }
            ReleaseDC(baseHwnd, hDC);
        }
        break;
        
    case WM_CREATE:
#if MEOPT_MOUSE
        /* Set the default mouse state. Get the number of buttons. Note under windows we do not
         * need to worry about a left/right swap since that is performed beneath us. */
        TTinitMouse(meMouseCfg | (GetSystemMetrics(SM_CMOUSEBUTTONS) & meMOUSE_NOBUTTONS));
#endif
        /* still must do some work if mouse is disabled */
        meCursors[meCURSOR_DEFAULT] = LoadCursor(NULL,IDC_ARROW);
        meCursors[meCURSOR_ARROW] = meCursors[meCURSOR_DEFAULT];
        mouseShow();
#ifdef _DRAGNDROP
        /* Enable drag and drop handling */
        DragAcceptFiles(hWnd, meTRUE);
#endif
        break;
        
#ifdef _DRAGNDROP
    case WM_DROPFILES:
        /* We cannot process the drag and drop events immediately as we do
         * not know what state the user has left the system in. Hive away the
         * drag and drop files until we are ready to process them. Retain the
         * cursor position so that we may show the appropriate file in a
         * specified buffer. */
        {
            HDROP hDrp = (HDROP) wParam;
            POINT pt;                   /* Position in the window */
            WORD fcount;                /* Count of the number of files */
            meUByte dfname[meBUF_SIZE_MAX];       /* Dropped filename */
            
            /* Get the position of the mouse */
            DragQueryPoint(hDrp, &pt);
            
            /* Get the files from the drop */
            fcount = DragQueryFile(hDrp, 0xffffffff, "", 0);
            if(fcount > 0)
            {
                WORD ii;
                
                for(ii = 0; ii < fcount; ii++)
                {
                    int len;
                    struct s_DragAndDrop *dadp;
                    
                    if((len = DragQueryFile(hDrp,ii,(LPSTR) dfname,meBUF_SIZE_MAX)) <= 0)
                        continue;
                    
                    /* Get the drag and drop buffer and add to the list */
                    dadp = (struct s_DragAndDrop *) meMalloc(sizeof(struct s_DragAndDrop) + len);
                    meStrcpy(dadp->fname, dfname);
                    dadp->mousePos = pt;
                    dadp->next = dadHead;
                    dadHead = dadp;
                }
            }
            DragFinish(hDrp);
            
            /* Flush the input queue, send an abort to kill any command
             * off. The drag and drop list is processed once we return to
             * a base state. */
            if(dadHead != NULL)
                addKeyToBuffer(breakc);  /* Break character (ctrl-G) */
        }
        break;
#endif
#if MEOPT_MOUSE
        /************************************************************************
         * MOUSE Handling
         ************************************************************************/
    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
#ifdef WM_MOUSEWHEEL
    case WM_MOUSEWHEEL:
#endif
        if(WinMouse(hWnd,message,wParam,lParam) == meFALSE)
            goto unhandled_message;
        break;
#else
    case WM_MOUSEMOVE:
        mouseShow();
        goto unhandled_message;
#endif
        
        /************************************************************************
         * KEYBOARD Handling
         ************************************************************************/
    case WM_SYSKEYDOWN:
    case WM_KEYDOWN:
    case WM_SYSKEYUP:
    case WM_KEYUP:
    case WM_SYSCHAR:
    case WM_CHAR:
        if(WinKeyboard(hWnd, message, wParam, lParam) == meFALSE)
            goto unhandled_message;
        break;
        
        /************************************************************************
         * MISCELLANEOUS Handling
         * Timers, paint events, clipboard, termination etc.
         ************************************************************************/
    case WM_TIMER:
        if(wParam >= NUM_TIMERS)
            /* unhandled timer */
            return meTRUE;
        meTimerState[wParam] = (meTimerState[wParam] & ~TIMER_SET) | TIMER_EXPIRED;
        break;
        
    case WM_SIZE:
        /* Resize the window when requested. */
        if((wParam != SIZE_MINIMIZED) && ((frame = meMessageGetFrame(hWnd)) != NULL))
        {
            RECT cRect;
            int nrow, ncol, setSize=0;
            if(wParam == SIZE_MAXIMIZED)
                meFrameGetWinMaximized(frame) = 1; /* Set the maximized state */
            else if(wParam == SIZE_RESTORED)
                meFrameGetWinMaximized(frame) = 0; /* Release the minimised state */
            /* Inform Emacs window S/W that the window has changed size */
            GetClientRect(hWnd, &cRect);
            nrow = cRect.bottom / eCellMetrics.cell.sizeY;
            ncol = cRect.right / eCellMetrics.cell.sizeX;
#if _WIN_DEBUG_POS
            if(logfp != NULL)
            {
                fprintf(logfp,"WM_SIZE - %x %x %d %d -> %d %d\n",wParam,lParam,cRect.right,cRect.bottom,ncol,nrow);
                fflush(logfp);
            }
#endif
            if(ncol != frame->width)
            {
                meFrameChangeWidth(frame,ncol);
                setSize = 1;
            }
            if(nrow != (frame->depth+1))
            {
                meFrameChangeDepth(frame,nrow);
                setSize = 1;
            }
            if(setSize)
                meFrameSetWindowSizeInternal(frame);
        }
        break;
        
    case WM_WINDOWPOSCHANGING:
        if(meMessageGetFrame(hWnd) != NULL)
        {
            LPWINDOWPOS pos = (WINDOWPOS *) lParam;
            
            if((pos->flags & SWP_NOSIZE) == 0)
            {
                int col, row;
                
                col = (pos->cx - eCellMetrics.borderWidth) / eCellMetrics.cell.sizeX;
                row = (pos->cy - eCellMetrics.borderDepth) / eCellMetrics.cell.sizeY;
#if _WIN_DEBUG_POS
                if(logfp != NULL)
                {
                    fprintf(logfp,"WM_WINDOWPOSCHANGING - %x %x - %d %d - %d %d -> %d %d\n",wParam,lParam,eCellMetrics.borderWidth,eCellMetrics.borderDepth,pos->cx,pos->cy,col,row);
                    fflush(logfp);
                }
#endif
                pos->cx = (col * eCellMetrics.cell.sizeX) + eCellMetrics.borderWidth;
                pos->cy = (row * eCellMetrics.cell.sizeY) + eCellMetrics.borderDepth;
            }
        }
        goto unhandled_message;
        
    case WM_GETMINMAXINFO:
        {
            LPMINMAXINFO si = (LPMINMAXINFO) lParam;
            if((eCellMetrics.maxDepth != si->ptMaxTrackSize.y) || (eCellMetrics.maxPosTop != si->ptMaxPosition.y) || (eCellMetrics.maxPosLeft != si->ptMaxPosition.x))
               
            {
                RECT wa;
                int bw,bd;
                
                if((bw = GetSystemMetrics(SM_CXSCREEN)) > 0)
                    eCellMetrics.borderWidth = si->ptMaxSize.x - bw;
                else
                    eCellMetrics.borderWidth = 0;
                bw = eCellMetrics.borderWidth >> 1;
                if((bd = GetSystemMetrics(SM_CYSCREEN)) > 0)
                    eCellMetrics.borderDepth = si->ptMaxSize.y - bd;
                else
                    eCellMetrics.borderDepth = 0;
                bd = eCellMetrics.borderDepth >> 1;
                
                eCellMetrics.maxDepth = si->ptMaxTrackSize.y;
                eCellMetrics.maxPosTop = si->ptMaxPosition.y;
                eCellMetrics.maxPosLeft = si->ptMaxPosition.x;
                /* Wierdly when positioning the window the x needs to be the left of the border&drop-shadow (e.g. -8) but the top doesn't, i.e. 0 */
                /* When positioning a non-maximised window try to leave at least a boarder thckness gap to left, right and bottom (top has titlebar) */ 
                eCellMetrics.workArea.left = si->ptMaxPosition.x+bw;
                eCellMetrics.workArea.right = si->ptMaxPosition.x+si->ptMaxSize.x-bw;
                eCellMetrics.workArea.top = si->ptMaxPosition.y+bd;
                eCellMetrics.workArea.bottom = si->ptMaxPosition.y+si->ptMaxSize.y-bd;
                
                if(SystemParametersInfo(SPI_GETWORKAREA,0,&wa,0))
                {
                    if(wa.top > eCellMetrics.workArea.top)
                        eCellMetrics.workArea.top = wa.top;
                    if(wa.bottom < eCellMetrics.workArea.bottom)
                        eCellMetrics.workArea.bottom = wa.bottom;
                    if(wa.left > eCellMetrics.workArea.left)
                        eCellMetrics.workArea.left = wa.left;
                    if(wa.right < eCellMetrics.workArea.right)
                        eCellMetrics.workArea.right = wa.right;
                }
                /* must now add in the title bar caption height so a resize allows for it */
                if((bd = GetSystemMetrics(SM_CYSMCAPTION)) > 0)
                    eCellMetrics.borderDepth += bd;
            }
        }
        break;
        
    case WM_RENDERALLFORMATS:           /* Clipboard data requests */
        if(!OpenClipboard(baseHwnd))
            break;
        EmptyClipboard();
    case WM_RENDERFORMAT:
        {
            HANDLE hmem;
            hmem = WinKillToClipboard();
            SetClipboardData(CF_UNICODETEXT, hmem);
            /* Force the stale state. If another application is pulling data
             * from us while we are the clipboard owner we must force the
             * clipboard to be refreshed whenever the 'yank' buffer changes.
             * If nobody is listening then we do not care. This simply allows
             * us to process optimally and not to continually report a
             * clipboard change whenever we change the yank data. */
            clipState |= CLIP_STALE;
        }
        if(message == WM_RENDERALLFORMATS)
            CloseClipboard();
        break;
        
    case WM_PAINT:
        if((frame = meMessageGetFrame(hWnd)) != NULL)
            meFrameDraw(frame);            /* Paint the frame */
        break;
        
    case WM_CLOSE:
        if((frame = meMessageGetFrame(hWnd)) != NULL)
        {
#if MEOPT_MWFRAME
            if(meFrameDelete(frame,6) > 0)
                return meFALSE;
#endif
            if(ttThreadId != GetCurrentThreadId ())
                PostMessage(hWnd,WM_CLOSE,0,0);
            else if(((frame = meMessageGetFrame(hWnd)) != NULL) &&
                    !(frame->flags & meFRAME_NOT_FOCUS))
            {
                /* Use the normal command to save buffers and exit
                 * if we have focus.
                 * If it doesn't exit then carry on as normal
                 * Must ensure we ask the user, not a macro
                 */
                int savcle;
                savcle = clexec;
                clexec = meFALSE;
                if(!exitEmacs(1,3))
                    update(meTRUE);
                clexec = savcle;
                return meFALSE;
            }
            else if(anyChangedBuffer()
#if MEOPT_SPELL
                    || anyChangedDictionary()
#endif
#if MEOPT_REGISTRY
                    || anyChangedRegistry()
#endif
#if MEOPT_IPIPES
                    || anyActiveIpipe()
#endif
                    )
            {
                /* Display the modeless Cancel dialog box and disable the
                 * application window. */
                if(DialogBox(ttInstance,MAKEINTRESOURCE (IDD_QUITEXIT),
                             meFrameGetWinHandle(frameCur),(DLGPROC) WinQuitExit) == meFALSE)
                    return meFALSE;
            }
            exitEmacs(1,8);
        }
        break;
        
    case WM_DESTROY:
        /* if just closing down a frame the frame will already be unlinked so
         * we won't find the frame */
        if((frame = meMessageGetFrame(hWnd)) != NULL)
        {
            WinShutdown();
            /* End of call */
            PostQuitMessage(0);
        }
        break;
        
    case WM_DESTROYCLIPBOARD:
        if(clipState & CLIP_IGNORE_DC)
            clipState &= ~CLIP_IGNORE_DC;
        else if(clipState & CLIP_OWNER)
            clipState &= ~CLIP_OWNER;
        break;
        
    case WM_MOVE:
        if((frame = meMessageGetFrame(hWnd)) != NULL)
        {
            /* Move; Handle the position of the window changing. Need to force a
             * re-paint of then window. */
            InvalidateRect(hWnd,NULL,meFALSE);
            meFrameGetWinPaintAll(frame) = 1;
        }
        break;
        
    case WM_ERASEBKGND:
        if((frame = meMessageGetFrame(hWnd)) != NULL)
        {
            /* Indicate to the PAINT that we must honour the next draw region and
             * not try to optimise the painting otherwise we will not re-paint the
             * screen fully. Must invalidate the window incase we don't get a paint
             * message in which case the window will stop updating */
            InvalidateRect(hWnd,NULL,meFALSE);
            meFrameGetWinPaintAll(frame) = 1;
        }
        /* ME redraws all background in WM_PAINT so return done to reduce flicker */
        return 1;
        
    case WM_NCMOUSEMOVE:
        /* The mouse is in the non-client area, therefore the cursor may have changed,
         * reset the last lParam to force a reset of the cursor when it re-enters the
         * client area */
        setCursorLastLParam = -1;
        goto unhandled_message;
        
#if MEOPT_CLIENTSERVER
    case WM_USER:
        ttServerCheck = 1;
        break;
#endif
        
    default:
unhandled_message:
#if _WIN_DEBUG_MSG
        if(logfp != NULL)
        {
            fprintf(logfp,"Unhandled message %x %x %x\n",message, wParam, lParam);
            fflush(logfp);
        }
#endif
        return DefWindowProc(hWnd,message,wParam,lParam);
    }
    return meFALSE;
}
#endif /* _ME_WINDOW */

/*
 * TTtitle. Put the name of the buffer into the window frame
 */

void
meFrameSetWindowTitle(meFrame *frame, meUByte *str)
{
    static meUByte buf[meBUF_SIZE_MAX];           /* This must be static */
    meUByte *ss=buf;
    
    if(str != NULL)
    {
        meStrcpy(ss,str);
        ss += meStrlen(ss);
    }
    meStrcpy(ss," - ");
    ss += 3;
#if MEOPT_EXTENDED
    if(frameTitle != NULL)
        meStrcpy(ss,frameTitle);
    else
#endif
#ifdef _TITLE_VER_MINOR
        meStrcpy(ss,ME_FULLNAME " '" meVERSION "." meVERSION_MINOR);
#else
        meStrcpy(ss,ME_FULLNAME " '" meVERSION);
#endif
    
#ifdef _ME_CONSOLE
    if(meSystemCfg & meSYSTEM_CONSOLE)
        SetConsoleTitle((const char *) buf);	            /* Set console window title */
#ifdef _ME_WINDOW
    else
#endif /* _ME_WINDOW */
#endif /* _ME_CONSOLE */
#ifdef _ME_WINDOW
        SetWindowText(meFrameGetWinHandle(frame),(LPCSTR) buf);        /* Change the window text */
#endif /* _ME_WINDOW */
}

/* TTsetBgcol; Set the default fill of the background. Now in windows then we
 * have to modify the class window in order to effect the default background
 * fill. */
void
TTsetBgcol(void)
{
#ifdef _ME_WINDOW
#ifdef _ME_CONSOLE
    if(!(meSystemCfg & meSYSTEM_CONSOLE))
#endif /* _ME_CONSOLE */
    {
        HBRUSH newBrush;
        int bcol;
        
        /* Get the new background color scheme */
        bcol = meStyleGetBColor(meSchemeGetStyle(globScheme));
        if((newBrush = CreateSolidBrush(eCellMetrics.pInfo.cPal[bcol].cpixel)) != NULL)
        {
            meFrameLoopBegin();
            
            meFrameLoopContinue(loopFrame->flags & meFRAME_HIDDEN);
        
#ifdef _WIN64
            SetClassLongPtr(meFrameGetWinHandle(loopFrame),GCLP_HBRBACKGROUND,(LONG_PTR)(newBrush));
#else
            SetClassLong(meFrameGetWinHandle(loopFrame),GCL_HBRBACKGROUND,(LONG)(newBrush));
#endif
            
            meFrameLoopEnd();
            
            /* The new brush has been installed. Delete the old brush if we have defined it and remember the old context */
            if(ttBrush != NULL)
                DeleteObject(ttBrush);
            ttBrush = newBrush;
        }
        else if(newBrush != NULL)
            DeleteObject(newBrush);
        
    }
#endif /* _ME_WINDOW */
}
