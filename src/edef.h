/*
 *      SCCS:           %W%             %G%             %U%
 *
 *      Last Modified : <011029.1149>
 *
 *      EDEF:           Global variable definitions for
 *                      MicroEMACS 3.2
 *
 *                      written by Dave G. Conroy
 *                      modified by Steve Wilhite, George Jones
 *                      greatly modified by Daniel Lawrence
 *                      greatly modified again by JASSPA.
 *
 ****************************************************************************
 *
 * Modifications to the original file by Jasspa.
 *
 * Copyright (C) 1988 - 2000, JASSPA
 * The MicroEmacs Jasspa distribution can be copied and distributed freely for
 * any non-commercial purposes. The MicroEmacs Jasspa Distribution can only be
 * incoportated into commercial software with the expressed permission of
 * JASSPA.
 *
 ****************************************************************************/

/* initialized global external declarations */

extern  meDIRLIST       curDirList ;
extern  struct osdDIALOG  *osdDialogHd; /* Root of the on screen displays */
extern  struct osdDISPLAY *osdDisplayHd;/* Menu display list */
extern  meREGISTERS    *meRegHead;      /* The register Head            */
extern  meREGISTERS    *meRegCurr;      /* The current register set     */
extern  SELHILIGHT      selhilight;     /* Selection hilight            */
extern  HILBLOCK       *hilBlock;       /* Hilighting colour changes    */
extern  meCOLOR         noColors ;      /* No defined colours           */
extern  int             styleTableSize; /* Size of the colour table     */
extern  meSTYLE        *styleTable;     /* Highlighting colour table    */
#if ABREV
extern  meABREV        *globalAbrev ;   /* Global Abreviation file      */
#endif
extern  mePOS  *mePosition ;            /* Position stack head          */
extern  uint16  mePositionMark ;        /* Position next alpha mark name*/
extern  uint8  *disLineBuff ;           /* interal display buffer array */
extern  uint16  hilBlockS ;             /* Hilight - HilBlock array siz */
extern  int     disLineSize ;           /* interal display buffer size  */
extern  int     cursorState ;           /* Current state of cursor      */
extern  FRAMELINE       *frameStore;    /* Frame store line pointers    */
extern  uint8  *progName ;              /* the program name (argv[0])   */
extern  uint8 **ModeLineStr ;           /* modeline line format         */
extern  uint8   orgModeLineStr[] ;      /* original modeline line format*/
extern  uint8  *modeLineStr ;           /* current modeline format      */
extern  uint8   modeLineFlags ;         /* current modeline flags       */
extern  int32   autotime;               /* auto save time in seconds    */
extern  int     keptVersions;           /* No. of kept backup versions  */
extern  uint32  delaytime;              /* mouse-time delay time        */
extern  uint32  repeattime;             /* mouse-time repeat time       */
extern  uint32  idletime;               /* idle-time delay time         */
extern  uint8   fillbullet[];           /* Fill bullet character class  */
extern  int16   fillbulletlen;          /* Fill lookahead limit         */
extern  int16   fillcol;                /* Fill column                  */
extern  uint8   filleos[];              /* Fill E-O-S character class   */
extern  int16   filleoslen;             /* Fill E-O-S ' ' insert len    */
extern  uint8   fillignore[];           /* Fill Ignore character class  */
extern  uint16  tabsize;                /* Virtual Tab size             */
extern  uint16  tabwidth;               /* Real TAB size                */
extern  int16   matchlen;               /* Fence matching delay length  */
extern  uint8   *homedir;               /* Home directory               */
extern  uint8   *searchPath;            /* emf search path              */
extern  uint8   *curdir;                /* current working directory    */
extern  uint8   *curFuncName ;          /* Current macro command name   */
extern  uint8   *execstr;               /* pointer to string to execute */
extern  int     execlevel;              /* execution IF level           */
extern  int     bufHistNo;              /* inc buff hist numbering      */
extern  int     lastCommand ;           /* The last user executed key   */
extern  int     lastIndex ;             /* The last user executed comm  */
extern  int     thisCommand ;           /* The cur. user executed key   */
extern  int     thisIndex ;             /* The cur. user executed comm  */
extern  uint8   hexdigits[];
extern  uint16  keyTableSize;           /* The current size of the key table */
extern  KEYTAB  keytab[];               /* key bind to functions table  */
extern  uint8   fillmode;               /* Justify mode                 */
extern  uint8   scrollFlag ;            /* horiz/vert scrolling method  */
extern  uint8   sgarbf;                 /* State of screen unknown      */
extern  uint8   clexec;                 /* command line execution flag  */
extern  uint8   mcStore;                /* storing text to macro flag   */
extern  uint8   cmdstatus;              /* last command status          */
extern  uint8   kbdmode;                /* current keyboard macro mode  */
extern  int8    macbug;                 /* macro debuging flag          */
extern  uint8   thisflag;               /* Flags, this command          */
extern  uint8   lastflag;               /* Flags, last command          */
extern  uint8   lastReplace;            /* set to non-zero if last was a replace */

extern  uint32  meSystemCfg;            /* ME system config variable    */
#define meSYSTEM_CONSOLE    0x000001    /* This is a console version    */
#define meSYSTEM_RGBCOLOR   0x000002    /* System has definable rgb color */
#define meSYSTEM_ANSICOLOR  0x000004    /* Ansi standard color (8 colors) */
#define meSYSTEM_XANSICOLOR 0x000008    /* Extended Ansi colors (16)    */
#define meSYSTEM_FONTS      0x000010    /* Use termcap fonts (bold etc) */
#define meSYSTEM_UNIXSYSTEM 0x000080    /* This is a unix sys           */
#define meSYSTEM_MSSYSTEM   0x000100    /* This is a dos or win32 sys   */
#define meSYSTEM_DRIVES     0x000200    /* This system has drives (C:\) */
#define meSYSTEM_DOSFNAMES  0x000400    /* Dos 8.3 file name restriction*/
#define meSYSTEM_IPIPES     0x000800    /* The system supports ipipes   */
#define meSYSTEM_TABINDANY  0x001000    /* Tab key indents from any pos */
#define meSYSTEM_ALTMENU    0x002000    /* Enable Alt as menu hot key   */
#define meSYSTEM_ALTPRFX1   0x004000    /* Enable Alt as prefix1        */
#define meSYSTEM_KEEPUNDO   0x008000    /* Keep undo history after save */
#define meSYSTEM_FONTFIX    0x010000    /* Enables ANSI 0-31 rendering  */
#define meSYSTEM_CLNTSRVR   0x020000    /* Enables the client server    */
#define meSYSTEM_CTCHASPC   0x040000    /* Enables win32 catch A-space  */
#define meSYSTEM_SHOWWHITE  0x080000    /* Display TAB, CR & space chars*/
#define meSYSTEM_HIDEBCKUP  0x100000    /* Hide backup files            */
#define meSYSTEM_TABINDFST  0x200000    /* Tab key indents first col pos*/

#ifdef _UNIX
#ifdef _CLIENTSERVER
#define meSYSTEM_MASK (meSYSTEM_ANSICOLOR|meSYSTEM_FONTS|meSYSTEM_DOSFNAMES|meSYSTEM_IPIPES|meSYSTEM_TABINDANY|meSYSTEM_ALTMENU|meSYSTEM_ALTPRFX1|meSYSTEM_KEEPUNDO|meSYSTEM_FONTFIX|meSYSTEM_CLNTSRVR|meSYSTEM_SHOWWHITE|meSYSTEM_HIDEBCKUP|meSYSTEM_TABINDFST)
#else
#define meSYSTEM_MASK (meSYSTEM_ANSICOLOR|meSYSTEM_FONTS|meSYSTEM_DOSFNAMES|meSYSTEM_IPIPES|meSYSTEM_TABINDANY|meSYSTEM_ALTMENU|meSYSTEM_ALTPRFX1|meSYSTEM_KEEPUNDO|meSYSTEM_FONTFIX|meSYSTEM_SHOWWHITE|meSYSTEM_HIDEBCKUP|meSYSTEM_TABINDFST)
#endif
#endif
#ifdef _DOS
#define meSYSTEM_MASK (meSYSTEM_TABINDANY|meSYSTEM_ALTMENU|meSYSTEM_ALTPRFX1|meSYSTEM_KEEPUNDO|meSYSTEM_SHOWWHITE|meSYSTEM_HIDEBCKUP|meSYSTEM_TABINDFST)
#endif
#ifdef _WIN32
extern  uint32 meSYSTEM_MASK;           /* ME system mask - dependant on win32 flavour */
#endif

/* the $mouse variables always exist so the C variables must */
#define meMOUSE_NOBUTTONS   0x0000f     /* # mouse buttons              */
#define meMOUSE_ENBLE       0x00010     /* mouse is enabled             */
#define meMOUSE_SWAPBUTTONS 0x00020     /* swap mouse buttons           */
#define meMOUSE_ICON        0xf0000     /* Mask to set the mouse pointer icon */
extern  uint32 meMouseCfg;              /* ME mouse config variable     */
extern  uint8 mouse_pos;                /* mouse virtual position       */
extern  int16 mouse_X;                  /* mouse X pos at last event    */
extern  int16 mouse_Y;                  /* mouse X pos at last event    */
extern  int16 mouse_dX;                 /* mouse delta X last event pos */
extern  int16 mouse_dY;                 /* mouse delta X last event pos */

#if MEUNDO
extern  uint32 undoContFlag;            /* continuation of an undo      */
#endif

/* File Modes */
#if (defined _DOS) || (defined _WIN32)
extern  uint16 meUmask ;                /* file creation umask          */
#endif
#ifdef _UNIX
extern  uint16    meUmask ;             /* file creation umask          */
extern  mode_t    meXUmask ;            /* directory creation umask     */
extern  uid_t     meUid ;               /* current user id              */
extern  gid_t     meGid ;               /* current group id             */
extern  int       meGidSize ;           /* number of groups the user belongs to   */
extern  gid_t    *meGidList ;           /* list of group ids the user belongs to  */
#endif /* _UNIX */

/* Signals */
#ifdef _UNIX
#ifdef _POSIX_SIGNALS
extern  sigset_t  meSigHoldMask ;       /* signals held when spawning and reading */
#endif /* _POSIX_SIGNALS */
#ifdef _BSD_SIGNALS
extern int meSigHoldMask;               /* signals held when spawning and reading */
#endif /* _BSD_SIGNALS */
extern int meSigLevel ;                 /* signal level */
#endif /* _UNIX */

/* Environment Variables */
#if ((defined(_UNIX)) && (defined(_NOPUTENV)))
extern char    **meEnviron;             /* Our own environment */
#endif /* ((defined(_UNIX)) && (defined(_NOPUTENV))) */

#ifdef _IPIPES
/* Incremental pipe variables */
extern meIPIPE  *ipipes ;               /* list of all the current pipes*/
extern int       noIpipes ;             /* count of all the cur pipes   */
#endif

#define meALARM_DIE       0x01
#define meALARM_WINSIZE   0x02
#define meALARM_VARIABLE  0x04
#define meALARM_PIPED     0x10

extern  uint8   alarmState;             /* Auto-save alarm time         */
extern  int32   startTime;              /* me start time. used as offset*/
extern  LINE   *menuLine ;              /* Menu-poke line */
extern  LINE   *lpStore;                /* line off which to store macro*/
extern  BUFFER *lpStoreBp ;             /* help is stored in a buffer   */
extern  uint16  thiskey;                /* the current key              */
extern  int     taboff;                 /* tab offset for display       */
extern  uint16  prefixc[];              /* prefix chars           */
extern  uint16  reptc;                  /* current universal repeat char*/
extern  uint16  breakc;                 /* current abort-command char*/
#ifdef _CLIPBRD
/* When implementing the clipboard it is best if the clipboard is only
 * attempted to be got or set once per command. This is because of other
 * window interaction.
 *
 * If the command is a macro the the following nasty things can happen if this
 * is not so
 *
 * 1) The 1st yank in a macro may return a different value to the 2nd yank, if
 *    the user defines another region in a different window.
 *
 * 2) If the macro is looping round cutting and pasting then the user will now
 *    be able to reliably cut 'n' paste in another window.
 *
 * 3) A macro could cut a region and the paste a different one!
 *
 * Nasty. Try and ensure these don't happen.
 */

#define CLIP_TRY_GET 0x01               /* We can try and get the clipbd*/
#define CLIP_TRY_SET 0x02               /* We can try and set the clipbd*/
#define CLIP_OWNER   0x04               /* We are the owners of clipbrd */
#define CLIP_STALE   0x08               /* Clipboard is stale           */
#define CLIP_RECVED  0x10               /* Clipboard has been obtained  */
extern  uint8   clipState;              /* clipboard status flag        */
#endif
extern  uint32  cursorBlink ;           /* cursor-blink blink time      */
extern  int     blinkState ;            /* cursor blink state           */
#if COLOR
extern  meCOLOR cursorColor;            /* cursor-color scheme          */
extern  meSCHEME osdScheme;             /* Menu line color scheme       */
extern  meSCHEME mlScheme;              /* Message line color scheme    */
extern  meSCHEME mdLnScheme;            /* Mode line color scheme       */
extern  meSCHEME sbarScheme;            /* Scroll bar color scheme      */
extern  meSCHEME globScheme;            /* Global color scheme          */
extern  meSCHEME trncScheme;            /* truncate color scheme        */
extern  int     gsbarmode;              /* global scroll bar mode       */
extern  uint8   boxChars [];            /* Array of box characters      */
extern  uint8   windowChars [];         /* Array of window characters   */
extern  uint8   displayTab;             /* tab \t display character */
extern  uint8   displayNewLine;         /* new-line \n display character */
extern  uint8   displaySpace;           /* space ' ' display character */
#endif

extern  uint8  *envars[];               /* list of recognized env. vars */
extern  uint8  *derNames[];             /* name list of directives      */
extern  uint8   derTypes[];             /* type list of directives      */
extern  uint8  *funcNames[];            /* name list of user funcs      */
extern  uint8   funcTypes[];            /* type list of user funcs      */
extern  KILL   *kbufp;                  /* current kill buffer chunk pointer */
extern  KILL   *kbufh;                  /* kill buffer header pointer   */
extern  uint8   lkbdptr[];              /* Holds last keybd macro data  */
extern  int     lkbdlen;                /* Holds length of last macro   */
extern  uint8  *kbdptr;                 /* start of current keyboard buf*/
extern  int     kbdlen;                 /* len of current macro         */
extern  int     kbdoff;                 /* current offset of macro      */
extern  int     kbdrep;                 /* number of repetitions        */
extern  uint8   emptym[];               /* empty literal                */
extern  uint8   errorm[];               /* error literal                */
extern  uint8   abortm[];               /* abort literal                */
extern  uint8   truem[];                /* true literal         */
extern  uint8   falsem[];               /* false litereal               */
extern  meVARLIST usrVarList;           /* user variables list          */

#if CFENCE
extern int16  statementIndent ;
extern int16  continueIndent ;
extern int16  continueMax ;
extern int16  commentIndent ;
extern int16  switchIndent ;
extern int16  braceIndent ;
extern int16  caseIndent ;
extern int16  commentMargin ;
extern uint8  commentContOrg[] ;
extern uint8 *commentCont ;
#endif

/* global buffer names */
extern uint8  BvariablesN[];
extern uint8  BbindingsN[];
extern uint8  BtranskeyN[];
extern uint8  BcompleteN[];
extern uint8  BcommandsN[];
extern uint8  BicommandN[];
extern uint8  BregistryN[];
extern uint8  BbuffersN[];
extern uint8  BcommandN[];
extern uint8  BolhelpN[];
extern uint8  BserverN[];
extern uint8  BaboutN[];
extern uint8  BstdinN[];
extern uint8  BmainN[];
extern uint8  BhelpN[];

extern uint8 *fileIgnore ;
#if FLNEXT
extern uint8 *flNextFileTemp ;
extern uint8 *flNextLineTemp ;

extern uint8    noNextLine ;
extern uint8  **nextName ;
extern uint8   *nextLineCnt ;
extern uint8 ***nextLineStr ;
#endif

extern uint8   fileHookCount ;
extern uint8 **fileHookExt ;
extern uint8 **fileHookFunc ;
extern int16  *fileHookArg ;

#if DORCS
extern uint8 *rcsFile ;
extern uint8 *rcsCoStr ;
extern uint8 *rcsCoUStr ;
extern uint8 *rcsCiStr ;
extern uint8 *rcsCiFStr ;
extern uint8 *rcsUeStr ;
#endif

/* history variables used in meGetStringFromUser */
extern  uint8 numStrHist ;              /* curent no. of hist. strings    */
extern  uint8 numBuffHist ;             /* curent no. of hist. buff names */
extern  uint8 numCommHist ;             /* curent no. of hist. comm names */
extern  uint8 numFileHist ;             /* curent no. of hist. file names */
extern  uint8 numSrchHist ;             /* curent no. of hist. search strs*/
extern  uint8 **strHist ;               /* string history list            */
extern  uint8 **buffHist ;              /* etc.                           */
extern  uint8 **commHist ;
extern  uint8 **fileHist ;
extern  uint8 **srchHist ;
extern  int16 HistNoFilesLoaded ;       /* Count of no files loaded by hist */

#if HILIGHT
extern uint8       noHilights ;
extern HILNODEPTR *hilights ;
extern uint8       noIndents ;
extern HILNODEPTR *indents ;
#endif

#if LCLBIND
extern uint8   useMlBinds;              /* flag to use ml bindings      */
extern uint16  mlNoBinds;               /* no. of message line bindings */
extern KEYTAB *mlBinds;                 /* pointer to ml local bindings */
#endif

/* uninitialized global external declarations */
extern  uint8   resultStr[MAXBUF] ;     /* $result variable             */
extern  uint8   evalResult[TOKENBUF] ;  /* Result string from functions */
extern  int     curgoal;                /* Goal for C-P, C-N            */
extern  int16   numWindows;             /* Current number of windows    */
extern  WINDOW  *curwp;                 /* Current window               */
extern  BUFFER  *curbp;                 /* Current buffer               */
extern  WINDOW  *wheadp;                /* Head of list of windows      */
extern  BUFFER  *bheadp;                /* Head of list of buffers      */
extern  meABREV *aheadp;                /* Head of list of abrev files  */

extern  struct  KLIST* klhead;          /* Head of klist                */

/* the character lookup tables */
extern  uint8 charMaskTbl1[] ;
extern  uint8 charMaskTbl2[] ;
extern  uint8 charCaseTbl[] ;
extern  uint8 charLatinUserTbl[] ;
extern  uint8 charUserLatinTbl[] ;
extern  uint8 charMaskFlags[] ;
extern  uint8 isWordMask ;

/* the following are global variables but not defined in this file */
extern  VVIDEO vvideo;                  /* Virtual video - see display.c */
extern  int    mlfirst;                 /* initial command, set by respawn() */
extern  uint8  meCopyright[] ;

/* fileio file pointer */
#ifdef _WIN32
extern HANDLE  ffrp;
extern HANDLE  ffwp;
#else
extern FILE   *ffrp;
extern FILE   *ffwp;
#endif

/* the following are the cursor position and ml variables */
extern int     mwRow ;                  /* Main Windows current row. */
extern int     mwCol ;                  /* Main Windows current column */
extern int     osdCol ;                 /* The osd current column */
extern int     osdRow ;                 /* The osd current row */

#define MLSTATUS_KEEP    0x01
#define MLSTATUS_RESTORE 0x02
#define MLSTATUS_POSML   0x04
#define MLSTATUS_POSOSD  0x08
#define MLSTATUS_CLEAR   0x10
#define MLSTATUS_NINPUT  0x20
#define MLSTATUS_OSDPOS  0x40
extern uint8   mlStatus ;
extern int     mlCol ;
extern uint8  *mlStore ;
extern int     mlStoreCol ;

/**************************************************************************
* Constant declarations for the external definitions above. These are     *
* only processed in main.c                                                *
**************************************************************************/
#ifdef  maindef

/* for MAIN.C */

/* initialized global definitions */
#ifdef _INSENSE_CASE
meDIRLIST  curDirList={0,0} ;
#else
meDIRLIST  curDirList={1,0} ;
#endif
#if MEOSD
struct osdDIALOG  *osdDialogHd = NULL;  /* Root of the on screen displays */
struct osdDISPLAY *osdDisplayHd = NULL; /* Menu display list */
#endif
meREGISTERS *meRegHead=NULL ;           /* The register Head            */
meREGISTERS *meRegCurr=NULL ;           /* The current register set     */
SELHILIGHT selhilight={1,0} ;           /* Selection hilight            */
uint16   hilBlockS=20 ;                 /* Hilight - HilBlock array siz */
meSTYLE *styleTable = NULL;             /* Highlighting colour table    */
#if ABREV
meABREV *globalAbrev = NULL;            /* Global Abreviation file      */
#endif
mePOS *mePosition=NULL ;                /* Position stack head          */
meCOLOR noColors=0 ;                    /* No defined colours           */
int styleTableSize = 2;                 /* Size of the style table      */
HILBLOCK *hilBlock;                     /* Hilighting style change      */
uint8  *disLineBuff=NULL ;              /* interal display buffer array */
int     disLineSize=512 ;               /* interal display buffer size  */
int     cursorState=0 ;                 /* Current state of cursor      */
FRAMELINE *frameStore = NULL;           /* Frame store line pointers    */
uint8  *progName=NULL ;                 /* the program name (argv[0])   */
uint8   orgModeLineStr[]="%s%r%u me (%e) - %l %b (%f) ";
uint8  *modeLineStr=orgModeLineStr;     /* current modeline format      */
int32   autotime = 300 ;                /* auto save time in seconds    */
int     keptVersions = 0 ;              /* No. of kept backup versions  */
uint8   fillbullet[16]="*)].-";         /* Fill bullet character class  */
int16   fillbulletlen = 15;             /* Fill lookahead limit         */
int16   fillcol = 78;                   /* Current fill column          */
uint8   filleos[16]=".!?";              /* Fill E-O-S character class   */
int16   filleoslen=1;                   /* Fill E-O-S ' ' insert len    */
uint8   fillignore[16]=">_@";           /* Fill Ignore character class  */
uint16  tabsize  = 4;                   /* Virtual Tab size             */
uint16  tabwidth = 8;                   /* Real TAB size                */
int16   matchlen = 2000;                /* Fence matching sleep length  */
uint8   *searchPath=NULL;               /* emf search path              */
uint8   *homedir=NULL;                  /* Home directory               */
uint8   *curdir=NULL;                   /* current working directory    */
uint8   *execstr = NULL;                /* pointer to string to execute */
uint32  delaytime = 500;                /* mouse-time delay time 500ms  */
uint32  repeattime = 25;                /* mouse-time repeat time 25ms  */
uint32  idletime = 1000;                /* idle-time delay time 1sec    */
int     execlevel = 0;                  /* execution IF level           */
int     bufHistNo = 1;                  /* inc buff hist numbering      */
int     lastCommand = 0 ;               /* The last user executed key   */
int     lastIndex = -1 ;                /* The last user executed comm  */
int     thisCommand = 0 ;               /* The cur. user executed key   */
int     thisIndex = -1 ;                /* The cur. user executed comm  */

#ifdef _WIN32
HANDLE  ffrp;                           /* File read pointer, all func. */
HANDLE  ffwp;                           /* File write pointer, all func.*/
#else
FILE   *ffrp;                           /* File read pointer, all func. */
FILE   *ffwp;                           /* File write pointer, all func.*/
#endif
uint16  thiskey ;                       /* the current key              */
uint8   hexdigits[] = "0123456789ABCDEF";
uint32  cursorBlink = 0 ;               /* cursor-blink blink time      */
int     blinkState=1 ;                  /* cursor blink state           */
#if COLOR
meCOLOR cursorColor=meCOLOR_FDEFAULT;   /* cursor color                 */
meSCHEME osdScheme =meSCHEME_NDEFAULT;  /* Menu line color scheme       */
meSCHEME mlScheme  =meSCHEME_NDEFAULT;  /* Message line color scheme    */
meSCHEME mdLnScheme=meSCHEME_RDEFAULT;  /* Mode line color scheme       */
meSCHEME sbarScheme=meSCHEME_RDEFAULT;  /* Scroll bar color scheme      */
meSCHEME globScheme=meSCHEME_NDEFAULT;  /* Global color scheme          */
meSCHEME trncScheme=meSCHEME_NDEFAULT;  /* Truncate color scheme        */
#endif
int     gsbarmode = (WMUP |             /* Has upper end cap            */
                     WMDOWN |           /* Has lower end cap            */
                     WMBOTTM |          /* Has a mode line character    */
                     WMSCROL |          /* Has a box on scrolling shaft */
                     WMRVBOX |          /* Reverse video on box         */
                     WMSPLIT |          /* Has a splitter               */
                     WMVBAR             /* Window has a vertical bar    */
                     );                 /* global scroll bar mode       */
uint8 boxChars[BCLEN+1] =               /* Set of box characters        */
"|+++++++++-";
uint8 windowChars[WCLEN+1] =            /* Set of window characters     */
{
    '=',                                /* Mode line current sep */
    '-',                                /* Mode libe inactive sep */
    '#',                                /* Root indicator */
    '*',                                /* Edit indicator */
    '%',                                /* View indicator */
    /* Single scroll bar */
    '=',                                /* Buffer split */
    '^',                                /* Scroll bar up */
    ' ',                                /* Scroll bar up shaft */
    ' ',                                /* Scroll box */
    ' ',                                /* Scroll bar down shaft */
    'v',                                /* Scroll bar down */
    '*',                                /* Scroll bar/mode line point */
    /* Double scroll bar */
    '=','=',                            /* Buffer split */
    '^','^',                            /* Scroll bar up */
    ' ',' ',                            /* Scroll bar up shaft */
    ' ',' ',                            /* Scroll box */
    ' ',' ',                            /* Scroll bar down shaft */
    'v','v',                            /* Scroll bar down */
    '*','*',                            /* Scroll bar/mode line point */
    /* Single horizontal scroll bar */
    '|',                                /* Buffer split */
    '<',                                /* Scroll bar left */
    ' ',                                /* Scroll bar left shaft */
    ' ',                                /* Scroll box */
    ' ',                                /* Scroll bar right shaft */
    '>',                                /* Scroll bar right */
    '*',                                /* Scroll bar/mode line point */
    /* Double horizontal scroll bar */
    '|','|',                            /* Buffer split */
    '<','<',                            /* Scroll bar left */
    ' ',' ',                            /* Scroll bar left shaft */
    ' ',' ',                            /* Scroll box */
    ' ',' ',                            /* Scroll bar right shaft */
    '>','>',                            /* Scroll bar right */
    '*','*',                            /* Scroll bar/mode line point */
    ' ',                                /* OSD title bar char */
    'x',                                /* OSD Title bar kill char */
    '*',                                /* OSD resize char */
    ' ',                                /* OSD button start char e.g. '['   */
    ' ',                                /* OSD button close char e.g. ']'   */
    /* Display characters */
    '>',                                /* tab \t display character */
    '\\',                               /* new-line \n display character */
    '.',                                /* space ' ' display character */
    0
} ;
uint8 displayTab=' ';                   /* tab \t display character */
uint8 displayNewLine=' ';               /* new-line \n display character */
uint8 displaySpace=' ';                 /* space ' ' display character */
int32 startTime;                        /* me start time used as offset */
uint8 thisflag;                         /* Flags, this command          */
uint8 lastflag;                         /* Flags, last command          */
uint8 alarmState=0;                     /* Unix auto-save alarm time    */
uint8 fillmode='B';                     /* Justification mode           */
uint8 scrollFlag = 1 ;                  /* horiz/vert scrolling method  */
uint8 sgarbf = TRUE;                    /* TRUE if screen is garbage    */
uint8 clexec = FALSE;                   /* command line execution flag  */
uint8 mcStore = FALSE;                  /* storing text to macro flag   */
int8  macbug = 0 ;                      /* macro debuging flag          */
uint8 cmdstatus = TRUE;                 /* last command status          */
uint8 kbdmode=STOP;                     /* current keyboard macro mode  */
uint8 lastReplace=0;                    /* set to non-zero if last was a replace */
uint8 modeLineFlags=                    /* current modeline flags       */
(WFMODE|WFRESIZE|WFMOVEL) ;
uint32 meSystemCfg=                     /* ME system config variable    */
#ifdef _DOS
(meSYSTEM_ANSICOLOR|meSYSTEM_XANSICOLOR|meSYSTEM_MSSYSTEM|meSYSTEM_DRIVES|meSYSTEM_DOSFNAMES|meSYSTEM_TABINDANY|meSYSTEM_ALTMENU|meSYSTEM_ALTPRFX1) ;
#endif
#ifdef _WIN32
(meSYSTEM_RGBCOLOR|meSYSTEM_FONTS|meSYSTEM_MSSYSTEM|meSYSTEM_DRIVES|meSYSTEM_DOSFNAMES|meSYSTEM_TABINDANY|meSYSTEM_ALTMENU|meSYSTEM_ALTPRFX1|meSYSTEM_CTCHASPC
#ifdef _IPIPES
|meSYSTEM_IPIPES
#endif
) ;
#endif
#ifdef _UNIX
(meSYSTEM_RGBCOLOR|meSYSTEM_FONTS|meSYSTEM_UNIXSYSTEM|meSYSTEM_IPIPES|meSYSTEM_TABINDANY|meSYSTEM_ALTMENU|meSYSTEM_ALTPRFX1) ;
#endif
#ifdef _WIN32
uint32 meSYSTEM_MASK=                   /* ME system mask - dependant on win32 flavour */
(meSYSTEM_FONTS|meSYSTEM_TABINDANY|meSYSTEM_ALTMENU|meSYSTEM_ALTPRFX1|meSYSTEM_KEEPUNDO|meSYSTEM_FONTFIX|meSYSTEM_CTCHASPC|meSYSTEM_SHOWWHITE|meSYSTEM_HIDEBCKUP|meSYSTEM_TABINDFST
#if !defined (_WIN32s)
|meSYSTEM_CLNTSRVR
#endif
) ;
#endif
uint32 meMouseCfg=(3|meMOUSE_ENBLE);    /* ME mouse config variable    */
uint8 mouse_pos=0xff;                   /* mouse virtual position       */
int16 mouse_X = 0;                      /* mouse X pos at last event    */
int16 mouse_Y = 0;                      /* mouse X pos at last event    */
int16 mouse_dX = 0;                     /* mouse delta X last event pos */
int16 mouse_dY = 0;                     /* mouse delta X last event pos */

#if MEUNDO
uint32 undoContFlag=0;                  /* continuation of an undo      */
#endif

/* File Modes */
#if (defined _DOS) || (defined _WIN32)
uint16 meUmask=0x020 ;                  /* file creation umask          */
#endif
#ifdef _UNIX
uint16    meUmask ;                     /* file creation umask          */
mode_t    meXUmask ;                    /* directory creation umask     */
uid_t     meUid ;                       /* current user id              */
gid_t     meGid ;                       /* current group id             */
int       meGidSize ;                   /* number of groups the user belongs to   */
gid_t    *meGidList=NULL ;              /* list of group ids the user belongs to  */
#endif /* _UNIX */

/* Signals */
#ifdef _UNIX
#ifdef _POSIX_SIGNALS
sigset_t  meSigHoldMask ;               /* signals help when spawning and reading */
#endif /* _POSIX_SIGNALS */
#ifdef _BSD_SIGNALS
int meSigHoldMask = 0;                  /* signals held when spawning and reading */
#endif /* _BSD_SIGNALS */
int       meSigLevel = 0;               /* signal level */
#endif /* _UNIX */

/* Environment Variables */
#if ((defined(_UNIX)) && (defined(_NOPUTENV)))
char    **meEnviron = NULL;             /* Our own environment */
#endif /* ((defined(_UNIX)) && (defined(_NOPUTENV))) */

#ifdef _IPIPES
meIPIPE  *ipipes=NULL ;                 /* list of all the current pipes*/
int       noIpipes=0 ;                  /* count of all the cur pipes   */
#endif

LINE *menuLine = NULL;                  /* Menu-poke line */
LINE *lpStore = NULL;                   /* line off which to store macro*/
BUFFER *lpStoreBp = NULL;               /* help is stored in a buffer   */
uint16 prefixc[ME_PREFIX_NUM+1]=
{ ME_INVALID_KEY,                       /* unused 0th value             */
  ME_SPECIAL|SKEY_esc,                  /* prefix1 = Escape             */
  'X'-'@',                              /* prefix2 = ^X                 */
  'H'-'@',                              /* prefix3 = ^H                 */
  ME_INVALID_KEY, ME_INVALID_KEY,       /* rest unused                  */
  ME_INVALID_KEY, ME_INVALID_KEY,
  ME_INVALID_KEY, ME_INVALID_KEY,
  ME_INVALID_KEY, ME_INVALID_KEY,
  ME_INVALID_KEY, ME_INVALID_KEY,
  ME_INVALID_KEY, ME_INVALID_KEY,
  ME_INVALID_KEY
} ;
uint16 reptc    = 'U'-'@';              /* current universal repeat char*/
uint16 breakc   = 'G'-'@';              /* current abort-command char*/

KLIST  *klhead    = NULL;               /* klist header pointer            */
uint8   lkbdptr[NKBDM];                 /* Holds last keybd macro data     */
int     lkbdlen=0;                      /* Holds length of last macro      */
uint8  *kbdptr=NULL;                    /* start of current keyboard buf   */
int     kbdlen=0;                       /* len of current macro            */
int     kbdoff=0;                       /* current offset of macro         */
int     kbdrep=0;                       /* number of repetitions           */
uint8   emptym[]  = "";                 /* empty literal                   */
uint8   errorm[]  = "ERROR";            /* error literal                   */
uint8   abortm[]  = "ABORT";            /* abort literal                   */
uint8   truem[]   = "1";                /* true literal            */
uint8   falsem[]  = "0";                /* false litereal                  */
meVARLIST usrVarList={NULL,0} ;         /* user variables list             */


#if CFENCE
int16   statementIndent = 4 ;
int16   continueIndent  = 10 ;
int16   continueMax     = 0 ;
int16   commentIndent   = 1 ;
int16   switchIndent    = 0 ;
int16   braceIndent     = -4 ;
int16   caseIndent      = -4 ;
int16   commentMargin   = -1 ;
uint8   commentContOrg[]= " * " ;
uint8  *commentCont     = commentContOrg ;
#endif

/* global buffer names */
uint8  BvariablesN[] = "*variables*" ;
uint8  BbindingsN[] = "*bindings*" ;
uint8  BtranskeyN[] = "*translate-key*" ;
uint8  BcompleteN[] = "*complete*" ;
uint8  BcommandsN[] = "*commands*" ;
uint8  BicommandN[] = "*icommand*" ;
uint8  BregistryN[] = "*registry*" ;
uint8  BbuffersN[] = "*buffers*" ;
uint8  BcommandN[] = "*command*" ;
uint8  BolhelpN[] = "*on-line help*" ;
uint8  BserverN[] = "*server*" ;
uint8  BaboutN[] = "*about*" ;
uint8  BstdinN[] = "*stdin*" ;
uint8  BmainN[] = "*scratch*" ;
uint8  BhelpN[] = "*help*" ;

uint8 *fileIgnore=NULL ;

#if FLNEXT
uint8 *flNextFileTemp=NULL ;
uint8 *flNextLineTemp=NULL ;

uint8    noNextLine=0 ;
uint8  **nextName=NULL ;
uint8   *nextLineCnt=NULL ;
uint8 ***nextLineStr=NULL ;
#endif

uint8    fileHookCount=0 ;
uint8  **fileHookExt=NULL ;
uint8  **fileHookFunc=NULL ;
int16   *fileHookArg=NULL ;

#if DORCS
uint8 *rcsFile=NULL ;
uint8 *rcsCoStr=NULL ;
uint8 *rcsCoUStr=NULL ;
uint8 *rcsCiStr=NULL ;
uint8 *rcsCiFStr=NULL ;
uint8 *rcsUeStr=NULL ;
#endif

/* history variables used in meGetStringFromUser */
uint8 numStrHist = 0 ;                  /* curent no. of hist. strings    */
uint8 numBuffHist = 0 ;                 /* curent no. of hist. buff names */
uint8 numCommHist = 0 ;                 /* curent no. of hist. comm names */
uint8 numFileHist = 0 ;                 /* curent no. of hist. file names */
uint8 numSrchHist = 0 ;                 /* curent no. of hist. search strs*/
uint8 **strHist ;                       /* string history list            */
uint8 **buffHist ;                      /* etc.                           */
uint8 **commHist ;
uint8 **fileHist ;
uint8 **srchHist ;
int16 HistNoFilesLoaded = 0 ;           /* Count of no files loaded by hist */

#if HILIGHT
uint8       noHilights=0 ;
HILNODEPTR *hilights=NULL ;
uint8       noIndents=0 ;
HILNODEPTR *indents=NULL ;
#endif

#if LCLBIND
uint8   useMlBinds=0;                   /* flag to use ml bindings      */
uint16  mlNoBinds=0;                    /* no. of message line bindings */
KEYTAB *mlBinds=NULL;                   /* pointer to ml local bindings */
#endif

int     curgoal;                        /* Goal for C-P, C-N            */
int16   numWindows;                     /* Number of windows            */
WINDOW  *curwp;                         /* Current window               */
BUFFER  *curbp;                         /* Current buffer               */
WINDOW  *wheadp;                        /* Head of list of windows      */
BUFFER  *bheadp;                        /* Head of list of buffers      */
meABREV *aheadp=NULL;                   /* Head of list of abrev files  */

uint16  keyTableSize;                   /* The current size of the key table */
uint8   resultStr [MAXBUF];             /* Result string from commands  */

/* the following are the cursor position and ml variables */
int mwRow =0 ;                          /* Main Windows current row. */
int mwCol =0 ;                          /* Main Windows current column */
int osdCol=0 ;                          /* The osd current column */
int osdRow=0 ;                          /* The osd current row */

int mlCol=0 ;                           /* ml current column */
int mlStoreCol=0 ;                      /* ml Store column */
uint8 *mlStore;                         /* stored message line. */
uint8 mlStatus=0 ;                      /* ml status
                                         * 0=not using it,
                                         * 1=using it.
                                         * 2=using it & its been broken so
                                         * next time mlerease is used, it will
                                         * restore */
#ifdef _CLIPBRD
uint8 clipState=0;                      /* clipboard status flag        */
#endif
#endif
