/* -*- C -*- ****************************************************************
 *
 *  System        : MicroEmacs Jasspa Distribution
 *  Module        : registry.c
 *  Synopsis      : Internal registry support routines
 *  Created By    : Jon Green
 *  Created       : 26/03/1998
 *  Last Modified : <010605.0936>
 *
 *  Description
 *
 *  Notes
 *
 ****************************************************************************
 * 
 * Copyright (c) 1998-2000 Jon Green
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
 * Jon Green         jon@jasspa.com
 *
 ****************************************************************************/

#define __REGISTRYC                     /* Define the name of the file */

#include "emain.h"

#if REGSTRY

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
static RNODE root =
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
static RNODE *
rnodeNew (uint8 *name)
{
    RNODE *np;                          /* Pointer to the node */
    int len;                            /* Length of the string */

    /* Get the length of the node name */
    len = ((name == NULL) ? -1 : meStrlen (name));

    /* Allocate space for the node and copy in the node name */
    if ((np = (RNODE *) malloc (sizeof (RNODE) + len)) == NULL)
        return NULL;
    memset (np, 0, sizeof (RNODE));
    meStrcpy (np->name, name);
    return (np);
}

/*
 * rnodeDel
 * Recursively delete a node.
 */
static RNODE *
rnodeDelete (RNODE *np)
{
    /* Recursively delete the tree */
    while (np != NULL)
    {
        RNODE *tnp;                     /* Temporary node pointer */

        tnp = np;
        np = tnp->sblg;
        if (tnp->chld != NULL)          /* Destruct children */
            rnodeDelete (tnp->chld);
        if (tnp->value != NULL)
            free (tnp->value);          /* Destruct the text value */
/*        fprintf (stdout, "Deleting %s\n", tnp->name);*/
        if (tnp->name != '\0')          /* Root node ?? */
        {
            free (tnp);                 /* No - Destruct node */
        }
        else
            break;
    }
    return np;                          /* Return NULL pointer */
}

/*
 * rnodeLink
 * Link a new node to the parent.
 * Perform an insertion sort.
 */
static void
rnodeLink (RNODE *pnp, RNODE *np)
{
    RNODE *tnp;

    np->prnt = pnp;
    /* Link the new node as a child immediately if possible */
    if (((tnp = pnp->chld) == NULL) ||
        (meStrcmp (np->name, tnp->name) < 0))
    {
        pnp->chld = np;
        np->sblg = tnp;
    }
    else
    {
        /* Iterate down the list and add the node */
        for (;;)
        {
            /* Link to the next */
            if (((pnp = tnp->sblg) == NULL) ||
                (meStrcmp (np->name, pnp->name) < 0))
            {
                np->sblg = pnp;
                tnp->sblg = np;
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
rnodeUnlink (RNODE *np)
{
    /* Link to the parent. */
    RNODE *pnp;

    pnp = np->prnt;                     /* Get the parent out */

    if (pnp->chld == np)
        pnp->chld = np->sblg;
    else
    {
        pnp = pnp->chld;                /* Descend child to head */
        while (pnp->sblg != np)         /* Find sibling in list */
        {
            pnp = pnp->sblg;
            meAssert (pnp != NULL);
        }
        pnp->sblg = np->sblg;           /* Unlink the sibling */
    }

    /* Fix up the root and sibling pointers */
    np->sblg = NULL;
    np->prnt = NULL;
}

/*
 * rnodeSet
 * Assign a new value to the node.
 */
static void
rnodeSet (RNODE *np, uint8 *value)
{
    /* Free off the old value */
    if (np->value != NULL)
        free (np->value);
    /* Assign the new value */
    if ((value == NULL) || (value[0] == '\0'))
        np->value = NULL;
    else
        np->value = meStrdup (value);
}

/*
 * Find a node.
 */

static RNODE *
rnodeFind (RNODE *np, uint8 *name)
{
    for (np = np->chld; np != NULL; np = np->sblg)
    {
        uint8 rc, *r = name;             /* Temporary search string */
        uint8 pc, *p = np->name;         /* Temporary registry string */
        
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
parseFile (RNODE *rnp, LINE *hlp)
{
    LINE  *lp;                          /* Current line pointer */
    RNODE *cnp;                         /* Current node pointer */
    RNODE *lnp;                         /* Last node pointer */
    uint8 *lsp;                         /* Current line string pointer */
    uint8  buf [TOKENBUF];              /* Local parse buffer */
    int level = 0;                      /* Nesting depth */
    int needValue = 0;                  /* Need a value flag */

    cnp = rnp;                          /* Current node pointer */
    lnp = NULL;                         /* No last node pointer */
    lp = lforw(hlp) ;                   /* Initial line pointer */
    lsp = lp->l_text ;                  /* Initial line string pointer */
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
            lp = lforw(lp) ;                   /* Initial line pointer */
            lsp = lp->l_text ;                 /* Initial line string pointer */
            lineNo++ ;                         /* Initial line number */
            break ;
            
        case '"':
            if (needValue)
            {
                /* Link the value to the name */
                if (lnp->value)
                    free (lnp->value);
                lnp->value = meStrdup (buf+1);
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
                mlwrite(MWWAIT,(uint8 *)"[Registry unexpected '{' %s:%d]",
                        rnp->value, lineNo);
            else
            {
                level++;
                cnp = lnp;
                lnp = NULL;
            }
            break;
        case '}':
            if (!needValue && (level > 0))
            {
                cnp = cnp->prnt;        /* Back up a level */
                lnp = NULL;
                level--;
            }
            else
                mlwrite(MWWAIT,(uint8 *)"[Registry mismatched '}' %s:%d]",
                        rnp->value, lineNo);
            break;
        case '=':
            if ((needValue == 0) && (lnp != NULL))
                needValue = 1;
            else
                mlwrite(MWWAIT,(uint8 *)"[Registry syntax, expected <name>=<value> %s:%d]",
                        rnp->value, lineNo);
            break;
        default:
            if (isDigit(buf[0]) && (needValue == 0) && (lnp != NULL))
                lnp->mode |= meAtoi(buf) ;
            else
                mlwrite(MWWAIT,(uint8 *)"[Registry syntax, unexpected token \"%s\":%d]",
                        buf,lineNo);
        }
    }

    /* End of file. Check for unmatched braces. */
    if (level > 0)
        mlwrite(MWWAIT,(uint8 *)"[Registry mismatched '}' at EOF. %d levels open %s:%d]",
                level, rnp->value, lineNo);
}


/*
 * regSave
 * Save the registry back to file
 */
int
regSave (RNODE *rnp, uint8 *fname)
{
    int   ss, level = 0;

    /* Check the root pointer */
    if (rnp == NULL)
        return mlwrite(MWABORT|MWPAUSE,(uint8 *)"Registry: No node specified on export");

    /* Find the filename */
    if ((fname == NULL) || (fname[0] == '\0'))
    {
        fname = NULL;
        if (rnp->mode & REGMODE_FROOT)
            fname = rnp->value;        /* Use the default file name */
        if (fname == NULL)
            return mlwrite(MWABORT|MWPAUSE,(uint8 *)"Registry: No file name specified on save");
    }

    /* Open the file */
    if((ss=ffWriteFileOpen(fname,(rnp->mode & REGMODE_BACKUP) ? (meRWFLAG_WRITE|meRWFLAG_BACKUP):meRWFLAG_WRITE,NULL)) == TRUE)
    {
        RNODE *rr ;
        
        /* Add a recognition string to the header */
        ss = ffWriteFileWrite(12,(uint8 *) ";-!- erf -!-",1) ;

        /* Recurse the children of the node and write to file */
        rr = rnp->chld ;
        while ((ss == TRUE) && (rr != NULL))
        {
            uint8 buff[4096] ;
            int  len ;
            /* Print the node */
            if((len = level) != 0)
                memset(buff,' ',len) ;
            buff[len++] = '"' ;
            len = expandexp(-1,rr->name,4096-11,len,buff,-1,NULL,meEXPAND_BACKSLASH|meEXPAND_FFZERO) ;
            buff[len++] = '"' ;
            if (rr->mode & (REGMODE_HIDDEN|REGMODE_INTERNAL))
            {
                buff[len++] = ' ' ;
                buff[len++] = '0' + (rr->mode & (REGMODE_HIDDEN|REGMODE_INTERNAL)) ;
            }
            if (rr->value != NULL)
            {
                buff[len++] = ' ' ;
                buff[len++] = '=' ;
                buff[len++] = ' ' ;
                buff[len++] = '"' ;
                len = expandexp(-1,rr->value,4096-4,len,buff,-1,NULL,meEXPAND_BACKSLASH|meEXPAND_FFZERO) ;
                buff[len++] = '"' ;
            }
            /* write open '{' if it has children */
            if (rr->chld != NULL)
            {
                buff[len++] = ' ' ;
                buff[len++] = '{' ;
            }
            if((ss = ffWriteFileWrite(len,buff,1)) != TRUE)
                break ;
            
            /* Descend child */
            if (rr->chld != NULL)
            {
                rr = rr->chld;
                level++;
                continue;
            }
            /* Ascend the tree */
            for (;;)
            {
                /* Move to sibling */
                if (rr->sblg != NULL)
                {
                    rr = rr->sblg;
                        break;
                }
                
                if (rr->prnt != NULL)
                {
                    if (--level < 0)
                    {
                        rr = NULL;
                        break;
                    }
                    rr = rr->prnt;
                    /* as we are assending the tree, at least the first 'level'
                     * number of chars in buffer must be ' 's so just splat in the '}'
                     */
                    buff[level] = '}' ;
                    if((ss = ffWriteFileWrite(level+1,buff,1)) != TRUE)
                        break ;
                }
            }
        }
        ffWriteFileClose(fname,meRWFLAG_WRITE,NULL) ;
        if(ss == TRUE)
        {
            rnp->mode &= ~REGMODE_CHANGE;
            return TRUE ;
        }
    }
    return mlwrite(MWABORT|MWPAUSE,(uint8 *)"[Failed to write registry %s]",fname) ;
}

/*
 * regset
 * Assign a new value and key to the registry item
 */
RNODE *
regSet (RNODE *sroot, uint8 *subkey, uint8 *value)
{
    /* Find or create the specified node tree */
    if (sroot == NULL)
        sroot = &root;

    if (subkey != NULL)
    {
        uint8 buf [1024];
        RNODE *tnp;
        uint8 *p;

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
RNODE *
regFind (RNODE *sroot, uint8 *subkey)
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
RNODE *
vregFind (RNODE *sroot, uint8 *fmt, ...)
#else
RNODE *
vregFind (RNODE *sroot, uint8 *fmt)
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
        uint8 buf [MAXBUF];
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


static int
regTestSave(RNODE *sroot, int flags)
{
    if(!(sroot->mode & REGMODE_FROOT) || !(sroot->mode & REGMODE_CHANGE) ||
       (!(sroot->mode & REGMODE_AUTO) && (sroot->mode & REGMODE_DISCARD)))
        return TRUE ;
    
    if((flags & 0x01) && !(sroot->mode & REGMODE_AUTO))
    {
        uint8 prompt[MAXBUF] ;
        int ret ;
        meStrcpy(prompt,"Save registry ") ;
        meStrcat(prompt,(sroot->value != NULL) ? sroot->value:sroot->name) ;
        if((ret = mlyesno(prompt)) == ABORT)
            return ctrlg(FALSE,1) ;
        if(ret == FALSE)
            return TRUE ;
    }
    return regSave (sroot, NULL);
}

/*
 * regDelete
 * Delete a node in the registry.
 */
int
regDelete (RNODE *sroot)
{
    /* Test and save first as user may abort */
    if(regTestSave(sroot,1) != TRUE)
        return ABORT ;
    /* Unlink the node */
    if (sroot->prnt != NULL)
        rnodeUnlink (sroot);
    rnodeDelete(sroot);
    return TRUE ;
}

/*
 * regRead
 * Open the specified registory.
 * Mode options are:-
 *
 * default        - Exists in memory
 * REGMODE_RELOAD - Delete the existing file,
 * REGMODE_MERGE  - Merge the existing file.
 */
RNODE *
regRead (uint8 *rname, uint8 *fname, int mode)
{
    LINE hlp, *lp ;
    uint8 *fn ;
    RNODE *rnp;                         /* Root node pointer */

    /* Find the registry entry */
    if ((rnp = rnodeFind (&root, rname)) != NULL)
    {
        /* if not merging or reloading then we've can use the existing node */
        if (!(mode & (REGMODE_MERGE|REGMODE_RELOAD)))
            goto finished;
        fn = rnp->value ;
    }
    else
        fn = NULL ;
    
    /* find the registry file */
    /* have we been given a valid file name ? */
    if ((fname != NULL) && (fname [0] != '\0'))
    {
        uint8 filename[FILEBUF] ;	/* Filename */
        if(fileLookup(fname,(uint8 *)".erf",meFL_CHECKDOT|meFL_USESRCHPATH,filename))
            fn = meStrdup(filename) ;
        else
            fn = meStrdup(fname) ;
    }
    /* else use the old file name (if there is one) else fail */
    else if(fn == NULL)
    {
        mlwrite(MWABORT|MWWAIT,(uint8 *)"[No file name given to load]") ;
        return NULL ;
    }
    
    /* Load in the registry file */
    hlp.l_fp = &hlp ;
    hlp.l_bp = &hlp ;
    if((ffReadFile(fn,meRWFLAG_SILENT,NULL,&hlp) == ABORT) &&
       !(mode & REGMODE_CREATE))
    {
        mlwrite (MWABORT|MWWAIT,(uint8 *)"[Cannot load registry file %s]", fname);
        return NULL ;
    }
    lp = &hlp ;

    if ((rnp != NULL) && (mode & REGMODE_RELOAD))
    {
        /* Want to replace with new one. so delete old */
        if(fn == rnp->value)
            rnp->value = NULL ;
        rnodeUnlink (rnp);
        rnodeDelete (rnp);
        rnp = NULL;
    }
    /* Construct the node for the file node */
    if (rnp == NULL)
    {
        /* Construct the node */
        if((rnp = regSet (&root, rname, NULL)) == NULL)
            return NULL ;
        rnp->mode = 0;
    }
    /* Set the node to be a file root node */
    rnp->mode |= REGMODE_FROOT;
    
    /* set the new file name */
    if(rnp->value != fn)
    {
        meNullFree(rnp->value) ;
        rnp->value = fn ;
    }
    /* Now parse the registry file */
    parseFile (rnp, lp) ;
    freeLineLoop(lp,0) ;

finished:
    rnp->mode |= mode & REGMODE_STORE_MASK ;
    return rnp ;
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
static uint8 meRegModeList[]="!hfubadmrc?g" ;
static int
regDecodeMode (int *modep, uint8 *modeStr)
{
    int mode = *modep;                  /* The mode */
    int ii, jj ;                        /* Local loop counter */
    uint8 *ss, cc ;
    
    /* Get the mode out */
    if (modeStr != NULL)
    {
        for (ii = 0; (cc=modeStr[ii]) != '\0'; ii++)
        {
            if((ss=meStrchr(meRegModeList,toLower(cc))) == NULL)
                return mlwrite(MWABORT|MWPAUSE,(uint8 *)"Illegal Registry mode [%c]",cc) ;
            jj = (1<< (((int) ss)-((int) meRegModeList))) ;
            if(isUpper(cc))
                mode &= ~jj ;
            else
                mode |=  jj ;
        }
    }
    *modep = mode;
    return TRUE;
}

/*
 * readRegistry
 * Read a file into the registry.
 */
int
readRegistry (int f, int n)
{
    uint8 filebuf [FILEBUF];
    uint8 rootbuf [32];
    uint8 modebuf [10];
    int mode = 0;

    /* Get the input from the command line */
    if ((meGetString((uint8 *)"Read registry root",0, 0, rootbuf, 32) == ABORT) ||
        (meGetString((uint8 *)"Registry file",MLFILECASE, 0, filebuf, FILEBUF) == ABORT) ||
        (meGetString((uint8 *)"File Mode", 0, 0, modebuf, 10) == ABORT))
        return ABORT;

    /* Get the mode out */
    if (regDecodeMode (&mode, modebuf) == ABORT)
        return ABORT;

    /* Read the file */
    if (regRead (rootbuf, filebuf, mode) == NULL)
        return FALSE;
    return TRUE;
}

/*
 * findRegistryName
 * Find the absolute node name associated with a node.
 * Use tail recursion to do the expansion
 */
static uint8 *
findRegistryName (RNODE *rnp, uint8 *p)
{
    uint8 *q;

    if (rnp->prnt != NULL)
    {
        q = findRegistryName (rnp->prnt, p);
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
    uint8 rootbuf [MAXBUF];
    uint8 modebuf [10];
    int mode;
    RNODE *rnp;

    /* Get the input from the command line */
    if ((meGetString((uint8 *)"Registry node",0, 0, rootbuf, MAXBUF) == ABORT) ||
        (meGetString((uint8 *)"Mark modes", 0, 0, modebuf, 10) == ABORT))
        return ABORT;

    /* Find the node */
    if ((rnp = regFind (&root, rootbuf)) == NULL)
        return mlwrite(MWCLEXEC|MWABORT,(uint8 *)"[Cannot find node %s]",rootbuf);


    /* Find the integer offset if there is one. */
    if (f)
    {
        RNODE *tnp;
        int level = 0;

        tnp = rnp;
        f = n ;
        while (--n >= 0)
        {
            if ((tnp->mode & (REGMODE_HIDDEN|REGMODE_INTERNAL)) || (tnp->chld == NULL))
            {
                for (;;)
                {
                    /* Make sure we are not back at the root */
                    if (tnp == rnp)
                        tnp = NULL;
                    else if (tnp->sblg != NULL)
                        tnp = tnp->sblg;
                    else
                    {
                        tnp = tnp->prnt;
                        level--;
                        continue;
                    }
                    break;
                }
            }
            else
            {
                tnp = tnp->chld;
                level++;
            }
        }

        /* tnp is the current node. Change the node */
        if ((rnp = tnp) == NULL)
            mlwrite (MWCLEXEC|MWABORT,(uint8 *)"[Cannot locate node %d]", f);
    }

    /* Get the mode out */
    mode = rnp->mode;
    if (regDecodeMode (&mode, modebuf) == ABORT)
        return ABORT;
    rnp->mode = mode & REGMODE_STORE_MASK ;

    /* If this is a query then return the path of the current node in
     * $result. */
    if (mode & REGMODE_QUERY)
        findRegistryName (rnp, resultStr);
    else if (mode & REGMODE_GETMODE)
    {
        uint8 *ss=resultStr ;
        int ii ;
        for(ii=0 ; ii<8 ; ii++)
            if(rnp->mode & (1<<ii))
                *ss++ = meRegModeList[ii] ;
        *ss = '\0' ;
    }

    return TRUE;
}

/*
 * saveRegistry
 * Save the registry to a file.
 */

int
saveRegistry (int f, int n)
{
    uint8 filebuf [FILEBUF];
    uint8 rootbuf [128];
    RNODE *rnp;
    
    if(n & 0x2)
    {
        rnp = root.chld ;
        while (rnp != NULL)
        {
            /* we don't what to save the history this way */
            if(meStrcmp(rnp->name,"history") &&
               (regTestSave(rnp,n) != TRUE))
                    return ABORT ;
            rnp = rnp->sblg ;
        }
        return TRUE ;
    }
    
    /* Get the input from the command line */
    if (meGetString((uint8 *)"Save registry root",0, 0, rootbuf, 128) == ABORT)
        return ABORT;
    
    /* Find the root */
    if ((rnp = regFind (&root, rootbuf)) == NULL)
        return mlwrite(MWCLEXEC|MWABORT,(uint8 *)"[Cannot find node %s]",rootbuf);
    
    if (rnp->mode & REGMODE_FROOT)
        meStrcpy(filebuf,rnp->value) ;
    else
        filebuf[0] = '\0' ;
    
    /* Get the filename */
    if(meGetString((uint8 *)"Registry file",MLFILECASE|MLFILECASE, 0, filebuf, FILEBUF) != TRUE)
        return ABORT;
    return regSave (rnp, filebuf);
}

/*
 * deleteRegistry
 * Delete the registry entry.
 */

int
deleteRegistry (int f, int n)
{
    uint8 rootbuf [MAXBUF];
    RNODE *rnp;

    if (meGetString((uint8 *)"Registry Path", 0, 0, rootbuf, MAXBUF) == ABORT)
        return ABORT;
    if ((rnp = regFind (&root, rootbuf)) == NULL)
        return mlwrite(MWCLEXEC|MWABORT,(uint8 *)"[Cannot find node %s]",rootbuf);
    return regDelete (rnp);
}

/*
 * setRegistry
 * Assign a new value to the registry
 */
int
setRegistry (int f, int n)
{
    uint8 rootbuf [MAXBUF];
    uint8 valbuf [MAXBUF];
    uint8 skeybuf [MAXBUF];
    RNODE *rnp;

    /* Get the arguments */
    if ((meGetString((uint8 *)"Registry Path", 0, 0, rootbuf, MAXBUF) == ABORT) ||
        (meGetString((uint8 *)"Sub key", 0, 0, skeybuf, MAXBUF) == ABORT) ||
        (meGetString((uint8 *)"Value", 0, 0, valbuf, MAXBUF) == ABORT))
        return ABORT;

    /* Assigns the new value */
    if ((rnp = regFind (&root, rootbuf)) == NULL)
        return mlwrite(MWCLEXEC|MWABORT,(uint8 *)"[Cannot find node %s]",rootbuf);
    if (regSet (rnp, skeybuf, valbuf) == NULL)
        return mlwrite(MWCLEXEC|MWABORT,(uint8 *)"[Cannot set registry node]");
    return TRUE;
}

/*
 * getRegistry
 * Retrieve a value from the registry. Return the value in
 * $result.
 */

int
getRegistry (int f, int n)
{
    uint8 rootbuf [MAXBUF];
    uint8 skeybuf [MAXBUF];
    RNODE *rnp;

    resultStr [0] = '\0';
    /* Get the arguments */
    if ((meGetString((uint8 *)"Registry Path", 0, 0, rootbuf, MAXBUF) == ABORT) ||
        (meGetString((uint8 *)"Sub key", 0, 0, skeybuf, MAXBUF) == ABORT))
        return ABORT;

    /* Assigns the new value */
    if (((rnp = regFind (&root, rootbuf)) == NULL) ||
        ((rnp = regFind (rnp, skeybuf)) == NULL))
        return mlwrite(MWCLEXEC|MWABORT,(uint8 *)"[Cannot find node %s/%s]",rootbuf,skeybuf);
    
    if (rnp->value != NULL)
    {
        meStrncpy (resultStr, rnp->value, MAXBUF-1);
        resultStr [MAXBUF-1] = '\0';
    }
    return TRUE;
}

/*
 * findRegistry
 * Find a key in the registry by indexing
 */
int
findRegistry (int f, int n)
{
    uint8 rootbuf [MAXBUF];
    uint8 valbuf [12];
    uint8 skeybuf [MAXBUF];
    int index;
    RNODE *rnp;

    /* Get the arguments */
    if ((meGetString((uint8 *)"Registry Path", 0, 0, rootbuf, MAXBUF) == ABORT) ||
        (meGetString((uint8 *)"Sub key", 0, 0, skeybuf, MAXBUF) == ABORT) ||
        (meGetString((uint8 *)"Index", 0, 0, valbuf, 12) == ABORT))
        return ABORT;
    index = meAtoi (valbuf);

    /* Assigns the new value */
    if (((rnp = regFind (&root, rootbuf)) == NULL) ||
        ((rnp = regFind (rnp, skeybuf)) == NULL))
        return mlwrite(MWCLEXEC|MWABORT,(uint8 *)"[Cannot find node %s/%s]",rootbuf,skeybuf);
    /* Find the node that is indexed */
    rnp = rnp->chld;
    while ((--index >= 0) && rnp)
        rnp = rnp->sblg;
    if (rnp == NULL)
    {
        resultStr [0] ='\0';
        return mlwrite (MWCLEXEC|MWABORT,(uint8 *)"Cannot find node at index %d", index);
    }
    else
    {
        meStrncpy (resultStr, rnp->name, MAXBUF-1);
        resultStr [MAXBUF-1] = '\0';
    }
    return TRUE;
}

/*
 * listRegistry
 * List the contents of the registry.
 */
int
listRegistry (int f, int n)
{
    BUFFER *bp;                         /* Buffer pointer */
    WINDOW *wp;                         /* Window associated with buffer */
    RNODE *rnp;                         /* Draw the nodes */
    uint8 vstrbuf [MAXBUF];             /* Vertical string buffer */
    uint8 buf[MAXBUF*2];                /* Working line buffer */
    int level = 0;                      /* Depth in the registry */
    int len;                            /* Length of the string */

    /* Find the buffer and vapour the old one */
    if((wp = wpopup(BregistryN,(BFND_CREAT|BFND_CLEAR|WPOP_USESTR))) == NULL)
        return FALSE;
    bp = wp->w_bufp ;                   /* Point to the buffer */

    /* Recurse the children of the node and write to file */
    rnp = &root;
    do
    {
        /* Add continuation bars if we are at a child level */
        if ((len = level) != 0)
            meStrncpy (buf, vstrbuf, len);

        /* Add connection bars for siblings */
        if (rnp->sblg == NULL)
            buf [len++] = boxChars[BCNE];
        else
            buf [len++] = boxChars[BCNES] ;

        /* Add continuation barss for children */
        if (rnp->mode & REGMODE_INTERNAL)
            buf[len++] = '!';
        else if (rnp->chld == NULL)
            buf[len++] = boxChars[BCEW];
        else if (rnp->mode & REGMODE_HIDDEN)
            buf[len++] = '+';
        else
            buf[len++] = '-';
        buf[len++] = ' ';

        /* Add the name of the node */
        buf[len++] = '"';
        len = expandexp(-1,rnp->name,(MAXBUF*2)-7,len,buf,-1,NULL,meEXPAND_BACKSLASH|meEXPAND_FFZERO) ;
        buf[len++] = '"';

        /* Add the value */
        if ((rnp->value != NULL) && !(rnp->mode & REGMODE_INTERNAL))
        {
            buf[len++] = ' ';
            buf[len++] = '=';
            buf[len++] = ' ';
            buf[len++] = '"';
            len = expandexp(-1,rnp->value,(MAXBUF*2)-2,len,buf,-1,NULL,meEXPAND_BACKSLASH|meEXPAND_FFZERO) ;
            buf[len++] = '"';
        }
        /* Add the string to the print buffer */
        buf[len] = '\0';
        addLineToEob(bp,buf);

        /* Descend child */
        if ((rnp->chld != NULL) && !(rnp->mode & (REGMODE_HIDDEN|REGMODE_INTERNAL)))
        {
            vstrbuf[level++] = (rnp->sblg != NULL) ? boxChars[BCNS] : ' ' ;
            rnp = rnp->chld;
            continue;
        }

        /* Ascend the tree */
        for (;;)
        {
            /* Move to sibling */
            if (rnp->sblg != NULL)
            {
                rnp = rnp->sblg;
                break;
            }

            --level;
            if ((rnp = rnp->prnt) == NULL)
                break;
        }

    }
    while (rnp != NULL);

    /* Set up the buffer for display */
    bp->b_dotp = lforw(bp->b_linep);
    bp->b_doto = 0 ;
    bp->line_no = 0 ;
    meModeSet(bp->b_mode,MDVIEW) ;
    meModeClear(bp->b_mode,MDATSV) ;
    meModeClear(bp->b_mode,MDUNDO) ;
    resetBufferWindows(bp) ;
    return TRUE;
}

#ifdef FREE_ALL_MEMORY
void regFreeMemory(void)
{
    rnodeDelete (root.chld);
}
#endif

#endif
