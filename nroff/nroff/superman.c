/****************************************************************************
 *
 *			Copyright 1996-2004 Jon Green.
 *                         All Rights Reserved
 *
 *
 *  System        :
 *  Module        :
 *  Object Name   : $RCSfile: superman.c,v $
 *  Revision      : $Revision: 1.4 $
 *  Date          : $Date: 2004-02-07 19:29:49 $
 *  Author        : $Author: jon $
 *  Last Modified : <260228.1021>
 *
 *  Description
 *
 *  Notes
 *
 *  History
 *
 * 1.0.4j - JG 28/02/26 - Added -i filelist option. Fixed readline overflow.
 * 1.0.4i - JG 24/02/26 - Added -v verbose option
 * 1.0.4h - JG 07/02/04 - Ported to HP-UX 11.0.
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

#define MODULE_VERSION  "1.0.4j"
#define MODULE_NAME     "superman"
#define MAX_LINE_LEN    4096            /* Length of longest line */

static char *progname = MODULE_NAME;    /* Name of the module */
static int  useBase = 0;                /* Use the base name */

#ifdef _UNIX
static char *eolStr = "\n";
static char eofStr [2] = {0, 0};
#else
static char *eolStr = "\r\n";
static char eofStr [2] = {0x1A, 0};
#endif

static int
readLine (FILE *fp, char *cbuf, int clen)
{
    int     c;
    int     len;

    if (fp == NULL)
        return (-1);

    cbuf [0] = '\0';
    len = 0;
    while (len < clen)
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

    /* Check that we have not overflowed */
    if (len >= clen)
    {
        uFatal ("readline overflow, line longer than %d characters\n", clen);
    }
    return (len);
}   /* End of 'readLine' */

static void
doSuperMan (FILE *fo, int pass, char *fname)
{
    FILE    *fp;
    char    buf [MAX_LINE_LEN];
    int     mode = 0;
    char    *smfname;
    int     lineNo;

    if ((fp = fopen (fname, "rb")) == NULL) {
        uError ("Cannot open file [%s] for reading.\n", fname);
        return;
    }

    uFileSet (&lineNo, fname);
    while (readLine (fp, buf, sizeof (buf)) >= 0)
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

static void
processFile (FILE* fo, int pass, char *filename)
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

    uVerbose (1, "Processing File [%d]: \"%s\"\n", pass, filename);

    fname = unifyFilename (filename);
    if (splitFilename (fname, NULL, NULL, &base, &ext) != 0)
        uFatal ("Cannot split filename [%s]\n", fname);
    if (useBase)
        bname = makeFilename (NULL, NULL, base, ext);
    else
        bname = fname;

    isIncludeFile = ((strcmp (ext, "so") == 0) || (strcmp (ext, "tni") == 0));
    if ((isIncludeFile == 0) && (strcmp (ext, "sm") == 0))
    {
        doSuperMan (fo, pass, filename);
        pass = -1;
    }
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
                        fprintf (fo, "%s", eolStr);
                        isCleanLine = 1;
                        lineNo++;
                    }
                    else if (i != '\r') {
                        fputc (i, fo);
                        isCleanLine = 0;
                    }
                } while ((i = fgetc (fp)) != EOF);

                if (isCleanLine == 0)
                    fprintf (fo, "%s", eolStr);
                fprintf (fo, ".SUPERMANEND %s%s", bname, eolStr);
            }
            fclose (fp);
            uFileSet (NULL, NULL);
        }
    }
}

/*
 * Open any filelist file, process each line by reading the file name and
 * processing the file.
 * 
 * fo - The output file pointer
 * pass - The process pass 1 or 2
 * filename - The name of the filelist file.
 */
static void
processFileList (FILE* fo, int pass, char *filename)
{
    FILE *ifp;                          /* Input file pointer */
    char buf [MAX_LINE_LEN];            /* File name buffer */
    int len;                            /* Length of line */

    /* See if there is anything to do */
    if (filename == NULL)
        return;                         /* Quit no filelist. */
    uVerbose (1, "Processing filelist \"%s\"\n", filename);
    
    /* Attempt to open the file */
    if ((ifp = fopen (filename, "r")) == NULL)
    {
        uError ("Cannot open file list \"%s\"\n", filename);
        return;                         /* Quit, cannot open */
    }

    /* Read each line from the file for the file name */
    while ((len = readLine (ifp, buf, sizeof (buf))) >= 0)
    {
        /* Skip any blank lines */
        if (len != 0)
        {
            /* Process the file */
            processFile (fo, pass, buf);
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
    fprintf (stdout, "-i <flst> : Input file list, 1 file per line\n");
    fprintf (stdout, "-I        : Version Information\n");
    fprintf (stdout, "-q        : Quiet mode\n");
    fprintf (stdout, "-o <file> : Output file name.\n");
    fprintf (stdout, "-v <level>: Verbose level.\n");
    exit (1);
}

int main (int argc, char *argv [])
{
    FILE *fo = NULL;                    /* Output File Pointer */
    int i;                              /* Loop ounter */
    int ecount = 0;                     /* Error count */
    int wcount = 0;                     /* Warn count */
    char *filelist = NULL;              /* File list */
    char *outFile = NULL;               /* The output file */

    uErrorSet (20, &ecount);            /* Max Num entries + counter */
    uWarnSet (&wcount);                 /* Warning count */
    uUtilitySet (progname);             /* Set name of program */
    uVerboseSet (1);                    /* Verbose setting */

    while (1) {
        i = getopt (argc, argv, "be:E:i:Ih?qv:o:");
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
        case 'i':
            if (filelist != NULL)
                uWarn ("Discarding previous file list %s\n", filelist);
            filelist = optarg;
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
            /* Verbose setting */
            uVerboseSet (optarg[0] - '0');
            break;
        default:
            uError ("Cannot understand option [-%c]\n", i);
            usage();
            break;
        }
    }

    uOpenErrorChannel ();
    argv = getfiles (&argc, argv, optind);

    /* Open the output file */
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
        processFile (fo, 1, argv[i]);
    processFileList (fo, 1, filelist);

    uVerbose (0, "Pass 2 - Generating files.\n");
    for (i = 1; i < argc; i++)
        processFile (fo, 2, argv[i]);
    processFileList (fo, 2, filelist);
    
    /* Close the output file */
    if (fo != NULL)
    {
        fprintf (fo, "%s", eofStr);
        if (fo != stdout)
            fclose (fo);
    }

    uCloseErrorChannel ();
    return (ecount);
}
