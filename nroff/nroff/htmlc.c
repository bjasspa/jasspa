/****************************************************************************
 *
 *			Copyright 1995 Jon Green
 *			      All Rights Reserved
 *
 *
 *  System        :
 *  Module        :
 *  Object Name   : $RCSfile: htmlc.c,v $
 *  Revision      : $Revision: 1.1 $
 *  Date          : $Date: 2000-10-21 14:31:26 $
 *  Author        : $Author: jon $
 *  Last Modified : <030597.1458>
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
 *  Copyright (c) 1995 Jon Green
 *
 *  All Rights Reserved.
 *
 *  This Document may not, in whole or in part, be copied,
 *  photocopied, reproduced, translated, or reduced to any
 *  electronic medium or machine readable form without prior
 *  written consent from Jon Green.
 ****************************************************************************/

static const char rcsid[] = "@(#) : $Id: htmlc.c,v 1.1 2000-10-21 14:31:26 jon Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>

#if ((defined _HPUX) || (defined _LINUX))
#include <unistd.h>
#else
#include <getopt.h>
#endif

#include <utils.h>

#define FINDBLK_FAIL    0x00            /* Finding block failed */
#define FINDBLK_FIND    0x01            /* Finding blaock passed */
#define FINDBLK_ALLOC   0x02            /* Finding block allocated */

#define MODE_SUBDIR     0x01            /* Mode field identifies sub-field */
#define MAXPATHNAMELEN  1024            /* Max length of pathname */

/*
 * 1.0.4c - JG 16/11/95 - Ported to UNIX world
 * 1.0.4d - JG 05/01/95 - Added external references
 */

#define MODULE_VERSION  "1.0.4d"
#define MODULE_NAME     "htmlc"

typedef struct sRefBlk {
    struct sRefBlk  *next;              /* Linked list next pointer */
    struct sRefBlk  *prev;              /* Linked list previous pointer */
    struct sRefBlk  *defFile;           /* Defined file. */
    struct sRefBlk  *refFile;           /* 1st Referencing file. */
    int             mode;               /* Sub-directory reference */
    char            name[1];            /* Charater string */
} *RefBlk;

static char *progname = MODULE_NAME;    /* Name of program */
static char *dirName = NULL;
static char *dirHtmlName = NULL;
static char *contentsName = NULL;
static char *outFileName = NULL;
static char *outPath = NULL;
static char ctrlZStr[2] = {0x1a,'\0'};  /* Add Ctrl-Z */
static char *htmlEofStr;                /* End of file string */
static char *htmlEolStr;                /* End ofline. */

static RefBlk fileBlks = NULL;
static RefBlk nameBlks = NULL;
static RefBlk  curFileBlk;              /* Current File Block */

static FILE *fo = NULL;                 /* Output File Pointer */
static int  curAutoFile = 0;            /* Automatic file */
static int  curPass;
static char *linkPath=NULL;             /* <<P< Link path to pre-pred */
static int  subDirMode = 0;             /* Sub-directory mode */
static int  activeMode = 0;             /* Currently Active Mode */

static char *
constructPathname (RefBlk newblk)
{
    static char pathbuffer [MAXPATHNAMELEN];

    if (subDirMode != 0)
    {
        if (((newblk->defFile->mode ^ curFileBlk->mode) & MODE_SUBDIR) != 0)
        {
            if (((newblk->defFile->mode & MODE_SUBDIR) != 0) &&
                ((curFileBlk->mode & MODE_SUBDIR) == 0))
            {
                strcpy (pathbuffer, "../");
            }
            else
            {
                strcpy (pathbuffer, dirName);
                strcat (pathbuffer, "/");
            }
            strcat (pathbuffer, newblk->defFile->name);
            return (pathbuffer);
        }
    }
    return (newblk->defFile->name);
}

static RefBlk
allocBlk (char *name)
{
    RefBlk  bp;
    int     slen;

    uVerbose (6, "allocBlk [%s]\n", name);

    slen = strlen (name);
    if ((bp = (RefBlk) malloc (sizeof (struct sRefBlk) + slen)) == NULL)
        uFatal ("No memory sucker !!\n");

    strcpy (bp->name, name);
    bp->mode = 0;

    /*
     * Initialise the rest of the structure
     */

    bp->next = NULL;
    bp->prev = NULL;
    bp->defFile = NULL;
    bp->refFile = NULL;
    return (bp);
}

int
findBlk (RefBlk *head, char *name, int mode, RefBlk *found)
{
    RefBlk p;
    RefBlk np;
    int    i;

    uVerbose (6, "findBlk [%s]\n", name);

    *found = NULL;
    /* List is NULL ?? - Add if necessary. */
    if ((p = *head) == NULL) {
        uVerbose (6, "findBlk - At head\n");
        if (mode == FINDBLK_ALLOC) {
            np = allocBlk (name);       /* Make the new block */
            *found = np;                /* Return to caller. */
            *head = np;                 /* Change the head itself */
            return (FINDBLK_ALLOC);     /* Return allocated state */
        }
        return (FINDBLK_FAIL);          /* Not found - so fail. */
    }

    for (;;) {
        if ((i = strcmp (name, p->name)) == 0)
        {                               /* Found Label */
            uVerbose (6, "findBlk - Found Match [%s]\n", p->name);
            *found = p;                 /* Return pointer to block */
            return (FINDBLK_FIND);      /* Return found status */
        }
        /* Less than current string - hence is not in list - add or
           fail depending upon the current search criteria */
        if (i < 0) {
            uVerbose (6, "findBlk - Less Than [%s]\n", p->name);
            if (mode != FINDBLK_ALLOC)
                return (FINDBLK_FAIL);
            np = allocBlk (name);
            if ((np->prev = p->prev) == NULL)
                *head = np;
            else
                np->prev->next = np;
            p->prev = np;
            np->next = p;
            *found = np;
            return (FINDBLK_ALLOC);
        }

        /* If we have reached the end of the list then allocate or fail.
           Otherwise just mode onto the next node.  */

        if (p->next == NULL) {
            uVerbose (6, "findBlk - Greater Than [%s]\n", p->name);
            if (mode != FINDBLK_ALLOC)
                return (FINDBLK_FAIL);
            np = allocBlk (name);
            p->next = np;
            np->prev = p;
            *found = np;
            return (FINDBLK_ALLOC);
        }
        else
            p = p->next;
    }
}

/*
 * Process the file markers in the file as follows:-
 *
 * PASS1:
 *      Cache the filename in the table - check for duplicates.
 * PASS2:
 *      Open and create the named file in the file system, close
 *      previous file.
 */

static char *
handleFmarker (char *filename)
{
    RefBlk  newblk;
    char    *fname;

    activeMode = 0;                     /* Reset the active mode */
    if ((subDirMode & MODE_SUBDIR) &&
        (strcmp (dirHtmlName, filename) == 0))
        activeMode |= MODE_SUBDIR;      /* Root file */

    uVerbose (3, "Processing File [%s] in pass %d.\n", filename, curPass);
    uInteractive ("Pass %d: %s", curPass, filename);

    if (curPass == 1) {
        /* PASS 1.
           Load the table with filenames. */

        switch (findBlk (&fileBlks, filename, FINDBLK_ALLOC, &newblk)) {
        case FINDBLK_ALLOC:
            curFileBlk = newblk;
            if ((subDirMode & MODE_SUBDIR) != 0)
                curFileBlk->mode = activeMode;
            break;
        case FINDBLK_FIND:
            uWarn ("Duplicate filename encountered [%s]\n", filename);
            break;
        default:
            uWarn ("Bad status [%s]\n", filename);
            break;
        }
    }
    else {
        /* PASS 2.
           Expand and write files. */

        switch (findBlk (&fileBlks, filename, FINDBLK_FIND, &newblk)) {
        case FINDBLK_FIND:
            curFileBlk = newblk;
            if (outFileName == NULL)
            {
                /*
                 * Append output path if requested.
                 */

                fname = NULL;
                if (outPath != NULL)
                {
                    fname = bufStr (NULL, outPath);
                    fname = bufChr (fname, '/');
                }

                /*
                 * Append sub-directory if not at top level.
                 */

                if (((subDirMode & MODE_SUBDIR) != 0) &&
                    ((activeMode & MODE_SUBDIR) == 0))
                {
                    fname = bufStr (fname, dirName);
                    fname = bufChr (fname, '/');
                }

                fname = bufStr (fname, curFileBlk->name);

                if (fo != NULL) {
                    if (htmlEofStr != NULL)
                        fprintf (fo, htmlEofStr);
                    fclose (fo);
                }
                uVerbose (3, "Opening Output file [%s].\n", fname);
                if ((fo = fopen (fname, "wb")) == NULL)
                    uFatal ("Cannot open file [%s] for output\n",
                                fname);
                bufFree (fname);
            }
            else {
                static char buffer [100];

                if (((subDirMode & MODE_SUBDIR) != 0) &&
                    ((activeMode & MODE_SUBDIR) == 0))
                    sprintf (buffer,
                             "<<FILE> %s/%s >%s",
                             dirName,
                             curFileBlk->name,
                             htmlEolStr);
                else
                    sprintf (buffer,
                             "<<FILE> %s >%s",
                             curFileBlk->name,
                             htmlEolStr);
                return (buffer);
            }
            break;
        default:
            uFatal ("File system changed between Pass 1 and Pass 2 for %s\n",
                    filename);
        }
    }
    return (NULL);                      /* Remove the data from file */
}

/*
 * Process the symbolic definition markers in the file as follows:-
 *
 * PASS1:
 *      Cache the symbolic name as a definition - check for duplicates.
 *      Cache with the current filename.
 * PASS2:
 *      Remove the entries from the file, not required.
 */

static char *
handleDmarker (char *name)
{
    RefBlk  newblk;

    uVerbose (3, "Processing Definition [%s] in pass %d.\n", name, curPass);

    if (curPass == 1) {
        /* PASS 1.
           Load the table with filenames. */

        switch (findBlk (&nameBlks, name, FINDBLK_ALLOC, &newblk)) {
        case FINDBLK_ALLOC:
            newblk->defFile = curFileBlk;
            break;
        case FINDBLK_FIND:
            if (newblk->defFile == NULL)
                newblk->defFile = curFileBlk;
            else
                uWarn ("Duplicate Defn [%s] : First defined in [%s]\n",
                       name, newblk->defFile->name);
            break;
        default:
            uWarn ("Bad status [%s]\n", name);
            break;
        }
    }

    /* PASS 2.
       Do - nothing. Data will be removed */

    return (NULL);                      /* Remove the data from the file */
}

/*
 * Process the symbolic reference markers in the file as follows:-
 *
 * PASS1:
 *      Cache the symbolic name as a reference.
 *      Cache with the current filename.
 * PASS2:
 *      Exchange the entries in the file for the definition file name.
 */

static char *
handleRmarker (char *name)
{
    RefBlk  newblk;

    uVerbose (3, "Processing Reference [%s] in pass %d.\n", name, curPass);

    /* If the name of the string is 'contents' then apply an
       new name if provided. */
    if (contentsName != NULL)
        if (strcmp ("contents", name) == 0)
            name = contentsName;

    if (curPass == 1) {
        /* PASS 1.
           Load the table with filenames. */

        switch (findBlk (&nameBlks, name, FINDBLK_ALLOC, &newblk)) {
        case FINDBLK_ALLOC:
            newblk->refFile = curFileBlk;
            break;
        case FINDBLK_FIND:
            if (newblk->refFile == NULL)
                newblk->refFile = curFileBlk;
            break;
        default:
            uWarn ("Bad status [%s]\n", name);
            break;
        }
        return (NULL);
    }

    /* PASS 2.
       Determine the defining block and swap out the data */

    if (findBlk (&nameBlks, name, FINDBLK_FIND, &newblk) == FINDBLK_FIND) {
        if (newblk->defFile != NULL) {
            uVerbose(4, "Replacing reference [%s] with [%s].\n",
                     newblk->name, newblk->defFile->name);
            return (constructPathname (newblk));
        } else if (findBlk (&nameBlks, "undef", FINDBLK_FIND,
                            &newblk) == FINDBLK_FIND)
        {
            return (constructPathname (newblk));
        }
        else
            return ("undef.html");
    }
    uFatal ("File system changed between Pass 1 and Pass 2\n");
    return (NULL);
}

static char *
handlePmarker (char *rbuf)
{
    static char pathname [MAXPATHNAMELEN];
    /*
     * Ignore the position marker in pass 1.
     */

    if (curPass == 2)
    {
        char *p = NULL;                 /* Path pointer */
        /*
         * PASS 2.
         * Replace the position marker with the current path position.
         */
        if ((subDirMode & MODE_SUBDIR) != 0)
        {
            /*
             * If we are not at the root then return the path back to the
             * root directory.
             */
            if ((activeMode & MODE_SUBDIR) == 0)
                p = "../";         /* No - return path to root */
        }

        if (linkPath == NULL)
            return (p);                 /* Return path */
        if (p == NULL)                  /* Any offset ?? */
            return (linkPath);          /* No - return link path */
        /*
         * Construct relative path with link name.
         */
        strcpy (pathname, p);           /* Add relative offset */
        strcat (pathname, linkPath);    /* Add the link path */
        return (pathname);              /* Return the constructed path */
    }
    return (NULL);
}

static char *
handleAmarker (char *rbuf)
{
    char sprintBuf [20];
    char *opdata;
    char *tbuf;

    /*
     * Create a new filename for the item.
     */
    sprintf (sprintBuf, "auto%04d", curAutoFile++);
    tbuf = bufNStr (NULL, sprintBuf);
    tbuf = bufStr (tbuf, rbuf);


    /*
     * Handle as a new file.
     */
    opdata = handleFmarker (tbuf);
    tbuf = bufFree (tbuf);
    return (opdata);
}

static
void processFile (char *fname)
{
    FILE *fp;
    int  c;
    int  c1;
    char *rbuf = NULL;
    char *opdata;
    int  lineNo = 1;

    if ((fp = fopen (fname, "rb")) == NULL) {
        uError ("Cannot open file for reading.\n");
        return;
    }

    uVerbose (1, "%s", fname);
    uVerbose (1 + curPass, "Processing File.\n");
    uFileSet (&lineNo, fname);

    uInteractive ("%s [%d]", fname, curPass);

    for (;;) {
        for (;;) {
            c = fgetc (fp);
            if (c == '<')
                break;
        scanline:
            if (c == EOF) {
                uFileSet (NULL, NULL);
                fclose (fp);
                if (outFileName == NULL)
                {
                    if (fo != NULL) {
                        if (htmlEofStr != NULL)
                            fprintf (fo, htmlEofStr);
                        fclose (fo);
                        fo = NULL;
                    }
                }
                goto tidyExit;
                return;
            }
            if (fo == NULL)
                continue;
            if (c == '\r')
                continue;
            if (c == '\n') {
                lineNo++;
                fprintf (fo, htmlEolStr);
                continue;
            }
            fputc (c, fo);
        }

        if ((c = fgetc (fp)) != '<') {
            if (fo != NULL)
                fprintf (fo, "<");
            goto scanline;
        }

        if (((c = fgetc (fp)) != 'F') &&
            (c != 'R') &&
            (c != 'A') &&
            (c != 'P') &&
            (c != 'D')) {
            if (fo != NULL)
                fprintf (fo, "<<");
            goto scanline;
        }

        rbuf = NULL;
        while ((c1 = fgetc (fp)) != '<') {
            if (c1 == EOF)
                uFatal ("Premature EOF.\n");
            rbuf = bufChr (rbuf, c1);
        }

        switch (c) {
        case 'F':
            opdata = handleFmarker (rbuf);
            break;
        case 'A':
            opdata = handleAmarker (rbuf);
            break;
        case 'D':
            opdata = handleDmarker (rbuf);
            break;
        case 'R':
            opdata = handleRmarker (rbuf);
            break;
        case 'P':
            opdata = handlePmarker (rbuf);
            break;
        default:
            opdata = rbuf;
        }
        if ((fo != NULL) && (opdata != NULL)) {
            uVerbose (6, "opdata = %s\n", opdata);
            fprintf (fo, opdata);
        }
        else if (fo == NULL)
            uVerbose (6, "FO = NULL\n");
        else
            uVerbose (6, "OPDATA = NULL\n");
        bufFree (rbuf);
    }
tidyExit:
    return;
}


static
void resolveReferences (void)
{
    RefBlk  p;
    RefBlk  undef;
    int     i;

    uVerbose (1, "Resolving References\n");
    for (i = 0,p = nameBlks; p != NULL; p = p->next, i++) {
#if 0
        uVerbose (1, "Resolving [%s] File [%s] Mode [%d].\n",
                  p->name,
                  p->refFile,
                  p->mode);
#endif
        if ((p->defFile == NULL) && (p->refFile != NULL))
        {
/*            curFileName = p->refFile->name;*/
            uWarn ("Symbol [%s] referenced and not defined.\n", p->name);
        }
        else if ((p->refFile == NULL) && (p->defFile != NULL)) {
/*            curFileName = p->defFile->name;*/
            uVerbose (1, "Symbol [%s] defined and not referened\n", p->name);
        }
        if (p->defFile == NULL) {
            if (findBlk (&nameBlks, "undef", FINDBLK_FIND, &undef) ==
                FINDBLK_FIND)
                p->defFile = undef->defFile;
            uVerbose (3, "Assigning default to symbol [%s].\n", p->name);
        }
    }
    uVerbose (0, "Total of [%d] References\n", i);
}

static void
usage (void)
{
    fprintf (stdout, "%s : Usage %s [Options] <files>\n", progname, progname);
    fprintf (stdout, "\n");
    fprintf (stdout, "-?/h      : Help - this page\n");
    fprintf (stdout, "-c <name> : Force contents page to be <name>.\n");
    fprintf (stdout, "-C <name> : Named contents page + path (e.g. -s -c).\n");
    fprintf (stdout, "-E <file> : Log errors to <file> (append)\n");
    fprintf (stdout, "-e <file> : Log errors to <file> (re-write)\n");
    fprintf (stdout, "-I        : Version Information\n");
    fprintf (stdout, "-l <path> : Linking path to extenal modules.\n");
    fprintf (stdout, "-o <file> : Create super html output file <file>.\n");
    fprintf (stdout, "-p <path> : Output path.\n");
    fprintf (stdout, "-q        : Quiet mode\n");
    fprintf (stdout, "-s <file> : Sub-Directory File Name Mode\n");
    fprintf (stdout, "-S <name> : Named contents page + path (e.g. -s -c).\n");
    fprintf (stdout, "-v <level>: Verbose level.\n");
    exit (1);
}

int main (int argc, char *argv [])
{
    int c;
    int i;
    int ecount = 0;                     /* Error count */
    int wcount = 0;                     /* Warn count */

    uErrorSet (20, &ecount);            /* Max Num entries + counter */
    uWarnSet (&wcount);                 /* Warning count */
    uUtilitySet (progname);             /* Set name of program */
    uVerboseSet (1);                    /* Verbose setting */

    while (1) {
        c = getopt (argc, argv, "s:v:p:e:E:c:C:S:o:l:i");
        if (c == EOF)
            break;
        switch (c) {
        case 'e':
            uErrorChannelSet (optarg, UERROR_LOG_CREATE);
            break;
        case 'E':
            uErrorChannelSet (optarg, UERROR_LOG_APPEND);
            break;
        case 'S':
        case 'C':
            contentsName = optarg;
            /* Allow drop through */
        case 's':
            subDirMode = MODE_SUBDIR;
            dirName = optarg;
            dirHtmlName = bufNStr (NULL, optarg);
            dirHtmlName = bufNStr (dirHtmlName, ".html");
            break;
        case 'v':
            uVerboseSet ((int)(optarg[0] - '0'));
            break;
        case 'l' :
            linkPath = optarg;
            break;
        case 'p':
            outPath = optarg;
            break;
        case 'c':
            contentsName = optarg;
            break;
        case 'i':
            uInteractiveSet (0);
            break;
        case 'o':
            outFileName = optarg;
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
        case 'q':
            uInteractiveSet (0);        /* Disable interactive mode */
            break;
        default:
            uError ("Cannot understand option [-%c]\n", c);
            usage();
            break;
        }
    }

    uOpenErrorChannel ();
    uVerbose (0, "%s. Version %s. %s\n", progname, MODULE_VERSION, __DATE__);

    argv = getfiles (&argc, argv, optind);

    /* Process all of the files */

    uVerbose (0, "Pass 1 - Collecting Symbols\n");
    curPass = 1;

    for (i = 1; i < argc; i++)
        processFile (argv[i]);

    resolveReferences();

    uVerbose (0, "Pass 2 - Generating files.\n");

    if (outFileName != NULL) {
        if ((fo = fopen (outFileName, "wb")) == NULL) {
            uError ("Cannot open output file [%s].\n", outFileName);
            exit (1);
        }
#ifndef _UNIX
        htmlEolStr = "\r\n";
        htmlEofStr = ctrlZStr;
#else
        htmlEolStr = "\n";
        htmlEofStr = "";
#endif

        if ((subDirMode & MODE_SUBDIR) != 0)
            fprintf (fo, "<<SUBDIRECTORY> %s >%s", dirName, htmlEolStr);
    }
    else {
        htmlEolStr = "\n";
        htmlEofStr = NULL;
    }

    curPass = 2;
    curAutoFile = 0;                /* Automatic file */

    for (i = 1; i < argc; i++)
        processFile (argv[i]);

    if (fo != NULL) {
        if (htmlEofStr != NULL)
            fprintf (fo, htmlEofStr);
        fclose (fo);
    }
    uCloseErrorChannel ();
    return (ecount);
}
