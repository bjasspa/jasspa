/****************************************************************************
 *
 *			Copyright 1995-2004 Jon Green.
 *			   All Rights Reserved
 *
 *
 *  System        :
 *  Module        :
 *  Object Name   : $RCSfile: nroff.c,v $
 *  Revision      : $Revision: 1.2 $
 *  Date          : $Date: 2004-01-06 00:53:51 $
 *  Author        : $Author: jon $
 *  Last Modified : <040104.0036>
 *
 *  Description
 *
 *  Notes
 *
 *  History
 *
 * 1.3.4a - JG 03/01/04 Ported to Sun Solaris 9
 * 1.3.4  - JG 24/03/98 Added .Gr for graphical objects.
 * 1.3.3e - JG 30/05/97 Added .ie/.el + Id comparitor.
 * 1.3.3d - JG 10/05/97 Added hypertext jumps .Lj, .XJ.
 * 1.3.3b - JG 02/09/96 Extended the .Li to include the .XI stuff
 * 1.3.3a - JG 20/04/97 Lowercase module name.
 * 1.3.3  - JG 09/03/96 Added function binding tables.
 * 1.3.2d - JG 15/02/96 Added .ll and .pl for droff - set line and page length
 * 1.3.2c - JG 17/01/96 Corrected problem of macro expansion in instr.
 * 1.3.2b - JG 16/01/96 Removed comma parameters - groff does not understand.
 * 1.3.2a - JG 10/01/96 Added check to detect un-closed .RS statement.
 * 1.3.2  - JG 09/12/95 Increased global error checking. Added bullets.
 *                      Added support for point size change (.ps)
 *                      Added support for vertical spacing (.vs)
 *                      Fixed .I/BP to use .ne 3 so stop page wrapping.
 * 1.3.1a - JG 16/11/95 Increased error detection rate on \\ statements.
 * 1.3.1  - JG 09/11/95 Corrected double \\ on IP and BP lines.
 * 1.3.0h - JG 09/11/95 Added .sp and .ne to nroff data set.
 * 1.3.0g - JG 09/11/95 Added warning ignore to file
 *                      Use .\" IGNORE to ignore next line error and warning.
 *                      Use .\" IGNORE-WARNING to ignore warnings only.
 *                      Use .\" IGNORE-ERROR to ignore errors only.
 *                      Use .\" ERROR <message> to force an error.
 *                      Use .\" WARNING <message> to force a warning.
 * 1.3.0f - JG 10/08/95 Added the store previous command flag
 * 1.3.0e - JG 09/08/95 Detect spurious arguments on .PP
 * 1.3.0d - JG 09/08/95 Increased checking on text lines.
 * 1.3.0c - JG 05/08/95 Increased checking on .CE statements.
 * 1.3.0b - JG 05/08/95 Increased checking on .Ht and .Hr statements.
 * 1.3.0a - JG 01/08/95 Added string macro .ds.
 * 1.3.0  - JG 13/06/95 Stablised - added version number.
 *
 *
 ****************************************************************************
 *
 *  Copyright (c) 1995-2004 Jon Green.
 *
 *  All Rights Reserved.
 *
 *  This Document may not, in whole or in part, be copied,
 *  photocopied, reproduced, translated, or reduced to any
 *  electronic medium or machine readable form without prior
 *  written consent from Jon Green.
 *
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <utils.h>
#include "_nroff.h"
#include "nroff.h"

#define VERSION_STRING "1.3.4a"
#ifdef _WIN32
#define PLATFORM_STRING "Win32"
#elif defined (_DOS)
#define PLATFORM_STRING "MsDos"
#elif defined (_LINUX)
#define PLATFORM_STRING "Linux"
#elif defined (_SUNOS)
#define PLATFORM_STRING "SunOS"
#else
#define PLATFORM_STRING "Undefined"
#endif

char *nroffVersion = "[Nroff Parser : " VERSION_STRING " (" PLATFORM_STRING ")]";
char *nroffId = "nroff";

/* Local definitions */

#define dprintf(x)                      /* NULL */
#define nrCallback(x,y) (((nrftab != NULL) && (nrftab->x != NULL)) ? \
                         (*(nrftab->x)) y, 1 : 0)

/* Type definitions */

typedef void (* KeyFunc)(void);         /* Key type table */

typedef struct KeyStruct {
    char    *name;
    int     index;
    int     size;
    KeyFunc func;
    long    type;                       /* Type of command */
} KeyType;

typedef struct dsRecord_st {
    struct dsRecord_st *next;
    int len;
    char name[3];
    char *text;
} dsRecord;

/* Construct the key table from the definition file. */

#define EXT_KW(x,y,z,f,p) static void z (void);
#define INT_KW(x,y,z,p)   static void z (void);
#include    "nroff.def"
#undef  EXT_KW
#undef  INT_KW

enum {
#define EXT_KW(x,y,z,f,p) y,
#define INT_KW(x,y,z,p)   y,
#include    "nroff.def"
#undef  EXT_KW
#undef  INT_KW
        KW_MAX
};   /* End of 'enum' */

static KeyType keyTab [] = {
#define EXT_KW(x,y,z,f,p) {x, y, 0, z, p},
#define INT_KW(x,y,z,p)   {x, y, 0, z, p},
#include    "nroff.def"
#undef  EXT_KW
#undef  INT_KW
        {NULL, KW_MAX, 0, NULL, 0}
};

/* Local memory definitions */

static const char rcsid[] = "@(#) : $Id: nroff.c,v 1.2 2004-01-06 00:53:51 jon Exp $";

static dsRecord *dsHead = NULL;
static char *instr;                     /* Instruction buffer */
static nrFUNCTION *nrftab = NULL;       /* Nroff function table */

static int  exitFile;
nrFILE      *nrfp = NULL;
static int  supermaninc = 0;
static int  supermanfile = 0;
static int  includeMode = 0;
static int  nrWarnIgnore = 0;           /* Ignore warning flag */
static int  nrErrorIgnore = 0;
static int  bulletLineSpace = 0;
static long nroffMode = 0;              /* Current nroff mode */
static long nroffCmdMode = 0;           /* Current nroff command mode */
static long nroffLastCmdMode = 0;       /* Last nroff command mode */
static int  nroffCmd = 0;               /* Current nroff command index */
static int  nroffLastCmd = 0;           /* Last nroff command index */
static int  vs_size = VS_DEFAULT;       /* Vertical spacing */
static int  vs_pre_size = 0;            /* Previous vertical spacing */
static int  ps_size = PS_DEFAULT;       /* Point size */
static int  ignoreInput = 0;            /* Ignore the input */

static char *libraryModule = NULL;
static char *pushLibraryModule = NULL;

static int  nrCompiling = NROFF_MODE_DEFAULT; /* Default flag */
static char *THname = NULL;             /* Section Name */
static char *THsection = NULL;          /* Section id */

static char *fontTable[] = {"\\fP", "\\fR", "\\fI", "\\fB", "\\fS", "\\fC", NULL};
static char *fontChar [] = {"P", "R", "I", "B", "S", "C", NULL};
static char *fontNo   [] = {"",  "1", "2", "3", "4", "", NULL};

static void _textline_func (char *line);
static char *nroffIdentifier;           /* Nroff string identifier for cmd */
static int  ieel_cond = 0;              /* .ie .el condition flag */
/*
 * Dynamic String Macro stack mechanism
 */

static dsRecord *
_dsConstruct (char *name, char *data)
{
    dsRecord *p;

    /* Create a new record and add the name and string */

    p = (dsRecord *) malloc (sizeof (dsRecord));
    if ((p->len = strlen (name)) > 2)
    {
        uWarn ("Macro string names are max 2 characters long [%s]. Truncated\n",
                name);
        name [2] = '\0';
        p->len = 2;
    }
    strcpy (p->name, name);
    p->text = bufNStr (NULL, data);

    /* Link to head of the list */
    p->next = dsHead;
    dsHead = p;
    return (p);
}

static dsRecord *
_dsDestruct (dsRecord *p)
{
    dsRecord *q;

    /* Free off the record - fixing up record */
    bufFree (p->text);
    q = p->next;
    free (p);

    /* Return the old next pointer */
    return (q);
}

static void
dsPop (dsRecord *d)
{
    dsRecord *p;

    /* If the destructor is NULL destuct the whole list */
    if (d == NULL)
        while ((p = dsHead) != NULL)
            dsHead = _dsDestruct (p);

    /* if explicit record at the head; remove the head */
    else if (d == dsHead)
        dsHead = _dsDestruct (dsHead);

    /* Otherwise; scan down the linked list and destruct */
    else
        for (p = dsHead; p->next != NULL; p = p->next)
            if (d == p->next) {
                p->next = _dsDestruct (d);
                break;
            }
}

static dsRecord *
dsFind (char *name)
{
    dsRecord *p;
    int i;

    if (name == NULL)
    {
        uWarn ("Macro string search with NULL name\n");
        return (NULL);
    }
    i = strlen (name);

    /* Linear search down the list for the header */
    for (p = dsHead; p != NULL; p = p->next)
        if ((strcmp (name, p->name) == 0) && (i == p->len))
            return (p);                 /* Return found */
    return (p);                         /* Return NULL */
}

static void
dsPush (char *name, char *data)
{
    dsRecord *p;

    /* Find name on list */
    if ((p = dsFind (name)) != NULL)
    {
        /* Warn if found - already defined */
        uWarn (".ds variable [%s] already defined\n", p->name);
        dsPop (p);
    }
    /* Construct a new record */
    _dsConstruct (name, data);
}

/*
 * Im field RE handling
 */

char *
nrImGetFirst (char *comp)
{
    char *p;

    if (comp == NULL)
        return (NULL);
    if (*comp == '\0')
        return (NULL);

    for (p = comp; *p != 0; p++) {
        if ((*p == '|') || (*p == ';')) {
            *p = '\0';
            break;
        }
    }
    return (comp);
}

char **
nrImGetAll (char *comp)
{
    int i;
    int j;
    char *p;
    char **argv;
    char *t;

    /* Check string is there !! */
    if (comp == NULL)
        return (NULL);
    if (*comp == '\0')
        return (NULL);

    t = bufNStr (NULL, comp);
    /* Remove the string separator */
    for (i = 1, j = 0, p = t; *p != 0; p++, j++) {
        if (*p == '|') {
            *p = '\0';
            i++;
        }
    }

    /*
     * Allocate a char * list with enough space
     * for the string in the bottom of it.
     */

    argv = (char **) malloc ((sizeof (char *) * (i+2)) + j + 1);
    argv [i] = NULL;                    /* Terminate argv list */
    p = (char *)(&argv [i+1]);          /* Get start of string area */
    memcpy (p, t, j+1);                 /* Copy in string list */

    /* Set up the argv list */
    for (j = 0; j < i; j++) {
        argv [j] = p;                   /* Point to start of string */
        while (*p++ != '\0')            /* Move to next string start */
            /* NULL */;
    }
    bufFree (t);
    return (argv);
}

int
nrImCmp (char *item, char *ritem)
{
    int len;

    if (ritem == NULL)
        ritem = "misc";
    len = strlen (item);
    if ((strncmp (item, ritem, len) == 0) &&
        ((ritem [len] == '\0') || (ritem [len] == ';')))
        return (0);
    return (1);
}

int
nrImCmpAll (char *item, char **ritem)
{
    if (ritem == NULL)
        return (nrImCmp (item, NULL));

    do {
        if (nrImCmp (item, *ritem) == 0)
            return (0);
    } while (*++ritem != NULL);
    return (1);
}

int
nrImCmpAllC (char *item, char *ritem)
{
    char **argv;
    int  status = 1;

    if ((argv = nrImGetAll (ritem)) != NULL)
    {
        status = nrImCmpAll (item, argv);
        free (argv);
    }
    return (status);
}

/*
 * File handling S/W
 */

nrFILE *
__nrfPush (char *filename, FILE *fp)
{
    nrFILE *p;                          /* Local file pointer */

    p = (nrFILE *) malloc (sizeof (nrFILE));
    strcpy (p->fileName, filename);
    p->lineNo = 0;
    p->prev = nrfp;
    p->superman = 0;
    p->fp = fp;
    uFileSet (&(p->lineNo), p->fileName);
    uInteractive ("Processing %s", p->fileName);
    return (p);
}

nrFILE *nrFilePush (char *filename)
{
    FILE    *fp;
    char    buffer [10];
    int     supermanFlag;

    if (filename == NULL)
        return (NULL);
    if ((fp = fopen (filename, "rb")) == NULL) {
        uError ("Cannot open file [%s] for reading.\n", filename);
        return (NULL);
    }

    /* Test for a superman block - read in first 6 bytes */

    supermanFlag = 0;
    if (fread (buffer, 1, 9, fp) == 9) {
        buffer [9] = '\0';
        if (strcmp (buffer, ".SUPERMAN") == 0)
            supermanFlag = 1;
    }
    fseek (fp, 0L, SEEK_SET);           /* Back to top */

    /*
     * Openned file. - Set up push block - assign superman status.
     */

    nrfp = __nrfPush (filename, fp);
    nrfp->superman = supermanFlag;
    return (nrfp);
}

nrFILE *nrFilePop (void)
{
    nrFILE  *p;

    if ((p = nrfp) != NULL) {
        nrfp = p->prev;
        fclose (p->fp);
        free (p);
    }
    if (nrfp != NULL)
    {
        uFileSet (&(nrfp->lineNo), nrfp->fileName);
        uInteractive ("Processing %s", nrfp->fileName);
    }
    else
        uFileSet (NULL, NULL);
    return (nrfp);
}

int
isvalidstr (char *s)
{
    char    c;

    while ((c = *s++) != '\0')
        if ((c < ' ') || (c >= 127))
            return (0);
    return (1);
}

int
isalnumstr (char *s)
{
    char    c;

/*    if (isnullstr (s))*/
/*        return (1);*/
    while ((c = *s++) != '\0')
        if (isalnum(c) == 0)
            return (0);
    return (1);
}

int
isnullstr (char *s)
{
    if (strcmp (s, "-") == 0)
        return (1);
    return (0);
}

/*
 * Line manipulation functions
 */

int wordLen (char *s)
{
    int     i;
    char    c;

    for (i = 0; ((c = *s++) != '\0') && c != ' '; i++)
        ;
    return (i);
}

int trueStrLen (char *s)
{
    int i;
    int c;

    if (s == NULL)
        return (0);
    i = 0;
    while ((c = *s++) != '\0') {
        switch (c)
        {
        case MONO_CHAR:
        case ITALIC_CHAR:
        case BOLD_CHAR:
        case ROMAN_CHAR:
        case PREV_CHAR:
        case MSPACE_CHAR:
        case ZSPACE_CHAR:
        case CONTINUE_CHAR:
            break;
        case COPYRIGHT_CHAR:
            i += 3;
        case '\0':
            break;
        default:
            i++;
        }   /* End of 'switch' */
    }   /* End of 'while' */
    return (i);
}   /* End of 'trueStrLen' */

void strOverCopy (char *s, int index, int offset)
{
    char    *r;
    int     j;

    for (j = index, r = &s[index]; (*r++ = s[j+offset]) != '\0'; j++)
        ;
}

void
strInsertCopy (char *s, int o, char *i)
{
    int j;
    int ilen;
    int slen;

    if (i == NULL)
        return;
    ilen = strlen (i);
    s = &s[o];
    slen = strlen (s);
    /* Shuffle up the string */
    for (j = slen+1; --j >= 0; s[j+ilen] = s[j])
        ;
    /* Insert the new string */
    for (j = ilen; --j >= 0; s[j] = i[j])
        ;
}

char *leftTrim (char *s)
{
    uDebug (1, ("LTrim[%d][%s]\n", 1, s));
    if (s == NULL)
        return (s);
    while (*s != '\0')
        if (*s == ' ')
            s++;
        else
            break;
    if (*s == '\0')
        s = NULL;
    uDebug (1, ("LTrim[%d][%s]\n", 2, s));
    return (s);
}

char *rightTrim (char *s)
{
    char    *t;

    uDebug (1, ("RTrim[%d][%s]\n", 1, s));
    if (s == NULL)
        return (s);
    for (t = s; *t != '\0'; t++)
        ;
    while (t != s) {
        t--;
        if (*t == ' ')
            *t = '\0';
        else
            break;
    }   /* End of 'while' */
    return (s);
}

int inStr (int start, char *s, char search)
{
    int     i;
    char    c;

    if (s != NULL)
        for (i = 0; (c = s[i]) != 0; i++)
            if ((c == search) && (i >= start))
                return (i);
    return (-1);
}

char *stripBackSlash (int mode, char *s, char **next)
{
    int     i;
    char    *r;

    if (s == NULL || *s == '\0')        /* Check for no input */
        return (NULL);

    i = inStr (0, s, '\\');             /* Look for '\' character */
    while (i >= 0) {
/*fprintf (stderr, "StripBackSlash:[%s]\n", s);*/
        strOverCopy (s, i, 1);          /* Junk backslash character */
        switch (s[i])
        {
        case 's':                       /* Small font */
            if ((s[i+1] == '+') || (s[i+1] == '-'))
            {
                if (isdigit (s[i+2]))
                    strOverCopy (s, i, 3);      /* Remove the 's+n' */
                else
                    uError ("\\s[+-]n required\n");
            }
            else if (isdigit (s[i+1]))
                strOverCopy (s, i, 2);      /* Remove the 'sn' */
            else
                uError ("Unrecognised \\s%c option\n", s[i+1]);
            break;
        case 'w':                       /* Width - terminate line */
            s [i] = '\0';
            break;
        case 'v':
            if (s[i+1] != '\'')
            {
                strOverCopy (s, i, 1);  /* Remove the 'v' */
                uError ("\v'<width>' - initial quote missing.\n");
            }
            else
            {
                int j = 2;
                char lc;

                for (;;)
                {
                    if ((lc = s[i+j]) == '\0')
                    {
                        uError ("\v'<width>' - terminal qote missing.\n");
                        break;
                    }
                    if (lc == '\'')
                    {
                        strOverCopy (s, i, j);
                        break;
                    }
                    j++;
                }
            }
            break;
        case 'c':                       /* Width - terminate line */
            s [i] = CONTINUE_CHAR;      /* Continuation character */
            break;
        case ' ':
            s[i] = USPACE_CHAR;
            break;
        case '^':                       /* [1/12 em] Micro space character */
        case '|':                       /* [1/6 em]  Micro space character */
            s[i] = MSPACE_CHAR;
            break;
        case '&':
#if 0
            strOverCopy (s, i, 1);      /* Remove the '&' */
            i = inStr (i, s, '\\');     /* Look for '\' character */
            continue;
#else
            s[i] = ZSPACE_CHAR;
            break;
#endif
        case '0':                       /* Zero width space. */
            s[i] = ' ';
            break;
        case '(':
            if ((s[i+1] == 'b') && (s[i+2]== 'u'))
            {
                s[i] = BULLET_CHAR;
                strOverCopy (s, i+1, 2);
                break;
            }
            if ((s[i+1] == 'c') && (s[i+2]== 'o'))
            {
                s[i] = COPYRIGHT_CHAR;
                strOverCopy (s, i+1, 2);
                break;
            }
            if ((s[i+1] == 'e') && (s[i+2]== 'm'))
            {
                s[i] = '-';
                s[i+1] = '-';
                strOverCopy (s, i+2, 1);
                break;
            }
            if ((s[i+1] == 'l') && (s[i+2]== 'q'))
            {
                s[i] = '`';
                strOverCopy (s, i+1, 2);
                break;
            }
            if ((s[i+1] == 'r') && (s[i+2]== 'q'))
            {
                s[i] = '\'';
                strOverCopy (s, i+1, 2);
                break;
            }
            break;
        case '*': {
            dsRecord *p;
            char *dsname;
            int dslen;
            int reEvaluate = 0;

            if (s[i+1] == '(')
            {
                dsname = bufChr (NULL, s[i+2]);
                dsname = bufChr (dsname, s[i+3]);
                dslen = 4;
            }
            else
            {
                dsname = bufChr (NULL, s[i+1]);
                dslen = 2;
            }
            strOverCopy (s, i, dslen);
            uDebug (2, (".ds search for [%s]. Line is\n[%s]\n", dsname, &s[i]));

            if ((p = dsFind (dsname)) == NULL)
                uError ("Cannot find string macro name [%s]\n", dsname);
            else
            {
                /*
                 * If the input string is semi truncated then allow it to
                 * grow. This does assume that the string has enough space
                 * to grow !!.
                 */
                char *nextItem;

                if (next != NULL)
                {
                    dslen = strlen (p->text);
                    nextItem = *next;
                    strInsertCopy (nextItem, 0, p->text);
                    *next = &nextItem [dslen];
                }
                strInsertCopy (s, i, p->text);
                reEvaluate = 1;

            }
            uDebug (1, (".ds found for [%s]. Line is\n[%s]\n", dsname, &s[i]));
            bufFree (dsname);
            if (reEvaluate != 0)
            {
                i = inStr (i, s, '\\');
                continue;
            }
        }
            break;
        case 'f':
            if (nroffMode & CMD_MCS)
                uWarn ("Font change with \"\\fX\" is "
                       "not allowed in a \".CS\" block\n");
            if (s[i+1] == 'B' || s[i+1] == 'C' ||
                s[i+1] == 'I' || s[i+1] == 'R' ||
                s[i+1] == 'P' || s[i+1] == '1' ||
                s[i+1] == '2' || s[i+1] == '3' ||
                s[i+1] == '4')
            {
                strOverCopy (s, i, 1);
                switch (s[i])
                {
                case 'C':
                case '4':
                    s[i] = MONO_CHAR;
                    break;
                case 'B':
                case '3':
                    s[i] = BOLD_CHAR;
                    break;
                case 'I':
                case '2':
                    s[i] = ITALIC_CHAR;
                    break;
                case 'R':
                case '1':
                    s[i] = ROMAN_CHAR;
                    break;
                case 'P':
                    s[i] = PREV_CHAR;
                    break;
                }   /* End of 'switch' */
            }
            else
                uWarn ("Font change to \"\\f%c\" is not recognised\n", s[i+1]);
            break;
        case 'n':
            uWarn ("\\n Register interpolation N/S. Require \"\\\\n\" ?\n");
                break;
        case 't':
            s [i] = '\t';
            break;
        case 'b':
            uWarn ("\\b - Bracket building function N/S. "
                   "Require \"\\\\b\" ?\n");
            break;
        case 'r':
            uWarn ("\\r - Reverse 1-em motion N/S. Require \"\\\\r\" ?");
            break;
        case 'e':
            s[i] = ESC_CHAR;            /* Escape character */
            break;
        case '\\':
            s[i] = '\\';                /* Back slash */
            break;
        case '-':                       /* Hyphen character */
        case '`':                       /* Grave accent character */
        case '\'':                      /* Accute accent character */
        case '.':                       /* Period (dot) */
            break;
        case '"':
            s[i] = '\0';                /* Comment */

        default:
            uWarn ("\\%c [%#04x] - Unsupported escape sequence\n",
                   s[i], s[i] & 0x00ff);
            break;
        }
        i = inStr (i+1, s, '\\');
    }
    if (mode == USPACE_CHAR)
        for (r = s; *r != '\0'; r++)
            if (*r == ' ')
                *r = USPACE_CHAR;
    return (s);
}

char *getFirstParam (char **sp)
{
    char    *r;
    int     len;
    int     i;
    char    *instr;

    instr = *sp;
    r = NULL;
    instr = leftTrim (instr);        /* Trim white space at SOL */
    uDebug (1, ("[%d][%s]\n", 1, instr));
    /*
     * Process the line while it is not empty.
     */

    if (instr != NULL && *instr != '\0') {
        len = strlen (instr);
        /*
         * Process quoted argument.
         */
        if (instr[0] == '"') {
            i = inStr (1, instr, '"');
            if (i > 0) {
                r = &instr [1];
                instr [i] = '\0';
                instr = &instr [i+1];
                r = stripBackSlash (0, r, &instr);
            }
            else
                uError ("Closing bracket mis-match.\n");
        }
        /*
         * Try processing a comment.
         */
        else if ((instr [0] == '\\') && (instr [1] == '"'))
        {
            instr = NULL;
        }
        /*
         * Process unquoted argument.
         */
        else
        {
            i = inStr (0, instr, ' ');
            if (i > 0) {
                r = &instr [0];
                instr [i] = '\0';
                if ((i > 0) && (instr [i-1] == '"'))
                    uError ("Opening bracket mis-match before next argument.\n");
                instr = &instr [i+1];
                uDebug (1, ("[%d][%s]\n", 2, instr));
                uDebug (1, ("[%d][%s]\n", 3, r));
                r = stripBackSlash (0, r, &instr);
                uDebug (1, ("[%d][%s]\n", 4, r));
            }
            else {
                if ((len > 0) && (instr [len-1] == '"'))
                    uError ("Opening bracket mis-match before <EOL>.\n");
                r = stripBackSlash (0, instr, NULL);
                uDebug (1, ("[%d][%s]\n", 5, r));
                instr = NULL;
            }
        }
    }
    uDebug (1, ("[%d][%s]\n", 6, r));
    *sp = instr;
    if ((r != NULL) && (*r == '\0'))    /* May be a comment !! */
        r = NULL;                       /* Yes - truncate rest of line */
    return (r);
}

char *getRestOfLineParam (char **sp)
{
    instr = *sp;
    instr = rightTrim (leftTrim (instr));
    if (instr != NULL)
    {
        if (*instr == '"')
            instr++;
        else if ((*instr == '\\') && (instr [1] == '"'))
            instr = NULL;               /* Comment */
    }
    *sp = instr;
    return (instr);
}

char *getAllParam (char **sp)
{
    char    *s;
    char    *d;
    char    *first;
    int     len;
    char    c;

    if ((first = getFirstParam(sp)) != NULL) {
        while ((s = getFirstParam(sp)) != NULL) {
            len = strlen (first);
            d = &first [len];
            *d++ = ' ';
            while ((c = *s++) != '\0')
                *d++ = c;
            *d = '\0';
        }
    }
    return (first);
}

char *getOnlyParam (char **sp)
{
    char *instr;
    char *first;

    if ((first = getFirstParam(sp)) != NULL)
    {
        instr = getAllParam (sp);
        *sp = instr;
    }
    return (first);
}

static void
nrCheck (void)
{
    if (nroffCmdMode & (CMD_SEC|CMD_PAR))
    {
        uDebug (2, ("Nroff Mode = %#010lx\n", nroffMode));
        if ((nroffMode & CMD_MRS) != 0)
        {
            uError (".RS Blocks remains open (level %ld). Require .RE statement\n",
                    (long)(CMD_MRS_GET(nroffMode)));
            nroffMode &= ~CMD_MRS;
        }

        if (nroffMode & CMD_MCS)
        {
            uError (".CS Block remains open. Require .CE statement.\n");
            nroffMode &= ~CMD_MCS;      /* Fix */
        }
        if (nroffMode & CMD_MBS)
        {
            uError (".BS Block remains open. Require .BE statement.\n");
            nroffMode &= ~CMD_MBS;
        }
        if (nroffMode & CMD_MAD)
        {
            uError ("Right justification (.na) disabled. Enable with .ad\n");
            nroffMode &= ~CMD_MAD;
        }
        if (nroffMode & CMD_MFI)
        {
            uError ("Fill mode (.nf) disabled. Enable with .fi\n");
            nroffMode &= ~CMD_MFI;
        }
        if (ps_size != PS_DEFAULT)
        {
            uError ("Point size (.ps) set to %d. Restore with .ps %d\n",
                     ps_size, PS_DEFAULT);
            ps_size = PS_DEFAULT;
        }
        if (vs_size != VS_DEFAULT)
        {
            uError ("Vertical size (.vs) set to %d. Restore with .vs %d\n",
                     vs_size, VS_DEFAULT);
            vs_pre_size = vs_size;      /* Force previous */
            vs_size = VS_DEFAULT;       /* Set default */
        }
    }

    if (nroffCmdMode & CMD_SPA)
    {
        /*
         * PP/LP/IP/BP/TP are a bit strange. On most implementations it
         * modifies the point size and the vertical spacing. Need to
         * make sure that no sizing operations are in effect.
         */
        if (ps_size != PS_DEFAULT)
        {
            uError ("Point size (.ps) set to %d. Restore with .ps %d\n",
                     ps_size, PS_DEFAULT);
            ps_size = PS_DEFAULT;
        }
        if (vs_size != VS_DEFAULT)
        {
            uError ("Vertical size (.vs) set to %d. Restore with .vs %d\n",
                     vs_size, VS_DEFAULT);
            vs_pre_size = vs_size;      /* Force previous */
            vs_size = VS_DEFAULT;       /* Set default */
        }
    }

    if (((nroffMode & CMD_MCS) != 0) && ((nroffCmdMode & ~CMD_PPO) != 0))
        uError (".CS block open - Illegal nroff command [%s].\n",
                 keyTab [nroffCmd].name);

    if ((nroffMode & CMD_MCS) == 0)
    {
        if ((nroffCmdMode & CMD_PRE) && (nroffMode & CMD_PST))
        {
            if (nroffLastCmd == KW_CE)
                uWarn ("Double space condition detected. Fix with .CE 0\n");
            else if (nroffLastCmd == KW_BE)
                uWarn ("Double space condition detected. Fix with .BE 0\n");
            else
                uWarn ("Double space condition detected.\n");
        }
    }

    if ((nroffCmdMode & CMD_INV) == 0)
    {
        nroffMode &= ~CMD_PPO;
        if (nroffCmdMode & CMD_PST)
            nroffMode |= CMD_PST;
    }

    if ((nroffCmdMode & CMD_MTP) != 0)
        uError ("Expected text following .TP operation\n");
}

static void
FH_func (void)
{
    nrCallback (FH_func, ());
    if (supermanfile == 0)
        exitFile = 1;

    if ((nroffMode & (CMD_MID|CMD_MIM|CMD_MXI|CMD_MTH|CMD_MSH)) !=
        (CMD_MID|CMD_MIM|CMD_MXI|CMD_MTH|CMD_MSH))
        uError (".FH Placement error\n");
    if (getAllParam (&instr) != NULL)
        uWarn (".FH Unexpected arguments\n");
    nrCheck ();
    nroffMode |= CMD_MFH;
}

static void
Id_func (void)
{
    char *s;

    getFirstParam (&instr);             /* Ignore */
    getFirstParam (&instr);             /* Ignore */
    getFirstParam (&instr);             /* Ignore */
    s = getFirstParam (&instr);         /* Get the date. */
    if (s == NULL)
        uError ("RCS Date string expected YYYY/MM/DD\n");
    else if (!(isdigit (s[0]) &&
               isdigit (s[1]) &&
               isdigit (s[2]) &&
               isdigit (s[3]) &&
               s[4] == '/' &&
               isdigit (s[5]) &&
               isdigit (s[6]) &&
               s[7] == '/' &&
               isdigit (s[8]) &&
               isdigit (s[9]) &&
               s[10] == '\0'))
    {
        uError ("Invalid date string format [%s] expected YYYY/MM/DD\n", s);
    }
    else
    {
        dsRecord *p;
        /* Add the date to the macro string buffer */

        if ((p = dsFind ("Dt")) != NULL)
            dsPop (p);
        dsPush ("Dt", s);
    }

    nrCallback (Id_func, (s));
    if (nroffMode != 0)
        uError (".Id Error - should be placed at start of file.\n");
    nroffMode |= CMD_MID;
}

static void
Im_func (void)
{
    char    *module;
    char    *class;
    char    *p;

    if ((module = bufNStr (NULL, getFirstParam (&instr))) == NULL)
        uWarn ("Module assingment expected.\n");
    else
    {
        for (p = module; *p != '\0'; p++)
            if (isupper (*p))
                *p = tolower (*p);
    }
    class = bufNStr (NULL, getFirstParam (&instr));
    if (getAllParam (&instr) != NULL)
        uWarn (".Im Unexpected arguments - maximum of 2 arguments expected\n");

    nrCallback (Im_func, (module, class));

    bufFree (module);
    bufFree (class);

    if ((nroffMode != CMD_MID))
        uError (".Im placement - follows .Id statement.\n");
    nroffMode |= CMD_MIM;
}

static void
Kw_func (void)
{
    int     argc;
    char    **argv;
    char    *p;

    argv = bufNArg (NULL, &argc, NULL);
    while ((p = bufNStr (NULL, getFirstParam (&instr))) != NULL)
        argv = bufNArg (argv, &argc, p);

    if (argc == 0)
        uWarn ("1 or more arguments expected\n");

    nrCallback (Kw_func, (argv));

    if (nroffMode & CMD_MSH)
        uError (".Kw expected before .SH/SS statement.\n");
    bufNArgFree (argv, argc);
}

static void
TH_func (void)
{
    char    *sid;
    char    *snum;
    char    *sdate;
    char    *stitle;
    char    *scompany;

    THname = bufFree (THname);
    THsection = bufFree (THsection);

    if ((sid = bufNStr (NULL, getFirstParam (&instr))) == NULL) {
        uWarn ("Name string expected. Param #1.\n");
        sid = bufNStr (NULL, "ERROR");
    }
    if ((snum = bufNStr (NULL, getFirstParam (&instr))) == NULL)
        uWarn ("Number expected. Param #2.\n");
    else if (strcmp(snum, "-") == 0)
        snum = bufFree (snum);

    if ((sdate = bufNStr (NULL, getFirstParam (&instr))) != NULL)
        if (strcmp(sdate, "-") == 0)
            sdate = bufFree (sdate);

    if ((scompany = bufNStr (NULL, getFirstParam (&instr))) != NULL)
        if (strcmp(scompany, "-") == 0)
            scompany = bufFree (scompany);
    stitle = bufNStr (NULL, getFirstParam (&instr));

    uDebug (1, ("TH [%s][%s][%s][%s][%s]\n", sid, snum, sdate, scompany, stitle));

    nrCallback (TH_func, (sid, snum, sdate, scompany, stitle));
    bufFree (stitle);
    bufFree (scompany);
    bufFree (sdate);

    THsection = snum;
    THname = sid;

    if (nroffMode != (CMD_MID|CMD_MIM))
        uError (".TH placement Error\n");
    nroffMode |= CMD_MTH;

    vs_size = VS_DEFAULT;       /* Vertical spacing */
    vs_pre_size = 0;            /* Previous vertical spacing */
    ps_size = PS_DEFAULT;       /* Point size */
}

static void
Gr_func (void)
{
    char *align;
    char *image;
    char *ending;

    if ((align = bufNStr (NULL, getFirstParam (&instr))) == NULL)
    {
        uError ("Alignment string expected. Param #1.\n");
        align = "l";
    }
    if (!((align[0] == 'l') || (align[0] == 'r') || (align[0] == 'c')))
    {
        uError ("Alignment string of 'l', 'r', 'c' expected. Param #1. Got %s.\n", align);
        align[0] = 'l';
    }
    if ((image = bufNStr (NULL, getFirstParam (&instr))) == NULL)
    {
        uError ("Image string expected. Param #2n\n");
        image = bufNStr (NULL, "logo");
    }

    if ((ending = getAllParam (&instr)) != NULL)
        uError ("Unexpected arguments [%s]\n", ending);

    nrCallback (Gr_func, (align, image));
    bufFree (ending);
    bufFree (image);
    bufFree (align);
}

static void
NH_func (void)
{
    char    *sid;
    char    *snum;
    char    *stitle;
    char    *xrefname;
    char    *ending;

    if ((sid = bufNStr (NULL, getFirstParam (&instr))) == NULL) {
        uError ("Name string expected. Param #1.\n");
        sid = bufNStr (NULL, "ERROR");
    }

    if ((snum = bufNStr (NULL, getFirstParam (&instr))) == NULL)
        uError ("Number expected. Param #2.\n");
    else if (strcmp(snum, "-") == 0)
        snum = bufFree (snum);
    else if (isalnumstr (snum) == 0)
        uError ("Alphanumeric expected in param #2. Found [%s].\n",
                 snum);

    if ((stitle = bufNStr (NULL, getFirstParam (&instr))) == NULL)
        uError ("Title expected. Param #3.\n");

    xrefname = bufNStr (NULL, getFirstParam (&instr));
    if ((ending = getAllParam (&instr)) != NULL)
        uError ("Unexpected arguments [%s]\n", ending);

    uDebug (1, ("NH [%s][%s][%s]\n", sid, snum, stitle));
    /*
     * If we are compiling then try and resolve the xref name.
     */

    if (xrefname == NULL)
        nrResolveExternalReference ((nrCompiling & NROFF_MODE_COMPILE) ? LS_COMPILE : 0,
                                    sid, snum, NULL, &xrefname);
    /*
     * Call the user function.
     */
    nrCallback (NH_func, (sid, snum, stitle, xrefname));
    bufFree (stitle);
    bufFree (snum);
    bufFree (sid);
    bufFree (xrefname);
    nrCheck ();
}

static void
GH_func (void)
{
    char    *title;

    if ((title = bufNStr (NULL, getAllParam (&instr))) == NULL)
        uError ("Title name expected.\n");
    nrCallback (GH_func, (title));
    bufFree (title);
    nrCheck ();
}

static void
SH_func (void)
{
    char *s;

    if ((s = getOnlyParam(&instr)) == NULL)
        uError (".SH with no argument.\n");
    if (instr != NULL)
        uWarn (".SH Arguments should be quoted with \"...\" - Truncated.\n");

    nrCallback (ne_func, (3));          /* Need 3 lines */
    nrCallback (SH_func, (s));          /* Call title line */

    nroffMode |= CMD_MSH;
    if ((nroffMode & (CMD_MID|CMD_MIM|CMD_MXI|CMD_MTH)) !=
        (CMD_MID|CMD_MIM|CMD_MXI|CMD_MTH))
        uError (".SH Placement error\n");
    nrCheck ();
}

static void
SS_func (void)
{
    char *s;

    if ((s = getOnlyParam (&instr)) == NULL)
        uError (".SS with no argument.\n");
    if (instr != NULL)
        uWarn (".SS Arguments should be quoted with \"...\" - Truncated.\n");

    nrCallback (ne_func, (3));          /* Need 3 lines */
    nrCallback (SS_func, (s));          /* Call title line */

    nroffMode |= CMD_MSH;
    if ((nroffMode & (CMD_MID|CMD_MIM|CMD_MXI|CMD_MTH)) !=
       (CMD_MID|CMD_MIM|CMD_MXI|CMD_MTH))
        uError (".SS Placement error\n");
    nrCheck ();
}

static void
PD_func (void)
{
    int  i;
    char *p;

    i = 1;
    if ((p = getFirstParam (&instr)) != NULL) {
        if (isdigit (*p))
            i = (int)(*p - '0');
    }
    if (getAllParam (&instr) != NULL)
        uWarn (".PD Unexpected arguments\n");
    nrCallback (PD_func, (i));          /* Call title line */
}

static void
PP_func (void)
{
    nrCallback (PP_func, ());
    if (getAllParam (&instr) != NULL)
        uWarn (".PP Unexpected arguments\n");
    nrCheck ();
}

static void
Hl_func (void)
{
    char    *buftext;
    char    *bufname;
    char    *bufnum;
    char    *catdata;
    char    *modulename;
    char    *filename;
    int     linkType;

    buftext = bufNStr (NULL, getFirstParam (&instr));
    if (buftext == NULL)
        uError ("Name string expected\n");

    if ((bufname = bufNStr(NULL, getFirstParam (&instr))) != NULL) {
        if (strcmp (bufname, "-") == 0)
        {
            bufFree (bufname);
            bufname = bufNStr (NULL, buftext);
        }
        if (isvalidstr (bufname) == 0)
            uError ("Invalid string in param #2. Found [%s].\n", bufname);
    }

    if ((bufnum = bufNStr (NULL, getFirstParam (&instr))) != NULL) {
        if (isnullstr (bufnum) != 0)
            bufnum = bufFree (bufnum);
        else if (isalnumstr (bufnum) == 0)
            uWarn ("Alphanumeric expected in param #3. Found [%s].\n", bufnum);
    } else
        uWarn ("Section number expected. Param #3.\n");

    if ((catdata = bufNStr (NULL, getFirstParam (&instr))) != NULL) {
        if (isnullstr (catdata) != 0)
            catdata = bufFree (catdata);
    }

    modulename = bufNStr (NULL, getFirstParam (&instr));  /* Module name */
    filename = bufNStr (NULL, getFirstParam (&instr));  /* File name */

    if (getAllParam (&instr) != NULL)
        uError ("Additional .Hl or .Hh parameters ignored, last valid field [%s].\n",
                 catdata);

    uDebug (1, ("Hl/h [%s][%s][%s][%s]\n", bufname, bufnum, buftext, catdata));

    /*
     * Resolve the external references.
     * Call the user function with the data that has been collected.
     */

    linkType = nrResolveExternalReference ((nrCompiling & NROFF_MODE_COMPILE) ? LS_COMPILE : 0,
                                           bufname, bufnum, &modulename, &filename);
    if (((nrCompiling & NROFF_MODE_COMPILE) != 0) &&
        (filename == NULL))
        uError ("Cannot resolve external reference to [%s%s]\n",
                 (bufname == NULL) ? "NoName" : bufname,
                 (bufnum == NULL) ? "" : bufnum);

    if (nroffCmd == KW_Hl) {
        nrCallback (Hl_func, (buftext, bufname, bufnum,
                              catdata, modulename, filename, linkType));
    } else {
        nrCallback (Hh_func, (buftext, bufname, bufnum,
                              catdata, modulename, filename, linkType));
    }
    if (nroffMode & CMD_MFI)
        nrCallback (br_func, ());

    bufFree (buftext);
    bufFree (bufname);
    bufFree (bufnum);
    bufFree (catdata);
    bufFree (modulename);
    bufFree (filename);
    nroffMode &= ~CMD_PPO;
    if (nroffMode & CMD_MTP)
        TP_func ();
    nrCheck ();
}

static void
Hh_func (void)
{
    Hl_func ();
}

static void
_Htp_func (int package)
{
    char    *bufname;
    char    *bufnum;
    char    *catdata;
    char    *modulename;
    char    *filename;
    int     linkType;

    if ((bufname = bufNStr(NULL, getFirstParam (&instr))) != NULL) {
        if (isvalidstr (bufname) == 0)
            uWarn ("Invalid string in param #1. Found [%s].\n", bufname);
    }

    if ((bufnum = bufNStr (NULL, getFirstParam (&instr))) != NULL) {
        if (isalnumstr (bufnum) == 0)
            uWarn ("Alphanumeric expected in param #2. Found [%s].\n",
                   bufnum);
    } else
        uWarn ("Section number expected. Param #2.\n");

    if ((catdata = bufNStr (NULL, getFirstParam (&instr))) != NULL) {
        if (isnullstr (catdata) != 0)
            catdata = bufFree (catdata);
    }

    modulename = bufNStr (NULL, getFirstParam (&instr));  /* Module name */
    filename = bufNStr (NULL, getFirstParam (&instr));  /* File name */

    if (getAllParam (&instr) != NULL)
        uError ("Additional .Ht parameters ignored, last valid field [%s].\n",
                 catdata);

    /*
     * Resolve the external references.
     * Call the user function with the data that has been collected.
     */

    linkType = nrResolveExternalReference
              (((nrCompiling & NROFF_MODE_COMPILE) ? LS_COMPILE: 0) | package,
               bufname, bufnum, &modulename, &filename);
    if (((nrCompiling & NROFF_MODE_COMPILE) != 0) &&
        (filename == NULL))
        uError ("Cannot resolve external reference to [%s%s]\n",
                 (bufname == NULL) ? "NoName" : bufname,
                 (bufnum == NULL) ? "" : bufnum);
    /* Note the callers never see .Hp - this only determines how the
     * cross reference is found. */
    nrCallback (Ht_func, (bufname, bufnum, catdata,
                          modulename, filename, linkType));
    if (nroffMode & CMD_MFI)
        nrCallback (br_func, ());

    bufFree (bufname);
    bufFree (bufnum);
    bufFree (catdata);
    bufFree (modulename);
    bufFree (filename);
    nroffMode &= ~CMD_PPO;
    if (nroffMode & CMD_MTP)
        TP_func ();
    nrCheck ();
}

static void
Ht_func (void)
{
    _Htp_func (0);
}

static void
Hp_func (void)
{
    _Htp_func (LS_PACKAGE);
}

static void
Hr_func (void)
{
    char    *bufname;
    char    *bufnum;
    char    *catdata;

    if ((bufname = bufNStr(NULL, getFirstParam (&instr))) != NULL) {
        if (isvalidstr (bufname) == 0)
            uWarn ("Invalid string in param #1. Found [%s].\n", bufname);
    }

    if ((bufnum = bufNStr (NULL, getFirstParam (&instr))) != NULL) {
        if (isalnumstr (bufnum) == 0)
            uWarn ("Alphanumeric expected in param #2. Found [%s].\n",
                   bufnum);
    } else
        uWarn ("Section number expected. Param #2.\n");

    catdata = bufNStr (NULL, getFirstParam (&instr));
    if (getAllParam (&instr) != NULL)
        uError ("Additional .Hr parameters ignored, last valid field [%s].\n",
                 catdata);

    nrCallback (Hr_func, (bufname, bufnum, catdata));
    if (nroffMode & CMD_MFI)
        nrCallback (br_func, ());

    bufFree (bufname);
    bufFree (bufnum);
    bufFree (catdata);
    nroffMode &= ~CMD_PPO;
    if (nroffMode & CMD_MTP)
        TP_func ();
    nrCheck ();
}

static void
Hg_func (void)
{
    char    *name;
    char    *abut;

    if ((name = bufNStr(NULL, getFirstParam (&instr))) == NULL)
        uWarn ("Parameter required for #1.\n", name);
    abut = bufNStr (NULL, getAllParam (&instr));
    nrCallback (Hg_func, (name, abut));
    if (nroffMode & CMD_MFI)
        nrCallback (br_func, ());

    bufFree (name);
    bufFree (abut);
    nroffMode &= ~CMD_PPO;
    if (nroffMode & CMD_MTP)
        TP_func ();
    nrCheck ();
}

static char *
fontExpand (int initial, int f1, int f2)
{
    char *p;
    char *q;
    int  argc = 0;
    int  current;

    current = initial;
    p = bufNStr (NULL, "\\&");
    while ((q = getFirstParam (&instr)) != NULL)
    {
        int  temp;

        temp = ((argc % 2) == 0) ? f1 : f2;
        if (current != temp)
        {
            current = temp;
            p = bufNStr (p, fontTable [temp]);
        }
        p = bufNStr (p, q);
        argc++;
    }

    /* Restore the correct font. */
    if (current != initial)
        p = bufNStr (p, fontTable [initial]);

    if (argc < 2)
        uWarn ("2 or more arguments expected.\n");
    return (p);
}

static void
handleFont (int f1, int f2)
{
    char *p;

    p = fontExpand (FT_R, f1, f2);
    nroffMode &= ~CMD_PPO;
    nrCheck ();
    _textline_func (p);
    free (p);
}

static void IR_func (void) { handleFont (FT_I, FT_R); }
static void RI_func (void) { handleFont (FT_R, FT_I); }
static void BR_func (void) { handleFont (FT_B, FT_R); }
static void RB_func (void) { handleFont (FT_R, FT_B); }
static void SB_func (void) { handleFont (FT_R, FT_B); }
static void CR_func (void) { handleFont (FT_C, FT_R); }
static void RC_func (void) { handleFont (FT_R, FT_C); }
static void IB_func (void) { handleFont (FT_I, FT_B); }
static void BI_func (void) { handleFont (FT_B, FT_I); }
static void CB_func (void) { handleFont (FT_C, FT_B); }
static void BC_func (void) { handleFont (FT_B, FT_C); }
static void IC_func (void) { handleFont (FT_I, FT_C); }
static void CI_func (void) { handleFont (FT_C, FT_I); }

static void
ig_func (void)
{
    if (ignoreInput == 1)
        uError ("Ignore is nested\n");
    else
        ignoreInput = 1;
}

static void
igl_func (void)
{
    ;                                   /* Do nothing !! */
}

static void
na_func (void)
{
    instr = stripBackSlash (0, instr, NULL);
    if (getAllParam (&instr) != NULL)
        uError (".na Unexpected arguments\n");
    if (nroffMode & CMD_MAD)
        uError ("Right justification already disabled\n");
    else
        nroffMode |= CMD_MAD;
    nrCallback (na_func, ());
}

static void
ft_func (void)
{
    int i;
    char *s;
    char *p;

    instr = stripBackSlash (0, instr, NULL);
    if ((p = getFirstParam (&instr)) == NULL)
    {
        nrCallback (ft_func, (0));
        /*        uError ("Font argument expected\n");*/
    }
    else if (getAllParam (&instr) != NULL)
        uError (".na Unexpected arguments\n");
    else
    {
        for (i = 1; (s = fontChar [i]) != NULL; i++)
        {
            if (strcmp (s, p) == 0)
            {
                nrCallback (ft_func, (i));
                return;
            }
        }
        for (i = 1; (s = fontNo [i]) != NULL; i++)
        {
            if (strcmp (s, p) == 0)
            {
                nrCallback (ft_func, (i));
                return;
            }
        }
        uError ("Unrecognised font requested [%s]\n", p);
    }
}

static void
ps_func (void)
{
    char *p;
    int  i;

    instr = stripBackSlash (0,instr, NULL);
    if ((p = getFirstParam (&instr)) == NULL)
        uError ("Point size argument expected\n");
    else
    {
        i = atoi (p);
        if ((i < PS_MIN) || (i > PS_MAX))
            uWarn ("Possible invalid point size [%d]\n", i);
        if (i == ps_size)
            uWarn ("Point size already set to %d\n");
        else
            nrCallback (ps_func, (i - ps_size));
        ps_size = i;
    }
    if (getAllParam (&instr) != NULL)
        uError (".ps <size> followed by illegal options\n");
}

static void
vs_func (void)
{
    char *p;
    int i;

    i = 0;
    instr = stripBackSlash (0,instr, NULL);
    if ((p = getFirstParam (&instr)) == NULL)
    {
        if (vs_pre_size == 0)
            uError ("No vertical spacing to restore\n");
        else
            i = vs_pre_size;
    }
    else
    {
        i = atoi (p);
        if ((i < VS_MIN) || (i > VS_MAX))
            uWarn ("Possible invalid vertical spacing [%d]\n", i);
    }

    if (getAllParam (&instr) != NULL)
        uError (".vs [<size>] followed by illegal option\n");

    if (i != 0)
    {
        if (i == vs_size)
            uWarn ("Vertical size already set to %d\n");
        else
            nrCallback (vs_func, (i - vs_size));
        vs_pre_size = vs_size;
        vs_size = i;
    }
}

static void
nf_func (void)
{
    instr = stripBackSlash (0,instr, NULL);
    if (getAllParam (&instr) != NULL)
        uError (".nf Unexpected arguments\n");
    if (nroffMode & CMD_MFI)
        uError ("Fill mode already disabled\n");
    else
        nroffMode |= CMD_MFI;
    nrCallback (nf_func, ());
}

static void
fi_func (void)
{
    instr = stripBackSlash (0,instr, NULL);
    if (getAllParam (&instr) != NULL)
        uWarn (".fi Unexpected arguments\n");
    if (nroffMode & CMD_MFI)
        nroffMode &= ~CMD_MFI;
    else
        uError ("Fill mode already enabled\n");
    nrCallback (fi_func, ());
}

static void
ad_func (void)
{
    instr = stripBackSlash (0,instr, NULL);
    if (getAllParam (&instr) != NULL)
        uError (".ad Unexpected arguments\n");
    if (nroffMode & CMD_MAD)
        nroffMode &= ~CMD_MAD;
    else
        uError ("Right justification already enabled\n");
    nrCallback (ad_func, ());
}

static void
_XIP_func (int package)
{
    char    *sname;
    char    *sid;
    char    *sdesc;
    char    *scomp;
    int     same;

    /*
     * Get the name of the function.
     */
    if ((sname = bufNStr (NULL, getFirstParam (&instr))) != NULL) {
        if (isvalidstr (sname) == 0)
            uError ("Invalid string in param #1. Found [%s].\n", sname);
    }
    else {
        uError ("Name parameter expected in \"%s\".\n", nroffIdentifier);
    }

    /*
     * Get the section number of the function. '-' means use the
     * default section number.
     */

    if ((sid = bufNStr (NULL, getFirstParam (&instr))) != NULL) {
        if (strcmp (sid, "-") == 0) {
            bufFree (sid);
            sid = NULL;
        }
        else if (isalnumstr (sid) == 0)
            uError ("Alphanumeric expected in param #2 of \"%s\". Found [%s].\n",
                     nroffIdentifier, sid);
    }

    /*
     * Get the section description. This is just a text string.
     */

    if ((sdesc = bufNStr (NULL, getFirstParam (&instr))) != NULL) {
        if (strcmp (sdesc, "-") == 0) {
            bufFree (sdesc);
            sdesc = NULL;
        }
    } else
        uWarn ("%s description missing.\n", nroffIdentifier);

    /*
     * Get the alturnative category label.
     */

    if ((scomp = bufNStr (NULL, getFirstParam (&instr))) != NULL) {
        if (strcmp (scomp, "-") == 0) {
            bufFree (scomp);
            scomp = NULL;
        }
    }

    /*
     * Check for unexpected's. We usually forget to encase the
     * description in quotes which gives a problem.
     * Bitch about the arguments.
     */

    if (getAllParam (&instr) != NULL)
        uError ("%s Unexpected arguments\n", nroffIdentifier);

    if (package & LS_PACKAGE)
        nrCallback (XP_func, (sname, sid, sdesc, scomp));
    else if (package & LS_JUMP)
        nrCallback (XJ_func, (sname, sid, sdesc, scomp));
    else
        nrCallback (XI_func, (sname, sid, sdesc, scomp));
    /*
     * Make sure that the first .XI label is the same as the TH section.
     * Make a comparison of the name space of the .XI and .TH statements and
     * determine if we are in an error state.
     */
    same = (((sname != NULL) && (THname != NULL) &&
             (strcmp (sname, THname) == 0)) &&
            ((sid == NULL) || ((THsection != NULL) &&
                               (strcmp (sid, THsection) == 0))));
    if ((nroffMode & CMD_MXI) ==  0)    /* First cross reference ?? */
    {
        nroffMode |= CMD_MXI;           /* Yes - tag as first reference */
#if 0
        if (!same)
            uError ("First \"%s\" entry must be the same as the .TH entry\n",
                    nroffIdentifier);
#endif
    }
    else if (same)
        uError ("The \"%s\" entry corresponding to the .TH entry must appear first.\n",
                nroffIdentifier);

    if ((nroffMode & (CMD_MIM|CMD_MID|CMD_MTH)) != (CMD_MTH|CMD_MIM|CMD_MID))
        uError ("%s Placement error\n", nroffIdentifier);

    /* Free of the buffers */
    bufFree (sname);
    bufFree (sid);
    bufFree (sdesc);
    bufFree (scomp);

}

static void
XI_func (void)
{
    _XIP_func (0);
}

static void
XP_func (void)
{
    _XIP_func (LS_PACKAGE);
}

static void
XJ_func (void)
{
    _XIP_func (LS_JUMP);
}

static void
br_func (void)
{
    if (getAllParam (&instr) != NULL)
        uError (".br Unexpected arguments\n");
    nrCallback (br_func, ());
    nrCheck ();
}

static void
bp_func (void)
{
    if (getAllParam (&instr) != NULL)
        uError (".bp Unexpected arguments\n");
    nrCallback (bp_func, ());
    nrCheck ();
}

static void
BS_func (void)
{
    char *p;                            /* Bullet item */
    int preSpace = 1;                   /* Pre-space line */

    bulletLineSpace = 0;                /* Reset bullet line spacing */
    if ((p = getFirstParam (&instr)) != NULL)
    {
        if (isdigit (*p))
             preSpace = (int)(*p - '0');
        else
            uError (".BS - Numeric 1st argument expected.\n");

        if ((p = getFirstParam (&instr)) != NULL)
        {
            if (isdigit (*p))
                bulletLineSpace = (int)(*p - '0');
            else
                uWarn (".BS - Numeric 2nd argument expected.\n");
            p = getFirstParam (&instr);  /* Bullet character */
        }
    }
    if (getAllParam (&instr) != NULL)
        uWarn (".BS Unexpected argument\n");
    nrCallback (BS_func, (preSpace, bulletLineSpace, p));

    if ((nroffMode & CMD_MBS) != 0)
        uError (".BS is nested - illegal order, close previous with .BE\n");
    if ((preSpace > 0) && (nroffMode & CMD_PST))
        uWarn ("Advise \".BS 0 ...\" invocation to retain correct spacing\n");
    else if ((preSpace == 0) && ((nroffMode & CMD_PPO) == 0))
        uWarn ("Advise \".BS 1 ...\" invocation to retain correct spacing\n");
    if (preSpace > 0)
        nroffCmdMode |= CMD_PPO;
    else
        nroffCmdMode &= ~CMD_PPO;
    nrCheck ();
    nroffMode |= CMD_MBS;               /* Set mode after the check */

}

static void
BU_func (void)
{
    char *p;

    p = getFirstParam (&instr);         /* Get bullet character */
    if (getAllParam (&instr) != NULL)
        uWarn (".BU Unexpected argument\n");
    nrCallback (BU_func, (bulletLineSpace, p));
    if ((nroffMode & CMD_MBS) == 0)
        uError ("Opening \".BS\" has not been encountered.\n");
    nrCheck ();
}

static void
BE_func (void)
{
    char *p;
    int  i;

    i = 1;
    if ((p = getFirstParam (&instr)) != NULL) {
        if (isdigit (*p))
            i = (int)(*p - '0');
        else
            uWarn (".BE - Numeric argument expected.\n");
        if (getAllParam (&instr) != NULL)
            uWarn (".BE Unexpected arguments\n");
    }
    bulletLineSpace = 0;                /* Reset bullet line spacing */
    if ((nroffMode & CMD_MBS) == 0)
        uError ("Opening \".BS\" has not been encountered.\n");
    nrCallback (BE_func, (i));
    if ((i != 0) & ((nroffMode & CMD_PPO) != 0))
        uWarn ("Advise \".BE 0\" for invocation to retain correct spacing.\n");
    if (i > 0)
        nroffCmdMode |= CMD_PPO;
    else
        nroffCmdMode &= ~CMD_PPO;
    nroffMode &= ~CMD_MBS;
    nrCheck ();
}

static void
CS_func (void)
{
    char *p;
    int  i;

    i = 1;
    if ((p = getFirstParam (&instr)) != NULL) {
        if (isdigit (*p))
            i = (int)(*p - '0');
        else
            uWarn (".CS - Numeric argument expected.\n");
        if (getAllParam (&instr) != NULL)
            uWarn (".CS Unexpected arguments\n");
    }
    if ((nroffMode & CMD_MCS) != 0)
        uError (".CS is nested - illegal order, close previous with .CE\n");
    if ((i > 0) && (nroffMode & CMD_PPO))
        uWarn ("Advise \".CS 0\" for invocation to retain correct spacing.\n");
    else if ((i == 0) & ((nroffMode & CMD_PPO) == 0))
        uWarn ("Advise \".CS\" for invocation to retain correct spacing.\n");
    if (i > 0)
        nroffCmdMode |= CMD_PPO;
    else
        nroffCmdMode &= ~CMD_PPO;
    nrCheck ();                         /* Check status */
    nrCallback (CS_func, (i));          /* Call user function */
    nroffMode |= CMD_MCS;               /* Set mode after the check */
}

static void
CE_func (void)
{
    char *p;
    int  i;

    i = 1;
    if ((p = getFirstParam (&instr)) != NULL) {
        if (isdigit (*p))
            i = (int)(*p - '0');
        else
            uWarn (".CE - Numeric argument expected.\n");
        if (getAllParam (&instr) != NULL)
            uWarn (".CE Unexpected arguments\n");
    }
    if ((nroffMode & CMD_MCS) == 0)
        uError (".CE command with no matching .CS command\n");
    if ((i != 0) & ((nroffMode & CMD_PPO) != 0))
        uWarn ("Advise \".CE 0\" for invocation to retain correct spacing.\n");
    if (i > 0)
        nroffCmdMode |= CMD_PPO;
    else
        nroffCmdMode &= ~CMD_PPO;
    nroffMode &= ~CMD_MCS;
    nrCallback (CE_func, (i));
    nrCheck ();                         /* Check status */
}

static void
dsInitialise (void)
{
    dsPush ("lq", "`");
    dsPush ("rq", "'");
    dsPush ("**", "*");
}

static void
ds_func (void)
{
    char *name;
    char *data;

    if ((name = bufNStr (NULL, getFirstParam (&instr))) != NULL)
    {
        data = bufNStr (NULL, getRestOfLineParam (&instr));
        if (data == NULL)
            uWarn ("Expected data in .ds %s\n", name);

/*        uWarn ("Defining %s=%s\n", name, data);*/
        dsPush (name, data);
        bufFree (name);
        bufFree (data);
    }
}

static void
rm_func (void)
{
    char *name;
    dsRecord *p;

    if ((name = bufNStr (NULL, getFirstParam (&instr))) == NULL)
        uWarn ("Marco name string expected\n");
    else if (getAllParam (&instr) != NULL)
        uWarn ("Unexpected parameter\n");
    if (name != NULL) {
        if ((p = dsFind (name)) != NULL)
            dsPop (p);
        else
            uWarn ("Cannot find macro [%s] to remove\n", name);
    }

    bufFree (name);
}

static void
ne_func (void)
{
    char *s;
    int  i;

    if ((s = getOnlyParam (&instr)) == NULL)
        uError ("Numeric argument expected with .ne\n");
    else if ((i = atoi (s)) < 1)
        uWarn ("Illegal value specified with .ne\n");
    else
        nrCallback (ne_func, (i));
/*    nrCheck ();*/
}

static void
sp_func (void)
{
    char *s;
    int  i;

    if ((s = getOnlyParam (&instr)) == NULL)
        i = 1;
    else if ((i = atoi (s)) < 1)
    {
        uWarn ("Illegal value specified with .sp\n");
        i = 1;
    }
    nrCallback (sp_func, (i));
    nrCheck ();
}

static void
hy_func (void)
{
    /* Ignore the line */
}

static void
ll_func (void)
{
    char *s;
    int  i;

    if ((s = getOnlyParam (&instr)) == NULL)
        uError ("Numeric argument expected with .ll\n");
    else if ((i = atoi (s)) < 1)
        uWarn ("Illegal value specified with .ll\n");
    else
        nrCallback (ll_func, (i));
    nrCheck ();
}

static void
pl_func (void)
{
    char *s;
    int  i;

    if ((s = getOnlyParam (&instr)) == NULL)
        uError ("Numeric argument expected with .pl\n");
    else if ((i = atoi (s)) < 1)
        uWarn ("Illegal value specified with .pl\n");
    else
        nrCallback (pl_func, (i));
    nrCheck ();
}

static void
RS_func (void)
{
    long i;

    if (getAllParam (&instr) != NULL)
        uWarn (".RS Unexpected arguments\n");
    nrCallback (RS_func, ());

    i = CMD_MRS_GET(nroffMode)+1;
    nroffMode =  ((nroffMode & ~CMD_MRS) | CMD_MRS_SET (nroffMode, i));
    nrCheck ();
}

static void
RE_func (void)
{
    long i;

    if (getAllParam (&instr) != NULL)
        uWarn (".RE Unexpected arguments\n");
    if ((nroffMode & CMD_MRS) == 0)
        uError (".RE Unexpected. No matching .RS found\n");
    nrCallback (RE_func, ());
    i = CMD_MRS_GET(nroffMode)-1;
    if (i < 0)
    {
        uError (".RE encountered with no matching .RS statement\n");
        i = 0;
    }
    nroffMode = (nroffMode & ~CMD_MRS) | CMD_MRS_SET (nroffMode, i);
    nrCheck ();
}

static void
TP_func (void)
{
    if ((nroffMode & CMD_MTP) == 0)
    {
        nrCallback (ne_func, (3));
        /* Ignore the first argument */
        getFirstParam (&instr);
        if (getAllParam (&instr) != NULL)
            uWarn (".TP Unexpected arguments\n");
        nrCallback (TP_func, (1));
        nrCheck ();
        nroffMode |= CMD_MTP;
    }
    else
    {
        nroffMode &= ~CMD_MTP;
        nrCallback (TP_func, (0));
    }
}

static void
IP_func (void)
{
    char *s;

    nrCallback (ne_func, (3));
    s = stripBackSlash (0, getOnlyParam (&instr), NULL);
    nrCallback (IP_func, (s));
    if (instr != NULL)
        uWarn (".IP Arguments should be quoted with \"...\" - Truncated.\n");
    nrCheck ();
}

static void
LP_func (void)
{
    if (getAllParam (&instr) != NULL)
        uWarn (".LP Unexpected arguments\n");
    nrCallback (LP_func, ());
    nrCheck ();
}

static void
BP_func (void)
{
    char *s;

    nrCallback (ne_func, (3));
    s = stripBackSlash (0, getOnlyParam (&instr), NULL);
    nrCallback (BP_func, (s));
    if (instr != NULL)
        uWarn (".BP Arguments should be quoted with \"...\" - Truncated.\n");
    nrCheck ();
}

static void
fontFunction (int font)
{
    char *s;
    char *p = NULL;

    if ((nroffMode & (CMD_MRF|CMD_MBF|CMD_MIF|CMD_MSF|CMD_MCF)) != 0)
    {
        if (nroffMode & CMD_MRF)
            font = FT_R;
        else if (nroffMode & CMD_MBF)
            font = FT_B;
        else if (nroffMode & CMD_MIF)
            font = FT_I;
        else if (nroffMode & CMD_MSF)
            font = FT_S;
        else if (nroffMode & CMD_MCF)
            font = FT_C;
        else
        {
            uError ("Bad font definition\n");
            font = 0;
        }

        nroffMode &= ~(CMD_MRF|CMD_MBF|CMD_MIF|CMD_MSF|CMD_MCF);
        s = instr;
        instr = NULL;
    }
    else
        s = getOnlyParam(&instr);

    if (s == NULL)
    {
        /*        nrCallback (ft_func, (font));*/
        if (font == FT_R)
            nroffMode |= CMD_MRF;
        else if (font == FT_B)
            nroffMode |= CMD_MBF;
        else if (font == FT_I)
            nroffMode |= CMD_MIF;
        else if (font == FT_S)
            nroffMode |= CMD_MSF;
        else if (font == FT_C)
            nroffMode |= CMD_MCF;
        else
        {
            uError ("Bad font definition\n");
            nroffMode &= ~(CMD_MRF|CMD_MBF|CMD_MIF|CMD_MSF|CMD_MCF);
        }
    }
    else
    {
        if (font != FT_S)               /* Frig for small !! */
            p = bufStr (NULL, fontTable [font]);
        p = bufStr (p, s);
        if (font != FT_S)               /* Frig for small !! */
            p = bufStr (p, fontTable [FT_P]);
        nroffMode &= ~CMD_PPO;
        if (instr != NULL)
            uWarn ("Arguments should be quoted with \"...\" - Truncated.\n");
        _textline_func (p);
        free (p);
        nrCheck ();
    }
}

static void B_func (void) { fontFunction (FT_B); }
static void I_func (void) { fontFunction (FT_I); }
static void R_func (void) { fontFunction (FT_R); }
static void C_func (void) { fontFunction (FT_C); }
static void SM_func (void) { fontFunction (FT_S); }

static void
comment_func (void)
{
    char *s;

    if ((instr = leftTrim (instr)) == NULL) /* Do not allow NULL string through */
        return;                             /* Quit early. */

    /* Ignore errors and warnings if tagged. */
    if (strncmp (instr, "IGNORE", 6) == 0)
    {
        s = getFirstParam (&instr);
        if (strcmp (s, "IGNORE ") == 0)
        {
            uWarnIgnore (nrWarnIgnore = 2);
            uErrorIgnore (nrErrorIgnore = 2);
        }
        if (strcmp (s, "IGNORE-WARNING") == 0)
            uWarnIgnore (nrWarnIgnore = 2);
        else if (strcmp (s, "IGNORE-ERROR") == 0)
            uErrorIgnore (nrErrorIgnore = 2);
        else
            uWarn ("Invalid IGNORE string\n");
    }
    else if ((strncmp (instr, "ERROR", 5) == 0) ||
             (strncmp (instr, "WARNING", 7) == 0))
    {
        char *f;

        s = getFirstParam (&instr);
        f = instr;
        if (strcmp (s, "ERROR") == 0)
            uError ("In-line [%s]\n", (instr == NULL) ? "UNDEFINED" : instr);
        else if (strcmp (s, "WARNING") == 0)
            uWarn ("In-line [%s]\n", (instr == NULL) ? "UNDEFINED" : instr);
    }
    else if (instr[0] == '!')
        nrCallback (comment_func, (&instr[1], 1));
    else
        nrCallback (comment_func, (instr, 0));
}

static void
Ss_func (void)
{
    uError ("Search construct .Ss found. Pre-process file using nrsearch\n");
}

static void
Se_func (void)
{
    uError ("Search construct .Se found. Pre-process file using nrsearch\n");
}

static void
Ls_func (void)
{
    char      *mname;
    char      *fname;

    if ((mname = getFirstParam (&instr)) == NULL)
        uWarn ("Expected module name in .Ls - Ignored\n");
    if ((fname = getFirstParam (&instr)) != NULL)
    {
        if (instr != NULL)
            uWarn (".Ls Multiple arguments unexpected.\n");
    }

    /*
     * Push the new library module. Note that the depth is 1 deep only.
     */
    pushLibraryModule = libraryModule;
    libraryModule = bufNStr (NULL, mname);
    if (fname != NULL)
        nrLibModuleSetAlias (libraryModule, fname);
    nrCallback (Ls_func, (mname, fname));
}

static void
Le_func (void)
{
    nrCallback (Le_func, (libraryModule));
    bufFree (libraryModule);
    libraryModule = pushLibraryModule;
    pushLibraryModule = NULL;
}

static void
_Lip_func (int package)
{
    char *xrefName;
    char *fileName;
    char *name = NULL;
    char *section = NULL;
    char *description = NULL;
    char *category = NULL;

    if ((xrefName = getFirstParam (&instr)) == NULL)
        uError ("Cross reference name expected in \"%s\".\n",
                nroffIdentifier);
    else if ((fileName = getFirstParam (&instr)) == NULL)
        uError ("File reference expected in \"%s\"\n",
                nroffIdentifier);
#if 0
    else if (getAllParam (&instr) != NULL)
        uError ("\"%s\" Unexpected argumentes - ignoring entry\n",
                nroffIdentifier);
#endif
    else
    {
        if (libFindAddReference (libraryModule, xrefName, fileName,
                                 NULL, NULL, NULL, package|LS_EXTERN) == 0)
        {
            uWarn ("Duplicate reference [%s in %s] - JG need to check consistency\n",
                   xrefName, fileName);
        }
        /*
         * Handle a library with the extended name syntax
         */

        if ((name = getFirstParam (&instr)) != NULL)
        {
            section = getFirstParam (&instr);
            description = getFirstParam (&instr);
            category = getFirstParam (&instr);

            if (getAllParam (&instr) != NULL)
                uError ("\"%s\" Unexpected argumentes - ignoring entry\n",
                        nroffIdentifier);

            if ((name != NULL) && (section != NULL))
            {

                if (package & LS_PACKAGE)
                    nrCallback (Lp_func, (name, section, description, category));
                else if (package & LS_JUMP)
                    nrCallback (Lj_func, (name, section, description, category));
                else
                    nrCallback (Li_func, (name, section, description, category));
            }
        }
    }
}

static void
Lp_func (void)
{
    _Lip_func (LS_PACKAGE);
}

static void
Li_func (void)
{
    _Lip_func (0);
}

static void
Lj_func (void)
{
    _Lip_func (LS_JUMP);
}

static void
Me_func (void)
{
    char *text;

    if ((text = getOnlyParam (&instr)) == NULL)
        uWarn ("Expected single argument for .Me\n");
    if (instr != NULL)
        uWarn (".Me Multiple arguments unexpected.\n");
    nrCallback (Me_func, (text));
}

static void
so_func (void)
{
    char    *fname;

    if ((fname = getOnlyParam (&instr)) == NULL)
        uWarn ("Expected Filename in .so - Ignored\n");
    if (instr != NULL)
        uWarn (".so Multiple arguments unexpected.\n");
    nrFilePush (fname);
    nrCallback (so_func, (fname));
}

static void
if_func (void)
{
    int cond = 1;
    int status = 2;
    char c;
    char *isGroffStr = "\\n(.g";

    instr = leftTrim (instr);
    if (instr == NULL)
        return;
    /*
     * Get true / false condition
     */
    if ((c = *instr) == '!')            /* Not ?? */
    {
        cond = 0;                       /* Yes - reverse sense */
        c = *++instr;                   /* Skip not ! */
    }

    if (c == 'n')
    {
        instr++;
        status = 1;
    }
    else if (c == 't')
    {
        instr++;
        status = 0;
    }
    else if (strncmp (instr, isGroffStr, strlen (isGroffStr) == 0))
    {
        instr = &instr [strlen (isGroffStr)];
        status = 0;
    }
    else if (strncmp (instr, "\"\\n(.I\"", 7) == 0)
    {
        char converter [1024];

        sprintf (converter, "\"\\n(.I\"%s\"", nroffId);
        if (strncmp (instr, converter, strlen (converter)) == 0)
        {
            instr = &instr [strlen (converter)];
            status = 1;
        }
        else
            status = 0;
    }

    if (status == 2)
        uWarn ("Cannot understand .if condition [%s]\n", instr);
    else if (status == cond)
    {
        instr = leftTrim (instr);
        ieel_cond = 1;
/*        uWarn (".if is true\n");*/
        return;
    }
/*  uWarn (".if is FALSE\n");*/
    ieel_cond = 0;
    instr = NULL;
}

static void
ie_func (void)
{
    if_func ();
}

static void
el_func (void)
{
    if (ieel_cond == 1)                 /* Last ie true ?? */
    {
/*        uWarn ("Else is false\n");*/
        ieel_cond = 0;                  /* Yes - make this one false */
        instr = NULL;
    }
    else
    {
/*        uWarn ("Else is true\n");*/
        ieel_cond = 1;                  /* Make last true */
        instr = leftTrim (instr);       /* Trim back to new command */
    }
}

static void
_textline_func (char *line)
{
    /*
     * If we are in a CS block then treat as literal spaces.
     * If we are in no fill mode then treat as literal spaces.
     */
    if (nroffMode & (CMD_MCS|CMD_MFI))
        instr = stripBackSlash (USPACE_CHAR, line, NULL);
    else
        instr = stripBackSlash (0, line, NULL);

    if (nroffMode & (CMD_MRF|CMD_MBF|CMD_MIF|CMD_MSF|CMD_MCF))
        fontFunction (0);
    else
        nrCallback (textline_func, (line));
    if (nroffMode & CMD_MTP)
        TP_func ();
}

static void
textline_func (void)
{
    char c;

    nroffCmd  = 0;
    if (instr == NULL)
        nroffCmdMode = CMD_PPO;
    else
    {
        c = *instr;
        if ((c == '.') || (c == '\'') || /* (c == '<') || */ (c == ','))
            uWarn ("Advise a \"\\&\" at start of line to protect \"%c\"\n",c);
        if (((nroffMode & CMD_MCS) == 0) && (c == ' '))
            uWarn ("Space detected at start of line, advise removal\n");
        nroffCmdMode = 0;
    }
    _textline_func (instr);
    nrCheck ();
}

/*
 * File handling S/W
 */

nrFILE *nrLocalPush (char *filename)
{
    nrFILE  *p;

    if (filename == NULL)
        return (NULL);

    /* Set up push block */

    p = (nrFILE *) malloc (sizeof (nrFILE));
    strcpy (p->fileName, filename);
    p->lineNo = /*nrfp->lineNo*/ 0;

    uFileSet (&(p->lineNo), p->fileName);
    uInteractive ("Processing %s", p->fileName);

    p->superman = 0;
    p->prev = nrfp;
    p->fp = nrfp->fp;
    nrfp = p;
    return (p);
}

nrFILE *nrLocalPop (void)
{
    nrFILE  *p;

    if ((p = nrfp) != NULL) {
        nrfp = p->prev;
        nrfp->lineNo += p->lineNo;
        nrfp->fp = p->fp;
        free (p);
    }
    if (nrfp != NULL)
    {
        uFileSet (&(nrfp->lineNo), nrfp->fileName);
        uInteractive ("Processing %s", nrfp->fileName);
    }
    else
        uFileSet (NULL, NULL);
    return (nrfp);
}

static void
endFile_func (void)
{
    char *fname;

    if ((fname = getFirstParam (&instr)) == NULL)
        uWarn ("Expected Filename in .SUPERMANEND - Ignored\n");
    else {
        uDebug (1, ("SupermanEnd [%s]\n", fname));
        if ((supermaninc == 2) && (includeMode == 0)) {
            if ((nroffMode & CMD_MID) == 0)
                uError (".Id Statement Required.\n");
            if ((nroffMode & CMD_MIM) == 0)
                uError (".Im Statement Required.\n");
            if ((nroffMode & CMD_MTH) == 0)
                uError (".TH Statement Required.\n");
            if ((nroffMode & CMD_MFH) == 0)
                uError (".FH Statement Required.\n");
            if ((nroffMode & CMD_MXI) == 0)
                uError (".XI Statement Required.\n");
        }
        nrCallback(endFile_func, (fname, includeMode, 0));
        dsPop (NULL);
        nrLocalPop ();
        supermaninc = 0;
    }
    includeMode = 0;
}

static void
includeFile_func (void)
{
    char *fname;

    supermanfile = 1;
    if ((fname = getFirstParam (&instr)) == NULL)
        uWarn ("Expected Filename in .SUPERMANINC - Ignored\n");
    else {
        uDebug (1, ("SupermanInc [%s]\n", fname));
        nroffMode = (CMD_MID|CMD_MIM|CMD_MTH|CMD_MXI);
        nrLocalPush (fname);
        supermaninc = 1;
        includeMode = 0;
        nrCallback (includeFile_func, (fname, &includeMode));
    }
}

static void
startFile_func (void)
{
    char *fname;

    supermanfile = 1;
    if ((fname = getFirstParam (&instr)) == NULL)
        uWarn ("Expected Filename in .SUPERMANFILE - Ignored\n");
    else {
        uDebug (1, ("SupermanFile [%s]\n", fname));
        nroffMode = 0;
        nrLocalPush (fname);
        supermaninc = 2;
        nrCallback (startFile_func, (fname, 0));
    }
}

static int
readLine (char **cbuf)
{
    static char    buffer [5000];
    int     c;
    int     len;
    int     illegal = 0;                /* Illegal character */

    /*
     * Read data into the static buffer.
     */

    if (nrfp == NULL)
        return (-1);

    *cbuf = buffer;
    buffer [0] = '\0';
    len = 0;
    for (;;)
    {
        c = fgetc (nrfp->fp);
        if ((c == EOF)
#ifndef _UNIX
            || (c == 0x1A)
#endif
            ) {
            if ((nrFilePop() == NULL) && (len == 0)) {
                buffer [0] = '\0';
                return (-1);
            }
            else if (len != 0) {
                buffer [len] = '\0';
                return (len);
            }
            else
                continue;
        }

        if (c == '\n') {
            nrfp->lineNo++;                     /* On the next line */
            /*
             * Detect a backslash at the end of the line and
             * concatinate to the current line.
             *
             * Only treat as continuation if the line ends with a backslash
             * and it is not an escaped backslash
             */
            if (illegal != 0) {
                uWarn ("Illegal character %#04x\n", illegal);
                illegal = 0;
            }

            if ((len > 0) && (buffer [len-1] == '\\') &&
                ((len < 2) || (buffer [len - 2] != '\\')))
            {
                len--;
                continue;
            }
            buffer [len] = '\0';
            return (len);
        }
        else if (c == '\r')
            continue;
#ifdef _UNIX
        else if (c == 0x1A)
            continue;
#endif
        if ((c < ' ') || (c >= 0x7f))
            if (c != '\t')
                illegal = c;
        buffer [len++] = c;
    }   /* End of 'while' */
}   /* End of 'readLine' */

void nrInstallFunctionTable (nrFUNCTION *table)
{
    nrftab = table;
    dsInitialise ();
}

int
nroff (nrFILE *fp, int mode)
{
    static  int setup = 0;
    KeyType *kp;
    char    *inLine;
    int     endSuperman;
    char    *endName;

    nrCompiling = mode;                 /* Set the compilation mode */
    supermanfile = 0;
    exitFile = 0;
    if ((nrfp = fp) == NULL)
        return (0);

    if (setup == 0)
    {
        for (kp = keyTab; kp->name != NULL; kp++)
            kp->size = strlen (kp->name);
        setup = 1;
    }

    nroffMode = 0;
    nrCallback (startFile_func, (nrfp->fileName, nrfp->superman));
    endName = bufNStr (NULL, nrfp->fileName);
    endSuperman = nrfp->superman;
    uWarnIgnore (nrWarnIgnore = 0);
    uErrorIgnore (nrErrorIgnore = 0);

    while ((exitFile == 0) && (readLine (&inLine) != -1))
    {
        inLine = rightTrim (inLine);

        /* If we are ignoring input then skip it until the end marker */
        if (ignoreInput != 0)
        {
            if ((inLine != NULL) && (strcmp (inLine, "..") == 0))
                ignoreInput = 0;
            continue;
        }

        /* Process the line */
        if ((inLine != NULL) && (inLine [0] == '.'))
        {
do_again:
            uDebug (1, ("Command Line [%s]\n", inLine));
            for (kp = keyTab; kp->name != NULL; kp++) {
                if (strncmp (kp->name, inLine, kp->size) == 0) {

                    nroffIdentifier = kp->name;
                    /* Got a match - process it */
                    instr = &inLine [kp->size];
                    if ((kp->index == KW_if) ||
                        (kp->index == KW_ie) ||
                        (kp->index == KW_el))
                    {
                        kp->func ();
                        if (instr == NULL)
                            goto do_quit;
                        else
                        {
                            inLine = instr;
                            goto do_again;
                        }
                    }

                    if ((kp->type & CMD_INV) == 0)
                    {
                        nroffLastCmd = nroffCmd;
                        nroffLastCmdMode = nroffCmdMode;
                    }
                    nroffCmd = kp->index;
                    nroffCmdMode = kp->type;
                    if (includeMode == 0)
                        kp->func ();
                    else if (kp->index == KW_SUPERMANEND)
                        kp->func ();
                    break;
                }
            }
            if (kp->name == NULL)
                uError ("Unrecognised nroff tag [%s]\n", inLine);

        }
        else if (nrfp->superman == 0)
        {
            if ((inLine != NULL) && (inLine [0] == '\0'))
                inLine = NULL;
            if (includeMode == 0)
            {
                instr = inLine;
                textline_func ();
            }
        }
do_quit:
        uWarnIgnore (--nrWarnIgnore);
        uErrorIgnore (--nrErrorIgnore);
    }

    if (ignoreInput != 0)
        uError ("Ignore is still enabled\n");

    /* Flush any files */
    while (nrFilePop() != NULL)
        ;
    nrCallback (endFile_func, (endName, 0, endSuperman));
    dsPop (NULL);
    bufFree (endName);
    return (1);
}
