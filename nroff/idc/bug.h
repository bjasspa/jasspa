/****************************************************************************
 *
 *  			Copyright 1996 Jon Green.
 *                         All Rights Reserved
 *
 *
 *  System        : 
 *  Module        : 
 *  Object Name   : $RCSfile: bug.h,v $
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

#ifndef __IDC_H__
#define __IDC_H__

#include <utils.h>
#include "hml.h"
#include "hash.h"
#include "list.h"

#define READLINELEN 1024                /* Length of input line */
#define MAXERRORS   10                  /* Limit of number of errors */

/* 
 * Date:
 * A bit field of values as follows:-
 *  31         25 24   21 20     16 15     11 10  8 7       3 2   0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |Y Y Y Y Y Y Y|M M M M|D D D D D|H H H H H|M M M|H H H H H|M M M|
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  Year          Month   Day       Hour From Min F Hour To   Min T
 */

#define DATEYEARMASK      0x0000007fL
#define DATEMONTHMASK     0x0000000fL
#define DATEDAYMASK       0x0000001fL
#define DATEHOURMASK      0x0000001fL
#define DATEMINMASK       0x00000007L

#define DATEYEARSHIFT     25
#define DATEMONTHSHIFT    21
#define DATEDAYSHIFT      16
#define DATEFROMHOURSHIFT 11
#define DATEFROMMINSHIFT  8
#define DATETOHOURSHIFT   3
#define DATETOMINSHIFT    0

#define dateSetYear(d,y)   (((d) & ~(DATEYEARMASK << DATEYEARSHIFT))| \
                            ((((y)-70) & DATEYEARMASK) << DATEYEARSHIFT))
#define dateGetYear(d)     ((((d) >> DATEYEARSHIFT) & DATEYEARMASK)+70)
#define dateSetMonth(d,m)  (((d) & ~(DATEMONTHMASK << DATEMONTHSHIFT))| \
                            (((m) & DATEMONTHMASK) << DATEMONTHSHIFT))
#define dateGetMonth(d)    (((d) >> DATEMONTHSHIFT) & DATEMONTHMASK)
#define dateSetDay(d,a)    (((d) & ~0x0000001fL)|((a) & 0x1f))
#define dateGetDay(d)      ((d) & 0x000000ffL)

#define SEVERITY_A           1
#define SEVERITY_B           2
#define SEVERITY_C           3
#define SEVERITY_D           4
#define SEVERITY_E           5
#define SEVERITY_MAX         5

#define STATUS_OPEN          1
#define STATUS_SUSPENDED     2
#define STATUS_CLOSED        3
#define STATUS_REJECTED      4

#define MENU_AUTO_HEADER     0
#define MENU_FILE_HEADER     1
#define MENU_DESCRIPTION     2
#define MENU_ITEM            3
#define MENU_FOOTER          4
#define MENU_ITEM_XREF       5
#define MENU_ITEM_SCREEN     6

#define BUGNO(x)     (((Bug *)(x))->list.sort)
#define BUGNEXT(x)   ((Bug *)(((Bug *)(x))->list.next))
#define BUGPREV(x)   ((Bug *)(((Bug *)(x))->list.prev))

#define TEKNO(x)     (((Tek *)(x))->list.sort)
#define TEKNEXT(x)   ((Tek *)(((Tek *)(x))->list.next))
#define TEKPREV(x)   ((Tek *)(((Tek *)(x))->list.prev))

#define RELNO(x)     (((Release *)(x))->list.sort)
#define RELNEXT(x)   ((Release *)(((Release *)(x))->list.next))
#define RELPREV(x)   ((Release *)(((Release *)(x))->list.prev))

#define PATNO(x)     (((Patch *)(x))->list.sort)
#define PATNEXT(x)   ((Patch *)(((Patch *)(x))->list.next))
#define PATPREV(x)   ((Patch *)(((Patch *)(x))->list.prev))

#define FAQNO(x)     (((Faq *)(x))->list.sort)
#define FAQNEXT(x)   ((Faq *)(((Faq *)(x))->list.next))
#define FAQPREV(x)   ((Faq *)(((Faq *)(x))->list.prev))

#define NEWNO(x)     (((New *)(x))->list.sort)
#define NEWNEXT(x)   ((New *)(((New *)(x))->list.next))
#define NEWPREV(x)   ((New *)(((New *)(x))->list.prev))

#define INFNO(x)     (((Inf *)(x))->list.sort)
#define INFNEXT(x)   ((Inf *)(((Inf *)(x))->list.next))
#define INFPREV(x)   ((Inf *)(((Inf *)(x))->list.prev))

typedef struct st_rpFILE {
    FILE *fp;
    char *fileName;
    int lineNo;
    int errorCount;
    int warnCount;
    char *line;
    int eofFlag;
} rpFILE;

typedef struct st_Name {
    struct st_Name *next;
    char **name;
    unsigned long mode;
    void *data;
} Name;

typedef struct st_rpPatch 
{
    ListItem list;
    long date;
    Name *name;
    Name *release;
    char *summary;
    Args notes;
    Args description;
    Args workaround;
    Args affecting;
    Args seeAlso;
} Patch;
    
typedef struct st_rpFaq 
{
    ListItem list;
    long date;
    Name *name;
    Name *release;
    char *question;
    char *module;
    Args notes;
    Args description;
    Args seeAlso;
} Faq;
    
typedef struct st_rpBug {
    ListItem list;
    long flags;
    int status;
    int severity;
    long date;
    char *localId;
    char *module;
    char *component;
    Name *release;
    int  patchNo;
    char *reportBy;
    char *reportFor;
    char *engineer;
    char *summary;
    char *system;
    char *systemSpec;
    Args description;
    Args workaround;
    char *fixBy;
    char *fixStr;
    long fixDate;
    Name *fixRelease;
    Args fixDescription;
    char *unsusBy;
    long unsusDate; 
    char *susBy;
    long susDate;
    Args susDescription;
    Args notes;
    Args seeAlso;
} Bug;

typedef struct st_rpRelease 
{
    ListItem list;
    long date;
    Name *name;
    int  state;
    Args notes;
    Args description;
    Args seeAlso;
} Release;
    
typedef struct st_rpTek 
{
    ListItem list;
    long date;
    Name *name;
    Name *release;
    char *reportBy;
    char *module;
    char *summary;
    Args notes;
    Args description;
    Args seeAlso;
} Tek;
    
typedef struct st_rpNew 
{
    ListItem list;
    long date;
    Name *release;
    char *reportBy;
    char *module;
    char *summary;
    Args description;
    Args notes;
    Args seeAlso;
} New;
    
typedef struct st_rpInf 
{
    ListItem list;
    char *summary;
    Args description;
    Args notes;
} Inf;
    
extern Name *names;
extern Name *swmodules;
extern Name *releaseNames;
extern Name *patchNames;
extern Name undefinedReleaseName;

extern Name *nameAdd (Name **n, char *name, unsigned long mode);
extern Name *nameAddMode (Name **n, char *name, unsigned long mode);
extern char *nameFind (Name *n, char *name, unsigned long mode);
extern Name *nameFindNode (Name *n, char *name);
extern char *nameFindMode (Name *n, char *name, unsigned long *mode);
extern char *nameFindAdd (Name **n, char *name, unsigned long mode);

#define NAME_REPORTED  0x0001
#define NAME_FIXED     0x0002
#define NAME_SUSPENDED 0x0004
#define NAME_ENGINEER  0x0008
#define NAME_REJECTED  0x0010

#define STATE_FULL      0x0001
#define STATE_SPECIFIED 0x0002
#define STATE_PATCHED   0x0003

/* line.c */
extern int reportPackager (rpFILE *fp);
extern void rpClose (rpFILE *rfp);
extern rpFILE *rpOpen (char *fileName);
extern void *rpRealloc (void *f, int i);
extern void *rpMalloc (int i);
extern void rpFree (void *f);

extern rpFILE *rpfp;                    /* File pointer */

/*
 * User functions 
 */

extern int  rp_description (char **list, int count);
extern int  rp_fixedDescription (char **list, int count);
extern int  rp_notes (char **notes, int count);
extern int  rp_seeAlso (char **list, int count);
extern int  rp_suspendedDesc (char **list, int count);
extern int  rp_workaround (char **notes, int count);
extern void rp_patchNumber (int i);
extern int  rp_affecting (char **affecting, int count);
extern void rp_patch (int i);
extern void rp_bugNumber (int i);
extern void rp_component (char *date);
extern void rp_date (unsigned long date);
extern void rp_end (void);
extern void rp_engineer (char *name);
extern void rp_fixedBy (char *s);
extern void rp_fixedDate (unsigned long date, Name *n);
extern void rp_localId (char *s);
extern void rp_module (char *date);
extern void rp_name (char *name);
extern void rp_release (Name *name);
extern void rp_reportBy (char *name);
extern void rp_reportFor (char *name);
extern void rp_releaseNumber (int i);
extern void rp_releaseState (int i);
extern void rp_severity (int i);
extern void rp_status (int i);
extern void rp_summary (char *s);
extern void rp_suspendedBy (char *name);
extern void rp_suspendedDate (unsigned long date);
extern void rp_unsuspendedBy (char *name);
extern void rp_unsuspendedDate (unsigned long date);
extern void rp_system (char *s);
extern void rp_systemSpec (char *s);
extern void rp_tekNumber (int i);
extern void rp_faqNumber (int i);
extern void rp_newNumber (int i);
extern void rp_infNumber (int i);

extern char *strDate (unsigned long date);
extern char *strShortDate (unsigned long date);
extern void outputItemCount (int i, int j);
extern unsigned long todaysDate;

/* tek.c */
extern char *tekIndex (void);
extern Tek  *tekEnd (Tek *tek);
extern void tekInit (void);

/* rel.c */
extern Release *releaseEnd (Release *release);
extern char *releaseIndex (void);
extern void relInit (void);

/* faq.c */
extern char *faqIndex (void);
extern Faq *faqEnd (Faq *faq);
extern void faqInit (void);

/* pat.c */
extern char *patchIndex (void);
extern Patch *patchEnd (Patch *patch);
extern Patch *patchGetHead (void);
extern void patchInit (void);

/* bug.c */
extern Bug *bugGetHead (void);
extern Bug *bugEnd (Bug *bug);
extern char *bugIndex (void);
extern void bugMenuItem (Bug *p, int type);
extern void bugInit (void);

/* new.c */
extern char *newIndex (void);
extern New  *newEnd (New *new);
extern void newInit (void);

/* inf.c */
extern char *infIndex (void);
extern Inf  *infEnd (Inf *inf);
extern void infInit (void);

#endif /* __IDC_H__ */
