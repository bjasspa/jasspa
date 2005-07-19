/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * emode.h - Define interface to the modes.
 *
 * Copyright (C) 1998-2005 JASSPA (www.jasspa.com)
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
#define DEFMODE(varnam,strnam,chrnam,masklbl,maskval) varnam,
enum
{
#include "emode.def"
    MDNUMMODES
};
#undef DEFMODE

#define meMODE_BYTE_SIZE (MDNUMMODES+7)/8

#define meModeCopy(md,ms)     (memcpy((md),(ms),sizeof(meMode)))
#define meModeTest(mb,flag)   ((mb)[(flag) >> 3] &   (1 << ((flag) & 0x07)))
#define meModeSet(mb,flag)    ((mb)[(flag) >> 3] |=  (1 << ((flag) & 0x07)))
#define meModeClear(mb,flag)  ((mb)[(flag) >> 3] &= ~(1 << ((flag) & 0x07)))
#define meModeToggle(mb,flag) ((mb)[(flag) >> 3] ^=  (1 << ((flag) & 0x07)))

typedef meUByte meMode[meMODE_BYTE_SIZE] ;


extern meMode globMode ;                /* global editor mode		*/
extern meMode modeLineDraw ;
extern meUByte *modeName[] ;		/* name of modes		*/
extern meUByte  modeCode[] ;		/* letters to represent modes	*/


#ifdef	INC_MODE_DEF

#define DEFMODE(varnam,strnam,chrnam,masklbl,maskval) (meUByte *)strnam,
meUByte *modeName[] = {                  /* name of modes                */
#include "emode.def"
    NULL
};
#undef DEFMODE

#define DEFMODE(varnam,strnam,chrnam,masklbl,maskval) chrnam,
meUByte modeCode[] =
{
#include "emode.def"
};
#undef DEFMODE

#define DEFMODE(varnam,strnam,chrnam,masklbl,maskval) masklbl=maskval,
enum
{
#include "emode.def"
} ;
#undef DEFMODE

meMode globMode = {
    /* Byte [0] default mask */
#ifndef _NANOEMACS
    MDATSV_MASK|MDBACK_MASK|
#endif
#ifdef _WIN32
    MDCRLF_MASK|
#endif /* _WIN32 */
#ifdef _DOS
    MDCRLF_MASK|MDCTRLZ_MASK|
#endif /* _DOS */
    MDAUTO_MASK,
    
    /* Byte [1] default mask */
    MDEXACT_MASK|MDFENCE_MASK,
    
    /* Byte [2] default mask */
    MDMAGIC_MASK|MDQUIET_MASK,
    
    /* Byte [3] default mask */
    MDTAB_MASK|MDUNDO_MASK,
    
    /* Byte [4] default mask */
    0
} ;

meMode modeLineDraw = { 
    /* Byte [0] */
    MDAUTO_MASK|MDATSV_MASK|MDBACK_MASK|MDBINRY_MASK|MDCRYPT_MASK,
    /* Byte [1] */
    MDDIR_MASK|MDEXACT_MASK|MDINDEN_MASK|MDJUST_MASK,
    /* Byte [2] */
    MDMAGIC_MASK|MDNRRW_MASK|MDOVER_MASK|MDRBIN_MASK,
    /* Byte [3] */
    MDTAB_MASK|MDTIME_MASK|MDUNDO_MASK|MDUSR1_MASK|MDUSR2_MASK|MDUSR3_MASK|MDUSR4_MASK|MDUSR5_MASK,
    /* Byte [4] */
    MDUSR6_MASK|MDUSR7_MASK|MDUSR8_MASK|MDVIEW_MASK|MDWRAP_MASK
} ;

#endif /* INC_MODE_DEF */

#endif /* __EMODE_H */
