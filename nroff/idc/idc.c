/****************************************************************************
 *
 *  			Copyright 1996-2004 Jon Green.
 *                         All Rights Reserved
 *
 *
 *  System        : 
 *  Module        : 
 *  Object Name   : $RCSfile: idc.c,v $
 *  Revision      : $Revision: 1.2 $
 *  Date          : $Date: 2004-01-06 00:53:09 $
 *  Author        : $Author: jon $
 *  Last Modified : <040104.0006>
 *
 *  Description	
 *
 *  Notes
 *
 *  History
 *	
 * Version 2.2.0a - 03/01/04 - JG
 * Ported to Sun Solaris 9.
 * 
 * Version 2.2.0 - 03/09/96 - JG
 * Added new hml in-line text formatting extensions
 * Removed "Base Address:" construct.
 * Removed "eftp:" construct - "ftp:" is now a line item not a 
 * group item.
 *
 * Version 2.1.1 - 31/08/96 - JG
 * Added title option. 
 * 
 * Version 2.0.2g - 27/01/96 - JG
 * Increased error reporting. Return exit status on error count.
 * Fixed the "reportFor" field being on same line - added bullet.
 * 
 * Version 2.0.2f - 05/01/96 - JG
 * Added home page for external link to extenal module.
 * 
 * Version 2.0.2e - 16/11/95 - JG
 * RTF needs to generate smaller files. Create RTF for each I/P file.
 * 
 * Version 2.0.2d - 07/11/95 - JG
 * Removed indent for bulleted text in the bug synopsis section.
 *
 * Version 2.0.2c - 05/11/95 - JG
 * Added RTF conversion capability.
 * Renamed utility idc - Information Database Compiler.
 *
 * Version 2.0.2b - 03/11/95 - JG
 * Getting rid of HTM specific fields.
 *
 * Version 2.0.2a - 02/11/95 - JG
 * Corrected crashing problem. Renamed pages for consistency.
 *
 * Version 2.0.2 - 2/11/95 - JG
 * Added bug sub-menu. Moved all bug info from
 * bug2html.c to bug.c - tidied menu page naming for consistency.
 *
 * Version 2.0.1d - 29/10/95 - JG
 * Added news/infomation referencing.
 *
 * Version 2.0.1c - 27/10/95 - JG
 * Added home page referencing.
 * *
 * Version 2.1.0a - 29/08/96 - JG
 * Added UNIX filename handling routines.
 * 
 * Version 2.1.0 - 12/03/96 - JG
 * Added new error reporting.
 * 
 * Version 2.0.0 - 14/10/95 - JG
 * Added patch information.
 *
 * Version 1.0.0 - 10/10/95 - JG
 * Original Version.
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
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <time.h>

#include "idc.h"

#if ((defined _HPUX) || (defined _LINUX) || (defined _SUNOS))
#include <unistd.h>
#else
#include <getopt.h>
#endif

#define MODULE_VERSION  "2.2.0a"
#define MODULE_NAME     "idc"

char *progname = MODULE_NAME;           /* Name of the progrm */
rpFILE *rpfp = NULL;                    /* File pointer */

static Bug *bug = NULL;                 /* Current Bug */
static Tek *tek = NULL;                 /* Current technical note */
static Faq *faq = NULL;                 /* Current Freq. Asked Question */
static New *new = NULL;                 /* Current news item */
static Inf *inf = NULL;                 /* Current Information page */
static Release *release = NULL;         /* Current release information */
static Patch *patch = NULL;             /* Current patch information */
char *project = NULL;                   /* Name of the project */
char *wwwroot = NULL;                   /* Name of the project */

static char *mainTitle = NULL;          /* Main title */
static char *buildTime = NULL;          /* Time system built */
static int  opType = HML_FORMAT_HTML;   /* Output type */

unsigned long todaysDate = 0;           /* Todays date */

/* Constant string definitions */

/*
 * strDate ()
 * Convert a date word to long form of date e.g. "25th December 1995"
 */

char *
strDate (unsigned long date)
{
    static char buffer [30];
    static char *ths [] =
          {"th","st","nd","rd"};
    static char *months [] =
          {"January","February","March","April","May","June","July",
           "August","September", "October", "November", "December" };
    static char thLookup [] =
          {1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,3,0,0,0,0,0,0,0,1};

    int day, month, year;

    if (date == 0)
        return ("Date undefined");

    year = dateGetYear (date);
    month = dateGetMonth (date);
    day = dateGetDay (date);
    sprintf (buffer, "%d%s %s %4d",
             day,
             ths [(int)(thLookup[day-1])],
             months [month-1],
             year+1900);
    return (buffer);
}

/*
 * strShortDate.
 * Convert date to short form e.g. "Dec 25"
 */
char *
strShortDate (unsigned long date)
{
    static char buffer [30];
    static char *months [] =
          {"Jan","Feb","Mar","Apr","May","Jun","Jul",
           "Aug","Sep", "Oct", "Nov", "Dec" };

    int day, month;

    if (date == 0)
        return ("Date undefined");

    month = dateGetMonth (date);
    day = dateGetDay (date);
    sprintf (buffer, "%s %d", months [month-1], day);
    return (buffer);
}

/*
 * outputItemCount (i,j)
 * Display count of items.
 */

void
outputItemCount (int i, int j)
{
    hmlPara ();
    if (j == -1)
        hmlFormatStr ("%d item(s) reported.", i);
    else
        hmlFormatStr ("%d item(s) reported. [%d Active; %d Closed].",
                      i, j, i - j);
}

static char *
aboutIndex (void)
{
    static char *title = "About";
    
    hmlFileHeader (title, NULL);
    hmlIndent (1);
    hmlPara ();
    hmlBullet ("Pages compiled on: %s", buildTime);
    hmlPara ();
    hmlBullet ("Generated by: %s", progname);
    hmlBullet ("Version: %s", MODULE_VERSION);
    hmlBullet ("Built on: %s", __DATE__);
    hmlPara ();
    hmlBullet ("Component: %s", hmlVersion ());
    hmlBullet ("Component: %s", uVersion ());
    hmlIndent (-1);
    hmlEnd ();
    
    return (title);
}
    
static void
rp_bugOutputBugs ()
{
    typedef char *charp;
    charp buffer [20];
    charp *p;

    p = buffer;
    *p++ = bugIndex ();
    *p++ = releaseIndex ();
    *p++ = patchIndex ();
    *p++ = tekIndex ();
    *p++ = faqIndex ();
    *p++ = newIndex ();
    *p++ = infIndex ();
    *p++ = aboutIndex ();
    *p++ = NULL;

    /* Output the main menu */
    hmlFileHeader (mainTitle, "buglist");
    hmlDefinition ("home");
    hmlIndent (1);
    hmlPara ();
    hmlFont (FONT_BOLD);
    hmlFormatStr ("FOR INTERNAL USE ONLY.");
    hmlFont (FONT_NORMAL);
    hmlBreak ();
    hmlFormatStr ("Generated on %s.", buildTime);
    hmlPara ();

    /* Output the bug index */
    for (p = buffer; *p != NULL; p++)
    {
        hmlXref (*p, *p);
        hmlBreak ();
    }

    if (opType == HML_FORMAT_HTML)
    {
        hmlPara ();
        hmlFont (FONT_ITALIC);
        hmlFormatStr ("Note: If you experience problems when page links are "
                      "not resolved properly then the HTML pages have been "
                      "changed during your viewing session. You may resolve "
                      "by flushing your browser disk and memory caches, "
                      "alternatively restart your WWW browser.");
        hmlFont(FONT_NORMAL);
    }
    hmlIndent (-1);
    hmlEnd ();
}

void bugInit (void)
{
    hmlAddQuick ("bug");
    hmlAddQuick ("eng");
    hmlAddQuick ("mod");
    hmlAddQuick ("rpt");
}

void rp_end (void)
{
    if (bug != NULL)
        bug = bugEnd (bug);
    if (tek != NULL)
        tek = tekEnd (tek);
    if (release != NULL)
        release = releaseEnd (release);
    if (patch != NULL)
        patch = patchEnd (patch);
    if (faq != NULL)
        faq = faqEnd (faq);
    if (new != NULL)
        new = newEnd (new);
    if (inf != NULL)
        inf = infEnd (inf);
}

void rp_name (char *s)
{
    if (release != NULL)
    {
        if (release->date == 0)
            uError ("Date is 0. Please specify before name field.\n");
        release->name = nameAddMode (&releaseNames, s, release->date);
        release->name->data = (void *) release;
    }
    else if (patch != NULL)
    {
        if (patch->name != NULL)
            uError ("Name already specified. Ignoring\n");
        else
            patch->name = nameFindNode (names, s);
    }
    else if (faq != NULL)
    {
        if (faq->name != NULL)
            uError ("Name already specified. Ignoring\n");
        else if ((faq->name = nameFindNode (names, s)) == NULL)
        {
            uWarn ("Cannot find name [%s] ... Adding.\n", s);
            faq->name = nameAdd (&names, s, 0);
        }
    }
    else
        nameAdd (&names,  s, 0);
}

void rp_bugNumber (int i)
{
    uInteractive ("Processing bug #%d", i);
    rp_end ();
    bug = (Bug *)(ListItemConstruct (sizeof (Bug)));
    BUGNO(bug) = i;
}

void rp_releaseNumber (int i)
{
    uInteractive ("Processing release #%d", i);
    rp_end ();
    release = (Release *)(ListItemConstruct (sizeof (Release)));
    RELNO(release) = i;
}

void rp_tekNumber (int i)
{
    uInteractive ("Processing technical note #%d", i);
    rp_end ();
    tek = (Tek *)(ListItemConstruct (sizeof (Tek)));
    TEKNO(tek) = i;
}

void rp_faqNumber (int i)
{
    uInteractive ("Frequently asked question #%d", i);
    rp_end ();
    faq = (Faq *)(ListItemConstruct (sizeof (Faq)));
    FAQNO(faq) = i;
}

void rp_infNumber (int i)
{
    uInteractive ("Information Number #%d", i);
    rp_end ();
    inf = (Inf *)(ListItemConstruct (sizeof (Inf)));
    INFNO(inf) = i;
}

void rp_newNumber (int i)
{
    uInteractive ("Processing new item #%d", i);
    rp_end ();
    new = (New *)(ListItemConstruct (sizeof (New)));
    NEWNO(new) = i;
}

void rp_patchNumber (int i)
{
    uInteractive ("Processing patch #%d", i);
    rp_end ();
    patch = (Patch *)(ListItemConstruct (sizeof (Patch)));
    PATNO(patch) = i;
}

void rp_localId (char *s)
{
    if (bug->localId != NULL)
        uError ("Local identity already set to [%s]\n");
    else
        bug->localId = bufNStr(NULL, s);
}

void rp_date (unsigned long date)
{
    if ((bug != NULL) && (bug->date == 0))
        bug->date = date;
    else if ((release != NULL) && (release->date == 0))
        release->date = date;
    else if ((tek != NULL) && (tek->date == 0))
        tek->date = date;
    else if ((new != NULL) && (new->date == 0))
        new->date = date;
    else if ((faq != NULL) && (faq->date == 0))
        faq->date = date;
    else if ((patch != NULL) && (patch->date == 0))
        patch->date = date;
    else
        uError ("Date already specified.\n");
}

void rp_module (char *module)
{
    if ((bug != NULL) && (bug->module == NULL))
        bug->module = bufNStr (NULL, module);
    else if ((tek != NULL) && (tek->module == NULL))
        tek->module = bufNStr (NULL, module);
    else if ((faq != NULL) && (faq->module == NULL))
        faq->module = bufNStr (NULL, module);
    else if ((new != NULL) && (new->module == NULL))
        new->module = bufNStr (NULL, module);
    else
        uError ("Module already specified. Ignoring [%s].\n", module);
}

void rp_component (char *component)
{
    if (bug->component != NULL)
        uError ("Component already specified. Ignoring [%s].\n", component);
    else
        bug->component = bufNStr (NULL, component);
}

int rp_notes (char **notes, int count)
{
    if ((bug != NULL) && (bug->notes.argv == NULL))
    {
        bug->notes.argv = notes;
        bug->notes.argc = count;
        return (1);
    }
    else if ((release != NULL) && (release->notes.argv == NULL))
    {
        release->notes.argv = notes;
        release->notes.argc = count;
        return (1);
    }
    else if ((patch != NULL) && (patch->notes.argv == NULL))
    {
        patch->notes.argv = notes;
        patch->notes.argc = count;
        return (1);
    }
    else if ((faq != NULL) && (faq->notes.argv == NULL))
    {
        faq->notes.argv = notes;
        faq->notes.argc = count;
        return (1);
    }
    else if ((new != NULL) && (new->notes.argv == NULL))
    {
        new->notes.argv = notes;
        new->notes.argc = count;
        return (1);
    }
    else if ((tek != NULL) && (tek->notes.argv == NULL))
    {
        tek->notes.argv = notes;
        tek->notes.argc = count;
        return (1);
    }
    else if ((inf != NULL) && (inf->notes.argv == NULL))
    {
        inf->notes.argv = notes;
        inf->notes.argc = count;
        return (1);
    }
    uError ("Notes already specified. Ignoring\n");
    return (0);
}

int rp_affecting (char **affecting, int count)
{
    if ((patch != NULL) && (patch->affecting.argv == NULL))
    {
        patch->affecting.argv = affecting;
        patch->affecting.argc = count;
        return (1);
    }
    uError ("Affecting already specified. Ignoring\n");
    return (0);
}

int rp_seeAlso (char **list, int count)
{
    if ((bug != NULL) && (bug->seeAlso.argv == NULL))
    {
        bug->seeAlso.argv = list;
        bug->seeAlso.argc = count;
        return (1);
    }
    else if ((release != NULL) && (release->seeAlso.argv == NULL))
    {
        release->seeAlso.argv = list;
        release->seeAlso.argc = count;
        return (1);
    }
    else if ((tek != NULL) && (tek->seeAlso.argv == NULL))
    {
        tek->seeAlso.argv = list;
        tek->seeAlso.argc = count;
        return (1);
    }
    else if ((inf != NULL) && (inf->seeAlso.argv == NULL))
    {
        inf->seeAlso.argv = list;
        inf->seeAlso.argc = count;
        return (1);
    }
    else if ((patch != NULL) && (patch->seeAlso.argv == NULL))
    {
        patch->seeAlso.argv = list;
        patch->seeAlso.argc = count;
        return (1);
    }
    else if ((faq != NULL) && (faq->seeAlso.argv == NULL))
    {
        faq->seeAlso.argv = list;
        faq->seeAlso.argc = count;
        return (1);
    }
    else if ((new != NULL) && (new->seeAlso.argv == NULL))
    {
        new->seeAlso.argv = list;
        new->seeAlso.argc = count;
        return (1);
    }
    else
        uError ("See Also already specified. Ignoring\n");
    return (0);
}

int rp_workaround (char **list, int count)
{
    if ((bug != NULL) && (bug->workaround.argv == NULL))
    {
        bug->workaround.argv = list;
        bug->workaround.argc = count;
        return (1);
    }
    else if ((patch != NULL) && (patch->workaround.argv == NULL))
    {
        patch->workaround.argv = list;
        patch->workaround.argc = count;
        return (1);
    }
    uError ("Workaround already specified\n.");
    return (0);
}

void rp_release (Name *name)
{
    if ((bug != NULL) && (bug->release == NULL))
        bug->release = ((name == NULL) ? &undefinedReleaseName : name);
    else if ((tek != NULL) && (tek->release == NULL))
        tek->release = ((name == NULL) ? &undefinedReleaseName : name);
    else if ((new != NULL) && (new->release == NULL))
        new->release = ((name == NULL) ? &undefinedReleaseName : name);
    else if ((patch != NULL) && (patch->release == NULL))
        patch->release = ((name == NULL) ? &undefinedReleaseName : name);
    else if ((faq != NULL) && (faq->release == NULL))
        faq->release = ((name == NULL) ? &undefinedReleaseName : name);
    else
        uError ("Release already specified. Ignoring [%s]\n", name->name);
}

void rp_patch (int i)
{
    if ((bug != NULL) && (bug->patchNo == 0))
    {
        if (bug->fixDate != 0)
            uError ("Patch specified after Fix Date\n");
        bug->patchNo = i;
    }
    else
        uError ("Patch already specified. Ignoring [%d]\n", i);
}

void rp_reportBy (char *name)
{
    if ((bug != NULL) && (bug->reportBy == NULL))
        bug->reportBy = bufNStr (NULL, name);
    else if ((tek != NULL) && (tek->reportBy == NULL))
        tek->reportBy = bufNStr (NULL, name);
    else if ((new != NULL) && (new->reportBy == NULL))
        new->reportBy = bufNStr (NULL, name);
    else
        uError ("Report by already specified. Ignoring [%s]\n", name);
}

void rp_reportFor (char *name)
{
    if (bug->reportFor != 0)
        uError ("Report for already specified. Ignoring [%s]\n", name);
    else
        bug->reportFor = bufNStr (NULL, name);
}

void rp_status (int i)
{
    if ((bug->status != 0) && (bug->status != i))
        uError ("Status already reported\n");
    else
        bug->status = i;
}

void rp_releaseState (int i)
{
    if ((release->state != 0) && (release->state != i))
        uError ("State already reported\n");
    else
        release->state = i;
}

void rp_engineer (char *name)
{
    if (bug->engineer != 0)
        uError ("Engineer already specified. Ignoring [%s]\n", name);
    else
        bug->engineer = bufNStr (NULL, name);
}

void rp_severity (int i)
{
    if (bug->severity != 0) {
        uError ("Severity already reported. Current %d. New %d\n",
                bug->severity, i);
        return;
    }
    if (i > SEVERITY_MAX) {
        i -= SEVERITY_MAX;
        if (bug->status == STATUS_OPEN)
            bug->status = STATUS_CLOSED;            /* May need to put checks in here */
    }
    bug->severity = i;
}

void rp_summary (char *s)
{
    if ((bug != NULL) && (bug->summary == NULL))
        bug->summary = bufNStr (NULL, s);
    else if ((tek != NULL) && (tek->summary == NULL))
        tek->summary = bufNStr (NULL, s);
    else if ((patch != NULL) && (patch->summary == NULL))
        patch->summary = bufNStr (NULL, s);
    else if ((new != NULL) && (new->summary == NULL))
        new->summary = bufNStr (NULL, s);
    else if ((faq != NULL) && (faq->question == NULL))
        faq->question = bufNStr (NULL, s);
    else if ((inf != NULL) && (inf->summary == NULL))
        inf->summary = bufNStr (NULL, s);
    else
        uError ("Summary already specified. Ignoring [%s]\n", s);
}

void rp_system (char *s)
{
    if (bug->system != 0)
        uError ("System already specified. Ignoring [%s]\n", s);
    else
        bug->system = bufNStr (NULL, s);
}

void rp_systemSpec (char *s)
{
    if (bug->systemSpec != 0)
        uError ("System specification already specified. Ignoring [%s]\n", s);
    else
        bug->systemSpec = bufNStr (NULL, s);
}

int rp_description (char **list, int count)
{
    if ((bug != NULL) && (bug->description.argv == NULL))
    {
        bug->description.argv = list;
        bug->description.argc = count;
        return (1);
    }
    else if ((release != NULL) && (release->description.argv == NULL))
    {
        release->description.argv = list;
        release->description.argc = count;
        return (1);
    }
    else if ((tek != NULL) && (tek->description.argv == NULL))
    {
        tek->description.argv = list;
        tek->description.argc = count;
        return (1);
    }
    else if ((patch != NULL) && (patch->description.argv == NULL))
    {
        patch->description.argv = list;
        patch->description.argc = count;
        return (1);
    }
    else if ((faq != NULL) && (faq->description.argv == NULL))
    {
        faq->description.argv = list;
        faq->description.argc = count;
        return (1);
    }
    else if ((new != NULL) && (new->description.argv == NULL))
    {
        new->description.argv = list;
        new->description.argc = count;
        return (1);
    }
    else if ((inf != NULL) && (inf->description.argv == NULL))
    {
        inf->description.argv = list;
        inf->description.argc = count;
        return (1);
    }
    uError ("Description already specified. Ignoring\n");
    return (0);
}

void rp_fixedBy (char *s)
{
    if (bug->fixBy != 0)
        uError ("Fixed by already specified. Ignoring [%s]\n", s);
    else
        bug->fixBy = bufNStr (NULL, s);
}

void rp_fixedDate (unsigned long date, Name *n)
{
    uDebug (1, ("Fixed date = %ld, name = %s\n", date,
            (n == NULL) ? "NULL" : n->name[0]));
    if (bug->fixDate != 0)
        uError ("Fixed date already specified.\n");
    else if ((bug->fixDate = date) != 0)
    {
        /*
         * If there is no name then find a release name in the relase
         * names list that encompasses the fix.
         */

        if ((bug->fixRelease = n) == NULL)
        {
            if ((n = releaseNames) != NULL)
            {
                do
                {
                    if ((n->mode > date) && (n->data != NULL))
                    {
                        int state;

                        state = ((Release *)(n->data))->state;
                        if (state == STATE_FULL)
                            break;
                        if ((state == STATE_PATCHED) && (bug->patchNo != 0))
                            break;
                    }
                }
                while ((n = n->next) != NULL);
            }
            bug->fixRelease = n;
        }
    }
}

int rp_fixedDescription (char **list, int count)
{
    if (bug->fixDescription.argv!= NULL)
    {
        uError ("Fixed description already specified. Ignoring\n");
        return (0);
    }

    bug->fixDescription.argv = list;
    bug->fixDescription.argc = count;
    return (1);
}

void rp_suspendedBy (char *name)
{
    if (bug->susBy != NULL)
        uError ("Suspend by already specified. Ignoring [%s]\n", name);
    else
        bug->susBy = bufNStr (NULL, name);
}

void rp_unsuspendedBy (char *name)
{
    if (bug->unsusBy != NULL)
        uError ("Unsuspend by already specified. Ignoring [%s]\n", name);
    else
        bug->unsusBy = bufNStr (NULL, name);
}

void rp_suspendedDate (unsigned long date)
{
    if (bug->susDate != 0)
        uError ("Suspend date already specified.\n");
    bug->susDate = date;
}

void rp_unsuspendedDate (unsigned long date)
{
    if (bug->unsusDate != 0)
        uError ("Unsuspend date already specified.\n");
    bug->unsusDate = date;
}

int rp_suspendedDesc (char **list, int count)
{
    if (bug->susDescription.argv != NULL)
    {
        uError ("Suspend description already specified. Ignoring\n");
        return (0);
    }

    bug->susDescription.argv = list;
    bug->susDescription.argc = count;
    return (1);
}

static void
composeGenerationTime (void)
{
    time_t     clock;		        /* Time in machine format. */
    struct tm  *time_ptr;               /* Pointer to time frame. */
    char *s;
        
    clock = time (0);
    time_ptr = (struct tm *) localtime (&clock);	/* Get time frame */
    buildTime = bufNStr (NULL, asctime (time_ptr));
    for (s = buildTime; *s != '\0'; s++) {
        if ((*s == '\n') || (*s == '\r')) {
            *s = '\0';
            break;
        }
    }
    
    todaysDate = dateSetYear (todaysDate, time_ptr->tm_year);
    todaysDate = dateSetMonth (todaysDate, time_ptr->tm_mon + 1);
    todaysDate = dateSetDay (todaysDate, time_ptr->tm_mday);
    
    hmlGenId (bufFormat (NULL, "Generated on %s", buildTime));
}

static void
usage (void)
{
    fprintf (stdout, "%s : Usage %s [Options] <files>\n", progname, progname);
    fprintf (stdout, "\n");
    fprintf (stdout, "-?        : Help - this page\n");
    fprintf (stdout, "-d <num>  : Debug setting (0-9)\n");
    fprintf (stdout, "-E <file> : Log errors to <file> (append)\n");
    fprintf (stdout, "-e <file> : Log errors to <file> (re-write)\n");
    fprintf (stdout, "-h        : Write html file output [Default]\n");
    fprintf (stdout, "-H <home> : Add home page external reference\n");
    fprintf (stdout, "-i        : Enable interactive status prints [Default]\n");
    fprintf (stdout, "-I        : Version Information\n");
    fprintf (stdout, "-o <file> : Output to <file> basename; default is idc\n");
    fprintf (stdout, "-p <file> : Name of the logo picture (image)\n");
    fprintf (stdout, "-q        : Disable interactive status prints\n");
    fprintf (stdout, "-t <title>: Assign a title to the database\n");
    fprintf (stdout, "-r        : Write rtf file output.\n");
    fprintf (stdout, "-v <num>  : Verbose setting (0-9)\n");
    exit (1);
}

int
main (int argc, char *argv [])
{
    char *oname;                        /* Output Name */
    char *hname = NULL;                 /* Home name */
    char *pname = NULL;                 /* Picture Image name */
    int  ecount = 0;                    /* Error count */
    int  wcount = 0;                    /* Warning count */
    int  i;                             /* Local integer register */
    int  c;                             /* Local character register */
    
    /* Set up the error reporting */
    progname = MODULE_NAME;
    oname = NULL;
    uErrorSet (20, &ecount);            /* Max Num entries + counter */
    uWarnSet (&wcount);                 /* Warning count */
    uUtilitySet (MODULE_NAME);          /* Set name of program */
    uVerboseSet (0);                    /* Verbose setting */
    
    /* Collect the arguments and options */
    while (1)
    {
        c = getopt (argc, argv, "d:e:E:hH:iIo:qrt:v:");
        if (c == EOF)
            break;
        switch (c)
        {
        case 'd':
            uDebugSet ((int)(optarg[0] - '0'));
            break;
        case 'e':
            uErrorChannelSet (optarg, UERROR_LOG_CREATE);
            break;
        case 'E':
            uErrorChannelSet (optarg, UERROR_LOG_APPEND);
            break;
        case 'H':
            hname = optarg;
            break;
        case 'h':
            opType = HML_FORMAT_HTML;
            break;
        case 'i':
            uInteractiveSet (1);        /* Enabled interactive mode */
            break;
        case 'I':
            fprintf (stdout, "%s. Version %s. %s. [%s].\n",
                     progname, MODULE_VERSION, __DATE__, hmlVersion());
            exit (0);
            break;
        case 'o':
            oname = optarg;
            break;
        case 'p':
            pname = optarg;
            break;
        case 'q':
            uInteractiveSet (0);        /* Disable interactive mode */
            break;
        case 'r':
            opType = HML_FORMAT_RTF;
            break;
        case 't':
            mainTitle = optarg;
            break;
        case 'v':
            uVerboseSet ((int)(optarg[0] - '0'));
            break;
        default:
            fprintf (stdout, "Cannot understand option [-%c]\n", c);
            usage();
            break;
        }
    }

    /* Initialise the program variables */
    uOpenErrorChannel ();
    hmlIdentity (bufFormat (NULL, "%s - Version %s - %s - %s",
                            progname, MODULE_VERSION, __DATE__,
                            "Jon Green (1995-96)"));
    if (mainTitle == NULL)
        mainTitle = "Information Development Database";
    composeGenerationTime ();
    
    hmlAddHome (hname);                 /* Add external home page name */
    hmlAddQuick ("home");               /* Add home reference */
    bugInit ();
    faqInit ();
    infInit ();
    newInit ();
    patchInit ();
    relInit ();
    tekInit ();

    /* Process the input files. */
    argv = getfiles (&argc, argv, optind);

    if (argc == 1)
        uError ("No files specified\n");
    else
    {
        if (oname != NULL)
            hmlOpen (opType, oname, pname);
            
        for (i = 1; i < argc; i++)
        {
            uInteractive ("Processing file [%s]", argv [i]);
            
            /* Open files */
            if (oname == NULL)
                hmlOpen (opType, argv [i], pname);
            rpOpen (argv [i]);
            
            /* Do reports. */
            reportPackager (rpfp);
            if ((i+1) == argc)
                rp_bugOutputBugs ();
            
            /* Close files. */
            rpClose (rpfp);
            rpfp = NULL;
            if (oname == NULL)
                hmlClose();
        }
        
        if (oname != NULL)
            hmlClose ();
    }
    
    /* Shut down */
    uCloseErrorChannel ();
    return (ecount);
}
