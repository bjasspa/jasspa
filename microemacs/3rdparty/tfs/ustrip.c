/* -*- C -*- ****************************************************************
 *
 *  System        : Built-in File System
 *  Module        : Strip the archive from the end of a file.
 *  Object Name   : $RCSfile: ustrip.c,v $
 *  Revision      : $Revision: 1.1 $
 *  Date          : $Date: 2010-08-30 15:12:48 $
 *  Author        : $Author: bill $
 *  Created By    : Jon Green
 *  Created       : Sun Nov 8 07:39:18 2009
 *  Last Modified : <100829.0216>
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

#ifdef _WIN32
/**
 * Replacement of UNIX truncate()
 * 
 * @param [in] filename The name of the file to truncate.
 * @param [in] offset The bytes offset in the file to discard from.
 * 
 * @return 0 on success.
 */
static int 
truncate (char *filename, off_t offset)
{
    int status = 0;
    int fh;
    
    fh = _open (filename, _O_RDWR|_O_CREAT, _S_IREAD|_S_IWRITE);
    if (fh > 0)
    {
        status = _chsize (fh, offset);
        _close (fh);
    }
    else
        status = -1;
    return status ;
}
#endif

/**
 * Strip the archive from the end of a file. The archive is removed from the
 * end of the file leaving the original file. No backup file is made.
 * 
 * @param [in] options  The option mask.
 * @param [in] archpath The name of the archive to strip.
 * 
 * @return The status of the call, 0 is success.
 */
int
tfs_strip (int options, char *archpath)
{
    tfs_t tfs;
    int status = 0;
    
    /* Determine if we have access to the file. */
#ifdef _WIN32
    /* Can't read if doesn't exist or its a directory */
    status = ((GetFileAttributes(archpath) & FILE_ATTRIBUTE_DIRECTORY) != 0) ;
#else
    status = access (archpath, F_OK | W_OK | R_OK);
#endif
    if (status != 0)
        fprintf (stderr, "Cannot access file \"%s\"\n", archpath);
    else
    {
        /* Mount the file system. */
        tfs = tfs_mount (archpath, TFS_CHECK_NONE);
        if (tfs == NULL)
            fprintf (stderr, "Stripping archive and none attached \"%s\"\n", archpath);
        else
        {
            tfsinfo_t *minfo;
        
            /* Get the offset of the archive. */
            minfo = tfs_info (tfs);
            if (minfo == NULL)
            {
                fprintf (stderr, "Cannot get archive information \"%s\"\n", archpath);
                status = -1;
            }
            else if (minfo->file_offset == 0)
                fprintf (stderr, "Stripping archive and none attached \"%s\"\n", archpath);
            else
            {
                /* Delete the end of the file which contains the archive. */
                status = truncate (archpath, minfo->file_offset);
                if ((options & TFSUTIL_OPT_QUIET) == 0)
                    fprintf (stderr, "Stripped archive \"%s\" [%s]\n", archpath, strstatus (status));
                else if (status != 0)
                    fprintf (stderr, "ERROR: Problem stripping archive from \"%s\"\n",
                             archpath);
            }
        }
        
        /* Unmount the file system */
        tfs_umount (tfs);
    }
    
    /* Return the status to the caller. */
    return status;
}
                
        
