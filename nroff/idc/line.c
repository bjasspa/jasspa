/****************************************************************************
 *
 *                      Copyright 1996 Jon Green.
 *                         All Rights Reserved
 *
 *
 *  System        :
 *  Module        :
 *  Object Name   : $RCSfile: line.c,v $
 *  Revision      : $Revision: 1.1 $
 *  Date          : $Date: 2000-10-21 14:31:23 $
 *  Author        : $Author: jon $
 *  Last Modified : <990831.2209>
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
 *  Copyright (c) 1996 Jon Green.
 *
 *  All Rights Reserved.
 *
 * This  Document  may  not, in  whole  or in  part, be  copied,  photocopied,
 * reproduced,  translated,  or  reduced to any  electronic  medium or machine
 * readable form without prior written consent from Jon Green.
 *
 ****************************************************************************/

static const char rcsid[] = "@(#) : $Id: line.c,v 1.1 2000-10-21 14:31:23 jon Exp $";

/* Include Files */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "idc.h"

/* Local definitions */

#define KWF_SAVELAST   0x01             /* Save last command */
#define KWF_TERMINAL   0x02             /* Termination command */
#define KWF_ACCUMULATE 0x04             /* Accumulate lines */
#define KWF_BLKACCUM   0x08             /* Block accumulate lines */
#define KWF_COMMENT    0x10             /* Comment line */

#define CMD_BLANK   0x0000002000        /* Blank line received */
#define CMD_INDENT  0x00000000FF        /* Indent mask */
#define CMD_CODE    0x0000000100        /* Code line received */
#define CMD_BULLET  0x0000000200        /* Bullet line received */
#define CMD_LITERAL 0x0000000400        /* Literal line received */
#define CMD_TITLE   0x0000000800        /* Title line received */
#define CMD_ROMAN   0x0000001000        /* Restore Roman text */

#define ACCUM_RESTART    0x01           /* Reset accumulation buffer */
#define ACCUM_TERMINATE  0x02           /* Terminate the accumulation buffer */
#define ACCUM_RETURN     0x04           /* Return the accumulation buffer */

/* Type definitions */

typedef void (* KeyFunc)(void);         /* Key type table */

typedef struct KeyStruct {
    char    *name;
    int     index;
    int     size;
    KeyFunc func;
    int     flag;
} KeyType;

/* Construct the key table from the definition file. */

#define KEYWORD(x,y,z,f) static void z (void);
#define KEYWORDF(x,y,z,f)   /* NULL */
#define KEYWORD_NULL(x,y,f) /* NULL */
#include    "idc.def"
#undef  KEYWORD_NULL
#undef  KEYWORD
#undef  KEYWORDF

enum
{
#define KEYWORD(x,y,z,f) y,
#define KEYWORDF(x,y,z,f) y,
#define KEYWORD_NULL(x,y,f) y,
#include "idc.def"
#undef  KEYWORD
#undef  KEYWORDF
#undef  KEYWORD_NULL
    KW_MAX
};

static KeyType keyTab[] = {
#define KEYWORD(x,y,z,f)  {x, y, 0, z, f},
#define KEYWORDF(x,y,z,f)  {x, y, 0, z, f},
#define KEYWORD_NULL(x,y,f) {x,y,0,NULL,f},
#include    "idc.def"
#undef  KEYWORD_NULL
#undef  KEYWORD
#undef  KEYWORDF
        {NULL, KW_MAX, 0, NULL, 0}
};

/* Local static definition */

static char *instr = NULL;
static int lastCommand = 0;
static KeyFunc lineFunc = NULL;
static long cmdMode = 0L;
static Args cmdArgs = {NULL, 0};

void *
rpMalloc (int i)
{
    void *p;

    if ((p = malloc (i)) == NULL)
        uFatal ("No memory\n");
    return ((void *)(p));
}

void
rpFree (void *p)
{
    if (p == NULL)
        return;
    free (p);
}

void *
rpRealloc (void *p, int i)
{
    if (p == NULL)
    {
        uWarn ("Reallocing with NULL\n");
        return (rpMalloc (i));
    }
    else if ((p = realloc (p, i)) == NULL)
        uFatal ("No memory\n");
    return (p);
}

rpFILE *
rpOpen (char *fileName)
{
    uDebug (1, ("Opening file [%s]\n", fileName));
    if (rpfp != NULL)
        uFatal ("File [%s] already open in fpOpen request\n",
                rpfp->fileName);
    rpfp = (rpFILE *) rpMalloc (sizeof (rpFILE));
    if ((rpfp->fp = fopen (fileName, "r")) == NULL)
        uFatal ("Cannot open file %s\n", fileName);

    /* For quick reading  increase the buffer size. Make it 64K bytes. */
    setvbuf (rpfp->fp, NULL, _IOFBF, 64*1024);

    rpfp->fileName = bufNStr (NULL, fileName);
    rpfp->lineNo = 0;
    rpfp->line = NULL;
    rpfp->eofFlag = 0;
    uFileSet (&rpfp->lineNo, rpfp->fileName);

    uDebug (2, ("Openned file [%s]\n", fileName));
    return (rpfp);
}

void
rpClose (rpFILE *rfp)
{
    uDebug (1, ("Closing file [%s]\n", rfp->fileName));
    fclose (rfp->fp);
    bufFree (rfp->line);
    bufFree (rfp->fileName);
    uFileSet (NULL, NULL);
    free (rfp);
    rfp = NULL;
}

/*
 * Accumulate functions.
 * These functions accumulate command line text into a buffer.
 */

static char *
strAccumulate (char *s, int mode)
{
    static char *abuf = NULL;

    if (mode & ACCUM_RESTART)
        abuf = bufFree (abuf);
/*    s = skipWhiteSpace (s);*/
/*    s = compWhiteSpace (s);*/
/*    s = trimWhiteSpace (s);*/
    if (*s != '\0')
    {
        if (abuf != NULL)
        {
            abuf = bufStr (abuf, " ");
            abuf = bufStr (abuf, s);
        }
        else
            abuf = bufStr (NULL, s);
    }
    if (abuf == NULL)
        return (bufStr (NULL, ""));
    else if (mode & ACCUM_TERMINATE)
    {
        /* Clean up the string.
         * We copy to ensure we do not loose the malloc header. */

        s = cleanWhiteSpace (abuf);     /* Clean up white space */
        s = bufStr (NULL, s);           /* Duplicate cleaned */
        abuf = bufFree (abuf);          /* Free original */
        return (s);                     /* Return cleaned */
    }
    return (abuf);
}

static char **
blockAccumulate (char *s, int mode)
{
    static char **pbuf = NULL;
    static int  pcount = 0;
    char **sp;

    uDebug (1, ("Block Accumulate [%#04x][%s].\n", mode, s));

    if (mode & ACCUM_RESTART)
        pbuf = NULL;
    if (mode & ACCUM_RETURN)
        return (pbuf);
    if ((s != NULL) && (*s != '\0'))
    {
        if (pbuf == NULL)
        {
            pcount = 1;
            pbuf = malloc (sizeof (char *) * (pcount+1));
            pbuf [0] = s;
        }
        else
        {
            pbuf [pcount++] = s;
            pbuf = realloc (pbuf, sizeof (char *) * (pcount+1));
        }
        pbuf [pcount] = NULL;
    }

    if (mode & ACCUM_TERMINATE)
    {
        sp = pbuf;
        pbuf = NULL;
        pcount = 0;
        return (sp);
    }
    return (pbuf);
}

/*
 * Parse runctions.
 * Parse the command line for a designated line formation.
 */

static int
parseNumber (char **sp, int mode, int lo, int hi, int *result)
{
    int  newNumber = 0;
    char *s;
    char c;

    for (s = *sp; ((c = *s) && ((c >= '0') && (c <= '9'))); s++)
        newNumber = (newNumber * 10) + (int)(c-'0');

    if (*sp == s)
    {
        uDebug (1, ("Failed to parse number\n"));
        return (0);
    }

    *sp = s;

    if (mode != 0)
    {
        if ((lo > hi) && ((newNumber < lo) || (newNumber >= hi)))
        {
            uError ("Error out of bounds. Valid range is %d-%d.\n",
                     lo, hi);
            newNumber = lo;
            return (0);
        }
    }
    if (result)
        *result = newNumber;
    uDebug (1, ("Parsed number correctly [%d]\n", newNumber));
    return (1);
}

static int
parseChar (char **sp, char *string, int mode)
{
    char *ostring = string;
    char c;
    char *s;
    int  status = 0;

    s = skipWhiteSpace (*sp);
    for (c = *s; *string != '\0'; string++)
    {
        if (c == *string)
        {
            status = 1;
            s++;
            break;
        }
    }
    if (status != 0)
        *sp = s;
    else if (mode == 0)
        uError ("Error expected on of [%s]\n", ostring);
    return (status);
}

static char *
parseAlphaNumeric (char **sp)
{
    char *s;
    char *r;
    char c;

    s = skipWhiteSpace (*sp);
    r = NULL;

    while (((c = *s) != '\0') &&
           (((c >= 'a') && (c <= 'z')) ||
            ((c >= 'A') && (c <= 'Z')) ||
            ((c >= '0') && (c <= '9'))))
    {
        r = bufChr (r, c);
        s++;
    }

    *sp = s;
    return (r);
}

#if 0
/* Function no longer required - shut compiler up  !! */
static char *
parseFirstParam (char **sp)
{
    char *s;
    char *r;
    char c;

    s = skipWhiteSpace (*sp);
    r = NULL;

    while (((c = *s) != '\0') &&
           (c != ' ') && (c != '\t'))
    {
        r = bufChr (r, c);
        s++;
    }

    *sp = skipWhiteSpace(s);
    return (r);
}
#endif 

static int
parseString (char **sp, char **strings)
{
    char *s;
    int i;

    s = skipWhiteSpace (*sp);
    for (i = 0, s = *sp; strings[i] != NULL; i++)
    {
        if (strcasecmp (strings [i], s) == 0)
        {
            *sp = s;
            return (i);
        }
    }
    return (-1);
}

static int
parseDate (char **sp, int mode, unsigned long *date)
{
    unsigned long ldate = 0;
    int  day;
    int  month;
    int  year;

    /* Look for a '*' in the date feild - that means today !! */
    if (parseChar (sp, "*", 1) != 0)
    {
        if (date != NULL)
            *date = todaysDate;
        return (1);
    }

    /* Look for DD/MM/YY in the date field. */
    if (parseNumber (sp, mode, 1, 31, &day) &&
        parseChar (sp, ",/-.", 0) &&
        parseNumber (sp, mode, 1, 12, &month) &&
        parseChar (sp, ",/-.", 0) &&
        parseNumber (sp, mode, 94, 99, &year))
    {
        ldate = dateSetDay (ldate, day);
        ldate = dateSetMonth (ldate, month);
        ldate = dateSetYear (ldate, year);
        if (date != NULL)
            *date = ldate;
        return (1);
    }
    return (0);
}

static char *
parseXreference (char **sp, Args *args)
{
    char *name = NULL;
    char *section = NULL;

    if ((name = parseAlphaNumeric (sp)) != NULL)
    {

        if (parseChar (sp, "(", 0) &&
            (section = parseAlphaNumeric (sp)) &&
            parseChar (sp, ")", 0))
        {
            name = bufStr (NULL,name);
            section = bufStr (NULL, section);
            uDebug (2, ("parseXreference [%s](%s)\n", name, section));

            bufArg (args, name);
            bufArg (args, section);
            return (name);
        }
        else
            uError ("Expected <keyword>(<section>)\n");
    }
    return (NULL);
}

/*
 * Callback functions bound to the command words.
 */

static void
name_func (void)
{
    uDebug (1, ("name_func [%s].\n", instr));
    rp_name (instr);
}

static void
swmodule_func (void)
{
    uDebug (1, ("swmodule_func [%s].\n", instr));
    nameAdd (&swmodules, instr, 0);
}

static void
swproject_func (void)
{
    uDebug (1, ("swproject_func [%s].\n", instr));
    if (project != NULL)
        free (project);
    project = strdup (instr);
}

static void
wwwroot_func (void)
{
    uDebug (1, ("wwwroot_func [%s].\n", instr));
    if (wwwroot != NULL)
        free (wwwroot);
    wwwroot = strdup (instr);
}

static void
tekNumber_func (void)
{
    int i;

    uDebug (1, ("tekNumber_func [%s].\n", instr));
    if (parseNumber (&instr, 1, 0, 100, &i) == 0) {
        uError ("Invalid technical note number [%s]\n", instr);
        i = 0;
    }
    rp_tekNumber (i);
}

static void
releaseNo_func (void)
{
    int i;

    uDebug (1, ("releaseNo_func [%s].\n", instr));
    if (parseNumber (&instr, 1, 0, 1000, &i) == 0) {
        uError ("Invalid release number [%s]\n", instr);
        i = 0;
    }
    rp_releaseNumber (i);
}

static void
patchNo_func (void)
{
    int i;

    uDebug (1, ("patchNo_func [%s].\n", instr));
    if (parseNumber (&instr, 1, 0, 1000, &i) == 0) {
        uError ("Invalid patch number [%s]\n", instr);
        i = 0;
    }
    rp_patchNumber (i);
}

static void
newsNumber_func (void)
{
    int i;

    uDebug (1, ("newsNumber_func [%s].\n", instr));
    if (parseNumber (&instr, 1, 0, 1000, &i) == 0) {
        uError ("Invalid news number [%s]\n", instr);
        i = 0;
    }
    rp_newNumber (i);
}

static void
infoNumber_func (void)
{
    int i;

    uDebug (1, ("infoNumber_func [%s].\n", instr));
    if (parseNumber (&instr, 1, 0, 1000, &i) == 0) {
        uError ("Invalid info number [%s]\n", instr);
        i = 0;
    }
    rp_infNumber (i);
}

static void
bugNumber_func (void)
{
    int i;

    uDebug (1, ("bugNumber_func [%s].\n", instr));
    if (parseNumber (&instr, 1, 1, 10000, &i) == 0) {
        uError ("Invalid bug number [%s]\n", instr);
        i = 0;
    }
    rp_bugNumber (i);
}

static void
faqNo_func (void)
{
    int i;

    uDebug (1, ("faqNo_func [%s].\n", instr));
    if (parseNumber (&instr, 1, 0, 1000, &i) == 0) {
        uError ("Invalid frequently asked question number [%s]\n", instr);
        i = 0;
    }
    rp_faqNumber (i);
}

static void
localId_func (void)
{
    uDebug (1, ("localId_func [%s].\n", instr));
    rp_localId (instr);
}

static void
end_func (void)
{
    uDebug (1, ("Doing end func\n"));
    rp_end ();
}

static void
date_func (void)
{
    unsigned long date = 0;

    uDebug (1, ("date_func [%s].\n", instr));
    if (parseDate (&instr, 1, &date) == 0)
        uError ("Date string expected. dd/mm/yy\n");
    rp_date (date);
}

static void
module_func (void)
{
    uDebug (1, ("module_func [%s].\n", instr));
    rp_module (nameFindAdd (&swmodules, instr, NAME_REPORTED));
}

static void
component_func (void)
{
    uDebug (1, ("component_func [%s].\n", instr));
    rp_component (instr);
}

static void
notes_func (void)
{
    uDebug (1, ("notes_func [%s].\n", instr));
    if (rp_notes (cmdArgs.argv, cmdArgs.argc) == 0)
        bufArgFree (&cmdArgs);
}

static void
seeAlso_func (void)
{
    Args seeAlsoArgs = {NULL, 0};
    int  i;

    uDebug (1, ("seeAlso_func [%s].\n", instr));
    bufArg (&seeAlsoArgs, NULL);
    while (parseXreference (&instr, &seeAlsoArgs) != NULL)
        parseChar (&instr, ",.", 1);

    for (i = 0; i < seeAlsoArgs.argc; i+= 2)
        uDebug (1, ("seeAlso_func %d. [%s(%s)].\n",
                     i, seeAlsoArgs.argv [i], seeAlsoArgs.argv [i+1]));

    if (rp_seeAlso (seeAlsoArgs.argv, seeAlsoArgs.argc) == 0)
        bufArgFree (&seeAlsoArgs);
}

static void
patch_func (void)
{
    int i;

    uDebug (1, ("patch_func [%s].\n", instr));

    if (parseNumber (&instr, 1, 1, 1000, &i) == 0) {
        uError ("Invalid patch number [%s]\n", instr);
        i = 0;
    }
    rp_patch (i);
}

static void
release_func (void)
{
    Name *n;

    uDebug (1, ("release_func [%s].\n", instr));

    if ((n = nameFindNode (releaseNames, instr)) == NULL)
        uError ("Invalid release name [%s]\n", instr);
    rp_release (n);
}

static void
releaseState_func (void)
{
    static char *releaseStateStrings [] =
    {
        "Full", "Specified", "Patched", NULL
    };

    int i;

    uDebug (1, ("releaseState_func [%s].\n", instr));
    if ((i = parseString (&instr, releaseStateStrings)) < 0)
        uError ("Invalid release status string [%s]\n", instr);
    rp_releaseState (i+1);
}

static void
reportBy_func (void)
{
    uDebug (1, ("reportBy_func [%s].\n", instr));
    rp_reportBy (nameFindAdd (&names, instr, NAME_REPORTED));
}

static void
reportFor_func (void)
{
    uDebug (1, ("reportFor_func [%s].\n", instr));
    rp_reportFor (instr);
}

static void
status_func (void)
{
    static char *statusStrings [] =
    {
        "Open", "Suspended", "Closed", "Rejected", NULL
    };

    int i;

    uDebug (1, ("status_func [%s].\n", instr));
    if ((i = parseString (&instr, statusStrings)) < 0)
        uError ("Invalid Status string [%s]\n", instr);
    rp_status (i+1);
}

static void
engineer_func (void)
{
    uDebug (1, ("engineer_func [%s].\n", instr));
    rp_engineer (nameFindAdd (&names, instr, NAME_ENGINEER));
}

static void
severity_func (void)
{
    static char *severityStrings [] =
    {
        "A",  "B",  "C",  "D",  "E",
        "#A", "#B", "#C", "#D", "#E"
    };

    int i;

    uDebug (1, ("severity_func [%s].\n", instr));
    if ((i = parseString (&instr, severityStrings)) < 0)
        uError ("Invalid Severity string [%s]\n", instr);
    rp_severity (i+1);
}

/*
 * Handle the summary field.
 */

static void
summary_func (void)
{
    uDebug (1, ("summary_func [%s].\n", instr));
    rp_summary (instr);
}

static void
system_func (void)
{
    uDebug (1, ("system_func [%s].\n", instr));
    rp_system (instr);
}

static void
systemSpec_func (void)
{
    uDebug (1, ("systemSpec_func [%s].\n", instr));
    rp_systemSpec (instr);
}

static void
fixedBy_func (void)
{
    uDebug (1, ("fixedBy_func [%s].\n", instr));
    rp_fixedBy (nameFindAdd (&names, instr, NAME_FIXED));
}

static void
fixedDate_func (void)
{
    unsigned long date = 0;
    Name *n;

    uDebug (1, ("fixedDate_func [%s].\n", instr));

    if ((instr != NULL) && (*instr != '\0') &&
        (parseDate (&instr, 0, &date) == 0)) {
        if ((n = nameFindNode (releaseNames, instr)) == NULL)
            uError ("Release name or date expected. Got [%s]\n", instr);
        else
            rp_fixedDate (n->mode, n);
    }
    else
        rp_fixedDate (date, NULL);
}

static void
fixedDescription_func (void)
{
    uDebug (1, ("fixedDescription_func [%s].\n", instr));
    if (rp_fixedDescription (cmdArgs.argv, cmdArgs.argc) == 0)
        bufArgFree (&cmdArgs);
}

static void
affecting_func (void)
{
    uDebug (1, ("affecting_func [%s].\n", instr));
    if (rp_affecting (cmdArgs.argv, cmdArgs.argc) == 0)
        bufArgFree (&cmdArgs);
}

static void
suspendBy_func (void)
{
    uDebug (1, ("suspendBy_func [%s].\n", instr));
    rp_suspendedBy (nameFindAdd (&names, instr, NAME_SUSPENDED));
}

static void
unsuspendBy_func (void)
{
    uDebug (1, ("unsuspendBy_func [%s].\n", instr));
    rp_unsuspendedBy (nameFindAdd (&names, instr, NAME_SUSPENDED));
}

static void
suspendDate_func (void)
{
    unsigned long date = 0;

    uDebug (1, ("suspendDate_func [%s].\n", instr));
    parseDate (&instr, 0, &date);
    rp_suspendedDate (date);
}

static void
unsuspendDate_func (void)
{
    unsigned long date = 0;

    uDebug (1, ("unsuspendDate_func [%s].\n", instr));
    parseDate (&instr, 0, &date);
    rp_unsuspendedDate (date);
}

static void
suspendDescription_func (void)
{
    uDebug (1, ("suspendDescription_func [%s].\n", instr));
    if (rp_suspendedDesc (cmdArgs.argv, cmdArgs.argc) == 0)
        bufArgFree (&cmdArgs);
}

static void
workaround_func (void)
{
    uDebug (1, ("workaround_func [%s].\n"));
    if (rp_workaround (cmdArgs.argv, cmdArgs.argc) == 0)
        bufArgFree (&cmdArgs);
}

static void
description_func (void)
{
    uDebug (1, ("description_func exit.\n"));
    if (rp_description (cmdArgs.argv, cmdArgs.argc) == 0)
        bufArgFree (&cmdArgs);
}

/*
 * Functions to handle the description field.
 */

static void
blkAccumulator_lfunc (void)
{
    char *s;

    uDebug (1, ("blkAccumulator_lfunc [%s].\n", instr));
    if (cmdMode & CMD_CODE)
    {
        if (*instr == '\0')
            instr = " ";
        blockAccumulate (bufStr (NULL, instr), 0);
    }
    else if (cmdMode & CMD_LITERAL)
    {
        s = trimWhiteSpace(instr);
        if (*s == '\0')
            instr = " ";
#if 0
#ifndef JDGADDED
        blockAccumulate (bufStr (NULL, instr), 0);
#endif
#endif
        s = strAccumulate (s, ACCUM_TERMINATE);
        blockAccumulate (s, 0);
    }
    else
    {
        /*
         * We are processing a regular line. If we have just detected
         * a font change then if the next line is not blank then add
         * flag as a change in font.
         */

        s = trimWhiteSpace(instr);
        if (*s == '\0')
        {
            s = strAccumulate (s, ACCUM_TERMINATE);
            blockAccumulate (s, 0);
        }
        else if (cmdMode & CMD_ROMAN)
        {
            s = bufFormat (NULL, "%c%s", CHAR_FORMAT_ROMAN, s);
            strAccumulate (s, 0);
        }
        else
            strAccumulate (s, 0);
    }
    cmdMode &= ~CMD_ROMAN;              /* Remove font change notifier */
}

static void
blkAccumulator_func (void)
{
    char *s;

    uDebug (1, ("blkAccumulator_func [%s].\n", instr));
    s = strAccumulate (instr, ACCUM_RESTART);
    lineFunc = blkAccumulator_lfunc;
}

static void
blkAccumulator_tfunc (void)
{
    char **sp;
    char *s;
    int i;

    uDebug (1, ("blkAccumulator_tfunc [%s].\n", instr));

    s = strAccumulate (instr, ACCUM_TERMINATE);
    sp = blockAccumulate (s, ACCUM_TERMINATE);
    if (sp == NULL)
    {
        uDebug (1,("Line is NULL\n"));
        i = 0;
    }
    else
    {
        for (i = 0; sp [i] != NULL; i++)
            uDebug (1,("Para %d: [%s]\n", i, sp[i]));
    }
    uDebug (1, ("blkAccumulator_tfunc exit.\n"));
    lineFunc = NULL;
    cmdMode &= ~CMD_ROMAN;              /* Remove font change notifier */

    /* Set up the parameters for return */
    cmdArgs.argv = sp;
    cmdArgs.argc = i;
}

static void
indent_func (void)
{
    long i;
    char *s;

    if ((i = (cmdMode & CMD_INDENT)) == 0xff)
        uFatal ("Too many indents %d\n", i);
    cmdMode = (cmdMode & ~CMD_INDENT)|(i + 1);
    uDebug (1, ("indent_func\n"));
    /*
     * Terminate current block.
     * Add indent marker.
     */
    s = strAccumulate ("\0", ACCUM_TERMINATE);
    blockAccumulate (s, 0);
    s = bufFormat (NULL, "%c", CHAR_FORMAT_INDENT);
    blockAccumulate (s, 0);
}

static void
eindent_func (void)
{
    long i;
    char *s;

    uDebug (1, ("eindent_func\n"));

    if ((i = (cmdMode & CMD_INDENT)) == 0)
        uFatal ("Too many 'eindent' invocations\n");
    else
        cmdMode = (cmdMode & ~(CMD_INDENT|CMD_ROMAN))|--i;
    /*
     * Terminate current block.
     * Add unindent marker.
     */
    s = strAccumulate ("\0", ACCUM_TERMINATE);
    blockAccumulate (s, 0);
    s = bufFormat (NULL, "%c", CHAR_FORMAT_EINDENT);
    blockAccumulate (s, 0);
}

static void
code_func (void)
{
    char *s;
    uDebug (1, ("code_func\n"));

    if ((cmdMode & CMD_CODE) != 0)
        uError ("nested 'code' encountered\n");
    else if ((cmdMode & CMD_BULLET) != 0)
        uError ("'code' section requested in 'bullet'\n");
    else if ((cmdMode & CMD_LITERAL) != 0)
        uError ("'code' section requested in 'literal'\n");
    else
        cmdMode |= CMD_CODE;
    /*
     * Terminate current block.
     * Add code marker.
     */
    s = strAccumulate ("\0", ACCUM_TERMINATE);
    blockAccumulate (s, 0);
    s = bufFormat (NULL, "%c", CHAR_FORMAT_CODE);
    blockAccumulate (s, 0);
}

static void
ecode_func (void)
{
    char *s;
    uDebug (1, ("ecode_func\n"));

    if ((cmdMode & CMD_CODE) == 0)
        uError ("No openning 'code' encountered\n");
    else
        cmdMode &= ~(CMD_CODE|CMD_ROMAN);
    /*
     * Terminate current block.
     * Add code terminal marker.
     */
    s = strAccumulate ("\0", ACCUM_TERMINATE);
    blockAccumulate (s, 0);
    s = bufFormat (NULL, "%c", CHAR_FORMAT_ECODE);
    blockAccumulate (s, 0);

    /*
     * Normalise the block of code.
     */
}

static void
bullet_func (void)
{
    char *s;
    if ((cmdMode & CMD_BULLET) != 0)
        uError ("nested 'bullet' encountered\n");
    else
        cmdMode |= CMD_BULLET;
    uDebug (1, ("bullet_func\n"));
    /*
     * Terminate current block.
     * Add bullet marker.
     */
    s = strAccumulate ("\0", ACCUM_TERMINATE);
    blockAccumulate (s, 0);
    s = bufFormat (NULL, "%c", CHAR_FORMAT_BULLET);
    blockAccumulate (s, 0);
}

static void
ebullet_func (void)
{
    char *s;
    uDebug (1, ("ebullet_func\n"));

    if ((cmdMode & CMD_BULLET) == 0)
        uError ("No openning 'bullet' encountered\n");
    else
        cmdMode &= ~(CMD_BULLET|CMD_ROMAN);
    /*
     * Terminate current block.
     * Add bullet termination marker.
     */
    s = strAccumulate ("\0", ACCUM_TERMINATE);
    blockAccumulate (s, 0);
    s = bufFormat (NULL, "%c", CHAR_FORMAT_EBULLET);
    blockAccumulate (s, 0);
}

static void
literal_func (void)
{
    char *s;
    uDebug (1, ("literal_func\n"));

    if ((cmdMode & CMD_LITERAL) != 0)
        uError ("nested 'literal' encountered\n");
    else
        cmdMode |= CMD_LITERAL;
    /*
     * Terminate current block.
     * Add literal marker.
     */
    s = strAccumulate ("\0", ACCUM_TERMINATE);
    blockAccumulate (s, 0);
    s = bufFormat (NULL, "%c", CHAR_FORMAT_LITERAL);
    blockAccumulate (s, 0);
}

static void
eliteral_func (void)
{
    char *s;
    uDebug (1, ("eliteral_func\n"));

    if ((cmdMode & CMD_LITERAL) == 0)
        uError ("No openning 'literal' encountered\n");
    else
        cmdMode &= ~(CMD_LITERAL|CMD_ROMAN);
    /*
     * Terminate current block.
     * Add literal termination marker.
     */
    s = strAccumulate ("\0", ACCUM_TERMINATE);
    blockAccumulate (s, 0);
    s = bufFormat (NULL, "%c", CHAR_FORMAT_ELITERAL);
    blockAccumulate (s, 0);
}

static void
title_func (void)
{
    char *s;
    uDebug (1, ("title_func\n"));
    if ((cmdMode & CMD_TITLE) != 0)
        uError ("nested 'title' encountered\n");
    else
        cmdMode |= CMD_TITLE;
    /*
     * Terminate the current block.
     * Add title marker.
     */
    s = strAccumulate ("\0", ACCUM_TERMINATE);
    blockAccumulate (s, 0);
    s = bufFormat (NULL, "%c", CHAR_FORMAT_TITLE);
    blockAccumulate (s, 0);
}

static void
etitle_func (void)
{
    char *s;
    uDebug (1, ("etitle_func\n"));
    if ((cmdMode & CMD_TITLE) == 0)
        uError ("No openning 'title' encountered\n");
    else
        cmdMode &= ~CMD_TITLE;
    /*
     * Terminate the current block.
     * Add title marker.
     */
    s = strAccumulate ("\0", ACCUM_TERMINATE);
    blockAccumulate (s, 0);
    s = bufFormat (NULL, "%c", CHAR_FORMAT_ETITLE);
    blockAccumulate (s, 0);
}

static void
ftp_func (void)
{
    char *s;
    int  paragraphType = CHAR_FORMAT_FTP;

    uDebug (1, ("ftp_func [%s]\n", instr));

    cmdMode |= CMD_ROMAN;               /* Mark as local font change */
    if ((cmdMode & (CMD_CODE|CMD_TITLE)) != 0)
        uError ("Illegal enclosure for a command\n");
    if (cmdMode & CMD_BLANK)/* && ((cmdMode & (CMD_BULLET|CMD_LITERAL)) == 0))*/
        paragraphType += 1;

    s = strAccumulate ("\0", ACCUM_TERMINATE);
    blockAccumulate (s, 0);
    s = bufFormat (NULL, "%c%s", paragraphType, instr);
    blockAccumulate (s, 0);
}

static void
http_func (void)
{
    char *s;
    int  paragraphType = CHAR_FORMAT_HTTP;

    uDebug (1, ("http_func [%s]\n", instr));

    cmdMode |= CMD_ROMAN;               /* Mark as local font change */
    if ((cmdMode & (CMD_CODE|CMD_TITLE)) != 0)
        uError ("Illegal enclosure for a command\n");
    if (cmdMode & CMD_BLANK) /* && ((cmdMode & (CMD_BULLET|CMD_LITERAL)) == 0))*/
        paragraphType += 1;

    s = strAccumulate ("\0", ACCUM_TERMINATE);
    blockAccumulate (s, 0);
    s = bufFormat (NULL, "%c%s", paragraphType, instr);
    blockAccumulate (s, 0);
}


static void
mailto_func (void)
{
    char *s;
    int  paragraphType = CHAR_FORMAT_MAILTO;

    uDebug (1, ("mailto_func [%s]\n", instr));

    cmdMode |= CMD_ROMAN;               /* Mark as local font change */
    if ((cmdMode & (CMD_CODE|CMD_TITLE)) != 0)
        uError ("Illegal enclosure for a command\n");
    if (cmdMode & CMD_BLANK)/* && ((cmdMode & (CMD_BULLET|CMD_LITERAL)) == 0))*/
        paragraphType += 1;

    s = strAccumulate ("\0", ACCUM_TERMINATE);
    blockAccumulate (s, 0);
    s = bufFormat (NULL, "%c%s", paragraphType, instr);
    blockAccumulate (s, 0);
}

static void
file_func (void)
{
    char *s;
    int  paragraphType = CHAR_FORMAT_FILE;

    uDebug (1, ("file_func [%s]\n", instr));

    cmdMode |= CMD_ROMAN;               /* Mark as local font change */
    if ((cmdMode & (CMD_CODE|CMD_TITLE)) != 0)
        uError ("Illegal enclosure for a command\n");
    if (cmdMode & CMD_BLANK)/* && ((cmdMode & (CMD_LITERAL|CMD_BULLET)) == 0))*/
        paragraphType += 1;

    s = strAccumulate ("\0", ACCUM_TERMINATE);
    blockAccumulate (s, 0);
    s = bufFormat (NULL, "%c%s", paragraphType, instr);
    blockAccumulate (s, 0);
}


static void
bold_func (void)
{
    char *s;
    int  paragraphType = CHAR_FORMAT_BOLD;

    uDebug (1, ("bold_func [%s]. Blank = %s. Lit/Bullet = %s.\n", instr,
                ((cmdMode & CMD_BLANK) != 0) ? "TRUE" : "FALSE",
                ((cmdMode & CMD_BLANK) != 0) ? "TRUE" : "FALSE"));

    cmdMode |= CMD_ROMAN;               /* Mark as local font change */
    if ((cmdMode & (CMD_CODE|CMD_TITLE)) != 0)
        uError ("Illegal enclosure for a command\n");
    if (cmdMode & CMD_BLANK)/* && ((cmdMode & (CMD_LITERAL|CMD_BULLET)) == 0))*/
        paragraphType += 1;

    s = strAccumulate ("\0", ACCUM_TERMINATE);
    blockAccumulate (s, 0);
    s = bufFormat (NULL, "%c%s", paragraphType, instr);
    blockAccumulate (s, 0);
}


static void
italic_func (void)
{
    char *s;
    int  paragraphType = CHAR_FORMAT_ITALIC;

    uDebug (1, ("italic_func [%s]\n", instr));

    cmdMode |= CMD_ROMAN;               /* Mark as local font change */
    if ((cmdMode & (CMD_CODE|CMD_TITLE)) != 0)
        uError ("Illegal enclosure for a command\n");
    if (cmdMode & CMD_BLANK)/* && ((cmdMode & (CMD_LITERAL|CMD_BULLET)) == 0))*/
        paragraphType += 1;

    s = strAccumulate ("\0", ACCUM_TERMINATE);
    blockAccumulate (s, 0);
    s = bufFormat (NULL, "%c%s", paragraphType, instr);
    blockAccumulate (s, 0);
}


static void
mono_func (void)
{
    char *s;
    int  paragraphType = CHAR_FORMAT_MONO;

    uDebug (1, ("mono_func [%s]\n", instr));

    cmdMode |= CMD_ROMAN;               /* Mark as local font change */
    if ((cmdMode & (CMD_CODE|CMD_TITLE)) != 0)
        uError ("Illegal enclosure for a command\n");
    if (cmdMode & CMD_BLANK)/* && ((cmdMode & (CMD_LITERAL|CMD_BULLET)) == 0))*/
        paragraphType += 1;

    s = strAccumulate ("\0", ACCUM_TERMINATE);
    blockAccumulate (s, 0);
    s = bufFormat (NULL, "%c%s", paragraphType, instr);
    blockAccumulate (s, 0);
}


static void
checkCmdMode (void)
{
    if ((cmdMode & ~CMD_BLANK) == 0)
        return;
    if ((cmdMode & CMD_LITERAL) != 0)
        uError ("Unterminated 'literal' statement\n");
    if ((cmdMode & CMD_TITLE) != 0)
        uError ("Unterminated 'title' statement\n");
    if ((cmdMode & CMD_BULLET) != 0)
        uError ("Unterminated 'bullet' statement\n");
    if ((cmdMode & CMD_CODE) != 0)
        uError ("Unterminated 'code' statement\n");
    if ((cmdMode & CMD_INDENT) != 0)
        uError ("Unterminated 'indent' statement\n");
    cmdMode = 0;
}


void
putLine (char *buffer)
{
    rpfp->line = bufNStr (NULL, buffer);
}

int
getLine (char *buffer)
{
    int c;
    int len = 0;
    FILE *fp;
    int tooLong = 0;

    fp = rpfp->fp;                      /* Cache copy */
    if (rpfp->line != NULL)
    {
        strcpy (buffer, rpfp->line);
        rpfp->line = bufFree (rpfp->line);
        return (rpfp->eofFlag);
    }

    while ((c = fgetc (fp)) != EOF)
    {
        if (c == '\n')
            break;
        if ((c == '\r') || (c == 0x0C))
            continue;
        if (c == '\t') {
            do
            {
                *buffer++ = ' ';
                len++;
            } while ((len % 8) != 0);
            continue;
        }

        /* Check for illegals */
        if (c >= 127) {
            uWarn ("Illegal character [%#04x]. Replacing with ' '.\n",
                   ((int)(c)) & 0x00ff);
            c = ' ';
        }

        if (len < (READLINELEN-1))
        {
            *buffer++ = c;
            len++;
        }
        else if (tooLong == 0)
        {
            uWarn ("Line too long - truncating\n");
            tooLong = 1;
        }
    }
    rpfp->lineNo++;
    *buffer = '\0';
    return (rpfp->eofFlag = (c != EOF));
}

static int
isCommandForm (char *p)
{
    int i;
    char c;
    int space_count = 0;

    c = *p++;
    if (((c >= 'a') && (c <= 'z')) ||
        ((c >= 'A') && (c <= 'Z')))
    {
        i = 1;
        while (((c = *p++) != 0) &&
               (((c >= 'a') && (c <= 'z')) ||
                ((c >= 'A') && (c <= 'Z')) ||
                ((c >= '1') && (c <= '9')) ||
                (c == ' ')))
        {
            if (c == ' ')
                space_count++;
            i++;
        }

        if ((c == ':') && (space_count < 2))
        {
            uDebug (31, ("Command Form Located\n"));
            return (i+1);
        }
    }
    else if ((c == '#') && (*p == ':'))
    {
        uDebug (31, ("Command Form Located\n"));
        return (2);
    }

    uDebug (31, ("Command Form Not Found\n"));
    return (0);
}

static void
accumulator_lfunc (void)
{
    uDebug (1, ("accumulator_lfunc [%s].\n", instr));
    strAccumulate (instr, 0);
}

static void
accumulator_func (void)
{
    uDebug (1, ("accumulator_func [%s].\n", instr));
    strAccumulate (instr, ACCUM_RESTART);
    lineFunc = accumulator_lfunc;
}

static char *
accumulator_tfunc (void)
{
    char *s;
    uDebug (1, ("accumulator_tfunc [%s].\n", instr));
    s = strAccumulate (instr, ACCUM_TERMINATE);
    uDebug (1, ("accumulator_tfunc - processed [%s].\n", s));
    lineFunc = NULL;
    return (s);
}


int
reportPackager (rpFILE *fp)
{
    static KeyFunc termFunc = NULL;
    static KeyFunc accuFunc = NULL;
    static KeyFunc blkAccuFunc = NULL;
    static int setup = 0;

    KeyType *kp;
    char inLine [READLINELEN];
    int  cmdLen;
    int  status;

    uDebug (1, ("In reportPackager\n"));
    if (setup == 0)
    {
        for (kp = keyTab; kp->name != NULL; kp++)
            kp->size = strlen (kp->name);
        setup = 1;
    }

    do
    {
        status = getLine (inLine);
        uDebug (1, ("Command Line [%s]\n", inLine));
        if ((cmdLen = isCommandForm (inLine)) != 0)
        {
            for (kp = keyTab; kp->name != NULL; kp++)
            {
                if (cmdLen != kp->size)
                    continue;
                if (strncasecmp (kp->name, inLine, kp->size) == 0)
                {
                    if (kp->flag & KWF_COMMENT)
                        break;
                    /*
                     * Call the termination function
                     */
                    if (kp->flag & KWF_TERMINAL)
                    {
                        instr = "\0";
                        if (termFunc != NULL)
                        {
                            (termFunc)();
                            termFunc = NULL;
                        }
                        else if (accuFunc != NULL)
                        {
                            char *s;

                            s = accumulator_tfunc ();

                            instr = s;
                            (accuFunc)();
                            accuFunc = NULL;
                            bufFree (s);
                        }
                        else if (blkAccuFunc != NULL)
                        {
                            blkAccumulator_tfunc ();
                            (blkAccuFunc)();
                            blkAccuFunc = NULL;
                        }
                        checkCmdMode ();
                    }

                    /* Got a match - process it */
                    instr = skipWhiteSpace (&inLine [kp->size]);
                    instr = trimWhiteSpace (instr);

                    /*
                     * Save the last context.
                     */

                    if (kp->flag & KWF_SAVELAST)
                        lastCommand = kp->index;
                    if (kp->flag & KWF_ACCUMULATE)
                    {
                        accuFunc = kp->func;
                        accumulator_func ();
                        uDebug (1, ("Clear Blank # 2\n"));
                        cmdMode &= ~CMD_BLANK;
                        break;
                    }
                    if (kp->flag & KWF_BLKACCUM)
                    {
                        blkAccuFunc = kp->func;
                        blkAccumulator_func ();
                        uDebug (1, ("Clear Blank #3\n"));
                        cmdMode &= ~CMD_BLANK;
                        break;
                    }

                    /*
                     * Call the function.
                     */
                    (kp->func)();
                    uDebug (1, ("Clear Blank - Call function #4\n"));
                    cmdMode &= ~CMD_BLANK;
                    break;
                }
            }
            if (kp->name == NULL)
                uError ("Unrecognised command line [%s]\n",
                        inLine);
            uDebug (1, ("#5 Clear  CMD_BLANK\n"));
            cmdMode &= ~CMD_BLANK;
        }
        else if (lineFunc)
        {
            instr = trimWhiteSpace (inLine);
            if (instr[0] == '\0') {
                uDebug (1, ("Set Blank\n"));
                cmdMode |= CMD_BLANK;
            }
            else {
                uDebug (1, ("Clear Blank #6 \n"));
                cmdMode &= ~CMD_BLANK;
            }
            (lineFunc)();
        }
    } while (status);
    return (1);
}
