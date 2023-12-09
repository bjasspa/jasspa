/****************************************************************************
 *
 *  			Copyright 1996 Jon Green.
 *                         All Rights Reserved
 *
 *
 *  System        : 
 *  Module        : 
 *  Object Name   : $RCSfile: inf.c,v $
 *  Revision      : $Revision: 1.1 $
 *  Date          : $Date: 2000-10-21 14:31:23 $
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

static Inf *infHead = NULL;

static void
infOutputMenuItem (Inf *p)
{
#if 0
    hmlXref (p->summary, "%s", p->summary);
    hmlBreak ();
#else
    hmlXref (hmlXrefPageName ("inf%04d", INFNO(p)), "#%04d", INFNO(p));
    hmlFormatStr (" %s", p->summary);
    hmlBreak ();
#endif

}

char *
infIndex (void)
{
    static char *title = "General Information";
    Inf *p;
    
    /* Opening page stuff. */
    hmlFileHeader (title, NULL);
    hmlDefinition ("inf");
    hmlIndent (1);
    hmlPara ();
    
    for (p = infHead; p != NULL; p = INFNEXT (p))
        infOutputMenuItem (p);
    hmlIndent (-1);
    hmlEnd ();
    
    return (title);
}    

static void
infOutput (Inf *p)
{
    /* Opening page stuff. */
/*    hmlFileHeader (p->summary, NULL);*/
    hmlNumericFileHeader ("inf", INFNO(p), "Information Page #%d");
    
    hmlTitle (1, "Synopsis");
    hmlIndent (1);
    hmlPara ();
    hmlFormatStr ("%s", p->summary);
    hmlIndent (-1);
    
    /* Synopsis */
    hmlDescription ("Details", 1, &p->description);
    hmlDescription ("Notes", 1, &p->notes);
    hmlReference   ("See Also", 1, &p->seeAlso);
    hmlEnd ();
}        
        
static void
infCheck (Inf *p)
{
    Inf *tp;
    
    for (tp = infHead; tp != NULL; tp = INFNEXT(tp))
        if (INFNO(tp) == INFNO(p))
            uWarn ("Duplication information number [%d]\n", INFNO(p));
    if (p->summary == NULL)
    {
        uError ("Summay expected\n");
        p->summary = bufFormat (NULL, "UNDEFINED #%d", INFNO(p));
    }
    if (p->description.argv == NULL)
        uWarn ("Description required\n");
}

Inf *
infEnd (Inf *inf)
{
    if (inf != NULL)
    {
        infCheck (inf);
        INFNO(inf) = -INFNO(inf);       /* Reverse insertion numbering */
        ListItemInsert ((ListItem **)(&infHead), &inf->list);
        INFNO(inf) = -INFNO(inf);       /* Restore insertion numbering */
        infOutput (inf);
        bufArgFree (&inf->description);
        bufArgFree (&inf->notes);
        bufArgFree (&inf->seeAlso);
    }
    return (NULL);
}

void
infInit (void)
{
    hmlAddQuick ("inf");
}

