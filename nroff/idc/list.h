/****************************************************************************
 *
 *  			Copyright 1996 Jon Green.
 *                         All Rights Reserved
 *
 *
 *  System        : 
 *  Module        : 
 *  Object Name   : $RCSfile: list.h,v $
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

#ifndef __LIST_H__
#define __LIST_H__

typedef struct st_ListItem {
    struct st_ListItem *next;
    struct st_ListItem *prev;
    int    sort;
    void   *data;
} ListItem;

typedef int (* ListSortFunc)(void *d1, void *d2);  /* List sort function */

extern ListItem *ListDuplicate (ListItem *head);
extern ListItem *ListItemConstruct (int size);
extern void ListDestruct (ListItem **head, ListItem *p);
extern void ListDestructAll (ListItem **head);
extern void ListItemInsert (ListItem **head, ListItem *p);
extern void ListSort (ListItem **head, ListSortFunc func);

#endif /* __LIST_H__ */
