/****************************************************************************
 *
 *			Copyright 1997-2004 Jon Green.
 *			    All Rights Reserved
 *
 *
 *  System        :
 *  Module        :
 *  Object Name   : $RCSfile: nrorder.c,v $
 *  Revision      : $Revision: 1.2 $
 *  Date          : $Date: 2004-01-06 00:53:51 $
 *  Author        : $Author: jon $
 *  Last Modified : <040103.2006>
 *
 *  Description
 *
 *  Notes
 *
 *  History
 *
 * 1.0.0a JG 2004/01/03 Ported to Sun Solaris 9
 * 1.0.0  JG 2000/10/21 Ported to Win32
 *
 ****************************************************************************
 *
 *  Copyright (c) 1997-2004 Jon Green.
 *
 *  All Rights Reserved.
 *
 *  This Document may not, in whole or in part, be copied,
 *  photocopied, reproduced, translated, or reduced to any
 *  electronic medium or machine readable form without prior
 *  written consent from Jon Green.
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

#include "nroff.h"

#define MODULE_VERSION  "1.0.0a"
#define MODULE_NAME     "nrorder"

static int  lineMode = 0;
static int  sectionMode = 0;
static int  caseMode = 0;
static int  fileMode = 0;
static int  zMode = 0;
static char *appendMode = "wb";
static char *progname = MODULE_NAME;    /* Name of the module */
static char *initialName = NULL;
static int  initialFound = 0;
static FILE *fpr = NULL;

static void
nrTH_func (char *id, char *num, char *date, char *company, char *title)
{
    TagP    tp;
    char    *cf;
    char    *cn;
    int     i;

    if (sectionMode == 0)
        num = "0";
    cf = bufStr (NULL, nrfp->fileName);
    for (i = 0; cf[i]; i++)
        if (isupper (cf[i]))
            cf[i] = tolower (cf[i]);
    /*
     * If this is the initial file then force a '\001' into the name
     * to force it to the top.
     */

    cn = bufNStr (NULL, num);
    if ((initialName != NULL) && (id != NULL) &&
        (strcmp (initialName, id) == 0))
    {
        cn = bufStr (cn, "\001");
        initialFound += 1;
    }
    cn = bufStr (cn, id);
    if (caseMode) {
        for (i = 0; cn[i]; i++)
        if (isupper (cn[i]))
            cn[i] = tolower (cn[i]);
    }
    tp = tagAlloc (cn, num, NULL, cf, 0, NULL);
    add_tag (NULL, tp);
    bufFree (cf);
    bufFree (cn);
}

static void
nrStartInc (char *s, int *imode)
{
    *imode = 1;
}

static void
alpha_term (void)
{
    TagP tp;

    for (tp = tagIterateInit (NULL); tp != NULL; tp = tagIterate (tp)) {
        fprintf (fpr, "%s", tp->file);
        if (lineMode)
        {
#ifndef _UNIX
            fprintf (fpr, "\r\n");
#else
            fprintf (fpr, "\n");
#endif
        }
        else
            fprintf (fpr, " ");
    }
    free_tags (NULL);
    if (initialName != NULL)
    {
        if (initialFound == 0)
            uError ("Could not find initial name [%s]\n", initialName);
        else if (initialFound > 1)
            uError ("%d occurences of initial name [%s] found\n",
                     initialFound, initialName);
    }
}

static void
usage (void)
{
    fprintf (stdout, "%s : Usage %s [Options] <files>\n", progname, progname);
    fprintf (stdout, "\n");
    fprintf (stdout, "-1        : 1 entry per line.\n");
    fprintf (stdout, "-?/h      : Help - this page\n");
    fprintf (stdout, "-a        : Append output file.\n");
    fprintf (stdout, "-c        : Make sort case insensitive.\n");
    fprintf (stdout, "-E <file> : Log errors to <file> (append)\n");
    fprintf (stdout, "-e <file> : Log errors to <file> (re-write)\n");
    fprintf (stdout, "-f        : Generate list for each file.\n");
    fprintf (stdout, "-I        : Version information.\n");
    fprintf (stdout, "-i <name> : Initialial page name.\n");
    fprintf (stdout, "-o <file> : Output to <file> default is <files>.out\n");
    fprintf (stdout, "-q        : Quiet mode\n");
    fprintf (stdout, "-s        : Order within sections.\n");
    fprintf (stdout, "-z        : Terminate file with ctrl-z.\n");
    fprintf (stdout, "\n");
    fprintf (stdout, "This utility generates space seprated list of files sorting them into\n");
    fprintf (stdout, "alphabetic order, if requested.  The  primary use of this function is\n");
    fprintf (stdout, "to define the post script printing order for groff(1).\n");
    exit (1);
}

static void
orderInitialise (void)
{
    static  nrFUNCTION funcTab = {NULL};

    /* Headers */
    nrInstall (funcTab, TH_func, nrTH_func);

    /* Files */
    nrInstall (funcTab, startFile_func, NULL);
    nrInstall (funcTab, includeFile_func, nrStartInc);
    nrInstall (funcTab, endFile_func, NULL);

    nrInstallFunctionTable (&funcTab);
}

int main (int argc, char *argv [])
{
    char    *oname = "nrorder.out";
    nrFILE  *nrfp;
    int     ecount = 0;                 /* Error count */
    int     wcount = 0;                 /* Warn count */
    int     c;
    int     i;

    orderInitialise ();

    /* Configure the error reporting */
    uErrorSet (0, &ecount);             /* Max Num entries + counter */
    uWarnSet (&wcount);                 /* Warning count */
    uUtilitySet (progname);             /* Set name of program */

    while (1) {
        c = getopt (argc, argv, "1ace:E:fh?Ii:o:qsz");
        if (c == EOF)
            break;
        switch (c) {
        case '1':
            lineMode = 1;
            break;
        case 'a':
            appendMode = "ab";
            break;
        case 'c':
            caseMode = 1;
            break;
        case 'e':
            uErrorChannelSet (optarg, UERROR_LOG_CREATE);
            break;
        case 'E':
            uErrorChannelSet (optarg, UERROR_LOG_APPEND);
            break;
        case 'f':
            fileMode = 1;
            break;
        case 'h':
        case '?':
            usage();
            break;
        case 'I':
            fprintf (stdout, "%s. Version %s. %s. %s.\n",
                     progname, MODULE_VERSION, __DATE__, nroffVersion);
            exit (0);
            break;
        case 'i':
            initialName = bufFree (initialName);
            initialName = bufNStr (NULL, optarg);
            break;
        case 'o':
            oname = optarg;
            break;
        case 'q':
            uInteractiveSet (0);        /* Disable interactive mode */
            break;
        case 's':
            sectionMode = 1;
            break;
        case 'z':
            zMode = 1;
            break;
        default:
            uError ("Cannot understand option [-%c]\n", c);
            usage();
            break;
        }
    }

    uOpenErrorChannel ();
    argv = getfiles (&argc, argv, optind);

    if (argc > 1) {
        if (oname != NULL) {
            if ((fpr = fopen (oname, appendMode)) == NULL)
                uFatal ("Cannot open file [%s]\n", oname);
            uVerbose (0, "Output in file [%s].\n", oname);
        }
    }
    else
        fpr = NULL;

    /* Process all of the files */
    for (i = 1; i < argc; i++) {
        if ((nrfp = nrFilePush (argv[i])) == NULL)
            exit (1);

        /* Set up the conversion */
        nroff (nrfp, NROFF_MODE_DEFAULT);
        if (fileMode != 0)
            alpha_term ();
    }
    if (fpr != NULL) {
        alpha_term ();
#ifndef _UNIX
        if (zMode) {
            if (lineMode == 0)
                fprintf (fpr,"\r\n");
            fprintf (fpr,"%c", 0x1A);
        }
#else
        if (lineMode == 0)
            fprintf (fpr,"\n");
#endif

        fclose (fpr);
    }

    uCloseErrorChannel ();
    return (ecount);
}
