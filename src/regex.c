/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * regex.c - regex compiler and matcher.
 *
 * Copyright (C) 1999-2001 Steven Phillips
 * Copyright (C) 2002-2009 JASSPA (www.jasspa.com)
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
 * Created:     Wed Aug 11 1999
 * Synopsis:    regex compiler and matcher.
 * Authors:     Steven Phillips
 * Notes:
 *      If using elsewhere extract the code within the 2 "Header for regex"
 *      comments and place into a regex.h header file.
 * 
 *  To compile a small test harness run
 *      gcc -D_TEST_RIG regex.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _NOT_JASSPA_ME
/* Header for regex - start *************************************************/

#define meREGEXCLASS_SIZE 32
typedef unsigned char meRegexClass[meREGEXCLASS_SIZE] ;

typedef struct meRegexItem {
    /* linked list of all malloced items */
    struct meRegexItem *lnext ;   /* next item in the global list */
    /* current regex item linkage */
    struct meRegexItem *next ;    /* next item in the regex */
    struct meRegexItem *child ;   /* Groups sub-expression list */
    struct meRegexItem *alt ;     /* Group alternative sub expression */
    int matchMin ;                /* minimum number of matches */
    int matchMax ;                /* maximum number of matches */
    union {
        unsigned char cc ;        /* character data */
        unsigned char dd[2] ;     /* double character data */
        int group ;               /* group number */
        meRegexClass cclass ;     /* Class bit mask */
    } data ;
    unsigned char type ;          /* item type */
} meRegexItem ;    

typedef struct {
    int start ;
    int end ;
} meRegexGroup ;

typedef struct meRegex {
    /* Public elements */
    unsigned char *regex ;        /* the regex string */
    int groupNo ;                 /* the number of groups in regex */
    int newlNo ;                  /* the number of \n char found */
    int flags ;                   /* compile + okay flag - set to 0 to force recompile */
    meRegexGroup *group ;         /* the group start and end offsets */
    
    /* Private elements */
    int regexSz ;                 /* malloced size of regex string */
    int groupSz ;                 /* malloced size of group array */
    meRegexItem  *lhead ;         /* link list of all malloced items */
    meRegexItem  *lnext ;         /* the next free item */
    meRegexItem  *head ;          /* pointer to the regex first item */
    meRegexClass  start ;         /* first character cclass bit mask */
} meRegex ;

/* meRegexComp return values */
#define meREGEX_OKAY               0
/* First set of errors can be cause by the regex being incomplete - 
 * useful for isearch - ME uses UNKNOWN for gnu regex */
#define meREGEX_ERROR_UNKNOWN      1
#define meREGEX_ERROR_TRAIL_BKSSH  2
#define meREGEX_ERROR_OCLASS       3
#define meREGEX_ERROR_OGROUP       4
/* the next set are regex errors which aren't caused by being incomplete */
#define meREGEX_ERROR_CLASS        5
#define meREGEX_ERROR_UNSUPCLASS   6
#define meREGEX_ERROR_BKSSH_CHAR   7
#define meREGEX_ERROR_CGROUP       8
#define meREGEX_ERROR_INTERVAL     9
#define meREGEX_ERROR_BACK_REF     10
#define meREGEX_ERROR_NESTGROUP    11
/* the next set are internal errors */
#define meREGEX_ERROR_MALLOC       12

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

#define meRegexInvalidate(regex)   (regex->flags = 0)
#define meRegexClassChanged(regex) (regex->flags |= meREGEX_CLASSCHANGE)

/* meRegexMatch flags */
#define meREGEX_BACKWARD        0x01
#define meREGEX_BEGBUFF         0x02
#define meREGEX_ENDBUFF         0x04
#define meREGEX_MATCHWHOLE      0x08

int
meRegexComp(meRegex *regex, unsigned char *regStr, int flags) ;
int
meRegexMatch(meRegex *regex, unsigned char *string, int len, 
             int offsetS, int offsetE, int flags) ;
void
meRegexFree(meRegex *regex) ;

/* Header for regex - end ***************************************************/


#define isDigit(c)       (isdigit(c))
#define isLower(c)       (islower(c))
#define isUpper(c)       (isupper(c))
#define isAlpha(c)       (isalpha(c))
#define isAlphaNum(c)    (isalnum(c))
#define isWord(c)        (isalnum(c) || ((c) == '_'))
#define isXDigit(c)      (isxdigit(c))
#define isSpace(c)       (isspace(c))
#define isPrint(c)       (isprint(c))
#define isCntrl(c)       (iscntrl(c))
#define isPunct(c)       (ispunct(c))
#ifdef isgraph
#define isGraph(c)       (isgraph(c))
#else
#define isGraph(c)       (isPrint(c) && ((c) != ' '))
#endif

#define toLower(c)       (isUpper(c) ? tolower(c):(c))
#define toUpper(c)       (isLower(c) ? toupper(c):(c))
#define toggleCase(c)    (isUpper(c) ? tolower(c):(isLower(c) ? toupper(c):(c)))
#define hexToNum(c)      ((c <= '9') ? (c^0x30)   : \
                          (c >= 'a') ? (c-'a'+10) : \
                                       (c-'A'+10))
#define MEOPT_MAGIC 1

#else

#include "emain.h"
#include "esearch.h"

#define isCntrl(c)       (iscntrl(c))
#define isPunct(c)       (ispunct(c))
#define isGraph(c)       (isPrint(c) && ((c) != ' '))

#endif

#if MEOPT_MAGIC

/* NOTE - order of the items in meREGEXITEM is important,
 * meREGEXITEM_BEGBUFF must be the first non-closure appending item
 * (i.e. \`* == \`\*) in the list with all items which can have
 * closures before it
 */
enum {
    meREGEXITEM_NOOP=0,
    meREGEXITEM_GROUP,
    meREGEXITEM_BACKREF,
    meREGEXITEM_SINGLE,
    meREGEXITEM_DOUBLE,
    meREGEXITEM_CLASS,
    meREGEXITEM_ANYCHR,
    meREGEXITEM_ALPHACHR,
    meREGEXITEM_NALPHACHR,
    meREGEXITEM_DIGITCHR,
    meREGEXITEM_NDIGITCHR,
    meREGEXITEM_XDIGITCHR,
    meREGEXITEM_NXDIGITCHR,
    meREGEXITEM_LOWERCHR,
    meREGEXITEM_NLOWERCHR,
    meREGEXITEM_ALNUMCHR,
    meREGEXITEM_NALNUMCHR,
    meREGEXITEM_SPACECHR,
    meREGEXITEM_NSPACECHR,
    meREGEXITEM_UPPERCHR,
    meREGEXITEM_NUPPERCHR,
    meREGEXITEM_WORDCHR,
    meREGEXITEM_NWORDCHR,
    meREGEXITEM_BEGBUFF,
    meREGEXITEM_ENDBUFF,
    meREGEXITEM_BEGLINE,
    meREGEXITEM_ENDLINE,
    meREGEXITEM_BEGWORD,
    meREGEXITEM_ENDWORD,
    meREGEXITEM_WORDBND,
    meREGEXITEM_NWORDBND
} meREGEXITEM ;

/* meRegexClass functions and macros */
#define meRegexClassTest(clss,cc)    (clss[(cc)>>3] &   (1<<((cc)&0x7)))
#define meRegexClassSet(clss,cc)     (clss[(cc)>>3] |=  (1<<((cc)&0x7)))
#define meRegexClassClear(clss,cc)   (clss[(cc)>>3] &= ~(1<<((cc)&0x7)))
#define meRegexClassToggle(clss,cc)  (clss[(cc)>>3] ^=  (1<<((cc)&0x7)))
#define meRegexClassSetAll(clss)     (memset(clss,0xff,meREGEXCLASS_SIZE))
#define meRegexClassClearAll(clss)   (memset(clss,0x00,meREGEXCLASS_SIZE))

#define meREGEX_MAX_NESTED_GROUPS 20
/* static variables to store the matching info */
static int           matchFlags ;
static meRegexItem **matchItem=NULL ;
static meRegexItem **matchAlt=NULL ;
static int          *matchIOff ;
static int          *matchIMNo ;
static int           matchLen ;
static int           matchSz=0 ;

/* during regex compiling, we know if the nth \( has been encountered
 * but we need to know if the \) has been encounted, this is cause 
 * "\1\(x\)" == ERROR
 * "\(x\1\)" == x1
 * "\(x\)\1" == xx
 * Weird but true - so we use the matchItem array to store this using
 * the macro below
 */
#define meRegexGroupCompComplete(n) (((unsigned char *) matchItem)[(n)])
#define meRegexGroupCompDepth matchLen

char *meRegexCompErrors[meREGEX_ERROR_MALLOC+1]={
    "OKAY",
    "Unknown",
    "Trailing \\",
    "Open class ([...])",
    "Open group (\\(...\\))",
    "Bad class ([...])",
    "Unsupported class (\\sC)",
    "Bad \\ char",
    "Unmatched \\)",
    "Bad interval (\\{...\\})",
    "Undefined back-reference (\\#)",
    "Too many nested groups (\\(...\\))",
    "Malloc failure"
} ;

static void
meRegexClassToggleAll(meRegexClass clss)
{
    int ii=meREGEXCLASS_SIZE ;
    while(--ii >= 0)
        clss[ii] ^= 0xff ;
}
static void
meRegexClassMergeAll(meRegexClass dclss, meRegexClass sclss)
{
    int ii=meREGEXCLASS_SIZE ;
    while(--ii >= 0)
        dclss[ii] |= sclss[ii] ;
}


#define meRegexItemMatch(regex,item,string,len,offset,retOffset) \
do { \
    retOffset = -1 ; \
    switch(item->type) \
    { \
    case meREGEXITEM_NOOP: \
        retOffset = offset ; \
        break ; \
    case meREGEXITEM_GROUP: \
        retOffset = offset ; \
        break ; \
    case meREGEXITEM_BACKREF: \
        { \
            /* there are a couple of problems with this, the obvious \
             * one is case sensitivity (or lack of) the strncmp should \
             * be case insensitive when necesary. The other one is \
             * obscur and really a bogus regex. No distinction is made \
             * between the start and end makers, if the \# is in the closure, \
             * this can go wrong second time around (note that to get a second \
             * time around there must have been a first, i.e. consider: \
             *    \(a\|\1\)* \
             */ \
            meRegexItem *__bri ; \
            int __so, __eo, __ii ; \
            retOffset = -1 ; \
            /* look back for the enclosure */ \
            __eo = item->data.group ; \
            __ii = matchLen ; \
            while(--__ii > 0) \
                if(((__bri=matchItem[__ii])->type == meREGEXITEM_GROUP) && \
                   (__bri->data.group == __eo)) \
                    break ; \
            if(__ii > 0) \
            { \
                __eo = matchIOff[__ii] ; \
                while(--__ii > 0) \
                    if(matchItem[__ii] == __bri) \
                    { \
                        __so = matchIOff[__ii] ; \
                        __ii = __eo - __so ; \
                        if(!strncmp((char *) string+__so,(char *) string+offset,__ii)) \
                            retOffset = offset+__ii ; \
                        break ; \
                    } \
            } \
            break ; \
        } \
    case meREGEXITEM_SINGLE: \
        if(string[offset] == item->data.cc) \
            retOffset = offset+1 ; \
        break ; \
    case meREGEXITEM_DOUBLE: \
        if((string[offset] == item->data.dd[0]) || (string[offset] == item->data.dd[1])) \
            retOffset = offset+1 ; \
        break ; \
    case meREGEXITEM_CLASS: \
        if(meRegexClassTest(item->data.cclass,string[offset])) \
            retOffset = offset+1 ; \
        break ; \
    case meREGEXITEM_ANYCHR: \
        if((string[offset] != '\0') && (string[offset] != '\n')) \
            retOffset = offset+1 ; \
        break ; \
    case meREGEXITEM_ALPHACHR: \
        if((string[offset] != '\0') && isAlpha(string[offset])) \
            retOffset = offset+1 ; \
        break ; \
    case meREGEXITEM_NALPHACHR: \
        if((string[offset] != '\0') && !isAlpha(string[offset])) \
            retOffset = offset+1 ; \
        break ; \
    case meREGEXITEM_DIGITCHR: \
        if((string[offset] != '\0') && isDigit(string[offset])) \
            retOffset = offset+1 ; \
        break ; \
    case meREGEXITEM_NDIGITCHR: \
        if((string[offset] != '\0') && !isDigit(string[offset])) \
            retOffset = offset+1 ; \
        break ; \
    case meREGEXITEM_XDIGITCHR: \
        if((string[offset] != '\0') && isXDigit(string[offset])) \
            retOffset = offset+1 ; \
        break ; \
    case meREGEXITEM_NXDIGITCHR: \
        if((string[offset] != '\0') && !isXDigit(string[offset])) \
            retOffset = offset+1 ; \
        break ; \
    case meREGEXITEM_LOWERCHR: \
        if((string[offset] != '\0') && isLower(string[offset])) \
            retOffset = offset+1 ; \
        break ; \
    case meREGEXITEM_NLOWERCHR: \
        if((string[offset] != '\0') && !isLower(string[offset])) \
            retOffset = offset+1 ; \
        break ; \
    case meREGEXITEM_ALNUMCHR: \
        if((string[offset] != '\0') && isAlphaNum(string[offset])) \
            retOffset = offset+1 ; \
        break ; \
    case meREGEXITEM_NALNUMCHR: \
        if((string[offset] != '\0') && !isAlphaNum(string[offset])) \
            retOffset = offset+1 ; \
        break ; \
    case meREGEXITEM_SPACECHR: \
        if((string[offset] != '\0') && isSpace(string[offset])) \
            retOffset = offset+1 ; \
        break ; \
    case meREGEXITEM_NSPACECHR: \
        if((string[offset] != '\0') && !isSpace(string[offset])) \
            retOffset = offset+1 ; \
        break ; \
    case meREGEXITEM_UPPERCHR: \
        if((string[offset] != '\0') && isUpper(string[offset])) \
            retOffset = offset+1 ; \
        break ; \
    case meREGEXITEM_NUPPERCHR: \
        if((string[offset] != '\0') && !isUpper(string[offset])) \
            retOffset = offset+1 ; \
        break ; \
    case meREGEXITEM_WORDCHR: \
        if((string[offset] != '\0') && isWord(string[offset])) \
            retOffset = offset+1 ; \
        break ; \
    case meREGEXITEM_NWORDCHR: \
        if((string[offset] != '\0') && !isWord(string[offset])) \
            retOffset = offset+1 ; \
        break ; \
    case meREGEXITEM_BEGBUFF: \
        if((matchFlags & meREGEX_BEGBUFF) && (offset == 0)) \
            retOffset = offset ; \
        break ; \
    case meREGEXITEM_ENDBUFF: \
        if((matchFlags & meREGEX_ENDBUFF) && (offset == len)) \
            retOffset = offset ; \
        break ; \
    case meREGEXITEM_BEGLINE: \
        if((offset == 0) || (string[offset-1] == '\n')) \
            retOffset = offset ; \
        break ; \
    case meREGEXITEM_ENDLINE: \
        if((offset == len) || (string[offset] == '\n')) \
            retOffset = offset ; \
        break ; \
    case meREGEXITEM_BEGWORD: \
        if(((offset == 0) || !isWord(string[offset-1])) && isWord(string[offset])) \
            retOffset = offset ; \
        break ; \
    case meREGEXITEM_ENDWORD: \
        if((offset != 0) && !isWord(string[offset]) && isWord(string[offset-1])) \
            retOffset = offset ; \
        break ; \
    case meREGEXITEM_WORDBND: \
        if(offset != 0) \
        { \
            if((isWord(string[offset]) == 0) != (isWord(string[offset-1]) == 0)) \
                retOffset = offset ; \
        } \
        else if(isWord(string[offset])) \
            retOffset = offset ; \
        break ; \
    case meREGEXITEM_NWORDBND: \
        if(offset != 0) \
        { \
            if((isWord(string[offset]) == 0) == (isWord(string[offset-1]) == 0)) \
                retOffset = offset ; \
        } \
        else if(!isWord(string[offset])) \
            retOffset = offset ; \
        break ; \
    } \
} while(0)


static int
meRegexGroupMatch(meRegex *regex, unsigned char *string, int len, int offset)
{
    meRegexItem *ci ;
    int ii, matchNo=0, depth ;
    int curMatchLen[meREGEX_MAX_NESTED_GROUPS+1] ;
    
    ci = regex->head ;
    regex->group[0].end = -1 ;
    /* store the group and start offset and inc*/
    matchItem[0] = NULL ;
    matchAlt[0]  = ci ;
    matchIOff[0] = offset ;
    matchIMNo[0] = 0 ;
    matchLen = 1 ;
    depth = 0 ;
    
    for(;;)
    {
        /* match as many of the item as we can */
add_more:
        while((ci->matchMax < 0) || (matchNo < ci->matchMax))
        {
            meRegexItemMatch(regex,ci,string,len,offset,ii) ;
            if(ii < 0)
                break ;
            matchNo++ ;
            matchItem[matchLen] = ci ;
            matchIOff[matchLen] = ii ;
            matchIMNo[matchLen] = matchNo ;
            matchLen++ ;
            /* check the match buffer size */
            if(matchLen >= matchSz)
            {
                meRegexItem **itp, **iap ;
                int *iip ;
                if((itp=(meRegexItem **)malloc((matchSz+128+4)*(sizeof(meRegexItem *)+sizeof(meRegexItem *)+
                                                sizeof(int)+sizeof(int)))) == NULL)
                    return -1 ;
                matchSz += 128 ;
                memcpy(itp,matchItem,matchLen*sizeof(meRegexItem *)) ;
                iap = (meRegexItem **) (itp+matchSz+4) ;
                memcpy(iap,matchAlt,matchLen*sizeof(meRegexItem *)) ;
                matchAlt = iap ;
                iip = (int *) (iap+matchSz+4) ;
                memcpy(iip,matchIOff,matchLen*sizeof(int)) ;
                matchIOff = iip ;
                iip += matchSz+4 ;
                memcpy(iip,matchIMNo,matchLen*sizeof(int)) ;
                matchIMNo = iip ;
                free(matchItem) ;
                matchItem = itp ;
            }
            if(ci->type == meREGEXITEM_GROUP)
            {
                curMatchLen[depth++] = matchLen-1 ;
                ci = ci->child ;
                matchAlt[matchLen-1] = ci ;
                matchNo = 0 ;
            }
            /* if the item is 0 length (\1 could be), only match it matchMin times */
            else if((matchNo >= ci->matchMin) && (offset == ii))
                break ;
            offset = ii ;
        }
        if(matchNo < ci->matchMin)
        {
back_step:
            /* failed to match, step back if we can */
            while(--matchLen > 0)
            {
                ci = matchItem[matchLen] ;
                matchNo = matchIMNo[matchLen] ;
                if(ci->type == meREGEXITEM_GROUP)
                {
                    if(matchNo < 0)
                    {
                        /* a closing \) or \| */
                        curMatchLen[depth++] = -matchNo ;
                    }
                    /* an opening \( is thare an \| left */
                    else 
                    {
                        ci = matchAlt[matchLen] ;
                        if((ci = ci->alt) != NULL)
                        {
                            offset = matchIOff[matchLen] ;
                            matchAlt[matchLen++] = ci ;
                            matchNo = 0 ;
                            goto add_more ;
                        }
                        depth-- ;
                        ci = matchItem[matchLen] ;
                        if(matchNo > ci->matchMin)
                        {
                            offset = matchIOff[matchLen] ;
                            break ;
                        }
                    }
                }
                else if(matchNo > ci->matchMin)
                {
                    offset = matchIOff[matchLen-1] ;
                    break ;
                }
            }
            if(matchLen == 0)
            {
                if((ci = matchAlt[0]->alt) == NULL)
                    break ;
                offset = matchIOff[0] ;
                matchAlt[0] = ci ;
                matchLen++ ;
                matchNo = 0 ;
                goto add_more ;
            }
        }
move_on:
        if((ci = ci->next) == NULL)
        {
            if(depth == 0)
            {
                if(offset > regex->group[0].end)
                {
                    /* this is the longest match so far, set the group offsets -
                     * note we make the group offsets relative to the match start */
                    int ss = matchIOff[0] ;
                    regex->group[0].start = ss ;
                    regex->group[0].end = offset ;
                    if(regex->groupNo)
                    {
                        ii = regex->groupNo+1 ;
                        while(--ii > 0)
                            regex->group[ii].start = regex->group[ii].end = -1 ;
                        for(ii=1 ; ii < matchLen ; ii++)
                        {
                            if((matchItem[ii])->type == meREGEXITEM_GROUP)
                            {
                                int jj = (matchItem[ii])->data.group ;
                                if((regex->group[jj].start == -1) || (regex->group[jj].end != -1))
                                {
                                    regex->group[jj].start = matchIOff[ii] - ss ;
                                    regex->group[jj].end = -1 ;
                                }
                                else
                                    regex->group[jj].end = matchIOff[ii] - ss ;
                            }
                        }
                    }
                }
                /* get it back pedaling, there may be a longer match */
                depth = 0 ;
                goto back_step ;
            }
            ii = curMatchLen[--depth] ;
            ci = matchItem[ii] ;
            matchNo = matchIMNo[ii] ;
            matchItem[matchLen] = ci ;
            matchIOff[matchLen] = offset ;
            matchIMNo[matchLen] = -curMatchLen[depth] ;
            matchLen++ ;
            /* if the group is 0 length, only match it matchMin times */
            if((matchNo >= ci->matchMin) && (offset == matchIOff[ii]))
                goto move_on ;
        }
        else
            matchNo = 0 ;
    }
    return (regex->group[0].end < 0) ? 0:1 ;
}

/* meRegexMatch
 *
 * returns 1 if matched at current location, else 0
 */
int
meRegexMatch(meRegex *regex, unsigned char *string, int len, 
             int offsetS, int offsetE, int flags)
{
    int offset ;
    
    /* check args are valid first, if not fail */
    if(!(regex->flags & meREGEX_VALID) || (offsetS < 0) || (offsetE > len))
        return 0 ;
    
    matchFlags = flags ;
    
    /* if the regex can only be matched at the start of a line or this
     * match must match the whole string... */
    if((regex->flags & meREGEX_BEGLINE) || (matchFlags & meREGEX_MATCHWHOLE))
    {
        /* if we are at the start test this case else theres no match */
        if((offsetS == 0) &&
           ((regex->flags & meREGEX_MAYBEEMPTY) || meRegexClassTest(regex->start,*string)) &&
           meRegexGroupMatch(regex,string,len,0) &&
           (!(matchFlags & meREGEX_MATCHWHOLE) || (regex->group[0].end == len)))
            return 1 ;
        return 0 ;
    }
    
    if(flags & meREGEX_BACKWARD)
    {
        offset = offsetE ;
        do
            if(((regex->flags & meREGEX_MAYBEEMPTY) || meRegexClassTest(regex->start,string[offset])) &&
               meRegexGroupMatch(regex,string,len,offset))
                return 1 ;
        while(--offset >= offsetS) ;
    }
    else
    {
        offset = offsetS ;
        do
            if(((regex->flags & meREGEX_MAYBEEMPTY) || meRegexClassTest(regex->start,string[offset])) &&
               meRegexGroupMatch(regex,string,len,offset))
                return 1 ;
        while(++offset <= offsetE) ;
    }
    return 0 ;
}

static meRegexItem *
meRegexItemCreate(meRegex *regex)
{
    meRegexItem *item ;
    
    if((item=regex->lnext) == NULL)
    {
        if((item=(meRegexItem *)malloc(sizeof(meRegexItem))) == NULL)
            return NULL ;
        item->lnext = regex->lhead ;
        regex->lhead = item ;
    }
    else
        regex->lnext = item->lnext ;
    item->next = NULL ;
    item->child = NULL ;
    item->alt = NULL ;
    item->matchMin = 1 ;
    item->matchMax = 1 ;
    return item ;
}

static int
meRegexGroupComp(meRegex *regex, meRegexItem **head, unsigned char **regStr) ;

/* NOTE - Then next these are not GNU compliant, but they make life easier! */
#define meRegexItemGetBackslashChar(ss,cc)                                   \
do {                                                                         \
    switch(cc)                                                               \
    {                                                                        \
    case 'e':   cc = 0x1b; break;                                            \
    case 'f':   cc = 0x0c; break;                                            \
    case 'g':   cc = 0x07; break;                                            \
    case 'n':   cc = 0x0a; break;                                            \
    case 'r':   cc = 0x0d; break;                                            \
    case 't':   cc = 0x09; break;                                            \
    case 'v':   cc = 0x0b; break;                                            \
    case 'x':                                                                \
        cc = *ss ;                                                           \
        if(isXDigit(cc))                                                     \
        {                                                                    \
            register unsigned char c1 ;                                      \
            c1 = *++ss ;                                                     \
            if(isXDigit(c1))                                                 \
            {                                                                \
                cc = (hexToNum(cc) << 4) | hexToNum(c1) ;                    \
                ss++ ;                                                       \
            }                                                                \
            else                                                             \
                cc = 0 ;                                                     \
        }                                                                    \
        else                                                                 \
            cc = 0 ;                                                         \
        break;                                                               \
    }                                                                        \
    if(cc == '\0')                                                           \
    {                                                                        \
        if(*ss == '\0')                                                      \
            return meREGEX_ERROR_TRAIL_BKSSH ;                               \
        return meREGEX_ERROR_BKSSH_CHAR ;                                    \
    }                                                                        \
} while(0)

static int
meRegexItemGet(meRegex *regex, meRegexItem *lastItem,
               unsigned char **regStr, meRegexItem **retItem)
{
    unsigned char *rs=*regStr, cc, dd ;
    meRegexItem *item ;
    
    /* if '\0' termination here is unexpected */
    if((cc=*rs++) == '\0')
        return meREGEX_ERROR_UNKNOWN ;
    
    if(cc == '\\')
    {
        if((dd=*rs++) == '\0')
            return meREGEX_ERROR_TRAIL_BKSSH ;
        if(dd == '|')
        {
            /* return, the calling preceedure will handle this */
            *retItem = NULL ;
            *regStr = rs ;
            return meREGEX_ALT ;
        }
        if(dd == ')')
        {
            /* return, the calling preceedure will handle this */
            *retItem = NULL ;
            *regStr = rs ;
            return meREGEX_ERROR_CGROUP ;
        }
    }
    if((lastItem != NULL) && (lastItem->type < meREGEXITEM_BEGBUFF))
    {
        int min=-1, max ;
        if(cc == '?')
        {
            min = 0 ;
            max = 1 ;
        }
        else if(cc == '*')
        {
            min = 0 ;
            max = -1 ;
        }
        else if(cc == '+')
        {
            min = 1 ;
            max = -1 ;
        }
        else if((cc == '\\') && (dd == '{'))
        {
            unsigned char *srs=rs ;
            while(isDigit(*rs))
                rs++ ;
            if(rs != srs)
                min = atoi((char *) srs) ;
            else if(*rs != ',')
                return meREGEX_ERROR_INTERVAL ;
            else
                min = 0 ;
            if((cc=*rs++) == ',')
            {
                srs=rs ;
                while(isDigit(*rs))
                    rs++ ;
                if(rs != srs)
                    max = atoi((char *) srs) ;
                else
                    max = -1 ;
                cc = *rs++ ;
            }
            else
                max = min ;
            
            if((cc != '\\') || (*rs++ != '}'))
                return meREGEX_ERROR_INTERVAL ;
        }
        if(min >= 0)
        {
            lastItem->matchMin = min ;
            lastItem->matchMax = max ;
            *retItem = NULL ;
            *regStr = rs ;
            return meREGEX_OKAY ;
        }
    }
    
    /* either an item or and error will occur, so get the item node */
    if((item=meRegexItemCreate(regex)) == NULL)
        return meREGEX_ERROR_MALLOC ;
    *retItem = item ;
    
    if(cc == '\\')
    {
        switch(dd)
        {
        case '(':
            {
                int gg, ii ;
                if(++meRegexGroupCompDepth >= meREGEX_MAX_NESTED_GROUPS)
                    return meREGEX_ERROR_MALLOC ;
                item->type = meREGEXITEM_GROUP ;
                gg = ++(regex->groupNo) ;
                item->data.group = gg ;
                /* set to 0 while compiling - \gg will -> gg*/
                meRegexGroupCompComplete(gg) = 0 ;
                if((ii=meRegexGroupComp(regex,&(item->child),&rs)) != meREGEX_ERROR_CGROUP)
                {
                    if(ii == meREGEX_OKAY)
                        ii = meREGEX_ERROR_OGROUP ;
                    return ii ;
                }
                meRegexGroupCompComplete(gg) = 1 ;
                meRegexGroupCompDepth-- ;
                break ;
            }
        case '`':
            item->type = meREGEXITEM_BEGBUFF ;
            break ;
        case '\'':
            item->type = meREGEXITEM_ENDBUFF ;
            regex->flags |= meREGEX_MAYENDBUFF ;
            break ;
        case '<':
            item->type = meREGEXITEM_BEGWORD ;
            break ;
        case '>':
            item->type = meREGEXITEM_ENDWORD ;
            break ;
        case 'a':
            item->type = meREGEXITEM_ALPHACHR ;
            break ;
        case 'A':
            item->type = meREGEXITEM_NALPHACHR ;
            break ;
        case 'b':
            item->type = meREGEXITEM_WORDBND ;
            break ;
        case 'B':
            item->type = meREGEXITEM_NWORDBND ;
            break ;
        case 'd':
            item->type = meREGEXITEM_DIGITCHR ;
            break ;
        case 'D':
            item->type = meREGEXITEM_NDIGITCHR ;
            break ;
        case 'h':
            item->type = meREGEXITEM_XDIGITCHR ;
            break ;
        case 'H':
            item->type = meREGEXITEM_NXDIGITCHR ;
            break ;
        case 'l':
            item->type = meREGEXITEM_LOWERCHR ;
            break ;
        case 'L':
            item->type = meREGEXITEM_NLOWERCHR ;
            break ;
        case 'm':
            item->type = meREGEXITEM_ALNUMCHR ;
            break ;
        case 'M':
            item->type = meREGEXITEM_NALNUMCHR ;
            break ;
        case 's':
            item->type = meREGEXITEM_SPACECHR ;
            break ;
        case 'S':
            item->type = meREGEXITEM_NSPACECHR ;
            break ;
        case 'u':
            item->type = meREGEXITEM_UPPERCHR ;
            break ;
        case 'U':
            item->type = meREGEXITEM_NUPPERCHR ;
            break ;
        case 'w':
            item->type = meREGEXITEM_WORDCHR ;
            break ;
        case 'W':
            item->type = meREGEXITEM_NWORDCHR ;
            break ;
            
        case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            dd -= '0' ;
            if((dd == 0) || (dd > regex->groupNo))
                return meREGEX_ERROR_BACK_REF ;
            /* if we're currently compiling this group, this is a
             * literal char */
            if(meRegexGroupCompComplete(dd))
            {
                item->type = meREGEXITEM_BACKREF ;
                item->data.group = dd ;
                break ;
            }
            dd += '0' ;
            /* no break, literal char */
        default:
            meRegexItemGetBackslashChar(rs,dd) ;
            if((regex->flags & meREGEX_ICASE) && isAlpha(dd))
            {
                item->type = meREGEXITEM_DOUBLE ;
                item->data.dd[0] = dd ;
                item->data.dd[1] = toggleCase(dd) ;
            }
            else
            {
                item->type = meREGEXITEM_SINGLE ;
                item->data.cc = dd ;
            }
        }
    }
    else if(cc == '[')
    {
        int inv ;
        item->type = meREGEXITEM_CLASS ;
        meRegexClassClearAll(item->data.cclass) ;
        if(*rs == '^')
        {
            inv = 1 ;
            rs++ ;
        }
        else
            inv = 0 ;
        cc = *rs++ ;
        do
        {
            if(cc == '\0')
                return meREGEX_ERROR_OCLASS ;
            if(cc == '\\')
            {
                static char *classChar="aAdDhHlLmMsSuUwW" ;
                char *ss ;
                cc = *rs++ ;
                if((ss=strchr(classChar,cc)) != NULL)
                {
                    int ii, jj, kk, invert ;
                    ii = ((size_t) ss) - ((size_t) classChar) ;
                    invert = ii & 1 ;
                    ii >>= 1 ;
                    jj = 255 ;
                    do
                    {
                        if(ii == 0)
                            kk = isAlpha(jj) ;
                        else if(ii == 1)
                            kk = isDigit(jj) ;
                        else if(ii == 2)
                            kk = isXDigit(jj) ;
                        else if(ii == 3)
                            kk = isLower(jj) ;
                        else if(ii == 4)
                            kk = isAlphaNum(jj) ;
                        else if(ii == 5)
                            kk = isSpace(jj) ;
                        else if(ii == 6)
                            kk = isUpper(jj) ;
                        else
                            kk = isWord(jj) ;
                        if((kk != 0) ^ invert)
                            meRegexClassSet(item->data.cclass,jj) ;
                    } while(--jj > 0) ;
                    continue ;
                }
                else
                    meRegexItemGetBackslashChar(rs,cc) ;
            }
            if((*rs == '-') && ((dd=rs[1]) != ']'))
            {
                rs += 2 ;
                if(dd == '\\')
                {
                    dd = *rs++ ;
                    meRegexItemGetBackslashChar(rs,dd) ;
                }
                if(dd < cc)
                    return meREGEX_ERROR_CLASS ;
                do {
                    meRegexClassSet(item->data.cclass,cc) ;
                    if((regex->flags & meREGEX_ICASE) && isAlpha(cc))
                    {
                        unsigned char ee=toggleCase(cc) ;
                        meRegexClassSet(item->data.cclass,ee) ;
                    }
                } while(cc++ != dd) ;
            }
            else if((cc == '[') && (*rs == ':'))
            {
                unsigned char *s1=rs+1, *s2 ;
                int ii=0, jj ;
                s2 = s1 ;
                while((cc=*s2++) != '\0')
                {
                    if((cc == ':') && (*s2 == ']'))
                        break ;
                    ii++ ;
                }
                if(cc == '\0')
                    ;
                else if((ii == 5) && !strncmp((char *) s1,"space",5))
                    ii = 0x1 ;
                else if((ii == 5) && !strncmp((char *) s1,"blank",5))
                    ii = 0x1 ;
                else if((ii == 5) && !strncmp((char *) s1,"digit",5))
                    ii = 0x2 ;
                else if((ii == 6) && !strncmp((char *) s1,"xdigit",6))
                    ii = 0x4 ;
                else if((ii == 5) && !strncmp((char *) s1,"upper",5))
                    ii = 0x8 ;
                else if((ii == 5) && !strncmp((char *) s1,"lower",5))
                    ii = 0x10 ;
                else if((ii == 5) && !strncmp((char *) s1,"alpha",5))
                    ii = 0x18 ;
                else if((ii == 5) && !strncmp((char *) s1,"alnum",5))
                    ii = 0x1a ;
                else if((ii == 5) && !strncmp((char *) s1,"print",5))
                    ii = 0x20 ;
                else if((ii == 5) && !strncmp((char *) s1,"punct",5))
                    ii = 0x40 ;
                else if((ii == 5) && !strncmp((char *) s1,"graph",5))
                    ii = 0x80 ;
                else if((ii == 5) && !strncmp((char *) s1,"cntrl",5))
                    ii = 0x100 ;
                else
                    cc = '\0' ;
                
                if(cc != '\0')
                {
                    /* don't bother with '\0' - can never match */
                    jj = 255 ;
                    do
                    {
                        if(((ii & 0x001) && isSpace(jj)) ||
                           ((ii & 0x002) && isDigit(jj)) ||
                           ((ii & 0x004) && isXDigit(jj)) ||
                           ((ii & 0x008) && isLower(jj)) ||
                           ((ii & 0x010) && isUpper(jj)) ||
                           ((ii & 0x020) && isPrint(jj)) ||
                           ((ii & 0x040) && isPunct(jj)) ||
                           ((ii & 0x080) && isGraph(jj)) ||
                           ((ii & 0x100) && isCntrl(jj)) )
                            meRegexClassSet(item->data.cclass,jj) ;
                    } while(--jj > 0) ;
                    rs = s2+1 ;
                }
                else
                    meRegexClassSet(item->data.cclass,']') ;
            }
            else
            {
                meRegexClassSet(item->data.cclass,cc) ;
                if((regex->flags & meREGEX_ICASE) && isAlpha(cc))
                {
                    cc = toggleCase(cc) ;
                    meRegexClassSet(item->data.cclass,cc) ;
                }
            }
        } while((cc=*rs++) != ']') ;
        if(inv)
        {
            meRegexClassToggleAll(item->data.cclass) ;
            /* should not match \0 */
            meRegexClassClear(item->data.cclass,'\0') ;
        }
    }
    else if((cc == '^') && (lastItem == NULL))
        item->type = meREGEXITEM_BEGLINE ;
    else if(cc == '$')
        item->type = meREGEXITEM_ENDLINE ;
    else if(cc == '.')
        item->type = meREGEXITEM_ANYCHR ;
    else if((regex->flags & meREGEX_ICASE) && isAlpha(cc))
    {
        item->type = meREGEXITEM_DOUBLE ;
        item->data.dd[0] = cc ;
        item->data.dd[1] = toggleCase(cc) ;
    }
    else
    {
        item->type = meREGEXITEM_SINGLE ;
        item->data.cc = cc ;
    }
    {
        /* if this item will match a \n then increment the newlNo counter */
        static unsigned char newlStr[]="\n" ;
        int ii ;
        meRegexItemMatch(regex,item,newlStr,1,0,ii) ;
        if(ii > 0)
            (regex->newlNo)++ ;
    }
    *regStr = rs ;
    return meREGEX_OKAY ;
}


static int
meRegexGroupComp(meRegex *regex, meRegexItem **head, unsigned char **regStr)
{
    meRegexItem *ci, *ni, *hi ;
    int ii, snewlNo, lnewlNo ;
    
    snewlNo = lnewlNo = regex->newlNo ;
    *head = NULL ;
    hi = ci = NULL ;
    while((*regStr)[0] != '\0')
    {
        if((ii = meRegexItemGet(regex,ci,regStr,&ni)) != meREGEX_OKAY)
        {
            /* can be \| or \) in which case we must create a NOOP is 
             * the Alt is empty and continue if its a \| 
             */
            if((ii != meREGEX_ALT) && (ii != meREGEX_ERROR_CGROUP))
                return ii ;
            if(ci == NULL)
            {
                if((ci=meRegexItemCreate(regex)) == NULL)
                    return meREGEX_ERROR_MALLOC ;
                ci->type = meREGEXITEM_NOOP ;
                if(hi != NULL)
                    hi->alt = ci ;
                else
                    *head = ci ;
                hi = ci ;
            }
            if(regex->newlNo > lnewlNo)
                lnewlNo = regex->newlNo ;
            if(ii == meREGEX_ERROR_CGROUP)
            {
                regex->newlNo = lnewlNo ;
                return ii ;
            }
            regex->newlNo = snewlNo ;
            ci = NULL ;
        }
        if(ni != NULL)
        {
            if(ci == NULL)
            {
                if(hi != NULL)
                    hi->alt = ni ;
                else
                    *head = ni ;
                hi = ni ;
            }
            else
                ci->next = ni ;
            ci = ni ;
        }
    }
    if(ci == NULL)
    {
        if((ci=meRegexItemCreate(regex)) == NULL)
            return meREGEX_ERROR_MALLOC ;
        ci->type = meREGEXITEM_NOOP ;
        if(hi != NULL)
            hi->alt = ci ;
        else
            *head = ci ;
    }
    if(regex->newlNo < lnewlNo)
        regex->newlNo = lnewlNo ;
    return meREGEX_OKAY ;
}

static int
meRegexGroupStartComp(meRegex *regex, meRegexItem *hi, int first) ;

static int
meRegexItemStartComp(meRegex *regex, meRegexItem *item, int first)
{
    int gotStart ;
    
    switch(item->type)
    {
    case meREGEXITEM_SINGLE:
        meRegexClassSet(regex->start,item->data.cc) ;
        gotStart = 1 ;
        break ;
    case meREGEXITEM_DOUBLE:
        meRegexClassSet(regex->start,item->data.dd[0]) ;
        meRegexClassSet(regex->start,item->data.dd[1]) ;
        gotStart = 1 ;
        break ;
    case meREGEXITEM_GROUP:
        gotStart = meRegexGroupStartComp(regex,item->child,first) ;
        /* the first is past down to the group to handle, is flag as not the first */
        first = 0 ;
        break ;
    case meREGEXITEM_CLASS:
        meRegexClassMergeAll(regex->start,item->data.cclass) ;
        gotStart = 1 ;
        break ;
    case meREGEXITEM_DIGITCHR:
        {
            int ii=9 ;
            do
                meRegexClassSet(regex->start,'0'+ii) ;
            while(--ii >= 0) ;
        }
        gotStart = 1 ;
        break ;
    case meREGEXITEM_NDIGITCHR:
        {
            int ii=255 ;
            /* don't bother with '\0' - can never match */
            do
                if((ii < '0') || (ii > '9'))
                    meRegexClassSet(regex->start,ii) ;
            while(--ii > 0) ;
        }
        gotStart = 1 ;
        break ;
    case meREGEXITEM_ALPHACHR:
    case meREGEXITEM_NALPHACHR:
    case meREGEXITEM_XDIGITCHR:
    case meREGEXITEM_NXDIGITCHR:
    case meREGEXITEM_LOWERCHR:
    case meREGEXITEM_NLOWERCHR:
    case meREGEXITEM_ALNUMCHR:
    case meREGEXITEM_NALNUMCHR:
    case meREGEXITEM_SPACECHR:
    case meREGEXITEM_NSPACECHR:
    case meREGEXITEM_UPPERCHR:
    case meREGEXITEM_NUPPERCHR:
    case meREGEXITEM_WORDCHR:
    case meREGEXITEM_NWORDCHR:
        /* the WORDCHR & SPACE classes are not burnt in so flag
         * that we are using them so if the classes are flagged as
         * changed the first will be recompiled.
         */
        {
            int ii=255, jj, invert, type ;
            type = item->type ;
            if((invert = (type == meREGEXITEM_NSPACECHR)))
                type = meREGEXITEM_SPACECHR ;
            else if((invert = (type == meREGEXITEM_NWORDCHR)))
                type = meREGEXITEM_WORDCHR ;
            else if((invert = (type == meREGEXITEM_NXDIGITCHR)))
                type = meREGEXITEM_XDIGITCHR ;
            else if((invert = (type == meREGEXITEM_NLOWERCHR)))
                type = meREGEXITEM_LOWERCHR ;
            else if((invert = (type == meREGEXITEM_NUPPERCHR)))
                type = meREGEXITEM_UPPERCHR ;
            else if((invert = (type == meREGEXITEM_NALNUMCHR)))
                type = meREGEXITEM_ALNUMCHR ;
            else if((invert = (type == meREGEXITEM_NALPHACHR)))
                type = meREGEXITEM_ALPHACHR ;
            
            /* don't bother with '\0' - can never match */
            do
            {
                if(type == meREGEXITEM_SPACECHR)
                    jj = isSpace(ii) ;
                else if(type == meREGEXITEM_WORDCHR)
                    jj = isWord(ii) ;
                else if(type == meREGEXITEM_XDIGITCHR)
                    jj = isXDigit(ii) ;
                else if(type == meREGEXITEM_LOWERCHR)
                    jj = isLower(ii) ;
                else if(type == meREGEXITEM_UPPERCHR)
                    jj = isUpper(ii) ;
                else if(type == meREGEXITEM_ALNUMCHR)
                    jj = isAlphaNum(ii) ;
                else
                    jj = isAlpha(ii) ;
                if((jj != 0) ^ invert)
                    meRegexClassSet(regex->start,ii) ;
            } while(--ii > 0) ;
            if(type != meREGEXITEM_XDIGITCHR)
                regex->flags |= meREGEX_CLASSUSED ;
            gotStart = 1 ;
            break ;
        }
    case meREGEXITEM_ANYCHR:
        meRegexClassSetAll(regex->start) ;
        /* this can never match \0 or \n */
        meRegexClassClear(regex->start,'\0') ;
        meRegexClassClear(regex->start,'\n') ;
        gotStart = 1 ;
        break ;
    default:
        /* meREGEXITEM_NOOP and meREGEXITEM_BEGBUFF type items return 0 as
         * they are not chars. meREGEXITEM_BACKREF returns 0 cos to have got
         * here we must have parsed the Group and if we're still we should
         * continue */
        gotStart = 0 ;
    }
    
    if(gotStart)
        /* we may have got a start but if matchMin is 0, we haven't! */
        gotStart = item->matchMin ;
    
    if(first && (item->type != meREGEXITEM_BEGBUFF) && (item->type != meREGEXITEM_BEGLINE))
        regex->flags &= ~meREGEX_BEGLINE ;
    return gotStart ;
}

static int
meRegexGroupStartComp(meRegex *regex, meRegexItem *hi, int first)
{
    meRegexItem *ci ;
    int gotStart=1, ff ;
    
    while(hi != NULL)
    {
        ff = first ;
        ci = hi ;
        do {
            if(meRegexItemStartComp(regex,ci,ff))
                /* we got a definite start - break out */
                break ;
            ff = 0 ;
        } while((ci = ci->next) != NULL) ;
        if(ci == NULL)
            gotStart = 0 ;
        hi = hi->alt ;
    }
    return gotStart ;
}

int
meRegexComp(meRegex *regex, unsigned char *regStr, int flags)
{
    int ii ;
    
    /* If first time, malloc initial match buffers */
    if(matchSz == 0)
    {
        if((matchItem=(meRegexItem **)malloc((128+4)*(sizeof(meRegexItem *)+sizeof(meRegexItem *)+
                                                      sizeof(int)+sizeof(int)))) == NULL)
            return meREGEX_ERROR_MALLOC ;
        matchSz = 128 ;
        matchAlt  = (meRegexItem **) (matchItem+128+4) ;
        matchIOff = (int *) (matchAlt+128+4) ;
        matchIMNo = (int *) (matchIOff+128+4) ;
    }
    if(((ii=strlen((char *) regStr)) > regex->regexSz) || (regex->regex == NULL))
    {
        if((regex->regex = (unsigned char *)realloc(regex->regex,ii+1)) == NULL)
        {
            regex->regexSz = 0 ;
            return meREGEX_ERROR_MALLOC ;
        }
        regex->regexSz = ii ;
    }
    else if(((regex->flags & (meREGEX_VALID|meREGEX_ICASE)) == (flags|meREGEX_VALID)) && 
            !strcmp((char *) regStr,(char *) regex->regex))
    {
        /* same as last time, check and maybe recompile the first,
         * then return */
        if((regex->flags & meREGEX_CLASSUSED) &&
           (regex->flags & meREGEX_CLASSCHANGE))
        {
            regex->flags &= ~meREGEX_CLASSCHANGE ;
            regex->flags |= meREGEX_BEGLINE ;
            meRegexClassClearAll(regex->start) ;
            if(!meRegexGroupStartComp(regex,regex->head,1))
                regex->flags |= meREGEX_MAYBEEMPTY ;
        }
        return meREGEX_OKAY ;
    }
    
    /* reset regex variables */
    strcpy((char *) regex->regex,(char *) regStr) ;
    regex->flags = flags ;
    regex->lnext = regex->lhead ;
    regex->groupNo = 0 ;
    regex->newlNo = 0 ;
    meRegexGroupCompDepth = 0 ;    
    if((ii=meRegexGroupComp(regex,&(regex->head),&regStr)) != meREGEX_OKAY)
        return ii ;
    
    /* check the group array is large enough */
    if(regex->groupNo >= regex->groupSz)
    {
        if((regex->group=(meRegexGroup *)realloc(regex->group,(regex->groupNo+1) * sizeof(meRegexGroup))) == NULL)
        {
            regex->groupSz = 0 ;
            return meREGEX_ERROR_MALLOC ;
        }
        regex->groupSz = regex->groupNo+1 ;
    }
    /* flag it as valid and compile the start optimizations */
    regex->flags |= meREGEX_VALID|meREGEX_BEGLINE ;
    meRegexClassClearAll(regex->start) ;
    if(!meRegexGroupStartComp(regex,regex->head,1))
        regex->flags |= meREGEX_MAYBEEMPTY ;
    
    return meREGEX_OKAY ;
}

#if (defined _ME_FREE_ALL_MEMORY) || (defined _TEST_RIG)

void
meRegexFree(meRegex *regex)
{
    meRegexItem *item ;
    
    regex->flags = 0 ;
    while((item=regex->lhead) != NULL)
    {
        regex->lhead = item->lnext ;
        free(item) ;
    }
    if(regex->regex != NULL)
    {
        free(regex->regex) ;
        regex->regex = NULL ;
        regex->regexSz = 0 ;
    }
    if(regex->group != NULL)
    {
        free(regex->group) ;
        regex->group = NULL ;
        regex->groupSz = 0 ;
    }
}
#endif
#ifdef _TEST_RIG

#include <time.h>

char *meRegexItemNames[] = {
    "NOOP    ",
    "GROUP   ",
    "BACKREF ",
    "SINGLE  ",
    "DOUBLE  ",
    "CLASS   ",
    "ANYCHR  ",
    "SPACECHR ",
    "NSPACECHR",
    "WORDCHR ",
    "NWORDCHR",
    "BEGBUFF ",
    "ENDBUFF ",
    "BEGLINE ",
    "ENDLINE ",
    "BEGWORD ",
    "ENDWORD ",
    "WORDBND ",
    "NWORDBND"
} ;

static void
meRegexGroupPrint(meRegexItem *hi, int indent) ;

static void
meRegexItemPrint(meRegexItem *item, int indent)
{
    int ii ;
    printf("%*c%s %d %2 d ",indent,' ',meRegexItemNames[item->type],item->matchMin,item->matchMax) ;
    switch(item->type) {
    case meREGEXITEM_SINGLE:
        printf("'%c'\n",item->data.cc) ;
        break ;
    case meREGEXITEM_DOUBLE:
        printf("'%c' '%c'\n",item->data.dd[0],item->data.dd[1]) ;
        break ;
    case meREGEXITEM_BACKREF:
        printf("%d\n",item->data.group) ;
        break ;
    case meREGEXITEM_GROUP:
        printf("%d\n",item->data.group) ;
        meRegexGroupPrint(item->child,indent+2) ;
        break ;
    case meREGEXITEM_CLASS:
        for(ii=0 ; ii<meREGEXCLASS_SIZE ; ii++)
            printf("%02x ",item->data.cclass[ii]) ;
        /* no break */
    default:
        printf("\n") ;
    }
}

static void
meRegexGroupPrint(meRegexItem *hi, int indent)
{
    meRegexItem *ci ;
    int ii=0 ;
    
    while(hi != NULL)
    {
        printf("%*cver %d\n",indent,' ',++ii) ;
        ci = hi ;
        do
            meRegexItemPrint(ci,indent+2) ;
        while((ci = ci->next) != NULL) ;
        hi = hi->alt ;
    }
}

void
meRegexPrint(meRegex *regex)
{
    int ii ;
    printf("Regex [%s] 0x0%x %d lines\nStart: ",regex->regex,regex->flags,regex->newlNo) ;
    for(ii=0 ; ii<meREGEXCLASS_SIZE ; ii++)
        printf("%02x ",regex->start[ii]) ;
    printf("\n\n") ;
    meRegexGroupPrint(regex->head,2) ;
}

static meRegex mereRegex={0} ;

int
main(int argc, char *argv[])
{
    int ii, cflags=0, mflags=0, offsetS, offsetE ;
    unsigned char *rs, *ss ;
    clock_t st, mt, et ;
    
    if(argc < 2)
    {
        printf("Usage: %s <regex-str> <match-str> [flags] [start-offset,end-offset]\n\n"
               "Where flags can be any combination of:\n"
               "       b - Search backwards\n"
               "       B - match-str is at the beginning of the buffer\n"
               "       E - match-str is at the end of the buffer\n"
               "       i - Case insensitive search\n\n",argv[0]) ;
        return 1 ;
    }
    if(argc < 3)
    {
        ii = atoi(argv[1]) ;
        /* \(a.*bcde\|a.*cde\|a.*de\|a.*e\)\(a.*b\|a.*c\|a.*d\|a.*e\)\1 */
        switch(ii)
        {
        case 1:
            rs = (unsigned char *) "\\(a.*\\|ab.*\\|abc.*\\|abc.*\\)\\(a.*\\|ab.*\\|abc.*\\|abc.*\\)\\1" ;
            ss = (unsigned char *) "abcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcd" ;
            break ;
        default:
            rs = (unsigned char *) "\\(a.*bcde\\|a.*cde\\|a.*de\\|a.*e\\)\\(a.*b\\|a.*c\\|a.*d\\|a.*e\\)\\1" ;
            ss = (unsigned char *) "abcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcd" ;
            break ;
        }
    }
    else
    {
        rs = (unsigned char *) argv[1] ;
        ss = (unsigned char *) argv[2] ;
        if(argc > 3)
        {
            if(strchr(argv[3],'i') != NULL)
                cflags |= meREGEX_ICASE ;
            if(strchr(argv[3],'b') != NULL)
                mflags |= meREGEX_BACKWARD ;
            if(strchr(argv[3],'B') != NULL)
                mflags |= meREGEX_BEGBUFF ;
            if(strchr(argv[3],'E') != NULL)
                mflags |= meREGEX_ENDBUFF ;
        }
    }
    if(argc <= 4)
    {
        offsetS = 0 ;
        offsetE = strlen(ss) ;
    }
    else
        sscanf(argv[4],"%d,%d",&offsetS,&offsetE) ;
    
    st = clock() ;
    if((ii=meRegexComp(&mereRegex,rs,cflags)) != meREGEX_OKAY)
    {
        printf("Failed to compile regex - %d\n",ii) ;
        return ii ;
    }
    printf("compiled regex okay - %d\n",mereRegex.groupNo) ;
    meRegexPrint(&mereRegex) ;
    mt = clock() ;
    
    if(meRegexMatch(&mereRegex,ss,strlen(ss),offsetS,offsetE,mflags))
    {
        printf("regex matched - %d %d\nGroups:",
               mereRegex.group[0].start,
               mereRegex.group[0].end) ;
        for(ii=0 ; ii <= mereRegex.groupNo ; ii++)
            printf(" (%d %d)", mereRegex.group[ii].start, mereRegex.group[ii].end) ;
        printf("\n") ;
    }
    else
        printf("regex failed to matched\n") ;
    et = clock() ;
    printf("Times %d & %d\n",mt-st,et-mt) ;
    
    return 0 ;
}


#endif
#endif
