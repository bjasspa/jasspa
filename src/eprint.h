/* -*- C -*- ****************************************************************
 *
 *  			Copyright 1999 Jasspa
 *
 *  System        : MicroEmacs Jasspa Distribution
 *  Object Name   : eprint.h
 *  Created By    : Jon Green
 *  Created       : Mon Mar 9 23:54:24 1998
 *  Last Modified : <000718.2343>
 *
 *  Description	
 *       Printer definitions
 *  Notes
 *
 *  History
 *	
 ****************************************************************************
 *
 * Copyright (C) 1998 - 1999, JASSPA 
 *
 * The MicroEmacs Jasspa distribution can be copied and distributed freely for
 * any non-commercial purposes. The MicroEmacs Jasspa Distribution can only be
 * incorporated into commercial software with the expressed permission of
 * JASSPA.
 *  
 ****************************************************************************/

#ifndef __EPRINT_H
#define __EPRINT_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

/* Local printer types include
 * mePD_INT   0x01     Integer data
 * mePD_STR   0x00     String data
 * mePD_WIN   0x02     Autosave items for windows into Microemacs registry. 
 */
#define mePD_STR   0x00                   /* String data */
#define mePD_INT   0x01                   /* Integer data */
#define mePD_WIN   0x02                   /* Windows autosave */

/* Define the meaning for bits in /print/flags */ 
#define PFLAG_ENBLHEADER         0x10   /* Enable the header */
#define PFLAG_ENBLFOOTER         0x20   /* Enable the footer */
#define PFLAG_ENBLLINENO         0x40   /* Enable line nos */
#define PFLAG_ENBLTRUNCC         0x80   /* Enable truncated line '\' char */

/* Define the destinations */
#define PDEST_BUFFER             0x00   /* To Buffer only */
#define PDEST_INTERNAL           0x01   /* Internal queue */
#define PDEST_FILE               0x02   /* To File only */
#define PDEST_COMLINE            0x03   /* To File & Command line */

/*
 * For the windows dialogue define the timer constants used to update the
 * dialogue. 
 */
#ifdef _WIN32
#define PRINT_TIMER_ID           10     /* Timer handle identity */
#define PRINT_TIMER_RESPONSE    100     /* Respond within 100 milliseconds */
#endif

/*
 * PDESC
 * Print descriptor.
 */
typedef union
{
    int32 l;                             /* Integer component */
    uint8 *p;                            /* Character pointer */
} LPU;

enum
{
#define DEFPRINT(s,t,v) v,
#include "eprint.def"
    PRINT_MAX
#undef DEFPRINT
};

typedef struct
{
    LPU param [PRINT_MAX];              /* Parameter */
    struct tm *ptime;                   /* Print time */
    int pInternal;                      /* Internal driver, i.e. windows */
    int pDestination;                   /* Output destination */
    int pNoLines;                       /* Number of lines */
    int pPageNo;                        /* Page number */
    int pLineNo;                        /* Line number */
    int pLinesPerPage;                  /* Number of lines per page */
    int pLineNumDigits;                 /* Number of line number digits */
    int pNoHeaderLines;                 /* Number of lines needed for header */
    int pNoFooterLines;                 /* Number of lines needed for footer */
    uint8 **filter;                     /* Character filter table */
    int colorNum;                       /* Number of printer colors */
    int schemeNum;                      /* Number of printer schemes */
    uint32 *color;                      /* Color table */
    meSTYLE *scheme;                    /* Scheme table */
} PRINTER;

extern uint8 *printNames[];
extern uint8 printTypes[];
extern PRINTER printer;

#define mePrintColorGetRed(cc)      ((cc & 0x00ff0000) >> 16)
#define mePrintColorGetGreen(cc)    ((cc & 0x0000ff00) >> 8)
#define mePrintColorGetBlue(cc)     ((cc & 0x000000ff))

#define mePrintColorSetRed(cc,rr)   (cc | (((uint32) (rr & 0xff)) << 16))
#define mePrintColorSetGreen(cc,gg) (cc | (((uint32) (gg & 0xff)) << 8))
#define mePrintColorSetBlue(cc,bb)  (cc | ((uint32) (bb & 0xff)))

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif
#endif /* __EPRINT_H */
