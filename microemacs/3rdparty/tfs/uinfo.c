/* -*- C -*- ****************************************************************
 *
 *  System        : Built-in File System
 *  Module        : Display the archive information.
 *  Object Name   : $RCSfile: uinfo.c,v $
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
 * Get the archive information.
 * 
 * @param [in] options  The option mask.
 * @param [in] archpath The name of the archive to strip.
 * 
 * @return The status of the call, 0 is success.
 */
int
tfs_printinfo (int options, char *archpath)
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
            tfsinfo_t *minfo;
        
            /* Get the offset of the archive. */
            minfo = tfs_info (tfs);
            if (minfo == NULL)
            {
                fprintf (stderr, "Cannot get archive information \"%s\"\n", archpath);
                status = -1;
            }
            else 
            {
                /* Print the information. */
                fprintf (stdout, "Name: %s\n", strnil (archpath));
                fprintf (stdout, "Version: v%d.%d.%d\n", 
                         minfo->version_major,
                         minfo->version_minor,
                         minfo->version_micro);
                fprintf (stdout, "Created: %04d-%02d-%02d, %02d:%02d:%02d\n",
                         minfo->year, minfo->month, minfo->day, 
                         minfo->hour, minfo->minute, minfo->second);
                fprintf (stdout, "Start Addr: 0x%x/%d\n", 
                         (int)(minfo->file_offset), (int)(minfo->file_offset));
                fprintf (stdout, "Byte Size: 0x%x/%d\n", 
                         (int)(minfo->length), (int)(minfo->length));
            }
        }
    }
    
    /* Return the status to the caller. */
    return status;
}
                
        
