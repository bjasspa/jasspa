/****************************************************************************
 *
 *			Copyright 1997-2004 Jon Green.
 *			    All Rights Reserved
 *
 *
 *  System        :
 *  Module        :
 *  Object Name   : $RCSfile: nrcheck.c,v $
 *  Revision      : $Revision: 1.3 $
 *  Date          : $Date: 2004-02-07 19:29:49 $
 *  Author        : $Author: jon $
 *  Last Modified : <040207.1920>
 *
 *  Description
 *
 *  Notes
 *
 *  History
 *
 * 1.0.0d - JG 07/02/04 Ported to HP-UX 11.00
 * 1.0.0c - JG 03/01/04 Ported to Sun Solaris 9
 * 1.0.0b - JG 05/12/95 Added bullet support.
 * 1.0.0a - JG 16/11/96 Integrated new utilies library.
 *
 ****************************************************************************
 *
 *  Copyright (c) 1997-2004 Jon Green.
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
#include "nroff.h"

/* Macro Definitions */
#define MODULE_VERSION  "1.0.0d"
#define MODULE_NAME     "nrcheck"

#define NORMAL_MODE 0x0000
#define BOLD_MODE   0x0100
#define ITALIC_MODE 0x0200
#define MONO_MODE   0x8000
#define MODE_CHAR   0x007f
#define MODE_MASK   0xff00
#define JUST_MODE   0x0400
#define PARA_MODE   0x0800
#define SPACE_MODE  0x1000
#define TITLE_MODE  0x2000
#define PAGE_MODE   0x4000
#define FONT_MODE   (BOLD_MODE|ITALIC_MODE|MONO_MODE)

#define PARA_CLOSE  0
#define PARA_NORMAL 1
#define PARA_BLANK  2
#define PARA_TERM   3

#define FULL_INDENT 360                 /* Full indent distance. */
#define HALF_INDENT 180                 /* Half indent distance. */
#define LINE_HIEGHT 100                  /* twips height */

#define dprintf(x)  /*printf x*/

static int  para_clean;
static int  para_mode;
static int  indent;
static int  sub_indent;
static int  mode;
static char *im_buf = NULL;
static char *date_buf = NULL;
static char *sectionName = NULL;
static char *sectionId = NULL;
static char *sectionComponent = NULL;
static char *progname = MODULE_NAME;
static int  prevMode = 0;

static void
setParaMode (int mode)
{
    /* Turn off any highlighting modes if required. */
    if (((mode  & BOLD_MODE) == 0) && ((para_mode & BOLD_MODE) != 0)) {
        para_mode &= ~BOLD_MODE;
    }
    if (((mode  & ITALIC_MODE) == 0) && ((para_mode & ITALIC_MODE) != 0)) {
        para_mode &= ~ITALIC_MODE;
    }
    if (((mode  & MONO_MODE) == 0) && ((para_mode & MONO_MODE) != 0)) {
        para_mode &= ~MONO_MODE;
    }

    /* Turn on any highlighting modes if required. */
    if (((mode  & BOLD_MODE) != 0) && ((para_mode & BOLD_MODE) == 0)) {
        para_mode |= BOLD_MODE;
    }
    if (((mode  & ITALIC_MODE) != 0) && ((para_mode & ITALIC_MODE) == 0)) {
        para_mode |= ITALIC_MODE;
    }
    if (((mode  & MONO_MODE) != 0) && ((para_mode & MONO_MODE) == 0)) {
        para_mode |= MONO_MODE;
    }
}

static void
insertPara (int pmode)
{
    /* End the last paragraph. Close any highlighting if necessary */
    if (pmode == PARA_TERM) {
        setParaMode (0);
        para_mode = PARA_TERM;
    }
    else if (para_clean != 0) {
        setParaMode (0);                /* Turn off any enabled modes */
        para_mode = 0;                  /* Reset the mode */
    }

    /* Start a new paragraph if requested. Use the formatting required. */
    if (((pmode & 0xf) == PARA_BLANK) ||
        ((pmode & 0xf) == PARA_NORMAL)) {

        para_mode = pmode;
        para_clean = 1;
    }

    if ((pmode != PARA_NORMAL) && (pmode != PARA_TERM)) {
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
            break;
        case ' ':
            if (((lmode & SPACE_MODE) != 0) && (space != 0))
                break;
            space = 1;
            break;
        case ESC_CHAR:
            break;
        case USPACE_CHAR:
            break;
        default:
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
    setParaMode (0);
}

static void
nrId_func (char *date)
{
    date_buf = bufFree (date_buf);
    if (date != NULL)
        date_buf = bufStr (NULL, date);
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
nrTH_func (char *id, char *num, char *date, char *company, char *title)
{

    bufFree (sectionName);
    bufFree (sectionId);

    indent = 0;
    sub_indent = 0;
    mode = BOLD_MODE | PAGE_MODE | TITLE_MODE;

    sectionName = bufStr (NULL, id);
    sectionId = bufNStr (NULL, num);

    rtfFormatStr (mode, "", NULL);

    setParaMode (0);
    mode = 0;
    prevMode = 0;
}

static void
nrKw_func (char **argv)
{
    rtfFormatStr (mode, "", NULL);
}

static void
nrNH_func (char *id, char *num, char *title, char *xref)
{

    bufFree (sectionName);
    bufFree (sectionId);

    indent = 0;
    sub_indent = 0;
    insertPara (PARA_CLOSE);
    mode = BOLD_MODE | PAGE_MODE | TITLE_MODE;

    sectionName = bufStr (NULL, id);
    sectionId = bufNStr (NULL, num);

    rtfFormatStr (mode, "", NULL);

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
    setParaMode (PARA_TERM);
    sub_indent = 0;
    mode = 0;
}

static void
nrGH_func (char *title)
{
    if (title == NULL)
        return;

    indent = 0;
    sub_indent = 0;
    insertPara (PARA_CLOSE);
    mode = BOLD_MODE | PAGE_MODE;

    rtfFormatStr (mode, "", NULL);
    rtfFormatStr (mode, title, NULL);
    setParaMode (PARA_TERM);
    mode = 0;
}

static void
nrSH_func (char *s)
{
    indent = 0;
    sub_indent = 0;
    insertPara (PARA_BLANK);
    insert (s, BOLD_MODE);
    insertPara (PARA_TERM);
    prevMode = 0;
    mode = JUST_MODE;
    indent = FULL_INDENT;
}

static void
nrSS_func (char *s)
{
    sub_indent = 0;
    indent = HALF_INDENT;
    insertPara (PARA_BLANK);
    insert (s, BOLD_MODE);
    insertPara (PARA_TERM);
    prevMode = 0;
    mode = JUST_MODE;
    indent = FULL_INDENT;
    sub_indent = 0;
}

static void
nrPP_func (void)
{
    insertPara (PARA_BLANK);
    sub_indent = 0;
}

static void
nrHl_func (char *text, char *name, char *section,
           char *concat, char *module, char *file, int type)
{
    setParaMode (0);
    rtfFormatStr (mode, " ", NULL);
}

static void
nrHt_func (char *name, char *section, char *concat,
           char *module, char *file, int type)
{
    setParaMode (0);
    rtfFormatStr (mode, " ", NULL);
}

static void
nrHg_func (char *name, char *concat)
{
    setParaMode (0);
}

static void
nrCS_func (int i)
{
    mode |= PARA_MODE;
    mode &= ~JUST_MODE;
    indent += FULL_INDENT;              /* Add indent for Code start */
    sub_indent = 0;                     /* Clear the sub-indent */
    while (--i >= 0)
        insertPara (PARA_BLANK);
}

static void
nrCE_func (int i)
{
    mode &= ~PARA_MODE;
    mode |= JUST_MODE;
    indent -= FULL_INDENT;              /* Remove the indent */
    while (--i >= 0)
        insertPara (PARA_BLANK);
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
nrbr_func (void)
{
    insertPara (PARA_TERM);
}

static void
nrbp_func (void)
{
    /* Do not do pages in check. Make new paragraph */
    insertPara (PARA_BLANK);
}

static void
nrsp_func (int i)
{
    /* Do not do pages in check. Make new paragraph */
    insertPara (PARA_BLANK);
}

static void
nrTP_func (int i)
{
    if (i != 0)                         /* Start of function */
    {
        sub_indent = 0;
        insertPara (PARA_BLANK);
    }
    else
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
        insertPara (PARA_BLANK);
}

static void
nrStart (char *fname, int flag)
{
    /* Set up the conversion */

    para_mode = 0;
    para_clean = 0;
    mode = 0;
}

static void
nrStartInc (char *fname, int *imode)
{
    /* Set up the conversion */

    para_mode = 0;
    para_clean = 0;
    mode = 0;
    *imode = 1;                         /* Disable processing of includes */
}

static void
nrEnd (char *fname, int imode, int flag)
{
    if (imode == 0)
        insertPara (PARA_CLOSE);     /* Flush out the buffer */
}

static void
usage (void)
{
    fprintf (stdout, "%s : Usage %s [Options] <files>\n", progname, progname);
    fprintf (stdout, "\n");
    fprintf (stdout, "-?/h      : Help - this page\n");
    fprintf (stdout, "-E <file> : Log errors to <file> (append)\n");
    fprintf (stdout, "-e <file> : Log errors to <file> (re-write)\n");
    fprintf (stdout, "-I        : Version information.\n");
    fprintf (stdout, "-q        : Quiet mode\n");
    exit (1);
}

static void
checkInitialise (void)
{
    static  nrFUNCTION funcTab = {NULL};

    /* Headers */
    nrInstall (funcTab, NH_func, nrNH_func);
    nrInstall (funcTab, FH_func, nrFH_func);
    nrInstall (funcTab, TH_func, nrTH_func);
    nrInstall (funcTab, GH_func, nrGH_func);
    nrInstall (funcTab, SH_func, nrSH_func);
    nrInstall (funcTab, SS_func, nrSS_func);

    /* Identification */
    nrInstall (funcTab, XI_func, NULL);
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
    nrInstall (funcTab, Hr_func, NULL);
    nrInstall (funcTab, Hg_func, nrHg_func);

    /* Fonts */
    nrInstall (funcTab, CS_func, nrCS_func);
    nrInstall (funcTab, CE_func, nrCE_func);
    nrInstall (funcTab, ft_func, nrft_func);

    /* Indentation */
    nrInstall (funcTab, RS_func, nrRS_func);
    nrInstall (funcTab, RE_func, nrRE_func);

    /* Text adjustment */
    nrInstall (funcTab, na_func, nrna_func);
    nrInstall (funcTab, nf_func, nrnf_func);
    nrInstall (funcTab, fi_func, nrfi_func);
    nrInstall (funcTab, ad_func, nrad_func);
    nrInstall (funcTab, br_func, nrbr_func);
    nrInstall (funcTab, bp_func, nrbp_func);
    nrInstall (funcTab, ll_func, NULL);
    nrInstall (funcTab, pl_func, NULL);
    nrInstall (funcTab, sp_func, nrsp_func);
    nrInstall (funcTab, ne_func, NULL);

    /* Files */
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
    nrFILE  *nrfp;
    int     ecount = 0;                 /* Error count */
    int     wcount = 0;                 /* Warn count */
    int     c;
    int     i;

    /* Initialise the error channel */
    uErrorSet (0, &ecount);             /* Max Num entries + counter */
    uWarnSet (&wcount);                 /* Warning count */
    uUtilitySet (progname);             /* Set name of program */

    checkInitialise ();

    while (1) {
        c = getopt (argc, argv, "e:E:h?Iq");
        if (c == EOF)
            break;
        switch (c) {
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
        case 'q':
            uInteractiveSet (0);        /* Disable interactive mode */
            break;
        default:
            uError ("Cannot understand option [-%c]\n", c);
            usage();
            break;
        }
    }

    uOpenErrorChannel ();
    argv = getfiles (&argc, argv, optind);

    /* Process all of the files */
    for (i = 1; i < argc; i++) {
        if ((nrfp = nrFilePush (argv[i])) == NULL)
            exit (1);
        nroff (nrfp, NROFF_MODE_DEFAULT);    /* Set up the conversion */
    }

    uCloseErrorChannel ();
    return (ecount);
}
