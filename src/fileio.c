/*
 *	SCCS:		%W%		%G%		%U%
 *
 *	Last Modified :	<000907.1414>
 * 
 *****************************************************************************
 * 
 * Modifications to the original file by Jasspa. 
 * 
 * Copyright (C) 1988 - 1999, JASSPA 
 * The MicroEmacs Jasspa distribution can be copied and distributed freely for
 * any non-commercial purposes. The MicroEmacs Jasspa Distribution can only be
 * incorporated into commercial software with the expressed permission of
 * JASSPA.
 * 
 ****************************************************************************/

/*
 * The routines in this file read and write ASCII files from the disk. All of
 * the knowledge about files are here. A better message writing scheme should
 * be used.
 */

#define	__FILEIOC			/* Define program name */

#include "emain.h"

#if (defined _UNIX) || (defined _DOS) || (defined _WIN32)
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

#ifdef _WIN32
#define ffpInvalidVal INVALID_HANDLE_VALUE
#else
#define ffpInvalidVal NULL
#endif

/*
 * char-mask lookup tables
 * 
 * Table 1:
 * 
 * Bits   Function        Use
 * 
 * 0x0f    getType        Used by macro language to determine the arg type
 * 0x10    isDisplayable  Used in display to determine if a character can be directly displayed
 * 0x20    isPokable      Used by screen-poke to determine if a character can be directly poked
 * 0x40    isPrint        Used by print to determine if a character can be directly printed
 * 0x80    isSpace        Is a white char
 * 
 * 
 * Table 2:
 * 
 * Bits   Function        Use
 * 
 * 0x01    isLower        Is Lower case test
 * 0x02    isUpper        Is Upper case test
 * 0x03    isAlpha        Is Alpha test (Lower | Upper)
 * 0x04    isXDigit       Is Hex digit char (0-9 | a-f | A-F)
 * 0x07    isAlphaNum     Is alpha numeric char (Upper|Lower|HexDigit)
 * 0x08    isSpllExt      Is Spell Extended character (e.g. '.', '-' etc)
 * 0x0f    isSpllWord     Is Spell word
 * 0xf0    isWord         Is a word letter - selected by $buffer-mask
 * 
 */

uint8 charMaskTbl1[256] =
{
#if (defined _DOS) || (defined _WIN32)
    0x80, 0x2A, 0x2A, 0x2A, 0x2A, 0x2A, 0x2A, 0x2A, 0x2A, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0x2A, 0x2A, 
    0x2A, 0x2A, 0x2A, 0x2A, 0x2A, 0x2A, 0x2A, 0x2A, 0x2A, 0x2A, 0x2A, 0x2A, 0x2A, 0x2A, 0x2A, 0x2A, 
#else
    0x80, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x8A, 0x8A, 0x8A, 0x8A, 0x8A, 0x0A, 0x0A, 
    0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 
#endif
    0xFA, 0x76, 0x79, 0x72, 0x74, 0x73, 0x75, 0x7A, 0x7A, 0x7A, 0x77, 0x7A, 0x7A, 0x78, 0x7C, 0x7A, 
    0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x7B, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 
    0x71, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 
    0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 
    0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 
#if (defined _DOS) || (defined _WIN32)
    0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x3A, 
    0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 
    0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 
#else
    0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x0A, 
    0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 
    0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 
#endif
    0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 
    0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 
    0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 
    0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 
    0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 
    0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A
};

uint8 charMaskTbl2[256] =
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x08, 0x00, 
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x10, 
    0x00, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

uint8 charCaseTbl[256] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    0x40, 'a',  'b',  'c',  'd',  'e',  'f',  'g',  'h',  'i',  'j',  'k',  'l',  'm',  'n',  'o',  
    'p',  'q',  'r',  's',  't',  'u',  'v',  'w',  'x',  'y',  'z',  0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
    0x60, 'A',  'B',  'C',  'D',  'E',  'F',  'G',  'H',  'I',  'J',  'K',  'L',  'M',  'N',  'O',  
    'P',  'Q',  'R',  'S',  'T',  'U',  'V',  'W',  'X',  'Y',  'Z',  0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
    0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
    0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
    0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
    0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
    0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
    0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF
};   
uint8 charUserLatinTbl[256] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
    0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
    0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
    0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
    0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
    0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
    0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF
}; 
uint8 charLatinUserTbl[256] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
    0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
    0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
    0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
    0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
    0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
    0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF
}; 
uint8 charMaskFlags[]="luhs1234dpPwaAMLU" ;
uint8 isWordMask=CHRMSK_DEFWORDMSK;


static int       ffremain ;
       uint8    *ffbuf=NULL ;
static uint8    *ffcur ;
static uint8     ffbinary=0 ;
static uint8     ffcrypt=0 ;
static uint8     ffauto=0 ;
static uint8     ffautoRet=0 ;
static uint8     ffnewFile ;
static int       ffread ;

#define AUTO_CRLF  0x01
#define AUTO_CTRLZ 0x02

#ifdef _URLSUPP

#include <stdarg.h>

#ifdef _WIN32
/* winsock2.h must be included before */
#if (_MSC_VER != 900)
#include <winsock2.h>
#else
#include <winsock.h>
#endif
#include <io.h>
typedef void (*meATEXIT)(void) ;
#define meSOCKET SOCKET
#define meBadSocket          INVALID_SOCKET
#define meSocketIsBad(ss)    ((ss) == meBadSocket)

#define meCloseSocket(ss)    (closesocket(ss),ss=meBadSocket)
#else
#include <sys/time.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <netinet/in.h>
#define meSOCKET int
#define meBadSocket -1
#define meSocketIsBad(ss)    ((ss) < 0)
#define meCloseSocket(ss)    (close(ss),ss=meBadSocket)
#endif

#define meOpenSocket         socket

#define meSOCKET_TIMEOUT 240000

static meSOCKET ffccsk=meBadSocket ;
static meSOCKET ffsock=meBadSocket ;
static uint8 *ffsockAddr=NULL ;

/* Define the URL file types */
#define meURLTYPE_FILE 0
#define meURLTYPE_HTTP 1
#define meURLTYPE_FTP  2
#define meURLTYPE_DIR  4
static int fftype=meURLTYPE_FILE ;

static struct sockaddr_in meSockAddr ;
static int ffsize ;
static int ffstartTime ;
#define ffURL_CONSOLE        0x01
#define ffURL_SHOW_CONSOLE   0x02
#define ffURL_SHOW_PROGRESS  0x04
static int ffurlFlags=ffURL_CONSOLE ;
static BUFFER *ffurlBp=NULL ;
static BUFFER *ffurlReadBp ;
static uint8 *ffurlFlagsVName[2]={(uint8 *)"http-flags",(uint8 *)"ftp-flags"} ;
static uint8 *ffurlConsoleBName[2]={(uint8 *)"*http-console*",(uint8 *)"*ftp-console*"} ;

#ifndef INADDR_NONE
/* This may not be defined, particularly suns */
#define INADDR_NONE -1
#endif

#define ftpERROR         0
#define ftpPOS_PRELIMIN  1
#define ftpPOS_COMPLETE  2
#define ftpPOS_INTERMED  3

static void
strBase64Encode3(uint8 *dd, uint8 c1, uint8 c2, uint8 c3)
{
    static uint8 base64Table[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" ;
    dd[0] = base64Table[(c1 >> 2)] ;
    dd[1] = base64Table[((c1 << 4) & 0x30) | ((c2 >> 4) & 0x0f)] ;
    dd[2] = base64Table[((c2 << 2) & 0x3c) | ((c3 >> 6) & 0x03)] ;
    dd[3] = base64Table[(c3 & 0x3f)] ;
}

static uint8 *
strBase64Encode(uint8 *dd, uint8 *ss)
{
    uint8 c1, c2, c3 ;
    
    while((c1=*ss++) != '\0')
    {
        if((c2=*ss++) == '\0')
        {
            c3 = '\0' ;
            strBase64Encode3(dd,c1,c2,c3) ;
            dd += 2 ;
            *dd++ = '=' ;
            *dd++ = '=' ;
            break ;
        }
        else if((c3=*ss++) == '\0')
        {
            strBase64Encode3(dd,c1,c2,c3) ;
            dd += 3 ;
            *dd++ = '=' ;
            break ;
        }
        strBase64Encode3(dd,c1,c2,c3) ;
        dd += 4 ;
    }
    *dd = '\0' ;
    return dd ;
}

/* `hostname' can be of any form described in inet_addr(3);
 * `port' can be a service name, a port number, or NULL to have the
 * OS choose a port (this is probably only useful for servers);
 * `proto' can be "udp" or "tcp"
 */
static int
make_inet_addr(uint8 *hostname, uint8 *port, uint8 *proto)
{
    struct hostent *hp ;
    struct servent *sp ;
    
    memset(&meSockAddr,0,sizeof(meSockAddr)) ;
    meSockAddr.sin_family = AF_INET ;
    
    if ((sp = getservbyname((char *)port,(char *)proto)) != NULL)
    {
        meSockAddr.sin_port = sp->s_port;
    }
    else if ((meSockAddr.sin_port = htons(atoi((char *)port))) == 0)
        return mlwrite(MWABORT|MWPAUSE,(uint8 *)"[Bad port number %s]", port) ;
    
    /* First resolve the hostname, then resolve the port */
    if ((meSockAddr.sin_addr.s_addr = inet_addr((char *)hostname)) != INADDR_NONE)
    {
        /* in_addr.s_addr is already set */
    }
    else if(((hp = gethostbyname((char *)hostname)) != NULL) &&
            (hp->h_length <= sizeof(struct in_addr)) &&
            (hp->h_addrtype == AF_INET))
        memcpy(&(meSockAddr.sin_addr),hp->h_addr,hp->h_length) ;
    else
        return mlwrite(MWABORT|MWPAUSE,(uint8 *)"[Unknown host %s]", hostname) ;
    
    return TRUE ;
}

static void
ffurlConsoleAddText(uint8 *str, int flags)
{
    if(flags & 0x01)
    {
        ffurlBp->b_dotp = lback(ffurlBp->b_linep) ;
        ffurlBp->b_doto = llength(ffurlBp->b_dotp) ;
        ffurlBp->line_no = ffurlBp->elineno - 1 ;
    }
    else
    {
        ffurlBp->b_dotp = ffurlBp->b_linep ;
        ffurlBp->b_doto = 0 ;
        ffurlBp->line_no = ffurlBp->elineno ;
    }
    bufferPosStore(ffurlBp->b_dotp,ffurlBp->b_doto,ffurlBp->line_no) ;
    if(flags & 0x01)
    {
        LINE *olp, *nlp ;
        uint8 buff[MAXBUF], cc ;
        int ii, ll ;
        
        olp = ffurlBp->b_dotp ;
        ll = ii = ffurlBp->b_doto ;
        meStrncpy(buff,ltext(olp),ii) ;
        while((cc=*str++) != '\0')
            buff[ii++] = cc ;
        meStrcpy(buff+ii,ltext(olp)+ll) ;
        addLine(olp,buff) ;
        nlp = lback(olp) ;
        nlp->l_fp = olp->l_fp ;
        olp->l_fp->l_bp = nlp ;
        if(olp->l_flag & LNMARK)
            lunmarkBuffer(ffurlBp,olp,nlp) ;
        meFree(olp) ;
    }
    else
        addLineToEob(ffurlBp,str) ;
    if(flags & 0x02)
    {
        ffurlBp->b_dotp = lback(ffurlBp->b_linep) ;
        ffurlBp->b_doto = llength(ffurlBp->b_dotp) ;
        ffurlBp->line_no = ffurlBp->elineno - 1 ;
    }
    else
    {
        ffurlBp->b_dotp = ffurlBp->b_linep ;
        ffurlBp->b_doto = 0 ;
        ffurlBp->line_no = ffurlBp->elineno ;
    }
    bufferPosUpdate(ffurlBp,(flags & 0x01) ? 0:1,ffurlBp->b_doto) ;
    
    if(!(flags & 0x04) && (ffurlFlags & ffURL_SHOW_CONSOLE))
        screenUpdate(TRUE,sgarbf) ;
}

static meSOCKET
ffOpenConnectUrlSocket(uint8 *host, uint8 *port)
{
    meSOCKET ss ;
    int ii ;
    
    if(ffurlBp != NULL)
    {
        uint8 buff[MAXBUF] ;
        sprintf((char *)buff,"Connecting to: %s",host);
        ffurlConsoleAddText(buff,0) ;
    }
    mlwrite(MWCURSOR,(uint8 *)"[Connecting to %s]",host);
    
    if(!make_inet_addr(host,port,(uint8 *)"tcp"))
        return meBadSocket ;
    
    if((ss=meOpenSocket(AF_INET,SOCK_STREAM,0)) < 0)
    {
        mlwrite(MWABORT|MWPAUSE,(uint8 *)"[Failed to open socket]") ;
        return meBadSocket ;
    }
    if(connect(ss,(struct sockaddr *) &meSockAddr,sizeof(meSockAddr)) != 0)
    {
        meCloseSocket(ss) ;
        mlwrite(MWABORT|MWPAUSE,(uint8 *)"[Failed to connect to %s:%s]",host,port) ;
        return meBadSocket ;
    }
    
    ii = 1 ;
    setsockopt (ss,SOL_SOCKET,SO_KEEPALIVE,(char *) &ii,sizeof(int));
    
    return ss ;
}


static int
ffReadSocketLine(meSOCKET ss, uint8 *buff)
{
    uint8 *dd=buff, cc ;
    
    for(;;)
    {
        if(ffremain == 0)
        {
            ffremain = recv(ss,(char *) ffbuf,FIOBUFSIZ,0) ;
            if(ffremain <= 0)
            {
                mlwrite(MWABORT|MWPAUSE,(uint8 *)"[Failed to read line]") ;
                return FALSE ;
            }
            ffbuf[ffremain]='\n' ;
            ffcur = ffbuf ;
        }
        ffremain-- ;
        cc = *ffcur++ ;
        if(cc == '\n')
        {
            *dd = '\0' ;
            if(ffurlBp != NULL)
                ffurlConsoleAddText(buff,0) ;
            return TRUE ;
        }
        if(cc != '\r')
            *dd++ = cc ;
    }
}

static int
ftpReadReply(void)
{
    uint8 buff[1024] ;
    int ret ;
    
    ffremain = 0 ;
    if(ffReadSocketLine(ffccsk,buff) == FALSE)
        return FALSE ;
    if(meStrlen(buff) < 4)
        return ftpERROR ;
        
    ret = buff[0] - '0' ;
    if(buff[3] == '-')
    {
        /* multi-line reply */
        uint8 c0, c1, c2 ;
        c0 = buff[0] ;
        c1 = buff[1] ;
        c2 = buff[2] ;
        do {
            if(ffReadSocketLine(ffccsk,buff) == FALSE)
                return FALSE ;
        } while((buff[0] != c0) || (buff[1] != c1) ||
                (buff[2] != c2) || (buff[3] != ' ')) ;
    }
    if(ret > ftpPOS_INTERMED)
        ret = ftpERROR ;
    return ret ;
}


static int 
ftpCommand(int flags, char *fmt, ...)
{
    va_list ap;
    int ii ;
    
    va_start(ap,fmt);
    vsprintf((char *)ffbuf,fmt,ap);
    va_end(ap);
    if(ffurlBp != NULL)
    {
        if(!meStrncmp(ffbuf,"PASS",4))
            ffurlConsoleAddText((uint8 *)"PASS XXXX",0) ;
        else
            ffurlConsoleAddText(ffbuf,0) ;
    }
    ii = meStrlen(ffbuf) ;
    ffbuf[ii++] = '\r' ;
    ffbuf[ii++] = '\n' ;
    ffbuf[ii]   = '\0' ;
    if(send(ffccsk,(char *)ffbuf,ii,0) <= 0)
    {
        mlwrite(MWABORT|MWPAUSE,(uint8 *)"[Failed to send command]") ;
        return ftpERROR ;
    }
    if(flags & 0x01)
        return ftpPOS_COMPLETE ;
    return ftpReadReply();
}
    
void
ffCloseSockets(int logoff)
{
    timerClearExpired(SOCKET_TIMER_ID) ;
    
    if(!meSocketIsBad(ffsock))
        meCloseSocket(ffsock) ;
    
    if(logoff && !meSocketIsBad(ffccsk))
    {
        ftpCommand(1,"QUIT") ;
        meCloseSocket(ffccsk) ;
        if(meNullFree(ffsockAddr))
            ffsockAddr = NULL ;
    }
    ffremain = 0 ;
}


static int
ftpLogin(uint8 *user, uint8 *pass)
{
    int ii ;
    /* get the initial message */
    if(ftpReadReply() != ftpPOS_COMPLETE)
        return FALSE ;
    
    ii = ftpCommand(0,"USER %s",user) ;
    if(ii == ftpPOS_INTERMED)
        ii = ftpCommand(0,"PASS %s", pass) ;
    if(ii != ftpPOS_COMPLETE)
        return mlwrite(MWABORT|MWPAUSE,(uint8 *)"[Failed to login]") ;
        
    return TRUE ;
}

static int
ftpGetDataChannel(void)
{
    uint8 *aac, *ppc ;
    int aai[4], ppi[2] ;
    uint8 *ss ;
    
    if((ftpCommand(0,"PASV") != ftpPOS_COMPLETE) ||
       ((ss=meStrchr(ffbuf,'(')) == NULL) || 
       (sscanf((char *)ss,"(%d,%d,%d,%d,%d,%d)",aai,aai+1,aai+2,aai+3,ppi,ppi+1) != 6))
        return mlwrite(MWABORT|MWPAUSE,(uint8 *)"[Failed to get PASSIVE address]") ;
    
    aac = (uint8 *)&meSockAddr.sin_addr;
    ppc = (uint8 *)&meSockAddr.sin_port;
    aac[0] = (uint8) aai[0] ;
    aac[1] = (uint8) aai[1] ;
    aac[2] = (uint8) aai[2] ;
    aac[3] = (uint8) aai[3] ;
    ppc[0] = (uint8) ppi[0] ;
    ppc[1] = (uint8) ppi[1] ;

    return TRUE ;
}

/*
 * Need to start a listen on the data channel before we send the command,
 * otherwise the server's connect may fail.
 */
static int
ftpOpenDataChannel(void)
{
    if(meSocketIsBad(ffsock = meOpenSocket(AF_INET, SOCK_STREAM, 0)))
        return mlwrite(MWABORT|MWPAUSE,(uint8 *)"[Failed to open data channel]") ;
    
    if(connect(ffsock,(struct sockaddr *) &meSockAddr,sizeof(meSockAddr)) < 0)
        return mlwrite(MWABORT|MWPAUSE,(uint8 *)"[Failed to connect data channel]") ;
    
    return TRUE ;
}


int
ffurlGetInfo(int type, uint8 **host, uint8 **port, uint8 **user, uint8 **pass)
{
    REGHANDLE root, reg ;
    uint8 *ss ;
    
    if (((root = regFind (NULL,(uint8 *)"/url")) == NULL) &&
        ((root = regSet (NULL,(uint8 *)"/url", NULL)) == NULL))
        return FALSE ;
    ss = (uint8 *) ((fftype == meURLTYPE_FTP) ? "ftp":"http") ;
    if (((reg = regFind (root, ss)) == NULL) &&
        ((reg = regSet (root, ss, NULL)) == NULL))
        return FALSE ;
    if (((root = regFind (reg, *host)) == NULL) &&
        ((root = regSet (reg, *host, NULL)) == NULL))
        return FALSE ;
    
    reg = regFind (root,(uint8 *)"user") ;
    if (*user != NULL)
    {
        if((*pass == NULL) && (reg != NULL) && !meStrcmp(regGetValue(reg),*user) &&
           ((reg = regFind (root,(uint8 *)"pass")) != NULL))
            *pass = regGetValue(reg) ;
    }
    else if(reg != NULL)
    {
        *user = regGetValue(reg) ;
        if((reg = regFind (root,(uint8 *)"pass")) != NULL)
            *pass = regGetValue(reg) ;
    }
    if(*user != NULL)
    {
        if(*pass == NULL)
        {
            uint8 buff[NPAT] ;
            
            if((mlreply((uint8 *)"Password", MLNOHIST, 0, buff, NPAT-1) != TRUE) ||
               ((reg = regSet(root,(uint8 *)"pass",buff)) == NULL))
                return FALSE ;
            reg->mode |= REGMODE_INTERNAL ;
            *pass = regGetValue(reg) ;
        }
        else if(((reg = regFind(root,(uint8 *)"pass")) == NULL) || meStrcmp(regGetValue(reg),*pass))
            regSet(root,(uint8 *)"pass",*pass) ;
        if(((reg = regFind(root,(uint8 *)"user")) == NULL) || meStrcmp(regGetValue(reg),*user))
            regSet(root,(uint8 *)"user",*user) ;
    }
    if ((reg = regFind (root,(uint8 *)"host")) != NULL)
        *host = regGetValue(reg) ;
    reg = regFind (root,(uint8 *)"port") ;
    if (*port != NULL)
    {
        if((reg == NULL) || meStrcmp(regGetValue(reg),*port))
            regSet(root,(uint8 *)"port",*port) ;
    }
    else if(reg != NULL)
        *port = regGetValue(reg) ;
    return TRUE ;
}

void
ffurlCreateName(uint8 *buff, int type, uint8 *host, uint8 *port, uint8 *user, uint8 *pass, uint8 *file)
{
    if(type & meURLTYPE_FTP)
        meStrcpy(buff,"ftp://") ;
    else
        meStrcpy(buff,"http://") ;
    if(user != NULL)
    {
        meStrcat(buff,user) ;
        if(pass != NULL)
        {
            meStrcat(buff,":") ;
            meStrcat(buff,pass) ;
        }
        meStrcat(buff,"@") ;
    }
    meStrcat(buff,host) ;
    if(port != NULL)
    {
        meStrcat(buff,":") ;
        meStrcat(buff,port) ;
    }
    if(file != NULL)
    {
        int ll ;
        meStrcat(buff,file) ;
        ll = meStrlen(buff) ;
        if((type & meURLTYPE_DIR) && (buff[ll-1] != DIR_CHAR))
        {
            buff[ll++] = DIR_CHAR ;
            buff[ll]   = '\0' ;
        }
    }
}

static int ffUrlFileOpen(uint8 *urlName, uint8 *user, uint8 *pass, uint32 rwflag) ;

int
ffFtpFileOpen(uint8 *host, uint8 *port, uint8 *user, uint8 *pass, uint8 *file, uint32 rwflag)
{
    uint8 buff[MAXBUF] ;
    int dirlist ;
    
    /* are we currently connected to this site?? */
    ffurlCreateName(buff,meURLTYPE_FTP,host,port,user,NULL,NULL) ;
    if((ffsockAddr == NULL) || meStrcmp(buff,ffsockAddr))
    {
        /* if we're already logged on somewhere else kill the connection */
        ffCloseSockets(1) ;
        
        if(port == NULL)
            port = (uint8 *)"21" ;
        if(user == NULL)
        {
            /* user = "anonymous" ;*/
            user = (uint8 *)"ftp" ;
            pass = (uint8 *)"guest" ;
        }
    
        if(meSocketIsBad(ffccsk=ffOpenConnectUrlSocket(host,port)))
            return FALSE ;
    
        if(ftpLogin(user,pass) != TRUE)
        {
            ffCloseSockets(1) ;
            return FALSE ;
        }
        if(ftpCommand(0,"TYPE I") != ftpPOS_COMPLETE)
        {
            ffCloseSockets(1) ;
            return FALSE ;
        }
        ffsockAddr = meStrdup(buff) ;
    }
    
    if(ffurlBp != NULL)
    {
        uint8 buff[MAXBUF] ;
        ffurlConsoleAddText((uint8 *)"",0x04) ;
        if(rwflag & meRWFLAG_WRITE)
            meStrcpy(buff,"Writing: ");
        else if(rwflag & meRWFLAG_READ)
            meStrcpy(buff,"Reading: ");
        else
            meStrcpy(buff,"Deleting: ");
        meStrcat(buff,file);
        ffurlConsoleAddText(buff,0) ;
    }
    mlwrite(MWCURSOR,(uint8 *)"[Connected, %sing %s]",
            (rwflag & meRWFLAG_WRITE) ? "writ":
            (rwflag & meRWFLAG_READ)  ? "read":"delet",file);
    if(rwflag & meRWFLAG_BACKUP)
    {
        /* try to back-up the existing file - no error checking! */
        uint8 filename[FILEBUF] ;
        
        if(!createBackupName(filename,file,'~') &&
           (ftpCommand(0,"RNFR %s",file) == ftpPOS_INTERMED))
            ftpCommand(0,"RNTO %s",filename) ;
    }
    if(rwflag & meRWFLAG_DELETE)
    {
        /* if backing up as well the file is already effectively deleted */
        if(!(rwflag & meRWFLAG_BACKUP))
            ftpCommand(0,"DELE %s",file) ;
    }
    if(rwflag & meRWFLAG_MKDIR)
        ftpCommand(0,"MKD %s",file) ;
    
    if(!(rwflag & (meRWFLAG_WRITE|meRWFLAG_READ)))
        return TRUE ;
    
    dirlist = meStrlen(file) ;
    dirlist = (file[dirlist-1] == DIR_CHAR) ;
                  
    /* send the read command */
    for(;;)
    {
        if(ftpGetDataChannel() != TRUE)
        {
            ffCloseSockets(0) ;
            return FALSE ;
        }
        
        if(rwflag & meRWFLAG_WRITE)
            ftpCommand(1,"STOR %s",file) ;
        else if(dirlist)
            ftpCommand(1,"LIST %s",file) ;
        else
            ftpCommand(1,"RETR %s",file) ;
    
        /* open up the recv channel */
        if(ftpOpenDataChannel() != TRUE)
        {
            ffCloseSockets(0) ;
            return FALSE ;
        }
        
        /* find out if all's well */
        if(ftpReadReply() == ftpPOS_PRELIMIN)
        {
            if(dirlist && (ffurlReadBp != NULL))
                meModeSet(ffurlReadBp->b_mode,MDDIR) ;
            return TRUE ;
        }
        if((rwflag & meRWFLAG_WRITE) || dirlist)
        {
            ffCloseSockets(0) ;
            return mlwrite(MWABORT|MWPAUSE,(uint8 *)"[Failed to %s file]",
                           (rwflag & meRWFLAG_WRITE) ? "write":"read");
        }
        ffCloseSockets(0) ;
        dirlist = 1 ;
    }
}

static int
ffHttpFileOpen(uint8 *host, uint8 *port, uint8 *user, uint8 *pass, uint8 *file, uint32 rwflag)
{
    uint8 buff[MAXBUF+200], *thost, *tport, *ss, cc ;
    
    if ((ss = getUsrVar((uint8 *)"http-proxy-addr")) != errorm)
    {
        thost = ss ;
        tport = getUsrVar((uint8 *)"http-proxy-port") ;
    }
    else
    {
        thost = host ;
        tport = port ;
    }
    
    if(tport == NULL)
        tport = (uint8 *)"80" ;
    
    if(meSocketIsBad(ffsock=ffOpenConnectUrlSocket(thost,tport)))
        return FALSE ;
       
    if(ffurlBp != NULL)
    {
        ffurlConsoleAddText((uint8 *)"",0x04) ;
        sprintf((char *)buff,"Reading: %s",file);
        ffurlConsoleAddText(buff,0) ;
    }
    mlwrite(MWCURSOR,(uint8 *)"[Connected, reading %s]", file);

    /* send GET message to http */
    meStrcpy(buff,"GET ") ;
    if(thost == host)
        meStrcpy(buff+4,file) ;
    else
        ffurlCreateName(buff+4,fftype,host,port,NULL,NULL,file) ;
    ss = buff + meStrlen(buff) ;
    meStrcpy(ss," HTTP/1.0\r\nConnection: Keep-Alive\r\nHost: ") ;
    ss += meStrlen(ss) ;
    meStrcpy(ss,host) ;
    ss += meStrlen(ss) ;
    if(user != NULL)
    {
        /* password supplied, encode */
        uint8 tbuff[MAXBUF] ;
        meStrcpy(tbuff,user) ;
        meStrcat(tbuff,":") ;
        meStrcat(tbuff,pass) ;
        meStrcat(ss,"\r\nAuthorization: Basic ") ;
        ss += 23 ;
        ss = strBase64Encode(ss,user) ;
    }
    meStrcpy(ss,"\r\n\r\n") ;
    if(ffurlBp != NULL)
    {
        uint8 *s2 ;
        ss = s2 = buff ;
        while((cc=*s2++) != '\0')
        {
            if(cc == '\r')
            {
                s2[-1] = '\0' ;
                ffurlConsoleAddText(ss,0) ;
                s2[-1] = '\r' ;
                ss = ++s2 ;
            }
        }
        resetBufferWindows(ffurlBp) ;
        if(ffurlFlags & ffURL_SHOW_CONSOLE)
            screenUpdate(TRUE,sgarbf) ;
    }
    if(send(ffsock,(char *)buff,meStrlen(buff),0) <= 0)
    {
        ffCloseSockets(1) ;
        return FALSE ;
    }
    /* must now ditch the header, read up to the first blank line */
    while(ffReadSocketLine(ffsock,buff) == TRUE)
    {
        if(buff[0] == '\0')
        {
            ffread = ffremain ;
            return TRUE ;
        }
        if(!meStrncmp(buff,"Content-Length:",15))
            ffsize = meAtoi(buff+15) ;
        else if(!meStrncmp(buff,"Location:",9))
        {
            /* The requested file is not here, its at the given location */
            ss = buff+9 ;
            while(((cc=*ss) == ' ') || (cc == '\t'))
                ss++ ;
            if(*ss != '\0')
            {
                /* printf("Got Location: [%s]\n",ss) ;*/
                if(ffurlBp != NULL)
                    ffurlConsoleAddText((uint8 *)"",0x04) ;
                ffCloseSockets(1) ;
                return ffUrlFileOpen(ss,user,pass,rwflag) ;
            }
        }
    }
    ffCloseSockets(1) ;
    return mlwrite(MWABORT|MWPAUSE,(uint8 *)"[Failed to read header of %s]",ss) ;
}

static int
ffUrlFileOpen(uint8 *urlName, uint8 *user, uint8 *pass, uint32 rwflag)
{
    uint8 buff[MAXBUF], sockAddr[MAXBUF], *host, *port, *ss, *dd, *ee, cc ;
    int ii ;
    
#ifdef _WIN32
    static int init=0 ;
    if(!init)
    {
        WSADATA wsaData;
        WORD    wVersionRequested;
        
        wVersionRequested = MAKEWORD (1, 1);
    
        if(WSAStartup(wVersionRequested, &wsaData))
            return mlwrite(MWABORT|MWPAUSE,"[Failed to initialise sockets]") ;
        atexit((meATEXIT) WSACleanup) ;
        init = 1 ;
    }
#endif
    fftype = (urlName[0] == 'f') ? meURLTYPE_FTP:meURLTYPE_HTTP ;
    if(rwflag & meRWFLAG_READ)
    {
        ffrp = ffpInvalidVal ;
        ffsize = 0x7fffffff ;
    }
    else if(rwflag & meRWFLAG_WRITE)
        ffwp = ffpInvalidVal ;
    
    /* setup the flags and console buffer */
    ffurlFlags = ffURL_CONSOLE ;
    if ((ss = getUsrVar (ffurlFlagsVName[fftype-meURLTYPE_HTTP])) != errorm)
    {
        if(meStrchr(ss,'c') != NULL)
            ffurlFlags |= ffURL_CONSOLE ;
        if(meStrchr(ss,'s') != NULL)
            ffurlFlags |= ffURL_SHOW_CONSOLE ;
        if(meStrchr(ss,'p') != NULL)
            ffurlFlags |= ffURL_SHOW_PROGRESS ;
    }
    if((ffurlFlags & ffURL_CONSOLE) &&
       ((ffurlBp=bfind(ffurlConsoleBName[fftype-meURLTYPE_HTTP],BFND_CREAT)) != NULL))
    {
        meModeClear(ffurlBp->b_mode,MDUNDO) ;
        if(ffurlFlags & ffURL_SHOW_CONSOLE)
            wpopup(ffurlConsoleBName[fftype-meURLTYPE_HTTP],0) ;
    }
    
    /* skip the http: or ftp: */
    ss = (fftype == meURLTYPE_FTP) ? urlName+6:urlName+7 ;
    dd = buff ;
    ee = sockAddr ;
    host = dd ;
    port = NULL ;
    while(((cc=*ss++) != '\0') && (cc != DIR_CHAR))
    {
        *dd++ = cc ;
        *ee++ = cc ;
        if(cc == ':')
        {
            dd[-1] = '\0' ;
            port = dd ;
        }
        else if(cc == '@')
        {
            dd[-1] = '\0' ;
            user = buff ;
            pass = port ;
            host = dd ;
            port = NULL ;
        }
    }
    *dd = '\0' ;
    *ee = '\0' ;
    ss-- ;
    
    if(port != NULL)
        port[-1] = '\0' ;
    
    if(ffurlGetInfo(fftype,&host,&port,&user,&pass) != TRUE)
        return FALSE ;
    
    /* is it a http: or ftp: */
    if(fftype == meURLTYPE_FTP)
        ii = ffFtpFileOpen(host,port,user,pass,ss,rwflag) ;
    else
        ii = ffHttpFileOpen(host,port,user,pass,ss,rwflag) ;
    
    if(ii == TRUE)
    {
        if((rwflag & meRWFLAG_READ) && (ffurlReadBp != NULL))
        {
            uint8 *ff, buff[MAXBUF+128] ;
            if(fftype == meURLTYPE_HTTP)
            {
                if(!(rwflag & meRWFLAG_INSERT))
                {
                    ffurlCreateName(buff,fftype,host,port,user,NULL,ss) ;
                    if(fnamecmp(buff,ffurlReadBp->b_fname) &&
                       ((ff=meStrdup(buff)) != NULL))
                        /* the original fname is freed in ffReadFile */
                        ffurlReadBp->b_fname = ff ;
                }
            }
            else if(meModeTest(ffurlReadBp->b_mode,MDDIR))
            {
                /* for an ftp dir 2 things must be done:
                 * 1) If the file name does not end in a '/' then add one and correct the buffer name
                 * 2) Add the top Directory Listing line
                 */
                int ll ;
                ll = meStrlen(ffurlReadBp->b_fname) ;
                if(ffurlReadBp->b_fname[ll-1] != DIR_CHAR)
                {
                    meStrcpy(buff,ffurlReadBp->b_fname) ;
                    buff[ll] = DIR_CHAR ;
                    buff[ll+1] = '\0' ;
                    /* the original fname is freed in ffReadFile */
                    ffurlReadBp->b_fname = NULL ;
                    resetBufferNames(ffurlReadBp,buff) ;
                }
                meStrcpy(buff,"Directory listing of: ") ;
                ffurlCreateName(buff+22,(fftype|meURLTYPE_DIR),host,port,user,NULL,ss) ;
                addLineToEob(ffurlReadBp,buff) ;
            }
            ffurlReadBp = NULL ;
        }
        if((rwflag & (meRWFLAG_READ|meRWFLAG_WRITE)) && (ffurlBp != NULL))
        {
            struct meTimeval tp ;
            if(ffurlFlags & ffURL_SHOW_PROGRESS)
                ffurlConsoleAddText((uint8 *)"Progress: ",0x02) ;
            gettimeofday(&tp,NULL) ;
            ffstartTime = (tp.tv_sec * 1000) + (tp.tv_usec/1000) ;
        }
    }
    else
        ffurlBp = NULL ;
    return ii ;
}

static int
ffUrlFileClose(uint8 *fname, uint32 rwflag)
{
    if(rwflag & (meRWFLAG_READ|meRWFLAG_WRITE))
    {
        if(fname[0] == 'f')
        {
            if(!meSocketIsBad(ffsock))
                meCloseSocket(ffsock) ;
            /* get the transfer complete */
            if(ftpReadReply() != ftpPOS_COMPLETE)
                mlwrite(MWABORT|MWPAUSE,(uint8 *)"[Problem occurred during download]") ;
        }
        if(ffurlBp != NULL)
        {
            uint8 buff[MAXBUF] ;
            struct meTimeval tp ;
            int dwldtime ;
            gettimeofday(&tp,NULL) ;
            dwldtime = ((tp.tv_sec * 1000) + (tp.tv_usec/1000)) - ffstartTime ;
            if(dwldtime <= 0)
                dwldtime = 1 ;
            sprintf((char *)buff,"%d bytes %s in %d.%d seconds (%d.%d Kbytes/s)",
                    ffread,(rwflag & meRWFLAG_WRITE) ? "sent":"received",
                    dwldtime/1000,(dwldtime/100) % 10,
                    ffread/dwldtime,((ffread*100)/dwldtime)% 100) ;
            ffurlConsoleAddText(buff,0x04) ;
            ffurlConsoleAddText((uint8 *)"",0) ;
            ffurlBp = NULL ;
        }
    }
    if(ffsockAddr == NULL)
        ffCloseSockets(1) ;
    else
        timerSet(SOCKET_TIMER_ID,-1,meSOCKET_TIMEOUT) ;
    return TRUE ;
}

#endif

/*
 * creates the standard bckup name with the extra
 * character(s) being backl, where a '~' is used
 * for backups and '#' for auto-saves
 * returns 1 if fails, else 0 and name in filename buffer
 */
int
createBackupName(uint8 *filename, uint8 *fn, uint8 backl)
{
    uint8 *s, *t ;
    
    if(fn == NULL)
        return 1 ;
#ifdef _URLSUPP
    if((backl == '#') && isUrlLink(fn))
    {
        uint8 tmp[FILEBUF] ;
        s = getFileBaseName(fn) ;
        meStrcpy(tmp,s) ;
        s = tmp ;
        while((s=meStrchr(s,DIR_CHAR)) != NULL)  /* got a '/', -> '.' */
            *s++ = '.' ;
        mkTempName(filename,tmp,NULL) ;
    }
    else
#endif
    {
        meStrcpy(filename,fn) ;
#ifdef _UNIX
        if((backl == '~') && ((meSystemCfg & (meSYSTEM_DOSFNAMES|meSYSTEM_HIDEBCKUP)) == meSYSTEM_HIDEBCKUP))
        {
            if((t=meStrrchr(fn,DIR_CHAR)) == NULL)
                t = fn ;
            else
                t++ ;
            s = filename + ((size_t) t - (size_t) fn) ;
            *s++ = '.' ;
            meStrcpy(s,t) ;
        }
#endif            
    }
    t = filename + meStrlen(filename) ;
#ifndef _DOS
    if(meSystemCfg & meSYSTEM_DOSFNAMES)
#endif
    {
        s = t ;
        while(--s != filename)
            if((*s == '.') || (*s == DIR_CHAR))
                break ;
        if(*s != '.')
        {
            *t = '.' ;
            t[1] = backl ;
            t[2] = backl ;
            t[3] = backl ;
            t[4] = '\0' ;
        }
#ifndef _DOS
        /* In the Win32 environment. If the extension is longer
         * than 3 characters then replace the last character with
         * the backup character. Otherwise behave like a standard
         * DOS name.
         */
        else if ((t - s) > 3)
            t[-1] = backl;
#endif    
        else
        {
            t = s ;
            if(t[2] == '\0')
                t[2] = backl ;
            t[3] = backl ;
            t[4] = '\0' ;
        }
        if(!meStrcmp(filename,fn))
            return 1 ;
    }
#ifndef _DOS
    else
    {
        *t++ = backl ;
        *t   = 0 ;
    }
#endif
    return 0 ;
}

static int
ffgetBuf(void)
{
#ifdef _URLSUPP
    if(ffrp == ffpInvalidVal)
    {
        if((ffremain = ffsize-ffread) > 0)
        {
            if(ffremain > FIOBUFSIZ)
                ffremain = FIOBUFSIZ ;
            ffremain = recv(ffsock,(char *) ffbuf,ffremain,0) ;
        }
        if(ffremain <= 0)
        {
            ffremain = -1 ;
            return TRUE ;
        }
        if((ffurlBp != NULL) &&
           (ffurlFlags & ffURL_SHOW_PROGRESS))
            ffurlConsoleAddText((uint8 *)"#",(llength(ffurlBp->b_dotp) >= TTncol - 2) ? 0x02:0x03) ;
    }
    else
#endif
#ifdef _WIN32
    {
        if(ReadFile(ffrp,ffbuf,FIOBUFSIZ,&ffremain,NULL) == 0)
            return mlwrite(MWABORT,"File read error %d",GetLastError());
        
        if(ffremain <= 0)
        {
            ffremain = -1 ;
            return TRUE ;
        }
    }
#else
    {
        ffremain = fread(ffbuf,1,FIOBUFSIZ,ffrp) ;
        if(ffremain <= 0)
        {
            if(ferror(ffrp))
                return mlwrite(MWABORT,(uint8 *)"File read error %d",errno);
            ffremain = -1 ;
            return TRUE ;
        }
    }
#endif
#if	CRYPT
    if(ffcrypt)
        meCrypt(ffbuf, ffremain);
#endif
    ffbuf[ffremain]='\n' ;
    ffread += ffremain ;
    return TRUE ;
}

/*
 * Read a line from a file and create a new line to hold it.
 * If mode & MDBINARY then loading it in binary mode an a line
 * is upto 16 bytes long.
 * 
 * Returns ABORT if a serious system problem arose (read or malloc
 * failure).
 * Returns Error is nothing was read cos at the end of the file
 * Else success so returns TRUE.
 */
static int	
ffgetline(LINE **line)
{
    uint8  buf[16] ;
    register uint8 *text ;
    register uint8 *endp ;
    register int    len=0, newl, ii ;
    register LINE  *lp ;
    
    do {
        if(ffremain <= 0)
        {
            if(ffgetBuf() != TRUE)
                return ABORT ;
            if(ffremain < 0)
                break ;
            ffcur = ffbuf ;
        }
        if(ffbinary)
        {
            if(ffremain < (16-len))
            {
                memcpy(buf+len,ffcur,ffremain) ;
                len += ffremain ;
                ffremain = -1 ;
            }
            else
            {
                ii = 16-len ;
                memcpy(buf+len,ffcur,ii) ;
                ffremain -= ii ;
                ffcur += ii ;
                len = 16 ;
            }
        }
        else
        {
            register uint8 cc ;
            endp = ffcur ;
            newl = 0 ;
            while(((cc=*ffcur++) != '\n') && (cc != '\r'))
                newl++ ;
            
            /* If we ended in '\r' check for a '\n' */
            if(cc == '\r')
            {
                if(*ffcur == '\n')
                {
                    ffremain -= newl+2 ;
#ifdef _CTRLM
                    if(ffauto)
                        ffautoRet |= AUTO_CRLF ;
                    ffcur[-1] = '\n' ;
#else
                    if(ffauto)
                    {
                        ffautoRet |= AUTO_CRLF ;
                        ffcur[-1] = '\n' ;
                    }
                    else
                        newl++;
#endif
                    ffcur++ ;
                }
                else
                {
                    ffremain -= newl+1 ;
                    ffcur[-1] = '\n' ;
                }
            }
            else
                ffremain -= newl+1 ;
            
            if(len == 0)
            {
                len = newl ;
                if((lp = (LINE *) meMalloc(sizeof(LINE)+len)) == NULL)
                    return ABORT ;
                lp->l_flag = 0 ;
                text = lp->l_text ;
            }
            else
            {
                ii = len ;
                len += newl ;
                if((lp = (LINE *) meRealloc(lp,sizeof(LINE)+len)) == NULL)
                    return ABORT ;
                text = lp->l_text + ii ;
            }
            while((cc = *endp++) != '\n')
            {
                if(cc == '\0')
                    len-- ;
                else
                    *text++ = cc ;
            }
            *text = '\0' ;
            if(len >= (0xfffe - FIOBUFSIZ))
            {
                /* TOO Big! break out */
                ffremain = -2 ;
                break ;
            }
        }
    } while(ffremain < 0) ;
    
    if(ffbinary)
    {
        long cpos ;
        
        if(len == 0)
            return FALSE ;
        if((lp = (LINE *) meMalloc(sizeof(LINE)+78)) == NULL)
            return ABORT ;
        lp->l_flag = 0 ;
        text = lp->l_text ;
        
        cpos = (ffread - ffremain - 1) & ~(0x0fL) ;
        
        for(newl=7 ; newl>=0 ; newl--)
        {
            text[newl] = hexdigits[cpos&0x0f] ;
            cpos >>= 4 ;
        }
        text[8] = ':' ;
        text[9] = ' ' ;
                  
        for(newl=0 ; newl<16 ; newl++)
        {
            if(newl < len)
            {
                uint8 cc = buf[newl] ;
                text[(newl*3)+10] = hexdigits[cc/0x10] ;
                text[(newl*3)+11] = hexdigits[cc%0x10] ;
                text[(newl*3)+12] = ' ' ;
                if(isDisplayable(cc))
                    text[newl+62] = cc ;
                else
                    text[newl+62] = '.' ;
            }
            else
            {
                text[(newl*3)+10] = ' ' ;
                text[(newl*3)+11] = ' ' ;
                text[(newl*3)+12] = ' ' ;
                text[62+newl] = ' ' ;
            }
        }
        text[58] = ' ' ;
        text[59] = '|' ;
        text[60] = ' ' ;
        text[61] = ' ' ;
        text[78] = '\0' ;
        lp->l_size = 78 ;
        lp->l_used = 78 ;
        lp->l_flag = 0 ;			/* line is not marked.	*/
        *line = lp ;
        return TRUE ;
    }
    if(ffremain < 0)  /* had to break */
    {
        if(ffremain == -2)
            /* had to break because line was too long */
            ;
        else if(len == 0)
            return FALSE ;
        else if(ffauto && (len == 1) && (lp->l_text[0] == 26))
        {
            ffautoRet |= AUTO_CTRLZ ;
            meFree(lp) ;
            return FALSE ;
        }
        /* Don't flag as the end, otherwise we won't get a proper linep */
        /* mlwrite(MWCURSOR|MWPAUSE,"Warning - Incomplete last line");*/
            
        lp->l_flag = LNNEOL ;				/* line has no \n */
    }
    lp->l_size = len ;
    lp->l_used = len ;
    *line = lp ;

    return TRUE ;
}

static int
ffReadFileOpen(uint8 *fname, uint32 flags, BUFFER *bp)
{
    if(bp != NULL)
    {
        ffbinary = (meModeTest(bp->b_mode,MDBINRY) != 0) ;
        ffcrypt  = (meModeTest(bp->b_mode,MDCRYPT) != 0) ;
        ffauto   = (meModeTest(bp->b_mode,MDAUTO) != 0) ;
    }
    else
    {
        ffbinary = 0 ;
        ffcrypt = 0 ;
        ffauto = 1 ;
    }        
    ffautoRet= 0 ;
    ffremain = 0 ;
    ffread = 0 ;
    
    if(fname != NULL)
    {
#if URLAWAR
        if(isUrlLink(fname))
        {
#ifdef _URLSUPP
#ifdef _UNIX
            /* must stop any signals or the connection will fail */
            meSigHold() ;
#endif
            ffurlReadBp = bp ;
            if(ffUrlFileOpen(fname,NULL,NULL,flags) != TRUE)
            {
#ifdef _UNIX
                meSigRelease() ;
#endif
                return ABORT ;
            }
            /* cannot return here, must do ffrsetup */
#else
            return mlwrite(MWABORT|MWPAUSE,"[No url support on this platform]") ;
#endif
        }        
        else
#endif
        {
#ifdef _WIN32
            if((ffrp=CreateFile(fname,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL,NULL)) == INVALID_HANDLE_VALUE)
#else
            if ((ffrp=fopen((char *)fname, "rb")) == NULL)
#endif
            {
                if(!(flags & meRWFLAG_SILENT))
                    /* File not found.      */
                    mlwrite(MWCLEXEC,(uint8 *)"[%s: No such file]", fname);
                return ABORT ;
            }
        }
    }
#ifdef _UNIX
#ifdef _URLSUPP
    if(ffrp != ffpInvalidVal)
#endif
        meSigHold() ;
#endif

    return TRUE ;
}

/* close the read file handle, Ignore any close error */
static void
ffReadFileClose(uint8 *fname, uint32 flags)
{
    /* Windows pipeCommand comes in as a pipe because the source file
     * has already been opened, but from now one treat it as a normal
     * file.
     * The output file has been set up as a temporary file with the
     * 'delete when closed' flag.
     */
#ifdef _URLSUPP
    if(ffrp == ffpInvalidVal)
        ffUrlFileClose(fname,meRWFLAG_READ) ;
    else
#endif
    {
#ifdef _WIN32
        if((ffrp != GetStdHandle(STD_INPUT_HANDLE)) &&
           (CloseHandle(ffrp) == 0) && !(flags & meRWFLAG_SILENT))
            mlwrite(MWABORT|MWPAUSE,(uint8 *)"Error closing file");
#else
#ifdef _UNIX
        if(fname == NULL)
        {
            if(ffrp != stdin)
                pclose(ffrp);
        }
        else if((fclose(ffrp) != 0) && !(flags & meRWFLAG_SILENT))
            mlwrite(MWABORT|MWPAUSE,(uint8 *)"Error closing file");
#else
        if((ffrp != stdin) && (fclose(ffrp) != 0) && !(flags & meRWFLAG_SILENT))
            mlwrite(MWABORT|MWPAUSE,(uint8 *)"Error closing file");
#endif
#endif
    }
#ifdef _UNIX
    meSigRelease() ;
#endif
}

int
ffReadFile(uint8 *fname, uint32 flags, BUFFER *bp, LINE *hlp)
{
    LINE *lp0, *lp1, *lp2 ;
    int   ss, nline ;
#ifdef _URLSUPP
    int ff ;
    ff = ((bp != NULL) && (fname != NULL) && (bp->b_fname == fname)) ;
#endif
    flags |= meRWFLAG_READ ;
    if(ffReadFileOpen(fname,flags,bp) != TRUE)
        return ABORT ;
    
    nline = 0;
    lp2 = hlp ;	        /* line after  insert */
    lp0 = lp2->l_bp ;	/* line before insert */
    while((ss=ffgetline(&lp1)) == TRUE)
    {
        /* re-link new line between lp0 and lp1 */
        lp0->l_fp = lp1;
        lp1->l_bp = lp0;
        lp0 = lp1 ;
        nline++ ;
        if(TTbreakTest(1))
        {
            ss = ctrlg(FALSE,1) ;
            break ;
        }
    }
    /* complete the link */
    lp2->l_bp = lp0;
    lp0->l_fp = lp2;
    
    ffReadFileClose(fname,flags) ;
    
#ifdef _URLSUPP
    /* loading an ftp or http file can lead to a file name change, free
     * off the old one now that we don't need it any more */
    if(ff && (bp->b_fname != fname))
        free(fname) ;
#endif
    if(bp != NULL)
    {
        bp->elineno += nline ;
        if(!(flags & meRWFLAG_INSERT) && ffauto && nline)
        {
            if(ffautoRet & AUTO_CRLF)
                meModeSet(bp->b_mode,MDCRLF) ;
            else
                meModeClear(bp->b_mode,MDCRLF) ;
            if(ffautoRet & AUTO_CTRLZ)
                meModeSet(bp->b_mode,MDCTRLZ) ;
            else
                meModeClear(bp->b_mode,MDCTRLZ) ;
        }
    }
    return ss ;
}


static int
ffputBuf(void)
{
    int written ;
#if	CRYPT
    if(ffcrypt)
        meCrypt(ffbuf,ffremain) ;
#endif
#ifdef _URLSUPP
    if(ffwp == ffpInvalidVal)
    {
        if(((written = (int) send(ffsock,(char *) ffbuf,ffremain,0)) == ffremain) &&
           (ffurlBp != NULL) && (ffurlFlags & ffURL_SHOW_PROGRESS))
            ffurlConsoleAddText((uint8 *)"#",(llength(ffurlBp->b_dotp) >= TTncol - 2) ? 0x02:0x03) ;
    }
    else
#endif
#ifdef _WIN32
    {
        if(WriteFile(ffwp,ffbuf,ffremain,&written,NULL) == 0)
            written = -1 ;
    }
#else
        written = (int) fwrite(ffbuf,1,ffremain,ffwp) ;
#endif
    if(written != ffremain)
        return mlwrite(MWABORT,(uint8 *)"Write I/O error");
    ffremain = 0 ;
    return 0 ;
}

#define ffputc(c)                                                            \
(((ffbuf[ffremain++]=c),(ffremain == FIOBUFSIZ))? ffputBuf():0)
    
int
ffWriteFileOpen(uint8 *fname, uint32 flags, BUFFER *bp)
{
    if(bp != NULL)
    {
#if CRYPT
        if(resetkey(bp) != TRUE)
            return FALSE ;
#endif
    }
    if(fname == NULL)
    {
#ifdef _WIN32
        ffwp = GetStdHandle(STD_OUTPUT_HANDLE) ;
#else
        ffwp = stdout ;
#endif
        ffnewFile = 0 ;
    }
#if URLAWAR
    else if(isFtpLink(fname))
    {
#ifdef _URLSUPP
#ifdef _UNIX
        /* must stop any signals or the connection will fail */
        meSigHold() ;
#endif
        if(ffUrlFileOpen(fname,NULL,NULL,(flags & (meRWFLAG_WRITE|meRWFLAG_BACKUP|meRWFLAG_DELETE|meRWFLAG_MKDIR))) != TRUE)
        {
#ifdef _UNIX
            meSigRelease() ;
#endif
            return ABORT ;
        }
        if(!(flags & meRWFLAG_WRITE))
        {
            int rr ;
            rr = ffUrlFileClose(fname,flags & (meRWFLAG_WRITE|meRWFLAG_BACKUP|meRWFLAG_DELETE|meRWFLAG_MKDIR)) ;
#ifdef _UNIX
            meSigRelease() ;
#endif
            return rr ;
        }
        ffnewFile = 0 ;
#else
        return mlwrite(MWABORT|MWPAUSE,"[No url support on this platform]") ;
#endif
    }
#endif
    else
    {
        if(!(ffnewFile=meTestExist(fname)))
        {
            if(flags & meRWFLAG_BACKUP)
            {
                uint8 filename[FILEBUF] ;
                int ii ;
                
#ifdef _WIN32
                /* Win95 first version has nasty bug in MoveFile, doing
                 * 
                 * MoveFile("a.bbb","a.bbb~")
                 * CreateFile("a.bbb",..)
                 * 
                 * Resulted in a.bbb backing up to a.bbb~ and then for some unknown bloody
                 * reason opening a.bbb opens a.bbb~ instead of creating a new file.
                 * 
                 * So alternatively do:
                 *
                 * MoveFile("a.bbb","a.bbb~.~_~")
                 * MoveFile("a.bbb~.~_~","a.bbb~")
                 * CreateFile("a.bbb",..)
                 * 
                 * Which sadly works (we love you bill!)
                 * 
                 * This is done for all flavours of windows as these drives can be networked
                 */
                extern int platformId;
                uint8 filenameOldB[FILEBUF], *filenameOld ;
                
                if(!(meSystemCfg & meSYSTEM_DOSFNAMES))
                {
                    strcpy(filenameOldB,fname) ;
                    strcat(filenameOldB,".~a~") ;
                    filenameOld = filenameOldB ;
                    if(!meTestExist(filenameOld) && meUnlink(filenameOld))
                        mlwrite(MWABORT|MWPAUSE,"[Unable to remove backup file %s]", filenameOld) ;
                    else if(meRename(fname,filenameOld))
                        mlwrite(MWABORT|MWPAUSE,"[Unable to backup file to %s]",filenameOld) ;
                }
                else
                    filenameOld = fname ;
#else
#define filenameOld fname
#endif
                ii = createBackupName(filename,fname,'~') ;
#ifndef _DOS
                if(!(meSystemCfg & meSYSTEM_DOSFNAMES) && (keptVersions > 0))
                {
                    int ll ;
                    
                    ll = meStrlen(filename)-1 ;
                    filename[ll++] = '.' ;
                    filename[ll++] = '~' ;
                    for(ii=0 ; ii<keptVersions ; ii++)
                    {
                        sprintf((char *)filename+ll,"%d~",ii) ;
                        if(meTestExist(filename))
                            break ;
                    }
                    if(ii == keptVersions)
                    {
                        if(meUnlink(filename))
                            mlwrite(MWABORT|MWPAUSE,(uint8 *)"[Unable to remove backup file %s]", filename) ;
                        ii-- ;
                    }
                    if(ii)
                    {
                        uint8 filename2[FILEBUF] ;
                        meStrcpy(filename2,filename) ;
                        while(ii > 0)
                        {
                            sprintf((char *)filename2+ll,"%d~",ii--) ;
                            sprintf((char *)filename+ll,"%d~",ii) ;
                            if(meRename(filename,filename2))
                            {
                                mlwrite(MWABORT|MWPAUSE,(uint8 *)"[Unable to backup file to %s (%d - %s)]", 
                                        filename2,errno,sys_errlist[errno]) ;
                                if(meUnlink(filename))
                                {
                                    mlwrite(MWABORT|MWPAUSE,(uint8 *)"[Unable to remove backup file %s]", filename) ;
                                    break ;
                                }
                            }
                        }
                    }
                }
#endif
                if(!ii)
                {
                    if(!meTestExist(filename) && meUnlink(filename))
                        mlwrite(MWABORT|MWPAUSE,(uint8 *)"[Unable to remove backup file %s]", filename) ;
                    else if(meRename(filenameOld,filename))
                        mlwrite(MWABORT|MWPAUSE,(uint8 *)"[Unable to backup file to %s (%d - %s)]", 
                                filename,errno,sys_errlist[errno]) ;
                    else if(flags & 0xffff)
                        meChmod(filename,(flags & 0xffff)) ;
                }
            }
            else if(flags & meRWFLAG_DELETE)
            {
                /* if backing up as well the file is already effectively deleted */
                if(meUnlink(fname))
                    return ABORT ;
            }
        }
        if(!(flags & meRWFLAG_WRITE))
        {
            if(flags & meRWFLAG_MKDIR)
            {
#ifdef _WIN32
                if(CreateDirectory(fname,NULL) == 0)
                    return ABORT ;
#else
                if(mkdir((char *)fname,meXUmask) != 0)
                    return ABORT ;
#endif
            }
            return TRUE ;
        }
#ifdef _WIN32
        {
            DWORD create ;
            if(flags & meRWFLAG_OPENTRUNC)
                create = TRUNCATE_EXISTING ;
            else if(flags & meRWFLAG_OPENEND)
                create = OPEN_ALWAYS ;
            else
                create = CREATE_ALWAYS ;
            /* Windows must open the file with the correct permissions to support the
             * compress attribute
             */
            if((ffwp=CreateFile(fname,GENERIC_WRITE,FILE_SHARE_READ,NULL,create,
                               ((bp == NULL) ? meUmask:bp->stats.stmode),
                               NULL)) == INVALID_HANDLE_VALUE)
                return mlwrite(MWABORT,(uint8 *)"Cannot open file [%s] for writing",fname);
            if(flags & meRWFLAG_OPENEND)
                SetFilePointer(ffwp,0,NULL,FILE_END) ;
        }
#else
        {
            char *ss ;
            if(flags & meRWFLAG_OPENEND)
                ss = "ab" ;
            else
                ss = "wb" ;
            if((ffwp=fopen((char *)fname,ss)) == NULL)
                return mlwrite(MWABORT,(uint8 *)"Cannot open file [%s] for writing",fname);
        }
#endif
        mlwrite(MWCURSOR|MWCLEXEC,(uint8 *)"[%sriting %s]",
                (flags & meRWFLAG_AUTOSAVE) ? "Auto-w":"W",fname);	/* tell us were writing */
    }
    ffremain = 0 ;
    if(bp != NULL)
    {
        ffbinary = (meModeTest(bp->b_mode,MDBINRY) != 0) ;
        ffcrypt  = (meModeTest(bp->b_mode,MDCRYPT) != 0) ;
        if(!ffbinary &&
#ifdef _CTRLZ
           (!meModeTest(bp->b_mode,MDAUTO) || meModeTest(bp->b_mode,MDCTRLZ))
#else
           (meModeTest(bp->b_mode,MDAUTO) && meModeTest(bp->b_mode,MDCTRLZ))
#endif
           )
            ffauto = 1 ;
        else
            ffauto = 0 ;
    
        if(!ffbinary &&
#ifdef _CTRLM
           (!meModeTest(bp->b_mode,MDAUTO) || meModeTest(bp->b_mode,MDCRLF))
#else
           (meModeTest(bp->b_mode,MDAUTO) && meModeTest(bp->b_mode,MDCRLF))
#endif
           )
            ffautoRet = 1 ;
        else
            ffautoRet = 0 ;
#if NARROW
        if((bp->narrow != NULL) && !(flags & meRWFLAG_IGNRNRRW))
            unnarrowBuffer(bp) ;
#endif
#if	TIMSTMP
        if(!(flags & meRWFLAG_AUTOSAVE))
            set_timestamp(bp);			/* Perform time stamping */
#endif
    }
    else
    {
        ffbinary = 0 ;
        ffcrypt  = 0 ;
        ffauto   = 0 ;
        ffautoRet= 0 ;
    }
#ifdef _UNIX
#ifdef _URLSUPP
    if(ffwp != ffpInvalidVal)
#endif
        meSigHold() ;
#endif

#if (defined _DOS) || (defined _WIN32)
    /* the directory time stamps on dos are not updated so the best
     * we can do is flag that the dirLists are out of date when WE
     * create a new file. Obviously if something else creates a file
     * then that file many be missed in hte completion lists etc
     */
    if(ffnewFile == 1)
    {
        extern meDIRLIST fileNames ;

        curDirList.timeStamp = 1 ;
        fileNames.timeStamp = 1 ;
    }
#endif
    return TRUE ;
}


int
ffWriteFileWrite(register int len, register uint8 *buff, int eolFlag)
{
    uint8 cc, dd ;
    
    if(ffbinary)
    {
        while(--len >= 0)
            if(*buff++ == ':')
                break ;
        for(;;)
        {
            while((--len >= 0) && ((cc = *buff++) == ' '))
                ;
            if((len < 0) || (cc == '|'))
                break ;
            if(!isXDigit(cc) || (--len < 0) ||
               ((dd = *buff++),!isXDigit(dd)))
                return mlwrite(MWABORT|MWPAUSE,(uint8 *)"Binary format error");
            cc = (hexToNum(cc)<<4) | hexToNum(dd) ;
            if(ffputc(cc))
                return ABORT ;
        }
    }
    else
    {
        while(--len >= 0)
        {
            cc=*buff++ ;
            if(ffputc(cc))
                return ABORT ;
        }
        if(eolFlag)
        {
            if(ffautoRet && ffputc('\r'))
                return ABORT ;
            if(ffputc('\n'))
                return ABORT ;
        }
    }
    return TRUE ;
}

int
ffWriteFileClose(uint8 *fname, uint32 flags, BUFFER *bp)
{
    int ret ;
    
    /* add a ^Z at the end of the file if needed */
    if((ffauto && ffputc(26)) || ffputBuf())
        ret = ABORT ;
#ifdef _URLSUPP
    else if(ffwp == ffpInvalidVal)
        ret = ffUrlFileClose(fname,flags) ;
#endif
#ifdef _WIN32
    else if((ffwp != GetStdHandle(STD_OUTPUT_HANDLE)) && (CloseHandle(ffwp) == 0))
        ret = mlwrite(MWABORT|MWPAUSE,(uint8 *)"Error closing file");
#else
    else if((ffwp != stdout) && (fclose(ffwp) != 0))
        ret = mlwrite(MWABORT|MWPAUSE,(uint8 *)"Error closing file");
#endif
    else
        ret = TRUE ;
#ifdef _UNIX
    meSigRelease() ;
#endif

#if NARROW
    if((bp != NULL) && (bp->narrow != NULL) && !(flags & meRWFLAG_IGNRNRRW))
        redoNarrowInfo(bp) ;
#endif
    return ret ;
}


int
ffWriteFile(uint8 *fname, uint32 flags, BUFFER *bp)
{
    register int   ss ;
    
    flags |= meRWFLAG_WRITE ;
    if((ss=ffWriteFileOpen(fname,flags,bp)) == TRUE)
    {
        register LINE *lp ;
        long noLines ;
        
        /* store the number of lines now cos the close may narrow the buffer
         * and the number of lines will be wrong */
        noLines = bp->elineno+1 ;
        lp = lforw(bp->b_linep);            /* First line.          */
        while((lp != bp->b_linep) &&
              ((ss=ffWriteFileWrite(llength(lp),ltext(lp),
                                    !(lp->l_flag & LNNEOL))) == TRUE))
            lp = lforw(lp) ;
        ffWriteFileClose(fname,flags,bp) ;
        if(ss == TRUE)
        {
            if(ffnewFile)
                mlwrite(MWCLEXEC,(uint8 *)"[New file %s, Wrote %d lines]",fname,noLines);
            else
                mlwrite(MWCLEXEC,(uint8 *)"[Wrote %d lines]",noLines);
            return TRUE ;
        }
    }
    return FALSE ;
}

int
ffCopyFile(uint8 *sfname, uint8 *dfname, uint32 dFlags)
{
    int rr=TRUE ;
    if(dfname != NULL)
    {
        if(ffReadFileOpen(sfname,meRWFLAG_READ,NULL) != TRUE)
            return ABORT ;
        if(ffWriteFileOpen(dfname,meRWFLAG_WRITE|(dFlags & meRWFLAG_BACKUP),NULL) != TRUE)
            return ABORT ;
        for(;;)
        {
            if((rr=ffgetBuf()) != TRUE)
                break ;
            if(ffremain < 0)
                break ;
            if(ffputBuf() != 0)
            {
                rr = ABORT ;
                break ;
        }
        }
        ffReadFileClose(sfname,meRWFLAG_READ) ;
        ffremain = 0 ;
        ffWriteFileClose(dfname,meRWFLAG_WRITE,NULL) ;
    }
    if((rr == TRUE) && (dFlags & (meRWFLAG_DELETE|meRWFLAG_MKDIR)))
    {
        rr = ffWriteFileOpen(sfname,dFlags & (meRWFLAG_DELETE|meRWFLAG_MKDIR|meRWFLAG_BACKUP),NULL) ;
    }
    return rr ;
}
