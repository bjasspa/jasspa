/****************************************************************************
 *
 *  			Copyright 1996 Jon Green.
 *                         All Rights Reserved
 *
 *
 *  System        : 
 *  Module        : 
 *  Object Name   : $RCSfile: name.c,v $
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

Name *names = NULL;
Name *patchNames = NULL;
Name *releaseNames = NULL;
Name *swmodules = NULL;

static char *undefinedNames [] =
{
    "UNDEFINED RELEASE",
    NULL
};              

Name undefinedReleaseName =
{
    NULL,
    undefinedNames,
    0
};

Name *nameAdd (Name **n, char *s, unsigned long mode)
{
    Name *p;
    Name *q;
    
    if ((s == NULL) | (*s == '\0'))
        return (NULL);
    
    p = (Name *) malloc (sizeof (Name));
    p->mode = mode;
    p->name = varStrGetAll (s);
    
    if (((q = *n) == NULL) || (strcasecmp (p->name[0], q->name[0]) < 0))
    {
        *n = p;
        p->next = q;
    }
    else
    {
        while ((q->next != NULL) &&
               ((strcasecmp (p->name[0], q->next->name[0]) > 0)))
            q = q->next;
        p->next = q->next;
        q->next = p;
    }
    return (p);
}

Name *nameFindNode (Name *n, char *s)
{
    Name *p;
    
    if ((s == NULL) | (*s == '\0'))
        return (NULL);
    
    for (p = n; p != NULL; p = p->next)
        if (varStrCmpAll (s, p->name) == 0)
            return (p);
    return (NULL);
}

char *nameFind (Name *n, char *s, unsigned long mode)
{
    Name *p;
    
    if ((p = nameFindNode (n, s)) == NULL)
        return (NULL);
    
    p->mode |= mode;
    return (p->name [0]);
}

char *nameFindMode (Name *n, char *s, unsigned long *mode)
{
    Name *p;
    
    if ((p = nameFindNode (n, s)) == NULL)
        return (NULL);
    
    if (mode != NULL)
        *mode = p->mode;
    return (p->name [0]);
}

char *nameFindAdd (Name **n, char *s, unsigned long mode)
{
    char *p;
    
    if ((s == NULL) || (*s == '\0'))
        return (NULL);
    if ((p = nameFind (*n, s, mode)) != NULL)
        return (p);
    uWarn ("Cannot find name [%s]. Adding to dictionary\n", s);
    return (nameAdd (n, s, mode)->name[0]);
}

Name *nameAddMode (Name **n, char *s, unsigned long mode)
{
    Name *p;
    Name *q;
    
    if ((s == NULL) | (*s == '\0'))
        return (NULL);
    
    p = (Name *) malloc (sizeof (Name));
    p->mode = mode;
    p->name = varStrGetAll (s);
    
    if (((q = *n) == NULL) || (p->mode < q->mode))
    {
        *n = p;
        p->next = q;
    }
    else
    {
        while ((q->next != NULL) && (p->mode > q->next->mode))
            q = q->next;
        p->next = q->next;
        q->next = p;
    }
    return (p);
}

    
