/****************************************************************************
 *
 *			Copyright 1997-2004 Jon Green.
 *			    All Rights Reserved
 *
 *
 *  System        :
 *  Module        :
 *  Object Name   : $RCSfile: ntags.c,v $
 *  Revision      : $Revision: 1.2 $
 *  Date          : $Date: 2004-01-06 00:53:51 $
 *  Author        : $Author: jon $
 *  Last Modified : <040103.2004>
 *
 *  Description
 *
 *  Notes
 *
 *  History
 *
 * 1.0.0c 03/01/04 JG Ported to Sun Solaris 9
 * 1.0.0b 03/03/97 JG Ported towin32
 * 1.0.0a 16/08/95 JG Added -o option for output name.
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
#include <ctype.h>
#include <string.h>
#include <signal.h>
#if ((defined _HPUX) || (defined _LINUX) || (defined _SUNOS))
#include <unistd.h>
#else
#include <getopt.h>
#endif

#include <utils.h>

#define MODULE_VERSION  "1.0.0c"
#define MODULE_NAME     "ntags"

typedef struct tagEntry {
    struct tagEntry *prev;
    struct tagEntry *next;
    char            *line;
    char            *name;
    char            *file;
} Tag, *TagP;


static FILE *fi;
static TagP head = NULL;
static char *oname = "tags";            /* Output file name */
static char *progname = MODULE_NAME;    /* Program name */

static TagP
tagAlloc (char *name, char *file, char *line)
{
    TagP  np;

    if ((np = (TagP) malloc (sizeof (Tag))) == NULL)
        uFatal ("No memory\n");
    np->name = bufStr (NULL, name);
    np->line = bufStr (NULL, line);
    np->file = bufStr (NULL, file);
    np->next = NULL;
    np->prev = NULL;
    return (np);
}

static void
tagFree (TagP tp)
{
    if (tp->name)
        free (tp->name);
    if (tp->line)
        free (tp->line);
    if (tp->file)
        free (tp->file);
    free (tp);
}


static TagP
add_tag (TagP np)
{
    TagP p;
    int  i;

/*    printf ("Add tag [%s][%s][%s]\n", np->name, np->file, np->line);*/

    if ((p = head) == NULL) {
        head = np;
        return (NULL);
    }

    for (;;) {
        if ((i = strcmp (np->name, p->name)) == 0) {
            return (p);   /* Duplicate label */
        }
        if (i < 0) {
            if ((np->prev = p->prev) == NULL)
                head = np;
            else
                np->prev->next = np;
            p->prev = np;
            np->next = p;
            return (NULL);
        }
        if (p->next == NULL) {
            p->next = np;
            np->prev = p;
            return (NULL);
        }
        else
            p = p->next;
    }
}

static int
make_tags (void)
{
    TagP tp;
    FILE *fo;
    char *p;

    uVerbose (0, "Creating tag file [%s]\n", oname);

    if ((fo = fopen (oname, "wb")) == NULL)
        uWarn ("Cannot open output file [%s]\n", oname);
    else {

        for (tp = head; tp != NULL; tp = tp->next)
        {
            fprintf (fo, "%s\t%s\t/^", tp->name, tp->file);
            for (p = tp->line; *p != '\0'; p++)
            {
                if (*p == '\\')
                    fputc (*p, fo);
                fputc (*p, fo);
            }
            fprintf (fo, "/\n");
        }
        fclose (fo);
        return (0);
    }
    return (1);
}

static void
xi_insert (char *line, char *file, int lineno)
{
    char *name;
    char *s;
    TagP tp;
    TagP np;
    char c;

/*    printf ("xi_insert [%s]\n", line);*/
    for (s = &line[3]; (((c = *s) != 0) && ((c == ' ') || (c == '\t'))); s++)
        ;
    for (name = NULL; (((c = *s) != 0) && (c != ' ') && (c != '\t')); s++)
        if (isalnum(c) || (c == '-') || (c == '_'))
            name = bufChr (name, c);

    if ((name == NULL) || (name[0] == '\0')) {
        uWarn ("Illegal .X%c line\n", line [2]);
        bufFree (name);
        return;
    }
    np = tagAlloc (name, file, line);
    bufFree (name);

    if ((tp = add_tag (np)) != NULL) {
        uWarn ("Duplicate entry on [%s] from [%s]\n", np->name, tp->file);
        tagFree (np);
    }
}

static void
get_xi (FILE *fp, char *fname)
{
    int c;
    char    *rbuf;
    int fline = 1;


    uFileSet (&fline, fname);           /* Install error reporting filename */
    uInteractive ("Processing %s", fname);

    for (;;)
    {
        if ((c = fgetc (fp)) != '.')
            goto scanline;
        if ((c = fgetc (fp)) != 'X')
            goto scanline;
        if (((c = fgetc (fp)) != 'I') || (c != 'J'))
            goto scanline;
        
        if (c == 'I')
            rbuf = bufStr (NULL, ".XI");
        else
            rbuf = bufStr (NULL, ".XJ");
        while (((c = fgetc(fp)) != EOF) && (c != '\n'))
            rbuf = bufChr (rbuf, (char)(c));

        xi_insert (rbuf, fname, fline);

        rbuf = bufFree (rbuf);
        if (c != '\n')
            goto xi_exit;

scanline:
        while (c != '\n') {
            if (c == EOF)
                goto xi_exit;
            c = fgetc (fp);
        }
        fline++;
    }
xi_exit:
    uFileSet (NULL, NULL);
}

static void
usage (void)
{
    fprintf (stdout, "%s : Usage %s [Options] <files>\n", progname, progname);
    fprintf (stdout, "\n");
    fprintf (stdout, "-I        : Version Information.\n");
    fprintf (stdout, "-o <file> : Output to <file> default is 'tags'.\n");
    exit (1);
}

int
main (int argc, char *argv [])
{
    int ecount = 0;                     /* Error count */
    int wcount = 0;                     /* Warn count */
    int i;
    int c;

    /* Set up the error reporting mechanism */

    uErrorSet (0, &ecount);             /* Max Num entries + counter */
    uWarnSet (&wcount);                 /* Warning count */
    uUtilitySet (progname);             /* Set name of program */
    uInteractiveSet (0);                /* No interaction */

    /* Process each of the files in turn */

    while (1) {
        c = getopt (argc, argv, "o:I");
        if (c == EOF)
            break;
        switch (c) {
        case 'o':
            oname = optarg;
            break;
        case 'I':
            fprintf (stdout, "%s. Version %s. %s.\n",
                     progname, MODULE_VERSION, __DATE__);
            exit (0);
            break;
        case 'h':
        case '?':
            usage();
            break;
        default:
            uError ("Cannot understand option [-%c]\n", c);
            usage();
            break;
        }
    }

    uOpenErrorChannel ();
    argv = getfiles (&argc, argv, optind);
    uVerbose (0, "%s. Version %s. %s\n", progname, MODULE_VERSION, __DATE__);

    for (i = 1; i < argc; i++) {
        if ((fi = fopen (argv[i], "rb")) == NULL) {
            uWarn ("Cannot open file [%s]\n", argv[i]);
            continue;
        }

        get_xi (fi, argv[i]);
        fclose (fi);
    }

    uCloseErrorChannel ();
    return (make_tags ());
}
