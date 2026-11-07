/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * search.c - Search routines.
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
 * Created:     07/05/85
 * Synopsis:    Search routines.
 * Authors:     Unknown, John M. Gamble, Jon Green & Steven Phillips
 * Description:
 *     The functions in this file implement commands that search in the forward
 *     and backward directions.  There are no special characters in the search
 *     strings.  Probably should have a regular expression search, or something
 *     like that.
 * 
 * Notes:
 *     The search engine now uses the regex routines defined in regex.c.
 */

/*--- Include defintions */

#define	__SEARCHC			/* Define filename */

/*--- Include files */

#include "emain.h"
#include "esearch.h"

/*--- Local macro definitions */

#define notFound() (mlwrite(MWABORT|MWCLEXEC,notFoundStr))

/*--- Local variable definitions */

static meUByte notFoundStr[] ="[Not Found]";
static int exactFlag ;                      /* Use to cache EXACT mode state */
meUByte  srchPat [meBUF_SIZE_MAX] ;         /* Search pattern array */
meUByte  srchRPat[meBUF_SIZE_MAX] ;         /* Reversed pattern array */
int      srchLastState=meFALSE ;            /* status of last search */
meUByte *srchLastMatch=NULL ;               /* pointer to the last match string */

#if MEOPT_MAGIC

int           srchLastMagic = 0 ;              /* last search was a magic        */
static int    reportErrors = meTRUE ;

int      mereNewlBufSz=0 ;
meUByte *mereNewlBuf=NULL ;

#define mereNewlBufSzCheck(ll)                                               \
do {                                                                         \
    if(ll >= mereNewlBufSz)                                                  \
    {                                                                        \
        if((mereNewlBuf = meRealloc(mereNewlBuf,ll+128)) == NULL)            \
        {                                                                    \
            mereNewlBufSz = 0 ;                                              \
            return meFALSE ;                                                 \
        }                                                                    \
        mereNewlBufSz = ll+127 ;                                             \
    }                                                                        \
} while(0)

meRegex mereRegex ;

static int
mere_compile_pattern(meUByte *apat)
{
    int ii ;
    
    if(((ii=meRegexCompCache(&mereRegex,apat,
                        (exactFlag) ? meREGEX_ICASE:0)) != meREGEX_OKAY) && reportErrors)
        mlwrite(MWABORT,(meUByte *)"[Regex Error: %s]",meRegexCompErrors[ii]) ;
    return ii ;
}

static int
mere_scanner(int direct, int beg_or_end, int *n, SCANNERPOS *sp)
{
    meWindow *cwp=frameCur->windowCur;
    register meLine *lp=cwp->dotLine, *nlp;
    register meInt lnno=cwp->dotLineNo, nlnno;
    register int   ii=cwp->dotOffset, jj, kk, count=*n, flags;
    
    srchLastState = meFALSE ;
        
    if(mereRegex.newlNo == 0)
    {
        /* Moved the TTbreakTest() to the start of the loop as some of the
         * main problems are caused by search string with no length, such as ^,
         * or [ ]*$ on a line with no end spaces. In these cases is the user
         * is trying to replace it then they couldn't interupt as this
         * immediately returned success, hence the move!
         */
        if(TTbreakTest(1))
        {
            ctrlg(meFALSE,1) ;
            return meFALSE ;
        }
        flags = (lp == cwp->buffer->baseLine) ? meREGEX_ENDBUFF:0 ;
        if(lnno == 0)
            flags |= meREGEX_BEGBUFF ;
        jj = meLineGetLength(lp) ;
        if(direct == meFORWARD)
        {
            if((lp == cwp->buffer->baseLine) &&
               (!(mereRegex.flags & meREGEX_MAYENDBUFF) ||
                !(mereRegex.flags & meREGEX_MAYBEEMPTY) ))
                /* never match $ or \n on the last line going forwards, only \' */
                return meFALSE ;
            kk = jj ;
        }
        else
        {
            flags |= meREGEX_BACKWARD ;
            /* when searching backward, we must not match the current position
             * forwrds, always the char before and onwards. if ii == 0 then its now -1! */
#if MEOPT_UNICODE
            kk = (ii > 0) ? meUtf8Prev(meLineGetText(lp),ii):-1 ;
#else
            kk = ii - 1 ;
#endif
            ii = 0 ;
        }
        if((kk < 0) || ((ii=meRegexMatch(&mereRegex,meLineGetText(lp),jj,ii,kk,flags)) == 0))
        {
            if(direct == meFORWARD)
            {
                flags &= ~meREGEX_BEGBUFF ;
                /* On entry, if count=0 then no line limit, therefore test if (--count == 0) as count == -1 */ 
                while((--count != 0) && !(flags & meREGEX_ENDBUFF))
                {
                    if(TTbreakTest(1))
                    {
                        ctrlg(meFALSE,1) ;
                        return meFALSE ;
                    }
                    lnno++ ;
                    if((lp=meLineGetNext(lp)) == cwp->buffer->baseLine)
                    {
                        if(!(mereRegex.flags & meREGEX_MAYENDBUFF) ||
                           !(mereRegex.flags & meREGEX_MAYBEEMPTY) )
                            break ;
                        flags |= meREGEX_ENDBUFF ;
                    }
                    ii = meLineGetLength(lp) ;
                    if((ii=meRegexMatch(&mereRegex,meLineGetText(lp),ii,0,ii,flags)) != 0)
                        break ;
                }
            }
            else
            {
                flags &= ~meREGEX_ENDBUFF ;
                while((--count != 0) && !(flags & meREGEX_BEGBUFF))
                {
                    if(TTbreakTest(1))
                    {
                        ctrlg(meFALSE,1) ;
                        return meFALSE ;
                    }
                    lp = meLineGetPrev(lp) ;
                    ii = meLineGetLength(lp) ;
                    if(--lnno == 0)
                        flags |= meREGEX_BEGBUFF ;
                    if((ii=meRegexMatch(&mereRegex,meLineGetText(lp),ii,0,ii,flags)) != 0)
                        break ;
                }
            }
            if(!ii)
                return meFALSE ;
        }
        ii = mereRegex.group[0].start ;
        kk = mereRegex.group[0].end ;
        nlp = lp ;
        nlnno = lnno ;
        jj = kk - ii ;
        mereNewlBufSzCheck(jj) ;
        memcpy(mereNewlBuf,meLineGetText(lp)+ii,jj) ;
        srchLastMatch = mereNewlBuf ;
    }
    else
    {
        /* generic multi-line case */
        int noln=mereRegex.newlNo+1, offs, offe ;
        
        if(direct == meFORWARD)
        {
            offs = ii ;
            offe = meLineGetLength(lp)+1 ;
        }
        else
        {
#if MEOPT_UNICODE
            if((offe = (ii > 0) ? meUtf8Prev(meLineGetText(lp),ii):-1) < 0)
#else
            if((offe = ii-1) < 0)
#endif
            {
                if(--lnno < 0)
                    return meFALSE ;
                lp = meLineGetPrev(lp) ;
                offe = meLineGetLength(lp) ;
            }
            offs = 0 ;
        }
        do
        {
            if(TTbreakTest(1))
            {
                ctrlg(meFALSE,1) ;
                return meFALSE ;
            }
            flags = (lnno == 0) ? meREGEX_BEGBUFF:0 ;
            ii = noln ;
            jj = ii ;
            nlp = lp ;
            do
            {
                if(nlp == cwp->buffer->baseLine)
                    break ;
                jj += meLineGetLength(nlp) ;
                nlp = meLineGetNext(nlp) ;
            } while(--ii > 0) ;
            mereNewlBufSzCheck(jj) ;
            if(ii)
            {
                flags |= meREGEX_ENDBUFF ;
                ii = noln - ii + 1 ;
            }
            else
                ii = noln ;
            jj = 0 ;
            nlp = lp ;
            for(;;)
            {
                kk = meLineGetLength(nlp) ;
                memcpy(mereNewlBuf+jj,meLineGetText(nlp),kk) ;
                jj += kk ;
                /* we don't want the last \n or move nlp to past the
                 * last line so drop out here */
                if(--ii == 0)
                    break ;
                mereNewlBuf[jj++] = meCHAR_NL ;
                nlp = meLineGetNext(nlp) ;
            }
            mereNewlBuf[jj] = '\0' ;
            
            if(direct != meFORWARD)
                flags |= meREGEX_BACKWARD ;
            ii = meRegexMatch(&mereRegex,mereNewlBuf,jj,offs,offe,flags) ;
            if(ii)
                break ;
            
            if(direct == meFORWARD)
            {
                if(lp == cwp->buffer->baseLine)
                    break ;
                lp = meLineGetNext(lp) ;
                lnno++ ;
            }
            else
            {
                if(flags & meREGEX_BEGBUFF)
                    break ;
                lp = meLineGetPrev(lp) ;
                lnno-- ;
            }
            offs = 0 ;
            offe = meLineGetLength(lp)+1 ;
        } while(--count != 0) ;
        
        if(!ii)
            return meFALSE ;
        
        ii = mereRegex.group[0].start ;
        kk = mereRegex.group[0].end ;
        srchLastMatch = mereNewlBuf+ii ;
        while(ii > meLineGetLength(lp))
        {
            jj = meLineGetLength(lp)+1 ;
            ii -= jj ;
            kk -= jj ;
            lnno++ ;
            lp = meLineGetNext(lp) ;
        }
        nlp = lp ;
        nlnno = lnno ;
        while(kk > meLineGetLength(nlp))
        {
            kk -= meLineGetLength(nlp)+1 ;
            nlnno++ ;
            nlp = meLineGetNext(nlp) ;
        }
    }    
    if(beg_or_end == PTEND)
    {
        cwp->dotLine = nlp ;
        cwp->dotLineNo = nlnno ;
        cwp->dotOffset = kk ;
    }
    else
    {
        cwp->dotLine = lp ;
        cwp->dotLineNo = lnno ;
        cwp->dotOffset = ii ;
    }
    if (sp != NULL)
    {
        sp->startline = lp ;
        sp->startline_no = lnno ;
        sp->startoff = ii ;
        sp->endline = nlp ;
        sp->endline_no = nlnno ;
        sp->endoffset = kk ;
    }
    setShowRegion(cwp->buffer,lnno,ii,nlnno,kk) ;
    cwp->updateFlags |= WFMOVEL|WFSELHIL ;
    /* change the start and end position */
    mereRegexGroupEnd(0) -= mereRegex.group[0].start ;
    mereRegexGroupStart(0) = 0 ;
    srchLastMatch[mereRegexGroupEnd(0)] = '\0' ;
    srchLastState = meTRUE ;
    *n = count ;
    return meTRUE ;
}

#endif

#if MEOPT_UNICODE
/* The char scanner takes the pattern forwards in both directions */
#define scanRevPat(fwd) (fwd)

static meUInt scanPat[meBUF_SIZE_MAX];

static meUInt
scanFold(meUInt uc)
{
    return (uc < 0x80) ? (meUInt) toLower(uc):meUniFold(uc);
}

#define scanCharEq(a,b) (((a) == (b)) || (exactFlag && (scanFold(a) == scanFold(b))))

/* Gets the char at lp, off, \n at a line end, and moves past it; returns 0 at the buffer end */
static int
scanCharNext(meBuffer *bp, meLine **lpp, int *offp, meInt *lnop, meUInt *uc)
{
    meLine *lp=*lpp;
    int off=*offp;
    
    if((off < meLineGetLength(lp)) && (lp->text[off] < 0x80))
    {
        *uc = lp->text[off];
        *offp = off+1;
        return 1;
    }
    if(off >= meLineGetLength(lp))
    {
        if(lp == bp->baseLine)
            return 0;
        *uc = meCHAR_NL;
        *lpp = meLineGetNext(lp);
        *offp = 0;
        (*lnop)++;
        return 1;
    }
    *offp = off + meUtf8Decode(lp->text+off,meLineGetLength(lp)-off,uc);
    return 1;
}

/* Gets the char before lp, off and moves back to it; returns 0 at the buffer start */
static int
scanCharPrev(meBuffer *bp, meLine **lpp, int *offp, meInt *lnop, meUInt *uc)
{
    meLine *lp=*lpp;
    int off=*offp;
    
    if((off > 0) && (lp->text[off-1] < 0x80))
    {
        *uc = lp->text[--off];
        *offp = off;
        return 1;
    }
    if(off == 0)
    {
        if((lp = meLineGetPrev(lp)) == bp->baseLine)
            return 0;
        *uc = meCHAR_NL;
        *lpp = lp;
        *offp = meLineGetLength(lp);
        (*lnop)--;
        return 1;
    }
    off = meUtf8Prev(lp->text,off);
    meUtf8Decode(lp->text+off,meLineGetLength(lp)-off,uc);
    *offp = off;
    return 1;
}

/*
 * scanner -- Search for a pattern in either direction, a char at a time so a match never starts
 * or ends inside a char and case folding works for every char. A stray byte in the pattern is the
 * char of that value. As before, going backwards the cur position is the match end and the match
 * position its start.
 */
static int 
scanner(meUByte *patrn, int direct, int beg_or_end, int *count, SCANNERPOS *sp)
{
    meWindow *cwp=frameCur->windowCur;
    meBuffer *bp=cwp->buffer;
    meLine *curline, *matchline;
    int curoff, matchoff, patLen, nlPat, ii;
    meInt curlnno, matchlnno;
    meUInt cc;
    
    srchLastState = meFALSE;
    for(patLen=0, ii=0 ; patrn[ii] != '\0' ; patLen++)
        ii += meUtf8DecodePat(patrn+ii,meUTF8_LEN_MAX,scanPat+patLen);
    if(patLen == 0)
        return meFALSE;
    nlPat = ((patLen == 1) && (scanPat[0] == meCHAR_NL));
    curline = cwp->dotLine;
    curoff = cwp->dotOffset;
    curlnno = cwp->dotLineNo;
    if(direct == meFORWARD)
    {
        /* only try a match where a byte can start the first char: its first byte, or ignoring
         * case the first byte of any char which folds the same, and after a lead byte the
         * second byte of one. Matching case the first char's last byte is found with memchr */
        meUByte fb[meUTF8_LEN_MAX], lead[32], lead2[32], *text, *pp;
        meUInt pc=scanPat[0], cs[16];
        int len, nn, sb, sl;
        
#define scanLeadSet(cc) (lead[(cc) >> 3] |= (1 << ((cc) & 7)))
#define scanLeadAdd(uc)                                                      \
        do {                                                                 \
            if(meUtf8Encode(fb,(uc)) > 1)                                    \
                lead2[fb[1] >> 3] |= (1 << (fb[1] & 7));                     \
            scanLeadSet(fb[0]);                                              \
        } while(0)
        memset(lead,0,32);
        memset(lead2,0,32);
        if(exactFlag)
        {
            for(nn=meUniFoldSources(scanFold(pc),cs,16) ; --nn >= 0 ; )
            {
                if(cs[nn] < 0x80)
                {
                    scanLeadSet(toLower(cs[nn]));
                    scanLeadSet(toUpper(cs[nn]));
                }
                else
                    scanLeadAdd(cs[nn]);
            }
            sb = -1;
        }
        else
            sb = 0;
        scanLeadAdd(pc);
        if(sb == 0)
        {
            sl = meUtf8Encode(fb,pc) - 1;
            sb = fb[sl];
        }
        for(;;)
        {
            len = meLineGetLength(curline);
            text = curline->text;
            if(sb >= 0)
            {
                while(((curoff+sl) < len) && ((pp = (meUByte *) memchr(text+curoff+sl,sb,len-curoff-sl)) != NULL))
                {
                    curoff = (int) (pp - text) - sl;
                    if(!sl || !memcmp(text+curoff,fb,sl))
                    {
                        matchline = curline;
                        matchoff = curoff;
                        matchlnno = curlnno;
                        for(ii=0 ; ii<patLen ; ii++)
                            if(!scanCharNext(bp,&matchline,&matchoff,&matchlnno,&cc) || !scanCharEq(cc,scanPat[ii]))
                                break;
                        if(ii == patLen)
                            goto scan_found;
                    }
                    curoff++;
                }
                curoff = len;
            }
            for( ; curoff < len ; curoff++)
            {
                cc = text[curoff];
                if((lead[cc >> 3] & (1 << (cc & 7))) &&
                   ((cc < 0xc0) || (lead2[text[curoff+1] >> 3] & (1 << (text[curoff+1] & 7)))))
                {
                    matchline = curline;
                    matchoff = curoff;
                    matchlnno = curlnno;
                    for(ii=0 ; ii<patLen ; ii++)
                        if(!scanCharNext(bp,&matchline,&matchoff,&matchlnno,&cc) || !scanCharEq(cc,scanPat[ii]))
                            break;
                    if(ii == patLen)
                        goto scan_found;
                }
            }
            /* the line end, a \n */
            if((curline == bp->baseLine) || ((*count > 0) && (--(*count) == 0) && !nlPat))
                return meFALSE;
            if(scanPat[0] == meCHAR_NL)
            {
                matchline = curline;
                matchoff = curoff;
                matchlnno = curlnno;
                for(ii=0 ; ii<patLen ; ii++)
                    if(!scanCharNext(bp,&matchline,&matchoff,&matchlnno,&cc) || !scanCharEq(cc,scanPat[ii]))
                        break;
                if(ii == patLen)
                    goto scan_found;
            }
            curline = meLineGetNext(curline);
            curoff = 0;
            curlnno++;
            if(TTbreakTest(1))
            {
                ctrlg(meFALSE,1);
                return meFALSE;
            }
        }
    }
    for(;;)
    {
        matchline = curline;
        matchoff = curoff;
        matchlnno = curlnno;
        {
            if((curoff == 0) &&
               ((meLineGetPrev(curline) == bp->baseLine) || ((*count > 0) && (--(*count) == 0))))
                return meFALSE;
            for(ii=patLen ; ii>0 ; ii--)
                if(!scanCharPrev(bp,&matchline,&matchoff,&matchlnno,&cc) || !scanCharEq(cc,scanPat[ii-1]))
                    break;
            if(ii == 0)
                break;
            scanCharPrev(bp,&curline,&curoff,&curlnno,&cc);
        }
        if(TTbreakTest(1))
        {
            ctrlg(meFALSE,1);
            return meFALSE;
        }
    }
scan_found:
#else
#define scanRevPat(fwd) srchRPat

#define cNotEq(a,b)                                                          \
(((a) != (b)) &&                                                             \
 (!exactFlag || (toggleCase(a) != (b))))

#define cEq(a,b)                                                             \
(((a) == (b)) ||                                                             \
 (exactFlag && (toggleCase(a) == (b))))

/*
 * scanner -- Search for a pattern in either direction.
 */
static int 
scanner(meUByte *patrn, int direct, int beg_or_end, int *count, SCANNERPOS *sp)
{
    meWindow *cwp=frameCur->windowCur;
    register meUByte cc, aa, bb;        /* character at current position */
    register meUByte *patptr;           /* pointer into pattern */
    register meUByte *curptr;           /* pointer into pattern */
    register meLine *curline;           /* current line during scan */
    register int curoff;                /* position within current line */
    register meUByte *matchptr;         /* pointer into pattern */
    register meLine *matchline;         /* current line during matching */
    register int matchoff;              /* position in matching line */
    register meInt curlnno, matchlnno;
    
    srchLastState = meFALSE;
    
    /* Setup local scan pointers to global ".". */
    aa = patrn[0];
    curline = cwp->dotLine;
    curlnno = cwp->dotLineNo;
    if (direct == meFORWARD)
    {
        curptr = curline->text+cwp->dotOffset ;
        for (;;)
        {
            /* Get the character resolving newlines, and
             * test it against first char in pattern.
             */
            if((cc=*curptr++) == '\0')    /* if at EOL */
            {
                /* if on last line or searching a fixed number of lines and this is the last */
                if((curline == cwp->buffer->baseLine) ||
                   ((*count > 0) && (--(*count) == 0) &&
                    /* must check that the pattern isn't "\n" */
                    ((patrn[0] != meCHAR_NL) || (patrn[1] != '\0'))))
                    return meFALSE;  /* We could not find a match */
                
                curline = meLineGetNext(curline) ;  /* skip to next line */
                curlnno++ ;
                curptr = curline->text ;
                cc = meCHAR_NL;
            }
            
            if(cEq(cc,aa)) /* if we find it..*/
            {
                /* Setup match pointers. */
                matchline = curline ;
                matchlnno = curlnno ;
                matchptr = curptr ;
                patptr = &patrn[0];
                
                /* Scan through the pattern for a match. */
                while((bb = *++patptr) != '\0')
                {
                    if((cc=*matchptr++) == '\0')    /* if at EOL */
                    {
                        if(matchline != cwp->buffer->baseLine)
                        {
                            matchline = meLineGetNext(matchline) ; /* skip to next line */
                            matchlnno++ ;
                            matchptr = matchline->text ;
                            cc = meCHAR_NL;          /* and return a <NL> */
                        }
                    }
                    if (cNotEq(cc,bb))
                        break ;
                }
                if(bb == '\0')
                {
                    /* Save the current position in case we need to restore it on a match. */
                    if(curptr == curline->text)
                    {
                        curline = meLineGetPrev(curline);
                        curlnno-- ;
                        curoff = meLineGetLength(curline);
                    }
                    else
                        curoff = (int) (((size_t) curptr) - ((size_t) curline->text)) - 1;
                    matchoff = (int) (((size_t) matchptr) - ((size_t) matchline->text));
                    break;
                }
            }
            if(TTbreakTest(1))
            {
                ctrlg(meFALSE,1);
                return meFALSE;
            }
        }
    }
    else /* Reverse.*/
    {
        curoff = cwp->dotOffset;
        for (;;)
        {
            /* Get the character resolving newlines, and
             * test it against first char in pattern.
             */
            if(curoff == 0)
            {
                if(((curline = meLineGetPrev(curline)) == cwp->buffer->baseLine) ||
                   ((*count > 0) && (--(*count) == 0)))
                    return meFALSE;  /* We could not find a match */
                curlnno--;
                curoff = meLineGetLength(curline);
                cc = meCHAR_NL;
            }
            else
                cc = meLineGetChar(curline, --curoff);
            
            if(cEq(cc,aa)) /* if we find it..*/
            {
                /* Setup match pointers.
                 */
                matchline = curline;
                matchlnno = curlnno;
                matchoff = curoff;
                patptr = &patrn[0];
                
                /* Scan through the pattern for a match.
                 */
                while((bb = *++patptr) != '\0')
                {
                    if (matchoff == 0)
                    {
                        if((matchline = meLineGetPrev(matchline)) == cwp->buffer->baseLine)
                            cc = '\0';
                        else
                        {
                            matchlnno--;
                            matchoff = meLineGetLength(matchline);
                            cc = meCHAR_NL;
                        }
                    }
                    else
                        cc = meLineGetChar(matchline,--matchoff);
                    if(cNotEq(cc,bb))
                        break;
                }
                
                if(bb == '\0')
                {
                    /* Save the current position in case we need to restore it on a match. */
                    if(curoff == meLineGetLength(curline))  /* if at EOL */
                    {
                        curline = meLineGetNext(curline);  /* skip to next line */
                        curlnno++;
                        curoff = 0;
                    }
                    else
                        curoff++;  /* get the char */
                    
                    break ;
                }
                
            }
            if(TTbreakTest(1))
            {
                ctrlg(meFALSE,1);
                return meFALSE;
            }
        }
    }
#endif
                    
    /*
     * A SUCCESSFULL MATCH!!!
     * reset the frameCur->windowCur "." pointers
     * Fill in the scanner position structure if required.
     */
    if(sp != NULL)
    {
        sp->startline = curline;
        sp->startoff = curoff;
        sp->startline_no = curlnno;
        sp->endline = matchline;
        sp->endoffset = matchoff;
        sp->endline_no = matchlnno;
    }
    setShowRegion(cwp->buffer,curlnno,curoff,matchlnno,matchoff);
    /* If we are going in reverse, then the 'end' is actually the beginning of the pattern. Toggle it. */
    if((beg_or_end ^ direct) == PTEND) /* at end of string */
    {
        cwp->dotLine = matchline;
        cwp->dotOffset = matchoff;
        cwp->dotLineNo = matchlnno;
    }
    else /* at beginning of string */
    {
        cwp->dotLine = curline;
        cwp->dotOffset = curoff;
        cwp->dotLineNo = curlnno;
    }
    /* Flag that we have moved and got a region to hilight.*/
    cwp->updateFlags |= WFMOVEL|WFSELHIL;
    srchLastMatch = srchPat;
    srchLastState = meTRUE;
    return meTRUE;
                    
        
}

int
expandchar(int c, meUByte *d, int flags)
{
    register int  doff=1 ;

#if MEOPT_UNICODE
    /* internal UTF-8 & escape bytes are left for the file writer to encode or the display to
     * render */
    if((c & 0x80) || ((flags & meEXPAND_PRINTABLE) ? isPrint(c): isDisplayable(c)))
#else
    if((flags & meEXPAND_PRINTABLE) ? isPrint(c): isDisplayable(c))
#endif
    {
        if((flags & meEXPAND_BACKSLASH) && ((c == '\\') || (c == '"')))
        {
            *d = '\\';
            d[doff++] = c;
        }
        else /* any other character */
            *d = c;
    }
    else if(c < 0x20) /* control character */
    {   /* 0 -> \C@ this may not be okay */
        *d = '\\';
        d[doff++] = 'C' ;
        d[doff++] = c ^ 0x40;
    }
    else /* its a nasty character */
    {
        *d = '\\';
        d[doff++] = 'x';
        d[doff++] = hexdigits[c >> 4] ;
        d[doff++] = hexdigits[c&0x0f] ;
    }
    return doff ;
}

int
expandexp(int slen, meUByte *s, int dlen, int doff, meUByte *d, 
          int cpos, int *opos, int flags)
{
    meUByte cc ;
    int ii ;
    
    if(slen < 0)
        slen = (int) meStrlen(s);
    for(ii=0 ; (doff<dlen-6) && (ii<slen) ; ii++)
    {
        if(cpos == ii)
            *opos = doff ;
        if(((cc = *s++) == meCHAR_LEADER) && (flags & meEXPAND_FFZERO))
        {
            if((cc = *s++) == meCHAR_TRAIL_NULL)
                cc = 0 ;
            else if(cc == meCHAR_TRAIL_SPECIAL)
            {
                d[doff++] = '\\' ;
                d[doff++] = 's' ;
                cc = *s++ ;
                ii++ ;
            }
            else if(cc == meCHAR_TRAIL_HOTKEY)
            {
                d[doff++] = '\\' ;
                cc = 'H' ;
            }
            else if(cc == meCHAR_TRAIL_HILSTART)
            {
                d[doff++] = '\\' ;
                cc = '{' ;
            }
            else if(cc == meCHAR_TRAIL_HILSTOP)
            {
                d[doff++] = '\\' ;
                cc = '}' ;
            }
            ii++ ;
        }
        doff += expandchar( ((int) cc) & 0xff, d+doff, flags) ;
    }
    d[doff] = '\0' ;
    return doff ;
}

/*
 * rvstrcpy -- Reverse string copy.
 * 
 * The source buffer may vary, but the result is always to srchRPat
 */
static void
rvstrcpy(register meUByte *str)
{
    register meUByte *rvstr ;
    register int ii ;
    
    if((ii = (int) meStrlen(str)) > meBUF_SIZE_MAX-1)
        ii = meBUF_SIZE_MAX-1;
    
    rvstr = srchRPat+ii ;
    *rvstr = '\0' ;
    while (--ii >= 0)
        *--rvstr = *str++ ;
}

/*
 * readpattern -- Read a pattern.  Stash it in srchPat.  If it is the
 *	create the reverse pattern and the magic
 *	pattern, assuming we are in MAGIC mode (and defined that way).
 *	Apat is not updated if the user types in an empty line.  If
 *	the user typed an empty line, and there is no old pattern, it is
 *	an error.  Display the old pattern, in the style of Jeff Lomicka.
 *	There is some do-it-yourself control expansion.  Change to using
 *	<META> to delimit the end-of-pattern to allow <NL>s in the search
 *	string. 
 */
static int
readpattern(meUByte *prompt, int defnum)
{
    int	status;

    /* Read a pattern.  Either we get one,
     * or we just get the META charater, and use the previous pattern.
     * Then, if it's the search string, make a reversed pattern.
     * *Then*, make the meta-pattern, if we are defined that way.
     */
    if((status = meGetString(prompt,MLSEARCH,defnum,srchPat,meBUF_SIZE_MAX)) > 0)
    {
        exactFlag = (meModeTest(frameCur->windowCur->buffer->mode,MDEXACT) == 0);
#if MEOPT_MAGIC
        /* Only make the meta-pattern if in magic mode,
         * since the pattern in question might have an
         * invalid meta combination.
         */
        if(meModeTest(frameCur->windowCur->buffer->mode,MDMAGIC))
        {
            srchLastMagic = meSEARCH_LAST_MAGIC_MAIN ;
            status = (mere_compile_pattern(srchPat) == meREGEX_OKAY) ;
        }
        else
        {
            srchLastMagic = 0 ;
            rvstrcpy(srchPat);
        }
#else
        rvstrcpy(srchPat);
#endif
    }
    return status ;
}
/*
 * replaces -- Search for a string and replace it with another
 *	string.  Query might be enabled (according to kind).
 */
static int
replaces(int kind, int ff, int nn)
/* kind - Query enabled flag */
{
    /*---	Local defintions for the state machine */

#define	SL_EXIT		0	/* Exit replaces */
#define	SL_GETSEARCH	1	/* Get the search string */
#define	SL_GETREPLACE	2	/* Get the replace string */
#define	SL_EDITREPLACE	3	/* Edit replace with string */
#define	SL_FIRSTREPLACE	4	/* First replacement condition */
#define	SL_NEXTREPLACE	5	/* Next replacement condition */
#define	SL_QUERYINPUT	6	/* Get the query input from the user */
#define	SL_SUBSTITUTE	7	/* Substitute string */

/*---	Local variable defintions */
	
    static meUByte *dpat = NULL;
    static int dpatlen = 0;

    register int i;		/* loop index */
    register int slength;	/* Length of search string */
    register int rlength;	/* length of replace string */
    meUByte      rpat[meBUF_SIZE_MAX];	/* replacement pattern		*/
    int	         numsub;	/* number of substitutions */
    int	         nummatch;	/* number of found matches */
    int	         status;	/* success flag on pattern inputs */
    int	         nlflag;	/* last char of search string a <NL>? */
    int	         nlrepl;	/* was a replace done on the last line? */
    meLine	*lastline=0;	/* position of last replace and */
    int	         lastoff;       /* offset (for 'u' query option) */
    meInt        lastlno;       /* line no (for 'u' query option) */
    int	         onemore=meFALSE;  /* only do one more replace */
    int	         ilength=0;	/* Last insert length */
    int	         state_mc;	/* State machine */
    int 	 cc;		/* input char for query */
    meUByte	 tmpc;		/* temporary character */
    meUByte	 tpat[meBUF_SIZE_MAX];	/* temporary to hold search pattern */
    meWindow    *cwp;
    
    /* Determine if we are  in the correct viewing  mode, and if the number
       of repetitions is correct. */

    if(ff == 0)
        /* as many can over rest of buffer */
        nn = -1;
    else if (nn < 0)
    {
        /* Check for negative repetitions. */
        ff = -nn;           /* Force line counting */
        nn = -1;            /* Force to be 0 - do as many as can */
    }
    else
        ff = 0;
    
    /*--- Execute a state machine  to get around  the replace mechanism.  This
      is  not  super  efficient  in  terms  of  speed,  but  is  good  for
      versitility.  Hence we should be able to add what we like at a later
      date.  */
    cwp = frameCur->windowCur;
    for(state_mc = SL_GETSEARCH; state_mc != SL_EXIT; /* NULL */)
    {
        switch(state_mc)
        {
            /*
             * Ask the user for the text of a pattern.
             */
        case SL_GETSEARCH:
            
            if ((status = readpattern
                 ((kind == meFALSE ? (meUByte *)"Replace":(meUByte *)"Query replace"), 
                  1+lastReplace)) <= 0)
		return status;		/* Aborted out - Exit */
            
            slength = (int) meStrlen(srchPat);	/* Get length of search string */
            
/*            mlwrite (MWWAIT,"Search [%d:%s]", slength, srchPat);*/
            
            /* Set up flags so we can make sure not to do a recursive
             * replace on the last line. */
            
            nlflag = (srchPat[slength - 1] == meCHAR_NL);
            nlrepl = meFALSE;
            state_mc = SL_GETREPLACE;	/* Move to next state */
            break;
            
        case SL_GETREPLACE:	/* Ask for replacement string */
        case SL_EDITREPLACE:	/* Edit replacement string */
            
            meStrcpy(tpat,"Replace [");
            i = expandexp(-1,srchPat, meBUF_SIZE_MAX-17, 9, tpat, -1, NULL, 0);
            meStrcpy(tpat+i,"] with");
            
            if ((status=meGetString(tpat,MLSEARCH,0,rpat,meBUF_SIZE_MAX)) <= 0)
            {
                lastReplace = 0;
                return status;
            }	/* End of 'if' */
            
            rlength = (int) meStrlen(rpat);	/* Get length of replace string. */
            
            if (kind)	  	/* Build query replace question string.  */
            {
                tpat[i+6] = ' ';
                tpat[i+7] = '[';
                i = expandexp(-1,rpat, meBUF_SIZE_MAX-i-13, i+8, tpat, -1, NULL, 0);
                meStrcpy(tpat+i,((i+30) < frameCur->width) ? "] (?/y/n/a/e/l/u) ? ":"] ? ");
            }            
            state_mc = (state_mc == SL_GETREPLACE) ? SL_FIRSTREPLACE:SL_NEXTREPLACE;
            lastReplace = 1;
            break;
            
        case SL_FIRSTREPLACE:		/* Do the first replacement */
            
            numsub = 0;
            nummatch = 0;
            
            /* Initialize last replaced pointers. */
            lastline = NULL;
            
            state_mc = SL_NEXTREPLACE;		/* Do the next replacement */
            break;	
            
        case  SL_NEXTREPLACE:		/* Next replcaement condition */
            
            if (!((nn < 0 || nn > nummatch) &&
                  ((nlflag == meFALSE) || (nlrepl == meFALSE)) && (onemore == meFALSE)))
            {
                /*---	Exit condition has been met */
                
		state_mc = SL_EXIT;		/* State machine to exit */
		break;				/* Exit case */
            }	/* End of 'if' */
            
            /*--- Search  for  the  pattern.   If  we  search  with  a regular
              expression, also save the true length of matched string.  */
            
#if MEOPT_MAGIC
            if(srchLastMagic)
            {
                if (!mere_scanner(meFORWARD, PTBEG, &ff, NULL))
                {
                    state_mc = SL_EXIT;
                    break;
		}
            }
            else
#endif
                if (!scanner(srchPat, meFORWARD, PTBEG, &ff, NULL))
            {
                state_mc = SL_EXIT;
                break;		/* all done */
            }
            
            ++nummatch;	/* Increment # of matches */
            
            /* Check if we are on the last line. */
            
            nlrepl = (meLineGetNext(cwp->dotLine) == cwp->buffer->baseLine);
            
            if (kind)
		state_mc = SL_QUERYINPUT;
            else
		state_mc = SL_SUBSTITUTE;
            break;
            
        case SL_QUERYINPUT:			/* Get input from user. */
            
#if MEOPT_UNDO
            undoContFlag++ ;
#endif
            /* SWP 2003-05-10 - Found nasty problem using toolbar with query-replace-all-strings,
             * the command can lead to changing buffer, the prompt can lead to the idle macro
             * being executed with in this case was the toolbar, and as the buffer had changed
             * it refreshed the tool buffers which use the search command, this changes the
             * regex info as this is stored in a global. This leads to a crash!
             * As a fix cash the info now and restore after mlCharReply returns, as this is
             * a query call this is not likely to be a macro so not really a performance issue,
             *  just make it work!
             */
            {
                meUByte srchPatStore[meBUF_SIZE_MAX];	/* replacement pattern		*/
#if MEOPT_MAGIC
                meRegexGroup groupStore[10];
                int ii, srchLastMagicStore;
                
                if((srchLastMagicStore = srchLastMagic) != 0)
                {
                    ii = mereRegex.groupNo + 1;
                    if(ii > 10)
                        ii = 10;
                    memcpy(groupStore,mereRegex.group,ii*sizeof(meRegexGroup));
                }
#endif
                meStrcpy(srchPatStore,srchPat);
                cc = mlCharReply(tpat,mlCR_LOWER_CASE|mlCR_UPDATE_ON_USER|mlCR_CURSOR_IN_MAIN,(meUByte *)"y na!elu",
                                 (meUByte *)"(Y)es, (N)o, Yes to (a)ll, (E)dit replace, (L)ast, (U)ndo, (C-g)Abort ? ");
                meStrcpy(srchPat,srchPatStore);
#if MEOPT_MAGIC
                if((srchLastMagic = srchLastMagicStore) != 0)
                {
                    mere_compile_pattern(srchPat);
                    memcpy(mereRegex.group,groupStore,ii*sizeof(meRegexGroup));
                }
#endif
            }            
            switch(cc)			/* And respond appropriately. */
            {
            case -1:
                return ctrlg(meFALSE,1);
                
            case 'y':			/* yes, substitute */
            case ' ':
		state_mc = SL_SUBSTITUTE;
		break;
                
            case 'n':			/* no, onword */
		meWindowForwardChar(cwp, 1);
		state_mc = SL_NEXTREPLACE;
		break;
                
            case 'e':			/* Edit replace with string */
		state_mc = SL_EDITREPLACE;
		break;
							
            case 'l':			/* last one */
		onemore = meTRUE;
		state_mc = SL_SUBSTITUTE;
		break;
                
            case 'a':			/* yes/stop asking */
            case '!':
		kind = meFALSE;
		state_mc = SL_SUBSTITUTE;
		break;

            case 'u':	/* undo last and re-prompt */
                if(lastline == NULL)
                {   /* Restore old position. */
                    TTbell();		/* There is nothing to undo. */
                    break;
		}
                
                /*---	Reset the last position */
                
                cwp->dotLine  = lastline ; 
                cwp->dotOffset  = lastoff ;
                cwp->dotLineNo = lastlno ;
                cwp->updateFlags |= WFMOVEL ;
		lastline = NULL;
                
		/* Delete the new string. */
                
		meWindowBackwardBytes(cwp, ilength);
                if (!ldelete(ilength,6))
                    return mlwrite(MWABORT,(meUByte *)"[ERROR while deleting]");
                
		/* And put in the old one. */
                i = bufferInsertText(dpat,0) ;
#if MEOPT_UNDO
                meUndoAddReplaceEnd(i) ;
#endif
		
                /* Record one less substitution, backup, and reprompt. */
		--numsub;
                lastoff = cwp->dotOffset ;
                lastlno = cwp->dotLineNo ;
                meWindowBackwardBytes(cwp, slength);
                /* must research for this instance to setup the auto-repeat sections
                 * correctly */
                --nummatch;	/* decrement # of matches */
                state_mc = SL_NEXTREPLACE ;
		break;
            }	/* end of switch */
            break;
            
            /*
             * Delete the sucker.
             */
        case SL_SUBSTITUTE:			/* Substitute the string */
            
            if(bufferSetEdit() <= 0)               /* Check we can change the buffer */
                return meABORT ;
#if MEOPT_EXTENDED
            /* check that this replace is legal */
#if MEOPT_MAGIC
            if(srchLastMagic)
                i = mereRegexGroupEnd(0) ;
            else
#endif
                i = slength + cwp->dotOffset ;
            i += cwp->dotOffset;
            {
                meLine *lp = cwp->dotLine;
                if(i > meLineGetLength(lp))
                {
                    for(;;)
                    {
                        if(meLineGetFlag(lp) & meLINE_PROTECT)
                            return mlwrite(MWABORT,(meUByte *)"[Protected line in matched string!]") ;
#if MEOPT_NARROW
                        if((lp != cwp->dotLine) &&
                           (meLineGetFlag(lp) & meLINE_ANCHOR_NARROW))
                            return mlwrite(MWABORT,(meUByte *)"[Narrow in matched string!]") ;
#endif
                        i -= meLineGetLength(lp) + 1 ;
                        if(i < 0)
                            break;
                        lp = meLineGetNext(lp) ;
                    }
                }
                if(lp == cwp->buffer->baseLine)
                    /* current match includes the buffer's last line, this must be the last replace or ME could spin (e.g. rep '\n' -> 'a\n') */
                    onemore = meABORT;
            }
#endif
#if MEOPT_MAGIC
            /* only set the new regex slength here as if the user is doing a query replace
             * and does an undo, the slength must be correct for the last insertion */
            if(srchLastMagic)
                slength = mereRegexGroupEnd(0) ;
#endif
            if(dpatlen <= slength)
            {
                meNullFree(dpat) ;
		dpatlen = slength+1;
		if ((dpat = meMalloc (dpatlen)) == NULL)
                {
                    dpatlen = 0 ;
                    return meFALSE ;
                }
            }
            if(mldelete(slength,dpat) != 0)
                return mlwrite(MWABORT,(meUByte *)"[ERROR while deleting]") ;
            ilength = 0 ;
            
            /*
             * And insert its replacement.
             */
            
            for (i = 0 ; i < rlength; i++)
            {
                tmpc = rpat[i] ;
#if MEOPT_MAGIC
                if (srchLastMagic && 	               /* Magic string ?? */
                    (tmpc == '\\') && 
                    (i+1 < rlength))                   /* Not too long ?? */
                {
                    int jj, kk, ll, rng=0, changeCase=0 ;
                    
                    tmpc = rpat[++i];
                    if((i+7 < rlength) && (tmpc == '{'))
                    {
                        rng=1;
                        tmpc = rpat[++i];
                    }
                    if((i+1 < rlength) && ((tmpc == 'l') || (tmpc == 'u') || (tmpc == 'c')))
                    {
                        changeCase = (tmpc == 'l') ? -1:tmpc;
                        tmpc = rpat[++i];
                    }
                    if(tmpc == '*')
                    {
                        if(i != 1)
                            status = mlwrite(MWABORT,(meUByte *)"[ERROR: Replace \\*<macro> exec must be first]");
                        else if(lineExec(0,1,rpat+2) <= 0)
                            status = mlwrite(MWABORT,(meUByte *)"[ERROR: Replace \\*<macro> returned error]");
                        else if((i = bufferInsertText(resultStr,0)) < 0)
                            status = i;
                        else
                        {
                            ilength += i-1;
                            i = rlength;
                        }
                    }
                    else if((tmpc == '&') || ((tmpc >= '0') && ((int)(tmpc-'0' <= mereRegexGroupNo()))))
                    {
                        meUByte lchr, rchr, cc;
                        /* Found auto repeat char replacement */
                        tmpc = (tmpc == '&') ? 0:tmpc-'0' ;
                        
                        /* if start offset is < 0, it was a ? auto repeat which was not found, therefore replace str == "" */ 
                        jj = mereRegexGroupStart(tmpc);
                        kk = mereRegexGroupEnd(tmpc);                        
                        if(rng)
                        {
                            tmpc = rpat[++i];
                            if((lchr=rpat[++i]) == tmpc)
                                lchr = '\0';
                            else if(rpat[++i] != tmpc)
                                rng = -1;
                            if(rng < 0)
                                ;
                            else if((rchr=rpat[++i]) == tmpc)
                                rchr = '\0';
                            else if(rpat[++i] != tmpc)
                                rng = -1;
                            if(rng > 0)
                            {
                                rng = 0;
                                while(((cc=rpat[++i]) >= '0') && (cc <= '9'))
                                    rng = (rng * 10) + cc - '0';
                            }
                            if((rng <= 0) || (cc != '\\') || (rpat[++i] != '}') || ((lchr == '\0') && (rchr == '\0')))
                            {
                                mlwrite(MWABORT,(meUByte *)"[ERROR: Invalid replace \\{...\\} interval]");
                                status = -1;
                                rng = -1;
                            }
                            else if(jj >= 0)
#if MEOPT_UNICODE
                                /* pad to a width in cells */
                                rng -= meStrCells(dpat+jj,kk-jj);
#else
                                rng = rng + jj - kk;
#endif
                            if((rng > 0) && (lchr != '\0') && ((ll = (rchr == '\0') ? rng:(rng>>1)) > 0))
                            {
                                rng -= ll;
                                if(lchr == meCHAR_NL)
                                {
                                    do {
                                        if((status = lineInsertNewline(0)) <= 0)
                                            break;
                                        ilength++;
                                    } while(--ll);
                                }
                                else if((status = lineInsertChar(ll,lchr)) > 0)
                                    ilength += ll;
                            }
                        }
                            
#if MEOPT_UNICODE
                        /* copy the group a char at a time, a case change can change a char's length */
                        while((jj >= 0) && (jj < kk) && (status > 0))
                        {
                            meUByte cb[meUTF8_LEN_MAX];
                            meUInt uc;
                            int cl;
                            jj += meUtf8Decode(dpat+jj,kk-jj,&uc);
                            if(changeCase)
                            {
                                if(changeCase > 0)
                                {
                                    uc = meUniCharToUpper(uc);
                                    if(changeCase == 'c')
                                        changeCase = -1;
                                }
                                else
                                    uc = meUniCharToLower(uc);
                            }
                            if(uc == meCHAR_NL)
                            {
                                status = lineInsertNewline(0);
                                ilength++;
                            }
                            else
                            {
                                cl = meUtf8Encode(cb,uc);
                                status = lineInsertString(cl,cb);
                                ilength += cl;
                            }
                        }
#else
                        if(jj >= 0)
                        {
                            for( ; ((jj < kk) && (status > 0)) ; ilength++)
                            {
                                tmpc = dpat[jj++] ;
                                if(changeCase)
                                {
                                    if(changeCase > 0)
                                    {
                                        tmpc = toUpper(tmpc) ;
                                        if(changeCase == 'c')
                                            changeCase = -1 ;
                                    }
                                    else
                                        tmpc = toLower(tmpc) ;
                                }
                                if(tmpc == meCHAR_NL)
                                    status = lineInsertNewline(0);
                                else
                                    status = lineInsertChar(1, tmpc);
                            }
                        }
#endif
                        if((rng > 0) && (status > 0))
                        {
                            if(rchr == meCHAR_NL)
                            {
                                do {
                                    if((status = lineInsertNewline(0)) <= 0)
                                        break;
                                    ilength++;
                                } while(--rng);
                            }
                            else if((status = lineInsertChar(rng,rchr)) > 0)
                                ilength += rng;
                        }
                        /* subtract one from insert length as one is added at
                         * the end of the for loop */
                        if(status > 0)
                            ilength--;
                    }
                    else
                    {
                        if(tmpc == 'n')
                            tmpc = '\n';
                        else if(tmpc == 'r')
                            tmpc = '\r';
                        else if(tmpc == 't')
                            tmpc = '\t';
                        if(tmpc != meCHAR_NL)
                            status = lineInsertChar(1, tmpc);
                        else
                            status = lineInsertNewline(0);
                    }
                }
                else
#endif
                    if (tmpc == meCHAR_NL)
                    status = lineInsertNewline(0);
                else
                    status = lineInsertChar(1, tmpc);
                /* Insertion error ? */
                if(status <= 0) 
                    break ;
                /* insertion succeeded increment length */
                ilength++;
            }	/* End of 'for' */
#if MEOPT_UNDO
            meUndoAddReplace(dpat,ilength) ;
#endif
            if (status <= 0) 
                return meABORT ;
            
            if (kind) 
            {			/* Save position for undo. */
		lastline = cwp->dotLine;
		lastoff = cwp->dotOffset; 
		lastlno = cwp->dotLineNo; 
            }
            numsub++;			/* increment # of substitutions */
            state_mc = SL_NEXTREPLACE;	/* Do next replacement */
            if (slength == 0)
            {
                meWindowForwardChar(cwp, 1);
                if ((ff > 0) && (cwp->dotOffset == 0) && (--ff == 0))
                    state_mc = SL_EXIT;
            }
            break;
            
        default:
            return mlwrite(MWABORT,(meUByte *)"[Bad state machine value %d]", state_mc);
        }	/* End of 'switch' on state_mc */
    }   /* End of 'for' */
    
    /*--- And report the results. crop rpat to ensure resultStr is not overrun */
    rlength = (int) meStrlen(rpat);
    if(rlength > meBUF_SIZE_MAX-16)
        rlength = meBUF_SIZE_MAX-16;
    cc = 0xff;
    while((memchr(rpat,cc,rlength) != NULL) && (cc > 1))
        cc--;
    slength = sprintf((char *)resultStr,"%c%d%c%c%c",cc,numsub,cc,((!kind) ? 'a':((onemore == meTRUE) ? 'l':'q')),cc);
    memcpy(resultStr+slength,rpat,rlength);
    resultStr[slength+rlength] = cc;
    resultStr[slength+rlength+1] = '\0';

    return mlwrite(((nn < 0) || (numsub == nn)) ? MWCLEXEC:(MWABORT|MWCLEXEC),(meUByte *)"%d substitutions", numsub);
}	/* End of 'replaces' */

/*
 * replaceStr -- Search and replace.
 */
int
replaceStr(int f, int n)
{
    return replaces(meFALSE, f, n) ;
}

/*
 * queryReplaceStr -- search and replace with query.
 */
int	
queryReplaceStr(int f, int n)
{
    return replaces(meTRUE, f, n) ;
}

/*
 * searchForw -- Search forward.  Get a search string from the user, and
 *	search, beginning at ".", for the string.  If found, reset the "."
 *	to be just after the match string, and (perhaps) repaint the display.
 */

int
searchForw(int f, int n)
{
    register int status;
    int          repCount;              /* Repeat count */

    /* Resolve the repeat count. */
    if (n <= 0) {
        n = -n;
        repCount = 1;
    }
    else {
        repCount = n;
        n = 0;
    }
    
    /* Ask the user for the text of a pattern.  If the
     * response is meTRUE (responses other than meFALSE are
     * possible), search for the pattern.
     */
    if ((status = readpattern((meUByte *)"Search", 1+lastReplace)) > 0)
    {
        lastReplace = 0 ;
        do
        {
#if MEOPT_MAGIC
            if(srchLastMagic)
                status = mere_scanner(meFORWARD, PTEND, &n, NULL);
            else
#endif
                status = scanner(srchPat, meFORWARD, PTEND, &n, NULL);
        } while ((--repCount > 0) && status);
        
        /* ...and complain if not there.
         */
        if(status == meFALSE)
            notFound();
    }
    return(status);
}

/*
 * searchBack -- Reverse search.  Get a search string from the user, and
 *	search, starting at "." and proceeding toward the front of the buffer.
 *	If found "." is left pointing at the first character of the pattern
 *	(the last character that was matched).
 */
int
searchBack(int f, int n)
{
    register int status;
    int          repCount;              /* Repeat count */
    
    /* Resolve the repeat count. */
    if (n <= 0) {
        n = -n;
        repCount = 1;
    }
    else {
        repCount = n;
        n = 0;
    }
    
    /* Ask the user for the text of a pattern.  If the
     * response is meTRUE (responses other than meFALSE are
     * possible), search for the pattern.
     */
    if ((status = readpattern((meUByte *)"Reverse search", 1+lastReplace)) > 0)
    {
        lastReplace = 0 ;
        do
        {
#if MEOPT_MAGIC
            if(srchLastMagic)
                status = mere_scanner(meREVERSE, PTBEG, &n, NULL);
            else
#endif
                status = scanner(scanRevPat(srchPat), meREVERSE, PTBEG, &n, NULL);
        } while ((--repCount > 0) && status);
        
        /* ...and complain if not there.
         */
        if(status == meFALSE)
            notFound();
    }
    return(status);
}

/* NOTE: The hunt commands can only assume that the srchPat is correctly
 * set to the last user search. system uses of iscanner (such as tags, next,
 * timestamp etc) mean that the srchRPat and meRegex may have been trashed.
 */
/*
 * huntForw -- Search forward for a previously acquired search string,
 *	beginning at ".".  If found, reset the "." to be just after
 *	the match string, and (perhaps) repaint the display.
 */

int
huntForw(int f, int n)
{
    register int status;
    int          repCount;              /* Repeat count */

    /* Resolve the repeat count. */
    if (n <= 0) {
        n = -n;
        repCount = 1;
    }
    else {
        repCount = n;
        n = 0;
    }
    
    /* Make sure a pattern exists, or that we didn't switch
     * into MAGIC mode until after we entered the pattern.
     */
    if(srchPat[0] == '\0')
        return mlwrite(MWABORT,(meUByte *)"[No pattern set]");
    
#if MEOPT_MAGIC
    /* if magic the recompile the search string 
     * SWP 2003-10-06 - check if the last main search was magic, use
     * of &xse will set the STRCMP bit but srcPat remains set to the
     * last main search which could be non- regex
     */
    srchLastMagic &= meSEARCH_LAST_MAGIC_MAIN ;
    if(srchLastMagic)
    {
        if(mere_compile_pattern(srchPat) != meREGEX_OKAY)
            return meABORT ;
    }
#endif
    
    /* Search for the pattern... */
    do
    {
#if MEOPT_MAGIC
        if(srchLastMagic)
            status = mere_scanner(meFORWARD, PTEND, &n, NULL);
        else
#endif
            status = scanner(srchPat, meFORWARD, PTEND, &n, NULL);
    } while ((--repCount > 0) && status);
    
    /* ...and complain if not there. */
    if(status == meFALSE)
        notFound() ;
    
    return(status);
}

/*
 * huntBack -- Reverse search for a previously acquired search string,
 *	starting at "." and proceeding toward the front of the buffer.
 *	If found "." is left pointing at the first character of the pattern
 *	(the last character that was matched).
 */
int
huntBack(int f, int n)
{
    register int status;
    int          repCount;              /* Repeat count */
    
    /* Resolve null and negative arguments. */
    if (n <= 0) {
        n = -n;
        repCount = 1;
    }
    else {
        repCount = n;
        n = 0;
    }
    
    /* Make sure a pattern exists, or that we didn't switch
     * into MAGIC mode until after we entered the pattern.
     */
    if(srchPat[0] == '\0')
        return mlwrite(MWABORT,(meUByte *)"[No pattern set]");
    
#if MEOPT_MAGIC
    /* if last was a magic search then recompile the search string 
     * SWP 2003-10-06 - check if the last main search was magic, use
     * of &xse will set the STRCMP bit but srcPat remains set to the
     * last main search which could be non- regex
     */
    srchLastMagic &= meSEARCH_LAST_MAGIC_MAIN ;
    if(srchLastMagic)
    {
        if(mere_compile_pattern(srchPat) != meREGEX_OKAY)
            return meABORT ;
    }
    else
#endif
        rvstrcpy(srchPat);
    
    /* Go search for it... */
    do
    {
#if MEOPT_MAGIC
        if(srchLastMagic)
            status = mere_scanner(meREVERSE, PTBEG, &n, NULL);
        else
#endif
            status = scanner (scanRevPat(srchPat), meREVERSE, PTBEG, &n, NULL);
    } while ((--repCount > 0) && status);
    
    /* ...and complain if not there. */
    if(status == meFALSE)
        notFound();
    return(status);
}


int
iscanner(meUByte *apat, int n, int flags, SCANNERPOS *sp)
{
    int direct, magic;
    int beg_or_end;
    
    direct = (flags & ISCANNER_BACKW) ? meREVERSE:meFORWARD ;
    beg_or_end = (flags & ISCANNER_PTBEG) ? PTBEG : PTEND;
    exactFlag = ((flags & ISCANNER_EXACT) == 0);

#if MEOPT_MAGIC
    magic = (flags & ISCANNER_MAGIC) ;
    /* if this is the users search buffer then set the LastMagic flag */
    if(apat == srchPat)
        srchLastMagic = magic ;
    
    /*
     * Only make the meta-pattern if in magic mode, since the pattern in
     * question might have an invalid meta combination.
     */
    if(magic)
    {
        int status ;
        
        if (flags & ISCANNER_QUIET)
            reportErrors = meFALSE;
        status = mere_compile_pattern(apat);
        if (flags & ISCANNER_QUIET)
            reportErrors = meTRUE;        
        /*
         * Bail out here if we have a false status, we cannot construct a valid
         * search string yet it is not compleate.
         */
        if (status != meREGEX_OKAY)
            return ((status < meREGEX_ERROR_CLASS) ? -2:-1) ;
        
        return mere_scanner(direct,beg_or_end,&n,sp);
    }
#endif
    
    if(direct == meREVERSE)
        /* Reverse string copy */
        rvstrcpy(apat);
    
    return scanner (((direct == meFORWARD) ? apat : scanRevPat(apat)),direct, beg_or_end, &n, sp);
}

int
searchBuffer(int f, int n)
{
    meUByte flagsStr[20] ;
    int flags, rr ;
    
    if(((rr=meGetString((meUByte *)"Search Flags",0,0,flagsStr,20)) > 0) &&
       ((rr=meGetString((meUByte *)"Search String",MLSEARCH,1+lastReplace,srchPat,meBUF_SIZE_MAX)) > 0))
    {
        flags = 0 ;
        if(meStrchr(flagsStr,'b'))
            flags |= ISCANNER_BACKW|ISCANNER_PTBEG ;
        if(meStrchr(flagsStr,'e') ||
           ((meStrchr(flagsStr,'E') == NULL) &&
            meModeTest(frameCur->windowCur->buffer->mode,MDEXACT)))
            flags |= ISCANNER_EXACT ;
#if MEOPT_MAGIC
        if(meStrchr(flagsStr,'m') ||
           ((meStrchr(flagsStr,'M') == NULL) &&
            meModeTest(frameCur->windowCur->buffer->mode,MDMAGIC)))
            flags |= ISCANNER_MAGIC ;
#endif
        if(n == 0)
        {
            while(iscanner(srchPat,0,flags,NULL) > 0)
                n++;
            sprintf((char *)resultStr,"%d",n);
            return meTRUE;
        }
        else if(n <= 0)
        {
            f = -n;
            n = 1;
        }
        else
            f = 0;
        do
            rr = iscanner(srchPat,f,flags,NULL);
        while ((rr > 0) && (--n > 0));
    }
    return rr;
}

#ifdef _ME_FREE_ALL_MEMORY
void srchFreeMemory(void)
{
#if MEOPT_MAGIC
    void meRegexFree(meRegex *regex) ;

    meRegexFree(&mereRegex) ;
    meRegexFree(&meRegexStrCmp) ;
    meRegexCacheFree() ;
    meNullFree(mereNewlBuf) ;
#endif
}
#endif

