/****************************************************************************
 *
 *			Copyright 1995 Division Limited.
 *			      All Rights Reserved
 *
 *
 *  System        :
 *  Module        :
 *  Object Name   : $RCSfile: tab.c,v $
 *  Revision      : $Revision: 1.1 $
 *  Date          : $Date: 2000-10-21 14:31:31 $
 *  Author        : $Author: jon $
 *  Last Modified : <270496.2121>
 *
 *  Description
 *
 *  Notes
 *
 *  History
 *
 *  $Log: not supported by cvs2svn $
 *  Revision 1.1  1996/09/26 17:53:25  jon
 *  Initial revision
 *
 *
 ****************************************************************************
 *
 *  Copyright (c) 1995 Division Ltd.
 *
 *  All Rights Reserved.
 *
 *  This Document may not, in whole or in part, be copied,
 *  photocopied, reproduced, translated, or reduced to any
 *  electronic medium or machine readable form without prior
 *  written consent from Division Ltd.
 ****************************************************************************/

static const char rcsid[] = "@(#) : $Id: tab.c,v 1.1 2000-10-21 14:31:31 jon Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>

#ifndef _HPUX
#include <getopt.h>
#else
#include <unistd.h>
#endif
#include <utils.h>
#include "_nroff.h"
/*#include "nroff.h"*/

/*
 * 1.0.0  - JG 10/12/95 Orginal
 *
 * 1.0.0a - JG Added multiple library support.
 */

typedef struct {
    char *name;
    int id;
    int data;
} OptionKeywords;

#define TS_OPTION_CENTRE   1
#define TS_OPTION_EXPAND   2
#define TS_OPTION_BOX      3
#define TS_OPTION_DBOX     4
#define TS_OPTION_ABOX     5
#define TS_OPTION_TAB      6
#define TS_OPTION_LINESIZ  7
#define TS_OPTION_TERMINAL 8
#define TS_OPTION_END      0

static OptionKeywords keywords [] =
{
    { "center",    TS_OPTION_CENTRE  , 1 },
    { "expand",    TS_OPTION_EXPAND  , 2 },
    { "box"   ,    TS_OPTION_BOX     , 1 },
    { "doublebox", TS_OPTION_DBOX    , 2 },
    { "allbox",    TS_OPTION_ABOX    , 3 },
    { "tab",       TS_OPTION_TAB     , (int)('\t') },
    { "linesize",  TS_OPTION_LINESIZ , 1 },
    { ";",         TS_OPTION_TERMINAL, 1 },
    { "",          TS_OPTION_END     , 0 }
};

typedef struct {
    char name;
    int id;
} ValueKeywords;

#define TS_COL_BOLD        0x00000001   /* Bold column */
#define TS_COL_ITALIC      0x00000002   /* Italic column */
#define TS_COL_FONT_MASK   0x00000003   /* Font mask */
#define TS_COL_LEFT        0x00000010   /* Left aliged column */
#define TS_COL_RIGHT       0x00000020   /* Right aligned column */
#define TS_COL_CENTRE      0x00000040   /* Centre aligned column */
#define TS_COL_ADJUST_MASK 0x00000070   /* Adjustment mask */
#define TS_COL_EQUAL       0x00000100   /* Equal width column */
#define TS_COL_WIDTH       0x00000200   /* Specified width column */
#define TS_COL_SPAN        0x00000800   /* Span column with next */
#define TS_COL_BORD        0x00001000   /* Right single boarder */
#define TS_COL_DBORD       0x00002000   /* Right double boarder */
#define TS_COL_MAX         32           /* Maximum number of columns */

#define TS_ROW_SPAN        0x00010000   /* Row span */
#define TS_ROW_BORD        0x00020000   /* Row border */
#define TS_ROW_DBORD       0x00040000   /* Row double border */

#define TS_TABLE_BORD      0x01000000   /* Table border */
#define TS_TABLE_DBORD     0x02000000   /* Table double border */

static ValueKeywords vkeywords [] = 
{
    { 'b', TS_COL_BOLD    },
    { 'i', TS_COL_ITALIC  },
    { 'l', TS_COL_LEFT    },
    { 'r', TS_COL_RIGHT   },
    { 'c', TS_COL_CENTRE  },
    { 's', TS_COL_SPAN    },
    { 'w', TS_COL_WIDTH   },
    { 'e', TS_COL_EQUAL   },
    { '|', TS_COL_BORD    },
    { '.', 0              },
    { '\0',0              }
};

/* 
 * Format for the data
 */
typedef struct stEntry {
    struct stEntry *root;               /* Pointer to the root. */
    struct stEntry *head;               /* Pointer to head of row. */
    struct stEntry *row;                /* Pointer to next item in row. */
    struct stEntry *parent;             /* Parent node */
    struct stEntry *col;                /* Pointer to next column */
    int    columnCount;                 /* Number of columns */
    int    column;                      /* Column Id */
    int    curMode;                     /* Current mode of entry */
    float  curWidth;                    /* Current width of entry */
    int    origMode;                    /* Original mode of entry */
    float  origWidth;                   /* Original wifth of entry */
    char   *text;                       /* Pointer to entry text */
} Entry;

static int  tsPosition = 0;                /* margin position of table */
static int  tsBox = 0;                     /* box line type */
static char tsTab = '\t';                  /* tab character */
static int  tsLine = 1;                    /* box line size */
static Entry *tableHead = NULL;            /* Head of the table */

/*
 * entryConstruct
 * Construct a new entry node.
 */
static Entry *
entryConstruct (Entry *root)
{
    Entry *e;
    
    e = malloc (sizeof (Entry));
    if (e == NULL)
        uFatal ("Cannot allocate table entry\n");
    e->root = NULL;
    e->head = NULL;
    e->row = NULL;
    e->col = NULL;
    e->curMode = 0;
    e->curWidth = 0;
    e->origMode = 0;
    e->origWidth = 0;
    e->text = NULL;
    return (e);
}

/*
 * Destruct the entry tree.
 */

static void
treeDestruct (Entry *node)
{
    Entry *col;
    Entry *row;
    
    
    if (node == NULL)
        return;
    do 
    {
        col = node->col;
        do
        {
            row = node->row;
            if (node->text != NULL)
                free (node->text);
            free (node);
        }
        while ((node = row) != NULL);
    } while ((node = col) != NULL);
}        

/*
 * entryLinkRow
 * Link a new item to the row.
 */

static Entry *
entryLinkRow (Entry *last, Entry *node, int flag)
{
    Entry *row;                         /* Head of the row. */
    Entry *parent;                      /* Parent node */
    
    if (node == NULL)
        node = entryConstruct (node);
    
    row = last->head;                   /* Head of the row. */
    node->head = row;                   /* Link to head of row. */
    node->root = last->root;            /* Link to head of table. */
    last->row = node;                   /* Link last to the node */
    
    if ((parent = last->parent) != NULL)
    {
        /* Link to parent. */
        if ((node->parent = parent->row) != NULL)
            parent->row->col = node;    /* Vertical column link */
        else
            uError ("Column linkage inconsistancy\n");
        
        if (flag)
        {
            node->origMode = parent->origMode;
            node->origWidth = parent->origWidth;
        }
    }
    row->columnCount++;                 /* Increment number of columns. */
    node->column = last->column+1;      /* Column identity */
    return (node);
}

/*
 * entryLikCol
 * Link a new item to the column.
 */
static Entry *
entryLinkCol (Entry *last, Entry *node, int flag)
{
    
    if ((last == NULL) || (node == NULL))
        node = entryConstruct ((last == NULL) ? last : last->root);
    if ((node->parent = last) == NULL)
        tableHead = node->root = node;
    else
    {
        node->root = last->root;
        last->col = node;
    }
    node->head = node;                  /* This node is the head !! */
    
    node->column = 1;
    node->columnCount = 1;
    
    if ((node->parent = last) != NULL) /* Link parent + child */
    {
        last->col = node;
        if (flag)
        {
            Entry *p, *q;
            
            node->origMode = last->origMode;
            node->origWidth = last->origWidth;
            
            for (q = node, p = last; p->row != NULL; p = p->row)
                q = entryLinkRow (q, NULL, flag);
        }
    }
    return (node);
}
    
/*
 * nrTSoptions
 * Extract the options from a Nroff text line.
 */

static void 
nrTSoptions (char *line)
{
    char *p;
    OptionKeywords *kp;
    int  id = 0;
    
    uVerbose (0, "nrTSoptions (%s)\n", line);
    tsPosition = 0;                     /* margin position of table */
    tsBox = 0;                          /* box line type */
    tsTab = '\t';                       /* tab character */
    tsLine = 1;                         /* box line size */
    
    while ((p = getFirstParam (&line)) != NULL)
    {
        for (kp = keywords; (id = kp->id) != TS_OPTION_END; kp++)
        {
            if (strncmp (p, kp->name, strlen (kp->name)) == 0)
                break;
        }
        
        switch (id)
        {
        case TS_OPTION_CENTRE:          /* Centre position */
        case TS_OPTION_EXPAND:          /* Flush with margins */
            if (tsPosition != 0)
                uError ("Table position already specified. Ignoring \"%s\".\n", 
                        kp->name);
            else
                tsPosition = kp->data;  /* Set the data */
            break;
        case TS_OPTION_BOX:             /* Enclose table in box */
        case TS_OPTION_DBOX:            /* Enclose table in double box */
        case TS_OPTION_ABOX:            /* Enclose all entries in a box */
            if (tsBox != 0)
                uError ("Box boarder already specified. Ignoring \"%s\".\n", kp->name);
            else
                tsBox = kp->data;       /* Set the data */
            break;
        case TS_OPTION_TAB:
            if ((p[3] == '(') && (p[5] == ')') && (p[6] == '\0'))
                tsTab = p[4];
            else
                uError ("Illegal tab option [%s]. Expect \"tab(x)\"\n", p);
            break;
        case TS_OPTION_LINESIZ:
            if ((p[8] == '(') && (p[10] == ')') && (p[11] == '\0'))
                tsLine = (p[9] - '0');
            else
                uError ("Illegal linesize option [%s]. Expect \"linesize(x)\"\n", p);
            break;
        case TS_OPTION_TERMINAL:        /* Terminal marker */
            break;
        default:
            uError ("Illegal option [%s]\n", p);
            break;
        }
    }
    
    if (id != TS_OPTION_TERMINAL)
        uError ("Table option line not terminated with a semi-colon\n");
}              

static int
getUnitOption (char **line, float *fitem)
{
    char *s, *r;
    char c;
    int status = 1;
    
    s = *line;
    
    if (*++s != '(')
        uError ("Open bracket '(' expected.\n");
    else
    {
        /* Get the length of the string - assume 'i' units. */
        for (r = ++s; (c = *r) != '\0'; r++)
        {
            if (c == 'i')
            {
                *r++ = '\0';
                *fitem = atof (s);
                break;
            }
        }
        
        /* Check the end of the string */
        if (c != 'i')   
            uError ("w(XX.Xi) expected\n");
        if (*r != ')')
            uError ("Close bracked ')' expected.\n");
        else
            status = 0;
        s = r;
    }
    *line = s;
    return (status);
}

static Entry *
nrTSvalues (Entry *e, char *line, int *rstatus)
{
    char *p;
    char c;
    Entry  *r = NULL;
    int status = 0;
    int id;
    int i;
    
    uVerbose (0, "nrTSvalues (%s)\n", line);
    
    while ((p = getFirstParam (&line)) != NULL)
    {
        /* If this is not a column separator then move onto the
           next row */
        if (*p != '|')
        {
            if (r == NULL)
                e = r = entryLinkCol (e, NULL, 0);
            else
                r = entryLinkRow (r, NULL, 0);    /* Make new row item */
        }
        
        /* Process the entry  */
        while ((c = *p) != '\0')
        {
            /* Look up keyword */
            for (i = 0, id = -1; vkeywords[i].name != '\0'; i++)
            {
                if (vkeywords [i].name == c)
                {
                    id = vkeywords [i].id;
                    break;
                }
            }
            /* Check if found */
            if (id == -1)
            {
                uError ("Unrecognised TS format option '%c'\n", c);
                break;
            }
            /* Found keyword - process. */
            switch (id)
            {
            case TS_COL_WIDTH:
                if (getUnitOption (&p, &r->origWidth) != 0)
                    r->origMode |= TS_COL_WIDTH;
                break;
            case TS_COL_BORD:
                if (r->column == 1)
                    uError ("'%s' unexpected in first column\n", p);
                r->origMode |= TS_COL_BORD;
                if (p[1] == '|')        /* Double boarder ?? */
                {
                    r->origMode |= TS_COL_DBORD;
                    p++;
                }
                if (p[1] != '\0')
                    uError ("Unexpected character '%c'\n", p[1]);
                break;
            case 0:
                status = 1;
                break;
            default:
                r->origMode |= id;
                break;
            }
            p++;
        }
    }
    return (e);
}

static int
findTab (char **dest, char **src)
{
    char *p;
    char c;
    
    p = *src;
    if ((c = *p) == '\0')
        return 0;
    
    *dest = p;                          /* Start of the return string */
    do {
        if (c == tsTab)                 /* Tab character ?? */
        {
            *p++ = '\0';                /* Yes - terminate string !! */
            break;                      /* Quit loop */
        }
    } while ((c = *++p) != '\0');       /* Get next char */
    
    *src = p;                           /* Restore new source position. */
    return (1);                         /* Return OK status */
}

static Entry *
nrTSdata (Entry *e, char *line)
{
    Entry *r;
    char *item;
    char c;                             /* Local character pointer */
    
    if (e == NULL)
        r = tableHead;
    else if ((r = e->col) == NULL)      /* Does the next row exist ?? */
        r = entryLinkCol (e, r, 1);     /* No - create a new one */
    e = r;                              /* Last row !! */
    
    c = *line;
    /* Check for horizontal line */
    if (((c == '_') || (c == '=')) && (line[1] == '\0'))
    {
        r->curMode = (c == '_') ? TS_TABLE_BORD : TS_TABLE_DBORD;
        return (r);
    }
    
    while (*line != '\0')
    {
        r->curMode = r->origMode;
        if (findTab (&item, &line) == 0)
        {
            uWarn ("Need to add check here for rows\n");
            break;
        }
        else
        {
            if (strcmp (item, "T{") != 0)
            {
                if (strcmp ("\\^", item) == 0)
                    r->curMode = TS_ROW_SPAN; /* Spanning column */
                else if (strcmp ("\\_", item) == 0)
                    r->curMode = TS_ROW_BORD; /* Horizontal line */
                else if (strcmp ("\\=", item) == 0)
                    r->curMode = TS_ROW_DBORD;/* Horizontal double line */
                else                    /* This MUST be a text entry. Add it. */
                    r->text = strdup (item);
                    
            }
            else
                uFatal ("Cannot cope with complex tables.\n");
        }
        if ((r = r->row) == NULL)
        {
            if (*line == '\0')
                break;
            else
                uError ("Unexpected text at end of line [%s]\n", line);
        }
    }
    return (e);
}
#define EPSILON  0.00001f

static float
computeTextWidth (char *text)
{
    int len;
    
    if ((text == NULL) || ((len = strlen (text)) == 0))
        return (0.0f);
    return ((float) (len) / 10.0f);
}

static void
computeMinWidth (Entry *r, float *width)
{
    
    float f;
    
    f = computeTextWidth (r->text);
    if (r->curWidth < r->origWidth)
        r->curWidth = r->origWidth;
    if (f > r->curWidth)
        r->curWidth = f;
    else
        f = r->curWidth;
    if (f > *width)
        *width = f;
}

static void
nrTSend (void)
{
    Entry *row;
    Entry *col;
    float width [132];
    int i;
    int columns = 0;
    
    for (i = 0, row = tableHead; row != NULL; row = row->row, i++)
    {
        columns++;
        width [i] = 0.0f;
        for (col = row; col != NULL; col = col->col)
            computeMinWidth (col, &width [i]);
    }
    
    for (i = 0; i < columns; i++)
        printf ("Column %d = %6.3f\"\n", i+1, width [i]);
}
                  

static void
nrTSprint (void)
{
    Entry *r;
    Entry *c;
    
    for (c = tableHead; c != NULL; c = c->col)
        for (r = c; r != NULL; r = r->row)
        {
            printf ("Row: %d. Value: [%s] %6.3f\"\n", 
                    r->column, r->text, r->curWidth);
        }
}

int main (int argc, char *argv [])
{
    int     ecount = 0;                 /* Error count */
    int     wcount = 0;                 /* Warn count */
    Entry   *e;
    Entry   *r;
    
    /* Initialise the error channel */
    uErrorSet (0, &ecount);             /* Max Num entries + counter */
    uWarnSet (&wcount);                 /* Warning count */
    uUtilitySet ("table");              /* Set name of program */
              
    uOpenErrorChannel ();
    
    r = e = nrTSvalues  (NULL, strdup ("l l."), NULL);
    r = nrTSdata (NULL, strdup ("ISO 8859-1	west European languages (Latin-1)"));
    r = nrTSdata (r, strdup ("ISO 8859-2	east European languages (Latin-2)"));
    r = nrTSdata (r, strdup ("ISO 8859-3	southeast European and miscellaneous languages (Latin-3)"));
    r = nrTSdata (r, strdup ("ISO 8859-4	Scandinavian/Baltic languages (Latin-4)"));
    r = nrTSdata (r, strdup ("ISO 8859-5	Latin/Cyrillic"));
    r = nrTSdata (r, strdup ("ISO 8859-6	Latin/Arabic"));
    r = nrTSdata (r, strdup ("ISO 8859-7	Latin/Greek"));
    r = nrTSdata (r, strdup ("ISO 8859-8	Latin/Hebrew"));
    r = nrTSdata (r, strdup ("ISO 8859-9	Latin-1 modification for Turkish (Latin-5)"));
    r = nrTSdata (r, strdup ("ISO 8859-10	Lappish/Nordic/Eskimo languages (Latin-6)"));
    nrTSend ();
    nrTSprint ();

#if 0
    nrTSoptions (strdup ("center box linesize(6) tab(@) ;"));
    r = e = nrTSvalues  (NULL, strdup ("l l l l."), NULL);
    r = nrTSdata (NULL, strdup ("1234567890@BBB@CCC@DDD"));
    r = nrTSdata (r, strdup ("EEE@FFF@1234567890@HHH"));
    r = nrTSdata (r, strdup ("III@JJJ@KKK@LLL"));
    r = nrTSdata (r, strdup ("MMM@NNN@OOO@PPP"));
    nrTSend ();
    nrTSprint ();
#endif
#if 0    
    nrTSoptions ("center box linesize(6) tab(@) ;");
    nrTSoptions ("center box linesize(6) tab() ;");
    nrTSoptions ("center allbox box linesize(6) tab(@) ;");
    nrTSoptions ("centre box linesize(6) tab(@) ;");
    nrTSoptions ("fluff center box linesize() tab(@) ;");
    nrTSoptions ("center box linesize(6) tab(@)");
    nrTSvalues ("ci | ci sw(3.5i)");
    nrTSvalues ("ci | ci s");
    nrTSvalues ("ci | ci ci");
    nrTSvalues ("c || l s.");
    nrTSdata   ("_");
#endif    
    uCloseErrorChannel ();
    return (ecount);
}


