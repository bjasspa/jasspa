/*
 *	SCCS:		%W%		%G%		%U%
 *
 *	Last Modified :	<000107.1959>
 *
 *	EVAR.H: Environment and user variable definitions
 *		for MicroEMACS
 *
 *		written 1986 by Daniel Lawrence
 *
 * Jon Green	17-05-91
 * Added suffix modes so that the suffix defintions may be defined in the 
 * start up file.
 *
 ****************************************************************************
 * 
 * Modifications to the original file by Jasspa. 
 * 
 * Copyright (C) 1988 - 1999, JASSPA 
 * The MicroEmacs Jasspa distribution can be copied and distributed freely for
 * any non-commercial purposes. The MicroEmacs Jasspa Distribution can only be
 * incorporated into commercial software with the expressed permission of
 * JASSPA.
 * 
 ****************************************************************************/


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

#define	DEFVAR(s,v)	(uint8 *) s,
#define	DEFFUN(v,s,t)
#define	DEFDER(v,s,t)

uint8 *envars[] =
{
#include	"evar.def"
    0
};

#undef	DEFVAR
#undef	DEFFUN
#undef	DEFDER


/**	list of recognized user function names	*/

#define	DEFVAR(s,v)	/* NULL */
#define	DEFFUN(v,s,t)	(uint8 *) s,
#define	DEFDER(v,s,t)

uint8 *funcNames[] =
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

uint8 funcTypes[] =
{
#include	"evar.def"
};

#undef	DEFVAR
#undef	DEFFUN
#undef	DEFDER

/**	list of recognized user directives	*/

#define	DEFVAR(s,v)	/* NULL */
#define	DEFFUN(v,s,t)
#define	DEFDER(v,s,t)	(uint8 *) s,

uint8 *derNames[] =
{
#include	"evar.def"
};

#undef	DEFVAR
#undef	DEFFUN
#undef	DEFDER

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
