/*****************************************************************************
*
*	Title:		%M%
*
*	Synopsis:	Spawning process.
*
******************************************************************************
*
*	Filename:		%P%
*
*	Author:			Danial Lawrence
*
*	Creation Date:		10/05/91 08:27
*
*	Modification date:	%G% : %U%
*
*	Current rev:		%I%
*
*	Special Comments:
*
*	Contents Description:	Spawn:	various system access commands
*					for MicroEMACS
*
*****************************************************************************
*
* Modifications to the original file by Jasspa.
*
* Copyright (C) 1988 - 2000, JASSPA
* The MicroEmacs Jasspa distribution can be copied and distributed freely for
* any non-commercial purposes. The MicroEmacs Jasspa Distribution can only be
* incorporated into commercial software with the expressed permission of
* JASSPA.
*
****************************************************************************/

/*---	Include defintions */

#define	__SPAWNC			/* Name the file */

/*---	Include files */

#include "emain.h"

#ifdef _UNIX
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

#ifdef _SUNOS_X86
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
#endif

#if ((defined(_UNIX)) && (!defined(WEXITSTATUS)))
#define WEXITSTATUS(status) ((int)(WIFEXITED(status)?(((*((int *)(&status)))>>8)&0xff) : -1))
#endif /* WIFEXITSTATUS */

#ifdef _DOS
#include	<process.h>
#endif

/*---	Local macro definitions */

/*---	Local type definitions */

/*---	Local variable definitions */

int	mlfirst = -1;		/* first command used by mlreplt() */

#ifdef _UNIX
static uint8 *
getShellCmd(void)
{
    static uint8 *shellCmd=NULL ;
    if(shellCmd == NULL)
    {
        uint8 *cp;
        if(((cp = meGetenv("SHELL")) == NULL) || (*cp == '\0'))
            cp = (uint8 *)"/bin/sh" ;
        shellCmd = meStrdup(cp) ;
    }
    return shellCmd ;
}
#endif

/*
 * Create a temporary name for any command spawn files. On windows & dos
 * concatinate the $TEMP variable with the filename to create the new
 * temporary file name.
 *
 * Under unix create /tmp/me<pid><name>
 * Otherwise create $TEMP/<name>
 * On non-unix systems an extension can be passed in, see macro def in
 * eextrn.h
 */
#ifdef _UNIX
void
__mkTempName (uint8 *buf, uint8 *name)
{
    /* Under UNIX use the process id in order to create the temporary file
     * for this session of me
     */
    if(name != NULL)
        sprintf((char *)buf, "/tmp/me%d%s",(int) getpid(),name);
    else
    {
        strcpy((char *)buf, "/tmp/meXXXXXX") ;
        mktemp((char *)buf) ;
    }
}

#else

void
__mkTempName (uint8 *buf, uint8 *name, uint8 *ext)
{
#ifdef _CONVDIR_CHAR
#define PIPEDIR_CHAR _CONVDIR_CHAR
#else
#define PIPEDIR_CHAR DIR_CHAR
#endif
    static uint8 *tmpDir=NULL ;
    uint8 *pp ;
    int ii ;

    if(tmpDir == NULL)
    {
        /* Get location of the temporary directory from the environment $TEMP */
        if ((pp = (uint8 *) meGetenv ("TEMP")) != NULL)
            tmpDir = malloc((ii=meStrlen(pp))+2) ;
        if(tmpDir != NULL)
        {
            meStrcpy(tmpDir,pp) ;
            if(tmpDir[ii-1] != PIPEDIR_CHAR)
            {
                tmpDir[ii++] = PIPEDIR_CHAR ;
                tmpDir[ii]   = '\0' ;
            }
        }
        else
#if (defined _DOS) || (defined _WIN32)
            /* the C drive is more reliable than ./ as ./ could be on a CD-Rom etc */
            tmpDir = "c:\\" ;
#else
            tmpDir = "./" ;
#endif
    }
    if(ext == NULL)
        ext = "" ;

    if(name != NULL)
        sprintf((char *)buf, "%sme%s%s",tmpDir,name,ext) ;
    else
    {
        for(ii=0 ; ii<999 ; ii++)
        {
            sprintf((char *)buf, "%smetmp%d%s",tmpDir,ii,ext) ;
            if(meTestExist(buf))
                break ;
        }

    }
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
#ifdef _WIN32
    return (WinLaunchProgram (NULL, LAUNCH_SHELL, NULL, NULL, NULL));
#endif
#ifdef _DOS
    TTclose();
    if ((cp=meGetenv("COMSPEC")) == NULL)
        system("command.com");
    else
        system(cp);
    TTopen();
    sgarbf = TRUE;
    return(TRUE);
#endif
#ifdef _UNIX
#ifdef _XTERM
    if(!(meSystemCfg & meSYSTEM_CONSOLE))
    {
        switch(fork())
        {
        case 0:
            /* we want the children to die on interrupt */
            execlp("xterm", "xterm", "-sl", "200", "-sb", NULL);
            mlwrite(MWABORT,(uint8 *)"exec failed, %s", sys_errlist[errno]);
            meExit(127);
        case -1:
            return mlwrite(MWABORT,(uint8 *)"exec failed, %s", sys_errlist[errno]);
        default:
            return TRUE ;
        }
    }
#endif
    TTclose();				/* stty to old settings */
    system((char *)getShellCmd()) ;
    sgarbf = TRUE;
    TTopen();
    return(TRUE);
#endif
}

#ifdef _UNIX
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
    if((f==FALSE) && (mlyesno((uint8 *)"Suspend") != TRUE))
        return FALSE ;

    TTclose();				/* stty to old settings */
    kill(getpid(), SIGTSTP);
    TTopen();
    sgarbf = TRUE;

    return TRUE ;
}

#endif


/* Note: the given string cmdstr must be large enough to strcat
 * " </dev/null" on the end */
int
doShellCommand(uint8 *cmdstr)
{
    uint8 path[FILEBUF] ;		/* pathfrom where to execute */
    int  systemRet ;                    /* return value of last system  */
    int  cd, ss ;
#ifdef _UNIX
    meWAIT_STATUS ws;
#endif

    getFilePath(curbp->b_fname,path) ;
    cd = (meStrcmp(path,curdir) && (meChdir(path) != -1)) ;

#ifdef _WIN32
    ss = WinLaunchProgram(cmdstr,LAUNCH_SYSTEM, NULL, NULL, &systemRet) ;
#else
#ifdef _UNIX
    /* if no data is piped in then pipe in /dev/null */
    if(meStrchr(cmdstr,'<') == NULL)
        meStrcat(cmdstr," </dev/null") ;
    ss = system((char *)cmdstr) ;
    ws = (meWAIT_STATUS)(ss);
    if(WIFEXITED(ws))
    {
        systemRet = WEXITSTATUS(ws) ;
        ss = TRUE ;
    }
    else
    {
        systemRet = -1 ;
        ss = FALSE ;
    }
#else
    systemRet = system(cmdstr) ;
#ifdef _DOS
    /* dos is naughty with modes, a system call could call a progam that
     * changes the screen stuff under our feet and not restore the current
     * mode! The only thing we can do is call TTopen to ensure we restore
     * our mode.
     * We might check all states, but with hidden things like flashing etc.
     * its not worth the effort - sorry, you do it if you want.
     */
    TTopen() ;
#endif
    ss = (systemRet < 0) ? FALSE:TRUE ;
#endif
#endif

    if(cd)
        meChdir(curdir) ;
    meStrcpy(resultStr,meItoa(systemRet)) ;
    return ss ;
}

int
meShellCommand(int f, int n)
{
    uint8 cmdstr[MAXBUF+20];		/* string holding command to execute */

    /* get the line wanted */
    if((mlreply((uint8 *)"System", 0, 0, cmdstr, MAXBUF)) != TRUE)
        return ABORT ;
    return doShellCommand(cmdstr) ;
}

#ifdef _IPIPES

/*
 *---	Interactive PIPE into the list buffer.
 */
#ifdef _WIN32

static BOOL CALLBACK
closeChildConsole(HWND hwnd, long lipipe)
{
    DWORD process ;
    IPIPEBUF *ipipe = (IPIPEBUF *)(lipipe);

    GetWindowThreadProcessId (hwnd,&process);
    if (process == ipipe->dwProcessId)
    {
        PostMessage(hwnd,WM_CLOSE,999,0) ;
        ipipe->pid = -3 ;
        return FALSE ;
    }
    /* keep looking */
    return TRUE ;
}
#endif


static void
ipipeKillBuf(IPIPEBUF *ipipe)
{

    if(ipipe->pid > 0)
    {
#ifdef _WIN32
        /* first try to kill by sending a CLOSE message to the console */
        if(ipipe->dwProcessId != 0)
            EnumWindows(closeChildConsole,(long) ipipe) ;
        if(ipipe->pid > 0)
        {
            /* cannot terminate via the console - we can only Terminate */
            TerminateProcess(ipipe->hProcess,999) ;
            /* On windows theres no child signal, so flag as killed */
            ipipe->pid = -5 ;
        }
        /* Close the process */
        CloseHandle(ipipe->hProcess);
#else
        kill(0-ipipe->pid,SIGKILL) ;
#endif
    }
}

int
ipipeKill(int f, int n)
{
    IPIPEBUF *ipipe ;

    if(!meModeTest(curbp->b_mode,MDPIPE))
    {
        TTbell() ;
        return FALSE ;
    }
    ipipe = ipipes ;
    while(ipipe->bp != curbp)
        ipipe = ipipe->next ;
    ipipeKillBuf(ipipe) ;
    return TRUE ;
}

void
ipipeRemove(IPIPEBUF *ipipe)
{
#ifndef _WIN32
    meSigHold() ;
#endif
    if(ipipe->pid > 0)
        ipipeKillBuf(ipipe) ;

    if(ipipe == ipipes)
        ipipes = ipipe->next ;
    else
    {
        IPIPEBUF *pp ;

        pp = ipipes ;
            while(pp->next != ipipe)
                pp = pp->next ;
        pp->next = ipipe->next ;
    }
    noIpipes-- ;
    if(ipipe->bp != NULL)
    {
        meModeClear(ipipe->bp->b_mode,MDPIPE) ;
        meModeClear(ipipe->bp->b_mode,MDLOCK) ;
    }
#ifdef _WIN32
    CloseHandle(ipipe->rfd) ;
    CloseHandle(ipipe->outWfd) ;
#else
    close(ipipe->rfd) ;
    if(ipipe->rfd != ipipe->outWfd)
        close(ipipe->outWfd) ;
    meSigRelease() ;
#endif
    free(ipipe) ;
}

#ifdef _WIN32

static int
readFromPipe(IPIPEBUF *ipipe, int nbytes, uint8 *buff)
{
    DWORD  bytesRead ;

    /* See if process has ended first */
    if(ipipe->pid < 0)
        return ipipe->pid ;
#ifdef _CLIENTSERVER
    if(ipipe->pid == 0)
    {
        if(ttServerToRead == 0)
            return 0 ;
        if(nbytes > ttServerToRead)
            nbytes = ttServerToRead ;
        if(ReadFile(ipipe->rfd,buff,nbytes,&bytesRead,NULL) == 0)
            return -1 ;
        return (int) bytesRead ;
    }
#endif
    /* Must peek on a pipe cos if we try to read too many this will fail */
    if((PeekNamedPipe(ipipe->rfd, (LPVOID) NULL, (DWORD) 0,
                      (LPDWORD) NULL, &bytesRead, (LPDWORD) NULL) != TRUE) ||
       (bytesRead <= 0))
        return 0 ;
    if(bytesRead > (DWORD) nbytes)
        bytesRead = (DWORD) nbytes ;
    if(ReadFile(ipipe->rfd,buff,bytesRead,&bytesRead,NULL) == 0)
        return -1 ;
    return (int) bytesRead ;
}

#else

#ifdef _CLIENTSERVER

#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <netinet/in.h>

static int
readFromPipe(IPIPEBUF *ipipe, int nbytes, uint8 *buff)
{
    int ii ;
    if(ipipe->pid == 0)
    {
        if ((ii = recv(ipipe->rfd,(char *) buff,nbytes,0)) < 0)
            ii = 0 ;
    }
    else
        ii = read(ipipe->rfd,buff,nbytes) ;
#if 0
    if(ii > 0)
    {
        static FILE *fplog=NULL ;
        if(fplog == NULL)
            fplog = fopen("log","wb+") ;
        fwrite(buff,1,ii,fplog) ;
    }
#endif
    return ii ;

}


#else

#define readFromPipe(ipipe,nbytes,buff) read(ipipe->rfd,buff,nbytes)

#endif

#endif

#define getNextCharFromPipe(ipipe,cc,rbuff,curROff,curRRead)                 \
((curROff < curRRead) ?                                                      \
 ((cc=rbuff[curROff++]), 1):                                                 \
 (((curRRead=readFromPipe(ipipe,MAXBUF,rbuff)) > 0) ?                        \
  ((cc=rbuff[0]),curROff=1): 0))


#define ipipeStoreInputPos()                                                 \
do {                                                                         \
    LINE *lp_new ;                                                           \
    noLines += addLine(lp_old,buff) ;                                        \
    lp_new = lback(lp_old) ;                                                 \
    if(lp_old != bp->b_linep)                                                \
    {                                                                        \
        noLines-- ;                                                          \
        lp_new->l_fp = lp_old->l_fp ;                                        \
        lp_old->l_fp->l_bp = lp_new ;                                        \
        if(lp_old->l_flag & LNMARK)                                          \
            lunmarkBuffer(bp,lp_old,lp_new);                                 \
        meFree(lp_old);                                                      \
    }                                                                        \
    else                                                                     \
    {                                                                        \
        bp->line_no-- ;                                                      \
        ipipe->curRow-- ;                                                    \
    }                                                                        \
    bp->line_no += noLines ;                                                 \
    bp->elineno += noLines ;                                                 \
    ipipe->curRow = curRow ;                                                 \
    bp->topLineNo = bp->line_no-curRow ;                                     \
    bp->b_dotp = lp_new ;                                                    \
    bp->b_doto = p1 - buff ;                                                 \
    bufferPosUpdate(bp,noLines,bp->b_doto) ;                                 \
} while(0)


#define IPIPE_OVERWRITE  0x01

void
ipipeRead(IPIPEBUF *ipipe)
{
    BUFFER *bp=ipipe->bp ;
    LINE   *lp_old ;
    int     len, curOff, maxOff, curRow, ii ;
    uint32  noLines ;
    uint8  *p1, cc, buff[1025] ;
    uint8   rbuff[MAXBUF] ;
    int     curROff=0, curRRead=0 ;
#if _UNIX
    int     na, nb ;
#endif

    if(meModeTest(bp->b_mode,MDWRAP))
        maxOff = TTncol - 2 ;
    else
        maxOff = MAXBUF - 2 ;
#ifdef _UNIX
    meSigHold() ;
#ifdef _CLIENTSERVER
    if(ipipe->pid == 0)
    {
        struct sockaddr_un cssa ;	/* for unix socket address */

        ii = sizeof(struct sockaddr_un);
        cssa.sun_family = AF_UNIX;
        ipipe->rfd = accept(ipipe->rfd,(struct sockaddr *)&cssa, (void *)&ii) ;
    }
#endif
#endif
    alphaMarkGet(bp,'I') ;
    if(meModeTest(bp->b_mode,MDLOCK))
    {
        /* Work out which windows are locked to the current cursor position */
        WINDOW *wp = wheadp;

        while(wp != NULL)
        {
            if((wp->w_bufp == bp) &&
               (wp->w_dotp == bp->b_dotp) &&
               (wp->w_doto == bp->b_doto))
                break ;
            wp = wp->w_wndp;
        }
        if(wp == NULL)
            bp->intFlag &= ~BIFLOCK ;
        else
            bp->intFlag |= BIFLOCK ;
    }
    /* This is a quick sanity check which is needed if the buffer has
     * been changed by something. If curRow becomes greater than line_no
     * the topLineNo becomes negative and things go wrong.
     * Discovered problem when using gdb mode as the gdb input handler
     * kills ^Z^Z lines making curRow > line_no.
     */
    if((curRow=ipipe->curRow) > bp->line_no)
        curRow = bp->line_no ;
    len = bp->b_doto ;
    lp_old = bp->b_dotp ;
    bufferPosStore(lp_old,bp->b_doto,bp->line_no) ;
    meStrcpy(buff,lp_old->l_text) ;
    p1 = buff+len ;
    noLines = 0 ;
    curOff = getcol(buff,len) ;
    for(;;)
    {
        if(!getNextCharFromPipe(ipipe,cc,rbuff,curROff,curRRead))
        {
            uint8 *ins ;

            if(ipipe->pid == -5)
                ins = (uint8 *)"[TERMINATED]" ;
            else if(ipipe->pid == -4)
                ins = (uint8 *)"[EXIT]" ;
            else if(ipipe->pid == -3)
                ins = (uint8 *)"[KILLED]" ;
            else if(ipipe->pid == -2)
                ins = (uint8 *)"[CORE DUMP]" ;
            else
                /* none left to read */
                break ;
            ipipe->pid = -1 ;
            meStrcpy(p1,ins) ;
            cc = meNLCHAR ;
        }
        switch(cc)
        {
        case 7:
            TTbell() ;
            break ;
        case 8:
            if(p1 != buff)
            {
                p1-- ;
                len-- ;
                curOff = getcol(buff,len) ;
            }
            break ;
        case '\r':
            p1 = buff ;
            len = curOff = 0 ;
            break ;
        case meNLCHAR:
#if _UNIX
            if(!(ipipe->flag & IPIPE_OVERWRITE) && (curRow+1 < ipipe->noRows))
            {
                /* if in over-write mode and not at the bottom, move instead */
                nb = curRow + 1 ;
                na = 0 ;
                goto move_cursor_pos ;
            }
#endif
            ii = addLine(lp_old,buff) ;
            noLines += ii ;
            if(curRow < ipipe->noRows-1)
                curRow += ii ;
            p1 = buff ;
            *p1 = '\0' ;
            len = curOff = 0 ;
            break ;
#if _UNIX
        case 27:
            if(getNextCharFromPipe(ipipe,cc,rbuff,curROff,curRRead))
            {
                int gotN=0, gotQ=0 ;

                na=0 ;
                nb=-1 ;
                if(cc == '[')
                {
get_another:
                    if(getNextCharFromPipe(ipipe,cc,rbuff,curROff,curRRead))
                    {
                        if(isDigit(cc))
                        {
                            gotN |= 1 ;
                            na = na*10 + (cc - '0') ;
                            goto get_another ;
                        }
                        switch(cc)
                        {
                        case ';':
                            gotN |= 2 ;
                            nb = na ;
                            na = 0 ;
                            goto get_another ;
                        case '?':
                            gotQ = 1 ;
                            goto get_another ;
                        case '@':
                            if(!gotN)
                                na = 1 ;
                            nb = meStrlen(p1) ;

                            do
                                p1[nb+na] = p1[nb] ;
                            while(--nb >= 0) ;
                            len += na ;
                            memset(p1,' ',na) ;
                            break ;
                        case 'C':
                            if(!gotN)
                                na = 1 ;
                            if((na + len) >= ipipe->noCols)
                                na = ipipe->noCols - len - 1 ;
                            nb = na - meStrlen(p1) ;
                            p1 += na ;
                            len += na ;
                            if(nb > 0)
                            {
                                memset(p1-nb,' ',nb) ;
                                *p1 = '\0' ;
                            }
                            curOff = getcol(buff,len) ;
                            break ;
                        case 'D':
                            if(!gotN)
                                na = 1 ;
                            if(len < na)
                                na = len ;
                            p1 -= na ;
                            len -= na ;
                            curOff = getcol(buff,len) ;
                            break ;
                        case 'A':
                            if(!gotN)
                                na = 1 ;
                            nb = curRow - na ;
                            na = len ;
                            goto move_cursor_pos ;

                        case 'B':
                            if(!gotN)
                                na = 1 ;
                            nb = curRow + na ;
                            na = len ;
                            goto move_cursor_pos ;

                        case 'H':
                            if(gotN & 2)
                                na-- ;
                            else
                            {
                                nb = na ;
                                na = 0 ;
                            }
                            if(gotN & 1)
                                nb-- ;
                            else
                                nb = 0 ;
move_cursor_pos:
                            if(nb < 0)
                                nb = 0 ;
                            else if(nb >= ipipe->noRows)
                                nb = ipipe->noRows - 1 ;
                            if(na < 0)
                                na = 0 ;
                            else if(na >= ipipe->noCols)
                                na = ipipe->noCols - 1 ;
                            ipipeStoreInputPos() ;
                            bp->line_no += nb - curRow  ;
                            lp_old = bp->b_dotp ;
                            if(nb > curRow)
                            {
                                while((curRow != nb) && (lp_old != bp->b_linep))
                                {
                                    curRow++ ;
                                    lp_old = lforw(lp_old) ;
                                }
                                while(curRow != nb)
                                {
                                    curRow++ ;
                                    addLineToEob(bp,(uint8 *)"") ;
                                }
                            }
                            else
                            {
                                while(curRow != nb)
                                {
                                    curRow-- ;
                                    lp_old = lback(lp_old) ;
                                }
                            }
                            len = na ;
                            bp->b_dotp = lp_old ;
                            meStrcpy(buff,lp_old->l_text) ;
                            bufferPosStore(lp_old,(uint16)len,bp->line_no) ;
                            na -= meStrlen(buff) ;
                            p1 = buff+len ;
                            if(na > 0)
                            {
                                memset(p1-na,' ',na) ;
                                *p1 = '\0' ;
                            }
                            curOff = getcol(buff,len) ;
                            noLines = 0 ;
                            break ;

                        case 'h':
                            if(na != 4)
                                goto cant_handle_this ;
                            ipipe->flag |= IPIPE_OVERWRITE ;
                            break ;
                        case 'l':
                            if(na != 4)
                                goto cant_handle_this ;
                            ipipe->flag &= ~IPIPE_OVERWRITE ;
                            break ;
                        case 'm':
                            /* These are font changes, like bold etc.
                             * We won't do anything with them for now
                             */
                            break ;
                        case 'n':
                            {
                                char outb[20] ;

                                if(na != 6)
                                    goto cant_handle_this ;

                                sprintf(outb,"\033[%d;%dR",curRow,len) ;
                                write(ipipe->outWfd,outb,strlen(outb)) ;
                                break ;
                            }
                        case 'J':
                            {
                                LINE *lp ;

                                lp = lp_old ;
                                if(na == 2)
                                {
                                    for(ii=curRow ; ii>0 ; ii--)
                                        lp = lback(lp) ;
                                    memset(buff,' ',meStrlen(buff)) ;
                                }
                                else if(na == 0)
                                {
                                    memset(buff+len,' ',meStrlen(buff+len)) ;
                                    if(lp != bp->b_linep)
                                        lp = lforw(lp) ;
                                }
                                else
                                    goto cant_handle_this ;
                                while(lp != bp->b_linep)
                                {
                                    memset(lp->l_text,' ',llength(lp)) ;
                                    lp->l_flag |= LNCHNG ;
                                    lp = lforw(lp) ;
                                }
                                curOff = len ;
                                break ;
                            }
                        case 'K':
                            *p1 = '\0' ;
                            break ;
                        case 'P':
                            if(!gotN)
                                na = 1 ;
                            if(((int) meStrlen(p1)) > na)
                                meStrcpy(p1,p1+na) ;
                            else
                                *p1 = '\0' ;
                            break ;
                        default:
cant_handle_this:
#ifndef NDEBUG
                            if(nb < 0)
                                printf("Don't cope with term code \\E[%s%d%c\n",
                                       (gotQ) ? "?":"",na,cc) ;
                            else
                                printf("Don't cope with term code \\E[%s%d;%d%c\n",
                                       (gotQ) ? "?":"",nb,na,cc) ;
#endif
                            break ;
                        }
                    }
                    break ;
                }
                else if(cc == '7')
                {
                    ipipe->strRow = curRow ;
                    ipipe->strCol = len ;
                }
                else if(cc == '8')
                {
                    nb = ipipe->strRow ;
                    na = ipipe->strCol ;
                    goto move_cursor_pos ;
                }
            }
            /* follow through */
#endif
        default:
            if(curOff >= maxOff)
            {
                uint8 bb[2] ;
                bb[0] = p1[0] ;
                bb[1] = p1[1] ;
                p1[0] = '\\' ;
                p1[1] = '\0' ;
                ii = addLine(lp_old,buff) ;			/* Add string */
                noLines += ii ;
                if(curRow < ipipe->noRows-1)
                    curRow += ii ;
                p1[0] = bb[0] ;
                p1[1] = bb[1] ;
                meStrcpy(buff,p1) ;
                p1 = buff ;
                len = curOff = 0 ;
            }
            if(ipipe->flag & IPIPE_OVERWRITE)
            {
                uint8 *ss, *dd ;
                int ll ;

                ll = meStrlen(p1) ;
                ss = p1 + ll ;
                dd = ss+1 ;
                do
                    *dd-- = *ss-- ;
                while(ll--) ;
            }
            else if(*p1 == '\0')
                p1[1] = '\0' ;
            else if((cc == TAB) && (get_tab_pos(curOff) == 0))
            {
                /* theres a strangeness with vt100 tab as it doesn't
                 * seem to erase the next character and seems to be used
                 * (at least by tcsh) to move the cursor one to the right.
                 * So catch this special case of one character move.
                 * NOTE the previous else if checked there is another char.
                 */
                p1++ ;
                curOff++ ;
                break ;
            }
            *p1++ = cc ;
            if(isDisplayable(cc))
                curOff++ ;
            else if(cc == TAB)
                curOff += get_tab_pos(curOff) + 1 ;
            else if (cc  < 0x20)
                curOff += 2 ;
            else
                curOff += 4 ;
            len++ ;
        }
    }
    ipipeStoreInputPos() ;
#ifdef _UNIX
#ifdef _CLIENTSERVER
    /* the unix client server trashed the rfd at the top of this function due
     * to the way sockets are handled. But the read handle is the same as the write
     * so its trivial to restore */
    if(ipipe->pid == 0)
        ipipe->rfd = ipipe->outWfd ;
#endif
#endif
    if((ii=ipipe->pid) < 0)
        ipipeRemove(ipipe) ;
#if ((defined (_UNIX)) && (defined (_POSIX_SIGNALS)))
    /* as soon as the BLOCK of sigchld is removed, if the process has finished
     * while reading then it will get registered now, this fact has to be taken
     * into acount and handled carefully. If not typically the [EXIT] line
     * will be missed if signals still blocked during the execution of the ipipe
     * function which leads to side effects like poor cursor blink etc.
     */
    meSigRelease() ;
#endif
    alphaMarkSet(bp,'I',bp->b_dotp,bp->b_doto,1) ;
    if(bp->ipipeFunc >= 0)
        /* If the process has ended the argument will be 0, else 1 */
        execBufferFunc(bp,bp->ipipeFunc,(meEBF_ARG_GIVEN|meEBF_HIDDEN),(ii >= 0)) ;
    update(FALSE) ;
}


int
ipipeWriteString(int f, int n)
{
    IPIPEBUF *ipipe ;
    int       ss ;
    uint8     buff[MAXBUF];	/* string to add */

    if(!meModeTest(curbp->b_mode,MDPIPE))
        return mlwrite(MWABORT,(uint8 *)"[Not an ipipe-buffer]") ;
    /* ask for string to insert */
    if((ss=mlreply((uint8 *)"String", 0, 0, buff, MAXBUF)) != TRUE)
        return ss ;

    ipipe = ipipes ;
    while(ipipe->bp != curbp)
        ipipe = ipipe->next ;
    while(n--)
    {
#ifdef _WIN32
        DWORD written ;
        WriteFile(ipipe->outWfd,buff,meStrlen(buff),&written,NULL) ;
#else
        write(ipipe->outWfd,buff,meStrlen(buff)) ;
#endif
    }
    return TRUE ;
}

int
ipipeSetSize(WINDOW *wp, BUFFER *bp)
{
    IPIPEBUF *ipipe ;

    ipipe = ipipes ;
    while(ipipe->bp != bp)
        ipipe = ipipe->next ;
    if(((ipipe->noRows != wp->numTxtRows) ||
        (ipipe->noCols != wp->numTxtCols-1) ) &&
       (ipipe->pid > 0))
    {
        int ii ;
        ii = wp->numTxtRows - ipipe->noRows ;
        ipipe->noRows = wp->numTxtRows ;
        ipipe->noCols = wp->numTxtCols-1 ;
        if(ii > 0)
        {
            if((ipipe->curRow += ii) > bp->elineno)
                ipipe->curRow = (int16) bp->elineno ;
        }
        else if(ipipe->curRow >= ipipe->noRows)
            ipipe->curRow = ipipe->noRows-1 ;
        /* Check the window is displaying this buffer before we
         * mess with the window settings */
        if((wp->w_bufp == bp) && meModeTest(bp->b_mode,MDLOCK))
        {
            if (wp->line_no < ipipe->curRow)
                wp->topLineNo = 0;
            else
                wp->topLineNo = wp->line_no-ipipe->curRow ;
            wp->w_flag |= WFMOVEL ;
        }
#ifdef _UNIX
#if ((defined(TIOCSWINSZ)) || (defined(TIOCGWINSZ)))
        {
            /* BSD-style.  */
            struct winsize size;

            size.ws_col = ipipe->noCols ;
            size.ws_row = ipipe->noRows ;
            size.ws_xpixel = size.ws_xpixel = 0 ;
#ifdef TIOCSWINSZ
            ioctl(ipipe->outWfd,TIOCSWINSZ,&size) ;
#else
            ioctl(ipipe->outWfd,TIOCGWINSZ,&size) ;
#endif
            kill(ipipe->pid,SIGWINCH) ;
        }
#else
#ifdef TIOCGSIZE
        {
            /* SunOS - style.  */
            struct ttysize size;

            size.ts_col = ipipe->noCols ;
            size.ts_lines = ipipe->noRows ;
            ioctl(ipipe->outWfd,TIOCSSIZE,&size) ;
            kill(ipipe->pid,SIGWINCH) ;
        }
#endif /* TIOCGSIZE */
#endif /* TIOCSWINSZ/TIOCGWINSZ */
#endif /* _UNIX */
    }
    return TRUE ;
}

#ifdef _UNIX
/* allocatePty; Allocate a pty. We use the old BSD method of searching for a
 * pty, once we have aquired one then we look for the tty. Return the name of
 * the tty to the caller so that it may be opened. */
static int
allocatePty(uint8 *ptyName)
{
    struct stat stb ;
    int    fd ;
#ifdef _IRIX
    {
/*        struct sigaction ocstat, cstat;*/
        char * name;
/*        sigemptyset(&cstat.sa_mask);*/
/*        cstat.sa_handler = SIG_DFL;*/
/*        cstat.sa_flags = 0;*/
/*        sigaction(SIGCLD, &cstat, &ocstat);*/
        name = _getpty(&fd,O_RDWR,0600,0);
/*        sigaction(SIGCLD, &ocstat, (struct sigaction *)0);*/
        if(name == NULL)
            return -1 ;
        if((fd >=  0) && (fstat (fd, &stb) >= 0))
        {
            /* Return the name of the tty and the file descriptor of the pty */
            meStrcpy (ptyName, name);
            return fd ;
        }
    }
#else
    register int c, ii ;

    /* Some systems name their pseudoterminals so that there are gaps in
       the usual sequence - for example, on HP9000/S700 systems, there
       are no pseudoterminals with names ending in 'f'.  So we wait for
       three failures in a row before deciding that we've reached the
       end of the ptys.  */
    int failed_count = 0;

    for (c ='p' ; c <= 'z' ; c++)
    {
        for (ii=0 ; ii<16 ; ii++)
        {
#ifdef _HPUX
            sprintf((char *)ptyName,"/dev/ptym/pty%c%x", c, ii);
#else
            sprintf((char *)ptyName,"/dev/pty%c%x",c,ii);
#endif
/*            printf("Im trying [%s]\n",ptyName) ;*/
            if(stat((char *)ptyName,&stb) < 0)
            {
                /* Cannot open PTY */
/*                printf("Failed\n") ;*/
                failed_count++;
                if (failed_count >= 3)
                    return -1 ;
            }
            else
            {
                /* Found a potential pty */
                failed_count = 0;
                fd = open((char *)ptyName,O_RDWR,0) ;
                if(fd >= 0)
                {
#if 0
                    /* Jon: POSIX - this is what we should be doing */
                    /* Set up the permissions of the slave */
                    if (grantpt (fd) < 0)
                    {
                        close (fd);
                        continue;
                    }
                    /* Unlock the slave */
                    if (unlockpt(fd) < 0)
                    {
                        close (fd);
                        continue;
                    }
                    else
                    {
                        char *p;
                        if ((p = ptsname (fd)) == NULL)
                        {
                            close (fd);
                            continue;
                        }
                        meStrcpy (ptyName, p);
                    }
#endif
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
                        close (fd);
                        continue;
                    }
                    /* setupPty(fd) ;*/
                    return fd ;
                }
                else
                {
                    /* printf("Couldn't open\n") ;*/
                }
            }
        }
    }
#endif
    return -1;
}


/* childSetupTty; Restore the correct terminal settings on the child tty
 * process. We restore the settings from our initial save of the environmet. */
static void
childSetupTty(void)
{
#ifdef _USG
#ifdef _TERMIOS
    extern struct termios otermio ;
    struct termios ntermio ;
#else
    extern struct termio otermio ;
    struct termio ntermio ;
#endif
    ntermio = otermio;

    /* the new ipipe stuff needs the echo to do its terminal output */
    ntermio.c_lflag |= (ECHO|ECHOE|ECHOK) ;

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
    ioctl (0, TIOCSETC, &otchars) ;
#endif
    ioctl (0, TIOCSLTC, &oltchars) ;
#endif /* _BSD */
}
#endif

int
doIpipeCommand(uint8 *comStr, uint8 *path, uint8 *bufName, int flags)
{
    IPIPEBUF    *ipipe ;
    BUFFER      *bp ;
    uint8        line[MAXBUF] ;
    int          cd ;
#ifdef _UNIX
    int          fds[2], outFds[2], ptyFp ;
    int          pid;                   /* Child process identity */
#endif
#ifdef _WIN32
    int          rr ;
#endif
    /* get or create the command buffer */
    if(((bp=bfind(bufName,0)) != NULL) && meModeTest(bp->b_mode,MDPIPE))
    {
        sprintf((char *)line,"%s already active, kill",bufName) ;
        if(mlyesno(line) != TRUE)
            return FALSE ;
    }
    cd = (meStrcmp(path,curdir) && (meChdir(path) != -1)) ;

#ifdef _WIN32
    /* Launch the ipipe */
    if((rr=WinLaunchProgram(comStr,(LAUNCH_IPIPE|flags), NULL, NULL, NULL)) != TRUE)
    {
        if(cd)
            meChdir(curdir) ;
        if(rr == ABORT)
            /* returns ABORT when trying to IPIPE a DOS app on win95 (it doesn't work)
             * Try doPipe instead */
            return doPipeCommand(comStr,path,bufName,flags) ;
        return FALSE;
    }
#else

    /* Allocate a pseudo terminal to do the work */
    if((ptyFp=allocatePty(line)) >= 0)
    {
        fds[0] = outFds[1] = ptyFp ;
#if ((defined _LINUX) || (defined _FREEBSD) || (defined _SUNOS_X86) || (defined _BSD))
        /* On the BSD systems we open the tty prior to the fork. If this is a
         * later POSIX platform then we will expect O_NOCTTY to exist and we
         * open the tty with O_NOCTTY. Do not let this terminal become our
         * controlling tty. This prevents an application from unintentionally
         * aquiring the controlling terminal as a side effect of the open. */
#if (defined O_NOCTTY)
        fds[1] = outFds[0] = open(line,O_RDWR|O_NOCTTY,0) ;
#else
        fds[1] = outFds[0] = open(line,O_RDWR,0) ;
#endif /* O_NOCTTY */
#else
        fds[1] = outFds[0] = -1 ;
#endif /* _LINUX/_FREEBSD/.. */
    }
    else
    {
        /* Could not get a pty use a pipe instead */
        pipe(fds) ;
        pipe(outFds) ;
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
    if (fds[0] > 0)
        fcntl(fds[0],F_SETFL,O_NONBLOCK) ;
    if ((fds[1] > 0) && (ptyFp < 0))
        fcntl(fds[1],F_SETFL,O_NONBLOCK) ;
#else
#ifdef O_NDELAY
    if (fds[0] > 0)
        fcntl(fds[0],F_SETFL,O_NDELAY) ;
    if ((fds[1] > 0) && (ptyFp < 0))
        fcntl(fds[1],F_SETFL,O_NDELAY) ;
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
        uint8 *ss ;

        /* Dissassociate the new process from the controlling terminal */
#if ((defined(_BSD)) && (defined(TIOCNOTTY)))
        /* Under BSD then we allocate a dummy tty and then immediatly shut it.
         * This has the desired effect of dissassociating the terminal */
        if (ptyFp >= 0)
        {
            /* Under BSD 4.2 then we have to break the tty off. We make a
             * dummy call to open a tty and then immediately close it. This
             * was fixed on BSD4.3, but the same technique works so continue
             * to use it !! */
            int tempFp;

            if ((tempFp = open ("/dev/tty", O_RDWR, 0)) >= 0)
            {
                ioctl (tempFp, TIOCNOTTY);
                close (tempFp);
            }
        }
        
        /* Put the process into parent group 0. Note that setsid() does the
         * same job under SVR4. */
        setpgrp (0,0);                  /* BSD */
#else
        /* Under POSIX.1 environments then simply use setsid() to
         * dissassociate from the terminal. This will also sort out the group
         * ID's groups. */
        setsid ();                      /* Disassociate terminal */

        /* Assign the parent group. Old System V has a setpgrp() with no
         * arguments. Newer programs should use setgpid() instead. It's
         * debatable if we actually need this because setsid() might do it,
         * however no harm will come from re-assigning the parent group. */
#ifdef _SVID   
        setpgrp ();                     /* Old System V */
#else
        setpgid (0,0);                  /* Newer UNIX systems */
#endif        
        /* Not sure what the hell this does, why is it here ?? */
#if (defined (TIOCSCTTY) && (defined (_LINUX) || defined (_FREEBSD)))
        if((ptyFp >= 0) && (outFds[0] >= 0))
            ioctl (outFds[0],TIOCSCTTY,0);
#endif
#endif
        /* On BSD systems then we should try and use the new Barkley line
         * disciplines for communication, especially if we are a pty otherwise
         * we will get some problems with the shell. For simple pipes we do
         * not need to bother. */
#if (defined (_BSD) && defined (NTTYDISC) && defined (TIOCSETD))
        if ((ptyFp >= 0) && (outFds[0] >= 0))
        {
            /* Use new line discipline.  */
            int ldisc = NTTYDISC;
            ioctl (outFds[0], TIOCSETD, &ldisc);
        }
#endif /* defined (NTTYDISC) && defined (TIOCSETD) */

        /* The child process has inherited the parent signals. Ensure that all
         * of the signals are reset to their correct default value */
#ifdef _POSIX_SIGNALS
        {
            struct sigaction sa ;

            sigemptyset(&sa.sa_mask) ;
            sa.sa_flags=SA_RESETHAND;
            sa.sa_handler=NULL ;
            sigaction(SIGHUP,&sa,NULL) ;
            sigaction(SIGINT,&sa,NULL) ;
            sigaction(SIGQUIT,&sa,NULL) ;
            sigaction(SIGTERM,&sa,NULL) ;
            sigaction(SIGUSR1,&sa,NULL) ;
            sigaction(SIGALRM,&sa,NULL) ;
            sigaction(SIGCHLD,&sa,NULL) ;

            /* Release any signals that might be blocked */
            sigprocmask(SIG_SETMASK,&sa.sa_mask,NULL);
        }
#else
        signal(SIGHUP,SIG_DFL) ;
        signal(SIGINT,SIG_DFL) ;
        signal(SIGQUIT,SIG_DFL) ;
        signal(SIGTERM,SIG_DFL) ;
        signal(SIGUSR1,SIG_DFL) ;
        signal(SIGALRM,SIG_DFL) ;
        signal(SIGCHLD,SIG_DFL) ;
#ifdef _BSD_SIGNALS
        /* Release any signals that might be blocked */
        sigsetmask (0);
#endif /* _BSD_SIGNALS */
#endif /* _POSIX_SIGNALS  */

#if !((defined _LINUX) || (defined _FREEBSD) || (defined _SUNOS_X86) || (defined _BSD))
        /* Some systems the tty is opened late as here */
        if(ptyFp >= 0)
        {
#ifdef O_NOCTTY
            fds[1] = outFds[0] = open((const char *)line,O_RDWR|O_NOCTTY,0) ;
#else
            fds[1] = outFds[0] = open((const char *)line,O_RDWR,0) ;
#endif /* O_NOCTTY */
        }
#endif /* !_LINUX/_FREEBSD/_SUNOS_X86/_BSD */

        /* On solaris (this is POSIX I believe) then push the line emulation
         * modes */
#if (defined(_SUNOS_X86))
        if(ptyFp >= 0)
        {
#if 0
            /* Push the hardware emulation mode */
            /* JON: This push does work, however it does cause us to loose
             * terminal control with respect to the SIGWINCH calls hence we do
             * not recieve an indication when the window has been re-sized.
             * So we omit this, but have left the code for reference. */
            ioctl (fds[1], I_PUSH, "ptem");
#endif
            /* Push the line discipline module */
            ioctl (fds[1], I_PUSH, "ldterm");
        }
#endif /* _SUNOS_X86 */

        /* Close the existing stdin/out/err */
        close (0);
        close (1);
        close (2);

        /* Duplicate the new descriptors on stdin/out/err */
        dup2 (outFds[0],0);
        dup2 (fds[1],1);
        dup2 (fds[1],2);                /* stdout => stderr */

        /* Dispose of the descriptors */
        close (outFds[0]) ;
        close (fds[1]) ;

        /* Fix up the line disciplines */
        childSetupTty() ;

        /* Set up the arguments for the pipe */
        if((ss=getUsrVar((uint8 *)"ipipe-term")) != NULL)
            mePutenv(meStrdup(ss)) ;
        ss = getShellCmd() ;

        args[0] = (char *) ss ;
        if(meStrcmp(ss,comStr))
        {
            args[1] = "-c" ;
            args[2] = (char *) comStr ;
            args[3] = NULL ;
        }
        else
            args[1] = NULL ;

#ifndef _NOPUTENV
        execv(args[0],args) ;
#else
        /* We need to push the environment variable, however in order to do
         * this then we need to supply the absolute pathname of the
         * executable. Search the $PATH for the executable. */
        if (meEnviron != NULL)
        {
            char buf[MAXBUF];

            if (executableLookup (args[0], buf))
                args[0] = buf;
            execve (args[0], args, meEnviron) ;
        }
        else
            execv(args[0],args) ;
#endif
        exit(1) ;                       /* Should never get here unless we fail */
    }
#endif /* _WIN32 */
    if(cd)
        meChdir(curdir) ;
    if((ipipe = meMalloc(sizeof(IPIPEBUF))) == NULL)
    {
#ifdef _UNIX
        meSigRelease ();
#endif /*_UNIX */
        return FALSE ;
    }
#ifdef _WIN32
    {
        extern IPIPEBUF pipeInfo ;
        /* WinLaunch has a static IPIPEBUF in which it puts all
         * the required info, copy it.
         */
        memcpy(ipipe,&pipeInfo,sizeof(IPIPEBUF)) ;
    }
#else
    ipipe->pid = pid ;
    ipipe->rfd = fds[0] ;
    ipipe->outWfd = outFds[1] ;
#endif /* _WIN32 */
    /* Link in hte ipipe */
    ipipe->next = ipipes ;
    ipipes = ipipe ;
    noIpipes++ ;

#ifdef _UNIX
    /* Release the signals - we can now cope if the child dies. */
    meSigRelease ();
#endif /*_UNIX */

    /* Create the output buffer */
    {
        meMODE sglobMode ;
        meModeCopy(sglobMode,globMode) ;
        meModeSet(globMode,MDWRAP) ;
        meModeSet(globMode,MDPIPE) ;
        meModeSet(globMode,MDLOCK) ;
        meModeClear(globMode,MDUNDO) ;
        bp=bfind(bufName,BFND_CREAT|BFND_CLEAR) ;
        meModeCopy(globMode,sglobMode) ;
    }
    if((ipipe->bp = bp) == NULL)
    {
        ipipeRemove(ipipe) ;
        return mlwrite(MWABORT,(uint8 *)"[Failed to create %s buffer]",bufName) ;
    }
    /* setup the buffer */
    meStrcpy(line,"cd ") ;
    meStrcat(line,path) ;
    addLineToEob(bp,line) ;			/* Add string */
    bp->b_fname = meStrdup(path) ;
    addLineToEob(bp,comStr) ;			/* Add string */
    addLineToEob(bp,(uint8 *)"") ;		/* Add string */
    addLineToEob(bp,(uint8 *)"") ;		/* Add string */
    bp->b_dotp = lback(bp->b_linep) ;
    bp->b_doto = 0 ;
    bp->line_no = bp->elineno-1 ;
    alphaMarkSet(bp,'I',bp->b_dotp,bp->b_doto,1) ;

    /* Set up the window dimensions - default to having auto wrap */
    ipipe->flag = 0 ;
    ipipe->strRow = 0 ;
    ipipe->strCol = 0 ;
    ipipe->noRows = 0 ;
    ipipe->noCols = 0 ;
    ipipe->curRow = (int16) bp->line_no ;
    /* get a popup window for the command output */
    {
        WINDOW *wp ;
        if((flags & LAUNCH_SILENT) || ((wp = wpopup(bufName,0)) == NULL))
            wp = curwp ;
        /* while executing the wpopup function the ipipe could have exited so check */
        if(ipipes == ipipe)
        {
            ipipeSetSize(wp,bp) ;
            if(bp->ipipeFunc >= 0)
                /* Give argument of 1 to indicate process has not exited */
                execBufferFunc(bp,bp->ipipeFunc,(meEBF_ARG_GIVEN|meEBF_HIDDEN),1) ;
        }
    }
    /* reset again incase there was a delay in the wpopup call */
    bp->b_dotp = lback(bp->b_linep) ;
    bp->b_doto = 0 ;
    bp->line_no = bp->elineno-1 ;
    resetBufferWindows(bp) ;

    return TRUE ;
}

int
ipipeCommand(int f, int n)
{
    register int  ss ;			/* Fast variable */
    uint8         lbuf[MAXBUF];		/* command line send to shell */
    uint8         nbuf[MAXBUF], *bn ;	/* buffer name */
    uint8          pbuf[FILEBUF] ;

    if(!(meSystemCfg & meSYSTEM_IPIPES))
    {
        /* No ipipes flagged so just do a normal pipe */
        return pipeCommand(f,n) ;
    }
    /*---	Get the command to pipe in */
    if((ss=mlreply((uint8 *)"Ipipe", 0, 0, lbuf, MAXBUF)) != TRUE)
        return ss ;
    if((n & 0x01) == 0)
    {
        /* prompt for and get the new buffer name */
        if((ss = getBufferName((uint8 *)"Buffer", 0, 0, nbuf)) != TRUE)
            return ss ;
        bn = nbuf ;
    }
    else
        bn = BicommandN ;

    getFilePath(curbp->b_fname,pbuf) ;
    return doIpipeCommand(lbuf,pbuf,bn,(n & 0x1f)) ;
}

#endif


/*
 * Pipe a one line command into a window
 * Bound to ^X @
 */
int
doPipeCommand(uint8 *comStr, uint8 *path, uint8 *bufName, int flags)
{
    register BUFFER *bp;	/* pointer to buffer to zot */
    uint8 line[MAXBUF] ;                /* new com line with "> command" */
    int cd, ret ;
#ifdef _DOS
    static int8 pipeStderr=0 ;
    uint8 cc, *ss, *dd ;
    int gotPipe=0 ;
#endif
#ifndef _UNIX
    uint8 filnam[MAXBUF] ;

    mkTempCommName (filnam,COMMAND_FILE) ;
#endif

    /* get rid of the output buffer if it exists and create new */
    if((bp=bfind(bufName,BFND_CREAT|BFND_CLEAR)) == FALSE)
        return FALSE ;
    cd = (meStrcmp(path,curdir) && (meChdir(path) != -1)) ;

#ifdef _DOS
    if(!pipeStderr)
        pipeStderr = (meGetenv("ME_PIPE_STDERR") != NULL) ? 1 : -1 ;
    /* convert '/' to '\' in program name */
    dd = line ;
    ss = comStr ;
    while((cc=*ss++) && (cc != ' '))
    {
        if(cc == '/')
            cc = '\\' ;
        *dd++ = cc ;
    }
    if(cc)
    {
        *dd++ = cc ;
        while((cc=*ss++))
        {
            if((cc == ' ') && (*ss == '>'))
                gotPipe = 1 ;
            *dd++ = cc ;
        }
    }
    if(!gotPipe)
    {
        *dd++ = ' ' ;
        *dd++ = '>' ;
        if(pipeStderr > 0)
            *dd++ = '&' ;
        strcpy(dd,filnam);
    }
    if((flags & LAUNCH_SILENT) == 0)
        mlerase(MWERASE|MWCURSOR);
    system(line);
    if(cd)
        meChdir(curdir) ;
    /* Call TTopen as we can't guarantee whats happend to the terminal */
    TTopen();
    if(meTestExist(filnam))
        return FALSE;
#endif
#ifdef _WIN32
    {
        int ss ;
        ss = WinLaunchProgram(comStr,(LAUNCH_PIPE|flags), NULL, filnam, NULL) ;
        if(cd)
            meChdir(curdir) ;
        if(ss == FALSE)
            return FALSE ;
    }
#endif
#ifdef _UNIX
    {
        uint8 *ss ;
        int ll ;

        if((ss=meStrchr(comStr,'|')) == NULL)
        {
            ll = meStrlen(comStr) ;
            ss = comStr + ll ;
        }
        else
            ll = (int) ss - (int) comStr ;

        meStrncpy(line,comStr,ll) ;
        line[ll] = '\0' ;
        /* if no data is piped in then pipe in /dev/null */
        if(meStrchr(line,'<') == NULL)
        {
            meStrncpy(line+ll," </dev/null",11) ;
            ll += 11 ;
        }
        /* merge stderr and stdout */
        meStrncpy(line+ll," 2>&1",5) ;
        ll += 5 ;
        meStrcpy(line+ll,ss) ;
    }
    TTclose();				/* stty to old modes    */
    ffrp = popen((char *)line, "r");
    if(cd)
        meChdir(curdir) ;
    TTopen();
    TTflush();
#endif

    meStrcpy(line,"cd ") ;
    meStrcat(line,path) ;
    addLineToEob(bp,line) ;
    addLineToEob(bp,comStr) ;
    addLineToEob(bp,(uint8 *)"") ;

    /* and read the stuff in */
#ifdef _UNIX
    ret = ifile(bp,NULL,meRWFLAG_SILENT) ;
#else
    ret = ifile(bp,filnam,meRWFLAG_SILENT) ;
#endif
    /* give it the path as a filename */
    bp->b_fname = meStrdup(path) ;
    /* make this window in VIEW mode, update all mode lines */
    meModeClear(bp->b_mode,MDEDIT) ;
    meModeSet(bp->b_mode,MDVIEW) ;
    bp->b_dotp = lforw(bp->b_linep) ;
    bp->line_no = 0 ;
    resetBufferWindows(bp) ;

    if((flags & LAUNCH_SILENT) == 0)
        wpopup(bp->b_bname,WPOP_MKCURR) ;

#ifndef _UNIX
    /* and get rid of the temporary file */
    meUnlink(filnam);
#endif
    return ret ;
}

/*
 * Pipe a one line command into a window
 * Bound to ^X @
 */
int
pipeCommand(int f, int n)
{
    register int ss ;
    uint8 line[MAXBUF];	        /* command line send to shell */
    uint8 nbuf[MAXBUF], *bn ;	/* buffer name */
    uint8 pbuf[MAXBUF] ;

    /* get the command to pipe in (-14 cos need to append a string) */
    if((ss=mlreply((uint8 *)"Pipe", 0, 0, line, MAXBUF-14)) != TRUE)
        return ss ;
    if((n & 0x01) == 0)
    {
        /* prompt for and get the new buffer name */
        if((ss = getBufferName((uint8 *)"Buffer", 0, 0, nbuf)) != TRUE)
            return ss ;
        bn = nbuf ;
    }
    else
        bn = BcommandN ;

    getFilePath(curbp->b_fname,pbuf) ;

    return doPipeCommand(line,pbuf,bn,(n & 0x1f)) ;
}

/*
 * filter a buffer through an external DOS program. This needs to be rewritten
 * under UNIX to use pipes.
 *
 * Bound to ^X #
 */
int
meFilter(int f, int n)
{
    register int     s;			/* return status from CLI */
    register BUFFER *bp;		/* pointer to buffer to zot */
    uint8            line[MAXBUF];	/* command line send to shell */
    uint8           *tmpnam ;		/* place to store real file name */
#ifdef _UNIX
    int	             exitstatus;	/* exit status of command */
#endif
    uint8 filnam1[MAXBUF];
    uint8 filnam2[MAXBUF];

    /* Construct the filter names */
    mkTempName (filnam1,(uint8 *)FILTER_IN_FILE,NULL);
    mkTempName (filnam2,(uint8 *)FILTER_OUT_FILE,NULL);

    /* get the filter name and its args */
    if ((s=mlreply((uint8 *)"Filter", 0, 0, line, MAXBUF)) != TRUE)
        return(s);

    if((s=bchange()) != TRUE)               /* Check we can change the buffer */
        return s ;

    /* setup the proper file names */
    bp = curbp;
    tmpnam = bp->b_fname ;	/* save the original name */
    bp->b_fname = NULL ;	/* set it to NULL         */

    /* write it out, checking for errors */
    if (writeout(bp,0,filnam1) != TRUE)
    {
        bp->b_fname = tmpnam ;
        return mlwrite(MWABORT,(uint8 *)"[Cannot write filter file]");
    }

#ifdef _DOS
    strcat(line," <");
    strcat(line, filnam1);
    strcat(line," >");
    strcat(line, filnam2);
    mlerase(MWERASE|MWCURSOR);
    system(line);
    sgarbf = TRUE;
    s = TRUE;
#endif
#ifdef _WIN32
    s = WinLaunchProgram(line,LAUNCH_FILTER,filnam1,filnam2,NULL);
    sgarbf = TRUE;
#endif
#ifdef _UNIX
    TTclose();			/* stty to old modes	*/
    meStrcat(line," <");
    meStrcat(line, filnam1);
    meStrcat(line," >");
    meStrcat(line, filnam2);
    errno = 0;			/* clear errno before call */
    if((exitstatus = system((char *)line)) != 0)
    {
        if(errno)
            mlwrite(MWCURSOR|MWWAIT,(uint8 *)"exit status %d ", exitstatus);
        else
            mlwrite(MWCURSOR|MWWAIT,(uint8 *)"exit status %d, errno %d ",
                    exitstatus, errno);
    }
    TTopen();
    sgarbf = TRUE;
    s = TRUE;
#endif

    /* on failure, escape gracefully */
    if(s == TRUE)
    {
        bp->b_fname = filnam2 ;
        if((bclear(bp) != TRUE) ||
           ((curbp->intFlag |= BIFFILE),(swbuffer(curwp,curbp) != TRUE)))
            s = FALSE ;
    }
    /* reset file name */
    bp->b_fname = tmpnam ;
    /* and get rid of the temporary file */
    meUnlink(filnam1);
    meUnlink(filnam2);

    if(s != TRUE)
        mlwrite(0,(uint8 *)"[Execution failed]");
    else
        meModeSet(bp->b_mode,MDEDIT) ;		/* flag it as changed */

    return s ;
}
