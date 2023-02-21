
#include "emain.h"

#if MEOPT_EXTENDED

/*********************************************************************
 * Filename:   sha256.c
 * Author:     Brad Conte (brad AT bradconte.com)
 * Copyright:
 * Disclaimer: This code is presented "as is" without any guarantees.
 * Details:    Implementation of the SHA-256 hashing algorithm.
               SHA-256 is one of the three algorithms in the SHA2
               specification. The others, SHA-384 and SHA-512, are not
               offered in this implementation.
               Algorithm specification can be found here:
 * http://csrc.nist.gov/publications/fips/fips180-2/fips180-2withchangenotice.pdf
               This implementation uses little endian byte order.
 *********************************************************************/
// SHA256 outputs a 32 byte digest
#define SHA256_BLOCK_SIZE 32

typedef struct {
    meUByte data[64];
    meUInt datalen;
    meULong bitlen;
    meUInt state[8];
} SHA256_CTX;

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

static const meUInt k[64] = {
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
sha256_transform(SHA256_CTX *ctx, const meUByte data[])
{
    meUInt a, b, c, d, e, f, g, h, i, j, t1, t2, m[64];
    
    for (i = 0, j = 0; i < 16; ++i, j += 4)
        m[i] = (data[j] << 24) | (data[j + 1] << 16) | (data[j + 2] << 8) | (data[j + 3]);
    for ( ; i < 64; ++i)
        m[i] = SIG1(m[i - 2]) + m[i - 7] + SIG0(m[i - 15]) + m[i - 16];
    
    a = ctx->state[0];
    b = ctx->state[1];
    c = ctx->state[2];
    d = ctx->state[3];
    e = ctx->state[4];
    f = ctx->state[5];
    g = ctx->state[6];
    h = ctx->state[7];
    
    for (i = 0; i < 64; ++i) {
        t1 = h + EP1(e) + CH(e,f,g) + k[i] + m[i];
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

void
sha256_init(SHA256_CTX *ctx)
{
    ctx->datalen = 0;
    ctx->bitlen = 0;
    ctx->state[0] = 0x6a09e667;
    ctx->state[1] = 0xbb67ae85;
    ctx->state[2] = 0x3c6ef372;
    ctx->state[3] = 0xa54ff53a;
    ctx->state[4] = 0x510e527f;
    ctx->state[5] = 0x9b05688c;
    ctx->state[6] = 0x1f83d9ab;
    ctx->state[7] = 0x5be0cd19;
}

void
sha256_update(SHA256_CTX *ctx, const meUByte data[], size_t len)
{
    meUInt i;
    
    for (i = 0; i < len; ++i) {
        ctx->data[ctx->datalen] = data[i];
        ctx->datalen++;
        if (ctx->datalen == 64) {
            sha256_transform(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}

void
sha256_final(SHA256_CTX *ctx, meUByte hash[])
{
    meUInt i;
    
    i = ctx->datalen;
    
    // Pad whatever data is left in the buffer.
    if (ctx->datalen < 56) {
        ctx->data[i++] = 0x80;
        while (i < 56)
            ctx->data[i++] = 0x00;
    }
    else {
        ctx->data[i++] = 0x80;
        while (i < 64)
            ctx->data[i++] = 0x00;
        sha256_transform(ctx, ctx->data);
        memset(ctx->data, 0, 56);
    }
    
    // Append to the padding the total message's length in bits and transform.
    ctx->bitlen += ctx->datalen * 8;
    ctx->data[63] = (meUByte) ctx->bitlen;
    ctx->data[62] = (meUByte) (ctx->bitlen >> 8);
    ctx->data[61] = (meUByte) (ctx->bitlen >> 16);
    ctx->data[60] = (meUByte) (ctx->bitlen >> 24);
    ctx->data[59] = (meUByte) (ctx->bitlen >> 32);
    ctx->data[58] = (meUByte) (ctx->bitlen >> 40);
    ctx->data[57] = (meUByte) (ctx->bitlen >> 48);
    ctx->data[56] = (meUByte) (ctx->bitlen >> 56);
    sha256_transform(ctx, ctx->data);
    
    // Since this implementation uses little endian byte ordering and SHA uses big endian,
    // reverse all the bytes when copying the final state to the output hash.
    for (i = 0; i < 4; ++i) {
        hash[i]      = (ctx->state[0] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 4]  = (ctx->state[1] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 8]  = (ctx->state[2] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 12] = (ctx->state[3] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 16] = (ctx->state[4] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 20] = (ctx->state[5] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 24] = (ctx->state[6] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 28] = (ctx->state[7] >> (24 - i * 8)) & 0x000000ff;
    }
}

// HMAC-SHA256 support
void
hmac_sha256_init(SHA256_CTX *iCtx, SHA256_CTX *oCtx, const meUByte *key, int keyLen)
{
    meUByte pad[64];
    meUByte keyHsh[32];
    int ii;
    
    if(keyLen > 64)
    {
        sha256_init(iCtx);
        sha256_update(iCtx,key,keyLen);
        sha256_final(iCtx,keyHsh);
        key = keyHsh;
        keyLen = 32;
    }
    
    sha256_init(iCtx);
    memset(pad,0x36,64);
    ii = keyLen;
    while(--ii >= 0)
        pad[ii] ^= key[ii];
    sha256_update(iCtx,pad,64);
    
    sha256_init(oCtx);
    memset(pad,0x5c,64);
    ii = keyLen;
    while(--ii >= 0)
        pad[ii] ^= key[ii];
    sha256_update(oCtx,pad,64);
}

int
generateHash(int f, int n)
{
    register meLine *elp,*lp;
    SHA256_CTX iCtx, oCtx;
    meUByte *ss, cc, hd[32];
    int ii, elo;
    
    if(meModeTest(frameCur->bufferCur->mode,MDCR))
    {
        if(meModeTest(frameCur->bufferCur->mode,MDLF))
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
    else if(meModeTest(frameCur->bufferCur->mode,MDLF))
    {
        ss = (meUByte *) "\n";
        ii = 1;
    }
    else
    {
        ss = (meUByte *) "";
        ii = 0;
    }
    if(n & 0x04)
    {
        /* HMAC-SHA256, must get key */
        if(meGetString((meUByte *) "Key",MLNOHIST|MLHIDEVAL,0,resultStr,meBUF_SIZE_MAX) <= 0)
            return meABORT ;
        hmac_sha256_init(&iCtx,&oCtx,resultStr,meStrlen(resultStr));
        resultStr[0] = '\0';
    }
    else
        sha256_init(&iCtx);
    if((n & 0x03) == 0)
    {
        if(meGetString((meUByte *) "Data",0,0,resultStr,meBUF_SIZE_MAX) <= 0)
            return meABORT ;
        sha256_update(&iCtx,resultStr,meStrlen(resultStr));
    }
    else
    {
        if(n & 2)
        {
            int slo;
            if(frameCur->windowCur->markLine == NULL)
                return noMarkSet();
            if(meModeTest(frameCur->bufferCur->mode,MDBINARY) || meModeTest(frameCur->bufferCur->mode,MDRBIN))
            {
                if(frameCur->windowCur->dotLineNo < frameCur->windowCur->markLineNo)
                {
                    lp = frameCur->windowCur->dotLine;
                    elp = frameCur->windowCur->markLine;
                    if(frameCur->windowCur->markOffset)
                        elp = meLineGetNext(elp);
                }
                else
                {
                    lp = frameCur->windowCur->markLine;
                    elp = frameCur->windowCur->dotLine;
                    if(frameCur->windowCur->dotOffset)
                        elp = meLineGetNext(elp);
                }
                elo = 0;
            }
            else if(frameCur->windowCur->dotLineNo == frameCur->windowCur->markLineNo)
            {
                elp = lp = frameCur->windowCur->dotLine;
                slo = frameCur->windowCur->dotOffset;
                elo = frameCur->windowCur->markOffset-slo;
                if(elo < 0)
                {
                    slo = frameCur->windowCur->markOffset;
                    elo = 0 - elo;
                }
                sha256_update(&iCtx,meLineGetText(lp)+slo,elo);
                elo = 0;
            }
            else
            {
                if(frameCur->windowCur->dotLineNo < frameCur->windowCur->markLineNo)
                {
                    lp = frameCur->windowCur->dotLine;
                    slo = frameCur->windowCur->dotOffset;
                    elp = frameCur->windowCur->markLine;
                    elo = frameCur->windowCur->markOffset;
                }
                else
                {
                    lp = frameCur->windowCur->markLine;
                    slo = frameCur->windowCur->markOffset;
                    elp = frameCur->windowCur->dotLine;
                    elo = frameCur->windowCur->dotOffset;
                }
                if(slo < meLineGetLength(lp))
                    sha256_update(&iCtx,meLineGetText(lp)+slo,meLineGetLength(lp)-slo);
                if(ii && !(lp->flag & meLINE_NOEOL))
                    sha256_update(&iCtx,ss,ii);
                lp = meLineGetNext(lp);
            }
        }
        else
        {            
            elp = frameCur->bufferCur->baseLine;
            lp = meLineGetNext(elp);
        }
        if(meModeTest(frameCur->bufferCur->mode,MDBINARY))
        {
            meUByte dd;
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
                                sha256_update(&iCtx,hd,1);
                            }
                        }
                        break;
                    }
                }
                lp = meLineGetNext(lp);
            }
        }
        else if(meModeTest(frameCur->bufferCur->mode,MDRBIN))
        {
            meUByte dd;
            while(lp != elp)
            {
                ss = meLineGetText(lp);
                while((cc=*ss++) != '\0')
                {
                    if(!isXDigit(cc) || ((dd = *ss++) == '\0') || !isXDigit(dd))
                        return mlwrite(MWABORT|MWPAUSE,(meUByte *)"[rbin format error]");
                    hd[0] = (hexToNum(cc)<<4) | hexToNum(dd);
                    sha256_update(&iCtx,hd,1);
                }
                lp = meLineGetNext(lp);
            }
        }
        else
        {
            while(lp != elp)
            {
                sha256_update(&iCtx,meLineGetText(lp),meLineGetLength(lp));
                if(ii && !(lp->flag & meLINE_NOEOL))
                    sha256_update(&iCtx,ss,ii);
                lp = meLineGetNext(lp);
            }
            if(n & 2)
            {
                if(elo)
                    sha256_update(&iCtx,meLineGetText(elp),elo);
            }
            else if(meModeTest(frameCur->bufferCur->mode,MDCTRLZ))
                sha256_update(&iCtx,(meUByte *) "\x1a",1);
        }
    }
    if(n & 0x04)
    {
        sha256_final(&iCtx,resultStr);
        sha256_update(&oCtx,resultStr,32);
        sha256_final(&oCtx,hd);
    }
    else
        sha256_final(&iCtx,hd);
    ss = resultStr;
    for(ii=0 ; ii<32 ; ii++)
    {
        cc = hd[ii];
        *ss++ = hexdigits[cc >> 4];
        *ss++ = hexdigits[cc & 0x0f];
    }
    *ss = '\0';
    return meTRUE;
}

#endif
