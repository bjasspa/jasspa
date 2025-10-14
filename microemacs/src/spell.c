/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * spell.c - Spell checking routines.
 *
 * Copyright (C) 1994-2001 Steven Phillips
 * Copyright (C) 2002-2024 JASSPA (www.jasspa.com)
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
 * Created:     21/12/94
 * Synopsis:    Spell checking routines.
 * Authors:     Steven Phillips
 * Notes:
 *      The external input/output mostly complies with ispell, there are a
 *      few extenstions such as the disabling of hypens and auto-replacement
 *      words. The internals, including dictionaries, are completely different.
 *
 * TODO:
 *
 * hunspell aff features not yet implemented:
 *
 * a. NEEDAFFIX, CIRCUMFIX, COMPOUND*, ONLYINCOMPOUND (only used by Deutsch given EN ordinals are supported via ME regex rules)
 *
 * b. KEEPCASE (used only by Francais)
 *
 * c. COMPLEXPREFIXES support:
 *
 *   The used of COMPLEXPREFIXES rule to allow the removal of two prefixes, see
 *   complexprefixes.aff test for example of twofold stripping of prefixes. However
 *   multi-prefix stripping is associated with right to left languages, e.g. hebrew & arabic
 *   and given ME's lack of basic support for right to left languages, this is not a high
 *   priority in ME.
 *
 * d. Limited 2char and numeric aff rule flags
 *
 * Hunspell allow only the following combination of rules (when COMPLEXPREFIXES is disabled)
 *
 *         <base>
 *         <base>S1(bs)
 *         <base>S1(bs)S2(S1)
 *   P1(S1)<base>S1(bs)
 *   P1(S1)<base>S1(bs)S2(S1)
 *   P1(s2)<base>S1(bs)S2(S1)
 *   P1(bs)<base>S1(bs)
 *   P1(bs)<base>S1(bs)S2(S1)
 *   P1(bs)<base>
 *   P1(bs)<base>S1(P1)
 *   P1(bs)<base>S1(P1)S2(S1)
 *
 * Where P1(bs) is the a prefix flagged on the base word & S2(S1) is a suffix flagged as a subrule of
 * a suffix. S1(bs) is applied before p1(bs). The following are not allowed
 *
 *   P2(bs)P1(bs)<base>
 *   P2(P1)P1(bs)<base>
 *         <base>S1(bs)S2(bs)
 *   P1(bs)<base>S1(P1)S2(bs)
 *   P1(bs)<base>S1(bs)S2(P1)
 *   P1(S1)<base>S1(bs)S2(P1)
 *         <base>S1(bs)S2(S1)S3(S1)
 *         <base>S1(bs)S2(S1)S3(S2)
 *
 */

#define __SPELLC                        /* Define filename */

#include <string.h>
#include "emain.h"
#include "esearch.h"

#if MEOPT_SPELL

#define DTACTIVE        0x01
#define DTCHNGD         0x02
#define DTCREATE        0x04

#define SP_REG_ROOT  "/history/spell"   /* Root of spell in registry */
#define SP_REGI_SAVE "autosave"         /* Registry item - autosave */

#define TBLMINSIZE 128
#define TBLINCSIZE 512

/* Regex special rules are stored in rule slot [0], so rule '!' (0x21) is [1] etc so last rule '\xff' is [0xff-0x21+1] & there are 0xff-0x21+2 rules */
#define SPELLRULE_FIRST 0x21
#define SPELLRULE_LAST  0xff
#define SPELLRULE_SEP   '\x01'
#define SPELLRULE_COUNT (SPELLRULE_LAST-SPELLRULE_FIRST+2)
#define SPELLRULE_OFFST (SPELLRULE_FIRST-1)
#define SPELLRULE_REGEX 0
#define RULE_PREFIX     1
#define RULE_SUFFIX     2
#define RULE_MIXABLE    4
#define RULE_SUBR_REFP  8
#define RULE_SUBR_USEP  16
#define RULE_SUBR_REFS  32
#define RULE_SUBR_USES  64
#define RULE_CAN_GUESS  128

typedef struct meSpellRule {
    struct meSpellRule *next;
    meUByte *ending;
    meUByte *remove;
    meUByte *append;
    meUByte endingLen;
    meUByte removeLen;
    meUByte appendLen;
    meByte  changeLen;
    meUByte subRuleLen;
    /* score is used by guessList to eliminate primary suffixes */
    meUByte score;
    /* rule is used by find-words for continuation & guessList to eliminate twofold suffixes */
    meUByte rule;
    meUByte subRule[1];
} meSpellRule;

static meUByte meRuleFlags[SPELLRULE_COUNT];
static meSpellRule *meRuleTable[SPELLRULE_COUNT];
static meRegexClass *meRuleClassLst;
static meUByte meRuleClassCnt;
static meUByte meRuleMax=1;

/* Restrict base words to a maximum length of 127 chars, and affix flags to 63 (this could be quite
 * tight as base words can have multiple entries leading to concatenated flags).
 * Also restrict the maximum affix length (remove or append) to 32, so as ME supports twofold
 * suffix removal the maximum length is 127+3*32 (1 pre and 2 suf).
 * However as right to left languages need twofold 'prefix' instead of suffix keep the prefix and
 * suffix padding requirements symetrical.
 *
 * All functions that add/remove affixes should be given the starting word 64 bytes into the buffer
 * and with at least 64 chars after it. This are theoretical limits but, as we still limit all word
 * input meWORD_SIZE_MAX this is purely theoretical and very unlikely to ever be fully required.
 */
#define meWORD_SIZE_MAX  127
#define meWORD_FLAG_MAX  63
#define meAFFIX_SIZE_MAX 32
#define meAFFIXBUF_SIZE_MAX (2*meAFFIX_SIZE_MAX)
#define meWORDBUF_SIZE_MAX (meWORD_SIZE_MAX+(2*meAFFIXBUF_SIZE_MAX)+1)
#define meGUESS_MAX     20
#define meSCORE_MAX     60

typedef meUByte meWordBuf[meWORDBUF_SIZE_MAX];
typedef meUByte meDictAddr[3];
typedef struct {
    meDictAddr  next;
    meUByte     wordLen;
    meUByte     flagLen;
    meUByte     data[1];
} meDictWord;
/* following is the same as above but with a fixed max buffer used for word adding */
typedef struct {
    meDictAddr  next;
    meUByte     wordLen;
    meUByte     flagLen;
    meUByte     data[meWORD_SIZE_MAX+meWORD_SIZE_MAX];
} meDictAddWord;

#define meDICTWORD_SIZE  meIntFromPtr(&((meDictWord *)0)->data)

typedef struct meDictionary {
    meUByte     flags;
    meUByte    *fname;
    meInt       noWords;
    meInt       tableSize;
    meInt       dSize;
    meInt       dUsed;
    meDictAddr *table;
    struct meDictionary *next;
} meDictionary;

static meDictionary *dictHead;
static meDictionary *dictIgnr;
static meDictWord *wordCurr;
static meUByte suffixRmvLenMax;
static meUByte guessWordLenMin;
static meUByte guessWordLenMax;
static meUByte caseFlags;
static meUByte hyphenCheck;
static meUByte maxScore;

/* find-words static variables */
static meUByte        sfwPlc;
static meUByte        sfwWfl;
static meInt          sfwDctI;
static meUByte       *sfwMsk;
static meDictionary  *sfwDct;
static meDictWord    *sfwWrd;
static meSpellRule   *sfwRp1;
static meSpellRule   *sfwRp2;
static meSpellRule   *sfwRp3;

#define SPELL_FLAG_LMASK      meWORD_FLAG_MAX
#define SPELL_ERROR           0x80
#define SPELL_NOSUGGEST       0x40

#define SPELL_CASE_FUPPER     0x01
#define SPELL_CASE_FLOWER     0x02
#define SPELL_CASE_CUPPER     0x04
#define SPELL_CASE_CLOWER     0x08

#define toLatinLower(c)       (isUpper(toDisplayCharset(c)) ? toInternalCharset(toggleCase(toDisplayCharset(c))):(c))
#define toggleLatinCase(c)    toInternalCharset(toggleCase(toDisplayCharset(c)))

#define meEntryGetAddr(ed)       (((ed)[0] << 16) | ((ed)[1] << 8) | (ed)[2])
#define meEntrySetAddr(ed,off)   (((ed)[0] = (meUByte) ((off)>>16)),((ed)[1] = (meUByte) ((off)>> 8)),((ed)[2] = (meUByte) (off)))
#define meWordGetErrorFlag(wd)   ((wd)->flagLen & SPELL_ERROR)
#define meWordSetErrorFlag(wd)   ((wd)->flagLen |= SPELL_ERROR)
#define meWordIsSuggestable(wd)  (((wd)->flagLen & (SPELL_ERROR|SPELL_NOSUGGEST))==0)
#define meWordGetNoSuggestFlag(wd) ((wd)->flagLen & SPELL_NOSUGGEST)
#define meWordSetNoSuggestFlag(wd) ((wd)->flagLen |= SPELL_NOSUGGEST)
#define meWordGetAddr(wd)        meEntryGetAddr((wd)->next)
#define meWordSetAddr(wd,off)    meEntrySetAddr((wd)->next,off)
#define meWordGetWordLen(wd)     ((wd)->wordLen)
#define meWordSetWordLen(wd,len) ((wd)->wordLen=len)
#define meWordGetWord(wd)        ((wd)->data)
#define meWordGetFlagLen(wd)     ((wd)->flagLen & SPELL_FLAG_LMASK)
#define meWordSetFlagLen(wd,len) ((wd)->flagLen = ((wd)->flagLen & ~SPELL_FLAG_LMASK) | len)
#define meWordGetFlag(wd)        ((wd)->data+meWordGetWordLen(wd))

#define meSpellHashFunc(tsz,str,len,hsh)                                     \
do {                                                                         \
    register meUInt hh=0;                                                    \
    register int ll=len;                                                     \
    while(--ll >= 0)                                                         \
        hh = (hh << 5) + hh + str[ll];                                       \
    hsh = (meInt) (hh & (tsz-1));                                            \
} while(0)

static meDictWord *
meDictionaryLookupWord(meDictionary *dict, meUByte *word, int len)
{
    meDictWord *ent;
    meInt n, off;
    int s, ll;

    meSpellHashFunc(dict->tableSize,word,len,n);
    off = meEntryGetAddr(dict->table[n]);

    while(off != 0)
    {
        ent = (meDictWord *) mePtrOffset(dict->table,off);
        ll = meWordGetWordLen(ent);
        if(ll >= len)
        {
            if(ll > len)
                break;
            s = meStrncmp(meWordGetWord(ent),word,ll);
            if(s > 0)
                break;
            if(s == 0)
                return ent;
        }
        off = meWordGetAddr(ent);
    }
    return NULL;
}

static void
meDictionaryDeleteWord(meDictionary *dict, meUByte *word, int len)
{
    meDictWord  *ent;
    if((ent = meDictionaryLookupWord(dict,word,len)) != NULL)
    {
        /* Remove the old entry */
        meWordGetWord(ent)[0] = '\0';
        (dict->noWords)--;
        dict->flags |= DTCHNGD;
    }
}

static int
meDictionaryAddWord(meDictionary *dict, meDictWord *wrd)
{
    meDictWord *ent, *lent, *nent;
    meInt n, len, off;
    meUByte wlen, flen, olen, *word;

    word = meWordGetWord(wrd);
    wlen = meWordGetWordLen(wrd);
    flen = meWordGetFlagLen(wrd);
    if((ent = meDictionaryLookupWord(dict,word,wlen)) != NULL)
    {
        if(flen == 0)
            return meTRUE;
        if(!meWordGetErrorFlag(ent) && !meWordGetErrorFlag(wrd) &&
           ((olen=meWordGetFlagLen(ent)) > 0))
        {
            meUByte *ff, buff[meWORD_SIZE_MAX];
            memcpy(buff,meWordGetFlag(ent),olen);
            buff[olen] = '\0';
            ff = meWordGetFlag(wrd);
            ff[flen] = '\0';
            if(meStrstr(buff,ff) != NULL)
                return meTRUE;
            if(meStrstr(ff,buff) == NULL)
            {
                /* Combine the word flags with a separator char, this avoid inappropriate flag mixing, e.g. unzip okay and zipless okay but not unzipless */
                if((flen+olen+1) > meWORD_FLAG_MAX)
                    return meFALSE;
                ff[flen++] = SPELLRULE_SEP;
                memcpy(ff+flen,buff,olen);
                flen += olen;
            }
        }
        /* Remove the old entry */
        meWordGetWord(ent)[0] = '\0';
        (dict->noWords)--;
    }
    dict->flags |= DTCHNGD;
    len = meDICTWORD_SIZE + wlen + flen;

    if((dict->dUsed+len) > dict->dSize)
    {
        dict->dSize = dict->dUsed+len+TBLINCSIZE;
        if((dict->table = (meDictAddr *) meRealloc(dict->table,dict->dSize)) == NULL)
            return meFALSE;
    }
    nent = (meDictWord *) mePtrOffset(dict->table,dict->dUsed);
    nent->wordLen = wrd->wordLen;
    nent->flagLen = wrd->flagLen;
    meWordSetFlagLen(nent,flen);
    memcpy(meWordGetWord(nent),word,wlen);
    memcpy(meWordGetFlag(nent),meWordGetFlag(wrd),flen);

    lent = NULL;
    meSpellHashFunc(dict->tableSize,word,wlen,n);
    off = meEntryGetAddr(dict->table[n]);
    while(off != 0)
    {
        ent = (meDictWord *) mePtrOffset(dict->table,off);
        if(meWordGetWordLen(ent) >= wlen)
        {
            if((meWordGetWordLen(ent) > wlen) ||
               meStrncmp(meWordGetWord(ent),word,wlen) > 0)
                break;
        }
        off = meEntryGetAddr(ent->next);
        lent = ent;
    }
    if(lent == NULL)
        meEntrySetAddr(dict->table[n],dict->dUsed);
    else
        meWordSetAddr(lent,dict->dUsed);
    meWordSetAddr(nent,off);
    dict->dUsed += len;
    (dict->noWords)++;
    return meTRUE;
}


static void
meDictionaryRehash(meDictionary *dict)
{
    meDictWord *ent;
    meDictAddr *table, *tbl;
    meInt tableSize, oldtableSize, ii, dUsed, dSize, noWords, off;
    int repeat;

    oldtableSize = dict->tableSize;
    noWords = dict->noWords >> 3;
    tableSize = TBLMINSIZE;
    while(tableSize <= noWords)
        tableSize <<= 1;
    if(tableSize == oldtableSize)
        return;
    /* do rehash twice, the first time gets all words linked to the right hash value list, the 2nd time round
     * gets the words in the right order in the list which makes traversal more efficient due to reduced memory paging */
    repeat = 2;
    do {
        dUsed = sizeof(meDictAddr)*tableSize;
        dSize = dict->dUsed+dUsed;

        if((table = (meDictAddr *) meMalloc(dSize)) == NULL)
            return;
        memset(table,0,dUsed);
        tbl = dict->table;
        dict->dUsed = dUsed;
        dict->dSize = dSize;
        dict->table = table;
        dict->tableSize = tableSize;
        dict->noWords = 0;
        for(ii=0 ; ii<oldtableSize ; ii++)
        {
            off = meEntryGetAddr(tbl[ii]);
            while(off != 0)
            {
                ent = (meDictWord *) mePtrOffset(tbl,off);
                /* Check this word has not been erased, if not then add */
                if(meWordGetWord(ent)[0] != '\0')
                    meDictionaryAddWord(dict,ent);
                off = meWordGetAddr(ent);
            }
        }
        free(tbl);
        oldtableSize = tableSize;
    } while(--repeat);
}

#define meDICT_HDR_SZ (2+3+3)

static int
meDictionaryLoad(meDictionary *dict)
{
    unsigned char *td;
    meInt dSize, dUsed;
    meStat stats;
    int rr;

    /* use internal fileio functions to support loading TFS dictionaries */
    if((dict->fname == NULL) || (((rr = getFileStats(dict->fname,0,&stats,NULL)) & meIOTYPE_REGULAR) == 0) ||
       (stats.stsizeHigh != 0) || (stats.stsizeLow < meDICT_HDR_SZ) ||
       ((meior.type = (rr & 0x7f)),(ffReadFileOpen(&meior,dict->fname,meRWFLAG_READ|meRWFLAG_NODIRLIST|meRWFLAG_SILENT,NULL) <= 0)))
        return mlwrite(MWABORT,(meUByte *)"Failed to open dictionary [%s]",dict->fname);
    dUsed = stats.stsizeLow - meDICT_HDR_SZ;
    dSize = dUsed + TBLINCSIZE;

    if((ffgetBuf(&meior,0,meFIOBUFSIZ) < 0) || (ffremain < meDICT_HDR_SZ) ||
       (ffbuf[0] != 0xED) || (ffbuf[1] != 0xF1))
        rr = mlwrite(MWABORT,(meUByte *)"[%s does not have correct id string]",dict->fname);
    else if((td = (unsigned char *) meMalloc(dSize)) == NULL)
        rr = meFALSE;
    else
    {
        dict->dUsed = dUsed;
        dict->dSize = dSize;
        dict->noWords = meEntryGetAddr((ffbuf+2));
        dict->tableSize = meEntryGetAddr((ffbuf+5));
        dUsed = ffremain-meDICT_HDR_SZ;
        memcpy(td,ffbuf+meDICT_HDR_SZ,dUsed);
        while(((rr=ffgetBuf(&meior,0,meFIOBUFSIZ)) > 0) && (ffremain > 0))
        {
            memcpy(td+dUsed,ffbuf,ffremain);
            dUsed += ffremain;
        }
        if(rr < 0)
        {
            free(td);
            rr = mlwrite(MWABORT,(meUByte *)"failed to read dictionary %s",dict->fname);
        }
        else
        {
            dict->table = (meDictAddr *) td;
            dict->flags |= DTACTIVE;
            meDictionaryRehash(dict);
            rr = meTRUE;
        }
    }
    ffReadFileClose(&meior,meRWFLAG_READ|meRWFLAG_SILENT);

    return rr;
}


static int
meSpellInitDictionaries(void)
{
    meDictionary   *dict, *ldict;

    ldict = NULL;
    dict = dictHead;
    if(dict == NULL)
        return mlwrite(MWABORT,(meUByte *)"[No dictionaries]");
    if(!meRuleMax)
    {
        meSpellRule *rr;
        int ii=SPELLRULE_COUNT-1, ll, rl;
        meUByte bb;
        while((meRuleTable[ii] == NULL) && (--ii > 1))
            ;
        meRuleMax = ii;
        do {
            if((rr=meRuleTable[ii]) != NULL)
            {
                bb = (meRuleFlags[ii] & RULE_SUFFIX) ? RULE_SUBR_REFS:RULE_SUBR_REFP;
                do {
                    if((ll=rr->subRuleLen) > 0)
                    {
                        while(--ll >= 0)
                        {
                            rl = rr->subRule[ll] - SPELLRULE_OFFST;
                            if(meRuleTable[rl] != NULL)
                            {
                                meRuleFlags[rl] |= bb;
                                meRuleFlags[ii] |= (meRuleFlags[rl] & RULE_SUFFIX) ? RULE_SUBR_USES:RULE_SUBR_USEP;
                            }
                        }
                    }
                } while((rr = rr->next) != NULL);
            }
        } while((--ii) > 0);
    }
    while(dict != NULL)
    {
        if(!(dict->flags & DTACTIVE) &&
           (meDictionaryLoad(dict) <= 0))
        {
            if(ldict == NULL)
                dictHead = dict->next;
            else
                ldict->next = dict->next;
            meFree(dict->fname);
            meFree(dict);
            return meFALSE;
        }
        ldict = dict;
        dict = ldict->next;
    }
    if(dictIgnr == NULL)
    {
        meDictAddr *table;
        meInt dSize, dUsed;

        dUsed = sizeof(meDictAddr)*TBLMINSIZE;
        dSize = dUsed + TBLINCSIZE;
        if(((dictIgnr = (meDictionary *) meMalloc(sizeof(meDictionary))) == NULL) ||
           ((table = (meDictAddr *) meMalloc(dSize)) == NULL))
            return meFALSE;
        memset(table,0,dUsed);
        dictIgnr->dSize = dSize;
        dictIgnr->dUsed = dUsed;
        dictIgnr->table = table;
        dictIgnr->noWords = 0;
        dictIgnr->tableSize = TBLMINSIZE;
        dictIgnr->flags = DTACTIVE;
        dictIgnr->fname = NULL;
        dictIgnr->next = NULL;
    }
    return meTRUE;
}

static meDictionary *
meDictionaryFind(int flag)
{
    meDictionary *dict;
    meUByte fname[meBUF_SIZE_MAX], tmp[meBUF_SIZE_MAX];
    int found;

    if(meGetString((meUByte *)"Dictionary name",MLFILECASE,0,tmp,meBUF_SIZE_MAX) <= 0)
        return meFALSE;

    if(!fileLookup(tmp,extDictCnt,extDictLst,meFL_CHECKPATH|meFL_USESRCHPATH,fname))
    {
        meStrcpy(fname,tmp);
        found = 0;
    }
    else
        found = 1;
    dict = dictHead;
    while(dict != NULL)
    {
        if(!fnamecmp(dict->fname,fname))
            return dict;
        dict = dict->next;
    }
    if(!(flag & 1) ||
       (!found && !(flag & 2)))
    {
        mlwrite(MWABORT|MWCLEXEC,(meUByte *)"[Failed to find dictionary %s]",fname);
        return NULL;
    }
    if(((dict = (meDictionary *) meMalloc(sizeof(meDictionary))) == NULL) ||
       ((dict->fname = meStrdup(fname)) == NULL))
        return NULL;
    dict->table = NULL;
    dict->flags = 0;
    dict->next = dictHead;
    dictHead = dict;
    if(!found || (flag & 4))
    {
        meDictAddr *table;
        meInt tableSize, dSize, dUsed;
        tableSize = TBLMINSIZE;
        dUsed = sizeof(meDictAddr)*tableSize;
        dSize = dUsed + TBLINCSIZE;
        if((table = (meDictAddr *) meMalloc(dSize)) == NULL)
            return NULL;
        memset(table,0,dUsed);
        dict->dSize = dSize;
        dict->dUsed = dUsed;
        dict->table = table;
        dict->noWords = 0;
        dict->tableSize = tableSize;
        dict->flags = (DTACTIVE | DTCREATE);
    }
    return dict;
}

static void
meDictWordDump(meDictWord *ent, meUByte *buff)
{
    meUByte cc, *ss=meWordGetWord(ent), *dd=buff;
    int len;

    len = meWordGetWordLen(ent);
    do {
        cc = *ss++;
        *dd++ = toDisplayCharset(cc);
    } while(--len > 0);

    len = meWordGetFlagLen(ent);
    if(meWordGetErrorFlag(ent))
        *dd++ = '>';
    else if(meWordGetNoSuggestFlag(ent))
        *dd++ = '!';
    else if(len)
        *dd++ = '/';
    if(len)
    {
        ss = meWordGetFlag(ent);
        do {
            cc = *ss++;
            *dd++ = toDisplayCharset(cc);
        } while(--len > 0);
    }
    *dd = '\0';
}

int
dictionaryAdd(int f, int n)
{
    meDictionary *dict;

    if(n == 0)
        f = 7;
    else if(n > 0)
        f = 3;
    else
        f = 1;
    if((dict = meDictionaryFind(f)) == NULL)
        return meFALSE;
    if(n < 0)
    {
        meDictAddr *tbl;
        meInt tableSize, ii, off;

        if(meModeTest(frameCur->windowCur->buffer->mode,MDVIEW))
            /* don't allow character insert if in read only */
            return rdonly();
        if((meSpellInitDictionaries() <= 0) ||
           (lineInsertString(0,(meUByte *)"Dictionary: ") <= 0) ||
           (lineInsertString(0,dict->fname) <= 0) ||
           (lineInsertNewline(0) <= 0) ||
           (lineInsertNewline(0) <= 0))
            return meABORT;

        tableSize = dict->tableSize;
        tbl = dict->table;
        frameCur->windowCur->dotOffset = 0;
        for(ii=0 ; ii<tableSize ; ii++)
        {
            off = meEntryGetAddr(tbl[ii]);
            while(off != 0)
            {
                meDictWord *ent;
                meUByte buff[meBUF_SIZE_MAX];

                ent = (meDictWord *) mePtrOffset(tbl,off);
                off = meWordGetAddr(ent);
                if(meWordGetWord(ent)[0] != '\0')
                {
                    meDictWordDump(ent,buff);
                    if((lineInsertString(0,buff) <= 0) ||
                       (lineInsertNewline(0) <= 0))
                        return meABORT;
                }
            }
        }
    }
    return meTRUE;
}


int
spellRuleAdd(int f, int n)
{
    meSpellRule  *rr;
    meRuleMax = 0;
    if(n == 0)
    {
        for(n=0 ; n<SPELLRULE_COUNT; n++)
        {
            while((rr=meRuleTable[n]) != NULL)
            {
                meRuleTable[n] = rr->next;
                free(rr);
            }
            meRuleFlags[n] = 0;
        }
        hyphenCheck = 1;
        maxScore = meSCORE_MAX;
        if(meRuleClassCnt)
        {
            free(meRuleClassLst);
            meRuleClassLst = NULL;
            meRuleClassCnt = 0;
        }
        /* reset the find-words */
        if(sfwMsk != NULL)
        {
            free(sfwMsk);
            sfwMsk = NULL;
        }
    }
    else
    {
        meSpellRule   *pr;
        meUByte *ss, cc, bb, rl, al;
        meUByte buff[meBUF_SIZE_MAX];
        int rule, ll;

        if((rule = mlCharReply((meUByte *)"Rule flag: ",0,NULL,NULL)) < 0)
            return meABORT;
        if(f == meFALSE)
        {
            if(rule == '-')
            {
                if((rule = mlyesno((meUByte *)"Enable hyphen")) == meABORT)
                    return meABORT;
                hyphenCheck = (meUByte) rule;
                return meTRUE;
            }
            if(rule == '#')
            {
                if(meGetString((meUByte *)"Guess score",MLNOSPACE,0,buff,meBUF_SIZE_MAX) <= 0)
                    return meABORT;
                maxScore = ((ll = meAtoi(buff)) > 255) ? 255:ll;
                return meTRUE;
            }
            if(rule == '*')
            {
                rule = SPELLRULE_REGEX;
                if((meGetString((meUByte *)"Rule",MLNOSPACE,0,buff,meBUF_SIZE_MAX) <= 0) ||
                   ((rr = meMalloc(sizeof(meSpellRule)+(ll=meStrlen(buff)))) == NULL))
                    return meABORT;
                rr->ending = rr->subRule;
                memcpy(rr->ending,buff,ll+1);
                rr->remove = NULL;
                rr->append = NULL;
                rr->endingLen = 0;
                rr->removeLen = 0;
                rr->appendLen = 0;
                rr->changeLen = 0;
                rr->subRuleLen = 0;
            }
            else
                return mlwrite(MWABORT,(meUByte *)"[Invalid spell special rule flag]");
        }
        else if((rule < SPELLRULE_FIRST) || (rule > SPELLRULE_LAST))
            return mlwrite(MWABORT,(meUByte *)"[Invalid spell rule flag]");
        else
        {
            meUByte ee[255];

            rule -= SPELLRULE_OFFST;
            if((meGetString((meUByte *)"Ending",MLNOSPACE,0,buff,255) <= 0) ||
               ((ll = meStrlen(buff) + 1),
                (meGetString((meUByte *)"Remove",MLNOSPACE,0,buff+ll,meAFFIX_SIZE_MAX) <= 0)) ||
               ((rl = (meUByte) meStrlen(buff+ll)),
                (meGetString((meUByte *)"Append",MLNOSPACE,0,buff+ll+rl,meAFFIX_SIZE_MAX) <= 0)) ||
               ((al = (meUByte) meStrlen(buff+ll+rl)),
                (meGetString((meUByte *)"Sub-rules",MLNOSPACE,0,buff+ll+rl+al,meWORD_FLAG_MAX) <= 0)))
                return meABORT;

            ss = buff;
            bb = 0;
            f = 0;
            while((cc=*ss++) != '\0')
            {
                if(cc == '[')
                {
                    meUByte c2, ff;
                    if((cc = *ss++) == '^')
                    {
                        ff = 0x01;
                        cc = *ss++;
                    }
                    else
                        ff = 0;
                    if((cc == '\0') || (cc == ']') || ((c2 = *ss++) == '\0'))
                        cc = '\0';
                    else if(c2 == ']')
                    {
                        if(ff)
                            ee[bb++] = ff;
                        ee[bb++] = cc;
                    }
                    else
                    {
                        meRegexClass rc;
                        ee[bb++] = ff|0x02;
                        meRegexClassClearAll(rc);
                        meRegexClassSet(rc,cc);
                        meRegexClassSet(rc,c2);
                        while(((cc = *ss++) != '\0') && (cc != ']'))
                            meRegexClassSet(rc,cc);
                        if((ff = meRuleClassCnt))
                        {
                            while((--ff > 0) && meRegexClassCmp(rc,meRuleClassLst[ff]))
                                ;
                        }
                        if(!ff)
                        {
                            if((meRuleClassCnt & 0x1f) == 0)
                            {
                                if(meRuleClassCnt > 0xc0)
                                    return mlwrite(MWABORT,(meUByte *)"[Spell rule - too many different char classes]");
                                if((meRuleClassLst = meRealloc(meRuleClassLst,sizeof(meRegexClass)*(meRuleClassCnt+0x20))) == NULL)
                                    return meABORT;
                            }
                            /* don't use idx 0 as this would create a \0 terminater */
                            if((ff = meRuleClassCnt++) == 0)
                            {
                                meRuleClassCnt = 2;
                                ff = 1;
                            }
                            meRegexClassCpy(meRuleClassLst[ff],rc);
                        }
                        ee[bb++] = ff;
                    }
                    if(cc == 0)
                        return mlwrite(MWABORT,(meUByte *)"[Spell rule - bad class in ending]");
                }
                else if(cc == '.')
                    ee[bb++] = 0x04;
                else
                    ee[bb++] = cc;
                f++;
            }
            ee[bb] = '\0';
            if(f < rl)
                return mlwrite(MWABORT,(meUByte *)"[Spell rule - cennot remove more chars than tested]");
            cc = (meUByte) meStrlen(buff+ll+rl+al);
            if((rr = meMalloc(sizeof(meSpellRule)+cc+rl+al+bb)) == NULL)
                return meABORT;
            rr->remove = rr->subRule+cc;
            rr->append = rr->remove+rl;
            rr->ending = rr->append+al;
            memcpy(rr->subRule,buff+ll+rl+al,cc);
            rr->subRuleLen = cc;
            memcpy(rr->remove,buff+ll,rl+al);
            rr->removeLen = rl;
            rr->appendLen = al;
            memcpy(rr->ending,ee,bb+1);
            rr->endingLen = f;
            rr->changeLen = al - rl;
            if(n < 0)
            {
                meRuleFlags[rule] |= RULE_PREFIX;
                n = -n;
            }
            else
                meRuleFlags[rule] |= RULE_SUFFIX;
            if((meRuleFlags[rule] & (RULE_PREFIX|RULE_SUFFIX)) == (RULE_PREFIX|RULE_SUFFIX))
                return mlwrite(MWABORT,(meUByte *)"[Spell rule flag cannot be used as prefix and suffix]");
            if(n & 2)
                meRuleFlags[rule] |= RULE_MIXABLE;
        }
        if((pr = meRuleTable[rule]) == NULL)
        {
            rr->next = NULL;
            meRuleTable[rule] = rr;
        }
        else
        {
            for(;; pr = pr->next)
            {
                if(!meStrcmp(rr->ending,pr->ending))
                {
                    rr->ending = pr->ending;
                    break;
                }
                if(pr->next == NULL)
                    break;
            }
            rr->next = pr->next;
            pr->next = rr;
        }
    }
    return meTRUE;
}

/* Note the return value for this is:
 * meABORT - there was a major failure (i.e. couldn't open the file)
 * meFALSE - user quit
 * meTRUE  - succeded
 * this is used by the exit function which ignore the major failures
 */
static int
meDictionarySave(meDictionary *dict, int n)
{
    meRegNode *rNd;
    int ii;

    if(!(dict->flags & DTCHNGD))
        return meTRUE;

    /* Never auto-save created dictionaries */
    if((dict->flags & DTCREATE) ||
       ((n & 0x01) &&
        (((rNd=regFind(NULL,(meUByte *)SP_REG_ROOT "/" SP_REGI_SAVE))==NULL) || (regGetLong(rNd,0) == 0))))
    {
        meUByte prompt[meBUF_SIZE_MAX];
        int  ret;
        meStrcpy(prompt,dict->fname);
        meStrcat(prompt,": Save dictionary");
        if((ret = mlyesno(prompt)) < 0)
        {
            ctrlg(meFALSE,1);
            return meFALSE;
        }
        if(ret == meFALSE)
            return meTRUE;
        if(dict->flags & DTCREATE)
        {
            meUByte fname[meBUF_SIZE_MAX], *pp, *ss;
            /* if the ctreated dictionary does not have a complete path then
             * either use $user-path or get one */
            if(meStrrchr(dict->fname,DIR_CHAR) != NULL)
                meStrcpy(fname,dict->fname);
            else if(meUserPath != NULL)
            {
                meStrcpy(fname,meUserPath);
                meStrcat(fname,dict->fname);
            }
            else
            {
                ss = dict->fname;
                if(inputFileName((meUByte *)"Save to directory",fname,1) <= 0)
                    return meFALSE;
                pp = fname + meStrlen(fname);
                if(pp[-1] != DIR_CHAR)
                    *pp++ = DIR_CHAR;
                meStrcpy(pp,ss);
            }

            if(((ii=meStrlen(fname)) < 4) ||
#ifdef _INSENSE_CASE
               meStricmp(fname+ii-4,(const meUByte *)".edf")
#else
               meStrcmp(fname+ii-4,".edf")
#endif
               )
                meStrcpy(fname+ii,".edf");
            free(dict->fname);
            dict->fname = meStrdup(fname);
        }
    }
    if(ffWriteFileOpen(&meiow,dict->fname,meRWFLAG_WRITE,NULL) > 0)
    {
        meUByte header[8];

        header[0] = 0xED;
        header[1] = 0xF1;
        meEntrySetAddr(header+2,dict->noWords);
        meEntrySetAddr(header+5,dict->tableSize);

        if(ffWriteFileWrite(&meiow,8,header,0) > 0)
            ffWriteFileWrite(&meiow,dict->dUsed,(meUByte *) dict->table,0);
        if(ffWriteFileClose(&meiow,meRWFLAG_WRITE,NULL) > 0)
        {
            dict->flags &= ~DTCHNGD;
            return meTRUE;
        }
    }
    return mlwrite(MWABORT|MWPAUSE,(meUByte *)"[Failed to write dictionary %s]",dict->fname);
}

/* Note the return value for this is:
 * meABORT - there was a major failure (i.e. couldn't open the file)
 * meFALSE - user quit
 * meTRUE  - succeded
 * this is used by the exit function which ignore the major failures
 */
int
dictionarySave(int f, int n)
{
    meDictionary   *dict;

    if(n & 0x02)
    {
        dict = dictHead;
        while(dict != NULL)
        {
            if((f=meDictionarySave(dict,n)) <= 0)
                return f;
            dict = dict->next;
        }
    }
    else
    {
        /* when saving a single disable the prompt */
        if(((dict = meDictionaryFind(0)) == NULL) ||
           ((f=meDictionarySave(dict,0)) <= 0))
            return f;
    }

    return meTRUE;
}

/* returns true if any dictionary needs saving */
int
anyChangedDictionary(void)
{
    meDictionary *dict;

    dict = dictHead;
    while(dict != NULL)
    {
        if(dict->flags & DTCHNGD)
            return meTRUE;
        dict = dict->next;
    }
    return meFALSE;
}

static void
meDictionaryFree(meDictionary *dict)
{
    meNullFree(dict->table);
    meNullFree(dict->fname);
    free(dict);
}

/* dictionaryDelete
 *
 * Argument is a bitmask where:
 *
 * 0x01 prompt before losing changes
 * 0x02 delete ignore dictionary
 * 0x04 delete all non-ignore dictionaries
 */
int
dictionaryDelete(int f, int n)
{
    /* reset the find-words */
    if(sfwMsk != NULL)
    {
        free(sfwMsk);
        sfwMsk = NULL;
    }
    if(n & 0x04)
    {
        if(dictIgnr != NULL)
        {
            meDictionaryFree(dictIgnr);
            dictIgnr = NULL;
        }
        if(!(n & 0x02))
            /* just remove the ignore */
            return meTRUE;
    }

    if(n & 0x02)
    {
        meDictionary *dd;

        while((dd=dictHead) != NULL)
        {
            if((n & 0x01) && (meDictionarySave(dd,0x01) <= 0))
                return meFALSE;
            dictHead = dd->next;
            meDictionaryFree(dd);
        }
    }
    else
    {
        meDictionary *dict, *dd;

        if((dict = meDictionaryFind(0)) == NULL)
            return meFALSE;

        if((n & 0x01) && (meDictionarySave(dict,0x01) <= 0))
            return meFALSE;
        if(dict == dictHead)
            dictHead = dict->next;
        else
        {
            dd = dictHead;
            while(dd->next != dict)
                dd = dd->next;
            dd->next = dict->next;
        }
        meDictionaryFree(dict);
    }
    return meTRUE;
}


static int
meSpellGetCurrentWord(meUByte *word)
{
    register meWindow *cwp=frameCur->windowCur;
    register meUByte *dp, c, alnm, nalnm=1;
    register int sz=0, alphaCnt=0;

    dp = (meUByte *) word;
    do
    {
        alnm = nalnm;
        c = meLineGetChar(cwp->dotLine,cwp->dotOffset);
        cwp->dotOffset++;
        *dp++ = c;
        if(isAlpha(c))
            alphaCnt = 1;
        nalnm = isAlphaNum(c);
    } while((++sz < meWORD_SIZE_MAX) && (nalnm || (alnm && isSpllExt(c))));

    dp[-1] = 0;
    cwp->dotOffset--;
    if(!alphaCnt)
        return -1;

    return sz - 1;
}

#define wordPrefixRuleAdd(bwd,rr)         memcpy((bwd),(rr)->append,(rr)->appendLen);
#define wordPrefixRuleRemove(bwd,rr)                                             \
do {                                                                             \
    if((rr)->removeLen)                                                          \
        memcpy((bwd),(rr)->remove,(rr)->removeLen);                              \
} while(0)

#define wordSuffixRuleGetEnd(wd,wlen,rr)  ((wd)+(wlen)-(rr)->removeLen)
#define wordSuffixRuleAdd(ewd,rr)                                                \
do {                                                                             \
    if((rr)->appendLen)                                                          \
        memcpy((ewd),(rr)->append,(rr)->appendLen);                              \
} while(0)
#define wordSuffixRuleRemove(ewd,rr)                                             \
do {                                                                             \
    if((rr)->removeLen)                                                          \
        memcpy((ewd),(rr)->remove,(rr)->removeLen);                              \
} while(0)

static int
ruleEndingCmp(meUByte *ww, meUByte *ending)
{
    meUByte wc, ec, rc;
    while((ec = *ending++) != 0)
    {
        if((wc = *ww++) == 0)
            return 1;
        if(ec & 0xf8)
        {
            if(wc != ec)
                return 1;
        }
        else if(ec & 0x02)
        {
            rc = *ending++;
            if(meRegexClassTest(meRuleClassLst[rc],wc))
            {
                if(ec & 0x01)
                    return 1;
            }
            else if((ec & 0x01) == 0)
                return 1;
        }
        else if((ec & 0x01) && (wc == *ending++))
            return 1;
    }
    return 0;
}

static meUByte *
wordApplyPrefixRule(meUByte *wd, int wlen, meSpellRule *rr)
{
    meUByte *nw;
    int len;

    if((rr->removeLen > wlen) || (((len=rr->endingLen) != 0) && ((len > wlen) || ruleEndingCmp(wd,rr->ending))))
        return NULL;
    nw = wd - rr->changeLen;
    wordPrefixRuleAdd(nw,rr);
    return nw;
}

static meUByte *
wordApplySuffixRule(meUByte *wd, int wlen, meSpellRule *rr)
{
    meUByte *ew, len;

    ew = wd+wlen;
    if((rr->removeLen > wlen) || (((len=rr->endingLen) != 0) && ((len > wlen) || ruleEndingCmp(ew-len,rr->ending))))
        return NULL;
    ew -= rr->removeLen;
    wordSuffixRuleAdd(ew,rr);
    return ew;
}

static meDictWord *
wordTryAffixRules(meUByte *word, int wlen)
{
    meDictionary *dict;
    meSpellRule *rr;
    meDictWord *wd;
    int ii, jj, ll;

    /* Try all the suffix rules to see if we can derive the word from another */
    ii = meRuleMax;
    do
    {
        rr = meRuleTable[ii];
        if(meRuleFlags[ii] & RULE_SUFFIX)
        {
            while(rr != NULL)
            {
                int len, slen, clen, elen;
                clen = rr->changeLen;
                elen = rr->endingLen;
                if(wlen >= elen+clen)
                {
                    int alen, nlen=wlen - clen;
                    alen = rr->appendLen;
                    if(!memcmp(rr->append,word+wlen-alen,alen))
                    {
                        meUByte ff;
                        wordSuffixRuleRemove(word+wlen-alen,rr);
                        if((elen == 0) || !ruleEndingCmp(word+nlen-elen,rr->ending))
                        {
                            dict = dictHead;
                            while(dict != NULL)
                            {
                                if(((wd=meDictionaryLookupWord(dict,word,nlen)) != NULL) && ((ll=meWordGetFlagLen(wd)) > 0) &&
                                   (memchr(meWordGetFlag(wd),ii+SPELLRULE_OFFST,ll) != NULL))
                                {
                                    /* got <base>S1(bs) - return */
                                    memcpy(word+wlen-alen,rr->append,alen);
                                    return wd;
                                }
                                dict = dict->next;
                            }
                            /* check for prefix subRules in the current suffix */
                            slen = rr->subRuleLen;
                            while(--slen >= 0)
                            {
                                ff = rr->subRule[slen]-SPELLRULE_OFFST;
                                if(meRuleFlags[ff] & RULE_PREFIX)
                                {
                                    meSpellRule *pr = meRuleTable[ff];
                                    while(pr != NULL)
                                    {
                                        meUByte *nw;
                                        int pnlen, pelen;

                                        if((nlen > pr->appendLen) && !memcmp(pr->append,word,pr->appendLen))
                                        {
                                            pnlen = nlen - pr->changeLen;
                                            nw = word + pr->changeLen;
                                            wordPrefixRuleRemove(nw,pr);
                                            if(((pelen=pr->endingLen) == 0) || !ruleEndingCmp(nw,pr->ending))
                                            {
                                                dict = dictHead;
                                                while(dict != NULL)
                                                {
                                                    /* note the prefix is a subRule of the suffix rule (ii) so the word must have the suffix flag not the preffix */
                                                    if(((wd=meDictionaryLookupWord(dict,nw,pnlen)) != NULL) && ((ll=meWordGetFlagLen(wd)) > 0) &&
                                                       (memchr(meWordGetFlag(wd),ii+SPELLRULE_OFFST,ll) != NULL))
                                                    {
                                                        /* got P1(S1)<base>S1(bs) - return */
                                                        wordPrefixRuleAdd(word,pr);
                                                        memcpy(word+wlen-alen,rr->append,alen);
                                                        return wd;
                                                    }
                                                    dict = dict->next;
                                                }
                                            }
                                            wordPrefixRuleAdd(word,pr);
                                        }
                                        pr = pr->next;
                                    }
                                }
                            }
                            /* No suitable prefix in the suffix subRules, next check to see if another affix rule has this suffix rule as a sub-rule */
                            if((ff=meRuleFlags[ii] & (RULE_SUBR_REFS|RULE_SUBR_REFP)) != 0)
                            {
                                ff <<= 1;
                                jj = meRuleMax;
                                do
                                {
                                    if((meRuleFlags[jj] & ff) && (ii != jj))
                                    {
                                        if(meRuleFlags[jj] & RULE_SUFFIX)
                                        {
                                            meSpellRule *sr = meRuleTable[jj];
                                            while(sr != NULL)
                                            {
                                                if(((len = sr->subRuleLen) > 0) && (memchr(sr->subRule,ii+SPELLRULE_OFFST,len) != NULL))
                                                {
                                                    clen = sr->changeLen;
                                                    elen = sr->endingLen;
                                                    if(nlen >= elen+clen)
                                                    {
                                                        int salen, snlen=nlen - clen;
                                                        salen = sr->appendLen;
                                                        if(!memcmp(sr->append,word+nlen-salen,salen))
                                                        {
                                                            wordSuffixRuleRemove(word+nlen-salen,sr);
                                                            if((elen == 0) || !ruleEndingCmp(word+snlen-elen,sr->ending))
                                                            {
                                                                /* Note: Initial S1(?) has now become S2(S1) as we have just found a new S1(?) */
                                                                dict = dictHead;
                                                                while(dict != NULL)
                                                                {
                                                                    if(((wd=meDictionaryLookupWord(dict,word,snlen)) != NULL) && ((ll=meWordGetFlagLen(wd)) > 0) &&
                                                                       (memchr(meWordGetFlag(wd),jj+SPELLRULE_OFFST,ll) != NULL))
                                                                    {
                                                                        /* got <base>S1(bs)S2(S1) - return */
                                                                        memcpy(word+nlen-salen,sr->append,salen);
                                                                        memcpy(word+wlen-alen,rr->append,alen);
                                                                        return wd;
                                                                    }
                                                                    dict = dict->next;
                                                                }
                                                                /* check S1(bs) for prefix subRules in the current suffix */
                                                                slen = sr->subRuleLen;
                                                                while(--slen >= 0)
                                                                {
                                                                    len = sr->subRule[slen]-SPELLRULE_OFFST;
                                                                    if(meRuleFlags[len] & RULE_PREFIX)
                                                                    {
                                                                        meSpellRule *pr = meRuleTable[len];
                                                                        while(pr != NULL)
                                                                        {
                                                                            meUByte *nw;
                                                                            int pnlen, pelen;

                                                                            if((snlen > pr->appendLen) && !memcmp(pr->append,word,pr->appendLen))
                                                                            {
                                                                                pnlen = snlen - pr->changeLen;
                                                                                nw = word + pr->changeLen;
                                                                                wordPrefixRuleRemove(nw,pr);
                                                                                if(((pelen=pr->endingLen) == 0) || !ruleEndingCmp(nw,pr->ending))
                                                                                {
                                                                                    dict = dictHead;
                                                                                    while(dict != NULL)
                                                                                    {
                                                                                        if(((wd=meDictionaryLookupWord(dict,nw,pnlen)) != NULL) && ((ll=meWordGetFlagLen(wd)) > 0) &&
                                                                                           (memchr(meWordGetFlag(wd),jj+SPELLRULE_OFFST,ll) != NULL))
                                                                                        {
                                                                                            /* got P1(S1)<base>S1(bs)S2(S1) - return */
                                                                                            wordPrefixRuleAdd(word,pr);
                                                                                            memcpy(word+nlen-salen,sr->append,salen);
                                                                                            memcpy(word+wlen-alen,rr->append,alen);
                                                                                            return wd;
                                                                                        }
                                                                                        dict = dict->next;
                                                                                    }
                                                                                }
                                                                                wordPrefixRuleAdd(word,pr);
                                                                            }
                                                                            pr = pr->next;
                                                                        }
                                                                    }
                                                                }
                                                                /* check S2(S1) for prefix subRules in the current suffix */
                                                                slen = rr->subRuleLen;
                                                                while(--slen >= 0)
                                                                {
                                                                    ff = rr->subRule[slen]-SPELLRULE_OFFST;
                                                                    if(meRuleFlags[ff] & RULE_PREFIX)
                                                                    {
                                                                        meSpellRule *pr = meRuleTable[ff];
                                                                        while(pr != NULL)
                                                                        {
                                                                            meUByte *nw;
                                                                            int pnlen, pelen;

                                                                            if((snlen > pr->appendLen) && !memcmp(pr->append,word,pr->appendLen))
                                                                            {
                                                                                pnlen = snlen - pr->changeLen;
                                                                                nw = word + pr->changeLen;
                                                                                wordPrefixRuleRemove(nw,pr);
                                                                                if(((pelen=pr->endingLen) == 0) || !ruleEndingCmp(nw,pr->ending))
                                                                                {
                                                                                    dict = dictHead;
                                                                                    while(dict != NULL)
                                                                                    {
                                                                                        /* note the prefix is a subRule of the suffix rule (ii) so the word must have the suffix flag not the preffix */
                                                                                        if(((wd=meDictionaryLookupWord(dict,nw,pnlen)) != NULL) && ((ll=meWordGetFlagLen(wd)) > 0) &&
                                                                                           (memchr(meWordGetFlag(wd),jj+SPELLRULE_OFFST,ll) != NULL))
                                                                                        {
                                                                                            /* got P1(S2)<base>S1(bs)S2(S1) - return */
                                                                                            wordPrefixRuleAdd(word,pr);
                                                                                            memcpy(word+nlen-salen,sr->append,salen);
                                                                                            memcpy(word+wlen-alen,rr->append,alen);
                                                                                            return wd;
                                                                                        }
                                                                                        dict = dict->next;
                                                                                    }
                                                                                }
                                                                                wordPrefixRuleAdd(word,pr);
                                                                            }
                                                                            pr = pr->next;
                                                                        }
                                                                    }
                                                                }
                                                                if(meRuleFlags[jj] & RULE_MIXABLE)
                                                                {
                                                                    int kk;
                                                                    kk = meRuleMax;
                                                                    do
                                                                    {
                                                                        if((meRuleFlags[kk] & (RULE_PREFIX|RULE_MIXABLE)) == (RULE_PREFIX|RULE_MIXABLE))
                                                                        {
                                                                            meSpellRule *pr = meRuleTable[kk];
                                                                            while(pr != NULL)
                                                                            {
                                                                                meUByte *nw;
                                                                                int pnlen, pelen;

                                                                                if((snlen > pr->appendLen) && !memcmp(pr->append,word,pr->appendLen))
                                                                                {
                                                                                    pnlen = snlen - pr->changeLen;
                                                                                    nw = word + pr->changeLen;
                                                                                    wordPrefixRuleRemove(nw,pr);
                                                                                    if(((pelen=pr->endingLen) == 0) || !ruleEndingCmp(nw,pr->ending))
                                                                                    {
                                                                                        dict = dictHead;
                                                                                        while(dict != NULL)
                                                                                        {
                                                                                            if(((wd=meDictionaryLookupWord(dict,nw,pnlen)) != NULL) && ((ll=meWordGetFlagLen(wd)) > 0) &&
                                                                                               (memchr(meWordGetFlag(wd),jj+SPELLRULE_OFFST,ll) != NULL) &&
                                                                                               (memchr(meWordGetFlag(wd),kk+SPELLRULE_OFFST,ll) != NULL))
                                                                                            {
                                                                                                /* TODO should check flags are in same group if there is a SPELLRULE_SEP char */
                                                                                                /* got P1(bs)<base>S1(bs)S2(S1) - return */
                                                                                                wordPrefixRuleAdd(word,pr);
                                                                                                memcpy(word+nlen-salen,sr->append,salen);
                                                                                                memcpy(word+wlen-alen,rr->append,alen);
                                                                                                return wd;
                                                                                            }
                                                                                            dict = dict->next;
                                                                                        }
                                                                                    }
                                                                                    wordPrefixRuleAdd(word,pr);
                                                                                }
                                                                                pr = pr->next;
                                                                            }
                                                                        }
                                                                    } while(--kk > 0);
                                                                }
                                                            }
                                                            memcpy(word+nlen-salen,sr->append,salen);
                                                        }
                                                    }
                                                }
                                                sr = sr->next;
                                            }
                                        }
                                        else
                                        {
                                            meSpellRule *sr = meRuleTable[jj];
                                            while(sr != NULL)
                                            {
                                                if(((len = sr->subRuleLen) > 0) && (memchr(sr->subRule,ii+SPELLRULE_OFFST,len) != NULL))
                                                {
                                                    meUByte *nw;
                                                    int pnlen, pelen;

                                                    if((nlen > sr->appendLen) && !memcmp(sr->append,word,sr->appendLen))
                                                    {
                                                        pnlen = nlen - sr->changeLen;
                                                        nw = word + sr->changeLen;
                                                        wordPrefixRuleRemove(nw,sr);
                                                        if(((pelen=sr->endingLen) == 0) || !ruleEndingCmp(nw,sr->ending))
                                                        {
                                                            dict = dictHead;
                                                            while(dict != NULL)
                                                            {
                                                                if(((wd=meDictionaryLookupWord(dict,nw,pnlen)) != NULL) && ((ll=meWordGetFlagLen(wd)) > 0) &&
                                                                   (memchr(meWordGetFlag(wd),jj+SPELLRULE_OFFST,ll) != NULL))
                                                                {
                                                                    /* got P1(bs)<base>S1(P1) - return */
                                                                    wordPrefixRuleAdd(word,sr);
                                                                    memcpy(word+wlen-alen,rr->append,alen);
                                                                    return wd;
                                                                }
                                                                dict = dict->next;
                                                            }
                                                        }
                                                        wordPrefixRuleAdd(word,sr);
                                                    }
                                                }
                                                sr = sr->next;
                                            }
                                        }
                                    }
                                } while(--jj > 0);
                            }
                            if(meRuleFlags[ii] & RULE_MIXABLE)
                            {
                                jj = meRuleMax;
                                do
                                {
                                    if((meRuleFlags[jj] & (RULE_PREFIX|RULE_MIXABLE)) == (RULE_PREFIX|RULE_MIXABLE))
                                    {
                                        meSpellRule *sr = meRuleTable[jj];
                                        while(sr != NULL)
                                        {
                                            meUByte *nw;
                                            int pnlen, pelen;

                                            if((nlen > sr->appendLen) && !memcmp(sr->append,word,sr->appendLen))
                                            {
                                                pnlen = nlen - sr->changeLen;
                                                nw = word + sr->changeLen;
                                                wordPrefixRuleRemove(nw,sr);
                                                if(((pelen=sr->endingLen) == 0) || !ruleEndingCmp(nw,sr->ending))
                                                {
                                                    dict = dictHead;
                                                    while(dict != NULL)
                                                    {
                                                        if(((wd=meDictionaryLookupWord(dict,nw,pnlen)) != NULL) && ((ll=meWordGetFlagLen(wd)) > 0) &&
                                                           (memchr(meWordGetFlag(wd),ii+SPELLRULE_OFFST,ll) != NULL) &&
                                                           (memchr(meWordGetFlag(wd),jj+SPELLRULE_OFFST,ll) != NULL))
                                                        {
                                                            /* TODO should check flags are in same group if there is a SPELLRULE_SEP char */
                                                            /* got P1(bs)<base>S1(bs) - return */
                                                            wordPrefixRuleAdd(word,sr);
                                                            memcpy(word+wlen-alen,rr->append,alen);
                                                            return wd;
                                                        }
                                                        dict = dict->next;
                                                    }
                                                }
                                                wordPrefixRuleAdd(word,sr);
                                            }
                                            sr = sr->next;
                                        }
                                    }
                                } while(--jj > 0);
                            }
                        }
                        memcpy(word+wlen-alen,rr->append,alen);
                    }
                }
                rr = rr->next;
            }
        }
        else
        {
            while(rr != NULL)
            {
                meUByte *nw;
                int si, nlen, elen;

                if((wlen > rr->appendLen) && !memcmp(rr->append,word,rr->appendLen))
                {
                    nlen = wlen - rr->changeLen;
                    nw = word + rr->changeLen;
                    wordPrefixRuleRemove(nw,rr);
                    if(((elen=rr->endingLen) == 0) || ((elen <= nlen) && !ruleEndingCmp(nw,rr->ending)))
                    {
                        /* Try the word on its own first */
                        dict = dictHead;
                        while(dict != NULL)
                        {
                            if(((wd=meDictionaryLookupWord(dict,nw,nlen)) != NULL) && ((ll=meWordGetFlagLen(wd)) > 0) &&
                               (memchr(meWordGetFlag(wd),ii+SPELLRULE_OFFST,ll) != NULL))
                            {
                                /* got P1(bs)<base> */
                                wordPrefixRuleAdd(word,rr);
                                return wd;
                            }
                            dict = dict->next;
                        }
                        /* Try removing any subRule suffices */
                        si = rr->subRuleLen;
                        while(--si >= 0)
                        {
                            if(meRuleFlags[rr->subRule[si]-SPELLRULE_OFFST] & RULE_SUFFIX)
                            {
                                meSpellRule *sr = meRuleTable[rr->subRule[si]-SPELLRULE_OFFST];
                                while(sr != NULL)
                                {
                                    int si, alen, slen;
                                    slen = nlen-sr->changeLen;
                                    elen = sr->endingLen;
                                    if(slen >= elen)
                                    {
                                        alen = sr->appendLen;
                                        if(!memcmp(sr->append,nw+nlen-alen,alen))
                                        {
                                            wordSuffixRuleRemove(nw+nlen-alen,sr);
                                            if((elen == 0) || !ruleEndingCmp(nw+slen-elen,sr->ending))
                                            {
                                                dict = dictHead;
                                                while(dict != NULL)
                                                {
                                                    if(((wd=meDictionaryLookupWord(dict,nw,slen)) != NULL) && ((ll=meWordGetFlagLen(wd)) > 0) &&
                                                       (memchr(meWordGetFlag(wd),ii+SPELLRULE_OFFST,ll) != NULL))
                                                    {
                                                        /* got P1(bs)<base>S1(P1) */
                                                        memcpy(nw+nlen-alen,sr->append,alen);
                                                        wordPrefixRuleAdd(word,rr);
                                                        return wd;
                                                    }
                                                    dict = dict->next;
                                                }
                                            }
                                            memcpy(nw+nlen-alen,sr->append,alen);
                                        }
                                        /* in the case of P1(bs)<base>S1(P1)S2(S1) we must find and remove S2(S1) before S1(P1) */

                                        si = sr->subRuleLen;
                                        while(--si >= 0)
                                        {
                                            if(meRuleFlags[sr->subRule[si]-SPELLRULE_OFFST] & RULE_SUFFIX)
                                            {
                                                meSpellRule *ssr = meRuleTable[sr->subRule[si]-SPELLRULE_OFFST];
                                                while(ssr != NULL)
                                                {
                                                    int salen, sslen, selen;
                                                    sslen = nlen-ssr->changeLen;
                                                    selen = ssr->endingLen;
                                                    if(sslen >= selen)
                                                    {
                                                        salen = ssr->appendLen;
                                                        if(!memcmp(ssr->append,nw+nlen-salen,salen))
                                                        {
                                                            wordSuffixRuleRemove(nw+nlen-salen,ssr);
                                                            if((selen == 0) || !ruleEndingCmp(nw+sslen-selen,ssr->ending))
                                                            {
                                                                slen = sslen-sr->changeLen;
                                                                if((slen >= elen) && !memcmp(sr->append,nw+sslen-alen,alen))
                                                                {
                                                                    wordSuffixRuleRemove(nw+sslen-alen,sr);
                                                                    if((elen == 0) || !ruleEndingCmp(nw+slen-elen,sr->ending))
                                                                    {
                                                                        dict = dictHead;
                                                                        while(dict != NULL)
                                                                        {
                                                                            if(((wd=meDictionaryLookupWord(dict,nw,slen)) != NULL) && ((ll=meWordGetFlagLen(wd)) > 0) &&
                                                                               (memchr(meWordGetFlag(wd),ii+SPELLRULE_OFFST,ll) != NULL))
                                                                            {
                                                                                /* got P1(bs)<base>S1(P1)S2(S1) */
                                                                                memcpy(nw+sslen-alen,sr->append,alen);
                                                                                memcpy(nw+nlen-salen,ssr->append,salen);
                                                                                wordPrefixRuleAdd(word,rr);
                                                                                return wd;
                                                                            }
                                                                            dict = dict->next;
                                                                        }
                                                                    }
                                                                    memcpy(nw+sslen-alen,sr->append,alen);
                                                                }
                                                            }
                                                            memcpy(nw+nlen-salen,ssr->append,salen);
                                                        }
                                                    }
                                                    ssr = ssr->next;
                                                }
                                            }
                                        }
                                    }
                                    sr = sr->next;
                                }
                            }
                        }
                    }
                    wordPrefixRuleAdd(word,rr);
                }
                rr = rr->next;
            }
        }
    } while(--ii > 0);
    return NULL;
}

static int
wordTrySpecialRules(meUByte *word, int wlen)
{
    meUByte cc;
    meSpellRule  *rr;

    if((rr = meRuleTable[SPELLRULE_REGEX]) != NULL)
    {
        /* Now try all the special rules - e.g. [0-9]*1st for 1st, 21st etc */
        cc = word[wlen];
        word[wlen] = '\0';
        while(rr != NULL)
        {
            if(regexStrCmp(word,rr->ending,meRSTRCMP_WHOLE) > 0)
            {
                word[wlen] = cc;
                return meTRUE;
            }
            rr = rr->next;
        }
        word[wlen] = cc;
    }
    return meFALSE;
}

/* wordCheckSearch
 *
 * Looks in all dictionaries for the given word, then tries all
 * the rules. Returns:
 *   meFALSE  Word not found
 *   meTRUE   found and word is good
 *   meABORT  found and word is erroneous
 */
static int
wordCheckSearch(meUByte *word, int wlen)
{
    meDictionary *dict;
    meDictWord   *wd;

    if((wd=meDictionaryLookupWord(dictIgnr,word,wlen)) == NULL)
    {
        dict = dictHead;
        while(dict != NULL)
        {
            if((wd=meDictionaryLookupWord(dict,word,wlen)) != NULL)
                break;
            dict = dict->next;
        }
        if(wd == NULL)
        {
            /* Now try all the suffixes on the original word */
            wd = wordTryAffixRules(word,wlen);

            if(wd == NULL)
                return wordTrySpecialRules(word,wlen);
        }
    }
    wordCurr = wd;
    if(meWordGetErrorFlag(wd))
        return meABORT;
    return meTRUE;
}

/* wordCheckBase
 *
 * Checks
 * 1) The word is a word
 * 2) Tries wordCheckSearch
 * 3) If upper case then capitalises and tries wordCheckSearch
 * 4) If capitalised the makes lower case and tries wordCheckSearch
 *
 * If all the above fail then it fails. Returns:
 *   meFALSE  Word is okay
 *   meTRUE   found and word is good
 *   meABORT  found and word is erroneous
 */
static int
wordCheckBase(meUByte *word, int wlen)
{
    meUByte *ss, cc;
    int ii;

    if(caseFlags == 0)
        /* No letters in the word -> not a word -> word okay */
        return meTRUE;

    if((ii=wordCheckSearch(word,wlen)) != meFALSE)
        return ii;

    /* If all upper then capitalise */
    if(caseFlags == (SPELL_CASE_FUPPER|SPELL_CASE_CUPPER))
    {
        ii = wlen;
        ss = word+1;
        while(--ii > 0)
        {
            cc = *ss;
            *ss++ = toggleLatinCase(cc);
        }
        if(wordCheckSearch(word,wlen) > 0)
            return meTRUE;
    }
    if(caseFlags & SPELL_CASE_FUPPER)
    {
        cc = *word;
        *word = toggleLatinCase(cc);
        if(wordCheckSearch(word,wlen) > 0)
            return meTRUE;
        *word = cc;
    }
    /* We failed to find it, restore the case if we changed it */
    if(caseFlags == (SPELL_CASE_FUPPER|SPELL_CASE_CUPPER))
    {
        ii = wlen;
        ss = word+1;
        while(--ii > 0)
        {
            cc = *ss;
            *ss++ = toggleLatinCase(cc);
        }
    }
    return meFALSE;
}

/* wordCheck
 *
 * Check for the given word in all dictionaries, returns:
 *   meFALSE  Word not found
 *   meTRUE   found and word is good
 *   meABORT  found and word is erroneous
 *
 * WARNING:
 *   The word buffer should have padding before and after the word (theoretical max is meAFFIXBUF_SIZE_MAX).
 *   The function only preserves word[0..len-1] chars, chars before & after (including terminator) may be trashed!
 */
static int
wordCheck(meUByte *word, int len)
{
    meUByte *ss;
    int ii, hyphen;

    if((ii=wordCheckBase(word,len)) != meFALSE)
        return ii;
    if(isSpllExt(word[len-1]))
    {
        len--;
        if(wordCheckBase(word,len) > 0)
            return meTRUE;
    }
    if(hyphenCheck)
    {
        /* check for hyphenated words such as wise-cracks */
        for(ss=word,hyphen=0 ; len ; len--,ss++)
        {
            if(*ss == '-')
            {
                if((ss != word) &&
                   (wordCheckBase(word,(meUByte)(ss-word)) <= 0))
                    return meFALSE;
                word = ss+1;
                hyphen = 1;
            }
        }
        if(hyphen &&
           (((len=ss-word) == 0) || (wordCheckBase(word,(meUByte)len) > 0)))
        {
            wordCurr = NULL;
            return meTRUE;
        }
    }
    return meFALSE;
}


#if 0
#define __LOG_FILE 1
#endif

#ifdef __LOG_FILE
FILE *fp;
#endif

/* wordGuessCalcScore
 *
 * Given the user source word (swd) and a dictionary word (dwd) the
 */
static int
wordGuessCalcScore(const meUByte *swd, int slen, const meUByte *dwd, int dlen, int testFlag)
{
#ifdef __LOG_FILE
    meUByte *swp=(meUByte *) swd, *dwp=(meUByte *) dwd;
#endif
    int  scr=0;
    meUByte cc, lcc='\0', dd, ldd='\0';

    for(;;)
    {
        if(slen == 0)
        {
            while(--dlen >= 0)
            {
                dd = *dwd++;
                if(!isSpllExt(dd))
                    /* ignore missed ' s etc */
                    scr += 22;
            }
            break;
        }
        if(dlen == 0)
        {
            while(--slen >= 0)
            {
                cc = *swd++;
                if(!isSpllExt(cc))
                    /* ignore missed ' s etc */
                    scr += 25;
            }
            break;
        }
        cc = *swd;
        dd = *dwd;
        dd = toLatinLower(dd);
        if(dd == cc)
        {
            swd++;
            slen--;
            lcc = cc;
            dwd++;
            dlen--;
            ldd = dd;
            continue;
        }
        if((dlen > 1) && (dwd[1] == cc))
        {
            if((slen > 1) && (swd[1] == dd) &&
               ((dlen <= slen) || (dwd[2] != dd)))
            {
                /* transposed chars */
                scr += 23;
                swd += 2;
                slen -= 2;
                lcc = dd;
            }
            else
            {
                if(ldd == dd)
                    /* missing double letter i.e. leter -> letter */
                    scr += 21;
                else if(!isSpllExt(dd))
                    /* missed letter i.e. ltter -> letter, ignore missed ' s etc */
                    scr += 22;
                swd++;
                lcc = cc;
                if(testFlag && (slen > 1))
                    /* if this is only a test on part of the word the adjust the
                     * other words length as well to keep the end point aligned
                     * this is because of base word tests. when testing the base
                     * of anomaly with anommoly the comparison is between [anommol]
                     * with [anomaly] the realignment caused by the double m means
                     * that it should be between [anommol] and [anomal]
                     */
                    slen -= 2;
                else
                    slen--;
            }
            dwd += 2;
            dlen -= 2;
            ldd = cc;
        }
        else if((slen > 1) && (swd[1] == dd))
        {
            if(lcc == cc)
                /* extra double letter i.e. vowwel -> vowel */
                scr += 24;
            else if(!isSpllExt(cc))
                /* extra letter i.e. lertter -> letter, ignore an extra ' etc. */
                scr += 25;
            swd += 2;
            slen -= 2;
            lcc = dd;
            dwd++;
            ldd = dd;
            if(testFlag && (dlen > 1))
                /* if this is only a test on part of the word the adjust the
                 * other words length as well to keep the end point aligned
                 * this is because of base word tests. when testing the base
                 * of anomaly with anommoly the comparison is between [anommol]
                 * with [anomaly] the realignment caused by the double m means
                 * that it should be between [anommol] and [anomal]
                 */
                dlen -= 2;
            else
                dlen--;
        }
        else
        {
            scr += 27;
            swd++;
            slen--;
            lcc = cc;
            dwd++;
            dlen--;
            ldd = dd;
        }
        if(scr >= maxScore)
            break;
    }
#ifdef __LOG_FILE
#if __LOG_FILE == 1
    if(!testFlag && (scr < maxScore))
#endif
    {
        slen += (int) (swd - swp);
        dlen += (int) (dwd - dwp);
        cc = swp[slen];
        swp[slen] = '\0';
        dd = dwp[dlen];
        dwp[dlen] = '\0';
        fprintf(fp,"WordScore [%s] with [%s] scr = %d, len %d (%d)\n",swp,dwp,scr,dlen-slen,testFlag);
        swp[slen] = cc;
        dwp[dlen] = dd;
    }
#endif
    return (scr >= maxScore) ? maxScore:scr;
}

static void
wordGuessAddToList(meUByte *word, int wlen, int curScr, meWordBuf *words, int *scores)
{
    int ii, jj;

    for(ii=0 ; ii<meGUESS_MAX ; ii++)
    {
        if(curScr < scores[ii])
        {
            for(jj=meGUESS_MAX-2 ; jj>=ii ; jj--)
            {
                scores[jj+1] = scores[jj];
                meStrcpy(words[jj+1],words[jj]);
            }
            scores[ii] = curScr;
            memcpy(words[ii],word,wlen);
            words[ii][wlen] = '\0';
#ifdef __LOG_FILE
            {
                fprintf(fp,"Added %d [%s]:\n",ii,words[ii]);
            }
#endif
            break;
        }
        /* check for duplicate */
        if((curScr == scores[ii]) && !memcmp(words[ii],word,wlen) && (words[ii][wlen] == '\0'))
            break;
    }
}

static void
wordGuessScoreSuffixRules(meUByte *word, int olen)
{
    meSpellRule *rr, *sr;
    int ii, jj, sl, scr, use;

    suffixRmvLenMax = 0;
    if(isSpllExt(word[olen-1]))
        /* if the last letter is a '.' then ignore it, i.e. "anommolies."
         * should compare suffixes with the "ies", not "es." */
        olen--;
    ii = meRuleMax;
    do
    {
        if(meRuleFlags[ii] & RULE_SUFFIX)
        {
            use = 0;
            rr = meRuleTable[ii];
            while(rr != NULL)
            {
                rr->rule = maxScore;
                if((jj = rr->appendLen) >= olen)
                    rr->score = maxScore;
                else
                {
                    if((scr = wordGuessCalcScore(word+olen-jj,jj,rr->append,jj,1)) < maxScore)
                    {
                        use = 1;
                        rr->score = scr;
                        if(rr->removeLen > suffixRmvLenMax)
                            suffixRmvLenMax = rr->removeLen;
                    }
                    if((meRuleFlags[ii] & RULE_SUBR_USES) && ((sl = rr->subRuleLen) > 0))
                    {
                        meUByte *sbf, buf[meAFFIXBUF_SIZE_MAX];
                        int si, sj, sscr;
                        memcpy(buf,rr->append,jj);
                        sscr = maxScore;
                        do {
                            si = rr->subRule[--sl]-SPELLRULE_OFFST;
                            if(meRuleFlags[si] & RULE_SUFFIX)
                            {
                                /* this makes the assumption that the primary suffix appends enough
                                 * to test all relevant secondary suffixes correctly */
                                sr = meRuleTable[si];
                                while(sr != NULL)
                                {
                                    if((sbf = wordApplySuffixRule(buf,jj,sr)) != NULL)
                                    {
                                        sj = jj + sr->changeLen;
                                        if((scr = wordGuessCalcScore(word+olen-sj,sj,buf,sj,1)) < maxScore)
                                        {
                                            use = 1;
                                            if(scr < sscr)
                                                sscr = scr;
                                            /* wrt. the base word only the primary suffix remove len counts */
                                            if(rr->removeLen > suffixRmvLenMax)
                                                suffixRmvLenMax = rr->removeLen;
                                        }
                                        wordSuffixRuleRemove(sbf,sr);
                                    }
                                    sr = sr->next;
                                }
                            }
                        } while(sl > 0);
                        rr->rule = sscr;
                    }
                }
                rr = rr->next;
            }
            if(use)
                meRuleFlags[ii] |= RULE_CAN_GUESS;
            else
                meRuleFlags[ii] &= ~RULE_CAN_GUESS;
        }
    } while(--ii > 0);
}

static void
wordGuessAddAffixRulesToList(meUByte *sword, int slen, meUByte *bword, int blen,
                             meUByte *wfp, meUByte wfc, meUByte *flags, meUByte noflags, meWordBuf *words,
                             int *scores, int bScr, meUByte prf, meUByte srf)
{
    meSpellRule *rr;
    meUByte el, *rws;
    int rwl, rScr, ii, fi;

    fi = noflags;
    while(--fi >= 0)
    {
        if((ii = flags[fi] - SPELLRULE_OFFST) < 0)
            noflags = fi;
        else if(meRuleFlags[ii] & RULE_PREFIX)
        {
            if((prf == 0) && ((srf == 0) || (meRuleFlags[ii] & RULE_MIXABLE)))
            {
                rr = meRuleTable[ii];
                while(rr != NULL)
                {
                    if(((rwl = blen+rr->changeLen) <= guessWordLenMax) &&
                       ((rws = wordApplyPrefixRule(bword,blen,rr)) != NULL))
                    {
                        if(srf || (rr->subRuleLen == 0))
                        {
                            if((rwl >= guessWordLenMin) &&
                               ((rScr = wordGuessCalcScore(sword,slen,rws,rwl,0)) < scores[meGUESS_MAX-1]))
                                wordGuessAddToList(rws,rwl,rScr,words,scores);
                        }
                        else if(((rScr = rwl - suffixRmvLenMax) < 1) ||
                                ((rScr = wordGuessCalcScore(sword,(rScr < slen) ? rScr:slen,rws,rScr,1)) < scores[meGUESS_MAX-1]))
                        {
                            wordGuessAddAffixRulesToList(sword,slen,rws,rwl,wfp,wfc,rr->subRule,rr->subRuleLen,words,scores,rScr,1,0);
                            if((rwl >= guessWordLenMin) &&
                               ((rScr = wordGuessCalcScore(sword,slen,rws,rwl,0)) < scores[meGUESS_MAX-1]))
                                wordGuessAddToList(rws,rwl,rScr,words,scores);
                        }
                        wordPrefixRuleRemove(bword,rr);
                    }
                    rr = rr->next;
                }
            }
        }
        else if((meRuleFlags[ii] & RULE_CAN_GUESS) && ((srf & 2) == 0))
        {
            /* if prf is not 0 then this is a prefix subRule so doesn't matter if not mixable */
            rr = meRuleTable[ii];
            while(rr != NULL)
            {
                /* TODO? this does not support P1(S1)<base>S1(bs)S2(S1) */

                if(((rwl = blen+rr->changeLen) <= guessWordLenMax) &&
                   ((bScr+rr->score < scores[meGUESS_MAX-1]) || ((srf == 0) && (bScr+rr->rule < scores[meGUESS_MAX-1])) ||
                    ((prf == 0) && ((meRuleFlags[ii] & RULE_MIXABLE) || ((meRuleFlags[ii] & RULE_SUBR_USEP) && rr->subRuleLen)))) &&
                   (((el=rr->endingLen) == 0) || ((el <= blen) && !ruleEndingCmp(bword+blen-el,rr->ending))))
                {
                    rws = wordSuffixRuleGetEnd(bword,blen,rr);
                    wordSuffixRuleAdd(rws,rr);
                    if((rwl >= guessWordLenMin) && (bScr+rr->score < scores[meGUESS_MAX-1]) &&
                       ((rScr = wordGuessCalcScore(sword,slen,bword,rwl,0)) < scores[meGUESS_MAX-1]))
                        wordGuessAddToList(bword,rwl,rScr,words,scores);
                    if(((srf == 0) && (bScr+rr->rule < scores[meGUESS_MAX-1]) &&
                        (((rScr = rwl - suffixRmvLenMax) < 1) ||
                         ((rScr = wordGuessCalcScore(sword,(rScr < slen) ? rScr:slen,bword,rScr,1)) < scores[meGUESS_MAX-1]))) ||
                       ((prf == 0) && (meRuleFlags[ii] & RULE_SUBR_USEP) && rr->subRuleLen))
                        wordGuessAddAffixRulesToList(sword,slen,bword,rwl,wfp,wfc,rr->subRule,rr->subRuleLen,words,scores,rScr,prf,
                                                     ((srf == 0) ? 1:3)|(meRuleFlags[ii] & RULE_MIXABLE));
                    if((prf == 0) && (meRuleFlags[ii] & RULE_MIXABLE))
                        /* don't allow any suffix rule to be added, prefix only */
                        wordGuessAddAffixRulesToList(sword,slen,bword,rwl,wfp,wfc,wfp,wfc,words,scores,0,0,(1|RULE_SUFFIX|RULE_MIXABLE));
                    wordSuffixRuleRemove(rws,rr);
                }
                rr = rr->next;
            }
        }
    }
}

static int
wordGuessGetList(meUByte *word, int olen, meWordBuf *words)
{
    meDictionary *dict;
    meDictWord *ent;
    meUByte cc, *wb, *ww, wbuff[meWORDBUF_SIZE_MAX];
    meInt ii, off;
    int scores[meGUESS_MAX];
    int wlen;

#ifdef __LOG_FILE
    fp = fopen("log","w+");
    cc = word[olen];
    word[olen] = '\0';
    fprintf(fp,"Spell suggest - Word [%s] maxScore: %d\n",word,maxScore);
    word[olen] = cc;
#endif
    ii = meGUESS_MAX-1;
    do
    {
        scores[ii] = maxScore;
        words[ii][0] = '\0';
    } while(--ii >= 0);
    wb = wbuff+meAFFIXBUF_SIZE_MAX;
    /* check for two words with a missing ' ' or '-' */
    if((wlen=olen-1) > 0)
    {
        /* wordCheck only preserves wlen chars, chars before & after (including terminator) can be trashed!
         * so we must use two different buffers to do this */
        meUByte *sw, swb[meWORDBUF_SIZE_MAX];
        sw = swb+meAFFIXBUF_SIZE_MAX+meWORD_SIZE_MAX;
        memcpy(wb,word,wlen);
        *sw = word[wlen];
        for(;;)
        {
            if((wordCheck(wb,wlen) > 0) && (wordCheck(sw,olen-wlen) > 0))
            {
                wb[wlen] = ' ';
                memcpy(wb+wlen+1,sw,olen-wlen);
                wordGuessAddToList(wb,olen+1,22,words,scores);
                wb[wlen] = '-';
                wordGuessAddToList(wb,olen+1,22,words,scores);
                if(scores[meGUESS_MAX-1] <= 22)
                    break;
            }
            if(--wlen <= 0)
                break;
            --sw;
            *sw = wb[wlen];
        }
    }
    /* Implement COMPLEXPREFIXES support - see main comment in header */
    if(caseFlags & (SPELL_CASE_FUPPER|SPELL_CASE_CUPPER))
    {
        meUByte cc;
        ww = word;
        while((cc = *ww) != '\0')
            *ww++ = toLatinLower(cc);
    }
    guessWordLenMin = maxScore/22;
    guessWordLenMax = olen + guessWordLenMin;
    guessWordLenMin = (guessWordLenMin >= olen) ? 1:olen-guessWordLenMin;
    wordGuessScoreSuffixRules(word,olen);
    dict = dictHead;
    while(dict != NULL)
    {
        for(ii=0 ; ii<dict->tableSize ; ii++)
        {
            off = meEntryGetAddr(dict->table[ii]);
            while(off != 0)
            {
                ent = (meDictWord *) mePtrOffset(dict->table,off);
                /* Check this is not an error or no-suggest word and has not been erased */
                if(meWordIsSuggestable(ent) && ((ww=meWordGetWord(ent))[0] != '\0') &&
                   ((wlen = meWordGetWordLen(ent)) <= guessWordLenMax))
                {
                    int wfl, wScr;
#ifdef __LOG_FILE
                    {
                        meUByte ccc=ww[wlen];
                        ww[wlen]='\0';
                        fprintf(fp,"Word [%s]:\n",ww);
                        ww[wlen]=ccc;
                    }
#endif
                    if((wlen >= guessWordLenMin) &&
                       ((wScr = wordGuessCalcScore(word,olen,ww,wlen,0)) < scores[meGUESS_MAX-1]))
                        wordGuessAddToList(ww,wlen,wScr,words,scores);
                    if((wfl=meWordGetFlagLen(ent)) > 0)
                    {
                        memcpy(wb,ww,wlen);
                        if((wScr = wlen - suffixRmvLenMax) >= 1)
                            wScr = wordGuessCalcScore(word,(wScr < olen) ? wScr:olen,wb,wScr,1);
                        wordGuessAddAffixRulesToList(word,olen,wb,wlen,meWordGetFlag(ent),wfl,meWordGetFlag(ent),wfl,words,scores,wScr,0,0);
                    }
                    if(TTbreakTest(0))
                        return -1;
                }
                off = meWordGetAddr(ent);
            }
        }
        dict = dict->next;
    }
    off=0;
    while((off<meGUESS_MAX) && (scores[off] < maxScore))
        off++;

    cc = word[olen-1];
    if(isSpllExt(cc))
    {
        /* if the last letter is a '.' etc then loop through the guesses adding one */
        for(ii=0 ; ii<off ; ii++)
        {
            wlen = meStrlen(words[ii]);
            /* must check that its not there already */
            if(words[ii][wlen-1] != cc)
            {
                words[ii][wlen]   = cc;
                words[ii][wlen+1] = '\0';
            }
        }
    }
#ifdef __LOG_FILE
    {
        fclose(fp);
    }
#endif
    return off;
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
#define SPELLWORD_NOSGGST 0x400      /* Accept word as correct but don't use in suggest list */

static int
spellWordToInternalCharset(meUByte *db, meUByte *ss)
{
    meUByte cc, *dd=db;
    cc = *ss++;
    if(isUpper(cc))
        caseFlags = SPELL_CASE_FUPPER;
    else if(isLower(cc))
        caseFlags = SPELL_CASE_FLOWER;
    else
        caseFlags = 0;
    *dd++ = toInternalCharset(cc);
    while((cc=*ss++) != '\0')
    {
        if(isUpper(cc))
            caseFlags |= SPELL_CASE_CUPPER;
        else if(isLower(cc))
            caseFlags |= SPELL_CASE_CLOWER;
        *dd++ = toInternalCharset(cc);
    }
    *dd = '\0';
    return (size_t) (dd-db);
}


static void
spellWordToDisplayCharset(meUByte *dd, meUByte *ss)
{
    meUByte cc;

    cc = *ss++;
    cc = toDisplayCharset(cc);
    if((caseFlags & SPELL_CASE_FUPPER) || (caseFlags == (SPELL_CASE_FLOWER|SPELL_CASE_CUPPER)))
        cc = toUpper(cc);
    *dd++ = cc;
    while((cc=*ss++) != '\0')
    {
        cc = toDisplayCharset(cc);
        if(caseFlags == (SPELL_CASE_FUPPER|SPELL_CASE_CUPPER))
            cc = toUpper(cc);
        *dd++ = cc;
    }
    *dd = '\0';
}

int
spellWord(int f, int n)
{
    meWordBuf wb;
    meUByte *wd=wb+meAFFIXBUF_SIZE_MAX;
    int len;

    if(meSpellInitDictionaries() <= 0)
        return meABORT;

    if(n & SPELLWORD_GET)
    {
        if(meGetString((meUByte *)"Enter word", MLNOSPACE,0,resultStr+1,meWORD_SIZE_MAX) <= 0)
            return meABORT;
        len = spellWordToInternalCharset(wd,resultStr+1);
    }
    else if(n & SPELLWORD_GETNEXT)
    {
        meWindow *cwp=frameCur->windowCur;
        meUByte  cc, chkDbl, curDbl;
        meUShort soff, eoff;
        meWordBuf lword;

        chkDbl = (n & SPELLWORD_DOUBLE);
        curDbl = 0;
        while((cwp->dotOffset > 0) && isAlphaNum(meLineGetChar(cwp->dotLine,cwp->dotOffset)))
            cwp->dotOffset--;
        for(;;)
        {
            while(((cc = meLineGetChar(cwp->dotLine,cwp->dotOffset)) != '.') &&
                  !isAlphaNum(cc))
            {
                if(!isSpace(cc))
                    curDbl = 0;
                if(meWindowForwardChar(cwp, 1) == meFALSE)
                {
                    resultStr[0] = 'F';
                    resultStr[1] = '\0';
                    return meTRUE;
                }
            }
            soff = cwp->dotOffset;
            if(meSpellGetCurrentWord((meUByte *) resultStr+1) < 0)
                continue;
            eoff = cwp->dotOffset;
            if(curDbl && !meStricmp(lword,resultStr+1))
            {
                resultStr[0] = 'D';
                setShowRegion(cwp->buffer,cwp->dotLineNo,soff,cwp->dotLineNo,eoff);
                cwp->updateFlags |= WFMOVEL|WFSELHIL;
                return meTRUE;
            }
            len = spellWordToInternalCharset(wd,(meUByte *) resultStr+1);
            if(wordCheck(wd,len) <= 0)
                break;
            if(chkDbl)
            {
                /* store the last word for double word check */
                meStrcpy(lword,resultStr+1);
                curDbl = 1;
            }
        }
        setShowRegion(cwp->buffer,cwp->dotLineNo,soff,cwp->dotLineNo,eoff);
        cwp->updateFlags |= WFMOVEL|WFSELHIL;
    }
    else
    {
        meWindow *cwp=frameCur->windowCur;
        meUByte  cc;
        meUShort off, soff, eoff;

        soff = off = cwp->dotOffset;
        for(;;soff--)
        {
            cc = meLineGetChar(cwp->dotLine,soff);
            if((soff==0) || isSpllWord(cc))
                break;
        }
        if(!isSpllWord(cc))
            return meFALSE;
        while(soff > 0)
        {
            --soff;
            cc = meLineGetChar(cwp->dotLine,soff);
            if(!isSpllWord(cc))
            {
                soff++;
                break;
            }
            if(!isAlphaNum(cc) &&
               ((soff == 0) || !isAlphaNum(meLineGetChar(cwp->dotLine,soff-1))))
                break;
        }
        /* if the first character is not alphanumeric or a '.' then move
         * on one, this stops misspellings of 'quoted words'
         */
        if(((cc = meLineGetChar(cwp->dotLine,soff)) != '.') && !isAlphaNum(cc))
            soff++;
        cwp->dotOffset = soff;
        len = meSpellGetCurrentWord((meUByte *) resultStr+1);
        eoff = cwp->dotOffset;
        cwp->dotOffset = off;
        setShowRegion(cwp->buffer,cwp->dotLineNo,soff,cwp->dotLineNo,eoff);
        cwp->updateFlags |= WFMOVEL|WFSELHIL;
        if(len < 0)
        {
            resultStr[0] = 'N';
            return meTRUE;
        }
        len = spellWordToInternalCharset(wd,(meUByte *) resultStr+1);
    }
    if(n & SPELLWORD_ADD)
    {
        meDictAddWord  dwb;
        meDictionary  *dict;
        meDictWord    *dw=(meDictWord *) &dwb;

        memcpy(meWordGetWord(dw),wd,len);
        meWordSetWordLen(dw,len);

        if(meGetString((meUByte *)"Enter flags", MLNOSPACE,0,wd,meWORD_FLAG_MAX) <= 0)
            return meABORT;
        len = meStrlen(wd);
        memcpy(meWordGetFlag(dw),wd,len);
        /* set flag len directly to clear the ERROR flag */
        dw->flagLen = len;
        if(n & SPELLWORD_ERROR)
            meWordSetErrorFlag(dw);
        else if(n & SPELLWORD_NOSGGST)
            meWordSetNoSuggestFlag(dw);
        if(n & SPELLWORD_IGNORE)
            dict = dictIgnr;
        else
            dict = dictHead;
        return meDictionaryAddWord(dict,dw);
    }
    if(n & SPELLWORD_DELETE)
    {
        meDictionary  *dict;

        if(n & SPELLWORD_IGNORE)
            meDictionaryDeleteWord(dictIgnr,wd,len);
        else
        {
            dict = dictHead;
            while(dict != NULL)
            {
                meDictionaryDeleteWord(dict,wd,len);
                dict = dict->next;
            }
        }
        return meTRUE;
    }
    if(n & SPELLWORD_DERIV)
    {
        meSpellRule  *rr;
        meUByte *nwd, *swd;

        if(n & SPELLWORD_INFO)
        {
            /* dump all derivatives into the current buffer */
            int ii;

            if(meModeTest(frameCur->windowCur->buffer->mode,MDVIEW))
                /* don't allow character insert if in read only */
                return (rdonly());

            if(wordCheck(wd,len) > 0)
            {
                lineInsertString(len,wd);
                lineInsertNewline(0);
            }
            ii = meRuleMax;
            do
            {
                rr = meRuleTable[ii];
                if(meRuleFlags[ii] & RULE_SUFFIX)
                {
                    while(rr != NULL)
                    {
                        if((swd = wordApplySuffixRule(wd,len,rr)) != NULL)
                        {
                            /* got <base>S1(bs) */
                            int slen = len + rr->changeLen;
                            meSpellRule  *ssr, *psr;
                            meUByte *sswd, *pwd;
                            int ssi, psi, sslen, jj;
                            
                            if(wordCheck(wd,slen) > 0)
                            {
                                lineInsertString(slen,wd);
                                lineInsertNewline(0);
                            }

                            if((ssi=rr->subRuleLen) > 0)
                            {
                                ssi--;
                                do {
                                    ssr = meRuleTable[rr->subRule[ssi]-SPELLRULE_OFFST];
                                    if(meRuleFlags[rr->subRule[ssi]-SPELLRULE_OFFST] & RULE_SUFFIX)
                                    {
                                        while(ssr != NULL)
                                        {
                                            if((sswd = wordApplySuffixRule(wd,slen,ssr)) != NULL)
                                            {
                                                /* got <base>S1(bs)S2(S1) */
                                                sslen = slen + ssr->changeLen;
                                                if(wordCheck(wd,sslen) > 0)
                                                {
                                                    lineInsertString(sslen,wd);
                                                    lineInsertNewline(0);
                                                }
                                                psi = rr->subRuleLen-1;
                                                do {
                                                    if(meRuleFlags[rr->subRule[psi]-SPELLRULE_OFFST] & RULE_PREFIX)
                                                    {
                                                        psr = meRuleTable[rr->subRule[psi]-SPELLRULE_OFFST];
                                                        while(psr != NULL)
                                                        {
                                                            if((pwd = wordApplyPrefixRule(wd,sslen,psr)) != NULL)
                                                            {
                                                                /* got P1(S1)<base>S1(bs)S2(S1) */
                                                                if(wordCheck(pwd,sslen+psr->changeLen) > 0)
                                                                {
                                                                    lineInsertString(sslen+psr->changeLen,pwd);
                                                                    lineInsertNewline(0);
                                                                }
                                                                wordPrefixRuleRemove(wd,psr);
                                                            }
                                                            psr = psr->next;
                                                        }
                                                    }
                                                } while(--psi >= 0);
                                                if((psi = ssr->subRuleLen) > 0)
                                                {
                                                    psi--;
                                                    do {
                                                        if(meRuleFlags[ssr->subRule[psi]-SPELLRULE_OFFST] & RULE_PREFIX)
                                                        {
                                                            psr = meRuleTable[ssr->subRule[psi]-SPELLRULE_OFFST];
                                                            while(psr != NULL)
                                                            {
                                                                if((pwd = wordApplyPrefixRule(wd,sslen,psr)) != NULL)
                                                                {
                                                                    /* got P1(S2)<base>S1(bs)S2(S1) */
                                                                    if(wordCheck(pwd,sslen+psr->changeLen) > 0)
                                                                    {
                                                                        lineInsertString(sslen+psr->changeLen,pwd);
                                                                        lineInsertNewline(0);
                                                                    }
                                                                    wordPrefixRuleRemove(wd,psr);
                                                                }
                                                                psr = psr->next;
                                                            }
                                                        }
                                                    } while(--psi >= 0);
                                                }
                                                if((meRuleFlags[ii] & RULE_MIXABLE) == RULE_MIXABLE)
                                                {
                                                    jj = meRuleMax;
                                                    do
                                                    {
                                                        if((meRuleFlags[jj] & (RULE_PREFIX|RULE_MIXABLE)) == (RULE_PREFIX|RULE_MIXABLE))
                                                        {
                                                            psr = meRuleTable[jj];
                                                            while(psr != NULL)
                                                            {
                                                                if((pwd = wordApplyPrefixRule(wd,sslen,psr)) != NULL)
                                                                {
                                                                    /* got P1(bs)<base>S1(bs)S2(S1) */
                                                                    if(wordCheck(pwd,sslen+psr->changeLen) > 0)
                                                                    {
                                                                        lineInsertString(sslen+psr->changeLen,pwd);
                                                                        lineInsertNewline(0);
                                                                    }
                                                                    wordPrefixRuleRemove(wd,psr);
                                                                }
                                                                psr = psr->next;
                                                            }
                                                        }
                                                    } while(--jj > 0);
                                                }
                                                wordSuffixRuleRemove(sswd,ssr);
                                            }
                                            ssr = ssr->next;
                                        }
                                    }
                                    else
                                    {
                                        while(ssr != NULL)
                                        {
                                            if((pwd = wordApplyPrefixRule(wd,slen,ssr)) != NULL)
                                            {
                                                /* got P1(S1)<base>S1(bs) */
                                                if(wordCheck(pwd,slen+ssr->changeLen) > 0)
                                                {
                                                    lineInsertString(slen+ssr->changeLen,pwd);
                                                    lineInsertNewline(0);
                                                }
                                                wordPrefixRuleRemove(wd,ssr);
                                            }
                                            ssr = ssr->next;
                                        }
                                    }
                                } while(--ssi >= 0);
                            }
                            if((meRuleFlags[ii] & RULE_MIXABLE) == RULE_MIXABLE)
                            {
                                jj = meRuleMax;
                                do
                                {
                                    if((meRuleFlags[jj] & (RULE_PREFIX|RULE_MIXABLE)) == (RULE_PREFIX|RULE_MIXABLE))
                                    {
                                        psr = meRuleTable[jj];
                                        while(psr != NULL)
                                        {
                                            if((pwd = wordApplyPrefixRule(wd,slen,psr)) != NULL)
                                            {
                                                /* got P1(bs)<base>S1(bs) */
                                                if(wordCheck(pwd,slen+psr->changeLen) > 0)
                                                {
                                                    lineInsertString(slen+psr->changeLen,pwd);
                                                    lineInsertNewline(0);
                                                }
                                                wordPrefixRuleRemove(wd,psr);
                                            }
                                            psr = psr->next;
                                        }
                                    }
                                } while(--jj > 0);
                            }
                            wordSuffixRuleRemove(swd,rr);
                        }
                        rr = rr->next;
                    }
                }
                else if(meRuleFlags[ii] & RULE_PREFIX)
                {
                    while(rr != NULL)
                    {
                        if((nwd = wordApplyPrefixRule(wd,len,rr)) != NULL)
                        {
                            /* got P1(bs)<base> */
                            int psi, plen=len+rr->changeLen;
                            if(wordCheck(nwd,plen) > 0)
                            {
                                lineInsertString(plen,nwd);
                                lineInsertNewline(0);
                            }
                            if((psi=rr->subRuleLen) > 0)
                            {
                                meSpellRule  *psr, *ssr;
                                meUByte *sswd;
                                int ssi, pslen;
                                psi--;
                                do {
                                    if(meRuleFlags[rr->subRule[psi]-SPELLRULE_OFFST] & RULE_SUFFIX)
                                    {
                                        psr = meRuleTable[rr->subRule[psi]-SPELLRULE_OFFST];
                                        while(psr != NULL)
                                        {
                                            if((swd = wordApplySuffixRule(nwd,plen,psr)) != NULL)
                                            {
                                                /* got P1(bs)<base>S1(P1) */
                                                pslen = plen + psr->changeLen;
                                                if(wordCheck(nwd,pslen) > 0)
                                                {
                                                    lineInsertString(pslen,nwd);
                                                    lineInsertNewline(0);
                                                }
                                                if((ssi=psr->subRuleLen) > 0)
                                                {
                                                    ssi--;
                                                    do {
                                                        if(meRuleFlags[psr->subRule[ssi]-SPELLRULE_OFFST] & RULE_SUFFIX)
                                                        {
                                                            ssr = meRuleTable[psr->subRule[ssi]-SPELLRULE_OFFST];
                                                            while(ssr != NULL)
                                                            {
                                                                if((sswd = wordApplySuffixRule(nwd,pslen,ssr)) != NULL)
                                                                {
                                                                    /* got P1(bs)<base>S1(P1)S2(S1) */
                                                                    if(wordCheck(nwd,pslen+ssr->changeLen) > 0)
                                                                    {
                                                                        lineInsertString(pslen+ssr->changeLen,nwd);
                                                                        lineInsertNewline(0);
                                                                    }
                                                                    wordSuffixRuleRemove(sswd,ssr);
                                                                }
                                                                ssr = ssr->next;
                                                            }
                                                        }
                                                    } while(--ssi >= 0);
                                                }
                                                wordSuffixRuleRemove(swd,psr);
                                            }
                                            psr = psr->next;
                                        }
                                    }
                                } while(--psi >= 0);
                            }
                            wordPrefixRuleRemove(wd,rr);
                        }
                        rr = rr->next;
                    }
                }
            } while(--ii > 0);
            return meTRUE;
        }
        if(((f = mlCharReply((meUByte *)"Rule flag: ",0,NULL,NULL)) < SPELLRULE_FIRST) || (f > SPELLRULE_LAST))
            return meABORT;
        f -= SPELLRULE_OFFST;
        rr = meRuleTable[f];
        if(meRuleFlags[f] & RULE_PREFIX)
        {
            while(rr != NULL)
            {
                if((nwd = wordApplyPrefixRule(wd,len,rr)) != NULL)
                {
                    spellWordToDisplayCharset((meUByte *) resultStr,nwd);
                    return meTRUE;
                }
                rr = rr->next;
            }
        }
        else
        {
            while(rr != NULL)
            {
                if(wordApplySuffixRule(wd,len,rr) != NULL)
                {
                    spellWordToDisplayCharset((meUByte *) resultStr,wd);
                    return meTRUE;
                }
                rr = rr->next;
            }
        }
        resultStr[0] = '\0';
        return meTRUE;
    }
    if(n & SPELLWORD_GUESS)
    {
        meWordBuf words[meGUESS_MAX];
        meUByte *ss, *dd;
        int nw, cw, ll, ii;

        if((nw = wordGuessGetList(wd,len,words)) < 0)
            return meABORT;
        dd = (meUByte *) resultStr;
        *dd++ = '|';
        cw = 0;
        ll = meBUF_SIZE_MAX-3;
        while((cw < nw) && ((ii=meStrlen((ss=words[cw++]))) < ll))
        {
            spellWordToDisplayCharset(dd,ss);
            dd += ii;
            *dd++ = '|';
            ll -= ii+1;
        }
        *dd = '\0';
        return meTRUE;
    }
    wordCurr = NULL;
    if(((f=wordCheck(wd,len)) == meABORT) && (wordCurr != NULL))
    {
        resultStr[0] = 'A';
        f = meWordGetFlagLen(wordCurr);
        memcpy(resultStr+1,meWordGetFlag(wordCurr),f);
        resultStr[f+1] = '\0';
    }
    else if(f == meTRUE)
        resultStr[0] = 'O';
    else
        resultStr[0] = 'E';
    if((n & SPELLWORD_INFO) && (wordCurr != NULL))
        meDictWordDump(wordCurr,(meUByte *) resultStr+1);
    return meTRUE;
}


int
findWordsInit(meUByte *mask)
{
    int rr;
    if(sfwMsk != NULL)
    {
        free(sfwMsk);
        sfwMsk = NULL;
    }
    if((rr=meSpellInitDictionaries()) <= 0)
        return rr;

    sfwWrd = NULL;
    sfwDct = dictHead;
    sfwRp1 = NULL;
    sfwRp2 = NULL;
    sfwRp3 = NULL;
    sfwPlc = 0;
    sfwWfl = 0;
    if((sfwMsk = meMalloc(meStrlen(mask)+1)) != NULL)
    {
        meUByte *ww, cc;
        ww = sfwMsk;
        while((cc = *mask++) != '\0')
            *ww++ = toInternalCharset(cc);
        *ww = '\0';
    }
    return meTRUE;
}

meUByte *
findWordsNext(void)
{
    meUByte *ww, *wp, *wFp, *wRs1, *wRs2, *wRs3;
    meInt off, wl, wRl1, wRl2, wRf1, wRf2, wRf3;
    meUByte wFl0, wFl1, wFl2;

    if(sfwMsk == NULL)
        return abortm;
    caseFlags = SPELL_CASE_FLOWER|SPELL_CASE_CLOWER;
    wp = evalResult+meAFFIXBUF_SIZE_MAX;
    if(sfwWrd != NULL)
    {
        wl = meWordGetWordLen(sfwWrd);
        memcpy(wp,meWordGetWord(sfwWrd),wl);
        wp[wl] = '\0';
        if(sfwRp1 == NULL)
            goto sfwJumpBase;
        wFp = meWordGetFlag(sfwWrd);
        wFl0 = sfwRp1->rule;
        wRf1 = wFp[wFl0]-SPELLRULE_OFFST;
        wRl1 = wl + sfwRp1->changeLen;
        if(meRuleFlags[wRf1] & RULE_PREFIX)
        {
            wRs1 = wordApplyPrefixRule(wp,wl,sfwRp1);
            if(sfwRp2 == NULL)
                goto sfwJumpP1bsBase;
            wFl1 = sfwRp2->rule;
            wRf2 = sfwRp1->subRule[wFl1]-SPELLRULE_OFFST;
            wRl2 = wRl1 + sfwRp2->changeLen;
            wRs2 = wordApplySuffixRule(wRs1,wRl1,sfwRp2);
            if(sfwRp3 == NULL)
                goto sfwJumpP1bsBaseS1p1;
            wFl2 = sfwRp3->rule;
            wRf3 = sfwRp2->subRule[wFl2]-SPELLRULE_OFFST;
            goto sfwJumpP1bsBaseS1p1S2s1;
        }
        wRs1 = wordApplySuffixRule(wp,wl,sfwRp1);
        if(sfwRp2 == NULL)
        {
            if(sfwRp3 == NULL)
                goto sfwJumpBaseS1bs;
            wFl2 = sfwRp3->rule;
            wRf3 = wFp[wFl2]-SPELLRULE_OFFST;
            goto sfwJumpP1bsBaseS1bs;
        }
        wFl1 = sfwRp2->rule;
        wRf2 = sfwRp1->subRule[wFl1]-SPELLRULE_OFFST;
        if(meRuleFlags[wRf2] & RULE_PREFIX)
            goto sfwJumpP1s1BaseS1bs;
        wRl2 = wRl1 + sfwRp2->changeLen;
        wRs2 = wordApplySuffixRule(wp,wRl1,sfwRp2);
        if(sfwRp3 == NULL)
            goto sfwJumpBaseS1bsS2s1;
        wFl2 = sfwRp3->rule;
        if(!sfwPlc)
        {
            wRf3 = wFp[wFl2]-SPELLRULE_OFFST;
            goto sfwJumpP1bsBaseS1bsS2s1;
        }
        if(sfwPlc == 1)
        {
            wRf3 = sfwRp1->subRule[wFl2]-SPELLRULE_OFFST;
            goto sfwJumpP1s1BaseS1bsS2s1;
        }
        wRf3 = sfwRp2->subRule[wFl2]-SPELLRULE_OFFST;
        goto sfwJumpP1s2BaseS1bsS2s1;
    }
    while(sfwDct != NULL)
    {
        for(sfwDctI=0 ; sfwDctI<sfwDct->tableSize ; sfwDctI++)
        {
            off = meEntryGetAddr(sfwDct->table[sfwDctI]);
            while(off != 0)
            {
                sfwWrd = (meDictWord *) mePtrOffset(sfwDct->table,off);
                if(!meWordGetErrorFlag(sfwWrd) && ((ww=meWordGetWord(sfwWrd))[0] != '\0'))
                {
                    /* got <base> */
                    wl = meWordGetWordLen(sfwWrd);
                    memcpy(wp,ww,wl);
                    wp[wl] = '\0';
                    if(regexStrCmp(wp,sfwMsk,meRSTRCMP_ICASE|meRSTRCMP_WHOLE) > 0)
                    {
                        spellWordToDisplayCharset(wp,wp);
                        return wp;
                    }
sfwJumpBase:
                    if((sfwWfl = meWordGetFlagLen(sfwWrd)) > 0)
                    {
                        wFl0 = sfwWfl;
                        wFp = meWordGetFlag(sfwWrd);
                        do {
                            if((wRf1 = wFp[--wFl0]-SPELLRULE_OFFST) <= 0)
                                sfwWfl = wFl0;
                            else if((sfwRp1 = meRuleTable[wRf1]) != NULL)
                            {
                                if(meRuleFlags[wRf1] & RULE_PREFIX)
                                {
                                    do
                                    {
                                        if((wRs1 = wordApplyPrefixRule(wp,wl,sfwRp1)) != NULL)
                                        {
                                            /* got P1(bs)<base> */
                                            sfwRp1->rule = wFl0;
                                            wRl1 = wl + sfwRp1->changeLen;
                                            wRs1[wRl1] = '\0';
                                            if(regexStrCmp(wRs1,sfwMsk,meRSTRCMP_ICASE|meRSTRCMP_WHOLE) > 0)
                                            {
                                                spellWordToDisplayCharset(wRs1,wRs1);
                                                return wRs1;
                                            }
sfwJumpP1bsBase:
                                            if((wFl1 = sfwRp1->subRuleLen) > 0)
                                            {
                                                do {
                                                    wRf2 = sfwRp1->subRule[--wFl1]-SPELLRULE_OFFST;
                                                    if(meRuleFlags[wRf2] & RULE_SUFFIX)
                                                    {
                                                        sfwRp2 = meRuleTable[wRf2];
                                                        while(sfwRp2 != NULL)
                                                        {
                                                            if((wRs2 = wordApplySuffixRule(wRs1,wRl1,sfwRp2)) != NULL)
                                                            {
                                                                sfwRp2->rule = wFl1;
                                                                wRl2 = wRl1 + sfwRp2->changeLen;
                                                                wRs1[wRl2] = '\0';
                                                                if(regexStrCmp(wRs1,sfwMsk,meRSTRCMP_ICASE|meRSTRCMP_WHOLE) > 0)
                                                                {
                                                                    /* got P1(bs)<base>S1(P1) */
                                                                    spellWordToDisplayCharset(wRs1,wRs1);
                                                                    return wRs1;
                                                                }
sfwJumpP1bsBaseS1p1:
                                                                if((wFl2 = sfwRp2->subRuleLen) > 0)
                                                                {
                                                                    do {
                                                                        wRf3 = sfwRp2->subRule[--wFl2]-SPELLRULE_OFFST;
                                                                        if((wRf3 != wRf2) && (meRuleFlags[wRf3] & RULE_SUFFIX))
                                                                        {
                                                                            sfwRp3 = meRuleTable[wRf3];
                                                                            while(sfwRp3 != NULL)
                                                                            {
                                                                                if((wRs3 = wordApplySuffixRule(wRs1,wRl2,sfwRp3)) != NULL)
                                                                                {
                                                                                    wRs1[wRl2+sfwRp3->changeLen] = '\0';
                                                                                    if(regexStrCmp(wRs1,sfwMsk,meRSTRCMP_ICASE|meRSTRCMP_WHOLE) > 0)
                                                                                    {
                                                                                        /* got P1(bs)<base>S1(P1)S2(S1) */
                                                                                        sfwRp3->rule = wFl2;
                                                                                        spellWordToDisplayCharset(wRs1,wRs1);
                                                                                        return wRs1;
                                                                                    }
                                                                                    wordSuffixRuleRemove(wRs3,sfwRp3);
                                                                                }
sfwJumpP1bsBaseS1p1S2s1:
                                                                                sfwRp3 = sfwRp3->next;
                                                                            }
                                                                        }
                                                                    } while(wFl1 > 0);
                                                                }
                                                                wordSuffixRuleRemove(wRs2,sfwRp2);
                                                            }
                                                            sfwRp2 = sfwRp2->next;
                                                        }
                                                    }
                                                } while(wFl1 > 0);
                                            }
                                            wordPrefixRuleRemove(wp,sfwRp1);
                                        }
                                    } while((sfwRp1 = sfwRp1->next) != NULL);

                                }
                                else
                                {
                                    do
                                    {
                                        if((wRs1 = wordApplySuffixRule(wp,wl,sfwRp1)) != NULL)
                                        {
                                            sfwRp1->rule = wFl0;
                                            wRl1 = wl + sfwRp1->changeLen;
                                            wp[wRl1] = '\0';
                                            if(regexStrCmp(wp,sfwMsk,meRSTRCMP_ICASE|meRSTRCMP_WHOLE) > 0)
                                            {
                                                /* got <base>S1(bs) */
                                                spellWordToDisplayCharset(wp,wp);
                                                return wp;
                                            }
sfwJumpBaseS1bs:
                                            if(meRuleFlags[wRf1] & RULE_MIXABLE)
                                            {
                                                wFl2 = sfwWfl;
                                                do {
                                                    if((wRf3 = wFp[--wFl2]-SPELLRULE_OFFST) <= 0)
                                                        break;
                                                    if((meRuleFlags[wRf3] & (RULE_PREFIX|RULE_MIXABLE)) == (RULE_PREFIX|RULE_MIXABLE))
                                                    {
                                                        sfwRp3 = meRuleTable[wRf3];
                                                        while(sfwRp3 != NULL)
                                                        {
                                                            if((wRs3 = wordApplyPrefixRule(wp,wRl1,sfwRp3)) != NULL)
                                                            {
                                                                wRs3[wRl1+sfwRp3->changeLen] = '\0';
                                                                if(regexStrCmp(wRs3,sfwMsk,meRSTRCMP_ICASE|meRSTRCMP_WHOLE) > 0)
                                                                {
                                                                    /* got P1(bs)<base>S1(bs) */
                                                                    sfwRp3->rule = wFl2;
                                                                    spellWordToDisplayCharset(wRs3,wRs3);
                                                                    return wRs3;
                                                                }
                                                                wordPrefixRuleRemove(wp,sfwRp3);
                                                            }
sfwJumpP1bsBaseS1bs:
                                                            sfwRp3 = sfwRp3->next;
                                                        }
                                                    }
                                                } while(wFl2 > 0);
                                            }
                                            if((wFl1 = sfwRp1->subRuleLen) > 0)
                                            {
                                                do {
                                                    wRf2 = sfwRp1->subRule[--wFl1]-SPELLRULE_OFFST;
                                                    sfwRp2 = meRuleTable[wRf2];
                                                    if(meRuleFlags[wRf2] & RULE_PREFIX)
                                                    {
                                                        while(sfwRp2 != NULL)
                                                        {
                                                            if((wRs2 = wordApplyPrefixRule(wp,wRl1,sfwRp2)) != NULL)
                                                            {
                                                                wRs2[wRl1+sfwRp2->changeLen] = '\0';
                                                                if(regexStrCmp(wRs2,sfwMsk,meRSTRCMP_ICASE|meRSTRCMP_WHOLE) > 0)
                                                                {
                                                                    /* got P1(S1)<base>S1(bs) */
                                                                    sfwRp2->rule = wFl1;
                                                                    spellWordToDisplayCharset(wRs2,wRs2);
                                                                    return wRs2;
                                                                }
                                                                wordPrefixRuleRemove(wp,sfwRp2);
                                                            }
sfwJumpP1s1BaseS1bs:
                                                            sfwRp2 = sfwRp2->next;
                                                        }
                                                    }
                                                    else if(wRf2 != wRf1)
                                                    {
                                                        while(sfwRp2 != NULL)
                                                        {
                                                            if((wRs2 = wordApplySuffixRule(wp,wRl1,sfwRp2)) != NULL)
                                                            {
                                                                sfwRp2->rule = wFl1;
                                                                wRl2 = wRl1 + sfwRp2->changeLen;
                                                                wp[wRl2] = '\0';
                                                                if(regexStrCmp(wp,sfwMsk,meRSTRCMP_ICASE|meRSTRCMP_WHOLE) > 0)
                                                                {
                                                                    /* got <base>S1(bs)S2(S1) */
                                                                    spellWordToDisplayCharset(wp,wp);
                                                                    return wp;
                                                                }
sfwJumpBaseS1bsS2s1:
                                                                if(meRuleFlags[wRf1] & RULE_MIXABLE)
                                                                {
                                                                    wFl2 = sfwWfl;
                                                                    do {
                                                                        if((wRf3 = wFp[--wFl2]-SPELLRULE_OFFST) <= 0)
                                                                            break;
                                                                        if((meRuleFlags[wRf3] & (RULE_PREFIX|RULE_MIXABLE)) == (RULE_PREFIX|RULE_MIXABLE))
                                                                        {
                                                                            sfwRp3 = meRuleTable[wRf3];
                                                                            while(sfwRp3 != NULL)
                                                                            {
                                                                                if((wRs3 = wordApplyPrefixRule(wp,wRl2,sfwRp3)) != NULL)
                                                                                {
                                                                                    wRs3[wRl2+sfwRp3->changeLen] = '\0';
                                                                                    if(regexStrCmp(wRs3,sfwMsk,meRSTRCMP_ICASE|meRSTRCMP_WHOLE) > 0)
                                                                                    {
                                                                                        /* got P1(bs)<base>S1(bs)S2(S1) */
                                                                                        sfwPlc = 0;
                                                                                        sfwRp3->rule = wFl2;
                                                                                        spellWordToDisplayCharset(wRs3,wRs3);
                                                                                        return wRs3;
                                                                                    }
                                                                                    wordPrefixRuleRemove(wp,sfwRp3);
                                                                                }
sfwJumpP1bsBaseS1bsS2s1:
                                                                                sfwRp3 = sfwRp3->next;
                                                                            }
                                                                        }
                                                                    } while(wFl2 > 0);
                                                                }
                                                                wFl2 = sfwRp1->subRuleLen;
                                                                do {
                                                                    wRf3 = sfwRp1->subRule[--wFl2]-SPELLRULE_OFFST;
                                                                    if(meRuleFlags[wRf3] & RULE_PREFIX)
                                                                    {
                                                                        sfwRp3 = meRuleTable[wRf3];
                                                                        while(sfwRp3 != NULL)
                                                                        {
                                                                            if((wRs3 = wordApplyPrefixRule(wp,wRl2,sfwRp3)) != NULL)
                                                                            {
                                                                                wRs3[wRl2+sfwRp3->changeLen] = '\0';
                                                                                if(regexStrCmp(wRs3,sfwMsk,meRSTRCMP_ICASE|meRSTRCMP_WHOLE) > 0)
                                                                                {
                                                                                    /* got P1(S1)<base>S1(bs)S2(S1) */
                                                                                    sfwPlc = 1;
                                                                                    sfwRp3->rule = wFl2;
                                                                                    spellWordToDisplayCharset(wRs3,wRs3);
                                                                                    return wRs3;
                                                                                }
                                                                                wordPrefixRuleRemove(wp,sfwRp3);
                                                                            }
sfwJumpP1s1BaseS1bsS2s1:
                                                                            sfwRp3 = sfwRp3->next;
                                                                        }
                                                                    }
                                                                } while(wFl2 > 0);
                                                                if((wFl2 = sfwRp2->subRuleLen) > 0)
                                                                {
                                                                    do {
                                                                        wRf3 = sfwRp2->subRule[--wFl2]-SPELLRULE_OFFST;
                                                                        if(meRuleFlags[wRf3] & RULE_PREFIX)
                                                                        {
                                                                            sfwRp3 = meRuleTable[wRf3];
                                                                            while(sfwRp3 != NULL)
                                                                            {
                                                                                if((wRs3 = wordApplyPrefixRule(wp,wRl2,sfwRp3)) != NULL)
                                                                                {
                                                                                    wRs3[wRl2+sfwRp3->changeLen] = '\0';
                                                                                    if(regexStrCmp(wRs3,sfwMsk,meRSTRCMP_ICASE|meRSTRCMP_WHOLE) > 0)
                                                                                    {
                                                                                        /* got P1(S2)<base>S1(bs)S2(S1) */
                                                                                        sfwPlc = 2;
                                                                                        sfwRp3->rule = wFl2;
                                                                                        spellWordToDisplayCharset(wRs3,wRs3);
                                                                                        return wRs3;
                                                                                    }
                                                                                    wordPrefixRuleRemove(wp,sfwRp3);
                                                                                }
sfwJumpP1s2BaseS1bsS2s1:
                                                                                sfwRp3 = sfwRp3->next;
                                                                            }
                                                                        }
                                                                    } while(wFl2 > 0);
                                                                }
                                                                wordSuffixRuleRemove(wRs2,sfwRp2);
                                                            }
                                                            sfwRp2 = sfwRp2->next;
                                                        }
                                                    }
                                                } while(wFl1 > 0);
                                            }
                                            wordSuffixRuleRemove(wRs1,sfwRp1);
                                        }
                                    } while((sfwRp1 = sfwRp1->next) != NULL);
                                }
                            }
                        } while(wFl0 > 0);
                    }
                }
                off = meWordGetAddr(sfwWrd);
            }
        }
        sfwDct = sfwDct->next;
    }
    sfwWrd = NULL;
    return emptym;
}

#endif
