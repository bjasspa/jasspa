/****************************************************************************
 *
 *			   Copyright 1995-2004 Jon Green.
 *			      All Rights Reserved
 *
 *
 *  System        :
 *  Module        :
 *  Object Name   : $RCSfile: difftags.c,v $
 *  Revision      : $Revision: 1.2 $
 *  Date          : $Date: 2004-01-06 00:53:50 $
 *  Author        : $Author: jon $
 *  Last Modified : <040104.0024>
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
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <utils.h>

FILE *f1;                               /* First input file */
FILE *f2;                               /* Second input file */
FILE *fo;                               /* Output file */

static char b1 [1024];
static char b2 [1024];
static int  lineNo1 = 0;
static int  lineNo2 = 0;
static int  undefinedCount = 0;
static int  matchCount = 0;
static int  unknownCount = 0;

void
printLine (char *cause, char *s1, int l1, char *s2, int l2)
{
    int i;
    char buffer [1024];

    fprintf (fo, "%s  ", cause);
    if (s1 != NULL) {
        sprintf (buffer, "%s [%d]", s1, l1);
        fprintf (fo, "%s", buffer);
        i = strlen (buffer);
    }
    else
    {
        fprintf (fo, "---");
        i = 3;
    }

    while (i < 55) {
        fprintf (fo, " ");
        i++;
    }
    if (s2 != NULL)
        fprintf (fo, "%s [%d]\n", s2, l2);
    else
        fprintf (fo, "---\n");
}

void
getLine (FILE **fp, char *buffer, int *lineNo)
{
    int  status;
    char junk [1024];

    if (*fp == NULL) {
        *buffer = '\0';
        return;
    }
    status = fscanf (*fp, "%[^\t]\t%[^\n]\n", buffer, junk);
    if (/*(status == EOF) ||*/ (status != 2)) {
        fclose (*fp);
        *fp = NULL;
        *buffer = '\0';
    }
    *lineNo += 1;
    return;
}

int
main (int argc, char *argv [])
{
    int status;

    if ((argc < 3) || (argc > 4)) {
        fprintf (stdout, "Usage: difftags <cfile1> <docfile2> [<fileout>]\n");
        exit (2);
    }

    /* Process each of the files in turn */

    if ((f1 = fopen (argv [1], "r")) == NULL) {
        fprintf (stderr, "Cannot open file [%s]\n", argv[1]);
        exit (2);
    }

    if ((f2 = fopen (argv [2], "r")) == NULL) {
        fprintf (stderr, "Cannot open file [%s]\n", argv[2]);
        exit (2);
    }

    if (argc == 4) {
        if ((fo = fopen (argv [3], "w")) == NULL) {
            fprintf (stderr, "Cannot open output file [%s]\n", argv[3]);
        exit (2);
        }
    }
    else
        fo = stdout;

    getLine (&f1, b1, &lineNo1);
    getLine (&f2, b2, &lineNo2);

    printLine ("Files:    ", argv [1], 0, argv [2], 0);
    fprintf (fo, "\n");
    while ((f1 != NULL) || (f2 != NULL))
    {
        /* Evaluate the matched status */

        if (b1[0] == '\0')
            status = 1;
        else if (b2[0] == '\0')
            status = -1;
        else
            status = strcmp (b1, b2);

        /* Given the status print the result. */

        if (status == 0) {
            printLine ("-- ok     ", b1, lineNo1, b2, lineNo2);
            getLine (&f1, b1, &lineNo1);
            getLine (&f2, b2, &lineNo2);
            matchCount++;
        }
        else if (status < 0) {
            printLine ("** Undef  ", b1, lineNo1, NULL, 0);
            getLine (&f1, b1, &lineNo1);
            undefinedCount++;
        }
        else {
            printLine ("** Unknown", NULL, 0, b2, lineNo2);
            getLine (&f2, b2, &lineNo2);
            unknownCount++;
        }
    }
    fprintf (fo, "\n");
    printLine ("Total:    ", argv [1], lineNo1, argv [2], lineNo2);
    fprintf (fo, "\n");
    fprintf (fo, "Matched = %d. Undefined = %d. Unknown = %d\n",
             matchCount, undefinedCount, unknownCount);
    fclose (fo);
    return (0);
}
