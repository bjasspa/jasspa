/****************************************************************************
 *
 *			Copyright 1997 Jon Green.
 *			    All Rights Reserved
 *
 *
 *  System        : MicroEmacs
 *  Module        : MicroEmacs ICON File.
 *  Object Name   : $RCSfile: meicons.c,v $
 *  Revision      : $Revision: 1.1 $
 *  Date          : $Date: 2001-05-29 14:32:27 $
 *  Author        : $Author: jon $
 *  Last Modified : <220297.0956>
 *
 *  Description	  : This is a simple file used as a respository for
 *                  MicroEMACS Icons. It does nothing.
 *  Notes
 *
 *  History
 *
 *  $Log: not supported by cvs2svn $
 *
 ****************************************************************************
 *
 *  Copyright (c) 1997 Jon Green.
 *
 *  All Rights Reserved.
 *
 *  This Document may not, in whole or in part, be copied,
 *  photocopied, reproduced, translated, or reduced to any
 *  electronic medium or machine readable form without prior
 *  written consent from Jon Green.
 ****************************************************************************/

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
