/*****************************************************************************
*
*	Title:		%M%
*
*	Synopsis:	File handling.
*
******************************************************************************
*
*	Filename:		%P%
*
*	Author:			Danial Lawrence
*
*	Creation Date:		14/05/86 12:37		<010915.2110>
*
*	Modification date:	%G% : %U%
*
*	Current rev:		%I%
*
*	Special Comments:
*
*	Contents Description:
*
*	The routines in this file handle the reading, writing
*	and lookup of disk files.  All of details about the
*	reading and writing of the disk are in "fileio.c".
*
* Jon Green	13-07-90
* Added Document Mode. Search for .doc or .txt extension for WRAP and
* indent to automatically be enabled.
*
* Jon Green	17-05-91
* Added suffix modes so that the suffix defintions may be defined in the
* start up file.
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

/*--- Include defintions */

#define __FILEC 1                  /* Define the filename */

/*--- Include files */

#include "emain.h"
#include "efunc.h"
#if (defined _UNIX) || (defined _DOS) || (defined _WIN32)
#include <errno.h>
#include <limits.h>                     /* Constant limit definitions */
#include <sys/types.h>
#include <sys/stat.h>
#ifdef _WIN32 
#include <direct.h>                     /* Directory entries */
#elif (defined _DIRENT)
#include <dirent.h>
#else
#include <sys/dir.h>
#endif
#include <time.h>
#endif

#ifdef _DOS
#include <sys/file.h>
#include <dir.h>
#include <errno.h>
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


int
getFileStats(uint8 *file, int flag, meSTAT *stats, uint8 *lname)
{
    if(lname != NULL)
        *lname = '\0' ;
    /* setup the default stat values */
    if(stats != NULL)
    {
        stats->stmtime= -1 ;
        stats->stsize = 0 ;
#ifdef _UNIX
        stats->stuid  = meUid ;
        stats->stgid  = meGid ;
        stats->stdev  = -1 ;
        stats->stino  = -1 ;
#endif
    }
    if(isHttpLink(file))
        return meFILETYPE_HTTP ;
    if(isFtpLink(file))
        return meFILETYPE_FTP ;
    if(isUrlFile(file))
        /* skip the file: */
        file += 5 ;

#ifdef _DOS
    {
        union REGS reg ;                /* cpu register for use of DOS calls */
        int        len ;
        
        if(((len = meStrlen(file)) == 0) ||
           (strchr(file,'*') != NULL) ||
           (strchr(file,'?') != NULL))
        {
            if(flag & 1)
                mlwrite(MWABORT|MWPAUSE,"[%s illegal name]", file);
            return meFILETYPE_NASTY ;
        }
        if((file[len-1] == DIR_CHAR) || ((len == 2) && (file[1] == _DRV_CHAR)))
            goto gft_directory ;
        reg.x.ax = 0x4300 ;
        reg.x.dx = ((unsigned long) file) ;
        intdos(&reg, &reg);
        
        if(reg.x.cflag)
            return meFILETYPE_NOTEXIST ;
        if(stats != NULL)
        {
            struct ffblk fblk ;
            stats->stmode = (reg.x.cx|0x020) ;
            if(!findfirst(file, &fblk, FA_RDONLY|FA_HIDDEN|FA_SYSTEM|FA_DIREC))
            {
                stats->stmtime = (((uint32) fblk.ff_ftime) & 0x0ffff) | (((uint32) fblk.ff_fdate) << 16) ;
                stats->stsize = fblk.ff_fsize ;
            }
        }
        if(reg.x.ax & 0x010)
        {
gft_directory:
            if(flag & 2)
                mlwrite(MWABORT|MWPAUSE,"[%s directory]", file) ;
            return meFILETYPE_DIRECTORY ;
        }
        return meFILETYPE_REGULAR ;
    }
#else
#ifdef _WIN32
    {
        DWORD status;
        int   len ;
        
        if(((len = meStrlen(file)) == 0) || 
           (strchr(file,'*') != NULL) || (strchr(file,'?') != NULL))
        {
            if(flag & 1)
                mlwrite(MWABORT|MWPAUSE,"[%s illegal name]", file);
            return meFILETYPE_NASTY ;
        }
        if((file[len-1] == DIR_CHAR) || ((len == 2) && (file[1] == _DRV_CHAR)))
            goto gft_directory ;
        
        if(stats != NULL)
        {
            HANDLE          *fh ;
            WIN32_FIND_DATA  fd ;
            
            fh = FindFirstFile(file,&fd) ;
            if(fh == INVALID_HANDLE_VALUE)
                return meFILETYPE_NOTEXIST ;
            status = fd.dwFileAttributes ;
            /* Dangerously ingoring the nFileSizeHigh for now */
            stats->stsize = fd.nFileSizeLow ;
            stats->stmtime = ((fd.ftLastWriteTime.dwLowDateTime  >> 24) & 0x000000ff) |
                      ((fd.ftLastWriteTime.dwHighDateTime <<  8) & 0x7fffff00) ;
            FindClose(fh) ;
            stats->stmode = (uint16) status | FILE_ATTRIBUTE_ARCHIVE ;
        }
        else if((status = GetFileAttributes(file)) == 0xFFFFFFFF)
            return meFILETYPE_NOTEXIST ;
        if (status & FILE_ATTRIBUTE_DIRECTORY)
        {
gft_directory:
            if(flag & 2)
                mlwrite(MWABORT|MWPAUSE,"[%s directory]", file);
            return meFILETYPE_DIRECTORY ;
        }
#if 0
        /* FILE_ATTRIBUTE_SYSTEM
         * The file or directory is part of, or is used exclusively by,
         * the operating system. */
        else if (status & FILE_ATTRIBUTE_SYSTEM)
        {
            if(flag & 1)
                mlwrite(MWABORT|MWPAUSE,"[%s operating system file]", file);
            return meFILETYPE_NASTY ;
        }
#endif
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
        return meFILETYPE_REGULAR ;
    }
#else
#ifdef _UNIX
    {
        struct stat statbuf;
        long stmtime = -1 ;
        
        if((lname == NULL) && (stats == NULL))
        {
            if(stat((char *)file, &statbuf) != 0)
                return meFILETYPE_NOTEXIST ;
        }
        else if(lstat((char *)file, &statbuf) != 0)
            return meFILETYPE_NOTEXIST ;
        else if(S_ISLNK(statbuf.st_mode))
        {
            uint8 lbuf[FILEBUF], buf[FILEBUF], *ss ;
            int ii, jj ;
            
            ii = meStrlen(file) ;
            meStrncpy(lbuf,file,ii) ;
            do {
                if(file[ii-1] == DIR_CHAR)
                    ii-- ;
                lbuf[ii] = '\0' ;
                if(statbuf.st_mtime > stmtime)
                    stmtime = statbuf.st_mtime ;
                if((jj=readlink((char *)lbuf,(char *)buf, FILEBUF)) <= 0)
                {
                    if(flag & 1)
                        mlwrite(MWABORT|MWPAUSE,(uint8 *)"[%s symbolic link problems]", file);
                    return meFILETYPE_NASTY ;
                }
                buf[jj] = '\0' ;
                if((buf[0] == DIR_CHAR) || ((ss=meStrrchr(lbuf,DIR_CHAR)) == NULL))
                    meStrcpy(lbuf,buf) ;
                else
                {
                    ss++ ;
                    ii = ((int) ss) - ((int) lbuf) ;
                    meStrcpy(ss,buf) ;
                    ii += jj ;
                }
            } while(((jj=lstat((char *)lbuf, &statbuf)) == 0) && S_ISLNK(statbuf.st_mode)) ;
            if(lname != NULL)
                fileNameCorrect(lbuf,lname,NULL) ;
            if(jj)
            {
                if(flag & 1)
                    mlwrite(MWABORT|MWPAUSE,(uint8 *)"[%s dangling symbolic link]",lbuf);
                if(stats != NULL)
                    stats->stmtime = stmtime ;
                return meFILETYPE_NOTEXIST ;
            }
        }
        
        if(stats != NULL)
        {
            stats->stmode = statbuf.st_mode ;
            stats->stuid  = statbuf.st_uid ;
            stats->stgid  = statbuf.st_gid ;
            stats->stdev  = statbuf.st_dev ;
            stats->stino  = statbuf.st_ino ;
            stats->stsize = statbuf.st_size ;
            if(statbuf.st_mtime > stmtime)
                stats->stmtime= statbuf.st_mtime ;
            else
                stats->stmtime= stmtime ;
        }
        if(S_ISREG(statbuf.st_mode))
        {
            return meFILETYPE_REGULAR ;
        }
        if(S_ISDIR(statbuf.st_mode))
        {
            if(flag & 2)
                mlwrite(MWABORT|MWPAUSE,(uint8 *)"[%s directory]", file);
            return meFILETYPE_DIRECTORY ;
        }
        if(flag & 1)
        {
#ifdef S_IFIFO
            if(S_ISFIFO(statbuf.st_mode))
                mlwrite(MWABORT|MWPAUSE,(uint8 *)"[%s is a FIFO]", file);
#endif
#ifdef S_IFCHR
            else if(S_ISCHR(statbuf.st_mode))
                mlwrite(MWABORT|MWPAUSE,(uint8 *)"[%s is character special]", file);
#endif
#ifdef S_IFBLK
            else if(S_ISBLK(statbuf.st_mode))
                mlwrite(MWABORT|MWPAUSE,(uint8 *)"[%s is block special]", file);
#endif
            else
                mlwrite(MWABORT|MWPAUSE,(uint8 *)"[%s not a regular file]", file);
        }
        return meFILETYPE_NASTY ;
    }
#else
    return meFILETYPE_REGULAR ;
#endif /* _UNIX */
#endif /* _WIN32 */
#endif /* _DOS */
}

int
fnamecmp(uint8 *f1, uint8 *f2)
{
    if((f1==NULL) || (f2==NULL))
        return 1 ;
#ifdef _INSENSE_CASE
    return meStricmp(f1,f2) ;
#else
    return meStrcmp(f1,f2) ;
#endif
}

/*--- Set up directories. */

void
set_dirs(void)
{
#if (defined _UNIX) || (defined _DOS) || (defined _WIN32)
#ifdef _UNIX
#ifdef _SEARCH_PATH
    static uint8 lpath[] = _SEARCH_PATH ;
#else
    static uint8 lpath[] = "/usr/local/microemacs" ;
#endif
#endif    
    uint8 *s1 ;
#ifdef _UNIX
    uint8 *s2 ;
    int ll ;
#endif
    
    s1 = meGetenv("HOME");                      /* Get home directory */
    if (s1 != NULL)			        /* Null ?? */
    {
        homedir = meStrdup(s1) ;
        fileNameConvertDirChar(homedir) ;
    }
    else
        homedir = NULL ;			/* Set to NULL */
#ifdef _UNIX
    s1 = meGetenv("MEPATH");			/* Get home directory */
    if(s1 == NULL)
    {
        s1 = lpath ;
        s2 = meGetenv("PATH");			/* Get home directory */
    }
    else
        s2 = lpath ;
    ll = meStrlen(s1) + 1 ;
    if(s2 != NULL)
        ll += meStrlen(s2) + 1 ;
        
    if((searchPath = malloc(ll)) != NULL)
    {
        meStrcpy(searchPath,s1) ;
        if(s2 != NULL)
        {
            ll = meStrlen(s1) ;
            searchPath[ll++] = PATHCHR ;
            meStrcpy(searchPath+ll,s2) ;
        }
    }
#else
    s1 = meGetenv("MEPATH");			/* Get home directory */
    if(s1 == NULL)
        s1 = meGetenv("PATH");			/* Get home directory */
    if(s1 == NULL)
        searchPath = NULL ;
    else
        searchPath = meStrdup(s1);
#endif
#else
    homedir = NULL ;				/* Set to NULL */
    searchPath = NULL ;		        	/* Set to NULL */
#endif
#ifdef _UNIX
    {
        uint8 *pwd;
        struct stat dotstat, pwdstat;

        /* If PWD is accurate, use it instead of calling gwd.
         * This solves problems with bad ~/ as the shell will
         * store as /user/.... etc.
         */
        if(((pwd = meGetenv("PWD")) != NULL) &&
           (pwd[0] == DIR_CHAR) && (stat((char *)pwd,&pwdstat) == 0) &&
           (stat(".", &dotstat) == 0) && (dotstat.st_ino == pwdstat.st_ino) &&
           (dotstat.st_dev == pwdstat.st_dev))
        {
            uint8 cc, *dd ;
            int ll=meStrlen(pwd) ;
            
            curdir = dd = meMalloc(ll+2) ;
            while((cc=*pwd++) != '\0')
                *dd++ = cc ;
            if(dd[-1] != DIR_CHAR)
                *dd++ = DIR_CHAR ;
            *dd = 0 ;
        }
        else
            curdir = gwd(0) ;
    }
#else    
    curdir = gwd(0) ;
#endif
    if(curdir == NULL)
    {
        printf("Failed to get cwd\n") ;
        meExit(1);
    }
}   /* End of "set_dirs" () */

/* Look up the existance of a file along the normal or PATH
 * environment variable. Look first in the HOME directory if
 * asked and possible
 */

int
fileLookup(uint8 *fname, uint8 *ext, uint8 flags, uint8 *outName)
{
    register uint8 *path;  /* environmental PATH variable */
    register uint8 *sp;    /* pointer into path spec */
    register int   ii;     /* index */
    uint8 nname[FILEBUF] ;
    uint8 buf[FILEBUF] ;
 
    if(ext != NULL)
    {
        if((sp=meStrrchr(fname,DIR_CHAR)) == NULL)
            sp = fname ;
        /* Jon 990608 - If the extension is the same then we do not
         * concatinate it, otherwise we do !!. Compare the complete extension
         * with the end of the string, allowing double extensions to be
         * handled properly. */
        if (((ii = meStrlen (sp) - meStrlen(ext)) < 0) ||
 #ifdef _INSENSE_CASE
              meStricmp(&sp[ii], ext)
 #else
              meStrcmp(&sp[ii], ext)
 #endif
            )
        {
            meStrcpy(nname,fname) ;
            meStrcat(nname,ext);
            fname = nname ;
        }
    }
    if(flags & meFL_CHECKDOT)
    {
        fileNameCorrect(fname,outName,NULL) ;
#ifdef _UNIX
        if(flags & meFL_EXEC)
        {
            if(!meTestExec(outName) &&
               (getFileStats(outName,0,NULL,NULL) == meFILETYPE_REGULAR))
            return 1 ;
        }
        else
#endif
            if(!meTestExist(outName))
                return 1 ;
    }
    if(flags & meFL_USEPATH)
        path = meGetenv("PATH") ;
    else if(flags & meFL_USESRCHPATH)
        path = searchPath ;
    else
        return 0 ;
    while((path != NULL) && (*path != '\0'))
    {
        /* build next possible file spec */
        sp = path ;
        if((path = meStrchr(++path,PATHCHR)) != NULL)
        {
            ii = path++ - sp ;
            meStrncpy(buf,sp,ii) ;
        }
        else
        {
            meStrcpy(buf,sp) ;
            ii = meStrlen(buf) ;
        }
        /* Check for zero length path */
        if (ii <= 0)
            continue;
        /* Add a directory separator if missing */
        if (buf[ii-1] != DIR_CHAR)
            buf[ii++] = DIR_CHAR ;
        meStrcpy(buf+ii,fname) ;               /* Concatinate the name */
        
        /* and try it out */
        fileNameCorrect(buf,outName,NULL) ;
#ifdef _UNIX
        if(flags & meFL_EXEC)
        {
            if(!meTestExec(outName) &&
               (getFileStats(outName,0,NULL,NULL) == meFILETYPE_REGULAR))
            return 1 ;
        }
        else
#endif
            if(!meTestExist(outName))
                return 1 ;
    }
    return 0 ; /* no such luck */
}


int
executableLookup(uint8 *fname, uint8 *outName)
{
#if (defined _WIN32) || (defined _DOS)
    uint8 *ss ;
    int ll=meStrlen(fname) ;
    int ii ;
    
    
    if(ll > 4)
    {
        ss = fname+ll-4 ;
        ii = noExecExtensions ;
        while(--ii >= 0)
#ifdef _INSENSE_CASE
            if(!meStrnicmp(ss,execExtensions[ii],4))
#else
            if(!meStrncmp(ss,execExtensions[ii],4))
#endif
            {
                if(fileLookup(fname,NULL,meFL_CHECKDOT|meFL_USEPATH,outName))
                    return ii+1 ;
                return 0 ;
            }
    }
    ii=noExecExtensions ;
    while(--ii >= 0)
        if(fileLookup(fname,execExtensions[ii],meFL_CHECKDOT|meFL_USEPATH,outName))
            return ii+1 ;
#endif
#ifdef _UNIX
    uint8 flags ;
    
    flags = (meStrchr(fname,DIR_CHAR) != NULL) ? meFL_CHECKDOT|meFL_EXEC:meFL_USEPATH|meFL_EXEC ;
    if(fileLookup(fname,NULL,flags,outName))
        return 1 ;
#endif
    return 0 ;
}

int
bufferOutOfDate(BUFFER *bp)
{
    meSTAT stats ;
    if((bp->b_fname != NULL) && getFileStats(bp->b_fname,0,&stats,NULL) &&
       (stats.stmtime > bp->stats.stmtime))
        return TRUE ;
    return FALSE ;
}

/* get current working directory
*/
uint8 *
gwd(uint8 drive)
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

    uint8 dir[MAXBUF] ;
    register int ll ;

#ifdef _UNIX
    meGetcwd(dir,MAXBUF-2) ;
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
        drive = tolower (drive) - 'a' + 1;
        if ((drive == curDrive) || (_chdrive (drive) != 0))
            drive = 0;             /* Failed drive change or same drive */
        else
            drive = curDrive;      /* Save to restore */
    }

    /* Pick up the directory information */
    GetCurrentDirectory (MAXBUF, dir);
    if (meStrlen(dir) > 2)
    {
        uint8 *p;                   /* Local character pointer */
    
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
    union REGS reg ;                /* cpu register for use of DOS calls */

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

#if DIR_CHAR != '/'
    /* convert all '/' to '\' for dos etc */
    p1 = dir+3 ;
    while((p1=strchr(p1,'/')) != NULL)  /* got a '/', -> '\' */
        *p1++ = DIR_CHAR ;
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

uint8 *
getFileBaseName(uint8 *fname)
{
    uint8 cc, *p, *p1 ;
    p = p1 = fname ;
    while((cc=*p1++) != '\0')
    {
        if((cc == DIR_CHAR) && (*p1 != '\0'))
            p = p1 ;
    }
    return p ;
}

void
getFilePath(uint8 *fname, uint8 *path)
{
    if(fname != NULL)
    {
        uint8 *ss ;
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
inputFileName(uint8 *prompt, uint8 *fn, int corFlag)
{
    uint8 tmp[FILEBUF], *buf ;
    int  s ;
    
    buf = (corFlag) ? tmp:fn ;

    getFilePath(curbp->b_fname,buf) ;
    s = meGetString(prompt,(MLFILECASE|MLNORESET|MLMACNORT), 0, buf, FILEBUF) ;
    if(corFlag && (s == TRUE))
        fileNameCorrect(tmp,fn,NULL) ;
    return s ;
}


#if CRYPT
int
resetkey(BUFFER *bp) /* reset the encryption key if needed */
{
    /* if we are in crypt mode */
    if(meModeTest(bp->b_mode,MDCRYPT))
    {
        if((bp->b_key == NULL) &&
           (setBufferCryptKey(bp,NULL) != TRUE))
            return FALSE ;

        /* and set up the key to be used! */
        meCrypt(NULL, 0);
        meCrypt(bp->b_key, meStrlen(bp->b_key));

    }
    return TRUE ;
}
#endif

#define FILENODE_ATTRIBLEN 4

typedef struct FILENODE {
    struct FILENODE *next;              /* Next node in the list */
    uint8  *fname;                        /* Name of the file */
    uint8  *lname;                        /* Linkname */
    uint8   attrib[FILENODE_ATTRIBLEN] ;
    int32   size ;
#ifdef _UNIX
    time_t  mtime ;
#endif
#ifdef _DOS
    time_t  mtime ;
#endif
#ifdef _WIN32
    FILETIME mtime ;
#endif
} FILENODE;

FILENODE *curHead ;

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

#if (defined _WIN32) || (defined _DOS)
int
meTestExecutable(uint8 *fileName)
{
    int ii ;
    if((ii = strlen(fileName)) > 4)
    {
        uint8 *ee ;
        ee = fileName+ii-4 ;
        ii = noExecExtensions ;
        while(--ii >= 0)
#ifdef _INSENSE_CASE
            if(!meStrnicmp(ee,execExtensions[ii],4))
#else
            if(!meStrncmp(ee,execExtensions[ii],4))
#endif
                return 1 ;
    }
    return 0 ;
}
#endif

#ifdef _UNIX
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

static FILENODE *
getDirectoryInfo(uint8 *fname)
{
    uint8 bfname[MAXBUF];                 /* Working file name buffer */
    uint8 *fn;
    FILENODE *curFile ;
    FILENODE *cfh=NULL ;
    int noFiles=0;                        /* No files count */
    int ii;                               /* Local loop counter */
    
#ifdef _UNIX
    DIR    *dirp ;
#if (defined _DIRENT)
    struct  dirent *dp ;
#else
    struct  direct *dp ;
#endif
    struct stat sbuf ;
    uint8 *ff;
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
            if((ff = meStrdup(dp->d_name)) == NULL)
                return NULL ;
            curFile->lname = NULL ;
            curFile->fname = ff ;
            meStrcpy(ff,dp->d_name) ;
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
            {
                /* Check if its a symbolic link */
                uint8 link[1024];
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
            curFile->size = sbuf.st_size ;
            curFile->mtime = sbuf.st_mtime ;
        }
        closedir(dirp) ;
    }
#endif
#ifdef _DOS
    static uint8 *searchString = "*.*";
    struct ffblk fblk ;
    uint8 *ff ;
    int done ;

    /* Render the list of files. */
    curHead = NULL ;
    meStrcpy(bfname,fname) ;
    fn = bfname+meStrlen(bfname) ;
    meStrcpy(fn,searchString) ;
    done = findfirst(bfname,&fblk,FA_RDONLY|FA_HIDDEN|FA_SYSTEM|FA_DIREC) ;
    while(!done)
    {
        if(((noFiles & 0x0f) == 0) &&
           ((curHead = (FILENODE *) meRealloc(curHead,sizeof(FILENODE)*(noFiles+16))) == NULL))
            return NULL ;
        curFile = &(curHead[noFiles++]) ;
        if((ff = meStrdup(fblk.ff_name)) == NULL)
            return NULL ;
        makestrlow(ff) ;
        curFile->fname = ff ;
        curFile->lname = NULL ;
        curFile->size  = fblk.ff_fsize ;
        curFile->mtime = (((uint32) fblk.ff_ftime) & 0x0ffff) | (((uint32) fblk.ff_fdate) << 16) ;
        /* construct attribute string */
        if(fblk.ff_attrib & FA_DIREC)
            meStrncpy(curFile->attrib,"drwx",4) ;
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
    static uint8 *searchString = "*.*";
    HANDLE          *fh ;
    WIN32_FIND_DATA  fd ;
    uint8           *ff ;
    
    /* Render the list of files. */
    curHead = NULL ;
    meStrcpy(bfname,fname) ;
    fn = bfname+meStrlen(bfname) ;
    meStrcpy(fn,searchString) ;
    if((fh = FindFirstFile(bfname,&fd)) != INVALID_HANDLE_VALUE)
    {
        do
        {
            if(((noFiles & 0x0f) == 0) &&
               ((curHead = (FILENODE *) meRealloc(curHead,sizeof(FILENODE)*(noFiles+16))) == NULL))
                return NULL ;
            curFile = &(curHead[noFiles++]) ;
            if((ff = meStrdup(fd.cFileName)) == NULL)
                return NULL ;
            curFile->fname = ff ;
            curFile->lname = NULL ;
            curFile->size  = (((uint32) fd.nFileSizeHigh) << 16) + ((uint32) fd.nFileSizeLow) ;
            curFile->mtime.dwLowDateTime  = fd.ftLastWriteTime.dwLowDateTime ;
            curFile->mtime.dwHighDateTime = fd.ftLastWriteTime.dwHighDateTime ;
            /* construct attribute string */
            if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                meStrncpy(curFile->attrib,"drwx",4) ;
                /* On network drives the size is sometimes invalid. Clear
                 * it just to make sure that this is not the case */
                curFile->size = 0;
            }
            else
            {
                curFile->attrib[0] = '-' ;
                curFile->attrib[1] = 'r' ;
                curFile->attrib[2] = (fd.dwFileAttributes & FILE_ATTRIBUTE_READONLY) ? '-' : 'w' ;
                curFile->attrib[3] = '-' ;
                if(meTestExecutable(ff))
                    curFile->attrib[3] = 'x' ;
            }
        } while (FindNextFile(fh, &fd));
        FindClose(fh);
    }
#endif
    curFile = curHead ;
    /* Build a profile of the current directory. */
    for(ii=0 ; ii<noFiles ; ii++)
    {
        cfh = addFileNode(cfh,curFile) ;
        curFile++ ;
    }
    return cfh ;
}

static int
readDirectory(BUFFER *bp, uint8 *fname)
{
    FILENODE *fnode, *fn ;
#ifdef _UNIX
    struct tm *tmp;
#endif
#ifdef _WIN32
    SYSTEMTIME tmp;
#endif
    uint8 buf[MAXBUF];                  /* Working line buffer */
    int32 totSize=0 ;
    int len, dirs=0, files=0 ;

    if((fnode=getDirectoryInfo(fname)) == NULL)
        return ABORT ;
    sprintf ((char *)buf, "Directory listing of: %s", fname);
    addLineToEob(bp,buf);
    fn = fnode ;
    while(fn != NULL)
    {
        totSize += fn->size ;
        if(fn->attrib[0] == 'd')
            dirs++ ;
        else
            files++ ;
        fn = fn->next ;
    }
    buf[0] = ' ' ;
    buf[1] = ' ' ;
    len = 2 ;
    if (totSize > 9999999)
        len += sprintf ((char *)buf+len, "%7ldK ", totSize/1024);
    else
        len += sprintf ((char *)buf+len, "%7ld  ", totSize);
    sprintf((char *)buf+len, "used in %d files and %d dirs",files,dirs) ;
    addLineToEob(bp,buf);
    addLineToEob(bp,(uint8 *)"");                   /* Blank line */
    while(fnode != NULL)
    {
        len = 0;                /* Reset to the start of the line */
        buf[len++] = ' ';
        meStrncpy(buf+len,fnode->attrib,FILENODE_ATTRIBLEN) ;
        len += FILENODE_ATTRIBLEN ;
        buf[len++] = ' ' ;
        /* Add the file statistics */
        if (fnode->size > 9999999)
            len += sprintf((char *)buf+len, "%7ldK ", fnode->size/1024);
        else
            len += sprintf((char *)buf+len, "%7ld  ", fnode->size);
        
#ifdef _UNIX
        if ((tmp = localtime(&fnode->mtime)) != NULL)
            len += sprintf((char *)buf+len, "%4d/%02d/%02d %02d:%02d ",
                           tmp->tm_year+1900, tmp->tm_mon+1, tmp->tm_mday,
                           tmp->tm_hour, tmp->tm_min);

        else
#endif
#ifdef _DOS
        if((fnode->mtime & 0x0ffff) != 0x7fff)
            len += sprintf(buf+len, "%4d/%02d/%02d %02d:%02d ",
                           (int) ((fnode->mtime >> 25) & 0x007f)+1980,
                           (int) ((fnode->mtime >> 21) & 0x000f),
                           (int) ((fnode->mtime >> 16) & 0x001f),
                           (int) ((fnode->mtime >> 11) & 0x001f),
                           (int) ((fnode->mtime >>  5) & 0x003f)) ;
        else
#endif
#ifdef _WIN32
        if(FileTimeToSystemTime(&fnode->mtime,&tmp))
            len += sprintf(buf+len, "%4d/%02d/%02d %02d:%02d ",
                           tmp.wYear,tmp.wMonth,tmp.wDay,tmp.wHour,tmp.wMinute) ;
        else
#endif
            len += sprintf((char *)buf+len, "--/--/-- --:-- ") ;
            
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
            
        addLineToEob(bp,buf);
        free(fnode->fname) ;
        fnode = fnode->next ;
    }
    free(curHead) ;
    meModeSet(bp->b_mode,MDVIEW) ;
    meModeClear(bp->b_mode,MDATSV) ;
    meModeClear(bp->b_mode,MDUNDO) ;
    return TRUE ;
}



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
 * NULL and bp->b_fname != fname) so fname should be dynamically allocated
 * or the file name reset after readin.
 */
/* bp        buffer to load into */
/* fname     name of file to read */
/* lockfl    check for file locks? */
/* frompipe  input coming from pipe. Dont open file */
int
readin(register BUFFER *bp, uint8 *fname)
{
    int   ss=ABORT ;
    uint8 lfn[FILEBUF], afn[FILEBUF], *fn=fname ;
    
#if CRYPT
    if(resetkey(bp) != TRUE)
        return ABORT ;
#endif
    if(bp->b_fname != fname)
    {
        meNullFree(bp->b_fname) ;
        bp->b_fname = fname ;
    }
    if(fn != NULL)
    {
        meSTAT stats ;
        int ft ;
        if((ft=getFileStats(fn,1,&(bp->stats),lfn)) == meFILETYPE_HTTP)
            meModeSet(bp->b_mode,MDVIEW) ;
        else if(ft != meFILETYPE_FTP)
        {
            if(ft == meFILETYPE_NASTY)
                goto error_end ;
            if(lfn[0] != '\0')
                /* something was found, don't want to do RCS on this,
                 * the link may be dangling which returns 3 */
                fn = lfn ;
#if DORCS
            else if((ft == meFILETYPE_NOTEXIST) &&       /* file not found */
               (rcsFilePresent(fn) == TRUE))
            {
                if(doRcsCommand(fn,rcsCoStr) != TRUE)
                    goto error_end ;
                if((ft=getFileStats(fn,1,&(bp->stats),lfn)) == meFILETYPE_NASTY)
                    goto error_end ;
            }
#endif
            if((autotime > 0) && !createBackupName(afn,fn,'#',0) &&
               (getFileStats(afn,0,&stats,NULL) == meFILETYPE_REGULAR) && (stats.stmtime > bp->stats.stmtime))
            {
                uint8 prompt[200] ;
                uint8 *tt ;
                tt = getFileBaseName(afn) ;
                sprintf((char *)prompt,"Recover newer %s",tt) ;
                if(mlyesno(prompt) == TRUE)
                {
                    fn = afn ;
                    ft = meFILETYPE_REGULAR ;
                    memcpy(&bp->stats,&stats,sizeof(meSTAT)) ;
                }
            }
            if(ft == meFILETYPE_DIRECTORY)
            {
                if(readDirectory(bp,fn) != TRUE)
                    goto error_end ;
                ss = TRUE ;
                goto newfile_end;
            }
            if(ft == meFILETYPE_NOTEXIST)
            {   /* File not found.      */
                uint8 dirbuf [FILEBUF];

                /* See if we can write to the directory. */
                getFilePath (fn, dirbuf);
                if((getFileStats(dirbuf,0,NULL,NULL) != meFILETYPE_DIRECTORY)
#ifdef _UNIX
                   || meTestWrite(dirbuf)
#endif
                   )
                {
                    /* READ ONLY DIR */
                    mlwrite(MWPAUSE,(uint8 *)"%s: %s", dirbuf, sys_errlist[errno]);
                    /* Zap the filename - it is invalid.
                       We only want a buffer */
                    mlwrite (0,(uint8 *)"[New buffer %s]", getFileBaseName(fname));
                    meNullFree(bp->b_fname) ;
                    bp->b_fname = NULL;
                    ss = ABORT;
                }
                else
                {
                    mlwrite(0,(uint8 *)"[New file %s]", fname);
                    ss = TRUE ;
                }
                /* its not linked to a file so change the flag */
                bp->intFlag &= ~BIFFILE ;
                goto newfile_end;
            }
            
            /* Make sure that we can read the file. If we are root then
             * we do not test the 'stat' bits. Root is allowed to read
             * anything */
            if ((meTestRead (fn)) ||
                (
#ifdef _UNIX
                 (meUid != 0) &&
#endif
                 (!meStatTestRead(bp->stats))))
            {
                /* We are not allowed to read the file */
#if ((defined _UNIX) || (defined _DOS))
                mlwrite(MWABORT,(uint8 *)"[%s: %s]", fn, sys_errlist[errno]) ;
#else
                mlwrite(MWABORT,"[cannot read: %s]", fn) ;
#endif
                /* Zap the filename - it is invalid.
                   We only want a buffer */
                meNullFree(bp->b_fname) ;
                bp->b_fname = NULL;
                /* its not linked to a file so change the flag */
                bp->intFlag &= ~BIFFILE ;
                goto newfile_end;
            }
#ifdef _WIN32
            if(!meStatTestSystem(bp->stats))
            {
                /* if windows system file read in a readonly */
                meModeSet(bp->b_mode,MDVIEW) ;
                mlwrite(MWCURSOR|MWCLEXEC,"[Reading %s (system file)]", fn);
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
                meModeSet(bp->b_mode,MDVIEW) ;
        }
        if(meModeTest(bp->b_mode,MDVIEW))
            mlwrite(MWCURSOR|MWCLEXEC,(uint8 *)"[Reading %s (readonly)]", fn);
        else
            mlwrite(MWCURSOR|MWCLEXEC,(uint8 *)"[Reading %s]", fn);
    }
    ss = ffReadFile(fn,0,bp,bp->b_linep) ;
    
    /*
    ** Set up the modification time field of the buffer structure.
    */
    if(ss != ABORT)
    {
        mlwrite(MWCLEXEC,(uint8 *)"[Read %d line%s]",bp->elineno,(bp->elineno==1) ? "":"s") ;
        if(fn == afn)
            /* this is a recovered file so flag the buffer as changed */
            meModeSet(bp->b_mode,MDEDIT) ;
        ss = TRUE ;
    }
    else
        meModeSet(bp->b_mode,MDVIEW) ;

newfile_end:

    bp->b_dotp = lforw(bp->b_linep);
    bp->line_no = 0 ;
    bp->b_doto = 0 ;
    
error_end:
    return ss ;
}

/*
 * Insert file "fname" into the current
 * buffer, Called by insert file command. Return the final
 * status of the read.
 */
/* must have the buffer line no. correct */
int
ifile(BUFFER *bp, uint8 *fname, uint32 flags)
{
    register WINDOW *wp ;
    register LINE   *lp ;
    register int     ss ;
    register long    nline ;
    meMODE md ;
    
    meModeSet(bp->b_mode,MDEDIT) ;              /* we have changed	*/
    meModeCopy(md,bp->b_mode) ;
              
#if CRYPT
    if(resetkey(bp) != TRUE)
        return FALSE ;
#endif
    if(!(flags & meRWFLAG_SILENT))
        mlwrite(MWCURSOR|MWCLEXEC,(uint8 *)"[Inserting file]");
    
    nline = bp->elineno ;
    lp = bp->b_dotp ;
    ss = ffReadFile(fname,flags|meRWFLAG_INSERT,bp,lp) ;
    nline = bp->elineno-nline ;
    /* restore the mode */
    meModeCopy(bp->b_mode,md) ;

    if(ss != ABORT)
    {
        ss = TRUE ;
        if(!(flags & meRWFLAG_SILENT))
           mlwrite(MWCLEXEC,(uint8 *)"[inserted %d line%s]",nline,(nline==1) ? "":"s") ;
    }
    for (wp=wheadp; wp!=NULL; wp=wp->w_wndp)
        if (wp->w_bufp == bp)
        {
            if(wp->line_no >= bp->line_no)
                wp->line_no += nline ;
            if(wp->mlineno >= bp->line_no)
                wp->mlineno += nline ;
            wp->w_flag |= WFMAIN|WFMOVEL ;
        }
#if MEUNDO
    if((bp == curbp) && meModeTest(bp->b_mode,MDUNDO))
    {
        int ii = 0 ;
        curwp->w_doto = 0 ;
        while(nline--)
        {
            lp = lback(lp) ;
            ii += llength(lp) + 1 ;
        }
        meUndoAddInsChars(ii) ;
    }
#endif
    return ss ;
}

/*
 * Insert a file into the current
 * buffer. This is really easy; all you do it
 * find the name of the file, and call the standard
 * "insert a file into the current buffer" code.
 * Bound to "C-X C-I".
 */
int
insFile(int f, int n)
{
    register int s;
    uint8 fname[FILEBUF] ;

    if((s=inputFileName((uint8 *)"Insert file",fname,1)) != TRUE)
        return s ;
    /* Allow existing or url files */
    if(((s=getFileStats(fname,3,NULL,NULL)) != meFILETYPE_REGULAR) &&
       (s != meFILETYPE_HTTP) && (s != meFILETYPE_FTP))
        return ABORT ;
    if((s=bchange()) != TRUE)               /* Check we can change the buffer */
        return s ;
    /* set mark to previous line so it doesnt get moved down */
    curwp->w_markp = lback(curwp->w_dotp) ;
    curwp->w_marko = 0 ;
    curwp->mlineno = curwp->line_no-1 ;

    /* store current line in buffer */
    curbp->b_dotp = curwp->w_dotp ;
    curbp->line_no = curwp->line_no ;   /* must have the line no. correct */
    
    for(; n>0 ; n--)
        if((s = ifile(curbp,fname,0)) != TRUE)
            break ;

    /* move the mark down 1 line into the correct place */
    curwp->w_markp = lforw(curwp->w_markp);
    curwp->mlineno++ ;
    return s ;
}


/*
 * Take a file name, and from it
 * fabricate a buffer name. This routine knows
 * about the syntax of file names on the target system.
 * I suppose that this information could be put in
 * a better place than a line of code.
 */
uint8
makename(uint8 *bname, uint8 *fname)
{
    register uint8 *cp1 ;
    register uint8 *cp2 ;
    int             i ;

    cp1 = getFileBaseName(fname) ;
    cp2 = bname ;
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
    return (i == 1) ? 0:BIFNAME ;
}

#define fileNameWild(fileName)                                                        \
((strchr((char *)fileName,'*') != NULL) || (strchr((char *)fileName,'?') != NULL) ||  \
 (strchr((char *)fileName,'[') != NULL))

static int
findFileSingle(uint8 *fname, int bflag, int32 lineno)
{
#ifdef _UNIX
    meSTAT  stats ;
#endif
    BUFFER *bp ;
    int   gft ;
#ifdef _URLSUPP
    int fnlen ;
#endif
#ifdef _UNIX
    gft = getFileStats(fname,1,&stats,NULL) ;
#else
    gft = getFileStats(fname,1,NULL,NULL) ;
#endif
    if((gft == meFILETYPE_NASTY) || ((gft == meFILETYPE_NOTEXIST) && !(bflag & BFND_CREAT)))
        return 0 ;
    bflag |= BFND_CREAT ;
    
    /* if this is a directory then add the '/' */
    if(gft == meFILETYPE_DIRECTORY)
    {
        uint8 *ss=fname+meStrlen(fname) ;
        if(ss[-1] != DIR_CHAR)
        {
            ss[0] = DIR_CHAR ;
            ss[1] = '\0' ;
        }
    }
#ifdef _URLSUPP
    if((gft != meFILETYPE_FTP) ||
       ((fnlen = meStrlen(fname)),(fname[fnlen-1] == DIR_CHAR)))
        fnlen = 0 ;
#endif
    for (bp=bheadp; bp!=NULL; bp=bp->b_bufp)
    {
        if((bp->b_fname != NULL) && (bp->b_bname != NULL) && (bp->b_bname[0] != '*'))
        {
            if(
#ifdef _UNIX
               !fnamecmp(bp->b_fname,fname) || 
               ((stats.stdev != (dev_t)(-1)) &&
                (bp->stats.stdev == stats.stdev) && 
                (bp->stats.stino == stats.stino) &&
                (bp->stats.stsize == stats.stsize)))
#else
               !fnamecmp(bp->b_fname,fname))
#endif
                break ;
#ifdef _URLSUPP
            /* at this point the type of an ftp file (i.e. reg of dir)
             * is unknown, the filename will be changed,but the comparison
             * of ftp: file names must allow for this */
            if(fnlen && !meStrncmp(bp->b_fname,fname,fnlen) && 
               (bp->b_fname[fnlen] == DIR_CHAR) && (bp->b_fname[fnlen+1] == '\0'))
                break ;
#endif
        }
    }
    if(bp == NULL)
    {
        bp = bfind(fname,bflag);
        bp->b_fname = meStrdup(fname) ;
        bp->intFlag |= BIFFILE ;
        if(gft == meFILETYPE_DIRECTORY)
        {
            /* if its a directory then add dir mode and remove binary */
            meModeSet(bp->b_mode,MDDIR) ;
            meModeClear(bp->b_mode,MDBINRY) ;
            meModeClear(bp->b_mode,MDRBIN) ;
        }
        bp->line_no = lineno ;
        bp->histNo = bufHistNo ;
    }
    else
    {
        if(!meModeTest(bp->b_mode,MDNACT))
            bp->intFlag |= BIFLOAD ;
        bp->histNo = bufHistNo ;
    }
    return 1 ;
}


static void
fileMaskToRegex(uint8 *dfname, uint8 *sfname)
{
    uint8 *ss=sfname, *dd=dfname, cc ;
    
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
findFileList(uint8 *fname, int bflag, int32 lineno)
{
    register int nofiles=0, ii ;
    uint8 fileName[FILEBUF], *baseName ;
    
    bufHistNo++ ;
    fileNameCorrect(fname,fileName,&baseName) ;
              
#if URLAWAR
    if(isUrlLink(fileName))
        nofiles += findFileSingle(fileName,bflag,lineno) ;
    else
#endif
        /* if the base name has a wild card letter (i.e. *, ? '[')
         * and a file with that exact name does not exist then load
         * any files which match the wild card mask */
        if(fileNameWild(baseName) && meTestRead(fileName))
    {
        uint8 mask[FILEBUF] ;
        
        fileMaskToRegex(mask,baseName) ;
        *baseName = '\0' ;
        getDirectoryList(fileName,&curDirList) ;
        for(ii=0 ; ii<curDirList.size ; ii++)
        {
            uint8 *ss = curDirList.list[ii] ;
#ifdef _INSENSE_CASE
            if(regexStrCmp(ss,mask,0))
#else
            if(regexStrCmp(ss,mask,1))
#endif
            {
                meStrcpy(baseName,ss) ;
                nofiles += findFileSingle(fileName,bflag,lineno) ;
            }
        }
    }
    else
    {
        nofiles += findFileSingle(fileName,bflag,lineno) ;
    }
    return nofiles ;
}

int
findSwapFileList(uint8 *fname, int bflag, int32 lineno)
{
    BUFFER *bp ;
    int     ret ;

    bufHistNo++ ;
    if(!findFileList(fname,bflag,lineno))
        return mlwrite(MWABORT|MWCLEXEC,(uint8 *)"[Failed to find file %s]",fname);
    for(bp=bheadp ; bp->histNo!=bufHistNo ; bp=bp->b_bufp)
        ;
    bufHistNo-=2 ;
    ret = swbuffer(curwp,bp) ;  /* make buffer BP current */
    bufHistNo++ ;
    return ret ;
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
    uint8 fname[FILEBUF], prompt[16], *ss ;
    
    ss = prompt ;
    *ss++ = 'f' ;
    *ss++ = 'i' ;
    *ss++ = 'n' ;
    *ss++ = 'd' ;
    *ss++ = '-' ;
    if(n & BFND_BINARY)
        *ss++ = 'b' ;
    if(n & BFND_CRYPT)
        *ss++ = 'c' ;
    if(n & BFND_RBIN)
        *ss++ = 'r' ;
    *ss++ = 'f' ;
    *ss++ = 'i' ;
    *ss++ = 'l' ;
    *ss++ = 'e' ;
    *ss   = '\0' ;
    if(inputFileName(prompt,fname,0) != TRUE)
        return ABORT ;
    n = (n & (BFND_CREAT|BFND_BINARY|BFND_CRYPT|BFND_RBIN)) | BFND_MKNAM ;
    return findSwapFileList(fname,n,0) ;
}

/*
 * Find file into other window. This is trivial since all the hard stuff is
 * done by the routines splitWindVert() and filefind().
 *
 * The count is used to put the "found file" into the upper or lower window.
 *
 * Normally mapped to ^X4. (or C-X 4 if you prefer).
 */
int
nextWndFindFile(int f, int n)
{
    uint8 fname[FILEBUF];	/* file user wishes to find */

    if(inputFileName((uint8 *)"Find file",fname,0) != TRUE)
        return ABORT ;
    if(wpopup(NULL,WPOP_MKCURR) == NULL)
        return FALSE ;
    n = (n & (BFND_CREAT|BFND_BINARY|BFND_CRYPT|BFND_RBIN)) | BFND_MKNAM ;
    return findSwapFileList(fname,n,0) ;
}

int
readFile(int f, int n)
{
    uint8 fname[FILEBUF];	/* file user wishes to find */
    register int s;		/* status return */

    if(inputFileName((uint8 *)"Read file", fname,0) != TRUE)
        return ABORT ;
    n = (n & (BFND_CREAT|BFND_BINARY|BFND_CRYPT|BFND_RBIN)) | BFND_MKNAM ;
    if((s=zotbuf(curbp,clexec)) == TRUE)
        s = findSwapFileList(fname,n,0) ;
    return s ;
}

int
viewFile(int f, int n)	/* visit a file in VIEW mode */
{
    uint8 fname[FILEBUF];	/* file user wishes to find */
    register int ss, vv;	/* status return */

    if (inputFileName((uint8 *)"View file", fname,0) != TRUE)
        return ABORT ;
    n = (n & (BFND_CREAT|BFND_BINARY|BFND_CRYPT|BFND_RBIN)) | BFND_MKNAM ;
    /* Add view mode globally so any new buffers will be created
     * with view mode */
    vv = meModeTest(globMode,MDVIEW) ;
    meModeSet(globMode,MDVIEW) ;
    ss = findSwapFileList(fname,n,0) ;
    /* if view mode was not set globally restore it */
    if(!vv)
        meModeClear(globMode,MDVIEW) ;
    return ss ;
}

/*
 * writeCheck
 * This performs some sime access checks to determine if we
 * can write the file.
 */
static int 
writeCheck (uint8 *pathname, int flags, meSTAT *statp)
{
    uint8 dirbuf [FILEBUF];
#ifdef _URLSUPP
    if(isFtpLink(pathname))
        return TRUE ;
    else if(isHttpLink(pathname))
        return mlwrite (MWABORT,(uint8 *)"Cannot write to: %s",pathname);
    else
#endif        
#if URLAWAR
    if(isUrlLink(pathname))
        return mlwrite (MWABORT,(uint8 *)"Cannot write to: %s",pathname);
    else
#endif        
    {
        /* Quick test for read only. */
#ifdef _UNIX
        /* READ ONLY directory test
           See if we can write to the directory. */
        getFilePath (pathname, dirbuf);
        if (meTestWrite (dirbuf))
            return mlwrite (MWABORT,(uint8 *)"Read Only Directory: %s", dirbuf);
#endif        
        /* See if there is an existing file */
        if ((flags & 1) == 0)           /* Validity check enabled ?? */
            return TRUE;                /* No - quit. */
        if (meTestExist (pathname))     /* Does it exist ?? */
            return TRUE;                /* No - quit */
        if ((statp != NULL) && !meStatTestWrite(*statp))
        {
            /* No - advised read only - see if the users wants to write */
            sprintf((char *)dirbuf, "%s is read only. Try anyway", pathname);
            if(mlyesno(dirbuf) != TRUE)
                return mlwrite (MWABORT,(uint8 *)"File not written.");
        }
    }   
    return TRUE;
}

static uint8 *
writeFileChecks(uint8 *dfname, uint8 *sfname, uint8 *lfname, int flags)
{
    register int s;
    meSTAT stats ;
    uint8 *fn ;
    
    if((sfname != NULL) &&
       (getFileStats(dfname,0,NULL,NULL) == meFILETYPE_DIRECTORY))
    {
        s = meStrlen(dfname) ;
        if(dfname[s-1] != DIR_CHAR)
            dfname[s++] = DIR_CHAR ;
        meStrcpy(dfname+s,getFileBaseName(sfname)) ;
    }
    if (((s=getFileStats(dfname,3,&stats,lfname)) != meFILETYPE_REGULAR) && (s != meFILETYPE_NOTEXIST)
#ifdef _URLSUPP
        && (s != meFILETYPE_FTP)
#endif
        )
        return NULL ;
    fn = (lfname[0] == '\0') ? dfname:lfname ;
    if(flags & 0x01)
    {
        uint8 prompt[MAXBUF+48];
        BUFFER *bp ;
        
        /* Check for write-out filename problems */
        if(s == meFILETYPE_REGULAR)
        {
            sprintf((char *)prompt,"File %s already exists, overwrite",fn) ;
            if(mlyesno(prompt) != TRUE)
            {
                ctrlg(FALSE,1);
                return NULL ;
            }
        }
        /* Quick check on the file write condition */
        if(writeCheck (fn, flags, &stats) != TRUE)
            return NULL ;
        /*
         * Check to see if the new filename conflicts with the filenames for any
         * other buffer and produce a warning if so.
         */
        bp=bheadp ;
        while(bp != NULL)
        {
            if((bp != curbp) &&
#ifdef _UNIX
               (!fnamecmp(bp->b_fname,fn) || 
                ((stats.stdev != (dev_t)(-1)) &&
                 (bp->stats.stdev == stats.stdev) &&
                 (bp->stats.stino == stats.stino) &&
                 (bp->stats.stsize == stats.stsize))))
#else
               !fnamecmp(bp->b_fname,fn))
#endif
            {
                sprintf((char *)prompt, "Buffer %s is the same file, overwrite",bp->b_bname) ;
                if(mlyesno(prompt) != TRUE)
                {
                    ctrlg(FALSE,1);
                    return NULL ;
                }
            }
            bp = bp->b_bufp ;
        }
    }
    return fn ;
}

#define meFILEOP_CHECK    0x001
#define meFILEOP_BACKUP   0x002
#define meFILEOP_FTPCLOSE 0x010
#define meFILEOP_DELETE   0x020
#define meFILEOP_MOVE     0x040
#define meFILEOP_COPY     0x080
#define meFILEOP_MKDIR    0x100
int
fileOp(int f, int n)
{
    uint8 sfname[FILEBUF], dfname[FILEBUF], lfname[FILEBUF], *fn ;
    int dFlags=0 ;
	
    if((n & (meFILEOP_FTPCLOSE|meFILEOP_DELETE|meFILEOP_MOVE|meFILEOP_COPY|meFILEOP_MKDIR)) == 0)
        return mlwrite(MWABORT,(uint8 *)"[No operation set]") ;
       
    if(n & meFILEOP_DELETE)
    {
        if (inputFileName((uint8 *)"Delete file", sfname,1) != TRUE)
            return ABORT ;
        if(n & meFILEOP_CHECK)
        {        
            uint8 prompt[MAXBUF];
            sprintf((char *)prompt, "%s: Delete file",sfname) ;
            if(mlyesno(prompt) != TRUE)
                return ctrlg(FALSE,1);
        }
        fn = NULL ;
        dFlags = meRWFLAG_DELETE ;
    }
    else if(n & (meFILEOP_MOVE|meFILEOP_COPY))
    {
        static uint8 prompt[]="Copy file to" ;
        if(n & meFILEOP_MOVE)
        {
            prompt[0] = 'M' ;
            prompt[2] = 'v' ;
            prompt[3] = 'e' ;
            dFlags = meRWFLAG_DELETE ;
        }
        else
        {
            prompt[0] = 'C' ;
            prompt[2] = 'p' ;
            prompt[3] = 'y' ;
            dFlags = 0 ;
        }
        prompt[9] = '\0' ;
        if (inputFileName(prompt, sfname,1) != TRUE)
            return ABORT ;
        prompt[9] = ' ' ;
        if (inputFileName(prompt, dfname,1) != TRUE)
            return ABORT ;
#ifdef _URLSUPP
        if(isUrlLink(sfname) && isUrlLink(dfname))
            return mlwrite (MWABORT,(uint8 *)"[Cannot read and write to URL at the same time]") ;
#endif
        if((fn=writeFileChecks(dfname,sfname,lfname,n)) == NULL)
            return ABORT ;
        /* can this be done by a simple rename? */
        if(((n & (meFILEOP_BACKUP|meFILEOP_MOVE)) == meFILEOP_MOVE) && !meRename(sfname,fn))
            return TRUE ;
    }
    else if(n & meFILEOP_MKDIR)
    {
        int s ;
        if (inputFileName((uint8 *)"Create dir", sfname,1) != TRUE)
            return ABORT ;
        /* check that nothing of that name currently exists */
        if (((s=getFileStats(sfname,0,NULL,NULL)) != meFILETYPE_NOTEXIST)
#ifdef _URLSUPP
            && (s != meFILETYPE_FTP)
#endif
            )
            return mlwrite(MWABORT,(uint8 *)"[%s already exists]",sfname);
        fn = NULL ;
        dFlags = meRWFLAG_MKDIR ;
    }
    if(n & meFILEOP_BACKUP)
        dFlags |= meRWFLAG_BACKUP ;
    if(n & meFILEOP_FTPCLOSE)
        dFlags |= meRWFLAG_FTPCLOSE ;
    return ffFileOp(sfname,fn,dFlags) ;
}

/*
 * This function performs an auto writeout to disk for the given buffer
 */
void
autowriteout(register BUFFER *bp)
{
    uint8 fn[FILEBUF], lname[FILEBUF], *ff ;
    int ss ;
    
    bp->autotime = -1 ;
    
    if(bp->b_fname == NULL)
        ss = ABORT ;
    else
    {
        getFileStats(bp->b_fname,0,NULL,lname) ;
        ff = (lname[0] == '\0') ? bp->b_fname:lname ;
        if(createBackupName(fn,ff,'#',1))
            ss = ABORT ;
        else if((ss=ffWriteFileOpen(fn,meRWFLAG_WRITE|meRWFLAG_AUTOSAVE,bp)) == TRUE)
        {    
            LINE *lp ;
            
            lp = lforw(bp->b_linep);            /* First line.          */
            while((lp != bp->b_linep) &&
                  ((ss=ffWriteFileWrite(llength(lp),ltext(lp),
                                    !(lp->l_flag & LNNEOL))) == TRUE))
            {
                lp = lforw(lp) ;
                if(TTbreakTest(1))
                {
                    ss = ABORT ;
                    break ;
                }
            }
            ffWriteFileClose(fn,meRWFLAG_WRITE|meRWFLAG_AUTOSAVE,bp) ;
        }
    }
    if(ss == TRUE)
        mlerase(MWCLEXEC) ;
    else
        mlwrite(0,(uint8 *)"[Auto-writeout failure for buffer %s]",bp->b_bname) ;
}

/*
 * This function removes an auto writeout file for the given buffer
 */
void
autowriteremove(register BUFFER *bp)
{
    uint8 fn[FILEBUF] ;

    if((autotime > 0) && bufferNeedSaving(bp) &&
       !createBackupName(fn,bp->b_fname,'#',0) &&
       !meTestExist(fn))
        /* get rid of any tempory file */
        meUnlink(fn) ;
    bp->autotime = -1 ;
}

/*
 * This function performs the details of file
 * writing. Uses the file management routines in the
 * "fileio.c" package. The number of lines written is
 * displayed. Sadly, it looks inside a LINE; provide
 * a macro for this. Most of the grief is error
 * checking of some sort.
 */
static int
writeOut(register BUFFER *bp, uint32 flags, uint8 *fn)
{
#if (defined _UNIX) || (defined _DOS) || (defined _WIN32)
    /* Add write permission to backup file. */
    if(meModeTest(bp->b_mode,MDBACK))
    {
        register uint32 ss;
        
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
        if(ss == bp->stats.stmode)
            ss = 0 ;
        flags |= meRWFLAG_BACKUP | ss ;
    }
#endif
#if	TIMSTMP
    set_timestamp(bp);			/* Perform time stamping */
#endif
    if(ffWriteFile(fn,flags,bp) != TRUE)
        return FALSE ;
    /* No write error.      */
    /* must be done before buffer is flagged as not changed */
#if MEUNDO
    if(meSystemCfg & meSYSTEM_KEEPUNDO)
    {
        /* go through undo list removing any unedited flags */
        UNDOND *nn ;
        nn = bp->fUndo ;
        while(nn != NULL)
        {
            if(nn->type & MEUNDO_FRST)
                nn->type |= MEUNDO_MDEL ;
            nn = nn->next ;
        }
    }
    else
        meUndoRemove(bp) ;
#endif
    autowriteremove(bp) ;
    meModeClear(bp->b_mode,MDEDIT) ;
    meModeClear(bp->b_mode,MDSAVE) ;

    if(fn != NULL)
    {
#ifndef _WIN32
        /* set the right file attributes */
        if(bp->stats.stmode != meUmask)
            meChmod(fn,bp->stats.stmode) ;
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
        getFileStats(fn,0,&bp->stats,NULL) ;
        /* Change the view status back to the file's permissions. 
         * For root we let the super user do as they wish so no 
         * read protection is added. */
        if(
#if (defined _UNIX)
           (meUid != 0) &&
#endif
           (!meStatTestWrite(bp->stats)))
            meModeSet(curbp->b_mode,MDVIEW) ;
        else
            meModeClear(curbp->b_mode,MDVIEW) ;
    }
    return TRUE ;
}

/*
 * This function performs the details of file writing. Uses the file
 * management routines in the "fileio.c" package. The number of lines written
 * is displayed. Sadly, it looks inside a LINE; provide a macro for this. Most
 * of the grief is error checking of some sort.
 */
int
writeout(register BUFFER *bp, int flags, uint8 *fname)
{
    uint8 lname[FILEBUF], *fn ;
    if(!meStrcmp(bp->b_bname,"*stdin*"))
        fn = NULL ;
    else if((bp->b_bname[0] == '*') || (fname == NULL))
        return mlwrite(MWABORT,(uint8 *)"[No file name set for buffer %s]",bp->b_bname);
    else
    {
        meSTAT stats ;
        /*
         * Check that the file has not been modified since it was read it in. This
         * is a bit of a problem since the info we require is hidden in a buffer
         * structure which may not necessarily be the current buffer. If it is not
         * in the current buffer then dont bother to look for it.
         */
        getFileStats(fname,0,&stats,lname) ;
        if((flags & 1) && (stats.stmtime > bp->stats.stmtime))
        {
            uint8 prompt[MAXBUF];
            sprintf((char *)prompt, "%s modified, write", fname);
            if(mlyesno(prompt) != TRUE)
                return ctrlg(FALSE,1);
        }
        fn = (lname[0] == '\0') ? fname:lname ;
        /* Quick check on the file write condition */
        if (writeCheck (fn, flags, &stats) != TRUE)
            return ABORT;
    }
    
    return writeOut(bp,((flags & 0x02) ? meRWFLAG_IGNRNRRW:0),fn) ;
}

void
resetBufferNames(BUFFER *bp, uint8 *fname)
{
    if(fnamecmp(bp->b_fname,fname))
    {
        uint8 buf[MAXBUF], *bb ;
        meNullFree(bp->b_fname) ;
        bp->b_fname = meStrdup(fname) ;
        unlinkBuffer(bp) ;
        bp->intFlag = (bp->intFlag & ~BIFNAME) | makename(buf, fname) ;
        if((bb = meMalloc(meStrlen(buf)+1)) != NULL)
        {
            meFree(bp->b_bname) ;
            meStrcpy(bb,buf) ;
            bp->b_bname = bb ;
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
    uint8 fname[FILEBUF], lname[FILEBUF], *fn ;
    
    if(inputFileName((uint8 *)"Write file",fname,1) != TRUE)
        return ABORT ;

    if(curbp->b_fname != NULL)
        fn = curbp->b_fname ;
    else if(curbp->b_bname[0] != '*')
        fn = curbp->b_bname ;
    else
        fn = NULL ;
    
    if((fn=writeFileChecks(fname,fn,lname,n)) == NULL)
        return ABORT ;
    
    if(!writeOut(curbp,((n & 0x02) ? meRWFLAG_IGNRNRRW:0),fn))
        return FALSE ;
    
    resetBufferNames(curbp,fname) ;
    addModeToWindows(WFMODE) ; /* and update ALL mode lines */

    return TRUE ;
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
    register int    s;
    
    /* Note that we check for existance here just incase sombody has
     * deleted it under our feet. There is nothing more annoying than the 
     * editor bitching there are no changes when it's been zapped !! */
    /* Further note: the file name can be NULL, e.g. theres no file name
     * so this must be tested before meTestExist, BUT the file name can be
     * NULL and still be saved, e.g. the buffer name is *stdin* - so be careful */
    if((n & 0x01) && (curbp->b_fname != NULL) &&  /* Are we Checking ?? */
       (meTestExist (curbp->b_fname) == 0) &&     /* Does file actually exist ? */
       !meModeTest(curbp->b_mode,MDEDIT))         /* Have we edited buffer ? */
        /* Return, no changes.  */
        return mlwrite(0,(uint8 *)"[No changes made]") ;
    if((s=writeout(curbp,n,curbp->b_fname)) == TRUE)
        addModeToWindows(WFMODE) ;  /* and update ALL mode lines */
    return (s);
}



int
appendBuffer(int f, int n)
{
    register uint32 flags ;
    register int ss ;
    uint8 fname[FILEBUF], lname[FILEBUF], *fn ;

    if(inputFileName((uint8 *)"Append to file",fname,1) != TRUE)
        return ABORT ;

    if(((ss=getFileStats(fname,3,NULL,lname)) != meFILETYPE_REGULAR) && (ss != meFILETYPE_NOTEXIST))
        return ABORT ;
    fn = (lname[0] == '\0') ? fname:lname ;
    if(ss == meFILETYPE_NOTEXIST)
    {
        if(n & 0x01)
        {
            uint8 prompt[MAXBUF];
            sprintf((char *)prompt, "%s does not exist, create",fn) ;
            if(mlyesno(prompt) != TRUE)
                return ctrlg(FALSE,1);
        }
        ss = 0 ;
    }
    else if(n & 0x04)
        flags = meRWFLAG_OPENTRUNC ;
    else
        flags = meRWFLAG_OPENEND ;
    if(n & 0x02)
        flags |= meRWFLAG_IGNRNRRW ;
    return ffWriteFile(fname,flags,curbp) ;
}
    
/*
 * save-some-buffers, query saves all modified buffers
 */
int
saveSomeBuffers(int f, int n)
{
    register BUFFER *bp;    /* scanning pointer to buffers */
    register int status=TRUE ;
    uint8 prompt[MAXBUF] ;

    bp = bheadp;
    while (bp != NULL)
    {
        if(bufferNeedSaving(bp))
        {
            if(n & 1)
            {
                if(bp->b_fname != NULL)
                    sprintf((char *)prompt, "Save file %s", bp->b_fname) ;
                else
                    sprintf((char *)prompt, "Save buffer %s", bp->b_bname) ;
                if((status = mlyesno(prompt)) == ABORT)
                    return ABORT ;
            }
            if((status == TRUE) &&
               (writeout(bp, 0, bp->b_fname) != TRUE))
                return ABORT ;
        }
        bp = bp->b_bufp;            /* on to the next buffer */
    }
    addModeToWindows(WFMODE) ;  /* and update ALL mode lines */
    return TRUE ;
}

/*
 * The command allows the user
 * to modify the file name associated with
 * the current buffer. It is like the "f" command
 * in UNIX "ed". The operation is simple; just zap
 * the name in the BUFFER structure, and mark the windows
 * as needing an update. You can type a blank line at the
 * prompt if you wish.
 */
int
changeFileName(int f, int n)
{
    register int s, ft ;
    uint8 fname[FILEBUF], lname[FILEBUF], *fn ;

    if ((s=inputFileName((uint8 *)"New file name",fname,1)) == ABORT)
        return s ;

    if(((ft=getFileStats(fname,3,NULL,lname)) != meFILETYPE_REGULAR) && (ft != meFILETYPE_NOTEXIST)
#ifdef _URLSUPP
        && (ft != meFILETYPE_FTP)
#endif
       )
        return ABORT ;
    fn = (lname[0] == '\0') ? fname:lname ;
    meNullFree(curbp->b_fname) ;

    if (s == FALSE)
        curbp->b_fname = NULL ;
    else
        curbp->b_fname = meStrdup(fname) ;

#if (defined _UNIX) || (defined _DOS) || (defined _WIN32)
    if(meTestWrite(fn))
        meModeSet(curbp->b_mode,MDVIEW) ;
    else
        meModeClear(curbp->b_mode,MDVIEW) ;	/* no longer read only mode */
#endif
    addModeToWindows(WFMODE) ;  /* and update ALL mode lines */

    return (TRUE);
}

#ifdef _DOS
int
_meChdir(uint8 *path)
{
    register int s ;
    int len ;

    len = strlen (path)-1 ;

    if((len > 1) && (path[1] == _DRV_CHAR))
    {
        union REGS reg ;		/* cpu register for use of DOS calls */

        reg.h.ah = 0x0e ;
        reg.h.dl = path[0] - 'a' ;
        intdos(&reg, &reg);
    }
    if((len > 3) && (path[len] == DIR_CHAR))
        path[len] = '\0' ;
    else
        len = -1 ;
    s = chdir(path) ;
    if(len > 0)
        path[len] = DIR_CHAR ;
    return s ;
}
#endif

int
changeDir(int f, int n)
{
    /*
     * Change directory.
     *
     * Alain D D Williams, Sept 1986.
     *
     * Hacked by Tony Bamford to change relative filenames to absolute
     * paths, Nov 1986.
     * Completely hack by Steve Phillips to make it not to bloody
     * dangerous.
     * Done it by making any files loaded imediately absolute (only
     * sane way of doing it) and disallowing operation if cant be done.
     */
#if (defined _UNIX) || (defined _DOS) || (defined _WIN32)
    register int    s;
    uint8 *dd, dname[FILEBUF];		/* directory to change to   */

    if((s = inputFileName((uint8 *)"Directory Name ",dname,1)) != TRUE)
        return(s);

    if(meChdir(dname) == -1)
        return mlwrite(MWABORT,(uint8 *)"directory %s: %s", dname, sys_errlist[errno]);
    if((dd = meStrdup(dname)) != NULL)
    {
        meFree(curdir) ;
        curdir = dd ;
    }
    return(TRUE);
#else
    return mlwrite(MWABORT,(uint8 *)"Cant change directory as cant get absolute path") ;
#endif
}


/************************ New file routines *****************************/
#ifdef _CONVDIR_CHAR
void
fileNameConvertDirChar(uint8 *fname)
{
    /* convert all '\\' to '/' for dos etc */
    while((fname=meStrchr(fname,_CONVDIR_CHAR)) != NULL)  /* got a '\\', -> '/' */
        *fname++ = DIR_CHAR ;
}
#endif

void
pathNameCorrect(uint8 *oldName, int nameType, uint8 *newName, uint8 **baseName)
{
    register uint8 *p, *p1 ;
    uint8 *urls, *urle ;
    int flag ;
#if (defined _DOS) || (defined _WIN32)
    uint8 *gwdbuf ;
#endif
    
    fileNameConvertDirChar(oldName) ;
    flag = 0 ;
    p = p1 = oldName ;
    /* search for
     * 1) set to root,  xxxx/http:// -> http://  (for urls)
     * 2) set to root,  xxxx/ftp://  -> ftp://   (for urls)
     * 2) set to root,  xxxx/file:yy -> yy       (for urls)
     * 3) set to root,  xxxx///yyyyy -> //yyyyy  (for network drives)
     * 4) set to root,  xxxx//yyyyy  -> /yyyyy
     * 5) set to Drive, xxxx/z:yyyyy -> z:yyyyy
     * 6) set to home,  xxxx/~yyyyy  -> ~yyyyy
     */
    for(;;)
    {
#if URLAWAR
        if(isHttpLink(p1))
        {
            flag = 2 ;
            urls = p1 ;
            if((p=meStrchr(p1+7,DIR_CHAR)) == NULL)
                p = p1 + meStrlen(p1) ;
            p1 = p ;
        }
        else if(isFtpLink(p1))
        {
            flag = 3 ;
            urls = p1 ;
            if((p=meStrchr(p1+6,DIR_CHAR)) == NULL)
                p = p1 + meStrlen(p1) ;
            urle = p ;
            p1 = p ;
        }
        else if(isUrlFile(p1))
        {
            flag = 0 ;
            p1 += 5 ;
            p = p1 ;
            /* loop here as if at the start of the file name, this
             * is so it handles "file:ftp://..." correctly */
            continue ;
        }
        else
#endif
            if(flag != 2)
        {
            /* note that ftp://... names are still processed, ftp://.../xxx//yyy -> ftp://.../yyy etc */
            if(p1[0] == DIR_CHAR)
            {
#ifndef _UNIX
                if(p1[1] == DIR_CHAR)
                {
                    /* Got a xxxx///yyyyy -> //yyyyy must not optimise further */
                    flag = 1 ;
                    while(p1[2] == DIR_CHAR)
                        p1++ ;
                    urls = p1 ;
                    if((p=strchr(p1+2,DIR_CHAR)) == NULL)
                        p = p1 + strlen(p1) ;
                    urle = p ;
                    p1 = p ;
                }
                else
#endif
                    /* Got a xxxx//yyyyy -> /yyyyy */
                    p = p1 ;
            }
            else if((homedir != NULL) && (p1[0] == '~'))
            {
                /* Got a home,  xxxx/~yyyyy  -> ~yyyyy - remove ftp:// or //yyyy/... */
                flag = 0 ;
                p = p1 ;
            }
#ifdef _DRV_CHAR
            else if(isAlpha(p1[0]) && (p1[1] == _DRV_CHAR))
            {
                /* got a Drive, xxxx/z:yyyyy -> z:yyyyy - remove ftp:// or //yyyy/... */
                flag = 0 ;
                p = p1 ;
            }
#endif
        }
        if((p1=meStrchr(p1,DIR_CHAR)) == NULL)
            break ;
        p1++ ;
    }
    
    p1 = newName ;
    if(flag == 2)
        meStrcpy(p1,urls) ;
    else
    {
        if(flag)
        {
            int ll= (size_t) urle - (size_t) urls ;
            meStrncpy(p1,urls,ll) ;
            p1 += ll ;
        }
        urle = p1 ;
        if(p[0] == '\0')
        {
            *p1++ = DIR_CHAR ;
            *p1 = '\0' ;
        }
        else if((homedir != NULL) && (p[0] == '~'))
        {
            meStrcpy(p1,homedir) ;
            p1 += meStrlen(p1) ;
            if(p[1] != DIR_CHAR)
            {
                *p1++ = DIR_CHAR ;
                *p1++ = '.' ;
                *p1++ = '.' ;
                *p1++ = DIR_CHAR ;
            }
            meStrcpy(p1,p+1) ;
        }
	else if(flag)
            meStrcpy(p1,p) ;
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
            strcpy(p1+2,p) ;
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
    }
    if(flag != 2)
    {
        p = NULL ;
        p1 = urle ;
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
                uint8 *tt ;
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
    }
    if(baseName != NULL)
    {
        uint8 cc ;
        p = p1 = newName ;
        while((cc=*p1++) != '\0')
        {
            if((cc == DIR_CHAR) && (*p1 != '\0'))
                p = p1 ;
            else if((cc == '[') || (cc == '?') || (cc == '*'))
                /* this is base of a wild file base name break */
                break ;
        }
        *baseName = p ;
    }
}

#ifdef _WIN32
void
fileNameCorrect(uint8 *oldName, uint8 *newName, uint8 **baseName)
{
    uint8 *bn ;
    
    pathNameCorrect(oldName,PATHNAME_COMPLETE,newName,&bn) ;
    if(baseName != NULL)
        *baseName = bn ;
#if URLAWAR
    if(isUrlLink(newName))
        return ;
#endif
    /* ensure the drive letter is stable, make it lower case */
    if((newName[1] == _DRV_CHAR) && isUpper(newName[0]))
        newName[0] = toLower(newName[0]) ;
    
    if(!fileNameWild(bn))
    {
        /* with windows naff file name case insensitivity we must get
         * the correct a single name letter case by using FindFirstFile*/
        HANDLE *handle;
        WIN32_FIND_DATA fd;
        if((handle = FindFirstFile(newName,&fd)) != INVALID_HANDLE_VALUE)
        {
            strcpy(bn,fd.cFileName) ;
            /* If a directory that append the '/' */
            if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                bn += strlen(bn) ;
                *bn++ = DIR_CHAR ;
                *bn = '\0' ;
            }
            FindClose (handle);
        }
    }
}
#endif

void
getDirectoryList(uint8 *pathName, meDIRLIST *dirList)
{
    uint8 **fls ;
    int    noFiles ;
#ifdef _UNIX
    struct stat statbuf;
    int32 timeStamp ;
    
    if(!stat((char *)pathName,&statbuf))
        timeStamp = statbuf.st_mtime ;
    else
        timeStamp = 0 ;
#endif
    
    if((dirList->path != NULL) &&
       !meStrcmp(dirList->path,pathName) &&
#ifdef _UNIX
       (timeStamp <= dirList->timeStamp)
#else
       !dirList->timeStamp
#endif
       )
        return ;
    
    /* free off the old */
    meNullFree(dirList->path) ;
    freeFileList(dirList->size,dirList->list) ;
    fls = NULL ;
    noFiles = 0 ;
                  
#ifdef _DOS
    if(pathName[0] == '\0')
    {
        union REGS reg ;		/* cpu register for use of DOS calls */
        uint8 *ff ;
        int    ii ;
        
        for (ii = 1; ii <= 26; ii++)    /* Drives are a-z (1-26) */
        {
            reg.x.ax = 0x440e ;
            reg.h.bl = ii ;
            intdos(&reg,&reg) ;
            if((reg.x.cflag == 0) || (reg.x.ax != 0x0f))
            {
                if(((fls = meRealloc(fls,sizeof(uint8 *) * (noFiles+1))) == NULL) ||
                   ((ff = meMalloc(4*sizeof(uint8))) == NULL))
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
        uint8 *ff, *ee, es[4] ;
        int done ;
        
        /* append the *.* - Note that this function assumes the pathName has a '/' and
         * its an array with 3 extra char larger than the string size */
        ee = pathName + strlen(pathName) ;
        ee[0] = '*' ;
        es[1] = ee[1] ;
        ee[1] = '.' ;
        es[2] = ee[2] ;
        ee[2] = '*' ;
        es[3] = ee[3] ;
        ee[3] = '\0' ;
        done = findfirst(pathName,&fblk,FA_RDONLY|FA_HIDDEN|FA_SYSTEM|FA_DIREC) ;
        ee[0] = '\0' ;
        ee[1] = es[1] ;
        ee[2] = es[2] ;
        ee[3] = es[3] ;
        while(!done)
        {
            if(((noFiles & 0x0f) == 0) &&
               ((fls = meRealloc(fls,sizeof(uint8 *) * (noFiles+16))) == NULL))
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
            strcpy(ff,fblk.ff_name) ;
            makestrlow(ff) ;
            if(fblk.ff_attrib & FA_DIREC)
            {
                ff += strlen(ff) ;
                *ff++ = DIR_CHAR ;
                *ff   = '\0' ;
            }
            done = findnext(&fblk) ;
        }
        dirList->timeStamp = 0 ;
    }
#endif
#ifdef _WIN32
    if(pathName[0] == '\0')
    {
        uint8 *ff ;
        int ii ;
        DWORD  dwAvailableDrives, dwMask;
            
        /* Get the drives */
        dwAvailableDrives = GetLogicalDrives ();

        /* Drives are a-z (bit positions 0-25) */
        for (ii = 1, dwMask = 1; ii <= 26; ii++, dwMask <<= 1)
        {
            if((ii == 1) || (dwAvailableDrives & dwMask)) /* Drive exists ?? */
            {
                /* Yes - add to the drive list */
                if(((fls = meRealloc(fls,sizeof(uint8 *) * (noFiles+1))) == NULL) ||
                   ((ff = meMalloc(4*sizeof(uint8))) == NULL))
                {
                    fls = NULL ;
                    noFiles = 0 ;
                    break ;
                }
                fls[noFiles++] = ff ;
                *ff++ = ii + 'a' - 1;
                *ff++ = _DRV_CHAR ;
                *ff++ = DIR_CHAR ;
                *ff   = '\0' ;
            }
        }
    }
    else
    {
        HANDLE *handle;
        WIN32_FIND_DATA fd;
        uint8 *ff, *ee, es[4] ;
        
        /* append the *.* - Note that this function assumes the pathName has a '/' and
         * its an array with 3 extra char larger than the string size */
        ee = pathName + strlen(pathName) ;
        ee[0] = '*' ;
        es[1] = ee[1] ;
        ee[1] = '.' ;
        es[2] = ee[2] ;
        ee[2] = '*' ;
        es[3] = ee[3] ;
        ee[3] = '\0' ;
        handle = FindFirstFile(pathName,&fd) ;
        ee[0] = '\0' ;
        ee[1] = es[1] ;
        ee[2] = es[2] ;
        ee[3] = es[3] ;
        if(handle != INVALID_HANDLE_VALUE)
        {
            do
            {
                if(((noFiles & 0x0f) == 0) &&
                   ((fls = meRealloc(fls,sizeof(uint8 *) * (noFiles+16))) == NULL))
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
                strcpy(ff,fd.cFileName) ;
                if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    ff += strlen(ff) ;
                    *ff++ = DIR_CHAR ;
                    *ff   = '\0' ;
                }
            } while (FindNextFile (handle, &fd));
            FindClose (handle);
        }
        dirList->timeStamp = 0 ;
    }
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
            uint8 *ff, *bb, fname[FILEBUF] ;
            
            meStrcpy(fname,pathName) ;
            bb = fname + meStrlen(fname) ;
            
            while((dp = readdir(dirp)) != NULL)
            {
                if(((noFiles & 0x0f) == 0) &&
                   ((fls = meRealloc(fls,sizeof(uint8 *) * (noFiles+16))) == NULL))
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
                    ff += strlen(ff) ;
                    *ff++ = DIR_CHAR ;
                    *ff   = '\0' ;
                }
                else if(dp->d_type == DT_UNKNOWN)
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
        dirList->timeStamp = timeStamp ;
    }
#endif  /* _UNIX */
    dirList->path = meStrdup(pathName) ;
    dirList->size = noFiles ;
    dirList->list = fls ;
    if(noFiles > 1)
#ifdef _INSENSE_CASE
        sortStrings(noFiles,fls,0,stridif) ;
#else
        sortStrings(noFiles,fls,0,strcmp) ;
#endif
}

/*
** frees the file list created by getFileList
*/
void
freeFileList(int noStr, uint8 **files)
{
    if(files == NULL)
        return ;

    while (--noStr >= 0)
        meFree(files[noStr]) ;

    meFree((void *) files) ;
}


