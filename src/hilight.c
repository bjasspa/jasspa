/* -*- C -*- ****************************************************************
 *
 *  System        : MicroEmacs Jasspa Distribution
 *  Module        : hilight.c
 *  Synopsis      : Token based hilighting routines
 *  Created By    : Steven Phillips
 *  Created       : 21/12/94
 *  Last Modified : <000508.0833>
 *
 *  Description
 *
 *  Notes
 *     The format of hilight tokens are as similar to regex as possible.
 *     There is a mis-use of the \{\} intervals to indicate the start
 *     and end of the hilighted part of a token.
 * 
 ****************************************************************************
 * 
 * Copyright (c) 1994-2000 Steven Phillips    
 *    
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the  authors be held liable for any damages  arising  from
 * the use of this software.
 *     
 * This software was generated as part of the MicroEmacs JASSPA  distribution,
 * (http://www.jasspa.com) but is excluded from those licensing restrictions.
 *
 * Permission  is  granted  to anyone to use this  software  for any  purpose,
 * including  commercial  applications,  and to alter it and  redistribute  it
 * freely, subject to the following restrictions:
 *
 *  1. The origin of this software must not be misrepresented; you must not
 *     claim that you wrote the original software. If you use this software
 *     in a product, an acknowledgment in the product documentation would be
 *     appreciated but is not required.
 *  2. Altered source versions must be plainly marked as such, and must not be
 *     misrepresented as being the original software.
 *  3. This notice may not be removed or altered from any source distribution.
 *
 * Steven Phillips         bill@jasspa.com
 *
 ****************************************************************************/

#define	__HILIGHTC				/* Name file */

#include "emain.h"

/*
 * HilFunc
 * This function is invoked whenever the source text reaches a defined postion,
 * presently defined at the start and end of the highlight selection blocks.
 * The call back function deals with the change in colour that results. 
 * */
struct HILDATA;                         /* Forward reference structure */
typedef void (*HilFunc)(int, struct HILDATA *);

/*
 * HILDATA
 * This structure contains data that is exported from hilightLine to the
 * worker functions. It contains enough information to allow the selection
 * colour modifications to be performed.
 */
typedef struct HILDATA {
    HILNODEPTR root;                    /* Root of the hilighting */
    HILNODEPTR node;                    /* Current node used in hilighting */
    HILNODEPTR cnode;                   /* Current working node */
    HILNODEPTR hnode;                   /* Current hilighting node */
    HILBLOCK *blkp;                     /* Block pointer */
    int noColChng;                      /* Number of colour changes */
    int srcPos;                         /* Source position */
    uint16 srcOff;                      /* Offset in source to find */
    HilFunc hfunc;                      /* Callback function */
    uint16 len;                         /* Transient length information */
    uint16 flag;                        /* Video structure flag */
    uint8 colno;                        /* Color number */
} HILDATA;    

#define meHIL_TEST_BACKREF 0x00
#define meHIL_TEST_SINGLE  0x01
#define meHIL_TEST_CLASS   0x02
#define meHIL_TEST_EOL     0x03
#define meHIL_TEST_SPACE   0x04
#define meHIL_TEST_DIGIT   0x05
#define meHIL_TEST_XDIGIT  0x06
#define meHIL_TEST_LOWER   0x07
#define meHIL_TEST_UPPER   0x08
#define meHIL_TEST_ALPHA   0x09
#define meHIL_TEST_ALNUM   0x0a
#define meHIL_TEST_SWORD   0x0b
#define meHIL_TEST_WORD    0x0c
#define meHIL_TEST_ANY     0x0d
#define meHIL_TEST_MASK    0x0f
#define meHIL_TEST_NOCLASS (meHIL_TEST_ANY-meHIL_TEST_CLASS)
#define meHIL_TEST_NOFIXED (meHIL_TEST_ANY-meHIL_TEST_CLASS)

#define meHIL_TEST_INVERT        0x10
#define meHIL_TEST_MATCH_NONE    0x20
#define meHIL_TEST_MATCH_MULTI   0x40
#define meHIL_TEST_VALID         0x80
#define meHIL_TEST_INVALID       0x00
#define meHIL_TEST_TOKEN         (meHIL_TEST_VALID|meHIL_TEST_INVERT|meHIL_TEST_WORD)
#define meHIL_TEST_SOL           0xfe

#define meHIL_TEST_DEF_GROUP     10

static uint8 *meHilTestNames[meHIL_TEST_NOCLASS]={
    (uint8 *)"space",(uint8 *)"digit",(uint8 *)"xdigit",(uint8 *)"lower",(uint8 *)"upper",
    (uint8 *)"alpha",(uint8 *)"alnum",(uint8 *)"sword",(uint8 *)"word",(uint8 *)"any", 
} ;

#define meHIL_MODECASE  0x01
#define meHIL_MODESTART 0x02
#define meHIL_MODESTTLN 0x04
#define meHIL_MODETOEOL 0x08
#define meHIL_MODEMOVE  0x10

#define HLASOL      0x2000
#define HLASTTLINE  0x4000

#if HILIGHT

static uint8 varTable[11] ;

static void
freeToken(HILNODEPTR root)
{
    uint8 ii, top ;
    
    top = hilListSize(root) ;
    for(ii=0 ; ii<top ; ii++)
        freeToken(root->list[ii]) ;
    meNullFree(root->table) ;
    meNullFree(root->list) ;
    meNullFree(root->close) ;
    meNullFree(root->rtoken) ;
    meNullFree(root->rclose) ;
    free(root) ;
}

static HILNODEPTR
createHilight(uint8 hilno, uint8 *noHils , HILNODEPTR **hTbl)
{
    HILNODEPTR    root ;
    
    if(hilno >= *noHils)
    {
        if((*hTbl = (HILNODEPTR *) 
            meRealloc(*hTbl,(hilno+1)*sizeof(HILNODEPTR))) == NULL)
            return NULL ;
        memset((*hTbl)+(*noHils),0,(hilno+1-(*noHils))*sizeof(HILNODEPTR)) ;
        *noHils = hilno+1 ;
    }
    else
    {
        if((root=(*hTbl)[hilno]) != NULL)
        {
            HILNODEPTR hn ;
            while((hn = (HILNODEPTR) root->rclose) != NULL)
            {
                root->rclose = hn->rclose ;
                meFree(hn) ;
            }
            root->rtoken = NULL ;
            root->close = NULL ;
            freeToken(root) ;
        }
        (*hTbl)[hilno] = NULL ;
    }
    if((root = meMalloc(sizeof(HILNODE))) == NULL)
        return NULL ;
    memset(root,0,sizeof(HILNODE)) ;
    return root ;
}

static void
ListToTable(HILNODEPTR root)
{
    uint8 pos, ii, size ;
    uint8 cc ;
    
    if(root->listSize < 8)
        return ;
    
    root->table = (uint8 *) meMalloc(TableSize) ;
    
    root->table[0] = root->ordSize ;
    pos  = root->ordSize ;
    size = hilListSize(root) ;
    for(; (pos<size) && ((uint8)(root->list[pos]->token[0]) < TableLower) ; pos++)
        ;
    cc = TableLower ;
    for(ii=1 ; ii<TableSize ; ii++,cc++)
    {
        root->table[ii] = pos ;
        for(; (pos<size) && (root->list[pos]->token[0] == cc) ; pos++)
            ;
    }
    root->table[TableSize-1] = size;
}

#define ADDTOKEN_REMOVE   0x10000
#define ADDTOKEN_DUMMY    0x20000
#define ADDTOKEN_OPTIM    0x40000

#define tokenSetType(node,tt) \
((node)->type = (((node)->type & (HLASOL|HLASTTLINE)) | (tt)))

static void
hilNodeOptimize(HILNODEPTR root, int flags) ;

static HILNODEPTR
addTokenNode(HILNODEPTR root, uint8 *token, int flags)
{
    HILNODEPTR    node, nn ;
    uint8 c1=1, ii, jj, top, spec ;
    int len ;
    
    /* Have we run out of room, is so try to optimize */
    if(!(flags & ADDTOKEN_OPTIM) && (hilListSize(root) == 0xfe))
        hilNodeOptimize(root,flags) ;

    if((*token == 0xff) && (*++token != 0xff))
    {
        /* special test */
        ii  = 0 ;
        top = root->ordSize ;
        spec = 1 ;
    }
    else
    {
        ii = root->ordSize ;
        top = hilListSize(root) ;
        spec = 0 ;
    }
    for( ; ii<top ; ii++)
    {
        uint8 c2, *s1, *s2 ;
        node = root->list[ii] ;
        s1 = token ;
        s2 = node->token ;
        while((c1 = *s1) == (c2 = *s2))
        {
            if(c1 == '\0')
            {
                if(flags & ADDTOKEN_DUMMY)
                {
                    /* if adding a dummy, the node already exists as a legit, 
                     * the only thinkg to do is correct the ASTTLINE & ASOL flags,
                     * otherwise leave it alone */
                    node->type &= (~(HLASOL|HLASTTLINE)) | flags ; 
                    return node ;
                }
                /* if removing token or readding, this node must
                 * be reset
                 */
                if(node->rtoken != NULL)
                {
                    meFree(node->rtoken) ;
                    node->rtoken = NULL ;
                }
                if(node->close != NULL)
                {
                    meFree(node->close) ;
                    node->close = NULL ;
                }
                if(node->rclose != NULL)
                {
                    meFree(node->rclose) ;
                    node->rclose = NULL ;
                }
                tokenSetType(node,(flags & ADDTOKEN_REMOVE) ? HLVALID:0) ;
                return node ;
            }
            s1++ ;
            s2++ ;
        }
        if(c1 == '\0')
        {
            if(flags & ADDTOKEN_REMOVE)
                return NULL ;
            break ;
        }
        if(c2 == '\0')
        {
            node->type &= (~(HLASOL|HLASTTLINE)) | flags ; 
            return addTokenNode(node,s1,flags) ;
        }
        if(c2 > c1)
            break ;
    }
    if(flags & ADDTOKEN_REMOVE)
        return NULL ;
    
    /* ii points to position to insert top must now be the table size */
    /* going to be added to this list, check that we have enough room */
    if(((top=hilListSize(root)) == 0xfe) && !(flags & ADDTOKEN_OPTIM))
    {
        mlwrite(MWABORT|MWWAIT,(uint8 *)"Table full, cant add [%s]. Optimise!",token) ;
        return NULL ;
    }
    top++ ;
    len = meStrlen(token) ;
    if(((root->list = (HILNODEPTR *) meRealloc(root->list,top*sizeof(HILNODEPTR))) == NULL) ||
       ((nn = meMalloc(sizeof(HILNODE)+len)) == NULL))
        return NULL ;
    memset(nn,0,sizeof(HILNODE)) ;
    nn->type = ((flags & ADDTOKEN_DUMMY) ?
                (HLVALID|(flags & (HLASOL|HLASTTLINE))):HLASOL|HLASTTLINE) ;
    meStrcpy(nn->token,token) ;
    
    if(c1 == 0)
    {
        int kk, ll ;
        for(jj=0,kk=ii ;  kk<top-1 ; kk++,jj++)
            if(meStrncmp(token,root->list[kk]->token,len))
                break ;
        if((nn->list = (HILNODEPTR *) meMalloc(jj*sizeof(HILNODEPTR))) == NULL)
            return NULL ;
        for(jj=0,ll=ii ;  ll<kk ; ll++,jj++)
        {
            nn->list[jj] = root->list[ll] ;
            meStrcpy(nn->list[jj]->token,nn->list[jj]->token+len) ;
            if((nn->list[jj]->type & (HLASOL|HLSOL))  != (HLASOL|HLSOL))
                nn->type &= ~HLASOL ;
            if((nn->list[jj]->type & (HLASTTLINE|HLSTTLINE))  != (HLASTTLINE|HLSTTLINE))
                nn->type &= ~HLASTTLINE ;
        }
        nn->listSize = jj ;
        root->list[ii++] = nn ;
        for( ; ll<top ; ii++,ll++)
            root->list[ii] = root->list[ll] ;
        jj = 1 - jj ;
    }
    else
    {
        for(jj=top-1 ; jj>ii ; jj--)
            root->list[jj] = root->list[jj-1] ;
        root->list[ii] = nn ;
        jj = 1 ;
    }
    if(spec)
        (root->ordSize) += jj ;
    else
        (root->listSize) += jj ;
    
    if(root->table == NULL)
        ListToTable(root) ;
    else
    {
        if(spec)
            ii = 0 ;
        else
            ii = GetTableSlot((uint8)(*token)) + 1 ;
        for(; ii<TableSize ; ii++)
            (root->table[ii]) += jj ;
    }
    return nn ;
}

static void
hilNodeOptimize(HILNODEPTR root, int flags)
{
    int ii, sz, no=0, cc=0, jj=0 ;
    uint8 tok[2] ;
    
    ii = root->ordSize ;
    sz = ii+root->listSize ;
    while(ii < sz)
    {
        if(root->list[ii]->token[0] != cc)
        {
            if(jj > no)
            {
                tok[0] = cc ;
                no = jj ;
            }
            cc = root->list[ii]->token[0] ;
            jj = 0 ;
        }
        else
            jj++ ;
        ii++ ;
    }
    if(no)
    {
        tok[1] = '\0' ;
        addTokenNode(root,tok,flags|ADDTOKEN_DUMMY|ADDTOKEN_OPTIM) ;
    }
}


static uint8 *
meHiltItemCompile(uint8 *dest, uint8 **token,
                  uint8 noGroups, uint8 changeCase)
{
    uint8 cc, *ss=*token ;
    if((cc=*ss++) == '[')
    {
        uint8 *s1, *dd ;
        uint8 tt, ii=0 ;
        
        *dest++ = 0xff ;
        tt = meHIL_TEST_VALID ;
        if(*ss == '^')
        {
            tt |= meHIL_TEST_INVERT ;
            ss++ ;
        }
        if(((cc = *ss++) == '[') && (*ss == ':'))
        {
            s1 = ss ;
            while(*++s1 != '\0')
            {
                if(*s1 == ']')
                {
                    if((s1[1] == ']') && (*--s1 == ':'))
                    {
                        *s1 = '\0' ;
                        for(ii=0 ; ii<meHIL_TEST_NOCLASS ; ii++)
                            if(!meStrcmp(ss+1,meHilTestNames[ii]))
                                break ;
                        if(ii == meHIL_TEST_NOCLASS)
                        {
                            /* this is an unknown char class return an error */
                            mlwrite(MWABORT|MWWAIT,(uint8 *)"[Unknown hilight class %s]",ss+1);
                            *s1 = ':' ;
                            return NULL ;
                        }
                        *s1 = ':' ;
                        ss = s1+3 ;
                        ii += meHIL_TEST_SPACE ;
                    }
                    break ;
                }
            }
        }
        *dest++ = ((ii == 0) ? meHIL_TEST_CLASS:ii)|tt ;
        *dest++ = meHIL_TEST_DEF_GROUP ;
        if(ii == 0)
        {
            dd = dest++ ;
            do {
                if(cc == '\0')
                {
                    mlwrite(MWABORT|MWWAIT,(uint8 *)"[Open []");
                    return NULL ;
                }
                *dest++ = cc ;
            } while((cc=*ss++) != ']') ;
            *dd = (uint8) (dest-dd-1) ;
        }
    }
    else if(cc == '\\')
    {
        cc = *ss++ ;
        switch(cc)
        {
        case 'S':
            *dest++ = 0xff ;
            switch(*ss++)
            {
            case 'w':
                *dest++ = meHIL_TEST_VALID|meHIL_TEST_INVERT|meHIL_TEST_WORD ;
                break ;
            case ' ':
            case '-':
                *dest++ = meHIL_TEST_VALID|meHIL_TEST_INVERT|meHIL_TEST_SPACE ;
                break ;
            default:
                mlwrite(MWABORT|MWWAIT,(uint8 *)"[Unsupported \\SC char class]");
                return NULL ;
            }
            *dest++ = meHIL_TEST_DEF_GROUP ;
            break ;
        case 'W':
            *dest++ = 0xff ;
            *dest++ = meHIL_TEST_VALID|meHIL_TEST_INVERT|meHIL_TEST_WORD ;
            *dest++ = meHIL_TEST_DEF_GROUP ;
            break ;
        case 'a':   *dest++ = 0x07; break;
        case 'd':   *dest++ = 0x7f; break;
        case 'e':   *dest++ = 0x1b; break;
        case 'f':   *dest++ = 0x0c; break;
        case 'n':   *dest++ = 0x0a; break;
        case 'r':   *dest++ = 0x0d; break;
        case 's':
            *dest++ = 0xff ;
            switch(*ss++)
            {
            case 'w':
                *dest++ = meHIL_TEST_VALID|meHIL_TEST_WORD ;
                break ;
            case ' ':
            case '-':
                *dest++ = meHIL_TEST_VALID|meHIL_TEST_SPACE ;
                break ;
            default:
                mlwrite(MWABORT|MWWAIT,(uint8 *)"[Unsupported \\sC char class]");
                return NULL ;
            }
            *dest++ = meHIL_TEST_DEF_GROUP ;
            break ;
        case 't':   *dest++ = 0x09; break;
        case 'v':   *dest++ = 0x0b; break;
        case 'w':
            *dest++ = 0xff ;
            *dest++ = meHIL_TEST_VALID|meHIL_TEST_WORD ;
            *dest++ = meHIL_TEST_DEF_GROUP ;
            break ;
        case 'x':
            cc = *ss ;
            if(isXDigit(cc))
            {
                register uint8 c1=*++ss ;
                
                if(isXDigit(c1))
                {
                    cc = (hexToNum(cc) << 4) | hexToNum(c1) ;
                    ss++ ;
                }
                else
                    cc = hexToNum(cc) ;
                if(cc == 0)
                {
                    mlwrite(MWABORT|MWWAIT,(uint8 *)"[Invalid \\x character]");
                    return NULL ;
                }
                if(changeCase)
                    cc = toLower(cc) ;
                if(cc == 0xff)
                    *dest++ = 0xff ;
                *dest++ = cc ;
            }
            break;
        case '|':
        case '`':
        case '\'':
        case '<':
        case '>':
        case 'b':
        case 'B':
            mlwrite(MWABORT|MWWAIT,(uint8 *)"[Hilight does not support \\%c]",cc);
            return NULL ;
        default:
            if(isDigit(cc))
            {
                if(((cc-='0') == 0) || (cc > noGroups))
                {
                    mlwrite(MWABORT|MWWAIT,(uint8 *)"[Back reference out of range]");
                    return NULL ;
                }
                *dest++ = 0xff ;
                *dest++ = meHIL_TEST_VALID|meHIL_TEST_BACKREF ;
                *dest++ = meHIL_TEST_DEF_GROUP ;
                *dest++ = cc ;
            }
            else
            {
                if(changeCase)
                    cc = toLower(cc) ;
                *dest++ = cc;
            }
        }
    }
    else if(cc == '.')
    {
        *dest++ = 0xff ;
        *dest++ = meHIL_TEST_VALID|meHIL_TEST_ANY ;
        *dest++ = meHIL_TEST_DEF_GROUP ;
    }
    else
    {
        if(changeCase)
            cc = toLower(cc) ;
        *dest++ = cc;
    }
    *token = ss ;
    return dest ;
}

/* compiles the given token into a string form compatible with the main
 * hilight search engine.
 * The retValues are:
 * 
 *     retValues[0] : Token length in items
 *     retValues[1] : Token hilight start offset.
 *     retValues[2] : Token hilight end offset.
 *     retValues[3] : Number of groups.
 * 
 * Returns a pointer to the start of the last item or NULL on failure.
 */
static uint8 *
meHiltStringCompile(uint8 *dest, uint8 *token, uint8 changeCase,
                    int *retValues)
{
    uint8 cc, *dd, *lastDest ;
    uint8 nextGroupNo=0, group=0 ;
    int startOffset=0, endOffset=-1, len=0 ;
    
    while((cc=*token) != '\0')
    {
        if(cc == '\\')
        {
            switch(token[1])
            {
            case '(':
                if(nextGroupNo >= 9)
                {
                    mlwrite(MWABORT|MWWAIT,(uint8 *)"[Too many groups]");
                    return NULL ;
                }
                group = 1 ;
                token += 2 ;
                if((*token == '\0') || ((*token == '\\') && (token[1] == ')')))
                {
                    mlwrite(MWABORT|MWWAIT,(uint8 *)"[Bad group \\(.\\)]");
                    return NULL ;
                }
                break ;
            case '|':
                mlwrite(MWABORT|MWWAIT,(uint8 *)"[Hilight does not support \\|]");
                return NULL ;
            case ')':
                mlwrite(MWABORT|MWWAIT,(uint8 *)"[Unmatched group \\)]");
                return NULL ;
            case '{':
                startOffset = len ;
                token += 2 ;
                break ;
            case '}':
                if(endOffset < 0)
                    endOffset = len ;
                token += 2 ;
                break ;
            }
            if((cc = *token) == '\0')
                break ;
        }
        if(cc == '$')
        {
            if(*++token != '\0')
            {
                mlwrite(MWABORT|MWWAIT,(uint8 *)"[End-of-line $ must be the last char]");
                return NULL ;
            }
            if(endOffset < 0)
                endOffset = len ;
            dd = dest ;
            *dd++ = 0xff ;
            *dd++ = meHIL_TEST_VALID|meHIL_TEST_EOL ;
            *dd++ = meHIL_TEST_DEF_GROUP ;
        }
        else if((dd = meHiltItemCompile(dest,&token,nextGroupNo,changeCase)) == NULL)
            return NULL ;
        if(group)
        {
            if((*token++ != '\\') || (*token++ != ')'))
            {
                mlwrite(MWABORT|MWWAIT,(uint8 *)"[A group \\(.\\) must enclose a single char]");
                return NULL ;
            }
            if(dd <= dest+2)
            {
                /* groups must encase a test (a [a-z] or . etc) if not the
                 * turn it into a test */
                dest[3] = dest[0] ;
                dest[1] = (meHIL_TEST_VALID|meHIL_TEST_SINGLE) ;
                dest[0] = 0xff ;
                dd = dest+4 ;
            }
            dest[2] = ++nextGroupNo ;
            group = 0 ;
        }
        if(((cc=token[0]) == '?') || (cc == '*') || (cc == '+'))
        {
            token++ ;
            if(dd <= dest+2)
            {
                /* closures can only be done on tests (a [a-z]* or .+ etc). If not then
                 * turn it into a test */
                dest[3] = dest[0] ;
                dest[2] = meHIL_TEST_DEF_GROUP ;
                dest[1] = (meHIL_TEST_VALID|meHIL_TEST_SINGLE) ;
                dest[0] = 0xff ;
                dd = dest+4 ;
            }
            if(cc != '+')
                dest[1] |= meHIL_TEST_MATCH_NONE ;
            if(cc != '?')
                dest[1] |= meHIL_TEST_MATCH_MULTI ;
        }
        lastDest = dest ;
        dest = dd ;
        len++ ;
    }
    /* can't have a zero length search string */
    if(len == 0)
    {
        mlwrite(MWABORT|MWWAIT,(uint8 *)"[Invalid zero length token]");
        return NULL ;
    }
    *dest = '\0' ;
    if(endOffset < 0)
        endOffset = len ;
    retValues[0] = len ;
    retValues[1] = startOffset ;
    retValues[2] = endOffset ;
    retValues[3] = nextGroupNo ;
    
    return lastDest ;
}

static HILNODEPTR
meHiltTokenAddSearchString(HILNODEPTR root, HILNODEPTR node, uint8 *token, int flags)
{
    uint8 buff[MAXBUF], *dd, *ss, *lastItem ;
    int retValues[4], l1, l2, len ;
    
    dd = buff ;
    ss = token ;
    /* must remove the start of line '^' as its a zero length item 
     * implemented in flags */
    if(*ss == '^')
    {
        if(node == NULL)
            flags |= HLSOL ;
        else
        {
            /* dirty special case */
            *dd++ = 0xff ;
            *dd++ = meHIL_TEST_SOL ;
        }
        ss++ ;
    }
    if((lastItem=meHiltStringCompile(dd,ss,(uint8) ((root->type & HFCASE) != 0),
                                     retValues)) == NULL)
        return NULL ;
    len = retValues[0] ;
    l1 = retValues[1] ;
    l2 = retValues[2] ;
    
    l2 = len - l2 ;
    len -= l1 + l2 ;
    
    if(node == NULL)
    {
        uint8  sttTst, endTst ;
        ss = buff ;
        /* if the token starts with "^[[:space:]]*\\{" then we can
         * optimize this to HLSTTLINE */
        if(l1 && (len || l2) && (flags & HLSOL) && (ss[0] == 0xff) &&
           (ss[1] == (meHIL_TEST_VALID|meHIL_TEST_MATCH_NONE|meHIL_TEST_MATCH_MULTI|meHIL_TEST_SPACE)))
        {
            flags &= ~HLSOL ;
            flags |= HLSTTLINE ;
            l1-- ;
            ss += 3 ;
        }
        /* if the token ends with ".*\\}" then we can
         * optimize this to HLENDLINE */
        if(!l2 && (len || l1) && (lastItem[0] == 0xff) &&
           (lastItem[1] == (meHIL_TEST_VALID|meHIL_TEST_MATCH_NONE|meHIL_TEST_MATCH_MULTI|meHIL_TEST_ANY)))
        {
            flags |= HLENDLINE ;
            len-- ;
            lastItem[0] = '\0' ;
        }
        
        /* setup the start and end Test flag */
        if(flags & HLTOKEN)
        {
            sttTst = meHIL_TEST_TOKEN ;
            endTst = meHIL_TEST_TOKEN ;
        }
        else
        {
            if(l1 && (len || l2) && (ss[0] == 0xff) &&
               ((ss[1] & ~(meHIL_TEST_MASK|meHIL_TEST_INVERT)) == meHIL_TEST_VALID) &&
               ((ss[1] & meHIL_TEST_MASK) > meHIL_TEST_CLASS))
            {
                sttTst = ss[1] ;
                l1-- ;
                ss += 3 ;
            }
            else
                sttTst = meHIL_TEST_INVALID ;
            if(l2 && (len || l1) && (lastItem[0] == 0xff) &&
               ((lastItem[1] & ~(meHIL_TEST_MASK|meHIL_TEST_INVERT)) == meHIL_TEST_VALID) &&
               ((lastItem[1] & meHIL_TEST_MASK) > meHIL_TEST_CLASS))
            {
                endTst = lastItem[1] ;
                l2-- ;
                lastItem[0] = '\0' ;
            }
            else
                endTst = meHIL_TEST_INVALID ;
        }
        /* if the token starts at the begining of the line add the
         * optimizing AllSOL flag */
        if(flags & HLSOL)
            flags |= HLASOL|HLASTTLINE ;
        else if(flags & HLSTTLINE)
            flags |= HLASTTLINE ;
        /* must add all sections of the token as dummy nodes */
        dd = ss ;
        while(dd <= lastItem)
        {
            if((*dd++ == 0xff) && (*dd++ != 0xff) && (dd != ss+2)) 
            {
                dd[-2] = '\0' ;
                if(addTokenNode(root,ss,flags|ADDTOKEN_DUMMY) == NULL)
                    return NULL ;
                dd[-2] = 0xff ;
                dd++ ;
            }
        }
        if(((node = addTokenNode(root,ss,flags)) == NULL) || (flags & ADDTOKEN_REMOVE))
            return NULL ;
        
        if(flags & (HLBRANCH|HLBRACKET))
            flags &= ~HLCONTIN ;
        tokenSetType(node,flags) ;
        node->tknSttTst = sttTst ;
        node->tknEndTst = endTst ;
        node->tknSttOff = l1 ;
        node->tknEndOff = l2 ;
    }
    else
    {
        if((dd = meStrdup(buff)) == NULL)
            return NULL ;
        
        node->close = dd ;
        node->clsSttOff = l1 ;
        node->clsEndOff = l2 ;
    }
    /* store the len temporarily into scheme for len tests on the replace strings */
    node->scheme = len ;
    return node ;
}

static HILNODEPTR
meHiltTokenAddReplaceString(HILNODEPTR root, HILNODEPTR node, uint8 *token, int mainRepl)
{
    uint8 buff[MAXBUF], *dd, *ss, cc ;
    int len ;
    
    ss = token ;
    dd = buff ;
    len = 0 ;
    while((cc=*ss++) != '\0')
    {
        if(cc == '\\')
        {
            cc = *ss++ ;
            if((cc > '0') && (cc <= '9'))
            {
                *dd++ = 0xff ;
                cc -= '0' ;
            }
        }
        else if(cc == 0xff)
        {
            *dd++ = 0xff ;
            cc = *ss++ ;
        }
        *dd++ = cc ;
        len++ ;
    }
    *dd = 0 ;
    
    if((dd = meStrdup(buff)) == NULL)
        return NULL ;
    
    if(mainRepl)
        node->rtoken = dd ;
    else
        node->rclose = dd ;
    
    if(len != node->scheme)
    {
        node->type |= HLRPLCDIFF ;
        root->type |= HFRPLCDIFF ;
    }
    return node ;
}


int
hilight(int f, int n)
{
    HILNODEPTR root, node ;
    uint8  buf[MAXBUF] ;
    uint16 type ;
    uint8  hilno ;
    int ii ;
    
    if(n == 0)
    {
        if((mlreply((uint8 *)"Enter hi-light number",0,0,buf,MAXBUF-1) != TRUE) ||
           ((hilno = (uint8) meAtoi(buf)) == 0) || 
           (mlreply((uint8 *)"Flags",0,0,buf,MAXBUF-1) != TRUE))
            return ABORT ;
        type = (uint16) meAtoi(buf) ;
        
        if((root = createHilight(hilno,&noHilights,&hilights)) == NULL)
            return FALSE ;
        
        /* force a complete update next time */ 
        sgarbf = TRUE ;
        root->type = type ;
        if(type & HFLOOKB)
        {
            if(mlreply((uint8 *)"lines",0,0,buf,MAXBUF) != TRUE)
                return ABORT ;
            root->ignore = (uint8) meAtoi(buf) ;
        }
        if((ii=mlreply((uint8 *)"Scheme",0,0,buf,MAXBUF)) == ABORT)
            return ABORT ;
        if(ii == FALSE)
            ii = globScheme ;
        else if((ii=convertUserScheme(meAtoi(buf), -1)) < 0)
            return FALSE ;
        root->scheme = (meSCHEME) ii ;
        if((ii=mlreply((uint8 *)"Trunc scheme",0,0,buf,MAXBUF)) == ABORT)
            return ABORT ;
        if(ii == FALSE)
            ii = trncScheme ;
        else if((ii=convertUserScheme(meAtoi(buf), -1)) < 0)
            return FALSE ;
        root->close = (uint8 *) ii ;
        hilights[hilno] = root ;
        return TRUE ;
    }
    n = (n < 0) ? ADDTOKEN_REMOVE:0 ;
    
    if((mlreply((uint8 *)"Hi number",0,0,buf,MAXBUF-1) != TRUE) ||
       ((hilno = (uint8) meAtoi(buf)) == 0) ||
       (hilno >= noHilights) ||
       ((root  = hilights[hilno]) == NULL) ||
       (mlreply((uint8 *)"Type",0,0,buf,MAXBUF-1) != TRUE))
        return FALSE ;
    type = (uint16) meAtoi(buf) ;
    if(type & HLCOLUMN)
    {
        int fmCol, toCol ;
        
        if((mlreply((uint8 *)"From",0,0,buf,MAXBUF-1) != TRUE) ||
           ((fmCol = (uint8) meAtoi(buf)) < 0) ||
           (!(n & ADDTOKEN_REMOVE) &&
            ((mlreply((uint8 *)"To",0,0,buf,MAXBUF-1) != TRUE) ||
             ((toCol = (uint8) meAtoi(buf)) < fmCol))))
            return FALSE ;
        while(((node = (HILNODEPTR) root->rclose) != NULL) && (((int) node->close) < fmCol))
            root = node ;
        if((node != NULL) && ((int) node->close) == fmCol)
        {
            if(n & ADDTOKEN_REMOVE)
            {
                root->rclose = node->rclose ;
                meFree(node) ;
                return TRUE ;
            }
        }
        else if(n & ADDTOKEN_REMOVE)
            return TRUE ;
        else if((node = meMalloc(sizeof(HILNODE))) == NULL)
            return ABORT ;
        else
        {
            memset(node,0,sizeof(HILNODE)) ;
            node->type = HLCOLUMN ;
            node->close = (uint8 *) fmCol ;
            node->rclose = root->rclose ;
            root->rclose = (uint8 *) node ;
        }
        node->rtoken = (uint8 *) toCol ;
        goto get_scheme ;
    }
    
    if(mlreply((uint8 *)"Token",0,0,buf,MAXBUF-1) != TRUE)
        return FALSE ;
    
    /* add the token */
    node = meHiltTokenAddSearchString(root,NULL,(uint8 *) buf,(type|n)) ;
    if(n & ADDTOKEN_REMOVE)
        return TRUE ;
    if(node == NULL)
        return FALSE ;
    type = node->type ;
    
    if(!(type & HLVALID))
    {
        if(((type & HLREPLACE) && 
            ((mlreply((uint8 *)"Replace",0,0,buf,MAXBUF-1) != TRUE) ||
             (meHiltTokenAddReplaceString(root,node,(uint8 *)buf,1) == NULL))) ||
           ((type & HLBRACKET) &&
            ((mlreply((uint8 *)"Close",0,0,buf,MAXBUF-1) != TRUE) ||
             (meHiltTokenAddSearchString(root,node,(uint8 *) buf,0) == NULL) ||
             ((type & HLREPLACE) && 
              ((mlreply((uint8 *)"Replace",0,0,buf,MAXBUF-1) != TRUE) ||
               (meHiltTokenAddReplaceString(root,node,(uint8 *)buf,0) == NULL))) ||
             (mlreply((uint8 *)"Ignore",0,0,buf,MAXBUF-1) != TRUE) ||
             ((node->ignore = buf[0]) > 255))) ||
           ((type & HLCONTIN) &&
            ((mlreply((uint8 *)"Continue",0,0,buf,MAXBUF-1) != TRUE) ||
             ((node->close = (uint8 *) meStrdup(buf)) == NULL) ||
             ((type & HLREPLACE) && 
              ((mlreply((uint8 *)"Replace",0,0,buf,MAXBUF-1) != TRUE) ||
               ((node->rclose = (uint8 *) meStrdup(buf)) == NULL))))) ||
           ((type & HLBRANCH) &&
            ((mlreply((uint8 *)"hilno",0,0,buf,MAXBUF-1) != TRUE) ||
             ((node->ignore = (uint8) meAtoi(buf)) > 255))))
            return FALSE;
get_scheme:        
        /* Get the colour index. This uses the root hilight 
         * colour if not specified. */
        ii = mlreply((uint8 *)"Scheme",0,0,buf,MAXBUF-1);
        if (ii == ABORT)
            return (FALSE);
        
        if (ii == TRUE) 
        {
            if ((ii=convertUserScheme(meAtoi(buf),-1)) < 0)
                return (FALSE);
            node->scheme = ii ;
        }
        else
            node->scheme = root->scheme ;
    }
    return TRUE ;
}


#define findTokenCharTest(ret,lastChar,srcText,tokTest,testStr)              \
do {                                                                         \
    uint8 *__ts, ftctcc ;                                                    \
    if((ftctcc=*srcText++) == '\0') ftctcc=meNLCHAR ;                        \
                                                                             \
    switch(tokTest & meHIL_TEST_MASK)                                        \
    {                                                                        \
    case meHIL_TEST_BACKREF:                                                 \
	__ts = testStr ;                                                     \
	ret = (varTable[*testStr++] == ftctcc) ;                             \
	break ;                                                              \
    case meHIL_TEST_SINGLE:                                                  \
	__ts = testStr ;                                                     \
	ret = (*testStr++ == ftctcc) ;                                       \
	break ;                                                              \
    case meHIL_TEST_CLASS:                                                   \
        {                                                                    \
	    uint8 rc, nrc ;                                                  \
                                                                             \
	    __ts = testStr ;                                                 \
	    for(ret=*testStr++ ; ret>0 ; ret--)                              \
            {                                                                \
		rc=*testStr++ ;                                              \
		nrc = *testStr ;                                             \
		if((nrc == '-') && (ret > 1))                                \
                {                                                            \
		    /* range */                                              \
		    testStr++ ;                                              \
		    nrc = *testStr++ ;                                       \
		    ret -= 2 ;                                               \
		    if((ftctcc >= rc) && (ftctcc <= nrc))                    \
                    {                                                        \
			testStr += ret - 1 ;                                 \
			break ;                                              \
		    }                                                        \
		}                                                            \
		/* single char compare */                                    \
		else if(ftctcc == rc)                                        \
                {                                                            \
		    testStr += ret - 1 ;                                     \
		    break ;                                                  \
		}                                                            \
	    }                                                                \
	    break ;                                                          \
	}                                                                    \
    case meHIL_TEST_EOL:                                                     \
	ret = (ftctcc == meNLCHAR) ;                                         \
	break ;                                                              \
    case meHIL_TEST_SPACE:                                                   \
	ret = isSpace(ftctcc) ;                                              \
	break ;                                                              \
    case meHIL_TEST_DIGIT:                                                   \
	ret = isDigit(ftctcc) ;                                              \
	break ;                                                              \
    case meHIL_TEST_XDIGIT:                                                  \
	ret = isXDigit(ftctcc) ;                                             \
	break ;                                                              \
    case meHIL_TEST_LOWER:                                                   \
	ret = isLower(ftctcc) ;                                              \
	break ;                                                              \
    case meHIL_TEST_UPPER:                                                   \
	ret = isUpper(ftctcc) ;                                              \
	break ;                                                              \
    case meHIL_TEST_ALPHA:                                                   \
	ret = isAlpha(ftctcc) ;                                              \
	break ;                                                              \
    case meHIL_TEST_ALNUM:                                                   \
	ret = isAlphaNum(ftctcc) ;                                           \
	break ;                                                              \
    case meHIL_TEST_SWORD:                                                   \
	ret = isSpllWord(ftctcc) ;                                           \
	break ;                                                              \
    case meHIL_TEST_WORD:                                                    \
	ret = isWord(ftctcc) ;                                               \
	break ;                                                              \
    case meHIL_TEST_ANY:                                                     \
	ret = 1 ;                                                            \
	break ;                                                              \
    }                                                                        \
    if(tokTest & meHIL_TEST_INVERT)                                          \
	ret = !ret ;                                                         \
    if(!ret)                                                                 \
    {                                                                        \
	if(tokTest & meHIL_TEST_MATCH_NONE)                                  \
        {                                                                    \
	    ret = 1 ;                                                        \
	    srcText-- ;                                                      \
	}                                                                    \
	tokTest = 0 ;                                                        \
    }                                                                        \
    else if((lastChar=ftctcc) == meNLCHAR)                                   \
	tokTest = 0 ;                                                        \
    else if(tokTest & meHIL_TEST_MATCH_MULTI)                                \
    {                                                                        \
	if((tokTest & meHIL_TEST_MASK) <= meHIL_TEST_CLASS)                  \
	    testStr = __ts ;                                                 \
	tokTest |= meHIL_TEST_MATCH_NONE ;                                   \
    }                                                                        \
} while(tokTest & meHIL_TEST_MATCH_MULTI)

static int
__findTokenSingleCharTest(uint8 cc, uint8 tokTest)
{
    int ret ;
    switch(tokTest & meHIL_TEST_MASK)
    {
    case meHIL_TEST_EOL:
        ret = (cc == '\0') ;
        break ;
    case meHIL_TEST_SPACE:
        ret = isSpace(cc) ;
        break ;
    case meHIL_TEST_DIGIT:
        ret = isDigit(cc) ;
        break ;
    case meHIL_TEST_XDIGIT:
        ret = isXDigit(cc) ;
        break ;
    case meHIL_TEST_LOWER:
        ret = isLower(cc) ;
        break ;
    case meHIL_TEST_UPPER:
        ret = isUpper(cc) ;
        break ;
    case meHIL_TEST_ALPHA:
        ret = isAlpha(cc) ;
        break ;
    case meHIL_TEST_ALNUM:
        ret = isAlphaNum(cc) ;
        break ;
    case meHIL_TEST_SWORD:
        ret = isSpllWord(cc) ;
        break ;
    case meHIL_TEST_WORD:
        ret = isWord(cc) ;
        break ;
    case meHIL_TEST_ANY:
        ret = 1 ;
        break ;
    }
    if(tokTest & meHIL_TEST_INVERT)
        ret = !ret ;
    return ret ;
}    

#define findTokenSingleCharTest(cc,tokTest)                                  \
((tokTest == meHIL_TEST_INVALID) || __findTokenSingleCharTest(cc,tokTest))

static HILNODEPTR
findToken(HILNODEPTR root, uint8 *text, uint8 mode, 
          uint8 lastChar, uint16 *len)
{
    HILNODEPTR     nn, node ;
    uint8  cc, *s1, *s2;
    int status;
    int hi, low, mid ;
    uint16 type ;
    
    if(*text == '\0')
    {
        /* one test is needed here for the one char token "\n"
         * This is either the \n char or \s11 white space char
         */
        low = root->listSize ;
        hi = root->ordSize ;
        while((--low >= 0) && ((nn=root->list[hi++]),((cc=nn->token[0]) <= meNLCHAR)))
        {
            if((cc == meNLCHAR) && (nn->token[1] == 0) &&
               !((type=nn->type) & HLVALID) && (nn->tknEndTst == meHIL_TEST_INVALID) &&
               (!(mode & meHIL_MODETOEOL) || (type & HLBRINEOL)) &&
               (!(type & HLSTTLINE) || (mode & meHIL_MODESTTLN)) &&
               (!(type & HLSOL) || (mode & meHIL_MODESTART)) &&
               findTokenSingleCharTest(lastChar,nn->tknSttTst))
            {
                *len = (nn->tknEndOff) ? 1:0 ;
                return nn ;
            }
        }
        if((hi = root->ordSize) > 0)
        {
            s1 = text ;
            while(--hi >= 0)
            {
                nn = root->list[hi] ;
                if(!((type=nn->type) & HLVALID) && (nn->tknEndTst == meHIL_TEST_INVALID) &&
                   (((mode & meHIL_MODETOEOL) == 0) || (type & HLBRINEOL)) &&
                   (!(type & HLSTTLINE) || (mode & meHIL_MODESTTLN)) &&
                   (!(type & HLSOL) || (mode & meHIL_MODESTART)) &&
                   findTokenSingleCharTest(lastChar,nn->tknSttTst))
                {
                    uint8 rr, tt, lstc=1 ;
                    s2 = nn->token ;
                    tt = *s2++ ;
                    s2++ ;
                    findTokenCharTest(rr,lstc,s1,tt,s2) ;
                    if(rr &&
                       ((*s2 == '\0') || 
                        ((lstc == 1) && (*s2 == '\n') && (s2[1] == '\0'))))
                    {
                        varTable[nn->token[1]] = '\0' ;
                        *len = (nn->tknEndOff) ? 1:0 ;
                        return nn ;
                    }
                }
            }
        }
        return NULL ;
    }
    if(root->table == NULL)
    {
        low = root->ordSize ;
        hi  = low+root->listSize ;
    }
    else
    {
        cc = *text ;
        if(mode & meHIL_MODECASE)
            cc = toLower(cc) ;
        low = GetTableSlot(cc) ;
        hi  = root->table[low+1] ;
        low = root->table[low] ;
    }
    if(low != hi)
    {
        hi-- ;
        if(mode & meHIL_MODECASE)
        {
            do
            {
                mid = (low + hi) >> 1;		/* Get mid value. */
                s1 = text ;
                nn = root->list[mid] ;
                s2 = (uint8 *) nn->token ;
                do {
                    cc = *s1++ ;
                    if((status=((int) toLower(cc)) - ((int) *s2++)) != 0)
                        break ;
                } while(*s2 != '\0') ;
                if(status == 0)
                    break ;
                else if (status < 0)
                {
                    if((status == '\0' - meNLCHAR) && (cc == '\0') && (*s2 == '\0') &&
                       (nn->tknEndTst == meHIL_TEST_INVALID))
                    {
                        if(!nn->tknEndOff)
                            s1-- ;
                        break ;
                    }
                    hi  = mid-1 ;		/* Discard bottom half */
                }
                else
                    low = mid+1 ;		/* Discard top half */
            } while(low <= hi) ;		/* Until converges */
        }
        else
        {
            do
            {
                mid = (low + hi) >> 1;		/* Get mid value. */
                s1 = text ;
                nn = root->list[mid] ;
                s2 = (uint8 *) nn->token ;
                do {
                    if((status=((int) (cc=*s1++)) - ((int) *s2++)) != 0)
                        break ;
                } while(*s2 != '\0') ;
                if(status == 0)
                    break ;
                else if (status < 0)
                {
                    if((status == '\0' - meNLCHAR) && (cc == '\0') && (*s2 == '\0') &&
                       (nn->tknEndTst == meHIL_TEST_INVALID))
                    {
                        if(!nn->tknEndOff)
                            s1-- ;
                        break ;
                    }
                    hi  = mid-1 ;		/* Discard bottom half */
                }
                else
                    low = mid+1 ;		/* Discard top half */
            } while(low <= hi) ;		/* Until converges */
        }
        if(low <= hi)
        {
            type = nn->type ;
            if((cc != '\0') && (hilListSize(nn) > 0) &&
               (!(type & HLASTTLINE) || (mode & meHIL_MODESTTLN)) &&
               (!(type & HLASOL) || (mode & meHIL_MODESTART)) &&
               ((node = findToken(nn,s1,mode,lastChar,len)) != NULL))
            {
                *len += s1-text ;
                return node ;
            }
            if(!(type & HLVALID) && 
               (!(mode & meHIL_MODETOEOL) || (type & HLBRINEOL)) &&
               (!(type & HLSTTLINE) || (mode & meHIL_MODESTTLN)) &&
               (!(type & HLSOL) || (mode & meHIL_MODESTART)) &&
               findTokenSingleCharTest(lastChar,nn->tknSttTst) &&
               findTokenSingleCharTest(*s1,nn->tknEndTst))
            {
                *len = s1-text ;
                return nn ;
            }
        }
    }
    if((hi = root->ordSize) > 0)
    {
        /* note that findTokenCharTest may not set lstc even if it does 
         * match as the ? & * may match 0 chars - so theres no last!
         * Therefore initialize to a sensible value */
        uint8 dd, lstc=1 ;
        mid = 0 ;
        
        for( ; mid < hi ; mid++)
        {
            nn = root->list[mid] ;
            s2 = nn->token ;
            dd = *s2++ ;
            s2++ ;
            s1 = text ;
            findTokenCharTest(cc,lstc,s1,dd,s2) ;
            if(cc)
            {
                if(lstc != meNLCHAR)
                {
                    if(mode & meHIL_MODECASE)
                    {
                        while((dd=*s2++) != '\0')
                        {
                            cc = *s1++ ;
                            if((toLower(cc)) != dd)
                                break ;
                        }
                    }
                    else
                    {
                        while((dd=*s2++) != '\0')
                            if((cc=*s1++) != dd)
                                break ;
                    }
                    if((dd == meNLCHAR) && (cc == '\0') &&
                       (nn->tknEndTst == meHIL_TEST_INVALID))
                    {
                        if(!nn->tknEndOff)
                            s1-- ;
                        dd = *s2 ;
                    }
                }
                else
                {
                    if(!nn->tknEndOff)
                        s1-- ;
                    dd = (nn->tknEndTst == meHIL_TEST_INVALID) ? *s2:0 ;
                    cc = '\0' ;
                }
                if(dd == '\0')
                {
                    varTable[nn->token[1]] = lstc ;
                    type = nn->type ;
                    if((cc != '\0') && (hilListSize(nn) > 0) &&
                       (!(type & HLASTTLINE) || (mode & meHIL_MODESTTLN)) &&
                       (!(type & HLASOL) || (mode & meHIL_MODESTART)) &&
                       ((node = findToken(nn,s1,mode,lastChar,len)) != NULL))
                    {
                        *len += s1-text ;
                        return node ;
                    }
                    if(!(type & HLVALID) && 
                       (!(mode & meHIL_MODETOEOL) || (type & HLBRINEOL)) &&
                       (!(type & HLSTTLINE) || (mode & meHIL_MODESTTLN)) &&
                       (!(type & HLSOL) || (mode & meHIL_MODESTART)) &&
                       findTokenSingleCharTest(lastChar,nn->tknSttTst) &&
                       findTokenSingleCharTest(*s1,nn->tknEndTst))
                    {
                        *len = s1-text ;
                        return nn ;
                    }
                }
            }
        }
    }
    return NULL ;
}

#define __hilCopyChar(dstPos,cc)                                             \
{                                                                            \
    /* the largest character size is a tab which is user definable */        \
    if(dstPos >= disLineSize)                                                \
    {                                                                        \
        disLineSize += 512 ;                                                 \
        disLineBuff = realloc(disLineBuff,disLineSize+32) ;                  \
    }                                                                        \
    if(isDisplayable(cc))                                                    \
    {                                                                        \
        if(cc == ' ')                                                        \
            disLineBuff[dstPos++] = displaySpace ;                           \
        else if(cc == TAB)                                                   \
            disLineBuff[dstPos++] = displayTab ;                             \
        else                                                                 \
            disLineBuff[dstPos++] = cc ;                                     \
    }                                                                        \
    else if(cc == TAB)                                                       \
    {                                                                        \
        int ii=get_tab_pos(dstPos) ;                                         \
        disLineBuff[dstPos++] = displayTab ;                                 \
        while(--ii >= 0)                                                     \
            disLineBuff[dstPos++] = ' ' ;                                    \
    }                                                                        \
    else if(cc < 0x20)                                                       \
    {                                                                        \
        disLineBuff[dstPos++] = '^' ;                                        \
        disLineBuff[dstPos++] = cc ^ 0x40 ;                                  \
    }                                                                        \
    else                                                                     \
    {                                                                        \
        /* Its a nasty character */                                          \
        disLineBuff[dstPos++] = '\\' ;                                       \
        disLineBuff[dstPos++] = 'x' ;                                        \
        disLineBuff[dstPos++] = hexdigits[cc/0x10] ;                         \
        disLineBuff[dstPos++] = hexdigits[cc%0x10] ;                         \
    }                                                                        \
}

/*
 * hilSchemeChange.
 * This function manages the scheme changes. Each change in scheme results
 * in a call. The changes of scheme are added to the "hd->blkp" structure.
 * In addition the last block node is retained in "hd->hnode" which is the
 * current hilighting scheme. The selection hilighting uses this node to 
 * switch the selection schemes on an off. The correct state is mainated by
 * this function.
 */
static void
hilSchemeChange (HILNODEPTR node, HILDATA *hd)
{
    register HILBLOCK *blkp;
    
    if (hd->noColChng > 0)
    {
        /* Re-allocate a new hilight block - run out of room */
        if(hd->noColChng == hilBlockS)
        {
            hilBlockS += 20 ;
            /* add 2 to hilBlockS to allow for a double trunc-scheme change */
            hilBlock = realloc(hilBlock, (hilBlockS+2)*sizeof(HILBLOCK)) ;
        }
        blkp = hilBlock + hd->noColChng + 1 ;
    }
    else
    {
        blkp = hilBlock + 1 ;
        blkp->column = 0;
    }
    /* Apply the new change of colour to the new enry created. */
    hd->noColChng++ ;
    blkp->scheme = node->scheme + (meSCHEME)(hd->colno);
    hd->hnode = node;
    hd->blkp = blkp;
}

static int
hilCopyChar(register int dstPos, register uint8 cc, HILDATA *hd)
{
    /* Change the selection hilight if required. */
    if ((hd->srcOff != 0xffff) && (hd->srcPos >= hd->srcOff))
        hd->hfunc (dstPos, hd);
    /* Copy the character. */
    __hilCopyChar(dstPos,cc)
              return dstPos ;
}

static int
hilCopyReplaceString(int sDstPos, register uint8 *srcText,
                     int len, HILDATA *hd)
{
    int dstPos = sDstPos ;
    int hoff, dstLen ;
    uint8 cc ;
    
    if((hd->srcOff != 0xffff) && (hd->srcPos+len >= hd->srcOff))
    {
        /* as the source len and destination len could be different, the best
         * we can do is set the hilight change position to be the ratio
         * between the two */
        uint8 *ss=srcText ;
        dstLen = 0 ;
        while((cc = *ss++) != 0)
        {
            if(cc == 0xff)
                ss++ ;
            dstLen++ ;
        }
        if(len)
            hoff = sDstPos+((hd->srcOff-hd->srcPos)*dstLen/len) ;
        else
            hoff = sDstPos+(hd->srcOff-hd->srcPos)*dstLen ;
    }
    else
        hoff = 0x7fffffff ;
    while((cc = *srcText++) != '\0')
    {
        if((cc == 0xff) && ((cc=*srcText++) != 0xff))
            cc = varTable[cc] ;
        if(dstPos >= hoff)
        {
            hd->hfunc (dstPos, hd);
            /* we may have another offset to cope with */
            if((hd->srcOff != 0xffff) && (hd->srcPos+len >= hd->srcOff))
            {
                if(len)
                    hoff = sDstPos+((hd->srcOff-hd->srcPos)*dstLen/len) ;
                else
                    hoff = sDstPos+(hd->srcOff-hd->srcPos)*dstLen ;
            }
            else
                hoff = 0x7fffffff ;
        }
        __hilCopyChar(dstPos,cc)
              }
    /* This is not quite right - but will have to do for present. Change
     * the hilighting according to the ammount of text used. */
    return dstPos ;
}

static int
hilCopyString(register int dstPos, register uint8 *srcText,HILDATA *hd)
{
    uint8 cc ;
    /* Handle the selection hilighting if enabled. */
    if (hd->srcOff != 0xffff)
    {
        uint16 srcPos;
        
        srcPos = hd->srcPos;
        while((cc = *srcText++) != '\0')
        {
            if (hd->srcOff <= srcPos)
                (hd->hfunc)(dstPos, hd);
            __hilCopyChar(dstPos,cc);
            srcPos++;
        }
    }
    else
    {
        while((cc = *srcText++) != '\0')
            __hilCopyChar(dstPos,cc);
    }
    return dstPos ;
}

static int
hilCopyLenString(register int dstPos, register uint8 *srcText,
                 register int len, HILDATA *hd)
{
    uint8 cc ;
    
    /* Handle the selection hilighting if enabled. */
    if ((hd->srcOff != 0xffff) && ((hd->srcOff - hd->srcPos) < len))
    {
        uint16 srcPos;
        
        srcPos = hd->srcPos;
        while (--len >= 0)
        {
            if (hd->srcOff <= srcPos)
                (hd->hfunc)(dstPos, hd);
            
            cc = *srcText++ ;
            __hilCopyChar(dstPos,cc);
            srcPos++;
        }
    }
    else
    {
        while(len--)
        {
            cc = *srcText++ ;
            __hilCopyChar(dstPos,cc);
        }
    }
    return dstPos ;
}


/*
 * hilHandleCallback
 * This function handles the callbacks when the text has reached a designated
 * position in the source line.
 */
static void
hilHandleCallback (int dstPos, struct HILDATA *hd)
{
    /* Handle the start of the selection hilight */
    if (hd->flag & VFSHBEG)
    {
        /* Change to selection hilighting */
        hd->colno += meSCHEME_SELECT;    /* To new index */
        hd->blkp->column = dstPos;       /* Terminate the current style */
        hilSchemeChange (hd->hnode, hd); /* New colour node */
        hd->blkp->column = dstPos;       /* New end of this style */
        hd->flag &= ~VFSHBEG;            /* Knock off the background mode */
        
        /* Reprime the callback with the location of the end of hilight */
        if (hd->flag & VFSHEND)
            hd->srcOff = selhilight.eoff;
        else
            hd->srcOff = 0xffff ; /* Set to unreachable value */
    }
    else if (hd->flag & VFSHEND)
    {
        /* Restore the existing hilighting */
        hd->colno -= meSCHEME_SELECT;    /* To new index */
        hd->blkp->column = dstPos;       /* Terminate current colour */
        hilSchemeChange (hd->hnode, hd); /* New colour node */
        hd->blkp->column = dstPos;       /* New end of this colour */
        hd->flag &= ~VFSHEND;            /* Remove the end flag */
        hd->srcOff = 0xffff ;            /* Set to unreachable value */
    }
}

uint16 
hilightLine(VIDEO *vp1)
{
    uint8 *srcText=vp1->line->l_text ;
    uint8  hilno = vp1->hilno, cc=meNLCHAR, mode ;
    HILNODEPTR node;
    uint16 len ;
    register int srcWid=vp1->line->l_used, dstPos=0 ;
    HILDATA hd;
    
    /* Initialise the hilight data structure */
    hd.root = hilights[hilno];          /* Root of the hilighting */
    hd.flag = vp1->flag;                /* Cache the video flag */
    hd.noColChng = 0;                   /* Reset number of colour changes */
    hd.srcPos = 0;                      /* Reset the source position */
    hd.srcOff = 0xffff;                 /* No callback required */
    hd.blkp = hilBlock + 1;             /* block pointer */
    
    /* Determine if we are processing a forground or hilighted line.
     * Determined by the current line flag */
    hd.colno = meSCHEME_NORMAL ;
    if (hd.flag & VFCURRL) 
        hd.colno += meSCHEME_CURRENT ;
    
    /* Determine if we are handling a selection hilighted line.
     * If so then set up the starting background colour in accordance
     * with the start/end/all selection hilighting flags in the video
     * structure. */
    if (hd.flag & VFSHMSK)              /* Selection hilight to be done ? */
    {
        hd.colno += meSCHEME_SELECT ;   /* Assume selection hilight enabled */
        
        if (hd.flag & VFSHALL)          /* Whole line hilighted ?? */
            hd.flag &= ~VFSHMSK;        /* Yes - kill off the flag */
        else
        {
            /* Check for the beginning of a slection hilight. If it is at the 
             * beginning of the line then use this as the starting colour. */
            if (hd.flag & VFSHBEG)
            {
                if (selhilight.soff!=0) /* Starts in column 0 ?? */
                {   /* Does not start in column 0 */
                    hd.colno -= meSCHEME_SELECT ;   /* remove selection hilight */
                    hd.srcOff = selhilight.soff;
                    hd.hfunc = hilHandleCallback;
                }
                else
                    hd.flag &=~VFSHBEG; /* Knock off the begin - found */
            }
            
            /* Check for the end of a selection hilight. If it is at the 
             * beginning of the line then it effectivelly disables the 
             * hilight. */
            if ((hd.flag & VFSHEND) && ((hd.flag & VFSHBEG) == 0))
            {
                if (selhilight.eoff==0) /* End's in column 0 ?? */
                {
                    hd.colno -= meSCHEME_SELECT; /* Yes - use normal colour */
                    hd.flag &=~VFSHEND;     /* Knock off end - found */
                }
                else
                {
                    hd.srcOff = selhilight.eoff;
                    hd.hfunc = hilHandleCallback;
                }
            }
        }
    }
    
    mode = (meHIL_MODESTTLN|meHIL_MODESTART|(hd.root->type&HFCASE)) ;
    if(vp1->bracket != NULL)
    {
        node = vp1->bracket ;
        hilSchemeChange ((hd.cnode = node), &hd);
        goto BracketJump ;
    }
    hilSchemeChange ((hd.cnode = hd.root), &hd);
    
    for(;;)
    {
        node = (HILNODEPTR) hd.root->rclose ;
        while(node != NULL)
        {
            if((int) node->close == hd.srcPos)
            {
                if((len = (int) node->rtoken) > srcWid)
                    len = srcWid ;
                len -= hd.srcPos ;
                goto column_token ;
            }
            else if((int) node->close > hd.srcPos)
                break ;
            node = (HILNODEPTR) node->rclose ;
        }
        if((node = findToken(hd.root,srcText+hd.srcPos,mode,cc,&len)) != NULL)
        {
column_token:
            if(node->tknEndOff)
                len -= node->tknEndOff ;
            if(len == 0)
            {
                /* the match string was 0 in length we will not move forward.
                 * This could lead to a spin, but not always as this could be
                 * a branch token.
                 * First time around set a flag to say we're here, next time
                 * around do something about it */
                if(mode & meHIL_MODEMOVE)
                    goto advance_char ;
                mode |= meHIL_MODEMOVE ;
            }
            else
                mode &= ~meHIL_MODEMOVE ;
            if(node->tknSttOff)
            {
                dstPos = hilCopyLenString(dstPos,srcText+hd.srcPos,node->tknSttOff,&hd) ;
                hd.srcPos += node->tknSttOff ;
                len -= node->tknSttOff ;
            }
            
            if(hd.blkp->column == dstPos)
            {
                hd.blkp->scheme = node->scheme + hd.colno ;
                hd.hnode = node;
            }
            else
            {
                hd.blkp->column = dstPos ;
                hilSchemeChange (node, &hd);
                hd.blkp->column = dstPos ;
            }
            if(node->type & HLREPLACE)
                dstPos = hilCopyReplaceString(dstPos, (uint8 *) node->rtoken,len,&hd) ;
            else if(len)
                dstPos = hilCopyLenString(dstPos,srcText+hd.srcPos,len,&hd) ;
            
            hd.srcPos += len ;
            
BracketJump:
            if(node->type & HLBRACKET)
            {
                uint8 ignore, tt=1, *c1, *c2, ss, *s1, *s2, solt ;
                
                c1 = (uint8 *) node->close ;
                /* the SOL is a special close bracket code for '^' */
                if((solt = ((c1[0] == 0xff) && (c1[1] == meHIL_TEST_SOL))))
                    c1 += 2 ;
                ignore = node->ignore ;
                s1 = srcText+hd.srcPos ;
                for( ; ; hd.srcPos++)
                {
                    if(!solt || (hd.srcPos == 0))
                    {                       
                        c2 = c1 ;
                        s2 = s1 ;
                        while((tt = *c2++) != '\0')
                        {
                            if((tt == 0xff) && ((tt = *c2++) != 0xff))
                            {
                                uint8 ret, vv ;
                                
                                vv = *c2++ ;
                                findTokenCharTest(ret,ss,s2,tt,c2) ;
                                if(!ret)
                                {
                                    tt = 1 ;
                                    break ;
                                }
                                varTable[vv] = ss ;
                            }
                            else
                            {
                                if((ss = *s2++) == '\0')
                                    ss = meNLCHAR ;
                                else if(mode & meHIL_MODECASE)
                                    ss = toLower(ss) ;
                                if(tt != ss)
                                    break ;
                            }
                            if(ss == meNLCHAR)
                            {
                                if((tt=*c2) != '\0')
                                    break ;
                                if(!node->clsEndOff)
                                    s2-- ;
                            }
                        }
                    }
                    else
                        tt = 1 ;
                    if((tt == '\0') || (hd.srcPos == srcWid))
                        break ;
                    ss = *s1++ ;
                    dstPos = hilCopyChar(dstPos,ss, &hd) ;
                    if(ss == ignore)
                    {
                        if(++hd.srcPos == srcWid)
                            break ;
                        dstPos = hilCopyChar(dstPos,*s1++,&hd) ;
                    }
                }
                if(tt != '\0')
                {
                    hd.blkp->column = dstPos ;
                    if(node->type & HLSNGLLNBT)
                        node = NULL ;
                    goto hiline_exit ;
                }
                len = s2-s1 ;
                if(node->clsSttOff)
                {
                    dstPos = hilCopyLenString(dstPos,s1,node->clsSttOff,&hd) ;
                    hd.srcPos += node->clsSttOff ;
                    s1 += node->clsSttOff ;
                    len -= node->clsSttOff ;
                }
                if(node->clsEndOff)
                    len -= node->clsEndOff ;
                if(node->type & HLREPLACE)
                    dstPos = hilCopyReplaceString(dstPos,(uint8 *) node->rclose,
                                                  len,&hd) ;
                else
                    dstPos = hilCopyLenString(dstPos,s1,len,&hd) ;
                hd.srcPos += len ;
                hd.cnode = hd.root ;
            }
            else if(node->type & HLCONTIN)
            {
                char *ss ;
                int   pos ;
                
                ss  = (char *) node->close ;
                len = strlen(ss) ;
                pos = srcWid ;
                while((srcText[--pos] == ' ') && (pos > 0))
                    ;
                pos -= len-1 ;
                if((pos >= hd.srcPos) &&
                   !strncmp((char *)srcText+pos,ss,len))
                {
                    dstPos = hilCopyString(dstPos,srcText+hd.srcPos, &hd) ;
                    hd.blkp->column = dstPos ;
                    goto hiline_exit ;
                }
            }
            if(node->type & HLENDLINE)
            {
                if(!(node->type & HLBRINEOL) ||
                   (hd.srcPos == srcWid))
                {
                    dstPos = hilCopyString(dstPos,srcText+hd.srcPos, &hd) ;
                    hd.blkp->column = dstPos ;
                    node = NULL ;
                    goto hiline_exit ;
                }
                hd.blkp->column = dstPos ;
                mode |= meHIL_MODETOEOL ;
                hd.cnode = node;
            }
            else
            {
                if(node->type & HLBRANCH)
                {
                    if((hilno = node->ignore) == 0)
                        hilno = vp1->wind->w_bufp->hiLight ;
                    hd.root = hilights[hilno] ;
                    hd.cnode = hd.root;
                }
                if(hd.blkp->column == dstPos)
                {
                    hd.blkp->scheme = hd.cnode->scheme + hd.colno ;
                    hd.hnode = node;
                }
                else
                {
                    hd.blkp->column = dstPos ;
                    hilSchemeChange(hd.cnode, &hd);
                    hd.blkp->column = dstPos ;
                }
            }
            if(dstPos)
            {
                cc = disLineBuff[dstPos-1] ;
                mode &= ~(meHIL_MODESTTLN|meHIL_MODESTART) ;
            }
        }
        else
        {
advance_char:
            if((cc=srcText[hd.srcPos]) == 0)
                break ;
            mode &= ~(meHIL_MODEMOVE|meHIL_MODESTART) ;
            if((mode & meHIL_MODESTTLN) && !isSpace(cc))
                mode &= ~meHIL_MODESTTLN ;
            dstPos = hilCopyChar(dstPos,cc, &hd) ;
            hd.srcPos++ ;
        }
    }
    
    if(hd.blkp->column != dstPos)
        hd.blkp->column = dstPos ;
    node = NULL ;
hiline_exit:
    
    if((hd.flag & (VFSHBEG|VFSHEND)) == VFSHEND)
    {
        /* must end the selection hilighting */
        hd.colno -= meSCHEME_SELECT ;
        hilSchemeChange (hd.hnode, &hd);
        hd.blkp->column = dstPos;
    }
    else if((hd.flag & (VFSHBEG|VFSHEND)) == VFSHBEG)
    {
        /* must end the selection hilighting */
        hd.colno += meSCHEME_SELECT ;
        hilSchemeChange (hd.hnode, &hd);
        hd.blkp->column = dstPos;
    }
    
    if((vp1[1].hilno != hilno) || (vp1[1].bracket != node))
    {
        vp1[1].flag |= VFCHNGD ;
        vp1[1].hilno = hilno ;
        vp1[1].bracket = node ;
    }
    return hd.noColChng ;
}

void
hilightLookBack(WINDOW *wp)
{
    VIDEO  *vptr ;
    BUFFER *bp ;
    register LINE *lp, *blp ;
    uint8  hilno ;
    HILNODEPTR root, bracket ;
    int ii, jj ;
    
    bp = wp->w_bufp ;
    hilno=bp->hiLight ;
    root = hilights[hilno] ;
    bracket = NULL ;
    
    ii = wp->topLineNo - wp->line_no ;
    jj = root->ignore ;
    lp = wp->w_dotp ;
    while(ii < jj)
    {
        if((blp=lback(lp)) == bp->b_linep)
            break ;
        lp = blp ;
        ii++ ;
    }
    if(ii > 0)
    {
        VIDEO vps[2] ;
        
        /* the flag is used only to set the current line, do the select hilighting
         * and flag the next line as changed if in a bracket, therefore init to 
         * 0 and forget about */
        vps[0].wind = wp ;
        vps[0].hilno = hilno ;
        vps[0].bracket = NULL ;
        vps[0].flag = 0 ;
        vps[1].hilno = hilno ;
        vps[1].bracket = NULL ;
        vps[1].flag = 0 ;
        for(;;)
        {
            vps[0].line = lp ;
            if((hilightLine(vps) > 1) && (vps[0].bracket != NULL) &&
               (vps[0].bracket == vps[1].bracket) &&
               (vps[0].bracket->type & (HLONELNBT|HLSNGLLNBT)))
                vps[1].bracket = NULL ;
            if(--ii == 0)
                break ;
            vps[0].hilno = vps[1].hilno ;
            vps[0].bracket = vps[1].bracket ;
            lp = lforw(lp) ;
        }
        hilno = vps[1].hilno ;
        bracket = vps[1].bracket ;
    }
    vptr  = wp->w_vvideo->video + wp->firstRow ;  /* Video block */
    if((vptr->hilno != hilno) || (vptr->bracket != bracket))
    {
        vptr->hilno = hilno ;
        vptr->bracket = bracket ;
        vptr->flag |= VFCHNGD ;
    }
}

#define hilOffsetChar(off,dstPos,dstJmp,cc)                                  \
{                                                                            \
    int ii ;                                                                 \
    if(isDisplayable(cc))                                                    \
        ii = 1 ;                                                             \
    else if(cc == TAB)                                                       \
        ii = get_tab_pos(dstPos) + 1 ;                                       \
    else if(cc < 0x20)                                                       \
        ii = 2 ;                                                             \
    else                                                                     \
        ii = 4 ;                                                             \
    if(dstJmp)                                                               \
    {                                                                        \
        *off++ = ii+dstJmp ;                                                 \
        dstJmp = 0 ;                                                         \
    }                                                                        \
    else                                                                     \
        *off++ = ii ;                                                        \
    dstPos += ii ;                                                           \
}

#define hilOffsetString(lastcc,off,dstPos,dstJmp,srcText)                    \
{                                                                            \
    uint8 __cc, *__ss=(srcText) ;                                            \
    while((__cc = *__ss++) != '\0')                                          \
    {                                                                        \
        hilOffsetChar(off,dstPos,dstJmp,__cc)                                \
        lastcc = __cc ;                                                      \
    }                                                                        \
}

#define hilOffsetLenString(lastcc,off,dstPos,dstJmp,srcText,len)             \
{                                                                            \
    uint16 __ll=len ;                                                        \
    uint8 *__ss=(srcText) ;                                                  \
    while(__ll--)                                                            \
    {                                                                        \
        lastcc = *__ss++ ;                                                   \
        hilOffsetChar(off,dstPos,dstJmp,lastcc)                              \
    }                                                                        \
}

static int
hilOffsetReplaceString(register uint8 **offPtr, register int dstPos,
                       int *dstJmp, register uint8 *srcText, int len)
{
    uint8 *off=*offPtr, cc, lastcc='\0' ;
    int            rlen=dstPos, ii, dd=*dstJmp ;
    
    while((cc = *srcText++) != '\0')
    {
        if((cc == 0xff) && ((cc=*srcText++) != 0xff))
            cc = varTable[cc] ;
        lastcc = cc ;
        hilOffsetChar(off,dstPos,dd,cc)
    }
    
    rlen = dstPos-rlen ;
    if(len)
    {
        dd = *dstJmp ;
        off = *offPtr ;
        while(len)
        {
            ii = rlen / len-- ;
            if(dd)
            {
                *off++ = ii+dd ;
                dd = 0 ;
                *dstJmp = 0 ;
            }
            else
                *off++ = ii ;
            rlen -= ii ;
        }
        *offPtr = off ;
    }
    else
        *dstJmp += rlen ;
    *off = lastcc ;
    return dstPos ;
}

void
hilightCurLineOffsetEval(WINDOW *wp)
{
    uint8 *srcText=wp->w_dotp->l_text ;
    uint8 *off=wp->curLineOff->l_text ;
    uint8  hilno=wp->w_bufp->hiLight, cc=meNLCHAR, mode ;
    HILNODEPTR node, root ;
    uint16 len ;
    register int srcWid=wp->w_dotp->l_used, srcPos=0, dstPos=0 ;
    int dstJmp=0 ;
    
    root = hilights[hilno];          /* Root of the hilighting */
    
    mode = (meHIL_MODESTTLN|meHIL_MODESTART|(root->type&HFCASE)) ;
    /*    if(vp1->bracket != NULL)*/
    /*    {*/
    /*        node = vp1->bracket ;*/
    /*        hilSchemeChange ((hd.cnode = node), &hd);*/
    /*        goto BracketJump ;*/
    /*    }*/
    for(;;)
    {
        node = (HILNODEPTR) root->rclose ;
        while(node != NULL)
        {
            if((int) node->close == srcPos)
            {
                if((len = (int) node->rtoken) > srcWid)
                    len = srcWid ;
                len -= srcPos ;
                goto column_token ;
            }
            else if((int) node->close > srcPos)
                break ;
            node = (HILNODEPTR) node->rclose ;
        }
        if((node = findToken(root,srcText+srcPos,mode,cc,&len)) != NULL)
        {
column_token:
            if(node->tknEndOff)
                len -= node->tknEndOff ;
            if(len == 0)
            {
                /* the match string was 0 in length we will not move forward.
                 * This could lead to a spin, but not always as this could be
                 * a branch token.
                 * First time around set a flag to say we're here, next time
                 * around do something about it */
                if(mode & meHIL_MODEMOVE)
                    goto advance_char ;
                mode |= meHIL_MODEMOVE ;
            }
            else
                mode &= ~meHIL_MODEMOVE ;
            if(node->type & HLREPLACE)
            {
                if(node->tknSttOff)
                {
                    hilOffsetLenString(cc,off,dstPos,dstJmp,srcText+srcPos,node->tknSttOff) ;
                    srcPos += node->tknSttOff ;
                    len -= node->tknSttOff ;
                }
                dstPos = hilOffsetReplaceString(&off,dstPos,&dstJmp,(uint8 *) node->rtoken,len) ;
				if(*off)
					cc = *off ;
            }
            else
            {
                hilOffsetLenString(cc,off,dstPos,dstJmp,srcText+srcPos,len) ;
            }
            srcPos += len ;
            
            /*BracketJump:*/
            if(node->type & HLBRACKET)
            {
                uint8 ignore, tt=1, *c1, ss, *c2, *s1, *s2, solt ;
                
                c1 = (uint8 *) node->close ;
                /* the SOL is a special close bracket code for '^' */
                if((solt = ((c1[0] == 0xff) && (c1[1] == meHIL_TEST_SOL))))
                    c1 += 2 ;
                ignore = node->ignore ;
                s1 = srcText+srcPos ;
                for( ; ; srcPos++)
                {
                    if(!solt || (srcPos == 0))
                    {                       
                        c2 = c1 ;
                        s2 = s1 ;
                        while((tt = *c2++) != '\0')
                        {
                            if((tt == 0xff) && ((tt = *c2++) != 0xff))
                            {
                                uint8 ret, vv ;
                                
                                vv = *c2++ ;
                                findTokenCharTest(ret,ss,s2,tt,c2) ;
                                if(!ret)
                                {
                                    tt = 1 ;
                                    break ;
                                }
                                varTable[vv] = ss ;
                            }
                            else if((ss = *s2++) != tt)
                            {
                                if((tt != meNLCHAR) || (ss != '\0'))
                                    break ;
                            }
                            if(ss == '\0')
                            {
                                if((tt=*c2) != '\0')
                                    break ;
                                if(!node->clsEndOff)
                                    s2-- ;
                            }
                        }
                    }
                    else
                        tt = 1 ;
                    if((tt == '\0') || (srcPos == srcWid))
                        break ;
                    ss = *s1++ ;
                    hilOffsetChar(off,dstPos,dstJmp,ss) ;
                    if(ss == ignore)
                    {
                        if(++srcPos == srcWid)
                            break ;
                        ss = *s1++ ;
                        hilOffsetChar(off,dstPos,dstJmp,ss) ;
                    }
                }
                if(tt != '\0')
                    goto hiline_exit ;
                
                len = s2-s1 ;
                
                if(node->clsEndOff)
                    len -= node->clsEndOff ;
                
                if(node->type & HLREPLACE)
                {
                    if(node->clsSttOff)
                    {
                        hilOffsetLenString(cc,off,dstPos,dstJmp,s1,node->clsSttOff) ;
                        srcPos += node->clsSttOff ;
                        len -= node->clsSttOff ;
                    }
                    dstPos = hilOffsetReplaceString(&off,dstPos,&dstJmp,(uint8 *) node->rclose,len) ;
					if(*off)
						cc = *off ;
                }
                else
                {
                    hilOffsetLenString(cc,off,dstPos,dstJmp,s1,len) ;
                }
                srcPos += len ;
            }
            else if(node->type & HLCONTIN)
            {
                char *ss ;
                int   pos ;
                
                ss  = (char *) node->close ;
                len = strlen(ss) ;
                pos = srcWid ;
                while((srcText[--pos] == ' ') && (pos > 0))
                    ;
                pos -= len-1 ;
                if((pos >= srcPos) &&
                   !strncmp((char *)srcText+pos,ss,len))
                {
                    hilOffsetString(cc,off,dstPos,dstJmp,srcText+srcPos) ;
                    goto hiline_exit ;
                }
            }
            if(node->type & HLENDLINE)
            {
                if(!(node->type & HLBRINEOL) || (srcPos == srcWid))
                {
                    hilOffsetString(cc,off,dstPos,dstJmp,srcText+srcPos) ;
                    goto hiline_exit ;
                }
                mode |= meHIL_MODETOEOL ;
            }
            else
            {
                if(node->type & HLBRANCH)
                {
                    if((hilno = node->ignore) == 0)
                        hilno = wp->w_bufp->hiLight ;
                    root = hilights[hilno] ;
                }
            }
            if(dstPos)
                mode &= ~(meHIL_MODESTTLN|meHIL_MODESTART) ;
        }
        else
        {
advance_char:
            if((cc=srcText[srcPos]) == 0)
                break ;
            mode &= ~(meHIL_MODEMOVE|meHIL_MODESTART) ;
            if((mode & meHIL_MODESTTLN) && !isSpace(cc))
                mode &= ~meHIL_MODESTTLN ;
            hilOffsetChar(off,dstPos,dstJmp,cc) ;
            srcPos++ ;
        }
    }
hiline_exit:
    *off = '\0' ;
}


#define HIGOTCONT     0x8000

#define INDINDCURLINE 0x0100
#define INDINDONWARD  0x0200
#define INDGOTIND     0x0400

#define INDBRACKETOPEN   (0x1000)
#define INDBRACKETCLOSE  (0x1800)  
#define INDCONTINUE      (0x0800|INDGOTIND)
#define INDEXCLUSION     (0x2000)
#define INDFIXED         (INDGOTIND)
#define INDIGNORE        (0x2800)
#define INDNEXTONWARD    (INDINDONWARD|INDGOTIND)
#define INDCURONWARD     (INDINDCURLINE|INDINDONWARD|INDGOTIND)
#define INDSINGLE        (INDINDCURLINE|INDGOTIND)

#define noINDTYPES 8
int
indent(int f, int n)
{
    static uint8  typesChar[(noINDTYPES*2)+1]="bcefinosBCEFINOS" ;
    static uint16 typesFlag[noINDTYPES]=
    { INDBRACKETOPEN, INDCONTINUE, INDEXCLUSION, INDFIXED, INDIGNORE, INDNEXTONWARD, INDCURONWARD, INDSINGLE } ;
    static uint16 typesType[noINDTYPES]=
    { 0x00, 0x00, HLBRACKET, HLENDLINE|HLSTTLINE, HLENDLINE, 0x00, 0x00, 0x00 } ;
    HILNODEPTR    root, node ;
    uint8         buf[MAXBUF] ;
    uint8         indno ;
    int           itype ;
    uint16        htype ;
    
    if(n == 0)
    {
        if((mlreply((uint8 *)"Enter indent no",0,0,buf,MAXBUF-1) != TRUE) ||
           ((indno = (uint8) meAtoi(buf)) == 0) || 
           (mlreply((uint8 *)"Flags",0,0,buf,MAXBUF-1) != TRUE) ||
           ((itype = meAtoi(buf)),
            (mlreply((uint8 *)"look back",0,0,buf,MAXBUF) != TRUE)))
            return mlwrite(MWABORT|MWPAUSE,(uint8 *)"Invalid init-indent entry") ;
        if((indents[indno] = createHilight(indno,&noIndents,&indents)) == NULL)
            return FALSE ;
        indents[indno]->ignore = (uint8) meAtoi(buf) ;
        indents[indno]->type   = itype ;
        
        return TRUE ;
    }
    
    n = (n < 0) ? ADDTOKEN_REMOVE:0 ;
    
    if((mlreply((uint8 *)"Ind no",0,0,buf,MAXBUF-1) != TRUE) ||
       ((indno = (uint8) meAtoi(buf)) > noIndents) ||
       ((root  = indents[indno]) == NULL) ||
       ((itype = mlCharReply((uint8 *)"Type: ",0,typesChar,NULL)) == -1) ||
       (mlreply((uint8 *)"Token",0,0,buf,MAXBUF-1) != TRUE))
        return FALSE ;
    
    itype = (int) (((uint8 *)meStrchr(typesChar,itype)) - typesChar) ;
    if(itype >= noINDTYPES)
    {
        itype -= noINDTYPES ;
        htype = (HLTOKEN|typesType[itype]) ;
    }
    else
        htype = typesType[itype] ;
    
    /* add the token */
    node = meHiltTokenAddSearchString(root,NULL,(uint8 *) buf,(htype|n)) ;
    if(n & ADDTOKEN_REMOVE)
    {
        if(itype == 0)
        {
            if(mlreply((uint8 *)"Close",0,0,buf,MAXBUF-1) != TRUE)
                return FALSE ;
            meHiltTokenAddSearchString(root,NULL,(uint8 *) buf,(htype|n)) ;
        }
        return TRUE ;
    }
    if(node == NULL)
        return FALSE ;
    
    node->scheme = typesFlag[itype] ;
    
    if(itype == 1)
        root->type |= HIGOTCONT ;
    else if(itype <= 2)
    {
        if(mlreply((uint8 *)"Close",0,0,buf,MAXBUF-1) != TRUE)
            return FALSE ;
        if(itype == 0)
        {
            if((node = meHiltTokenAddSearchString(root,NULL,(uint8 *) buf,(htype|n))) == NULL)
                return FALSE ;
            node->scheme = INDBRACKETCLOSE ;
        }
        else
        {
            if((meHiltTokenAddSearchString(root,node,(uint8 *) buf,0) == NULL) ||
               (mlreply((uint8 *)"Ignore",0,0,buf,MAXBUF-1) != TRUE))
                return FALSE;
            node->ignore = buf[0] ;
        }
    }
    if(typesFlag[itype] & INDGOTIND)
    {
        if(mlreply((uint8 *)"Indent",0,0,buf,MAXBUF-1) != TRUE)
            return FALSE ;
        node->scheme |= ((int8) meAtoi(buf)) & 0x00ff ;
    }
    return TRUE ;
}

int
indentLine(void)
{
    HILBLOCK *blkp;
    HILNODEPTR *bhis ;
    VIDEO  vps[2] ;
    uint16 noColChng, fnoz, ii ;
    uint8 *ss ;
    int ind, cind, coff ;
    
    vps[0].wind = curwp ;
    vps[0].hilno = curbp->indent ;
    vps[1].hilno = curbp->indent ;
    vps[0].bracket = NULL ;
    vps[1].bracket = NULL ;
    vps[0].flag = 0 ;
    vps[0].line = curwp->w_dotp ;
    
    bhis = hilights ;
    hilights = indents ;
    noColChng = hilightLine(vps) ;
    hilights = bhis ;
    blkp = hilBlock + 1 ;
    /*    printf("Got %d colour changes\n",noColChng) ;*/
    /*    for(ii=0 ; ii<noColChng ; ii++)*/
    /*        printf("Colour change %d is to 0x%x at column %d\n",ii,hilCol[ii].scheme,hilCol[ii].column) ;*/
    ss = disLineBuff ;
    ss[blkp[noColChng-1].column] = '\0' ;
    while(*ss == ' ')
        ss++ ;
    cind = ss - disLineBuff ;
    if(blkp[0].scheme != 0x00)
        fnoz = blkp[0].scheme ;
    else if(noColChng > 1)
        fnoz = blkp[1].scheme ;
    else
        fnoz = 0 ;
    if((fnoz & 0xff00) == INDFIXED)
        ind = ((int8) (fnoz & 0x0ff)) ;
    else
    {
        int brace=0, jj, contFlag=0, aind, lind, nind ;
        
        if((fnoz & INDINDCURLINE) &&
           ((fnoz == blkp[0].scheme) || (cind >= blkp[0].column)))
            aind = ((int8) (fnoz & 0x0ff)) ;
        else
            aind = 0 ;
        hilights = indents ;
        jj = indents[curbp->indent]->ignore ;
        for(ind=0 ; (--jj >=0) ; )
        {
            if((vps[0].line = lback(vps[0].line)) == curbp->b_linep)
                break ;
            noColChng = hilightLine(vps) ;
            if((blkp[0].scheme & 0xff00) == INDFIXED)
                continue ;
            ss = disLineBuff ;
            ss[blkp[noColChng-1].column] = '\0' ;
            while(*ss == ' ')
                ss++ ;
            if(*ss == '\0')
                continue ;
            /*            printf("Got prev line breakdown %d colour changes ind %d\n",noColChng,ind) ;*/
            for(ii=0 ; ii<noColChng ; ii++)
            {
                fnoz = (blkp[ii].scheme & 0xff00) ;
                if(fnoz == INDNEXTONWARD)
                    ind += ((int8) (blkp[ii].scheme & 0x0ff)) ;
                else if((fnoz == INDSINGLE) && 
                        ((ii == 0) || ((ii == 1) && ((ss-disLineBuff) >= blkp[0].column))))
                    ind -= ((int8) (blkp[ii].scheme & 0x0ff)) ;
                else if((fnoz == INDCURONWARD) && (ii != 0) &&
                        ((ii != 1) || ((ss-disLineBuff) < blkp[0].column)))
                    ind += ((int8) (blkp[ii].scheme & 0x0ff)) ;
                else if(fnoz == INDBRACKETOPEN)
                {
                    if(brace >= 0)
                        blkp[brace].column = blkp[ii].column ;
                    brace++ ;
                }
                else if(fnoz == INDBRACKETCLOSE)
                    brace-- ;
                else if(fnoz == INDCONTINUE)
                {
                    contFlag |= 0x03 ;
                    if(!(contFlag & 0x04))
                        ind += ((int8) (blkp[ii].scheme & 0x0ff)) ;
                }
                /*                printf("Colour change %d is to 0x%x at column %d, ind %d\n",ii,blkp[ii].scheme,blkp[ii].column,ind) ;*/
            }
            if(brace < 0)
            {
                if((contFlag == 0) && (indents[curbp->indent]->type & HIGOTCONT))
                    contFlag = 0x07 ;
                nind = ind = 0 ;
                continue ;
            }
            if((contFlag & 0x03) == 0x02)
                ind = lind ;
            else if(brace)
                ind = blkp[brace-1].column ;
            else
            {
                if(contFlag == 7)
                    ind += nind ;
                nind = ind ;
                ind += ss - disLineBuff + aind ;
                if((contFlag == 0) && (indents[curbp->indent]->type & HIGOTCONT))
                    contFlag = 0x07 ;
            }
            if(ind < 0)
                ind = 0 ;
            if(!brace && (contFlag & 0x01))
            {
                lind = ind ;
                ind = 0 ;
                contFlag &= ~0x01 ;
                continue ;
            }
            break ;
        }
        hilights = bhis ;
    }
    /*    printf("\nIndent line to %d\n\n",ind) ;*/
    /* Always do the doto change so the tab on the left hand edge moves
     * the cursor to the first non-white char
     */
    if((coff = getccol()-cind) < 0)
        coff = 0 ;
    coff += ind ;
    /* Now change the indent if required */
    if(cind != ind)
    {
        register int rr ;
        if((rr=bchange()) != TRUE)               /* Check we can change the buffer */
            return rr ;
        curwp->w_doto = 0 ;
        ss = curwp->w_dotp->l_text ;
        while((*ss == ' ') || (*ss == '\t'))
            ss++ ;
        cind = ss - curwp->w_dotp->l_text ;
        ldelete(cind,6) ;
        if(meModeTest(curbp->b_mode,MDTAB))
            rr = 0 ;
        else
        {
            rr = ind / tabwidth ;
            ind -= rr * tabwidth ;
            linsert(rr,'\t') ;
        }
        linsert(ind,' ') ;
#if MEUNDO
        if(meModeTest(curbp->b_mode,MDUNDO))
        {
            meUndoAddReplaceEnd(ind+rr) ;
            curbp->fUndo->doto = ind+rr ;
            curbp->fUndo->type |= MEUNDO_REVS ;
        }
#endif
    }
    setccol(coff) ;
    return TRUE ;
}

#endif /* HILIGHT */
