/* -*- C -*- ****************************************************************
 *
 * System        : MicroEmacs Jasspa Distribution
 * Module        : winterm.c
 * Synopsis      : Win32 platform support
 * Created By    : Jon Green
 * Created       : 21/12/1996
 *
 * Description
 *
 * This is the windows terminal driver for the WIN32 build for Microsoft
 * windows environments.  This file contains the display and terminal input
 * functions for collecting and displaying data.
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
 * A sure indication that you have messed something up is the caret not
 * flashing - it might work but it will not be correct.
 *
 * History
 *
 * JG 150198 V1.7 Fixed the event handling following SP optimisations.
 * SP 100198 V1.6 Optimised the windows code.
 * JG 210697 V1.5 Fixed the character cell painting such that all paints now
 *                extend to the extremities of the screen. The screen edges
 *                were not being filled properly in previous versions.
 * JG 030497 V1.4 Added idle time key codes.
 * JG 210397 V1.3 Added idle mouse functionality for scroll bars.
 * JG 210197 V1.2 Screen sizing. Added new sizing policy for the window. Picked
 *                up the maximizing information to allow the screen to be placed
 *                correctly especially when the Windows '95 task bar is showing.
 * JG 200197 V1.1 Palette Fix. Added new palette management.
 * JG 211296 V1.0 Original Created.
 *
 ****************************************************************************
 *
 * Copyright (c) 1996-2000 Jon Green
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

#include <string.h>                     /* String functions */
#include <time.h>                       /* Time definitions */

#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL (WM_MOUSELAST+1)  // message that will be supported
                                        // by the OS
#endif

/* Emacs standard headers */
#include "emain.h"
#include "commdlg.h"                    /* Common dialogs */
#include "cderr.h"                      /* Common dialoge errors */
#include "evers.h"                      /* Version information */
#include "eskeys.h"                     /* Emacs special keys */
#include "winterm.h"                    /* Windows terminal definitions */
#include "wintermr.h"                   /* Windows resource file */

#include <process.h>
#ifdef _FULLDEBUG
#include <crtdbg.h>
#endif

/*FILE *logfp=NULL ;*/

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

/* External References */
extern LINE   *mline;                   /* Pointer to the message line */

/* Ini-file reference */
#define ME_INI_FILE    "ME32.INI"       /* Name of the ini file */

/* When we get a message that we can't completely handle we must dispatch the
 * message and let windows handle it. Used a define for easy searching */
#define meMessageHandler(msg) DispatchMessage(msg)

/* Macros to move into and out of screen <=> character space */
#define rowToClient(y) ((eCellMetrics.cell.sizeY * (y))+eCellMetrics.offsetY)
#define colToClient(x) ((eCellMetrics.cell.sizeX * (x))+eCellMetrics.offsetX)
#define clientToRow(y) (((y) - eCellMetrics.offsetY) / eCellMetrics.cell.sizeY)
#define clientToCol(x) (((x) - eCellMetrics.offsetX) / eCellMetrics.cell.sizeX)

/* Define the default search sections in the .ini file */
static char *iniSections [] =
{
    "Me" meVERSION meVERSION_MINOR meDAY,   /* [MeYYMMDD] */
    "Defaults",                             /* [Defaults] */
    NULL
};
/* define the user-name, used for the server file names */
static char *meUserName="guest" ;

int meInitGeom[4]={CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT } ;

/* Define the font file that we might want to load from the .ini file.
 * "fontId" is the font identifier that we search for in the file */
static char *fontId="fontfile" ;
static char *fontFile=NULL ;
static int   fontAdded=0 ;

/* CellMetrics
 * This structure defines the character font items used to address the
 * screen. Thise details the size of the font in screen space and
 * also contains the windows definition of the font.
 *
 * The charCellPosX/Y is a look up table for screen to client
 * coordinates. This is the text position.
 *
 * The screenCellPosX/Y is a lookup table for screen to client
 * coordinates. This is the cell position including padding
 * that appears around the edge of the screen.
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
    RECT canvas;                        /* Screen extents */
    MINMAXINFO minMaxInfo;              /* Window extent information */
    int maximized;                      /* Maximised flags */
    int offsetX;                        /* Offset of column 0 */
    int offsetY;                        /* Offset of row 0 */
    int leadingY;                       /* External leading between rows */
    CharMetrics cell;                   /* Metrics of a character cell */
    int cellX;                          /* Number of cells in x */
    int cellY;                          /* Number of cells in y */
    int16 screenCellNrow;               /* Screen cell number of rows */
    int16 screenCellNcol;               /* Screen cell number of columns */
    int16 *screenCellPosX;              /* Screen cell position in X */
    int16 *screenCellPosY;              /* Screen cell position in Y */
    int16 *charCellPosX;                /* Character cell position in X */
    int16 *charCellPosY;                /* Character cell position in Y */
    uint8 *charCellTmpX;                /* Temporary X position */
    int16 *paintStartCol;               /* Screen paint start col */
    int16 *paintEndCol;                 /* Screen paint end col */
    int    paintAll;                    /* Screen paint all flag */
    INT   *cellSpacing;                 /* Spacing of cells */
    HFONT fontdef[meFONT_MAX];          /* Definition of the font. */
    PaletteInfo pInfo;                  /* Emacs colour palette tables */
} CellMetrics;

#ifdef _DRAGNDROP
/* Retain a list of drag and drop files with thier raw screen position and
 * filename for subsequent processing. This is a single linked list of
 * filenames and screen positions. */
typedef struct s_DragAndDrop
{
    struct s_DragAndDrop *next;         /* Next drag and drop event */
    POINT mousePos;                     /* Position of the mouse */
    uint8 fname[1];                      /* Filename */
};

static struct s_DragAndDrop *dadHead = NULL;
#endif
#ifdef _CLIENTSERVER
HANDLE *clientHandle = INVALID_HANDLE_VALUE;
HANDLE *serverHandle = INVALID_HANDLE_VALUE;
HANDLE *connectHandle= INVALID_HANDLE_VALUE;
int ttServerSize = 0;
int ttServerToRead = 0;
#endif

CellMetrics eCellMetrics={0};           /* Cell metrics */
HWND   ttHwnd=NULL;                     /* This is the window handle */
#define ttHwndNull NULL
RECT   ttRect;                          /* Area of screen to update */
LONG APIENTRY
MainWndProc (HWND hWnd, UINT message, UINT wParam, LONG lParam) ;

/***************************************************************************/
#ifdef _WINCON
/* Windows NT Console screen buffer handle.  All console I/O is done through
 * this handle.
 */
static HANDLE hInput, hOutput;			/* Handles to console I/O */
static char chConsoleTitle[256];	    	/* Preserve the title of the console. */
static DWORD ConsoleMode, OldConsoleMode;	/* Current and old console modes */
static SMALL_RECT consolePaintArea={0};		/* Update area for console */
static CHAR_INFO *ciScreenBuffer = NULL;	/* Copy of screen buffer memory */
static COORD ConsoleSize, OldConsoleSize ;
uint32 *colTable=NULL ;				/* Currently allocated colour table */
#define consoleNumColors 16			/* Number of colours in console window */
static uint8 consoleColors[consoleNumColors*3] =
{
    0,0,0,    0,0,200, 0,200,0, 0,200,200, 200,0,0, 200,0,200, 200,200,0, 200,200,200,
    75,75,75, 0,0,255, 0,255,0, 0,255,255, 255,0,0, 255,0,255, 255,255,0, 255,255,255
} ;

/* Function prototypes */
static void
WinTermCellResize (int x, int y) ;
void
WinTermResize (void);
HANDLE
WinKillToClipboard (void);
#endif
/***************************************************************************/

/* Standard EMACS terminal definitions */
uint16 TTnrow = 0;              /* Number of rows */
uint16 TTncol = 0;              /* Number of columns */
uint16 TTmrow = 0;              /* Maximum number of rows */
uint16 TTmcol = 0;              /* Maximum number of columns */
uint16 TTmargin = 8;            /* Minimum size of margin. */
uint16 TTscrsiz = 64;

/* Local variables holding context state */
#define TIMER_INACTIVE      0           /* Inactive value for the mouse */

static int ttfcol = meCOLOR_FDEFAULT;   /* Foregound colour */
static int ttbcol = meCOLOR_BDEFAULT;   /* Background colour */
static int ttinitialised = 0;           /* Window has initialised. */
static int ttshowState;                 /* Show state of the window */
static int ttmodif = 0;                 /* Modified keyboard state */
HANDLE ttInstance;                      /* Instance of the application */
static DWORD ttThreadId = 0;            /* Current thread identity */
static HBRUSH ttBrush = NULL;           /* Current background brush */

/* Font type settings */
LOGFONT ttlogfont={0};                  /* Current logical font */

#if MOUSE
/* Local definitions for mouse handling code */
/* mouseState
 * A integer interpreted as a bit mask that holds the current state of
 * the mouse interaction. */
#define MOUSE_STATE_LEFT         0x0001 /* Left mouse button is pressed  */
#define MOUSE_STATE_MIDDLE       0x0002 /* Middle mouse button is pressed*/
#define MOUSE_STATE_RIGHT        0x0004 /* Right mouse button is pressed */
#define MOUSE_STATE_VISIBLE      0x0400 /* Mouse is currently visible    */
#define MOUSE_STATE_BUTTONS      (MOUSE_STATE_LEFT|MOUSE_STATE_MIDDLE|MOUSE_STATE_RIGHT)
#define MOUSE_STATE_LOCKED       0x0800 /* Mouse is locked in */

static UINT mouseButs = 0;              /* State of the mouse buttons. */
static int  mouseState =0;              /* State of the mouse. */
/* bit button lookup - [0] = no keys, [1] = left, [2]=middle, [4] = right */
static uint16 mouseKeys[8] = { 0, 1, 2, 0, 3, 0, 0, 0 } ;

#define mouseHide() ((mouseState & MOUSE_STATE_VISIBLE) ? (SetCursor(NULL),(mouseState &= ~MOUSE_STATE_VISIBLE)):0)
#define mouseShow() ((mouseState & MOUSE_STATE_VISIBLE) ? 0:(SetCursor(meCursors[meCurCursor]),(mouseState |= MOUSE_STATE_VISIBLE)))

/* Cursor definitions  */
static uint8 meCurCursor=0 ;
static HCURSOR meCursors[meCURSOR_COUNT]={NULL,NULL,NULL,NULL,NULL,NULL,NULL} ;
static LPCTSTR meCursorName[meCURSOR_COUNT]=
{
    IDC_ARROW,
    IDC_ARROW,
    IDC_IBEAM,
    IDC_CROSS,
    IDC_UPARROW,
    IDC_WAIT,
    IDC_NO
} ;

/* Convert the mouse coordinates to cell space. Compute the fractional bits
 * which are 1/128ths. Because we lock the mouse into the window then cater
 * for the -ve case by ANDing with 0x8000.
 * Added console support
 */
#define __winMousePosUpdate(lpos)                                            \
    if (LOWORD(lpos) & 0x8000)        /* Dirty check for -ve */              \
        mouse_X = 0, mouse_dX = 0;                                           \
    else                                                                     \
    {                                                                        \
        mouse_X = clientToCol (LOWORD(lpos));                                \
        mouse_dX = ((((LOWORD(lpos)) - colToClient(mouse_X)) << 8) /         \
                    eCellMetrics.cell.sizeX);                                \
        if (mouse_X > TTncol)                                                \
            mouse_X = TTncol;                                                \
    }                                                                        \
    if (HIWORD(lpos) & 0x8000)        /* Dirty check for -ve */              \
        mouse_Y = 0, mouse_dY = 0;                                           \
    else                                                                     \
    {                                                                        \
        mouse_Y = clientToRow (HIWORD(lpos));                                \
        mouse_dY = ((((HIWORD(lpos)) - rowToClient(mouse_Y)) << 8) /         \
                    eCellMetrics.cell.sizeY);                                \
        if (mouse_Y > TTnrow)                                                \
            mouse_Y = TTnrow;                                                \
    }

#ifdef _WINCON
#define mousePosUpdate(lpos)                                                 \
do {                                                                         \
    if (meSystemCfg & meSYSTEM_CONSOLE)                                      \
    {                                                                        \
	mouse_X = LOWORD(lpos) ;                                             \
	mouse_Y = HIWORD(lpos) ;                                             \
	mouse_dX = mouse_dY = 0 ;                                            \
    }                                                                        \
    else                                                                     \
    {                                                                        \
        __winMousePosUpdate(lpos)                                            \
    }                                                                        \
} while(0)
#else
#define mousePosUpdate(lpos)                                                 \
do {                                                                         \
    __winMousePosUpdate(lpos)                                                \
} while(0)
#endif

#endif /* MOUSE */

int platformId;                         /* Running under NT, 95, or Win32s? */

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
    if (!fWin32s)
        return 0;                       /* Not win32s */

    /* Register the 16bit DLL. We do this when we are called. This saves
       problems with a win32s 32-bit DLL under Win 3.1 with win32s installed. */
again:
    if (UTRegister (ttInstance,         /* 'me32s' module handle */
                    "methnk16.dll",     /* 16-bit thunk dll */
                    NULL,               /* Nothing to do */
                    "UTProc",           /* 16-bit dispatch routine */
                    &pfnUTProc,         /* Receives thunk address */
                    NULL,               /* No callback function */
                    NULL) == FALSE)     /* no shared memroy */
    {

        /* This fails the first time !! */
        if (doneOnce == 0)
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
    UTUnRegister (ttInstance);
    return status;
}
#endif

/* gettimeofday - Retreives the time of day to millisecond resolution */
void
gettimeofday (struct meTimeval *tp, struct meTimezone *tz)
{
    SYSTEMTIME stime;
    UNREFERENCED_PARAMETER (tz);

    /* Get the second resolution time */
    tp->tv_sec = time (NULL);

    /* Get the microsecond time */
    GetLocalTime (&stime);
    tp->tv_usec = (long)(stime.wMilliseconds * 1000);
}

/**************************************************************************
 *
 * Client/Server Functions
 *
 **************************************************************************/
#ifdef _CLIENTSERVER
void
TTopenClientServer (void)
{
    /* If the server has not been created then create it now */
    if (serverHandle == INVALID_HANDLE_VALUE)
    {
        meIPIPE *ipipe ;
        BUFFER *bp ;
        meMODE sglobMode ;
        uint8 fname [MAXBUF];
        int ii ;

        /* create the response file name */
        mkTempName (fname, meUserName, ".rsp");

        /* Open the response file for read/write, if this fails we are not the server, another
         * ME is */
        if ((clientHandle = CreateFile (fname,
                                        GENERIC_WRITE,
                                        FILE_SHARE_READ,
                                        NULL,
                                        CREATE_ALWAYS,
                                        FILE_ATTRIBUTE_NORMAL,
                                        NULL)) == INVALID_HANDLE_VALUE)
        {
            meSystemCfg &= ~meSYSTEM_CLNTSRVR ;
            return ;
        }
        /* Open command file for read/write */
        mkTempName (fname, meUserName, ".cmd");
        if ((serverHandle = CreateFile (fname,
                                        GENERIC_READ,
                                        FILE_SHARE_READ|FILE_SHARE_WRITE,
                                        NULL,
                                        CREATE_ALWAYS,
                                        FILE_ATTRIBUTE_NORMAL,
                                        NULL)) == INVALID_HANDLE_VALUE)
        {
            CloseHandle (clientHandle);
            clientHandle = INVALID_HANDLE_VALUE;
            meSystemCfg &= ~meSYSTEM_CLNTSRVR ;
            return ;
        }
        /* Create the ipipe buffer */
        meModeCopy(sglobMode,globMode) ;
        meModeSet(globMode,MDPIPE) ;
        meModeSet(globMode,MDLOCK) ;
        meModeSet(globMode,MDHIDE) ;
        meModeClear(globMode,MDWRAP) ;
        meModeClear(globMode,MDUNDO) ;
        if(((bp=bfind("*server*",BFND_CREAT)) == NULL) ||
           ((ipipe = meMalloc(sizeof(meIPIPE))) == NULL))
        {
            CloseHandle (clientHandle);
            CloseHandle (serverHandle);
            clientHandle = INVALID_HANDLE_VALUE;
            serverHandle = INVALID_HANDLE_VALUE ;
            meSystemCfg &= ~meSYSTEM_CLNTSRVR ;
            return ;
        }
        meModeCopy(globMode,sglobMode) ;
        bp->intFlag |= BIFNODEL ;
        ipipe->next = ipipes ;
        ipipe->pid = 0 ;
        ipipe->rfd = serverHandle ;
        ipipe->outWfd = clientHandle ;
        ipipe->process = 0 ;
        ipipe->processId = 0 ;
        ipipe->thread = NULL ;
        ipipe->childActive = NULL ;
        ipipe->threadContinue = NULL ;
        ipipes = ipipe ;
        noIpipes++ ;
        ipipe->bp = bp ;
        /* setup the response file and server buffer */
        {
            uint8 buff[MAXBUF] ;

            ii = sprintf((char *)buff,"%d\n",ttHwnd) ;
            WriteFile(clientHandle,buff,ii,&ii,NULL) ;

            sprintf((char *)buff,"Client Server: %s",fname) ;
            addLineToEob(bp,buff) ;     /* Add string */
            addLineToEob(bp,"") ;       /* Add string */
            addLineToEob(bp,"") ;       /* Add string */
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
        ipipe->curRow = (int16) bp->line_no ;
        /* get a popup window for the command output */
        ipipeSetSize(curwp,bp) ;
    }
}

int
TTcheckClientServer(void)
{
    int ii ;

    if (serverHandle == INVALID_HANDLE_VALUE)
        return 0 ;
    /* The handle exists. If the file is non-NULL then there is something to
     * do */
    ii = GetFileSize (serverHandle, NULL);
    if (ii == ttServerSize)
        return 0;
    ttServerToRead += ii - ttServerSize ;
    ttServerSize = ii;
    return 1 ;
}

void
TTkillClientServer (void)
{
    if (serverHandle != INVALID_HANDLE_VALUE)
    {
        meIPIPE *ipipe ;
        uint8 fname [MAXBUF];

        /* get the ipipe node */
        ipipe = ipipes ;
        while(ipipe != NULL)
        {
            if(ipipe->pid == 0)
                break ;
            ipipe = ipipe->next ;
        }
        /* if found (should be) delete buffer (this will close the file handles */
        if(ipipe != NULL)
        {
            ipipe->bp->intFlag |= BIFBLOW ;
            zotbuf(ipipe->bp,1) ;
        }
        else
        {
            /* Close and delete the server files */
            CloseHandle (serverHandle);
            CloseHandle (clientHandle);
        }
        clientHandle = INVALID_HANDLE_VALUE;
        serverHandle = INVALID_HANDLE_VALUE;
        meSystemCfg &= ~meSYSTEM_CLNTSRVR ;

        /* remove the command and response files */
        mkTempName (fname, meUserName, ".cmd");
        DeleteFile (fname);
        mkTempName (fname, meUserName, ".rsp");
        DeleteFile (fname);
    }
    if (connectHandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle (connectHandle);
        connectHandle = INVALID_HANDLE_VALUE;
        if(ttHwnd != NULL)
        {
            /* make this the current window */
            if (IsIconic (ttHwnd))
                ShowWindow (ttHwnd, SW_SHOWNORMAL);
            else
                SetForegroundWindow (ttHwnd);
        }
    }
}

int
TTconnectClientServer (void)
{
    /* If the server has not been connected then do it now */
    if (connectHandle == INVALID_HANDLE_VALUE)
    {
        HANDLE hndl;
        uint8 fname [MAXBUF];
        DWORD ii ;

        /* Create the file name, if the file exists, or deleting it
         * succeeds then there is no server, fial */
        mkTempName (fname, meUserName, ".cmd");
        if(meTestExist(fname) || DeleteFile (fname))
            return 0 ;
        if ((connectHandle = CreateFile (fname, GENERIC_WRITE, FILE_SHARE_READ, NULL,
                                         OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,NULL)) == INVALID_HANDLE_VALUE)
            return 0 ;
        /* Goto the end of the file */
        SetFilePointer (connectHandle,0,NULL,FILE_END);

        /* Try opening the response file and get the ttHwnd value */
        mkTempName (fname, meUserName, ".rsp");
        if ((hndl = CreateFile (fname, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL,
                                 OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) != INVALID_HANDLE_VALUE)
        {
            if((ReadFile(hndl,&fname,20,&ii,NULL) != 0) && (ii > 0))
                ttHwnd = (HWND) atoi(fname) ;
            CloseHandle (hndl);
        }
    }
    return 1 ;
}
void
TTsendClientServer(uint8 *line)
{
    if (connectHandle != INVALID_HANDLE_VALUE)
    {
        int ll=meStrlen(line) ;
        DWORD ww ;
        WriteFile(connectHandle,line,ll,&ww,NULL) ;
    }
}

#endif

#ifdef _WINCON
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
ConsolePaint (void)
{

    /* Work out region to update */
    if (consolePaintArea.Right > 0)
    {
        COORD coordUpdateBegin, coordBufferSize;

        /* Set size of screen buffer */
        coordBufferSize.X = TTncol;
        coordBufferSize.Y = TTnrow+1;

        consolePaintArea.Right-- ;
        /* top left cord of buffer to write from */
        coordUpdateBegin.X = consolePaintArea.Left ;
        coordUpdateBegin.Y = consolePaintArea.Top ;

        /* Write to console */
        WriteConsoleOutput(hOutput, ciScreenBuffer, coordBufferSize,
                           coordUpdateBegin, &consolePaintArea);

        /* Remove the region, as we just updated it */
        consolePaintArea.Right = consolePaintArea.Bottom = 0 ;
        consolePaintArea.Left = consolePaintArea.Top = (SHORT) 0x7fff ;
    }
    return 1 ;
}

/* Draw string to console buffer */
void
ConsoleDrawString(uint8 *ss, WORD wAttribute, int x, int y, int len)
{
    CHAR_INFO *pCI;     /* Pointer to current screen buffer location */
    BOOL bAny = FALSE;  /* Anything to refresh? */
    uint8 cc ;
    int r=x+len ;

    /* Get pointer to correct location in screen buffer */
    pCI = &ciScreenBuffer[(y * TTncol) + x];

    /* Copy the string to the screen buffer memory, and flag any changes */
    while (--len >= 0)
    {
        if (((cc=*ss++) != pCI->Char.AsciiChar) ||
            (wAttribute != pCI->Attributes))
        {
            bAny = TRUE;
            pCI->Char.AsciiChar = cc ;
            pCI->Attributes = wAttribute;
        }
        pCI++;
    }

    /* Adjust the current update region */
    if (bAny)
    {
        if (y < consolePaintArea.Top)
            consolePaintArea.Top = y ;
        if (y > consolePaintArea.Bottom)
            consolePaintArea.Bottom = y ;
        if (x < consolePaintArea.Left)
            consolePaintArea.Left = x ;
        if (r > consolePaintArea.Right)
            consolePaintArea.Right = r ;
    }
}

/* signal handler routine for console app */
BOOL WINAPI
ConsoleHandlerRoutine(DWORD dwCtrlType)
{
    /* Trap user's click on 'X' window button, and exit cleanly */
    switch (dwCtrlType)
    {
    case CTRL_BREAK_EVENT:
    case CTRL_CLOSE_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
        meDie() ;
    }
    return FALSE ;
}

/*
 * TTend
 * Close the terminal down
 */
int
TTend (void)
{
    if (meSystemCfg & meSYSTEM_CONSOLE)
    {
        CONSOLE_CURSOR_INFO CursorInfo;
        COORD dwCursorPosition;
        
        /* restore the console buffer size */
        SetConsoleScreenBufferSize(hOutput, OldConsoleSize);

        /* Restore the console mode and title */
        SetConsoleMode(hInput, OldConsoleMode);
        SetConsoleTitle(chConsoleTitle);

        /* Show the cursor */
        GetConsoleCursorInfo (hOutput, &CursorInfo);
        CursorInfo.bVisible = TRUE;
        SetConsoleCursorInfo (hOutput, &CursorInfo);

        /* Set cursor to bottom of screen, and print a newline */
        dwCursorPosition.X = 0;
        dwCursorPosition.Y = ConsoleSize.Y - 1;
        SetConsoleCursorPosition(hOutput, dwCursorPosition);
        putchar ('\n');
    }
    return TRUE ;
}

/* Get a console message and format as a standard windows message */
int
meGetConsoleMessage(MSG *msg)
{
    INPUT_RECORD ir;
    DWORD dwCount;

    /* If in console mode, check whether we must render the clipboard */
    if (clipState & CLIP_OWNER)
    {
        HANDLE hmem;

        hmem = WinKillToClipboard ();
        if (hmem == NULL)
            hmem = GlobalAlloc (GHND, 1);

        OpenClipboard (NULL);
        EmptyClipboard ();
        SetClipboardData (CF_OEMTEXT, hmem);
        CloseClipboard ();

        clipState &= ~CLIP_OWNER;
    }

    /* Get the next keyboard/mouse/resize event */
    ReadConsoleInput(hInput, &ir, 1, &dwCount);

    /* Let the proper event handler field this event */
    if (ir.EventType == KEY_EVENT)
    {
        KEY_EVENT_RECORD *kr = &ir.Event.KeyEvent;

        /* Make message a la windows GUI */
        msg->lParam = 0;
        /* SWP - 8/5/99 another odd one from bill, the cursor keys on win9? seem
         * to come through with an AsciiChar value of -32 or 224 instead of 0... Why? */
        if ((kr->uChar.AsciiChar == 0) || (((uint8) kr->uChar.AsciiChar) == 224))
        {
            /* WM_KEYDOWN of WM_KEYUP */
            if (kr->bKeyDown)
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
        else if (kr->bKeyDown)
        {
            msg->message = WM_CHAR;
            msg->wParam = (uint8) kr->uChar.AsciiChar;
        }
        else
            msg->message = 0;

        /* if we filled in a message, then success! */
        if (msg->message != 0)
        {
            /* Set up the ALT key state also */
            if ((kr->dwControlKeyState & LEFT_ALT_PRESSED) ||
                (kr->dwControlKeyState & RIGHT_ALT_PRESSED))
                msg->lParam |= 1<<29;
            if(kr->dwControlKeyState & ENHANCED_KEY)
                msg->lParam |= 0x01000000 ;
            return TRUE ;
        }
    }
    else if (ir.EventType == MOUSE_EVENT)
    {
        MOUSE_EVENT_RECORD *mr = &ir.Event.MouseEvent;
        static DWORD dwLastButtonState = 0;
        DWORD dwButtonState ;

        dwButtonState = dwLastButtonState ^ mr->dwButtonState;

        /* Has the state of the mouse buttons changed? */
        if (dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)
        {
            if (dwLastButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)
                msg->message = WM_LBUTTONUP;
            else
                msg->message = WM_LBUTTONDOWN;
            msg->wParam |= MK_LBUTTON;
        }
        else if (dwButtonState & FROM_LEFT_2ND_BUTTON_PRESSED)
        {
            if (dwLastButtonState & FROM_LEFT_2ND_BUTTON_PRESSED)
                msg->message = WM_MBUTTONUP;
            else
                msg->message = WM_MBUTTONDOWN;
            msg->wParam |= MK_MBUTTON;
        }
        else if (dwButtonState & RIGHTMOST_BUTTON_PRESSED)
        {
            if (dwLastButtonState & RIGHTMOST_BUTTON_PRESSED)
                msg->message = WM_RBUTTONUP;
            else
                msg->message = WM_RBUTTONDOWN;
            msg->wParam |= MK_RBUTTON;
        }
#ifdef WM_MOUSEWHEEL
#ifdef MOUSE_WHEELED
        else if (mr->dwEventFlags & MOUSE_WHEELED)
        {
            msg->message = WM_MOUSEWHEEL;
            /* Haven't got NT5 so cant see it working and as usual the MS docs are crap
             * so I don't know whether this was a wheel up or down event */
            msg->wParam  = (1) ? 0x0:0x80000000 ;
        }
#endif
#endif
        /* Mouse moved? */
        else if (mr->dwEventFlags & MOUSE_MOVED)
            msg->message = WM_MOUSEMOVE;
        else
            msg->message = 0;

        /* Remember state for next time */
        dwLastButtonState = mr->dwButtonState;

        /* got anything to send? */
        if (msg->message != 0)
        {
            /* Build the wParam with mouse buttons and key states */
#ifdef WM_MOUSEWHEEL
            if(msg->message != WM_MOUSEWHEEL)
#endif
            {
                msg->wParam = 0;
                if (mr->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)
                    msg->wParam |= MK_LBUTTON;
                if (mr->dwButtonState & FROM_LEFT_2ND_BUTTON_PRESSED)
                    msg->wParam |= MK_MBUTTON;
                if (mr->dwButtonState & RIGHTMOST_BUTTON_PRESSED)
                    msg->wParam |= MK_RBUTTON;
            }
            if ((mr->dwControlKeyState & RIGHT_CTRL_PRESSED) ||
                (mr->dwControlKeyState & LEFT_CTRL_PRESSED))
                msg->wParam |= MK_CONTROL;
            if (mr->dwControlKeyState & SHIFT_PRESSED)
                msg->wParam |= MK_SHIFT;
            /* Set up mouse position */
            msg->lParam = ((mr->dwMousePosition.Y) << 16) | mr->dwMousePosition.X;

            return TRUE ;
        }
    }
    else if (ir.EventType == WINDOW_BUFFER_SIZE_EVENT)
    {
        /* in true MS style the console resize stuff is a complete pain! The
         * user can change the buffer size and we get the change, but this is
         * no use to us as we want window size changes. Instead we have to get
         * the current window size and change the screen size to that.
         */
        CONSOLE_SCREEN_BUFFER_INFO Console;
        COORD size ;
        
        GetConsoleScreenBufferInfo(hOutput, &Console);
        /* this should be the window size, not the buffer size
         * as this needs the scroll-bar to use */
        size.X = Console.srWindow.Right-Console.srWindow.Left+1;
        size.Y = Console.srWindow.Bottom-Console.srWindow.Top+1;

        SetConsoleScreenBufferSize(hOutput, size);

        /* Tell micro-emacs about it */
        WinTermCellResize (size.X,size.Y) ;
    }
    else if (ir.EventType == FOCUS_EVENT)
    {
        if(ir.Event.FocusEvent.bSetFocus)
        {
            /* Record the fact we have focus */
            TTfocus = 1;

            /* Kick of the blinker - as default value for cursorBlink
             * is 0 this will not happen until after the window is created */
            if((cursorState >= 0) && cursorBlink)
                TThandleBlink(2) ;
        }
        else
        {
            TTfocus = 0;
            if(cursorState >= 0)
            {
                /* because the cursor is a part of the solid cursor we must
                 * remove the old one first and then redraw
                 */
                if(blinkState)
                    TThideCur() ;
                blinkState = 1 ;
                TTshowCur() ;
            }
        }
    }
    else
    {
        /* Menu or focus events, so do nowt */
        /* printf("Got unknown console input %d\n",ir.EventType) ;*/
    }
    return FALSE ;
}

#endif

/****************************************************************************
 *
 * WINDOWS SCREEN FUNCTIONS
 *
 ****************************************************************************/

/* WinSpecialChar; Draw a special character to the screen. x is the lefthand
 * edge of the character, y is the top of the character. The background is
 * assumed to be filled. A foreground pen is expected to be selected. */
static void
WinSpecialChar (HDC hdc, CharMetrics *cm, int x, int y, uint8 cc, COLORREF fcol)
{
    POINT points [3];                   /* Triangular points */
    int ii ;

    /* Fill in the character */
    switch (cc)
    {
    case 0x07:          /* Line space '.' */
        MoveToEx (hdc, x + cm->midX, y + cm->midY, NULL);
        LineTo   (hdc, x + cm->midX + 1, y + cm->midY);
        break;
    
    case 0x09:          /* Line & Poly / Tab -> */
        ii = ((cm->midY+1) >> 1) ;
        MoveToEx (hdc, x, y + cm->midY, NULL);
        LineTo   (hdc, x + cm->midX - 1, y + cm->midY);
        points [0].x = x + cm->midX - 1 ;
        points [0].y = y + ii ;
        points [1].x = x + cm->midX - 1 ;
        points [1].y = y + cm->sizeY - ii - 1 ;
        points [2].x = x + cm->sizeX - 2 ;
        points [2].y = y + cm->midY;
        goto makePoly;

    case 0x0a:          /* Line & Poly / CR <-| */
        ii = ((cm->midY+1) >> 1) ;
        MoveToEx (hdc, x + cm->midX,      y + cm->midY, NULL);
        LineTo   (hdc, x + cm->sizeX - 2, y + cm->midY);
        LineTo   (hdc, x + cm->sizeX - 2, y + ii - 2) ;
        points [0].x = x + cm->midX ;
        points [0].y = y + ii ;
        points [1].x = x + cm->midX ;
        points [1].y = y + cm->sizeY - ii - 1 ;
        points [2].x = x + 1 ;
        points [2].y = y + cm->midY;
        goto makePoly;

    case 0x0b:          /* Line Drawing / Bottom right _| */
        MoveToEx (hdc, x, y + cm->midY, NULL);
        LineTo   (hdc, x + cm->midX, y + cm->midY);
        LineTo   (hdc, x + cm->midX, y - 1);
        break;

    case 0x0c:          /* Line Drawing / Top right */
        MoveToEx (hdc, x, y + cm->midY, NULL);
        LineTo   (hdc, x + cm->midX, y + cm->midY);
        LineTo   (hdc, x + cm->midX, y + cm->sizeY + 1);
        break;

    case 0x0d:          /* Line Drawing / Top left */
        MoveToEx (hdc, x + cm->sizeX, y + cm->midY, NULL);
        LineTo   (hdc, x + cm->midX, y + cm->midY);
        LineTo   (hdc, x + cm->midX, y + cm->sizeY + 1);
        break;

    case 0x0e:          /* Line Drawing / Bottom left |_ */
        MoveToEx (hdc, x + cm->midX, y, NULL);
        LineTo   (hdc, x + cm->midX, y + cm->midY);
        LineTo   (hdc, x + cm->sizeX + 1, y + cm->midY);
        break;

    case 0x0f:          /* Line Drawing / Centre cross + */
        MoveToEx (hdc, x, y + cm->midY, NULL);
        LineTo   (hdc, x + cm->sizeX + 1, y + cm->midY);
        MoveToEx (hdc, x + cm->midX, y, NULL);
        LineTo   (hdc, x + cm->midX, y + cm->sizeY + 1);
        break;

    case 0x10:          /* Cursor Arrows / Right */
        points [0].x = x;
        points [0].y = y + 1;
        points [1].x = x;
        points [1].y = y + cm->sizeY - 1;
        points [2].x = x + cm->sizeX;
        points [2].y = y + cm->midY;
        goto makePoly;

    case 0x11:          /* Cursor Arrows / Left */
        points [0].x = x + cm->sizeX;
        points [0].y = y + 1;
        points [1].x = x + cm->sizeX;
        points [1].y = y + cm->sizeY - 1;
        points [2].x = x;
        points [2].y = y + cm->midY;
        goto makePoly;

    case 0x12:          /* Line Drawing / Horizontal line - */
        MoveToEx (hdc, x, y + cm->midY, NULL);
        LineTo   (hdc, x + cm->sizeX + 1, y + cm->midY);
        break;

    case 0x15:          /* Line Drawing / Left Tee |- */
        MoveToEx (hdc, x + cm->midX, y, NULL);
        LineTo   (hdc, x + cm->midX, y + cm->sizeY + 1);
        MoveToEx (hdc, x + cm->midX, y + cm->midY, NULL);
        LineTo   (hdc, x + cm->sizeX + 1, y + cm->midY);
        break;

    case 0x16:          /* Line Drawing / Right Tee -| */
        MoveToEx (hdc, x + cm->midX, y, NULL);
        LineTo   (hdc, x + cm->midX, y + cm->sizeY + 1);
        MoveToEx (hdc, x, y + cm->midY, NULL);
        LineTo   (hdc, x + cm->midX, y + cm->midY);
        break;

    case 0x17:          /* Line Drawing / Bottom Tee _|_ */
        MoveToEx (hdc, x,  y + cm->midY, NULL);
        LineTo   (hdc, x + cm->sizeX + 1, y + cm->midY);
        MoveToEx (hdc, x + cm->midX, y, NULL);
        LineTo   (hdc, x + cm->midX, y + cm->midY);
        break;

    case 0x18:          /* Line Drawing / Top Tee -|- */
        MoveToEx (hdc, x,  y + cm->midY, NULL);
        LineTo   (hdc, x + cm->sizeX + 1, y + cm->midY);
        MoveToEx (hdc, x + cm->midX, y + cm->sizeY, NULL);
        LineTo   (hdc, x + cm->midX, y + cm->midY);
        break;

    case 0x19:          /* Line Drawing / Vertical Line | */
        MoveToEx (hdc, x + cm->midX, y, NULL);
        LineTo   (hdc, x + cm->midX, y + cm->sizeY + 1);
        break;

    case 0x1e:          /* Cursor Arrows / Up */
        {
            int hh ;
            if((hh = cm->sizeY-cm->sizeX-2) < 0)
                hh = 0 ;

            points [0].x = x + cm->midX;
            points [0].y = y + hh ;
            points [1].x = x + cm->sizeX - 1;
            points [1].y = y + cm->sizeY - 2;
            points [2].x = x ;
            points [2].y = y + cm->sizeY - 2;
            goto makePoly;
        }

    case 0x1f:          /* Cursor Arrows / Down */
        {
            int hh ;
            if((hh = cm->sizeX+1) >= cm->sizeY)
                hh = cm->sizeY-1 ;

            points [0].x = x ;
            points [0].y = y + 1;
            points [1].x = x + cm->sizeX - 1;
            points [1].y = y + 1;
            points [2].x = x + cm->midX ;
            points [2].y = y + hh ;
        }
        /* Construct the polygon */
makePoly:
        {
            HBRUSH obrush;
            HBRUSH fbrush;

            fbrush = CreateSolidBrush (fcol);
            obrush = (HBRUSH) SelectObject (hdc, fbrush);
            SetPolyFillMode (hdc, WINDING);
            Polygon (hdc, points, 3);
            SelectObject (hdc, obrush);
            DeleteObject (fbrush);
        }
        break;
    }
}



/* WinDrawCursor; Draw the cursor on the screen. We use the information from
 * the frame store to determine what character to render */
void
WinDrawCursor(HDC hdc)
{
    RECT rline;                         /* Rectangle of line */
    int clientRow;                      /* Text region row start - client units */
    int clientCol;                      /* Text region col start - client units */
    FRAMELINE *flp;                     /* Frame store line pointer */
    meSTYLE style;                      /* Current style */
    uint8 cc ;                          /* Current char  */

    /* Set up the drawing borders. */
    rline.top    = eCellMetrics.screenCellPosY [TTcurr];
    rline.bottom = eCellMetrics.screenCellPosY [TTcurr+1];
    rline.left   = eCellMetrics.screenCellPosX [TTcurc];
    rline.right  = eCellMetrics.screenCellPosX [TTcurc+1];
    /* Set up text start position */
    clientRow = eCellMetrics.charCellPosY [TTcurr];
    clientCol = eCellMetrics.charCellPosX [TTcurc];

    flp = frameStore + TTcurr ;
    cc = flp->text[TTcurc];                          /* Get char under cursor */
    style = meSchemeGetStyle(flp->scheme[TTcurc]) ;  /* Get style under cursor */

    /* Check the current character set and select the special font or convert
     * the character for the current font. Bring the appropriate font into the
     * device context. */
    if (((cc & 0xe0) == 0) && (meSystemCfg & meSYSTEM_FONTFIX))
    {
	HBRUSH brush;
	HPEN pen;                   /* Foreground pen */
	HPEN oldpen;                /* first pen */
	COLORREF cref;              /* Background color */

	if (!TTfocus)
	    cref = eCellMetrics.pInfo.cPal [meStyleGetBColor(style)].cpixel;
	else
	    cref = eCellMetrics.pInfo.cPal [cursorColor].cpixel;

	/* Fill in the background */
	brush = CreateSolidBrush (cref);
	FillRect (hdc, &rline, brush);
	DeleteObject (brush);

	/* Create a pen to draw the character */
	pen = CreatePen (PS_SOLID, 0, eCellMetrics.pInfo.cPal [meStyleGetFColor(style)].cpixel);
	oldpen = SelectObject (hdc, pen);

	/* This is a special character, render the character to the
	 * screen. We need to create a pen to handle the object */
	WinSpecialChar (hdc, &eCellMetrics.cell, rline.left, rline.top, cc, 
                        eCellMetrics.pInfo.cPal [meStyleGetFColor(style)].cpixel);

	if (!TTfocus)
	{
	    /* If there is no focus then put a rectangle around the
	     * object; bring in the backgound color */
	    pen = CreatePen (PS_SOLID, 0, eCellMetrics.pInfo.cPal [cursorColor].cpixel);
	    pen = SelectObject (hdc, pen);
	    DeleteObject (pen);

	    /* Draw the rectangle */
	    MoveToEx (hdc, rline.top - 1, rline.left - 1, NULL);
	    LineTo (hdc, rline.top + eCellMetrics.cell.sizeY + 1, rline.left - 1);
	    LineTo (hdc, rline.top + eCellMetrics.cell.sizeY + 1, rline.left + eCellMetrics.cell.sizeX + 1);
	    LineTo (hdc, rline.top - 1, rline.left + eCellMetrics.cell.sizeX + 1);
	    LineTo (hdc, rline.top - 1, rline.left - 1);
	}
	pen = SelectObject (hdc, oldpen);
	DeleteObject (pen);

	/* Finished !! */
	return;
    }

    /* Set up the font */
    SetTextColor (hdc, eCellMetrics.pInfo.cPal [meStyleGetBColor(style)].cpixel);
    SetBkColor (hdc, eCellMetrics.pInfo.cPal [cursorColor].cpixel);
    SelectObject (hdc, eCellMetrics.fontdef [meStyleGetFont(style)]);

    /* Output the character */
    ExtTextOut (hdc,
                clientCol,                /* Text start position */
                clientRow,
                ETO_OPAQUE|ETO_CLIPPED,   /* Fill background */
                &rline,                   /* Background area */
                &cc,                      /* Text string */
                1,                        /* Length of string */
                eCellMetrics.cellSpacing);

    if(!TTfocus)
    {
        /* on top draw the normal character but smaller, this creates the
         * rectangle effect */
        SetTextColor (hdc, eCellMetrics.pInfo.cPal [meStyleGetFColor(style)].cpixel);
        SetBkColor (hdc, eCellMetrics.pInfo.cPal [meStyleGetBColor(style)].cpixel);

        rline.top++ ;
        rline.bottom-- ;
        rline.left++ ;
        rline.right-- ;

        ExtTextOut (hdc,
                    clientCol,      /* Text start position */
                    clientRow,
                    ETO_CLIPPED,    /* Clip char to smaller rectangle */
                    &rline,         /* Background area */
                    &cc,            /* Text string */
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
            ttlogfont.lfWeight = FW_BOLD ;
        else if(font & meFONT_LIGHT)
            ttlogfont.lfWeight = FW_EXTRALIGHT ;
        else
            ttlogfont.lfWeight = FW_NORMAL ;
        ttlogfont.lfItalic    = (font & meFONT_ITALIC)    ? TRUE : FALSE;
        ttlogfont.lfUnderline = (font & meFONT_UNDERLINE) ? TRUE : FALSE;
        
        /* Create the font - use the existing font if it exists */
        if ((eCellMetrics.fontdef[font] = CreateFontIndirect (&ttlogfont)) == NULL)
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
WinPaint (HWND hWnd)
{
    PAINTSTRUCT ps;                     /* Paint structure */
    RECT rline;                         /* Rectangle of line */
    int col;                            /* Current column position */
    meSTYLE schm;                       /* Current style */
    meCOLOR font;                       /* Text font type */
    meCOLOR fcol;                       /* Foreground colour */
    meCOLOR bcol;                       /* Background colour */
    meCOLOR pfcol;                      /* Pen foreground colour */
    meCOLOR pbcol;                      /* Pen background colour */
    int srow;                           /* Start row */
    int erow;                           /* End row */
    int scol;                           /* Start column */
    int ecol;                           /* End column */
    int clientRow;                      /* Text region row start - client units */
    FRAMELINE *flp;                     /* Frame store line pointer */
    int drawCursor;                     /* draw cursor flag */
    HBRUSH bbrush = NULL;               /* Background brush */
    HPEN pen;                           /* Foreground pen */
    HPEN oldpen = NULL;                 /* fist pen */
#define DEBUG_BG 0
#if DEBUG_BG
    static uint8 bgcol=0 ;
    if(++bgcol >= noColors)
        bgcol=0 ;
#endif
    BeginPaint (hWnd, &ps);

    /* I'm not sure if I need these here or not ?? */
    SetMapMode (ps.hdc, MM_TEXT);       /* Text mode */
    SetMapperFlags (ps.hdc, 1);         /* Allow interpolation */

    /* quick check to see if theres anything to do */
    if ((ps.rcPaint.top == ps.rcPaint.bottom) ||
        (ps.rcPaint.left == ps.rcPaint.right))
    {
        EndPaint(hWnd, &ps);
        return;
    }

#if (meFONT_MAX == 0)
    /* Change the font and colour space. */
    SelectObject (ps.hdc, eCellMetrics.fontdef);
#endif
    if (eCellMetrics.pInfo.hPal != NULL)
    {
        SelectPalette (ps.hdc, eCellMetrics.pInfo.hPal, FALSE);
        RealizePalette (ps.hdc);
    }

    /* Initialise some variables now. */
    schm = meSCHEME_INVALID ;
    font = meCOLOR_INVALID ;
    fcol = meCOLOR_INVALID ;
    bcol = meCOLOR_INVALID ;
    pfcol = meCOLOR_INVALID ;
    pbcol = meCOLOR_INVALID ;

    /* Convert the paint region from client coordinates to screen coordinates.
     * Note that the end row/column is rounded down at the sub-pixel level. */
    srow = clientToRow (ps.rcPaint.top);
    erow = clientToRow (ps.rcPaint.bottom + eCellMetrics.cell.sizeY - 1);
    if (erow > TTnrow)
        erow = TTnrow;

    scol = clientToCol (ps.rcPaint.left);
    ecol = clientToCol (ps.rcPaint.right + eCellMetrics.cell.sizeX - 1);
    if (ecol > TTncol)
        ecol = TTncol;
    
    /* Redraw the cursor if we have zapped it */
    if ((cursorState >= 0) && blinkState)
        drawCursor = ((srow <= TTcurr) && (erow >= TTcurr) &&
                      (scol <= TTcurc) && (ecol > TTcurc)) ;
    else
        drawCursor = 0 ;

    /* Process each row in turn until we reach the end of the line */
    for (flp = frameStore + srow; srow <= erow; srow++, flp++)
    {
        meSCHEME *fschm;
        uint8 *tbp, cc;
        uint8 *ftext;
        int   length;
        int   tcol, spFlag;
        
        /* Determine the boundaries we are painting around */
        if(eCellMetrics.paintAll)
            col = ecol;
        else if((col = eCellMetrics.paintEndCol[srow]) > 0)
            scol = eCellMetrics.paintStartCol[srow] ;
        else
            continue ;
        
        /* Reset the paint extremities - we set these for optimisation
         * purposes. */
        eCellMetrics.paintStartCol[srow] = TTncol ;
        eCellMetrics.paintEndCol[srow] = 0 ;
        tbp = eCellMetrics.charCellTmpX;

        /* Set up the drawing borders. */
        rline.top    = eCellMetrics.screenCellPosY [srow];
        rline.bottom = eCellMetrics.screenCellPosY [srow+1];
        rline.right  = eCellMetrics.screenCellPosX [col];
        
        /* Set up text start position */
        clientRow = eCellMetrics.charCellPosY [srow];

        /* As we render right to left then we start with the end character - 1 */
        col--;
        
        ftext = flp->text ;              /* Point to appropriate text block */
        fschm = flp->scheme + col ;      /* Point to appropriate colour block  */

        for (;;)
        {
	    if(schm != *fschm)
	    {
		/* Set up the colour change */
                meSTYLE style ;
                uint8 ff ;
                
		schm = *fschm ;
                style = meSchemeGetStyle(schm) ;
		ff = (uint8) meStyleGetFColor(style) ;
		if (fcol != ff)
		{
		    fcol = ff ;
		    SetTextColor (ps.hdc, eCellMetrics.pInfo.cPal[fcol].cpixel);
		}
		ff = (uint8) meStyleGetBColor(style) ;
#if DEBUG_BG
                ff = bgcol ;
#endif
                if (bcol != ff)
		{
		    bcol = ff ;
		    SetBkColor (ps.hdc, eCellMetrics.pInfo.cPal[bcol].cpixel);
		}
#if meFONT_MAX
                ff = (uint8) meStyleGetFont(style) ;
                
                /* If there is a modification on the font then apply it now.
                 * Note that the following looks a little cumbersome and
                 * unecessary, however the compiler will reduce the first pair
                 * of expressions into a single test so we only enter the
                 * conditional block when we need to. Both of the following
                 * are applied at the end of the line and occur infrequently. */
                if(meSchemeTestNoFont(schm))
                {
                    ff &= ~(meFONT_BOLD|meFONT_ITALIC|meFONT_UNDERLINE) ;
                }
                    
                if (font != ff)
                {
		    font = ff ;
                    if(eCellMetrics.fontdef[font] == NULL)
                        WinLoadFont(font) ;
                    /* Change font */
                    SelectObject (ps.hdc, eCellMetrics.fontdef[font]);
                }
#endif
	    }

	    /* how many characters can we draw until we get a color change or
	     * reach the end */
	    tcol = col;
	    spFlag = 0;
	    do
	    {
		cc = ftext[col] ;
		if((meSystemCfg & meSYSTEM_FONTFIX) && ((cc & 0xe0) == 0))
                {
                    spFlag++ ;
                    cc = ' ' ;
                }
                tbp[col] = cc ;
            } while((--col >= scol) && (*--fschm == schm)) ;
            
            
	    /* Output the current text item. Set up the current left margin
             * and determine the length of text that we have to output. */
	    length = tcol - col;
            col++;                      /* Move to current position */
	    rline.left = eCellMetrics.screenCellPosX [col];

	    /* Output regular text */
	    ExtTextOut (ps.hdc,
			eCellMetrics.charCellPosX [col], /* Text start position */
			clientRow,
			ETO_OPAQUE,     /* Fill background */
			&rline,         /* Background area */
			tbp+col,        /* Text string */
			length,         /* Length of string */
			eCellMetrics.cellSpacing);
            col--;                      /* Restore position */
            
            /* Special characters */
	    if (spFlag != 0)
            {
                /* Correct the pen colours if required. We do this here
                 * because we expect less special characters. We do not want
                 * to create and delete new pens unless we really are going to
                 * use them. */
                if (pfcol != fcol)
                {
		    HPEN p;
                    pfcol = fcol;
		    /* Set up for line drawing */
		    pen = CreatePen (PS_SOLID, 0, eCellMetrics.pInfo.cPal [fcol].cpixel);
		    p = SelectObject (ps.hdc, pen);
		    if (oldpen == NULL)
			oldpen = p;
		    else
			DeleteObject (p);
                }
                if (pbcol != bcol)
                {
                    pbcol = bcol;
		    /* Set up for line drawing */
		    if (bbrush != NULL)
			DeleteObject (bbrush);
		    bbrush = CreateSolidBrush (eCellMetrics.pInfo.cPal [bcol].cpixel);
                }
                /* Render the special characters */
                do
                {
                    while (((cc=ftext[tcol]) & 0xe0) != 0)
                        tcol-- ;
                    
                    WinSpecialChar (ps.hdc, &eCellMetrics.cell,
                                    eCellMetrics.screenCellPosX [tcol],
                                    rline.top, ftext[tcol],
                                    eCellMetrics.pInfo.cPal [fcol].cpixel) ;
                    tcol-- ;
                }
                while(--spFlag > 0);
            }

	    /* are we at the end?? */
	    if(scol > col)
		break ;

	    rline.right = rline.left;
	}
    }
    if (drawCursor)
        WinDrawCursor(ps.hdc) ;

    eCellMetrics.paintAll = 0 ;

    /* Relinquish the resources */
    if (oldpen != NULL)
    {
        oldpen = SelectObject (ps.hdc, oldpen);
        DeleteObject (oldpen);
    }

    if (bbrush)
        DeleteObject (bbrush);

    EndPaint(hWnd, &ps);
}

/*
 * WinKillToClipboard
 * Copy the data into the clipboard from the kill buffer.
 */
static HANDLE
WinKillToClipboard (void)
{
    HANDLE hmem;                        /* Windows global memory handle */
    uint8 *bufp;                        /* Windows global memory pointer */
    KILL *killp;                        /* Pointer to the kill data */
    uint8 cc;                           /* Local character pointer */
    uint8 *dd;                          /* Pointer to the kill data */
    int killSize = 0;                   /* Number of bytes in kill buffer */

    /* Determine the size of the data in the kill buffer.
     * Make sure that \r\n are appended to the end of each
     * line. */
    if (klhead != NULL)
    {
        for (killp = klhead->kill; killp != NULL; killp = killp->next)
        {
            for (dd = killp->data; (cc = *dd++) != '\0'; killSize++)
                if (cc == meNLCHAR)
                    killSize++; /* Add 1 for the '\r' */
        }
    }

    /* Create global buffer for the data */
    hmem = GlobalAlloc (GMEM_MOVEABLE, killSize + 1);
    bufp = GlobalLock (hmem);

    /* Copy the data into the buffer */
    if (klhead != NULL)
    {
        for (killp = klhead->kill; killp != NULL; killp = killp->next)
        {
            dd = killp->data;
            while((cc = *dd++))
            {
                /* Convert the end of line to CR/LF */
                if (cc == meNLCHAR)
                    *bufp++ = '\r';
                /* Convert any special characters */
                else if ((meSystemCfg & meSYSTEM_FONTFIX) && (cc < TTSPECCHARS))
                    cc = ttSpeChars [cc];
                /* Copy in the character */
                *bufp++ = cc;
            }
        }
    }

    /* NULL terminate the buffer and unlock */
    *bufp = '\0';                       /* Null terminate string */
    GlobalUnlock (hmem);                /* Unlock the memory region */
    return (hmem);
}

/*
 * TTsetClipboard
 * Make the information available to other applications.
 * Do not copy the data simply mark the clipboard signalling
 * that data is available.
 */
void
TTsetClipboard (void)
{
    clipState &= ~CLIP_TRY_SET ;
    /* We aquire the clipboard and flush it under the following conditions;
     * "We do NOT own it" or "Clipboard is stale". The clipboard becomes stale
     * when we own it but another application has aquired our clipboard data.
     * In this case we need to reset the clipboard so that the application may
     * aquire our next data block that has changed. */
    if (((clipState ^ CLIP_OWNER) & (CLIP_OWNER|CLIP_STALE)) &&
        (OpenClipboard(ttHwnd) == TRUE))
    {
        EmptyClipboard ();
        SetClipboardData (((ttlogfont.lfCharSet == OEM_CHARSET) ? CF_OEMTEXT : CF_TEXT), NULL);
        CloseClipboard ();
        clipState |= CLIP_OWNER ;
        clipState &= ~CLIP_STALE ;
    }
}

/*
 * TTgetClipboard.
 * Pop the contents of the clipboard into the kill buffer ready for
 * a yank. */
void
TTgetClipboard(void)
{
    HANDLE hmem;                        /* Windows clipboard memory handle */
    uint8 *bufp;                        /* Clipboard data pointer */
    uint8 cc;                           /* Local character buffer */
    uint8 *dd, *tp;                     /* Pointers to the data areas */

    clipState &= ~CLIP_TRY_GET ;

    /* Check the standard clipboard status */
    if (clipState & CLIP_OWNER)
        return;                         /* Must be already saved */

    /* Get the data from the clipboard */
    OpenClipboard (ttHwnd);
    if ((hmem = GetClipboardData ((ttlogfont.lfCharSet == OEM_CHARSET) ? CF_OEMTEXT : CF_TEXT)) != NULL)
    {
        int len;
        uint8 *tmpbuf;

        bufp = GlobalLock (hmem);       /* Lock global buffer */
        len = strlen (bufp);            /* Get length of text */

        /* Compute the length of the data and construct
         * a stripped down copy of the string excluding the
         * '\r' characters
         */
        if ((tmpbuf = (uint8 *) meMalloc (len+1)) == NULL)
            goto do_unlock;             /* Failed memory allocation */

        tp = tmpbuf;                    /* Start of the temporary buffer */
        dd = bufp;                      /* Start of clipboard data */
        while ((cc = *dd++) !='\0')
        {
            if ((cc == '\r') && (*dd == '\n'))
                len--;
            else
                *tp++ = cc;
        }
        *tp = '\0';

        /* Make sure that it is not the same as the current
         * save buffer head */

        if ((len == 0) ||
            (klhead == NULL) ||
            (klhead->kill == NULL) ||
            (klhead->kill->next != NULL) ||
            (meStrcmp (klhead->kill->data,tmpbuf)))
        {
            /* Always ksave, don't want to glue them together */
            ksave ();
            if ((dd = kaddblock (len+1)) != NULL)
                memcpy (dd, tmpbuf, len+1);
            thisflag = CFKILL ;
        }
        meFree (tmpbuf);                /* Relinquish temp buffer */
do_unlock:
        GlobalUnlock (hmem);            /* Unlock clipboard data */
    }
    CloseClipboard ();
}


void
mkTempCommName(uint8 *filename, uint8 *basename)
{
    HANDLE hdl ;
    uint8 *ss ;
    int ii ;

    mkTempName(filename,basename,NULL) ;
    ss = filename+meStrlen(filename) - 3 ;
    for(ii=0 ; ii<999 ; ii++)
    {
        if(meTestExist(filename))
            break ;
        else if ((hdl = CreateFile(filename,GENERIC_READ,FILE_SHARE_READ,NULL,
                                   OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL)) != INVALID_HANDLE_VALUE)
        {
            CloseHandle(hdl);
            break ;
        }
        sprintf(ss,"%d~",ii) ;
    }
}


#ifdef _IPIPES
#ifdef USE_BEGINTHREAD
void
childActiveThread(void *lpParam)
#else
DWORD WINAPI 
childActiveThread(LPVOID lpParam)
#endif
{
    meIPIPE *ipipe=(meIPIPE *) lpParam ;
    DWORD bytesRead ;
    uint8 buff[4] ;
    
    do {
        /* wait for child process activity */
        if((ReadFile(ipipe->rfd,buff,1,&bytesRead,NULL) != 0) &&
           (bytesRead > 0))
        {
            ipipe->nextChar = buff[0]  ;
            ipipe->flag |= IPIPE_NEXT_CHAR ;
        }
        else
            ipipe->flag |= IPIPE_CHILD_EXIT ;
        
        /* flag the child is active! */
        if(!SetEvent(ipipe->childActive))
            break ;
        
        /* if there was a problem, the pipe is dead - exit */ 
        if(ipipe->flag & IPIPE_CHILD_EXIT)
            break ;
        
        /* wait for the main thread to read all available output and
         * flag for us to start waiting again */
    } while((WaitForSingleObject(ipipe->threadContinue,INFINITE) == WAIT_OBJECT_0) &&
            !(ipipe->flag & IPIPE_CHILD_EXIT)) ;
#ifndef USE_BEGINTHREAD
    return 0 ;
#endif
}

#endif

/*
 * WinLaunchProgram
 * Launches an external program using the DOS shell.
 *
 * Returns TRUE if all went well, FALSE if wait cancelled and FAILED if
 * failed to launch.
 *
 * Cmd is the command string to launch.
 *
 * DOSApp is TRUE if the external program is a DOS program to be run
 * under a DOS shell. If DOSApp is FALSE, the program is launched
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
 * FALSE).
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
WinLaunchProgram (uint8 *cmd, int flags, uint8 *inFile, uint8 *outFile,
#ifdef _IPIPES
                  meIPIPE *ipipe,
#endif
                  int *sysRet)
{
    PROCESS_INFORMATION mePInfo ;
    STARTUPINFO meSuInfo ;
    uint8  cmdLine[MAXBUF+102], *cp ;          /* Buffer for the command line */
    uint8  dummyInFile[FILEBUF] ;              /* Dummy input file */
    uint8  pipeOutFile[FILEBUF] ;              /* Pipe output file */
    int    status ;
#ifdef _WIN32s
    uint8 *endOfComString = NULL;             /* End of the com string */
    static int pipeStderr = 0;                 /* Remember the stderr state */
#else
    HANDLE inHdl, outHdl, dumHdl ;
#endif

#if 1
    static uint8 *compSpecName=NULL ;
    static int compSpecLen ;

    /* Get the comspec */
    if (((flags & LAUNCH_NOCOMSPEC) == 0) && (compSpecName == NULL))
    {
        if (((compSpecName = meGetenv("COMSPEC")) == NULL) ||
            ((compSpecName = meStrdup(compSpecName)) == NULL))
        {
            /* If no COMSPEC setup the default */
            if(platformId != VER_PLATFORM_WIN32_NT)
                compSpecName = "command.com";
            else
                compSpecName = "cmd.exe" ;
        }
        compSpecLen = strlen(compSpecName) ;
    }
#else
    uint8 *compSpecName ;
    if((compSpecName = meGetenv("COMSPEC")) == NULL)
    {
        /* If no COMSPEC setup the default */
        if(platformId == VER_PLATFORM_WIN32_NT)
            compSpecName = "cmd.exe" ;
        else
            compSpecName = "command.com";
    }
#endif

#ifdef _WIN32s
    /* If the pipe to stderr is not defined then get the value. Note that this
     * is a once off get and we only look on the first run */
    if (pipeStderr == 0)
        pipeStderr = ((meGetenv("ME_PIPE_STDERR") != NULL) ? 1 : -1) ;
#endif

    /* set the startup window size */
    memset (&meSuInfo, 0, sizeof (STARTUPINFO));
    meSuInfo.cb = sizeof(STARTUPINFO);

    /* If there is an output file then we need to synchronise */
    if(flags & LAUNCH_SHELL)
    {
        if (flags & LAUNCH_NOCOMSPEC)
            cp = cmd;                   /* No comspec - use the command */
        else
            cp = compSpecName ;         /* Use the comspec */
    }
    else
    {
        /* Create the command line */
        uint8 c1, c2, *dd, *ss, *s2 ;
        ss = cmd ;
        dd = dummyInFile ;
        c1 = *ss++ ;
        if(c1 != '"')
        {
            *dd++ = c1 ;
            c1 = ' ' ;
        }
        while(((c2=*ss)) && (c2 != c1))
        {
            *dd++ = c2 ;
            ss++ ;
        }
        *dd = '\0' ;
        cp = dd = cmdLine ;
        /* If there is no command spec then skip */
        if ((flags & LAUNCH_NOCOMSPEC) == 0)
        {
            strncpy(dd,compSpecName,compSpecLen) ;
            dd += compSpecLen ;
            /* copy in the <shell> /c */
            strncpy(dd," /c ",4) ;
            dd += 4 ;
#ifdef _WIN32s
            endOfComString = dd;            /* End of the COM string */
#endif
        }


        if((((int) dd)-((int)cmdLine)+strlen(cmd)) >= MAXBUF+100)
            return FALSE ;
        if((platformId == VER_PLATFORM_WIN32_NT) &&
           ((flags & LAUNCH_NOCOMSPEC) == 0))
            *dd++ = '"';
        if(c1 == '"')
            *dd++ = '"' ;
        s2 = dummyInFile ;
        if ((flags & LAUNCH_LEAVENAMES) == 0)
        {
            /* Convert directory separators to windows complient characters */
            while((c2=*s2++))
            {
                if(c2 == '/')
                    c2 = '\\' ;
                *dd++ = c2 ;
            }
        }
        strcpy(dd,ss) ;
        if((platformId == VER_PLATFORM_WIN32_NT) &&
           ((flags & LAUNCH_NOCOMSPEC) == 0))
            strcat (dd,"\"");
        if ((flags & LAUNCH_SHOWWINDOW) == 0)
        {
            meSuInfo.wShowWindow = SW_HIDE;
            meSuInfo.dwFlags |= STARTF_USESHOWWINDOW ;
        }

        /* Only a shell needs to be visible, so hide the rest */
        meSuInfo.lpTitle = cmd;
        meSuInfo.hStdInput  = INVALID_HANDLE_VALUE ;
        meSuInfo.hStdOutput = INVALID_HANDLE_VALUE ;
        meSuInfo.hStdError  = INVALID_HANDLE_VALUE ;

/*        fprintf(fp,"Running [%s]\n",cp) ;*/
/*        fflush(fp) ;*/

        /* If its a system call then we don't care about input or output */
        if((flags & LAUNCH_SYSTEM) == 0)
        {
            SECURITY_ATTRIBUTES sbuts ;

            sbuts.nLength = sizeof(SECURITY_ATTRIBUTES) ;
            sbuts.lpSecurityDescriptor = NULL ;
            sbuts.bInheritHandle = TRUE ;

            if(flags & LAUNCH_FILTER)
            {
#ifdef _WIN32s
                strcat (cp, " <");
                strcat (cp, inFile);
                strcat (cp, " >");
                strcat (cp, outFile);
                strcpy (pipeOutFile, outFile);
#else
                HANDLE h ;
                if((meSuInfo.hStdInput=CreateFile(inFile,GENERIC_READ,FILE_SHARE_READ,&sbuts,
                                                  OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL)) == INVALID_HANDLE_VALUE)
                    return FALSE ;
                if((meSuInfo.hStdOutput=CreateFile(outFile,GENERIC_WRITE,FILE_SHARE_READ,&sbuts,
                                                   OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL)) == INVALID_HANDLE_VALUE)
                {
                    CloseHandle(meSuInfo.hStdInput) ;
                    return FALSE ;
                }
                if(CreatePipe(&meSuInfo.hStdError,&h,&sbuts,0) != 0)
                    CloseHandle(h) ;
#endif
            }
#ifdef _IPIPES
            else if(flags & LAUNCH_IPIPE)
            {
                /* Its an IPIPE so create the pipes */
                if(CreatePipe(&meSuInfo.hStdInput,&inHdl,&sbuts,0) == 0)
                    return FALSE ;
                if(CreatePipe(&outHdl,&meSuInfo.hStdOutput,&sbuts,0) == 0)
                {
                    CloseHandle(meSuInfo.hStdInput) ;
                    return FALSE ;
                }
                /* Duplicate stdout => stderr, don't really care if this fails */
                DuplicateHandle (GetCurrentProcess(),meSuInfo.hStdOutput,
                                 GetCurrentProcess(),&meSuInfo.hStdError,0,TRUE,
                                 DUPLICATE_SAME_ACCESS) ;
            }
#endif
            else
            {
                /* if an output file name has been given, use that, else create one */
                if(outFile == NULL)
                {
                     /* Create the output file */
                    mkTempCommName(pipeOutFile,COMMAND_FILE) ;
                    outFile = pipeOutFile ;
                }
#ifdef _WIN32s
                if (pipeStderr > 0)
                    strcat (cp, " >& ");
                else
                    strcat (cp, " > ");
                strcat (cp, outFile);
#else
                meSuInfo.hStdOutput=CreateFile(outFile,GENERIC_WRITE,FILE_SHARE_WRITE,
                                               &sbuts,CREATE_ALWAYS,FILE_ATTRIBUTE_TEMPORARY,NULL) ;
                if(meSuInfo.hStdOutput == INVALID_HANDLE_VALUE)
                    return FALSE ;
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
                mkTempName (dummyInFile, DUMMY_STDIN_FILE,NULL);

                if ((dumHdl = CreateFile(dummyInFile,GENERIC_WRITE,FILE_SHARE_WRITE,NULL,
                                         CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL)) != INVALID_HANDLE_VALUE)
                    CloseHandle (dumHdl);

                /* Re-open the file for reading */
                if ((dumHdl = CreateFile(dummyInFile,GENERIC_READ,FILE_SHARE_READ,NULL,
                                         OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL)) == INVALID_HANDLE_VALUE)
                {
                    DeleteFile(dummyInFile) ;
                    CloseHandle(meSuInfo.hStdOutput) ;
                    return FALSE;
                }
                meSuInfo.hStdInput = dumHdl;
                /* Duplicate stdout => stderr, don't really care if this fails */
                DuplicateHandle (GetCurrentProcess (), meSuInfo.hStdOutput,
                                 GetCurrentProcess (), &meSuInfo.hStdError,0,TRUE,
                                 DUPLICATE_SAME_ACCESS) ;
#endif
            }
#ifndef _WIN32s
            meSuInfo.dwFlags |= STARTF_USESTDHANDLES ;
#endif
        }
    }
#ifdef _WIN32s
    {
        uint8 curDirectory[1024];
        uint8 cmd [1024];
        uint8 batname [1024];
        FILE *fp;

        if (endOfComString == NULL)
            status = SynchSpawn (cmd, /*SW_HIDE*/SW_SHOWNORMAL);
        else
        {
            /* Get the current directory in the 32-bit world */
            _getcwd (curDirectory, sizeof (curDirectory));

            /* Create a BAT file to hold the command */
            mkTempName (batname,NULL, ".bat");

            if ((fp = fopen (batname, "w")) != NULL)
            {
                /* Change drive */
                fprintf (fp, "%c:\n", curDirectory [0]);
                /* Change directory */
                fprintf (fp, "cd %s\n", curDirectory);
                /* Do the command */
                fprintf (fp, "%s\n", endOfComString);
                fclose (fp);

                /* Append the command to run the exec file to the end of the
                 * command string */
                strcpy (endOfComString, batname);

                status = SynchSpawn (cp, /*SW_HIDE*/SW_SHOWNORMAL);
            }
            else
                status = 0;
        }
        /* Correct the status frpom the call */
        if (status != 1)
            status = FALSE;
    }
#else /* ! _WIN32s */
    /* start the process and get a handle on it */
    if(CreateProcess (NULL,
                      cp,
                      NULL,
                      NULL,
                      ((flags & LAUNCH_SHELL) ? FALSE:TRUE),
                      ((flags & LAUNCH_DETACHED) ? DETACHED_PROCESS : CREATE_NEW_CONSOLE),
                      NULL,
                      NULL,
                      &meSuInfo,
                      &mePInfo))
    {
        status = TRUE ;
#if 0
        /* We do not do this here as it causes problems for win95 ipipes on
         * network drives */
        WaitForSingleObject(GetCurrentProcess(), 50);
        WaitForInputIdle(mePInfo.hProcess, 5000);
#endif
        CloseHandle(mePInfo.hThread);
        /* Ipipes need the process handle and we dont wait for it */
        if((flags & LAUNCH_IPIPE) == 0)
        {
            /* For shells we must close the process handle but we dont wait for it */
            if((flags & LAUNCH_SHELL) == 0)
            {
                /* Wait for filter, system and pipe process to end */
                for (;;)
                {
                    DWORD procStatus;

                    procStatus = WaitForSingleObject (mePInfo.hProcess, 200);
                    if (procStatus == WAIT_TIMEOUT)
                    {
                        if (TTahead() && (TTbreakFlag != 0))
                            status = TRUE;
                        else
                            continue;
                    }
                    else if (procStatus == WAIT_FAILED)
                        status = FALSE;
                    else
                        status = TRUE;
                    /* If we're interested in the result, get it */
                    if(sysRet != NULL)
                        GetExitCodeProcess(mePInfo.hProcess,sysRet) ;
                    break;
                }
            }
            /* Close the process */
            CloseHandle (mePInfo.hProcess);
        }
    }
    else
    {
        mlwrite(0,"[Failed to run \"%s\"]",cp) ;
        status = FALSE ;
    }
    /* Close the file handles */
    if(meSuInfo.hStdInput != INVALID_HANDLE_VALUE)
        CloseHandle(meSuInfo.hStdInput);
    if(meSuInfo.hStdOutput != INVALID_HANDLE_VALUE)
        CloseHandle(meSuInfo.hStdOutput);
    if(meSuInfo.hStdError != INVALID_HANDLE_VALUE)
        CloseHandle(meSuInfo.hStdError);
#endif /* WIN32s */

#ifdef _IPIPES
    if(flags & LAUNCH_IPIPE)
    {
        if(status == FALSE)
        {
            CloseHandle(inHdl);
            CloseHandle(outHdl);
        }
        else
        {
            ipipe->pid = 1 ;
            ipipe->rfd = outHdl ;
            ipipe->outWfd = inHdl ;
            ipipe->process = mePInfo.hProcess ;
            ipipe->processId = mePInfo.dwProcessId ;
            
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
            ipipe->childActive = NULL ;
            ipipe->threadContinue = NULL ;
#ifdef USE_BEGINTHREAD
            {
                unsigned long thread ;
                if(((ipipe->childActive=CreateEvent(NULL, TRUE, FALSE, NULL)) != 0) &&
                   ((ipipe->threadContinue=CreateEvent(NULL, FALSE, FALSE, NULL)) != 0) &&
                   ((thread=_beginthread(childActiveThread,0,ipipe)) != -1))
                    ipipe->thread = (HANDLE) thread ;
                else
                    ipipe->thread = NULL ;
            }
#else                
            if(((ipipe->childActive=CreateEvent(NULL, TRUE, FALSE, NULL)) != 0) &&
               ((ipipe->threadContinue=CreateEvent(NULL, FALSE, FALSE, NULL)) != 0))
               ipipe->thread = CreateThread(NULL,0,childActiveThread,ipipe,0,&(ipipe->threadId)) ;
            else
                ipipe->thread = NULL ;
#endif
        }
    }
    else
#endif
        if(flags & LAUNCH_PIPE)
    {
        /* Delete the dummy stdin file if there is one. */
        DeleteFile(dummyInFile) ;
    }
    return status ;
}

/*
 * WinTermCellResize()
 * This forces the screen to row/column size as determined by the current
 * font.
 */

static void
WinTermCellResize (int x, int y)
{
    int hWindow;                        /* Height of Window in pixels */
    int wWindow;                        /* Width of Window in pixels */
    int wBorder;                        /* Width of the boarder */
    int hBorder;                        /* Height of the border */
    RECT wRect;                         /* Window rectangle */
    RECT cRect;                         /* Client rectangle */

    if ((x < 0) && (y < 0))             /* Wasting out time !! */
        return;

    /* If in console mode, do nothing */
#ifdef _WINCON
    if (meSystemCfg & meSYSTEM_CONSOLE)
    {
        /* Always force the original console size upon the user - if ya wanna
         * change the size, use the window mode! */
        if(x <= 0)
            x = eCellMetrics.cellX ;
        if(y <= 0)
            y = eCellMetrics.cellY ;

        /* Must re-allocate the screen buffer if the size changed (can only happen via
         * window properties */
        if ((x != eCellMetrics.cellX) ||
            (y != eCellMetrics.cellY))
        {
            /* Store the size of the screen */
            eCellMetrics.cellX = x ;
            eCellMetrics.cellY = y ;

            /* Re-allocate memory for screen buffer */
            if (ciScreenBuffer != NULL)
                free (ciScreenBuffer) ;
            ciScreenBuffer = (CHAR_INFO *)malloc (sizeof (CHAR_INFO) * x * y);
            memset ((void *)ciScreenBuffer, 0, sizeof (CHAR_INFO) * x * y);
        }
        return ;
    }
#endif

    /* Get the current screen widths */
    GetWindowRect (ttHwnd, &wRect);
    GetClientRect (ttHwnd, &cRect);

    /* Compute the new window widths in terms of pixels */
    /* Resize the x axis if requested.
     * Compute the desired window width, if a resize is requested. */
    if (x > 0)
        wWindow = (x * eCellMetrics.cell.sizeX) + (2 * eCellMetrics.offsetX);
    else
        wWindow = cRect.right - cRect.left;

    /* Add on the boarders by differencing the window and the client area */
    wBorder = (wRect.right - wRect.left) - cRect.right;
    wWindow += wBorder;

    /* Re-size in place if possible */
    if ((wWindow + wBorder + eCellMetrics.cell.sizeX - 1) > eCellMetrics.minMaxInfo.ptMaxSize.x)
    {
        wRect.left = eCellMetrics.minMaxInfo.ptMaxPosition.x;
        wRect.right = wRect.left + eCellMetrics.minMaxInfo.ptMaxSize.x;
    }
    else
    {
        wRect.right = wRect.left + wWindow;
        if (wRect.right > (eCellMetrics.minMaxInfo.ptMaxSize.x +
                           eCellMetrics.minMaxInfo.ptMaxPosition.x))
        {
            wRect.right = (eCellMetrics.minMaxInfo.ptMaxSize.x +
                           eCellMetrics.minMaxInfo.ptMaxPosition.x);
            wRect.left = wRect.right - wWindow;
        }
        else if (wRect.left < eCellMetrics.minMaxInfo.ptMaxPosition.x)
            wRect.left = eCellMetrics.minMaxInfo.ptMaxPosition.x;
    }

    /* Resize the y axis if requested. If an explicit size is requested the
     * assume that size. Otherwise take the client area as is. */
    if (y > 0)
        hWindow = (y * eCellMetrics.cell.sizeY) + (2 * eCellMetrics.offsetY);
    else
        hWindow = cRect.bottom - cRect.top;

    /* Re-align the vertical window - reposition if necessary
     * Add on the boarders by differencing the window and the client area. */
    hBorder =  (wRect.bottom - wRect.top) - cRect.bottom;
    hWindow += hBorder;

    /* Re-size in place if possible */
    if ((hWindow + hBorder + eCellMetrics.cell.sizeY - 1) > eCellMetrics.minMaxInfo.ptMaxSize.y)
    {
        wRect.top = eCellMetrics.minMaxInfo.ptMaxPosition.y;
        wRect.bottom = wRect.top + eCellMetrics.minMaxInfo.ptMaxSize.y;
    }
    else
    {
        wRect.bottom = wRect.top + hWindow;
        if (wRect.bottom > (eCellMetrics.minMaxInfo.ptMaxSize.y +
                            eCellMetrics.minMaxInfo.ptMaxPosition.y))
        {
            wRect.bottom = (eCellMetrics.minMaxInfo.ptMaxSize.y +
                            eCellMetrics.minMaxInfo.ptMaxPosition.y);
            wRect.top = wRect.bottom - hWindow;
        }
        else if (wRect.top < eCellMetrics.minMaxInfo.ptMaxPosition.y)
            wRect.top = eCellMetrics.minMaxInfo.ptMaxPosition.y;
    }

    /* Change the position of the window and get the new client area. */
    SetWindowPos (ttHwnd, NULL, wRect.left, wRect.top,
                  wRect.right - wRect.left,
                  wRect.bottom - wRect.top,
                  SWP_NOZORDER);
    GetClientRect (ttHwnd, &eCellMetrics.canvas); /* Get the new canvas size */
}

/*
 * WinTermResize()
 * Note that this is a bit of a nasty one.
 * Once TTmcol and TTmrow are defined we may not alter their value
 * until EMACS has initialised (that includes calling changeScreenWidth/depth.
 * We rely on the ttinitialised state to ensure that we set these
 * values only once.
 *
 * Once initialised we immediately call WinTermResize() to change the
 * screen size. This resizes the screen if the font has been changed
 * during the initialisation phase. The resize is then coped with
 * properly within EMACS.
 */
void
WinTermResize (void)
{
    int nrow;
    int ncol;
    int ii;

#ifdef _WINCON
    /* If in console mode... */
    if (meSystemCfg & meSYSTEM_CONSOLE)
    {
        /* nothing to do for console version */
        ncol = eCellMetrics.cellX ;
        nrow = eCellMetrics.cellY ;
    }
    else
#endif
    {
        /* Get the new canvas and invalidate the whole screen. */
        GetClientRect (ttHwnd, &eCellMetrics.canvas); /* Get the new canvas size */
        InvalidateRect (ttHwnd, &eCellMetrics.canvas, FALSE);
        eCellMetrics.paintAll = 1 ;

        nrow = (eCellMetrics.canvas.bottom - (2 * eCellMetrics.offsetY)) / eCellMetrics.cell.sizeY;
        if (nrow < 1)
            nrow = 1;

        ncol = (eCellMetrics.canvas.right - (2 * eCellMetrics.offsetX)) / eCellMetrics.cell.sizeX;
        if (ncol < 1)
            ncol = 1;

        /* Force a cell resize to close down the left and right margin spaces,
         * making the window fit the display correctly. */
        WinTermCellResize (ncol, nrow);

        /* Get the new number of rows. */
        nrow = (eCellMetrics.canvas.bottom - (2 * eCellMetrics.offsetY)) / eCellMetrics.cell.sizeY;
        ncol = (eCellMetrics.canvas.right - (2 * eCellMetrics.offsetX)) / eCellMetrics.cell.sizeX;

        /* Store the size of the screen */
        eCellMetrics.cellX = ncol;
        eCellMetrics.cellY = nrow;

        /* Set up the frame store */
        if (ncol > eCellMetrics.screenCellNcol)
        {
            /* Set up the column cell LUT positions. Note allocate a single
             * array and split into two for re-use */
            eCellMetrics.screenCellPosX = realloc (eCellMetrics.screenCellPosX,
                                                   sizeof (int16) * 2 * (ncol + 1));
            eCellMetrics.charCellPosX = &eCellMetrics.screenCellPosX [ncol+1];
            eCellMetrics.screenCellNcol = ncol;
            /* Construct the cell spacing. This is the  width of the characters. */
            eCellMetrics.cellSpacing = (INT *) realloc (eCellMetrics.cellSpacing,
                                                        sizeof (INT) * (ncol + 1));
            /* Construct the temporary rendering buffer */
            eCellMetrics.charCellTmpX = realloc (eCellMetrics.charCellTmpX, ncol+1);
        }

        /* Grow the existing rows */
        if (nrow > eCellMetrics.screenCellNrow)
        {
            /* Set up the row cell LUT positions. Note allocate a single
             * array and split into 4 for re-use. */
            eCellMetrics.screenCellPosY = realloc (eCellMetrics.screenCellPosY, sizeof (int16) * 4 * (nrow + 1));
            eCellMetrics.charCellPosY   = &eCellMetrics.screenCellPosY [nrow+1];
            eCellMetrics.paintStartCol  = &eCellMetrics.charCellPosY [nrow+1];
            eCellMetrics.paintEndCol    = &eCellMetrics.paintStartCol [nrow+1];
            eCellMetrics.screenCellNrow = nrow;
        }

        /* Initialise the row cell LUT tables */
        for (ii = 0; ii <= nrow; ii++)
        {
            eCellMetrics.charCellPosY [ii] =
                      (eCellMetrics.screenCellPosY [ii] = rowToClient (ii));
            eCellMetrics.paintStartCol[ii] = ncol ;
            eCellMetrics.paintEndCol  [ii] = 0 ;
        }
        eCellMetrics.screenCellPosY [0] = (int16) eCellMetrics.canvas.top;
        eCellMetrics.screenCellPosY [nrow] = (int16) eCellMetrics.canvas.bottom;

        /* Initialise the column cell LUT tables. */
        for (ii = 0; ii <= ncol; ii++)
        {
            eCellMetrics.charCellPosX [ii] =
                      (eCellMetrics.screenCellPosX [ii] = colToClient (ii));
            eCellMetrics.cellSpacing [ii] = eCellMetrics.cell.sizeX;
        }
        eCellMetrics.screenCellPosX [0] = (int16) eCellMetrics.canvas.left;
        eCellMetrics.screenCellPosX [ncol] = (int16) eCellMetrics.canvas.right;
    }

    /* If this is the initial startup state then set the TT values */
    if (ttinitialised == 0)
    {
        TTmcol = ncol;
        TTmrow = nrow;
        TTnrow = TTmrow - 1;
        TTncol = TTmcol;
    }
    else
    {
        /* Inform Emacs window S/W that the window has changed size */
        changeScreenWidth (TRUE, ncol);
        changeScreenDepth (TRUE, nrow);
    }
}

/*
 * WinExit
 * User/program has called exit. We use this instead of the converntional
 * exit(2) invocation. This ensures that the process is shut down properly
 * and Windows receives the correct termination messages on the message
 * queue.
 */
void
WinExit (int status)
{
#ifdef _WINCON
    /* If not in console mode... */
    if (!(meSystemCfg & meSYSTEM_CONSOLE))
#endif
        DestroyWindow (ttHwnd);
    exit (status);
}

/* WinShutdown; Shutdown the system. It is important that we relinquish some
 * of the resources that we may have aquired during the session. The most
 * important resource to relinquish is the font resource. */
void
WinShutdown (void)
{
    /* ZZZZ - should this have console support */
#ifdef _CLIENTSERVER
    /* Close & delete the client file */
    TTkillClientServer ();
#endif
#if 0
    /* SWP 16/10/2000 - found that this disables all me32's from accepting drag 'n' drop
     * events, e.g. start an me32 and it drag 'n' drops okay, now start another and exit
     * it. Now try and drag 'n' drop in the first again, it fails! taken out this code */
#ifdef _DRAGNDROP
    /* Disable drag and drop handling */
    if (ttHwnd)
        DragAcceptFiles (ttHwnd, FALSE);
#endif
#endif
    /* Free off the fonts - (not loaded if console version) */
    if(fontAdded && (RemoveFontResource (fontFile) == TRUE))
    {
        /* Tell other windows that we have removed the font
         * resource. Note that the accepted convention here is to
         * use a SendMessage(), however this has the effect of
         * locking up since the Send is synchronous. Instead we
         * use a PostMessage() which sends the message but without
         * blocking. */
        PostMessage (HWND_BROADCAST, WM_FONTCHANGE, 0, 0);
    }

    /* Destroy the palette information */
    if (eCellMetrics.pInfo.hPal != NULL)  /* Release the palette */
    {
        DeleteObject (eCellMetrics.pInfo.hPal);
        eCellMetrics.pInfo.hPal = NULL;
    }
    if (eCellMetrics.pInfo.cPal != NULL)
    {
        meFree (eCellMetrics.pInfo.cPal);
        eCellMetrics.pInfo.cPal = NULL;
    }
    if (eCellMetrics.pInfo.hPalRefCount)
    {
        meFree (eCellMetrics.pInfo.hPalRefCount);
        eCellMetrics.pInfo.hPalRefCount = NULL;
    }
    eCellMetrics.pInfo.hPalSize = 0;
    noColors = 0;

    /* Free off the background brush */
    if (ttBrush != NULL)
    {
        DeleteObject (ttBrush);
        ttBrush = NULL;
    }

    /* Save any buffers as an emergency quit */
#ifdef WE_SHOULD_NOT_NEED_THIS_HERE
    /* Save any buffers as an emergency quit */
    {
        register BUFFER *bp;    /* scanning pointer to buffers */

        bp = bheadp;
        while (bp != NULL)
        {
            if(bufferNeedSaving(bp))
                autowriteout(bp) ;
            bp = bp->b_bufp;            /* on to the next buffer */
        }
        saveHistory(TRUE,0) ;
    }
#endif
}

#if MOUSE
/*
 * TTinitMouse
 * Sort out what to do with the mouse buttons.
 */
void
TTinitMouse(void)
{
    if(meMouseCfg & meMOUSE_ENBLE)
    {
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
        mouseKeys[4] = b3 ;

#ifdef _WINCON
        if (!(meSystemCfg & meSYSTEM_CONSOLE))
#endif
        {
            cc = (uint8) ((meMouseCfg & meMOUSE_ICON) >> 16) ;
            if(cc >= meCURSOR_COUNT)
                cc = 0 ;
            if(cc != meCurCursor)
            {
                if(meCursors[cc] == NULL)
                    meCursors[cc] = LoadCursor (NULL,meCursorName[cc]) ;
                if(meCursors[cc] != NULL)
                {
                    if(mouseState & MOUSE_STATE_VISIBLE)
                        SetCursor(meCursors[cc]) ;
                    meCurCursor = cc ;
                }
            }
        }
    }
}

/*
 * WinMouse
 * Handle mouse events from the message queues.
 * Returning TRUE if the event is handled; otherwise FALSE.
 */
int
WinMouse (UINT message, UINT wParam, LONG lParam)
{
    if(!(meMouseCfg & meMOUSE_ENBLE))
    {
        switch (message)
        {
        case WM_MOUSEMOVE:
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP:
            return TRUE ;
        }
        return FALSE ;
    }
    switch (message)
    {
    case WM_MOUSEMOVE:
        /* If a button was pressed/release outside the window we don't
         * see it!!! The first chance we get to see it is when the user
         * moves the mouse back into the window - so check the mouse
         * button state is the same, if not treat this event as a button
         * event
         */
        if(wParam == mouseButs)
        {
            uint32 arg;           /* Decode key argument */
            uint16 cc ;

            /* Mouse buttons are the same - handle as a move mouse */

            /* Convert the mouse coordinates to cell space. Compute
             * the fractional bits which are 1/128ths */
            mousePosUpdate(lParam) ;

            /* Check for a mouse-move key */
            cc = ME_SPECIAL | ttmodif | (SKEY_mouse_move+mouseKeys[mouseState&MOUSE_STATE_BUTTONS]) ;
            /* Are we after all movements or mouse-move bound ?? */
            if((!TTallKeys && (decode_key(cc,&arg) != -1)) || (TTallKeys & 0x1))
                addKeyToBuffer(cc) ;        /* Add key to keyboard buffer */
            mouseShow() ;
            break ;
        }
        /* Jon 00/02/12: Fault report by "Dave E" that the screen was
         * always selected under Windows '95 when double clicking from an
         * icon. A trace from his machine reveieled that the mouse was
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
        return FALSE;
    
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
        {
            int mouseCode, ii ;

            mouseButs = wParam ;
            mousePosUpdate(lParam) ;
            mouseCode = 0;
            if (wParam & MK_LBUTTON)
                mouseCode |= MOUSE_STATE_LEFT;
            if (wParam & MK_MBUTTON)
                mouseCode |= MOUSE_STATE_MIDDLE;
            if (wParam & MK_RBUTTON)
                mouseCode |= MOUSE_STATE_RIGHT;

            for (ii=1 ; ii < 8; ii <<= 1)
            {
                if(((mouseCode ^ mouseState) & ii) != 0)
                {
                    uint16 cc ;
                    if (mouseCode & ii)             /* Release ?? */
                        cc = SKEY_mouse_pick_1 ;    /* Get mouse pick key binding */
                    else
                        cc = SKEY_mouse_drop_1 ;    /* Get mouse drop key binding */

                    /* Generate the key code and report */
                    cc = (ME_SPECIAL|ttmodif)|(cc+mouseKeys[ii]-1) ;

                    addKeyToBuffer(cc) ;
                }
            }
            mouseState = (mouseState & ~MOUSE_STATE_BUTTONS)|mouseCode ;
            mouseShow() ;
            if (mouseState & MOUSE_STATE_BUTTONS)
            {
                if ((mouseState & MOUSE_STATE_LOCKED) == 0)
                {
                    SetCapture (ttHwnd);    /* Lock the mouse */
                    mouseState |= MOUSE_STATE_LOCKED;
                }
            }
            else if (mouseState & MOUSE_STATE_LOCKED)
            {
                ReleaseCapture ();         /* Relinquish the mouse */
                mouseState &= ~MOUSE_STATE_LOCKED;
            }
            break;
        }
#ifdef WM_MOUSEWHEEL
    case WM_MOUSEWHEEL:
        {
            uint16 cc ;

/*            mlwrite(0,"%x %x %x",(int) message,wParam,(int) lParam) ;*/
            mousePosUpdate(lParam) ;
            if(wParam & 0x80000000)
                cc = ME_SPECIAL|SKEY_mouse_wdown ;
            else
                cc = ME_SPECIAL|SKEY_mouse_wup ;
            cc |= ttmodif ;
            addKeyToBuffer(cc) ;
            break;
        }
#endif
    default:
        return (FALSE);
    }

    return (TRUE);
}
#endif

/*
 * WinKeyboard
 * Handle keyboard message types.
 * Returning TRUE if the event is handled; otherwise FALSE.
 */
int
WinKeyboard (UINT message, UINT wParam, LONG lParam)
{
    uint16 cc;                  /* Local keyboard character */

#ifdef _WIN_KEY_DEBUGGING
    {
        FILE *fp = NULL;
        
        if ((fp = fopen ("c:/me.dump", "a")) != NULL)
        {
            char *name;
            
            switch (message)
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
            default:
                name = "?WM_UNKNOWN?";
                break;
            }
            
            fprintf (fp, "%s::%d(0x%08x). wParam = %d(%04x) lParam = %d(%08x)\n",
                     name, message, message, wParam, wParam, lParam, lParam);
            fclose (fp);
        }
    }
#endif
    
    switch (message)
    {
    case WM_SYSKEYDOWN:
        /* For some unknown reason, bill has decided that an F10 key
         * press will generate a SYSKEYDOWN, cope with this oddity */
        if (wParam != VK_F10)
        {
            if ((lParam & (1<<29)) == 0)    /* Windows got no input focus ? */
            {
                SetFocus (ttHwnd);          /* No - aquire focus */
                return FALSE;
            }

            ttmodif |= ME_ALT;              /* The ALT key is pressed */
            goto done_syskeydown;
        }
    case WM_KEYDOWN:
        /* Get the ALT key status */
        if (lParam & (1<<29))
            ttmodif |= ME_ALT;
        else
            ttmodif &= ~ME_ALT;
done_syskeydown:
        /* Determine the special keyboard character we are handling */
        switch (wParam)
        {
        case VK_SHIFT:
            ttmodif |= ME_SHIFT;
            break;
        case VK_CONTROL:
            ttmodif |= ME_CONTROL;
            break;
        case VK_MENU:
            ttmodif |= ME_ALT;
            break;
        case VK_LEFT:
            /* Distinguish cursor/number-pad keys */
            cc = ((lParam & 0x01000000) ? SKEY_left : SKEY_kp_left) ;
            goto do_keydown ;
        case VK_RIGHT:
            /* Distinguish cursor/number-pad keys */
            cc = ((lParam & 0x01000000) ? SKEY_right : SKEY_kp_right) ;
            goto do_keydown ;
        case VK_UP:
            /* Distinguish cursor/number-pad keys */
            cc = ((lParam & 0x01000000) ? SKEY_up : SKEY_kp_up) ;
            goto do_keydown ;
        case VK_DOWN:
            /* Distinguish cursor/number-pad keys */
            cc = ((lParam & 0x01000000) ? SKEY_down : SKEY_kp_down) ;
            goto do_keydown ;
        case VK_INSERT:
            /* Distinguish control-pad/number-pad keys */
            cc = ((lParam & 0x01000000) ? SKEY_insert : SKEY_kp_insert) ;
            goto do_keydown ;
        case VK_DELETE:
            /* Distinguish control-pad/number-pad keys */
            cc = ((lParam & 0x01000000) ? SKEY_delete : SKEY_kp_delete) ;
            goto do_keydown ;
        case VK_PRIOR:
            /* Distinguish control-pad/number-pad keys */
            cc = ((lParam & 0x01000000) ? SKEY_page_up : SKEY_kp_page_up) ;
            goto do_keydown;
        case VK_NEXT:
            /* Distinguish control-pad/number-pad keys */
            cc = ((lParam & 0x01000000) ? SKEY_page_down : SKEY_kp_page_down) ;
            goto do_keydown;
        case VK_HOME:
            /* Distinguish control-pad/number-pad keys */
            cc = ((lParam & 0x01000000) ? SKEY_home : SKEY_kp_home) ;
            goto do_keydown;
        case VK_END:
            /* Distinguish control-pad/number-pad keys */
            cc = ((lParam & 0x01000000) ? SKEY_end : SKEY_kp_end) ;
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
#if 0
        case VK_LWIN:
        case VK_RWIN:
            /* The left/right window buttons. Note that we handle this
             * specially in that if the key is bound then we process it, if it
             * is not bound then we pretend that we have not processed it,
             * this will allow the default handler to process the key */
            {
                uint32 arg;               /* Decode key argument */

                cc = ME_SPECIAL | ttmodif | ((wParam == VK_LWIN) ? SKEY_start_left : SKEY_start_right);
                if (decode_key(cc,&arg) != -1)  /* Key bound ?? */
                    goto do_addbuf;             /* Yes - allow key to be pr0ocessed */
                return FALSE;                   /* *IMPORTANT* Key is not processed */
            }
            return FALSE;                       /* *IMPORTANT* Key is not processed */
#endif
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
            cc = SKEY_f3 + (wParam - VK_F3);
            goto do_keydown;
        case VK_F10:
        case VK_F11:
        case VK_F12:
            cc = SKEY_f10 + (wParam - VK_F10);
do_keydown:
            cc = (ME_SPECIAL | ttmodif | cc) ;
            /* Add the character to the typeahead buffer.
             * Note that we do not process (lParam & 0xff) which is the
             * auto-repeat count - this always appears to be 1. */
#ifdef _WIN_KEY_DEBUGGING
            {
                FILE *fp = NULL;
                
                if ((fp = fopen ("c:/me.dump", "a")) != NULL)
                {
                    fprintf (fp, "addKeyToBuffer %c - %d(0x%04x)\n",
                             cc & 0xff, cc, cc);
                    fclose (fp);
                }
            }
#endif
            addKeyToBuffer (cc);
#if MOUSE
            mouseHide() ;
#endif
            break;

        default:
#ifndef DISABLE_ALT_C_KEY_DETECTION
            /* !!!!!!!!!!!  DOUBLE KEYS - THIS MAY BE THE PROBLEM AREA !!!!!!!!!!!! */
            /*
             * I am not sure about this case. If this is a KEYDOWN message
             * and bit-29 of "lParam" is non-zero then we are dealing
             * with Alt-C characters. which do not appear in any of
             * the other messages.  Process as normal.
             *
             * The Microsoft documentation (MSVC 2.0) says that this bit is always
             * zero. However it appears that this bit is infact set when the ALT and
             * CTRL keys are pressed together - Wierd but true !!
             *
             * Jon Green - 03/03/97
             *
             *!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

            if (message != WM_KEYDOWN)
                return (FALSE);           /* NOT PROCESSED - return a false state */
            /* found that C-1, C-2 etc come down this route */
            if((ttmodif & ME_CONTROL) && (wParam >= '0') && (wParam <= '9'))
            {
                static uint8 shiftDigits[] = ")!\"#$%^&*(";

                cc  = (ttmodif & (ME_CONTROL|ME_ALT)) | ((ttmodif & ME_SHIFT) ? shiftDigits [wParam - '0'] : wParam) ;
            }
            else if ((lParam & (1<<29)) == 0)  /* Get out quickly - this is fastest check */
            {
                /* Jon 99/01/27: Another hole we are missing <Ctrl-@>,
                 * <Ctrl-'>, etc. and they appear to be coming in here so plug
                 * it up !! I'm not sure if there is any rational to the way
                 * Microsoft have mapped their character set or are we doing
                 * something terribly wrong ??
                 *
                 * Fail on most keys */
                if ((ttmodif & ME_ALT) || ((ttmodif & ME_CONTROL) == 0))
                    return FALSE;

                /* The only keys we want to process here are those that do not
                 * come back to use as WM_CHAR. Fail on all of the others - we
                 * will see them later as a different message type */
                if (wParam & 0x80)
                {
                    wParam &= 0x7f;
                    if ((ttmodif & ME_SHIFT) == 0)
                    {
                        switch (wParam)
                        {
                            /* C-: */
                        case 0x3a : cc = ttmodif | ';'; break;
                            /* C-= */
                        case 0x3b : cc = ttmodif | 0x3d; break;
                            /* C-, */
                        case 0x3c:
                            /* C-= */
                        case 0x3d:
                            /* C-. */
                        case 0x3e:
                            /* C-? */
                        case 0x3f:
                            cc = ttmodif | (wParam & ~0x10);
                            break;
                            /* C-' */
                        case 0x40: cc = ttmodif | 0x27; break;
                            /* C-~ */
                        case 0x5e: cc = ttmodif | 0x23; break;
                            /* C-` */
                        case 0x5f: cc = ttmodif | 0x5d; break;
                            /* Other specials */
                            /* cc = ttmodif | (wParam & 0x7f);*/
                        default:
                            return FALSE;;
                        }
                    }
                    else
                    {
                        /* Process the rest */
                        switch (wParam)
                        {
                            /* C-+ */
                        case 0x3b:
                            cc = (~ME_SHIFT & ttmodif) | 0x2b; break;
                            /* C-{ */
                        case 0x5b:
                            /* C-\ */
                        case 0x5c:
                            /* C-} */
                        case 0x5d:
                            /* C-~ */
                        case 0x5e:
                            cc = (~ME_SHIFT & ttmodif) | wParam | 0x20;
                            break;
                        case 0x40:
                            cc = (~ME_SHIFT & ttmodif) | wParam;
                            break;
                            /* C-: */
                        case 0x3a:
                            /* C-> */
                        case 0x3e:
                            /* C-? */
                        case 0x3f:
                            /* C-< */
                        case 0x3c:
                            cc = (~ME_SHIFT & ttmodif) | wParam; break;
                        default:
                            return FALSE;
                        }
                    }
                    /*                cc = ttmodif | (wParam & 0x7f);*/
                }                    
                else if (wParam == VK_TAB)
                {
                    cc = SKEY_tab;
                    goto return_spec;
                }
                else
                    return FALSE;
            }
            else if ((wParam >= 'A') && (wParam <= 'Z'))
            {
                cc  = ttmodif | toLower(wParam) ;
            }
            else if ((wParam >= VK_NUMPAD0) && (wParam <= VK_NUMPAD9))
            {
                cc  = ttmodif | ME_SPECIAL | /*SKEY_kp_0 */ '0' + (wParam - VK_NUMPAD0);
            }
            else if (wParam == VK_MULTIPLY)
                cc  = ttmodif | '*' /* ME_SPECIAL | SKEY_kp_multiply*/ ;
            else if (wParam == VK_ADD)
                cc  = ttmodif | '+' /* ME_SPECIAL | SKEY_kp_add */;
            else if (wParam == VK_SUBTRACT)
                cc  = ttmodif | '-' /*| ME_SPECIAL | SKEY_kp_subtract*/;
            else if (wParam == VK_DECIMAL)
                cc  = ttmodif | '.' /*| ME_SPECIAL | SKEY_kp_decimal*/;
            else if (wParam == VK_DIVIDE)
                cc  = ttmodif  | '/' /*| ME_SPECIAL | SKEY_kp_divide */;
            else if (wParam ==  VK_DELETE)
                cc  = ttmodif | SKEY_delete | ME_SPECIAL /*| SKEY_kp_delete*/;
            else if (wParam == VK_INSERT)
                cc  = ttmodif | ME_SPECIAL | SKEY_insert /*| SKEY_kp_insert*/;
            else if (wParam == VK_SPACE)
                cc = ttmodif | ' ' ;
            else if (wParam == VK_BACK)
                cc  = ME_SPECIAL | SKEY_backspace | ttmodif ;
            else if (wParam & 0x80)
            {
                wParam &= 0x7f;
                if ((ttmodif & ME_SHIFT) == 0)
                {
                    switch (wParam)
                    {
                        /* A-C-: */
                    case 0x3a: cc = ttmodif | 0x3b; break;
                        /* A-C-= */
                    case 0x3b: cc = ttmodif | 0x3d; break;
                    case 0x3c:     /* A-C-, */
                    case 0x3d:     /* A-C-- */
                    case 0x3e:     /* A-C-. */
                    case 0x3f:     /* A-C-? */
                        cc = ttmodif | (wParam & ~0x10);
                        break;
                        /* A-C-' */
                    case 0x40: cc = ttmodif | 0x27; break;
                        /* A-C-~ */
                    case 0x5e: cc = ttmodif | 0x23; break;
                        /* A-C-` */
                    case 0x5f: cc = ttmodif | 0x5d; break;
                    default: cc = ttmodif | (wParam & 0x7f);
                    }
                }
                else
                {
#ifdef _WIN32s
                    /* Special case for Alt-Tab - this is not for us !! Windows does not
                       intercept this key on win32s. */
                    if ((wParam == '\t') && ((ttmodif & (ME_CONTROL|ME_SHIFT)) == 0))
                        return FALSE;    /* Not processed */
#endif
                    /* Process the rest */
                    switch (wParam)
                    {
                    case 0x3b:
                        cc = (~ME_SHIFT & ttmodif) | 0x2b; break;
                    case 0x3d:
                        cc = (~ME_SHIFT & ttmodif) | 0x5f; break;
                    case 0x5b:             /* A-C-{ */
                    case 0x5c:             /* A-C-\ */
                    case 0x5d:             /* A-C-} */
                    case 0x5e:             /* A-C-~ */
                        cc = (~ME_SHIFT & ttmodif) | wParam | 0x20;
                        break;
                    case 0x40:
                        cc = (~ME_SHIFT & ttmodif) | wParam;
                        break;
                    default:
                        cc = (~ME_SHIFT & ttmodif) | wParam;
                        break;
                    }
                }
/*                cc = ttmodif | (wParam & 0x7f);*/
            }
            else
                return (FALSE);          /* NOT PROCESSED - return a false state */

            /* Add the character to the typeahead buffer.
             * Note that we do no process (lParam & 0xff) which is the
             * auto-repeat count. - this always appears to be 1 */
#ifdef _WIN_KEY_DEBUGGING
            {
                FILE *fp = NULL;
                
                if ((fp = fopen ("c:/me.dump", "a")) != NULL)
                {
                    fprintf (fp, "addKeyToBuffer %c - %d(0x%04x)\n",
                             cc & 0xff, cc, cc);
                    fclose (fp);
                }
            }
#endif
            addKeyToBuffer (cc);
#if MOUSE
            mouseHide() ;
#endif

#else  /* DISABLE_ALT_C_KEY_DETECTION */
            return (TRUE);
#endif /* DISABLE_ALT_C_KEY_DETECTION */
        }
        break;

    case WM_SYSKEYUP:
    case WM_KEYUP:
        switch (wParam)
        {
        case VK_MENU:    ttmodif &= ~ME_ALT;   break;
        case VK_SHIFT:   ttmodif &= ~ME_SHIFT; break;
        case VK_CONTROL: ttmodif &= ~ME_CONTROL;  break;
        default:
            return (FALSE);
        }
        break;

    case WM_SYSCHAR:
        ttmodif |= ME_ALT;              /* MUST be an ALT key input */
        goto done_syschar;

    case WM_CHAR:
        /* Detect the ALT key */
        if (lParam & (1<<29))
            ttmodif |= ME_ALT;
        else
            ttmodif &= ~ME_ALT;
done_syschar:
        /* Handle any special keys here */
        switch (wParam)
        {
        case VK_RETURN:
            /* Distinguish between the Number Pad and standard enter */
#if 0
            cc = ((lParam & 0x01000000) ? SKEY_kp_enter : SKEY_return) ;
            goto return_spec;
#else
            /* Look at the scan code to determine if this is a C-m or a
             * <return>. The scan code for 'M' is 0x32; the scan code for
             * <RETURN> is 0x1c. If this is a C-m then simply drop through,
             * wParam should be 0x1d which is C-m. */
            if ((lParam & 0xff0000) == (0x1c<<16))
            {
                cc = SKEY_return; /* Return */
                goto return_spec;
            }
#endif
        }

        cc = wParam;
        if (cc == 0x20)
        {
            if((ttmodif == ME_ALT) && !(meSystemCfg & meSYSTEM_CTCHASPC))
                return (FALSE);          /* NOT PROCESSED - return a false state */
        }
        else if (cc < 0x20)
        {
            if ((ttmodif & ME_CONTROL) == 0)
            {
                if (cc == 0x09)
                {
                    cc = SKEY_tab;
                    goto return_spec;
                }
#if 0
                /* Jon:991129; Moved to case above. More of the keys 
                 * need to be migrated like this. */
                if (cc == 0x0d)
                {
                    /* Distinguish between the Number Pad and standard enter */
                    cc = ((lParam & 0x01000000) ? SKEY_kp_enter : SKEY_return) ;
                    goto return_spec;
                }
#endif
                if (cc == 0x1b)
                {
                    cc = SKEY_esc;
                    goto return_spec;
                }
                if (cc == 0x08)
                {
                    cc = SKEY_backspace;
                    goto return_spec;
                }
            }
        }
        else
        {
            if (cc == 0x7f)
            {
                /* Special case of the C-backspace key */
                cc = SKEY_backspace ;
                goto return_spec;
            }
        }
        if((ttmodif & ME_ALT) || ((ttmodif & ME_CONTROL) && (cc >= 0x20)))
        {
            /* Must make the letters lower case */
            cc = toLower(cc) ;
            /*            cc |= ((ttmodif & 0x01) << 8) | ((ttmodif & 0x0e) << 7);*/
            cc |= ttmodif & (ME_CONTROL|ME_ALT) ;
        }

        /* Add the character to the typeahead buffer.
         * Note that we do no process (lParam & 0xff) which is the
         * auto-repeat count. - this always appears to be 1 */
#ifdef _WIN_KEY_DEBUGGING
        {
            FILE *fp = NULL;
            
            if ((fp = fopen ("c:/me.dump", "a")) != NULL)
            {
                fprintf (fp, "addKeyToBuffer %c - %d(0x%04x)\n",
                         cc & 0xff, cc, cc);
                fclose (fp);
            }
        }
#endif
        addKeyToBuffer(cc) ;
#if MOUSE
        mouseHide() ;
#endif
        break;
return_spec:
        cc = (ME_SPECIAL | ttmodif | cc) ;
#ifdef _WIN_KEY_DEBUGGING
        {
            FILE *fp = NULL;
            
            if ((fp = fopen ("c:/me.dump", "a")) != NULL)
            {
                fprintf (fp, "addKeyToBuffer %c - %d(0x%04x)\n",
                         cc & 0xff, cc, cc);
                fclose (fp);
            }
        }
#endif
        addKeyToBuffer(cc) ;
#if MOUSE
        mouseHide() ;
#endif
        break;
    default:
        return (FALSE);
    }
    return (TRUE);
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
TTaddColor(meCOLOR index, uint8 r, uint8 g, uint8 b)
{
#ifdef _WINCON
    if (meSystemCfg & meSYSTEM_CONSOLE)
    {
        uint8 *ss ;
        int            ii, jj, idif, jdif ;

        jdif = 256+256+256 ;
        ss = consoleColors ;
        for(ii=0 ; ii<consoleNumColors ; ii++)
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
            colTable = realloc(colTable, (index+1)*sizeof(uint32)) ;
            memset(colTable+noColors,0,(index-noColors+1)*sizeof(uint32)) ;
            noColors = index+1 ;
        }
        colTable[index] = jj ;
    }
    else
#endif
    {
        COLORREF cReq;                      /* Color requested */
        COLORREF cAsg;                      /* Color assigned */
        HDC      hDC;                       /* Device context */

        cReq = RGB (r, g, b);               /* Construct the new colour reference */
        hDC = GetDC(NULL);                  /* Get the device context */

        /* Determine if we are adding to the device context or the palette */
        if (GetDeviceCaps (hDC, RASTERCAPS) & RC_PALETTE)
        {
            int maxPaletteSize;             /* Maximum size of the palette */
            int closeIndex;                 /* A close existing palette index */
            PALETTEENTRY closeEntry;        /* A close palette entry */
            COLORREF closeColor;           /* A close existing palette colour */

            maxPaletteSize = GetDeviceCaps(hDC, SIZEPALETTE);

            /* Delete the old palette entry if it existed previously */
            if ((index < noColors) && (eCellMetrics.pInfo.cPal[index].rgb != 0))
            {
                closeIndex = GetNearestPaletteIndex (eCellMetrics.pInfo.hPal,
                                                     eCellMetrics.pInfo.cPal[index].rgb);

                /* Reference count has reached zero. Remove the palette entry */
                if (--eCellMetrics.pInfo.hPalRefCount [closeIndex] <= 0)
                {
                    if (--eCellMetrics.pInfo.hPalSize == 0)
                    {
                        /* Destruct the palette */
                        meFree (eCellMetrics.pInfo.hPalRefCount);
                        eCellMetrics.pInfo.hPalRefCount = NULL;

                        DeleteObject (eCellMetrics.pInfo.hPal);
                        eCellMetrics.pInfo.hPal = NULL;
                    }
                    else
                    {
                        if (closeIndex <  eCellMetrics.pInfo.hPalSize)
                        {
                            /* Reduce the size of the palette by re-shuffling */
                            GetPaletteEntries (eCellMetrics.pInfo.hPal,
                                               eCellMetrics.pInfo.hPalSize, 1, &closeEntry);
                            SetPaletteEntries (eCellMetrics.pInfo.hPal,
                                               closeIndex, 1, &closeEntry);

                            /* Move the reference count entry - do not bother to resize the
                             * reference count table */
                            eCellMetrics.pInfo.hPalRefCount [closeIndex] =
                                      eCellMetrics.pInfo.hPalRefCount [eCellMetrics.pInfo.hPalSize];
                        }
                        /* Resize the palette */
                        ResizePalette (eCellMetrics.pInfo.hPal, eCellMetrics.pInfo.hPalSize);
                    }
                }
            }

            /* Create a new entry for the colour
             * Check the current palette for the colour. If we have it already we do
             * not need to create a new palette entry. */
            if (eCellMetrics.pInfo.hPalSize > 0)
            {
                closeIndex = GetNearestPaletteIndex (eCellMetrics.pInfo.hPal, cReq);
                if (closeIndex != CLR_INVALID)
                {
                    GetPaletteEntries (eCellMetrics.pInfo.hPal, closeIndex, 1, &closeEntry);
                    closeColor = RGB (closeEntry.peRed, closeEntry.peGreen, closeEntry.peBlue);
                }
                else
                    closeColor = 0xffffffff;

                /* Check for duplicates */
                if (cReq == closeColor)
                {
                    /* Already have it in the palette */
                    eCellMetrics.pInfo.hPalRefCount [closeIndex] += 1;
                }
                else
                {
                    /* Fail if the colour map is full */
                    if (eCellMetrics.pInfo.hPalSize == maxPaletteSize)
                    {
                        /* ERROR - not enough colours */
                        ReleaseDC(NULL, hDC);
                        return FALSE ;
                    }
                    else
                    {
                        /* A  a new entry to the palette */
                        closeIndex = eCellMetrics.pInfo.hPalSize;
                        eCellMetrics.pInfo.hPalSize++;
                        ResizePalette (eCellMetrics.pInfo.hPal, eCellMetrics.pInfo.hPalSize);

                        closeEntry.peRed = r;
                        closeEntry.peGreen = g;
                        closeEntry.peBlue = b;
                        closeEntry.peFlags = 0;
                        SetPaletteEntries (eCellMetrics.pInfo.hPal, closeIndex, 1, &closeEntry);

                        /* Resize the pallete reference table */
                        eCellMetrics.pInfo.hPalRefCount = (int *) realloc (eCellMetrics.pInfo.hPalRefCount,
                                                                           (sizeof (int) * eCellMetrics.pInfo.hPalSize));
                        eCellMetrics.pInfo.hPalRefCount [closeIndex] = 1;
                    }
                }
            }
            else
            {
                LPLOGPALETTE lPal;          /* Logical palette */

                /* No palette is defined. Allocate a new palette */
                lPal = HeapAlloc (GetProcessHeap (), HEAP_ZERO_MEMORY,
                                  sizeof (LOGPALETTE) + sizeof (PALETTEENTRY));
                lPal->palVersion = 0x300;   /* Windows magic number !! */
                lPal->palNumEntries = 1;    /* 2 entries in the palette */

                lPal->palPalEntry [0]. peRed = r;
                lPal->palPalEntry [0]. peGreen = g;
                lPal->palPalEntry [0]. peBlue = b;
                lPal->palPalEntry [0]. peFlags = 0;


                /* Create the palette */
                eCellMetrics.pInfo.hPal = CreatePalette (lPal);
                HeapFree (GetProcessHeap (), 0, lPal);  /* Dispose of the local palette */

                eCellMetrics.pInfo.hPalSize = 1;

                /* Create the reference count */
                eCellMetrics.pInfo.hPalRefCount = (int *) malloc (sizeof (int));
                eCellMetrics.pInfo.hPalRefCount [0] = 1;
            }

            /* Save the assigned colour */
            cAsg = PALETTERGB (r, g, b);
        }
        else
        {
            /* Determine what colour will actually be used on non-colour mapped systems */
            cAsg = GetNearestColor (hDC, PALETTERGB (r, g, b));
        }

        ReleaseDC (NULL, hDC);

        /* Grow the colour table if required */
        if (index >= noColors)
        {
            int ii;

            eCellMetrics.pInfo.cPal = (PaletteItem *) realloc ((void *) eCellMetrics.pInfo.cPal,
                                                               sizeof (PaletteItem) * (index+1));
            /* Set to black */
            for (ii = noColors; ii <= index; ii++)
            {
                eCellMetrics.pInfo.cPal [ii].rgb = 0;
                eCellMetrics.pInfo.cPal [ii].cpixel = PALETTERGB (0, 0, 0);
            }
            noColors = index + 1;
        }

        /* Assign the allocated colour */
        eCellMetrics.pInfo.cPal [index].cpixel = cAsg;
        eCellMetrics.pInfo.cPal [index].rgb = cReq;
    }
    return TRUE ;
}

/*
 * TTchangeFont
 * Change the current font setting. Re-compute the cell metrics for the
 * client area and change to the new font. If the font cannot be found
 * then default to the default OEM font.
 */
static int
TTchangeFont (uint8 *fontName, int fontType, int fontWeight,
              int fontHeight, int fontWidth)
{
    HDC   hDC;                          /* Device context */
    HFONT newFont = NULL;               /* The new font */
    INT   nCharWidth;                   /* The character width */
    RECT  rect;                         /* Area of the client window */
    TEXTMETRIC textmetric;              /* Text metrics */
    LOGFONT logfont;                    /* Logical font */
    int   status = TRUE;                /* Status of the invocation */

    hDC = GetDC(ttHwnd);
    SetMapMode (hDC, MM_TEXT);
    SetMapperFlags (hDC, 1);            /* Allow interpolation */

    /* Reset the default logical font */
    memset (&logfont, 0, sizeof (LOGFONT));
    logfont.lfWeight = FW_DONTCARE;
    logfont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    logfont.lfQuality = DRAFT_QUALITY;
    logfont.lfPitchAndFamily = FIXED_PITCH|FF_DONTCARE;

    if (fontType != -1)
    {
        if (fontType <= -2)
        {
            CHOOSEFONT chooseFont;

            memset (&chooseFont, 0, sizeof (CHOOSEFONT));
            chooseFont.lStructSize = sizeof (CHOOSEFONT);
            chooseFont.hwndOwner = ttHwnd;
            chooseFont.lpLogFont = &logfont;
            chooseFont.Flags = CF_FIXEDPITCHONLY|CF_SCREENFONTS;

            /* This stupid Microsoft system, why is the CHOOSEFONT structure
             * size not sizeof (CHOOSEFONT). Typical Microsoft !! */
            if (ChooseFont (&chooseFont) == 0)
                /* SWP - if the user cancelled just return false leaving the font alone */
                return FALSE ;
            /* Save the values in $result */
            sprintf(resultStr,"%1d%4d%4d%4d%s",(logfont.lfWeight/100),logfont.lfWidth,
                    logfont.lfHeight,logfont.lfCharSet,logfont.lfFaceName) ;
            if (fontType < -2)
                /* if a -ve argument was past to changeFont then don't set the font */
                return TRUE ;
            
            /* SWP - we dont want italic as the main font */
            logfont.lfItalic = 0;
        }
        else if ((fontType >= 0) &&
                 (fontName != NULL) && (fontName[0] != '\0') &&
                 (!((fontHeight == 0) && (fontWidth == 0))))
        {
            /* Determine the weight on the font */
            switch (fontWeight)
            {
            case 0: logfont.lfWeight = FW_DONTCARE;   break;
            case 1: logfont.lfWeight = FW_THIN;       break;
            case 2: logfont.lfWeight = FW_EXTRALIGHT; break;
            case 3: logfont.lfWeight = FW_LIGHT;      break;
            case 4: logfont.lfWeight = FW_NORMAL;     break;
            case 5: logfont.lfWeight = FW_MEDIUM;     break;
            case 6: logfont.lfWeight = FW_SEMIBOLD;   break;
            case 7: logfont.lfWeight = FW_BOLD;       break;
            case 8: logfont.lfWeight = FW_EXTRABOLD;  break;
            case 9: logfont.lfWeight = FW_HEAVY;      break;
            default:logfont.lfWeight = FW_DONTCARE;   break;
            };

            /* Determine the type of the font */
            logfont.lfCharSet = fontType;
            logfont.lfHeight = fontHeight;    /* Height */
            logfont.lfWidth = fontWidth;      /* Width */
            strncpy (logfont.lfFaceName, fontName,  sizeof (logfont.lfFaceName));
        }
        else
        {
            /* The font has some horrible attributes. Use a default font */
            goto defaultFont;           /* Do not create a new font */
        }

        /* Create the new font */
        if ((newFont = CreateFontIndirect (&logfont)) == NULL)
            status = FALSE;
    }
    else
        fontType = ttlogfont.lfCharSet ;

    /* Get the default font */
defaultFont:
    if (newFont == NULL)
    {
        newFont = GetStockObject ((fontType == OEM_CHARSET) ? OEM_FIXED_FONT : ANSI_FIXED_FONT);
        logfont.lfCharSet = fontType ;
    }
    
    /* Delete the exisiting font */
    if (eCellMetrics.fontdef[0] != NULL)
    {
        int ii ;
        /* Iterate over the font face table and locate duplicated fonts */
        for (ii = 1; ii < meFONT_MAX; ii++)
        {
            if ((eCellMetrics.fontdef[ii] != NULL) && 
                (eCellMetrics.fontdef[ii] != eCellMetrics.fontdef[0]))
            {
                /* Delete the font container */
                DeleteObject (eCellMetrics.fontdef[ii]);
            }
            eCellMetrics.fontdef [ii] = NULL;
        }
        DeleteObject (eCellMetrics.fontdef[0]);
    }

    eCellMetrics.fontdef[0] = newFont;
    SelectObject (hDC, eCellMetrics.fontdef[0]);
    GetTextMetrics(hDC, &textmetric);

    /* Build up the cell metrics */
    nCharWidth  = textmetric.tmAveCharWidth /*+ textmetric.tmInternalLeading*/;

    /* Obtain the resolution of the screen */
    rect.left   = GetDeviceCaps(hDC, LOGPIXELSX);   /* 1/4 inch */
    rect.right  = GetDeviceCaps(hDC, HORZRES);
    rect.top    = GetDeviceCaps(hDC, LOGPIXELSY);   /* 1/4 inch */

    /* Get the text metrics for the current font */
#if 1
    /* SWP bodge */
    eCellMetrics.cell.sizeX = nCharWidth ;
#else
    eCellMetrics.cell.sizeX = ((textmetric.tmMaxCharWidth <= 0) ? 1
                               : textmetric.tmMaxCharWidth /*+ textmetric.tmExternalLeading*/);
#endif
    eCellMetrics.cell.sizeY = (textmetric.tmHeight <= 0) ? 1 : textmetric.tmHeight;
    eCellMetrics.cell.midX = eCellMetrics.cell.sizeX / 2;
    eCellMetrics.cell.midY = eCellMetrics.cell.sizeY / 2;

    eCellMetrics.leadingY = textmetric.tmExternalLeading;

    /* Offsets from the edge of the window 1/4 of width and 1/8 of height is nice */
    eCellMetrics.offsetX = eCellMetrics.cell.sizeX / 4;
    if ((eCellMetrics.offsetY = (eCellMetrics.cell.sizeY / 8) - (eCellMetrics.leadingY/2)) < 0)
        eCellMetrics.offsetY = 0;

    /* Store logfont into the ttlogfont for font style and language char set changes  */
    memcpy(&ttlogfont, &logfont, sizeof (LOGFONT));
    GetTextFace (hDC, sizeof (ttlogfont.lfFaceName), ttlogfont.lfFaceName);
    ttlogfont.lfHeight = eCellMetrics.cell.sizeY;
    ttlogfont.lfWidth = eCellMetrics.cell.sizeX;
    
    /* Release the window */
    ReleaseDC(ttHwnd, hDC);
    WinTermResize ();
    return (status);
}

/* changeFont
 * Change the size of the font.
 *
 * change-font <name> <charSet> <weight> <width> <height>
 *
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
 * <charSet> - The type of character set required.
 *
 *             0 = OEM (or bitmapped)
 *             1 = ANSI (True Type etc).
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
    uint8 fontName[FONTBUFSIZ] ;        /* Input font name buffer */
    uint8 buff[FONTBUFSIZ] ;            /* Input buffer */
    int  fontType;                      /* Type of font 0=ANSI,255=OEM etc */
    int  fontWeight;                    /* Weight of font (0-9) */
    int  fontHeight;                    /* Height of font */
    int  fontWidth;                     /* Width of font */
    int  status;                        /* Status of invocation */

#ifdef _WINCON
    /* Ignore this function for console mode */
    if (meSystemCfg & meSYSTEM_CONSOLE)
        return notAvailable(f,n) ;
#endif

    /* Call up the dialog if no argument is supplied */
    if (!f || (n < 0))
        status = TTchangeFont (NULL, n-3, 0, 0, 0);
    else
    {
        /* Get the name of the font. If it is specified as default then
         * do not collect the remaining arguments */
        if (meGetString ("Font Name ['' for default]", 0, 0, fontName, FONTBUFSIZ) == ABORT)
            return (FALSE);
        if (fontName[0] == '\0')
            status = TTchangeFont (NULL, -1, 0, 0, 0);
        else if ((meGetString ("Font Type [ANSI=0]", 0, 0, buff, FONTBUFSIZ) == TRUE) &&
                 ((fontType = meAtoi(buff)),
                  (meGetString ("Font Weight [1-9; 0=don't care]", 0, 0, buff, FONTBUFSIZ) == TRUE)) &&
                 ((fontWeight = meAtoi(buff)),
                  (meGetString ("Font Width", 0, 0, buff, FONTBUFSIZ) == TRUE)) &&
                 ((fontWidth = meAtoi(buff)),
                  (meGetString ("Font Height", 0, 0, buff, FONTBUFSIZ) == TRUE)))
        {
            fontHeight = meAtoi (buff);
            status = TTchangeFont (fontName, fontType,
                                   fontWeight, fontHeight, fontWidth);
        }
        else
            status = FALSE;
    }
    return status;

}
#undef FONTBUFSIZ

/*
 * TThideCur - hide the cursor
 */
void
TThideCur (void)
{
    if((TTcurr <= TTnrow) && (TTcurc < TTncol) && !eCellMetrics.paintAll)
    {
#ifdef _WINCON
        if (meSystemCfg & meSYSTEM_CONSOLE)
        {
            FRAMELINE *flp;                     /* Frame store line pointer */
            meSCHEME schm;                      /* Current colour */
            uint8 cc ;                          /* Current cchar  */
            WORD dcol;

            flp  = frameStore + TTcurr ;
            cc   = flp->text[TTcurc] ;          /* Get char under cursor */
            schm = flp->scheme[TTcurc] ;        /* Get colour under cursor */

            dcol = TTschemeSet(schm) ;
            ConsoleDrawString (&cc, dcol, TTcurc, TTcurr, 1);
        }
        else
#endif
        {
            RECT rect;                  /* Area of screen to update */
            int startCol = TTcurc;
#if meFONT_MAX
            /* If the cursor preceeeds an italic character then we need to fix
             * the previous character. This is done by ME drawing the previous
             * char but not invalidating the area so that only the overhang into
             * the next char is drawn.
             */
            FRAMELINE *flp;             /* Frame store line pointer */
            
            flp = frameStore + TTcurr;
            if(startCol > 0)
            {
                if ((meSchemeGetStyle(flp->scheme[startCol-1]) & meSTYLE_ITALIC) &&
                    (flp->text[startCol-1] != ' '))
                {
                    /* Back up, the previous character will be corrupted. */
                    startCol-- ;
                }
            }
#endif
            if(eCellMetrics.paintStartCol[TTcurr] > startCol)
                eCellMetrics.paintStartCol[TTcurr] = startCol ;
            if(eCellMetrics.paintEndCol[TTcurr] <= TTcurc)
                eCellMetrics.paintEndCol[TTcurr] = TTcurc+1 ;
            /* Set up the area on the client window to be modified
               and signal that the client is about to be updated */
            rect.left   = eCellMetrics.screenCellPosX[TTcurc];
            rect.right  = eCellMetrics.screenCellPosX[TTcurc+1];
            rect.top    = eCellMetrics.screenCellPosY[TTcurr];
            rect.bottom = eCellMetrics.screenCellPosY[TTcurr+1];
            InvalidateRect (ttHwnd, &rect, FALSE);
        }
    }
}

/*
 * TTshowCur - show the cursor
 */
void
TTshowCur (void)
{
    if((TTcurr <= TTnrow) && (TTcurc < TTncol) && !eCellMetrics.paintAll)
    {
#ifdef _WINCON
        if (meSystemCfg & meSYSTEM_CONSOLE)
        {
            FRAMELINE *flp;                     /* Frame store line pointer */
            meSCHEME schm;                      /* Current colour */
            uint8 cc ;                          /* Current cchar  */
            WORD dcol;

            flp  = frameStore + TTcurr ;
            cc   = flp->text[TTcurc] ;          /* Get char under cursor */
            schm = flp->scheme[TTcurc] ;        /* Get colour under cursor */

            dcol = TTcolorSet(colTable[meStyleGetBColor(meSchemeGetStyle(schm))],
                              colTable[cursorColor]) ;

            ConsoleDrawString (&cc, dcol, TTcurc, TTcurr, 1);
        }
        else
#endif
        {
            RECT rect;                  /* Area of screen to update */
            int startCol = TTcurc;
#if meFONT_MAX
            /* If the cursor preceeeds an italic character then we need to fix
             * the previous character. This is done by ME drawing the previous
             * char but not invalidating the area so that only the overhang into
             * the next char is drawn.
             */
            FRAMELINE *flp;             /* Frame store line pointer */
            
            flp = frameStore + TTcurr;
            if(startCol > 0)
            {
                if ((meSchemeGetStyle(flp->scheme[startCol-1]) & meSTYLE_ITALIC) &&
                    (flp->text[startCol-1] != ' '))
                {
                    /* Back up, the previous character will be corrupted. */
                    startCol-- ;
                }
            }
#endif
            if(eCellMetrics.paintStartCol[TTcurr] > startCol)
                eCellMetrics.paintStartCol[TTcurr] = startCol ;
            if(eCellMetrics.paintEndCol[TTcurr] <= TTcurc)
                eCellMetrics.paintEndCol[TTcurr] = TTcurc+1 ;
            /* Set up the area on the client window to be modified
               and signal that the client is about to be updated */
            rect.left   = eCellMetrics.screenCellPosX[TTcurc];
            rect.right  = eCellMetrics.screenCellPosX[TTcurc+1];
            rect.top    = eCellMetrics.screenCellPosY[TTcurr];
            rect.bottom = eCellMetrics.screenCellPosY[TTcurr+1];
            InvalidateRect (ttHwnd, &rect, FALSE);
        }
    }
}

/*
 * TTcolour
 * Change the foreground and background colours.
 */
void
TTcolour (int fcol, int bcol)
{
    if (ttfcol != fcol)
        ttfcol = meFColorCheck (fcol);
    if (ttbcol != bcol)
        ttbcol = meBColorCheck (bcol);
}

/*
 * meGetMessage
 * Get a new message from the windows message queue. Handle
 * any streams immediatly.
 */
void
meGetMessage (MSG *msg)
{
#if (defined _WINCON) || (defined _IPIPES)
    if (
#ifdef _WINCON
        (meSystemCfg & meSYSTEM_CONSOLE)
#ifdef _IPIPES
        ||
#endif
#endif
#ifdef _IPIPES
        noIpipes
#endif
        )
    {
        static HANDLE *hTable ;
        static int hTableSize=0 ;
        int ii, jj ;
#ifdef _IPIPES
        meIPIPE *ipipe, *pp ;
        ii = noIpipes ;
#else
        ii = 0 ;
#endif
#ifdef _WINCON
        if(meSystemCfg & meSYSTEM_CONSOLE)
            ii++ ;
#endif
        if(ii > hTableSize)
        {
            hTableSize = ii+1 ;
            meNullFree(hTable) ;
            if((hTable = meMalloc(hTableSize*sizeof(HANDLE))) == NULL)
            {
                /* if this fails we really are in big trouble, time to get out! */
                meDie() ;
            }
        }
        for(;;)
        {
#ifdef _IPIPES
            /* Loop round first time to read anything available and close
             * those processes that have finished
             */
            ipipe = ipipes ;
            while(ipipe != NULL)
            {
                DWORD doRead ;
                pp = ipipe->next ;
#ifdef _CLIENTSERVER
                if(ipipe->pid == 0)
                {
                    if(TTcheckClientServer())
                        ipipeRead(ipipe) ;
                }
                else
#endif
                {
                    if(ipipe->pid > 0)
                    {
                        if(ipipe->thread != NULL)
                        {
                            if((doRead = (WaitForSingleObject(ipipe->childActive,0) == WAIT_OBJECT_0)))
                                ResetEvent(ipipe->childActive) ;
                        }
                        else if(PeekNamedPipe(ipipe->rfd, (LPVOID) NULL, (DWORD) 0,
                                              (LPDWORD) NULL, &doRead, (LPDWORD) NULL) == FALSE)
                        {
                            /* If peek failed, wipe our hands of it. Close the process */
                            CloseHandle(ipipe->process);
                            ipipe->pid = -4 ;
                        }
                    }
                    if((ipipe->pid < 0) || doRead)
                        ipipeRead(ipipe) ;
                    else if((platformId != VER_PLATFORM_WIN32_NT) &&
                            /* ipipe->bp->b_nwnd &&*/
                            (!GetExitCodeProcess(ipipe->process,&doRead) || (doRead != STILL_ACTIVE)))
                    {
                        /* Win95 fails to spot the exit state some times, this fixes it */
                        CloseHandle(ipipe->process);
                        ipipe->pid = -4 ;
                        pp = ipipe ;
                    }
                }
                ipipe = pp ;
            }
#endif
            /* Now simply loop through creating a wait object list of all remaining
             * processes & console.
             */
            ii = 0 ;
#ifdef _WINCON
            if(meSystemCfg & meSYSTEM_CONSOLE)
                hTable[ii++] = hInput ;
#endif
#ifdef _IPIPES
            ipipe = ipipes ;
            while(ipipe != NULL)
            {
#ifdef _CLIENTSERVER
                if(ipipe->pid > 0)
#endif
                {
                    if(ipipe->thread != NULL)
                        hTable[ii++] = ipipe->childActive ;
                    else
                        hTable[ii++] = ipipe->rfd ;
                }
                ipipe = ipipe->next ;
            }
#endif
            /* Wait for either user or process activity */
            jj = MsgWaitForMultipleObjects(ii,hTable,FALSE,INFINITE,QS_ALLINPUT) - WAIT_OBJECT_0 ;
#ifdef _WINCON
            if(meSystemCfg & meSYSTEM_CONSOLE)
            {
                if(((jj <= 0) || (jj >= ii)) && meGetConsoleMessage(msg))
                   return ;
            }
            else
#endif
            if((jj < 0) || (jj >= ii))
                /* User activity, go get it! */
                break ;
        }
    }
#endif /* _IPIPES & _WINCON */
    /* Note: console versions cannot get here */
    if(GetMessage(msg,              /* address of structure with message */
                  ttHwndNull,           /* handle of window */
                  0,                /* first message */
                  0) <= 0)          /* last message */
        meDie() ;
}

/*
 * TTgetc
 * Wait for a character. If a character does not arrive then
 * suspend awaiting a character from the keyboard.
 */
void
TTwaitForChar(void)
{
#if MOUSE
    uint16 mc ;
    uint32 arg ;
    /* If no keys left then if theres currently no mouse timer and
     * theres a button press (No mouse-time key) then check for a
     * time-mouse-? key, if found add a timer start a mouse checking
     */
    if(!isTimerSet(MOUSE_TIMER_ID) &&
       ((mc=(mouseState & MOUSE_STATE_BUTTONS)) != 0))
    {
        mc = ME_SPECIAL | ttmodif | (SKEY_mouse_time + mouseKeys[mc]) ;
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
        doIdlePickEvent ();         /* Check the idle event */

    /* Pend for messages */
    for (;;)
    {
        MSG msg;                            /* Message buffer */

        if(isTimerExpired(AUTOS_TIMER_ID))  /* Alarm expired ?? */
            autoSaveHandler();              /* Initiate an auto save */
        if(isTimerExpired(CALLB_TIMER_ID))  /* Alarm expired ?? */
            callBackHandler();              /* Initiate any callbacks */
        if(isTimerExpired(CURSOR_TIMER_ID)) /* Alarm expired ?? */
            TThandleBlink(0);               /* Initiate a cursor blink */
#ifdef _URLSUPP
        if(isTimerExpired(SOCKET_TIMER_ID)) /* socket connection time-out */
            ffFileOp(NULL,NULL,meRWFLAG_FTPCLOSE|meRWFLAG_SILENT) ;
#endif
        if(TTahead())
            break ;

        if (sgarbf == TRUE)
        {
            update (FALSE);
            mlerase (MWCLEXEC);
        }

        /* Suspend until there is another message to process. */
        TTflush () ;                /* Make sure the screen is up-to-date */
#ifdef _FULLDEBUG
        /* Do heap walk in idle time */
        _CrtCheckMemory();
#endif
        meGetMessage(&msg);         /* Suspend for a message */

        /* Closing down the system */
        if (msg.message == WM_CLOSE)
            WinExit (0);
        /* Timer - handle short and sweet here */
        else if (msg.message == WM_TIMER)
        {
#ifdef _WINCON
            if (meSystemCfg & meSYSTEM_CONSOLE)
                timerAlarm(msg.wParam) ;
            else
#endif
                if(msg.wParam < NUM_TIMERS)
            {
                meTimerState[msg.wParam] = (meTimerState[msg.wParam] & ~TIMER_SET) | TIMER_EXPIRED;
                continue;
            }
        }
#if MOUSE
        /* Mouse movement or button press */
        else if ((msg.message >= WM_MOUSEFIRST) &&
                 (msg.message <= WM_MOUSELAST))
        {
            if (WinMouse (msg.message, msg.wParam, msg.lParam))
                continue;
        }
#endif
        /* Keyboard message */
        else if ((msg.message >= WM_KEYFIRST) &&
                 (msg.message <= WM_KEYLAST))
        {
            if (!(meSystemCfg & meSYSTEM_CONSOLE))
                TranslateMessage (&msg);    /* Translate keyboard characters */
            if (WinKeyboard (msg.message, msg.wParam, msg.lParam))
                continue;
        }

        /* Only get here if we have not handled the message
         * post to the dispatcher if not a console. */
#ifdef _WINCON
        if (!(meSystemCfg & meSYSTEM_CONSOLE))
#endif
            meMessageHandler(&msg) ;
    }
}

/*
 * TTputs
 * Put a string to the screen simply by invalidating the region. The
 * display functions should have updated the frameStore which is what
 * is used for rendering so it is only necessary to invalidate the
 * screen region to invoke a paint operation.
 */
void
TTputs (int row, int col, int len)
{
    if(!eCellMetrics.paintAll)
    {
        RECT rect;                          /* Area of screen to update */

        if(eCellMetrics.paintStartCol[row] > col)
            eCellMetrics.paintStartCol[row] = col ;
        if(eCellMetrics.paintEndCol[row] < (col+len))
            eCellMetrics.paintEndCol[row] = col+len ;

        /* Set up the area on the client window to be modified
           and signal that the client is about to be updated */
        rect.left   = eCellMetrics.screenCellPosX [col];
        rect.right  = eCellMetrics.screenCellPosX [col+len];
        rect.top    = eCellMetrics.screenCellPosY [row];
        rect.bottom = eCellMetrics.screenCellPosY [row+1];
        InvalidateRect (ttHwnd, &rect, FALSE);
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
    if((ttRect.right >= 0) && !eCellMetrics.paintAll)
    {
        int row ;
        for(row=ttRect.top; row < ttRect.bottom ; row++)
        {
            if(eCellMetrics.paintStartCol[row] > (int16) ttRect.left)
                eCellMetrics.paintStartCol[row] = (int16) ttRect.left ;
            if(eCellMetrics.paintEndCol[row] < (int16) ttRect.right)
                eCellMetrics.paintEndCol[row] = (int16) ttRect.right ;
        }
        ttRect.left   = eCellMetrics.screenCellPosX[ttRect.left] ;
        ttRect.right  = eCellMetrics.screenCellPosX[ttRect.right] ;
        ttRect.top    = eCellMetrics.screenCellPosY[ttRect.top] ;
        ttRect.bottom = eCellMetrics.screenCellPosY[ttRect.bottom] ;
        InvalidateRect (ttHwnd, &ttRect, FALSE) ;
    }
}

/*
 * TTstart
 * Start the EMACS window environment. This is called once at the start
 * when it creates the Window and determines the initial text metrics.
 */
TTstart (void)
{
#ifdef _WINCON
    if (meSystemCfg & meSYSTEM_CONSOLE)
    {
        CONSOLE_SCREEN_BUFFER_INFO Console;
        CONSOLE_CURSOR_INFO CursorInfo;
        COORD dwCursorPosition;
        
        /* console can't support fonts and only has XANSI */
        meSYSTEM_MASK &= ~meSYSTEM_FONTS ;
        meSystemCfg = (meSystemCfg & ~(meSYSTEM_FONTS|meSYSTEM_RGBCOLOR)) | (meSYSTEM_ANSICOLOR|meSYSTEM_XANSICOLOR) ;

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
        GetConsoleScreenBufferInfo(hOutput, &Console);

        OldConsoleSize.X = Console.dwSize.X ;
        OldConsoleSize.Y = Console.dwSize.Y ;

        /* let MicroEMACS know our starting screen size */
        /* this should be the window size, not the buffer size
         * as this needs the scroll-bar to use */
        ConsoleSize.X = Console.srWindow.Right-Console.srWindow.Left+1;
        ConsoleSize.Y = Console.srWindow.Bottom-Console.srWindow.Top+1;
        /* now fix the window buffer size to this window size to
         * get rid of the horrid scroll bars! */
        SetConsoleScreenBufferSize(hOutput,ConsoleSize);
        WinTermCellResize (ConsoleSize.X,ConsoleSize.Y) ;
        WinTermResize () ;

        consolePaintArea.Right = consolePaintArea.Bottom = 0 ;
        consolePaintArea.Left = consolePaintArea.Top = (SHORT) 0x7fff ;

        /* we always have a mouse under NT */
        /*        mexist = GetNumberOfConsoleMouseButtons(&nbuttons);*/

        /* save the original console mode to restore on exit */
        GetConsoleMode(hInput, &OldConsoleMode);

        /* and reset this to what MicroEMACS needs */
        ConsoleMode = ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT;
        SetConsoleMode(hInput, ConsoleMode);

        /* Set emergency quit handler routine */
        SetConsoleCtrlHandler (ConsoleHandlerRoutine, TRUE);

        /* Hide the cursor - this does not seem to work on win98!! */
        GetConsoleCursorInfo (hOutput, &CursorInfo);
        CursorInfo.bVisible = FALSE;
        SetConsoleCursorInfo (hOutput, &CursorInfo);
        /* so move the cursor to a fixed less annoying position */
        dwCursorPosition.X = ConsoleSize.X-1;
        dwCursorPosition.Y = 0;
        SetConsoleCursorPosition(hOutput, dwCursorPosition);
    }
    else
#endif
    {
#ifdef _WINCON
        /* If in window mode, then bin the console we were given */
        FreeConsole ();
#endif
        ttHwnd = CreateWindow ("MicroEmacsClass",
                               "MicroEmacs " meVERSION,
                               WS_OVERLAPPEDWINDOW,  /* No scroll bars */
                               meInitGeom[2],
                               meInitGeom[3],
                               meInitGeom[0],
                               meInitGeom[1],
                               NULL,
                               NULL,
                               ttInstance,
                               NULL);

        if (!ttHwnd)
            return (FALSE);
        TTchangeFont (NULL, -1, 0, 0, 0);
    }

    /* Create the default colours */
    /* Construct the palette. Add two colours; black and white. */
    TTaddColor (meCOLOR_FDEFAULT,255, 255, 255);  /* White */
    TTaddColor (meCOLOR_BDEFAULT,  0,   0,   0);  /* Black */
    TTcolour (meCOLOR_FDEFAULT,meCOLOR_BDEFAULT);  /* Default colours - none created yet */

    /* To be continued in TTstartStage2() after the display memory
     * has been initialised */
    return (TRUE);
}

/*
 * TTstartStage2
 * Display memory sub-system is initialised. Start up the window
 */
int
TTstartStage2 (void)
{
    /* Create the cursor */
    TTshowCur ();                       /* Show the cursor */

#ifdef _WINCON
    if (!(meSystemCfg & meSYSTEM_CONSOLE))
#endif
    {
        ShowWindow(ttHwnd, ttshowState);    /* Create the window */
        UpdateWindow(ttHwnd);               /* Show it off - ready for errors */
    }
    ttinitialised = 1;                  /* TT is now initialised */
    return (TRUE);
}

/*
 * TTahead()
 * Typeahead. Search for any keyboard or mouse messages on the input
 * queue. Process them and return the type-ahead buffer length.
 */

int
TTahead (void)
{
    MSG msg;                            /* Local message buffer */

    /* Peek all Keyboard and Mouse messages until there are no another
       message to process. */
#ifdef _WINCON
    if (meSystemCfg & meSYSTEM_CONSOLE)
    {
        DWORD dwCount;
        INPUT_RECORD ir;
        for (;;)
        {
            if ((PeekConsoleInput(hInput, &ir, 1, &dwCount) == TRUE) && (dwCount > 0))
            {
                if(meGetConsoleMessage(&msg))
                {
                    /* Keyboard message */
                    if ((msg.message >= WM_KEYFIRST) && (msg.message <= WM_KEYLAST))
                        WinKeyboard (msg.message, msg.wParam, msg.lParam) ;
                    else if ((msg.message >= WM_MOUSEFIRST) && (msg.message <= WM_MOUSELAST))
                        WinMouse (msg.message, msg.wParam, msg.lParam) ;
                }
            }
            else if (PeekMessage (&msg, ttHwndNull, WM_TIMER, WM_TIMER, PM_REMOVE) != FALSE)
                timerAlarm(msg.wParam) ;
            else
                break;
        }
    }
    else
#endif
    {
        for (;;)
        {
            if (PeekMessage (&msg, ttHwndNull, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE) != FALSE)
            {
                TranslateMessage (&msg);    /* Translate keyboard characters */
                if (!WinKeyboard (msg.message, msg.wParam, msg.lParam))
                    meMessageHandler(&msg) ;
            }
#if MOUSE
            /* Check out the mouse. */
            else if (PeekMessage (&msg, ttHwndNull, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE) != FALSE)
            {
                if (WinMouse (msg.message, msg.wParam, msg.lParam) == FALSE)
                    meMessageHandler(&msg) ;
            }
#endif
            else if (PeekMessage (&msg, ttHwndNull, WM_TIMER, WM_TIMER, PM_REMOVE) != FALSE)
            {
                /* Check out the timers */
                if(msg.wParam < NUM_TIMERS)
                    meTimerState[msg.wParam] = (meTimerState[msg.wParam] & ~TIMER_SET) | TIMER_EXPIRED;
                else
                    meMessageHandler(&msg) ;
            }
            else
                break;
        }
    }
    /* don't process the timers if we have a key waiting!
     * This is because the timers can generate a lot of timer
     * keys, filling up the input buffer - these are not wanted.
     * By not processing, we leave them there until we need them.
     */
    if(TTnoKeys)
        return TTnoKeys ;
#if MOUSE
    /* If the mouse alarm has returned Check the mouse */
    if (isTimerExpired(MOUSE_TIMER_ID))
    {
        uint16 mc;
        uint32 arg;

        timerClearExpired (MOUSE_TIMER_ID);
        mc = ME_SPECIAL | ttmodif | (SKEY_mouse_time + mouseKeys[mouseState&MOUSE_STATE_BUTTONS]);
        /* mouse-move bound ?? */
        if((!TTallKeys && (decode_key(mc,&arg) != -1)) || (TTallKeys & 0x2))
        {
            /* Timer expired and still bound. Report the key. */
            addKeyToBuffer(mc) ;
            /* Set the new timer and state */
            /* Start a new timer to clock in at 'repeat' intervals */
            /* printf("Setting mouse timer for repeat %d\n",repeattime) ;*/
            timerSet(MOUSE_TIMER_ID,-1,repeattime);
        }
    }
#endif

    /* If the idle timer has gone then deal with it */
    if (isTimerExpired(IDLE_TIMER_ID))
    {
        register int index;
        uint32 arg;           /* Decode key argument */

        if((index=decode_key(ME_SPECIAL|SKEY_idle_time,&arg)) != -1)
        {
            /* Execute the idle-time key */
            execFuncHidden(ME_SPECIAL|SKEY_idle_time,index,arg) ;

            /* Now set the timer for the next */
            timerSet(IDLE_TIMER_ID,-1,idletime) ;
        }
        else if(decode_key(ME_SPECIAL|SKEY_idle_drop,&arg) != -1)
            meTimerState[IDLE_TIMER_ID] = IDLE_STATE_DROP ;
        else
            meTimerState[IDLE_TIMER_ID] = 0 ;
    }
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
TTaheadFlush (void)
{
    MSG msg;                            /* Local message buffer */

#ifdef _WINCON
    if (meSystemCfg & meSYSTEM_CONSOLE)
    {
        DWORD dwCount;
        INPUT_RECORD ir;
        for (;;)
        {
            if ((PeekConsoleInput(hInput, &ir, 1, &dwCount) == TRUE) && (dwCount > 0))
            {
                if(meGetConsoleMessage(&msg))
                {
                    /* Keyboard message */
                    if ((msg.message >= WM_KEYFIRST) && (msg.message <= WM_KEYLAST))
                        WinKeyboard (msg.message, msg.wParam, msg.lParam) ;
                    else if ((msg.message >= WM_MOUSEFIRST) && (msg.message <= WM_MOUSELAST))
                        WinMouse (msg.message, msg.wParam, msg.lParam) ;
                }
            }
            else if (PeekMessage (&msg, ttHwndNull, 0, 0, PM_REMOVE) != FALSE)
            {
                if (msg.message == WM_TIMER)
                    /* Timer has expired */
                    timerAlarm(msg.wParam) ;
            }
            else
                break;
        }
    }
    else
#endif
    {
        /* Peek all Keyboard and Mouse messages until there are no another
           message to process. */
        for (;;)
        {
            if (PeekMessage (&msg, ttHwndNull, 0, 0, PM_REMOVE) != FALSE)
            {
                /* Check out the keyboard. */
                if ((msg.message >=  WM_KEYFIRST) && (msg.message <= WM_KEYLAST))
                {
                    TranslateMessage (&msg);    /* Translate keyboard characters */
                    if (!WinKeyboard (msg.message, msg.wParam, msg.lParam))
                        meMessageHandler(&msg) ;
                }
#if MOUSE
                /* Check out the mouse. */
                else if ((msg.message >= WM_MOUSEFIRST) && (msg.message <= WM_MOUSELAST))
                {
                    if (WinMouse (msg.message, msg.wParam, msg.lParam) == FALSE)
                        meMessageHandler(&msg) ;
                }
#endif
                else if (msg.message == WM_TIMER)
                {
                    /* Check out the timers */
                    if(msg.wParam < NUM_TIMERS)
                        meTimerState[msg.wParam] = (meTimerState[msg.wParam] & ~TIMER_SET) | TIMER_EXPIRED;
                    else
                        meMessageHandler(&msg) ;
                }
                else
                    meMessageHandler(&msg) ;
            }
            else
                break;
        }
    }
#if MOUSE
    /* If the mouse alarm has returned Check the mouse */
    if (isTimerExpired(MOUSE_TIMER_ID))
    {
        uint16 mc;
        uint32 arg;

        timerClearExpired (MOUSE_TIMER_ID);
        mc = ME_SPECIAL | ttmodif | (SKEY_mouse_time + mouseKeys[mouseState&MOUSE_STATE_BUTTONS]);
        /* mouse-move bound ?? */
        if((!TTallKeys && (decode_key(mc,&arg) != -1)) || (TTallKeys & 0x2))
        {
            /* Timer expired and still bound. Report the key. */
            addKeyToBuffer(mc) ;
            /* Set the new timer and state */
            /* Start a new timer to clock in at 'repeat' intervals */
            /* printf("Setting mouse timer for repeat %d\n",repeattime) ;*/
            timerSet(MOUSE_TIMER_ID,-1,repeattime);
        }
    }
#endif

    /* If the idle timer has gone then deal with it */
    if (isTimerExpired(IDLE_TIMER_ID))
    {
        register int index;
        uint32 arg;           /* Decode key argument */

        if((index=decode_key(ME_SPECIAL|SKEY_idle_time,&arg)) != -1)
        {
            /* Execute the idle-time key */
            execFuncHidden(ME_SPECIAL|SKEY_idle_time,index,arg) ;

            /* Now set the timer for the next */
            timerSet(IDLE_TIMER_ID,-1,idletime) ;
        }
        else if(decode_key(ME_SPECIAL|SKEY_idle_drop,&arg) != -1)
            meTimerState[IDLE_TIMER_ID] = IDLE_STATE_DROP ;
        else
            meTimerState[IDLE_TIMER_ID] = 0 ;
    }
    return TTnoKeys;
}

/*
 * TTsleep()
 * Sleep for the specified time period. Only sleep while there
 * is no type-ahead data.
 */
void
TTsleep (int msec, int intable)
{
    if (intable && ((kbdmode == PLAY) || (clexec == TRUE)))
        return;

    /* Do not actually need the absolute time as this will
     * remain the next alarm. Ensure that the value is sane */
    if (msec < 10)
        msec = 10;
    timerSet (SLEEP_TIMER_ID,-1,msec);

    do
    {
        MSG msg;                            /* Message buffer */

        if(isTimerExpired(AUTOS_TIMER_ID))  /* Alarm expired ?? */
            autoSaveHandler();              /* Initiate an auto save */
        if(isTimerExpired(CALLB_TIMER_ID))  /* Alarm expired ?? */
            callBackHandler();              /* Initiate any callbacks */
        if(isTimerExpired(CURSOR_TIMER_ID)) /* Alarm expired ?? */
            TThandleBlink(0);               /* Initiate a cursor blink */
#ifdef _URLSUPP
        if(isTimerExpired(SOCKET_TIMER_ID)) /* socket connection time-out */
            ffFileOp(NULL,NULL,meRWFLAG_FTPCLOSE|meRWFLAG_SILENT) ;
#endif

        /* Call TTahead first to get the input */
        if (intable && TTahead())
            break;

        /* Suspend until there is another message to process. */
        meGetMessage (&msg);
#ifdef _WINCON
        if (meSystemCfg & meSYSTEM_CONSOLE)
        {
            /* Keyboard message */
            if ((msg.message >= WM_KEYFIRST) && (msg.message <= WM_KEYLAST))
                WinKeyboard (msg.message, msg.wParam, msg.lParam) ;
            else if ((msg.message >= WM_MOUSEFIRST) && (msg.message <= WM_MOUSELAST))
                WinMouse (msg.message, msg.wParam, msg.lParam) ;
            else if (msg.message == WM_TIMER)
                /* Timer has expired */
                timerAlarm(msg.wParam) ;
        }
        else
#endif
        {
            TranslateMessage (&msg);    /* Translate keyboard characters */
            meMessageHandler(&msg) ;
        }
    } while (!isTimerExpired(SLEEP_TIMER_ID));

    timerKill(SLEEP_TIMER_ID);              /* Kill off the timer */
}

/*
 * TTdepth
 * Change the size of the screen height.
 *
 * This function is initiated from window.c in changeScreenDepth ()
 * [Emacs command change-screen-depth]. This presents a problem in that
 * WinTermResize() also calls changeScreenDepth (). This causes a recursive loop !!
 * So to make sure that we do not get stuck WinTermResize() ONLY invokes
 * changeScreenDepth () when the current TTnrow is not equal to it's new depth.
 *
 * As it turns out this recursive loop is useful. The user might request a
 * screen depth of 1000 columns changeScreenDepth() will allocate these and call
 * TTdepth() when it has finished. TTdepth() invokes WinTermCellResize()
 * which will try to establish the new width which will fail and truncate to
 * the largest width it can allocate. On invocation of WinTermResize() a
 * new number of rows is computed from the window size (now the truncated)
 * and changeScreenDepth() is invoked. changeScreenDepth() will fix it's rows to match the
 * screen size and invokes TTdepth() again. TTdepth() drops out immediatly
 * since the CellMetrics should now match the screen depth, hence we recurse
 * out and everybody is happy !!.
 */
void
TTdepth (int y)
{
    if (y != eCellMetrics.cellY)
    {
        eCellMetrics.maximized = 0;         /* Knock off the maximise flag */
        WinTermCellResize (-1, y);          /* Kick off cellular resize */
        WinTermResize ();                   /* Resize the terminal */
    }
}

/*
 * TTwidth
 * Changes the width of the screen.
 *
 * See TTdepth notes, apply similarly to the width.
 */
void
TTwidth (int x)
{
    if (x != eCellMetrics.cellX)
    {
        eCellMetrics.maximized = 0;         /* Knock off the maximise flag */
        WinTermCellResize (x, -1);          /* Kill off cellular resize */
        WinTermResize ();                   /* Resize the terminal */
    }
}

/****************************************************************************
 *
 * Read me32.ini
 *
 ****************************************************************************/
void
readIniFile (void)
{
    DWORD status;
    char  buf1 [MAXBUF];
    char  buf2 [MAXBUF];
    char  buf3 [MAXBUF];
    char  logname [MAXBUF];
    int   len;
    char  *p;
    LPTSTR lpSectionNames;
    LPTSTR lpTemp;
    int ii;

    status = MAXBUF;                    /* Size of the retrieve buffer */
    if (((p = meGetenv ("MENAME")) != NULL) && (*p != '\0'))
        strcpy (logname, p);
    else if ((GetUserName (logname, &status) == TRUE) && (logname[0] != '\0'))
        ;
    else if (((p = meGetenv ("LOGNAME")) != NULL) && (*p != '\0'))
        strcpy (logname, p);
    else
        strcpy (logname, meUserName);

    /* Get the user defaults and push them into the environment. */
    /* copy logname to a different buffer so we can change logname if required */
    strcpy (buf2, logname);
    lpSectionNames = HeapAlloc (GetProcessHeap(), HEAP_ZERO_MEMORY, 0x7fff);
    GetPrivateProfileString (buf2, NULL, "", lpSectionNames,
                             0x7fff, ME_INI_FILE);
    for (lpTemp = lpSectionNames; *lpTemp; lpTemp += lstrlen(lpTemp) + 1)
    {
        GetPrivateProfileString (buf2,lpTemp,"",buf1,MAXBUF,ME_INI_FILE);
        /* Get the environment name and make upper case. */
        strcpy (buf3, lpTemp);
        for (p = buf3; *p != '\0'; p++)
            *p = toUpper (*p);
        /* If the user has chaged their MENAME then remember the new name. */
        if ((strcmp(buf3, "MENAME") == 0) && (buf1[0] != '\0'))
            strcpy (logname, buf1);
        else
        {
            /* Add a '=' and concat the data */
            strcat (buf3, "=");             /* Add '=' assignment */
            strcat (buf3, buf1);            /* Add the data */
            mePutenv (meStrdup (buf3));     /* Duplicate for putenv */
        }
    }
    HeapFree (GetProcessHeap(), 0L, lpSectionNames);

    /* set the $MENAME env to logname */
    strcpy (buf1,"MENAME=");
    strcpy (buf1+7,logname);
    if((p = meStrdup(buf1)) != NULL)
    {
        mePutenv (p) ;
        meUserName = p+7 ;
    }

    /*
     * [Defaults] mepath=pathname
     *
     * Get MicroEmacs path next. We look in "MeYYMMDD" for the profile string,
     * or the default directory */
    buf1[0] = buf2[0] = '\0';

    /* Search for the default search path */
    for (ii = 0; iniSections [ii] != NULL; ii++)
    {
        GetPrivateProfileString (iniSections [ii],"mepath","",buf1,MAXBUF,ME_INI_FILE);
        if (buf1[0] != '\0')
            break;
    }

    /* Search for the default user path */
    for (ii = 0; iniSections [ii] != NULL; ii++)
    {
        GetPrivateProfileString (iniSections [ii],"userpath","",buf2,MAXBUF,ME_INI_FILE);
        if (buf2[0] != '\0')
            break;
    }

    /* If the user path cannot be found then copy the default search path */
    if ((buf1[0] != '\0') && (buf2[0] == '\0'))
        strcpy (buf2, buf1);

    /* Construct the userpath from the logname */
    if (buf2[0] != '\0')
    {
        char cc;

        /* Concatinate the mename */
        len = strlen(buf2);
        if (((cc = buf2[len-1]) != '\\') && (cc != '/'))
            strcat (buf2, "/");
        strcat (buf2, meUserName);
    }

    /* Concatinate the search and user paths. */
    if ((buf1[0] != '\0') || (buf2[0] != '\0'))
    {
        strcpy (buf3, "MEPATH=");
        if (buf2[0] != '\0')
        {
            strcat (buf3, buf2);
            strcat (buf3, ";");
        }
        if (buf1[0] != '\0')
        {
            strcat (buf3, buf1);
            strcat (buf3, ";");
        }

        /* Push into the environment. Note that we strdup the name since it
         * MUST exist for the duration of the program life.
         * Convert all '\\' to '/' */
        p = buf3;
#if DIR_CHAR == '/'
        while((p=strchr(p,'\\')) != NULL)    /* got a '\\', -> '/' */
            *p++ = DIR_CHAR ;
#else
        while((p=strchr(p,'/')) != NULL)     /* got a '/', -> '\\' */
            *p++ = DIR_CHAR ;
#endif
        p = meStrdup (buf3);
        mePutenv (p);
    }

#ifdef _WINCON
    if(!(meSystemCfg & meSYSTEM_CONSOLE))
#endif
    {
        /*
         * [Defaults]
         * geometry=wxh+x+y
         *
         * Get the initial window size and position */
        buf1[0] = '\0';
        for (ii = 0; iniSections [ii] != NULL; ii++)
        {
            GetPrivateProfileString (iniSections [ii],"geometry","",buf1,MAXBUF,ME_INI_FILE);
            if (buf1[0] != '\0')
                break;
        }
        if (buf1[0] != '\0')
            sscanf(buf1,"%dx%d+%d+%d",meInitGeom,meInitGeom+1,meInitGeom+2,meInitGeom+3) ;

        /*
         * [Defaults]
         * fontfile=app850.fon
         *
         * Get the font file resources into memory */
        buf1[0] = '\0';
        GetPrivateProfileString (iniSections [0],fontId,"",buf1,MAXBUF,ME_INI_FILE);
        if (buf1[0] == '\0')
            GetPrivateProfileString (iniSections [1],fontId,"",buf1,MAXBUF,ME_INI_FILE);
        if (((len = strlen (buf1)) > 0) && (strcmp (buf1, "none") != 0))
            fontFile = meStrdup (buf1);
    }
}

LRESULT CALLBACK
WinQuitExit (HWND hwndDlg,     /* window handle of dialog box     */
             UINT message,     /* type of message                 */
             WPARAM wParam,    /* message-specific information    */
             LPARAM lParam)    /* message-specific information    */
{
    switch (message)
    {
    case WM_INITDIALOG:  /* message: initialize dialog box  */
        return TRUE;

    case WM_COMMAND:     /* message: received a command */
        /* User pressed "Cancel" button--stop print job. */
        if ((LOWORD (wParam)) == IDOK)
            EndDialog (hwndDlg, TRUE);
        else if ((LOWORD (wParam)) == IDCANCEL)
            EndDialog (hwndDlg, FALSE);
        else
            return FALSE;
        return TRUE;
    default:
        return FALSE;     /* didn't process a message   */

    }
}

#ifdef _WINCON
/****************************************************************************

    FUNCTION: main (int, char *[])

    PURPOSE: calls initialization function, processes message loop

****************************************************************************/

int
main (int argc, char *argv[])
{
    HMODULE hInstance, hPrevInstance;
    int nCmdShow = 1;
    int ii;

    hInstance = GetModuleHandle (NULL);
    hPrevInstance = NULL;	/* Can we do something more safe here? */

#else
/****************************************************************************

    FUNCTION: WinMain(HANDLE, HANDLE, LPSTR, int)

    PURPOSE: calls initialization function, processes message loop

****************************************************************************/

int APIENTRY
WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    char **argv;                        /* Argument string table */
    int  argc;                          /* Argument count */
#endif

#ifdef _FULLDEBUG
#if _FULLDEBUG==1
    /* Enable heap checking on each allocate and free */
    _CrtSetDbgFlag (_CRTDBG_ALLOC_MEM_DF);
#endif
#endif

/*    if(logfp == NULL)*/
/*        logfp = fopen("log","w+") ;*/

    /* Initialise the window data and register window class */
    if (!hPrevInstance)
    {
        WNDCLASS  wc;

        wc.style = 0;
        wc.lpfnWndProc = (WNDPROC) MainWndProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = hInstance;
        wc.hIcon = LoadIcon (hInstance, MAKEINTRESOURCE(IDI_ICON1));
        wc.hCursor = NULL;
        wc.hbrBackground = GetStockObject(BLACK_BRUSH);
        wc.lpszMenuName =  "InputMenu";
        wc.lpszClassName = "MicroEmacsClass";

        if (!RegisterClass (&wc))
            return (FALSE);

        ttInstance =  hInstance;
        ttshowState = nCmdShow;
    }
    {
        OSVERSIONINFO os;

        os.dwOSVersionInfoSize = sizeof(os);
        GetVersionEx(&os);
        platformId = os.dwPlatformId;
        if(platformId != VER_PLATFORM_WIN32s)
        {
#ifdef _IPIPES
            meSYSTEM_MASK |= meSYSTEM_DOSFNAMES|meSYSTEM_IPIPES ;
            meSystemCfg |= meSYSTEM_IPIPES ;
#else
            meSYSTEM_MASK |= meSYSTEM_DOSFNAMES;
#endif
        }
    }
    readIniFile();
    ttThreadId = GetCurrentThreadId ();

#ifdef _WINCON
    if(!(meSystemCfg & meSYSTEM_CONSOLE))
#endif
    {
        /* Load the font resources into the system */
        if ((fontFile != NULL) && (AddFontResource(fontFile)))
        {
            fontAdded = 1 ;
            /* Broadcast a message to inform other windows that some new resources
             * have been loaded. Note that the accepted convention here is to use
             * a SendMessage(), however this has the effect of locking up since
             * the Send is synchronous. Instead we use a PostMessage() which sends
             * the message but without blocking. */
            PostMessage (HWND_BROADCAST, WM_FONTCHANGE, 0, 0);
        }
    }

#ifndef _WINCON
    /* Transpose the argument string into a standard argc, argv line since
     * Windows abandons all standard 'C' practices and does it's own thing
     * (bunch of M***** F*****'s !!). */
    argv = (char **) meMalloc (sizeof (char *) * 2);
    argv [1] = NULL;
    argc = 1;

    /* Get the process name out. This is horrible, the command Line name
     * is returned as UNI-Code characters. In addition we have to cope
     * for spaces in the name, so we check for the ".exe" extension */
    {
        LPTSTR pp;                      /* Command line string pointer */
        /* Find the executable name */
        if ((pp = GetCommandLine ()) == NULL)
        {
            argv[0] = "me";             /* Use a sensible default */
        }
        else
        {
            char *qq, cc, ec ;
            char nambuf [MAXBUF];

            /* ingore the first quote, if 2 quotes then the progname was in
             * quotes so don't stop at a space */
            if(*pp == '"')
            {
                ec = '"' ;
                pp++ ;
            }
            else
                ec = ' ' ;
            qq = nambuf ;
            while(((cc=*pp++) != '\0') && (cc != ec))
            {
                if(cc == '\\')
                    cc = '/' ;
                *qq++ = cc ;
            }
            /* Copy the command name */
            *qq = '\0' ;
            argv[0] = meStrdup (nambuf);
        }
    }

    /* Get the rest of the parameters from the passed in command line */
    if ((lpCmdLine != NULL) && (*lpCmdLine != '\0'))
    {
        char *lpbuf;                    /* Newly allocated line buffer */
        char cc;                        /* Local character buffer */
        char endc;                      /* Termination character for option. */

        lpbuf = meStrdup (lpCmdLine);
        while (*lpbuf != '\0')
        {
            /* Construct larger argv container */
            argv = realloc (argv, (sizeof (char *) * (argc+2)));
            argv [argc+1] = NULL;

            /* Determine the end of option. This may be a quoted string
             * option or unquoted string option. */
            if ((endc=*lpbuf) != '"')
            {
                /* if the next argument does not start with a '-' (an Com-line option)
                 * and the rest of the line makes up the name of an existing file then
                 * add the rest of the line as the last arg and quit. Why? Windows
                 * stupidity of course! With Explorer extension associativity a user
                 * should specify to open .foo files with [me32 "%1"] so the file name
                 * is correctly quoted. But windows being windows allows [me32] so when
                 * the user double clicks on c:\program files\fred.foo the command line
                 * is [me32 c:\program files\fred.foo] - nuff said. */
                if((endc != '-') && !meTestRead(lpbuf))
                {
                    argv [argc++] = lpbuf;
                    break ;
                }
                endc = ' ';
            }
            else
                lpbuf++;

            /* Scan to the end of the string */
            argv [argc++] = lpbuf;
            while ((cc = *lpbuf) != '\0')
            {
                if (cc == endc)
                {
                    *lpbuf++ = '\0';
                    break;
                }
                else
                    lpbuf++;
            }

            /* Remove any white space */
            while ((cc = *lpbuf) != '\0')
            {
                if (cc == ' ')
                    lpbuf++;
                else
                    break;
            }
        }
    }
#endif

    /* Call EMACS with the command line that we have just constructed.
     * Note that we cannot delete the string that we have allocated since
     * EMACS may retain parts of the argument list. */
    mesetup (argc, argv);

    /* Just incase the window has been resized during start up go and check */
    WinTermResize ();                   /* Resize the screen */

    /* Sint in a continual loop and process messages. */
    while (1)
    {
        doOneKey() ;
        if(TTbreakFlag)
        {
            TTinflush() ;
            if((selhilight.flags & (SELHIL_ACTIVE|SELHIL_KEEP)) == SELHIL_ACTIVE)
                selhilight.flags &= ~SELHIL_ACTIVE ;
            TTbreakFlag = 0 ;
        }
#ifdef _DRAGNDROP
        /* if drag and drop is enabled then process the drag and drop
         * files now. Note we make the invocation here as we
         * know we have returned to a base state; any macro's would have
         * been aborted. However it is better that macro debugging
         * is disabled so explicitly disable it. */
        if (dadHead != NULL)
        {
            struct s_DragAndDrop *dadp; /* Drag and drop pointer */

            /* Disable the cursor to allow the mouse position to be
             * artificially moved */
            macbug = 0;                 /* Force macro debugging off */
#if MOUSE
            mouseHide();                /* Hide the cursor */
#endif
            /* Iterate down the list and get the files. */
            while ((dadp = dadHead) != NULL)
            {
                /* Re-position the mouse */
                mouse_X = clientToCol (dadp->mousePos.x);
                mouse_Y = clientToRow (dadp->mousePos.y);
                mouse_dX = ((dadp->mousePos.x - colToClient(mouse_X)) << 8) / eCellMetrics.cell.sizeX;
                mouse_dY = ((dadp->mousePos.y - rowToClient(mouse_Y)) << 8) / eCellMetrics.cell.sizeY;
                if (mouse_X > TTncol)
                    mouse_X = TTncol;
                if (mouse_Y > TTnrow)
                    mouse_Y = TTnrow;

                /* Find the window with the mouse */
                setCursorToMouse (FALSE, 0);

                /* Find the file into buffer */
                findSwapFileList (dadp->fname,BFND_CREAT|BFND_MKNAM,0);

                /* Destruct the list */
                dadHead = dadp->next;
                meFree (dadp);
            }
            /* Display a message indicating last trasaction */
            mlwrite (0, "Drag and Drop transaction completed");
        }
#endif
    }
    return (0);
}

/****************************************************************************

    FUNCTION: MainWndProc(HWND, unsigned, WORD, LONG)

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

LONG APIENTRY
MainWndProc (HWND hWnd, UINT message, UINT wParam, LONG lParam)
{
    meAssert ((ttHwnd == 0) || (hWnd == ttHwnd));

    switch (message)
    {
    case WM_SETFOCUS:
        {
            BYTE keyBuf [256];          /* Keyboard buffer */

            /* Record the fact we have focus */
            TTfocus = 1;

            /* Mark the screen as invalid */
            InvalidateRect (hWnd, NULL, FALSE);
            eCellMetrics.paintAll = 1 ;

            /* Kick of the blinker - as default value for cursorBlink
             * is 0 this will not happen until after the window is created */
            if((cursorState >= 0) && cursorBlink)
                TThandleBlink(2) ;

            /* We have been swapped out. Therefore we potentially do not
             * own the clipboard contents*/
            clipState &= ~CLIP_OWNER ;

            /* Get the state of the keyboard keys into sync */
            GetKeyboardState (keyBuf);

            ttmodif = 0;
            if (keyBuf [VK_SHIFT] & 0x80)
                ttmodif |= ME_SHIFT;
            if (keyBuf [VK_MENU] & 0x80)
                ttmodif |= ME_ALT;
            if (keyBuf [VK_CONTROL] & 0x80)
                ttmodif |= ME_CONTROL;
#if MOUSE
            /* Make sure the cursor is ok */
            SetCursor (meCursors[meCurCursor]);
            mouseState |= MOUSE_STATE_VISIBLE ;
#endif
        }
        break;

    case WM_KILLFOCUS:
        TTfocus = 0;
        if(cursorState >= 0)
        {
            /* because the cursor is a part of the solid cursor we must
             * remove the old one first and then redraw
             */
            if(blinkState)
                TThideCur() ;
            blinkState = 1 ;
            TTshowCur() ;
        }
#if MOUSE
        /* Relinquish control of the mouse */
        if (mouseState & MOUSE_STATE_LOCKED)
        {
            ReleaseCapture ();          /* Relinquish the mouse */
            mouseState &= ~MOUSE_STATE_LOCKED;
        }
#endif
        break;

#if MOUSE
    case WM_SETCURSOR:
        /* If we have not got the focus or this is not the main window cursor
         * then we dont handle it, use the default Proc handle */
        {
            static LONG lstLParam=-1 ;

            if((lParam & 0xffff) != 0x01)
            {
                lstLParam = -1 ;
                goto unhandled_message ;
            }
            if(lParam != lstLParam)
            {
                lstLParam = lParam ;
                SetCursor(meCursors[meCurCursor]) ;
                mouseState |= MOUSE_STATE_VISIBLE ;
            }
        }
        break ;
#endif
        
    case WM_QUERYNEWPALETTE:
        /* About to get focus, realise our palette */
        if (eCellMetrics.pInfo.hPal != NULL)
        {
            HDC hDC = GetDC (hWnd);

            SelectPalette (hDC, eCellMetrics.pInfo.hPal, FALSE);
            if (RealizePalette (hDC))
            {
                InvalidateRect (hWnd, NULL, FALSE);
                eCellMetrics.paintAll = 1 ;
            }
            ReleaseDC (hWnd, hDC);
            return TRUE;
        }
        break;

    case WM_PALETTECHANGED:
        /* Another application has changed the palette */
        if (((HWND) wParam != hWnd) && (eCellMetrics.pInfo.hPal != NULL))
        {
            HDC hDC = GetDC (hWnd);

            SelectPalette (hDC, eCellMetrics.pInfo.hPal, TRUE);
            if (RealizePalette (hDC) != GDI_ERROR)
                UpdateColors (hDC);
            ReleaseDC (hWnd, hDC);
        }
        break;

    case WM_CREATE:
#if MOUSE
        /* Set the default mouse state. Get the number of buttons. Note
         * under windows we do not need to worry about a left/right swap
         * since that is performed beneath us. */
        meMouseCfg |= GetSystemMetrics(SM_CMOUSEBUTTONS) & meMOUSE_NOBUTTONS ;
        TTinitMouse() ;
        meCursors[meCURSOR_DEFAULT] = meCursors[meCURSOR_ARROW] = LoadCursor (NULL,IDC_ARROW) ;
        mouseShow() ;
#endif
#ifdef _DRAGNDROP
        /* Enable drag and drop handling */
        DragAcceptFiles (hWnd, TRUE);
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
            POINT pt;                   /* Position in the window */
            WORD fcount;                /* Count of the number of files */
            uint8 dfname [MAXBUF];       /* Dropped filename */

            /* Get the position of the mouse */
            DragQueryPoint ((HANDLE)(wParam), &pt);

            /* Get the files from the drop */
            fcount = DragQueryFile ((HANDLE)(wParam), 0xffffffff, "", 0);
            if (fcount > 0)
            {
                WORD ii;

                for (ii = 0; ii < fcount; ii++)
                {
                    int len;
                    struct s_DragAndDrop *dadp;

                    if ((len = DragQueryFile ((HANDLE)(wParam), ii,
                                              dfname, MAXBUF)) <= 0)
                        continue;

                    /* Get the drag and drop buffer and add to the list */
                    dadp = (struct s_DragAndDrop *) meMalloc (sizeof (struct s_DragAndDrop) + len);
                    strcpy (dadp->fname, dfname);
                    dadp->mousePos = pt;
                    dadp->next = dadHead;
                    dadHead = dadp;
                }
            }
            DragFinish ((HANDLE)(wParam));

            /* Flush the input queue, send an abort to kill any command
             * off. The drag and drop list is processed once we return to
             * a base state. */
            if (dadHead != NULL)
                addKeyToBuffer(breakc);  /* Break character (ctrl-G) */
        }
        break;
#endif
#if MOUSE
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
        if (WinMouse (message, wParam, lParam) == FALSE)
            goto unhandled_message;
        break;
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
        if (WinKeyboard (message, wParam, lParam) == FALSE)
            goto unhandled_message;
        break;

     /************************************************************************
      * MISCELLANEOUS Handling
      * Timers, paint events, clipboard, termination etc.
      ************************************************************************/
    case WM_TIMER:
        if(wParam >= NUM_TIMERS)
            /* unhandled timer */
            return TRUE;
        meTimerState[wParam] = (meTimerState[wParam] & ~TIMER_SET) | TIMER_EXPIRED;
        break;

    case WM_SIZE:
        /* Resize the window when requested. */
        switch (wParam)
        {
        case SIZE_MAXIMIZED:
            eCellMetrics.maximized = 1; /* Set the maximised state */
            goto do_window_resize;
        case SIZE_RESTORED:
            eCellMetrics.maximized = 0; /* Release the minimised state */
do_window_resize:
            if (ttinitialised)
                WinTermResize ();
        case SIZE_MINIMIZED:            /* Minimized - ignore this case
                                         * Do not re-size the screen */
        default:
            break;
        }
        break;

    case WM_GETMINMAXINFO:
        /* Pick up the maximise and minimize information */
        eCellMetrics.minMaxInfo = *((LPMINMAXINFO)(lParam));
        break;

    case WM_RENDERALLFORMATS:           /* Clipboard data requests */
    case WM_RENDERFORMAT:
        {
            HANDLE hmem;

            hmem = WinKillToClipboard ();
            if (hmem == NULL)
                hmem = GlobalAlloc (GHND, 1);

            OpenClipboard (ttHwnd);
            EmptyClipboard ();
            SetClipboardData ((ttlogfont.lfCharSet == OEM_CHARSET) ? CF_OEMTEXT : CF_TEXT, hmem);
            CloseClipboard ();
            /* Force the stale state. If another application is pulling data
             * from us while we are the clipboard owner we must force the
             * clipboard to be refreshed whenever the 'yank' buffer changes.
             * If nobody is listening then we do not care. This simply allows
             * us to process optimally and not to continually report a
             * clipboard change whenever we change the yank data. */
            clipState |= CLIP_STALE;
        }
        break;

    case WM_PAINT:
        if (ttinitialised)
            WinPaint (hWnd);            /* Paint the Window */
        break;

    case WM_CLOSE:
        if (ttThreadId != GetCurrentThreadId ())
            PostMessage (ttHwnd, WM_CLOSE, 0, 0);
        else if(TTfocus)
        {
            /* Use the normal command to save buffers and exit
             * if we have focus.
             * If it doesn't exit then carry on as normal
             * Must ensure we ask the user, not a macro
             */
            int savcle ;
            savcle = clexec ;
            clexec = FALSE ;
            saveExitEmacs(0,1) ;
            clexec = savcle ;
            return FALSE ;
        }
        else if (!((anycb() == FALSE)   /* All buffers clean.   */
#ifdef _IPIPES
                   && ((ipipes == NULL)
#ifdef _CLIENTSERVER
                       || ((ipipes->pid == 0) && (ipipes->next == NULL))
#endif
                       )
#endif
                 ))
        {
            /* Display the modeless Cancel dialog box and disable the
             * application window. */
            if (DialogBox (ttInstance,
                           MAKEINTRESOURCE (IDD_QUITEXIT),
                           ttHwnd,
                           (DLGPROC) WinQuitExit) == FALSE)
                return FALSE ;
        }
        meDie ();
        break;

    case WM_DESTROY:
        WinShutdown ();
        /* End of call */
        PostQuitMessage(0);
        break;
        
#ifdef WM_INPUTLANGCHANGE
    case WM_INPUTLANGCHANGE:
        /* the user has changed language, change the font type if different */
        if(ttlogfont.lfCharSet != wParam)
        {
            TTchangeFont (ttlogfont.lfFaceName,wParam,ttlogfont.lfWeight/100,
                          ttlogfont.lfHeight,ttlogfont.lfWidth) ;
        }
        goto unhandled_message;         /* must call the DefWindowProc as well */
#endif
        
    case WM_MOVE:
        /* Move; Handle the position of the window changing. Need to force a
         * re-paint of then window. */
        InvalidateRect (hWnd, NULL, FALSE);
        eCellMetrics.paintAll = 1 ;
        break;

        /* Indicate to the PAINT that we must honour the next draw region and
         * not try to optimise the painting otherwise we will not re-paint the
         * screen fully. */
    case WM_ERASEBKGND:
        eCellMetrics.paintAll = 1 ;
        goto unhandled_message;         /* Assume unhandled */

    default:
unhandled_message:
/*        fprintf(logfp,"Unhandled message %x %x %x\n",message, wParam, lParam) ;*/
/*        fflush(logfp) ;*/
        {
            int ii ;
            ii = DefWindowProc(hWnd, message, wParam, lParam) ;
            return ii ;
        }
    }
    return FALSE ;
}

/*
 * TTtitle. Put the name of the buffer into the window frame
 */

void
TTtitleText (uint8 *str)
{
    static uint8 buf [MAXBUF];           /* This must be static */

    if (str != NULL)
        meStrcpy (buf, str);
    else
        buf [0] = '\0';
#ifdef _TITLE_VER_MINOR
    meStrcat (buf, " - MicroEmacs '" meVERSION "." meVERSION_MINOR);
#else
    meStrcat (buf, " - MicroEmacs '" meVERSION) ;
#endif

#ifdef _WINCON
    if (meSystemCfg & meSYSTEM_CONSOLE)
        SetConsoleTitle (buf);	            /* Set console window title */
    else
#endif
        SetWindowText (ttHwnd, buf);        /* Change the window text */
}

/* TTsetBgcol; Set the default fill of the background. Now in windows then we
 * have to modify the class window in order to effect the default background
 * fill. */
void
TTsetBgcol (void)
{
    if (!(meSystemCfg & meSYSTEM_CONSOLE))
    {
        HBRUSH newBrush ;
        int bcol;

        /* Get the new background color scheme */
        bcol = meStyleGetBColor(meSchemeGetStyle(globScheme)) ;
        if (((newBrush = CreateSolidBrush (eCellMetrics.pInfo.cPal [bcol].cpixel)) != NULL) &&
            (SetClassLong (ttHwnd, GCL_HBRBACKGROUND, (LONG)(newBrush)) != (LONG)(NULL)))
        {
            /* The new brush has been installed. Delete the old brush if we
             * have defined it and remember the old context */
            if (ttBrush != NULL)
                DeleteObject (ttBrush);
            ttBrush = newBrush;
        }
        else if (newBrush != NULL)
            DeleteObject (newBrush);
    }
}
