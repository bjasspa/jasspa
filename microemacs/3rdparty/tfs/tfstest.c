/* -*- C -*- *****************************************************************
 *
 * Copyright (C) 2025 JASSPA (www.jasspa.com).
 *
 * This is part of JASSPA's MicroEmacs, see the LICENSE file for licensing and
 * copying information.
 *
 * Synopsis:    
 * Authors:     Steven Phillips
 *
 *****************************************************************************/

#include <tfs.h>

int
main(int argc, char *argv[])
{
    tfsMount *tfs;
    int ii;
    
    if(argc < 2)
    {
        printf("Usage: %s <tfs-file> [ <cmd> <name> ]\n",argv[0]);
        return -1;
    }
    if((tfs=tfs_mount((tfsUByte *) argv[1])) == NULL)
    {
        printf("Error: tfs_mount failed for [%s]\n",argv[1]);
        return -1;
    }
    printf("tfs_mount success: tfs://%s\n",argv[1]);
    if(argc == 4)
    {
        if(argv[2][0] == 'l')
        {
            tfsDirectory *dir;
            if((dir = tfs_dopen(tfs,(tfsUByte *) argv[3])) == NULL)
            {
                printf("Error: tfs_dopen failed for tfs://%s?%s\n",argv[1],argv[3]);
            }
            else
            {
                tfsDirent de;
                printf("tfs_dopen success: tfs://%s?%s (%d,%d)\n",argv[1],argv[3],dir->dcount,dir->fcount);
                while((ii=tfs_dread(dir,&de)) > 0)
                {
                    printf("  %s%s\n",de.name,(de.mode & tfsSTMODE_DIRECTORY) ? "/":"");
                }
                if(ii < 0)
                    printf("Error: tfs_dread failed for tfs://%s?%s (%d)\n",argv[1],argv[3],ii);
                tfs_dclose(dir);
            }
        }
        else if(argv[2][0] == 's')
        {
            tfsStat stt;
            if(tfs_stat(tfs,(tfsUByte *) argv[3],&stt))
            {
                printf("Error: tfs_stat failed for tfs://%s?%s\n",argv[1],argv[3]);
            }
            else
            {
                printf("tfs_stat success: tfs://%s?%s (Md:%02x,Ct:%08x,Ps:%lld,Ln:%lld,Cm:%lld)\n",argv[1],argv[3],
                       stt.mode,stt.ctime,stt.entPos,stt.fileLen,stt.compLen);
            }
        }
        else if(argv[2][0] == 'f')
        {
            tfsFile *fl;
            if((fl=tfs_fopen(tfs,(tfsUByte *) argv[3])) == NULL)
            {
                printf("Error: tfs_fopen failed for tfs://%s?%s\n",argv[1],argv[3]);
            }
            else
            {
                FILE *fp;
                char buf[2048];
                if((fp = fopen("tfstest.out","wb")) == NULL)
                {
                    printf("Error: Failed to open output file\n");
                }
                else
                {
                    while((ii=tfs_fread(buf,2048,fl)) > 0)
                    {
                        if(fwrite(buf,1,ii,fp) != ii)
                        {
                            printf("Error: fwrite failed\n");
                            break;
                        }
                    }
                    if(ii < 0)
                        printf("Error: tfs_fread failed for tfs://%s?%s (%d)\n",argv[1],argv[3],ii);
                    fclose(fp);
                }
                tfs_fclose(fl);
            }
        }
    }
    tfs_umount(tfs);
    printf("Test complete\n");
    return 0;
}
