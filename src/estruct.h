/**-*- c -*-*************************************************************
 * 
 *	SCCS:		%W%		%G%		%U%
 *
 *	Last Modified :	<001012.1819>
 *
 *	ESTRUCT:	Structure and preprocesser defined for
 *			MicroEMACS 3.7
 *
 *			written by Dave G. Conroy
 *			modified by Steve Wilhite, George Jones
 *			greatly modified by Daniel Lawrence
 *
 * Jon Green	13-07-90
 * Added Document Mode. Search for .doc or .txt extension for WRAP and 
 * indent to automatically be enabled.
 * Jon Green	17-05-91
 * Added suffix modes so that the suffix defintions may be defined in the 
 * start up file.
 * 
 ************************************************************************
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
/*	internal constants	*/

#define NBINDS	 384			/* max # of globally bound keys */
#define NBUFN    16                     /* # of bytes, buffer name      */
#define	NSTRING	 128			/* # of bytes, string buffers	*/
#define NKBDM    256                    /* # of strokes, keyboard macro */
#define NPAT     128                    /* # of bytes, pattern          */
#define	NBLOCK	 16			/* line block chunk size	*/
#define NWINDOWS 64                     /* # of windows MAXIMUM         */
#define MAXBUF	 1024			/* size of various inputs       */
#define FILEBUF	 1024			/* size of a file name input    */
#define TOKENBUF MAXBUF+4		/* MAXBUF + an overrun safe area*/
#define	TSTMPLEN 40			/* Max len of a time stamp str. */

#define	meNLCHAR   0x0a                 /* the \n char ^J, not ^M       */

#define	ME_SHIFT   0x0100		/* special key shift    	*/
#define	ME_CONTROL 0x0200		/* special key conrtol		*/
#define	ME_ALT     0x0400		/* special key alt		*/
#define	ME_SPECIAL 0x0800		/* special key (function keys)	*/
#define ME_PREFIX_NUM  16               /* the number of prefixes       */
#define ME_PREFIX_BIT  12               /* the number of prefixes       */
#define ME_PREFIX_MASK 0xf000           /* the prefix bit mask          */
#define ME_PREFIX1 0x1000               /* prefix 1 flag, or'ed in      */
#define ME_PREFIX2 0x2000               /* prefix 2 flag, or'ed in      */
#define ME_PREFIX3 0x3000               /* prefix 3 flag, or'ed in      */
#define ME_PREFIX4 0x4000               /* prefix 4 flag, or'ed in      */
#define ME_INVALID_KEY 0x0000           /* an invalid key number        */

/* Status variables. These are sometimes defined within the compiler flags.
 * Inherit the values provided that they are correct. */

/* Status value FALSE */
#ifndef FALSE
#define FALSE   0                       /* False, no, bad, etc.         */
#endif /* FALSE */

/* Status value TRUE */
#ifndef TRUE
#define TRUE    1                       /* True, yes, good, etc.        */
#endif /* TRUE */

/* Status value ABORT */
#define ABORT   2                       /* Death, ^G, abort, etc.       */

/* Keyboard states */
#define STOP	 0			/* keyboard macro not in use	*/
#define PLAY	 1			/*		  playing	*/
#define RECORD	 2			/*		  recording	*/
#define KBD_IDLE 3                      /* keyboard not in use/idle     */

/* Search etc. directions */
#define FORWARD 0			/* forward direction		*/
#define REVERSE 1			/* backwards direction		*/

/* File I/O States */
#define FIOSUC  0                       /* File I/O, success.           */
#define FIOFNF  1                       /* File I/O, file not found.    */
#define FIOEOF  2                       /* File I/O, end of file.       */
#define FIOERR  3                       /* File I/O, error.             */
#define	FIOLNG	4			/*line longer than allowed len	*/
#define FIOFUN	5			/* File I/O, eod of file/bad line*/
#define FIOBUFSIZ 2048

/* Maximum history size */
#define MLHISTSIZE 20

/* Last command states */
#define CFCPCN  0x0001                  /* Last command was C-P, C-N    */
#define CFKILL  0x0002                  /* Last command was a kill      */
#define	CFYANK	0x0004			/* Last command was yank	*/
#define	CFRYANK	0x0008			/* Last command was reyank	*/
#define CFUNDO  0x0010                  /* Last command was undo        */

/* Path separator character used in pathname lists i.e. env variables */
#ifdef _UNIX
#define PATHCHR ':'
#else
#define PATHCHR ';'
#endif

/* Standard function prototypes used in various static tables */
typedef	int	(*Fintv)(void);
typedef	int	(*Fintc)(char);
typedef	int	(*Finti)(int);
typedef	int	(*Fintii)(int,int);
typedef	int	(*Fints)(char *);
typedef	int	(*Fintss)(const char *, const char *);
typedef	int	(*Fintssi)(const char *, const char *,size_t);
typedef int	(*Fintccci)(char *,char *,char *,int) ;

/* meSTYLE contains color and font information coded into an int32 the
 * following #defines and macros are used to manipulate them.
 */
typedef uint8   meCOLOR ;
#define meCOLOR_FDEFAULT 0
#define meCOLOR_BDEFAULT 1
#define meCOLOR_INVALID  0xff
#define meFColorCheck(x)   (((x)>=noColors)       ? meCOLOR_FDEFAULT : (x))
#define meBColorCheck(x)   (((x)>=noColors)       ? meCOLOR_BDEFAULT : (x))


typedef uint32  meSTYLE ;
#define meSTYLE_NDEFAULT  0x00000100
#define meSTYLE_RDEFAULT  0x00080001

#define meSTYLE_FCOLOR    0x000000ff
#define meSTYLE_BCOLOR    0x0000ff00
#define meSTYLE_FONT      0x001f0000
#define meSTYLE_BOLD      0x00010000
#define meSTYLE_ITALIC    0x00020000
#define meSTYLE_LIGHT     0x00040000
#define meSTYLE_REVERSE   0x00080000
#define meSTYLE_UNDERLINE 0x00100000
#define meSTYLE_CMPBCOLOR (meSTYLE_BCOLOR|meSTYLE_REVERSE)
#define meSTYLE_INVALID   0xffffffff

#define meFONT_COUNT      5
#define meFONT_BOLD       0x01
#define meFONT_ITALIC     0x02
#define meFONT_LIGHT      0x04
#define meFONT_REVERSE    0x08
#define meFONT_UNDERLINE  0x10
#define meFONT_MAX        0x20
#define meFONT_MASK       0x1f

#define meStyleSet(ss,fc,bc,ff) ((ss)=((fc)|((bc)<<8)|((ff)<<16)))
#define meStyleGetFColor(ss)    ((ss)&meSTYLE_FCOLOR)
#define meStyleGetBColor(ss)    (((ss)&meSTYLE_BCOLOR) >> 8)
#define meStyleGetFont(ss)      (((ss)&meSTYLE_FONT) >> 16)
#define meStyleCmp(s1,s2)       ((s1) != (s2))
#define meStyleCmpBColor(s1,s2) ((s1 & meSTYLE_CMPBCOLOR) != ((s2) & meSTYLE_CMPBCOLOR))

/* An meSCHEME is simply an index into the meSTYLE table.
 * Each scheme created by add-color-scheme consists of 8 meSTYLEs, as there
 * are upto 256 schemes there can be 8*256 styles hence its a uint16.
 * note that users enter the base scheme number, i.e. 0, 1, 2, ...
 * which is then multiplied by 8 and stored, i.e. 0, 8, 16, ...
 */
typedef uint16 meSCHEME ;
#define meSCHEME_NORMAL   0              /* Normal style */
#define meSCHEME_RNORMAL  1              /* Reverse normal style */
#define meSCHEME_CURRENT  2              /* Current foreground colour */        
#define meSCHEME_RCURRENT 3              /* Reverse current foreground colour */        
#define meSCHEME_SELECT   4              /* Selected foreground colour */        
#define meSCHEME_RSELECT  5              /* Reverse celected foreground colour */        
#define meSCHEME_CURSEL   6              /* Current selected foreground color */
#define meSCHEME_RCURSEL  7              /* Reverse current selected foreground color */
#define meSCHEME_STYLES   8              /* Number of styles in a scheme */
#define meSCHEME_NDEFAULT 0
#define meSCHEME_RDEFAULT meSCHEME_STYLES
#define meSCHEME_INVALID   0xffff

#define meSCHME_STYLE         0x0fff
#define meSCHME_NOFONT        0x1000

#define meSchemeCheck(x)                 (((x)>=styleTableSize) ? meSCHEME_NDEFAULT:(x))
#define meSchemeGetStyle(x)              (styleTable[(x) & meSCHME_STYLE])
#define meSchemeTestStyleHasFont(x)      (styleTable[(x) & meSCHME_STYLE] & (meSTYLE_UNDERLINE|meSTYLE_ITALIC|meSTYLE_BOLD))
#define meSchemeTestNoFont(x)            ((x) & meSCHME_NOFONT)
#define meSchemeSetNoFont(x)             ((x) | meSCHME_NOFONT)
/*
 * All text is kept in circularly linked lists of "LINE" structures. These
 * begin at the header line (which is the blank line beyond the end of the
 * buffer). This line is pointed to by the "BUFFER". Each line contains a the
 * number of bytes in the line (the "used" size), the size of the text array,
 * and the text. The end of line is not stored as a byte; its implied. Future
 * additions will include update hints, and a list of marks into the line.
 */
typedef struct LINE
{
    struct LINE *l_fp;			/* Link to the next line        */
    struct LINE *l_bp;			/* Link to the previous line    */
    uint16  l_size;                     /* Allocated size               */
    uint16  l_used;                     /* Used size                    */
    uint8   l_flag;		        /* Line is marked if true	*/
    uint8   l_text[1];                  /* A bunch of characters.       */
} LINE ;

#define LNSMASK   0x0f
#define LNNOSCHM  0x10
#define LNCHNG    0x10
#define LNMARK    0x20
#define LNNEOL    0x40			/* Save line with no \n or \0 	*/

#define lforw(lp)       ((lp)->l_fp)
#define lback(lp)       ((lp)->l_bp)
#define lgetc(lp, n)    ((lp)->l_text[(n)])
#define lputc(lp, n, c) ((lp)->l_text[(n)]=(c))
#define ltext(lp)       ((lp)->l_text)
#define lchng(lp)       ((lp)->l_flag &= LNCHNG)
#define llength(lp)     ((lp)->l_used)

/*
 * There is a window structure allocated for every active display window. The
 * windows are kept in a big list, in top to bottom screen order, with the
 * listhead at "wheadp". Each window contains its own values of dot and mark.
 * The flag field contains some bits that are set by commands to guide
 * redisplay; although this is a bit of a compromise in terms of decoupling,
 * the full blown redisplay is just too expensive to run for every input
 * character.
 * 
 * The physical screen extents of the window are contained in firstCol, firstRow,
 * numCols numRows. These define the physical extremities of the window, within
 * those boundaries lie the message line, scroll bar and text area. 
 * The separate window extents appear to duplicate some of the existing values,
 * (which they do), however they do serve a purpose in that they decouple the 
 * window tesselation and layout (i.e. where a window is positioned on the
 * screen) from having to have any knowledge about the exact format of the 
 * contents of the window, thereby reducing maintenance of the tesselation 
 * code which is a bit of a mind bender at times.
 * 
 * Note that the "flags" are at the top. We access these alot on the display
 * side. Positioned here ensures that they are available immediatly without
 * any indirect offset - just give that compiler a hand. 
 */

/*
 * WINDOW structure w_flag values.
 * Define states to the display drivers to update the screen. 
 */
#define WFFORCE       0x0001            /* Window needs forced reframe  */
#define WFRESIZE      0x0002            /* Reset the width of lines.    */
#define WFMOVEL       0x0004            /* Movement from line to line   */
#define WFMOVEC       0x0008            /* Movement from col to col     */
#define WFMAIN        0x0010            /* Update window main area      */
#define WFMODE        0x0020            /* Update window mode line      */
#define WFDOT         0x0040            /* Redraw windows dot line      */
#define WFREDRAW      0x0080            /* Redraw main window area      */
#define WFSBSPLIT     0x0100            /* Scroll split has changed     */
#define WFSBUP        0x0200            /* Scroll up arrow change       */
#define WFSBUSHAFT    0x0400            /* Scroll bar up shaft change   */
#define WFSBBOX       0x0800            /* Scroll bar box change        */
#define WFSBDSHAFT    0x1000            /* Scroll bar down shaft change */
#define WFSBDOWN      0x2000            /* Scroll bar down arrow change */
#define WFSBML        0x4000            /* Scroll bar mode line changed */
#define WFSELHIL      0x8000            /* Selection highlight changed  */
#define WFSELDRAWN    0x10000           /* Editing within a line        */
#define WFLOOKBK      0x20000           /* Check the hilight look-back  */
/* Composite mode to force an update of the scroll box */
#define WFSBOX        (WFSBUSHAFT|WFSBBOX|WFSBDSHAFT)    
/* Composite mode to force update of the scroll bar */
#define WFSBAR        (WFSBSPLIT|WFSBUP|WFSBDOWN|WFSBML|WFSBOX)

/* A garbage update of the screen adds following modes */
#define WFUPGAR       (WFMODE|WFRESIZE|WFSBAR)

/*
 * WINDOW structure w_mode values.
 * Modes define the sroll bar state.
 */
#define WMVWIDE 0x01                    /* PUBLIC:  Wide vert divider 2 chars    */
#define WMUP    0x02                    /* PUBLIC:  Has upper end cap            */
#define WMDOWN  0x04                    /* PUBLIC:  Has lower end cap            */
#define WMBOTTM 0x08                    /* PUBLIC:  Has a mode line character    */
#define WMSCROL 0x10                    /* PUBLIC:  Has a box on scrolling shaft */
#define WMRVBOX 0x20                    /* PUBLIC:  Reverse video on box         */
#define WMHWIDE 0x40                    /* PUBLIC:  Wide horz divider 2 chars    */
#define WMSPLIT 0x80                    /* PUBLIC:  Has a splitter               */
#define WMVBAR  0x100                   /* PUBLIC:  Window has a vertical bar    */
#define WMHORIZ 0x200                   /* PRIVATE: This is a horizontal bar     */
#define WMWIDE  0x400                   /* PRIVATE: Wide scroll-bar (internal)   */
  
/* The user interface mask for the scroll mode */
#define WMUSER  (WMVWIDE|WMUP|WMDOWN|WMBOTTM|WMSCROL|WMRVBOX|WMHWIDE|WMSPLIT|WMVBAR)
/*
 * Define the window character array. This is a fixed array of 
 * characters which define the window components
 */
/* Mode line */
#define WCMLCWSEP     0                  /* Mode line current window sep  (=)*/
#define WCMLIWSEP     1                  /* Mode line inactive window sep (-)*/
#define WCMLROOT      2                  /* Mode line root indicatior     (#)*/
#define WCMLBCHNG     3                  /* Mode line buffer changed char (*)*/
#define WCMLBVIEW     4                  /* Mode line buffer view char    (%)*/
/* Scroll Bar */
#define WCVSBSPLIT    5                  /* Buffer split character #1        */
#define WCVSBUP       6                  /* Scroll bar up character          */
#define WCVSBUSHAFT   7                  /* Scroll bar up shaft character    */
#define WCVSBBOX      8                  /* Scroll bar box character         */
#define WCVSBDSHAFT   9                  /* Scroll bar down shaft character  */
#define WCVSBDOWN    10                  /* Scroll bar down character        */
#define WCVSBML      11                  /* Scroll bar/mode line point       */
#define WCVSBSPLIT1  12                  /* Buffer split character #1        */
#define WCVSBSPLIT2  13                  /* Buffer split character #2        */
#define WCVSBUP1     14                  /* Wide scroll bar up char #1       */
#define WCVSBUP2     15                  /* Wide scroll bar up char #2       */
#define WCVSBUSHAFT1 16                  /* Wide scroll bar up shaft char #1 */
#define WCVSBUSHAFT2 17                  /* Wide scroll bar up shaft char #2 */
#define WCVSBBOX1    18                  /* Wide scroll bar box char #1      */
#define WCVSBBOX2    19                  /* Wide scroll bar box char #2      */
#define WCVSBDSHAFT1 20                  /* Wide scroll bar dn shaft char #1 */
#define WCVSBDSHAFT2 21                  /* Wide scroll bar dn shaft char #2 */
#define WCVSBDOWN1   22                  /* Wide scroll bar down char #1     */
#define WCVSBDOWN2   23                  /* Wide scroll bar down char #2     */
#define WCVSBML1     24                  /* Wide scroll bar/mode point #1    */
#define WCVSBML2     25                  /* Wide scroll bar/mode point #2    */
#define WCHSBSPLIT   26                  /* Buffer split character #1        */
#define WCHSBUP      27                  /* Scroll bar up character          */
#define WCHSBUSHAFT  28                  /* Scroll bar up shaft character    */
#define WCHSBBOX     29                  /* Scroll bar box character         */
#define WCHSBDSHAFT  30                  /* Scroll bar down shaft character  */
#define WCHSBDOWN    31                  /* Scroll bar down character        */
#define WCHSBML      32                  /* Scroll bar/mode line point       */
#define WCHSBSPLIT1  33                  /* Buffer split character #1        */
#define WCHSBSPLIT2  34                  /* Buffer split character #2        */
#define WCHSBUP1     35                  /* Wide scroll bar up char #1       */
#define WCHSBUP2     36                  /* Wide scroll bar up char #2       */
#define WCHSBUSHAFT1 37                  /* Wide scroll bar up shaft char #1 */
#define WCHSBUSHAFT2 38                  /* Wide scroll bar up shaft char #2 */
#define WCHSBBOX1    39                  /* Wide scroll bar box char #1      */
#define WCHSBBOX2    40                  /* Wide scroll bar box char #2      */
#define WCHSBDSHAFT1 41                  /* Wide scroll bar dn shaft char #1 */
#define WCHSBDSHAFT2 42                  /* Wide scroll bar dn shaft char #2 */
#define WCHSBDOWN1   43                  /* Wide scroll bar down char #1     */
#define WCHSBDOWN2   44                  /* Wide scroll bar down char #2     */
#define WCHSBML1     45                  /* Wide scroll bar/mode point #1    */
#define WCHSBML2     46                  /* Wide scroll bar/mode point #2    */
#define WCOSDTTLBR   47                  /* Osd title bar char               */
#define WCOSDTTLKLL  48                  /* Osd title bar kill char          */
#define WCOSDRESIZE  49                  /* Osd resize char                  */
#define WCOSDBTTOPN  50                  /* Osd button start char e.g. '['   */
#define WCOSDBTTCLS  51                  /* Osd button close char e.g. ']'   */
#define WCDISPTAB    52                  /* Display tab char                 */
#define WCDISPCR     53                  /* Display new-line char            */
#define WCDISPSPACE  54                  /* Display space char               */
#define WCLEN        55                  /* Number of characters             */

/* The box chars BC=Box Chars, N=North, E=East, S=South, W=West,
 * So BCNS = Box Char which has lines from North & South to centre, i.e. 
 *           the vertical line |
 *    BCNESW = the + etc.
 */
#define BCNS         0
#define BCES         1
#define BCSW         2
#define BCNE         3
#define BCNW         4
#define BCESW        5
#define BCNES        6
#define BCNESW       7
#define BCNSW        8
#define BCNEW        9
#define BCEW        10
#define BCLEN       11                  /* Number of characters         */

typedef struct  WINDOW {
    struct  WINDOW *w_wndp;             /* Next window                  */
    struct  WINDOW *w_wnup;             /* Previous window              */
    struct  WINDOW *w_lvideo;           /* VVIDEO thread of windows     */
    struct  VVIDEO *w_vvideo;           /* Virtual video block          */
    struct  BUFFER *w_bufp;             /* Buffer displayed in window   */
    struct  BUFFER *l_bufp;             /* Last Buffer displayed        */
    LINE   *w_dotp;		        /* Line containing "."          */
    LINE   *w_markp;		        /* Line containing "mark"       */
    LINE   *model ;                     /* window's mode-line buffer    */
    LINE   *curLineOff ;                /* Current lines char offsets   */
    int32   topLineNo ;                 /* windows top line number      */
    int32   line_no;                    /* current line number          */
    int32   mlineno;                    /* current mark line number     */
    uint32  w_flag;                     /* Flags.                       */
    uint16  w_doto;                     /* Byte offset for "."          */
    uint16  w_marko;                    /* Byte offset for "mark"       */
    uint16  w_force;                    /* If NZ, forcing row.          */
    uint16  firstRow;                   /* Window starting row          */
    uint16  firstCol;                   /* Window starting column       */
    uint16  numCols;                    /* Window number text columns   */
    uint16  numRows;                    /* Window number text rows      */
    uint16  numTxtRows;                 /* # of rows of text in window  */
    uint16  numTxtCols;                 /* Video number of columns      */
    uint16  w_scscroll;                 /* cur horizontal scroll column */
    uint16  w_sscroll;                  /* the horizontal scroll column */
    uint16  w_margin;                   /* The margin for the window    */
    uint16  w_scrsiz;                   /* The screen size              */
    uint16  w_sbpos[WCVSBML+1];         /* Scroll bar positions         */
    uint16  w_mode;                     /* Operating mode of window     */
} WINDOW ;

/* meSTAT Contains the file node information */
typedef struct {
    int32   stmtime;			/* modification time of file	*/
#ifdef _UNIX
    off_t   stsize ;                    /* File's Size                  */
    uid_t   stuid ;                     /* File's User id               */
    gid_t   stgid ;                     /* File's Group id              */
    dev_t   stdev ;                     /* Files device ID no.          */
    ino_t   stino ;                     /* Files Inode number           */
#else
    int32   stsize ;                    /* File's Size                  */
#endif
    uint16  stmode;                     /* file mode flags              */
} meSTAT ;

/*
 * meABREV
 * 
 * structure to store info on an abbreviation file
 */
typedef struct meABREV {
    struct meABREV *next ;      /* Pointer to the next abrev    */
    LINE   hlp ;                /* head line                    */
    uint8  loaded ;     	/* modification time of file	*/
    uint8  fname[1] ;           /* Users abrev file name        */
} meABREV ;

/* structure to hold user variables and their definitions	*/
typedef struct meVARIABLE
{
    struct meVARIABLE *next ;   /* Next pointer, MUST BE FIRST as with meVARLIST */
    uint8 *value ;		/* value (string) */
    uint8  name[1] ;            /* name of user variable */
} meVARIABLE;

typedef struct meVARLIST
{
    struct meVARIABLE *head ;
    int count ;
} meVARLIST ;

/*	structure for the name binding table		*/

typedef struct meCMD {
    struct meCMD *anext ;		/* alphabetically next command */
    struct meCMD *hnext ;		/* next command in hash table */
    meVARLIST     varList ;             /* command variables list */
    uint8        *name;		        /* name of function key */
    int           id ;                  /* command id number */
    Fintii        func;	 	        /* function name is bound to */
} meCMD ;

typedef struct meMACRO {
    meCMD        *anext ;	       	/* alphabetically next command */
    meCMD        *hnext ;      		/* next command in hash table */
    meVARLIST     varList ;             /* command variables list */
    uint8        *name ;		/* name of function key */
    int           id ;                  /* command id number */
    uint8        *fname ;		/* file name for file-macros */
    LINE         *hlp ;		 	/* Head line of macro */
    int32         callback ;		/* callback time for macro */
} meMACRO ;

#define	MACHIDE  0x01			/* Hide the function		*/
#define	MACEXEC	 0x02			/* Buffer is being executed	*/
#define	MACFILE  0x04			/* macro file define            */


/* An alphabetic mark is as follows. Alphabetic marks are implemented as a
 * linked list of amark structures, with the head of the list being pointed to
 * by b_amark in the buffer structure.
 */
typedef	struct	meAMARK {
    struct meAMARK *next ;		/* pointer to next mark in list		     */
    LINE           *line ;		/* pointer to line associated with this mark */
    uint16          offs ;		/* line offset                               */
    uint16          name ;		/* mark name, (letter associated with it)    */
} meAMARK;

/* A position, stores the current window, buffer, line etc which can
 * be restore later, used by push-position and pop-position */
typedef	struct	mePOS {
    struct mePOS   *next ;              /* pointer to previous position (stack)	     */
    WINDOW         *window ;            /* Current window                            */
    struct BUFFER  *buffer ;            /* windows buffer                            */
    int32           topLineNo ;         /* windows top line number                   */
    int32           line_no ;           /* current line number                       */
    uint16          winMinRow ;         /* Which window - store the co-ordinate      */
    uint16          winMinCol ;         /* so we can restore to the best matching    */
    uint16          winMaxRow ;         /* window on a goto - this greatly improves  */
    uint16          winMaxCol ;         /* its use.                                  */
    uint16          line_amark ;        /* Alpha mark to current line                */
    uint16          w_scscroll ;        /* cur horizontal scroll column              */
    uint16          w_sscroll ;         /* the horizontal scroll column              */
    uint16          w_doto ;            /* Byte offset for "."                       */
    uint16          flags ;             /* Whats stored bit mask                     */
    uint16          name ;		/* position name, (letter associated with it)*/
} mePOS;
#define mePOS_WINDOW    0x01
#define mePOS_WINXSCRL  0x02
#define mePOS_WINXCSCRL 0x04
#define mePOS_WINYSCRL  0x08
#define mePOS_BUFFER    0x10
#define mePOS_LINEMRK   0x20
#define mePOS_LINENO    0x40
#define mePOS_LINEOFF   0x80
#define mePOS_DEFAULT   \
(mePOS_WINDOW|mePOS_WINXSCRL|mePOS_WINXCSCRL|mePOS_WINYSCRL| \
 mePOS_BUFFER|mePOS_LINEMRK|mePOS_LINEOFF)

#if NARROW
typedef	struct	meNARROW {
    struct meNARROW *next ;		/* pointer to next narrow in list	     */
    struct meNARROW *prev ;		/* pointer to previous narrow in list	     */
    LINE            *slp ;		/* pointer to narrow start line              */
    LINE            *elp ;		/* pointer to narrow end line                */
    int32            noLines ;          /* Number of lines narrowed out              */
    int32            sln ;              /* Narrows start line number                 */
    uint16           name ;		/* amark name                                */
} meNARROW ;

#endif

/*
 * Text is kept in buffers. A buffer header, described below, exists for every
 * buffer in the system. The buffers are kept in a big list, so that commands
 * that search for a buffer by name can find the buffer header. There is a
 * safe store for the dot and mark in the header, but this is only valid if
 * the buffer is not being displayed (that is, if "b_nwnd" is 0). The text for
 * the buffer is kept in a circularly linked list of lines, with a pointer to
 * the header line in "b_linep".
 * 	Buffers may be "Inactive" which means the files accosiated with them
 * have not been read in yet. These get read in at "use buffer" time.
 */
typedef struct  BUFFER {
    struct  BUFFER *b_bufp ;            /* Link to next struct BUFFER	*/
#if ABREV
    meABREV *abrevFile ;                /* Abreviation file		*/
#endif
    meAMARK *b_amark ;  		/* pointer to the mark list	*/
#if LCLBIND
    struct KEYTAB *bbinds ;		/* pointer to local bindings	*/
#endif
#if NARROW
    meNARROW *narrow ;		        /* pointer to narrow structures */
#endif
    meVARLIST varList ;               /* User local buffer variables  */
    meSTAT  stats ;                     /* file stats - inc. mod time   */
    LINE   *b_dotp;			/* Link to "." LINE structure   */
    LINE   *b_markp;			/* The same as the above two,   */
    LINE   *b_linep;			/* Link to the header LINE      */
    uint8  *b_fname ;                   /* File name                    */
    uint8  *b_bname ;                   /* Buffer name                  */
    uint8  *modeLineStr ;               /* buffer mode-line format      */
#if FLNEXT
    uint8  *nextFile ;                  /* Next's current File name     */
#endif
#if CRYPT
    uint8  *b_key;		        /* current encrypted key	*/
#endif
#if MEUNDO
    struct  UNDOND *fUndo ;             /* First undo node              */
    struct  UNDOND *lUndo ;             /* Last undo node               */
    uint32  undoContFlag ;              /* Was the last undo this com'd?*/ 
#endif
    int32   autotime;    		/* auto-save time for file	*/
    int32   topLineNo ;			/* Windows top line number      */
    int32   line_no;                    /* current line number          */
    int32   mlineno;                    /* current mark line number     */
    int32   elineno;                    /* end line number              */
    int     fhook;                      /* file hook function           */
    int     bhook;                      /* b buffer hook function       */
    int     dhook;                      /* d buffer hook function       */
    int     ehook;                      /* e buffer hook function       */
    int     inputFunc;                  /* input handle function        */
#ifdef _IPIPES
    int     ipipeFunc;                  /* ipipe input handle function  */
#endif
    int     histNo;                     /* Buff switch hist no.         */
    uint16  b_doto;                     /* Offset of "." in above LINE  */
    uint16  b_marko;                    /* but for the "mark"           */
#if LCLBIND
    uint16  nobbinds;                   /* but for the "mark"           */
#endif
    meSCHEME scheme;                    /* Current scheme index         */
    meSCHEME lscheme[LNNOSCHM];         /* line scheme index            */
    uint8    lschemeNext ;              /* Next line scheme index       */
    meMODE  b_mode;			/* editor mode of this buffer	*/
    uint8   intFlag;			/* internal buffer flags	*/
    uint8   b_nwnd;     		/* Count of windows on buffer   */
    uint8   isWordMask ;                /* isWord lookup table bit mask */
    uint8   modeLineFlags ;             /* buffer mode-line flags       */
#if HILIGHT
    uint8   hiLight ;          	        /* hilight number               */
    uint8   indent ;                    /* indent number                */
#endif
} BUFFER ;


#define	BIFBLOW    0x01		/* Buffer is to be blown away           */
#define	BIFLOAD    0x02		/* Used on a reload to check tim        */
#define	BIFLOCK    0x04		/* Used in ipipe to flag a lock         */
#define	BIFNAME    0x08		/* The buffer name has a <?> extension  */
#define	BIFFILE    0x10		/* The buffer is a file - used at creation only */
#define	BIFNODEL   0x20		/* The buffer cannot be deleted         */


/*
 * The starting position of a region, and the size of the region in
 * characters, is kept in a region structure.  Used by the region commands.
 */
typedef struct  {
    int32   size;               /* Length in characters.        */
    int32   line_no;            /* Origin LINE number.          */
    LINE   *linep;		/* Origin LINE address.         */
    uint16  offset;             /* Origin LINE offset.          */
} REGION ;


/*	structure for the table of initial key bindings		*/

typedef struct KEYTAB {
    uint16 code ;               /* Key code                     */
    uint16 index ;
    uint32 arg ;
} KEYTAB;


/*	The editor holds deleted text chunks in the KILL buffer. The
	kill buffer is logically a stream of ascii characters, however
	due to its unpredicatable size, it gets implemented as a linked
	list of chunks. (The d_ prefix is for "deleted" text, as k_
	was taken up by the keycode structure			*/

typedef	struct	KILL {
    struct KILL   *next;		/* link to next chunk, NULL if last */
    uint8  data[1] ;            /* First byte of the data (nil trm) */
} KILL;

/*
 * KILL chunks are pointed at by a linked list, which could be very long indeed,
 * saving all the text deleted per session. In practice the number of deleted
 * chunks is kept to a predefined number. A new (ie most recent) delete is
 * inserted into the head of the chain.
 *
 * The structure after a couple of deletes is therefore:
 *
 * 	--------------			------------------
 *	| klhead     |  ----------->	| KILL structure |  ---------> ....
 *	|            |	kl_kill		|  		 |
 *	--------------			------------------
 *		|			most recent delete
 *		|kl_next
 *		V
 * 	----------------		------------------
 *	| klhead->next |  ----------->	| KILL structure |  ---------> ....
 *	|              |		|  		 |
 *	----------------		------------------
 *		|			second most recent delete
 *		|
 *		V
 * 	------------------		------------------
 *	| klhead->next   | ----------->	| KILL structure |  ---------> ....
 *	|         ->next |		|  		 |
 *	------------------		------------------
 *		|			first delete in session
 *		|
 *		V
 *	      N U L L
 *
 * The final thing to know about klist structures is that the kl_type field
 * can take on one of two possible values, if set to CFKILL, then it contains
 * killed text, if not (ie it is zero) it contains deleted text. The important
 * difference between killed and deleted text is that killed text goes in
 * large chunks, whereas deleted text happens in single character throws. It
 * is therefore possible in single character deletions, to look to see what
 * type of text is stored in a given kill buffer and so know whether to
 * create a new buffer for the soon-to-be-deleted text.
 */

#define	NKILL	15		/* number of kills held in kill list	   */

typedef	struct	KLIST {
    KILL         *kill ;	/* pointer to kill chunk		   */
    struct KLIST *next ;	/* link to next list element, NULL if last */
} KLIST;


/**	list of recognized user functions	*/

#define FUN_ARG1    	0x01
#define FUN_ARG2    	0x02
#define FUN_ARG3    	0x04
#define FUN_SETVAR    	0x08
#define FUN_GETVAR    	0x10
#define FUN_MONAMIC 	FUN_ARG1
#define FUN_DYNAMIC 	(FUN_ARG1|FUN_ARG2)
#define FUN_TRINAMIC	(FUN_ARG1|FUN_ARG2|FUN_ARG3)

#if     HILIGHT

/* hilight init flags */
#define HFCASE     0x01
#define HFLOOKB    0x02

/* hilight init internal flags */
#define HFRPLCDIFF 0x8000

/* hilight token flags */
#define HLTOKEN    0x0001
#define HLENDLINE  0x0002
#define HLBRACKET  0x0004
#define HLCONTIN   0x0008
#define HLBRINEOL  0x0010
#define HLSTTLINE  0x0020
#define HLREPLACE  0x0040
#define HLBRANCH   0x0080
#define HLSOL      0x0100
#define HLVALID    0x0200
#define HLCOLUMN   0x0400
#define HLONELNBT  0x0800
#define HLSNGLLNBT 0x1000

/* hilight token internal flags */
#define HLRPLCDIFF 0x8000

/* indent init flags */
#define HICASE   0x01

typedef struct HILNODE {
    struct HILNODE **list ;
    uint8   *table ; 
    uint8   *close ;
    uint8   *rtoken ;
    uint8   *rclose ;
    uint16   type ; 
    meSCHEME scheme ;
    uint8    tknSttOff ; 
    uint8    tknEndOff ; 
    uint8    clsSttOff ; 
    uint8    clsEndOff ; 
    uint8    tknSttTst ; 
    uint8    tknEndTst ; 
    uint8    ordSize ; 
    uint8    listSize ; 
    uint8    ignore ;
    uint8    token[1] ;
} HILNODE, *HILNODEPTR ;

#define TableLower (uint8)(' ')
#define TableUpper (uint8)('z')
#define TableSize  ((TableUpper - TableLower) + 3)
#define InTable(cc) (((uint8)(cc) >= TableLower) &&  \
                     ((uint8)(cc) <= TableUpper))
#define GetTableSlot(cc)                             \
(((uint8)(cc) < TableLower) ? 0:                     \
 (((uint8)(cc) > TableUpper) ? TableSize-2:          \
  ((uint8)(cc)-TableLower+1)))
#define hilListSize(r) (r->ordSize + r->listSize)

#endif

/*
 * HILBLOCK
 * Hilighting screen structure. The structure contains blocks of style
 * information used to render the screen. Each block describes the style
 * (front and back color + font) and the column on which the style ends.
 */
typedef struct {
    uint16   column;             /* change column */
    meSCHEME scheme;             /* style index */
} HILBLOCK;    

/*
 * Selection Highlighting 
 * The selection highlighting buffer retains information about the 
 * selection highlighting for the current buffer. The current restriction
 * is that only one buffer may have selection hilighting enabled
 */

typedef struct SELHILIGHT {
    uint16 uFlags;                      /* Hilighting user flags     */
    uint16 flags;                       /* Hilighting flags          */
    struct BUFFER *bp;                  /* Selected hilight buffer   */
    int32  dlineno;                     /* Dot line number           */
    int32  mlineno;                     /* Mark line number          */
    int32  dlineoff;                    /* Current line offset       */
    int32  mlineoff;                    /* Current mark offset       */
    int    sline;                       /* Start line                */
    int    soff;                        /* Start offset              */
    int    eline;                       /* End line number           */
    int    eoff;                        /* End offset                */
} SELHILIGHT;

#define SELHIL_ACTIVE    0x0001         /* Buffer has been edited    */
#define SELHIL_FIXED     0x0002         /* Buffer has been edited    */
#define SELHIL_KEEP      0x0004         /* Buffer has been edited    */
#define SELHIL_CHANGED   0x0008         /* Hilighting been changed   */
#define SELHIL_SAME      0x0010         /* Highlighting is same point*/

#define SELHILU_ACTIVE   0x0001         /* Buffer has been edited    */
#define SELHILU_KEEP     0x0002         /* Buffer has been edited    */

#define VFMESSL 0x0001                  /* Message line */
#define VFMENUL 0x0002                  /* Menu line changed flag */
#define VFMODEL 0x0004                  /* Mode line */
#define VFMAINL 0x0008                  /* Main line */
#define VFCURRL 0x0010                  /* Current line */
#define VFCHNGD 0x0020                  /* Changed flag */
#define VFSHBEG 0x0100                  /* Start of the selection hilight */
#define VFSHEND 0x0200                  /* End of the selection hilight */
#define VFSHALL 0x0400                  /* Whole line is selection hilighted */

#define VFSHMSK 0x0700                  /* Mask of the flags  */
#define VFTPMSK 0x003f                  /* Mask of the line type */

typedef struct  VIDEO
{
    WINDOW    *wind ;
    LINE      *line ;
    uint16     endp ;
    uint16     flag ;                   /* Flags */
    meSCHEME   eolScheme ;              /* the EOL scheme */
#if     HILIGHT
    uint8      hilno ;                  /* hilight-no */
    HILNODEPTR bracket ;
#endif
} VIDEO;


/* Virtual Video Structure.
 * Provides the VIDEO structure management for horizontal split
 * Windows. A single VVIDEO exists for each VIDEO structure.
 */

#define VVROOT   0x0001                 /* This is the root video page */
#define VVFSEPUP 0x0002                 /* Vertical sepeator is out of date */

typedef struct VVIDEO
{
    VIDEO *video;                       /* Pointer to the video block */
    WINDOW *window;                     /* Windows attached to video block */
    struct VVIDEO *next;                /* Next video item */
} VVIDEO;

/* Frame Store
 * The frame store holds the physical screen structure. This comprises the
 * text information and the colour information of each character on the
 * screen.
 *
 * The frame store is required primarily on windowing platforms such as
 * MS-Windows and X-Windows and allows the canvas to be regenerated 
 * without recomputing the contents of the window when the windowing
 * sub-system requires the window (or parts of the window) to be refreshed.
 * 
 * Hence rather than EMACS generating new lines every time we render from
 * the Frame buffer which contains a representation of the screen. (Color +
 * ASCII text information).
 * 
 * Under MS-Windows we receive a WM_PAINT message. This message may be received
 * when we are in focus or not (e.g. a window placed over ours and moved
 * will cause a WM_PAINT messaage for the area of the screen which has
 * become invalid).
 */

typedef struct
{
    uint8    *text ;                  /* Text held on the line. */
    meSCHEME *scheme ;                /* index to the Style (fore + back + font) of each cell */
} FRAMELINE;                          /* Line of screen text */


#ifdef _IPIPES
/* The following is structure required for unix ipipes */

#define IPIPE_OVERWRITE   0x01
#define IPIPE_NEXT_CHAR   0x02
#define IPIPE_CHILD_EXIT  0x04

typedef struct meIPIPE {
    BUFFER    *bp ;
    struct meIPIPE *next ;
    int        pid ;
#ifdef _WIN32
    HANDLE     rfd ;
    HANDLE     outWfd ;
    HANDLE     process ;
    DWORD      processId ;
    HWND       childWnd ;
    /* wait thread variables */
    HANDLE     childActive ;
    HANDLE     threadContinue ;
    HANDLE     thread ;
    DWORD      threadId ;
    uint8      nextChar ;
#else
    int        rfd ;
    int        outWfd ;
#endif
    int16      noRows ;
    int16      noCols ;
    int16      strRow ;
    int16      strCol ;
    int16      curRow ;
    int16      flag ;
} meIPIPE ;

#endif

#if MEUNDO

typedef int UNDOCOORD[2] ;

typedef struct UNDOND {
    struct UNDOND *next ;
    union {
        int32      dotp ;
        UNDOCOORD *pos ;
    } udata ;
    int32 count ;
    uint16 doto ;
    uint8 type ;
    uint8 str[1] ;
} UNDOND ;

#define MEUNDO_MDEL 0x01
#define MEUNDO_MINS 0x02
#define MEUNDO_REVS 0x04
#define MEUNDO_SING 0x08
#define MEUNDO_CONT 0x10
#define MEUNDO_FRST 0x20
#define MEUNDO_REPL 0x40
#define MEUNDO_NRRW 0x80

#endif

/* The variable register (#0 - #9) uses a linked structure
 * This is so they can easily be pushed and poped
 */
#define meNUMREG 10
typedef struct meREGISTERS {
    struct meREGISTERS *prev ;
    meVARLIST *varList ;
    uint8 *execstr ;
    int   f, n ;
    int   force ;
    uint8 reg[meNUMREG][MAXBUF] ;
} meREGISTERS ;


/* Note that the first part of meDIRLIST structure must remain
 * the same as meNAMESVAR as it is used for $file-names variable
 */
typedef struct {
    int     exact ;
    int     size ;
    uint8 **list ;
    uint8  *mask ;
    int     curr ;
    uint8  *path ;
    int     timeStamp ;
}  meDIRLIST ;

typedef struct {
    int     exact ;
    int     size ;
    uint8 **list ;
    uint8  *mask ;
    int     curr ;
} meNAMESVAR ; 

#if REGSTRY
/*
 * REGISTRY
 * ========
 * Definitions for the registry API. The structures remain private
 * within the registry source file, providing a API interface for
 * maintainability.
 */

/*
 * REG_HANDLE
 * Generates a registry ASCII handle which is overlayed into a
 * 32-bit integer. The handle is the lookup from the macro 
 * interface.
 */
#define REG_HANDLE(a,b,c,d) ((((uint32)(a))<<24)|(((uint32)(b))<<16)| \
                             (((uint32)(c))<<8)|((uint32)(d)))

/* Registry Open types */
#define REGMODE_INTERNAL 0x001           /* Internal registry - hidden */
#define REGMODE_HIDDEN   0x002           /* Node is hidden */
#define REGMODE_FROOT    0x004           /* File root */
#define REGMODE_CHANGE   0x008           /* Tree has changed */
#define REGMODE_BACKUP   0x010           /* Perform a backup of the file */
#define REGMODE_AUTO     0x020           /* Automatic save */
#define REGMODE_DISCARD  0x040           /* Discardable entry (memory only) */
#define REGMODE_MERGE    0x080           /* Merge a loaded registry */
#define REGMODE_RELOAD   0x100           /* Reload existing registry */
#define REGMODE_CREATE   0x200           /* Create if does not exist */
#define REGMODE_QUERY    0x400           /* Query the current node */
#define REGMODE_GETMODE  0x800           /* Return modes set in $result */
#define REGMODE_STORE_MASK 0x07f         /* Bits actually worth storing */
/*
 * RNODE
 * Data structure to hold a hierarchy node
 */
typedef struct RNODE
{
    uint8 *value;                       /* The value of the node */
    struct RNODE *prnt;                 /* Pointer to the parent */
    struct RNODE *chld;                 /* Pointer to the child node */
    struct RNODE *sblg;                 /* Pointer to the sibling node */
    uint8 mode;                         /* Mode flag */
    uint8 name[1];                      /* The name of the node */
} RNODE;
/*
 * REGHANDLE: Public anonymous typeless handle to the registry node.
 */
typedef struct RNODE *REGHANDLE;
#endif


