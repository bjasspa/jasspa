/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * file.c - File handling.
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
 * Synopsis:    File handling.
 * Authors:     Unknown, Jon Green & Steven Phillips
 * Description:
 *     The routines in this file handle the reading, writing
 *     and lookup of disk files.  All of details about the
 *     reading and writing of the disk are in "fileio.c".
 */

#define __FILEC 1                  /* Define the filename */

#include "emain.h"
#include "efunc.h"
#include "eskeys.h"
#include "esearch.h"
#if (defined _UNIX) || (defined _DOS) || (defined _WIN32)
#include <limits.h>                     /* Constant limit definitions */
#include <sys/types.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>                     /* Directory entries */
/* number of seconds from 1 Jan. 1601 00:00 to 1 Jan 1970 00:00 UTC */
#define EPOCH_DIFF 11644473600LL
#else
#ifdef _DIRENT
#include <dirent.h>
#else
#include <sys/dir.h>
#endif
#endif
#endif

#ifdef _DOS
#include <sys/file.h>
#include <dir.h>
#include <pc.h>
#include <dos.h>

/* attribute stuff */
#define A_RONLY         0x01
#define A_HIDDEN        0x02
#define A_SYSTEM        0x04
#define A_LABEL         0x08
#define A_DIR           0x10
#define A_ARCHIVE       0x20

/* dos call values */
#define DOSI_GETDRV     0x19
#define DOSI_GETDIR     0x47

#endif  /* _DOS */

#ifdef _UNIX
#include <utime.h>

#define statTestRead(st)                                                   \
((((st).st_uid == meUid) && ((st).st_mode & S_IRUSR)) ||                   \
 ((st).st_mode & S_IROTH) ||                                               \
 (((st).st_mode & S_IRGRP) &&                                              \
  (((st).st_gid == meGid) || (meGidSize && meGidInGidList((st).st_gid)))))
#define statTestWrite(st)                                                  \
((((st).st_uid == meUid) && ((st).st_mode & S_IWUSR)) ||                   \
 ((st).st_mode & S_IWOTH) ||                                               \
 (((st).st_mode & S_IWGRP) &&                                              \
  (((st).st_gid == meGid) || (meGidSize && meGidInGidList((st).st_gid)))))
#define statTestExec(st)                                                   \
((((st).st_uid == meUid) && ((st).st_mode & S_IXUSR)) ||                   \
 ((st).st_mode & S_IXOTH) ||                                               \
 (((st).st_mode & S_IXGRP) &&                                              \
  (((st).st_gid == meGid) || (meGidSize && meGidInGidList((st).st_gid)))))
#endif

/* Min length of root */
#ifdef _DRV_CHAR
#define _ROOT_DIR_LEN  3                /* 'c:\' */
#else
#define _ROOT_DIR_LEN  1                /* '/' */
#endif

#ifdef _UNIX
int
meFileTestDir(meUByte *fname)
{
    struct stat statbuf;
    return ((stat((char *)fname,&statbuf) != 0) || !S_ISDIR(statbuf.st_mode));
}
#endif

int
getFileStats(meUByte *file, int flag, meStat *stats, meUByte *lname)
{
    register int ft;
    if(lname != NULL)
        *lname = '\0' ;
    /* setup the default stat values */
    if(stats != NULL)
    {
        meFiletimeInit(stats->stmtime) ;
        stats->stsizeHigh = 0 ;
        stats->stsizeLow = 0 ;
#ifdef _UNIX
        stats->stuid  = meUid ;
        stats->stgid  = meGid ;
        stats->stdev  = -1 ;
        stats->stino  = -1 ;
#endif
    }
    ft = ffUrlGetType(file);
    if(ffUrlTypeIsHttpFtp(ft))
        return ft;
#if MEOPT_TFS
    if(ffUrlTypeIsTfs(ft))
    {
        tfsStat tfs_statbuf;
        tfsMount *tfsh;
        meUByte *ss;
        int st;

        if((ss=meStrstr(file,"?/")) != NULL)
        {
            *ss = '\0';
            tfsh = tfs_mount(file+6);
            *ss++ = '?';
        }
        else
        {
            tfsh = tfsdev;
            ss = file+5;
        }
        if(tfsh != NULL)
        {
            st = tfs_stat(tfsh,ss,&tfs_statbuf);
            if(tfsh != tfsdev)
                tfs_umount(tfsh);
        }
        else
            st = -1;
        if(st < 0)
        {
            if(ss[meStrlen(ss)-1] == DIR_CHAR)
            {
                if(flag & gfsERRON_DIR)
                    mlwrite(MWABORT|MWPAUSE,(meUByte *)"[%s directory]", file) ;
                ft |= meIOTYPE_DIRECTORY;
            }
            return (ft|meIOTYPE_NOTEXIST);
        }
        if(tfs_statbuf.mode & tfsSTMODE_DIRECTORY)
        {
            if(flag & gfsERRON_DIR)
                mlwrite(MWABORT|MWPAUSE,(meUByte *)"[%s directory]", file) ;
            ft |= meIOTYPE_DIRECTORY;
        }
        else
            ft |= meIOTYPE_REGULAR;
        if(stats != NULL)
        {
            /* Get the file size. */
            stats->stsizeLow = (meUInt) tfs_statbuf.fileLen;
            /* Set the permissions. */
            stats->stmode = 0;
            if(ft & meIOTYPE_DIRECTORY)
            {
#ifdef _DOS
                stats->stmode = FA_DIR;
#endif
#ifdef _WIN32
                stats->stmode = FILE_ATTRIBUTE_DIRECTORY;
#endif
#ifdef _UNIX
                stats->stmode = S_IXUSR|S_IXGRP|S_IXOTH|0040000;
#endif
            }
#ifdef _WIN32
            stats->stmode |= FILE_ATTRIBUTE_READONLY;
            {
                /* Convert the time to Windows format */
                meULong dt=(tfs_statbuf.ctime+EPOCH_DIFF)*10000000LL;
                stats->stmtime.dwLowDateTime = (DWORD) dt;
                stats->stmtime.dwHighDateTime = (DWORD) (dt>>32);
            }
#endif
#ifdef _DOS
            stats->stmode |= FA_RDONLY;
            stats->stmtime = tfs_statbuf.ctime;
#endif
#ifdef _UNIX
            /* NOTE: TFS does not support sym-links so nothing to be gained by trying to store stdev or stino, fname cmp will be enough */
            stats->stmode |= S_IRGRP|S_IROTH|S_IRUSR;
            if(ft & meIOTYPE_REGULAR)
                stats->stmode |= 0100000;
            stats->stmtime = tfs_statbuf.ctime;
#endif
        }
        return ft;
    }
#endif
    if(ffUrlTypeIsFile(ft))
        /* skip the file: */
        file += 5;

#ifdef _DOS
    {
        int ii ;

        if(((ii = meStrlen(file)) == 0) ||
           (strchr((char *) file,'*') != NULL) || (strchr((char *) file,'?') != NULL))
        {
            if(flag & gfsERRON_ILLEGAL_NAME)
                mlwrite(MWABORT|MWPAUSE,(meUByte *)"[%s illegal name]", file);
            return (ft|meIOTYPE_NOTEXIST);
        }
        if((file[ii-1] == DIR_CHAR) || ((ii == 2) && (file[1] == _DRV_CHAR)))
            goto gft_directory;

#ifdef __DJGPP2__
        ii = meFileGetAttributes(file) ;
        if(ii < 0)
            return (ft|meIOTYPE_NOTEXIST);
#else
        {
            union REGS reg ;                /* cpu register for use of DOS calls */
            reg.x.ax = 0x4300 ;
            reg.x.dx = ((unsigned long) file) ;
            intdos(&reg, &reg);

            if(reg.x.cflag)
                return (ft|meIOTYPE_NOTEXIST);
            ii = reg.x.cx ;
        }
#endif
        if(stats != NULL)
        {
            struct ffblk fblk ;
            stats->stmode = ii | meFILE_ATTRIB_ARCHIVE ;
            if(!findfirst((char *) file,&fblk,FA_RDONLY|FA_HIDDEN|FA_SYSTEM|FA_DIREC))
            {
                stats->stmtime = (((meUInt) fblk.ff_ftime) & 0x0ffff) | (((meUInt) fblk.ff_fdate) << 16) ;
                stats->stsizeLow = fblk.ff_fsize ;
            }
        }
        if(ii & meFILE_ATTRIB_DIRECTORY)
        {
gft_directory:
            if(flag & gfsERRON_DIR)
                mlwrite(MWABORT|MWPAUSE,(meUByte *)"[%s directory]", file) ;
            return (ft|meIOTYPE_DIRECTORY);
        }
        return (ft|meIOTYPE_REGULAR);
    }
#else
#ifdef _WIN32
    {
        DWORD status;
        int len;

        if(((len = meStrlen(file)) == 0) || (meStrchr(file,'*') != NULL) || (meStrchr(file,'?') != NULL))
        {
            if(flag & gfsERRON_ILLEGAL_NAME)
                mlwrite(MWABORT|MWPAUSE,(meUByte *)"[%s illegal name]", file);
            return (ft|meIOTYPE_NOTEXIST);
        }
        if(stats != NULL)
        {
            HANDLE *fh;
            WIN32_FIND_DATA fd;

            if(file[len-1] == DIR_CHAR)
            {
                meUByte fn[meBUF_SIZE_MAX];
                meStrcpy(fn,file);
                fn[len] = '.';
                fn[len+1] = '\0';
                if((fh = FindFirstFile((const char *) fn,&fd)) == INVALID_HANDLE_VALUE)
                {
                    if((file[1] == _DRV_CHAR) && ((len == 2) || ((len == 3) && (file[2] == DIR_CHAR))))
                    {
                        if((GetLogicalDrives() & (1 << ((file[0] | 0x20) - 'a'))) == 0)
                            ft |= meIOTYPE_NOTEXIST;
                        goto gft_directory;
                    }
                    else if((file[0] == DIR_CHAR) && (file[1] == DIR_CHAR))
                    {
                        /* shared paths can be funny about access to '.' (e.g. it could be top level drive), test any */
                        fn[len] = '*';
                        fh = FindFirstFile((const char *) fn,&fd);
                        if(fh == INVALID_HANDLE_VALUE)
                        {
                            ft |= meIOTYPE_NOTEXIST;
                            goto gft_directory;
                        }
                        else if((fd.cFileName[0] != '.') || (fd.cFileName[1] != '\0'))
                        {
                            /* findFirst not returned '.' so can't get stats, but it does exist */
                            FindClose(fh);
                            goto gft_directory;
                        }
                    }
                    else
                    {
                        ft |= meIOTYPE_NOTEXIST;
                        goto gft_directory;
                    }
                }
            }
            else if((fh = FindFirstFile((const char *) file,&fd)) == INVALID_HANDLE_VALUE)
            {
                ft |= meIOTYPE_NOTEXIST;
                return ft;
            }
            status = fd.dwFileAttributes;
            stats->stsizeHigh = fd.nFileSizeHigh;
            stats->stsizeLow = fd.nFileSizeLow;
            stats->stmtime.dwHighDateTime = fd.ftLastWriteTime.dwHighDateTime;
            stats->stmtime.dwLowDateTime = fd.ftLastWriteTime.dwLowDateTime;
            FindClose(fh);
            stats->stmode = (meUShort) status | FILE_ATTRIBUTE_ARCHIVE;
        }
        else if((file[1] == _DRV_CHAR) && ((len == 2) || ((len == 3) && (file[2] == DIR_CHAR))))
            goto gft_directory;
        else if(file[len-1] == DIR_CHAR)
        {
            file[len-1] = '\0';
            status = GetFileAttributes((const char *) file);
            file[len-1] = DIR_CHAR;
            if(status == 0xFFFFFFFF)
            {
                ft |= meIOTYPE_NOTEXIST;
                goto gft_directory;
            }
        }
        else
        {
            status = GetFileAttributes((const char *) file);
            if(status == 0xFFFFFFFF)
                return (ft|meIOTYPE_NOTEXIST);
        }
        if(status & FILE_ATTRIBUTE_DIRECTORY)
        {
gft_directory:
            if(flag & gfsERRON_DIR)
                mlwrite(MWABORT|MWPAUSE,(meUByte *)"[%s directory]", file);
            return (ft|meIOTYPE_DIRECTORY);
        }
        /* FILE_ATTRIBUTE_ARCHIVE
         * The file or directory is an archive file or directory.
         * Applications use this flag to mark files for backup or removal.
         *
         * FILE_ATTRIBUTE_HIDDEN
         * The file or directory is hidden.
         * It is not included in an ordinary directory listing.
         *
         * FILE_ATTRIBUTE_READONLY
         * The file or directory is read-only.
         * Applications can read the file but cannot write to it or delete it.
         * In the case of a directory, applications cannot delete it.
         *
         * FILE_ATTRIBUTE_SYSTEM
         * The file or directory is part of, or is used exclusively by,
         * the operating system.
         *
         * FILE_ATTRIBUTE_NORMAL
         * The file or directory has no other attributes set.
         * This attribute is valid only if used alone. */
        return (ft|meIOTYPE_REGULAR);
    }
#else
#ifdef _UNIX
    {
        struct stat statbuf;
        long stmtime = -1;
        int len;

        if((len = meStrlen(file)) == 0)
        {
            if(flag & gfsERRON_ILLEGAL_NAME)
                mlwrite(MWABORT|MWPAUSE,(meUByte *)"[%s illegal name]", file);
            return (ft|meIOTYPE_NOTEXIST);
        }
        if((lname == NULL) && (stats == NULL))
        {
            if(stat((char *)file,&statbuf) != 0)
                return ft | ((file[len-1] == DIR_CHAR) ? (meIOTYPE_DIRECTORY|meIOTYPE_NOTEXIST):meIOTYPE_NOTEXIST);
        }
        else if(lstat((char *)file, &statbuf) != 0)
            return ft | ((file[len-1] == DIR_CHAR) ? (meIOTYPE_DIRECTORY|meIOTYPE_NOTEXIST):meIOTYPE_NOTEXIST);
        else if(S_ISLNK(statbuf.st_mode))
        {
            meUByte lbuf[meBUF_SIZE_MAX], buf[meBUF_SIZE_MAX], *ss ;
            size_t ii, jj ;
            int maxi=10 ;

            ii = meStrlen(file);
            memcpy(lbuf,file,ii);
            do {
                if(file[ii-1] == DIR_CHAR)
                    ii--;
                lbuf[ii] = '\0';
                if(statbuf.st_mtime > stmtime)
                    stmtime = statbuf.st_mtime;
                if((jj=readlink((char *)lbuf,(char *)buf, meBUF_SIZE_MAX)) <= 0)
                {
                    if(flag & gfsERRON_BAD_FILE)
                        mlwrite(MWABORT|MWPAUSE,(meUByte *)"[%s symbolic link problems]", file);
                    return (ft|meIOTYPE_NASTY);
                }
                buf[jj] = '\0' ;
                if((buf[0] == DIR_CHAR) || ((ss=meStrrchr(lbuf,DIR_CHAR)) == NULL))
                    meStrcpy(lbuf,buf) ;
                else
                {
                    ss++ ;
                    ii = (size_t)(ss - lbuf) ;
                    meStrcpy(ss,buf) ;
                    ii += jj ;
                }
            } while(((jj=lstat((char *)lbuf, &statbuf)) == 0) && (--maxi > 0) && S_ISLNK(statbuf.st_mode)) ;

            if(lname != NULL)
            {
                fileNameCorrect(lbuf,lname,NULL) ;
                if(S_ISDIR(statbuf.st_mode))
                {
                    /* make sure that a link to a dir has a trailing '/' */
                    meUByte *ss=lname+meStrlen(lname) ;
                    if(ss[-1] != DIR_CHAR)
                    {
                        ss[0] = DIR_CHAR ;
                        ss[1] = '\0' ;
                    }
                }
            }
            if(jj)
            {
                if(flag & gfsERRON_BAD_FILE)
                    mlwrite(MWABORT|MWPAUSE,(meUByte *)"[%s dangling symbolic link]",lbuf);
                if(stats != NULL)
                    stats->stmtime = stmtime ;
                return (ft|meIOTYPE_NOTEXIST);
            }
        }

        if(stats != NULL)
        {
            stats->stmode = statbuf.st_mode;
            stats->stuid = statbuf.st_uid;
            stats->stgid = statbuf.st_gid;
            stats->stdev = statbuf.st_dev;
            stats->stino = statbuf.st_ino;
#ifdef _LARGEFILE_SOURCE
            stats->stsizeHigh = (meUInt) (statbuf.st_size >> 32);
            stats->stsizeLow = (meUInt) statbuf.st_size;
#else
            stats->stsizeLow = statbuf.st_size ;
#endif
            if(statbuf.st_mtime > stmtime)
                stats->stmtime= statbuf.st_mtime ;
            else
                stats->stmtime= stmtime ;
        }
        if(S_ISREG(statbuf.st_mode))
            return (ft|meIOTYPE_REGULAR);
        if(S_ISDIR(statbuf.st_mode))
        {
            if(flag & gfsERRON_DIR)
                mlwrite(MWABORT|MWPAUSE,(meUByte *)"[%s directory]", file);
            return (ft|meIOTYPE_DIRECTORY);
        }
        if(flag & gfsERRON_BAD_FILE)
        {
#ifdef S_IFIFO
            if(S_ISFIFO(statbuf.st_mode))
                mlwrite(MWABORT|MWPAUSE,(meUByte *)"[%s is a FIFO]", file);
#endif
#ifdef S_IFCHR
            else if(S_ISCHR(statbuf.st_mode))
                mlwrite(MWABORT|MWPAUSE,(meUByte *)"[%s is character special]", file);
#endif
#ifdef S_IFBLK
            else if(S_ISBLK(statbuf.st_mode))
                mlwrite(MWABORT|MWPAUSE,(meUByte *)"[%s is block special]", file);
#endif
            else
                mlwrite(MWABORT|MWPAUSE,(meUByte *)"[%s not a regular file]", file);
        }
        return (ft|meIOTYPE_NASTY);
    }
#else
    return (ft|meIOTYPE_REGULAR);
#endif /* _UNIX */
#endif /* _WIN32 */
#endif /* _DOS */
}

int
fnamecmp(meUByte *f1, meUByte *f2)
{
    if((f1==NULL) || (f2==NULL))
        return 1 ;
#ifdef _INSENSE_CASE
    return meStricmp(f1,f2);
#else
    return meStrcmp(f1,f2);
#endif
}

/* Search the directory and subdirectories for MicroEmacs macro directories, don't get 2 of any path type
 *
 * Flags:
 *   0x01 Add the given base path
 *   0x02 Add <base-path>/<user-sub-path>
 *   0x04 Add <base-path>/<std-sub-paths>
 *   0x08 Is program name so check for <...>/bin/ or <...>/bin/<spath>/ and if found check for <...>/macros etc
 *
 * gotPaths: 0x01 Spelling, 0x02 Macros, 0x04 Company, 0x08 User-path
 */
int
mePathAddSearchPath(int index, meUByte *path_name, meUByte *path_base, int flags, int *gotPathsP)
{
    /* Common sub-directories of JASSPAs MicroEmacs */
    static meUByte *subdirs[] =
    {
        (meUByte *) "spelling",         /* Spelling dictionaries */
        (meUByte *) "macros",           /* Standard distribution macros */
        (meUByte *) "company",          /* Company wide files */
    } ;
    meUByte cc, *ss, base_name[meBUF_SIZE_MAX];
    int ii, jj, ll, mm, gotPaths=*gotPathsP;

    /* Iterate over all of the paths */
    while(*path_base != '\0')
    {
        /* Construct the base name */
        ll = 0;
        while((cc = *path_base) != '\0')
        {
            path_base++;
            if(cc == mePATH_CHAR)
                break;
            base_name[ll++] = cc;
        }
        /* Clean up any trailing directory characters */
        if(ll < _ROOT_DIR_LEN)
            continue;
        if(base_name[ll-1] == DIR_CHAR)
            ll--;
        base_name[ll] = '\0';
        if(getFileStats(base_name,0,NULL,NULL) & meIOTYPE_DIRECTORY)
        {
            /* If this is the program path, in a <bpth>/bin/ or <bpth>/bin/sub-dir/ and <bpth>/macros exists then add <bpth>/<sub-dirs> to the search path */
            if((flags & 8) && ((gotPaths & 0x0f) != 0x0f) && (ll > 4) &&
               (!memcmp((ss=base_name+ll-4),"/bin",4) || (((ss=meStrrchr(base_name+4,DIR_CHAR)) != NULL) && !memcmp((ss-=4),"/bin",4))))
            {
                meUByte sp[256];
                meStrcpy(sp,++ss);
                meStrcpy(ss,subdirs[1]);
                if(!meTestDir(base_name))
                {
                    *ss = '\0';
                    index = mePathAddSearchPath(index,path_name,base_name,6,&gotPaths);
                }
                meStrcpy(ss,sp);
            }
            base_name[ll++] = DIR_CHAR;
            /* check for base_name/$user-name first */
            if((flags & 2) && !(gotPaths & 8) && (meUserName != NULL))
            {
                meStrcpy(base_name+ll,meUserName);
                if(getFileStats(base_name,0,NULL,NULL) & meIOTYPE_DIRECTORY)
                {
                    /* it exists, add it to the front as we haven't got a user path yet */
                    gotPaths |= 8;
                    jj = ll + meStrlen(meUserName);
                    if(index)
                    {
                        base_name[jj++] = mePATH_CHAR;
                        memcpy(base_name+jj,path_name,index);
                    }
                    memcpy(path_name,base_name,jj+index);
                    index += jj;
                }
            }
            if((flags & 4) && ((gotPaths & 0x07) != 0x07))
            {
                /* Append the search paths if necessary. We construct the standard JASSPA MicroEmacs
                 * paths and then test for the existance of the directory. If the directory exists then
                 * we add it to the search path. We do not add any directories to the search path that
                 * do not exist. */
                ii = 2;
                mm = 4;
                do {
                    if(!(gotPaths & mm))
                    {
                        jj = meStrlen(subdirs[ii]);
                        memcpy(base_name+ll,subdirs[ii],jj+1);
                        /* Test the directory for existance, if it does not exist then do not add it as we
                         * do not want to search any directory unecessarily. */
                        if(getFileStats(base_name,0,NULL,NULL) & meIOTYPE_DIRECTORY)
                        {
                            /* it exists, add it */
                            gotPaths |= mm;
                            if(index)
                                path_name[index++] = mePATH_CHAR;
                            jj += ll;
                            memcpy(path_name+index,base_name,jj);
                            index += jj;
                        }
                    }
                    mm >>= 1;
                } while(--ii >= 0);
            }
            if(flags & 1)
            {
                ll--;
                if(index)
                    path_name[index++] = mePATH_CHAR;
                memcpy(path_name+index,base_name,ll);
                index += ll;
            }
        }
        if(gotPaths & 0x02)
            break;
    }
    *gotPathsP = gotPaths;
    path_name[index] = '\0';
    return index;
}

/* Look up the existance of a file along the normal or PATH
 * environment variable. Look first in the HOME directory if
 * asked and possible
 */

int
fileLookup(meUByte *fname, int extCnt, meUByte **extLst, meUByte flags, meUByte *outName)
{
    register meUByte *path;  /* environmental PATH variable */
    register meUByte *sp;    /* pointer into path spec */
    register int ii, jj, fl;
    meUByte buf[meBUF_SIZE_MAX];

    if(extCnt)
    {
        fl = meStrlen(fname);
        jj = extCnt;
        while(--jj >= 0)
            if(((ii=fl - meStrlen(extLst[jj])) >= 0) &&
#ifdef _INSENSE_CASE
               !meStricmp(fname+ii,extLst[jj])
#else
               !meStrcmp(fname+ii,extLst[jj])
#endif
               )
            {
                extCnt = 0;
                break;
            }
    }
    /* if meFL_CHECKPATH and fname has a path/drive char then this is an absolute or relative
     * pathed fname, if not then this must be searched for only, correct flags appropriately */
    if((flags & meFL_CHECKPATH) && ((meStrchr(fname,DIR_CHAR) != NULL)
#ifdef _CONVDIR_CHAR
       || (meStrchr(fname,_CONVDIR_CHAR) != NULL)
#endif
#ifdef _DRV_CHAR
       || (meStrchr(fname,_DRV_CHAR) != NULL)
#endif
       ))
        flags = (flags & ~(meFL_USESRCHPATH|meFL_USEPATH)) | meFL_CHECKDOT;
    if(flags & meFL_CHECKDOT)
    {
        fileNameCorrect(fname,outName,NULL);
        if(extCnt)
        {
            ii = meStrlen(outName);
            for(jj=0 ; jj<extCnt ; jj++)
            {
                if((extLst[jj][0] == DIR_CHAR) && (outName[ii-1] == DIR_CHAR))
                   meStrcpy(outName+ii,&(extLst[jj][1]));
                else
                   meStrcpy(outName+ii,extLst[jj]);
#if MEOPT_TFS
                if(ffUrlTypeIsTfs(ffUrlGetType(outName)))
                {
                    if((sp=(meUByte *) strstr((char *) outName,"?/")) != NULL)
                    {
                        tfsMount *tfsh;
                        int tt;
                        *sp = '\0';
                        tfsh = tfs_mount(outName+6);
                        *sp++ = '?';
                        if(tfsh != NULL)
                        {
                            tt = tfs_type(tfsh,sp);
                            tfs_umount(tfsh);
                            if(tt & tfsSTMODE_FILE)
                                return 1;
                        }
                    }
                    else if((tfsdev != NULL) && (tfs_type(tfsdev,outName+5) & tfsSTMODE_FILE))
                        return 1;
                }
                else
#endif
#ifdef _UNIX
                if(flags & meFL_EXEC)
                {
                    if(!meTestExec(outName) && (getFileStats(outName,0,NULL,NULL) & meIOTYPE_REGULAR))
                        return 1;
                }
                else
#endif
                    if(!meTestExist(outName))
                    return 1;
            }
        }
#if MEOPT_TFS
        else if(ffUrlTypeIsTfs(ffUrlGetType(outName)))
        {
            if((sp=(meUByte *) strstr((char *) outName,"?/")) != NULL)
            {
                tfsMount *tfsh;
                int tt;
                *sp = '\0';
                tfsh = tfs_mount(outName+6);
                *sp++ = '?';
                if(tfsh != NULL)
                {
                    tt = tfs_type(tfsh,sp);
                    tfs_umount(tfsh);
                    if(tt & tfsSTMODE_FILE)
                        return 1;
                }
            }
            else if((tfsdev != NULL) && (tfs_type(tfsdev,outName+5) & tfsSTMODE_FILE))
                return 1;
        }
#endif
#ifdef _UNIX
        else if(flags & meFL_EXEC)
        {
            if(!meTestExec(outName) && (getFileStats(outName,0,NULL,NULL) & meIOTYPE_REGULAR))
                return 1 ;
        }
#endif
        else if(!meTestExist(outName))
            return 1 ;
    }
    if(flags & meFL_USEPATH)
        path = meGetenv("PATH");
    else if(flags & meFL_USESRCHPATH)
        path = searchPath;
    else
        path = NULL;
    while((path != NULL) && (*path != '\0'))
    {
        /* build next possible file spec */
        sp = path;
#if MEOPT_TFS
#if mePATH_CHAR == ':'
        /* Special case: if the path starts with tfs: then assume its a tfs path rather than just 'tfs' */
        if((path[0] == 't') && (path[1] == 'f') && (path[2] == 's') && (path[3] == ':'))
            path += 4;
#endif
#endif
        if((path = meStrchr(path,mePATH_CHAR)) != NULL)
        {
            ii = path++ - sp ;
            memcpy(buf,sp,ii) ;
        }
        else
        {
            meStrcpy(buf,sp) ;
            ii = meStrlen(buf) ;
        }
        /* Check for zero length path */
        if(ii)
        {
            /* Add a directory separator if missing */
#ifdef _CONVDIR_CHAR
            if ((buf[ii-1] != DIR_CHAR) && (buf[ii-1] != _CONVDIR_CHAR))
#else
            if (buf[ii-1] != DIR_CHAR)
#endif
                buf[ii++] = DIR_CHAR;
            meStrcpy(buf+ii,fname);
            /* and try it out */
            //fileNameCorrect(buf,outName,NULL) ;
            if(extCnt)
            {
                ii += fl;
                for(jj=0 ; jj<extCnt ; jj++)
                {
                    meStrcpy(buf+ii,extLst[jj]);
#if MEOPT_TFS
                    if(ffUrlTypeIsTfs(ffUrlGetType(buf)))
                    {
                        int tt=0;
                        if((sp=(meUByte *) strstr((char *) buf,"?/")) != NULL)
                        {
                            tfsMount *tfsh;
                            *sp = '\0';
                            tfsh = tfs_mount(buf+6);
                            *sp++ = '?';
                            if(tfsh != NULL)
                            {
                                tt = tfs_type(tfsh,sp);
                                tfs_umount(tfsh);
                            }
                        }
                        else if(tfsdev != NULL)
                            tt = tfs_type(tfsdev,buf+5);
                        if(tt & tfsSTMODE_FILE)
                        {
                            fileNameCorrect(buf,outName,NULL);
                            return 1;
                        }
                    }
                    else
#endif
#ifdef _UNIX
                        if(flags & meFL_EXEC)
                    {
                        if(!meTestExec(buf) && (getFileStats(buf,0,NULL,NULL) & meIOTYPE_REGULAR))
                        {
                            fileNameCorrect(buf,outName,NULL);
                            return 1;
                        }
                    }
                    else
#endif
                        if(!meTestExist(buf))
                    {
                        fileNameCorrect(buf,outName,NULL);
                        return 1;
                    }
                }
            }
#if MEOPT_TFS
            else if(ffUrlTypeIsTfs(ffUrlGetType(buf)))
            {
                int tt=0;
                if((sp=(meUByte *) strstr((char *) buf,"?/")) != NULL)
                {
                    tfsMount *tfsh;
                    *sp = '\0';
                    tfsh = tfs_mount(buf+6);
                    *sp++ = '?';
                    if(tfsh != NULL)
                    {
                        tt = tfs_type(tfsh,sp);
                        tfs_umount(tfsh);
                    }
                }
                else if(tfsdev != NULL)
                    tt = tfs_type(tfsdev,buf+5);
                if(tt & tfsSTMODE_FILE)
                {
                    fileNameCorrect(buf,outName,NULL);
                    return 1;
                }
            }
#endif
#ifdef _UNIX
            else if(flags & meFL_EXEC)
            {
                if(!meTestExec(buf) && (getFileStats(buf,0,NULL,NULL) & meIOTYPE_REGULAR))
                {
                    fileNameCorrect(buf,outName,NULL);
                    return 1;
                }
            }
#endif
            else if(!meTestExist(buf))
            {
                fileNameCorrect(buf,outName,NULL);
                return 1;
            }
        }
    }
#if MEOPT_CALLBACK
    if(flags & meFL_CALLBACK)
    {
        meUInt arg;
        if((ii=decode_key(ME_SPECIAL|SKEY_find_file,&arg)) != -1)
        {
            meStrcpy(buf,resultStr);
            meStrcpy(resultStr,fname);
            if((execFuncHidden(ME_SPECIAL|SKEY_find_file,ii,arg) > 0) && !meTestExist(resultStr))
            {
                fileNameCorrect(resultStr,outName,NULL);
                ii = 1;
            }
            else
                ii = 0;
            meStrcpy(resultStr,buf);
            return ii;
        }
    }
#endif
    return 0; /* no such luck */
}

int
executableLookup(meUByte *fname, meUByte *outName)
{
#if (defined _WIN32) || (defined _DOS)
    if(fileLookup(fname,extExecCnt,extExecLst,meFL_CHECKDOT|meFL_CHECKPATH|meFL_USEPATH,outName))
        return 1;
#else
    if(fileLookup(fname,0,NULL,meFL_CHECKPATH|meFL_USEPATH|meFL_EXEC,outName))
        return 1;
#endif
    return 0;
}

/* return: 0 if no file, or its not changeable, can't get timestamp or file has not changed, -1 if file deleted, 1 of older, 2 if newer */
int
bufferOutOfDate(meBuffer *bp)
{
    meStat stats;
    int ft;
    if(bp->fileName == NULL)
        return 0;
    ft = getFileStats(bp->fileName,0,&stats,NULL);
    if(ft & (meIOTYPE_TFS|meIOTYPE_HTTP|meIOTYPE_FTP|meIOTYPE_FTPE))
        return 0;
    if(ft & meIOTYPE_NOTEXIST)
        return ((meFiletimeIsSet(bp->stats.stmtime)) ? -1:0);
    if(meFiletimeIsSame(stats.stmtime,bp->stats.stmtime))
        return 0;
    return ((meFiletimeIsModified(stats.stmtime,bp->stats.stmtime)) ? 2:1);
}

/* get current working directory
 */
meUByte *
gwd(meUByte drive)
{
    /*
     * Get the current working directory into a static area and return
     * a pointer to it.
     *
     * This routine accounts for the differences between BSD getwd() and
     * System V getcwd().
     *
     * Return NULL on error or if we dont have a routine for the current
     * system (true for VMS, MS-DOS, etc etc until someone writes them).
     */
#if (defined _UNIX) || (defined _DOS) || (defined _WIN32)

    meUByte dir[meBUF_SIZE_MAX] ;
    register int ll ;

#ifdef _UNIX
    meGetcwd(dir,meBUF_SIZE_MAX-2) ;
#endif  /* _UNIX */

#ifdef _WIN32
    /* Change drive to the destination drive.
     * remeber so that we can get back. Note we
     * do no test for fails on the drive. We will
     * pick up the current working directory if
     * it fails. */
    if (drive != 0)
    {
        int curDrive;

        curDrive = _getdrive ();
        drive = toUpper(drive) - 'A' + 1;
        if ((drive == curDrive) || (_chdrive(drive) != 0))
            drive = 0;             /* Failed drive change or same drive */
        else
            drive = curDrive;      /* Save to restore */
    }

    /* Pick up the directory information */
    GetCurrentDirectory (meBUF_SIZE_MAX,(char *) dir);
    if (meStrlen(dir) > 2)
    {
        meUByte *p;                   /* Local character pointer */

        /* convert all '\\' to '/' */
        p = dir+2 ;
        while((p=meStrchr(p,'\\')) != NULL)    /* got a '\\', -> '/' */
            *p++ = DIR_CHAR ;
    }
    /* change the drive back - dont care if it fails cos theres nothing we can do! */
    if (drive != 0)
        _chdrive (drive) ;
#endif /* _WIN32 */

#ifdef _DOS
#ifdef __DJGPP2__
    unsigned int curDrive=0, newDrive, availDrives ;

    if(drive != 0)
    {
        _dos_getdrive(&curDrive) ;
        newDrive = toLower(drive) - 'a' + 1 ;
        if(newDrive == curDrive)
            _dos_setdrive(newDrive,&availDrives) ;
        else
            /* already current drive */
            curDrive = 0;
    }

    if(getcwd((char *) dir,meBUF_SIZE_MAX) == NULL)
        dir[0] = '\0' ;
    if(curDrive != 0)
        /* change the drive back */
        _dos_setdrive(curDrive,&availDrives) ;
    fileNameConvertDirChar(dir) ;
    makestrlow(dir) ;
#else
    union REGS reg;                /* cpu register for use of DOS calls */

    if(drive == 0)
    {
        reg.h.ah = DOSI_GETDRV ;
        intdos (&reg, &reg);
        drive = reg.h.al + 'a' ;
    }
    else if(drive < 'a')
        drive += 'a' - 'A' ;
    dir[0] = drive ;
    dir[1] = ':' ;
    dir[2] = DIR_CHAR ;

    reg.h.ah = DOSI_GETDIR ;
    reg.h.dl = drive - 'a' + 1 ;
    reg.x.si = (unsigned long) (dir+3) ;
    (void) intdos(&reg, &reg);
#endif
#endif  /* _DOS */

    if((ll = meStrlen(dir)) > 0)
    {
        if(dir[ll-1] != DIR_CHAR)
        {
            dir[ll] = DIR_CHAR ;
            dir[ll+1] = '\0' ;
        }
        return meStrdup(dir) ;
    }

#endif  /* UNKNOWN */
    /*
    ** An unknown system to me - dont know what to do here.
    */
    return NULL ;

}

meUByte *
getFileBaseName(meUByte *fname)
{
    meUByte cc, *p, *p1 ;
    p = p1 = fname ;
    while((cc=*p1++) != '\0')
    {
        if((cc == DIR_CHAR) && (*p1 != '\0'))
            p = p1 ;
    }
    return p ;
}

void
getFilePath(meUByte *fname, meUByte *path)
{
    if(fname != NULL)
    {
        meUByte *ss ;
        meStrcpy(path,fname) ;
        if((ss=meStrrchr(path,DIR_CHAR)) == NULL)
            path[0] = '\0' ;
        else
            ss[1] = '\0' ;
    }
    else
        meStrcpy(path,curdir) ;
}

int
inputFileName(meUByte *prompt, meUByte *fn, int corFlag)
{
    meUByte tmp[meBUF_SIZE_MAX], *buf ;
    int s, ll ;

    buf = (corFlag) ? tmp:fn ;

    getFilePath(frameCur->windowCur->buffer->fileName,buf) ;
    s = meGetString(prompt,(MLFILECASE|MLNORESET|MLMACNORT), 0, buf, meBUF_SIZE_MAX) ;
    if(s > 0)
    {
        if(((ll = meStrlen(buf)) > 0) && (buf[ll-1] == '.') &&
           ((ll == 1) || (buf[ll-2] == '/') || (buf[ll-2] == '\\') ||
            ((buf[ll-2] == '.') &&
             ((ll == 2) || (buf[ll-3] == '/') || (buf[ll-3] == '\\')))))
        {
            buf[ll] = '/' ;
            buf[ll+1] = '\0' ;
        }
        if(corFlag)
            fileNameCorrect(tmp,fn,NULL) ;
    }
    return s ;
}


#if (defined _WIN32) || (defined _DOS)
int
meTestExecutable(meUByte *fileName)
{
    int ii ;
    if((ii = meStrlen(fileName)) > 4)
    {
        meUByte *ee ;
        ee = fileName+ii-4 ;
        ii = extExecCnt;
        while(--ii >= 0)
#ifdef _INSENSE_CASE
            if(!meStrnicmp(ee,extExecLst[ii],4))
#else
            if(!meStrncmp(ee,extExecLst[ii],4))
#endif
                return 1 ;
    }
    return 0 ;
}
#endif

#if MEOPT_DIRLIST
#define meFINDFILESINGLE_GFS_OPT (gfsERRON_ILLEGAL_NAME|gfsERRON_BAD_FILE)
#define FILENODE_ATTRIBLEN 4

typedef struct FILENODE {
    struct FILENODE *next;                  /* Next node in the list */
    meUByte  *fname;                        /* Name of the file */
    meUByte  *lname;                        /* Linkname */
    meUByte   attrib[FILENODE_ATTRIBLEN];
    meUInt    sizeHigh;
    meUInt    sizeLow;
#ifdef _UNIX
    time_t  mtime;
#endif
#ifdef _DOS
    time_t  mtime;
#endif
#ifdef _WIN32
    FILETIME mtime;
#endif
} FILENODE;

FILENODE *curHead;

static FILENODE *
addFileNode(FILENODE *cfh, FILENODE *cf)
{
    FILENODE *pf, *nf ;
    if(cfh == NULL)
    {
        cf->next = NULL ;
        return cf ;
    }
    pf = NULL ;
    nf = cfh ;
    if(cf->attrib[0] != 'd')
    {
        /* skip all the directories */
        while((nf != NULL) && (nf->attrib[0] == 'd'))
        {
            pf = nf ;
            nf = nf->next ;
        }
        /* now find the correct alpha place */
        while((nf != NULL) && (meStrcmp(nf->fname,cf->fname) < 0))
        {
            pf = nf ;
            nf = nf->next ;
        }
    }
    else
    {
        /* now find the correct alpha place */
        while((nf != NULL) && (nf->attrib[0] == 'd') &&
              (meStrcmp(nf->fname,cf->fname) < 0))
        {
            pf = nf ;
            nf = nf->next ;
        }
    }
    cf->next = nf ;
    if(pf == NULL)
        return cf ;
    pf->next = cf ;
    return cfh ;
}

static FILENODE *
getDirectoryInfo(meUByte *fname)
{
    meUByte bfname[meBUF_SIZE_MAX];                 /* Working file name buffer */
    meUByte *fn;
    FILENODE *curFile;
    FILENODE *cfh=NULL;
    int noFiles=0;                        /* No files count */

#if MEOPT_TFS
    if(ffUrlTypeIsTfs(ffUrlGetType(fname)))
    {
        tfsMount *tfsh;
        meUByte *fp,*ff;

        /* Render the list of files. */
        curHead = NULL;
        meStrcpy(bfname,fname);
        if((fp=(meUByte *) strstr((char *) bfname,"?/")) != NULL)
        {
            *fp = '\0';
            tfsh = tfs_mount(bfname+6);
            *fp++ = '?';
        }
        else
        {
            tfsh = tfsdev;
            fp = bfname+5;
        }
        if(tfsh != NULL)
        {
            tfsDirectory *dirp;
            if((dirp = tfs_dopen(tfsh,fp)) != NULL)
            {
                tfsDirent de;
                int ll=dirp->dcount+dirp->fcount, isRoot=((fp[0]=='\0') || ((fp[0]=='/') && (fp[1]=='\0')));
                if((curHead = (FILENODE *) meMalloc(sizeof(FILENODE)*(ll+((isRoot) ? 0:1)))) == NULL)
                {
                    tfs_dclose(dirp);
                    if(tfsh != tfsdev)
                        tfs_umount(tfsh);
                    return NULL;
                }
                if(!isRoot && ((ff = meStrdup((meUByte *) "..")) != NULL))
                {
                    curFile = &(curHead[noFiles++]);
                    /* construct attribute string */
                    curFile->attrib[0] = 'd';
                    curFile->attrib[1] = 'r';
                    curFile->attrib[2] = 'w';
                    curFile->attrib[3] = 'x';
                    curFile->sizeHigh = 0;
                    curFile->sizeLow = 0;
                    curFile->lname = NULL;
                    curFile->fname = ff;
#ifdef _DOS
                    curFile->mtime = 0;
#else
#ifdef _WIN32

                    {
                        /* Convert the time to Windows format */
                        meULong dt=(dirp->ctime+EPOCH_DIFF)*10000000LL;
                        curFile->mtime.dwLowDateTime = (DWORD) dt;
                        curFile->mtime.dwHighDateTime = (DWORD) (dt>>32);
                    }
#else
                    curFile->mtime = dirp->ctime;
#endif
#endif
                }
                while(tfs_dread(dirp,&de) > 0)
                {
                    curFile = &(curHead[noFiles++]);
                    if((ff = meStrdup(de.name)) == NULL)
                    {
                        tfs_dclose(dirp);
                        if(tfsh != tfsdev)
                            tfs_umount(tfsh);
                        return NULL;
                    }
                    /* construct attribute string */
                    curFile->attrib[1] = (de.mode & tfsSTMODE_READ) ? 'r':'-';
                    curFile->attrib[2] = (de.mode & tfsSTMODE_WRITE) ? 'w':'-';
                    curFile->attrib[3] = (de.mode & tfsSTMODE_EXECUTE) ? 'x':'-';
                    if(de.mode & tfsSTMODE_DIRECTORY)
                    {
                        curFile->attrib[0] = 'd';
                        curFile->sizeHigh = 0;
                        curFile->sizeLow = 0;
                    }
                    else
                    {
                        curFile->attrib[0] = '-';
                        curFile->sizeHigh = (meUInt) (de.fileLen>>32);
                        curFile->sizeLow = (meUInt) (de.fileLen & 0xffffffff);
                    }                        
                    curFile->lname = NULL;
                    curFile->fname = ff;
#ifdef _DOS
                    curFile->mtime = 0;
#else
#ifdef _WIN32
                    {
                        /* Convert the time to Windows format */
                        meULong dt=(de.ctime+EPOCH_DIFF)*10000000LL;
                        curFile->mtime.dwLowDateTime = (DWORD) dt;
                        curFile->mtime.dwHighDateTime = (DWORD) (dt>>32);
                    }
#else
                    curFile->mtime = de.ctime;
#endif
#endif
                }
                tfs_dclose(dirp);
            }
            if(tfsh != tfsdev)
               tfs_umount(tfsh);
        }
    }
    else
#endif
    {
#ifdef _UNIX
        DIR    *dirp ;
#if (defined _DIRENT)
        struct  dirent *dp ;
#else
        struct  direct *dp ;
#endif
        struct stat sbuf ;
        meUByte *ff;
        int llen;

        /* Render the list of files. */
        curHead = NULL ;
        meStrcpy(bfname,fname) ;
        fn = bfname+meStrlen(bfname) ;

        if((dirp = opendir((char *)fname)) != NULL)
        {
            while((dp = readdir(dirp)) != NULL)
            {
                if(((noFiles & 0x0f) == 0) &&
                   ((curHead = (FILENODE *) meRealloc(curHead,sizeof(FILENODE)*(noFiles+16))) == NULL))
                    return NULL ;
                curFile = &(curHead[noFiles++]) ;
                if((ff = meStrdup((meUByte *) dp->d_name)) == NULL)
                    return NULL ;
                curFile->lname = NULL ;
                curFile->fname = ff ;
                meStrcpy(fn,dp->d_name) ;
                stat((char *)bfname,&sbuf) ;
                if(sbuf.st_uid != meUid)
                {
                    /* remove the user modes */
                    sbuf.st_mode &= ~00700 ;
                    if(sbuf.st_gid != meGid)
                        /* replace with group */
                        sbuf.st_mode |= ((sbuf.st_mode & 00070) << 3)  ;
                    else
                        /* replace with other */
                        sbuf.st_mode |= ((sbuf.st_mode & 00007) << 6)  ;
                }
#ifdef DT_LNK
                if(dp->d_type == DT_LNK)
#endif
                {
                    /* Check if its a symbolic link */
                    meUByte link[1024];
                    if((llen=readlink((char *)bfname,(char *)link,1024)) > 0)
                    {
                        link[llen] = '\0' ;
                        curFile->lname = meStrdup(link) ;
                    }
                }
                /* construct attribute string */
                if(S_ISREG(sbuf.st_mode))
                    curFile->attrib[0] = '-' ;
                else if(S_ISDIR(sbuf.st_mode))
                    curFile->attrib[0] = 'd' ;
                else
                    curFile->attrib[0] = 's' ;
                curFile->attrib[1] = (statTestRead(sbuf))  ? 'r' : '-' ;
                curFile->attrib[2] = (statTestWrite(sbuf)) ? 'w' : '-' ;
                curFile->attrib[3] = (statTestExec(sbuf))  ? 'x' : '-' ;
#ifdef _LARGEFILE_SOURCE
                curFile->sizeHigh = (meUInt) (sbuf.st_size >> 32) ;
                curFile->sizeLow = (meUInt) sbuf.st_size ;
#else
                curFile->sizeHigh = 0 ;
                curFile->sizeLow = (meUInt) sbuf.st_size ;
#endif
                curFile->mtime = sbuf.st_mtime ;
            }
            closedir(dirp) ;
        }
#endif
#ifdef _DOS
        static meUByte *searchString = (meUByte *) "*.*";
        struct ffblk fblk ;
        meUByte *ff ;
        int done ;

        /* Render the list of files. */
        curHead = NULL ;
        meStrcpy(bfname,fname) ;
        fn = bfname+meStrlen(bfname) ;
        meStrcpy(fn,searchString) ;
        done = findfirst((char *) bfname,&fblk,FA_RDONLY|FA_HIDDEN|FA_SYSTEM|FA_DIREC) ;
        while(!done)
        {
            if(((noFiles & 0x0f) == 0) &&
               ((curHead = (FILENODE *) meRealloc(curHead,sizeof(FILENODE)*(noFiles+16))) == NULL))
                return NULL ;
            curFile = &(curHead[noFiles++]) ;
            if((ff = meStrdup((meUByte *) fblk.ff_name)) == NULL)
                return NULL ;
            makestrlow(ff) ;
            curFile->fname = ff ;
            curFile->lname = NULL ;
            curFile->sizeHigh = 0 ;
            curFile->sizeLow  = (meUInt) fblk.ff_fsize ;
            curFile->mtime = (((meUInt) fblk.ff_ftime) & 0x0ffff) | (((meUInt) fblk.ff_fdate) << 16) ;
            /* construct attribute string */
            if(fblk.ff_attrib & FA_DIREC)
                memcpy(curFile->attrib,"drwx",4) ;
            else
            {
                curFile->attrib[0] = '-' ;
                curFile->attrib[1] = 'r' ;
                curFile->attrib[2] = (fblk.ff_attrib & FA_RDONLY) ? '-' : 'w' ;
                curFile->attrib[3] = '-' ;
                if(meTestExecutable(ff))
                    curFile->attrib[3] = 'x' ;
            }
            done = findnext(&fblk) ;
        }
#endif
#ifdef _WIN32
        static meUByte *searchString = (meUByte *) "*.*";
        HANDLE          *fh ;
        WIN32_FIND_DATA  fd ;
        meUByte           *ff ;

        /* Render the list of files. */
        curHead = NULL ;
        meStrcpy(bfname,fname) ;
        fn = bfname+meStrlen(bfname) ;
        meStrcpy(fn,searchString) ;
        if((fh = FindFirstFile((const char *) bfname,&fd)) != INVALID_HANDLE_VALUE)
        {
            do
            {
                if(((noFiles & 0x0f) == 0) &&
                   ((curHead = (FILENODE *) meRealloc(curHead,sizeof(FILENODE)*(noFiles+16))) == NULL))
                    return NULL ;
                curFile = &(curHead[noFiles++]) ;
                if((ff = meStrdup((meUByte *) fd.cFileName)) == NULL)
                    return NULL ;
                curFile->fname = ff ;
                curFile->lname = NULL ;
                curFile->mtime.dwLowDateTime  = fd.ftLastWriteTime.dwLowDateTime ;
                curFile->mtime.dwHighDateTime = fd.ftLastWriteTime.dwHighDateTime ;
                /* construct attribute string */
                memcpy(curFile->attrib,"-rwx",4) ;
                if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    curFile->attrib[0] = 'd' ;
                    /* On network drives the size is sometimes invalid. Clear
                     * it just to make sure that this is not the case */
                    curFile->sizeHigh = 0;
                    curFile->sizeLow = 0;
                }
                else
                {
                    if(fd.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
                        curFile->attrib[2] =  '-' ;
                    if(!meTestExecutable(ff))
                        curFile->attrib[3] = '-' ;
                    curFile->sizeHigh = (meUInt) fd.nFileSizeHigh ;
                    curFile->sizeLow = (meUInt) fd.nFileSizeLow ;
                }
            } while (FindNextFile(fh, &fd));
            FindClose(fh);
        }
#endif
    }
    curFile = curHead ;
    /* Build a profile of the current directory. */
    while(--noFiles >= 0)
    {
        cfh = addFileNode(cfh,curFile) ;
        curFile++ ;
    }
    return cfh ;
}

static int
readDirectory(meUByte *fname, meBuffer *bp, meLine *blp, meUInt flags)
{
    FILENODE *fnode, *fn ;
#ifdef _UNIX
    struct tm *tmp;
#endif
#ifdef _WIN32
    SYSTEMTIME tmp;
    FILETIME ftmp ;
#endif
    meUByte buf[meBUF_SIZE_MAX];                  /* Working line buffer */
    meUInt totSizeHigh=0, totSizeLow=0, ui ;
    int len, dirs=0, files=0 ;

    if((fnode=getDirectoryInfo(fname)) == NULL)
        return meABORT ;
    meStrcpy(buf,"Directory listing of: ") ;
    /* use the buffer's filename in preference to the given fname as the
     * buffer's filename is what the user asked for, fname may be what the
     * symbolic link points to */
    meStrcat(buf,(bp->fileName == NULL) ? fname:bp->fileName) ;
    bp->lineCount += addLine(blp,buf) ;
    fn = fnode ;
    while(fn != NULL)
    {
        totSizeHigh += fn->sizeHigh ;
        ui = totSizeLow + fn->sizeLow ;
        if((ui >> 16) < ((totSizeLow >> 16) + (fn->sizeLow >> 16)))
            totSizeHigh++ ;
        totSizeLow = ui ;
        if(fn->attrib[0] == 'd')
            dirs++ ;
        else
            files++ ;
        fn = fn->next ;
    }
    buf[0] = ' ' ;
    buf[1] = ' ' ;
    len = 2 ;
    if (totSizeHigh > 0)
    {
        ui = (totSizeHigh << 12) | (totSizeLow >> 20) ;
        len += sprintf((char *)buf+len, "%7dM ",ui) ;
    }
    else if (totSizeLow > 9999999)
        len += sprintf((char *)buf+len, "%7dK ",totSizeLow >> 10) ;
    else
        len += sprintf((char *)buf+len, "%7d  ",totSizeLow) ;
    sprintf((char *)buf+len,"used in %d files and %d dirs\n",files,dirs) ;
    bp->lineCount += addLine(blp,buf) ;
    while(fnode != NULL)
    {
        len = 0;                /* Reset to the start of the line */
        buf[len++] = ' ';
        meStrncpy(buf+len,fnode->attrib,FILENODE_ATTRIBLEN) ;
        len += FILENODE_ATTRIBLEN ;
        buf[len++] = ' ' ;
        /* Add the file statistics */
        if(fnode->sizeHigh > 0)
        {
            ui = (fnode->sizeHigh << 12) | (fnode->sizeLow >> 20) ;
            len += sprintf((char *)buf+len, "%7dM ",ui) ;
        }
        else if (fnode->sizeLow > 9999999)
            len += sprintf((char *)buf+len, "%7dK ", fnode->sizeLow >> 10);
        else
            len += sprintf((char *)buf+len, "%7d  ", fnode->sizeLow);

#ifdef _UNIX
        if ((tmp = localtime(&fnode->mtime)) != NULL)
            len += sprintf((char *)buf+len, "%4d-%02d-%02d %02d:%02d:%02d ",
                           tmp->tm_year+1900, tmp->tm_mon+1, tmp->tm_mday,
                           tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

        else
#endif
#ifdef _DOS
            if((fnode->mtime & 0x0ffff) != 0x7fff)
            len += sprintf((char *)buf+len, "%4d-%02d-%02d %02d:%02d:%02d ",
                           (int) ((fnode->mtime >> 25) & 0x007f)+1980,
                           (int) ((fnode->mtime >> 21) & 0x000f),
                           (int) ((fnode->mtime >> 16) & 0x001f),
                           (int) ((fnode->mtime >> 11) & 0x001f),
                           (int) ((fnode->mtime >>  5) & 0x003f),
                           (int) ((fnode->mtime & 0x001f)  << 1)) ;
        else
#endif
#ifdef _WIN32
            if(FileTimeToLocalFileTime(&fnode->mtime,&ftmp) &&
               FileTimeToSystemTime(&ftmp,&tmp))
            len += sprintf((char *)buf+len, "%4d-%02d-%02d %02d:%02d:%02d ",
                           tmp.wYear,tmp.wMonth,tmp.wDay,tmp.wHour,tmp.wMinute,tmp.wSecond) ;
        else
#endif
            len += sprintf((char *)buf+len, "XXXX-XX-XX XX:XX:XX ") ;

        len += sprintf ((char *)buf+len, "%s", fnode->fname);
        if(fnode->attrib[0] == 'd')
            buf[len++] = '/' ;

        if(fnode->lname != NULL)
        {
            sprintf((char *)buf+len, " -> %s", fnode->lname);
            free(fnode->lname) ;
        }
        else
            buf[len] = '\0' ;

        bp->lineCount += addLine(blp,buf) ;
        free(fnode->fname) ;
        fnode = fnode->next ;
    }
    free(curHead) ;
    if((flags & meRWFLAG_PRESRVFMOD) == 0)
    {
        bp->fileFlag = meBFFLAG_DIR;
        meModeSet(bp->mode,MDVIEW);
        meModeClear(bp->mode,MDAUTOSV);
        meModeClear(bp->mode,MDUNDO);
    }
    return meTRUE ;
}

#else
#define meFINDFILESINGLE_GFS_OPT (gfsERRON_ILLEGAL_NAME|gfsERRON_BAD_FILE|gfsERRON_DIR)
#endif

/*
 * Read file "fname" into the current
 * buffer, blowing away any text found there. Called
 * by both the read and find commands. Return the final
 * status of the read. Also called by the mainline,
 * to read in a file specified on the command line as
 * an argument. If the filename ends in a ".c", CMODE is
 * set for the current buffer.
 */
/* Note for Dynamic file names
 * readin sets the buffer file name to point to fname (freeing old if not
 * NULL and bp->fileName != fname) so fname should be dynamically allocated
 * or the file name reset after readin.
 */
/* bp        buffer to load into */
/* fname     name of file to read */
/* lockfl    check for file locks? */
/* frompipe  input coming from pipe. Dont open file */
int
readin(register meBuffer *bp, meUByte *fname)
{
    int   ss=meABORT ;
    meUByte lfn[meBUF_SIZE_MAX], afn[meBUF_SIZE_MAX], *fn=fname ;

#if MEOPT_CRYPT
    if(meCryptBufferInit(bp) <= 0)
        return meABORT;
#endif
    if(bp->fileName != fname)
    {
        meNullFree(bp->fileName) ;
        bp->fileName = fname ;
    }
    if(fn != NULL)
    {
#if MEOPT_EXTENDED
        meUInt ui ;
#endif
        meStat stats;
        int ft, aft;
        if((ft=getFileStats(fn,gfsERRON_ILLEGAL_NAME|gfsERRON_BAD_FILE,&(bp->stats),lfn)) & meIOTYPE_HTTP)
            meModeSet(bp->mode,MDVIEW);
        else if((ft & (meIOTYPE_FTP|meIOTYPE_FTPE)) == 0)
        {
            if(ft & meIOTYPE_NASTY)
                goto error_end ;
            if(lfn[0] != '\0')
                /* something was found, don't want to do RCS on this,
                 * the link may be dangling which returns 3 */
                fn = lfn ;
#if MEOPT_RCS
            else if((ft & meIOTYPE_NOTEXIST) && ((ft & (meIOTYPE_DIRECTORY|meIOTYPE_TFS)) == 0) && (rcsFilePresent(fn) > 0))
            {
                if(doRcsCommand(fn,rcsCoStr) <= 0)
                    goto error_end ;
                if((ft=getFileStats(fn,gfsERRON_ILLEGAL_NAME|gfsERRON_BAD_FILE,&(bp->stats),lfn)) & meIOTYPE_NASTY)
                    goto error_end ;
            }
#endif
            if((autoTime > 0) && !createBackupName(afn,fn,'#',0) &&
               ((aft=getFileStats(afn,0,&stats,NULL)) & meIOTYPE_REGULAR) &&
               meFiletimeIsModified(stats.stmtime,bp->stats.stmtime))
            {
                meUByte prompt[200];
                meUByte *tt;
                tt = getFileBaseName(afn);
                memcpy(prompt,"Recover newer ",14);
                meStrncpy(prompt+14,tt,200-14-1);
                prompt[199] = '\0';
                if(mlyesno(prompt) > 0)
                {
                    fn = afn;
                    ft = aft;
                    memcpy(&bp->stats,&stats,sizeof(meStat)) ;
                }
            }
            if(ft & meIOTYPE_DIRECTORY)
            {
#if MEOPT_DIRLIST
                if(ft & meIOTYPE_NOTEXIST)
                {
                    mlwrite(MWABORT,(meUByte *)"[Directory does not exist: %s]",fn);
                    goto error_end;
                }
                if(readDirectory(fn,bp,bp->baseLine,0) <= 0)
                    goto error_end;
                ss = meTRUE ;
                goto newfile_end;
#else
                mlwrite(MWABORT,(meUByte *)"[Directory listing not enabled]");
                goto error_end;
#endif
            }
            if(ft & meIOTYPE_NOTEXIST)
            {
                meUByte dirbuf [meBUF_SIZE_MAX];

                /* See if we can write to the directory. */
                getFilePath(fn,dirbuf);
                if(((getFileStats(dirbuf,0,NULL,NULL) & (meIOTYPE_NOTEXIST|meIOTYPE_DIRECTORY)) != meIOTYPE_DIRECTORY)
#ifdef _UNIX
                   || meTestWrite(dirbuf)
#endif
                   )
                {
                    /* READ ONLY DIR */
                    mlwrite(MWPAUSE,(meUByte *)"[Cannot write to directory: %s]",dirbuf);
                    /* Zap the filename - it is invalid, we only want a buffer */
                    mlwrite (0,(meUByte *)"[New buffer %s]", getFileBaseName(fname));
                    meNullFree(bp->fileName);
                    bp->fileName = NULL;
                    ss = meABORT;
                }
                else
                {
                    mlwrite(0,(meUByte *)"[New file %s]", fname);
                    ss = meTRUE ;
                }
                /* its not linked to a file so change the flag */
                bp->intFlag &= ~BIFFILE ;
                goto newfile_end;
            }

            /* Make sure that we can read the file. If we are root then
             * we do not test the 'stat' bits. Root is allowed to read
             * anything */
            if ((
#if MEOPT_TFS
                 !ffUrlTypeIsTfs(ffUrlGetType(fname)) &&
#endif
                 (meTestRead (fn))) ||
                (
#ifdef _UNIX
                 (meUid != 0) &&
#endif
                 (!meStatTestRead(bp->stats))))
            {
                /* We are not allowed to read the file */
                mlwrite(MWABORT,(meUByte *)"[Cannot read file %s]",fn);
                /* Zap the filename - it is invalid. we only want a buffer */
                meNullFree(bp->fileName) ;
                bp->fileName = NULL;
                /* its not linked to a file so change the flag */
                bp->intFlag &= ~BIFFILE ;
                goto newfile_end;
            }
#if MEOPT_EXTENDED
            if((fileSizePrompt > 0) && ((ui=(bp->stats.stsizeHigh << 12) | (bp->stats.stsizeLow >> 20)) > fileSizePrompt))
            {
                meUByte prompt[200];
                memcpy(prompt,"File ",5);
                meStrncpy(prompt+5,getFileBaseName(fn),200-5-25);
                prompt[200-25] = '\0';
                sprintf((char *) (prompt+meStrlen(prompt))," is %dMB, continue",ui);
                if(mlyesno(prompt) <= 0)
                    goto error_end;
            }
#endif
#ifdef _WIN32
            if(!meStatTestSystem(bp->stats))
            {
                /* if windows system file read in a readonly */
                meModeSet(bp->mode,MDVIEW);
                mlwrite(MWCURSOR|MWCLEXEC,(meUByte *)"[Reading %s (system file)]", fn);
            }
            else
#endif
                /* Depending on whether we have write access set the buffer into
                 * view mode. Again root gets the privelidge of being able to
                 * write if possible. */
                if ((meTestWrite (fn)) ||
                    (
#ifdef _UNIX
                     (meUid != 0) &&
#endif
                     (!meStatTestWrite(bp->stats))))
                meModeSet(bp->mode,MDVIEW);
        }
        mlwrite(MWCURSOR|MWCLEXEC,(meUByte *)"[Reading %s%s]",fn,
                meModeTest(bp->mode,MDVIEW) ? " (readonly)" : "");
    }
    ss = ffReadFile(&meior,fn,meRWFLAG_READ,bp,bp->baseLine,0,0,0);

    /*
    ** Set up the modification time field of the buffer structure.
    */
    if(ss != meABORT)
    {
        mlwrite(MWCLEXEC,(meUByte *)"[Read %d line%s]",bp->lineCount,(bp->lineCount==1) ? "":"s");
        if(fn == afn)
            /* this is a recovered file so flag the buffer as changed */
            meModeSet(bp->mode,MDEDIT);
        ss = meTRUE;
    }
    else
        meModeSet(bp->mode,MDVIEW);

newfile_end:

    bp->dotLine = meLineGetNext(bp->baseLine);
    bp->dotLineNo = 0;
    bp->dotOffset = 0;

error_end:
    return ss;
}

/*
 * Insert file "fname" into the current
 * buffer, Called by insert file command. Return the final
 * status of the read.
 */
/* must have the buffer line no. correct */
int
meBufferInsertFile(meBuffer *bp, meUByte *fname, meUInt flags,
                   meUInt uoffset, meUInt loffset, meInt length)
{
    register meWindow *wp;
    register meLine *lp;
    register int ss;
    register long nline;

    meModeSet(bp->mode,MDEDIT);              /* we have changed	*/

#if MEOPT_CRYPT
    if(meCryptBufferInit(bp) <= 0)
        return meFALSE;
#endif
    if(!(flags & meRWFLAG_SILENT))
        mlwrite(MWCURSOR|MWCLEXEC,(meUByte *)"[Inserting file]");

    nline = bp->lineCount;
    lp = bp->dotLine;
#if MEOPT_DIRLIST
    if(flags & meRWFLAG_MKDIR)
    {
        /* slight miss-use of this flag to inform this function that a dir
         * listing is to be inserted */
        ss = readDirectory(fname,bp,lp,flags|meRWFLAG_INSERT);
    }
    else
#endif
        ss = ffReadFile(&meior,fname,flags|meRWFLAG_READ|meRWFLAG_INSERT,bp,lp,uoffset,loffset,length);
    nline = bp->lineCount-nline;

    if(ss != meABORT)
    {
        ss = meTRUE;
        if(!(flags & meRWFLAG_SILENT))
            mlwrite(MWCLEXEC,(meUByte *)"[inserted %d line%s]",nline,(nline==1) ? "":"s");
    }
    meFrameLoopBegin();
    for(wp=loopFrame->windowList; wp!=NULL; wp=wp->next)
    {
        if(wp->buffer == bp)
        {
            if(wp->dotLineNo >= bp->dotLineNo)
                wp->dotLineNo += nline;
            if(wp->markLineNo >= bp->dotLineNo)
                wp->markLineNo += nline;
            wp->updateFlags |= WFMAIN|WFMOVEL;
        }
    }
    meFrameLoopEnd();
#if MEOPT_UNDO
    if((bp == frameCur->windowCur->buffer) && meModeTest(bp->mode,MDUNDO))
    {
        int ii = 0;
        frameCur->windowCur->dotOffset = 0;
        while(nline--)
        {
            lp = meLineGetPrev(lp);
            ii += meLineGetLength(lp) + 1;
        }
        meUndoAddInsChars(ii);
    }
#endif
    return ss;
}

/*
 * Insert a file into the current buffer. This is really easy; all you do is find the name of the
 * file, and call the standard "insert a file into the current buffer" code. Bound to "C-X C-I".
 */
int
insertFile(int f, int n)
{
    meWindow *cwp;
    meUByte fname[meBUF_SIZE_MAX];
    meStat stats;
    meUInt uoffset, loffset=0;
    meInt length=0;
    int s, flags=0;

    if((s=inputFileName((meUByte *)"Insert file",fname,1)) <= 0)
        return s ;
    /* Allow existing or url files if not doing a partial insert */
    if((s = getFileStats(fname,meFINDFILESINGLE_GFS_OPT,&stats,NULL)) & meIOTYPE_NOTEXIST)
        return mlwrite(MWABORT,(meUByte *)"[Given file does not exist]");
    if((s & (meIOTYPE_REGULAR
#if MEOPT_DIRLIST
             |meIOTYPE_DIRECTORY
#endif
#if MEOPT_TFS
             |meIOTYPE_TFS
#endif
#if MEOPT_SOCKET
             |meIOTYPE_HTTP|meIOTYPE_FTP|meIOTYPE_FTPE
#endif
             )) == 0)
        return mlwrite(MWABORT,(meUByte *)"[Given file type not supported]");
    if((n & 4) && (((s & meIOTYPE_REGULAR) == 0) || (s & meIOTYPE_TFS)))
        return mlwrite(MWABORT,(meUByte *)"[Partial insert not supported for given file type]");
#if MEOPT_EXTENDED
    if((fileSizePrompt > 0) && ((n & 4) == 0) && ((uoffset=(stats.stsizeHigh << 12) | (stats.stsizeLow >> 20)) > fileSizePrompt))
    {
        meUByte prompt[200];
        memcpy(prompt,"File ",5);
        meStrncpy(prompt+5,getFileBaseName(fname),200-5-25);
        prompt[200-25] = '\0';
        sprintf((char *) (prompt+meStrlen(prompt))," is %dMB, continue",uoffset);
        if(mlyesno(prompt) <= 0)
            return meABORT;
    }
#endif
#if MEOPT_DIRLIST
    if(s & meIOTYPE_DIRECTORY)
        flags |= meRWFLAG_MKDIR;
#endif
    /* Check we can change the buffer */
    if((s=bufferSetEdit()) <= 0)
        return s;
    cwp = frameCur->windowCur;
    /* set mark to previous line so it doesnt get moved down */
    cwp->markLine = meLineGetPrev(cwp->dotLine) ;
    cwp->markOffset = 0;
    cwp->markLineNo = cwp->dotLineNo-1;

    /* store current line in buffer */
    cwp->buffer->dotLine = cwp->dotLine;
    cwp->buffer->dotLineNo = cwp->dotLineNo;   /* must have the line no. correct */

    if((n & 2) == 0)
        flags |= meRWFLAG_PRESRVFMOD ;
    if(n & 4)
    {
        meUByte arg[meSBUF_SIZE_MAX] ;

        if(meGetString((meUByte *)"UOffest",0,0,arg,meSBUF_SIZE_MAX) <= 0)
            return meABORT;
        uoffset = meAtoi(arg);
        if(meGetString((meUByte *)"LOffest",0,0,arg,meSBUF_SIZE_MAX) <= 0)
            return meABORT;
        loffset = meAtoi(arg);
        if((meGetString((meUByte *)"Length",0,0,arg,meSBUF_SIZE_MAX) <= 0) ||
           ((length = meAtoi(arg)) == 0))
            return meABORT;
    }

    if(((s = meBufferInsertFile(cwp->buffer,fname,flags,uoffset,loffset,length)) > 0) && (n & 2))
        meStatCopy(&(cwp->buffer->stats),&stats);

    /* move the mark down 1 line into the correct place */
    cwp->markLine = meLineGetNext(cwp->markLine);
    cwp->markLineNo++;
    return s;
}

/*
 * Take a file name, and from it
 * fabricate a buffer name. This routine knows
 * about the syntax of file names on the target system.
 * I suppose that this information could be put in
 * a better place than a line of code.
 */
void
makename(meUByte *bname, meUByte *fname)
{
    register meUByte *cp1;
    register meUByte *cp2;
    int i;

    cp1 = getFileBaseName(fname);
    cp2 = bname;
    while((*cp2++=*cp1++) != '\0')
        ;
    cp2-- ;
    /*
     * Now we've made our buffer name, check that it is unique.
     */
    for(i = 1 ; (bfind(bname, 0) != NULL) ; i++)
    {
        /*
         * There is another buffer with this name. Try sticking a
         * numeric "i" on the end of the buffer.
         */
        sprintf((char *)cp2, "<%d>", i);
    }
}

#define fileNameWild(fileName)                                                        \
((strchr((char *)fileName,'*') != NULL) || (strchr((char *)fileName,'?') != NULL) ||  \
 (strchr((char *)fileName,'[') != NULL))

static int
findFileSingle(meUByte *fname, int bflag, meInt lineno, meUShort colno)
{
    meBuffer *bp ;
    int   gft ;
#if MEOPT_SOCKET
    int fnlen ;
#endif
#ifdef _UNIX
    meStat  bstt,stats;
    gft = getFileStats(fname,meFINDFILESINGLE_GFS_OPT & (bflag | ~BFND_CREAT),&stats,NULL);
#else
    /* Stop getFileStats complaining on gfsERRON_ILLEGAL_NAME if BFND_CREAT not set (error reported elsewhere) */
    gft = getFileStats(fname,meFINDFILESINGLE_GFS_OPT & (bflag | ~BFND_CREAT),NULL,NULL);
#endif
    if((gft & (meIOTYPE_NASTY
#if (MEOPT_DIRLIST == 0)
                |meIOTYPE_DIRECTORY
#endif
#if (MEOPT_TFS == 0)
                |meIOTYPE_TFS
#endif
#if (MEOPT_SOCKET == 0)
                |meIOTYPE_HTTP|meIOTYPE_FTP|meIOTYPE_FTPE
#endif
                )) ||
       ((gft & meIOTYPE_NOTEXIST) && !(bflag & BFND_CREAT)))
        return 0 ;

    bflag |= BFND_CREAT;

    /* if this is a directory then add the '/' */
    if(gft & meIOTYPE_DIRECTORY)
    {
        meUByte *ss=fname+meStrlen(fname);
        if(ss[-1] != DIR_CHAR)
        {
            ss[0] = DIR_CHAR;
            ss[1] = '\0';
        }
    }
#if MEOPT_SOCKET
    if(((gft & (meIOTYPE_FTP|meIOTYPE_FTPE)) == 0) || ((fnlen = meStrlen(fname)),(fname[fnlen] == DIR_CHAR)))
        fnlen = 0;
#endif
    for(bp=bheadp; bp!=NULL; bp=bp->next)
    {
        if((bp->fileName != NULL) && (bp->name != NULL) && (bp->name[0] != '*'))
        {
            if(!fnamecmp(bp->fileName,fname))
                break;
#ifdef _UNIX
            /* On UNIX the same file could be loaded via different symbolic links, so if the stdev & stino are the same its the same file
             * originally, however the file may have been moved so only use it if the bp->fileName still exists and has same file details */
            if((stats.stdev != (dev_t)(-1)) && (bp->stats.stdev == stats.stdev) && (bp->stats.stino == stats.stino) &&
               (getFileStats(bp->fileName,0,&bstt,NULL) & (meIOTYPE_REGULAR|meIOTYPE_DIRECTORY)) &&
               (bstt.stino == stats.stino) && (bstt.stsizeHigh == stats.stsizeHigh) && (bstt.stsizeLow == stats.stsizeLow))
                break;
#endif
#if MEOPT_SOCKET
            /* at this point the type of an ftp file (i.e. regulare file or dir) is unknown, the
             * filename will be changed if dir ('/' added), but the comparison of ftp: file names
             * must allow for this */
            if(fnlen && !meStrncmp(bp->fileName,fname,fnlen) &&
               (bp->fileName[fnlen] == DIR_CHAR) && (bp->fileName[fnlen+1] == '\0'))
                break ;
#endif
        }
    }
    if(bp == NULL)
    {
        bp = bfind(fname,bflag);
        bp->fileName = meStrdup(fname);
        bp->intFlag |= BIFFILE;
        if(gft & meIOTYPE_DIRECTORY)
        {
            /* if its a directory then add dir flag and remove binary */
            bp->fileFlag = meBFFLAG_DIR;
            meModeClear(bp->mode,MDBINARY);
            meModeClear(bp->mode,MDRBIN);
        }
        bp->dotLineNo = lineno ;
        bp->dotOffset = colno ;
    }
    else if(!meModeTest(bp->mode,MDNACT))
        bp->intFlag |= BIFLOAD;
    bp->histNo = bufHistNo;
    return 1;
}

static void
fileMaskToRegex(meUByte *dfname, meUByte *sfname)
{
    meUByte *ss=sfname, *dd=dfname, cc ;

    while((cc = *ss++) != '\0')
    {
        switch(cc)
        {
        case '*':
            *dd++ = '.' ;
            *dd++ = '*' ;
            break ;
        case '?':
            *dd++ = '.' ;
            break ;
        case '[':
            *dd++ = '[' ;
            *dd++ = *ss++ ;
            while((cc = *ss++) != ']')
                if((*dd++ = cc) == '\0')
                    return ;
            *dd++ = ']' ;
            break ;
        case '\\':
        case '+':
        case '.':
        case '$':
        case '^':
            *dd++ = '\\' ;
        default:
            *dd++ = cc ;
        }
    }
    *dd = '\0' ;
}

int
findFileList(meUByte *fname, int bflag, meInt lineno, meUShort colno)
{
    register int nofiles=0, ii ;
    meUByte fileName[meBUF_SIZE_MAX], *baseName, cc ;

    bufHistNo++ ;
    fileNameCorrect(fname,fileName,&baseName) ;

    cc = ffUrlGetType(fileName);
    if(!ffUrlTypeIsHttpFtp(cc) && fileNameWild(baseName) &&
       (ffUrlTypeIsTfs(cc) || meTestRead(fileName)))
    {
        /* if the base name has a wild card letter (i.e. *, ? '[')
         * and a file with that exact name does not exist then load
         * any files which match the wild card mask */
        meUByte mask[meBUF_SIZE_MAX] ;

        fileMaskToRegex(mask,baseName) ;
        cc = *baseName ;
        *baseName = '\0' ;
        getDirectoryList(fileName,&curDirList) ;
        for(ii=0 ; ii<curDirList.size ; ii++)
        {
            meUByte *ss = curDirList.list[ii] ;
#ifdef _INSENSE_CASE
            if(regexStrCmp(ss,mask,meRSTRCMP_ICASE|meRSTRCMP_WHOLE))
#else
            if(regexStrCmp(ss,mask,meRSTRCMP_WHOLE))
#endif
            {
                meStrcpy(baseName,ss) ;
                nofiles += findFileSingle(fileName,bflag,lineno,colno) ;
            }
        }
        *baseName = cc ;
    }
    if(nofiles == 0)
        nofiles += findFileSingle(fileName,bflag,lineno,colno) ;
    return nofiles ;
}

int
findSwapFileList(meUByte *fname, int bflag, meInt lineno, meUShort colno)
{
    meBuffer *bp;
    int ret;

    bufHistNo += 2;
    if(!findFileList(fname,bflag,lineno,colno))
        return mlwrite(MWABORT|MWCLEXEC,(meUByte *)"[Failed to find file %s]",fname);
    for(bp=bheadp ; bp->histNo!=bufHistNo ; bp=bp->next)
        ;
    bufHistNo -= 2;
    ret = swbuffer(frameCur->windowCur,bp);  /* make buffer BP current */
    bufHistNo++;
    return ret;
}

/*
 * Select a file for editing.
 * Look around to see if you can find the
 * fine in another buffer; if you can find it
 * just switch to the buffer. If you cannot find
 * the file, create a new buffer, read in the
 * text, and switch to the new buffer.
 * Bound to C-X C-F.
 */
int
findFile(int f, int n)
{
    meUByte fname[meBUF_SIZE_MAX], prompt[16], *ss;

    ss = prompt;
    *ss++ = 'f';
    *ss++ = 'i';
    *ss++ = 'n';
    *ss++ = 'd';
    *ss++ = '-';
    if(n & BFND_BINARY)
        *ss++ = 'b';
    if(n & BFND_CRYPT)
        *ss++ = 'c';
    if(n & BFND_RBIN)
        *ss++ = 'r';
    *ss++ = 'f';
    *ss++ = 'i';
    *ss++ = 'l';
    *ss++ = 'e';
    *ss   = '\0';
    if(inputFileName(prompt,fname,0) <= 0)
        return meABORT;
    n = (n & (BFND_CREAT|BFND_BINARY|BFND_CRYPT|BFND_RBIN|BFND_NOHOOK)) | BFND_MKNAM;
    return findSwapFileList(fname,n,0,0);
}

#if MEOPT_EXTENDED
/*
 * Find file into other window. This is trivial since all the hard stuff is
 * done by the routines windowSplitDepth() and filefind().
 *
 * The count is used to put the "found file" into the upper or lower window.
 *
 * Normally mapped to ^X4. (or C-X 4 if you prefer).
 */
int
nextWndFindFile(int f, int n)
{
    meUByte fname[meBUF_SIZE_MAX];	/* file user wishes to find */

    if(inputFileName((meUByte *)"Find file",fname,0) <= 0)
        return meABORT;
    if(meWindowPopup(NULL,NULL,WPOP_MKCURR,NULL) == NULL)
        return meFALSE;
    n = (n & (BFND_CREAT|BFND_BINARY|BFND_CRYPT|BFND_RBIN|BFND_NOHOOK)) | BFND_MKNAM;
    return findSwapFileList(fname,n,0,0);
}
#endif

int
readFile(int f, int n)
{
    meUByte fname[meBUF_SIZE_MAX];	/* file user wishes to find */
    register int s;		/* status return */

    if(inputFileName((meUByte *)"Read file", fname,0) <= 0)
        return meABORT;
    if(n & 0x20)
	frameCur->windowCur->buffer->intFlag |= BIFBLOW;
    n = (n & (BFND_CREAT|BFND_BINARY|BFND_CRYPT|BFND_RBIN|BFND_NOHOOK)) | BFND_MKNAM;
    if((s=zotbuf(frameCur->windowCur->buffer,clexec)) > 0)
        s = findSwapFileList(fname,n,0,0);
    return s;
}

int
viewFile(int f, int n)	/* visit a file in VIEW mode */
{
    meUByte fname[meBUF_SIZE_MAX];	/* file user wishes to find */
    register int ss, vv;	/* status return */

    if (inputFileName((meUByte *)"View file", fname,0) <= 0)
        return meABORT ;
    n = (n & (BFND_CREAT|BFND_BINARY|BFND_CRYPT|BFND_RBIN|BFND_NOHOOK)) | BFND_MKNAM ;
    /* Add view mode globally so any new buffers will be created
     * with view mode */
    vv = meModeTest(globMode,MDVIEW) ;
    meModeSet(globMode,MDVIEW) ;
    ss = findSwapFileList(fname,n,0,0) ;
    /* if view mode was not set globally restore it */
    if(!vv)
        meModeClear(globMode,MDVIEW) ;
    return ss ;
}

#define meWRITECHECK_CHECK    0x001
#define meWRITECHECK_BUFFER   0x002
#define meWRITECHECK_NOPROMPT 0x004

/*
 * writeCheck
 * This performs some simple access checks to determine if we
 * can write the file.
 */
static int
writeCheck(meUByte *pathname, int flags, meStat *statp)
{
    meUByte dirbuf[meBUF_SIZE_MAX], ft=ffUrlGetType(pathname);

    if(ffUrlTypeIsNotFile(ft))
    {
#if MEOPT_SOCKET
        if(ffUrlTypeIsFtp(ft))
            return meTRUE;
#endif
        return mlwrite(MWABORT,(meUByte *)"Cannot write to: %s",pathname);
    }
    /* Quick test for read only. */
#ifdef _UNIX
    /* READ ONLY directory test
       See if we can write to the directory. */
    getFilePath(pathname,dirbuf);
    if (meTestWrite (dirbuf))
        return mlwrite (MWABORT,(meUByte *)"Read Only Directory: %s", dirbuf);
#endif
    /* See if there is an existing file */
    if((flags & meWRITECHECK_CHECK) == 0)       /* Validity check enabled ?? */
        return meTRUE;                          /* No - quit. */
    if(meTestExist(pathname))                   /* Does it exist ?? */
        return meTRUE;                          /* No - quit */
    if((statp != NULL) && !meStatTestWrite(*statp))
    {
        /* No - advised read only - see if the users wants to write */
        sprintf((char *)dirbuf, "%s is read only, overwrite", pathname);
        if((flags & meWRITECHECK_NOPROMPT) || (mlyesno(dirbuf) <= 0))
            return ctrlg(meFALSE,1);
    }
    return meTRUE;
}

static meUByte *
writeFileChecks(meUByte *dfname, meUByte *sfname, meUByte *lfname, int flags)
{
    register int s;
    meStat stats;
    meUByte *fn;

    if((sfname != NULL) &&
       (((s=getFileStats(dfname,0,NULL,NULL)) & meIOTYPE_DIRECTORY) ||
        ((s & (meIOTYPE_FTP|meIOTYPE_FTPE)) && (dfname[meStrlen(dfname)-1] == DIR_CHAR))))
    {
        s = meStrlen(dfname);
        if(dfname[s-1] != DIR_CHAR)
            dfname[s++] = DIR_CHAR;
        meStrcpy(dfname+s,getFileBaseName(sfname));
    }
    if(((s=getFileStats(dfname,gfsERRON_ILLEGAL_NAME|gfsERRON_BAD_FILE|gfsERRON_DIR,&stats,lfname)) & (meIOTYPE_REGULAR|meIOTYPE_NOTEXIST
#if MEOPT_SOCKET
                                                                                                       |meIOTYPE_FTP|meIOTYPE_FTPE
#endif
                                                                                                       )) == 0)
        return NULL;
    fn = (lfname[0] == '\0') ? dfname:lfname;
    if(flags & meWRITECHECK_CHECK)
    {
        meUByte prompt[meBUF_SIZE_MAX+48];

        /* Check for write-out filename problems */
        if(s & meIOTYPE_REGULAR)
        {
            sprintf((char *)prompt,"%s already exists, overwrite",fn);
            if((flags & meWRITECHECK_NOPROMPT) || (mlyesno(prompt) <= 0))
            {
                ctrlg(meFALSE,1);
                return NULL;
            }
        }
        /* Quick check on the file write condition */
        if(writeCheck(fn,flags,&stats) <= 0)
            return NULL;

        if(flags & meWRITECHECK_CHECK)
        {
            /*
             * Check to see if the new filename conflicts with the filenames for any
             * other buffer and produce a warning if so.
             */
            meBuffer *bp=bheadp, *cbp=frameCur->windowCur->buffer;

            while(bp != NULL)
            {
                if((bp != cbp) &&
#ifdef _UNIX
                   (!fnamecmp(bp->fileName,fn) ||
                    ((stats.stdev != (dev_t)(-1)) &&
                     (bp->stats.stdev == stats.stdev) &&
                     (bp->stats.stino == stats.stino) &&
                     (bp->stats.stsizeHigh == stats.stsizeHigh) &&
                     (bp->stats.stsizeLow == stats.stsizeLow))))
#else
                   !fnamecmp(bp->fileName,fn))
#endif
                {
                    sprintf((char *)prompt, "Buffer %s is the same file, overwrite",bp->name);
                    if(mlyesno(prompt) <= 0)
                    {
                        ctrlg(meFALSE,1);
                        return NULL;
                    }
                }
                bp = bp->next;
            }
        }
    }
    return fn;
}

#if MEOPT_EXTENDED

#define meFILEOP_CHECK     0x001
#define meFILEOP_BACKUP    0x002
#define meFILEOP_NOPROMPT  0x004
#define meFILEOP_PRESRVTS  0x008
#define meFILEOP_FTPCLOSE  0x010
#define meFILEOP_DELETE    0x020
#define meFILEOP_MOVE      0x040
#define meFILEOP_COPY      0x080
#define meFILEOP_MKDIR     0x100
#define meFILEOP_CHMOD     0x200
#define meFILEOP_TOUCH     0x400
#define meFILEOP_WC_MASK   0x005

int
fileOp(int f, int n)
{
    meUByte ft, sfname[meBUF_SIZE_MAX], dfname[meBUF_SIZE_MAX], lfname[meBUF_SIZE_MAX+22], *fn=NULL ;
    int ii, rr=meTRUE, dFlags=0, fileMask=-1 ;

    if((n & (meFILEOP_FTPCLOSE|meFILEOP_DELETE|meFILEOP_MOVE|meFILEOP_COPY|meFILEOP_MKDIR|meFILEOP_CHMOD|meFILEOP_TOUCH)) == 0)
        rr = mlwrite(MWABORT,(meUByte *)"[No operation set]") ;
    else if(n & meFILEOP_DELETE)
    {
        if(inputFileName((meUByte *)"Delete file",sfname,1) <= 0)
            rr = 0 ;
        else if((ft=ffUrlGetType(sfname)) & (meIOTYPE_SSL|meIOTYPE_TFS|meIOTYPE_HTTP))
            rr = mlwrite(MWABORT,(meUByte *)"[Cannot delete %s]",sfname);
        else if((n & meFILEOP_CHECK) && !ffUrlTypeIsFtp(ft))
        {
            if(meTestExist(sfname))
            {
                mlwrite(MWABORT|MWCLEXEC,(meUByte *)"[%s: No such file]",sfname);
                rr = -2 ;
            }
            else if(meTestWrite(sfname))
            {
                sprintf((char *)lfname, "%s is read only, delete",sfname) ;
                if((n & meFILEOP_NOPROMPT) || (mlyesno(lfname) <= 0))
                {
                    ctrlg(meFALSE,1);
                    rr = -8 ;
                }
            }
        }
        dFlags = meRWFLAG_DELETE;
    }
    else if(n & (meFILEOP_MOVE|meFILEOP_COPY))
    {
        static meUByte prompt[]="Copy file";
        if(n & meFILEOP_MOVE)
        {
            prompt[0] = 'M';
            prompt[2] = 'v';
            prompt[3] = 'e';
            dFlags = meRWFLAG_DELETE;
        }
        else
        {
            prompt[0] = 'C';
            prompt[2] = 'p';
            prompt[3] = 'y';
        }
        if((inputFileName(prompt,sfname,1) <= 0) || (inputFileName((meUByte *)"To", dfname,1) <= 0))
            rr = 0;
        else if((fn=writeFileChecks(dfname,sfname,lfname,(n & meFILEOP_WC_MASK))) == NULL)
            rr = -6;
        else
            fileMask = meFileGetAttributes(sfname);
        dFlags |= meRWFLAG_NODIRLIST;
        if(n & meFILEOP_PRESRVTS)
            dFlags |= meRWFLAG_PRESRVTS;
    }
    else if(n & meFILEOP_MKDIR)
    {
        if (inputFileName((meUByte *)"Create dir",sfname,1) <= 0)
            rr = 0;
        /* check that nothing of that name currently exists */
        else if((getFileStats(sfname,0,NULL,NULL) & (meIOTYPE_NOTEXIST
#if MEOPT_SOCKET
                                                     |meIOTYPE_FTP|meIOTYPE_FTPE
#endif
                                                     )) == 0)
        {
            mlwrite(MWABORT|MWCLEXEC,(meUByte *)"[%s already exists]",sfname);
            rr = -7;
        }
        else
            dFlags = meRWFLAG_MKDIR;
    }
    else if(n & (meFILEOP_CHMOD|meFILEOP_TOUCH))
    {
        if((inputFileName((meUByte *) ((n & meFILEOP_CHMOD) ? "Chmod":"Touch"),sfname,1) <= 0) ||
           (meGetString((meUByte *)"To",0,0,dfname,meBUF_SIZE_MAX) <= 0))
            rr = 0;
        /* check that nothing of that name currently exists */
        else if((((ii=getFileStats(sfname,0,NULL,NULL)) & (meIOTYPE_TFS|meIOTYPE_HTTP|meIOTYPE_FTP|meIOTYPE_FTPE)) != 0) ||
                ((ii & (meIOTYPE_REGULAR|meIOTYPE_DIRECTORY)) == 0))
        {
            mlwrite(MWABORT|MWCLEXEC,(meUByte *)"[%s not a local file or directory]",sfname);
            rr = -6 ;
        }
        else if(n & meFILEOP_CHMOD)
        {
            meFileSetAttributes(sfname,meAtoi(dfname)) ;
            return meTRUE ;
        }
        else
        {
#ifdef _UNIX
            struct utimbuf fileTimes ;

            fileTimes.actime = fileTimes.modtime = time(NULL) + meAtoi(dfname) ;
            utime((char *) sfname,&fileTimes) ;
#endif
#ifdef _WIN32
            HANDLE fp ;
            FILETIME ftm;
            SYSTEMTIME st;
            if((fp=CreateFile((const char *) sfname,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL,NULL)) == INVALID_HANDLE_VALUE)
            {
                mlwrite(MWABORT|MWCLEXEC,(meUByte *)"[%s: Cannot access file]",sfname);
                rr = -6 ;
            }
            else
            {
                GetSystemTime(&st) ;
                SystemTimeToFileTime(&st,&ftm) ;
                rr = meAtoi(dfname) ;
                if(rr != 0)
                {
                    ULARGE_INTEGER uli ;
                    uli.LowPart = ftm.dwLowDateTime ;
                    uli.HighPart = ftm.dwHighDateTime ;
                    uli.QuadPart += ((ULONGLONG) rr) * 10000000 ;
                    ftm.dwLowDateTime = uli.LowPart ;
                    ftm.dwHighDateTime = uli.HighPart ;
                }
                f = SetFileTime(fp,NULL,NULL,&ftm) ;
                CloseHandle(fp) ;
            }
#endif
            return meTRUE ;
        }
    }
    if(rr > 0)
    {
        if(n & meFILEOP_BACKUP)
            dFlags |= meRWFLAG_BACKUP;
        if(n & meFILEOP_FTPCLOSE)
            dFlags |= meRWFLAG_SOCKCLOSE;
        if((rr = ffFileOp(sfname,fn,dFlags,fileMask)) > 0)
            return meTRUE ;
    }
    resultStr[0] = '0' - rr;
    resultStr[1] = '\0';
    return meABORT ;
}
#endif

/*
 * This function performs an auto writeout to disk for the given buffer
 */
void
autowriteout(register meBuffer *bp)
{
    meUByte fn[meBUF_SIZE_MAX], *ff;

    bp->autoTime = -1;

    if((ff=bp->fileName) != NULL)
    {
        meLine *blp, *lp=NULL;
#ifdef _UNIX
        meUByte lname[meBUF_SIZE_MAX];
        getFileStats(ff,0,NULL,lname);
        if(lname[0] != '\0')
            ff = lname;
#endif
        if(!createBackupName(fn,ff,'#',1) &&
           ((blp = translateBufferBack(bp,meRWFLAG_WRITE|meRWFLAG_AUTOSAVE)) != NULL))
        {

            if(ffWriteFileOpen(&meiow,fn,meRWFLAG_WRITE|meRWFLAG_AUTOSAVE,bp) > 0)
            {
                lp = meLineGetNext(blp);            /* First line.          */
                while((lp != blp) &&
                      (
#if MEOPT_EXTENDED
                       (lp->flag & meLINE_MARKUP) ||
#endif
                       (ffWriteFileWrite(&meiow,meLineGetLength(lp),meLineGetText(lp),!(lp->flag & meLINE_NOEOL)) > 0)))
                {
                    if(TTbreakTest(1))
                    {
                        lp = NULL;
                        break;
                    }
                    lp = meLineGetNext(lp);
                }
                if(ffWriteFileClose(&meiow,meRWFLAG_WRITE|meRWFLAG_AUTOSAVE,bp) <= 0)
                    lp = NULL;
            }
            if(blp != bp->baseLine)
                meLineLoopFree(blp,1);
            if(lp != NULL)
            {
                mlerase(MWCLEXEC);
                return;
            }
        }
    }
    mlwrite(MWABORT,(meUByte *)"[Auto-writeout failure for buffer %s]",bp->name) ;
}

/*
 * This function removes an auto writeout file for the given buffer
 */
void
autowriteremove(register meBuffer *bp)
{
    meUByte fn[meBUF_SIZE_MAX] ;

    if((autoTime > 0) && bufferNeedSaving(bp) &&
       !createBackupName(fn,bp->fileName,'#',0) && !meTestExist(fn))
        /* get rid of any tempory file */
        meUnlinkNT(fn);
    bp->autoTime = -1 ;
}

/*
 * This function performs the details of file
 * writing. Uses the file management routines in the
 * "fileio.c" package. The number of lines written is
 * displayed. Sadly, it looks inside a meLine; provide
 * a macro for this. Most of the grief is error
 * checking of some sort.
 */
int
writeOut(register meBuffer *bp, meUInt flags, meUByte *fn)
{
    if(meModeTest(bp->mode,MDBACKUP))
        flags |= meRWFLAG_BACKUP ;
#if MEOPT_TIMSTMP
    set_timestamp(bp);			/* Perform time stamping */
#endif
    if(ffWriteFile(&meiow,fn,flags,bp) <= 0)
        return meFALSE ;
    /* No write error.      */
    /* must be done before buffer is flagged as not changed */
#if MEOPT_UNDO
    if(meSystemCfg & meSYSTEM_KEEPUNDO)
    {
        /* go through undo list removing any unedited flags */
        meUndoNode *nn ;
        nn = bp->undoHead ;
        while(nn != NULL)
        {
            if(meUndoIsSetEdit(nn))
                nn->type |= meUNDO_UNSET_EDIT ;
            nn = nn->next ;
        }
    }
    else
        meUndoRemove(bp) ;
#endif
    autowriteremove(bp) ;
    meModeClear(bp->mode,MDEDIT) ;

    if(fn != NULL)
    {
#ifndef _WIN32
        /* set the right file attributes */
        if(bp->stats.stmode != meUmask)
            meFileSetAttributes(fn,bp->stats.stmode) ;
#ifdef _UNIX
        /* If we are the root then restore the existing settings on the
         * file. This should really be done using a $system flag - but
         * one assumes that this is the default action for the super user. */
        if (meUid == 0)
        {
            if((bp->stats.stuid != meUid) || (bp->stats.stgid != meGid))
                chown ((char *)fn, bp->stats.stuid, bp->stats.stgid);
        }
        /* else restore the group for normal users if possible */
        else if((bp->stats.stgid != meGid) && meGidSize && meGidInGidList(bp->stats.stgid))
            chown ((char *)fn, meUid, bp->stats.stgid);
#endif
#endif
        /* Read in the new stats */
        getFileStats(fn,0,&bp->stats,NULL);
        /* Change the view status back to the file's permissions.
         * For root we let the super user do as they wish so no
         * read protection is added. */
        if(
#if (defined _UNIX)
           (meUid != 0) &&
#endif
           (!meStatTestWrite(bp->stats)))
            meModeSet(frameCur->windowCur->buffer->mode,MDVIEW) ;
        else
            meModeClear(frameCur->windowCur->buffer->mode,MDVIEW) ;
    }
    return meTRUE ;
}

static meUByte fileHasBinary[]="File had binary characters which will be lost, write" ;
static meUByte fileHasInLnEn[]="File had inconsistent line endings which will be corrected, write" ;
/*
 * This function performs the details of file writing. Uses the file
 * management routines in the "fileio.c" package. The number of lines written
 * is displayed. Sadly, it looks inside a meLine; provide a macro for this. Most
 * of the grief is error checking of some sort.
 */
static int
writeout(register meBuffer *bp, int flags)
{
    meUByte lname[meBUF_SIZE_MAX], *fn;

    if(!meStrcmp(bp->name,"*stdin*"))
        fn = NULL;
    else if((bp->name[0] == '*') || (bp->fileName == NULL))
        return mlwrite(MWABORT,(meUByte *)"[No file name set for buffer %s]",bp->name);
    else
    {
        meStat stats;
        /*
         * Check that the file has not been modified since it was read it in. This
         * is a bit of a problem since the info we require is hidden in a buffer
         * structure which may not necessarily be the current buffer. If it is not
         * in the current buffer then dont bother to look for it.
         */
        getFileStats(bp->fileName,0,&stats,lname);
        if(flags & 1)
        {
            if(meFiletimeIsNotSame(stats.stmtime,bp->stats.stmtime))
            {
                meUByte prompt[meBUF_SIZE_MAX] ;
                sprintf((char *) prompt,"%s has been %s, write",bp->fileName,(meFiletimeIsSet(stats.stmtime)) ? "modified":"moved or deleted") ;
                if(mlyesno(prompt) <= 0)
                    return ctrlg(meFALSE,1) ;
            }
            if(bp->fileFlag & (meBFFLAG_BINARY|meBFFLAG_LTDIFF))
            {
                if(mlyesno((bp->fileFlag & meBFFLAG_BINARY) ? fileHasBinary:fileHasInLnEn) <= 0)
                    return ctrlg(meFALSE,1) ;
                bp->fileFlag &= ~(meBFFLAG_BINARY|meBFFLAG_LTDIFF) ;
            }
        }
        fn = (lname[0] == '\0') ? bp->fileName:lname;
        /* Quick check on the file write condition */
        if(writeCheck(fn,flags,&stats) <= 0)
            return meABORT;
    }

    return writeOut(bp,((flags & 0x02) ? meRWFLAG_IGNRNRRW:0),fn);
}

void
resetBufferNames(meBuffer *bp, meUByte *fname)
{
    if(fnamecmp(bp->fileName,fname))
    {
        meUByte buf[meBUF_SIZE_MAX], *bb ;
        meStrrep(&bp->fileName,fname) ;
        unlinkBuffer(bp) ;
        makename(buf, fname) ;
        if((bb = meStrdup(buf)) != NULL)
        {
            meFree(bp->name) ;
            bp->name = bb ;
        }
        linkBuffer(bp) ;
    }
}

/*
 * Ask for a file name, and write the contents of the current buffer to that
 * file. Update the remembered file name and clear the buffer changed flag.
 * This handling of file names is different from the earlier versions, and is
 * more compatable with Gosling EMACS than with ITS EMACS. Bound to "C-X C-W".
 */
int
writeBuffer(int f, int n)
{
    register meBuffer *bp;
    meUByte fname[meBUF_SIZE_MAX], lname[meBUF_SIZE_MAX], *fn;

    if(inputFileName((meUByte *)"Write file",fname,1) <= 0)
        return meABORT ;

    if((bp=frameCur->windowCur->buffer)->fileName != NULL)
        fn = bp->fileName ;
    else if(bp->name[0] != '*')
        fn = bp->name ;
    else
        fn = NULL ;

    if((n & 0x01) && (bp->fileFlag & (meBFFLAG_BINARY|meBFFLAG_LTDIFF)) &&
       (mlyesno((bp->fileFlag & meBFFLAG_BINARY) ? fileHasBinary:fileHasInLnEn) <= 0))
        return ctrlg(meFALSE,1);
    if((fn=writeFileChecks(fname,fn,lname,(n & 0x01)|meWRITECHECK_BUFFER)) == NULL)
        return meABORT ;

    if(!writeOut(bp,((n & 0x02) ? meRWFLAG_IGNRNRRW:0),fn))
        return meFALSE ;

    resetBufferNames(bp,fname) ;
    frameAddModeToWindows(WFMODE) ; /* and update ALL mode lines */

    return meTRUE ;
}

/*
 * Save the contents of the current buffer in its associatd file. No nothing
 * if nothing has changed (this may be a bug, not a feature). Error if there
 * is no remembered file name for the buffer. Bound to "C-X C-S". May get
 * called by "C-Z".
 */
int
saveBuffer(int f, int n)
{
    meBuffer *cbp=frameCur->windowCur->buffer;
    register int s;

    /* Note that we check for existance here just incase sombody has
     * deleted it under our feet. There is nothing more annoying than the
     * editor bitching there are no changes when it's been zapped !! */
    /* Further note: the file name can be NULL, e.g. theres no file name
     * so this must be tested before meTestExist, BUT the file name can be
     * NULL and still be saved, e.g. the buffer name is *stdin* - so be careful */
    if((n & 0x01) && (cbp->fileName != NULL) &&  /* Are we Checking ?? */
       (meTestExist(cbp->fileName) == 0) &&      /* Does file actually exist ? */
       !meModeTest(cbp->mode,MDEDIT))            /* Have we edited buffer ? */
        /* Return, no changes.  */
        return mlwrite(0,(meUByte *)"[No changes made]") ;
    if((s=writeout(cbp,n)) > 0)
        frameAddModeToWindows(WFMODE) ;  /* and update ALL mode lines */
    return (s);
}

/* save-some-buffers, query saves all modified buffers */
int
saveSomeBuffers(int f, int n)
{
    register meBuffer *bp;    /* scanning pointer to buffers */
    register int status=meTRUE ;
    meUByte prompt[meBUF_SIZE_MAX] ;

    bp = bheadp;
    while (bp != NULL)
    {
        if(bufferNeedSaving(bp))
        {
            if(n & 1)
            {
                if(bp->fileName != NULL)
                    sprintf((char *)prompt, "%s: Save file (?/y/n/a/o/g) ? ", bp->fileName) ;
                else
                    sprintf((char *)prompt, "%s: Save buffer (?/y/n/a/o/g) ? ", bp->name) ;
                if((status = mlCharReply(prompt,mlCR_LOWER_CASE,(meUByte *)"ynaog",(meUByte *)"(Y)es, (N)o, Yes to (a)ll, N(o) to all, (G)oto, (C-g)Abort ? ")) < 0)
                    return ctrlg(meFALSE,1) ;
                else if(status == 'g')
                {
                    swbuffer(frameCur->windowCur,bp);
                    /* return abort to halt any calling macro (e.g. compile) */
                    return meABORT;
                }
                else if(status == 'o')
                    break ;
                else if(status == 'n')
                    status = meFALSE ;
                else
                {
                    if(status == 'a')
                        n ^= 1 ;
                    status = meTRUE ;
                }
            }
            if((status > 0) && (writeout(bp,0) <= 0))
                return meABORT ;
        }
        bp = bp->next;            /* on to the next buffer */
    }
    frameAddModeToWindows(WFMODE) ;  /* and update ALL mode lines */
    return meTRUE ;
}

#if MEOPT_EXTENDED
int
appendBuffer(int f, int n)
{
    register meUInt flags ;
    register int ft;
    meUByte fname[meBUF_SIZE_MAX], lname[meBUF_SIZE_MAX], *fn ;

    if(inputFileName((meUByte *)"Append to file",fname,1) <= 0)
        return meABORT ;
    if(((ft=getFileStats(fname,gfsERRON_ILLEGAL_NAME|gfsERRON_BAD_FILE|gfsERRON_DIR,NULL,lname)) & (meIOTYPE_TFS|meIOTYPE_HTTP|meIOTYPE_FTP|meIOTYPE_FTPE)) ||
       ((ft & (meIOTYPE_REGULAR|meIOTYPE_NOTEXIST)) == 0))
        return mlwrite(MWABORT,(meUByte *)"[Unsupported file type to append to]");
    fn = (lname[0] == '\0') ? fname:lname ;
    if(ft & meIOTYPE_NOTEXIST)
    {
        if(n & 0x01)
        {
            meUByte prompt[meBUF_SIZE_MAX+24];
            sprintf((char *)prompt, "%s does not exist, create",fn) ;
            if(mlyesno(prompt) <= 0)
                return ctrlg(meFALSE,1);
        }
        flags = 0 ;
    }
    else if(n & 0x04)
        flags = meRWFLAG_OPENTRUNC ;
    else
        flags = meRWFLAG_OPENEND ;
    if(n & 0x02)
        flags |= meRWFLAG_IGNRNRRW ;
    return ffWriteFile(&meiow,fname,flags,frameCur->windowCur->buffer) ;
}

/*
 * The command allows the user
 * to modify the file name associated with
 * the current buffer. It is like the "f" command
 * in UNIX "ed". The operation is simple; just zap
 * the name in the meBuffer structure, and mark the windows
 * as needing an update. You can type a blank line at the
 * prompt if you wish.
 */
int
changeFileName(int f, int n)
{
    meBuffer *cbp;
    register int s;
    meUByte fname[meBUF_SIZE_MAX], lname[meBUF_SIZE_MAX], *fn;

    if((s=inputFileName((meUByte *)"New file name",fname,1)) == meABORT)
        return s;

    if((getFileStats(fname,gfsERRON_ILLEGAL_NAME|gfsERRON_BAD_FILE|gfsERRON_DIR,NULL,lname) & (meIOTYPE_REGULAR|meIOTYPE_NOTEXIST
#if MEOPT_SOCKET
                                                                                               |meIOTYPE_FTP|meIOTYPE_FTPE
#endif
                                                                                               )) == 0)
        return meABORT;
    fn = (lname[0] == '\0') ? fname:lname;
    cbp = frameCur->windowCur->buffer;
    meNullFree(cbp->fileName);

    if(s == meFALSE)
        cbp->fileName = NULL;
    else
        cbp->fileName = meStrdup(fname);

#if (defined _UNIX) || (defined _DOS) || (defined _WIN32)
    if(meTestWrite(fn))
        meModeSet(cbp->mode,MDVIEW);
    else
        meModeClear(cbp->mode,MDVIEW);	/* no longer read only mode */
#endif
    frameAddModeToWindows(WFMODE);      /* and update ALL mode lines */

    return meTRUE;
}
#endif

#ifdef _DOS
int
_meChdir(meUByte *path)
{
    register int s ;
    int len ;

    len = meStrlen(path)-1 ;

#ifndef __DJGPP2__
    if((len > 1) && (path[1] == _DRV_CHAR))
    {
        union REGS reg ;		/* cpu register for use of DOS calls */

        reg.h.ah = 0x0e ;
        reg.h.dl = path[0] - 'a' ;
        intdos(&reg, &reg);
    }
#endif
    if((len > 3) && (path[len] == DIR_CHAR))
        path[len] = '\0' ;
    else
        len = -1 ;
    s = chdir((char *) path) ;
    if(len > 0)
        path[len] = DIR_CHAR ;
    return s ;
}
#endif

/************************ New file routines *****************************/
#ifdef _CONVDIR_CHAR
void
fileNameConvertDirChar(meUByte *fname)
{
    /* convert all '\\' to '/' for dos etc */
    while((fname=meStrchr(fname,_CONVDIR_CHAR)) != NULL)  /* got a '\\', -> '/' */
        *fname++ = DIR_CHAR ;
}
#endif

void
fileNameSetHome(meUByte *ss)
{
    int ll = meStrlen(ss);
    meNullFree(homedir);
    homedir = meMalloc(ll+2);
    meStrcpy(homedir,ss);
    fileNameConvertDirChar(homedir);
    if(homedir[ll-1] != DIR_CHAR)
    {
        homedir[ll++] = DIR_CHAR;
        homedir[ll] = '\0';
    }
}

void
pathNameCorrect(meUByte *oldName, int nameType, meUByte *newName, meUByte **baseName)
{
    meUByte ft, *p, *p1, *urls, *urle;
    int flag;
#if (defined _DOS) || (defined _WIN32)
    meUByte *gwdbuf;
#endif

    fileNameConvertDirChar(oldName);
    flag = 0;
    p = p1 = oldName;
    /* search for
     * 1) set to root,  xxxx/http:// -> http://  (for urls)
     * 2) set to root,  xxxx/ftp://  -> ftp://   (for urls)
     * 3) set to root,  xxxx/tfs://  -> tfs://   (for urls)
     * 4) set to root,  xxxx/file:yy -> yy       (for urls)
     * 5) set to root,  xxxx///yyyyy -> //yyyyy  (for network drives)
     * 6) set to root,  xxxx//yyyyy  -> /yyyyy
     * 7) set to Drive, xxxx/z:yyyyy -> z:yyyyy
     * 8) set to home,  xxxx/~yyyyy  -> ~yyyyy
     */
    for(;;)
    {
        if(((ft = ffUrlGetType(p1)) & (meIOTYPE_FILE|meIOTYPE_HTTP|meIOTYPE_FTP|meIOTYPE_FTPE|meIOTYPE_TFS)) != 0)
        {
double_url:
            if(ffUrlTypeIsHttp(ft))
            {
                flag = 2;
                urls = p1;
                urle = p1+7+(ft & meIOTYPE_SSL);
            }
            else if(ffUrlTypeIsFtp(ft))
            {
                flag = 3;
                urls = p1;
                urle = p1+6+(((ft & (meIOTYPE_SSL|meIOTYPE_FTPE))) ? 1:0);
            }
            else if(ffUrlTypeIsTfs(ft))
            {
                flag = 4;
                urls = p1;
                urle = p1+6;
                /* this can either be a built in tfs ref (tfs:/<path>) or external tfs file (tfs://<file>?<path>) where the path starts with '/' */
            }
            else if(ffUrlTypeIsFile(ft))
            {
                flag = 0;
                p1 += 5;
                p = p1;
                /* loop here as if at the start of the file name, this
                 * is so it handles "file:ftp://..." correctly */
                continue;
            }
            if((p=meStrchr(urle,DIR_CHAR)) == NULL)
                break;
            if((p[-1] == ':') && (((ft = ffUrlGetType(urle)) & (meIOTYPE_FILE|meIOTYPE_HTTP|meIOTYPE_FTP|meIOTYPE_FTPE|meIOTYPE_TFS)) != 0))
            {
                p1 = urle;
                goto double_url;
            }
            p1 = p;
            if(flag != 2)
                urle = p;
        }
        else if(flag == 2)
            p = p1;
        /* note that ftp://... names are still processed, ftp://.../xxx//yyy -> ftp://.../yyy etc */
        else if(p1[0] == DIR_CHAR)
        {
#ifndef _UNIX
            if(p1[1] == DIR_CHAR)
            {
                /* Got a xxxx///yyyyy -> //yyyyy must not optimise further */
                flag = 1 ;
                while(p1[2] == DIR_CHAR)
                    p1++ ;
                urls = p1 ;
                if((p=meStrchr(p1+2,DIR_CHAR)) == NULL)
                    p = p1 + meStrlen(p1);
                urle = p;
                p1 = p;
            }
            else
#endif
                /* Got a xxxx//yyyyy -> /yyyyy */
                p = p1;
        }
        else if(p1[0] == '~')
        {
            /* Got a home, xxx/~/yyy -> ~/yyy, xxx/~yyy  -> ~yyy, ftp://.../xxx/~/yyy -> ftp://.../~/yyy */
            if(flag == 3)
            {
                if(p1[1] == '/')
                {
                    /* for ftp ~/ only, allow ftp://.../~/../../../yyy */
                    p = p1-1 ;
                    while(!meStrncmp(p1+2,"../",3))
                        p1 = p1+3 ;
                }
            }
            else
            {
                p = p1;
                flag = 0;
            }
        }
#ifdef _DRV_CHAR
        else if(isAlpha(p1[0]) && (p1[1] == _DRV_CHAR))
        {
            /* got a Drive, xxxx/z:yyyyy -> z:yyyyy - remove ftp:// or //yyyy/... */
            flag = 0;
            p = p1;
        }
#endif
        if(((p1=meStrchr(p1,DIR_CHAR)) == NULL) || (*++p1 == '\0'))
            break;
    }

    p1 = newName ;
    if((flag == 2) || (p == NULL))
    {
        meStrcpy(p1,urls);
        if(baseName != NULL)
            *baseName = p1 + (((p == NULL) ? urle:p)-urls);
        return;
    }
    if(flag)
    {
        int ll = (size_t) urle - (size_t) urls;
        memcpy(p1,urls,ll);
        p1 += ll;
    }
    urle = p1;
    if(p[0] == '\0')
    {
        *p1++ = DIR_CHAR;
        *p1 = '\0';
    }
    else if((flag == 0) && (p[0] == '~'))
    {
        p++;
        {
#if MEOPT_REGISTRY
            meRegNode *rNd=NULL ;
            meUByte *pe ;
            int ll ;

            if((nameType == PATHNAME_PARTIAL) && (meStrchr(p,DIR_CHAR) == NULL))
            {
                /* special case when user is entering a file name and uses complete with 'xxxx/~yy' */
                *p1++ = '~';
                meStrcpy(p1,p);
                if(baseName != NULL)
                    *baseName = p1;
                return;
            }
            if((p[0] != '\0') && (p[0] != DIR_CHAR) && ((rNd = regFind(NULL,(meUByte *)"history/" meSYSTEM_NAME "/alias-path")) != NULL) &&
               ((rNd = regGetChild(rNd)) != NULL))
            {
                /* look for an alias/abbrev path */
                if((pe = meStrchr(p,DIR_CHAR)) == NULL)
                    ll = meStrlen(p);
                else
                    ll = (int) (((size_t) pe) - ((size_t) (p)));

                while((rNd != NULL) && (((int) meStrlen(rNd->name) != ll) || meStrncmp(p,rNd->name,ll)))
                    rNd = regGetNext(rNd);
            }
            if(rNd != NULL)
            {
                if(rNd->value != NULL)
                {
                    meStrcpy(p1,rNd->value) ;
                    p1 += meStrlen(p1) - 1 ;
                    if(p1[0] != DIR_CHAR)
                        p1++ ;
                }
                p += ll ;
            }
            else
#endif
                if(homedir != NULL)
            {
                meStrcpy(p1,homedir) ;
                p1 += meStrlen(p1) - 1 ;
                if((p[0] != '\0') && (p[0] != DIR_CHAR))
                {
                    *p1++ = DIR_CHAR ;
                    *p1++ = '.' ;
                    *p1++ = '.' ;
                    *p1++ = DIR_CHAR ;
                }
            }
            else
                p-- ;
            meStrcpy(p1,p) ;
        }
    }
    else if(flag)
        meStrcpy(p1,p);
#ifdef _DRV_CHAR
    /* case for /xxxx... */
    else if(p[0] == DIR_CHAR)
    {
        if((p != oldName) && (oldName[1] == _DRV_CHAR))
            /* file name was D:xxxxxx//yyyyyy, convert to D:/yyyyyy */
            p1[0] = oldName[0] ;
        else
            /* file name is /yyyyyy, convert to D:/yyyyyy */
            p1[0] = curdir[0] ;
        p1[1] = _DRV_CHAR ;
        meStrcpy(p1+2,p) ;
    }
    /* case for xxxx... */
    else if(p[1] != _DRV_CHAR)
#else
    /* case for xxxx... */
    else if(p[0] != DIR_CHAR)
#endif
    {
        meStrcpy(p1,curdir) ;
        meStrcat(p1,p) ;
    }
#ifdef _DRV_CHAR
    /* case for D:xxxxxx */
    else if((p[1] == _DRV_CHAR) && (p[2] != DIR_CHAR) &&
            ((gwdbuf = gwd(p[0])) != NULL))
    {
        meStrcpy(p1,gwdbuf) ;
        meStrcat(p1,p+2) ;
        meFree(gwdbuf) ;
    }
#endif
    else
        meStrcpy(p1,p) ;
    p = NULL ;
    p1 = urle ;
    if((flag == 3) && !meStrncmp(p1,"/~/",3))
    {
        /* for ftp ~/ only, allow ftp://xxx/~/../../../yyy */
        p1 += 2 ;
        while(!meStrncmp(p1+1,"../",3))
            p1 = p1+3 ;
    }
    while((p1=meStrchr(p1,DIR_CHAR)) != NULL)
    {
        if((p1[1] == '.') && (p1[2] == '.') &&         /* got /../YYYY */
           ((p1[3] == DIR_CHAR) || ((nameType == PATHNAME_COMPLETE) && (p1[3] == '\0'))))
        {
            if(p == NULL)        /* got /../YYYY */
            {
#ifdef _DRV_CHAR
                /* check for X:/../YYYY */
                if(urle[1] == _DRV_CHAR)
                    p = urle+2 ;
                else
#endif
                    p = urle ;
            }
            /* else got /XXX/../YYYY */

            p1 += 3 ;
            while((*p++ = *p1++) != '\0')
                ;
            p = NULL ;
            p1 = urle ;
        }
        else if((p1[1] == '.') &&                      /* got /./YYYY */
                ((p1[2] == DIR_CHAR) || ((nameType == PATHNAME_COMPLETE) && (p1[2] == '\0'))))
        {
            meUByte *tt ;
            tt = p1+2 ;
            while((*p1++ = *tt++) != '\0')
                ;
            if(p == NULL)
                p1 = urle ;
            else
                p1 = p ;
        }
        else
        {
            p = p1 ;
            p1++ ;
        }
    }
#ifdef _SINGLE_CASE
    if(flag == 0)
        makestrlow(newName) ;
#endif
    if(baseName != NULL)
    {
        meUByte cc ;
        p = p1 = newName ;
        while((cc=*p1++) != '\0')
        {
            if((cc == DIR_CHAR) && (*p1 != '\0'))
                p = p1 ;
            else if((cc == '[') || (cc == '*') || ((cc == '?') && ((flag != 4) || (*p1 != DIR_CHAR))))
                /* this is base of a wild file base name break */
                break ;
        }
        *baseName = p ;
    }
}

#ifdef _WIN32
void
fileNameCorrect(meUByte *oldName, meUByte *newName, meUByte **baseName)
{
    meUByte *bn;

    pathNameCorrect(oldName,PATHNAME_COMPLETE,newName,&bn) ;
    if(baseName != NULL)
        *baseName = bn ;

    if(ffUrlTypeIsNotFile(ffUrlGetType(newName)))
        return ;

    /* ensure the drive letter is stable, make it lower case */
    if((newName[1] == _DRV_CHAR) && isLower(newName[0]))
        newName[0] = (newName[0] ^ 0x20);

    if(!fileNameWild(bn))
    {
        /* with windows naff file name case insensitivity we must get
         * the correct a single name letter case by using FindFirstFile*/
        HANDLE *handle;
        WIN32_FIND_DATA fd;
        if((handle = FindFirstFile((const char *) newName,&fd)) != INVALID_HANDLE_VALUE)
        {
            meStrcpy(bn,fd.cFileName) ;
            /* If a directory that append the '/' */
            if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                bn += meStrlen(bn) ;
                *bn++ = DIR_CHAR ;
                *bn = '\0' ;
            }
            FindClose (handle);
        }
    }
}
#endif

void
getDirectoryList(meUByte *pathName, meDirList *dirList)
{
#if MEOPT_REGISTRY
    meUByte upb[meBUF_SIZE_MAX] ;
#endif
    meUByte **fls=NULL, ft;
    int noFiles=0;
#ifdef _UNIX
    struct stat statbuf;
    meFiletime stmtime ;
#endif
#ifdef _WIN32
    meFiletime stmtime ;
    WIN32_FIND_DATA fd;
    HANDLE *handle;
#endif

    ft = ffUrlGetType(pathName);
    if(ffUrlTypeIsNotFile(ft))
    {
        if(dirList->path != NULL)
        {
            if(!meStrcmp(dirList->path,pathName))
                return;
            meFree(dirList->path);
            freeFileList(dirList->size,dirList->list);
        }
        dirList->path = meStrdup(pathName) ;
        dirList->size = 0 ;
        dirList->list = NULL ;
#if MEOPT_SOCKET
        if(ffUrlTypeIsFtp(ft))
        {
            meLine hlp, *lp ;
            meUByte *ss ;
            int ii ;
            hlp.next = &hlp ;
            hlp.prev = &hlp ;
            if(ffReadFile(&meior,pathName,meRWFLAG_READ|meRWFLAG_SILENT,NULL,&hlp,0,0,0) >= 0)
            {
                ii = 0;
                lp = hlp.next;
                while(lp != &hlp)
                {
                    ii++;
                    lp = lp->next;
                }
                if((ii > 0) &&
                   (meRegexComp(&meRegexStrCmp,(meUByte *) "\\([-\\a]+ +\\d+ \\S+\\( +\\S+\\)?\\( +\\d+,\\)? +\\d+\\( +0[xX]\\h+\\)? +\\a\\a\\a +\\d\\d? +\\(\\d\\d\\d\\d\\|\\d\\d?:\\d\\d\\)\\|\\d\\d-\\d\\d-\\d\\d +\\d\\d?\\:\\d\\d[aApP][mM] +\\(\\d+\\|<DIR>\\)\\) +\\(.*\\)",0) == meREGEX_OKAY) &&
                   ((fls = meMalloc(ii * sizeof(meUByte *))) != NULL))
                {
                    lp = hlp.next;
                    while (lp != &hlp)
                    {
                        if(meRegexMatch(&meRegexStrCmp,lp->text,meLineGetLength(lp),0,meLineGetLength(lp),meRSTRCMP_WHOLE))
                        {
                            ii = meRegexStrCmp.group[7].end - meRegexStrCmp.group[7].start ;
                            if((fls[noFiles] = meMalloc(ii+2)) == NULL)
                                break ;
                            meStrncpy(fls[noFiles],lp->text+meRegexStrCmp.group[7].start,ii) ;
                            if(lp->text[0] == 'd')
                                fls[noFiles][ii++] = '/' ;
                            else if((lp->text[0] == 'l') && ((ss = meStrstr(fls[noFiles]," -> ")) != NULL))
                                ii = ((size_t) ss) - ((size_t) fls[noFiles]) ;
                            else if(isDigit(lp->text[0]) && (lp->text[meRegexStrCmp.group[6].start+1] == 'D'))
                                fls[noFiles][ii++] = '/' ;
                            fls[noFiles++][ii] = '\0' ;
                        }
                        lp = lp->next;
                    }
                    if((noFiles == 0) || (lp != &hlp))
                    {
                        while(--noFiles >= 0)
                            meFree(fls[noFiles]);
                        meFree(fls);
                        noFiles = 0;
                    }
                }
                meLineLoopFree(&hlp,0) ;
            }
        }
#endif
#if MEOPT_TFS
        if(ffUrlTypeIsTfs(ft))
        {
            tfsMount *tfsh;
            meUByte *fn;
            if((fn = meStrstr(pathName,"?/")) != NULL)
            {
                *fn = '\0';
                tfsh = tfs_mount(pathName+6);
                *fn++ = '?';
            }
            else
            {
                tfsh = tfsdev;
                fn = pathName+5;
            }
            if(tfsh != NULL)
            {
                tfsDirectory *dirp;
                if((dirp = tfs_dopen(tfsh,fn)) != NULL)
                {
                    tfsDirent de;
                    meUByte *ff;
                    int rr, ll, isRoot=((fn[0]=='\0') || ((fn[0]=='/') && (fn[1]=='\0')));
                    if((fls = meMalloc(sizeof(meUByte *) * (dirp->dcount+dirp->fcount+((isRoot) ? 0:1)))) != NULL)
                    {
                        if(!isRoot && ((ff = meStrdup((meUByte *) "../")) != NULL))
                            fls[noFiles++] = ff;
                        while((rr=tfs_dread(dirp,&de)) > 0)
                        {
                            ll = de.nameLen;
                            if((ff = meMalloc(ll + ((de.mode & tfsSTMODE_DIRECTORY) ? 2:1))) == NULL)
                                break;
                            fls[noFiles++] = ff;
                            memcpy(ff,de.name,ll);
                            if(de.mode & tfsSTMODE_DIRECTORY)
                                ff[ll++] = '/';
                            ff[ll] = '\0';
                        }
                        if(rr != 0)
                        {
                            if(fls != NULL)
                            {
                                while(--noFiles >= 0)
                                    meFree(fls[noFiles]);
                                meFree(fls);
                            }
                            noFiles = 0;
                        }
                    }
                    tfs_dclose(dirp);
                }
                if(tfsh != tfsdev)
                    tfs_umount(tfsh);
            }
        }
#endif  /* MEOPT_TFS */
        if(noFiles > 0)
        {
            dirList->size = noFiles ;
            dirList->list = fls ;
#ifdef _INSENSE_CASE
            sortStrings(noFiles,fls,0,meStridif) ;
#else
            sortStrings(noFiles,fls,0,(meIFuncSS) strcmp) ;
#endif
        }
        return ;
    }

#if MEOPT_REGISTRY
    if((pathName[0] == '~') && (pathName[1] == '\0') && (homedir != NULL))
    {
        meUByte *ss ;
        int len ;

        meStrcpy(upb,homedir) ;
        len = meStrlen(upb) ;
        upb[len-1] = '\0' ;
        if((ss = meStrrchr(upb,DIR_CHAR)) != NULL)
        {
            ss[1] = '\0' ;
            len = meStrlen(upb) ;
        }
        else
            upb[len-1] = DIR_CHAR ;
        pathName = upb ;
    }
#endif
    {
#ifdef _UNIX
        if(!stat((char *)pathName,&statbuf))
            stmtime = statbuf.st_mtime ;
        else
            meFiletimeInit(stmtime) ;
#endif
#ifdef _WIN32
        meUByte *ss;
        int len ;

        if((pathName[0] == DIR_CHAR) && (pathName[1] == DIR_CHAR) &&
           (((ss=meStrchr(pathName+2,DIR_CHAR)) == NULL) ||
            (meStrchr(ss+1,DIR_CHAR) == NULL)))
        {
            if(dirList->path != NULL)
            {
                if(!meStrcmp(dirList->path,pathName) && ((ss == NULL) || (dirList->size > 0)))
                    return;
                meFree(dirList->path);
                freeFileList(dirList->size,dirList->list);
            }
            dirList->path = meStrdup(pathName);
            dirList->size = 0;
            dirList->list = NULL;
#if MEOPT_EXTENDED
            if(ss != NULL)
            {
                NETRESOURCE nr, *rr;
                HANDLE eh;

                pathName[0] = '\\';
                pathName[1] = '\\';
                *ss = '\0';
                memset(&nr,0,sizeof(NETRESOURCE));
                nr.dwType = RESOURCETYPE_DISK;
                nr.lpRemoteName = (char *) pathName;
                if(WNetOpenEnum(RESOURCE_GLOBALNET,RESOURCETYPE_ANY,0,&nr,&eh) == NO_ERROR)
                {
                    DWORD fc=-1, bs=meFIOBUFSIZ;
                    if((WNetEnumResource(eh,&fc,ffbuf,&bs) == NO_ERROR) && (fc > 0) &&
                       ((fls = meMalloc(sizeof(meUByte *) * fc)) != NULL))
                    {
                        int ll,ii=1+(int)(ss-pathName);
                        *ss = '\\';
                        len = meStrlen(pathName);
                        // May need to ignore if rr[bs].dwType == RESOURCETYPE_PRINT (2), but RESOURCETYPE_ANY (0) could be a disk (1) too
                        rr = (NETRESOURCE *) ffbuf;
                        for(bs=0; bs<fc; bs++)
                        {
                            if(!meStrncmp(rr[bs].lpRemoteName,pathName,len) &&
                               ((fls[noFiles] = meMalloc((ll=strlen(rr[bs].lpRemoteName+ii))+2)) != NULL))
                            {
                                memcpy(fls[noFiles],rr[bs].lpRemoteName+ii,ll);
                                fls[noFiles][ll] = DIR_CHAR;
                                fls[noFiles][ll+1] = '\0';
                                noFiles++;
                            }
                        }
                        if(noFiles > 0)
                        {
                            dirList->size = noFiles ;
                            dirList->list = fls ;
#ifdef _INSENSE_CASE
                            sortStrings(noFiles,fls,0,meStridif) ;
#else
                            sortStrings(noFiles,fls,0,(meIFuncSS) strcmp) ;
#endif
                        }
                        else
                            meFree(fls);
                    }
                    WNetCloseEnum(eh);
                }
                pathName[0] = DIR_CHAR;
                pathName[1] = DIR_CHAR;
                *ss = DIR_CHAR;
            }
#endif
            return;
        }
        meFiletimeInit(stmtime);
        if(((len = meStrlen(pathName)) > 0) && (pathName[len-1] == DIR_CHAR))
        {
            pathName[len-1] = '\0';
            if((handle = FindFirstFile((const char *) pathName,&fd)) != INVALID_HANDLE_VALUE)
            {
                stmtime.dwHighDateTime = fd.ftLastWriteTime.dwHighDateTime ;
                stmtime.dwLowDateTime = fd.ftLastWriteTime.dwLowDateTime ;
                FindClose(handle) ;
            }
            pathName[len-1] = DIR_CHAR ;
        }
#endif

        if((dirList->path != NULL) &&
           !meStrcmp(dirList->path,pathName) &&
#if (defined _UNIX) || (defined _WIN32)
           meFiletimeIsSame(stmtime,dirList->stmtime)
#else
           !dirList->stmtime
#endif
           )
            return;
    }

    /* free off the old */
    meNullFree(dirList->path) ;
    freeFileList(dirList->size,dirList->list) ;

#ifdef _DOS
    if(pathName[0] == '\0')
    {
        union REGS reg ;		/* cpu register for use of DOS calls */
        meUByte *ff ;
        int    ii ;

        for (ii = 1; ii <= 26; ii++)    /* Drives are a-z (1-26) */
        {
            reg.x.ax = 0x440e ;
            reg.h.bl = ii ;
            intdos(&reg,&reg) ;
            if((reg.x.cflag == 0) || (reg.x.ax != 0x0f))
            {
                if(((fls = meRealloc(fls,sizeof(meUByte *) * (noFiles+1))) == NULL) ||
                   ((ff = meMalloc(4*sizeof(meUByte))) == NULL))
                {
                    fls = NULL ;
                    noFiles = 0 ;
                    break ;
                }
                fls[noFiles++] = ff ;
                *ff++ = ii + 'a' - 1 ;
                *ff++ = _DRV_CHAR ;
                *ff++ = DIR_CHAR ;
                *ff   = '\0' ;
            }
        }
    }
    else
    {
        struct ffblk fblk ;
        meUByte *ff, *ee, es[4] ;
        int done ;

        /* append the *.* - Note that this function assumes the pathName has a '/' and
         * its an array with 3 extra char larger than the string size */
        ee = pathName + meStrlen(pathName) ;
        ee[0] = '*' ;
        es[1] = ee[1] ;
        ee[1] = '.' ;
        es[2] = ee[2] ;
        ee[2] = '*' ;
        es[3] = ee[3] ;
        ee[3] = '\0' ;
        done = findfirst((char *)pathName,&fblk,FA_RDONLY|FA_HIDDEN|FA_SYSTEM|FA_DIREC) ;
        ee[0] = '\0' ;
        ee[1] = es[1] ;
        ee[2] = es[2] ;
        ee[3] = es[3] ;
        while(!done)
        {
            if(((noFiles & 0x0f) == 0) &&
               ((fls = meRealloc(fls,sizeof(meUByte *) * (noFiles+16))) == NULL))
            {
                noFiles = 0 ;
                break ;
            }
            if((ff = meMalloc(strlen(fblk.ff_name)+2)) == NULL)
            {
                fls = NULL ;
                noFiles = 0 ;
                break ;
            }
            fls[noFiles++] = ff ;
            meStrcpy(ff,fblk.ff_name) ;
            makestrlow(ff) ;
            if(fblk.ff_attrib & FA_DIREC)
            {
                ff += meStrlen(ff) ;
                *ff++ = DIR_CHAR ;
                *ff   = '\0' ;
            }
            done = findnext(&fblk) ;
        }
    }
    dirList->stmtime = 0 ;
#endif
#ifdef _WIN32
    if(pathName[0] == '\0')
    {
        meUByte *ff;
        int ii;
        DWORD ad, dwMask;

        /* Get the drives */
        ad = GetLogicalDrives();

        /* Drives are a-z (bit positions 0-25) */
        for(ii=1, dwMask=1 ; ii <= 26; ii++, dwMask <<= 1)
        {
            if(ad & dwMask) /* Drive exists ?? */
            {
                /* Yes - add to the drive list */
                if(((fls = meRealloc(fls,sizeof(meUByte *) * (noFiles+1))) == NULL) ||
                   ((ff = meMalloc(4*sizeof(meUByte))) == NULL))
                {
                    fls = NULL ;
                    noFiles = 0 ;
                    break ;
                }
                fls[noFiles++] = ff;
                *ff++ = ii + 'A' - 1;
                *ff++ = _DRV_CHAR;
                *ff++ = DIR_CHAR;
                *ff   = '\0';
            }
        }
    }
    else
    {
        meUByte *ff, *ee, es[4] ;

        /* append the *.* - Note that this function assumes the pathName has a '/' and
         * its an array with 3 extra char larger than the string size */
        ee = pathName + meStrlen(pathName) ;
        ee[0] = '*' ;
        es[1] = ee[1] ;
        ee[1] = '.' ;
        es[2] = ee[2] ;
        ee[2] = '*' ;
        es[3] = ee[3] ;
        ee[3] = '\0' ;
        handle = FindFirstFile((const char *) pathName,&fd) ;
        ee[0] = '\0' ;
        ee[1] = es[1] ;
        ee[2] = es[2] ;
        ee[3] = es[3] ;
        if(handle != INVALID_HANDLE_VALUE)
        {
            do
            {
                if(((noFiles & 0x0f) == 0) &&
                   ((fls = meRealloc(fls,sizeof(meUByte *) * (noFiles+16))) == NULL))
                {
                    noFiles = 0 ;
                    break ;
                }
                if((ff = meMalloc(meStrlen(fd.cFileName)+2)) == NULL)
                {
                    fls = NULL ;
                    noFiles = 0 ;
                    break ;
                }
                fls[noFiles++] = ff ;
                meStrcpy(ff,fd.cFileName) ;
                if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    ff += meStrlen(ff) ;
                    *ff++ = DIR_CHAR ;
                    *ff   = '\0' ;
                }
            } while (FindNextFile (handle, &fd));
            FindClose (handle);
        }
    }
    dirList->stmtime.dwHighDateTime = stmtime.dwHighDateTime ;
    dirList->stmtime.dwLowDateTime = stmtime.dwLowDateTime ;
#endif
#ifdef _UNIX
    {
        DIR    *dirp ;

        if((dirp = opendir((char *)pathName)) != NULL)
        {
#if (defined _DIRENT)
            struct  dirent *dp ;
#else
            struct  direct *dp ;
#endif
            struct stat statbuf;
            meUByte *ff, *bb, fname[meBUF_SIZE_MAX] ;

            meStrcpy(fname,pathName) ;
            bb = fname + meStrlen(fname) ;

            while((dp = readdir(dirp)) != NULL)
            {
                if(((noFiles & 0x0f) == 0) &&
                   ((fls = meRealloc(fls,sizeof(meUByte *) * (noFiles+16))) == NULL))
                {
                    noFiles = 0 ;
                    break ;
                }
                if((ff = meMalloc(meStrlen(dp->d_name)+2)) == NULL)
                {
                    fls = NULL ;
                    noFiles = 0 ;
                    break ;
                }
                fls[noFiles++] = ff ;
                meStrcpy(ff,dp->d_name) ;
                if((ff[0] == '.') && ((ff[1] == '\0') || ((ff[1] == '.') && (ff[2] == '\0'))))
                {
                    ff += (ff[1] == '\0') ? 1:2 ;
                    *ff++ = DIR_CHAR ;
                    *ff   = '\0' ;
                }
#ifdef DT_DIR
                else if(dp->d_type == DT_DIR)
                {
                    ff += meStrlen(ff) ;
                    *ff++ = DIR_CHAR ;
                    *ff   = '\0' ;
                }
                else if((dp->d_type == DT_UNKNOWN) || (dp->d_type == DT_LNK))
#else
                else
#endif
                {
                    meStrcpy(bb,dp->d_name) ;
                    if(!stat((char *)fname,&statbuf) && S_ISDIR(statbuf.st_mode))
                    {
                        ff += meStrlen(ff) ;
                        *ff++ = DIR_CHAR ;
                        *ff   = '\0' ;
                    }
                }
            }
            closedir(dirp) ;
        }
        dirList->stmtime = stmtime ;
    }
#endif  /* _UNIX */

#if MEOPT_REGISTRY
    if(pathName == upb)
    {
        meRegNode *rNd ;
        meUByte *ff ;
        int len ;

        /* add the alias/abbrev paths to the list */
        if(((rNd = regFind(NULL,(meUByte *)"history/" meSYSTEM_NAME "/alias-path")) != NULL) &&
           ((rNd = regGetChild(rNd)) != NULL))
        {
            do
            {
                if(((noFiles & 0x0f) == 0) &&
                   ((fls = meRealloc(fls,sizeof(meUByte *) * (noFiles+16))) == NULL))
                {
                    noFiles = 0 ;
                    break ;
                }
                len = meStrlen(rNd->name) ;
                if((ff = meMalloc(len+2)) == NULL)
                {
                    fls = NULL ;
                    noFiles = 0 ;
                    break ;
                }
                fls[noFiles++] = ff ;
                meStrcpy(ff,rNd->name) ;
                ff[len] = DIR_CHAR ;
                ff[len+1] = '\0' ;
            } while((rNd = regGetNext(rNd)) != NULL) ;
        }
        pathName = (meUByte *) "~" ;
    }
#endif

    dirList->path = meStrdup(pathName) ;
    dirList->size = noFiles ;
    dirList->list = fls ;
    if(noFiles > 1)
#ifdef _INSENSE_CASE
        sortStrings(noFiles,fls,0,meStridif) ;
#else
        sortStrings(noFiles,fls,0,(meIFuncSS) strcmp) ;
#endif
}

/*
** frees the file list created by getFileList
*/
void
freeFileList(int noStr, meUByte **files)
{
    if(files == NULL)
        return ;

    while (--noStr >= 0)
        meFree(files[noStr]) ;

    meFree((void *) files) ;
}
