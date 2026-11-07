/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * undo.c - Undo & Redo support routines.
 *
 * Copyright (C) 1996-2001 Steven Phillips
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
 * Created:     1996
 * Synopsis:    Undo & Redo support routines.
 * Authors:     Steven Phillips
 */

#define	__UNDOC	1			/* Define the filename */

#include "emain.h"

#if MEOPT_UNDO

meUndoNode *
meUndoCreateNode(size_t size)
{
    meWindow *cwp=frameCur->windowCur;
    meBuffer *cbp=cwp->buffer;
    meUndoNode *nn;

    if((nn = meMalloc(size)) != NULL)
    {
        nn->type = 0;
        nn->next = cbp->undoHead;
        cbp->undoHead = nn;
        nn->udata.dotp = cwp->dotLineNo;
        nn->doto  = cwp->dotOffset;
        nn->str[0] = '\0';
        if(cbp->undoContFlag == undoContFlag)
            nn->type |= meUNDO_CONTINUE;
        else
            cbp->undoContFlag = undoContFlag;
    }
    else
    {
        /* ditch all undos to get more memory back and make ME more stable */
        meUndoRemove(cbp);
        meModeClear(cbp->mode,MDUNDO);
    }
    return nn;
}


void
meUndoAddInsChar(int len)
{
    meWindow *cwp=frameCur->windowCur;
    meBuffer *cbp=cwp->buffer;
    if(meModeTest(cbp->mode,MDUNDO))
    {
        meUByte type=meUNDO_SINGLE|meUNDO_INSERT ;
        meUndoNode *nn ;

        if(cbp->undoContFlag == undoContFlag)
            type |= meUNDO_CONTINUE ;

        if(((nn = cbp->undoHead) != NULL) && (nn->type == type) &&
           (nn->udata.dotp == cwp->dotLineNo) &&
           (nn->doto+len == cwp->dotOffset))
        {
            nn->doto += len ;
            nn->count += len ;
            cbp->undoContFlag = undoContFlag ;
        }
        else if((nn = meUndoCreateNode(sizeof(meUndoNode))) != NULL)
        {
            nn->type |= meUNDO_SINGLE|meUNDO_INSERT ;
            nn->count = len ;
        }
    }
}

#if MEOPT_UNICODE
/* a single char delete node only holds whole chars */
#define meUndoIsOneChar(nn) (meUtf8Prev((nn)->str,(nn)->count) == 0)
#else
#define meUndoIsOneChar(nn) ((nn)->count == 1)
#endif

/* len is the length of the char at dot, a delete node's string grows in 16 byte steps */
void
meUndoAddDelChar(int len)
{
    meWindow *cwp=frameCur->windowCur;
    meBuffer *cbp=cwp->buffer;
    if(meModeTest(cbp->mode,MDUNDO))
    {
        meUByte type=meUNDO_SINGLE|meUNDO_DELETE;
        meUndoNode *nn;
        meUByte   *ss;
#if MEOPT_UNICODE
        meUInt uc;
#endif

        ss = cwp->dotLine->text+cwp->dotOffset;
        if(*ss == '\0')
        {
            if((cwp->dotLineNo == cbp->lineCount-1) && (cwp->dotOffset))
                /* This is trying to just remove the end line when the line
                 * before is not empty, this will fail so don't store it
                 */
                return;
            if(cwp->dotLine->flag & meUNDO_EOL_FLAGS)
            {
                /* the line's ending is kept by a multi char node */
                meUndoAddDelChars(1);
                return;
            }
            ss = (meUByte *) "\n";
            len = 1;
        }
#if MEOPT_UNICODE
        else if((len == 1) ? (*ss & 0x80):(meUtf8Decode(ss,len,&uc) != len))
        {
            /* part of a char, the chars in a single node are found by decoding */
            meUndoAddDelChars(len);
            return;
        }
#endif
        if(cbp->undoContFlag == undoContFlag)
            type |= meUNDO_CONTINUE;

        nn = cbp->undoHead;
        if((nn != NULL) && ((nn->type & ~meUNDO_FORWARD) == type) &&
           (nn->udata.dotp == cwp->dotLineNo))
        {
            if(!(nn->type & meUNDO_FORWARD) && (nn->doto == cwp->dotOffset))
                ;
            else if(((nn->doto-len) == cwp->dotOffset) &&
                    (((nn->type & meUNDO_FORWARD) != 0) || meUndoIsOneChar(nn)))
            {
                nn->doto -= len;
                nn->type |= meUNDO_FORWARD;
            }
            else
                goto meUndoAddDelCharNew;
            if(((nn->count+len-1) & ~0x0f) != ((nn->count-1) & ~0x0f))
            {
                meUndoNode *tn;
                if((tn = meRealloc(nn,sizeof(meUndoNode)+((nn->count+len-1) & ~0x0f)+18)) == NULL)
                    return;
                nn = tn;
                cbp->undoHead = nn ;
            }
            memcpy(nn->str+nn->count,ss,len);
            nn->count += len;
            nn->str[nn->count] = '\0' ;
            cbp->undoContFlag = undoContFlag ;
        }
        else
        {
meUndoAddDelCharNew:
            if((nn = meUndoCreateNode(sizeof(meUndoNode)+18)) != NULL)
            {
                nn->type |= meUNDO_DELETE|meUNDO_SINGLE ;
                nn->count = len ;
                memcpy(nn->str,ss,len);
                nn->str[len] = '\0' ;
            }
        }
    }
}

void
meUndoAddRepChar(void)
{
    meWindow *cwp=frameCur->windowCur;
    meBuffer *cbp=cwp->buffer;
    if(meModeTest(cbp->mode,MDUNDO))
    {
        /* a replace from the next command at the next byte joins the node, as typing does; one
         * within the same command has meUNDO_CONTINUE so is a node of its own, undone with it */
        meUByte type=meUNDO_SINGLE|meUNDO_INSERT|meUNDO_DELETE ;
        meUndoNode *nn;
        meUByte   cc;

        if(cbp->undoContFlag == undoContFlag)
            type |= meUNDO_CONTINUE;
        if((cc = cwp->dotLine->text[cwp->dotOffset]) == '\0')
            cc = meCHAR_NL;
        nn = cbp->undoHead;
        if((nn != NULL) && (nn->type == type) &&
           (nn->udata.dotp == cwp->dotLineNo) &&
           (nn->doto == cwp->dotOffset))
        {
            if(!(nn->count & 0x0f))
            {
                meUndoNode *tn;
                if((tn = meRealloc(nn,sizeof(meUndoNode)+nn->count+18)) == NULL)
                    return;
                nn = tn;
                cbp->undoHead = nn;
            }
            nn->str[nn->count++] = cc;
            nn->str[nn->count]   = '\0';
            cbp->undoContFlag = undoContFlag;
        }
        else if((nn = meUndoCreateNode(sizeof(meUndoNode)+18)) != NULL)
        {
            nn->type |= meUNDO_DELETE|meUNDO_SINGLE|meUNDO_INSERT ;
            nn->count = 1 ;
            nn->str[0] = cc ;
            nn->str[1] = '\0' ;
        }
        else
            return;
        nn->doto++ ;
    }
}

void
meUndoAddInsChars(meInt numChars)
{
    meUndoNode *nn ;

    if(meModeTest(frameCur->windowCur->buffer->mode,MDUNDO) &&
       ((nn = meUndoCreateNode(sizeof(meUndoNode))) != NULL))
    {
        nn->type |= meUNDO_INSERT ;
        nn->count = numChars ;
    }
}

/* A line's ending as a state char of a delete node, never 0 */
#define meUndoEolState(lp) \
    (0x40 | (((lp)->flag & meLINE_EOLCR) ? 1:0) | (((lp)->flag & meLINE_EOLLF) ? 2:0) | (((lp)->flag & meLINE_NOEOL) ? 4:0))

/* Returns the number of line ends in the numChars from dot and sets *flags to their states or'ed */
static int
meUndoCountEols(meWindow *cwp, meInt numChars, meLineFlag *flags)
{
    meLine *ll=cwp->dotLine, *bl=cwp->buffer->baseLine;
    int len, nn=0;

    *flags = 0;
    if((len = meLineGetLength(ll) - cwp->dotOffset) < numChars)
    {
        for(;;)
        {
            numChars -= len+1;
            *flags |= ll->flag & meUNDO_EOL_FLAGS;
            nn++;
            ll = meLineGetNext(ll);
            if(((numChars == 0) && (ll == bl)) || (numChars <= (len=meLineGetLength(ll))))
                break;
        }
    }
    return nn;
}

/* After the text ss of a delete node has been inserted before dot, gives the lines it ended back
 * their endings; a state more than the text's '\n's is the ending of dot's line */
static void
meUndoRestoreEols(meWindow *cwp, meUByte *ss)
{
    meLine *lp=cwp->dotLine, *bl=cwp->buffer->baseLine;
    meUByte *ee=ss, cc;

    while((cc=*ee++) != '\0')
        if(cc == meCHAR_NL)
            lp = meLineGetPrev(lp);
    for(; ((cc=*ee) != '\0') && (lp != bl) ; ee++, lp=meLineGetNext(lp))
    {
        lp->flag &= ~meUNDO_EOL_FLAGS;
        if(cc & 1)
            lp->flag |= meLINE_EOLCR;
        if(cc & 2)
            lp->flag |= meLINE_EOLLF;
        if(cc & 4)
            lp->flag |= meLINE_NOEOL;
    }
}

void
meUndoAddDelChars(meInt numChars)
{
    meWindow *cwp=frameCur->windowCur;
    meBuffer *cbp=cwp->buffer;
    meUndoNode *nn ;
    meLineFlag eolFlags;
    int eols;

    if(numChars < 0)
        numChars = 0 ;
    /* the line ends are only kept when one is not the buffer style */
    if(!meModeTest(cbp->mode,MDUNDO))
        return;
    if(((eols = meUndoCountEols(cwp,numChars,&eolFlags)) > 0) && !eolFlags)
        eols = 0;
    if((nn = meUndoCreateNode(sizeof(meUndoNode)+numChars+((eols) ? eols+1:0))) != NULL)
    {
        meLine   *ll = cwp->dotLine ;
        int     len ;
        meUByte  *dd=nn->str, *ss=ll->text+cwp->dotOffset, *ee ;

        nn->type |= meUNDO_DELETE ;
        if(eols)
            nn->type |= meUNDO_EOLS;
        nn->count = 0;
        /* the states follow the text's '\0', which is at most numChars in */
        ee = nn->str+numChars+1;
        if((len = meLineGetLength(ll) - cwp->dotOffset) < numChars)
        {
            for(;;)
            {
                numChars-=len+1 ;
                for(; len ; len--)
                    *dd++ = *ss++ ;
                if(eols)
                    *ee++ = meUndoEolState(ll);
                ll = meLineGetNext(ll) ;
                ss = ll->text ;
                /* A bit of a bodge here to cope with the bogus last line
                 * If the last but 1 line's '\n' is about to be removed
                 * don't actually store the '\n' as this is automatically
                 * added by the system
                 */
                if((numChars == 0) && (ll == cbp->baseLine) && meLineGetLength(meLineGetPrev(ll)))
                    break ;
                *dd++ = meCHAR_NL ;
                /* an empty last line is only added back by its '\n' */
                if(numChars == 0)
                    break ;
                if(numChars <= (len=meLineGetLength(ll)))
                    break ;
            }
        }
        for(; numChars ; numChars--)
            *dd++ = *ss++ ;
        *dd = '\0' ;
        if(eols)
        {
            /* the text can be a byte short (the last line's end), move the states to follow it */
            *ee = '\0';
            ee -= eols;
            if(ee != dd+1)
                memmove(dd+1,ee,eols+1);
        }
    }
}

void
meUndoAddReplaceBgn(meLine *elinep, meUShort elineo)
{
    meWindow *cwp=frameCur->windowCur;
    meBuffer *cbp=cwp->buffer;
    if(meModeTest(cbp->mode,MDUNDO))
    {
        meUndoNode *nn ;
        meLineFlag eolFlags=0;
        int     len, eols=0 ;

        if(elinep == cbp->baseLine)
        {
            elinep = meLineGetPrev(elinep) ;
            elineo = meLineGetLength(elinep) ;
        }
        if(elinep == cwp->dotLine)
            len = elineo - cwp->dotOffset ;
        else
        {
            meLine *ll = cwp->dotLine ;
            len = meLineGetLength(ll) - cwp->dotOffset + elineo+1 ;
            eolFlags = ll->flag;
            eols = 1;
            while((ll = meLineGetNext(ll)) != elinep)
            {
                len += meLineGetLength(ll)+1 ;
                eolFlags |= ll->flag;
                eols++;
            }
            /* the line ends are only kept when one is not the buffer style */
            if(!(eolFlags & meUNDO_EOL_FLAGS))
                eols = 0;
        }
        if((nn = meUndoCreateNode(sizeof(meUndoNode)+len+((eols) ? eols+1:0))) != NULL)
        {
            meLine  *ll = cwp->dotLine ;
            meUByte *dd=nn->str, *ss=ll->text+cwp->dotOffset ;

            nn->type |= meUNDO_DELETE ;
            if(eols)
            {
                meLine *el=ll;
                meUByte *ee=nn->str+len+1;
                nn->type |= meUNDO_EOLS;
                do
                    *ee++ = meUndoEolState(el);
                while((el = meLineGetNext(el)) != elinep);
                *ee = '\0';
            }
            /* This should be zero because added on the end call. */
            nn->count = 0;
            if(elinep == cwp->dotLine)
            {
                for(; len ; len--)
                    *dd++ = *ss++ ;
            }
            else
            {
                len = meLineGetLength(ll) - cwp->dotOffset ;
                for(; len ; len--)
                    *dd++ = *ss++ ;
                *dd++ = meCHAR_NL ;
                while((ll = meLineGetNext(ll)) != elinep)
                {
                    len = meLineGetLength(ll) ;
                    ss = ll->text ;
                    for(; len ; len--)
                        *dd++ = *ss++ ;
                    *dd++ = meCHAR_NL ;
                }
                ss = ll->text ;
                for(; elineo ; elineo--)
                    *dd++ = *ss++ ;
            }
            *dd = '\0' ;
        }
    }
}

void
meUndoAddReplaceEnd(meInt numChars)
{
    meWindow *cwp=frameCur->windowCur;
    meBuffer *cbp=cwp->buffer;
    if(meModeTest(cbp->mode,MDUNDO))
    {
        cbp->undoHead->udata.dotp = cwp->dotLineNo ;
        cbp->undoHead->doto  = cwp->dotOffset ;
        cbp->undoHead->count = numChars ;
        cbp->undoHead->type |= meUNDO_INSERT ;
    }
}

void
meUndoAddReplace(meUByte *dstr, meInt count)
{
    meWindow *cwp=frameCur->windowCur;
    meBuffer *cbp=cwp->buffer;
    if(meModeTest(cbp->mode,MDUNDO))
    {
        meUndoCoord *co ;
        meUndoNode    *nn ;
        int        doto, contFlag ;

        contFlag = (cbp->undoContFlag == undoContFlag) ;

        nn = cbp->undoHead ;
        if((nn == NULL) || !meUndoIsReplace(nn) ||
           (nn->count != count) || (nn->doto == 0xffff) ||
           meStrcmp(nn->str,dstr))
        {
            meUByte *dd ;
            if((nn = meUndoCreateNode(sizeof(meUndoNode)+meStrlen(dstr))) == NULL)
                return ;
            nn->type |= meUNDO_DELETE|meUNDO_INSERT|meUNDO_REPLACE ;
            nn->udata.pos = NULL ;
            nn->doto = 0 ;
            nn->count = count ;
            dd = nn->str ;
            while((*dd++ = *dstr++))
                ;
        }
        /* replace is the same as last time */
        if(!(nn->doto & 0x0f))
        {
            meUndoCoord *tp;
            if((tp = meRealloc(nn->udata.pos,sizeof(meUndoCoord)*(nn->doto+16))) == NULL)
                return;
            nn->udata.pos = tp;
        }
        co = nn->udata.pos ;
        doto = cwp->dotOffset ;
        if(contFlag)
            /* sub 1 of so 0 becomes less than 0 */
            doto = -1 - doto ;
        co[nn->doto][0]   = cwp->dotLineNo ;
        co[nn->doto++][1] = doto ;
        cbp->undoContFlag = undoContFlag ;
    }
}

meInt *
meUndoAddLineSort(meInt lineCount)
{
    meUndoNode *nn;
    meInt *lineSort;
    if(meModeTest(frameCur->windowCur->buffer->mode,MDUNDO) &&
       ((lineSort = meMalloc((lineCount + 1) * sizeof(meInt))) != NULL) &&
       ((nn = meUndoCreateNode(sizeof(meUndoNode))) != NULL))
    {
        nn->type |= meUNDO_SPECIAL|meUNDO_LINE_SORT;
        nn->count = lineCount;
        nn->udata.lineSort = lineSort;
        *lineSort++ = frameCur->windowCur->dotLineNo;
        return lineSort ;
    }
    return NULL ;
}

#if MEOPT_NARROW
/* NOTE: Narrow undos are only added if there is already an
 * undo. This is so the state of the buffer will be correct
 * for the previous undo
 */
void
meUndoAddNarrow(meInt sln, meInt name,
                meInt markupCmd, meLine *firstLine)
{
    meUndoNarrow *nn ;
    int ll ;
    
    ll = (markupCmd > 0) ? meLineGetLength(firstLine):0 ;
    if((frameCur->windowCur->buffer->undoHead != NULL) &&
       ((nn = (meUndoNarrow *) meUndoCreateNode(sizeof(meUndoNarrow)+ll)) != NULL))
    {
        nn->type |= meUNDO_SPECIAL|meUNDO_NARROW|meUNDO_NARROW_ADD ;
        nn->udata.dotp = sln ;
        nn->count = 0 ;
        nn->name = name ;
        nn->markupCmd = markupCmd ;
        if(markupCmd > 0)
            meStrcpy(nn->str,meLineGetText(firstLine)) ;
    }
}
void
meUndoAddUnnarrow(meInt sln, meInt eln, meInt name, meScheme scheme, 
                  meInt markupCmd, meLine *markupLine)
{
    meUndoNarrow *nn ;
    int ll ;
    
    ll = (markupLine) ? meLineGetLength(markupLine):0 ;
    if((frameCur->windowCur->buffer->undoHead != NULL) &&
       ((nn = (meUndoNarrow*) meUndoCreateNode(sizeof(meUndoNarrow)+ll)) != NULL))
    {
        nn->type |= meUNDO_SPECIAL|meUNDO_NARROW ;
        nn->udata.dotp = sln ;
        nn->count = eln ;
        nn->name = name ;
        nn->scheme = scheme ;
        nn->markupCmd = markupCmd ;
        if(markupLine != NULL)
        {
            nn->markupFlag = meTRUE ;
            meStrcpy(nn->str,meLineGetText(markupLine)) ;
        }
        else
            nn->markupFlag = meFALSE ;
    }
}
#endif

void
meUndoRemove(meBuffer *bp)
{
    meUndoNode *nn ;

    while((nn=bp->undoHead) != NULL)
    {
        bp->undoHead = nn->next ;
        if(meUndoIsReplace(nn) || meUndoIsLineSort(nn))
            free(nn->udata.pos) ;
        free(nn) ;
    }
}

int
meUndo(int f, int n)
{
    meWindow *cwp=frameCur->windowCur;
    meBuffer *cbp=cwp->buffer;
    if(n < -1)
    {
#ifndef NDEBUG
        if(n == -4)
        {
            meUndoNode *nn ;
            FILE *undoFp=NULL ;
            if(undoFp == NULL)
                undoFp = fopen("undo.log","w+") ;
            fprintf(undoFp,"[Undo stack for %s]\n",cbp->name) ;
            nn=cbp->undoHead ;
            while(nn != NULL)
            {
                if(meUndoIsLineSort(nn))
                {
                    fprintf(undoFp,"Undo 0x%02x %p %d %d:",nn->type,nn->next,nn->udata.lineSort[0],nn->count) ;
                    for(n=0 ; n<nn->count ; n++)
                        fprintf(undoFp," %d",nn->udata.lineSort[n+1]) ;
                    fprintf(undoFp,"\n") ;
                }
#if MEOPT_NARROW
                else if(meUndoIsNarrow(nn))
                {
                    meUndoNarrow *nun = (meUndoNarrow *) nn ;
                    fprintf(undoFp,"Undo 0x%02x %p Nrrw %x %d %d %d [%s]\n",nun->type,nun->next,
                            nun->name,nun->count,nun->udata.dotp,nun->markupCmd,nun->str) ;
                }
#endif
                else
                {
                    fprintf(undoFp,"Undo 0x%02x %p %d (%d,%d) [%s]\n",nn->type,nn->next,nn->count,
                            nn->udata.dotp,nn->doto,nn->str) ;
                    if(meUndoIsReplace(nn))
                    {
                        for(n=0 ; n<nn->doto ; n++)
                            fprintf(undoFp,"(%d,%d) ",nn->udata.pos[n][0],nn->udata.pos[n][1]) ;
                        fprintf(undoFp,"\n") ;
                    }
                }
                nn = nn->next ;
            }
            fprintf(undoFp,"---------------\n") ;
            fflush(undoFp) ;
        }
        else
#endif
            undoContFlag++ ;
    }
    else if(!meModeTest(cbp->mode,MDUNDO))
        return ctrlg(meFALSE,1) ;
    else if(n < 0)
        meUndoRemove(cbp) ;
    else
    {
        static meUndoNode *cun ;
        static meInt  ccount ;
        static meUShort cdoto ;

        if((lastflag != meCFUNDO) && ((cun = cbp->undoHead) != NULL))
        {
            cdoto = cun->doto ;
            ccount = cun->count ;
        }
        for(;;)
        {
            meInt count, cont ;
            if((cun == NULL) || ((n <= 0) && !meModeTest(cbp->mode,MDEDIT)))
                break ;
            if(bufferSetEdit() <= 0)               /* Check we can change the buffer */
                return meABORT;
            cont=0 ;
            if(cun->type & meUNDO_SPECIAL)
            {
                if(meUndoIsSetEdit(cun))
                {
                    if(!(cun->type & meUNDO_UNSET_EDIT))
                    {
                        autowriteremove(cbp);
                        meModeClear(cbp->mode,MDEDIT);
                        frameAddModeToWindows(WFMODE);  /* update ALL mode lines */
                    }
                }
                else if(meUndoIsLineSort(cun))
                {
                    meLine *ln, *eln, **list;
                    meInt *lineSort, *undoInfo, dddd;
                    lineSort = cun->udata.lineSort;
                    meWindowGotoLine(cwp,(*lineSort++) + 1);
                    if((list = meMalloc(cun->count * sizeof(meLine *))) == NULL)
                        return meABORT;
                    undoInfo = meUndoAddLineSort(cun->count);
                    eln = cwp->dotLine;
                    ln = meLineGetPrev(eln);
                    for(count=0 ; count<cun->count ; eln=meLineGetNext(eln),count++)
                    {
                        list[*lineSort++] = eln;
                        eln->prev = (meLine *) mePtrFromInt(count);
                    }
                    for(count=0 ; count<cun->count ; ln=meLineGetNext(ln),count++)
                    {
                        if(undoInfo != NULL)
                        {
                            dddd = meIntFromPtr(list[count]->prev);
                            *undoInfo++ = dddd;
                        }
                        ln->next = list[count];
                        list[count]->prev = ln;
                    }
                    ln->next = eln;
                    eln->prev = ln;
                    cwp->dotLine = list[0];
                    lineSetChanged(WFMOVEL|WFMAIN);
                    meFree(list);
                }
#if MEOPT_NARROW
                else if(meUndoIsNarrow(cun))
                {
                    meUndoNarrow *nun = (meUndoNarrow *) cun;
                    meInt name;
                    name = nun->name;
                    meWindowGotoLine(cwp,nun->udata.dotp+1);
                    if(nun->type & meUNDO_NARROW_ADD)
                    {
                        meNarrow *nrrw;
                        nrrw = cbp->narrow;
                        while(nrrw->name != name)
                            nrrw = nrrw->next;
                        cbp->dotLine = cwp->dotLine;
                        cbp->dotLineNo = cwp->dotLineNo;
                        cbp->dotOffset = 0;
                        meBufferRemoveNarrow(cbp,nrrw,
                                             (nun->markupCmd > 0) ? nun->str:NULL,1);
                    }
                    else
                    {
                        meLine *slp ;
                        slp = cwp->dotLine;
                        meWindowGotoLine(cwp,ccount+1);
                        meBufferCreateNarrow(cbp,slp,cwp->dotLine,
                                             nun->udata.dotp,ccount,name,nun->scheme,
                                             (nun->markupFlag) ? nun->str:NULL,nun->markupCmd,1);
                    }
                }
#endif
                if(cun->type & meUNDO_CONTINUE)
                    cont=1;
                goto meUndoNext;
            }
            if(cun->type & meUNDO_REPLACE)
            {
                meWindowGotoLine(cwp,cun->udata.pos[cdoto-1][0]+1);
                count = cun->udata.pos[cdoto-1][1];
                if(count < 0)
                {
                    cont = 1;
                    count = -1 - count;
                }
                cwp->dotOffset = (meUShort) count;
            }
            else
            {
                if(cun->type & meUNDO_CONTINUE)
                    cont = 1;
                meWindowGotoLine(cwp,cun->udata.dotp+1);
                cwp->dotOffset = cdoto;
            }
            if(cun->type & meUNDO_SINGLE)
            {
                count = 1;
#if MEOPT_UNICODE
                /* undo a whole char, a multi-byte replace is a node per byte */
                if(!(cun->type & meUNDO_INSERT))
                    count = ccount - meUtf8Prev(cun->str,ccount);
                else if(!(cun->type & meUNDO_DELETE) && cdoto)
                {
                    count = cdoto - meUtf8Prev(cwp->dotLine->text,cdoto);
                    if(count > ccount)
                        count = ccount;
                }
#endif
                ccount -= count;
            }
            else
                count = ccount;
            if(cun->type & meUNDO_INSERT)
            {
                meWindowBackwardBytes(cwp,count);
                if((count == 1) || (cun->type & meUNDO_SINGLE))
                    meUndoAddDelChar(count);
                else
                    meUndoAddDelChars(count);
                mldelete(count,NULL);
            }
            if(cun->type & meUNDO_DELETE)
            {
                /* When dealing with long lines this loop becomes infinitly
                 * long because of the number of times that the line is
                 * re-allocated - pre-allocate the line length first. In order
                 * to reduce the processing overhead then we find the longest
                 * strings and then add them back in in one go, this ensures
                 * that we only ever re-allocate the line once. 
                 * Jon - 99/12/12.
                 */
                meUByte *ss, cc;
                ss = cun->str;
                /* Deal with a single character undo */
                if(cun->type & meUNDO_SINGLE)
                {
                    ss += ccount ;
                    if((cc = *ss) == meCHAR_NL)
                        lineInsertNewline(meBUFINSFLAG_UNDOCALL);
                    else if(count > 1)
                        lineInsertString(count,ss);
                    else if (cc != '\0')
                        lineInsertChar(1, cc);
                    meUndoAddInsChar(count) ;
                }
                else
                {
                    /* Deal with a multiple character undo. */
                    count = bufferInsertText(ss,meBUFINSFLAG_UNDOCALL) ;
                    if(cun->type & meUNDO_EOLS)
                        meUndoRestoreEols(cwp,ss);
                    if(count > 0)
                        meUndoAddInsChars(count) ;
                }
            }
            if((cun->type & meUNDO_SINGLE) && (ccount > 0))
            {
                if(cun->type & meUNDO_FORWARD)
                    cdoto += count ;
                else if(cun->type & meUNDO_INSERT)
                    cdoto -= count ;
            }
            else if(cun->type & meUNDO_REPLACE)
            {
                cdoto-- ;
                if(!cdoto)
                    goto meUndoNext ;
            }
            else
            {
meUndoNext:
                if((cun = cun->next) != NULL)
                {
                    cdoto = cun->doto ;
                    ccount = cun->count ;
                }
            }
            if(!cont && (--n == 0))
                break ;
        }
        thisflag = meCFUNDO ;
    }
    return meTRUE ;
}
#endif
