/*****************************************************************************
 *
 *	Title:		search.c
 *
 *	Synopsis:	Search routines.
 *
 ******************************************************************************
 *
 *	Filename:		search.c
 *
 *	Author:			Unknown
 *
 *	Creation Date:		07/05/85 08:19		
 *
 *	Modification date:	<011031.1645>
 *
 *	Current rev:		10.1
 *
 *	Special Comments:	
 *
 *	Contents Description:	
 *
 * The functions in this file implement commands that search in the forward
 * and backward directions.  There are no special characters in the search
 * strings.  Probably should have a regular expression search, or something
 * like that.
 *
 * Aug. 1986 John M. Gamble:
 *	Made forward and reverse search use the same scan routine.
 *
 *	Added a limited number of regular expressions - 'any',
 *	'character class', 'closure', 'beginning of line', and
 *	'end of line'.
 *
 *	Replacement metacharacters will have to wait for a re-write of
 *	the replaces function, and a new variation of ldelete().
 *
 *	For those curious as to my references, i made use of
 *	Kernighan & Plauger's "Software Tools."
 *	I deliberately did not look at any published grep or editor
 *	source (aside from this one) for inspiration.  I did make use of
 *	Allen Hollub's bitmap routines as published in Doctor Dobb's Journal,
 *	June, 1985 and modified them for the limited needs of character class
 *	matching.  Any inefficiences, bugs, stupid coding examples, etc.,
 *	are therefore my own responsibility.
 *
 * July 90. Jon Green (No references cos couldn't find fore-mentioned book !)
 *
 *	Corrected bug on 
 *	an 'undo' of a search replace function. Now insert the correct 
 *	text back into the source file, not the MAGIC pattern !!!. Required
 *	new ldelete() function, called mldelete. (see line.c).
 *
 *	Fixed bug on magic wild card '.' when used in conjunction with '*'.
 *	ie '.*'. The code always used to go to the LAST occurence of the 
 *	pattern on a foward search. This is not what is required. We want to go
 *	to the first occurence of the pattern. Tricky one that, have to
 *	look for the next literal in the MC string which we can search on.
 *	This is what we use to delimit on.
 *
 *	Added the repeat pattern, that is allow text to be imported from
 *	the search function into the replace function. ie consider the
 *	following :-
 *
 *	Old Text buffer : a += 1;
 *	                  a <<= 3;
 *
 *	Require :         a = a + 1;
 *	                  a = a << 3;
 *
 *	Can now do operation with search and replace as :=
 *
 *	Search : ^{.*} {.*}=
 *	Replace: $0 = $0 $1
 *
 *	Easy !! It always pissed me off that I couldn't do that. Wonder
 *	how many people will use it ?
 *
 * July 91	Jon Green
 *
 *	Added incrental replace facility on search and replace. Allows the same
 *	search pattern while varying the replace pattern.
 * 
 * June 99      Steven Phillips
 * 
 *  Moved over to GNU regex at last.
 *
 * August 99    Steven Phillips
 * 
 *  Had problems with GPL license, implemented new GNU compliant regex and
 *  moved over to this.
 *
 *****************************************************************************
 * 
 * Modifications to the original file by Jasspa. 
 *
 * Copyright (C) 1988 - 1999, JASSPA 
 *
 * The MicroEmacs Jasspa distribution can be copied and distributed freely for
 * any non-commercial purposes. The MicroEmacs Jasspa Distribution can only be
 * incorporated into commercial software with the expressed permission of
 * JASSPA.
 *  
 ****************************************************************************/

/*---	Include defintions */

#define	__SEARCHC			/* Define filename */

/*---	Include files */

#include "emain.h"
#include "esearch.h"

/*---	Local macro definitions */

#define notFound() (mlwrite(MWABORT|MWCLEXEC,notFoundStr))

/*---	External references */

/*---	Local type definitions */

/*---	Local variable definitions */

static uint8 notFoundStr[] ="[Not Found]";
static int  exactFlag = 1;              /* Use to cache EXACT mode state */
uint8  srchPat [NPAT]="";                /* Search pattern array */
uint8  srchRPat[NPAT]="";                /* Reversed pattern array */
int    srchLastState=FALSE;              /* status of last search */
uint8 *srchLastMatch=NULL;               /* pointer to the last match string */

/*
 * boundry -- Return information depending on whether we may search no
 *	further.  Beginning of file and end of file are the obvious
 *	cases, but we may want to add further optional boundry restrictions
 *	in future, a la VMS EDT.  At the moment, just return TRUE or
 *	FALSE depending on if a boundry is hit (ouch).
 */
#define boundry(curline,curoff,dir)                                          \
(((dir) == FORWARD) ?                                                        \
 (curline == curbp->b_linep) :                                               \
 ((curoff == 0) && (lback(curline) == curbp->b_linep)))


#if	MAGIC

int           srchLastMagic=FALSE;              /* last search was a magic        */
static int    reportErrors = TRUE;

static int    mereNewlBufSz=0 ;
static uint8 *mereNewlBuf=NULL ;

#define mereNewlBufSzCheck(ll)                                               \
do {                                                                         \
    if(ll >= mereNewlBufSz)                                                  \
    {                                                                        \
        if((mereNewlBuf = meRealloc(mereNewlBuf,ll+128)) == NULL)            \
        {                                                                    \
            mereNewlBufSz = 0 ;                                              \
            return FALSE ;                                                   \
        }                                                                    \
        mereNewlBufSz = ll+127 ;                                             \
    }                                                                        \
} while(0)

#ifdef _GNU_REGEX

struct re_pattern_buffer mereBuffer={0} ;
struct re_registers mereRegs={0} ;
uint8        mereLastPat[NPAT]="";  /* last pattern array - reset by set-char-mask */
static uint8 mereTranslate[256] ;
int          mereNumRegs=0 ;
static int   mereNumNewl=0 ;


static int
mere_compile_pattern(uint8 *apat)
{
    const char *err ;
    char cc, *ss ;
    int ii, ll ;
    
    if(!mereBuffer.allocated)
    {
        re_syntax_options = RE_CHAR_CLASSES|RE_INTERVALS ;
        mereBuffer.allocated = 8;
        mereBuffer.buffer = malloc(mereBuffer.allocated);
        mereBuffer.fastmap = malloc(256);
        mereBuffer.regs_allocated = REGS_UNALLOCATED;
    }
    else if(!strcmp(apat,mereLastPat))
        return meREGEX_OKAY ;

    /* set-up the translation table */
    if(exactFlag)
    {
        /* Map uppercase characters to corresponding lowercase ones.  */
        for (ii = 1; ii < 256 ; ii++)
            mereTranslate[ii] = toLower(ii) ;
        mereBuffer.translate = mereTranslate ;
    }
    else
        mereBuffer.translate = NULL ;
    
    mereNumNewl = 0 ; 
    ss = apat ;
    ll = 0 ;
    while((cc=*ss++) != '\0')
    {
        if(cc == meNLCHAR)
            mereNumNewl++ ;
        ll++ ;
    }
    if((err=re_compile_pattern (apat,ll,&mereBuffer)) != NULL)
    {
        if(reportErrors)
            mlwrite(MWABORT,"[Regex Error: %s]",err) ;
        return meREGEX_ERROR_UNKNOWN ;
    }
    strncpy(mereLastPat,apat,NPAT-1) ;
    mereLastPat[NPAT-1] = '\0' ;
    return meREGEX_OKAY ;
}

static int
mere_scanner(int direct, int beg_or_end, int *n, SCANNERPOS *sp)
{
    register LINE *lp=curwp->w_dotp, *nlp ;
    register int32 lnno=curwp->line_no, nlnno ;
    register int   ii=curwp->w_doto, jj=llength(lp), kk, count=*n ;
    
    srchLastState = FALSE ;
    /* Moved the TTbreakTest() to the start of the loop as some of the
     * main problems as caused by search string with no length, such as ^,
     * or [ ]*$ on a line with no end spaces. In these cases is the user
     * is trying to replace it then they couldn't interupt as this
     * immediately returned success, hence the move!
     */
    if(TTbreakTest(1))
    {
        ctrlg(FALSE,1) ;
        return FALSE ;
    }
    /* treat the first line as a special case */
    if(direct == FORWARD)
    {
        if(lp == curbp->b_linep)
            return FALSE ;
        ii = re_search_2(&mereBuffer,NULL,0,(char *) ltext(lp),jj,
                         ii,jj-ii,&mereRegs,jj) ;
    }
    /* special case on the first line if going backward, if ii is 0
     * it cannot match and we must catch this case as 0-ii == 0 == forward! */
    else if(ii != 0)
    {
        ii = re_search_2(&mereBuffer,NULL,0,(char *) ltext(lp),jj,
                         ii-1,0-ii,&mereRegs,jj) ;
    }
    else
        ii = -1 ;
    
    if(ii >= 0)
    {
        kk = mereRegs.end[0] ;
        nlp = lp ;
        nlnno = lnno ;
        jj = kk - ii ;
        mereNewlBufSzCheck(jj) ;
        memcpy(mereNewlBuf,ltext(lp)+ii,jj) ;
        srchLastMatch = (char *) mereNewlBuf ;
    }
    else if(mereNumNewl == 0)
    {
        if(direct == FORWARD)
        {
            /* On entry, if count=0 then no line limit, therefore test if (--count == 0) as count == -1 */ 
            while((--count != 0) && ((lp=lforw(lp)) != curbp->b_linep))
            {
                if(TTbreakTest(1))
                {
                    ctrlg(FALSE,1) ;
                }
                lnno++ ;
                ii = llength(lp) ;
                if((ii=re_search_2(&mereBuffer,NULL,0,(char *) ltext(lp),ii,
                                   0,ii,&mereRegs,ii)) >= 0)
                    break ;
            }
        }
        else
        {
            while((--count != 0) && ((lp=lback(lp)) != curbp->b_linep))
            {
                if(TTbreakTest(1))
                {
                    ctrlg(FALSE,1) ;
                    return FALSE ;
                }
                lnno-- ;
                ii = llength(lp) ;
                if((ii=re_search_2(&mereBuffer,NULL,0,(char *) ltext(lp),ii,
                                   ii,-ii,&mereRegs,ii)) >= 0)
                    break ;
            }
        }
        if(ii < 0)
            return FALSE ;
        kk = mereRegs.end[0] ;
        nlp = lp ;
        nlnno = lnno ;
        jj = kk - ii ;
        mereNewlBufSzCheck(jj) ;
        memcpy(mereNewlBuf,ltext(lp)+ii,jj) ;
        srchLastMatch = (char *) mereNewlBuf ;
    }
    else if(--count == 0)
        return FALSE ;
    else
    {
        /* treat the 1st-2nd line as a special case */
        ii = curwp->w_doto ;
        if(direct == FORWARD)
        {
            nlp = lforw(lp) ;
            kk = llength(nlp) ;
            lputc(lp,jj,meNLCHAR) ;
            ii = re_search_2(&mereBuffer,(char *) ltext(lp),jj+1,(char *) ltext(nlp),kk,
                             ii,jj+1+kk-ii,&mereRegs,jj+1+kk) ;
            lputc(lp,jj,0) ;
        }
        else
        {
            nlp = lp ;
            lp = lback(lp) ;
            if(lp == curbp->b_linep)
                return FALSE ;
            lnno-- ;
            kk = jj ;
            jj = llength(lp) ;
            lputc(lp,jj,meNLCHAR) ;
            ii = re_search_2(&mereBuffer,(char *) ltext(lp),jj+1,(char *) ltext(nlp),kk,
                             jj+1+((ii) ? ii-1:0),0-(jj+1+ii),&mereRegs,jj+1+kk) ;
            lputc(lp,jj,0) ;
        }
        if(ii < 0)
        {
            if(mereNumNewl == 1)
            {
                if(direct == FORWARD)
                {
                    /* On entry, if count=0 then no line limit, therefore test if (--count == 0) as count == -1 */ 
                    while((--count != 0) && ((lp=nlp) != curbp->b_linep))
                    {
                        if(TTbreakTest(1))
                        {
                            ctrlg(FALSE,1) ;
                            return FALSE ;
                        }
                        lnno++ ;
                        jj = kk ;
                        nlp = lforw(nlp) ;
                        kk = llength(nlp) ;
                        lputc(lp,jj,meNLCHAR) ;
                        ii = re_search_2(&mereBuffer,(char *) ltext(lp),jj+1,(char *) ltext(nlp),kk,
                                         0,jj+1+kk,&mereRegs,jj+1+kk) ;
                        lputc(lp,jj,0) ;
                        if(ii >= 0)
                            break ;
                    }
                }
                else
                {
                    /* On entry, if count=0 then no line limit, therefore test if (--count == 0) as count == -1 */ 
                    while((--count != 0) && ((nlp=lp),((lp=lback(lp)) != curbp->b_linep)))
                    {
                        if(TTbreakTest(1))
                        {
                            ctrlg(FALSE,1) ;
                            return FALSE ;
                        }
                        lnno-- ;
                        kk = jj ;
                        jj = llength(lp) ;
                        lputc(lp,jj,meNLCHAR) ;
                        ii = re_search_2(&mereBuffer,(char *) ltext(lp),jj+1,(char *) ltext(nlp),kk,
                                         jj+1+kk,0-(jj+1+kk),&mereRegs,jj+1+kk) ;
                        lputc(lp,jj,0) ;
                        if(ii >= 0)
                            break ;
                    }
                }
                
            }
            else
            {
                /* generic multi-line case */
                int noln=2, tnoln=mereNumNewl+1, offs ;
                
                if(direct == FORWARD)
                    offs = curwp->w_doto ;
                else
                    offs = llength(curwp->w_dotp)-curwp->w_doto ;
                while(--count != 0)
                {
                    if(TTbreakTest(1))
                    {
                        ctrlg(FALSE,1) ;
                        return FALSE ;
                    }
                    if(direct == FORWARD)
                    {
                        if(nlp == curbp->b_linep)
                            break ;
                        if(noln == tnoln)
                        {
                            lnno++ ;
                            lp = lforw(lp) ;
                        }
                    }
                    else
                    {
                        if((lp=lback(lp)) == curbp->b_linep)
                            break ;
                        lnno-- ;
                    }
                    if(noln == tnoln)
                        offs = 0 ;
                    else
                        noln++ ;
                    ii = noln ;
                    jj = ii ;
                    nlp = lp ;
                    while(--ii >= 0)
                    {
                        jj += llength(nlp) ;
                        nlp = lforw(nlp) ;
                    }
                    mereNewlBufSzCheck(jj) ;
                    ii = noln ;
                    jj = 0 ;
                    nlp = lp ;
                    for(;;)
                    {
                        kk = llength(nlp) ;
                        memcpy(mereNewlBuf+jj,ltext(nlp),kk) ;
                        jj += kk ;
                        /* we don't want the last \n or move nlp to past the
                         * last line so drop out here */
                        if(--ii == 0)
                            break ;
                        mereNewlBuf[jj++] = meNLCHAR ;
                        nlp = lforw(nlp) ;
                    }
                    if(direct == FORWARD)
                        ii = re_search_2(&mereBuffer,NULL,0,mereNewlBuf,jj,
                                         offs,jj,&mereRegs,jj) ;
                    else
                        ii = re_search_2(&mereBuffer,NULL,0,mereNewlBuf,jj,
                                         jj-((offs) ? offs-1:0),0-(jj-offs),&mereRegs,jj) ;
                    if(ii >= 0)
                        break ;
                }
            }
            if(ii < 0)
                return FALSE ;
        }
        kk = mereRegs.end[0] ;
        srchLastMatch = (char *) mereNewlBuf+ii ;
        while(ii > llength(lp))
        {
            jj = llength(lp)+1 ;
            ii -= jj ;
            kk -= jj ;
            lnno++ ;
            lp = lforw(lp) ;
        }
        nlp = lp ;
        nlnno = lnno ;
        while(kk > llength(nlp))
        {
            kk -= llength(nlp)+1 ;
            nlnno++ ;
            nlp = lforw(nlp) ;
        }
    }
    if(beg_or_end == PTEND)
    {
        curwp->w_dotp = nlp ;
        curwp->line_no = nlnno ;
        curwp->w_doto = kk ;
    }
    else
    {
        curwp->w_dotp = lp ;
        curwp->line_no = lnno ;
        curwp->w_doto = ii ;
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
    setShowRegion(curbp,lnno,ii,nlnno,kk) ;
    curwp->w_flag |= WFMOVEL|WFSELHIL ;
    /* change the regs \( \) start and end points so the start is at zero */
    ii = mereRegexGroupStart(0) ;
    jj = mereNumRegs = mereBuffer.re_nsub ;
    do {
        if(mereRegexGroupStart(jj) >= 0)
        {#
            mereRegexGroupStart(jj) -= ii ;
            mereRegexGroupEnd(jj)   -= ii ;
        }
    } while(--jj >= 0) ;
    srchLastMatch[mereRegexGroupEnd(0)] = '\0' ;
    srchLastState = TRUE ;
    return TRUE ;
}

#else

meRegex mereRegex={0} ;

static int
mere_compile_pattern(uint8 *apat)
{
    int ii ;
    
    if(((ii=meRegexComp(&mereRegex,apat,
                        (exactFlag) ? meREGEX_ICASE:0)) != meREGEX_OKAY) && reportErrors)
        mlwrite(MWABORT,(uint8 *)"[Regex Error: %s]",meRegexCompErrors[ii]) ;
    return ii ;
}

static int
mere_scanner(int direct, int beg_or_end, int *n, SCANNERPOS *sp)
{
    register LINE *lp=curwp->w_dotp, *nlp ;
    register int32 lnno=curwp->line_no, nlnno ;
    register int   ii=curwp->w_doto, jj, kk, count=*n, flags ;
    
    srchLastState = FALSE ;
        
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
            ctrlg(FALSE,1) ;
            return FALSE ;
        }
		flags = (lp == curbp->b_linep) ? meREGEX_ENDBUFF:0 ;
        if(lnno == 0)
            flags |= meREGEX_BEGBUFF ;
		jj = llength(lp) ;
        if(direct == FORWARD)
        {
            if((lp == curbp->b_linep) &&
               (!(mereRegex.flags & meREGEX_MAYENDBUFF) ||
                !(mereRegex.flags & meREGEX_MAYBEEMPTY) ))
                /* never match $ or \n on the last line going forwards, only \' */
                return FALSE ;
            kk = jj ;
        }
        else
        {
            flags |= meREGEX_BACKWARD ;
            /* when searching backward, we must not match the current position
             * forwrds, always the char before and onwards. if ii == 0 then its now -1! */
            kk = ii - 1 ;
			ii = 0 ;
        }
        if((kk < 0) || ((ii=meRegexMatch(&mereRegex,ltext(lp),jj,ii,kk,flags)) == 0))
        {
            if(direct == FORWARD)
            {
                flags &= ~meREGEX_BEGBUFF ;
                /* On entry, if count=0 then no line limit, therefore test if (--count == 0) as count == -1 */ 
                while((--count != 0) && !(flags & meREGEX_ENDBUFF))
                {
                    if(TTbreakTest(1))
                    {
                        ctrlg(FALSE,1) ;
                        return FALSE ;
                    }
                    lnno++ ;
                    if((lp=lforw(lp)) == curbp->b_linep)
                    {
                        if(!(mereRegex.flags & meREGEX_MAYENDBUFF) ||
                           !(mereRegex.flags & meREGEX_MAYBEEMPTY) )
                            break ;
                        flags |= meREGEX_ENDBUFF ;
                    }
                    ii = llength(lp) ;
                    if((ii=meRegexMatch(&mereRegex,ltext(lp),ii,0,ii,flags)) != 0)
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
                        ctrlg(FALSE,1) ;
                        return FALSE ;
                    }
                    lp = lback(lp) ;
                    ii = llength(lp) ;
                    if(--lnno == 0)
                        flags |= meREGEX_BEGBUFF ;
                    if((ii=meRegexMatch(&mereRegex,ltext(lp),ii,0,ii,flags)) != 0)
                        break ;
                }
            }
            if(!ii)
                return FALSE ;
        }
        ii = mereRegex.group[0].start ;
        kk = mereRegex.group[0].end ;
        nlp = lp ;
        nlnno = lnno ;
        jj = kk - ii ;
        mereNewlBufSzCheck(jj) ;
        memcpy(mereNewlBuf,ltext(lp)+ii,jj) ;
        srchLastMatch = mereNewlBuf ;
    }
    else
    {
        /* generic multi-line case */
        int noln=mereRegex.newlNo+1, offs, offe ;
        
        if(direct == FORWARD)
        {
            offs = ii ;
            offe = llength(lp)+1 ;
        }
        else
        {
            if((offe = ii-1) < 0)
            {
                if(--lnno < 0)
                    return FALSE ;
                lp = lback(lp) ;
                offe = llength(lp)+1 ;
            }
            offs = 0 ;
        }
        do
        {
            if(TTbreakTest(1))
            {
                ctrlg(FALSE,1) ;
                return FALSE ;
            }
            flags = (lnno == 0) ? meREGEX_BEGBUFF:0 ;
            ii = noln ;
            jj = ii ;
            nlp = lp ;
            do
            {
                if(nlp == curbp->b_linep)
                    break ;
                jj += llength(nlp) ;
                nlp = lforw(nlp) ;
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
                kk = llength(nlp) ;
                memcpy(mereNewlBuf+jj,ltext(nlp),kk) ;
                jj += kk ;
                /* we don't want the last \n or move nlp to past the
                 * last line so drop out here */
                if(--ii == 0)
                    break ;
                mereNewlBuf[jj++] = meNLCHAR ;
                nlp = lforw(nlp) ;
            }
            mereNewlBuf[jj] = '\0' ;
            
            if(direct != FORWARD)
				flags |= meREGEX_BACKWARD ;
			ii = meRegexMatch(&mereRegex,mereNewlBuf,jj,offs,offe,flags) ;
            if(ii)
                break ;
            
            if(direct == FORWARD)
            {
                if(flags & meREGEX_ENDBUFF)
                    break ;
                lp = lforw(lp) ;
				lnno++ ;
            }
            else
            {
                if(flags & meREGEX_BEGBUFF)
                    break ;
                lp = lback(lp) ;
				lnno-- ;
            }
			offs = 0 ;
			offe = llength(lp)+1 ;
        } while(--count != 0) ;
        if(!ii)
            return FALSE ;
        ii = mereRegex.group[0].start ;
        kk = mereRegex.group[0].end ;
        srchLastMatch = mereNewlBuf+ii ;
        while(ii > llength(lp))
        {
            jj = llength(lp)+1 ;
            ii -= jj ;
            kk -= jj ;
            lnno++ ;
            lp = lforw(lp) ;
        }
        nlp = lp ;
        nlnno = lnno ;
        while(kk > llength(nlp))
        {
            kk -= llength(nlp)+1 ;
            nlnno++ ;
            nlp = lforw(nlp) ;
        }
    }    
    if(beg_or_end == PTEND)
    {
        curwp->w_dotp = nlp ;
        curwp->line_no = nlnno ;
        curwp->w_doto = kk ;
    }
    else
    {
        curwp->w_dotp = lp ;
        curwp->line_no = lnno ;
        curwp->w_doto = ii ;
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
    setShowRegion(curbp,lnno,ii,nlnno,kk) ;
    curwp->w_flag |= WFMOVEL|WFSELHIL ;
    /* change the start and end position */
    mereRegexGroupEnd(0) -= mereRegex.group[0].start ;
    mereRegexGroupStart(0) = 0 ;
    srchLastMatch[mereRegexGroupEnd(0)] = '\0' ;
    srchLastState = TRUE ;
    *n = count ;
    return TRUE ;
}

#endif

#endif


#if 0
/* Local copy eq () for search */

static int
cEq (register int bc, register int pc)
{
    
    if (bc == pc)                       /* Quick test first - think positive */
        return (TRUE);                  /* Phew - gambit paid off */
    if (!exactFlag)                     /* Get exact out the way quick */
        return (FALSE);                 /* Failed. !! */
    
    /*
     * We are not in exact mode - if we are dealing with alphabetic then
     * convert to the same case and compare. 
     */
    
    bc = toLower(bc) ;
    pc = toLower(pc) ;
    return (bc == pc);
}

#define cNotEq(a,b) (!cEq(a,b))

#else

#define cNotEq(a,b)                                                          \
(((a) != (b)) &&                                                             \
 (!exactFlag || (toggleCase(a) != (b))))

#define cEq(a,b)                                                             \
(((a) == (b)) ||                                                             \
 (exactFlag && (toggleCase(a) == (b))))

#endif

/*
 * scanner -- Search for a pattern in either direction.
 */
static int 
scanner(uint8 *patrn, int direct, int beg_or_end, int *count,
        SCANNERPOS *sp)
{
    register uint8 cc, aa, bb ;	        /* character at current position */
    register uint8 *patptr;	        /* pointer into pattern */
    register uint8 *curptr;	        /* pointer into pattern */
    register LINE *curline;		/* current line during scan */
    register int curoff;		/* position within current line */
    register uint8 *matchptr;	        /* pointer into pattern */
    register LINE *matchline;		/* current line during matching */
    register int matchoff;		/* position in matching line */
    register int32 curlnno, matchlnno ;
    
    srchLastState = FALSE ;
    /* If we are going in reverse, then the 'end' is actually the beginning of
     * the pattern. Toggle it.
     */
    beg_or_end ^= direct;
    
    /* Setup local scan pointers to global ".". */
    curline = curwp->w_dotp;
    curlnno = curwp->line_no;
    if (direct == FORWARD)
        curptr = curline->l_text+curwp->w_doto ;
    else			/* Reverse.*/
        curoff = curwp->w_doto;
    
    /* Scan each character until we hit the head link record. */
/*    if (boundry(curline, curoff, direct))*/
/*        return FALSE ;*/
    
    aa = patrn[0] ;
    for (;;)
    {
        /* Get the character resolving newlines, and
         * test it against first char in pattern.
         */
        if (direct == FORWARD)
        {
            if((cc=*curptr++) == '\0')		/* if at EOL */
            {
                if(curline != curbp->b_linep)
                {
                    curline = lforw(curline) ;	/* skip to next line */
                    curlnno++ ;
                    curptr = curline->l_text ;
                    cc = meNLCHAR;		/* and return a <NL> */
                    if((*count > 0) && (--(*count) == 0) &&
                       ((patrn[0] != meNLCHAR) || (patrn[1] != '\0')))
                        /* must check that the pattern isn't "\n" */
                        break ;
                }
            }
        }
        else			/* Reverse.*/
        {
            if (curoff == 0)
            {
                if((curline = lback(curline)) == curbp->b_linep)
                    cc = '\0';
                else
                {
                    if((*count > 0) && (--(*count) == 0))
                        break ;
                    curlnno-- ;
                    curoff = llength(curline);
                    cc = meNLCHAR;
                }
            }
            else
                cc = lgetc(curline, --curoff);
        }
        if(cc == '\0')
            break ;
        
        if(cEq(cc,aa))	/* if we find it..*/
        {
            /* Setup match pointers.
            */
            matchline = curline ;
            matchlnno = curlnno ;
            matchptr = curptr ;
            matchoff = curoff ;
            patptr = &patrn[0];
            
            /* Scan through the pattern for a match.
            */
            while((bb = *++patptr) != '\0')
            {
                if (direct == FORWARD)
                {
                    if((cc=*matchptr++) == '\0')		/* if at EOL */
                    {
                        if(matchline != curbp->b_linep)
                        {
                            matchline = lforw(matchline) ;	/* skip to next line */
                            matchlnno++ ;
                            matchptr = matchline->l_text ;
                            cc = meNLCHAR;			/* and return a <NL> */
                        }
                    }
                }
                else			/* Reverse.*/
                {
                    if (matchoff == 0)
                    {
                        if((matchline = lback(matchline)) == curbp->b_linep)
                            cc = '\0';
                        else
                        {
                            matchlnno-- ;
                            matchoff = llength(matchline);
                            cc = meNLCHAR;
                        }
                    }
                    else
                        cc = lgetc(matchline, --matchoff);
                }
                if (cNotEq(cc,bb))
                    goto fail;
            }
            
            /*
             * A SUCCESSFULL MATCH!!!
             * reset the curwp "." pointers
             * Fill in the scanner position structure if required.
             */
            /* Save the current position in case we need to restore it on a
             * match.
             */
            if (direct == FORWARD)
            {
                if(curptr == curline->l_text)
                {
                    curline = lback(curline) ;
                    curlnno-- ;
                    curoff = llength(curline);
                }
                else
                    curoff = ((size_t) curptr) - ((size_t) curline->l_text) - 1 ;
                matchoff = ((size_t) matchptr) - ((size_t) matchline->l_text) ;
            }
            else			/* Reverse.*/
            {
                if(curoff == llength(curline))		/* if at EOL */
                {
                    curline = lforw(curline) ;	/* skip to next line */
                    curlnno++ ;
                    curoff = 0;
                }
                else
                    curoff++ ;	/* get the char */
            }
        
            
            if (sp != NULL)
            {
                sp->startline = curline;
                sp->startoff = curoff;
                sp->startline_no = curlnno ;
                sp->endline = matchline;
                sp->endoffset = matchoff;
                sp->endline_no = matchlnno ;
            }
            setShowRegion(curbp,curlnno,curoff,matchlnno,matchoff) ;
            if (beg_or_end == PTEND)	/* at end of string */
            {
                curwp->w_dotp = matchline ;
                curwp->w_doto = matchoff ;
                curwp->line_no = matchlnno ;
            }
            else		/* at beginning of string */
            {
                curwp->w_dotp = curline;
                curwp->w_doto = curoff;
                curwp->line_no = curlnno ;
            }
             /* Flag that we have moved and got a region to hilight.*/
            curwp->w_flag |= WFMOVEL|WFSELHIL ;
            srchLastMatch = srchPat ;
            srchLastState = TRUE ;
            return TRUE;
            
        }
fail:			/* continue to search */
        if(TTbreakTest(1))
        {
            ctrlg(FALSE,1) ;
            return FALSE ;
        }
        
/*        if (boundry(curline, curoff, direct))*/
/*            break;*/
        /* Quit when we have done our work !! */
    }
    
    return FALSE;	/* We could not find a match */
}

int
expandchar(int c, uint8 *d, int flags)
{
    register int  doff=1 ;

    if((flags & meEXPAND_PRINTABLE) ? isPrint(c): isDisplayable(c))
    {
        if((flags & meEXPAND_BACKSLASH) && ((c == '\\') || (c == '"')))
        {
            *d = '\\';
            d[doff++] = c;
        }
        else			/* any other character */
            *d = c;
    }
    else if(c < 0x20)		/* control character */
    {   /* 0 -> \C@ this may not be okay */
        *d = '\\';
        d[doff++] = 'C' ;
        d[doff++] = c ^ 0x40;
    }
    else        		/* its a nasty character */
    {
        *d = '\\';
        d[doff++] = 'x';
        d[doff++] = hexdigits[c >> 4] ;
        d[doff++] = hexdigits[c&0x0f] ;
    }
    return doff ;
}

int
expandexp(int slen, uint8 *s, int dlen, int doff, uint8 *d, 
          int cpos, int *opos, int flags)
{
    uint8 cc ;
    int ii ;
    
    if(slen < 0)
        slen = meStrlen(s) ;
    for(ii=0 ; (doff<dlen-6) && (ii<slen) ; ii++)
    {
        if(cpos == ii)
            *opos = doff ;
        if(((cc = *s++) == 0xff) && (flags & meEXPAND_FFZERO))
        {
            if((cc = *s++) == 2)
            {
                d[doff++] = '\\' ;
                d[doff++] = 's' ;
                cc = *s++ ;
                ii++ ;
            }
            else if(cc == 1)
                cc = 0 ;
            ii++ ;
        }
        doff += expandchar( ((int) cc) & 0xff, d+doff, flags) ;
    }	/* End of 'while' */
    d[doff] = '\0' ;
    return doff ;
}

/*
 * rvstrcpy -- Reverse string copy.
 * 
 * The source buffer may vary, but the result is always to srchRPat
 */
static void
rvstrcpy(register uint8 *str)
{
    register uint8 *rvstr ;
    register int ii ;
    
    if((ii = meStrlen(str)) > NPAT-1)
        ii = NPAT-1;
    
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
readpattern(uint8 *prompt, int defnum)
{
    int	status;

    /* Read a pattern.  Either we get one,
     * or we just get the META charater, and use the previous pattern.
     * Then, if it's the search string, make a reversed pattern.
     * *Then*, make the meta-pattern, if we are defined that way.
     */
    if((status = meGetString(prompt,MLSEARCH,defnum,srchPat,NPAT)) == TRUE)
    {
        exactFlag = (meModeTest(curbp->b_mode,MDEXACT) == 0);
#if	MAGIC
        /* Only make the meta-pattern if in magic mode,
         * since the pattern in question might have an
         * invalid meta combination.
         */
        if((srchLastMagic=meModeTest(curbp->b_mode,MDMAGIC)) != 0)
            status = (mere_compile_pattern(srchPat) == meREGEX_OKAY) ;
        else
#endif
            rvstrcpy(srchPat);
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
#define	SL_INCREPLACE	3	/* Incremental replace */
#define	SL_FIRSTREPLACE	4	/* First replacement condition */
#define	SL_NEXTREPLACE	5	/* Next replacement condition */
#define	SL_QUERYINPUT	6	/* Get the query input from the user */
#define	SL_SUBSTITUTE	7	/* Substitute string */

/*---	Local variable defintions */
	
    static uint8 *dpat = NULL;
    static int dpatlen = 0;

    register int i;		/* loop index */
    register int slength;	/* Length of search string */
    register int rlength;	/* length of replace string */
    uint8        rpat[NPAT];	/* replacement pattern		*/
    int	         numsub;	/* number of substitutions */
    int	         nummatch;	/* number of found matches */
    int	         status;	/* success flag on pattern inputs */
    int	         nlflag;	/* last char of search string a <NL>? */
    int	         nlrepl;	/* was a replace done on the last line? */
    int	         origoff;	/* and offset (for . query option) */
    int32        origlno;       /* line no (for . query option) */
    LINE	*lastline = 0;	/* position of last replace and */
    int	         lastoff;       /* offset (for 'u' query option) */
    int32        lastlno;       /* line no (for 'u' query option) */
    int	onemore =FALSE;		/* only do one more replace */
    int	ilength = 0;		/* Last insert length */
    int	state_mc;		/* State machine */
    
    int 	cc;		/* input char for query */
    uint8	tmpc;		/* temporary character */
    uint8	tpat[NPAT];	/* temporary to hold search pattern */
    
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

    for (state_mc = SL_GETSEARCH; state_mc != SL_EXIT; /* NULL */)
    {
        switch (state_mc)
        {
            /*
             * Ask the user for the text of a pattern.
             */
        case SL_GETSEARCH:
            
            if ((status = readpattern
                 ((kind == FALSE ? (uint8 *)"Replace":(uint8 *)"Query replace"), 
                  1+lastReplace)) != TRUE)
		return (status);		/* Aborted out - Exit */
            
            slength = meStrlen (srchPat);	/* Get length of search string */
            
/*            mlwrite (MWWAIT,"Search [%d:%s]", slength, srchPat);*/
            
            /* Set up flags so we can make sure not to do a recursive
             * replace on the last line. */
            
            nlflag = (srchPat[slength - 1] == meNLCHAR);
            nlrepl = FALSE;
            state_mc = SL_GETREPLACE;	/* Move to next state */
            break;
            
        case SL_GETREPLACE :	/* Ask for replacement string */
        case SL_INCREPLACE :	/* Ask for incremental replacement */
            
            meStrcpy(tpat,"Replace [") ;
            i = expandexp(-1,srchPat, NPAT-17, 9, tpat, -1, NULL, 0) ;
            meStrcpy(tpat+i,"] with");
            
            if ((status=meGetString(tpat,MLSEARCH,0,rpat,NPAT)) != TRUE)
            {
                lastReplace = 0 ;
                return (status);
            }	/* End of 'if' */
            
            rlength = meStrlen(rpat);	/* Get length of replace string. */
            
            if (kind)	  	/* Build query replace question string.  */
            {
                tpat[i+6] = ' ' ;
                tpat[i+7] = '[' ;
                i = expandexp(-1,rpat, NPAT-i-13, i+8, tpat, -1, NULL, 0) ;
                meStrcpy(tpat+i,"] ? ") ;
            }            
            state_mc = (state_mc == SL_GETREPLACE) ?
                SL_FIRSTREPLACE : SL_NEXTREPLACE;
            lastReplace = 1 ;
            break;
            
        case SL_FIRSTREPLACE:		/* Do the first replacement */
            
            /* Save original '.' position, initialise  the number of matches 
               and substitutions, and scan through the file.  */
            
            origoff = curwp->w_doto;
            origlno = curwp->line_no;
            numsub = 0;
            nummatch = 0;
            
            /* Initialize last replaced pointers. */
            lastline = NULL;
            
            state_mc = SL_NEXTREPLACE;		/* Do the next replacement */
            break;	
            
        case  SL_NEXTREPLACE:		/* Next replcaement condition */
            
            if (!((nn < 0 || nn > nummatch) &&
                  (nlflag == FALSE || nlrepl == FALSE) &&
                  onemore == FALSE))
            {
                /*---	Exit condition has been met */
                
		state_mc = SL_EXIT;		/* State machine to exit */
		break;				/* Exit case */
            }	/* End of 'if' */
            
            /*--- Search  for  the  pattern.   If  we  search  with  a regular
              expression, also save the true length of matched string.  */
            
#if	MAGIC
            if(srchLastMagic)
            {
                if (!mere_scanner(FORWARD, PTBEG, &ff, NULL))
                {
                    state_mc = SL_EXIT;
                    break;
		}
            }
            else
#endif
                if (!scanner(srchPat, FORWARD, PTBEG, &ff, NULL))
            {
                state_mc = SL_EXIT;
                break;		/* all done */
            }
            
            ++nummatch;	/* Increment # of matches */
            
            /* Check if we are on the last line. */
            
            nlrepl = (lforw(curwp->w_dotp) == curbp->b_linep);
            
            if (kind)
		state_mc = SL_QUERYINPUT;
            else
		state_mc = SL_SUBSTITUTE;
            break;
            
        case SL_QUERYINPUT:			/* Get input from user. */
            
#if MEUNDO
            undoContFlag++ ;
#endif
            cc = mlCharReply(tpat,mlCR_LOWER_CASE|mlCR_UPDATE_ON_USER|mlCR_CURSOR_IN_MAIN,(uint8 *)"y nil!u.",
                             (uint8 *)"(Y)es (N)o (I)nc (L)ast (!)Do rest (U)ndo (^G)Abort (.)Abort back ? ");
            
            switch (cc)			/* And respond appropriately. */
            {
            case -1:
                return ctrlg(FALSE,1);
                
            case 'y':			/* yes, substitute */
            case ' ':
		state_mc = SL_SUBSTITUTE;
		break;
                
            case 'n':			/* no, onword */
		WforwChar(curwp, 1);
		state_mc = SL_NEXTREPLACE;
		break;
                
            case 'i':			/* Incremental change */
		state_mc = SL_INCREPLACE;
		break;
							
            case 'l':			/* last one */
		onemore = TRUE;
		state_mc = SL_SUBSTITUTE;
		break;
                
            case '!':			/* yes/stop asking */
		kind = FALSE;
		state_mc = SL_SUBSTITUTE;
		break;

            case 'u':	/* undo last and re-prompt */
                
                if (lastline == NULL)
                {   /* Restore old position. */
                    TTbell();		/* There is nothing to undo. */
                    break;
		}
                
                /*---	Reset the last position */
                
                curwp->w_dotp  = lastline ; 
                curwp->w_doto  = lastoff ;
                curwp->line_no = lastlno ;
                curwp->w_flag |= WFMOVEL ;
		lastline = NULL;
                
		/* Delete the new string. */
                
		WbackChar(curwp, ilength);
                if (!ldelete(ilength,6))
                    return mlwrite(MWABORT,(uint8 *)"[ERROR while deleting]");
                
		/* And put in the old one. */
                
                for (i = 0; i < slength; i++)
                {
                    tmpc = dpat[i];
                    status = ((tmpc == meNLCHAR) ? lnewline(): linsert(1, tmpc));
                    
                    /* Insertion error? */
                    
                    if (!status)
                        return mlwrite(MWABORT,(uint8 *)"[Out of memory while inserting]");
                
                }	/* End of 'for' */
#if MEUNDO
                meUndoAddReplaceEnd(slength) ;
#endif
		/* Record one less substitution, backup, and reprompt. */
                
		--numsub;
                lastoff = curwp->w_doto ;
                lastlno = curwp->line_no ;
                WbackChar(curwp, slength);
                /* must research for this instance to setup the auto-repeat sections
                 * correctly */
                --nummatch;	/* decrement # of matches */
                state_mc = SL_NEXTREPLACE ;
		break;
		
            case '.':	/* abort! and return */
                /* restore old position */
                gotoLine(TRUE,origlno+1) ;
                curwp->w_doto  = origoff ;
                state_mc = SL_EXIT;		/* Exit state machine */
		break;
            }	/* end of switch */
            break;
            
            /*
             * Delete the sucker.
             */
        case SL_SUBSTITUTE:			/* Substitute the string */
            
            if(bchange() != TRUE)               /* Check we can change the buffer */
                return ABORT ;
#if	MAGIC
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
                    return FALSE ;
                }
            }
            if(mldelete(slength,dpat) != 0)
                return mlwrite(MWABORT,(uint8 *)"[ERROR while deleting]") ;
            ilength = 0 ;
            
            /*
             * And insert its replacement.
             */
            
            for (i = 0 ; i < rlength; i++)
            {
                tmpc = rpat[i] ;
#if	MAGIC
                if (srchLastMagic && 	               /* Magic string ?? */
                    (tmpc == '\\') && 
                    (i+1 < rlength))                   /* Not too long ?? */
                {
                    tmpc = rpat[++i] ;
                    
                    if ((tmpc == '&') ||
                        ((tmpc >= '0') && (tmpc <= '9') && ((int)(tmpc-'0' <= mereRegexGroupNo()))))
                    {
                        /* Found auto repeat char replacement */
                        register int j,k;	/* Local loop counter */
                        
                        tmpc -= (tmpc == '&') ? '&':'0' ;
                        
                        /* if start offset is < 0, it was a ? auto repeat which was not found,
                           therefore replace str == "" */ 
                        if((j=mereRegexGroupStart(tmpc)) >= 0)
                        {
                            for(k=mereRegexGroupEnd(tmpc) ;
                                ((j < k) && (status != FALSE)); ilength++)
                            {
                                if ((tmpc=dpat[j++]) != meNLCHAR)
                                    status = linsert(1, tmpc);
                                else
                                    status = lnewline ();
                            }
                        }
                    }
                    else
                    {
                        if(tmpc == 'n')
                            tmpc = '\n' ;
                        else if(tmpc == 'r')
                            tmpc = '\r' ;
                        else if(tmpc == 't')
                            tmpc = '\t' ;
                        if (tmpc != meNLCHAR)
                            status = linsert(1, tmpc);
                        else
                            status = lnewline ();
                        ilength++;
                    }
                }
                else
#endif
                {
                    if (tmpc != meNLCHAR)
                        status = linsert(1, tmpc);
                    else
                        status = lnewline ();
                    ilength++;
                }
                /* Insertion error ? */
                
                if (!status) 
                    return mlwrite(MWABORT,(uint8 *)"[Out of memory while inserting]"); 
            }	/* End of 'for' */
#if MEUNDO
            meUndoAddReplace(dpat,ilength) ;
#endif
            
            if (kind) 
            {			/* Save position for undo. */
		lastline = curwp->w_dotp;
		lastoff = curwp->w_doto; 
		lastlno = curwp->line_no; 
            }
            numsub++;			/* increment # of substitutions */
            state_mc = SL_NEXTREPLACE;	/* Do next replacement */
            if (slength == 0)
            {
                WforwChar(curwp, 1);
                if ((ff > 0) && (curwp->w_doto == 0) && (--ff == 0))
                    state_mc = SL_EXIT;
            }
            break;
            
        default:
            return mlwrite(MWABORT,(uint8 *)"[Bad state machine value %d]", state_mc);
        }	/* End of 'switch' on state_mc */
    }   /* End of 'for' */
    
    /*---	And report the results. */
    return mlwrite(((nn < 0) || (numsub == nn)) ? MWCLEXEC:(MWABORT|MWCLEXEC),(uint8 *)"%d substitutions", numsub);
}	/* End of 'replaces' */

/*
 * replaceStr -- Search and replace.
 */
int
replaceStr(int f, int n)
{
    return(replaces(FALSE, f, n));
}

/*
 * queryReplaceStr -- search and replace with query.
 */
int	
queryReplaceStr(int f, int n)
{
    return(replaces(TRUE, f, n));
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
     * response is TRUE (responses other than FALSE are
     * possible), search for the pattern.
     */
    if ((status = readpattern((uint8 *)"Search", 1+lastReplace)) == TRUE)
    {
        lastReplace = 0 ;
        do
        {
#if	MAGIC
            if(srchLastMagic)
                status = mere_scanner(FORWARD, PTEND, &n, NULL);
            else
#endif
                status = scanner(srchPat, FORWARD, PTEND, &n, NULL);
        } while ((--repCount > 0) && status);
        
        /* ...and complain if not there.
         */
        if(status == FALSE)
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
     * response is TRUE (responses other than FALSE are
     * possible), search for the pattern.
     */
    if ((status = readpattern((uint8 *)"Reverse search", 1+lastReplace)) == TRUE)
    {
        lastReplace = 0 ;
        do
        {
#if	MAGIC
            if(srchLastMagic)
                status = mere_scanner(REVERSE, PTBEG, &n, NULL);
            else
#endif
                status = scanner(srchRPat, REVERSE, PTBEG, &n, NULL);
        } while ((--repCount > 0) && status);
        
        /* ...and complain if not there.
         */
        if(status == FALSE)
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
        return mlwrite(MWABORT,(uint8 *)"[No pattern set]");
    
#if MAGIC
    /* if magic the recompile the search string */
    if(srchLastMagic && (mere_compile_pattern(srchPat) != meREGEX_OKAY))
        return ABORT ;
#endif
    
    /* Search for the pattern... */
    do
    {
#if MAGIC
        if(srchLastMagic)
            status = mere_scanner(FORWARD, PTEND, &n, NULL);
        else
#endif
            status = scanner(srchPat, FORWARD, PTEND, &n, NULL);
    } while ((--repCount > 0) && status);
    
    /* ...and complain if not there. */
    if(status == FALSE)
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
        return mlwrite(MWABORT,(uint8 *)"[No pattern set]");
    
#if MAGIC
    if(srchLastMagic)
    {
        /* if magic then recompile the search string */
        if(mere_compile_pattern(srchPat) != meREGEX_OKAY)
            return ABORT ;
    }
    else
#endif
        rvstrcpy(srchPat);
    
    /* Go search for it... */
    do
    {
#if	MAGIC
        if(srchLastMagic)
            status = mere_scanner(REVERSE, PTBEG, &n, NULL);
        else
#endif
            status = scanner (srchRPat, REVERSE, PTBEG, &n, NULL);
    } while ((--repCount > 0) && status);
    
    /* ...and complain if not there. */
    if(status == FALSE)
        notFound();
    return(status);
}


int
iscanner(uint8 *apat, int n, int flags, SCANNERPOS *sp)
{
    int direct, magic;
    int beg_or_end;
    
    direct = (flags & ISCANNER_BACKW) ? REVERSE:FORWARD ;
    beg_or_end = (flags & ISCANNER_PTBEG) ? PTBEG : PTEND;
    exactFlag = ((flags & ISCANNER_EXACT) == 0);

#if	MAGIC
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
            reportErrors = FALSE;
        status = mere_compile_pattern(apat);
        if (flags & ISCANNER_QUIET)
            reportErrors = TRUE;        
        /*
         * Bail out here if we have a false status, we cannot construct a valid
         * search string yet it is not compleate.
         */
        if (status != meREGEX_OKAY)
            return ((status < meREGEX_ERROR_CLASS) ? -2:-1) ;
        
        return mere_scanner(direct,beg_or_end,&n,sp);
    }
#endif
    
    if(direct == REVERSE)
        /* Reverse string copy */
        rvstrcpy(apat);
    
    return scanner (((direct == FORWARD) ? apat : srchRPat),direct, beg_or_end, &n, sp);
}

#ifdef _ME_FREE_ALL_MEMORY
void srchFreeMemory(void)
{
#if MAGIC
#ifdef _GNU_REGEX
#else
    meRegexFree(&mereRegex) ;
#endif
#endif
}
#endif

