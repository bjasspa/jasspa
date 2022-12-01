/* -*- C -*- ****************************************************************
 *
 *  			      Copyright 2021 Maxinity Software Ltd.
 *			      All Rights Reserved
 *
 *  Created By  : Steven Phillips
 *  Created     : 2021-04-10 16:38:33
 *  Description :
 *
 *  Notes
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
