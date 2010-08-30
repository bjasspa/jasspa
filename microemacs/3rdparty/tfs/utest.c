/* -*- C -*- ****************************************************************
 *
 *  System        : Built-in File System
 *  Module        : Test the integrity of the archive.
 *  Object Name   : $RCSfile: utest.c,v $
 *  Revision      : $Revision: 1.1 $
 *  Date          : $Date: 2010-08-30 15:12:48 $
 *  Author        : $Author: bill $
 *  Created By    : Jon Green
 *  Created       : Sun Nov 8 07:39:18 2009
 *  Last Modified : <100829.0210>
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

/**
 * Test the integrity of the archive.
 * 
 * @param [in] options  The option mask.
 * @param [in] archpath The name of the archive to strip.
 * 
 * @return The status of the call, 0 is success.
 */
int
tfs_test (int options, char *archpath)
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
            fprintf (stderr, "No archive found in \"%s\"\n", archpath);
        else
        {
            int head_status;
            int data_status;
            
            /* Check the header */
            head_status = tfs_check (tfs, TFS_CHECK_HEAD);
            if ((options & TFSUTIL_OPT_QUIET) == 0)
                fprintf (stderr, "Head check on \"%s\" [%s]\n", archpath, strstatus (head_status));
            else if (head_status != 0)
                fprintf (stderr, "Archive header corrupt \"%s\"\n", archpath);
            
            /* Check the data */
            data_status = tfs_check (tfs, TFS_CHECK_DATA);
            if ((options & TFSUTIL_OPT_QUIET) == 0)
                fprintf (stderr, "Data check on \"%s\" [%s]\n", archpath, strstatus (data_status));
            else if (data_status != 0)
                fprintf (stderr, "Archive data corrupt \"%s\"\n", archpath);
            
            /* Collate the status */
            status |= head_status | data_status;
            
            /* Unmount the file system */
            tfs_umount (tfs);
        }
    }        
    
    /* Return the status to the caller. */
    return status;
}
                
