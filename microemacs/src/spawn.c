/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * spawn.c - Routines for launching external process.
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
 * Synopsis:    Routines for launching external process.
 * Authors:     Unknown, Jon Green & Steven Phillips
 * Description:	
 *      Various system access commands.
 */

#define	__SPAWNC			/* Name the file */

#include "emain.h"

#ifdef IPIPE_DEBUG
#ifdef _STDARG
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#endif

#ifdef _UNIX
#include <errno.h>
#include <fcntl.h>                      /* This should not be required for POSIX !! */
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

/* Definitions for the terimal I/O */
#ifdef _USG
#ifdef _TERMIOS
#include <termios.h>
#else
#include <termio.h>
#endif /* _TERMIOS */
#endif /* _USG */
#ifdef _BSD
#include <sgtty.h>                      /* For stty/gtty functions */
#endif /* _BSD */

#ifdef _SUNOS
#include <stropts.h>
#endif

#ifdef _POSIX_VDISABLE
#define CDISABLE _POSIX_VDISABLE
#else /* not _POSIX_VDISABLE */
#ifdef CDEL
#undef CDISABLE
#define CDISABLE CDEL
#else /* not CDEL */
#define CDISABLE 255
#endif /* not CDEL */
#endif /* not _POSIX_VDISABLE */

#ifndef WEXITSTATUS
#define WEXITSTATUS(status) ((int)(WIFEXITED(status)?(((*((int *)(&status)))>>8)&0xff) : -1))
#endif /* WIFEXITSTATUS */

#endif /* _UNIX */

#ifdef _WIN32
#include <direct.h>
#endif

#ifdef _DOS
#include <process.h>
#endif

#if MEOPT_SPAWN

#ifdef _UNIX
static meUByte *
getShellCmd(void)
{
    static meUByte *shellCmd=NULL ;
    if(shellCmd == NULL)
    {
        meUByte *cp, env[meBUF_SIZE_MAX], exe[meBUF_SIZE_MAX] ;
        if(((cp = meGetenv("SHELL")) != NULL) && (cp[0] != '\0') &&
           (meStrcpy(env,cp),(executableLookup(env,exe) != 0)))
            cp = exe ;
        else
            cp = (meUByte *)"/bin/sh" ;
        shellCmd = meStrdup(cp) ;
    }
    return shellCmd ;
}
#endif

/*
 * Create a subjob with a copy of the command intrepreter in it. When the
 * command interpreter exits, mark the screen as garbage so that you do a full
 * repaint. Bound to "^X C". The message at the start in VMS puts out a newline.
 * Under some (unknown) condition, you don't get one free when DCL starts up.
 */
int
meShell(int f, int n)
{
#ifdef _DOS
    register char *cp;
#endif
    meUByte path[meBUF_SIZE_MAX];		/* pathfrom where to execute */
    int  cd, ss=meABORT;
    
    getFilePath(frameCur->windowCur->buffer->fileName,path);
    cd = (meStrcmp(path,curdir) && (meChdir(path) != -1));

#ifdef _WIN32
    ss = WinLaunchProgram(NULL,LAUNCH_SHELL,NULL,NULL,
#if MEOPT_IPIPES
                           NULL, 
#endif
                           NULL);
#endif /* _WIN32 */
#ifdef _DOS
    TTclose();
    if ((cp=meGetenv("COMSPEC")) == NULL)
        ss = system("command.com");
    else
        ss = system(cp);
    TTopen();
    sgarbf = meTRUE;
#endif
#ifdef _UNIX
#ifdef _ME_WINDOW
#ifdef _ME_CONSOLE
    if(!(meSystemCfg & meSYSTEM_CONSOLE))
#endif
    {
        char *termPrg[] = {
#ifdef _LINUX
            "gnome-terminal",NULL,
            "konsole",NULL,
#endif
#ifdef _MACOS
            "open","-a","Terminal","$P",NULL,
#endif
            "xterm",NULL,
            NULL
        };
        int ii=0;
        char *pp;
        while(((pp=termPrg[ii]) != NULL) && !executableLookup((meUByte *) pp,evalResult))
        {
            while(termPrg[++ii] != NULL)
                ;
            ii++;
        }
        if(pp == NULL)
            ss = mlwrite(MWABORT,(meUByte *)"[Failed to find terminal program]");
        else
        {
#ifdef _MACOS
            int jj;
            char *vv;
            jj = ii;
            while((termPrg[++jj] != NULL) && ((termPrg[jj][0] != '$') || (termPrg[jj][1] != 'P')))
                ;
            if(termPrg[jj] != NULL)
            {
                vv = termPrg[jj];
                termPrg[jj] = (char *) path;
            }
#endif
            switch(fork())
            {
            case 0:
                /* we want the children to die on interrupt */
                execvp(pp,&(termPrg[ii]));
                fprintf(stderr,"exec of [%s] failed, %s\n",pp,strerror(errno));
                _exit(0);
            case -1:
                ss = mlwrite(MWABORT,(meUByte *)"exec failed, %s", strerror(errno));
            default:
                ss = meTRUE;
            }
#ifdef _MACOS
            if(termPrg[jj] != NULL)
                termPrg[jj] = vv;
#endif
        }
    }
#ifdef _ME_CONSOLE
    else
#endif
#endif /* _ME_WINDOW */
#ifdef _ME_CONSOLE
    {
	TTclose();				/* stty to old settings */
	ss = system((char *)getShellCmd()) ;
	sgarbf = meTRUE;
	TTopen();
	ss = (ss < 0) ? meFALSE:meTRUE ;
    }
#endif /* _ME_CONSOLE */
#endif
    if(cd)
        meChdir(curdir) ;
    return ss;
}


/* Note: the given string cmdstr must be large enough to strcat
 * " </dev/null" on the end */
int
doShellCommand(meUByte *cmdstr, int flags)
{
    meUByte path[meBUF_SIZE_MAX];      /* pathfrom where to execute */
    meInt systemRet;                   /* return value of last system  */
    int cd, ss;
#ifdef _UNIX
    meWAIT_STATUS ws;
    meUByte *cmdline, *pp; 
#endif

    getFilePath(frameCur->windowCur->buffer->fileName,path);
    cd = (meStrcmp(path,curdir) && (meChdir(path) != -1));

#ifdef _WIN32
    ss = WinLaunchProgram(cmdstr,LAUNCH_SYSTEM|flags, NULL, NULL, 
#if MEOPT_IPIPES
                          NULL,
#endif
                          &systemRet);
#else
#ifdef _UNIX
    /* if no data is piped in then pipe in /dev/null */
    if(((pp=meStrchr(cmdstr,'<')) == NULL) || (flags & LAUNCH_NOWAIT))
    {
        if((cmdline = meMalloc(meStrlen(cmdstr)+16)) == NULL)
            return meFALSE;
        meStrcpy(cmdline,cmdstr);
        if(pp == NULL)
            meStrcat(cmdline," </dev/null");
        if(flags & LAUNCH_NOWAIT)
            meStrcat(cmdline," &");
    }
    else
        cmdline = cmdstr;
    ss = system((char *)cmdline);
    ws = (meWAIT_STATUS)(ss);
    if(WIFEXITED(ws))
    {
        systemRet = WEXITSTATUS(ws);
        ss = meTRUE;
    }
    else
    {
        systemRet = -1000;
        ss = meFALSE;
    }
    if(cmdline != cmdstr)
        meFree(cmdline);
#else
    systemRet = system((char *) cmdstr);
#ifdef _DOS
    /* dos is naughty with modes, a system call could call a progam that
     * changes the screen stuff under our feet and not restore the current
     * mode! The only thing we can do is call TTopen to ensure we restore
     * our mode.
     * We might check all states, but with hidden things like flashing etc.
     * its not worth the effort - sorry, you do it if you want.
     */
    TTopen();
#endif
    ss = (systemRet < 0) ? meFALSE:meTRUE;
#endif
#endif

    if(cd)
        meChdir(curdir);
    meStrcpy(resultStr,meItoa(systemRet));
    return ss;
}

int
meShellCommand(int f, int n)
{
    meUByte *cmdstr, cmdbuff[meBUF_SIZE_MAX+20];
    meBuffer *bp ;
    
    /* get the line wanted */
    if((meGetString((meUByte *)"System", 0, 0, cmdbuff, meBUF_SIZE_MAX)) <= 0)
        return meABORT ;
    if(n & LAUNCH_BUFCMDLINE)
    {
        if((bp=bfind(cmdbuff,0)) == NULL)
            return mlwrite(MWABORT,(meUByte *)"[%s: no such buffer]",cmdbuff);
        cmdstr = meLineGetText(meLineGetNext(bp->baseLine)) ;
    }
    else
        cmdstr = cmdbuff ;
    f = (n & LAUNCH_USER_FLAGS) ;
    if((n & LAUNCH_WAIT) == 0)
        f |= LAUNCH_NOWAIT ;
    
    return doShellCommand(cmdstr,f) ;
}

#if MEOPT_IPIPES

/*
 *---	Interactive PIPE into the list buffer.
 */
#ifdef _WIN32

static BOOL CALLBACK
ipipeFindChildWindow(HWND hwnd, LPARAM lipipe)
{
    DWORD process ;
    meIPipe *ipipe = (meIPipe *)(lipipe);

    GetWindowThreadProcessId (hwnd,&process);
    if (process == ipipe->processId)
    {
        ipipe->childWnd = hwnd ;
        return meFALSE ;
    }
    /* keep looking */
    return meTRUE ;
}

static HWND
ipipeGetChildWindow(meIPipe *ipipe)
{
    ipipe->childWnd = NULL ;
    EnumWindows(ipipeFindChildWindow,(LPARAM) ipipe);
    return ipipe->childWnd ;
}

#ifdef _WIN32s

#define ipipeKillProcessTree(ppid) meFALSE

#else

#include <tlhelp32.h>

static int
ipipeKillProcessTree(DWORD ppid)
{
    typedef HANDLE (WINAPI *CREATETOOLHELP32SNAPSHOT)(DWORD,DWORD) ;
    typedef BOOL (WINAPI *PROCESS32FIRST)(HANDLE,LPPROCESSENTRY32) ;
    typedef BOOL (WINAPI *PROCESS32NEXT)(HANDLE,LPPROCESSENTRY32) ;

    static int procGetFuncs=0 ;
    static CREATETOOLHELP32SNAPSHOT procCreateSnapshot ;
    static PROCESS32FIRST procGetFirst ;
    static PROCESS32NEXT procGetNext ;
    HANDLE procSnap, procHandle ;
    PROCESSENTRY32 pe ;
    DWORD *pidList ;
    int pidCount, pidCur ;
    
    if(!procGetFuncs)
    {
        HINSTANCE libHandle ; 
        
        procGetFuncs = 1 ;
        if((libHandle = LoadLibrary("kernel32")) != NULL) 
        { 
            procCreateSnapshot = (CREATETOOLHELP32SNAPSHOT) GetProcAddress(libHandle,"CreateToolhelp32Snapshot") ; 
            if(((procGetFirst = (PROCESS32FIRST) GetProcAddress(libHandle,"Process32First")) == NULL) ||
               ((procGetNext = (PROCESS32NEXT) GetProcAddress(libHandle,"Process32Next")) == NULL))
                procCreateSnapshot = NULL ;
        }
    } 
    if(procCreateSnapshot == NULL)
        return meFALSE ;
 
    procSnap = procCreateSnapshot(TH32CS_SNAPPROCESS,0) ; 
    if(procSnap == INVALID_HANDLE_VALUE)
        return meFALSE ;
    
    if((pidList = meMalloc(64*sizeof(DWORD))) == NULL)
    {
        CloseHandle(procSnap) ;
        return meFALSE ;
    }
    pidList[0] = ppid ;
    pidCount = 1 ;
    pidCur = 0 ;
    
    pe.dwSize = sizeof(PROCESSENTRY32) ;
    do {
        ppid = pidList[pidCur] ;
        if(!procGetFirst(procSnap,&pe)) 
            break ;
        do {
            if(pe.th32ParentProcessID == ppid)
            {
                if(((pidCount & 0x3f) == 0) &&
                   ((pidList = meRealloc(pidList,(pidCount+64)*sizeof(DWORD))) == NULL))
                {
                    CloseHandle(procSnap) ;
                    return meFALSE ;
                }
                pidList[pidCount++] = pe.th32ProcessID ;
            }
        } while(procGetNext(procSnap,&pe)) ;
    } while(++pidCur != pidCount) ;
    CloseHandle(procSnap) ;
    
    /* kill from the parent down */
    for(pidCur=0 ; pidCur<pidCount ; pidCur++)
    {
        if((procHandle = OpenProcess(PROCESS_TERMINATE,FALSE,pidList[pidCur])) != NULL)
            TerminateProcess(procHandle,999) ;
    }
    return meTRUE ;
}
#endif
#endif

#if (IPIPE_DEBUG >= 3)
/* ipipeLogWrite; record what goes *to* the child. On Windows this is the only channel by which
 * ME can affect the ConPTY, so a stray or badly timed write is visible nowhere else. Control
 * characters are escaped so the marker stays on one line. */
static void
ipipeLogWrite(const char *where, meUByte *str, int len)
{
    meUByte buff[256];
    int ii, jj=0;

    for(ii=0 ; (ii<len) && (jj<(int)sizeof(buff)-5) ; ii++)
    {
        meUByte cc=str[ii];
        if(cc == 0x1b)
        {
            buff[jj++] = '<';
            buff[jj++] = 'E';
            buff[jj++] = '>';
        }
        else if((cc < 0x20) || (cc == 0x7f))
            jj += sprintf((char *) buff+jj,"<%02x>",cc);
        else
            buff[jj++] = cc;
    }
    buff[jj] = '\0';
    meIPipeLog(":WRITE-%s:%d:[%s]:",where,len,buff);
}
#endif

static void
ipipeWriteString(meIPipe *ipipe, int n, meUByte *str)
{
    while(n--)
    {
#ifdef _WIN32
        DWORD written ;
#if (IPIPE_DEBUG >= 3)
        ipipeLogWrite("STR",str,(int) meStrlen(str));
#endif
        WriteFile(ipipe->outWfd,str,(DWORD) meStrlen(str),&written,NULL) ;
#else
#if (IPIPE_DEBUG >= 3)
        ipipeLogWrite("STR",str,(int) meStrlen(str));
#endif
        write(ipipe->outWfd,str,meStrlen(str)) ;
#endif
    }
}


static void
ipipeKillBuf(meIPipe *ipipe, int type)
{

    if(ipipe->pid > 0)
    {
        if(type == 0)
        {
#ifdef _WIN32
            {
                /* on windows surprise surprise writing C-c to stdin does not work very
                 * well, it does not have the same effect. There are a few extra things
                 * we can do and hte combination of doing them all does seem to have
                 * the right effect - but this may be very machine dependant.
                 * 
                 * One thing we can do is make the child process window have the 
                 * current input focus and then send the key strokes down to it.
                 * But to do this we must have a foreground window and we must attatch
                 * the thread to the current thread so we can steal the current window.
                 */
                HWND  foreWnd, chldWnd ;
                DWORD foreThread, chldThread ;
                BOOL success=0 ;
                
                if(((foreWnd=GetForegroundWindow()) != NULL) &&
                   ((chldWnd=ipipeGetChildWindow(ipipe)) != NULL))
                {
                    foreThread = GetWindowThreadProcessId(foreWnd,NULL) ;
                    if((GetCurrentThreadId() == foreThread) || 
                       !AttachThreadInput(GetCurrentThreadId(),foreThread,meTRUE))
                        foreThread = 0 ;
                    
                    chldThread = ipipe->processId ;
                    if((GetCurrentThreadId() == chldThread) || 
                       !AttachThreadInput(GetCurrentThreadId(),chldThread,meTRUE))
                        chldThread = 0;
                    
                    /* Set the fore window to the child */
                    if((success=SetForegroundWindow(chldWnd)) != 0)
                    {
                        /* success, now we can start sending the string down */
                        BYTE scanCodeCtrl, scanCodeC ;
                        
                        scanCodeCtrl = (BYTE) MapVirtualKey(VK_CONTROL, 0);
                        scanCodeC = (BYTE) MapVirtualKey('C', 0);
                        keybd_event(VK_CONTROL,scanCodeCtrl,0,0) ;
                        keybd_event('C',scanCodeC,0,0) ;
                        keybd_event('C',scanCodeC,KEYEVENTF_KEYUP,0) ;
                        keybd_event(VK_CONTROL,scanCodeCtrl,KEYEVENTF_KEYUP,0) ;
                        
                        /* call halt here and let the other process get the keys -
                         * the length of the sleep is a fine balance, too small and the
                         * child process will not get the Ctrl-C, but if its to large the
                         * user could press a key, entering a char or changing the state
                         * of the Ctrl or Shift keys - these events should go to the original
                         * foreWnd so that app will get very confused! So the sleep should
                         * be as small as possible
                         */
                        Sleep(50);
                        
                        /* Swap back to the original fore window */
                        SetForegroundWindow(foreWnd);
                        
                    }
                    /* Detach the threads */
                    if(foreThread)
                        AttachThreadInput(GetCurrentThreadId(),foreThread,meFALSE);
                    if(chldThread)
                        AttachThreadInput(GetCurrentThreadId(),chldThread,meFALSE);
                }
            
                /* one other thing we can do is to send a Ctrl-C event if we are trying
                 * to write a C-c (0x03), but we will write the string as well */
                GenerateConsoleCtrlEvent(CTRL_C_EVENT,ipipe->processId) ;
                
                /* if we've succeeded in sending the Ctrl-C keys and the event
                 * then quit now - if they don't work, nothing will */
                if(success)
                    return;
            }
#endif
            /* send a control-C signal */
            ipipeWriteString(ipipe,1,(meUByte *) "\x03") ;
            return;
        }
#ifdef _WIN32
        if(ipipe->pid > 0)
        {
            if((type >= 0) || (ipipeKillProcessTree(ipipe->processId) <= 0))
                /* kill only parent or cannot traverse process tree - so only Terminate this one */
                TerminateProcess(ipipe->process,999) ;
            /* On windows theres no child signal, so flag as killed */
            ipipe->pid = -5 ;
        }
        /* Close the process */
        CloseHandle(ipipe->process);
#else
        if(type < 0)
            kill(0-ipipe->pid,0-type);
        else
            kill(ipipe->pid,type);
#endif
    }
}

int
ipipeKill(int f, int n)
{
    meBuffer *cbp=frameCur->windowCur->buffer;
    meIPipe *ipipe;

    if(!meModeTest(cbp->mode,MDPIPE))
    {
        TTbell();
        return meFALSE;
    }
    ipipe = ipipes;
    while(ipipe->bp != cbp)
        ipipe = ipipe->next;
    if(f == meFALSE)
        /* Use -ve arg by default to kill the whole group/tree */
#ifdef _WIN32
        n = -1;
#else
        n = (meSystemCfg & meSYSTEM_TERMSIG) ? -SIGTERM:-SIGKILL;
#endif
    ipipeKillBuf(ipipe,n);
    return meTRUE;
}

void
ipipeRemove(meIPipe *ipipe)
{
#ifndef _WIN32
    meSigHold();
#endif
    if(ipipe->pid > 0)
#ifdef _WIN32
        ipipeKillBuf(ipipe,-1);
#else
        ipipeKillBuf(ipipe,(meSystemCfg & meSYSTEM_TERMSIG) ? -SIGTERM:-SIGKILL);
#endif

    if(ipipe == ipipes)
        ipipes = ipipe->next;
    else
    {
        meIPipe *pp ;

        pp = ipipes ;
            while(pp->next != ipipe)
                pp = pp->next;
        pp->next = ipipe->next;
    }
    noIpipes-- ;
    if(ipipe->bp != NULL)
    {
        meModeClear(ipipe->bp->mode,MDPIPE);
        meModeClear(ipipe->bp->mode,MDLOCK);
    }
#ifdef _WIN32
    /* if we're using a child activity thread the close it down */
    if(ipipe->thread != NULL)
    {
        DWORD exitCode;

        if(GetExitCodeThread(ipipe->thread,&exitCode) && (exitCode == STILL_ACTIVE))
        {
            /* get the thread going again */
            ipipe->flag |= meIPIPE_CHILD_EXIT;
            SetEvent(ipipe->threadContinue);
            if(WaitForSingleObject(ipipe->thread,200) != WAIT_OBJECT_0)
                TerminateThread(ipipe->thread,0);
        }
#ifndef USE_BEGINTHREAD
        CloseHandle (ipipe->thread);
#endif
    }
    if(ipipe->threadContinue != NULL)
        CloseHandle(ipipe->threadContinue);
    if(ipipe->childActive != NULL)
        CloseHandle(ipipe->childActive);
    CloseHandle(ipipe->rfd);
    CloseHandle(ipipe->outWfd);
    meIPipeConPTYClose(ipipe);
#else
    close(ipipe->rfd) ;
    if(ipipe->rfd != ipipe->outWfd)
        close(ipipe->outWfd);
    meSigRelease();
#endif
    free(ipipe);
}

/* IPIPE_DEBUG is set in estruct.h so winterm.c can log the launch as well */
#ifdef IPIPE_DEBUG
static FILE *logFp=NULL;

static meTime ipipeTimeNow(void);

/* meIPipeLog; write a marker to ./ipipe.log, opening the file on first use. The launch and
 * child-activity code runs before the ipipe buffer exists so it cannot rely on the open in
 * doIpipeCommand, and it lives in another file so it cannot see logFp. Each marker is stamped
 * with the millisecond so the gaps between the launch, the resize and the child's first output
 * can be read off - who was first matters more than what was called. */
void
meIPipeLog(const char *fmt, ...)
{
    va_list args;

    if((logFp == NULL) && ((logFp = fopen("./ipipe.log","wb+")) == NULL))
        return;
    fprintf(logFp,"[%ld]",(long) ipipeTimeNow());
    va_start(args,fmt);
    vfprintf(logFp,fmt,args);
    va_end(args);
    fflush(logFp);
}
#endif

#ifdef _WIN32

static int
readFromPipe(meIPipe *ipipe, int nbytes, meUByte *buff, int doSleep)
{
    DWORD bRead, bAvail;

    /* See if process has ended first */
    if(ipipe->pid < 0)
        return ipipe->pid;
#if MEOPT_CLIENTSERVER
    if(ipipe->pid == 0)
    {
        if(ttServerToRead == 0)
            return 0 ;
        if(nbytes > ttServerToRead)
            nbytes = ttServerToRead;
        if(ReadFile(ipipe->rfd,buff,nbytes,&bRead,NULL) == 0)
            return -1;
#ifdef IPIPE_DEBUG
        if((bRead > 0) && (logFp != NULL))
        {
            fwrite(buff,1,bRead,logFp);
#if (IPIPE_DEBUG >= 3)
            fwrite("ZZAZ",1,4,logFp);
            fflush(logFp);
#endif
        }
#endif
        return (int) bRead;
    }
#endif
    if(ipipe->flag & meIPIPE_CHILD_EXIT)
    {
        GetExitCodeProcess(ipipe->process,(LPDWORD) &(ipipe->exitCode));
        CloseHandle(ipipe->process);
        ipipe->pid = -4 ;
        return ipipe->pid;
    }
    if(ipipe->flag & meIPIPE_NEXT_CHAR)
    {
        buff[0] = ipipe->nextChar;
        ipipe->flag &= ~meIPIPE_NEXT_CHAR;
        bRead = 1;
        --nbytes;
    }
    else
        bRead = 0;
    /* Must peek on a pipe cos if we try to read too many this will fail */
    if((PeekNamedPipe(ipipe->rfd,(LPVOID) NULL,(DWORD) 0,(LPDWORD) NULL,&bAvail,(LPDWORD) NULL) == 0) || (bAvail <= 0))
    {
        if(doSleep)
            /* Currently in the middle of a terminal code, throw a short sleep to give the child process a
             * chance to write the rest of the code, set to a very short time due to Windows timer granularity */
            Sleep(2);
        if(!doSleep || (PeekNamedPipe(ipipe->rfd,(LPVOID) NULL,(DWORD) 0,(LPDWORD) NULL,&bAvail,(LPDWORD) NULL) == 0) || (bAvail <= 0))
        {
#ifdef IPIPE_DEBUG
            if((bRead > 0) && (logFp != NULL))
            {
                fwrite(buff,1,bRead,logFp);
#if (IPIPE_DEBUG >= 3)
                fwrite("ZZBZ",1,4,logFp);
                fflush(logFp);
#endif
            }
#endif
            return bRead;
        }
    }
    if(bAvail > (DWORD) nbytes)
        bAvail = (DWORD) nbytes;
    if(ReadFile(ipipe->rfd,buff+bRead,bAvail,&bAvail,NULL) == 0)
        return -1;
    if((bRead += bAvail) == 0)
        return 0;
    if((ipipe->flag & meIPIPE_HAVE_READ) == 0)
    {
        ipipe->flag |= meIPIPE_HAVE_READ;
        if((bRead > 30) && (buff[0] == 0x1b))
        {
            int ii=0, ll=bRead-10;
            meUByte cc, c2;
            /* Windows ConPTYs always start by initialising the screen, propably because of the base design which is to get the child process to print to a
             * virtual terminal and send the host a set of codes to bring about the changes needed to get them in sync (a diff engine as it were), so it needs
             * a clean slate. The init largely involves going to to the top ('\E[H') printing new line (\r\n) for as many lines as the console has and then back
             * to the top - skip is as we have a clean slate and this will add a lot of pointless blank lines. */
            while((ii < ll) && ((cc=buff[ii++]) == 0x1b) && (buff[ii++] == '['))
            {
                if((c2=buff[ii++]) == '?')
                    c2 = buff[ii++];
                while((c2 == ';') || ((c2 >= '0') && (c2 <= '9')))
                    c2 = buff[ii++];
                if((c2 < 'A') || (c2 > 'z') || ((c2 > 'Z') && (c2 < 'a')))
                    break;
            }
            if((ii < ll) && (cc == '\r') && (c2 == 'H') && (buff[ii++] == '\n'))
            {
                while((ii < ll) && ((cc=buff[ii++]) == '\r') && (buff[ii++] == '\n'))
                    ;
                if((cc == 0x1b) && (buff[ii++] == '[') && (buff[ii++] == 'H'))
                {
#ifdef IPIPE_DEBUG
                    if(logFp != NULL)
                    {
                        fwrite(":SKIP-INIT:",1,11,logFp);
#if (IPIPE_DEBUG >= 3)
                        fwrite(buff,1,ii,logFp);
                        fwrite(":END:",1,5,logFp);
#endif
                    }
#endif
                    bRead -= ii;
                    memmove(buff,buff+ii,bRead);
                }
            }
        }
    }
#ifdef IPIPE_DEBUG
    if((bRead > 0) && (logFp != NULL))
    {
        fwrite(buff,1,bRead,logFp);
#if (IPIPE_DEBUG >= 3)
        fputc('Z',logFp);
        fputc('Z',logFp);
        fputc('C'+doSleep,logFp);
        fputc('Z',logFp);
        fflush(logFp);
#endif
    }
#endif
    return (int) bRead;
}

#else

#if MEOPT_CLIENTSERVER

#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <netinet/in.h>

#endif

static int
readFromPipe(meIPipe *ipipe, int nbytes, meUByte *buff, int doSleep)
{
    int ii;

#if MEOPT_CLIENTSERVER
    if(ipipe->pid == 0)
    {
        if ((ii = recv(ipipe->rfd,(char *) buff,nbytes,0)) < 0)
            ii = 0;
    }
    else
#endif
    {
        if(doSleep)
        {
            /* Currently in the middle of a terminal code, throw a short sleep to give the child
               process a chance to write the rest of the code, currently 20ms */
            fd_set rfds;
            struct timeval tv;

            FD_ZERO(&rfds);
            FD_SET(ipipe->rfd,&rfds);
            tv.tv_sec = 0;
            tv.tv_usec = 20000;
            if(select(ipipe->rfd+1,&rfds,NULL,NULL,&tv) <= 0)
                return 0;
        }
        ii = read(ipipe->rfd,buff,nbytes);
    }
#ifdef IPIPE_DEBUG
    if((ii > 0) && (logFp != NULL))
    {
        fwrite(buff,1,ii,logFp);
#if (IPIPE_DEBUG >= 3)
        fputc('Z',logFp);
        fputc('Z',logFp);
        fputc('C'+doSleep,logFp);
        fputc('Z',logFp);
        fflush(logFp);
#endif
    }
#endif
    return ii;
}


#endif

#ifdef IPIPE_DEBUG
/* (lineCount-1 - dotLineNo) == (noRows-1 - curRow) must hold. When the left side is larger the
 * buffer has gained rows nothing can reclaim - ipipeAddLine only inserts *before* lp_old.
 * The counters are what is in doubt, so the list is walked and both results reported. */
static void
ipipeCheckInvariant(meIPipe *ipipe, const char *where)
{
    meBuffer *bp;
    meLine   *lp;
    int       count=0, dotNo=-1, below, want;

    if((logFp == NULL) || (ipipe->noRows <= 0) || ((bp = ipipe->bp) == NULL))
        return;
    for(lp = meLineGetNext(bp->baseLine) ; lp != bp->baseLine ; lp = meLineGetNext(lp))
    {
        if(lp == bp->dotLine)
            dotNo = count;
        count++;
    }
    if(bp->dotLine == bp->baseLine)
        /* the cursor is one past the last line, which is a position not a fault */
        dotNo = count;
    if(dotNo < 0)
    {
        /* the cursor line is in no buffer - it has been retired by a commit or a wrap and the
         * dot left pointing at it, so anything that redraws is reading freed memory */
        fprintf(logFp,":INVAR:%s:DOTLINE-LOST:curRow=%d noRows=%d dotLineNo=%d lineCount=%d walked=%d:",
                where,(int)ipipe->curRow,(int)ipipe->noRows,(int)bp->dotLineNo,(int)bp->lineCount,count);
        fflush(logFp);
        return;
    }
    below = count - 1 - dotNo;
    want = ipipe->noRows - 1 - ipipe->curRow;
    /* below < want is benign, it just means the buffer has yet to fill the screen */
    if((below > want) || (count != (int)bp->lineCount) || (dotNo != (int)bp->dotLineNo))
        fprintf(logFp,":INVAR:%s:below=%d want=%d excess=%d curRow=%d noRows=%d dotLineNo=%d/%d lineCount=%d/%d:",
                where,below,want,below-want,(int)ipipe->curRow,(int)ipipe->noRows,
                dotNo,(int)bp->dotLineNo,count,(int)bp->lineCount);
    else
        return;
    fflush(logFp);
}
#define ipipeCheckInvar(ipipe,where) ipipeCheckInvariant(ipipe,where)
#define ipipeCheckWins(where) ipipeCheckWindows(bp,where)
/* walk the list rather than trust the counters, see ipipeWalk below */
static int ipipeWalk(meBuffer *bp, int *dotNo);
static int ipipeLineNo(meBuffer *bp, meLine *lp);
static int ipipeCheckWindows(meBuffer *bp, const char *where);
/* Note a state that arrived wrong and was corrected rather than propagated. */
#define ipipeLogFix(what,was,now)                                            \
    meIPipeLog(":FIX:%s:%d->%d:",(what),(int)(was),(int)(now))
/* An '\E[8;<rows>;<cols>t' from the child, ie the size the far end thinks it has, reported
 * against the size ME set it to. A disagreement makes every column ME clamps to maxOff wrong. */
#define ipipeLogSize(prmC,prmS,prmL)                                         \
    meIPipeLog(":CHILD-SIZE:%d %d vs ours %d %d:",                           \
               ((prmC) > 1) ? (int)(prmS)[1]:-1,(int)(prmL),                 \
               (int)ipipe->noRows,(int)ipipe->noCols)
/* A cursor position the child asked for that is off ME's screen, so ME has to fake it. */
#define ipipeLogClampPos(what,want,got)                                      \
    meIPipeLog(":CUP-CLAMP:%s:%d->%d:",(what),(int)(want),(int)(got))
/* Checked once per character, so the cheap pointer test comes first and the list is only walked
 * to fill in the report. bp->dotLine must be the line being built and every line added since the
 * last commit went in above it, putting it at bp->dotLineNo + noLines. First break only, cc being
 * the character just processed. */
#define ipipeCheckBreak()                                                    \
do {                                                                         \
    if(!cupBroke && (lp_old != bp->dotLine))                                 \
    {                                                                        \
        int wDot, wCnt = ipipeWalk(bp,&wDot);                                \
        cupBroke = 1;                                                        \
        meIPipeLog(":BREAK:after=0x%02x row=%d dotNo=%d/%d lpNo=%d cnt=%d/%d nl=%u:", \
                   (int)cc,curRow,wDot,(int)bp->dotLineNo,                   \
                   ipipeLineNo(bp,lp_old),wCnt,(int)bp->lineCount,           \
                   (unsigned int)noLines);                                   \
    }                                                                        \
} while(0)
#define ipipeLogClamp(want,got)                                              \
do {                                                                         \
    if(logFp != NULL)                                                        \
    {                                                                        \
        fprintf(logFp,":WALK-CLAMP:want=%d got=%d:",(int)(want),(int)(got));  \
        fflush(logFp);                                                       \
    }                                                                        \
} while(0)
#define ipipeLogDrop(what)                                                   \
do {                                                                         \
    if(logFp != NULL)                                                        \
    {                                                                        \
        fprintf(logFp,":ESCDROP:%s:",what);                                  \
        fflush(logFp);                                                       \
    }                                                                        \
} while(0)
#else
#define ipipeCheckInvar(ipipe,where)
#define ipipeCheckWins(where)
#define ipipeLogDrop(what)
#define ipipeLogClamp(want,got)
#define ipipeLogFix(what,was,now)
#define ipipeLogSize(prmC,prmS,prmL)
#define ipipeLogClampPos(what,want,got)
#define ipipeCheckBreak()
#endif

static meTime
ipipeTimeNow(void)
{
    struct meTimeval tp;

    gettimeofday(&tp,NULL);
    return ((tp.tv_sec-startTime)*1000) + (tp.tv_usec/1000);
}

#define ipipeGetNextChar(ipipe,cc,rbuff,curROff,curRRead,doSleep)            \
((curROff < curRRead) ?                                                      \
 (((cc)=rbuff[curROff++]), 1):                                               \
 (((curRRead=readFromPipe(ipipe,meBUF_SIZE_MAX,rbuff,doSleep)) > 0) ?        \
  (((cc)=rbuff[0]),curROff=1): 0))

#define ipipeAddLine(ipipe,lp_old,buff,cbuff,dotoP)                          \
((ipipe->flag & meIPIPE_ANSICOLOR) ? ipipeAddColorLine(lp_old,buff,cbuff,dotoP):addLine(lp_old,buff))
#define ipipeDecodeLine(ipipe,src,buff,cbuff,offs)                           \
((ipipe->flag & meIPIPE_ANSICOLOR) ? ipipeDecodeColorLine(src,buff,cbuff,offs):(meStrcpy(buff,src),offs))

#define ipipeStoreInputPos()                                                 \
do {                                                                         \
    meLine *lp_new;                                                          \
    int doto = (int) (p1 - buff);                                            \
    noLines += ipipeAddLine(ipipe,lp_old,buff,cbuff,&doto);                  \
    lp_new = meLineGetPrev(lp_old);                                          \
    if(lp_old != bp->baseLine)                                               \
    {                                                                        \
        noLines--;                                                           \
        lp_new->next = lp_old->next;                                         \
        lp_old->next->prev = lp_new;                                         \
        meLineSwap(bp,lp_old,lp_new);                                        \
        meFree(lp_old);                                                      \
    }                                                                        \
    else                                                                     \
        bp->dotLineNo--;                                                     \
    bp->dotLineNo += noLines;                                                \
    bp->lineCount += noLines;                                                \
    ipipe->curRow = curRow;                                                  \
    if(bp->lineCount <= ipipe->noRows)                                       \
        bp->vertScroll = 0;                                                  \
    else                                                                     \
        bp->vertScroll = bp->dotLineNo-curRow;                               \
    bp->dotLine = lp_new;                                                    \
    bp->dotOffset = (meUShort) doto;                                         \
    meBufferUpdateLocation(bp,noLines,bp->dotOffset);                        \
} while(0)


/* Maximum number of parameters collected from a CSI sequence, must be large enough to hold a
 * combined SGR setting both an rgb fg & bg colour, i.e. '\E[0;38;2;r;g;b;48;2;r;g;bm' */
#define meIPIPE_PRM_MAX 16

static int
ipipeDecodeColorLine(const meUByte *src, meUByte *buff, meUByte *cbuff, int offs)
{
    meUByte cc='A', dd;
    int rr=0, ii=0;
    
    while((dd=*src++) != '\0')
    {
        if(dd == '\x03')
        {
            if(--offs == 0)
                rr = ii;
            if((dd = *src++) != '\0')
                cc = dd;
            else
                /* consume a bare \CC at the end of a line */ 
                --src;
        }
        else
        {
            cbuff[ii] = cc;
            buff[ii++] = dd;
        }            
        if(--offs == 0)
            rr = ii;
    }
    buff[ii] = '\0';
    return rr;
}

static void
ipipeClearColorLine(meLine *lp)
{
    meUByte *ss=lp->text;
    int ll = 0;
    while((ss=meStrchr(ss,'\x03')) != NULL)
    {
        ll += 2;
        ss++;
    }
    ll = lp->length - ll;
    memset(lp->text,' ',ll);
    lp->text[ll] = '\0';
    lp->unused += (meUByte) (lp->length - ll);
    lp->length = (meUShort) ll;
    lp->flag |= meLINE_CHANGED;
}

static int
ipipeAddColorLine(meLine *lp, const meUByte *buff, const meUByte *cbuff, int *dotoP)
{
    meUByte encbuff[3*meBUF_SIZE_MAX+1];
    meUByte cc='A', sc;
    meUByte *op=encbuff;
    int doto = (dotoP == NULL) ? 0:*dotoP;
    int i = 0;
    while(buff[i])
    {
        sc = (cbuff[i]) ? cbuff[i] : 'A';
        if(sc != cc)
        {
            cc = sc;
            *op++ = '\x03';
            *op++ = cc;
        }
        *op++ = buff[i++];
        if(--doto == 0)
            *dotoP = (int) (op-encbuff);
    }
    if(cc != 'A')
    {
        *op++ = '\x03';
        *op++ = 'A';
    }
    *op = '\0';
    return addLine(lp,encbuff);
}

/* ipipeBaseToColor - convert one of the 16 standard terminal colours, i.e. 0-7 and their
 * bright versions 8-15, into the colour encoding */
static meUByte
ipipeBaseToColor(int idx)
{
    meUByte cc;

    cc = (meUByte) ((idx & 0x07) | meIPIPE_COL_SET);
    if(idx & 0x08)
        cc |= meIPIPE_COL_BRIGHT;
    if(((idx & 0x07) == 0) || ((idx & 0x07) == 7))
        /* black & white have no hue */
        cc |= meIPIPE_COL_GREY;
    return cc;
}

/* ipipeRgbToColor - reduce a 24 bit rgb colour to one of the 8 base colours plus a shade,
 * see meIPIPE_COL_* for the returned encoding */
static meUByte
ipipeRgbToColor(int r, int g, int b)
{
    meUByte cc;
    int mx, mn;

    mx = (r > g) ? r:g;
    if(b > mx)
        mx = b;
    mn = (r < g) ? r:g;
    if(b < mn)
        mn = b;
    if((mx < 0x20) || (((mx - mn) << 4) < mx))
    {
        /* Either too dark, or too little colour in relation to the brightness (i.e. less than
         * about 6% saturated) for the hue to be significant, so this is a neutral grey. Note
         * that the test must be relative as the pale backgrounds used to mark up diffs have a
         * very low absolute colour range, e.g. 230,255,236 is a green. Use black or white as
         * the base colour depending on which end of the scale the grey is closest to */
        if(mx < 0x40)
            cc = 0 | meIPIPE_COL_DARK;              /* black                        */
        else if(mx < 0xa0)
            cc = 0 | meIPIPE_COL_BRIGHT;            /* dark grey, i.e. bright black */
        else if(mx < 0xd8)
            cc = 7 | meIPIPE_COL_DARK;              /* light grey, i.e. dark white  */
        else
            cc = 7 | meIPIPE_COL_BRIGHT;            /* white                        */
        return (meUByte) (cc | meIPIPE_COL_SET | meIPIPE_COL_GREY | meIPIPE_COL_RGB);
    }
    /* Split the channels about the mid point to get the nearest of the 6 hues, the base colour
     * bits are in the same order as the channels, i.e. bit 0 = red, 1 = green & 2 = blue */
    mn = (mx + mn) >> 1;
    cc = (meUByte) (((r > mn) ? 0x01:0) | ((g > mn) ? 0x02:0) | ((b > mn) ? 0x04:0));
    if(mx >= 0xc0)
        cc |= meIPIPE_COL_BRIGHT;
    else if(mx < 0x80)
        cc |= meIPIPE_COL_DARK;
    return (meUByte) (cc | meIPIPE_COL_SET | meIPIPE_COL_RGB);
}

/* ipipePalToColor - convert a 256 colour palette index to a base colour plus shade */
static meUByte
ipipePalToColor(int idx)
{
    static const meUByte cubeLvl[6] = { 0x00, 0x5f, 0x87, 0xaf, 0xd7, 0xff };

    if(idx < 0)
        return 0;
    if(idx < 16)
        /* The 16 standard terminal colours, treat these as if given as \E[3xm or \E[9xm */
        return ipipeBaseToColor(idx);
    if(idx < 232)
    {
        /* 6x6x6 colour cube */
        idx -= 16;
        return ipipeRgbToColor(cubeLvl[(idx / 36) % 6],cubeLvl[(idx / 6) % 6],cubeLvl[idx % 6]);
    }
    if(idx < 256)
    {
        /* 24 step grey ramp */
        idx = 8 + ((idx - 232) * 10);
        return ipipeRgbToColor(idx,idx,idx);
    }
    return 0;
}

/* ipipeAnsiExtColor - convert an extended colour SGR argument list, i.e. the parameters
 * following the 38 (fg) or 48 (bg), which is either '5;n' selecting a 256 palette colour or
 * '2;r;g;b' giving a 24 bit rgb colour. sub flags whether each parameter was introduced by the
 * ':' sub-parameter separator. Returns the number of parameters consumed, 0 if the form is not
 * understood, in which case the caller must abandon the sequence as the number of parameters to
 * be skipped is unknown */
static int
ipipeAnsiExtColor(const int *prm, const meUByte *sub, int cnt, meUByte *colp)
{
    int itu=0, nn;

    if(cnt > 1)
    {
        if(sub[0])
        {
            /* The colon sub-parameter separator was used so the number of arguments belonging
             * to this colour is known exactly, the ITU-T form has an extra colour-space id
             * before the rgb values, e.g. '38:2::r:g:b' */
            for(nn=1 ; (nn < cnt) && sub[nn] ; nn++)
                ;
            cnt = nn;
            itu = (cnt > 4);
        }
        if(prm[0] == 5)
        {
            *colp = ipipePalToColor(prm[1]);
            return 2;
        }
        if(prm[0] == 2)
        {
            nn = (itu) ? 2:1;
            if(cnt > (nn + 2))
            {
                *colp = ipipeRgbToColor(prm[nn],prm[nn+1],prm[nn+2]);
                return nn + 3;
            }
        }
    }
    return 0;
}

/* ipipeAnsiToScheme - convert the current ANSI colour & style into the meth scheme tag with the
 * closest meaning, see macros/meth.emf for the tag character to scheme mapping */
static meUByte
ipipeAnsiToScheme(meUByte fg, meUByte bg, meUByte styl)
{
    if(bg & meIPIPE_COL_SET)
    {
        /* A bright background colour (i.e. \E[10xm) is taken as a request to simply highlight
         * the text, whereas a standard or rgb background is more likely to be conveying a
         * meaning such as a diff addition or removal */
        int hl = ((bg & (meIPIPE_COL_BRIGHT|meIPIPE_COL_RGB)) == meIPIPE_COL_BRIGHT);
        if(bg & meIPIPE_COL_GREY)
            /* A dark grey bg is usually the terminal's own background so ignore it, a light
             * grey or white bg is highlighting the current item */
            return (bg & meIPIPE_COL_MASK) ? 's':'A';   /* .scheme.hlwhite */
        switch(bg & meIPIPE_COL_MASK)
        {
        case 1: return (hl) ? 'u':'O';  /* red bg     -> .scheme.hlred:.scheme.gdfrej      */
        case 2: return (hl) ? 'v':'N';  /* green bg   -> .scheme.hlgreen:.scheme.gdfsel    */
        case 3: return (hl) ? 'w':'M';  /* yellow bg  -> .scheme.hlyellow:.scheme.gdfchange*/
        case 4: return 'x';             /* blue bg    -> .scheme.hlblue                    */
        case 5: return 'y';             /* magenta bg -> .scheme.hlmagenta                 */
        case 6: return 'z';             /* cyan bg    -> .scheme.hlcyan                    */
        case 7: return 's';             /* white bg   -> .scheme.hlwhite                   */
        default: return 'A';            /* black bg, assume the terminal's own background  */
        }
    }
    if(fg & meIPIPE_COL_SET)
    {
        if(fg & meIPIPE_COL_GREY)
        {
            /* Grey text darker than mid-grey (which includes \E[90m) is being dimmed down to
             * de-emphasize it, e.g. placeholder or supplementary help text, anything lighter
             * is simply the normal text colour */
            if(!(fg & meIPIPE_COL_MASK))
                return 'h';             /* dim grey fg -> .scheme.comment */
        }
        else
        {
            switch(fg & meIPIPE_COL_MASK)
            {
            case 1: return (styl & meIPIPE_STY_BOLD) ? 'k':'R'; /* red fg -> (bold) .scheme.error:.scheme.rmv */
            case 2: return 'Q';         /* green fg   -> .scheme.add   */
            case 3: return 'l';         /* yellow fg  -> .scheme.warn  */
            case 4: return 'S';         /* blue fg    -> .scheme.dir   */
            case 5: return 'S';         /* magenta fg -> .scheme.dir   */
            case 6: return 'm';         /* cyan fg    -> .scheme.info  */
            }
        }
    }
    if(styl & meIPIPE_STY_DIM)
        /* faint text -> .scheme.comment */
        return 'h';
    /* bold -> .scheme.bold */
    return (styl & 0x07) ? 'C'+(styl & 0x07):'A';
}

void
ipipeRead(meIPipe *ipipe)
{
    meBuffer *bp=ipipe->bp;
    meLine   *lp_old;
    int     len, maxOff, curRow, ii;
    meUInt  noLines;
    meUByte  *p1, cc=0, buff[meBUF_SIZE_MAX+1], cbuff[meBUF_SIZE_MAX+1], rbuff[meBUF_SIZE_MAX];
    int     curROff=0, curRRead=0;
    int     prmA, prmL;
#ifdef IPIPE_DEBUG
    int     cupBroke=0;                 /* the dot bookkeeping has already been reported broken */
#endif
#ifdef _WIN32
    /* On Windows ConPTY explicitly generates a new line blank line for wrapped text rather than rely on the terminal handler,
     * this needs to be caught and handled properly otherwise an additional line is inserted at the end of the buffer.
     * The blank line insertion sequence is NL (\r\n) followed by 'ESC [ y;x H' move where y is the starting line and x is the last column,
     * followed by text which triggers the line wrap. If this sequence occurs we must avoid adding a 2nd line on the text-wrap. */
    int scrollWrapCUP = 0;
#else
#define scrollWrapCUP 0
#endif
    
    maxOff = ipipe->noCols;
#ifdef _UNIX
    meSigHold();
#if MEOPT_CLIENTSERVER
    if(ipipe->pid == 0)
    {
        struct sockaddr_un cssa ;	/* for unix socket address */

        ii = sizeof(struct sockaddr_un);
        cssa.sun_family = AF_UNIX;
        ipipe->rfd = accept(ipipe->rfd,(struct sockaddr *)&cssa, (void *)&ii);
    }
#endif
#endif
    meAnchorGet(bp,'I');
    if(meModeTest(bp->mode,MDLOCK))
    {
        /* Work out which windows are locked to the current cursor position */
        meWindow *wp;
        
        meFrameLoopBegin();
        wp = loopFrame->windowList;
        while(wp != NULL)
        {
            if((wp->buffer == bp) &&
               (wp->dotLine == bp->dotLine) &&
               (wp->dotOffset == bp->dotOffset))
                break;
            wp = wp->next;
        }
        meFrameLoopBreak(wp != NULL);
        meFrameLoopEnd();
        if(wp == NULL)
            bp->intFlag &= ~BIFLOCK;
        else
            bp->intFlag |= BIFLOCK;
    }
    /* A dot on the baseLine is this code's way of saying the cursor is one past the last line,
     * so its line number must be the line count - see the baseLine branch of ipipeStoreInputPos,
     * which subtracts one on the strength of it. Derive it rather than trust what arrived: the
     * 'I' anchor cannot preserve this position at all, as meAnchorGet's forward search tests
     * only the real lines and never comes back round to the sentinel, so it fails to find the
     * anchor and leaves whatever stale dotLine/dotLineNo pair the buffer happened to hold. */
    if((bp->dotLine == bp->baseLine) && (bp->dotLineNo != bp->lineCount))
    {
        ipipeLogFix("eob-dotno",(int)bp->dotLineNo,(int)bp->lineCount);
        bp->dotLineNo = bp->lineCount;
    }
    /* This is a quick sanity check which is needed if the buffer has
     * been changed by something. If curRow becomes greater than dotLineNo
     * the vertScroll becomes negative and things go wrong.
     * Discovered problem when using gdb mode as the gdb input handler
     * kills ^Z^Z lines making curRow > dotLineNo.
     */
    if((curRow=ipipe->curRow) > bp->dotLineNo)
        curRow = bp->dotLineNo;
    len = bp->dotOffset;
    lp_old = bp->dotLine;
    meBufferStoreLocation(lp_old,bp->dotOffset,bp->dotLineNo);
    len = ipipeDecodeLine(ipipe,lp_old->text,buff,cbuff,bp->dotOffset);
    p1 = buff+len;
    noLines = 0;
    if(ipipe->oscSplit != 0)
    {
        if((ipipeTimeNow() - ipipe->oscSplit) <= meIPIPE_OSC_TIMEOUT)
            goto osc_consume;
        ipipe->oscSplit = 0;
        ipipeLogDrop("OSC-EXPIRED");
    }
    for(;;)
    {
        ipipeCheckBreak();
        if(!ipipeGetNextChar(ipipe,cc,rbuff,curROff,curRRead,scrollWrapCUP))
        {
            ipipe->ansiCc = 'A';
            if((ipipe->pid >= -1) || (ipipe->pid < -5))
                /* none left to read */
                break;
            if((ipipe->flag & meIPIPE_RAW) != 0)
            {
                /* Do not annotate the end of the ipipe in raw mode */
                *p1 = '\0';
                cc = 0;
            }
            else
            {
                cc = meCHAR_NL;
                if(ipipe->pid == -4)
                    ii = sprintf((char *) p1,"[EXIT %d]",(int) ipipe->exitCode);
                else
                {
                    meUByte *ins;
                    if(ipipe->pid == -3)
                    {
                        ipipe->exitCode = -1000-9;
                        ins = (meUByte *)"[KILLED]";
                    }
                    else if(ipipe->pid == -2)
                    {
                        ipipe->exitCode = -1000-11;
                        ins = (meUByte *)"[CORE DUMP]";
                    }
                    else
                    {
                        ipipe->exitCode = -1000-15;
                        ins = (meUByte *)"[TERMINATED]";
                    }
                    ii = meStrlen(ins);
                    memcpy(p1,ins,ii+1);
                }
                memset(cbuff+len,ipipe->ansiCc,ii);
            }
            ipipe->pid = -1;
        }
        switch(cc)
        {
        case 0: /* ignore */
            break;
        case 0x07:
            TTbell();
            break;
        case 0x08:
            if(p1 != buff)
            {
                p1--;
                len--;
            }
            break;
        case 0x09: /* HT - move the cursor right to the next terminal tab stop (8 based) */
            prmL = 8 - (len & 0x07);
            goto cursor_forward;
        case 0x0a: /* LF */
        case 0x0b: /* VT */
        case 0x0c: /* FF */
            /* LF, VT and FF are the same in moving down one line, the tty driver's ONLCR translation setting affects
             * only LF and effectivrly adds a CR. With no PTY nothing supplies the CR so meIPIPE_LFISNL is forced on */
            if((ipipe->flag & meIPIPE_USEPTY) && !(ipipe->flag & meIPIPE_OVERWRITE) && (curRow+1 < ipipe->noRows))
            {
                /* if in over-write mode and not at the bottom, move instead */
                prmA = curRow + 1;
                prmL = (ipipe->flag & meIPIPE_LFISNL) ? 0:len;
                goto move_cursor_pos;
            }
            ii = ipipeAddLine(ipipe,lp_old,buff,cbuff,NULL);
            noLines += ii;
            if(curRow < ipipe->noRows-1)
                curRow += ii;
            p1 = buff;
            if(!(ipipe->flag & meIPIPE_LFISNL) && (len > 0))
            {
                memset(buff,' ',len);
                if(ipipe->flag & meIPIPE_ANSICOLOR)
                    memset(cbuff,'A',len);
                p1 = buff+len;
            }
            else
                len = 0;
            *p1 = '\0';
#ifdef _WIN32
            /* See scrollWrapCUP comment above */
            scrollWrapCUP = 1;
#endif
            break;
        case 0x0d: /* CR */
            p1 = buff;
            len = 0;
            break;
        case 0x7f: /* DEL - some shells/consoles echo this for erase, treat as backspace */
            ipipeLogDrop("DEL");
            break;
        case 0x1b:
            if((ipipe->flag & meIPIPE_USEPTY) && ipipeGetNextChar(ipipe,cc,rbuff,curROff,curRRead,1))
            {
                int gotQ=0, gotN=0, gotC=0, prmS[meIPIPE_PRM_MAX], prmC=0;
                meUByte prmB[meIPIPE_PRM_MAX];

                prmL=0;
                prmA=-1;
                if(cc == '[')
                {
get_another:
                    if(ipipeGetNextChar(ipipe,cc,rbuff,curROff,curRRead,1))
                    {
                        if(isDigit(cc))
                        {
                            gotN = 1;
                            prmL = prmL*10 + (cc - '0');
                            goto get_another;
                        }
                        switch(cc)
                        {
                        case ':':
                        case ';':
                            /* ':' separates the sub-parameters of a single parameter, only the
                             * extended colour SGRs use them so simply flag which parameters
                             * they are, otherwise the two separators are the same */
                            if(prmC < meIPIPE_PRM_MAX)
                            {
                                prmB[prmC] = (meUByte) gotC;
                                prmS[prmC++] = prmL;
                            }
                            gotC = (cc == ':');
                            prmL = 0;
                            goto get_another;
                        case '<':
                        case '=':
                        case '>':
                            /* ignore private parameter bytes as we don't support any of the codes using them */
                            goto get_another;
                        case '?':
                            gotQ = 1;
                            goto get_another;
                        case '@':
                        {
                            /* ICH - insert prmL blanks, shifting the rest of the line right. Check prmL to avoid overruns */
                            if(!gotN)
                                prmL = 1;
                            if(prmL > (maxOff - len))
                                prmL = maxOff - len;
                            if(prmL <= 0)
                                break;
                            if((ii = meStrlen(p1)) > (maxOff - len - prmL))
                                ii = maxOff - len - prmL;
                            if(ipipe->flag & meIPIPE_ANSICOLOR)
                            {
                                memmove(cbuff+len+prmL,cbuff+len,ii);
                                memset(cbuff+len,(char)ipipe->ansiCc,prmL);
                            }
                            memmove(p1+prmL,p1,ii);
                            p1[prmL+ii] = '\0';
                            memset(p1,' ',prmL);
                            break;
                        }
                        
                        case 'A':
                            if(!gotN)
                                prmL = 1;
                            prmA = curRow - prmL;
                            prmL = len;
                            goto move_cursor_pos;

                        case 'B':
                            if(!gotN)
                                prmL = 1;
                            prmA = curRow + prmL;
                            prmL = len;
                            goto move_cursor_pos;
                        
                        case 'C':
                            if(!gotN)
                                prmL = 1;
cursor_forward:
                            /* CUF - move right, stopping at the right margin. Note the clamp must not go negative! */
                            if((prmL + len) >= maxOff)
                                prmL = maxOff - len - 1;
                            if(prmL <= 0)
                                break;
                            ii = prmL - meStrlen(p1);
                            p1 += prmL;
                            len += prmL;
                            if(ii > 0)
                            {
                                memset(p1-ii,' ',ii);
                                if(ipipe->flag & meIPIPE_ANSICOLOR)
                                    memset(cbuff+(p1-ii-buff),'A',ii);
                                *p1 = '\0';
                            }
                            break;

                        case 'D':
                            /* CUB - move left, stopping at column 0 */
                            if(!gotN)
                                prmL = 1;
                            else if(prmL <= 0)
                                break;
                            if(len < prmL)
                                prmL = len;
                            p1 -= prmL;
                            len -= prmL;
                            break;

                        case 'G':
                        case '`':
                            /* CHA/HPA - cursor horizontal absolute, 1-based */
                            if(!gotN)
                                prmL = 1;
                            prmL--;  /* convert to 0-based */
                            if(prmL < 0)
                                prmL = 0;
                            else if(prmL >= maxOff)
                                prmL = maxOff - 1;
                            ii = prmL - meStrlen(buff);
                            p1  = buff + prmL;
                            len = prmL;
                            if(ii > 0)
                            {
                                memset(p1-ii,' ',ii);
                                if(ipipe->flag & meIPIPE_ANSICOLOR)
                                    memset(cbuff+(p1-ii-buff),'A',ii);
                                *p1 = '\0';
                            }
                            break;

                        case 'H':
                        case 'f':
                            /* cup - ESC [ <row> ; <col> H */
                            if(prmC)
                            {
                                prmA = prmS[0]-1;
                                prmL--;
                            }
                            else
                            {
                                prmA = (gotN) ? prmL-1:0;
                                prmL = 0;
                            }
#ifdef _WIN32
                            /* See scrollWrapCUP comment above */
                            if((scrollWrapCUP == 1) && prmC && (prmA == curRow-1) && (prmL == maxOff-1))
                                scrollWrapCUP = 2;
                            else
                                scrollWrapCUP = 0;
#endif
move_cursor_pos:
                            if(prmA < 0)
                                prmA = 0;
                            else if(prmA >= ipipe->noRows)
                            {
                                ipipeLogClampPos("row",prmA,ipipe->noRows-1);
                                prmA = ipipe->noRows - 1;
                            }
                            if(prmL < 0)
                                prmL = 0;
                            else if(prmL >= maxOff)
                            {
                                ipipeLogClampPos("col",prmL,maxOff-1);
                                prmL = maxOff - 1;
                            }
                            ipipeStoreInputPos();
                            /* The dot's line number is tracked a line at a time by the walks below
                             * rather than adjusted by (prmA - curRow) up front. Neither walk is
                             * guaranteed to travel that far - the buffer runs out going down, and
                             * the scrollback runs out going up - so assuming it leaves dotLineNo
                             * describing a line the dot is not on, and everything downstream that
                             * works in line numbers (the window scroll, the trim, the anchors)
                             * then disagrees with everything that follows the line pointers.
                             * ipipeStoreInputPos always leaves the dot on a real line, so both
                             * walks can step from it without a sentinel check first. */
                            lp_old = bp->dotLine;
                            if(prmA > curRow)
                            {
                                /* Stop before the sentinel and let the second loop extend the
                                 * buffer for the rows that do not exist yet. Walking onto it
                                 * instead would leave the dot on a line that is in no buffer,
                                 * invisible to anything that counts. */
                                while((curRow != prmA) && (meLineGetNext(lp_old) != bp->baseLine))
                                {
                                    curRow++;
                                    bp->dotLineNo++;
                                    lp_old = meLineGetNext(lp_old);
                                }
                                while(curRow != prmA)
                                {
                                    curRow++;
                                    bp->dotLineNo++;
                                    addLineToEob(bp,(meUByte *)"");
                                    lp_old = meLineGetPrev(bp->baseLine);
                                }
                            }
                            else
                            {
                                /* the list is circular, without the baseLine test an over-large
                                 * curRow walks through the sentinel to the end of the buffer */
                                while((curRow != prmA) && (meLineGetPrev(lp_old) != bp->baseLine))
                                {
                                    curRow--;
                                    bp->dotLineNo--;
                                    lp_old = meLineGetPrev(lp_old);
                                }
                                if(curRow != prmA)
                                    /* out of scrollback - curRow keeps the row actually reached */
                                    ipipeLogClamp(prmA,curRow);
                            }
                            len = prmL;
                            bp->dotLine = lp_old;
                            ipipeDecodeLine(ipipe,lp_old->text,buff,cbuff,0);
                            meBufferStoreLocation(lp_old,(meUShort)len,bp->dotLineNo);
                            prmL -= meStrlen(buff);
                            p1 = buff+len;
                            if(prmL > 0)
                            {
                                memset(p1-prmL,' ',prmL);
                                if(ipipe->flag & meIPIPE_ANSICOLOR)
                                    memset(cbuff+(p1-prmL-buff),'A',prmL);
                                *p1 = '\0';
                            }
                            noLines = 0;
                            break;

                        case 'h':
                            if(gotQ)
                            {
                                if(prmL == 7)
                                    ipipe->flag &= ~meIPIPE_NOAUTOWRAP;
                                else if(prmL == 25)
                                {
                                    meModeSet(bp->mode,MDCURSOR);
                                    if(bp == frameCur->windowCur->buffer)
                                        meCursorUpdate();
                                }
#ifndef NDEBUG
                                /* safe to ignore: cursor key mode (prmL = 1) & bracketed paste (2004), focus in/out (1004)
                                 * synchronized output (2026), unknown private mode (2031), win32-input-mode (9001 - set by ConPTY to request
                                 * key events are sent as '\E[<vk>;<sc>;<uc>;<kd>;<cs>;<rc>_' records, ME does not so it falls back to plain VT) */
                                else if((prmL != 1) && (prmL < 1000 || prmL > 1006) && (prmL != 2004) && (prmL != 2026) && (prmL != 2031) && (prmL != 9001))
                                    goto cant_handle_this;
#endif
                            }
                            else if(prmL == 4)
                                ipipe->flag |= meIPIPE_OVERWRITE;
#ifndef NDEBUG
                            else
                                goto cant_handle_this;
#endif
                            break;
                        case 'l':
                            if(gotQ)
                            {
                                if(prmL == 7)
                                    ipipe->flag |= meIPIPE_NOAUTOWRAP;
                                else if(prmL == 25)
                                {
                                    meModeClear(bp->mode,MDCURSOR);
                                    if(bp == frameCur->windowCur->buffer)
                                        meCursorUpdate();
                                }
#ifndef NDEBUG
                                /* safe to ignore: cursor key mode (prmL = 1) & bracketed paste (2004), focus in/out (1004)
                                 * synchronized output (2026), unknown private mode (2031), win32-input-mode (9001 - see the 'h' handler above) */
                                else if((prmL != 1) && (prmL < 1000 || prmL > 1006) && (prmL != 2004) && (prmL != 2026) && (prmL != 2031) && (prmL != 9001))
                                    goto cant_handle_this;
#endif
                            }
                            else if(prmL == 4)
                                ipipe->flag &= ~meIPIPE_OVERWRITE;
#ifndef NDEBUG
                            else
                                goto cant_handle_this;
#endif
                            break;
                        case 'm':
                            if(ipipe->flag & meIPIPE_ANSICOLOR)
                            {
                                meUByte newFg=ipipe->ansiFg, newBg=ipipe->ansiBg, newSt=ipipe->ansiSt;
                                int nn;
                                if(prmC < meIPIPE_PRM_MAX)
                                {
                                    prmB[prmC] = (meUByte) gotC;
                                    prmS[prmC++] = prmL;
                                }
                                for(ii=0 ; ii<prmC ; ii++)
                                {
                                    if((prmA = prmS[ii]) == 0)
                                    {
                                        newFg = 0;
                                        newBg = 0;
                                        newSt = 0;
                                    }
                                    else if(prmA < 30)
                                    {
                                        if(prmA < 5)
                                        {
                                            if(prmA > 2)
                                                /* 3 = italic, 4 = underline */
                                                newSt |= 1<<(prmA - 2);
                                            else if(prmA == 2)
                                                newSt |= meIPIPE_STY_DIM;
                                            else
                                                newSt |= meIPIPE_STY_BOLD;
                                        }
                                        else if(prmA < 22)
                                            ;
                                        else if(prmA == 22)
                                            /* 22 turns off both the bold & dim intensities */
                                            newSt &= ~(meIPIPE_STY_BOLD|meIPIPE_STY_DIM);
                                        else if(prmA < 25)
                                            newSt &= ~(1<<(prmA-22));
                                    }
                                    else if(prmA < 40)
                                    {
                                        if(prmA < 38)
                                            newFg = ipipeBaseToColor(prmA - 30);
                                        else if(prmA == 38)
                                        {
                                            /* extended fg colour, i.e. 256 palette or 24 bit rgb */
                                            if((nn = ipipeAnsiExtColor(prmS+ii+1,prmB+ii+1,prmC-ii-1,&newFg)) == 0)
                                                /* unknown form: abandon the rest of the sequence */
                                                break;
                                            ii += nn;
                                        }
                                        else
                                            newFg = 0;
                                    }
                                    else if(prmA < 50)
                                    {
                                        if(prmA < 48)
                                            newBg = ipipeBaseToColor(prmA - 40);
                                        else if(prmA == 48)
                                        {
                                            /* extended bg colour, i.e. 256 palette or 24 bit rgb */
                                            if((nn = ipipeAnsiExtColor(prmS+ii+1,prmB+ii+1,prmC-ii-1,&newBg)) == 0)
                                                /* unknown form: abandon the rest of the sequence */
                                                break;
                                            ii += nn;
                                        }
                                        else
                                            newBg = 0;
                                    }
                                    else if(prmA < 90)
                                        ;
                                    else if(prmA < 98)
                                        newFg = ipipeBaseToColor(prmA - 90 + 8);
                                    else if((prmA >= 100) && (prmA < 108))
                                        newBg = ipipeBaseToColor(prmA - 100 + 8);
                                }
                                ipipe->ansiFg = newFg;
                                ipipe->ansiBg = newBg;
                                ipipe->ansiSt = newSt;
                                ipipe->ansiCc = ipipeAnsiToScheme(newFg,newBg,newSt);
                            }
                            break;
                        case 'n':
                            {
                                char outb[32];

                                if(prmL != 6)
#ifndef NDEBUG
                                    goto cant_handle_this;
#else
                                    break;
#endif
                                sprintf(outb,"\033[%d;%dR",curRow+1,len+1);
#if (IPIPE_DEBUG >= 3)
                                ipipeLogWrite("DSR",(meUByte *)outb,(int)strlen(outb));
#endif
#ifdef _WIN32
                                { DWORD wr; WriteFile(ipipe->outWfd,outb,(DWORD)strlen(outb),&wr,NULL); }
#else
                                write(ipipe->outWfd,outb,strlen(outb));
#endif
                                break;
                            }
                        case 'J':
                            {
                                meLine *lp;
                                int jj;

                                lp = lp_old;
                                if(prmL == 1)
                                {
                                    ii = (*p1 == '\0') ? len:len+1;
                                    memset(buff,' ',ii);
                                    if(ipipe->flag & meIPIPE_ANSICOLOR)
                                        memset(cbuff,'A',ii);
                                    for(jj=curRow ; jj>0 ; jj--)
                                    {
                                        if((lp = meLineGetPrev(lp)) == bp->baseLine)
                                            break;
                                        if(ipipe->flag & meIPIPE_ANSICOLOR)
                                            ipipeClearColorLine(lp);
                                        else
                                        {
                                            memset(lp->text,' ',meLineGetLength(lp));
                                            lp->flag |= meLINE_CHANGED;
                                        }
                                    }
                                    break;
                                }
                                if(prmL == 3)
                                    /* erase scrollback. ME's buffer is the session log, losing
                                     * it would be worse than ignoring the request */
                                    break;
                                if(prmL == 2)
                                {
                                    for(ii=curRow ; ii>0 ; ii--)
                                        lp = meLineGetPrev(lp);
                                    ii = meStrlen(buff);
                                    memset(buff,' ',ii);
                                    if(ipipe->flag & meIPIPE_ANSICOLOR)
                                        memset(cbuff,'A',ii);
                                }
                                else if(prmL == 0)
                                {
                                    ii = meStrlen(buff+len);
                                    memset(buff+len,' ',ii);
                                    if(ipipe->flag & meIPIPE_ANSICOLOR)
                                        memset(cbuff+len,'A',ii);
                                    if(lp != bp->baseLine)
                                        lp = meLineGetNext(lp);
                                }
                                else
#ifndef NDEBUG
                                    goto cant_handle_this;
#else
                                    break;
#endif
                                while(lp != bp->baseLine)
                                {
                                    if(ipipe->flag & meIPIPE_ANSICOLOR)
                                        ipipeClearColorLine(lp);
                                    else
                                    {
                                        memset(lp->text,' ',meLineGetLength(lp));
                                        lp->flag |= meLINE_CHANGED;
                                    }
                                    lp = meLineGetNext(lp);
                                }
                                break;
                            }
                        case 'K':
                            if(!gotN || (prmL == 0))
                                *p1 = '\0';
                            else if(prmL <= 2)
                            {
                                /* 1 erases up to and including the cursor, 2 the whole line;
                                 * both blank rather than truncate so the cursor column survives */
                                ii = (prmL == 1) ? ((*p1 == '\0') ? len:len+1):len;
                                memset(buff,' ',ii);
                                if(ipipe->flag & meIPIPE_ANSICOLOR)
                                    memset(cbuff,'A',ii);
                                if(prmL == 2)
                                    *p1 = '\0';
                            }
                            break;
                        case 'P':
                            {
                                /* DCH - delete prmL chars and shifts the remainder left. */
                                int ll;
                                if(!gotN)
                                    prmL = 1;
                                else if(prmL <= 0)
                                    break;
                                if((ll = meStrlen(p1) - prmL) <= 0)
                                    *p1 = '\0';
                                else
                                {
                                    memmove(p1,p1+prmL,ll+1);
                                    if(ipipe->flag & meIPIPE_ANSICOLOR)
                                        memmove(cbuff+len,cbuff+len+prmL,ll);
                                }
                                break;
                            }
                        case 'X':
                            {
                                /* ECH - erase prmL chars with no shift. */
                                int ll;
                                if(!gotN)
                                    prmL = 1;
                                else if(prmL <= 0)
                                    break;
                                if((ll = meStrlen(p1)) <= prmL)
                                    *p1 = '\0';
                                else
                                {
                                    memset(p1,' ',prmL);
                                    if(ipipe->flag & meIPIPE_ANSICOLOR)
                                        memset(cbuff+len,(char)ipipe->ansiCc,prmL);
                                }
                                break;
                            }
                        case 'c': /* Ignore Device Attributes */
                        case 'q': /* Ignore DECLL (DEC Load LEDs) */
                        case 'r': /* Ignore DECSTBM � scrolling region */
                        case 'u': /* Ignore Kitty keyboard protocol */
                            break;

                        case 't':
                            {
                                /* XTWINOPS - window manipulation, the first parameter is the operation.
                                 * All requests are ignored, in particular the resize '\E[8;<rows>;<cols>t'
                                 * which ConPTY sends whenever the child changes the console size - ME owns
                                 * the ipipe size (see ipipeSetSize) so honouring it would create a resize
                                 * loop. Only the text area size report (18) is answered. */
                                char outb[24];

                                if((prmC ? prmS[0]:prmL) == 8)
                                    /* the child's own size report - what the far end believes the
                                     * terminal to be, against the ipipe->noRows/noCols we set it to */
                                    ipipeLogSize(prmC,prmS,prmL);
                                if((prmC ? prmS[0]:prmL) != 18)
                                    break;
                                sprintf(outb,"\033[8;%d;%dt",ipipe->noRows,ipipe->noCols);
#if (IPIPE_DEBUG >= 3)
                                ipipeLogWrite("XTWINOPS",(meUByte *)outb,(int)strlen(outb));
#endif
#ifdef _WIN32
                                { DWORD wr; WriteFile(ipipe->outWfd,outb,(DWORD)strlen(outb),&wr,NULL); }
#else
                                write(ipipe->outWfd,outb,strlen(outb));
#endif
                                break;
                            }

                        default:
#ifndef NDEBUG
cant_handle_this:
                            if(prmC)
                                printf("Don't cope with term code \\E[%s%d;%d%c\n",(gotQ) ? "?":"",prmS[0],prmL,cc);
                            else
                                printf("Don't cope with term code \\E[%s%d%c\n",(gotQ) ? "?":"",prmL,cc);
#endif
                            break;
                        }
                    }
                    else
                        /* the rest of the CSI never arrived inside readFromPipe's timeout, so the
                         * sequence was split across ipipeRead calls and is dropped, parameters and
                         * all - there is no parser state carried between calls to resume from */
                        ipipeLogDrop("CSI");
                    break;
                }
                else if(cc == '7')
                {
                    ipipe->strRow = curRow;
                    ipipe->strCol = len;
                    break;
                }
                else if(cc == '8')
                {
                    prmA = ipipe->strRow;
                    prmL = ipipe->strCol;
                    goto move_cursor_pos;
                }
                else if(cc == ']')
                {
osc_consume:
                    /* OSC: consume until BEL (0x07) or ST (ESC \). The payload is unbounded
                     * so this is the one sequence worth resuming across reads, see oscSplit. */
                    for(;;)
                    {
                        if(!ipipeGetNextChar(ipipe,cc,rbuff,curROff,curRRead,1))
                        {
                            /* Only the first bail records the time; refreshing it here would let
                             * a continuously writing child hold the resume open for ever. */
                            if(ipipe->oscSplit == 0)
                            {
                                ipipe->oscSplit = ipipeTimeNow();
                                ipipeLogDrop("OSC");
                            }
                            break;
                        }
                        if(cc == 7)
                        {
                            ipipe->oscSplit = 0;
                            break;
                        }
                        if(cc == 27)
                        {
                            /* ST is ESC \ and the two are always written together, so if the
                             * backslash is late it simply appears as text */
                            ipipe->oscSplit = 0;
                            ipipeGetNextChar(ipipe,cc,rbuff,curROff,curRRead,1);
                            break;
                        }
                    }
                    break;
                }
                else if(cc == '(' || cc == ')')
                {
                    /* character set designation: consume the single designator byte and ignore all */
                    ipipeGetNextChar(ipipe,cc,rbuff,curROff,curRRead,1);
                    break;
                }
            }
            /* fall through */
        default:
            if(cc < 0x20)
            {
                /* ignore remaining C0 chars: 0x01-0x06, 0x0E-0x1A, 0x1C-0x1F. A lone ESC would drop here as well */
                if(cc == 0x1b)
                    ipipeLogDrop("ESC");
                break;
            }
#if MEOPT_EXTENDED
            if(!(ipipe->flag & meIPIPE_NOUTF8) && (cc >= 0x80))
            {
#if (IPIPE_DEBUG >= 2)
                meUByte c1=cc;
#endif
                meUByte c2, c3;
                if((cc < 0xc0) || !ipipeGetNextChar(ipipe,c2,rbuff,curROff,curRRead,1) || ((c2 & 0xc0) != 0x80))
                    /* orphan continuation byte - discard */
                    break;
                if(cc < 0xe0)
                    /* 2-byte sequence (0xc0-0xdf) */
                    cc = utf8ToMeChar((((meUInt)(cc & 0x1f)) << 6) | (c2 & 0x3f));
                else if(!ipipeGetNextChar(ipipe,c3,rbuff,curROff,curRRead,1) || ((c3 & 0xc0) != 0x80))
                    break;
                else if(cc < 0xf0)
                    /* 3-byte sequence */
                    cc = utf8ToMeChar((((meUInt) (cc & 0x0f)) << 12) | (((meUInt) (c2 & 0x3f)) << 6) | (c3 & 0x3f));
                else if(!ipipeGetNextChar(ipipe,c2,rbuff,curROff,curRRead,1) || ((c2 & 0xc0) != 0x80))
                    break;
                else
                    /* 4-byte: ME supports only up to U+FFFF - consume */
                    cc = meCHAR_UNDEF;
#if (IPIPE_DEBUG >= 2)
                if((cc == meCHAR_UNDEF) && (logFp != NULL))
                {
                    /* Put an easy to spot marker into the log with U+FFFF code */
                    fprintf(logFp,":ZZUZ:%04x:",((c1 < 0xe0) ? ((((meUInt)(c1 & 0x1f)) << 6) | (c2 & 0x3f)):((c1 < 0xf0) ? ((((meUInt) (c1 & 0x0f)) << 12) | (((meUInt) (c2 & 0x3f)) << 6) | (c3 & 0x3f)):-1)));
                    fflush(logFp);
                }
#endif
                /* Should unrepresentable (cc == meCHAR_UNDEF) be discarded? */
            }
#endif
            if(len >= maxOff)
            {
                if(ipipe->flag & meIPIPE_NOAUTOWRAP)
                {
                    /* stay at right margin - back up and overwrite last column */
                    p1--;
                    len--;
                }
#ifdef _WIN32
                else if((scrollWrapCUP == 2) && (meLineGetLength(meLineGetNext(lp_old)) == 0))
                {
                    /* ConPTY's bottom-of-viewport scroll-resync idiom was confirmed (see init & use scrollWrapCUP),
                     * the blank line for this row already exists via \n insertion, so simple reuse */
                    lp_old = meLineGetNext(lp_old);
                    if(curRow < ipipe->noRows-1)
                        curRow++;
                    /* the buffer dot follows the line being built, see the wrap below */
                    bp->dotLine = lp_old;
                    bp->dotLineNo++;
                    bp->dotOffset = 0;
                    p1 = buff;
                    *p1 = '\0';
                    len = 0;
                }
#endif
                else
                {
                    meUByte bb[2];
                    int splitIdx = (int)(p1-buff);
                    bb[0] = p1[0];
                    bb[1] = p1[1];
                    if(ipipe->flag & meIPIPE_ANSICOLOR)
                        cbuff[splitIdx] = ipipe->ansiCc;
                    p1[0] = windowChars[WCDISPSPLTLN];
                    p1[1] = '\0';
                    ii = ipipeAddLine(ipipe,lp_old,buff,cbuff,NULL);
                    noLines += ii;
                    if(lp_old != bp->baseLine)
                    {
                        /* The wrap just produced the row lp_old used to occupy, so retire lp_old
                         * exactly as ipipeStoreInputPos does for a real commit, and advance lp_old
                         * to the line after it. Otherwise this old line is never spliced out by
                         * anything (the wrap doesn't move lp_old, and the commit that eventually
                         * follows only retires whatever lp_old still points to) and is left
                         * stranded in the buffer, resurfacing later as stale content one row after
                         * where it should have been. */
                        meLine *lp_next = meLineGetNext(lp_old);
                        meLine *lp_new = meLineGetPrev(lp_old);
                        noLines--;
                        lp_new->next = lp_old->next;
                        lp_old->next->prev = lp_new;
                        meLineSwap(bp,lp_old,lp_new);
                        meFree(lp_old);
                        lp_old = lp_next;
                    }
                    /* bp->dotLine is the line being built, so it has to move with lp_old - the
                     * splice above frees the line it was pointing at, which left it dangling for
                     * every redraw until the next commit reset it. ipipeStoreInputPos does the
                     * same retire and then sets bp->dotLine for exactly this reason. The line
                     * number moves with it: the wrap adds and retires one line so the buffer is
                     * no longer than it was, but the row being written is one further down. */
                    bp->dotLine = lp_old;
                    bp->dotLineNo++;
                    bp->dotOffset = 0;
                    if(curRow < ipipe->noRows-1)
                        curRow += ii;
                    p1[0] = bb[0];
                    p1[1] = bb[1];
                    meStrcpy(buff,p1);
                    if(ipipe->flag & meIPIPE_ANSICOLOR)
                        memmove(cbuff,cbuff+splitIdx,meBUF_SIZE_MAX-splitIdx);
                    p1 = buff;
                    len = 0;
                }
            }
#ifdef _WIN32
            scrollWrapCUP = 0;
#endif
            if(ipipe->flag & meIPIPE_OVERWRITE)
            {
                int ll = meStrlen(p1)+1;
                memmove(p1+1,p1,ll);
                if(ipipe->flag & meIPIPE_ANSICOLOR)
                {
                    int p1i = (int)(p1-buff);
                    memmove(cbuff+p1i+1,cbuff+p1i,ll);
                }
            }
            else if(*p1 == '\0')
                p1[1] = '\0';
            if(ipipe->flag & meIPIPE_ANSICOLOR)
                cbuff[p1-buff] = ipipe->ansiCc;
            *p1++ = cc;
            len++;
        }
    }
    ipipeStoreInputPos();
    ipipeCheckInvar(ipipe,"read");
    ipipeCheckWins("end");
#ifdef _UNIX
#if MEOPT_CLIENTSERVER
    /* the unix client server trashed the rfd at the top of this function due
     * to the way sockets are handled. But the read handle is the same as the write
     * so its trivial to restore */
    if(ipipe->pid == 0)
        ipipe->rfd = ipipe->outWfd;
#endif
#endif
    
    if((ii=ipipe->pid) < 0)
    {
        curRow = ipipe->exitCode;
        ipipeRemove(ipipe);
    }
#ifdef _WIN32
    else if(ipipe->thread != NULL)
        /* get the thread going again */
        SetEvent(ipipe->threadContinue);
#endif

#if ((defined (_UNIX)) && (defined (_POSIX_SIGNALS)))
    /* as soon as the BLOCK of sigchld is removed, if the process has finished
     * while reading then it will get registered now, this fact has to be taken
     * into acount and handled carefully. If not typically the [EXIT] line
     * will be missed if signals still blocked during the execution of the ipipe
     * function which leads to side effects like poor cursor blink etc.
     */
    meSigRelease();
#endif
    
    meAnchorSet(bp,'I',bp->dotLine,bp->dotLineNo,bp->dotOffset,1);
    if(bp->ipipeFunc >= 0)
    {
        /* If the process has ended the argument will be 0 with $result set to the exitCode, else 1 */
        if(ii < 0)
        {
            meStrcpy(rbuff,resultStr);
            sprintf((char *)resultStr,"%d",curRow);
        }
        execBufferFunc(bp,bp->ipipeFunc,(meEBF_ARG_GIVEN|meEBF_USE_B_DOT|meEBF_HIDDEN),(ii >= 0));
        ipipeCheckWins("hook");
        if(ii < 0)
            meStrcpy(resultStr,rbuff);
    }
    else if ((ii < 0) && (bp->intFlag & BIFLOCK))
    {
        meWindow *wp;
        
        /* The pipe has ended and is not under manual control, a BIFLOCK
         * exists which indidates that one or more windows are tied to the
         * buffer cursor position. For each window that is locked with the
         * buffer then re-center the window so that the last line is at the
         * bottom of the window. Work out which windows are locked to the
         * current buffer position and re-center them. */
        meFrameLoopBegin();
        wp = loopFrame->windowList;
        while(wp != NULL)
        {
            /* If the window position matches the buffer then re-center */
            if((wp->buffer == bp) &&
               (wp->dotLine == bp->dotLine) &&
               (wp->dotOffset == bp->dotOffset))
            {
                /* Force a bottom window recenter */
                wp->windowRecenter = -1;
                wp->updateFlags |= WFFORCE;
            }
            wp = wp->next;
        }
        meFrameLoopEnd();
    }
    ipipeCheckWins("update");
    update(meFALSE);
}

int
ipipeWrite(int f, int n)
{
    meUByte buff[meBUF_SIZE_MAX];	/* string to add */
    meBuffer *cbp=frameCur->windowCur->buffer;
    meIPipe *ipipe;
    int ss;

    if(!meModeTest(cbp->mode,MDPIPE))
        return mlwrite(MWABORT,(meUByte *)"[Not an ipipe-buffer]");
    /* ask for string to insert */
    if((ss=meGetString((meUByte *)"String", 0, 0, buff, meBUF_SIZE_MAX)) <= 0)
        return ss;
    
    ipipe = ipipes;
    while(ipipe->bp != cbp)
        ipipe = ipipe->next;
    ipipeWriteString(ipipe,n,buff);

    return meTRUE;
}

/* DELETING OFF SCREEN CONTENT - discards buffer lines below the cursor when the window
 * shrinks. The child has the new size and will never address those rows again, and leaving
 * them strands them for good (ipipeAddLine only inserts *before* lp_old). If that loss ever
 * matters, the alternative is to keep them and push ipipe->curRow down instead, at the cost
 * of ME and the child disagreeing about which buffer line is which screen row. */
#ifdef IPIPE_DEBUG
/* The position of any line in the buffer. The baseLine is the cursor's "one past the last line"
 * position and reports as lineCount, so -1 means the line is in no buffer at all - ie stale. */
static int
ipipeLineNo(meBuffer *bp, meLine *lp)
{
    meLine *ll;
    int nn=0;

    if(lp == bp->baseLine)
        return (int) bp->lineCount;
    for(ll = meLineGetNext(bp->baseLine) ; ll != bp->baseLine ; ll = meLineGetNext(ll))
    {
        if(ll == lp)
            return nn;
        nn++;
    }
    return -1;
}

/* Every window showing bp must hold lines bp still owns - updateWindow walks the display from
 * wp->dotLine, so a window left on a retired line turns the next redraw into a walk through
 * freed memory. Reports the first offender and returns non-zero, so the caller can name what it
 * was doing at the time. */
static int
ipipeCheckWindows(meBuffer *bp, const char *where)
{
    meWindow *wp;
    int bad=0;

    meFrameLoopBegin();
    wp = loopFrame->windowList;
    while(wp != NULL)
    {
        if((wp->buffer == bp) && (wp->dotLine != bp->baseLine) &&
           (ipipeLineNo(bp,wp->dotLine) < 0))
        {
            meIPipeLog(":WIN-STALE:%s:wid=%d dotLineNo=%d vertScroll=%d curWinBuf=%s cnt=%d:",
                       where,(int)wp->id,(int)wp->dotLineNo,(int)wp->vertScroll,
                       (frameCur->windowCur->buffer != NULL) ?
                       (char *)frameCur->windowCur->buffer->name:"?",(int)bp->lineCount);
            bad = 1;
            break;
        }
        wp = wp->next;
    }
    meFrameLoopBreak(bad);
    meFrameLoopEnd();
    return bad;
}

/* walk the list rather than trust bp->lineCount / bp->dotLineNo. *dotNo follows ipipeLineNo -
 * the dot on the baseLine is one past the last line, only a stale dot reports -1. */
static int
ipipeWalk(meBuffer *bp, int *dotNo)
{
    meLine *lp;
    int nn=0;

    *dotNo = -1;
    for(lp = meLineGetNext(bp->baseLine) ; lp != bp->baseLine ; lp = meLineGetNext(lp))
    {
        if(lp == bp->dotLine)
            *dotNo = nn;
        nn++;
    }
    if(bp->dotLine == bp->baseLine)
        *dotNo = nn;
    return nn;
}
#endif


static void
ipipeTrimTail(meIPipe *ipipe)
{
    meBuffer *bp=ipipe->bp;
    meWindow *wp;
    meLine   *lp, *lpp;
    meInt     nn, last;

    /* the rows the screen has below the cursor, against the lines the buffer has below it */
    nn = (bp->lineCount - 1 - bp->dotLineNo) - (ipipe->noRows - 1 - ipipe->curRow);
#if (IPIPE_DEBUG >= 3)
    if(logFp != NULL)
    {
        int wDot, wCnt = ipipeWalk(bp,&wDot);
        fprintf(logFp,":TRIM:nn=%d curRow=%d noRows=%d dot=%d/%d cnt=%d/%d want=%d below=%d:",
                (int)nn,(int)ipipe->curRow,(int)ipipe->noRows,wDot,(int)bp->dotLineNo,
                wCnt,(int)bp->lineCount,
                (int)(ipipe->noRows-1-ipipe->curRow),(int)(bp->lineCount-1-bp->dotLineNo));
        fflush(logFp);
    }
#endif
    if(nn <= 0)
        return;
    /* bring any window sitting on a line that is about to go back to the cursor line */
    last = bp->lineCount - 1 - nn;
    meFrameLoopBegin();
    wp = loopFrame->windowList;
    while(wp != NULL)
    {
        if((wp->buffer == bp) && (wp->dotLineNo > last))
        {
            wp->dotLine = bp->dotLine;
            wp->dotLineNo = bp->dotLineNo;
            wp->dotOffset = 0;
            wp->updateFlags |= WFMOVEL|WFMAIN;
        }
        wp = wp->next;
    }
    meFrameLoopBreak(0);
    meFrameLoopEnd();

    while(nn-- > 0)
    {
        lp = meLineGetPrev(bp->baseLine);
        if((lp == bp->baseLine) || (lp == bp->dotLine))
            break;
        lpp = meLineGetPrev(lp);
        lpp->next = lp->next;
        lp->next->prev = lpp;
        /* the window loop above has already moved any window that was showing a line this loop
         * can reach, this is the safety net for anything that still references it */
        meLineSwap(bp,lp,lpp);
        meFree(lp);
        bp->lineCount--;
    }
#if (IPIPE_DEBUG >= 3)
    if(logFp != NULL)
    {
        int wDot, wCnt = ipipeWalk(bp,&wDot);
        fprintf(logFp,":TRIMMED:dot=%d/%d cnt=%d/%d noRows=%d curRow=%d:",
                wDot,(int)bp->dotLineNo,wCnt,(int)bp->lineCount,
                (int)ipipe->noRows,(int)ipipe->curRow);
        fflush(logFp);
    }
#endif
}


void
ipipeSetSize(meWindow *wp, meBuffer *bp)
{
    meIPipe *ipipe;
    meShort noRows, noCols;
    int ii;

    ipipe = ipipes;
    while((ipipe != NULL) && (ipipe->bp != bp))
        ipipe = ipipe->next;
    if(ipipe == NULL)
        return;
    /* Cannot assume bp's dot is at the terminal's cursor position, any terminal resize will lead
     * the terminal process to restructure & draw relative to the current Terminal cursor position
     * so we must go back to the right location to figure out the correct resize action.
     * 
     * This can lose content if the terminal size is reduced and lines goes below the bottom of the
     * window, so care is needed to get the starting position and behaviour right, it must match a
     * terminal, otherwise there will be debris in the buffer that could throw off future input.
     */
    meAnchorSet(bp,meANCHOR_IPIPE_DOT,bp->dotLine,bp->dotLineNo,bp->dotOffset,1);
    meAnchorGet(bp,'I');
    /* Size to the given window alone. The callers only pass the current window or the sole
     * window showing the buffer, so there is nobody to fight with. Taking the largest of all
     * windows, as this used to, hands the child more rows than a smaller window can display and
     * the excess sits below that window for good. */
    noRows = wp->textDepth;
    noCols = wp->textWidth;
    if(bp->ipipeFlags & meBUFFER_IPIPE_WRAP)
        noCols = noCols-1;
    else if((noCols = ipipe->noCols) == 0)
    {
        meUByte *ss;
        if(((ss=getUsrVar((meUByte *)"ipipe-width")) == NULL) || ((noCols=((meShort) meAtoi(ss))) <= 0) || (noCols > meBUF_SIZE_MAX - 2))
            noCols = meBUF_SIZE_MAX - 2;
    }
#if (IPIPE_DEBUG >= 3)
    if(logFp != NULL)
    {
        fprintf(logFp,":IPIPE-SIZE:%s:%d %d -> %d %d (%d %d):",bp->name,ipipe->noRows,ipipe->noCols,noRows,noCols,meModeTest(bp->mode,MDWRAP),wp->textWidth);
        fflush(logFp);
    }
#endif
    if((ipipe->noRows != noRows) || (ipipe->noCols != noCols))
    {
        ii = ((int) noRows) - ((int) ipipe->noRows);
        ipipe->noRows = noRows;
        ipipe->noCols = noCols;
        
        if(ipipe->pid > 0)
        {
            /* the screen model, the buffer trim and the child's winsize are all PTY only. With a
             * plain pipe the buffer is an append-only log, not a screen, and there is no terminal
             * to tell - only the vertScroll below still applies. */
            if(ipipe->flag & meIPIPE_USEPTY)
            {
                if(ii > 0)
                {
#ifdef _WIN32
                    /* ConPTY anchors the top of the viewport when the window grows, its post-resize
                     * full-screen repaint redraws the same top line unchanged and simply adds blank
                     * rows at the bottom, rather than reflowing/revealing more scrollback above the
                     * cursor as a Unix terminal would. So curRow must be left as-is here; advancing it
                     * makes the subsequent ESC[H home from the repaint walk too far back into
                     * scrollback. */
#else
                    /* growing reveals scrollback above the cursor, but only as much as exists */
                    int jj;
                    if(ii > (jj = bp->dotLineNo - ipipe->curRow))
                        ii = jj;
                    if((ii > 0) && ((ipipe->curRow += ii) >= noRows))
                        ipipe->curRow = noRows-1;
#endif
                }
                else if(ipipe->curRow >= ipipe->noRows)
                    ipipe->curRow = ipipe->noRows-1;
                ipipeTrimTail(ipipe);
            }
            /* Check the window is displaying this buffer before messing with the window settings */
            if((wp->buffer == bp) && meModeTest(bp->mode,MDLOCK))
            {
                if((bp->lineCount <= wp->textDepth) || (bp->dotLineNo < ipipe->curRow))
                    wp->vertScroll = 0;
                else
                    wp->vertScroll = bp->dotLineNo-ipipe->curRow;
                wp->updateFlags |= WFMOVEL;
            }
            if(ipipe->flag & meIPIPE_USEPTY)
            {
#ifdef _UNIX
#if ((defined TIOCSWINSZ) || (defined TIOCGWINSZ))
                {
                    /* BSD-style.  */
                    struct winsize size;

                    size.ws_col = ipipe->noCols;
                    size.ws_row = ipipe->noRows;
                    size.ws_xpixel = size.ws_ypixel = 0;
#ifdef TIOCSWINSZ
                    ioctl(ipipe->outWfd,TIOCSWINSZ,&size);
#else
                    ioctl(ipipe->outWfd,TIOCGWINSZ,&size);
#endif
                    kill(ipipe->pid,SIGWINCH);
                }
#else
#ifdef TIOCGSIZE
                {
                    /* SunOS - style.  */
                    struct ttysize size;

                    size.ts_col = ipipe->noCols;
                    size.ts_lines = ipipe->noRows;
                    ioctl(ipipe->outWfd,TIOCSSIZE,&size);
                    kill(ipipe->pid,SIGWINCH);
                }
#endif /* TIOCGSIZE */
#endif /* TIOCSWINSZ/TIOCGWINSZ */
#endif /* _UNIX */
#ifdef _WIN32
                meIPipeConPTYResize(ipipe,ipipe->noCols,ipipe->noRows);
#endif /* _WIN32 */
            }
        }
    }
    /* put the editing position back where it was, see meANCHOR_IPIPE_DOT above. */
    meAnchorGet(bp,meANCHOR_IPIPE_DOT);
    meAnchorDelete(bp,meANCHOR_IPIPE_DOT);
}

#ifdef _UNIX

#ifdef _PTY_MASTER
/* In POSIX.1 standard open /dev/ptmx to get a new PTY, could use the posix_openpt function instead */
/* documentation for "Pseudo-TTY Drivers" - ptm(7) and pts(7) */
int grantpt(int fd);
int unlockpt(int fd);
char *ptsname(int fd);

static int
allocatePty(meUByte *ptyName)
{
    int fd;
    char *ss;
    if(((fd = open("/dev/ptmx", O_RDWR)) >= 0) &&  /* open master */
       (grantpt(fd) >= 0) &&                       /* change permission of slave */
       (unlockpt(fd) >= 0) &&                      /* unlock slave */
       ((ss=ptsname(fd)) != NULL))                 /* Get slave full path name */        
    {
        meStrcpy(ptyName,ss);
/*        printf("PTY success [%s]\n",ptyName);*/
        return fd;
    }
    return -1;
}
#else
/* allocatePty; Allocate a pty. We use the old BSD method of searching for a
 * pty, once we have aquired one then we look for the tty. Return the name of
 * the tty to the caller so that it may be opened. */
static int
allocatePty(meUByte *ptyName)
{
    int fd;
#ifdef _IRIX
    struct stat stb;
    /* struct sigaction ocstat, cstat;*/
    char * name;
    /* sigemptyset(&cstat.sa_mask);*/
    /* cstat.sa_handler = SIG_DFL;*/
    /* cstat.sa_flags = 0;*/
    /* sigaction(SIGCLD, &cstat, &ocstat);*/
    name = _getpty(&fd,O_RDWR,0600,0);
    /* sigaction(SIGCLD, &ocstat, (struct sigaction *)0);*/
    if(name == NULL)
        return -1;
    if((fd >=  0) && (fstat (fd, &stb) >= 0))
    {
        /* Return the name of the tty and the file descriptor of the pty */
        meStrcpy (ptyName, name);
        return fd;
    }
#else
    register int c, ii;
    struct stat stb;
    /* Some systems name their pseudoterminals so that there are gaps in
       the usual sequence - for example, on HP9000/S700 systems, there
       are no pseudoterminals with names ending in 'f'.  So we wait for
       three failures in a row before deciding that we've reached the
       end of the ptys.  */
    int failed_count=0;
    for (c ='p' ; c <= 'z' ; c++)
    {
        for (ii=0 ; ii<16 ; ii++)
        {
#ifdef _HPUX
            sprintf((char *)ptyName,"/dev/ptym/pty%c%x",c,ii);
#else
            sprintf((char *)ptyName,"/dev/pty%c%x",c,ii);
#endif
            if(stat((char *)ptyName,&stb) < 0)
            {
                /* Cannot open PTY */
/*                printf("PTY not exist [%s]\n",ptyName);*/
                failed_count++;
                if (failed_count >= 3)
                    return -1;
            }
            else
            {
                /* Found a potential pty */
                failed_count = 0;
                fd = open((char *)ptyName,O_RDWR,0);
                if(fd >= 0)
                {
                    /* check to make certain that both sides are available
                       this avoids a nasty yet stupid bug in rlogins */
#ifdef _HPUX
                    sprintf((char *)ptyName,"/dev/pty/tty%c%x",c,ii);
#else
                    sprintf((char *)ptyName,"/dev/tty%c%x",c,ii);
#endif
                    /* If we can read and write the tty then it is not in use. */
                    if(access((char *)ptyName,W_OK|R_OK) != 0)
                    {
                        /* tty in use, close down the pty and try the next one */
                        close(fd);
                        continue;
                    }
/*                    printf("PTY success [%s]\n",ptyName);*/
                    return fd;
                }
                else
                {
/*                    printf("PTY open failed [%s] %d\n",ptyName,errno);*/
                }
            }
        }
    }
#endif /* _IRIX */
    return -1;
}
#endif /* _PTY_MASTER */


/* childSetupTty; Restore the correct terminal settings on the child tty
 * process. We restore the settings from our initial save of the environmet. */
static void
childSetupTty(void)
{
#ifdef _USG
#ifdef _TERMIOS
    extern struct termios otermio;
    struct termios ntermio;
#else
    extern struct termio otermio;
    struct termio ntermio;
#endif
    ntermio = otermio;

    /* the new ipipe stuff needs the echo to do its terminal output */
    ntermio.c_lflag |= (ECHO|ECHOE|ECHOK);

#ifdef _TERMIOS
    tcsetattr (0, TCSAFLUSH, &ntermio);
#else
    ioctl(0, TCSETA, &ntermio); /* and activate them */
#endif /* _TERMIOS */
#endif /* _USG */

#ifdef _BSD
    extern struct sgttyb  osgttyb;      /* Old tty context */
    extern struct tchars  otchars;      /* Old terminal context */
    extern struct ltchars oltchars;     /* Old line discipline context */

    /* Restore the terminal settings */
    stty (0, &osgttyb);
#ifdef _BSD_CBREAK
    ioctl (0, TIOCSETC, &otchars);
#endif
    ioctl (0, TIOCSLTC, &oltchars);
#endif /* _BSD */
}
#endif

static int
doIpipeCommand(meUByte *comStr, meUByte *path, meUByte *bufName, int ipipeFunc, int flags)
{
    meIPipe  *ipipe;
    meBuffer *bp;
    meUByte   line[meBUF_SIZE_MAX];
    int       cd;
#ifdef _UNIX
    meUByte  *term;
    int       fds[2], outFds[2], ptyFp;
    int       pid;                   /* Child process identity */
#endif
#ifdef _WIN32
    int       rr;
#endif
    /* get or create the command buffer */
    if(((bp=bfind(bufName,0)) != NULL) && meModeTest(bp->mode,MDPIPE))
    {
        cd = (int) meStrlen(bufName);
        if(cd > (meBUF_SIZE_MAX-22))
            cd = meBUF_SIZE_MAX-22;
        memcpy(line,bufName,cd);
        meStrcpy(line+cd," already active, kill");
        if(mlyesno(line) <= 0)
            return meFALSE;
    }
    if((ipipe = meMalloc(sizeof(meIPipe))) == NULL)
        return meFALSE;
    cd = (meStrcmp(path,curdir) && (meChdir(path) != -1));

#ifdef _WIN32
    /* Launch the ipipe */
    if((rr=WinLaunchProgram(comStr,(LAUNCH_IPIPE|flags), NULL, NULL, ipipe, NULL)) <= 0)
    {
        if(cd)
            meChdir(curdir);
        free(ipipe);
        if(rr == meABORT)
            /* returns meABORT when trying to IPIPE a DOS app on win95 (it
             * doesn't work) Try doPipe instead and maintain the same
             * environment as the macros may rely on callbacks etc. */
            return doPipeCommand(comStr,path,bufName,ipipeFunc,(flags&~LAUNCH_TO_VAR),NULL);
        return meFALSE;
    }
    if(ipipe->hPCon == NULL)
        flags &= ~LAUNCH_USEPTY;
    else
        flags |= LAUNCH_USEPTY;
#else

    if((term=getUsrVar((meUByte *) ((flags & LAUNCH_ANSICOLOR) ? "ipipe-color-term":"ipipe-term"))) == errorm)
    {
        /* if tput exits with code 0 then the term exists, otherwise it most likely doesn't */
        if(ipipeTermSys == NULL)
            ipipeTermSys = (meUByte *) ((system("tput -T vt100-nam longname > /dev/null 2>&1")) ? "TERM=vt100":"TERM=vt100-nam");
        if(flags & LAUNCH_ANSICOLOR)
        {
            /* Note: ansi has am without xenl, so a terminfo-aware child gives up its last row to avoid auto-wrap issues */
            if(ipipeTermCol == NULL)
                ipipeTermCol = (system("tput -T ansi longname > /dev/null 2>&1")) ? ipipeTermSys:(meUByte *) "TERM=ansi";
            term = ipipeTermCol;
        }
        else
            term = ipipeTermSys;
    }
    
    /* Allocate a pseudo terminal to do the work */
#ifdef _CYGWIN
    /* Cygwin has to use windows ConPTY which aren't great, favour non-PTY unless explicitly told to (like Windows) */
    if((flags & LAUNCH_USEPTY) && ((ptyFp=allocatePty(line)) >= 0))
#else
    if(((flags & LAUNCH_NOPTY) == 0) && ((ptyFp=allocatePty(line)) >= 0))
#endif
    {
        flags |= LAUNCH_USEPTY;
        fds[0] = outFds[1] = ptyFp;
#if ((defined _LINUX_BASE) || (defined _FREEBSD_BASE) || (defined _SUNOS) || (defined _BSD))
        /* On the BSD systems we open the tty prior to the fork. If this is a
         * later POSIX platform then we will expect O_NOCTTY to exist and we
         * open the tty with O_NOCTTY. Do not let this terminal become our
         * controlling tty. This prevents an application from unintentionally
         * aquiring the controlling terminal as a side effect of the open. */
#if (defined O_NOCTTY)
        fds[1] = outFds[0] = open((char *) line,O_RDWR|O_NOCTTY,0);
#else
        fds[1] = outFds[0] = open((char *) line,O_RDWR,0);
#endif /* O_NOCTTY */
#else
        fds[1] = outFds[0] = -1;
#endif /* _LINUX/_FREEBSD/.. */
    }
    else
    {
        /* Could not get a pty use a pipe instead */
        flags &= ~LAUNCH_USEPTY;
        pipe(fds);
        pipe(outFds);
    }

    /* The master end of the pty must be non-blocking. Under POSIX we can
     * apply these on the open, BSD systems then it is not possible to apply
     * these on the open. For ease of code maintainance then we apply these
     * afterwards usig fnctl.
     *
     * POSIX calls for O_NONBLOCK. BSD systems and earlier UNIX systems are
     * O_NDELAY. Pick whichever is defined - they are typically defined to the
     * same thing if both exist
     *
     * Note that these settings apply irrespective of whether we are dealing
     * with a PTY or PIPE */
#ifdef O_NONBLOCK
    if(fds[0] > 0)
        fcntl(fds[0],F_SETFL,O_NONBLOCK);
    if((fds[1] > 0) && ((flags & LAUNCH_USEPTY) == 0))
        fcntl(fds[1],F_SETFL,O_NONBLOCK);
#else
#ifdef O_NDELAY
    if (fds[0] > 0)
        fcntl(fds[0],F_SETFL,O_NDELAY);
    if ((fds[1] > 0) && ((flags & LAUNCH_USEPTY) == 0))
        fcntl(fds[1],F_SETFL,O_NDELAY);
#endif /* O_NDELAY */
#endif /* O_NONBLOCK */

    /* Hold up the child and alarm signals */
    meSigHold ();

    /* Create the new child process */
    if((pid=meFork()) == 0)
    {
        /******************************************************************
        * CHILD CHILD CHILD CHILD CHILD CHILD CHILD CHILD CHILD CHILD     *
        ******************************************************************/
        char *args[4];		/* command line send to shell */
        meUByte *ss;
        
        /* close parent side */
        close(fds[0]);
        if(outFds[1] != fds[0])
            close(outFds[1]);
        
        /* Dissassociate the new process from the controlling terminal */
#if (defined _BSD) && (defined TIOCNOTTY)
        /* Under BSD then we allocate a dummy tty and then immediatly shut it.
         * This has the desired effect of dissassociating the terminal */
        if(flags & LAUNCH_USEPTY)
        {
            /* Under BSD 4.2 then we have to break the tty off. We make a
             * dummy call to open a tty and then immediately close it. This
             * was fixed on BSD4.3, but the same technique works so continue
             * to use it !! */
            int tempFp;

            if((tempFp = open ("/dev/tty", O_RDWR, 0)) >= 0)
            {
                ioctl(tempFp, TIOCNOTTY);
                close(tempFp);
            }
        }
        
        /* Put the process into parent group 0. Note that setsid() does the
         * same job under SVR4. */
        setpgrp(0,0);                  /* BSD */
#else
        /* Under POSIX.1 environments then simply use setsid() to
         * dissassociate from the terminal. This will also sort out the group
         * ID's groups. */
        setsid();                      /* Disassociate terminal */

        /* Assign the parent group. Old System V has a setpgrp() with no
         * arguments. Newer programs should use setgpid() instead. It's
         * debatable if we actually need this because setsid() might do it,
         * however no harm will come from re-assigning the parent group. */
#ifdef _SVID   
        setpgrp();                     /* Old System V */
#else
        setpgid(0,0);                  /* Newer UNIX systems */
#endif        
        /* Not sure what the hell this does, why is it here ?? */
#if (defined TIOCSCTTY) && ((defined _LINUX_BASE) || (defined _FREEBSD_BASE))
        if((flags & LAUNCH_USEPTY) && (outFds[0] >= 0))
            ioctl(outFds[0],TIOCSCTTY,0);
#endif
#endif
        /* On BSD systems then we should try and use the new Barkley line
         * disciplines for communication, especially if we are a pty otherwise
         * we will get some problems with the shell. For simple pipes we do
         * not need to bother. */
#if (defined _BSD) && (defined NTTYDISC) && (defined TIOCSETD)
        if((flags & LAUNCH_USEPTY) && (outFds[0] >= 0))
        {
            /* Use new line discipline.  */
            int ldisc = NTTYDISC;
            ioctl(outFds[0], TIOCSETD, &ldisc);
        }
#endif /* defined (NTTYDISC) && defined (TIOCSETD) */

        /* The child process has inherited the parent signals. Ensure that all
         * of the signals are reset to their correct default value */
#ifdef _POSIX_SIGNALS
        {
            struct sigaction sa;

            sigemptyset(&sa.sa_mask);
            sa.sa_flags=SA_RESETHAND;
            sa.sa_handler=NULL;
            sigaction(SIGHUP,&sa,NULL);
            sigaction(SIGINT,&sa,NULL);
            sigaction(SIGQUIT,&sa,NULL);
            sigaction(SIGTERM,&sa,NULL);
            sigaction(SIGUSR1,&sa,NULL);
            sigaction(SIGALRM,&sa,NULL);
            sigaction(SIGCHLD,&sa,NULL);

            /* Release any signals that might be blocked */
            sigprocmask(SIG_SETMASK,&sa.sa_mask,NULL);
        }
#else
        signal(SIGHUP,SIG_DFL);
        signal(SIGINT,SIG_DFL);
        signal(SIGQUIT,SIG_DFL);
        signal(SIGTERM,SIG_DFL);
        signal(SIGUSR1,SIG_DFL);
        signal(SIGALRM,SIG_DFL);
        signal(SIGCHLD,SIG_DFL);
#ifdef _BSD_SIGNALS
        /* Release any signals that might be blocked */
        sigsetmask (0);
#endif /* _BSD_SIGNALS */
#endif /* _POSIX_SIGNALS  */

#if !((defined _LINUX_BASE) || (defined _FREEBSD_BASE) || (defined _SUNOS) || (defined _BSD))
        /* Some systems the tty is opened late as here */
        if(flags & LAUNCH_USEPTY)
        {
            fds[1] = outFds[0] = open((const char *)line,O_RDWR,0);
        }
#endif /* !_LINUX/_FREEBSD/_SUNOS/_BSD */

        /* On solaris (this is POSIX I believe) then push the line emulation
         * modes */
#ifdef _SUNOS
        if(flags & LAUNCH_USEPTY)
        {
            /* Push the hardware emulation mode */
            ioctl(fds[1], I_PUSH, "ptem");
            
            /* Push the line discipline module */
            ioctl(fds[1], I_PUSH, "ldterm");
        }
#endif /* _SUNOS */

        /* Close the existing stdin/out/err */
        close(0);
        close(1);
        close(2);

        /* Duplicate the new descriptors on stdin/out/err */
        dup2(outFds[0],0);
        dup2(fds[1],1);
        dup2(fds[1],2);                /* stdout => stderr */

        /* Dispose of the descriptors */
        close(outFds[0]);
        close(fds[1]);

        /* Fix up the line disciplines */
        childSetupTty();

        if((term != NULL) && ((term=(meUByte *) strdup((char *) term)) != NULL))
            mePutenv(term);
        
        ss = getShellCmd();
        args[0] = (char *) ss;
        if(meStrcmp(ss,comStr))
        {
            args[1] = "-c";
            args[2] = (char *) comStr;
            args[3] = NULL;
        }
        else
            args[1] = NULL;

#ifndef _NOPUTENV
        execv(args[0],args);
#else
        /* We need to push the environment variable, however in order to do
         * this then we need to supply the absolute pathname of the
         * executable. Search the $PATH for the executable. */
        if(meEnviron != NULL)
        {
            char buf[meBUF_SIZE_MAX];

            if(executableLookup(args[0],buf))
                args[0] = buf;
            execve(args[0],args,meEnviron);
        }
        else
            execv(args[0],args);
#endif
        exit(1);                       /* Should never get here unless we fail */
    }
    /* close child side */
    close(fds[1]);
    if(outFds[0] != fds[1])
        close(outFds[0]);
    ipipe->pid = pid;
    ipipe->exitCode = 0;
    ipipe->rfd = fds[0];
    ipipe->outWfd = outFds[1];
#endif /* _WIN32 */
    
    if(cd)
        meChdir(curdir);
    
    /* Link in the ipipe */
    ipipe->next = ipipes;
    ipipes = ipipe;
    noIpipes++;

    /* Create the output buffer */
    {
        meMode sglobMode;
        meModeCopy(sglobMode,globMode);
        if (flags & (LAUNCH_RAW|LAUNCH_NO_WRAP))
            meModeClear(globMode,MDWRAP);
        else
            meModeSet(globMode,MDWRAP);
        meModeSet(globMode,MDPIPE);
        meModeSet(globMode,MDLOCK);
        meModeClear(globMode,MDUNDO);
        bp=bfind(bufName,BFND_CREAT|BFND_CLEAR);
        meModeCopy(globMode,sglobMode);
    }
    if((ipipe->bp = bp) == NULL)
    {
#ifdef _UNIX
        meSigRelease ();
#endif /*_UNIX */
        ipipeRemove(ipipe);
        return mlwrite(MWABORT,(meUByte *)"[Failed to create %s buffer]",bufName);
    }
#ifdef IPIPE_DEBUG
    if(logFp == NULL)
        logFp = fopen("./ipipe.log","wb+");
#endif
    /* setup the buffer */
    if(flags & LAUNCH_BUFIPIPE)
        bp->ipipeFunc = ipipeFunc;
    bp->ipipeFlags = ((flags & LAUNCH_USEPTY) ? (meBUFFER_IPIPE_USED|meBUFFER_IPIPE_PTY):meBUFFER_IPIPE_USED) | 
          ((flags & (LAUNCH_RAW|LAUNCH_NO_WRAP)) ? 0:meBUFFER_IPIPE_WRAP) | ((flags & LAUNCH_ANSICOLOR) ? meBUFFER_IPIPE_COLOR:0);
    bp->fileName = meStrdup(path);
    if((flags & LAUNCH_RAW) == 0)
    {
        meStrcpy(line,"cd ");
        meStrcat(line,path);
        addLineToEob(bp,line);
        addLineToEob(bp,comStr);
        addLineToEob(bp,(meUByte *)"\n");
    }
    else
        bp->ipipeFlags |= meBUFFER_IPIPE_RAW;
    bp->dotLine = meLineGetPrev(bp->baseLine);
    bp->dotOffset = 0;
    bp->dotLineNo = bp->lineCount-1;
    meAnchorSet(bp,'I',bp->dotLine,bp->dotLineNo,bp->dotOffset,1);

    /* Set up the window dimensions - default to having auto wrap */
    ipipe->flag = (flags & (LAUNCH_RAW|LAUNCH_USEPTY|LAUNCH_ANSICOLOR));
    /* On a tty the ONLCR output translation turns the child's LF into CR+LF, so the carriage return arrives
     * separately and the LF must only index. If the child clears ONLCR it is a full-screen application and
     * a bare LF means index as well, so a tty wants indexing either way.
     * Without a tty there is no translation and nothing else supplies the CR so force meIPIPE_LFISNL on. */
    if((flags & LAUNCH_USEPTY) == 0)
        ipipe->flag |= meIPIPE_LFISNL;
#if MEOPT_EXTENDED
#ifdef _WIN32
    /* TODO Currently processes are launched with current codepage to avoid UTF8 encoding issues, but even this is not great as ME may be set to use a different codepage.
     * However windows PTY makes this much worse, it always uses UTF8 regardless, so we must take control of this setting.
     * long term we should consider change the exec of all over to utf8 and use ME's conversion so there is parity between platforms. */
    if(((flags & LAUNCH_USEPTY) == 0) || (flags & (LAUNCH_NOUTF8|LAUNCH_RAW)))
#else
    if(((meSystemCfg & meSYSTEM_IO_UTF8) == 0) || (flags & (LAUNCH_NOUTF8|LAUNCH_RAW)))
#endif
        ipipe->flag |= meIPIPE_NOUTF8;
    else
        bp->ipipeFlags |= meBUFFER_IPIPE_UTF8;
#endif
    ipipe->oscSplit = 0;
    ipipe->ansiCc = 'A';
    ipipe->ansiFg = 0;
    ipipe->ansiBg = 0;
    ipipe->ansiSt = 0;
    ipipe->strRow = 0;
    ipipe->strCol = 0;
    ipipe->noRows = 0;
    ipipe->noCols = 0;
    /* initialising this to 0 tries to preserve the 3 line header from being trashed by a program that redraws screen by moving cursor to 1,1 etc. This is common on Windows */
    ipipe->curRow = 0;
    ipipeSetSize(frameCur->windowCur,bp);
#ifdef _UNIX
    /* Release the signals - we can now cope if the child dies or writes. */
    meSigRelease ();
#endif /*_UNIX */
  
    if(!(flags & LAUNCH_SILENT))
    {
        /* get a popup window for the command output */
        meWindow *wp;
        if(((wp = meWindowPopup(bp,NULL,0,NULL)) != NULL) && (ipipes == ipipe))
            ipipeSetSize(wp,bp);
    }
    
    if((bp->ipipeFunc >= 0) && (ipipes == ipipe))
        /* Give argument of 1 to indicate process has not exited */
        execBufferFunc(bp,bp->ipipeFunc,(meEBF_ARG_GIVEN|meEBF_USE_B_DOT|meEBF_HIDDEN),1);
    
    /* reset again incase there was a delay in the meWindowPopup call */
    bp->dotLine = meLineGetPrev(bp->baseLine);
    bp->dotOffset = 0;
    bp->dotLineNo = bp->lineCount-1;
    resetBufferWindows(bp);

    return meTRUE;
}

int
ipipeCommand(int f, int n)
{
    meBuffer *bp;
    meUByte lbuf[meBUF_SIZE_MAX], *cl ;	/* command line send to shell */
    meUByte nbuf[meBUF_SIZE_MAX], *bn ;	/* buffer name */
    meUByte pbuf[meBUF_SIZE_MAX];
    int ipipeFunc ;                     /* ipipe-buffer function. */
    int ss;

    if(!(meSystemCfg & meSYSTEM_IPIPES))
        /* No ipipes flagged so just do a normal pipe */
        return pipeCommand(f,n);
    
    /* Get the command to pipe in */
    if((ss=meGetString((meUByte *)"Ipipe", 0, 0, lbuf, meBUF_SIZE_MAX)) <= 0)
        return ss;
    if((alarmState & meALARM_PIPED) && ((n & LAUNCH_USEPTY) == 0))
        n |= LAUNCH_NOPTY;
    if(n & LAUNCH_BUFCMDLINE)
    {
        if((bp=bfind(lbuf,0)) == NULL)
            return mlwrite(MWABORT,(meUByte *)"[%s: no such buffer]",lbuf);
        cl = meLineGetText(meLineGetNext(bp->baseLine));
    }
    else
        cl = lbuf;
    
    if((n & LAUNCH_BUFFERNM) == 0)
    {
        /* prompt for and get the new buffer name */
        if((ss = getBufferName((meUByte *)"Buffer", 0, 0, nbuf)) <= 0)
            return ss;
        bn = nbuf;
    }
    else
        bn = BicommandN;

    /* Get the buffer ipipe if requested. */
    if ((n & LAUNCH_BUFIPIPE) != 0)
    {
        /* prompt for the $buffer-ipipe */
        if ((ss = meGetString((meUByte *)"Command", MLCOMMAND, 0,
                              pbuf, meBUF_SIZE_MAX)) <= 0)
            return ss;
        ipipeFunc = decode_fncname(pbuf,1);
    }
    else
        ipipeFunc = -1;
    
    getFilePath(frameCur->windowCur->buffer->fileName,pbuf);
    return doIpipeCommand(cl,pbuf,bn,ipipeFunc,(n & LAUNCH_USER_FLAGS));
}

int
anyActiveIpipe(void)
{
    if((ipipes == NULL)
#if MEOPT_CLIENTSERVER
       || ((ipipes->pid == 0) && (ipipes->next == NULL))
#endif
       )
        return meFALSE;
    return meTRUE;
}

#endif


/*
 * Pipe a one line command into a window
 * Bound to ^X @
 */
int
doPipeCommand(meUByte *comStr, meUByte *path, meUByte *bufName, int ipipeFunc, int flags, meRegister *regs)
{
    register meBuffer *bp;	/* pointer to buffer to zot */
    meUByte buff[meBUF_SIZE_MAX+3];
    meInt systemRet;
    int cd, ret;
#ifdef _DOS
    static meByte pipeStderr=0;
    meUByte cc, *dd, *cl, *ss;
    int gotPipe=0;
#endif
#ifdef _UNIX
    FILE *pfp;
    meWAIT_STATUS ws;
    meUByte *cl, *ss;
    size_t ll;
#else
    meUByte filnam[meBUF_SIZE_MAX];

    mkTempName(filnam,NULL,NULL);
#endif
    
    /* get rid of the output buffer if it exists and create new */
    if(flags & LAUNCH_TO_VAR)
        flags = (flags & ~LAUNCH_BUFIPIPE) | LAUNCH_RAW;
    else if((bp=bfind(bufName,BFND_CREAT|BFND_CLEAR)) == NULL)
        return meFALSE;
    cd = (meStrcmp(path,curdir) && (meChdir(path) != -1));

#ifdef _DOS
    if(!pipeStderr)
        pipeStderr = (meGetenv("ME_PIPE_STDERR") != NULL) ? 1 : -1;
    
    if((cl = meMalloc(meStrlen(comStr) + meStrlen(filnam) + 4)) == NULL)
        return meFALSE;
        
    dd = cl;
    ss = comStr;
    /* convert '/' to '\' in program name */
    while((cc=*ss++) && (cc != ' '))
    {
        if(cc == '/')
            cc = '\\';
        *dd++ = cc;
    }
    if(cc)
    {
        *dd++ = cc;
        while((cc=*ss++))
        {
            if((cc == ' ') && (*ss == '>'))
                gotPipe = 1;
            *dd++ = cc;
        }
    }
    *dd = '\0';
    if(!gotPipe)
    {
        *dd++ = ' ';
        *dd++ = '>';
        if(pipeStderr > 0)
            *dd++ = '&';
        meStrcpy(dd,filnam);
    }
    if((flags & LAUNCH_SILENT) == 0)
        mlerase(MWERASE|MWCURSOR);
    systemRet = system((char *) cl);
    if(cd)
        meChdir(curdir);
    /* Call TTopen as we can't guarantee whats happend to the terminal */
    TTopen();
    meFree(cl);
    if(meTestExist(filnam))
        return meFALSE;
#endif
#ifdef _WIN32
    
    ret = WinLaunchProgram(comStr,(LAUNCH_PIPE|flags), NULL, filnam,
#if MEOPT_IPIPES
                           NULL, 
#endif
                           &systemRet);
    if(cd)
        meChdir(curdir);
    if(ret == meFALSE)
        return meFALSE;

#endif
#ifdef _UNIX
    ll = meStrlen(comStr);
    if((cl = meMalloc(ll + 17)) == NULL)
        return meFALSE;
    
    if((ss=meStrchr(comStr,'|')) == NULL)
        ss = comStr + ll;
    else
        ll = (size_t)(ss - comStr);
    
    meStrncpy(cl,comStr,ll);
    cl[ll] = '\0';
    /* if no data is piped in then pipe in /dev/null */
    if(meStrchr(cl,'<') == NULL)
    {
        memcpy(cl+ll," </dev/null",11);
        ll += 11;
    }
    /* merge stderr and stdout */
    memcpy(cl+ll," 2>&1",5);
    ll += 5;
    meStrcpy(cl+ll,ss);
    
    /* Must flag to our sigchild handler that we are running a piped command
     * otherwise it will call waitpid with -1 and loose the exit status of
     * this process */
    alarmState |= meALARM_PIPE_COMMAND;
    pfp = popen((char *) cl, "r");
    /* With no buffer fname swbuffer -> readin -> ffReadFile will assume its pipe and we must set and close meior.fp */
    meior.fp = pfp;
    if(cd)
        meChdir(curdir);
    TTflush();
    meFree(cl);
#endif
    if((flags & LAUNCH_RAW) == 0)
    {
        meStrcpy(buff,"cd ");
        meStrcpy(buff+3,path);
        addLineToEob(bp,buff);
        addLineToEob(bp,comStr);
        addLineToEob(bp,(meUByte *)"");
    }
    /* If this is an ipipe launch then call the callback. */
    if((flags & LAUNCH_BUFIPIPE) && (ipipeFunc >= 0))
    {
#if MEOPT_IPIPES
        bp->ipipeFunc = ipipeFunc;
#endif
        execBufferFunc(bp,ipipeFunc,(meEBF_ARG_GIVEN|meEBF_USE_B_DOT|meEBF_HIDDEN),1);
    }
    /* and read the stuff in */
#ifdef _UNIX
    if(flags & LAUNCH_TO_VAR)
        ret = ffReadFileToBuffer(NULL,buff,meBUF_SIZE_MAX);
    else
        ret = meBufferInsertFile(bp,NULL,meRWFLAG_SILENT|meRWFLAG_PRESRVFMOD,0,0,0);
    /* close the pipe and get exit status */
    ws = (meWAIT_STATUS) pclose(pfp);
    if(WIFEXITED(ws))
        systemRet = WEXITSTATUS(ws);
    else
        systemRet = -1000;
    alarmState &= ~meALARM_PIPE_COMMAND;
#else
    if(flags & LAUNCH_TO_VAR)
        ret = ffReadFileToBuffer(filnam,buff,meBUF_SIZE_MAX);
    else
        ret = meBufferInsertFile(bp,filnam,meRWFLAG_SILENT|meRWFLAG_PRESRVFMOD,0,0,0);
    /* and get rid of the temporary file */
    meUnlinkNT(filnam);
#endif
    
#if MEOPT_EXTENDED
    if((ret > 0) && !(flags & LAUNCH_TO_VAR) && !(flags & LAUNCH_RAW) &&
       (meSystemCfg & meSYSTEM_IO_UTF8) && !(flags & LAUNCH_NOUTF8))
        meBufferDecodeUtf8(bp);
#endif
    if(flags & LAUNCH_TO_VAR)
    {
        if(ret > 0)
            ret = setVar(bufName,buff,regs);
    }
    else
    {
        /* give it the path as a filename */
        bp->fileName = meStrdup(path);
        /* make this window in VIEW mode, update all mode lines */
        meModeClear(bp->mode,MDEDIT);
        meModeSet(bp->mode,MDVIEW);
        bp->dotLine = meLineGetNext(bp->baseLine);
        bp->dotLineNo = 0;
        resetBufferWindows(bp);

        if((flags & LAUNCH_SILENT) == 0)
            meWindowPopup(bp,NULL,WPOP_MKCURR,NULL);

        /* Issue the callback if required. */
#if MEOPT_IPIPES
        ipipeFunc = bp->ipipeFunc;
        if ((flags & LAUNCH_BUFIPIPE) && (ipipeFunc >= 0))
            execBufferFunc(bp,ipipeFunc,(meEBF_ARG_GIVEN|meEBF_USE_B_DOT|meEBF_HIDDEN),0);
#endif 
    }
    meStrcpy(resultStr,meItoa(systemRet));
    return ret;
}

/*
 * Pipe a one line command into a window
 * Bound to ^X @
 */
int
pipeCommand(int f, int n)
{
    register int ss;
    meBuffer *bp;
    meUByte lbuf[meBUF_SIZE_MAX], *cl; /* command line send to shell */
    meUByte nbuf[meBUF_SIZE_MAX], *bn;	/* buffer name */
    meUByte pbuf[meBUF_SIZE_MAX];
    meRegister *regs=NULL;

    /* get the command to pipe in */
    if((ss=meGetString((meUByte *)"Pipe", 0, 0, lbuf, meBUF_SIZE_MAX)) <= 0)
        return ss;
    if(n & LAUNCH_BUFCMDLINE)
    {
        if((bp=bfind(lbuf,0)) == NULL)
            return mlwrite(MWABORT,(meUByte *)"[%s: no such buffer]",lbuf);
        cl = meLineGetText(meLineGetNext(bp->baseLine));
    }
    else
        cl = lbuf;
    
    if(n & LAUNCH_TO_VAR)
    {
        /* prompt for and get the variable name */
        /* horrid global variable, see notes at definition */
        extern meRegister *gmaLocalRegPtr;
        gmaLocalRegPtr = meRegCurr;
        alarmState |= meALARM_VARIABLE;
        ss = meGetString((meUByte *)"Variable",MLVARBL,0,nbuf,meSBUF_SIZE_MAX);
        alarmState &= ~meALARM_VARIABLE;
        regs = gmaLocalRegPtr;
        if(ss <= 0)
            return ss;
        bn = nbuf;
    }
    else if((n & LAUNCH_BUFFERNM) == 0)
    {
        /* prompt for and get the new buffer name */
        if((ss = getBufferName((meUByte *)"Buffer", 0, 0, nbuf)) <= 0)
            return ss;
        bn = nbuf;
    }
    else
        bn = BcommandN;

    getFilePath(frameCur->windowCur->buffer->fileName,pbuf);

    return doPipeCommand(cl,pbuf,bn,-1,(n & LAUNCH_USER_FLAGS),regs);
}

#if MEOPT_EXTENDED
/*
 * filter a buffer through an external program. This needs to be rewritten
 * under UNIX to use pipes.
 *
 * Bound to ^X #
 */
int
meFilter(int f, int n)
{
    register int     s;			/* return status from CLI */
    register meBuffer *bp;		/* pointer to buffer to zot */
    meUByte            line[meBUF_SIZE_MAX];	/* command line send to shell */
    meUByte           *tmpnam ;		/* place to store real file name */
#ifdef _UNIX
    int	             exitstatus;	/* exit status of command */
#endif
    meUByte filnam1[meBUF_SIZE_MAX];
    meUByte filnam2[meBUF_SIZE_MAX];

    /* Construct the filter names */
    mkTempName(filnam1,NULL,NULL);
    mkTempName(filnam2,NULL,NULL);

    /* get the filter name and its args */
    if((s=meGetString((meUByte *)"Filter",0,0,line,meBUF_SIZE_MAX)) <= 0)
        return s;

    if((s=bufferSetEdit()) <= 0)               /* Check we can change the buffer */
        return s;

    /* setup the proper file names */
    bp = frameCur->windowCur->buffer;
    tmpnam = bp->fileName;	/* save the original name */
    bp->fileName = NULL;	/* set it to NULL         */

    /* write it out, checking for errors */
    if(writeOut(bp,meRWFLAG_SILENT,filnam1) <= 0)
    {
        bp->fileName = tmpnam;
        return mlwrite(MWABORT,(meUByte *)"[Cannot write filter file]");
    }

#ifdef _DOS
    meStrcat(line," <");
    meStrcat(line, filnam1);
    meStrcat(line," >");
    meStrcat(line, filnam2);
    mlerase(MWERASE|MWCURSOR);
    system((char *) line);
    sgarbf = meTRUE;
    s = meTRUE;
#endif
#ifdef _WIN32
    s = WinLaunchProgram(line,LAUNCH_FILTER,filnam1,filnam2,
#if MEOPT_IPIPES
                         NULL,
#endif
                         NULL);
    sgarbf = meTRUE;
#endif
#ifdef _UNIX
    meStrcat(line," <");
    meStrcat(line,filnam1);
    meStrcat(line," >");
    meStrcat(line,filnam2);
    errno = 0;			/* clear errno before call */
    if((exitstatus = system((char *)line)) != 0)
    {
        if(errno == 0)
            mlwrite(MWCURSOR|MWWAIT,(meUByte *)"exit status %d",exitstatus);
        else
            mlwrite(MWCURSOR|MWWAIT,(meUByte *)"exit status %d, errno %d",exitstatus,errno);
    }
    sgarbf = meTRUE;
    s = meTRUE;
#endif

    /* on failure, escape gracefully */
    if(s > 0)
    {
        bp->fileName = filnam2;
        if((bclear(bp) <= 0) ||
           ((frameCur->windowCur->buffer->intFlag |= BIFFILE),
            (swbuffer(frameCur->windowCur,frameCur->windowCur->buffer) <= 0)))
            s = meFALSE;
    }
    /* reset file name */
    bp->fileName = tmpnam;
    /* and get rid of the temporary file */
    meUnlinkNT(filnam1);
    meUnlinkNT(filnam2);

    if(s <= 0)
        mlwrite(0,(meUByte *)"[Execution failed]");
    else
        meModeSet(bp->mode,MDEDIT);		/* flag it as changed */

    return s;
}
#endif

#ifdef _UNIX
#if MEOPT_EXTENDED
int
suspendEmacs(int f, int n)		/* suspend MicroEMACS and wait to wake up */
{
    /*
    ** Note that we might have got here by hitting the wrong keys. If you've
    ** ever tried suspending something when you havent got job control in your
    ** shell, its painful.
    **
    ** Confirm with the user that they want to suspend if the basename of the
    ** SHELL environment variable is NOT "ksh" or "csh" and it hasnt got a "j"
    ** in it.
    */
    if((n & 0x01) && (mlyesno((meUByte *)"Suspend") <= 0))
        return meFALSE ;

    TTclose();				/* stty to old settings */
    kill(getpid(),SIGTSTP);
    TTopen();
    sgarbf = meTRUE;

    return meTRUE;
}
#endif
#endif

#endif /* MEOPT_SPAWN */
