/* -*- C -*- ****************************************************************
 *
 *  System        : MicroEmacs Jasspa Distribution
 *  Module        : abbrev.c
 *  Synopsis      : Abbrevation expansion routines
 *  Created By    : Steven Phillips
 *  Created       : 12/06/1995
 *  Last Modified : <010305.0749>
 *
 *  Description
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
 *  Notes
 * 
 ****************************************************************************
 * 
 * Copyright (c) 1995-2000 Steven Phillips    
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

#include "emain.h"
#include "efunc.h"

#define __ABREVC

#if ABREV


static int
setAbbrev(int f, int n, meABREV **abrevPtr)
{
    register meABREV *abrev ;
    register int      status;		/* return status of name query */
    uint8             buf[MAXBUF]; 	/* name of file to execute */
    
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
           ((abrev = meMalloc(sizeof(meABREV)+meStrlen((char *) buf))) != NULL))
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
doExpandAbbrev(uint8 *abName, int abLen, meABREV *abrev)
{
    register LINE    *lp, *hlp ;
    
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
