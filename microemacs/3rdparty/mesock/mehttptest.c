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
meHttpTestPrint(meUByte type,meUByte *buff,void *data)
{
    printf("LOG %d: %s\n",type,buff);
}
int
main(int argc, char *argv[])
{
#ifdef _WIN32
    WSADATA wsaData;
#endif
    meSockFile sFp;
    meCookie cook;
    meUByte buff[meSOCK_BUFF_SIZE+1], *host, *user, *pass, *file, *frdt, *pfnm;
    meInt port;
    int flgs, rr, ii, ret;
    
    // mehttptest 95 www.bbc.co.uk 443 "" "" "/"
    if((argc < 7) || ((host = (meUByte *) argv[2])[0] == '\0') || ((file = (meUByte *) argv[6])[0] == '\0'))
    {
        printf("meHttpTest Error - Usage:\n  %s <flg> <host> [port] [user] [pass] <file> [cookies] [form-data] [submit-fname] [file2]\n",argv[0]);
        return 1;
    }
#ifdef _WIN32
    if(WSAStartup(MAKEWORD(1,1),&wsaData))
    {
        printf("meHttpTest Error: Call to WSAStartup failed (%d)\n",(int) GetLastError());
        return 2;
    }
#endif
    //meHttpSetup(meHttpTestPrint,NULL,0,0);
    meSockSetup(meHttpTestPrint,NULL,20000,60000,meSOCK_IOBUF_MIN,ioBuff);
    meSockFileInit(&sFp);
    flgs = (argv[1][0] != '\0') ? atoi(argv[1]):0;
    port = (argv[3][0] != '\0') ? atoi(argv[3]):0;
    user = (argv[4][0] != '\0') ? (meUByte *) argv[4]:NULL;
    pass = (argv[5][0] != '\0') ? (meUByte *) argv[5]:NULL;
    cook.value = ((argc > 7) && (argv[7][0] != '\0')) ? (meUByte *) argv[7]:NULL;
    cook.buffLen = -1;
    frdt = ((argc > 8) && (argv[8][0] != '\0')) ? (meUByte *) argv[8]:NULL;
    pfnm = ((argc > 9) && (argv[9][0] != '\0')) ? (meUByte *) argv[9]:NULL;
    
    if((ret = meSockHttpOpen(&sFp,flgs,host,port,user,pass,file,&cook,(frdt == NULL) ? 0:strlen((char *) frdt),frdt,pfnm,buff)) < 0)
    {
        printf("meHttpOpen returned error %d: %s\n",ret,buff);
        return 4;
    }
    if(ret == 0)
    {
        printf("meHttpOpen relocate: %s\n",buff);
        meSockClose(&sFp,1);
        return 5;
    }
    
    rr = sFp.length;
    printf("About to read... %d\n",rr);
    /*Read servers response.*/
    for(;;)
    {
        ii = ((rr < 0) || (rr > meSOCK_BUFF_SIZE)) ? meSOCK_BUFF_SIZE:rr;
        if((ret = meSockRead(&sFp,ii,buff,0)) < 0)
        {
            printf("meHttpRead returned error %d: %s\n",ret,buff);
            break;
        }
        if(ret == 0)
        {
            printf("Read complete!\n");
            rr = 0;
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
    if((rr == 0) && (argc > 10) && (argv[10][0] != '\0'))
    {
        printf("About to close 0... %d\n",rr);
        meSockClose(&sFp,0);
        if((ret = meSockHttpOpen(&sFp,flgs,host,port,user,pass,(meUByte *) argv[10],&cook,0,NULL,NULL,buff)) < 0)
        {
            printf("meHttpOpen2 returned error %d: %s\n",ret,buff);
            return 14;
        }
        if(ret == 0)
        {
            printf("meHttpOpen2 relocate: %s\n",buff);
            meSockClose(&sFp,1);
            return 15;
        }
        rr = sFp.length;
        printf("About to read2... %d\n",rr);
        /*Read servers response.*/
        for(;;)
        {
            ii = ((rr < 0) || (rr > meSOCK_BUFF_SIZE)) ? meSOCK_BUFF_SIZE:rr;
            if((ret = meSockRead(&sFp,ii,buff,0)) < 0)
            {
                printf("meHttpRead2 returned error %d: %s\n",ret,buff);
                break;
            }
            if(ret == 0)
            {
                printf("Read2 complete!\n");
                rr = 0;
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
    }
    printf("About to close 1... %d\n",rr);
    meSockClose(&sFp,1);
    printf("About to end... %d\n",rr);
    meSockEnd();
}
