/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * abbrev.c - Abbrevation expansion routines
 *
 * Copyright (c) 1995-2001 Steven Phillips
 * Copyright (C) 2002 JASSPA (www.jasspa.com)
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

#if ABBREV


static int
setAbbrev(int f, int n, meABBREV **abrevPtr)
{
    register meABBREV *abrev ;
    register int status;        /* return status of name query */
    uint8 buf[MAXBUF];          /* name of file to execute */
    
    if(n == 0)
    {
        if((abrev=*abrevPtr) != NULL)
        {
            abrev->loaded = 0 ;
            freeLineLoop(&(abrev->hlp),0) ;
        }
        return TRUE ;
    }
        
    if((status=meGetString((uint8 *)"Abbrev file",MLFILECASE,0,buf,MAXBUF)) != TRUE)
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
           ((abrev = meMalloc(sizeof(meABBREV)+meStrlen((char *) buf))) != NULL))
        {
            meStrcpy(abrev->fname,buf) ;
            abrev->loaded = 0 ;
            abrev->next = aheadp ;
            aheadp = abrev ;
            abrev->hlp.l_fp = &(abrev->hlp) ;
            abrev->hlp.l_bp = &(abrev->hlp) ;
        }
    }
    *abrevPtr = abrev ;
    
    return TRUE ;
}

int
bufferAbbrev(int f, int n)
{
    return setAbbrev(f,n,&(curbp->abrevFile)) ;
}
int
globalAbbrev(int f, int n)
{
    return setAbbrev(f,n,&globalAbrev) ;
}


static int
doExpandAbbrev(uint8 *abName, int abLen, meABBREV *abrev)
{
    register LINE *lp, *hlp ;
    
    hlp = &abrev->hlp ;
    if(!abrev->loaded)
    {
        uint8 fname[FILEBUF] ;
        
        if(!fileLookup(abrev->fname,(uint8 *)".eaf",meFL_CHECKDOT|meFL_USESRCHPATH,fname) ||
           (ffReadFile(fname,meRWFLAG_SILENT,NULL,hlp) == ABORT))
            return mlwrite(MWABORT,(uint8 *)"[Failed to abbrev file %s]",abrev->fname);
        abrev->loaded = 1 ;
    }
    lp = lforw(hlp) ;
    while(lp != hlp)
    {
        if((lp->l_used > abLen) &&
           !strncmp((char *) lp->l_text,(char *) abName,abLen) &&
           (lp->l_text[abLen] == ' '))
        {
            uint8 buf[TOKENBUF] ;
            curwp->w_doto -= abLen ;
            ldelete(abLen,2) ;
            /* grab token */
            token(lp->l_text+abLen,buf);
            return stringExec(FALSE,1,buf+1) ;
        }
        lp = lforw(lp) ;
    }
    return FALSE ;
}

int
expandAbbrev(int f, int n)
{
    register int len, ii ;
    uint8 buf[MAXBUF] ;
    
    if(bchange() != TRUE)               /* Check we can change the buffer */
        return ABORT ;
    ii = curwp->w_doto ;
    if(((curbp->abrevFile != NULL) || (globalAbrev != NULL)) &&
       (--ii >= 0) && (!isSpace(curwp->w_dotp->l_text[ii])))
    {
        len = 1 ;
        while((--ii >= 0) && !isSpace(curwp->w_dotp->l_text[ii]))
            len++ ;
        strncpy((char *) buf,(char *) &(curwp->w_dotp->l_text[++ii]),len) ;
        if(((curbp->abrevFile != NULL) && ((ii=doExpandAbbrev(buf,len,curbp->abrevFile)) != FALSE)) ||
           ((globalAbrev != NULL)      && ((ii=doExpandAbbrev(buf,len,globalAbrev)) != FALSE)))
            return ii ;
    }
    /* We used to insert a space if the expansion was not defined
     * there seems to be no good reason to do this. Instead bitch
     * if we are not in a macro and return a false condition. The
     * error is not serious enough to abort */
    mlwrite (MWCLEXEC,(uint8 *)"[No abbrev expansion defined]"); 
    return FALSE;
    
}
#endif
