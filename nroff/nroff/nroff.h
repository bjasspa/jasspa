/****************************************************************************
 *
 *			Copyright 1995 Jon Green.
 *			   All Rights Reserved
 *
 *
 *  System        : nroff
 *  Module        : Main library
 *  Object Name   : $RCSfile: nroff.h,v $
 *  Revision      : $Revision: 1.2 $
 *  Date          : $Date: 2002-03-10 18:35:57 $
 *  Author        : $Author: jon $
 *  Last Modified : <020310.1827>
 *
 *  Description
 *
 *  Notes
 *
 *  History
 *
 *  $Log: not supported by cvs2svn $
 *  Revision 1.1  2000/10/21 14:31:30  jon
 *  Import
 *
 *
 ****************************************************************************
 *
 *  Copyright (c) 1995 Jon Green.
 *
 *  All Rights Reserved.
 *
 *  This Document may not, in whole or in part, be copied,
 *  photocopied, reproduced, translated, or reduced to any
 *  electronic medium or machine readable form without prior
 *  written consent from Jon Green.
 *
 ****************************************************************************/

#ifndef __NROFF_H                       /* Guard file */
#define __NROFF_H 1

#ifdef _UNIX
#define eolStr "\n"
#define eofStr ""
#else
#define eolStr "\r\n"
#define eofStr "\032"                   /* End of file is ^Z 0r 0x1a */
#endif

/* Definitions for internal type representation */

#define USPACE_CHAR     ((char) 1)
#define ITALIC_CHAR     ((char) 2)
#define BOLD_CHAR       ((char) 3)
#define ROMAN_CHAR      ((char) 4)
#define MONO_CHAR       ((char) 5)
#define PREV_CHAR       ((char) 6)
#define ESC_CHAR        ((char) 7)
#define COPYRIGHT_CHAR  ((char) 16)
#define BULLET_CHAR     ((char) 17)
#define MSPACE_CHAR     ((char) 18)
#define DEGREE_CHAR     ((char) 19)
#define CONTINUE_CHAR   ((char) 20)
#define ZSPACE_CHAR     ((char) 21)


#define FT_P   0
#define FT_R   1
#define FT_I   2
#define FT_B   3
#define FT_S   4
#define FT_C   5

typedef struct snrFILE {                /* File block */
    char            fileName [FILENAME_MAX];
    int             lineNo;
    FILE            *fp;
    struct snrFILE  *prev;
    int             superman;
} nrFILE;

/*
 * Nroff function table.
 */

typedef void (* v_cccccci)(char *c1, char *c2, char *c3, char *c4, char *c5, char *c6, int i);
typedef void (* v_ccccci)(char *c1, char *c2, char *c3, char *c4, char *c5, int i);
typedef void (* v_ccccc)(char *c1, char *c2, char *c3, char *c4, char *c5);
typedef void (* v_cccc)(char *c1, char *c2, char *c3, char *c4);
typedef void (* v_ccc)(char *c1, char *c2, char *c3);
typedef void (* v_cc)(char *c1, char *c2);
typedef void (* v_c)(char *c1);

typedef void (* v_ci)(char *c1, int i2);
typedef void (* v_cip)(char *c1, int *i2);
typedef void (* v_cii)(char *c1, int i2, int i3);

typedef void (* v_i)(int i1);
typedef void (* v_ic)(int i1, char *c2);
typedef void (* v_iic)(int i1, int i2, char *c3);

typedef void (* v_v)(void);
typedef void (* v_cp)(char **c1);

typedef struct  {
#define EXT_KW(n,i,f,t,m) t f;
#define INT_KW(n,i,f,m)   void *f;
#include "nroff.def"
#undef EXT_KW
#undef INT_KW
} nrFUNCTION;

#define nrInstall(f,x,y) f.x = y
extern void nrInstallFunctionTable (nrFUNCTION *table);

/* Nroff file handling routines */

extern nrFILE *nrfp;
extern nrFILE *nrFilePush (char *name);
extern nrFILE *__nrfPush (char *file, FILE *fp);
extern nrFILE *nrFilePop (void);

/* Header definitions */

#define NROFF_MODE_DEFAULT   0x00       /* Default mode */
#define NROFF_MODE_COMPILE   0x01       /* Compiling */
#define NROFF_MODE_SYMBOL    0x02       /* Collecting symbols */

extern int  nroff (nrFILE *fp, int mode);
extern char *nroffVersion;
extern char *nroffId;                   /* Nroff converter identity */

/* Line trimming */

extern char *leftTrim (char *s);
extern char *rightTrim (char *s);
extern int inStr (int start, char *s, char search);

/* Sting length processing */

extern int wordLen (char *s);
extern int trueStrLen (char *s);
extern void strOverCopy (char *s, int index, int offset);

/* Identity marker comparison routines */

extern char *nrImGetFirst (char *comp);
extern char **nrImGetAll (char *comp);
extern int nrImCmp (char *item, char *ritem);
extern int nrImCmpAll (char *item, char **ritem);
extern int nrImCmpAllC (char *item, char *ritem);

/* dtags.c - Tag definition */

typedef struct tagEntry {
    struct tagEntry *prev;
    struct tagEntry *next;
    int             line;
    char            *desc;
    char            *name;
    char            *file;
    char            *section;
    void            *data;
} Tag, *TagP;

extern TagP tagAlloc (char *name, char *section, char *desc, char *file, int line, void *data);
extern TagP tagDup (TagP tp);
extern void tagFree (TagP tp);
extern TagP add_tag (TagP *tagHead, TagP np);
extern TagP sub_tag (TagP *tagHead, TagP np);
extern TagP find_tag (TagP *tagHead, char *name, char *section, int unlink);
extern int  diff_tag (TagP s1, TagP s2);
extern void free_tags (TagP *tagHead);
extern TagP tagIterateInit (TagP *tagHead);
extern TagP tagIterate (TagP tp);
extern int  tagCount (TagP tagHead);

/* catman.c - Catman definitions */

typedef struct {
    long size;                          /* Size of compressed file */
    long offset;                        /* Offset into compressed file */
    char name [64];                     /* null terminated manpage name */
    char file [16];                     /* name of manual page filename */
} CMan;

typedef struct cmfilelist {
    struct cmfilelist *next;            /* Next file in list */
    char              file [14];        /* Name of the file. */
    long              size;             /* Size of compressed file in bytes */
    long              offset;           /* Offset into dbz file */
} CFile;

typedef struct cmlist {
    struct cmlist *next;                /* Next item in list */
    CFile         *fdesc;               /* File descriptor */
    CMan          cman;                 /* CatMan data */
} CManList;

extern void catmanInitialise (char *name, int mode);
extern void catmanTerminate (void);

extern FILE *catmanOpenFile (char *name);
extern void catmanCloseFile (FILE *fp);

extern void catmanEntry (char *entryName, char *section);

extern char *findCatman (char *file, char *name, char *section);
extern int  dbzConcatinate (char *filename);

/* nlibrary.c - Library cross referencing routines. */

#define LI_STATUS   0                   /* Index of status byte */
#define LI_FILE     1                   /* Index of file name byte */
#define LI_NAME     2                   /* Indec if name word */

#define LS_EXTERN   0x01                /* External definition */
#define LS_REF      0x02                /* Internal reference */
#define LS_DEF      0x03                /* Internal definition */
#define LS_BROWSE   0x08                /* Browse item */
#define LS_PACKAGE  0x10                /* Package item */
#define LS_COMPILE  0x20                /* Compiling item */
#define LS_JUMP     0x40                /* Jump item */

#define nrLibItemName(x)        (&(x)->name [LI_NAME])
#define nrLibItemBrowse(x)      ((x)->browse)
#define nrLibItemFile(x)        (&(x)->name [(int)((x)->name[LI_FILE])])
#define nrLibItemNext(x)        ((x)->next)
#define nrLibItemStatus(x)      ((x)->name[LI_STATUS])
#define nrLibItemData(x)        ((x)->data)

#define nrLibModuleHead()       (nrLibModule)
#define nrLibModuleAlias(x)     ((x)->alias)
#define nrLibModuleItem(x)      ((x)->item)
#define nrLibModulePackage(x)   ((x)->package)
#define nrLibModuleNext(x)      ((x)->next)
#define nrLibModuleStatus(x)    ((x)->name[LI_STATUS])
#define nrLibModuleName(x)      (&(x)->name [1])
#define nrLibModuleData(x)      ((x)->data)

struct st_LibItem;

typedef struct st_LibModule
{
    struct st_LibModule *next;          /* Next module */
    struct st_LibItem *item;            /* First item in module */
    struct st_LibItem *package;         /* First package in module */
    char *alias;                        /* Alias name for the module */
    void *data;                         /* User data node */
    char name [1];                      /* Name of the module */
} LibModule;

typedef struct st_LibItem
{
    struct st_LibItem *next;
    struct st_LibModule *module;
    int    browse;
    void *data;                         /* User data node */
    char name [4];
} LibItem;

typedef struct st_LibBrowse
{
    char *prevName;
    int  prevBrowseNo;
    char *nextName;
    int  nextBrowseNo;
} LibBrowse;

extern LibModule *nrLibModule;
extern LibModule *undefModule;

extern int nrLibNameFormat (int newmode);
extern LibModule *nrLibModuleConstruct (char *name, int ref);
extern LibModule *nrLibModuleDestruct (LibModule *p);
extern LibModule *nrLibModuleFind (LibModule *p, char *name);
extern LibModule *nrLibModuleAdd (LibModule **lp, char *name, int ref,
                                  int *status);
extern int nrLibModuleSetAlias (char *module, char *alias);

extern LibItem *nrLibItemConstruct (char *item, char *file, int ref);
extern LibItem *nrLibItemDestuct (LibItem *p);
extern LibItem *nrLibItemFind (LibItem *p, char *name, char **file);
extern LibItem *nrLibItemAdd (LibItem **lp, char *name, char *file,
                              int ref, int *rstatus);
extern int libFindAddReference (char *module, char *name, char *file,
                                char **rmodule, char **rfile, void **h,
                                int ref);
extern char *nrMakeXref (char *name, char *section);
extern int nrResolveExternalReference (int compiling, char *name, char *section, char **pmodule, char **pfile);

extern int libResolveBrowseSequence (char *module);
extern LibBrowse *libFindBrowseSequence (char *module, char *name, char *section);

/* Library loading routines */

extern int nrLibSetModule (char *name);
extern char *nrLibGetModule (void);
extern int nrLibSetPath (char *path);
extern int nrLibLoad (char *name);

/* Title routines */

extern int nrTitleSet (char *s);
extern void *nrTitleGet (void *handle, char **title, char **xref,
                         char **rmodule, char **rfile);
/* Home routines */

extern void nrHomeSet (char *s);
extern char *nrHomeGet (void);

/* Level routines */

extern int nrGetLevel (char *name, char *id);

#endif
