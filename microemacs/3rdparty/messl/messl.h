/* -*- C -*- ****************************************************************
 *
 * Copyright (c) 2020 Maxinity Software Ltd (www.maxinity.co.uk).
 * 
 * All Rights Reserved.
 * 
 * This  document  may  not, in  whole  or in  part, be  copied,  photocopied,
 * reproduced,  translated,  or  reduced to any  electronic  medium or machine
 * readable form without prior written consent from Maxinity Software Ltd.
 *
 ****************************************************************************/

#ifndef __MESSL_H
#define __MESSL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#if (_MSC_VER != 900)
#include <winsock2.h>
#else
#include <winsock.h>
#endif
#define meSOCKET   SOCKET
#else
#define meSOCKET   int
#endif

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#define meSSL_BUFF_SIZE 2048

typedef unsigned char meUByte;
typedef unsigned int  meUInt;
typedef int  meInt;
typedef void (*meSslLogger)(meUByte *txt,void *data);

typedef struct meSslFile {
    void *ssl;
    meSOCKET sck;
    meInt length;
} meSslFile;

#define meSSL_LOG_WARNING 0x01
#define meSSL_LOG_HEADERS 0x02
#define meSSL_LOG_VERBOSE 0x04
#define meSSL_IGN_CRT_ERR 0x08

int
meSslSetup(meInt flags,meSslLogger logger,void *logData, meUByte *rbuff);
void
meSslSetProxy(meUByte *host, meInt port);
int
meSslOpen(meSslFile *sFp, meUByte *host, meInt port, meUByte *user, meUByte *pass, meUByte *file, meUByte *cookie, meUByte *postFName, meUInt rwflag, meUByte *rbuff);
int
meSslRead(meSslFile *sFp, int sz, meUByte *rbuff);
void
meSslClose(meSslFile *sFp, int shutDown);
void
meSslEnd();


#if defined(__cplusplus) || defined(c_plusplus)
}
#endif
#endif /* __MESSL_H */
