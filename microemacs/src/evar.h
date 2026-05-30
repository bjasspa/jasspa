/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * evar.h - Variable, function and derivative definitions.
 *
 * Copyright (C) 1988-2024 JASSPA (www.jasspa.com)
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
 * Created:     1986
 * Synopsis:    Variable, function and derivative definitions.
 * Authors:     Daniel Lawrence, Jon Green & Steven Phillips
 * Description:
 *     Includes evar.def to create ordered lists of all system variables,
 *     macro functions and derivatives.
 * Notes:
 *     The lists MUST be alphabetically order as a binary chop look-up
 *     algorithm is used and the message line auto-complete relies on this.
 */


#define	DEFVAR(s,v)	v,
#define	DEFFUN(v,s,h,t)
#define	DEFDER(v,s,t)

enum	{
#include	"evar.def"
    NEVARS			/* Number of variables */
} ;	
#undef	DEFVAR
#undef	DEFFUN
#undef	DEFDER


#define	DEFVAR(s,v)
#define	DEFFUN(v,s,h,t)	v,
#define	DEFDER(v,s,t)

enum	{
    NFNULL,
#include	"evar.def"
} ;	
#undef	DEFVAR
#undef	DEFFUN
#undef	DEFDER


#define	DEFVAR(s,v)
#define	DEFFUN(v,s,h,t)
#define	DEFDER(v,s,t)   v,

enum	{
#include	"evar.def"
    NDERIV			/* Number of derivatives */
} ;	

#undef	DEFVAR
#undef	DEFFUN
#undef	DEFDER


#ifdef __EXECC


/**	list of recognized environment variables	*/

#define	DEFFUN(v,s,h,t)
#define	DEFDER(v,s,t)
#define	DEFVAR(s,v)	(meUByte *) s,

meUByte *envars[] =
{
#include	"evar.def"
    0
};

#undef	DEFVAR
#define	DEFVAR(s,v)

/**	list of recognized user function names	*/

#undef	DEFFUN
#define	DEFFUN(v,s,h,t)	h,

int funcHashs[] =
{
    0x0000000,
#include	"evar.def"
    0x1000000
};
/* This array can be tested using the !test directive, enable KEY_TEST in exec.c */
meUByte funcOffst[26] = {
#if MEOPT_EXTENDED
    2,7,12,15,16,21,26,0,28,0,34,39,45,49,52,54,0,57,64,71,74,0,76,79,0,0
#else
    1,6,10,11,12,0,14,0,15,0,0,18,21,24,26,0,0,27,29,0,0,0,0,0,0,0
#endif
};

/**	list of recognized user function types	*/
#undef	DEFFUN
#define	DEFFUN(v,s,h,t)	t,

meUByte funcTypes[] =
{
    0,
#include	"evar.def"
};

#undef	DEFFUN
#define	DEFFUN(v,s,h,t)	(meUByte *) s,

meUByte *funcNames[] =
{
    NULL,
#include	"evar.def"
    NULL
};

#undef	DEFFUN
#define	DEFFUN(v,s,h,t)

/**	list of user directive types	*/
#define DRFLAG_ASGLEXECLVL 0x001
#define DRFLAG_ADBLEXECLVL 0x002
#define DRFLAG_AMSKEXECLVL 0x003
#define DRFLAG_SSGLEXECLVL 0x004
#define DRFLAG_SDBLEXECLVL 0x008
#define DRFLAG_SMSKEXECLVL 0x00c
#define DRFLAG_SWITCH      0x010
#define DRFLAG_TEST        0x020
#define DRFLAG_ARG         0x040
#define DRFLAG_OPTARG      0x080
#define DRFLAG_NARG        0x100
#define DRFLAG_JUMP        0x200

#define DRTESTFAIL   DRFLAG_OPTARG
#define DRUNTILF     (DRUNTIL|DRTESTFAIL)
#define DRWHILEF     (DRWHILE|DRTESTFAIL)
#define DRLOOPMAX    6

#undef	DEFDER
#define	DEFDER(v,s,t)	t,

int dirTypes[] =
{
#include	"evar.def"
};

#undef	DEFVAR
#undef	DEFFUN
#undef	DEFDER

#endif /* maindef */
