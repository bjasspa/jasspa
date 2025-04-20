/* -*- C -*- *****************************************************************
 *
 * Copyright (C) 2025 JASSPA (www.jasspa.com).
 *
 * This is part of JASSPA's MicroEmacs, see the LICENSE file for licensing and
 * copying information.
 *
 * Synopsis:    Main library for reading a TFS (Tacked-on File System)
 * Authors:     Steven Phillips
 *
 *****************************************************************************/

#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "tfs.h"
#include "Lzma2Dec.h"

#ifndef NDEBUG
#define TFS_DEBUG_ENABLE  1
#endif

#if TFS_DEBUG_ENABLE
#define TFS_DEBUG(x) printf x
#else
#define TFS_DEBUG(x) /* NULL */
#endif

#ifdef _WIN32
#define fseeko(a,b,c) fseek((a),(long) (b),(c))
#define ftello ftell
#endif

#define tfsLZMA2_PROP  0x18
#define tfsDECBUF_SIZE 8192
/* the start of this structure must remain the same as tfsFile */
typedef struct {
    FILE *fp;
    tfsULong fileLen;
    tfsULong fremain;
    tfsUByte mode;
    
    /* compression info */
    int bremain;
    int boffset;
    tfsULong cremain;
    CLzma2Dec dec;
    tfsUByte buf[tfsDECBUF_SIZE];
} tfsFileComp;

// TODO: SSP for not debug turn into a macro
int
tfsVllToUInt(tfsUByte *buf, tfsUInt *rVal)
{
    unsigned char cc, *bb=(unsigned char *) buf;
    tfsUInt nn=0;
    int ii=0;
    do
    {
        cc = bb[ii++];
        nn = (nn << 7) | (cc & 0x7f);
    } while(cc & 0x80);
    *rVal = nn;
    return ii;
}
// TODO: SSP for not debug turn into a macro
int
tfsVllToULong(tfsUByte *buf, tfsULong *rVal)
{
    tfsUByte cc;
    tfsULong nn=0;
    int ii=0;
    do
    {
        cc = buf[ii++];
        nn = (nn << 7) | (cc & 0x7f);
    } while(cc & 0x80);
    *rVal = nn;
    return ii;
}

int
tfsVllToUIntf(FILE *fp, tfsUInt *rVal)
{
    tfsUInt nn=0;
    int cc;
    do
    {
        if((cc = fgetc(fp)) == EOF)
            return -1;
        nn = (nn << 7) | (cc & 0x7f);
    } while(cc & 0x80);
    *rVal = nn;
    return 0;
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
static ISzAlloc tfsAllocFuncs = {
    tfsAlloc, tfsFree
};

int
tfs_statf(FILE *fp, tfsMount *tfs, tfsUByte *name, tfsStat *stt)
{
    tfsUByte *ss, *nn=name;
    tfsULong dep, decl, defl, curPos=tfs->rootPos;
    tfsUInt del, dc, fc;
    tfsUByte deb[tfsDIRENT_MAX];
    int ii;
    
    if(*nn == '\0')
    {
        stt->entPos = curPos;
        stt->compLen = 0;
        stt->fileLen = 0;
        stt->ctime = tfs->ctime;
        stt->mode = tfsSTMODE_READ|tfsSTMODE_WRITE|tfsSTMODE_EXECUTE|tfsSTMODE_DIRECTORY;
        return 0;
    }
    do
    {
        TFS_DEBUG(("  tfs_statf directory: %lld\n",curPos));
        if((ss = (tfsUByte *) strchr((char *) nn,'/')) != NULL)
            *ss = '\0';
        if((fseeko(fp,curPos,SEEK_SET) != 0) || tfsVllToUIntf(fp,&dc) || tfsVllToUIntf(fp,&fc))
        {
            TFS_DEBUG(("tfs_statf error: Invalid directory found \"%s?/%s\".\n",tfs->name,name));
            return -1;
        }
        while(dc)
        {
            if(tfsVllToUIntf(fp,&del) || (fread(deb,1,del,fp) != del))
            {
                TFS_DEBUG(("tfs_statf error: Failed to read direntd in \"%s?/%s\".\n",tfs->name,name));
                return -1;
            }
            deb[del] = '\0';
            ii = tfsVllToULong(deb,&dep);
            TFS_DEBUG(("    tfs_statf direntd: %s\n",deb+ii));
            if(!strcmp((char *) (deb+ii),(char *) nn))
                break;
            dc--;
        }
        if(ss != NULL)
            *ss++ = '/';
        if(dc > 0)
        {
            if((ss == NULL) || (*ss == '\0'))
            {
                stt->entPos = curPos-dep;
                stt->compLen = 0;
                stt->fileLen = 0;
                stt->ctime = tfs->ctime;
                stt->mode = tfsSTMODE_READ|tfsSTMODE_WRITE|tfsSTMODE_EXECUTE|tfsSTMODE_DIRECTORY;
                return 0;
            }
            curPos = curPos-dep;
        }
        else if(ss != NULL)
            /* nn is not a directory so if this is not the leaf, i.e. a file, then its not found */
            return -1;
        else
        {
            while(fc)
            {
                if(tfsVllToUIntf(fp,&del) || (fread(deb,1,del,fp) != del))
                {
                    TFS_DEBUG(("tfs_statf error: Failed to read direntf in \"%s?/%s\".\n",tfs->name,name));
                    return -1;
                }
                deb[del] = '\0';
                ii = 1;
                ii += tfsVllToULong(deb+ii,&defl);
                if(defl > 0)
                {
                    ii += tfsVllToULong(deb+ii,&dep);
                    if(deb[0] & tfsSTMODE_COMPRESSED)
                        ii += tfsVllToULong(deb+ii,&decl);
                }   
                TFS_DEBUG(("    tfs_statf direntf: %s (%lld,%02x)\n",deb+ii,defl,deb[0]));
                if(!strcmp((char *) (deb+ii),(char *) nn))
                {
                    stt->entPos = (defl > 0) ? (curPos-dep):0;
                    stt->compLen = (deb[0] & tfsSTMODE_COMPRESSED) ? decl:0;
                    stt->fileLen = defl;
                    stt->ctime = tfs->ctime;
                    stt->mode = deb[0]|tfsSTMODE_FILE;
                    return 0;
                }
                fc--;
            }
            return -1;
        }
    } while((nn = ss)[0] != '\0');
    return -1;
}

tfsMount *
tfs_mount(tfsUByte *name)
{
    tfsUByte buf[32];
    tfsMount *tfs=NULL;
    tfsULong ul, tailPos;
    FILE *fp;
    int ii, ll;
    
    TFS_DEBUG(("tfs_mount (%s)\n",name));
    
    if((fp = fopen((char *) name,"rb")) == NULL)	
        TFS_DEBUG(("tfs_mount error: Failed to open [%s]\n",name));
    else
    {
        if((fseeko(fp,-32,SEEK_END) != 0) || (fread(buf,1,32,fp) != 32))
        {
            TFS_DEBUG(("tfs_mount error: Failed to read tail of [%s]\n",name));
        }
        else if((buf[28] != 'T') || (buf[29] != 'F') || (buf[30] != 'S') || (buf[31] > 31) || (buf[(ii=31-buf[31])] != tfsVERSION))
        {
            TFS_DEBUG(("tfs_mount error: Invalid tail in [%s]\n",name));
        }
        else
        {
            tailPos = ftello(fp)-1-buf[31];
            ll = strlen((char *) name);
            if((tfs = malloc(sizeof(tfsMount) + ll)) != NULL)
            {
                memcpy(tfs->name,name,ll+1);
                ii++;
                ii += tfsVllToULong(buf+ii,&ul);
                tfs->headPos = tailPos-ul;
                ii += tfsVllToULong(buf+ii,&ul);
                tfs->rootPos = tailPos-ul;
                tfs->ctime = (((tfsUInt) buf[ii]) << 24) | (((tfsUInt) buf[ii+1]) << 16) | (((tfsUInt) buf[ii+2]) << 8) | ((tfsUInt) buf[ii+3]);
            }
        }
        fclose(fp);
    }
    return tfs;
}

void
tfs_umount(tfsMount *tfs)
{
    TFS_DEBUG(("tfs_umount (%s)\n",tfs->name));
    if(tfs != NULL)
        free(tfs);
}

int
tfs_stat(tfsMount *tfs, tfsUByte *name, tfsStat *stt)
{
    FILE *fp;
    int rr;
    
    if(*name == '/')
        name++;
    TFS_DEBUG(("tfs_stat (tfs://%s?/%s)\n",tfs->name,name));

    if((fp = fopen((char *) tfs->name,"rb")) == NULL)
    {
        TFS_DEBUG(("tfs_stat error: Cannot open tfs \"%s\".\n",tfs->name));
        return -1;
    }
    rr = tfs_statf(fp,tfs,name,stt);
    fclose(fp);
    return rr;
}

int
tfs_type(tfsMount *tfs, tfsUByte *name)
{
    tfsStat stt;
    FILE *fp;
    int rr;
    
    if(*name == '/')
        name++;
    TFS_DEBUG(("tfs_type (tfs://%s?/%s)\n",tfs->name,name));

    if((fp = fopen((char *) tfs->name,"rb")) == NULL)
    {
        TFS_DEBUG(("tfs_type error: Cannot open tfs \"%s\".\n",tfs->name));
        return 0;
    }
    rr = tfs_statf(fp,tfs,name,&stt);
    fclose(fp);
    return (rr < 0) ? 0:stt.mode;
}

tfsDirectory *
tfs_dopen(tfsMount *tfs, tfsUByte *name)
{
    tfsStat stt;
    tfsDirectory *dp;
    tfsUInt dc, fc;
    FILE *fp;
    
    if(*name == '/')
        name++;
    
    TFS_DEBUG(("tfs_dopen (tfs://%s?/%s)\n",tfs->name,name));

    if((fp = fopen((char *) tfs->name,"rb")) == NULL)
    {
        TFS_DEBUG(("tfs_dopen error: Cannot open tfs \"%s\".\n",tfs->name));
        return NULL;
    }
    if((tfs_statf(fp,tfs,name,&stt)) != 0)
    {
        TFS_DEBUG(("tfs_dopen error: Cannot find node \"%s?/%s\".\n",tfs->name,name));
    }
    else if((stt.mode & tfsSTMODE_DIRECTORY) == 0)
    {
        TFS_DEBUG(("tfs_dopen error: Node is not a directory \"%s?/%s\".\n",tfs->name,name));
    }
    else if((fseeko(fp,stt.entPos,SEEK_SET) != 0) || tfsVllToUIntf(fp,&dc) || tfsVllToUIntf(fp,&fc))
    {
        TFS_DEBUG(("tfs_dopen error: Node is not a directory \"%s?/%s\".\n",tfs->name,name));
    }
    else if((dp = (tfsDirectory *) malloc(sizeof(tfsDirectory))) != NULL)
    {
        dp->fp = fp;
        dp->dirPos = stt.entPos;
        dp->ctime = stt.ctime;
        dp->dcount = dp->dremain = dc;
        dp->fcount = dp->fremain = fc;
        return dp;
    }
    fclose(fp);
    return NULL;
}

void
tfs_dclose(tfsDirectory *dp)
{
    TFS_DEBUG(("tfs_dclose\n"));
    fclose(dp->fp);
    free(dp);
}

int
tfs_dread(tfsDirectory *dp,tfsDirent *dirent)
{
    tfsUByte deb[tfsDIRENT_MAX];
    tfsULong dep, decl, defl;
    tfsUInt del;
    int ii, ll;
    
    if(dp->dremain)
    {
        dp->dremain--;
        if(tfsVllToUIntf(dp->fp,&del) || (fread(deb,1,del,dp->fp) != del))
        {
            TFS_DEBUG(("tfs_dread error: Failed to read direntd.\n"));
            return -1;
        }
        ii = tfsVllToULong(deb,&dep);
        ll = del-ii;
        dirent->entPos = dp->dirPos-dep;
        dirent->compLen = 0;
        dirent->fileLen = 0;
        dirent->ctime = dp->ctime;
        dirent->mode = tfsSTMODE_READ|tfsSTMODE_WRITE|tfsSTMODE_EXECUTE|tfsSTMODE_DIRECTORY;
        dirent->nameLen = ll;
        memcpy(dirent->name,deb+ii,ll);
        dirent->name[ll] = '\0';
        return 1;
    }
    if(dp->fremain)
    {
        dp->fremain--;
        if(tfsVllToUIntf(dp->fp,&del) || (fread(deb,1,del,dp->fp) != del))
        {
            TFS_DEBUG(("tfs_dread error: Failed to read direntf.\n"));
            return -1;
        }
        ii = 1;
        ii += tfsVllToULong(deb+ii,&defl);
        if(defl > 0)
        {
            ii += tfsVllToULong(deb+ii,&dep);
            if(deb[0] & tfsSTMODE_COMPRESSED)
                ii += tfsVllToULong(deb+ii,&decl);
        }
        ll = del-ii;
        dirent->entPos = (defl > 0) ? (dp->dirPos-dep):0;
        dirent->compLen = (deb[0] & tfsSTMODE_COMPRESSED) ? decl:0;
        dirent->fileLen = defl;
        dirent->ctime = dp->ctime;
        dirent->mode = deb[0]|tfsSTMODE_FILE;
        dirent->nameLen = ll;
        memcpy(dirent->name,deb+ii,ll);
        dirent->name[ll] = '\0';
        return 1;
    }
    return 0;
}

tfsFile *
tfs_fopen(tfsMount *tfs, tfsUByte *name)
{
    tfsStat stt;
    tfsFile *fl;
    FILE *fp;
    
    if(*name == '/')
        name++;
    
    TFS_DEBUG(("tfs_fopen (tfs://%s?/%s)\n",tfs->name,name));

    if((fp = fopen((char *) tfs->name,"rb")) == NULL)
    {
        TFS_DEBUG(("tfs_fopen error: Cannot open tfs \"%s\".\n",tfs->name));
        return NULL;
    }
    if((tfs_statf(fp,tfs,name,&stt)) != 0)
    {
        TFS_DEBUG(("tfs_fopen error: Cannot find node \"%s?/%s\".\n",tfs->name,name));
    }
    else if(stt.mode & tfsSTMODE_DIRECTORY)
    {
        TFS_DEBUG(("tfs_fopen error: Node is a directory \"%s?/%s\".\n",tfs->name,name));
    }
    else if(fseeko(fp,stt.entPos,SEEK_SET) != 0)
    {
        TFS_DEBUG(("tfs_fopen error: Cannot move to start of file \"%s?/%s\".\n",tfs->name,name));
    }
    else if((fl = (tfsFile *) malloc((stt.mode & tfsSTMODE_COMPRESSED) ? sizeof(tfsFileComp):sizeof(tfsFile))) != NULL)
    {
        fl->fp = fp;
        fl->mode = stt.mode;
        fl->fileLen = stt.fileLen;
        fl->fremain = stt.fileLen;
        if(stt.mode & tfsSTMODE_COMPRESSED)
        {
            tfsFileComp *fc = (tfsFileComp *) fl;
            fc->bremain = 0;
            fc->cremain = stt.compLen;
        
            Lzma2Dec_Construct(&(fc->dec));
            Lzma2Dec_Allocate(&(fc->dec),tfsLZMA2_PROP,&tfsAllocFuncs);
            Lzma2Dec_Init(&(fc->dec));
        }
        return fl;
    }
    fclose(fp);
    return NULL;
}

void
tfs_fclose(tfsFile *fl)
{
    TFS_DEBUG(("tfs_fclose\n"));
    if(fl->mode & tfsSTMODE_COMPRESSED)
    {
        tfsFileComp *fc = (tfsFileComp *) fl;
        Lzma2Dec_Free(&(fc->dec),&tfsAllocFuncs);
    }
    fclose(fl->fp);
    free(fl);
}

size_t
tfs_fread(void *dest, size_t size, tfsFile *fl)
{
    tfsULong fr;
    size_t rd;
    
    if(fl->mode & tfsSTMODE_COMPRESSED)
    {
        tfsFileComp *fc = (tfsFileComp *) fl;
        SizeT sz=size,sl,dl;
        ELzmaStatus status;
        SRes res;
        
        for(;;)
        {
            if((sl=fc->bremain) > 0)
            {
                dl = sz;
                if((res = Lzma2Dec_DecodeToBuf(&(fc->dec),dest,&dl,fc->buf+fc->boffset,&sl,LZMA_FINISH_ANY,&status)) != SZ_OK)
                {
                    TFS_DEBUG(("tfs_fread error: Decode returned error (%d).\n",res));
                    return -1;
                }
                if((fc->bremain -= sl) > 0)
                    fc->boffset += sl;
                if(dl == sz)
                    return size;
                dest = ((tfsUByte *) dest)+dl;
                sz -= dl;
            }
            sl = tfsDECBUF_SIZE;
            if(fc->bremain > 0)
            {
                tfsUByte *p1, *p2;
                int ii;
                if(fc->boffset == 0)
                {
                    TFS_DEBUG(("tfs_fread error: Decode buffer too small.\n"));
                    return -1;
                }
                ii = fc->bremain;
                sl -= ii;
                p1 = fc->buf;
                p2 = p1+fc->boffset;
                do {
                    *p1++ = *p2++;
                } while(--ii > 0);
            }
            fc->boffset = 0;
            if((sl > fc->cremain) && ((sl = (SizeT) fc->cremain) == 0))
            {
                TFS_DEBUG(("tfs_fread error: Reached end of compressed data.\n"));
                return (size-sz);
            }
            if((rd=fread(fc->buf+fc->bremain,1,sl,fl->fp)) <= 0)
            {
                TFS_DEBUG(("tfs_fread error: Failed to read compressed data.\n"));
                return -1;
            }
            fc->cremain -= rd;
            fc->bremain += rd;
        }
    }
    if((fr=fl->fremain) == 0)
        return 0;
    if(fr < size)
        size = (size_t) fr;
    if((rd=fread(dest,1,size,fl->fp)) > 0)
        fl->fremain -= rd;
    return rd;
}
