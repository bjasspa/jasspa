/* -*- C -*- ****************************************************************
 *
 *  			Copyright 2002 JASSPA.
 *			      All Rights Reserved
 *
 *
 *  System        : 
 *  Module        : 
 *  Object Name   : $RCSfile: eopt.h,v $
 *  Revision      : $Revision: 2.1 $
 *  Date          : $Date: 2002-04-10 22:47:16 $
 *  Author        : $Author: jon $
 *  Created By    : Steven Phillips
 *  Created       : Sat Jan 19 00:45:54 2002
 *  Last Modified : <020315.1033>
 *
 *  Description	
 *
 *  Notes
 *
 *  History
 *	
 *  $Log: not supported by cvs2svn $
 *  Revision 1.1  2002/03/01 21:54:26  Phillips
 *  Major changes to create NanoEmacs!
 *  Made macro getting of keys for binding very strict, no superfluous chars allowed
 *  Changed mlwrite so that when aborting and no !force is used it will write message even with a MWEXCEL
 *  Made printing optionally quiet for JST insert printing
 *  Minor changes to scroll commands to abort when at the top or bottom
 *  Small change to numerical interface to suspend-emacs
 *
 *
 ****************************************************************************
 *
 *  Copyright (c) 2002 JASSPA.
 * 
 *  All Rights Reserved.
 * 
 * This  document  may  not, in  whole  or in  part, be  copied,  photocopied,
 * reproduced,  translated,  or  reduced to any  electronic  medium or machine
 * readable form without prior written consent from JASSPA.
 *
 ****************************************************************************/

#ifndef __EOPT_H
#define __EOPT_H

/* List of available options	*/
meUByte meOptionList[]=
#if MEOPT_ABBREV
    "abb\n"
#endif
#if MEOPT_CALLBACK
    "cal\n"
#endif
#if MEOPT_CFENCE
    "cfe\n"
#endif
#if MEOPT_CLIENTSERVER
    "cli\n"
#endif
#if MEOPT_COLOR
    "col\n"
#endif
#if MEOPT_CRYPT
    "cry\n"
#endif
#if MEOPT_DEBUGM
    "deb\n"
#endif
#if MEOPT_DIRLIST
    "dir\n"
#endif
#if MEOPT_EXTENDED
    "ext\n"
#endif
#if MEOPT_FILEHOOK
    "fho\n"
#endif
#if MEOPT_FRAME
    "fra\n"
#endif
#if MEOPT_CMDHASH
    "has\n"
#endif
#if MEOPT_HILIGHT
    "hil\n"
#endif
#if MEOPT_HSPLIT
    "hsp\n"
#endif
#if MEOPT_IPIPES
    "ipi\n"
#endif
#if MEOPT_ISEARCH
    "ise\n"
#endif
#if MEOPT_LOCALBIND
    "lbi\n"
#endif
#if MEOPT_MAGIC
    "mag\n"
#endif
#if MEOPT_MOUSE
    "mou\n"
#endif
#if MEOPT_MWFRAME
    "mwf\n"
#endif
#if MEOPT_NARROW
    "nar\n"
#endif
#if MEOPT_FILENEXT
    "nex\n"
#endif
#if MEOPT_OSD
    "osd\n"
#endif
#if MEOPT_POKE
    "pok\n"
#endif
#if MEOPT_POSITION
    "pos\n"
#endif
#if MEOPT_PRINT
    "pri\n"
#endif
#if MEOPT_RCS
    "rcs\n"
#endif
#if MEOPT_REGISTRY
    "reg\n"
#endif
#if MEOPT_SCROLL
    "scr\n"
#endif
#if MEOPT_SOCKET
    "soc\n"
#endif
#if MEOPT_SPAWN
    "spa\n"
#endif
#if MEOPT_SPELL
    "spe\n"
#endif
#if MEOPT_TAGS
    "tag\n"
#endif
#if MEOPT_TIMSTMP
    "tim\n"
#endif
#if MEOPT_TYPEAH
    "typ\n"
#endif
#if MEOPT_UNDO
    "und\n"
#endif
#if MEOPT_WORDPRO
    "wor\n"
#endif
;

#endif /* __EOPT_H */
