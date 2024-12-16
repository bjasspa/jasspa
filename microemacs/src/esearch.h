/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * esearch.h - Searching definitions.
 *
 * Copyright (C) 1988-2024 JASSPA (www.jasspa.com)
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
 * Synopsis:    Searching definitions.
 * Authors:     Unknown, Jon Green & Steven Phillips
 * Description:
 *     Defines, typdefs, and global variables that are of use for the
 *     routines in search.c and isearch.c.
 */

/*
 * PTBEG, PTEND, meFORWARD, and meREVERSE are all toggle-able values for
 * the scan routines.
 */
#define	PTBEG	0	/* Leave the point at the beginning on search.*/
#define	PTEND	1	/* Leave the point at the end on search.*/

#if MEOPT_MAGIC

#define meREGEXCLASS_SIZE 32
typedef unsigned char meRegexClass[meREGEXCLASS_SIZE];
typedef unsigned char meRegexDByte[2];

typedef struct meRegexItem {
    /* linked list of all malloced items */
    struct meRegexItem *lnext;    /* next item in the global list */
    /* current regex item linkage */
    struct meRegexItem *next;     /* next item in the regex */
    struct meRegexItem *child;    /* Groups sub-expression list */
    struct meRegexItem *alt;      /* Group alternative sub expression */
    unsigned short matchMin;      /* minimum number of matches */
    unsigned short matchMax;      /* maximum number of matches */
    union {
        unsigned char cc;         /* character data */
        meRegexDByte dd;          /* double character data */
        int group;                /* group number and state */
        meRegexClass cclass;      /* Class bit mask */
    } data;
    unsigned char flag;           /* item type */
    unsigned char type;           /* item type */
} meRegexItem;     

#define meREGEXITEM_FLAG_HASQ  0x01
#define meREGEXITEM_FLAG_LAZY  0x02
#define meREGEXITEM_MATCH_MAX  0xffff

typedef struct {
    int start;
    int end;
    int regexStart;
} meRegexGroup;

typedef struct meRegex {
    /* Public elements */
    unsigned char *regex;         /* the regex string */
    int regexLen;                 /* the length of the regex string */
    int groupNo;                  /* the number of groups in regex */
    int newlNo;                   /* the number of \n char found */
    int flags;                    /* compile + okay flag - set to 0 to force recompile */
    meRegexGroup *group;          /* the group start and end offsets */
    
    /* Private elements */
    int regexSz;                  /* malloced size of regex string */
    int groupSz;                  /* malloced size of group array */
    meRegexItem  *lhead;          /* link list of all malloced items */
    meRegexItem  *lnext;          /* the next free item */
    meRegexItem  *head;           /* pointer to the regex first item */
    meRegexClass  start;          /* first character class bit mask */
} meRegex;

/* meRegexComp return values */
#define meREGEX_OKAY               0
/* First set of errors can be cause by the regex being incomplete - 
 * useful for isearch - ME uses UNKNOWN for gnu regex */
#define meREGEX_ERROR_UNKNOWN      1
#define meREGEX_ERROR_TRAIL_BKSSH  2
#define meREGEX_ERROR_OCLASS       3
#define meREGEX_ERROR_OGROUP       4
#define meREGEX_ERROR_OINTERVAL    5
/* the next set are regex errors which aren't caused by being incomplete */
#define meREGEX_ERROR_CLASS        6
#define meREGEX_ERROR_UNSUPCLASS   7
#define meREGEX_ERROR_BKSSH_CHAR   8
#define meREGEX_ERROR_CGROUP       9
#define meREGEX_ERROR_INTERVAL     10
#define meREGEX_ERROR_BACK_REF     11
#define meREGEX_ERROR_NESTGROUP    12
#define meREGEX_ERROR_BADQUAL      13
/* the next set are internal errors */
#define meREGEX_ERROR_MALLOC       14

/* meRegexComp internal return value - never actually returned */
#define meREGEX_ALT               -1

/* meRegexComp flags - only ICASE should be passed by the user */
#define meREGEX_ICASE           0x01
#define meREGEX_VALID           0x02
#define meREGEX_MAYBEEMPTY      0x04
#define meREGEX_BEGLINE         0x08
#define meREGEX_MAYENDBUFF      0x10
#define meREGEX_CLASSCHANGE     0x20
#define meREGEX_CLASSUSED       0x40

/* meRegexMatch flags */
#define meREGEX_BACKWARD        0x01
#define meREGEX_BEGBUFF         0x02
#define meREGEX_ENDBUFF         0x04
#define meREGEX_MATCHWHOLE      0x08

int
meRegexComp(meRegex *regex, unsigned char *regStr, int flags);
int
meRegexMatch(meRegex *regex, unsigned char *string, int len, 
             int offsetS, int offsetE, int flags);

extern char    *meRegexCompErrors[];
extern meRegex  mereRegex;          /* meRegex used by main search */
extern meRegex  meRegexStrCmp;      /* meRegex used by meRegexStrCmp if main is not used  */
extern int      mereNewlBufSz;
extern meUByte *mereNewlBuf;

#define meSEARCH_LAST_MAGIC_MAIN   0x01
#define meSEARCH_LAST_MAGIC_STRCMP 0x02

extern int      srchLastMagic;      /* last search was a magic        */

#define mereRegexInvalidate()   (mereRegex.flags = 0)
#define mereRegexClassChanged() (mereRegex.flags |= meREGEX_CLASSCHANGE)
#define mereRegexGroupRegexStart(n) (mereRegex.group[(n)].regexStart)
#define mereRegexGroupStart(n)  (mereRegex.group[(n)].start)
#define mereRegexGroupEnd(n)    (mereRegex.group[(n)].end)
#define mereRegexGroupNo()      (mereRegex.groupNo)

/* meRegexClass functions and macros */
#define meRegexClassTest(clss,cc)    (clss[(cc)>>3] &   (1<<((cc)&0x7)))
#define meRegexClassSet(clss,cc)     (clss[(cc)>>3] |=  (1<<((cc)&0x7)))
#define meRegexClassClear(clss,cc)   (clss[(cc)>>3] &= ~(1<<((cc)&0x7)))
#define meRegexClassToggle(clss,cc)  (clss[(cc)>>3] ^=  (1<<((cc)&0x7)))
#define meRegexClassSetAll(clss)     (memset(clss,0xff,meREGEXCLASS_SIZE))
#define meRegexClassClearAll(clss)   (memset(clss,0x00,meREGEXCLASS_SIZE))
#define meRegexClassCpy(cls1,cls2)   (memcpy(cls1,cls2,meREGEXCLASS_SIZE))
#define meRegexClassCmp(cls1,cls2)   memcmp(cls1,cls2,meREGEXCLASS_SIZE)

#endif

typedef struct {                       
    meLine *startline;                  /* Pattern start position */
    int   startoff;                     /* Pattern start line offset */
    int   startline_no;                 /* Pattern start line number */
    meLine *endline;                    /* Pattern end position */
    int   endoffset;                    /* Pattern end offset */
    int   endline_no;                   /* Pattern end line number */
} SCANNERPOS;

/* iscanner - external front end to buffer searching
 * returns meTRUE (1)  if the search was successful
 *         meFALSE (0) if the search failed
 *         -1        if the regex apat failed to compile (error)
 *         -2        if the regex apat failed to compile (incomplete?)
 */
extern int iscanner (meUByte *apat, int n, int flags, SCANNERPOS *sp);

/* search arrays and variables, defined in search.c, but exported
 * here so others can access them.
 */ 
extern meUByte  srchPat[];              /* current search string          */
extern meUByte  srchRPat[];             /* reverse current search string  */
/* the following variables store info on the last match - for @s# support     */
extern int      srchLastState;          /* status of last search          */
extern meUByte *srchLastMatch;          /* pointer to the last match string */

/* Incremental search defines */
#define ISCANNER_MAGIC   0x01           /* Require magical scanner */
#define ISCANNER_PTBEG   0x02           /* Point at start of string */
#define ISCANNER_QUIET   0x04           /* Turn off pattern building errors */
#define ISCANNER_EXACT   0x08           /* Exact pattern matching */
#define ISCANNER_BACKW   0x10           /* Search backwards */
#define ISCANNER_EOBQUIT 0x20           /* Quit if the end or beginning of buffer is reached */
#define ISCANNER_SCANMR  0x40           /* Isearch flag to scan more */
#if MEOPT_IPIPES
#define ISCANNER_PIPE    0x80           /* Isearch performed in ipipe buffer */
#endif

#if MEOPT_ISEARCH
#define	HISTBUFSIZ	256	        /* Length of our command buffer */
#endif
