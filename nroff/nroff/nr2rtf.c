/****************************************************************************
 *
 *			Copyright 1996-2004 Jon Green.
 *                         All Rights Reserved
 *
 *
 *  System        :
 *  Module        :
 *  Object Name   : $RCSfile: nr2rtf.c,v $
 *  Revision      : $Revision: 1.3 $
 *  Date          : $Date: 2004-02-07 19:29:49 $
 *  Author        : $Author: jon $
 *  Last Modified : <040207.1919>
 *
 *  Description
 *
 *  Notes
 *
 *  History
 *
 * Version 1.1.0g  - 07/02/04 - JG
 * Ported to HP-UX 11.00
 *
 * Version 1.1.0f  - 03/01/04 - JG
 * Ported to Sun Solaris 9
 *
 * Version 1.1.0e  - 03/05/97 - JG
 * Ported to win32
 *
 * Version 1.1.0d  - 16/04/97
 * Added copyright option.
 *
 * Version 1.1.0c  - 03/09/96
 * Added logo option.
 *
 * Version 1.1.0b  - 28/08/96 - JG
 * Added file handling name routines for UNIX
 *
 * Version 1.1.0a  - 06/07/96 - JG
 * Corrected header line. Lines now adjacent with image.
 *
 * Version 1.1.0  - 15/12/95 - JG
 * Added immediate mode compilation.
 *
 * Version 1.0.0e - 05/12/95 - JG
 * Added support for bullets.
 *
 * Version 1.0.0d - 16/11/95 - JG
 * Linked with new utilities library.
 *
 * Version 1.0.0c - 03/10/95 - JG
 * Remove the sub-indent anomaly with RTF generation on .CS.
 *
 * Version 1.0.0b - 03/10/95 - JG
 * Added word mode to generated rtf for word.
 *
 * Version 1.0.0a - 16/08/95 - JG
 * Added browse section number to rtf converter.
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
#include <sys/types.h>
#include <time.h>

#if ((defined _HPUX) || (defined _LINUX) || (defined _SUNOS))
#include <unistd.h>
#else
#include <getopt.h>
#endif
#include <utils.h>

#include "nroff.h"
#include "rtf.h"

/* Macro Definitions */

#define MODULE_VERSION  "1.1.0g"
#define MODULE_NAME     "nr2rtf"

#define NORMAL_MODE     0x0000
#define BOLD_MODE       0x0100
#define ITALIC_MODE     0x0200
#define MONO_MODE       0x8000
#define FONT_MODE       (BOLD_MODE|ITALIC_MODE|MONO_MODE)

#define MODE_CHAR       0x007f
#define MODE_MASK       0xff00
#define JUST_MODE       0x0400
#define PARA_MODE       0x0800
#define SPACE_MODE      0x1000
#define TITLE_MODE      0x2000
#define PAGE_MODE       0x4000
#define SUB_TITLE_MODE  0x1000

#define PARA_CLOSE      0
#define PARA_NORMAL     1
#define PARA_BLANK      2
#define PARA_TERM       3

#define FONT_POINTS(x)  ((x)*2)         /* Convert to points */

static int para_clean;
static int para_mode;
static int font_type;                   /* Current font type */
static int font_size;                   /* Current font size */
/*static int bold;*/
/*static int italic;*/
static int indent;
static int sub_indent;
static int first_indent;
static int mode;
static int quietMode = 0;
/*static int warnMode = 0;*/
static int wordMode = 0;
static int  prevMode = 0;
static int compiling = 0;               /* Compile to resolve all refrences */
static char *logoName = "logo.bmp";     /* Default logo name */
static int ps_size = 0;
static int vs_size = 0;
static char *im_buf = NULL;
static char *sectionDate = NULL;
/*static char *comp_buf = NULL;*/
static char *progname;
static char *copyrightName = NULL;
static char *sectionName = NULL;
static char *sectionId = NULL;
static char *sectionComponent = NULL;
static char *localName = NULL;
static char *localNum = NULL;
/*static FILE *fp;*/
static FILE *fo;
static FILE *fpr = NULL;
static fontType *rtfFont;
static int  year;                       /* The current year */
static int  month;                      /* The current month */
static int  day;                        /* The current day */

static fontType winFont =
{
    2,                  /* SS Section font type */
    20,                 /* SS Section font size */
    2,                  /* SH Section font type */
    24,                 /* SH Section font size */
    2,                  /* Text section font type */
    20,                 /* Text section font size */
    4,                  /* Code section font type */
    18,                 /* Code section font size */
    2,                  /* Title line font type */
    28,                 /* Title line font size */
    4,                  /* Mono font type */
    20,                 /* Mono font size */
    2,                  /* Copyright font type */
    16                  /* Copyright font size */
};

static fontType manFont =
{
    0,                  /* SS Section font type */
    20,                 /* SS Section font size */
    0,                  /* SH Section font type */
    24,                 /* SH Section font size */
    0,                  /* Text section font type */
    20,                 /* Text section font size */
    4,                  /* Code section font type */
    16,                 /* Code section font size */
    0,                  /* Title line font type */
    28,                 /* Title line font size */
    4,                  /* Mono font type */
    20,                 /* Mono font size */
    0,                  /* Copyright font type */
    16                  /* Copyright font size */
};

static char *
makeTopicName (char *head, char *s)
{
    char    *tname;
    char    c;

    tname = bufStr (NULL, head);
    while ((c = *s++) != '\0')
    {
        if (((c >= 'A') && (c <= 'Z')) ||
            ((c >= 'a') && (c <= 'z')) ||
            (c == '.') ||
            (c == '_'))
        {
            tname = bufChr (tname, c);
        }
    }
    return (tname);
}

static void
rtfEof (void)
{
#ifndef _UNIX
    fprintf (fpr, "%c", 0x1a);
#endif
}

static void
rtfEol (void)
{
#ifndef _UNIX
    fprintf (fpr, "\r\n");
#else
    fprintf (fpr, "\n");
#endif
}

static void
rtfStr (char *format, ...)
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
        rtfStr ("}");
    }
    if (((mode  & ITALIC_MODE) == 0) && ((para_mode & ITALIC_MODE) != 0)) {
        para_mode &= ~ITALIC_MODE;
        rtfStr ("}");
    }
    if (((mode  & MONO_MODE) == 0) && ((para_mode & MONO_MODE) != 0)) {
        para_mode &= ~MONO_MODE;
        rtfStr ("}");
    }

    /* Turn on any highlighting modes if required. */
    if (((mode  & BOLD_MODE) != 0) && ((para_mode & BOLD_MODE) == 0)) {
        para_mode |= BOLD_MODE;
        rtfStr ("{\\b ");
    }
    if (((mode  & ITALIC_MODE) != 0) && ((para_mode & ITALIC_MODE) == 0)) {
        para_mode |= ITALIC_MODE;
        rtfStr ("{\\i ");
    }
    if (((mode  & MONO_MODE) != 0) && ((para_mode & MONO_MODE) == 0)) {
        para_mode |= MONO_MODE;
        rtfStr ("{\\f4 ");
    }
}

static void
insertPara (int pmode)
{
    char *justStr;
    char *paraStr;
    char *titlStr;
    char *pageStr;
    char firstStr [20];
    char tabStr [20];

    /* End the last paragraph. Close any highlighting if necessary */
    if (pmode == PARA_TERM) {
        setParaMode (0);
        para_mode = PARA_TERM;
    }
    else if (para_clean != 0) {
        setParaMode (0);                /* Turn off any enabled modes */
        para_mode = 0;                  /* Reset the mode */
        rtfEol ();                      /* Terminate the line */
    }

    /* Start a new paragraph if requested. Use the formatting required. */
    if (((pmode & 0xf) == PARA_BLANK) ||
        ((pmode & 0xf) == PARA_NORMAL))
    {

        /* Determine the formating parameters */
        justStr = ((mode & JUST_MODE) &&
                   (pmode != PARA_BLANK))? "\\qj" : "";
        paraStr = (mode & PARA_MODE) ? "\\keep" : "";
        if (mode & TITLE_MODE)
        {
            if (mode & SUB_TITLE_MODE)
                titlStr = "\\keepn";        /* Sub-header line */
            else
                titlStr = "\\keepn \\sb60"; /* Header line */
        }
        else
            titlStr = "";
        pageStr = (mode & PAGE_MODE) ? "\\page" : "\\pard";

        /*
         * Handle the first indent. If it is smaller than 0 then
         * set up a tab to take us to the indent.
         */
        if (first_indent < 0)
            sprintf (tabStr, "\\tx%d", indent+sub_indent);
        else
            tabStr [0] = '\0';
        if (first_indent != 0)
            sprintf (firstStr, "\\fi%d", first_indent);
        else
            firstStr [0] = '\0';

        /* Pump out the start of the paragraph */
        rtfStr ("\\par %s%s%s %s%s\\li%d%s \\f%d\\fs%d ",
                pageStr,
                paraStr,
                titlStr,
                justStr,
                tabStr,
                indent+sub_indent,
                firstStr,
                font_type,
                font_size + ps_size);
        para_mode = pmode;
        para_clean = 1;
    }

    if ((pmode != PARA_NORMAL) && (pmode != PARA_TERM)) {
        if (pmode == PARA_BLANK)
            rtfEol ();
        para_clean = 0;
        para_mode = 0;
    }

}   /* End of 'insertPara' */

static void
rtfFormatStr (int lmode, char *s, int *newMode)
{
    int space = 0;
    char c;

    if (newMode != NULL)
        *newMode = lmode;
    if (para_mode == PARA_TERM)
        insertPara (PARA_CLOSE);
    if (para_mode == 0)
        insertPara (PARA_NORMAL);
    setParaMode (lmode);

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
                uWarn ("Roman requested and already in Roman.\n");
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
        case '{':
        case '}':
        case '\\':
            rtfStr ("\\%c", c);
            break;
        case ' ':
            if (((lmode & SPACE_MODE) != 0) && (space != 0))
                break;
            rtfStr ("%c", c);
            space = 1;
            break;
        case ESC_CHAR:
            rtfStr ("\\");
            break;
        case USPACE_CHAR:
            rtfStr (" ");
            break;
        case MSPACE_CHAR:
        case ZSPACE_CHAR:
            break;
        case COPYRIGHT_CHAR:
            rtfStr ("(c)");
            break;
        case DEGREE_CHAR:
            rtfStr ("o");
            break;
        case BULLET_CHAR:
            rtfStr ("{\\f1\\b\\fs24\\'b7}");   /* A Bullet !! */
            break;
        default:
            rtfStr ("%c", c);
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
    if (para_mode == PARA_TERM)
        insertPara (PARA_CLOSE);
    if (para_mode == 0)
        insertPara (PARA_NORMAL);
#if 0
    setParaMode (mode);                 /* Enable/Disable necessary modes */
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
        rtfFormatStr (mode,buffer, &mode);
        if ((mode & SPACE_MODE) && (*s == '\0'))
            break;
        rtfFormatStr (mode, " ", &mode);
    }
}

static void
nrFH_func (void)
{
    indent = 0;
    sub_indent = 0;
    insertPara (PARA_CLOSE);
    setParaMode (0);
    /* Set up the font size */
    font_type = rtfFont->co_font;
    font_size = rtfFont->co_size;

    /* Add the copyright string */
    insertPara (PARA_BLANK);
    if (copyrightName != NULL)
    {
        rtfFormatStr (ITALIC_MODE, "", NULL);
        rtfStr ("(c) Copyright %s %4d", copyrightName, year);
        insertPara (PARA_TERM);
    }
    /* Add the modification date */
    if (sectionDate != NULL)
    {
        rtfFormatStr (ITALIC_MODE, "", NULL);
        rtfStr ("Last Modified: %s", sectionDate);
        insertPara (PARA_TERM);
    }
    /* Add the generated on date */
    rtfFormatStr (ITALIC_MODE, "", NULL);
    rtfStr ("Generated On: %4d/%02d/%02d", year, month, day);
    insertPara (PARA_TERM);
    setParaMode (0);
}

static void
nrId_func (char *date)
{
    sectionDate = bufFree (sectionDate);
    if (date != NULL)
        sectionDate = bufStr (NULL, date);
}

static void
nrIm_func (char *module, char *component)
{
    im_buf = bufFree (im_buf);
    if (module != NULL)
        im_buf = bufStr (NULL, module);
    bufFree (sectionComponent);
    if ((sectionComponent = bufNStr (NULL, nrImGetFirst(component))) != NULL)
        if (islower ((int)(sectionComponent[0])))
            sectionComponent[0] = toupper (sectionComponent[0]);

}

static void
insertMainIcon (void)
{
    char *home;

    /* Create icon and link to the home page. */

    if (wordMode != 0)                  /* Quit if word mode */
        return;

    home = nrHomeGet ();                /* Get the home page */
    rtfStr ("{\\uldb \\{bml %s\\}}{\\v %%%s@%s.hlp}", logoName, home, home);
    rtfEol ();
}

static void
insertTitleLine (void)
{
    void *p;
    char *xref;
    char *module;
    char *title;

    if (wordMode != 0)                  /* Quit if word mode */
        return;

    /*
     * Check for a title line. Add a new line if required.
     */

    if ((p = nrTitleGet (NULL, &title, &xref, &module, NULL)) == NULL)
        return;
#if 0
    insertPara (PARA_CLOSE);
    mode = PARA_MODE|TITLE_MODE|SUB_TITLE_MODE;
#else
    setParaMode (0);
#endif
    rtfEol ();
    rtfStr ("\\line {\\f%d\\fs%d ", rtfFont->tx_font, FONT_POINTS (9));

    do
    {
        rtfFormatStr (0,"", NULL);      /* Force para out */
        rtfStr ("{\\strike ");
        rtfFormatStr (0,"[", NULL);
        rtfFormatStr (0, title, NULL);
        rtfFormatStr (0, "]", NULL);
        rtfStr ("}{\\v %s", xref);
        if (module != NULL)
            rtfStr ("@%s.hlp",module);
        rtfStr ("} ");

    } while ((p = nrTitleGet (p, &title, &xref, &module, NULL)) != NULL);

    rtfStr ("}");
    setParaMode (0);
    mode = 0;
    prevMode = 0;
}

static void
nrTH_func (char *id, char *num, char *date, char *company, char *title)
{

    bufFree (sectionName);
    bufFree (sectionId);

    indent = 0;
    sub_indent = 0;
    first_indent = 0;
    mode = BOLD_MODE | PAGE_MODE | TITLE_MODE;
    font_type = rtfFont->tl_font;
    font_size = rtfFont->tl_size;

    sectionName = bufStr (NULL, id);
    sectionId = bufNStr (NULL, num);

    rtfFormatStr (mode, "", NULL);
    if (wordMode == 0)
    {
        if (sectionId == NULL) {
            rtfStr ("${\\footnote %s}", sectionName);
#ifdef THREF
            rtfStr ("#{\\footnote %s}", nrMakeXref (sectionName, NULL));
#endif
#if 0
            rtfStr ("K{\\footnote %s}", sectionName);
#endif
        }
        else {
            rtfStr ("${\\footnote %s(%s)}", sectionName, sectionId);
#ifdef THREF
            rtfStr ("#{\\footnote %s}", nrMakeXref (sectionName, sectionId));
#endif
#if 0
            rtfStr ("K{\\footnote %s(%s)}", sectionName, sectionId);
#endif
        }
        if (sectionComponent != NULL)
            rtfStr ("K{\\footnote %s}", sectionComponent);
    }
    /*
     * Add a browse string back to the local section.
     */

#if 0
    if ((localName != NULL) &&
        ((strcmp (localName, sectionName) != 0) ||
         (strnullcmp (localNum, sectionId) != 0)))
    {
        if (wordMode == 0)
        {
            if (localNum == NULL)
                rtfStr ("+{\\footnote %s}", localName);
            else
                rtfStr ("+{\\footnote %s%s}", localName, localNum);
        }
    }
#endif

    insertMainIcon ();
    if (title == NULL) {
        rtfFormatStr (mode, sectionName, NULL);
        if (sectionId != NULL) {
            rtfFormatStr (mode, "(", NULL);
            rtfFormatStr (mode, sectionId, NULL);
            rtfFormatStr (mode, ")", NULL);
        }
    }
    else
        rtfFormatStr (mode, title, NULL);

    insertTitleLine ();              /* Add tile line to title */

    mode = 0;
    setParaMode (0);
    font_type = rtfFont->tx_font;
    font_size = rtfFont->tx_size;
}

static void
nrKw_func (char **argv)
{
    char    *parameter;

    rtfFormatStr (mode, "", NULL);
    if (wordMode == 0)
    {
        while ((parameter = *argv++) != NULL)
            rtfStr ("K{\\footnote %s}", parameter);
    }
}

static void
nrNH_func (char *id, char *num, char *title, char *xref)
{
#if 0
    indent = 0;
    sub_indent = 0;
    insertPara (PARA_CLOSE);
#else
    nrFH_func ();
#endif
    bufFree (sectionName);
    bufFree (sectionId);

    mode = BOLD_MODE | PAGE_MODE | TITLE_MODE;
    font_type = rtfFont->tl_font;
    font_size = rtfFont->tl_size;

    sectionName = bufStr (NULL, id);
    sectionId = bufNStr (NULL, num);

    rtfFormatStr (mode, "", NULL);

    if (wordMode == 0)
    {
        if (sectionId == NULL) {
            rtfStr ("${\\footnote %s}", sectionName);
            rtfStr ("#{\\footnote %s}", nrMakeXref (sectionName, NULL));
        }
        else {
            rtfStr ("${\\footnote %s(%s)}", sectionName, sectionId);
            rtfStr ("#{\\footnote %s}", nrMakeXref (sectionName, sectionId));
        }
    }

    insertMainIcon ();

    if (title == NULL) {
        rtfFormatStr (mode, sectionName, NULL);
        if (sectionId != NULL) {
            rtfFormatStr (mode, "(", NULL);
            rtfFormatStr (mode, sectionId, NULL);
            rtfFormatStr (mode, ")", NULL);
        }
    }
    else
        rtfFormatStr (mode, title, NULL);

    insertTitleLine ();                 /* Add tile line to title */

    font_type = rtfFont->tx_font;
    font_size = rtfFont->tx_size;
    setParaMode (PARA_TERM);
    sub_indent = 0;
    mode = 0;

}

static void
nrGH_func (char *title)
{

    char    *gtitle;

    if (title == NULL)
        return;

    indent = 0;
    sub_indent = 0;
    insertPara (PARA_CLOSE);
    mode = BOLD_MODE | PAGE_MODE;
    font_type = rtfFont->tx_font;
    font_size = rtfFont->tx_size;

    rtfFormatStr (mode, "", NULL);
    gtitle = makeTopicName ("hg", title);
    bufFree (gtitle);
    if (wordMode == 0)
        rtfStr ("#{\\footnote %s}", gtitle);
    rtfFormatStr (mode, title, NULL);
    setParaMode (PARA_TERM);
    mode = 0;
}

static void
nrSH_func (char *s)
{
    indent = 0;
    sub_indent = 0;
    first_indent = 0;
    insertPara (PARA_BLANK);
    font_type = rtfFont->sh_font;
    font_size = rtfFont->sh_size;
    insert (s, BOLD_MODE);
    insertPara (PARA_TERM);
    font_type = rtfFont->tx_font;
    font_size = rtfFont->tx_size;
    prevMode = 0;
    mode = JUST_MODE;
    indent = FULL_INDENT;
}

static void
nrSS_func (char *s)
{
    first_indent = 0;
    sub_indent = 0;
    indent = HALF_INDENT;
    insertPara (PARA_BLANK);
    font_type = rtfFont->ss_font;
    font_size = rtfFont->ss_size;
    insert (s, BOLD_MODE);
    insertPara (PARA_TERM);
    font_type = rtfFont->tx_font;
    font_size = rtfFont->tx_size;
    prevMode = 0;
    mode = JUST_MODE;
    indent = FULL_INDENT;
    sub_indent = 0;
}

static void
nrPP_func (void)
{
    insertPara (PARA_BLANK);
    font_type = rtfFont->tx_font;
    font_size = rtfFont->tx_size;
    sub_indent = 0;
}

static void
nrHl_func (char *text, char *name, char *section,
           char *concat, char *module, char *file, int type)
{
    setParaMode (0);
    rtfFormatStr (mode, "", NULL);

    if (name != NULL) {
        if (wordMode == 0)
        {
            rtfStr ("{\\strike %s}", text, name);
            rtfStr ("{\\v %s", nrMakeXref (name, section));
            if (module != NULL)
                rtfStr ("@%s.hlp",module);
            rtfStr ("}");
        }
        else
            rtfStr (text);
    }

    if (concat != NULL)
        rtfFormatStr (mode, concat, NULL);
    rtfFormatStr (mode, " ", NULL);
}

static void
nrHt_func (char *name, char *section, char *concat,
           char *module, char *file, int type)
{
    setParaMode (0);
    rtfFormatStr (mode, "", NULL);

    if (name != NULL)
    {
        if (wordMode == 0)
        {
            rtfStr ("{\\strike %s",name);
            if (section != NULL)
                rtfStr ("(%s)", section);
            rtfStr ("}{\\v %s", nrMakeXref (name, section));
            if (module != NULL)
                rtfStr ("@%s.hlp",module);
            rtfStr ("}");
        }
        else
        {
            rtfStr ("{\\b %s}", name);
            if (section != NULL)
                rtfStr ("(%s)", section);
        }
    }

    if (concat != NULL)
        rtfFormatStr (mode, concat, NULL);
    rtfFormatStr (mode, " ", NULL);
}

static void
nrHr_func (char *name, char *section, char *concat)
{
    rtfFormatStr (0, "", NULL);
    if (name != NULL)
    {
        if (wordMode == 0)
        {
            rtfStr ("{\\cf4 %s",name);
            if (section != NULL)
                rtfStr ("(%s)", section);
            rtfStr ("}");
        }
        else
        {
            rtfStr ("{\\b %s}", name);
            if (section != NULL)
                rtfStr ("(%s)", section);
        }
    }

    if (concat != NULL)
        rtfFormatStr (mode, concat, NULL);
    rtfFormatStr (mode, " ", NULL);
}

static void
nrHg_func (char *name, char *concat)
{
    char    *gname;

    setParaMode (0);
    rtfFormatStr (mode, "", NULL);

    if (name != NULL) {
        rtfStr ("{\\ul %s}",name);
        gname = makeTopicName ("hg", name);
        rtfStr ("{\\v %s}",gname);
        bufFree (gname);
    }
    if (concat != NULL)
        rtfFormatStr (mode, concat, NULL);
    rtfFormatStr (mode, " ", NULL);
}

static void
nrGr_func (char *align, char *fname)
{
    rtfFormatStr (mode, "", NULL);
    if (align == NULL)
        align = "l";
    if (wordMode == 0)
    {
        rtfStr ("\\{bm%c %s.bmp\\}", align[0], fname);
    }
}

static void
nrBS_func (int i, int j, char *bullet)
{
    insertPara (PARA_TERM);
    indent += FULL_INDENT;

    /* Compute the new bullet offset */
    if ((bullet == NULL) || (strlen (bullet) <= 1))
        first_indent = -((FULL_INDENT * 3)/4);
    else
        first_indent = -FULL_INDENT;

    /* Fill out blank lines as requested */
    while (--i >= 0)
        insertPara (PARA_BLANK);
    insert ("",0);                      /* Force paragraph out */

    /* Insert the bullet */
    if (bullet == NULL)
        rtfStr ("{\\f1\\b\\fs24\\'b7}");    /* A bullet - believe it or not !! */
    else
        insert (bullet, 0);
    rtfStr ("\\tab ");
    first_indent = 0;
}

static void
nrBU_func (int i, char *bullet)
{
    insertPara (PARA_TERM);
    /* Compute the new bullet offset */
    if ((bullet == NULL) || (strlen (bullet) <= 1))
        first_indent = -((FULL_INDENT * 3)/4);
    else
        first_indent = -FULL_INDENT;
    while (--i >= 0)
        insertPara (PARA_BLANK);
    insert ("",0);                      /* Force paragraph out */
    if (bullet == NULL)
        rtfStr ("{\\f1\\b\\fs24\\'b7}");    /* A bullet - believe it or not !! */
    else
        insert (bullet, 0);
    rtfStr ("\\tab ");
    first_indent = 0;
#if 0
    insertPara (PARA_TERM);
    while (--i >= 0)
        insertPara (PARA_BLANK);
    insert ("",0);                      /* Force paragraph out */
    if (bullet == NULL)
        rtfStr ("{\\f1\\b\\fs24\\'b7}");    /* A bullet - believe it or not !! */
    else
        insert (bullet, 0);
    rtfStr ("\\tab ");
#endif
}

static void
nrBE_func (int i)
{
    insertPara (PARA_TERM);
    indent -= FULL_INDENT;
    first_indent = 0;
    while (--i >= 0)
        insertPara (PARA_BLANK);
}

static void
nrCS_func (int i)
{
    insertPara (PARA_TERM);
    mode |= PARA_MODE;
    mode &= ~JUST_MODE;
    indent += FULL_INDENT;              /* Add indent for Code start */
    sub_indent = 0;                     /* Clear the sub-indent */

    font_type = rtfFont->cs_font;
    font_size = rtfFont->cs_size;

    while (--i >= 0)
        insertPara (PARA_BLANK);
}

static void
nrCE_func (int i)
{
    mode &= ~PARA_MODE;
    mode |= JUST_MODE;
    indent -= FULL_INDENT;              /* Remove the indent */
    font_type = rtfFont->tx_font;
    font_size = rtfFont->tx_size;

    insertPara (PARA_TERM);
    while (--i >= 0)
        insertPara (PARA_BLANK);
    /* font_type = rtfFont->tx_font; */
    /* font_size = rtfFont->tx_size; */
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
nrps_func (int i)
{
    ps_size = ps_size + (i*2);
}

static void
nrvs_func (int i)
{
    vs_size = i*2;
}

static void
nrad_func (void)
{
    mode |= JUST_MODE;
}

static void
nrbr_func (void)
{
    insertPara (PARA_TERM);
}

static void
nrbp_func (void)
{
    /* Do not do pages in rtf. Make new paragraph */
    insertPara (PARA_BLANK);
}

static void nrsp_func (int i)
{
    /* Do not do pages in rtf. Make new paragraph */
    insertPara (PARA_BLANK);
}

static void
nrne_func (int i)
{
    /* Do not do pages in rtf. */
}

static void
nrTP_func (int i)
{
    if (i != 0)                         /* Start of function */
    {
        sub_indent = 0;
        insertPara (PARA_BLANK);
    }
    else                                /* End of function */
    {
        sub_indent = FULL_INDENT;
        insertPara (PARA_TERM);
    }
}

static void
nrIP_func (char *s)
{
    nrTP_func (1);
    insert (s, 0);
    nrTP_func (0);
}

static void
nrLP_func (void)
{
    insertPara (PARA_BLANK);
    sub_indent = 0;
}

static void
nrBP_func (char *s)
{
    nrTP_func (1);
    insert (s, BOLD_MODE);
    nrTP_func (0);
}

static void
nrso_func (char *filename)
{
    ;
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
    if (mode & PARA_MODE) {
        if (s == NULL)
            insertPara (PARA_BLANK);
        else {
            insert (s, mode);
            insertPara (PARA_TERM);
        }
    }
    else if (s != NULL)
        insert (s, mode);
    else
#if  1
        insertPara (PARA_BLANK);
#else
    nrPP_func();
#endif
}

static void
nrXI_func (char *name, char *id, char *desc, char *comp)
{
    if (wordMode != 0)
        return;
    if (id == NULL)
        id = sectionId;
#ifdef THREF
    if (id != NULL)
    {
        if ((strcmp (name, sectionName) != 0) || (strcmp (id, sectionId) != 0))
#endif
            rtfStr ("#{\\footnote %s}", nrMakeXref (name, id));
#ifdef THREF
    }
#endif
}

static void
nrXJ_func (char *name, char *id, char *desc, char *comp)
{
    if (wordMode != 0)
        return;
    if (id == NULL)
        id = sectionId;
#ifdef THREF
    if (id != NULL)
    {
        if ((strcmp (name, sectionName) != 0) || (strcmp (id, sectionId) != 0))
#endif
            rtfStr ("\\line #{\\footnote %s}", nrMakeXref (name, id));
#ifdef THREF
    }
#endif
}

static void
nrStart (char *fname, int flag)
{
    /* Set up the conversion */

    para_mode = 0;
    para_clean = 0;
    mode = 0;
    prevMode = 0;
}

static void
nrStartInc (char *fname, int *imode)
{
    /* Set up the conversion */

    para_mode = 0;
    para_clean = 0;
    prevMode = 0;
    mode = 0;
    *imode = 1;                         /* Disable processing of includes */
}

static void
nrEnd (char *fname, int imode, int flag)
{
    if (imode == 0)
        insertPara (PARA_CLOSE);     /* Flush out the buffer */
}

void
droff_init (void)
{
    int     i;

    for (i = 0; rtfHeader [i] != NULL; i++) {
        rtfStr (rtfHeader[i]);
        rtfEol ();
    }
}

void
droff_term (void)
{
    rtfStr (rtfFooter);
    rtfEol ();
}

void quit (int signum)
{
    exit (1);
}

static void
usage (void)
{
    fprintf (stdout, "%s : Usage %s [Options] <files>\n", progname, progname);
    fprintf (stdout, "\n");
    fprintf (stdout, "-?/h      : Help - this page\n");
    fprintf (stdout, "-c <name> : Copyright <name>\n");
    fprintf (stdout, "-E <file> : Log errors to <file> (append)\n");
    fprintf (stdout, "-e <file> : Log errors to <file> (re-write)\n");
    fprintf (stdout, "-f        : Use windows help page font (Arial) [Default].\n");
    fprintf (stdout, "-H <home> : Home page name. (default is 'home')\n");
    fprintf (stdout, "-I        : Version Information\n");
    fprintf (stdout, "-i <num>  : Local section number.\n");
    fprintf (stdout, "-l <lib>  : Library component\n");
    fprintf (stdout, "-L <path> : Library search path\n");
    fprintf (stdout, "-m        : Use man page font (Roman).\n");
    fprintf (stdout, "-M <mod>  : Module name\n");
    fprintf (stdout, "-n <name> : Local section name.\n");
    fprintf (stdout, "-o <file> : Output to <file> default is <files>.out\n");
    fprintf (stdout, "-p <file> : Name of the logo picture (image)\n");
    fprintf (stdout, "-q        : Quiet\n");
    fprintf (stdout, "-s        : Output to standard out.\n");
    fprintf (stdout, "-t <name:link:section>\n");
    fprintf (stdout, "-t arg    : Title to appear at top of page\n");
    fprintf (stdout, "-w        : MS Word output mode (no links). Man page font is enabled.\n");
    fprintf (stdout, "-x        : Resolve all references of module\n");
    exit (1);
}

static void rtfInitialise (void)
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
    nrInstall (funcTab, Id_func, nrId_func);
    nrInstall (funcTab, Im_func, nrIm_func);
    nrInstall (funcTab, Kw_func, nrKw_func);

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
    nrInstall (funcTab, CS_func, nrCS_func);
    nrInstall (funcTab, CE_func, nrCE_func);
    nrInstall (funcTab, ft_func, nrft_func);

    /* Bullets */
    nrInstall (funcTab, BS_func, nrBS_func);
    nrInstall (funcTab, BU_func, nrBU_func);
    nrInstall (funcTab, BE_func, nrBE_func);

    /* Indentation */
    nrInstall (funcTab, RS_func, nrRS_func);
    nrInstall (funcTab, RE_func, nrRE_func);

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
    nrInstall (funcTab, ne_func, nrne_func);

    /* Files */
    nrInstall (funcTab, so_func, nrso_func);
    nrInstall (funcTab, startFile_func, nrStart);
    nrInstall (funcTab, includeFile_func, nrStartInc);
    nrInstall (funcTab, endFile_func, nrEnd);

    /* Text */
    nrInstall (funcTab, textline_func, nrTextline);

    nrInstallFunctionTable (&funcTab);
}

int
main (int argc, char *argv [])
{
    char    *oname = NULL;
    char    *mname = NULL;
    int     ecount = 0;                 /* Error count */
    int     wcount = 0;                 /* Warn count */
    nrFILE  *nrfp;
    int     c;
    int     i;

    uErrorSet (0, &ecount);             /* Max Num entries + counter */
    uWarnSet (&wcount);                 /* Warning count */
    uUtilitySet (MODULE_NAME);          /* Set name of program */
    uVerboseSet (0);                    /* Verbose setting */

    progname = MODULE_NAME;
    rtfFont = &winFont;

    while (1) {
        c = getopt (argc, argv, "a:c:e:E:fH:i:Il:L:mM:n:o:p:qst:wx");
        if (c == EOF)
            break;
        switch (c) {
        case 'c':
            copyrightName = optarg;
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
        case 'a':
            oname = optarg;
            break;
        case 'H':
            nrHomeSet (optarg);
            break;
        case 'M':
            mname = optarg;
            if (nrLibSetModule (mname) != 0)
                exit (1);
            break;
        case 'L':
            nrLibSetPath (optarg);
            break;
        case 'l':
            nrLibLoad (optarg);
            break;
        case 'm':
            rtfFont = &manFont;
            break;
        case 'f':
            rtfFont = &winFont;
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
        case 'w':
            wordMode = 1;
            rtfFont = &manFont;
            break;
        case 'I':
            fprintf (stdout, "%s. Version %s. %s. %s.\n",
                     progname, MODULE_VERSION, __DATE__, nroffVersion);
            exit (0);
            break;
        case 't':
            nrTitleSet (optarg);
            break;
        case 'x':
            compiling = 1;
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

    rtfInitialise();
    argv = getfiles (&argc, argv, optind);

    if ((oname == NULL) && (mname != NULL))
        oname = strdup (makeFilename (NULL, NULL, mname, "rtf"));

    if (argc > 1) {
        if (oname != NULL) {
            if ((fpr = fopen (oname, "wb")) == NULL)
                uFatal ("Cannot open file [%s]\n", oname);
            uVerbose (0, "Output in file [%s].\n", oname);
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
        if (quietMode == 0) {
            uVerbose (0, "Processing file [%s]\n", nrfp->fileName);
            fflush (stdout);
        }
#endif

        /* Break up the file name and add an extension */
        if (oname == NULL && fpr == NULL)
        {
            char *drive;
            char *path;
            char *base;
            char *name;

            if (splitFilename (nrfp->fileName, &drive, &path, &base, NULL) != 0)
                uFatal ("Cannot decompose filename [%s]\n", nrfp->fileName);
            if ((name = makeFilename (drive, path, base, "rtf")) == NULL)
                uFatal ("Cannot compose filename from split [%s]\n",
                        nrfp->fileName);
            if ((fpr = fopen (name, "wb")) == NULL)
                uFatal ("Cannot open file %s\n", name);
            droff_init ();              /* Start up if in sections */
        }

        nroff (nrfp, ((compiling == 0) ? NROFF_MODE_DEFAULT :
                      NROFF_MODE_COMPILE));

        fflush (fpr);
        if ((oname == NULL) && (fpr != stdout)) {
            droff_term ();
            rtfEof ();
            fclose (fpr);
            fpr = NULL;
        }
    }
    if (fpr != NULL) {
        droff_term ();
        rtfEof ();
        fclose (fpr);
    }

    uCloseErrorChannel ();
    return (ecount);
}
