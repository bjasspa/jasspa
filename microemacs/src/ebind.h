/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * ebind.h - initial key to command binding definitions.
 *
 * Copyright (C) 1988-2009 JASSPA (www.jasspa.com)
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
 * Created:     For MicroEMACS 3.7
 * Synopsis:    initial key to command binding definitions.
 * Authors:     Unknown, Jon Green & Steven Phillips
 * Description:
 *     Includes ebind.def to create an ordered list of initial
 *     key binding definitions.
 */
#ifdef maindef

#define	DEFBIND(x,y,z)	{(meUShort)(x), (meUShort)(z), (meUInt)(y)},

meBind  keytab[meBIND_MAX] = {

#include	"ebind.def"
    
    { ME_INVALID_KEY, 0, 0 }

};

#undef	DEFBIND

#endif /* maindef */
