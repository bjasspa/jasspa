/****************************************************************************
 *
 *			Copyright 1995-2004 Jon Green.
 *			   All Rights Reserved
 *
 *
 *  System        : nroff
 *  Module        : Main library
 *  Object Name   : $RCSfile: _nroff.h,v $
 *  Revision      : $Revision: 1.2 $
 *  Date          : $Date: 2004-01-06 00:53:50 $
 *  Author        : $Author: jon $
 *  Last Modified : <040104.0038>
 *
 *  Description
 *
 *  Notes
 *
 *  History
 *
 ****************************************************************************
 *
 *  Copyright (c) 1995-2004 Jon Green.
 *
 *  All Rights Reserved.
 *
 *  This Document may not, in whole or in part, be copied,
 *  photocopied, reproduced, translated, or reduced to any
 *  electronic medium or machine readable form without prior
 *  written consent from Jon Green.
 *
 ****************************************************************************/

#ifndef ___NROFF_H                       /* Guard file */
#define ___NROFF_H 1

/* Definitions for internal type representation */

#define USPACE_CHAR ((char) 1)
#define ITALIC_CHAR ((char) 2)
#define BOLD_CHAR   ((char) 3)
#define ROMAN_CHAR  ((char) 4)
#define MONO_CHAR   ((char) 5)

#define PS_MAX      20                  /* Maximum point size */
#define PS_MIN      7                   /* Minimum point size */
#define PS_DEFAULT  10                  /* Point size has default of 10 */

#define VS_OFF      2                   /* Spacing is 2 points bigger */
#define VS_DEFAULT  (PS_DEFAULT+VS_OFF) /* Vertical spacing default */
#define VS_MAX      (PS_MAX+VS_OFF)     /* Vertical spacing maximum */
#define VS_MIN      (PS_MIN+VS_OFF)     /* Vertical spacing minumim */

/* Nroff modes */

#define CMD_PAR     0x00000001L         /* Paragraph Command */
#define CMD_SEC     0x00000002L         /* Section Command */
#define CMD_BLK     0x00000004L         /* Blank line command */
#define CMD_TXT     0x00000008L         /* Text command line */
#define CMD_LTX     0x00000010L         /* Literal text command */
#define CMD_INV     0x00000020L         /* Invisible command */
#define CMD_PRE     0x00000040L         /* Pre Space commad */
#define CMD_PST     0x00000080L         /* Post space command */
#define CMD_SPA     0x00000100L         /* Sub-paragraph command */
#define CMD_PPO     (CMD_PRE|CMD_PST)   /* Pre and post space */

#define CMD_MRF     0x00001000L         /* R  Mode */
#define CMD_MBF     0x00002000L         /* B  Mode */
#define CMD_MIF     0x00004000L         /* I  Mode */
#define CMD_MCF     0x00008000L         /* C  Mode */
#define CMD_MSF     0x00000800L         /* S  Mode */

#define CMD_MRS     0xF0000000L         /* RS Mode */
#define CMD_MCS     0x00010000L         /* CS Mode */
#define CMD_MBS     0x00020000L         /* BS Mode */
#define CMD_MID     0x00040000L         /* ID Mode */
#define CMD_MIM     0x00080000L         /* Im Mode */
#define CMD_MTH     0x00100000L         /* TH Mode */
#define CMD_MXI     0x00200000L         /* XI Mode */
#define CMD_MFH     0x00400000L         /* FH Mode */
#define CMD_MFI     0x00800000L         /* FI Mode fill */
#define CMD_MAD     0x01000000L         /* AD Mode - adjust */
#define CMD_MSH     0x02000000L         /* SH Mode */
#define CMD_MTP     0x04000000L         /* TP Mode */

#define CMD_MRS_GET(x)   (((x)>>28)&0x0000000fL)
#define CMD_MRS_SET(x,y) (((x)&(~CMD_MRS))|((((long)((y)&0x0f))<<28)&CMD_MRS))

/* Error reporting routines */

extern void nrWarn (char *format , ...);
extern void nrError (char *format , ...);
extern int  nrErrorTotal (int flag);

/* External References */

extern int isvalidstr (char *s);
extern int isalnumstr (char *s);

/* Parameter list processing */

extern char *getFirstParam (char **sp);
extern char *getAllParam (char **sp);
extern char *getOnlyParam (char **sp);
extern char *getRestOfLineParam (char **sp);

extern void nrCheckTextMode (char *s);
extern void nrCheckParaMode (char *s);

#endif
