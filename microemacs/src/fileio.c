/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * fileio.c - File reading and writing routines.
 *
 * Copyright (C) 1988-2009 JASSPA (www.jasspa.com)
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

#if (defined _UNIX) || (defined _DOS) || (defined _WIN32)
#include <errno.h>
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
meUByte charUserLatinTbl[256] = {
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
meUByte charLatinUserTbl[256] = {
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

meUByte isWordMask=CHRMSK_DEFWORDMSK;
meUByte charMaskFlags[]="luhs1234dpPwaAMLUk";
#endif

meUByte *charKeyboardMap=NULL;

/* File read/write buffer size  */
#if MEOPT_LARGEBUF
#define meFIOBUFSIZ 32768
#else
#define meFIOBUFSIZ 2048
#endif
#define meBINARY_BPL       16   /* bytes per line */
#define meRBIN_BPL        256   /* bytes per line */

static meInt     ffread;
static meInt     ffremain;
static meUByte  *ffcur;
static meUByte   ffbuf[meFIOBUFSIZ+1];

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
#include <time.h>

#ifdef _WIN32
#include <io.h>
typedef void (*meATEXIT)(void);

#define SHUT_RDWR            SD_BOTH

#define meSocketGetError WSAGetLastError
#define meSocketOpen     socket
#define meSocketConnect  connect
#define meSocketClose    closesocket
#define meSocketSetOpt   setsockopt
#define meSocketRead     recv
#define meSocketWrite    send
#define meSocketShutdown shutdown
#define meGetservbyname  getservbyname
#define meGethostbyname  gethostbyname
#define meInet_addr      inet_addr
#define meHtons          htons

#else

#include <sys/time.h>
#include <ctype.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/un.h>
#define meSocketGetError()   (errno)

#define meSocketOpen     socket
#define meSocketConnect  connect
#define meSocketClose    close
#define meSocketSetOpt   setsockopt
#define meSocketRead     recv
#define meSocketWrite    send
#define meSocketShutdown shutdown
#define meGetservbyname  getservbyname
#define meGethostbyname  gethostbyname
#define meInet_addr      inet_addr
#define meHtons          htons

#endif

#define meSOCKET_TIMEOUT 240000

static meUByte *ffUrlFlagsVName[2]={(meUByte *)"http-flags",(meUByte *)"ftp-flags"} ;
static meUByte *ffUrlConsoleBName[2]={(meUByte *)"*http-console*",(meUByte *)"*ftp-console*"} ;

#ifndef INADDR_NONE
/* This may not be defined, particularly suns */
#define INADDR_NONE -1
#endif

#define ftpERROR         0
#define ftpPOS_PRELIMIN  1
#define ftpPOS_COMPLETE  2
#define ftpPOS_INTERMED  3

static void
strBase64Encode3(meUByte *dd, meUByte c1, meUByte c2, meUByte c3)
{
    static meUByte base64Table[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" ;
    dd[0] = base64Table[(c1 >> 2)] ;
    dd[1] = base64Table[((c1 << 4) & 0x30) | ((c2 >> 4) & 0x0f)] ;
    dd[2] = base64Table[((c2 << 2) & 0x3c) | ((c3 >> 6) & 0x03)] ;
    dd[3] = base64Table[(c3 & 0x3f)] ;
}

static meUByte *
strBase64Encode(meUByte *dd, meUByte *ss)
{
    meUByte c1, c2, c3 ;

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
make_inet_addr(meIo *io, meUByte *hostname, meUByte *port, meUByte *proto)
{
    struct hostent *hp ;
    struct servent *sp ;

    memset(&(io->sockAddr),0,sizeof(struct sockaddr_in)) ;
    io->sockAddr.sin_family = AF_INET ;

    if ((sp = meGetservbyname((char *)port,(char *)proto)) != NULL)
    {
        io->sockAddr.sin_port = sp->s_port;
    }
    else
    {
        meUShort usport=atoi((char *)port) ;
        if ((io->sockAddr.sin_port = meHtons(usport)) == 0)
            return mlwrite(MWABORT|MWPAUSE,(meUByte *)"[Bad port number %s]", port) ;
    }
    /* First resolve the hostname, then resolve the port */
    if ((io->sockAddr.sin_addr.s_addr = meInet_addr((char *)hostname)) != INADDR_NONE)
    {
        /* in_addr.s_addr is already set */
    }
    else if(((hp = meGethostbyname((char *)hostname)) != NULL) &&
            (hp->h_length <= ((int) sizeof(struct in_addr))) &&
            (hp->h_addrtype == AF_INET))
        memcpy(&(io->sockAddr.sin_addr),hp->h_addr,hp->h_length) ;
    else
        return mlwrite(MWABORT|MWPAUSE,(meUByte *)"[Unknown host %s]", hostname) ;

    return meTRUE ;
}

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

        olp = io->urlBp->dotLine ;
        ll = ii = io->urlBp->dotOffset ;
        meStrncpy(buff,meLineGetText(olp),ii) ;
        while((cc=*str++) != '\0')
            buff[ii++] = cc ;
        meStrcpy(buff+ii,meLineGetText(olp)+ll) ;
        addLine(olp,buff) ;
        nlp = meLineGetPrev(olp) ;
        nlp->next = olp->next ;
        olp->next->prev = nlp ;
        if(olp->flag & meLINE_ANCHOR)
            meLineResetAnchors(meLINEANCHOR_ALWAYS|meLINEANCHOR_RETAIN,io->urlBp,
                              olp,nlp,0,0) ;
        meFree(olp) ;
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
        if(io->urlFlags & meIOURLF_SHOW_CONSOLE)
            screenUpdate(meTRUE,2-sgarbf) ;
#ifdef _WIN32
        TTaheadFlush();
#endif
    }
}

static meSOCKET
ffOpenConnectUrlSocket(meIo *io, meUByte *host, meUByte *port)
{
    meUByte buff[meBUF_SIZE_MAX] ;
    meSOCKET ss ;
    int ii ;

    sprintf((char *)buff,"[Connecting to %s:%s]",host,port);
    if(io->urlBp != NULL)
        ffUrlConsoleAddText(io,buff,0) ;
    mlwrite(MWCURSOR|MWSPEC,buff);

    if(!make_inet_addr(io,host,port,(meUByte *)"tcp"))
        return meBadSocket ;

    if((ss=meSocketOpen(AF_INET,SOCK_STREAM,IPPROTO_TCP)) < 0)
    {
        sprintf((char *)buff,"[Failed to open socket - %d]",meSocketGetError()) ;
        if(io->urlBp != NULL)
            ffUrlConsoleAddText(io,buff,0) ;
        mlwrite(MWABORT|MWPAUSE|MWSPEC,buff) ;
        return meBadSocket ;
    }
    ii = 1 ;
    meSocketSetOpt(ss,SOL_SOCKET,SO_REUSEADDR,(char *) &ii,sizeof(int)) ;
    if(meSocketConnect(ss,(struct sockaddr *) &(io->sockAddr),sizeof(struct sockaddr_in)) != 0)
    {
        meSocketClose(ss) ;
        sprintf((char *)buff,"[Failed to connect to %s:%s - %d]",host,port,meSocketGetError()) ;
        if(io->urlBp != NULL)
            ffUrlConsoleAddText(io,buff,0) ;
        mlwrite(MWABORT|MWPAUSE|MWSPEC,buff) ;
        return meBadSocket ;
    }

    return ss ;
}

static int
ffReadSocketLine(meIo *io,meSOCKET ss, meUByte *buff)
{
    meUByte *dd=buff, cc ;
    for(;;)
    {
        if(meSocketRead(ss,(char *) dd,1,0) <= 0)
        {
            sprintf((char *)buff,"[Failed to read line - %d]",meSocketGetError()) ;
            if(io->urlBp != NULL)
                ffUrlConsoleAddText(io,buff,0) ;
            return mlwrite(MWABORT|MWPAUSE|MWSPEC,buff) ;
        }
        if((cc=*dd) == '\n')
            break ;
        if(cc != '\r')
            dd++ ;
    }
    *dd = '\0' ;
    if(io->urlFlags & meIOURLF_SHOW_DETAILS)
        ffUrlConsoleAddText(io,buff,0) ;
    return meTRUE ;
}

static int
ftpReadReply(meIo *io)
{
    int ret ;

    ffremain = 0 ;
    if(ffReadSocketLine(io,io->ccsk,resultStr) <= 0)
        return meFALSE ;
    if(meStrlen(resultStr) < 4)
        return ftpERROR ;

    ret = resultStr[0] - '0' ;
    if(resultStr[3] == '-')
    {
        /* multi-line reply */
        meUByte c0, c1, c2 ;
        c0 = resultStr[0] ;
        c1 = resultStr[1] ;
        c2 = resultStr[2] ;
        do {
            if(ffReadSocketLine(io,io->ccsk,resultStr) <= 0)
                return meFALSE ;
        } while((resultStr[0] != c0) || (resultStr[1] != c1) ||
                (resultStr[2] != c2) || (resultStr[3] != ' ')) ;
    }
    if(ret > ftpPOS_INTERMED)
        ret = ftpERROR ;
    return ret ;
}

static int
ftpCommand(meIo *io, int flags, char *fmt, ...)
{
    va_list ap;
    int ii ;

    va_start(ap,fmt);
    vsprintf((char *)ffbuf,fmt,ap);
    va_end(ap);
    if(io->urlFlags & meIOURLF_SHOW_DETAILS)
    {
        if(!meStrncmp(ffbuf,"PASS",4))
            ffUrlConsoleAddText(io,(meUByte *)"PASS XXXX",0) ;
        else
            ffUrlConsoleAddText(io,ffbuf,0) ;
    }
    ii = meStrlen(ffbuf) ;
    ffbuf[ii++] = '\r' ;
    ffbuf[ii++] = '\n' ;
    ffbuf[ii]   = '\0' ;
    if(meSocketWrite(io->ccsk,(char *)ffbuf,ii,0) <= 0)
    {
        sprintf((char *)ffbuf,"[Failed to send command - %d]",meSocketGetError()) ;
        if(io->urlBp != NULL)
            ffUrlConsoleAddText(io,ffbuf,0) ;
        mlwrite(MWABORT|MWPAUSE|MWSPEC,ffbuf) ;
        return ftpERROR ;
    }
    if(flags & 0x01)
        return ftpPOS_COMPLETE ;
    return ftpReadReply(io);
}

static void
ffCloseSockets(meIo *io, int logoff)
{
    timerClearExpired(SOCKET_TIMER_ID) ;
#if MEOPT_SSL
    if(ffUrlTypeIsSecure(io->type))
        meSslClose(&(io->sslp),1);
    else
#endif
    {
        if(!meSocketIsBad(io->sock))
        {
            meSocketShutdown(io->sock,SHUT_RDWR);
            meSocketClose(io->sock) ;
            io->sock = meBadSocket ;
        }
        if(logoff && !meSocketIsBad(io->ccsk))
        {
            ftpCommand(io,1,"QUIT") ;
            meSocketShutdown(io->ccsk,SHUT_RDWR);
            meSocketClose(io->ccsk) ;
            io->ccsk = meBadSocket ;
            if(meNullFree(io->sockUrl))
                io->sockUrl = NULL ;
            if(meNullFree(io->sockHome))
                io->sockHome = NULL ;
        }
    }
    ffremain = 0 ;
}

static int
ftpLogin(meIo *io, meUByte *user, meUByte *pass)
{
    int ii ;
    /* get the initial message */
    if(ftpReadReply(io) != ftpPOS_COMPLETE)
        return meFALSE ;

    ii = ftpCommand(io,0,"USER %s",user) ;
    if(ii == ftpPOS_INTERMED)
        ii = ftpCommand(io,0,"PASS %s", pass) ;
    if(ii != ftpPOS_COMPLETE)
    {
        if(io->passwdReg != NULL)
        {
            regDelete(io->passwdReg) ;
            io->passwdReg = NULL ;
        }
        return mlwrite(MWABORT,(meUByte *)"[Failed to login]") ;
    }
    return meTRUE ;
}

static int
ftpGetDataChannel(meIo *io)
{
    meUByte *aac, *ppc ;
    int aai[4], ppi[2] ;
    meUByte *ss ;

    if(ftpCommand(io,0,"PASV") != ftpPOS_COMPLETE)
        return meABORT ;
    if(((ss=meStrchr(resultStr,'(')) == NULL) ||
       (sscanf((char *)ss,"(%d,%d,%d,%d,%d,%d)",aai,aai+1,aai+2,aai+3,ppi,ppi+1) != 6))
    {
        ss = (meUByte *) "[Failed to extract PASSIVE address]" ;
        if(io->urlBp != NULL)
            ffUrlConsoleAddText(io,ss,0) ;
        return mlwrite(MWABORT|MWPAUSE|MWSPEC,ss) ;
    }
    aac = (meUByte *)&(io->sockAddr.sin_addr);
    ppc = (meUByte *)&(io->sockAddr.sin_port);
    aac[0] = (meUByte) aai[0] ;
    aac[1] = (meUByte) aai[1] ;
    aac[2] = (meUByte) aai[2] ;
    aac[3] = (meUByte) aai[3] ;
    ppc[0] = (meUByte) ppi[0] ;
    ppc[1] = (meUByte) ppi[1] ;

    if(meSocketIsBad(io->sock = meSocketOpen(AF_INET, SOCK_STREAM, IPPROTO_TCP)))
    {
        sprintf((char *)resultStr,"[Failed to open data channel - %d]",meSocketGetError()) ;
        if(io->urlBp != NULL)
            ffUrlConsoleAddText(io,resultStr,0) ;
        return mlwrite(MWABORT|MWPAUSE|MWSPEC,resultStr) ;
    }

    aai[0] = 1 ;
    meSocketSetOpt(io->sock,SOL_SOCKET,SO_REUSEADDR,(char *) aai,sizeof(int)) ;

    if(meSocketConnect(io->sock,(struct sockaddr *) &(io->sockAddr),sizeof(struct sockaddr_in)) < 0)
    {
        sprintf((char *)resultStr,"[Failed to connect data channel - %d]",meSocketGetError()) ;
        if(io->urlBp != NULL)
            ffUrlConsoleAddText(io,resultStr,0) ;
        return mlwrite(MWABORT|MWPAUSE|MWSPEC,resultStr) ;
    }
    return meTRUE ;
}

int
ffUrlGetInfo(meIo *io, meUByte **host, meUByte **port, meUByte **user, meUByte **pass)
{
    meRegNode *root, *reg ;
    meUByte *ss ;

    if (((root = regFind (NULL,(meUByte *)"/url")) == NULL) &&
        ((root = regSet (NULL,(meUByte *)"/url", NULL)) == NULL))
        return meFALSE ;
    ss = (meUByte *) ((ffUrlTypeIsFtp(io->type)) ? "ftp":"http") ;
    if (((reg = regFind (root, ss)) == NULL) &&
        ((reg = regSet (root, ss, NULL)) == NULL))
        return meFALSE ;
    if (((root = regFind (reg, *host)) == NULL) &&
        ((root = regSet (reg, *host, NULL)) == NULL))
        return meFALSE ;

    reg = regFind (root,(meUByte *)"user") ;
    if (*user != NULL)
    {
        if((*pass == NULL) && (reg != NULL) && !meStrcmp(regGetValue(reg),*user) &&
           ((reg = regFind (root,(meUByte *)"pass")) != NULL))
            *pass = regGetValue(reg) ;
    }
    else if(reg != NULL)
    {
        *user = regGetValue(reg) ;
        if((reg = regFind (root,(meUByte *)"pass")) != NULL)
            *pass = regGetValue(reg) ;
    }
    io->passwdReg = NULL ;
    if(*user != NULL)
    {
        if(*pass == NULL)
        {
            meUByte buff[meSBUF_SIZE_MAX] ;
            /* Always get the password from the user even when running a macro.
             * This is important for &stat which may trigger a password request */
            meStrcpy(buff,*user) ;
            meStrcat(buff,"@") ;
            meStrcat(buff,*host) ;
            meStrcat(buff," password") ;
            if((meGetStringFromUser(buff, MLNOHIST|MLHIDEVAL, 0, buff, meSBUF_SIZE_MAX) <= 0) ||
               ((io->passwdReg = regSet(root,(meUByte *)"pass",buff)) == NULL) ||
               ((regGetValue(io->passwdReg) == NULL) && ((io->passwdReg->value = meStrdup(buff)) == NULL)))
                return meFALSE ;
            io->passwdReg->mode |= meREGMODE_INTERNAL ;
            *pass = regGetValue(io->passwdReg) ;
        }
        else if(((reg = regFind(root,(meUByte *)"pass")) == NULL) || meStrcmp(regGetValue(reg),*pass))
            regSet(root,(meUByte *)"pass",*pass) ;
        if(((reg = regFind(root,(meUByte *)"user")) == NULL) || meStrcmp(regGetValue(reg),*user))
            regSet(root,(meUByte *)"user",*user) ;
    }
    if ((reg = regFind (root,(meUByte *)"host")) != NULL)
        *host = regGetValue(reg) ;
    reg = regFind (root,(meUByte *)"port") ;
    if (*port != NULL)
    {
        if((reg == NULL) || meStrcmp(regGetValue(reg),*port))
            regSet(root,(meUByte *)"port",*port) ;
    }
    else if(reg != NULL)
        *port = regGetValue(reg) ;
    return meTRUE ;
}

void
ffUrlCreateName(meUByte *buff, meUShort type, meUByte *host, meUByte *port, meUByte *user, meUByte *pass, meUByte *file)
{
    meUByte *ss=buff;
    if(type & meIOTYPE_FTP)
        *ss++ = 'f';
    else
    {
        *ss++ = 'h';
        *ss++ = 't';
    }
    *ss++ = 't';
    *ss++ = 'p';
    if(type & meIOTYPE_SSL)
        *ss++ = 's';
    *ss++ = ':';
    *ss++ = '/';
    *ss++ = '/';
    if(user != NULL)
    {
        meStrcpy(ss,user);
        ss += meStrlen(ss);
        if(pass != NULL)
        {
            *ss++ = ':';
            meStrcpy(ss,pass);
            ss += meStrlen(ss);
        }
        *ss++ = '@';
    }
    meStrcpy(ss,host);
    ss += meStrlen(ss);
    if(port != NULL)
    {
        *ss++ = ':';
        meStrcpy(ss,port);
        ss += meStrlen(ss);
    }
    if(file != NULL)
    {
        meStrcpy(ss,file);
        ss += meStrlen(ss);
        if((type & meIOTYPE_DIR) && (ss[-1] != DIR_CHAR))
        {
            *ss++ = DIR_CHAR;
            *ss = '\0';
        }
    }
}

static int ffUrlFileOpen(meIo *io, meUByte *urlName, meUByte *user, meUByte *pass, meUInt rwflag, meBuffer *bp);

int
ffFtpFileOpen(meIo *io, meUByte *host, meUByte *port, meUByte *user, meUByte *pass, meUByte *file, meUInt rwflag)
{
    meUByte buff[meBUF_SIZE_MAX], lbuff[meBUF_SIZE_MAX+32], *ss ;
    int dirlist, ll ;

    /* are we currently connected to this site?? */
    ffUrlCreateName(buff,io->type,host,port,user,NULL,NULL) ;
    if((io->sockUrl == NULL) || meStrcmp(buff,io->sockUrl))
    {
        /* if we're already logged on somewhere else kill the connection */
        ffCloseSockets(io,1) ;

        if(port == NULL)
            port = (meUByte *)"21" ;
        if(user == NULL)
        {
            /* user = "anonymous" ;*/
            user = (meUByte *)"ftp" ;
            pass = (meUByte *)"guest" ;
        }

        if(meSocketIsBad(io->ccsk=ffOpenConnectUrlSocket(io,host,port)))
            return meFALSE ;

        if(ftpLogin(io,user,pass) <= 0)
        {
            ffCloseSockets(io,1) ;
            return meFALSE ;
        }
        if(ftpCommand(io,0,"TYPE I") != ftpPOS_COMPLETE)
        {
            ffCloseSockets(io,1) ;
            return meFALSE ;
        }
        io->sockUrl = meStrdup(buff) ;
        if((ftpCommand(io,0,"PWD") == ftpPOS_COMPLETE) &&
           ((ss = meStrchr(resultStr,'"')) != NULL) &&
           ((io->sockHome = meStrchr(ss+1,'"')) != NULL))
        {
            if(io->sockHome[-1] == '/')
                io->sockHome[-1] = '\0' ;
            else
                io->sockHome[0] =  '\0' ;
            io->sockHome = meStrdup(ss+1) ;
        }
    }

    if((file[1] == '~') && (file[2] == '/'))
    {
        if(io->sockHome != NULL)
        {
            meStrcpy(buff,io->sockHome) ;
            if(((ll = meStrlen(buff)) > 0) && (buff[ll-1] == '/'))
                ll-- ;
        }
        else
            ll = 0 ;
        meStrcpy(buff+ll,file+2) ;
        file = buff ;
    }

    if(rwflag & meRWFLAG_WRITE)
        ss = (meUByte *) "writing" ;
    else if(rwflag & meRWFLAG_READ)
        ss = (meUByte *) "reading" ;
    else if(rwflag & meRWFLAG_DELETE)
        ss = (meUByte *) "deleting" ;
    else if(rwflag & meRWFLAG_MKDIR)
        ss = (meUByte *) "mkdir" ;
    else
        ss = (meUByte *) "stating" ;
    sprintf((char *)lbuff,"[Connected, %s %s]",ss,file);
    if(io->urlBp != NULL)
        ffUrlConsoleAddText(io,lbuff,0) ;
    mlwrite(MWCURSOR|MWSPEC,(meUByte *)lbuff);
    if(rwflag & meRWFLAG_BACKUP)
    {
        /* try to back-up the existing file - no error checking! */
        meUByte filename[meBUF_SIZE_MAX] ;

        if(createBackupName(filename,file,'~',1) ||
           ((ftpCommand(io,0,"RNFR %s",file) == ftpPOS_INTERMED) &&
            (ftpCommand(io,0,"RNTO %s",filename) != ftpPOS_COMPLETE) &&
            ((ftpCommand(io,0,"DELE %s",filename) != ftpPOS_COMPLETE) ||
             (ftpCommand(io,0,"RNFR %s",file) != ftpPOS_INTERMED) ||
             (ftpCommand(io,0,"RNTO %s",filename) != ftpPOS_COMPLETE))))
            mlwrite(MWABORT|MWPAUSE,(meUByte *)"[Unable to backup file %s]",file) ;
    }
    if((rwflag & meRWFLAG_DELETE) && !(rwflag & meRWFLAG_BACKUP))
    {
        /* if backing up as well the file is already effectively deleted */
        ll = meStrlen(file) ;
        if(file[ll-1] == '/')
        {
            if((ftpCommand(io,0,"CWD %s..",file) != ftpPOS_COMPLETE) ||
               (ftpCommand(io,0,"RMD %s",file) != ftpPOS_COMPLETE))
                return -5 ;
        }
        else if((ftpCommand(io,0,"DELE %s",file) != ftpPOS_COMPLETE) &&
                ((ftpCommand(io,0,"CWD %s/..",file) != ftpPOS_COMPLETE) ||
                 (ftpCommand(io,0,"RMD %s",file) != ftpPOS_COMPLETE)))
            return -5 ;
    }
    if(rwflag & meRWFLAG_MKDIR)
    {
        if(ftpCommand(io,0,"MKD %s",file) != ftpPOS_COMPLETE)
            return -4 ;
    }
    if(rwflag & meRWFLAG_STAT)
    {
        ss = evalResult ;
        if(ftpCommand(io,0,"CWD %s",file) == ftpPOS_COMPLETE)
        {
            /* this is a directory */
            *ss++ = 'D' ;
            *ss++ = '|' ;
        }
        else if(ftpCommand(io,0,"SIZE %s",file) == ftpPOS_COMPLETE)
        {
            /* this is a regular file */
            *ss++ = 'R' ;
            if((ll=meStrlen(resultStr) - 4) > 0)
            {
                meStrcpy(ss,resultStr+4) ;
                ss += ll ;
            }
            *ss++ = '|' ;
            if((ftpCommand(io,0,"MDTM %s",file) == ftpPOS_COMPLETE) &&
               ((ll=meStrlen(resultStr) - 4) > 0))
            {
                meStrcpy(ss,resultStr+4) ;
                if(ss[4] == '0')
                    ss[4] = ' ' ;
                if(ss[6] == '0')
                    ss[6] = ' ' ;
                if(ss[8] == '0')
                    ss[8] = ' ' ;
                if(ss[10] == '0')
                    ss[10] = ' ' ;
                if(ss[12] == '0')
                    ss[12] = ' ' ;
                ss += ll ;
            }
        }
        else
        {
            /* does not exist */
            *ss++ = 'X' ;
            *ss++ = '|' ;
        }
        /* get the mod time */
        *ss = '\0' ;
    }
    if(!(rwflag & (meRWFLAG_WRITE|meRWFLAG_READ)))
        return meTRUE ;

    ll = meStrlen(file) ;
    if(rwflag & meRWFLAG_FTPNLST)
        dirlist = -1 ;
    else if(file[ll-1] == DIR_CHAR)
        dirlist = 1 ;
    else
        dirlist = 0 ;

    /* send the read command */
    for(;;)
    {
        if(dirlist && (ftpCommand(io,0,"CWD %s",file) != ftpPOS_COMPLETE))
            break;

        if(ftpGetDataChannel(io) <= 0)
            break;

        if(rwflag & meRWFLAG_WRITE)
            ftpCommand(io,1,"STOR %s",file);
        else if(rwflag & meRWFLAG_FTPNLST)
            ftpCommand(io,1,(dirlist < 0) ? "NLST -a":"NLST");
        else if(dirlist)
            ftpCommand(io,1,"LIST");
        else
            ftpCommand(io,1,"RETR %s",file);

        /* find out if all's well */
        if(ftpReadReply(io) == ftpPOS_PRELIMIN)
        {
            if(dirlist)
                io->type |= meIOTYPE_DIR;
            return meTRUE ;
        }
        if((rwflag & (meRWFLAG_WRITE|meRWFLAG_NODIRLIST)) || (dirlist > 0))
            break;
        ffCloseSockets(io,0) ;
        dirlist = 1 ;
    }
    ffCloseSockets(io,0) ;
    if(!(rwflag & meRWFLAG_SILENT))
        mlwrite(MWCLEXEC|MWABORT,(meUByte *)"[Failed to %s file]",(rwflag & meRWFLAG_WRITE) ? "write":"read");
    return (rwflag & meRWFLAG_WRITE) ? -3:-2 ;
}

static int
ffHttpFileOpen(meIo *io, meUByte *host, meUByte *port, meUByte *user, meUByte *pass, meUByte *file, meUInt rwflag)
{
    meUByte pfb[meBUF_SIZE_MAX], buff[meBUF_SIZE_MAX+meBUF_SIZE_MAX], *thost, *tport, *ss, *s1, *pfn, cc ;
    meInt pfs;
    FILE *pfp=NULL;

    if(((ss = getUsrVar((meUByte *)"http-post-file")) != errorm) && (*ss != '\0'))
    {
        SetUsrLclCmdVar((meUByte *) "http-post-file",(meUByte *) "",&usrVarList) ;
        if((pfp=fopen((char *)ss,"rb")) == NULL)
            return meFALSE ;
        pfn = ss;
    }
    if ((ss = getUsrVar((meUByte *)"http-proxy-addr")) != errorm)
    {
        thost = ss ;
        tport = getUsrVar((meUByte *)"http-proxy-port") ;
    }
    else
    {
        thost = host ;
        tport = port ;
    }

    if(tport == NULL)
        tport = (meUByte *)"80" ;

    /* if we're already logged on somewhere else kill the connection */
    ffCloseSockets(io,0) ;
    if(meSocketIsBad(io->sock=ffOpenConnectUrlSocket(io,thost,tport)))
    {
        if(pfp != NULL)
            fclose(pfp);
        return meFALSE ;
    }
    sprintf((char *)buff,"[Connected, reading %s]",file);
    if(io->urlBp != NULL)
        ffUrlConsoleAddText(io,buff,0) ;
    mlwrite(MWCURSOR|MWSPEC,buff);
    
    ss = buff;
    if(pfp != NULL)
    {
        /* send POST message to http */
        meStrcpy(ss,"POST ") ;
        ss += 5;
    }
    else
    {
        /* send GET message to http */
        meStrcpy(ss,"GET ") ;
        ss += 4;
    }
    if(thost == host)
        meStrcpy(ss,file) ;
    else
        ffUrlCreateName(ss,io->type,host,port,NULL,NULL,file) ;
    ss += meStrlen(ss) ;
    meStrcpy(ss," HTTP/1.0\r\nConnection: Keep-Alive\r\nHost: ") ;
    ss += meStrlen(ss);
    meStrcpy(ss,host);
    ss += meStrlen(ss);
    if((port != NULL) && meStrcmp(port,"80"))
    {
        *ss++ = ':';
        meStrcpy(ss,port);
        ss += meStrlen(ss);
    }
    if(user != NULL)
    {
        /* password supplied, encode */
        meUByte tbuff[meBUF_SIZE_MAX] ;
        meStrcpy(ss,"\r\nAuthorization: Basic ") ;
        ss += 23 ;
        meStrcpy(tbuff,user) ;
        meStrcat(tbuff,":") ;
        meStrcat(tbuff,pass) ;
        ss = strBase64Encode(ss,tbuff) ;
    }
    if(pfp != NULL)
    {
        meInt ii, ll;
        meUByte *s2 ;
        fseek(pfp,0,SEEK_END);
        ii = ftell(pfp) ;
        fseek(pfp,0,SEEK_SET) ;
        if((s1 = meStrrchr(pfn,'/')) == NULL)
            s1 = pfn;
        if((s2 = meStrrchr(s1,'\\')) == NULL)
            s2 = s1;
        pfs = sprintf((char *) pfb,"\r\n----5Iz6dTINmxNFw6S42Ryf98IBXX1NCe%x",(meUInt) clock());
        ll = sprintf((char *) pfb+pfs,"\r\nContent-Disposition: form-data; name=\"file\"; filename=\"%s\"\r\nContent-Type: application/octet-stream\r\n\r\n",s2);
        ss += sprintf((char *) ss,"\r\nContent-Length: %d",pfs-2+ll+ii+pfs+4);
    }
    if((s1 = getUsrVar((meUByte *)"http-cookies")) != errorm)
    {
        meStrcpy(ss,"\r\nCookie: ") ;
        ss += 10 ;
        meStrcpy(ss,s1) ;
        ss += meStrlen(ss) ;
    }
    meStrcpy(ss,"\r\n\r\n") ;
    if(io->urlFlags & meIOURLF_SHOW_DETAILS)
    {
        meUByte *s2 ;
        ss = s2 = buff ;
        while((cc=*s2++) != '\0')
        {
            if(cc == '\r')
            {
                s2[-1] = '\0' ;
                ffUrlConsoleAddText(io,ss,0) ;
                s2[-1] = '\r' ;
                ss = ++s2 ;
            }
        }
        resetBufferWindows(io->urlBp) ;
        if(io->urlFlags & meIOURLF_SHOW_CONSOLE)
            screenUpdate(meTRUE,2-sgarbf) ;
    }
    if(meSocketWrite(io->sock,(char *)buff,meStrlen(buff),0) <= 0)
    {
        ffCloseSockets(io,0) ;
        if(pfp != NULL)
            fclose(pfp);
        return meFALSE ;
    }
    if(pfp != NULL)
    {
        meInt ii;
        if(meSocketWrite(io->sock,(char *)pfb+2,meStrlen(pfb+2),0) <= 0)
        {
            ffCloseSockets(io,0) ;
            fclose(pfp);
            return meFALSE ;
        }
        while(((ii=fread(ffbuf,1,meFIOBUFSIZ,pfp)) > 0) &&
              (meSocketWrite(io->sock,(char *)ffbuf,ii,0) > 0))
            ;
        fclose(pfp);
        if(ii != 0)
            return meFALSE ;
        meStrcpy(pfb+pfs,"--\r\n");
        if(meSocketWrite(io->sock,(char *)pfb,pfs+4,0) <= 0)
        {
            ffCloseSockets(io,0) ;
            return meFALSE ;
        }
    }

    /* must now ditch the header, read up to the first blank line */
    while(ffReadSocketLine(io,io->sock,buff) > 0)
    {
        if(buff[0] == '\0')
        {
            ffread = ffremain ;
            return meTRUE ;
        }
        if(!meStrncmp(buff,"Content-Length:",15))
            io->length = meAtoi(buff+15) ;
        else if(((buff[0] == 'L') || (buff[0] == 'l')) && !meStrncmp(buff+1,"ocation:",8))
        {
            /* The requested file is not here, its at the given location */
            ss = buff+9 ;
            while(((cc=*ss) == ' ') || (cc == '\t'))
                ss++ ;
            if(*ss != '\0')
            {
                /* printf("Got Location: [%s]\n",ss) ;*/
                ffCloseSockets(io,0);
                io->redirect++;
                if(io->redirect > 5)
                {
                    if(rwflag & meRWFLAG_SILENT)
                        return meABORT;
                    return mlwrite(MWABORT|MWPAUSE,(meUByte *)"[To many redirections - see console]");
                }
                /* if this starts with http:// https:// etc. then start again */
                cc = ffUrlGetType(ss);
                if(ffUrlTypeIsHttpFtp(cc))
                {
                    io->type = cc;
                    return ffUrlFileOpen(io,ss,user,pass,rwflag,NULL);
                }
                if((ss[0] != '/') && ((s1 = meStrrchr(file,'/')) != NULL))
                {
                    pfs = s1 + 1 - file;
                    memcpy(pfb,file,pfs);
                    meStrcpy(pfb+pfs,ss);
                    ss = pfb;
                }
                if(!meStrcmp(ss,file))
                {
                    if(rwflag & meRWFLAG_SILENT)
                        return meABORT;
                    return mlwrite(MWABORT|MWPAUSE,(meUByte *)"[Redirection loop to %s]",ss);
                }
                return ffHttpFileOpen(io,host,port,user,pass,ss,rwflag);
            }
        }
    }
    ffCloseSockets(io,0);
    if(rwflag & meRWFLAG_SILENT)
        return meABORT;
    return mlwrite(MWABORT|MWPAUSE,(meUByte *)"[Failed to read header of %s]",ss);
}

#if MEOPT_SSL
static void
ffSUrlLogger(meUByte *txt, void *iop)
{
    meIo *io = (meIo *) iop;
    if(io->urlBp != NULL)
        ffUrlConsoleAddText(io,txt,0);
}

static int
ffSUrlFileOpen(meIo *io, meUByte *host, meUByte *port, meUByte *user, meUByte *pass, meUByte *file, meUInt rwflag)
{
    meUByte buff[meSSL_BUFF_SIZE], *s1, *s2;
    meInt ii;
    
    if(ffUrlTypeIsFtp(io->type))
        return mlwrite(MWABORT|MWPAUSE,(meUByte *)"[Error - FTPS not yet supported]");
    if(meSslSetup((io->urlFlags & (meIOURLF_WARNING|meIOURLF_SHOW_DETAILS|meIOURLF_VERBOSE|meIOURLF_IGN_CRT_ERR)),(io->urlBp != NULL) ? ffSUrlLogger:NULL,io,buff) < 0)
    {
        mlwrite(MWABORT|MWPAUSE,(meUByte *) "[%s]",buff);
        return meFALSE;
    }

    if(((s1 = getUsrVar((meUByte *)"http-proxy-addr")) != errorm) && (*s1 != '\0'))
    {
        ii = 0;
        if((s2 = getUsrVar((meUByte *)"http-proxy-port")) != errorm)
            ii = meAtoi(s2);
        meSslSetProxy(s1,ii);
    }
    else
        meSslSetProxy(NULL,0);
    if(((s1 = getUsrVar((meUByte *)"http-cookies")) == errorm) || (*s1 == '\0'))
        s1 = NULL;
    if(((s2 = getUsrVar((meUByte *)"http-post-file")) == errorm) || (*s2 == '\0'))
        s2 = NULL;
    else
        SetUsrLclCmdVar((meUByte *)"http-post-file",(meUByte *) "",&usrVarList) ;

    sprintf((char *)buff,"[Connecting to %s:%s]",host,((port == NULL) ? "443":(char *)port));
    if(io->urlBp != NULL)
        ffUrlConsoleAddText(io,buff,0) ;
    mlwrite(MWCURSOR|MWSPEC,buff);
    
    if((ii = meSslOpen(&(io->sslp),host,((port == NULL) || (port[0] == '\0')) ? 0:atoi((char *)port),user,pass,file,s1,s2,rwflag,buff)) < 0)
    {
        sprintf((char *)buff,"[Failed to connect to %s:%s - %d]",host,((port == NULL) ? "443":(char *)port),ii) ;
        if(io->urlBp != NULL)
            ffUrlConsoleAddText(io,buff,0) ;
        mlwrite(MWABORT|MWPAUSE|MWSPEC,buff);
        return meFALSE;
    }
    if(ii == 0)
    {
        meUByte cc;
        /* printf("Got Location: [%s]\n",ss) ;*/
        /* if this starts with http:// https:// etc. then start again */
        meSslClose(&(io->sslp),0);
        io->redirect++;
        if(io->redirect > 5)
        {
            if(rwflag & meRWFLAG_SILENT)
                return meABORT;
            return mlwrite(MWABORT|MWPAUSE,(meUByte *)"[To many redirections - see console]");
        }
        cc = ffUrlGetType(buff);
        if(ffUrlTypeIsHttpFtp(cc))
        {
            io->type = cc;
            return ffUrlFileOpen(io,buff,user,pass,rwflag,NULL);
        }
        if((buff[0] != '/') && ((s1 = meStrrchr(file,'/')) != NULL))
        {
            meStrcpy(ffbuf,buff);
            ii = s1 + 1 - file;
            memcpy(buff,file,ii);
            meStrcpy(buff+ii,ffbuf);
        }
        if(!meStrcmp(buff,file))
        {
            if(rwflag & meRWFLAG_SILENT)
                return meABORT;
            return mlwrite(MWABORT|MWPAUSE,(meUByte *)"[Redirection loop to %s]",buff);
        }
        return ffSUrlFileOpen(io,host,port,user,pass,buff,rwflag);
    }
    io->length = io->sslp.length;
    return meTRUE ;
}
#else
static int
ffSUrlFileOpen(meIo *io, meUByte *host, meUByte *port, meUByte *user, meUByte *pass, meUByte *file, meUInt rwflag)
{
    return mlwrite(MWABORT|MWPAUSE,(meUByte *)"[Error - HTTPS & FTPS not yet supported]");
}
#endif

static void
ffUrlFileSetupFlags(meIo *io, meUInt rwflag)
{
    meInt ti=(ffUrlTypeIsFtp(io->type)) ? 1:0;
    meUByte *ss;
    
    /* setup the flags and console buffer */
    io->urlBp = NULL;
    io->urlFlags = 0;
    if((ss = getUsrVar(ffUrlFlagsVName[ti])) == errorm)
        ss = (meUByte *) "c";
    if(meStrchr(ss,'i') != NULL)
        io->urlFlags |= meIOURLF_IGN_CRT_ERR;
    if((meStrchr(ss,'c') != NULL) &&
       ((io->urlBp=bfind(ffUrlConsoleBName[ti],BFND_CREAT)) != NULL))
    {
        meModeClear(io->urlBp->mode,MDUNDO) ;
        /* must not show the console if inserting a file as the destination buffer will be displayed and unstable */
        if(meStrchr(ss,'d') != NULL)
            io->urlFlags |= meIOURLF_SHOW_DETAILS;
        if(meStrchr(ss,'p') != NULL)
            io->urlFlags |= meIOURLF_SHOW_PROGRESS;
        if((meStrchr(ss,'s') != NULL) && !(rwflag & meRWFLAG_INSERT))
            io->urlFlags |= meIOURLF_SHOW_CONSOLE;
        if(meStrchr(ss,'v') != NULL)
            io->urlFlags |= meIOURLF_VERBOSE;
        if(meStrchr(ss,'w') != NULL)
            io->urlFlags |= meIOURLF_WARNING;
        if(io->urlBp->lineCount)
            ffUrlConsoleAddText(io,(meUByte *)"",0x04) ;
        if((io->urlFlags & meIOURLF_SHOW_CONSOLE) && !(rwflag & meRWFLAG_SILENT))
            meWindowPopup(ffUrlConsoleBName[ti],0,NULL) ;
    }
}

static int
ffUrlFileOpen(meIo *io, meUByte *urlName, meUByte *user, meUByte *pass, meUInt rwflag, meBuffer *bp)
{
#ifdef _WIN32
    static int init=0 ;
#endif
    meUByte buff[meBUF_SIZE_MAX+32], sockAddr[meBUF_SIZE_MAX], *host, *port, *ss, *dd, *ee, *uu, cc ;
    int ii ;
    
    io->fp = meBadFile;
    io->length = -1;
    ffUrlFileSetupFlags(io,rwflag) ;
    
#ifdef _WIN32
    if(!init)
    {
        WSADATA wsaData;
        
        if(WSAStartup(MAKEWORD(1,1),&wsaData))
            return mlwrite(MWABORT|MWPAUSE,(meUByte *)"[Failed to initialise sockets]") ;
        atexit((meATEXIT) WSACleanup) ;
        init = 1 ;
    }
#endif

    ss = urlName + ((ffUrlTypeIsFtp(io->type)) ? 6:7);
    if(ffUrlTypeIsSecure(io->type))
        ss++;
    dd = buff ;
    ee = sockAddr ;
    uu = port = NULL ;
    host = dd ;
    while(((cc=*ss++) != '\0') && (cc != DIR_CHAR))
    {
        *dd++ = cc ;
        *ee++ = cc ;
        if(cc == ':')
            port = dd ;
        else if(cc == '@')
        {
            uu = buff ;
            host = dd ;
            port = NULL ;
        }
    }
    *dd = '\0' ;
    *ee = '\0' ;
    ss-- ;

    if(port != NULL)
        port[-1] = '\0' ;
    if(uu != NULL)
    {
        host[-1] = '\0' ;
        user = uu ;
        if((pass = meStrchr(uu,':')) != NULL)
            *pass++ = '\0' ;
    }

    if(ffUrlGetInfo(io,&host,&port,&user,&pass) <= 0)
        return meFALSE ;

    if(ss[0] == '\0')
        ss = (meUByte *) "/" ;
    /* is it a http: or ftp: */
    if(ffUrlTypeIsSecure(io->type))
        ii = ffSUrlFileOpen(io,host,port,user,pass,ss,rwflag) ;
    else if(ffUrlTypeIsFtp(io->type))
        ii = ffFtpFileOpen(io,host,port,user,pass,ss,rwflag) ;
    else
        ii = ffHttpFileOpen(io,host,port,user,pass,ss,rwflag) ;

    if(ii > 0)
    {
        if((rwflag & meRWFLAG_READ) && (bp != NULL))
        {
            meUByte *ff, buff[meBUF_SIZE_MAX+128] ;
            if(ffUrlTypeIsHttp(io->type))
            {
                if(!(rwflag & meRWFLAG_INSERT))
                {
                    ffUrlCreateName(buff,io->type,host,port,user,NULL,ss) ;
                    if(fnamecmp(buff,bp->fileName) && ((ff=meStrdup(buff)) != NULL))
                        /* the original fname is freed in ffReadFile */
                        bp->fileName = ff ;
                }
            }
            else if(io->type & meIOTYPE_DIR)
            {
                /* for an ftp dir 2 things must be done:
                 * 1) If the file name does not end in a '/' then add one and correct the buffer name
                 * 2) Add the top Directory Listing line
                 */
                int ll ;
                meStrcpy(buff,"Directory listing of: ") ;
                if(rwflag & meRWFLAG_INSERT)
                    meStrcpy(buff+22,urlName) ;
                else
                {
                    meStrcpy(buff+22,bp->fileName) ;
                    ll = meStrlen(buff) ;
                    if(buff[ll-1] != DIR_CHAR)
                    {
                        buff[ll] = DIR_CHAR ;
                        buff[ll+1] = '\0' ;
                        /* the original fname is free'd in ffReadFile */
                        bp->fileName = NULL ;
                        resetBufferNames(bp,buff+22) ;
                    }
                }
                addLineToEob(bp,buff) ;
            }
        }
        if((rwflag & (meRWFLAG_READ|meRWFLAG_WRITE)) && (io->urlBp != NULL))
        {
            struct meTimeval tp ;
            if(io->urlFlags & meIOURLF_SHOW_PROGRESS)
            {
                ffUrlConsoleAddText(io,(meUByte *)"[Progress: ",0x02);
                io->urlFlags |= meIOURLF_CLOSE_PROGRESS;
            }
            gettimeofday(&tp,NULL);
            io->startTime = (tp.tv_sec * 1000) + (tp.tv_usec/1000);
        }
    }
    else
        io->urlBp = NULL ;
    return ii ;
}

static int
ffUrlFileClose(meIo *io, meUInt rwflag)
{
    if(io->urlFlags & meIOURLF_CLOSE_PROGRESS)
    {
        ffUrlConsoleAddText(io,(meUByte *)"]",0x03) ;
        io->urlFlags &= ~meIOURLF_CLOSE_PROGRESS ;
    }
    if(rwflag & (meRWFLAG_READ|meRWFLAG_WRITE))
    {
        if(ffUrlTypeIsFtp(io->type))
        {
            if(!meSocketIsBad(io->sock))
            {
                meSocketShutdown(io->sock,SHUT_RDWR);
                meSocketClose(io->sock) ;
                io->sock = meBadSocket ;
            }
            /* get the transfer complete */
            if(ftpReadReply(io) != ftpPOS_COMPLETE)
                mlwrite(MWABORT|MWPAUSE,(meUByte *)"[Problem occurred during download]") ;
        }
        if(io->urlBp != NULL)
        {
            meUByte buff[meBUF_SIZE_MAX] ;
            struct meTimeval tp ;
            int dwldtime, kbsec, kbhsec ;
            gettimeofday(&tp,NULL) ;
            dwldtime = ((tp.tv_sec * 1000) + (tp.tv_usec/1000)) - io->startTime ;
            if(dwldtime <= 0)
                dwldtime = 1 ;
            if(ffread > 0x1000000)
            {
                kbsec = (ffread / 128) * 125 ;
                if(dwldtime > 100)
                    kbhsec = (kbsec/(dwldtime/100)) % 100 ;
                else
                    kbhsec = 0 ;
            }
            else
            {
                kbsec = (ffread * 125) / 128 ;
                kbhsec = ((kbsec*100)/dwldtime) % 100 ;
            }
            kbsec /= dwldtime ;
            sprintf((char *)buff,"[Complete, %d bytes %s in %d.%02d seconds (%d.%02d Kbytes/s)]",
                    (int) ffread,(rwflag & meRWFLAG_WRITE) ? "sent":"received",
                    dwldtime/1000,(dwldtime/10) % 100,kbsec,kbhsec) ;
            ffUrlConsoleAddText(io,buff,0) ;
            io->urlBp = NULL ;
        }
    }
    if(io->sockUrl == NULL)
        ffCloseSockets(io,1) ;
    else
        timerSet(SOCKET_TIMER_ID,-1,meSOCKET_TIMEOUT) ;
    return meTRUE ;
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
        meUByte cc ;
        int ii, ll ;
        while((cc=*ss) != '\0')
        {
            for(ii=0 ; ii<subCount ; ii++)
            {
                ll = meStrlen(subFrom[ii]) ;
                if(!meStrncmp(ss,subFrom[ii],ll))
                {
                    meStrcpy(dd,subTo[ii]) ;
                    dd += meStrlen(dd) ;
                    ss += ll ;
                    break ;
                }
            }
            if((ii == subCount) || (ll == 0))
            {
                *dd++ = cc ;
                ss++ ;
            }
        }
        *dd = '\0' ;
    }
    else
    {
        meStrcpy(dd,ss) ;
        dd += meStrlen(dd) ;
    }
    return dd ;
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
	    backupPathFlag = -1 ;
	if(((s=meGetenv("MEBACKUPSUB")) != NULL) && (meStrlen(s) > 0))
        {
            meUByte cc, dd ;
            int ii ;

            while((dd=*s++) != '\0')
            {
                if(dd == 's')
                {
                    ii=backupSubCount++ ;
                    if(((dd = *s++) == '\0') ||
                       ((backupSubFrom=meRealloc(backupSubFrom,backupSubCount*sizeof(meUByte *))) == NULL) ||
                       ((backupSubTo  =meRealloc(backupSubTo  ,backupSubCount*sizeof(meUByte *))) == NULL))
                    {
                        backupSubCount = 0 ;
                        break ;
                    }
                    backupSubFrom[ii] = s ;
                    while(((cc=*s) != '\0') && (cc != dd))
                        s++ ;
                    if(cc != '\0')
                    {
                        *s++ = '\0' ;
                        backupSubTo[ii] = s ;
                        while(((cc=*s) != '\0') && (cc != dd))
                            s++ ;
                        *s++ = '\0' ;
                    }
                    if(cc == '\0')
                    {
                        backupSubCount = 0 ;
                        break ;
                    }
                }
            }
        }
    }
#endif
    ft = ffUrlGetType(fn);
    if(ffUrlTypeIsPipe(ft))
        return 1 ;
#if MEOPT_SOCKET
    if((backl == '#') && ffUrlTypeIsHttpFtp(ft))
    {
        meUByte tmp[meBUF_SIZE_MAX] ;
        s = getFileBaseName(fn) ;
        meStrcpy(tmp,s) ;
        s = tmp ;
        while((s=meStrchr(s,DIR_CHAR)) != NULL)  /* got a '/', -> '_' */
            *s++ = '_' ;
        mkTempName(filename,tmp,NULL) ;
    }
    else
#endif
#if MEOPT_EXTENDED_BACKUP
    if((backupPathFlag > 0) && !ffUrlTypeIsHttpFtp(ft))
    {
        if(backupPathFlag == 1)
        {
            meStrcpy(filename,backupPath) ;
            t = filename + meStrlen(backupPath) ;
            s = fn ;
            if(s[0] == DIR_CHAR)
                s++ ;
            createBackupNameStrcpySub(t,s,backupSubCount,backupSubFrom,backupSubTo) ;
        }
        else
        {
            int ll ;
            s = getFileBaseName(fn) ;
            ll = ((size_t)s - (size_t)fn) ;
            meStrncpy(filename,fn,ll) ;
            t = filename+ll ;
            t = createBackupNameStrcpySub(t,backupPath,backupSubCount,
                                          backupSubFrom,backupSubTo) ;
            createBackupNameStrcpySub(t,s,backupSubCount,backupSubFrom,backupSubTo) ;
        }
#ifdef _DRV_CHAR
        /* ensure the path has no ':' in it - breaks every thing, change to a '_' */
        while((t=meStrchr(t,_DRV_CHAR)) != NULL)
            *t++ = '_' ;
#endif
        if(flag & meBACKUP_CREATE_PATH)
        {
            meIo tio;
            int ii=0 ;
            while(((t=meStrrchr(filename,DIR_CHAR)) != NULL) && (t != filename))
            {
                *t = '\0' ;
                if(!meTestExist(filename))
                {
                    if(meTestDir(filename))
                        return 1 ;
                    *t = DIR_CHAR ;
                    break ;
                }
                ii++ ;
            }
            while(--ii >= 0)
            {
                if(ffWriteFileOpen(&tio,filename,meRWFLAG_MKDIR|meRWFLAG_SILENT,NULL) <= 0)
                    return 1 ;
                filename[meStrlen(filename)] = DIR_CHAR ;
            }
        }
    }
    else
#endif
        meStrcpy(filename,fn) ;

#ifdef _UNIX
    if((backl == '~') && ((meSystemCfg & (meSYSTEM_DOSFNAMES|meSYSTEM_HIDEBCKUP)) == meSYSTEM_HIDEBCKUP))
    {
        if((t=meStrrchr(filename,DIR_CHAR)) == NULL)
            t = filename ;
        else
            t++ ;
        s = filename + meStrlen(filename) + 1 ;
        do
            s[0] = s[-1] ;
        while(--s != t) ;
        s[0] = '.' ;
    }
#endif
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
ffgetBuf(meIo *io)
{
#ifdef MEOPT_TFS
    if(ffUrlTypeIsTfs(io->type))
    {
        if((ffremain = tfs_fread(ffbuf,1,meFIOBUFSIZ,io->tfsp)) <= 0)
        {
            /* if(ferror(io->fp))*/
            /* return mlwrite(MWABORT,(meUByte *)"[File read error %d]",errno); */
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
        else if(ffremain > meFIOBUFSIZ)
            ffremain = meFIOBUFSIZ;
#if MEOPT_SSL
        if(ffUrlTypeIsSecure(io->type))
            ffremain = meSslRead(&(io->sslp),ffremain,ffbuf);
        else
#endif
            ffremain = meSocketRead(io->sock,(char *) ffbuf,ffremain,0);
        if(ffremain <= 0)
        {
            if(io->length != -1)
                return mlwrite(MWABORT,(meUByte *)"[File socket read error]");
            io->length = -2;
            return meFALSE;
        }
        if(io->urlFlags & meIOURLF_CLOSE_PROGRESS)
            ffUrlConsoleAddText(io,(meUByte *)"#",(meLineGetLength(io->urlBp->dotLine) >= frameCur->width - 2) ? 0x02:0x03) ;
    }
    else
#endif
#ifdef _WIN32
    {
        if(ReadFile(io->fp,ffbuf,meFIOBUFSIZ,(DWORD *)&ffremain,NULL) == 0)
        {
            ffremain = 0;
            if((io->fp != GetStdHandle(STD_INPUT_HANDLE)) || (GetLastError() != ERROR_BROKEN_PIPE))
                return mlwrite(MWABORT,(meUByte *)"[File read error %d]",GetLastError());
        }
        if(ffremain <= 0)
            return meFALSE;
    }
#else
    {
        if((ffremain = fread(ffbuf,1,meFIOBUFSIZ,io->fp)) <= 0)
        {
            if(ferror(io->fp))
                return mlwrite(MWABORT,(meUByte *)"[File read error %d]",errno);
            return meFALSE;
        }
    }
#endif
#if MEOPT_CRYPT
    if(io->flags & meIOFLAG_CRYPT)
        meCrypt(ffbuf,ffremain);
#endif
    ffbuf[ffremain]='\n';
    ffread += ffremain;
    return meTRUE ;
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
            if(ffgetBuf(io) < 0)
                return meABORT ;
            ffcur = ffbuf ;
            if(ffremain <= 0)
            {
                if(ecc)
                {
                    ffremain = 0 ;
                    *ffcur = '\0' ;
                }
                else
                    ffremain = -1 ;
                break ;
            }
        }
        if(io->flags & (meIOFLAG_BINMOD|meIOFLAG_RBNMOD))
        {
            int bpl = (io->flags & meIOFLAG_BINMOD) ? meBINARY_BPL : meRBIN_BPL;
            if(len == 0)
            {
                ii = (io->flags & meIOFLAG_BINMOD) ? (meBINARY_BPL*4)+14 : 2*meRBIN_BPL;
                if((lp = meLineMalloc(ii,0)) == NULL)
                    return meABORT ;
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

                ffremain -= ((size_t) ffcur) - ((size_t) endp) ;

                if(newl)
                {
                    if(len == 0)
                    {
                        len = newl ;
                        if((lp = (meLine *) meLineMalloc(len,0)) == NULL)
                            return meABORT ;
                        text = lp->text ;
                    }
                    else
                    {
                        size_t ss ;
                        ii = len ;
                        len += newl ;
                        ss = meLineMallocSize(len) ;
                        if((lp = (meLine *) meRealloc(lp,ss)) == NULL)
                            return meABORT ;
                        text = lp->text + ii;
                        lp->unused = (meUByte) (ss - len - meLINE_SIZE);
                        lp->length = len;
                    }
                    if(ecc == '\r')
                        ffcur[-1] = '\n' ;
                    while((cc = *endp++) != '\n')
                        if(cc != '\0')
                            *text++ = cc ;
                    *text = '\0' ;
                }
                else if((len == 0) && ((lp = (meLine *) meLineMalloc(0,0)) == NULL))
                    return meABORT ;
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
                    ffremain = -1 ;
            }
            else
                ecc = 0 ;
        }
    } while(ffremain < 0) ;

    if(io->flags & (meIOFLAG_BINMOD|meIOFLAG_RBNMOD))
    {
        if(len == 0)
            return meFALSE ;

        if(io->flags & meIOFLAG_BINMOD)
        {
            meUInt cpos ;
            meUByte cc ;

            text = lp->text ;
            newl = meBINARY_BPL ;
            while(--newl >= 0)
            {
                if(newl < len)
                {
                    cc = text[newl] ;
                    text[(newl*3)+10] = hexdigits[cc >> 4] ;
                    text[(newl*3)+11] = hexdigits[cc & 0x0f] ;
                    text[(newl*3)+12] = ' ' ;
                    if(!isDisplayable(cc))
                        cc = '.' ;
                }
                else
                {
                    text[(newl*3)+10] = ' ' ;
                    text[(newl*3)+11] = ' ' ;
                    text[(newl*3)+12] = ' ' ;
                    cc = ' ' ;
                }
                text[newl+(meBINARY_BPL*3)+14] = cc ;
            }
            cpos = io->offset + (meUInt) ((ffread - ffremain - 1) & ~((meInt) (meBINARY_BPL-1))) ;
            newl=8 ;
            while(--newl >= 0)
            {
                text[newl] = hexdigits[cpos&0x0f] ;
                cpos >>= 4 ;
            }
            text[8] = ':' ;
            text[9] = ' ' ;

            text[(meBINARY_BPL*3)+10] = ' ' ;
            text[(meBINARY_BPL*3)+11] = '|' ;
            text[(meBINARY_BPL*3)+12] = ' ' ;
            text[(meBINARY_BPL*3)+13] = ' ' ;
            text[(meBINARY_BPL*4)+14] = '\0' ;
        }
        else
        {
            meUByte cc ;

            text = lp->text ;
            lp->length = len*2 ;
            text[len*2] = '\0' ;
            while(--len >= 0)
            {
                cc = text[len] ;
                text[(len*2)]   = hexdigits[cc >> 4] ;
                text[(len*2)+1] = hexdigits[cc & 0x0f] ;
            }
        }
        *line = lp ;
        return meTRUE ;
    }
    if(ffremain < 0)
    {
        /* had to break */
        if(len == 0)
            return meFALSE ;
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
        meAssert((ecc == '\n') || (ecc == '\r')) ;
        if(ecc == '\r')
        {
            if(*ffcur == '\n')
            {
                if(io->flags & meIOFLAG_NOTSET)
                    io->flags = (io->flags & ~meIOFLAG_NOTSET) | (meIOFLAG_CR|meIOFLAG_LF);
                else if((io->flags & (meIOFLAG_CR|meIOFLAG_LF)) != (meIOFLAG_CR|meIOFLAG_LF))
                    io->flags |= meIOFLAG_LTDIFF;
                ffremain-- ;
                ffcur++ ;
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
    *line = lp ;

    return meTRUE ;
}

static int
ffReadFileOpen(meIo *io, meUByte *fname, meUInt flags, meBuffer *bp)
{
    io->flags = meIOFLAG_NOTSET;
    ffremain = 0;
    ffread = 0;
    if(bp != NULL)
    {
        if(meModeTest(bp->mode,MDBINARY))
            io->flags = meIOFLAG_BINMOD;
        else if(meModeTest(bp->mode,MDRBIN))
            io->flags = meIOFLAG_RBNMOD;
#if MEOPT_CRYPT
        if(meModeTest(bp->mode,MDCRYPT))
            io->flags = meIOFLAG_CRYPT;
#endif
    }
    else
    {
#if MEOPT_CRYPT
        if(flags & meRWFLAG_CRYPT)
            io->flags = meIOFLAG_CRYPT;
#endif
    }

    /* If meIOTYPE_PIPE the fp handle must be already set, if file type should be NONE as any file: prefix should have been removed by now */
    if(ffUrlTypeIsHttpFtp(io->type))
    {
#if MEOPT_SOCKET
        int rr ;
#ifdef _UNIX
        io->redirect = 0;
        /* must stop any signals or the connection will fail */
        meSigHold() ;
#endif
        if((rr=ffUrlFileOpen(io,fname,NULL,NULL,flags,bp)) <= 0)
        {
#ifdef _UNIX
            meSigRelease() ;
#endif
            return rr ;
        }
        return meTRUE ;
#else
        return mlwrite(MWABORT|MWPAUSE,(meUByte *) "[No url support in this version]") ;
#endif
    }
    else if (ffUrlTypeIsTfs(io->type))
    {
#ifdef MEOPT_TFS
        io->tfsp = tfs_fopen(tfsdev,(char *)(fname+5));
        if(io->tfsp == NULL)
        {
            if(!(flags & meRWFLAG_SILENT))
                /* File not found.      */
                mlwrite(MWCLEXEC,(meUByte *)"[%s: No such TFS file]",fname);
            return -2 ;
        }
#else
        return mlwrite(MWABORT|MWPAUSE,(meUByte *) "[No tfs support in this version]") ;
#endif
    }
    else if(!ffUrlTypeIsPipe(io->type))
    {
#ifdef _WIN32
        /* On windows a file could be temporarily lock be another process (e.g. virus scanner)
         * so if the failure is ERROR_SHARING_VIOLATION wait and try again */
        int retries=10 ;
        for(;;)
        {
            if(((io->fp=CreateFile((const char *) fname,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,
                                   FILE_ATTRIBUTE_NORMAL,NULL)) != INVALID_HANDLE_VALUE) ||
               (GetLastError() != ERROR_SHARING_VIOLATION) ||
               (--retries == 0))
                break ;
            Sleep(100) ;
        }
        if(io->fp == INVALID_HANDLE_VALUE)
#else
        if ((io->fp=fopen((char *)fname, "rb")) == NULL)
#endif
        {
            if(!(flags & meRWFLAG_SILENT))
                /* File not found.      */
                mlwrite(MWCLEXEC,(meUByte *)"[%s: No such file]", fname);
            return -2 ;
        }
    }
#ifdef _UNIX
    meSigHold() ;
#endif
    return meTRUE ;
}

/* close the read file handle, Ignore any close error */
static void
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
        ffUrlFileClose(io,meRWFLAG_READ) ;
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
    meSigRelease() ;
#endif
    io->type = meIOTYPE_NONE;
}

int
ffReadFile(meIo *io, meUByte *fname, meUInt flags, meBuffer *bp, meLine *hlp,
           meUInt uoffset, meUInt loffset, meInt length)
{
    meLine *lp0, *lp1, *lp2 ;
    int   ss, nline ;
#if MEOPT_SOCKET
    int ff = ((bp != NULL) && (fname != NULL) && (bp->fileName == fname)) ;
#endif
    io->type = ffUrlGetType(fname);
    if(ffReadFileOpen(io,fname,flags,bp) <= 0)
        return meABORT ;

    io->offset = 0 ;
    if(length != 0)
    {
        /* partial reading only supported by the insert-file command on regular files */
        if(length < 0)
        {
            meUInt fsu, fsl ;
#ifdef _WIN32
            fsl = GetFileSize(io->fp,(DWORD *) &fsu) ;
#else
#ifdef _LARGEFILE_SOURCE
            off_t fs ;
            fseeko(io->fp,0,SEEK_END) ;
            fs = ftello(io->fp) ;
            fsu = (meUInt) (fs >> 32) ;
            fsl = (meUInt) fs ;
#else
            fseek(io->fp,0,SEEK_END) ;
            fsl = ftell(io->fp) ;
            fsu = 0 ;
#endif
#endif
            /* ignore the upper offset if moving from the end of the file -
             * can't be bothered to do the maths */
            uoffset = fsu ;
            if(loffset <= fsl)
                loffset = fsl - loffset ;
            else if(fsu == 0)
                loffset = 0 ;
            else
            {
                uoffset-- ;
                loffset = 0xffffffff - (loffset - fsl - 1) ;
            }
            length = -length ;
        }
        io->offset = loffset ;
        if(io->flags & (meIOFLAG_BINMOD|meIOFLAG_RBNMOD))
        {
            loffset &= (io->flags & meIOFLAG_BINMOD) ? meBINARY_BPL-1:meRBIN_BPL-1;
            io->offset -= loffset;
            length += loffset;
        }
#ifdef _WIN32
        SetFilePointer(io->fp,io->offset,(PLONG) &uoffset,FILE_BEGIN) ;
#else
#ifdef _LARGEFILE_SOURCE
        fseeko(io->fp,(((off_t) uoffset) << 32) | ((off_t) io->offset),SEEK_SET) ;
#else
        fseek(io->fp,io->offset,SEEK_SET) ;
#endif
#endif
    }

    nline = 0;
    lp2 = hlp ;	        /* line after  insert */
    lp0 = lp2->prev ;	/* line before insert */
    while((ss=ffgetline(io,&lp1)) == meTRUE)
    {
        /* re-link new line between lp0 and lp1 */
        lp0->next = lp1;
        lp1->prev = lp0;
        lp0 = lp1 ;
        nline++ ;
        if((length > 0) && ((ffread-ffremain) >= length))
            break ;
        if(TTbreakTest(1))
        {
            ss = ctrlg(meFALSE,1) ;
            break ;
        }
    }
    /* complete the link */
    lp2->prev = lp0;
    lp0->next = lp2;

#if MEOPT_SOCKET
    /* loading an ftp or http file can lead to a file name change, free
     * off the old one now that we don't need it any more */
    if(ff && (bp->fileName != fname))
        free(fname) ;
#endif
    if(bp != NULL)
    {
        bp->lineCount += nline ;
        if((flags & meRWFLAG_PRESRVFMOD) == 0)
        {
            bp->fileFlag = io->flags & (meIOFLAG_BINARY|meIOFLAG_LTDIFF) ;
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
                meModeClear(bp->mode,MDCTRLZ) ;
                if(io->flags & meIOFLAG_CR)
                    meModeSet(bp->mode,MDCR) ;
                else
                    meModeClear(bp->mode,MDCR) ;
                if(io->flags & meIOFLAG_LF)
                    meModeSet(bp->mode,MDLF) ;
                else
                    meModeClear(bp->mode,MDLF) ;
            }
        }
    }
    if(length > 0)
    {
        meUInt el, eu=uoffset ;
        el = io->offset + (meUInt) (ffread-ffremain) ;
        if(el < io->offset)
        {
            eu-- ;
            el = 0xffffffff - (((meUInt) (ffread-ffremain)) - io->offset - 1) ;
        }
        sprintf((char *)resultStr,"|0x%x|0x%x|0x%x|0x%x|",io->offset,el,uoffset,eu) ;
    }
    ffReadFileClose(io,flags) ;
    return ss ;
}

static int
ffputBuf(meIo *io)
{
#if MEOPT_CRYPT
    if(io->flags & meIOFLAG_CRYPT)
        meCrypt(ffbuf,ffremain);
#endif
#if MEOPT_SOCKET
    if(ffUrlTypeIsFtp(io->type))
    {
        if(((int) meSocketWrite(io->sock,(char *) ffbuf,ffremain,0)) != ffremain)
        {
            io->flags |= meIOFLAG_ERROR;
            return mlwrite(MWABORT,(meUByte *)"[Send failed - %d]",meSocketGetError()) ;
        }
        else if(io->urlFlags & meIOURLF_CLOSE_PROGRESS)
            ffUrlConsoleAddText(io,(meUByte *)"#",(meLineGetLength(io->urlBp->dotLine) >= frameCur->width - 2) ? 0x02:0x03) ;
    }
    else
#endif
    {
#ifdef _WIN32
        meInt written ;
        if((WriteFile(io->fp,ffbuf,ffremain,(DWORD *)&written,NULL) == 0) || (written != ffremain))
        {
            io->flags |= meIOFLAG_ERROR;
            return mlwrite(MWABORT,(meUByte *)"[Write failed - %d]",meFileGetError());
        }
#else
        if(((int) fwrite(ffbuf,1,ffremain,io->fp)) != ffremain)
        {
            io->flags |= meIOFLAG_ERROR;
            return mlwrite(MWABORT,(meUByte *)"[Write failed - %d]",meFileGetError());
        }
#endif
    }
    ffremain = 0 ;
    return 0 ;
}

#define ffputc(io,c)                                                         \
(((ffbuf[ffremain++]=c),(ffremain == meFIOBUFSIZ))? ffputBuf(io):0)

int
ffWriteFileOpen(meIo *io, meUByte *fname, meUInt flags, meBuffer *bp)
{
    int ii ;
    
    io->flags = 0;
    if(bp != NULL)
    {
#if MEOPT_CRYPT
        if(resetkey(bp) <= 0)
            return meFALSE ;
#endif
    }
    io->type = ffUrlGetType(fname);
    if(ffUrlTypeIsPipe(io->type))
    {
#ifdef _WIN32
        io->fp = GetStdHandle(STD_OUTPUT_HANDLE) ;
#else
        io->fp = stdout ;
#endif
#ifdef _UNIX
        meSigHold() ;
#endif
    }
    else if(ffUrlTypeIsHttpFtp(io->type))
    {
#if MEOPT_SOCKET
        io->redirect = 0;
        if(io->type & (meIOTYPE_SSL|meIOTYPE_HTTP))
            return mlwrite(MWABORT|MWPAUSE,(meUByte *) "[Cannot write to urls of this type]");
#ifdef _UNIX
        /* must stop any signals or the connection will fail */
        meSigHold() ;
#endif
        if((ii=ffUrlFileOpen(io,fname,NULL,NULL,(flags & (meRWFLAG_WRITE|meRWFLAG_BACKUP|meRWFLAG_DELETE|meRWFLAG_MKDIR|meRWFLAG_STAT|meRWFLAG_SILENT)),bp)) <= 0)
        {
#ifdef _UNIX
            meSigRelease() ;
#endif
            return ii ;
        }
        if(!(flags & meRWFLAG_WRITE))
        {
            ii = ffUrlFileClose(io,flags & (meRWFLAG_WRITE|meRWFLAG_BACKUP|meRWFLAG_DELETE|meRWFLAG_MKDIR|meRWFLAG_STAT|meRWFLAG_SILENT)) ;
#ifdef _UNIX
            meSigRelease() ;
#endif
            return ii ;
        }
#else
        return mlwrite(MWABORT|MWPAUSE,(meUByte *) "[No url support in this version]") ;
#endif
    }
    else if(ffUrlTypeIsTfs(io->type))
    {
        return mlwrite(MWABORT|MWPAUSE,(meUByte *) "[Cannot write to TFS]") ;
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
            meUByte filename[meBUF_SIZE_MAX] ;
            
            ii = createBackupName(filename,fname,'~',1) ;
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
                meUByte filenameOldB[meBUF_SIZE_MAX], *filenameOld ;
                
                if(!(meSystemCfg & meSYSTEM_DOSFNAMES))
                {
                    meStrcpy(filenameOldB,fname) ;
                    meStrcat(filenameOldB,".~a~") ;
                    filenameOld = filenameOldB ;
                    if(!meTestExist(filenameOld) && meUnlink(filenameOld))
                        mlwrite(MWABORT|MWPAUSE,(meUByte *)"[Unable to remove backup file %s]", filenameOld) ;
                    else if(meRename(fname,filenameOld) && (ffFileOp(fname,filenameOld,meRWFLAG_DELETE,-1) <= 0))
                        mlwrite(MWABORT|MWPAUSE,(meUByte *)"[Unable to backup file to %s]",filenameOld) ;
                }
                else
                    filenameOld = fname ;
#else
#define filenameOld fname
#endif
#if MEOPT_EXTENDED_BACKUP
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
                            mlwrite(MWABORT|MWPAUSE,(meUByte *)"[Unable to remove backup file %s]", filename) ;
                        ii-- ;
                    }
                    if(ii)
                    {
                        meUByte filename2[meBUF_SIZE_MAX] ;
                        meStrcpy(filename2,filename) ;
                        while(ii > 0)
                        {
                            sprintf((char *)filename2+ll,"%d~",ii--) ;
                            sprintf((char *)filename+ll,"%d~",ii) ;
                            if(meRename(filename,filename2) && (ffFileOp(filename,filename2,meRWFLAG_DELETE,-1) <= 0))
                            {
                                mlwrite(MWABORT|MWPAUSE,(meUByte *)"[Unable to backup file to %s (%d - %s)]",
                                        filename2,errno,sys_errlist[errno]) ;
                                if(meUnlink(filename))
                                {
                                    mlwrite(MWABORT|MWPAUSE,(meUByte *)"[Unable to remove backup file %s]", filename) ;
                                    break ;
                                }
                            }
                        }
                    }
                }
#endif
                if(!meTestExist(filename) && meUnlink(filename))
                    mlwrite(MWABORT|MWPAUSE,(meUByte *)"[Unable to remove backup file %s]", filename) ;
                else if(meRename(filenameOld,filename) && (ffFileOp(filenameOld,filename,meRWFLAG_DELETE,-1) <= 0))
                    mlwrite(MWABORT|MWPAUSE,(meUByte *)"[Unable to backup file to %s (%d - %s)]",
                            filename,errno,sys_errlist[errno]) ;
                else if(bp != NULL)
                {
                    meUShort ss;
#ifdef _DOS
                    ss = bp->stats.stmode & ~(meFILE_ATTRIB_READONLY|meFILE_ATTRIB_HIDDEN) ;
                    if(meSystemCfg & meSYSTEM_HIDEBCKUP)
                        ss |= meFILE_ATTRIB_HIDDEN ;
#endif
#ifdef _WIN32
                    ss = bp->stats.stmode & ~(FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_READONLY) ;
                    if((meSystemCfg & (meSYSTEM_DOSFNAMES|meSYSTEM_HIDEBCKUP)) == meSYSTEM_HIDEBCKUP)
                        ss |= FILE_ATTRIBUTE_HIDDEN ;
#endif
#ifdef _UNIX
                    ss = bp->stats.stmode | S_IWUSR ;
#endif
                    if(ss != bp->stats.stmode)
                        meFileSetAttributes(filename,ss) ;
                }
            }
            else
                mlwrite(MWABORT|MWPAUSE,(meUByte *)"[Unable to backup file %s]",fname) ;
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
                if(ii && (errno == ENOTDIR))
                    /* this happens when its a symbolic link */
                    ii = meUnlink(fname);
#endif
            }
            else
                ii = meUnlink(fname) ;
            if(ii)
            {
                mlwrite(MWCLEXEC|MWABORT,(meUByte *)"[Failed to delete file \"%s\"]",fname) ;
                return -5 ;
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
                    mlwrite(MWABORT,(meUByte *)"[Failed to create directory \"%s\"]",fname) ;
                    return -4 ;
                }
            }
            return meTRUE ;
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
            
            /* Cannot write to a readonly file, and cannot write to hidden or system file if attribs dont include these flag */
            if(!(io->flags & meIOFLAG_NEWFILE) && (fa & (FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM)))
                meFileSetAttributes(fname,FILE_ATTRIBUTE_NORMAL) ;

            /* Windows must open the file with the correct permissions to support the compress attribute */
            if((io->fp=CreateFile((const char *) fname,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ,NULL,create,
                                   ((bp == NULL) ? meUmask:bp->stats.stmode),
                                   NULL)) == INVALID_HANDLE_VALUE)
            {
                mlwrite(MWABORT,(meUByte *)"[Cannot open file \"%s\" for writing - %d]",fname,GetLastError());
                return -3 ;
            }
            if(flags & meRWFLAG_OPENEND)
                SetFilePointer(io->fp,0,NULL,FILE_END) ;
        }
#else
        {
            char *ss ;
            if(flags & meRWFLAG_OPENEND)
                ss = "ab" ;
            else
                ss = "wb" ;
            if((io->fp=fopen((char *)fname,ss)) == NULL)
            {
                mlwrite(MWABORT,(meUByte *)"[Cannot open file \"%s\" for writing]",fname);
                return -3 ;
            }
        }
#endif
        mlwrite(MWCURSOR|MWCLEXEC,(meUByte *)"[%sriting %s]",
                (flags & meRWFLAG_AUTOSAVE) ? "Auto-w":"W",fname);	/* tell us were writing */
#ifdef _UNIX
        meSigHold() ;
#endif
    }
    ffremain = 0 ;
    if(bp != NULL)
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
#if MEOPT_CRYPT
        if(meModeTest(bp->mode,MDCRYPT))
            io->flags |= meIOFLAG_CRYPT;
#endif
#if MEOPT_NARROW
        if((bp->narrow != NULL) && !(flags & meRWFLAG_IGNRNRRW))
            meBufferExpandNarrowAll(bp) ;
#endif
#if MEOPT_TIMSTMP
        if(!(flags & meRWFLAG_AUTOSAVE))
            set_timestamp(bp);
#endif
    }
    else
    {
        io->flags = meIOFLAG_CR;
#if MEOPT_CRYPT
        if(flags & meRWFLAG_CRYPT)
            io->flags |= meIOFLAG_CRYPT;
#endif
    }

#if (defined _DOS)
    /* the directory time stamps on dos are not updated so the best
     * we can do is flag that the dirLists are out of date when WE
     * create a new file. Obviously if something else creates a file
     * then that file many be missed in the completion lists etc
     */
    if(io->flags & meIOFLAG_NEWFILE)
    {
#if MEOPT_EXTENDED
        extern meDirList fileNames ;
        fileNames.stmtime = 1 ;
#endif
        curDirList.stmtime = 1 ;
    }
#endif
    return meTRUE ;
}

int
ffWriteFileWrite(meIo *io, int len, meUByte *buff, int eolFlag)
{
    meUByte cc, dd ;

    if(io->flags & (meIOFLAG_BINMOD|meIOFLAG_RBNMOD))
    {
        if(io->flags & meIOFLAG_BINMOD)
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
                if(!isXDigit(cc) || (--len < 0) || ((dd = *buff++),!isXDigit(dd)))
                {
                    io->flags |= meIOFLAG_ERROR;
                    return mlwrite(MWABORT|MWPAUSE,(meUByte *)"Binary format error");
                }
                cc = (hexToNum(cc)<<4) | hexToNum(dd) ;
                if(ffputc(io,cc))
                    return meABORT ;
            }
        }
        else
        {
            while(--len >= 0)
            {
                cc = *buff++ ;
                if(!isXDigit(cc) || (--len < 0) || ((dd = *buff++),!isXDigit(dd)))
                {
                    io->flags |= meIOFLAG_ERROR;
                    return mlwrite(MWABORT|MWPAUSE,(meUByte *)"[rbin format error]");
                }
                cc = (hexToNum(cc)<<4) | hexToNum(dd) ;
                if(ffputc(io,cc))
                    return meABORT ;
            }
        }
    }
    else
    {
        while(--len >= 0)
        {
            cc=*buff++ ;
            if(ffputc(io,cc))
                return meABORT ;
        }
        if(eolFlag)
        {
            if(((io->flags & meIOFLAG_CR) && ffputc(io,'\r')) ||
               ((io->flags & meIOFLAG_LF) && ffputc(io,'\n')))
                return meABORT ;
        }
    }
    return meTRUE ;
}

int
ffWriteFileClose(meIo *io, meUInt flags, meBuffer *bp)
{
    int ret ;

    /* add a ^Z at the end of the file if needed */
    ret = meABORT ;
    if(!(io->flags & meIOFLAG_ERROR))
    {
        if(io->flags & meIOFLAG_CTRLZ)
            ffputc(io,26) ;
        if(ffputBuf(io) >= 0)
            ret = meTRUE ;
    }

#if MEOPT_SOCKET
    if(ffUrlTypeIsHttpFtp(io->type))
    {
        if(ffUrlFileClose(io,flags) <= 0)
            ret = meABORT ;
#ifdef _UNIX
        meSigRelease() ;
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
        meSigRelease() ;
#endif
    }

#if MEOPT_NARROW
    if((bp != NULL) && (bp->narrow != NULL) && !(flags & meRWFLAG_IGNRNRRW))
        meBufferCollapseNarrowAll(bp) ;
#endif
    return ret ;
}

int
ffWriteFile(meIo *io, meUByte *fname, meUInt flags, meBuffer *bp)
{
    flags |= meRWFLAG_WRITE ;
    if(ffWriteFileOpen(io,fname,flags,bp) > 0)
    {
        register meLine *lp ;
        long noLines ;

        /* store the number of lines now cos the close may narrow the buffer and the number of
         * lines will be wrong */
        noLines = bp->lineCount+1 ;
        lp = meLineGetNext(bp->baseLine);            /* First line.          */
        while(lp != bp->baseLine)
        {
#if MEOPT_EXTENDED
            if(lp->flag & meLINE_MARKUP)
                noLines-- ;
            else
#endif
            {
                if(ffWriteFileWrite(io,meLineGetLength(lp),meLineGetText(lp),
                                    !(lp->flag & meLINE_NOEOL)) <= 0)
                    break ;
            }
            lp = meLineGetNext(lp) ;
        }
        if(ffWriteFileClose(io,flags,bp) > 0)
        {
            if(io->flags & meIOFLAG_NEWFILE)
                mlwrite(MWCLEXEC,(meUByte *)"[New file %s, Wrote %d lines]",fname,noLines);
            else
                mlwrite(MWCLEXEC,(meUByte *)"[Wrote %d lines]",noLines);
            return meTRUE ;
        }
    }
    return meFALSE ;
}

#if MEOPT_EXTENDED
int
ffFileOp(meUByte *sfname, meUByte *dfname, meUInt dFlags, meInt fileMode)
{
    int rr=meTRUE, r1 ;

    /* By now sfname & dfname should have had a 'file:' URL prefix removed */
    meior.type = ffUrlGetType(sfname);
    meAssert((meior.type & meIOTYPE_FILE) == 0);
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
                dFlags &= ~meRWFLAG_DELETE ;
                dfname = NULL ;
            }
            else
                rr = mlwrite(MWABORT,(meUByte *)"[Failed to rename file]") ;
        }
#if MEOPT_SOCKET
        else if(ffUrlTypeIsFtpu(meior.type) && ffUrlTypeIsFtpu(meiow.type))
        {
            /* If support for ftps is added then the above test needs to check that both are same type, i.e. not ftp: & ftps: */ 
            meUByte cc, *sfn, *dfn, sfbuff[meBUF_SIZE_MAX], dfbuff[meBUF_SIZE_MAX];
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
                sfn = sfname + ll;
                dfn = dfname + ll;
                if((sfn[1] == '~') && (sfn[2] == '/'))
                {
                    if(meior.sockHome != NULL)
                    {
                        meStrcpy(sfbuff,meior.sockHome);
                        if(((ll = meStrlen(sfbuff)) > 0) && (sfbuff[ll-1] == '/'))
                            ll--;
                    }
                    else
                        ll = 0;
                    meStrcpy(sfbuff+ll,sfn+2) ;
                    sfn = sfbuff ;
                }
                if((dfn[1] == '~') && (dfn[2] == '/'))
                {
                    if(meior.sockHome != NULL)
                    {
                        meStrcpy(dfbuff,meior.sockHome);
                        if(((ll = meStrlen(dfbuff)) > 0) && (dfbuff[ll-1] == '/'))
                            ll--;
                    }
                    else
                        ll = 0;
                    meStrcpy(dfbuff+ll,dfn+2);
                    dfn = dfbuff;
                }
                if((ftpCommand(&meior,0,"RNFR %s",sfn) != ftpPOS_INTERMED) ||
                   (ftpCommand(&meior,0,"RNTO %s",dfn) != ftpPOS_COMPLETE))
                    rr = mlwrite(MWABORT,(meUByte *)"[Failed to rename ftp file]") ;
#ifdef _UNIX
                meSigRelease() ;
#endif
                dFlags &= ~meRWFLAG_DELETE ;
                dfname = NULL ;
            }
        }
#endif
    }
    if(dfname != NULL)
    {
#ifdef _UNIX
        struct utimbuf fileTimes ;
#endif
#ifdef _WIN32
        FILETIME creationTime, lastAccessTime, lastWriteTime ;
#endif
        int presTimeStamp ;

        if((rr=ffReadFileOpen(&meior,sfname,meRWFLAG_READ|(dFlags & (meRWFLAG_NODIRLIST|meRWFLAG_SILENT)),NULL)) <= 0)
            return rr ;
        if((rr=ffWriteFileOpen(&meiow,dfname,meRWFLAG_WRITE|(dFlags & (meRWFLAG_BACKUP|meRWFLAG_SILENT)),NULL)) <= 0)
        {
            ffReadFileClose(&meior,meRWFLAG_READ|(dFlags & meRWFLAG_SILENT)) ;
            return rr ;
        }
        presTimeStamp = ((dFlags & meRWFLAG_PRESRVTS) && (meior.fp != meBadFile) && (meiow.fp != meBadFile)) ;
        if(presTimeStamp)
        {
#ifdef _UNIX
            struct stat stt ;
            if((presTimeStamp = (fstat(fileno(meior.fp),&stt) == 0)))
            {
                fileTimes.actime = stt.st_atime ;
                fileTimes.modtime = stt.st_mtime ;
            }
#endif
#ifdef _WIN32
            presTimeStamp = GetFileTime(meior.fp,&creationTime,&lastAccessTime,&lastWriteTime) ;
#endif
        }
        while(((r1=ffgetBuf(&meior)) > 0) && (ffremain > 0) && (ffputBuf(&meiow) >= 0))
            ;
        ffReadFileClose(&meior,meRWFLAG_READ|(dFlags & meRWFLAG_SILENT)) ;
        ffremain = 0 ;
#ifdef _WIN32
        if(presTimeStamp)
            SetFileTime(meiow.fp,&creationTime,&lastAccessTime,&lastWriteTime) ;
#endif
        rr = ffWriteFileClose(&meiow,meRWFLAG_WRITE|(dFlags & meRWFLAG_SILENT),NULL) ;
        if(r1 < 0)
            rr = r1 ;
        else if((rr > 0) && (fileMode >= 0) && !ffUrlTypeIsNotFile(meiow.type))
        {
            meFileSetAttributes(dfname,fileMode) ;
#ifdef _UNIX
            if(presTimeStamp)
                utime((char *) dfname,&fileTimes) ;
#endif
        }
    }
    if((rr > 0) && (dFlags & (meRWFLAG_DELETE|meRWFLAG_MKDIR|meRWFLAG_STAT)))
    {
        rr = ffWriteFileOpen(&meior,sfname,dFlags & (meRWFLAG_DELETE|meRWFLAG_MKDIR|meRWFLAG_STAT|meRWFLAG_BACKUP|meRWFLAG_SILENT),NULL) ;
    }
#if MEOPT_SOCKET
    /* TODO - Can a http/https socket be reused? Regardless of whether content-length sent, can we simply send the next request? And is this reliable?
     * If so may improve performance by removing the overhead of opening the connection (particularly for https) in which case it may be better to
     * look at sockUrl so https? can keep open and extend FTPCLOSE to include HTTP. */
    if((rr > 0) && (dFlags & meRWFLAG_FTPCLOSE) && (!meSocketIsBad(meior.ccsk) || !meSocketIsBad(meiow.ccsk)))
    {
#ifdef _UNIX
        meSigHold() ;
#endif
        if(!meSocketIsBad(meior.ccsk))
        {
            if(!(dFlags & meRWFLAG_NOCONSOLE))
                ffUrlFileSetupFlags(&meior,dFlags);
            ffCloseSockets(&meior,1);
        }
        if(!meSocketIsBad(meiow.ccsk))
        {
            if(!(dFlags & meRWFLAG_NOCONSOLE))
                ffUrlFileSetupFlags(&meiow,dFlags);
            ffCloseSockets(&meiow,1);
        }
#ifdef _UNIX
        meSigRelease() ;
#endif
    }
#endif
    return rr ;
}
#endif
