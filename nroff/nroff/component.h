/****************************************************************************
 *
 *			Copyright 1996-2004 Jon Green.
 *                         All Rights Reserved
 *
 *
 *  System        :
 *  Module        :
 *  Object Name   : $RCSfile: component.h,v $
 *  Revision      : $Revision: 1.2 $
 *  Date          : $Date: 2004-01-06 00:53:50 $
 *  Author        : $Author: jon $
 *  Last Modified : <040104.0038>
 *
 *  Description
 *
 *  Notes
 *
 *  History
 *
 ****************************************************************************
 *
 *  Copyright (c) 1996-2004 Jon Green.
 *
 *  All Rights Reserved.
 *
 * This  Document  may  not, in  whole  or in  part, be  copied,  photocopied,
 * reproduced,  translated,  or  reduced to any  electronic  medium or machine
 * readable form without prior written consent from Jon Green.
 *
 ****************************************************************************/

#ifndef __COMPONT_H__
#define __COMPONT_H__

extern char  *componentGetFirst (char *item);
extern char **componentGetAll   (char *item);
extern int    componentCmp      (char *item, char  *ritem);
extern int    componentCmpAll   (char *item, char **ritem);
extern int    componentCmpAllC  (char *item, char  *ritem);

#endif /* __COMPONT_H__ */
