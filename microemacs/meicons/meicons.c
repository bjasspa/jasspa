/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * meicons.c - Main entry point.
 *
 * Originally written by Jon Green
 * Copyright (C) 1997-2002 JASSPA (www.jasspa.com)
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
 * Created:     Jon Green (JASSPA)
 * Synopsis:    Main entry point.
 * Description: This is a simple file used as a respository for
 *              MicroEMACS Icons. It does nothing.
 */

#include <stdlib.h>                     /* Standard library definitions  */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>                    /* Window definitions */
#undef WIN32_LEAN_AND_MEAN
#include "meicons.h"                    /* Definitions file */

/****************************************************************************

    FUNCTION: WinMain(HANDLE, HANDLE, LPSTR, int)

    PURPOSE: calls initialization function, processes message loop

****************************************************************************/

int APIENTRY
WinMain (HANDLE hInstance, HANDLE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    MSG msg;                            /* Message buffer */
    HWND   ttHwnd;                      /* This is the window handle */

    /* Initialise the window data and register window class */
    if (!hPrevInstance)
    {
        LONG APIENTRY MainWndProc(HWND, UINT, UINT, LONG);
        WNDCLASS  wc;

        wc.style = 0; /*CS_DBLCLKS; */         /* double-click messages */
        wc.lpfnWndProc = (WNDPROC) MainWndProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = hInstance;
        wc.hIcon = LoadIcon (hInstance, MAKEINTRESOURCE(IDI_ICON_001));
        wc.hCursor = NULL;
        wc.hbrBackground = GetStockObject(BLACK_BRUSH);
        wc.lpszMenuName =  "InputMenu";
        wc.lpszClassName = "MicroEmacsClass";

        if (!RegisterClass (&wc))
            return (FALSE);
    }

    ttHwnd = CreateWindow ("MicroEmacsClass",
                           "MicroEmacs",
                           WS_OVERLAPPEDWINDOW,  /* No scroll bars */
                           CW_USEDEFAULT,
                           CW_USEDEFAULT,
                           CW_USEDEFAULT,
                           CW_USEDEFAULT,
                           NULL,
                           NULL,
                           hInstance,
                           NULL);

    if (!ttHwnd)
        return (FALSE);

    ShowWindow(ttHwnd, 0);          /* Create the window */
    UpdateWindow(ttHwnd);           /* Show it off - ready for errors */
    ExitProcess(0);

    if (GetMessage(&msg,            /* address of structure with message */
                   ttHwnd,          /* handle of window */
                   0,               /* first message */
                   0) != FALSE)     /* last message */
    {

        TranslateMessage (&msg);    /* Translate keyboard characters */
        DispatchMessage (&msg); /* Unhandled */
    }
    return (0);
}

/****************************************************************************

    FUNCTION: MainWndProc(HWND, unsigned, WORD, LONG)

    PURPOSE:  Processes messages

    MESSAGES:

        WM_CREATE     - create window
        WM_DESTROY    - destroy window

    COMMENTS:

        This demonstrates how input messages are received, and what the
        additional information is that comes with the message.

****************************************************************************/

LONG APIENTRY
MainWndProc (HWND hWnd, UINT message, UINT wParam, LONG lParam)
{
    switch (message)
    {
    case WM_CREATE:
        break;

    case WM_DESTROY:                    /* End of call */
        PostQuitMessage(0);
        break;
    default:
        return (DefWindowProc(hWnd, message, wParam, lParam));
    }
    return (FALSE);
}
