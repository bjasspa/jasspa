/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * osd.c - On-Screen Display routines.
 *
 * Copyright (C) 1997-2001 Jon Green & Steven Phillips    
 * Copyright (C) 2002 JASSPA (www.jasspa.com)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 675 Mass Ave, Cambridge, MA 02139, USA.
 */
/*
 * Created:     26/07/97
 * Synopsis:    On-Screen Display routines.
 * Authors:     Jon Green & Steven Phillips
 * Description:
 *     This file contains the on screen display routines that
 *     generate menus; dialogues and alike.
 */

/* Include defintions */

#define __OSDC                          /* Define filename */

/* Include files */
#include "emain.h"                      /* Standard Microemacs definitions */

#if MEOPT_OSD

#include "efunc.h"                      /* Define the command identifiers */
#include "eskeys.h"                     /* Define the key definitions */
#include "eterm.h"

#ifdef _DOS
#include <pc.h>
#endif

/* Local Definitions */
#define MF_DISABLE  0x00000001          /* D - disabled Menu item */
#define MF_DISPTYPE 0x00000002          /* d - Display menu type (i.e. the .., > etc */
#define MF_MANUAL   0x00000004          /* m - Manual open */
#define MF_SUBMNU   0x00000008          /* M - Sub-menu item */
#define MF_LINEFILL 0x00000010          /* - - Pad extra chars with '-', else ' ' */
#define MF_CHECK    0x00000020          /* C - This is a check-box item */
#define MF_NOEXIT   0x00000040          /* x - Do not exit */
#define MF_STR      0x00000080          /* i - Execute as a line-string */
#define MF_EAST     0x00000100          /* e - Make sub-menu popup east  */
#define MF_SOUTH    0x00000200          /* s - Make sub-menu popup south */
#define MF_WEST     0x00000400          /* w - Make sub-menu popup west  */
#define MF_NORTH    0x00000800          /* n - Make sub-menu popup north */
#define MF_HORZADD  0x00001000          /* h - Add to the next menu line */
#define MF_CENTER   0x00002000          /* c - Center the item */
#define MF_TAB      0x00004000          /* t - has tab position */
#define MF_SCRLBOX  0x00008000          /* b - child scroll box */
#define MF_RIGHT    0x00010000          /* r - push item right */
#define MF_NOFILL   0x00020000          /* f - Do not fill the item to the full length */
#define MF_ENTRY    0x00040000          /* E - Entry field type */
#define MF_BUTTON   0x00080000          /* B - Button field type */
#define MF_PREPND   0x00100000          /* p - prepend the check-box */
#define MF_SEP      0x00200000          /* S - Separator field type */
#define MF_REDRAW   0x00400000          /* R - Redraw the current menu */
#define MF_SCHEME   0x00800000          /* H - drawn color scHeme (or Hilight) */
#define MF_CHILD    0x01000000          /* I - chIld or Include dialog */
#define MF_GRID     0x02000000          /* G - Draw grid around item */
#define MF_NBPAGE   0x04000000          /* P - Note book page */
#define MF_SIZE     0x08000000          /* z - Specify an item size */
#define MF_NINPUT   0x10000000          /* N - ML Newline input style */
#define MF_REPEAT   0x20000000          /* T - Button Repeat execution */
#define MF_NHILIGHT 0x40000000          /* X - No hilighting when under mouse */

static meUByte osdItemFlags[]="DdmM-CxieswnhctbrfEBpSRHIGPzNTu" ; 

/* Following are the internal flags worked out from the information given */
#define MFI_NOTEXT  0x00000001          /* No text is given, just pad */
#define MFI_ARG     0x00000002          /* Argument is present */


#define RF_BOARDER  0x00000001          /* b - Draw boarder around menu */
#define RF_ABSPOS   0x00000002          /* a - Menu has an absolute position */
#define RF_ALPHA    0x00000004          /* A - Alphabetic sorted list */
#define RF_TITLE    0x00000008          /* t - Draw a title bar */
#define RF_DEFAULT  0x00000010          /* d - Default item to select */
#define RF_CENTER   0x00000020          /* c - Center the title text */
#define RF_RIGHT    0x00000040          /* r - push title text right */
#define RF_TSCHEME  0x00000080          /* H - title drawn color scHeme (or Hilight) */
#define RF_SIZE     0x00000100          /* s - The dialog size */
#define RF_MSCHEME  0x00000200          /* S - The main dialog scheme */
#define RF_RESIZE   0x00000400          /* R - The dialog has a resize macro */
#define RF_MAINMN   0x00000800          /* M - The main top menu */
#define RF_CONTROL  0x00001000          /* C - The dialog has a contorlling command */
#define RF_HOTKEY   0x00002000          /* k - Disable hot keys for this dialog */
#define RF_NOTEBOOK 0x00004000          /* N - Dialog is a notebook */
#define RF_GRID     0x00008000          /* G - Dialog is a grid layout */
#define RF_NEWLINE  0x00010000          /* n - Dialog is a grid layout */
#define RF_FRSTLET  0x00020000          /* f - Use first letter as hotkey */
#define RF_INSENSE  0x00040000          /* i - Case insensitive (Alpha lists) */
#define RF_OFFSTPOS 0x00080000          /* o - Dialog Offset position */
#define RF_FOCALITM 0x00100000          /* F - Set the item to focus on - temporary */
#define RF_RISLBUT  0x00200000          /* B - Right is list button */

#define RF_CONFIG   0x08000000          /* The display is currently being configured */
#define RF_REDRAW   0x10000000          /* The menu has been changed - redraw */
#define RF_CHILD    0x20000000          /* This is a display shown as a child */
#define RF_DISABLE  0x40000000          /* This is a temporarily disabled */
#define RF_NOPOP    0x80000000          /* Flag to stop the pop with a control */

static meUByte osdMenuFlags[]="baAtdcrHsSRMCkNGnfioFB" ;


#define CF_HORZADD  0x0001              /* Add to the next menu line */
#define CF_HORZCUT  0x0002              /* Flags that ran out of room for HORZADD */
#define CF_SET      0x0004              /* State is set */
#define CF_ROWFRST  0x0010              /* Item on first row */
#define CF_ROWTGGL  0x0020              /* Item row toggle */
#define CF_ROWMASK  0x0030              /* Item row mask */
#define CF_NBPTOP   0x0040              /* Notebook Page on top */
#define CF_NBPBOT   0x0080              /* Notebook Page on top */

/* Local type definitions */


/*
 * osdITEM
 * Contains the user definition of the menu item.
 */
typedef struct osdITEM
{
    struct osdITEM *next ;              /* Next menu item */
    struct osdITEM *prev ;              /* Previous menu item */
    meUByte *strData ;                  /* String data */
    meInt  argc;                        /* Argument */
    meInt  flags;                       /* Item is enabled */
    meScheme scheme;                    /* Items scheme */
    meShort  item;                      /* Menu item */
    meShort  cmdIndex;                  /* Command index or string start index */
    meShort  tab;                       /* Tab order number */
    meShort  len;                       /* Rendered length of the menu */
    meShort  height;                    /* Rendered height of the menu */
    meUByte  iflags;                    /* internal flags */
    meUByte  key;                       /* The actual key */
} osdITEM;

typedef struct osdDIALOG
{
    struct osdDISPLAY *displays;        /* list of displays */
    struct osdDIALOG *next;             /* Next root item */
    struct osdDIALOG *prev;             /* Previous root item */
#if MEOPT_LOCALBIND
    struct meBind *binds ;		/* pointer to osd bindings */
#endif
    osdITEM *itemHead;                  /* Head of list of items */
    osdITEM *itemTail;                  /* Tail of list of items */
    meUByte *strData ;                  /* String data */
    int      flags;                     /* Identification of root item */
#if MEOPT_LOCALBIND
    meUShort nobinds;                   /* no of osd binds */
#endif
    meScheme mScheme;                   /* Main dialog color scheme */
    meScheme tScheme;                   /* title bar color scheme */
    meShort  id ;                       /* Identity of root item */
    meShort  cmdIndex;                  /* Command index or string start index */
    meShort  cntIndex;                  /* Control index or string start index */
    meShort  rszIndex;                  /* Command to resize the dialog */
    meShort  defItem ;                  /* default item number */
    meShort  focalItem ;                /* item to focus on next redraw - temporary */
    meShort  x;                         /* Absolute Position x of menu */
    meShort  y;                         /* Absolute Position y of menu */
    meShort  width[2];                  /* width of menu (min/max) */
    meShort  depth[2];                  /* depth of menu (min/max) */
} osdDIALOG;

typedef struct {
    int      totLen ;
    int      curPos ;
    int      wndLen ;
    
    int      sbarLen ;
    int      sbarMaxLen ;
    int      barTop ;
    int      barLen ;
    int      boxTop ;
    int      boxLen ;
    
    /* note the positions could be negative due to osd scroll dialogs */
    meShort  xPos ;
    meShort  yPos ;
    meShort  flags ;
    
    meUByte  data[2] ;
} meSCROLLBAR ;

typedef struct {
    struct osdDISPLAY *display ;
    meSCROLLBAR *vertSBar ;
    meSCROLLBAR *horzSBar ;
    meShort  wndWidth ;
    meShort  wndDepth ;
} osdCHILD ;

/*
 * osdCONTEXT
 * Contains the runtime configuration of a single menu item. Retaining
 * it's position on the screen, field length etc. Points to the 
 * menu item.
 */
typedef struct
{
    osdITEM  *menu;                     /* Pointer into the menu */
    osdCHILD *child;                    /* Pointer to the child if present */
    meShort  mcflags;                   /* Flags associated with menu */
    meShort  x;                         /* Screen position of the item in x */
    meShort  y;                         /* Screen position of the item in y */
    meShort  width;                     /* The displayed length of the menu item */
    meShort  depth;                     /* The displayed height of the menu item */
    meShort  awidth;                    /* The actual length of the menu item */
    meShort  adepth;                    /* The actual height of the menu item */
} osdCONTEXT;                           /* The context of the menu */

/*
 * osdDISPLAY
 * The single instanace of the menu on the screen. Contains the 
 * root information for a displayed menu.
 */
typedef struct osdDISPLAY
{
    struct osdDISPLAY *displays;        /* Next display of this menu */
    struct osdDISPLAY *next;            /* Next menu level */
    struct osdDISPLAY *prev;            /* Prev menu level */
    osdDIALOG *dialog ;                 /* The dialog being displayed */
    int        flags ;                  /* Current flags */
    int        area ;                   /* The storage area allocated */
    meScheme  *storeSchm ;              /* snap shot stored colp */
    meUByte   *storeText ;              /* snap shot stored text */
    meScheme  *drawnSchm ;              /* snap shot drawn colp */
    meUByte   *drawnText ;              /* snap shot drawn text */
    meShort    nbpContext;              /* Context number of the current notebook page */
    meShort    curContext;              /* Current context */
    meShort    newContext;              /* Current context */
    meShort    numContexts;             /* Number of contexts */
    meShort    multi ;                  /* Multiple colums/rows */
    meShort    x;                       /* Position x of menu */
    meShort    y;                       /* Position y of menu */
    meShort    focalX[2];               /* Focal Position min/max x of menu */
    meShort    focalY[2];               /* Focal Position min/max y of menu */
    meShort    width ;                  /* Width of menu */
    meShort    depth ;                  /* Depth of menu */
    osdCONTEXT context [1];             /* Context of the menu */
} osdDISPLAY;

/* Local Variable Definitions */
static osdDISPLAY *osdCurMd ;
static osdDISPLAY *osdCurChild ;
static osdDISPLAY *osdNewMd ;
static osdDISPLAY *osdNewChild ;

static osdDISPLAY *osdMainMenuMd=NULL ;
int osdMainMenuId=-1 ;



/*****************************************************************************
 * 
 * osdDIALOG menu functions
 * 
****************************************************************************/
/*
 * dialogFind
 * Find the root of the on-screen data definitions
 */
static osdDIALOG *
dialogFind (int id)
{
    osdDIALOG *rp;                           /* Pointer to item */
    
    /* Iterate down the list looking for the block */
    for (rp = osdDialogHd ; rp != NULL ; rp = rp->next)
        if (rp->id == id)
            break ;
    return rp;                          /* Return the pointer */
}

static void
dialogResetDisplays(osdDIALOG *rp, int type)
{
    osdDISPLAY *md ;    
    
    if(((md=rp->displays) != NULL) && !(md->flags & RF_REDRAW)) 
    {
        /* The dialog is currently being displayed and it has just been
         * changed so reset the display and set the redraw flag
         */
        while(md != NULL)
        {
            if(!(md->flags & RF_CONFIG))
            {
                md->flags |= RF_REDRAW ;
                if(md == osdMainMenuMd)
                    frameCur->video.lineArray[0].flag |= VFCHNGD ;
                if(type == 2)
                {
                    md->curContext = -1 ;
                    md->newContext = -1 ;
                }
                else if(type == 1)
                {
                    if((md->curContext >= 0) && (md->context[md->curContext].menu->flags & (MF_DISABLE|MF_SEP)))
                        md->curContext = -1 ;
                    if((md->newContext >= 0) && (md->context[md->newContext].menu->flags & (MF_DISABLE|MF_SEP)))
                        md->newContext = -1 ;
                }
                if(md->flags & RF_CHILD)
                {
                    osdDISPLAY *tmd ;    
                    if(type == 2)
                    {
                        md->x = 0 ;
                        md->y = 0 ;
                    }
                    tmd = md ;
                    do {
                        tmd = tmd->prev ;
                    } while(tmd->flags & RF_CHILD) ;
                    tmd->flags |= RF_REDRAW ;
                }
            }
            md = md->displays ;
        }
    }
}

/*
 * dialogDestruct
 * Destruct the root of a menu given an id. Destructs the siblings
 * associated with the root and then destructs the root itself.
 */
static void
dialogDestruct (int id)
{
    osdDIALOG *rp;
    
    if ((rp = dialogFind (id)) != NULL)
    {
        osdITEM *mp;                       /* Pointer to the menu item */
        
        /* Destruct the siblings associated with the root */
        while ((mp = rp->itemHead) != NULL)
        {
            rp->itemHead = mp->next;     /* Continue linkage */
            meNullFree (mp->strData);   /* Destruct the str data */
            meFree (mp);                /* Destruct the node */
        }
        rp->itemTail = NULL;
        /* Destruct the malloced area of the root */
        if(rp->strData != NULL)
        {
            meFree(rp->strData) ;       /* Destruct the str data */
            rp->strData = NULL ;
        }
        /* reset the flags and dialog size */
        rp->flags = 0 ;
        rp->width[0] = 0 ;
        rp->width[1] = 0 ;
        rp->depth[0] = 0 ;
        rp->depth[1] = 0 ;
        dialogResetDisplays(rp,2) ;
    }
}

/*
 * dialogConstruct
 * Construct a new root node and insert it into the list. Allocates space
 * for the new root item and then adds it to the list.
 */
static osdDIALOG *
dialogConstruct (int id)
{
    osdDIALOG *rp;
    osdDIALOG *rpp;                          /* Previous root pointer */
    
    if((rp = dialogFind (id)) != NULL)
    {
        meNullFree(rp->strData) ;       /* Destruct the str data */
        rp->strData = NULL ;
        return rp ;
    }
    /* Find previous node */
    for (rpp = NULL, rp = osdDialogHd ; rp != NULL ; rp = rp->next)
    {
        if (rp->id > id)
            break;
        rpp = rp;
    }
    
    /* Insert the new node */
    if ((rp = (osdDIALOG *) meMalloc (sizeof (osdDIALOG))) != NULL)
    {        
        memset (rp, 0, sizeof (osdDIALOG));  /* Clear the node */
        rp->id = id;                    /* Add the identity */
        
        /* Link to previous node */
        if ((rp->prev = rpp) == NULL)
        {
            rp->next = osdDialogHd ;
            osdDialogHd = rp;
        }
        else
        {
            rp->next = rpp->next;
            rpp->next = rp;
        }
        /* Fix forward pointer */
        if (rp->next != NULL)
            rp->next->prev = rp;
    }
    return rp;                          /* Return the new node */
}

static void displayDestruct (osdDISPLAY *md) ;

static void
displayDestructChild(osdCHILD *child)
{
    if(child->vertSBar != NULL)
        free(child->vertSBar) ;
    if(child->horzSBar != NULL)
        free(child->horzSBar) ;
    if(child->display != NULL)
        displayDestruct(child->display) ;
    free(child) ;
}

static void
displayDestruct (osdDISPLAY *md)
{
    osdDISPLAY *tmd ;
    int ii ;
    
    /* if this is the current display, set osdCurMd to the previous */
    if (md == osdCurMd)
    {
        osdCurChild = osdCurMd = md->prev ;
        if(osdCurChild != NULL)
        {
            while((osdCurChild->curContext >= 0) &&
                  (osdCurChild->context[osdCurChild->curContext].menu->flags & MF_CHILD))
                osdCurChild = osdCurChild->context[osdCurChild->curContext].child->display ;
        }
    }
    if (md == osdNewMd)
    {
        osdNewChild = osdNewMd = md->prev ;
        if(osdNewChild != NULL)
        {
            while((osdNewChild->newContext >= 0) &&
                  (osdNewChild->context[osdNewChild->newContext].menu->flags & MF_CHILD))
                osdNewChild = osdNewChild->context[osdNewChild->newContext].child->display ;
        }
    }
    if((tmd=md->dialog->displays) == md)
        md->dialog->displays = md->displays ;
    else
    {
        while(tmd->displays != md)
            tmd = tmd->displays ;
        tmd->displays = md->displays ;
    }
    
    for (ii=0 ; ii < md->numContexts ; ii++)
        if(md->context[ii].child != NULL)
            displayDestructChild(md->context[ii].child) ;
    
    meFree(md->storeSchm) ;
    meFree(md) ;
}

/*
 * itemFind
 * Find an item in the root list
 */
static osdITEM *
itemFind (osdDIALOG *rp, int item, meUByte *name)
{
    osdITEM *mp;
    int status;
    
    /* search for item in reverse order as 99% of times they're added in order */
    if (rp->flags & RF_ALPHA)
    {
        register meUByte *ss ;
    
        for (mp = rp->itemTail; mp != NULL; mp = mp->prev)
        {
            /* Compare the item string data */
            ss = mp->strData ;
            if(mp->flags & MF_CHECK)
                ss += 6 ;
            if (rp->flags & RF_INSENSE)
                status = meStridif (ss,name);
            else
                status = meStrcmp (ss,name);
            /* Evaluate presence of node */
            if (status == 0)
                return mp;
            else if (status < 0)
                break;
        }
    }
    else
    {
        for (mp = rp->itemTail; mp != NULL; mp = mp->prev)
        {
            /* Compare the node identities */
            if((status = (mp->item - item)) == 0)
                return mp;
            else if (status < 0)
                break;
        }
    }
    return NULL;
}

/*
 * itemAdd
 * Add an item in the root list
 */
static void
itemAdd (osdDIALOG *rp, osdITEM *newmp)
{
    osdITEM *mp;                           /* Menu pointer */
    meUByte *ns ;
    int status;
    
    if (rp->flags & RF_ALPHA)
    {
        ns = newmp->strData ;
        if(newmp->flags & MF_CHECK)
            ns += 6 ;
    }
    /* search for item in reverse order as 99% of times they're added in order */
    /* Determine if this is a string sorted list or a numeric list */
    if (rp->flags & RF_ALPHA)
    {
        register meUByte *ss ;
    
        for (mp = rp->itemTail; mp != NULL; mp = mp->prev)
        {
            /* Compare the item string data */
            ss = mp->strData ;
            if(mp->flags & MF_CHECK)
                ss += 6 ;
            if (rp->flags & RF_INSENSE)
                status = meStridif (ss,ns);
            else
                status = meStrcmp (ss,ns);
            if (status <= 0)
                break;
        }
    }
    else
    {
        for (mp = rp->itemTail; mp != NULL; mp = mp->prev)
        {
            /* Compare the node identities */
            if((mp->item - newmp->item) <= 0)
                break;
        }
    }
    /* Add the new item to the list */
    if(mp == NULL)
    {
        newmp->prev = NULL ;
        newmp->next = rp->itemHead ;
        rp->itemHead = newmp;
    }
    else
    {
        newmp->prev = mp ;
        newmp->next = mp->next ;
        mp->next = newmp;
    }
    if(newmp->next == NULL)
        rp->itemTail = newmp;
    else
        newmp->next->prev = newmp ;
    
    if (rp->flags & RF_ALPHA)
    {
        /* if its an alpha dialog then replace the item number with the
         * sequential next and update the following */
        status = (newmp->prev == NULL) ? 0:newmp->prev->item ;
        do
        {
            newmp->item = ++status ;
            newmp = newmp->next ;
        } while(newmp != NULL) ;
    }
}

/*    
 * menuSetArgs
 * Set up the menu arguments for calling an external function
 */
static int
menuExecute (osdITEM *mp, int flags, int n)
{
    meUByte *p, *q;                        /* Local character pointers */
    int   cc, f, mpflags ;              /* Immediate character */
    meUByte mlStatusStore ;
    meUByte oldAllKeys ;
    meUByte oldUseMlBinds ;
    
	/* NOTE - we must cache the mp->flags as the item may not
	 * exist after execution. */
    if ((mpflags=mp->flags) & MF_NBPAGE)
    {
        /* This can only get executed when the user has selected it, so
         * we can use the osdCurChild variable */
        if(!(mp->iflags & MFI_ARG) || (osdCurChild->nbpContext == -1))
            return meABORT ;
        if(osdCurChild->context[osdCurChild->nbpContext].menu->argc != mp->argc)
        {
            if(osdCurChild->context[osdCurChild->nbpContext].child != NULL)
            {
                displayDestructChild(osdCurChild->context[osdCurChild->nbpContext].child) ;
                osdCurChild->context[osdCurChild->nbpContext].child = NULL ;
            }
            osdCurChild->context[osdCurChild->nbpContext].menu->argc = mp->argc ;
        }
        return meTRUE ;
    }

    /* Make sure that there is something to do */
    if(mp->cmdIndex < 0)
        return meABORT;

    oldAllKeys = TTallKeys ;
    TTallKeys = 0 ;
    if (mpflags & MF_ENTRY)
    {
        /* If entry then must set frameCur->mlStatus to 0x08 bit to get entries at the osd
         * entry field, must back up previous value.
         * Also flush input to get rid of any mouse-moves
         */
        mlStatusStore = frameCur->mlStatus ;
        oldUseMlBinds = useMlBinds ;
        
        useMlBinds = 1 ;
    
        frameCur->mlStatus = MLSTATUS_POSOSD ;
        if(mpflags & MF_NINPUT)
            frameCur->mlStatus |= MLSTATUS_NINPUT ;
        if(osdCol >= 0)
            frameCur->mlStatus |= MLSTATUS_OSDPOS ;
        if(n < 0)
            TTinflush() ;
        else if(osdCol >= 0)
            TTallKeys = oldAllKeys ;
    }
    else
    {
        /* Return the menu name to the caller. Strip out any '&' delimiters
         * from the string prior to return. */
        q = resultStr;                      /* Start of the result string */
        if (mp->iflags & MFI_NOTEXT)
            *q = '\0' ;
        else
        {
            p = mp->strData ;
            if(mpflags & MF_CHECK)
                p += 6 ;
            if(flags & (RF_HOTKEY|RF_FRSTLET))
                meStrcpy(q,mp->strData) ;
            else
            {
                do
                {
                    if (((cc=*p++) == '\\') || (cc == '&'))
                        cc = *p++ ;
                    *q++ = cc;
                } while(cc != '\0') ;
            }
        }
    }
    
    /* Get the f and n out */
    f = ((mp->iflags & MFI_ARG) != 0) ;  /* Argument flag */
    
    if (mpflags & (MF_ENTRY|MF_CHECK))
    {
        if(!f)
            f = 1 ;
        else if(n < 0)
            n = 0 - mp->argc ;
        else
            n = mp->argc ;
    }
    else
        n = mp->argc ;

    /* Call the function specified */
    if (mpflags & MF_STR)             /* String function */
        f = lineExec (f, n, mp->strData + mp->cmdIndex);
    else
        f = execFunc ((int) mp->cmdIndex, f, n);
    if (mpflags & MF_ENTRY)
    {
        /* Restore the previous frameCur->mlStatus value */
        frameCur->mlStatus = mlStatusStore ;
        useMlBinds = oldUseMlBinds ;
    }
    TTallKeys = oldAllKeys ;
    return f;
}

/*
 * menuRenderArea
 * 
 * Render the given area to the screen from the frameCur->store
 */
static void
menuRenderArea(int x, int y, int len, int dep)
{
    if(x < 0)
    {
        len += x ;
        x = 0 ;
    }
    if((x+len) >= frameCur->width)
        len = frameCur->width - x ;
    if(y < 0)
    {
        dep += y ;
        y = 0 ;
    }
    if((y+dep) > frameCur->depth)
        dep = frameCur->depth + 1 - y ;
#ifdef _DOS
    {
        meScheme *schmp, scheme ;
        meUByte *textp ;
        meUByte  cc ;
        int   ii, xx ;
    
        while(--dep >= 0)
        {
            schmp = frameCur->store[y].scheme + x ;
            textp = frameCur->store[y].text + x ;
            ii = len ;
            xx = x ;
            while(--ii >= 0)
            {
                scheme = *schmp++ ;
                cc = TTschemeSet(scheme) ;
                ScreenPutChar(*textp++,cc,xx++,y);
            }
            y++ ;
        }
    }
#endif

#ifdef _WIN32
#ifdef _ME_CONSOLE
#ifdef _ME_WINDOW
    if (meSystemCfg & meSYSTEM_CONSOLE)
#endif /* _ME_WINDOW */
    {
        meScheme *schmp, scheme ;
        meUByte *textp ;
        WORD  cc ;
        int   ii, xx ;
    
        while(--dep >= 0)
        {
            schmp = frameCur->store[y].scheme + x ;
            textp = frameCur->store[y].text + x ;
            ii = len ;
            xx = x ;
            while(--ii >= 0)
            {
                scheme = *schmp++ ;
                cc = (WORD) TTschemeSet(scheme) ;
                ConsoleDrawString (textp++, cc, xx++, y, 1);
            }
            y++ ;
        }
    }
#ifdef _ME_WINDOW
    else
#endif /* _ME_WINDOW */
#endif /* _ME_CONSOLE */
#ifdef _ME_WINDOW
    {
        TTsetArea(y,x,dep,len) ;
        TTapplyArea() ;
    }
#endif /* _ME_WINDOW */
#endif /* _WIN32 */

#ifdef _UNIX
#ifdef _ME_WINDOW
#ifdef _ME_CONSOLE
    if(!(meSystemCfg & meSYSTEM_CONSOLE))
#endif /* _ME_CONSOLE */
#ifdef _XTERM
        meFrameXTermDraw(frameCur,y,x,y+dep,x+len) ;
#endif /* _XTERM */
#ifdef _ME_CONSOLE
    else
#endif /* _ME_CONSOLE */
#endif /* _ME_WINDOW */
#ifdef _ME_CONSOLE
    {
#ifdef _TCAP
        meScheme *schmp ;
        meScheme scheme ;
        meUByte  cc ;
        meUByte *textp ;
        int   ii ;
        
        while(dep--)
        {
            schmp = frameCur->store[y].scheme + x ;
            textp = frameCur->store[y].text + x ;
            ii = len ;
            TCAPmove(y++,x);	/* Go to correct place. */
            while(ii--)
            {
                scheme = *schmp++ ;
                TCAPschemeSet(scheme) ;
                cc = *textp++ ;
                TCAPputc(cc) ;
            }
        }
        TCAPschemeReset() ;
        /* restore cursor position */
        if((cursorState >= 0) && blinkState)
            TTshowCur() ;
        else
            TThideCur() ;
        TCAPflush() ;
#endif
    }
#endif /* _ME_CONSOLE */
#endif /* _UNIX */

}



/*
 * osdDisplaySnapshotCreate
 * 
 * Allocate the memory required to capture the screen behind the menu.
 */
static int
osdDisplaySnapshotCreate(osdDISPLAY *md)
{
    int area;                           /* Area of the screen */

    /* Allocate a new area */
    area = md->depth * md->width;      /* Pixel area coverage */
    if(md->area > 0)
    {
        if(md->area >= area)
            return 1 ;
        meFree(md->storeSchm) ;
    }
    if((md->storeSchm = meMalloc(area*2*(sizeof(meUByte)+sizeof(meUShort)))) == NULL)
        return 0 ;
    
    md->drawnSchm = (md->storeSchm+area) ;
    md->storeText = (meUByte *) (md->drawnSchm+area) ;
    md->drawnText = (md->storeText+area) ;
    md->area = area ;
    return 1 ;
}

/*
 * osdDisplaySnapshotStore
 * Capture the screen behind the menu.
 */
static void
osdDisplaySnapshotStore(osdDISPLAY *md)
{
    meFrameLine *flp ;
    meScheme  *colp ;
    meUByte     *text ;
    int depth, width, xx, yy ;
    
    /* Copy the region from the frame store into the buffer */
    width = md->width ;
    depth = md->depth ;
    if((xx=md->x) < 0)
    {
        width += xx ;
        xx = 0 ;
    }
    if((xx+width) >= frameCur->width)
        width = frameCur->width - xx ;
    if((yy = md->y) < 0)
    {
        depth += yy ;
        yy = 0 ;
    }
    if((yy+depth) > frameCur->depth)
        depth = frameCur->depth + 1 - yy ;
    flp  = frameCur->store + yy ;
    text = md->storeText ;
    colp = md->storeSchm ;
    
    for ( ; --depth >= 0 ; flp++,text+=width,colp+=width)
    {
        memcpy (colp,flp->scheme+xx,width*sizeof(meScheme));
        memcpy (text,flp->text+xx,width*sizeof(meUByte));
    }
}
/*
 * osdDisplaySnapshotRestore
 * Capture the screen behind the menu.
 */
static void
osdDisplaySnapshotRestore(osdDISPLAY *md)
{
    meFrameLine *flp ;
    meScheme  *colp ;
    meUByte     *text ;
    int width, depth, xx, yy ;
    
    /* Restore the old area of the screen */
    /* Copy the region from the buffer into the frame store */
    width = md->width ;
    depth = md->depth ;
    if((xx=md->x) < 0)
    {
        width += xx ;
        xx = 0 ;
    }
    if((xx+width) >= frameCur->width)
        width = frameCur->width - xx ;
    if((yy = md->y) < 0)
    {
        depth += yy ;
        yy = 0 ;
    }
    if((yy+depth) > frameCur->depth)
        depth = frameCur->depth + 1 - yy ;
    
    flp  = frameCur->store + yy ;
    text = md->storeText ;
    colp = md->storeSchm ;
    for(; --depth >= 0 ; flp++,text+=width,colp+=width)
    {
        memcpy (flp->scheme+xx,colp, width*sizeof(meScheme));
        memcpy (flp->text+xx,text, width*sizeof(meUByte));
    }
}

static void
osdDisplaySnapshotDraw(osdDISPLAY *md, int sx, int sy, int nx, int ny, int drawToScreen)
{
    meFrameLine *flp ;
    meScheme  *colp ;
    meUByte     *text ;
    int width, ii ;
    
    /* Draw the osd dialog to the screen */
    /* Copy the region from the buffer into the frame store */
    /* check the region to be drawn is visible first */
    if(md->x < -sx)
    {
        ii = md->x + sx ;
        if((nx += ii) <= 0)
            return ;
        sx -= ii ;
    }
    if((ii = md->x + sx + nx - frameCur->width) > 0)
    {
        if((nx -= ii) <= 0)
            return ;
    }
    if(md->y < -sy)
    {
        ii = md->y + sy ;
        if((ny += ii) <= 0)
            return ;
        sy -= ii ;
    }
    if((ii = md->y + sy + ny - frameCur->depth - 1) > 0)
    {
        if((ny -= ii) <= 0)
            return ;
    }
    width = md->width ;
    text = md->drawnText + (width*sy) + sx ;
    colp = md->drawnSchm + (width*sy) + sx ;
    sx += md->x ;
    sy += md->y ;
    flp  = frameCur->store + sy ;
    
    for (ii=ny ; --ii >= 0 ; flp++,text+=width,colp+=width)
    {
        memcpy (flp->scheme+sx, colp, nx*sizeof(meScheme));
        memcpy (flp->text+sx, text, nx*sizeof(meUByte));
    }
    if(drawToScreen)
        menuRenderArea(sx,sy,nx,ny) ;
}


/*
 * menuSetItemDetails
 * Determines the details of the menu item.
 */
static void
menuSetItemLength (osdDIALOG *rp, osdITEM *mp)
{
    meUByte *p;                           /* Pinter to the text */
    meUByte cc;                           /* Immediate character */
    int len, clen;                      /* Length of text */
    
    mp->key = '\0' ;
    if(mp->flags & (MF_CHILD|MF_SIZE))
        return ;
    
    mp->height = 1 ;
    if(mp->iflags & MFI_NOTEXT)
    {
        if(mp->flags & MF_HORZADD)
            /* Separaters with no text are 1 char long in a horiz pack */
            mp->len = 1 ;
        else
            mp->len = 0 ;
        return ;
    }
    
    p = mp->strData;                    /* Point to the string */
    if(mp->flags & MF_CHECK)
        p += 6 ;
    
    len = clen = 0 ;
    if(rp->flags & (RF_HOTKEY|RF_FRSTLET))
    {
        if(rp->flags & RF_FRSTLET)
            mp->key = *p ;
        for(;;)
        {
            if((cc = *p++) == '\0')
                break ;
            if((cc == meNLCHAR) && !(rp->flags & RF_NEWLINE))
            {
                mp->height++ ;
                if(clen > len)
                    len = clen ;
                clen = 0 ;
            }
            else
                clen++;
        }
    }
    else
    {
        /* Process the menu text escape character '&' */
        for(;;)
        {
            if(((cc = *p++) == meNLCHAR) && !(rp->flags & RF_NEWLINE))
            {
                (mp->height)++ ;
                if(clen > len)
                    len = clen ;
                clen = 0 ;
            }
            else
            {
                if(cc == '\\')
                    cc = *p++ ;
                else if(cc == '&')
                {
                    if((cc = *p++) != '\0')
                        mp->key = cc ;          /* The character */
                }
                if(cc == '\0')
                    break ;
                clen++;
            }
        }
    }
    if(clen > len)
        len = clen ;
    /* Determine if this is a special-menu & if so whether 
     * to display the type. If so then add the appropriate
     * value for an auto or manual open sub-menu
     */
    if(mp->flags & MF_CHECK)
    {
        if(mp->strData[0] != '\0')
            len++ ;
        if(mp->strData[1] != '\0')
            len++ ;
        if(mp->strData[2] != '\0')
            len++ ;
        if(mp->strData[4] != '\0')
            len++ ;
        if(mp->strData[5] != '\0')
            len++ ;
    }
    else if((mp->flags & MF_BUTTON) ||
            ((mp->flags & MF_DISPTYPE) && (mp->flags & MF_SUBMNU)))
        /* Add 2 for '[xxxx]', '..' or ' >' */
        len += 2;
    /* set up the length */
    mp->len = len ;
}

typedef void (*meScrollBarDrawFunc)(meSCROLLBAR *sb) ;
meScrollBarDrawFunc meScrollBarDrawBar ;
meScrollBarDrawFunc meScrollBarDrawMain ;

static int
fillScrollBar(register meSCROLLBAR *sb, int redraw)
{
    meUByte  *txtp1, *txtp2, *wbase ;
    int boxTop, len ;
    int boxLen ;                    /* Length of the box */
    
    if(sb->flags & WMSCROL)
    {
        if (sb->totLen > sb->barLen)
        {
            int shaftMovement;              /* Permitted movement on shaft */
            
            boxLen = (sb->wndLen * sb->barLen) / sb->totLen ;
            if (boxLen == 0)
                boxLen = 1;
            if((sb->curPos > 0) &&
               ((shaftMovement = sb->barLen - boxLen) > 0))
            {
                boxTop = (sb->curPos * shaftMovement) / (sb->totLen - sb->wndLen) ;
                if (boxTop > shaftMovement)
                    boxTop = shaftMovement ;
            }
            else
                boxTop = 0 ;
        }
        else
        {
            boxTop = 0 ;
            boxLen = sb->barLen ;
        }
        if(!redraw && (sb->boxTop == boxTop) &&
           (sb->boxLen == boxLen))
            return 0 ;
        sb->boxTop = boxTop ;
        sb->boxLen = boxLen ;
    }
    else if(!redraw)
        return 0 ;
    
    if (sb->flags & WMHORIZ)
        wbase = &windowChars[WCHSBSPLIT] ;
    else
        wbase = &windowChars[WCVSBSPLIT] ;
    txtp1 = sb->data ;
    if (sb->flags & WMWIDE)
    {
        wbase += WCVSBSPLIT1-WCVSBSPLIT ;
        txtp2 = txtp1+sb->sbarLen ;
        len = 2;
    }
    else
    {
        txtp2 = NULL ;
        len = 1;
    }
    
    if(sb->flags & WMSPLIT)
    {
        *txtp1++ = *wbase++ ;
        if(txtp2)
            *txtp2++ = *wbase++ ;
    }
    else
        wbase += len ;
    if(sb->flags & WMUP)
    {
        *txtp1++ = *wbase++ ;
        if(txtp2)
            *txtp2++ = *wbase++ ;
    }
    else
        wbase += len ;
    
    if(sb->flags & WMSCROL)
    {
        meUByte cc ;
        int ii, jj ;
        
        if((ii = boxTop) != 0)
        {
            cc = *wbase++ ;
            jj = ii ;
            while(--ii >= 0)
                *txtp1++ = cc ;
            if(txtp2)
            {
                cc = *wbase++ ;
                while(--jj >= 0)
                    *txtp2++ = cc ;
            }
        }
        else
            wbase += len ;
        if((ii = boxLen) != 0)
        {
            cc = *wbase++ ;
            jj = ii ;
            while(--ii >= 0)
                *txtp1++ = cc ;
            if(txtp2)
            {
                cc = *wbase++ ;
                while(--jj >= 0)
                    *txtp2++ = cc ;
            }
        }
        else
            wbase += len ;
        if((ii = sb->barLen-boxTop-boxLen) != 0)
        {
            cc = *wbase++ ;
            jj = ii ;
            while(--ii >= 0)
                *txtp1++ = cc ;
            if(txtp2)
            {
                cc = *wbase++ ;
                while(--jj >= 0)
                    *txtp2++ = cc ;
            }
        }
        else
            wbase += len ;
    }
    else
    {
        int ii, jj ;
        
        /* As scroll bar disabled, must fill the slack with spaces */
        if((ii = sb->barLen) != 0)
        {
            jj = ii ;
            while(--ii >= 0)
                *txtp1++ = ' ' ;
            if(txtp2)
            {
                while(--jj >= 0)
                    *txtp2++ = ' ' ;
            }
        }
        if(txtp2)
            wbase += 6 ;
        else
            wbase += 3 ;
    }
    if(sb->flags & WMDOWN)
    {
        *txtp1++ = *wbase++ ;
        if(txtp2)
            *txtp2++ = *wbase++ ;
    }
    else
        wbase += len ;
    if(sb->flags & WMBOTTM)
    {
        *txtp1 = *wbase++ ;
        if(txtp2)
            *txtp2 = *wbase ;
    }
    return 1 ;
}

static meSCROLLBAR *
initScrollBar(meSCROLLBAR *sb, int sbarLen, int flags)
{
    int len ;
    
    if((sb == NULL) || (sb->sbarMaxLen < sbarLen))
    {
        if((sb = meRealloc(sb,sizeof(meSCROLLBAR)+(sbarLen*2))) == NULL)
            return NULL ;
        sb->sbarMaxLen = sbarLen ;
    }
    sb->sbarLen = sbarLen ;
    sb->flags = flags ;
    sb->totLen = -1 ;
    
    /* Must always calc the shaft length as if this is greater than
     * zero even without a scroll bar this must be filled */
    len = 0 ;
    if(flags & WMSPLIT)
        len++ ;
    if(flags & WMUP)
        len++ ;
    sb->barTop = len ;
    if(flags & WMDOWN)
        len++ ;
    if(flags & WMBOTTM)
        len++ ;
    sb->barLen = sbarLen - len  ;
    
    if(flags & WMBOTTM)
        sbarLen-- ;
    sb->wndLen = sbarLen ;
    
    if(flags & WMHORIZ)
    {
        if(flags & WMHWIDE)
            sb->flags |= WMWIDE ;
    }
    else
    {
        if(flags & WMVWIDE)
            sb->flags |= WMWIDE ;
    }
        
    
    return sb ;
}

#define osdRENDITEM_DRAW    0x01        /* draw to screen */
#define osdRENDITEM_KEEPCUR 0x02        /* Keep the current context */
#define osdRENDITEM_REDWALL 0x04        /* Redraw all children including end item */
#define osdRENDITEM_FLLWNEW 0x08        /* Follow down the newContext, not the cur */
#define osdRENDITEM_REFOCUS 0x10        /* Check child focus point */
#define osdRENDITEM_FORCE   0x20        /* Force redraw of whole item eg scroll bars */

static void
menuRenderChildScrollBar(meSCROLLBAR *sb, osdDISPLAY *md, osdCONTEXT *mcp)
{
    meUByte  *txtp1, *txtp2 ;
    meUShort *colp1, *colp2 ;
    meUByte  *sbt1,  *sbt2 ;
    meUShort  scheme, mscheme ;
    int     invOff1, invOff2 ;
    int     txtInc, ii ;
    
    sbt1 = sb->data ;
    if(sb->flags & WMWIDE)
        sbt2 = sbt1+sb->sbarLen ;
    else
        sbt2 = NULL ;
    
    if(sb->flags & WMHORIZ)
    {
        ii = md->width*(mcp->y+mcp->depth-1) + mcp->x + 1 ;
        colp1 = md->drawnSchm + ii ;
        txtp1 = md->drawnText + ii ;
        if(sbt2 != NULL)
        {
            colp2 = colp1 ;
            txtp2 = txtp1 ;
            colp1 -= md->width ;
            txtp1 -= md->width ;
        }
        txtInc = 1 ;
    }
    else
    {
        txtInc = md->width ;
        ii = txtInc*(mcp->y+1) + mcp->x + mcp->width - 1 ;
        colp1 = md->drawnSchm + ii ;
        txtp1 = md->drawnText + ii ;
        if(sbt2 != NULL)
        {
            colp2 = colp1 ;
            txtp2 = txtp1 ;
            colp1-- ;
            txtp1-- ;
        }
    }
    if(sb->flags & WMRVBOX)
    {
        invOff1 = sb->barTop + sb->boxTop ;
        invOff2 = sb->boxLen ;
    }
    else
        invOff1 = -1 ;
    mscheme = mcp->menu->scheme ;
    scheme = mscheme ;
    for(ii=sb->sbarLen ; ii ; ii--)
    {
        if(invOff1 == 0)
        {
            if(invOff2)
            {
                scheme = mscheme^1 ;
                invOff1 = invOff2 ;
                invOff2 = 0 ;
            }
            else
                scheme = mscheme ;
        }
        invOff1-- ;
        *colp1 = scheme ;
        *txtp1 = *sbt1++ ;
        colp1 += txtInc ;
        txtp1 += txtInc ;
        if(sbt2 != NULL)
        {
            *colp2 = scheme ;
            *txtp2 = *sbt2++ ;
            colp2 += txtInc ;
            txtp2 += txtInc ;
        }
    }
}

static void
menuRenderChildMain(osdDISPLAY *md, osdCONTEXT *mcp, int flags)
{
    osdDISPLAY *cmd;                   /* Child menu display pointer */
    osdCHILD   *child ;
    osdITEM    *mp;                    /* Menu pointer */
    meScheme   *colp, *ccolp ;
    meUByte      *txtp, *ctxtp ;
    int         width, depth ;
    int         mdw, cmdw ;
    
    /* Draw the child dialog to the current dialogs snapshot area */
    child = mcp->child ;
    cmd = child->display ;
    mp  = mcp->menu ;
    mdw = md->width ;
    cmdw = cmd->width ;
    colp = md->drawnSchm + (mcp->y*mdw) + mcp->x ;
    txtp = md->drawnText + (mcp->y*mdw) + mcp->x ;
    ccolp = cmd->drawnSchm - (cmd->y*cmdw) - cmd->x ;
    ctxtp = cmd->drawnText - (cmd->y*cmdw) - cmd->x ;
    width = child->wndWidth ;
    depth = child->wndDepth ;
    
    if(mp->flags & MF_SCRLBOX)
    {
        colp += mdw+1 ;
        txtp += mdw+1 ;
    }
        
    for( ; depth ; depth--,txtp+=mdw,colp+=mdw,ctxtp+=cmdw,ccolp+=cmdw)
    {
        memcpy (colp, ccolp, width * sizeof (meUShort));
        memcpy (txtp, ctxtp, width * sizeof (meUByte));
    }
    /* Draw the text region to screen if required - draw the mcp width and height
     * so the scroll bars are also rendered
     */
    if (flags & osdRENDITEM_DRAW)
        osdDisplaySnapshotDraw(md,mcp->x,mcp->y,mcp->width,mcp->depth,1) ;
}

static void
menuRenderChildBoarder (osdDISPLAY *md, osdCONTEXT *mcp)
{
    osdCHILD *child;
    meScheme *colp1, *colp2, scheme ;
    meUByte    *txtp1, *txtp2 ;
    int       width, depth, ii ;
    int       hw, mdw ;
    
    child = mcp->child ;
    mdw = md->width ;
    colp1 = md->drawnSchm + (mcp->y*mdw) + mcp->x ;
    txtp1 = md->drawnText + (mcp->y*mdw) + mcp->x ;
    width = child->wndWidth ;
    depth = child->wndDepth ;
    
    hw = (mcp->awidth-width-2) ;
    
    /* Get the colour of the item */
    scheme = md->dialog->mScheme ;
    
    /* Render to line */
    colp2 = colp1 ;
    txtp2 = txtp1 ;
    *colp2++ = scheme ;
    *txtp2++ = boxChars [BCES] ;
    ii = width ;
    while (--ii >= 0)
    {
        *colp2++ = scheme ;
        *txtp2++ = boxChars [BCEW] ;
    }
    *colp2 = scheme ;
    *txtp2 = boxChars [BCSW] ;
    if(hw)
    {
        colp2[1] = scheme ;
        txtp2[1] = ' ' ;
    }
    /* Render left line */
    colp1 += mdw ;
    txtp1 += mdw ;
    ii = depth ;
    while (--ii >= 0)
    {
        *colp1 = scheme ;
        *txtp1 = boxChars [BCNS] ;
        colp1 += mdw ;
        txtp1 += mdw ;
    }
    *colp1 = scheme ;
    *txtp1 = boxChars [BCNE] ;
    
    /* If theres no vert scroll bar, render the right line
     * (must allow for double width scroll bar flag)
     */
    if(mcp->child->vertSBar == NULL)
    {
        colp2 += mdw ;
        txtp2 += mdw ;
        ii = depth ;
        while (--ii >= 0)
        {
            *colp2 = scheme ;
            *txtp2 = boxChars [BCNS] ;
            if(hw)
            {
                colp2[1] = scheme ;
                txtp2[1] = ' ' ;
            }
            colp2 += mdw ;
            txtp2 += mdw ;
        }
    }
    /* If theres no horz scroll bar, render the bottom line */
    if(mcp->child->horzSBar == NULL)
    {
        colp2 = colp1+1 ;
        txtp2 = txtp1+1 ;
        ii = width ;
        while (--ii >= 0)
        {
            *colp2++ = scheme ;
            *txtp2++ = boxChars [BCEW] ;
        }
    }
    else
    {
        colp2 = colp1+width+1 ;
        txtp2 = txtp1+width+1 ;
    }
    /* render bottom right corner */
    *colp2 = scheme ;
    *txtp2 = boxChars [BCNW] ;
    if(hw)
    {
        colp2[1] = scheme ;
        txtp2[1] = ' ' ;
    }
    /* now check for a wide bottom scroll bar, if wide then
     * render the corners and the bar if there is no scroll bar */
    if(mcp->adepth-depth-2)
    {
        colp1 += mdw ;
        txtp1 += mdw ;
        *colp1++ = scheme ;
        *txtp1++ = ' ' ;
        /* If theres no horz scroll bar, render the bottom line */
        if(mcp->child->horzSBar == NULL)
        {
            ii = width ;
            while (--ii >= 0)
            {
                *colp1++ = scheme ;
                *txtp1++ = ' ' ;
            }
        }
        else
        {
            colp1 += width ;
            txtp1 += width ;
        }
        /* render bottom right corner */
        *colp1 = scheme ;
        *txtp1 = ' ' ;
        if(hw)
        {
            colp1[1] = scheme ;
            txtp1[1] = ' ' ;
        }
    }
}

static int
osdRenderEntryLine(meUByte *txtp, meUByte *ss, int len, int cpos, int ww)
{
    meUByte  expbuf[meBUF_SIZE_MAX+1], *s1 ;
    int start, col ;
    
    if(len == 0)
    {
        memset(txtp,' ',ww) ;
        return 0 ;
    }
    col = 0 ;
    len = expandexp(len,ss,meBUF_SIZE_MAX,0,expbuf,cpos,&col,0) ;
    if(cpos && !col)
        col = len ;
    ww-- ;
    if(col > ww)
    {
        int div = ww >> 1 ;
        
        start = (((col - (div >> 1)) / div) * div) ;
        col -= start-1 ;
        *txtp++ = '$' ;
        ww-- ;
    }
    else
        start = 0 ;
    
    s1 = expbuf + start ;
    len -= start ;
    if(len > ww)
    {
        memcpy(txtp,s1,ww) ;
        txtp[ww] = '$' ;
    }
    else
    {
        memcpy(txtp,s1,len) ;
        memset(txtp+len,' ',ww-len+1) ;
    }
    return col ;
}

static int
osdRenderEntry(meUByte *txtp, meUByte *ss, int flags, int cpos, 
               int totWidth, int ww, int dd, int *crow)
{
    meUByte *s1, *s2 ;                        /* Current character */
    int len, col, row, start ;
    
    if(flags & RF_NEWLINE)
    {
        if(crow != NULL)
            *crow = 0 ;
        return osdRenderEntryLine(txtp,ss,-1,cpos, ww) ;
    }
    
    s1 = meStrchr(ss,meNLCHAR) ;
    s2 = ss ;
    start = 0 ;
    while((s1 != NULL) && ((len=((int) s1 - (int) s2)) < cpos))
    {
        start++ ;
        s2 = s1+1 ;
        cpos -= len+1 ;
        s1 = meStrchr(s2,meNLCHAR) ;
    } 
        
    col = 0 ;
    while(s1 != NULL)
    {
        col++ ;
        s1 = meStrchr(s1+1,meNLCHAR) ;
    }
    row = (dd-1) >> 1 ;
    if((col+1+row) < dd)
        row = dd - 1 - col ;
    if(row > start)
        row = start ;
    if(crow != NULL)
        *crow = row ;
    start -= row ;
    s1 = ss ;
    while(--start >= 0)
        s1 = ((meUByte *)meStrchr(s1,meNLCHAR))+1 ;
    dd -= row+1 ;
    while(--row >= 0)
    {
        s2 = meStrchr(s1,meNLCHAR) ;
        len = ((int) s2 - (int) s1) ;
        osdRenderEntryLine(txtp,s1,len,0,ww) ;
        txtp += totWidth ;
        s1 = s2 + 1 ;
    }
    s2 = meStrchr(s1,meNLCHAR) ;
    if(s2 == NULL)
        len = -1 ;
    else
        len = ((int) s2 - (int) s1) ;
    col = osdRenderEntryLine(txtp,s1,len,cpos,ww) ;
    while(--dd >= 0)
    {
        txtp += totWidth ;
        if(s2 == NULL)
            len = 0 ;
        else
        {
            s1 = s2+1 ;
            s2 = meStrchr(s1,meNLCHAR) ;
            if(s2 == NULL)
                len = -1 ;
            else
                len = ((int) s2 - (int) s1) ;
        }
        osdRenderEntryLine(txtp,s1,len,0,ww) ;
    }
    return col ;
}

static meShort
menuRenderChildRefocus(meShort curPos, meShort childSize, meShort windSize,
                       meShort focalMin, meShort focalMax)
{
    if(((focalMax+curPos) < 0) ||
       ((focalMin+curPos) >= windSize))
    {
        /* Use the focal point to set the scroll */
        if((curPos = (windSize>>1) - focalMin) > 0)
            curPos = 0 ;
        else if((curPos + childSize) < windSize)
            curPos = windSize - childSize ;
    }
    return curPos ;
}

/*
 * menuRenderItem
 * Render the text of a menu. Renders the text in a menu, excluding
 * any border information. 'offset' is the offset into the menu
 * display context array.
 */
static void
menuRenderItem (osdDISPLAY *md, int offset, int flags)
{
    osdCONTEXT *mcp;                    /* Menu context pointer */
    osdITEM *mp;                        /* Menu pointer */
    
    mcp = &md->context [offset];
    mp = mcp->menu ;
        
    if(mp->flags & MF_CHILD)
    {
        osdCHILD    *child;                 /* Child pointer */
        osdDISPLAY  *cmd;                   /* Child menu display pointer */
        int ii ;
        
        if((offset == md->curContext) && !(flags & osdRENDITEM_KEEPCUR))
            md->curContext = -1 ;
        
        /* Draw the child dialog to the current dialogs snapshot area */
        child = mcp->child ;
        cmd = child->display ;
        
        ii = (flags & osdRENDITEM_FLLWNEW) ? cmd->newContext : cmd->curContext ;
        
        if((ii >= 0) &&
           ((flags & osdRENDITEM_REDWALL) || (cmd->context[ii].menu->flags & MF_CHILD)))
            menuRenderItem(cmd,ii,(flags&0xfe)) ;
        
        if(mp->flags & MF_SCRLBOX)
        {
            meSCROLLBAR *horzSBar ;
            meSCROLLBAR *vertSBar ;
            
            if((horzSBar = child->horzSBar) != NULL)
            {
                if(flags & osdRENDITEM_REFOCUS)
                    cmd->x = menuRenderChildRefocus(cmd->x,cmd->width,child->wndWidth,
                                                    cmd->focalX[0],cmd->focalX[1]) ;
                if(flags & osdRENDITEM_FORCE)
                {
                    horzSBar->curPos = -cmd->x ;
                    fillScrollBar(horzSBar,1) ;
                    menuRenderChildScrollBar(horzSBar,md,mcp) ;
                }
                else if((-cmd->x != horzSBar->curPos) &&
                         ((horzSBar->curPos = -cmd->x),
                          fillScrollBar(horzSBar,0)))
                    menuRenderChildScrollBar(horzSBar,md,mcp) ;
            }
            if((vertSBar = child->vertSBar) != NULL)
            {
                if(flags & osdRENDITEM_REFOCUS)
                    cmd->y = menuRenderChildRefocus(cmd->y,cmd->depth,child->wndDepth,
                                                    cmd->focalY[0],cmd->focalY[1]) ;
                if(flags & osdRENDITEM_FORCE)
                {
                    vertSBar->curPos = -cmd->y ;
                    fillScrollBar(vertSBar,1) ;
                    menuRenderChildScrollBar(vertSBar,md,mcp) ;
                }
                else if((-cmd->y != vertSBar->curPos) &&
                         ((vertSBar->curPos = -cmd->y),
                          fillScrollBar(vertSBar,0)))
                    menuRenderChildScrollBar(vertSBar,md,mcp) ;
            }
            if(flags & osdRENDITEM_REFOCUS)
            {
                md->focalX[0] = cmd->focalX[0] + cmd->x + mcp->x ;
                if(md->focalX[0] < 0)
                    md->focalX[0] = 0 ;
                md->focalX[1] = cmd->focalX[1] + cmd->x + mcp->x ;
                if(md->focalX[1] >= child->wndWidth)
                    md->focalX[0] = child->wndWidth-1 ;
                md->focalY[0] = cmd->focalY[0] + cmd->y + mcp->y ;
                if(md->focalY[0] < 0)
                    md->focalY[0] = 0 ;
                md->focalY[1] = cmd->focalY[1] + cmd->y + mcp->y ;
                if(md->focalY[1] >= child->wndDepth)
                    md->focalY[0] = child->wndDepth-1 ;
            }
        }
        menuRenderChildMain(md,mcp,0) ;
    }
    else if(!(mp->iflags & MFI_NOTEXT))
    {
        meScheme  scheme;             /* Colour information */
        meScheme *colp;               /* Colour pointer */
        meUByte    *txtp;               /* Text pointer */
        meUByte    *p;                  /* Local text pointer */
        meUByte     fill;               /* The fill char */
        meUByte     chkStr[6] ;         /* The checkbox string */
        meShort     maxx ;              /* Length of the string */
        meShort     curx ;
        
        /* Get the colour of the item */
        scheme = mp->scheme ;
        /* if the menu item is selected then use the hilight scheme */
        if(offset == md->curContext)
        {
            if(!(flags & osdRENDITEM_KEEPCUR))
                md->curContext = -1 ;
            else if(!(mp->flags & MF_NHILIGHT))
                scheme += meSCHEME_SELECT ;
        }
        
        if(mp->flags & MF_LINEFILL)
        {
            if(mp->flags & MF_HORZADD)
                fill = boxChars [BCNS] ;
            else
                fill = boxChars [BCEW] ;
        }
        else
            fill = ' ' ;
        
        /* Get the screen position and length of the item */
        colp = md->drawnSchm + (mcp->y*md->width) + mcp->x ;
        txtp = md->drawnText + (mcp->y*md->width) + mcp->x ;
        maxx = mcp->width;
        curx = 0 ;
        
        if((offset == md->curContext) ||
           ((md->dialog->focalItem == mp->item) && (md->curContext < 0)))
        {
            /* Set the focal point */
            md->focalX[0] = mcp->x ;
            md->focalX[1] = mcp->x + maxx - 1 ;
            md->focalY[0] = mcp->y ;
            md->focalY[1] = mcp->y + mcp->depth - 1 ;
            if(md->curContext < 0)
                md->dialog->focalItem = 0 ;
        }
        /* Copy in the default colpair right across, only the hilight char need to be changed */
        {
            meUShort *cp=colp ;
            int ll, ii=mcp->depth ;
            while(--ii >= 0)
            {
                ll=maxx ;
                while(--ll >= 0)
                    *cp++ = scheme ;
                cp += md->width-maxx ;
            }
        }
        if((mp->flags & (MF_CENTER|MF_RIGHT)) && (maxx > mcp->awidth))
        {
            /* If center of right aligned then move the text across */
            int offset = maxx - mcp->awidth ;
            if(mp->flags & MF_CENTER)
                offset >>= 1 ;
            curx += offset ;
            while(offset-- > 0)
                *txtp++ = fill ;
        }
        if(mp->flags & MF_CHECK)
        {
            meUByte *bs=(meUByte *) mp->strData ;
            int len, ii ;
            
            for(ii=0,len=0 ; ii<6 ; ii++)
            {
                if((bs[ii] != '\0') &&
                   ((ii != 2) || !(mcp->mcflags & CF_SET)) &&
                   ((ii != 3) ||  (mcp->mcflags & CF_SET)))
                    chkStr[len++] = bs[ii] ;
            }
            if(mp->flags & MF_PREPND)
            {
                meUByte *ss=chkStr ;
                curx += len ;
                while(--len >= 0)
                    *txtp++ = *ss++ ;
                chkStr[0] = '\0' ;
            }
            else
            {
                chkStr[len] = '\0' ;
                maxx -= len ;
            }
        }
        else if((mp->flags & MF_DISPTYPE) && (mp->flags & MF_SUBMNU))
            maxx -= 2 ;
        else if(mp->flags & MF_BUTTON)
        {
            *txtp++ = windowChars[WCOSDBTTOPN] ;
            curx++ ;
            maxx-- ;
        }
        /* Render the string */
        if(mp->flags & MF_ENTRY)
        {
            if(menuExecute (mp,0,1) == meTRUE)
            {
                osdRenderEntry(txtp,resultStr,md->flags,0,
                               (int) md->width,mcp->awidth,mcp->adepth,NULL) ;
                txtp += mcp->awidth ;
                curx += mcp->awidth ;
            }
        }
        else if(!(mp->iflags & MFI_NOTEXT))
        {
            meShort sttx = curx ;
            meShort cury = mcp->adepth ;
            meUByte cc;                        /* Current character */
            p = (meUByte *) mp->strData ;
            if(mp->flags & MF_CHECK)
                p += 6 ;
            for(;;)
            {
                if((cc = *p++) == '\0')
                    break ;
                if((cc == meNLCHAR) && !(md->flags & RF_NEWLINE))
                {
                    if(--cury <= 0)
                        break ;
                    txtp += md->width + sttx - curx ;
                    colp += md->width ;
                    curx = sttx ;
                }
                else if(curx < maxx)
                {
                    if((cc == '&') && !(md->flags & (RF_HOTKEY|RF_FRSTLET)))
                    {
                        colp[curx] = scheme+meSCHEME_CURRENT ;
                        cc = *p++ ;
                    }
                    else if((cc == '\\') && !(md->flags & (RF_HOTKEY|RF_FRSTLET)))
                        cc = *p++ ;
                    *txtp++ = cc ;
                    curx++ ;
                }
            }
        }
        if(mp->flags & MF_BUTTON)
        {   
            *txtp++ = windowChars[WCOSDBTTCLS] ;
            curx++ ;
            maxx++ ;
        }
        
        /* Fill in to the end of the line */
        while(curx++ < maxx)
            *txtp++ = fill ;
        
        if((mp->flags & MF_CHECK) && (chkStr[0] != '\0'))
        {
            meUByte cc, *ss=chkStr ;
            while((cc = *ss++) != '\0')
                *txtp++ = cc ;
        }
        else if((mp->flags & MF_DISPTYPE) && (mp->flags & MF_SUBMNU))
        {
            /* Add menu annotaion */
            if (mp->flags & MF_MANUAL)
            {
                *txtp++ = ' ';
                *txtp = '>';
            }
            else
            {
                *txtp++ = '.';
                *txtp = '.';
            }
        }
    }
    /* Draw the text region to screen if required */
    if (flags & osdRENDITEM_DRAW)
        osdDisplaySnapshotDraw(md,mcp->x,mcp->y,mcp->width,mcp->depth,1) ;
}

void
osdDisp(meUByte *buf, meUByte *cont, int cpos)
{
    osdDISPLAY    *md=osdCurChild ;
    osdCONTEXT    *mcp;                    /* Menu context pointer */
    osdITEM       *mp;                     /* Menu pointer */
    meUByte         *txtp;                   /* Text pointer */
    int            xx, yy, ww, dd ;
    int            col, row ;
    
    /* The following code is used to try to solve a particularly nasty bug.
     * If the screen is updated from scratch then all the currently displayed
     * dialogs are reconfigured and redrawn. The problem with this is that
     * the dialog initialisation command may destroy the dialog and rebuild
     * it, thus losing the current context and if the user is currently typing
     * texting into an osd entry then BANG as theres no current context.
     * 
     * To reproduce create a callback macro which updates the screen, e.g. the
     * mail-checker, Open the organizer and start typing in any entry. When the
     * callback macro executes everything appears okay except the current entry
     * is nolonger hilighted as current. Now type a key and without the following
     * code - BANG.
     * 
     * The following attempts to solve this particular and specific problem by
     * restoring the current context
     */
    static int curChildItem=0 ;
    if(md->curContext < 0)
    {
        /* we've lost the current context, try to restore, if we can't fail */
        int ii ;
        for (ii=0 ; ii < md->numContexts ; ii++)
            if(md->context[ii].menu->item == curChildItem)
            {
                md->curContext = ii ;
                break ;
            }
        if(md->curContext < 0)
            return ;
    }
    else
        /* not lost it, store the current item incase we do */
        curChildItem = md->context[md->curContext].menu->item ;
    
    mcp = &md->context[md->curContext];
    mp = mcp->menu ;
    xx = mcp->x ;
    yy = mcp->y ;
    ww = mcp->awidth ;
    dd = mcp->adepth ;
    
    if(mp->flags & (MF_CENTER|MF_RIGHT))
    {
        /* If center of right aligned then move the text across */
        int offset = mcp->width - ww ;
        if(mp->flags & MF_CENTER)
            offset >>= 1 ;
        xx += offset ;
    }
    txtp = md->drawnText + (yy*md->width) + xx ;
    
    col = osdRenderEntry(txtp,buf,md->flags,cpos,(int) md->width,ww,dd,&row) ;
    
    /* Set the focal point */
    md->focalX[0] = xx+col-1 ;
    md->focalX[1] = xx+col ;
    md->focalY[0] = yy+row ;
    md->focalY[1] = yy+row ;
    /* set the osd cursor position */
    osdRow = md->y + yy + row ;
    osdCol = md->x + xx + col ;
    
    if(md == osdCurMd)
        osdDisplaySnapshotDraw(md,mcp->x,mcp->y,mcp->width,mcp->depth,1) ;
    else
    {
        
        do {
            md = md->prev ;
            osdRow += md->y + md->context[md->curContext].y ;
            osdCol += md->x + md->context[md->curContext].x ;
            if(md->context[md->curContext].menu->flags & MF_SCRLBOX)
            {
                osdRow++ ;
                osdCol++ ;
            }
        } while (md != osdCurMd) ;
        menuRenderItem(md,md->curContext,osdRENDITEM_DRAW|osdRENDITEM_KEEPCUR|osdRENDITEM_REFOCUS) ;
    }
    TTmove(osdRow,osdCol) ;
}                
                
/*
 * menuRenderLine
 * Render a line of the menu with borders.
 */
static void
menuRenderLine(osdDISPLAY *md, int x, int y, meScheme scheme,
               int drawBoarder, int len, int type)
{
    meUShort *colp ;
    meUByte  *txtp ;
    meUByte   cc ;

    colp = md->drawnSchm + (y*md->width) + x ;
    txtp = md->drawnText + (y*md->width) + x ;
    
    /* Add the lefthand edge - if x > 1 then this is a 2nd+ column */
    cc = '\0' ;
    if(x > 1)
    {
        if(type == 2)
            cc = boxChars[BCESW] ;
        else if(type == 3)
            cc = boxChars[BCNEW] ;
        else if(type == 1)
        {
            if(txtp[-1] == boxChars[BCNS])
                cc = boxChars[BCNES] ;
            else
                cc = boxChars[BCNESW] ;
        }
        else if(!drawBoarder)
            cc = boxChars[BCNS] ;
    }
    else if(type == 4)
        cc = windowChars[WCOSDTTLBR] ;
    else if(drawBoarder)
    {
        if(type == 3)
            cc = boxChars[BCNE] ;
        else if(type == 2)
            cc = boxChars[BCES] ;
        else if(type == 1)
            cc = boxChars[BCNES] ;
        else
            cc = boxChars[BCNS] ;
    }
    if(cc != '\0')
    {
        colp[-1] = scheme;
        txtp[-1] = cc ;
    }
    /* Fill in the remainer of the line */
    if(type == 4)
        cc = windowChars[WCOSDTTLBR] ;
    else if(type == 0)
        cc = ' ' ;
    else
        cc = boxChars [BCEW] ;
    while (--len >= 0)
    {
        *colp++ = scheme ;
        *txtp++ = cc ;
    }

    /* Add the righthand character */
    if((type == 4) || drawBoarder)
    {
        *colp = scheme;
        if(type == 4)
            cc = windowChars[WCOSDTTLKLL] ;
        else if(type == 3)
            cc = boxChars[BCNW] ;
        else if(type == 2)
            cc = boxChars[BCSW] ;
        else if(type == 1)
            cc = boxChars[BCNSW] ;
        else
            cc = boxChars[BCNS] ;
        *txtp = cc ;
    }
}

static void
menuRenderItemGrid(osdDISPLAY *md, int x, int y, int width, int depth)
{
    meUByte  *txtp, *tt ;
    meUByte   cc ;
    int ii ;
    
    txtp = md->drawnText + (y*md->width) + x ;
    tt = txtp-md->width-1 ;
    cc = *tt ;
    if(cc == ' ')
        *tt++ = boxChars[BCES] ;
    else if(cc == boxChars[BCNE])
        *tt++ = boxChars[BCNES] ;
    else if((cc == boxChars[BCEW]) ||
            (cc == boxChars[BCESW]) ||
            (cc == boxChars[BCSW]))
        *tt++ = boxChars[BCESW] ;
    else
        *tt++ = boxChars[BCNESW] ;
    for(ii=0 ; ii<width ; ii++)
    {
        cc = *tt ;
        if((cc == ' ') || (cc == boxChars[BCEW]))
            *tt++ = boxChars[BCEW] ;
        else
            *tt++ = boxChars[BCNEW] ;
    }
    cc = *tt ;
    if(cc == ' ')
        *tt = boxChars[BCSW] ;
    else if(cc == boxChars[BCEW])
        *tt = boxChars[BCESW] ;
    else if(cc == boxChars[BCNW])
        *tt = boxChars[BCNSW] ;
    else
        *tt = boxChars[BCNESW] ;
    tt = txtp+width ;
    for(ii=1 ; ii<depth ; ii++)
    {
        *tt = boxChars[BCNS] ;
        tt += md->width ;
    }
    tt = txtp-1 ;
    for(ii=1 ; ii<depth ; ii++)
    {
        *tt = boxChars[BCNS] ;
        tt += md->width ;
    }
    if(*tt == ' ')
        *tt++ = boxChars[BCNE] ;
    else
        *tt++ = boxChars[BCNEW] ;
    for(ii=0 ; ii<width ; ii++)
        *tt++ = boxChars[BCEW] ;
    *tt = boxChars[BCNW] ;
}

static void
menuRenderCorrectNoteBookPage(osdDISPLAY *md, osdCONTEXT *mcp)
{
    meUByte  *tt ;
    meUByte   cc ;
    int ii ;
    
    tt = md->drawnText + (mcp->y*md->width) + mcp->x ;
    if(mcp->mcflags & CF_NBPTOP)
    {
        tt += md->width ;
        cc = tt[-1] ;
        if(cc == boxChars[BCNEW])
            tt[-1] = boxChars[BCNW] ;
        else
            tt[-1] = boxChars[BCNS] ;
        cc = tt[mcp->width] ;
        if(cc == boxChars[BCNEW])
            tt[mcp->width] = boxChars[BCNE] ;
        else
            tt[mcp->width] = boxChars[BCNS] ;
    }
    else
    {
        tt -= md->width ;
        cc = tt[-1] ;
        if(cc == boxChars[BCESW])
            tt[-1] = boxChars[BCSW] ;
        else
            tt[-1] = boxChars[BCNS] ;
        cc = tt[mcp->width] ;
        if(cc == boxChars[BCESW])
            tt[mcp->width] = boxChars[BCES] ;
        else
            tt[mcp->width] = boxChars[BCNS] ;
    }
    for(ii=0 ; ii<mcp->width ; ii++)
        *tt++ = ' ' ;
}

/*
 * menuRender
 * Render a menu
 * 
 * NOTE:- This assumes the menu has just been constructed
 * Calling this function a second time without setting the 
 * contexts may go wrong.
 */

static void
menuRender (osdDISPLAY *md)
{
    int col;                            /* The column number */
    int row;                            /* The row number */
    int itemNo;                         /* The current item number */
    int xstart;                         /* The start position in X */
    int xpos;                           /* The curr position in X */
    int noRows;                         /* The no. rows in a depth */
    osdCONTEXT *mcp;                    /* Menu context pointer */
    int colWidth;                       /* Width of the column */
    int drawBoarder ;                   /* Do we draw a boarder? */
    int drawGrid ;                      /* Do we draw in a grid? */
    meUShort scheme;
    osdCONTEXT *nbmcp=NULL;             /* Menu context pointer */
    
    scheme = md->dialog->mScheme ;
    
    /* Initialise the variable of the menu */
    mcp = md->context ;
    xstart = 0 ;
    noRows = md->depth ;
    if((drawBoarder = md->flags & RF_BOARDER) != 0)
    {
        /* Increment xstart so that its the start of the actual item
         * the menuRenderLine allows for this
         * Also reduce the number of rows be one to leave room for
         * the bottom boarder
         */
        xstart++ ;
        noRows-- ;
    }
    drawGrid = md->flags & RF_GRID ;
    itemNo = 0 ;
    for (col = 0; col <= md->multi; col++)
    {
        /* Note that the lengths of every item is the 
         * full colWidth regardless of whether the next
         * is to be horizontally packed. The length is
         * re-adjusted below
         */
        colWidth = mcp[itemNo].width;
        row = 0 ;
        if(md->flags & RF_TITLE)
        {
            /* Render the title bar at the end */
            row++ ;
        }
        else if(drawBoarder)
        {
            /* Render the top border */
            menuRenderLine (md,xstart,row++,scheme,
                            drawBoarder, colWidth, 2) ;
        }
        if(drawGrid)
            menuRenderLine (md,xstart,row++,scheme,
                            drawBoarder,colWidth,0) ;
        /* Render each of the lines in turn */
        for (; row < noRows; )
        {
            int depth ;
            /* Render a blank line */
            if((itemNo >= md->numContexts) || 
               ((depth=mcp[itemNo].depth),((row+depth) > noRows)))
            {
                menuRenderLine (md, xstart, row, scheme,
                                drawBoarder, colWidth, 0) ;
                row++ ;
            }
            else
            {
                osdITEM *mp;
                int      ii, width, tin, spare, fillcent ;
                
                
                /* If the item is to be LINEFILLed then render whole line
                 * with '-'s else with ' '
                 */
                if(mcp[itemNo].menu->flags & MF_LINEFILL)
                    tin = 1 ;
                else
                    tin = 0 ;
                for(ii=0 ; ii<depth ; ii++)
                    menuRenderLine (md,xstart,row+ii,scheme,
                                    drawBoarder,colWidth,tin) ;
                xpos = xstart ;
                /* Get the actual length of the line */
                tin=itemNo ;
                width=0 ;
                fillcent=0 ;
                do {
                    mp = mcp[tin].menu;      /* Menu item pointer */
                    width += mcp[tin].awidth ;
                    if(drawGrid)
                        width++ ;
                    if((mp->flags & MF_CENTER) || !(mp->flags & MF_NOFILL))
                        fillcent++ ;
                } while(mcp[tin++].mcflags & CF_HORZADD) ;
                
                /* calc the spare chars */
                spare = colWidth - width ;
                if(drawGrid)
                    spare-- ;
                /* If this is a split then use all the spare to push 
                 * this line to the right
                 */
                if(mcp[itemNo].mcflags & CF_HORZCUT)
                {
                    xpos += spare ;
                    spare = 0 ;
                }
                do {
                    int ll, ss ;
                    
                    mp = mcp[itemNo].menu;      /* Menu item pointer */
                    /* restore the depth from being the row to just this item */ 
                    mcp[itemNo].depth = mcp[itemNo].adepth ;
                    /* calc the length of this item */
                    ll = mcp[itemNo].awidth ;
                    /* is this the current notebook context? if so store it
                     * as we must correct the rendering  later */
                    if(!(mp->flags & MF_CHILD) &&
                       (md->nbpContext != -1) && 
                       (mcp[itemNo].menu->argc == mcp[md->nbpContext].menu->argc))
                        nbmcp = mcp+itemNo ;
                    /* calc any spare's this item uses */
                    if((mp->flags & MF_CENTER) || !(mp->flags & MF_NOFILL))
                        ss = spare / fillcent-- ;
                    else if(!fillcent && (mp->flags & MF_RIGHT))
                        ss = spare ;
                    else
                        ss = 0 ;
                    spare -= ss ;
                    
                    /* if this is a filled item then the spare is added to the length
                     * else we must position it correctly */
                    if(mp->flags & (MF_CHILD|MF_NOFILL))
                    {
                        int s2 ;
                        if(mp->flags & MF_CENTER)
                            s2 = ss >> 1 ;
                        else if(mp->flags & MF_RIGHT)
                            s2 = ss ;
                        else
                            s2 = 0 ;
                        xpos += s2 ;
                        ss -= s2 ;
                        mcp[itemNo].width = ll ;
                    }
                    else
                        mcp[itemNo].width = ll + ss ;
                    
                    if(drawGrid)
                    {
                        xpos++ ;
                        if(mp->flags & MF_GRID)
                            menuRenderItemGrid(md,xpos,row,mcp[itemNo].width,depth) ;
                    }
                    /* Get the menu and store the x and y positions */
                    mcp[itemNo].x = xpos;       /* Start position in x */
                    mcp[itemNo].y = row;
                    
                    /* Evaluate the State */
                    if (mp->flags & MF_CHECK)
                    {
                        if (menuExecute (mp,md->flags,1) == meTRUE)
                            mcp[itemNo].mcflags |= CF_SET;   /* State is set */
                        else
                            mcp[itemNo].mcflags &= ~CF_SET;   /* State is not set */
                    }
                    if((mp->flags & (MF_SCRLBOX|MF_CHILD)) == (MF_SCRLBOX|MF_CHILD))
                        menuRenderChildBoarder(md,&mcp[itemNo]) ;
                    menuRenderItem(md, itemNo, osdRENDITEM_KEEPCUR|osdRENDITEM_REDWALL|osdRENDITEM_FORCE|osdRENDITEM_REFOCUS);
                    xpos += ll + ss ;
                } while(mcp[itemNo++].mcflags & CF_HORZADD) ;
                row += depth ;
            }
        }
        if(drawBoarder)
        {
            /* Render the bottom border */
            menuRenderLine (md, xstart, row, scheme,
                            drawBoarder, colWidth, 3) ;
        }
        xstart += colWidth+1 ;
    }
    if(nbmcp != NULL)
        menuRenderCorrectNoteBookPage(md,nbmcp) ;
    if(md->flags & RF_TITLE)
    {
        meUByte *ss ;
        
        /* If the dialog has a resize command then draw the bottom '*' */
        if(md->flags & RF_RESIZE)
            md->drawnText[(row*md->width)+xstart-1] = windowChars[WCOSDRESIZE] ;
        scheme = md->dialog->tScheme ;
        colWidth = md->width-2 ;
        
        /* Render the title bar */
        menuRenderLine (md,1,0,scheme,1,colWidth,4) ;
        if((ss = md->dialog->strData) != NULL)
        {
            meUByte *txtp ;
            int   ll ;
            
            /* We know the color is correct so just splat in the text */
            txtp = md->drawnText ;
            ll = meStrlen(ss) ;
            if(ll > colWidth)
                ll = colWidth ;
            else if(md->flags & RF_CENTER)
                txtp += (colWidth-ll) >> 1 ;
            else if(md->flags & RF_RIGHT)
                txtp += colWidth-ll ;
            while(ll--)
                *txtp++ = *ss++ ;
        }
    }
}


/*
 * osdDisplayPop
 * Pop the display frame off the stack
 */
static void
osdDisplayPop (osdDISPLAY *md)
{
    osdDISPLAY *tmd, *pmd ;             /* Temporary display frame */

    if (md == NULL)                     /* Nothing to do ?? */
        return;                         /* Nope - quit */
    
    /* unlink md */
    if (md->prev == NULL)
        osdDisplayHd = NULL;                /* Root of the list */
    else
        md->prev->next = NULL;          /* Terminate end of list */
    
    /* Advance to the end of the list */
    for (tmd = md; tmd->next != NULL; tmd = tmd->next)
        /* NULL */;
    
    /* restore the snapshots & free off backwards */
    for(;;)
    {
        osdDisplaySnapshotRestore(tmd) ;
        menuRenderArea(tmd->x,tmd->y,tmd->width,tmd->depth) ;
        pmd = tmd->prev ;
        displayDestruct(tmd) ;
        if(tmd == md)
            break ;
        tmd = pmd ;
    }
}


/*
 * menuConfigure
 * Compute the layout of the dialog
 */
static osdDISPLAY *
menuConfigure(osdDIALOG *rp, osdDISPLAY *md, int child)
{
    osdCONTEXT *mcp;                    /* Menu context pointer */
    osdITEM *mp;                        /* Menu pointer */
    meShort curItem=-1,newItem=-1;        /* Display's curContext & newContext item # */
    int minWidth, maxWidth;             /* The min/max allowed width */
    int minDepth, maxDepth;             /* The min/max allowed depth */
    int ii,jj;                          /* Local loop counter */
    
    /* Get the length of the dialog & Determine if there is any work to perform */
    for (ii=0, mp = rp->itemHead; mp != NULL; mp = mp->next)
        if (!(mp->flags & MF_DISABLE))
            ii++;
    if(ii == 0)
    {
        mlwrite(MWABORT|MWPAUSE,(meUByte *)"[No active items in osd dialog %d]", rp->id);
        return NULL;
    }
    if(md == NULL)
    {
        if((md = meMalloc(sizeof (osdDISPLAY) + (sizeof (osdCONTEXT) * (ii-1)))) == NULL)
            return NULL ;
        memset (md,0,sizeof(osdDISPLAY) + (sizeof (osdCONTEXT) * (ii - 1)));
        md->dialog = rp ;
        md->flags = rp->flags;          /* Copy the flags across */
        if(child)
            md->flags |= RF_CHILD ;
        md->displays = rp->displays ;
        rp->displays = md ;
    }
    else
    {
        if(md->curContext >= 0)
            curItem = md->context[md->curContext].menu->item ;
        if(md->newContext >= 0)
            newItem = md->context[md->newContext].menu->item ;
        if(ii > md->numContexts)
        {  
            /* realloc a larger memory block and zero */
            osdDISPLAY *nmd ;
            if((nmd = meMalloc (sizeof (osdDISPLAY) + (sizeof (osdCONTEXT) * (ii-1)))) == NULL)
                return NULL ;
            jj = sizeof(osdDISPLAY) + (sizeof (osdCONTEXT) * (md->numContexts - 1)) ;
            memcpy (nmd,md,jj) ;
            memset (((char *) nmd)+jj,0,(sizeof(osdCONTEXT) * (ii - md->numContexts)));
            /* link in the new and free off the old */
            jj = md->numContexts ;
            while(--jj >= 0)
                if((md->context[jj].child != NULL) && (md->context[jj].child->display != NULL))
                        md->context[jj].child->display->prev = nmd ;
            
            if(child)
	    {
                osdDISPLAY *tmd ;    
                tmd = md->prev ;
                jj = tmd->numContexts ;
                while(--jj >= 0)
                    if((tmd->context[jj].child != NULL) && (tmd->context[jj].child->display == md))
                        tmd->context[jj].child->display = nmd ;
            }
            else
            {
                if(nmd->prev != NULL)
                    nmd->prev->next = nmd;
                if(nmd->next != NULL)
                    nmd->next->prev = nmd;
                if(osdDisplayHd == md)
                    osdDisplayHd = nmd ;
                if(osdCurMd == md)
                    osdCurMd = nmd ;
                if(osdNewMd == md)
                    osdNewMd = nmd ;
            }
            if(osdCurChild == md)
                osdCurChild = nmd ;
            if(osdNewChild == md)
                osdNewChild = nmd ;
            if(rp->displays == md)
                rp->displays = nmd ;
            else
            {
                osdDISPLAY *tmd=rp->displays ;
                while(tmd->displays != md)
                    tmd = tmd->displays ;
                tmd->displays = nmd ;
            }
            free(md) ;
            md = nmd ;
        }
/*        md->focalX[0] = md->focalX[1] = 0 ;*/
/*        md->focalY[0] = md->focalY[1] = 0 ;*/
        md->flags &= ~RF_REDRAW ;   /* Remove the internal redraw flag */
    }
    /* Set the CONFIG flag to stop dialogResetDisplays messing with this */
    md->flags |= RF_CONFIG ;
    md->numContexts = ii ;          /* Number of items to render */
    md->newContext = -1 ;
    md->curContext = -1 ;
    md->multi = 0 ;                 /* Reset column count */
    md->width = 0 ;                 /* Reset the width of the menu */
    
    /* Setup the min/maximum allowable width & depth allowing for the boarder title and grid */
    if((maxWidth = rp->width[1]) == 0)
        maxWidth = frameCur->width ;
    else if(maxWidth < 0)
        maxWidth = 65535 ;
    if((minWidth = rp->width[0]) < 0)
        minWidth = maxWidth ;
    if((maxDepth = rp->depth[1]) == 0)
        maxDepth = frameCur->depth + 1 ;
    else if(maxDepth < 0)
        maxDepth = 65535 ;
    if((minDepth = rp->depth[0]) < 0)
        minDepth = maxDepth ;
    if(md->flags & RF_BOARDER)
    {
        maxWidth -= 2 ;
        minWidth -= 2 ;
        maxDepth -= 2 ;
        minDepth -= 2 ;
    }
    else if(md->flags & RF_TITLE)
    {
        maxDepth-- ;
        minDepth-- ;
    }
    if(rp->flags & RF_GRID)
    {
        maxWidth-- ;
        minWidth-- ;
        maxDepth-- ;
        minDepth-- ;
    }
    
    /* setup each contexts menu, flags and size */
    {
        osdCHILD   *child ;
        osdDISPLAY *cmd ;
        meShort width, depth ;
        int mcflags = CF_ROWFRST;           /* The mcp row flags */
        
        /* Loop through setting the context menu pointers and flags */
        for (mcp = md->context, mp = rp->itemHead; mp != NULL; mp = mp->next)
        {
            int flags ;
            /* Skip un-rendered entries */
            if(mp->flags & MF_DISABLE)
                continue;
            
            mcp->menu = mp ;                /* Point to the menu */
            width = mp->len ;
            depth = mp->height ;
            if(mp->flags & MF_CHILD)
            {
                osdDIALOG  *crp ;
                int mnw, mnd ;
                
                if((child=mcp->child) == NULL)
                {
                    if((child = meMalloc(sizeof(osdCHILD))) == NULL)
                        return NULL ;
                    child->display = NULL ;
                    child->horzSBar = NULL ;
                    child->vertSBar = NULL ;
                    mcp->child = child ;
                }
                /* the item is a child type (dialog in a dialog). Construct the child */
                /* Execute the menu constructor */
                menuExecute(mp,md->flags,-1);
                /* Find the root of the sub-menu */
                if((crp = dialogFind(mp->argc)) == NULL)
                    return NULL ;
                if(crp->cmdIndex >= 0)
                    execFunc((int) crp->cmdIndex,0,1) ;
                if((child->display != NULL) && 
                   (child->display->dialog != crp))
                {
                    /* the current child is displaying a different dialog, we must
                     * burn this one and start again */
                    displayDestruct(child->display) ;
                    child->display = NULL ;
                }
                if(mp->flags & MF_SCRLBOX)
                {
                    mnw = crp->width[0] ;
                    if(mnw == 0)
                        crp->width[0] = width ;
                    mnd = crp->depth[0] ;
                    if(mnd == 0)
                        crp->depth[0] = depth ;
                }
                if((cmd = menuConfigure(crp,child->display,1)) == NULL)
                    return NULL ;
                cmd->prev = md ;
                child->display = cmd ;
                if(mp->flags & MF_SCRLBOX)
                {
                    crp->width[0] = mnw ;
                    crp->depth[0] = mnd ;
                    flags = gsbarmode & (WMVWIDE|WMUP|WMDOWN|WMSCROL|WMRVBOX|WMHWIDE) ;
                    width += 2 ;
                    depth += 2 ;
                    if(flags & WMHWIDE)
                        depth++ ;
                    if(flags & WMVWIDE)
                        width++ ;
                }
                else
                {
                    width = cmd->width ;
                    depth = cmd->depth ;
                }
            }
            else if(mcp->child != NULL)
            {
                displayDestructChild(mcp->child) ;
                mcp->child = NULL ;
            }

            /* correct the size if to big */
            if(width > maxWidth)
            {
                width = maxWidth ;
                if(rp->flags & RF_GRID)
                    width-- ;
            }
            if(depth > maxDepth)
            {
                depth = maxDepth ;
                if(rp->flags & RF_GRID)
                    depth-- ;
            }
            mcp->awidth = width ;
            mcp->adepth = depth ;
            mcp->width = width ;
            mcp->depth = depth ;
            if(mp->flags & MF_CHILD)
            {
                if(mp->flags & MF_SCRLBOX)
                {
                    meSCROLLBAR *sb ;
                
                    width -= 2 ;
                    depth -= 2 ;
                    if(flags & WMHWIDE)
                        depth-- ;
                    if(flags & WMVWIDE)
                        width-- ;
                    if((cmd->width+cmd->x) < width)
                        cmd->x = 0 ;
                    if((cmd->depth+cmd->y) < depth)
                        cmd->y = 0 ;
                    if(cmd->width > width)
                    {
                        if((sb = initScrollBar(child->horzSBar,width,flags|WMHORIZ)) == NULL)
                            return NULL ;
                        sb->totLen = cmd->width ;
                        sb->curPos = -1 ;
                        child->horzSBar = sb ;
                    }
                    else if(child->horzSBar != NULL)
                    {
                        free(child->horzSBar) ;
                        child->horzSBar = NULL ;
                    }
                    if(cmd->depth > depth)
                    {
                        if((sb = initScrollBar(child->vertSBar,depth,flags)) == NULL)
                            return NULL ;
                        sb->totLen = cmd->depth ;
                        sb->curPos = -1 ;
                        child->vertSBar = sb ;
                    }
                    else if(child->vertSBar != NULL)
                    {
                        free(child->vertSBar) ;
                        child->vertSBar = NULL ;
                    }
                }
                else
                {
                    if((rp->flags & RF_NOTEBOOK) && (mcp != md->context))
                        mcp[-1].mcflags &= ~CF_HORZADD ;
                }
                child->wndWidth = width ;
                child->wndDepth = depth ;
                menuRender(cmd) ;
            }
            if(mp->flags & MF_HORZADD)
                mcp->mcflags = mcflags|CF_HORZADD ;
            else
            {
                mcp->mcflags = mcflags ;
                mcflags = (mcflags & CF_ROWTGGL) ^ CF_ROWTGGL ;
            }
            mcp++ ;
        }
        /* ensure the last context hasn't got a HORZADD */
        mcp[-1].mcflags &= ~CF_HORZADD ;
    }
    mcp = md->context ;
    /* setup the curContext & each contexts allowable horzadd's */
    {
        int curWidth = 0 ;                   /* width of current row */
        meShort nbpContext = -1;                     /* Current Notebook page Menu context pointer */
        
        /* Loop through setting the context menu pointers and flags */
        for (ii=0 ; ii < md->numContexts ; ii++)
        {
            /* If a display was given with a curContext then this finds the
             * new context number
             */
            if(rp->flags & RF_NOTEBOOK)
            {
                if(mcp[ii].child == NULL)
                    mcp[ii].mcflags |= (nbpContext == -1) ? CF_NBPTOP:CF_NBPBOT ;
                else
                    nbpContext = ii ;
            }
            if(mcp[ii].menu->item == curItem)
                md->curContext = ii ;
            if(mcp[ii].menu->item == newItem)
                md->newContext = ii ;
            if(mcp[ii].mcflags & CF_HORZADD)
            {
                curWidth += mcp[ii].width ;
                if(rp->flags & RF_GRID)
                    curWidth++ ;
                if((curWidth + mcp[ii+1].width) > maxWidth)
                {
                    mcp[ii].mcflags &= ~CF_HORZADD ;
                    mcp[ii+1].mcflags |= CF_HORZCUT ;
                    curWidth = 0 ;
                }
            }
            else
                curWidth = 0 ;
        }
        md->nbpContext = nbpContext ;
    }
    /* Now set each row width and depth */
    {
        int curWidth;                   /* width of current row */
        int curDepth;                   /* depth of current row */
        
        curWidth = curDepth = 0 ;
        /* Loop through setting the context menu pointers and flags */
        for (ii=jj=0 ; ii < md->numContexts ; ii++)
        {
            curWidth += mcp[ii].width ;
            if(rp->flags & RF_GRID)
                curWidth++ ;
            if(mcp[ii].depth > curDepth)
                curDepth = mcp[ii].depth ;
            if(!(mcp[ii].mcflags & CF_HORZADD))
            {
                if(mcp[jj].mcflags & CF_HORZCUT)
                    curWidth = maxWidth ;
                else if(curWidth < minWidth)
                    curWidth = minWidth ;
                if(rp->flags & RF_GRID)
                    curDepth++ ;
                for ( ; jj <= ii ; jj++)
                {
                    mcp[jj].width = curWidth ;
                    mcp[jj].depth = curDepth ;
                }
                curWidth = curDepth = 0 ;
            }
        }
    }
    
    if(maxWidth < frameCur->width)
        /* Reset the width now cos if the width was set then thats been done and
         * if the depth was  fixed as well then we may need multi-columns */
        maxWidth = frameCur->width ;
    
    /* Determine the width and depth of the menu */
    {
        int subWidth;                   /* Sub-width of menu */
        int subDepth;                   /* Sub-depth of menu */
        
        subWidth = subDepth = 0 ;
        for (ii=jj=0 ; ii < md->numContexts ; ii++)
        {
            /* Are we at the maximum depth? */
            if((subDepth+mcp[ii].depth) > maxDepth)
            {
                if(rp->flags & RF_GRID)
                {
                    subWidth++ ;
                    subDepth++ ;
                }
                /* Have we got room for another column? if not break out
                 * setting the number of contexts to the number we can fit in
                 */
                if(subWidth >= maxWidth)
                    break ;
                maxWidth -= subWidth + 1 ;
                md->multi++;                 /* Next column */
                md->depth = maxDepth;        /* Depth of column (allow for border) */
            
                /* Assign the new width to the items */
                for ( ; jj < ii ; jj++)
                    mcp[jj].width = subWidth;    /* Assign the colum width */
                md->width += subWidth + 1 ;  /* Assign new width */
                subWidth = subDepth = 0 ;
            }
            subDepth += mcp[ii].depth ;
            /* Is this the longest line so far? */
            if(subWidth < mcp[ii].width)
                subWidth = mcp[ii].width ;
            while(mcp[ii].mcflags & CF_HORZADD)
                ii++ ;
        }
        if(subWidth > maxWidth)
        {
            /* we haven't got enough room for this row, don't render it! */
            if((--md->multi) < 0)
                return NULL ;
            md->width-- ;
        }
        else
        {
            if(rp->flags & RF_GRID)
            {
                subWidth++ ;
                subDepth++ ;
            }
            /* Back fill the positions in the table */
            for ( ; jj < ii ; jj++)
                mcp[jj].width = subWidth;    /* Assign the colum width */
            
            /* If not multi-column assign the new depth */
            if (md->multi == 0)
            {
                if(subDepth < minDepth)
                {
                    subDepth = minDepth ;
                    if(rp->flags & RF_GRID)
                        subDepth++ ;
                }
                md->depth = subDepth;
            }
            md->width += subWidth ;              /* Assign the new width */
        }
        /* any items that we have prepared but can't be rendered must be freed */
        for(ii=jj ; ii<md->numContexts ; ii++)
            if(md->context[ii].child != NULL)
                displayDestructChild(md->context[ii].child) ;
        
        md->numContexts = jj ;   /* Number of items to render */
    }
    /* Finish up, assign border widths and terminate list */
    if(md->flags & RF_BOARDER)
    {
        md->width += 2 ;                /* Add for border. */
        md->depth += 2 ;                /* Add for border. */
    }
    else if(md->flags & RF_TITLE)
        md->depth++ ;                   /* Add for title bar */
    
    /* ZZZZZ - some freeing off required! */
    if(!osdDisplaySnapshotCreate(md))
        return NULL ;
    /* safe to let dialogResetDisplays function again */
    md->flags &= ~RF_CONFIG ;
    return md ;
}


static void
menuPosition(osdDISPLAY *md, int redraw)
{
    if(!redraw || 
       (!(md->flags & RF_TITLE) &&
        (md->prev != NULL) && !(md->prev->flags & RF_DISABLE)))
    {
        if(md->flags & RF_ABSPOS)
        {
            /* If this is an absolute position then set the requested position
             * Not on a redraw, on redraws the correct position is already set
             */
            md->x = md->dialog->x ;
            md->y = md->dialog->y ;
        }
        else
        {
            /* Position relative to the menu.
             * Cases for positioning a sub-menu are:-
             *
             * Main menu as parent
             * -------------------
             *
             * ============XXX============================
             *            +----+
             *            |    |
             *            |    |
             *            +----+
             *
             * SubMenu as parent, right below
             *
             *       +----+
             *       |    +----+
             *       |   >|    |
             *       |    |    |
             *       +----|    |
             *            +----+
             *
             * Submenu as parent, right above
             * Submenu as parent, left below
             * Submenu as parent, left above
             */
            osdDISPLAY *pmd ;              /* Previous menu display */
            osdCONTEXT *mcp ;              /* Menu context pointer */
            int flags ;
            
            /* Get the pointer to the previous menu */
            if (((pmd = md->prev) != NULL) &&
                !(pmd->flags & RF_DISABLE))
            {
                md->x = 0 ;
                md->y = 0 ;
                while(pmd->curContext >= 0)
                {
                    mcp = &pmd->context[pmd->curContext] ;
                    md->x += pmd->x + mcp->x ;
                    md->y += pmd->y + mcp->y ;
                    if(mcp->menu->flags & MF_SCRLBOX)
                    {
                        md->x++ ;
                        md->y++ ;
                    }
                    if(!((flags=mcp->menu->flags) & MF_CHILD))
                        break ;
                    pmd = mcp->child->display ;
                }
                if(pmd->curContext < 0)
                    mcp = NULL ;
            }
            else
                mcp = NULL;
            
            /* Previous menu not present */
            if (mcp == NULL)
            {
#if MEOPT_MOUSE
                md->x = mouse_X;                /* Position at the mouse */
                md->y = mouse_Y;
#else
                md->x = 0 ;
                md->y = 0 ;
#endif
            }
            else
            {
                /* From the previous menu item work out where the
                 * menu should be placed
                 */
                if(flags & MF_NORTH)
                    md->y -= md->depth ;
                else if(flags & MF_SOUTH)
                    md->y += mcp->depth ;
                else if(flags & MF_WEST)
                    md->x -= md->width ;
                else
                    md->x += mcp->width ;
                if(md->flags & RF_BOARDER)
                {
                    if(flags & (MF_NORTH|MF_SOUTH))
                        md->x -= 1 ;
                    else
                        md->y -= 1 ;
                }
            }
            if(md->flags & RF_OFFSTPOS)
            {
                /* If there's an offset position add this to the calc position */
                md->x += md->dialog->x ;
                md->y += md->dialog->y ;
            }
        }
    }
    /* Check the proposed position is okay, if not, adjust */
    if(redraw && (md->flags & RF_TITLE))
    {
        /* On a redraw check that at least something is showing */
        if ((md->x + md->width) <= 0)
            md->x = 1-md->width ;
        else if (md->x >= (meShort) frameCur->width)
            md->x = frameCur->width - 1 ;
        if ((md->y + md->depth) <= 0)
            md->y = 1-md->depth ;
        else if (md->y > (meShort) frameCur->depth)
            md->y = frameCur->depth ;
    }
    else
    {
        /* When first drawn show as much as possible */
        if (md->x < 0)
            md->x = 0 ;
        else if ((md->x + md->width) > frameCur->width)
            md->x = frameCur->width - md->width;
        if (md->y < 0)
            md->y = 0 ;
        else if ((md->y + md->depth) > frameCur->depth+1)
            md->y = frameCur->depth + 1 - md->depth ;
    }
}
            
/*
 * osdDisplayFindNext
 * Find the next item in the display list.
 */
static int
osdDisplayFindNext (osdDISPLAY *md, int curCont, int dir, int canLoop, int depth)
{
    int ii, ei, si, jj, rowm, mn, mx ;
    
    /* if non are selected yet then select the first or last */
    if ((curCont < 0) || (depth < 0))
    {
        /* quick case - just do it */
        if (dir & (MF_EAST|MF_SOUTH))            /* Forwards */
        {
            for (ii = 0 ; ii < md->numContexts ; ii++)
                if (!(md->context [ii].menu->flags & MF_SEP))
                    return ii;
        }
        else
        {
            for (ii = md->numContexts-1 ; ii >= 0 ; ii--)
                if (!(md->context[ii].menu->flags & MF_SEP))
                    return ii;
        }
        return -1 ;
    }
    ii = curCont;
    /* First thing to do is to get the row mask */
    rowm = md->context[ii].mcflags & CF_ROWMASK ;
    /* Now get the start and end context no for the row */
    for (si=ii ; (si >= 0) && ((md->context[si].mcflags & CF_ROWMASK) == rowm) ; si--)
        ;
    si++ ;
    for (ei=ii ; (ei < md->numContexts) && ((md->context[ei].mcflags & CF_ROWMASK) == rowm)  ; ei++)
        ;
    ei-- ;
    /* moving north or south is trivial as its simply the next item after
     * the end of the current row
     */
    if(dir == MF_SOUTH)
    {
        depth += md->context[ii].y ;
        mn = md->context[ii].x ;
        mx = mn + md->context[ii].width ;
        for(;;)
        {
            if(++ei >= md->numContexts)
            {
                if(!canLoop)
                    return ii ;
                depth = 0 ;
                ei = 0 ;
            }
            if(ei == si)
                return ii ;
            if(!(md->context[ei].menu->flags & MF_SEP) &&
               (md->context[ei].y >= depth) &&
               (md->context[ei].x < mx) &&
               ((md->context[ei].x+md->context[ei].width) > mn))
                return ei ;
        }
    }
    else if(dir == MF_NORTH)
    {
        depth = md->context[ii].y - depth ;
        mn = md->context[ii].x ;
        mx = mn + md->context[ii].width ;
        for(;;)
        {
#if 1
            /* The gcc based compilers seems to be a bit broken here
             * I've no idea why the version below breaks it, but
             * it does, so do it this way instead.
             */
            if(si == 0)
            {
                if(!canLoop)
                    return ii ;
                depth = 0x7fffffff ;
                si = md->numContexts-1 ;
            }
            else
                si-- ;
#else
            if(si == 0)
            {
                if(!canLoop)
                    return ii ;
                depth = 0x7fffffff ;
                si = md->numContexts ;
            }
            si-- ;
#endif
            if(si == ei)
                return ii ;
            if(!(md->context[si].menu->flags & MF_SEP) &&
               (md->context[si].y <= depth) &&
               (md->context[si].x < mx) &&
               ((md->context[si].x+md->context[si].width) > mn))
                return si ;
        }
    }
    /* moving East or West is more difficult as multi-column dialogs mean that
     * we are looking for any context which is in the current rows y range */
    /* First thing to do is to find the y span */
    mn = md->context[si].y ;
    mx = mn + md->context[si].depth ;
    for (jj=si+1 ; jj <= ei ; jj++)
    {
        if(mn > md->context[jj].y)
            mn = md->context[jj].y ;
        if(mx < (md->context[jj].y+md->context[jj].depth))
            mx = md->context[jj].y+md->context[jj].depth ;
    }
    jj = ii ;
    if(dir == MF_EAST)
    {
        for(;;)
        {
            if(++jj >= md->numContexts)
            {
                if(!canLoop)
                    return ii ;
                jj = 0 ;
            }
            if((jj == ii) ||
               (!(md->context[jj].menu->flags & MF_SEP) &&
                (md->context[jj].y >= mn) &&
                ((md->context[jj].y+md->context[jj].depth) <= mx)))
                break ;
        }
    }
    else
    {
        for(;;)
        {
#if 1
            /* The gcc based compilers seems to be a bit broken here
             * I've no idea why the version below breaks it, but
             * it does, so do it this way instead.
             */
            if(jj == 0)
            {
                if(!canLoop)
                    return ii ;
                jj = md->numContexts-1 ;
            }
            else
                jj-- ;
#else
            if(jj == 0)
                jj = md->numContexts ;
            jj-- ;
#endif
            if((jj == ii) ||
               (!(md->context[jj].menu->flags & MF_SEP) &&
                (md->context[jj].y >= mn) &&
                ((md->context[jj].y+md->context[si].depth) <= mx)))
                break ;
        }
    }
    return jj ;
}


#define QUIT_MENU     1                 /* Iterate the menu */
#define MOUSE_MENU    2                 /* Mouse was used to execute */
#define EXECUTE_MENU  4                 /* Execute the menu item */
#define OPEN_MENU     8                 /* Open the sub-menu */
#define FOCUS_MENU   16                 /* Make this the current menu */
#define ENTER_MENU   32                 /* Go into the sub-menu */

static void
osdDisplaySub(osdDISPLAY *md, int flags) ;

static osdDISPLAY *
osdDisplayPush(int id, int flags)
{
    osdDISPLAY *md;                       /* Menu display item */
    osdDISPLAY *lmd, *nmd;                /* Last/next menu display item */
    osdDIALOG  *rp;                       /* Pointer to the root menu */
    int defItem ;
    
    defItem = (id & 0xffff0000) >> 16 ;
    id &= 0x0ffff ;
    if ((rp = dialogFind (id)) == NULL)   /* Find the root of the menu */
    {
        mlwrite(MWABORT|MWPAUSE,(meUByte *)"[Cannot find osd object identity %d]", id);
        return NULL;
    }
    /* Execute the initialization command if present */
    if(rp->cmdIndex >= 0)
       execFunc((int) rp->cmdIndex,0,1) ;
    
    /* Construct the menu */
    if((md=menuConfigure(rp,NULL,0)) == NULL)
        return NULL ;
    
    /* Get the last menu display item */
    for (lmd = NULL, nmd = osdDisplayHd; nmd != NULL; nmd = nmd->next)
        lmd = nmd ;
    
    /* Link into the menu */
    if ((md->prev = lmd) == NULL)
        osdDisplayHd = md;
    else
        lmd->next = md;
    
    /* Position the menu */
    menuPosition(md,0);
        
    if(flags & FOCUS_MENU)
    {
        /* Make this the current display */
        osdCurMd = md ;
        osdCurChild = md ;
        
        /* If theres a default selection then set it */
        if((defItem) || (rp->flags & RF_DEFAULT))
        {
            int ii ;
            if(!defItem)
                defItem = rp->defItem ;
            for(ii=0 ; ii<md->numContexts ; ii++)
            {
                if((md->context[ii].menu->item == defItem) &&
                   !(md->context[ii].menu->flags & MF_SEP))
                {
                    md->curContext = ii ;
                    break ;
                }
            }
        }
        else if((flags & ENTER_MENU) && (md->curContext < 0))
            md->curContext = osdDisplayFindNext(md, -1, MF_EAST, 1, 0);
    }
    /* Snapshot the region occupied by the menu. */
    osdDisplaySnapshotStore(md) ;
    menuRender(md) ;
    osdDisplaySnapshotDraw(md,0,0,md->width,md->depth,1) ;
    
    if(md->curContext >= 0)
        /* open the sub-menu if needed */
        osdDisplaySub(md,0) ;
    
    return md ;
}
    

void
osdStoreAll(void)
{
    osdDISPLAY *tmd ;                 /* Temporary display frame */
    
    if((tmd=osdDisplayHd) == NULL)
        return ;
    
    /* Advance to the end of the list */
    for ( ; tmd->next != NULL; tmd=tmd->next)
        /* NULL */;
    
    /* restore the snapshots backwards */
    while(tmd != NULL)
    {
        /* Snapshot the region occupied by the menu. */
        osdDisplaySnapshotRestore(tmd) ;
        tmd = tmd->prev ;
    }
}

void
osdRestoreAll(int garb)
{
    meUByte oldAllKeys=TTallKeys ;
    osdDISPLAY *tmd ;                 /* Temporary display frame */
    
    if(!TTallKeys)
        TTallKeys = 1 ;
    /* Now redraw all the osdDisplayHds back again */
    tmd = osdDisplayHd ;
    while(tmd != NULL)
    {
        if(tmd->flags & RF_REDRAW)
            garb = 1 ;
        if(garb)
        {
            /* reconstruct the menu - if this fails the user is in big trouble,
             * I recon this will core-dump! the good news is that this really
             * shouldn't fail!
             */
            if((tmd = menuConfigure(tmd->dialog,tmd,0)) == NULL)
                break ;
            
            /* Position the menu */
            menuPosition(tmd,1);
        
            /* Snapshot the region occupied by the menu. */
            osdDisplaySnapshotStore(tmd) ;
            menuRender (tmd) ;
        }
        else
            /* Snapshot the region occupied by the menu. */
            osdDisplaySnapshotStore(tmd) ;
        
        osdDisplaySnapshotDraw(tmd,0,0,tmd->width,tmd->depth,1) ;
    
        tmd = tmd->next ;
    }
    /* Restore state */
    TTallKeys = oldAllKeys ;
}

/*
 * osdDisplayEvaluate
 * Evaluate any states in the menu
 */
static void
osdDisplayEvaluate (void)
{
    osdCONTEXT *mcp;                    /* Pointer to menu display contexts */
    int oldState;                       /* Last state of menu flags */
    int ii;                             /* Local loop counter */
    int flag=osdRENDITEM_DRAW|osdRENDITEM_KEEPCUR ;
    
    for (mcp = osdCurChild->context, ii = 0; ii < osdCurChild->numContexts; ii++, mcp++)
    {
        /* Ignore any non-state entries */
        if (mcp->menu->flags & MF_CHECK)
        {
            /* Get the status of the state */
            oldState = mcp->mcflags;          /* Save the old flags */
            if (menuExecute (mcp->menu,osdCurChild->flags, 1) == meTRUE)
                mcp->mcflags |= CF_SET;       /* State is set */
            else
                mcp->mcflags &= ~CF_SET;      /* State is not set */
            
            /* Evaluate a change. Update the screen. */
            if (((mcp->mcflags ^ oldState) & CF_SET) != 0)
            {
                if(osdCurChild != osdCurMd)
                    flag = osdRENDITEM_KEEPCUR|osdRENDITEM_REDWALL ;
                menuRenderItem(osdCurChild, ii, flag) ;
            }
        }
    }
    if(flag == (osdRENDITEM_KEEPCUR|osdRENDITEM_REDWALL))
        menuRenderItem(osdCurMd,osdCurMd->curContext,osdRENDITEM_DRAW|osdRENDITEM_KEEPCUR) ;
}

static void
osdDisplaySetNewFocus(int flags) ;

/*
 * Display a sub-menu
 */
static void
osdDisplaySub(osdDISPLAY *md, int flags)
{
    osdDISPLAY *nmd ;
    osdITEM *mp;
    int index;

    if(((index = md->curContext) == -1) ||
       ((mp = md->context [index].menu) == NULL) ||
       !(mp->flags & MF_SUBMNU) ||
       ((mp->flags & MF_MANUAL) && !(flags & OPEN_MENU)))
        return ;
    
    /* as this could be a child loop up to the parent */
    nmd = md ;
    while(nmd->flags & RF_CHILD)
        nmd = nmd->prev ;
    
    /* Check to see if its already being displayed */
    if(nmd->next == NULL)
    {
        /* No - Execute the menu constructor */
        menuExecute (mp,md->flags,-1);
        /* This may have reset this menu item, so get it again */
        
        /* Find the root of the sub-menu */
        if((dialogFind(mp->argc & 0x0ffff) == NULL) ||
           ((md = osdDisplayPush(mp->argc,flags)) == NULL))
            return ;           
    }
    else if(flags & FOCUS_MENU)
    {
        /* Make this the current display */
        osdNewMd = nmd->next ;
        osdNewChild = nmd->next ;
        osdNewMd->newContext = osdNewMd->curContext ;
        if((flags & ENTER_MENU) && (osdNewMd->newContext < 0))
            osdNewMd->newContext = osdDisplayFindNext(osdNewMd, -1, MF_EAST, 1, 0);
        osdDisplaySetNewFocus(0) ;
    }
}

/*
 * Set the selection hilighting for a menu item
 */
static void
osdDisplaySetNewFocus(int flags)
{
    osdDISPLAY *md ;
    
    if((osdNewMd == osdCurMd) &&
       (osdNewChild == osdCurChild) &&
       (osdNewChild->curContext == osdNewChild->newContext))
    {
        if((osdNewChild->newContext < 0) || !(flags & OPEN_MENU) ||
           ((osdNewMd->next != NULL) && !(flags & (ENTER_MENU|FOCUS_MENU))))
            return ;
    }
    else
    {
        osdDisplayPop(osdNewMd->next);
        
        if((osdNewMd->curContext >= 0) && 
           ((osdNewMd->curContext != osdNewMd->newContext) ||
            (osdNewChild->curContext != osdNewChild->newContext)))
        {
            int flags=osdRENDITEM_REDWALL ;
            if(osdNewMd->curContext != osdNewMd->newContext)
                flags |= osdRENDITEM_DRAW ;
            menuRenderItem(osdNewMd,osdNewMd->curContext,flags) ;
        }
        osdCurMd = osdNewMd ;
        osdCurChild = osdNewChild ;
    }
    
    if (osdNewChild->newContext >= 0)
    {
        md = osdNewMd ;
        for(;;)
        {
            md->curContext = md->newContext ;
            if(md == osdNewChild)
                break ;
            md = md->context[md->curContext].child->display ;
        }
        menuRenderItem(osdNewMd,osdNewMd->curContext,
                       osdRENDITEM_DRAW|osdRENDITEM_KEEPCUR|osdRENDITEM_REDWALL|osdRENDITEM_REFOCUS) ;
        /* do we need to open a sub-menu */
        osdDisplaySub(osdNewChild,flags) ;
    }
    else
        osdNewMd->curContext = -1 ;
}


static void
osdDisplayMove(osdDISPLAY *md, int dx, int dy)
{
    osdDISPLAY *tmd ;                      /* The osdDisplayHd to move */
    int xx, yy ;
    
    xx = md->x ;
    yy = md->y ;
    
    /* Try to position the menu */
    md->x += dx ;
    md->y += dy ;
    
    menuPosition(md,1);
    
    /* Did we succeed, if not drop out */
    dx = md->x - xx ;
    dy = md->y - yy ;
    if((dx == 0) && (dy == 0))
        return ;
    md->x = xx ;
    md->y = yy ;
    
    /* two different paths, one - the simple case where this is the last dialog
     * and we can optimise the refresh for this. two where there are children
     * where we just have to grit our teeth and bare it
     */
    if((tmd=md->next) == NULL)
    {
        /* Restore whats underneath */
        osdDisplaySnapshotRestore(md) ;
        /* now move the menu and sub-menus and redraw */
        md->x += dx ;
        md->y += dy ;
        osdDisplaySnapshotStore(md) ;
        osdDisplaySnapshotDraw(md,0,0,md->width,md->depth,0) ;
        /* Draw an area to screen which covers both the old and new position */
        if(dx < 0)
        {
            xx = md->x ;
            dx = md->width-dx ;
        }
        else
            dx += md->width ;
        if(dy < 0)
        {
            yy = md->y ;
            dy = md->depth-dy ;
        }
        else
            dy += md->depth ;
        menuRenderArea(xx,yy,dx,dy) ;
    }
    else
    {
        /* Restore whats underneath */
        while(tmd->next != NULL)
            tmd = tmd->next ;
        for(;;)
        {
            osdDisplaySnapshotRestore(tmd) ;
            menuRenderArea(tmd->x,tmd->y,tmd->width,tmd->depth) ;
            if(tmd == md)
                break ;
            tmd = tmd->prev ;
        }
        /* now move the menu and sub-menus and redraw */
        md->x += dx ;
        md->y += dy ;
        tmd = md->next ;
        while((tmd != NULL) && !(tmd->flags & RF_TITLE))
        {
            menuPosition(tmd,1);
            tmd = tmd->next ;
        }
        /* Draw an area to screen which covers both the old and new position */
        while(md != NULL)
        {
            osdDisplaySnapshotStore(md) ;
            osdDisplaySnapshotDraw(md,0,0,md->width,md->depth,1) ;
            md = md->next ;
        }
    }
}

#if MEOPT_MOUSE

static void
osdDisplayResize(int dx, int dy)
{
    osdDISPLAY *md ;                      /* The osdDisplayHd to move */
    int xx, yy ;
    
    /* ZZZZ - Dont cope with this not being the last osdDisplayHd */
    if((md=osdCurMd)->next != NULL)
        return ;
    
    /* Store the old size */
    xx = md->width ;
    yy = md->depth ;
    
    if(dx > frameCur->width)
        dx = frameCur->width ;
    if(dy > frameCur->depth)
        dy = frameCur->depth+1 ;
    
    /* Construct the $result string */
    sprintf((char *)resultStr,"%4d%4d",dx,dy) ;
    
    /* Can the macro resize okay */
    if(execFunc((int) md->dialog->rszIndex,0,1) != meTRUE)
        return ;
    
    /* restore the screen underneath */
    osdDisplaySnapshotRestore(md) ;
                
    /* Find the root and reconstruct the menu */
    if((md = menuConfigure(md->dialog,md,0)) == NULL)
        return ;
                
    /* Position the menu */
    menuPosition(md,1);
                
    /* Snapshot the region occupied by the menu. */
    osdDisplaySnapshotStore(md) ;
                
    /* Render it to screen */
    menuRender (md) ;
    osdDisplaySnapshotDraw(md,0,0,md->width,md->depth,0) ;
    if(md->width > xx)
        xx = md->width ;
    if(md->depth > yy)
        yy = md->depth ;
    menuRenderArea(md->x,md->y,xx,yy) ;
}


static void
osdDisplayMouseMove(osdDISPLAY *md)
{
    meUByte oldAllKeys=TTallKeys ;
    meShort mmx, mmy ;
    int cc ;
    
    /* Enable all mouse movements - important if we've come from meGetStringFromUser */
    TTallKeys = 1 ;
    mmx = mouse_X ;
    mmy = mouse_Y ;
    for( ; (cc=meGetKeyFromUser(meFALSE,0,meGETKEY_SILENT)) != (ME_SPECIAL|SKEY_mouse_drop_1) ; )
    {
        if(cc == (ME_SPECIAL|SKEY_mouse_move_1))
        {
            if((mmx != mouse_X) || (mmy != mouse_Y))
            {
                osdDisplayMove(md,mouse_X-mmx,mouse_Y-mmy) ;
                mmx = mouse_X ;
                mmy = mouse_Y ;
                TTflush() ;
            }
        }
    }
    /* Restore state */
    TTallKeys = oldAllKeys ;
}

static void
osdDisplayMouseResize(void)
{
    meUByte oldAllKeys=TTallKeys ;
    meShort mmx, mmy ;
    int cc ;
    
    /* Enable all mouse movements - important if we've come from meGetStringFromUser */
    TTallKeys = 1 ;
    mmx = mouse_X ;
    mmy = mouse_Y ;
    for( ; (cc=meGetKeyFromUser(meFALSE,0,meGETKEY_SILENT)) != (ME_SPECIAL|SKEY_mouse_drop_1);)
    {
        if(cc == (ME_SPECIAL|SKEY_mouse_move_1))
        {
            if((mmx != mouse_X) || (mmy != mouse_Y))
            {
                osdDisplayResize(mouse_X-osdCurMd->x+1,mouse_Y-osdCurMd->y+1) ;
                mmx = mouse_X ;
                mmy = mouse_Y ;
                TTflush() ;
            }
        }
    }
    /* Restore state */
    TTallKeys = oldAllKeys ;
}

static void
osdScrollBarDrawBar(meSCROLLBAR *sb)
{
    menuRenderChildScrollBar(sb,osdNewChild,&osdNewChild->context[osdNewChild->newContext]) ;
}

static void
osdScrollBarDrawMain(meSCROLLBAR *sb)
{
    osdDISPLAY *md ;
    osdCONTEXT *mcp ;
    
    md = osdNewChild ;
    mcp = &md->context[md->newContext] ;
    if(sb->flags & WMHORIZ)
        mcp->child->display->x = - sb->curPos ;
    else
        mcp->child->display->y = - sb->curPos ;
    
    for(; md->flags & RF_CHILD ;)
    {
        menuRenderChildMain(md,mcp,0) ;
        md = md->prev ;
        mcp = &md->context[md->newContext] ;
    }
    menuRenderChildMain(md,mcp,osdRENDITEM_DRAW) ;
}

static void
scrollScrollBarEvent(meSCROLLBAR *sb, int dd)
{
    meUByte oldAllKeys=TTallKeys ;
    int newPos, cc ;
    
    /* Enable all mouse timed events */
    TTallKeys = 2 ;
    for(;;)
    {
        newPos = sb->curPos + dd ;
        if(newPos < 0)
            newPos = 0 ;
        else if(newPos > (sb->totLen - sb->wndLen))
            newPos = sb->totLen - sb->wndLen ;
        if(newPos != sb->curPos)
        {
            sb->curPos = newPos ;
            if(fillScrollBar(sb,0))
                meScrollBarDrawBar(sb) ;
            meScrollBarDrawMain(sb) ;
        }
        while((cc = meGetKeyFromUser(meFALSE,0,meGETKEY_SILENT)) != (ME_SPECIAL|SKEY_mouse_time_1))
        {
            if(cc == (ME_SPECIAL|SKEY_mouse_drop_1))
            {
                TTallKeys = oldAllKeys ;
                return ;
            }
        }
    }
}

static void
boxDragScrollBarEvent(meSCROLLBAR *sb)
{
    meUByte oldAllKeys=TTallKeys ;
    long startPos;                      /* Starting line number (top row) */
    long startMouse;                    /* Starting mouse position */
    long mouseRatio;                    /* Ratio of mouse movement to lines */
    long maxPos;                        /* The maximum top row */
    long mousePos;                      /* Curent mouse position */
    long curPos;                        /* Current top row. */
    int  cc ;
    
    /* Calc the initial position and values */
    /* If the buffer is wholly contained in the window then there is nothing
     * to do - quit now. Make sure that the top line is showing */
    if ((maxPos = sb->totLen - sb->wndLen) <= 0)
    {
        mouseRatio = -1;
        startPos = 0 ;
    }
    else
    {
        long scrollLength;               /* Length of scroll shaft */
        
        startPos = sb->curPos ;
        /* We normalise the scroll bar if there is over-hanging
         * text by supplying a scroll at the start of the scroll */
        if (maxPos < startPos)
            startPos = maxPos ;
        
        /* Remeber the starting mouse position. Keep the precision up by ustilising
         * the fractional mouse components if available. Store as a fixed point 
         * 8 bit fractional integer */
        if(sb->flags & WMHORIZ)
            startMouse = ((long)(mouse_X) << 8) + (long)(mouse_dX);
        else
            startMouse = ((long)(mouse_Y) << 8) + (long)(mouse_dY);
        
        scrollLength = (long) (sb->barLen - sb->boxLen) ;
        
        /* Build a ratio factor of number of lines to the length of the scroll. 
         * Again keep the precision by storing as a 8-bit fractional integer. */
        if (scrollLength <= 0)
            mouseRatio = -1;            /* Cannot do alot with this !! */
        else
            mouseRatio = ((long)(sb->totLen - sb->wndLen) << 8) / scrollLength ;
    }
    curPos = startPos ;
    /* Enable all mouse move events */
    TTallKeys = 1 ;
    for(;;)
    {
        if(curPos != sb->curPos)
        {
            sb->curPos = curPos ;
            if(fillScrollBar(sb,0))
                meScrollBarDrawBar(sb) ;
            meScrollBarDrawMain(sb) ;
        }
        while((cc = meGetKeyFromUser(meFALSE,0,meGETKEY_SILENT)) != (ME_SPECIAL|SKEY_mouse_move_1))
        {
            if(cc == (ME_SPECIAL|SKEY_mouse_drop_1))
            {
                TTallKeys = oldAllKeys ;
                return ;
            }
        }
        /* if the mouse ratio is -1 then nothing to do */
        if (mouseRatio > 0)
        {
            /* Compute the current mouse position. Use the fractional mouse information
             * if it is available. */
            if(sb->flags & WMHORIZ)
                mousePos = ((long)(mouse_X) << 8) + (long)(mouse_dX);
            else
                mousePos = ((long)(mouse_Y) << 8) + (long)(mouse_dY);
            mousePos -= startMouse;             /* Delta mouse position */
            
            /* Compute the required current position in the buffer. Take care with precision
             * here. If we are dealing with a very large buffer then reduce the accuracy of
             * the calculation to ensure that the buffer does not overflow. Biggest we can 
             * cater for here is 2^23 lines - this should be sufficient !!
             *
             * Simple compute by multiplying the delta mouse position by the ratio and add
             * to our reference start line. */
            if (mouseRatio >= (1 << 16))
                curPos = startPos + ((mousePos * (mouseRatio>>8)) >> 8);
            else
                curPos = startPos + ((mousePos * mouseRatio) >> 16);
            if (curPos < 0)
                curPos = 0;               /* Normailize incase of underflow */
            else if (curPos > maxPos)
                curPos = maxPos;
        }
        /* loop round */
    }
}

static void
handleScrollBarEvent(meSCROLLBAR *sb)
{
    int pos, spos ;
    
    if(sb->flags & WMHORIZ)
        spos = mouse_X - sb->xPos ;
    else
        spos = mouse_Y - sb->yPos ;
    
    pos = 0 ;
    if(sb->flags & WMSPLIT)
    {
        if(pos == spos)
            /* dont handle split yet */
            return ;
        pos++ ;
    }
    if(sb->flags & WMUP)
    {
        if(pos == spos)
        {
            /* On the up arrow */
            scrollScrollBarEvent(sb,-1) ;
            return ;
        }
        pos++ ;
    }
    if(sb->flags & WMSCROL)
    {
        int ii = pos ;
        pos += sb->boxTop ;
        if(pos > spos)
        {
            /* on the upper shaft */
            scrollScrollBarEvent(sb,-sb->wndLen) ;
            return ;
        }
        pos += sb->boxLen ;
        if(pos > spos)
        {
            /* on the box */
            boxDragScrollBarEvent(sb) ;
            return ;
        }
        pos = ii + sb->barLen ;
        if(pos > spos)
        {
            /* on the lower shaft */
            scrollScrollBarEvent(sb,sb->wndLen) ;
            return ;
        }
    }
    else if((pos += sb->barLen) > spos)
        /* in the blank bar */
        return ;
    if(sb->flags & WMDOWN)
    {
        if(pos == spos)
        {
            /* On the down arrow */
            scrollScrollBarEvent(sb,1) ;
            return ;
        }
    }
    return ;
}

/*
 * osdDisplayGetMousePosition
 * 
 * Given that xx, yy is in the given display, the function works out what
 * is underneath this point. Returns the following values:
 * 
 * 0 - nothing underneath
 * 1 - a menu item is underneath
 * 2 - the title bar
 * 3 - the kill 'x'
 * 
 * if xx,yy is over a child then the function is recursively called with
 * the childs display. But the childs kill & title bar (if present) are
 * ignored.
 */
static int
osdDisplayGetMousePosition(osdDISPLAY *md, meShort xx, meShort yy, int leftPick)
{
    osdCONTEXT *mcp;
    int ii;
    
    osdNewChild = md ;
    xx -= md->x ;
    yy -= md->y ;
    if(md->flags & RF_TITLE)
    {
        if(yy == 0)
        {
            /* if this is the kill 'x' then return 3 */
            if(xx == (md->width - 1))
            {
                if(md->flags & RF_DISABLE)
                    return 0 ;
                return 3  ;
            }
            /* On the rest of the title bar only a left pick button does anything
             * Handle the moving of a display */
            if(leftPick)
                osdDisplayMouseMove(md) ;
            /* events handled so return 0 */
            return 0 ;
        }
        else if(yy == (md->depth - 1))
        {
            if(leftPick && (md->flags & RF_RESIZE) &&
               !(md->flags & RF_DISABLE) && (xx == (md->width - 1)))
                /* Mouse on the the dialog bottom resize '*' */
                osdDisplayMouseResize() ;
            /* events handled so return 0 */
            return 0 ;
        }
    }            
    if(md->flags & RF_DISABLE)
        return -1 ;
    for (mcp = md->context, ii = 0; ii < md->numContexts; ii++, mcp++)
    {
        if ((yy >= mcp->y) && (yy < (mcp->y + mcp->depth)) &&
            (xx >= mcp->x) && (xx < (mcp->x + mcp->width)) )
        {
            if (mcp->menu->flags & MF_SEP)
            {
                /* if it has an argument then the argument is the item number to execute */
                if(leftPick && (mcp->menu->iflags & MFI_ARG) && !(mcp->menu->flags & MF_NBPAGE))
                {
                    osdITEM *mp;
                    int itm ;
                    itm = mcp->menu->argc ;
                    ii = md->numContexts ;
                    while(--ii >= 0)
                    {
                        mp = md->context[ii].menu ;
                        if((mp->item == itm) && 
                           ((mp->flags & MF_SUBMNU) || (mp->cmdIndex >= 0)))
                            break ;
                    }
                }
                else
                    ii = -1 ;
                if(ii < 0)
                    break;
                mcp = md->context+ii ;
            }
            md->newContext = ii ;
            xx -= mcp->x ;
            yy -= mcp->y ;
            /* Must be on the item. */
            if(mcp->menu->flags & MF_CHILD)
            {
                osdCHILD *child = mcp->child ;
                md = child->display ;
                if(mcp->menu->flags & MF_SCRLBOX)
                {
                    if(xx-- == 0)
                        return 0 ;
                    if(yy-- == 0)
                        return 0 ;
                    if(xx >= child->wndWidth)
                    {
                        /* only playing with the scroll bar if
                         * 1) Its a leftPick event
                         * 2) we're not past the end of scroll-bar
                         * 3) There is one
                         */
                        if(leftPick && (child->vertSBar != NULL) && (yy < child->wndDepth))
                        {
                            /* calculate the scroll bar's x & y position.
                             * easier to do here as this could be inside another
                             * scrolled dialog etc.
                             */
                            child->vertSBar->xPos = mouse_X - xx + child->wndWidth ;
                            child->vertSBar->yPos = mouse_Y - yy ;
                            /* Setup the scroll bar callback functions */
                            meScrollBarDrawBar = osdScrollBarDrawBar ;
                            meScrollBarDrawMain = osdScrollBarDrawMain ;
                            handleScrollBarEvent(child->vertSBar) ;
                        }
                        /* Always return 0 as any event has now been handled */
                        return 0 ;
                    }
                    if(yy >= child->wndDepth)
                    {
                        /* only playing with the scroll bar if
                         * 1) Its a leftPick event
                         * 2) There is one
                         */
                        if(leftPick && (child->horzSBar != NULL))
                        {
                            /* calculate the scroll bar's x & y position.
                             * easier to do here as this could be inside another
                             * scrolled dialog etc.
                             */
                            child->horzSBar->xPos = mouse_X - xx ;
                            child->horzSBar->yPos = mouse_Y - yy + child->wndDepth ;
                            /* Setup the scroll bar callback functions */
                            meScrollBarDrawBar = osdScrollBarDrawBar ;
                            meScrollBarDrawMain = osdScrollBarDrawMain ;
                            handleScrollBarEvent(child->horzSBar) ;
                        }
                        /* Always return 0 as any event has now been handled */
                        return 0 ;
                    }
                }
                return osdDisplayGetMousePosition(md,xx,yy,leftPick) ;
            }
            /* store the position in the dialog */
            osdCol = xx ;
            osdRow = yy ;
            return 1 ;
        }
    }
    return 0 ;
}

static int
osdDisplayMouseLocate(int leftPick)
{
    osdDISPLAY *md;

    /* Go to the last entry in the list */
    for (md = osdCurMd; md->next != NULL; md = md->next)
         ;

    /* Iterate backwards over the menu display blocks to locate the position
     * of the mouse. */
    do
    {
        /* Check if the mouse is within the window of the menu. A separate
         * check is performed is this is a horizontal or vertical menu */
        if ((mouse_Y >= md->y) && (mouse_Y < (md->y + md->depth)) &&
            (mouse_X >= md->x) && (mouse_X < (md->x + md->width)) )
        {
            osdNewMd = md ;
            return osdDisplayGetMousePosition(md,mouse_X,mouse_Y,leftPick) ;
        }
    } while ((md = md->prev) != NULL);
    
    return -1 ;
}

int
osdMouseContextChange(int leftPick)
{
    if((osdDisplayMouseLocate(leftPick) > 0) &&
       ((osdNewChild != osdCurChild) || 
        (osdCurChild->newContext != osdCurChild->curContext)))
        return meTRUE ;
    return meFALSE ;
}
#endif

/*
 * osdDisplaySelect
 * Character selection of the displayed menu.
 */
static int
osdDisplaySelect (int cc, osdDISPLAY *md)
{
    osdCONTEXT *mcp;                   /* Menu context pointer */
    osdITEM *mp;                       /* Menu pointer */
    int ii, best=-1;                   /* Item count */
    
    if((md->next != NULL) && !(md->next->flags & RF_FRSTLET) && 
       osdDisplaySelect(cc,md->next))
        return 1 ;
    for (mcp = md->context,ii=0; ii < md->numContexts; ii++, mcp++)
    {
        mp = mcp->menu ;
        if(mp->flags & MF_CHILD)
        {
            if(!(mcp->child->display->flags & RF_FRSTLET) && 
               osdDisplaySelect(cc,mcp->child->display))
            {
                osdNewMd = md ;
                md->newContext = ii ;
                return 1 ;                 /* Found */
            }
        }
        else if (mp->key == cc)
        {
            osdNewMd = md ;
            osdNewChild = md ;
            md->newContext = ii ;
            return 1 ;                 /* Found */
        }
        else if ((toLower(mp->key) == toLower(cc)) && (best == -1))
            best = ii ;
    }
    if (best != -1)
    {
        osdNewMd = md ;
        osdNewChild = md ;
        md->newContext = best ;
        return 1 ;                 /* Found */
    }
    return 0 ;                         /* Not found */
}

/*
 * osdDisplaySelectFirstLetter
 * Character selection of an item in a list - no hot keys
 */
static int
osdDisplaySelectFirstLetter(int cc, osdDISPLAY *md)
{
    osdCONTEXT *mcp;                   /* Menu context pointer */
    osdITEM *mp;                       /* Menu pointer */
    int ii, type, best=-1, bll=-1;
    
    type = 259 ;
    for (mcp = md->context,ii=0; ii < md->numContexts; ii++, mcp++)
    {
        mp = mcp->menu ;
        if (mp->key != '\0')
        {
            int dd, ll ;
            if (md->flags & RF_INSENSE)
                dd = toUpper(cc) - toUpper(mp->key) ;
            else
                dd = cc - mp->key ;
            if(dd < 0)
                dd += 256 ;
            if((ll = md->curContext - ii) < 0)
                ll += md->numContexts ;
            if(dd == 0)
            {
                if(ll > bll)
                {
                    type = 2 ;
                    best = ii ;
                    bll = ll ;
                }
            }
            else if((type > 2+dd) || ((type == 2+dd) && (ii > best)))
            {
                type = 2+dd ;
                best = ii ;
            }
        }
    }
    if (best == -1)
        return 0 ;             /* Not found */
    md->newContext = best ;
    return 1 ;                 /* Found */
}

static int
osdDisplayTabMove(osdDISPLAY *md, int dir, int mustSel)
{
    int tab, tt, btt=0, bii=-1, ii=md->curContext ;
    
    if(mustSel || (ii < 0))
        tab = -1 ;
    else
        tab = md->context[ii].menu->tab ;
    
    /* loop through the contexts find the closest match
     * (going backwards just for ease) */
    for(ii=md->numContexts ; ii-- ; )
    {
        if(((tt=md->context[ii].menu->tab) >= 0) && (tt != tab))
        {
            if(dir)
                tt -= tab ;
            else
                tt = tab - tt ;
            if((bii < 0) || 
               ((tt  > 0) && ((btt < 0) || (tt < btt))) ||
               ((btt < 0) && (tt < btt)))
            {
                bii = ii ;
                btt = tt ;
            }
        }
    }
    if(!mustSel && (btt <= 0) && (md->flags & RF_CHILD) &&
       osdDisplayTabMove(md->prev,dir,0))
        return 1 ;
    if((btt != 0) || mustSel)
    {
        if(bii < 0)
        {
            /* If this is a child and it has no tab order then always
             * select the file. This is for things like file list
             * widgets.
             */
            if(mustSel)
                dir = 1 ;
            if((bii = md->curContext) < 0)
                bii = (dir) ? -1 : md->numContexts ;
            for(;;)
            {
                if(dir)
                {
                    if(++bii == md->numContexts)
                    {
                        if(md->curContext < 0)
                            return 0 ;
                        bii = 0 ;
                    }
                }
                else
                {
                    if(bii <= 0)
                    {
                        if(md->curContext < 0)
                            return 0 ;
                        bii = md->numContexts ;
                    }
                    bii-- ;
                }
                if(bii == md->curContext)
                    return 0 ;
                if(!(md->context[bii].menu->flags & MF_SEP))
                    break ;
            }
        }
        if(md->context[bii].menu->flags & MF_CHILD)
        {
            if(!osdDisplayTabMove(md->context[bii].child->display,dir,1))
                return 0 ;
        }
        else
            osdNewChild = md ;
        md->newContext = bii ;
        if(!mustSel)
        {
            while(md->flags & RF_CHILD)
            {
                md = md->prev ;
                md->newContext = md->curContext ;
            }
            osdNewMd = md ;
        }
        return 1 ;
    }
/*    if(!mustSel && (md->flags & RF_CHILD))*/
/*        return osdDisplayTabMove(md->prev,dir,0) ;*/
    return 0 ;
}

static int
osdDisplayKeyMove (int dir, int nn)
{
    osdDISPLAY *hmd=osdCurMd, *md ;
    int loop, depth, flags ;
    
    for(loop=1,md=hmd ; md != osdCurChild ; loop++)
    {
        md->newContext = md->curContext ;
        md = md->context[md->curContext].child->display ;
    }
    md->newContext = md->curContext ;
    
    for( ; loop>=0 ; loop--)
    {
        if(loop == 0)
            hmd = md ;
        if((dir & (MF_NORTH|MF_SOUTH)) && (nn == 0) && (md->prev != NULL) &&
           !(md->prev->flags & RF_DISABLE) && (md->prev->newContext >= 0) &&
           ((md->prev->context[md->prev->newContext].menu->flags & (MF_CHILD|MF_SCRLBOX)) == (MF_CHILD|MF_SCRLBOX)))
            depth = md->prev->context[md->prev->newContext].child->wndDepth - 1 ;
        else
            depth = nn ;
        md->newContext = osdDisplayFindNext(md,md->newContext,dir, 1, depth);
        if(md->newContext >= 0)
        {
            flags = md->context[md->newContext].menu->flags;
            if((md->newContext != md->curContext) || (loop == 0))
            {
                if(flags & MF_CHILD)
                {
                    /* if this is a child then move into the child */
                    loop += 2 ;
                    md = md->context[md->newContext].child->display ;
                    continue ;
                }
                osdNewMd = hmd ;
                osdNewChild = md ;
                return 1 ;
            }
            /* If this is the first loop & we have a sub-menu and we're going
             * the right direction open it. */
            if((flags & MF_SUBMNU) &&
               (((flags & (MF_NORTH|MF_SOUTH)) && (dir & (MF_NORTH|MF_SOUTH))) ||
                ((flags & (MF_EAST |MF_WEST )) && (dir & (MF_EAST |MF_WEST ))) ))
            {
                /* we may want to actually go back to the parent, lets check */
                if(!(flags & dir) && (md->prev != NULL) &&
                   (md->prev->context[md->prev->curContext].menu->flags & (flags & (MF_NORTH|MF_SOUTH|MF_EAST|MF_WEST))))
                {
                    /* The cursor movement is back to the parent */
                    osdNewChild = md->prev ;
                    if(osdNewChild == osdNewMd->prev)
                        osdNewMd = osdNewChild ;
                    osdNewMd->newContext = osdNewMd->curContext ;
                    return 1 ;
                }
                
                if(md->next == NULL)
                    osdDisplaySub(md,OPEN_MENU) ;
                if((md = md->next) == NULL)
                    return -1 ;
                if((md->newContext=md->curContext) < 0)
                    continue ;
                osdNewMd = hmd ;
                osdNewChild = md ;
                return 1 ;
            }
        }
        /* If we've got this far, we can't move in the current menu
         * So lets see if we are actually try to manipulate the
         * parent menu
         */
        if(((md = md->prev) == NULL) ||
           (md->flags & RF_DISABLE))
            break ;
    }
    return -1 ;
    
}

static int
osdDisplayRedraw(void)
{
    osdDISPLAY *tmd, *md ;                 /* Temporary display frame */
    
    md = osdDisplayHd ;
    while(md != NULL)
    {
        if(md->flags & RF_REDRAW)
            break ;
        md = md->next ;
    }
    if(md == NULL)
        return 0 ;
    
    /* Advance to the end of the list */
    for (tmd=md ; tmd->next != NULL; tmd=tmd->next)
        /* NULL */;
    
    /* restore the snapshots backwards */
    for(;;)
    {
        /* Snapshot the region occupied by the menu. */
        osdDisplaySnapshotRestore(tmd) ;
        if(tmd == md)
            break ;
        tmd = tmd->prev ;
    }

    /* Now redraw all the osdDisplayHds back again */
    while(tmd != NULL)
    {
        int mxx, mnx, mxy, mny ; 
        
        mnx = tmd->x ;
        mxx = tmd->x + tmd->width ;
        mny = tmd->y ;
        mxy = tmd->y + tmd->depth ;
        
        /* reconstruct the menu - if this fails the user is in big trouble,
         * I recon this will core-dump! the good news is that this really
         * shouldn't fail!
         * SWP 30/12/98 - This does core-dump, unfortunately this does happen,
         * particulary on NoteBooks where an error in the child creation of a
         * new page can lead to this type of failure. return -1 and hope
         * the calling function can cope.
         */
        if((tmd = menuConfigure(tmd->dialog,tmd,0)) == NULL)
        {
            sgarbf = meTRUE ;
            return -1 ;
        }
        /* Position the menu */
        menuPosition(tmd,1);
        
        /* Snapshot the region occupied by the menu. */
        osdDisplaySnapshotStore(tmd) ;
        menuRender (tmd) ;
        osdDisplaySnapshotDraw(tmd,0,0,tmd->width,tmd->depth,0) ;
        
        /* Now draw to screen, render an area big enough to cover both the old
         * and the new */
        if(tmd->x < mnx)
            mnx = tmd->x ;
        if((tmd->x+tmd->width) > mxx)
            mxx = tmd->x+tmd->width ;
        if(tmd->y < mny)
            mny = tmd->y ;
        if((tmd->y+tmd->depth) > mxy)
            mxy = tmd->y+tmd->depth ;
        menuRenderArea(mnx,mny,mxx-mnx,mxy-mny) ;
        
        tmd = tmd->next ;
    }
    return 1 ;
}


/*
 * menuInteraction
 * Handle the menu interaction.
 */
static osdITEM *
menuInteraction (int *retState)
{
    int osdCursorState ;
    int cc, nit, ii, n, f ;
    meUInt arg ;
    int state ;
    osdITEM *mp ;
#ifdef _UNIX
    int moveCursor ;
#endif

    *retState = meFALSE ;
    
    /* Hide the cursor */
    osdCursorState = cursorState ;
#ifdef _UNIX
     if((moveCursor=((meSystemCfg & (meSYSTEM_CONSOLE|meSYSTEM_RGBCOLOR|
                                     meSYSTEM_ANSICOLOR|meSYSTEM_FONTS)) == meSYSTEM_CONSOLE)))
     {
          /* if running on termcap with no color or fonts then move the cursor
           * to the current item else the user will have no idea what the current item is */
         showCursor(meFALSE,1) ;
     }
     else
#endif
         if(cursorState >= 0)
             showCursor(meTRUE,-1-cursorState) ;
    for(;;)
    {
#ifdef _UNIX
        if(moveCursor)
        {
            if (osdCurChild->curContext >= 0)
            {
                /* Get the screen position of the current item */
                int xx, yy ;
                xx = osdCurChild->x + osdCurChild->context[osdCurChild->curContext].x ;
                yy = osdCurChild->y + osdCurChild->context[osdCurChild->curContext].y ;
                TTmove(yy,xx) ;
            }
        }
#endif
        TTflush() ;
        osdCol = -1 ;
        nit = 0 ;
        state = 0 ;
        /* Get a command from the keyboard */
        cc = meGetKeyFromUser(meFALSE,0,meGETKEY_SILENT);
        /* handle and osd bindings first */
        if(osdCurMd->dialog->nobinds)
        {
            register meBind  *ktp ;
            
            ktp = osdCurMd->dialog->binds;
            ii = osdCurMd->dialog->nobinds;
            while(--ii>=0)
                if(ktp[ii].code == cc)
                    break ;
            if(ii >= 0)
            {
                if(ktp[ii].arg)
                {
                    f = 1 ;
                    n = (int) (ktp[ii].arg + 0x80000000) ;
                }
                else
                {
                    f = 0 ;
                    n = 1 ;
                }
                lastCommand = thisCommand ;
                lastIndex = thisIndex ;
                thisCommand = cc ;
                thisIndex = ktp[ii].index ;
                TTallKeys = 0 ;
                ii = execFunc(thisIndex,f,n) ;
                TTallKeys = 1 ;
                if(ii != meTRUE)
                    break;
                continue ;
            }
        }
        /* Handle any literal keys */
#if MEOPT_MOUSE
        switch (cc)
        {
        case ME_SPECIAL|SKEY_mouse_pick_3:
            if(!(osdCurMd->flags & RF_RISLBUT))
                break ;
            /* no break */
        case ME_SPECIAL|SKEY_mouse_pick_1:
            nit = osdDisplayMouseLocate(1) ;
            if((nit == 1) && (osdNewChild->context[osdNewChild->newContext].menu->flags & MF_REPEAT))
            {
                state = MOUSE_MENU|EXECUTE_MENU|OPEN_MENU ;
                /* Enable all mouse timed events */
                TTallKeys = 2 ;
            }
            break ;
            
        case ME_SPECIAL|SKEY_mouse_time_3:
            if(!(osdCurMd->flags & RF_RISLBUT))
                break ;
            /* no break */
        case ME_SPECIAL|SKEY_mouse_time_1:
            nit = osdDisplayMouseLocate(0) ;
            if((nit == 1) && (osdNewChild->curContext == osdNewChild->newContext) &&
               (osdNewChild->context[osdNewChild->newContext].menu->flags & MF_REPEAT))
                state = MOUSE_MENU|EXECUTE_MENU|OPEN_MENU ;
            else
                nit = 0 ;
            break ;
        
        case ME_SPECIAL|SKEY_mouse_move_3:
            if(!(osdCurMd->flags & RF_RISLBUT))
                break ;
            /* no break */
        case ME_SPECIAL|SKEY_mouse_move_1:
        case ME_SPECIAL|SKEY_mouse_move:  /* Mouse moved */
            nit = osdDisplayMouseLocate(0) ;
            break ;
        
        case ME_SPECIAL|SKEY_mouse_drop_3:
            if(!(osdCurMd->flags & RF_RISLBUT))
            {
                state = QUIT_MENU;
                break ;
            }
            /* no break */
        case ME_SPECIAL|SKEY_mouse_drop_1:
            nit = osdDisplayMouseLocate(0) ;
            if(nit == -1)
                /* complete miss - abort */
                state = QUIT_MENU ;
            else if(nit == 1)
            {
                /* selected item */
                if(!(osdNewChild->context[osdNewChild->newContext].menu->flags & MF_REPEAT))
                    state = MOUSE_MENU|EXECUTE_MENU|OPEN_MENU ;
            }
            else if(nit == 3)
            {
                /* The kill title bar button */
                if((osdNewMd->prev == NULL) ||
                   (osdNewMd->prev->flags & RF_DISABLE))
                    state = QUIT_MENU ;
                else
                {
                    osdNewMd = osdNewChild = osdNewMd->prev ; 
                    nit = 1 ;
                }
            }
            /* disable the mouse timed events if on a repeat button */
            TTallKeys = 1 ;
            break;
        case ME_SPECIAL|SKEY_mouse_drop_2:
            /* middle buttons execute the current item - particularly useful for wheel mice */
            if(osdCurChild->curContext >= 0)
            {
                osdNewChild = osdNewMd = osdCurMd ;
                for(;;)
                {
                    osdNewChild->newContext = osdNewChild->curContext ;
                    if(osdNewChild == osdCurChild)
                        break ;
                    osdNewChild = osdNewChild->context[osdNewChild->newContext].child->display ;
                }
                nit = 1 ;
                state = EXECUTE_MENU|OPEN_MENU|FOCUS_MENU|ENTER_MENU;
            }
            break;
        
        default:
#else
        {
#endif /* MEOPT_MOUSE */
            /*
             * Handle bounded keys
             */
            ii = decode_key((meUShort)(cc), &arg) ;
            if(arg)
            {
                f = 1 ;
                n = (int) (arg + 0x80000000) ;
            }
            else
            {
                f = 0 ;
                n = 1 ;
            }
            switch (ii)
            {
            case CK_GOBOF:                  /* M-< - beginning of buffer */
                nit = osdDisplayKeyMove(MF_SOUTH,-1) ;
                break;
            case CK_GOEOF:                  /* M-< - end of buffer */
                nit = osdDisplayKeyMove(MF_NORTH,-1) ;
                break;
            case CK_FORCHR:                 /* ^F  - Forward character */
                nit = osdDisplayKeyMove(MF_EAST,n) ;
                break;
            case CK_BAKCHR:                 /* ^B  - Previous charater */
                nit = osdDisplayKeyMove(MF_WEST,n) ;
                break;
            case CK_BAKLIN:                 /* C-p - Previous line */
                nit = osdDisplayKeyMove(MF_NORTH,n) ;
                break;
            case CK_FORLIN:                 /* C-n - Next line */
                nit = osdDisplayKeyMove(MF_SOUTH,n) ;
                break;
            case CK_MOVUWND:                  /* M-N - Move menu up */
                if(f == 0)
                    /* if scrolling by a page then assume its a page-up and
                     * we want to scroll a child dialog */
                    nit = osdDisplayKeyMove(MF_NORTH,0) ;
                else if(osdCurMd->flags & (RF_TITLE|RF_BOARDER))
                    /* Allow the menu to move if it has a title bar or a boarder
                     * This is different to hte mouse as for the mouse to be
                     * able to move the menu it must have a title bar */
                    osdDisplayMove(osdCurMd,0,-1) ;
                break ;
            case CK_MOVDWND:                  /* M-N - Move menu down */
                if(f == 0)
                    nit = osdDisplayKeyMove(MF_SOUTH,0) ;
                else if(osdCurMd->flags & (RF_TITLE|RF_BOARDER))
                    osdDisplayMove(osdCurMd,0,1) ;
                break ;
            case CK_MOVLWND:                 /* M-N - Move menu left */
                if(f == 0)
                    nit = osdDisplayKeyMove(MF_EAST,0) ;
                else if(osdCurMd->flags & (RF_TITLE|RF_BOARDER))
                    osdDisplayMove(osdCurMd,-1,0) ;
                break ;
            case CK_MOVRWND:                 /* M-N - Move menu right */
                if(f == 0)
                    nit = osdDisplayKeyMove(MF_WEST,0) ;
                else if(osdCurMd->flags & (RF_TITLE|RF_BOARDER))
                    osdDisplayMove(osdCurMd,1,0) ;
                break ;
            case CK_DELFOR:                 /* Del - Delete */
            case CK_ABTCMD:                 /* ^G  - Abort */
            case CK_QUIT:                   /* C-x C-c or A-f4 - quit */
            case CK_EXIT:                   /* exit */
                state = QUIT_MENU;
                break;
            case CK_NEWLIN:                 /* ^M  - Return */
                if(osdCurChild->curContext >= 0)
                {
                    osdNewChild = osdNewMd = osdCurMd ;
                    for(;;)
                    {
                        osdNewChild->newContext = osdNewChild->curContext ;
                        if(osdNewChild == osdCurChild)
                            break ;
                        osdNewChild = osdNewChild->context[osdNewChild->newContext].child->display ;
                    }
                    nit = 1 ;
                    state = EXECUTE_MENU|OPEN_MENU|FOCUS_MENU|ENTER_MENU;
                }
                break;
                
            case CK_DOTAB:
                nit = osdDisplayTabMove(osdCurChild,1,0) ;
                break ;
            case CK_DELTAB:
                nit = osdDisplayTabMove(osdCurChild,0,0) ;
                break ;
                      
            case CK_RECENT:                 /* ^L  - Redraw the screen */
                sgarbf = meTRUE;
                update(meTRUE) ;
                break;
            default:
                if ((cc >= ' ') && (cc < 256))
                {
                    if(osdCurChild->flags & RF_FRSTLET)
                        nit = osdDisplaySelectFirstLetter(cc,osdCurChild) ;
                    else if((nit = osdDisplaySelect(cc,osdCurMd)) != 0)
                        state = EXECUTE_MENU|OPEN_MENU|FOCUS_MENU|ENTER_MENU ;
                }
                break;
            }
        }
        if(state & QUIT_MENU)
            break ;
        
        if(nit != 1)
            continue ;
        
        mp = osdNewChild->context[osdNewChild->newContext].menu ;
        /* have we got something to execute?? */
        if((state & EXECUTE_MENU) && !(mp->flags & MF_SUBMNU) && (mp->cmdIndex < 0))
        {
            /* Hot keys can be used to reference other items to execute cause they
             * themselves are separaters so get the real thing to execute */
            
            /* if it has an argument then the argument is the item number to execute */
            if(mp->iflags & MFI_ARG)
            {
                if (mp->flags & MF_NBPAGE)
                {
                    if(osdNewChild->nbpContext == -1)
                        nit = -1 ;
                    else
                        nit = osdNewChild->newContext ;
                }
                else
                {
                    int itm ;
                    
                    itm = mp->argc ;
                    nit = osdNewChild->numContexts ;
                    while(--nit >= 0)
                    {
                        mp = osdNewChild->context[nit].menu ;
                        if((mp->item == itm) && 
                           ((mp->flags & MF_SUBMNU) || (mp->cmdIndex >= 0)))
                            break ;
                    }
                }
            }
            else
                nit = -1 ;
            if(nit < 0)
            {
                mlwrite(MWABORT|MWPAUSE,(meUByte *)"[Nothing to execute for item]") ;
                continue ;
            }
            osdNewChild->newContext = nit ;
        }
        if((state & EXECUTE_MENU) && !(mp->flags & MF_SUBMNU))
        {
            osdDISPLAY *nem, *nec ;
            osdCONTEXT *nep ;
            int         necflags ;
            
            /* we are about to execute an item which is not a sub-menu
             * look back for a non-exit item point
             */
            nem = osdNewMd ;
            nec = nem ;
            for(;;)
            {
                nep = &nec->context[nec->newContext] ;
                if((necflags = nep->menu->flags) & MF_NOEXIT)
                    break ;
                if(nep->child == NULL)
                {
                    nec = NULL ;
                    break ;
                }
                nec = nep->child->display ;
            }
            while((nec == NULL) && ((nem = nem->prev) != NULL))
            {
                if(nem->flags & RF_DISABLE)
                {
                    nem = NULL ;
                    break ;
                }
                nec = nem ;
                for(;;)
                {
                    nep = &nec->context[nec->curContext] ;
                    if((necflags = nep->menu->flags) & MF_NOEXIT)
                        break ;
                    if(nep->child == NULL)
                    {
                        nec = NULL ;
                        break ;
                    }
                    nec = nep->child->display ;
                }
            }
            
            if((nem == NULL) && !(mp->flags & MF_ENTRY))
            {
                /* if no non-exit was found & this is not an entry field then we can
                 * quit before execute - store the md's flags so the item can be 
                 * properly executed */
                *retState = osdNewChild->flags ;
                break ;
            }   
            /* Set the new position */
            osdDisplaySetNewFocus(state) ;
            osdNewMd = nem ;
            osdNewChild = nec ;
            state &= MOUSE_MENU ;
            if(menuExecute(mp,osdCurChild->flags,-1) != meTRUE)
                /* The function called failed - quit */
                break ;
            if(osdNewChild == NULL)
            {
                /* This must have been an entry with no non-exit - quit but flag as success */
                *retState = meTRUE ;
                break ;
            }
            /* Do we have to redraw this display? */
            if(necflags & MF_REDRAW)
                osdNewMd->flags |= RF_REDRAW ;
            /* Pop off any unwanted sub-menus */
            if(osdNewMd != osdCurMd)
                osdDisplayPop(osdNewMd->next) ;
            osdCurMd = osdNewMd ;
            osdCurChild = osdNewChild ;
            if((nit=osdDisplayRedraw()) < 0)
                /* Failed to redraw - quit */
                break ;
            else if(nit == 0)
            {
                /* assign focus */
#if MEOPT_MOUSE
                if(state & MOUSE_MENU)
                    osdDisplayMouseLocate(0) ;
#endif
                osdDisplaySetNewFocus(0) ;
                osdDisplayEvaluate () ;
            }
            else if((state & MOUSE_MENU)
#if MEOPT_MOUSE
                    && (osdDisplayMouseLocate(0) == 1)
#endif
                    )
                osdDisplaySetNewFocus(0) ;

        }
        else
            /* Set the new position */
            osdDisplaySetNewFocus(state) ;
    }
        
    /* Restore the context */
    if(osdCursorState != cursorState)
        showCursor(meTRUE,osdCursorState-cursorState) ;
    TTflush() ;
    if (state & EXECUTE_MENU)
        return mp ;
    return NULL ;
}

/*
 * osd (On Screen Display)
 * Main entry point to define or invoke a menu or on screen display item.
 */

int
osd (int f, int n)
{
    meUByte oldAllKeys=TTallKeys ;
    osdITEM    *mp;                     /* Pointer to the menu */
    osdDISPLAY *md, *pmd;               /* Pointer to the osdDisplayHd */
    meUByte oldClexec;	                /* command line execution flag	*/
    meUByte buf [meBUF_SIZE_MAX];                 /* Reply buffer */
    int   noDis, ii ;                   /* Status of the invocation */

    /* If no arguments are defined then a menu is being
     * defined. */
    if ((f == meFALSE) || (n < 0))
    {
        osdDIALOG *rp;                  /* Pointer to container root */
        meUByte txtbuf [meBUF_SIZE_MAX];          /* Text string buffer */
        meUByte cc, *dd, *bb, iflags ;
        int   id, item, flags ;
        int   txtlen, cmdlen;           /* Command length */
        int   argc, namidx, scheme ;
        meShort width, depth, tab ;
        
        /* Get the menu identity */
        if ((ii=meGetString((meUByte *)"Identity", 0, 0, buf, 16)) == meFALSE)
        {
            pmd = osdCurMd ;
            if(n < 0)
            {
                /* redraw dialog(s) */
                if((n == -1) && (pmd != NULL))
                    pmd->flags |= RF_REDRAW ;
                else if(osdDisplayHd != NULL)
                    osdDisplayHd->flags |= RF_REDRAW ;
                else
                    return meABORT ;
                return (osdDisplayRedraw() < 0) ? meABORT:meTRUE ;
            }
            if((pmd != NULL) && (pmd->flags & RF_NOPOP))
            {
                if(osdDisplayRedraw() < 0)
                    /* Failed to redraw - quit */
                    return meABORT ;
                pmd = pmd->prev ;
                noDis = 0 ;
                goto do_control_inter ;
            }
        }
        if ((ii != meTRUE) || (buf[0] == 'E'))
            /* The 'E' test is to catch the string "ERROR", returned when the user
             * gives an undefined variable, e.g. %undefined. This is a common occurance */
            return meABORT ;
        if((id = meAtoi(buf)) < 0)
        {
            /* top line menu-poking */
            /*
             * Poke a menu on the screen. The flags are interpreted as follows:-
             * 
             * -ve          Delete the menu line from the screen.
             *  0           Hide the menu line. Returns boolean state depending on 
             *              whether the menu line was showing or not.
             * 0x01         Restore the menu line to the screen.
             * 0x02         Define the menu line string.
             */
            
            if (meGetString((meUByte *)"Flags", 0, 0, buf, 16) == meABORT)
                return meABORT ;
            flags = meAtoi(buf) ;
            if (flags == 0)
                return frameSetupMenuLine (0) ;         /* hide the menu return whether it existed */
            if (flags < 0)
            {
                /* Destruct the old line */
                frameSetupMenuLine (0);             /* Delete the existing window */
                if(osdMainMenuMd != NULL)
                {
                    displayDestruct(osdMainMenuMd) ;
                    osdMainMenuMd = NULL ;
                }
                osdMainMenuId = -1 ;
            }
            else if(osdMainMenuId < 0)
                /* Main menu Id has not been defined */
                return meFALSE ;
            else
            {
                /* Allocate the menu window space for the menu line */
                frameSetupMenuLine(1);
                /* Flag the line as changed */
                frameCur->video.lineArray[0].flag |= VFCHNGD ;
            }
            return meTRUE ;              /* Finished */
        }
                
        if(id > 32767)
            return mlwrite(MWABORT|MWPAUSE,(meUByte *)"Invalid identity [%s]", buf);
        
        
        if(n < 0)
        {
            dialogDestruct (id);          /* Destruct the root */
            return meTRUE ;
        }
        
        /* Get the menu index */
        if ((meGetString((meUByte *)"Item", 0, 0, buf, 16) == meABORT) ||
            (((item = meAtoi(buf)) < 0) || (item > 32767)))
            return mlwrite(MWABORT|MWPAUSE,(meUByte *)"Invalid item identity [%s]", buf);
        
        /* Determine if this is a root item */
        if (item == 0)
        {
            if ((rp = dialogConstruct (id)) == NULL)
                return meABORT ;
            if (meGetString((meUByte *)"Flags", 0, 0, buf,meBUF_SIZE_MAX) != meTRUE)
                return meABORT ;
            
            /* turn the flag string into bits */
            bb = buf ;
            flags = 0 ;
            while((cc = *bb++) != '\0')
                if((dd = meStrchr(osdMenuFlags,cc)) != NULL)
                    flags |= 1 << ((int) (dd-osdMenuFlags)) ;
            
            rp->flags = flags ;
            
            if(rp->flags & RF_NOTEBOOK)
                rp->flags |= RF_GRID ;
            
            /* If it has a main dialog scheme then get it */
            if(flags & RF_MSCHEME) 
            {
                if (meGetString((meUByte *)"Scheme", 0, 0, buf, meBUF_SIZE_MAX) != meTRUE)
                    return meABORT ;
                rp->mScheme = convertUserScheme(meAtoi(buf),osdScheme) ;
            }
            else
                rp->mScheme = osdScheme ;
            /* If its an absolute position menu, get the position */
            if(rp->flags & (RF_ABSPOS|RF_OFFSTPOS))
            {
                if((meGetString((meUByte *)"X-pos", 0, 0, buf, 16) != meTRUE) ||
                   ((rp->x = (meShort) meAtoi(buf)),
                    (meGetString((meUByte *)"Y-pos", 0, 0, buf, 16) != meTRUE)))
                    return meABORT ;
                rp->y = (meShort) meAtoi(buf) ;
            }
            /* If theres a size given, get it. Note the following values mean:
             * 
             *  0 - Use minimum space required, max is screen-size
             * -1 - Set the size to the screen-size
             * -2 - Use minimum space required but no restriction on the max size
             * 
             */
            if(rp->flags & RF_SIZE)
            {
                for(; item<2 ; item++)
                {
                    if((meGetString((meUByte *)"Width", 0, 0, buf, 16) != meTRUE) ||
                       ((rp->width[item] = (meShort) meAtoi(buf)),
                        (meGetString((meUByte *)"Depth", 0, 0, buf, 16) != meTRUE)))
                        return meABORT ;
                    rp->depth[item] = (meShort) meAtoi(buf) ;
                }
            }
            if(rp->flags & RF_DEFAULT)
            {
                if(meGetString((meUByte *)"Default", 0, 0, buf, 16) != meTRUE)
                    return meABORT ;
                rp->defItem = (meShort) meAtoi(buf) ;
            }
            if(rp->flags & RF_FOCALITM)
            {
                if(meGetString((meUByte *)"Focus", 0, 0, buf, 16) != meTRUE)
                    return meABORT ;
                rp->focalItem = (meShort) meAtoi(buf) ;
            }
            if(rp->flags & RF_TITLE)
            {
                if(flags & RF_TSCHEME) 
                {
                    if (meGetString((meUByte *)"Scheme", 0, 0, buf, meBUF_SIZE_MAX) != meTRUE)
                        return meABORT ;
                    rp->tScheme = convertUserScheme(meAtoi(buf),rp->mScheme) ;
                }
                else
                    rp->tScheme = rp->mScheme ;
                if(meGetString((meUByte *)"Text", 0, 0, txtbuf, meBUF_SIZE_MAX) == meTRUE)
                    rp->strData = meStrdup(txtbuf) ;
            }
            /* get the resize command if there is one */
            if(rp->flags & RF_RESIZE)
            {
                if ((ii = meGetString((meUByte *)"Command", MLCOMMAND, 0, buf, meBUF_SIZE_MAX)) != meTRUE)
                    return meABORT ;
                if ((rp->rszIndex = decode_fncname(buf,0)) < 0)
                    return mlwrite (MWABORT|MWPAUSE,(meUByte *)"Cannot find command [%s]", buf);
            }
            /* get the resize command if there is one */
            if(rp->flags & RF_CONTROL)
            {
                if ((ii = meGetString((meUByte *)"Control", MLCOMMAND, 0, buf, meBUF_SIZE_MAX)) != meTRUE)
                    return meABORT ;
                if ((rp->cntIndex = decode_fncname(buf,0)) < 0)
                    return mlwrite (MWABORT|MWPAUSE,(meUByte *)"Cannot find command [%s]", buf);
            }
            else
                rp->cntIndex = -1 ;
            /* get the initialize command if there is one */
            if ((ii = meGetString((meUByte *)"Command", MLCOMMAND, 0, buf, meBUF_SIZE_MAX)) == meABORT)
                return meABORT ;
            rp->cmdIndex = -1 ;                     /* Assume no command */
            if (ii == meTRUE)
            {
                if ((rp->cmdIndex = decode_fncname(buf,0)) < 0)
                    return mlwrite (MWABORT|MWPAUSE,(meUByte *)"Cannot find command [%s]", buf);
            }
            /* is this the main menu? is so store the id */
            if(rp->flags & RF_MAINMN)
                osdMainMenuId = id ;
            /* finished the root definition */
            dialogResetDisplays(rp,0) ;
            return meTRUE;
        }
        
        if ((rp = dialogFind (id)) == NULL)
            return mlwrite(MWABORT|MWPAUSE,(meUByte *)"[Osd dialog %d undefined]", id);
        
            
        /* Get the enable flag */
        if (meGetString((meUByte *)"Flags", 0, 0, buf,meBUF_SIZE_MAX) != meTRUE)
            return meABORT ;
        /* turn the flag string into bits */
        bb = buf ;
        flags = 0 ;
        iflags = 0 ;
        while((cc = *bb++) != '\0')
            if((dd = meStrchr(osdItemFlags,cc)) != NULL)
                flags |= 1 << ((int) (dd-osdItemFlags)) ;
        
        if(rp->flags & RF_NOTEBOOK)
        {
            if(flags & MF_CHILD)      
            {
                flags &= ~(MF_SCRLBOX|MF_HORZADD|MF_NBPAGE) ;
                flags |= MF_GRID ;
            }
            else if(flags & MF_NBPAGE)
            {
                flags &= ~(MF_CHECK|MF_ENTRY|MF_SUBMNU) ;
                flags |= MF_HORZADD|MF_GRID|MF_REDRAW|MF_NOEXIT ;
            }
            else
                return mlwrite(MWABORT,(meUByte *)"[Illegal NoteBook item]") ;
        }
        
        /* if this item has a sub-menu and no direction is set,
         * Set it to the default east direction
         */
        if((flags & (MF_SUBMNU|MF_NORTH|MF_SOUTH|MF_WEST)) == MF_SUBMNU)
            flags |= MF_EAST ;
        if(flags & MF_CHILD)
        {
            if (rp->flags & RF_ALPHA)
                return mlwrite(MWABORT,(meUByte *)"[Cannot have child item with an Alpha dialog]") ;
            iflags |= MFI_NOTEXT ;
            if(flags & MF_SCRLBOX)
                flags |= MF_SIZE ;
            txtlen = 0 ;
        }
        
        if(flags & MF_TAB) 
        {
            if(meGetString((meUByte *)"Tab", 0, 0, buf, 16) != meTRUE)
                return meABORT ;
            tab = (meShort) meAtoi(buf) ;
        }
        else
            tab = -1 ;
        
        scheme = rp->mScheme ;
        if(flags & MF_SCHEME) 
        {
            if (meGetString((meUByte *)"Scheme", 0, 0, buf, meBUF_SIZE_MAX) != meTRUE)
                return meABORT ;
            scheme = convertUserScheme(meAtoi(buf),osdScheme) ;
        }
        if(flags & MF_SIZE) 
        {
            if((meGetString((meUByte *)"Width", 0, 0, buf, 16) != meTRUE) ||
               ((width = (meShort) meAtoi(buf)),
                (meGetString((meUByte *)"Depth", 0, 0, buf, 16) != meTRUE)))
                return meABORT ;
            depth = (meShort) meAtoi(buf) ;
        }
        if(!(flags & MF_CHILD))
        {
            flags &= ~MF_SCRLBOX ;
            /* Get the string field - not needed if deleting a non alpha item */
            if ((ii = meGetString((meUByte *)"Text", 0, 0, txtbuf, meBUF_SIZE_MAX)) == meABORT)
                return meABORT ;
            else if (ii == meFALSE)
            {
                if (rp->flags & RF_ALPHA)
                    return mlwrite(MWABORT,(meUByte *)"[Cannot have item with no text in an Alpha dialog]") ;
                iflags |= MFI_NOTEXT ;
                txtlen = 0 ;
            }
            else
                txtlen = meStrlen(txtbuf) + 1 ;  /* Length of the string. */
        }
        /* Get the numeric argument. Check for 'f' which means false or
         * a value which means true. */
        argc = 1 ;
        if ((ii = meGetString((meUByte *)"Argument", 0, 0, buf, 16)) == meABORT)
            return meABORT ;
        if (ii == meTRUE)
        {
            if(toLower(buf[0]) != 'f')
            {
                iflags |= MFI_ARG;
                argc = meAtoi(buf) ;
            }
        }
        else
            flags |= MF_SEP ;

        /* Get the command */
        if ((ii = meGetString((meUByte *)"Command", MLCOMMAND, 0, buf, meBUF_SIZE_MAX)) == meABORT)
            return meABORT ;
        namidx = -1;              /* Assume no sub-command */
        cmdlen = 0;                     /* Assume no command string */
        if (ii == meTRUE)
        {
            if (flags & MF_STR)
                cmdlen = meStrlen (buf) + 1;
            else if ((namidx = decode_fncname(buf,0)) < 0)
                return mlwrite(MWABORT|MWPAUSE,(meUByte *)"Cannot find command [%s]", buf);
        }
        
        if((txtlen+cmdlen) == 0)
            dd = NULL ;
        else if((dd = meMalloc(txtlen+cmdlen)) == NULL)
            return meABORT ;
        if(txtlen > 0)
        {
            /* Copy in the text data - convert any non-pokable chars to '.' */
            ii = txtlen ;
            dd[--ii] = 0 ;
            while(--ii >= 0)
            {
                cc = txtbuf[ii] ;
                if(!isPokable(cc) && (cc != meNLCHAR))
                    cc = '.' ;
                dd[ii] = cc ;
            }
        }
        if ((mp = itemFind (rp, item, (flags & MF_CHECK) ? dd+6:dd)) == NULL)
        {
            if ((mp = (osdITEM *) meMalloc(sizeof(osdITEM))) == NULL)
                return meABORT ;
            mp->item = item ;
            mp->flags = flags ;
            mp->iflags = iflags ;
            mp->strData = dd ;
            /* Add new item */
            itemAdd(rp, mp) ;
        }
        else
        {
            meNullFree(mp->strData) ;
            mp->flags = flags ;
            mp->iflags = iflags ;
            mp->strData = dd ;
        }
        if(flags & MF_CHECK)
        {
            for(ii=0 ; ii<6 ; ii++)
                if(dd[ii] == '^')
                    dd[ii] = '\0' ;
        }
        mp->scheme = scheme ;
        mp->argc = argc ;
        mp->tab = tab ;
        if(flags & MF_SIZE)
        {
            mp->len = width ;
            mp->height = depth ;
        }
        
        /* Copy in the command string */
        if (cmdlen > 0)
        {
            meStrcpy(dd+txtlen,buf);
            mp->cmdIndex = txtlen ;
        }
        else
            mp->cmdIndex = namidx ;
        
        menuSetItemLength(rp,mp);
        dialogResetDisplays(rp,1) ;
        if((flags & MF_ENTRY) && ((mp->height < 1) || (mp->len < 3)))
        {
            /* an entry box this small will blow ME up! fix it and bitch */
            mp->height = 1 ;
            mp->len = 3 ;
            return mlwrite(MWABORT|MWPAUSE,(meUByte *)"[Entry size is too small]");
        }

        return meTRUE ;
    }

    noDis = 0 ;
    pmd = NULL ;
    md = osdDisplayHd ;
    while(md != NULL)
    {
        if(!(md->flags & RF_DISABLE))
        {
            md->flags |= RF_DISABLE ;
            noDis++ ;
        }
        pmd = md ;
        md = md->next ;
    }
    
    /* Argument supplied  - render the menu */
    /* Flag that we're in osd */
    TTallKeys = 1 ;                     /* Enable all keys */
    if(osdDisplayPush(n,FOCUS_MENU) == NULL)
    {
        TTallKeys = oldAllKeys ;
        return meABORT ;
    }
do_control_inter:
    /* we must set the clexec flag to false to ensure we execute
     * as if we are not currently running a macro. This is for 2 reasons
     * 
     * 1)  We are now waiting for user input. If this flag is set then the
     *     rest of the system thinks the opposite so things like screen
     *     updates wont happen.
     * 
     * 2)  If a command like find-file is executed (not as a string) it would
     *     not get the argument of the user and fail if this flag was set.
     */
    oldClexec = clexec ;
    clexec = meFALSE ;
    md = (pmd != NULL) ? pmd->next:osdDisplayHd ;
    if(!(md->flags & RF_NOPOP) &&
       (md->dialog->cntIndex >= 0))
    {
        osdCurMd->flags |= RF_NOPOP ;
        ii = execFunc((int) osdCurMd->dialog->cntIndex,0,1) ;
        md = (pmd != NULL) ? pmd->next:osdDisplayHd ;
        osdDisplayPop(md);             /* Crash the list and clean up */
    }
    else
    {
        char oldUseMlBinds = useMlBinds ;
        useMlBinds = 1 ;
        mp = menuInteraction(&ii) ;
        /* remove the osd flag */
        useMlBinds = oldUseMlBinds ;
        md = (pmd != NULL) ? pmd->next:osdDisplayHd ;
        if(!(md->flags & RF_NOPOP))
            osdDisplayPop(md);             /* Crash the list and clean up */
        if(mp != NULL)
            ii = menuExecute(mp,ii,-1) ;
    }
    while(--noDis >= 0)
    {
        pmd->flags &= ~RF_DISABLE ;
        pmd = pmd->prev ;
    }
    clexec = oldClexec ;
    TTallKeys = oldAllKeys ;
    
    return ii ;
}

    
void
osdMainMenuUpdate(int force)
{
    if(force || (osdMainMenuMd == NULL) || (osdMainMenuMd->flags & RF_REDRAW))
    {
        osdDIALOG  *rp ;
        
        /* Find the root of the sub-menu */
        if(((rp = dialogFind(osdMainMenuId)) == NULL) ||
           ((osdMainMenuMd = menuConfigure(rp,osdMainMenuMd,0)) == NULL))
            return ;
        
        osdMainMenuMd->x = 0 ;
        osdMainMenuMd->y = 0 ;
        menuRender(osdMainMenuMd) ;
    }
    
    /* Optimisation here - if the top menu is currently active 
     * don't do anything */
    if((osdDisplayHd != NULL) && (osdDisplayHd->dialog->id == osdMainMenuId))
        return ;
    
    osdDisplaySnapshotDraw(osdMainMenuMd,0,0,osdMainMenuMd->width,1,1) ;
    
    frameCur->video.lineArray->flag &= ~VFCHNGD ;
}


int
osdMainMenuCheckKey(int cc)
{
    if(frameCur->menuDepth <= 0)
        return 0 ;
    
    if(osdMainMenuMd == NULL)
        osdMainMenuUpdate(0) ;
    if(osdMainMenuMd != NULL)
    {
        osdCONTEXT *mcp;                   /* Menu context pointer */
        osdITEM *mp;                       /* Menu pointer */
        int ii, best=-1;                   /* Item count */
        
        for (mcp = osdMainMenuMd->context,ii=0; ii < osdMainMenuMd->numContexts; ii++, mcp++)
        {
            mp = mcp->menu ;
            if (mp->key == cc)
            {
                best = ii ;
                break ;
            }
            else if ((toLower(mp->key) == toLower(cc)) && (best == -1))
                best = ii ;
        }
        if (best != -1)
            return (osdMainMenuId | (osdMainMenuMd->context[best].menu->item << 16)) ;
    }
    return 0 ;
}

#if MEOPT_LOCALBIND
int
osdBindKey(int f, int n)
{
    osdDIALOG *rp;
    meUByte buf[16];
    
    /* Get the menu root */
    if(meGetString((meUByte *)"Identity", 0, 0, buf, 16) != meTRUE)
        return meABORT ;
    if((rp=dialogFind(meAtoi(buf))) == NULL)
        return mlwrite(MWABORT|MWPAUSE,(meUByte *)"[Osd dialog %s undefined]",buf);
    return bindkey((meUByte *)"Osd bind", f, n, &(rp->nobinds), &(rp->binds)) ;
}
int
osdUnbindKey(int f, int n)
{
    osdDIALOG *rp;
    meUByte buf[16];
    
    /* Get the menu root */
    if(meGetString((meUByte *)"Identity", 0, 0, buf, 16) != meTRUE)
        return meABORT ;
    if((rp=dialogFind(meAtoi(buf))) == NULL)
        return mlwrite(MWABORT|MWPAUSE,(meUByte *)"[Osd dialog %s undefined]",buf);
    return unbindkey((meUByte *)"Osd",0,&(rp->nobinds),&(rp->binds)) ;
}
#endif

#ifdef _ME_FREE_ALL_MEMORY
void osdFreeMemory(void)
{
    osdDIALOG *rp;                           /* Pointer to item */
    
    /* Iterate down the list looking for the block */
    for (rp = osdDialogHd ; rp != NULL ; rp = rp->next)
        dialogDestruct (rp->id);          /* Destruct the root */
    while (osdDialogHd != NULL)
    {
        rp = osdDialogHd ;
        osdDialogHd = rp->next ;
        free(rp) ;
    }
    if(osdMainMenuMd != NULL)
    {
        meFree(osdMainMenuMd->storeSchm) ;
        meFree(osdMainMenuMd) ;
    }
}
#endif

#endif

