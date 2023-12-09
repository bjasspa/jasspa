/****************************************************************************
 *
 *  			Copyright 1996 Jon Green.
 *                         All Rights Reserved
 *
 *
 *  System        : 
 *  Module        : 
 *  Object Name   : $RCSfile: tek.c,v $
 *  Revision      : $Revision: 1.1 $
 *  Date          : $Date: 2000-10-21 14:31:25 $
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

static Tek *tekHead = NULL;
static char *tekTitle = "Technical Reports [Alphabetic]";
static char *tekLabelledTitle = "Technical Reports [Numeric]";

static void
tekLabelledIndex (void)
{
    Tek *p;
    
    /* Openning page stuff. */
    hmlFileHeader (tekLabelledTitle, NULL);
    hmlIndent (1);
    hmlPara ();
    hmlXref (tekTitle, "Tehcnical Reports - Alphabetic.");
    hmlPara ();
    
    for (p = tekHead; p != NULL; p = TEKNEXT (p))
    {
        hmlXref (hmlXrefPageName ("tek%04d", TEKNO(p)), "#%04d", TEKNO(p));
        hmlFormatStr (" [%s] %s", p->module, p->summary);
/*        hmlFormatStr (" %s", p->summary);*/
        hmlBreak ();
    }
    
    hmlIndent (-1);
    hmlEnd ();
}    

static int
tekSort (void *d1, void *d2)
{
    int status;
    Tek *p1 = (Tek *)(d1);
    Tek *p2 = (Tek *)(d2);
    
    uDebug (8, ("Sorting : %d, %d\n    %d:%s\n    %d:%s\n",
                TEKNO(p1), TEKNO(p2),
                TEKNO(p1), p1->summary,
                TEKNO(p2), p2->summary));
    
    status = strcasecmp (p1->module, p2->module);
    if (status == 0)
        status = strcasecmp (p1->summary, p2->summary);
    return (status);
}

#if 0
static void
tekOutputMenuItem (Tek *p)
{
    hmlXref (hmlXrefPageName ("tek%04d", TEKNO(p)), "#%04d", TEKNO(p));
    hmlFormatStr (" [%s] %s", p->module, p->summary);
    hmlBreak ();
}
#endif

char *
tekIndex (void)
{
    ListItem *p;
    ListItem *sortHead;
    Tek *t;
    
    /* Produce labelled list of TEK */
    tekLabelledIndex ();

    /* Duplicate the TEK list and sort according to alphabetic order. */
    sortHead = ListDuplicate (&tekHead->list);
    ListSort (&sortHead, tekSort);
    
    /* Opening page stuff. */
    hmlFileHeader (tekTitle, NULL);
    hmlDefinition ("tek");
    hmlIndent (1);
    
    hmlPara ();
    hmlXref (tekLabelledTitle,
             "Technical Reports - Numeric.");
    
    hmlPara ();
    hmlListOpen ();
    for (p = sortHead; p != NULL; p = p->next)
    {
        t = (Tek *)(p->data);
        
        hmlListTag ();
        hmlXref (hmlXrefPageName ("tek%04d", TEKNO(t)), "#%04d", TEKNO(t));
        
        hmlListData ();
        hmlFormatStr (" [%s] %s", t->module, t->summary);
    }
    hmlListClose ();
    hmlIndent (-1);
    hmlEnd ();
    
    ListDestructAll (&sortHead);
    
#if 0    
    /* Openning page stuff. */
    hmlFileHeader (tekTitle, NULL);
    hmlDefinition ("tek");
    hmlIndent (1);
    hmlPara ();
    
    for (p = tekHead; p != NULL; p = TEKNEXT (p))
        tekOutputMenuItem (p);
    hmlIndent (-1);
    hmlEnd ();
#endif    
    return (tekTitle);
}    

static void
tekOutput (Tek *p)
{
    /* Opening page stuff. */
    hmlNumericFileHeader ("tek", TEKNO(p), "Technical Report #%d");
    
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
    hmlBullet (strDate (p->date));
    if (p->release != NULL)
        hmlBullet ("%s", p->release->name[0]);
    if (p->reportBy)
        hmlBullet ("%s", p->reportBy);
    hmlIndent (-1);
    hmlLine ();;
    
    /* Description */
    hmlDescription ("Description", 1, &p->description);
    hmlDescription ("Notes", 1, &p->notes);
    hmlReference ("See Also", 1, &p->seeAlso);
    hmlEnd ();
}        
        
static void
tekCheck (Tek *p)
{
    Tek *tp;
    
    for (tp = tekHead; tp != NULL; tp = TEKNEXT(tp))
        if (TEKNO(tp) == TEKNO(p))
            uWarn ("Duplication tek number [%d]\n", TEKNO(p));
    if (p->date == 0)
        uWarn ("Technical report must be dated\n");
    if (p->summary == NULL)
        uWarn ("Technical report must have summary\n");
    if (p->description.argv == NULL)
        uWarn ("Description required\n");
    if (p->module == NULL)
        uWarn ("Module not assigned\n");
}

Tek *
tekEnd (Tek *tek)
{
    if (tek != NULL)
    {
        tekCheck (tek);
        ListItemInsert ((ListItem **)(&tekHead), &tek->list);
        tekOutput (tek);
        
        /* Free off stuff not required again */
        bufArgFree (&tek->description);
        bufArgFree (&tek->notes);
        bufArgFree (&tek->seeAlso);
    }
    return (NULL);
}


void
tekInit (void)
{
    hmlAddQuick ("tek");
}
