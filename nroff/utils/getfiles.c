/****************************************************************************
 *
 *			Copyright 1995-2004 Jon Green.
 *			   All Rights Reserved
 *
 *
 *  System        :
 *  Module        :
 *  Object Name   : $RCSfile: getfiles.c,v $
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
 *  Copyright (c) 1995-2004 Jon Green.
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
#include <string.h>
#include <ctype.h>              /* for tolower() */

#ifndef  _UNIX
#ifdef __GO32__
#include <dir.h>
#endif
#ifdef _DOS
#include <dos.h>
#endif
#endif

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>                    /* Window definitions */
#undef WIN32_LEAN_AND_MEAN
#endif

#include "utils.h"

/* UNIX UNIX UNIX UNIX UNIX UNIX UNIX UNIX UNIX UNIX UNIX UNIX UNIX */

#ifdef _UNIX

char	**getfiles (int *argc, char *argv[], int optind)
{
    if (optind > 0)
    {
        *argc = *argc - optind + 1;
        argv = &argv [optind-1];
#if 0
        {
            int ii;
            
            /*
             * The following hack forms a base name.
             */
            for (ii = 0; ii < *argc; ii++)
                if (strchr (argv[ii], '/') != NULL)
                    argv[ii] = strrchr (argv [ii], '/') + 1;
        }
#endif
    }
    return (argv);
}

#else

/* DOS DOS DOS DOS DOS DOS DOS DOS DOS DOS DOS DOS DOS DOS DOS DOS DOS */

#define dbprintf(x) /* printf x */

typedef struct s_name {
    struct  s_name  *next;
    char            str[1];
} t_name;

static int
readLine (FILE *fp, char *cbuf)
{
    int     c;
    int     len;

    if (fp == NULL)
        return (-1);

    *cbuf = '\0';
    len = 0;
    for (;;)
    {
        c = fgetc (fp);
        if ((c == EOF) || (c == 0x1A))
        {
            *cbuf = '\0';
            return (-1);
        }

        if (c == '\n') {
            *cbuf = '\0';
            return (len);
        }
        else if ((c == '\r') || (c == ' ') || (c == '\t'))
            continue;
        *cbuf++ = c;
        len++;
    }   /* End of 'while' */
}   /* End of 'readLine' */


static int
addNewItem (char *s, t_name **head, int sortFlag)
{
    t_name *new_item;
    t_name *prev_tail;
    t_name *tail;
    int    status = 1;

    dbprintf (("Adding file %s\n", s));
    new_item = (t_name *) malloc (sizeof (t_name) + strlen (s) + 1);
    strcpy (new_item->str, s);
    if ((tail = *head) != NULL) {
        for (prev_tail = NULL;
             ((tail != NULL) &&
              ((sortFlag == 0) || (status = strcmp (s, tail->str)) > 0));
             prev_tail = tail, tail = tail->next)
            /* NULL operation */;
        /* Make sure not the same name !! */
        if ((tail != NULL) && (status == 0)) {
            free (new_item);
            return (0);
        }

        /* Different name - add item */
        if (prev_tail != NULL)
            prev_tail->next = new_item;
        else
            *head = new_item;
    }
    else
        *head = new_item;
    new_item->next = tail;
    return (1);
}

char
**getfiles (int *argc, char *argv[], int optind)
{
    int         count;		/* Count of files */
    int         i;
    t_name      *tail;
    t_name      *head;
    int         file_cnt;
    char        **table;
    const int   attribute = 0;
    char        *fname;
    char        buffer [1024];
    int         status;
    FILE        *fp;
    int         sortFlag = 1;
    int         literalFlag = 0;

     /* This bit of voodo messes around with the arguments and expands
        them into files. Need to return to a regular argc, argv list firts
        hence fiddle parameters. */

    dbprintf(("Option Count = %d\n", *argc));

    if (optind > 0) {
        *argc = *argc - optind + 1;
        argv = &argv [optind-1];
    }

/*---	Do for every file in the argument list */

    count = 0;
    head = NULL;

    for (file_cnt = 1; file_cnt < *argc; file_cnt++)
    {
        fname = argv[file_cnt];
        dbprintf (("File = %s\n", fname));
        if (*fname == '@')
        {
            if ((fp = fopen (&fname[1], "r")) == NULL) {
                fprintf (stderr, "Bad @ reference in file list\n");
                exit (1);
            }

            /* We do not sort a response file */
            sortFlag = 0;
            while ((status = readLine (fp, buffer)) >= 0) {
                if (status > 0)
                    count += addNewItem (buffer, &head, sortFlag);
            }
            fclose (fp);
        }
        else if (strcmp (fname, "SF=1") == 0)
            sortFlag = 1;
        else if (strcmp (fname, "SF=0") == 0)
            sortFlag = 0;
        else if (strcmp (fname, "LF=1") == 0)
            literalFlag = 1;
        else if (strcmp (fname, "LF=0") == 0)
            literalFlag = 0;
        else if (literalFlag == 1)
            count += addNewItem (buffer, &head, sortFlag);
        else
        {
#ifdef _WIN32
            HANDLE          f_block;
            WIN32_FIND_DATA f_data;
            char            tmpname [1024];
            char            *drive;     /* Drive letter string */
            char            *dir;       /* Directory string */

            splitFilename (reslashFilename (tmpname, argv[file_cnt], '/', '\\'),
                           &drive, &dir, NULL, NULL);
            dbprintf (("Argument = %s\n", tmpname));
            f_block = FindFirstFile (tmpname, &f_data);
            if (f_block != INVALID_HANDLE_VALUE)
            {
                do
                {
                    char *p;

                    dbprintf (("FindFirst %s\n", f_data.cFileName));
                    p = makeFilename (drive, dir, f_data.cFileName, NULL);
                    dbprintf (("p = %s. Drive = %s, dir = %s\n", p, drive, dir));
                    count += addNewItem (reslashFilename (tmpname, p, '\\', '/'), &head, sortFlag);
                }
                while (FindNextFile (f_block, &f_data));
                FindClose (f_block);
            }
#else
#ifdef __GO32__
            struct ffblk    f_block;

            status = findfirst (fname, &f_block, attribute);
            while (status == 0)  {
                count += addNewItem (f_block.ff_name, &head, sortFlag);
                status = findnext(&f_block);
            }   /* End of 'while' */
#else
            struct FIND    *f_block;	/* File control block */

            f_block = findfirst (argv[file_cnt], attribute);
            dbprintf (("Fblock = %p\n", f_block));
            while (f_block)  {
                dbprintf (("FindFirst %s\n", f_block->name));
                count += addNewItem (f_block->name, &head, sortFlag);
                f_block = findnext();
            }
#endif
#endif
        }   /* End if 'if' */
    }	/* End of 'for' */

    /* Set up a table */

    table = (char **) malloc (sizeof (char *) * (count+2));
    table [count] = (char *) 0;
    for (tail = head, i = 1; i <= count; i++, tail = tail->next) {
        int j;
        char *p;

        table [i] = tail->str;
        for (j = 0, p = table[i]; *p != 0; p++) {
            if (isupper (*p))
                *p = tolower (*p);
        }
    }

    table[0] = argv[0];
    *argc = count+1;
    return (table);


}	/* End of 'getfiles' */
#endif
