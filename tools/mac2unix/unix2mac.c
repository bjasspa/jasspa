/* -*- C -*- ****************************************************************
 *
 *			Copyright 2003 Jon Green.
 *			      All Rights Reserved
 *
 *
 *  System        :
 *  Module        :
 *  Object Name   : $RCSfile: unix2mac.c,v $
 *  Revision      : $Revision: 1.1 $
 *  Date          : $Date: 2003-07-27 17:14:48 $
 *  Author        : $Author: jon $
 *  Created By    : Jon Green
 *  Created       : Sun Jul 27 17:43:46 2003
 *  Last Modified : <030727.1802>
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
 *  Copyright (c) 2003 Jon Green.
 *
 *  All Rights Reserved.
 *
 * This  document  may  not, in  whole  or in  part, be  copied,  photocopied,
 * reproduced,  translated,  or  reduced to any  electronic  medium or machine
 * readable form without prior written consent from Jon Green.
 *
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#define PROGNAME "unix2mac"

int
main (int argc, char *argv[])
{
    FILE *ifp;
    FILE *ofp;
    int cc;

    /* Confirm that the arguments are OK */
    if (argc != 3)
    {
        fprintf (stderr, "ERROR: Command line \"%s <originalFile> <convertedFile>\"\n",
                 PROGNAME);
        return (1);
    }

    /* Make sure the input and output are different */
    if ((argv[1] == NULL) || (*(argv[1]) == '\0'))
    {
        fprintf (stderr, "ERROR: First argument is empty\n");
        return 2;
    }
    if ((argv[2] == NULL) || (*(argv[2]) == '\0'))
    {
        fprintf (stderr, "ERROR: Second argument is empty\n");
        return 2;
    }

    /* Make sure the names are not the same. */
    if (strcmp (argv[1], argv[2]) == 0)
    {
        fprintf (stderr, "ERROR: Input and output file are the same\n");
        return 3;
    }

    /* Open the input file */
    if ((ifp = fopen (argv[1], "rb")) == NULL)
    {
        fprintf (stderr, "Cannot open file \"%s\" for reading\n", argv[1]);
        return 4;
    }
    if ((ofp = fopen (argv[2], "wb")) == NULL)
    {
        fprintf (stderr, "Cannot open file \"%s\" for writing\n", argv[2]);
        fclose (ifp);
        return 4;
    }

    /* Perform the translation */
    while ((cc = fgetc (ifp)) != EOF)
    {
        /* Ignore '\r' */
        if (cc == '\r')
            continue;

        /* Translate '\n' */
        if (cc == '\n')
            cc = '\r';

        /* Put the character */
        fputc (cc, ofp);
    }

    /* Close the streams */
    fclose (ifp);
    fclose (ofp);
    return (0);
}
