/*
 *      SCCS:           %W%             %G%             %U%
 *
 *      Last Modified : <010203.2001>
 *
 *      EFUNC.H:        MicroEMACS function declarations and names
 *
 *      This file list all the C code functions used by MicroEMACS
 *      and the names to use to bind keys to them. To add functions,
 *      declare it here in both the extern function list and the name
 *      binding table.
 * 
 ****************************************************************************
 * 
 * Modifications to the original file by Jasspa. 
 * 
 * Copyright (C) 1988 - 1999, JASSPA 
 * The MicroEmacs Jasspa distribution can be copied and distributed freely for
 * any non-commercial purposes. The MicroEmacs Jasspa Distribution can only be
 * incoportated into commercial software with the expressed permission of
 * JASSPA.
 * 
 ****************************************************************************/

/*      Name to function binding table.
        This table gives the names of all the bindable functions end their C
        function address.  These are used for the bind-to-key function. */

/*---   Set up the values of the functions */

#define DEFFUNC(s,t,f,r,v,n,h)  r,

enum
{
#include        "efunc.def"
    CK_MAX
};

#undef  DEFFUNC

#define cmdHashSize 511
extern meCMD  *cmdHash[cmdHashSize];    /* command hash lookup table    */
extern int     cmdTableSize;            /* The current size of the command table */
extern meCMD  *__cmdTable[];            /* initial command table        */
extern meCMD **cmdTable;                /* command table                */
extern meCMD  *cmdHead;                 /* command alpha list head      */
extern uint8   commandFlag[] ;          /* selection hilight cmd flags  */

#define comSelStart   0x01              /* Start hilighting - remove fix and make active */
#define comSelStop    0x02              /* If active but not fixed then stop, make inactive */
#define comSelKill    0x04              /* Selection kill - remove active and fix */
#define comSelSetDot  0x08              /* Set a new dot position */
#define comSelSetMark 0x10              /* Set a new mark position */
#define comSelSetFix  0x20              /* Fix the selection region */
#define comSelDelFix  0x40              /* Stop hilighting if fixed, does not remove the fix */
#define comSelIgnFail 0x80              /* Don't stop even on a fail */

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
#define getMacro(i)          ((meMACRO *) cmdTable[i])
#define getMacroLine(i)      (((meMACRO *) cmdTable[i])->hlp)

/*---   If we are in the main program then set up the Name binding table. */

#ifdef  maindef

#define DEFFUNC(s,t,f,r,v,n,h)  t,

uint8 commandFlag[] = 
{
#include        "efunc.def"
};
#undef  DEFFUNC


#define DEFFUNC(s,t,f,r,v,n,h)          extern meCMD v ;

#include        "efunc.def"

#undef  DEFFUNC

#define DEFFUNC(s,t,f,r,v,n,h)  meCMD v = {(meCMD *) n, (meCMD *) h, { (meVARIABLE *) 0, 0}, (uint8 *)s, r, (Fintii) f} ;

#include        "efunc.def"

#undef  DEFFUNC

#define CMDTABLEINITSIZE (((CK_MAX>>5)+1)<<5)

int cmdTableSize = CK_MAX ;

#define DEFFUNC(s,t,f,r,v,n,h)          & v,

meCMD *__cmdTable[CMDTABLEINITSIZE] = 
{
#include        "efunc.def"
    NULL
};
#undef  DEFFUNC

meCMD **cmdTable = __cmdTable ;
meCMD  *cmdHead = &__meFunc_ABTCMD ;
meCMD  *cmdHash[cmdHashSize] = 
{
    NULL,                 NULL,                 NULL,                 NULL,                 NULL,
    NULL,                 NULL,                 NULL,                 NULL,                 NULL,
    &__meFunc_SWNDV,      NULL,                 NULL,                 NULL,                 &__meFunc_GOBOP,
    NULL,                 NULL,                 &__meFunc_DELWND,     NULL,                 NULL,
    &__meFunc_FNDFIL,     NULL,                 NULL,                 NULL,                 NULL,
    NULL,                 &__meFunc_BAKSRCH,    NULL,                 &__meFunc_SAVREGY,    NULL,
    NULL,                 NULL,                 NULL,                 NULL,                 &__meFunc_RESIZEHORZ,
    NULL,                 &__meFunc_INSMAC,     NULL,                 &__meFunc_UNVARG,     NULL,
    NULL,                 NULL,                 NULL,                 &__meFunc_SVDICTS,    NULL,
    &__meFunc_SAVSBUF,    NULL,                 &__meFunc_EXENCMD,    &__meFunc_INSTAB,     NULL,
    NULL,                 NULL,                 NULL,                 NULL,                 &__meFunc_DELWBAK,
    &__meFunc_REDREGY,    &__meFunc_HIWRD,      &__meFunc_GOFNC,      NULL,                 NULL,
    NULL,                 NULL,                 &__meFunc_CHGDIR,     &__meFunc_SUSPEND,    &__meFunc_QUIT,
    NULL,                 &__meFunc_BUFMOD,     &__meFunc_INSFLNM,    NULL,                 NULL,
    &__meFunc_BAKCHR,     NULL,                 NULL,                 &__meFunc_YANK,       NULL,
    &__meFunc_BUFPOS,     NULL,                 NULL,                 &__meFunc_ONLYWND,    &__meFunc_REDFILE,
    &__meFunc_PRTBUF,     &__meFunc_SWNDH,      &__meFunc_CRTCLBK,    &__meFunc_APPBUF,     NULL,
    NULL,                 &__meFunc_IPIPWRT,    NULL,                 &__meFunc_GOTOPOS,    &__meFunc_SETMRK,
    NULL,                 &__meFunc_SAVBUF,     &__meFunc_ABTCMD,     NULL,                 NULL,
    &__meFunc_FNDTAG,     NULL,                 NULL,                 NULL,                 NULL,
    NULL,                 &__meFunc_DELBLK,     NULL,                 NULL,                 NULL,
    NULL,                 NULL,                 &__meFunc_LOWWRD,     &__meFunc_SSWM,       NULL,
    &__meFunc_WRPWRD,     &__meFunc_ENDMAC,     &__meFunc_SHOWCUR,    NULL,                 NULL,
    &__meFunc_DELBAK,     NULL,                 NULL,                 &__meFunc_TRNLINE,    &__meFunc_STRREP,
    NULL,                 NULL,                 NULL,                 NULL,                 &__meFunc_DEFFMAC,
    NULL,                 NULL,                 &__meFunc_CHGFONT,    NULL,                 &__meFunc_NEWLIN,
    NULL,                 NULL,                 NULL,                 NULL,                 NULL,
    NULL,                 NULL,                 NULL,                 NULL,                 NULL,
    &__meFunc_UPDATE,     NULL,                 &__meFunc_WRTHIST,    NULL,                 NULL,
    NULL,                 &__meFunc_DELSBUF,    &__meFunc_UNDO,       NULL,                 NULL,
    NULL,                 NULL,                 NULL,                 NULL,                 NULL,
    NULL,                 &__meFunc_SCNSIZ,     NULL,                 NULL,                 NULL,
    &__meFunc_GOBOL,      &__meFunc_DELFOR,     NULL,                 &__meFunc_TRNSKEY,    &__meFunc_NAMBUF,
    NULL,                 NULL,                 NULL,                 NULL,                 &__meFunc_ABOUT,
    &__meFunc_LCLBIND,    &__meFunc_REYANK,     NULL,                 NULL,                 NULL,
    &__meFunc_DOTAB,      &__meFunc_GOEOP,      &__meFunc_BAKLIN,     &__meFunc_BISRCH,     &__meFunc_SETCRY,
    NULL,                 &__meFunc_BAKWND,     &__meFunc_WRTMSG,     NULL,                 NULL,
    &__meFunc_PKSCRN,     &__meFunc_GOEOL,      NULL,                 NULL,                 NULL,
    &__meFunc_DELTAB,     &__meFunc_OSDBIND,    NULL,                 NULL,                 &__meFunc_WRTBUF,
    NULL,                 NULL,                 &__meFunc_LCLUNBD,    NULL,                 &__meFunc_DELDICT,
    NULL,                 &__meFunc_BINDKEY,    &__meFunc_FILTBUF,    NULL,                 NULL,
    NULL,                 NULL,                 &__meFunc_OSD,        NULL,                 &__meFunc_GETREGY,
    &__meFunc_FORLIN,     NULL,                 NULL,                 &__meFunc_HILIGHT,    &__meFunc_SORTLNS,
    NULL,                 NULL,                 NULL,                 &__meFunc_KILRECT,    NULL,
    &__meFunc_KILREG,     NULL,                 NULL,                 NULL,                 NULL,
    NULL,                 NULL,                 &__meFunc_SETPOS,     NULL,                 NULL,
    &__meFunc_DEFHELP,    NULL,                 NULL,                 NULL,                 &__meFunc_FLHOOK,
    NULL,                 NULL,                 NULL,                 NULL,                 NULL,
    &__meFunc_PRTCOL,     &__meFunc_UNSET,      &__meFunc_SCLPRV,     NULL,                 NULL,
    &__meFunc_BAKWRD,     NULL,                 NULL,                 NULL,                 NULL,
    NULL,                 NULL,                 &__meFunc_BUFABREV,   NULL,                 NULL,
    NULL,                 &__meFunc_FORSRCH,    NULL,                 NULL,                 &__meFunc_ENGHORZWIN,
    NULL,                 &__meFunc_EXECMD,     NULL,                 NULL,                 NULL,
    &__meFunc_ADDCOLSCHM, NULL,                 NULL,                 NULL,                 NULL,
    NULL,                 NULL,                 &__meFunc_MOVUWND,    NULL,                 &__meFunc_GOBOF,
    NULL,                 &__meFunc_USEBUF,     &__meFunc_GLOBMOD,    NULL,                 NULL,
    NULL,                 NULL,                 NULL,                 NULL,                 NULL,
    NULL,                 NULL,                 NULL,                 &__meFunc_MOVLWND,    &__meFunc_YANKRECT,
    &__meFunc_RCASHI,     NULL,                 &__meFunc_NRRWBUF,    NULL,                 NULL,
    &__meFunc_DESCVAR,    &__meFunc_MCRQURY,    NULL,                 NULL,                 &__meFunc_NBUFMOD,
    NULL,                 &__meFunc_SFNDBUF,    NULL,                 NULL,                 NULL,
    &__meFunc_KILEOL,     &__meFunc_SETVAR,     &__meFunc_REDVWND,    NULL,                 NULL,
    &__meFunc_FORCHR,     NULL,                 NULL,                 NULL,                 &__meFunc_HELPCOM,
    &__meFunc_REDHIST,    &__meFunc_ADDRULE,    NULL,                 NULL,                 &__meFunc_CTOMOUSE,
    &__meFunc_SPWCLI,     NULL,                 NULL,                 NULL,                 &__meFunc_DIRTREE,
    &__meFunc_FNDBFIL,    NULL,                 NULL,                 NULL,                 NULL,
    NULL,                 &__meFunc_SCLNXT,     NULL,                 NULL,                 NULL,
    NULL,                 &__meFunc_SHOWSEL,    &__meFunc_ENGWIN,     NULL,                 NULL,
    &__meFunc_HUNBAK,     &__meFunc_SWPMRK,     NULL,                 NULL,                 NULL,
    NULL,                 &__meFunc_IPIPKLL,    &__meFunc_CMPBUF,     NULL,                 &__meFunc_PPPWND,
    &__meFunc_GOAMRK,     &__meFunc_ADDDICT,    NULL,                 NULL,                 &__meFunc_HUNFOR,
    NULL,                 NULL,                 NULL,                 NULL,                 &__meFunc_DELREGY,
    NULL,                 NULL,                 NULL,                 NULL,                 NULL,
    NULL,                 NULL,                 &__meFunc_GOEOF,      NULL,                 NULL,
    NULL,                 NULL,                 NULL,                 NULL,                 NULL,
    NULL,                 &__meFunc_MOVDWND,    NULL,                 NULL,                 NULL,
    &__meFunc_EXEMAC,     &__meFunc_LSTBUF,     NULL,                 NULL,                 &__meFunc_SPLLWRD,
    NULL,                 NULL,                 &__meFunc_CMDWAIT,    &__meFunc_SYSTEM,     &__meFunc_NXTWND,
    NULL,                 NULL,                 NULL,                 NULL,                 NULL,
    NULL,                 NULL,                 &__meFunc_QUOTE,      NULL,                 NULL,
    NULL,                 &__meFunc_QEXIT,      NULL,                 NULL,                 NULL,
    NULL,                 &__meFunc_INSSPC,     NULL,                 &__meFunc_CLRMSG,     NULL,
    NULL,                 NULL,                 NULL,                 NULL,                 NULL,
    NULL,                 NULL,                 &__meFunc_LSTCOM,     &__meFunc_DELFWRD,    NULL,
    NULL,                 &__meFunc_PIPCMD,     &__meFunc_QREP,       &__meFunc_HELP,       NULL,
    NULL,                 NULL,                 &__meFunc_MRKREGY,    &__meFunc_RCASLOW,    NULL,
    &__meFunc_EXESTR,     NULL,                 NULL,                 NULL,                 NULL,
    &__meFunc_CPYREG,     &__meFunc_ADDCOL,     &__meFunc_NAMMAC,     NULL,                 NULL,
    &__meFunc_OPNLIN,     NULL,                 NULL,                 NULL,                 &__meFunc_DELBUF,
    NULL,                 NULL,                 &__meFunc_MOVRWND,    NULL,                 NULL,
    NULL,                 NULL,                 NULL,                 &__meFunc_FNDCFIL,    NULL,
    &__meFunc_INSSTR,     NULL,                 &__meFunc_EXABREV,    NULL,                 &__meFunc_EXEBUF,
    NULL,                 NULL,                 &__meFunc_DEFMAC,     &__meFunc_MLBIND,     NULL,
    &__meFunc_ADNXTLN,    NULL,                 NULL,                 NULL,                 &__meFunc_CHGFIL,
    NULL,                 NULL,                 NULL,                 &__meFunc_VIWFIL,     NULL,
    NULL,                 NULL,                 &__meFunc_RESIZEALL,  NULL,                 NULL,
    &__meFunc_VOIDFUNC,   NULL,                 NULL,                 &__meFunc_FORWRD,     NULL,
    NULL,                 NULL,                 NULL,                 NULL,                 NULL,
    &__meFunc_APROPS,     NULL,                 NULL,                 &__meFunc_SETAMRK,    NULL,
    &__meFunc_EXIT,       &__meFunc_STCHRMK,    NULL,                 &__meFunc_DESCKEY,    NULL,
    &__meFunc_IPIPCMD,    NULL,                 &__meFunc_FISRCH,     &__meFunc_RESIZE,     &__meFunc_SFNDFIL,
    NULL,                 &__meFunc_INSFIL,     NULL,                 NULL,                 NULL,
    NULL
} ;

#endif


