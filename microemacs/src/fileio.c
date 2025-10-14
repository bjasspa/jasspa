/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * fileio.c - File reading and writing routines.
 *
 * Copyright (C) 1988-2024 JASSPA (www.jasspa.com)
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
 * Created:     Unknown
 * Synopsis:    File reading and writing routines.
 * Authors:     Unknown, Steven Phillips
 */

#define	__FILEIOC			/* Define program name */

#include "emain.h"

#ifdef _WIN32
#define meFileGetError()   GetLastError()
#else
#include <errno.h>
#define meFileGetError()   (errno)
#endif

#if (defined _UNIX) || (defined _DOS) || (defined _WIN32)
#include <sys/types.h>
#include <sys/stat.h>
#ifdef _UNIX
#include <utime.h>
#endif
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

meUByte charMaskTbl1[256] =
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
    0x7A,
#else
    0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x0A,
    0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A,
    0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A,
    0x0A,
#endif
    0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A,
    0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A,
    0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A,
    0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A,
    0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A,
    0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A
};

#if MEOPT_EXTENDED
meUByte charMaskTbl2[256] =
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

meUByte charCaseTbl[256] = {
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
meUByte charDsplyIntrnlTbl[256] = {
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
meUByte charIntrnlDsplyTbl[256] = {
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
meUShort charToUnicode[128] = { 0 };
meUByte isWordMask=CHRMSK_DEFWORDMSK;
meUByte charMaskFlags[]="luhs1234dpPwaAMIDkc";
#endif

meUByte *charKeyboardMap=NULL;

#define meBINARY_BPL       16   /* bytes per line */
#define meRBIN_BPL       1024   /* bytes per line */

meInt ffread;
meInt ffremain;
meUByte *ffcur;
/* must add at least 4 to meFIOBUFSIZ, but EOF padding for crypt may require upto a block */
meUByte ffbuf[meFIOBUFSIZ+meCRYPT_BLK_SIZE];

meUByte
ffUrlGetType(meUByte *url)
{
    meUByte type, cc;
    
    if(url == NULL)
        return meIOTYPE_PIPE;
    type = meIOTYPE_NONE;
    if((cc = *url++) == 'h')
    {
        if((*url++ == 't') && (*url++ == 't') && (*url++ == 'p'))
        {
            if((cc = *url++) == ':')
                type = meIOTYPE_HTTP;
            else if((cc == 's') && (*url++ == ':'))
                type = meIOTYPE_SSL|meIOTYPE_HTTP;
        }
    }
    else if(cc == 'f')
    {
        if((cc = *url++) == 't')
        {
            if(*url++ == 'p')
            {
                if((cc = *url++) == ':')
                    type = meIOTYPE_FTP;
                else if((cc == 's') && (*url++ == ':'))
                    type = meIOTYPE_SSL|meIOTYPE_FTP;
                else if((cc == 'e') && (*url++ == ':'))
                    type = meIOTYPE_SSL|meIOTYPE_FTPE;
            }
        }
        else if(cc == 'i')
        {
            if((*url++ == 'l') && (*url++ == 'e') && (*url == ':'))
                return meIOTYPE_FILE;
        }
    }
    else if(cc == 't')
    {
        if((*url++ == 'f') && (*url++ == 's') && (*url++ == ':'))
            type = meIOTYPE_TFS;
    }
    if((type == meIOTYPE_NONE) || (*url++ != '/') || (*url != '/'))
        return meIOTYPE_NONE;
    return type;
}

#if MEOPT_SOCKET

#include <stdarg.h>

#define meSOCKET_TIMEOUT      115000
#define meBUF_SOCK_SIZE_MAX   (((meSOCK_BUFF_SIZE) > (meBUF_SIZE_MAX)) ? (meSOCK_BUFF_SIZE) : (meBUF_SIZE_MAX))

static meUByte *ffUrlFlagsVName[2]={(meUByte *)"http-flags",(meUByte *)"ftp-flags"};
static meUByte *ffUrlFlagsVDef[2]={(meUByte *)"c",(meUByte *)"ci"};
static meUByte *ffUrlConsoleBName[2]={(meUByte *)"*http-console*",(meUByte *)"*ftp-console*"};
static meCookie ffUrlCookie;


static void
ffUrlConsoleAddText(meIo *io, meUByte *str, int flags)
{
    meInt lc=io->urlBp->lineCount;
    if(flags & 0x01)
    {
        io->urlBp->dotLine = meLineGetPrev(io->urlBp->baseLine);
        io->urlBp->dotOffset = meLineGetLength(io->urlBp->dotLine);
        io->urlBp->dotLineNo = lc - 1;
    }
    else
    {
        io->urlBp->dotLine = io->urlBp->baseLine;
        io->urlBp->dotOffset = 0;
        io->urlBp->dotLineNo = lc;
    }
    meBufferStoreLocation(io->urlBp->dotLine,io->urlBp->dotOffset,io->urlBp->dotLineNo);
    if(flags & 0x01)
    {
        meLine *olp, *nlp;
        meUByte buff[meBUF_SIZE_MAX], cc;
        int ii, ll;
        
        olp = io->urlBp->dotLine;
        ll = ii = io->urlBp->dotOffset;
        meStrncpy(buff,meLineGetText(olp),ii);
        while((cc=*str++) != '\0')
            buff[ii++] = cc;
        meStrcpy(buff+ii,meLineGetText(olp)+ll);
        addLine(olp,buff);
        nlp = meLineGetPrev(olp);
        nlp->next = olp->next;
        olp->next->prev = nlp;
        if(olp->flag & meLINE_ANCHOR)
            meLineResetAnchors(meLINEANCHOR_ALWAYS|meLINEANCHOR_RETAIN,io->urlBp,
                               olp,nlp,0,0);
        meFree(olp);
    }
    else
        addLineToEob(io->urlBp,str);
    if(flags & 0x02)
    {
        io->urlBp->dotLine = meLineGetPrev(io->urlBp->baseLine);
        io->urlBp->dotOffset = meLineGetLength(io->urlBp->dotLine);
        io->urlBp->dotLineNo = io->urlBp->lineCount - 1;
    }
    else
    {
        io->urlBp->dotLine = io->urlBp->baseLine;
        io->urlBp->dotOffset = 0;
        io->urlBp->dotLineNo = io->urlBp->lineCount;
    }
    meBufferUpdateLocation(io->urlBp,io->urlBp->lineCount-lc,io->urlBp->dotOffset);
    
    if(!(flags & 0x04))
    {
        if(io->urlOpts & meSOCKOPT_SHOW_CONSOLE)
            screenUpdate(meTRUE,2-sgarbf);
#ifdef _WIN32
        TTaheadFlush();
#endif
    }
}

static void
ffCloseSockets(meIo *io, int force)
{
    if(meSockIsInUse(io))
    {
        /* This could be a callback and as a result only rely on exclusively socket content of io as rest may have been used to read/write local file */
        /* If console was in use find the buffer again, but don't bother creating one for this */
        if((io->urlOpts & (meSOCKOPT_LOG_STATUS|meSOCKOPT_LOG_ERROR|meSOCKOPT_LOG_WARNING|meSOCKOPT_LOG_DETAILS|meSOCKOPT_LOG_VERBOSE)) &&
           ((io->urlBp = bfind(ffUrlConsoleBName[meSockIsFtp(io) ? 1:0],0)) == NULL))
            io->urlOpts &= ~(meSOCKOPT_SHOW_STATUS|meSOCKOPT_LOG_STATUS|meSOCKOPT_LOG_ERROR|meSOCKOPT_LOG_WARNING|meSOCKOPT_LOG_DETAILS|meSOCKOPT_LOG_VERBOSE);
        else
            io->urlOpts &= ~meSOCKOPT_SHOW_STATUS;
        
        if(meSockClose(io,force))
            timerSet(SOCKET_TIMER_ID,-1,meSOCKET_TIMEOUT);
    }
    ffremain = 0;
}

int
ffUrlGetInfo(meIo *io, meUByte **host, meUByte **port, meUByte **user, meUByte **pass)
{
    meRegNode *root, *rNd;
    meUByte *ss;
    
    if(((root = regFind(NULL,(meUByte *)"/url")) == NULL) && ((root = regSet(NULL,(meUByte *)"/url",NULL)) == NULL))
        return meFALSE;
    ss = (meUByte *) ((ffUrlTypeIsFtp(io->type)) ? "ftp":"http");
    if(((rNd = regFind(root,ss)) == NULL) && ((rNd = regSet(root,ss,NULL)) == NULL))
        return meFALSE;
    if(((root = regFind(rNd,*host)) == NULL) && ((root = regSet(rNd,*host,NULL)) == NULL))
        return meFALSE;
    
    rNd = regFind(root,(meUByte *)"user");
    if(*user != NULL)
    {
        if((*pass == NULL) && (rNd != NULL) && !meStrcmp(regGetValue(rNd),*user) &&
           ((rNd = regFind(root,(meUByte *)"pass")) != NULL))
            *pass = regGetValue(rNd);
    }
    else if(rNd != NULL)
    {
        *user = regGetValue(rNd);
        if((rNd = regFind(root,(meUByte *)"pass")) != NULL)
            *pass = regGetValue(rNd);
    }
    io->passwdReg = NULL;
    if(*user != NULL)
    {
        if(*pass == NULL)
        {
            meUByte buff[meSBUF_SIZE_MAX];
            /* Always get the password from the user even when running a macro.
             * This is important for &stat which may trigger a password request */
            meStrcpy(buff,*user);
            meStrcat(buff,"@");
            meStrcat(buff,*host);
            meStrcat(buff," password");
            if((meGetStringFromUser(buff, MLNOHIST|MLHIDEVAL, 0, buff, meSBUF_SIZE_MAX) <= 0) ||
               ((io->passwdReg = regSet(root,(meUByte *)"pass",buff)) == NULL) ||
               ((regGetValue(io->passwdReg) == NULL) && ((io->passwdReg->value = meStrdup(buff)) == NULL)))
                return meFALSE;
            io->passwdReg->mode |= meREGMODE_INTERNAL;
            *pass = regGetValue(io->passwdReg);
        }
        else if(((rNd = regFind(root,(meUByte *)"pass")) == NULL) || meStrcmp(regGetValue(rNd),*pass))
            regSet(root,(meUByte *)"pass",*pass);
        if(((rNd = regFind(root,(meUByte *)"user")) == NULL) || meStrcmp(regGetValue(rNd),*user))
            regSet(root,(meUByte *)"user",*user);
    }
    if ((rNd = regFind(root,(meUByte *)"host")) != NULL)
        *host = regGetValue(rNd);
    rNd = regFind(root,(meUByte *)"port");
    if (*port != NULL)
    {
        if((rNd == NULL) || meStrcmp(regGetValue(rNd),*port))
            regSet(root,(meUByte *)"port",*port);
    }
    else if(rNd != NULL)
        *port = regGetValue(rNd);
    return meTRUE;
}

void
ffSUrlLogger(meIo *io, meUByte type,meUByte *txt)
{
    if(io->urlBp != NULL)
        ffUrlConsoleAddText(io,txt,0);
    if((type <= meSOCKOPT_LOG_ERROR) && ((io->urlOpts & meSOCKOPT_SHOW_STATUS) || (type == meSOCKOPT_LOG_ERROR)))
        mlwrite((type == meSOCKOPT_LOG_ERROR) ? (MWABORT|MWPAUSE):MWCURSOR,(meUByte *) "[%s]",txt);
}

int
ffFtpFileOpen(meIo *io, meUInt rwflag, meUByte *url, meBuffer *bp)
{
    meUByte buff[meBUF_SOCK_SIZE_MAX+32], urlBuff[meBUF_SIZE_MAX], *hst, *prt, *usr, *psw, *ups=NULL, *fl, *dd, cc;
    meUShort flags;
    meInt ii, dirlist;
    
    fl = url + 6;
    flags = 0;
    if(ffUrlTypeIsSecure(io->type))
    {
        flags |= (ffUrlTypeIsFtpe(io->type)) ? meSOCKFLG_EXPLICIT_SSL:meSOCKFLG_USE_SSL;
        fl++;
    }
    usr = psw = prt = NULL;
    hst = dd = urlBuff;
    while(((cc=*fl++) != '\0') && (cc != '/'))
    {
        if(cc == '@')
        {
            *dd++ = '\0';
            if(usr != NULL)
            {
                /* already had an '@' which can't be part of URL so assume part of password or user name if no ':' encountered */
                hst[-1] = '@';
                if((psw == NULL) && ((psw = prt) != NULL))
                    ups = fl-(dd-psw)-1;
            }
            else
            {
                usr = hst;
                if((psw = prt) != NULL)
                    ups = fl-(dd-psw)-1;
            }
            hst = dd;
            prt = NULL;
        }
        else if((cc == ':') && (prt == NULL))
        {
            /* if prt != NULL then this is probably part of the password */
            *dd++ = '\0';
            prt = dd;
        }
        else
            *dd++ = cc;
    }
    fl--;
    if(cc == '\0')
    {
        fl[0] = '/';
        fl[1] = '\0';
    }
    *dd = '\0';
    if((prt == NULL) || (prt[0] == '\0'))
    {
        ii = 0;
        prt = (meUByte *) ((flags & meSOCKFLG_USE_SSL) ? "990":"21");
    }
    else
        ii = atoi((char *) prt);
    
    if(ffUrlGetInfo(io,&hst,&prt,&usr,&psw) <= 0)
        return meFALSE;
    
    if(meSockFtpOpen(io,flags,hst,ii,usr,psw,buff) < 0)
        return meFALSE;
    
    if((fl[1] == '~') && (fl[2] == '/'))
    {
        if(io->urlHome != NULL)
        {
            meStrcpy(urlBuff,io->urlHome);
            meStrcat(urlBuff,fl+2);
            fl = urlBuff;
        }
        else
            fl += 2;
    }
    
    if(rwflag & meRWFLAG_WRITE)
        dd = (meUByte *) "writing";
    else if(rwflag & meRWFLAG_READ)
        dd = (meUByte *) "reading";
    else if(rwflag & meRWFLAG_DELETE)
        dd = (meUByte *) "deleting";
    else if(rwflag & meRWFLAG_MKDIR)
        dd = (meUByte *) "mkdir";
    else
        dd = (meUByte *) "stating";
    if(io->urlFlags & meSOCKOPT_LOG_STATUS)
    {
        sprintf((char *)buff,"Connected - %s %s",dd,fl);
        if(io->urlBp != NULL)
            ffUrlConsoleAddText(io,buff,0);
        if(io->urlOpts & meSOCKOPT_SHOW_STATUS)
            mlwrite(MWCURSOR,(meUByte *) "[%s]",buff);
    }
    if(rwflag & meRWFLAG_BACKUP)
    {
        /* try to back-up the existing file - no error checking! */
        meUByte filename[meBUF_SIZE_MAX];
        
        if(createBackupName(filename,fl,'~',1) ||
           ((meSockFtpCommand(io,buff,"RNFR %s",fl) == ftpPOS_INTERMED) &&
            (meSockFtpCommand(io,buff,"RNTO %s",filename) != ftpPOS_COMPLETE) &&
            ((meSockFtpCommand(io,buff,"DELE %s",filename) != ftpPOS_COMPLETE) ||
             (meSockFtpCommand(io,buff,"RNFR %s",fl) != ftpPOS_INTERMED) ||
             (meSockFtpCommand(io,buff,"RNTO %s",filename) != ftpPOS_COMPLETE))))
            mlwrite(MWABORT|MWPAUSE,(meUByte *)"[Unable to backup file %s]",fl);
    }
    if((rwflag & meRWFLAG_DELETE) && !(rwflag & meRWFLAG_BACKUP))
    {
        /* if backing up as well the file is already effectively deleted */
        ii = meStrlen(fl);
        if(fl[ii-1] == '/')
        {
            if((meSockFtpCommand(io,buff,"CWD %s..",fl) != ftpPOS_COMPLETE) ||
               (meSockFtpCommand(io,buff,"RMD %s",fl) != ftpPOS_COMPLETE))
                return -5;
        }
        else if((meSockFtpCommand(io,buff,"DELE %s",fl) != ftpPOS_COMPLETE) &&
                ((meSockFtpCommand(io,buff,"CWD %s/..",fl) != ftpPOS_COMPLETE) ||
                 (meSockFtpCommand(io,buff,"RMD %s",fl) != ftpPOS_COMPLETE)))
            return -5;
    }
    if(rwflag & meRWFLAG_MKDIR)
    {
        if(meSockFtpCommand(io,buff,"MKD %s",fl) != ftpPOS_COMPLETE)
            return -4;
    }
    if(rwflag & meRWFLAG_STAT)
    {
        dd = evalResult;
        if(meSockFtpCommand(io,buff,"CWD %s",fl) == ftpPOS_COMPLETE)
        {
            /* this is a directory */
            *dd++ = 'D';
            *dd++ = '|';
        }
        else if(meSockFtpCommand(io,buff,"SIZE %s",fl) == ftpPOS_COMPLETE)
        {
            /* this is a regular file */
            *dd++ = 'R';
            if((ii=meStrlen(buff) - 4) > 0)
            {
                meStrcpy(dd,buff+4);
                dd += ii;
            }
            *dd++ = '|';
            /* get the mod time */
            if((meSockFtpCommand(io,buff,"MDTM %s",fl) == ftpPOS_COMPLETE) && ((ii=meStrlen(buff) - 4) > 0))
            {
                meStrcpy(dd,buff+4);
                if(dd[4] == '0')
                    dd[4] = ' ';
                if(dd[6] == '0')
                    dd[6] = ' ';
                if(dd[8] == '0')
                    dd[8] = ' ';
                if(dd[10] == '0')
                    dd[10] = ' ';
                if(dd[12] == '0')
                    dd[12] = ' ';
                dd += ii;
            }
        }
        else
        {
            /* does not exist */
            *dd++ = 'X';
            *dd++ = '|';
        }
        *dd = '\0';
    }
    if(!(rwflag & (meRWFLAG_WRITE|meRWFLAG_READ)))
        return meTRUE;
    
    ii = meStrlen(fl);
    if(rwflag & meRWFLAG_FTPNLST)
        dirlist = -1;
    else if(fl[ii-1] == DIR_CHAR)
        dirlist = 1;
    else
        dirlist = 0;
    
    /* send the read command */
    for(;;)
    {
        if(dirlist && (meSockFtpCommand(io,buff,"CWD %s",fl) != ftpPOS_COMPLETE))
            break;
        
        if(meSockFtpGetDataChannel(io,buff) < 0)
            break;
        
        if(rwflag & meRWFLAG_WRITE)
            ii = meSockFtpCommand(io,buff,"STOR %s",fl);
        else if(rwflag & meRWFLAG_FTPNLST)
            ii = meSockFtpCommand(io,buff,(dirlist < 0) ? "NLST -a":"NLST");
        else if(dirlist)
            ii = meSockFtpCommand(io,buff,"LIST");
        else
            ii = meSockFtpCommand(io,buff,"RETR %s",fl);
        
        /* find out if all's well */
        if(ii == ftpPOS_PRELIMIN)
        {
            if(meSockFtpConnectData(io,buff) < 0)
            {
                meSockClose(io,0);
                break;
            }
            if(dirlist)
            {
                io->type |= meIOTYPE_DIR;
                if(bp != NULL)
                {
                    /* for an ftp dir 2 things must be done:
                     * 1) If the file name does not end in a '/' then add one and correct the buffer name
                     * 2) Add the top Directory Listing line
                     */
                    meStrcpy(buff,"Directory listing of: ");
                    dd = buff+22;
                    if(ups != NULL)
                    {
                        ii = ups-url;
                        memcpy(dd,url,ii);
                        meStrcpy(dd+ii,meStrchr(ups,'@'));
                        url = buff;
                    }
                    else
                        meStrcpy(dd,url);
                    ii = meStrlen(dd);
                    if(dd[ii-1] != DIR_CHAR)
                    {
                        dd[ii] = DIR_CHAR;
                        dd[ii+1] = '\0';
                    }
                    if(!(rwflag & meRWFLAG_INSERT) && fnamecmp(dd,bp->fileName))
                    {
                        /* the original fname is freed in ffReadFile */
                        bp->fileName = NULL;
                        resetBufferNames(bp,dd);
                    }
                    addLineToEob(bp,buff);
                }
            }
            return meTRUE;
        }
        meSockClose(io,0);
        if((rwflag & (meRWFLAG_WRITE|meRWFLAG_NODIRLIST)) || (dirlist > 0))
            break;
        dirlist = 1;
    }
    if(!(rwflag & meRWFLAG_SILENT))
        mlwrite(MWCLEXEC|MWABORT,(meUByte *)"[Failed to %s file]",(rwflag & meRWFLAG_WRITE) ? "write":"read");
    return (rwflag & meRWFLAG_WRITE) ? -3:-2;
}

static int
ffHttpFileOpen(meIo *io, meUInt rwflag, meUByte *url, meCookie *cookie, meUByte *hdr, meInt fdLen, meUByte *frmData, meUByte *postFName, meBuffer *bp)
{
    meUByte buff[meBUF_SOCK_SIZE_MAX], urlBuff[meBUF_SIZE_MAX], *hst, *prt, *usr, *psw, *ups=NULL, *fl, *dd, cc;
    meUShort flags;
    meInt ii;
    
    fl = url + 7;
    flags = 0;
    if(ffUrlTypeIsSecure(io->type))
    {
        flags |= meSOCKFLG_USE_SSL;
        fl++;
    }
    usr = psw = prt = NULL;
    hst = dd = urlBuff;
    while(((cc=*fl++) != '\0') && (cc != '/'))
    {
        if(cc == '@')
        {
            *dd++ = '\0';
            if(usr != NULL)
            {
                /* already had an '@' which can't be part of URL so assume part of password or user name if no ':' encountered */
                hst[-1] = '@';
                if((psw == NULL) && ((psw = prt) != NULL))
                    ups = fl-(dd-psw)-1;
            }
            else
            {
                usr = hst;
                if((psw = prt) != NULL)
                    ups = fl-(dd-psw)-1;
            }
            hst = dd;
            prt = NULL;
        }
        else if((cc == ':') && (prt == NULL))
        {
            /* if prt != NULL then this is probably part of the password */
            *dd++ = '\0';
            prt = dd;
        }
        else
            *dd++ = cc;
    }
    fl--;
    if(cc == '\0')
        return mlwrite(MWABORT|MWPAUSE,(meUByte *)"[Invalid URL, missing '/' after host - %s]",url);
    *dd = '\0';
    if((prt == NULL) || (prt[0] == '\0'))
    {
        ii = 0;
        prt = (meUByte *) ((flags & meSOCKFLG_USE_SSL) ? "443":"80");
    }
    else
        ii = atoi((char *) prt);
    
    if(ffUrlGetInfo(io,&hst,&prt,&usr,&psw) <= 0)
        return meFALSE;
    
    if((ii=meSockHttpOpen(io,flags,hst,ii,usr,psw,fl,cookie,hdr,fdLen,frmData,postFName,buff)) < 0)
        return meFALSE;
    
    if(ii == 0)
    {
        meUByte cc;
        /* printf("Got Location: [%s]\n",ss);*/
        /* if this starts with http:// https:// etc. then start again */
        meSockClose(io,0);
        io->redirect++;
        if(io->redirect > 5)
        {
            if(rwflag & meRWFLAG_SILENT)
                return meABORT;
            return mlwrite(MWABORT|MWPAUSE,(meUByte *) "[To many redirections - see console]");
        }
        cc = ffUrlGetType(buff);
        if(cc & meIOTYPE_HTTP)
        {
            if((cookie != NULL) && (((dd = meStrstr(buff,hst)) == NULL) || (((cc=dd[-1]) != '/') && (cc != '@'))))
                cookie = NULL;
            io->type = cc;
            dd = buff;
        }
        else if(cc == meIOTYPE_NONE)
        {
            if(buff[0] != '/')
            {
                if((dd = meStrrchr(fl,'/')) != NULL)
                    fl = dd;
                fl++;
            }
            ii = fl-url;
            memcpy(urlBuff,url,ii);
            meStrcpy(urlBuff+ii,buff);
            dd = urlBuff;
        }
        else if(!(io->urlOpts & meSOCKOPT_REDIR_HALT))
        {
            if(rwflag & meRWFLAG_SILENT)
                return meABORT;
            return mlwrite(MWABORT|MWPAUSE,(meUByte *)"[Redirection to an unsupported file type - %s]",buff);
        }
        if(io->urlOpts & meSOCKOPT_REDIR_HALT)
        {
            meStrcpy(resultStr,dd);
            if(clexec || (rwflag & meRWFLAG_SILENT))
                return meABORT;
            return mlwrite(MWABORT|MWPAUSE,(meUByte *) "[Following redirections disabled - see console]");
        }
        else if(!meStrcmp(url,dd) && ((io->redirect > 1) || ((fdLen == 0) && (postFName == NULL))))
        {
            if(rwflag & meRWFLAG_SILENT)
                return meABORT;
            return mlwrite(MWABORT|MWPAUSE,(meUByte *)"[Redirection loop to %s]",buff);
        }
        return ffHttpFileOpen(io,rwflag,dd,cookie,hdr,0,NULL,NULL,bp);
    }
    meStrcpy(resultStr,meItoa(ii));
    io->length = io->urlLen;
    if(((rwflag & (meRWFLAG_READ|meRWFLAG_INSERT)) == meRWFLAG_READ) && (bp != NULL))
    {
        if(ups != NULL)
        {
            ii = ups-url;
            memcpy(buff,url,ii);
            meStrcpy(buff+ii,meStrchr(ups,'@'));
            url = buff;
        }
        if(fnamecmp(url,bp->fileName))
        {
            /* the original fname is freed in ffReadFile */
            bp->fileName = NULL;
            resetBufferNames(bp,url);
        }
    }
    return meTRUE;
}

static void
ffUrlFileSetupFlags(meIo *io, meUInt rwflag)
{
    meInt ti=(ffUrlTypeIsFtp(io->type)) ? 1:0;
    meUByte *ss;
    
    /* setup the flags and console buffer */
    io->urlBp = NULL;
    io->urlOpts = 0;
    if((ss = getUsrVar(ffUrlFlagsVName[ti])) == errorm)
        ss = ffUrlFlagsVDef[ti];
    else
    {
        if(meStrchr(ss,'i') != NULL)
            io->urlOpts |= meSOCKOPT_IGN_CRT_ERR;
        if(meStrchr(ss,'h') != NULL)
            io->urlOpts |= meSOCKOPT_REDIR_HALT;
    }
    if(meStrchr(ss,'C') != NULL)
        io->urlOpts |= meSOCKOPT_CLOSE;
    if(!(rwflag & meRWFLAG_SILENT))
        io->urlOpts |= meSOCKOPT_LOG_STATUS|meSOCKOPT_LOG_ERROR|meSOCKOPT_SHOW_STATUS;
    if((meStrchr(ss,'c') != NULL) &&
       ((io->urlBp=bfind(ffUrlConsoleBName[ti],BFND_CREAT)) != NULL))
    {
        io->urlOpts |= meSOCKOPT_LOG_STATUS|meSOCKOPT_LOG_ERROR;
        if(meStrchr(ss,'d') != NULL)
            io->urlOpts |= meSOCKOPT_LOG_DETAILS;
        if(meStrchr(ss,'p') != NULL)
            io->urlOpts |= meSOCKOPT_SHOW_PROGRESS;
        if((meStrchr(ss,'s') != NULL) && !(rwflag & meRWFLAG_INSERT))
            io->urlOpts |= meSOCKOPT_SHOW_CONSOLE;
        if(meStrchr(ss,'v') != NULL)
            io->urlOpts |= meSOCKOPT_LOG_VERBOSE;
        if(meStrchr(ss,'w') != NULL)
            io->urlOpts |= meSOCKOPT_LOG_WARNING;
        meModeClear(io->urlBp->mode,MDUNDO);
        if(io->urlBp->lineCount)
            ffUrlConsoleAddText(io,(meUByte *)"",0x04);
        /* must not show the console if inserting a file as the destination buffer will be displayed and unstable */
        if((io->urlOpts & meSOCKOPT_SHOW_CONSOLE) && !(rwflag & meRWFLAG_SILENT))
            meWindowPopup(NULL,ffUrlConsoleBName[ti],0,NULL);
    }
}

static int
ffUrlFileOpen(meIo *io, meUInt rwflag, meUByte *url, meBuffer *bp)
{
    int ii;
    
    io->fp = meBadFile;
    io->length = -1;
    ffUrlFileSetupFlags(io,rwflag);
    
    /* is it a http: or ftp: */
    if(ffUrlTypeIsFtp(io->type))
    {
        ii = ffFtpFileOpen(io,rwflag,url,bp);
    }
    else
    {
        meCookie *cookie;
        meVariable *vp;
        meUByte *s1, *s2, *cv, *hdr;
        meInt s1l;
        hdr = ((vp = getUsrLclCmdVarP((meUByte *)"http-header",usrVarList)) == NULL) ? NULL:vp->value;
        if(((vp = getUsrLclCmdVarP((meUByte *)"http-post-data",usrVarList)) == NULL) || ((s1=vp->value) == NULL) || ((s1l=meStrlen(s1)) == 0))
        {
            s1 = NULL;
            s1l = 0;
        }
        else
            /* clear the variable value now so the data does not get pushed again on a redirect */
            vp->value = meStrdup((meUByte *)"");
        if(((vp = getUsrLclCmdVarP((meUByte *)"http-post-file",usrVarList)) == NULL) || ((s2=vp->value) == NULL) || (*s2 == '\0'))
            s2 = NULL;
        else
            /* clear the variable value now so the file does not get pushed again on a redirect */
            vp->value = meStrdup((meUByte *)"");
        if(((vp = getUsrLclCmdVarP((meUByte *)"http-cookies",usrVarList)) == NULL) || ((cv=vp->value) == NULL))
            cookie = NULL;
        else
        {
            cookie = &ffUrlCookie;
            if(ffUrlCookie.value != cv)
            {
                ffUrlCookie.value = cv;
                ffUrlCookie.buffLen = meStrlen(cv);
            }
        }
        ii = ffHttpFileOpen(io,rwflag,url,cookie,hdr,s1l,s1,s2,bp);
        if((cookie != NULL) && (ffUrlCookie.value != cv))
            vp->value = ffUrlCookie.value;
        if(s1 != NULL)
            free(s1);
        if(s2 != NULL)
            free(s2);
    }
    if(ii <= 0)
        io->urlBp = NULL;
    else if((rwflag & (meRWFLAG_READ|meRWFLAG_WRITE)) && (io->urlBp != NULL))
    {
        struct meTimeval tp;
        if(io->urlOpts & meSOCKOPT_SHOW_PROGRESS)
            ffUrlConsoleAddText(io,(meUByte *)"Progress: ",0x02);
        gettimeofday(&tp,NULL);
        io->startTime = (tp.tv_sec * 1000) + (tp.tv_usec/1000);
    }
    return ii;
}

static int
ffUrlFileClose(meIo *io, meUInt rwflag)
{
    if(rwflag & (meRWFLAG_READ|meRWFLAG_WRITE))
    {
        meUByte buff[meBUF_SOCK_SIZE_MAX];
        if(ffUrlTypeIsFtp(io->type))
        {
            /* close data channel */
            meSockClose(io,0);
            /* get the transfer complete */
            if(meSockFtpReadReply(io,buff) != ftpPOS_COMPLETE)
                mlwrite(MWABORT|MWPAUSE,(meUByte *)"[Problem occurred during download]");
        }
        if(io->urlBp != NULL)
        {
            struct meTimeval tp;
            int dwldtime, kbsec, kbhsec;
            gettimeofday(&tp,NULL);
            dwldtime = (int) (((tp.tv_sec * 1000) + (tp.tv_usec/1000)) - io->startTime);
            if(dwldtime <= 0)
                dwldtime = 1;
            if(ffread > 0x1000000)
            {
                kbsec = (ffread / 128) * 125;
                if(dwldtime > 100)
                    kbhsec = (kbsec/(dwldtime/100)) % 100;
                else
                    kbhsec = 0;
            }
            else
            {
                kbsec = (ffread * 125) / 128;
                kbhsec = ((kbsec*100)/dwldtime) % 100;
            }
            kbsec /= dwldtime;
            sprintf((char *)buff,"Complete: %d bytes %s in %d.%02d seconds (%d.%02d Kbytes/s)",
                    (int) ffread,(rwflag & meRWFLAG_WRITE) ? "sent":"received",
                    dwldtime/1000,(dwldtime/10) % 100,kbsec,kbhsec);
            ffUrlConsoleAddText(io,buff,0);
            io->urlBp = NULL;
        }
    }
    ffCloseSockets(io,0);
    return meTRUE;
}

#endif

#ifdef _DOS
#define MEOPT_EXTENDED_BACKUP 0
#else
#if MEOPT_EXTENDED
#define MEOPT_EXTENDED_BACKUP 1
#else
#define MEOPT_EXTENDED_BACKUP 0
#endif
#endif

#if MEOPT_EXTENDED_BACKUP
meUByte *
createBackupNameStrcpySub(meUByte *dd, meUByte *ss, int subCount, meUByte **subFrom, meUByte **subTo)
{
    if(subCount != 0)
    {
        meUByte cc;
        int ii, ll;
        while((cc=*ss) != '\0')
        {
            for(ii=0 ; ii<subCount ; ii++)
            {
                ll = meStrlen(subFrom[ii]);
                if(!meStrncmp(ss,subFrom[ii],ll))
                {
                    meStrcpy(dd,subTo[ii]);
                    dd += meStrlen(dd);
                    ss += ll;
                    break;
                }
            }
            if((ii == subCount) || (ll == 0))
            {
                *dd++ = cc;
                ss++;
            }
        }
        *dd = '\0';
    }
    else
    {
        meStrcpy(dd,ss);
        dd += meStrlen(dd);
    }
    return dd;
}
#endif

/*
 * creates the standard bckup name with the extra
 * character(s) being backl, where a '~' is used
 * for backups and '#' for auto-saves
 * returns 1 if fails, else 0 and name in filename buffer
 */
int
createBackupName(meUByte *filename, meUByte *fn, meUByte backl, int flag)
{
    meUByte ft, *s, *t;
#if MEOPT_EXTENDED_BACKUP
    static int     backupPathFlag=0;
    static meUByte *backupPath=NULL;
    static int     backupSubCount=0;
    static meUByte **backupSubFrom=NULL;
    static meUByte **backupSubTo=NULL;
    if(!backupPathFlag)
    {
        if(((s=meGetenv("MEBACKUPPATH")) != NULL) && (meStrlen(s) > 0) &&
           ((backupPath=meStrdup(s)) != NULL))
        {
            fileNameConvertDirChar(backupPath);
#ifdef _DRV_CHAR
            if((backupPath[0] == DIR_CHAR) || (isAlpha(backupPath[0]) && (backupPath[1] == _DRV_CHAR)))
#else
            if(backupPath[0] == DIR_CHAR)
#endif
                backupPathFlag = 1;
            else
                backupPathFlag = 2;
        }
	else
	    backupPathFlag = -1;
	if(((s=meGetenv("MEBACKUPSUB")) != NULL) && (meStrlen(s) > 0))
        {
            meUByte cc, dd;
            int ii;
            
            while((dd=*s++) != '\0')
            {
                if(dd == 's')
                {
                    ii=backupSubCount++;
                    if(((dd = *s++) == '\0') ||
                       ((backupSubFrom=meRealloc(backupSubFrom,backupSubCount*sizeof(meUByte *))) == NULL) ||
                       ((backupSubTo  =meRealloc(backupSubTo  ,backupSubCount*sizeof(meUByte *))) == NULL))
                    {
                        backupSubCount = 0;
                        break;
                    }
                    backupSubFrom[ii] = s;
                    while(((cc=*s) != '\0') && (cc != dd))
                        s++;
                    if(cc != '\0')
                    {
                        *s++ = '\0';
                        backupSubTo[ii] = s;
                        while(((cc=*s) != '\0') && (cc != dd))
                            s++;
                        *s++ = '\0';
                    }
                    if(cc == '\0')
                    {
                        backupSubCount = 0;
                        break;
                    }
                }
            }
        }
    }
#endif
    ft = ffUrlGetType(fn);
    if(ffUrlTypeIsPipe(ft))
        return 1;
#if MEOPT_SOCKET
    if((backl == '#') && ffUrlTypeIsHttpFtp(ft))
    {
        meUByte tmp[meBUF_SIZE_MAX];
        s = getFileBaseName(fn);
        meStrcpy(tmp,s);
        s = tmp;
        while((s=meStrchr(s,DIR_CHAR)) != NULL)  /* got a '/', -> '_' */
            *s++ = '_';
        mkTempName(filename,tmp,NULL);
    }
    else
#endif
#if MEOPT_EXTENDED_BACKUP
        if((backupPathFlag > 0) && !ffUrlTypeIsHttpFtp(ft))
    {
        if(backupPathFlag == 1)
        {
            meStrcpy(filename,backupPath);
            t = filename + meStrlen(backupPath);
            s = fn;
            if(s[0] == DIR_CHAR)
                s++;
            createBackupNameStrcpySub(t,s,backupSubCount,backupSubFrom,backupSubTo);
        }
        else
        {
            int ll;
            s = getFileBaseName(fn);
            ll = ((size_t)s - (size_t)fn);
            meStrncpy(filename,fn,ll);
            t = filename+ll;
            t = createBackupNameStrcpySub(t,backupPath,backupSubCount,
                                          backupSubFrom,backupSubTo);
            createBackupNameStrcpySub(t,s,backupSubCount,backupSubFrom,backupSubTo);
        }
#ifdef _DRV_CHAR
        /* ensure the path has no ':' in it - breaks every thing, change to a '_' */
        while((t=meStrchr(t,_DRV_CHAR)) != NULL)
            *t++ = '_';
#endif
        if(flag & meBACKUP_CREATE_PATH)
        {
            meIo tio;
            int ii=0;
            while(((t=meStrrchr(filename,DIR_CHAR)) != NULL) && (t != filename))
            {
                *t = '\0';
                if(!meTestExist(filename))
                {
                    if(meTestDir(filename))
                        return 1;
                    *t = DIR_CHAR;
                    break;
                }
                ii++;
            }
            while(--ii >= 0)
            {
                if(ffWriteFileOpen(&tio,filename,meRWFLAG_MKDIR|meRWFLAG_SILENT,NULL) <= 0)
                    return 1;
                filename[meStrlen(filename)] = DIR_CHAR;
            }
        }
    }
    else
#endif
        meStrcpy(filename,fn);
    
#ifdef _UNIX
    if((backl == '~') && ((meSystemCfg & (meSYSTEM_DOSFNAMES|meSYSTEM_HIDEBCKUP)) == meSYSTEM_HIDEBCKUP))
    {
        if((t=meStrrchr(filename,DIR_CHAR)) == NULL)
            t = filename;
        else
            t++;
        s = filename + meStrlen(filename) + 1;
        do
            s[0] = s[-1];
        while(--s != t);
        s[0] = '.';
    }
#endif
    t = filename + meStrlen(filename);
#ifndef _DOS
    if(meSystemCfg & meSYSTEM_DOSFNAMES)
#endif
    {
        s = t;
        while(--s != filename)
            if((*s == '.') || (*s == DIR_CHAR))
                break;
        if(*s != '.')
        {
            *t = '.';
            t[1] = backl;
            t[2] = backl;
            t[3] = backl;
            t[4] = '\0';
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
            t = s;
            if(t[2] == '\0')
                t[2] = backl;
            t[3] = backl;
            t[4] = '\0';
        }
        if(!meStrcmp(filename,fn))
            return 1;
    }
#ifndef _DOS
    else
    {
        *t++ = backl;
        *t   = 0;
    }
#endif
    return 0;
}

int
ffgetBuf(meIo *io,int offset, int len)
{
#if MEOPT_TFS
    if(ffUrlTypeIsTfs(io->type))
    {
        if((ffremain = tfs_fread(ffbuf+offset,len,io->tfsp)) <= 0)
        {
            /* TODO - this does not handle errors, just assumed to work */
            return meFALSE;
        }
    }
    else
#endif
#if MEOPT_SOCKET
        if(ffUrlTypeIsHttpFtp(io->type))
    {
        if(io->length < 0)
        {
            if(io->length == -2)
                return meFALSE;
            ffremain = meFIOBUFSIZ;
        }
        else if((ffremain = io->length-ffread) <= 0)
            return meFALSE;
        if(ffremain > len)
            ffremain = len;
        ffremain = meSockRead(io,ffremain,ffbuf+offset,0);
        if(ffremain <= 0)
        {
            if(io->length != -1)
                return mlwrite(MWABORT,(meUByte *)"[File socket read error]");
            io->length = -2;
            return meFALSE;
        }
        if(io->urlOpts & meSOCKOPT_SHOW_PROGRESS)
            ffUrlConsoleAddText(io,(meUByte *)"#",(meLineGetLength(io->urlBp->dotLine) >= frameCur->width - 2) ? 0x02:0x03);
    }
    else
#endif
#ifdef _WIN32
    {
        if(ReadFile(io->fp,ffbuf+offset,len,(DWORD *)&ffremain,NULL) == 0)
        {
            ffremain = 0;
            if((io->fp != GetStdHandle(STD_INPUT_HANDLE)) || (GetLastError() != ERROR_BROKEN_PIPE))
                return mlwrite(MWABORT,(meUByte *)"[File read error %d]",meFileGetError());
        }
        if(ffremain <= 0)
            return meFALSE;
    }
#else
    {
        if((ffremain = fread(ffbuf+offset,1,len,io->fp)) <= 0)
        {
            if(ferror(io->fp))
                return mlwrite(MWABORT,(meUByte *)"[File read error %d]",meFileGetError());
            return meFALSE;
        }
    }
#endif
    ffread += ffremain;
    return meTRUE;
}

/*
 * Read a line from a file and create a new line to hold it.
 * If mode & MDBINARY then loading it in binary mode an a line
 * is upto 16 bytes long.
 *
 * Returns meABORT if a serious system problem arose (read or malloc
 * failure).
 * Returns Error is nothing was read cos at the end of the file
 * Else success so returns meTRUE.
 */
static int
ffgetline(meIo *io, meLine **line)
{
    meLine  *lp;
    meUByte *text, *endp, *lle=NULL;
    meUByte cc, cs, ecc=0;
    int len=0, newl, ii;
    
    do {
        if(ffremain <= 0)
        {
            ffcur = ffbuf;
#if MEOPT_CRYPT
            if(io->flags & meIOFLAG_CRYPT)
            {
                if(cryCtx.blk[5])
                    ffremain = 0;
                else
                {
                    /* always read 1 byte more than a block so we know when we hit the end of file */
                    ii=(2*meCRYPT_BLK_SIZE)+1;
                    ffbuf[2*meCRYPT_BLK_SIZE] = (meUByte) cryCtx.blk[4];
                    do {
                        if(ffgetBuf(io,ii,meFIOBUFSIZ+1-ii) < 0)
                            return meABORT;
                        if(ffremain <= 0)
                        {
                            /* no more to read, must sort out padding after decrypt */
                            cryCtx.blk[5] = 1;
                            break;
                        }
                    } while((ii += ffremain) <= meFIOBUFSIZ);
                    if(ii > meFIOBUFSIZ)
                    {
                        meAssert(ii == meFIOBUFSIZ+1);
                        cryCtx.blk[4] = ffbuf[meFIOBUFSIZ];
                        ii--;
                    }
                    else if((ii & (meCRYPT_BLK_SIZE-1)) != 0)
                        return mlwrite(MWABORT,(meUByte *)"[File block read error %d]",ii);
                    ffremain = ii-(2*meCRYPT_BLK_SIZE);
                    meDecrypt(ffbuf,ffremain);
                    if(cryCtx.blk[5])
                        /* remove padding */
                        ffremain -= ffbuf[ffremain-1];
                }
            }
            else
#endif
                if(ffgetBuf(io,0,meFIOBUFSIZ) < 0)
                return meABORT;
            if(ffremain <= 0)
            {
                if(ecc)
                {
                    ffremain = 0;
                    *ffcur = '\0';
                }
                else
                    ffremain = -1;
                break;
            }
            ffbuf[ffremain]='\n';
        }
        if(io->flags & (meIOFLAG_BINMOD|meIOFLAG_RBNMOD))
        {
            int bpl = (io->flags & meIOFLAG_BINMOD) ? meBINARY_BPL : meRBIN_BPL;
            if(len == 0)
            {
                ii = (io->flags & meIOFLAG_BINMOD) ? (meBINARY_BPL*4)+14 : 2*meRBIN_BPL;
                if((lp = meLineMalloc(ii,0)) == NULL)
                    return meABORT;
            }
            if(ffremain < (bpl-len))
            {
                memcpy(lp->text+len,ffcur,ffremain);
                len += ffremain;
                ffremain = -1;
            }
            else
            {
                ii = bpl-len;
                memcpy(lp->text+len,ffcur,ii);
                ffremain -= ii;
                ffcur += ii;
                len = bpl;
            }
        }
        else
        {
            endp = ffcur;
            newl = 0;
            if(ecc == 0)
            {
                if((ii=meLINE_LLEN_MAX-len) < ffremain)
                {
                    lle = ffcur+ii;
                    cs = *lle;
                    *lle = '\n';
                }
                while(((ecc=*ffcur++) != '\n') && (ecc != '\r'))
                    if(ecc == '\0')
                        io->flags |= meIOFLAG_BINARY;
                    else
                        newl++;
                
                ffremain -= ((size_t) ffcur) - ((size_t) endp);
                
                if(newl)
                {
                    if(len == 0)
                    {
                        len = newl;
                        if((lp = (meLine *) meLineMalloc(len,0)) == NULL)
                            return meABORT;
                        text = lp->text;
                    }
                    else
                    {
                        size_t ss;
                        ii = len;
                        len += newl;
                        ss = meLineMallocSize(len);
                        if((lp = (meLine *) meRealloc(lp,ss)) == NULL)
                            return meABORT;
                        text = lp->text + ii;
                        lp->unused = (meUByte) (ss - len - meLINE_SIZE);
                        lp->length = len;
                    }
                    if(ecc == '\r')
                        ffcur[-1] = '\n';
                    while((cc = *endp++) != '\n')
                        if(cc != '\0')
                            *text++ = cc;
                    *text = '\0';
                }
                else if((len == 0) && ((lp = (meLine *) meLineMalloc(0,0)) == NULL))
                    return meABORT;
                if(lle != NULL)
                {
                    *lle = cs;
                    if(ffcur > lle)
                    {
                        ffcur--;
                        ffremain++;
                    }
                    else
                        lle = NULL;
                }
            }
            if(ffremain >= 0)
            {
                if((ffremain == 0) && (ecc == '\r'))
                    /* the cr is the last char in the buffer, we cannot check if the next char is a '\r' so must get more */
                    ffremain = -1;
            }
            else
                ecc = 0;
        }
    } while(ffremain < 0);
    
    if(io->flags & (meIOFLAG_BINMOD|meIOFLAG_RBNMOD))
    {
        if(len == 0)
            return meFALSE;
        
        if(io->flags & meIOFLAG_BINMOD)
        {
            meUInt cpos;
            meUByte cc;
            
            text = lp->text;
            newl = meBINARY_BPL;
            while(--newl >= 0)
            {
                if(newl < len)
                {
                    cc = text[newl];
                    text[(newl*3)+10] = hexdigits[cc >> 4];
                    text[(newl*3)+11] = hexdigits[cc & 0x0f];
                    text[(newl*3)+12] = ' ';
                    if(!isDisplayable(cc))
                        cc = '.';
                }
                else
                {
                    text[(newl*3)+10] = ' ';
                    text[(newl*3)+11] = ' ';
                    text[(newl*3)+12] = ' ';
                    cc = ' ';
                }
                text[newl+(meBINARY_BPL*3)+14] = cc;
            }
            cpos = io->offset + (meUInt) ((ffread - ffremain - 1) & ~((meInt) (meBINARY_BPL-1)));
            newl=8;
            while(--newl >= 0)
            {
                text[newl] = hexdigits[cpos&0x0f];
                cpos >>= 4;
            }
            text[8] = ':';
            text[9] = ' ';
            
            text[(meBINARY_BPL*3)+10] = ' ';
            text[(meBINARY_BPL*3)+11] = '|';
            text[(meBINARY_BPL*3)+12] = ' ';
            text[(meBINARY_BPL*3)+13] = ' ';
            text[(meBINARY_BPL*4)+14] = '\0';
        }
        else
        {
            meUByte cc;
            
            text = lp->text;
            lp->length = len*2;
            text[len*2] = '\0';
            while(--len >= 0)
            {
                cc = text[len];
                text[(len*2)]   = hexdigits[cc >> 4];
                text[(len*2)+1] = hexdigits[cc & 0x0f];
            }
        }
        *line = lp;
        return meTRUE;
    }
    if(ffremain < 0)
    {
        /* had to break */
        if(len == 0)
            return meFALSE;
        if((len == 1) && (lp->text[0] == 26))
        {
            io->flags |= meIOFLAG_CTRLZ;
            meFree(lp);
            return meFALSE;
        }
        /* Don't flag as the end, otherwise we won't get a proper linep */
        /* mlwrite(MWCURSOR|MWPAUSE,"Warning - Incomplete last line");*/
        /* line has no \n */
        lp->flag = meLINE_NOEOL;
    }
    else if(lle != NULL)
    {
        /* had to break because line was too long - this 'line' therefore has no \n  */
        lp->flag = meLINE_NOEOL;
    }
    else
    {
        /* If we ended in '\r' check for a '\n' */
        meAssert((ecc == '\n') || (ecc == '\r'));
        if(ecc == '\r')
        {
            if(*ffcur == '\n')
            {
                if(io->flags & meIOFLAG_NOTSET)
                    io->flags = (io->flags & ~meIOFLAG_NOTSET) | (meIOFLAG_CR|meIOFLAG_LF);
                else if((io->flags & (meIOFLAG_CR|meIOFLAG_LF)) != (meIOFLAG_CR|meIOFLAG_LF))
                    io->flags |= meIOFLAG_LTDIFF;
                ffremain--;
                ffcur++;
            }
            else if(io->flags & meIOFLAG_NOTSET)
                io->flags = (io->flags & ~meIOFLAG_NOTSET) | meIOFLAG_CR;
            else if((io->flags & (meIOFLAG_CR|meIOFLAG_LF)) != meIOFLAG_CR)
                io->flags |= meIOFLAG_LTDIFF;
        }
        else if(io->flags & meIOFLAG_NOTSET)
            io->flags = (io->flags & ~meIOFLAG_NOTSET) | meIOFLAG_LF;
        else if((io->flags & (meIOFLAG_CR|meIOFLAG_LF)) != meIOFLAG_LF)
            io->flags |= meIOFLAG_LTDIFF;
    }
    *line = lp;
    
    return meTRUE;
}

int
ffReadFileOpen(meIo *io, meUByte *fname, meUInt flags, meBuffer *bp)
{
    io->flags = meIOFLAG_NOTSET;
    if(bp != NULL)
    {
        if(meModeTest(bp->mode,MDBINARY))
            io->flags = meIOFLAG_BINMOD;
        else if(meModeTest(bp->mode,MDRBIN))
            io->flags = meIOFLAG_RBNMOD;
#if MEOPT_CRYPT
        if(meModeTest(bp->mode,MDCRYPT))
            flags |= meRWFLAG_CRYPT;
#endif
    }
    
#ifdef _UNIX
    /* must stop any signals or a socket connection may fail or a read */
    meSigHold();
#endif
    /* If meIOTYPE_PIPE the fp handle must be already set, if file type should be NONE as any file: prefix should have been removed by now */
    if(ffUrlTypeIsHttpFtp(io->type))
    {
#if MEOPT_SOCKET
        int rr;
        io->redirect = 0;
        if((rr=ffUrlFileOpen(io,flags,fname,bp)) <= 0)
        {
#ifdef _UNIX
            meSigRelease();
#endif
            return rr;
        }
#else
#ifdef _UNIX
        meSigRelease();
#endif
        return mlwrite(MWABORT|MWPAUSE,(meUByte *) "[No url support in this version]");
#endif
    }
    else if (ffUrlTypeIsTfs(io->type))
    {
#if MEOPT_TFS
        tfsMount *tfsh;
        meUByte *fn;
        if((fn=meStrstr(fname,"?/")) != NULL)
        {
            *fn = '\0';
            tfsh = tfs_mount(fname+6);
            *fn++ = '?';
        }
        else
        {
            tfsh = tfsdev;
            fn = fname+5;
        }
        if(tfsh != NULL)
        {
            io->tfsp = tfs_fopen(tfsh,fn);
            if(tfsh != tfsdev)
                tfs_umount(tfsh);
        }
        else
            io->tfsp = NULL;
        if(io->tfsp == NULL)
        {
            if(!(flags & meRWFLAG_SILENT))
                /* File not found.      */
                mlwrite(MWCLEXEC,(meUByte *)"[%s: No such TFS file]",fname);
#ifdef _UNIX
            meSigRelease();
#endif
            return -2;
        }
#else
#ifdef _UNIX
        meSigRelease();
#endif
        return mlwrite(MWABORT|MWPAUSE,(meUByte *) "[No tfs support in this version]");
#endif
    }
    else if(!ffUrlTypeIsPipe(io->type))
    {
#ifdef _WIN32
        /* On windows a file could be temporarily lock be another process (e.g. virus scanner)
         * so if the failure is ERROR_SHARING_VIOLATION wait and try again */
        int retries=10;
        for(;;)
        {
            if(((io->fp=CreateFile((const char *) fname,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,
                                   FILE_ATTRIBUTE_NORMAL,NULL)) != INVALID_HANDLE_VALUE) ||
               (GetLastError() != ERROR_SHARING_VIOLATION) ||
               (--retries == 0))
                break;
            Sleep(100);
        }
        if(io->fp == INVALID_HANDLE_VALUE)
#else
        if ((io->fp=fopen((char *)fname, "rb")) == NULL)
#endif
        {
            if(!(flags & meRWFLAG_SILENT))
                /* File not found.      */
                mlwrite(MWCLEXEC,(meUByte *)"[%s: No such file]", fname);
#ifdef _UNIX
            meSigRelease();
#endif
            return -2;
        }
    }
#if MEOPT_CRYPT
    if(flags & meRWFLAG_CRYPT)
    {
        int ii=0, ll;
        do {
            ffgetBuf(io,ii,meCRYPT_BLK_SIZE-ii);
            ii += ffremain;
        } while(ii < meCRYPT_BLK_SIZE);
        memcpy(cryCtx.blk,ffbuf,meCRYPT_BLK_SIZE);
        ll = ((cryCtx.blk[1] >> 17) & 0x00f) + 1;
        ii = 0;
        do
        {
            ffgetBuf(io,ii,ll-ii);
            ii += ffremain;
        } while(ii < ll);
        cryCtx.blk[4] = ffbuf[ll-1];
        cryCtx.blk[5] = 0;
        io->flags |= meIOFLAG_CRYPT;
    }
#endif
    ffremain = 0;
    ffread = 0;
    return meTRUE;
}

/* close the read file handle, Ignore any close error */
void
ffReadFileClose(meIo *io, meUInt flags)
{
    /* Windows pipeCommand comes in as a pipe because the source file
     * has already been opened, but from now one treat it as a normal
     * file.
     * The output file has been set up as a temporary file with the
     * 'delete when closed' flag.
     */
    if(ffUrlTypeIsHttpFtp(io->type))
    {
#if MEOPT_SOCKET
        ffUrlFileClose(io,meRWFLAG_READ);
#endif
    }
    else if (ffUrlTypeIsTfs(io->type))
    {
#if MEOPT_TFS
        tfs_fclose(io->tfsp);
        io->tfsp = NULL;
#endif
    }
    else
    {
        /* If meIOTYPE_PIPE the fp handle must be closed by caller (if required) */
        if(!ffUrlTypeIsPipe(io->type))
        {
#ifdef _WIN32
            if((CloseHandle(io->fp) == 0) && !(flags & meRWFLAG_SILENT))
#else
            if((fclose(io->fp) != 0) && !(flags & meRWFLAG_SILENT))
#endif
                mlwrite(MWABORT|MWPAUSE,(meUByte *)"Error closing file");
        }
        io->fp = meBadFile;
    }
#ifdef _UNIX
    meSigRelease();
#endif
    io->type = meIOTYPE_NONE;
}

int
ffReadFile(meIo *io, meUByte *fname, meUInt flags, meBuffer *bp, meLine *hlp,
           meUInt uoffset, meUInt loffset, meInt length)
{
    meLine *lp0, *lp1, *lp2;
    int   ss, nline;
#if MEOPT_SOCKET
    int ff = ((bp != NULL) && (fname != NULL) && (bp->fileName == fname));
#endif
    io->type = ffUrlGetType(fname);
    if(ffReadFileOpen(io,fname,flags,bp) <= 0)
        return meABORT;
    
    io->offset = 0;
    if(length != 0)
    {
        /* partial reading only supported by the insert-file command on regular files */
        if(length < 0)
        {
            meUInt fsu, fsl;
#ifdef _WIN32
            fsl = GetFileSize(io->fp,(DWORD *) &fsu);
#else
#ifdef _LARGEFILE_SOURCE
            off_t fs;
            fseeko(io->fp,0,SEEK_END);
            fs = ftello(io->fp);
            fsu = (meUInt) (fs >> 32);
            fsl = (meUInt) fs;
#else
            fseek(io->fp,0,SEEK_END);
            fsl = ftell(io->fp);
            fsu = 0;
#endif
#endif
            /* ignore the upper offset if moving from the end of the file -
             * can't be bothered to do the maths */
            uoffset = fsu;
            if(loffset <= fsl)
                loffset = fsl - loffset;
            else if(fsu == 0)
                loffset = 0;
            else
            {
                uoffset--;
                loffset = 0xffffffff - (loffset - fsl - 1);
            }
            length = -length;
        }
        io->offset = loffset;
        if(io->flags & (meIOFLAG_BINMOD|meIOFLAG_RBNMOD))
        {
            loffset &= (io->flags & meIOFLAG_BINMOD) ? meBINARY_BPL-1:meRBIN_BPL-1;
            io->offset -= loffset;
            length += loffset;
        }
#ifdef _WIN32
        SetFilePointer(io->fp,io->offset,(PLONG) &uoffset,FILE_BEGIN);
#else
#ifdef _LARGEFILE_SOURCE
        fseeko(io->fp,(((off_t) uoffset) << 32) | ((off_t) io->offset),SEEK_SET);
#else
        fseek(io->fp,io->offset,SEEK_SET);
#endif
#endif
    }
    
    nline = 0;
    lp2 = hlp;	        /* line after  insert */
    lp0 = lp2->prev;	/* line before insert */
    while((ss=ffgetline(io,&lp1)) == meTRUE)
    {
        /* re-link new line between lp0 and lp1 */
        lp0->next = lp1;
        lp1->prev = lp0;
        lp0 = lp1;
        nline++;
        if((length > 0) && ((ffread-ffremain) >= length))
            break;
        if(TTbreakTest(1))
        {
            ss = ctrlg(meFALSE,1);
            break;
        }
    }
    /* complete the link */
    lp2->prev = lp0;
    lp0->next = lp2;
    
#if MEOPT_SOCKET
    /* loading an ftp or http file can lead to a file name change, free
     * off the old one now that we don't need it any more */
    if(ff && (bp->fileName != fname))
        free(fname);
#endif
    if(bp != NULL)
    {
        bp->lineCount += nline;
        if((flags & meRWFLAG_PRESRVFMOD) == 0)
        {
            bp->fileFlag = io->flags & (meIOFLAG_BINARY|meIOFLAG_LTDIFF);
            if(io->type & meIOTYPE_DIR)
                bp->fileFlag |= meBFFLAG_DIR;
            if(io->flags & meIOFLAG_CTRLZ)
            {
                /* this must be a dos file */
                meModeSet(bp->mode,MDCR);
                meModeSet(bp->mode,MDLF);
                meModeSet(bp->mode,MDCTRLZ);
            }
            else if((io->flags & meIOFLAG_NOTSET) == 0)
            {
                meModeClear(bp->mode,MDCTRLZ);
                if(io->flags & meIOFLAG_CR)
                    meModeSet(bp->mode,MDCR);
                else
                    meModeClear(bp->mode,MDCR);
                if(io->flags & meIOFLAG_LF)
                    meModeSet(bp->mode,MDLF);
                else
                    meModeClear(bp->mode,MDLF);
            }
        }
    }
    if(length > 0)
    {
        meUInt el, eu=uoffset;
        el = io->offset + (meUInt) (ffread-ffremain);
        if(el < io->offset)
        {
            eu--;
            el = 0xffffffff - (((meUInt) (ffread-ffremain)) - io->offset - 1);
        }
        sprintf((char *)resultStr,"|0x%x|0x%x|0x%x|0x%x|",io->offset,el,uoffset,eu);
    }
    ffReadFileClose(io,flags);
    return ss;
}

int
ffReadFileToBuffer(meUByte *sfname, meUByte *buff, meInt buffLen)
{
    int rr;
    buff[0] = '\0';
    meior.type = ffUrlGetType(sfname);
    meAssert((meior.type & meIOTYPE_FILE) == 0);
    if((rr=ffReadFileOpen(&meior,sfname,meRWFLAG_READ|meRWFLAG_SILENT,NULL)) <= 0)
        return rr;
    if(((rr=ffgetBuf(&meior,0,meFIOBUFSIZ)) >= 0) && (ffremain > 0))
    {
        
        if(ffremain >= buffLen)
            ffremain = buffLen-1;
        memcpy(buff,ffcur,ffremain);
        buff[ffremain] = '\0';
    }
    ffReadFileClose(&meior,meRWFLAG_READ|meRWFLAG_SILENT);
    return (rr < 0) ? meABORT:meTRUE;
}

static int
ffputBuf(meIo *io)
{
#if MEOPT_CRYPT
    if(io->flags & meIOFLAG_CRYPT)
        meEncrypt(ffbuf,ffremain);
#endif
#if MEOPT_SOCKET
    if(ffUrlTypeIsFtp(io->type))
    {
        if(meSockWrite(io,ffremain,ffbuf,0) < 0)
        {
            io->flags |= meIOFLAG_ERROR;
            return meABORT;
        }
        else if(io->urlOpts & meSOCKOPT_SHOW_PROGRESS)
            ffUrlConsoleAddText(io,(meUByte *)"#",(meLineGetLength(io->urlBp->dotLine) >= frameCur->width - 2) ? 0x02:0x03);
    }
    else
#endif
    {
#ifdef _WIN32
        meInt written;
        if((WriteFile(io->fp,ffbuf,ffremain,(DWORD *)&written,NULL) == 0) || (written != ffremain))
        {
            io->flags |= meIOFLAG_ERROR;
            return mlwrite(MWABORT,(meUByte *)"[Write failed - Error %d]",meFileGetError());
        }
#else
        if(((int) fwrite(ffbuf,1,ffremain,io->fp)) != ffremain)
        {
            io->flags |= meIOFLAG_ERROR;
            return mlwrite(MWABORT,(meUByte *)"[Write failed - Error %d]",meFileGetError());
        }
#endif
    }
    ffremain = 0;
    return 0;
}

#define ffputc(io,c)                                                         \
(((ffbuf[ffremain++]=c),(ffremain == meFIOBUFSIZ))? ffputBuf(io):0)

int
ffWriteFileOpen(meIo *io, meUByte *fname, meUInt flags, meBuffer *bp)
{
    int ii;
    
    io->flags = 0;
#if MEOPT_CRYPT
    if((bp != NULL) && meModeTest(bp->mode,MDCRYPT) && (meCryptBufferInit(bp) <= 0))
       return meFALSE;
#endif
    io->type = ffUrlGetType(fname);
    if(ffUrlTypeIsPipe(io->type))
    {
#ifdef _WIN32
        io->fp = GetStdHandle(STD_OUTPUT_HANDLE);
#else
        io->fp = stdout;
#endif
#ifdef _UNIX
        meSigHold();
#endif
    }
    else if(ffUrlTypeIsHttpFtp(io->type))
    {
#if MEOPT_SOCKET
        io->redirect = 0;
        if(io->type & meIOTYPE_HTTP)
            return mlwrite(MWABORT|MWPAUSE,(meUByte *) "[Cannot write to urls of this type]");
#ifdef _UNIX
        /* must stop any signals or the connection will fail */
        meSigHold();
#endif
        if((ii=ffUrlFileOpen(io,(flags & (meRWFLAG_WRITE|meRWFLAG_BACKUP|meRWFLAG_DELETE|meRWFLAG_MKDIR|meRWFLAG_STAT|meRWFLAG_SILENT)),fname,bp)) <= 0)
        {
#ifdef _UNIX
            meSigRelease();
#endif
            return ii;
        }
        if(!(flags & meRWFLAG_WRITE))
        {
            ii = ffUrlFileClose(io,flags & (meRWFLAG_WRITE|meRWFLAG_BACKUP|meRWFLAG_DELETE|meRWFLAG_MKDIR|meRWFLAG_STAT|meRWFLAG_SILENT));
#ifdef _UNIX
            meSigRelease();
#endif
            return ii;
        }
#else
        return mlwrite(MWABORT|MWPAUSE,(meUByte *) "[No url support in this version]");
#endif
    }
    else if(ffUrlTypeIsTfs(io->type))
    {
        return mlwrite(MWABORT|MWPAUSE,(meUByte *) "[Cannot write to TFS]");
    }
    else
    {
#ifdef _WIN32
        meUInt fa = meFileGetAttributes(fname);
        if(fa == INVALID_FILE_ATTRIBUTES)
#else
        if(meTestExist(fname))
#endif
            io->flags |= meIOFLAG_NEWFILE;
        else if(flags & meRWFLAG_BACKUP)
        {
            meUByte filename[meBUF_SIZE_MAX];
            
            ii = createBackupName(filename,fname,'~',1);
            if(!ii)
            {
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
                meUByte filenameOldB[meBUF_SIZE_MAX], *filenameOld;
                
                if(!(meSystemCfg & meSYSTEM_DOSFNAMES))
                {
                    meStrcpy(filenameOldB,fname);
                    meStrcat(filenameOldB,".~a~");
                    filenameOld = filenameOldB;
                    if(!meTestExist(filenameOld) && meUnlink(filenameOld))
                        mlwrite(MWABORT|MWPAUSE,(meUByte *)"[Unable to remove backup file %s]", filenameOld);
                    else if(meRename(fname,filenameOld) && (ffFileOp(fname,filenameOld,meRWFLAG_DELETE,-1) <= 0))
                        mlwrite(MWABORT|MWPAUSE,(meUByte *)"[Unable to backup file to %s]",filenameOld);
                }
                else
                    filenameOld = fname;
#else
#define filenameOld fname
#endif
#if MEOPT_EXTENDED_BACKUP
                if(!(meSystemCfg & meSYSTEM_DOSFNAMES) && (keptVersions > 0))
                {
                    int ll;
                    
                    ll = meStrlen(filename)-1;
                    filename[ll++] = '.';
                    filename[ll++] = '~';
                    for(ii=0 ; ii<keptVersions ; ii++)
                    {
                        sprintf((char *)filename+ll,"%d~",ii);
                        if(meTestExist(filename))
                            break;
                    }
                    if(ii == keptVersions)
                    {
                        if(meUnlink(filename))
                            mlwrite(MWABORT|MWPAUSE,(meUByte *)"[Unable to remove backup file %s]", filename);
                        ii--;
                    }
                    if(ii)
                    {
                        meUByte filename2[meBUF_SIZE_MAX];
                        meStrcpy(filename2,filename);
                        while(ii > 0)
                        {
                            sprintf((char *)filename2+ll,"%d~",ii--);
                            sprintf((char *)filename+ll,"%d~",ii);
                            if(meRename(filename,filename2) && (ffFileOp(filename,filename2,meRWFLAG_DELETE,-1) <= 0))
                            {
                                mlwrite(MWABORT|MWPAUSE,(meUByte *)"[Unable to backup file to %s (%d)]",filename2,meFileGetError());
                                if(meUnlink(filename))
                                {
                                    mlwrite(MWABORT|MWPAUSE,(meUByte *)"[Unable to remove backup file %s]", filename);
                                    break;
                                }
                            }
                        }
                    }
                }
#endif
                if(!meTestExist(filename) && meUnlink(filename))
                    mlwrite(MWABORT|MWPAUSE,(meUByte *)"[Unable to remove backup file %s]", filename);
                else if(meRename(filenameOld,filename) && (ffFileOp(filenameOld,filename,meRWFLAG_DELETE,-1) <= 0))
                    mlwrite(MWABORT|MWPAUSE,(meUByte *)"[Unable to backup file to %s (%d)]",filename,meFileGetError());
                else if(bp != NULL)
                {
                    meUShort ss;
#ifdef _DOS
                    ss = bp->stats.stmode & ~(meFILE_ATTRIB_READONLY|meFILE_ATTRIB_HIDDEN);
                    if(meSystemCfg & meSYSTEM_HIDEBCKUP)
                        ss |= meFILE_ATTRIB_HIDDEN;
#endif
#ifdef _WIN32
                    ss = bp->stats.stmode & ~(FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_READONLY);
                    if((meSystemCfg & (meSYSTEM_DOSFNAMES|meSYSTEM_HIDEBCKUP)) == meSYSTEM_HIDEBCKUP)
                        ss |= FILE_ATTRIBUTE_HIDDEN;
#endif
#ifdef _UNIX
                    ss = bp->stats.stmode | S_IWUSR;
#endif
                    if(ss != bp->stats.stmode)
                        meFileSetAttributes(filename,ss);
                }
            }
            else
                mlwrite(MWABORT|MWPAUSE,(meUByte *)"[Unable to backup file %s]",fname);
        }
        else if(flags & meRWFLAG_DELETE)
        {
            /* if backing up as well the file is already effectively deleted */
#ifdef _WIN32
            if(fa & (FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM))
                meFileSetAttributes(fname,FILE_ATTRIBUTE_NORMAL);
            if(fa & FILE_ATTRIBUTE_DIRECTORY)
#else
            if(!meTestDir(fname))
#endif
            {
                io->type |= meIOTYPE_DIR;
#ifdef _WIN32
                ii = (RemoveDirectory((const char *)fname) == 0);
#else
                ii = rmdir((char *) fname);
#ifdef _UNIX
                if(ii && (errno == ENOTDIR))
                    /* this happens when its a symbolic link */
                    ii = meUnlink(fname);
#endif
#endif
            }
            else
                ii = meUnlink(fname);
            if(ii)
            {
                mlwrite(MWCLEXEC|MWABORT,(meUByte *)"[Failed to delete file \"%s\"]",fname);
                return -5;
            }
        }
        if(!(flags & meRWFLAG_WRITE))
        {
            if(flags & meRWFLAG_MKDIR)
            {
                io->type |= meIOTYPE_DIR;
#ifdef _WIN32
                if(CreateDirectory((const char *)fname,NULL) == 0)
#else
#ifdef _DOS
                if(mkdir((char *)fname,0) != 0)
#else
                if(mkdir((char *)fname,meXUmask) != 0)
#endif
#endif
                {
                    mlwrite(MWABORT,(meUByte *)"[Failed to create directory \"%s\"]",fname);
                    return -4;
                }
            }
            return meTRUE;
        }
#ifdef _WIN32
        {
            DWORD create;
            if(flags & meRWFLAG_OPENTRUNC)
                create = TRUNCATE_EXISTING;
            else if(flags & meRWFLAG_OPENEND)
                create = OPEN_ALWAYS;
            else
                create = CREATE_ALWAYS;
            
            /* Cannot write to a readonly file, and cannot write to hidden or system file if attribs dont include these flag */
            if(!(io->flags & meIOFLAG_NEWFILE) && (fa & (FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM)))
                meFileSetAttributes(fname,FILE_ATTRIBUTE_NORMAL);
            
            /* Windows must open the file with the correct permissions to support the compress attribute */
            if((io->fp=CreateFile((const char *) fname,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ,NULL,create,
                                  ((bp == NULL) ? meUmask:bp->stats.stmode),
                                  NULL)) == INVALID_HANDLE_VALUE)
            {
                mlwrite(MWABORT,(meUByte *)"[Cannot open file \"%s\" for writing - %d]",fname,GetLastError());
                return -3;
            }
            if(flags & meRWFLAG_OPENEND)
                SetFilePointer(io->fp,0,NULL,FILE_END);
        }
#else
        {
            char *ss;
            if(flags & meRWFLAG_OPENEND)
                ss = "ab";
            else
                ss = "wb";
            if((io->fp=fopen((char *)fname,ss)) == NULL)
            {
                mlwrite(MWABORT,(meUByte *)"[Cannot open file \"%s\" for writing]",fname);
                return -3;
            }
        }
#endif
        mlwrite(MWCURSOR|MWCLEXEC,(meUByte *)"[%sriting %s]",
                (flags & meRWFLAG_AUTOSAVE) ? "Auto-w":"W",fname);	/* tell us were writing */
#ifdef _UNIX
        meSigHold();
#endif
    }
    ffremain = 0;
    if(bp != NULL)
    {
#if MEOPT_CRYPT
        if(meModeTest(bp->mode,MDCRYPT))
            flags |= meRWFLAG_CRYPT;
#endif
        if(meModeTest(bp->mode,MDXLATE))
            /* translateBufferBack must have been called already returned an rbin formatted line loop */
            io->flags = meIOFLAG_RBNMOD;
        else
        {
            if(meModeTest(bp->mode,MDBINARY))
                io->flags = meIOFLAG_BINMOD;
            else if(meModeTest(bp->mode,MDRBIN))
                io->flags = meIOFLAG_RBNMOD;
            else
            {
                io->flags = 0;
                if(meModeTest(bp->mode,MDCR))
                    io->flags |= meIOFLAG_CR;
                if(meModeTest(bp->mode,MDLF))
                    io->flags |= meIOFLAG_LF;
                if(meModeTest(bp->mode,MDCTRLZ))
                    io->flags |= meIOFLAG_CTRLZ;
            }
#if MEOPT_NARROW
            if((bp->narrow != NULL) && !(flags & meRWFLAG_IGNRNRRW))
                meBufferExpandNarrowAll(bp);
#endif
        }
#if MEOPT_TIMSTMP
        if(!(flags & meRWFLAG_AUTOSAVE))
            set_timestamp(bp);
#endif
    }
    else
        io->flags = meIOFLAG_CR;
    
#if (defined _DOS)
    /* the directory time stamps on dos are not updated so the best
     * we can do is flag that the dirLists are out of date when WE
     * create a new file. Obviously if something else creates a file
     * then that file many be missed in the completion lists etc
     */
    if(io->flags & meIOFLAG_NEWFILE)
    {
#if MEOPT_EXTENDED
        extern meDirList fileNames;
        fileNames.stmtime = 1;
#endif
        curDirList.stmtime = 1;
    }
#endif
#if MEOPT_CRYPT
    if(flags & meRWFLAG_CRYPT)
    {
        /* create random junk to start the file which must not be encypted
         * so must do this before setting the crypt flag */
        ffremain = meCRYPT_BLK_SIZE + ((cryCtx.blk[1] >> 17) & 0x00f);
        memcpy(ffbuf,cryCtx.blk,ffremain);
        if(ffputBuf(io))
            return meABORT;
        io->flags |= meIOFLAG_CRYPT;
    }
#endif
    return meTRUE;
}

int
ffWriteFileWrite(meIo *io, int len, meUByte *buff, int eolFlag)
{
    meUByte cc, dd;
    
    if(io->flags & (meIOFLAG_BINMOD|meIOFLAG_RBNMOD))
    {
        if(io->flags & meIOFLAG_BINMOD)
        {
            while(--len >= 0)
                if(*buff++ == ':')
                    break;
            for(;;)
            {
                while((--len >= 0) && ((cc = *buff++) == ' '))
                    ;
                if((len < 0) || (cc == '|'))
                    break;
                if(!isXDigit(cc) || (--len < 0) || ((dd = *buff++),!isXDigit(dd)))
                {
                    io->flags |= meIOFLAG_ERROR;
                    return mlwrite(MWABORT|MWPAUSE,(meUByte *)"Binary format error");
                }
                cc = (hexToNum(cc)<<4) | hexToNum(dd);
                if(ffputc(io,cc))
                    return meABORT;
            }
        }
        else
        {
            while(--len >= 0)
            {
                cc = *buff++;
                if(!isXDigit(cc) || (--len < 0) || ((dd = *buff++),!isXDigit(dd)))
                {
                    io->flags |= meIOFLAG_ERROR;
                    return mlwrite(MWABORT|MWPAUSE,(meUByte *)"[rbin format error]");
                }
                cc = (hexToNum(cc)<<4) | hexToNum(dd);
                if(ffputc(io,cc))
                    return meABORT;
            }
        }
    }
    else
    {
        if((ffremain+len) < meFIOBUFSIZ)
        {
            memcpy(ffbuf+ffremain,buff,len);
            ffremain += len;
        }
        else
        {
            while(--len >= 0)
            {
                cc=*buff++;
                if(ffputc(io,cc))
                    return meABORT;
            }
        }
        if(eolFlag)
        {
            if(((io->flags & meIOFLAG_CR) && ffputc(io,'\r')) ||
               ((io->flags & meIOFLAG_LF) && ffputc(io,'\n')))
                return meABORT;
        }
    }
    return meTRUE;
}

int
ffWriteFileClose(meIo *io, meUInt flags, meBuffer *bp)
{
    int ret;
    
    /* add a ^Z at the end of the file if needed */
    ret = meABORT;
    if(!(io->flags & meIOFLAG_ERROR) && (!(io->flags & meIOFLAG_CTRLZ) || !ffputc(io,26)))
    {
#if MEOPT_CRYPT
        /* Add padding to cope with all file lengths - this code relies on meCRYPT_BLK_SIZE being a power of 2 */
        meAssert(meCRYPT_BLK_SIZE == 16);
        if(io->flags & meIOFLAG_CRYPT)
        {
            int ps=meCRYPT_BLK_SIZE-(ffremain&(meCRYPT_BLK_SIZE-1));
            memset(ffbuf+ffremain,ps,ps);
            ffremain += ps;
        }
#endif
        if((ffremain == 0) || (ffputBuf(io) >= 0))
            ret = meTRUE;
    }
    
#if MEOPT_SOCKET
    if(ffUrlTypeIsHttpFtp(io->type))
    {
        if(ffUrlFileClose(io,flags) <= 0)
            ret = meABORT;
#ifdef _UNIX
        meSigRelease();
#endif
    }
    else
#endif
    {
        if(!ffUrlTypeIsPipe(io->type))
        {
#ifdef _WIN32
            if((CloseHandle(io->fp) == 0) && !(io->flags & meIOFLAG_ERROR))
#else
            if((fclose(io->fp) != 0) && !(io->flags & meIOFLAG_ERROR))
#endif
                ret = mlwrite(MWABORT|MWPAUSE,(meUByte *)"[Error closing file - %d]",meFileGetError());
        }
        io->fp = meBadFile;
#ifdef _UNIX
        meSigRelease();
#endif
    }
    
#if MEOPT_NARROW
    if((bp != NULL) && (bp->narrow != NULL) && !(flags & meRWFLAG_IGNRNRRW))
        meBufferCollapseNarrowAll(bp);
#endif
    return ret;
}

int
ffWriteFile(meIo *io, meUByte *fname, meUInt flags, meBuffer *bp)
{
    meLine *blp, *lp;
    flags |= meRWFLAG_WRITE;
    if((blp = translateBufferBack(bp,flags)) != NULL)
    {
        if(ffWriteFileOpen(io,fname,flags,bp) > 0)
        {
            long noLines;
            
            /* store the number of lines now cos the close may narrow the buffer and the number of
             * lines will be wrong */
            noLines = bp->lineCount+1;
            lp = meLineGetNext(blp);            /* First line.          */
            while(lp != blp)
            {
#if MEOPT_EXTENDED
                if(lp->flag & meLINE_MARKUP)
                    noLines--;
                else
#endif
                {
                    if(ffWriteFileWrite(io,meLineGetLength(lp),meLineGetText(lp),
                                        !(lp->flag & meLINE_NOEOL)) <= 0)
                        break;
                }
                lp = meLineGetNext(lp);
            }
            if(ffWriteFileClose(io,flags,bp) > 0)
            {
                if(blp != bp->baseLine)
                    meLineLoopFree(blp,1);
                if(io->flags & meIOFLAG_NEWFILE)
                    mlwrite(MWCLEXEC,(meUByte *)"[New file %s, Wrote %d lines]",fname,noLines);
                else
                    mlwrite(MWCLEXEC,(meUByte *)"[Wrote %d lines]",noLines);
                return meTRUE;
            }
        }
        if(blp != bp->baseLine)
            meLineLoopFree(blp,1);
    }
    return meFALSE;
}

#if MEOPT_EXTENDED
int
ffFileOp(meUByte *sfname, meUByte *dfname, meUInt dFlags, meInt fileMode)
{
    int rr=meTRUE, r1;
    
    if((dfname != NULL) || ((dFlags & (meRWFLAG_DELETE|meRWFLAG_MKDIR|meRWFLAG_STAT)) != 0))
    {
        /* By now sfname & dfname should have had a 'file:' URL prefix removed */
        meior.type = ffUrlGetType(sfname);
        meAssert((meior.type & meIOTYPE_FILE) == 0);
    }
    if((dfname != NULL) && (dFlags & meRWFLAG_DELETE))
    {
        /* simply move the file if the source is to be deleted and they are regular files,
         * the WriteOpen will handle backups etc (ignore delete failures when dfname is an
         * ftp location as the failure is likely caused by the file not existing */
        if((rr=ffWriteFileOpen(&meiow,dfname,(dFlags & (meRWFLAG_DELETE|meRWFLAG_BACKUP|meRWFLAG_SILENT)),NULL)) <= 0)
        {
            if((rr != -5) || !ffUrlTypeIsFtp(meiow.type))
                return rr;
            rr = meTRUE;
        }
        if(!ffUrlTypeIsNotFile(meior.type) && !ffUrlTypeIsNotFile(meiow.type))
        {
            if(!meRename(sfname,dfname))
            {
                dFlags &= ~meRWFLAG_DELETE;
                dfname = NULL;
            }
            else
                rr = mlwrite(MWABORT,(meUByte *)"[Failed to rename file]");
        }
#if MEOPT_SOCKET
        else if(ffUrlTypeIsFtp(meior.type) && (meior.type == meiow.type))
        {
            meUByte cc, *fn, fnb[meBUF_SIZE_MAX], buff[meBUF_SOCK_SIZE_MAX];
            int ll;
            
            ll = 6;
            if(ffUrlTypeIsSecure(meior.type))
                ll++;
            while(((cc = sfname[ll]) != '\0') && (cc != '/') && (dfname[ll] == cc))
                ll++;
            if((cc == '/') && (dfname[ll] == '/'))
            {
                /* special case, moving a file on a same ftp site (i.e. rename a file) */
                if(ffReadFileOpen(&meior,sfname,(dFlags & meRWFLAG_SILENT),NULL) <= 0)
                    return meABORT;
                fn = sfname + ll;
                if((fn[1] == '~') && (fn[2] == '/'))
                {
                    if(meior.urlHome != NULL)
                    {
                        meStrcpy(fnb,meior.urlHome);
                        r1 = meStrlen(fnb);
                    }
                    else
                        r1 = 0;
                    meStrcpy(fnb+r1,fn+2);
                    fn = fnb;
                }
                if(meSockFtpCommand(&meior,buff,"RNFR %s",fn) != ftpPOS_INTERMED)
                    rr = meFALSE;
                else
                {
                    fn = dfname + ll;
                    if((fn[1] == '~') && (fn[2] == '/'))
                    {
                        if(meior.urlHome != NULL)
                        {
                            meStrcpy(buff,meior.urlHome);
                            r1 = meStrlen(fnb);
                        }
                        else
                            r1 = 0;
                        meStrcpy(fnb+r1,fn+2);
                        fn = fnb;
                    }
                    if(meSockFtpCommand(&meior,buff,"RNTO %s",fn) != ftpPOS_COMPLETE)
                        rr = meFALSE;
                }
                if(rr == meFALSE)
                    rr = mlwrite(MWABORT,(meUByte *)"[Failed to rename ftp file]");
#ifdef _UNIX
                meSigRelease();
#endif
                dFlags &= ~meRWFLAG_DELETE;
                dfname = NULL;
            }
        }
#endif
    }
    if(dfname != NULL)
    {
#ifdef _UNIX
        struct utimbuf fileTimes;
#endif
#ifdef _WIN32
        FILETIME creationTime, lastAccessTime, lastWriteTime;
#endif
        int presTimeStamp;
        
        if((rr=ffReadFileOpen(&meior,sfname,meRWFLAG_READ|(dFlags & (meRWFLAG_NODIRLIST|meRWFLAG_SILENT)),NULL)) <= 0)
            return rr;
        if((rr=ffWriteFileOpen(&meiow,dfname,meRWFLAG_WRITE|(dFlags & (meRWFLAG_BACKUP|meRWFLAG_SILENT)),NULL)) <= 0)
        {
            ffReadFileClose(&meior,meRWFLAG_READ|(dFlags & meRWFLAG_SILENT));
            return rr;
        }
        presTimeStamp = ((dFlags & meRWFLAG_PRESRVTS) && (meior.fp != meBadFile) && (meiow.fp != meBadFile));
        if(presTimeStamp)
        {
#ifdef _UNIX
            struct stat stt;
            if((presTimeStamp = (fstat(fileno(meior.fp),&stt) == 0)))
            {
                fileTimes.actime = stt.st_atime;
                fileTimes.modtime = stt.st_mtime;
            }
#endif
#ifdef _WIN32
            presTimeStamp = GetFileTime(meior.fp,&creationTime,&lastAccessTime,&lastWriteTime);
#endif
        }
        while(((r1=ffgetBuf(&meior,0,meFIOBUFSIZ)) > 0) && (ffremain > 0) && (ffputBuf(&meiow) >= 0))
            ;
        ffReadFileClose(&meior,meRWFLAG_READ|(dFlags & meRWFLAG_SILENT));
        ffremain = 0;
#ifdef _WIN32
        if(presTimeStamp)
            SetFileTime(meiow.fp,&creationTime,&lastAccessTime,&lastWriteTime);
#endif
        rr = ffWriteFileClose(&meiow,meRWFLAG_WRITE|(dFlags & meRWFLAG_SILENT),NULL);
        if(r1 < 0)
            rr = r1;
        else if((rr > 0) && (fileMode >= 0) && !ffUrlTypeIsNotFile(meiow.type))
        {
            meFileSetAttributes(dfname,fileMode);
#ifdef _UNIX
            if(presTimeStamp)
                utime((char *) dfname,&fileTimes);
#endif
        }
    }
    if((rr > 0) && (dFlags & (meRWFLAG_DELETE|meRWFLAG_MKDIR|meRWFLAG_STAT)))
    {
        rr = ffWriteFileOpen(&meior,sfname,dFlags & (meRWFLAG_DELETE|meRWFLAG_MKDIR|meRWFLAG_STAT|meRWFLAG_BACKUP|meRWFLAG_SILENT),NULL);
    }
#if MEOPT_SOCKET
    /* TODO bug: could read from ftp, wait 59 seconds then write to ftp - could get into here to close read io but also closes write io, or read is not closed when it should */
    if((rr > 0) && (dFlags & meRWFLAG_SOCKCLOSE))
    {
        timerClearExpired(SOCKET_TIMER_ID);
        if(meSockIsInUse(&meior) || meSockIsInUse(&meiow))
        {
#ifdef _UNIX
            meSigHold();
#endif
            if(dFlags & meRWFLAG_ATEXIT)
            {
                meior.urlBp = NULL;
                meiow.urlBp = NULL;
            }
            ffCloseSockets(&meior,1);
            ffCloseSockets(&meiow,1);
#ifdef _UNIX
            meSigRelease();
#endif
        }
        if(dFlags & meRWFLAG_ATEXIT)
            meSockEnd();
    }
#endif
    return rr;
}

/* n bits:
 * 0x0003 Base conversion type - 0 = lossless bin, 1 = utf8, 2 = utf16, 3 = utf32 
 * 0x0004 Big-Endian utf16 or utf32 (little endian if not set)
 * 0x0008 Check for & remove Unicode BOM or add BOM on save 
 * 0x0100 utf - test for type (bits 0x0f must not be set) 
 * 0x0200 Convert to new buffer rather than replace current
 * 0x0400 Hide output buffer (used with 0x200)
 * 
 * Default is 0x0100 - ID and translate a Unicode file
 */
static meUByte *uniBom[] = { (meUByte *) "EFBBBF",(meUByte *) "FFFE",(meUByte *) "FFFE0000",NULL,NULL,(meUByte *) "FEFF",(meUByte *) "0000FEFF" };
int
translateBuffer(int f, int n)
{
    meBuffer *bp;
    meUByte cc, ca, cb, c1, c2, c3, c4, flg=0, *dd, *ss;
    meLine *lp, *elp, *blp, *plp, *nlp;
    int ii, uc, rr, lc=0, ll;
    
    if(!f)
        n = 0x0100;
    if(n & 0x0200)
    {
        /* prompt for and get the new buffer name */
        meUByte buf[meBUF_SIZE_MAX];
        if((rr = getBufferName((meUByte *)"Output buffer",0,0,buf)) <= 0)
            return rr;
        if((bp=bfind(buf,BFND_CREAT|BFND_CLEAR)) == NULL)
            return meFALSE;
        blp = bp->baseLine;
    }
    else
    {
        bp = frameCur->windowCur->buffer;
        if(meModeTest(bp->mode,MDEDIT)
#if MEOPT_IPIPES
           || meModeTest(bp->mode,MDPIPE)
#endif
#if MEOPT_NARROW
           || (bp->narrow != NULL)
#endif
#if MEOPT_UNDO
           || (bp->undoHead != NULL)
#endif
           )
            return mlwrite(MWABORT,(meUByte *)"[Current buffer must be clean]");
        if((blp=meLineMalloc(0,0)) == NULL)
            return meABORT;
    }
    nlp = plp = blp;
    elp = frameCur->windowCur->buffer->baseLine;
    lp = meLineGetNext(elp);
    ss = lp->text;
    if((n & 0x010f) == 0x0100)
    {
        ii = 6;
        do {
            if((dd = uniBom[ii]) != NULL)
            {
                ll = meStrlen(dd);
                if(!memcmp(ss,dd,ll))
                    break;
            }
        } while(--ii >= 0);
        if(ii >= 0)
        {
            n |= (ii+1) | 0x0008;
            ss += ll;
        }
        else
            n |= 1;
    }
    else if((n & 0x000f) > 0x0008)
    {
        dd = uniBom[(n & 0x0007) - 1];
        ll = meStrlen(dd);
        if(!memcmp(ss,dd,ll))
            ss += ll;
        else
            n ^= 0x0008;
    }
    dd = ffbuf;
    ll = 0;
    for(;;)
    {
        while(((cc = *ss++) == '\0') &&
              ((lp = meLineGetNext(lp)) != elp))
            ss = lp->text;
        if((lp == elp) || ((cb = *ss++) == '\0'))
            break;
        uc = (((cc < 'A') ? cc-'0':cc-'A'+10) << 4) | ((cb < 'A') ? cb-'0':cb-'A'+10);
        if(n & 0x0002)
        {
            if(((ca = *ss++) == '\0') || ((cb = *ss++) == '\0'))
                break;
            c2 = (((ca < 'A') ? ca-'0':ca-'A'+10) << 4) | ((cb < 'A') ? cb-'0':cb-'A'+10);
            if(n & 0x0001)
            {
                if(((ca = *ss++) == '\0') || ((cb = *ss++) == '\0'))
                    break;
                c3 = (((ca < 'A') ? ca-'0':ca-'A'+10) << 4) | ((cb < 'A') ? cb-'0':cb-'A'+10);
                if(((ca = *ss++) == '\0') || ((cb = *ss++) == '\0'))
                    break;
                c4 = (((ca < 'A') ? ca-'0':ca-'A'+10) << 4) | ((cb < 'A') ? cb-'0':cb-'A'+10);
                if(n & 0x0004)
                    uc = (uc << 24) | (((int) c2) << 16) | (((int) c3) << 8) | c4;
                else
                    uc = (((int) c4) << 24) | (((int) c3) << 16) | (((int) c2) << 8) | uc;
            }
            else
            {
                if(n & 0x0004)
                    uc = (uc << 8) | c2;
                else
                    uc = (((int) c2) << 8) | uc;
                if(((ii=(uc & 0xff00)) >= 0xd800) && (ii <= 0xdfff))
                {
                    if(ii >= 0xdc00)
                        break;
                    while(((ca = *ss++) == '\0') &&
                          ((lp = meLineGetNext(lp)) != elp))
                        ss = lp->text;
                    if((lp == elp) || ((cb = *ss++) == '\0') ||
                       ((c3 = *ss++) == '\0') || ((c4 = *ss++) == '\0'))
                        break;
                    c2 = (((ca < 'A') ? ca-'0':ca-'A'+10) << 4) | ((cb < 'A') ? cb-'0':cb-'A'+10);
                    c3 = (((c3 < 'A') ? c3-'0':c3-'A'+10) << 4) | ((c4 < 'A') ? c4-'0':c4-'A'+10);
                    if(n & 0x0004)
                        ii = (((int) c2) << 8) | c3;
                    else
                        ii = (((int) c3) << 8) | c2;
                    if(((ii & 0xff00) < 0xdc00) || ((ii & 0xff00) > 0xdfff))
                        break;
                    uc = 0x10000+((uc&0x3ff)<<10)+(ii&0x3ff);
                }
            }
        }
        else if((n & 0x0001) && (uc & 0x80))
        {
            if((uc & 0x40) == 0)
                break;
            ii = 0;
            c2 = 0x40;
            c3 = 0;
            do {
                while(((ca = *ss++) == '\0') &&
                      ((lp = meLineGetNext(lp)) != elp))
                    ss = lp->text;
                if((lp == elp) || ((cb = *ss++) == '\0') ||
                   (((c1 = (((ca < 'A') ? ca-'0':ca-'A'+10) << 4) | ((cb < 'A') ? cb-'0':cb-'A'+10)) & 0xc0) != 0x80))
                    break;
                ii = (ii << 6) | (c1 & 0x3f);
                c3 += 6;
            } while((uc & (c2 >>= 1)) != 0);
            uc = ((uc & (c2-1)) << c3) | ii;
        }
        if((uc == 10) || (ll >= meLINE_LLEN_MAX))
        {
            if((n & 0x0003) && (uc == 10))
            {
                if((ll > 0) && (dd[-1] == '\r'))
                {
                    ll--;
                    if(flg == 0)
                        flg = meIOFLAG_CR|meIOFLAG_LF;
                    else if(flg != (meIOFLAG_CR|meIOFLAG_LF))
                        flg |= meIOFLAG_LTDIFF;
                }
                else if(flg == 0)
                    flg = meIOFLAG_LF;
                else if(flg != meIOFLAG_LF)
                    flg |= meIOFLAG_LTDIFF;
            }
            if((nlp=meLineMalloc(ll,0)) == NULL)
                break;
            memcpy(nlp->text,ffbuf,ll);
            nlp->text[ll] = '\0';
            plp->next = nlp;
            nlp->prev = plp;
            plp = nlp;
            lc++;
            dd = ffbuf;
            ll = 0;
            if(uc == 10)
                continue;
            nlp->flag |= meLINE_NOEOL;
        }
        if((uc <= 2) || (uc == 0x07))
        {
            *dd++ = 0x01;
            *dd++ = '0';
            *dd++ = '0';
            *dd++ = '0'+uc;
            ll += 4;
        }
        else if(!(n & 0x0003))
        {
            *dd++ = (meUByte) uc;
            ll++;
        }
        else if((uc < 256) && ((uc < 128) || (charToUnicode[uc-128] == uc)))
        {
            *dd++ = (meUByte) uc;
            ll++;
        }
        else
        {
            ii = 127;
            while((charToUnicode[ii] != uc) && (--ii >= 0))
                ;
            if(ii >= 0)
            {
                *dd++ = (meUByte) (ii+128);
                ll++;
            }
            else if(uc < 0x1000)
            {
                *dd++ = 0x01;
                *dd++ = hexdigits[uc >> 8];
                *dd++ = hexdigits[(uc >> 4) & 0x0f];
                *dd++ = hexdigits[uc & 0x0f];
                ll += 4;
            }
            else if(uc < 0x100000)
            {
                *dd++ = 0x02;
                *dd++ = hexdigits[uc >> 16];
                *dd++ = hexdigits[(uc >> 12) & 0x0f];
                *dd++ = hexdigits[(uc >> 8) & 0x0f];
                *dd++ = hexdigits[(uc >> 4) & 0x0f];
                *dd++ = hexdigits[uc & 0x0f];
                ll += 6;
            }
            else
            {
                *dd++ = meCHAR_UNDEF;
                ll++;
            }
        }
    }
    if((cc == '\0') && (ll > 0))
    {
        if((nlp=meLineMalloc(ll,0)) != NULL)
        {
            memcpy(nlp->text,ffbuf,ll);
            nlp->text[ll] = '\0';
            nlp->flag |= meLINE_NOEOL;
            plp->next = nlp;
            nlp->prev = plp;
            plp = nlp;
            lc++;
        }
        else
            cc = 1;
    }
    blp->prev = plp;
    plp->next = blp;
    if(cc != '\0')
    {
        if(n & 0x0200)
        {
            bp->intFlag |= BIFBLOW;
            zotbuf(bp,clexec);
        }
        else
            meLineLoopFree(blp,1);
        if(nlp == NULL)
            /* ran out of memory */
            return meABORT;
        if((uc = (int) (ss-lp->text) -2) < 0)
            uc = 0;
        ll = 1;
        while((lp = meLineGetPrev(lp)) != elp)
            ll++;
        return mlwrite(MWABORT,(meUByte *)"[Format error in source data at line %d column %d]",ll,uc);
    }
    if(!(n & 0x0200))
    {
        meAnchor *mk;
        while((mk=bp->anchorList) != NULL)
        {
            bp->anchorList = mk->next;
            meFree(mk);
        }
#if MEOPT_LOCALBIND
        if(bp->bindList != NULL)
        {
            meFree(bp->bindList);
            bp->bindList = NULL;
            bp->bindCount = 0;
        }
#endif
        meLineLoopFree(bp->baseLine,1);
        bp->baseLine = blp;
    }
    bp->lineCount = lc; 
    bp->dotLine = meLineGetNext(blp);
    bp->dotLineNo = 0;
    bp->dotOffset = 0;
    bp->dotLineNo = 0;
    bp->markLine = NULL;
    bp->markOffset = 0;
    bp->markLineNo = 0;
    bp->vertScroll = 0;
    bp->tabWidth = tabWidth;
    bp->indentWidth = indentWidth;
#if MEOPT_FILEHOOK
    bp->fhook = bp->dhook = bp->bhook = bp->ehook = -1;
#endif
#if MEOPT_HILIGHT
    bp->hilight = 0;
    bp->indent = 0;
#endif
    bp->fileFlag = (bp->fileFlag & ~(meBFFLAG_BINARY|meBFFLAG_LTDIFF)) | (flg & meIOFLAG_LTDIFF);
    meModeSet(bp->mode,MDXLATE);
    if(flg & meIOFLAG_CR)
        meModeSet(bp->mode,MDCR);
    else
        meModeClear(bp->mode,MDCR);
    meModeSet(bp->mode,MDLF);
    meModeClear(bp->mode,MDRBIN);
    bp->xlateFlag = (n & 0x00ff);
    resetBufferWindows(bp);
    setBufferContext(bp);
    /* the buffer's fhook could have done some really whacky things like
     * delete the buffer. If it has avoid crashing by checking the buffer
     * is still a buffer! */
    {
        meBuffer *tbp = bheadp;
        while(tbp != bp)
            if((tbp=tbp->next) == NULL)
                return meFALSE;
    }
    if((n & 0x0600) == 0x0200)
        meWindowPopup(bp,NULL,0,NULL);
    return meTRUE;
}

meLine *
translateBufferBack(meBuffer *bp, meUInt flags)
{
    meLine *lp, *elp, *blp, *plp, *nlp;
    meUByte xlt, cc, c2, *dd, *ss;
    meUInt uc;
    int ii, ll;
    
    if(!meModeTest(bp->mode,MDXLATE))
        return bp->baseLine;
    if((blp=meLineMalloc(0,0)) == NULL)
        return NULL;
    nlp = plp = blp;
    dd = ffbuf;
    ll = 0;
    xlt = (bp->xlateFlag & 0x07);
    if((bp->xlateFlag & 0x0f) > 0x08)
    {
        ss = uniBom[xlt - 1];
        ll = meStrlen(ss);
        memcpy(dd,ss,ll);
        dd += ll;
    }
    elp = bp->baseLine;
    lp = meLineGetNext(elp);
    ss = lp->text;
    for(;;)
    {
        if((uc = *ss++) == '\0')
        {
            if((lp->flag & meLINE_NOEOL) == 0)
            {
                if(meModeTest(bp->mode,MDCR))
                {
                    if((xlt & 6) == 6)
                    {
                        *dd++ = '0';
                        *dd++ = '0';
                        ll += 2;
                        if(xlt & 1)
                        {
                            *dd++ = '0';
                            *dd++ = '0';
                            *dd++ = '0';
                            *dd++ = '0';
                            ll += 4;
                        }
                    }
                    *dd++ = '0';
                    *dd++ = 'D';
                    ll += 2;
                    if((xlt & 6) == 2)
                    {
                        *dd++ = '0';
                        *dd++ = '0';
                        ll += 2;
                        if(xlt & 1)
                        {
                            *dd++ = '0';
                            *dd++ = '0';
                            *dd++ = '0';
                            *dd++ = '0';
                            ll += 4;
                        }
                    }
                }
                if(meModeTest(bp->mode,MDLF))
                {
                    if((xlt & 6) == 6)
                    {
                        *dd++ = '0';
                        *dd++ = '0';
                        ll += 2;
                        if(xlt & 1)
                        {
                            *dd++ = '0';
                            *dd++ = '0';
                            *dd++ = '0';
                            *dd++ = '0';
                            ll += 4;
                        }
                    }
                    *dd++ = '0';
                    *dd++ = 'A';
                    ll += 2;
                    if((xlt & 6) == 2)
                    {
                        *dd++ = '0';
                        *dd++ = '0';
                        ll += 2;
                        if(xlt & 1)
                        {
                            *dd++ = '0';
                            *dd++ = '0';
                            *dd++ = '0';
                            *dd++ = '0';
                            ll += 4;
                        }
                    }
                }
            }
            if((lp = meLineGetNext(lp)) == elp)
                break;
            ss = lp->text;
        }
        else if(xlt == 0)
        {
            if(uc > 0x02)
            {
                *dd++ = hexdigits[uc >> 4];
                *dd++ = hexdigits[uc & 0x0f];
            }
            else if((uc == 0x02) || (*ss++ != '0') || ((cc = *ss++),!isXDigit(cc)) || ((c2 = *ss++),!isXDigit(c2)))
                break;
            else
            {
                *dd++ = cc;
                *dd++ = c2;
            }
            ll += 2;
        }
        else
        {
            if(uc <= 0x02)
            {
                ii = (uc == 0x02) ? 5:3;
                uc = 0;
                do {
                    cc = *ss++;
                    if(!isXDigit(cc))
                        break;
                    uc = (uc<<4) | hexToNum(cc);
                } while(--ii > 0);
                if(ii)
                {
                    uc = 1;
                    break;
                }
            }
            else if((uc == 0x07) || ((uc >= 128) && (((uc = charToUnicode[uc-128]) == 0) || (uc > 0x10ffff))))
                uc = 0x0fffd;
            if(xlt & 0x02)
            {
                if(xlt & 0x01)
                    ii = 4;
                else if(uc > 0x0ffff)
                {
                    uc -= 0x10000;
                    if(xlt & 0x04)
                        uc = 0xd800dc00 | ((uc & 0x0ffc00) << 6) | (uc & 0x0003ff);
                    else
                        uc = 0xdc00d800 | ((uc & 0x0ffc00) >> 10) | ((uc & 0x0003ff) << 16);
                    ii = 4;
                }
                else
                    ii = 2;
                ll += ii << 1;
                if(xlt & 0x04)
                {
                    while(--ii >= 0)
                    {
                        cc = uc >> (ii << 3) & 0x0ff;
                        *dd++ = hexdigits[cc >> 4];
                        *dd++ = hexdigits[cc & 0x0f];
                    }
                }
                else
                {
                    do {
                        *dd++ = hexdigits[(uc >> 4) & 0x0f];
                        *dd++ = hexdigits[uc & 0x0f];
                        uc >>= 8;
                    } while(--ii > 0);
                }
            }
            else if(uc < 128)
            {
                *dd++ = hexdigits[uc >> 4];
                *dd++ = hexdigits[uc & 0x0f];
                ll += 2;
            }
            else
            {
                if(uc & 0x1ff800)
                {
                    if(uc & 0x1f0000)
                    {
                        *dd++ = 'F';
                        *dd++ = hexdigits[uc >> 18];
                        *dd++ = hexdigits[0x08 | ((uc >> 16) & 0x03)];
                        *dd++ = hexdigits[((uc >> 12) & 0x0f)];
                        ll += 8;
                    }
                    else
                    {
                        *dd++ = 'E';
                        *dd++ = hexdigits[uc >> 12];
                        ll += 6;
                    }
                    *dd++ = hexdigits[0x08 | ((uc >> 10) & 0x03)];
                }
                else
                {
                    *dd++ = hexdigits[0x0c | (uc >> 10)];
                    ll += 4;
                }
                *dd++ = hexdigits[((uc >> 6) & 0x0f)];
                *dd++ = hexdigits[0x08 | ((uc >> 4) & 0x03)];
                *dd++ = hexdigits[(uc & 0x0f)];
            }
        }            
        if(ll >= meLINE_LLEN_MAX)
        {
            if((nlp=meLineMalloc(ll,0)) == NULL)
                break;
            memcpy(nlp->text,ffbuf,ll);
            nlp->text[ll] = '\0';
            plp->next = nlp;
            nlp->prev = plp;
            plp = nlp;
            dd = ffbuf;
            ll = 0;
        }
    }
    if((uc == '\0') && (ll > 0))
    {
        if((nlp=meLineMalloc(ll,0)) != NULL)
        {
            memcpy(nlp->text,ffbuf,ll);
            nlp->text[ll] = '\0';
            plp->next = nlp;
            nlp->prev = plp;
            plp = nlp;
        }
        else
            uc = 1;
    }
    blp->prev = plp;
    plp->next = blp;
    if(uc == '\0')
        return blp;
    meLineLoopFree(blp,1);
    uc = (meUInt) (ss-lp->text);
    ll = 1;
    while((lp = meLineGetPrev(lp)) != elp)
        ll++;
    mlwrite(MWABORT,(meUByte *)"[Format error in source data at line %d column %d]",ll,uc);
    return NULL;
}

#endif
