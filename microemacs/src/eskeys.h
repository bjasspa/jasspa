/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * eskeys.h - Extended key definitions.
 *
 * Copyright (C) 1997-2023 JASSPA (www.jasspa.com)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 675 Mass Ave, Cambridge, MA 02139, USA.
 */
/*
 * Created:     Thu Sep 18 1997
 * Synopsis:    Extended key definitions.
 * Authors:     Steven Phillips & Jon Green
 * Description:
 *     Includes eskeys.def to create an ordered list of extended (special)
 *     key names.
 */

#ifndef __ESKEYS_H__
#define __ESKEYS_H__

#define	DEFSKEY(s,i,c,d,t) t,
#define	DEFSSKEY(t,i,c)

enum
{
#include	"eskeys.def"
    SKEY_MAX
};

#undef	DEFSKEY

extern meUByte *specKeyNames[] ;

#ifdef	maindef

#define	DEFSKEY(s,i,c,d,t) (meUByte *) s,

meUByte *specKeyNames[]=
{
#include	"eskeys.def"
};
#undef	DEFSKEY

#endif /* maindef */

#undef	DEFSSKEY

#endif /* __ESKEYS_H__ */

