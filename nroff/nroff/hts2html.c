/****************************************************************************
 *
 *			Copyright 1995 Jon Green.
 *		          All Rights Reserved
 *
 *
 *  System        :
 *  Module        :
 *  Object Name   : $RCSfile: hts2html.c,v $
 *  Revision      : $Revision: 1.1 $
 *  Date          : $Date: 2000-10-21 14:31:27 $
 *  Author        : $Author: jon $
 *  Last Modified : <120597.2022>
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
 *  Copyright (c) 1995 Jon Green.
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

#if ((defined _HPUX) || (defined _LINUX))
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

/*
 * 1.0.4c - JG 16/11/95 - Ported to UNIX world
 * 1.0.4d - JG 20/04/97 - Added &HTML& expansion for file extension.
 * 1.0.4e - JG 12/05/97 - Added binary entry.
 */

#define MODULE_VERSION  "1.0.4e"
#define MODULE_NAME     "hts2html"
#define LINELEN (1024*8)                /* Maximum length of I/P line */

static const char rcsid[] = "@(#) : $Id: hts2html.c,v 1.1 2000-10-21 14:31:27 jon Exp $";

static char *progname = MODULE_NAME;    /* Program name */
static char *outPath = NULL;            /* Output path */
static char *linkExtension = NULL;      /* Link file extension */
static int  linkExtLen = 0;             /* Link extension length */
static FILE *fo = NULL;                 /* Output file pointer */
static int  binaryFile = 0;             /* Processing a binary file */

static void
convertBinary (char *buffer)
{
    int len;
    int ii;
    unsigned char byte;
    
    if (((len = strlen (buffer)) & 1) != 0)
    {
        uError ("Even number of characters expected on a binary line\n");
        return;
    }
    
    /* Convert the line to binary */
    byte = 0;
    while (len > 0)
    {
        ii = *buffer++;
        if (isdigit (ii))
            byte |= ii - '0';
        else
            byte |= tolower (ii) + 10 - 'a';
        
        if (len & 1)
        {
            fwrite (&byte, 1, 1, fo);
            byte = 0;
        }
        else
            byte <<= 4;
        len--;
    }
}        

static void
processBinary (char *buffer)
{
    char *pname;
    char *fname;
    int len;
    
    /* Always close the previous file descriptor. */
    if (fo != NULL)
        fclose (fo);

    /* Extract the file name from the <<FILE> name> tag. */
    fname = cleanWhiteSpace (buffer);
    if ((len = strlen (fname)) == 0)
        uError ("Zero length file in <<FILE>%s\n", buffer);
    else if (fname [len-1] != '>') {
        uVerbose (1, "len = %d. fname = [%s]\n", len, fname);
        uError ("Expected closing '>' in <<FILE>%s\n", buffer);
    }
    else
    {
        /* Strip white space off end. */
        fname [len-1] = '\0';
        fname = trimWhiteSpace (fname);
        
        /* Close the previous output file descriptor and open a new
         * one with the new file name. */
        pname = makeFilename (NULL, outPath, fname, NULL);
        if ((fo = fopen (pname, "wb")) == NULL)
            uError ("Cannot open output file [%s]\n", pname);
        else
            uInteractive ("Expanding: %s", pname);
        binaryFile = 1;
    }
}
    

static void
processDirectory (char *buffer)
{
    char *pname;
    char *dname;
    int  len;

    /*
     * Extract the directory name from the <<SUBDIRECTORY> name> tag.
     */
    dname = cleanWhiteSpace (buffer);
    if ((len = strlen (dname)) == 0)
        uError ("Zero length file in <<SUBDIRECTORY>%s\n", buffer);
    else if (dname [len-1] != '>')
        uError ("Expected closing '>' in <<SUBDIRECTORY>%s\n", buffer);
    else
    {
        struct stat sbuf;
        
        /* Strip white space off end. */
        dname [len-1] = '\0';
        dname = trimWhiteSpace (dname);

        /* Make the new directory */
        pname = makeFilename (NULL, outPath, dname, NULL);
        
        /* Check the existance of the directory */
        if (stat (pname, &sbuf) >= 0)
        {
            /* A node exists with the pathname */
            if (S_IFDIR & sbuf.st_mode)
                return;
        }
        if (mkdir (pname, 0777) < 0)
            uFatal ("Cannot construct sub-directory [%s]\n", pname);
        uVerbose (1, "Constructing directory [%s]\n", pname);
        
        
    }
}

static void
processFile (char *buffer)
{
    char *pname;
    char *fname;
    int  len;

    /* Always close the previous file descriptor. */
    if (fo != NULL)
        fclose (fo);

    /* Extract the file name from the <<FILE> name> tag. */
    fname = cleanWhiteSpace (buffer);
    if ((len = strlen (fname)) == 0)
        uError ("Zero length file in <<BINARY>%s\n", buffer);
    else if (fname [len-1] != '>') {
        uVerbose (1, "len = %d. fname = [%s]\n", len, fname);
        uError ("Expected closing '>' in <<BINARY>%s\n", buffer);
    }
    else
    {
        /* Strip white space off end. */
        fname [len-1] = '\0';
        fname = trimWhiteSpace (fname);
        
        /* Close the previous output file descriptor and open a new
         * one with the new file name. */
        pname = makeFilename (NULL, outPath, fname, NULL);
        if ((fo = fopen (pname, "wb")) == NULL)
            uError ("Cannot open output file [%s]\n", pname);
        else
            uInteractive ("Expanding: %s", pname);
        binaryFile = 0;
    }
}

static void
processData (char *fname)
{
    FILE *fp;
    int  c;
    char *bp;
    char buffer [LINELEN];
    int  lineNo = 0;
    int  n;

    if ((fp = fopen (fname, "rb")) == NULL) {
        uError ("Cannot open file [%s] for reading.\n", fname);
        return;
    }

    uFileSet (&lineNo, fname);
    uInteractive ("Expanding File [%s]\n", fname);
    uVerbose (1, "Expanding File [%s]\n", fname);

    /* Process each line of the file indevidually */
    do
    {
        /* Read in a line of data. */
        bp = buffer;
        n = 0;
        while ((c = fgetc (fp)) != EOF)
        {
            if ((c == '\r') || (c == 0x1a)) /* Ignore '\r' and ctrl-z */
                continue;
            if (c == '\n') {
                lineNo++;
                break;
            }
            if (++n >= LINELEN)
            {
                if (n == LINELEN)
                    uError ("Line length exceeds %d characters. Truncated.\n",
                            LINELEN);
                continue;
            }
            /*
             * Determine if there is a special '&HTML&' file extension
             * character in the buffer.
             */
            *bp++ = c;
            if ((c == '&') && (n > 5))
            {
                if (strncmp (bp-6, "&HTML&", 6) == 0)
                {
                    bp -= 6;
                    n += linkExtLen + 6;
                    strcpy (bp, linkExtension);
                    bp += linkExtLen;
                }
            }
        }

        *bp = '\0';
        /*
         * Determine if this is a control line
         */
        if ((buffer [0] == '<') && (buffer [1] == '<'))
        {
            if (strncmp (buffer, "<<FILE>", 7) == 0)
                processFile (&buffer[7]);
            else if (strncmp (buffer, "<<SUBDIRECTORY>", 15) == 0)
                processDirectory (&buffer[15]);
            else if (strncmp (buffer, "<<BINARY>", 9) == 0)
                processBinary (&buffer [9]);
            else
                uError ("Illegal HTS command word [%s]\n", buffer);
            continue;
        }
        if (fo != NULL)
        {
            if (binaryFile)
                convertBinary (buffer);
            else
                fprintf (fo, "%s\n", buffer);
        }
        else
            uError ("No output file specified. Ignoring data\n");
    } while (c != EOF);

    /*
     * Close the output file, if open.
     */

    if (fo != NULL)
    {
        fclose (fo);
        fo = NULL;
    }
    fclose (fp);
    uFileSet (NULL, NULL);
}


static void
usage (void)
{
    fprintf (stdout, "%s : Usage %s [Options] <files>\n", progname, progname);
    fprintf (stdout, "\n");
    fprintf (stdout, "-?/h      : Help - this page\n");
    fprintf (stdout, "-E <file> : Log errors to <file> (append)\n");
    fprintf (stdout, "-e <file> : Log errors to <file> (re-write)\n");
    fprintf (stdout, "-l <link> : Link file extension (default .html)\n");
    fprintf (stdout, "-I        : Version Information\n");
    fprintf (stdout, "-p <path> : Output path.\n");
    fprintf (stdout, "-q        : Quiet.\n");
    fprintf (stdout, "-v <level>: Verbose level.\n");
    exit (1);
}

int main (int argc, char *argv [])
{
    int ecount = 0;                     /* Error count */
    int wcount = 0;                     /* Warn count */
    int c;
    int i;

    uErrorSet (20, &ecount);            /* Max Num entries + counter */
    uWarnSet (&wcount);                 /* Warning count */
    uUtilitySet (progname);             /* Set name of program */
    uVerboseSet (1);                    /* Verbose setting */

    while (1) {
        c = getopt (argc, argv, "e:E:Ih?l:p:qv:");
        if (c == EOF)
            break;
        switch (c) {
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
        case 'l':
            linkExtension = optarg;
            break;
        case 'q':
            uInteractiveSet (0);
            break;
        case 'v':
            uVerboseSet (optarg[0] - '0');
            break;
        case 'p':
            outPath = optarg;
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

    /* Set up the default extension */
    if (linkExtension == NULL)
        linkExtension = ".html";
    linkExtLen = strlen (linkExtension);

    uVerbose (0, "%s. Version %s. %s\n", progname, MODULE_VERSION, __DATE__);
    uOpenErrorChannel ();
    argv = getfiles (&argc, argv, optind);

    /* Process all of the files */
    for (i = 1; i < argc; i++)
        processData (argv[i]);

    uCloseErrorChannel ();
    return (ecount);
}
