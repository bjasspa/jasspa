/****************************************************************************
 *
 *  			Copyright 1997-2004 Jon Green.
 *			    All Rights Reserved
 *
 *
 *  System        : 
 *  Module        : 
 *  Object Name   : $RCSfile: hts.c,v $
 *  Revision      : $Revision: 1.3 $
 *  Date          : $Date: 2004-02-07 19:29:49 $
 *  Author        : $Author: jon $
 *  Last Modified : <040207.1916>
 *
 *  Description	
 *
 *  Notes
 *
 *  History
 *	
 * 1.0.0b - JG 2004-02-07 Ported to HP-UX
 * 1.0.0a - JG 2004-01-03 Ported to Sun Solaris 9
 * 1.0.0  - JG 1997-05-10 Created new utility.
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
#include <sys/stat.h>

#if ((defined _HPUX) || (defined _LINUX) || (defined _SUNOS))
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#else
#include <getopt.h>
#endif

#ifdef _WIN32
#include <direct.h>
#endif

#include <utils.h>

#define MODULE_VERSION  "1.0.0b"
#define MODULE_NAME     "hts"
#define LINELEN (1024*8)                /* Maximum length of I/P line */

#define HTS_CONCAT   1                  /* Concatination mode */
#define HTS_BINARY   2                  /* Binary mode */

static char *progname = MODULE_NAME;    /* Program name */
static char *subDirectory = NULL;       /* SubDirectory name */
static int  operativeMode = 0;          /* Operation mode */

static int
processData (FILE *ofp, char *filename)
{
    FILE *ifp;                          /* Input file pointer */
    char *fname;
    char *ext;                          /* Extension component */
    char *base;                         /* Base name component */
    char *bname;                        /* The base name */
    int  cc;
    
    uVerbose (1, "Processing File [%s]\n", filename);
    
    if ((ifp = fopen (filename, "rb")) == NULL)
        return uError ("Cannot open file [%s]\n", filename);
    
    /* Concatinate the data */
    if (operativeMode & HTS_CONCAT)
    {
        while ((cc = fgetc (ifp)) != EOF)
            fputc (cc, ofp);
        fclose (ifp);
        return 0;
    }
                  
    /* Work out the basename of the file */
    fname = unifyFilename (filename);
    if (splitFilename (fname, NULL, NULL, &base, &ext) != 0)
    {
        fclose (ifp);
        return uError ("Cannot split filename [%s]\n");
    }
    bname = makeFilename (NULL, NULL, base, ext);
    
    /* Append the new file entry */
    fprintf (ofp, "<<%s> ", (operativeMode & HTS_BINARY) ? "BINARY" : "FILE");
    if (subDirectory)
        fprintf (ofp, "%s/", subDirectory);
    fprintf (ofp, "%s >\n", bname);
    
    /* Append the data */
    if (operativeMode & HTS_BINARY)
    {
        unsigned char buf [1];
        char xdigits [] = "0123456789ABCDEF";
        int ii = 35;
        
        while (fread (buf, 1, 1, ifp) == 1)
        {
            if (--ii < 0)
            {
                ii = 34;
                fputc ('\n', ofp);
            }
            fputc (xdigits [(buf [0] >> 4) & 0xf], ofp);
            fputc (xdigits [buf [0] & 0xf], ofp);
        }
        fputc ('\n', ofp);
    }
    else
    {
        while ((cc = fgetc (ifp)) != EOF)
            fputc (cc, ofp);
    }
    
    /* Close the file */
    fclose (ifp);
    return (0);
}
    
static void
usage (void)
{
    fprintf (stdout, "%s : Usage %s [Options] <files>\n", progname, progname);
    fprintf (stdout, "\n");
    fprintf (stdout, "-?/h      : Help - this page\n");
    fprintf (stdout, "-a <file> : Append to .hts file\n");
    fprintf (stdout, "-b        : Add in binary mode.\n");
    fprintf (stdout, "-c        : Concatinate mode (.hts)\n");
    fprintf (stdout, "-E <file> : Log errors to <file> (append)\n");
    fprintf (stdout, "-e <file> : Log errors to <file> (re-write)\n");
    fprintf (stdout, "-I        : Version Information\n");
    fprintf (stdout, "-o <file> : Output to .hts file\n");
    fprintf (stdout, "-q        : Quiet.\n");
    fprintf (stdout, "-s <name> : Defined extraction subdirectory.\n");
    fprintf (stdout, "-v <level>: Verbose level.\n");
    exit (1);
}

int main (int argc, char *argv [])
{
    int ecount = 0;                     /* Error count */
    int wcount = 0;                     /* Warn count */
    int c;
    int i;
    FILE *ofp = NULL;                   /* Output file pointer */
    char *appendFile = NULL;            /* Append file name */
    char *outputFile = NULL;            /* Output file name */

    uErrorSet (20, &ecount);            /* Max Num entries + counter */
    uWarnSet (&wcount);                 /* Warning count */
    uUtilitySet (progname);             /* Set name of program */
    uVerboseSet (1);                    /* Verbose setting */

    while (1) {
        c = getopt (argc, argv, "a:bce:E:Ih?o:qs:v:");
        if (c == EOF)
            break;
        switch (c) {
        case 'a':
            appendFile = optarg;
            break;
        case 'b':
            if (operativeMode == 0)
                operativeMode = HTS_BINARY;
            else
                uWarn ("Operating mode already defined. Ignoring -b\n");
            break;
        case 'c':
            if (operativeMode == 0)
                operativeMode = HTS_CONCAT;
            else
                uWarn ("Operating mode already defined. Ignoring -c\n");
            break;
        case 'e':
            uErrorChannelSet (optarg, UERROR_LOG_CREATE);
            break;
        case 'E':
            uErrorChannelSet (optarg, UERROR_LOG_APPEND);
            break;
        case 'I':
            fprintf (stdout, "%s. Version %s. %s\n",
                     progname, MODULE_VERSION, __DATE__);
            exit (0);
                       break;
        case 'o':
            outputFile = optarg;
            break;
        case 'q':
            uInteractiveSet (0);
            break;
        case 'v':
            uVerboseSet (optarg[0] - '0');
            break;
        case 's':
            subDirectory = optarg;
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

    uVerbose (0, "%s. Version %s. %s\n", progname, MODULE_VERSION, __DATE__);
    uOpenErrorChannel ();
    argv = getfiles (&argc, argv, optind);
    
    /* Process the output file. */
    if (appendFile != 0)
    {
        if ((ofp = fopen (appendFile, "ab")) == NULL)
            uFatal ("Cannot open output file [%s] for append\n", appendFile);
    }
    else if (outputFile != NULL)
    {
        if ((ofp = fopen (outputFile, "wb")) == NULL)
            uFatal ("Cannot open output file [%s] for overwrite\n", outputFile);
    }
    else
        uFatal ("No output file specified\n");
        
    /* Add the sub-directory */
    if (subDirectory != NULL)
    {
        char *p;
        
        /* Convert any backslashes into forward slashes */
        for (p = subDirectory; *p != '\0'; p++)
            if (*p == '\\')
                *p = '/';
        
        /* Remove the trainling '/' */
        if (*--p == '/')
            *p = '\0';
        
        /* Make sure we still have a string */
        if (*subDirectory == '\0')
            subDirectory = NULL;
        else
            fprintf (ofp, "<<SUBDIRECTORY> %s >\n", subDirectory);
    }
    
    /* Process all of the files */
    for (i = 1; i < argc; i++)
        if (processData (ofp, argv[i]))
            break;

    uCloseErrorChannel ();
    return (ecount);
}



