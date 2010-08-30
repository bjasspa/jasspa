/* -*- C -*- ****************************************************************
 *
 *  System        : Built-in File System
 *  Module        : TFS Utility
 *  Object Name   : $RCSfile: tfsutil.h,v $
 *  Revision      : $Revision: 1.1 $
 *  Date          : $Date: 2010-08-30 15:12:47 $
 *  Author        : $Author: bill $
 *  Created By    : Jon Green
 *  Created       : Sun Nov 8 06:53:16 2009
 *  Last Modified : <100830.1243>
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

#ifndef __TFSUTIL_H
#define __TFSUTIL_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <assert.h>
#include <errno.h>
#include <time.h>
#ifdef _WIN32
#include <io.h>
#else
#include <dirent.h>
#include <unistd.h>
#endif

#include "tfs.h"

/* Handle the nil string when printing a string. */
#define strnil(s) (((s) == NULL) ? "" : (s))

/* Handle a status string */
#define strstatus(i) (((i) == 0) ? "ok" : "fail")

/* On Windows systems then there is no fseeko() and ftello() */
#ifdef _WIN32
#ifndef PATH_MAX
#define PATH_MAX MAX_PATH
#endif
#ifndef fseeko 
#define fseeko fseek
#endif
#ifndef ftello
#define ftello ftell
#endif
#endif

/* Define the utility options */
#define TFSUTIL_OPT_COMPRESS_LEVEL  0x00f  /* Compression level */
#define TFSUTIL_OPT_COMPRESS_FILE   0x010  /* Compress files */
#define TFSUTIL_OPT_COMPRESS_DIR    0x020  /* Compress directories */
#define TFSUTIL_OPT_SKIP_ZERO_LEN   0x040  /* Skip zero length files */
#define TFSUTIL_OPT_QUIET           0x080  /* Minimise status messages */
#define TFSUTIL_OPT_VERBOSE         0x100  /* Add informative messages */

/* Version numbers */
extern int tfs_version_major;
extern int tfs_version_minor;
extern int tfs_version_micro;

/**
 * List the archive on stdout.
 *
 * @param [in] options  The option mask.
 * @param [in] archpath The name of the archive to process.
 *
 * @return The status of the call, 0 is success.
 */
extern int
tfs_list (int options, char *archpath);

/**
 * Extract the archive to a directory.
 *
 * @param [in] options  The option mask.
 * @param [in] dirname The name of the output directory.
 * @param [in] archpath The name of the archive.
 *
 * @return The status of the call, 0 is success.
 */
extern int
tfs_xdir (int options, char *dirname, char *archpath);

/**
 * Extract an acrchive to the named file system file. The archive may be
 * appended to the end of the file.
 *
 * @param [in] options  The option mask.
 * @param [in] fsysname The filename system name of the output archive.
 * @param [in] archpath The path to the archive to extract.
 *
 * @return The status 0 on success otherwise an error.
 */
extern int
tfs_xfile (int options, char *fsysname, char *archpath);

/**
 * Get the archive information.
 *
 * @param [in] options  The option mask.
 * @param [in] archpath The path to the archive to extract.
 *
 * @return The status 0 on success otherwise an error.
 */
extern int
tfs_printinfo (int options, char *archpath);

/**
 * Strip the archive from the end of a file. The archive is removed from the
 * end of the file leaving the original file. No backup file is made.
 *
 * @param [in] options  The option mask.
 * @param [in] archpath The name of the archive to strip.
 *
 * @return The status of the call, 0 is success.
 */
extern int
tfs_strip (int options, char *archpath);

/**
 * Test the integrity of the archive.
 *
 * @param [in] options  The option mask.
 * @param [in] archpath The name of the archive to strip.
 *
 * @return The status of the call, 0 is success.
 */
extern int
tfs_test (int options, char *archpath);

/**
 * Construct a TFS archive.
 *
 * @param [in] options  The option mask.
 * @param [in] fp The file pointer to write.
 * @param [in] direname The directory to process.
 *
 * @return Status of the call, 0 is success.
 */
extern int
tfs_build (int options, FILE *fp, char *dirname);

/**
 * Construct a TFS archive.
 *
 * @param [in] options  The option mask.
 * @param [in] appendfile The name of the file to be appended.
 * @param [in] outputfile The name of the file to output, may be NULL if we are
 *                        rewriting the same file.
 * @param [in] fileordirename The name of the archive to append. This may be
 *                            and existing archive file or a directory.
 *
 * @return Status of the call, 0 is success.
 */
extern int
tfs_append (int options, char *appendfile, char *outputfile, char *fileordir);

/**
 * Construct a TFS archive.
 *
 * @param [in] options  The option mask.
 * @param [in] outputfile The name of the file to output.
 * @param [in] fileordirename The name of the archive to append. This may be
 *                            and existing archive file or a directory.
 *
 * @return Status of the call, 0 is success.
 */
extern int
tfs_create (int options, char *outputfile, char *fileordir);

/**
 * Copy a file.
 *
 * @param [in] options  The option mask.
 * @param [in] srcfile  The name of the file to be copied.
 * @param [in] destfile The name of the file to create.
 *
 * @return Status of the call, 0 is success.
 */
extern int
tfs_filecopy (int options, char *srcfile, char *destfile);

/**
 * Append a file.
 *
 * @param [in] options  The option mask.
 * @param [in] srcfile  The name of the file to be copied.
 * @param [in] destfile The name of the file to append.
 *
 * @return Status of the call, 0 is success.
 */
extern int
tfs_fileappend (int options, char *srcfile, char *destfile);

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif
#endif /* __TFSUTIL_H */
