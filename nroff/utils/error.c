/****************************************************************************
 *
 *			Copyright 1996-2004 Jon Green.
 *                         All Rights Reserved
 *
 *
 *  System        :
 *  Module        :
 *  Object Name   : $RCSfile: error.c,v $
 *  Revision      : $Revision: 1.2 $
 *  Date          : $Date: 2004-01-06 00:52:20 $
 *  Author        : $Author: jon $
 *  Last Modified : <040104.0022>
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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "utils.h"

       int  uDebugLevel = 0;            /* Debugging level */
static int  uVerboseLevel = 1;          /* Verbose level */
static int  *uLineNo = NULL;            /* Pointer to file line number */
static char *uFileName = NULL;          /* Pointer to filename */
static int  *uErrorCount = NULL;        /* Pointer to error count */
static int  *uWarnCount = NULL;         /* Pointer to warn count */
static int  uErrorMax = -1;             /* Maximum Num errors before abort */
static int  uInteractiveEnable = 1;     /* Enable interactive mode */
static FILE *fperror = NULL;            /* Error channel */
static char *uProgname = NULL;          /* Name of utility */
static int  uErrorIgnoreFlag = 0;       /* Ignore errors */
static int  uWarnIgnoreFlag = 0;        /* Ignore warnings */
static char *uChannelName = NULL;       /* Error chanel name */
static int  uChannelMode = 0;           /* Error channel mode */

/*
 * uInteractiveMode (int i)
 * Enable/Disable interactive mode.
 * i == 0 - disable.
 * i != 0 - enable.
 */

void
uInteractiveSet (int i)
{
    uInteractiveEnable = i;
}

void
uUtilitySet (char *name)
{
    uProgname = name;
}

void
uErrorSet (int max, int *count)
{
    uErrorCount = count;
    uErrorMax = (max == 0) ? -1 : max;  /* Only trap if >= 1 */
}

void uErrorIgnore (int enable)
{
    uErrorIgnoreFlag = enable;
}

void uWarnIgnore (int enable)
{
    uWarnIgnoreFlag = enable;
}

void
uWarnSet (int *count)
{
    uWarnCount = count;
}

void
uDebugSet (int level)
{
    uDebugLevel = level;
}

void
uVerboseSet (int level)
{
    uVerboseLevel = level;
}

void
uFileSet (int *lineNo, char *filename)
{
    uLineNo = lineNo;
    uFileName = filename;
}

void uErrorChannelSet (char *name, int mode)
{
    if (uChannelName != NULL)
        free (uChannelName);
    if (name == NULL)
        uChannelName = NULL;
    else
        uChannelName = strdup (name);
    uChannelMode = mode;
}

void
uOpenErrorChannel (void)
{
    if (fperror != NULL) {
        fclose (fperror);
        fperror = NULL;
    }
    if (uChannelName == NULL)
    {
        fperror = NULL;
        return;
    }
    if ((fperror = fopen (uChannelName,
                          (uChannelMode == 0) ? "w" : "a")) == NULL)
    {
        fprintf (stderr, "Cannot open error logging file [%s]\n",
                 uChannelName);
        exit (2);
    }
}

void
uCloseErrorChannel (void)
{
    if (fperror != NULL)
        fclose (fperror);
    fperror = NULL;
    uInteractive (NULL);
}

/*
 * Report Warnings, Errors and Fatal messages.
 */
void
uInteractive (char *format, ...)
{
    static int currentLen = 0;
    va_list ap;

    if (uInteractiveEnable == 0)        /* Interactive enabled ?? */
        return;                         /* No - quit now */

    /*
     * Clear the previous entry. If there is
     * no more output then clear line and quit early.
     */

    if (currentLen > 0)
    {
        while (--currentLen >= 0)
            fprintf (stdout, "\b \b");
        if (format == NULL) {
            fflush (stdout);
            return;
        }
    }
    else if (format == NULL)
        return;

    /*
     * Display the interactive message.
     * Rememeber the line length so we can clear later.
     */

    if (uProgname != NULL)
        currentLen = fprintf (stdout, "%s: ", uProgname);
    va_start (ap, format);
    currentLen += vfprintf (stdout, format, ap);
    va_end (ap);
    fflush (stdout);
}

static void
_uReport (FILE *fp, char *label, char *format, va_list ap)
{
    fprintf (fp, label);
    if ((uFileName != NULL) && (uLineNo != NULL))
        fprintf (fp, "%s: %d: ", uFileName, *uLineNo);
    else if (uProgname != NULL)
        fprintf (fp, "%s: ", uProgname);
    vfprintf(fp, format, ap);
    fflush (fp);
}

void
__uDebug (char *format, ...)
{
    static char *label = "DEBUG: ";
    va_list ap;

    va_start(ap, format);
    _uReport (stderr, label, format, ap);
    if (fperror != NULL)
        _uReport (fperror, label, format, ap);
    va_end(ap);
}

void
uFatal (char *format, ...)
{
    static char *label = "FATAL: ";
    va_list ap;

    uInteractive (NULL);

    va_start(ap, format);
    _uReport (stderr, label, format, ap);
    if (fperror != NULL)
        _uReport (fperror, label, format, ap);
    va_end(ap);

    uCloseErrorChannel ();
    exit (1);
}

int
uError (char *format, ...)
{
    static char *label = "ERROR: ";
    va_list ap;

    if (uErrorIgnoreFlag > 0)
        return (0);

    uInteractive (NULL);

    va_start(ap, format);
    _uReport (stderr, label, format, ap);
    if (fperror != NULL)
        _uReport (fperror, label, format, ap);
    va_end(ap);

    if (uErrorCount != NULL)
    {
        *uErrorCount += 1;
        if ((uErrorMax >= 0) && (*uErrorCount > uErrorMax))
            uFatal ("Too many errors\n");
    }
    return (-1);
}

void
uReport (char *format, ...)
{
    static char *label = "REPORT: ";
    va_list ap;

    uInteractive (NULL);

    va_start(ap, format);
    _uReport (stderr, label, format, ap);
    if (fperror != NULL)
        _uReport (fperror, label, format, ap);
    va_end(ap);
}

void
uWarn (char *format, ...)
{
    static char *label = "WARN: ";
    va_list ap;

    if (uWarnIgnoreFlag > 0)
        return;

    uInteractive (NULL);

    va_start(ap, format);
    _uReport (stderr, label, format, ap);
    if (fperror != NULL)
        _uReport (fperror, label, format, ap);
    va_end(ap);

    if (uWarnCount != NULL)
        *uWarnCount += 1;
}

void duVerboseM (int i, int j, char *format, ...)
{
    static char *label = "VERBOSE: ";
    va_list ap;

    if ((i & j) == 0)
        return;

    va_start(ap, format);
    _uReport (stderr, label, format, ap);
    if (fperror != NULL)
        _uReport (fperror, label, format, ap);
    va_end(ap);
}

void
uVerbose (int i, char *format, ...)
{
    va_list ap;

    if (i >= uVerboseLevel)
        return;
    else
        uInteractive (NULL);

    va_start(ap, format);
    vfprintf(stdout, format, ap);
    fflush (stdout);
    va_end(ap);
}
