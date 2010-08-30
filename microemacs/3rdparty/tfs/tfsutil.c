/* -*- C -*- ****************************************************************
 *
 *  System        : Built-in File System
 *  Module        : Archive builder.
 *  Object Name   : $RCSfile: tfsutil.c,v $
 *  Revision      : $Revision: 1.1 $
 *  Date          : $Date: 2010-08-30 15:12:47 $
 *  Author        : $Author: bill $
 *  Created By    : Jon Green
 *  Created       : Sat Nov 7 19:24:43 2009
 *  Last Modified : <100829.0138>
 *
 *  Description
 *
 *  Notes
 *
 *  History
 *
 ****************************************************************************
 *
 *  Copyright (c) 2009 Jon Green.
 *
 *  All Rights Reserved.
 *
 * This  document  may  not, in  whole  or in  part, be  copied,  photocopied,
 * reproduced,  translated,  or  reduced to any  electronic  medium or machine
 * readable form without prior written consent from Jon Green.
 *
 ****************************************************************************/

#include "tfsutil.h"

/** The name of the program */
#define PROG_NAME      "tfs"
/** Copyright statement for program */
#define PROG_COPYRIGHT "Copyright (c) 2009 Jon Green <jon@jasspa.com>"
/* Platform identification. */
#ifdef _SUNOS
#define PROG_PLATFORM  "SunOS"
#elif (defined _LINUX)
#define PROG_PLATFORM  "Linux"
#elif (defined _WIN32)
#define PROG_PLATFORM  "Win32"
#else
#define PROG_PLATFORM  "UNIX"
#endif

/* Version numbers */
int tfs_version_major = TFS_VERSION_MAJOR;
int tfs_version_minor = TFS_VERSION_MINOR;
int tfs_version_micro = TFS_VERSION_MICRO;

/**
 * version; print the version information on the given stream.
 *
 * @param fp  (in)  The file pointer to the stream on which the message
 *                  should be delivered.
 */
static void
version (FILE *fp)
{
    fprintf (fp, "%s: V%d.%d.%d : (%s) %s\n",
             PROG_NAME, 
             tfs_version_major, tfs_version_minor, tfs_version_micro,
             PROG_PLATFORM, __DATE__);
    fprintf (fp, "%s: %s\n", PROG_NAME, PROG_COPYRIGHT);
}

/**
 * usage; Print the usage of the utility on the given stream. Provides some
 * help for the user. There is NO return from this call.
 *
 * @param fp  (in) The reporting stream file descriptor.
 */
static void
usage (FILE *fp)
{
    fprintf (fp, "Synopsis: Build and manipulate a TFS archive.\n");
    fprintf (fp, "Usage:    %s [options] <dir> or <file>\n", PROG_NAME);
    fprintf (fp, "Options:\n");
    fprintf (fp, "-a <file>  : Append archive to end of existing file\n");
    fprintf (fp, "-c <file>  : Create a standalone archive\n");
    fprintf (fp, "-f <file>  : Extract archive to a file.\n");
    fprintf (fp, "-i         : Print archive information.\n");
    fprintf (fp, "-l         : List the contents of the archive.\n");
    fprintf (fp, "-n/-N      : Exclude(n)/Allow(N) zero length files, exclude by default\n");
    fprintf (fp, "-o <file>  : The name of the output file (optionally used with -a).\n");
    fprintf (fp, "-q         : Quiet, minimise messages.\n");
    fprintf (fp, "-s         : Strip an archive from the end of a file.\n");
    fprintf (fp, "-t         : Test the archive\n");
    fprintf (fp, "-v         : Enable verbose information.\n");    
    fprintf (fp, "-x <dir>   : Extract archive to a directory.\n");
    fprintf (fp, "-z <type>  : Compression options (default is -z9f)\n");
    fprintf (fp, "             d - Compress directories\n");
    fprintf (fp, "             f - Compress file\n");
    fprintf (fp, "             0-9 Compression level, 9 is maximum, 0 is none\n"); 
    fprintf (fp, "-h/-?      : Help information\n");
    fprintf (fp, "\n");

    /* No return */
    exit (1);
}

/**
 * The main program.
 *
 * @param [in] argc The number of arguments.
 * @param [in] argv
 */
int
main (int argc, char *argv[])
{
    char *tfsopt_append = NULL;         /* File to append. */
    char *tfsopt_output = NULL;         /* Name of the output file. */
    char *tfsopt_create = NULL;         /* Create a file. */
    char *tfsopt_xdir = NULL;           /* Extract to directory */
    char *tfsopt_xfile = NULL;          /* Extract to file */
    char cc, *ss, tfsopt_do = 0;
    int tfsopt_flags;                   /* Option flags */
    int status = 0;                     /* Status of the call. */
    int ii;                             /* Local integer variable */
    
    /* Assign default options */
    tfsopt_flags = (TFSUTIL_OPT_COMPRESS_FILE |
                    TFSUTIL_OPT_SKIP_ZERO_LEN);
    
    /* Process the options */
    ii = 0 ;
    while((++ii < argc) && (argv[ii][0] == '-'))
    {
        cc = argv[ii][1] ;
        if(strchr("acfilstx",cc) != NULL)
        {
            if (tfsopt_do != 0)
                fprintf (stderr, "WARN: Ignoring option %c - using option %c\n", tfsopt_do, cc);
            tfsopt_do = cc;
        }
        if(strchr("acfoxz",cc) != NULL)
        {
            if(argv[ii][2] != '\0')
                ss = argv[ii] + 2 ;
            else if(++ii < argc)
                ss = argv[ii] ;
            else
            {
                fprintf (stderr, "ERROR: Argument expected with -%c option\n",cc);
                usage (stderr);
            }
        }
        switch (cc)
        {
            /* -a <file>  : Append archive to end of existing file. */
        case 'a':
            if (tfsopt_append != NULL)
                free (tfsopt_append);
            tfsopt_append = strdup (ss);
            break;

            /* -c <file>  : Create a standalone archive */
        case 'c':
            if (tfsopt_create != NULL)
                free (tfsopt_create);
            tfsopt_create = strdup (ss);
            break;

            /* -f <file>  : Extract archive to a file. */
        case 'f':
            if (tfsopt_xfile != NULL)
                free (tfsopt_xfile);
            tfsopt_xfile = strdup (ss);
            break;
            
            /* -i : Print the achive information. */
            /* -l : List the contents of the archive. */
            /* -s : Strip the archive from end of file. */
            /* -t : Test the archive */
        case 'l':
        case 's':
        case 't':
        case 'i':
            break;
            
            /* -n : Exclude zero length files */
        case 'n':
            tfsopt_flags |= TFSUTIL_OPT_SKIP_ZERO_LEN;
            break;
            
            /* -N : Allow zero length files */
        case 'N':
            tfsopt_flags &= ~TFSUTIL_OPT_SKIP_ZERO_LEN;
            break;
            
            /* -o <file>  : Name of the output file. */
        case 'o':
            if (tfsopt_output != NULL)
                free (tfsopt_output);
            tfsopt_output = strdup (ss);
            break;

            /* -q : Quiet running, minimise messages. */
        case 'q':
            tfsopt_flags &= ~TFSUTIL_OPT_VERBOSE;
            tfsopt_flags |= TFSUTIL_OPT_QUIET;            
            break;      
            
            /* -v : Enable verbose messages */
        case 'v':
            tfsopt_flags &= ~TFSUTIL_OPT_QUIET;
            tfsopt_flags |= TFSUTIL_OPT_VERBOSE;
            break;
            
            /* -x <dir>   : Extract archive to a directory. */
        case 'x':
            if (tfsopt_xdir != NULL)
                free (tfsopt_xdir);
            tfsopt_xdir = strdup (ss);
            break;

            /* -z <type>  : Compression options (default is -zf) */
        case 'z':
            while((cc=*ss++) != '\0')
            {
                /* f - Compress file */
                if (cc == 'f')
                    tfsopt_flags |= TFSUTIL_OPT_COMPRESS_FILE;
                /* d - Compress directories */
                else if (cc == 'd')
                    tfsopt_flags |= TFSUTIL_OPT_COMPRESS_DIR;
                /* ! - No compression */
                else if (cc == '!')
                    tfsopt_flags &= ~(TFSUTIL_OPT_COMPRESS_FILE |
                                      TFSUTIL_OPT_COMPRESS_DIR);
                else
                    fprintf (stderr, "WARN: Ignoring compression option option %c\n", cc);
            }
            break;

        default:
            fprintf (stderr, "Illegal option %c (%#02x)\n\n",
                     (isprint (cc) ? cc : '?'), (int)(cc));
            /* Allow drop through */

            /* -?/-h : Help */
        case '?':
        case 'h':
            version (stdout);
            usage (stdout);
            /* No return */
        }
    }

    /* Print the version information */
    if ((tfsopt_flags & TFSUTIL_OPT_QUIET) != 0)
        version (stderr);

    /* Make sure some  arguments have been specified. */
    if ((argc - ii) == 0)
    {
        fprintf (stderr, "ERROR: No arguments specified.\n");
        exit (1);
    }

    /* Process the arguments that exist */
    for (status = 0; (ii < argc) && (status == 0); ii++)
    {
        switch (tfsopt_do)
        {
            /* Append archive */
        case 'a': 
            status = tfs_append (tfsopt_flags, tfsopt_append, tfsopt_output, argv[ii]);
            break;
            
            /* Create an archive */
        case 'c': 
            status = tfs_create (tfsopt_flags, tfsopt_create, argv[ii]);
            break;
            
            /* Extract the archive as a file */
        case 'f':
            status = tfs_xfile (tfsopt_flags, tfsopt_xfile, argv[ii]);
            break;
            
            /* Print the archive information */
        case 'i':
            status = tfs_printinfo (tfsopt_flags, argv[ii]);
            break;
            
            /* List the archive */
        case 'l':
            status = tfs_list (tfsopt_flags, argv[ii]);
            break;
            
            /* Remove the archive */
        case 's':
            status = tfs_strip (tfsopt_flags, argv[ii]);
            break;
            
            /* Test the archive. */
        case 't':
            status = tfs_test (tfsopt_flags, argv[ii]);
            break;
            
            /* Extract the archive as a directory */
        case 'x':
            status = tfs_xdir (tfsopt_flags, tfsopt_xdir, argv[ii]);
            break;
            
            /* Handle an error */
        default:
            fprintf (stderr, "Unhandled option '%c'(%d)\n",
                     (isprint (tfsopt_do) ? tfsopt_do : '?'), tfsopt_do);
            status = -1;
            break;
        }
    }
    
    /* Exit with the status of the call. */
    return (status);
}
