/* -*- C -*- ****************************************************************
 *
 *  			Copyright 1999 Jasspa.
 *			      All Rights Reserved
 *
 *
 *  System        : Microsoft Window's private data structures.
 *  Module        : MicroEmacs.
 *
 *  File Name     : 
 *  Revision      : 1.0
 *  Author        : Jon Green
 *  Created       : Mon May 24 14:12:57 1999
 *  Last Modified : <000220.1429>
 *
 *  Description	  : Export shared definitions of the windows specific
 *                  environment with the different windows specific source
 *                  files.
 *
 *  Notes
 *
 *  History
 *	
 ****************************************************************************
 *
 * Copyright (c) 1999 Jasspa.
 * 
 * All Rights Reserved.
 * 
 * This  document  may  not, in  whole  or in  part, be  copied,  photocopied,
 * reproduced,  translated,  or  reduced to any  electronic  medium or machine
 * readable form without prior written consent from Jasspa.
 * 
 * Mailto: jasspa@geocities.com
 *
 ****************************************************************************/

#ifndef __WINTERM_H__
#define __WINTERM_H__

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

/* Font type settings */
extern LOGFONT ttlogfont;               /* Current logical font */

/* The metrics associated with a charcter cell */
typedef struct
{
    int sizeX;                          /* Character cell size in X width */
    int sizeY;                          /* Character cell size in Y (height) */
    int midX;                           /* Mid position of the cell */
    int midY;                           /* Mid position of the cell */
}  CharMetrics;

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif
#endif /* __WINTERM_H__ */

