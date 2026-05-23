/* -*- C -*- *****************************************************************
 *
 * Copyright (C) 2026 Maxinity Software Ltd (maxinity.co.uk).
 *
 * All rights reserved.
 * 
 * This document may not, in whole or in part, be copied, photocopied,
 * reproduced, translated, or reduced to any electronic medium or machine
 * readable form without prior written consent from Maxinity Software Ltd.
 *
 * Synopsis:    
 * Authors:     Steven Phillips
 *
 *****************************************************************************/

#ifndef __NWTERM_H
#define __NWTERM_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include "../eterm.h"

/* Dirty-region tracking – mirrors the Windows TTinitArea/TTaddArea/TTapplyArea
 * macros from eterm.h but using a plain struct (no Win32 RECT needed). */
typedef struct { int left; int right; int top; int bottom; } meNWRect;
extern meNWRect ttRect;

#undef  TTinitArea
#define TTinitArea() \
    (ttRect.left=0x7ffff, ttRect.right=-1, ttRect.top=0x7ffff, ttRect.bottom=-1)

#undef  TTsetArea
#define TTsetArea(r,c,h,w) \
    (ttRect.left=(c), ttRect.right=(c)+(w), ttRect.top=(r), ttRect.bottom=(r)+(h))

#undef  TTaddArea
#define TTaddArea(r,c,h,w)                       \
do {                                             \
    if (ttRect.left   > (c))     ttRect.left   = (c);     \
    if (ttRect.right  < (c)+(w)) ttRect.right  = (c)+(w); \
    if (ttRect.top    > (r))     ttRect.top    = (r);     \
    if (ttRect.bottom < (r)+(h)) ttRect.bottom = (r)+(h); \
} while(0)

extern void TTapplyArea(void);
extern void TTputs(int row, int col, int len);

extern int  TTstart(void);
#define TTend()     TTclose()
extern int  TTopen(void);
extern int  TTclose(void);
extern void TTflush(void);
extern void TTshowCur(void);
extern void TThideCur(void);
#undef TTNbell
#undef TTbeep
extern void TTNbell(void);
extern void TTbeep(void);
extern int  TTaddColor(meColor index, meUByte r, meUByte g, meUByte b);
extern void TTsetBgcol(void);
extern int  meMain(int argc, char *argv[]);
extern void meFrameSetWindowTitle(meFrame *frame, meUByte *title);
extern void meFrameSetWindowSize(meFrame *frame);
extern void meFrameRepositionWindow(meFrame *frame, int resize);
extern int  meFrameTermInit(meFrame *frame, meFrame *sibling);
extern void meFrameTermFree(meFrame *frame, meFrame *sibling);
extern void meFrameTermMakeCur(meFrame *frame);
#define meXFONT_MASK         (meFONT_BOLD|meFONT_ITALIC|meFONT_LIGHT)
#define meStyleGetXFont(ss)  (((ss)>>16) & meXFONT_MASK)

typedef struct
{
    int       fwidth;                   /* Font width in pixels */
    int       fdepth;                   /* Font depth in pixels */
    int       fhwidth;                  /* Font half width in pixels */
    int       fhdepth;                  /* Font half depth in pixels */
    int       fadepth;                  /* Font up-arrow depth in pixels */
    int       ascent;                   /* Font ascent */
    int       underline;                /* The underline position */
    char     *fontName;
    int       fontSize;
    int       fontMono;
} meCellMetrics;                        /* The character cell metrics */

extern meCellMetrics mecm;

#define colToClient(x)   (mecm.fwidth  * (x))
#define rowToClient(y)   (mecm.fdepth  * (y))
#define clientToCol(x)   ((x) / mecm.fwidth)
#define clientToRow(y)   ((y) / mecm.fdepth)

#ifdef _CLIPBRD
extern void TTinitClipboard(void);
extern void TTgetClipboard(int flag);
extern void TTsetClipboard(int cpData);
#endif

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif
#endif /* __NWTERM_H */
