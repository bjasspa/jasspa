/****************************************************************************
 *
 *  			Copyright 1996 Jon Green.
 *                         All Rights Reserved
 *
 *
 *  System        : 
 *  Module        : 
 *  Object Name   : $RCSfile: list.c,v $
 *  Revision      : $Revision: 1.1 $
 *  Date          : $Date: 2000-10-21 14:31:23 $
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

static const char rcsid[] = "@(#) : $Id: list.c,v 1.1 2000-10-21 14:31:23 jon Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"

ListItem *
ListItemConstruct (int size)
{
    ListItem *p;
    
    p = (ListItem *) malloc (size);
    memset (p, 0, size);
    return (p);
}

void
ListItemInsert (ListItem **head, ListItem *p)
{
    ListItem *q;
    ListItem *r;
    
    if ((q = *head) == NULL)
    {
        *head = p;
        p->next = NULL;
        p->prev = NULL;
        return;
    }
    
    while (p->sort < q->sort)
    {
        if (q->next == NULL) {
            p->next = NULL;
            p->prev = q;
            q->next = p;
            return;
        }
        else
            q = q->next;
    }
    if ((r = q->prev) == NULL)
        *head = p;
    else
        r->next = p;
    p->prev = r;
    q->prev = p;
    p->next = q;
}        

ListItem *
ListDuplicate (ListItem *head)
{
    ListItem *p;                        /* Current list item */
    ListItem *n;                        /* New list item */
    ListItem *r;                        /* Replicated head */
    ListItem *t;                        /* Replicated tail */
    
    r = NULL;
    t = NULL;
    for (p = head; p != NULL; p = p->next)
    {
        n = ListItemConstruct (sizeof (ListItem));
        if (t == NULL)
        {
            r = n;
            t = n;
        }
        else
        {
            t->next = n;
            n->prev = t;
            t = n;
        }
        n->data = (void *)(p);
    }
    return (r);
}
    

void
ListDestruct (ListItem **head, ListItem *p)
{
    if ((head == NULL) || (p == NULL))
        return;
    /* Fix up previous node */
    if (p->prev != NULL)
        p->prev->next = p->next;
    else
        *head = p->next;
    /* Fix up next node */
    if (p->next != NULL)
        p->next->prev = p->prev;
    free (p);
}
        
        
    

void
ListDestructAll (ListItem **head)
{
    if (head == NULL)
        return;
    while (*head != NULL)
        ListDestruct (head, *head);
}
    

void 
ListSort (ListItem **head, ListSortFunc func)
{
    
    ListItem *p;
    ListItem *r;
    int resort;
    
    if ((head == NULL) || (*head == NULL))
        return;
    
    do
    {
        resort = 0;
        for (p = *head; p->next != NULL; p = p->next)
            if (func (p->data, p->next->data) > 0)
            {
                r = p->next;
                /* Sort out parent linkage */
                if (p->prev == NULL)
                    *head = r;
                else
                    p->prev->next = r;
                r->prev = p->prev;
                p->prev = r;
                /* Sort out child linkage */
                if ((p->next = r->next) != NULL)
                    p->next->prev = p;
                r->next = p;
                p = r;                  /* Set up for next loop */
                resort = 1;
            }
        
    }
    while (resort != 0);
}
        
    
