/****************************************************************************
 *
 *  			Copyright 1996 Jon Green.
 *                         All Rights Reserved
 *
 *
 *  System        : 
 *  Module        : 
 *  Object Name   : $RCSfile: new.c,v $
 *  Revision      : $Revision: 1.1 $
 *  Date          : $Date: 2000-10-21 14:31:24 $
 *  Author        : $Author: jon $
 *  Last Modified : <231205.1243>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "idc.h"

static New *newHead = NULL;

static void
newOutputMenuItem (New *p)
{
    hmlXref (hmlXrefPageName ("new%04d", NEWNO(p)), "#%04d", NEWNO(p));
    hmlFormatStr (" %s", p->summary);
    hmlBreak ();
}

char *
newIndex (void)
{
    static char *title = "News Items";
    New *p;
    
    /* Openning page stuff. */
    hmlFileHeader (title, NULL);
    hmlDefinition ("new");
    hmlIndent (1);
    hmlPara ();
    
    for (p = newHead; p != NULL; p = NEWNEXT (p))
        newOutputMenuItem (p);
    hmlIndent (-1);
    hmlEnd ();
    
    return (title);
}    

static void
newOutput (New *p)
{
    /* Opening page stuff. */
    hmlNumericFileHeader ("new", NEWNO(p), "News Report #%d");
    
    /* Synopsis */
    hmlTitle (1, "Synopsis");
    hmlIndent (1);
    hmlPara ();
    hmlFormatStr ("%s", p->summary);
    hmlIndent (-1);
    
    /* Description */
    hmlDescription ("Description", 1, &p->description);
    hmlDescription ("Notes", 1, &p->notes);
    hmlReference ("See Also", 1, &p->seeAlso);
    
    /* Details */
    hmlTitle (1, "Identification");
    hmlIndent (1);
    if (p->module)
        hmlBullet ("%s", p->module);
    hmlBullet ("%s", strDate (p->date));
    if (p->release != NULL)
        hmlBullet ("%s", p->release->name[0]);
    hmlBullet ("%s", p->reportBy);
    hmlIndent (-1);
   
    hmlEnd ();
}        
        
static void
newCheck (New *p)
{
    New *tp;
    
    for (tp = newHead; tp != NULL; tp = NEWNEXT(tp))
        if (NEWNO(tp) == NEWNO(p))
            uWarn ("Duplication new number [%d]\n", NEWNO(p));
    if (p->date == 0)
        uWarn ("News report must be dated\n");
    if (p->summary == NULL)
        uWarn ("News report must have summary\n");
    if (p->description.argv == NULL)
        uWarn ("Description required\n");
}

New *
newEnd (New *new)
{
    if (new != NULL)
    {
        newCheck (new);
        ListItemInsert ((ListItem **)(&newHead), &new->list);
        newOutput (new);
    }
    return (NULL);
}


void
newInit (void)
{
    hmlAddQuick ("new");
}

