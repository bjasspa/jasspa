/****************************************************************************
 *
 *			   Copyright 1995-2004 Jon Green.
 *			      All Rights Reserved
 *
 *
 *  System        :
 *  Module        :
 *  Object Name   : $RCSfile: nlibrary.c,v $
 *  Revision      : $Revision: 1.5 $
 *  Date          : $Date: 2004-02-07 19:29:49 $
 *  Author        : $Author: jon $
 *  Last Modified : <040207.1902>
 *
 *  Description
 *
 *  Notes
 *
 *  History
 *
 ****************************************************************************
 *
 *  Copyright (c) 1995-2004 Jon Green
 *
 *  All Rights Reserved.
 *
 *  This Document may not, in whole or in part, be copied,
 *  photocopied, reproduced, translated, or reduced to any
 *  electronic medium or machine readable form without prior
 *  written consent from Jon Green.
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <utils.h>
#include "_nroff.h"
#include "nroff.h"

#define TEST 0                          /* 1 to enabled standalone test */
#define dprintf(x) /* printf x */

typedef struct st_LibPath
{
    struct st_LibPath *next;
    char *name;
} LibPath;

static LibPath *nrLibPathHead = NULL;   /* Library path head pointer */
static LibPath *nrLibPathTail = NULL;   /* Library path tail pointer */
static char *nrLibModName = NULL;       /* Library module name */
static char *nrHome = NULL;             /* Home page */
static int nrLibCurrentNameFormat = 0;  /* Name format */

LibModule *nrLibModule = NULL;
LibModule *undefModule = NULL;

/*
 * Library name mode
 */
int
nrLibNameFormat (int newmode)
{
    int oldFormat = nrLibCurrentNameFormat;
    
    if (newmode >= 0)
        nrLibCurrentNameFormat = newmode;
    return oldFormat;
}

/*
 * Library name searching utilies.
 */

char *
nrLibGetModule (void)
{
    return (nrLibModName);
}

int
nrLibSetModule (char *name)
{
    if (nrLibModName != NULL)
    {
        uError ("Module name duplicately defined. Ingnoring [%s]. Using [%s]\n",
                nrLibModName, name);
        return (1);
    }

    nrLibModName = bufStr (NULL, name);
    return (0);
}

int
nrLibSetPath (char *path)
{
    LibPath *l;
    char    *p;
    char    c;
    char    last_c;

    if (path == NULL)
        return (1);

    /*
     * Construct the path name. Modify all of the back slashes to
     * forward slashes.
     */

    p = bufStr (NULL, "");              /* Allocate a buffer */
    for (last_c = '\0'; ((c = *path++) != '\0'); last_c = c)
    {
        if (c == '\\')
            c = '/';
        p = bufChr (p, c);
    }

    if (last_c != '/')
        p = bufChr (p, '/');

    /*
     * Construct the new library entry.
     */

    l = (LibPath *) malloc (sizeof (LibPath));
    l->name = p;
    l->next = NULL;

    /*
     * Add the search path to the end of the library path list
     */

    if (nrLibPathTail == NULL)
        nrLibPathHead = l;
    else
        nrLibPathTail->next = l;
    nrLibPathTail = l;
    return (0);
}

int
nrLibLoad (char *name)
{
    LibPath *l;
    FILE *fp;
    char *p;
    int  status = 0;

    /*
     * Make sure that we have not already loaded the module.
     */

    if (nrLibModuleFind (nrLibModule, name) != NULL)
    {
        uError ("Library module [%s] already loaded\n");
        return (1);
    }

    /*
     * Try and locate the library - iterate through each of the paths
     * and open.
     */

    for (l = nrLibPathHead; l != NULL; l = l->next)
    {
        p = bufFormat (NULL, "%s%s.lbn", l->name, name);
        if ((fp = fopen (p, "rb")) != NULL)
        {
            nrfp = __nrfPush (p, fp);
            if (nroff (nrfp, NROFF_MODE_SYMBOL) == 0) {
                uError ("Problems loading the library [%s]\n", p);
                status = 1;
            }
            p = bufFree (p);
            nrfp = NULL;
            return (status);
        }
        else
            bufFree (p);
    }
    uError ("Cannot find library [%s.lbn]\n", name);
    return (1);
}

/*
 * Library name and module caching utilities
 */


LibModule *
nrLibModuleConstruct (char *name, int ref)
{
    LibModule *p;
    int nlen;

    nlen = strlen (name);
    p = malloc (sizeof (LibModule) + nlen + 1);
    p->next = NULL;
    p->item = NULL;
    p->package = NULL;
    p->alias = NULL;
    p->data = NULL;
    nrLibModuleStatus(p) = ref;
    strcpy (&p->name[1], name);
    return (p);
}

LibModule *
nrLibModuleDestruct (LibModule *p)
{
    if (p)
    {
        bufFree (nrLibModuleAlias(p));
        free (p);
    }
    return (NULL);
}

LibModule *
nrLibModuleFind (LibModule *p, char *name)
{
/*    int status;*/

    if (p == NULL)
        return (NULL);
    while ((p != NULL) && (strcmp (name, nrLibModuleName (p)) != 0))
        p = nrLibModuleNext (p);
    return (p);
}

LibModule *
nrLibModuleAdd (LibModule **lp, char *name, int ref, int *status)
{
    LibModule *p;
    LibModule *q;

    if (((p = *lp) != NULL) &&
        ((p = nrLibModuleFind (p, name)) != NULL))
    {
        if (status != NULL)
            *status = 0;
        return (p);
    }

    /*
     * Construct a new library and add to the end of the module
     * list. Ensure that modules are added to the end of the list
     * hence they are searched in the order of definition.
     */

    p = nrLibModuleConstruct (name, ref);
    if (*lp == NULL)
        *lp = p;
    else
    {
        for (q = *lp; nrLibModuleNext(q) != NULL; q = nrLibModuleNext(q))
            ;
        nrLibModuleNext(q) = p;
    }
    return (p);
}

int
nrLibModuleSetAlias (char *module, char *alias)
{
    LibModule *m;
    int status;

    m = nrLibModuleAdd (&nrLibModule, module, LS_DEF, &status);
    if ((m != NULL) && (alias != NULL))
    {
        bufFree (nrLibModuleAlias(m));
        nrLibModuleAlias(m) = bufNStr (NULL, alias);
    }
    return (status);
}

LibItem *
nrLibItemConstruct (char *item, char *file, int ref)
{
    LibItem *p;
    int     ilen;
    int     flen;

    if (item)
        ilen = strlen (item);
    else
        ilen = 0;

    if (file)
        flen = strlen (file);
    else
        flen = 0;
    p = malloc (sizeof (LibItem) + ilen + flen);
    p->next = NULL;
    p->module = NULL;
    p->browse = 0;
    p->data = NULL;
    p->name[LI_STATUS] = ref;
    if (ilen)
        strcpy (&p->name[LI_NAME], item);

    ilen += LI_NAME + 1;
    p->name[LI_FILE] = ilen;
    if (flen)
        strcpy (&p->name[ilen], file);
    else
        p->name[LI_FILE] = 0;

    return (p);
}


LibItem *
nrLibItemDestuct (LibItem *p)
{
    if (p != NULL)
        free (p);
    return (NULL);
}

LibItem *
nrLibItemFind (LibItem *p, char *name, char **file)
{
    int status;

    if (p == NULL)
        return (NULL);
    while ((status = strcmp (name, nrLibItemName (p))) > 0)
        if ((p = p->next) == NULL)
            break;
    if (status == 0)
    {
        if (file != NULL)
        {
            if (p->name [LI_FILE] == 0)
                *file = NULL;
            else
                *file = nrLibItemFile(p);
        }
        return (p);
    }
    return (NULL);
}

/*
 * Add a new item to the library. Returns 1 if it is a new item and zero
 * if it is an existing item.
 */

LibItem *
nrLibItemAdd (LibItem **lp, char *name, char *file, int ref, int *rstatus)
{
    LibItem *p;
    LibItem *q;
    int status;

    /*
     * Add a new item to the head of the list
     */
    status = 1;
    if ((((p = *lp) == NULL) ||
         (status = strcmp (name, nrLibItemName (p))) < 0))
    {
        q = nrLibItemConstruct (name, file, ref);
        q->next = *lp;
        if (rstatus)
            *rstatus = 1;               /* Constructed new item - return 1 */
        return (*lp = q);
    }

    /*
     * Traverse down the list until we are about to fail.
     */

    if (status > 0)
    {
        while ((q = p->next) != NULL)
        {
            if ((status = strcmp (name, nrLibItemName (q))) <= 0)
            {
                if (status == 0)        /* Equal */
                    p = q;              /* Set up p for later. */
                break;
            }
            else
                p = q;
        }
    }

    if (status == 0)
    {
        /*
         * If the new item is a definition then return 0 if there is a
         * conflict of definitions, otherwise accept.
         */

        if (rstatus)
        {
            if (ref & nrLibItemStatus(p) & LS_DEF)
                *rstatus = 0;
            else
                *rstatus = 1;
        }
        nrLibItemStatus(p) |= ref;
        return (p);
    }

    q = nrLibItemConstruct (name, file, ref);
    q->next = p->next;
    p->next = q;
    if (rstatus)                        /* New item - return 1 */
        *rstatus = 1;
    return (q);
}

LibBrowse *
libFindBrowseSequence (char *module, char *name, char *section)
{
    LibItem *i;
    LibModule *m;
    int browseNo;
    char *xref;

    /*
     * Find the browse module.
     */
    if ((m = nrLibModuleFind (nrLibModule, module)) == NULL)
        return (NULL);

    xref = nrMakeXref (name,section);
    if ((i = nrLibItemFind (nrLibModuleItem (m), xref, NULL)) == NULL)
        return (NULL);
    if ((browseNo = nrLibItemBrowse (i)) == 0)
        return (NULL);

#if 0
    browseData->prevName = NULL;
    browseData->nextName = NULL;
    browseData->
    char *prevSection;
    int  prevBrowseNo;
    char *nextName;
    char *nextSection;
    int  nextBrowseNo;


    for (i = nrLibModuleItem (m); i != NULL; i = nrLibItemNext (i))
        if (((nrLibItemStatus(i) & LS_BROWSE) != 0) &&
            ((nrLibItemBrowse(i) == 0)))
            nrLibItemBrowse(i) = browseNo++;
#endif
    return (NULL);
}

int
libResolveBrowseSequence (char *module)
{
    LibModule *m;
    LibItem   *i;
    int       browseNo = 1;

    /*
     * Find the browse module.
     */
    if ((m = nrLibModuleFind (nrLibModule, module)) == NULL)
        return (1);


    /*
     * If the module name exists as an entry then assign as the
     * first browse sequence.
     */

    if ((i = nrLibItemFind (nrLibModuleItem(m), module, NULL)) != NULL)
        nrLibItemBrowse(i) = browseNo++;

    /*
     * Search the list linearly and assign the browse sequence as
     * we go.
     */

    for (i = nrLibModuleItem (m); i != NULL; i = nrLibItemNext (i))
        if (((nrLibItemStatus(i) & LS_BROWSE) != 0) &&
            ((nrLibItemBrowse(i) == 0)))
            nrLibItemBrowse(i) = browseNo++;
    return (0);
}

/*
 * Returns zero is this is defined, otherwise non-zero if this is a
 * new item. 
 */

int
libFindAddReference (char *module, char *name, char *file,
                     char **rmodule, char **rfile, void **handle, int ref)
{
    LibModule *m;
    LibItem   *i;
    int       status = 0;

    dprintf (("libFindAddReference - [%s] module:[%s] name:[%s] file\n",
              module, name, file));


    if (module != NULL)
    {
        dprintf (("libFindAddReference - [%s] module\n", module));
        if ((m = nrLibModuleFind (nrLibModule, module)) == NULL)
            m = nrLibModuleAdd (&nrLibModule, module, ref, NULL);
        
        if (ref & LS_PACKAGE)
            i = nrLibItemAdd (&nrLibModulePackage(m), name, file, ref, &status);
        else
            i = nrLibItemAdd (&nrLibModuleItem(m), name, file, ref, &status);

        /*
         * If the module name is non null and it matches the name
         * of the library module then return NULL, otherwise return
         * the module name of the library.
         */

        if (rmodule != NULL)
        {
            if ((nrLibModName != NULL) &&
                (((nrLibModuleAlias(m) != NULL) &&
                  (strcmp (nrLibModName, nrLibModuleAlias(m)) == 0)) ||
                 (strcmp (nrLibModName, nrLibModuleName(m)) == 0)))
                *rmodule = NULL;
            else
                *rmodule = ((nrLibModuleAlias(m) != NULL) ?
                            nrLibModuleAlias(m) : nrLibModuleName(m));
        }
        if (rfile != NULL)
        {
            if (i->name [LI_FILE] == 0)
                *rfile = NULL;
            else
                *rfile = nrLibItemFile(i);
        }
        if (handle != NULL)
            *handle = i;
        return (status);
    }

    for (m = nrLibModuleHead(); m != NULL; m = nrLibModuleNext(m))
    {
        /* Determine if we are dealing with the item or package list */
        if (ref & LS_PACKAGE)
            i = nrLibModulePackage(m);
        else
            i = nrLibModuleItem(m);
                  
        /* Search the list */
        if ((i = nrLibItemFind (i, name, NULL)) != NULL)
        {
            if (rmodule != NULL)
            {
                if ((nrLibModName != NULL) &&
                    (((nrLibModuleAlias(m) != NULL) &&
                      (strcmp (nrLibModName, nrLibModuleAlias(m)) == 0)) ||
                     (strcmp (nrLibModName, nrLibModuleName(m)) == 0)))
                    *rmodule = NULL;
                else
                    *rmodule = ((nrLibModuleAlias(m) != NULL) ?
                                nrLibModuleAlias(m) : nrLibModuleName(m));
            }
            if (rfile != NULL)
            {
                if (i->name [LI_FILE] == 0)
                    *rfile = NULL;
                else
                    *rfile = nrLibItemFile(i);
            }
            if (handle)
                *handle = (void *) i;
            /*
             * The item is found - return a status depending on what was
             * requested.
             */

            if (ref != 0)
            {
                /* Duplicate label */
                if ((ref & nrLibItemStatus (i) & LS_DEF) != 0)
                    status = 0;
                else
                    status = 1;
                nrLibItemStatus (i) |= ref;
            }
            return (status);
        }
    }

    return (libFindAddReference ("undefined", name, file,
                                 rmodule, rfile, handle, ref));
}

static char *
nrXlateRefStr (char *dest, char *src)
{
    char *p = dest;
    int c;
    
    for (/* NULL */; (c = *src) != '\0'; src++)
    {
        if (isupper (c))
            *p++ = tolower (c);
        else if (islower (c))
            *p++ = c;
        else if ((nrLibCurrentNameFormat == 0) && (isdigit (c)))
            *p++ = c;
        else if (nrLibCurrentNameFormat != 0)
        {
            switch (c)
            {
            case '0': *p++='Z'; *p++='E'; *p++='R'; *p++='O'; break;
            case '1': *p++='O'; *p++='N'; *p++='E'; break; 
            case '2': *p++='T'; *p++='W'; *p++='O'; break;
            case '3': *p++='T'; *p++='H'; *p++='R'; *p++='E'; *p++='E'; break;
            case '4': *p++='F'; *p++='O'; *p++='U'; *p++='R'; break;
            case '5': *p++='F'; *p++='I'; *p++='V'; *p++='E'; break;
            case '6': *p++='S'; *p++='I'; *p++='X'; break; 
            case '7': *p++='S'; *p++='E'; *p++='V'; *p++='E'; *p++='N'; break;
            case '8': *p++='E'; *p++='I'; *p++='G'; *p++='H'; *p++='T'; break;
            case '9': *p++='N'; *p++='I'; *p++='N'; *p++='E'; *p++='T'; break;
            case '_': *p++='U'; *p++='S'; break; 
            case '#': *p++='H'; *p++='A'; *p++='A'; *p++='S'; *p++='H'; break;
            case '&': *p++='A'; *p++='N'; *p++='D'; break; 
            case '>': *p++='G'; *p++='T'; break; 
            case '<': *p++='L'; *p++='H'; break;
            case ':': *p++='C'; *p++='O'; *p++='L'; *p++='O'; *p++='N'; break;
            case '.': *p++='D'; *p++='O'; *p++='T'; break;
            case ',': *p++='C'; *p++='O'; *p++='M'; *p++='M'; *p++='A'; break;
            case ';': *p++='S'; *p++='E'; *p++='M'; *p++='I'; break; 
            case '-': *p++='M'; *p++='I'; *p++='N'; *p++='U'; *p++='S'; break;
            case '+': *p++='P'; *p++='L'; *p++='U'; *p++='S'; break;
            case '@': *p++='A'; *p++='T'; break;
            case '\'':*p++='R'; *p++='Q'; *p++='U'; break; 
            case '`': *p++='L'; *p++='Q'; *p++='U'; break; 
            case '"': *p++='D'; *p++='Q'; *p++='U'; break;
            case '~': *p++='T'; *p++='I'; *p++='L'; *p++='D'; *p++='E'; break;
            case '(': *p++='L'; *p++='R'; *p++='B'; break; 
            case ')': *p++='R'; *p++='R'; *p++='B'; break;
            case '{': *p++='L'; *p++='C'; *p++='B'; break;
            case '}': *p++='R'; *p++='C'; *p++='B'; break;
            case '[': *p++='L'; *p++='S'; *p++='B'; break;
            case ']': *p++='R'; *p++='S'; *p++='B'; break;
            case '?': *p++='Q'; *p++='U'; break; 
            case '|': *p++='V'; *p++='B'; break;
            case '=': *p++='E'; *p++='Q'; break;
            case '*': *p++='S'; *p++='T'; break;
            case '\\':*p++='B'; *p++='S'; break;
            case '/': *p++='F'; *p++='S'; break;
            case '!': *p++='E'; *p++='X'; break;
            case '%': *p++='P'; *p++='E'; break; 
            case '^': *p++='C'; *p++='A'; break;
            }
        }
    }
    return p;
}    


char *
nrMakeXref (char *name, char *section)
{
    static char buffer [256];
    char *s;
    char *p;

    p=buffer;
    if ((s = name) != NULL)
        p = nrXlateRefStr (buffer, name);
    if ((s = section) != NULL)
        p = nrXlateRefStr (p, section);

    /*
     * Terminate the buffer or return NULL
     */
    if (p == buffer)
        return (NULL);
    else
        *p = '\0';
    return (buffer);
}

/*
 * Determine the current level of the page. The level is zero if the name and
 * the section id are exactly the same as the home page and the module name.
 */

int
nrGetLevel (char *name, char *id)
{
    char *xref;

    xref = nrMakeXref (name, id);       /* Construct the Xref name */

    if ((xref != NULL) &&
        (nrLibGetModule () != NULL) &&
        (strcmp (xref, nrLibGetModule()) == 0) &&
        (strcmp (xref, nrHomeGet()) == 0))
        return (0);
    return (1);
}

/*
 * Library lookup functions
 */

int
nrResolveExternalReference (int flags, char *name, char *section,
                            char **pmodule, char **pfile)
{
    char *module = NULL;                /* Module name */
    char *file = *pfile;                /* File name */
    int  status = 0;                    /* Type of package */

    /*
     * Collect the data.
     */

    if (pmodule != NULL)
        module = *pmodule;
    if (pfile != NULL)
        file = *pfile;

    if ((flags & LS_COMPILE) != 0)
    {
        /*
         * If the module and file are specified then use them as given
         * otherwise look them up.
         */

        if ((module == NULL) || (file == NULL))
        {
            char *xrefname;             /* Cross reference name */
            char *rmodule = NULL;       /* Returned module name */
            char *rfile = NULL;         /* Returned file name */
            LibItem *li;                /* Library item */

            xrefname = nrMakeXref (name, section);
            if (libFindAddReference (module, xrefname, NULL,
                                     &rmodule, &rfile, (void *)(&li), 
                                     (flags & LS_PACKAGE) | LS_DEF) != 0)
            {

                uError ("Undefined external reference to [%s]\n", xrefname);
            }
            else
            {
                /*
                 * Replace the existing module information with the
                 * data that has been returned.
                 */
                if ((flags & LS_PACKAGE) == 0)  /* Is it a jump ?? */
                    status = nrLibItemStatus (li) & LS_JUMP;
                if (rmodule != NULL)
                    bufFree (module);
                module = rmodule;
                if (file != NULL)
                    bufFree (file);
                file = rfile;
            }
        }

    }

    /*
     * If we are not compiling then bin the information we do not need
     * it.
     */

    else
    {
        module = bufFree (module);
        file = bufFree (file);
    }

    /*
     * Having decided what to return - do so. Note that the return
     * exists in a dynamic memory area. !!
     */

    if (pmodule)
        *pmodule = bufNStr (NULL, module);
    if (pfile)
        *pfile = bufNStr (NULL, file);
    return (status);
}

typedef struct st_Title
{
    struct st_Title *next;
    char *text;
    char *xref;
    char *module;
    char *file;
} Title;

static Title *nrTitleHead = NULL;
static Title *nrTitleTail = NULL;

/*
 * nrTitleSet ()
 * Define a new title entry.
 * s = title:name:section
 */
int
nrTitleSet (char *s)
{
    Title *t;
    char  c;
    char *section;
    char *name;
    char  *cp;

    t = (Title *) malloc (sizeof (Title));
    s = bufNStr (NULL, s);

    t->next = NULL;
    t->module = NULL;
    t->file = NULL;

    if (s == NULL)
        return (1);

    /*
     * Search for the first colon for the title.
     */
    for (cp = NULL; (c = *s) != '\0'; cp = bufChr (cp, c))
    {
        s++;
        if (c == ':')
            break;
        if (c == '_')                   /* May use '_' as a space */
            c = ' ';
    }

    if (cp == NULL)
        uError ("Title sting not defined\n");
    else
        t->text = cp;
    /*
     * Search for the second colon - the name name.
     */
    for (name = NULL; (c = *s) != '\0'; name = bufChr (name, c))
    {
        s++;
        if (c == ':')
            break;
    }

    if (name == NULL)
        uError ("Title [%s] name not defined\n");

    /*
     * Search for the third colon - the section number.
     */
    for (section = NULL; (c = *s) != '\0'; section = bufChr (section, c))
    {
        s++;
        if (c == ':')
            break;
    }

    t->xref = bufNStr (NULL, nrMakeXref (name, section));
    t->file = NULL;

    /*
     * Add the title to the end of the list.
     */

    if (nrTitleTail == NULL)
        nrTitleHead = t;
    else
        nrTitleTail->next = t;
    nrTitleTail = t;

    bufFree (name);
    bufFree (section);
    return (0);
}

void *
nrTitleGet (void *handle, char **title, char **xref, char **rmodule, char **rfile)
{
    Title *t = NULL;
    char  *module = NULL;
    char  *file = NULL;

    if (handle == NULL)
        t = nrTitleHead;
    else
        t = ((Title *)(handle))->next;

    if (t != NULL)
    {
        if ((t->module) == NULL)
        {
            if (libFindAddReference (NULL, t->xref, NULL,
                                     &module, &file, NULL, LS_DEF) != 0)
                uError ("Undefined external reference to [%s] in title\n",
                        t->xref);
            t->module = module;
            t->file = file;
        }
        if (title)
            *title = t->text;
        if (xref)
            *xref = t->xref;
        if (rmodule)
        {
            if ((nrLibModName != NULL) && (t->module != NULL) &&
                (strcmp (nrLibModName, t->module) == 0))
                *rmodule = NULL;
            else
                *rmodule = t->module;
        }
        if (rfile)
            *rfile = t->file;
    }
    return (t);
}

/*
 * Home page.
 * This defines the name of the home page. Used to determine the master
 * index page.
 */

void
nrHomeSet (char *s)
{
    nrHome = bufFree (nrHome);
    nrHome = bufNStr (nrHome, s);
}

char *
nrHomeGet (void)
{
    if (nrHome == NULL)
        return ("home");
    return (nrHome);
}

#if TEST
int
main ()
{
    LibItem *i;
    char **p;
    char *module;
    int status;
    int ii;

    char *array [] = {"abb", "aab", "aaa", "dcd", "bbb", "aaa",
                  "zzz","yyy","000","aab", "aac", NULL};

    i = NULL;

    for (p = array; *p != NULL; p++)
    {

        status = libFindAddReference (NULL, *p, "foo.1", &module, NULL,
                                      NULL, LS_REF);
        printf ("Added [%s] to [%s] : Status = %d\n",
                *p, module, status);
        fflush (stdout);
    }

    i = nrLibModule->item;
    for (ii = 1; i != NULL; i = i->next, ii++)
    {
        printf ("%d. %s:%s\n", ii, &i->name[LI_NAME],
                &i->name[(int)(i->name[LI_FILE])]);
        fflush (stdout);
    }
    return (0);
}

#endif
