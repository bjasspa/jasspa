/* -!- c -!- *****************************************************************
*
*       Title:          %M%
*
*       Synopsis:       Buffer operations
*
******************************************************************************
*
*       Filename:               %P%
*
*       Author:                 Danial Lawrence
*
*       Creation Date:          14/05/86 12:37          <001011.1802>
*
*       Modification date:      %G% : %U%
*
*       Current rev:            %I%
*
*       Special Comments:       
*
*       Contents Description:   
*
* Buffer management.
* Some of the functions are internal, and some are actually attached to user
* keys. Like everyone else, they set hints for the display system.
*
*****************************************************************************
* 
* (C)opyright 1987 by Daniel M. Lawrence
* MicroEMACS 3.8 can be copied and distributed freely for any
* non-commercial purposes. MicroEMACS 3.8 can only be incorporated
* into commercial software with the permission of the current author.
* 
* Modifications to the original file by Jasspa. 
* 
* Copyright (C) 1988 - 1999, JASSPA 
* The MicroEmacs Jasspa distribution can be copied and distributed freely for
* any non-commercial purposes. The MicroEmacs Jasspa Distribution can only be
* incorporated into commercial software with the expressed permission of
* JASSPA.
* 
****************************************************************************/

/*---   Include defintions */

#define __BUFFERC                       /* Define filename */

/*---   Include files */

#include "emain.h"
#include "efunc.h"
#include "esearch.h"

/*---   Local variable definitions */

int
getBufferName(uint8 *prompt, int opt, int defH, uint8 *buf)
{
    /* Only setup the history to a specific buffer if we're
     * told to and we're not running a macro
     */
    if(defH && (clexec != TRUE))
    {
	register BUFFER *bp ;
	if(defH == 1)
	    bp = curbp ;
	else
	{
	    bp = replacebuffer(curbp) ;
	    defH = 1 ;
	}
	addHistory(MLBUFFER, bp->b_bname) ;
    }
    return mlreply(prompt,opt|MLBUFFERCASE,defH,buf,MAXBUF) ;
}

/* Check extent.
   Check the extent string for the filename extent. */
				
static int
checkExtent (uint8 *filename, int len, uint8 *sptr, Fintssi cmpFunc)
{
    int  extLen ;
    char cc ;
    
    if(sptr == NULL)
	return 0 ;              /* Exit - fail */
    filename += len ;
    /*---       Run through the list, checking the extents as we go. */
    for(;;)
    {
	
	while(*sptr == ' ')
	    sptr++ ;
	extLen = 0 ;
	while(((cc=sptr[extLen]) != ' ') && ((cc=sptr[extLen]) != '\0'))
	    extLen++ ;
	if((extLen <= len) &&
	   !cmpFunc((char *) sptr,(char *) filename-extLen,extLen))
	    return 1 ;
	if(cc == '\0')
	    return 0 ;
	sptr += extLen+1 ;
    }
}

/*
 * assignHooks.
 * Given a buffer and a hook name, determine the file
 * hooks associated with the file and assign.
 *
 * Note that the hookname (hooknm) is modified, ultimatly
 * returning the 'f' prefixed name.
 */

static uint8 defaultHookName[] = "fhook-default" ;
static uint8 binaryHookName[]  = "fhook-binary" ;
static uint8 triedDefault=0 ;
static uint8 triedBinary=0 ;

static void
assignHooks (BUFFER *bp, uint8 *hooknm)
{
    int fhook;                          /* File hook indentity */

    /* Get the identity of the hook */
    fhook = decode_fncname(hooknm,1);

    /* Get the file into memory if we have to */
    if((fhook == -1) &&
       ((hooknm != defaultHookName) || !triedDefault) &&
       ((hooknm != binaryHookName) || !triedBinary) &&
       (meStrlen(hooknm) > 6))
    {
	uint8 fn[FILEBUF], buff[MAXBUF] ;             /* Temporary buffer */

	buff[0] = 'h' ;
	buff[1] = 'k' ;
	meStrcpy(buff+2,hooknm+6) ;
	if(!fileLookup(buff,(uint8 *)".emf",meFL_CHECKDOT|meFL_USESRCHPATH,fn))
	{
	    if(hooknm == defaultHookName)
		triedDefault++ ;
	    else if(hooknm == binaryHookName)
		triedBinary++ ;
	    else
		mlwrite(MWABORT|MWWAIT,(uint8 *)"Failed to find file [%s]",buff);
	}
	else
	{
	    dofile(fn,0,1) ;
	    bp->fhook = decode_fncname(hooknm,1) ;
	}
    }
    bp->fhook = decode_fncname(hooknm,1) ;
    *hooknm   = 'b' ;
    bp->bhook = decode_fncname(hooknm,1) ;
    *hooknm   = 'd' ;
    bp->dhook = decode_fncname(hooknm,1) ;
    *hooknm   = 'e' ;
    bp->ehook = decode_fncname(hooknm,1) ;
    *hooknm   = 'f';
    return;
}

/*
 * findBufferContext - buffer context hook identification.
 *
 * Search for buffer context hook information this informs MicroEmacs
 * at the last knockings what buffer hook it should be using. This
 * over-rides the file extension buffer hook which is used as a
 * fall-back.
 *
 * The first line is searched for magic strings embedded in the text
 * performing a case insensitive search for a magic string as defined
 * by add-magic-hook. This is higher priority than the file extension, but
 * lower priority than than the explit mode string.
 *
 * Search for information on the first non-blank line of the form
 * "-*- name -*-" or "-*- mode: name -*-" in much the same way as
 * GNU Emacs. There are so many files in Linux with these attributes
 * we may as well use them.
 */
static void
setBufferContext(BUFFER *bp)
{
    register BUFFER *tbp ;
    uint32 w_flag;
    int32 topLineNo ;
    int ii ;
    
    /* First setup the global scheme - this can be missed by buffers loaded with -c */
    bp->scheme = globScheme;
    /* must do this to do the hook properly */
    if((tbp = curbp) != bp)
    {
	storeWindBSet(tbp,curwp) ;
        w_flag = curwp->w_flag ;
        topLineNo = curwp->topLineNo ;
        tbp->b_nwnd-- ;
	curbp = curwp->w_bufp = bp ;
        bp->b_nwnd++ ;
    }
    /* must do this to do the hook properly */
    restoreWindBSet(curwp,bp) ;
    isWordMask = bp->isWordMask ;
    
    /* Search the buffer for alternative hook information */
    if ((bp->intFlag & BIFFILE) && !meModeTest(bp->b_mode,MDBINRY) &&
        !meModeTest(bp->b_mode,MDDIR) && ((ii = fileHookCount) > 0))
    {
	uint8 *pp, cc ;
	LINE *lp ;
	int nn ;
	
	for(lp=curwp->w_dotp ; lp!=bp->b_linep ; lp = lforw(lp))
	{
	    for (pp = lp->l_text; (cc=*pp++) != '\0' ; )
		if (!(isSpace (cc)))
		    break ;
	    if(cc)
	    {
		curwp->w_dotp = lp ;
		while(--ii >= 0)
		{
		    if((nn=fileHookArg[ii]) != 0)
		    {
			int flags = ISCANNER_MAGIC|ISCANNER_QUIET ;
			if(nn < 0)
			    nn = -nn ;
			else
			    flags |= ISCANNER_EXACT ;
			if(iscanner(fileHookExt[ii],nn,flags,NULL) == TRUE)
			{
			    /* Must set the region flag to zero or it will
			     * be displayed
			     */
			    selhilight.flags = 0 ;
			    assignHooks(bp,fileHookFunc[ii]) ;
			    break ;
			}
		    }
		}
		restoreWindBSet(curwp,bp) ;
		break ;
	    }
	}
    }
    if(bp->fhook < 0)
    {
	uint8 *hooknm ;
	
	/* Do file hooks */
	if(meModeTest(bp->b_mode,MDBINRY))
	    hooknm = binaryHookName ;
	else
	{
	    uint8 *sp ;
	    int    ll ;
	    
	    sp = bp->b_bname ;
	    /* Find the length of the string to pass into check_extension.
	     * Check if its name has a <?> and/or a backup ~, if so reduce
	     * the length so '<?>' & '~'s not inc.
	     */
	    ll = meStrlen(sp) ;
	    if(bp->intFlag & BIFNAME)
	    {
		while(sp[--ll] != '<')
		    ;
	    }
	    if(sp[ll-1] == '~')
		ll-- ;
            if(ll)
            {
                uint8 cc = sp[ll-1] ;
                if(cc == '~')
                {
                    ll-- ;
                    if((ll > 2) && (sp[ll-1] == '~') && (sp[ll-2] == '.'))
                        ll -= 2 ;
                }
                else if(isDigit(cc))
                {
                    int ii=ll-2 ;
                    while(ii > 0)
                    {
                        cc = sp[ii--] ;
                        if(!isDigit(cc))
                        {
                            if((cc == '~') && (sp[ii] == '.'))
                                ll = ii ;
                            break ;
                        }
                    }
                }
            }
	    ii = fileHookCount ;
	    while(--ii >= 0)
		if((fileHookArg[ii] == 0) &&
		   checkExtent(sp,ll,fileHookExt[ii],
#ifdef _INSENSE_CASE
			       strnicmp
#else
			       strncmp
#endif
			       ))
	    {
		hooknm = fileHookFunc[ii] ;
		break ;
	    }
	    if(ii < 0)
		hooknm = defaultHookName ;
	}
	assignHooks(bp,hooknm) ;
    }
    if(bp->fhook >= 0)
        execBufferFunc(bp,bp->fhook,meEBF_ARG_GIVEN,(bp->intFlag & BIFFILE)) ;
    storeWindBSet(bp,curwp) ;
    if(tbp != curbp)
    {
        bp->b_nwnd-- ;
	curbp = curwp->w_bufp = tbp ;
        tbp->b_nwnd++ ;
        curwp->topLineNo = topLineNo ;
        curwp->w_flag |= w_flag ;
	restoreWindBSet(curwp,tbp) ;
	isWordMask = tbp->isWordMask ;
    }    
    resetBufferWindows(bp) ;
}


int
swbuffer(WINDOW *wp, BUFFER *bp)        /* make buffer BP current */
{
    register WINDOW *twp ;
    register BUFFER *tbp ;
    long             lineno=0 ;
    
    tbp = wp->w_bufp ;
    if((wp == curwp) && (curbp->ehook >= 0))
        execBufferFunc(curbp,curbp->ehook,0,1) ;
    tbp->histNo = ++bufHistNo ;
    if(--tbp->b_nwnd == 0)
    {
        /* Get the line number before it beomes corrupted */
        if ((meModeTest(bp->b_mode,MDNACT)) && (bp->intFlag & BIFFILE))
        {
            lineno = bp->line_no ;
            storeWindBSet(tbp,wp) ;
            bp->line_no = lineno ;
        }
        else
            storeWindBSet(tbp,wp) ;
    }
    if(meModeTest(bp->b_mode,MDNACT))
    {   
	/* buffer not active yet */
	if(bp->intFlag & BIFFILE)
	{
	    /* The buffer is linked to a file, load it in */
	    lineno = bp->line_no ;
	    readin(bp,bp->b_fname);
	}
	meModeClear(bp->b_mode,MDNACT) ;
	/* Now set the buffer context */
	setBufferContext(bp) ;
    }
    
    /* Switch. reset any scroll */
    wp->w_bufp = bp ;
    wp->w_sscroll = 0 ;
    wp->w_scscroll = 0 ;
    wp->w_flag |= WFMODE|WFMAIN|WFSBOX ;

    if(wp == curwp)
    {
        curbp = bp;
        if(isWordMask != curbp->isWordMask)
        {
            isWordMask = curbp->isWordMask ;
#if MAGIC
            mereRegexClassChanged() ;
#endif
        }
    }
    if(bp->b_nwnd++ == 0)
    {
	/* First use.           */
	restoreWindBSet(wp,bp) ;
        if(meModeTest(curbp->b_mode,MDPIPE))
        {
            /* if its a piped command we can't rely on the bp markp being correct,
             * reset */
	    wp->w_markp = NULL;                 /* Invalidate "mark"    */
	    wp->w_marko = 0;
	    wp->mlineno = 0;
        }            
	if(lineno > 0)
	    gotoLine(TRUE,lineno) ;
    }
    else
    {
	twp = wheadp;                            /* Look for old.        */
	while (twp != NULL)
	{
	    if (twp!=wp && twp->w_bufp==bp) 
	    {
		restoreWindWSet(wp,twp) ;
		break;
	    }
	    twp = twp->w_wndp;
	}
    }
#ifdef _IPIPES
    if(meModeTest(bp->b_mode,MDPIPE) &&
       ((wp == curwp) || (bp->b_nwnd == 1)))
	ipipeSetSize(wp,bp) ;
#endif        
    if((wp == curwp) && (curbp->bhook >= 0))
        execBufferFunc(curbp,curbp->bhook,0,1) ;
    
    if(bp->intFlag & BIFLOAD)
    {
        bp->intFlag &= ~BIFLOAD ;
	if(bufferOutOfDate(bp))
	{
            lineno = bp->line_no ;
            update(TRUE) ;
            if((mlyesno((uint8 *)"File changed on disk, reload") == TRUE) &&
               (bclear(bp) == TRUE))
            {
                /* achieve cheekily by calling itself as the bclear make it inactive,
                 * This ensures the re-evaluation of the hooks etc.
                 */
                /* SWP 7/7/98 - If the file did not exist to start with, i.e. new file,
                 * and now it does then we need to set the BIFFILE flag */
                bp->intFlag |= BIFFILE ;
                bp->line_no = lineno+1 ;
		return swbuffer(wp,bp) ;
	    }
	}
	else
	    mlwrite(MWCLEXEC,(uint8 *)"[Old buffer]");
    }
    return TRUE ;
}

/*
 * Attach a buffer to a window. The
 * values of dot and mark come from the buffer
 * if the use count is 0. Otherwise, they come
 * from some other window.
 */
int
findBuffer(int f, int n)
{
    register BUFFER *bp;
    register int    s;
    uint8           bufn[MAXBUF];

    if((s = getBufferName((uint8 *)"Find buffer",0,2,bufn)) != TRUE)
	return s ;
    if(n > 0)
	f = BFND_CREAT ;
    else
	f = 0 ;
    if((bp=bfind(bufn,f)) == NULL)
	return FALSE ;
    if(n < 0)
	return HideBuffer(bp) ;
    return swbuffer(curwp, bp) ;
}

/*
 * Find a buffer into another window. This is an alternative to mapping
 * ^X2 (ie C-x 2) onto splitWindVert().
 *
 *                                      Tony Bamford 31/8/86 tony@root.UUCP
 */
int     
nextWndFindBuf(int f, int n)
{
    register BUFFER *bp;
    register int    s;
    uint8           bufn[MAXBUF];
    
    if((s = getBufferName((uint8 *)"Use buffer", 0, 2, bufn)) != TRUE)
	return(s);
    if(n)
	n = BFND_CREAT ;
    if((wpopup(NULL,WPOP_MKCURR) == NULL) ||
       ((bp=bfind(bufn,n)) == NULL))
	return FALSE ;
    return swbuffer(curwp, bp) ;
}

int     
nextBuffer(int f, int n)   /* switch to the next buffer in the buffer list */
{
    register BUFFER *bp, *pbp;
    
    bp = curbp ;
    if(n < 0)
    {
        /* cycle backward through the buffers to find an eligable one */
        while(n < 0)
        {
            if(bp == bheadp)
                bp = NULL;
            pbp = bheadp ;
            while(pbp->b_bufp != bp)
                pbp = pbp->b_bufp;
            bp = pbp ;
            if(bp == curbp)
                break ;
            if(!meModeTest(bp->b_mode,MDHIDE))
                n++ ;
        }
    }
    else
    {
        /* cycle forwards through the buffers to find an eligable one */
        while(n > 0)
        {
            bp = bp->b_bufp;
            if(bp == NULL)
                bp = bheadp;
            if(bp == curbp)
                break ;
            if(!meModeTest(bp->b_mode,MDHIDE))
                n-- ;
        }
    }
    if(bp == curbp)
        return ctrlg(FALSE,1);
    return(swbuffer(curwp,bp));
}


/*
** Find a replacement buffer.
** given a buffer oldbuf (assumed to be displayed), then find the best
** buffer to replace it by. There are 3 main catagories
** 1) undisplayed normal buffer.
** 2) displayed normal buffer.
** 3) default buffer main. (created).
** buffers higher in the history have preference.
** returns a pointer to the replacement (guaranteed not to be the oldbuf).
*/
BUFFER *
replacebuffer(BUFFER *oldbuf)
{
    BUFFER *bp, *best=NULL, *next=NULL ;
    int     histNo=-1, nextNo=-1 ;
    
    for(bp=bheadp ; bp != NULL;bp = bp->b_bufp)
    {
	if((bp != oldbuf) && !meModeTest(bp->b_mode,MDHIDE))
	{
	    if((bp->b_nwnd == 0) && (bp->histNo > histNo))
	    {
		histNo = bp->histNo ;
		best = bp ;
	    }
	    else if(nextNo <= bp->histNo)
	    {
		nextNo = bp->histNo ;
		next = bp ;
	    }
	}
    }
    /*
    ** if didnt find a buffer, create a new *scratch* 
    */
    if(best == NULL)
	best = next ;
    if(best == NULL)
	best = bfind(BmainN,BFND_CREAT) ;
	
    return best ;
}


int
HideBuffer(BUFFER *bp)
{
    register BUFFER *bp1 ;
    WINDOW          *wp ;
    
    while(bp->b_nwnd > 0)
    {
	for(wp = wheadp; wp != (WINDOW*) NULL; wp = wp->w_wndp)
	    if(wp->w_bufp == bp)
	    {
		/* find a replacement buffer */
		/* if its the same then can't replace so return */
		if((bp1 = replacebuffer(bp)) == bp)
		    /* this is true if only *scratch* is left */
		    return FALSE ;
		    
		if(swbuffer(wp, bp1) != TRUE)
		    return FALSE ;
	    }
    }
    bp->histNo = 0 ;
    
    return TRUE ;
}


void
resetBufferWindows(BUFFER *bp)
{
    register WINDOW *wp ;
    wp = wheadp ;
    while(wp != NULL)
    {
	if(wp->w_bufp == bp)
	{
	    wp->topLineNo = 0 ;                 /* Fix top line         */
	    wp->w_dotp  = bp->b_dotp ;          /* Fix "."              */
	    wp->w_doto  = bp->b_doto ;
	    wp->line_no = bp->line_no ;
	    wp->w_markp = NULL;                 /* Invalidate "mark"    */
	    wp->w_marko = 0;
	    wp->mlineno = 0;
	    wp->w_flag |= WFMOVEL|WFREDRAW|WFSBOX ;
	}
	wp = wp->w_wndp ;
    }   
}
/*
 * This routine blows away all of the text
 * in a buffer. If the buffer is marked as changed
 * then we ask if it is ok to blow it away; this is
 * to save the user the grief of losing text. The
 * window chain is nearly always wrong if this gets
 * called; the caller must arrange for the updates
 * that are required. Return TRUE if everything
 * looks good.
 */
/* Note for Dynamic file names
 * bclear does not touch the file name.
 */
int
bclear(register BUFFER *bp)
{
    meMODE bmode ;
    register meAMARK *mk, *nmk ;
    register LINE    *lp ;
    register int      s;
    
#ifdef _IPIPES
    if(meModeTest(bp->b_mode,MDPIPE))
    {
	meIPIPE *ipipe ;
	
	if(!(bp->intFlag & BIFBLOW) &&
	   ((s=mlyesno((uint8 *)"Kill active process")) != TRUE))
	    return s ;
	
        /* while waiting for a Yes from the user, the process may
         * have finished, so be careful */
        ipipe = ipipes ;
	while(ipipe != NULL)
        {
            if(ipipe->bp == bp)
            {
                ipipeRemove(ipipe) ;
                break ;
            }
	    ipipe = ipipe->next ;
        }
    }
#endif        
    if(bufferNeedSaving(bp) && !(bp->intFlag & BIFBLOW))
    {   /* Something changed    */
	uint8 prompt[MAXBUF+20] ;

	if(bp->b_fname != NULL)
	    meStrcpy(prompt,bp->b_fname) ;
	else
	    meStrcpy(prompt,bp->b_bname) ;
        meStrcat(prompt,": Discard changes") ;
	s = mlyesno(prompt) ;
	if(s != TRUE)
	    return(s);
        autowriteremove(bp) ;
    }
    
    /* There is a problem when we kill a buffer, if it is the current
     * buffer (curwp) then the end hook needs to be executed before
     * the local variables are blown away. It would seem sensible that
     * it is done here. Execute the end hook and then kill off the end
     * hook to ensure that it is not executed again. On a re-read of
     * a file the file hooks are re-assigned so this should not cause
     * a problem - I hope !!.
     * 
     * Note that this problem has been introduced by local variables
     * it is not a problem if global variables are used.
     * 
     * zotbuf() calls bclear; hence any operation to delete a buffer
     * will cause the end hook to be executed.
     */
    if((curwp->w_bufp == bp) && (bp->ehook >= 0))
    {
	execBufferFunc(bp,bp->ehook,0,1) ;      /* Execute the end hook */
	bp->ehook = -1;                         /* Disable the end hook */
    }
    if(!meModeTest(bp->b_mode,MDNACT) && (bp->dhook >= 0))
	execBufferFunc(bp,bp->dhook,0,1) ;      /* Execute the delete hook */
    
    /* Continue the destruction of the buffer space. */
    /* The following modes to preserve are: MDBINARY, MDCRYPT, MDDEL */
    meModeCopy(bmode,bp->b_mode) ;
    meModeCopy(bp->b_mode,globMode) ;
    meModeSet(bp->b_mode,MDNACT) ;
    if(meModeTest(bmode,MDBINRY))
        meModeSet(bp->b_mode,MDBINRY) ;
    if(meModeTest(bmode,MDCRYPT))
        meModeSet(bp->b_mode,MDCRYPT) ;
    if(meModeTest(bmode,MDDEL))
        meModeSet(bp->b_mode,MDDEL) ;
    bp->intFlag &= ~(BIFLOAD|BIFBLOW|BIFLOCK) ;
    lp = bp->b_linep ;
    freeLineLoop(lp,0) ;
    lp->l_flag  = 0 ;
    bp->b_dotp  = lp ;
    bp->topLineNo = 0 ;
    bp->b_doto  = 0;
    bp->line_no = 0;
    bp->b_markp = NULL;                     /* Invalidate "mark"    */
    bp->b_marko = 0;
    bp->mlineno = 0;
    bp->elineno = 0;
    bp->fhook = bp->dhook = bp->bhook = bp->ehook = -1 ;
#if MEUNDO
    meUndoRemove(bp) ;
#endif
#if FLNEXT
    meNullFree(bp->nextFile) ;
    bp->nextFile = NULL ;
#endif
    /* Clean out the local buffer variables */
    if (bp->varList.head != NULL)
    {
	meVARIABLE *bv, *tv;
	bv = bp->varList.head ;
	do
	{
	    tv = bv;
	    bv = bv->next;
	    meNullFree(tv->value) ;
	    free(tv) ;
	} while (bv != NULL);
	bp->varList.head = NULL ;
	bp->varList.count = 0 ;
    }
    mk = bp->b_amark;
    while(mk != NULL)
    {
	nmk = mk->next ;
	meFree(mk) ;
	mk = nmk ;
    }
    bp->b_amark = NULL ;
#if LCLBIND
    if(bp->bbinds != NULL)
	meFree(bp->bbinds) ;
    bp->bbinds = NULL ;
    bp->nobbinds = 0 ;
#endif
#if NARROW
    if(bp->narrow != NULL)
    {
        meNARROW *nw, *nnw ;
        nnw = bp->narrow ;
        do
        {
            nw = nnw ;
            nnw = nw->next ;
            /* Create the loop for freeing */
            nw->elp->l_fp = nw->slp ;
            freeLineLoop(nw->slp,1) ;
            meFree(nw) ;
        } while(nnw != NULL) ;
        bp->narrow = NULL ;
    }
#endif
    resetBufferWindows(bp) ;
    return TRUE ;
}


void
linkBuffer(BUFFER *bp)
{    
    register BUFFER *sb, *lb;   /* buffer to insert after */
    register uint8  *bname=bp->b_bname ;
    
    /* find the place in the list to insert this buffer */
    lb = NULL ;
    sb = bheadp;
    while(sb != NULL)
    {
#ifdef _INSENSE_CASE
	if(meStridif(sb->b_bname,bname) >= 0)
#else
	if(meStrcmp(sb->b_bname,bname) >= 0)
#endif
	    break ;
	lb = sb ;
	sb = sb->b_bufp;
    }
    /* and insert it */
    bp->b_bufp = sb ;
    if(lb == NULL)
	/* insert at the begining */
	bheadp = bp;
    else
	lb->b_bufp = bp ;
}

void
unlinkBuffer(BUFFER *bp)
{
    register BUFFER *bp1 ;
    register BUFFER *bp2 ;
    
    bp1 = NULL;                             /* Find the header.     */
    bp2 = bheadp;
    while (bp2 != bp)
    {
	bp1 = bp2;
	bp2 = bp2->b_bufp;
    }
    bp2 = bp2->b_bufp;                      /* Next one in chain.   */
    if (bp1 == NULL)                        /* Unlink it.           */
	bheadp = bp2;
    else
	bp1->b_bufp = bp2;
}

/* Note for Dynamic file names
 * zotbuf frees the file name so it must be dynamic and unique to buffer 
 *        or NULL.
 */
int      
zotbuf(register BUFFER *bp, int silent) /* kill the buffer pointed to by bp */
{
    register int     s ;
    
    /* Blow text away. last chance for user to abort so do first */
    if ((s=bclear(bp)) != TRUE)
	return (s);
    /* if this is the scratch and the only buffer then don't delete */
    
    /*
    ** If the buffer we want to kill is displayed on the screen, make
    ** it the current window and then remove it from the screen. Once
    ** this has been done, we can then go about zapping it.
    */
    /* This must really be the curwp, ie curbp must be bp otherwise
    ** swbuffer fucks up big time.
    */
    if(HideBuffer(bp) != TRUE)
	/* only scratch left */
	return TRUE ;
    
    meFree(bp->b_linep);             /* Release header line. */
    unlinkBuffer(bp) ;
    meNullFree(bp->b_fname) ;
#if CRYPT
    meNullFree(bp->b_key) ;
#endif
    meNullFree(bp->modeLineStr) ;
    if(!silent)
	/* say it went ok       */
	mlwrite(0,(uint8 *)"[buffer %s killed]", bp->b_bname);
    meNullFree(bp->b_bname) ;
    meFree(bp);                             /* Release buffer block */
    return (TRUE);
}

/*
 * Dispose of a buffer, by name.
 * Ask for the name. Look it up (don't get too
 * upset if it isn't there at all!). Get quite upset
 * if the buffer is being displayed. Clear the buffer (ask
 * if the buffer has been changed). Then free the header
 * line and the buffer header. Bound to "C-X K".
 */
int
delBuffer(int f, int n)
{
    register BUFFER *bp;
    register int s;
    uint8 bufn[MAXBUF];

    if((s = getBufferName((uint8 *)"Delete buffer", MLNOSTORE,1,bufn)) != TRUE)
	return(s);
    
    if((bp=bfind(bufn, 0)) == NULL) 
	/* Easy if unknown.     */
	return mlwrite(MWABORT|MWCLEXEC,(uint8 *)"[%s: no such buffer]", bufn);
    if(bp->intFlag & BIFNODEL)
	return mlwrite(MWABORT|MWCLEXEC,(uint8 *)"[%s cannot be deleted]", bufn);
        
    if(!(n & 0x01))
	bp->intFlag |= BIFBLOW ;

    return zotbuf(bp,clexec) ;
}


int
delSomeBuffers(int f, int n)
{
    BUFFER *bp, *nbp ;
    uint8   prompt[MAXBUF] ;
    int     s ;
    
    bp = bheadp ;
    while(bp != NULL)
    {
	nbp = bp->b_bufp ;
	if(!meModeTest(bp->b_mode,MDHIDE))
	{
	    if(!(n & 0x01) ||
	       ((sprintf((char *)prompt,"Delete buffer [%s] ",bp->b_bname)),
		((s=mlyesno(prompt)) == TRUE)))
	    {
                if(!(n & 0x01))
                    bp->intFlag |= BIFBLOW ;
		if(zotbuf(bp,clexec) != TRUE)
		    return ABORT ;
	    }
	    else if(s == ABORT)
		return ABORT ;
	}
	bp = nbp ;
    }
    return TRUE ;
}

int
changeBufName(int f, int n)     /*      Rename the current buffer       */
{
    uint8 bufn[MAXBUF], *nn;     /* buffer to hold buffer name */
    register int s ;
    
    /* prompt for and get the new buffer name */
    if((s = getBufferName((uint8 *)"New buffer name", 0, 0, bufn)) != TRUE)
	return s ;

    unlinkBuffer(curbp) ;
    if(n & 1)
    {
	/* check for duplicates */
	if(bfind(bufn,0) != NULL)
	{
	    /* if the names the same */
	    linkBuffer(curbp) ;
	    return mlwrite(MWABORT|MWCLEXEC,(uint8 *)"[Already exists!]") ;
	}
    }
    else
	/* if bit one is clear then make it a valid name */
	curbp->intFlag = (curbp->intFlag & ~BIFNAME) | makename(bufn,bufn) ;
    if((nn = meMalloc(meStrlen(bufn)+1)) == NULL)
    {
	linkBuffer(curbp) ;
	return ABORT ;
    }
    meStrcpy(nn,bufn);   /* copy buffer name to structure */
    meFree(curbp->b_bname) ;
    curbp->b_bname = nn ;
    addModeToWindows(WFMODE) ;
    linkBuffer(curbp) ;
    
    return TRUE ;
}


/*
 * The argument "text" points to
 * a string. Append this line to the
 * given buffer. Handcraft the EOL
 * on the end. Return TRUE if it worked and
 * FALSE if you ran out of room.
 * if pos is TRUE then insert to current line
 * else to the end of the buffer
 */
int 
addLine(register LINE *ilp, uint8 *text)
{
    register LINE  *lp ;
    register int    ntext ;
    register uint8 *ss ;
    
    ntext = meStrlen(text);
    if ((lp=lalloc(ntext)) == NULL)
	return 0 ;
    
    ss = lp->l_text ;
    while((*ss++ = *text++))
	;
    
    ilp->l_bp->l_fp = lp;
    lp->l_bp = ilp->l_bp;
    ilp->l_bp = lp;
    lp->l_fp = ilp ;
    return 1 ;
}

/*
 * This routine rebuilds the
 * text in the given buffer
 * that holds the buffer list. It is called
 * by the list buffers command. Return TRUE
 * if everything works. Return FALSE if there
 * is an error (if there is no memory).
 */
static void
makelist(BUFFER *blistp, int verb)
{
    register BUFFER *bp ;
    register uint8   cc, *cp1, *cp2 ;
    register int     ii, ff ;
    uint8 line[MAXBUF] ;
    
    addLineToEob(blistp,(uint8 *)"AC    Size Buffer          File") ;
    addLineToEob(blistp,(uint8 *)"--    ---- ------          ----") ;
    bp = bheadp;                            /* For all buffers      */
    
    /* output the list of buffers */
    while (bp != NULL)
    {
	if(!verb && meModeTest(bp->b_mode,MDHIDE))
	{
	    bp = bp->b_bufp;
	    continue;
	}
	cp1 = line ;                      /* Start at left edge   */
      
	/* output status of ACTIVE flag (has the file been read in? */
	if(meModeTest(bp->b_mode,MDDEL))         /* "D" to be deleted    */
	    *cp1++ = 'D';
	else if(meModeTest(bp->b_mode,MDNACT))   /* "@" if activated     */
	    *cp1++ = ' ';
	else
	    *cp1++ = '@';
	
	/* output status of changed flag */
	if(meModeTest(bp->b_mode,MDSAVE))       /* "S" if to be saved   */
	    *cp1++ = 'S';
	else if(meModeTest(bp->b_mode,MDEDIT))  /* "*" if changed       */
	    *cp1++ = '*';
	else if(meModeTest(bp->b_mode,MDVIEW))  /* "%" if in view mode  */
	    *cp1++ = '%';
	else
	    *cp1++ = ' ';
	if((bp == blistp) || meModeTest(bp->b_mode,MDNACT))
	    cp2 = (uint8 *) "-" ;
	else
	    cp2 = meItoa(bp->elineno+1);
	ii = 8 - meStrlen(cp2) ;
	while(ii-- > 0)
	    *cp1++ = ' ' ;
	while((cc = *cp2++) != '\0')
	    *cp1++ = cc ;
	cp2 = bp->b_bname ;
	ii = NBUFN ;
	if((ff=(meStrchr(cp2,' ') != NULL)))
	    *cp1 = '"';
	else
	    *cp1 = ' ';
	while((*++cp1 = *cp2++) != 0)
	    ii-- ;
	if(ff)
	    *cp1++ = '"';
	else
	    *cp1++ = ' ';
	ii-- ;
	
	for( ; ii>0 ; ii--)
	    *cp1++ = ' ';
	
	if(verb)
	{
#if MEUNDO
	    UNDOND *nn ;
#endif
	    LINE   *ll ;
	    int     jj ;
	    
	    ii = 0 ;
	    jj = 0 ;
	    ll = bp->b_linep ;
	    do
	    {
		ii += llength(ll)+1 ;
		jj += ll->l_size ;
		ll = lforw(ll) ;
	    } while(ll != bp->b_linep) ;
	    *cp1++ = ' ';
	    cp2 = meItoa(ii) ;
	    while((cc = *cp2++) != '\0')
		*cp1++ = cc ;
	    ff = line + 45 - cp1 ;
	    while(ff-- > 0)
		*cp1++ = ' ' ;
	    jj += (sizeof(LINE)*(bp->elineno+1)) + sizeof(BUFFER) ;
	    cp2 = meItoa(jj) ;
	    while((cc = *cp2++) != '\0')
		*cp1++ = cc ;
#if MEUNDO
	    ff = line + 53 - cp1 ;
	    while(ff-- > 0)
		*cp1++ = ' ' ;
	    ii = 0 ;
	    nn = bp->fUndo ;
	    while(nn != NULL)
	    {
		ii += sizeof(UNDOND) ;
		if(nn->type & MEUNDO_MDEL)
		    ii += meStrlen(nn->str) ;
		if(nn->type & MEUNDO_REPL)
		    ii += sizeof(UNDOCOORD)*nn->doto ;
		nn = nn->next ;
	    }
	    cp2 = meItoa(ii) ;
	    while((cc = *cp2++) != '\0')
		*cp1++ = cc ;
#endif
	}
	else if((bp->b_bname[0] != '*') &&
	   ((cp2 = bp->b_fname) != NULL))
	{
	    if((ii=meStrlen(cp2))+(cp1-line) > TTncol)
	    {
		*cp1++ = '$' ;
		cp2 += ii - (TTncol-(1+cp1-line)) ;
	    }
	    while((cc = *cp2++) != '\0')
		*cp1++ = cc ;
	}
	*cp1 = '\0' ;
	addLineToEob(blistp,line) ;
	bp = bp->b_bufp;
    }
    meModeSet(blistp->b_mode,MDVIEW) ;        /* put this buffer view mode */
}


/*---   List all  of the  active buffers.   First update  the special buffer
	that holds the list.  Next make sure at least 1 window is displaying
	the buffer  list, splitting  the screen  if this  is what  it takes.
	Lastly, repaint  all of  the windows  that are  displaying the list.
	Bound to "C-X C-B".   */
	
int
listBuffers (int f, int n)
{
    register WINDOW    *wp ;
    register BUFFER    *bp ;
    
    if((wp=wpopup(BbuffersN,(BFND_CREAT|BFND_CLEAR|WPOP_USESTR))) == NULL)
	return mlwrite(MWABORT,(uint8 *)"[Failed to create list]") ;
    bp = wp->w_bufp ;
    makelist(bp,f) ;
    
    bp->b_dotp = lforw(bp->b_linep);
    bp->b_doto = 0 ;
    bp->line_no = 0 ;
    resetBufferWindows(bp) ;
    return TRUE ;
}

/*
** Returns :-
** TRUE  if the given buffer needs saving
** FALSE if it doesnt
 */
int
bufferNeedSaving(BUFFER *bp)
{
    /* If changed (edited) and valid buffer name (not system) */
    if(meModeTest(bp->b_mode,MDEDIT) && (bp->b_bname[0] != '*'))
	return TRUE ;
    return FALSE ;
}

/*
 * Look through the list of
 * buffers. Return TRUE if there
 * are any changed buffers. Buffers
 * that hold magic internal stuff are
 * not considered; who cares if the
 * list of buffer names is hacked.
 * Return FALSE if no buffers
 * have been changed.
 */
int
anycb(void)
{
    register BUFFER *bp;
    
    bp = bheadp;
    while (bp != NULL)
    {
	if(bufferNeedSaving(bp))
	    return (TRUE);
	bp = bp->b_bufp;
    }
    return (FALSE);
}

BUFFER *
createBuffer(register uint8 *bname)
{
    register BUFFER *bp;
    register LINE   *lp;

    if((bp=(BUFFER *)meMalloc(sizeof(BUFFER))) == NULL)
	return NULL ;
    if((lp=lalloc(0)) == NULL)
    {
	meFree(bp);
	return NULL ;
    }
    /* set everything to 0 */
    memset(bp,0,sizeof(BUFFER)) ;
    if((bp->b_bname = meMalloc(meStrlen(bname)+1)) == NULL)
	return NULL ;
    meStrcpy(bp->b_bname, bname);
    linkBuffer(bp) ;
    
    /* and set up the non-zero buffer fields */
    meModeCopy(bp->b_mode,globMode) ;
    meModeSet(bp->b_mode,MDNACT) ;
    bp->b_dotp   = bp->b_linep = lp;
    bp->autotime = -1 ;
    bp->stats.stmtime = (long) -1;
    bp->stats.stmode  = meUmask ;
    bp->stats.stsize  = 0 ;
#ifdef _UNIX
    bp->stats.stuid = meUid ;
    bp->stats.stgid = meGid ;
    bp->stats.stdev = -1 ;
    bp->stats.stino = -1 ;
#endif
#ifdef _IPIPES
    bp->ipipeFunc = -1 ;
#endif
    bp->inputFunc = -1 ;
    bp->isWordMask = CHRMSK_DEFWORDMSK ;
    bp->fhook = -1 ;
    bp->bhook = -1 ;
    bp->dhook = -1 ;
    bp->ehook = -1 ;
#if COLOR
    /* set the colors of the new window */
    bp->scheme = globScheme;
#endif
    lp->l_fp = lp;
    lp->l_bp = lp;
    
    return bp ;
}


/*
 * Find a buffer, by name. Return a pointer
 * to the BUFFER structure associated with it.
 * If the buffer is not found
 * and the "cflag" is TRUE, create it. The "bflag" is
 * the settings for the flags in in buffer.
 */
BUFFER *
bfind(register uint8 *bname, int cflag)
{
    BUFFER *bp;
    meMODE  sglobMode ;
    uint8 intFlag=0 ;
    uint8 *bnm ;
    uint8 bn[MAXBUF] ;
    int ii ;
    
    if(cflag & BFND_MKNAM)
    {
	intFlag = makename(bn,bname) ;
	bnm = bn ;
    }
    else
    {
	bp = bheadp;
	while(bp != NULL)
	{
#ifdef _INSENSE_CASE
	    if((ii=meStricmp(bp->b_bname,bname)) == 0)
#else
	    if((ii=meStrcmp(bp->b_bname,bname)) == 0)
#endif
	    {
		if(cflag & BFND_CLEAR)
		{
		    bp->intFlag |= BIFBLOW ;             /* Don't complain!      */
                    bclear(bp) ;
                    /* The BFND_CLEAR functionality is only used for system buffers
                     * such as *help* *command* etc. i.e. not user loaded buffer. 
                     * These buffers do not and must not be linked to a filename.
                     * This can happen if the user renames "./foo" to *compile* say
                     * and then runs another compile, it its still linked to foo then
                     * it gets loaded */
                    bp->intFlag &= ~BIFFILE ;
                    if(bp->b_fname != NULL)
                    {
                        meFree(bp->b_fname) ;
                        bp->b_fname = NULL ;
                    }
		    /* set the modes correctly */
		    if(cflag & BFND_BINARY)
			meModeSet(bp->b_mode,MDBINRY) ;
		    if(cflag & BFND_CRYPT)
			meModeSet(bp->b_mode,MDCRYPT) ;
		}
		return bp ;
	    }
	    if(ii>0)
		break ;
	    bp = bp->b_bufp;
	}
	if(!cflag)
	    return NULL ;
	bnm = bname ;
    }
    meModeCopy(sglobMode,globMode) ;
    if(cflag & BFND_BINARY)
	meModeSet(globMode,MDBINRY) ;
    if(cflag & BFND_CRYPT)
	meModeSet(globMode,MDCRYPT) ;
    bp = createBuffer(bnm) ;
    bp->intFlag |= intFlag ;
    meModeCopy(globMode,sglobMode) ;
    return bp ;
}



int
adjustMode(BUFFER *bp, int nn)  /* change the editor mode status */
{
    register uint8 *mode ;
    int   func ;
    uint8 prompt[50];            /* string to prompt user with */
    uint8 cbuf[NPAT];            /* buffer to recieve mode name into */
    
    if(nn >= 128)
    {
	nn -= 128 ;
	func = 0 ;
    }
    else if(nn < 0)
    {
	nn = 0 - nn ;
	func = -1 ;
    }
    else if(nn == 0)
	func = 0 ;
    else
	func = 1 ;
    
    if (bp == NULL)
	mode = globMode ;
    else
	mode = bp->b_mode ;
    
    if(nn < 2)
    {
	/* build the proper prompt string */
	meStrcpy(prompt,"Buffer mode to ");
	if(bp == NULL)
	    meStrncpy(prompt,"Global",6);
	
	if(func == 0)
	    meStrcpy(prompt+15, "toggle");
	else if(func > 0)
	    meStrcpy(prompt+15, "add");
	else
            meStrcpy(prompt+15, "delete");
	
	/* Setup the completion */
	mlgsStrList = modeName ;
	mlgsStrListSize = MDNUMMODES ;
	
	/* prompt the user and get an answer */
	if(mlreply(prompt, MLLOWER|MLUSER, 0, cbuf, NPAT) == ABORT)
	    return(FALSE);
    
	/* test it against the modes we know */
	for (nn=0; nn < MDNUMMODES ; nn++) 
	    if(meStrcmp(cbuf,modeName[nn]) == 0) 
		break ;
    }
    else
	nn -= 2 ;
    
    if(nn >= MDNUMMODES)
	return mlwrite(MWABORT,(uint8 *)"[No such mode]");
    
    switch(nn)
    {
    case MDEDIT:
	if(bp == NULL)
	    goto invalid_global ;
		
	if(meModeTest(bp->b_mode,MDEDIT))
	{
	    if(func <= 0)
	    {
		autowriteremove(bp) ;
#if MEUNDO
		meUndoRemove(bp) ;
#endif
		meModeClear(bp->b_mode,MDEDIT) ;
		addModeToWindows(WFMODE) ;
	    }
	}
	else if(func >= 0)
	{
	    if(bp != curbp)
	    {
		register BUFFER *tbp = curbp ;
		
		storeWindBSet(tbp,curwp) ;
		swbuffer(curwp,bp) ;
		
		nn = bchange() ;
		
		swbuffer(curwp,tbp) ;
		restoreWindBSet(curwp,tbp) ;
	    }
	    else
		nn = bchange() ;
            
            if(nn != TRUE)
                return nn ;
	}
        if((func == 0) && !clexec)
            mlwrite(0,(uint8 *)"[%s edit mode is now %s]",
                    (bp == NULL) ? "Global":"Buffer",
                    meModeTest(mode,MDEDIT) ? "on":"off");
	return TRUE ;
	
    case MDQUIET:
	/* quiet mode - only set globaly */
	if(bp != NULL)
	    goto invalid_global ;
	break ;
	
    case MDLOCK:
	if((bp == NULL) || !meModeTest(bp->b_mode,MDPIPE)) 
	    goto invalid_global ;
	break ;
	    
    case MDHIDE:
    case MDDEL:
    case MDSAVE:
	if(bp == NULL)
	    goto invalid_global ;
	if((nn == MDSAVE) && !bufferNeedSaving(bp))
	    return mlwrite(0,(uint8 *)"[No changes made]") ;
	break ;
	
    case MDDIR:
    case MDNACT:
    case MDNRRW:
    case MDPIPE:
invalid_global:
	return mlwrite(MWABORT,(uint8 *)"[Cannot change this mode]");
    }
    /* make the mode change */
    if(func == 0)
    {
        meModeToggle(mode,nn) ;
        if(!clexec)
            mlwrite(0,(uint8 *)"[%s %s mode is now %s]",
                    (bp == NULL) ? "Global":"Buffer",modeName[nn],
                    meModeTest(mode,nn) ? "on":"off");
    }
    else if(func > 0)
	meModeSet(mode,nn) ;
    else
	meModeClear(mode,nn) ;
    
    if(meModeTest(mode,nn))
    {
	/*
	 * make sure that, if INDENT is set, CMODE isnt
	 * and vice versa
	 */
	if(nn == MDINDEN)
	    meModeClear(mode,MDCMOD) ;
	else if(nn == MDCMOD)
	    meModeClear(mode,MDINDEN) ;
    }
    else if(bp != NULL)
    {
#if MEUNDO
	if(nn == MDUNDO)
	    meUndoRemove(bp) ;
	else
#endif
            if(nn == MDATSV)
	    bp->autotime = -1 ;
    }
    /* display new mode line */
    if(bp != NULL)
        /* and update all buffer window mode lines */
        addModeToBufferWindows(bp,WFMODE) ;
    
    return TRUE ;
}
   
int     
bufferMode(int f, int n)        /* prompt and set an editor mode */
{
    return adjustMode(curbp,(f) ? n:0) ;
}

int     
globalMode(int f, int n)        /* prompt and set a global editor mode */
{
    return adjustMode(NULL,(f) ? n:0);
}

int     
namedBufferMode(int f, int n)   /* prompt and set an editor mode */
{
    register BUFFER *bp;
    register int s;
    uint8 bufn[MAXBUF];
    
    if((s = getBufferName((uint8 *)"Buffer name", MLNOSTORE,1,bufn)) != TRUE)
	return s ;
    
    if((bp=bfind(bufn, 0)) == NULL) 
	/* Easy if unknown.     */
	return mlwrite(MWABORT|MWCLEXEC,(uint8 *)"[No such buffer %s]", bufn);
    return adjustMode(bp,(f) ? n:0) ;
}

