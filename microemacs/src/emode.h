/****************************************************************************
 *
 *			Copyright 2000 Jasspa
 *
 *  System        : MicroEmacs Jasspa Distribution
 *  Object Name   : emode.h
 *  Created By    : Steven Phillips
 *  Created       : Thu Jan 15 20:16:54 2000
 *  Last Modified : <011114.1151>
 *
 *  Description
 *       Define interface to the modes
 *
 *  Notes
 *
 *  History
 *
 ****************************************************************************
 *
 * Copyright (C) 1998 - 2000, JASSPA
 * The MicroEmacs Jasspa distribution can be copied and distributed freely for
 * any non-commercial purposes. The MicroEmacs Jasspa Distribution can only be
 * incorporated into commercial software with the expressed permission of
 * JASSPA.
 *
 ****************************************************************************/

#ifndef __EMODE_H
#define __EMODE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

/* Expand the mode definitions from the .def file */
#define DEFMODE(varnam,strnam,chrnam,masklbl,maskval) varnam,
enum
{
#include "emode.def"
    MDNUMMODES
};
#undef DEFMODE

#define MDNUMBYTES (MDNUMMODES+7)/8

#define meModeCopy(md,ms)     (memcpy((md),(ms),sizeof(meMODE)))
#define meModeTest(mb,flag)   ((mb)[(flag) >> 3] &   (1 << ((flag) & 0x07)))
#define meModeSet(mb,flag)    ((mb)[(flag) >> 3] |=  (1 << ((flag) & 0x07)))
#define meModeClear(mb,flag)  ((mb)[(flag) >> 3] &= ~(1 << ((flag) & 0x07)))
#define meModeToggle(mb,flag) ((mb)[(flag) >> 3] ^=  (1 << ((flag) & 0x07)))

typedef uint8 meMODE[MDNUMBYTES] ;


extern meMODE globMode ;                /* global editor mode		*/
extern meMODE modeLineDraw ;
extern uint8 *modeName[] ;		/* name of modes		*/
extern uint8  modeCode[] ;		/* letters to represent modes	*/


#ifdef	INC_MODE_DEF

#define DEFMODE(varnam,strnam,chrnam,masklbl,maskval) (uint8 *)strnam,
uint8 *modeName[] = {                  /* name of modes                */
#include "emode.def"
    NULL
};
#undef DEFMODE

#define DEFMODE(varnam,strnam,chrnam,masklbl,maskval) chrnam,
uint8 modeCode[] =
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

meMODE globMode =
#ifdef _WIN32
{ MDAUTO_MASK|MDATSV_MASK|MDBACK_MASK|MDCRLF_MASK, MDEXACT_MASK|MDFENCE_MASK, MDMAGIC_MASK, MDQUIET_MASK|MDTAB_MASK|MDUNDO_MASK, 0 } ;
#else
#ifdef _DOS
{ MDAUTO_MASK|MDATSV_MASK|MDBACK_MASK|MDCRLF_MASK|MDCTRLZ_MASK, MDEXACT_MASK|MDFENCE_MASK, MDMAGIC_MASK, MDQUIET_MASK|MDTAB_MASK|MDUNDO_MASK, 0 } ;
#else
{ MDAUTO_MASK|MDATSV_MASK|MDBACK_MASK, MDEXACT_MASK|MDFENCE_MASK, MDMAGIC_MASK, MDQUIET_MASK|MDTAB_MASK|MDUNDO_MASK, 0 } ;
#endif /* _DOS */
#endif /* _WIN32 */

meMODE modeLineDraw = { 
    MDAUTO_MASK|MDATSV_MASK|MDBACK_MASK|MDBINRY_MASK|MDCMOD_MASK|MDCRYPT_MASK,
    MDDIR_MASK|MDEXACT_MASK|MDINDEN_MASK|MDJUST_MASK,
    MDMAGIC_MASK|MDNRRW_MASK|MDOVER_MASK,
    MDRBIN_MASK|MDTAB_MASK|MDTIME_MASK|MDUNDO_MASK|MDUSR1_MASK|MDUSR2_MASK,
    MDUSR3_MASK|MDUSR4_MASK|MDUSR5_MASK|MDUSR6_MASK|MDUSR7_MASK|MDUSR8_MASK|MDVIEW_MASK|MDWRAP_MASK
} ;

#endif /* INC_MODE_DEF */

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif
#endif /* __EMODE_H */
