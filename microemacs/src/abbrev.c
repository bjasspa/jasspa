/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * abbrev.c - Abbrevation expansion routines
 *
 * Copyright (c) 1995-2001 Steven Phillips
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
 * Created:     12/06/1995
 * Synopsis:    Abbrevation expansion routines
 * Authors:     Steven Phillips
 * Description:
 *        Handle character expansion. The expansion sequences are held in 
 *        abbreviation files (.eaf). The file is loaded into a buffer and 
 *        subsequent expansion requests use the buffer for the matching and
 *        expanding text.
 *
 *        The .eaf file is organsied with the competion match string 
 *        followed by white space, followed by the replacement text in quotes.
 *        Each entry occupies one line. The replacement text is executed
 *        using the command execute-string
 *
 */

#include "emain.h"
#include "efunc.h"

#define __ABBREVC

#if MEOPT_ABBREV


static int
setAbbrev(int f, int n, meAbbrev **abrevPtr)
{
    register meAbbrev *abrev;
    register int status;        /* return status of name query */
    meUByte buf[meBUF_SIZE_MAX];          /* name of file to execute */
    
    if(n == 0)
    {
        if((abrev=*abrevPtr) != NULL)
        {
            abrev->loaded = 0 ;
            meLineLoopFree(&(abrev->hlp),0) ;
        }
        return meTRUE ;
    }
        
    if((status=meGetString((meUByte *)"Abbrev file",MLFILECASE,0,buf,meBUF_SIZE_MAX)) <= 0)
        return status ;
    
    if(buf[0] == '\0')
        abrev = NULL ;
    else
    {
        abrev = aheadp ;
        while(abrev != NULL)
        {
            if(!fnamecmp(buf,abrev->fname))
                break ;
            abrev = abrev->next ;
        }
        if((abrev == NULL) &&
           ((abrev = meMalloc(sizeof(meAbbrev)+meStrlen((char *) buf))) != NULL))
        {
            meStrcpy(abrev->fname,buf) ;
            abrev->loaded = 0 ;
            abrev->next = aheadp ;
            aheadp = abrev ;
            abrev->hlp.next = &(abrev->hlp) ;
            abrev->hlp.prev = &(abrev->hlp) ;
        }
    }
    *abrevPtr = abrev ;
    
    return meTRUE ;
}

int
bufferAbbrev(int f, int n)
{
    return setAbbrev(f,n,&(frameCur->windowCur->buffer->abbrevFile)) ;
}
int
globalAbbrev(int f, int n)
{
    return setAbbrev(f,n,&globalAbbrevFile) ;
}


static int
doExpandAbbrev(meUByte *abName, int abLen, meAbbrev *abrev)
{
    register meLine *lp, *hlp ;
    
    hlp = &abrev->hlp ;
    if(!abrev->loaded)
    {
        meUByte fname[meBUF_SIZE_MAX] ;
        
        if(!fileLookup(abrev->fname,extAbrevCnt,extAbrevLst,meFL_CHECKPATH|meFL_USESRCHPATH,fname) ||
           (ffReadFile(&meior,fname,meRWFLAG_READ|meRWFLAG_SILENT,NULL,hlp,0,0,0) == meABORT))
            return mlwrite(MWABORT,(meUByte *)"[Failed to abbrev file %s]",abrev->fname);
        abrev->loaded = 1 ;
    }
    lp = meLineGetNext(hlp) ;
    while(lp != hlp)
    {
        if((lp->length > abLen) &&
           !strncmp((char *) lp->text,(char *) abName,abLen) &&
           (lp->text[abLen] == ' '))
        {
            meUByte buf[meTOKENBUF_SIZE_MAX] ;
            frameCur->windowCur->dotOffset -= abLen ;
            ldelete(abLen,2) ;
            /* grab token */
            token(lp->text+abLen,buf);
            return stringExec(meFALSE,1,buf+1) ;
        }
        lp = meLineGetNext(lp) ;
    }
    return meFALSE ;
}

int
expandAbbrev(int f, int n)
{
    register meWindow *cwp;
    register int len, ii ;
    meUByte buf[meBUF_SIZE_MAX] ;
    
    if(bufferSetEdit() <= 0)               /* Check we can change the buffer */
        return meABORT ;
    cwp = frameCur->windowCur;
    ii = cwp->dotOffset ;
    if(((cwp->buffer->abbrevFile != NULL) || (globalAbbrevFile != NULL)) &&
       (--ii >= 0) && isWord(cwp->dotLine->text[ii]))
    {
        len = 1 ;
        while((--ii >= 0) && isWord(cwp->dotLine->text[ii]))
            len++ ;
        strncpy((char *) buf,(char *) &(cwp->dotLine->text[++ii]),len) ;
        if(((cwp->buffer->abbrevFile != NULL) && 
            ((ii=doExpandAbbrev(buf,len,cwp->buffer->abbrevFile)) != meFALSE)) ||
           ((globalAbbrevFile != NULL) && 
            ((ii=doExpandAbbrev(buf,len,globalAbbrevFile)) != meFALSE)))
            return ii ;
    }
    /* We used to insert a space if the expansion was not defined
     * there seems to be no good reason to do this. Instead bitch
     * if we are not in a macro and return a false condition. The
     * error is not serious enough to abort */
    mlwrite (MWCLEXEC,(meUByte *)"[No abbrev expansion defined]"); 
    return meFALSE;
    
}
#endif
