/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * eprint.h - Printer definitions.
 *
 * Copyright (C) 1998-2009 JASSPA (www.jasspa.com)
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
 * Created:     Mon Mar 9 1998
 * Synopsis:    Printer definitions.
 * Authors:     Jon Green
 * Description:
 *      Includes eprint.def to create a list of used printer registry values.
 */

#ifndef __EPRINT_H
#define __EPRINT_H

/* Local printer types include
 * mePD_INT   0x01     Integer data
 * mePD_STR   0x00     String data
 * mePD_WIN   0x02     Autosave items for windows into Microemacs registry. 
 */
#define mePD_STR   0x00                   /* String data */
#define mePD_INT   0x01                   /* Integer data */
#define mePD_WIN   0x02                   /* Windows autosave */

/* Define the meaning for bits in /print/flags */ 
#define PFLAG_ENBLHEADER         0x010    /* Enable the header */
#define PFLAG_ENBLFOOTER         0x020    /* Enable the footer */
#define PFLAG_ENBLLINENO         0x040    /* Enable line nos */
#define PFLAG_ENBLTRUNCC         0x080    /* Enable truncated line '\' char */
#define PFLAG_SILENT             0x100    /* Print silently */

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
    meInt l;                             /* Integer component */
    meUByte *p;                            /* Character pointer */
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
    meUByte **filter;                     /* Character filter table */
    int colorNum;                       /* Number of printer colors */
    int schemeNum;                      /* Number of printer schemes */
    meUInt *color;                      /* Color table */
    meStyle *scheme;                    /* Scheme table */
} PRINTER;

extern meUByte *printNames[];
extern meUByte printTypes[];
extern PRINTER printer;

#define mePrintColorGetRed(cc)      ((cc & 0x00ff0000) >> 16)
#define mePrintColorGetGreen(cc)    ((cc & 0x0000ff00) >> 8)
#define mePrintColorGetBlue(cc)     ((cc & 0x000000ff))

#define mePrintColorSetRed(cc,rr)   (cc | (((meUInt) (rr & 0xff)) << 16))
#define mePrintColorSetGreen(cc,gg) (cc | (((meUInt) (gg & 0xff)) << 8))
#define mePrintColorSetBlue(cc,bb)  (cc | ((meUInt) (bb & 0xff)))

#endif /* __EPRINT_H */
