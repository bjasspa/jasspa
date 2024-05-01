/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * spell.c - Spell checking routines.
 *
 * Copyright (C) 2023-2024 JASSPA (www.jasspa.com)
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

#include "emain.h"

#if MEOPT_EXTENDED

/*********************************************************************
 * The following code is based on sha256.c:
 * 
 * Filename:   sha256.c
 * Author:     Brad Conte (brad AT bradconte.com)
 * Copyright:
 * Disclaimer: This code is presented "as is" without any guarantees.
 * Details:    Implementation of the SHA-256 hashing algorithm.
 *             SHA-256 is one of the three algorithms in the SHA2 specification.
 *             The others, SHA-384 and SHA-512, are not offered in this implementation.
 *             Algorithm specification can be found here:
 *                  http://csrc.nist.gov/publications/fips/fips180-2/fips180-2withchangenotice.pdf
 *             This implementation uses little endian byte order.
 * 
 *********************************************************************/
#define meHASH_OVERRUN   0x01
#define meHASH_FINALISED 0x02
#define meHASH_SHA1      0x10

typedef struct {
    meULong bitlen;
    meUInt state[8];
    meUByte flag;
    meUByte datalen;
    meUByte data[64];
} meHash;

#include <stdlib.h>
#include <memory.h>

#define ROTLEFT(a,b) (((a) << (b)) | ((a) >> (32-(b))))
#define ROTRIGHT(a,b) (((a) >> (b)) | ((a) << (32-(b))))

#define CH(x,y,z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROTRIGHT(x,2) ^ ROTRIGHT(x,13) ^ ROTRIGHT(x,22))
#define EP1(x) (ROTRIGHT(x,6) ^ ROTRIGHT(x,11) ^ ROTRIGHT(x,25))
#define SIG0(x) (ROTRIGHT(x,7) ^ ROTRIGHT(x,18) ^ ((x) >> 3))
#define SIG1(x) (ROTRIGHT(x,17) ^ ROTRIGHT(x,19) ^ ((x) >> 10))

static const meUInt k1[4] = {
    0x5a827999,0x6ed9eba1,0x8f1bbcdc,0xca62c1d6
};
static const meUInt k2[64] = {
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

void
meHashTransform(meHash *ctx,const meUByte data[])
{
    meUInt a, b, c, d, e, f, g, h, i, j, t1, t2, m[80];
    
    for(i=0,j=0 ; i < 16 ; i++,j+=4)
        m[i] = (data[j] << 24) | (data[j + 1] << 16) | (data[j + 2] << 8) | (data[j + 3]);
    if(ctx->flag & meHASH_SHA1)
    {
        for( ; i < 80; i++)
        {
            t1 = m[i-3] ^ m[i-8] ^ m[i-14] ^ m[i-16];
            m[i] = ROTLEFT(t1,1);
        }
        a = ctx->state[0];
        b = ctx->state[1];
        c = ctx->state[2];
        d = ctx->state[3];
        e = ctx->state[4];
        for(i = 0; i < 20; i++)
        {
            t1 = (ROTLEFT(a,5) + ((b & c) | ((~b) & d)) + e + m[i] + k1[0]);
            e = d;
            d = c;
            c = ROTLEFT(b,30);
            b = a;
            a = t1;
        }
        for(i = 20; i < 40; i++)
        {
            t1 = (ROTLEFT(a,5) + (b ^ c ^ d) + e + m[i] + k1[1]);
            e = d;
            d = c;
            c = ROTLEFT(b,30);
            b = a;
            a = t1;
        }
        for(i = 40; i < 60; i++)
        {
            t1 = ROTLEFT(a,5) + ((b & c) | (b & d) | (c & d)) + e + m[i] + k1[2];
            e = d;
            d = c;
            c = ROTLEFT(b,30);
            b = a;
            a = t1;
        }
        for(i = 60; i < 80; i++)
        {
            t1 = ROTLEFT(a,5) + (b ^ c ^ d) + e + m[i] + k1[3];
            e = d;
            d = c;
            c = ROTLEFT(b,30);
            b = a;
            a = t1;
        }
        ctx->state[0] += a;
        ctx->state[1] += b;
        ctx->state[2] += c;
        ctx->state[3] += d;
        ctx->state[4] += e;
    }
    else
    {
        for ( ; i < 64 ; ++i)
            m[i] = SIG1(m[i-2]) + m[i-7] + SIG0(m[i-15]) + m[i-16];
        
        a = ctx->state[0];
        b = ctx->state[1];
        c = ctx->state[2];
        d = ctx->state[3];
        e = ctx->state[4];
        f = ctx->state[5];
        g = ctx->state[6];
        h = ctx->state[7];
        
        for (i = 0; i < 64; ++i)
        {
            t1 = h + EP1(e) + CH(e,f,g) + k2[i] + m[i];
            t2 = EP0(a) + MAJ(a,b,c);
            h = g;
            g = f;
            f = e;
            e = d + t1;
            d = c;
            c = b;
            b = a;
            a = t1 + t2;
        }
        
        ctx->state[0] += a;
        ctx->state[1] += b;
        ctx->state[2] += c;
        ctx->state[3] += d;
        ctx->state[4] += e;
        ctx->state[5] += f;
        ctx->state[6] += g;
        ctx->state[7] += h;
    }
}

void
meHashInit(meHash *ctx,int n)
{
    ctx->bitlen = 0;
    ctx->datalen = 0;
    if((ctx->flag = (n & meHASH_SHA1)) == 0)
    {
        ctx->state[0] = 0x6a09e667;
        ctx->state[1] = 0xbb67ae85;
        ctx->state[2] = 0x3c6ef372;
        ctx->state[3] = 0xa54ff53a;
        ctx->state[4] = 0x510e527f;
        ctx->state[5] = 0x9b05688c;
        ctx->state[6] = 0x1f83d9ab;
        ctx->state[7] = 0x5be0cd19;
    }
    else
    {
        ctx->state[0] = 0x67452301;
        ctx->state[1] = 0xefcdab89;
        ctx->state[2] = 0x98badcfe;
        ctx->state[3] = 0x10325476;
        ctx->state[4] = 0xc3d2e1f0;
    }
}

void
meHashUpdate(meHash *ctx, const meUByte data[], size_t len)
{
    meUInt ll, ii=0;
    
    if((ll=ctx->datalen) != 0)
    {
        do
        {
            if(ii == len)
            {
                ctx->datalen = ll;
                return;
            }
            ctx->data[ll++] = data[ii++];
        } while(ll < 64);
        
        meHashTransform(ctx,ctx->data);
        if((ctx->bitlen += 512) == 0)
            ctx->flag |= meHASH_OVERRUN;
    }
    ll = (len - ii) >> 6;
    while(ll)
    {
        meHashTransform(ctx,data+ii);
        if((ctx->bitlen += 512) == 0)
            ctx->flag |= meHASH_OVERRUN;
        ii += 64;
        ll--;
    }
    while(ii < len)
        ctx->data[ll++] = data[ii++];
    ctx->datalen = (meUByte) ll;
}

void
meHashFinal(meHash *ctx, meUByte hash[])
{
    if((ctx->flag & meHASH_FINALISED) == 0)
    {
        meUInt ii = ctx->datalen;
        ctx->bitlen += ii * 8;
        
        /* Pad whatever data is left in the buffer. */
        if(ii < 56)
        {
            ctx->data[ii++] = 0x80;
            while(ii < 56)
                ctx->data[ii++] = 0x00;
        }
        else
        {
            ctx->data[ii++] = 0x80;
            while(ii < 64)
                ctx->data[ii++] = 0x00;
            meHashTransform(ctx,ctx->data);
            memset(ctx->data,0,56);
        }
        
        /* Append to the padding the total message's length in bits and transform. */
        ctx->data[63] = (meUByte) ctx->bitlen;
        ctx->data[62] = (meUByte) (ctx->bitlen >> 8);
        ctx->data[61] = (meUByte) (ctx->bitlen >> 16);
        ctx->data[60] = (meUByte) (ctx->bitlen >> 24);
        ctx->data[59] = (meUByte) (ctx->bitlen >> 32);
        ctx->data[58] = (meUByte) (ctx->bitlen >> 40);
        ctx->data[57] = (meUByte) (ctx->bitlen >> 48);
        ctx->data[56] = (meUByte) (ctx->bitlen >> 56);
        meHashTransform(ctx,ctx->data);
        ctx->flag |= meHASH_FINALISED;
    }
    
    /* Since this implementation uses little endian byte ordering and SHA uses big endian, */
    /* reverse all the bytes when copying the final state to the output hash. */
    if(ctx->flag & meHASH_SHA1)
    {
        meInt ii=3, jj=0;
        do {
            hash[ii]      = (ctx->state[0] >> jj) & 0x000000ff;
            hash[ii + 4]  = (ctx->state[1] >> jj) & 0x000000ff;
            hash[ii + 8]  = (ctx->state[2] >> jj) & 0x000000ff;
            hash[ii + 12] = (ctx->state[3] >> jj) & 0x000000ff;
            hash[ii + 16] = (ctx->state[4] >> jj) & 0x000000ff;
            jj += 8;
        } while(--ii >= 0);
    }
    else
    {
        meInt ii=3, jj=0;
        do {
            hash[ii]      = (ctx->state[0] >> jj) & 0x000000ff;
            hash[ii + 4]  = (ctx->state[1] >> jj) & 0x000000ff;
            hash[ii + 8]  = (ctx->state[2] >> jj) & 0x000000ff;
            hash[ii + 12] = (ctx->state[3] >> jj) & 0x000000ff;
            hash[ii + 16] = (ctx->state[4] >> jj) & 0x000000ff;
            hash[ii + 20] = (ctx->state[5] >> jj) & 0x000000ff;
            hash[ii + 24] = (ctx->state[6] >> jj) & 0x000000ff;
            hash[ii + 28] = (ctx->state[7] >> jj) & 0x000000ff;
            jj += 8;
        } while(--ii >= 0);
    }
}

// HMAC-SHA256 support
void
meHashInitHmac(meHash *iCtx, meHash *oCtx, int n, const meUByte *key, int keyLen)
{
    meUByte pad[64];
    meUByte keyHsh[32];
    int ii;
    
    if(keyLen > 64)
    {
        meHashInit(iCtx,n);
        meHashUpdate(iCtx,key,keyLen);
        meHashFinal(iCtx,keyHsh);
        key = keyHsh;
        keyLen = 32;
    }
    
    meHashInit(iCtx,n);
    memset(pad,0x36,64);
    ii = keyLen;
    while(--ii >= 0)
        pad[ii] ^= key[ii];
    meHashUpdate(iCtx,pad,64);
    
    meHashInit(oCtx,n);
    memset(pad,0x5c,64);
    ii = keyLen;
    while(--ii >= 0)
        pad[ii] ^= key[ii];
    meHashUpdate(oCtx,pad,64);
}

int
generateHash(int f, int n)
{
    meHash iCtx, oCtx;
    meUByte *ss, *bb, cc, dd, hd[64];
    int ii, elo;
    
    if(n & 0x04)
    {
        /* HMAC-SHA256, must get key */
        if(meGetString((meUByte *) "Key",MLNOHIST|MLHIDEVAL,0,resultStr,meBUF_SIZE_MAX) <= 0)
            return meABORT;
        if(n & 0x08)
        {
            bb = ss = resultStr;
            while((cc=*ss++) != '\0')
            {
                if(!isXDigit(cc) || ((dd = *ss++) == '\0') || !isXDigit(dd))
                    return mlwrite(MWABORT|MWPAUSE,(meUByte *)"[Data hex format error]");
                *bb++ = (hexToNum(cc)<<4) | hexToNum(dd);
            }
            ii = (size_t) (bb - resultStr);
        }
        else
            ii = meStrlen(resultStr);
        meHashInitHmac(&iCtx,&oCtx,n,resultStr,ii);
        resultStr[0] = '\0';
    }
    else
        meHashInit(&iCtx,n);
    if(n & 0x02)
    {
        if(meGetString((meUByte *) "Data",0,0,resultStr,meBUF_SIZE_MAX) <= 0)
            return meABORT;
        if(n & 0x01)
        {
            bb = ss = resultStr;
            while((cc=*ss++) != '\0')
            {
                if(!isXDigit(cc) || ((dd = *ss++) == '\0') || !isXDigit(dd))
                    return mlwrite(MWABORT|MWPAUSE,(meUByte *)"[Data hex format error]");
                *bb++ = (hexToNum(cc)<<4) | hexToNum(dd);
            }
            ii = (size_t) (bb - resultStr);
        }
        else
            ii = meStrlen(resultStr);
        meHashUpdate(&iCtx,resultStr,ii);
    }
    else
    {
        meWindow *wp=frameCur->windowCur;
        meBuffer *bp=wp->buffer;
        register meLine *elp,*lp;
        
        if(meModeTest(bp->mode,MDCR))
        {
            if(meModeTest(bp->mode,MDLF))
            {
                ss = (meUByte *) "\r\n";
                ii = 2;
            }
            else
            {
                ss = (meUByte *) "\r";
                ii = 1;
            }
        }
        else if(meModeTest(bp->mode,MDLF))
        {
            ss = (meUByte *) "\n";
            ii = 1;
        }
        else
        {
            ss = (meUByte *) "";
            ii = 0;
        }
        if(n & 1)
        {            
            elp = bp->baseLine;
            lp = meLineGetNext(elp);
        }
        else
        {
            int slo;
            if(wp->markLine == NULL)
                return noMarkSet();
            if(meModeTest(bp->mode,MDBINARY) || meModeTest(bp->mode,MDRBIN))
            {
                if(wp->dotLineNo < wp->markLineNo)
                {
                    lp = wp->dotLine;
                    elp = wp->markLine;
                    if(wp->markOffset)
                        elp = meLineGetNext(elp);
                }
                else
                {
                    lp = wp->markLine;
                    elp = wp->dotLine;
                    if(wp->dotOffset)
                        elp = meLineGetNext(elp);
                }
                elo = 0;
            }
            else if(wp->dotLineNo == wp->markLineNo)
            {
                elp = lp = wp->dotLine;
                slo = wp->dotOffset;
                elo = wp->markOffset-slo;
                if(elo < 0)
                {
                    slo = wp->markOffset;
                    elo = 0 - elo;
                }
                meHashUpdate(&iCtx,meLineGetText(lp)+slo,elo);
                elo = 0;
            }
            else
            {
                if(wp->dotLineNo < wp->markLineNo)
                {
                    lp = wp->dotLine;
                    slo = wp->dotOffset;
                    elp = wp->markLine;
                    elo = wp->markOffset;
                }
                else
                {
                    lp = wp->markLine;
                    slo = wp->markOffset;
                    elp = wp->dotLine;
                    elo = wp->dotOffset;
                }
                if(slo < meLineGetLength(lp))
                    meHashUpdate(&iCtx,meLineGetText(lp)+slo,meLineGetLength(lp)-slo);
                if(ii && !(lp->flag & meLINE_NOEOL))
                    meHashUpdate(&iCtx,ss,ii);
                lp = meLineGetNext(lp);
            }
        }
        if(meModeTest(bp->mode,MDBINARY))
        {
            while(lp != elp)
            {
                ss = meLineGetText(lp);
                while((cc=*ss++) != '\0')
                {
                    if(cc == ':')
                    {
                        while(((cc=*ss++) != '\0') && (cc != '|'))
                        {
                            if(cc != ' ')
                            {
                                if(!isXDigit(cc) || ((dd = *ss++) == '\0') || !isXDigit(dd))
                                    return mlwrite(MWABORT|MWPAUSE,(meUByte *)"Binary format error");
                                hd[0] = (hexToNum(cc)<<4) | hexToNum(dd);
                                meHashUpdate(&iCtx,hd,1);
                            }
                        }
                        break;
                    }
                }
                lp = meLineGetNext(lp);
            }
        }
        else if(meModeTest(bp->mode,MDRBIN))
        {
            while(lp != elp)
            {
                ss = meLineGetText(lp);
                while((cc=*ss++) != '\0')
                {
                    if(!isXDigit(cc) || ((dd = *ss++) == '\0') || !isXDigit(dd))
                        return mlwrite(MWABORT|MWPAUSE,(meUByte *)"[rbin format error]");
                    hd[0] = (hexToNum(cc)<<4) | hexToNum(dd);
                    meHashUpdate(&iCtx,hd,1);
                }
                lp = meLineGetNext(lp);
            }
        }
        else
        {
            while(lp != elp)
            {
                meHashUpdate(&iCtx,meLineGetText(lp),meLineGetLength(lp));
                if(ii && !(lp->flag & meLINE_NOEOL))
                    meHashUpdate(&iCtx,ss,ii);
                lp = meLineGetNext(lp);
            }
            if(n & 1)
            {
                if(meModeTest(bp->mode,MDCTRLZ))
                    meHashUpdate(&iCtx,(meUByte *) "\x1a",1);
            }
            else if(elo)
                meHashUpdate(&iCtx,meLineGetText(elp),elo);
        }
    }
    if(iCtx.flag & meHASH_OVERRUN)
        return mlwrite(MWABORT|MWPAUSE,(meUByte *)"[Hash calc has overrun]");
    elo = (n & meHASH_SHA1) ? 20:32;
    if(n & 0x04)
    {
        meHashFinal(&iCtx,resultStr);
        meHashUpdate(&oCtx,resultStr,elo);
        meHashFinal(&oCtx,hd);
    }
    else
        meHashFinal(&iCtx,hd);
    ss = resultStr;
    for(ii=0 ; ii<elo ; ii++)
    {
        cc = hd[ii];
        *ss++ = hexdigits[cc >> 4];
        *ss++ = hexdigits[cc & 0x0f];
    }
    *ss = '\0';
    return meTRUE;
}

#endif
