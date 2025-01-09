/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * registry.c - Internal registry routines.
 *
 * Copyright (c) 1998-2001 Jon Green
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
 * Created:     26/03/1998
 * Synopsis:    Internal registry routines.
 * Authors:     Jon Green & Steven Phillips
 */

#define __REGISTRYC                     /* Define the name of the file */

#include "emain.h"

#if MEOPT_REGISTRY

#ifdef _STDARG
#include <stdarg.h>		/* Variable Arguments */
#endif

/* Local definitions for the registry file lexical analyser */
#define TOK_STRING    1
#define TOK_OPEN      2
#define TOK_CLOSE     3
#define TOK_ASSIGN    4
#define TOK_EOF       5
#define TOK_EOL       6


/* Static global states. */
static int lineNo;                      /* Current line number */
static meRegNode root =
{
    NULL,                               /* The value of the node */
    NULL,                               /* Parent pointer */
    NULL,                               /* Sibling pointer */
    NULL,                               /* Child pointer */
    '\0',                               /* Mode flag */
    {
        '\0'
    }                                   /* Root identified */
};
/*
 * rnodeNew
 * Allocate and initialise a new node
 */
static meRegNode *
rnodeNew (meUByte *name)
{
    meRegNode *np;                          /* Pointer to the node */
    int len;                            /* Length of the string */

    /* Get the length of the node name */
    len = meStrlen(name) ;

    /* Allocate space for the node and copy in the node name */
    if ((np = (meRegNode *) meMalloc(sizeof(meRegNode) + len)) == NULL)
        return NULL;
    memset(np, 0, sizeof(meRegNode));
    meStrcpy(np->name,name) ;
    return (np);
}

/*
 * rnodeDel
 * Recursively delete a node.
 */
static void
rnodeDelete (meRegNode *np)
{
    /* Root node ?? */
    if (np == &root)
    {
        /* delete the children and return */
        if (np->child != NULL)          /* Destruct children */
        {
            rnodeDelete (np->child);
            np->child = NULL ;
        }
    }
    else
    {
        /* Recursively delete the tree */
        meRegNode *tnp;                     /* Temporary node pointer */
        while (np != NULL)
        {
            tnp = np;
            np = tnp->next;
            if (tnp->child != NULL)          /* Destruct children */
                rnodeDelete (tnp->child);
            if (tnp->value != NULL)
                free (tnp->value);          /* Destruct the text value */
            /*        fprintf (stdout, "Deleting %s\n", tnp->name);*/
            free (tnp);                 /* No - Destruct node */
        }
    }
}

/*
 * rnodeLink
 * Link a new node to the parent.
 * Perform an insertion sort.
 */
static void
rnodeLink (meRegNode *pnp, meRegNode *np)
{
    meRegNode *tnp;

    np->parent = pnp;
    /* Link the new node as a child immediately if possible */
    if (((tnp = pnp->child) == NULL) ||
        (meStrcmp (np->name, tnp->name) < 0))
    {
        pnp->child = np;
        np->next = tnp;
    }
    else
    {
        /* Iterate down the list and add the node */
        for (;;)
        {
            /* Link to the next */
            if (((pnp = tnp->next) == NULL) ||
                (meStrcmp (np->name, pnp->name) < 0))
            {
                np->next = pnp;
                tnp->next = np;
                break;
            }
            else
                tnp = pnp;
        }
    }
}

/*
 * rnodeUnlink
 * Unlink a node from the tree.
 */
static void
rnodeUnlink (meRegNode *np)
{
    /* Link to the parent. */
    meRegNode *pnp;

    pnp = np->parent;                     /* Get the parent out */

    if (pnp->child == np)
        pnp->child = np->next;
    else
    {
        pnp = pnp->child;                /* Descend child to head */
        while (pnp->next != np)         /* Find sibling in list */
        {
            pnp = pnp->next;
            meAssert (pnp != NULL);
        }
        pnp->next = np->next;           /* Unlink the sibling */
    }

    /* Fix up the root and sibling pointers */
    np->next = NULL;
    np->parent = NULL;
}

/*
 * rnodeSet
 * Assign a new value to the node.
 */
static void
rnodeSet (meRegNode *np, meUByte *value)
{
    /* Free off the old value */
    if (np->value != NULL)
        free (np->value);
    /* Assign the new value */
    if ((value == NULL) || (value[0] == '\0'))
        np->value = NULL;
    else
        np->value = meStrdup(value);
}

/*
 * Find a node.
 */

static meRegNode *
rnodeFind (meRegNode *np, meUByte *name)
{
    for (np = np->child; np != NULL; np = np->next)
    {
        meUByte rc, *r = name;             /* Temporary search string */
        meUByte pc, *p = np->name;         /* Temporary registry string */
        
        while(((rc = *r++) == (pc = *p++)) && rc != '\0')
            ;
        
        /* Test for terminal condition */
        if ((rc == '/') && (pc == '\0') && ((rc = *r) != '\0'))
            return (rnodeFind (np, r));
        if((rc == '\0') && (pc == '\0'))
            return np ;
        
        /* If it's bigger than the currnent node we have
         * gone too far */
        if (pc > rc)
            return NULL;
    }
    return NULL;
}


/*
 * Parse the file and import into the registry.
 */
static void
parseFile(meRegNode *rnp, meLine *hlp)
{
    meLine  *lp;                          /* Current line pointer */
    meRegNode *cnp;                         /* Current node pointer */
    meRegNode *lnp;                         /* Last node pointer */
    meUByte *lsp;                         /* Current line string pointer */
    meUByte  buf [meTOKENBUF_SIZE_MAX];              /* Local parse buffer */
    int level = 0;                      /* Nesting depth */
    int needValue = 0;                  /* Need a value flag */

    cnp = rnp;                          /* Current node pointer */
    lnp = NULL;                         /* No last node pointer */
    lp = meLineGetNext(hlp) ;                   /* Initial line pointer */
    lsp = lp->text ;                  /* Initial line string pointer */
    lineNo = 1;                         /* Initial line number */
    
    /* Read in the file until all processed. */
    while (lp != hlp)
    {
        lsp = token(lsp,buf) ;
        switch (buf[0])
        {
        case '\0':
            /* end of line, move to next */
        case ';':
            /* comment to the end of line, move to next */
            lp = meLineGetNext(lp) ;                   /* Initial line pointer */
            lsp = lp->text ;                 /* Initial line string pointer */
            lineNo++ ;                         /* Initial line number */
            break ;
            
        case '"':
            if (needValue)
            {
                /* Link the value to the name */
                meStrrep(&lnp->value,buf+1) ;
                needValue = 0;
            }
            else
            {
                /* Allocate a new node and link to the tree */
                if ((lnp = rnodeFind (cnp,buf+1)) == NULL)
                {
                    if ((lnp = rnodeNew (buf+1)) != NULL)
                        rnodeLink (cnp, lnp);
                }
            }
            break;
        
        case '{':
            if ((lnp == NULL) || needValue)
            {
                mlwrite(MWWAIT,(meUByte *)"[Registry error: unexpected '{' %s:%d]",
                        rnp->value, lineNo);
                return ;
            }
            level++;
            cnp = lnp;
            lnp = NULL;
            break;
        case '}':
            if(needValue || (level <= 0))
            {
                mlwrite(MWWAIT,(meUByte *)"[Registry error: mismatched '}' %s:%d]",
                        rnp->value, lineNo);
                return ;
            }
            cnp = cnp->parent;        /* Back up a level */
            lnp = NULL;
            level--;
            break;
        case '=':
            if(needValue || (lnp == NULL))
            {
                mlwrite(MWWAIT,(meUByte *)"[Registry error: expected <name>=<value> %s:%d]",
                        rnp->value, lineNo);
                return ;
            }
            needValue = 1;
            break;
        default:
            if(!isDigit(buf[0]) || needValue || (lnp == NULL))
            {
                mlwrite(MWWAIT,(meUByte *)"[Registry error: unexpected token \"%s\":%d]",
                        buf,lineNo);
                return ;
            }
            lnp->mode |= meAtoi(buf) ;
        }
    }

    /* End of file. Check for unmatched braces. */
    if (level > 0)
        mlwrite(MWWAIT,(meUByte *)"[Registry error: mismatched '}' at EOF. %d levels open %s:%d]",
                level, rnp->value, lineNo);
}


/*
 * regSave
 * Save the registry back to file
 */
int
regSave(meRegNode *rnp, meUByte *fname, int mode)
{
    meRegNode *rr ;
    meInt ss=meTRUE, level=0, lineCount, charCount ;
    meUInt flags ;

    if(mode & meREGMODE_FROOT)
    {
        /* Find the filename */
        if ((fname == NULL) || (fname[0] == '\0'))
        {
            fname = NULL;
            if(rnp->mode & meREGMODE_FROOT)
                fname = rnp->value;        /* Use the default file name */
            if(fname == NULL)
                return mlwrite(MWABORT|MWPAUSE,(meUByte *)"Registry: No file name specified on save");
        }
        flags = meRWFLAG_WRITE ;
        if(mode & meREGMODE_BACKUP)
            flags |= meRWFLAG_BACKUP ;
        if(mode & (meREGMODE_ACRYPT|meREGMODE_PCRYPT))
        {
            meUInt key[meCRYPT_KEY_SIZE];
            meUByte s1[meBUF_SIZE_MAX];
            int len=0;
            
            if(mode & meREGMODE_PCRYPT)
            {
                if(meGetString((meUByte *) "Reg password",MLNOHIST|MLHIDEVAL,0,(meUByte *) s1,meSBUF_SIZE_MAX) <= 0)
                    return meABORT;
                len = meStrlen(s1)+1;
            }
            if(mode & meREGMODE_ACRYPT)
            {
                meStrcpy(s1+len,getFileBaseName(fname));
                len += meStrlen(s1+len)+1;
                if(meUserName == NULL)
                    s1[len++] = '\0';
                else
                {
                    meStrcpy(s1+len,meUserName);
                    len += meStrlen(meUserName)+1;
                }
            }
            meCryptKeyEncode(key,s1,len);
            meCryptInit(key);
            flags |= meRWFLAG_CRYPT;
        }
        /* Open the file */
        if(ffWriteFileOpen(&meiow,fname,flags,NULL) <= 0)
            return meABORT;
        
        /* Add a recognition string to the header */
        if(!(mode & (meREGMODE_ACRYPT|meREGMODE_PCRYPT)))
            ss = ffWriteFileWrite(&meiow,12,(meUByte *) ";-!- erf -!-",1) ;
    }
    else
    {
        mode &= ~(meREGMODE_ACRYPT|meREGMODE_PCRYPT);
        lineCount = 0;
        charCount = 0;
    }
    
    /* Recurse the children of the node and write to file */
    rr = rnp->child;
    while((ss > 0) && (rr != NULL))
    {
        meUByte buff[4096];
        int  len;
        /* Print the node */
        if((len = level) != 0)
            memset(buff,' ',len);
        buff[len++] = '"';
        len = expandexp(-1,rr->name,4096-11,len,buff,-1,NULL,meEXPAND_BACKSLASH|meEXPAND_FFZERO|meEXPAND_PRINTABLE);
        buff[len++] = '"' ;
        if (rr->mode & (meREGMODE_HIDDEN|meREGMODE_INTERNAL))
        {
            buff[len++] = ' ';
            buff[len++] = '0' + (rr->mode & (meREGMODE_HIDDEN|meREGMODE_INTERNAL));
        }
        if (rr->value != NULL)
        {
            buff[len++] = ' ';
            buff[len++] = '=';
            buff[len++] = ' ';
            buff[len++] = '"';
            len = expandexp(-1,rr->value,4096-4,len,buff,-1,NULL,meEXPAND_BACKSLASH|meEXPAND_FFZERO|meEXPAND_PRINTABLE) ;
            buff[len++] = '"';
        }
        /* write open '{' if it has children */
        if (rr->child != NULL)
        {
            buff[len++] = ' ';
            buff[len++] = '{';
        }
        if((mode & meREGMODE_FROOT) == 0)
        {
            buff[len] = '\0';
            if((ss = addLine(frameCur->windowCur->dotLine,buff)) <= 0)
                break ;
            lineCount += ss;
            charCount += len + 1;
        }
        else if((ss = ffWriteFileWrite(&meiow,len,buff,1)) <= 0)
            break ;
        /* Descend child */
        if (rr->child != NULL)
        {
            rr = rr->child;
            level++;
            continue;
        }
        /* Ascend the tree */
        for (;;)
        {
            /* Move to sibling */
            if (rr->next != NULL)
            {
                rr = rr->next;
                break;
            }
            
            if (rr->parent != NULL)
            {
                if (--level < 0)
                {
                    rr = NULL;
                    break;
                }
                rr = rr->parent;
                /* as we are assending the tree, at least the first 'level'
                 * number of chars in buffer must be ' 's so just splat in the '}'
                 */
                len = level ;
                buff[len++] = '}' ;
                if((mode & meREGMODE_FROOT) == 0)
                {
                    buff[len] = '\0' ;
                    if((ss = addLine(frameCur->windowCur->dotLine,buff)) <= 0)
                        break ;
                    lineCount += ss ;
                    charCount += len + 1 ;
                }
                else if((ss = ffWriteFileWrite(&meiow,len,buff,1)) <= 0)
                    break ;
            }
        }
    }
    
    if(mode & meREGMODE_FROOT)
    {
        if(ffWriteFileClose(&meiow,meRWFLAG_WRITE,NULL) <= 0)
            return meABORT ;
        rnp->mode &= ~meREGMODE_CHANGE;
    }
    else
    {
        meWindow *wp, *cwp=frameCur->windowCur;
        level = cwp->dotLineNo;
        cwp->buffer->lineCount += lineCount;
        meFrameLoopBegin();
        for(wp=loopFrame->windowList; wp!=NULL; wp=wp->next)
        {
            if (wp->buffer == cwp->buffer)
            {
                if(wp->dotLineNo >= level)
                    wp->dotLineNo += lineCount;
                if(wp->markLineNo >= level)
                    wp->markLineNo += lineCount;
                wp->updateFlags |= WFMAIN|WFMOVEL;
            }
        }
        meFrameLoopEnd();
        cwp->dotOffset = 0;
#if MEOPT_UNDO
        meUndoAddInsChars(charCount);
#endif
        if(ss <= 0)
            return mlwrite(MWABORT|MWPAUSE,(meUByte *)"[Failed to write registry %s]",fname);
    }
    return meTRUE;
}

/*
 * regset
 * Assign a new value and key to the registry item
 */
meRegNode *
regSet (meRegNode *sroot, meUByte *subkey, meUByte *value)
{
    /* Find or create the specified node tree */
    if (sroot == NULL)
        sroot = &root;

    if (subkey != NULL)
    {
        meUByte buf[meBUF_SIZE_MAX];
        meRegNode *tnp;
        meUByte *p;

        /* Determine if this is a ralative or absolute node */
        if (*subkey == '/')             /* Ignore leading '/' */
            subkey++;

        while (*subkey != '\0')
        {
            /* Incrementaly add new nodes */
            for (p = buf; (*p = *subkey) != '\0'; p++)
            {
                subkey++;
                if (*p == '/')
                {
                    *p = '\0';
                    break;
                }
            }

            /* Check for NULL name - simply ignore */
            if (buf [0] == '\0')
                continue;
            /* Construct a new node if we cannot find an existing node */
            if ((tnp = rnodeFind (sroot, buf)) == NULL)
            {
                if((tnp = rnodeNew (buf)) == NULL)
                    return NULL ;
                rnodeLink (sroot, tnp);
            }
            sroot = tnp;                /* Move to the next node */
        }
    }
    /* Assign the new value !! Note that the root node hodes the
     * filename */
    if (sroot != &root)
        rnodeSet (sroot, value);
    return sroot;
}

/*
 * regFind
 * Find the node in the registry
 */
meRegNode *
regFind (meRegNode *sroot, meUByte *subkey)
{
    if (sroot == NULL)                  /* A NULL root is meaningless */
        sroot = &root;
    if (subkey != NULL)
    {
        if (*subkey == '/')
            subkey++;
        if (*subkey != '\0')
            sroot = rnodeFind (sroot, subkey) ;
    }
    return sroot;
}

/*
 * vregFind
 * Find the node in the registry
 */
#ifdef _STDARG
meRegNode *
vregFind (meRegNode *sroot, meUByte *fmt, ...)
#else
meRegNode *
vregFind (meRegNode *sroot, meUByte *fmt)
#endif
{
    if (sroot == NULL)                  /* A NULL root is meaningless */
        sroot = &root;
    if (fmt != NULL)
    {
#ifdef _STDARG
        va_list	ap;
#else
        register char **ap;
#endif
        meUByte buf [meBUF_SIZE_MAX];
#ifdef _STDARG
        va_start(ap,fmt);
#else
        ap = &fmt;
#endif
        if (*fmt == '/')
            fmt++;
        vsprintf((char *)buf, (char *) fmt, ap);
        if (buf[0] != '\0')
            sroot = rnodeFind (sroot, buf);
#ifdef _STDARG
        va_end (ap);
#endif

    }
    return sroot;
}


/* Note the return value for this is:
 * meABORT - there was a major failure (i.e. couldn't open the file)
 * meFALSE - user quit
 * meTRUE  - succeded
 * this is used by the exit function which ignore the major failures
 */
static int
regTestSave(meRegNode *sroot, int flags)
{
    /* if its not a file or not  changed or the  changes  are to be  discarded
     * then nothing to do */
    if(!(sroot->mode & meREGMODE_FROOT) || !(sroot->mode & meREGMODE_CHANGE) ||
       (sroot->mode & meREGMODE_DISCARD))
        return meTRUE ;
    
    if((flags & 0x01) && !(sroot->mode & meREGMODE_AUTO))
    {
        meUByte prompt[meBUF_SIZE_MAX] ;
        int ret ;
        meStrcpy(prompt,(sroot->value != NULL) ? sroot->value:sroot->name) ;
        meStrcat(prompt,": Save registry") ;
        if((ret = mlyesno(prompt)) == meABORT)
        {
            ctrlg(meFALSE,1) ;
            return meFALSE ;
        }
        if(ret == meFALSE)
            return meTRUE ;
    }
    return regSave(sroot,NULL,sroot->mode) ;
}

/*
 * regDelete
 * Delete a node in the registry.
 */
int
regDelete (meRegNode *sroot)
{
    /* Test and save first as user may abort */
    if(regTestSave(sroot,1) <= 0)
        return meABORT ;
    /* Unlink the node */
    if (sroot->parent != NULL)
        rnodeUnlink (sroot);
    rnodeDelete(sroot);
    return meTRUE ;
}

/*
 * regRead
 * Open the specified registory.
 * Mode options are:-
 *
 * default        - Exists in memory
 * meREGMODE_RELOAD - Delete the existing file,
 * meREGMODE_MERGE  - Merge the existing file.
 */
meRegNode *
regRead(meUByte *rname, meUByte *fname, int mode)
{
    meLine hlp, *lp;
    meUByte *fn;
    meRegNode *rnp;                         /* Root node pointer */
    meUInt flags;
    
    /* Find the registry entry */
    if(*rname == '/')
        rname++;
    if ((rnp = rnodeFind (&root, rname)) != NULL)
    {
        /* if not merging or reloading then we've can use the existing node */
        if (!(mode & (meREGMODE_MERGE|meREGMODE_RELOAD)))
            goto finished;
        fn = rnp->value;
    }
    else
        fn = NULL ;
    
    if(mode & meREGMODE_FROOT)
    {
        /* find the registry file */
        /* have we been given a valid file name ? */
        if ((fname != NULL) && (fname [0] != '\0'))
        {
            meUByte filename[meBUF_SIZE_MAX] ;	/* Filename */
            if(fileLookup(fname,extRegCnt,extRegLst,meFL_CHECKPATH|meFL_USESRCHPATH,filename))
                fn = meStrdup(filename);
            else
                fn = meStrdup(fname);
        }
        /* else use the old file name (if there is one) else fail */
        else if(fn == NULL)
        {
            mlwrite(MWABORT|MWWAIT,(meUByte *)"[No file name given to load]") ;
            return NULL;
        }
    
        /* Load in the registry file */
        flags = meRWFLAG_SILENT;
        hlp.next = &hlp;
        hlp.prev = &hlp;
        if(mode & (meREGMODE_ACRYPT|meREGMODE_PCRYPT))
        {
            meUInt key[meCRYPT_KEY_SIZE];
            meUByte s1[meBUF_SIZE_MAX];
            int len=0;
            
            if(mode & meREGMODE_PCRYPT)
            {
                if(meGetString((meUByte *) "Reg password",MLNOHIST|MLHIDEVAL,0,(meUByte *) s1,meSBUF_SIZE_MAX) <= 0)
                    return NULL;
                len = meStrlen(s1)+1;
            }
            if(mode & meREGMODE_ACRYPT)
            {
                meStrcpy(s1+len,getFileBaseName(fn));
                len += meStrlen(s1+len)+1;
                if(meUserName == NULL)
                    s1[len++] = '\0';
                else
                {
                    meStrcpy(s1+len,meUserName);
                    len += meStrlen(meUserName)+1;
                }
            }
            meCryptKeyEncode(key,s1,len);
            meCryptInit(key);
            flags |= meRWFLAG_CRYPT;
        }
        if((ffReadFile(&meior,fn,flags|meRWFLAG_READ,NULL,&hlp,0,0,0) == meABORT) && !(mode & meREGMODE_CREATE))
        {
            mlwrite(MWABORT|MWWAIT,(meUByte *)"[Cannot load registry file %s]",fname);
            return NULL;
        }
        lp = &hlp ;
    }
    else
        lp = frameCur->windowCur->buffer->baseLine;
    
    if ((rnp != NULL) && (mode & meREGMODE_RELOAD))
    {
        /* Want to replace with new one. so delete old */
        if(fn == rnp->value)
            rnp->value = NULL;
        rnodeUnlink(rnp);
        rnodeDelete(rnp);
        rnp = NULL;
    }
    /* Construct the node for the file node */
    if (rnp == NULL)
    {
        /* Construct the node */
        if((rnp = regSet(&root, rname, NULL)) == NULL)
            return NULL ;
        rnp->mode = 0;
    }
    /* set the new file name */
    if(rnp->value != fn)
    {
        meNullFree(rnp->value);
        rnp->value = fn;
    }
    /* Now parse the registry file */
    parseFile(rnp,lp);
    
    if(mode & meREGMODE_FROOT)
        meLineLoopFree(lp,0);

finished:
    rnp->mode |= mode & meREGMODE_STORE_MASK;
    return rnp;
}


/***************************************************************************
 *
 * Command interface functions
 *
 ***************************************************************************/
/*
 * regDecodeMode
 * Decode the registry mode.
 */
static meUByte meRegModeList[]="!hifubadypmrc?g" ;
static int
regDecodeMode (int *modep, meUByte *modeStr)
{
    int mode = *modep;                  /* The mode */
    int ii, jj ;                        /* Local loop counter */
    meUByte *ss, cc ;
    
    /* Get the mode out */
    if (modeStr != NULL)
    {
        for (ii = 0; (cc=modeStr[ii]) != '\0'; ii++)
        {
            if((ss=meStrchr(meRegModeList,toLower(cc))) == NULL)
                return mlwrite(MWABORT|MWPAUSE,(meUByte *)"Illegal Registry mode [%c]",cc) ;
            jj = (1 << ((int)(ss - meRegModeList))) ;
            if(isUpper(cc))
                mode &= ~jj ;
            else
                mode |=  jj ;
        }
    }
    *modep = mode;
    return meTRUE;
}

/*
 * readRegistry
 * Read a file into the registry.
 */
int
readRegistry(int f, int n)
{
    meUByte filebuf [meBUF_SIZE_MAX];
    meUByte rootbuf [32];
    meUByte modebuf [10];
    int mode = 0;

    /* Get the input from the command line */
    if ((meGetString((meUByte *)"Read registry root",0, 0, rootbuf, 32) == meABORT) ||
        (((n & 4) == 0) &&
         (meGetString((meUByte *)"Registry file",MLFILECASE, 0, filebuf, meBUF_SIZE_MAX) == meABORT)) ||
        (meGetString((meUByte *)"File Mode", 0, 0, modebuf, 10) == meABORT))
        return meABORT;
    
    /* Get the mode out */
    if (regDecodeMode (&mode, modebuf) == meABORT)
        return meABORT;

    mode &= ~meREGMODE_FROOT ;
    if((n & 4) == 0)
        mode |= meREGMODE_FROOT ;
    
    /* Read the file */
    if(regRead(rootbuf, filebuf, mode) == NULL)
        return meFALSE;
    return meTRUE;
}

/*
 * findRegistryName
 * Find the absolute node name associated with a node.
 * Use tail recursion to do the expansion
 */
static meUByte *
findRegistryName (meRegNode *rnp, meUByte *p)
{
    meUByte *q;

    if (rnp->parent != NULL)
    {
        q = findRegistryName (rnp->parent, p);
        /* Add a name separator and concatinate the name */
        *q++ = '/';
    }
    else
        q = p;
    p = rnp->name;
    while ((*q++ = *p++) != '\0');
    return q-1;
}

/*
 * markRegistry
 * Mark a node in the registry.
 */
int
markRegistry (int f, int n)
{
    meUByte rootbuf [meBUF_SIZE_MAX];
    meUByte modebuf [10];
    int mode;
    meRegNode *rnp;

    /* Get the input from the command line */
    if ((meGetString((meUByte *)"Registry node",0, 0, rootbuf, meBUF_SIZE_MAX) == meABORT) ||
        (meGetString((meUByte *)"Mark modes", 0, 0, modebuf, 10) == meABORT))
        return meABORT;

    /* Find the node */
    if ((rnp = regFind (&root, rootbuf)) == NULL)
        return mlwrite(MWCLEXEC|MWABORT,(meUByte *)"[Cannot find node %s]",rootbuf);


    /* Find the integer offset if there is one. */
    if (f)
    {
        meRegNode *tnp, *nnp;

        tnp = rnp;
        f = n ;
        while((--n >= 0) && (tnp != NULL))
        {
            if(!(tnp->mode & (meREGMODE_HIDDEN|meREGMODE_INTERNAL)) &&
               ((nnp = tnp->child) != NULL))
                tnp = nnp ;
            else
                nnp = tnp->next ;
            
            while((tnp != NULL) && ((nnp == NULL) || (nnp->mode & meREGMODE_INVISIBLE)))
            {
                if(nnp == NULL)
                {
                    /* Make sure we are not back at the root */
                    if((tnp = tnp->parent) == rnp)
                        tnp = NULL;
                    else
                        nnp = tnp->next ;
                }
                else
                    nnp = nnp->next ;
            }
            tnp = nnp ;
        }

        /* tnp is the current node. Change the node */
        if ((rnp = tnp) == NULL)
            return mlwrite (MWCLEXEC|MWABORT,(meUByte *)"[Cannot locate node %d]", f);
    }

    /* Get the mode out */
    mode = rnp->mode;
    if (regDecodeMode (&mode, modebuf) == meABORT)
        return meABORT;
    rnp->mode = mode & meREGMODE_STORE_MASK ;

    /* If this is a query then return the path of the current node in
     * $result. */
    if (mode & meREGMODE_QUERY)
        findRegistryName (rnp, resultStr);
    else if (mode & meREGMODE_GETMODE)
    {
        meUByte *ss=resultStr ;
        int ii ;
        for(ii=0 ; ii<8 ; ii++)
            if(rnp->mode & (1<<ii))
                *ss++ = meRegModeList[ii] ;
        *ss = '\0' ;
    }

    return meTRUE;
}

/*
 * saveRegistry
 * Save the registry to a file.
 * 
 * Note the return value for this is:
 * meABORT - there was a major failure (i.e. couldn't open the file)
 * meFALSE - user quit
 * meTRUE  - succeded
 * this is used by the exit function which ignore the major failures
 */
int
saveRegistry (int f, int n)
{
    meUByte filebuf [meBUF_SIZE_MAX];
    meUByte rootbuf [128];
    meRegNode *rnp;
    meInt mode ;
    
    if(n & 0x2)
    {
        rnp = root.child ;
        while (rnp != NULL)
        {
            if((f=regTestSave(rnp,n)) <= 0)
                return f ;
            rnp = rnp->next ;
        }
        return meTRUE ;
    }
    
    /* Get the input from the command line */
    if(meGetString((meUByte *)"Save registry root",0, 0, rootbuf, 128) == meABORT)
        return meFALSE;
    
    /* Find the root */
    if((rnp = regFind (&root, rootbuf)) == NULL)
        return mlwrite(MWCLEXEC|MWABORT,(meUByte *)"[Cannot find node %s]",rootbuf);
    
    mode = rnp->mode & ~meREGMODE_FROOT ;
    if((n & 4) == 0)
    {
        if(rnp->mode & meREGMODE_FROOT)
            meStrcpy(filebuf,rnp->value) ;
        else
            filebuf[0] = '\0' ;
    
        /* Get the filename */
        if(meGetString((meUByte *)"Registry file",MLFILECASE|MLFILECASE, 0, filebuf, meBUF_SIZE_MAX) <= 0)
            return meFALSE ;
        mode |= meREGMODE_FROOT ;
    }
    return regSave(rnp,filebuf,mode) ;
}

/* returns true if any registry node needs saving */
int
anyChangedRegistry(void)
{
    meRegNode *rnp;
    
    rnp = root.child ;
    while (rnp != NULL)
    {
        if((rnp->mode & (meREGMODE_FROOT|meREGMODE_CHANGE|meREGMODE_DISCARD)) == (meREGMODE_FROOT|meREGMODE_CHANGE))
            return meTRUE ;
        rnp = rnp->next ;
    }
    return meFALSE ;
}

/*
 * deleteRegistry
 * Delete the registry entry.
 */

int
deleteRegistry (int f, int n)
{
    meUByte rootbuf [meBUF_SIZE_MAX];
    meRegNode *rnp;

    if (meGetString((meUByte *)"Registry Path", 0, 0, rootbuf, meBUF_SIZE_MAX) == meABORT)
        return meABORT;
    if ((rnp = regFind (&root, rootbuf)) == NULL)
        return mlwrite(MWCLEXEC|MWABORT,(meUByte *)"[Cannot find node %s]",rootbuf);
    return regDelete (rnp);
}

/*
 * setRegistry
 * Assign a new value to the registry
 */
int
setRegistry (int f, int n)
{
    meUByte buf1[meBUF_SIZE_MAX], *name ;
    meUByte buf2[meBUF_SIZE_MAX] ;
    meRegNode *rnp, *pnp, *nnp ;

    /* Get the arguments */
    if(meGetString((meUByte *)"Registry Path", 0, 0, buf1, meBUF_SIZE_MAX) == meABORT)
        return meABORT;
    if(n & 0x02)
    {
        if(((rnp = regFind(&root,buf1)) == NULL) || (rnp == &root))
            return mlwrite(MWCLEXEC|MWABORT,(meUByte *)"[Cannot find node %s]",buf1);
        
        /* setting the name of the node, not the value */
        if(meGetString((meUByte *)"name", 0, 0, buf2, meBUF_SIZE_MAX) == meABORT)
            return meABORT;
        if(((name=meStrrchr(buf2,'/')) != NULL) && (name[1] == '\0'))
        {
            *name = '\0' ;
            name = meStrrchr(buf2,'/') ;
        }
        if(name != NULL)
        {
            *name++ = '\0' ;
            if((pnp = regFind(&root,buf2)) == NULL)
                return mlwrite(MWCLEXEC|MWABORT,(meUByte *)"[Cannot find node %s]",buf2) ;
            nnp = pnp ;
            do {
                if(nnp == rnp)
                    return mlwrite(MWCLEXEC|MWABORT,(meUByte *)"[Cannot move node to itself or one of its children]") ;
            } while((nnp=nnp->parent) != NULL) ;
        }
        else
        {
            name = buf2 ;
            pnp = rnp->parent ;
        }
        if((pnp != rnp->parent) || meStrcmp(name,rnp->name))
        {
            if((name[0] == '\0') || (regFind(pnp,name) != NULL))
                return mlwrite(MWCLEXEC|MWABORT,(meUByte *)"[Cannot set registry node]");
            if((nnp = rnodeNew(name)) == NULL)
                return meABORT;
            rnodeUnlink(rnp) ;
            nnp->value = rnp->value ;
            nnp->child = rnp->child ;
            nnp->mode = rnp->mode ;
            meFree(rnp) ;
            rnodeLink(pnp,nnp) ;
            rnp = nnp->child ;
            while(rnp != NULL)
            {
                rnp->parent = nnp ;
                rnp = rnp->next ;
            }
        }
    }
    else
    {
        if(meGetString((meUByte *)"Value", 0, 0, buf2, meBUF_SIZE_MAX) == meABORT)
            return meABORT;

        /* Assigns the new value */
        if(regSet(&root,buf1,buf2) == NULL)
            return mlwrite(MWCLEXEC|MWABORT,(meUByte *)"[Cannot set registry node]");
    }
    return meTRUE;
}

/*
 * findRegistry
 * Find a key in the registry by indexing
 */
int
findRegistry (int f, int n)
{
    meUByte rootbuf [meBUF_SIZE_MAX];
    meUByte valbuf [12];
    int index;
    meRegNode *rnp ;

    /* Get the arguments */
    if((meGetString((meUByte *)"Registry Path", 0, 0, rootbuf, meBUF_SIZE_MAX) == meABORT) ||
       (meGetString((meUByte *)"Index", 0, 0, valbuf, 12) == meABORT))
        return meABORT;
    index = meAtoi (valbuf);

    /* Assigns the new value */
    if((rnp = regFind (&root, rootbuf)) == NULL)
        return mlwrite(MWCLEXEC|MWABORT,(meUByte *)"[Cannot find node %s]",rootbuf);
    /* Find the node that is indexed */
    f = index ;
    rnp = rnp->child;
    while((--index >= 0) && rnp)
        rnp = rnp->next;
    if(rnp == NULL)
    {
        resultStr [0] ='\0';
        return mlwrite (MWCLEXEC|MWABORT,(meUByte *)"Cannot find node at index %d", f);
    }
    meStrncpy(resultStr, rnp->name, meBUF_SIZE_MAX-1);
    resultStr[meBUF_SIZE_MAX-1] = '\0';
    return meTRUE;
}

/*
 * listRegistry
 * List the contents of the registry.
 */
int
listRegistry (int f, int n)
{
    meBuffer *bp;                         /* Buffer pointer */
    meWindow *wp;                         /* Window associated with buffer */
    meRegNode *rnp, *cnp, *nnp ;          /* Draw the nodes */
    meUByte vstrbuf [meBUF_SIZE_MAX];     /* Vertical string buffer */
    meUByte *bn, buf[meBUF_SIZE_MAX*2];   /* Working line buffer */
    int level = 0;                        /* Depth in the registry */
    int len;                              /* Length of the string */

    rnp = &root;
    if((n & 0x02) != 0)
    {
        if(meGetString((meUByte *)"Registry Path",0, 0, buf, meBUF_SIZE_MAX) == meABORT)
            return meABORT;

        /* Find the node */
        if((rnp = regFind(rnp,buf)) == NULL)
            return mlwrite(MWCLEXEC|MWABORT,(meUByte *)"[Cannot find node %s]",buf);
    }
    
    if((n & 0x01) == 0)
    {
        /* prompt for and get the new buffer name */
        if((len = getBufferName((meUByte *) "Buffer", 0, 0, buf)) <= 0)
            return len ;
        bn = buf ;
    }
    else
        bn = BregistryN ;
    
    /* Find the buffer and vapour the old one */
    if((wp = meWindowPopup(NULL,bn,BFND_CREAT|BFND_CLEAR|WPOP_USESTR,NULL)) == NULL)
        return meFALSE;
    bp = wp->buffer;                   /* Point to the buffer */

    /* Recurse the children of the node and write to file */
    do
    {
        /* get the current node's first drawn child */
        cnp = rnp->child;
        while((cnp != NULL) && (cnp->mode & meREGMODE_INVISIBLE))
            cnp = cnp->next;

        /* get the current node's next drawn sibling */
        nnp = rnp->next;
        while((nnp != NULL) && (nnp->mode & meREGMODE_INVISIBLE))
            nnp = nnp->next;

        /* Add continuation bars if we are at a child level */
        if((len = level) != 0)
            meStrncpy(buf,vstrbuf,len);
        
        /* Add connection bars for siblings */
        if(level && (nnp != NULL))
            buf[len++] = boxChars[BCNES] ;
        else
            buf[len++] = boxChars[BCNE];
        
        /* Add continuation barss for children */
        if (rnp->mode & meREGMODE_INTERNAL)
            buf[len++] = '!';
        else if (cnp == NULL)
            buf[len++] = boxChars[BCEW];
        else if (rnp->mode & meREGMODE_HIDDEN)
            buf[len++] = '+';
        else
            buf[len++] = '-';
        buf[len++] = ' ';
        
        /* Add the name of the node */
        buf[len++] = '"';
        len = expandexp(-1,rnp->name,(meBUF_SIZE_MAX*2)-7,len,buf,-1,NULL,meEXPAND_BACKSLASH|meEXPAND_FFZERO) ;
        buf[len++] = '"';
        
        /* Add the value */
        if ((rnp->value != NULL) && !(rnp->mode & meREGMODE_INTERNAL))
        {
            buf[len++] = ' ';
            buf[len++] = '=';
            buf[len++] = ' ';
            buf[len++] = '"';
            len = expandexp(-1,rnp->value,(meBUF_SIZE_MAX*2)-2,len,buf,-1,NULL,meEXPAND_BACKSLASH|meEXPAND_FFZERO) ;
            buf[len++] = '"';
        }
        /* Add the string to the print buffer */
        buf[len] = '\0';
        addLineToEob(bp,buf);
        /* Descend child */
        if((cnp != NULL) && !(rnp->mode & (meREGMODE_HIDDEN|meREGMODE_INTERNAL)))
        {
            vstrbuf[level] = (level && (nnp != NULL)) ? boxChars[BCNS] : ' ';
            level++;
            rnp = cnp;
            continue;
        }
        
        /* Ascend the tree */
        while((nnp == NULL) && (--level >= 0) && ((rnp = rnp->parent) != NULL))
        {
            /* Move to next drawn sibling */
            nnp = rnp->next;
            while((nnp != NULL) && (nnp->mode & meREGMODE_INVISIBLE))
                nnp = nnp->next;
        }
        rnp = nnp ;
    } while((level > 0) && (rnp != NULL));

    /* Set up the buffer for display */
    bp->dotLine = meLineGetNext(bp->baseLine);
    bp->dotOffset = 0;
    bp->dotLineNo = 0;
    meModeSet(bp->mode,MDVIEW);
    meModeClear(bp->mode,MDAUTOSV);
    meModeClear(bp->mode,MDUNDO);
    resetBufferWindows(bp);
    return meTRUE;
}

#ifdef _ME_FREE_ALL_MEMORY
void regFreeMemory(void)
{
    rnodeDelete (root.child);
}
#endif

#endif
