/*****************************************************************************
 *	SCCS:		%W%		%G%		%U%
 *
 *	Last Modified :	<001005.0107>
 *
 *****************************************************************************
 * 
 * This file contains ENAME, what you want MicroEMACS to call itself on the
 * display line, and the version number VERSION.
 *
 * These are put in here so that you wont have to grub around in the depths
 * of display.c every time you want to remind yourself that you've hacked
 * emacs by updating the version number.
 *
 * The definitions have been moved out of estruct.h, because, if you change
 * them and you have written your makefile properly, everything gets recompiled
 * whereas putting them here only makes "make" recompile display.c
 * 
 * v3.8e14 1st May 1992 - JDG
 * 
 * v3.8e15 11th May 1992 - JDG
 * 
 * Added toggle for left and right justification
 * 
 * v3.8e16 20th March 1994 - JDG
 * 
 * v3.8e16.1 21st March 1994 - JDG
 * 
 * Added control of the bell and the sleep delay on fence matching
 * 
 * v3.8e16.2 4th April 1994 - SWP
 * 
 * added save-kill, and buffer list functions. Debugged and enhanced
 * ISearch routines. Made file search search .\ first.
 * Debugged multiple window for one file exiting problem.
 * Address lowsey file name routines ie using lower case, '\\' not '/'
 * added multiple file loading on entry. 
 * renamed "control-X prefix" stuff to "1st prefix" (ctl1) and added 2
 * more. Stopped most mlwrites will loading emacs.rc, added -d option
 * for debugging. 
 * Added linekill variable to change ^K to del whole line if at start
 * Added letterkill variable to not add del chars to kill buffer when 1.
 * improved delete-word so it doesnt delete half the file.
 * 
 * v3.8e16.3  16th April 1994 - SWP
 * 
 * Re-wrote the file loading routines so they are much simplar and
 * faster.
 * Added history to getstring with routines to mainulate it and save it.
 * made routines for getting strings, buffers, commands, files etc all use
 * getstring (via mlreply).
 * enhanced getstring including new file, buffer and command name completion
 * made mldisp use search.c's strexpend.
 * Added backup creation facility used by variable backups
 * added functions read-history, save-history, next-section, prev-section,
 * quit-emacs, transpose-lines
 * 
 * v3.8e16.4 7th June 1994 - SWP
 * 
 * Made it true ANSII with only warnings for unsed variables and Fint calls
 * Improved the buffer list commands so they are just s,d,x,1 & 2
 * added edit continuation -b option, using the history.
 * Improved required buffer guessing for getBufferName.
 * Found the major instibility caused by zotbuf and many other procedures
 *    zotbuf (and others) abused curbp so it was not equal to curwp.
 * this caused many problems leading ultimately to death.
 * Broken zotbuf, readin and writeout (and maybe others) away from using
 * curbp, no need to do so on these lower level routes. This has made them
 * more usable and stops the need for the bogus (curbp=bp) commands.
 * All system buffers (buffer list, command, help etc) use wpopup command
 * which does not split again and again thank god.
 * 
 * v3.8e16.5  20th September 1994 - SWP 
 * 
 * Added grep and compile functions.
 * Added define-macro which is the same as the previous macro definition
 * except a name is given and the function is created. This means that there
 * are no macros set aside (40 before) and there is no limitation.
 * added name-last-kbd-macro execute-string and insert-macro so can 
 * now create kbd-macro functions.
 * Improved the new c-mode. Now seems to work.
 * Made all file names dynamically allocated. There is still a 1kb limitation
 * on input as mlgetstring etc, still use stack arrays, it is then dynamically
 * allocated.
 * Added better colour support with a hi-light colour support for the dot 
 * lines. Also added to the _TCAPFONT, the main window text font can be defined
 * which for vt100's give the same colouring ability, and the bold font is
 * used for the hi-lighting.
 * Added file name completion and typahead for unix, added mode name 
 * completion. Did a lot more but can't remember it all.
 * 
 * v3.8e16.6  3rd November 1994 - SWP
 * v3.8e16.7  3rd November 1994 - SWP
 * v3.8e16.8  3rd April 1995 - SWP
 * 97.3       12th February 1997 -  SWP
 * 97.6       30th June 1997  - JDG
 * 98.9       6th September 1998 - SWP/JDG
 * 
 * This was the first release of this build to the internet at geocities. We had
 * expected to do this a year ago, however it has taken us a year to sort out 
 * all of the documentation. In addition we added loads of new features and
 * it never really got quite finished up until now. The last 3 months has been 
 * hell trying to get everything up to some sort of reasonable standard. 
 * 
 * 98.10      10th October 1998 - SWP 
 * 
 * Patch of minor problems to the internet. A few fundemntal ones. JDG added
 * fonts fix in windows, can now use non-DOS fonts.
 * 
 ****************************************************************************
 * 
 * Modifications to the original file by Jasspa. 
 * 
 * Copyright (C) 1988-2000, JASSPA 
 * The MicroEmacs Jasspa distribution can be copied and distributed freely for
 * any non-commercial purposes. The MicroEmacs Jasspa Distribution can only be
 * incorporated into commercial software with the expressed permission of
 * JASSPA.
 * 
 ****************************************************************************/

/* The name that appears every where */
#define	ENAME		"MicroEmacs"

/* THESE MAY ONLY BE MODIFIED BY JASSPA THEY REPRESENT THE CURRENT 
 * RELEASE OF THE BUILD AND ARE USED AS A REFERENCE POINT WHEN
 * PROBLEMS OCCUR. THESE VALUES ARE NOT INTENDED TO REPRESENT 
 * THE BUILD DATE OF THE SOFTWARE AND MUST NOT BE MODIFIED AS SUCH */
#define meCENTURY "20"                  /* Current century. Y2 complient :-) */
#define meYEAR    "00"                  /* Current year */
#define meMONTH   "08"                  /* Current month */
#define meDAY     "15"                  /* Day of the month */

/* Version information - Date of build */
#define	meDATE                  meYEAR "/" meMONTH "/" meDAY 
/* Version information - Major version number is the year of build */
#define	meVERSION               meYEAR 
/* Version information - Minor version number is the month of build */
#define meVERSION_MINOR         meMONTH
