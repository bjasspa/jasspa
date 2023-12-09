/****************************************************************************
 *
 *  			Copyright 1996 Jon Green.
 *                         All Rights Reserved
 *
 *
 *  System        : 
 *  Module        : 
 *  Object Name   : $RCSfile: html.c,v $
 *  Revision      : $Revision: 1.2 $
 *  Date          : $Date: 2004-02-07 19:29:49 $
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
 *  Revision 1.1  2000/10/21 14:31:22  jon
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include "hmldrv.h"

extern hmldct *htmlOpen (char *logo);
extern char *wwwroot;

static int paragraphMode = 0;           /* Paragraph mode */

#define ALWAYS_FLUSH    0               /* Disable flush on flie write. */

void
htmlMakeFileName (char *s, char **result)
{
    char    *drive;
    char    *path;
    char    *base;

    if ((splitFilename (unifyFilename (s), &drive, &path, &base, NULL)) != 0)
        uFatal ("Cannot decopose filename [%s]\n", s);
    
    bufFree (*result);
    *result = strdup (makeFilename (drive, path, base, "html"));
}

static void
htmlEof (void)
{
#if 0
    fprintf (fpHml, "%c", 0x1a);       /* Add Ctrl-Z */
#endif
    fflush (fpHml);
    ;                                   /* Do nothing !! */
}

static void
htmlEol (void)
{
    fprintf (fpHml, "\n");
#if ALWAYS_FLUSH
    fflush (fpHml);
#endif
}

static void
htmlStr (char *format, ...)
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
htmlPara (void)
{
    htmlEol ();
    htmlStr ("<P>");
}

static void
htmlBreak (void)
{
    htmlEol ();
    htmlStr ("<BR>");
}

static void
htmlHead (void)
{
    htmlStr ("<HTML>");
    if (hmlIdent != NULL)
    {
        htmlEol ();
        htmlStr ("<!-- %s -->", hmlIdent);
    }
    htmlEol ();
    htmlStr ("<!-- %s -->", hmlVersion ());
    htmlEol ();
    htmlStr ("<!-- %s -->", uVersion ());
    if (hmlGenid != NULL)
    {
        htmlEol ();
        htmlStr ("<!-- %s -->", hmlGenid);
    }
}

static void
htmlFont (int mode)
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
        htmlStr ("</B>");
    }
    if (((mode  & FONT_ITALIC) == 0) && ((fontState & FONT_ITALIC) != 0)) {
        fontState &= ~FONT_ITALIC;
        htmlStr ("</I>");
    }
    if (((mode  & FONT_MONO) == 0) && ((fontState & FONT_MONO) != 0)) {
        fontState &= ~FONT_MONO;
        htmlStr ("</TT>");
    }

    /* Turn on any highlighting modes if required. */
    if (((mode  & FONT_BOLD) != 0) && ((fontState & FONT_BOLD) == 0)) {
        fontState |= FONT_BOLD;
        htmlStr ("<B>");
    }
    if (((mode  & FONT_ITALIC) != 0) && ((fontState & FONT_ITALIC) == 0)) {
        fontState |= FONT_ITALIC;
        htmlStr ("<I>");
    }
    if (((mode  & FONT_MONO) != 0) && ((fontState & FONT_MONO) == 0)) {
        fontState |= FONT_MONO;
        htmlStr ("<TT>");
    }
}

static int
htmlParagraphMode (int paragraph)
{
    if (paragraph >= 0)
        paragraphMode = paragraph;
    return (paragraphMode);
}

static void
htmlFormatStr (char *format, ...)
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
        case '<':
            htmlStr ("&lt;");
            break;
        case '>':
            htmlStr ("&gt;");
            break;
        case '&':
            htmlStr ("&amp;");
            break;
        case '\"':
            htmlStr ("&quot;");
            break;
        case '\t':
        case ' ':
            if ((space != 0) && (paragraphMode == 0))
                break;
            htmlStr ("%c", c);
            space = 1;
            break;
        case CHAR_USPACE:
            htmlStr (" ");
            break;
        default:
            htmlStr ("%c", c);
            break;
        }
        if ((c != ' ') || (c == '\t'))
            space = 0;
    }
}

static void
htmlLine (void)
{
    htmlEol ();
    htmlStr ("<HR>");
}

static void
htmlEnd (void)
{
    htmlEol ();
    htmlStr ("</BODY></HMTL>");
    htmlEol ();
}

static void
htmlBullet (char *format, ...)
{
    va_list ap;

    char buffer [1024];

    va_start (ap, format);
    vsprintf (buffer, format, ap);
    va_end (ap);

    htmlEol ();
    htmlStr ("<LI>");
    htmlFormatStr ("%s", buffer);
}

static void
htmlTitle (int level, char *format, ...)
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

    htmlEol ();
    htmlStr ("<H%d>", i);
    htmlFormatStr ("%s", buffer);
    htmlStr ("</H%d>", i);

    /* If we are at level 0 put out a rule */
    if (level == 0) {
        htmlLine ();
    }
}

static void
htmlListOpen (void)
{
    htmlEol ();
    htmlStr ("<DL COMPACT>");
}

static void
htmlListClose (void)
{
    htmlEol ();
    htmlStr ("</DL>");
}

static void
htmlListTag (void)
{
    htmlEol ();
    hmlStr ("<DT>");
}

static void
htmlListData (void)
{
    hmlStr ("<DD>");
}

static void
htmlIndent (int i)
{
    if (i > 0)
        while (--i >= 0)
            htmlStr ("<UL>");
    else
        while (++i <= 0)
            htmlStr ("</UL>");
}

static void
htmlXref (char *pageName, char *format, ...)
{
    char *s;
    char buffer [256];
    va_list ap;

    s = hmlMakeTopicHyperName (NULL, pageName, NULL);
    htmlEol ();
    htmlStr ("<A HREF=\"<<R%s<\">", s);

    va_start(ap, format);
    vsprintf(buffer, format, ap);
    va_end(ap);
    htmlFormatStr ("%s", buffer);

    htmlStr ("</A>");
    bufFree (s);
}

static void
htmlReference (char *title, int level, Args *args)
{
    int i;

    if (title != NULL)
    {
        if (args->argv == NULL)
            return;
        else
            htmlTitle (level, title);
    }

    htmlStr ("<UL>");
    htmlPara ();
    for (i = 0; i < args->argc; i += 2) {
        htmlXref (hmlXrefPageName ("%s%04d",
                                   args->argv [i],
                                   atoi (args->argv [i+1])),
                  "%s #%d", args->argv [i], atoi (args->argv [i+1]));
        htmlFormatStr ((args->argc > (i+2)) ? ", " : "." );
    }
    htmlStr ("</UL>");
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
        source = dest1;
    if (destp2 != NULL)
        *destp2 = bufNStr (NULL, source);
    
}

static void
htmlDescription (char *title, int level, Args *args)
{
#define PARA_MODE_REGULAR      0        /* Regular paragraph mode. */
#define PARA_MODE_CODE         1        /* Code paragraph */
#define PARA_MODE_LITERAL      2        /* Literal section */
#define PARA_MODE_LITERAL_CONT 3        /* Continuing literal section */
#define PARA_MODE_BULLET       4        /* Bulleted section */
#define PARA_MODE_TITLE        5        /* Titled section */    
    
    static char *leader[] =
    {
        "<P>",                          /* Mode 0 - Regular */
        "",                             /* Mode 1 - code section */
        "",                             /* Mode 2 - literal section */
        "<BR>",                         /* Mode 3 - literal section continue */
        "<LI>",                         /* Mode 4 - bullet section */
        "",                             /* Mode 5 - Title section */
    };

    int mode = PARA_MODE_REGULAR;       /* Regular mode is initial mode */
    int commandId;                      /* Command identifier word */
    int i;
    int indent = 0;
    char *curLine;                      /* Current line */
    /*
     * Place title for description section.
     * We run the description with a minumim indent of 1.
     */
    if (title != NULL)
    {
        if (args->argv == NULL)
            return;
        else
            htmlTitle (level, title);
    }
    htmlEol ();
    htmlStr ("<UL>");
    
    /* If no description is provided add one - indicates no description */
    if (args->argv == NULL) {
        htmlEol ();
        htmlStr ("<P>");
        htmlFormatStr ("No description provided.");
        htmlStr ("</UL>");
        return;
    }

    /*
     * Process the lines. For each line determine if any control character is 
     * presemt amd formats the lines accordingly.
     */
    
    for (i = 0; i < args->argc; i++)
    {
        curLine = args->argv [i];       /* Required for djgpp */
        commandId = ((int)((unsigned char)(*curLine)) & 0xff);
        
        uDebug (1, ("HTML Arg No [%d->%s]\n", i, (curLine == NULL) ? "nil" : curLine));
        uDebug (1, ("HTML Description [%d->%s]\n", commandId,
                    (curLine == NULL) ? "nil" : curLine));
        switch (commandId)
        {
        case CHAR_FORMAT_INDENT:        /* Indent start */
            htmlStr ("<UL>");
            continue;
        case CHAR_FORMAT_EINDENT:       /* Indent end */
            htmlStr ("</UL>");
            continue;
        case CHAR_FORMAT_CODE:          /* Code block start */
            htmlEol ();
            htmlIndent (1);
            htmlStr ("<PRE>");
            htmlParagraphMode (1);
            mode = PARA_MODE_CODE;
            indent = hmlCodeTidy (&args->argv [i]);
            continue;
        case CHAR_FORMAT_ECODE:         /* Code block end */
            htmlStr ("</PRE>");
            htmlIndent (-1);
            htmlParagraphMode (0);
            mode = PARA_MODE_REGULAR;
            continue;
        case CHAR_FORMAT_TITLE:         /* Title text */
            htmlEol ();
            htmlStr ("<P>");
            htmlFont (FONT_BOLD);
            mode = PARA_MODE_TITLE;
            continue;
        case CHAR_FORMAT_ETITLE:        /* End title text */
            htmlFont (0);
            mode = PARA_MODE_REGULAR;
            continue;
        case CHAR_FORMAT_BULLET:        /* Bulleted list start */
            htmlEol ();
            htmlStr ("<UL>");
            htmlEol ();
            htmlStr ("<P>");
            mode = PARA_MODE_BULLET;
            continue;
        case CHAR_FORMAT_EBULLET:       /* Bulleted list end */
            htmlStr ("</UL>");
            mode = PARA_MODE_REGULAR;
            continue;
        case CHAR_FORMAT_LITERAL:       /* Literal list start */
            htmlEol ();
            htmlStr ("<P>");
            mode = PARA_MODE_LITERAL;
            continue;
        case CHAR_FORMAT_ELITERAL:      /* Literal list end */
            mode = PARA_MODE_REGULAR;
            continue;
        case CHAR_FORMAT_ROMAN:
            htmlFormatStr (" %s", &curLine[1]);
            /* Adjust modes if required */
            if (mode == PARA_MODE_LITERAL)
                mode = PARA_MODE_LITERAL_CONT;
            continue;
        
        case CHAR_FORMAT_BOLD:          /* Bold enclosure */
        case CHAR_FORMAT_BOLDP:         /* Bold enclosure - new paragraph */
        case CHAR_FORMAT_ITALIC:        /* Italic enclosure */
        case CHAR_FORMAT_ITALICP:       /* Italic enclosure - new paragraph */
        case CHAR_FORMAT_MONO:          /* Mono enclosure */
        case CHAR_FORMAT_MONOP:         /* Mono enclosure - new paragraph */
            {
                int  fontMode = 0;      /* Format mode for fonts */
                
                if ((commandId & CHAR_FORMAT_PARA) ||
                    ((mode & (PARA_MODE_LITERAL|PARA_MODE_LITERAL_CONT)) != 0))
                {
                    htmlEol ();                 /* Add new paragraph marker... */
                    htmlStr (leader [mode]);    /* ... depending on mode. */
                    
                    /* Dereference command. */
                    commandId &= ~CHAR_FORMAT_PARA;
                }
                else
                    htmlStr (" ");
                
                /* Get the format string type */
                if (commandId == CHAR_FORMAT_BOLD)
                    fontMode = FONT_BOLD;
                else if (commandId == CHAR_FORMAT_ITALIC)
                    fontMode = FONT_ITALIC;
                else
                    fontMode = FONT_MONO;
                
                /* Print the string */
                htmlFont (fontMode);
                htmlFormatStr ("%s", &curLine[1]);
                htmlFont (0);
            }
            
            /* Adjust modes if required */
            if (mode == PARA_MODE_LITERAL)
                mode = PARA_MODE_LITERAL_CONT;
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
                
                if ((commandId & CHAR_FORMAT_PARA) ||
                    ((mode & (PARA_MODE_LITERAL|PARA_MODE_LITERAL_CONT)) != 0))
                {
                    htmlEol ();                 /* Add new paragraph marker... */
                    htmlStr (leader [mode]);    /* ... depending on mode. */
                    
                    /* Dereference command. */
                    commandId &= ~CHAR_FORMAT_PARA;
                }
                else
                    htmlStr (" ");

                /* Process the argument. */
                get2arguments (&curLine[1], &s1, &s2);
                htmlEol ();
                if (commandId == CHAR_FORMAT_FTP)
                    htmlStr ("<A HREF=\"ftp://%s\">", s1);
                else if (commandId == CHAR_FORMAT_FILE)
                    htmlStr ("<A HREF=\"file:%s\">", s1);
                else if (commandId == CHAR_FORMAT_MAILTO)
                    htmlStr ("<A HREF=\"mailto:%s\">", s1);
                else if (commandId == CHAR_FORMAT_HTTP)
                    htmlStr ("<A HREF=\"http://%s\">", s1);
                else
                    uFatal ("Cannot get here - HTML case bounds failure!\n");
                htmlFormatStr ("%s", s2);
                htmlStr ("</A>");
                bufFree (s1);
                bufFree (s2);
            }
            
            /* Adjust modes if required */
            if (mode == PARA_MODE_LITERAL)
                mode = PARA_MODE_LITERAL_CONT;
            continue;
        
        default:
            break;
        }
        
        /* Print the indent to a code section */
        if (mode == PARA_MODE_CODE)
        {
            int j;
            
            for (j = 0; j < indent; j++)
                htmlStr (" ");
            htmlFormatStr ("%s", curLine);
            htmlEol ();
        }
        else
        {
            /* Print the leaders if required. */
            if (leader [mode][0] != '\0')
            {
                htmlEol ();
                htmlStr (leader [mode]);
            }
            
            htmlFormatStr ("%s", curLine);
            
            /* Adjust modes if required */
            if (mode == PARA_MODE_LITERAL)
                mode = PARA_MODE_LITERAL_CONT;
        }
    }
    
    /* End of the description. Reduce the indentation */
    htmlStr ("</UL>");
    if (level != 0)
        htmlLine ();

#undef  PARA_MODE_REGULAR               /* Regular paragraph mode. */
#undef  PARA_MODE_CODE                  /* Code paragraph */
#undef  PRAR_MODE_LITERAL               /* Literal section */
#undef  PARA_MODE_LITERAL_CONT          /* Continuing literal section */
#undef  PARA_MODE_BULLET                /* Bulleted section */
#undef  PARA_MODE_TITLE                 /* Titled section */    
}

static void
htmlClose (void)
{
    htmlEof ();
    fclose (fpHml);
}

static void
htmlDefinition (char *format, ...)
{
    va_list ap;
    char buffer [1024];
    char *s;

    va_start(ap, format);
    vsprintf(buffer, format, ap);
    va_end(ap);

    s = hmlMakeTopicHyperName (NULL, buffer, NULL);
    htmlStr ("<<D%s<", s);
    bufFree (s);
}


static void
htmlNumericFileHeader (char *item, int digit, char *name)
{
    char **p;

    htmlStr ("<<F%s%04d.html<", item, digit);
    htmlHead();
    htmlEol ();
    htmlStr ("<HEAD><TITLE>");
    htmlStr (name, digit);
    htmlStr ("</TITLE></HEAD>");
    htmlEol ();
    htmlStr ("<<D%s%04d<", item, digit);
    htmlStr ("<BODY>");

    /* Add the title to the top of the page */
    htmlEol ();
    htmlStr ("<H1>");
    
    /*
     * If the home page is non-null then add an external reference to the
     * home page. 
     */
    
    if (hmlHomePage != NULL)
    {
        htmlStr ("<A HREF=\"<<P<%s.html\">"
                 "<IMG SRC=\"<<P<%s/%s\" ALIGN=BOTTOM ALT=\"[%s]\">"
                 "</A>",
                 hmlHomePage,
                 hmlHomePage,
                 hmlLogo,
                 hmlHomePage);
        htmlEol ();
    }
    htmlFormatStr (name, digit);
    htmlStr ("</H1>");

    /* Make the quick links */
    if ((p = hmlQuickStrs.argv) != NULL)
    {
        htmlEol ();
        htmlStr ("<FONT SIZE=2>");
        while (*p != NULL)
            htmlXref (*p, "[%s]", *p), p++;
        htmlEol ();
        htmlStr ("</FONT><BR>");
    }

    /* Horizontal rule */
    htmlLine ();
}

static void
htmlForm (int i)
{
    if (i == 1)
        htmlStr ("<form method=post form=\"application/x-www-form-urlencoded\" action=\"%s/cgi-bin/bugcgi.exe\">", wwwroot);
    else
        htmlStr ("</form>");
    htmlEol ();
}

static void
htmlFormField (char *type, char *name, char *value)
{
    htmlStr ("<input ");
    if (type != NULL)
        htmlStr ("type=%s ", type);
    if (name != NULL)
        htmlStr ("name=\"%s\" ", name);
    if (value != NULL)
        htmlStr ("value=\"%s\" ", value);
    htmlStr (">");
    htmlEol ();
}

static void
htmlFileHeader (char *title, char *name)
{
    char *sname = NULL;
    char **p;

    if (name != NULL)
        sname = hmlMakeTopicHyperName (NULL, name, NULL);

    if (sname != NULL)
        htmlStr ("<<F%s.html<", sname);
    else
        htmlStr ("<<A.html<");
    htmlHead ();
    htmlEol ();
    htmlStr ("<HEAD><TITLE>");
    htmlFormatStr ("%s", title);
    htmlStr ("</TITLE></HEAD>");
    htmlEol ();
    htmlStr ("<BODY>");
    if (sname == NULL)
    {
        sname = hmlMakeTopicHyperName (NULL, title, NULL);
        htmlDefinition (sname);
        sname = bufFree (sname);

    }
    else
        htmlDefinition (sname);

    /* Add the title to the top of the page */
    htmlEol ();
    htmlStr ("<H1>");
    
    /*
     * If the home page is non-null then add an external reference to the
     * home page. 
     */
    
    if (hmlHomePage != NULL)
    {
        htmlStr ("<A HREF=\"<<P<%s.html\">"
                 "<IMG SRC=\"<<P<%s/%s\" ALIGN=BOTTOM ALT=\"[%s]\">"
                 "</A>",
                 hmlHomePage,
                 hmlHomePage,
                 hmlLogo,
                 hmlHomePage);
        htmlEol ();
    }
    
    htmlFormatStr ("%s",title);
    htmlStr ("</H1>");

    /* Make the quick links */
    if ((p = hmlQuickStrs.argv) != NULL)
    {
        htmlEol ();
        htmlStr ("<FONT SIZE=2>");
        while (*p != NULL)
            htmlXref (*p, "[%s]", *p), p++;
        htmlEol ();
        htmlStr ("</FONT>");
        htmlEol ();
        htmlStr ("<BR>");
    }

    /* Horizontal rule */
    htmlLine ();
    bufFree (sname);
}

hmldct *
htmlOpen (char *logo)
{
    static hmldct htmlDriverTable =
    {
        htmlBreak,
        htmlBullet,
        htmlClose,
        htmlDefinition,
        htmlDescription,
        htmlEnd,
        htmlFileHeader,
        htmlFont,
        htmlFormatStr,
        htmlIndent,
        htmlLine,
        htmlNumericFileHeader,
        htmlPara,
        htmlReference,
        htmlStr,
        htmlTitle,
        htmlXref,
        htmlListOpen,
        htmlListClose,
        htmlListTag,
        htmlListData,
        htmlForm,
        htmlFormField,
    };

    /* Set up the logo name */
    if (logo == NULL)
        hmlLogo = "logo.gif";
    else
        hmlLogo = logo;
    
    return (&htmlDriverTable);
}
