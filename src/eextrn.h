/****************************************************************************
 * External function definitions
 *
 * Last Modified:       <010802.1939>
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
#if	!(defined __ABREVC) || (defined _ANSI_C)		/* ABREV.C externals */
#if ABREV
extern	int	bufferAbbrev APRAM((int f, int n));
extern	int	globalAbbrev APRAM((int f, int n));
extern	int	expandAbbrev APRAM((int f, int n));
#else
#define bufferAbbrev notAvailable
#define globalAbbrev notAvailable
#define expandAbbrev notAvailable
#endif
#endif
/*
 * basic.c
 */
#if	!(defined __BASICC) || (defined _ANSI_C)		/* BASIC.C externals */
extern	int	gotobol APRAM((int f, int n));
extern	int	gotoeol APRAM((int f, int n));
extern	int	eobError APRAM((void)) ;
extern	int	bobError APRAM((void)) ;
extern	int	WbackChar APRAM((register WINDOW *wp, register int n)) ;
extern	int	WforwChar APRAM((register WINDOW *wp, register int n)) ;
extern	int	backChar APRAM((int f, int n));
extern	int	forwChar APRAM((int f, int n));
extern	uint8   getCurChar APRAM((WINDOW *wp)) ;
extern	int	gotoLine APRAM((int f, int n));
#if NARROW
extern	int	gotoAbsLine APRAM((int32 line)) ;
#else
#define gotoAbsLine(l) gotoLine(1,l)
#endif
extern	int	topbot APRAM((int f, int n));
extern	int	gotobob APRAM((int f, int n));
extern	int	gotoeob APRAM((int f, int n));
extern	int	forwLine APRAM((int f, int n));
extern	int	backLine APRAM((int f, int n));
#if	WORDPRO
extern	int	backPara APRAM((int f, int n));
extern	int	forwPara APRAM((int f, int n));
#else
#define backPara notAvailable
#define forwPara notAvailable
#endif
extern	int	setMark APRAM((int f, int n));
extern	int	swapmark APRAM((int f, int n));
extern	void	bufferPosStore APRAM((LINE *lp, uint16 lo, int32 ln)) ;
extern	void	bufferPosUpdate APRAM((BUFFER *bp, uint32 noLines, uint16 newOff)) ;
#endif
/*
 * bind.c
 */
#if !(defined __BINDC) || (defined _ANSI_C)		/* BIND.C Externals */
extern  uint32  cmdHashFunc APRAM((register uint8 *cmdName)) ;
extern  uint16  meGetKeyFromString APRAM((uint8 **tp)) ;
extern  uint16  meGetKey APRAM((int flag)) ;
extern	int	descKey APRAM((int f,int n));
extern  int     meGetStringFromChar APRAM((uint16 cc, uint8 *d)) ;
extern	void	meGetStringFromKey APRAM((uint16 cc, uint8 *seq));
extern	int     decode_fncname APRAM((uint8 *fname, int silent));
extern	int	bindkey APRAM((uint8 *prom, int f, int n, uint16 *lclNoBinds,
                               KEYTAB **lclBinds)) ;
extern	int	unbindkey APRAM((uint8 *prom, int n, uint16 *lclNoBinds,
                                 KEYTAB **lclBinds)) ;
extern	int	globalBind APRAM((int f, int n));
extern	int	globalUnbind APRAM((int f, int n));
#if LCLBIND
extern	int	bufferBind APRAM((int f, int n));
extern	int	bufferUnbind APRAM((int f, int n));
extern	int	mlBind APRAM((int f, int n));
extern	int	mlUnbind APRAM((int f, int n));
#else
#define bufferBind notAvailable
#define bufferUnbind notAvailable
#define mlBind notAvailable
#define mlUnbind notAvailable
#endif
extern	int	listCommands APRAM((int f, int n));
extern	int	descBindings APRAM((int f, int n));
extern  void    charMaskTblInit APRAM((void));
extern	int	setCharMask APRAM((int f, int n));
extern	int	commandApropos APRAM((int f, int n));
#endif
/*
 * buffer.c
 */
#if !(defined __BUFFERC) || (defined _ANSI_C)		/* BUFFER.C Externals */
extern  int     getBufferName APRAM((uint8 *prompt, int opt, int defH, uint8 *buf)) ;
extern	int	findBuffer APRAM((int f, int n));
extern	int	nextWndFindBuf APRAM((int f, int n));
extern	int	nextBuffer APRAM((int f, int n));
extern	int	swbuffer APRAM((WINDOW *wp, BUFFER *bp));
extern	int	delBuffer APRAM((int f, int n));
extern	int	delSomeBuffers APRAM((int f, int n));
extern  BUFFER *replacebuffer APRAM((BUFFER *oldbuf)) ;
extern	int	HideBuffer APRAM((BUFFER *bp)) ;
extern  void    linkBuffer APRAM((BUFFER *bp)) ;
extern  void    unlinkBuffer APRAM((BUFFER *bp)) ;
extern	int	zotbuf APRAM((BUFFER *bp, int silent));
extern	int	changeBufName APRAM((int f,int n));
extern	int	adjustMode APRAM((BUFFER *bp, int nn));
extern	int	bufferMode APRAM((int f, int n));
extern	int	globalMode APRAM((int f, int n));
extern	int	namedBufferMode APRAM((int f, int n));
extern  int     addLine APRAM((register LINE *ilp, uint8 *text)) ;
#define addLineToBob(bp,text) (bp->elineno += addLine(bp->b_dotp,text))
#define addLineToEob(bp,text) (bp->elineno += addLine(bp->b_linep,text))
extern	int	listBuffers APRAM((int f, int n));
extern  int     bufferNeedSaving APRAM((BUFFER *bp)) ;
extern	int	anyChangedBuffer APRAM((void));
extern  BUFFER *createBuffer APRAM((register uint8 *bname)) ;
#define BFND_CREAT  0x01
#define BFND_MKNAM  0x02
#define BFND_BINARY 0x10
#define BFND_CRYPT  0x20
#define BFND_CLEAR  0x40

extern	BUFFER *bfind APRAM((uint8 *bname, int cflag));
extern	void    resetBufferWindows APRAM((BUFFER *bp)) ;
extern	int	bclear APRAM((BUFFER *bp));
extern  int     getBufferInfo APRAM((int32 *,int32 *,int32 *,int32 *)) ;
#endif
/*
 * crypt.c
 */
#if !(defined __CRYPTC) || (defined _ANSI_C)				/* CRYPT.C Externals */
#if	CRYPT
extern  int	setBufferCryptKey APRAM((BUFFER *bp, uint8 *key)) ;
extern	int	setCryptKey APRAM((int f, int n));
extern	int	meCrypt APRAM((uint8 *bptr, uint32 len));
#else
#define setCryptKey notAvailable
#define meCrypt notAvailable
#endif
#endif
/*
 * dir.c        - Directory handling
 */
#if !(defined __DIRLISTC) || (defined _ANSI_C)		/* DIRLIST.C Externals */
extern int directoryTree APRAM((int f, int n));
#endif
/*
 * display.c
 */
#if !(defined __DISPLAYC) || (defined _ANSI_C)		/* DISPLAY.C Externals */
extern	void	vtinit APRAM((void));
extern	void	vttidy APRAM((void));
extern int	addColor APRAM((int f, int n)) ;
extern	int	addColorScheme APRAM((int f, int n));
extern  int     convertUserScheme APRAM((int n, int defaultScheme));
extern  uint8   assessModeLine APRAM((uint8 *ml)) ;
extern  void    windCurLineOffsetEval APRAM((WINDOW *wp)) ;
extern  void    reframe APRAM((WINDOW *wp)) ;
extern  void    updCursor APRAM((register WINDOW *wp)) ;
extern  int     renderLine APRAM((uint8 *s, int len, int wid));
extern	int	screenUpdate APRAM((int f, int n));
extern	int	update APRAM((int force));
extern	void	updone APRAM((WINDOW *wp));
extern	void	updall APRAM((WINDOW *wp));
extern	int     showCursor APRAM((int f, int n)) ;
extern	int     showRegion APRAM((int f, int n)) ;
#define POKE_NOMARK   0x01      /* Don't mark the poke */
#define POKE_NOFLUSH  0x02      /* Don't flush the poke */
#define POKE_COLORS   0x04      /* The fore & back args are color strings */
extern  void    pokeScreen APRAM((int flags, int row, int col, uint8 *scheme, uint8 *str)) ;
extern	int	screenPoke APRAM((int f, int n)) ;
extern  void    menuline APRAM((int flag));

extern  int     renderLine APRAM((uint8 *s1, int len, int wid)) ;

/* Virtual video interfaces */
extern  int     vvideoAttach  APRAM ((VVIDEO *vvptr, WINDOW *wp));
extern  void    vvideoDetach  APRAM ((WINDOW *wp));
extern  int     vvideoEnlarge APRAM ((int rows));

extern  int     doRedrawEvent APRAM ((void));

#define MWCURSOR   0x001
#define MWSPEC     0x002
#define MWABORT    0x004
#define MWPAUSE    0x008
#define MWWAIT     0x010
#define MWCLEXEC   0x020
#define MWCLWAIT   0x040
#define MWUSEMLCOL 0x080
#define MWERASE    0x100

extern  int     mlwrite APRAM((int flags, uint8 *fmt, ...)) ;
#endif
/*
 * eval.c
 */
#if !(defined __EVALC) || (defined _ANSI_C)		/* EVAL.C Externals */
extern  int     regexStrCmp APRAM((uint8 *str, uint8 *reg, int exact)) ;
extern	uint8  *gtfun APRAM((uint8 *fname));
extern  uint8  *getUsrLclCmdVar APRAM((uint8 *vname, register meVARLIST *varList)) ;
#define getUsrVar(vname) getUsrLclCmdVar(vname,&usrVarList)
extern	int	descVariable APRAM((int f, int n));
extern	int	setVar APRAM((uint8 *vname, uint8 *vvalue, meREGISTERS *regs)) ;
extern	int	setVariable APRAM((int f, int n));
extern	int	unsetVariable APRAM((int f, int n));
extern	uint8  *meItoa APRAM((int i));
extern	uint8  *getval APRAM((uint8 *token));
extern	int	listVariables APRAM((int f, int n));
#endif
/*
 * exec.c
 */
#if !(defined __EXECC) || (defined _ANSI_C)		/* EXEC.C Externals */
extern	int	mePushRegisters APRAM((int flags));
extern	int	mePopRegisters APRAM((int flags));
extern  int     biChopFindString APRAM((register uint8 *ss, register int len, 
                                        register uint8 **tbl, register int size)) ;
extern	int	execFunc APRAM((int index, int f, int n)) ;
extern  void    execFuncHidden APRAM((int keyCode, int index, uint32 arg)) ;
#define meEBF_ARG_GIVEN   0x01
#define meEBF_HIDDEN      0x02
extern  void    execBufferFunc APRAM((BUFFER *bp, int index, int flags, int n)) ;
extern  int     lineExec APRAM((int f, int n, uint8 *cmdstr));
/* Note  tok (the destination token string) must be TOKENBUF in size, 
 * returning a string no bigger than MAXBUF with the \0 */
extern	uint8  *token APRAM((uint8 *src, uint8 *tok));
extern	int	macarg APRAM((uint8 *tok));
extern  int     meGetString APRAM((uint8 *prompt, int option, int defnum,
                               uint8 *buffer, int size));
extern	int	storemac APRAM((int f, int n));
extern	int	dofile APRAM((uint8 *fname, int f, int n));
extern	int	execCommand APRAM((int f, int n));
extern	int	execLine APRAM((int f, int n));
extern	int	execBuffer APRAM((int f, int n));
extern	int	execFile APRAM((int f, int n));
extern	int	startup APRAM((uint8 *sfname));
#endif
/*
 * file.c
 */
#if !(defined __FILEC) || (defined _ANSI_C)		/* FILE.C Externals */
extern  int fnamecmp APRAM((uint8 *f1, uint8 *f2)) ;
#define meFILETYPE_NASTY      0
#define meFILETYPE_REGULAR    1
#define meFILETYPE_DIRECTORY  2
#define meFILETYPE_NOTEXIST   3
#define meFILETYPE_HTTP       4
#define meFILETYPE_FTP        5
extern  int getFileStats APRAM((uint8 *file, int flag, meSTAT *stats, uint8 *lname)) ;
extern  void set_dirs APRAM((void)) ;
#define meFL_CHECKDOT    0x01
#define meFL_USESRCHPATH 0x02
#define meFL_USEPATH     0x04
#define meFL_EXEC        0x08
extern	int fileLookup APRAM((uint8 *fname, uint8 *ext, uint8 flags, uint8 *outName)) ;
extern	int executableLookup APRAM((uint8 *fname, uint8 *outName)) ;
extern  int bufferOutOfDate APRAM((BUFFER *bp)) ;
extern	uint8 *gwd APRAM((uint8 drive));
extern  uint8 *getFileBaseName APRAM((uint8 *fname)) ;
extern  void  getFilePath APRAM((uint8 *fname, uint8 *path)) ;
extern  int inputFileName APRAM((uint8 *prompt, uint8 *fn, int corFlag));
extern	int ifile APRAM((BUFFER *bp, uint8 *fname, uint32 flags));
extern	int insFile APRAM((int f, int n));
extern  int findFileList APRAM((uint8 *seed, int bflag, int32 lineno)) ;
extern  int findSwapFileList APRAM((uint8 *seed, int bflag, int32 lineno)) ;
extern	int findFile APRAM((int f, int n));
extern	int findBFile APRAM((int f, int n));
extern	int findCFile APRAM((int f, int n));
extern	int readFile APRAM((int f, int n));
extern	int nextWndFindFile APRAM((int f, int n));
extern	int viewFile APRAM((int f, int n));
extern	int fileOp APRAM((int f, int n));
extern  void freeFileList APRAM((int noStr, uint8 **files)) ;
#if	CRYPT
extern	int	resetkey APRAM((BUFFER *bp));
#endif
extern	int	readin APRAM((BUFFER *, uint8 *fname));
extern	uint8   makename APRAM((uint8 *bname, uint8 *fname));
extern  void    autowriteout APRAM((register BUFFER *bp)) ;
extern  void    autowriteremove APRAM((BUFFER *bp)) ;
extern	int	appendBuffer APRAM((int f, int n));
extern	int	writeBuffer APRAM((int f, int n));
extern	int	saveBuffer APRAM((int f, int n));
extern	int	writeout APRAM((BUFFER *bp, int flags, uint8 *fn));
extern	int     saveSomeBuffers APRAM((int f, int n)) ;
extern	void	resetBufferNames APRAM((BUFFER *bp, uint8 *fname));
extern	int	changeFileName APRAM((int f, int n));
extern	int	changeDir APRAM((int f, int n));
#ifdef _CONVDIR_CHAR
extern  void    fileNameConvertDirChar APRAM((uint8 *fname)) ;
#else
#define fileNameConvertDirChar(ff)
#endif
#define PATHNAME_COMPLETE 0
#define PATHNAME_PARTIAL  1
extern  void    pathNameCorrect APRAM((uint8 *oldName, int nameType, 
                                       uint8 *newName, uint8 **baseName)) ;
#ifdef _WIN32
extern  void    fileNameCorrect APRAM((uint8 *oldName, uint8 *newName, uint8 **baseName)) ;
#else
#define fileNameCorrect(o,n,b) pathNameCorrect(o,PATHNAME_COMPLETE,n,b)
#endif
extern  void    getDirectoryList APRAM((uint8 *pathName, meDIRLIST *dirList)) ;

#endif

/*
 * fileio.c
 */
#if !(defined __FILEIOC) || (defined _ANSI_C)		/* FILEIO.C Externals */
#define meRWFLAG_FMOD      0x0000ffff
#define meRWFLAG_SILENT    0x00010000
#define meRWFLAG_READ      0x00020000
#define meRWFLAG_INSERT    0x00040000
#define meRWFLAG_WRITE     0x00080000
#define meRWFLAG_BACKUP    0x00100000
#define meRWFLAG_OPENEND   0x00200000
#define meRWFLAG_OPENTRUNC 0x00400000
#define meRWFLAG_AUTOSAVE  0x00800000
#define meRWFLAG_CHKBREAK  0x01000000
#define meRWFLAG_IGNRNRRW  0x02000000
/* following using in ffCopyFile to remove the source file & create a dir */
#define meRWFLAG_DELETE    0x04000000
#define meRWFLAG_MKDIR     0x08000000
#define meRWFLAG_FTPCLOSE  0x10000000
#define meRWFLAG_NOCONSOLE 0x20000000

extern int      ffReadFile APRAM((uint8 *fname, uint32 flags, BUFFER *bp, LINE *hlp)) ;
#define meBACKUP_CREATE_PATH 0x0001
extern int      createBackupName APRAM((uint8 *filename, uint8 *fn, uint8 backl, int flag)) ;


extern int      ffWriteFileOpen APRAM((uint8 *fname, uint32 flags, BUFFER *bp)) ;
extern int      ffWriteFileWrite APRAM((register int len, 
                                        register uint8 *buff, int eolFlag)) ;
extern int      ffWriteFileClose APRAM((uint8 *fname, uint32 flags, BUFFER *bp)) ;
extern int      ffWriteFile APRAM((uint8 *fname, uint32 flags, BUFFER *bp)) ;
extern int	ffFileOp APRAM((uint8 *sfname, uint8 *dfname, uint32 dFlags)) ;
#endif
/*
 * input.c
 */
#if !(defined __INPUTC) || (defined _ANSI_C)		/* INPUT.C Externals */
#define mlCR_LOWER_CASE     0x01
#define mlCR_QUIT_ON_USER   0x02
#define mlCR_UPDATE_ON_USER 0x04
#define mlCR_CURSOR_IN_MAIN 0x08
#define mlCR_QUOTE_CHAR     0x10
#define mlCR_ALPHANUM_CHAR  0x20
extern	int     mlCharReply APRAM((uint8 *prompt, int mask, uint8 *validList, uint8 *helpStr)) ;
extern	int	mlyesno APRAM((uint8 *prompt));
extern	void	mlDisp APRAM((uint8 *prompt, uint8 *buf, uint8 *cont, int cpos)) ;
extern	int	getexecCommand APRAM((void));
#define meGETKEY_SILENT     0x01
#define meGETKEY_SINGLE     0x02
#define meGETKEY_COMMAND    0x04
extern	uint16  meGetKeyFromUser APRAM((int f, int n, int flag));
extern  int     createBuffList APRAM((uint8 ***listPtr, int noHidden)) ;
extern  int     createCommList APRAM((uint8 ***listPtr, int noHidden)) ;
extern  int     createVarList  APRAM((uint8 ***listPtr)) ;
#define MLBUFFER     0x00001		/* entering a buffer name	     */
#define MLCOMMAND    0x00002		/* entering a command		     */
#define MLFILE       0x00004		/* entering a filename		     */
#define MLSEARCH     0x00008		/* entering a search string          */
#define	MLVARBL      0x00010		/* entering a variable               */
#define	MLUSER       0x00020		/* entering a mode                   */
#define MLNOHIST     0x00040            /* Don't use the history (wont store */
#define MLUPPER      0x00080		/* convert chars to uppercase	     */
#define MLLOWER      0x00100		/* convert chars to lowercase	     */
#define	MLNOSPACE    0x00200		/* no spaces allowed		     */
#define	MLNOSTORE    0x00400		/* no history storing		     */
#define	MLISEARCH    0x00800		/* Special incremental search mode   */
#define	MLNORESET    0x01000		/* don't reset the buffer            */
#define	MLMACHIST    0x02000		/* Add output to history even if mac */
#define	MLMACNORT    0x04000		/* dont reset the buffer even if mac */
#define MLINSENSCASE 0x08000            /* ignore letter case in completion  */
#define MLFFZERO     0x10000            /* en/disable \x00 -> \xff encode    */

/* setup a #define with the correct flags for getting a filename, this must
 * consider where the platform file system only has one case and where it
 * is case insensitive
 */
#ifdef _SINGLE_CASE
/* Typically dos */
#define MLFILECASE   (MLFILE|MLLOWER)
#define MLBUFFERCASE MLBUFFER
#else
#ifdef _INSENSE_CASE
/* Typically windows */
#define MLFILECASE   (MLFILE|MLINSENSCASE)
#define MLBUFFERCASE (MLBUFFER|MLINSENSCASE)
#else
/* Typically unix */
#define MLFILECASE   MLFILE
#define MLBUFFERCASE MLBUFFER
#endif
#endif
/* Following must be setup when using MLUSER, mlgsStrList must be an
 * array of completion strings, and mlgsStrListSize must be a count
 * of the number of completions. Not the completion array is sorted!
 */
extern uint8 **mlgsStrList ;
extern int mlgsStrListSize ;
extern int meGetStringFromUser APRAM((uint8 *prompt, int option, int defnum, uint8 *buf, int nbuf)) ;
#endif
/*
 * hilight.c
 */
#if !(defined __HILIGHTC) || (defined _ANSI_C)		/* HILIGHT.C Externals */
extern	void	mlerase APRAM((int flag));
#if HILIGHT
extern	int	indent APRAM((int f, int n));
extern	int	hilight APRAM((int f, int n));
extern  void    hilightCurLineOffsetEval APRAM((WINDOW *wp)) ;
extern  int     indentLine APRAM((void)) ;
extern  uint16 hilightLine APRAM((VIDEO *vp1)) ;
extern  void    hilightLookBack APRAM((WINDOW *)) ;
#else
#define indent notAvailable
#define hilight notAvailable
#endif
#endif
/*
 * history.c
 */
#if !(defined __HISTORYC) || (defined _ANSI_C)		/* HISTORY.C Externals */
extern  void    initHistory APRAM((void)) ;
extern  int     setupHistory APRAM((int option, uint8 **numPtr, uint8 ***list)) ;
extern  void    addHistory APRAM((int option, uint8 *str)) ;
#if REGSTRY
extern  int     readHistory APRAM((int f, int n)) ;
extern  int     saveHistory APRAM((int f, int n)) ;
#else
#define readHistory notAvailable
#define saveHistory notAvailable
#endif
#endif
/*
 * isearch.c
 */
#if !(defined __ISEARCHC) || (defined _ANSI_C)		/* ISEARCH.C Externals */
#if ISRCH
extern	int	isearchBack APRAM((int f, int n));
extern	int	isearchForw APRAM((int f, int n));
#else
#define isearchBack notAvailable
#define isearchForw notAvailable
#endif
#endif
/*
** key.c
*/
#if !(defined __KEYC) || (defined _ANSI_C)			/* KEY.C Externals */
extern  void    count_key_table APRAM((void)) ;
extern  int     decode_key APRAM((register uint16 code, uint32 *arg)) ;
extern  int     delete_key APRAM((register uint16 code)) ;
extern  int	insert_key APRAM((register uint16 code, uint16 index, uint32 arg)) ;
#endif
/*
 * line.c
 */
#if !(defined __LINEC) || (defined _ANSI_C)		/* LINE.C Externals */
extern	LINE    *lalloc APRAM((int used));
extern	int	bchange APRAM((void));
extern	void	lchange APRAM((int flag));
extern	int	insSpace APRAM((int f, int n));
extern	int	insTab APRAM((int f, int n));
extern  void	lunmarkBuffer APRAM((BUFFER *bp, LINE *lp, LINE *nlp));
extern  uint8  *lmakespace APRAM((int n));
extern	int	linsert APRAM((int n, int c));
extern	int	lsinsert  APRAM((int n, uint8 *cp));
extern	int	lnewline APRAM((void));
extern	int	mldelete APRAM((int32 n, uint8 *kstring));
extern	int	ldelete APRAM((int32 n, int kflag));
extern	int	ldelnewline APRAM((void));
extern	int	ksave APRAM((void));
extern	uint8  *kaddblock APRAM((int32 count));
extern	int	yankfrom APRAM((struct KLIST *pklist));
extern	int	yank APRAM((int f, int n));
extern	int	reyank APRAM((int f, int n));
extern  void    freeLineLoop APRAM((LINE *lp, int flag)) ;
#endif
/*
 * maro.c
 */
#if !(defined __MACROC) || (defined _ANSI_C)		/* MACRO.C Externals */
extern	int	macroDefine APRAM((int f, int n));
extern	int	macroFileDefine APRAM((int f, int n));
extern	int	macroHelpDefine APRAM((int f, int n));
extern  void    helpBufferReset APRAM((BUFFER *bp));
extern	int	help APRAM((int f, int n));
extern	int	helpItem APRAM((int f, int n));
extern  int	helpCommand APRAM((int f, int n)) ;
extern  int	helpVariable APRAM((int f, int n)) ;
extern	int	startKbdMacro APRAM((int f, int n));
extern	int	endKbdMacro APRAM((int f, int n));
extern	int	execKbdMacro APRAM((int f, int n));
extern	int	stringExec APRAM((int f, int n, uint8 *macro)) ;
extern	int	execString APRAM((int f, int n));
extern  int     nameKbdMacro APRAM((int f, int n)) ;
extern	int	macroQuery APRAM((int f, int n));
extern  meMACRO *userGetMacro APRAM((uint8 *buf, int len)) ;
extern  int     insMacro APRAM((int f, int n)) ;
#endif /* __MACROC */
/*
 * main.c
 */
#if !(defined __MAINC) || (defined _ANSI_C)		/* MAIN.C Externals */
/*extern	void	edinit APRAM((char *bname)); */
extern  int     insertChar APRAM((register int c, register int n)) ;
extern  void    doOneKey APRAM((void)) ;
extern	int	execute APRAM((int c, int f, int n));
extern	int	meAbout APRAM((int f, int n));
extern	int	exitEmacs APRAM((int f, int n));
extern	int	quickExit APRAM((int f, int n));
extern  int     promptSaveAll APRAM((int f, int n)) ;
extern	int	saveExitEmacs APRAM((int f, int n));
extern	int	ctrlg APRAM((int f, int n));
extern	int	notAvailable APRAM((int f, int n));
extern  int     noMarkSet APRAM((void)) ;
extern	int	rdonly APRAM((void));
extern	int     voidFunc APRAM((int f, int n)) ;
extern	int	resterr APRAM((void));
extern	void	prefixFunc APRAM((void));
extern	void	uniArgument APRAM((void));
extern  void    mesetup APRAM((int argc, char *argv[]));
#if (defined _UNIX) || (defined _WIN32)
extern  int     meDie APRAM((void)) ;
#endif
extern  void    autoSaveHandler APRAM((void)) ;
extern  void    callBackHandler APRAM((void)) ;
extern	int     commandWait APRAM((int f, int n)) ;

#ifndef NDEBUG
extern  void    _meAssert APRAM((char *file, int line));
#define meAssert(x) while(!(x)){_meAssert(__FILE__,__LINE__);break;}
#else
#define meAssert(x) /* Nothing (x) */
#endif
#endif
/*
 * narrow.c
 */
#if !(defined __NARROWC) || (defined _ANSI_C)		/* NARROW.C Externals */
#if NARROW
extern void createNarrow APRAM((BUFFER *bp, LINE *slp, LINE *elp, int32 sln, int32 eln, uint16 name)) ;
extern void delSingleNarrow APRAM((BUFFER *bp, int useDot)) ;
extern void removeNarrow APRAM((BUFFER *bp, register meNARROW *nrrw, int useDot)) ;
extern void unnarrowBuffer APRAM((BUFFER *bp)) ;
extern void redoNarrowInfo APRAM((BUFFER *bp)) ;
extern int  narrowBuffer APRAM((int f, int n)) ;
#else
#define narrowBuffer notAvailable
#endif
#endif
/*
 * next.c
 */
#if !(defined __NEXTC) || (defined _ANSI_C)		/* NEXT.C Externals */
#if FLNEXT
extern int      getNextLine APRAM((int f, int n)) ;
extern int      addNextLine APRAM((int f, int n)) ;
#else
#define getNextLine notAvailable
#define addNextLine notAvailable
#endif
extern int      addFileHook APRAM((int f, int n)) ;
#if DORCS
extern int      rcsFilePresent APRAM((uint8 *fname)) ;
extern int      doRcsCommand APRAM((uint8 *fname, register uint8 *comStr)) ;
extern int      rcsCiCoFile APRAM((int f, int n)) ;
#else
#define rcsCiCoFile notAvailable
#endif
#if (defined _XTERM) || (defined _DOS) || (defined _WIN32)
extern int      changeFont APRAM((int f, int n));
#else
#define changeFont notAvailable
#endif
#endif

/*
 * osd.c
 */
#if !(defined __OSDC) || (defined _ANSI_C)              /* OSD.C Externals */
#if MEOSD
extern void     osdStoreAll APRAM((void)) ;
extern void     osdRestoreAll APRAM((int)) ;
extern void     osdDisp APRAM((uint8 *buf, uint8 *cont, int cpos)) ;
extern int      osdMouseContextChange APRAM((int leftPick)) ;
extern int      osdMainMenuCheckKey APRAM((int cc)) ;
extern int      osd APRAM((int f, int n));
extern void     osdMainMenuUpdate APRAM((int force)) ;
#if LCLBIND
extern	int	osdBind APRAM((int f, int n));
extern	int	osdUnbind APRAM((int f, int n));
#else
#define osdBind notAvailable
#define osdUnbind notAvailable
#endif
#else
#define osd notAvailable
#define osdBind notAvailable
#define osdUnbind notAvailable
#endif
#endif
/*
 * print.c
 */
#if !(defined __PRINTC) || (defined _ANSI_C)              /* PRINT.C Externals */
#if PRINT
extern  int     printColor APRAM ((int f, int n));
extern  int     printScheme APRAM ((int f, int n));
extern	int	printBuffer APRAM((int f, int n)) ;
extern	int	printRegion APRAM((int f, int n)) ;
#ifdef _WIN32
extern  int     WinPrint APRAM((uint8 *name, LINE *phead));
#endif /* _WIN32 */
#else
#define printColor notAvailable
#define printScheme notAvailable
#define printBuffer notAvailable
#define printRegion notAvailable
#endif /* PRINT */
#endif
/*
 * random.c
 */
#if !(defined __RANDOMC) || (defined _ANSI_C)		/* RANDOM.C Externals */
extern  void   *meMalloc APRAM((size_t s)) ;
extern  void   *meRealloc APRAM((void *, size_t s)) ;
extern  int     stricmp APRAM((const char *str1, const char *str2)) ;
extern  int     strnicmp APRAM((const char *str1, const char *str2, size_t)) ;
extern  int     stridif APRAM((const char *str1, const char *str2)) ;
extern  void    sortStrings APRAM((int noStr, uint8 **strs, int offset, Fintss cmpFunc)) ;
extern  int     sortLines APRAM((int f, int n)) ;
extern	int	bufferInfo APRAM((int f, int n));
extern	int	getcline APRAM((WINDOW *wp));
extern	int	getcol APRAM((uint8 *ss, int off));
#define getccol() getcol(curwp->w_dotp->l_text,curwp->w_doto)
extern	int	setccol APRAM((int pos));
extern	int	getcwcol APRAM((void));
extern	int	setcwcol APRAM((int pos));
extern	int	transChars APRAM((int f, int n));
extern	int	transLines APRAM((int f, int n));
extern	int	quoteKeyToChar APRAM((uint16 c)) ;
extern	int	quote APRAM((int f, int n));
extern	int	meTab APRAM((int f, int n));
extern	int	meBacktab APRAM((int f, int n));
extern	int	insLine APRAM((int f, int n));
extern	int	meLineSetIndent APRAM((int curInd, int newInd, int undo));
extern	int	meNewline APRAM((int f, int n));
extern	int	deblank APRAM((int f, int n));
extern	int	forwDelChar APRAM((int f, int n));
extern	int	backDelChar APRAM((int f, int n));
extern	int	killLine APRAM((int f, int n));
extern	int	mlClear APRAM((int f, int n));
extern	int	mlWrite APRAM((int f, int n));
extern  void    makestrlow APRAM((uint8 *str));
#if	CFENCE
extern  uint8   gotoFrstNonWhite APRAM((void)) ;
extern  int     doCindent APRAM((int *inComment)) ;
extern	int	cinsert APRAM((void));
extern	int	gotoFence APRAM((int f, int n));
#else
#define gotoFence notAvailable
#endif
#if WORDPRO
extern  int	winsert APRAM((void)) ;
#endif
extern	int	insString APRAM((int f, int n));

#define meAM_ABSLINE  0x0100
#define meAM_EXECBUFF 0x0101
#define meAM_FRSTNRRW 0x0102
#define meAM_FRSTPOS  0x4000
extern  int     alphaMarkGet APRAM((BUFFER *bp, uint16 name)) ;
extern  int     alphaMarkSet APRAM((BUFFER *bp, uint16 name, LINE *lp,
                                    uint16 off, int silent)) ;
extern	int	setAlphaMark APRAM((int f, int n));
extern	int	gotoAlphaMark APRAM((void));
extern  int     insFileName APRAM ((int f, int n)) ;
extern  int     cmpBuffers APRAM ((int f, int n)) ;
extern  int     createCallback APRAM ((int f, int n)) ;
extern  int     setCursorToMouse APRAM ((int f, int n)) ;
#endif
/*
 * region.c
 */
#if !(defined __REGIONC) || (defined _ANSI_C)		/* REGION.C Externals */
extern	int	getregion APRAM((REGION *rp));
extern	int	killRegion APRAM((int f, int n));
extern	int	copyRegion APRAM((int f, int n));
extern	int	lowerRegion APRAM((int f, int n));
extern	int	upperRegion APRAM((int f, int n));
extern	int	killRectangle APRAM((int f, int n));
extern	int	yankRectangle APRAM((int f, int n));
#endif
/*
 * registry.c
 */
#if !(defined __REGISTRYC) || (defined _ANSI_C)
/* Command line prototyps */
#if REGSTRY
extern int deleteRegistry APRAM((int f, int n));
extern int findRegistry APRAM((int f, int n));
extern int getRegistry APRAM((int f, int n));
extern int listRegistry APRAM((int f, int n));
extern int markRegistry APRAM((int f, int n));
extern int readRegistry APRAM((int f, int n));
extern int saveRegistry APRAM((int f, int n));
extern int setRegistry APRAM((int f, int n));

/* API registry prototypes made available for export */
extern REGHANDLE regFind APRAM((REGHANDLE root, uint8 *subkey));
#define regGetChild(reg) (reg->chld)
#define regGetNext(reg)  (reg->sblg)
extern REGHANDLE regRead APRAM((uint8 *rname, uint8 *fname, int mode));
extern REGHANDLE regSet APRAM((REGHANDLE root, uint8 *subkey, uint8 *value));
extern REGHANDLE vregFind APRAM((REGHANDLE root, uint8 *fmt, ...));
extern int  regDelete APRAM((REGHANDLE root));
extern int  regSave APRAM((REGHANDLE root, uint8 *fname));
extern int  anyChangedRegistry APRAM((void));
#define regGetName(reg)       (reg->name)
#define regGetValue(reg)      (reg->value)
#define regGetValueLen(reg)   (reg->len)
#define regGetLong(reg,def)   ((reg->value==NULL) ? def:meAtoi(reg->value))
#define regGetString(reg,def) ((reg->value==NULL) ? def:reg->value)

#else
#define deleteRegistry notAvailable
#define findRegistry notAvailable
#define getRegistry notAvailable
#define listRegistry notAvailable
#define markRegistry notAvailable
#define readRegistry notAvailable
#define saveRegistry notAvailable
#define setRegistry notAvailable
#endif
#endif
/*
 * search.c
 */
#if !(defined __SEARCHC) || (defined _ANSI_C)		/* SEARCH.C Externals */
#define meEXPAND_BACKSLASH 0x01
#define meEXPAND_FFZERO    0x02
extern  int     expandchar APRAM((int c, uint8 *d, int flags)) ;
extern  int     expandexp APRAM((int slen, uint8 *s, int dlen, int doff,
                                 uint8 *d, int cpos, int *opos, int flags)) ;
extern	int	eq APRAM((int bc, int pc));
extern	int	searchForw APRAM((int f, int n));
extern	int	huntForw APRAM((int f, int n));
extern	int	searchBack APRAM((int f, int n));
extern	int	huntBack APRAM((int f, int n));
extern	int	replaceStr APRAM((int f, int n));
extern	int	queryReplaceStr APRAM((int f, int n));
#endif
/*
 * spawn.c
 */
#if !(defined __SPAWNC) || (defined _ANSI_C)		/* SPAWN.C Externals */
#define LAUNCH_BUFFERNM      0x0001      /* Do not use the comspec    */
#define LAUNCH_SILENT        0x0002      /* Do not use the comspec    */
#define LAUNCH_NOCOMSPEC     0x0004      /* Do not use the comspec    */
#define LAUNCH_DETACHED      0x0008      /* Detached process launch   */
#define LAUNCH_LEAVENAMES    0x0010      /* Leave the names untouched */
#define LAUNCH_SHOWWINDOW    0x0020      /* Dont hide the new window  */
#define LAUNCH_USER_FLAGS    0x002F      /* User flags bitmask        */
#define LAUNCH_SHELL         0x0100
#define LAUNCH_SYSTEM        0x0200
#define LAUNCH_FILTER        0x0400
#define LAUNCH_PIPE          0x0800
#define LAUNCH_IPIPE         0x1000
extern	int	meShell APRAM((int f, int n));
#ifdef _UNIX
extern	int	suspendEmacs APRAM((int f, int n));
#else
#define suspendEmacs voidFunc
#endif
extern	int	doShellCommand APRAM((uint8 *cmdstr)) ;
extern	int	meShellCommand APRAM((int f, int n));
extern	int	spawn APRAM((int f, int n));
extern  int     doPipeCommand APRAM((uint8 *comStr, uint8 *path, uint8 *bufName, 
                                 int silent)) ;
extern	int	pipeCommand APRAM((int f, int n));
#ifdef _IPIPES
extern  int     doIpipeCommand APRAM((uint8 *comStr, uint8 *path, uint8 *bufName, 
                                  int silent)) ;
extern	int	ipipeCommand  APRAM((int f, int n)) ;
extern	int	ipipeWrite APRAM((int f, int n)) ;
extern  void    ipipeRead APRAM((meIPIPE *ipipe)) ;
extern  int     ipipeSetSize APRAM((WINDOW *wp, BUFFER *bp)) ;
extern	void    ipipeRemove APRAM((meIPIPE *ipipe)) ;
#ifdef _UNIX
extern  void    ipipeCheck APRAM((void)) ;
#endif
extern	int	ipipeKill APRAM((int f, int n)) ;
extern  int     anyActiveIpipe APRAM((void)) ;
#else
#define ipipeWriteString voidFunc
#define ipipeCommand  pipeCommand
#define ipipeWrite voidFunc
#define ipipeKill voidFunc
#endif
extern	int	meFilter APRAM((int f, int n));
#ifdef _UNIX
extern  void    __mkTempName APRAM((uint8 *buf, uint8 *name));
#define mkTempName(bb,nn,ee) __mkTempName((bb),(nn))
#else
extern  void    __mkTempName APRAM((uint8 *buf, uint8 *name, uint8 *ext));
#define mkTempName(bb,nn,ee) __mkTempName((bb),(nn),(ee))
#endif
#ifdef _WIN32
extern void mkTempCommName(uint8 *filename, uint8 *basename) ;
#else
#define mkTempCommName(filename,basename) mkTempName(filename,basename,NULL)
#endif
#define COMMAND_FILE      "stdout.~~~"
#define DUMMY_STDIN_FILE  "stdin.~~~" 
#define FILTER_IN_FILE    "fltinp.~~~"
#define FILTER_OUT_FILE   "fltout.~~~"
#endif
/*
 * spell.c
 */
#if !(defined __SPELLC) || (defined _ANSI_C)			/* SPELL.C Externals */
#if SPELL
extern	int	addDict APRAM((int f, int n));
extern	int	addSpellRule APRAM((int f, int n));
extern	int	deleteDict APRAM((int f, int n));
extern	int	saveDict APRAM((int f, int n));
extern	int	spellWord APRAM((int f, int n));
extern  int     anyChangedDictionary APRAM((void)) ;
#else
#define addDict notAvailable
#define addSpellRule notAvailable
#define deleteDict notAvailable
#define saveDict notAvailable
#define spellWord notAvailable
#endif
extern	void	findWordsInit APRAM((uint8 *mask));
extern	uint8  *findWordsNext APRAM((void));
#endif
/*
 * tag.c
 */
#if !(defined __TAGC) || (defined _ANSI_C)			/* TAG.C Externals */
extern	int	findTag APRAM((int f, int n));
#endif
/*
 * termio.c
 */
#if !(defined __TERMIOC) || (defined _ANSI_C)		/* TERMIO.C Externals */
extern	void    TTdoBell APRAM((int)) ;
extern	void    TTbell APRAM((void));
extern  int     charListToShorts APRAM((uint16 *sl, uint8 *cl)) ;
extern  int     keyListToShorts APRAM((uint16 *sl, uint8 *kl)) ;
extern  void    translateKeyAdd APRAM((meTRANSKEY *tcapKeys, int count, int time,
                                       uint16 *key, uint16 map)) ;
extern	int	translateKey APRAM((int f, int n));
extern  char   *meTParm APRAM((char *str, ...)) ;
#endif
/*
 * time.c
 */
#if !(defined __TIMEC) || (defined _ANSI_C)		/* TIME.C Externals */
extern	int	set_timestamp APRAM((BUFFER *bp));
#endif
/*
 * undo.c
 */
#if !(defined __UNDOC) || (defined _ANSI_C)		/* UNDO.C Externals */
#if MEUNDO
extern  UNDOND *meUndoCreateNode     APRAM((size_t size)) ;
extern	void	meUndoAddInsChar     APRAM((void));
extern	void	meUndoAddDelChar     APRAM((void));
extern  void    meUndoAddRepChar     APRAM((void));
extern	void	meUndoAddInsChars    APRAM((int32 numChars));
extern	void	meUndoAddDelChars    APRAM((int32 numChars));
extern	void	meUndoAddReplaceBgn  APRAM((LINE *elinep, uint16 elineo));
extern	void	meUndoAddReplaceEnd  APRAM((int32 numChars));
extern	void    meUndoAddReplace     APRAM((uint8 *dstr, int32 count)) ;
#if NARROW
extern  void    meUndoAddUnnarrow    APRAM((int32 sln, int32 eln, uint16 name)) ;
extern  void    meUndoAddNarrow      APRAM((int32 sln, uint16 name)) ;
#endif
extern	void	meUndoRemove         APRAM((BUFFER *bp)) ;
extern	int	meUndo               APRAM((int f, int n));
#else
#define meUndo notAvailable
#endif
#endif
/*
 * window.c
 */
#if !(defined __WINDOWC) || (defined _ANSI_C)		/* WINDOW.C Externals */
extern  void    makeCurWind APRAM((WINDOW *wp)) ;
extern  void    addModeToWindows APRAM((int mode)) ;
extern  void    addModeToBufferWindows APRAM((BUFFER *bp, int mode)) ;
extern  void    fixWindowTextSize APRAM((WINDOW *wp));
extern  void    fixWindowScrollBars APRAM((WINDOW *wp));
extern  void    fixWindowScrollBox APRAM((WINDOW *wp));
extern  int     menuWindow  APRAM((int n));
extern	int	recenter APRAM((int f, int n));
extern	int	nextWindow APRAM((int f, int n));
extern	int	prevwind APRAM((int f, int n));
extern	int	scrollDown APRAM((int f, int n));
extern  int     scrollLeft APRAM((int f, int n));
extern  int     scrollRight APRAM((int f, int n));
extern	int	scrollUp APRAM((int f, int n));
extern	int	onlywind APRAM((int f, int n));
extern	int	delwind APRAM((int f, int n));
extern	int	growWindHorz APRAM((int f, int n));
extern	int	growWindVert APRAM((int f, int n));
extern	int	resizeWndVert APRAM((int f, int n));
extern	int	resizeWndHorz APRAM((int f, int n));
extern	int	shrinkWindVert APRAM((int f, int n));
extern	int	shrinkWindHorz APRAM((int f, int n));
extern	int	splitWindVert APRAM((int f, int n));
extern	int	splitWindHorz APRAM((int f, int n));
extern  int     resizeAllWnd APRAM((int f, int n));
extern	int	setPosition APRAM((int f, int n));
extern	int	gotoPosition APRAM((int f, int n));
extern  int     setScrollWithMouse APRAM((int f, int n));
/* uses the bfind flags for finding buffer "name"
 * if not null. Also if WPOP_MKCURR set then 
 * makes the buffer current.
 */
#define WPOP_MKCURR 0x100
#define WPOP_USESTR 0x200
#define WPOP_EXIST  0x400
extern	WINDOW *wpopup APRAM((uint8 *name, int flags));
extern	int	popupWindow APRAM((int f, int n));
extern	int	scrollNextUp APRAM((int f, int n));
extern	int	scrollNextDown APRAM((int f, int n));
extern	int	savewnd APRAM((int f, int n));
extern	int	restwnd APRAM((int f, int n));
extern	int	changeScreenDepth APRAM((int f, int n));
extern	int	changeScreenWidth APRAM((int f, int n));
#endif
/*
 * word.c
 */
#if !(defined __WORDC) || (defined _ANSI_C)		/* WORD.C Externals */
extern	int	backWord APRAM((int f, int n));
extern	int	forwWord APRAM((int f, int n));
extern	int	upperWord APRAM((int f, int n));
extern	int	lowerWord APRAM((int f, int n));
extern	int	capWord APRAM((int f, int n));
extern	int	forwDelWord APRAM((int f, int n));
extern	int	backDelWord APRAM((int f, int n));
#if WORDPRO
extern	int	wrapWord APRAM((int f, int n));
extern	int	justify	 APRAM((int leftMargin, int leftDoto));
extern	int	fillPara APRAM((int f, int n));
extern	int	killPara APRAM((int f, int n));
extern	int	countWords APRAM((int f, int n));
#else
#define wrapWord notAvailable
#define fillPara notAvailable
#define killPara notAvailable
#define countWords notAvailable
#endif
#endif

/* gettimeofday - Retreives the time of day to millisecond resolution
 *                This is a 4.2 BSD function. IRIX 5.x and Solaris prior to
 *                2.5 provide a single argument version, for backward
 *                compatibility the 2nd argument is ignored. HP-UX 10.x and
 *                versions of Solaris above 2.5 provide a 2 argument version.
 *                Win32 does not provide this function (of course). 
 *                
 *                Jon Green 18th Dec '96.
 */
/* windows has name clashes with timeval, so define an meTimeval */
#ifdef _WIN32

struct meTimeval
{
    int tv_sec;                         /* Time in seconds */
    int tv_usec;                        /* Time in milliseconds */
};

struct meTimezone 
{
    int tz_minuteswest;                 /* Offset (+/-ve) from UTC in minutes */
    int tz_dsttime;                     /* Flag indicating the type of DST correction */
};

extern void gettimeofday (struct meTimeval *tp, struct meTimezone *tz);

#else
#define meTimeval  timeval
#define meTimezone timezone
#endif

#ifdef _WIN32
/* The current win32 rename() function does not cope with long filenames
 * Use the Win32 definition - note that these functions have an oposite
 * boolean sense that the ANSI 'C' definitions, explicitly test to convert
 * to the more familier invocation.
 *
 * Why the hell we need these functions and the ANSI 'C' functions are not
 * valid is beyond belief - typical bloody Microsoft !!
 */
extern int chdir(const char *name) ;
extern int _getdrive(void) ;
#define meChdir(dir)        chdir(dir)
#define meRename(src,dst)   (MoveFile(src,dst)==FALSE)
#define meUnlink(fn)        (DeleteFile(fn)==FALSE)
/* Doesn't exist if function returns -1 */
#define meTestExist(fn)     (((int) GetFileAttributes(fn)) < 0)
/* Can't read if doesn't exist or its a directory */
#define meTestRead(fn)      (GetFileAttributes(fn) & FILE_ATTRIBUTE_DIRECTORY)
/* Can't write if exists and its readonly or a directory */
#define meTestWrite(fn)     ((((int) GetFileAttributes(fn)) & 0xffff8011) > 0)
/* File is a directory */
#define meTestDir(fn)       ((GetFileAttributes(fn) & (0xf0000000|FILE_ATTRIBUTE_DIRECTORY)) != FILE_ATTRIBUTE_DIRECTORY)
extern int meTestExecutable(uint8 *fileName) ;
#define meStatTestRead(st)  (((st).stmode & FILE_ATTRIBUTE_DIRECTORY) == 0)
#define meStatTestWrite(st) (((st).stmode & (FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_READONLY)) == 0)
#define meStatTestSystem(st) (((st).stmode & FILE_ATTRIBUTE_SYSTEM) == 0)
#define meChmod(fn,mode)    (SetFileAttributes(fn,mode))
extern void WinShutdown (void);
#define meExit(status)      (WinShutdown(), ExitProcess(status))
#endif

#ifdef _DOS
extern int  unlink(const char *file) ;
extern int  meGetFileAttributes(uint8 *fn) ;
#define meFILE_ATTRIB_READONLY  0x01
#define meFILE_ATTRIB_HIDDEN    0x02
#define meFILE_ATTRIB_SYSTEM    0x04
#define meFILE_ATTRIB_VOLLABEL  0x08
#define meFILE_ATTRIB_DIRECTORY 0x10
#define meFILE_ATTRIB_ARCHIVE   0x20
extern void _meChmod(uint8 *fn,uint16 attr) ;
extern int  _meChdir(uint8 *path) ;
/* Doesn't exist if function returns -1 */
#define meTestExist(fn)     (meGetFileAttributes(fn) < 0)
/* Can't read if doesn't exist or its a directory */
#define meTestRead(fn)      (meGetFileAttributes(fn) & 0x10)
/* Can't write if exists and its readonly or a directory */
#define meTestWrite(fn)     ((meGetFileAttributes(fn) & 0xffff8011) > 0)
/* File is a directory */
#define meTestDir(fn)       ((meGetFileAttributes(fn) & 0xf0000010) != 0x010)
extern int meTestExecutable(uint8 *fileName) ;
#define meStatTestRead(st)  (((st).stmode & 0x10) == 0)
#define meStatTestWrite(st) (((st).stmode & 0x11) == 0)
#define meChmod _meChmod
#define meChdir _meChdir
#endif

#ifdef _UNIX
/* Define the standard POSIX tests for the file stats */
#ifndef S_IROTH                         /* Other read permission  */
#define S_IROTH  0000004
#endif
#ifndef S_IRGRP                         /* Group read permission */
#define S_IRGRP  0000040
#endif
#ifndef S_IRUSR                         /* User read permission */
#define S_IRUSR  0000400                
#endif
#ifndef S_IWOTH                         /* Other write permission */
#define S_IWOTH  0000002        
#endif
#ifndef S_IWGRP                         /* Group write permission */
#define S_IWGRP  0000020        
#endif
#ifndef S_IWUSR                         /* User write permission */
#define S_IWUSR  0000200                
#endif
#ifndef S_IXOTH                         /* Other exeute permission */
#define S_IXOTH  0000001                
#endif
#ifndef S_IXGRP                         /* Group execute permission */
#define S_IXGRP  0000010                
#endif
#ifndef S_IXUSR                         /* User execute permission */
#define S_IXUSR  0000100
#endif
#ifndef S_IRWXO                         /* Other read/write/execute */
#define S_IRWXO  ((S_IROTH)|(S_IWOTH)|(S_IXOTH))
#endif 
#ifndef S_IRWXU                         /* User read/write/execute */
#define S_IRWXU  ((S_IRUSR)|(S_IWUSR)|(S_IXUSR))
#endif
#ifndef S_IRWXG                         /* Group read/write/exexute */
#define S_IRWXG  ((S_IRGRP)|(S_IWGRP)|(S_IXGRP))
#endif
#ifndef S_ISGID                         /* Set group id on exec */
#define S_ISGID  0002000               
#endif
#ifndef S_ISUID                         /* Set user id on exec */
#define S_ISUID  0004000  
#endif
#ifndef S_IFLNK                         /* Set if symbolic link -- NOT POSIX */
#define S_IFLNK  0120000
#endif
/* Posix test functions */
#ifndef S_ISDIR                         /* Test directory */
#define S_ISDIR(x)  (((x) & 0170000) == 0040000)
#endif
#ifndef S_ISCHR                         /* Test character special file */
#define S_ISCHR(x)  (((x) & 0170000) == 0020000)
#endif
#ifndef S_ISBLK                         /* Test block special file */
#define S_ISBLK(x)  (((x) & 0170000) == 0060000)
#endif
#ifndef S_ISREG                         /* Test regular file */
#define S_ISREG(x)  (((x) & 0170000) == 0100000)
#endif
#ifndef S_ISFIFO                        /* Test FIFO */
#define S_ISFIFO(x) (((x) & 0170000) == 0010000)
#endif
#ifndef S_ISLNK                         /* Test symbolic link -- NOT POSIX */
#define S_ISLNK(x)  (((x) & 0170000) == (S_IFLNK))
#endif

/* File modes defined in terms of POSIX tests */
extern int meGidInGidList(gid_t gid) ;
#define meStatTestRead(st)                                                   \
((((st).stuid == meUid) && ((st).stmode & S_IRUSR)) ||                       \
 ((st).stmode & S_IROTH) ||                                                  \
 (((st).stmode & S_IRGRP) &&                                                 \
  (((st).stgid == meGid) || (meGidSize && meGidInGidList((st).stgid))))) 
#define meStatTestWrite(st)                                                  \
((((st).stuid == meUid) && ((st).stmode & S_IWUSR)) ||                       \
 ((st).stmode & S_IWOTH) ||                                                  \
 (((st).stmode & S_IWGRP) &&                                                 \
  (((st).stgid == meGid) || (meGidSize && meGidInGidList((st).stgid))))) 
#define meStatTestExec(st)                                                   \
((((st).stuid == meUid) && ((st).stmode & S_IXUSR)) ||                       \
 ((st).stmode & S_IXOTH) ||                                                  \
 (((st).stmode & S_IXGRP) &&                                                 \
  (((st).stgid == meGid) || (meGidSize && meGidInGidList((st).stgid))))) 
#endif

/* Differentiate between different styles for waiting for a process to finish */
#ifdef _BSD
/* Use wait4 to enable us to wait on a PID. */
#define meWaitpid(pid,status,opt) wait4((pid),(status),(opt),NULL)
#define meWAIT_STATUS union wait
/* Use getwd() to get the current working directory */
#define meGetcwd(buf,size)        getwd((char *)(buf))
/* Use the lighter weight vfork() for forking */
#define meFork  vfork
#else
/* SVr4, POSIX.1 */
#define meWaitpid(pid,status,opt) waitpid((pid),(status),(opt))
#define meWAIT_STATUS int
/* POSIX.1 */
#define meGetcwd(buf,size)        getcwd((char *)(buf),(size))
/* Use fork() always */
#define meFork  fork
#endif /* _BSD */

#ifndef meRename
#define meRename(src,dst) rename((char *)(src),(char *)(dst))
#endif
#ifndef meUnlink
#define meUnlink(fn) unlink((char *)(fn))
#endif
#ifndef meChdir
#define meChdir(fn) chdir((char *)(fn))
#endif
#ifndef meTestExist
#define meTestExist(fn) access((char *)(fn),F_OK)
#endif
#ifndef meTestRead
#define meTestRead(fn) access((char *)(fn),R_OK)
#endif
#ifndef meTestWrite
#define meTestWrite(fn) access((char *)(fn),W_OK)
#endif
#ifndef meTestExec
#define meTestExec(fn) access((char *)(fn),X_OK)
#endif
#ifndef meTestDir
#define meTestDir(fn) (getFileStats(fn,0,NULL,NULL) != meFILETYPE_DIRECTORY)
#endif
#ifndef meChmod
#define meChmod(fn,attr) chmod((char *)(fn),attr)
#endif
#ifndef meExit
extern void exit(int i) ;
#ifdef _CLIENTSERVER
    /* Close & delete the client file */
#define meExit(n) (TTkillClientServer(),exit(n))
#else
#define meExit exit
#endif
#endif

/*
 * Terminal I/O file defintions
 */
extern	char   *getenv APRAM((const char *));

extern  void    free APRAM((void *block));
extern  void   *malloc APRAM((size_t s));
extern  void   *realloc APRAM((void *block, size_t s));
extern	void   *memcpy APRAM((void *, const void *, size_t));
extern	void   *memset APRAM((void *, int,    size_t));

extern	size_t	strlen APRAM((const char *s));
extern	int	strcmp APRAM((const char *s1, const char *s2));
extern	char   *strcpy APRAM((char *s1, const char *s2));
extern	char   *strcat APRAM((char *s1, const char *s2));
extern	char   *strdup APRAM((const char *s1));
extern	char   *strncpy APRAM((char *s1, const char *s2, size_t n));
extern	int	strncmp APRAM((const char *s1, const char *s2, size_t n));
extern	char   *strchr APRAM((const char *, int));
extern	char   *strrchr APRAM((const char *, int));
extern  long    strtol APRAM((const char*, char**, int));
extern  int     abs APRAM((int));
extern  int     system APRAM((const char*));

#ifdef _NOPUTENV
#define meGetenv(s1)        ((void *) megetenv((const char *)(s1)))
extern char    *megetenv APRAM((const char *s));
extern int      putenv APRAM((const char *s));
#else
#define meGetenv(s1)        ((void *) getenv((const char *)(s1)))
#endif
#define mePutenv(s1)        (putenv((const char *)(s1)))
#define meStrlen(s1)        strlen((const char *)(s1))
#define meStrcmp(s1,s2)     strcmp((const char *)(s1),(const char *)(s2))
#define meStrncmp(s1,s2,n)  strncmp((const char *)(s1),(const char *)(s2),(n))
#define meStrcat(s1,s2)     strcat((char *)(s1),(const char *)(s2))
#define meStrcpy(s1,s2)     strcpy((char *)(s1),(const char *)(s2))
#define meStrncpy(s1,s2,n)  strncpy((char *)(s1),(const char *)(s2),(n))
#define meStridif(s1,s2)    stridif((const char *)(s1),(const char *)(s2))
#define meStricmp(s1,s2)    stricmp((const char *)(s1),(const char *)(s2))
#define meStrnicmp(s1,s2,n) strnicmp((const char *)(s1),(const char *)(s2),(n))
#define meStrdup(s1)        ((void *) strdup((const char *)(s1)))
#define meStrstr(s1,s2)     ((void *) strstr((const char *)(s1),(const char *)(s2)))
#define meStrchr(s1,c1)     ((void *) strchr((const char *)(s1),(c1)))
#define meStrrchr(s1,c1)    ((void *) strrchr((const char *)(s1),(c1)))

#define CHRMSK_MACROTYPE   0x0f
#define CHRMSK_DISPLAYABLE 0x10
#define CHRMSK_POKABLE     0x20
#define CHRMSK_PRINTABLE   0x40
#define CHRMSK_SPACE       0x80

#define CHRMSK_LOWER       0x01
#define CHRMSK_UPPER       0x02
#define CHRMSK_ALPHA       0x03
#define CHRMSK_HEXDIGIT    0x04
#define CHRMSK_ALPHANUM    0x07
#define CHRMSK_SPLLEXT     0x08
#define CHRMSK_USER1       0x10
#define CHRMSK_USER2       0x20
#define CHRMSK_USER3       0x40
#define CHRMSK_USER4       0x80

#define CHRMSK_DEFWORDMSK  CHRMSK_ALPHANUM

#define getMacroType(c)  (charMaskTbl1[((uint8) (c))] & CHRMSK_MACROTYPE)
#define getMacroTypeS(s) (charMaskTbl1[((uint8) (*s))] & CHRMSK_MACROTYPE)
#define isDisplayable(c) (charMaskTbl1[((uint8) (c))] & CHRMSK_DISPLAYABLE)
#define isPokable(c)     (charMaskTbl1[((uint8) (c))] & CHRMSK_POKABLE)
#define isPrint(c)       (charMaskTbl1[((uint8) (c))] & CHRMSK_PRINTABLE)
#define isSpace(c)       (charMaskTbl1[((uint8) (c))] & CHRMSK_SPACE)

#define isDigit(c)       (((c) >= '0') && ((c) <= '9'))
#define isLower(c)       (charMaskTbl2[((uint8) (c))] & CHRMSK_LOWER)
#define isUpper(c)       (charMaskTbl2[((uint8) (c))] & CHRMSK_UPPER)
#define isAlpha(c)       (charMaskTbl2[((uint8) (c))] & CHRMSK_ALPHA)
#define isXDigit(c)      (charMaskTbl2[((uint8) (c))] & CHRMSK_HEXDIGIT)
#define isAlphaNum(c)    (charMaskTbl2[((uint8) (c))] & CHRMSK_ALPHANUM)
#define isSpllExt(c)     (charMaskTbl2[((uint8) (c))] & CHRMSK_SPLLEXT)
#define isWord(c)        (charMaskTbl2[((uint8) (c))] & isWordMask)
#define isSpllWord(c)    (charMaskTbl2[((uint8) (c))] & (CHRMSK_ALPHANUM|CHRMSK_SPLLEXT))
#define inWord()         (isWord(lgetc(curwp->w_dotp, curwp->w_doto)))
#define inPWord()        ((lgetc(curwp->w_dotp, curwp->w_doto)) > ' ')

#define toLower(c)       (isUpper(c) ? (charCaseTbl[((uint8) (c))]):c)
#define toUpper(c)       (isLower(c) ? (charCaseTbl[((uint8) (c))]):c)
#define toggleCase(c)    (charCaseTbl[((uint8) (c))])

#define toUserFont(c)    (charLatinUserTbl[((uint8) c)])
#define toLatinFont(c)   (charUserLatinTbl[((uint8) c)])

#define	hexToNum(c)      ((c <= '9') ? (c^0x30)   : \
                          (c >= 'a') ? (c-'a'+10) : \
                                       (c-'A'+10))

/*	Macro argument token types					*/

#define TKNUL	0x00			/* end-of-string		*/
#define TKARG	0x01			/* interactive argument 	*/
#define TKREG	0x02			/* register variable		*/
#define TKVAR	0x03			/* user variables		*/
#define TKENV	0x04			/* environment variables	*/
#define TKFUN	0x05			/* function.... 		*/
#define TKDIR	0x06			/* directive			*/
#define TKLBL	0x07			/* line label			*/
#define TKLIT	0x08			/* numeric literal		*/
#define TKSTR	0x09			/* quoted string literal	*/
#define TKCMD	0x0A			/* command name 		*/
#define TKLVR	0x0B			/* Local variable 		*/
#define TKCVR	0x0C			/* Command variable 		*/

#define meAtoi(s) strtol((char *)(s),(char **)NULL,0)
#define meAtol(s) (meAtoi(s) != 0)
#define meLtoa(s) ((s) ? truem:falsem)

/* is url file? test */
#define isHttpLink(fn) (!strncmp((char *)fn,"http://",7))
#define isFtpLink(fn)  (!strncmp((char *)fn,"ftp://",6))
#define isUrlLink(fn)  (isHttpLink(fn) || isFtpLink(fn))
#define isUrlFile(fn)  (!strncmp((char *)fn,"file:",5))


/* use this with some care */
#define meFree(x) free(x)
#define meNullFree(p) (((p)!=NULL)?(free(p),1):0)

#define mwResetCursor() TTmove(mwRow,mwCol)
#define mlResetCursor() ((mlStatus&MLSTATUS_POSOSD) ? TTmove(osdRow,osdCol):TTmove(TTnrow,mlCol))
#define resetCursor()   ((mlStatus&(MLSTATUS_POSOSD|MLSTATUS_POSML)) ? mlResetCursor():mwResetCursor())

#define get_tab_pos(c)      ((tabwidth-1) - (c)%tabwidth)       /* Get the number of characters to the next TAB position. */
#define next_tab_pos(c)     (tabwidth - ((c)%tabwidth))         /* */
#define	at_tab_pos(c)       ((c)%tabwidth)                      /* Get the number of characters to the next TAB position. */
#define get_no_tabs_pos(c)  ((c)/tabwidth)                      /* Get the number of TAB characters to the current position. */

#define restoreWindWSet(wp,ws)                                               \
((wp)->topLineNo=(ws)->topLineNo,(wp)->w_dotp=(ws)->w_dotp,                  \
 (wp)->w_doto=(ws)->w_doto,(wp)->w_markp=(ws)->w_markp,                      \
 (wp)->w_marko=(ws)->w_marko,(wp)->line_no=(ws)->line_no,                    \
 (wp)->mlineno=(ws)->mlineno)
#define restoreWindBSet(wp,bp)                                               \
((wp)->topLineNo=(bp)->topLineNo,(wp)->w_dotp=(bp)->b_dotp,                  \
 (wp)->w_doto=(bp)->b_doto,(wp)->w_markp=(bp)->b_markp,                      \
 (wp)->w_marko=(bp)->b_marko,(wp)->line_no=(bp)->line_no,                    \
 (wp)->mlineno=(bp)->mlineno)
#define storeWindBSet(bp,wp)                                                 \
((bp)->topLineNo=(wp)->topLineNo,(bp)->b_dotp=(wp)->w_dotp,                  \
 (bp)->b_doto=(wp)->w_doto,(bp)->b_markp=(wp)->w_markp,                      \
 (bp)->b_marko=(wp)->w_marko,(bp)->line_no=(wp)->line_no,                    \
 (bp)->mlineno=(wp)->mlineno)

#define setShowRegion(sbp,sln,slo,eln,elo)                                   \
(selhilight.bp=sbp,selhilight.mlineno=sln,selhilight.mlineoff=slo,           \
 selhilight.dlineno=eln,selhilight.dlineoff=elo,                             \
 selhilight.flags = SELHIL_ACTIVE|SELHIL_FIXED|SELHIL_CHANGED)

