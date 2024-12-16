/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * crypt.c - Encryption Routines.
 *
 * Copyright (C) 2000-2024 JASSPA (www.jasspa.com)
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
 * Created:     Steven Phillips
 * Synopsis:    Encryption Routines.
 * Authors:     Steven Phillips and contains code by Christophe Devine,
 *              David Blackman & Sebastiano Vigna
 * Notes:	Complete reauthoring of crypt to use AES
 */

#define	__CRYPTC		/* Define file */

#include "emain.h"

/* Init as if user called &set $random 1 */

/* a splitMix32 PRNG is used to init the 4 uint state of xoshiro128++ */
void
meXoshiro128Seed(void)
{
    meUInt sd, vv;
    int ii=3; 
    
#ifdef _WIN32
    sd = ((meUInt) GetTickCount()) ^ (((meUInt) GetCurrentProcessId()) << 12);
#else
    sd = ((meUInt) time(NULL)) ^ (((meUInt) getpid()) << 12);
#endif
    do {
        vv = (sd += 0x9e3779b9);
        vv ^= vv >> 16;
        vv *= 0x21f0aaad;
        vv ^= vv >> 15;
        vv *= 0x735a2d97;
        meRndSeed[ii] = vv ^ (vv >> 15);
    } while(--ii >= 0);
}

/*  Written in 2019 by David Blackman and Sebastiano Vigna (vigna@acm.org)

   To the extent possible under law, the author has dedicated all copyright
   and related and neighboring rights to this software to the public domain
   worldwide. This software is distributed without any warranty.
   
   See <http://creativecommons.org/publicdomain/zero/1.0/>.

   This is xoshiro128++ 1.0, one of our 32-bit all-purpose, rock-solid
   generators. It has excellent speed, a state size (128 bits) that is
   large enough for mild parallelism, and it passes all tests we are aware
   of.

   The state must be seeded so that it is not everywhere zero. */

#define xoshiro128Rotl(xx,kk) (((xx) << (kk)) | ((xx) >> (32 - (kk))))

meUInt
meXoshiro128Next(void)
{
    register meUInt rr=meRndSeed[0] + meRndSeed[3], tt=meRndSeed[1] << 9;
    rr = xoshiro128Rotl(rr,7) + meRndSeed[0];

    meRndSeed[2] ^= meRndSeed[0];
    meRndSeed[3] ^= meRndSeed[1];
    meRndSeed[1] ^= meRndSeed[2];
    meRndSeed[0] ^= meRndSeed[3];
    meRndSeed[2] ^= tt;
    meRndSeed[3] = xoshiro128Rotl(meRndSeed[3],11);

    return rr;
}


/*
 * Create a file name within the temp folder, name can be given or random one generated.
 * On windows & dos concatinate the $TEMP variable with the filename to create the new
 * temporary file name,  under unix file in /tmp/
 */
void
mkTempName(meUByte *buf, meUByte *name, meUByte *ext)
{
    meUByte *pp;
    int ii,jj,kk,rr;
#ifndef _UNIX
#ifdef _CONVDIR_CHAR
#define PIPEDIR_CHAR _CONVDIR_CHAR
#else
#define PIPEDIR_CHAR DIR_CHAR
#endif
    static meUByte *tmpDir=NULL;

    if(tmpDir == NULL)
    {
        /* Get location of the temporary directory from the environment $TEMP */
        if((((pp = (meUByte *) meGetenv("TMP")) == NULL) || meTestDir(pp)) &&
           (((pp = (meUByte *) meGetenv("TEMP")) == NULL) || meTestDir(pp)))
        {
            pp = NULL;
#if (defined _DOS) || (defined _WIN32)
            /* the C drive is more reliable than ./ as ./ could be on a CD-Rom etc, but in recent
             * versions of windows c:\ is readonly so look around for a temp folder */
            if(!meTestDir((meUByte *) "c:\\tmp\\"))
                tmpDir = (meUByte *) "c:\\tmp\\";
            else if(!meTestDir((meUByte *) "c:\\temp\\"))
                tmpDir = (meUByte *) "c:\\temp\\";
#if (defined _WIN32)
            else if(((pp = (meUByte *) meGetenv("LOCALAPPDATA")) != NULL) && !meTestDir(pp))
            {
                ii = meStrlen(pp);
                memcpy(buf,pp,ii);
                pp = buf;
                if(buf[ii-1] != PIPEDIR_CHAR)
                    buf[ii++] = PIPEDIR_CHAR;
                meStrcpy(buf+ii,"Temp");
                if(meTestDir(buf))
                    buf[ii] = '\0';
            }
#endif
#endif
        }
        if(tmpDir == NULL)
        {
            if((pp != NULL) && ((ii = meStrlen(pp)) > 0) && ((tmpDir = meMalloc(ii+2)) != NULL))
            {
                memcpy(tmpDir,pp,ii);
                if(tmpDir[ii-1] != PIPEDIR_CHAR)
                    tmpDir[ii++] = PIPEDIR_CHAR;
                tmpDir[ii] = '\0';
            }
            else 
#if (defined _DOS) || (defined _WIN32)
                tmpDir = (meUByte *) "c:\\";
#else
                tmpDir = (meUByte *) "./";
#endif
        }
    }
#endif
    
    if(ext == NULL)
        ext = (meUByte *) "";

    if(name != NULL)
#ifdef _UNIX
        sprintf((char *)buf,"/tmp/me%s%s",name,ext);
#else
        sprintf((char *)buf,"%sme%s%s",tmpDir,name,ext);
#endif
    else
    {
        if(meRndSeed[0] == 0)
            meXoshiro128Seed();
#ifdef _UNIX
        memcpy(buf,"/tmp/meXXXXXX",13);
        strcpy((char *) buf+13,(char *) ext);
        pp = buf+7;
#else
        ii = sprintf((char *) buf,"%smeXXXXXX",tmpDir);
        strcpy((char *) buf+ii,(char *) ext);
        pp = buf+ii-6;
#endif
        for(ii=0 ; ii<999 ; ii++)
        {
            rr = meXoshiro128Next();
            jj = 5;
            do {
                kk = rr & 0x1f;
                pp[jj] = (meUByte) (kk + ((kk > 25) ? 48-25:97));
                rr >>= 5;
            } while(--jj >= 0);
            if(meTestExist(buf))
                break;
        }
    }
}


#if MEOPT_CRYPT
/*
 *  FIPS-197 compliant AES implementation
 *
 *  Copyright (C) 2001-2003  Christophe Devine
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* forward S-box */

static meUInt FSb[256] =
{
    0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5,
    0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76,
    0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0,
    0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0,
    0xB7, 0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC,
    0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15,
    0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A,
    0x07, 0x12, 0x80, 0xE2, 0xEB, 0x27, 0xB2, 0x75,
    0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0,
    0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84,
    0x53, 0xD1, 0x00, 0xED, 0x20, 0xFC, 0xB1, 0x5B,
    0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF,
    0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85,
    0x45, 0xF9, 0x02, 0x7F, 0x50, 0x3C, 0x9F, 0xA8,
    0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5,
    0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2,
    0xCD, 0x0C, 0x13, 0xEC, 0x5F, 0x97, 0x44, 0x17,
    0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73,
    0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88,
    0x46, 0xEE, 0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB,
    0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C,
    0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79,
    0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E, 0xA9,
    0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08,
    0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6,
    0xE8, 0xDD, 0x74, 0x1F, 0x4B, 0xBD, 0x8B, 0x8A,
    0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E,
    0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E,
    0xE1, 0xF8, 0x98, 0x11, 0x69, 0xD9, 0x8E, 0x94,
    0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF,
    0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68,
    0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16
};

/* forward table */

#define FT \
\
    V(C6,63,63,A5), V(F8,7C,7C,84), V(EE,77,77,99), V(F6,7B,7B,8D), \
    V(FF,F2,F2,0D), V(D6,6B,6B,BD), V(DE,6F,6F,B1), V(91,C5,C5,54), \
    V(60,30,30,50), V(02,01,01,03), V(CE,67,67,A9), V(56,2B,2B,7D), \
    V(E7,FE,FE,19), V(B5,D7,D7,62), V(4D,AB,AB,E6), V(EC,76,76,9A), \
    V(8F,CA,CA,45), V(1F,82,82,9D), V(89,C9,C9,40), V(FA,7D,7D,87), \
    V(EF,FA,FA,15), V(B2,59,59,EB), V(8E,47,47,C9), V(FB,F0,F0,0B), \
    V(41,AD,AD,EC), V(B3,D4,D4,67), V(5F,A2,A2,FD), V(45,AF,AF,EA), \
    V(23,9C,9C,BF), V(53,A4,A4,F7), V(E4,72,72,96), V(9B,C0,C0,5B), \
    V(75,B7,B7,C2), V(E1,FD,FD,1C), V(3D,93,93,AE), V(4C,26,26,6A), \
    V(6C,36,36,5A), V(7E,3F,3F,41), V(F5,F7,F7,02), V(83,CC,CC,4F), \
    V(68,34,34,5C), V(51,A5,A5,F4), V(D1,E5,E5,34), V(F9,F1,F1,08), \
    V(E2,71,71,93), V(AB,D8,D8,73), V(62,31,31,53), V(2A,15,15,3F), \
    V(08,04,04,0C), V(95,C7,C7,52), V(46,23,23,65), V(9D,C3,C3,5E), \
    V(30,18,18,28), V(37,96,96,A1), V(0A,05,05,0F), V(2F,9A,9A,B5), \
    V(0E,07,07,09), V(24,12,12,36), V(1B,80,80,9B), V(DF,E2,E2,3D), \
    V(CD,EB,EB,26), V(4E,27,27,69), V(7F,B2,B2,CD), V(EA,75,75,9F), \
    V(12,09,09,1B), V(1D,83,83,9E), V(58,2C,2C,74), V(34,1A,1A,2E), \
    V(36,1B,1B,2D), V(DC,6E,6E,B2), V(B4,5A,5A,EE), V(5B,A0,A0,FB), \
    V(A4,52,52,F6), V(76,3B,3B,4D), V(B7,D6,D6,61), V(7D,B3,B3,CE), \
    V(52,29,29,7B), V(DD,E3,E3,3E), V(5E,2F,2F,71), V(13,84,84,97), \
    V(A6,53,53,F5), V(B9,D1,D1,68), V(00,00,00,00), V(C1,ED,ED,2C), \
    V(40,20,20,60), V(E3,FC,FC,1F), V(79,B1,B1,C8), V(B6,5B,5B,ED), \
    V(D4,6A,6A,BE), V(8D,CB,CB,46), V(67,BE,BE,D9), V(72,39,39,4B), \
    V(94,4A,4A,DE), V(98,4C,4C,D4), V(B0,58,58,E8), V(85,CF,CF,4A), \
    V(BB,D0,D0,6B), V(C5,EF,EF,2A), V(4F,AA,AA,E5), V(ED,FB,FB,16), \
    V(86,43,43,C5), V(9A,4D,4D,D7), V(66,33,33,55), V(11,85,85,94), \
    V(8A,45,45,CF), V(E9,F9,F9,10), V(04,02,02,06), V(FE,7F,7F,81), \
    V(A0,50,50,F0), V(78,3C,3C,44), V(25,9F,9F,BA), V(4B,A8,A8,E3), \
    V(A2,51,51,F3), V(5D,A3,A3,FE), V(80,40,40,C0), V(05,8F,8F,8A), \
    V(3F,92,92,AD), V(21,9D,9D,BC), V(70,38,38,48), V(F1,F5,F5,04), \
    V(63,BC,BC,DF), V(77,B6,B6,C1), V(AF,DA,DA,75), V(42,21,21,63), \
    V(20,10,10,30), V(E5,FF,FF,1A), V(FD,F3,F3,0E), V(BF,D2,D2,6D), \
    V(81,CD,CD,4C), V(18,0C,0C,14), V(26,13,13,35), V(C3,EC,EC,2F), \
    V(BE,5F,5F,E1), V(35,97,97,A2), V(88,44,44,CC), V(2E,17,17,39), \
    V(93,C4,C4,57), V(55,A7,A7,F2), V(FC,7E,7E,82), V(7A,3D,3D,47), \
    V(C8,64,64,AC), V(BA,5D,5D,E7), V(32,19,19,2B), V(E6,73,73,95), \
    V(C0,60,60,A0), V(19,81,81,98), V(9E,4F,4F,D1), V(A3,DC,DC,7F), \
    V(44,22,22,66), V(54,2A,2A,7E), V(3B,90,90,AB), V(0B,88,88,83), \
    V(8C,46,46,CA), V(C7,EE,EE,29), V(6B,B8,B8,D3), V(28,14,14,3C), \
    V(A7,DE,DE,79), V(BC,5E,5E,E2), V(16,0B,0B,1D), V(AD,DB,DB,76), \
    V(DB,E0,E0,3B), V(64,32,32,56), V(74,3A,3A,4E), V(14,0A,0A,1E), \
    V(92,49,49,DB), V(0C,06,06,0A), V(48,24,24,6C), V(B8,5C,5C,E4), \
    V(9F,C2,C2,5D), V(BD,D3,D3,6E), V(43,AC,AC,EF), V(C4,62,62,A6), \
    V(39,91,91,A8), V(31,95,95,A4), V(D3,E4,E4,37), V(F2,79,79,8B), \
    V(D5,E7,E7,32), V(8B,C8,C8,43), V(6E,37,37,59), V(DA,6D,6D,B7), \
    V(01,8D,8D,8C), V(B1,D5,D5,64), V(9C,4E,4E,D2), V(49,A9,A9,E0), \
    V(D8,6C,6C,B4), V(AC,56,56,FA), V(F3,F4,F4,07), V(CF,EA,EA,25), \
    V(CA,65,65,AF), V(F4,7A,7A,8E), V(47,AE,AE,E9), V(10,08,08,18), \
    V(6F,BA,BA,D5), V(F0,78,78,88), V(4A,25,25,6F), V(5C,2E,2E,72), \
    V(38,1C,1C,24), V(57,A6,A6,F1), V(73,B4,B4,C7), V(97,C6,C6,51), \
    V(CB,E8,E8,23), V(A1,DD,DD,7C), V(E8,74,74,9C), V(3E,1F,1F,21), \
    V(96,4B,4B,DD), V(61,BD,BD,DC), V(0D,8B,8B,86), V(0F,8A,8A,85), \
    V(E0,70,70,90), V(7C,3E,3E,42), V(71,B5,B5,C4), V(CC,66,66,AA), \
    V(90,48,48,D8), V(06,03,03,05), V(F7,F6,F6,01), V(1C,0E,0E,12), \
    V(C2,61,61,A3), V(6A,35,35,5F), V(AE,57,57,F9), V(69,B9,B9,D0), \
    V(17,86,86,91), V(99,C1,C1,58), V(3A,1D,1D,27), V(27,9E,9E,B9), \
    V(D9,E1,E1,38), V(EB,F8,F8,13), V(2B,98,98,B3), V(22,11,11,33), \
    V(D2,69,69,BB), V(A9,D9,D9,70), V(07,8E,8E,89), V(33,94,94,A7), \
    V(2D,9B,9B,B6), V(3C,1E,1E,22), V(15,87,87,92), V(C9,E9,E9,20), \
    V(87,CE,CE,49), V(AA,55,55,FF), V(50,28,28,78), V(A5,DF,DF,7A), \
    V(03,8C,8C,8F), V(59,A1,A1,F8), V(09,89,89,80), V(1A,0D,0D,17), \
    V(65,BF,BF,DA), V(D7,E6,E6,31), V(84,42,42,C6), V(D0,68,68,B8), \
    V(82,41,41,C3), V(29,99,99,B0), V(5A,2D,2D,77), V(1E,0F,0F,11), \
    V(7B,B0,B0,CB), V(A8,54,54,FC), V(6D,BB,BB,D6), V(2C,16,16,3A)

#define V(a,b,c,d) 0x##a##b##c##d
static meUInt FT0[256] = { FT };
#undef V

#define V(a,b,c,d) 0x##d##a##b##c
static meUInt FT1[256] = { FT };
#undef V

#define V(a,b,c,d) 0x##c##d##a##b
static meUInt FT2[256] = { FT };
#undef V

#define V(a,b,c,d) 0x##b##c##d##a
static meUInt FT3[256] = { FT };
#undef V

#undef FT

/* reverse S-box */

static meUInt RSb[256] =
{
    0x52, 0x09, 0x6A, 0xD5, 0x30, 0x36, 0xA5, 0x38,
    0xBF, 0x40, 0xA3, 0x9E, 0x81, 0xF3, 0xD7, 0xFB,
    0x7C, 0xE3, 0x39, 0x82, 0x9B, 0x2F, 0xFF, 0x87,
    0x34, 0x8E, 0x43, 0x44, 0xC4, 0xDE, 0xE9, 0xCB,
    0x54, 0x7B, 0x94, 0x32, 0xA6, 0xC2, 0x23, 0x3D,
    0xEE, 0x4C, 0x95, 0x0B, 0x42, 0xFA, 0xC3, 0x4E,
    0x08, 0x2E, 0xA1, 0x66, 0x28, 0xD9, 0x24, 0xB2,
    0x76, 0x5B, 0xA2, 0x49, 0x6D, 0x8B, 0xD1, 0x25,
    0x72, 0xF8, 0xF6, 0x64, 0x86, 0x68, 0x98, 0x16,
    0xD4, 0xA4, 0x5C, 0xCC, 0x5D, 0x65, 0xB6, 0x92,
    0x6C, 0x70, 0x48, 0x50, 0xFD, 0xED, 0xB9, 0xDA,
    0x5E, 0x15, 0x46, 0x57, 0xA7, 0x8D, 0x9D, 0x84,
    0x90, 0xD8, 0xAB, 0x00, 0x8C, 0xBC, 0xD3, 0x0A,
    0xF7, 0xE4, 0x58, 0x05, 0xB8, 0xB3, 0x45, 0x06,
    0xD0, 0x2C, 0x1E, 0x8F, 0xCA, 0x3F, 0x0F, 0x02,
    0xC1, 0xAF, 0xBD, 0x03, 0x01, 0x13, 0x8A, 0x6B,
    0x3A, 0x91, 0x11, 0x41, 0x4F, 0x67, 0xDC, 0xEA,
    0x97, 0xF2, 0xCF, 0xCE, 0xF0, 0xB4, 0xE6, 0x73,
    0x96, 0xAC, 0x74, 0x22, 0xE7, 0xAD, 0x35, 0x85,
    0xE2, 0xF9, 0x37, 0xE8, 0x1C, 0x75, 0xDF, 0x6E,
    0x47, 0xF1, 0x1A, 0x71, 0x1D, 0x29, 0xC5, 0x89,
    0x6F, 0xB7, 0x62, 0x0E, 0xAA, 0x18, 0xBE, 0x1B,
    0xFC, 0x56, 0x3E, 0x4B, 0xC6, 0xD2, 0x79, 0x20,
    0x9A, 0xDB, 0xC0, 0xFE, 0x78, 0xCD, 0x5A, 0xF4,
    0x1F, 0xDD, 0xA8, 0x33, 0x88, 0x07, 0xC7, 0x31,
    0xB1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xEC, 0x5F,
    0x60, 0x51, 0x7F, 0xA9, 0x19, 0xB5, 0x4A, 0x0D,
    0x2D, 0xE5, 0x7A, 0x9F, 0x93, 0xC9, 0x9C, 0xEF,
    0xA0, 0xE0, 0x3B, 0x4D, 0xAE, 0x2A, 0xF5, 0xB0,
    0xC8, 0xEB, 0xBB, 0x3C, 0x83, 0x53, 0x99, 0x61,
    0x17, 0x2B, 0x04, 0x7E, 0xBA, 0x77, 0xD6, 0x26,
    0xE1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0C, 0x7D
};

/* reverse table */

#define RT \
\
    V(51,F4,A7,50), V(7E,41,65,53), V(1A,17,A4,C3), V(3A,27,5E,96), \
    V(3B,AB,6B,CB), V(1F,9D,45,F1), V(AC,FA,58,AB), V(4B,E3,03,93), \
    V(20,30,FA,55), V(AD,76,6D,F6), V(88,CC,76,91), V(F5,02,4C,25), \
    V(4F,E5,D7,FC), V(C5,2A,CB,D7), V(26,35,44,80), V(B5,62,A3,8F), \
    V(DE,B1,5A,49), V(25,BA,1B,67), V(45,EA,0E,98), V(5D,FE,C0,E1), \
    V(C3,2F,75,02), V(81,4C,F0,12), V(8D,46,97,A3), V(6B,D3,F9,C6), \
    V(03,8F,5F,E7), V(15,92,9C,95), V(BF,6D,7A,EB), V(95,52,59,DA), \
    V(D4,BE,83,2D), V(58,74,21,D3), V(49,E0,69,29), V(8E,C9,C8,44), \
    V(75,C2,89,6A), V(F4,8E,79,78), V(99,58,3E,6B), V(27,B9,71,DD), \
    V(BE,E1,4F,B6), V(F0,88,AD,17), V(C9,20,AC,66), V(7D,CE,3A,B4), \
    V(63,DF,4A,18), V(E5,1A,31,82), V(97,51,33,60), V(62,53,7F,45), \
    V(B1,64,77,E0), V(BB,6B,AE,84), V(FE,81,A0,1C), V(F9,08,2B,94), \
    V(70,48,68,58), V(8F,45,FD,19), V(94,DE,6C,87), V(52,7B,F8,B7), \
    V(AB,73,D3,23), V(72,4B,02,E2), V(E3,1F,8F,57), V(66,55,AB,2A), \
    V(B2,EB,28,07), V(2F,B5,C2,03), V(86,C5,7B,9A), V(D3,37,08,A5), \
    V(30,28,87,F2), V(23,BF,A5,B2), V(02,03,6A,BA), V(ED,16,82,5C), \
    V(8A,CF,1C,2B), V(A7,79,B4,92), V(F3,07,F2,F0), V(4E,69,E2,A1), \
    V(65,DA,F4,CD), V(06,05,BE,D5), V(D1,34,62,1F), V(C4,A6,FE,8A), \
    V(34,2E,53,9D), V(A2,F3,55,A0), V(05,8A,E1,32), V(A4,F6,EB,75), \
    V(0B,83,EC,39), V(40,60,EF,AA), V(5E,71,9F,06), V(BD,6E,10,51), \
    V(3E,21,8A,F9), V(96,DD,06,3D), V(DD,3E,05,AE), V(4D,E6,BD,46), \
    V(91,54,8D,B5), V(71,C4,5D,05), V(04,06,D4,6F), V(60,50,15,FF), \
    V(19,98,FB,24), V(D6,BD,E9,97), V(89,40,43,CC), V(67,D9,9E,77), \
    V(B0,E8,42,BD), V(07,89,8B,88), V(E7,19,5B,38), V(79,C8,EE,DB), \
    V(A1,7C,0A,47), V(7C,42,0F,E9), V(F8,84,1E,C9), V(00,00,00,00), \
    V(09,80,86,83), V(32,2B,ED,48), V(1E,11,70,AC), V(6C,5A,72,4E), \
    V(FD,0E,FF,FB), V(0F,85,38,56), V(3D,AE,D5,1E), V(36,2D,39,27), \
    V(0A,0F,D9,64), V(68,5C,A6,21), V(9B,5B,54,D1), V(24,36,2E,3A), \
    V(0C,0A,67,B1), V(93,57,E7,0F), V(B4,EE,96,D2), V(1B,9B,91,9E), \
    V(80,C0,C5,4F), V(61,DC,20,A2), V(5A,77,4B,69), V(1C,12,1A,16), \
    V(E2,93,BA,0A), V(C0,A0,2A,E5), V(3C,22,E0,43), V(12,1B,17,1D), \
    V(0E,09,0D,0B), V(F2,8B,C7,AD), V(2D,B6,A8,B9), V(14,1E,A9,C8), \
    V(57,F1,19,85), V(AF,75,07,4C), V(EE,99,DD,BB), V(A3,7F,60,FD), \
    V(F7,01,26,9F), V(5C,72,F5,BC), V(44,66,3B,C5), V(5B,FB,7E,34), \
    V(8B,43,29,76), V(CB,23,C6,DC), V(B6,ED,FC,68), V(B8,E4,F1,63), \
    V(D7,31,DC,CA), V(42,63,85,10), V(13,97,22,40), V(84,C6,11,20), \
    V(85,4A,24,7D), V(D2,BB,3D,F8), V(AE,F9,32,11), V(C7,29,A1,6D), \
    V(1D,9E,2F,4B), V(DC,B2,30,F3), V(0D,86,52,EC), V(77,C1,E3,D0), \
    V(2B,B3,16,6C), V(A9,70,B9,99), V(11,94,48,FA), V(47,E9,64,22), \
    V(A8,FC,8C,C4), V(A0,F0,3F,1A), V(56,7D,2C,D8), V(22,33,90,EF), \
    V(87,49,4E,C7), V(D9,38,D1,C1), V(8C,CA,A2,FE), V(98,D4,0B,36), \
    V(A6,F5,81,CF), V(A5,7A,DE,28), V(DA,B7,8E,26), V(3F,AD,BF,A4), \
    V(2C,3A,9D,E4), V(50,78,92,0D), V(6A,5F,CC,9B), V(54,7E,46,62), \
    V(F6,8D,13,C2), V(90,D8,B8,E8), V(2E,39,F7,5E), V(82,C3,AF,F5), \
    V(9F,5D,80,BE), V(69,D0,93,7C), V(6F,D5,2D,A9), V(CF,25,12,B3), \
    V(C8,AC,99,3B), V(10,18,7D,A7), V(E8,9C,63,6E), V(DB,3B,BB,7B), \
    V(CD,26,78,09), V(6E,59,18,F4), V(EC,9A,B7,01), V(83,4F,9A,A8), \
    V(E6,95,6E,65), V(AA,FF,E6,7E), V(21,BC,CF,08), V(EF,15,E8,E6), \
    V(BA,E7,9B,D9), V(4A,6F,36,CE), V(EA,9F,09,D4), V(29,B0,7C,D6), \
    V(31,A4,B2,AF), V(2A,3F,23,31), V(C6,A5,94,30), V(35,A2,66,C0), \
    V(74,4E,BC,37), V(FC,82,CA,A6), V(E0,90,D0,B0), V(33,A7,D8,15), \
    V(F1,04,98,4A), V(41,EC,DA,F7), V(7F,CD,50,0E), V(17,91,F6,2F), \
    V(76,4D,D6,8D), V(43,EF,B0,4D), V(CC,AA,4D,54), V(E4,96,04,DF), \
    V(9E,D1,B5,E3), V(4C,6A,88,1B), V(C1,2C,1F,B8), V(46,65,51,7F), \
    V(9D,5E,EA,04), V(01,8C,35,5D), V(FA,87,74,73), V(FB,0B,41,2E), \
    V(B3,67,1D,5A), V(92,DB,D2,52), V(E9,10,56,33), V(6D,D6,47,13), \
    V(9A,D7,61,8C), V(37,A1,0C,7A), V(59,F8,14,8E), V(EB,13,3C,89), \
    V(CE,A9,27,EE), V(B7,61,C9,35), V(E1,1C,E5,ED), V(7A,47,B1,3C), \
    V(9C,D2,DF,59), V(55,F2,73,3F), V(18,14,CE,79), V(73,C7,37,BF), \
    V(53,F7,CD,EA), V(5F,FD,AA,5B), V(DF,3D,6F,14), V(78,44,DB,86), \
    V(CA,AF,F3,81), V(B9,68,C4,3E), V(38,24,34,2C), V(C2,A3,40,5F), \
    V(16,1D,C3,72), V(BC,E2,25,0C), V(28,3C,49,8B), V(FF,0D,95,41), \
    V(39,A8,01,71), V(08,0C,B3,DE), V(D8,B4,E4,9C), V(64,56,C1,90), \
    V(7B,CB,84,61), V(D5,32,B6,70), V(48,6C,5C,74), V(D0,B8,57,42)

#define V(a,b,c,d) 0x##a##b##c##d
static meUInt RT0[256] = { RT };
#undef V

#define V(a,b,c,d) 0x##d##a##b##c
static meUInt RT1[256] = { RT };
#undef V

#define V(a,b,c,d) 0x##c##d##a##b
static meUInt RT2[256] = { RT };
#undef V

#define V(a,b,c,d) 0x##b##c##d##a
static meUInt RT3[256] = { RT };
#undef V

#undef RT

/* round constants */

static meUInt RCON[10] =
{
    0x01000000, 0x02000000, 0x04000000, 0x08000000,
    0x10000000, 0x20000000, 0x40000000, 0x80000000,
    0x1B000000, 0x36000000
};

/* key schedule tables */
//#define GENERATE_INC
#ifdef GENERATE_INC

static int KT_init = 1;

static meUInt KT0[256];
static meUInt KT1[256];
static meUInt KT2[256];
static meUInt KT3[256];

#else

static meUInt KT0[256] = {
    0x00000000, 0x0e090d0b, 0x1c121a16, 0x121b171d, 0x3824342c, 0x362d3927, 0x24362e3a, 0x2a3f2331, 0x70486858, 0x7e416553, 0x6c5a724e, 0x62537f45, 0x486c5c74, 0x4665517f, 0x547e4662, 0x5a774b69,
    0xe090d0b0, 0xee99ddbb, 0xfc82caa6, 0xf28bc7ad, 0xd8b4e49c, 0xd6bde997, 0xc4a6fe8a, 0xcaaff381, 0x90d8b8e8, 0x9ed1b5e3, 0x8ccaa2fe, 0x82c3aff5, 0xa8fc8cc4, 0xa6f581cf, 0xb4ee96d2, 0xbae79bd9,
    0xdb3bbb7b, 0xd532b670, 0xc729a16d, 0xc920ac66, 0xe31f8f57, 0xed16825c, 0xff0d9541, 0xf104984a, 0xab73d323, 0xa57ade28, 0xb761c935, 0xb968c43e, 0x9357e70f, 0x9d5eea04, 0x8f45fd19, 0x814cf012,
    0x3bab6bcb, 0x35a266c0, 0x27b971dd, 0x29b07cd6, 0x038f5fe7, 0x0d8652ec, 0x1f9d45f1, 0x119448fa, 0x4be30393, 0x45ea0e98, 0x57f11985, 0x59f8148e, 0x73c737bf, 0x7dce3ab4, 0x6fd52da9, 0x61dc20a2,
    0xad766df6, 0xa37f60fd, 0xb16477e0, 0xbf6d7aeb, 0x955259da, 0x9b5b54d1, 0x894043cc, 0x87494ec7, 0xdd3e05ae, 0xd33708a5, 0xc12c1fb8, 0xcf2512b3, 0xe51a3182, 0xeb133c89, 0xf9082b94, 0xf701269f,
    0x4de6bd46, 0x43efb04d, 0x51f4a750, 0x5ffdaa5b, 0x75c2896a, 0x7bcb8461, 0x69d0937c, 0x67d99e77, 0x3daed51e, 0x33a7d815, 0x21bccf08, 0x2fb5c203, 0x058ae132, 0x0b83ec39, 0x1998fb24, 0x1791f62f,
    0x764dd68d, 0x7844db86, 0x6a5fcc9b, 0x6456c190, 0x4e69e2a1, 0x4060efaa, 0x527bf8b7, 0x5c72f5bc, 0x0605bed5, 0x080cb3de, 0x1a17a4c3, 0x141ea9c8, 0x3e218af9, 0x302887f2, 0x223390ef, 0x2c3a9de4,
    0x96dd063d, 0x98d40b36, 0x8acf1c2b, 0x84c61120, 0xaef93211, 0xa0f03f1a, 0xb2eb2807, 0xbce2250c, 0xe6956e65, 0xe89c636e, 0xfa877473, 0xf48e7978, 0xdeb15a49, 0xd0b85742, 0xc2a3405f, 0xccaa4d54,
    0x41ecdaf7, 0x4fe5d7fc, 0x5dfec0e1, 0x53f7cdea, 0x79c8eedb, 0x77c1e3d0, 0x65daf4cd, 0x6bd3f9c6, 0x31a4b2af, 0x3fadbfa4, 0x2db6a8b9, 0x23bfa5b2, 0x09808683, 0x07898b88, 0x15929c95, 0x1b9b919e,
    0xa17c0a47, 0xaf75074c, 0xbd6e1051, 0xb3671d5a, 0x99583e6b, 0x97513360, 0x854a247d, 0x8b432976, 0xd134621f, 0xdf3d6f14, 0xcd267809, 0xc32f7502, 0xe9105633, 0xe7195b38, 0xf5024c25, 0xfb0b412e,
    0x9ad7618c, 0x94de6c87, 0x86c57b9a, 0x88cc7691, 0xa2f355a0, 0xacfa58ab, 0xbee14fb6, 0xb0e842bd, 0xea9f09d4, 0xe49604df, 0xf68d13c2, 0xf8841ec9, 0xd2bb3df8, 0xdcb230f3, 0xcea927ee, 0xc0a02ae5,
    0x7a47b13c, 0x744ebc37, 0x6655ab2a, 0x685ca621, 0x42638510, 0x4c6a881b, 0x5e719f06, 0x5078920d, 0x0a0fd964, 0x0406d46f, 0x161dc372, 0x1814ce79, 0x322bed48, 0x3c22e043, 0x2e39f75e, 0x2030fa55,
    0xec9ab701, 0xe293ba0a, 0xf088ad17, 0xfe81a01c, 0xd4be832d, 0xdab78e26, 0xc8ac993b, 0xc6a59430, 0x9cd2df59, 0x92dbd252, 0x80c0c54f, 0x8ec9c844, 0xa4f6eb75, 0xaaffe67e, 0xb8e4f163, 0xb6edfc68,
    0x0c0a67b1, 0x02036aba, 0x10187da7, 0x1e1170ac, 0x342e539d, 0x3a275e96, 0x283c498b, 0x26354480, 0x7c420fe9, 0x724b02e2, 0x605015ff, 0x6e5918f4, 0x44663bc5, 0x4a6f36ce, 0x587421d3, 0x567d2cd8,
    0x37a10c7a, 0x39a80171, 0x2bb3166c, 0x25ba1b67, 0x0f853856, 0x018c355d, 0x13972240, 0x1d9e2f4b, 0x47e96422, 0x49e06929, 0x5bfb7e34, 0x55f2733f, 0x7fcd500e, 0x71c45d05, 0x63df4a18, 0x6dd64713,
    0xd731dcca, 0xd938d1c1, 0xcb23c6dc, 0xc52acbd7, 0xef15e8e6, 0xe11ce5ed, 0xf307f2f0, 0xfd0efffb, 0xa779b492, 0xa970b999, 0xbb6bae84, 0xb562a38f, 0x9f5d80be, 0x91548db5, 0x834f9aa8, 0x8d4697a3,
};
static meUInt KT1[256] = {
    0x00000000, 0x0b0e090d, 0x161c121a, 0x1d121b17, 0x2c382434, 0x27362d39, 0x3a24362e, 0x312a3f23, 0x58704868, 0x537e4165, 0x4e6c5a72, 0x4562537f, 0x74486c5c, 0x7f466551, 0x62547e46, 0x695a774b,
    0xb0e090d0, 0xbbee99dd, 0xa6fc82ca, 0xadf28bc7, 0x9cd8b4e4, 0x97d6bde9, 0x8ac4a6fe, 0x81caaff3, 0xe890d8b8, 0xe39ed1b5, 0xfe8ccaa2, 0xf582c3af, 0xc4a8fc8c, 0xcfa6f581, 0xd2b4ee96, 0xd9bae79b,
    0x7bdb3bbb, 0x70d532b6, 0x6dc729a1, 0x66c920ac, 0x57e31f8f, 0x5ced1682, 0x41ff0d95, 0x4af10498, 0x23ab73d3, 0x28a57ade, 0x35b761c9, 0x3eb968c4, 0x0f9357e7, 0x049d5eea, 0x198f45fd, 0x12814cf0,
    0xcb3bab6b, 0xc035a266, 0xdd27b971, 0xd629b07c, 0xe7038f5f, 0xec0d8652, 0xf11f9d45, 0xfa119448, 0x934be303, 0x9845ea0e, 0x8557f119, 0x8e59f814, 0xbf73c737, 0xb47dce3a, 0xa96fd52d, 0xa261dc20,
    0xf6ad766d, 0xfda37f60, 0xe0b16477, 0xebbf6d7a, 0xda955259, 0xd19b5b54, 0xcc894043, 0xc787494e, 0xaedd3e05, 0xa5d33708, 0xb8c12c1f, 0xb3cf2512, 0x82e51a31, 0x89eb133c, 0x94f9082b, 0x9ff70126,
    0x464de6bd, 0x4d43efb0, 0x5051f4a7, 0x5b5ffdaa, 0x6a75c289, 0x617bcb84, 0x7c69d093, 0x7767d99e, 0x1e3daed5, 0x1533a7d8, 0x0821bccf, 0x032fb5c2, 0x32058ae1, 0x390b83ec, 0x241998fb, 0x2f1791f6,
    0x8d764dd6, 0x867844db, 0x9b6a5fcc, 0x906456c1, 0xa14e69e2, 0xaa4060ef, 0xb7527bf8, 0xbc5c72f5, 0xd50605be, 0xde080cb3, 0xc31a17a4, 0xc8141ea9, 0xf93e218a, 0xf2302887, 0xef223390, 0xe42c3a9d,
    0x3d96dd06, 0x3698d40b, 0x2b8acf1c, 0x2084c611, 0x11aef932, 0x1aa0f03f, 0x07b2eb28, 0x0cbce225, 0x65e6956e, 0x6ee89c63, 0x73fa8774, 0x78f48e79, 0x49deb15a, 0x42d0b857, 0x5fc2a340, 0x54ccaa4d,
    0xf741ecda, 0xfc4fe5d7, 0xe15dfec0, 0xea53f7cd, 0xdb79c8ee, 0xd077c1e3, 0xcd65daf4, 0xc66bd3f9, 0xaf31a4b2, 0xa43fadbf, 0xb92db6a8, 0xb223bfa5, 0x83098086, 0x8807898b, 0x9515929c, 0x9e1b9b91,
    0x47a17c0a, 0x4caf7507, 0x51bd6e10, 0x5ab3671d, 0x6b99583e, 0x60975133, 0x7d854a24, 0x768b4329, 0x1fd13462, 0x14df3d6f, 0x09cd2678, 0x02c32f75, 0x33e91056, 0x38e7195b, 0x25f5024c, 0x2efb0b41,
    0x8c9ad761, 0x8794de6c, 0x9a86c57b, 0x9188cc76, 0xa0a2f355, 0xabacfa58, 0xb6bee14f, 0xbdb0e842, 0xd4ea9f09, 0xdfe49604, 0xc2f68d13, 0xc9f8841e, 0xf8d2bb3d, 0xf3dcb230, 0xeecea927, 0xe5c0a02a,
    0x3c7a47b1, 0x37744ebc, 0x2a6655ab, 0x21685ca6, 0x10426385, 0x1b4c6a88, 0x065e719f, 0x0d507892, 0x640a0fd9, 0x6f0406d4, 0x72161dc3, 0x791814ce, 0x48322bed, 0x433c22e0, 0x5e2e39f7, 0x552030fa,
    0x01ec9ab7, 0x0ae293ba, 0x17f088ad, 0x1cfe81a0, 0x2dd4be83, 0x26dab78e, 0x3bc8ac99, 0x30c6a594, 0x599cd2df, 0x5292dbd2, 0x4f80c0c5, 0x448ec9c8, 0x75a4f6eb, 0x7eaaffe6, 0x63b8e4f1, 0x68b6edfc,
    0xb10c0a67, 0xba02036a, 0xa710187d, 0xac1e1170, 0x9d342e53, 0x963a275e, 0x8b283c49, 0x80263544, 0xe97c420f, 0xe2724b02, 0xff605015, 0xf46e5918, 0xc544663b, 0xce4a6f36, 0xd3587421, 0xd8567d2c,
    0x7a37a10c, 0x7139a801, 0x6c2bb316, 0x6725ba1b, 0x560f8538, 0x5d018c35, 0x40139722, 0x4b1d9e2f, 0x2247e964, 0x2949e069, 0x345bfb7e, 0x3f55f273, 0x0e7fcd50, 0x0571c45d, 0x1863df4a, 0x136dd647,
    0xcad731dc, 0xc1d938d1, 0xdccb23c6, 0xd7c52acb, 0xe6ef15e8, 0xede11ce5, 0xf0f307f2, 0xfbfd0eff, 0x92a779b4, 0x99a970b9, 0x84bb6bae, 0x8fb562a3, 0xbe9f5d80, 0xb591548d, 0xa8834f9a, 0xa38d4697,
};
static meUInt KT2[256] = {
    0x00000000, 0x0d0b0e09, 0x1a161c12, 0x171d121b, 0x342c3824, 0x3927362d, 0x2e3a2436, 0x23312a3f, 0x68587048, 0x65537e41, 0x724e6c5a, 0x7f456253, 0x5c74486c, 0x517f4665, 0x4662547e, 0x4b695a77,
    0xd0b0e090, 0xddbbee99, 0xcaa6fc82, 0xc7adf28b, 0xe49cd8b4, 0xe997d6bd, 0xfe8ac4a6, 0xf381caaf, 0xb8e890d8, 0xb5e39ed1, 0xa2fe8cca, 0xaff582c3, 0x8cc4a8fc, 0x81cfa6f5, 0x96d2b4ee, 0x9bd9bae7,
    0xbb7bdb3b, 0xb670d532, 0xa16dc729, 0xac66c920, 0x8f57e31f, 0x825ced16, 0x9541ff0d, 0x984af104, 0xd323ab73, 0xde28a57a, 0xc935b761, 0xc43eb968, 0xe70f9357, 0xea049d5e, 0xfd198f45, 0xf012814c,
    0x6bcb3bab, 0x66c035a2, 0x71dd27b9, 0x7cd629b0, 0x5fe7038f, 0x52ec0d86, 0x45f11f9d, 0x48fa1194, 0x03934be3, 0x0e9845ea, 0x198557f1, 0x148e59f8, 0x37bf73c7, 0x3ab47dce, 0x2da96fd5, 0x20a261dc,
    0x6df6ad76, 0x60fda37f, 0x77e0b164, 0x7aebbf6d, 0x59da9552, 0x54d19b5b, 0x43cc8940, 0x4ec78749, 0x05aedd3e, 0x08a5d337, 0x1fb8c12c, 0x12b3cf25, 0x3182e51a, 0x3c89eb13, 0x2b94f908, 0x269ff701,
    0xbd464de6, 0xb04d43ef, 0xa75051f4, 0xaa5b5ffd, 0x896a75c2, 0x84617bcb, 0x937c69d0, 0x9e7767d9, 0xd51e3dae, 0xd81533a7, 0xcf0821bc, 0xc2032fb5, 0xe132058a, 0xec390b83, 0xfb241998, 0xf62f1791,
    0xd68d764d, 0xdb867844, 0xcc9b6a5f, 0xc1906456, 0xe2a14e69, 0xefaa4060, 0xf8b7527b, 0xf5bc5c72, 0xbed50605, 0xb3de080c, 0xa4c31a17, 0xa9c8141e, 0x8af93e21, 0x87f23028, 0x90ef2233, 0x9de42c3a,
    0x063d96dd, 0x0b3698d4, 0x1c2b8acf, 0x112084c6, 0x3211aef9, 0x3f1aa0f0, 0x2807b2eb, 0x250cbce2, 0x6e65e695, 0x636ee89c, 0x7473fa87, 0x7978f48e, 0x5a49deb1, 0x5742d0b8, 0x405fc2a3, 0x4d54ccaa,
    0xdaf741ec, 0xd7fc4fe5, 0xc0e15dfe, 0xcdea53f7, 0xeedb79c8, 0xe3d077c1, 0xf4cd65da, 0xf9c66bd3, 0xb2af31a4, 0xbfa43fad, 0xa8b92db6, 0xa5b223bf, 0x86830980, 0x8b880789, 0x9c951592, 0x919e1b9b,
    0x0a47a17c, 0x074caf75, 0x1051bd6e, 0x1d5ab367, 0x3e6b9958, 0x33609751, 0x247d854a, 0x29768b43, 0x621fd134, 0x6f14df3d, 0x7809cd26, 0x7502c32f, 0x5633e910, 0x5b38e719, 0x4c25f502, 0x412efb0b,
    0x618c9ad7, 0x6c8794de, 0x7b9a86c5, 0x769188cc, 0x55a0a2f3, 0x58abacfa, 0x4fb6bee1, 0x42bdb0e8, 0x09d4ea9f, 0x04dfe496, 0x13c2f68d, 0x1ec9f884, 0x3df8d2bb, 0x30f3dcb2, 0x27eecea9, 0x2ae5c0a0,
    0xb13c7a47, 0xbc37744e, 0xab2a6655, 0xa621685c, 0x85104263, 0x881b4c6a, 0x9f065e71, 0x920d5078, 0xd9640a0f, 0xd46f0406, 0xc372161d, 0xce791814, 0xed48322b, 0xe0433c22, 0xf75e2e39, 0xfa552030,
    0xb701ec9a, 0xba0ae293, 0xad17f088, 0xa01cfe81, 0x832dd4be, 0x8e26dab7, 0x993bc8ac, 0x9430c6a5, 0xdf599cd2, 0xd25292db, 0xc54f80c0, 0xc8448ec9, 0xeb75a4f6, 0xe67eaaff, 0xf163b8e4, 0xfc68b6ed,
    0x67b10c0a, 0x6aba0203, 0x7da71018, 0x70ac1e11, 0x539d342e, 0x5e963a27, 0x498b283c, 0x44802635, 0x0fe97c42, 0x02e2724b, 0x15ff6050, 0x18f46e59, 0x3bc54466, 0x36ce4a6f, 0x21d35874, 0x2cd8567d,
    0x0c7a37a1, 0x017139a8, 0x166c2bb3, 0x1b6725ba, 0x38560f85, 0x355d018c, 0x22401397, 0x2f4b1d9e, 0x642247e9, 0x692949e0, 0x7e345bfb, 0x733f55f2, 0x500e7fcd, 0x5d0571c4, 0x4a1863df, 0x47136dd6,
    0xdccad731, 0xd1c1d938, 0xc6dccb23, 0xcbd7c52a, 0xe8e6ef15, 0xe5ede11c, 0xf2f0f307, 0xfffbfd0e, 0xb492a779, 0xb999a970, 0xae84bb6b, 0xa38fb562, 0x80be9f5d, 0x8db59154, 0x9aa8834f, 0x97a38d46,
};
static meUInt KT3[256] = {
    0x00000000, 0x090d0b0e, 0x121a161c, 0x1b171d12, 0x24342c38, 0x2d392736, 0x362e3a24, 0x3f23312a, 0x48685870, 0x4165537e, 0x5a724e6c, 0x537f4562, 0x6c5c7448, 0x65517f46, 0x7e466254, 0x774b695a,
    0x90d0b0e0, 0x99ddbbee, 0x82caa6fc, 0x8bc7adf2, 0xb4e49cd8, 0xbde997d6, 0xa6fe8ac4, 0xaff381ca, 0xd8b8e890, 0xd1b5e39e, 0xcaa2fe8c, 0xc3aff582, 0xfc8cc4a8, 0xf581cfa6, 0xee96d2b4, 0xe79bd9ba,
    0x3bbb7bdb, 0x32b670d5, 0x29a16dc7, 0x20ac66c9, 0x1f8f57e3, 0x16825ced, 0x0d9541ff, 0x04984af1, 0x73d323ab, 0x7ade28a5, 0x61c935b7, 0x68c43eb9, 0x57e70f93, 0x5eea049d, 0x45fd198f, 0x4cf01281,
    0xab6bcb3b, 0xa266c035, 0xb971dd27, 0xb07cd629, 0x8f5fe703, 0x8652ec0d, 0x9d45f11f, 0x9448fa11, 0xe303934b, 0xea0e9845, 0xf1198557, 0xf8148e59, 0xc737bf73, 0xce3ab47d, 0xd52da96f, 0xdc20a261,
    0x766df6ad, 0x7f60fda3, 0x6477e0b1, 0x6d7aebbf, 0x5259da95, 0x5b54d19b, 0x4043cc89, 0x494ec787, 0x3e05aedd, 0x3708a5d3, 0x2c1fb8c1, 0x2512b3cf, 0x1a3182e5, 0x133c89eb, 0x082b94f9, 0x01269ff7,
    0xe6bd464d, 0xefb04d43, 0xf4a75051, 0xfdaa5b5f, 0xc2896a75, 0xcb84617b, 0xd0937c69, 0xd99e7767, 0xaed51e3d, 0xa7d81533, 0xbccf0821, 0xb5c2032f, 0x8ae13205, 0x83ec390b, 0x98fb2419, 0x91f62f17,
    0x4dd68d76, 0x44db8678, 0x5fcc9b6a, 0x56c19064, 0x69e2a14e, 0x60efaa40, 0x7bf8b752, 0x72f5bc5c, 0x05bed506, 0x0cb3de08, 0x17a4c31a, 0x1ea9c814, 0x218af93e, 0x2887f230, 0x3390ef22, 0x3a9de42c,
    0xdd063d96, 0xd40b3698, 0xcf1c2b8a, 0xc6112084, 0xf93211ae, 0xf03f1aa0, 0xeb2807b2, 0xe2250cbc, 0x956e65e6, 0x9c636ee8, 0x877473fa, 0x8e7978f4, 0xb15a49de, 0xb85742d0, 0xa3405fc2, 0xaa4d54cc,
    0xecdaf741, 0xe5d7fc4f, 0xfec0e15d, 0xf7cdea53, 0xc8eedb79, 0xc1e3d077, 0xdaf4cd65, 0xd3f9c66b, 0xa4b2af31, 0xadbfa43f, 0xb6a8b92d, 0xbfa5b223, 0x80868309, 0x898b8807, 0x929c9515, 0x9b919e1b,
    0x7c0a47a1, 0x75074caf, 0x6e1051bd, 0x671d5ab3, 0x583e6b99, 0x51336097, 0x4a247d85, 0x4329768b, 0x34621fd1, 0x3d6f14df, 0x267809cd, 0x2f7502c3, 0x105633e9, 0x195b38e7, 0x024c25f5, 0x0b412efb,
    0xd7618c9a, 0xde6c8794, 0xc57b9a86, 0xcc769188, 0xf355a0a2, 0xfa58abac, 0xe14fb6be, 0xe842bdb0, 0x9f09d4ea, 0x9604dfe4, 0x8d13c2f6, 0x841ec9f8, 0xbb3df8d2, 0xb230f3dc, 0xa927eece, 0xa02ae5c0,
    0x47b13c7a, 0x4ebc3774, 0x55ab2a66, 0x5ca62168, 0x63851042, 0x6a881b4c, 0x719f065e, 0x78920d50, 0x0fd9640a, 0x06d46f04, 0x1dc37216, 0x14ce7918, 0x2bed4832, 0x22e0433c, 0x39f75e2e, 0x30fa5520,
    0x9ab701ec, 0x93ba0ae2, 0x88ad17f0, 0x81a01cfe, 0xbe832dd4, 0xb78e26da, 0xac993bc8, 0xa59430c6, 0xd2df599c, 0xdbd25292, 0xc0c54f80, 0xc9c8448e, 0xf6eb75a4, 0xffe67eaa, 0xe4f163b8, 0xedfc68b6,
    0x0a67b10c, 0x036aba02, 0x187da710, 0x1170ac1e, 0x2e539d34, 0x275e963a, 0x3c498b28, 0x35448026, 0x420fe97c, 0x4b02e272, 0x5015ff60, 0x5918f46e, 0x663bc544, 0x6f36ce4a, 0x7421d358, 0x7d2cd856,
    0xa10c7a37, 0xa8017139, 0xb3166c2b, 0xba1b6725, 0x8538560f, 0x8c355d01, 0x97224013, 0x9e2f4b1d, 0xe9642247, 0xe0692949, 0xfb7e345b, 0xf2733f55, 0xcd500e7f, 0xc45d0571, 0xdf4a1863, 0xd647136d,
    0x31dccad7, 0x38d1c1d9, 0x23c6dccb, 0x2acbd7c5, 0x15e8e6ef, 0x1ce5ede1, 0x07f2f0f3, 0x0efffbfd, 0x79b492a7, 0x70b999a9, 0x6bae84bb, 0x62a38fb5, 0x5d80be9f, 0x548db591, 0x4f9aa883, 0x4697a38d,
};

#endif

/* platform-independant 32-bit integer manipulation macros */

#define GET_UINT32(n,b,i)                       \
{                                               \
    (n) = ( (meUInt) (b)[(i)    ] << 24 )       \
        | ( (meUInt) (b)[(i) + 1] << 16 )       \
        | ( (meUInt) (b)[(i) + 2] <<  8 )       \
        | ( (meUInt) (b)[(i) + 3]       );      \
}

#define PUT_UINT32(n,b,i)                       \
{                                               \
    (b)[(i)    ] = (meUByte) ( (n) >> 24 );       \
    (b)[(i) + 1] = (meUByte) ( (n) >> 16 );       \
    (b)[(i) + 2] = (meUByte) ( (n) >>  8 );       \
    (b)[(i) + 3] = (meUByte) ( (n)       );       \
}

/* AES key scheduling routine */

int aes_set_key(meCryptContext *ctx, meUByte *key, int nbits )
{
    int i;
    meUInt *RK, *SK;

    switch( nbits )
    {
        case 128: ctx->nr = 10; break;
        case 192: ctx->nr = 12; break;
        case 256: ctx->nr = 14; break;
        default : return( 1 );
    }

    RK = ctx->erk;

    for( i = 0; i < (nbits >> 5); i++ )
    {
        GET_UINT32( RK[i], key, i * 4 );
    }

    /* setup encryption round keys */

    switch( nbits )
    {
    case 128:

        for( i = 0; i < 10; i++, RK += 4 )
        {
            RK[4]  = RK[0] ^ RCON[i] ^
                        ( FSb[ (meUByte) ( RK[3] >> 16 ) ] << 24 ) ^
                        ( FSb[ (meUByte) ( RK[3] >>  8 ) ] << 16 ) ^
                        ( FSb[ (meUByte) ( RK[3]       ) ] <<  8 ) ^
                        ( FSb[ (meUByte) ( RK[3] >> 24 ) ]       );

            RK[5]  = RK[1] ^ RK[4];
            RK[6]  = RK[2] ^ RK[5];
            RK[7]  = RK[3] ^ RK[6];
        }
        break;

    case 192:

        for( i = 0; i < 8; i++, RK += 6 )
        {
            RK[6]  = RK[0] ^ RCON[i] ^
                        ( FSb[ (meUByte) ( RK[5] >> 16 ) ] << 24 ) ^
                        ( FSb[ (meUByte) ( RK[5] >>  8 ) ] << 16 ) ^
                        ( FSb[ (meUByte) ( RK[5]       ) ] <<  8 ) ^
                        ( FSb[ (meUByte) ( RK[5] >> 24 ) ]       );

            RK[7]  = RK[1] ^ RK[6];
            RK[8]  = RK[2] ^ RK[7];
            RK[9]  = RK[3] ^ RK[8];
            RK[10] = RK[4] ^ RK[9];
            RK[11] = RK[5] ^ RK[10];
        }
        break;

    case 256:

        for( i = 0; i < 7; i++, RK += 8 )
        {
            RK[8]  = RK[0] ^ RCON[i] ^
                        ( FSb[ (meUByte) ( RK[7] >> 16 ) ] << 24 ) ^
                        ( FSb[ (meUByte) ( RK[7] >>  8 ) ] << 16 ) ^
                        ( FSb[ (meUByte) ( RK[7]       ) ] <<  8 ) ^
                        ( FSb[ (meUByte) ( RK[7] >> 24 ) ]       );

            RK[9]  = RK[1] ^ RK[8];
            RK[10] = RK[2] ^ RK[9];
            RK[11] = RK[3] ^ RK[10];

            RK[12] = RK[4] ^
                        ( FSb[ (meUByte) ( RK[11] >> 24 ) ] << 24 ) ^
                        ( FSb[ (meUByte) ( RK[11] >> 16 ) ] << 16 ) ^
                        ( FSb[ (meUByte) ( RK[11] >>  8 ) ] <<  8 ) ^
                        ( FSb[ (meUByte) ( RK[11]       ) ]       );

            RK[13] = RK[5] ^ RK[12];
            RK[14] = RK[6] ^ RK[13];
            RK[15] = RK[7] ^ RK[14];
        }
        break;
    }

#ifdef GENERATE_INC
    /* setup decryption round keys */
    
    if( KT_init )
    {
        for( i = 0; i < 256; i++ )
        {
            KT0[i] = RT0[ FSb[i] ];
            KT1[i] = RT1[ FSb[i] ];
            KT2[i] = RT2[ FSb[i] ];
            KT3[i] = RT3[ FSb[i] ];
        }
        
        FILE *fp;
        int j;
        
        fp = fopen("aes_kt.h","w");
        for( j = 0; j < 4 ; j++ )
        {
            fprintf(fp,"static meUInt KT%d[256] = {",j);
            for(i = 0; i < 256; i++ )
            {
                if((i & 0xf) == 0)
                    fprintf(fp,"\n   ");
                if(j & 0x01)
                    fprintf(fp," 0x%08x,",(j & 0x02) ? RT3[ FSb[i] ]:RT1[ FSb[i] ]);
                else
                    fprintf(fp," 0x%08x,",(j & 0x02) ? RT2[ FSb[i] ]:RT0[ FSb[i] ]);
            }
            fprintf(fp,"\n};\n");
        }
        fclose(fp);
        KT_init = 0;
    }
#endif
    SK = ctx->drk;

    *SK++ = *RK++;
    *SK++ = *RK++;
    *SK++ = *RK++;
    *SK++ = *RK++;

    for( i = 1; i < ctx->nr; i++ )
    {
        RK -= 8;

        *SK++ = KT0[ (meUByte) ( *RK >> 24 ) ] ^
                KT1[ (meUByte) ( *RK >> 16 ) ] ^
                KT2[ (meUByte) ( *RK >>  8 ) ] ^
                KT3[ (meUByte) ( *RK       ) ]; RK++;

        *SK++ = KT0[ (meUByte) ( *RK >> 24 ) ] ^
                KT1[ (meUByte) ( *RK >> 16 ) ] ^
                KT2[ (meUByte) ( *RK >>  8 ) ] ^
                KT3[ (meUByte) ( *RK       ) ]; RK++;

        *SK++ = KT0[ (meUByte) ( *RK >> 24 ) ] ^
                KT1[ (meUByte) ( *RK >> 16 ) ] ^
                KT2[ (meUByte) ( *RK >>  8 ) ] ^
                KT3[ (meUByte) ( *RK       ) ]; RK++;

        *SK++ = KT0[ (meUByte) ( *RK >> 24 ) ] ^
                KT1[ (meUByte) ( *RK >> 16 ) ] ^
                KT2[ (meUByte) ( *RK >>  8 ) ] ^
                KT3[ (meUByte) ( *RK       ) ]; RK++;
    }

    RK -= 8;

    *SK++ = *RK++;
    *SK++ = *RK++;
    *SK++ = *RK++;
    *SK++ = *RK++;

    return( 0 );
}

/* AES 128-bit block encryption routine */

void aes_encrypt( meCryptContext *ctx, meUByte input[16], meUByte output[16] )
{
    meUInt *RK, X0, X1, X2, X3, Y0, Y1, Y2, Y3;

    RK = ctx->erk;

    GET_UINT32( X0, input,  0 ); X0 ^= RK[0];
    GET_UINT32( X1, input,  4 ); X1 ^= RK[1];
    GET_UINT32( X2, input,  8 ); X2 ^= RK[2];
    GET_UINT32( X3, input, 12 ); X3 ^= RK[3];

#define AES_FROUND(X0,X1,X2,X3,Y0,Y1,Y2,Y3)     \
{                                               \
    RK += 4;                                    \
                                                \
    X0 = RK[0] ^ FT0[ (meUByte) ( Y0 >> 24 ) ] ^  \
                 FT1[ (meUByte) ( Y1 >> 16 ) ] ^  \
                 FT2[ (meUByte) ( Y2 >>  8 ) ] ^  \
                 FT3[ (meUByte) ( Y3       ) ];   \
                                                \
    X1 = RK[1] ^ FT0[ (meUByte) ( Y1 >> 24 ) ] ^  \
                 FT1[ (meUByte) ( Y2 >> 16 ) ] ^  \
                 FT2[ (meUByte) ( Y3 >>  8 ) ] ^  \
                 FT3[ (meUByte) ( Y0       ) ];   \
                                                \
    X2 = RK[2] ^ FT0[ (meUByte) ( Y2 >> 24 ) ] ^  \
                 FT1[ (meUByte) ( Y3 >> 16 ) ] ^  \
                 FT2[ (meUByte) ( Y0 >>  8 ) ] ^  \
                 FT3[ (meUByte) ( Y1       ) ];   \
                                                \
    X3 = RK[3] ^ FT0[ (meUByte) ( Y3 >> 24 ) ] ^  \
                 FT1[ (meUByte) ( Y0 >> 16 ) ] ^  \
                 FT2[ (meUByte) ( Y1 >>  8 ) ] ^  \
                 FT3[ (meUByte) ( Y2       ) ];   \
}

    AES_FROUND( Y0, Y1, Y2, Y3, X0, X1, X2, X3 );       /* round 1 */
    AES_FROUND( X0, X1, X2, X3, Y0, Y1, Y2, Y3 );       /* round 2 */
    AES_FROUND( Y0, Y1, Y2, Y3, X0, X1, X2, X3 );       /* round 3 */
    AES_FROUND( X0, X1, X2, X3, Y0, Y1, Y2, Y3 );       /* round 4 */
    AES_FROUND( Y0, Y1, Y2, Y3, X0, X1, X2, X3 );       /* round 5 */
    AES_FROUND( X0, X1, X2, X3, Y0, Y1, Y2, Y3 );       /* round 6 */
    AES_FROUND( Y0, Y1, Y2, Y3, X0, X1, X2, X3 );       /* round 7 */
    AES_FROUND( X0, X1, X2, X3, Y0, Y1, Y2, Y3 );       /* round 8 */
    AES_FROUND( Y0, Y1, Y2, Y3, X0, X1, X2, X3 );       /* round 9 */

    if( ctx->nr > 10 )
    {
        AES_FROUND( X0, X1, X2, X3, Y0, Y1, Y2, Y3 );   /* round 10 */
        AES_FROUND( Y0, Y1, Y2, Y3, X0, X1, X2, X3 );   /* round 11 */
    }

    if( ctx->nr > 12 )
    {
        AES_FROUND( X0, X1, X2, X3, Y0, Y1, Y2, Y3 );   /* round 12 */
        AES_FROUND( Y0, Y1, Y2, Y3, X0, X1, X2, X3 );   /* round 13 */
    }

    /* last round */

    RK += 4;

    X0 = RK[0] ^ ( FSb[ (meUByte) ( Y0 >> 24 ) ] << 24 ) ^
                 ( FSb[ (meUByte) ( Y1 >> 16 ) ] << 16 ) ^
                 ( FSb[ (meUByte) ( Y2 >>  8 ) ] <<  8 ) ^
                 ( FSb[ (meUByte) ( Y3       ) ]       );

    X1 = RK[1] ^ ( FSb[ (meUByte) ( Y1 >> 24 ) ] << 24 ) ^
                 ( FSb[ (meUByte) ( Y2 >> 16 ) ] << 16 ) ^
                 ( FSb[ (meUByte) ( Y3 >>  8 ) ] <<  8 ) ^
                 ( FSb[ (meUByte) ( Y0       ) ]       );

    X2 = RK[2] ^ ( FSb[ (meUByte) ( Y2 >> 24 ) ] << 24 ) ^
                 ( FSb[ (meUByte) ( Y3 >> 16 ) ] << 16 ) ^
                 ( FSb[ (meUByte) ( Y0 >>  8 ) ] <<  8 ) ^
                 ( FSb[ (meUByte) ( Y1       ) ]       );

    X3 = RK[3] ^ ( FSb[ (meUByte) ( Y3 >> 24 ) ] << 24 ) ^
                 ( FSb[ (meUByte) ( Y0 >> 16 ) ] << 16 ) ^
                 ( FSb[ (meUByte) ( Y1 >>  8 ) ] <<  8 ) ^
                 ( FSb[ (meUByte) ( Y2       ) ]       );

    PUT_UINT32( X0, output,  0 );
    PUT_UINT32( X1, output,  4 );
    PUT_UINT32( X2, output,  8 );
    PUT_UINT32( X3, output, 12 );
}

/* AES 128-bit block decryption routine */

void aes_decrypt( meCryptContext *ctx, meUByte input[16], meUByte output[16] )
{
    meUInt *RK, X0, X1, X2, X3, Y0, Y1, Y2, Y3;

    RK = ctx->drk;

    GET_UINT32( X0, input,  0 ); X0 ^= RK[0];
    GET_UINT32( X1, input,  4 ); X1 ^= RK[1];
    GET_UINT32( X2, input,  8 ); X2 ^= RK[2];
    GET_UINT32( X3, input, 12 ); X3 ^= RK[3];

#define AES_RROUND(X0,X1,X2,X3,Y0,Y1,Y2,Y3)     \
{                                               \
    RK += 4;                                    \
                                                \
    X0 = RK[0] ^ RT0[ (meUByte) ( Y0 >> 24 ) ] ^  \
                 RT1[ (meUByte) ( Y3 >> 16 ) ] ^  \
                 RT2[ (meUByte) ( Y2 >>  8 ) ] ^  \
                 RT3[ (meUByte) ( Y1       ) ];   \
                                                \
    X1 = RK[1] ^ RT0[ (meUByte) ( Y1 >> 24 ) ] ^  \
                 RT1[ (meUByte) ( Y0 >> 16 ) ] ^  \
                 RT2[ (meUByte) ( Y3 >>  8 ) ] ^  \
                 RT3[ (meUByte) ( Y2       ) ];   \
                                                \
    X2 = RK[2] ^ RT0[ (meUByte) ( Y2 >> 24 ) ] ^  \
                 RT1[ (meUByte) ( Y1 >> 16 ) ] ^  \
                 RT2[ (meUByte) ( Y0 >>  8 ) ] ^  \
                 RT3[ (meUByte) ( Y3       ) ];   \
                                                \
    X3 = RK[3] ^ RT0[ (meUByte) ( Y3 >> 24 ) ] ^  \
                 RT1[ (meUByte) ( Y2 >> 16 ) ] ^  \
                 RT2[ (meUByte) ( Y1 >>  8 ) ] ^  \
                 RT3[ (meUByte) ( Y0       ) ];   \
}

    AES_RROUND( Y0, Y1, Y2, Y3, X0, X1, X2, X3 );       /* round 1 */
    AES_RROUND( X0, X1, X2, X3, Y0, Y1, Y2, Y3 );       /* round 2 */
    AES_RROUND( Y0, Y1, Y2, Y3, X0, X1, X2, X3 );       /* round 3 */
    AES_RROUND( X0, X1, X2, X3, Y0, Y1, Y2, Y3 );       /* round 4 */
    AES_RROUND( Y0, Y1, Y2, Y3, X0, X1, X2, X3 );       /* round 5 */
    AES_RROUND( X0, X1, X2, X3, Y0, Y1, Y2, Y3 );       /* round 6 */
    AES_RROUND( Y0, Y1, Y2, Y3, X0, X1, X2, X3 );       /* round 7 */
    AES_RROUND( X0, X1, X2, X3, Y0, Y1, Y2, Y3 );       /* round 8 */
    AES_RROUND( Y0, Y1, Y2, Y3, X0, X1, X2, X3 );       /* round 9 */

    if( ctx->nr > 10 )
    {
        AES_RROUND( X0, X1, X2, X3, Y0, Y1, Y2, Y3 );   /* round 10 */
        AES_RROUND( Y0, Y1, Y2, Y3, X0, X1, X2, X3 );   /* round 11 */
    }

    if( ctx->nr > 12 )
    {
        AES_RROUND( X0, X1, X2, X3, Y0, Y1, Y2, Y3 );   /* round 12 */
        AES_RROUND( Y0, Y1, Y2, Y3, X0, X1, X2, X3 );   /* round 13 */
    }

    /* last round */

    RK += 4;

    X0 = RK[0] ^ ( RSb[ (meUByte) ( Y0 >> 24 ) ] << 24 ) ^
                 ( RSb[ (meUByte) ( Y3 >> 16 ) ] << 16 ) ^
                 ( RSb[ (meUByte) ( Y2 >>  8 ) ] <<  8 ) ^
                 ( RSb[ (meUByte) ( Y1       ) ]       );

    X1 = RK[1] ^ ( RSb[ (meUByte) ( Y1 >> 24 ) ] << 24 ) ^
                 ( RSb[ (meUByte) ( Y0 >> 16 ) ] << 16 ) ^
                 ( RSb[ (meUByte) ( Y3 >>  8 ) ] <<  8 ) ^
                 ( RSb[ (meUByte) ( Y2       ) ]       );

    X2 = RK[2] ^ ( RSb[ (meUByte) ( Y2 >> 24 ) ] << 24 ) ^
                 ( RSb[ (meUByte) ( Y1 >> 16 ) ] << 16 ) ^
                 ( RSb[ (meUByte) ( Y0 >>  8 ) ] <<  8 ) ^
                 ( RSb[ (meUByte) ( Y3       ) ]       );

    X3 = RK[3] ^ ( RSb[ (meUByte) ( Y3 >> 24 ) ] << 24 ) ^
                 ( RSb[ (meUByte) ( Y2 >> 16 ) ] << 16 ) ^
                 ( RSb[ (meUByte) ( Y1 >>  8 ) ] <<  8 ) ^
                 ( RSb[ (meUByte) ( Y0       ) ]       );

    PUT_UINT32( X0, output,  0 );
    PUT_UINT32( X1, output,  4 );
    PUT_UINT32( X2, output,  8 );
    PUT_UINT32( X3, output, 12 );
}

#ifdef TEST

#include <string.h>
#include <stdio.h>

/*
 * Rijndael Monte Carlo Test: ECB mode
 * source: NIST - rijndael-vals.zip
 */

static meUByte AES_enc_test[3][16] =
{
    { 0xA0, 0x43, 0x77, 0xAB, 0xE2, 0x59, 0xB0, 0xD0, 0xB5, 0xBA, 0x2D, 0x40, 0xA5, 0x01, 0x97, 0x1B },
    { 0x4E, 0x46, 0xF8, 0xC5, 0x09, 0x2B, 0x29, 0xE2, 0x9A, 0x97, 0x1A, 0x0C, 0xD1, 0xF6, 0x10, 0xFB },
    { 0x1F, 0x67, 0x63, 0xDF, 0x80, 0x7A, 0x7E, 0x70, 0x96, 0x0D, 0x4C, 0xD3, 0x11, 0x8E, 0x60, 0x1A }
};
    
static meUByte AES_dec_test[3][16] =
{
    { 0xF5, 0xBF, 0x8B, 0x37, 0x13, 0x6F, 0x2E, 0x1F, 0x6B, 0xEC, 0x6F, 0x57, 0x20, 0x21, 0xE3, 0xBA },
    { 0xF1, 0xA8, 0x1B, 0x68, 0xF6, 0xE5, 0xA6, 0x27, 0x1A, 0x8C, 0xB2, 0x4E, 0x7D, 0x94, 0x91, 0xEF },
    { 0x4D, 0xE0, 0xC6, 0xDF, 0x7C, 0xB1, 0x69, 0x72, 0x84, 0x60, 0x4D, 0x60, 0x27, 0x1B, 0xC5, 0x9A }
};
    
int main(int argc, char *argv[])
{
    int m, n, i, j;
    meCryptContext ctx;
    meUByte buf[48];
    meUByte key[32];
    
    if(argc == 3)
    {
        memset(key,0,32);
        strcpy((char *) key,argv[1]);
        aes_set_key(&ctx,key,32 * 8);
        memset(buf,0,32);
        strncpy((char *) buf,argv[2],16);
        aes_encrypt(&ctx,buf,buf+16);
        
        printf("Password (%s)\nEncrypt (%s) : ",argv[1],argv[2]);
        for(i = 16 ; i < 32; i++ )
            printf("%02x",buf[i]);
        aes_set_key(&ctx,key,32 * 8);
        aes_decrypt(&ctx,buf+16,buf+32);
        printf("\nDecrypt (%s) : %s\n",buf+32,(strcmp(argv[2],(char *) (buf+32))) ? "fail":"pass");
        return 0;
    }
    
    for( m = 0; m < 2; m++ )
    {
        printf( "\n Rijndael Monte Carlo Test (ECB mode) - " );

        if( m == 0 ) printf( "encryption\n\n" );
        if( m == 1 ) printf( "decryption\n\n" );

        for( n = 0; n < 3; n++ )
        {
            printf( " Test %d, key size = %3d bits: ",
                    n + 1, 128 + n * 64 );

            fflush( stdout );

            memset( buf, 0, 16 );
            memset( key, 0, 16 + n * 8 );

            for( i = 0; i < 400; i++ )
            {
                aes_set_key( &ctx, key, 128 + n * 64 );

                for( j = 0; j < 9999; j++ )
                {
                    if( m == 0 ) aes_encrypt( &ctx, buf, buf );
                    if( m == 1 ) aes_decrypt( &ctx, buf, buf );
                }

                if( n > 0 )
                {
                    for( j = 0; j < (n << 3); j++ )
                    {
                        key[j] ^= buf[j + 16 - (n << 3)];
                    }
                }

                if( m == 0 ) aes_encrypt( &ctx, buf, buf );
                if( m == 1 ) aes_decrypt( &ctx, buf, buf );

                for( j = 0; j < 16; j++ )
                {
                    key[j + (n << 3)] ^= buf[j];
                }
            }

            if( ( m == 0 && memcmp( buf, AES_enc_test[n], 16 ) ) ||
                ( m == 1 && memcmp( buf, AES_dec_test[n], 16 ) ) )
            {
                printf( "failed!\n" );
                return( 1 );
            }

            printf( "passed.\n" );
        }
    }

    printf( "\n" );

    return( 0 );
}

#endif

/* enc must be a 32 bytes long, key can be any length */
void
meCryptKeyEncode(meUInt *enc,meUByte *key, int len)
{
    meUInt rndSs[4], ki[meSBUF_SIZE_MAX>>2];
    meUByte *dd=(meUByte *) ki;
    int ii;
    if(len > meSBUF_SIZE_MAX)
        len = meSBUF_SIZE_MAX;
    ii = ((len-1)|0x03)+1;
    memcpy(dd,key,len);
    while(len < ii)
        dd[len++] = '\x01';
    len >>= 2;
    for(ii=0 ; ii<len ; ii++)
        ki[ii] = (ki[ii] >> (ii+1)) | (ki[ii] << (31-ii));
        
    if(len < meCRYPT_KEY_SIZE)
    {
        for(ii=len-1 ; len<meCRYPT_KEY_SIZE ; ii++,len++)
            ki[len] = (ki[ii] >> len) | (ki[ii] << (32-len));
    }
    else if(len > meCRYPT_KEY_SIZE)
    {
        ii = 0;
        while(--len >= meCRYPT_KEY_SIZE)
        {
            ki[ii] ^= (ki[len] >> (ii+1)) | (ki[len] << (31-ii));
            ii++;
            if(ii & meCRYPT_KEY_SIZE)
                ii = 0;
        }
    }
    memcpy(rndSs,meRndSeed,4*4);
    memcpy(meRndSeed,ki,4*4);
    ii = 7;
    do {
        enc[ii] = ki[ii] ^ meXoshiro128Next();
    } while(--ii >= 0);
    memcpy(meRndSeed,rndSs,4*4);
}

int
meCryptBufferSetKey(meBuffer *bp, meUByte *key)
{
    meUByte buf[meBUF_SIZE_MAX];
    meUByte keybuf[meSBUF_SIZE_MAX]; /* new encryption string */
    int ll;
    
    if(key == NULL)
    {
	/* get the string to use as an encrytion string */
        meStrcpy(buf,(bp->fileName != NULL) ? bp->fileName:bp->name);
        meStrcat(buf," password");
        if(meGetString(buf,MLNOHIST|MLHIDEVAL,0,keybuf,meSBUF_SIZE_MAX) <= 0)
            return meFALSE;
        mlerase(MWCLEXEC);		/* clear it off the bottom line */
        key = keybuf;
    }
    if((ll = meStrlen(key)) == 0)
    {
        if(meModeTest(bp->mode,MDCRYPT))
        {
            meModeClear(bp->mode,MDCRYPT);
            if(!meModeTest(bp->mode,MDNACT))
                meModeSet(bp->mode,MDEDIT);
        }
        if(bp->cryptKey != NULL)
        {
            meFree(bp->cryptKey);
            bp->cryptKey = NULL;
        }
    }
    else if((bp->cryptKey == NULL) && ((bp->cryptKey = (meUInt *) meMalloc(sizeof(meUInt)*meCRYPT_KEY_SIZE)) == NULL))
        return meFALSE;
    else
    {
        meModeSet(bp->mode,MDCRYPT);
        if(!meModeTest(bp->mode,MDNACT))
            meModeSet(bp->mode,MDEDIT);
        /* and protect it */
        meCryptKeyEncode(bp->cryptKey,key,ll);
    }
    meBufferAddModeToWindows(bp,WFMODE);
    return meTRUE;
}

int
setCryptKey(int f, int n)	/* reset encryption key of current buffer */
{
    return meCryptBufferSetKey(frameCur->windowCur->buffer,NULL);
}

void
meCryptInit(meUInt *key)
{
    int ii;
    /* return can only be error if nbits is not 128,192 or 256 */
    meAssert((meCRYPT_KEY_SIZE*4*8) == 256);
    aes_set_key(&cryCtx,(meUByte *) key,meCRYPT_KEY_SIZE*4*8);
    if(meRndSeed[0] == 0)
        meXoshiro128Seed();
    ii = 7;
    do {
        cryCtx.blk[ii] ^= meXoshiro128Next();
    } while(--ii >= 0);
}

int
meCryptBufferInit(meBuffer *bp)
{
    /* if we are in crypt mode */
    if(meModeTest(bp->mode,MDCRYPT))
    {
        if((bp->cryptKey == NULL) && (meCryptBufferSetKey(bp,NULL) <= 0))
            return meFALSE;
        /* check crypt is still enabled (user may have removed the key) and set up the key to be used! */
        if(meModeTest(bp->mode,MDCRYPT))
            meCryptInit(bp->cryptKey);
    }
    return meTRUE;
}

void
meEncrypt(register meUByte *bptr, register meUInt len)
{
    register meUInt *bf1, *bf2;
    register int ii;
    meAssert(meCRYPT_BLK_SIZE == 16);
    meAssert((len & (meCRYPT_BLK_SIZE-1)) == 0);
    meAssert((((size_t) bptr) & 3) == 0);
    
    bf1 = cryCtx.blk;
    bf2 = (meUInt *) bptr;
    *bf2++ ^= *bf1++;
    *bf2++ ^= *bf1++;
    *bf2++ ^= *bf1++;
    *bf2++ ^= *bf1;
    bf1 = (meUInt *) bptr;
    ii = len >> 4;
    for(;;)
    {
        aes_encrypt(&cryCtx,(meUByte *) bf1,(meUByte *) bf1);
        if(--ii <= 0)
            break;
        *bf2++ ^= *bf1++;
        *bf2++ ^= *bf1++;
        *bf2++ ^= *bf1++;
        *bf2++ ^= *bf1++;
    }
    memcpy(cryCtx.blk,bf1,meCRYPT_BLK_SIZE);
}
void
meDecrypt(register meUByte *bptr, register meUInt len)
{
    /* when decrypting the data must be loaded with a 2*meCRYPT_BLK_SIZE offset into the buffer, so bptr must be at least len+2*meCRYPT_BLK_SIZE long */ 
    register meUInt *bf1, *bf2;
    register int ii;
    meAssert(meCRYPT_BLK_SIZE == 16);
    meAssert((len & (meCRYPT_BLK_SIZE-1)) == 0);
    meAssert((((size_t) bptr) & 3) == 0);
    bf2 = (meUInt *) (bptr+meCRYPT_BLK_SIZE);
    memcpy(bf2,cryCtx.blk,meCRYPT_BLK_SIZE);
    bf1 = (meUInt *) bptr;
    ii = len >> 4;
    do {
        aes_decrypt(&cryCtx,(meUByte *) (bf2+4),(meUByte *) bf1);
        *bf1++ ^= *bf2++;
        *bf1++ ^= *bf2++;
        *bf1++ ^= *bf2++;
        *bf1++ ^= *bf2++;
    } while(--ii > 0);
    memcpy(cryCtx.blk,bf2,meCRYPT_BLK_SIZE);
}

#endif
