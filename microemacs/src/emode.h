/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * emode.h - Define interface to the modes.
 *
 * Copyright (C) 1998-2009 JASSPA (www.jasspa.com)
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
 * Created:     Thu Jan 15 1998
 * Synopsis:    Define interface to the modes.
 * Authors:     Steven Phillips
 * Description:
 *      Includes emode.def to create an ordered list of global/buffer modes.
 *
 * Notes:
 *      The modes were originally defined in edef.h.
 *      The list MUST be ordered for the message-line auto-complete to work.
 */

#ifndef __EMODE_H
#define __EMODE_H

/* Expand the mode definitions from the .def file */
#define DEFMODE(modnam,strnam,chrnam) modnam,
enum
{
#include "emode.def"
    MDNUMMODES
};
#undef DEFMODE

#define meMODE_BYTE_SIZE (MDNUMMODES+7)/8

#define meModeCopy(md,ms)     ((md) = (ms))
#define meModeMask(mn)        (1 << (mn))
#define meModeTest(mb,mn)     ((mb) & meModeMask(mn))
#define meModeSet(mb,mn)      ((mb) |= meModeMask(mn))
#define meModeClear(mb,mn)    ((mb) &= ~(meModeMask(mn)))
#define meModeToggle(mb,mn)   ((mb) ^= meModeMask(mn))

typedef meUInt meMode ;


extern meMode globMode ;                /* global editor mode		*/
extern meMode modeLineDraw ;
extern meUByte *modeName[] ;		/* name of modes		*/
extern meUByte  modeCode[] ;		/* letters to represent modes	*/


#ifdef	INC_MODE_DEF

#define DEFMODE(modnam,strnam,chrnam) (meUByte *)strnam,
meUByte *modeName[] = {                  /* name of modes                */
#include "emode.def"
    NULL
};
#undef DEFMODE

#define DEFMODE(modnam,strnam,chrnam) chrnam,
meUByte modeCode[] =
{
#include "emode.def"
    '\0'
};
#undef DEFMODE

meMode globMode = {
    meModeMask(MDEXACT)  |
#ifndef _NANOEMACS
    meModeMask(MDAUTOSV) |
    meModeMask(MDBACKUP) |
#endif
    meModeMask(MDCR)     |
#ifdef _WIN32
    meModeMask(MDLF)     |
#endif /* _WIN32 */
#ifdef _DOS
    meModeMask(MDLF)     |
    meModeMask(MDCTRLZ)  |
#endif /* _DOS */
    meModeMask(MDFENCE)  |
    meModeMask(MDMAGIC)  |
    meModeMask(MDTAB)    |
    meModeMask(MDUNDO)
} ;

meMode modeLineDraw = { 
    meModeMask(MDAUTOSV) |
    meModeMask(MDBACKUP) |
    meModeMask(MDBINARY) |
    meModeMask(MDCRYPT)  |
    meModeMask(MDEXACT)  |
    meModeMask(MDINDENT) |
    meModeMask(MDJUST)   |
    meModeMask(MDLOCK)   |
    meModeMask(MDMAGIC)  |
    meModeMask(MDNARROW) |
    meModeMask(MDOVER)   |
    meModeMask(MDRBIN)   |
    meModeMask(MDTAB)    |
    meModeMask(MDTIME)   |
    meModeMask(MDUNDO)   |
    meModeMask(MDVIEW)   |
    meModeMask(MDWRAP)
} ;

#endif /* INC_MODE_DEF */

#endif /* __EMODE_H */
