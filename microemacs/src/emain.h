/****************************************************************************
 *
 *                      Copyright 1997 Jasspa
 *
 *  System        : MicroEmacs Jasspa Distribution
 *  Object Name   : emain.h
 *  Author        : Steve Philips
 *  Created       : Thu Nov 27 19:17:17 1997
 *  Last Modified : <010307.2208>
 *
 *  Description 
 *       Encapsulate all of the platform definitions into a sigle file
 *
 *  Notes
 *       Macro definitions defined within this file obey the
 *       following rules:-
 * 
 *       _XXXX - Underscore names are tested using
 *               "#if (defined _XXXX)", and are disabled by
 *               using "#undef _XXXX" following the definition
 *               to remove or disable.
 *
 *       XXXX  - Non-underscore names are tested using 
 *               "#if XXXXX". A zero disables the option.
 *               Non-zero (typically 1) enables the option.
 *
 *  History
 *      
 ****************************************************************************
 *
 * Copyright (C) 1997 - 1999, JASSPA 
 * 
 * The MicroEmacs Jasspa distribution can be copied and distributed freely for
 * any non-commercial purposes. The MicroEmacs Jasspa Distribution can only be
 * incorporated into commercial software with the expressed permission of
 * JASSPA.
 *  
 ****************************************************************************/
#ifndef __EMAIN_H__
#define __EMAIN_H__

/* These next 2 defines are platform specific, but as all supported 
 * platforms use these and all future ones should I've put them here
 * for now.
 */
#define _STDARG 1       /* Use <stdarg.h> in defintions                 */
#define _ANSI_C 1       /* ANSI_C to be used                            */

/* MicroEmacs Configuration options */
#define TYPEAH  1       /* type ahead causes update to be skipped       */
#define DEBUGM  1       /* enable macro debugging $debug                */
#define TIMSTMP 1       /* Enable time stamping of files on write.      */

#define COLOR   1       /* color commands and windows                   */
#define HILIGHT 1       /* color hilighting                             */

#define ISRCH   1       /* Incremental searches like ITS EMACS          */
#define WORDPRO 1       /* Advanced word processing features            */
#define CFENCE  1       /* fench matching in CMODE                      */
#define CRYPT   1       /* file encryption enabled?                     */
#define MAGIC   1       /* include regular expression matching?         */
#define ABREV   1       /* Abreviated files                             */
#define FLNEXT  1       /* include the file next buffer stuff           */
#define DORCS   1       /* do rcs check ins and outs                    */
#define MOUSE   1       /* do mouse pointer stuff                       */
#define LCLBIND 1       /* do local key binding (buffer)                */
#define PRINT   1       /* do printing of buffers and regions           */
#define SPELL   1       /* do spelling of words and buffers             */
#define MEUNDO  1       /* undo capability                              */
#define URLAWAR 1       /* understands url file names (may not read em) */
#define NARROW  1       /* enable narrowing functionality               */
#define REGSTRY 1       /* enable registry functionality                */
#define MEOSD   1       /* enable OSD functionality                     */

/* One may think that this should be platform dependant, BUT time and
 * experience has shown that it is best to always use a '/' in the body
 * of MicroEmacs (simplifies everything) and convert to or from the
 * system DIR Cchar if required. No system yet needs this but
 * as some need to check and convert to a '/' they define a CONVDIR_CHAR
 */
#define DIR_CHAR  '/'   /* directory divide char, ie /bin/ */

#ifdef _ANSI_C
#define APRAM(x)        x
#else
#define APRAM(x)        ()
#endif

/**************************************************************************
* UNIX : IRIX                                                             *
**************************************************************************/
#if (defined _IRIX5) || (defined _IRIX6)
#define meSYSTEM_NAME  "irix"           /* Identity name of the system   */
#define _IRIX          1                /* This is an irix box           */
#define _UNIX          1                /* This is a UNIX system         */
#define _USG           1                /* UNIX system V                 */
#define _DIRENT        1                /* Use <dirent.h> for directory  */
#define _TERMIOS       1                /* Use termios, not termio       */
#define _XTERM         1                /* Use Xlib                      */
#define _TCAP          1                /* Use TERMCAP                   */
#define _TCAPFONT      1                /* Use TERMCAP fonts to color    */
#define _CLIENTSERVER  1                /* Client server support         */
#define _URLSUPP       1                /* Supports url reading          */
#define _IPIPES        1                /* platform supports Inc. pipes  */
#define _POSIX_SIGNALS 1                /* use POSIX signals             */
/* WE DO YET NOT KNOW IF THIS IS PRESENT SO DISABLE */
#define _NOTPARM       1                /* No tparm (termcap) support    */
#endif

/**************************************************************************
* UNIX : HPUX                                                             *
**************************************************************************/
#if (defined _HPUX9) || (defined _HPUX10)
#define meSYSTEM_NAME "hpux"            /* Identity name of the system   */
#define _HPUX          1                /* This is a hpux box            */
#define _UNIX          1                /* This is a UNIX system         */
#define _USG           1                /* UNIX system V                 */
/*#define _TERMIOS       1*/            /* Use termios, not termio       */
#define _XTERM         1                /* Use Xlib                      */
#define _TCAP          1                /* Use TERMCAP                   */
#define _TCAPFONT      1                /* Use TERMCAP fonts to color    */
#define _CLIENTSERVER  1                /* Client server support         */
#define _URLSUPP       1                /* Supports url reading          */
#define _IPIPES        1                /* platform supports Inc. pipes  */
#define _POSIX_SIGNALS 1                /* use POSIX signals             */
#define _meDEF_SYS_ERRLIST              /* errno.h not def sys_errlist   */
/* WE DO YET NOT KNOW IF THIS IS PRESENT SO DISABLE */
#define _NOTPARM       1                /* No tparm (termcap) support    */
#endif

/**************************************************************************
* UNIX : UNIXWARE                                                         *
**************************************************************************/
#ifdef _UNIXWR1 
#define meSYSTEM_NAME  "unixwr1"        /* Identity name of the system   */
#define _UNIX          1                /* This is a UNIX system         */
#define _USG           1                /* UNIX system V                 */
#define _DIRENT        1                /* Use <dirent.h> for directory  */
#define _TERMIOS       1                /* Use termios, not termio       */
#define _XTERM         1                /* Use Xlib                      */
#define _TCAP          1                /* Use TERMCAP                   */
#define _TCAPFONT      1                /* Use TERMCAP fonts to color    */
#define _CLIENTSERVER  1                /* Client server support         */
#define _URLSUPP       1                /* Supports url reading          */
#define _IPIPES        1                /* platform supports Inc. pipes  */
#define _POSIX_SIGNALS 1                /* use POSIX signals             */
/* WE DO YET NOT KNOW IF THIS IS PRESENT SO DISABLE */
#define _NOTPARM       1                /* No tparm (termcap) support    */
#endif

/**************************************************************************
* UNIX : SunOS Sparc + Intel                                              *
**************************************************************************/
/* _SUNOS5? == Sparc Solaris; _SUNOS_X86 == i86 Solaris */
#if (defined _SUNOS55) || (defined _SUNOS56) || (defined _SUNOS_X86)
#define meSYSTEM_NAME  "sunos"          /* Identity name of the system   */
#define _SUNOS         1                /* This is a sunos box           */
#define _UNIX          1                /* This is a UNIX system         */
#define _USG           1                /* UNIX system V                 */
#define _DIRENT        1                /* Use <dirent.h> for directory  */
#define _TERMIOS       1                /* Use termios, not termio       */
#define _XTERM         1                /* Use Xlib                      */
#define _TCAP          1                /* Use TERMCAP                   */
#define _TCAPFONT      1                /* Use TERMCAP fonts to color    */
#define _CLIENTSERVER  1                /* Client server support         */
#define _URLSUPP       1                /* Supports url reading          */
#define _IPIPES        1                /* platform supports Inc. pipes  */
#define _POSIX_SIGNALS 1                /* use POSIX signals             */
#define _meDEF_SYS_ERRLIST              /* errno.h not def sys_errlist   */
#endif

/**************************************************************************
* UNIX : FreeBSD                                                          *
**************************************************************************/
#ifdef _FREEBSD
#define meSYSTEM_NAME  "freebsd"        /* Identity name of the system   */
#define _UNIX          1                /* This is a UNIX system         */
#define _USG           1                /* UNIX system V                 */
#define _DIRENT        1                /* Use <dirent.h> for directory  */
#define _TERMIOS       1                /* Use termios, not termio       */
#define _XTERM         1                /* Use Xlib                      */
#define _TCAP          1                /* Use TERMCAP                   */
#define _TCAPFONT      1                /* Use TERMCAP fonts to color    */
#define _CLIENTSERVER  1                /* Client server support         */
#define _URLSUPP       1                /* Supports url reading          */
#define _IPIPES        1                /* platform supports Inc. pipes  */
#define _POSIX_SIGNALS 1                /* use POSIX signals             */
#endif

/**************************************************************************
* UNIX : AIX                                                              *
**************************************************************************/
#ifdef _AIX
#define meSYSTEM_NAME  "aix"            /* Identity name of the system   */
#define _UNIX          1                /* This is a UNIX system         */
#define _USG           1                /* UNIX system V                 */
#define _DIRENT        1                /* Use <dirent.h> for directory  */
#define _TERMIOS       1                /* Use termios, not termio       */
#define _XTERM         1                /* Use Xlib                      */
#define _TCAP          1                /* Use TERMCAP                   */
#define _TCAPFONT      1                /* Use TERMCAP fonts to color    */
#define _CLIENTSERVER  1                /* Client server support         */
#define _URLSUPP       1                /* Supports url reading          */
#define _IPIPES        1                /* platform supports Inc. pipes  */
#define _POSIX_SIGNALS 1                /* use POSIX signals             */
#define _meDEF_SYS_ERRLIST              /* errno.h doesnt def sys_errlist*/
/* WE DO YET NOT KNOW IF THIS IS PRESENT SO DISABLE */
#define _NOTPARM       1                /* No tparm (termcap) support    */
#endif

/**************************************************************************
* UNIX : Linux                                                            *
**************************************************************************/
#ifdef _LINUX
#define meSYSTEM_NAME  "linux"          /* Identity name of the system   */
#define _UNIX          1                /* This is a UNIX system         */
#define _USG           1                /* UNIX system V                 */
#define _DIRENT        1                /* Use <dirent.h> for directory  */
#define _TERMIOS       1                /* Use termios, not termio       */
#define _XTERM         1                /* Use Xlib                      */
#define _TCAP          1                /* Use TERMCAP                   */
#define _TCAPFONT      1                /* Use TERMCAP fonts to color    */
#define _CLIENTSERVER  1                /* Client server support         */
#define _URLSUPP       1                /* Supports url reading          */
#define _IPIPES        1                /* platform supports Inc. pipes  */
#define _POSIX_SIGNALS 1                /* use POSIX signals             */
#endif

/**************************************************************************
* UNIX : Cyqwin on Windows                                                *
**************************************************************************/
#ifdef _CYGWIN
#define meSYSTEM_NAME  "cygwin"         /* Identity name of the system   */
#define _UNIX          1                /* This is a UNIX system         */
#define _USG           1                /* UNIX system V                 */
#define _DIRENT        1                /* Use <dirent.h> for directory  */
#define _TERMIOS       1                /* Use termios, not termio       */
#define _TCAP          1                /* Use TERMCAP                   */
#define _TCAPFONT      1                /* Use TERMCAP fonts to color    */
#undef  _WIN32                          /* This is not win32             */
#define _NO_XTERM      1                /* Do not want X-Windows         */
#define _POSIX_SIGNALS 1                /* use POSIX signals             */
#endif /* _CYGWIN */

/**************************************************************************
* UNIX : OpenStep NexT                                                    *
*                                                                         *
* Tested on Openstep 4.2 / NeXT Motorola 040                              *
**************************************************************************/
#ifdef _NEXT
#define meSYSTEM_NAME  "openstep"       /* Identity name of the system   */
#define _UNIX          1                /* This is a UNIX system         */
#define _BSD_43        1                /* This is a BSD 4.3 system      */
#define _BSD_CBREAK    1                /* Use CBREAK or RAW settings    */
#define _TCAP          1                /* Use TERMCAP                   */
#define _TCAPFONT      1                /* Use TERMCAP fonts to color    */
#define _URLSUPP       1                /* Supports url reading          */
#define _IPIPES        1                /* platform supports Inc. pipes  */
#define _NOSTRDUP      1                /* No strdup                     */
#define _NOPUTENV      1                /* No putenv support             */
#define _NOTPARM       1                /* No tparm (termcap) support    */

/* NeXT provides us with libc.h this includes all of the API definitions for
 * the standard 'C' library calls. Nice touch !! */
#include <libc.h>
#endif /* _NEXT */

/**************************************************************************
* Microsoft : DOS                                                         *
**************************************************************************/
#ifdef _DOS
#define meSYSTEM_NAME  "dos"            /* Identity name of the system   */
#define _CTRLZ         1                /* ^Z at end of files (MSDOS)    */
#define _CTRLM         1                /* ^M at end of lines (MSDOS)    */
#define _SINGLE_CASE   1                /* Files only have single case   */
#define _DRV_CHAR     ':'               /* drive divide letter, C:\dos   */
#define _CONVDIR_CHAR '\\'              /* Filename convert '\\' => '/'  */
#endif /* _DOS */

/**************************************************************************
* Microsoft : 32s/'95/'98/NT                                              *
**************************************************************************/
#ifdef _WIN32s
#define _WIN32         1                /* Use win32                    */
#endif
#ifdef _WIN32
#define meSYSTEM_NAME  "win32"          /* Identity name of the system  */
#define WIN32          1                /* Standard win32 definition    */
#define _CTRLZ         1                /* ^Z at end of files (MSDOS)   */
#define _CTRLM         1                /* ^M at end of lines (MSDOS)   */
#if (_MSC_VER != 900)
/* MSVC Not version 2 - win32s */
#define _IPIPES        1                /* platform supports Inc. pipes */
#define _CLIENTSERVER  1                /* Client server support        */
#endif
/*#define _URLSUPP  1*/                 /* Supports url reading         */
#define _CLIPBRD       1                /* Inter window clip board supp */
#define _WINDOW        1                /* Windowed, resizing & title   */
#define _INSENSE_CASE  1                /* File names case insensitive  */
#define _DRAGNDROP     1                /* Drag and drop supported.     */
#define _DRV_CHAR     ':'               /* drive divide letter, C:\dos  */
#define _CONVDIR_CHAR '\\'              /* Filename convert '\\' => '/' */
#ifndef _WIN32_FULL_INC
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>                    /* Standard windows API         */
#ifdef _DRAGNDROP
#include <shellapi.h>                   /* Drag and drop API             */
#endif /* _DRAGNDROP */
#endif /* _WIN32 */

/**************************************************************************
* X-Windows Setup - Definitions required for X-Lib support                *
 **************************************************************************/
#ifdef _NO_XTERM                        /* console only mode?            */
#undef _XTERM                           /* Do not want X-Windows         */
#endif
#ifdef _XTERM
#define _CLIPBRD  1             /* Inter window clip board supp          */
#define _WINDOW   1             /* Window, need resizing & title code    */
#include <X11/Intrinsic.h>      /* Intrinsics Definitions                */
#include <X11/StringDefs.h>     /* Standard Name-String definitions      */
#include <X11/Xutil.h>
#else
/* If this is a UNIX terminal then there is no mouse support */
#ifdef _UNIX
#undef  MOUSE
#define MOUSE     0             /* cannot support mouse on termcap       */
#endif /* _UNIX */
#endif /* _XTERM */

/**************************************************************************
* ALL: Standard microemacs includes                                       *
**************************************************************************/
#include <stdio.h>                      /* Always need this              */
#include <stdlib.h>                     /* Usually need this             */
#include <errno.h>                      /* Need errno and sys_errlist    */

/**************************************************************************
* USG[UNIX] : Include definitions. Assumed (near) POSIX compliance        *
**************************************************************************/
#if (defined _USG)
#include <signal.h>                     /* Required for signal(2)        */
#include <sys/stat.h>                   /* Required for stat(2) types    */
#include <unistd.h>
#endif

/**************************************************************************
* BSD[UNIX] : Include definitions (Support for BSD 4.2/3 only)            *
**************************************************************************/
#if ((defined(_BSD_43)) || (defined(_BSD42)))
/* Additional definitions for BSD4.2 and BSD4.3 */
#define _BSD                1           /* This is version of BSD        */
#ifndef _POSIX_SIGNALS
#define _BSD_SIGNALS        1           /* Use BSD Signals               */
#endif  /* _POSIX_SIGNALS */
#endif  /* ((defined(_BSD_43)) || (defined(_BSD42)) */

#ifdef _BSD
#include <signal.h>                     /* Required for signal(2)        */
#include <sys/stat.h>                   /* Required for stat(2) types    */
#include <sys/fcntl.h>                  /* Required for access(2) types  */
#include <sys/types.h>                  /* Required for seclect(2) types */
#include <unistd.h>
#endif

#ifdef _meDEF_SYS_ERRLIST
extern const char *sys_errlist[];
#endif
#ifdef _CYGWIN
#define sys_errlist _sys_errlist        /* Underscored in errno.h !! */
#endif

/* Standard Types */
typedef   signed char  int8 ;
typedef unsigned char  uint8 ;
typedef          short int16 ;
typedef unsigned short uint16 ;
typedef          long  int32 ;
typedef unsigned long  uint32 ;

#include "emode.h"      /* Mode enum, type & var defs    */
#include "estruct.h"    /* Type structure definitions    */
#include "edef.h"       /* Global variable definitions   */
#include "eterm.h"      /* platform terminal definitions */
#include "eextrn.h"     /* External function defintions  */

#endif /* __EMAIN_H__ */

