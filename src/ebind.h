/*
 *	SCCS:		%W%		%G%		%U%
 *
 *      File          : ebind.h
 *	Last Modified :	<010218.2255>
 *      Description   : Initial default key to function bindings for 
 *                      MicroEMACS 3.7
 * 
 ****************************************************************************
 * 
 * Modifications to the original file by Jasspa. 
 * 
 * Copyright (C) 1988 - 1999, JASSPA 
 * The MicroEmacs Jasspa distribution can be copied and distributed freely for
 * any non-commercial purposes. The MicroEmacs Jasspa Distribution can only be
 * incoportated into commercial software with the expressed permission of
 * JASSPA.
 * 
 ****************************************************************************/

#ifdef maindef

/*
 * Command table.
 * This table  is *roughly* in ASCII order, left to right across the
 * characters of the command. This expains the funny location of the
 * control-X commands.
 */
#define	DEFBIND(x,y,z)	{(uint16)(x), (uint16)(z), (uint32)(y)},

KEYTAB  keytab[NBINDS] = {

#include	"ebind.def"
    
    { ME_INVALID_KEY, 0, 0 }

};

#undef	DEFBIND

#endif /* maindef */
