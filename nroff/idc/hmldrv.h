/****************************************************************************
 *
 *  			Copyright 1996 Jon Green.
 *                         All Rights Reserved
 *
 *
 *  System        : 
 *  Module        : 
 *  Object Name   : $RCSfile: hmldrv.h,v $
 *  Revision      : $Revision: 1.1 $
 *  Date          : $Date: 2000-10-21 14:31:22 $
 *  Author        : $Author: jon $
 *  Last Modified : <250896.1704>
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
 *  Copyright (c) 1996 Jon Green.
 * 
 *  All Rights Reserved.
 * 
 * This  Document  may  not, in  whole  or in  part, be  copied,  photocopied,
 * reproduced,  translated,  or  reduced to any  electronic  medium or machine
 * readable form without prior written consent from Jon Green.
 *
 ****************************************************************************/

#ifndef __HMLDVR_H__
#define __HMLDVR_H__

#include "hml.h"

extern FILE *fpHml;                     /* File pointer */
extern Args hmlQuickStrs;               /* Quick hypertext strings */
extern char *hmlIdent;                  /* Identity comment string for file */
extern char *hmlGenid;                  /* Generator inforation for file */
extern char *hmlHomePage;               /* Name of the external home page */

extern char *hmlMakeTopicHyperName (char *head, char *name, char *id);
extern int  hmlCodeTidy (char **cp);

#endif /* __HMLDVR_H__ */
