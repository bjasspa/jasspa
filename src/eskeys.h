/****************************************************************************
 *
 *  			Copyright 1997 Jasspa
 *
 *  System        : MicroEmacs Jasspa Distribution
 *  Object Name   : eskeys.h
 *  Created       : Thu Sep 18 15:28:45 1997
 *  Last Modified : <000718.2344>
 *
 *  Description	
 *      Extended key definitions.
 *
 *  Notes
 *
 *  History
 *	
 ****************************************************************************
 *
 * Copyright (C) 1997-1999, JASSPA 
 * The MicroEmacs Jasspa distribution can be copied and distributed freely for
 * any non-commercial purposes. The MicroEmacs Jasspa Distribution can only be
 * incorporated into commercial software with the expressed permission of
 * JASSPA.
 *  
 ****************************************************************************/

#ifndef __ESKEYS_H__
#define __ESKEYS_H__

#define	DEFSKEY(s,i,d,t) t,

enum
{
#include	"eskeys.def"
    SKEY_MAX
};

#undef	DEFSKEY

extern uint8 *specKeyNames[] ;

#ifdef	maindef

#define	DEFSKEY(s,i,d,t) (uint8 *) s,

uint8 *specKeyNames[]=
{
#include	"eskeys.def"
};
#undef	DEFSKEY

#endif /* maindef */

#endif /* __ESKEYS_H__ */

