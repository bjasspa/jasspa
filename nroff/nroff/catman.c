/****************************************************************************
 *
 *  			Copyright 1997 Jon Green.
 *			    All Rights Reserved
 *
 *
 *  System        : 
 *  Module        : 
 *  Object Name   : $RCSfile: catman.c,v $
 *  Revision      : $Revision: 1.2 $
 *  Date          : $Date: 2000-10-21 15:02:02 $
 *  Author        : $Author: jon $
 *  Last Modified : <001021.1403>
 *
 *  Description	
 *
 *  Notes
 *
 *  History
 *	
 *  $Log: not supported by cvs2svn $
 *  Revision 1.1  2000/10/21 14:31:25  jon
 *  Import
 *
 *
 ****************************************************************************
 *
 *  Copyright (c) 1997 Jon Green.
 * 
 *  All Rights Reserved.
 * 
 *  This Document may not, in whole or in part, be copied,
 *  photocopied, reproduced, translated, or reduced to any
 *  electronic medium or machine readable form without prior
 *  written consent from Jon Green.
 ****************************************************************************/

static const char rcsid[] = "@(#) : $Id: catman.c,v 1.2 2000-10-21 15:02:02 jon Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <assert.h>

#include <utils.h>
#include "_nroff.h"
#include "nroff.h"

#ifndef NDEBUG
#define dbprintf(x) fflush (stdout);printf x
#else
#define dbprintf(x) /*fflush (stdout);printf x*/
#endif

extern  char    *progname;

static CManList *cmanHead;              /* Head of the list */
static int      cmanCount;              /* Count of entries */
static char     *ccatname;              /* Catman manpage file */
static char     *ctmpname;              /* Catman temp manpage file */
static FILE     *fpcat;                 /* Catman compress file */

static CFile    *cfileHead;
static int      cfileCount;              /* Count of files */
static long     cfileOffset;             /* Catman offset */

static CManList *
insertionSort (CManList *head, CManList *item)
{
    CManList *cml;
    CManList *lcml = NULL;
    int status;

    for (cml = head; cml != NULL; cml = cml->next) {
        if ((status = strcmp (item->cman.name, cml->cman.name)) < 0) {
            if (lcml == NULL) {
                item->next = cml;
                return (item);
            }
            else {
                item->next = lcml->next;
                lcml->next = item;
                return (head);
            }
        } else if (status == 0) {
            free (item);
            return (head);
        }
        lcml = cml;
    }
    item->next = NULL;
    if (lcml == NULL)
        return (item);
    lcml->next = item;
    return (head);
}

/*
 * Add a new file name to the list of files
 */

static CFile *
FindCFile (char *name)
{
    CFile   *cfp;

    /* Search for the filename first */
    for (cfp = cfileHead; cfp != NULL; cfp = cfp->next)
        if (strcmp (name, cfp->file) == 0)
            return (cfp);
    return (NULL);
}

static CFile *
AddCFile (char *name, long size, long offset)
{
    CFile   *cfp;

    /* Search for the filename first */
    if ((name != NULL) && ((cfp = FindCFile (name)) != NULL))
        return (cfp);

    /* Not found. Create a new block and return pointer */
    cfp = (CFile *) malloc (sizeof (CFile));
    if (name == NULL)
        cfp->file [0] = '\0';
    else {
        char *s;

        strcpy (cfp->file, name);
        for (s = cfp->file; *s != '\0'; s++)
            if (isupper (*s))
                *s = tolower (*s);
    }
    cfp->size = size;
    cfp->offset = offset;

    /* Add to head of the list */
    cfp->next = cfileHead;
    cfileHead = cfp;
    cfileCount += 1;
    return (cfp);
}

static void
DestructCFile (void)
{
    CFile   *cfp;

    while ((cfp = cfileHead) != NULL) {
        cfileHead = cfp->next;
        free (cfp);
    }
    cfileCount = 0;
}

#define CMAN_ADD    1
#define CMAN_LOWER  2
#define CMAN_SORT   4

static CManList *
AddCManList (char *name, CFile *cfp, int flags)
{
    CManList    *cml;
    char        *s;

    assert (cfileHead != NULL);
    dbprintf (("AddCManList: Adding cfile list [%s] from file [%s]\n",
              name, cfileHead->file));
    /* Allocate the block and add in the name */
    cml = (CManList *) malloc (sizeof (CManList));
    cmanCount += 1;
    strcpy (cml->cman.name, name);
    cml->fdesc = cfp;

    /* Convert to lower case if requested */
    if (flags & CMAN_LOWER) {
        for (s = cml->cman.name; *s != '\0'; s++)
            if (isupper (*s))
                *s = tolower (*s);
    }

    /* Add or insert into the list if requested */
    if (flags & CMAN_ADD) {
        cml->next = cmanHead;
        cmanHead = cml;
    }
    else if (flags & CMAN_SORT)
        cmanHead = insertionSort (cmanHead, cml);
    return (cml);
}

static void
DestructCManList (void)
{
    CManList    *cml;

    while ((cml = cmanHead) != NULL) {
        cmanHead = cml->next;
        free (cml);
    }
    cmanCount = 0;
}

static long
copyFile (FILE *fpfrom, FILE *fpto, long length)
{
    char    buffer [1024];
    int     readNo;
    long    readLength;

    assert (fpfrom != NULL);
    assert (fpto != NULL);
    dbprintf (("copyFile\n"));
    if (length == -1) {
        length = 0;
        do {
            readNo = fread (buffer, 1, 1024, fpfrom);
            if (readNo > 0)
                fwrite (buffer, 1, readNo, fpto);
            length += readNo;
        } while (readNo == 1024);
    }
    else
    {
        for (readLength = 0; readLength < length; readLength+= 1024L) {
            if ((length - readLength) > 1024L)
                readNo = 1024;
            else
                readNo = (int)(length - readLength);
            if (fread (buffer, 1, readNo, fpfrom) != readNo)
                exit (1);
            fwrite (buffer, 1, readNo, fpto);
        }
    }
    dbprintf (("CopyFile Done = %ld\n", length));
    return (length);
}

void putPad (FILE *fp, char *s, long n, int len)
{
    int     i;
    int     c;
    char    buffer [20];

    assert (fp != NULL);
    if (s == NULL) {
        sprintf (buffer, "%ld", n);
        s = buffer;
    }

    for (i = 0; i < len; i++) {
        if ((c = *s) != 0) {
            s++;
            if (isupper (c))
                c = tolower (c);
        }
        else
            c = ' ';
        putc (c, fp);
    }
}

void catmanInitialise (char *name, int manFlag)
{
    dbprintf (("catmanInitialise\n"));
    ctmpname = "catman.~~~";
    dbprintf (("Openning file [%s]\n", ctmpname));
    if ((fpcat = fopen (ctmpname, "wb")) == NULL)
        uFatal ("Cannot open file [%s]\n", ctmpname);

    ccatname = bufStr (NULL, name);

    cmanHead = NULL;
    cmanCount = 0;

    cfileHead = NULL;
    cfileCount = 0;
    cfileOffset = 0;
}

void catmanTerminate (void)
{
    FILE        *fp;
    CManList    *cml;
    CFile       *cfl;
    long        offset;

    dbprintf (("catmanTerminate: Closing files.\n"));
    assert (fpcat != NULL);

    fclose (fpcat);

    if (cfileHead == NULL)
        return;

    dbprintf (("catmanTerminate: Adding index entry\n"));
    cfl = AddCFile (NULL, cmanCount * sizeof (CMan), cfileOffset);
    sprintf (cfl->file, "d#%d#.0", cfileCount);
    offset = (cfileCount * (14+9+9)) + 28 - 1;

    dbprintf (("catmanTerminate: Fixing file offsets\n"));
    for (cfl = cfileHead; cfl != NULL; cfl = cfl->next)
        cfl->offset += offset;

    /* Process all of the entries and fill in the offset */
    dbprintf (("catmanTerminate: Fixing man offsets\n"));
    for (cml = cmanHead; cml != NULL; cml = cml->next) {
        dbprintf (("catmanTermnate: name = [%s], File = [%s]\n",
                   cml->cman.name, cml->fdesc->file));
        cml->cman.offset = cml->fdesc->offset;
        cml->cman.size = cml->fdesc->size;
        strcpy (cml->cman.file, cml->fdesc->file);
    }

    dbprintf (("catmanTerminate: Openning files.\n"));
    dbprintf (("Openning file [%s]\n", ccatname));
    if ((fp = fopen (ccatname, "wb")) == NULL)
        uFatal ("Cannot open file [%s]\n", ccatname);

    dbprintf (("Openning file [%s]\n", ctmpname));
    if ((fpcat = fopen (ctmpname, "rb")) == NULL) {
        uFatal ("Cannot open file [%s]\n", ctmpname);
    }

    /* Add the header of the database */
    dbprintf (("catmanTerminate: Writing database header.\n"));
    uVerbose (1, "!<man database compressed>\n");

    /* Add each item to the list in the database. */
    dbprintf (("catmanTerminate: Writing database index.\n"));
    for (cfl = cfileHead; cfl != NULL; cfl = cfl->next)
    {
        putPad (fp, cfl->file, 0L, 14);
        putPad (fp, NULL, cfl->offset, 9);
        putPad (fp, NULL, cfl->size, 9);
    }
    dbprintf (("catmanTerminate: Destructing cfile list.\n"));
    DestructCFile ();                   /* Destruct list - finished with it */

    /* Copy the rest of the file into the database */
    dbprintf (("catmanTerminate: Copying file body.\n"));
    copyFile (fpcat, fp, -1);

    /* Add the index onto the end. */
    dbprintf (("catmanTerminate: Writing long name index.\n"));
    for (cml = cmanHead; cml != NULL; cml = cml->next) {
        fwrite (&cml->cman, 1, sizeof (CMan), fp);
        dbprintf (("Sorted item [%s]\n", cml->cman.name));
    }
    dbprintf (("catmanTerminate: Destructing cman list.\n"));
    DestructCManList ();
    dbprintf (("catmanTerminate: Closing files.\n"));
    fclose (fp);
    fclose (fpcat);
/*    unlink (ctmpname);*/
}

FILE *catmanOpenFile (char *name)
{
    FILE    *fp;

    dbprintf (("catmanOpenFile: %s\n", name));
    dbprintf (("Openning file [%s]\n", "tmpdroff.cm"));
    assert (name != NULL);
    if ((fp = fopen ("tmpdroff.cm", "wb")) == NULL)
        uFatal ("Cannot open file [%s]\n", "tmpdroff.cm");

    /* Add name to list */
    AddCFile (name, 0L, 0L);
    return (fp);
}

void catmanCloseFile (FILE *fp)
{
    long        length;

    /* Close the output File */
    assert (fp != NULL);
    dbprintf (("catmanCloseFile\n"));
    fclose (fp);

    /* Compress the ouput file and concatinate onto the end of cat file */
    dbprintf (("catmanCloseFile : compress -f tmpdroff.cm [%s]\n",
               cfileHead->file));
    system ("compress -cf tmpdroff.cm > tmpdroff.cmz");
    fprintf (stdout, "compress -f tmpdroff.cm [%s]\n",cfileHead->file);
    dbprintf (("Openning file [%s]\n", "tmpdroff.cmz"));
    if ((fp = fopen ("tmpdroff.cmz", "rb")) == NULL) {
        uFatal ("Cannot open file %s\n", "tmpdroff.cmz");
    }

    length = copyFile (fp, fpcat, -1);
    assert (fp != NULL);
    fclose (fp);
/*    unlink ("tmpdroff.cm");*/
/*    unlink ("tmpdroff.cmz");*/

    /* Record the file size and offset. Change counters. */
    dbprintf (("Done copy\n"));
    cfileHead->size = length;
    cfileHead->offset = cfileOffset;
    cfileOffset += length;
    dbprintf (("Done catmanClose\n"));
    fflush(stdout);
}

void catmanEntry (char *entryName, char *section)
{
    CManList    *cml;
    char        *name;

    dbprintf (("catmanEntry (%s, %s)\n", entryName, section));

    if (entryName == NULL)
        return;

    name = bufNStr (NULL, entryName);
    if (section != NULL) {
        name = bufNStr (name, ".");
        name = bufNStr (name, section);
    }
    assert (cfileHead != NULL);
    cml = AddCManList (name, cfileHead, CMAN_SORT|CMAN_LOWER);
    dbprintf (("catmanEntry DONE: (%s, %s)\n", entryName, section));
    bufFree (name);
}

static FILE *
catmanReadIndex (char *filename, long *idxLength,
                 long *noItems, long *indexOffset)
{
    FILE *fp;
    long offset;
    long length;
    long items;

    if ((fp = fopen (filename, "rb")) == NULL) {
        uFatal ("Cannot open file [%s]\n", filename);
    }
    dbprintf (("Opened manual file %s\n", filename));

    /* Read in the index */
    fseek (fp, 27L, SEEK_SET);
    fscanf (fp, "d#%ld#.0%ld%ld", &items, &offset, &length);

    dbprintf (("Index Len = %ld\n", items));
    dbprintf (("Offset    = %ld\n", offset));
    dbprintf (("length    = %ld\n", length));

    if (idxLength != NULL)
        *idxLength = items;
    if (noItems != NULL)
        *noItems = length / sizeof (CMan);
    if (indexOffset != NULL)
        *indexOffset = offset;
    return (fp);
}

char *findCatman (char *file, char *name, char *section)
{
    FILE *fp;
    long noItems;
    long offset;
    long index;
    int  namLen;
    char *buffer;
    char *rtnbuf;
    CMan cbuf;
    long start;
    long end;
    long pos;
    int  status;
    int  i;

    /* Get the search name into a buffer. */

    if ((namLen = strlen (name)) == 0)
        return (NULL);
    if (section) {
        namLen += strlen (section);
    }
    namLen += 1;
    buffer = malloc (namLen+1);
    strcpy (buffer, name);
    strcat (buffer, ".");
    if (section)
        strcat (buffer, section);

    for (i = 0; buffer[i] != '\0'; i++)
        if (isupper (buffer[i]))
            buffer [i] = tolower (buffer [i]);


    /* Try openning the file */
    if ((fp = catmanReadIndex (file, &index, &noItems, &offset)) == NULL) {
        free (buffer);
        return (NULL);
    }

    /* Do a binary chop on the file to hone in on the name. The index is
       in a name separated list which is sorted */

    start = 0;
    end = noItems-1;
    for (;;) {
        pos = (start+end)/2L;
        dbprintf (("Searching (Start, End, Pos) = (%ld, %ld, %ld)\n",
                   start, end, pos));
        fseek (fp, (pos * sizeof (CMan)) + offset, SEEK_SET);
        fread (&cbuf, sizeof (CMan), 1, fp);
        dbprintf (("@ %ld: Comparing [%s] with db data [%s]\n", pos,
                  buffer, cbuf.name));
        if ((status = strncmp (buffer, cbuf.name, namLen)) == 0) {
            /* Back up until no match */
            do {
                pos--;
                fseek (fp, (pos * sizeof (CMan)) + offset, SEEK_SET);
                fread (&cbuf, sizeof (CMan), 1, fp);
                dbprintf (("@ %ld: Backing up [%s] with db data [%s]\n",
                           pos, buffer, cbuf.name));
            } while ((pos > 0) && (strncmp (buffer, cbuf.name, namLen) == 0));

            /* Go forward again and find all that match */
            pos++;
            rtnbuf = NULL;
            fseek (fp, (pos * sizeof (CMan)) + offset, 0);
            fread (&cbuf, sizeof (CMan), 1, fp);

            do {
                dbprintf (("@ %ld: Extracting [%s] with db data [%s]\n",
                          pos, buffer, cbuf.name));
                rtnbuf = bufNStr (rtnbuf, " ");
                rtnbuf = bufNStr (rtnbuf, cbuf.file);
                for (i = strlen (rtnbuf); i > 0; i--) {
                    if (rtnbuf [i] == '.') {
                        rtnbuf [i] = '\0';
                        break;
                    }
                }

                pos++;
#if 0
                if (fseek (fp, (pos * sizeof (CMan)) + offset, 0) != 0)
                    break;
                fread (&cbuf, sizeof (CMan), 1, fp);
#endif
                fread (&cbuf, sizeof (CMan), 1, fp);
            } while ((pos < noItems) &&
                     (strncmp (buffer, cbuf.name, namLen) == 0));
            fclose (fp);
            free (buffer);
            dbprintf (("@ %ld: Returning [%s] \n", pos, rtnbuf));
            fflush (stdout);
            return (rtnbuf);
        }
        else if (status < 0)
            end = pos;
        else
            start = pos+1;
        if ((start > end) || ((pos == start) && (pos == end))) {
            fclose (fp);
            free (buffer);
            return (NULL);
        }
    }
}

int dbzMakeInit (char *name)
{
    if (name == NULL)
        name = "man";

    ctmpname = bufStr (NULL, name);
    ctmpname = bufNStr (ctmpname, ".db~");
    if ((fpcat = fopen (ctmpname, "wb")) == NULL) {
        uFatal ("Cannot open file [%s]\n", ctmpname);
    }
    ccatname = bufStr (NULL, name);
    ccatname = bufNStr (ccatname, ".dbz");
    cmanCount = 0;
    cmanHead = NULL;
    cfileHead = NULL;
    cfileCount = 0;
    cfileOffset = 0;
    return (1);
}

int dbzConcatinate (char *filename)
{
    FILE *fp;
    long noItems;
    long offset;
    long index;
    long compactOffset;
    char indexName [14];
    long indexOffset;
    long indexSize;
    CMan cmanBuffer;
    CFile *cfp;
    int  i;


    /* Open file and read in index */
    if ((fp = catmanReadIndex (filename, &index, &noItems, &offset)) != NULL)
    {
        fseek (fp, 27L + 32L, SEEK_SET);
        compactOffset = (index * (14+9+9)) + 28;
        /* Read the index and generate file list */
        for (i = 1; i < index; i++) {
            fscanf (fp, "%s%ld%ld", indexName, &indexOffset, &indexSize);
            dbprintf (("dbzConcat: %s: @%ld=%ld\n", indexName,
                       indexOffset, indexSize));
            AddCFile (indexName, indexSize,
                      (indexOffset - compactOffset) + cfileOffset + 1);
        }

        /* Copy the file to the output file. */
        dbprintf (("dbzConcatinate: Seeking to %ld, length %ld\n",
                   compactOffset, offset - compactOffset));
        fseek (fp, compactOffset-1, SEEK_SET);
        cfileOffset += copyFile (fp, fpcat, offset - compactOffset - 1);

        /* Suck in the entry pages. */
        fseek (fp, offset, SEEK_SET);
        for (i = 0; i < noItems; i++) {
            fread (&cmanBuffer, sizeof (CMan), 1, fp);
            cfp = FindCFile (cmanBuffer.file);
            AddCManList (cmanBuffer.name, cfp, CMAN_SORT);
        }
        fclose (fp);
    }
    return (1);
}
