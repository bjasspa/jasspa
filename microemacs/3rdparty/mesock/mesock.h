/* -*- C -*- ****************************************************************
 *
 *  			      Copyright 2021 Maxinity Software Ltd.
 *			      All Rights Reserved
 *
 *  Created By  : Steven Phillips
 *  Created     : 2021-09-20 17:39:05
 *  Description	: Error numbers used
 *
 ****************************************************************************
 *
 *  Copyright (c) 2021 Maxinity Software Ltd.
 * 
 *  All Rights Reserved.
 * 
 * This  document  may  not, in  whole  or in  part, be  copied,  photocopied,
 * reproduced,  translated,  or  reduced to any  electronic  medium or machine
 * readable form without prior written consent from Maxinity Software Ltd.
 *
 ****************************************************************************/

#ifndef __MESOCK_H
#define __MESOCK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <winsock2.h>
#define meSOCKET   SOCKET
#else
#define meSOCKET   int
#endif

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#define meSOCK_IOBUF_MIN 4096
#define meSOCK_BUFF_SIZE 2048
#define meSOCK_USER_SIZE 126
#define meSOCK_HOST_SIZE 256

typedef unsigned char  meUByte;
typedef unsigned short meUShort;
typedef unsigned int   meUInt;
typedef int  meInt;
typedef void (*meSockLogger)(meUByte type,meUByte *txt,void *data);

typedef struct meCookie {
    meUByte *value;
    meInt buffLen;
} meCookie;

typedef struct meSockFile {
    meUShort flags;
    meUByte cusr[meSOCK_USER_SIZE];
    meUByte chst[meSOCK_HOST_SIZE];
    meUByte *home;
    void *ssl;
    void *ctrlSsl;
    meInt cprt;
    meInt length;
    meInt chnkLen;
    meSOCKET sck;
    meSOCKET ctrlSck;
} meSockFile;

#define meSOCK_LOG_STATUS    0x0001
#define meSOCK_LOG_ERROR     0x0002
#define meSOCK_LOG_WARNING   0x0004
#define meSOCK_LOG_DETAILS   0x0008
#define meSOCK_LOG_VERBOSE   0x0010
#define meSOCK_IGN_CRT_ERR   0x0020
#define meSOCK_USE_SSL       0x0040
#define meSOCK_EXPLICIT_SSL  0x0080
#define meSOCK_CLOSE         0x0100
#define meSOCK_PUBLIC_MASK   0x01ff
/* Following are for internal use */
#define meSOCK_INUSE         0x0200
#define meSOCK_SHUTDOWN      0x0400
#define meSOCK_CTRL_INUSE    0x0800
#define meSOCK_CTRL_NO_QUIT  0x1000
#define meSOCK_CTRL_SHUTDOWN 0x2000
#define meSOCK_CHUNKED       0x1000

#define meSockFileInit(sFp) ((sFp)->flags = 0)
#define meSockIsInUse(sFp)  ((sFp)->flags != 0)
#define meSockIsFtp(sFp)    ((sFp)->flags & meSOCK_CTRL_INUSE)
#define meSockDataIsInUse(sFp) ((sFp)->flags & meSOCK_INUSE)
#define meSockFileCanReuse(sFp) (((sFp)->flags & meSOCK_CLOSE) == 0)
#define meSockControlIsInUse(sFp) ((sFp)->flags & meSOCK_CTRL_INUSE)

void
meSockSetup(meSockLogger logger,void *logData, int timeoutSnd, int timeoutRcv, int ioBfSz, meUByte *ioBf);
void
meSockSetProxy(meUByte *host, meInt port);

int
meSockHttpOpen(meSockFile *sFp, meUShort flags, meUByte *host, meInt port, meUByte *user, meUByte *pass, meUByte *file, meCookie *cookie, 
               meInt fdLen, meUByte *frmData, meUByte *postFName, meUByte *rbuff);

#define ftpERROR        -1
#define ftpPOS_PRELIMIN  1
#define ftpPOS_COMPLETE  2
#define ftpPOS_INTERMED  3

int
meSockFtpOpen(meSockFile *sFp, meUShort flags, meUByte *host, meInt port, meUByte *user, meUByte *pass, meUByte *rbuff);
meInt
meSockFtpCommand(meSockFile *sFp, meUByte *rbuff, char *fmt, ...);
meInt
meSockFtpReadReply(meSockFile *sFp, meUByte *buff);
meInt
meSockFtpGetDataChannel(meSockFile *sFp, meUByte *buff);
meInt
meSockFtpConnectData(meSockFile *sFp, meUByte *buff);


int
meSockRead(meSockFile *sFp, int sz, meUByte *rbuff, int rOffset);
int
meSockWrite(meSockFile *sFp, int sz, meUByte *wbuff, int wOffset);
int
meSockClose(meSockFile *sFp, int force);

void
meSockEnd();

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif
#endif /* __MESOCK_H */
