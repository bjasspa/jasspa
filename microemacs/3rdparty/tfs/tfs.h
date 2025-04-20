/* -*- C -*- *****************************************************************
 *
 * Copyright (C) 2025 JASSPA (www.jasspa.com).
 *
 * This is part of JASSPA's MicroEmacs, see the LICENSE file for licensing and
 * copying information.
 *
 * Synopsis:    Tacked-on File System (TFS) library definitions
 * Authors:     Jon Green & Steven Phillips
 * 
 * Notes:
 * 
 * See header of tfsutil.c for documentation on the tfs file structure, while
 * this has been optimised for MicroEmacs, this remains a generic library.
 * 
 * This library uses LZMA2 compression available in the 7zip SDK (www.7zip.com)
 * A big thank you to to Igor Pavlov for providing this library for free and
 * without restrictions.  
 *
 *****************************************************************************/

#ifndef __TFS_H
#define __TFS_H

#include <stdio.h>

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#define tfsVERSION      0x02
#define tfsNAME_MAX     1024
#define tfsDIRENT_MAX   (tfsNAME_MAX+32)

#define tfsSTMODE_READ        0x01
#define tfsSTMODE_WRITE       0x02
#define tfsSTMODE_EXECUTE     0x04
#define tfsSTMODE_COMPRESSED  0x08
#define tfsSTMODE_FILE        0x10
#define tfsSTMODE_DIRECTORY   0x20

typedef unsigned char      tfsUByte;
typedef unsigned int       tfsUInt;
typedef unsigned long long tfsULong;

typedef struct {
    tfsULong headPos;
    tfsULong rootPos;
    tfsUInt ctime;
    tfsUByte version;
    tfsUByte name[1];
} tfsMount;

typedef struct {
    tfsULong entPos;
    tfsULong compLen;
    tfsULong fileLen;
    tfsUInt ctime;
    tfsUByte mode;
} tfsStat;

typedef struct {
    FILE *fp;
    tfsULong dirPos;
    tfsUInt ctime;
    int dcount;
    int fcount;
    int dremain;
    int fremain;
} tfsDirectory;

typedef struct {
    tfsULong entPos;
    tfsULong compLen;
    tfsULong fileLen;
    tfsUInt ctime;
    tfsUInt nameLen;
    tfsUByte mode;
    tfsUByte name[tfsNAME_MAX];
} tfsDirent;

typedef struct {
    FILE *fp;
    tfsULong fileLen;
    tfsULong fremain;
    tfsUByte mode;
} tfsFile;

/**
 * Open access to the TFS in given file.
 *
 * @param [in] name The name of the file containing the TFS.
 */
extern tfsMount *
tfs_mount(tfsUByte *name);

/**
 * Close access to the built-in file system.
 *
 * @param [in] tfs  The TFS structure.
 */
extern void
tfs_umount(tfsMount *tfs);

/**
 * Find the file type.
 *
 * @param [in] tfs  The file system mount point.
 * @param [in] name The pathname of the object to determine.
 *
 * @return The status of the call.
 * @retval TFS_TYPE_UNDEF This is a undefined file type.
 * @retval TFS_TYPE_FILE This is a regular file.
 * @retval TFS_TYPE_DIR This is a directory.
 */
extern int
tfs_type(tfsMount *tfs, tfsUByte *name);

/**
 * Get the status of the node.
 *
 * @param [in] tfs  The file system mount point.
 * @param [in] name The pathname of the object to determine.
 * @param [in] stt  The returned status information.
 * 
 * @return The status of the stat operation/
 * @retval 0   Operation was successful and object found.
 * @retval -ve Operation failed.
 */
extern int
tfs_stat(tfsMount *tfs, tfsUByte *name, tfsStat *stt);

/**
 * Open a directory.
 *
 * @param [in] tfs  The file system mount point.
 * @param [in] name The name of the directory to open.
 *
 * @return A handle to the directory or NULL on an error.
 */
extern tfsDirectory *
tfs_dopen(tfsMount *tfs, tfsUByte *name);

/**
 * Close the directory
 *
 * @param [in] dp  The directory reference from a tfs_dopen().
 */
extern void
tfs_dclose(tfsDirectory *dp);

/**
 * Read the next directory entry
 *
 * @param [in] dp     The directory reference from a tfs_dopen().
 * @param [in] dirent The returned directory entry information.
 *
 * @return The status of the read operation/
 * @retval 1   Operation was successful and entry returned.
 * @retval 0   No more entries to return.
 * @retval -ve Operation failed.
 */
extern int
tfs_dread(tfsDirectory *dp, tfsDirent *dirent);

/**
 * Open the built-in file system
 *
 * @param [in] tfs  The file system mount point.
 * @param [in] name The name of the file to open.
 *
 * @return A file descriptor or NULL on an error.
 */
extern tfsFile *
tfs_fopen(tfsMount *tfs, tfsUByte *name);

/**
 * Close the file descriptor.
 *
 * @param [in] fp The file descriptor to close.
 */
extern void
tfs_fclose(tfsFile *fp);

/**
 * Read from the file descriptor.
 *
 * @param [in] fp The file descriptor to read from.
 *
 * @return The number of items that have been read.
 */
extern size_t
tfs_fread(void *dest, size_t size, tfsFile *fp);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif
#endif /* __TFS_H */
