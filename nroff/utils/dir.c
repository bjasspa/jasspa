/****************************************************************************
 *
 *			Copyright 1996-2004 Jon Green.
 *                         All Rights Reserved
 *
 *
 *  System        : Utils
 *  Module        : File utilities
 *  Object Name   : $RCSfile: dir.c,v $
 *  Revision      : $Revision: 1.3 $
 *  Date          : $Date: 2004-02-07 19:29:49 $
 *  Author        : $Author: jon $
 *  Last Modified : <040207.1923>
 *
 *  Description     These utilities manipulate filenames.
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

static const char rcsid[] = "@(#) : $Id: dir.c,v 1.3 2004-02-07 19:29:49 jon Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "utils.h"

#define DIR_CHAR     '/'                /* Directory separator */
#define EXT_CHAR     '.'                /* Extension separator */
#define DRV_CHAR     ':'                /* Drive separator */

/*
 * Convert the directory separator characters in the filename
 */
char *
reslashFilename (char *buf, char *name, char from, char to)
{
    char *p;

    for (p = buf; (*p = *name++) != '\0'; p++)
    {
        if (*p == from)
            *p = to;
    }
    return (buf);
}

/*
 * makeFilename
 * Constructs a new filename from components and returns it to the caller.
 */

char *
makeFilename (char *drive, char *path, char *base, char *ext)
{
    static char fname [1024];
    char        cbuf [2] = "?";

    /* Add the drive */
    if ((drive != NULL) && (*drive != '\0'))
        sprintf (fname, "%s%c", drive, DRV_CHAR);
    else
        fname [0] = '\0';

    /* Add the pathname */
    if ((path != NULL) && (*path != '\0'))
    {
        strcat (fname, path);
        cbuf [0] = DIR_CHAR;
        strcat (fname, cbuf);
    }

    /* Add the basename */
    if (base != NULL)
        strcat (fname, base);

    /* Add the extension */
    if ((ext != NULL) && (*ext != '\0'))
    {
        cbuf [0] = EXT_CHAR;
        strcat (fname, cbuf);
        strcat (fname, ext);
    }

#if 0
    printf ("makeFilename [%s][%s][%s][%s] => [%s]\n",
            drive,path,base,ext,fname);
#endif
    if (*fname == '\0')
        return (NULL);
    return (fname);
}

/*
 * splitFilename
 * This function attempts to split a filename into it's component parts,
 * returing the components to the caller.
 */

int
splitFilename (char *fname, char **adrive, char **apath, char **abase, char **aext)
{
    static char drive[2];
    static char path [1024];
    static char base [256];
    static char ext [256];
    char        tname [1024];
    char *p;
    char *q;
    char *f;

    /*
     * Make sure the name is legal.
     */

    if (((f = fname) == NULL) || (*f == '\0'))
        return (-1);

    f = reslashFilename (tname, f, '\\', '/');
    /*
     * Get the drive letter out.
     */

    q = drive;
    if ((isalpha ((int)(f[0]))) && (f[1] == DRV_CHAR))
    {
        *q++ = *f;
        f = &f[2];
    }
    *q = '\0';                          /* Terminate string */

    /*
     * Get the pathname out by finding the last occurence of the drive
     * character.
     */

    q = path;                           /* Point to path component */
    if (((p = strrchr (f, DIR_CHAR)) != NULL) &&
        (p != f))
    {
        while (f != p)                  /* Copy pathname */
            *q++ = *f++;
        f++;                            /* Loose directory character */
    }
    *q = '\0';                          /* Terminate string */

    /*
     * Get the extension
     */

    q = base;
    if ((p = strrchr (f, EXT_CHAR)) != NULL)
    {
        strcpy (ext, &p[1]);
        while (f != p)                  /* Copy the basename */
            *q++ = *f++;
        *q = '\0';
    }
    else
    {
        ext[0] = '\0';
        strcpy (base, f);
    }

    /*
     * Return the arguments to the caller.
     */

    if (adrive != NULL)
        *adrive = drive;
    if (apath != NULL)
        *apath = path;
    if (abase != NULL)
        *abase = base;
    if (aext != NULL)
        *aext = ext;

#if 0
    printf ("\n");
    printf ("In:  [%s]\n", fname);
    printf ("Out: [%s][%s][%s][%s]\n", drive, path, base, ext);
    makeFilename (drive, path, base, ext);
#endif

    return (0);
}

/*
 * unifyFilename
 * Make unitform filename - assume that all upper case characters are converted
 * to lower case. Convert all backslash directory characters to forward slash.
 */

char *
unifyFilename (char *afname)
{
    static char fname [1024];
    char *p, *q;
    int c;

    if (afname == NULL)
        return NULL;

    p = fname;
    q = afname;

    /* Convert DOS directory characters and convert from upper to lower */
    while ((c = *q++) != '\0')
    {
        if (isupper (c))
            c = tolower (c);
        if (c == '\\')
            c = DIR_CHAR;
        *p++ = c;
    }
    *p = c;

    return (fname);
}

#if 0
int main (int argc, char *argv[])
{
    splitFilename ("a:foo", NULL, NULL, NULL, NULL);
    splitFilename ("a:/foo.c", NULL, NULL, NULL, NULL);
    splitFilename ("a:foo.tar.z", NULL, NULL, NULL, NULL);
    splitFilename ("a:foo/bar", NULL, NULL, NULL, NULL);
    splitFilename ("a:/foo/bar.z", NULL, NULL, NULL, NULL);
    splitFilename ("a:foo/bar/door/flowe.p.c", NULL, NULL, NULL, NULL);
    splitFilename ("a", NULL, NULL, NULL, NULL);
    splitFilename ("a.c", NULL, NULL, NULL, NULL);
    splitFilename ("a.c", NULL, NULL, NULL, NULL);
    splitFilename ("a:.c", NULL, NULL, NULL, NULL);
    splitFilename ("foo", NULL, NULL, NULL, NULL);
    splitFilename ("/foo.c", NULL, NULL, NULL, NULL);
    splitFilename ("foo.tar.z", NULL, NULL, NULL, NULL);
    splitFilename ("foo/bar", NULL, NULL, NULL, NULL);
    splitFilename ("/foo/bar.z", NULL, NULL, NULL, NULL);
    splitFilename ("foo/bar/door/flowe.p.c", NULL, NULL, NULL, NULL);
    return (0);
}

#endif
