/****************************************************************************
 *
 *  			Copyright 1996-2004 Jon Green.
 *                         All Rights Reserved
 *
 *
 *  System        : 
 *  Module        : 
 *  Object Name   : $RCSfile: utils.c,v $
 *  Revision      : $Revision: 1.2 $
 *  Date          : $Date: 2004-01-06 00:52:20 $
 *  Author        : $Author: jon $
 *  Last Modified : <040104.0023>
 *
 *  Description	
 *
 *  Notes
 *
 *  History
 *	
 ****************************************************************************
 *
 *  Copyright (c) 1996-2004 Jon Green.
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
#include <stdarg.h>
#include <string.h>
#include <ctype.h>              /* for tolower() */
#include "utils.h"

/*
 * Version 2.2.1b - 05/11/95 - JG
 * Added version infromation - this is donkeys old !!
 */

#define MODULE_NAME     "Text Processing Utilities"
#define MODULE_VERSION  "2.2.1b"

/*
 * Buffering S/W
 */

char *bufNStr (char *h, char *s)
{
    int slen;

    if ((s == NULL) || (*s == '\0'))
        return (h);

    slen = strlen (s) + 1;
    if (h == NULL) {
        h = malloc (slen);
        h[0] = '\0';
    }
    else
        h = realloc (h, slen + strlen(h));
    strcat (h, s);
    return (h);
}

char *bufStr (char *h, char *s)
{
    int slen;

    if (s != NULL)
        slen = strlen (s);
    else
        slen = 0;

    if (h == NULL) {
        h = malloc (slen+1);
        if (slen > 0)
            strcpy (h, s);
        else
            h[0] = '\0';
    }
    else if (slen > 0) {
        h = realloc (h, slen + strlen(h) + 1);
        strcat (h, s);
    }
    return (h);
}

char *bufChr (char *h, char c)
{
    static char lbuf[2] = {0,0};

    if (h == NULL) {
        h = malloc (2);
        h [0] = '\0';
    }
    else
        h = realloc (h, strlen (h)+2);
    lbuf[0] = c;
    strcat (h, lbuf);
    return (h);
}

char *bufFormat (char *h, char *format, ... )
{
    char buffer [1024];

    va_list ap;
    va_start(ap, format);
    vsprintf(buffer, format, ap);
    va_end(ap);
    return (bufStr (h, buffer));

}

char *bufFree (char *h)
{
    if (h != NULL)
        free (h);
    return (NULL);
}

char
**bufNArg (char **argv, int *argc, char *argp)
{
    int     i;

    if (argv == NULL) {
        argv = (char **) malloc (sizeof (char *));
        *argc = 0;
        return (argv);
    }

    i = *argc;
    argv = (char **) realloc ((char *)(argv),
                           sizeof (char *) * (i+2));
    argv [i++] = argp;
    argv [i] = NULL;
    *argc = i;
    return (argv);
}

void bufNArgFree (char **arg, int i)
{
    if (i)
        for (i = 0; arg[i] != NULL; i++)
            bufFree (arg[i]);
    free ((char *)(arg));
}

void
bufArg (Args *args, char *argp)
{
    int     i;

    if (args->argv == NULL) {
        args->argv = (char **) malloc (sizeof (char *));
        args->argc = 0;
        return;
    }

    i = args->argc;
    args->argv = (char **) realloc ((char *)(args->argv),
                                    sizeof (char *) * (i+2));
    args->argv [i++] = argp;
    args->argv [i] = NULL;
    args->argc = i;
}

void
bufArgFree (Args *args)
{
    int i;

    if (((i = args->argc) > 0) && (args->argv != NULL))
    {
        for (i = 0; args->argv[i] != NULL; i++)
            bufFree (args->argv[i]);
    }
    if (args->argv != NULL) {
        free ((char *)(args->argv));
        args->argv = NULL;
    }
    args->argc = 0;
}

#if !(defined (_DOS))
int
strcasecmp(const char *s1, const char *s2)
{
    char c1;
    char c2;

    for (c1 = tolower(*s1), c2 = tolower(*s2);
         c1 == c2;
         c1 = tolower(*(++s1)), c2 = tolower(*(++s2)))
        if (c1 == '\0')
            return 0;
    return (unsigned char) c1 < (unsigned char) c2 ? -1 : +1;
}

int
strncasecmp(const char *s1, const char *s2, size_t n)
{
    char c1;
    char c2;

    for (c1 = tolower(*s1), c2 = tolower(*s2);
         0 < n;
         c1 = tolower(*(++s1)), c2 = tolower(*(++s2)), --n)
    {
        if (c1 != c2)
            return (unsigned char) c1 < (unsigned char) c2 ? -1 : +1;
        else if (c1 == '\0')
            return 0;
    }
    return 0;
}
#endif

int strnullcmp (const char *s1, const char *s2)
{
    if ((s1 == NULL) || (s2 == NULL))
        return ((s1 == s2) ? 0 : 1);
    return (strcmp (s1, s2));
}

/*
 * Skip over white space
 */

char
*cleanWhiteSpace (char *s)
{
    char c;
    int i,j;

    /* Trim leading white space */
    while ((c = *s) && ((c == '\t') || (c == ' ')))
        s++;
    if (*s == '\0')
        return (s);
    for (i = 0, j = 1; s [j] != '\0'; j++)
    {
        if (((c = s[j]) == '\t') || (c == ' '))
        {
            c = ' ';
            if (s[i] == ' ')
                continue;
        }
        if (++i != j)
            s[i] = c;
    }
    if (!((i > 0) && (s[i] == ' ')))
        i++;
    s[i] = '\0';
    return (s);
}

char *
skipWhiteSpace (char *s)
{
    char c;

    while ((c = *s) && ((c == '\t') || (c == ' ')))
        s++;
    return (s);
}

char *
trimWhiteSpace (char *s)
{
    char c;
    char *r;

    for (r = s; *r != '\0'; r++)
        ;

    if (r != s)
    {
        do
        {
            if ((c = *--r) && ((c == '\t') || (c == ' ')))
                *r = '\0';
            else
                break;
        } while (r != s);
    }
    return (s);
}

char *
compWhiteSpace (char *s)
{
    char c;
    int  i, j;

    if (*s == '\0')
        return (s);
    if (s[0] == '\t')
        s[0] = ' ';
    for (i = 0, j = 1; s [j] != '\0'; j++)
    {
        if (((c = s[j]) == '\t') || (c == ' '))
        {
            c = ' ';
            if (s[i] == ' ')
                continue;
        }
        if (++i != j)
            s[i] = c;
    }
    s[i+1] = '\0';
    return (s);
}


/*
 * Im field RE handling
 */

char *
varStrGetFirst (char *comp)
{
    char *p;

    if (comp == NULL)
        return (NULL);
    if (*comp == '\0')
        return (NULL);

    for (p = comp; *p != 0; p++) {
        if ((*p == '|') || (*p == ';')) {
            *p = '\0';
            break;
        }
    }
    return (comp);
}

char **
varStrGetAll (char *comp)
{
    int i;
    int j;
    char *p;
    char **argv;
    char *t;

    /* Check string is there !! */
    if (comp == NULL)
        return (NULL);
    if (*comp == '\0')
        return (NULL);

    t = bufNStr (NULL, comp);
    /* Remove the string separator */
    for (i = 1, j = 0, p = t; *p != 0; p++, j++) {
        if (*p == '|') {
            *p = '\0';
            i++;
        }
    }

    /*
     * Allocate a char * list with enough space
     * for the string in the bottom of it.
     */

    argv = (char **) malloc ((sizeof (char *) * (i+2)) + j + 1);
    argv [i] = NULL;                    /* Terminate argv list */
    p = (char *)(&argv [i+1]);          /* Get start of string area */
    memcpy (p, t, j+1);                 /* Copy in string list */

    /* Set up the argv list */
    for (j = 0; j < i; j++) {
        argv [j] = p;                   /* Point to start of string */
        while (*p++ != '\0')            /* Move to next string start */
            /* NULL */;
    }
    bufFree (t);
    return (argv);
}

int
varStrCmp (char *item, char *ritem)
{
    int len;


    if (ritem == NULL)
        ritem = "misc";
    len = strlen (item);
    if ((strncasecmp (item, ritem, len) == 0) &&
        ((ritem [len] == '\0') || (ritem [len] == ';')))
        return (0);
    return (1);
}

int
varStrCmpAll (char *item, char **ritem)
{
    if (ritem == NULL)
        return (varStrCmp (item, NULL));

    do {
        if (varStrCmp (item, *ritem) == 0)
            return (0);
    } while (*++ritem != NULL);
    return (1);
}

int
varStrCmpAllC (char *item, char *ritem)
{
    char **argv;
    int  status = 1;

    if ((argv = varStrGetAll (ritem)) != NULL)
    {
        status = varStrCmpAll (item, argv);
        free (argv);
    }
    return (status);
}

char *uVersion (void)
{
    return (MODULE_NAME " V" MODULE_VERSION " " __DATE__ );
}
