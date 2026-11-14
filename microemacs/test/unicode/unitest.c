/*
 * unitest.c - Unit tests for src/unicode.c
 *
 * Build & run (see run_unitest.sh):
 *   python3 -I dev/mkunitbl.py --dump unitbl.dump <ucd-dir> src/eunitbl.h
 *   gcc -O2 -Wall -I src -o unitest test/unicode/unitest.c
 *   ./unitest unitbl.dump
 *
 * Without a dump file the table cross-check is skipped.
 */
#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <wchar.h>

typedef   signed char  meByte;
typedef unsigned char  meUByte;
typedef   signed short meShort;
typedef unsigned short meUShort;
typedef   signed int   meInt;
typedef unsigned int   meUInt;

#define MEOPT_UNICODE 1
#define _ME_UNITEST
#include "eunicode.h"
#include "unicode.c"

static int failCount, checkCount;

#define CHECK(cond,...) do { checkCount++; if(!(cond)) { if(++failCount <= 40) { printf("FAIL %s:%d: ",__FILE__,__LINE__); printf(__VA_ARGS__); printf("\n"); } } } while(0)

static void
testCodec(void)
{
    meUByte buf[8];
    meUInt uc, dc;
    int ll, dl, sl;

    for(uc=0 ; uc<=meUNI_MAX ; uc++)
    {
        ll = meUtf8Encode(buf,uc);
        CHECK(ll == ((uc == 0) ? 3:(uc < 0x80) ? 1:(uc < 0x800) ? 2:(uc < 0x10000) ? 3:4),"encode length U+%04X = %d",uc,ll);
        dl = meUtf8Decode(buf,ll,&dc);
        CHECK((dl == ll) && (dc == uc),"decode U+%04X -> %d,U+%04X",uc,dl,dc);
        sl = meUtf8SeqLen(buf,ll);
        if((uc == 0) || ((uc >= 0xd800) && (uc <= 0xdfff)))
            CHECK(sl == 0,"strict accepted internal form of U+%04X",uc);
        else
            CHECK(sl == ll,"strict rejected U+%04X (%d)",uc,sl);
        if(ll > 1)
        {
            sl = meUtf8SeqLen(buf,ll-1);
            if((uc != 0) && ((uc < 0xd800) || (uc > 0xdfff)))
                CHECK(sl == -1,"truncated U+%04X not -1 (%d)",uc,sl);
            dl = meUtf8Decode(buf,ll-1,&dc);
            CHECK((dl == 1) && meUniIsRaw(dc),"truncated U+%04X decoded %d,%X",uc,dl,dc);
        }
    }
    for(uc=0x80 ; uc<0x100 ; uc++)
    {
        ll = meUtf8Encode(buf,meUNI_RAW_BASE+uc);
        CHECK((ll == 2) && ((buf[0] & 0xfe) == 0xc0),"raw 0x%02X escape",uc);
        CHECK(meUtf8SeqLen(buf,2) == 0,"raw 0x%02X escape strictly valid",uc);
        dl = meUtf8Decode(buf,2,&dc);
        CHECK((dl == 2) && (dc == meUNI_RAW_BASE+uc),"raw 0x%02X decode %d,%X",uc,dl,dc);
        buf[0] = (meUByte) uc;
        dl = meUtf8Decode(buf,1,&dc);
        CHECK((dl == 1) && (dc == meUNI_RAW_BASE+uc),"lone 0x%02X decode %d,%X",uc,dl,dc);
    }
    for(uc=0 ; uc<0x80 ; uc++)
    {
        ll = meUtf8Encode(buf,meUNI_RAW_BASE+uc);
        CHECK((ll == 3) && (buf[0] == 0xe0) && (meUtf8SeqLen(buf,3) == 0),"raw 0x%02X escape form",uc);
        dl = meUtf8Decode(buf,ll,&dc);
        CHECK((dl == 3) && (dc == meUNI_RAW_BASE+uc),"raw 0x%02X decode %d,%X",uc,dl,dc);
    }
    dl = meUtf8Decode((const meUByte *) "\xe0\x80\x81",3,&dc);
    CHECK((dl == 1) && (dc == meUNI_RAW_BASE+0xe0),"unused overlong E0 80 81 not raw (%d,%X)",dl,dc);
    ll = meUtf8Encode(buf,0);
    CHECK((ll == 3) && !memcmp(buf,"\xe0\x80\x80",3),"NUL internal form");
    ll = meUtf8Encode(buf,meUNI_RAW_BASE+0x200);
    CHECK((ll == 3) && !memcmp(buf,"\xef\xbf\xbd",3),"invalid value not U+FFFD");
}

static void
testMalformed(void)
{
    static const struct { const char *ss; int len; int seqLen; } tv[] = {
        { "\xc0\xaf", 2, 0 },               /* overlong '/' */
        { "\xe0\x80\xaf", 3, 0 },
        { "\xf0\x80\x80\xaf", 4, 0 },
        { "\xc1\xbf", 2, 0 },
        { "\xe0\x9f\xbf", 3, 0 },           /* overlong U+07FF */
        { "\xf0\x8f\xbf\xbf", 4, 0 },       /* overlong U+FFFF */
        { "\xf4\x90\x80\x80", 4, 0 },       /* U+110000 */
        { "\xf5\x80\x80\x80", 4, 0 },
        { "\xff", 1, 0 },
        { "\xfe", 1, 0 },
        { "\x80", 1, 0 },
        { "\xbf", 1, 0 },
        { "\xed\xa0\x80", 3, 0 },           /* surrogate */
        { "\xe0\x80\x80", 3, 0 },           /* NUL internal form */
        { "\xed\x9f\xbf", 3, 3 },           /* U+D7FF */
        { "\xee\x80\x80", 3, 3 },           /* U+E000 */
        { "\xef\xbf\xbf", 3, 3 },           /* U+FFFF */
        { "\xf4\x8f\xbf\xbf", 4, 4 },       /* U+10FFFF */
        { "\xc3", 1, -1 },
        { "\xc3\x41", 2, 0 },
        { "\xe2\x82", 2, -1 },
        { "\xe2\x82\x41", 3, 0 },
        { "\xf0\x9f\x98", 3, -1 },
        { "\xf0\x9f\x98\x41", 4, 0 },
    };
    meUInt uc;
    int ii, sl, dl;

    for(ii=0 ; ii<(int) (sizeof(tv)/sizeof(tv[0])) ; ii++)
    {
        sl = meUtf8SeqLen((const meUByte *) tv[ii].ss,tv[ii].len);
        CHECK(sl == tv[ii].seqLen,"malformed vector %d: seqLen %d != %d",ii,sl,tv[ii].seqLen);
        dl = meUtf8Decode((const meUByte *) tv[ii].ss,tv[ii].len,&uc);
        if(tv[ii].seqLen > 0)
            CHECK((dl == tv[ii].seqLen) && !meUniIsRaw(uc),"vector %d decode %d,%X",ii,dl,uc);
        else if((tv[ii].len == 3) && !memcmp(tv[ii].ss,"\xe0\x80\x80",3))
            CHECK((dl == 3) && (uc == 0),"vector %d NUL decode %d,%X",ii,dl,uc);
        else if(((meUByte) tv[ii].ss[0] == 0xed) && (tv[ii].len == 3))
            CHECK((dl == 3) && (uc == 0xd800),"vector %d surrogate decode %d,%X",ii,dl,uc);
        else if(((meUByte) tv[ii].ss[0] & 0xfe) == 0xc0)
            CHECK((dl == 2) && meUniIsRaw(uc),"vector %d escape decode %d,%X",ii,dl,uc);
        else
            CHECK((dl == 1) && (uc == meUNI_RAW_BASE + (meUInt) (meUByte) tv[ii].ss[0]),"vector %d decode %d,%X",ii,dl,uc);
    }
}

static meUInt rndState=12345;
static meUInt
rnd(void)
{
    rndState ^= rndState << 13;
    rndState ^= rndState >> 17;
    rndState ^= rndState << 5;
    return rndState;
}

/* Forward and backward walks of random internal text must agree on every boundary */
static void
testNavigation(void)
{
    static const meUInt pick[] = { 0, 'a', 0xe9, 0x3b1, 0x4e00, 0x1f600, 0xd800, 0xdfff, 0x10ffff, 0x0301 };
    meUByte text[256];
    int bound[256], nb, len, off, ii, iter;
    meUInt uc;

    for(iter=0 ; iter<200000 ; iter++)
    {
        len = 0;
        while(len < 200)
        {
            switch(rnd() % 4)
            {
            case 0:
                len += meUtf8Encode(text+len,pick[rnd() % (sizeof(pick)/sizeof(pick[0]))]);
                break;
            case 1:
                len += meUtf8Encode(text+len,meUNI_RAW_BASE + (rnd() & 0xff));
                break;
            case 2:
                text[len++] = (meUByte) rnd();
                break;
            default:
                uc = rnd() % (meUNI_MAX + 1);
                len += meUtf8Encode(text+len,uc);
                break;
            }
        }
        nb = 0;
        off = 0;
        while(off < len)
        {
            bound[nb++] = off;
            ii = meUtf8Next(text,off,len);
            if(ii <= off)
                break;
            off = ii;
        }
        CHECK(off == len,"forward walk overran (%d,%d) iter %d",off,len,iter);
        off = len;
        ii = nb;
        while(off > 0)
        {
            off = meUtf8Prev(text,off);
            if(--ii < 0)
                break;
            if(bound[ii] != off)
                break;
        }
        CHECK((ii == 0) && (off == 0),"backward walk mismatch at %d (%d) iter %d",off,(ii >= 0) ? bound[ii]:-1,iter);
        if(failCount > 40)
            return;
    }
}

static meUInt
caseLookup(meUInt (*fn)(meUInt), meUInt uc)
{
    return fn(uc);
}

static void
testDump(const char *fname)
{
    static meUByte props[meUNI_MAX+1];
    static meUInt *maps[3];
    static meUInt (*fns[3])(meUInt) = { meUniToLower, meUniToUpper, meUniFold };
    char name[16];
    FILE *fp;
    meUInt uc, src, dst;
    int ii, nn, jj;

    if((fp = fopen(fname,"rb")) == NULL)
    {
        printf("SKIP table cross-check, failed to open %s\n",fname);
        return;
    }
    if(fread(props,1,sizeof(props),fp) != sizeof(props))
    {
        CHECK(0,"short dump file");
        fclose(fp);
        return;
    }
    for(uc=0 ; uc<=meUNI_MAX ; uc++)
        CHECK(meUniProps(uc) == props[uc],"props U+%04X = %02X expected %02X",uc,meUniProps(uc),props[uc]);
    for(ii=0 ; ii<3 ; ii++)
    {
        maps[ii] = malloc(sizeof(meUInt)*(meUNI_MAX+1));
        for(uc=0 ; uc<=meUNI_MAX ; uc++)
            maps[ii][uc] = uc;
        if(fscanf(fp,"%15s %d",name,&nn) != 2)
        {
            CHECK(0,"bad dump mapping header %d",ii);
            break;
        }
        for(jj=0 ; jj<nn ; jj++)
        {
            if(fscanf(fp,"%x %x",&src,&dst) != 2)
                break;
            maps[ii][src] = dst;
        }
        for(uc=0 ; uc<=meUNI_MAX ; uc++)
            CHECK(caseLookup(fns[ii],uc) == maps[ii][uc],"%s U+%04X = U+%04X expected U+%04X",name,uc,caseLookup(fns[ii],uc),maps[ii][uc]);
        free(maps[ii]);
    }
    fclose(fp);
}

/* Spot checks of the generation rules, independent of the dump */
static void
testRules(void)
{
    static const struct { meUInt uc; int width; } wv[] = {
        { 'A', 1 }, { 0xe9, 3 }, { 0xe0, 3 }, { 0xe1, 3 }, { 0xe3, 1 }, { 0x3b1, 3 }, { 0x4e00, 2 }, { 0x3000, 2 }, { 0xff21, 2 },
        { 0x0301, 0 }, { 0x200b, 0 }, { 0x200d, 0 }, { 0xfe0f, 0 }, { 0x00ad, 0 }, { 0x3164, 0 }, { 0x0600, 1 },
        { 0x1100, 2 }, { 0x1160, 0 }, { 0x11a8, 0 }, { 0xac00, 2 }, { 0x1f600, 2 }, { 0x20000, 2 },
        { 0x2fffd, 2 }, { 0x3fffd, 2 }, { 0x0007, 0 }, { 0x0085, 0 }, { 0xe000, 3 }, { 0x2500, 3 },
    };
    static const struct { meUInt uc, lower, upper, fold; } cv[] = {
        { 'A', 'a', 'A', 'a' }, { 0xc9, 0xe9, 0xc9, 0xe9 }, { 0xe9, 0xe9, 0xc9, 0xe9 },
        { 0x130, 0x69, 0x130, 0x130 }, { 0x131, 0x131, 0x49, 0x131 }, { 0xdf, 0xdf, 0xdf, 0xdf },
        { 0x1e9e, 0xdf, 0x1e9e, 0xdf }, { 0x23a, 0x2c65, 0x23a, 0x2c65 }, { 0x3a3, 0x3c3, 0x3a3, 0x3c3 },
        { 0x3c2, 0x3c2, 0x3a3, 0x3c3 }, { 0x414, 0x434, 0x414, 0x434 }, { 0x17f, 0x17f, 'S', 's' },
        { 0x1c5, 0x1c6, 0x1c4, 0x1c6 }, { 0x10400, 0x10428, 0x10400, 0x10428 }, { 0x4e00, 0x4e00, 0x4e00, 0x4e00 },
    };
    meUByte pp;
    int ii;

    for(ii=0 ; ii<(int) (sizeof(wv)/sizeof(wv[0])) ; ii++)
        CHECK(meUniPropWidth(meUniProps(wv[ii].uc)) == wv[ii].width,"width class U+%04X = %d expected %d",
              wv[ii].uc,meUniPropWidth(meUniProps(wv[ii].uc)),wv[ii].width);
    CHECK(meUniWidth(0x3b1,0) == 1,"ambiguous narrow");
    CHECK(meUniWidth(0xe9,0) == 1,"U+00E9 ambiguous narrow");
    CHECK(meUniWidth(0x3b1,1) == 2,"ambiguous wide");
    CHECK(meUniWidth(meUNI_RAW_BASE+0xe9,0) == 0,"raw byte width");
    for(ii=0 ; ii<(int) (sizeof(cv)/sizeof(cv[0])) ; ii++)
    {
        CHECK(meUniToLower(cv[ii].uc) == cv[ii].lower,"lower U+%04X = U+%04X",cv[ii].uc,meUniToLower(cv[ii].uc));
        CHECK(meUniToUpper(cv[ii].uc) == cv[ii].upper,"upper U+%04X = U+%04X",cv[ii].uc,meUniToUpper(cv[ii].uc));
        CHECK(meUniFold(cv[ii].uc) == cv[ii].fold,"fold U+%04X = U+%04X",cv[ii].uc,meUniFold(cv[ii].uc));
    }
    pp = meUniProps(0xe9);
    CHECK(meUniPropIsAlpha(pp) && meUniPropIsLower(pp) && meUniPropIsWord(pp) && !meUniPropIsUpper(pp),"U+00E9 classes");
    pp = meUniProps(0x1c5);
    CHECK(meUniPropIsUpper(pp) && meUniPropIsAlpha(pp),"U+01C5 titlecase classes");
    pp = meUniProps(0x0661);
    CHECK(meUniPropIsDigit(pp) && meUniPropIsWord(pp) && !meUniPropIsAlpha(pp),"U+0661 classes");
    pp = meUniProps(0x0301);
    CHECK(meUniPropIsMark(pp) && meUniPropIsWord(pp),"U+0301 classes");
    pp = meUniProps('_');
    CHECK(meUniPropIsWord(pp) && !meUniPropIsAlpha(pp),"underscore classes");
    pp = meUniProps('-');
    CHECK(!meUniPropIsWord(pp),"hyphen classes");
    pp = meUniProps(0x3000);
    CHECK(meUniPropIsSpace(pp) && (meUniPropGC(pp) == meUNIGC_Zs),"U+3000 classes");
    pp = meUniProps(0x00a0);
    CHECK(meUniPropIsSpace(pp),"U+00A0 space");
    pp = meUniProps(0x200b);
    CHECK(!meUniPropIsSpace(pp) && (meUniPropGC(pp) == meUNIGC_Cf),"U+200B classes");
    pp = meUniProps(0x0378);
    CHECK(meUniPropGC(pp) == meUNIGC_Cn,"U+0378 unassigned");
    pp = meUniProps(0xd800);
    CHECK(meUniPropGC(pp) == meUNIGC_Cs,"U+D800 surrogate");
}

/* Not a pass/fail test - glibc's table is usually an older Unicode version and the terminal is
 * what matters, but large disagreement would point at a rule error */
static void
compareWcwidth(void)
{
    meUInt uc;
    int me, lc, diff=0, shown=0;

    if(setlocale(LC_CTYPE,"C.UTF-8") == NULL)
    {
        printf("INFO no C.UTF-8 locale, wcwidth comparison skipped\n");
        return;
    }
    for(uc=0x20 ; uc<=meUNI_MAX ; uc++)
    {
        meUByte pp = meUniProps(uc);
        if((meUniPropGC(pp) == meUNIGC_Cn) || (meUniPropGC(pp) == meUNIGC_Cs) ||
           (meUniPropGC(pp) == meUNIGC_Co) || (meUniPropGC(pp) == meUNIGC_Cc))
            continue;
        if((lc = wcwidth((wchar_t) uc)) < 0)
            continue;
        me = meUniWidth(uc,0);
        if(me != lc)
        {
            diff++;
            if(shown++ < 10)
                printf("INFO width U+%04X me %d glibc %d\n",uc,me,lc);
        }
    }
    printf("INFO %d assigned code points differ from glibc wcwidth\n",diff);
}

int
main(int argc, char *argv[])
{
    printf("Unicode %s tables\n",meUniVersion());
    testCodec();
    testMalformed();
    testNavigation();
    testRules();
    if(argc > 1)
        testDump(argv[1]);
    else
        printf("SKIP table cross-check, no dump file given\n");
    compareWcwidth();
    printf("%d checks, %d failures\n",checkCount,failCount);
    return (failCount != 0);
}
