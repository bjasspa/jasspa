/* -*- C -*-
**
**  			Copyright 1999 Logica.
**			      All Rights Reserved
**
**	File name :             $FILE_NAME$
**
**	Description :           
**
**	Author :                Steven Phillips
**
**	External functions :
**			
**	Internal functions :  
**		
**  Created Date :          Tue Dec 7 11:42:08 1999
**
**  Last Modified :         <991208.2040>
**
**	Release :	 
**
*/

#include <ctype.h>
#include <stdarg.h>
#include <string.h>

#define DIR_CHAR '/'
#define FIOBUFSIZ 2048

#ifdef _WIN32
/* winsock2.h must be included before */
#include <winsock2.h>
#include <io.h>
typedef void (*meATEXIT)(void) ;
static SOCKET ffccsk ;
static SOCKET ffsock ;
#define meOpenSocket(a,b,c)  WSASocket(a,b,c,NULL,0,0)
#define meCloseSocket closesocket
#else
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>

#define meOpenSocket  socket
#define meCloseSocket close
static int ffsock=-1 ;
static int ffccsk=-1 ;
#endif
static struct sockaddr_in meSockAddr ;
char ffbuf[FIOBUFSIZ] ;
char *ffcur ;
int ffremain=0 ;
int ffread=0 ;

#ifndef INADDR_NONE
/* This may not be defined, particularly suns */
#define INADDR_NONE -1
#endif
#define ftpERROR         0
#define ftpPOS_PRELIMIN  1
#define ftpPOS_COMPLETE  2
#define ftpPOS_INTERMED  3

/* `hostname' can be of any form described in inet_addr(3);
 * `port' can be a service name, a port number, or NULL to have the
 * OS choose a port (this is probably only useful for servers);
 * `proto' can be "udp" or "tcp"
 */
static int
make_inet_addr(char *hostname, char *port, char *proto)
{
    struct hostent *hp ;
    struct servent *sp ;
    
    memset(&meSockAddr,0,sizeof(meSockAddr)) ;
    meSockAddr.sin_family = AF_INET ;
    
    if ((sp = getservbyname(port, proto)) != NULL)
    {
        meSockAddr.sin_port = sp->s_port;
    }
    else if ((meSockAddr.sin_port = htons(atoi(port))) == 0)
    {
        printf("[Bad port number %s]", port) ;
        return 0 ;
    }
    
    /* First resolve the hostname, then resolve the port */
    if ((meSockAddr.sin_addr.s_addr = inet_addr(hostname)) != INADDR_NONE)
    {
        /* in_addr.s_addr is already set */
    }
    else if(((hp = gethostbyname(hostname)) != NULL) &&
            (hp->h_length <= sizeof(struct in_addr)) &&
            (hp->h_addrtype == AF_INET))
        memcpy(&(meSockAddr.sin_addr),hp->h_addr,hp->h_length) ;
    else
    {
        printf("[Unknown host %s]", hostname) ;
        return 0 ;
    }
    
    return 1 ;
}

int
ftpReadReplyLine(char *buff)
{
    char *dd=buff, cc ;
    int ii ;
    
    for(ii=0;;)
    {
        if(ffremain == 0)
        {
            ffremain = recv(ffccsk,(char *) ffbuf,FIOBUFSIZ,0) ;
            if(ffremain <= 0)
            {
                printf("Failed to read line\n") ;
                return 0 ;
            }
            ffcur = ffbuf ;
        }
        ffremain-- ;
        cc = *ffcur++ ;
        if(cc == '\n')
        {
            *dd = '\0' ;
            printf("Read [%s]\n",buff) ;
            return 1 ;
        }
        if(cc != '\r')
            *dd++ = cc ;
    }
}

int
ftpReadReply(void)
{
    char buff[1024] ;
    int ret ;
    
    ffremain = 0 ;
    if(!ftpReadReplyLine(buff))
        return 0 ;
    if(strlen(buff) < 4)
        ret = ftpERROR ;
        
    ret = buff[0] - '0' ;
    if(buff[3] == '-')
    {
        /* multi-line reply */
        char c0, c1, c2 ;
        c0 = buff[0] ;
        c1 = buff[1] ;
        c2 = buff[2] ;
        do {
            if(!ftpReadReplyLine(buff))
                return 0 ;
        } while((buff[0] != c0) || (buff[1] != c1) ||
                (buff[2] != c2) || (buff[3] != ' ')) ;
    }
    if(ret > ftpPOS_INTERMED)
        ret = ftpERROR ;
    return ret ;
}


int 
ftpCommand(int flags, char *fmt, ...)
{
    va_list ap;
    int ii ;
    
    va_start(ap,fmt);
    vsprintf(ffbuf,fmt,ap);
    va_end(ap);
    printf("---> %s\n",ffbuf);
    ii = strlen(ffbuf) ;
    ffbuf[ii++] = '\r' ;
    ffbuf[ii++] = '\n' ;
    ffbuf[ii]   = '\0' ;
    if(send(ffccsk,ffbuf,ii,0) <= 0)
    {
        printf("Failed to send command\n") ;
        return 0 ;
    }
    if(flags & 0x01)
        return 1 ;
    return ftpReadReply();
}
    
void
ftpClose(void)
{
    if(ffsock >= 0)
    {
        meCloseSocket(ffsock) ;
        ffsock = -1 ;
    }
    if(ffccsk >= 0)
    {
        ftpCommand(1,"QUIT") ;
        meCloseSocket(ffccsk) ;
        ffccsk = -1 ;
    }
    ffremain = 0 ;
}

int
ftpOpenControlChannel(char *host, char *port)
{
    if(port == NULL)
        port = "21" ;
    if(!make_inet_addr(host,port,"tcp"))
        return 0 ;
    
    if((ffccsk=meOpenSocket(AF_INET,SOCK_STREAM,0)) < 0)
    {
        printf("[Failed to open socket]") ;
        return 0 ;
    }
    if(connect(ffccsk,(struct sockaddr *) &meSockAddr,sizeof(meSockAddr)) < 0)
    {
        meCloseSocket(ffccsk) ;
        ffccsk = -1 ;
        printf("[Failed to connect to %s:%s]",host,port) ;
        return 0 ;
    }
    return 1 ;

}

int
ftpLogin(char *user, char *pass)
{
    int ii ;
    /* get the initial message */
    if(ftpReadReply() != ftpPOS_COMPLETE)
        return 0 ;
    
    ii = ftpCommand(0,"USER %s",user) ;
    if(ii == ftpPOS_INTERMED)
        ii = ftpCommand(0,"PASS %s", pass) ;
    if(ii != ftpPOS_COMPLETE)
    {
        printf("Failed to login\n") ;
        return 0 ;
    }
    return 1 ;
}

int
ftpOpen(char *host, char *port, char *user, char *pass)
{
    unsigned char *aac, *ppc ;
    int aai[4], ppi[2] ;
    char *ss ;
    if(!ftpOpenControlChannel(host,port))
        return 0 ;
    
    if(!ftpLogin(user,pass))
    {
        ftpClose() ;
        return 0 ;
    }
    if(ftpCommand(0,"TYPE I") != ftpPOS_COMPLETE)
    {
        ftpClose() ;
        return 0 ;
    }
    if((ftpCommand(0,"PASV") != ftpPOS_COMPLETE) ||
       ((ss=strchr(ffbuf,'(')) == NULL) || 
       (sscanf(ss,"(%d,%d,%d,%d,%d,%d)",aai,aai+1,aai+2,aai+3,ppi,ppi+1) != 6))
    {
        ftpClose() ;
        printf("Failed to get PASSIVE address\n") ;
        return 0 ;
    }
    aac = (unsigned char *)&meSockAddr.sin_addr;
    ppc = (unsigned char *)&meSockAddr.sin_port;
    aac[0] = (unsigned char) aai[0] ;
    aac[1] = (unsigned char) aai[1] ;
    aac[2] = (unsigned char) aai[2] ;
    aac[3] = (unsigned char) aai[3] ;
    ppc[0] = (unsigned char) ppi[0] ;
    ppc[1] = (unsigned char) ppi[1] ;

    return 1 ;
}

/*
 * Need to start a listen on the data channel before we send the command,
 * otherwise the server's connect may fail.
 */
int
ftpOpenRecvChannel(void)
{
    struct sockaddr_in addr ;
    unsigned char *aa, *pp ;
    int ii, len ;
    
    ffsock = socket(AF_INET, SOCK_STREAM, 0);
    if (ffsock < 0)
    {
        perror("ftp: socket");
        return 0 ;
    }
    if(connect(ffsock,(struct sockaddr *) &meSockAddr,sizeof(meSockAddr)) < 0)
    {
        printf("Failed to open data channel") ;
        return 0 ;
    }
#if 0    
    ii = 1 ;
    if (setsockopt(ffsock,SOL_SOCKET,SO_REUSEADDR,(char *)&ii,sizeof(int)) < 0)
    {
        perror("ftp: setsockopt (reuse address)");
        return 0 ;
    }
    len = sizeof(struct sockaddr_in);
    if(bind(ffsock,(struct sockaddr *) &meSockAddr,len) < 0)
    {
        perror("ftp: bind");
        return 0 ;
    }
    if (listen(ffsock, 1) < 0)
        perror("ftp: listen");
    
    if((ii=accept(ffsock,(struct sockaddr *) &addr, &len)) < 0)
    {
        perror("ftp: accept");
        return 0 ;
    }
    meCloseSocket(ffsock) ;
    ffsock = ii ;
#endif
    return 1 ;
}

int
ftpReadOpen(char *urlName)
{
    char buff[1024], *addr, *port, *ss, *dd, cc ;
    char *user, *passwd ;
    int  ii, dirlist ;
    
#ifdef _WIN32
    static int init=0 ;
    if(!init)
    {
        WSADATA wsaData;
        WORD    wVersionRequested;
        
        wVersionRequested = MAKEWORD (1, 1);
    
        if(WSAStartup(wVersionRequested, &wsaData))
        {
            printf("[Failed to initialise sockets]") ;
            return 0 ;
        }
        atexit((meATEXIT) WSACleanup) ;
        init = 1 ;
    }
#endif
    ss = urlName ;
    /* skip the http: or ftp: */
    if(ss[0] == 'f')
        ss += 6 ;
    else
        ss += 7 ;
    dd = buff ;
    addr = dd ;
    port = NULL ;
    user = NULL ;
    while(((cc=*ss++) != '\0') && (cc != DIR_CHAR))
    {
        *dd++ = cc ;
        if(cc == ':')
            port = dd ;
        else if(cc == '@')
        {
            user = buff ;
            dd[-1] = '\0' ;
            addr = dd ;
            port = NULL ;
        }
    }
    *dd = '\0' ;
    ss-- ;
    if(user != NULL)
    {
        passwd = user ;
        while(*passwd != ':')
            passwd++ ;
        *passwd++ = '\0' ;
    }
    else
    {
        user = "anonymous" ;
        passwd = "guest" ;
    }
    dirlist = strlen(ss) ;
    dirlist = (ss[dirlist-1] == DIR_CHAR) ;
    
    if(ftpOpen(addr,port,user,passwd) == 0)
        return 0 ;
    
    /* send the read command */
    if(dirlist)
        ii = ftpCommand(1,"LIST %s",ss) ;
    else
        ii = ftpCommand(1,"RETR %s",ss) ;
    
    /* open up the recv channel */
    if(!ftpOpenRecvChannel())
    {
        ftpClose() ;
        return 0 ;
    }
    
    /* find out if all's well */
    if(ftpReadReply() != ftpPOS_PRELIMIN)
    {
        printf("Failed to get file\n") ;
        return 0 ;
    }
    return 1 ;
}

static int
ffgetBuf(void)
{
    ffremain = FIOBUFSIZ ;
    ffremain = recv(ffsock,(char *) ffbuf,ffremain,0) ;
    if(ffremain <= 0)
    {
        ffremain = -1 ;
        return 1 ;
    }
    ffbuf[ffremain]='\0' ;
    ffread += ffremain ;
    return 1 ;
}

int
ftpRead(void)
{
    int rr ;
    ffread = 0 ;
    printf("Reading [") ;
    for(rr=1 ;;)
    {
        if((rr=ffgetBuf()) != 1)
            break ;
        if(ffremain < 0)
            break ;
        printf("%s",ffbuf) ;
    }
    if(rr != 1)
        return 0 ;
    printf("] Read %d bytes\n",ffread) ;
    
    /* get the transfer complete message */
    return (ftpReadReply() != ftpPOS_COMPLETE) ;
}



int
main(int argc, char **argv)
{
    if(argc != 2 )
    {
        printf("Usage: ftp <url>\n") ;
        exit(1) ;
    }
    
    if(ftpReadOpen(argv[1]) == 0)
        exit(1) ;
    ftpRead() ;
    ftpClose() ;
}    
              

