/****************************************************************************
 *
 *  			Copyright 1996 Jon Green.
 *                         All Rights Reserved
 *
 *
 *  System        : 
 *  Module        : 
 *  Object Name   : $RCSfile: faq.c,v $
 *  Revision      : $Revision: 1.1 $
 *  Date          : $Date: 2000-10-21 14:31:21 $
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

static const char rcsid[] = "@(#) : $Id: faq.c,v 1.1 2000-10-21 14:31:21 jon Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "idc.h"

static Faq *faqHead = NULL;
static char *faqTitle = "Frequently Asked Questions";
static char *faqLabelledTitle = "Frequently Asked Questions [Labelled]";

static void
faqLabelledIndex (void)
{
    Faq *p;
    
    /* Openning page stuff. */
    hmlFileHeader (faqLabelledTitle, NULL);
    hmlIndent (1);
    hmlPara ();
    hmlXref (faqTitle, "Frequently asked questions - Alphabetic list.");
    hmlPara ();
    
    for (p = faqHead; p != NULL; p = FAQNEXT (p))
    {
        hmlXref (hmlXrefPageName ("faq%04d", FAQNO(p)), "#%04d", FAQNO(p));
        hmlFormatStr (" %s", p->question);
        hmlBreak ();
    }
    
    hmlIndent (-1);
    hmlEnd ();
}    

static int
faqSort (void *d1, void *d2)
{
    Faq *p1 = (Faq *)(d1);
    Faq *p2 = (Faq *)(d2);
    
    uDebug (8, ("Sorting : %d, %d\n    %d:%s\n    %d:%s\n",
                FAQNO(p1), FAQNO(p2),
                FAQNO(p1), p1->question,
                FAQNO(p2), p2->question));
    return (strcasecmp (p1->question, p2->question));
}

char *
faqIndex (void)
{
    ListItem *p;
    ListItem *sortHead;
    Faq      *f;
    
    /* Produce labelled list of FAQ */
    faqLabelledIndex ();
    
    /* Duplicate the FAQ list and sort according to alphabetic order. */
    sortHead = ListDuplicate (&faqHead->list);
    ListSort (&sortHead, faqSort);
    
    /* Opening page stuff. */
    hmlFileHeader (faqTitle, NULL);
    hmlDefinition ("faq");
    hmlIndent (1);
    
    hmlPara ();
    hmlXref (faqLabelledTitle,
             "Frequently asked questions - listed by identity.");
    
    hmlPara ();
    hmlListOpen ();
    for (p = sortHead; p != NULL; p = p->next)
    {
        f = (Faq *)(p->data);
        
        hmlListTag ();
        hmlXref (hmlXrefPageName ("faq%04d", FAQNO(f)), "Q.");
        
        hmlListData ();
        hmlFormatStr ("%s", f->question);
    }
    hmlListClose ();
    hmlIndent (-1);
    hmlEnd ();
    
    ListDestructAll (&sortHead);
    return (faqTitle);
}    

static void
faqOutput (Faq *p)
{
    /* Opening page stuff. */
    hmlNumericFileHeader ("faq", FAQNO(p), "Faq #%d");
    
    /* Synopsis */
    hmlTitle (1, "Question");
    hmlIndent (1);
    hmlPara ();
    hmlFormatStr ("%s", p->question);
    hmlIndent (-1);
    
    hmlDescription ("Answer", 1, &p->description);
    hmlDescription ("Notes", 1, &p->notes);
    hmlReference ("See Also", 1, &p->seeAlso);
    
    /* Details */
    if ((p->module != NULL) ||
        (p->release != NULL) ||
        (p->name != NULL))
    {
        hmlTitle (1, "Identification");
        hmlIndent (1);
        if (p->module != NULL)
            hmlBullet ("%s", p->module);
        hmlBullet ("%s", strDate (p->date));
        if (p->release != NULL)
            hmlBullet ("%s", p->release->name[0]);
        if (p->name != NULL)
            hmlBullet ("%s", p->name->name[0]);
        hmlIndent (-1);
    }
    
    hmlEnd ();
}        
        
static void
faqCheck (Faq *p)
{
    Faq *tp;
    
    for (tp = faqHead; tp != NULL; tp = FAQNEXT(tp))
        if (FAQNO(tp) == FAQNO(p))
            uWarn ("Duplicated faq number [%d]\n", FAQNO(p));
    if (p->date == 0)
        uWarn ("Frequently asked question must be dated\n");
    if (p->question == NULL)
        uWarn ("Frequently Asked question must have question\n");
    else if (isalpha (p->question [0]) == 0)
        uError ("Question should commence with Alphabetic\n");
    if (p->description.argv == NULL)
        uWarn ("Description required\n");
    if (p->name == NULL)
        uWarn ("Name required.\n");
}

void 
faqSemiDestruct (Faq *p)
{
    bufArgFree (&p->description);
    bufArgFree (&p->seeAlso);
    bufArgFree (&p->notes);
}

Faq *
faqEnd (Faq *faq)
{
    if (faq != NULL)
    {
        faqCheck (faq);
        ListItemInsert ((ListItem **)(&faqHead), &faq->list);
        faqOutput (faq);
        faqSemiDestruct (faq);
    }
    return (NULL);
}

void
faqInit (void)
{
    hmlAddQuick ("faq");
}

