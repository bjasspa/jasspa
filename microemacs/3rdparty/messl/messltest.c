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

#include "messl.h"

void
meSslTestPrint(meUByte *buff,void *data)
{
    printf("LOG: %s\n",buff);
}
int
main(int argc, char *argv[])
{
#ifdef _WIN32
    WSADATA wsaData;
#endif
    meSslFile sFp;
    meUByte buff[meSSL_BUFF_SIZE+1], *host, *user, *pass, *file, *cook, *pfnm;
    meInt port;
    int rr, ii, ret;
    
    if((argc < 7) || ((host = (meUByte *) argv[2])[0] == '\0') || ((file = (meUByte *) argv[6])[0] == '\0'))
    {
        printf("meSslTest Error - Usage:\n  %s [flg] <host> [port] [user] [pass] <file> [submit-fname] [file2]\n",argv[0]);
        return 1;
    }
#ifdef _WIN32
    if(WSAStartup(MAKEWORD(1,1),&wsaData))
    {
        printf("meSslTest Error: Call to WSAStartup failed (%d)\n",GetLastError());
        return 2;
    }
#endif
    ii = (argv[1][0] != '\0') ? atoi(argv[1]):7;
    if((ii=meSslSetup(ii,meSslTestPrint,NULL,buff)) < 0)
    {
        printf("meSslTest Setup Error %d: %s\n",ii,buff);
        return 3;
    }
    port = (argv[3][0] != '\0') ? atoi(argv[3]):0;
    user = (argv[4][0] != '\0') ? (meUByte *) argv[4]:NULL;
    pass = (argv[5][0] != '\0') ? (meUByte *) argv[5]:NULL;
    cook = ((argc > 6) && (argv[6][0] != '\0')) ? (meUByte *) argv[6]:NULL;
    pfnm = ((argc > 7) && (argv[7][0] != '\0')) ? (meUByte *) argv[7]:NULL;

    if((ret = meSslOpen(&sFp,host,port,user,pass,file,cook,pfnm,0,buff)) < 0)
    {
        printf("meSslOpen returned error %d: %s\n",ret,buff);
        return 4;
    }
    if(ret == 0)
    {
        printf("meSslOpen relocate: %s\n",buff);
        meSslClose(&sFp,1);
        return 5;
    }
    
    rr = sFp.length;
    printf("About to read... %d\n",rr);
    /*Read servers response.*/
    for(;;)
    {
        ii = ((rr < 0) || (rr > meSSL_BUFF_SIZE)) ? meSSL_BUFF_SIZE:rr;
        if((ret = meSslRead(&sFp,ii,buff)) < 0)
        {
            printf("meSslRead returned error %d: %s\n",ret,buff);
            break;
        }
        if(ret == 0)
        {
            printf("Read complete!\n");
            break;
        }
        buff[ret] = '\0';
        printf("Server said: %s\n",buff);
        if((rr > 0) && ((rr -= ret) <= 0))
        {
            printf("Read complete!\n");
            break;
        }
    }
    printf("About to close...\n");
    meSslClose(&sFp,1);
    if((ret == 0) && (argc > 8) && (argv[8][0] != '\0'))
    {
        if((ret = meSslOpen(&sFp,host,port,user,pass,(meUByte *) argv[8],cook,NULL,0,buff)) < 0)
        {
            printf("meSslOpen2 returned error %d: %s\n",ret,buff);
            return 14;
        }
        if(ret == 0)
        {
            printf("meSslOpen2 relocate: %s\n",buff);
            meSslClose(&sFp,1);
            return 15;
        }
        rr = sFp.length;
        printf("About to read2... %d\n",rr);
        /*Read servers response.*/
        for(;;)
        {
            ii = ((rr < 0) || (rr > meSSL_BUFF_SIZE)) ? meSSL_BUFF_SIZE:rr;
            if((ret = meSslRead(&sFp,ii,buff)) < 0)
            {
                printf("meSslRead2 returned error %d: %s\n",ret,buff);
                break;
            }
            if(ret == 0)
            {
                printf("Read2 complete!\n");
                break;
            }
            buff[ret] = '\0';
            printf("Server said: %s\n",buff);
            if((rr > 0) && ((rr -= ret) <= 0))
            {
                printf("Read2 complete!\n");
                break;
            }
        }
        printf("About to close2...\n");
        meSslClose(&sFp,1);
    }
    printf("About to end...\n");
    meSslEnd();
}
