/* -*- C -*- ****************************************************************
 *
 *  System        : MicroEmacs Jasspa Distribution
 *  Module        : spell.c
 *  Synopsis      : Spell checking routines
 *  Created By    : Steven Phillips
 *  Created       : 21/12/94
 *  Last Modified : <000221.0749>
 *
 *  Description
 *
 *  Notes
 *      The external input/output mostly complies with ispell, there are a 
 *      few extenstions such as the disabling of hypens and auto-replacement
 *      words. The internals, including dictionaries, are completely different.
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

#define __SPELLC                        /* Define filename */

#include <string.h>
#include "emain.h"

#if SPELL

#define DTACTIVE        0x01
#define DTCHNGD         0x02
#define DTCREATE        0x04

#define SP_REG_ROOT  "/history/spell"   /* Root of spell in registry */
#define SP_REGI_SAVE "autosave"         /* Registry item - autosave */

#define TBLINCSIZE 512
#define NOTBLSIZES 11

uint32 tableSizes[NOTBLSIZES+1] = 
{   211,   419,   811,  1601,  3203, 4801, 6421, 
    8419, 10831, 13001, 16103, 20011
} ;

#define FRSTSPELLRULE 'A'
#define LASTSPELLRULE 'z'
#define NOSPELLRULES  LASTSPELLRULE-FRSTSPELLRULE+1
#define RULE_PREFIX   1
#define RULE_SUFFIX   2
#define RULE_MIXABLE  4

typedef struct meSPELLRULE {
    struct meSPELLRULE *next ;
    uint8 *ending ;
    uint8 *remove ;
    uint8 *append ;
    uint8 endingLen ;
    uint8 removeLen ;
    uint8 appendLen ;
    int8  changeLen ;
    /* rule is used by find-words for continuation &
     * guessList to eliminate suffixes
     */
    uint8 rule ;
} meSPELLRULE ;

uint8 meRuleScore[NOSPELLRULES]={0} ;
uint8 meRuleFlags[NOSPELLRULES+1]={0} ;
meSPELLRULE  *meRuleTable[NOSPELLRULES+1]={0} ;

typedef uint8 meDICTADDR[3] ;
#define meMAXWORDLEN 127
#define meMAXGUESSES 20
#define meMAXSCORE   60

typedef struct {
    meDICTADDR    next ;
    uint8 wordLen ;     
    uint8 flagLen ;     
    uint8 data[1] ;
} meDICTWORD ;

typedef struct {
    meDICTADDR    next ;
    uint8 wordLen ;     
    uint8 flagLen ;     
    uint8 data[meMAXWORDLEN+meMAXWORDLEN] ;
} meDICTADDWORD ;

#define meDICTWORDSIZE  ((int)((meDICTWORD *)0)->data)

typedef struct meDICTIONARY {
    uint8   flags ;
    uint8  *fname ;
    uint32  noWords ;
    uint32  tableSize ;
    uint32  dSize ;
    uint32  dUsed ;
    meDICTADDR *table ;
    struct meDICTIONARY *next ;
} meDICTIONARY ;

typedef uint8 meWORDBUF[meMAXWORDLEN] ;
static int           longestPrefixChange=0 ;
static int           longestSuffixRemove=0 ;
static meDICTIONARY *dictHead=NULL ;
static meDICTIONARY *dictIgnr=NULL ;
static meDICTWORD   *lastIgnr=NULL ;
static meDICTWORD   *wordCurr=NULL ;
static int           caseFlags=0 ;
static int           hyphenCheck=1 ;
static int           maxScore=meMAXSCORE ;

/* find-words static variables */
static uint8 *sfwCurMask=NULL ;
static meDICTIONARY  *sfwCurDict=NULL ;
static uint32         sfwCurIndx=0 ;
static meDICTWORD    *sfwCurWord=NULL ;
static meSPELLRULE   *sfwPreRule=NULL ;
static meSPELLRULE   *sfwSufRule=NULL ;
static int            sfwFlags=0 ;

#define SPELL_ERROR           0x80

#define SPELL_CASE_FUPPER     0x01
#define SPELL_CASE_FLOWER     0x02
#define SPELL_CASE_CUPPER     0x04
#define SPELL_CASE_CLOWER     0x08

#define toLatinLower(c)       (isUpper(toUserFont(c)) ? toLatinFont(toggleCase(toUserFont(c))):(c))
#define toggleLatinCase(c)    toLatinFont(toggleCase(toUserFont(c)))

#define meEntryGetAddr(ed)       (((ed)[0] << 16) | ((ed)[1] << 8) | (ed)[2])
#define meEntrySetAddr(ed,off)   (((ed)[0] = (uint8) ((off)>>16)),((ed)[1] = (uint8) ((off)>> 8)),((ed)[2] = (uint8) (off)))
#define meWordGetErrorFlag(wd)   (wd->flagLen &  SPELL_ERROR)
#define meWordSetErrorFlag(wd)   (wd->flagLen |= SPELL_ERROR)
#define meWordGetAddr(wd)        meEntryGetAddr((wd)->next)
#define meWordSetAddr(wd,off)    meEntrySetAddr((wd)->next,off)
#define meWordGetWordLen(wd)     (wd->wordLen)
#define meWordSetWordLen(wd,len) (wd->wordLen=len)
#define meWordGetWord(wd)        (wd->data)
#define meWordGetFlagLen(wd)     (wd->flagLen & ~SPELL_ERROR)
#define meWordSetFlagLen(wd,len) (wd->flagLen = meWordGetErrorFlag(wd) | len)
#define meWordGetFlag(wd)        (wd->data+meWordGetWordLen(wd))


static uint32
hashFunc(uint32 tableSize, uint8 *word, int len)
{
    register uint32 h=0, g ;
    register uint8 c ;
    
    while(--len >= 0)
    {
        c = *word++ ;
        h = (h << 4) + c ;
        if((g=h & 0xf0000000) != 0)
        {
            h ^= (g >> 24) ;
            h ^= g ;
        }
    }
    return (h % tableSize) ;
}


static meDICTWORD *
lookupWord(meDICTIONARY *dict, uint8 *word, int len)
{
    uint32  n, off ;
    meDICTWORD    *ent ;
    int            s, ll ;
    
    n = hashFunc(dict->tableSize,word,len) ;
    off = meEntryGetAddr(dict->table[n]) ;
    
    while(off != 0)
    {
        ent = (meDICTWORD *) (off + (uint32) dict->table) ;
        ll = meWordGetWordLen(ent) ;
        if(ll >= len)
        {
            if(ll > len)
                break ;
            s = meStrncmp(meWordGetWord(ent),word,ll) ;
            if(s > 0)
                break ;
            if(s == 0)
                return ent ;
        }
        off = meWordGetAddr(ent) ;
    }
    return NULL ;
}

static void
dictionaryDeleteWord(meDICTIONARY *dict, uint8 *word, int len)
{
    meDICTWORD  *ent ;
    if((ent = lookupWord(dict,word,len)) != NULL)
    {
        /* Remove the old entry */
        meWordGetWord(ent)[0] = '\0' ;
        (dict->noWords)-- ;
        dict->flags |= DTCHNGD ;
    }
}

static int
dictionaryAddWord(meDICTIONARY *dict, meDICTWORD *wrd)
{
    uint32  n, len, off ;
    meDICTWORD *ent, *lent, *nent ;
    uint8  wlen, flen, olen, *word ;
    
    word = meWordGetWord(wrd) ;
    wlen = meWordGetWordLen(wrd) ;
    flen = meWordGetFlagLen(wrd) ;
    if((ent = lookupWord(dict,word,wlen)) != NULL)
    {
        if(flen == 0)
            return TRUE ;
        /* see if the word list already exists in the rule list */
        if(!meWordGetErrorFlag(ent) &&
           !meWordGetErrorFlag(wrd) &&
           ((olen=meWordGetFlagLen(ent)) > 0))
        {
            uint8 *ff, *ef ;
            ef = meWordGetFlag(ent) ;
            ff = meWordGetFlag(wrd) ;
            ff[flen] = '\0' ;
            if(meStrstr(ef,ff) != NULL)
                return TRUE ;
            ff[flen++] = '_' ;
            memcpy(ff+flen,ef,olen) ;
            flen += olen ;
        }
        /* Remove the old entry */
        meWordGetWord(ent)[0] = '\0' ;
        (dict->noWords)-- ;
    }
    dict->flags |= DTCHNGD ;
    len = meDICTWORDSIZE + wlen + flen ;
        
    if((dict->dUsed+len) > dict->dSize)
    {
        dict->dSize = dict->dUsed+len+TBLINCSIZE ;
        if((dict->table = (meDICTADDR *) meRealloc(dict->table,dict->dSize)) == NULL)
            return FALSE ;
    }
    nent = (meDICTWORD *) (dict->dUsed + (uint32) dict->table) ;
    nent->wordLen = wrd->wordLen ;
    nent->flagLen = wrd->flagLen ;
    meWordSetFlagLen(nent,flen) ;
    memcpy(meWordGetWord(nent),word,wlen) ;
    memcpy(meWordGetFlag(nent),meWordGetFlag(wrd),flen) ;
    
    lent = NULL ;
    n = hashFunc(dict->tableSize,word,wlen) ;
    off  = meEntryGetAddr(dict->table[n]) ;
    while(off != 0)
    {
        ent = (meDICTWORD *) (off + (uint32) dict->table) ;
        if(meWordGetWordLen(ent) >= wlen)
        {
            if((meWordGetWordLen(ent) > wlen) ||
               meStrncmp(meWordGetWord(ent),word,wlen) > 0)
                break ;
        }
        off = meEntryGetAddr(ent->next) ;
        lent = ent ;
    }
    if(lent == NULL)
        meEntrySetAddr(dict->table[n],dict->dUsed) ;
    else
        meWordSetAddr(lent,dict->dUsed) ;
    meWordSetAddr(nent,off) ;
    dict->dUsed += len ;
    (dict->noWords)++ ;
    if(dict == dictIgnr)
        lastIgnr = nent ;
    return TRUE ;
}


static void
rehashDictionary(meDICTIONARY *dict)
{
    meDICTWORD     *ent ;
    meDICTADDR     *table, *tbl ;
    uint32  tableSize, oldtableSize, ii, 
                   dUsed, dSize, noWords, off ;
    
    oldtableSize = dict->tableSize ;
    noWords = dict->noWords >> 2 ;
    for(ii=0 ; ii<NOTBLSIZES ; ii++)
        if(tableSizes[ii] > noWords)
            break ;
    if((tableSize = tableSizes[ii]) == oldtableSize)
        return ;
    dUsed = sizeof(meDICTADDR)*tableSize ;
    dSize = dict->dUsed+dUsed ;
    
    if((table = (meDICTADDR *) meMalloc(dSize)) == NULL)
        return ;
    memset(table,0,dUsed) ;
    tbl = dict->table ;
    dict->dUsed = dUsed ;
    dict->dSize = dSize ;
    dict->table = table ;
    dict->tableSize = tableSize ;
    dict->noWords = 0 ;
    for(ii=0 ; ii<oldtableSize ; ii++)
    {
        off = meEntryGetAddr(tbl[ii]) ;
        while(off != 0)
        {
            ent = (meDICTWORD *) (off + (uint32) tbl) ;
            /* Check this word has not been erased, if not then add */
            if(meWordGetWord(ent)[0] != '\0')
                dictionaryAddWord(dict,ent) ;
            off = meWordGetAddr(ent) ;
        }
    }
    free(tbl) ;
}

            
static int
loadDictionary(meDICTIONARY *dict)
{
    FILE           *fp ;
    meDICTADDR     *table, info[2] ;
    uint32   dSize, dUsed ;
    
    if((fp = fopen((char *)dict->fname,"rb")) == NULL)
        return mlwrite(MWABORT,(uint8 *)"Failed to open dictionary [%s]",dict->fname) ;
    fseek(fp,0,SEEK_END) ;
    dUsed = ftell(fp) ;
    fseek(fp,0,SEEK_SET) ;
    if((fgetc(fp) != 0xED) || (fgetc(fp) != 0xF1))
        return mlwrite(MWABORT,(uint8 *)"[%s does not have correct id string]",dict->fname) ;
    fread(info,sizeof(meDICTADDR),2,fp) ;
    dUsed -= ftell(fp) ;
    dSize = dUsed + TBLINCSIZE ;
    
#ifdef BUILD_INSURE_VERSIONS
    table = (meDICTADDR *) meMalloc(10) ;
#endif
    if ((table = (meDICTADDR *) meMalloc(dSize)) == NULL)
        return FALSE ;
    dict->table = table ;
    dict->dUsed = dUsed ;
    dict->dSize = dSize ;
    dict->noWords = meEntryGetAddr(info[0]) ;
    dict->tableSize = meEntryGetAddr(info[1]) ;
    if((dSize = fread(table,1,dUsed,fp)) != dUsed)
    {
        free(table) ;
        return mlwrite(MWABORT,(uint8 *)"failed to read dictionary %s",dict->fname) ;
    }
    fclose(fp) ;
    
    dict->flags |= DTACTIVE ;
    rehashDictionary(dict) ;
    
    return TRUE ;
}


static int
initDictionaries(void)
{
    meDICTIONARY   *dict, *ldict ;
    
    ldict = NULL ;
    dict = dictHead ;
    if(dict == NULL)
        return mlwrite(MWABORT,(uint8 *)"[No dictionaries]") ;
    while(dict != NULL)
    {
        if(!(dict->flags & DTACTIVE) &&
           (loadDictionary(dict) != TRUE))
        {
            if(ldict == NULL)
                dictHead = dict->next ;
            else
                ldict->next = dict->next ;
            meFree(dict->fname) ;
            meFree(dict) ;
            return FALSE ;
        }
        ldict = dict ;
        dict = ldict->next ;
    }
    if(dictIgnr == NULL)
    {
        meDICTADDR    *table ;
        uint32  dSize, dUsed ;
        
        dUsed = sizeof(meDICTADDR)*tableSizes[0] ;
        dSize = dUsed + TBLINCSIZE ;
        if(((dictIgnr = (meDICTIONARY *) meMalloc(sizeof(meDICTIONARY))) == NULL) ||
           ((table = (meDICTADDR *) meMalloc(dSize)) == NULL))
            return FALSE ;
        memset(table,0,dUsed) ;
        dictIgnr->dSize = dSize ;
        dictIgnr->dUsed = dUsed ;
        dictIgnr->table = table ;
        dictIgnr->noWords = 0 ;
        dictIgnr->tableSize = tableSizes[0] ;
        dictIgnr->flags = DTACTIVE ;
        dictIgnr->fname = NULL ;
        dictIgnr->next = NULL ;
    }
    return TRUE ;
}

static meDICTIONARY *
findDictionary(int flag)
{
    meDICTIONARY *dict ;
    uint8 fname[FILEBUF], tmp[FILEBUF] ;
    int found ;
    
    if(mlreply((uint8 *)"Dictionary name",MLFILECASE,0,tmp,FILEBUF) != TRUE)
        return FALSE ;
    
    if(!fileLookup(tmp,(uint8 *)".edf",meFL_CHECKDOT|meFL_USESRCHPATH,fname))
    {
        meStrcpy(fname,tmp) ;
        found = 0 ;
    }
    else
        found = 1 ;
    dict = dictHead ;
    while(dict != NULL)
    {
        if(!fnamecmp(dict->fname,fname))
            return dict ;
        dict = dict->next ;
    }
    if(!(flag & 1) ||
       (!found && !(flag & 2)))
    {
        mlwrite(MWABORT,(uint8 *)"[Failed to find dictionary %s]",fname) ;
        return NULL ;
    }
    if(((dict = (meDICTIONARY *) meMalloc(sizeof(meDICTIONARY))) == NULL) ||
       ((dict->fname = meStrdup(fname)) == NULL))
        return NULL ;
    dict->table = NULL ;
    dict->flags = 0 ;
    dict->next = dictHead ;
    dictHead = dict ;
    if(!found || (flag & 4))
    {
        meDICTADDR        *table ;
        uint32   tableSize, dSize, dUsed ;
        tableSize = tableSizes[0] ;
        dUsed = sizeof(meDICTADDR)*tableSize ;
        dSize = dUsed + TBLINCSIZE ;
        if((table = (meDICTADDR *) meMalloc(dSize)) == NULL)
            return NULL ;
        memset(table,0,dUsed) ;
        dict->dSize = dSize ;
        dict->dUsed = dUsed ;
        dict->table = table ;
        dict->noWords = 0 ;
        dict->tableSize = tableSize ;
        dict->flags = (DTACTIVE | DTCREATE) ;
    }
    return dict ;
}

static void
wordDump(meDICTWORD *ent, uint8 *buff)
{
    uint8  *ss=buff ;
    int len ;
    
    len = meWordGetWordLen(ent) ;
    memcpy(ss,meWordGetWord(ent),len) ;
    ss += len ;
    if((len = meWordGetFlagLen(ent)) > 0)
    {
        if(meWordGetErrorFlag(ent))
            *ss++ = '>' ;
        else
            *ss++ = '/' ;
        memcpy(ss,meWordGetFlag(ent),len) ;
        ss += len ;
    }
    *ss = '\0' ;
}

int
addDict(int f, int n)
{
    meDICTIONARY *dict ;
    
    if     (n == 0) f = 7 ;
    else if(n >  0) f = 3 ;
    else            f = 1 ;
    if((dict = findDictionary(f)) == NULL)
        return FALSE ;
    if(n < 0)
    {
        meDICTADDR    *tbl ;
        uint32  tableSize, ii, off ;
        
        if(meModeTest(curbp->b_mode,MDVIEW))
            /* don't allow character insert if in read only */
            return (rdonly()) ;
        if(initDictionaries() != TRUE)
            return ABORT ;
        
        lsinsert(0,(uint8 *)"Dictionary: ") ;
        lsinsert(0,dict->fname) ;
        lnewline() ;
        lnewline() ;
        
        tableSize = dict->tableSize ;
        tbl = dict->table ;
        curwp->w_doto = 0 ;
        for(ii=0 ; ii<tableSize ; ii++)
        {
            off = meEntryGetAddr(tbl[ii]) ;
            while(off != 0)
            {
                meDICTWORD    *ent ;
                uint8  buff[MAXBUF] ;
                
                ent = (meDICTWORD *) (off + (uint32) tbl) ;
                off = meWordGetAddr(ent) ;
                if(meWordGetWord(ent)[0] != '\0')
                {
                    wordDump(ent,buff) ;
                    lsinsert(0,buff) ;
                    lnewline() ;
                }
            }
        }
    }
    return TRUE ;
}


int
addSpellRule(int f, int n)
{
    meSPELLRULE  *rr ;
    if(n == 0)
    {
        uint8 *ss=NULL ;
        for(n=0 ; n<NOSPELLRULES+1 ; n++)
        {
            while((rr=meRuleTable[n]) != NULL)
            {
                meRuleTable[n] = rr->next ;
                if(ss != rr->ending)
                {
                    ss = rr->ending ;
                    free(ss) ;
                }
                meNullFree(rr->remove) ;
                meNullFree(rr->append) ;
                free(rr) ;
            }
            meRuleFlags[n] = 0 ;
        }
        hyphenCheck = 1 ;
        maxScore = meMAXSCORE ;
        longestPrefixChange = 0 ;
        /* reset the find-words */
        if(sfwCurMask != NULL)
        {
            free(sfwCurMask) ;
            sfwCurMask = NULL ;
        }
    }
    else
    {
        meSPELLRULE   *pr ;
        uint8 *ss, cc, bb ;
        uint8 buff[MAXBUF] ;
        int rule ;
        
        if((rule = mlCharReply((uint8 *)"Rule flag: ",0,NULL,NULL)) < 0)
            return ABORT ;
        if((f == FALSE) && (rule == '-'))
        {
            if((rule = mlyesno((uint8 *)"Enable hyphen")) == ABORT)
                return ABORT ;
            hyphenCheck = rule ;
            return TRUE ;
        }
        else if((f == FALSE) && (rule == '#'))
        {
            if(mlreply((uint8 *)"Guess score",MLNOSPACE,0,buff,MAXBUF) != TRUE)
                return ABORT ;
            maxScore = meAtoi(buff) ;
            return TRUE ;
        }
        if(rule == '*')
        {
            rule = NOSPELLRULES ;
            if(((rr = meMalloc(sizeof(meSPELLRULE))) == NULL) || 
               (mlreply((uint8 *)"Rule",MLNOSPACE,0,buff,MAXBUF) != TRUE) ||
               ((rr->ending = meStrdup(buff)) == NULL))
                return ABORT ;
            rr->remove = NULL ;
            rr->append = NULL ;
            rr->removeLen = 0 ;
            rr->appendLen = 0 ;
            rr->changeLen = 0 ;
            rr->endingLen = 0 ;
        }
        else
        {
            if((rule != '*') && ((rule < FRSTSPELLRULE) || (rule > LASTSPELLRULE) || (rule == '_')))
                return mlwrite(MWABORT,(uint8 *)"[Invalid spell rule flag]") ;
            
            rule -= FRSTSPELLRULE ;
            if(((rr = meMalloc(sizeof(meSPELLRULE))) == NULL) || 
               (mlreply((uint8 *)"Ending",MLNOSPACE,0,buff,MAXBUF) != TRUE) ||
               ((rr->ending = (uint8 *) meStrdup(buff)) == NULL) ||
               (mlreply((uint8 *)"Remove",MLNOSPACE,0,buff,MAXBUF) != TRUE) ||
               ((rr->remove = (uint8 *) meStrdup(buff)) == NULL) ||
               (mlreply((uint8 *)"Append",MLNOSPACE,0,buff,MAXBUF) != TRUE) ||
               ((rr->append = (uint8 *) meStrdup(buff)) == NULL))
                return ABORT ;
            rr->removeLen = meStrlen(rr->remove) ;
            rr->appendLen = meStrlen(rr->append) ;
            rr->changeLen = rr->appendLen - rr->removeLen ;
            f = 0 ;
            bb = 1 ;
            ss = rr->ending ;
            while((cc=*ss++) != '\0')
            {
                if(cc == '[')
                    bb = 0 ;
                else if(cc == ']')
                    bb = 1 ;
                if(bb)
                    f++ ;
            }
            if(!bb)
                return mlwrite(MWABORT,(uint8 *)"[Spell rule - ending has no closing ']']") ;
            rr->endingLen = f ;
            if(n < 0)
            {
                meRuleFlags[rule] = RULE_PREFIX ;
                if(rr->changeLen > longestPrefixChange)
                    longestPrefixChange = rr->changeLen ;
                n = -n ;
            }
            else
                meRuleFlags[rule] = RULE_SUFFIX ;
            if(n & 2)
                meRuleFlags[rule] |= RULE_MIXABLE ;
        }
        if((pr = meRuleTable[rule]) == NULL)
        {
            rr->next = meRuleTable[rule] ;
            meRuleTable[rule] = rr ;
        }
        else
        {
            for(;; pr = pr->next)
            {
                if(!meStrcmp(rr->ending,pr->ending))
                {
                    free(rr->ending) ;
                    rr->ending = pr->ending ;
                    break ;
                }
                if(pr->next == NULL)
                    break ;
            }
            rr->next = pr->next ;
            pr->next = rr ;
        }
    }
    return TRUE ;
}

static int
saveDictionary(meDICTIONARY *dict, int n)
{
    REGHANDLE   reg ;
    int         ii ;
    
    if(!(dict->flags & DTCHNGD))
        return TRUE ;
    
    /* Never auto-save created dictionaries */
    if((dict->flags & DTCREATE) ||
       ((n & 0x01) && 
        (((reg=regFind(NULL,(uint8 *)SP_REG_ROOT "/" SP_REGI_SAVE))==NULL) || (regGetLong(reg,0) == 0))))
    {
        uint8 prompt[MAXBUF] ;
        int  ret ;
        meStrcpy(prompt,"Save dictionary ") ;
        meStrcat(prompt,dict->fname) ;
        if((ret = mlyesno(prompt)) == ABORT)
            return ctrlg(FALSE,1) ;
        if(ret == FALSE)
            return TRUE ;
        if(dict->flags & DTCREATE)
        {
            uint8 fname[FILEBUF], *pp, *ss ;
            /* if the ctreated dictionary does not have a complete path then get one */
            if(meStrrchr(dict->fname,DIR_CHAR) == NULL)
            {
                ss = dict->fname ;
                if(inputFileName((uint8 *)"Save to directory",fname,1) != TRUE)
                    return ABORT ;
                pp = fname + meStrlen(fname) ;
                if(pp[-1] != DIR_CHAR)
                    *pp++ = DIR_CHAR ;
                meStrcpy(pp,ss) ;
            }
            else
                meStrcpy(fname,dict->fname) ;
            
            if(((ii=meStrlen(fname)) < 4) ||
#ifdef _INSENSE_CASE
               meStricmp(fname+ii-4,".edf")
#else
               meStrcmp(fname+ii-4,".edf")
#endif
               )
                meStrcpy(fname+ii,".edf") ;
            free(dict->fname) ;
            dict->fname = meStrdup(fname) ;
        }
    }
    if((ii=ffWriteFileOpen(dict->fname,0,NULL)) == TRUE)
    {    
        uint8 header[8] ;
                  
        header[0] = 0xED ;
        header[1] = 0xF1 ;
        meEntrySetAddr(header+2,dict->noWords) ;
        meEntrySetAddr(header+5,dict->tableSize) ;
        
        if((ii=ffWriteFileWrite(8,header,0)) == TRUE)
           ii = ffWriteFileWrite(dict->dUsed,(uint8 *) dict->table,0) ;
        ffWriteFileClose(dict->fname,0,NULL) ;
        if(ii == TRUE)
        {
            dict->flags &= ~DTCHNGD ;
            return TRUE ;
        }
    }
    return mlwrite(MWABORT|MWPAUSE,(uint8 *)"[Failed to write dictionary %s]",dict->fname) ;
}

int
saveDict(int f, int n)
{
    meDICTIONARY   *dict ;
    
    if(n & 0x02)
    {
        dict = dictHead ;
        while(dict != NULL)
        {
            if(saveDictionary(dict,n) != TRUE)
                return ABORT ;
            dict = dict->next ;
        }
    }
    else
    {
        /* when saving a single disable the prompt */
        if(((dict = findDictionary(0)) == NULL) ||
           (saveDictionary(dict,0) != TRUE))
            return FALSE ;
    }

    return TRUE ;
}

static void
freeDictionary(meDICTIONARY *dict)
{
    meNullFree(dict->table) ;
    meNullFree(dict->fname) ;
    free(dict) ;
}

/* deleteDict
 * 
 * Argument is a bitmask where:
 * 
 * 0x01 prompt before losing changes
 * 0x02 delete ignore dictionary
 * 0x04 delete all non-ignore dictionaries
 */
int
deleteDict(int f, int n)
{
    /* reset the find-words */
    if(sfwCurMask != NULL)
    {
        free(sfwCurMask) ;
        sfwCurMask = NULL ;
    }
    if(n & 0x04)
    {
        if(dictIgnr != NULL)
        {
            freeDictionary(dictIgnr) ;
            lastIgnr = NULL ;
            dictIgnr = NULL ;
        }
        if(!(n & 0x02))
            /* just remove the ignore */
            return TRUE ;
    }
    
    if(n & 0x02)
    {
        meDICTIONARY *dd ;
        
        while((dd=dictHead) != NULL)
        {
            if((n & 0x01) && (saveDictionary(dd,0x01) != TRUE))
                return FALSE ;
            dictHead = dd->next ;
            freeDictionary(dd) ;
        }
    }
    else
    {
        meDICTIONARY *dict, *dd ;
    
        if((dict = findDictionary(0)) == NULL)
            return FALSE ;
        
        if((n & 0x01) && (saveDictionary(dict,0x01) != TRUE))
            return FALSE ;
        if(dict == dictHead)
            dictHead = dict->next ;
        else
        {
            dd = dictHead ;
            while(dd->next != dict)
                dd = dd->next ;
            dd->next = dict->next ;
        }
        freeDictionary(dict) ;
    }
    return TRUE ;
}


static int
getCurWord(meWORDBUF word)
{
    register uint8  *dp, c, alnm, nalnm=1 ;
    register int sz=0, alphaCnt=0 ;
    
    dp = (uint8 *) word ;
    do
    {
        alnm = nalnm ;
        c = lgetc(curwp->w_dotp,curwp->w_doto) ;
        curwp->w_doto++ ;
        *dp++ = c ;
        if(isAlpha(c))
            alphaCnt = 1 ;
        nalnm = isAlphaNum(c) ;
    } while((++sz < meMAXWORDLEN) && 
            (nalnm || (alnm && isSpllExt(c)))) ;
    
    dp[-1] = 0 ;
    curwp->w_doto-- ;
    if(!alphaCnt)
        return -1 ;
    
    return sz - 1 ;
}

#define wordAddPrefixRule(bwd,rr)         memcpy(bwd,rr->append,rr->appendLen) ;
#define wordRemoveAddPrefixRule(bwd,rr)                                          \
do {                                                                             \
    if(rr->removeLen)                                                            \
        memcpy(bwd,rr->remove, rr->removeLen) ;                                  \
} while(0)

#define wordGetSuffixRuleEnd(wd,wlen,rr)  (wd+wlen-rr->removeLen)
#define wordAddSuffixRule(ewd,rr)         strcpy((char *) ewd,(char *) rr->append)
#define wordRemoveAddSuffixRule(ewd,rr)   strcpy((char *) ewd,(char *) rr->remove)

static meDICTWORD *
wordTrySuffixes(uint8 *word, int wlen, uint8 prefixRule)
{
    meDICTIONARY  *dict ;
    meSPELLRULE   *rr ;
    meDICTWORD    *wd ;
    uint8  flags ;
    int ii ;
    
    flags = (prefixRule == 0) ? RULE_SUFFIX:(RULE_SUFFIX|RULE_MIXABLE) ;
    
    /* Try all the suffix rules to see if we can derive the word from another */
    for(ii=0 ; ii<NOSPELLRULES ; ii++)
    {
        if((meRuleFlags[ii] & flags) == flags)
        {
            rr = meRuleTable[ii] ;
            while(rr != NULL)
            {
                uint8 cc ;
                int alen, nlen, clen, elen ;
                clen = rr->changeLen ;
                elen = rr->endingLen ;
                if(wlen >= elen+clen)
                {
                    nlen = wlen - clen ;
                    alen = meStrlen(rr->append) ;
                    if(!meStrncmp(rr->append,word+wlen-alen,alen))
                    {
                        cc = word[nlen] ;
                        wordRemoveAddSuffixRule(word+wlen-alen,rr) ;
                        if((elen == 0) || (regexStrCmp(word+nlen-elen,rr->ending,1) > 0))
                        {
                            dict = dictHead ;
                            while(dict != NULL)
                            {
                                if((wd=lookupWord(dict,word,nlen)) != NULL)
                                {
                                    /* check that the word found allows this rule */
                                    uint8 *flag, *sflag, ff ;
                                    int len, slen ;
                                    
                                    sflag = flag = meWordGetFlag(wd) ;
                                    slen = len = meWordGetFlagLen(wd) ;
                                    while(--len >= 0)
                                    {
                                        if((ff=*flag++) == '_')
                                        {
                                            sflag = flag ;
                                            slen = len ;
                                        }
                                        else if(ff == ii+FRSTSPELLRULE)
                                        {
                                            if(prefixRule == 0)
                                            {
                                                /* no prefix - found it, return */
                                                word[nlen] = cc ;
                                                return wd ;
                                            }
                                            /* check for the prefix rule as well if given */
                                            while(--slen >= 0)
                                            {
                                                if((ff=*sflag++) == '_')
                                                    break ;
                                                if(ff == prefixRule)
                                                {
                                                    /* found it, return */
                                                    word[nlen] = cc ;
                                                    return wd ;
                                                }
                                            }
                                        }
                                    }
                                }
                                dict = dict->next ;
                            }
                        }
                        word[nlen] = cc ;
                        memcpy(word+wlen-alen,rr->append,alen) ;
                    }
                }
                rr = rr->next ;
            }
        }
    }
    return NULL ;
}

static meDICTWORD *
wordTryPrefixes(uint8 *word, int wlen)
{
    meDICTIONARY *dict ;
    meSPELLRULE  *rr ;
    meDICTWORD   *wd ;
    int ii ;
    
    /* Now try all the prefix rules to see if we can derive the word */
    for(ii=0 ; ii<NOSPELLRULES ; ii++)
    {
        if(meRuleFlags[ii] & RULE_PREFIX)
        {
            rr = meRuleTable[ii] ;
            while(rr != NULL)
            {
                uint8 *nw, cc ;
                int nlen, elen ;
                
                if((wlen >= rr->appendLen) &&
                   !meStrncmp(rr->append,word,rr->appendLen))
                {
                    nlen = wlen - rr->changeLen ;
                    nw = word + rr->changeLen ;
                    wordRemoveAddPrefixRule(nw,rr) ;
                    if((elen=rr->endingLen) > 0)
                    {
                        cc = nw[elen] ;
                        nw[elen] = '\0' ;
                        if(regexStrCmp(nw,rr->ending,1) <= 0)
                        {
                            nw[elen] = cc ;
                            wordAddPrefixRule(word,rr) ;
                            rr = rr->next ;
                            continue ;
                        }
                        nw[elen] = cc ;
                    }
                    /* Try the word on its own first */
                    dict = dictHead ;
                    while(dict != NULL)
                    {
                        if((wd=lookupWord(dict,nw,nlen)) != NULL)
                        {
                            /* check that the word found allows this rule */
                            uint8 *flag ;
                            int len ;
                            
                            flag = meWordGetFlag(wd) ;
                            len = meWordGetFlagLen(wd) ;
                            while(--len >= 0)
                                if(*flag++ == ii+FRSTSPELLRULE)
                                    /* yes, lets get out of here */
                                    return wd ;
                        }
                        dict = dict->next ;
                    }
                    if(meRuleFlags[ii] & RULE_MIXABLE)
                    {
                        /* Now try all the suffixes for this prefix */
                        if((wd = wordTrySuffixes(nw,nlen,(uint8) (ii+FRSTSPELLRULE))) != NULL)
                            /* yes, lets get out of here */
                            return wd ;
                    }
                    wordAddPrefixRule(word,rr) ;
                }
                rr = rr->next ;
            }
        }
    }
    return NULL ;
}

static int
wordTrySpecialRules(uint8 *word, int wlen)
{
    uint8 cc ;
    meSPELLRULE  *rr ;
    
    if((rr = meRuleTable[NOSPELLRULES]) != NULL)
    {
        /* Now try all the special rules - e.g. [0-9]*1st for 1st, 21st etc */
        cc = word[wlen] ;
        word[wlen] = '\0' ;
        while(rr != NULL)
        {
            if(regexStrCmp(word,rr->ending,1) > 0)
            {
                word[wlen] = cc ;
                return TRUE ;
            }
            rr = rr->next ;
        }
        word[wlen] = cc ;
    }
    return FALSE ;
}

/* checkWordSearch
 * 
 * Looks in all dictionaries for the given word, then tries all
 * the rules. Returns:
 *   FALSE  Word not found
 *   TRUE   found and word is good
 *   ABORT  found and word is erroneous
 */
static int
checkWordSearch(uint8 *word, int wlen)
{
    meDICTIONARY *dict ;
    meDICTWORD   *wd ;
    
    if((wd=lookupWord(dictIgnr,word,wlen)) == NULL)
    {
        dict = dictHead ;
        while(dict != NULL)
        {
            if((wd=lookupWord(dict,word,wlen)) != NULL)
                break ;
            dict = dict->next ;
        }
        if(wd == NULL)
        {
            /* Now try all the suffixes on the original word */
            wd = wordTrySuffixes(word,wlen,0) ;
        
            if(wd == NULL)
            {
                /* Now try all the prefixes on the original word */
                wd = wordTryPrefixes(word,wlen) ;
                if(wd == NULL)
                    return wordTrySpecialRules(word,wlen) ;
            }
        }
    }
    wordCurr = wd ;
    if(meWordGetErrorFlag(wd))
        return ABORT ;
    return TRUE ;
}

/* checkWord
 * 
 * Checks
 * 1) The word is a word
 * 2) Tries checkWordSearch
 * 3) If upper case then capitalises and tries checkWordSearch
 * 4) If capitalised the makes lower case and tries checkWordSearch
 * 
 * If all the above fail then it fails. Returns:
 *   FALSE  Word is okay
 *   TRUE   found and word is good
 *   ABORT  found and word is erroneous
 */
static int
checkWord(uint8 *word, int wlen)
{
    uint8 *ss, cc ;
    int ii ;
    
    if(caseFlags == 0)
        /* No letters in the word -> not a word -> word okay */
        return TRUE ;
    
    if((ii=checkWordSearch(word,wlen)) != FALSE)
        return ii ;
    
    /* If all upper then capitalise */
    if(caseFlags == (SPELL_CASE_FUPPER|SPELL_CASE_CUPPER))
    {
        ii = wlen ;
        ss = word+1 ;
        while(--ii > 0)
        {
            cc = *ss ;
            *ss++ = toggleLatinCase(cc) ;
        }
        if(checkWordSearch(word,wlen) == TRUE)
            return TRUE ;
    }
    if(caseFlags & SPELL_CASE_FUPPER)
    {
        cc = *word ;
        *word = toggleLatinCase(cc) ;
        if(checkWordSearch(word,wlen) == TRUE)
            return TRUE ;
        *word = cc ;
    }
    /* We failed to find it, restore the case if we changed it */
    if(caseFlags == (SPELL_CASE_FUPPER|SPELL_CASE_CUPPER))
    {
        ii = wlen ;
        ss = word+1 ;
        while(--ii > 0)
        {
            cc = *ss ;
            *ss++ = toggleLatinCase(cc) ;
        }
    }
    return FALSE ;
}

/* checkCurWord
 * 
 * Check for the given word in all dictionaries, returns:
 *   FALSE  Word not found
 *   TRUE   found and word is good
 *   ABORT  found and word is erroneous
 */
static int
checkCurWord(uint8 *word)
{
    uint8 *ss ;
    int ii, len, hyphen ;
    
    len = meStrlen(word) ;
    
    if((ii=checkWord(word,len)) != FALSE)
        return ii ;
    if(isSpllExt(word[len-1]))
    {
        len-- ;
        if(checkWord(word,len) == TRUE)
            return TRUE ;
    }
    if(hyphenCheck)
    {
        /* check for hyphenated words such as wise-cracks */
        for(ss=word,hyphen=0 ; len ; len--,ss++)
        {
            if(*ss == '-')
            {
                if((ss != word) &&
                   (checkWord(word,(uint8)(ss-word)) != TRUE))
                    return FALSE ;
                word = ss+1 ;
                hyphen = 1 ;
            }
        }
        if(hyphen && 
           (((len=ss-word)==0) || (checkWord(word,(uint8)len) == TRUE)))
        {
            wordCurr = NULL ;
            return TRUE ;
        }
    }
    return FALSE ;
}    

static uint8 *
wordApplyPrefixRule(uint8 *wd, meSPELLRULE *rr)
{
    uint8 *nw, cc ;
    int len ;
    
    if((len=rr->endingLen) != 0)
    {
        cc = wd[len] ;
        wd[len] = '\0' ;
        if(regexStrCmp(wd,rr->ending,1) <= 0)
        {
            wd[len] = cc ;
            return NULL ;
        }
        wd[len] = cc ;
    }
    nw = wd - rr->changeLen ;
    wordAddPrefixRule(nw,rr) ;
    return nw ;
}

static uint8 *
wordApplySuffixRule(uint8 *wd, int wlen, meSPELLRULE *rr)
{
    uint8 *ew, len ;
    
    ew = wd+wlen ;
    if((len=rr->endingLen) != 0)
    {
        if((len > wlen) || (regexStrCmp(ew-len,rr->ending,1) <= 0))
            return NULL ;
    }
    ew -= rr->removeLen ;
    wordAddSuffixRule(ew,rr) ;
    return ew ;
}

static int
wordTestSuffixRule(uint8 *wd, int wlen, meSPELLRULE *rr)
{
    uint8 len ;
    
    if(((len=rr->endingLen) != 0) &&
       ((len > wlen) || (regexStrCmp(wd+wlen-len,rr->ending,1) <= 0)))
       return 0 ;
    return 1 ;
}


#if 0
#define __LOG_FILE
#endif

#ifdef __LOG_FILE
FILE *fp ;
#endif

/* wordGetGuessScore
 * 
 * Given the user source word (swd) and a dictionary word (dwd) the 
 */
static int
wordGetGuessScore(uint8 *swd, int slen, uint8 *dwd, int dlen, int testFlag)
{
    int  scr=0 ;
    uint8 cc, lcc='\0', dd, ldd='\0' ;
    
#ifdef __LOG_FILE
    {
        cc = swd[slen] ;
        swd[slen] = '\0' ;
        dd = dwd[dlen] ;
        dwd[dlen] = '\0' ;
        fprintf(fp,"WordScore [%s] with [%s] ",swd,dwd) ;
        swd[slen] = cc ;
        dwd[dlen] = dd ;
    }
#endif
    for(;;)
    {
        if(slen == 0)
        {
            while(--dlen >= 0)
            {
                dd = *dwd++ ;
                if(!isSpllExt(dd))
                    /* ignore missed ' s etc */
                    scr += 22 ;
            }
            break ;
        }
        if(dlen == 0)
        {
            while(--slen >= 0)
            {
                cc = *swd++ ;
                if(!isSpllExt(cc))
                    /* ignore missed ' s etc */
                    scr += 25 ;
            }
            break ;
        }
        cc = *swd ;
        dd = *dwd ;
        dd = toLatinLower(dd) ;
        if(dd == cc) 
        {
            swd++ ;
            slen-- ;
            lcc = cc ;
            dwd++ ;
            dlen-- ;
            ldd = dd ;
            continue ;
        }
        if((dlen > 1) && (dwd[1] == cc))
        {
            if((slen > 1) && (swd[1] == dd) && 
               ((dlen <= slen) || (dwd[2] != dd)))
            {
                /* transposed chars */
                scr += 23 ;
                swd += 2 ;
                slen -= 2 ;
                lcc = dd ;
            }
            else
            {
                if(ldd == dd)
                    /* missing double letter i.e. leter -> letter */
                    scr += 21 ;
                else if(!isSpllExt(dd))
                    /* missed letter i.e. ltter -> letter, ignore missed ' s etc */
                    scr += 22 ;
                swd++ ;
                lcc = cc ;
                if(testFlag && (slen > 1))
                    /* if this is only a test on part of the word the adjust the
                     * other words length as well to keep the end point aligned
                     * this is because of base word tests. when testing the base
                     * of anomaly with anommoly the comparison is between [anommol]
                     * with [anomaly] the realignment caused by the double m means
                     * that it should be between [anommol] and [anomal]
                     */
                    slen -= 2 ;
                else
                    slen-- ;
            }
            dwd += 2 ;
            dlen -= 2 ;
            ldd = cc ;
        }
        else if((slen > 1) && (swd[1] == dd))
        {
            if(lcc == cc)
                /* extra double letter i.e. vowwel -> vowel */
                scr += 24 ;
            else if(!isSpllExt(cc))
                /* extra letter i.e. lertter -> letter, ignore an extra ' etc. */
                scr += 25 ;
            swd += 2 ;
            slen -= 2 ;
            lcc = dd ;
            dwd++ ;
            ldd = dd ;
            if(testFlag && (dlen > 1))
                /* if this is only a test on part of the word the adjust the
                 * other words length as well to keep the end point aligned
                 * this is because of base word tests. when testing the base
                 * of anomaly with anommoly the comparison is between [anommol]
                 * with [anomaly] the realignment caused by the double m means
                 * that it should be between [anommol] and [anomal]
                 */
                dlen -= 2 ;
            else
                dlen-- ;
        }
        else
        {
            scr += 27 ;
            swd++ ;
            slen-- ;
            lcc = cc ;
            dwd++ ;
            dlen-- ;
            ldd = dd ;
        }
        if(scr >= maxScore)
            break ;
    }
#ifdef __LOG_FILE
    fprintf(fp,"scr = %d, len %d\n",scr,dlen-slen) ;
#endif
    return (scr >= maxScore) ? maxScore:scr ;
}

static void
wordAddToGuessList(uint8 *word, int curScr,
                   meWORDBUF *words, int *scores)
{
    int ii, jj ;
    
    for(ii=0 ; ii<meMAXGUESSES ; ii++)
    {
        if(curScr < scores[ii])
        {
            for(jj=meMAXGUESSES-2 ; jj>=ii ; jj--)
            {
                scores[jj+1] = scores[jj] ;
                meStrcpy(words[jj+1],words[jj]) ;
            }
            scores[ii] = curScr ;
            meStrcpy(words[ii],word) ;
            break ;
        }
        /* check for duplicate */
        if((curScr == scores[ii]) && !meStrcmp(words[ii],word))
            break ;
    }
}

static int
wordAddSuffixesToGuessList(uint8 *sword, int slen,
                           uint8 *bword, int blen,
                           uint8 *flags, int noflags,
                           meWORDBUF *words, int *scores, 
                           int bscore, int pflags)
{
    meSPELLRULE   *rr ;
    uint8 *nwd, *lend=NULL ;
    int ii ;
    
    /* try all the allowed suffixes */
    while(--noflags >= 0)
    {
        ii = flags[noflags] - FRSTSPELLRULE ;
        if(((bscore+meRuleScore[ii]) < scores[meMAXGUESSES-1]) &&
           ((meRuleFlags[ii] & pflags) == pflags))
        {
            rr = meRuleTable[ii] ;
            while(rr != NULL)
            {
                if(bscore+rr->rule < scores[meMAXGUESSES-1])
                {
                    int curScr, nlen ;
                    if(lend != rr->ending)
                    {
                        lend = rr->ending ;
                        if(!wordTestSuffixRule(bword,blen,rr))
                        {
                            do {
                                rr = rr->next ;
                            } while((rr != NULL) && (rr->ending == lend)) ;
                            continue ;
                        }
                    }
                    nwd = wordGetSuffixRuleEnd(bword,blen,rr) ;
                    wordAddSuffixRule(nwd,rr) ;
                    nlen = blen+rr->changeLen ;
                    if((curScr = wordGetGuessScore(sword,slen,bword,nlen,0)) < maxScore)
                        wordAddToGuessList(bword,curScr,words,scores) ;
                    wordRemoveAddSuffixRule(nwd,rr) ;
                    if(TTbreakTest(0))
                        return 1 ;
                }
                rr = rr->next ;
            }
        }
    }
    return 0 ;
}


static int
wordScoreSuffixes(uint8 *word, int olen)
{
    meSPELLRULE *rr ;
    int ii, jj, scr, rscr, bscr=maxScore ;
    
    longestSuffixRemove = 0 ;
    if(isSpllExt(word[olen-1]))
        /* if the last letter is a '.' then ignore it, i.e. "anommolies."
         * should compare suffixes with the "ies", not "es."
         */
        olen-- ;
    for(ii=0 ; ii<NOSPELLRULES ; ii++)
    {
        if(meRuleFlags[ii] & RULE_SUFFIX)
        {
            rscr = maxScore ;
            rr = meRuleTable[ii] ;
            while(rr != NULL)
            {
                jj = rr->appendLen ;
                if((jj < olen) &&
                   ((scr = wordGetGuessScore(word+olen-jj,jj,
                                             rr->append,jj,1)) < maxScore))
                {
                    rr->rule = scr ;
                    if(scr < rscr)
                        rscr = scr ;
                    if(rr->removeLen > longestSuffixRemove)
                        longestSuffixRemove = rr->removeLen ;
                }
                else
                    rr->rule = maxScore ;
                rr = rr->next ;
            }
            meRuleScore[ii] = rscr ;
            if(rscr < bscr)
                bscr = rscr ;
        }
    }
    return bscr ;
}

static int
wordGetGuessList(uint8 *word, int olen, meWORDBUF *words)
{
    meDICTIONARY *dict ;
    meDICTWORD *ent ;
    meWORDBUF wbuff ;
    uint8 *wb, *ww ;
    uint32 ii, off, noWrds ;
    int scores[meMAXGUESSES], curScr, bsufScr ;
    int wlen, mlen ;
    
#ifdef __LOG_FILE
    fp = fopen("log","w+") ;
#endif
    for(noWrds=0 ; noWrds<meMAXGUESSES ; noWrds++)
    {
        scores[noWrds] = maxScore ;
        words[noWrds][0] = '\0' ;
    }
    mlen = (olen < 3) ? olen:3 ;
    /* check for two words with a missing ' ' or '-' */
    if(olen > 1)
    {
        uint8 cc ;
        
        ww = wbuff ;
        for(wlen=1 ; wlen < olen ; wlen++)
        {
            cc = resultStr[wlen] ;
            memcpy(wbuff,resultStr+1,wlen) ;
            wbuff[wlen] = '\0' ;
            if(((wlen == 1) && isSpllExt(cc)) ||
               (checkCurWord(wbuff) == TRUE))
            {
                meStrcpy(wbuff,resultStr+wlen+1) ;
                if(checkCurWord(wbuff) == TRUE)
                {
                    memcpy(wbuff,resultStr+1,wlen) ;
                    meStrcpy(wbuff+wlen+1,resultStr+wlen+1) ;
                    wbuff[wlen] = ' ' ;
                    wordAddToGuessList(wbuff,22,words,scores) ;
                    if(hyphenCheck && !isSpllExt(cc))
                    {
                        wbuff[wlen] = '-' ;
                        wordAddToGuessList(wbuff,22,words,scores) ;
                    }
                    if(scores[meMAXGUESSES-1] <= 22)
                        break ;
                }
            }
        }
    }
    wb = wbuff+longestPrefixChange ;
    if(caseFlags & (SPELL_CASE_FUPPER|SPELL_CASE_CUPPER))
    {
        uint8 cc ;
        ww = word ;
        while((cc = *ww) != '\0')
            *ww++ = toLatinLower(cc) ;
    }
    bsufScr = wordScoreSuffixes(word,olen) ;
    dict = dictHead ;
    while(dict != NULL)
    {
        for(ii=0 ; ii<dict->tableSize ; ii++)
        {
            off = meEntryGetAddr(dict->table[ii]) ;
            while(off != 0)
            {
                ent = (meDICTWORD *) (off + (uint32) dict->table) ;
                /* Check this word has not been erased or is an error word,
                 * if not then do guess */
                if(!meWordGetErrorFlag(ent) && ((ww=meWordGetWord(ent))[0] != '\0'))
                {
                    int noflags ;
                    
                    wlen = meWordGetWordLen(ent) ;
                    if((noflags=meWordGetFlagLen(ent)) > 0)
                    {
                        uint8 *flags ;
                        meSPELLRULE *rr ;
                        int jj, ff, nlen ;
                        
                        memcpy(wb,ww,wlen) ;
                        wb[wlen] = '\0' ;
                        flags = meWordGetFlag(ent) ;
                        if((jj = wlen - longestSuffixRemove) < mlen)
                            jj = (wlen < mlen) ? wlen:mlen ;
                        else if(jj > olen)
                            jj = olen ;
                        curScr = wordGetGuessScore(word,jj,wb,jj,1) ;
                        if(curScr < scores[meMAXGUESSES-1])
                        {
                            /* try all suffixes on the word, wordAddSuffixesToGuessList returns
                             * true if the user aborted */
                            if(((curScr+bsufScr) < scores[meMAXGUESSES-1]) &&
                               wordAddSuffixesToGuessList(word,olen,wb,wlen,flags,noflags,words,
                                                          scores,curScr,RULE_SUFFIX))
                                return -1 ;
                            if((curScr = wordGetGuessScore(word,olen,wb,wlen,0)) < scores[meMAXGUESSES-1])
                                wordAddToGuessList(wb,curScr,words,scores) ;
                            if(TTbreakTest(0))
                                return -1 ;
                        }
                        /* try all the allowed prefixes */
                        ff = noflags ;
                        while(--ff >= 0)
                        {
                            if((jj = flags[ff]) == '_')
                                noflags = ff ;
                            else if(meRuleFlags[(jj-=FRSTSPELLRULE)] & RULE_PREFIX)
                            {
                                rr = meRuleTable[jj] ;
                                while(rr != NULL)
                                {
                                    if((ww = wordApplyPrefixRule(wb,rr)) != NULL)
                                    {
                                        nlen = wlen+rr->changeLen ;
                                        if((jj = nlen - longestSuffixRemove) < mlen)
                                            jj = (nlen < mlen) ? nlen:mlen ;
                                        else if(jj > olen)
                                            jj = olen ;
                                        curScr = wordGetGuessScore(word,jj,ww,jj,1) ;
                                        if(curScr < scores[meMAXGUESSES-1])
                                        {
                                            /* try all suffixes on the word */ 
                                            if(((curScr+bsufScr) <  scores[meMAXGUESSES-1]) && (meRuleFlags[jj] & RULE_MIXABLE))
                                            {
                                                int f1, f2 ;
                                                for(f1=f2=0 ; f2<ff ; )
                                                    if(flags[f2++] == '_')
                                                        f1 = f2 ;
                                                if(wordAddSuffixesToGuessList(word,olen,ww,nlen,flags+f1,noflags-f1,
                                                                              words,scores,curScr,RULE_SUFFIX|RULE_MIXABLE))
                                                    return -1 ;
                                            }
                                            if((curScr = wordGetGuessScore(word,olen,ww,nlen,0)) < scores[meMAXGUESSES-1])
                                                wordAddToGuessList(ww,curScr,words,scores) ;
                                            if(TTbreakTest(0))
                                                return -1 ;
                                        }
                                        wordRemoveAddPrefixRule(wb,rr) ;
                                    }
                                    rr = rr->next ;
                                }
                            }
                        }
                    }
                    else if((curScr = wordGetGuessScore(word,olen,ww,wlen,0)) < scores[meMAXGUESSES-1])
                    {
                        memcpy(wb,ww,wlen) ;
                        wb[wlen] = '\0' ;
                        wordAddToGuessList(wb,curScr,words,scores) ;
                    }
                }
                off = meWordGetAddr(ent) ;
            }
        }
        dict = dict->next ;
    }
    for(noWrds=0 ; (noWrds<meMAXGUESSES) && (scores[noWrds] < maxScore) ; noWrds++)
        ;
    {
        uint8 cc ;
        cc = word[olen-1] ;
        if(isSpllExt(cc))
        {
            /* if the last letter is a '.' etc then loop through the guesses adding one */
            for(ii=0 ; ii<noWrds ; ii++)
            {
                wlen = meStrlen(words[ii]) ;
                /* must check that its not there already */
                if(words[ii][wlen-1] != cc)
                {
                    words[ii][wlen]   = cc ;
                    words[ii][wlen+1] = '\0' ;
                }
            }
        }
    }
    return (int) noWrds ;
}


#define SPELLWORD_GET     0x01       /* Get the word from the user */
#define SPELLWORD_GETNEXT 0x02       /* Keep going till we come to a problem */
#define SPELLWORD_ADD     0x04       /* Add the given word */
#define SPELLWORD_IGNORE  0x08       /* Add word to the ignore dictionary */
#define SPELLWORD_ERROR   0x10       /* The given word is an erroneous word */
#define SPELLWORD_GUESS   0x20       /* Return the words guest list */
#define SPELLWORD_DERIV   0x40       /* Return the words derivatives */
#define SPELLWORD_DOUBLE  0x80       /* Check for double word */
#define SPELLWORD_INFO    0x100      /* Get info on word */
#define SPELLWORD_DELETE  0x200      /* Delete the given word */

static void
spellWordToLatinFont(uint8 *dd, uint8 *ss)
{
    uint8 cc ;
    cc = *ss++ ;
    if(isUpper(cc))
        caseFlags = SPELL_CASE_FUPPER ;
    else if(isLower(cc))
        caseFlags = SPELL_CASE_FLOWER ;
    else
        caseFlags = 0 ;
    *dd++ = toLatinFont(cc) ;
    while((cc=*ss++) != '\0')
    {
        if(isUpper(cc))
            caseFlags |= SPELL_CASE_CUPPER ;
        else if(isLower(cc))
            caseFlags |= SPELL_CASE_CLOWER ;
        *dd++ = toLatinFont(cc) ;
    }
    *dd = '\0' ;
}


static void
spellWordToUserFont(uint8 *dd, uint8 *ss)
{
    uint8 cc ;
    
    cc = *ss++ ;
    cc = toUserFont(cc) ;
    if((caseFlags & SPELL_CASE_FUPPER) || (caseFlags == (SPELL_CASE_FLOWER|SPELL_CASE_CUPPER)))
        cc = toUpper(cc) ;
    *dd++ = cc ;
    while((cc=*ss++) != '\0')
    {
        cc = toUserFont(cc) ;
        if(caseFlags == (SPELL_CASE_FUPPER|SPELL_CASE_CUPPER))
            cc = toUpper(cc) ;
        *dd++ = cc ;
    }
    *dd = '\0' ;
}

int
spellWord(int f, int n)
{
    meWORDBUF word ;
    int       len ;
    
    if(initDictionaries() != TRUE)
        return ABORT ;
    
    if(n & SPELLWORD_GET)
    {
        if(mlreply((uint8 *)"Enter word", MLNOSPACE,0,resultStr+1,meMAXWORDLEN) != TRUE)
            return ABORT ;
        spellWordToLatinFont(word,resultStr+1) ;
    }
    else if(n & SPELLWORD_GETNEXT)
    {
        uint8  cc, chkDbl, curDbl ;
        uint16 soff, eoff ;
        meWORDBUF lword ;
        
        chkDbl = (n & SPELLWORD_DOUBLE) ;
        curDbl = 0 ;
        while((curwp->w_doto > 0) && isAlphaNum(lgetc(curwp->w_dotp,curwp->w_doto)))
            curwp->w_doto-- ;
        for(;;)
        {
            while(((cc = lgetc(curwp->w_dotp,curwp->w_doto)) != '.') && 
                  !isAlphaNum(cc))
            {
                if(!isSpace(cc))
                    curDbl = 0 ;
                if(WforwChar(curwp, 1) == FALSE)
                {
                    resultStr[0] = 'F' ;
                    resultStr[1] = '\0' ;
                    return TRUE ;
                }
            }
            soff = curwp->w_doto ;
            if(getCurWord((uint8 *) resultStr+1) < 0)
                continue ;
            eoff = curwp->w_doto ;
            if(curDbl && !meStricmp(lword,resultStr+1))
            {
                resultStr[0] = 'D' ;
                setShowRegion(curbp,curwp->line_no,soff,curwp->line_no,eoff) ;
                curwp->w_flag |= WFMOVEL|WFSELHIL ;
                return TRUE ;
            }
            spellWordToLatinFont(word,(uint8 *) resultStr+1) ;
            if(checkCurWord(word) != TRUE)
                break ;
            if(chkDbl)
            {
                /* store the last word for double word check */
                meStrcpy(lword,resultStr+1) ;
                curDbl = 1 ;
            }
        }
        setShowRegion(curbp,curwp->line_no,soff,curwp->line_no,eoff) ;
        curwp->w_flag |= WFMOVEL|WFSELHIL ;
    }
    else
    {
        uint8  cc ;
        uint16 off, soff, eoff ;
        
        soff = off = curwp->w_doto ;
        for(;;soff--)
        {
            cc = lgetc(curwp->w_dotp,soff) ;
            if((soff==0) || isSpllWord(cc))
                    break ;
        }
        if(!isSpllWord(cc))
            return FALSE ;
        while(soff > 0)
        {
            --soff ;
            cc = lgetc(curwp->w_dotp,soff) ;
            if(!isSpllWord(cc))
            {
                soff++ ;
                break ;
            }
            if(!isAlphaNum(cc) && 
               ((soff == 0) || !isAlphaNum(lgetc(curwp->w_dotp,soff-1))))
                break ;
        }
        /* if the first character is not alphanumeric or a '.' then move
         * on one, this stops misspellings of 'quoted words'
         */
        if(((cc = lgetc(curwp->w_dotp,soff)) != '.') && !isAlphaNum(cc))
            soff++ ;
        curwp->w_doto = soff ;
        len = getCurWord((uint8 *) resultStr+1) ;
        eoff = curwp->w_doto ;
        curwp->w_doto = off ;
        setShowRegion(curbp,curwp->line_no,soff,curwp->line_no,eoff) ;
        curwp->w_flag |= WFMOVEL|WFSELHIL ;
        if(len < 0)
        {
            resultStr[0] = 'N' ;
            return TRUE ;
        }
        spellWordToLatinFont(word,(uint8 *) resultStr+1) ;
    }
    len = meStrlen(word) ;
    if(n & SPELLWORD_ADD)
    {
        meDICTADDWORD  wdbuff ;
        meDICTIONARY  *dict ;
        meDICTWORD    *wd= (meDICTWORD *) &wdbuff ;
        
        memcpy(meWordGetWord(wd),word,len) ;
        meWordSetWordLen(wd,len) ;
        
        if(mlreply((uint8 *)"Enter flags", MLNOSPACE,0,word,meMAXWORDLEN) != TRUE)
            return ABORT ;
        len = meStrlen(word) ;
        memcpy(meWordGetFlag(wd),word,len) ;
        /* set flag len directly to clear the ERROR flag */
        wd->flagLen = len ;
        if(n & SPELLWORD_ERROR)
            meWordSetErrorFlag(wd) ;
        if(n & SPELLWORD_IGNORE)
            dict = dictIgnr ;
        else
            dict = dictHead ;
        return dictionaryAddWord(dict,wd) ;
    }
    else if(n & SPELLWORD_DELETE)
    {
        meDICTIONARY  *dict ;
        
        if(n & SPELLWORD_IGNORE)
            dictionaryDeleteWord(dictIgnr,word,len) ;
        else
        {
            dict = dictHead ;
            while(dict != NULL)
            {
                dictionaryDeleteWord(dict,word,len) ;
                dict = dict->next ;
            }
        }
        return TRUE ;
    }
    else if(n & SPELLWORD_DERIV)
    {
        meSPELLRULE  *rr ;
        uint8 buff[MAXBUF], *wd, *nwd, *swd ;
        
        if(n & SPELLWORD_INFO)
        {
            /* dump all derivatives into the current buffer */
            int ii ;
            
            if(meModeTest(curbp->b_mode,MDVIEW))
                /* don't allow character insert if in read only */
                return (rdonly()) ;
            wd = buff+longestPrefixChange ;
            meStrcpy(wd,word) ;
            if(checkCurWord(wd) == TRUE)
            {
                lsinsert(0,wd) ;
                lnewline() ;
            }
            for(ii=0 ; ii<NOSPELLRULES ; ii++)
            {
                f = (meRuleFlags[ii] & RULE_PREFIX) ? 1:0 ;
                rr = meRuleTable[ii] ;
                while(rr != NULL)
                {
                    if(f)
                        nwd = wordApplyPrefixRule(wd,rr) ;
                    else if((swd = wordApplySuffixRule(wd,len,rr)) != NULL)
                        nwd = wd ;
                    else
                        nwd = NULL ;
                    if(nwd != NULL)
                    {
                        meStrcpy(word,nwd) ;
                        if(checkCurWord(word) == TRUE)
                        {
                            lsinsert(0,nwd) ;
                            lnewline() ;
                            if(meRuleFlags[ii] == (RULE_PREFIX|RULE_MIXABLE))
                            {
                                meSPELLRULE  *sr ;
                                int jj, ll ;
                                ll = len + rr->changeLen ;
                                for(jj=0 ; jj<NOSPELLRULES ; jj++)
                                {
                                    if(meRuleFlags[jj] == (RULE_SUFFIX|RULE_MIXABLE))
                                    {                                    
                                        sr = meRuleTable[jj] ;
                                        while(sr != NULL)
                                        {
                                            if((swd = wordApplySuffixRule(nwd,ll,sr)) != NULL)
                                            {
                                                meStrcpy(word,nwd) ;
                                                if(checkCurWord(word) == TRUE)
                                                {
                                                    lsinsert(0,nwd) ;
                                                    lnewline() ;
                                                }
                                                wordRemoveAddSuffixRule(swd,sr) ;
                                            }
                                            sr = sr->next ;
                                        }
                                    }
                                }
                            }
                        }
                        if(f)
                            wordRemoveAddPrefixRule(wd,rr) ;
                        else
                            wordRemoveAddSuffixRule(swd,rr) ;
                    }
                    rr = rr->next ;
                }
            }
            return TRUE ;
        }
        if(((f = mlCharReply((uint8 *)"Rule flag: ",0,NULL,NULL)) < FRSTSPELLRULE) || (f > LASTSPELLRULE))
            return ABORT ;
        f -= FRSTSPELLRULE ;
        wd = buff+longestPrefixChange ;
        meStrcpy(wd,word) ;

        rr = meRuleTable[f] ;
        f = (meRuleFlags[f] & RULE_PREFIX) ? 1:0 ;
        while(rr != NULL)
        {
            if(f)
                nwd = wordApplyPrefixRule(wd,rr) ;
            else if((nwd = wordApplySuffixRule(wd,len,rr)) != NULL)
                nwd = wd ;
            if(nwd != NULL)
            {
                spellWordToUserFont((uint8 *) resultStr,nwd) ;
                return TRUE ;
            }
            rr = rr->next ;
        }
        resultStr[0] = '\0' ;
        return TRUE ;
    }
    else if(n & SPELLWORD_GUESS)
    {
        meWORDBUF words[meMAXGUESSES] ;
        uint8 *ss, *dd ;
        int nw, cw, ll, ii ;
        
        if((nw = wordGetGuessList(word,len,words)) < 0)
            return ABORT ;
        dd = (uint8 *) resultStr ;
        *dd++ = '|' ;
        cw = 0 ;
        ll = MAXBUF-3 ;
        while((cw < nw) && ((ii=meStrlen((ss=words[cw++]))) < ll))
        {
            spellWordToUserFont(dd,ss) ;
            dd += ii ;
            *dd++ = '|' ;
            ll -= ii+1 ;
        }
        *dd = '\0' ;
        return TRUE ;
    }
    wordCurr = NULL ;
    if(((f=checkCurWord(word)) == ABORT) && (wordCurr != NULL))
    {
        resultStr[0] = 'A' ;
        f = meWordGetFlagLen(wordCurr) ;
        memcpy(resultStr+1,meWordGetFlag(wordCurr),f) ;
        resultStr[f+1] = '\0' ;
    }
    else if(f == TRUE)
        resultStr[0] = 'O' ;
    else
        resultStr[0] = 'E' ;
    if((n & SPELLWORD_INFO) && (wordCurr != NULL))
        wordDump(wordCurr,(uint8 *) resultStr+1) ;
    return TRUE ;
}


void
findWordsInit(uint8 *mask)
{
    if(sfwCurMask != NULL)
    {
        free(sfwCurMask) ;
        sfwCurMask = NULL ;
    }
    if(initDictionaries() != TRUE)
        return ;
    
    sfwCurWord = NULL ;
    sfwCurDict = dictHead ;
    sfwPreRule = NULL ;
    sfwSufRule = NULL ;
    if((sfwCurMask = meMalloc(meStrlen(mask)+1)) != NULL)
    {
        uint8 *ww, cc ;
        spellWordToLatinFont(sfwCurMask,mask) ;
        /* must convert to lower as the regexp works in user font */
        ww = sfwCurMask ;
        while((cc = *ww) != '\0')
            *ww++ = toLatinLower(cc) ;
        sfwFlags = (((cc = *sfwCurMask) == '.') || (cc == '\\') || (cc == '[') || (cc == '^')) ;
    }
}

uint8 *
findWordsNext(void)
{
    uint32 off ;
    uint8 *wp, *pwp, *swp, *ww, *flags ;
    int len, flen, preRule, sufRule ;
    
    if(sfwCurMask == NULL)
        return abortm ;
    wp = evalResult+longestPrefixChange ;
    if(sfwCurWord != NULL)
    {
        /* Get the word */
        len = meWordGetWordLen(sfwCurWord) ;
        ww = meWordGetWord(sfwCurWord) ;
        for(flen=0 ; flen<len ; flen++)
            wp[flen] = toLatinLower(ww[flen]) ;
        wp[len] = '\0' ;
        /* Get the flags */
        flen = meWordGetFlagLen(sfwCurWord) ;
        flags = meWordGetFlag(sfwCurWord) ;
        if(sfwPreRule != NULL)
        {
            pwp = wordApplyPrefixRule(wp,sfwPreRule) ;
            len += sfwPreRule->changeLen ;
            preRule = sfwPreRule->rule ;  
            if(sfwSufRule != NULL)
            {
                sufRule = sfwSufRule->rule ;  
                goto sfwJumpGotPreGotSuf ;
            }
            goto sfwJumpGotPreNotSuf ;
        }
        if(sfwSufRule != NULL)
        {
            sufRule = sfwSufRule->rule ;  
            goto sfwJumpNotPreGotSuf ;
        }
        goto sfwJumpNotPreNotSuf ;
    }
    while(sfwCurDict != NULL)
    {
        for(sfwCurIndx=0 ; sfwCurIndx<sfwCurDict->tableSize ; sfwCurIndx++)
        {
            off = meEntryGetAddr(sfwCurDict->table[sfwCurIndx]) ;
            while(off != 0)
            {
                sfwCurWord = (meDICTWORD *) (off + (uint32) sfwCurDict->table) ;
                if(!meWordGetErrorFlag(sfwCurWord) && 
                   ((ww=meWordGetWord(sfwCurWord))[0] != '\0'))
                {
                    /* Get the word */
                    len = meWordGetWordLen(sfwCurWord) ;
                    for(flen=0 ; flen<len ; flen++)
                        wp[flen] = toLatinLower(ww[flen]) ;
                    wp[len] = '\0' ;
                    if((sfwFlags || (wp[0] == sfwCurMask[0])) &&
                       (regexStrCmp(wp,sfwCurMask,1) > 0))
                    {
                        spellWordToUserFont(wp,wp) ;
                        return wp ;
                    }
                    /* Get the flags */
sfwJumpNotPreNotSuf:
                    if((flen = meWordGetFlagLen(sfwCurWord)) > 0)
                    {
                        flags = meWordGetFlag(sfwCurWord) ;
                        if(sfwFlags || (wp[0] == sfwCurMask[0]))
                        {
                            /* try all the allowed suffixes */
                            for(sufRule=0 ; sufRule<flen ; sufRule++)
                            {
                                if(meRuleFlags[flags[sufRule]-FRSTSPELLRULE] & RULE_SUFFIX)
                                {
                                    sfwSufRule = meRuleTable[flags[sufRule]-FRSTSPELLRULE] ;
                                    while(sfwSufRule != NULL)
                                    {
                                        if((swp = wordApplySuffixRule(wp,len,sfwSufRule)) != NULL)
                                        {
                                            if(regexStrCmp(wp,sfwCurMask,1) > 0)
                                            {
                                                sfwSufRule->rule = sufRule ;  
                                                spellWordToUserFont(wp,wp) ;
                                                return wp ;
                                            }
                                            wordRemoveAddSuffixRule(swp,sfwSufRule) ;
                                        }
sfwJumpNotPreGotSuf:
                                        sfwSufRule = sfwSufRule->next ;
                                    }
                                }
                            }
                        }
                        for(preRule=0 ; preRule<flen ; preRule++)
                        {
                            if(meRuleFlags[flags[preRule]-FRSTSPELLRULE] & RULE_PREFIX)
                            {
                                sfwPreRule = meRuleTable[flags[preRule]-FRSTSPELLRULE] ;
                                while(sfwPreRule != NULL)
                                {
                                    if((sfwFlags || (sfwPreRule->append[0] == sfwCurMask[0])) &&
                                       ((pwp = wordApplyPrefixRule(wp,sfwPreRule)) != NULL))
                                    {
                                        len += sfwPreRule->changeLen ;
                                        if(regexStrCmp(pwp,sfwCurMask,1) > 0)
                                        {
                                            sfwPreRule->rule = preRule ;  
                                            spellWordToUserFont(pwp,pwp) ;
                                            return pwp ;
                                        }
sfwJumpGotPreNotSuf:
                                        if(meRuleFlags[flags[preRule]-FRSTSPELLRULE] & RULE_MIXABLE)
                                        {
                                            /* try all the allowed suffixes */
                                            for(sufRule=preRule ; sufRule>=0 ; sufRule--)
                                                if(flags[sufRule] == '_')
                                                    break ;
                                            sufRule++ ;
                                            for( ; sufRule<flen ; sufRule++)
                                            {
                                                if(flags[sufRule] == '_')
                                                    break ;
                                                if(meRuleFlags[flags[sufRule]-FRSTSPELLRULE] == (RULE_SUFFIX|RULE_MIXABLE))
                                                {
                                                    sfwSufRule = meRuleTable[flags[sufRule]-FRSTSPELLRULE] ;
                                                    while(sfwSufRule != NULL)
                                                    {
                                                        if((swp = wordApplySuffixRule(pwp,len,sfwSufRule)) != NULL)
                                                        {
                                                            if(regexStrCmp(pwp,sfwCurMask,1) > 0)
                                                            {
                                                                sfwPreRule->rule = preRule ;  
                                                                sfwSufRule->rule = sufRule ;  
                                                                spellWordToUserFont(pwp,pwp) ;
                                                                return pwp ;
                                                            }
                                                            wordRemoveAddSuffixRule(swp,sfwSufRule) ;
                                                        }
sfwJumpGotPreGotSuf:
                                                        sfwSufRule = sfwSufRule->next ;
                                                    }
                                                }
                                            }
                                        }
                                        len -= sfwPreRule->changeLen ;
                                        wordRemoveAddPrefixRule(wp,sfwPreRule) ;
                                    }
                                    sfwPreRule = sfwPreRule->next ;
                                }
                            }
                        }
                    }
                }
                off = meWordGetAddr(sfwCurWord) ;
            }
        }
        sfwCurDict = sfwCurDict->next ;
    }
    sfwCurWord = NULL ;
    return emptym ;
}

#endif

