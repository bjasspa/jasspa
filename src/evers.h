/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * evers.h - The build version definition.
 *
 * Copyright (C) 1992-2002 JASSPA (www.jasspa.com)
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
 * Created:     1st May 1992
 * Synopsis:    The build version definition.
 * Authors:     Jon Green & Steven Phillips
 * Description:
 *     This file contains ENAME, what you want MicroEMACS to call itself
 *     on the display line, and the version number VERSION.
 *
 *     These are put in here so that you wont have to grub around in the depths
 *     of display.c & main.c etc every time you want to remind yourself that
 *     you've hacked emacs by updating the version number.
 *
 * Notes:
 *     The definitions have been moved out of estruct.h, because, if you change
 *     them and you have written your makefile properly, everything gets recompiled
 *     whereas putting them here only makes "make" recompile display.c
 * 
 *     The history was removed from this file as CVS and the help pages are used
 *     instead.
 */

/* The name that appears every where */
#define	ENAME           "MicroEmacs"

/* THESE MAY ONLY BE MODIFIED BY JASSPA THEY REPRESENT THE CURRENT 
 * RELEASE OF THE BUILD AND ARE USED AS A REFERENCE POINT WHEN
 * PROBLEMS OCCUR. THESE VALUES ARE NOT INTENDED TO REPRESENT 
 * THE BUILD DATE OF THE SOFTWARE AND MUST NOT BE MODIFIED AS SUCH */
#define meCENTURY       "20"          /* Current century. Y2 complient :-) */
#define meYEAR          "02"          /* Current year */
#define meMONTH         "01"          /* Current month */
#define meDAY           "01"          /* Day of the month */

/* Version information - Date of build */
#define	meDATE          meYEAR "/" meMONTH "/" meDAY 
/* Version information - Major version number is the year of build */
#define	meVERSION       meYEAR 
/* Version information - Minor version number is the month of build */
#define meVERSION_MINOR meMONTH
/* Version information - Version as a numeric date code */
#define meVERSION_CODE  meCENTURY meYEAR meMONTH meDAY 

