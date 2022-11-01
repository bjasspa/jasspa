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

#define meSOCK_BUFF_SIZE 2048

typedef unsigned char meUByte;
typedef unsigned int  meUInt;
typedef int  meInt;
typedef void (*meSockLogger)(meUByte *txt,void *data);

typedef struct meCookie {
    meUByte *value;
    meInt buffLen;
} meCookie;

typedef struct meSockFile {
    meUByte flags;
    meUByte chst[255];
    meInt cprt;
    meInt length;
    void *ssl;
    meSOCKET sck;
} meSockFile;

#define meSOCK_LOG_WARNING 0x01
#define meSOCK_LOG_HEADERS 0x02
#define meSOCK_LOG_VERBOSE 0x04
#define meSOCK_IGN_CRT_ERR 0x08
#define meSOCK_USE_HTTPS   0x10
#define meSOCK_REUSE       0x20
#define meSOCK_INUSE       0x40
#define meSOCK_SHUTDOWN    0x80

#define meSockFileIsInUse(sFp) ((sFp)->flags != 0)
#define meSockFileCanReuse(sFp) ((sFp)->flags & meSOCK_REUSE)
void
meSockSetup(meSockLogger logger,void *logData, int timeoutSnd, int timeoutRcv, int ioBfSz, meUByte *ioBf);
void
meSockSetProxy(meUByte *host, meInt port);

int
meSockHttpOpen(meSockFile *sFp, meUByte flags, meUByte *host, meInt port, meUByte *user, meUByte *pass, meUByte *file, meCookie *cookie, 
               meInt fdLen, meUByte *frmData, meUByte *postFName, meUByte *rbuff);

int
meSockRead(meSockFile *sFp, int sz, meUByte *rbuff, int rOffset);
int
meSockClose(meSockFile *sFp, int force);

void
meSockEnd();

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif
#endif /* __MESOCK_H */
