/****************************************************************************
 *
 *			   Copyright 1995-2004 Jon Green.
 *			      All Rights Reserved
 *
 *
 *  System        :
 *  Module        :
 *  Object Name   : $RCSfile: nrar.c,v $
 *  Revision      : $Revision: 1.3 $
 *  Date          : $Date: 2004-02-07 19:29:49 $
 *  Author        : $Author: jon $
 *  Last Modified : <040207.1920>
 *
 *  Description
 *
 *  Notes
 *
 *  History
 *
 * 1.0.0f JG 2004-02-07 Ported to HP-UX
 * 1.0.0e JG 2004-01-03 Ported to Sun Solaris 9
 * 1.0.0d JG 1999-05-29 Fixed cross reference problem with .NH sections
 * 1.0.0c JG 1997-05-03 Ported to win32
 * 1.0.0b JG 1996-11-27 Added symbol mode.
 * 1.0.0a JG 1996-10-31 Added multiple library support.
 * 1.0.0  JG 1995-12-10 Orginal
 *
 ****************************************************************************
 *
 *  Copyright (c) 1995-2004 Jon Green
 *
 *  All Rights Reserved.
 *
 *  This Document may not, in whole or in part, be copied,
 *  photocopied, reproduced, translated, or reduced to any
 *  electronic medium or machine readable form without prior
 *  written consent from Jon Green.
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>

#if ((defined _HPUX) || (defined _LINUX) || (defined _SUNOS))
#include <unistd.h>
#else
#include <getopt.h>
#endif
#include <utils.h>
#include "nroff.h"

#define MODULE_VERSION "1.0.0f"
#define MODULE_NAME    "nrar"

typedef struct {
    char *name;
    char *section;
    char *description;
    char *category;
} xrEntry;

static char *sectionName = NULL;
static char *sectionId = NULL;
static char *sectionModule = NULL;
static char *sectionComponent = NULL;
static char *sourceName = NULL;
static int  nhAutoCount = 0;            /* .NH Automatic count */
static char *progname  = MODULE_NAME;   /* Name of the program */
static char *moduleName = NULL;
static FILE *fpr = NULL;
static int  symbolMode = 0;             /* Extended symbol information */
static char *xreffile;                  /* Cross reference file */

/*
 * Construct the extended cross refence entry.
 */

static xrEntry *
xrConstruct (char *name, char *section, char *description, char *category)
{
    xrEntry *xe;

    if ((xe = malloc (sizeof (xrEntry))) == NULL)
        uFatal ("Cannot allocate Cross Reference extended information buffer\n");

    xe->name = bufNStr (NULL, name);

    if (section == NULL)
        section = sectionId;
    xe->section = bufNStr (NULL, section);
    xe->description = bufNStr (NULL, description);
    if (sectionComponent == NULL)
        xe->category = bufNStr (NULL, category);
    else
    {
        xe->category = bufNStr (NULL, sectionComponent);
        if (category != NULL)
        {
            xe->category = bufNStr (xe->category, "|");
            xe->category = bufNStr (xe->category, category);
        }
    }
    return (xe);
}

/*
 * Destruct the extended cross reference entry.
 */

static void
xrDestruct (xrEntry *xe)
{
    if (xe == NULL)
        return;
    bufFree (xe->name);
    bufFree (xe->section);
    bufFree (xe->description);
    bufFree (xe->category);
    free (xe);
}


/*
 * Print the extended corss reference entry
 * Format is <Name> <Section> "<Description>" <Category>
 */
static void
xrPrint (xrEntry *xe, FILE *fp)
{
    if (xe == NULL)
        return;
    if ((xe->name == NULL) || (xe->section == NULL))
        return;
    fprintf (fp, " %s %s \"%s\" %s",
             xe->name, xe->section,
             (xe->description == NULL) ? "" : xe->description,
             (xe->category == NULL) ? "" : xe->category);
}

static void
nrIm_func (char *module, char *component)
{
    bufFree (sectionModule);
    sectionModule = bufNStr (NULL, module);
    bufFree (sectionComponent);
    sectionComponent = bufNStr (NULL, component);
}

static void
nrTH_func (char *id, char *num, char *date, char *company, char *title)
{
    bufFree (sectionId);
    sectionId = bufNStr (NULL, num);
    bufFree (sectionName);
    sectionName = bufNStr (NULL, id);
    if (xreffile != NULL)
        free (xreffile);
    xreffile = strdup (sourceName);

#if 0
    /*
     * Add the TH name to the symbol table.
     */

    if (libFindAddReference (moduleName,
                             nrMakeXref (sectionName, sectionId),
                             xreffile, NULL, &file, NULL,
                             LS_EXTERN|LS_DEF) == 0)
        uError ("Duplicate reference name [%s%s:%s]\n",
                sectionName, (sectionId == NULL) ? "" : sectionId, file);
#endif
}

static void
nrNH_func (char *id, char *num, char *title, char *xref)
{
    char bufName [9];                   /* Automatic file name */
    char digName [4];                   /* Digit component */
    char *file = NULL;

    if (xref == NULL)
    {
        if (sectionModule == NULL)
        {
            uError ("Section identified not set with .Im\n");
            return;
        }
        /*
         * Construct a new name in the form a<ModuleName:4>NNN
         * Start from 0 and increment each time.
         */

        bufName [0] = 'a';
        strncpy (&bufName [1], sectionModule, 4);
        bufName [5] = '\0';             /* Ensure nil terminated */
        sprintf (digName,"%03d", nhAutoCount++);
        strcat (bufName,  digName);
        xref = bufName;
    }
    
    /* Cross reference name */
    if (xreffile != NULL)
        free (xreffile);
    xreffile = strdup (xref);

    if (libFindAddReference (moduleName, nrMakeXref (id, num), xref,
                             NULL, &file, NULL, LS_EXTERN|LS_DEF) == 0)
        uError ("Duplicate reference name [%s:%s]\n", xref, file);
}

static void
nrStart (char *s, int flag)
{
    char *p;
    char c;

    bufFree (sourceName);
    sourceName = bufNStr (NULL, s);
    /*
     * String the extension from the name
     */
    p = sourceName;
    while ((c = *p) != '\0')
    {
        if (c == '.')
        {
            *p = '\0';
            break;
        }
        p++;
    }
}

static void
nrStartInc (char *s, int *imode)
{
    *imode = 1;
}

static void
_nrXIJ_func (int package, char *name, char *id, char *desc, char *comp)
{
    char *file = NULL;
    LibItem *li = NULL;                    /* Library item */

    if (id == NULL)
        id = sectionId;
    if (libFindAddReference (moduleName, nrMakeXref (name, id), xreffile,
                             NULL, &file, (void *)(&li), 
                             (package & LS_JUMP)|LS_EXTERN|LS_DEF) == 0)
    {
        if ((((sectionName == NULL) || (name == NULL)) &&
             (sectionName != name)) || (strcmp (sectionName, name) != 0) ||
            (((sectionId == NULL) || (id == NULL)) &&
             (id != sectionId)) || (strcmp (sectionId, id) != 0))
            uError ("Duplicate reference name [%s%s:%s]\n",
                    name, (id == NULL) ? "" : id, file);
    }
    /* Add the user data if not present */
    if (symbolMode && (li != NULL) && (nrLibItemData(li) == NULL))
    {
        nrLibItemData(li) = xrConstruct (name, id, desc, comp);
/*        uError ("Adding a LS_JUMP label\n");*/
        nrLibItemStatus(li) |= package & LS_JUMP;
    }
}

static void
nrXI_func (char *name, char *id, char *desc, char *comp)
{
    _nrXIJ_func (0, name, id, desc, comp);
}

static void
nrXJ_func (char *name, char *id, char *desc, char *comp)
{
/*    uError ("Found a LS_JUMP label\n");*/
    _nrXIJ_func (LS_JUMP, name, id, desc, comp);
}

static void
nrXP_func (char *name, char *id, char *desc, char *comp)
{
    char *file = NULL;
    LibItem *li = NULL;                    /* Library item */

    if (id == NULL)
        id = sectionId;
    if (libFindAddReference (moduleName, nrMakeXref (name, id),
                             sourceName, NULL, &file, (void *)(&li),
                             LS_PACKAGE|LS_EXTERN|LS_DEF) == 0)
    {
        if ((((sectionName == NULL) || (name == NULL)) &&
             (sectionName != name)) || (strcmp (sectionName, name) != 0) ||
            (((sectionId == NULL) || (id == NULL)) &&
             (id != sectionId)) || (strcmp (sectionId, id) != 0))
            uError ("Duplicate package reference name [%s%s:%s]\n",
                    name, (id == NULL) ? "" : id, file);
    }

    /* Add the user data if not present */
    if (symbolMode && (li != NULL) && (nrLibItemData(li) == NULL))
        nrLibItemData(li) = xrConstruct (name, id, desc, comp);
}

static void
alpha_term (void)
{
    LibModule *m;
    LibItem *ii;
    xrEntry *xe;

    for (m = nrLibModuleHead(); m != NULL; m = nrLibModuleNext(m))
    {
#if 0
        uWarn ("Module = [%s]. Alias = [%s]\n",
               nrLibModuleName(m), nrLibModuleAlias(m));
#endif
        fprintf (fpr, ".Ls %s %s" eolStr, nrLibModuleName(m),
                 (nrLibModuleAlias(m) == NULL) ? "" : nrLibModuleAlias(m));

        /* Print package references */
        for (ii = nrLibModulePackage(m); ii != NULL; ii = nrLibItemNext(ii))
        {
            fprintf (fpr, ".Lp %s %s",
                     nrLibItemName(ii), nrLibItemFile (ii));
            /* Print the extended symbol information if required */
            if (symbolMode && ((xe = nrLibItemData(ii)) != NULL))
            {
                xrPrint (xe, fpr);
                xrDestruct (xe);
                nrLibItemData(ii) = NULL;
            }
            fprintf (fpr, eolStr);
        }

        /* Print symbol references */
        for (ii = nrLibModuleItem(m); ii != NULL; ii = nrLibItemNext(ii))
        {
            fprintf (fpr, ".L%c %s %s",
                     (nrLibItemStatus(ii) & LS_JUMP) ? 'j' : 'i',
                     nrLibItemName(ii), nrLibItemFile (ii));
            /* Print the extended symbol information if required */
            if (symbolMode && ((xe = nrLibItemData(ii)) != NULL))
            {
                xrPrint (xe, fpr);
                xrDestruct (xe);
                nrLibItemData(ii) = NULL;
            }
            fprintf (fpr, eolStr);
        }
        fprintf (fpr, ".Le" eolStr);
    }
    fprintf (fpr, "%s", eofStr);
}

static void
usage (void)
{
    fprintf (stdout, "%s : Usage %s [Options] <files>\n", progname, progname);
    fprintf (stdout, "\n");
    fprintf (stdout, "-?/h      : Help - this page\n");
    fprintf (stdout, "-A <mod>  : Alias module name\n");
    fprintf (stdout, "-E <file> : Log errors to <file> (append)\n");
    fprintf (stdout, "-e <file> : Log errors to <file> (re-write)\n");
    fprintf (stdout, "-l <lib>  : Library component\n");
    fprintf (stdout, "-L <path> : Library search path\n");
    fprintf (stdout, "-M <mod>  : Module name\n");
    fprintf (stdout, "-o <file> : Output to <file> default is <module>.lbn\n");
    fprintf (stdout, "-q        : Quiet mode\n");
    fprintf (stdout, "-s        : Add extended symbol information to library (search tags)\n");
    exit (1);
}

static void
arInitialise (void)
{
    static  nrFUNCTION funcTab = {NULL};

    /* Headers */
    nrInstall (funcTab, NH_func, nrNH_func);
    nrInstall (funcTab, TH_func, nrTH_func);

    /* Identification */
    nrInstall (funcTab, XI_func, nrXI_func);
    nrInstall (funcTab, XJ_func, nrXJ_func);
    nrInstall (funcTab, XP_func, nrXP_func);
    nrInstall (funcTab, Im_func, nrIm_func);

    /* Files */
    nrInstall (funcTab, startFile_func, nrStart);
    nrInstall (funcTab, includeFile_func, nrStartInc);
    nrInstall (funcTab, endFile_func, NULL);

    nrInstallFunctionTable (&funcTab);
}

int main (int argc, char *argv [])
{
    char    *oname = NULL;
    char    *alias = NULL;
    int     ecount = 0;                 /* Error count */
    int     wcount = 0;                 /* Warn count */
    nrFILE  *nrfp;
    int     c;
    int     i;

    /* Initialise the error channel */
    uErrorSet (0, &ecount);             /* Max Num entries + counter */
    uWarnSet (&wcount);                 /* Warning count */
    uUtilitySet (progname);             /* Set name of program */

    arInitialise ();

    while (1) {
        c = getopt (argc, argv, "A:e:E:IhM:o:L:l:qs");
        if (c == EOF)
            break;
        switch (c) {
        case 'A':
            alias = optarg;
            break;
        case 'e':
            uErrorChannelSet (optarg, UERROR_LOG_CREATE);
            break;
        case 'E':
            uErrorChannelSet (optarg, UERROR_LOG_APPEND);
            break;
        case 'I':
            fprintf (stdout, "%s. Version %s. %s. %s\n",
                     progname, MODULE_VERSION, __DATE__, nroffVersion);
            exit (0);
            break;
        case 'h':
            usage ();
            break;
        case 'L':
            nrLibSetPath (optarg);
            break;
        case 'l':
            nrLibLoad (optarg);
            break;
        case 'M':
            moduleName = optarg;
            if (nrLibSetModule (moduleName) != 0)
                exit (1);
            break;
        case 'o':
            oname = optarg;
            break;
        case 'q':
            uInteractiveSet (0);        /* Disable interactive mode */
            break;
        case 's':
            symbolMode = 1;
            break;
        default:
            uError ("Cannot understand option [-%c]\n", c);
            usage();
            break;
        }
    }

    uOpenErrorChannel ();
    argv = getfiles (&argc, argv, optind);

    if (moduleName == NULL)
        uFatal ("Module name required with the -m <module> option.\n");

    /*
     * Assign the alias if required.
     */

    if (alias != NULL)
        nrLibModuleSetAlias (moduleName, alias);

    if (oname == NULL)
    {
        oname = strdup (makeFilename (NULL, NULL, moduleName, "lbn"));
        uVerbose (0, "Output in file [%s].\n", oname);

    }

    /* Process all of the files */

    for (i = 1; i < argc; i++) {
        if ((nrfp = nrFilePush (argv[i])) == NULL)
            exit (1);

        /* Set up the conversion */
        nrStart (argv[i], 0);               /* Save filename */
        nroff (nrfp, NROFF_MODE_SYMBOL);    /* Not compiling. */
    }

    if (ecount == 0)
    {
        if ((fpr = fopen (oname, "wb")) == NULL)
            uFatal ("Cannot open file [%s]\n", oname);
        alpha_term ();
        fclose (fpr);
    }

    uCloseErrorChannel ();
    return (ecount);
}
