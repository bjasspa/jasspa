/****************************************************************************
 *
 *			Copyright 1997 Jon Green.
 *			    All Rights Reserved
 *
 *
 *  System        :
 *  Module        :
 *  Object Name   : $RCSfile: rtf.h,v $
 *  Revision      : $Revision: 1.1 $
 *  Date          : $Date: 2000-10-21 14:31:31 $
 *  Author        : $Author: jon $
 *  Last Modified : <110798.1202>
 *
 *  Description
 *
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

#ifndef RTF_H
#define RTF_H

#define FULL_INDENT 360                 /* Full indent distance. */
#define HALF_INDENT (FULL_INDENT/2)     /* Half indent distance. */
#define LINE_HIEGHT 100                 /* twips height */

#if 0
/* Supposed to be standard Windows */
#define SS_FONT     2                   /* SS Section font type */
#define SS_SIZE     24                  /* SS Section font size */
#define SH_FONT     2                   /* SH Section font type */
#define SH_SIZE     24                  /* SH Section font size */
#define TX_FONT     2                   /* Text section font type */
#define TX_SIZE     24                  /* Text section font size */
#define CS_FONT     4                   /* Code section font type */
#define CS_SIZE     20                  /* Code section font size */
#define TL_FONT     2                   /* Title line font type */
#define TL_SIZE     28                  /* Title line font size */
#define MO_FONT     4                   /* Mono font type */
#define MO_SIZE     24                  /* Mono font size */
#define CO_FONT     2                   /* Copyright font type */
#define CO_SIZE     20                  /* Copyright font size */
#else
/* Smaller font */
#define SS_FONT     2                   /* SS Section font type */
#define SS_SIZE     20                  /* SS Section font size */
#define SH_FONT     2                   /* SH Section font type */
#define SH_SIZE     24                  /* SH Section font size */
#define TX_FONT     2                   /* Text section font type */
#define TX_SIZE     20                  /* Text section font size */
#define CS_FONT     4                   /* Code section font type */
#define CS_SIZE     18                  /* Code section font size */
#define TL_FONT     2                   /* Title line font type */
#define TL_SIZE     28                  /* Title line font size */
#define MO_FONT     4                   /* Mono font type */
#define MO_SIZE     20                  /* Mono font size */
#define CO_FONT     2                   /* Copyright font type */
#define CO_SIZE     18                  /* Copyright font size */
#endif

char *rtfHeader[] =
{
    "{\\rtf1\\ansi \\seff0\\deflang1024",
    "",
    "{\\fonttbl",
    "{\\f0\\froman Times New Roman;}",
    "{\\f1\\froman Symbol;}",
    "{\\f2\\fswiss Arial;}",
    "{\\f3\\froman Serif;}",
    "{\\f4\\fmodern Courier New;}",
    "}",
    "{\\colortbl;",
    "\\red0\\green0\\blue0;",
    "\\red0\\green0\\blue255;",
    "\\red0\\green255\\blue0;",
    "\\red255\\green0\\blue0;",
    "\\red255\\green255\\blue255;",
    "}",
    NULL
};

char *rtfFooter = "}";

typedef struct {
    int ss_font;                        /* SS Section font type */
    int ss_size;                        /* SS Section font size */
    int sh_font;                        /* SH Section font type */
    int sh_size;                        /* SH Section font size */
    int tx_font;                        /* Text section font type */
    int tx_size;                        /* Text section font size */
    int cs_font;                        /* Code section font type */
    int cs_size;                        /* Code section font size */
    int tl_font;                        /* Title line font type */
    int tl_size;                        /* Title line font size */
    int mo_font;                        /* Mono font type */
    int mo_size;                        /* Mono font size */
    int co_font;                        /* Copyright font type */
    int co_size;                        /* Copyright font size */
} fontType;

extern fontType *rtfFont;

#endif
