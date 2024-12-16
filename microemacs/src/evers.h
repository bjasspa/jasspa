/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * evers.h - The build version definition.
 *
 * Copyright (C) 1992-2024 JASSPA (www.jasspa.com)
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
 *     These are put in here so that you wont have to grub around in the depths
 *     of display.c & main.c etc every time you want to remind yourself that
 *     you've hacked emacs by updating the version number.
 *
 * Notes:
 *     The definitions have been moved out of estruct.h, because, if you change
 *     them and you have written your makefile properly, everything gets recompiled.
 *     
 *     The history was removed from this file as CVS and the help pages are used
 *     instead.
 */
#define DEF_QUOTE(a)  #a
#define DEF_TO_STR(a) DEF_QUOTE(a)

/* The values of meCENTURY meYEAR, meMONTH, meDAY definitions have been moved to
 * evers.mak so they can be included and used by makefiles */

#define meCENTURY DEF_TO_STR(meVER_CN)
#define meYEAR    DEF_TO_STR(meVER_YR)
#if meVER_MN < 10
#define meMONTH   "0" DEF_TO_STR(meVER_MN)
#else
#define meMONTH   DEF_TO_STR(meVER_MN)
#endif
#if meVER_DY < 10
#define meDAY     "0" DEF_TO_STR(meVER_DY)
#else
#define meDAY     DEF_TO_STR(meVER_DY)
#endif

/* Version information - Date of build */
#define	meDATE          meYEAR "-" meMONTH "-" meDAY 
/* Version information - Major version number is the year of build */
#define	meVERSION       meYEAR 
/* Version information - Minor version number is the month of build */
#define meVERSION_MINOR meMONTH
/* Version information - Version as a numeric date code */
#define meVERSION_CODE  meCENTURY meYEAR meMONTH meDAY 
/* Version information - String as shown in about & -V */
#define meVERSION_INFO  ME_FULLNAME " " meVERSION " - Date " meCENTURY meDATE " - " meSYSTEM_NAME "\n"

/* The program names - these values are also used in the rc files */
#define ME_MICROEMACS_FULLNAME  "MicroEmacs"
#define ME_MICROEMACS_SHORTNAME "me"
#define ME_NANOEMACS_FULLNAME   "NanoEmacs"
#define ME_NANOEMACS_SHORTNAME  "ne"

#define ME_COMPANY_NAME         "JASSPA"
#define ME_COMPANY_SITE         "www.jasspa.com"
