/* -*- C -*- ****************************************************************
 *
 *    			   Copyright 2003 Jon Green.
 *			      All Rights Reserved
 *
 *
 *  System        : 
 *  Module        : 
 *  Object Name   : $RCSfile: scrubit.c,v $
 *  Revision      : $Revision: 1.1.1.1 $
 *  Date          : $Date: 2003-03-18 00:53:39 $
 *  Author        : $Author: jon $
 *  Created By    : <unknown>
 *  Created       : Mon Mar 17 23:42:07 2003
 *  Last Modified : <030318.0038>
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
#include <time.h>                       /* Include the time */

/* BUF_SIZE; Size of the buffer in 4-byte units */
#define BUF_SIZE     1024

int 
main (int argc, char *argv[])
{
    static int buf [1024*8];            /* Local buffer */
    int ii;
    
    /* Check the arguments */
    if (argc <= 1)
    {
        fprintf (stderr, "File name extpected\n");
        return -1;
    }
    
    /* Initialise the buffer */
    srand ((unsigned int)time(NULL));
    
    /* Open the files one by one */
    for (ii = 1; ii < argc; ii++)
    {
        char fname [1024];
        FILE *fp;
        int gbyte;
        int error = 0;
        
        for (gbyte = 0; error == 0; gbyte++)
        {
            int mbyte, bytes;
            int back;
            
            bytes = 0;
            mbyte = 0;
            back = 0;
            
            sprintf (fname, "%s.%d", argv[ii], gbyte);
            
            if ((fp = fopen (fname, "wb")) == NULL)
            {
                fprintf (stderr, "Cannot open file \"%s\" for writing\n", argv[ii]);
                error = 1;
                break;
            }
            
            fprintf (stdout, "Processing file \"%s\"\n", fname);
        
            /* Fill the file up with data */
            for (;;)
            {
                int len;
                int jj;
                
                for (jj = 0; jj < sizeof (buf)/sizeof (long) ; jj++)
                    buf[jj] = rand();
                
                len = fwrite (buf, 1, sizeof (buf), fp);
                if (len < sizeof (buf))
                {
                    /* Flush the file to disk and close it. */
                    fflush (fp);
                    fclose (fp);
                    
                    /* Delete the file */
                    remove (argv[ii]);
                    while (--back >= 0)
                        fprintf (stdout, "\b");
                    fprintf (stdout, "Finished \"%s\"\n", argv[ii]);
                    back = 0;
                    fflush (stdout);
                    error = 1;
                    break;
                }
                else
                {
                    bytes += len;
                    if (bytes > 1024*1024)
                    {
                        bytes -= 1024*1024;
                        mbyte++;
                        while (--back >= 0)
                            fprintf (stdout, "\b");
                        
                        /* Quit after a GByte */
                        if (mbyte >= 1024)
                        {
                            fflush (fp);
                            fclose (fp);
                            back = 0;
                            fflush (stdout);
                            break;
                        }
                        else
                        {
                            back = fprintf (stdout, "%dM", mbyte);
                            fflush (stdout);
                        }
                    }
                }
            }
        }
        
        /* Clean up the files */
        while (gbyte >= 0)
        {
            sprintf (fname, "%s.%d", argv[ii], gbyte);
            remove (fname);
            gbyte--;
        }
    }
    return 0;
}        
