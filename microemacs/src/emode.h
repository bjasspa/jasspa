/****************************************************************************
 *
 *			Copyright 2000 Jasspa
 *
 *  System        : MicroEmacs Jasspa Distribution
 *  Object Name   : emode.h
 *  Created By    : Steven Phillips
 *  Created       : Thu Jan 15 20:16:54 2000
 *  Last Modified : <000718.2343>
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
#define DEFMODE(varnam,strnam,chrnam) varnam,
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

meMODE globMode =
#ifdef _WIN32
/* MDAUTO|MDATSV|MDBACK|MDCRLF|MDEXACT|MDMAGIC|MDQUIET|MDTAB|MDUNDO */
{ 0x27, 0x08, 0x84, 0x0a, 0x00 } ;      /* Windows is \r\n              */
#else
#ifdef _DOS
/* MDAUTO|MDATSV|MDBACK|MDCRLF|BEFCTRLZ|MDEXACT|MDMAGIC|MDQUIET|MDTAB|MDUNDO */
{ 0xa7, 0x08, 0x84, 0x0a, 0x00 } ;      /* DOS is \r\n + CTRL-Z         */
#else
/* MDAUTO|MDATSV|MDBACK|MDEXACT|MDMAGIC|MDQUIET|MDTAB|MDUNDO */
{ 0x07, 0x08, 0x84, 0x0a, 0x00 } ;      /* UNIX is \n only              */
#endif /* _DOS */
#endif /* _WIN32 */

meMODE modeLineDraw = { 0x3f, 0x6a, 0x34, 0xfe, 0x3f } ;

#define DEFMODE(varnam,strnam,chrnam) (uint8 *)strnam,
uint8 *modeName[] = {                  /* name of modes                */
#include "emode.def"
    NULL
};
#undef DEFMODE

#define DEFMODE(varnam,strnam,chrnam) chrnam,
uint8 modeCode[] =
{
#include "emode.def"
};
#undef DEFMODE

#endif /* INC_MODE_DEF */

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif
#endif /* __EMODE_H */
