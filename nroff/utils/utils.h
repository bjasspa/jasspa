/****************************************************************************
 *
 *			Copyright 1996-2004 Jon Green.
 *                         All Rights Reserved
 *
 *
 *  System        :
 *  Module        :
 *  Object Name   : $RCSfile: utils.h,v $
 *  Revision      : $Revision: 1.2 $
 *  Date          : $Date: 2004-01-06 00:52:20 $
 *  Author        : $Author: jon $
 *  Last Modified : <040104.0023>
 *
 *  Description
 *
 *  Notes
 *
 *  History
 *
 ****************************************************************************
 *
 *  Copyright (c) 1996-2004 Jon Green.
 *
 *  All Rights Reserved.
 *
 * This  Document  may  not, in  whole  or in  part, be  copied,  photocopied,
 * reproduced,  translated,  or  reduced to any  electronic  medium or machine
 * readable form without prior written consent from Jon Green.
 *
 ****************************************************************************/

#ifndef _UTILS_H
#define _UTILS_H

#define bufArgInit(x) (*((Args *)(x))->argc = 0)
#define UERROR_LOG_APPEND 1
#define UERROR_LOG_CREATE 0

typedef struct {
    char **argv;
    int  argc;
} Args;

/* External References */

extern char *bufStr (char *h, char *s);
extern char *bufNStr (char *h, char *s);
extern char *bufChr (char *h, char c);
extern char *bufFormat (char *h, char *format, ... );
extern char *bufFree (char *h);

extern char **bufNArg (char **argv, int *argc, char *argp);
extern void bufNArgFree (char **arg, int i);
extern void bufArg (Args *args, char *argp);
extern void bufArgFree (Args *args);

extern char *varStrGetFirst (char *comp);
extern char **varStrGetAll (char *comp);
extern int varStrCmp (char *item, char *ritem);
extern int varStrCmpAll (char *item, char **ritem);
extern int varStrCmpAllC (char *item, char *ritem);

/*
 * Case insensitive character comparisions
 */

int strncasecmp (const char *s1, const char *s2, size_t n);
int strcasecmp (const char *s1, const char *s2);
int strnullcmp (const char *s1, const char *s2);
/*
 * White space functions.
 */

char *skipWhiteSpace (char *s);
char *trimWhiteSpace (char *s);
char *compWhiteSpace (char *s);
char *cleanWhiteSpace (char *s);

/*
 * Error reporting
 */

#ifndef NDEBUG
#define uDebug(x,y)         do {if ((x) <= uDebugLevel) __uDebug y; } while (0)
#define uDebugM(x,y,z)      do {if ((x&y) != 0) __uDebug y; } while (0)
#define duDebug(x,y)         do {if ((x) <= uDebugLevel) __uDebug z; } while (0)
#define duDebugM(x,y,z)      do {if ((x&y) != 0) __uDebug z; } while (0)
#else
#define uDebug(x,y)         /* NULL */
#define uDebugM(x,y,z)      /* NULL */
#define duDebug(x,y)         /* NULL */
#define duDebugM(x,y,z)      /* NULL */
#endif

extern int  uDebugLevel;
extern void uUtilitySet (char *name);
extern void uErrorSet (int max, int *count);
extern void uWarnSet (int *count);
extern void uVerboseSet (int level);
extern void uDebugSet (int level);
extern void uInteractiveSet (int i);
extern void uFileSet (int *lineNo, char *filename);
extern void uErrorIgnore (int enable);
extern void uWarnIgnore (int enable);
extern void uErrorChannelSet (char *name, int mode);

extern void uOpenErrorChannel (void);
extern void uCloseErrorChannel (void);
extern void __uDebug (char *format, ...);
extern void uFatal (char *format, ...);
extern int  uError (char *format, ...);
extern void uReport (char *format, ...);
extern void uWarn (char *format, ...);
extern void uInteractive (char *format, ...);
extern void uVerbose (int i, char *format, ...);
extern void duVerboseM (int i, int j, char *format, ...);

extern char *uVersion (void);

/*
 * File expansion - for DOS.
 * getfile.c
 */
extern char **getfiles (int *argc, char *argv[], int optind);

#define duError uError
#define duWarn  uWarn
#define duFatal uFatal
#define duReport uReport

/*
 * File handling utilities.
 */

extern char *makeFilename (char *drive, char *path, char *base, char *ext);
extern int  splitFilename (char *fname, char **adrive, char **apath, char **abase, char **aext);
extern char *unifyFilename (char *afname);
extern char *reslashFilename (char *buf, char *name, char from, char to);
#endif
