/****************************************************************************
 *
 *			Copyright 1996-2004 Jon Green.
 *                         All Rights Reserved
 *
 *
 *  System        :
 *  Module        :
 *  Object Name   : $RCSfile: superman.c,v $
 *  Revision      : $Revision: 1.3 $
 *  Date          : $Date: 2004-01-06 00:53:51 $
 *  Author        : $Author: jon $
 *  Last Modified : <040103.1956>
 *
 *  Description
 *
 *  Notes
 *
 *  History
 *
 * 1.0.4h - JG 03/01/04 - Ported to Sun Solaris 9.
 * 1.0.4g - JG 21/10/00 - Corrected the file extension comparison.
 * 1.0.4f - JG 20/04/97 - Added base name option.
 * 1.0.4e - JG 29/08/96 - Added UNIX file name utilities.
 * 1.0.4d - JG 05/01/95 - Added external references
 * 1.0.4c - JG 16/11/95 - Ported to UNIX world
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
#include <ctype.h>
#include <signal.h>
#if ((defined _HPUX) || (defined _LINUX) || (defined _SUNOS))
#include <unistd.h>
#else
#include <getopt.h>
#endif

#include <utils.h>

/*
 */

#define MODULE_VERSION  "1.0.4h"
#define MODULE_NAME     "superman"

static char *progname = MODULE_NAME;    /* Name of the module */
static char *outFile = NULL;
static int  useBase = 0;                /* Use the base name */

static FILE *fo;                        /* Output File Pointer */
static int  curVerbose = 1;             /* Current verbose level */
#ifdef _UNIX
static char *eolStr = "\n";
static char eofStr [2] = {0, 0};
#else
static char *eolStr = "\r\n";
static char eofStr [2] = {0x1A, 0};
#endif

#if 1
static int
readLine (FILE *fp, char *cbuf)
{
    int     c;
    int     len;

    if (fp == NULL)
        return (-1);

    cbuf [0] = '\0';
    len = 0;
    for (;;)
    {
        c = fgetc (fp);
        if ((c == EOF) || (c == 0x1A))
            return (-1);

        if (c == '\n') {
            cbuf [len] = '\0';
            return (len);
        }
        else if (c == '\r')
            continue;
        cbuf [len++] = c;
    }   /* End of 'while' */
}   /* End of 'readLine' */

static void
doSuperMan (int pass, char *fname)
{
    FILE    *fp;
    char    buf [1024];
    int     mode = 0;
    char    *smfname;
    int     lineNo;

    if ((fp = fopen (fname, "rb")) == NULL) {
        uError ("Cannot open file [%s] for reading.\n", fname);
        return;
    }

    uFileSet (&lineNo, fname);
    while (readLine (fp, buf) >= 0)
    {
        lineNo++;
        if (strncmp (".SUPERMANINC",buf, 12) == 0) {
            if (mode != 0)
                uError("Unexpected %s\n", buf);
            else if (pass == 1) {
                smfname = &buf [13];
                uInteractive ("Archiving [%s]", smfname);
                mode = 0x81;
            }
        }
        else if (strncmp (".SUPERMANFILE", buf, 13) == 0) {
            if (mode != 0)
                uError("Unexpected %s\n", buf);
            else if (pass == 2) {
                mode = 0x82;
                smfname = &buf [14];
                uInteractive ("Archiving [%s]", smfname);
            }
        }
        else if (((mode & 0x80) != 0) &&
                 (strncmp (".SUPERMANEND", buf, 12) == 0))
            mode |= 0x04;

        if ((mode & 0x80) != 0)
            fprintf (fo, "%s%s", buf, eolStr);
        if ((mode & 0x04) != 0)
            mode = 0;
    }
    if (mode != 0)
        uError ("Unexpected EOF in superman file\n");
    fclose (fp);
    uFileSet (NULL, NULL);
}

#endif

static void
processFile (int pass, char *filename)
{
    FILE *fp;
    int  i;
    int  isIncludeFile;
    int  isCopyMode;
    int  isCleanLine = 1;
    char *fname;
    char *ext;                          /* Extension component */
    char *base;                         /* Base name component */
    char *bname;                        /* The base name */

    uVerbose (1+pass, "Processing File.\n");

    fname = unifyFilename (filename);
    if (splitFilename (fname, NULL, NULL, &base, &ext) != 0)
        uFatal ("Cannot split filename [%s]\n", fname);
    if (useBase)
        bname = makeFilename (NULL, NULL, base, ext);
    else
        bname = fname;

    isIncludeFile = ((strcmp (ext, "so") == 0) || (strcmp (ext, "tni") == 0));

#if 1
    if ((isIncludeFile == 0) && (strcmp (ext, "sm") == 0))
    {
        doSuperMan (pass, filename);
        pass = -1;
    }
#endif

    if ((pass == 1) && isIncludeFile)
        isCopyMode = 1;
    else if ((pass == 2) && (isIncludeFile == 0))
        isCopyMode = 2;
    else
        isCopyMode = 0;

    if (isCopyMode)
    {
        /* Open the file first - make sure that it exists. */
        if ((fp = fopen (fname, "rb")) == NULL)
            uError ("Cannot open file [%s] for reading.\n", fname);
        else
        {
            int lineNo = 0;

            uFileSet (&lineNo, fname);
            if ((i = fgetc (fp)) == EOF)
                uWarn ("Zero Length File [%s] - skipping\n", fname);
            else
            {
                if (isCopyMode == 1) {
                    uInteractive ("Archiving [%s]", fname);
                    fprintf (fo, ".SUPERMANINC %s%s", bname, eolStr);
                }
                else {
                    uInteractive ("Archiving [%s]", fname);
                    fprintf (fo, ".SUPERMANFILE %s%s", bname, eolStr);
                }

                do {
                    if (i == 0x1A)
                        break;
                    if (i == '\n') {
                        fprintf (fo, eolStr);
                        isCleanLine = 1;
                        lineNo++;
                    }
                    else if (i != '\r') {
                        fputc (i, fo);
                        isCleanLine = 0;
                    }
                } while ((i = fgetc (fp)) != EOF);

                if (isCleanLine == 0)
                    fprintf (fo, eolStr);
                fprintf (fo, ".SUPERMANEND %s%s", bname, eolStr);
            }
            fclose (fp);
            uFileSet (NULL, NULL);
        }
    }
}

static void
usage (void)
{
    fprintf (stdout, "%s : Usage %s [Options] <files>\n", progname, progname);
    fprintf (stdout, "\n");
    fprintf (stdout, "-?/h      : Help - this page\n");
    fprintf (stdout, "-b        : Use basename or insertion\n");
    fprintf (stdout, "-E <file> : Log errors to <file> (append)\n");
    fprintf (stdout, "-e <file> : Log errors to <file> (re-write)\n");
    fprintf (stdout, "-I        : Version Information\n");
    fprintf (stdout, "-q        : Quiet mode\n");
    fprintf (stdout, "-o <file> : Output file name.\n");
    fprintf (stdout, "-v <level>: Verbose level.\n");
    exit (1);
}

int main (int argc, char *argv [])
{
    int i;                              /* Loop ounter */
    int ecount = 0;                     /* Error count */
    int wcount = 0;                     /* Warn count */

    uErrorSet (20, &ecount);            /* Max Num entries + counter */
    uWarnSet (&wcount);                 /* Warning count */
    uUtilitySet (progname);             /* Set name of program */
    uVerboseSet (1);                    /* Verbose setting */

    while (1) {
        i = getopt (argc, argv, "be:E:Ih?qv:o:");
        if (i == EOF)
            break;
        switch (i) {
        case 'b':
            useBase = 1;
            break;
        case 'e':
            uErrorChannelSet (optarg, UERROR_LOG_CREATE);
            break;
        case 'E':
            uErrorChannelSet (optarg, UERROR_LOG_APPEND);
            break;
        case 'h':
        case '?':
            usage();
            break;
        case 'I':
            fprintf (stdout, "%s. Version %s. %s.\n",
                     progname, MODULE_VERSION, __DATE__);
            exit (0);
            break;
        case 'o':
            outFile = optarg;
            break;
        case 'q':
            uInteractiveSet (0);        /* Disable interactive mode */
            break;
        case 'v':
            curVerbose = (optarg[0] - '0');
            break;
        default:
            uError ("Cannot understand option [-%c]\n", i);
            usage();
            break;
        }
    }

    uOpenErrorChannel ();
    argv = getfiles (&argc, argv, optind);

    if (outFile == NULL)
    {
        fo = stdout;
        uInteractiveSet (0);
    }
    else if ((fo = fopen (outFile, "wb")) == NULL)
        uFatal("Cannot open output file [%s]\n", outFile);

    /* Process all of the files */

    uVerbose (0, "Pass 1 - Collecting Symbols\n");
    for (i = 1; i < argc; i++)
        processFile (1, argv[i]);

    uVerbose (0, "Pass 2 - Generating files.\n");
    for (i = 1; i < argc; i++)
        processFile (2, argv[i]);
    if (fo != NULL)
    {
        fprintf (fo, "%s", eofStr);
        fclose (fo);
    }

    uCloseErrorChannel ();
    return (ecount);
}
