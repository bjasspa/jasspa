/* -*- C -*- ****************************************************************
 *
 *  			    Copyright 2002 Jon Green.
 *			      All Rights Reserved
 *
 *
 *  System        : 
 *  Module        : 
 *  Object Name   : $RCSfile: nr2tex.c,v $
 *  Revision      : $Revision: 1.1 $
 *  Date          : $Date: 2002-03-10 14:24:42 $
 *  Author        : $Author: jon $
 *  Created By    : Jon Green
 *  Created       : Thu Mar 7 20:45:45 2002
 *  Last Modified : <020310.1401>
 *
 *  Description	
 *
 *  Notes
 *
 *  History
 *	
 *  $Log: not supported by cvs2svn $
 *
 ****************************************************************************
 *
 *  Copyright (c) 2002 Jon Green.
 * 
 *  All Rights Reserved.
 * 
 * This  document  may  not, in  whole  or in  part, be  copied,  photocopied,
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
#include <sys/types.h>
#include <time.h>

#if ((defined _HPUX) || (defined _LINUX))
#include <unistd.h>
#else
#include <getopt.h>
#endif
#include <utils.h>

#include "nroff.h"

#define MODULE_VERSION  "1.0.0"
#define MODULE_NAME     "nr2tex"

#define FULL_INDENT 1                   /* Full indent distance. */
#define HALF_INDENT 1                   /* Half indent distance. */

#define NORMAL_MODE 0x0000
#define BOLD_MODE   0x0100
#define ITALIC_MODE 0x0200
#define MODE_CHAR   0x007f
#define MODE_MASK   0xff00
#define JUST_MODE   0x0400
#define PARA_MODE   0x0800
#define SPACE_MODE  0x1000
#define TITLE_MODE  0x2000
#define PAGE_MODE   0x4000
#define CODE_MODE   0x8000
#define MONO_MODE   0x10000
#define FONT_MODE   (MONO_MODE|BOLD_MODE|ITALIC_MODE)

#define PARA_CLOSE  0x01
#define PARA_NORMAL 0x02
#define PARA_TERM   0x04
#define PARA_RESET  (PARA_CLOSE|PARA_TERM)
#define PARA_FORCE  0x10
#define PARA_BREAK  0x20

static int  para_clean;
static int  para_mode;
static int  prevMode = 0;
static int  para_indent;
static int  indent;
static int  sub_indent;
static int  mode;
static int  quietMode = 0;
static int  ps_size = 0;
static char *im_buf = NULL;
static char *progname = MODULE_NAME;
static char *moduleName = NULL;         /* Name of the module */
static char *sectionName = NULL;
static char *sectionId = NULL;
static char *sectionComponent = NULL;
static char *sectionDate = NULL;        /* Date on which last modified */
static char *copyrightName = NULL;
static int  sectionLevel = 0;           /* Position of section */
static char *fileLaTeXName = NULL;
static char *localName = NULL;
static char *localNum = NULL;
static char *logoName = "logo.gif";     /* Default logo name */
static FILE *fo;
static FILE *fpr = NULL;
static int  compiling = 0;              /* Compile to resolve all refrences */
static int  year;                       /* The current year */
static int  month;                      /* The current month */
static int  day;                        /* The current day */
static int  bodgeMode=0;                /* Emacs bodge mode */

/* Constant string definitions */

int
hyperChr (int c)
{
    if ((c >= 'A') && (c <= 'Z'))
        return (tolower(c));
    if (((c >= '0') && (c <= '9')) ||
        ((c >= 'a') && (c <= 'z')) ||
        /*(c == '.') || */ (c == '_'))
        return (c);
    return ('\0');
}

static void
makeLaTeXName (char *s)
{
    int     j;
    char    *cb;

    cb = bufNStr (NULL, s);
    for (j = 0; ((cb[j] != '\0') && (cb[j] != '.')); j++)
        if (isupper (cb[j]))
            cb[j] = tolower (cb[j]);
    cb[j] = '\0';
    cb = bufStr (cb, "&LaTeX&");

    bufFree (fileLaTeXName);
    fileLaTeXName = cb;
}

static char *
makeLaTeXReference (int level, char *module, char *file)
{
    static char filename [1024];
    char        *home;

    filename[0] = '\0';
    home = nrHomeGet ();

    if (module == NULL)
    {
        if (level == 0)
        {
            if (strcmp (file, home) == 0)
                return (file);
            strcat (filename, home);
            strcat (filename, "/");
        }
        else
        {
            if (strcmp (file, home) == 0)
                strcat (filename, "../");
            else
                return (file);
        }
        strcat (filename, file);
        return (filename);
    }

    if (level == 1)
        strcat (filename, "../");

    if ((strcmp (file, home) == 0) && (strcmp (module, home) == 0))
        strcat (filename, file);
    else
    {
        strcat (filename, module);
        strcat (filename, "/");
        strcat (filename, file);
    }
    return (filename);
}

static void
latexEof (void)
{
#if 0
    fprintf (fpr, "%c", 0x1a);          /* Add Ctrl-Z */
#endif
    ;                                   /* Do nothing !! */
}

static void
latexEol (void)
{
    fprintf (fpr, "\n");
}

static void
latexStr (char *format , ...)
{
    va_list ap;
    va_start(ap, format);
    vfprintf(fpr, format, ap);
    va_end(ap);
}

static void
setParaMode (int mode)
{
    /* Turn off any highlighting modes if required. */
    if (((mode  & BOLD_MODE) == 0) && ((para_mode & BOLD_MODE) != 0)) {
        para_mode &= ~BOLD_MODE;
        latexStr ("}");
    }
    if (((mode  & ITALIC_MODE) == 0) && ((para_mode & ITALIC_MODE) != 0)) {
        para_mode &= ~ITALIC_MODE;
        latexStr ("}");
    }
    if (((mode  & MONO_MODE) == 0) && ((para_mode & MONO_MODE) != 0)) {
        para_mode &= ~MONO_MODE;
        latexStr ("}");
    }

    /* Turn on any highlighting modes if required. */
    if (((mode  & BOLD_MODE) != 0) && ((para_mode & BOLD_MODE) == 0)) {
        para_mode |= BOLD_MODE;
        latexStr ("\\textbf{");
    }
    if (((mode  & ITALIC_MODE) != 0) && ((para_mode & ITALIC_MODE) == 0)) {
        para_mode |= ITALIC_MODE;
        latexStr ("\\textit{");
    }
    if (((mode  & MONO_MODE) != 0) && ((para_mode & MONO_MODE) == 0)) {
        para_mode |= MONO_MODE;
        latexStr ("\\texttt{");
    }
}

static void
setIndentMode (void)
{
    int cur_indent;

    /* Compare the current paragraph indent level with what the application
       has requested - add and remove paragraph indents as appropriate.  Note
       when they are equal - we drop through both whiles and do nothing.  */

    cur_indent = indent+sub_indent;

    /* Add a new indent level */
    while (para_indent < cur_indent) {
        latexEol();
        latexEol();
        latexStr ("\\RS ");
        latexEol();
        latexEol();
        para_indent++;
    }

    /* Remove indent level */
    while (para_indent > cur_indent) {
        latexEol();
        latexEol();
        latexStr ("\\RE ");
        latexEol();
        latexEol();
        para_indent--;
    }
}

static void
insertPara (int pmode)
{
    /* End the last paragraph. Close any highlighting if necessary */
    if ((pmode & (PARA_TERM)) != 0)
    {
        setParaMode (0);                /* Remove highlight */
        setIndentMode ();
        para_mode = PARA_TERM;          /* Set into terminate mode */
    }

    if ((pmode & (PARA_CLOSE)) != 0)
    {
        if (para_clean != 0) {
            setParaMode (0);            /* Turn off any enabled modes */
            para_mode = 0;              /* Reset the mode */
            para_clean = 0;
            latexEol ();                 /* Terminate the line */
        }
    }

    /* Start a new paragraph if requested. Use the formatting required. */

    if (((pmode & PARA_NORMAL) != 0) &&
        ((pmode & (PARA_FORCE|CODE_MODE)) == 0))
    {
        setIndentMode ();
#if 0
        latexStr ("\\par ");
#else
        latexEol();
        latexEol();
#endif        
        para_mode = PARA_NORMAL;
        para_clean = 1;
    }

}   /* End of 'insertPara' */

static void
latexFormatStr (int lmode, char *s, int *newMode)
{
    int space = 0;
    char c;

    if ((lmode & TITLE_MODE) == 0)
    {
        if (newMode != NULL)
            *newMode = lmode;
        if ((lmode & (PARA_FORCE|CODE_MODE)) == 0) {
            if (para_mode & PARA_TERM)
                insertPara (PARA_CLOSE);
            if ((para_mode & PARA_NORMAL) == 0)
                insertPara (PARA_NORMAL);
        }

        setParaMode (lmode);
    }

    if ((para_mode & PARA_BREAK) != 0)
    {
        latexEol ();
        latexStr ("\\newline ");
        para_mode &= ~PARA_BREAK;
    }
    setIndentMode ();


    while ((c = *s++) != '\0')
    {
        switch (c) {
        case BOLD_CHAR:
            prevMode = para_mode & FONT_MODE;
            if ((para_mode & BOLD_MODE) != 0)
            {
                uWarn ("Already have bold mode enabled.\n");
                setParaMode (0);        /* Make safe !! */
            }

            /* Enable mode required. */
            setParaMode (BOLD_MODE);
            if (newMode != NULL)
                *newMode = (*newMode & ~FONT_MODE)|BOLD_MODE;
            break;
        case MONO_CHAR:
            prevMode = para_mode & FONT_MODE;
            if ((para_mode & MONO_MODE) != 0)
            {
                uWarn ("Already have mono mode enabled.\n");
                setParaMode (0);        /* Make safe !! */
            }

            /* Enable mode required. */
            setParaMode (MONO_MODE);
            if (newMode != NULL)
                *newMode = (*newMode & ~FONT_MODE)|MONO_MODE;
            break;
        case ITALIC_CHAR:
            prevMode = para_mode & FONT_MODE;
            if ((para_mode & ITALIC_MODE) != 0)
            {
                uWarn ("Already have italic mode enabled.\n");
                setParaMode (0);        /* Make safe !! */
            }

            /* Enable mode required. */
            setParaMode (ITALIC_MODE);
            if (newMode != NULL)
                *newMode = (*newMode & ~FONT_MODE)|ITALIC_MODE;
            break;
        case ROMAN_CHAR:
            prevMode = para_mode & FONT_MODE;
            if ((para_mode & FONT_MODE) == 0)
            {
                uWarn ("Roman requested and already in Roman.\n");
            }
            setParaMode (0);
            if (newMode != NULL)
                *newMode &= ~FONT_MODE;
            break;
        case PREV_CHAR:
            if ((para_mode & FONT_MODE) == 0)
                uError ("Current font is \\fP and current font is Roman\n");
            else if (prevMode == (para_mode & FONT_MODE))
                uWarn ("Previous and current font are the same\n");
            else
                setParaMode (prevMode);
            if (newMode != NULL)
                *newMode = (*newMode & ~FONT_MODE) | prevMode;
            break;
        case '<':
            latexStr ("$\\langle$");
            break;
        case '>':
            latexStr ("$\\rangle$");
            break;
        case '{':
            latexStr ("\\{");
            break;
        case '}':
            latexStr ("\\}");
            break;
        case '$':
            latexStr ("\\$");
            break;
        case '^':
            latexStr ("\\^");
            break;
        case '_':
            latexStr ("\\_");
            break;
        case '%':
            latexStr ("\\%");
            break;
        case '#':
            latexStr ("\\#");
            break;
        case '&':
            latexStr ("\\&");
            break;
        case '\\':
            latexStr ("$\\backslash$");
            break;
        case ' ':
            if (((lmode & SPACE_MODE) != 0) && (space != 0))
                break;
            latexStr ("%c", c);
            space = 1;
            break;
        case COPYRIGHT_CHAR:
            latexStr ("$\\copyright$");
            break;
        case DEGREE_CHAR:
            latexStr ("&#176;");
            break;
        case ZSPACE_CHAR:
        case MSPACE_CHAR:
            break;
        case BULLET_CHAR:
            latexStr ("$\\bullet$");
            break;
        case ESC_CHAR:
            latexStr ("$\\backslash$");
            break;
        case USPACE_CHAR:
            latexStr ("~");
            break;
        default:
            latexStr ("%c", c);
            break;
        }
        if (c != ' ')
            space = 0;
    }
}

static void
insert (char *s, int mode)
{
    int     length;
    int     i;
    char    buffer [1024];

    if (s == NULL)
        return;
#if 0
    if ((mode & (PARA_FORCE|CODE_MODE)) == 0) {
        if (para_mode == PARA_TERM)
            insertPara (PARA_CLOSE);
        if (para_mode == 0)
            insertPara (PARA_NORMAL);
    }
#if 0
    setParaMode (mode);                 /* Enable/Disable necessary modes */
#endif
#endif
    while (*s) {
        s = rightTrim(leftTrim (s));
        if (s == NULL)
            break;
        length = wordLen (s);
        for (i = 0; i < length; i++)
            buffer [i] = s[i];
        buffer [i] = '\0';
        s = &s [length];
        latexFormatStr (mode, buffer, &mode);
        if ((mode & SPACE_MODE) && (*s == '\0'))
            break;
        latexFormatStr (mode, " ", &mode);
    }
}

static void
nrFH_func (void)
{
    indent = 0;
    sub_indent = 0;
    insertPara (PARA_RESET);            /* Remove Bold; Indent etc. */
    latexStr ("\\FH ");
    latexEol ();
}

static void
nrIm_func (char *module, char *component)
{
    im_buf = bufFree (im_buf);
    if (module != NULL)
        im_buf = bufStr (NULL, module);
    bufFree (sectionComponent);
    if ((sectionComponent = bufNStr (NULL, nrImGetFirst (component))) != NULL)
        if (islower (sectionComponent[0]))
            sectionComponent[0] = toupper (sectionComponent[0]);

}

static void
nrId_func (char *date)
{
    bufFree (sectionDate);
    sectionDate = bufNStr (NULL, date);
}

static void
nrTH_func (char *id, char *num, char *date, char *company, char *title)
{
    bufFree (sectionName);
    bufFree (sectionId);

    indent = 0;
    sub_indent = 0;
    mode = BOLD_MODE | PAGE_MODE | TITLE_MODE;

    sectionName = bufStr (NULL, id);
    sectionId = bufNStr (NULL, num);

    sectionLevel = nrGetLevel (sectionName, sectionId);
    if (sectionLevel == 0)
        latexStr("\\TH{%s}{}", sectionName);
    else
        latexStr("\\TH{%s}{%d}", sectionName, sectionId);
    latexEol();                          /* Make pretty */
    latexStr ("% %s - Version %s - %s - %s - Jon Green (2002)",
             MODULE_NAME, MODULE_VERSION, nroffVersion, __DATE__);
    latexEol ();                         /* Make pretty */

    prevMode = 0;
    mode = 0;                           /* Start at mode zero !! */
}

static void
nrNH_func (char *id, char *num, char *title, char *xref)
{
    nrFH_func ();

    bufFree (sectionName);
    bufFree (sectionId);

    indent = 0;
    sub_indent = 0;
    insertPara (PARA_CLOSE);

    /* Go into title mode */
    mode = BOLD_MODE | PAGE_MODE | TITLE_MODE;

    sectionName = bufStr (NULL, id);
    sectionId = bufNStr (NULL, num);

    if (sectionLevel == 0)
        latexStr("\\TH{%s}{}", sectionName);
    else
        latexStr("\\TH{%s}{%d}", sectionName, sectionId);
    prevMode = 0;
    mode = 0;                           /* Start at mode zero !! */
}

static void
nrGH_func (char *title)
{

    if (title == NULL)
        return;

    indent = 0;
    sub_indent = 0;
    insertPara (PARA_CLOSE);
    mode = BOLD_MODE;

    latexFormatStr (mode, "", NULL);
    latexStr ("#{\\footnote hg%s}", nrMakeXref (title,NULL));
    latexFormatStr (mode, title, NULL);
    setParaMode (PARA_TERM);
    mode = 0;
}

static void
nrSH_func (char *s)
{
    indent = 0;                         /* Adjust margins to zero !! */
    sub_indent = 0;
    insertPara (PARA_RESET);            /* Shut down last paragraph !! */

    /* Add heading string to introduce title */

    latexStr ("\\SH{");                 /* Add openners !! */
    insert (s, BOLD_MODE|PARA_FORCE);   /* Add string - force out */
    setParaMode (0);                    /* Make sure all bolds processed !! */
    latexStr ("}");                     /* Add closers !! */
    latexEol ();                        /* Make pretty */

    mode = JUST_MODE;                   /* Set up mode for followig text */
    indent = FULL_INDENT;               /* Require 1 indent !! */
    prevMode = 0;
}

static void
nrSS_func (char *s)
{
    indent = 0;                         /* Adjust margins to zero !! */
    sub_indent = 0;
    insertPara (PARA_RESET);            /* Shut down last paragraph !! */

    /* Add heading string to introduce title */

    latexStr ("\\SS{");                 /* Add openners !! */
    insert (s, PARA_FORCE);             /* Add string - force out */
    setParaMode (0);                    /* Make sure all bolds processed !! */
    latexStr ("}");                     /* Add closers !! */
    latexEol ();                        /* Make pretty */

    mode = JUST_MODE;                   /* Set up mode for followig text */
    indent = FULL_INDENT;               /* Require 1 indent !! */
    prevMode = 0;
}

static void
nrPP_func (void)
{
    insertPara (PARA_TERM);
    sub_indent = 0;
}

static void
nrHl_func (char *text, char *name, char *section,
           char *concat, char *module, char *file, int type)
{
    /* Make sure in paragraph - shut down formating */
    latexFormatStr ((mode & ~(BOLD_MODE|ITALIC_MODE)), "", NULL);

    if (name != NULL) {
        latexEol ();                     /* Make pretty */
        latexStr ("\\Hl{");
        latexStr ("%s}{", nrMakeXref (name, section));

        /* Shove in the name to display to user */
        if (text != NULL)
            latexFormatStr (mode, text, NULL);
        else
        {
            latexFormatStr (mode, name, NULL);
            if (section != NULL)
                latexFormatStr (mode, section, NULL);
        }
        latexStr ("}");
    }

    if (concat != NULL)
        latexFormatStr (mode, concat, NULL);
    latexFormatStr (mode, " ", NULL);
}

static void
nrHt_func (char *name, char *section, char *concat,
           char *module, char *file, int type)
{
    /* Make sure in paragraph - shut down formating */
    latexFormatStr ((mode & ~(BOLD_MODE|ITALIC_MODE)), "", NULL);

    if (name != NULL) {
        /* Shove in the hypertext link */
        latexEol ();                     /* Make pretty */
        latexStr ("\\Ht{%s}{", nrMakeXref (name, section));

        /* Shove in the name to display to user */
        latexFormatStr (mode, name, NULL);
        if (section != NULL) {
            latexFormatStr (mode, "(", NULL);
            latexFormatStr (mode, section, NULL);
            latexFormatStr (mode, ")", NULL);
        }
        latexStr ("}");
    }

    if (concat != NULL)
        latexFormatStr (mode, concat, NULL);
    latexFormatStr (mode, " ", NULL);
}

static void
nrHr_func (char *name, char *section, char *concat)
{
    if (name != NULL)
        latexFormatStr (BOLD_MODE, name, NULL);
    if (section != NULL) {
        latexFormatStr (BOLD_MODE, "(", NULL);
        latexFormatStr (BOLD_MODE, section, NULL);
        latexFormatStr (BOLD_MODE, ")", NULL);
    }
    if (concat != NULL)
        latexFormatStr (mode, concat, NULL);
    latexFormatStr (mode, " ", NULL);
}

static void
nrHg_func (char *name, char *concat)
{
    /* Make sure in paragraph - shut down formating */
    latexFormatStr ((mode & ~(BOLD_MODE|ITALIC_MODE)), "", NULL);

    if (name != NULL) {
        latexEol ();                         /* Make pretty */
        latexStr ("<A HREF=<<Rhg%s<>", nrMakeXref (name, NULL));

        /* Shove in the name to display to user */
        latexStr ("%s</A>", name);
    }
    if (concat != NULL)
        latexFormatStr (mode, concat, NULL);
    latexFormatStr (mode, " ", NULL);
}

static void
nrGr_func (char *align, char *fname)
{
    latexStr ("<IMG SRC=\"%s.gif\" BORDER=0 ALIGN=BOTTOM ALT=\"[%s]\">" ,
             fname, fname);
}

static int bulletItemList = 0;

static void
nrBS_func (int i, int j, char *bullet)
{
    insertPara (PARA_TERM);

    /* Compute the new bullet offset */
    if ((bullet == NULL) || (strlen (bullet) <= 1)) {
        indent += FULL_INDENT;
        bulletItemList = 0;
    }
    else
        bulletItemList = 1;
    
    /* First space */
    insertPara (PARA_NORMAL);
    insert ("", 0);
    latexEol ();
    latexStr("\\BS{");
    if (bulletItemList == 0)
        latexStr ("$\\bullet$");
    else
    {
#if 0
        if (j == 0)
            latexStr ("<DL COMPACT>");
        else
            latexStr ("<DL>");
        latexEol ();
        latexStr ("<DT>");
        insert (bullet, 0);
        latexStr ("<DD>");
#else
        insert (bullet, 0);      
#endif
    }
    latexStr("}");
}

static void
nrBU_func (int i, char *bullet)
{
    if (bulletItemList == 0)
    {
        if (i != 0) {
            insert ("", 0);
            insertPara (PARA_NORMAL);
        }
        latexEol ();
        latexStr("\\BU{$\\bullet$}");
    }
    else
    {
        latexEol ();
        latexStr("\\BU{");
        insert (bullet, 0);
        latexStr ("}");
    }
}

static void
nrBE_func (int i)
{
    latexEol();
    latexStr ("\\BE");              /* End list */
    latexEol();
    if (i != 0)
        insertPara (PARA_NORMAL);
    
    if (bulletItemList == 0)
    {
        insertPara (PARA_TERM);
        indent -= FULL_INDENT;
    }
    else
    {
        bulletItemList = 0;             /* Reset list type */
    }
}

static void
nrCS_func (int i)
{
    mode |= CODE_MODE;
    mode &= ~JUST_MODE;
    indent += FULL_INDENT;              /* Full indent for code section */
    sub_indent = 0;                     /* Remove any sub-indent assignment */
    insertPara (PARA_RESET);
    latexStr ("\\CS ");
    latexEol ();
}

static void
nrCE_func (int i)
{
    mode &= ~CODE_MODE;
    mode |= JUST_MODE;
    indent -= FULL_INDENT;
    latexStr ("\\CE ");
    setIndentMode ();
    latexEol ();
    while (--i >= 0)
        insertPara (PARA_TERM);
    /* sub_indent = 0; */
}

static void
nrRS_func (void)
{
    indent += FULL_INDENT;
}

static void
nrRE_func (void)
{
    indent -= FULL_INDENT;
}

static void
nrna_func (void)
{
    mode &= ~JUST_MODE;
}

static void
nrnf_func (void)
{
    mode |= PARA_MODE;
}

static void
nrfi_func (void)
{
    mode &= ~PARA_MODE;
}

static void
nrad_func (void)
{
    mode |= JUST_MODE;
}

static void
nrps_func (int i)
{
    if (ps_size != 0)
        latexStr ("\\PS{0}");
    ps_size = ps_size + (i/2);
    if (ps_size != 0)
        latexStr ("\\PS{%d}", ps_size);
}

static void
nrvs_func (int i) {;}

static void
nrbr_func (void)
{
    para_mode |= PARA_BREAK;
}

static void
nrbp_func (void)
{
    /* We do not do new pages in LaTeX. Force a paragraph break. */
    insertPara (PARA_TERM);
}

static void nrsp_func (int i)
{
    /* We do not do new lines in LaTeX. Force a paragraph break. */
    insertPara (PARA_TERM);
}

static void
nrTP_func (int i)
{
    if (i != 0)                         /* Start of function */
    {
        sub_indent = 0;
        insertPara (PARA_TERM);
    }
    else                                /* End of function */
    {
        sub_indent = FULL_INDENT;
/*        para_mode |= PARA_BREAK;*/
    }
}

static void
nrIP_func (char *s)
{
    nrTP_func (1);
    if (s != NULL) {
        insert (s, 0);
        setParaMode (0);
    }
    nrTP_func (0);
}

static void
nrLP_func (void)
{
    sub_indent = 0;
    insertPara (PARA_TERM);
}

static void
nrBP_func (char *s)
{
    nrTP_func (1);
    if (s != NULL) {
        insert (s, BOLD_MODE);
        setParaMode (0);
    }
    nrTP_func (0);
}

static void
nrft_func (int font)
{
    prevMode = mode & FONT_MODE;
    switch (font)
    {
    case FT_B:
        mode = (mode & ~FONT_MODE)|BOLD_MODE;
        break;
    case FT_R:
        mode = mode & ~FONT_MODE;
        break;
    case FT_I:
        mode = (mode & ~FONT_MODE)|ITALIC_MODE;
        break;
    case FT_C:
        mode = (mode & ~FONT_MODE)|MONO_MODE;
        break;
    }
    if (prevMode == (mode & FONT_MODE))
        uError (".ft Font already enabled\n");
}

static void
nrTextline (char *s)
{
    if (mode & CODE_MODE) {
        if (s != NULL)
            insert (s, mode);
        latexStr("\\newline");
        latexEol();
    }
    else if (mode & PARA_MODE)
    {
        /* JDG 040895        setIndentMode ();*/
        if (s == NULL) {
#if 1
            insertPara (PARA_TERM);
#else
            nrPP_func ();               /* Pretend we have a .PP !! */
#endif
        }
        else {
            insert (s, mode);
            nrbr_func ();
        }
    }
    else if (s != NULL)
        insert (s, mode);
    else {
#if 1
        insertPara (PARA_TERM);
#else
        nrPP_func ();                   /* Pretend we have a .PP !! */
#endif
    }
}

static void
nrXI_func (char *name, char *id, char *desc, char *comp)
{
    if (bodgeMode)
    {
        if (name != NULL)
        {
            latexStr ("<!-- XI: %s", name);
            if (id != NULL)
                latexStr ("(%s)", id);
            else if (sectionId != NULL)
                latexStr ("(%s)", sectionId);
            latexStr (" -->");
            latexEol ();
        }
    }
    
    if (compiling != 0)                 /* If we are compiling do not need */
        return;

    if (id == NULL)
        id = sectionId;

#ifdef THREF
    if ((strcmp (name, sectionName) != 0) ||
        ((id != NULL) &&
         (sectionId != NULL) &&
         (strcmp (id, sectionId) != 0)))
#endif
    {
        /* Add definition x-ref */
        latexStr ("<<D%s<", nrMakeXref (name,id));
    }
}

static void
nrXJ_func (char *name, char *id, char *desc, char *comp)
{
    if (id == NULL)
        id = sectionId;
#ifdef THREF
    if ((strcmp (name, sectionName) != 0) ||
        ((id != NULL) &&
         (sectionId != NULL) &&
         (strcmp (id, sectionId) != 0)))
#endif
    {
        /* Add definition x-ref */
        latexStr ("<A NAME=\"%s\">", nrMakeXref (name,id));
        if (compiling == 0)                 /* If we are not compiling add definition */
        {
            latexStr ("<<D%s<", nrMakeXref (name,id));
        }
        if (bodgeMode)
        {
            latexStr ("<!-- XJ: %s(%s) -->", name, id);
            latexEol ();
        }
    }
}

static void
nrXP_func (char *name, char *id, char *desc, char *comp)
{
    if (compiling != 0)                 /* If we are compiling do not need */
        return;

    if (id == NULL)
        id = sectionId;

#ifdef THREF
    if ((strcmp (name, sectionName) != 0) ||
        ((id != NULL) &&
         (sectionId != NULL) &&
         (strcmp (id, sectionId) != 0)))
#endif
    {
        /* Add definition x-ref */
        latexStr ("<<DP%s<", nrMakeXref (name,id));
    }
}

static void
nrMe_func (char *text)
{
    if (bodgeMode == 0)
        return;
    if ((text != NULL) && (*text != '\0'))
    {
        insertPara (0);
        latexStr ("<!-- Me: %s -->", text);
    }
}

static void
nrStartInc (char *fname, int *imode)
{
    *imode = 1;                         /* Disable processing of includes */
}

static void
nrStart (char *fname, int flag)
{
    /* Set up the conversion */

    para_mode = 0;
    para_clean = 0;
    para_indent = 0;
    mode = 0;
    makeLaTeXName (fname);
    prevMode = 0;
}

static void
nrEnd (char *fname, int imode, int flag)
{
    if (imode == 0)
        insertPara (PARA_CLOSE);     /* Flush out the buffer */
}

static void
nrComment (char *comment, int priority)
{
    if (priority == 0)
        return;
    if (comment[0] != '\0')
        latexStr ("% ", comment);
    latexEol ();
}

void
droff_init (void)
{
#if 0
    int     i;

    for (i = 0; latexHeader [i] != NULL; i++) {
        latexStr (latexHeader[i]);
        latexEol ();
    }
#else
    ;
#endif
}

void
droff_term (void)
{
    latexEol ();
}

void quit (int signum)
{
    exit (1);
}

static void
usage (void)
{
    fprintf (stdout, "%s : Usage %s [Options] <files>\n", progname, progname);
    fprintf (stdout, "Convert Nroff files to LaTeX\n");
    fprintf (stdout, "\n");
    fprintf (stdout, "-?/h      : Help - this page\n");
    fprintf (stdout, "-c <name> : Copyright <name>\n");
    fprintf (stdout, "-b        : Bodge mode for Microemacs (temporary only)\n");
    fprintf (stdout, "-E <file> : Log errors to <file> (append)\n");
    fprintf (stdout, "-e <file> : Log errors to <file> (re-write)\n");
    fprintf (stdout, "-H <home> : Home page name. (default is 'home')\n");
    fprintf (stdout, "-I        : Version Information\n");
    fprintf (stdout, "-i <num>  : Local section identity number.\n");
    fprintf (stdout, "-l <lib>  : Library component\n");
    fprintf (stdout, "-L <path> : Library search path\n");
    fprintf (stdout, "-M <mod>  : Module name\n");
    fprintf (stdout, "-n <name> : Local section name.\n");
    fprintf (stdout, "-o <file> : Output to <file> default is <files>.out\n");
    fprintf (stdout, "-p <file> : Name of the logo picture (image)\n");
    fprintf (stdout, "-q        : Quiet\n");
    fprintf (stdout, "-s        : Output to standard out.\n");
    fprintf (stdout, "-t <name:link:section>\n");
    fprintf (stdout, "-t arg    : Title to appear at top of page\n");
    fprintf (stdout, "-x        : Resolve all references of module\n");
    exit (1);
}


static void latexInitialise (void)
{
    static  nrFUNCTION funcTab = {NULL};
    time_t     clock;		/* Time in machine format. */
    struct tm  *time_ptr;       /* Pointer to time frame. */

    /* Get the time - require the year */
    clock = time (0);
    time_ptr = (struct tm *) localtime (&clock);	/* Get time frame */
    year = time_ptr->tm_year + 1900;    /* The year */
    month = time_ptr->tm_mon + 1;       /* The month */
    day = time_ptr->tm_mday;            /* The day of the month */
    
    /* Headers */
    nrInstall (funcTab, NH_func, nrNH_func);
    nrInstall (funcTab, FH_func, nrFH_func);
    nrInstall (funcTab, TH_func, nrTH_func);
    nrInstall (funcTab, GH_func, nrGH_func);
    nrInstall (funcTab, SH_func, nrSH_func);
    nrInstall (funcTab, SS_func, nrSS_func);

    /* Identification */
    nrInstall (funcTab, XI_func, nrXI_func);
    nrInstall (funcTab, XJ_func, nrXJ_func);
    nrInstall (funcTab, XP_func, nrXP_func);
    nrInstall (funcTab, Id_func, nrId_func);
    nrInstall (funcTab, Im_func, nrIm_func);
    nrInstall (funcTab, Kw_func, NULL);

    /* Paragraphs */
    nrInstall (funcTab, PP_func, nrPP_func);
    nrInstall (funcTab, TP_func, nrTP_func);
    nrInstall (funcTab, IP_func, nrIP_func);
    nrInstall (funcTab, LP_func, nrLP_func);
    nrInstall (funcTab, BP_func, nrBP_func);

    /* Hypertext */
    nrInstall (funcTab, Hl_func, nrHl_func);
    nrInstall (funcTab, Hh_func, nrHl_func);
    nrInstall (funcTab, Ht_func, nrHt_func);
    nrInstall (funcTab, Hr_func, nrHr_func);
    nrInstall (funcTab, Hg_func, nrHg_func);

    /* Graphics */
    nrInstall (funcTab, Gr_func, nrGr_func);
    
    /* Fonts */
    nrInstall (funcTab, ft_func, nrft_func);
    nrInstall (funcTab, CS_func, nrCS_func);
    nrInstall (funcTab, CE_func, nrCE_func);

    /* Bullets */
    nrInstall (funcTab, BS_func, nrBS_func);
    nrInstall (funcTab, BU_func, nrBU_func);
    nrInstall (funcTab, BE_func, nrBE_func);

    /* Indentation */
    nrInstall (funcTab, RS_func, nrRS_func);
    nrInstall (funcTab, RE_func, nrRE_func);
    nrInstall (funcTab, comment_func, nrComment);

    /* Text adjustment */
    nrInstall (funcTab, na_func, nrna_func);
    nrInstall (funcTab, nf_func, nrnf_func);
    nrInstall (funcTab, fi_func, nrfi_func);
    nrInstall (funcTab, ps_func, nrps_func);
    nrInstall (funcTab, vs_func, nrvs_func);
    nrInstall (funcTab, ad_func, nrad_func);
    nrInstall (funcTab, br_func, nrbr_func);
    nrInstall (funcTab, bp_func, nrbp_func);
    nrInstall (funcTab, ll_func, NULL);
    nrInstall (funcTab, pl_func, NULL);
    nrInstall (funcTab, sp_func, nrsp_func);
    nrInstall (funcTab, ne_func, NULL);

    /* Files */
    nrInstall (funcTab, so_func, NULL);
    nrInstall (funcTab, startFile_func, nrStart);
    nrInstall (funcTab, includeFile_func, nrStartInc);
    nrInstall (funcTab, endFile_func, nrEnd);

    /* Text */
    nrInstall (funcTab, textline_func, nrTextline);

    /* Special */
    nrInstall (funcTab, Me_func, nrMe_func);
    
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

    uErrorSet (0, &ecount);             /* Max Num entries + counter */
    uWarnSet (&wcount);                 /* Warning count */
    uUtilitySet (progname);             /* Set name of program */
    uVerboseSet (0);                    /* Verbose setting */

    while (1) {
        c = getopt (argc, argv, "c:be:E:H:i:Il:L:M:n:o:p:qst:x");
        if (c == EOF)
            break;
        switch (c) {
        case 'c':
            copyrightName = optarg;
            break;
        case 'b':
            bodgeMode=1;
            break;
        case 'e':
            uErrorChannelSet (optarg, UERROR_LOG_CREATE);
            break;
        case 'E':
            uErrorChannelSet (optarg, UERROR_LOG_APPEND);
            break;
        case 's':
            fo = stdout;
            break;
        case 'q':
            quietMode = 1;
            uInteractiveSet (0);        /* Disable interactive mode */
            break;
        case 'n':
            localName = bufNStr (NULL, optarg);
            break;
        case 'i':
            localNum = bufNStr (NULL, optarg);
            break;
        case 'o':
            oname = optarg;
            break;
        case 'p':
            logoName = optarg;
            break;
        case 'H':
            nrHomeSet (optarg);
            break;
        case 'M':
            bufFree (moduleName);
            moduleName = bufNStr (NULL, optarg);
            if (nrLibSetModule (optarg) != 0)
                exit (1);
            break;
        case 'L':
            nrLibSetPath (optarg);
            break;
        case 'l':
            nrLibLoad (optarg);
            break;
        case 'I':
            fprintf (stdout, "%s. Version %s. %s. %s\n",
                     progname, MODULE_VERSION, __DATE__, nroffVersion);
            exit (0);
            break;
        case 't':
            nrTitleSet (optarg);
            break;
        case 'x':
            compiling = 1;
            break;
        case 'h':
        case '?':
            usage();
            break;
        default:
            uError ("Cannot understand option [-%c]\n", c);
            usage();
            break;
        }
    }

    /*
     * Note - load nroff functions after the options are processed to
     * ensure that the library is not processed early.
     */

    uOpenErrorChannel ();
    latexInitialise();
    argv = getfiles (&argc, argv, optind);

    if ((oname == NULL) && (moduleName != NULL))
        oname = strdup (makeFilename (NULL, NULL, moduleName, "hts"));

    if (argc > 1) {
        if (oname != NULL) {
            if ((fpr = fopen (oname, "wb")) == NULL)
                uFatal ("Cannot open file [%s]\n", oname);
            uVerbose (0, "Output in file [%s].\n", oname);

            /*
             * If this is a module and we are compiling then force the module subdirectory
             * to be created - this is the same as the module name.
             */
            if ((compiling) && (moduleName != NULL))
                fprintf (fpr, "<<SUBDIRECTORY> %s >\n", moduleName);

            droff_init ();
        }
    }
    else
        fpr = NULL;

    /* Process all of the files */

    for (i = 1; i < argc; i++) {
        if ((nrfp = nrFilePush (argv[i])) == NULL)
            exit (1);
#if 0
        makeLaTeXName (nrfp->fileName);
#endif
        if (oname == NULL && fpr == NULL)
        {
            char *drive;
            char *path;
            char *base;
            char *name;
            
            if (splitFilename (nrfp->fileName, &drive, &path, &base, NULL) != 0)
                uFatal ("Cannot decompose filename [%s]\n", nrfp->fileName);
            if ((name = makeFilename (drive, path, base, "tex")) == NULL)
                uFatal ("Cannot compose filename from split [%s]\n",
                        nrfp->fileName);
            if ((fpr = fopen (name, "wb")) == NULL)
                uFatal ("Cannot open file %s\n", name);
            droff_init ();              /* Start up if in sections */
        }
            
        nroff (nrfp, ((compiling == 0) ? NROFF_MODE_DEFAULT :
                      NROFF_MODE_COMPILE));

        fflush (fpr);
        fflush (stdout);
        if ((oname == NULL) && (fpr != stdout)) {
            droff_term ();
            latexEof ();
            fclose (fpr);
            fpr = NULL;
        }
    }
    if (fpr != NULL) {
        droff_term ();
        latexEof ();
        fclose (fpr);
    }

    uCloseErrorChannel ();
    return (ecount);
}
