/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * spell.c - Spell checking routines.
 *
 * Copyright (C) 2021-2024 JASSPA (www.jasspa.com)
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

#include "mesock.h"

static meUByte ioBuff[meSOCK_IOBUF_MIN];

void
meFtpTestPrint(meUByte type,meUByte *buff,void *data)
{
    printf("LOG %d: %s\n",type,buff);
    fflush(stdout);
}
int
main(int argc, char *argv[])
{
#ifdef _WIN32
    WSADATA wsaData;
#endif
    meSockFile sFp;
    meUByte buff[meSOCK_BUFF_SIZE+1], *host, *user, *pass, *dir;
    meInt port;
    int ii, flgs, ret;
    
    if((argc < 7) || ((host = (meUByte *) argv[2])[0] == '\0') || (argv[6][0] == '\0'))
    {
        printf("meFtpTest Error - Usage:\n  %s <flg> <host> [port] [user] [pass] <dir> <dir2>\n",argv[0]);
        return 1;
    }
#ifdef _WIN32
    if(WSAStartup(MAKEWORD(1,1),&wsaData))
    {
        printf("meFtpTest Error: Call to WSAStartup failed (%d)\n",(int) GetLastError());
        return 2;
    }
#endif
    //meFtpSetup(meFtpTestPrint,NULL,0,0);
    meSockSetup(meFtpTestPrint,NULL,20000,60000,meSOCK_IOBUF_MIN,ioBuff);
    meSockFileInit(&sFp);
    flgs = (argv[1][0] != '\0') ? atoi(argv[1]):0;
    port = (argv[3][0] != '\0') ? atoi(argv[3]):0;
    user = (argv[4][0] != '\0') ? (meUByte *) argv[4]:NULL;
    pass = (argv[5][0] != '\0') ? (meUByte *) argv[5]:NULL;
    if((ret = meSockFtpOpen(&sFp,flgs,host,port,user,pass,buff)) < 0)
    {
        printf("meFtpOpen returned error %d: %s\n",ret,buff);
        return 4;
    }
    printf("Home: %s\n",(sFp.home == NULL) ? "<null>":(char *) sFp.home);
    
    for(ii=6 ; ii<argc ; ii++)
    {
        dir = (meUByte *) argv[ii];
        if(dir[0] != '\0')
        {
            printf("Path %d: %s\n",ii,dir);
            if(meSockFtpCommand(&sFp,buff,"CWD %s",dir) != ftpPOS_COMPLETE)
                return 7;
            printf(" CWD Returned: %s\n",buff);
            if(meSockFtpGetDataChannel(&sFp,buff) < 0)
                return 6;
            if(meSockFtpCommand(&sFp,buff,"LIST") != ftpPOS_PRELIMIN)
                return 8;
            printf(" LIST Returned: %s\n",buff);
            if(meSockFtpConnectData(&sFp,buff) < 0)
                return 8;
            for(;;)
            {
                if((ret = meSockRead(&sFp,meSOCK_BUFF_SIZE,buff,0)) < 0)
                {
                    printf("meFtpRead returned error %d: %s\n",ret,buff);
                    break;
                }
                if(ret == 0)
                {
                    printf("Read complete!\n");
                    break;
                }
                buff[ret] = '\0';
                printf("%s",buff);
                fflush(stdout);
            }
            meSockClose(&sFp,0);
            if(meSockFtpReadReply(&sFp,buff) != ftpPOS_COMPLETE)
                return 9;
            printf(" LIST Reply: %s\n",buff);
        }
    }
    printf("About to close 1...\n");
    meSockClose(&sFp,1);
    printf("About to end...\n");
    meSockEnd();
}
