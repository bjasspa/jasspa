/* -*- C -*- *****************************************************************
 *
 * Copyright (C) 2025 JASSPA (www.jasspa.com).
 *
 * This is part of JASSPA's MicroEmacs, see the LICENSE file for licensing and
 * copying information.
 *
 * Synopsis:    Main util for generating a TFS (Tacked-on File System)
 * Authors:     Steven Phillips
 *
 * variable-length-* are +ve integers comprised of a sequence of bytes, the lower 7 bits
 *               are part of the number and the 8th/top bit indicates another byte to follow
 *               if set. These numbers are always little-endian in layout.
 * vll           variable-length-length
 * vlb           variable-length-back - length from start of current tag to the start of given data
 * vls           vl-string is a vll followed by a utf8 encoded string of given length, no \0
 *               terminator is given.
 * 
 * TFS file structure:
 * 
 *   head-tag:   "tfs", uint8 file format version
 *   File-1:     file-data
 *   File-2:     file-data
 *   ...
 *   File-n:     file-data
 *   dir-n:      dir-data for last sub-dir of last sub-dir etc
 *   ...
 *   dir-1:      dir-data for dir-1
 *   dir-2:      dir-data
 *   ...
 *   dir-dc:     dir-data
 *   Root-dir:   vll dir count (dc)
 *               vll file count (fc)
 *               dir-1:   vll length of vlb + <dname1>
 *                        vlb to dir-data
 *                        <dname1> (no null terminator)
 *               dir-2:   vll, vlb to dir-data, <dname2>
 *               ...
 *               dir-dc:  data for last sub-dir of dir
 *               file-1:  vll length of uint8 + vll [ + vlb ] [ + vll ] + <fname1>
 *                        uint8 flags
 *                        vll of file-data
 *                      [ vlb to file-data - not present if file length is 0 ]
 *                      [ vll of uncompressed file-size - not present if not compressed ]
 *                        <fname1> (no null terminator)
 *               file-2:  vll, ubyte, vll [, vlb ] [, vll ], <fname2>
 *               ...
 *               file-fc: data for last file of dir
 *   tail-tag:   uint8 file format version
 *               vlb to head-tag
 *               vlb to root directory
 *               uint32 tfs timestamp
 *               "TFS"
 *   tail-tag-len: uint8 len of tail-tag
 * 
 * The tail-tag-len is guaranteed to be 1 byte, so the last 4 bytes will always be "TFS"+len
 * 
 * Given max file length is limited (for ease) to uint32 the current maximum length of
 * the tail-tag is (1+5+5+4+3+1)=19, if sizes were increased to uint64 the max would be
 * (1+9+9+4+3+1)=27, so reading the last 32 bytes in the file will always read the
 * whole tail-tag.
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN             /* Get rid of the usual Microsoft junk */
#include <windows.h>                    /* Standard windows API */
#else
#ifdef _DIRENT
#include <dirent.h>
#else
#include <sys/dir.h>
#endif
#endif

#include "tfs.h"
#include "Lzma2Enc.h"
#define tfsLZMA2_PROP 0x18

static tfsUByte tfsHeadStr[4] = { 't', 'f', 's', tfsVERSION };

typedef struct tfsUFile
{
    struct tfsUFile *next;
    tfsULong posStart;
    tfsULong fileLen;
    tfsULong compLen;
    tfsUInt nameLen;
    tfsUByte flags;
    tfsUByte name[1];
} tfsUFile;
typedef struct tfsUDirectory
{
    struct tfsUDirectory *next;
    struct tfsUDirectory *childHead;
    struct tfsUDirectory *childTail;
    struct tfsUFile *fileHead;
    struct tfsUFile *fileTail;
    tfsULong posStart;
    tfsUInt nameLen;
    tfsUByte name[1];
} tfsUDirectory;

tfsUFile *fileCur;
FILE *fileReadFp;
FILE *fileWriteFp;

int
tfsUIntToVll(tfsUByte *buf, tfsUInt nn)
{
    tfsUByte bb[8];
    int ii,jj;
    
    if(nn < 0x080)
    {
        buf[0] = nn;
        return 1;
    }
    ii = 0;
    do
    {
        bb[ii++] = nn & 0x7f;
        nn >>= 7;
    } while(nn & ~0x07f);
    buf[0] = 0x80 | nn;
    jj = 1;
    while(--ii > 0)
        buf[jj++] = 0x80 | bb[ii];
    buf[jj++] = bb[0];
    return jj;
}
int
tfsULongToVll(tfsUByte *buf, tfsULong nn)
{
    tfsUByte bb[16];
    int ii,jj;
    if(nn < 0x080)
    {
        buf[0] = (tfsUByte) nn;
        return 1;
    }
    ii = 0;
    do
    {
        bb[ii++] = nn & 0x7f;
        nn >>= 7;
    } while(nn & ~0x07f);
    buf[0] = 0x80 | (tfsUByte) nn;
    jj = 1;
    while(--ii > 0)
        buf[jj++] = 0x80 | bb[ii];
    buf[jj++] = bb[0];
    return jj;
}
static void *
tfsAlloc(ISzAllocPtr ap, size_t size)
{
    return malloc(size);
}
static void
tfsFree(ISzAllocPtr ap, void *addr)
{
    if(addr != NULL)
        free(addr);
}
static ISzAlloc _allocFuncs = {
    tfsAlloc, tfsFree
};
static SRes
tfsAddFileRead(ISeqInStreamPtr is, void *bufP, size_t *size)
{
    size_t ll=*size;
    ll = fread(bufP,1,ll,fileReadFp);
    fileCur->fileLen += ll;
    *size = ll;
    /*    printf("tfsISeqOutStreamRead called: %d -> %d\n",ds,tfsInSz);*/
    return SZ_OK;
}
static ISeqInStream _inStream = {
    tfsAddFileRead
};
static size_t
tfsAddFileLzmaWrite(ISeqOutStreamPtr os, const void *ibp, size_t size)
{
    size_t ws;
    
    if((ws = fwrite(ibp,1,size,fileWriteFp)) != size)
    {
        printf("tfsAddFileLzmaWrite Error: Write failed returning %d\n",(int) ws);
        return 0;
    }
    fileCur->compLen += size;
    return size;
}
static ISeqOutStream _outStream = {
    tfsAddFileLzmaWrite
};

tfsUFile *
tfsAddFile(tfsUByte *name, tfsUByte *path)
{
    tfsUFile *fil;
    struct stat statbuf;
    tfsUByte buf[2048];
    tfsULong fLen;
    int ll;
    printf("Adding file: %s\n",path);
    if(stat((char *) path,&statbuf) != 0)
    {
        printf("Failed to get info on file [%s]\n",path);
        return NULL;
    }
    fLen = statbuf.st_size;
    ll = strlen((char *) name);
    if((fil=malloc(sizeof(tfsUFile)+ll)) == NULL)
    {
        printf("ERROR: Failed to malloc file for [%s]\n",path);
        return NULL;
    }
    fil->next = NULL;
#ifdef _WIN32
    fil->flags = ((statbuf.st_mode & S_IREAD) ? tfsSTMODE_READ:0)|((statbuf.st_mode & S_IWRITE) ? tfsSTMODE_WRITE:0)|((statbuf.st_mode & S_IEXEC) ? tfsSTMODE_EXECUTE:0);
#else
    fil->flags = ((statbuf.st_mode & S_IRUSR) ? tfsSTMODE_READ:0)|((statbuf.st_mode & S_IWUSR) ? tfsSTMODE_WRITE:0)|((statbuf.st_mode & S_IXUSR) ? tfsSTMODE_EXECUTE:0);
#endif
    fil->compLen = 0;
    fil->fileLen = 0;
    fil->nameLen = ll;
    memcpy(fil->name,name,ll+1);
    if(fLen == 0)
        return fil;
    
    if((fileReadFp = fopen((char *) path,"rb")) == NULL)
    {
        printf("Failed to open [%s]\n",path);
        free(fil);
        return NULL;
    }
    fil->posStart = ftell(fileWriteFp);
    if(fLen > 20)
    {
        CLzma2EncHandle pp;
        CLzma2EncProps props;
        Byte wp;
        SRes res;
        
        fileCur = fil;
        fil->flags |= tfsSTMODE_COMPRESSED;
        Lzma2EncProps_Init(&props);
#if 0
        /*    props.lzmaProps.level = level;*/
        /*    props.lzmaProps.dictSize = 1 << 22;*/
        if(fLen >= (1 << 20))
            props.dictSize = 1 << 20; // 1mb dictionary
        else
            props.dictSize = 1 << 16; // smaller dictionary = faster!
#endif    
        if((pp = Lzma2Enc_Create(&_allocFuncs,&_allocFuncs)) == NULL)
            printf("tfsAddFile Error: Failed to create CLzma2Enc\n");
        else
        {
            if((res = Lzma2Enc_SetProps(pp,&props)) != SZ_OK)
                printf("tfsAddFile Error: Failed to set CLzma2Enc props (%d)\n",res);
            else
            {
                Lzma2Enc_SetDataSize(pp,fil->fileLen);
                if((wp = Lzma2Enc_WriteProperties(pp)) != tfsLZMA2_PROP)
                    printf("tfsAddFile Error: Enc Write props wrong (%d,%d)\n",wp,tfsLZMA2_PROP);
                else if((res = Lzma2Enc_Encode2(pp,&_outStream,NULL,0,&_inStream,NULL,0,NULL)) != SZ_OK)
                    printf("tfsAddFile Error: Encode failed during file [%s] %d\n",path,res);
                else
                {
                    printf("tfsAddFile Success: [%s] %02x  %ld -> %ld\n",path,wp,(long) fileCur->fileLen,(long) fileCur->compLen);
                    ll = 0;
                }
            }
            Lzma2Enc_Destroy(pp);
        }
    }
    else
    {
        while((ll=fread(buf,1,2048,fileReadFp)) > 0)
        {
            if(fwrite(buf,1,ll,fileWriteFp) != ll)
            {
                printf("tfsAddFile Error: Failed to write\n");
                break;
            }
            fil->fileLen += ll;
        }
    }
    if((ll == 0) && (fLen < fil->fileLen))
    {
        printf("tfsAddFile Error: Failed to read [%s]\n",path);
        ll = -1;
    }
    fclose(fileReadFp);
    if(!ll)
        return fil;
    free(fil);
    return NULL;
}

#ifdef _WIN32
tfsUDirectory *
tfsAddDirectory(tfsUByte *name, tfsUByte *path)
{
    tfsUDirectory *dir, *dd;
    tfsUFile *ff;
    HANDLE *fh;
    WIN32_FIND_DATA fd;
    tfsUByte *fn;
    BOOL rr;
    int ll;
    
    printf("Adding dir:  %s\n",path);
    ll = strlen((char *) name);
    fn = name+ll;
    strcpy((char *) fn,"\\*.*") ;
          
    /* Process the directory */
    if((fh = FindFirstFile((char *) path,&fd)) == INVALID_HANDLE_VALUE)
    {
        printf("ERROR: Failed to open directory [%s]\n",path);
        return NULL;
    }
    if((dir=malloc(sizeof(tfsUDirectory)+ll)) == NULL)
    {
        printf("ERROR: Failed to malloc directory for [%s]\n",path);
        return NULL;
    }
    dir->next = NULL;
    dir->childHead = dir->childTail = NULL;
    dir->fileHead = dir->fileTail = NULL;
    dir->nameLen = ll;
    memcpy(dir->name,name,ll);
    dir->name[ll] = '\0';
    fn++;
    rr = TRUE;
    do
    {
        if((fd.cFileName[0] == '.') && ((fd.cFileName[1] == '\0') || ((fd.cFileName[1] == '.') && (fd.cFileName[2] == '\0'))))
            continue;
        if(strlen(fd.cFileName) >= tfsNAME_MAX)
        {
            printf("ERROR: Directory object name too long [%s]\n",fd.cFileName);
            break;
        }
        strcpy((char *) fn,(char *) fd.cFileName);
        
        if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            if((dd = tfsAddDirectory(fn,path)) == NULL)
                break;
            if(dir->childHead == NULL)
                dir->childHead = dd;
            else
                dir->childTail->next = dd;
            dir->childTail = dd;
        }
        else
        {
            if((ff = tfsAddFile(fn,path)) == NULL)
                break;
            if(dir->fileHead == NULL)
                dir->fileHead = ff;
            else
                dir->fileTail->next = ff;
            dir->fileTail = ff;
        }
    } while((rr=FindNextFile(fh,&fd)));
    FindClose(fh);
    
    if(rr)
        return NULL;
    return dir;
}
#else
tfsUDirectory *
tfsAddDirectory(tfsUByte *name, tfsUByte *path)
{
    tfsUDirectory *dir, *dd;
    tfsUFile *ff;
    DIR    *dirp;
#if (defined _DIRENT)
    struct  dirent *dp;
#else
    struct  direct *dp;
#endif
    struct stat sbuf;
    tfsUByte *fn;
    int ll;
    
    printf("Adding dir:  %s\n",path);
    if((dirp = opendir((char *) path)) == NULL)
    {
        printf("ERROR: Failed to open directory [%s]\n",path);
        return NULL;
    }
    ll = strlen((char *) name);
    if((dir=malloc(sizeof(tfsUDirectory)+ll)) == NULL)
    {
        printf("ERROR: Failed to malloc directory for [%s]\n",path);
        return NULL;
    }
    dir->next = NULL;
    dir->childHead = dir->childTail = NULL;
    dir->fileHead = dir->fileTail = NULL;
    dir->nameLen = ll;
    memcpy(dir->name,name,ll+1);
    fn = name+ll;
    *fn++ = '/';
    while((dp = readdir(dirp)) != NULL)
    {
        if((dp->d_name[0] == '.') && ((dp->d_name[1] == '\0') || ((dp->d_name[1] == '.') && (dp->d_name[2] == '\0'))))
            continue;
        if(strlen(dp->d_name) >= tfsNAME_MAX)
        {
            printf("ERROR: Directory object name too long [%s]\n",dp->d_name);
            break;
        }
        strcpy((char *) fn,(char *) dp->d_name);
        
        stat((char *) path,&sbuf);
        if(S_ISDIR(sbuf.st_mode))
        {
            if((dd = tfsAddDirectory(fn,path)) == NULL)
                break;
            if(dir->childHead == NULL)
                dir->childHead = dd;
            else
                dir->childTail->next = dd;
            dir->childTail = dd;
        }
        else
        {
            if((ff = tfsAddFile(fn,path)) == NULL)
                break;
            if(dir->fileHead == NULL)
                dir->fileHead = ff;
            else
                dir->fileTail->next = ff;
            dir->fileTail = ff;
        }
    }
    closedir(dirp);
    if(dp != NULL)
        return NULL;
    return dir;
}
#endif

int
tfsWriteDirectory(tfsUDirectory *dir)
{
    tfsUDirectory *sd;
    tfsUFile *ff;
    tfsULong ll, ps;
    tfsUInt dCnt, fCnt;
    tfsUByte *bb, buf[2048];
    int ln;
    
    /* must first write out all sub-dirs so their dir-data location is known */
    dCnt = 0;
    sd = dir->childHead;
    while(sd != NULL)
    {
        if(tfsWriteDirectory(sd))
            return -1;
        dCnt++;
        sd = sd->next;
    }
    fCnt = 0;
    ff = dir->fileHead;
    while(ff != NULL)
    {
        fCnt++;
        ff = ff->next;
    }
    dir->posStart = ps = ftell(fileWriteFp);
    ln = tfsUIntToVll(buf,dCnt);
    ln += tfsUIntToVll(buf+ln,fCnt);
    if(fwrite(buf,1,ln,fileWriteFp) != ln)
    {
        printf("tfsWriteDirectory Error: Failed to write dir counts\n");
        return -1;
    }
    sd = dir->childHead;
    while(sd != NULL)
    {
        ll = ps - sd->posStart; 
        ln = tfsULongToVll(buf+2,ll);
        memcpy(buf+ln+2,sd->name,sd->nameLen);
        ln += sd->nameLen;
        if(ln > 0x07f)
        {
            bb = buf;
            buf[0] = 0x80 | (ln >> 7);
            buf[1] = (ln & 0x7f);
            ln += 2;
        }
        else
        {
            bb = buf+1;
            buf[1] = ln;
            ln++;
        }
        if(fwrite(bb,1,ln,fileWriteFp) != ln)
        {
            printf("tfsWriteDirectory Error: Failed to write dir dir-data for [%s]\n",sd->name);
            return -1;
        }
        sd = sd->next;
    }
    ff = dir->fileHead;
    while(ff != NULL)
    {
        bb = buf+2;
        ln = 0;
        bb[ln++] = ff->flags;
        ln += tfsULongToVll(bb+ln,ff->fileLen);
        if(ff->fileLen)
        {
            ll = ps - ff->posStart;
            ln += tfsULongToVll(bb+ln,ll);
            if(ff->flags & tfsSTMODE_COMPRESSED)
                ln += tfsULongToVll(bb+ln,ff->compLen);
        }
        memcpy(bb+ln,ff->name,ff->nameLen);
        ln += ff->nameLen;
        if(ln > 0x07f)
        {
            bb -= 2;
            buf[0] = 0x80 | (ln >> 7);
            buf[1] = ln & 0x07f;
            ln += 2;
        }
        else
        {
            bb--;
            bb[0] = ln;
            ln++;
        }
        if(fwrite(bb,1,ln,fileWriteFp) != ln)
        {
            printf("tfsWriteDirectory Error: Failed to write dir file-data for [%s]\n",ff->name);
            return -1;
        }
        ff = ff->next;
    }
    return 0;
}

int
tfsWriteHead(void)
{
    /* tfs header is minimal, just an ID, as the data size is unknown at this stage, so would have
     * to either leave space for the largest possible size or except that read will always move to
     * end of file to locate the info block and dir structure */
    if(fwrite(tfsHeadStr,1,4,fileWriteFp) != 4)
    {
        printf("tfsWriteHead Error: Failed to write\n");
        return -1;
    }
    return 0;
}
int
tfsWriteTail(tfsUDirectory *rdir, tfsULong locHead)
{
    tfsULong ll, ps;
    tfsUInt tm;
    tfsUByte buf[64];
    int ln=0;
    
    ps = ftell(fileWriteFp);
    buf[ln++] = tfsVERSION;
    ll = ps - locHead;
    ln += tfsULongToVll(buf+ln,ll);
    ll = ps - rdir->posStart; 
    ln += tfsULongToVll(buf+ln,ll);
    tm = (tfsUInt) time(NULL);
    buf[ln++] = (tm >> 24) & 0x0ff;
    buf[ln++] = (tm >> 16) & 0x0ff;
    buf[ln++] = (tm >> 8) & 0x0ff;
    buf[ln++] = tm & 0x0ff;
    buf[ln++] = 'T';
    buf[ln++] = 'F';
    buf[ln++] = 'S';
    ln += tfsUIntToVll(buf+ln,ln);
    if(fwrite(buf,1,ln,fileWriteFp) != ln)
    {
        printf("tfsWriteDirectory Error: Failed to write tail-tag\n");
        return -1;
    }
    return 0;
}

int
main(int argc, char *argv[])
{
    tfsUDirectory *rdir;
    tfsULong locHead;
    tfsUByte *dn, buf[4096];
    int rr;
    
    if((argc != 3) && (argc != 4))
    {
        printf("Usage Error: %s <tfs> <dir-name> [ <base-file-name> ]\n",argv[0]);
        return 1;
    }
    else if((fileWriteFp = fopen(argv[1],"wb")) == NULL)
    {
        printf("tfs Error: Failed to create output file [%s]\n",argv[1]);
        return 1;
    }
    if(argc == 4)
    {
        FILE *fp;
        if((fp = fopen(argv[3],"rb")) == NULL)
        {
            printf("tfs Error: Failed to open base file [%s]\n",argv[3]);
            return 1;
        }
        while((rr=fread(buf,1,4096,fp)) > 0)
        {
            if(fwrite(buf,1,rr,fileWriteFp) != rr)
            {
                printf("tfs Error: Failed to write base file\n");
                break;
            }
        }
        if(!feof(fp))
        {
            printf("tfs Error: Failed to read base file [%s]\n",argv[3]);
            return 1;
        }
        fclose(fp);
    }
    strcpy((char *) buf,argv[2]);
    dn = buf;
    while((dn = (tfsUByte *) strrchr((char *) dn,'\\')) != NULL)
        *dn++ = '/';
    if(((dn = (tfsUByte *) strrchr((char *) buf,'/')) != NULL) && (dn[1] == '\0'))
        dn[1] = '\0';
    if((dn = (tfsUByte *) strrchr((char *) buf,'/')) == NULL)
        dn = buf;
    else
        dn++;
    
    locHead = ftell(fileWriteFp);
    if((rr=tfsWriteHead()) == 0)
    {
        if((rdir=tfsAddDirectory(dn,buf)) == NULL)
            rr = -1;
        else if((rr=tfsWriteDirectory(rdir)) == 0)
            rr = tfsWriteTail(rdir,locHead);
    }
    fclose(fileWriteFp);
#ifndef _WIN32
    if(argc == 4)
    {
        struct stat statbuf;
        if(stat(argv[3],&statbuf) == 0)
            chmod(argv[1],statbuf.st_mode);
    }
#endif
    return rr;
}
