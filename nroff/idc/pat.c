/****************************************************************************
 *
 *  			Copyright 1996 Jon Green.
 *                         All Rights Reserved
 *
 *
 *  System        : 
 *  Module        : 
 *  Object Name   : $RCSfile: pat.c,v $
 *  Revision      : $Revision: 1.1 $
 *  Date          : $Date: 2000-10-21 14:31:24 $
 *  Author        : $Author: jon $
 *  Last Modified : <250896.1704>
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

static const char rcsid[] = "@(#) : $Id: pat.c,v 1.1 2000-10-21 14:31:24 jon Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "idc.h"

static Patch *patchHead = NULL;

static void
patchOutputMenuItem (Patch *p)
{
    hmlXref (hmlXrefPageName ("pat%04d", PATNO(p)), "#%04d", PATNO(p));
    hmlFormatStr (" [%s] %s", p->release->name[0], p->summary);
    hmlBreak ();
}

char *
patchIndex (void)
{
    static char *title = "Patch Reports";
    Patch *p;
    
    /* Openning page stuff. */
    hmlFileHeader (title, NULL);
    hmlDefinition ("patch");
    hmlIndent (1);
    hmlPara ();
    
    for (p = patchHead; p != NULL; p = PATNEXT (p))
        patchOutputMenuItem (p);
    hmlIndent (-1);
    hmlEnd ();
    
    return (title);
}    

static Bug *
patchFindNextBug (Bug *b, int patchNo)
{
    if (b == NULL)
        b = bugGetHead();
    else 
        b = BUGNEXT(b);
    
    while (b != NULL)
    {
        if (b->patchNo == patchNo)
            break;
        b = BUGNEXT (b);
    }
    return (b);
}

static void
patchOutput (Patch *p)
{
    Bug *b;
    int count;
    
    /* Opening page stuff. */
    hmlNumericFileHeader ("pat", PATNO(p), "Patch Report #%d");
    
    /* Synopsis */
    hmlTitle (1, "Synopsis");
    hmlIndent (1);
    hmlPara ();
    hmlFormatStr ("%s", p->summary);
    hmlIndent (-1);
    
    /* Details */
    hmlTitle (1, "Details");
    hmlIndent (1);
    if (p->name != NULL)
        hmlBullet ("%s", p->name->name[0]);
    hmlBullet ("%s", strDate (p->date));
    if (p->release != NULL)
        hmlBullet ("%s", p->release->name[0]);
    hmlIndent (-1);
    hmlLine ();;
    
    /* Description */
    hmlDescription ("Affecting", 1, &p->affecting);
    hmlDescription ("Work Around", 1, &p->workaround);
    hmlDescription ("Description", 1, &p->description);
    hmlDescription ("Notes", 1, &p->notes);
    hmlReference ("See Also", 1, &p->seeAlso);
    
    /*
     * Output a list of bugs closed by the patch.
     */
    
    if ((b = patchFindNextBug (NULL, PATNO(p))) != NULL)
    {
        count = 0;
        hmlTitle (1, "Bugs closed by this patch.");
        hmlIndent (1);
        hmlPara ();
        do
        {
            bugMenuItem (b, 0);
            count++;
        }
        while ((b = patchFindNextBug (b, PATNO(p))) != NULL);
        outputItemCount (count, -1);
        hmlIndent (-1);
        hmlLine ();
    }
    hmlEnd ();
}        
        
static void
patchCheck (Patch *p)
{
    Patch *tp;
    
    for (tp = patchHead; tp != NULL; tp = PATNEXT(tp))
        if (PATNO(tp) == PATNO(p))
            uWarn ("Duplication patch number [%d]\n", PATNO(p));
    if (p->date == 0)
        uWarn ("Patch report must be dated\n");
    if (p->summary == NULL)
        uWarn ("Patch report must have summary\n");
    if (p->description.argv == NULL)
        uWarn ("Description required\n");
    if (p->release == NULL)
        uWarn ("Release not assigned\n");
}


Patch *
patchEnd (Patch *patch)
{
    if (patch != NULL)
    {
        patchCheck (patch);
        ListItemInsert ((ListItem **)(&patchHead), &patch->list);
        patchOutput (patch);
    }   
    return (NULL);
}

Patch *patchGetHead (void)
{
    return (patchHead);
}

void
patchInit (void)
{
    hmlAddQuick ("patch");
}


