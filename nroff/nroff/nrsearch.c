/****************************************************************************
 *
 *			Copyright 1996-2004 Jon Green.
 *                         All Rights Reserved
 *
 *
 *  System        :
 *  Module        :
 *  Object Name   : $RCSfile: nrsearch.c,v $
 *  Revision      : $Revision: 1.3 $
 *  Date          : $Date: 2004-02-07 19:29:49 $
 *  Author        : $Author: jon $
 *  Last Modified : <040207.1921>
 *
 *  Description
 *
 *  Notes
 *
 *  History
 *
 * 2.0.0c 07/02/04 JG Ported to HP-UX
 * 2.0.0b 03/01/04 JG Ported to Sun Solaris 9
 * 2.0.0a 30/05/97 JG Ported to win32.
 *
 ****************************************************************************
 *
 *  Copyright (c) 1996-2004 Jon Green.
 *
 *  All Rights Reserved.
 *
 * This  Document  may  not, in  whole  or in  part, be  copied,  photocopied,
 * reproduced,  translated,  or  reduced to any  electronic  medium or machine
 * readable form without prior written consent from Jon Green.
 *
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>

#if ((defined _HPUX) || (defined _LINUX) | (defined _SUNOS))
#include <unistd.h>
#else
#include <getopt.h>
#endif

#include <utils.h>                      /* Standard utilities */
#include "nroff.h"                      /* Nroff definitions */

#define MODULE_VERSION  "2.0.0c"
#define MODULE_NAME     "nrsearch"

#define WHITE_SPACE_SKIP       1
#define WHITE_SPACE_TERMINATE  2

#define TAG_BANK_SIZE          10       /* Size of the tag bank */

typedef struct KeyStruct {
    char    *name;
    int     index;
    int     size;
} KeyType;

/* Construct the key table from the definition file. */

enum {
#define DEF_KW(x,y) y,
#include    "nrsearch.def"
#undef  DEF_KW
        KW_MAX
};   /* End of 'enum' */

static KeyType keyTab [] = {
#define DEF_KW(x,y) {x, y, 0},
#include    "nrsearch.def"
#undef  DEF_KW
        {NULL, KW_MAX, 0}
};

/* Local type definitions */

typedef struct fEntry {
    struct fEntry *next;
    char *name;
    char *id;
    char *desc;
    struct fItem *item;
    char *component;
    char **componentList;
    int lineNo;
} fEntry;

typedef struct fItem {
    struct fItem *next;
    char *fileName;
    char *module;
    char *component;
    char **componentList;
    char *sectionId;
    struct fEntry *entry;
} fItem;

/* Local buffers */

static char *searchModule = NULL;
static char *searchComponent = NULL;
static char *searchId = NULL;
static char *responseFile = NULL;
static int  alphabeticMode = 0;
static char *nroffDescriptionString          = "\"%i\" %s\\n%d%n.br\\n";
static char *nroffNoDescriptionString        = "\"%i\" %s\\n.br\\n";
static char defaultNroffString [30];
static int  nroffResponse = 0;
static char *progname = MODULE_NAME;    /* Name of the module */
static FILE *fpr = NULL;                /* Response file pointer */
static fItem *ihead = NULL;             /* Item header */
static fItem *phead = NULL;             /* Package header */
static fItem **itemMode = &ihead;       /* Item mode */
static TagP tagBank[TAG_BANK_SIZE];     /* Bank of tags */

static fEntry *
imEntryAlloc (fItem *item, char *name, char *id, char *desc, char *comp, int lineNo)
{
    char tdesc [1024];
    char *q;
    fEntry *fe;

    
    /* Expand the description. It may contain control characters
     * which need to be replaced. */
    
    q = tdesc;
    if (desc != NULL)
    {
        char c;
        char *p = desc;
        
        while ((c = *p++) != '\0')
        {
            switch (c)
            {
            case USPACE_CHAR:
                *q++ = '\\';
                *q++ = ' ';
                break;
            case ITALIC_CHAR:
            case BOLD_CHAR:
            case ROMAN_CHAR:
            case MONO_CHAR:
            case PREV_CHAR:
                *q++ = '\\';
                *q++ = 'f';
                if (c == ITALIC_CHAR)
                    *q++ = 'I';
                else if (c == BOLD_CHAR)
                    *q++ = 'B';
                else if (c == ROMAN_CHAR)
                    *q++ = 'R';
                else if (c == MONO_CHAR)
                    *q++ = 'C';
                else
                    *q++ = 'P';
                break;
            case ESC_CHAR:
                *q++ = '\\';
                *q++ = 'e';
                break;
            case COPYRIGHT_CHAR:
                *q++ = '\\';
                *q++ = '(';
                *q++ = 'c';
                *q++ = 'o';
                break;
            case BULLET_CHAR:
                *q++ = '\\';
                *q++ = '(';
                *q++ = 'b';
                *q++ = 'u';
                break;
            case MSPACE_CHAR:
                *q++ = '\\';
                *q++ = '|';
                break;
            case CONTINUE_CHAR:
                *q++ = '\\';
                *q++ = 'c';
                *q++ = '\n';
                break;
            case ZSPACE_CHAR:
                *q++ = '\\';
                *q++ = '&';
                break;
            default:
                *q++ = c;
                break;
            }
        }
    }    
    *q = '\0';
        
/*    uWarn ("Adding .Im Entry [%s][%s][%s=>%s][%d]\n", name, id, desc, tdesc, lineNo);*/

    if ((fe = (fEntry *) malloc (sizeof (fEntry))) == NULL)
        uFatal ("No memory for fEntry\n");

    /* Form a linked list of entries */
    fe->next = item->entry;
    item->entry = fe;

    /* Add entry to item list */
    fe->name = bufNStr (NULL, name);
    if ((id != NULL) && (strcmp (id , "-") == 0))
        id = NULL;
    fe->id = bufNStr (NULL, id);
    fe->desc = bufNStr (NULL, tdesc);
    fe->item = item;
    fe->lineNo = lineNo;
    fe->component = bufNStr (NULL, comp);
    fe->componentList = nrImGetAll (comp);

/*    dbprintf (("Added .Im Entry [%s][%s][%s][%d]\n", name, id, desc, lineNo));*/

    return (fe);
}

static fEntry *
imEntryFree (fEntry *fe)
{
    fItem *fi;
    fEntry *p;

    if (fe != NULL) {

        /* Free up the string buffers */
        bufFree (fe->name);
        bufFree (fe->id);
        bufFree (fe->desc);
        bufFree (fe->component);
        if (fe->componentList != NULL)
            free (fe->componentList);

        /* Remove the item from the list */
        if ((fi = fe->item) != NULL) {
            if ((p = fi->entry) == fe)
                fi->entry = fe->next;
            else {
                while ((p != NULL) && (p->next != fe))
                    p = p->next;
                if (p != NULL)
                    p->next = fe->next;
            }
        }

        free (fe);
        return (fi->entry);
    }
    return (NULL);
}

static fItem *
imItemAlloc (fItem *head, char *file, char *module, char *component, char *sectionId)
{
    fItem *fi;

    uDebug (1, ("Adding .Im Item [%s][%s][%s][%s]\n",
                file, module, component, sectionId));

    if ((fi = (fItem *) malloc (sizeof (fItem))) == NULL)
        uFatal ("No memory for fItem.\n");

    /* Form a linked list of entries */
    fi->next = head;                    /* Note may be NULL */

    /* Add the parameter list to the items */
    fi->fileName = bufNStr (NULL, file);
    fi->module = bufNStr (NULL, module);
    fi->component = bufNStr (NULL, component);
    fi->componentList = nrImGetAll (component);
    fi->sectionId = bufNStr (NULL, sectionId);
    fi->entry = NULL;

    uDebug (1, ("Added .Im Item [%s][%s][%s][%s]\n",
                file, module, component, sectionId));
    return (fi);
}

static fItem *
imItemFree (fItem *fi)
{
    fItem *p;

    if (fi == NULL)
        return (NULL);

    /* Free up the entry list */
    while (fi->entry != NULL)
        imEntryFree (fi->entry);

    /* Free up the strings */
    bufFree (fi->fileName);
    bufFree (fi->module);
    bufFree (fi->sectionId);
    bufFree (fi->component);
    if (fi->componentList != NULL)
        free (fi->componentList);

    /* Save the next one and free the block */
    p = fi->next;
    free (fi);
    return (p);
}

static void
nrIm_func (char *module, char *component)
{

    ihead = imItemAlloc (ihead, nrfp->fileName, module,
                         ((component == NULL) ? "misc" : component), NULL);
    phead = imItemAlloc (phead, nrfp->fileName, module,
                         ((component == NULL) ? "misc" : component), NULL);
}

static void
nrLs_func (char *module, char *alias)
{

    ihead = imItemAlloc (ihead, nrfp->fileName, module, NULL, NULL);
    phead = imItemAlloc (phead, nrfp->fileName, module, NULL, NULL);
}

static void
nrTH_func (char *id, char *num, char *date, char *company, char *title)
{
    if (ihead)
        ihead->sectionId = bufStr (NULL, num);
    if (phead)
        phead->sectionId = bufStr (NULL, num);
}

static void
nrStartInc (char *s, int *imode)
{
    *imode = 1;
}

static void
nrXI_func (char *name, char *id, char *desc, char *comp)
{
    if (id == NULL)
        id = ihead->sectionId;
    imEntryAlloc (ihead, name, id, desc, comp, nrfp->lineNo);
}

static void
nrXP_func (char *name, char *id, char *desc, char *comp)
{
    if (id == NULL)
        id = phead->sectionId;
    imEntryAlloc (phead, name, id, desc, comp, nrfp->lineNo);
}
/*
 * Limited magic search on section id. 
 * Returns 0 if match else 1.
 * 
 * Regular expressions are:-
 * 
 *     * - Matches anything to end
 *     . - Matches any character
 *     | - Or condition
 * 
 * Exits on the first true match
 */

static int
sectionCmp (char *magic, char *item)
{
    char *p;
    int c;
    int c1;
    int skip = 0;                       /* Skip to next match */
    
    p = item;
    while ((c = *magic++) != '\0')
    {
        c1 = *p++;                      /* Get next item */
        switch (c)
        {
        case '|':
            if (c1 == '\0')             /* End of string ?? */
                return (0);             /* Must have matched !! */
            p = item;                   /* Get string for next attempt */
            break;
        case '*':
            return (0);                 /* Must have matched */
            break;
        case '.':
            if (c1 == '\0')             /* End of string ?? */
                skip = 1;               /* Jump to next match */
            break;                      /* Yes - not a match */
        default:
            if (isupper (c))  c = tolower(c);
            if (isupper (c1)) c1 = tolower(c1);
            if (c1 != c)
                skip = 1;
        }
        
        /*
         * Determine if we skip here. If so reset the item string and
         * find the next separator.
         */
        if (skip != 0)
        {
            /* Skip to next marker */
            while (((c = *magic) != '\0') && (c != '|'))
                magic++;
            p = item;                   /* Get string for next attempt */
            skip = 0;                   /* Reset skip flag */
        }
    }
    
    /*
     * Quick check on exit. We are at the end of the match string. 
     * We have a match if it is also the end of the item string.
     */
    if (*p == '\0')
        return (0);                     /* Found */
    return (1);                         /* Not found */
}
    
        
static void
constructResponseList (fItem *fi, char *searchModule, char *searchComponent,
                       char *searchId, int searchType)
{
    fEntry *fe;
    TagP  tp;
    
    uDebug (1, ("Constructing Response List for [%s][%s][%s]\n",
                searchModule, searchId, searchComponent));

    /* Iterate through the item lists */

    for (/* NULL */; fi != NULL; fi = fi->next)
    {
        for (fe = fi->entry; fe != NULL; fe = fe->next)
        {
            if ((searchModule != NULL) && (fi->module != NULL) &&
                (strcasecmp (searchModule, fi->module) != 0))
                break;
            if ((searchComponent != NULL) &&
                !(((fi->componentList != NULL) &&
                   (nrImCmpAll (searchComponent, fi->componentList) == 0)) ||
                  ((fe->componentList != NULL) &&
                   (nrImCmpAll (searchComponent, fe->componentList) == 0))))
                continue;
            
#if 0
            if ((searchId != NULL) && (fe->id != NULL) &&
                (strcmp (searchId, fe->id) != 0))
                continue;
#else
            if ((searchId != NULL) && (fe->id != NULL) &&
                (sectionCmp (searchId, fe->id) != 0))
                continue;
#endif
            /* Matched. */
            if (searchType==KW_remove)  /* Removing a tag ?? */
            {
                /* Delete existing tag */
                if ((tp = find_tag (&tagBank[0], fe->name, fe->id, 1)) != NULL)
                    tagFree (tp);           /* Delete the removed tag */
            }
            else if (searchType==KW_search)
            {
                /* Make tag to add to the tag list */
                tp = tagAlloc (fe->name, fe->id, fe->desc,
                               fi->fileName, fe->lineNo, (void *)(fe));
                /* Add the new tag */
                if (add_tag (&tagBank[0], tp) != NULL)
                    tagFree (tp);       /* Duplicate tag - remove */
            }
        }
    }
    uDebug (1, ("Constructed Response List\n"));
}

static void
destructResponseList (void)
{
    free_tags (&tagBank[0]);
}

/*
 * Reproduce the echo. Do some control character conversion before it
 * is printed out.
 *
 * With a little more time I will merge the outputResponse escape
 * formatting and this one into a single function to handle the
 * print - JG 27/09/96
 */

static void
echoResponseList (char *s)
{
    char c;

    /*
     * The first `""' of a line is ignored to allow line to allow
     * line to commence with white space.
     */
    if (tagBank[0] == NULL) /* Any output ?? */
        return;             /* No - quit now */

    if (fpr == NULL)
    {
        uWarn ("Cannot honour \"echo\" - no output stream active\n");
        return;
    }

    if (*s == '"')
        s++;

    while ((c = *s++) != '\0')
    {
        if (c != '\\')
        {
            fprintf (fpr, "%c", c);
            continue;
        }

        c = *s++;
        switch (c)
        {
        case 'n':
            fprintf (fpr, "%s", eolStr);
            break;
        case 'r':
            break;
        case 't':
            fprintf (fpr, "\t");
            break;
        case '\0':
            s--;
            /* Allow drop into next case */
        case '\\':
            fprintf (fpr, "\\");
            break;
        default:
            fprintf (fpr, "%c", c);
        }
    }
}



static void
outputResponseList (char *name, char *outputFormat)
{
    FILE *fp;
    fEntry *fe;
    fItem *fi;
    TagP tp;
    int c = '\0';
    int uc;
    int newLine = 1;
    char *format;

    /* Work out the output stream */

    uDebug (1,("OutputResponseList\n"));

    if (name == NULL)
        fp = fpr;
    else if ((fp = fopen (name, "wb")) == NULL)
        uFatal ("Cannot open file [%s] for writing\n", name);

    uDebug (1, ("OutputResponseList - about to iterate\n"));

    /* Iterate through all of the tags and push out the output */

    for (tp = tagIterateInit (&tagBank[0]); tp != NULL; tp = tagIterate (tp)) {

        uDebug (1, ("Iterating Name = %s\n", &tp->name[1]));

        fe = (fEntry *)(tp->data);
        fi = fe->item;

        if (alphabeticMode != 0) {
            if (c != tp->name[0]) {
                c = tp->name[0];
                uc = (islower (c)) ? toupper (c) : c;
                fprintf (fp, ".SS %c%s", uc, eolStr);
                newLine = 1;
            }
        }

        format = outputFormat;
        while ((c = *format++) != '\0')
        {
            if (c == '%')
            {
                c = *format++;
                switch (c)
                {
                case 'N' :
                    format = defaultNroffString;
                    break;
                case 'c':
                    if (fi->component != NULL) {
                        fprintf (fp, "%s", fi->component);
                        newLine = 0;
                    }
                    break;
                case 's':
                    fprintf (fp, "%s", tp->section);
                    newLine = 0;
                    break;
                case 'm':
                    if (fi->module != NULL) {
                        fprintf (fp, "%s", fi->module);
                        newLine = 0;
                    }
                    break;
                case 'i':
                    fprintf (fp, "%s", &tp->name[1]);
                    newLine = 0;
                    break;
                case 'd':
                    if ((tp->desc != NULL) && (tp->desc[0] != '\0')) {
                        fprintf (fp, "%s", tp->desc);
                        newLine = 0;
                    }
                    break;
                case 'f':
                    fprintf (fp, "%s", tp->file);
                    newLine = 0;
                    break;
                case 'l':
                    fprintf (fp, "%d", tp->line);
                    newLine = 0;
                    break;
                case 'n':
                    if (newLine == 0) {
                        fprintf (fp, "\n");
                        newLine = 1;
                    }
                    break;
                case '\0':
                    format--;
                    /* Allow drop into next case */
                case '%':
                    fprintf (fp, "%%");
                    newLine = 0;
                    break;
                default:
                    uError ("Bad syntax [%%%c]\n", c);
                    break;
                }
            }
            else if (c == '\\')
            {
                c = *format++;
                switch (c)
                {
                case 'n':
                    fprintf (fp, "%s", eolStr);
                    newLine = 1;
                    break;
                case 'r':
                    break;
                case 't':
                    fprintf (fp, "\t");
                    newLine = 0;
                    break;
                case '\0':
                    format--;
                    /* Allow drop into next case */
                case '\\':
                    fprintf (fp, "\\");
                    newLine = 0;
                    break;
                default:
                    fprintf (fp, "%c", c);
                    newLine = 0;
                }
            }
            else
            {
                fprintf (fp, "%c", c);
                newLine = 0;
            }
        }
    }
    if ((name != NULL) && (fp != NULL))
        fclose (fp);
    uDebug (1, ("OutputResponseList - Exiting\n"));
}

/*
 * pushResponseList
 * Hande a push or pop operation.
 * Push - Move the current response list to the specified bank.
 *        Delete the current response lust.
 * Pop  - Duplicate the back to the current response list.
 */

static int
pushResponseList (int type, int destination)
{
/*    uWarn ("Push/Pop Response [%d] [%d]\n", type, destination);*/
    /* Cannot copy to self !! */
    if (destination == 0)
        return (-1);

    /* Determine the destination buffer */
    if (type == KW_push)
    {
        /* Free current tag list and duplicate from bank */
        free_tags (&tagBank[destination]);
        tagBank [destination] = tagBank [0];
        tagBank[0] = NULL;
        uVerbose (1, "Pushing Bank[0] => Bank[%d]: %d entries.\n",
                  destination, tagCount(tagBank[destination]));

    }
    else
    {
        TagP tp;                        /* Local tag pointer */

        free_tags (&tagBank[0]);        /* Clear current list */

        for (tp = tagIterateInit (&tagBank [destination]);
             tp != NULL;
             tp = tagIterate (tp))
        {
            /* Duplicate tag list */
            if (add_tag (&tagBank[0], tagDup(tp)) != NULL)
                uFatal ("Clone error\n");
         }
        uVerbose (1, "Popping Bank[%d] => Bank[0]: %d entries.\n",
                  destination, tagCount(tagBank[0]));
    }
    return (0);
}

/*
 * flushResponseList
 * Delete the specified response list.
 * A value of -1 flushes all lists.
 */

static void
flushResponseList (int index)
{
/*    uWarn ("Flush Response [%d]\n", index);*/

    if (index == -1)                    /* Clear all lists ?? */
    {
        for (index = 0; index < TAG_BANK_SIZE; index++)
            free_tags (&tagBank [index]);
        uVerbose (1, "Flushing Bank [All]\n");

    }
    else
    {
        free_tags (&tagBank [index]);
        uVerbose (1, "Flushing Bank [%d]\n", index);
    }
}
/*
 * logicalResponseList
 * Logical operators on the response list. Processes sources from the banks
 * [1-9] into the current bank [0]. The current back is cleared first.
 *
 * and:  s1 & s2
 *       Items placed in the current list if they appear in s1 and s2
 *
 * nand: s1 & s2
 *       Items placed in the current list if they apper in s1 but do NOT
 *       appear in s2.
 *
 * or:   s1 & s2
 *       Items placed in the current list if they appear in s1 and s2
 *
 * xor:  s1 & s2
 *       Items placed in the current list if they appear in s1 and NOT s2
 *       or appear in s2 and NOT s1
 */

static void
logicalResponseList (int type, int s1, int s2, int clear)
{
    TagP  tp;                           /* Tage iteration pointer */
    TagP  tf;                           /* Tag found pointer */

/*    uWarn ("Logical Response [%d] [%d] [%d] [%d]\n", */
/*           type, s1, s2, clear);*/

    /* Clear the destination if requested. */
    if (clear)
        free_tags (&tagBank[0]);

    /* Iterate through s1 looking in s2 for a match */
    for (tp = tagIterateInit (&tagBank[s1]);
         tp != NULL;
         tp = tagIterate(tp))
    {
         tf = find_tag (&tagBank[s2], tp->name, tp->section, 0);

         /* Determine if we add tag to list or not */
         if ((type == KW_or) ||
             ((type == KW_and) && (tf != NULL)) ||
             (((type == KW_nand) || (type == KW_xor)) && (tf == NULL)))
         {
             tf = tagDup (tp);
             if (add_tag (&tagBank[0], tf) != NULL)
                 tagFree (tf);          /* Duplicate tag - remove */
         }
    }

    /* If it is xor, then swap arguments around and call as nand */
    if (type == KW_xor)
        logicalResponseList (KW_nand, s2, s1, 0);
}

static int
readLine (FILE *fp, char **cbuf, int *lineNo)
{
static char    buffer [1024];
    int     c;
    int     len;

    if (fp == NULL)
        return (-1);

    *cbuf = buffer;
    buffer [0] = '\0';
    len = 0;
    for (;;)
    {
        c = fgetc (fp);
        if ((c == EOF) || (c == 0x1A)) {
            buffer [len] = '\0';
            if (len == 0)
                return (-1);
            return (len);
        }

        if (c == '\n') {
            *lineNo += 1;
            buffer [len] = '\0';
            return (len);
        }
        else if (c == '\r')
            continue;
        buffer [len++] = c;
    }   /* End of 'while' */
}   /* End of 'readLine' */

static char *
searchSkipWhiteSpace (char *p, int flag)
{
    char c;

    if (p == NULL)
        return NULL;

    /* Skip to next white space marker if flag is set */
    if (flag != 0) {
        while (((c = *p) != '\0') && (c != '\t') && (c != ' '))
            p++;
        if (flag == WHITE_SPACE_TERMINATE)
            if (c != '\0')
                *p++ = '\0';
    }

    /* Skip over white space. */
    while (((c = *p) == '\t') || (c == ' '))
        p++;
    if (c == '\0')
        return NULL;
    return (p);
}

static char *
charDefaults (char *s, char *syntax, int *indexp)
{
    char *p;
    int  index;
#if 0
    uWarn ("charDefaults s = [%s], syntax = [%s]\n",
           (s == NULL) ? "NULL" : s,
           (syntax == NULL) ? "NULL" : syntax);
#endif
    if (syntax == NULL)                 /* End of string expected */
    {
        /* End of string expeccted */
        if ((p = searchSkipWhiteSpace (s, WHITE_SPACE_TERMINATE)) == NULL)
            index = 0;                  /* Ok - looking for EOL */
        else
            index = -1;                 /* Error */
    }
    else
    {
        p = searchSkipWhiteSpace (s, WHITE_SPACE_TERMINATE);
#if 0
        uWarn ("charDefaults s = [%s], p = [%s]\n",
               (s == NULL) ? "NULL" : s,
               (p == NULL) ? "NULL" : p);
#endif
        if (s == NULL)
            index = -1;
        else if (strlen (s) != 1)
            index = -1;
        else
        {
            char c;

            for (index = 0, c = *s; *syntax != '\0'; index++, syntax++)
                if (c == *syntax)
                    break;

            if (*syntax == '\0')        /* Got to end of string so error */
                index = -1;             /* Error state. */
        }
    }
    if (indexp != NULL)
        *indexp = index;
#if 0
    uWarn ("Exiting charDefaults s = [%s], p = [%s] index = %d\n",
           (s == NULL) ? "NULL" : s,
           (p == NULL) ? "NULL" : p,
           index);
#endif
    return (p);
}

static char *
starDefaults (char *s)
{
    if ((s != NULL) && (strcmp (s, "*") == 0))
        return (NULL);
    return (s);
}

static int
processResponse (char *fileName)
{
    FILE *fp;                           /* Working file pointer */
    FILE *ifp = NULL;                   /* Include file pointer stack (of 1) */
    int  lineNo = 0;                    /* Working line number */
    int  ilineNo = 0;                   /* Include line number */
    char *ifileName = NULL;             /* Include file name */

    int  count;                         /* Count of characters read */
    char *buffer;                       /* Character buffer pointer */
    char *s1;                           /* Local string pointers */
    char *s2;
    char *s3;
    int  inResponse = 0;                /* Processing a response */
    int  index;

    if ((fileName == NULL) || (fileName [0] == '\0'))
        uFatal ("No response file specified\n");
    if ((fp = fopen (fileName, "rb")) == NULL)
        uFatal ("Cannot open response file [%s] for reading\n", fileName);
    uFileSet (&lineNo, fileName);

    for (;;)
    {
        /* Handle any include poping */
        if ((count = readLine (fp, &buffer, &lineNo)) < 0)
        {
            if (ifp == NULL)            /* In an include file ?? */
                break;                  /* No - time to exit */

            /* Restore the original file by poping 1 element stack */

            uVerbose (1, "End of [%s] restoring [%s @ %d]\n",
                      fileName, ifileName, ilineNo);
/*            uWarn ("End of [%s] retoring [%s @ %d]\n",*/
/*                   fileName, ifileName, ilineNo);*/
            fclose (fp);                /* Restore file pointer */
            fp = ifp;
            ifp = NULL;
            free (fileName);            /* Restore filename */
            fileName = ifileName;
            lineNo = ilineNo;           /* Restore line number */

            uFileSet (&lineNo, fileName);
            continue;
        }

        /* Search the index table for the keywords */
        for (index = 0; index < KW_MAX; index++)
            if (strncmp (keyTab [index].name, buffer, keyTab [index].size) == 0)
                break;

        /* if we are in an nroff response file only look for KW_Ss */
        if ((nroffResponse == 1) && (inResponse == 0)) {
            if (index == KW_Ss)         /* Legal in nroff response file */
                inResponse = 1;
            else if (index == KW_Se)    /* Illegal if find here */
                uError ("Unexpected \".Se\".\n");
            else                        /* Push rest to response file */
            {
                if (count > 0)
                    fwrite (buffer, 1, count, fpr);
                fprintf (fpr, eolStr);
                destructResponseList ();
            }
        }
        else if (count == 0)              /* Skip blank lines - legal */
            continue;
        else if (index == KW_MAX) {     /* Must be valid response */
            uError ("Syntax Error.\n");
            continue;
        }
        else
        {
            switch (index)
            {
            case KW_Ss:
                uError ("Unexpected \".Ss\".\n");
                break;
            case KW_Se:
                if (nroffResponse == 1)
                    inResponse = 0;
                else
                    uError ("Unexpected \".Se\".\n");
                break;

            case KW_item:
                itemMode = &ihead;
                defaultNroffString [2] = 't';
                break;

            case KW_package:
                itemMode = &phead;
                defaultNroffString [2] = 'p';
                break;

            case KW_include:
            {
                /* Get the filename */
                s1 = searchSkipWhiteSpace (&buffer [keyTab [index].size], 0);
                s2 = searchSkipWhiteSpace (s1, WHITE_SPACE_TERMINATE);
                if ((s1 == NULL) || (s2 != NULL))
                {
                    uError ("Invalid \"include\" syntax - Ignoring\n");
                    break;
                }

                /* Make sure we are not already in an include file */
                if (ifp != NULL)
                {
                    uError ("Nested include - ignoring \"include %s\"\n", s1);
                    break;
                }

                /* Try openning the file */
                ifp = fp;
                if ((fp = fopen (s1, "rb")) == NULL)
                {
                    uFatal ("Cannot open include file [%s] for reading\n", s1);
                    fp = ifp;
                    break;
                }

                /* Push the values */
                ifileName = fileName;   /* Save filename */
                fileName = strdup (s1);

                ilineNo = lineNo;       /* Save the line number */
                lineNo = 0;

                uVerbose (1, "Processing file [%s]\n", fileName);
/*                uWarn ("Processing file [%s]\n", fileName);*/
                uFileSet (&lineNo, fileName);
            }
                break;

            case KW_and:
            case KW_nand:
            case KW_xor:
            case KW_or:
            {

                char *t1;               /* First argument */
                int  s1;                /* Source 1 */
                int  s2;                /* Source 2 */

/*                uWarn ("Logical operation [%s]\n", buffer);*/
                t1 = charDefaults (buffer, "*" , NULL); /* Skip token */
/*                uWarn ("Logical operation (1) [%s]\n", t1);*/
                t1 = charDefaults (t1, "123456789", &s1);
/*                uWarn ("Logical operation (2) [%s] s1 = %d\n", t1, s1);*/
                t1 = charDefaults (t1, "123456789", &s2);
/*                uWarn ("Logical operation (3) [%s] s2 = %d\n", t1, s2);*/

                if ((t1 != NULL) || (s1 == -1) || (s2 == -1))
                    uError ("Invalid arguments on \"%s\"\n",
                            keyTab [index].name);

                s1++;                   /* Correct for string offset */
                s2++;
/*                uWarn ("Logical operation s1 = %d; s2 = %d\n", s1, s2);*/
                logicalResponseList (index, s1, s2, 1);
                uVerbose (1, "Logical [Bank[%d](%d) %s Bank[%d](%d)]"
                             " found %d entries\n",
                          s1, tagCount(tagBank[s1]),
                          keyTab [index].name,
                          s2, tagCount(tagBank[s2]),
                          tagCount(tagBank[0]));
            }
                break;
            case KW_flush:
            {
                char *t1;               /* Current search buffer position */
                int  index;             /* Index of buffer to flush */

                t1 = charDefaults (buffer, "*" , NULL); /* Skip token */
                if (t1 == NULL)
                {
                    flushResponseList (0);
                    break;
                }
                t1 = charDefaults (buffer, "*0123456789" , &index);
                if (t1 != NULL)
                    uError ("Unexpected \"flush\" argument [%s]\n", t1);
                else
                    flushResponseList (index-1);
                break;

#if 0
                t1 = searchSkipWhiteSpace (&buffer [keyTab [index].size], 0);
                t2 = searchSkipWhiteSpace (t1, WHITE_SPACE_TERMINATE);
                if ((strlen (t1) != 1) || (t2 != NULL) ||
                    ((isdigit(*t) == 0) && (*t != '*')))
                {
                    uError ("Unexpected \"flush\" argument [%s]\n", t1);
                    break;
                }

                flushResponseList ((*t == '*') ? -1 : (*t - '0'));
#endif

            }
                break;

            case KW_push:
            case KW_pop:
            {
                char *t1;               /* Current search buffer position */
                char *t2;               /* Next search buffer position */


                if ((t1 = searchSkipWhiteSpace (&buffer [keyTab [index].size],
                                                0)) == NULL)
                {
                    uError ("Buffer number argument expected on \"%s\"\n",
                            keyTab [index].name);
                    break;
                }

                t2 = searchSkipWhiteSpace (t1, WHITE_SPACE_TERMINATE);
                if ((t2 != NULL) || (strlen (t1) != 1) || !isdigit((int)(*t1)))
                {
                    uError ("Unexpected \"%s\" arguments\n",
                            keyTab [index].name);
                    break;
                }

                if (pushResponseList (index, *t1 - '0') != 0)
                    uError ("Invalid buffer number argument on \"%s\"\n",
                            keyTab [index].name);
            }
                break;

            case KW_search:
            case KW_remove:
            {
                char *t1; /* s1 = Search Module */
                char *t2; /* s2 = Search Section */
                char *t3; /* s3 = Search Component */

                s1 = searchSkipWhiteSpace (&buffer [keyTab [index].size], 0);
                s2 = searchSkipWhiteSpace (s1, WHITE_SPACE_TERMINATE);
                s3 =  searchSkipWhiteSpace (s2, WHITE_SPACE_TERMINATE);
                searchSkipWhiteSpace (s3, WHITE_SPACE_TERMINATE);

                uInteractive ("Searching for [%s (%s) : %s]", s1, s2, s3);

                t1 = starDefaults (s1);
                t2 = starDefaults (s2);
                t3 = starDefaults (s3);

                constructResponseList (*itemMode, t1, t3, t2, index);
                uVerbose (0, "Search [%s (%s) : %s] found %d entries\n",
                          s1, s2, s3, tagCount(tagBank[0]));
            }
                break;

            case KW_echo:

                s1 = searchSkipWhiteSpace (&buffer [keyTab [index].size], 0);
                if (s1 == NULL)
                {
                    uError ("Argument expected on \"echo\".\n");
                    break;
                }

                echoResponseList (s1);
                break;

            case KW_output:
                /* S1 = filename */
                /* S2 = Output string */

                s1 = searchSkipWhiteSpace (&buffer [keyTab [index].size], 0);
                s2 = searchSkipWhiteSpace (s1, WHITE_SPACE_TERMINATE);

                s1 = starDefaults (s1);
                if (s2 == NULL)
                {
                    s2 = "\"%N";
                }

                if (*s2 != '\"')
                    uError ("String expected - skipping.\n");
                else
                    outputResponseList (s1, &s2[1]);
                /* I should not really destruct here; but I maintain for
                 * backward compatibility. */
                if (nroffResponse == 0)
                    destructResponseList ();
                break;

            case KW_alpha:
                alphabeticMode = 1;
                break;

            case KW_desc:
                sprintf (defaultNroffString, "%s %s",
                         (itemMode == &ihead) ? ".Ht" : ".Hp",
                         nroffDescriptionString);
                break;

            case KW_nodesc:
                sprintf (defaultNroffString, "%s %s",
                         (itemMode == &ihead) ? ".Ht" : ".Hp",
                         nroffNoDescriptionString);
                break;

            case KW_nonalpha:
                alphabeticMode = 0;
                break;

            case KW_comment:
                break;

            default:
                uError ("Syntax Error.\n");
                break;
            }
        }
    }
    if ((nroffResponse == 1) && (inResponse == 1))
        uError ("Unterminated .Ss.\n");
    uFileSet (NULL, NULL);
    return (0);
}

static void
usage (void)
{
    fprintf (stdout, "%s : Usage %s [Options] <files>\n", progname, progname);
    fprintf (stdout, "\n");
    fprintf (stdout, "-?/h           : Help - this page\n");
    fprintf (stdout, "-a             : Alphabetical index titles.\n");
    fprintf (stdout, "-c <component> : Named component to search for.\n");
    fprintf (stdout, "-d             : Add description to entry\n");
    fprintf (stdout, "-E <file>      : Log errors to <file> (append)\n");
    fprintf (stdout, "-e <file>      : Log errors to <file> (re-write)\n");
    fprintf (stdout, "-I             : Version information.\n");
    fprintf (stdout, "-i <id>        : Named id to search for.\n");
    fprintf (stdout, "-m <module>    : Named module to search for.\n");
    fprintf (stdout, "-n <file>      : Embedded Nroff response file\n");
    fprintf (stdout, "-o <file>      : Output to <file> default is <files>.out\n");
    fprintf (stdout, "-q             : Quiet mode\n");
    fprintf (stdout, "-r <file>      : Standard response file\n");
    exit (1);
}

static void
searchInitialise (void)
{
    static  nrFUNCTION funcTab = {NULL};

    /* Headers */
    nrInstall (funcTab, TH_func, nrTH_func);

    /* Identification */
    nrInstall (funcTab, XI_func, nrXI_func);
    nrInstall (funcTab, XJ_func, nrXI_func);
    nrInstall (funcTab, XP_func, nrXP_func);
    nrInstall (funcTab, Li_func, nrXI_func);
    nrInstall (funcTab, Lj_func, nrXI_func);
    nrInstall (funcTab, Lp_func, nrXP_func);
    nrInstall (funcTab, Ls_func, nrLs_func);
    nrInstall (funcTab, Im_func, nrIm_func);

    /* Files */
    nrInstall (funcTab, startFile_func, NULL);
    nrInstall (funcTab, includeFile_func, nrStartInc);
    nrInstall (funcTab, endFile_func, NULL);

    nrInstallFunctionTable (&funcTab);
}

int main (int argc, char *argv [])
{
    char    *oname = NULL;
    int     ecount = 0;                 /* Error count */
    int     wcount = 0;                 /* Warn count */
    nrFILE  *nrfp;
    int     c;
    int     i;

    /* Configure the error reporting */
    uErrorSet (0, &ecount);             /* Max Num entries + counter */
    uWarnSet (&wcount);                 /* Warning count */
    uUtilitySet (progname);             /* Set name of program */
    
    searchInitialise ();
    sprintf (defaultNroffString, "%s %s", ".Ht", nroffNoDescriptionString);

    while (1) {
        c = getopt (argc, argv, "ac:de:E:h?i:Im:n:o:qr:");
        if (c == EOF)
            break;
        switch (c) {
        case 'a':
            alphabeticMode = 1;
            break;
        case 'c':
            searchComponent = optarg;
            break;
        case 'd':
            sprintf (defaultNroffString, "%s %s",
                     ".Ht", nroffDescriptionString);
            break;
        case 'e':
            uErrorChannelSet (optarg, UERROR_LOG_CREATE);
            break;
        case 'E':
            uErrorChannelSet (optarg, UERROR_LOG_APPEND);
            break;
        case 'h':
        case '?':
            usage();
            break;
        case 'I':
            fprintf (stdout, "%s. Version %s. %s. %s.\n",
                     progname, MODULE_VERSION, __DATE__, nroffVersion);
            exit (0);
            break;
        case 'i':
            searchId = optarg;
            break;
        case 'm':
            searchModule = optarg;
            break;
        case 'o':
            oname = optarg;
            break;
        case 'q':
            uInteractiveSet (0);        /* Disable interactive mode */
            break;
        case 'r':
            responseFile = optarg;
            break;
        case 'n':
            responseFile = optarg;
            nroffResponse = 1;
            break;
        default:
            uError ("Cannot understand option [-%c]\n", c);
            usage();
            break;
        }
    }

    uOpenErrorChannel ();
    if (oname == NULL)
        oname = "nrsearch.out";

    argv = getfiles (&argc, argv, optind);

    if (argc > 1) {
        if (oname != NULL) {
            if ((fpr = fopen (oname, "wb")) == NULL)
                uFatal ("Cannot open file [%s]\n", oname);
            uVerbose (0, "Output in file [%s].\n", oname);
        }
    }
    else
        uFatal ("No files specified.\n");

    /* Process all of the files */
    for (i = 1; i < argc; i++) {
        if ((nrfp = nrFilePush (argv[i])) == NULL)
            exit (1);
        nroff (nrfp, NROFF_MODE_DEFAULT);        /* Set up the conversion */
    }

/*    uDebugSet (15);*/
    uDebug (1, ("Loaded data - starting search"));

    /* Initialise the key table */
    for (i = 0; i < KW_MAX; i++)
        keyTab [i].size = strlen (keyTab [i].name);

    if (responseFile == NULL)
    {
        if (fpr == NULL)
            fpr = stdout;

        constructResponseList (ihead, searchModule, searchComponent,
                               searchId, KW_search);
        outputResponseList (NULL, "%N");
        destructResponseList ();
        if (fpr == stdout)
            fpr = NULL;
    }
    else
        processResponse (responseFile);

    while (ihead != NULL)
        ihead = imItemFree (ihead);
    while (phead != NULL)
        phead = imItemFree (phead);

    if (fpr != NULL) {
        fprintf (fpr, "%s", eofStr);
        fclose (fpr);
    }

    uCloseErrorChannel ();
    return (ecount);
}
