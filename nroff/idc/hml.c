/****************************************************************************
 *
 *  			Copyright 1996 Jon Green.
 *                         All Rights Reserved
 *
 *
 *  System        : 
 *  Module        : 
 *  Object Name   : $RCSfile: hml.c,v $
 *  Revision      : $Revision: 1.1 $
 *  Date          : $Date: 2000-10-21 14:31:22 $
 *  Author        : $Author: jon $
 *  Last Modified : <990831.2330>
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
#include <stdarg.h>
#include <ctype.h>

#include "hmldrv.h"
#include <utils.h>

static const char rcsid[] = "@(#) : $Id: hml.c,v 1.1 2000-10-21 14:31:22 jon Exp $";

/*
 * Version 1.0.0 - 03/11/95 - JG
 * Original Version.
 *
 * Version 1.0.1 - 04/11/95 - JG.
 * Closed scope of functions - return table in Open.
 *
 * Version 1.0.1a - 07/11/95 - JG
 * Tidied header comment in html output.
 *
 * Version 1.0.1b - 16/11/95 JG
 * Strip extension in hmlOpen.
 *
 * Version 1.0.1c - 07/12/95 JG
 * Corrected the <FAQ> menus - were not terminating properly.
 * 
 * Version 1.0.1d - 05/01/96 - JG
 * Added Home page reference for link into manual pages. 
 * 
 * Version 1.1.0 - 03/09/96
 * Added In-line text formatting.
 */

#define MODULE_NAME     "Hypertext Engine"
#define MODULE_VERSION  "1.0.1c"

extern hmldct *htmlOpen (char *logo);   /* HTML external definition */
extern hmldct *rtfOpen (char *logo);    /* RTF external definition */

Args hmlQuickStrs = {NULL, 0};          /* Quick arguments */
char *hmlOpFileName = NULL;             /* Output file name */
char *hmlIdent = NULL;                  /* Identity comment string for file */
char *hmlGenid = NULL;                  /* Generator inforation for file */
char *hmlHomePage = NULL;               /* Home page - external link */
char *hmlLogo = NULL;                   /* Logo - current file */
hmldct *hmldctp;
FILE *fpHml = NULL;                     /* HML File stream */
static hmlFormatType = 0;               /* Current hml format */

int
hmlFormat (void)
{
    return hmlFormatType;
}

static int
hyperChr (int c)
{
    if ((c >= 'A') && (c <= 'Z'))
        return (tolower(c));
    if (((c >= '0') && (c <= '9')) ||
        ((c >= 'a') && (c <= 'z')) ||
        (c == '.') || (c == '_'))
        return (c);
    return ('\0');
}

char *
hmlMakeTopicHyperName (char *head, char *name, char *id)
{
    char    *tname;
    char    c;

    tname = bufStr (NULL, head);
    if (name != NULL) {
        while ((c = *name++) != '\0')
            if ((c = hyperChr(c)) != 0)
                tname = bufChr (tname, c);
    }

    if (id != NULL) {
        while ((c = *id++) != '\0')
            if ((c = hyperChr(c)) != 0)
                tname = bufChr (tname, c);
    }
    return (tname);
}

void
hmlMakeFileName (char *s, char **result)
{
    int     j;
    char    *cb;

    cb = bufNStr (NULL, s);
    for (j = 0; ((cb[j] != '\0') && (cb[j] != '.')); j++)
        if (isupper (cb[j]))
            cb[j] = tolower (cb[j]);
    cb[j] = '\0';
    cb = bufStr (cb, ".hml");

    bufFree (*result);
    *result = cb;
}

int
hmlCodeTidy (char **cp)
{
    char **p;
    char *s;
    int  min;
    int  i;


    for (min = 65565, p = &cp [1], s = *p;
         (unsigned char)(*s) != CHAR_FORMAT_ECODE;
         s = *++p)
    {
        uDebug (1, ("Loop [%s]\n", s));
        for (i = 0; *s == ' '; s++)
            i++;
        if (*s == '\0')
            continue;
        if (i < min)
            min = i;
        if (min == 0)
            break;
    }

    if (min == 65565)
        min = 0;

    if (min > 0)
    {
        for (p = &cp [1], s = *p;
             (unsigned char)(*s) != CHAR_FORMAT_ECODE;
             s = *++p)
        {
            uDebug (1, ("Loop1 [%s]\n", s));
            for (i = 0; s[i] != '\0'; /* NULL*/)
                if (++i == min)
                    break;
            if (i > 0) {
                s = bufStr (NULL, &s[i]);
                bufFree (*p);
                *p = s;
            }
        }
    }
    return (0);
}

void
hmlOpen (int format, char *s, char *logo)
{
    char *fileExtensions [] = {"htp", "rtf"};
    char *t;
    
    /*
     * Knock off the extension if there is one.
     */
    hmlFormatType = format;
    hmlOpFileName = bufStr (NULL, s);
    for (t = hmlOpFileName; *t != '\0'; t++)
        /* Nothing */;
    do
    {
        char c;

        if ((c = *--t) == '.') {
            *t = '\0';
            break;
        }
        else if ((c == '\\') || (c == '/') || (c == ':'))
            break;
    } while (t != hmlOpFileName);

    hmlOpFileName = bufFormat (hmlOpFileName, ".%s", fileExtensions [format]);
    fpHml = fopen (hmlOpFileName, "w");
    if (fpHml == NULL)
        uFatal ("Cannot open output file [%s]\n", hmlOpFileName);
    else
    {
        /* Change the read buffer size. Make it 64K bytes. */
        setvbuf (fpHml, NULL, _IOFBF, 64*1024);
    }
    
    if (format == HML_FORMAT_HTML)
        hmldctp = htmlOpen (logo);
    else if (format == HML_FORMAT_RTF)
        hmldctp = rtfOpen (logo);
    else
        uFatal ("Unrecognised format specified\n");
    if (hmldctp == NULL)
        uFatal ("Failed to initialise the format driver.\n");

}

void
hmlClose (void)
{
    hmlOpFileName = bufFree (hmlOpFileName);
    hmlLangClose ();
    fclose (fpHml);
}

void
hmlAddQuick (char *s)
{
    char *p;

    if (hmlQuickStrs.argv == NULL)
        bufArg (&hmlQuickStrs, NULL);

    uDebug (1, ("Adding quick sting [%s]\n", s));
    if ((p = bufNStr (NULL, s)) != NULL)
        bufArg (&hmlQuickStrs, p);
}

void
hmlAddHome (char *s)
{
    /*
     * Add the home page external reference to the system.
     */
   
    hmlHomePage = bufFree (hmlHomePage);
    hmlHomePage = bufNStr (hmlHomePage, s);
}

char *
hmlXrefPageName (char *format, ...)
{
    static char buffer [128];

    va_list ap;
    va_start(ap, format);
    vsprintf(buffer, format, ap);
    va_end(ap);
    return (buffer);
}

void hmlGenId (char *s)
{
    hmlGenid = s;
}

void hmlIdentity (char *s)
{
    hmlIdent = s;
}

char *hmlVersion (void)
{
    return (MODULE_NAME " V" MODULE_VERSION " " __DATE__ );
}
