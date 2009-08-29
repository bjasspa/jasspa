/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * evar.h - Variable, function and derivative definitions.
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
#define	DEFFUN(v,s,t)
#define	DEFDER(v,s,t)

enum	{
#include	"evar.def"
    NEVARS			/* Number of variables */
} ;	
#undef	DEFVAR
#undef	DEFFUN
#undef	DEFDER


#define	DEFVAR(s,v)	/* NULL */
#define	DEFFUN(v,s,t)	v,
#define	DEFDER(v,s,t)

enum	{
#include	"evar.def"
    NFUNCS			/* Number of functions */
} ;	
#undef	DEFVAR
#undef	DEFFUN
#undef	DEFDER


#define	DEFVAR(s,v)	/* NULL */
#define	DEFFUN(v,s,t)
#define	DEFDER(v,s,t)   v,

enum	{
#include	"evar.def"
    NDERIV			/* Number of derivatives */
} ;	

#define DRTESTFAIL   0x80
#define DRUNTILF     (DRUNTIL|DRTESTFAIL)

#undef	DEFVAR
#undef	DEFFUN
#undef	DEFDER


#ifdef __EXECC


/**	list of recognized environment variables	*/

#define	DEFVAR(s,v)	(meUByte *) s,
#define	DEFFUN(v,s,t)
#define	DEFDER(v,s,t)

meUByte *envars[] =
{
#include	"evar.def"
    0
};

#undef	DEFVAR
#undef	DEFFUN
#undef	DEFDER


/**	list of recognized user function names	*/

#define	DEFVAR(s,v)	/* NULL */
#define	DEFFUN(v,s,t)	(meUByte *) s,
#define	DEFDER(v,s,t)

meUByte *funcNames[] =
{
#include	"evar.def"
};

#undef	DEFVAR
#undef	DEFFUN
#undef	DEFDER

/**	list of recognized user function types	*/

#define	DEFVAR(s,v)	/* NULL */
#define	DEFFUN(v,s,t)	t,
#define	DEFDER(v,s,t)

meUByte funcTypes[] =
{
#include	"evar.def"
};

#undef	DEFVAR
#undef	DEFFUN
#undef	DEFDER

#if 0
/* list of recognized user directive names
 * 
 * As of ME'04 this list is nolonger used, see docmd in exec.c
 */
#define	DEFVAR(s,v)	/* NULL */
#define	DEFFUN(v,s,t)
#define	DEFDER(v,s,t)	(meUByte *) s,

meUByte *derNames[] =
{
#include	"evar.def"
};

#undef	DEFVAR
#undef	DEFFUN
#undef	DEFDER
#endif
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

#define	DEFVAR(s,v)	/* NULL */
#define	DEFFUN(v,s,t)
#define	DEFDER(v,s,t)	t,

int dirTypes[] =
{
#include	"evar.def"
};

#undef	DEFVAR
#undef	DEFFUN
#undef	DEFDER


#endif /* maindef */
