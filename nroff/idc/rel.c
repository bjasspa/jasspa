/****************************************************************************
 *
 *  			Copyright 1996 Jon Green.
 *                         All Rights Reserved
 *
 *
 *  System        : 
 *  Module        : 
 *  Object Name   : $RCSfile: rel.c,v $
 *  Revision      : $Revision: 1.1 $
 *  Date          : $Date: 2000-10-21 14:31:24 $
 *  Author        : $Author: jon $
 *  Last Modified : <990831.2340>
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

static const char rcsid[] = "@(#) : $Id: rel.c,v 1.1 2000-10-21 14:31:24 jon Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "idc.h"

static char *strState [] = {
    "Unknown",
    "Full",
    "Specified",
    "Patched"
};

Release *releaseHead = NULL;

/*
 * Constructors for the Release Information.
 */

static void
releaseDestruct (Release *p)
{
    if (p == NULL)
        return;

    bufArgFree (&p->description);
    bufArgFree (&p->notes);

    /* Clear the buffer */
    memset (p, 0, sizeof (Release));
}

static void
releaseSemiDestruct (Release *p)
{
    if (p == NULL)
        return;

    bufArgFree (&p->description);
    bufArgFree (&p->notes);
}


static void
releaseCheck (Release *p)
{
    Release *tp;

    for (tp = releaseHead; tp != NULL; tp = RELNEXT(tp))
        if (RELNO(tp) == RELNO(p))
            uWarn ("Duplication release number [%d]\n", RELNO(p));
    if (p->date == 0)
        uWarn ("Release report must be dated\n");
    if (p->state == 0)
        uWarn ("Release report must be assigned a state\n");
    if (p->description.argv == NULL)
        uWarn ("Description required\n");
}

Release *
releaseEnd (Release *release)
{
    if (release != NULL)
    {
        releaseCheck (release);
        ListItemInsert ((ListItem **)(&releaseHead), &release->list);
        releaseSemiDestruct (release);
    }
    return (NULL);
}

static Bug *
bugFindReleaseNext (Bug *p, int category, int status, Name *n)
{

    p = (p == NULL) ? bugGetHead() : BUGNEXT(p);
    while (p)
    {
        if (((category == 0) || (p->severity == category)) &&
            (((status == STATUS_OPEN) && (p->release->mode <= n->mode) &&
              ((p->status == STATUS_OPEN) ||
               ((p->status == STATUS_CLOSED) &&
                (p->fixRelease != NULL) &&
                (p->fixRelease->mode > n->mode)))) ||
             ((status == STATUS_SUSPENDED) && (p->status == STATUS_SUSPENDED) && (p->release->mode <= n->mode)) ||
             ((status == STATUS_CLOSED) && (p->status == STATUS_CLOSED) &&
              (p->fixRelease != NULL) && (p->fixRelease->mode == n->mode))))
            return (p);
        else
            p = BUGNEXT(p);
    }
    return (NULL);
}

static void
releaseIndexPage (char *title, Release *r)
{
    int category;
    int count;
    int aCount;
    Name *n;
    Bug *p;

    n = r->name;
    /* Openning page stuff. */
    hmlFileHeader (title, NULL);
    hmlIndent (1);
    hmlBullet ("%s", strDate (r->date));
    hmlBullet ("%s", strState [r->state]);
    hmlIndent (-1);
    hmlDescription ("Description", 1, &r->description);
    hmlDescription ("Notes", 1, &r->notes);

    if (bugFindReleaseNext (NULL, 0, STATUS_OPEN, n) != NULL)
    {
        hmlTitle (0, "Bugs present in this release. ");
        hmlIndent(1);
        hmlPara ();
        hmlFormatStr ("Items shown in bold are open, items in italic "
                      "are rejected, regular text items are closed.");
        hmlIndent (-1);
        for (category = SEVERITY_A; category <= SEVERITY_MAX; category++)
        {
            if ((p = bugFindReleaseNext (NULL, category, STATUS_OPEN, n)) == NULL)
                continue;

            hmlTitle (1, "Category %c", 'A' + category - 1);
            hmlIndent (1);
            hmlPara ();
            count = 0;
            aCount = 0;
            do {
                bugMenuItem (p, 1);
                if (p->status == STATUS_OPEN)
                    aCount++;
                count++;
            } while ((p = bugFindReleaseNext (p, category, STATUS_OPEN, n)) != NULL);

            outputItemCount (count, aCount);
            hmlIndent (-1);
        }
        hmlLine ();
    }
    if ((p = bugFindReleaseNext (NULL, 0, STATUS_SUSPENDED, n)) != NULL)
    {
        hmlTitle (0, "Bugs suspended through this release.");
        hmlIndent (1);
        hmlPara ();

        count = 0;
        do {
            bugMenuItem (p, 0);
            count++;
        } while ((p = bugFindReleaseNext (p, 0, STATUS_SUSPENDED, n)) != NULL);
        outputItemCount (count, -1);
        hmlIndent (-1);
        hmlLine ();
    }
    if ((p = bugFindReleaseNext (NULL, 0, STATUS_REJECTED, n)) != NULL)
    {
        hmlTitle (0, "Bugs rejected in this release.");
        hmlIndent (1);
        hmlPara ();

        count = 0;
        do {
            bugMenuItem (p, 0);
            count++;
        } while ((p = bugFindReleaseNext (p, 0, STATUS_REJECTED, n)) != NULL);
        outputItemCount (count, -1);
        hmlIndent (-1);
        hmlLine ();
    }
    if ((p = bugFindReleaseNext (NULL, 0, STATUS_CLOSED, n)) != NULL)
    {
        hmlTitle (0, "Bugs closed by this release.");
        hmlIndent (1);
        hmlPara ();

                count = 0;
        do {
            bugMenuItem (p, 0);
            count++;
        } while ((p = bugFindReleaseNext (p, 0, STATUS_CLOSED, n)) != NULL);

        outputItemCount (count, -1);
        hmlIndent (-1);
        hmlLine ();
    }
    hmlEnd ();
}

char *
releaseIndex (void)
{
    static char *title = "Release Reports";
    static char *releasePageName = "Release %s";
    Release *r;
    char *s;

    /* Generate the release index */
    hmlFileHeader (title, NULL);
    hmlDefinition ("rel");
    hmlIndent (1);
    hmlPara ();
    hmlFormatStr ("%s",
                  "The following is a list of release bug assignments. "
                  "The reports show the bugs present in a given release "
                  "without removing bugs that may have been subsequently "
                  "solved after the release. "
                  "The report also includes a list of bugs from previous "
                  "releases which have been closed by the release.");
    hmlPara ();

    /* Go to the end of the list and then come back in reverse. */
    for (r = releaseHead; (r = RELNEXT(r)) != NULL; /* NULL */)
    {
        hmlBullet ("");
        hmlXref (hmlXrefPageName (releasePageName, r->name->name[0]),
                 "%s", r->name->name[0]) ;
    }
    hmlIndent (-1);
    hmlLine ();
    hmlEnd ();
    
    /* Generate the release pages */
    for (r = releaseHead; ((r = RELNEXT(r)) != NULL); /* NULL */)
    {
        uInteractive ("Generating release reports for [%s]",
                  r->name->name[0]);
        s = bufFormat (NULL, releasePageName, r->name->name[0]);
        releaseIndexPage (s, r);
        bufFree (s);
    }
    return (title);
}

void
relInit (void)
{
    hmlAddQuick ("rel");
}

