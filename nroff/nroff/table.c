/****************************************************************************
 *
 *			Copyright 1995-2004 Jon Green.
 *			      All Rights Reserved
 *
 *
 *  System        :
 *  Module        :
 *  Object Name   : $RCSfile: table.c,v $
 *  Revision      : $Revision: 1.4 $
 *  Date          : $Date: 2004-02-07 19:29:49 $
 *  Author        : $Author: jon $
 *  Last Modified : <231205.1208>
 *
 *  Description
 *
 *  Notes
 *
 *  History
 *
 * 1.0.0b JG 2004-01-03 Ported to Sun Solaris 9
 * 1.0.0a JG 2002-10-21 Added multiple library support.
 * 1.0.0  JG 1995-12-10 Orginal
 *
 ****************************************************************************
 *
 *  Copyright (c) 1995-2004 Jon Green.
 *
 *  All Rights Reserved.
 *
 *  This Document may not, in whole or in part, be copied, photocopied,
 *  reproduced, translated, or reduced to any electronic medium or machine
 *  readable form without prior written consent from Jon Green.
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>

#if ((defined _HPUX) || (defined _LINUX) || (defined _SUNOS))
#include <unistd.h>
#else
#include <getopt.h>
#endif
#include <utils.h>
#include "_nroff.h"
/*#include "nroff.h"*/

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
 * Format for a row.
 */
typedef struct stRow {
    struct stRow *next;                 /* Next row */
    int noColumns;                      /* Number of columns */
    int mode [TS_COL_MAX];              /* Column mode */
    float width [TS_COL_MAX];           /* Column width */
} nrRow;

/*
 * Format for the data
 */
typedef struct stDataLine {
    struct stDataLine *next;            /* Next data line in text */
    char *line;                         /* Line of data */
} DataLine;

typedef struct stDataRowItem {
    struct stDataRowItem *next;         /* Next item in the row */
    int mode;                           /* Mode of row */
    float width;                        /* Width of item */
    DataLine *line;                     /* Pointer to the line list */
} DataRowItem;

typedef struct stDataRow {
    int mode;
    struct stDataRow *next;             /* Next row */
    struct stDataRowItem *row;          /* Pointer to row */
    int numCols;                        /* Number of columns */
} DataRow;

static DataRow *dataRowHead = NULL;
static DataRow *dataRowTail = NULL;
static int  tsPosition = 0;                /* margin position of table */
static int  tsBox = 0;                     /* box line type */
static char tsTab = '\t';                  /* tab character */
static int  tsLine = 1;                    /* box line size */

/*
 * rowConstruct
 * Construct a new row format element and add to tail of the row list.
 */

static nrRow *
rowConstruct (nrRow **head)
{
    nrRow *r;
    nrRow *p;

    r = (nrRow *) malloc (sizeof (nrRow));
    r->noColumns = 0;
    memset (r, 0, sizeof (nrRow));
    r->next = NULL;

    if ((p = *head) == NULL)
        *head = p;
    else
    {
        while (p->next != NULL)
            p = p->next;
        p->next = r;
    }
    return (r);
}

/*
 * rowDestruct
 * Destruct a list of row elements.
 */

static void
rowDestruct (nrRow **head)
{
    nrRow *r;

    while ((r = *head) != NULL)
    {
        *head = r->next;
        free (r);
    }
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

static DataLine *
DataLineAdd (DataRowItem *ri)
{
    DataLine *l;
    DataLine *p;

    l = (DataLine *) malloc (sizeof (DataLine));

    /* Initialise the data */
    l->line = NULL;
    l->next = NULL;
    /* Link to the data row item */
    if ((p = ri->line) == NULL)
        ri->line = l;
    else
    {
        while (p->next != NULL)
            p = p->next;
        p->next = l;
    }
    return (l);
}

static DataRowItem *
DataRowItemAdd (DataRow *r)
{
    DataRowItem *ri;
    DataRowItem *p;

    ri = (DataRowItem *) malloc (sizeof (DataRowItem));

    /* Initialise the data */
    ri->next = NULL;
    ri->mode = 0;
    ri->line = NULL;
    /* Link to the data row */
    if ((p = r->row) == NULL)
        r->row = ri;
    else
    {
        while (p->next != NULL)
            p = p->next;

        p->next = ri;
    }
    return (ri);
}

static DataRow *
dataRowConstruct (void)
{
    DataRow *r;

    r = (DataRow *) malloc (sizeof (DataRow));
    /* Initialise the data */
    r->next = NULL;
    r->row = NULL;
    r->numCols = 0;
    /* Link to end of row list */
    if (dataRowHead == NULL)
        dataRowHead = r;
    else
        dataRowTail->next = r;
    dataRowTail = r;
    return (r);
}

static int
nrTSvalues (char *line)
{
    char *p;
    char c;
    DataRow *r;
    int column = 0;
    int status = 0;
    int id;
    int i;

    uVerbose (0, "nrTSvalues (%s)\n", line);

    r = dataRowConstruct ();

    while ((p = getFirstParam (&line)) != NULL)
    {
        DataRowItem *ri = NULL;

        if (*p != '|')
            ri = DataRowItemAdd (r);    /* Make new row item */

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
                if (getUnitOption (&p, &ri->width) != 0)
                    ri->mode |= TS_COL_WIDTH;
                break;
            case TS_COL_BORD:
                if (column == 0)
                    uError ("'%s' unexpected in first column\n", p);
                else
                    column--;
                ri->mode |= TS_COL_BORD;
                if (p[1] == '|')        /* Double boarder ?? */
                {
                    ri->mode |= TS_COL_DBORD;
                    p++;
                }
                if (p[1] != '\0')
                    uError ("Unexpected character '%c'\n", p[1]);
                break;
            case 0:
                status = 1;
                break;
            default:
                ri->mode |= id;
                break;
            }
            p++;
        }
        column++;
    }
    return (status);
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

static void
nrTSdata (char *line)
{
    DataRow *r;
    DataRowItem *ri;
    DataLine *l;
    char *item;

    line++;
    r = dataRowConstruct ();            /* Construct a new row */

    /* Check for horizontal line */
    if ((line [0] == '_') && (line [1] == '\0'))
    {
        r->mode = TS_ROW_BORD;
        return;
    }
    /* Check for horizontal double line */
    if ((line [0] == '=') && (line [1] == '\0'))
    {
        r->mode = TS_ROW_DBORD;
        return;
    }

    while (*line != '\0')
    {
        if (findTab (&item, &line) == 0)
        {
            uWarn ("Need to add check here for rows\n");
            break;
        }
        else
        {
            ri = DataRowItemAdd (r);

            if (strcmp (item, "T{") == 0)
            {

                if (strcmp ("\\^", item) == 0)
                    ri->mode = TS_ROW_SPAN; /* Spanning column */
                else if (strcmp ("\\_", item) == 0)
                    ri->mode = TS_ROW_BORD; /* Horizontal line */
                else if (strcmp ("\\=", item) == 0)
                    ri->mode = TS_ROW_DBORD;/* Horizontal double line */
                else
                {
                    /* This MUST be a text entry. Add it. */
                    l = DataLineAdd (ri);   /* Construct holder */
                    l->line = strdup (item);/* Add the line. */
                }
            }
        }

    }
}

int
main (int argc, char *argv [])
{
    int     ecount = 0;                 /* Error count */
    int     wcount = 0;                 /* Warn count */

    /* Initialise the error channel */
    uErrorSet (0, &ecount);             /* Max Num entries + counter */
    uWarnSet (&wcount);                 /* Warning count */
    uUtilitySet ("table");              /* Set name of program */

    uOpenErrorChannel ();

    nrTSoptions (bufNStr (NULL, "center box linesize(6) tab(@) ;"));
    nrTSoptions (bufNStr (NULL, "center box linesize(6) tab() ;"));
    nrTSoptions (bufNStr (NULL, "center allbox box linesize(6) tab(@) ;"));
    nrTSoptions (bufNStr (NULL, "centre box linesize(6) tab(@) ;"));
    nrTSoptions (bufNStr (NULL, "fluff center box linesize() tab(@) ;"));
    nrTSoptions (bufNStr (NULL, "center box linesize(6) tab(@)"));
    nrTSvalues (bufNStr (NULL, "ci | ci sw(3.5i)"));
    nrTSvalues (bufNStr (NULL, "ci | ci s"));
    nrTSvalues (bufNStr (NULL, "ci | ci ci"));
    nrTSvalues (bufNStr (NULL, "c || l s."));
    nrTSdata   (bufNStr (NULL, "_"));
    nrTSdata   (bufNStr (NULL, "This is some test@And some Mode"));

    uCloseErrorChannel ();
    return (ecount);
}
