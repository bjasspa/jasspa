/****************************************************************************
 *
 *  			Copyright 1996 Jon Green.
 *                         All Rights Reserved
 *
 *
 *  System        : 
 *  Module        : 
 *  Object Name   : $RCSfile: bug.c,v $
 *  Revision      : $Revision: 1.1 $
 *  Date          : $Date: 2000-10-21 14:31:21 $
 *  Author        : $Author: jon $
 *  Last Modified : <990831.2346>
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

static const char rcsid[] = "@(#) : $Id: bug.c,v 1.1 2000-10-21 14:31:21 jon Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "idc.h"

static char *strStatus [] = {
    "Unknown",
    "Open",
    "Suspended",
    "Closed",
    "Rejected"
};

static char *strSeverity [] = {
    "Class 0 - Unassigned error category",
    "Class A - Critical error",
    "Class B - System is deficient in some way & no work around",
    "Class C - System deficient work around",
    "Class D - Desirable feature omitted",
    "Class E - Suggestion - not for near future"
};

static Bug *bugHead = NULL;             /* Head of bug list */

/*
 * Constructors for the Bug Information.
 */

static void
bugDestruct (Bug *p)
{
    if (p == NULL)
        return;

    /* Free off the data fields */
    bufFree (p->module);
    bufFree (p->reportBy);
    bufFree (p->localId);
    bufFree (p->reportFor);
    bufFree (p->engineer);
    bufFree (p->summary);
    bufFree (p->system);
    bufFree (p->systemSpec);
    bufFree (p->component);
    bufArgFree (&p->description);
    bufArgFree (&p->fixDescription);
    bufFree (p->susBy);
    bufArgFree (&p->susDescription);
    bufArgFree (&p->notes);

    /* Clear the buffer */
    memset (p, 0, sizeof (Bug));
}

Bug *
bugGetHead (void)
{
    return (bugHead);
}


static char *
toAscii (char *format, int i)
{
    static char buf [20];
    sprintf (buf, format, i);
    return buf;
}
              
static void
bugOutput (Bug *p)
{
    /* Opening page stuff. */
    hmlNumericFileHeader ("bug", BUGNO(p), "Bug Report #%d");

    /* Synopsis */
    hmlTitle (1, "Synopsis");
    hmlIndent (1);
    hmlPara ();
    hmlFormatStr ("%s", p->summary);
    hmlIndent (-1);
    
    /* Details */
    hmlTitle (1, "Details");
    hmlIndent (1);
    if (p->module)
        hmlBullet ("%s", p->module);
    if (p->component != NULL)
        hmlBullet ("%s", p->component);
    hmlBullet ("%s", strStatus [p->status]);
    hmlBullet ("%s", strSeverity [p->severity]);
    hmlBullet ("%s", p->release->name[0]);
    hmlIndent (-1);

    /* Text fields */
    hmlDescription ("Description", 1, &p->description);
    hmlDescription ("Workaround", 1, &p->workaround);
    hmlDescription ("Notes", 1, &p->notes);

    /* Reported By */
    hmlTitle (1, "Reported By");
    hmlIndent (1);
    hmlBullet ("%s", p->reportBy);
    if (p->reportFor != NULL)
        hmlBullet ("%s", p->reportFor);
    if (p->localId != NULL)
        hmlBullet ("Local Ident: %s", p->localId);

    hmlBullet ("%s", strDate (p->date));
    hmlBullet ("%s", p->system);
    if (p->systemSpec != NULL) 
        hmlBullet ("%s", p->systemSpec);
    hmlIndent (-1);
    hmlLine ();

    /* Engineering */
    hmlTitle (1, "Engineering");
    hmlIndent (1);
    hmlBullet ("%s", p->engineer);
    hmlBullet ("%s", strStatus [p->status]);
    hmlBullet ("%s", strSeverity [p->severity]);
    hmlIndent (-1);
    hmlLine ();

    /* Suspended Details */
    if (p->susDescription.argv != NULL)
    {
        hmlTitle (1, "Suspended Details");
        hmlIndent (1);
        if (p->susBy)
            hmlBullet ("%s", p->susBy);
        if (p->susDate)
            hmlBullet ("%s", strDate (p->susDate));
        if (p->unsusBy != NULL)
            hmlBullet ("Unsuspended by: %s", p->susBy);
        if (p->unsusDate != 0)
            hmlBullet ("Unsuspended on: %s", strDate (p->unsusDate));
        hmlIndent (-1);
        hmlDescription (NULL, 1, &p->susDescription);
    }

    /* Fixed Details */
    if (p->fixDescription.argv != NULL)
    {
        hmlTitle (1, "Fixed Details");
        hmlIndent (1);
        if (p->fixBy != NULL)
            hmlBullet ("%s", p->fixBy);
        if (p->fixDate != 0)
            hmlBullet ("Fixed on: %s", strDate (p->fixDate));
        if (p->fixRelease != NULL)
            hmlBullet ("First fixed release: %s", p->fixRelease->name[0]);
        if (p->patchNo != 0)
        {
            hmlBullet ("Patched by: ");
            hmlXref (hmlXrefPageName ("pat%04d", p->patchNo),
                     "Patch #%d", p->patchNo);
        }
        hmlIndent (-1);
        hmlDescription (NULL, 1, &p->fixDescription);
    }
    
    hmlReference ("See Also", 1, &p->seeAlso);
    
    if (p->status == STATUS_OPEN)
    {
        /* Close button */
        hmlForm (1);
        hmlFormField ("hidden", "Type:", "creq");
        hmlFormField ("hidden", "Project:", project);
        hmlFormField ("hidden", "Bug Number:", toAscii ("%d", BUGNO(p)));
        hmlFormField ("hidden", "E", "E");
        hmlFormField ("submit", NULL, "Close Bug");
        hmlForm (0);
        
        /* Note button */
        hmlForm (1);
        hmlFormField ("hidden", "Type:", "nreq");
        hmlFormField ("hidden", "Project:", project);
        hmlFormField ("hidden", "Bug Number:", toAscii ("%d", BUGNO(p)));
        hmlFormField ("hidden", "E", "E");
        hmlFormField ("submit", NULL, "Add Note");
        hmlForm (0);
    }
    hmlEnd ();
}

static void
bugCheck (Bug *p)
{
    Bug *tp;

    if (patchGetHead() != NULL)
        uError ("Bugs should appear before patches\n");
    for (tp = bugHead; tp != NULL; tp = BUGNEXT(tp))
        if (BUGNO(tp) == BUGNO(p))
            uWarn ("Duplication bug number [%d]\n", BUGNO(p));
    if ((p->severity < SEVERITY_A) || (p->severity > SEVERITY_E))
        uWarn ("Category field not valid.\n");
    if (p->date == 0)
        uWarn ("Bug report must be dated\n");
    if (p->status == 0)
        uWarn ("Bug report must be assigned a status\n");
    if (p->release == NULL)
        uWarn ("Release not specified\n");
    if (p->reportBy == NULL)
        uWarn ("Report By not specified\n");
    if (p->release->mode > p->date)
        uWarn ("Report date is less than release date\n");
    uDebug (1, ("Fixed Release is %s\n",
                 (p->fixRelease == NULL) ? "NULL" : "NOT NULL"));
    uDebug (1, ("Fixed Release is %s\n",
                 (p->fixRelease == NULL) ? "NULL" : p->fixRelease->name[0]));
    if (((p->susDate != 0) && (p->susDate < p->release->mode)) ||
        ((p->fixDate != 0) && (p->fixDate < p->release->mode)))
        uWarn ("Inconsistency in Date field and Fix/Suspend Dates\n");
    if (p->summary == NULL)
        uWarn ("Summary expected\n");
    if (p->engineer == NULL)
        uWarn ("Engineer not assigned\n");
    if (p->system == NULL)
        uWarn ("System not specified\n");
    if (p->description.argv == NULL)
        uWarn ("Description required\n");
    if ((p->fixBy != NULL) || (p->fixRelease != NULL) ||
        (p->fixDate != 0) || (p->fixDescription.argv != NULL))
    {
        if (p->fixBy == NULL)
            uWarn ("Fix By field undefined\n");
        if (p->fixRelease == NULL)
            uWarn ("Fix Date field undefined. Cannot determine fix release\n");
        if (p->fixDate == 0)
            uWarn ("Fix Date field undefined.\n");
        if (p->fixDescription.argv == NULL)
            uWarn ("Details of fix not provided\n");
        if ((p->status != STATUS_CLOSED) && (p->status != STATUS_REJECTED))
            uWarn ("Fix fields entered and Status is not Closed.\n");
    }
    else if ((p->status == STATUS_CLOSED) || (p->status == STATUS_REJECTED))
        uWarn ("Status indicates closed (or rejected) and no fix details provided\n");
    if ((p->susBy != NULL) || (p->susDate != 0) || (p->susDescription.argv != NULL))
    {
        if (p->susBy == NULL)
            uWarn ("Suspend By field undefined\n");
        if (p->susDate == 0)
            uWarn ("Suspend Date field undefined.\n");
        if (p->susDescription.argv == NULL)
            uWarn ("Details of suspension not provided\n");
        if ((p->status == STATUS_OPEN) && (p->unsusDate == 0))
            uWarn ("Suspend fields entered and Status is Open.\n");
    }
    else if (p->status == STATUS_SUSPENDED)
        uWarn ("Suspended status and the suspend fields not entered\n");
}

Bug
*bugEnd (Bug *bug)
{
    if (bug != NULL)
    {
        /* Check the fields of the bug report. */

        uDebug (1, ("Doing check\n"));
        bugCheck (bug);
        uDebug (1, ("Done check - doing output\n"));
        bugOutput (bug);
        uDebug (1, ("Done output - doing semi destruct\n"));

        /*
         * Free off the data fields
         */

        bufArgFree (&bug->description);
        bufArgFree (&bug->fixDescription);
        bufArgFree (&bug->susDescription);
        bufArgFree (&bug->notes);

        uDebug (1, ("Done semi destruct\n"));
        ListItemInsert ((ListItem **)(&bugHead), &bug->list);
    }
    return (NULL);
}

static Bug *
bugFindNext (Bug *p, int category, int status, char *reported, char *module,
             char *engineer)
{
    if (p == NULL)
        p = bugHead;
    else
        p = BUGNEXT(p);

    while (p)
    {
        if (((category == 0) || (p->severity == category)) &&
            ((status == 0) || (p->status == status)) &&
            ((reported == NULL) || (strcmp (reported, p->reportBy) == 0)) &&
            ((module == NULL) || (strcmp (module, p->module) == 0)) &&
            ((engineer == NULL) || (strcmp (engineer, p->engineer) == 0)))
            return (p);
        else
            p = BUGNEXT(p);
    }
    return (NULL);
}

static int
highlightType (int type)
{
    if (type == STATUS_OPEN)
        return (FONT_BOLD);
    if (type == STATUS_REJECTED)
        return (FONT_ITALIC);
    return (0);
}

void
bugMenuItem (Bug *p, int type)
{
    if (type)
        type = highlightType (p->status);
    hmlXref (hmlXrefPageName ("bug%04d", BUGNO(p)), "#%04d", BUGNO(p));
    if (type)
        hmlFont (type);
    hmlFormatStr (" [%s] %s", p->module, p->summary);
    if (type)
        hmlFont (FONT_NORMAL);
    hmlBreak ();
}

static void
bugMenuDateItem (Bug *p, long date, int type)
{
    if (type)
        type = highlightType (p->status);
    hmlXref (hmlXrefPageName ("bug%04d", BUGNO(p)), "#%04d", BUGNO(p));
    hmlFormatStr (" %s:", strShortDate (date));
    if (type)
        hmlFont (type);
    hmlFormatStr (" %s - %s",p->module, p->summary);
    if (type)
        hmlFont (FONT_NORMAL);
    hmlBreak ();
}

static void
bugIndexGenerate (char *title, char *reported, char *module, char *engineer)
{
    int category;
    int count;
    Bug *p;

    /* Openning page stuff. */
    hmlFileHeader (title, NULL);

    if (bugFindNext (NULL, 0, STATUS_OPEN, reported, module, engineer) != NULL)
    {
        hmlTitle (0, "Open Bugs");
        for (category = SEVERITY_A; category <= SEVERITY_MAX; category++)
        {
            if ((p = bugFindNext (NULL, category, STATUS_OPEN,
                                  reported, module, engineer)) == NULL)
                continue;

            hmlTitle (1, "Category %c", 'A' + category - 1);
            hmlIndent (1);
            hmlPara ();
            count = 0;
            do {
                bugMenuItem (p,0);
                count++;
            } while ((p = bugFindNext (p, category, STATUS_OPEN,
                                       reported, module, engineer)) != NULL);

            outputItemCount (count, -1);
            hmlIndent (-1);
        }
        hmlLine ();
    }
    if ((p = bugFindNext (NULL, 0, STATUS_SUSPENDED, reported,
                          module, engineer)) != NULL)
    {
        hmlTitle (0, "Suspended Bugs");
        hmlIndent (1);
        hmlPara ();

        count = 0;
        do {
            bugMenuItem (p,0);
            count++;
        } while ((p = bugFindNext (p, 0, STATUS_SUSPENDED,
                                   reported, module, engineer)) != NULL);

        outputItemCount (count, -1);
        hmlIndent (-1);
        hmlLine ();

    }
    if ((p = bugFindNext (NULL, 0, STATUS_REJECTED, reported,
                          module, engineer)) != NULL)
    {
        hmlTitle (0, "Rejected As Bugs");
        hmlIndent (1);
        hmlPara ();

        count = 0;
        do {
            bugMenuItem (p, 0);
            count++;
        } while ((p = bugFindNext (p, 0, STATUS_REJECTED,
                                   reported, module, engineer)) != NULL);

        outputItemCount (count, -1);
        hmlIndent (-1);
        hmlLine ();
    }
    if ((p = bugFindNext (NULL, 0, STATUS_CLOSED, reported,
                          module, engineer)) != NULL)
    {
        hmlTitle (0, "Closed Bugs");
        hmlIndent (1);
        hmlPara ();

                count = 0;
        do {
            bugMenuItem (p, 0);
            count++;
        } while ((p = bugFindNext (p, 0, STATUS_CLOSED,
                                   reported, module, engineer)) != NULL);

        outputItemCount (count, -1);
        hmlIndent (-1);
        hmlLine ();
    }
    
    hmlEnd ();
}

static char *
bugIndexCurrent (void)
{
    static char *title = "Bugs - Current status";

    bugIndexGenerate (title, NULL, NULL, NULL);
    return (title);
}

/*
 * Display bugs by modules.
 */

static char *
bugIndexReportedBy (void)
{
    static char *title = "Bugs - Reported By Index";
    Name *n;
    char *s;

    /* Opening page stuff. */
    hmlFileHeader (title, NULL);
    hmlDefinition ("rpt");
    hmlIndent (1);
    hmlPara ();
    hmlFormatStr ("The following is a list of named persons "
                  "who have submitted a bug report. "
                  "Following the link provides a list of all bugs "
                  "reported by the named person.");
    hmlPara ();

    /* Generate the index */
    for (n = names; n != NULL; n = n->next)
        if ((n->mode & NAME_REPORTED) != 0)
        {
            hmlXref (hmlXrefPageName ("Reports by %s", n->name [0]),
                     n->name[0]);
            hmlBreak ();
        }
    hmlIndent (-1);
    hmlEnd ();

    /* Construct the reported by page */
    for (n = names; n != NULL; n = n->next)
        if ((n->mode & NAME_REPORTED) != 0) {
            uInteractive ("Generating reported by pages for [%s]",
                          n->name[0]);
            s = bufFormat (NULL, "Reports by %s", n->name[0]);
            bugIndexGenerate (s, n->name[0], NULL, NULL);
            bufFree (s);
        }
    return (title);
}

/*
 * Display bugs by modules.
 */

static char *
bugIndexModules (void)
{
    static char *title = "Bugs - Module Index";
    Name *n;
    char *s;

    /* Opening page stuff. */
    hmlFileHeader (title, NULL);
    hmlDefinition ("mod");
    hmlIndent (1);
    hmlPara ();
    hmlFormatStr ("The following is a list of reported "
                  "modules assigned bugs. "
                  "The bug status of the module is "
                  "provided by following the link.");
    hmlPara ();

    /* Generate the index */
    for (n = swmodules; n != NULL; n = n->next)
        if ((n->mode & NAME_REPORTED) != 0)
        {
            hmlXref (hmlXrefPageName ("Module %s", n->name [0]),
                     n->name[0]);
            hmlBreak ();
        }
    hmlIndent (-1);
    hmlEnd ();

    /* Construct the module page */
    for (n = swmodules; n != NULL; n = n->next)
        if ((n->mode & NAME_REPORTED) != 0) {
            uInteractive ("Generating module reports for [%s]", n->name[0]);
            s = bufFormat (NULL, "Module %s", n->name[0]);
            bugIndexGenerate (s, NULL, n->name[0], NULL);
            bufFree (s);
        }
    return (title);
}

/*
 * Display bugs by an engineer.
 */

static char *
bugIndexEngineer (void)
{
    static char *title = "Bugs - Engineering Index";
    Name *n;
    char *s;

    /* Opening page stuff. */
    hmlFileHeader (title, NULL);
    hmlDefinition ("eng");
    hmlIndent (1);
    hmlPara ();
    hmlFormatStr ("The following is a list of named engineers "
                  "whom have been assigned bugs for closure "
                  "Following the link provides a list of all bugs assigned "
                  "with a closure status.");
    hmlPara ();

    /* Construct the engineering index */
    for (n = names; n != NULL; n = n->next)
        if ((n->mode & NAME_ENGINEER) != 0)
        {
            hmlXref (hmlXrefPageName ("Engineering for %s", n->name [0]),
                     n->name[0]);
            hmlBreak ();
        }
    hmlIndent (-1);
    hmlEnd ();

    /* Construct the engineering page */
    for (n = names; n != NULL; n = n->next)
        if ((n->mode & NAME_ENGINEER) != 0) {
            uInteractive ("Generating engineering reports for [%s]",
                          n->name[0]);
            s = bufFormat (NULL, "Engineering for %s", n->name[0]);
            bugIndexGenerate (s, NULL, NULL, n->name[0]);
            bufFree (s);
        }
    return (title);
}

/*
 * Display a list of bugs by number.
 */

static char *
bugIndexNumerical (void)
{
    static char *title = "Bugs - Numeric List";
    Bug *p;

    /* Opening page stuff. */
    hmlFileHeader (title, NULL);
    hmlIndent (1);
    hmlPara ();
    hmlFormatStr ("A list of all bugs ordered by their numerical bug "
                  "number. Items shown in bold are open, items in italic "
                  "are rejected, regular text items are closed.");
    hmlPara ();

    for (p = bugHead; p != NULL; p = BUGNEXT (p))
        bugMenuItem (p, 1);
    hmlIndent (-1);
    hmlEnd ();
    return (title);
}

/*
 * Display bugs by opening date.
 */

static int
bugReportSort (void *d1, void *d2)
{
    Bug *p1 = (Bug *)(d1);
    Bug *p2 = (Bug *)(d2);

    uDebug (8, ("Bug Report Sorting : %d, %d\n", BUGNO(p1), BUGNO(p2)));

    return ((p1->date >= p2->date) ? -1 : 1);
}

static char *
bugIndexReport (void)
{
    static char *title = "Bugs - Chronological Reported On";

    ListItem *p;
    ListItem *sortHead;

    /* Opening page stuff. */
    hmlFileHeader (title, NULL);
    hmlIndent (1);
    hmlPara ();
    hmlFormatStr ("This menu is a chronologically date ordered list "
                   "which shows when the bugs were reported.");
    hmlPara ();
    hmlFormatStr ("Items shown in bold are open, items in italic "
                  "are rejected, regular text items are closed.");
    hmlPara ();

    /* Duplicate the BUG list and sort according to reported by date. */
    sortHead = ListDuplicate (&bugHead->list);
    ListSort (&sortHead, bugReportSort);

    for (p = sortHead; p != NULL; p = p->next)
        bugMenuDateItem ((Bug *)(p->data), ((Bug *)(p->data))->date, 1);
    hmlIndent (-1);
    hmlEnd ();
    
    ListDestructAll (&sortHead);
    return (title);
}

/*
 * Display bugs by modified date.
 */

static int
bugMovementSort (void *d1, void *d2)
{
    Bug *p1 = (Bug *)(d1);
    Bug *p2 = (Bug *)(d2);
    long date1;
    long date2;

    uDebug (8, ("Bug Movement Sorting : %d, %d\n", BUGNO(p1), BUGNO(p2)));
    
    date1 = p1->date;
    if (date1 < p1->unsusDate)
        date1 = p1->unsusDate;
    if (date1 < p1->susDate)
        date1 = p1->susDate;
    if (date1 < p1->fixDate)
        date1 = p1->fixDate;
    
    date2 = p2->date;
    if (date2 < p2->unsusDate)
        date2 = p2->unsusDate;
    if (date2 < p2->susDate)
        date2 = p2->susDate;
    if (date2 < p2->fixDate)
        date2 = p2->fixDate;
    
    if (date1 == date2)
        return (BUGNO(p2) - BUGNO(p1));
    return ((date1 >= date2) ? -1 : 1);
}

static char *
bugIndexMovement (void)
{
    static char *title = "Bugs - Chronological Movement";

    ListItem *p;
    ListItem *sortHead;
    Bug      *b;

    /* Opening page stuff. */
    hmlFileHeader (title, NULL);
    hmlIndent (1);
    hmlPara ();
    hmlFormatStr ("This menu is a chronologically date ordered list "
                   "which shows when the bugs last changed status "
                   "The status change may be closed or suspended.");
    hmlPara ();
    hmlFormatStr ("Items shown in bold are open, items in italic "
                  "are rejected, regular text items are closed.");
    hmlPara ();

    /* Duplicate the BUG list and sort according to reported by date. */
    sortHead = ListDuplicate (&bugHead->list);
    ListSort (&sortHead, bugMovementSort);

    /* Produce the menu */
    for (p = sortHead; p != NULL; p = p->next)
    {
        int date;
        b = (Bug *)(p->data);
        date = b->date;
        if (b->fixDate > date)
            date = b->fixDate;
        if (b->susDate > date)
            date = b->susDate;
        bugMenuDateItem (b, date, 1);
    }
    hmlIndent (-1);
    hmlEnd ();

    ListDestructAll (&sortHead);
    return (title);
}

static char *
bugReport (void)
{
    static char *title = "Submit Bug Report";
    Name *n;
    Release *r;
    char cc;
     
    /* Openning page stuff */
    hmlFileHeader (title, "report");
    
    hmlStr ("  <form method=post form=\"application/x-www-form-urlencoded\" action=\"%s/cgi-bin/bugcgi\">\n", wwwroot);
    hmlStr ("  <input type=hidden name=\"Type:\" value=\"bug\">\n");
    hmlStr ("  <input type=hidden name=\"Project:\" value=\"%s\">\n", project);
    hmlStr ("  <table>\n");
    hmlStr ("    <tr>\n");
    hmlStr ("      <td valign=top><font color=\"#ff0000\"><b>Name:</b></font></td>\n");
    hmlStr ("      <td><i>Your name, always use the same name when submitting reports.</i><br>\n");
    hmlStr ("      <input type=text name=\"Report by:\" size=48 maxlength=80></td>\n");
    hmlStr ("    </tr>\n");
    hmlStr ("    <tr>\n");
    hmlStr ("      <td valign=top><font color=\"#ff0000\"><b>Email:</b></font></td>\n");
    hmlStr ("      <td><i>Your email address, used if there are problems with your submission.</i><br>\n");
    hmlStr ("      <input type=text name=\"Email:\" size=48 maxlength=80></td>\n");
    hmlStr ("    </tr>\n");
    hmlStr ("    <tr>\n");
    hmlStr ("      <td valign=top><font color=\"#ff0000\"><b>Module:</b></font></td>\n");
    hmlStr ("      <td>\n");
    hmlStr ("      <i>The system module when the fault is occuring.</i></br>\n");
    hmlStr ("      <select name=\"Module:\" size=1>\n");
    
    /* List the available modules */
    for (n = swmodules; n != NULL; n = n->next)
        hmlStr ("      <option>%s\n", n->name[0]);
    hmlStr ("      </select></td>\n");
    hmlStr ("    </tr>\n");
    hmlStr ("    <tr>\n");
    hmlStr ("      <td valign=top><font color=\"#ff0000\"><b>Build:</b></font></td>\n");
    hmlStr ("      <td><i>The build, or release against which the fault is being logged</i><br>\n");
    hmlStr ("      <select name=\"Release\" size=1>\n");
    
    /* List available releases */
   for (r = releaseHead; (r = RELNEXT(r)) != NULL; /* NULL */)
       hmlStr ("      <option>%s\n", r->name->name[0]);
    hmlStr ("      </select></td>\n");
    hmlStr ("    </tr>\n");
    hmlStr ("    <tr>\n");
    hmlStr ("      <td valign=top><b>System:</b></td>\n");
    hmlStr ("      <td><i>An optional field. A short text description that identifies the\n");
    hmlStr ("      system platform on which the fault is occuring. This may be an operating\n");
    hmlStr ("      system or hardware revision</i><br>\n");
    hmlStr ("      <input type=text name=\"System:\" size=24 maxlength=80></td>\n");
    hmlStr ("    </tr>\n");
    hmlStr ("    <tr>\n");
    hmlStr ("      <td valign=top><b>Engineer:</b></td>\n");
    hmlStr ("      <td><i>An optional field. The name of the engineer assigned to fix the problem</i><br>\n");
    hmlStr ("      <input type=text name=\"Engineer:\" size=24 vale=\"?\" maxlength=80></td>\n");
    hmlStr ("    </tr>\n");
    hmlStr ("    <tr>\n");
    hmlStr ("      <td valign=top><font color=\"#ff0000\"><b>Severity:</b></font></td>\n");
    hmlStr ("      <td><i>The severity of the fault</i><br>\n");
    
    /* Severity selection */
    for (cc = 0; cc < 5; cc++)
        hmlStr ("      <input type=radio name=\"Severity:\" value=\"%c\"> %s<br>\n",
                 cc + 'A', strSeverity[cc+1]);
    hmlStr ("      </td>\n");
    hmlStr ("    </tr>\n");
    hmlStr ("    <tr>\n");
    hmlStr ("      <td valign=top><font color=\"#ff0000\"><b>Summary:</b></font></td>\n");
    hmlStr ("      <td><i>A short one line description of the problem</i><br>\n");
    hmlStr ("      <input type=text name=\"Summary:\" size=60 maxlength=80></td>\n");
    hmlStr ("    </tr>\n");
    hmlStr ("    <tr>\n");
    hmlStr ("      <td valign=top><font color=\"#ff0000\"><b>Description:</b></font></td>\n");
    hmlStr ("      <td><i>A detailed explaination of the problem, formatted text\n");
    hmlStr ("      (i.e. source code fragments) should be entered into the\n");
    hmlStr ("      <b>Literal text</b> area</i><br>\n");
    hmlStr ("      <textarea name=\"Description:\" cols=60 rows=20 wrap=physical></textarea></td>\n");
    hmlStr ("    </tr>\n");
    hmlStr ("    <tr>\n");
    hmlStr ("      <td valign=top><b>Literal<br>Text:</b></td>\n");
    hmlStr ("      <td><i>Optional field (<b>Netscape Only</b>), allows entry of layout text such as source code,\n");
    hmlStr ("      the line style is presereved in this area, this may be used to suplement\n");
    hmlStr ("      the description.</i></br>\n");
    hmlStr ("      <textarea name=\"Literal:\" cols=60 rows=5 <wrap=off></textarea></td>\n");
    hmlStr ("    </tr>\n");
    hmlStr ("    <tr>\n");
    hmlStr ("      <td valign=top><b>Notes:</b></td>\n");
    hmlStr ("      <td><i>Additional notes that may be useful that are part of the problem\n");
    hmlStr ("      description.</i><br>\n");
    hmlStr ("      <textarea name=\"Notes:\" cols=60 rows=5 wrap=physical></textarea></td>\n");
    hmlStr ("    </tr>\n");
    hmlStr ("  </table>\n");
    hmlStr ("  <hr>\n");
    hmlStr ("  <input type=hidden name=\"E\" value=\"E\">\n");
    hmlStr ("  <input type=submit value=\"Submit Report\">\n");
    hmlStr ("  <input type=reset  value=\"Reset Form\">\n");
    hmlStr ("  </form>\n");
    hmlEnd ();
    return title;
}
    

char *
bugIndex (void)
{
    typedef char *charp;
    static char *title = "Bug Reports";
    charp buffer [20];
    charp *p = buffer;
    
    /* Output the general index */
    *p++ = bugIndexCurrent ();
    *p++ = bugIndexModules ();
    *p++ = bugIndexReportedBy ();
    *p++ = bugIndexEngineer ();
    *p++ = bugIndexNumerical ();
    *p++ = bugIndexReport ();
    *p++ = bugIndexMovement ();
    if (hmlFormat() == HML_FORMAT_HTML)
        *p++ = bugReport ();
    *p++ = NULL;

    /* Output the bug page */
    hmlFileHeader (title, NULL);
    hmlDefinition ("bug");
    hmlIndent (1);
    hmlPara ();
    
    /* Output the bug index */
    for (p = buffer; *p != NULL; p++)
    {
        uDebug (1, ("Menu item %s\n", *p));
        hmlXref (*p, *p);
        hmlBreak ();
    }
    hmlIndent (-1);
    
    hmlEnd ();
    
    return (title);
}
