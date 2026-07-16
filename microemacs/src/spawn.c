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

static void
ipipeWriteString(meIPipe *ipipe, int n, meUByte *str)
{
    while(n--)
    {
#ifdef _WIN32
        DWORD written ;
        WriteFile(ipipe->outWfd,str,(DWORD) meStrlen(str),&written,NULL) ;
#else
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
                        Sleep(50) ;
                        
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

#define IPIPE_DUMP 1
#ifdef IPIPE_DUMP
static FILE *logFp=NULL;
#endif

#ifdef _WIN32

static int
readFromPipe(meIPipe *ipipe, int nbytes, meUByte *buff)
{
    DWORD  bytesRead ;

    /* See if process has ended first */
    if(ipipe->pid < 0)
        return ipipe->pid ;
#if MEOPT_CLIENTSERVER
    if(ipipe->pid == 0)
    {
        if(ttServerToRead == 0)
            return 0 ;
        if(nbytes > ttServerToRead)
            nbytes = ttServerToRead ;
        if(ReadFile(ipipe->rfd,buff,nbytes,&bytesRead,NULL) == 0)
            return -1;
#ifdef IPIPE_DUMP
        if((bytesRead > 0) && (logFp != NULL))
            fwrite(buff,1,bytesRead,logFp);
#endif
        return (int) bytesRead;
    }
#endif
    if(ipipe->flag & meIPIPE_CHILD_EXIT)
    {
        GetExitCodeProcess(ipipe->process,(LPDWORD) &(ipipe->exitCode)) ;
        CloseHandle(ipipe->process);
        ipipe->pid = -4 ;
        return ipipe->pid ;
    }
    if(ipipe->flag & meIPIPE_NEXT_CHAR)
    {
        buff[0] = ipipe->nextChar ;
        ipipe->flag &= ~meIPIPE_NEXT_CHAR ;
#ifdef IPIPE_DUMP
        if(logFp != NULL)
            fwrite(buff,1,1,logFp);
#endif
        return 1 ;
    }
    /* Must peek on a pipe cos if we try to read too many this will fail */
    if((PeekNamedPipe(ipipe->rfd, (LPVOID) NULL, (DWORD) 0,
                      (LPDWORD) NULL, &bytesRead, (LPDWORD) NULL) != meTRUE) ||
       (bytesRead <= 0))
        return 0 ;
    if(bytesRead > (DWORD) nbytes)
        bytesRead = (DWORD) nbytes ;
    if(ReadFile(ipipe->rfd,buff,bytesRead,&bytesRead,NULL) == 0)
        return -1 ;
#ifdef IPIPE_DUMP
    if((bytesRead > 0) && (logFp != NULL))
        fwrite(buff,1,bytesRead,logFp);
#endif
    return (int) bytesRead ;
}

#else

#if MEOPT_CLIENTSERVER || (defined (IPIPE_DUMP))

#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <netinet/in.h>

static int
readFromPipe(meIPipe *ipipe, int nbytes, meUByte *buff)
{
    int ii;
    if(ipipe->pid == 0)
    {
        if ((ii = recv(ipipe->rfd,(char *) buff,nbytes,0)) < 0)
            ii = 0;
    }
    else
        ii = read(ipipe->rfd,buff,nbytes);
#ifdef IPIPE_DUMP
    if((ii > 0) && (logFp != NULL))
        fwrite(buff,1,ii,logFp);
#endif
    return ii;
}


#else

#define readFromPipe(ipipe,nbytes,buff) read(ipipe->rfd,buff,nbytes)

#endif

#endif

#define ipipeGetNextChar(ipipe,cc,rbuff,curROff,curRRead)                    \
((curROff < curRRead) ?                                                      \
 (((cc)=rbuff[curROff++]), 1):                                               \
 (((curRRead=readFromPipe(ipipe,meBUF_SIZE_MAX,rbuff)) > 0) ?                \
  (((cc)=rbuff[0]),curROff=1): 0))

#define ipipeAddLine(ipipe,lp_old,buff,cbuff)                                \
((ipipe->flag & meIPIPE_ANSICOLOR) ? ipipeAddColorLine(lp_old,buff,cbuff):addLine(lp_old,buff))
#define ipipeDecodeLine(ipipe,src,buff,cbuff,offs)                           \
((ipipe->flag & meIPIPE_ANSICOLOR) ? ipipeDecodeColorLine(src,buff,cbuff,offs):(meStrcpy(buff,src),offs))

#define ipipeStoreInputPos()                                                 \
do {                                                                         \
    meLine *lp_new;                                                          \
    noLines += ipipeAddLine(ipipe,lp_old,buff,cbuff);                        \
    lp_new = meLineGetPrev(lp_old);                                          \
    if(lp_old != bp->baseLine)                                               \
    {                                                                        \
        noLines--;                                                           \
        lp_new->next = lp_old->next;                                         \
        lp_old->next->prev = lp_new;                                         \
        if(lp_old->flag & meLINE_ANCHOR)                                     \
            meLineResetAnchors(meLINEANCHOR_ALWAYS|meLINEANCHOR_RETAIN,bp,   \
                               lp_old,lp_new,0,0);                           \
        meFree(lp_old);                                                      \
    }                                                                        \
    else                                                                     \
    {                                                                        \
        bp->dotLineNo--;                                                     \
        ipipe->curRow--;                                                     \
    }                                                                        \
    bp->dotLineNo += noLines;                                                \
    bp->lineCount += noLines;                                                \
    ipipe->curRow = curRow;                                                  \
    bp->vertScroll = bp->dotLineNo-curRow;                                   \
    bp->dotLine = lp_new;                                                    \
    bp->dotOffset = (meUShort) (p1 - buff);                                  \
    meBufferUpdateLocation(bp,noLines,bp->dotOffset);                        \
} while(0)


static int
ipipeDecodeColorLine(const meUByte *src, meUByte *buff, meUByte *cbuff, int offs)
{
    meUByte cc = 'A';
    int rr=0, i = 0;
    while(*src)
    {
        if((*src == '\x03') && src[1])
        {
            cc = *++src;
            src++;
        }
        else
        {
            cbuff[i] = cc;
            buff[i++] = *src++;
            if(--offs == 0)
                rr = i;
        }
    }
    buff[i] = '\0';
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
ipipeAddColorLine(meLine *lp, const meUByte *buff, const meUByte *cbuff)
{
    meUByte encbuff[3*meBUF_SIZE_MAX+1];
    meUByte cc='A', sc;
    meUByte *op=encbuff;
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
    }
    if(cc != 'A')
    {
        *op++ = '\x03';
        *op++ = 'A';
    }
    *op = '\0';
    return addLine(lp,encbuff);
}

static meUByte
ipipeAnsiToScheme(meUByte fg, meUByte bg, meUByte styl)
{
    if(bg)
    {
        switch(bg & 0x07)
        {
        case 1: return (bg & 0x20) ? 'u' : 'O'; /* red bg: hlred or gdfrej */
        case 2: return (bg & 0x20) ? 'v' : 'N'; /* green bg: hlgreen or gdfsel */
        case 3: return (bg & 0x20) ? 'w' : 'M'; /* yellow bg: hlyellow or gdfchange */
        default: return 'A';
        }
    }
    if(fg)
    {
        switch(fg & 0x07)
        {
        case 1: return (styl & 0x01) ? 'k':'R'; /* red fg -> (bold) .scheme.error:.scheme.rmv */
        case 2: return 'Q';            /* green fg -> .scheme.add */
        case 3: return 'l';            /* yellow fg -> .scheme.warn */
        case 4: return 'S';            /* blue fg -> .scheme.dir */
        case 6: return 'm';            /* cyan fg -> .scheme.info */
        }
    }
    /* bold -> .scheme.bold */
    return (styl & 0x07) ? 'C'+(styl & 0x07):'A';
}

void
ipipeRead(meIPipe *ipipe)
{
    meBuffer *bp=ipipe->bp;
    meLine   *lp_old;
    int     len, curOff, maxOff, curRow, ii;
    meUInt  noLines;
    meUByte  *p1, cc, buff[meBUF_SIZE_MAX+1], cbuff[meBUF_SIZE_MAX+1], rbuff[meBUF_SIZE_MAX];
    int     curROff=0, curRRead=0;
    int     prmA, prmL;

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
    /* This is a quick sanity check which is needed if the buffer has
     * been changed by something. If curRow becomes greater than dotLineNo
     * the vertScroll becomes negative and things go wrong.
     * Discovered problem when using gdb mode as the gdb input handler
     * kills ^Z^Z lines making curRow > dotLineNo.
     */
    if((curRow=ipipe->curRow) > bp->dotLineNo)
        curRow = bp->dotLineNo;
#ifdef IPIPE_DUMP
    if(logFp == NULL)
        logFp = fopen("./ipipe.log","wb+");
#endif
    len = bp->dotOffset;
    lp_old = bp->dotLine;
    meBufferStoreLocation(lp_old,bp->dotOffset,bp->dotLineNo);
    len = ipipeDecodeLine(ipipe,lp_old->text,buff,cbuff,bp->dotOffset);
    p1 = buff+len;
    noLines = 0;
    curOff = getcol(buff,len,bp->tabWidth);
    for(;;)
    {
        if(!ipipeGetNextChar(ipipe,cc,rbuff,curROff,curRRead))
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
        case 7:
            TTbell();
            break;
        case 8:
        case 0x7f: /* DEL - some shells/consoles echo this for erase, treat as backspace */
            if(p1 != buff)
            {
                p1--;
                len--;
                curOff = getcol(buff,len,bp->tabWidth);
            }
            break;
        case '\r':
            p1 = buff;
            len = curOff = 0;
            break;
        case meCHAR_NL:
            if((ipipe->flag & meIPIPE_USEPTY) && !(ipipe->flag & meIPIPE_OVERWRITE) && (curRow+1 < ipipe->noRows))
            {
                /* if in over-write mode and not at the bottom, move instead */
                prmA = curRow + 1;
                prmL = 0;
                goto move_cursor_pos;
            }
            ii = ipipeAddLine(ipipe,lp_old,buff,cbuff);
            noLines += ii;
            if(curRow < ipipe->noRows-1)
                curRow += ii;
            p1 = buff;
            *p1 = '\0';
            len = curOff = 0;
            break;
        case 15: /* ignore */
            break;
        case 27:
            if((ipipe->flag & meIPIPE_USEPTY) && ipipeGetNextChar(ipipe,cc,rbuff,curROff,curRRead))
            {
                int gotQ=0, gotN=0, prmS[8], prmC=0;

                prmL=0;
                prmA=-1;
                if(cc == '[')
                {
get_another:
                    if(ipipeGetNextChar(ipipe,cc,rbuff,curROff,curRRead))
                    {
                        if(isDigit(cc))
                        {
                            gotN = 1;
                            prmL = prmL*10 + (cc - '0');
                            goto get_another;
                        }
                        switch(cc)
                        {
                        case ';':
                            if(prmC < 8)
                                prmS[prmC++] = prmL;
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
                            if(!gotN)
                                prmL = 1;
                            ii = meStrlen(p1);
                            if(ipipe->flag & meIPIPE_ANSICOLOR)
                            {
                                memmove(cbuff+len+prmL,cbuff+len,ii);
                                memset(cbuff+len,(char)ipipe->ansiCc,prmL);
                            }
                            memmove(p1+prmL,p1,ii+1);
                            memset(p1,' ',prmL);
                            p1 += prmL;
                            len += prmL;
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
                            if((prmL + len) >= ipipe->noCols)
                                prmL = ipipe->noCols - len - 1;
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
                            curOff = getcol(buff,len,bp->tabWidth);
                            break;

                        case 'D':
                            if(!gotN)
                                prmL = 1;
                            if(len < prmL)
                                prmL = len;
                            p1 -= prmL;
                            len -= prmL;
                            curOff = getcol(buff,len,bp->tabWidth);
                            break;

                        case 'G':
                        case '`':
                            /* CHA/HPA - cursor horizontal absolute, 1-based */
                            if(!gotN)
                                prmL = 1;
                            prmL--;  /* convert to 0-based */
                            if(prmL < 0)
                                prmL = 0;
                            else if(prmL >= ipipe->noCols)
                                prmL = ipipe->noCols - 1;
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
                            curOff = getcol(buff,len,bp->tabWidth);
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
move_cursor_pos:
                            if(prmA < 0)
                                prmA = 0;
                            else if(prmA >= ipipe->noRows)
                                prmA = ipipe->noRows - 1;
                            if(prmL < 0)
                                prmL = 0;
                            else if(prmL >= ipipe->noCols)
                                prmL = ipipe->noCols - 1;
                            ipipeStoreInputPos();
                            bp->dotLineNo += prmA - curRow ;
                            lp_old = bp->dotLine;
                            if(prmA > curRow)
                            {
                                while((curRow != prmA) && (lp_old != bp->baseLine))
                                {
                                    curRow++;
                                    lp_old = meLineGetNext(lp_old);
                                }
                                while(curRow != prmA)
                                {
                                    curRow++;
                                    addLineToEob(bp,(meUByte *)"");
                                }
                            }
                            else
                            {
                                while(curRow != prmA)
                                {
                                    curRow--;
                                    lp_old = meLineGetPrev(lp_old);
                                }
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
                            curOff = getcol(buff,len,bp->tabWidth);
                            noLines = 0;
                            break;

                        case 'h':
                            if(gotQ)
                            {
                                if(prmL == 7)
                                    ipipe->flag &= ~meIPIPE_NOAUTOWRAP;
#ifndef NDEBUG
                                /* safe to ignore: cursor key mode (prmL = 1) & bracketed paste (2004), show/hide cursor (25), focus in/out (1004)
                                 * synchronized output (2026), unknown private mode (2031) */
                                else if((prmL != 1) && (prmL != 25) && (prmL < 1000 || prmL > 1006) && (prmL != 2004) && (prmL != 2026) && (prmL != 2031))
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
#ifndef NDEBUG
                                /* safe to ignore: cursor key mode (prmL = 1) & bracketed paste (2004), show/hide cursor (25), focus in/out (1004)
                                 * synchronized output (2026), unknown private mode (2031) */
                                else if((prmL != 1) && (prmL != 25) && (prmL < 1000 || prmL > 1006) && (prmL != 2004) && (prmL != 2026) && (prmL != 2031))
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
                                if(prmC < 8)
                                    prmS[prmC++] = prmL;
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
                                                newSt |= 1<<(prmA - 2);
                                            else if(prmA != 2)
                                                newSt |= 0x01;
                                        }
                                        else if(prmA < 22)
                                            ;
                                        else if(prmA < 25)
                                            newSt &= ~(1<<(prmA-22));
                                    }
                                    else if(prmA < 40)
                                    {
                                        if(prmA < 38)
                                            newFg = (meUByte) (prmA - 30 + 16);
                                        else if(prmA == 38)
                                            /* extended fg colour: skip remaining params */
                                            break;
                                        else
                                            newFg = 0;
                                    }
                                    else if(prmA < 50)
                                    {
                                        if(prmA < 48)
                                            newBg = (meUByte) (prmA - 40 + 16);
                                        else if(prmA == 48)
                                            /* extended bg colour: skip remaining params */
                                            break;
                                        else
                                            newBg = 0;
                                    }
                                    else if(prmA < 90)
                                        ;
                                    else if(prmA < 100)
                                    {
                                        if(prmA < 98)
                                            newFg = (meUByte) (prmA - 90 + 16 + 32);
                                    }
                                    else if(prmA < 108)
                                        newBg = (meUByte) (prmA - 100 + 16 + 32);
                                }
                                ipipe->ansiFg = newFg;
                                ipipe->ansiBg = newBg;
                                ipipe->ansiSt = newSt;
                                ipipe->ansiCc = ipipeAnsiToScheme(newFg,newBg,newSt);
                            }
                            break;
                        case 'n':
                            {
                                char outb[20];

                                if(prmL != 6)
#ifndef NDEBUG
                                    goto cant_handle_this;
#else
                                    break;
#endif
                                sprintf(outb,"\033[%d;%dR",curRow,len);
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

                                lp = lp_old;
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
                                curOff = len;
                                break;
                            }
                        case 'K':
                            *p1 = '\0';
                            break;
                        case 'P':
                            {
                                /* DCH - delete prmL chars and shifts the remainder left. */
                                int ll;
                                if(!gotN)
                                    prmL = 1;
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
                    /* OSC: consume until BEL (0x07) or ST (ESC \) */
                    do {
                        if(!ipipeGetNextChar(ipipe,cc,rbuff,curROff,curRRead))
                            break;
                    } while((cc != 7) && (cc != 27));
                    /* if terminated by ESC, consume the following \ */
                    if(cc == 27)
                        ipipeGetNextChar(ipipe,cc,rbuff,curROff,curRRead);
                    break;
                }
                else if(cc == '(' || cc == ')')
                {
                    /* character set designation: consume the single designator byte and ignore all */
                    ipipeGetNextChar(ipipe,cc,rbuff,curROff,curRRead);
                    break;
                }
            }
            /* fall through */
        default:
#if MEOPT_EXTENDED
            if(!(ipipe->flag & meIPIPE_NOUTF8) && (cc >= 0x80))
            {
                meUByte c2, c3;
                if((cc < 0xc0) || !ipipeGetNextChar(ipipe,c2,rbuff,curROff,curRRead) || ((c2 & 0xc0) != 0x80))
                    /* orphan continuation byte - discard */
                    break;
                if(cc < 0xe0)
                    /* 2-byte sequence (0xc0-0xdf) */
                    cc = utf8ToMeChar((((meUInt)(cc & 0x1f)) << 6) | (c2 & 0x3f));
                else if(!ipipeGetNextChar(ipipe,c3,rbuff,curROff,curRRead) || ((c3 & 0xc0) != 0x80))
                    break;
                else if(cc < 0xf0)
                    /* 3-byte sequence */
                    cc = utf8ToMeChar((((meUInt) (cc & 0x0f)) << 12) | (((meUInt) (c2 & 0x3f)) << 6) | (c3 & 0x3f));
                else if(!ipipeGetNextChar(ipipe,c2,rbuff,curROff,curRRead) || ((c2 & 0xc0) != 0x80))
                    break;
                else
                    /* 4-byte: ME supports only up to U+FFFF - consume */
                    cc = meCHAR_UNDEF;
#ifdef IPIPE_DUMP
                if((cc == meCHAR_UNDEF) && (logFp != NULL))
                    /* Put an easy to spot marker into the log */
                    fwrite("ZUNZ",1,4,logFp);
#endif
                /* Should unrepresentable (cc == meCHAR_UNDEF) be discarded? */
            }
#endif
            if(curOff >= maxOff)
            {
                if(ipipe->flag & meIPIPE_NOAUTOWRAP)
                {
                    /* stay at right margin - back up and overwrite last column */
                    p1--;
                    len--;
                    curOff = maxOff - 1;
                }
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
                    ii = ipipeAddLine(ipipe,lp_old,buff,cbuff);
                    noLines += ii;
                    if(curRow < ipipe->noRows-1)
                        curRow += ii;
                    p1[0] = bb[0];
                    p1[1] = bb[1];
                    meStrcpy(buff,p1);
                    if(ipipe->flag & meIPIPE_ANSICOLOR)
                        memmove(cbuff,cbuff+splitIdx,meBUF_SIZE_MAX-splitIdx);
                    p1 = buff;
                    len = curOff = 0;
                }
            }
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
            else if((cc == meCHAR_TAB) && (get_tab_pos(curOff,bp->tabWidth) == 0))
            {
                /* theres a strangeness with vt100 tab as it doesn't
                 * seem to erase the next character and seems to be used
                 * (at least by tcsh) to move the cursor one to the right.
                 * So catch this special case of one character move.
                 * NOTE the previous else if checked there is another char.
                 */
                p1++;
                curOff++;
                break;
            }
            if(ipipe->flag & meIPIPE_ANSICOLOR)
                cbuff[p1-buff] = ipipe->ansiCc;
            *p1++ = cc;
            if(isDisplayable(cc))
                curOff++;
            else if(cc == meCHAR_TAB)
                curOff += get_tab_pos(curOff, bp->tabWidth) + 1;
            else if (cc  < 0x20)
                curOff += 2;
            else
                curOff += 4;
            len++;
        }
    }
    ipipeStoreInputPos();
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
        curOff = ipipe->exitCode;
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
            sprintf((char *)resultStr,"%d",curOff);
        }
        execBufferFunc(bp,bp->ipipeFunc,(meEBF_ARG_GIVEN|meEBF_USE_B_DOT|meEBF_HIDDEN),(ii >= 0));
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
    noRows = wp->textDepth;
    noCols = wp->textWidth;
    if(bp->windowCount > 1)
    {
        /* buffer is displayed in more than one window, we need to be careful to avoid them fighting */
        meWindow *ww;
        meFrameLoopBegin();
        ww = loopFrame->windowList;
        while(ww != NULL)
        {
            /* If the window position matches the buffer then re-center */
            if(ww->buffer == bp)
            {
                if(ww->textDepth > noRows)
                    noRows = ww->textDepth;
                if(ww->textWidth > noCols)
                    noCols = ww->textWidth;
            }
            ww = ww->next;
        }
        meFrameLoopEnd();
    }
    if(meModeTest(bp->mode,MDWRAP))
        noCols = noCols-1;
    else if((noCols = ipipe->noCols) == 0)
    {
        meUByte *ss;
        if(((ss=getUsrVar((meUByte *)"ipipe-width")) == NULL) || ((noCols=((meShort) meAtoi(ss))) <= 0) || (noCols > meBUF_SIZE_MAX - 2))
            noCols = meBUF_SIZE_MAX - 2;
    }
    if((ipipe->noRows != noRows) || (ipipe->noCols != noCols))
    {
        ii = ((int) noRows) - ((int) ipipe->noRows);
        ipipe->noRows = noRows;
        ipipe->noCols = noCols;
        
        if(ipipe->pid > 0)
        {
            if(ii > 0)
            {
                if((ipipe->curRow += ii) > bp->lineCount)
                    ipipe->curRow = (meShort) bp->lineCount;
            }
            else if(ipipe->curRow >= ipipe->noRows)
                ipipe->curRow = ipipe->noRows-1;
            /* Check the window is displaying this buffer before we
             * mess with the window settings */
            if((wp->buffer == bp) && meModeTest(bp->mode,MDLOCK))
            {
                if (wp->dotLineNo < ipipe->curRow)
                    wp->vertScroll = 0;
                else
                    wp->vertScroll = wp->dotLineNo-ipipe->curRow;
                wp->updateFlags |= WFMOVEL;
            }
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
            if(ipipeTermCol == NULL)
                ipipeTermCol = (system("tput -T ansi longname > /dev/null 2>&1")) ? ipipeTermSys:(meUByte *) "TERM=ansi";
            term = ipipeTermCol;
        }
        else
            term = ipipeTermSys;
    }
    
    /* Allocate a pseudo terminal to do the work */
    if(((flags & LAUNCH_NOPTY) == 0) && ((ptyFp=allocatePty(line)) >= 0))
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
        if(flags & LAUNCH_USEPTY)
            meModeSet(globMode,MDPTY);
        else
            meModeClear(globMode,MDPTY);
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
    /* setup the buffer */
    if(flags & LAUNCH_BUFIPIPE)
        bp->ipipeFunc = ipipeFunc;
    bp->fileName = meStrdup(path);
    if((flags & LAUNCH_RAW) == 0)
    {
        meStrcpy(line,"cd ");
        meStrcat(line,path);
        addLineToEob(bp,line);
        addLineToEob(bp,comStr);
        addLineToEob(bp,(meUByte *)"\n");
    }
    bp->dotLine = meLineGetPrev(bp->baseLine);
    bp->dotOffset = 0;
    bp->dotLineNo = bp->lineCount-1;
    meAnchorSet(bp,'I',bp->dotLine,bp->dotLineNo,bp->dotOffset,1);

    /* Set up the window dimensions - default to having auto wrap */
    ipipe->flag = (flags & (LAUNCH_RAW|LAUNCH_USEPTY|LAUNCH_ANSICOLOR));
#if MEOPT_EXTENDED
    if(((meSystemCfg & meSYSTEM_IO_UTF8) == 0) || (flags & (LAUNCH_NOUTF8|LAUNCH_RAW)))
        ipipe->flag |= meIPIPE_NOUTF8;
#endif
    ipipe->ansiCc = 'A';
    ipipe->ansiFg = 0;
    ipipe->ansiBg = 0;
    ipipe->ansiSt = 0;
    ipipe->strRow = 0;
    ipipe->strCol = 0;
    ipipe->noRows = 0;
    ipipe->noCols = 0;
    ipipe->curRow = (meShort) bp->dotLineNo;
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
