#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <utils.h>

#include "_nroff.h"
#include "nroff.h"

static TagP __tagHead = NULL;

TagP
tagAlloc (char *name, char *section, char *desc, char *file,
          int line, void *data)
{
    TagP  np;
    int   c;

    if ((np = (TagP) malloc (sizeof (Tag))) == NULL)
        uFatal ("No memory\n");

    if (isupper (name[0]))
        c = tolower (name[0]);
    else
        c = name[0];

    np->name = bufChr (NULL, c);
    np->name = bufStr (np->name, name);
    np->line = line;
    np->file = bufStr (NULL, file);
    np->section = bufStr (NULL, section);
    np->desc = bufStr (NULL, desc);
    np->next = NULL;
    np->prev = NULL;
    np->data = data;
    return (np);
}

/*
 * Duplicate a tag
 */

TagP
tagDup (TagP tp)
{
    return (tagAlloc ((tp->name == NULL) ? NULL : &tp->name[1],
                      tp->section,
                      tp->desc,
                      tp->file,
                      tp->line,
                      tp->data));
}

void
tagFree (TagP tp)
{
    bufFree (tp->name);
    bufFree (tp->file);
    bufFree (tp->section);
    bufFree (tp->desc);
    tp->data = NULL;
    free (tp);
}

TagP
add_tag (TagP *tagHead, TagP np)
{
    TagP p;
    int  i;

    if (tagHead == NULL)
        tagHead = &__tagHead;
#if 0
    printf ("Add tag [%s][%s][%s][%d]\n",
            np->name, np->section, np->file, np->line);
#endif
    if ((p = *tagHead) == NULL) {
        *tagHead = np;
        return (NULL);
    }

    for (;;) {
        if ((i = strcmp (np->name, p->name)) == 0)
            i = strcmp (np->section, p->section);
        if (i == 0)
            return (np);   /* Duplicate label */
        if (i < 0) {
            if ((np->prev = p->prev) == NULL)
                *tagHead = np;
            else
                np->prev->next = np;
            p->prev = np;
            np->next = p;
            return (NULL);
        }
        if (p->next == NULL) {
            p->next = np;
            np->prev = p;
            return (NULL);
        }
        else
            p = p->next;
    }
}

/*
 * find_tag.
 * Try to locate the tag in the list. If found then unlink from
 * the list if told to.
 */

TagP
find_tag (TagP *tagHead, char *name, char *section, int unlink)
{
    TagP p;
    int  i;

    if (tagHead == NULL)
        tagHead = &__tagHead;
#if 0
    printf ("Find tag [%s][%s][%s]\n",
            name, section, (unlink == 0) ? "No unlink" : "Unlink");
#endif

    for (p = *tagHead; p != NULL; p = p->next)
    {
        if ((i = strcmp (name, p->name)) == 0)
            i = strcmp (section, p->section);
        if (i == 0)
        {
            /* Duplicate label -  hence found. */
            if (unlink == 0)            /* Unlink enabled ?? */
                return (p);             /* No - just return tap */

             /* Unlink the tag from the list and return to the caller. */

            if (p->prev == NULL)        /* Unlink previous */
                *tagHead = p->next;
            else
                p->prev->next = p->next;

            if (p->next != NULL)        /* Unlink next */
                p->next->prev = p->prev;
            p->prev = NULL;
            p->next = NULL;
            return (p);
        }
        else if (i < 0)                 /* Smaller than current ?? */
            break;                      /* Gone past it - so not found */
    }
    return (NULL);                      /* Not found */
}

/*
 * Remove a tag from the list.
 * Search the list for the specified tag. If it exists then remove
 * it from the list.
 */

TagP
sub_tag (TagP *tagHead, TagP np)
{
#if 0
    printf ("Sub tag [%s][%s][%s][%d]\n",
            np->name, np->section, np->file, np->line);
#endif
    return (find_tag (tagHead, np->name, np->section, 1));
}

void
free_tags (TagP *tagHead)
{
    TagP tp, fp;

    if (tagHead == NULL)
        tagHead = &__tagHead;

    tp = *tagHead;
    while (tp != NULL) {
        fp = tp;
        tp = tp->next;
        tagFree (fp);
    }
    *tagHead = NULL;
}

int
diff_tag (TagP s1, TagP s2)
{
    /* Test the name field */
    if ((s1->name == NULL) || (s2->name == NULL))
    {
        if (s1 != s2)
            return (1);
    }
    else
        if (strcmp (s1->name, s2->name) != 0)
            return (1);

    /* Test the section field */
    if ((s1->section == NULL) || (s2->section == NULL))
    {
        if (s1 != s2)
            return (1);
    }
    else
        if (strcmp (s1->section, s2->section) != 0)
            return (1);
#if 0
    /* Test the line */
    if (s1->line != s2->line)
        return (1);

    /* Test the file field */
    if ((s1->file == NULL) || (s2->file == NULL))
    {
        if (s1 != s2)
            return (1);
    }
    else
        if (strcmp (s1->file, s2->file) != 0)
            return (1);

    /* Test the desc field */
    if ((s1->desc == NULL) || (s2->desc == NULL))
    {
        if (s1 != s2)
            return (1);
    }
    else
        if (strcmp (s1->desc, s2->desc) != 0)
            return (1);
#endif
    return (0);
}

TagP
tagIterateInit (TagP *tagHead)
{
    if (tagHead == NULL)
        return (__tagHead);
    return (*tagHead);
}

TagP
tagIterate (TagP tp)
{
    if (tp != NULL)
        tp = tp->next;
    return (tp);
}

int
tagCount (TagP tagHead)
{
    int i;
    TagP tp;

    if (tagHead == NULL)
        tagHead = __tagHead;

    for (i = 0, tp = tagHead; tp != NULL; tp = tp->next)
        i++;
    return (i);
}
