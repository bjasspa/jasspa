/****************************************************************************
 *
 *			Copyright 1995 Jon Green.
 *			   All Rights Reserved
 *
 *
 *  System        :
 *  Module        :
 *  Object Name   : $RCSfile: droff.c,v $
 *  Revision      : $Revision: 1.1 $
 *  Date          : $Date: 2000-10-21 14:31:26 $
 *  Author        : $Author: jon $
 *  Last Modified : <000125.2130>
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
 *  Copyright (c) 1995 Jon Green.
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
#include <string.h>
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

/*
 * Version 1.0.0a - 04/10/95 - JG
 * Remove the sub-indent anomaly with RTF generation on .CS.
 *
 * Version 1.0.0b - 14/11/95 - JG
 * Corrected code to detect bad font chnages. Now produces
 * an error when a fonts are mixed badly.
 *
 * Version 1.0.0c - 15/11/95 - JG
 * Added .sp and .ne support for blank lines and space requirements
 * for pictures etc.
 *
 * Version 1.0.0d - 05/12/95 - JG
 * Added bullet support.
 *
 * Version 1.0.0e - 06/01/95 - JG
 * Modified arguments for the new compiling option.
 * 
 * Version 1.0.0f - 14/05/97 - JG
 * Added copyright option.
 */

#define MODULE_VERSION  "1.0.0f"
#define MODULE_NAME     "droff"

#define FULL_INDENT 5
#define HALF_INDENT (FULL_INDENT/2)

#define MAX_LINE_LEN        1024
#define DFL_LINE_LEN        79
#define DEFAULT_PAGE_LEN    66

#define PRE_HEADER  3
#define POST_HEADER 3
#define TOTAL_HEADER    (PRE_HEADER+POST_HEADER)

#define PRE_TRAILER  3
#define POST_TRAILER 3
#define TOTAL_TRAILER    (PRE_TRAILER+POST_TRAILER)

#define NORMAL_MODE 0x0000
#define BOLD_MODE   0x0100
#define ITALIC_MODE 0x0200
#define MODE_CHAR   0x007f
#define MODE_MASK   0xff00
#define JUST_MODE   0x0400
#define PARA_MODE   0x0800
#define SPACE_MODE  0x1000
#define MONO_MODE   0x2000
#define FONT_MODE   (MONO_MODE|BOLD_MODE|ITALIC_MODE)

#define dprintf(x)  /* printf x */

typedef
struct {
    int     pos;
    int     buf [MAX_LINE_LEN+5];
} Line;

static FILE *fo;
static int  clean_para;
static int  indent = 0;
static int  sub_indent = 0;
static int  prevailing_line = 1;
static Line line;
static Line header;
static Line trailer;
static int  line_left = 0;
static int  line_right = DFL_LINE_LEN;
static int  mode;
static int  prevMode = 0;
static int  op_pg_num = 0;
static int  op_pg_line = 0;
static char *im_buf = NULL;
static char *date_buf = NULL;
static int  page_len = DEFAULT_PAGE_LEN;
static char *progname = MODULE_NAME;
static int  quietMode = 0;
static int  asciiMode = 0;
static int  plainText = 0;
static int  lastMode = 0;
static char *copyrightName = NULL;
static char *companyName = NULL;
static int  day;
static int  month;
static int  year;

static void insertPara (int mode);

static void
droffEof (void)
{
#ifndef _UNIX
    if ((asciiMode == 0) || plainText)
        fprintf (fo, "%c", 0x1a);
#endif
}

static void
droffEol (void)
{
#ifndef _UNIX
    if ((asciiMode == 0) || plainText)
        fprintf (fo, "\r\n");
    else
#endif
        fprintf (fo, "\n");
}

static void
outChar (int ic)
{
    char    c;
    int     i;

    c = (char)(ic & MODE_CHAR);
    if (c == USPACE_CHAR) {
        c = ' ';
        ic &= 0xff;
    }
    else if (c == ' ')
        ic &= 0xff;
    else if (c == '\n') {
        droffEol ();
        return;
    }
    else if (c == ESC_CHAR)
        c = '\\';
    else if (c == COPYRIGHT_CHAR)
        c = 'C';
    else if (c == BULLET_CHAR)
    {
        c = '*';
        ic = BOLD_MODE;
    }
    else if (c == MSPACE_CHAR)
        return;
    else if (c == ZSPACE_CHAR)
        return;
    
    if (plainText)
    {
        switch (ic & FONT_MODE)
        {
        case BOLD_MODE:
        case MONO_MODE:
        case ITALIC_MODE:
        default:
            fprintf (fo, "%c", c);
            break;
        }
    }
    else if (asciiMode)
    {
        switch (ic & FONT_MODE) {
        case MONO_MODE:
            if (lastMode != MONO_MODE) {
                fprintf (fo, "%c[7m", 0x1b);
                lastMode = MONO_MODE;
            }
            fprintf (fo, "%c", c);
            break;

        case BOLD_MODE:
            if (lastMode != BOLD_MODE) {
                fprintf (fo, "%c[1m", 0x1b);
                lastMode = BOLD_MODE;
            }
            fprintf (fo, "%c", c);
            break;

        case ITALIC_MODE:
            if (lastMode != ITALIC_MODE) {
                fprintf (fo, "%c[4m", 0x1b);
                lastMode = ITALIC_MODE;
            }
            fprintf (fo, "%c", c);
            break;

        default:
            if (lastMode != 0) {
                fprintf (fo, "%c[0m", 0x1b);
                lastMode = 0;
            }
            fprintf (fo, "%c", c);
            break;
        }   /* End of 'switch' */
    }
    else 
    {
        switch (ic & FONT_MODE)
        {
        case BOLD_MODE:
        case MONO_MODE:
            for (i = 0; i < 3; i++)
                fprintf (fo, "%c%c", c, (char) 8);
            fprintf (fo, "%c", c);
            break;

        case ITALIC_MODE:
            fprintf (fo, "%c%c%c", '_', (char) 8, c);
            break;
        default:
            fprintf (fo, "%c", c);
            break;
        }   /* End of 'switch' */
    }
}

static void
outBuf (Line *lp, int reset)
{
    int     i;

    for (i = 0; i < lp->pos; i++) {
        outChar (lp->buf [i]);
        if (reset != 0)
            lp->buf [i] = 0;
    }   /* End of 'for' */
}


static int
outHeader ()
{
    int i;
    for (i = 0; i < PRE_HEADER; i++)
        droffEol ();
    outBuf (&header, 0);
    for (i = 0; i < POST_HEADER; i++)
        droffEol ();
    op_pg_num ++;
    return (TOTAL_HEADER);
}

static void
outTrailer ()
{
    int i;

    for (i = 0; i < PRE_TRAILER; i++)
        droffEol ();
    outBuf (&trailer, 0);
    for (i = 0; i < POST_TRAILER; i++)
        droffEol ();
}

static int
outLine (Line *lp)
{
    if (op_pg_line == 0)
        op_pg_line = outHeader ();
    if (lp != NULL)
        outBuf (lp, 1);
    droffEol();
    clean_para = 0;
    op_pg_line++;
    if (op_pg_line >= (page_len - TOTAL_HEADER)) {
        outTrailer ();
        op_pg_line = 0;
        return (1);
    }
    return (0);
}


static void
insertLineChar (char c, int *pos, Line *lp)
{
    int     i;

    for (i = lp->pos; i > *pos; i--)
        lp->buf [i] = lp->buf [i-1];
    lp->buf [i+1] = lp->buf [i];
    lp->buf [i] = c;
    lp->pos = lp->pos+1;
    *pos = *pos+1;
}

static void
copyRight (Line *db, Line *sb)
{
    int     slen;
    int     i, j;

    slen = sb->pos;
    for (i = db->pos; i < line_right; i++)
        db->buf[i] = (int)(' ');
    db->pos = line_right;
    for (i = line_right - slen, j = 0; j < slen; i++, j++)
        db->buf[i] = sb->buf[j];
}

static void
copyCentre (Line *db, Line *sb)
{
    int     slen;
    int     i, j;
    int     cstart;

    slen = sb->pos;
    cstart = ((line_right - line_left)/2) + line_left;
    for (i = db->pos; i < (cstart+slen); i++)
        db->buf[db->pos++] = (int)(' ');
    for (i = cstart - slen/2, j = 0; j < slen; j++, i++)
        db->buf[i] = sb->buf [j];
}

/* goto_gap	This routine will goto the 'n' th gap. in a line. The routine
		will return the number of gaps encountered. Doto is left at the
		last gap encountered + 1, or at the end of the line. Which ever
		occurs first.

*/

static int
goto_gap (int n, int *length, Line *lp)
{
    int	last_c;         /* Last character read. */
    int c;              /* Current character */
    int gap_count;      /* Number of gaps encountered */
    int spos;           /* Start position */

    spos = *length;     /* Get the starting position */
    gap_count = 0;	/* No gaps */
    last_c = '\0';	/* Reset word */

    /* Scan the line until we reach the end marker, or we have accumulated
       enough gaps.  Leave the dot at the end of the line or at the first
       charcter following the last gap fount.  ie.

       This is a line of text in the buffer.
               ^                           ^
       [Scan for 2nd Gap]           [Scan for all gaps]
    */

    while (spos < lp->pos)
    {
        c = lp->buf [spos] & MODE_CHAR;
dprintf (("c/l = [%c/%c:%d/%d]\n", c, last_c, c, last_c));
        if (last_c == ' ' && c != ' ') {
            /* Found a new word */
            gap_count++;                /* Record word */
            if (gap_count >= n)         /* Finished ?? */
                return (gap_count);
        }
        last_c = c;                     /* Save last character */
        spos++;                         /* Next character position */
        *length = spos;                 /* Record length */
    }
    return (gap_count);                 /* Record word */

}	/* End of "goto_gap" */

/* justify	- This routine will justify a block of text. The start of the
 *		  text is at doto, and the scaller specifies the length of the
 *		  text to be justified, as well as the new required length.
 *		  This routine assumes the caller has already removed tabs
 *		  as justification is performed on spaces only. Additionally the
 *		  caller has already formatted the line to the correct length.
 *		  Justification is performed on the right margin only. (The
 *		  left margin is assumed to be done).
 */

static void
justify (Line *lp)
{
int	gaps;			/* The number of gaps in the line */
int	spaces;			/* The number of spaces to be filled */
int	base_count;		/* The min numebr of spaces in each gap */
int	fill_count;		/* The number of spaces to be distributed */
int	sp_sum;			/* Pseudo space percentage usage of the line */
int	sp_inc;			/* Percentage space increment added on a gap */
int	gap_sum;		/* Pseudo gap percentage when space written */
int	gap_inc;		/* Percentage of gap used. */
int	i;			/* Local loop counters */
int	no_to_add;		/* No spaces to add */

int	clength;
int	rlength;
int     llength;
int     gap_pos;

    /* Check the parameters */

    llength = indent + sub_indent;
    rlength = line_right;
    clength = lp->pos;
    if (clength >= rlength)             /* Too big or ok. */
        return;	                        /* No - exit */

    /* Get a count on the number of gaps.  There cannot be more gaps that
       the length of the line !! */

    gap_pos = llength;                          /* line length */
    gaps = goto_gap (clength, &gap_pos, lp);    /* No gaps */
    spaces = rlength - lp->pos;                 /* Number to fill */
    if (spaces <= 0)                            /* Any filing to be done ?? */
        return;
    if (gaps == 0)                              /* No gaps ?? */
	return;

    /* Calculate the position of the spaces to be added in the line.  To
       obtain an even distribution of the spaces then use a percentage
       method whereby the percentage of the line is cacluated and the
       percentage of the remaining characters to infill are calculated.
       When the infill (gap) percentage is smaller than the space percentage
       then a fill character is added.  Percentages are done reletive to
       256, and are performed as summations rather than floating point
       division/multiplication to speed up the process.  */

    base_count = spaces/gaps;	/* No spaces to be put in every gap */
    fill_count = spaces%gaps;	/* Remaining spaces to be allocated. */

    sp_inc = 256;			/* Space increment */
    sp_sum = sp_inc;		/* Start from increment */

    gap_inc = (gaps *256)/(fill_count+1);	/* Gap increment. NOTE +1 */
    gap_sum = gap_inc;			/* Start from increment */

    /* Loop while not all of the spaces have been filled up. */

    gap_pos = llength;
    for (i = 0; i < spaces; sp_sum += sp_inc) {
        goto_gap(1, &gap_pos, lp);              /* Goto the next gap */

        /* If the space percentage is such that we add a filler character
           then do it.  */

        no_to_add = base_count;
	if (no_to_add != 0)
            i += base_count;            /* Add to No spaces added */
        if (sp_sum >= gap_sum) {	/* INfill here ?? */
            if (i < spaces) {	/* Finished ?? */
                i++;		/* Extra filler */
                no_to_add++;	/* Extra filler */
            }
            gap_sum += gap_inc;	/* Increment gap sum */
        }
        while (--no_to_add >= 0)
            insertLineChar (' ', &gap_pos, lp);	/* Yes - insert */
    }
}



static void
insert_word (char *s, int mode, Line *lp, int *newMode)
{
    int     cmode;
    int     length;

    mode &= ~SPACE_MODE;
    if (s == NULL)
        return;
    line_left = indent + sub_indent;
    while (lp->pos < line_left)
        lp->buf [lp->pos++] = ' ';
    length = trueStrLen (s);
    if ((length + lp->pos) > line_right) {
        while ((lp->pos > 0) &&
               ((((lp->buf [lp->pos-1]) & MODE_CHAR) == ' ') ||
                (((lp->buf [lp->pos-1]) & MODE_CHAR) == USPACE_CHAR)))
            lp->pos = lp->pos - 1;
        if (mode & JUST_MODE)
            justify (lp);
        insertPara (0);
        line_left = indent + sub_indent;
        while (lp->pos < line_left)
            lp->buf [lp->pos++] = ' ';
        if (length == 1 && *s == ' ')
            return;
    }
    while ((cmode = *s) != 0) {
        switch (cmode)
        {
        case MONO_CHAR:
            prevMode = mode & FONT_MODE;
            if (mode & MONO_MODE)
                uError ("Already have mono mode enabled.\n");
            mode = (mode & ~FONT_MODE)|MONO_MODE;
            if (newMode != NULL)
                *newMode = (*newMode & ~FONT_MODE) | MONO_MODE;;
            break;
        case ITALIC_CHAR:
            prevMode = mode & FONT_MODE;
            if (mode & ITALIC_MODE)
                uError ("Already have italic mode enabled.\n");
            mode = (mode & ~FONT_MODE)|ITALIC_MODE;
            if (newMode != NULL)
                *newMode = (*newMode & ~FONT_MODE) | ITALIC_MODE;;
            break;
        case BOLD_CHAR:
            prevMode = mode & FONT_MODE;
            if (mode & BOLD_MODE)
                uError ("Already have bold mode enabled.\n");
            mode = (mode & ~FONT_MODE)|BOLD_MODE;
            if (newMode != NULL)
                *newMode = (*newMode & ~FONT_MODE) | BOLD_MODE;;
            break;
        case ROMAN_CHAR:
            prevMode = mode & FONT_MODE;
            if ((mode & FONT_MODE) == 0)
                uError ("Already have roman mode enabled.\n");
            mode = mode & ~(FONT_MODE);
            if (newMode != NULL)
                *newMode = (*newMode & ~FONT_MODE);
            break;
        case PREV_CHAR:
            if ((mode & FONT_MODE) == 0)
                uError ("Current font is \\fP and current font is Roman\n");
            else if (prevMode == (mode & FONT_MODE))
                uWarn ("Previous and current font are the same\n");
            else
                mode = (mode & ~FONT_MODE) | prevMode;
            if (newMode != NULL)
                *newMode = (*newMode & ~FONT_MODE) | prevMode;
            break;
        case MSPACE_CHAR:
            break;
        case ZSPACE_CHAR:
            break;
        default:
            lp->buf [lp->pos++] = cmode | mode;
            break;
        }   /* End of 'switch' */
        s++;
    }
    clean_para = 1;
}

static void
insert (char *s, int mode, Line *lp)
{
    int     wlength;
    int     length;
    int     i;
    char    buffer [1024];
    int     firstMode = mode;
    int     no_space = 0;

#ifdef INSERT_DEBUG
    static  FILE *fp = NULL;

    if (fp == NULL)
    {
        fp = fopen ("droff.log","w");
    }

    fprintf (fp,"insert Entry (%#010x) [%s]\n", mode, s);
#endif
    if (s == NULL)
        return;
    if (lp == NULL)
        lp = &line;
    while (*s) {
        s = rightTrim(leftTrim (s));
        if (s == NULL)
            break;
        wlength = wordLen (s);
        for (length = 0, i = 0; i < wlength; i++)
        {
            if ((buffer [length] = s[i]) == CONTINUE_CHAR)
                no_space = 1;
            else
                length++;
        }
        
        buffer [length] = '\0';
        s = &s [wlength];
#ifdef INSERT_DEBUG
        fprintf (fp,"insert word In (%#010x) [%s]\n", mode, buffer);
#endif
        insert_word (buffer, mode, lp, &mode);
#ifdef INSERT_DEBUG
        fprintf (fp,"insert word Out (%#010x)\n", mode);
#endif
        if ((mode & SPACE_MODE) && (*s == '\0'))
            break;
#ifdef INSERT_DEBUG
        fprintf (fp,"insert word Space In (%#010x)\n", mode);
#endif
        if (no_space == 0) 
            insert_word (" ", mode, lp, &mode);
        else
            no_space = 0;
#ifdef INSERT_DEBUG
        fprintf (fp,"insert word Space Out (%#010x)\n", mode);
#endif
    }

    /*
     * Make sure the modes are consistent
     */

    if ((mode & FONT_MODE) != (firstMode & FONT_MODE))
        uWarn ("Possible inconsistent font change detected\n");
#ifdef INSERT_DEBUG
    fprintf (fp,"insert Exit (%#010x)\n", mode);
    fflush (fp);
#endif
}

static int
insert_pos (void)
{
    return (line.pos);
}

static void
insertPara (int mode)
{
    if (clean_para != 0) {
        outLine (&line);
    }
    if (mode == 1) {
        outLine (NULL);
    }
    line.pos = 0;
}   /* End of 'insertPara' */

static void
nrId_func (char *date)
{
    bufFree (date_buf);
    date_buf = bufNStr (NULL, date);
}

static void
nrIm_func (char *module, char *component)
{
    bufFree (im_buf);
    im_buf = bufNStr (NULL, module);
}

static void
nrTH_func (char *id, char *num, char *date, char *company, char *title)
{
    Line    temp;

    header.pos = 0;
    trailer.pos = 0;
    indent = 0;
    sub_indent = 0;

    if (date == NULL)
        date = date_buf;
    if (company == NULL)
        company = companyName;
        
    if (title == NULL)
        title = im_buf;

    if (id != NULL) {
        insert (id, BOLD_MODE|SPACE_MODE, &header);
        if (num != NULL)
        {
            insert ("(", BOLD_MODE|SPACE_MODE, &header);
            insert (num, BOLD_MODE|SPACE_MODE, &header);
            insert (")", BOLD_MODE|SPACE_MODE, &header);
        }
        copyRight (&header, &header);
    }
    else
        insert ("\001", 0, &header);

    if (title != NULL) {
        temp.pos = 0;
        insert (title, BOLD_MODE|SPACE_MODE, &temp);
        copyCentre (&header, &temp);
    }

    if (company != NULL)
        insert (company, BOLD_MODE, &trailer);
    else
        insert_word (" ", mode, &trailer, NULL);
/*      insert ("\001", 0, &trailer);*/

    if (date != NULL) {
        temp.pos = 0;
        insert (date, BOLD_MODE|SPACE_MODE, &temp);
        copyCentre (&trailer, &temp);
    }
#if 0
    outLine (&header);
    outLine (&trailer);
#endif

}

static void
nrSH_func (char *s)
{
    insertPara (0);
    insertPara (1);
    indent = 0;
    sub_indent = 0;
    insert (s, BOLD_MODE, NULL);
    insertPara (0);
    mode = JUST_MODE;
    indent = FULL_INDENT;
    prevMode &= ~FONT_MODE;
    mode &= ~FONT_MODE;
/*    mode = JUST_MODE;*/
}

static void
nrSS_func (char *s)
{
    insertPara (0);
    insertPara (1);
    sub_indent = 0;
    indent = HALF_INDENT;
    insert (s, BOLD_MODE, NULL);
    insertPara (0);
    mode = JUST_MODE;
    indent = FULL_INDENT;
    sub_indent = 0;
    prevMode &= ~FONT_MODE;
}

static void
nrPP_func (void)
{
    insertPara (0);
    insertPara (1);
    sub_indent = 0;
    prevMode = 0;
    mode &= ~FONT_MODE;
}

static void
nrHt_func (char *name, char *section, char *concat,
           char *module, char *file, int type)
{
    int     fmode;
    char    *buf;

    buf = NULL;
    fmode = mode;
    if (name != NULL) {
        buf = bufChr (buf, BOLD_CHAR);
        buf = bufStr (buf, name);
        buf = bufChr (buf, ROMAN_CHAR);
    }
    if (section != NULL) {
        buf = bufChr (buf, '(');
        buf = bufStr (buf, section);
        buf = bufChr (buf, ')');
    }
    if (concat != NULL) {
        buf = bufStr (buf, concat);
    }
    insert (buf, mode, NULL);
    bufFree (buf);
}

static void
nrHl_func (char *text, char *name, char *section,
           char *concat, char *module, char *file, int type)
{
    nrHt_func (text,NULL,concat,module,file,type);
}

static void
nrHh_func (char *text, char *name, char *section,
           char *concat, char *module, char *file, int type)
{
    int     fmode;
    char    *buf;

    buf = NULL;
    fmode = mode;
    if (text != NULL) {
        buf = bufStr (buf, text);
    }
    if (concat != NULL) {
        buf = bufStr (buf, concat);
    }
    insert (buf, mode, NULL);
    bufFree (buf);
}    

static void
nrHr_func (char *name, char *section, char *concat)
{
    nrHt_func (name,section,concat,NULL,NULL,0);
}

static void
nrHg_func (char *name, char *concat)
{
    int     fmode;
    char    *buf;

    buf = NULL;
    fmode = mode;
    if (name != NULL) {
        buf = bufChr (buf, ITALIC_CHAR);
        buf = bufStr (buf, name);
        buf = bufChr (buf, ROMAN_CHAR);
    }
    if (concat != NULL) {
        buf = bufStr (buf, concat);
    }
    insert (buf, mode, NULL);
    bufFree (buf);
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
    insertPara (0);
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
nrbp_func (void)
{
    if (clean_para != 0) {
        if (outLine (&line) != 0)       /* At head of page ?? */
            return;                     /* Yes - quit now */
    }
    if (op_pg_line != 0)                /* If at top of page then skip */
        while (outLine (NULL) == 0)     /* Skip to next page */
            /* Nothing */;
}

static void
nrne_func (int i)
{
    /*
     * If we have not got enough space on the page then start a new page
     */
    if ((op_pg_line + i) >= (page_len - TOTAL_HEADER))
    {
        insertPara (0);
        if (op_pg_line != 0)            /* If at top of page then skip */
        {
            while (outLine (NULL) == 0)
                /* Nothing */;
        }
    }
}

static void
nrll_func (int i)
{
    line_right = i;
}

static void
nrpl_func (int i)
{
    page_len = i;
}

static void
nrsp_func (int i)
{
    insertPara (0);
    while (--i >= 0)
        if (outLine (NULL) != 0)
           break;
}

static void
nrbr_func (void)
{
    insertPara (0);
}

static void
nrBS_func (int i, int j, char *bullet)
{
    insertPara (0);
    while (--i >= 0)
        insertPara (1);

    /* Compute the new bullet offset */
    if ((bullet == NULL) || (strlen (bullet) <= 1))
        sub_indent = HALF_INDENT;
    else
        sub_indent = 0;

    /* If the bullet is unspecified then default. */
    if (bullet == NULL)
        insert ("*", BOLD_MODE, NULL);
    else
        insert (bullet, 0, NULL);

    sub_indent = FULL_INDENT;
}

static void
nrBU_func (int i, char *bullet)
{
    insertPara (0);
    while (--i >= 0)
        insertPara (1);

    if ((bullet == NULL) || (strlen (bullet) <= 1))
        sub_indent = HALF_INDENT;
    else
        sub_indent = 0;

    /* If the bullet is unspecified then default. */
    if (bullet == NULL)
        insert ("*", BOLD_MODE, NULL);
    else
        insert (bullet, 0, NULL);

    sub_indent = FULL_INDENT;
}

static void
nrBE_func (int i)
{
    insertPara (0);
    while (--i >= 0)
        insertPara (1);
    sub_indent = 0;                     /* Reset sub-indent */
}

static void
nrCS_func (int i)
{
    insertPara (0);
    while (--i >= 0)
        insertPara (1);
    mode |= PARA_MODE;
    mode &= ~JUST_MODE;
    sub_indent = 0;                     /* Reset sub-indent */
    indent += FULL_INDENT/2;            /* Increment full indent */
}

static void
nrCE_func (int i)
{
    mode &= ~PARA_MODE;
    mode |= JUST_MODE;
    indent -= FULL_INDENT/2;
    insertPara (0);
    while (--i >= 0)
        insertPara (1);
    /* sub_indent = 0; */
}

static void
nrRS_func (void)
{
    indent += FULL_INDENT;
    insertPara (0);
}

static void
nrRE_func (void)
{
    indent -= FULL_INDENT;
    insertPara (0);
}

static void
nrPD_func (int i)
{
    prevailing_line = i;
}

static void
nrTP_func (int i)
{
    int j;
    
    if (i != 0)                         /* Start of function */
    {
        insertPara (0);
        for (j = prevailing_line; --j >= 0; /* NULL */)
             insertPara (1);
        sub_indent = 0;
    }
    else                                /* End of function */
    {
        sub_indent = FULL_INDENT;
        if (insert_pos() > (indent+sub_indent)) {
            insertPara (0);
        }
    }
}

static void
nrIP_func (char *s)
{
    nrTP_func (1);
    insert (s, 0, NULL);
    nrTP_func (0);
}

static void
nrLP_func (void)
{
    insertPara (0);
    insertPara (1);
    sub_indent = 0;
}

static void
nrBP_func (char *s)
{
    nrTP_func (1);
    insert (s, BOLD_MODE, NULL);
    nrTP_func (0);
}


static void
nrft_func (int font)
{
    int lastFont;
    
    lastFont = mode & FONT_MODE;
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
    case FT_P:
        mode = (mode & ~FONT_MODE) | (prevMode & FONT_MODE);
        break;
    }
    if (lastFont == (mode & FONT_MODE))
        uError (".ft Font already enabled\n");
    else
        prevMode = lastFont;
}


static void
nrTextline (char *s)
{
    if (s == NULL) {
        insertPara (0);
        insertPara (1);
    }
    else if (mode & PARA_MODE) {
        insert (s, mode, NULL);
        insertPara (0);
    }
    else
        insert (s, mode, NULL);
}

static void
nrStart (char *fname, int flag)
{
    /* Set up the conversion */

    mode = 0;
    prevMode = 0;
}

static void
nrStartInc (char *fname, int *imode)
{
    /* Set up the conversion */

    *imode = 1;
    mode = 0;
    prevMode = 0;
}

static void
droff_init (void)
{
    lastMode = 0;
}

static void
droff_term (void)
{
    insertPara (0);     /* Flush out the buffer */
    droffEol ();
}

static void
usage (void)
{
    fprintf (stdout, "%s : Usage %s [Options] <files>\n", progname, progname);
    fprintf (stdout, "\n");
    fprintf (stdout, "-?/h      : Help - this page\n");
    fprintf (stdout, "-a        : ANSI control characters for more(2)\n");
    fprintf (stdout, "-c <name> : Copyright <name>\n");
    fprintf (stdout, "-E <file> : Log errors to <file> (append)\n");
    fprintf (stdout, "-e <file> : Log errors to <file> (re-write)\n");
    fprintf (stdout, "-I        : Version Information\n");
    fprintf (stdout, "-l <len>  : Page length in lines.\n");
    fprintf (stdout, "-o <file> : Output to <file> default is <files>.out\n");
    fprintf (stdout, "-p        : Plain Text (no control characters)\n");
    fprintf (stdout, "-q        : Quiet\n");
    fprintf (stdout, "-s        : Output to standard out.\n");
    exit (1);
}

static void droffInitialise (void)
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
    
    /* Build the copyright/company name */
    if (copyrightName == NULL)
        companyName =  bufFormat (NULL, "(c) Copyright %4d/%02d/%02d.",
                                  year, month, day);
    else
        companyName = bufFormat (NULL, "%s %4d/%02d/%02d.", copyrightName,
                                 year, month, day);
    
    /* Headers */
    nrInstall (funcTab, NH_func, NULL);
    nrInstall (funcTab, FH_func, NULL);
    nrInstall (funcTab, TH_func, nrTH_func);
    nrInstall (funcTab, GH_func, NULL);
    nrInstall (funcTab, SH_func, nrSH_func);
    nrInstall (funcTab, SS_func, nrSS_func);

    /* Identification */
    nrInstall (funcTab, XI_func, NULL);
    nrInstall (funcTab, XJ_func, NULL);
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
    nrInstall (funcTab, Hh_func, nrHh_func);
    nrInstall (funcTab, Ht_func, nrHt_func);
    nrInstall (funcTab, Hr_func, nrHr_func);
    nrInstall (funcTab, Hg_func, nrHg_func);

    /* Bullets */
    nrInstall (funcTab, BS_func, nrBS_func);
    nrInstall (funcTab, BU_func, nrBU_func);
    nrInstall (funcTab, BE_func, nrBE_func);

    /* Indentation */
    nrInstall (funcTab, RS_func, nrRS_func);
    nrInstall (funcTab, RE_func, nrRE_func);

    /* Text adjustment */
    nrInstall (funcTab, ft_func, nrft_func);
    nrInstall (funcTab, CS_func, nrCS_func);
    nrInstall (funcTab, CE_func, nrCE_func);
    nrInstall (funcTab, PD_func, nrPD_func);
    nrInstall (funcTab, na_func, nrna_func);
    nrInstall (funcTab, nf_func, nrnf_func);
    nrInstall (funcTab, fi_func, nrfi_func);
    nrInstall (funcTab, ps_func, NULL);
    nrInstall (funcTab, vs_func, NULL);
    nrInstall (funcTab, ad_func, nrad_func);
    nrInstall (funcTab, br_func, nrbr_func);
    nrInstall (funcTab, bp_func, nrbp_func);
    nrInstall (funcTab, ll_func, nrll_func);
    nrInstall (funcTab, pl_func, nrpl_func);
    nrInstall (funcTab, sp_func, nrsp_func);
    nrInstall (funcTab, ne_func, nrne_func);

    /* Files */
    nrInstall (funcTab, so_func, NULL);
    nrInstall (funcTab, startFile_func, nrStart);
    nrInstall (funcTab, includeFile_func, nrStartInc);
    nrInstall (funcTab, endFile_func, NULL);

    /* Text */
    nrInstall (funcTab, textline_func, nrTextline);

    nrInstallFunctionTable (&funcTab);
}

int main
(int argc, char *argv [])
{
    char    *oname = NULL;
    int     ecount = 0;                 /* Error count */
    int     wcount = 0;                 /* Warn count */
    nrFILE  *nrfp;
    int     c;
    int     i;

    droffInitialise ();
    uErrorSet (0, &ecount);             /* Max Num entries + counter */
    uWarnSet (&wcount);                 /* Warning count */
    uUtilitySet (progname);             /* Set name of program */
    uVerboseSet (0);                    /* Verbose setting */
    uInteractiveSet (0);                /* Disable interaction */

    while (1) {
        c = getopt (argc, argv, "c:e:E:h?Il:o:sapq");
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
        case 'l':
            page_len = atoi (optarg);
            break;
        case 'q':
            quietMode = 1;
            break;
        case 'a':
            asciiMode = 1;
            break;
        case 'o':
            oname = optarg;
            break;
        case 'p':
            plainText = 1;
            break;
        case 'I':
            fprintf (stdout, "%s. Version %s. %s. %s.\n",
                     progname, MODULE_VERSION, __DATE__, nroffVersion);
            exit (0);
            break;
        case 'h':
        case '?':
            usage();
            break;
        default:
            fprintf (stdout, "Cannot understand option [-%c]\n", c);
            usage();
            break;
        }
    }
    
    uOpenErrorChannel ();
    argv = getfiles (&argc, argv, optind);

    if (argc > 1) {
        if (oname != NULL) {
            if ((fo = fopen (oname, "wb")) == NULL)
                uFatal ("Cannot open file [%s]\n", oname);
            uVerbose (1, "Output in file [%s].\n", oname);
            droff_init ();
        }
    }
    else
        fo = NULL;

    /* Process all of the files */

    for (i = 1; i < argc; i++) {
        if ((nrfp = nrFilePush (argv[i])) == NULL)
            exit (1);
        
        /* Break up the file name and add an extension */
        if (oname == NULL && fo == NULL)
        {
            char *drive;
            char *path;
            char *base;
            char *name;
            
            if (splitFilename (nrfp->fileName, &drive, &path, &base, NULL) != 0)
                uFatal ("Cannot decompose filename [%s]\n", nrfp->fileName);
            if ((name = makeFilename (drive, path, base, "nro")) == NULL)
                uFatal ("Cannot compose filename from split [%s]\n",
                        nrfp->fileName);
            if ((fo = fopen (name, "wb")) == NULL)
                uFatal ("Cannot open file %s\n", name);
            droff_init ();              /* Start up if in sections */
        }

        /* Set up the conversion */
        mode = 0;
        prevMode = 0;

        nroff (nrfp, NROFF_MODE_DEFAULT);

        fflush (fo);
        fflush (stdout);
        if ((oname == NULL) && (fo != stdout)) {
            droff_term ();
            droffEof ();
            fclose (fo);
            fo = NULL;
        }
    }
    if (fo != NULL) {
        droff_term ();
        droffEof ();
        fclose (fo);
    }

    uCloseErrorChannel ();
    return (ecount);
}
