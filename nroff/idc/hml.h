/****************************************************************************
 *
 *  			Copyright 1996 Jon Green.
 *                         All Rights Reserved
 *
 *
 *  System        : 
 *  Module        : 
 *  Object Name   : $RCSfile: hml.h,v $
 *  Revision      : $Revision: 1.1 $
 *  Date          : $Date: 2000-10-21 14:31:22 $
 *  Author        : $Author: jon $
 *  Last Modified : <990831.2338>
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

#ifndef __HML_H__
#define __HML_H__

#include <utils.h>

/* Special Characters */
#define CHAR_USPACE '\001'              /* Unpaddable Space */

/* Font classes */
#define FONT_NORMAL   0x00000000        /* Regular font */
#define FONT_BOLD     0x00000001        /* Bold mode */
#define FONT_ITALIC   0x00000002        /* Italic mode */
#define FONT_MONO     0x00000004        /* Mono mode */

/* Paragraph Modes */
#define PARA_SPACE    0x00000010        /* Remove spaces in paragraph */

#define CHAR_FORMAT_CODE     128        /* Start of code section */
#define CHAR_FORMAT_ECODE    129        /* End of code section */
#define CHAR_FORMAT_INDENT   130        /* Start of indent section */
#define CHAR_FORMAT_EINDENT  131        /* End of indent section */
#define CHAR_FORMAT_BULLET   132        /* Start of bullet section */
#define CHAR_FORMAT_EBULLET  133        /* End of bullet section */
#define CHAR_FORMAT_LITERAL  134        /* Start of literal section */
#define CHAR_FORMAT_ELITERAL 135        /* End of literal section */
#define CHAR_FORMAT_TITLE    136        /* Start of title section */
#define CHAR_FORMAT_ETITLE   137        /* End of title section */
/*
 * The following define inline changes to items.
 * Note that the 'paragraph' items indicate a blank line before the 
 * font change. The regular items MUST be even; the paragraph items are 
 * +1 and odd. 
 */
#define CHAR_FORMAT_PARA     1
#define CHAR_FORMAT_BOLD     138        /* Bold item */
#define CHAR_FORMAT_BOLDP    (CHAR_FORMAT_BOLD|CHAR_FORMAT_PARA)
#define CHAR_FORMAT_ITALIC   140        /* Italic item */
#define CHAR_FORMAT_ITALICP  (CHAR_FORMAT_ITALIC|CHAR_FORMAT_PARA)
#define CHAR_FORMAT_MONO     142        /* Monospaced item */
#define CHAR_FORMAT_MONOP    (CHAR_FORMAT_MONO|CHAR_FORMAT_PARA)
#define CHAR_FORMAT_MAILTO   144        /* Mail to item */
#define CHAR_FORMAT_MAILTOP  (CHAR_FORMAT_MAILTO|CHAR_FORMAT_PARA)
#define CHAR_FORMAT_FILE     146        /* File item */
#define CHAR_FORMAT_FILEP    (CHAR_FORMAT_FILE|CHAR_FORMAT_PARA)
#define CHAR_FORMAT_HTTP     148        /* Http item */
#define CHAR_FORMAT_HTTPP    (CHAR_FORMAT_HTTP|CHAR_FORMAT_PARA)
#define CHAR_FORMAT_FTP      150        /* ftp Item */
#define CHAR_FORMAT_FTPP     (CHAR_FORMAT_FTP|CHAR_FORMAT_PARA)
#define CHAR_FORMAT_ROMAN    152        /* Return to roman. */

#define HML_HEADING_1         0
#define HML_HEADING_2         1

#define HML_FORMAT_HTML       0
#define HML_FORMAT_RTF        1

/*
 * Define the driver control table.
 */

typedef struct st_hmlDriverControlTable
{
    void (*breakFunc) (void);
    void (*bulletFunc)(char *format, ...);
    void (*closeFunc)(void);
    void (*definitionFunc)(char *format, ...);
    void (*descriptionFunc)(char *title, int level, Args *args);
    void (*endFunc) (void);
    void (*fileHeaderFunc) (char *title, char *name);
    void (*fontFunc)(int mode);
    void (*formatStrFunc)(char *format, ...);
    void (*indentFunc)(int i);
    void (*lineFunc)(void);
    void (*numericFileHeaderFunc)(char *item, int digit, char *name);
    void (*paraFunc)(void);
    void (*referenceFunc)(char *title, int level, Args *args);
    void (*strFunc)(char *format, ...);
    void (*titleFunc)(int level, char *format, ...);
    void (*xrefFunc)(char *pageName, char *format, ...);
    void (*listOpenFunc)(void);
    void (*listCloseFunc)(void);
    void (*listTagFunc)(void);
    void (*listDataFunc)(void);
    void (*formFunc)(int i);
    void (*formFieldFunc)(char *type, char *name, char *value);
} hmldct;

extern hmldct *hmldctp;

extern char *hmlLogo;

#define hmlBreak          (hmldctp->breakFunc) 
#define hmlBullet         (hmldctp->bulletFunc) 
#define hmlLangClose      (hmldctp->closeFunc) 
#define hmlDefinition     (hmldctp->definitionFunc) 
#define hmlDescription    (hmldctp->descriptionFunc) 
#define hmlEnd            (hmldctp->endFunc) 
#define hmlFileHeader     (hmldctp->fileHeaderFunc) 
#define hmlFont           (hmldctp->fontFunc) 
#define hmlFormatStr      (hmldctp->formatStrFunc) 
#define hmlIndent         (hmldctp->indentFunc) 
#define hmlLine           (hmldctp->lineFunc) 
#define hmlNumericFileHeader (hmldctp->numericFileHeaderFunc) 
#define hmlPara           (hmldctp->paraFunc) 
#define hmlReference      (hmldctp->referenceFunc) 
#define hmlStr            (hmldctp->strFunc)
#define hmlTitle          (hmldctp->titleFunc) 
#define hmlXref           (hmldctp->xrefFunc) 
#define hmlListOpen       (hmldctp->listOpenFunc) 
#define hmlListClose      (hmldctp->listCloseFunc) 
#define hmlListTag        (hmldctp->listTagFunc) 
#define hmlListData       (hmldctp->listDataFunc) 
#define hmlForm           (hmldctp->formFunc) 
#define hmlFormField      (hmldctp->formFieldFunc) 

extern char *hmlXrefPageName (char *format, ...);
extern void hmlGenId (char *s);
extern void hmlIdentity (char *s);
extern void hmlAddQuick (char *s);
extern void hmlAddHome  (char *s);
extern void hmlHead (void);
extern int  hmlCodeTidy (char **cp);
extern void hmlClose (void);
extern void hmlOpen (int format, char *s, char *l);
extern char *hmlVersion (void);
extern int  hmlFormat (void);

#endif /* __HML_H__ */
