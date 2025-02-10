/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * efunc.h - Command name defintions.
 *
 * Copyright (C) 1988-2023 JASSPA (www.jasspa.com)
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
 * Created:     Unknown
 * Synopsis:    Command name defintions.
 * Authors:     Unknown, Jon Green & Steven Phillips
 * Description:
 *     Includes efunc.def to create a list of built in MicroEmacs commands
 *     within a structure which stores command variables and flags on how
 *     the command effects things like the display and region etc.
 *     A hash table is also created & initialized which is used for rapid
 *     command-name -> function lookups.
 *
 * Notes:
 *     Adding a new command requires:
 *       * Adding external declaration to eextern.h
 *       * adding new entry to the table given in efunc.def giving the
 *         command name (1st column), function (3rd) & ebind label (4th)
 *       * Determining the command flag (2nd column)
 *       * Correcting the table list pointers (5th Column)
 *       * Adding the command to the built in hash table (see below & 6th)
 *     To check this has been done correctly set KEY_TEST to 1 in exec.c
 *     and execute the !test macro directive.
 *
 *     The generated list of commands MUST be alphabetically ordered.
 */

#define DEFFUNC(s,t,k,f,r,n,h)  r,

enum
{
#include        "efunc.def"
    CK_MAX
};

#undef  DEFFUNC

extern int     cmdTableSize;            /* The current size of the command table */
extern meCommand   __cmdArray[];            /* Array of internal commands   */
extern meCommand  *__cmdTable[];            /* initial command table        */
extern meCommand **cmdTable;                /* command table                */
extern meCommand  *cmdHead;                 /* command alpha list head      */
extern meUByte commandFlag[] ;          /* selection hilight cmd flags  */
#if MEOPT_CMDHASH
#define cmdHashSize 1024
extern meCommand  *cmdHash[cmdHashSize];    /* command hash lookup table    */
#endif

#define comIgnore     0xff              /* Ignore this command wrt state & hilighting */
#define comSelStart   0x01              /* Start hilighting - remove fix and make active */
#define comSelStop    0x02              /* If active but not fixed then stop, make inactive */
#define comSelKill    0x04              /* Selection kill - remove active and fix */
#define comSelSetDot  0x08              /* Set a new dot position */
#define comSelSetMark 0x10              /* Set a new mark position */
#define comSelSetFix  0x20              /* Fix the selection region */
#define comSelDelFix  0x40              /* Stop hilighting if fixed, does not remove the fix */
#define comSelIgnFail 0x80              /* Don't stop even on a fail */

#define isComIgnore(ix)     (commandFlag[(ix)] == comIgnore)
#define isComSelStart(ix)   (commandFlag[(ix)] & comSelStart)
#define isComSelStop(ix)    (commandFlag[(ix)] & comSelStop)
#define isComSelKill(ix)    (commandFlag[(ix)] & comSelKill)
#define isComSelSetDot(ix)  (commandFlag[(ix)] & comSelSetDot)
#define isComSelSetMark(ix) (commandFlag[(ix)] & comSelSetMark)
#define isComSelSetFix(ix)  (commandFlag[(ix)] & comSelSetFix)
#define isComSelDelFix(ix)  (commandFlag[(ix)] & comSelDelFix)
#define isComSelIgnFail(ix) (commandFlag[(ix)] & comSelIgnFail)

#define getCommandFunc(i)    ((i<CK_MAX) ? cmdTable[i]->func:NULL)
#define getCommandName(i)    (cmdTable[i]->name)
#define getMacro(i)          ((meMacro *) cmdTable[i])
#define getMacroLine(i)      (((meMacro *) cmdTable[i])->hlp)

/*---   If we are in the main program then set up the Name binding table. */

#ifdef  maindef

#define CMDTABLEINITSIZE (((CK_MAX>>5)+1)<<5)

#define DEFFUNC(s,t,k,f,r,n,h)  t,

meUByte commandFlag[] =
{
#include        "efunc.def"
};
#undef  DEFFUNC

#if MEOPT_EXTENDED
#if MEOPT_CMDHASH
#define DEFFUNC(s,t,k,f,r,n,h)          {(meCommand *) n, (meCommand *) h, (meUByte *)s, f, (meVariable *) 0, k, r},
#else
#define DEFFUNC(s,t,k,f,r,n,h)          {(meCommand *) n, (meUByte *)s, f, (meVariable *) 0, r},
#endif
#else
#if MEOPT_CMDHASH
#define DEFFUNC(s,t,k,f,r,n,h)          {(meCommand *) n, (meCommand *) h, (meUByte *)s, f, k, r},
#else
#define DEFFUNC(s,t,k,f,r,n,h)          {(meCommand *) n, (meUByte *)s, f, r},
#endif
#endif

meCommand __cmdArray[CK_MAX] =
{
#include        "efunc.def"
};
#undef  DEFFUNC

int cmdTableSize = CK_MAX ;

#define DEFFUNC(s,t,k,f,r,n,h)          &__cmdArray[r],

meCommand *__cmdTable[CMDTABLEINITSIZE] =
{
#include        "efunc.def"
    NULL
};
#undef  DEFFUNC

/* initialize the command name lookup hash table.
 * This is horrid but greatly improves macro language performance without
 * increasing start-up time.
 */
meCommand **cmdTable = __cmdTable ;
meCommand  *cmdHead = &__cmdArray[0] ;

#if MEOPT_CMDHASH
meCommand  *cmdHash[cmdHashSize] =
{
NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     &__cmdArray[CK_VIWFIL],   NULL,                     
NULL,                     &__cmdArray[CK_HUNBAK],   NULL,                     NULL,                     &__cmdArray[CK_INSSPC],   NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     
NULL,                     &__cmdArray[CK_SAVREGY],  NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     &__cmdArray[CK_LOWWRD],   &__cmdArray[CK_HELPVAR],  NULL,                     
NULL,                     &__cmdArray[CK_GOFNC],    NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     &__cmdArray[CK_MOVLWND],  
NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     
NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     
NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     &__cmdArray[CK_BAKSRCH],  NULL,                     NULL,                     NULL,                     
NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     &__cmdArray[CK_BUFMOD],   NULL,                     NULL,                     NULL,                     
NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     
NULL,                     NULL,                     NULL,                     &__cmdArray[CK_SWNDV],    NULL,                     NULL,                     &__cmdArray[CK_SPLLWRD],  NULL,                     NULL,                     NULL,                     
NULL,                     NULL,                     NULL,                     &__cmdArray[CK_FNDREGY],  NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     
NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     &__cmdArray[CK_BAKLIN],   NULL,                     NULL,                     &__cmdArray[CK_NXTFRAME], 
NULL,                     &__cmdArray[CK_DELREGY],  NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     &__cmdArray[CK_SFNDFIL],  NULL,                     &__cmdArray[CK_MOVUWND],  
NULL,                     &__cmdArray[CK_QREP],     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     
NULL,                     NULL,                     NULL,                     NULL,                     &__cmdArray[CK_EXIT],     NULL,                     &__cmdArray[CK_NEWLIN],   NULL,                     &__cmdArray[CK_FLHOOK],   NULL,                     
NULL,                     NULL,                     &__cmdArray[CK_FORSRCH],  NULL,                     &__cmdArray[CK_ONLYWND],  &__cmdArray[CK_GOTOPOS],  NULL,                     NULL,                     NULL,                     NULL,                     
NULL,                     NULL,                     &__cmdArray[CK_REDREGY],  NULL,                     &__cmdArray[CK_QEXIT],    NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     
NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     &__cmdArray[CK_APROPS],   NULL,                     NULL,                     NULL,                     
NULL,                     NULL,                     &__cmdArray[CK_LSTBUF],   NULL,                     &__cmdArray[CK_SUSPEND],  NULL,                     &__cmdArray[CK_MOVRWND],  NULL,                     NULL,                     NULL,                     
NULL,                     NULL,                     NULL,                     &__cmdArray[CK_FISRCH],   NULL,                     NULL,                     &__cmdArray[CK_SYSTEM],   NULL,                     NULL,                     &__cmdArray[CK_SSWM],     
NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     &__cmdArray[CK_SORTLNS],  &__cmdArray[CK_MRKREGY],  
NULL,                     NULL,                     &__cmdArray[CK_PRTBUF],   NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     &__cmdArray[CK_DELBUF],   NULL,                     
NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     &__cmdArray[CK_GOAMRK],   NULL,                     
NULL,                     NULL,                     NULL,                     NULL,                     &__cmdArray[CK_CHGFONT],  NULL,                     NULL,                     &__cmdArray[CK_SWNDH],    NULL,                     NULL,                     
NULL,                     NULL,                     NULL,                     NULL,                     &__cmdArray[CK_ENDMAC],   NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     
NULL,                     &__cmdArray[CK_ADDRULE],  NULL,                     &__cmdArray[CK_KILREG],   NULL,                     NULL,                     NULL,                     NULL,                     &__cmdArray[CK_GOBOP],    NULL,                     
NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     
NULL,                     NULL,                     NULL,                     NULL,                     &__cmdArray[CK_SVDICTS],  &__cmdArray[CK_YANK],     NULL,                     NULL,                     NULL,                     &__cmdArray[CK_ADDCOLSCHM],                               
NULL,                     &__cmdArray[CK_QUOTE],    NULL,                     NULL,                     NULL,                     NULL,                     &__cmdArray[CK_FORWRD],   &__cmdArray[CK_CMPBUF],   NULL,                     NULL,                     
NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     &__cmdArray[CK_OSDUNBD],  NULL,                     NULL,                     
&__cmdArray[CK_CPYREG],   NULL,                     &__cmdArray[CK_KILRECT],  NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     
NULL,                     NULL,                     &__cmdArray[CK_GOEOP],    NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     &__cmdArray[CK_PRTRGN],   NULL,                     
NULL,                     NULL,                     &__cmdArray[CK_INSFIL],   NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     &__cmdArray[CK_USEBUF],   NULL,                     
NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     &__cmdArray[CK_EXENCMD],  &__cmdArray[CK_SCLPRV],   NULL,                     
NULL,                     NULL,                     NULL,                     NULL,                     &__cmdArray[CK_SPWCLI],   &__cmdArray[CK_INSSTR],   NULL,                     NULL,                     NULL,                     NULL,                     
NULL,                     NULL,                     &__cmdArray[CK_GOEOF],    NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     
NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     
&__cmdArray[CK_DIRTREE],  NULL,                     NULL,                     &__cmdArray[CK_YANKRECT], &__cmdArray[CK_SAVBUF],   NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     
NULL,                     NULL,                     NULL,                     &__cmdArray[CK_EXABBREV], NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     
NULL,                     NULL,                     NULL,                     &__cmdArray[CK_REDFILE],  NULL,                     &__cmdArray[CK_BISRCH],   NULL,                     NULL,                     NULL,                     NULL,                     
NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     &__cmdArray[CK_UNDO],     NULL,                     NULL,                     NULL,                     
NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     &__cmdArray[CK_APPBUF],   &__cmdArray[CK_FORCHR],   &__cmdArray[CK_CMDWAIT],  NULL,                     NULL,                     
NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     
NULL,                     NULL,                     &__cmdArray[CK_DELBLK],   NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     
NULL,                     NULL,                     NULL,                     &__cmdArray[CK_ABOUT],    &__cmdArray[CK_DESCBIN],  NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     
&__cmdArray[CK_LSTREGY],  NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     &__cmdArray[CK_EXECMD],   NULL,                     NULL,                     NULL,                     
&__cmdArray[CK_FILEOP],   NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     &__cmdArray[CK_MOVDWND],  NULL,                     
NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     &__cmdArray[CK_SRCHBUF],  NULL,                     NULL,                     
NULL,                     &__cmdArray[CK_KILEOL],   NULL,                     NULL,                     &__cmdArray[CK_SETMRK],   NULL,                     &__cmdArray[CK_OSDBIND],  &__cmdArray[CK_TRNLINE],  NULL,                     NULL,                     
NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     &__cmdArray[CK_INSFLNM],  &__cmdArray[CK_BINDKEY],  NULL,                     NULL,                     
NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     
&__cmdArray[CK_NAMMAC],   NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     &__cmdArray[CK_QUIT],     NULL,                     
NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     &__cmdArray[CK_SETPOS],   NULL,                     NULL,                     NULL,                     
NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     &__cmdArray[CK_DELFWRD],  &__cmdArray[CK_RECENT],   NULL,                     NULL,                     NULL,                     
NULL,                     &__cmdArray[CK_GLOBABBREV],                               NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     
&__cmdArray[CK_RCASHI],   NULL,                     NULL,                     NULL,                     &__cmdArray[CK_DEFMAC],   NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     
NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     
&__cmdArray[CK_EXEBUF],   NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     &__cmdArray[CK_STRREP],   NULL,                     NULL,                     &__cmdArray[CK_WRPWRD],   
NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     
&__cmdArray[CK_DELSBUF],  NULL,                     &__cmdArray[CK_UPDATE],   NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     &__cmdArray[CK_DOTAB],    
NULL,                     &__cmdArray[CK_MLBIND],   NULL,                     &__cmdArray[CK_NAMBUF],   &__cmdArray[CK_MLUNBD],   &__cmdArray[CK_CTOMOUSE], NULL,                     NULL,                     NULL,                     &__cmdArray[CK_CHGFIL],   
NULL,                     NULL,                     NULL,                     &__cmdArray[CK_HELPITM],  NULL,                     NULL,                     &__cmdArray[CK_GOBOL],    NULL,                     &__cmdArray[CK_FILPAR],   NULL,                     
NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     
&__cmdArray[CK_DELDICT],  NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     &__cmdArray[CK_PIPCMD],   
NULL,                     NULL,                     NULL,                     &__cmdArray[CK_RCASLOW],  &__cmdArray[CK_TRNSKEY],  NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     
NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     &__cmdArray[CK_KILPAR],   &__cmdArray[CK_RESIZEALL],NULL,                     &__cmdArray[CK_SHOWSEL],  NULL,                     
NULL,                     &__cmdArray[CK_XLATEBUF], NULL,                     NULL,                     NULL,                     NULL,                     &__cmdArray[CK_SETCRY],   NULL,                     NULL,                     NULL,                     
NULL,                     NULL,                     NULL,                     &__cmdArray[CK_DEFHELP],  NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     
&__cmdArray[CK_BAKWRD],   &__cmdArray[CK_HILIGHT],  NULL,                     NULL,                     NULL,                     &__cmdArray[CK_FILTBUF],  NULL,                     NULL,                     NULL,                     &__cmdArray[CK_WRTMSG],   
NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     &__cmdArray[CK_DEFFMAC],  NULL,                     NULL,                     NULL,                     NULL,                     
&__cmdArray[CK_GENHASH],  NULL,                     NULL,                     &__cmdArray[CK_SETVAR],   &__cmdArray[CK_FRMDPTH],  NULL,                     &__cmdArray[CK_UNSET],    NULL,                     NULL,                     NULL,                     
NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     &__cmdArray[CK_FRMWDTH],  NULL,                     &__cmdArray[CK_WDWDPTH],  &__cmdArray[CK_FNDFIL],   NULL,                     
NULL,                     NULL,                     &__cmdArray[CK_VOIDFUNC], &__cmdArray[CK_BUFPOS],   NULL,                     NULL,                     NULL,                     &__cmdArray[CK_EXESTR],   &__cmdArray[CK_WDWWDTH],  NULL,                     
&__cmdArray[CK_GOBOF],    NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     
NULL,                     &__cmdArray[CK_SCLNXT],   &__cmdArray[CK_NXTBUF],   NULL,                     NULL,                     NULL,                     &__cmdArray[CK_FORLIN],   NULL,                     &__cmdArray[CK_ADDDICT],  NULL,                     
NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     &__cmdArray[CK_OPNLIN],   NULL,                     NULL,                     NULL,                     
NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     &__cmdArray[CK_HIWRD],    NULL,                     NULL,                     NULL,                     
NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     
NULL,                     NULL,                     &__cmdArray[CK_GOLIN],    &__cmdArray[CK_CAPWRD],   NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     &__cmdArray[CK_ADDCOL],   
NULL,                     &__cmdArray[CK_SETAMRK],  NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     &__cmdArray[CK_NBUFMOD],  NULL,                     
NULL,                     NULL,                     NULL,                     &__cmdArray[CK_GLOBMOD],  NULL,                     NULL,                     &__cmdArray[CK_BUFABBREV],NULL,                     &__cmdArray[CK_IPIPCMD],  NULL,                     
&__cmdArray[CK_BAKCHR],   &__cmdArray[CK_BGNMAC],   NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     
NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     &__cmdArray[CK_SFNDBUF],  NULL,                     NULL,                     NULL,                     NULL,                     
NULL,                     NULL,                     &__cmdArray[CK_EXEFIL],   &__cmdArray[CK_HUNFOR],   &__cmdArray[CK_LSTVAR],   NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     
NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     
&__cmdArray[CK_WRTBUF],   NULL,                     NULL,                     NULL,                     NULL,                     &__cmdArray[CK_DESCKEY],  NULL,                     NULL,                     NULL,                     NULL,                     
NULL,                     &__cmdArray[CK_TRNCHR],   NULL,                     NULL,                     &__cmdArray[CK_NRRWBUF],  NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     
&__cmdArray[CK_OSD],      NULL,                     NULL,                     &__cmdArray[CK_HELP],     &__cmdArray[CK_ADNXTLN],  NULL,                     NULL,                     NULL,                     &__cmdArray[CK_GOEOL],    NULL,                     
&__cmdArray[CK_IPIPKLL],  NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     &__cmdArray[CK_PPPWND],   
NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     
NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     
NULL,                     &__cmdArray[CK_IPIPWRT],  NULL,                     NULL,                     &__cmdArray[CK_SETREGY],  &__cmdArray[CK_MCRQURY],  NULL,                     NULL,                     NULL,                     NULL,                     
NULL,                     NULL,                     &__cmdArray[CK_LCLBIND],  NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     
&__cmdArray[CK_INDENT],   NULL,                     NULL,                     &__cmdArray[CK_WRDCNT],   NULL,                     NULL,                     NULL,                     NULL,                     &__cmdArray[CK_REYANK],   NULL,                     
NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     
NULL,                     NULL,                     NULL,                     &__cmdArray[CK_INSTAB],   NULL,                     &__cmdArray[CK_LSTCOM],   NULL,                     NULL,                     NULL,                     NULL,                     
NULL,                     NULL,                     &__cmdArray[CK_BAKWND],   NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     
&__cmdArray[CK_FNDTAG],   NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     &__cmdArray[CK_SWPMRK],   NULL,                     
NULL,                     &__cmdArray[CK_HELPCOM],  NULL,                     NULL,                     &__cmdArray[CK_DELWND],   NULL,                     NULL,                     NULL,                     NULL,                     &__cmdArray[CK_LCLUNBD],  
NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     &__cmdArray[CK_DELTAB],   NULL,                     NULL,                     NULL,                     NULL,                     
NULL,                     NULL,                     &__cmdArray[CK_DELBAK],   &__cmdArray[CK_DELFRAME], &__cmdArray[CK_SHOWCUR],  NULL,                     NULL,                     &__cmdArray[CK_PRTSCHM],  &__cmdArray[CK_STCHRMK],  NULL,                     
NULL,                     NULL,                     &__cmdArray[CK_UBNDKEY],  NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     NULL,                     
&__cmdArray[CK_PKSCRN],   NULL,                     NULL,                     NULL,
} ;
#endif

#endif
