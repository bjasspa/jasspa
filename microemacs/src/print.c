/* -*- C -*- ****************************************************************
 *
 *  System        : MicroEmacs Jasspa Distribution
 *  Module        : print.c
 *  Synopsis      : Buffer printing routines
 *  Created By    : Jon Green & Steven Phillips
 *  Created       : 1996
 *  Last Modified : <010730.2036>
 *
 *  Description
 *     This file contains routines to format and print buffers
 *
 *  Notes
 * 
 ****************************************************************************
 * 
 * Copyright (c) 1996-2000 Jon Green & Steven Phillips    
 *    
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the  authors be held liable for any damages  arising  from
 * the use of this software.
 *     
 * This software was generated as part of the MicroEmacs JASSPA  distribution,
 * (http://www.jasspa.com) but is excluded from those licensing restrictions.
 *
 * Permission  is  granted  to anyone to use this  software  for any  purpose,
 * including  commercial  applications,  and to alter it and  redistribute  it
 * freely, subject to the following restrictions:
 *
 *  1. The origin of this software must not be misrepresented; you must not
 *     claim that you wrote the original software. If you use this software
 *     in a product, an acknowledgment in the product documentation would be
 *     appreciated but is not required.
 *  2. Altered source versions must be plainly marked as such, and must not be
 *     misrepresented as being the original software.
 *  3. This notice may not be removed or altered from any source distribution.
 *
 * Jon Green               jon@jasspa.com
 * Steven Phillips         bill@jasspa.com
 *
 ****************************************************************************/

#include "emain.h"

#if PRINT

#if (defined _UNIX) || (defined _DOS) || (defined _WIN32)
#include <sys/types.h>
#include <time.h>
#endif
#include "eprint.h"

#define TESTPRINT  0                    /* Enable printer testing */

uint8 *printNames [] =
{
#define DEFPRINT(s,t,v) (uint8 *)s,
#include "eprint.def"
    NULL
#undef DEFPRINT
};

uint8 printTypes[] =
{
#define DEFPRINT(s,t,v) t,
#include "eprint.def"
    0
#undef DEFPRINT
};

PRINTER printer;
static BUFFER *gbp;                     /* Source of print buffer */
static BUFFER *gpbp;                    /* Composition print buffer */

#if HILIGHT
#define mePRINT_FONT      0x00ff
#define mePRINT_COLOR     0xff00

#define mePRINTOPTION_FONT    0x001
#define mePRINTOPTION_FCOLOR  0x002
#define mePRINTOPTION_BCOLOR  0x004

#define mePRINTSTYLE_FCOLOR (meFONT_COUNT)
#define mePRINTSTYLE_BCOLOR (meFONT_COUNT+1)
#define mePRINTSTYLE_SIZE   (meFONT_COUNT+2)

static int     fontCount ;
static meSTYLE fontMasks[2][mePRINTSTYLE_SIZE] ;
static int16   fontStrlen[2][mePRINTSTYLE_SIZE] ;
static uint8  *fontStrings[2][mePRINTSTYLE_SIZE] ;
static uint8  *fontPaths[2] = {(uint8 *)"start",(uint8 *)"end"} ;
static uint8  *fontOrder = (uint8 *)"biu" ;

/* The hilighting characters */
static uint8 hilchars[mePRINTSTYLE_SIZE] = {'b', 'i', 'l', 'r', 'u', 'F', 'B'};
meSCHEME printTextScheme=meSCHEME_INVALID ;
meSCHEME printCurrScheme=meSCHEME_INVALID ;

#endif

#ifdef _ME_FREE_ALL_MEMORY
void
printFreeMemory(void)
{
    meNullFree(printer.filter) ;
    meNullFree(printer.color) ;
    meNullFree(printer.scheme) ;
}
#endif

/*
 * getTranslationLen
 * Compute the length of the string with characters translated
 */
static int
getTranslationLen (uint8 *str, int n)
{
    int len = 0;                        /* Length of the string */
    int cc;                             /* Character pointer */
    uint8 *p;

    while (((cc = *str++) != '\0') && (--n >= 0))
    {
        if ((p = printer.filter[cc]) == NULL)
            len++;
        else
            len += meStrlen (p);
    }
    return (len);
}

/*
 * doTranslation
 * Translate the characters in the string ready for the printer.
 */
static int
doTranslation (uint8 *dest, uint8 *str, int n)
{
    int len = 0;                        /* Length of the string */
    int cc;                             /* Character pointer */
    uint8 *p;                   /* Character pointer */

    while (((cc = *str++) != '\0') && (--n >= 0))
    {
        /* Convert special characters i.e. box chars to an ASCII equivelent */
        if ((meSystemCfg & meSYSTEM_FONTFIX) && (cc < TTSPECCHARS))
            cc = ttSpeChars [cc];
        
        /* Process the character through the filter */
        if ((p = (uint8 *) printer.filter[cc]) == NULL)
            dest [len++] = cc;
        else
        {

            while ((dest[len] = *p++) != '\0')
                len++;
        }
    }
    return len;
}

int
printColor (int f, int n)
{
    uint8 buf[20];              /* Local character buffer */
    int colNo;                  /* color number to add */
    int32 color;                /* the color */
    
    if(n == 0)
    {
        /* reset the color table */
        meNullFree(printer.color) ;
        printer.color = NULL ;
        printer.colorNum = 0;
        return TRUE ;
    }
        
    /* Get the color number and color */
    color = 0 ;
    if ((meGetString((uint8 *)"Number", 0, 0, buf, 20) == ABORT) ||
        ((colNo = meAtoi(buf)) < 0) || (colNo > 255) ||
        (meGetString((uint8 *)"Red",0,0,buf,20) == ABORT) ||
        ((color = mePrintColorSetRed(color,meAtoi(buf))),
         (meGetString((uint8 *)"Green",0,0,buf,20) == ABORT)) ||
        ((color = mePrintColorSetGreen(color,meAtoi(buf))),
         (meGetString((uint8 *)"Blue",0,0,buf,20) == ABORT)))
        return FALSE;
    color = mePrintColorSetBlue(color,meAtoi(buf)) ;

    /* Add a new entry to the printer color table */
    if (printer.colorNum <= colNo)
    {
        printer.color = meRealloc (printer.color, sizeof(uint32) * (colNo+1));
        if (printer.color == NULL)
        {
            /* Make safe */
            printer.colorNum = 0;
            return ABORT ;
        }
        /* Fill the memory with zero's */
        memset (&printer.color[printer.colorNum], 0,
                ((colNo+1) - printer.colorNum) * sizeof(uint32));
        printer.colorNum = colNo+1;
    }
    printer.color[colNo] = color ;
    return TRUE ;
}
int
printScheme (int f, int n)
{
    uint8 buf[20];              /* Local character buffer */
    int schmNo;                 /* color number to add */
    uint8 fc, bc, ff ;          /* temporary fore, back & font */
    meSTYLE scheme;             /* the scheme */
    
    if(n == 0)
    {
        /* reset the color table */
        meNullFree(printer.scheme) ;
        printer.scheme = NULL ;
        printer.schemeNum = 0;
        return TRUE ;
    }
        
    /* Get the scheme number and scheme */
    scheme = 0 ;
    if ((meGetString((uint8 *)"Number", 0, 0, buf, 10) == ABORT) ||
        ((schmNo = meAtoi(buf)) < 0) || (schmNo > 255) ||
        (meGetString((uint8 *)"Fore-col",0,0,buf,MAXBUF) == ABORT) ||
        ((fc = (uint8) meAtoi(buf)),
         (meGetString((uint8 *)"Back-col",0,0,buf,MAXBUF) == ABORT)) ||
        ((bc = (uint8) meAtoi(buf)),
         (meGetString((uint8 *)"Font",0,0,buf,MAXBUF) == ABORT)))
        return FALSE;
    ff = (uint8) meAtoi(buf) ;
    
    /* create the scheme */
    meStyleSet(scheme,fc,bc,ff) ;

    /* Add a new entry to the printer color table */
    if (printer.schemeNum <= schmNo)
    {
        printer.scheme = meRealloc (printer.scheme, sizeof(meSTYLE) * (schmNo+1));
        if (printer.scheme == NULL)
        {
            /* Make safe */
            printer.schemeNum = 0;
            return ABORT ;
        }
        /* Fill the memory with zero's */
        memset (&printer.scheme[printer.schemeNum], 0,
                ((schmNo+1) - printer.schemeNum) * sizeof(meSTYLE));
        printer.schemeNum = schmNo+1;
    }
    printer.scheme[schmNo] = scheme ;
    return TRUE ;
}

static int
printGetParams (void)
{
    REGHANDLE regPrint;              /* /print registry pointer */
    REGHANDLE regData;               /* Temporary data */
    uint8 *order, *ss ;
    int ii, jj, kk, option;
#if TESTPRINT 
    FILE *fp;
#endif
    mlwrite (0,(uint8 *)"[Configuring printer ...]");
	
    if(dofile("print",0,1) != TRUE)
        return ABORT ;
    
    /* Get the registry directory & the name of the driver out of the registry */
    if(((regPrint = regFind (NULL,(uint8 *)"/print")) == NULL) ||
       ((regData = regFind (regPrint,(uint8 *)"setup")) == NULL) ||
       ((ss = regGetString(regData,NULL)) == NULL) || !meAtoi(ss))
        return mlwrite (MWABORT|MWPAUSE,(uint8 *)"[Printer driver not setup]");

#ifdef WIN32
    if(((regData = regFind (regPrint,(uint8 *)"internal")) != NULL) &&
       ((ss = regGetString(regData,NULL)) != NULL))
        printer.pInternal = meAtoi(ss) ;
    else
        printer.pInternal = 0 ;
#endif
        
    /* Get the output destination */
    if (((regData = regFind (regPrint,(uint8 *)"dest")) == NULL) ||
        ((ss = regGetString (regData,NULL)) == NULL) ||
        ((printer.pDestination=meAtoi(ss)) < 0) ||
        (printer.pDestination > PDEST_COMLINE))
        printer.pDestination = PDEST_BUFFER ;
#ifdef _WIN32
    if ((printer.pDestination == PDEST_INTERNAL) && !printer.pInternal)
        printer.pDestination = PDEST_BUFFER ;
    else if ((printer.pDestination == PDEST_COMLINE) && printer.pInternal)
        printer.pDestination = PDEST_FILE ;
#else 
    /* Only windows support an internal device */
    if (printer.pDestination == PDEST_INTERNAL)
        printer.pDestination = PDEST_BUFFER ;
#endif

#if TESTPRINT
    /*
     * Get all of the printer codes out of the registry.
     */
    fp = fopen ("print.cfg", "wb");
#endif
    
    for (ii = 0; ii < PRINT_MAX; ii++)
    {
        /* NOTE:- If any of these change print.emf must be up-dated also */
        /* Try /print/<id> */
        if ((regData = regFind (regPrint,printNames[ii])) == NULL)
        {
            if (printTypes[ii] & mePD_INT)
                printer.param[ii].l = 0;
            else
                printer.param[ii].p = NULL;
        }
        else if (printTypes[ii] & mePD_INT)
            printer.param[ii].l = regGetLong(regData, 0) ;
        else
            printer.param[ii].p = regGetString(regData, NULL);
#if TESTPRINT
        if (fp)
        {
            if (printTypes[ii] & mePD_INT)
                fprintf (fp, "Parameter %s = %ld\n", printNames [ii],
                         printer.param[ii].l);
            else
                fprintf (fp, "Parameter %s = %s\n", printNames [ii],
                         (printer.param[ii].p == NULL) ? "(nil)" : printer.param[ii].p);
        }
#endif
    }

    /*
     * Get the filter codes out of the registry
     */

    /* Allocate the filter table */
    if (printer.filter == NULL)
        printer.filter = meMalloc (sizeof (uint8 *) * 256);
    memset(printer.filter,0, sizeof(uint8 *) * 256) ;

    if ((regData = regFind (regPrint,(uint8 *)"filter")) != NULL)
    {
        uint8 *nodeName;
        regData = regGetChild(regData) ;

        while (regData != NULL)
        {
            /* Get the node name out */
            nodeName = regGetName(regData) ;
            if (nodeName[0] != '\0')
            {
                if (nodeName[1] == '\0')
                    ii = nodeName[0];
                else
                    ii = meAtoi (nodeName);
                if ((ii >= 0) && (ii <= 255))
                    printer.filter [ii] = regGetString (regData, NULL);
            }
            regData = regGetNext (regData) ;
        }
    }
    /* get the font and color settings */
    option = 0 ;
    if(((regData = regFind (regPrint,(uint8 *)"scheme-flags")) != NULL) &&
       ((ss = regGetString (regData, NULL)) != NULL))
    {
        ii = meAtoi(ss) ;
        if(ii & mePRINTOPTION_FONT)
            option |= meFONT_MASK ;
        if(ii & mePRINTOPTION_FCOLOR)
            option |= 1 << mePRINTSTYLE_FCOLOR ;
        if(ii & mePRINTOPTION_BCOLOR)
            option |= 1 << mePRINTSTYLE_BCOLOR ;
    }
    if(!(option & (1<<mePRINTSTYLE_BCOLOR)))
        printer.param[mePS_BGCOL].p = NULL ;
        
    regData = regFind(regPrint,(uint8 *)"scheme-order") ;
    order = (regData == NULL) ? fontOrder : regGetString (regData, NULL) ;
        
    for(ii=0 ; ii<2 ; ii++)
    {
        /* Search in /print/<section>/order */
        for(jj=0 ; (jj<mePRINTSTYLE_SIZE) && (order[jj] != '\0') ; jj++)
        {
            fontStrings[ii][jj] = NULL ;
            fontStrlen[ii][jj] = 0 ;
            fontMasks[ii][jj] = 0 ;
            for(kk=0 ; kk < mePRINTSTYLE_SIZE ; kk++)
            {
                if(hilchars[kk] == order[jj])
                {
                    if(option & (1 << kk))
                    {
                        uint8 subkey[2] ;
                        subkey[0] = order[jj] ;
                        subkey[1] = '\0' ;
                        if ((regData = vregFind (regPrint,(uint8 *)"%s/%s", fontPaths[ii],subkey)) != NULL)
                        {
                            fontStrings[ii][jj] = regGetString (regData, NULL) ;
                            if(kk == mePRINTSTYLE_FCOLOR)
                            {
                                fontStrlen[ii][jj] = -1 ;
                                fontMasks[ii][jj] = meSTYLE_FCOLOR ;
                            }
                            else if(kk == mePRINTSTYLE_BCOLOR)
                            {
                                fontStrlen[ii][jj] = -1 ;
                                fontMasks[ii][jj] = meSTYLE_BCOLOR ;
                            }
                            else
                            {
                                fontStrlen[ii][jj] = (uint16) meStrlen(fontStrings[ii][jj]) ;
                                fontMasks[ii][jj] = 1 << (kk+16) ;
                            }
#if TESTPRINT
                            fprintf (fp, "Scheme %s - %d set to %c %d [%s]\n",fontPaths[ii],jj,order[jj],fontStrlen[ii][jj],fontStrings[ii][jj]);
#endif
                        }
                    }
                    break ;
                }
            }
        }
    }
    fontCount = jj ;
#if TESTPRINT
    if (fp)
        fclose (fp);
#endif
    mlwrite (0,(uint8 *)"[Printer read the registry ...]");
    /* Add a few defaults */
    if (printer.param[mePI_COLS].l < 1)
        printer.param[mePI_COLS].l = 1;
    if (printer.param[mePI_ROWS].l < 1)
        printer.param[mePI_ROWS].l = 1;
    if (printer.param[mePS_SCONT].p == NULL)
        printer.param[mePS_SCONT].p = (uint8 *)" ";
    if (printer.param[mePS_ECONT].p == NULL)
        printer.param[mePS_ECONT].p = (uint8 *)" ";
    /* sort out the header and footer */
    if((printer.param [mePI_FLAGS].l & PFLAG_ENBLHEADER) &&
       ((ss=printer.param [mePS_HEADER].p) != NULL))
    {
        printer.pNoHeaderLines = 1 ;
        while(*ss != '\0')
            if(*ss++ == '\n')
                printer.pNoHeaderLines++ ;
    }
    else
        printer.pNoHeaderLines = 0 ;
    if((printer.param [mePI_FLAGS].l & PFLAG_ENBLFOOTER) &&
       ((ss=printer.param [mePS_HEADER].p) != NULL))
    {
        printer.pNoFooterLines = 1 ;
        while(*ss != '\0')
            if(*ss++ == '\n')
                printer.pNoFooterLines++ ;
    }
    else
        printer.pNoFooterLines = 0 ;

    /* Default the output states */
    if (printer.param[mePS_BUFFER].p == NULL)
        printer.param[mePS_BUFFER].p = (uint8 *)"*printer*";
    return TRUE;
}

/*
 * printComputePageSetup
 * Compute the page setup for the printer
 */
static int
printComputePageSetup (int f)
{
    int xtraWidth;
    int xtraDepth;
    /* Sort out the parameters */
    if (printer.param[mePI_COLS].l < 1)
        printer.param[mePI_COLS].l = 1;
    if (printer.param[mePI_ROWS].l < 1)
        printer.param[mePI_ROWS].l = 1;
    /* allow for left/right margin, column dividers and line numbers */
    xtraWidth = printer.param[mePI_MLEFT].l + printer.param[mePI_MRIGHT].l ;
    xtraWidth += (printer.param[mePI_COLS].l-1) * printer.param[mePI_WSEP].l;
    if (printer.pLineNumDigits > 0)
        xtraWidth += printer.param[mePI_COLS].l * printer.pLineNumDigits ;
    
    /* allow for the row dividers, top/bottom margins and header/footer depths */
    xtraDepth = (printer.param[mePI_ROWS].l-1) ;
    xtraDepth += printer.param[mePI_MTOP].l + printer.param[mePI_MBOTTOM].l ;
    xtraDepth += printer.pNoHeaderLines + printer.pNoFooterLines ;

    /* Automatically fill some of the entries. */
    if ((printer.param[mePI_PAGEX].l == 0) && (printer.param[mePI_PAGEY].l == 0))
    {
        /* The page size is undefined, generate the page size from the
         * paper size */
        printer.param[mePI_PAGEX].l = (printer.param[mePI_PAPERX].l - xtraWidth) /
                  printer.param[mePI_COLS].l;
        printer.param[mePI_PAGEY].l = (printer.param[mePI_PAPERY].l - xtraDepth) /
                  printer.param[mePI_ROWS].l;
    }
    else if ((printer.param[mePI_PAPERX].l == 0) && (printer.param[mePI_PAPERY].l == 0))
    {
        /* The paper size is undefined, determine the paper size */
        printer.param[mePI_PAPERX].l = printer.param[mePI_PAGEX].l * printer.param[mePI_COLS].l + xtraWidth;
        printer.param[mePI_PAPERY].l = printer.param[mePI_PAGEY].l * printer.param[mePI_ROWS].l + xtraDepth;
    }

    /* Check to ensure that the page request is legal. */
    if ((printer.param[mePI_PAPERX].l < (printer.param[mePI_PAGEX].l * printer.param[mePI_COLS].l + xtraWidth)) ||
        (printer.param[mePI_PAPERY].l < (printer.param[mePI_PAGEY].l * printer.param[mePI_ROWS].l + xtraDepth)))
        return mlwrite (MWABORT|MWPAUSE,(uint8 *)"Invalid paper size %d x %d. Page requirements %d x %d",
                        printer.param[mePI_PAPERX].l,
                        printer.param[mePI_PAPERY].l,
                        printer.param[mePI_PAGEX].l * printer.param[mePI_COLS].l + xtraWidth,
                        (printer.param[mePI_PAGEY].l * printer.param[mePI_ROWS].l + xtraDepth));
    else if (!f)
        mlwrite (0,(uint8 *)"Valid paper size %d x %d. Page requirements %d x %d of %d x %d",
                 printer.param[mePI_PAPERX].l,
                 printer.param[mePI_PAPERY].l,
                 printer.param[mePI_PAGEX].l,
                 printer.param[mePI_PAGEY].l,
                 printer.param[mePI_COLS],
                 printer.param[mePI_ROWS]);
    return TRUE;
}

/*
 * Initialise the driver ready for a print.
 * Recompute the print metrics ready for the page.
 */
static int
printInit (int f, int n)
{
#ifdef _WIN32
    extern int printSetup(int n);
#endif
    /* Get the parameters out of the registry. */
    if (printGetParams () != TRUE)
        return FALSE;
    
    /* if the line numbers aren't required then set printer.pLineNumDigits to zero
     * so we only have to check that value */
    if (!(printer.param [mePI_FLAGS].l & PFLAG_ENBLLINENO))
        printer.pLineNumDigits = 0 ;

#ifdef _WIN32
    /* On windows call printSetup to handle the windows printer initialization
     * and the setup dialog if required.
     */
    if (printer.pInternal && (printSetup(n) != TRUE))
        return FALSE;
#endif
    
    if (printComputePageSetup (f) != TRUE)
        return FALSE;
    
    if(n < 0)
    {
        REGHANDLE regPrint;  
        
        regPrint = regFind (NULL,(uint8 *)"/print") ;
        regSet(regPrint, printNames[mePI_PAPERX], meItoa(printer.param[mePI_PAPERX].l));
        regSet(regPrint, printNames[mePI_PAPERY], meItoa(printer.param[mePI_PAPERY].l));
        regSet(regPrint, printNames[mePI_PAGEX], meItoa(printer.param[mePI_PAGEX].l));
        regSet(regPrint, printNames[mePI_PAGEY], meItoa(printer.param[mePI_PAGEY].l));
    }
    return TRUE;
}

/*
 * addComposedLine
 * Add a composed line to the lpage line list
 */
static void
addComposedLine (LINE **head, LINE **tail, uint8 *buf, int len)
{
    LINE *nline;                        /* New line */

    nline = lalloc (len);               /* Get the new line */
    if (len > 0)                        /* Copy in the data */
        memcpy (nline->l_text, buf, len);
    lforw (nline) = NULL;
    if ((lback (nline) = *tail) == NULL)
        *tail = (*head = nline);
    else
    {
        lforw (*tail) = nline;
        *tail = nline;
    }
}

/*
 * expandText
 * Expand control codes in the escape sequence
 */
static int
expandText (uint8 *buf, uint8 *strp, int level)
{
    uint8 cc;
    uint8 *bbuf = buf;
    uint8 *ss;

    if (strp == NULL)
        return 0;

    while ((cc = *strp++) != '\0')
    {
        /* Check out the escape characters */
        if (cc == '%')
        {
            switch (cc = *strp)
            {
            case 'c':
                if (((cc = *++strp) == 0xff) && (strp[1] == 0x01))
                {
                    strp++;
                    *buf++ = '\0';
                }
                else
                    *buf++ = cc;
                break;
            case 'h':
                buf += sprintf ((char *) buf, "%02d", printer.ptime->tm_hour);
                break;
            case 'm':
                buf += sprintf ((char *) buf, "%02d", printer.ptime->tm_min);
                break;
            case 's':
                buf += sprintf ((char *) buf, "%02d", printer.ptime->tm_sec);
                break;
            case 'D':
                buf += sprintf ((char *) buf, "%02d", printer.ptime->tm_mday);
                break;
            case 'M':
                buf += sprintf ((char *) buf, "%02d", printer.ptime->tm_mon+1);
                break;
            case 'Y':
                buf += sprintf ((char *) buf, "%02d", printer.ptime->tm_year % 100);
                break;
            case 'y':
                buf += sprintf ((char *) buf, "%d", printer.ptime->tm_year + 1900);
                break;
            case 'f':
                if ((gpbp->b_bname[0] == '*') || (gpbp->b_fname == NULL))
                    ss = (uint8 *) gpbp->b_bname;
                else
                    ss = (uint8 *) gpbp->b_fname;
                goto cat_string;
            case 'b':
                ss = (uint8 *) gbp->b_bname;
cat_string:
                if (ss != NULL)
                    buf += sprintf ((char *) buf, "%s", ss);
                break;
            case 'n':
                buf += sprintf ((char *) buf, "%d", printer.pPageNo);
                break;
            case '\0':
                break;
            default:
                *buf++ = cc;
                break;
            }
            if (*strp != '\0')
                strp++;
        }
        else if (cc == meNLCHAR)
        {
            if (level == 0)
                buf += expandText (buf, printer.param [mePS_EOL].p, 1);
            else
                *buf++ = meNLCHAR;
        }
        else if (cc == '\r')
        {
            if (level != 0)
                *buf++ = '\r';
        }
        else if (cc == '\t')
            *buf++ = ' ';
        else
        {
            if(cc == 0xff)
            {
                if((cc = *strp++) == '\0')
                    break ;
                if(cc == 0x01) 
                    cc = '\0' ;
            }
            *buf++ = cc;
        }
    }
    /* Return the delta length of the string */
    return buf - bbuf;
}
/*
 * expandTitleText
 * Expand the title text into the appropriate form
 */
static void
addFormatedLine (LINE **head, LINE **tail,
                 uint8 *buf, int noLines, uint8 *strp, int filter)
{
    uint8 *bhead = buf;
    uint8 *p = strp;

    while (--noLines >= 0)
    {
        int ii;
        uint8 cc;
        uint8 tbuf [MAXBUF];

        /* Get the new lines out */
        if (strp != NULL)
        {
            ii = 0 ;
            while(ii < printer.param [mePI_MLEFT].l)
                tbuf[ii++] = ' ' ;
            for (; (cc = *p) != '\0'; /* NULL */)
            {
                p++;
                if (cc == meNLCHAR)
                    break;
                /* Filter the character if required. */
                if (filter == 0)
                    tbuf [ii++] = cc;
                else
                    ii += doTranslation (&tbuf[ii], &cc, 1);
            }
        }
        else
            ii = 0;
        tbuf [ii] = '\0';

        /* Add the start of line code */
        buf = bhead + expandText (bhead, printer.param [mePS_SOL].p, 1);
        /* Expand the header text and append to the composition buffer */
        buf = buf + expandText (buf, tbuf, 0);
        /* Add the end of line code */
        buf = buf + expandText (buf, printer.param [mePS_EOL].p, 1);
        addComposedLine (head, tail, bhead, buf - bhead);
    }
}

#if HILIGHT
static int
printSetScheme(meSCHEME col, uint8 *buff)
{
    meSCHEME ts ;
    meSTYLE ss, cs ;
    int sore, ii, fstDiff, len=0 ;
    
    /* If the scheme is not registered then there is no colour change
     * to perform */
    ts = col / meSCHEME_STYLES ;
    /* Get the scheme out of print table */
    ss = (ts >= printer.schemeNum) ? 0 : printer.scheme[ts];
    if(printCurrScheme != meSCHEME_INVALID)
    {
        ts = printCurrScheme / meSCHEME_STYLES ;
        cs = (ts >= printer.schemeNum) ? 0 : printer.scheme[ts];
        /* loop through finding the first componant thats different */
        for(fstDiff=0 ; fstDiff<fontCount ; fstDiff++)
        {
            if(((ss & fontMasks[0][fstDiff]) != (cs & fontMasks[0][fstDiff])) ||
               ((ss & fontMasks[1][fstDiff]) != (cs & fontMasks[1][fstDiff])))
                break ;
        }
        sore = 1 ;
    }
    else
    {
        sore = 0 ;
        fstDiff = 0 ;
    }
    printCurrScheme = col ;
    
    if(fstDiff == fontCount)
        /* same characteristics, nothing to change */
        return 0 ;
    
    for(;;sore--)
    {
        ii = (sore) ? fontCount-1:fstDiff ;
        for(;;)
        {
            /* Always want to set the fore and back-ground colors (col 0 may not be black!) */
            if(fontStrlen[sore][ii] < 0)
            {
                int32 cno, cc, rr, gg, bb ;
                char *str ;
                if(fontMasks[sore][ii] & meSTYLE_FCOLOR)
                    cno = meStyleGetFColor(ss) ;
                else
                    cno = meStyleGetBColor(ss) ;
                cc = printer.color[cno] ;
                rr = mePrintColorGetRed(cc) ;
                gg = mePrintColorGetGreen(cc) ;
                bb = mePrintColorGetBlue(cc) ;
                str = meTParm((char *)fontStrings[sore][ii],cno,rr,gg,bb) ;
                if(buff != NULL)
                    meStrcpy(buff+len,str) ;
                len += strlen(str) ;
            }
            else if(((sore == 1) && (cs & fontMasks[sore][ii])) ||
                    ((sore == 0) && (ss & fontMasks[sore][ii])) )
            {
                if(buff != NULL)
                    meStrcpy(buff+len,fontStrings[sore][ii]) ;
                len += fontStrlen[sore][ii] ;
            }
            if(sore)
            {
                if(ii == fstDiff)
                    break ;
                ii-- ;
            }
            else if(++ii == fontCount)
                return len ;
        }
    }
}
#endif

/*
 * Render the page into a line store list
 */
static LINE *
composePage (int f)
{
    LINE *head = NULL;
    LINE *tail = NULL;
    uint8 buf [1024*5];                 /* Heafty line store !! */
    uint8 *p;                           /* Pointer to the buffer */
    int xx;                             /* Page column iterator */
    int yy;                             /* Page row iterator */
    int ll;                             /* Lines on page iterator */
    int ii;
    LINE *lp;                           /* Pointer to the line */

    /* Handle the end of file case */
    if (f)
    {
        if (f < 0)
        {
            /* End of file. */
            if (printer.param [mePS_EOF].p != NULL)
            {
                p = buf + expandText (buf, printer.param [mePS_EOF].p, 1);
                addComposedLine (&head, &tail, buf, p - buf);
            }
            return head;
        }
        else
        {
            /* Start of file */
            p = buf ;
            if (printer.param [mePS_SOF].p != NULL)
            {
                p += expandText (p, printer.param [mePS_SOF].p, 1);
            }
#if HILIGHT
            /* set the default bg-color */
            if(printer.param[mePS_BGCOL].p != NULL)
            {
                int32 cno, cc, rr, gg, bb ;
                char *str ;
                
                cno = printTextScheme / meSCHEME_STYLES ;
                cno = (cno >= printer.schemeNum) ? 0 : printer.scheme[cno];
                cno = meStyleGetBColor(cno) ;
                cc = printer.color[cno] ;
                rr = mePrintColorGetRed(cc) ;
                gg = mePrintColorGetGreen(cc) ;
                bb = mePrintColorGetBlue(cc) ;
                str = meTParm((char *)printer.param[mePS_BGCOL].p,cno,rr,gg,bb) ;
                meStrcpy(p,str) ;
                p += strlen(str) ;
            }
            /* set the scheme to the default text */
            p += printSetScheme(printTextScheme,p) ;
#endif
            if(p != buf)
                addComposedLine (&head, &tail, buf, p - buf);
            return head;
        }
    }

    /* Add the start of page marker */
    if (printer.param [mePS_SOP].p != NULL)
    {
        p = buf + expandText (buf, printer.param [mePS_SOP].p, 1);
        addComposedLine (&head, &tail, buf, p - buf);
    }

    /* Render the top margin and the header */
    if (printer.param [mePI_MTOP].l)
        addFormatedLine (&head, &tail, buf, printer.param [mePI_MTOP].l, NULL, 0);
    if (printer.pNoHeaderLines)
        addFormatedLine (&head, &tail, buf, printer.pNoHeaderLines, printer.param [mePS_HEADER].p, 1);

    /* Iterate over the number of page rows */
    for (yy = 0; yy < printer.param [mePI_ROWS].l; yy++)
    {
        /* Iterate over the lines on the page */
        for (ll = 0; ll < printer.param [mePI_PAGEY].l; ll++)
        {
            /* Add the start of line code */
            p = buf + expandText (buf, printer.param [mePS_SOL].p, 1);

            /* Add the leading margin */
            for (ii = printer.param [mePI_MLEFT].l; --ii >= 0; *p++ = ' ')
                /* NULL */;

            /* Iterate over the number of page columns */
            for (xx = 0;;)
            {
                /* Go to the start of the line in the buffer */
                ii = ((yy * printer.param [mePI_COLS].l + xx) * printer.param [mePI_PAGEY].l) + ll;
                lp = lforw (gbp->b_linep);
                while (--ii >= 0)
                {
                    if ((lp = lforw (lp)) == gbp->b_linep)
                    {
                        lp = NULL;
                        break;
                    }
                }

                /* Process the line */
                if (lp != NULL)
                {
                    /* Add the continuation marker - we have already done the line numbers */
                    if (((lp->l_flag & LNCHNG) == 0) && (printer.pLineNumDigits > 0))
                    {
                        for (ii = printer.pLineNumDigits-2; --ii >= 0; *p++ = ' ')
                            /* NULL */;
                        p += expandText (p, printer.param [mePS_SCONT].p, 1);
                        *p++ = ' ' ;
                    }

                    /* Concatinate the composed line information */
                    {
                        uint8 cc;
                        uint8 *str;

                        str = lp->l_text;
                        while ((cc = *str++) != '\0')
                        {
                            if (cc == 255)
                            {
                                if ((cc=*str++) == 0x01)
                                    cc = 0;
                                else if (cc == 0x02)
                                    cc=*str++;
                            }
                            *p++ = cc;
                        }
                    }
                }
                else
                {
                    /* NULL line. Pad the line with spaces */
                    ii = printer.param [mePI_PAGEX].l;
                    if (printer.pLineNumDigits > 0)
                        ii += printer.pLineNumDigits ;
                    while (--ii >= 0)
                        *p++ = ' ';
                }

                /* End of the line reached ?? */
                if (++xx >= printer.param [mePI_COLS].l)
                    break;

                /* Add a new column */
                p += expandText (p, printer.param [mePS_VSEP].p, 1);
            }

            /* Add the end of line marker and push the buffer */
            if (printer.param [mePI_STRIP].l)
            {
                while ((p - buf) > 0)
                {
                    char cc;
                    if (((cc = *(p-1)) == ' ') || (cc == '\t'))
                        p--;
                    else
                        break;
                }
            }
            p += expandText (p, printer.param [mePS_EOL].p, 1);
            addComposedLine (&head, &tail, buf, p - buf);
        }

        /* Add the horizontal separator if required */
        if (yy+1 < printer.param [mePI_ROWS].l)
        {
            /* Add the start of line code */
            p = buf + expandText (buf, printer.param [mePS_SOL].p, 1);

            /* Add the leading margin */
            for (ii = printer.param [mePI_MLEFT].l; --ii >= 0; *p++ = ' ')
                /* NULL */;

            for (xx = printer.param [mePI_COLS].l; --xx >= 0; /* NULL */)
            {
                int nn;

                /* Work out the width - length of text plus line # width */
                nn = printer.param [mePI_PAGEX].l + printer.pLineNumDigits;
                while (--nn >= 0)
                    p += expandText (p, printer.param [mePS_HSEP].p, 1);

                /* If there is another column then add the vertical crossover
                 * separator. */
                if (xx > 0)
                    p += expandText (p, printer.param [mePS_XSEP].p, 1);
            }

            /* Add the end of line marker and push the buffer */
            p += expandText (p, printer.param [mePS_EOL].p, 1);
            addComposedLine (&head, &tail, buf, p - buf);
        }
    }

    /* Render the footer and the separator between the footer and the textual
     * page */
    if (printer.pNoFooterLines)
        addFormatedLine (&head, &tail, buf, printer.pNoFooterLines, printer.param [mePS_FOOTER].p, 1);
    
    /* Add the end of line marker and push the buffer */
    if (printer.param [mePS_EOP].p != NULL)
    {
        p = buf + expandText (buf, printer.param [mePS_EOP].p, 1);
        addComposedLine (&head, &tail, buf, p - buf);
    }
    return head;
}

/*
 * dumpToInternal
 * Write the line into the iternal line list
 */
static void
dumpToInternal (LINE **headp, LINE **tailp, LINE *lp)
{
    /* Check for NULL case - this should not happen */
    if (lp == NULL)
        return;

    /* Link to the head */
    if (*tailp == NULL)
        *headp = lp;
    else
    {
        lforw(*tailp) = lp;             /* Attach head to end of list */
        lback(lp) = *tailp;
    }

    /* Fix up the tail */
    while (lforw(lp) != NULL)
        lp = lforw(lp);
    *tailp = lp;
}

/*
 * dumpToFile
 * Write the composed lines into the file
 */
static void
dumpToFile (FILE *fp, LINE *lp)
{
    /* Dump the line to file */
    while (lp != NULL)
    {
        LINE *tlp;                      /* Temp line store */

        tlp = lp;
        lp = lforw (lp);

        fwrite (tlp->l_text, 1, (int)(tlp->l_used), fp);
#if TESTPRINT
        {
            FILE *fp2;
            if ((fp2 = fopen ("spool2.out","ab")) != NULL)
            {
                fwrite (tlp->l_text, 1, (int)(tlp->l_used), fp2);
                fclose (fp2);
            }
        }
#endif
        meFree (tlp);
    }
}

/*
 * dumpToBuffer
 * Write the composed lines into a buffer. We have to un-translate the
 * lines here. This is not particularly efficient since we have already
 * translated (nil) characters to '\0's, now we have to expand them again !!
 * However this is not really a problem, just a bit of a pain !!.
 */
static void
dumpToBuffer (BUFFER *bp, LINE *lp)
{
    int ii;
    char *p;

    while (lp != NULL)
    {
        LINE *tlp;                      /* Temp line store */
        char cc;
        int len;
        int jj, kk;

        /* Advance the line pointer */
        tlp = lp;
        lp = lforw (lp);

        /* Determine how many '\0's are in the string */
        jj = 0;
        while (jj != tlp->l_used)
        {
            LINE *nlp;
            char *q;
            
            for (kk = jj, ii = tlp->l_used - jj, len = 0, p = (char *) &tlp->l_text [jj]; --ii >= 0; /* NULL */)
            {
                jj++;
                if ((cc = *p++) == '\0')
                    len += 2;
                else if (cc == meNLCHAR)
                    break;
                else 
                    len++;
            }
            
            /* Construct a new line for the buffer and translate out any 0's */
            nlp = lalloc (len);
            for (p = (char *) &tlp->l_text[kk], q = (char *) nlp->l_text; --len >= 0; /* NULL */)
            {
                if ((cc = *p++) == '\0')
                {
                    *q++ = (char)0xff;
                    *q++ = (char)0x01;
                }
                else if (cc == meNLCHAR)
                    break;
                else
                    *q++ = cc;
            }
                
            /* Link the new line to the end of the buffer. */
            lforw(lback(bp->b_linep)) = nlp;
            lback(nlp) = lback(bp->b_linep);
            lback(bp->b_linep) = nlp;
            lforw(nlp) = bp->b_linep;
            bp->elineno += 1;
        }
        /* Free off the old line and assign the converted line. */
        meFree (tlp);
    }
}

static void
printLinkLine (BUFFER *bp, LINE *nlp, uint16 lno)
{
    bp->b_linep->l_bp->l_fp = nlp;
    nlp->l_bp = bp->b_linep->l_bp;
    bp->b_linep->l_bp = nlp;
    nlp->l_fp = bp->b_linep;
    if (lno)
        nlp->l_flag &= ~LNCHNG;
    else
        printer.pLineNo++;              /* Next line */
}

#define PRTSIZ 2048
#if HILIGHT
static int
printAddLine (BUFFER *bp, LINE *lp, VIDEO *vps)
#else
static int
printAddLine (BUFFER *bp, LINE *lp)
#endif
{
#if HILIGHT
    meSCHEME  scheme ;
#endif
    uint16    noColChng ;
    HILBLOCK *blkp;
    LINE     *nlp;
    uint16    len, wid, ii, jj, kk, ll;

    /* Render the line to get the colour information */
#if HILIGHT
    if (vps[0].hilno)
    {
        noColChng = hilightLine (vps);
        blkp = hilBlock + 1;
        wid = blkp[noColChng-1].column;
    }
    else
#endif
    {
        wid = renderLine (lp->l_text,lp->l_used,0);
        noColChng = 0;
    }

    /* Commence rendering the line */
    len = 0;
    ii  = 0;
    for(;;)
    {
        /* Get the length of the text */
        ll = (uint16) printer.param[mePI_PAGEX].l;
        kk = wid-len ;
        if ((printer.param [mePI_FLAGS].l & PFLAG_ENBLTRUNCC))
        {
            if ((kk = wid-len) >= ll)
                kk = ll - 1 ;    /* Normalise to print length */
        }
        else if (kk > ll)
            kk = ll ;   /* Normalise to print length */

        /* Determine the length of all of the colour change strings. For
         * all of the colour changes to the end of the line, add the length
         * of storage of each to our line total (ll) */
        for (jj=0; jj<noColChng; jj++)
        {
#if HILIGHT
            scheme = blkp[jj].scheme ;
            ll += printSetScheme(scheme,NULL) ;
#endif
            if (blkp[jj].column > len+kk)
                break;
        }
#if HILIGHT
        /* add the reseting of the current scheme */
        ll += printSetScheme(printTextScheme,NULL) ;
#endif
        /* Insert line numbers if required. We insert line numbers at the
         * initial expansion so that we do not need to remember where we
         * are later. However we do not add the start of line continuation
         * markers until later (i.e. a line that continues on the next line)
         * This basically saves some space and is easy to deal with later */
        if ((ii == 0) && (printer.pLineNumDigits > 0))
            ll += (uint16) printer.pLineNumDigits ;
        ll += getTranslationLen (disLineBuff+len,kk) - kk;

        /* Construct a new composition line */
        if ((nlp=lalloc (ll)) == NULL)
            return (FALSE);
        printLinkLine (bp,nlp,ii++);

        /* Write the line numbers into the buffer if required */
        if ((nlp->l_flag & LNCHNG) && (printer.pLineNumDigits > 0))
        {
            char lnobuf [40];
            ll = (uint16) printer.pLineNumDigits ;
            sprintf (lnobuf, "% 20d ", printer.pLineNo);
            memcpy (nlp->l_text, &lnobuf [21-printer.pLineNumDigits], ll);
        }
        else
            ll = 0;

        /* Copy the line into the buffer */
        if (noColChng == 0)
        {
            /* No colour changes - simply copy the line */
            ll += doTranslation (nlp->l_text+ll, disLineBuff+len, kk);
        }
        else
        {
            /* Propogate the colour change strings into the print buffer
             * whenever the colour changes. */
            for (jj = 0; jj < kk; /* NULL*/)
            {
#if HILIGHT
                /* Get the colour change string */
                ll += printSetScheme(blkp->scheme,nlp->l_text+ll) ;
#endif
                if (blkp->column > len+kk)
                {
                    /* Off the end of the line !! */
                    ll += doTranslation (nlp->l_text+ll,
                                         disLineBuff+len+jj,
                                         kk-jj);
                    jj = kk;
                }
                else
                {
                    /* All fits !! */
                    ll += doTranslation (nlp->l_text+ll,
                                         disLineBuff+len+jj,
                                         blkp->column-len-jj);
                    jj = blkp->column-len;
                    blkp++;
                }
            }
#if HILIGHT
            /* add the reseting of the current scheme */
            ll += printSetScheme(printTextScheme,nlp->l_text+ll) ;
#endif
        }
        len += kk ;
        /* Handle the end of the line */
        if (len >= wid)
        {
            /* Pad the end of the line with spaces. */
            len = printer.param[mePI_PAGEX].l-kk ;
            memset(nlp->l_text+ll,' ',len);
            nlp->l_text[ll+len] = '\0';
            break ;
        }
        if ((printer.param [mePI_FLAGS].l & PFLAG_ENBLTRUNCC))
            /* Line continues - add the line continuation character */
            ll += expandText (nlp->l_text+ll, printer.param [mePS_ECONT].p, 1);
        nlp->l_text[ll] = '\0';
    }
    printer.pNoLines += ii;
    return (TRUE);
}

/*
 * printSection
 * Print a section of the buffer
 */

static int
printSection (WINDOW *wp, long sLineNo, long numLines, LINE *sLine, LINE *eLine, int nn)
{
    time_t clock;                       /* Time in machine format. */
    BUFFER *bp;                         /* Composition buffer */
    WINDOW *dwp;                        /* Destination window */
    BUFFER *dbp = NULL;                 /* Destination buffer */
    FILE *fp = NULL;                    /* Output file stream */
    LINE *lp, *nlp;                     /* Local line pointers */
    LINE *clp;                          /* Composed page list */
    LINE *ihead = NULL;                 /* Internal destination header */
    LINE *itail = NULL;                 /* Internal destination trailer */
    int status = TRUE;                  /* Return status */
    uint8 fname [MAXBUF];               /* File name buffer */
    int ii;                             /* Local loop counter. */

#if HILIGHT
    VIDEO          vps[2];
#endif
    /* Work out the line number range. */
    for (numLines += sLineNo, printer.pLineNumDigits = 1; numLines > 0; printer.pLineNumDigits++)
        numLines /= 10;

    /* Initialise the printer. */
    if (printInit (TRUE, nn) != TRUE)
        return ABORT;
    if(nn < 0)
        return TRUE ;
        
    /* Get the clock out and initialise */
    clock = time (0);	                 /* Get system time */
    printer.ptime = localtime (&clock);  /* Get time frame */
    printer.pLineNo = sLineNo;
    printer.pPageNo = 0;
    printer.pNoLines = 0;

    /* Determine the destination */
    switch (printer.pDestination)
    {
    case PDEST_INTERNAL:
        break;
    case PDEST_BUFFER:
        if ((dwp = wpopup (printer.param [mePS_BUFFER].p,(BFND_CREAT|BFND_CLEAR|WPOP_USESTR))) == NULL)
            return mlwrite(MWABORT,(uint8 *)"[Failed to create print buffer]") ;
        dbp = dwp->w_bufp;
        break;
    case PDEST_COMLINE:
    case PDEST_FILE:
        if ((printer.param [mePS_FILE].p == NULL) || (*printer.param [mePS_FILE].p == '\0'))
            mkTempName (fname, NULL, "prt");
        else
            meStrcpy (fname,printer.param [mePS_FILE].p);

        /* Endevour to open the file */
        if ((fp = fopen ((char *)fname, "wb")) == NULL)
            return mlwrite (MWABORT,(uint8 *)"Unable to open printer spool file %s", fname);
        break;
    }
    
    /* Shuffle the isDisplayable test so it actually tests isPrint */
    for(ii=0 ; ii<256 ; ii++)
        charMaskTbl1[ii] = (charMaskTbl1[ii] & ~(CHRMSK_DISPLAYABLE|CHRMSK_PRINTABLE)) |
              ((charMaskTbl1[ii] & CHRMSK_DISPLAYABLE) ? CHRMSK_PRINTABLE:0) |
              ((charMaskTbl1[ii] & CHRMSK_PRINTABLE) ? CHRMSK_DISPLAYABLE:0) ;
    /* Initialise the printer */
    if ((bp = createBuffer((uint8 *)"")) == NULL)
    {
        status = mlwrite (MWABORT,(uint8 *)"Unable to setup Printer");
        goto quitEarly;
    }
    /* Set up the buffer modes correctly so that we save the codes correctly */
    meModeSet (bp->b_mode,MDAUTO);
    meModeClear (bp->b_mode,MDCRLF);
    meModeClear (bp->b_mode,MDCTRLZ);
    
#if HILIGHT
    /* Clear the 2nd entry just in case the hilighting is not defined.
     * The 2nd entry is used as a continuation when hilighting is active. */
    vps[1].wind = vps[0].wind = wp ;
    vps[1].hilno = vps[0].hilno = wp->w_bufp->hiLight ;
    vps[1].bracket = vps[0].bracket = NULL ;
    vps[1].flag = vps[0].flag = 0 ;
    /* get the default text scheme */
    if (vps[0].hilno)
        printTextScheme = hilights[vps[0].hilno]->scheme ;
    else
        printTextScheme = wp->w_bufp->scheme ;
    printCurrScheme = meSCHEME_INVALID ;
#endif
    printer.pLinesPerPage = (printer.param[mePI_PAGEY].l *
                             printer.param[mePI_ROWS].l *
                             printer.param[mePI_COLS].l);
    /* Construct the start of file */
    if ((clp = composePage (1)) != NULL)
    {
        if (fp)                         /* File device */
            dumpToFile (fp, clp);
        else if (dbp)                   /* Buffer device */
            dumpToBuffer (dbp, clp);
        else                            /* Internal device */
            dumpToInternal (&ihead, &itail, clp);
    }

    do {
        printer.pPageNo++;              /* Next page */
        mlwrite (0,(uint8 *)"Printing page %d ..", printer.pPageNo);

        /* Construcr a page worth of formatted data */
        while ((sLine != eLine) && (printer.pNoLines < printer.pLinesPerPage))
        {
#if HILIGHT
            vps[0].line = sLine;
            if (printAddLine (bp,sLine,vps) != TRUE)
#else
            if (printAddLine (bp,sLine) != TRUE)
#endif
            {
                status = mlwrite (MWABORT,(uint8 *)"Internal error: Cannot add new prin line");
                goto quitEarly;
            }
            sLine = lforw (sLine);
#if HILIGHT
            /* Propogate the next line information back into the
             * current line */
            vps[0].flag = vps[1].flag;
            vps[0].hilno = vps[1].hilno;
            vps[0].bracket = vps[1].bracket;
#endif
        }

        /* Compose the formatted data into a page */
        gbp = bp;
        gpbp = wp->w_bufp;
        if ((clp = composePage (0)) == NULL)
        {
            status = mlwrite (MWABORT,(uint8 *)"Internal error: Cannot compose printer page");
            goto quitEarly;
        }

        /* Output the formatted page to the approipriate destination */
        if (fp)                         /* File device */
            dumpToFile (fp, clp);
        else if (dbp)                   /* Buffer device */
            dumpToBuffer (dbp, clp);
        else                            /* Internal device */
            dumpToInternal (&ihead, &itail, clp);

        /* Delete the formatted page data */
        if(printer.pNoLines)
        {
            lp = lforw (bp->b_linep);
            for (ii = 0; ii < printer.pLinesPerPage; ii++)
            {
                nlp = lforw (lp);
                meFree (lp);
                lp = nlp;
                if (--printer.pNoLines == 0)
                    break;
            }
            /* Fix up the pointers */
            lforw (bp->b_linep) = lp;
            lback (lp) = bp->b_linep;
        }
    } while (sLine != eLine);

    /* Construct the end of file */
    if ((clp = composePage (-1)) != NULL)
    {
        if (fp)
            dumpToFile (fp, clp);
        else if (dbp)                   /* Buffer device */
            dumpToBuffer (dbp, clp);
        else                            /* Internal device */
            dumpToInternal (&ihead, &itail, clp);
    }

    /* Close the file descriptor */
    if (fp)
    {
        fclose (fp);
        fp = NULL;
    }
                  
    /* If this is a command line then print the file. */
    switch (printer.pDestination)
    {
    case PDEST_COMLINE:
        {
            uint8 cmdLine [MAXBUF+20];
            uint8 *p, *q;
            uint8 cc;
            
            /* Expand the command line */
            p = printer.param [mePS_CMD].p;
            q = cmdLine;
            if ((p == NULL) || (*p == '\0'))
            {
                mlwrite (MWPAUSE|MWABORT,(uint8 *)"No command line specified. Print file %s", fname);
                goto quitEarly;
            }
            
            while ((cc = *p++) != '\0')
            {
                if (cc == '%')
                {
                    /* %f - insert the filename */
                    if ((cc = *p) == 'f')
                    {
                        uint8 *r = fname;
                        while ((*q = *r++) != '\0')
                            q++;
                        p++;                /* Skip the '%f' */
                    }
                    /* % at end of string, insert a % */
                    else if (cc == '\0')
                        *q++ = '%';
                    /* Otherwise insert the trailing character */
                    else
                    {
                        *q++ = cc;
                        p++;            /* Skip the character */
                    }
                }
                else
                    *q++ = cc;
            }
            *q = '\0';                  /* Terminate the string */
                    
            /* Call me's system to print the file. Me's version is
             * used cause its return value is correctly evaluated and
             * on unix a " </dev/null" safety arg is added to the cmdLine
             */
            if((status=doShellCommand(cmdLine)) == TRUE)
                status = (resultStr[0] == '0') ? TRUE:FALSE ;
            if(status != TRUE)
                mlwrite(MWABORT,(uint8 *)"[Failed to print file %s]",fname);
            break;
        
        case PDEST_INTERNAL:
#ifdef _WIN32
            status = WinPrint ((curbp->b_fname != NULL) ? curbp->b_fname
                               : (curbp->b_bname != NULL) ? curbp->b_bname
                               :(uint8 *)"UNKNOWN source",
                               ihead);
            ihead = NULL;               /* Reset - never returned */
            break;
#endif
        case PDEST_BUFFER:
            dbp->b_dotp = lforw (dbp->b_linep);
            dbp->b_doto = 0;
            dbp->line_no = 0;
            resetBufferWindows (dbp);
            break;
        }
    }
    
    /* Shut down the system */
quitEarly:
    /* Shut down the file */
    if (fp)
        fclose (fp);
    /* Kill off the buffer */
    if (bp)
        zotbuf (bp, TRUE);
    /* Free off internal list */
    while (ihead != NULL)
    {
        itail = ihead;
        ihead = lforw (ihead);
        meFree (itail);
    }
    /* Reshuffle the isDisplayable test back again */
    for(ii=0 ; ii<256 ; ii++)
        charMaskTbl1[ii] = (charMaskTbl1[ii] & ~(CHRMSK_DISPLAYABLE|CHRMSK_PRINTABLE)) |
              ((charMaskTbl1[ii] & CHRMSK_DISPLAYABLE) ? CHRMSK_PRINTABLE:0) |
              ((charMaskTbl1[ii] & CHRMSK_PRINTABLE) ? CHRMSK_DISPLAYABLE:0) ;

    /* Put out end of file indicator. */
    if (status == TRUE)
        mlwrite (0,(uint8 *)"Printing - Done. %d page(s).", printer.pPageNo);
    return status;


}

int
printRegion (int f, int n)
{
    LINE *startLine, *endLine;
    long startLineNo;
    long numLines;

    if (curwp->w_markp == NULL)
        return noMarkSet ();

    if ((startLineNo=curwp->line_no-curwp->mlineno) == 0)
        return TRUE;
    numLines = abs (startLineNo);
    if (startLineNo < 0)
    {
        startLineNo = curwp->line_no;
        startLine = curwp->w_dotp;
        endLine = curwp->w_markp;
    }
    else
    {
        startLineNo = curwp->mlineno;
        startLine = curwp->w_markp;
        endLine = curwp->w_dotp;
    }
    return printSection (curwp, startLineNo, numLines, startLine, endLine, n);
}

int
printBuffer (int f, int n)
{
    return printSection (curwp, 0, curbp->elineno,
                         lforw (curbp->b_linep), curbp->b_linep,n);
}

#endif
