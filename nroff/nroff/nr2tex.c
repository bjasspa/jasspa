/* -*- C -*- ****************************************************************
 *
 *                       Copyright 2002-2004 Jon Green.
 *                            All Rights Reserved
 *
 *
 *  System        :
 *  Module        :
 *  Object Name   : $RCSfile: nr2tex.c,v $
 *  Revision      : $Revision: 1.6 $
 *  Date          : $Date: 2004-02-07 19:29:49 $
 *  Author        : $Author: jon $
 *  Created By    : Jon Green
 *  Created       : Thu Mar 7 20:45:45 2002
 *  Last Modified : <260223.1810>
 *
 *  Description
 *
 *  Notes
 *
 *  History
 *
 *  1.1.0  JG 2026-02-23 Simplified LaTeX output to reduce constructs.
 *                       Introduced header/trailer file to define LaTeX
 *  1.0.0a JG 2004-02-07 Ported to HP-UX
 *  1.0.0a JG 2004-01-03 Ported to Sun Solaris 9
 *  1.0.0  JG 2002-03-07 Original
 *
 ****************************************************************************
 *
 *  Copyright (c) 2002-2004 Jon Green.
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

#if ((defined _HPUX) || (defined _LINUX) || (defined _SUNOS))
#include <unistd.h>
#else
#include <getopt.h>
#endif
#include <utils.h>

#include "nroff.h"

#define MODULE_VERSION  "1.1.0"
#define MODULE_NAME     "nr2tex"

#define FULL_INDENT 1                   /* Full indent distance. */
#define HALF_INDENT 1                   /* Half indent distance. */

#define NORMAL_FONT    0x000000         /* Normal font - Times */
#define ROMAN_FONT     NORMAL_FONT      /* Roman font - Times */
#define BOLD_FONT      0x000100         /* Bold Normal font */
#define ITALIC_FONT    0x000200         /* Italic Normal font */
#define MONO_FONT      0x000400         /* Mono font - Courier */
#define FONT_TYPE_MASK (MONO_FONT|BOLD_FONT|ITALIC_FONT)
#define LARGE_FONT     0x000800         /* Large size font */
#define SMALL_FONT     0x001000         /* Small size font */
#define FONT_SIZE_MASK (LARGE_FONT|SMALL_FONT)
#define FONT_MASK      (FONT_SIZE_MASK|FONT_TYPE_MASK)

#define MODE_CHAR      0x00007f         /* Character mask */
#define MODE_MASK      0xffff00         /* Mode mask */
#define NORMAL_MODE    0x000000         /* Normal mode */
#define RAGGED_MODE    0x002000         /* Ragged right mode */
#define CODE_MODE      0x010000         /* Inside a code block .CS/.CE */
#define IPLP_MODE      0x020000         /* Inside a IP/TP block .IP/.LP */
#define IPTP_MODE      0x040000         /* Received .TP, waiting for text */
#define BSBE_MODE      0x080000         /* Inside a BS/BE block .BS/.BU/.BE */
#define NOFILL_MODE    0x100000         /* Disable line filling */
#define PARA_MASK      (CODE_MODE|IPLP_MODE|BSBE_MODE)

static char *UNDEFINED = "UNDEFINED";
static int  rsreindent = 0;             /* The current .RS/.RE indent */
static int  quietMode = 0;
static int  currmode = 0;               /* The current mode */
static int  prevmode = 0;               /* The previous mode */
static char *imModule = NULL;           /* Current .Im module */
static char *imComponent = NULL;        /* Current .Im component */
static char *progname = MODULE_NAME;
static char *moduleName = NULL;         /* Name of the module */
static char *sectionName = NULL;        /* Name from .TH */
static char *sectionId = NULL;          /* Section number from .TH */
static char *sectionDesc = NULL;        /* Section description text from .TH */
static int  sectionLevel = 0;           /* Section number from .TH */
//static char *sectionComponent = NULL;
static char *sectionDate = NULL;        /* Date on which last modified */
static char *copyrightName = NULL;
static char *latexPrefix = NULL;        /* The leader file to load to start
                                         * the tex output file */
static char *latexPostfix = NULL;       /* The trailer file to load to start
                                         * the tex output file */
static char *fileLaTeXName = NULL;
static char *logoName = "logo.png";     /* Default logo name */
static FILE *fo;
static FILE *fpr = NULL;
static int  year;                       /* The current year */
static int  month;                      /* The current month */
static int  day;                        /* The current day */
static int  multiPage = 0;              /* Document is multi-paged */

/* Forward references */
static void latexFormatStr (char *s);
static void setFontSize (int mode);
static void setFont (int mode);

static void
latexEof (void)
{
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

/* Convert and reformat a ROFF string to LaTeX by re-escaping */
static void
latexFormatStr (char *s)
{
    char c;

    /* Bail quickly if the string is NULL */
    if (s == NULL)
        return;

    /* Iterate through the characters */
    while ((c = *s++) != '\0')
    {
        switch (c) {
            /* Normal font size */
        case FONTN_CHAR:
            if ((currmode & FONT_SIZE_MASK) == NORMAL_FONT)
                uWarn ("Already have normal size fonts enabled.\n");
            else
                setFontSize (NORMAL_FONT);
            break;
            /* Larger font size */
        case FONTL_CHAR:
            if ((currmode & LARGE_FONT) != 0)
                uWarn ("Already have large size fonts enabled.\n");
            else
                setFontSize (LARGE_FONT);
            break;
            /* Smaller font size */
        case FONTS_CHAR:
            if ((currmode & SMALL_FONT) != 0)
                uWarn ("Already have smaller size fonts enabled.\n");
            else
                setFontSize (SMALL_FONT);
            break;
            /* Bold font */
        case BOLD_CHAR:
            if ((currmode & BOLD_FONT) != 0)
                uWarn ("Already have bold mode enabled.\n");
            else
                setFont (BOLD_FONT);
            break;
            /* Mono font */
        case MONO_CHAR:
            if ((currmode & MONO_FONT) != 0)
                uWarn ("Already have mono mode enabled.\n");
            else
                setFont (MONO_FONT);
            break;
            /* Italic font */
        case ITALIC_CHAR:
            if ((currmode & ITALIC_FONT) != 0)
                uWarn ("Already have italic mode enabled.\n");
            else
                setFont (ITALIC_FONT);
            break;
            /* Roman font */
        case ROMAN_CHAR:
            if ((currmode & FONT_TYPE_MASK) == ROMAN_FONT)
                uWarn ("Roman requested and already in Roman.\n");
            else
                setFont (ROMAN_FONT);
            break;
            /* Previous font */
        case PREV_CHAR:
            if ((prevmode & FONT_TYPE_MASK) == (currmode & FONT_TYPE_MASK))
                uWarn ("Previous (%x) and current (%x) font are the same for \\fp.\n",
                       (prevmode & FONT_TYPE_MASK) >> 8,
                       (currmode & FONT_TYPE_MASK) >> 8);
            else
                setFont (prevmode & FONT_TYPE_MASK);
            break;
        case '<':
            latexStr ("{\\textless}");
            break;
        case '>':
            latexStr ("{\\textgreater}");
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
            latexStr ("{\\textasciicircum}");
            break;
        case '_':
            latexStr ("\\_");
            break;
        case '%':
            latexStr ("%s", "\\%");
            break;
        case '#':
            latexStr ("\\#");
            break;
        case '&':
            latexStr ("\\&");
            break;
        case '\\':
            latexStr ("{\\textbackslash}");
            break;
        case ' ':
            if (currmode & NOFILL_MODE)
                latexStr ("~");         /* No-break space in NOFILL mode */
            else
                latexStr ("%c", c);
            break;
        case COPYRIGHT_CHAR:
            latexStr ("{\\textcopyright}");
            break;
        case DEGREE_CHAR:
            latexStr ("{\\textdegree}");
            break;
        case ZSPACE_CHAR:               /* 1/6em space */
            latexStr ("{\\nobreak\\hspace{.16em}}");
            break;
        case MSPACE_CHAR:               /* 1/12em space */
            latexStr ("{\\nobreak\\hspace{.08em}}");
            break;
        case BULLET_CHAR:
            latexStr ("{\\textbullet}");
            break;
        case ESC_CHAR:
            latexStr ("{\\textbackslash}");
            break;
        case USPACE_CHAR:
            latexStr ("~");
            break;
        case TAB_CHAR:
            latexStr ("%c", c);
            uWarn ("Unexpected TAB character %d in stream\n", c);
            break;
        default:
            if ((c < ' ') || (c > 127))
            {
                uWarn ("Unprintable character %d in stream\n", c);
                c = '?';
            }
            latexStr ("%c", c);
            break;
        }
        //if (c != ' ')
        //     space = 0;
    }
}

static void
verbatimFormatStr (char *s)
{
    char c;

    /* Bail quickly if the string is NULL */
    if (s == NULL)
        return;

    /* Iterate through the characters */
    while ((c = *s++) != '\0')
    {
        switch (c) {
        case FONTN_CHAR:                /* Normal font size */
        case FONTL_CHAR:                /* Larger font size */
        case FONTS_CHAR:                /* Smaller font size */
        case BOLD_CHAR:                 /* Bold font */
        case MONO_CHAR:                 /* Mono font */
        case ITALIC_CHAR:               /* Italic font */
        case ROMAN_CHAR:                /* Roman font */
        case PREV_CHAR:                 /* Previous font */
        case ZSPACE_CHAR:               /* 1/6em space */
        case MSPACE_CHAR:               /* 1/12em space */
             /* Skip character */
            break;
        case BULLET_CHAR:
            latexStr ("*");
            break;
        case USPACE_CHAR:
            latexStr (" ");
            break;
        case TAB_CHAR:
            latexStr ("%c", c);
            uWarn ("Unexpected TAB character %d in stream\n", c);
            break;
        default:
            if ((c < ' ') || (c > 127))
            {
                uWarn ("Unprintable character %d in stream\n", c);
                c = '?';
            }
            latexStr ("%c", c);
            break;
        }
    }
}

static void
insert (char *s)
{
    int     length;
    int     i;
    char    buffer [1024];

    if (s == NULL)
        return;
    while (*s)
    {
        int add_space;

        /* Trim the string */
        s = rightTrim(leftTrim (s));
        if (s == NULL)
            break;

        /* Find the next word */
        add_space = 1;
        length = wordLen (s);
        if (length >= sizeof (buffer))
        {
            length = sizeof (buffer) - 1;
            add_space = 0;
        }

        /* Copy the word out */
        for (i = 0; i < length; i++)
            buffer [i] = s[i];
        buffer [i] = '\0';
        s = &s [length];

        /* Output the word */
        latexFormatStr (buffer);
        if (add_space != 0)
            latexFormatStr (" ");
    }
}

/* Write out the font */
static void
setFont (int mode)
{
    /* See if there is anything to do */
    if ((mode & FONT_MASK) != (currmode & FONT_MASK))
    {
        /* Set the previous mode */
        prevmode = (prevmode & ~FONT_TYPE_MASK) | (currmode & FONT_TYPE_MASK);

        /* Turn off any highlighting modes if required. */
        if (((mode & BOLD_FONT) == 0) && ((currmode & BOLD_FONT) != 0)) {
            currmode &= ~BOLD_FONT;
            latexStr ("}");
        }
        if (((mode & ITALIC_FONT) == 0) && ((currmode & ITALIC_FONT) != 0)) {
            currmode &= ~ITALIC_FONT;
            latexStr ("}");
        }
        if (((mode & MONO_FONT) == 0) && ((currmode & MONO_FONT) != 0)) {
            currmode &= ~MONO_FONT;
            latexStr ("}");
        }

        /* Turn on any highlighting modes if required. */
        if (((mode & BOLD_FONT) != 0) && ((currmode & BOLD_FONT) == 0)) {
            currmode |= BOLD_FONT;
            latexStr ("\\textbf{");
        }
        if (((mode & ITALIC_FONT) != 0) && ((currmode & ITALIC_FONT) == 0)) {
            currmode |= ITALIC_FONT;
            latexStr ("\\textit{");
        }
        if (((mode & MONO_FONT) != 0) && ((currmode & MONO_FONT) == 0)) {
            currmode |= MONO_FONT;
            latexStr ("\\texttt{");
        }
    }
}

static void
setFontSize (int mode)
{
    /* See if there is anything to do */
    if ((mode & FONT_SIZE_MASK) == FONT_SIZE_MASK)
        uError ("Illegal font size large+small\n");
    else if ((mode & FONT_SIZE_MASK) != (currmode & FONT_SIZE_MASK))
    {
        /* Process any changes */
        if ((currmode & FONT_SIZE_MASK) == NORMAL_FONT)
        {
            /* Open a large font */
            if ((mode & LARGE_FONT) != 0) {
                currmode |= LARGE_FONT;
                latexStr ("\\textlarger{");
            }
            /* Otherwise open a smaller font */
            else if ((mode & SMALL_FONT) != 0) {
                currmode |= SMALL_FONT;
                latexStr ("\\textsmaller{");
            }
        }
        else
        {
            /* Close down the large font */
            if ((currmode & LARGE_FONT) != 0)
            {
                /* Close the existing block */
                latexStr ("}");
                currmode &= ~LARGE_FONT;
            }

            /* Close down the small font */
            if ((currmode & SMALL_FONT) != 0)
            {
                /* Close the existing block */
                latexStr ("}");
                currmode &= ~SMALL_FONT;
            }
        }
    }
//    else
//        uWarn ("Illegal \\s<d> construct.\n");
}

void
setRaggedRight (int mode)
{
    /* Error checking */
    if ((mode & RAGGED_MODE) != 0)
    {
        /* Turn on ragged right mode */
        if ((currmode & RAGGED_MODE) == 0)
        {
            latexStr ("\\begin{raggedright}");
            latexEol ();
            currmode |= RAGGED_MODE;
        }
    }
    else
    {
        /* Turn off ragged right mode */
        if ((currmode & RAGGED_MODE) != 0)
        {
            latexStr ("\\end{raggedright}");
            latexEol ();
            currmode &= ~RAGGED_MODE;
        }
    }
}

static void
setPara (int mode, char *s)
{
    /* See if there is a mode change */
    if ((currmode & PARA_MASK) != (mode & PARA_MASK))
    {
        /* Process a .CE */
        if (((currmode & CODE_MODE) != 0) && ((mode & CODE_MODE) == 0))
        {
            latexStr ("\\end{verbatim}");
            latexEol ();
            latexStr ("\\end{scriptsize}");
            latexEol ();
            currmode &= ~CODE_MODE;
        }

        /* Proecss a .LP */
        if (((currmode & IPLP_MODE) != 0) && ((mode & IPLP_MODE) == 0))
        {
            /* Normalise fonts */
            setFont (NORMAL_FONT);
            setFontSize (NORMAL_FONT);

            /* End section */
            latexStr ("\\end{Description}%% .LP");
            latexEol ();
            currmode &= ~IPLP_MODE;
        }

        /* Proecss a .LP */
        if (((currmode & BSBE_MODE) != 0) && ((mode & BSBE_MODE) == 0))
        {
            /* Normalise fonts */
            setFont (NORMAL_FONT);
            setFontSize (NORMAL_FONT);

            /* End section */
            latexStr ("\\end{Description}%% .BE");
            latexEol ();
            currmode &= ~BSBE_MODE;
        }
    }

    /* Process a opening .IP/.TP construct */
    if ((mode & IPLP_MODE) != 0)
    {
        /* Normalise fonts */
        setFont (NORMAL_FONT);
        setFontSize (NORMAL_FONT);

        /* This is a .IP/.TP structure, see if we are starting or have a new
         * list. */
        if ((currmode & IPLP_MODE) == 0)
        {
            /* Insert the start of the construct */
            latexStr ("\\begin{Description}%% .IP");
            latexEol ();
            currmode |= IPLP_MODE;
        }
        /* Add a new item to the list */
        latexStr ("\\item[{");
        setFont (mode & FONT_MASK);     /* Apply any font mode */
        latexFormatStr (s);
        setFont (NORMAL_FONT);
        latexStr ("}]%% .IP");
        latexEol ();
    }

    /* Process a opening .BS/.BU construct */
    if ((mode & BSBE_MODE) != 0)
    {
        /* Normalise fonts */
        setFont (NORMAL_FONT);
        setFontSize (NORMAL_FONT);

        /* This is a .IP/.TP structure, see if we are starting or have a new
         * list. */
        if ((currmode & BSBE_MODE) == 0)
        {
            /* Insert the start of the construct */
            latexStr ("\\begin{Description}%% .BS");
            latexEol ();
            currmode |= BSBE_MODE;
        }

        /* Add a new item to the list with the bullet character */
        latexStr ("\\item[\\hspace{.5\\MANindent}{");
        setFont (mode & FONT_MASK);      /* Apply any font mode */
        if (s != NULL)
            latexFormatStr (s);
        else
            latexStr ("\\textbullet");
        setFont (NORMAL_FONT);
        latexStr ("}]%% .BU");
        latexEol ();
    }

    /* Process a opening .CS construct */
    if (((mode & CODE_MODE) != 0) && ((currmode & CODE_MODE) == 0))
    {
        /* Normalise fonts */
        setFont (NORMAL_FONT);
        setFontSize (NORMAL_FONT);

        /* Open the section */
        latexStr ("\\begin{scriptsize}");
        latexEol ();
        latexStr ("\\begin{verbatim}");
        latexEol ();
        currmode |= CODE_MODE;
    }
}

/* Indent paragraph.
 *
 * delta the indentation depth required
 * -ve - Reduce the indent
 * +ve - Increase the indent
 * 0   - Remove all indents
 */
static void
indentPara (int delta)
{
    /* If the delta is zero then remove all indents */
    if ((delta == 0) && (rsreindent > 0))
        delta = -rsreindent;

    /* Apply a new indent */
    while (delta > 0)
    {
        /* Insert the start of the construct */
        latexStr ("\\begin{Description}%% .RS");
        latexEol ();
        latexStr ("\\item[]%% .RS");
        latexEol ();
        rsreindent++;
        delta--;
    }

    /* Remove existing indent */
    while (delta < 0)
    {
        latexStr ("\\end{Description}%% .RE");
        latexEol ();
        rsreindent--;
        delta++;
    }
}

static void
setAll (int mode)
{
    setFont (mode & FONT_TYPE_MASK);
    setFontSize (mode & FONT_SIZE_MASK);
    setRaggedRight (mode & RAGGED_MODE);
    setPara (mode & PARA_MASK, NULL);
    indentPara (0);
}

/*
 * FH - End of the file
 */
static void
nrFH_func (void)
{
    /* Close anything that was open and output comment to effect closing */
    setAll (NORMAL_MODE);
    latexStr ("%% .FH");
    latexEol ();
}

static void
nrIm_func (char *module, char *component)
{
    /* Free off the old module and assign the new */
    imModule = bufFree (imModule);
    if (module != NULL)
        imModule = bufStr (NULL, module);

    /* Free off the old component and assign the new */
    imComponent = bufFree (imComponent);
    if (component != NULL)
    {
        if ((imComponent = bufNStr (NULL, component)) != NULL)
        {
            imComponent = nrImGetFirst (imComponent);
            if (islower ((int)(imComponent[0])))
                imComponent[0] = toupper (imComponent[0]);
        }
    }

    if (module != NULL)
    {
        latexStr ("%% .Im %s", module);
        if (component != NULL)
            latexStr (" %s", component);
        latexEol ();
    }
}

static void
nrId_func (char *date)
{
    bufFree (sectionDate);
    sectionDate = bufNStr (NULL, date);

    if (date != NULL)
    {
        latexStr ("%% Date: %s", date);
        latexEol ();
    }
}

static void
nrTH_func (char *id, char *num, char *date, char *company, char *title)
{
    /* Free off the old section Name and number and assign the new values. */
    bufFree (sectionName);
    bufFree (sectionId);
    bufFree (sectionDesc);
    sectionName = bufStr (NULL, id);
    sectionId = bufNStr (NULL, num);
    sectionLevel = nrGetLevel (sectionName, sectionId);
    sectionDesc = bufNStr (NULL, title);

    /* Insert a page break if a page has already been rendered */
    if (multiPage == 0)
        multiPage = 1;
    else
    {
        latexStr ("\\newpage");
        latexEol ();
    }

    /* Push reference string into TeX file as a comment */
    latexStr ("\\THsection*{");
    if (sectionDesc != NULL)
        latexFormatStr (sectionDesc);
    else
    {
        latexFormatStr (sectionName);
        if (sectionId != NULL)
        {
            latexStr("(");
            latexFormatStr (sectionId);
            latexStr(")");
        }
    }
    latexStr("}");
    latexEol();                          /* Make pretty */
    latexStr ("%% %s - Version %s - %s - %s - Jon Green (2002)",
             MODULE_NAME, MODULE_VERSION, nroffVersion, __DATE__);
    latexEol ();                         /* Make pretty */

    /* Add a ToC reference fopr the page */
    latexStr ("\\THreference{");
    if (sectionDesc != NULL)
        latexFormatStr (sectionDesc);
    else
    {
        latexFormatStr (sectionName);
        if (sectionId != NULL)
        {
            latexStr ("(");
            latexFormatStr (sectionId);
            latexStr (")");
        }
    }
    latexStr ("}{");
    latexFormatStr (nrMakeXref (sectionName,sectionId));
    latexStr ("}");
    latexEol();                          /* Make pretty */

    /* Add a ToC reference fopr the page */
    //latexStr ("\\ifpdfoutput");
    //latexEol();                          /* Make pretty */
    //latexStr ("\\fi");
    //latexEol();                          /* Make pretty */

    /* Reset the modes for the new page */
    currmode = 0;
    prevmode = 0;
    rsreindent = 0;
}

static void
nrNH_func (char *id, char *num, char *title, char *xref)
{
    /* Close the previous section */
    nrFH_func ();

    /* Set up the new section */
    bufFree (sectionName);
    bufFree (sectionId);
    bufFree (sectionDesc);
    sectionName = bufStr (NULL, id);
    sectionId = bufNStr (NULL, num);
    sectionDesc = bufNStr (NULL, title);

    /* Insert a page break if a page has already been rendered */
    if (multiPage == 0)
        multiPage = 1;
    else
    {
        latexStr ("\\newpage");
        latexEol ();
    }

    /* Push reference string into TeX file as a comment */
    latexStr ("\\NHsection*{");
    if (sectionDesc != NULL)
        latexFormatStr(sectionDesc);
    else
    {
        latexFormatStr (sectionName);
        if (sectionId != NULL)
        {
            latexStr("(");
            latexFormatStr(sectionId);
            latexStr(")");
        }
    }
    latexStr("}");
    latexEol();                          /* Make pretty */

    latexStr ("\\NHreference{");
    if (sectionDesc != NULL)
        latexFormatStr(sectionDesc);
    else
    {
        latexFormatStr (sectionName);
        if (sectionId != NULL)
        {
            latexStr ("(");
            latexFormatStr (sectionId);
            latexStr (")");
        }
    }
    latexStr ("}{");
    latexFormatStr (nrMakeXref (sectionName,sectionId));
    latexStr ("}");
    latexEol();                          /* Make pretty */

    /* Reset the modes for the new page */
    currmode = 0;
    prevmode = 0;
    rsreindent = 0;
}

static void
nrGH_func (char *title)
{

    if (title == NULL)
        return;

    uWarn (".GH Not implemented\n");
}

static void
nrSH_func (char *s)
{
    /* Close anything that was open */
    setAll (NORMAL_MODE);

    /* Heading is a section in TeX */
    if (s == NULL)
        s = UNDEFINED;
    latexStr ("\\SHsection*{");
    latexFormatStr (s);
    latexStr ("}");
    latexEol ();                        /* Make pretty */
}

static void
nrSS_func (char *s)
{
    /* Close anything that was open */
    setAll (NORMAL_MODE);

    /* Add heading string to introduce title */
    latexStr ("\\SSsection*{");
    latexFormatStr (s);
    latexStr ("}");
    latexEol ();                        /* Make pretty */
}

static void
nrPP_func (void)
{
    latexEol ();                        /* Make pretty */
    latexEol ();                        /* 2nd blank line makes a paragraph */
}

static void
nrHl_func (char *text, char *name, char *section,
           char *concat, char *module, char *file, int type)
{
    /* Shut down font formatting */
    setFont (NORMAL_FONT);
    setFontSize (NORMAL_FONT);

    /* Special to process a .TP description */
    if ((currmode & IPTP_MODE) != 0)
    {
        if ((currmode & IPLP_MODE) == 0)
        {
            setPara (NORMAL_MODE, NULL);
            /* Insert the start of the construct */
            latexStr ("\\begin{Description}%% .TP");
            latexEol ();
            currmode |= IPLP_MODE;
        }
        /* A .TP was previously received this line is the label */
        /* Add a new item to the list */
        latexStr ("\\item[{");
    }

    /* Process the name */
    if (name != NULL) {
        latexStr ("\\hyperlink{%s}{", nrMakeXref (name, section));

        /* Shove in the name to display to user */
        if (text != NULL)
            latexFormatStr (text);
        else
        {
            latexFormatStr (name);
            if (section != NULL)
                latexFormatStr (section);
        }
        latexStr ("}");
    }

    /* Add any optional concatinated string and clean up and push out a line
     * feed */
    latexFormatStr (concat);

    if ((currmode & IPTP_MODE) != 0)
    {
        currmode &= ~IPTP_MODE;         /* Knock off IPTP mode */
        latexStr ("}]%% .TP/.Hl");
        latexEol ();
    }
    else
    {
        latexStr (" %% .Hl");          /* Keep a space before start of comment */
        latexEol ();
    }
}

static void
nrHt_func (char *name, char *section, char *concat,
           char *module, char *file, int type)
{
    /* Special to process a .TP description */
    if ((currmode & IPTP_MODE) != 0)
    {
        if ((currmode & IPLP_MODE) == 0)
        {
            setPara (NORMAL_MODE, NULL);
            /* Insert the start of the construct */
            latexStr ("\\begin{Description}%% .IP");
            latexEol ();
            currmode |= IPLP_MODE;
        }
        /* A .TP was previously received this line is the label */
        /* Add a new item to the list */
        latexStr ("\\item[{");
    }

    /* Process the name */
    if (name != NULL)
    {
        // latexEol ();                     /* Make pretty */
        /* Insert the hypertext link */
        latexStr ("\\hyperlink{%s}{", nrMakeXref (name, section));

        /* Add the name to display to the user emboldended */
        setFont (BOLD_FONT);
        latexFormatStr (name);
        setFont (NORMAL_FONT);
        /* Add the section number in brackets i.e. (1) */
        if (section != NULL) {
            latexFormatStr ("(");
            latexFormatStr (section);
            latexFormatStr (")");
        }
        /* Close off the hyperlink command */
        latexStr ("}");
    }

    /* Add any optional concatinated string and clean up and push out a line
     * feed */
    latexFormatStr (concat);

    if ((currmode & IPTP_MODE) != 0)
    {
        currmode &= ~IPTP_MODE;         /* Knock off IPTP mode */
        latexStr ("}]%% .TP/.Ht");
        latexEol ();
    }
    else
    {
        latexStr (" %% .Ht");           /* Keep a space before start of comment */
        latexEol ();
    }
}

static void
nrHr_func (char *name, char *section, char *concat)
{
    /* Normal fonts */
    setFont (NORMAL_FONT);
    setFontSize (NORMAL_FONT);

    /* Special to process a .TP description */
    if ((currmode & IPTP_MODE) != 0)
    {
        if ((currmode & IPLP_MODE) == 0)
        {
            setPara (NORMAL_MODE, NULL);
            /* Insert the start of the construct */
            latexStr ("\\begin{Description}%% .IP");
            latexEol ();
            currmode |= IPLP_MODE;
        }
        /* A .TP was previously received this line is the label */
        /* Add a new item to the list */
        latexStr ("\\item[{");
    }

    /* Render the name in bold */
    if (name != NULL) {
        setFont (BOLD_FONT);
        latexFormatStr (name);
        setFont (NORMAL_FONT);
    }

    /* Render the section number */
    if (section != NULL) {
        latexFormatStr ("(");
        latexFormatStr (section);
        latexFormatStr (")");
    }

    /* Add any optional concatinated string and clean up and push out a line
     * feed */
    latexFormatStr (concat);
    if ((currmode & IPTP_MODE) != 0)
    {
        currmode &= ~IPTP_MODE;         /* Knock off IPTP mode */
        latexStr ("}]%% .TP/.Hr");
        latexEol ();
    }
    else
    {
        latexStr (" %% .Hr");           /* Keep a space before start of comment */
        latexEol ();
    }
}

/* Hyperlink */
static void
nrHg_func (char *name, char *concat)
{
    /* Normal font */
    setFont (NORMAL_FONT);
    setFontSize (NORMAL_FONT);

    if (name != NULL) {
        /* Add a \\href */
        latexStr ("\\href{%s}{", name);

        /* Add the visible text string */
        latexFormatStr (name);
        latexStr ("}");
    }
    latexFormatStr (concat);
    latexStr (" %% .Hg");
    latexEol ();
}

static void
nrGr_func (char *align, char *fname)
{
    latexStr ("\\includegraphics[keepaspectratio]{%s}", fname);
    latexStr ("%% .Gr");
    latexEol ();
}

/*
 * Start of bulleted list.
 * paraBefore - Insert a paragraph before when not zero
 * spacing - 0 = Compact, 1 = normal
 * bullet - The bullet string to render, NULL is the default bullet.
 */
static void
nrBS_func (int paraBefore, int spacing, char *bullet)
{
    /* Error checking */
    if ((currmode & BSBE_MODE) != 0)
        uError ("Unexpected .BS a .BS section is already active\n");

    /* Insert extra space beforce construct */
    if (paraBefore != 0)
        ;                               /* Ignore for TeX */
    if (spacing != 0)
        ;                               /* Ignore for TeX */

    /* Start the bullet list */
    setPara (BSBE_MODE, bullet);
}

/* Bullet item
 * leadingPara - Previous paragraph break when set.
 * bullet - The bullet character to render,
 */
static void
nrBU_func (int leadingPara, char *bullet)
{
    /* Error checking */
    if ((currmode & PARA_MASK) != BSBE_MODE)
        uError ("Unexpected .BU, no .BS list is open\n");

    /* Insert extra space beforce construct */
    if (leadingPara != 0)
        ;                               /* Ignore for TeX */

    /* Set the bullet list */
    setPara (BSBE_MODE, bullet);
}

static void
nrBE_func (int trailingPara)
{
    /* Error checking */
    if ((currmode & BSBE_MODE) == 0)
        uError ("Unexpected .BE, no .BS list is open\n");

    /* Disable bullet list */
    setPara (NORMAL_MODE, NULL);

    /* Ignore the traling paragrph */
    if (trailingPara != 0)
        ; /* Force a blank paragraph, does not apply to TeX */
}

static void
nrCS_func (int leadingPara)
{
    /* Error checking */
    if ((currmode & CODE_MODE) != 0)
        uError ("Unexpected .CS, a .CS section is already active\n");

    /* Force a blank paragraph */
    if (leadingPara != 0)
        ; /* Ingored in TeX */

    /* Enable a code section */
    setPara (CODE_MODE, NULL);
}

static void
nrCE_func (int trailingPara)
{
    /* Error checking */
    if ((currmode & CODE_MODE) == 0)
        uError ("Unexpected .CE, a .CS section is not active\n");

    /* Disable code section */
    setPara (NORMAL_MODE, NULL);

    /* Ignore the traling paragrph */
    if (trailingPara != 0)
        ; /* Force a blank paragraph, does not apply to TeX */
}

static void
nrRS_func (void)
{
    /* Close any bullet or IP block */
    setPara (NORMAL_MODE, NULL);
    /* Insert the start of the construct */
    indentPara (1);
}

static void
nrRE_func (void)
{
    /* Report error if no indent outstanding, otherwise remove indent */
    if (rsreindent <= 0)
        uError (".RE requested and no .RS construct\n");
    else
        indentPara (-1);
}

/* Turn on ragged right mode */
static void
nrna_func (void)
{
    /* Error checking */
    if ((currmode & RAGGED_MODE) != 0)
        uError (".na is already enabled\n");

    /* Set the mode */
    setRaggedRight (RAGGED_MODE);
    latexStr ("%% .na");
    latexEol ();
}

static void
nrad_func (void)
{
    /* Error checking */
    if ((currmode & RAGGED_MODE) == 0)
        uError (".ad is already enabled\n");

    /* Set the mode */
    setRaggedRight (NORMAL_MODE);
    latexStr ("%% .ad");
    latexEol ();
}

/* Turn off filling process literal lines */
static void
nrnf_func (void)
{
    /* Error checking */
    if ((currmode & NOFILL_MODE) != 0)
        uError (".nf and already in a no fill mode.\n");

    /* Enable no fill mode */
    currmode |= NOFILL_MODE;
    latexStr ("%% .nf");
    latexEol ();
}

static void
nrfi_func (void)
{
    /* Error checking */
    if ((currmode & NOFILL_MODE) == 0)
        uError (".fi and already in a fill mode.\n");

    /* Enable no fill mode */
    currmode &= ~NOFILL_MODE;
    latexStr ("%% .fi");
    latexEol ();
}

static void
nrps_func (int i)
{
    /* Change the font size */
    uWarn (".ps %d - not supported, use \\s-1 ... \\s0 of \\s+1 ... \\s0 instead\n");
    latexStr ("%% .ps %d", i);
    latexEol ();
}

static void
nrvs_func (int i)
{
    uWarn (".vs %d - not supported\n");
    latexStr ("%% .vs %d", i);
    latexEol ();
}

static void
nrbr_func (void)
{
    latexStr (" \\newline");
    latexStr ("%% .br");
    latexEol ();
}

static void
nrbp_func (void)
{
    /* New page */
    latexStr ("\\newpage%% .bp");
    latexEol ();
}

static void nrsp_func (int i)
{
    latexStr ("%% .sp");
    latexEol ();                        /* Make pretty */
    latexEol ();                        /* 2nd blank line makes a paragraph */
}

static void
nrTP_func (int first)
{
    /* Only process the first call back when first is set. */
    if (first != 0)
    {
        /* Set the IPTP_MODE flag */
        if ((currmode & IPTP_MODE) != 0)
        {
            /* An existing .TP is open awaiting a description, given a new .TP
             * then there is no description so make one. */
            currmode &= ~IPTP_MODE;         /* Knock off IPTP mode */
            setPara (IPLP_MODE, " ");       /* Use an IP <string> construct */
        }

        /* Record that we have seen a .TP linem, the next line is the label */
        currmode |= IPTP_MODE;
        latexStr ("%% .TP %d", first);
        latexEol ();                        /* Make pretty */
    }
    else
    {
        /* Ignore the 2nd Call back, this is the start of the text. */
        latexStr ("%% .TP %d", first);
        latexEol ();                        /* Make pretty */
    }
}

static void
nrIP_func (char *s)
{
    setPara (IPLP_MODE|NORMAL_FONT, s);
    latexStr ("%% .IP");
    latexEol ();                        /* Make pretty */
}

static void
nrLP_func (void)
{
    setPara (NORMAL_MODE, NULL);
    latexStr ("%% .LP");
    latexEol ();                        /* Make pretty */
}

static void
nrBP_func (char *s)
{
    setPara (IPLP_MODE|BOLD_FONT, s);
    latexStr ("%% .BP");
    latexEol ();                        /* Make pretty */
}

static void
nrft_func (int font)
{
    switch (font)
    {
    case FT_B:
        if ((currmode & BOLD_FONT) != 0)
            uWarn (".ft Already have bold mode enabled.\n");
        else
            setFont (BOLD_FONT);
        break;
    case FT_R:
        if ((currmode & FONT_TYPE_MASK) == ROMAN_FONT)
            uWarn (".ft Roman requested and already in Roman.\n");
        else
            setFont (ROMAN_FONT);
        break;
    case FT_I:
        if ((currmode & ITALIC_FONT) != 0)
            uWarn (".ft Already have italic mode enabled.\n");
        else
            setFont (ITALIC_FONT);
        break;
    case FT_C:
        if ((currmode & ITALIC_FONT) != 0)
            uWarn (".ft already have italic mode enabled.\n");
        else
            setFont (ITALIC_FONT);
        break;
    default:
        uError (".ft Unknown font %d requested.\n");
        break;
    }
    latexStr ("%% .ft %d", font);
    latexEol ();                        /* Make pretty */
}

static void
nrTextline (char *s)
{
    uVerbose (2, "nrTextline [%s]\n", s);

    /* Test for a verbatim code block */
    if ((currmode & CODE_MODE) != 0)
    {
        /* This is a code line so push the line out verbatim */
        verbatimFormatStr (s);
        latexEol();
    }
    /* Test for a .TP label line */
    else if ((currmode & IPTP_MODE) != 0)
    {
        /* A .TP was previously received this line is the label */
        currmode &= ~IPTP_MODE;         /* Knock off IPTP mode */
        setPara (IPLP_MODE, s);         /* Use an IP <string> construct */
    }
    else
    {
        /* Blank line, this must be a new paragraph */
        if ((s == NULL) || (*s == '\0'))
            latexEol();
        else if ((currmode & NOFILL_MODE) != 0)
        {
            /* No fill mode */
            latexFormatStr (s);
            latexStr ("\\newline");
            latexEol ();
        }
        else
        {
            /* Filled mode */
            insert (s);
            latexEol ();
        }
    }
}

static void
nrXI_func (char *name, char *id, char *desc, char *comp)
{
    if ((name == NULL) || (*name == '-'))
        name = sectionName;
    if ((id == NULL)  || (*id == '-'))
        id = sectionId;
    if ((desc == NULL) || (*desc == '-'))
        desc = sectionDesc;
#ifdef THREF
    if ((strcmp (name, sectionName) != 0) ||
        ((id != NULL) &&
         (sectionId != NULL) &&
         (strcmp (id, sectionId) != 0)))
#endif
    {
        /* Add definition x-ref */
        latexStr ("\\hypertarget{%s}{}", nrMakeXref (name,id));
        latexStr ("%% .XI %s", nrMakeXref (name,id));
        latexEol ();
    }
}

static void
nrXJ_func (char *name, char *id, char *desc, char *comp)
{
    if (name == NULL)
        name = sectionName;
    if (id == NULL)
        id = sectionId;
    if (desc == NULL)
        desc = sectionDesc;
#ifdef THREF
    if ((strcmp (name, sectionName) != 0) ||
        ((id != NULL) &&
         (sectionId != NULL) &&
         (strcmp (id, sectionId) != 0)))
#endif
    {
        /* Add definition x-ref */
        latexStr ("\\hypertarget{%s}{}", nrMakeXref (name,id));
        latexStr ("%% .XJ %s", nrMakeXref (name,id));
        latexEol ();
    }
}

static void
nrXP_func (char *name, char *id, char *desc, char *comp)
{
    if (name == NULL)
        name = sectionName;
    if (id == NULL)
        id = sectionId;
    if (desc == NULL)
        desc = sectionDesc;

#ifdef THREF
    if ((strcmp (name, sectionName) != 0) ||
        ((id != NULL) &&
         (sectionId != NULL) &&
         (strcmp (id, sectionId) != 0)))
#endif
    {
        latexStr ("\\hypertarget{%s}{}", nrMakeXref (name,id));
        latexStr ("%% .XP %s", nrMakeXref (name,id));
        latexEol ();
    }
}

static void
nrMe_func (char *text)
{
#if 0
    if ((text != NULL) && (*text != '\0'))
    {
        latexStr ("%% Me: ", text);
        latexEol ();
    }
#endif
    return;
}

/*
 * nrStart - The start of a nroff include file
 *
 * fname - The name of the file
 * imode - Pointer to flag to enable processing of file,
 *         Set to Enable (0) or Disable (1) processing of file.
 */
static void
nrStartInc (char *fname, int *imode)
{
    latexStr ("%% Include File: %s", fname);
    latexEol ();

    /* Disable processing of includes locally */
    *imode = 1;
}

/*
 * nrStart - The start of a nroff file
 *
 * fname - The name of the file
 * flag  - Unused (0)
 */
static void
nrStart (char *fname, int flag)
{
    latexStr ("%% Start File: %s", fname);
    latexEol ();

    /* Set up the conversion */
    /* Reset the modes for the new page */
    currmode = 0;
    prevmode = 0;
    rsreindent = 0;
}

/*
 * nrStart - The start of a nroff file
 *
 * fname - The name of the file
 * imode -
 * flag  - Unused (0)
 */
static void
nrEnd (char *fname, int imode, int flag)
{
    latexStr ("%% End File: %s", fname);
    latexEol ();
}

static void
nrComment (char *comment, int priority)
{
    if (priority == 0)
        return;
    if (comment[0] != '\0')
    {
        latexStr ("%% Comment: %s", comment);
        latexEol ();
    }
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
    fprintf (stdout, "-a <file> : TeX prefix file to prepend to start of output file\n");
    fprintf (stdout, "-A <file> : TeX postfix file to append to end of output file\n");
    fprintf (stdout, "-c <name> : Copyright <name>\n");
    fprintf (stdout, "-E <file> : Log errors to <file> (append)\n");
    fprintf (stdout, "-e <file> : Log errors to <file> (re-write)\n");
    fprintf (stdout, "-H <home> : Home page name. (default is 'home')\n");
    fprintf (stdout, "-I        : Version Information\n");
    fprintf (stdout, "-l <lib>  : Library component\n");
    fprintf (stdout, "-L <path> : Library search path\n");
    fprintf (stdout, "-M <mod>  : Module name\n");
    fprintf (stdout, "-o <file> : Output to <file> default is <files>.tex\n");
    fprintf (stdout, "-p <file> : Name of the logo picture (image)\n");
    fprintf (stdout, "-q        : Quiet\n");
    fprintf (stdout, "-s        : Output to standard out.\n");
    fprintf (stdout, "-v <level>: Change the verbose level 0..9, default 0=off\n");
    exit (1);
}

static void
latexInitialise (void)
{
    static  nrFUNCTION funcTab = {NULL};
    time_t     clock;           /* Time in machine format. */
    struct tm  *time_ptr;       /* Pointer to time frame. */

    /* Get the time - require the year */
    clock = time (0);
    time_ptr = (struct tm *) localtime (&clock);        /* Get time frame */
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

/*
 * insertFile - Open and insert file.
 *
 * outfpr - The output file pointer.
 * fname - The file to open and insert
 *
 * Returns the error state, 0 is successful, otherwise non-zero is failure.
 */
int
insertFile (FILE *outfpr, char *fname)
{
    int status = 0;                     /* The return status */

    /* Ensure that we have a valid file pointer and filename. */
    if ((fpr != NULL) && (fname != NULL))
    {
        FILE *infpr;

        /* Open the input file */
        if ((infpr = fopen (fname, "r")) == NULL)
        {
            /* Not found */
            uError ("Cannot open file [%s]\n", fname);
            status = 1;
        }
        else
        {
            int cc;

            /* Logging */
            uVerbose (1, "Inserting file %s.\n", fname);

            /* Copy the input file to the output */
            do
            {
                cc = fgetc (infpr);
                if (cc != EOF)
                    fputc (cc, outfpr);
            }
            while (cc != EOF);

            /* Close the input file */
            fclose (infpr);
        }
    }

    /* Return the status to the caller */
    return status;
}

int
main (int argc, char *argv [])
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

    /* Change to an ASCII name format */
    nrLibNameFormat(1);

    while (1) {
        c = getopt (argc, argv, "a:A:c:e:E:H:Il:L:M:o:p:qsv:");
        if (c == EOF)
            break;
        switch (c) {
        case 'a':
            latexPrefix = optarg;
            break;
        case 'A':
            latexPostfix = optarg;
            break;
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
        case 'v':
            {
                int param = (int)('0');

                if (optarg != NULL)
                    param = (int) optarg[0] - param;
                uVerboseSet (param);
            }
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
        oname = strdup (makeFilename (NULL, NULL, moduleName, "tex"));

    if (argc > 1)
    {
        if (oname != NULL)
        {
            /* Open the output file */
            if ((fpr = fopen (oname, "w")) == NULL)
                uFatal ("Cannot open file [%s]\n", oname);
            uVerbose (0, "Output in file [%s].\n", oname);

            /* Insert the prefix file if specified */
            insertFile (fpr, latexPrefix);

            /* Initialise the droff component */
            droff_init ();
        }
        else
            uVerbose (0, "No output file [NULL].\n");
    }
    else
        fpr = NULL;

    /* Process all of the files */
    for (i = 1; i < argc; i++)
    {
        if ((nrfp = nrFilePush (argv[i])) == NULL)
            exit (1);
        uVerbose (0, "Processing file %s.\n", argv[i]);

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

            /* Insert the prefix file if specified */
            insertFile (fpr, latexPrefix);
            droff_init ();              /* Start up if in sections */
        }

        nroff (nrfp, NROFF_MODE_DEFAULT);

        fflush (fpr);
        fflush (stdout);
        if ((oname == NULL) && (fpr != stdout)) {
            droff_term ();
            latexEof ();

            /* Insert the prefix file if specified */
            insertFile (fpr, latexPostfix);
            fclose (fpr);
            fpr = NULL;
        }
    }

    if (fpr != NULL) {
        droff_term ();
        insertFile (fpr, latexPostfix);
        latexEof ();
        fclose (fpr);
    }

    uCloseErrorChannel ();
    return (ecount);
}
