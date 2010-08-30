/* -*- C -*- ****************************************************************
 *
 *  System        : Built-in File System
 *  Module        : Extraction to a directory.
 *  Object Name   : $RCSfile: uxdir.c,v $
 *  Revision      : $Revision: 1.1 $
 *  Date          : $Date: 2010-08-30 15:12:48 $
 *  Author        : $Author: bill $
 *  Created By    : Jon Green
 *  Created       : Sat Nov 7 21:02:27 2009
 *  Last Modified : <100830.1256>
 *
 *  Description   : Opens archive and extracts contents hierarchically to a 
 *                  directory.
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

/**
 * Utility method to extract write a file from the archive and write to a file
 * in the file system.
 *
 * @param [in] tfs The TFS file system.
 * @param [in] archpath The pathname of a file in the archive.
 * @param [in] fsyspath The file system pathname to create.
 *
 * @return status of the call, 0 is successful.
 */
static int
write_file (tfs_t tfs, char *archpath, char *fsyspath)
{
    tfsfile_t rfp = NULL;
    FILE *wfp = NULL;
    int status = 1;

    /* Mount the file system. */
    for (;;)
    {
        char buf [2048];
        int nread;
        int nwrite;
        int total = 0;
        int error = 0;

        /* Open the archive for reading. */
        if ((rfp = tfs_fopen (tfs, archpath)) == NULL)
        {
            fprintf (stderr, "Cannot open archive \"%s\" for reading\n", archpath);
            break;
        }

        /* Open the new file for writing */
        if ((wfp = fopen (fsyspath, "wb")) == NULL)
        {
            fprintf (stderr, "Cannot open file \"%s\" for writing\n", fsyspath);
            break;
        }

        /* Copy the file data out of the archive */
        do
        {
            /* Read in a chunk. */
            nread = tfs_fread (buf, 1, sizeof (buf), rfp);
            if (nread > 0)
            {
                nwrite = fwrite (buf, 1, nread, wfp);
                if ((error == 0) && (nwrite != nread))
                    error = 1;
                total += nwrite;
            }
        } while (nread > 0);

        /* Print out the result. */
        if (error != 0)
            fprintf (stderr, "Error writing \"%s\"\n", fsyspath);
        else
            status = 0;
        fprintf (stderr, "Extracted \"%s\" %d bytes.\n",
                 fsyspath, (int)(total));
        break;
    }

    /* Close the file. */
    if (rfp != NULL)
        tfs_fclose (rfp);
    if (wfp != NULL)
        fclose (wfp);

    /* Return the status */
    return status;
}

/**
 * Utility method to create a directory in the file system.
 *
 * @param [in] dirname The file system directory name to create.
 *
 * @return status of the call, 0 is successful.
 */
static int
create_dir (char *dirname)
{
    int status;

    /* File system call to create a directory. There is a difference between
     * UNIX and other systems. Differentiate the systems based on the macro
     * definitions. */
#ifdef _WIN32
    status = (CreateDirectory(dirname,NULL) == 0);    
#else
    status = mkdir (dirname, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
    return status;
}

/**
 * Utility method to extract components from a directory. This method is
 * recursive and calls itself for each directory node that is encountered. On
 * each recusion then the pathname is appended with the new directory name
 * found and then called again to be evaluated.
 *
 * @param [in] tfs The mounted file system.
 * @param [in] dirname The file system directory we are processing.
 * @param [in] archpath The tfs pathname to process a nil terminated string of
 *                      the form "/level1/level2/level3".
 */
static void
extract_dir (tfs_t tfs, char *dirname, char *archpath)
{
    char pathbuf[PATH_MAX];
    char dirbuf[PATH_MAX];
    tfsdir_t dirp;
    int pathlen;
    int dirlen;

    /* Create the path */
    strcpy (pathbuf, archpath);
    pathlen = strlen (pathbuf);
    if (pathbuf[pathlen-1] != '/')
    {
        strcat (pathbuf, "/");
        pathlen++;
    }

    /* Create the directory */
    create_dir (dirname);

    /* Create the directory buffer. */
    strcpy (dirbuf, dirname);
    dirlen = strlen (dirbuf);
    if (dirbuf[dirlen-1] != '/')
    {
        strcat (dirbuf, "/");
        dirlen++;
    }

    /* Iterate over the directories */
    if ((dirp = tfs_opendir (tfs, archpath)) != NULL)
    {
        tfsdirent_t *dirent;
        tfsstat_t statbuf;

        while ((dirent = tfs_readdir (dirp)) != NULL)
        {
            /* Skip the current directory. */
            if ((dirent->len == 1) &&
                (strncmp (".", dirent->name, dirent->len) == 0))
                continue;
            /* Skip the parent directory */
            if ((dirent->len == 2) &&
                (strncmp ("..", dirent->name, dirent->len) == 0))
                continue;

            /* Create the archive name. */
            strncpy (&pathbuf[pathlen], dirent->name, dirent->len);
            pathbuf[pathlen + dirent->len] = '\0';
            /* Create the filesystem name */
            strncpy (&dirbuf[dirlen], dirent->name, dirent->len);
            dirbuf[dirlen + dirent->len] = '\0';

            /* Stat the pathanme */
            if (tfs_stat (tfs, pathbuf, &statbuf) == 0)
            {
                if (statbuf.st_mode == TFS_TYPE_FILE)
                    write_file (tfs, pathbuf, dirbuf);
                else if (statbuf.st_mode == TFS_TYPE_DIR)
                {
                    /* Create the directory and recurse into it. */
                    create_dir (dirbuf);
                    extract_dir (tfs, dirbuf, pathbuf);
                }
            }
            else
                printf ("tfs_stat: FAILED on \"%s\"\n", pathbuf);
        }

        /* Close the directory */
        tfs_closedir (dirp);
    }
}

/**
 * Extract the archive to a directory.
 *
 * @param [in] options  The option mask.
 * @param [in] dirname The name of the output directory.
 * @param [in] archpath The name of the archive.
 *
 * @return The status of the call, 0 is success.
 */
int
tfs_xdir (int options, char *dirname, char *archpath)
{
    tfs_t tfs;
    int status = 0;

    /* Mount the file system. */
    tfs = tfs_mount (archpath, TFS_CHECK_NONE);
    if (tfs == NULL)
    {
        fprintf (stderr, "Cannot find valid archive at \"%s\"\n", archpath);
        status = -1;
    }
    else
    {
        /* Extract the directory. */
        /* TODO: Process status */
        extract_dir (tfs, dirname, "/");
        
        /* Unmount the file system */
        tfs_umount (tfs);
    }

    /* Return the status to the caller. */
    return status;
}
