/****************************************************************************
 *
 *  			Copyright 1996 Jon Green.
 *                         All Rights Reserved
 *
 *
 *  System        : 
 *  Module        : 
 *  Object Name   : $RCSfile: rtf.c,v $
 *  Revision      : $Revision: 1.2 $
 *  Date          : $Date: 2004-02-07 19:29:49 $
 *  Author        : $Author: jon $
 *  Last Modified : <040207.1927>
 *
 *  Description	
 *
 *  Notes
 *
 *  History
 *	
 *  $Log: not supported by cvs2svn $
 *  Revision 1.1  2000/10/21 14:31:24  jon
 *  Import
 *
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

static const char rcsid[] = "@(#) : $Id: rtf.c,v 1.2 2004-02-07 19:29:49 jon Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include "hmldrv.h"

extern hmldct *rtfOpen (char *logo);

static int paragraphMode = 0;           /* Paragraph mode */
static int rtfvIndent = 0;              /* Indent distance */
static int rtfvFirstIndent = 0;         /* Indent on first line of paragraph */
static int rtfvFont = 0;
static int rtfvSize = 0;
static int rtfvPara = 0;                /* Paragraph mode */

#define ALWAYS_FLUSH    0               /* Disable flush on flie write. */

#define FULL_INDENT 360                 /* Full indent distance. */
#define HALF_INDENT 180                 /* Half indent distance. */
#define LINE_HIEGHT 100                 /* twips height */ 
#define BULLET_INDENT 200               /* Indent of a bullet */

#define PARA_DISABLE     0
#define PARA_FLUSH       0
#define PARA_CODE_BIT    0x001
#define PARA_TITLE_BIT   0x002
#define PARA_PAGE_BIT    0x004
#define PARA_ENABLE_BIT  0x008
#define PARA_BLANK_BIT   0x010
#define PARA_LIST_BIT    0x020
#define PARA_CURRENT_BIT 0x040
#define PARA_LAST_BIT    0x080
#define PARA_OPEN_BIT    0x100

#define PARA_LAST        PARA_LAST_BIT
#define PARA_CURRENT     PARA_CURRENT_BIT
#define PARA_ENABLE      PARA_ENABLE_BIT
#define PARA_NORMAL      PARA_ENABLE
#define PARA_CODE        (PARA_CODE_BIT|PARA_ENABLE)
#define PARA_TITLE       (PARA_TITLE_BIT|PARA_ENABLE)
#define PARA_PAGE        (PARA_PAGE_BIT|PARA_ENABLE)
#define PARA_LIST        (PARA_LIST_BIT|PARA_ENABLE)
#define PARA_BLANK       (PARA_BLANK_BIT|PARA_ENABLE)
#define PARA_OPEN        (PARA_OPEN_BIT)

#if 0
/* Supposed to be standard Windows */
#define SS_FONT     2                   /* SS Section font type */
#define SS_SIZE     24                  /* SS Section font size */
#define SH_FONT     2                   /* SH Section font type */
#define SH_SIZE     24                  /* SH Section font size */
#define TX_FONT     2                   /* Text section font type */
#define TX_SIZE     24                  /* Text section font size */
#define CS_FONT     4                   /* Code section font type */
#define CS_SIZE     20                  /* Code section font size */
#define TL_FONT     2                   /* Title line font type */
#define TL_SIZE     28                  /* Title line font size */
#else
/* Smaller font */
#define SS_FONT     2                   /* SS Section font type */
#define SS_SIZE     20                  /* SS Section font size */
#define SH_FONT     2                   /* SH Section font type */
#define SH_SIZE     24                  /* SH Section font size */
#define TX_FONT     2                   /* Text section font type */
#define TX_SIZE     20                  /* Text section font size */
#define CS_FONT     4                   /* Code section font type */
#define CS_SIZE     18                  /* Code section font size */
#define TL_FONT     2                   /* Title line font type */
#define TL_SIZE     28                  /* Title line font size */

#endif

static char *rtfHeader[] =
{
    "{\\rtf1\\ansi \\seff0\\deflang1024",
    "",
    "{\\fonttbl",
    "{\\f0\\froman Times New Roman;}",
    "{\\f1\\froman Symbol;}",
    "{\\f2\\fswiss Arial;}",
    "{\\f3\\froman Serif;}",
    "{\\f4\\fmodern Courier New;}",
    "}",
    "{\\colortbl;",
    "\\red0\\green0\\blue0;",
    "\\red0\\green0\\blue255;",
    "\\red0\\green255\\blue0;",
    "\\red255\\green0\\blue0;",
    "\\red255\\green255\\blue255;",
    "}",
    NULL
};

static char *rtfFooter = "}";

typedef struct {
    int ss_font;                        /* SS Section font type */
    int ss_size;                        /* SS Section font size */
    int sh_font;                        /* SH Section font type */
    int sh_size;                        /* SH Section font size */
    int tx_font;                        /* Text section font type */
    int tx_size;                        /* Text section font size */
    int cs_font;                        /* Code section font type */
    int cs_size;                        /* Code section font size */
    int tl_font;                        /* Title line font type */
    int tl_size;                        /* Title line font size */
} fontType;

static fontType *rtfFontType;

static fontType winFont =
{
    2,                                  /* SS Section font type */
    20,                                 /* SS Section font size */
    2,                                  /* SH Section font type */
    24,                                 /* SH Section font size */
    2,                                  /* Text section font type */
    20,                                 /* Text section font size */
    4,                                  /* Code section font type */
    18,                                 /* Code section font size */
    2,                                  /* Title line font type */
    28                                  /* Title line font size */
};

#if 0
static fontType manFont =
{
    0,                                  /* SS Section font type */
    20,                                 /* SS Section font size */
    0,                                  /* SH Section font type */
    24,                                 /* SH Section font size */
    0,                                  /* Text section font type */
    20,                                 /* Text section font size */
    4,                                  /* Code section font type */
    16,                                 /* Code section font size */
    0,                                  /* Title line font type */
    28                                  /* Title line font size */
};
#endif

void
rtfMakeFileName (char *s, char **result)
{
    char    *drive;
    char    *path;
    char    *base;

    if ((splitFilename (unifyFilename (s), &drive, &path, &base, NULL)) != 0)
        uFatal ("Cannot decopose filename [%s]\n", s);
    
    bufFree (*result);
    *result = strdup (makeFilename (drive, path, base, "rtf"));
}

static void
rtfEof (void)
{
    fprintf (fpHml, "%c", 0x1a);
    fflush (fpHml);
}

static void
rtfEol (void)
{
    fprintf (fpHml, "\r\n");
#if ALWAYS_FLUSH
    fflush (fpHml);
#endif
}

static void
rtfStr (char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    vfprintf(fpHml, format, ap);
#if ALWAYS_FLUSH
    fflush (fpHml);
#endif
    va_end(ap);
}

static void
rtfIndent (int i)
{
    if (i > 0)
        while (--i >= 0)
            rtfvIndent += FULL_INDENT;
    else
        while (++i <= 0)
            rtfvIndent -= FULL_INDENT;
}

static void
rtfPutPara (int mode)
{
    static int paraOpen = 0;
    static int lastMode = 0;
    
    char *justStr;
    char *paraStr;
    char *titlStr;
    char *pageStr;
    
    if (mode == PARA_OPEN)
    {
        paraOpen = 1;
        return;
    }
    
    if (paraOpen) {
        paraOpen = 0;
        rtfEol ();
    }
    
    if (mode == PARA_CURRENT)
        mode = rtfvPara;
    else if (mode == PARA_LAST)
        mode = lastMode;
    
    if ((mode & PARA_ENABLE_BIT) != 0)
    {
        char frstStr [30];
        char indtStr [30];
        
        if (rtfvFirstIndent == 0)
            frstStr [0] = '\0';
        else
            sprintf (frstStr, "\\fi%d", rtfvFirstIndent);
        
        sprintf (indtStr, "\\li%d", rtfvIndent);
        
        justStr = ((mode & PARA_CODE_BIT) == 0) ? "\\qj" : "";
        pageStr = ((mode & PARA_PAGE_BIT) != 0) ? "\\page" : "\\pard";
        titlStr = ((mode & PARA_TITLE_BIT) != 0)? "\\keepn" : "";
        paraStr = ((mode & PARA_CODE_BIT) != 0) ? "\\keep" : ""; 
        
        rtfStr ("\\par %s%s%s %s%s%s \\f%d\\fs%d ",
                pageStr, paraStr, titlStr,
                justStr, frstStr, indtStr,
                rtfvFont, rtfvSize);
        paraOpen = 1;
    }
    if (mode & PARA_BLANK_BIT)
    {
        rtfEol ();
        paraOpen = 0;
    }
    lastMode = mode;
}

static void
rtfPara (void)
{
    rtfvFont = rtfFontType->tx_font;
    rtfvSize = rtfFontType->tx_size;
    rtfPutPara (PARA_NORMAL);           /* First is blank !! */
    rtfPutPara (PARA_NORMAL);           /* Next is data */
}

static void
rtfBreak (void)
{
    rtfPutPara (PARA_LAST);
}

static void
rtfHead (void)
{
    ;
}

static void
rtfFont (int mode)
{
    static int fontState = 0;

    /* Reset the font state */
    if (mode == -1) {
        fontState = 0;
        return;
    }

    /* Turn off any highlighting modes if required. */
    if (((mode  & FONT_BOLD) == 0) && ((fontState & FONT_BOLD) != 0)) {
        fontState &= ~FONT_BOLD;
        rtfStr ("}");
    }
    if (((mode  & FONT_ITALIC) == 0) && ((fontState & FONT_ITALIC) != 0)) {
        fontState &= ~FONT_ITALIC;
        rtfStr ("}");
    }
    if (((mode  & FONT_MONO) == 0) && ((fontState & FONT_MONO) != 0)) {
        fontState &= ~FONT_MONO;
        rtfStr ("}");
    }

    /* Turn on any highlighting modes if required. */
    if (((mode  & FONT_BOLD) != 0) && ((fontState & FONT_BOLD) == 0)) {
        fontState |= FONT_BOLD;
        rtfStr ("{\\b ");
    }
    if (((mode  & FONT_ITALIC) != 0) && ((fontState & FONT_ITALIC) == 0)) {
        fontState |= FONT_ITALIC;
        rtfStr ("{\\i ");
    }
    if (((mode  & FONT_MONO) != 0) && ((fontState & FONT_MONO) == 0)) {
        fontState |= FONT_MONO;
        rtfStr ("\\f4 ");
    }
}

static int
rtfParagraphMode (int paragraph)
{
    if (paragraph >= 0)
        paragraphMode = paragraph;
    return (paragraphMode);
}

static void
rtfFormatStr (char *format, ...)
{
    int space = 0;
    char c;
    char buffer [4096];
    char *s;
    va_list ap;

    if (format == NULL)
        return;

    va_start (ap, format);
    vsprintf (buffer, format, ap);
    va_end (ap);
    s = buffer;
    
    while ((c = *s++) != '\0')
    {
        switch (c) {
        case '{':
        case '}':
        case '\\':
            rtfStr ("\\%c", c);
            break;
        case '\t':
        case ' ':
            if ((space != 0) && (paragraphMode == 0))
                break;
            rtfStr ("%c", c);
            space = 1;
            break;
        case CHAR_USPACE:
            rtfStr (" ");
            break;
        default:
            rtfStr ("%c", c);
            break;
        }
        if ((c != ' ') || (c == '\t'))
            space = 0;
    }
}

static void
rtfLine (void)
{
    /* Do nothing */;
}

static void
rtfEnd (void)
{
    rtfPara ();                         /* Force indents off, back to normal */
    rtfPutPara (PARA_FLUSH);            /* Flush out !! */
}

static void
rtfBullet (char *format, ...)
{
    va_list ap;

    char buffer [1024];

    va_start (ap, format);
    vsprintf (buffer, format, ap);
    va_end (ap);
    
    rtfIndent (1);
    rtfvFirstIndent = -BULLET_INDENT;
    rtfPutPara (PARA_NORMAL);
    rtfStr ("{\\f1\\b\\fs24\\'b7}");    /* A bullet - believe it or not !! */
/*    rtfStr ("\\tab ");*/
    rtfStr (" ");                       /* Some reason you don't use tabs  ?? */
    rtfFormatStr ("%s", buffer);
    rtfIndent (-1);
    rtfvFirstIndent = 0;
    rtfPutPara (PARA_FLUSH);
}

static void
rtfTitle (int level, char *format, ...)
{
    static int level2level [] = {1, 2};
    va_list ap;
    int i;
    char buffer [1024];

    va_start (ap, format);
    vsprintf (buffer, format, ap);
    va_end (ap);

    if (level < 0)
        i = (level = -level);
    else
        i = level2level [level];
    
    if (level == 1)
    {
        rtfvFont = rtfFontType->tl_font;
        rtfvSize = rtfFontType->tl_size;
    }
    else
    {        
        rtfvFont = rtfFontType->tx_font;
        rtfvSize = rtfFontType->tx_size;
    }
    
    rtfPara ();                         /* Puts blank then para */
    rtfFont (FONT_BOLD);
    rtfFormatStr ("%s", buffer);
    rtfFont (FONT_NORMAL);
    rtfPutPara (PARA_FLUSH);
}

static void
rtfListOpen (void)
{
/*    rtfPutPara (PARA_BLANK);*/
    rtfvIndent += 280;
    rtfvFirstIndent = -280;
    rtfvPara = PARA_LIST;
}

static void
rtfListClose (void)
{
    rtfPutPara (PARA_FLUSH);
    rtfvIndent -= 280;
    rtfvFirstIndent = 0;
    rtfvPara = PARA_NORMAL;
}

static void
rtfListTag (void)
{
    rtfPutPara (PARA_NORMAL);
}

static void
rtfListData (void)
{
    rtfStr (" ");
}

static void
rtfXref (char *pageName, char *format, ...)
{
    char *s;
    char buffer [256];
    va_list ap;

    va_start(ap, format);
    vsprintf(buffer, format, ap);
    va_end(ap);
    
    /* Add literal text */
    rtfStr ("{\\strike ");
    rtfFormatStr ("%s", buffer);
    rtfStr ("}");
    
    /* Add hypertext link */
    s = hmlMakeTopicHyperName (NULL, pageName, NULL);
    rtfStr ("{\\v %s}", s);
    bufFree (s);
}

static void
rtfReference (char *title, int level, Args *args)
{
    int i;

    if (title != NULL)
    {
        if (args->argv == NULL)
            return;
        else
            rtfTitle (level, title);
    }

    rtfIndent (1);
    rtfPara ();
    for (i = 0; i < args->argc; i += 2) {
        rtfXref (hmlXrefPageName ("%s%04d",
                                  args->argv [i],
                                  atoi (args->argv [i+1])),
                 "%s #%d", args->argv [i], atoi (args->argv [i+1]));
        rtfFormatStr ((args->argc > (i+2)) ? ", " : "." );
    }
    rtfIndent (-1);
    rtfPutPara (PARA_FLUSH);
}

static void
get2arguments (char *source, char **destp1, char **destp2)
{
    char *dest1 = NULL;
    char c;

    /* Collect the first string */
    while (((c = *source) != '\0') && (c != ' '))
    {
        dest1 = bufChr (dest1, c);
        source++;
    }
    
    /* Skip the white space. */
    while ((c = *source) == ' ')
        source++;
    
    /* Fill in the return arguments */
    if (destp1 != NULL)
        *destp1 = dest1;
    if (c == '\0')
        source = NULL;
    if (destp2 != NULL)
        *destp2 = bufStr (NULL, source);
    
}

static void
rtfDescription (char *title, int level, Args *args)
{
#define PARA_MODE_REGULAR      0        /* Regular paragraph mode. */
#define PARA_MODE_CODE         1        /* Code paragraph */
#define PARA_MODE_LITERAL      2        /* Literal section */
#define PARA_MODE_BULLET       4        /* Bulleted section */
#define PARA_MODE_TITLE        8        /* Titled section */    
    
    int mode = PARA_MODE_REGULAR;       /* Paragraph mode */
    int commandId;                      /* Command identifier word */
    int i;
    int indent = 0;
    char *curLine;
              
    /*
     * Place title for description section.
     * We run the description with a minumim indent of 1.
     */
    if (title != NULL)
    {
        if (args->argv == NULL)
            return;
        else
            rtfTitle (level, title);
    }
    rtfIndent (1);
    
    /* If no description is provided add one - indicates no description */
    if (args->argv == NULL) {
        rtfPara ();
        rtfFormatStr ("No description provided.");
        rtfIndent (-1);
        return;
    }

    /*
     * Process the lines. For each line determine if any control character is 
     * presemt amd formats the lines accordingly.
     */
    
    for (i = 0; i < args->argc; i++)
    {
        curLine = args->argv [i];
        commandId = ((int)((unsigned char)(*curLine)) & 0xff);
        
        uDebug (1, ("RTF Description [%s]\n", curLine));
        
        switch (commandId)
        {
        case CHAR_FORMAT_INDENT:        /* Indent start */
            rtfIndent (1);
            continue;
        case CHAR_FORMAT_EINDENT:       /* Indent end */
            rtfIndent (-1);
            continue;
        case CHAR_FORMAT_CODE:          /* Code block start */
            rtfvPara = PARA_CODE;
            rtfParagraphMode (1);
            indent = hmlCodeTidy (&args->argv [i]);
            rtfvFont = rtfFontType->cs_font;
            rtfvSize = rtfFontType->cs_size;
            rtfvIndent += FULL_INDENT;
            mode = PARA_MODE_CODE;
            rtfPutPara (PARA_CODE);     /* Blank first line */
            continue;
        case CHAR_FORMAT_ECODE:         /* Code block end */
            rtfPutPara (PARA_FLUSH);
            rtfvIndent -= FULL_INDENT;
            rtfParagraphMode (0);
            mode = PARA_MODE_REGULAR;
            rtfvFont = rtfFontType->tx_font;
            rtfvSize = rtfFontType->tx_size;
            continue;
        case CHAR_FORMAT_TITLE:         /* Title text */
            mode = PARA_MODE_TITLE;
            continue;
        case CHAR_FORMAT_ETITLE:        /* End title text */
            rtfPutPara (PARA_FLUSH);
            mode = PARA_MODE_REGULAR;
            continue;
        case CHAR_FORMAT_BULLET:        /* Bulleted list start */
/*            rtfIndent (1);*/
            rtfPutPara (PARA_BLANK);    /* Blank line first */
            mode = PARA_MODE_BULLET;
            continue;
        case CHAR_FORMAT_EBULLET:       /* Bulleted list end */
/*            rtfIndent (-1);*/
            rtfPutPara (PARA_FLUSH);
            mode = PARA_MODE_REGULAR;
            continue;
        case CHAR_FORMAT_LITERAL:       /* Literal list start */
            rtfPutPara (PARA_BLANK);    /* Blank line first */
            rtfvFont = rtfFontType->tx_font;
            rtfvSize = rtfFontType->tx_size;
            mode = PARA_MODE_LITERAL;
            continue;
        case CHAR_FORMAT_ELITERAL:      /* Literal list end */
            rtfPutPara (PARA_FLUSH);
            mode = PARA_MODE_REGULAR;
            continue;
        case CHAR_FORMAT_ROMAN:
            rtfFormatStr (" %s", &curLine[1]);
            continue;
        
        case CHAR_FORMAT_BOLD:          /* Bold enclosure */
        case CHAR_FORMAT_BOLDP:         /* Bold enclosure - new paragraph */
        case CHAR_FORMAT_ITALIC:        /* Italic enclosure */
        case CHAR_FORMAT_ITALICP:       /* Italic enclosure - new paragraph */
        case CHAR_FORMAT_MONO:          /* Mono enclosure */
        case CHAR_FORMAT_MONOP:         /* Mono enclosure - new paragraph */
            {
                int  fontMode = 0;      /* Format mode for fonts */
                
                /* Start of new paragraph - get start of paragraph correct
                 * We want a new paragraph when in literal mode ALWAYS.*/
                if ((commandId & CHAR_FORMAT_PARA) || (mode & PARA_MODE_LITERAL))
                {
                    if ((mode & PARA_MODE_LITERAL) != 0)
                        rtfPutPara (PARA_NORMAL); 
                    else if ((mode & PARA_MODE_BULLET) != 0)
                        rtfBullet ("%s", "");
                    else
                        rtfPara ();
                } 
                else /*if ((mode & PARRA_MODE_BULLET) == 0)*/
                    rtfStr (" ");
                
                /* Get the format string type */
                if ((commandId & ~CHAR_FORMAT_PARA) == CHAR_FORMAT_BOLD)
                    fontMode = FONT_BOLD;
                else if ((commandId & ~CHAR_FORMAT_PARA) == CHAR_FORMAT_ITALIC)
                    fontMode = FONT_ITALIC;
                else
                    fontMode = FONT_MONO;
                
                /* Print the string */
                rtfFont (fontMode);
                rtfFormatStr ("%s", &curLine[1]);
                rtfFont (0);
            }
            
            rtfPutPara (PARA_OPEN);
            if (mode == PARA_MODE_LITERAL)
                rtfPutPara (PARA_FLUSH);
#if 0            
            else
                rtfEol ();
#endif            
            continue;
        
        case CHAR_FORMAT_FILE:          /* File reference */
        case CHAR_FORMAT_FILEP:         /* File reference - new paragraph */
        case CHAR_FORMAT_FTP:           /* FTP reference */
        case CHAR_FORMAT_FTPP:          /* FTP reference - new paragraph */
        case CHAR_FORMAT_HTTP:          /* HTTP address */
        case CHAR_FORMAT_HTTPP:         /* HTTP address - new paragraph */
        case CHAR_FORMAT_MAILTO:        /* Mail to address */
        case CHAR_FORMAT_MAILTOP:       /* Mail to address - new paragraph */
            {
                char *s1, *s2;
                
                /* Start of new paragraph - get start of paragraph correct */
                if ((commandId & CHAR_FORMAT_PARA) || (mode & PARA_MODE_LITERAL))
                {
                    if ((mode & PARA_MODE_LITERAL) != 0)
                        rtfPutPara (PARA_NORMAL);
                    else if ((mode & PARA_MODE_BULLET) != 0)
                        rtfBullet ("%s", "");
                    else
                        rtfPara ();
                }
                else
                    rtfStr (" ");
                
                /* Process the argument. */
                get2arguments (&curLine[1], &s1, &s2);
                if (s2 != NULL)
                    rtfFormatStr ("%s ", s2);
                
                rtfStr ("{\\cf4 ");     /* Make this item red */
                if ((commandId & ~CHAR_FORMAT_PARA) == CHAR_FORMAT_FTP)
                    rtfFormatStr ("%s", "ftp://");
                else if ((commandId & ~CHAR_FORMAT_PARA) == CHAR_FORMAT_FILE)
                    rtfFormatStr ("%s", "file:");
                else if ((commandId & ~CHAR_FORMAT_PARA) == CHAR_FORMAT_MAILTO)
                    rtfStr ("%s", "mailto://");
                else if ((commandId & ~CHAR_FORMAT_PARA) == CHAR_FORMAT_HTTP)
                    rtfStr ("%s", "http://");
                else
                    uFatal ("Cannot get here - HTML case bounds failure!\n");
                rtfFormatStr ("%s", s1);
                rtfStr ("}");           /* Restore colour scheme */
                
                bufFree (s1);           /* Free allocated return buffers */
                bufFree (s2);
            }
            
            /* Adjust modes if required - and terminate paragraph */
            rtfPutPara (PARA_OPEN);
            if (mode == PARA_MODE_LITERAL)
                rtfPutPara (PARA_FLUSH);
#if 0            
            else
                rtfEol ();
#endif            
            continue;
        
        default:
            break;
        }

        if (mode == PARA_MODE_BULLET)
            rtfBullet ("%s", curLine);
        else if (mode == PARA_MODE_CODE)
        {
            int j;
            
            rtfPutPara (PARA_CODE); 
            if (indent > 20)
            {
                uError ("Indent = %d\n", indent);
                indent = 4;
            }
            for (j = 0; j < indent; j++)
                rtfStr (" ");
            rtfFormatStr ("%s", curLine);
        }
        else if (mode == PARA_MODE_LITERAL)
        {
            rtfPutPara (PARA_NORMAL); 
            rtfFormatStr ("%s", curLine);
            rtfPutPara (PARA_FLUSH);
            
        }
        else if (mode == PARA_MODE_TITLE)
        {
            rtfPara ();
            rtfFont (FONT_BOLD);
            rtfFormatStr ("%s", curLine);
            rtfFont (FONT_NORMAL);
            rtfPutPara (PARA_FLUSH);
        }
        else 
        {
            rtfPara ();
            rtfFormatStr ("%s", curLine);
        }
    }
    rtfPutPara (PARA_FLUSH);
    rtfIndent (-1);

#undef  PARA_MODE_REGULAR               /* Regular paragraph mode. */
#undef  PARA_MODE_CODE                  /* Code paragraph */
#undef  PRAR_MODE_LITERAL               /* Literal section */
#undef  PARA_MODE_LITERAL_CONT          /* Continuing literal section */
#undef  PARA_MODE_BULLET                /* Bulleted section */
#undef  PARA_MODE_TITLE                 /* Titled section */    
}

static void
rtfClose (void)
{
    rtfPutPara (PARA_FLUSH);
    rtfStr ("%s", rtfFooter);
    rtfEol ();
    rtfEof ();
    fclose (fpHml);
}

static void
rtfDefinition (char *format, ...)
{
    va_list ap;
    char buffer [1024];
    char *s;

    va_start(ap, format);
    vsprintf(buffer, format, ap);
    va_end(ap);

    s = hmlMakeTopicHyperName (NULL, buffer, NULL);
    rtfStr ("#{\\footnote %s}", s);
    bufFree (s);
}


static void
rtfNumericFileHeader (char *item, int digit, char *name)
{
    char **p;

    rtfIndent (0);
    rtfvFont = rtfFontType->tl_font;
    rtfvSize = rtfFontType->tl_size;
    
    rtfPutPara (PARA_TITLE|PARA_PAGE);
    rtfHead();
    rtfStr ("${\\footnote %s%04d}", item, digit);
    rtfStr ("#{\\footnote %s%04d}", item, digit);
    
    /*
     * If the home page is specified then link into RTF file.
     */
    
    if (hmlHomePage != NULL)
        rtfStr ("{\\uldb \\{bml logo.bmp\\}}{\\v %%%s@%s.hlp}",
                hmlHomePage, hmlHomePage);
    
    /* Add the title to the top of the page */
    rtfFont (FONT_BOLD);
    rtfFormatStr (name, digit);
    rtfFont (FONT_NORMAL);

    /* Make the quick links */
    if ((p = hmlQuickStrs.argv) != NULL)
    {
        rtfEol();
        rtfStr ("\\line {\\fs%d ", 18);  /* rtfFont->tl_size */  /* 18 = 9pt */
        while (*p != NULL)
        {
            rtfXref (*p, "[%s]", *p);
            rtfStr (" ");               /* Add space */
            p++;
        }
        rtfStr ("}");
    }
    
    /* Blank line in regular text */
    rtfvFont = rtfFontType->tx_font;
    rtfvSize = rtfFontType->tx_size;
    rtfPutPara (PARA_BLANK);
    
    /* Horizontal rule */
    rtfLine ();
}

static void
rtfFileHeader (char *title, char *name)
{
    char *sname = NULL;
    char **p;

    rtfIndent (0);
    rtfvFont = rtfFontType->tl_font;
    rtfvSize = rtfFontType->tl_size;
    
    rtfPutPara (PARA_TITLE|PARA_PAGE);
    rtfHead();
    
    if (name != NULL)
        sname = hmlMakeTopicHyperName (NULL, name, NULL);
    else
        sname = hmlMakeTopicHyperName (NULL, title, NULL);
    rtfStr ("${\\footnote %s}", sname);
    rtfDefinition (sname);
    
    /*
     * If the home page is specified then link into RTF file.
     */
    
    if (hmlHomePage != NULL)
        rtfStr ("{\\uldb \\{bml logo.bmp\\}}{\\v %%%s@%s.hlp}",
                hmlHomePage, hmlHomePage);
    
    /* Add the title to the top of the page */
    rtfFont (FONT_BOLD);
    rtfFormatStr ("%s", title);
    rtfFont (FONT_NORMAL);
    
    /* Make the quick links */
    if ((p = hmlQuickStrs.argv) != NULL)
    {
        rtfEol();
        rtfStr ("\\line {\\fs%d ", 18);  /* rtfFont->tl_size */  /* 18 = 9pt */
        while (*p != NULL)
        {
            rtfXref (*p, "[%s]", *p);
            rtfStr (" ");               /* Add space */
            p++;
        }
        rtfStr ("}");
    }
    
    /* Blank line in regular text */
    rtfvFont = rtfFontType->tx_font;
    rtfvSize = rtfFontType->tx_size;
    rtfPutPara (PARA_BLANK);
    
    /* Horizontal rule */
    rtfLine ();
    bufFree (sname);
}

static void
rtfForm (int i)
{
    ;
}

static void
rtfFormField (char *type, char *name, char *value)
{
    ;
}


hmldct *
rtfOpen (char *logo)
{
    static hmldct rtfDriverTable =
    {
        rtfBreak,
        rtfBullet,
        rtfClose,
        rtfDefinition,
        rtfDescription,
        rtfEnd,
        rtfFileHeader,
        rtfFont,
        rtfFormatStr,
        rtfIndent,
        rtfLine,
        rtfNumericFileHeader,
        rtfPara,
        rtfReference,
        rtfStr,
        rtfTitle,
        rtfXref,
        rtfListOpen,
        rtfListClose,
        rtfListTag,
        rtfListData,
        rtfForm,
        rtfFormField,
    };
    
    int     i;
    
    /* Set up the rtf File header. */
    rtfFontType = &winFont;
    for (i = 0; rtfHeader [i] != NULL; i++) {
        rtfStr ("%s", rtfHeader[i]);
        rtfEol ();
    }
    
        /* Set up the logo name */
    if (logo == NULL)
        hmlLogo = "logo.bmp";
    else
        hmlLogo = logo;
    
    /* Return driver table to caller. */
    return (&rtfDriverTable);
}

