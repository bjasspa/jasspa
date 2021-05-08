/* -*- C -*- ****************************************************************
 *
 *  System        : Built-in File System
 *  Module        : Build the archive.
 *  Object Name   : $RCSfile: ubuild.c,v $
 *  Revision      : $Revision: 1.1 $
 *  Date          : $Date: 2010-08-30 15:12:48 $
 *  Author        : $Author: bill $
 *  Created By    : Jon Green
 *  Created       : Wed Oct 21 16:54:05 2009
 *  Last Modified : <210426.0841>
 *
 *  Description
 *
 *  Notes
 *
 *  History
 *
 ****************************************************************************
 *
 *  Copyright (c) 2009 Jon Green.
 *
 *  All Rights Reserved.
 *
 * This  document  may  not, in  whole  or in  part, be  copied,  photocopied,
 * reproduced,  translated,  or  reduced to any  electronic  medium or machine
 * readable form without prior written consent from Jon Green.
 *
 ****************************************************************************/

#include "tfsutil.h"

/* Compression */
#include <zlib.h>

/*
 * TODO: Sort the problem.
 * Windows temporary file tmpfp() does not see to work on Vista use a
 * horrible name temorarily??
 *
 */
#define TMP_FILE "tfs~~~~~.tmp"

/*
 * Debugging.
 */
#define TFSBUILD_DEBUG  1               /* Logging enable == 1; disable == 0 */

#if TFSBUILD_DEBUG
#define TFS_DEBUG(x) printf x
#else
#define TFS_DEBUG(x) /* NULL */
#endif

/**
 * The structure of the binary file system pointer.
 */
typedef struct s_tfs_fp
{
    /* The file pointer. */
    FILE *fp;
    /* The base offset */
    off_t base_offset;
    /* The current file offset. */
    off_t offset;
    /* The number of bytes written. This should be the same as offset. */
    off_t written;
    /* The first node root directory */
    off_t root_node;
    /* The first free node or zero if there is no free space. */
    off_t free_node;
    /* The total number of bytes used. */
    off_t numbytes;
    /* The estimated length of a offset in bytes. */
    off_t offset_bytes;
    /* The actual length of the archive. */
    off_t actual_len;
} tfsfp_t;

/**
 * @struct s_binfs_attribute
 * Structure of an attribute.
 */
typedef struct s_tfs_attribute
{
    /** The next attribute */
    struct s_tfs_attribute *next;
    /** Identifier associated with the attribute */
    unsigned char tag;
    /** The length of the attribute. */
    size_t len;
    /** The data associated with the attribute. */
    unsigned char data[1];
} tfsAttribute_t;

/**
 * @struct s_binfs_node
 * Define the in-memory basic structure of a node.
 */
typedef struct s_tfs_node
{
    /** The type of node */
    unsigned char type;
    /** The offset of the node. */
    off_t offset;
    /** The node name */
    char *name;
    /** The length of the name */
    size_t name_len;
    /** File system name relative path. */
    char *fsname;
    /** Parent node. */
    struct s_tfs_node *parent;
    /** Next sibling node */
    struct s_tfs_node *sibling;
    /** Next child node */
    struct s_tfs_node *child;
    /** The payload name of the node defined in the payload section */
    unsigned char *data;
    /** The encoded version of the data */
    unsigned char *encode;
    /** The length of the data. */
    off_t data_len;
    /** The length of the encoded data. */
    off_t encode_len;
    /** The name encoding */
    unsigned char encode_type;
    /** Pointer to the list of name attributes */
    struct s_tfs_attribute *attribute;
} tfsNode_t;

/**
 * Write out 'n' bytes of data.
 *
 * @param [in] tfsfp The tfsfp file descriptor.
 * @param [in] value A integer value to write.
 * @param [in] len   The number of bytes to write.
 *
 * @return The number of bytes written.
 */
static int
tfsfpPutn (tfsfp_t *tfsfp, off_t value, int len)
{
    unsigned char buf[8];
    int ii;
    int written = 0;

    if ((ii = len) > 0)
    {
        /* Put out the integer backwards */
        do
        {
            --ii;
            buf [ii] = (unsigned char)(value & 0xff);
            value = value >> 8;
        }
        while (ii > 0);

        /* Put out the bytes MSB first. */
        for (ii = 0; ii < len; ii++)
            if (fputc (buf[ii] , tfsfp->fp) != EOF)
                written++;
        tfsfp->written += written;
        tfsfp->offset += len;
    }
    /* Return the status */
    return len;
}

#define tfsfpPut8(tfsfp,ii)   tfsfpPutn (tfsfp, ii, 1)
#define tfsfpPut16(tfsfp,ii)  tfsfpPutn (tfsfp, ii, 2)
#define tfsfpPut24(tfsfp,ii)  tfsfpPutn (tfsfp, ii, 3)
#define tfsfpPut32(tfsfp,ii)  tfsfpPutn (tfsfp, ii, 4)
#define tfsfpPut64(tfsfp,ii)  tfsfpPutn (tfsfp, ii, 8)

/**
 * Write out 'n' bytes of static data.
 *
 * @param [in] tfsfp The tfsfp file descriptor.
 * @param [in] c     The character to write.
 * @param [in] len   The number of bytes to write.
 *
 * @return The number of bytes written.
 */
static int
tfsfpPutc (tfsfp_t *tfsfp, unsigned char c, int len)
{
    int written = 0;

    if (len > 0)
    {
        int ii;

        for (ii = 0; ii < len; ii++)
            if (fputc (c , tfsfp->fp) != EOF)
                written++;
        tfsfp->written += written;
        tfsfp->offset += len;
    }
    /* Return the status */
    return len;
}

/**
 * Write out a vuimsbf value.
 *
 * @param [in] tfsfp The tfsfp file descriptor.
 * @param [in] value A integer value to write.
 *
 * @return The number of bytes written.
 */
static int
tfsfpPutv (tfsfp_t *tfsfp, off_t value)
{
    unsigned char buf[8];               /* Temporary accumulation buffer */
    unsigned char *p = &buf[7];         /* Point to last byte in the buffer. */
    int ii;                             /* Temporary loop counter. */
    int count = 0;                      /* Count of number of bytes. */
    int written = 0;                    /* Number of bytes written */

    /* Reverse process the string. */
    do
    {
        /* See if the value is zero. */
        if ((value & ~0x1f) == 0)
            break;
        /* Integer exceeds 0x1f so add a byte */
        *p-- = (unsigned char)(value & 0xff);
        value = value >> 8;
        count++;
    }
    while (count < 7);
    /* Add any residue information and set up the counter. Increment the
     * counter as there is always at least 1 byte. */
    *p = (unsigned char)(((count & 0x7) << 5) | (value & 0x1f));
    count++;

    /* Put out the bytes MSB first. */
    for (ii = 0; ii < count; ii++, p++)
        if (fputc (*p , tfsfp->fp) != EOF)
            written++;
    tfsfp->written += written;
    tfsfp->offset += count;

    /* Return the status */
    return count;
}

/**
 * Write out a fixed length vuimsbf value at a given length.
 *
 * @param [in] tfsfp The tfsfp file descriptor.
 * @param [in] value A integer value to write.
 * @param [in] len   The length of the data to write.
 *
 * @return The number of bytes written.
 */
static int
tfsfpPutvn (tfsfp_t *tfsfp, off_t value, int len)
{
    unsigned char buf[8];                     /* Temporary accumulation buffer */
    unsigned char *p = &buf[7];               /* Point to last byte in the buffer. */
    int ii;                             /* Temporary loop counter. */
    int written = 0;                    /* Number of bytes written */

    /* Truncate the word. */
    len = (len - 1) & 7;

    /* Construct the bytes MSB first. */
    for (ii = 0; ii < len; ii++)
    {
        *p-- = (unsigned char)(value & 0xff);
        value = value >> 8;
    }
    *p = (unsigned char)((value & 0x1f) | ((len << 5) & 0xe0));

    /* Write out the bytes */
    for (ii = 0; ii <= len; ii++, p++)
        if (fputc (*p , tfsfp->fp) != EOF)
            written++;

    /* Update the offsets. */
    tfsfp->written += written;
    tfsfp->offset += len + 1;

    /* Return the status */
    return len + 1;
}

/**
 * Find the length of vuimsbf value
 *
 * @param [in] value A integer value to determine the storage length.
 *
 * @return The number of bytes required.
 */
static int
tfsfpLenv (off_t value)
{
    int len = 1;                        /* Must be at least 1-byte */

    /* Reverse process the string. */
    while ((value & ~0x1f) != 0)
    {
        value = value >> 8;
        len++;
    }

    /* Return the status */
    return len;
}

#ifdef _NOT_USED
/**
 * Writes a file given a name to the archive.
 *
 * @param [in] tfsfp The tfsfp file descriptor.
 * @param [in] name  The name of the file to insert.
 *
 * @return The number of bytes written.
 */
static int
tfsfpPutfn (tfsfp_t *tfsfp, char *name)
{
    int written = 0;
    int count = 0;

    if (name != NULL)
    {
        FILE *fp;

        /* Open the file for reading. */
        if ((fp = fopen (name, "rb")) != NULL)
        {
            unsigned char buf [1024];
            int n;

            do
            {
                /* Read in a chunk. */
                n = fread (buf, 1, sizeof (buf), fp);
                if (n > 0)
                {
                    count += n;
                    written += fwrite (buf, 1, n, tfsfp->fp);
                }
            }
            while (n == sizeof (buf));

            /* Close the file. */
            fclose (fp);

            /* Increment the counters. */
            tfsfp->written += written;
            tfsfp->offset += count;

            TFS_DEBUG (("Read \"%s\": read=%d, written=%d\n", name, count, written));
        }
    }

    /* Return the number of bytes we read. */
    return count;
}
#endif

/**
 * Writes a file given a name to the archive.
 *
 * @param [in] tfsfp The tfsfp file descriptor.
 * @param [in] fp    The file pointer to write.
 *
 * @return The number of bytes written.
 */
static int
tfsfpPutfp (tfsfp_t *tfsfp, FILE *fp)
{
    int written = 0;
    int count = 0;

    if (fp != NULL)
    {
        unsigned char buf [1024];
        int n;

        do
        {
            /* Read in a chunk. */
            n = fread (buf, 1, sizeof (buf), fp);
            if (n > 0)
            {
                count += n;
                written += fwrite (buf, 1, n, tfsfp->fp);
            }
        }
        while (n == sizeof (buf));

        /* Increment the counters. */
        tfsfp->written += written;
        tfsfp->offset += count;

        TFS_DEBUG (("Read FP: read=%d, written=%d\n", count, written));
    }

    /* Return the number of bytes we read. */
    return count;
}

/**
 * Writes a block of data to the archive.
 *
 * @param [in] tfsfp The tfsfp file descriptor.
 * @param [in] buf   The buffer to write.
 * @param [in] len   The length of data to write.
 *
 * @return The number of bytes written.
 */
static int
tfsfpPutbuf (tfsfp_t *tfsfp, unsigned char *buf, int len)
{
    int written = 0;

    if ((buf != NULL) && (len > 0))
    {
        written += fwrite (buf, 1, len, tfsfp->fp);

        /* Increment the counters. */
        tfsfp->written += written;
        tfsfp->offset += len;
    }

    /* Return the number of bytes we read. */
    return len;
}

/**
 * Write out a vuimsbf value to a buffer.
 *
 * @param [in] tfsfp The tfsfp file descriptor.
 * @param [in] value A integer value to write.
 *
 * @return The number of bytes written.
 */
static int
bufPutv (unsigned char *buf, off_t value)
{
    unsigned char tmp[8];                     /* Temporary accumulation buffer */
    unsigned char *p = &tmp[7];               /* Point to last byte in the buffer. */
    int ii;                             /* Temporary loop counter. */
    int count = 0;                      /* Count of number of bytes. */

    /* Reverse process the string. */
    do
    {
        /* See if the value is zero. */
        if ((value & ~0x1f) == 0)
            break;
        /* Integer exceeds 0x1f so add a byte */
        *p-- = (unsigned char)(value & 0xff);
        value = value >> 8;
        count++;
    }
    while (count < 7);
    /* Add any residue information and set up the counter. Increment the
     * counter as there is always at least 1 byte. */
    *p = (unsigned char)(((count & 0x7) << 5) | (value & 0x1f));
    count++;

    /* Put out the bytes MSB first. */
    for (ii = 0; ii < count; ii++, p++)
        *buf++ = *p;

    /* Return the status */
    return count;
}

#ifdef _NOT_USED
/**
 * Write out 'n' bytes of data.
 *
 * @param [in] buf The destination buffer.
 * @param [in] value A integer value to write.
 * @param [in] len   The number of bytes to write.
 *
 * @return The number of bytes written.
 */
static int
bufPutn (unsigned char *buf, off_t value, int len)
{
    unsigned char tmp[8];
    int ii;

    if ((ii = len) > 0)
    {
        /* Put out the integer backwards */
        do
        {
            --ii;
            tmp [ii] = (unsigned char)(value & 0xff);
            value = value >> 8;
        }
        while (ii > 0);

        /* Put out the bytes MSB first. */
        for (ii = 0; ii < len; ii++)
            *buf++ = tmp [ii];
    }
    /* Return the status */
    return len;
}
#endif

#ifdef _NOT_USED
/**
 * Read in a single byte of data.
 *
 * @param [in] tfsfp The tfsfp file descriptor.
 *
 * @return The byte read
 */
static int
tfsfpGetc (tfsfp_t *tfsfp)
{
    return fgetc (tfsfp->fp);
}
#endif

/**
 * Get a vuimsbf value
 *
 * @param [in] tfsfp The tfsfp file descriptor.
 *
 * @return The number of bytes written.
 */
static off_t
tfsfpGetv (tfsfp_t *tfsfp)
{
    int cc;
    int ii;
    off_t value = 0;

    if ((cc = fgetc (tfsfp->fp)) != EOF)
    {
        value = (off_t)(cc & 0x1f);
        ii = (cc >> 5) & 0x7;
        while (--ii >= 0)
        {
            cc = fgetc (tfsfp->fp);
            value = (value << 8) | (off_t)(cc & 0xff);
        }
    }
    /* Return the value */
    return value;
}

/**
 * Construct a new node.
 *
 * @return A pointer to the new node or NULL on an error.
 */
static tfsNode_t *
nodeConstruct (void)
{
    tfsNode_t *np;

    /* Construct the node */
    if ((np = calloc (1, sizeof (tfsNode_t))) != NULL)
    {
        /* No significant initialisation. If there is any then add it here. */
        ;
    }

    /* Return the new node. */
    return np;
}

/**
 * Perform a pre-order traversal and get the next node.
 *
 *   >       0       Level 0
 *   >      /|\
 *   >     1 2 3     Level 1
 *   >      / \ \
 *   >     4   5 6   Level 2
 *   >    / \   \
 *   >   7   8   9   level 3
 *
 * We expect the following:-
 *
 * Pre-order:  0, 1, 2, 4, 7, 8, 5, 9, 3, 6, e
 * In-order:   1, 0, 7, 4, 8, 2, 9, 5, 6, 3, e
 * Post-order: 1, 7, 8, 4, 9, 5, 2, 6, 3, 0, e
 *
 * @param [in] node The starting node.
 *
 * @return The next pre-order node or NULL if there is no next.
 */

static tfsNode_t *
nodePreOrderNext (tfsNode_t *node)
{
    int up = 0;

    while (node != NULL)
    {
        tfsNode_t *p;

        if ((up == 0) && ((p = node->child) != NULL))
            return p;
        else if ((p = node->sibling) != NULL)
            return p;
        node = node->parent;
        up = 1;
    }
    return node;
}

/**
 * The first node of the traversal given by the leftmost child of the tree.
 * DIRECTORIES ONLY.
 *
 * @param [in] node The root node.
 *
 * @return The first child node.
 */
static tfsNode_t *
nodePostOrderFirst (tfsNode_t *node)
{
    tfsNode_t *np;

    /* Descent to the leftmost child */
    while ((np = node->child) != NULL)
        node = np;
    return node;
}

/**
 * Perform a post-order traversal and get the next node.
 *
 *   >       0       Level 0
 *   >      /|\
 *   >     1 2 3     Level 1
 *   >      / \ \
 *   >     4   5 6   Level 2
 *   >    / \   \
 *   >   7   8   9   level 3
 *
 * We expect the following:-
 *
 * Pre-order:  0, 1, 2, 4, 7, 8, 5, 9, 3, 6, e
 * In-order:   1, 0, 7, 4, 8, 2, 9, 5, 6, 3, e
 * Post-order: 1, 7, 8, 4, 9, 5, 2, 6, 3, 0, e
 *
 * @param [in] node The starting node.
 *
 * @return The next post-order node or NULL if there is no next.
 */

static tfsNode_t *
nodePostOrderNext (tfsNode_t *node)
{
    int up = 0;

    do
    {
        tfsNode_t *np;

        /* Decend to the leaf node if possible */
        if ((np = node->sibling) != NULL)
        {
            while ((node = np->child) != NULL)
                np = node;
            return np;
        }
        /* Ascend to the parent */
        else if (up == 0)
            return node->parent;
        /* Mark that we have moved */
        up = 1;
    }
    while ((node = node->parent) != NULL);
    return node;
}

/**
 * Destruct a node. Note that this does not perform an unlink,
 *
 * @param [in] node The node to be destructed.
 */
static void
nodeDestruct (tfsNode_t *node)
{
    if (node != NULL)
    {
        tfsAttribute_t *ap;

        /* Free off the locally allocated memory */
        if (node->name != NULL)
            free (node->name);
        if (node->fsname != NULL)
            free (node->fsname);
        /* Destruct the data payloads. */
        if (node->data != NULL)
            free (node->data);
        if (node->encode != NULL)
            free (node->encode);
        /* Destruct the attributes. */
        while ((ap = node->attribute) != NULL)
        {
            node->attribute = ap->next;
            free (ap);
        }
    }
}

/**
 * Link a child node to the parent and perform an insertion sort.
 *
 * @param [in] parent The parent node.
 * @param [in] child The child node to be linked.
 *
 * @return The status of the call.
 * @retval 0 The insertion was sucessful.
 * @retval 1 The parent does not exist.
 * @retval 2 The node is duplicated, no insertion performed.
 */
static int
nodeLinkChild (tfsNode_t *parent, tfsNode_t *child)
{
    tfsNode_t **npp;
    tfsNode_t *np;

    /* A few quick basic checks for sanity. */
    if (parent == NULL)
        return 1;
    if (child == NULL)
        return 0;

    /* Iterate over the list of children and insert. */
    for (npp = &parent->child; (np = *npp) != NULL; npp = &np->sibling)
    {
        int status;

        status = strcmp ((char *)(child->name), (char *)(np->name));
        if (status == 0)
            return 2;
        else if (status < 0)
            break;
    }

    /* Add the node */
    child->sibling = *npp;
    *npp = child;
    child->parent = parent;
    return (0);
}

/**
 * Create a new node as a child of an existing node. The node is addded as a
 * sibling link of the children and is alphabetically sorted.
 *
 * @param [in] parent The parent node.
 * @param [in] pathname The pathname of the file system object to add.
 * @param [in] name The name of the childnode to link.
 * @param [in] type The type of node to create i.e. TFS_TYPE_FILE.
 *
 * @return A pointer to the new child node that was created,
 * @retval NULL The call failed.
 * @retval !NULL The call was successful, pointer to the new child node.
 */
static tfsNode_t *
nodeAddChild (tfsNode_t *parent, char *pathname, char *name, unsigned char type)
{
    tfsNode_t *np;

    /* Construct the node */
    if ((np = nodeConstruct ()) != NULL)
    {
        /* Initialise the node. */
        np->type = type;
        np->name = strdup (name);
        np->name_len = strlen (name);
        np->fsname = strdup (pathname);

        /* Link the node to the parent. */
        if (nodeLinkChild (parent, np) != 0)
        {
            nodeDestruct (np);
            np = NULL;
        }
    }

    /* Return the node to the caller. */
    return np;
}

/**
 * Process the directory tree build the nodes. The base directory passed in
 * the call is recursivelly traversed to build up the list of files and
 * directories.
 *
 * @param [in] tfsfp The binary file system pointer. Used to accumulate
 *                   the file size only.
 * @param [in] node The root node to process.
 * @param [in] basedir The base directory.
 */
#ifdef _WIN32
static int
processDirectory (tfsfp_t *tfsfp, tfsNode_t *node, char *basedir)
{
    char *pe, pathname [PATH_MAX];
    int status = 0;
    HANDLE *fh ;
    WIN32_FIND_DATA  fd ;
    off_t estimate = 0;
    tfsNode_t *np;
    unsigned char type = TFS_TYPE_UNDEF;
    off_t data_len ;

    strcpy (pathname, basedir);
    pe = pathname + strlen(pathname) ;
    *pe++ = '\\' ;
    strcpy(pe,"*.*") ;
    
    /* Process the directory */
    fh = FindFirstFile(pathname,&fd) ;
    if (fh == INVALID_HANDLE_VALUE)
    {
        printf ("Error opening \"%s\"\n", basedir);
        return (1);
    }
    
    /* Iterate over the node and process all of the entries */
    do
    {
        TFS_DEBUG (("Processing: %s/%s\n", basedir, fd.cFileName));

        /* Ignore the current directory */
        if (strcmp (fd.cFileName, ".") == 0)
            continue;
        /* Ignore the parent directory */
        if (strcmp (fd.cFileName, "..") == 0)
            continue;

        /* Construct the full path */
        strcpy (pe, fd.cFileName);

        /* Find the basic attributes of the file. */
        TFS_DEBUG (("stat: %s = %d\n", pathname, fd.dwFileAttributes));

        /* Find the type */
        if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            type = TFS_TYPE_DIR;
            data_len = 0 ;
            /* Assume 10-byte directory node */
            estimate += 10;
        }
        else if(fd.nFileSizeHigh != 0)
        {
            printf ("File \"%s\" too large - skipping\n", pathname);
            type = TFS_TYPE_UNDEF;
        }
        else
        {
            type = TFS_TYPE_FILE;
            data_len = fd.nFileSizeLow;
        }
        if (type != TFS_TYPE_UNDEF)
        {
            /* Assume the node has a directory entry of 3 bytes + the
             * length of the name + the length of the data */
            estimate += 10 + 3 + strlen (fd.cFileName) + data_len;
            
            /* Add node to the construction tree. */
            if ((np = nodeAddChild (node, pathname, fd.cFileName, type)) != NULL)
            {
                np->data_len = data_len;
                TFS_DEBUG (("Added: [%d] %s @ %d\n",
                            type, pathname, (int)(data_len)));
            }
            else
            {
                printf ("ERROR adding: [%d] %s\n", type, pathname);
                status |= 2;
            }
        }
    } while (FindNextFile(fh, &fd));
    FindClose(fh);

    /* Add the size estimate to the file system size. */
    if (tfsfp != NULL)
        tfsfp->offset += estimate;

    /* Tail recurse over the directories that we have created. The compiler
     * might be smart enough to work out that this is a tail recursion
     * operation and not stack it, but not to worry. */
    for (node = node->child; node != NULL; node = node->sibling)
        if (node->type == TFS_TYPE_DIR)
            status |= processDirectory (tfsfp, node, node->fsname);

    return status;
}
#else
static int
processDirectory (tfsfp_t *tfsfp, tfsNode_t *node, char *basedir)
{
    int status = 0;
    DIR *base;
    struct dirent *dp;
    off_t estimate = 0;

    /* Process the directory */
    base = opendir (basedir);
    if (base == NULL)
    {
        printf ("Error opening \"%s\"\n", basedir);
        return (1);
    }

    /* Iterate over the node and process all of the entries */
    while ((dp = readdir (base)) != NULL)
    {
        struct stat statbuf;
        char pathname [PATH_MAX];

        TFS_DEBUG (("Processing: %s/%s\n",
                    strnil (basedir), strnil (dp->d_name)));

        /* Ignore the current directory */
        if (strcmp (dp->d_name, ".") == 0)
            continue;
        /* Ignore the parent directory */
        if (strcmp (dp->d_name, "..") == 0)
            continue;

        /* Construct the full path */
        strcpy (pathname, basedir);
        strcat (pathname, "/");
        strcat (pathname, dp->d_name);

        /* Find the basic attributes of the file. */
        if (stat (pathname, &statbuf) != -1)
        {
            tfsNode_t *np;
            unsigned char type = TFS_TYPE_UNDEF;
            off_t data_len = 0;

            TFS_DEBUG (("stat: %s = %d\n", strnil (pathname), (int) statbuf.st_size));

            /* Find the type */
            if ((statbuf.st_mode & S_IFDIR) != 0)
            {
                type = TFS_TYPE_DIR;
                /* Assume 10-byte directory node */
                estimate += 10;
            }
            else if ((statbuf.st_mode & S_IFREG) != 0)
            {
                type = TFS_TYPE_FILE;
                data_len = statbuf.st_size;
            }
            if (type != TFS_TYPE_UNDEF)
            {
                /* Assume the node has a directory entry of 3 bytes + the
                 * length of the name + the length of the data */
                estimate += 10 + 3 + strlen (dp->d_name) + data_len;

                /* Add node to the construction tree. */
                if ((np = nodeAddChild (node, pathname, dp->d_name, type)) != NULL)
                {
                    np->data_len = data_len;
                    TFS_DEBUG (("Added: [%d] %s @ %d\n",
                                type, strnil (pathname), (int)(data_len)));
                }
                else
                {
                    printf ("ERROR adding: [%d] %s\n", type, pathname);
                    status |= 2;
                }
            }
        }
        else
        {
            printf ("Stat failed on [%d] \"%s\"\n", errno, strnil (pathname));
            perror ("Stat failed");
        }
    }

    closedir (base);

    /* Add the size estimate to the file system size. */
    if (tfsfp != NULL)
        tfsfp->offset += estimate;

    /* Tail recurse over the directories that we have created. The compiler
     * might be smart enough to work out that this is a tail recursion
     * operation and not stack it, but not to worry. */
    for (node = node->child; node != NULL; node = node->sibling)
        if (node->type == TFS_TYPE_DIR)
            status |= processDirectory (tfsfp, node, node->fsname);

    return status;
}
#endif

/**
 * Build the directory node.
 *
 * @param [in] node The directory node to build.
 */
static int
buildDirectory (tfsNode_t *node)
{
    tfsNode_t *np;
    int flags;
    int buf_size = 0;

    /* Iterate over all of the children. */
    for (np = node->child; np != NULL; np = np->sibling)
    {
        /* Work out the current type */
        if (np->type == TFS_TYPE_FILE)
            flags = 1;
        else if (np->type == TFS_TYPE_DIR)
            flags = 2;
        else
            continue;

        /* flags */
        buf_size += tfsfpLenv (flags);
        /* node */
        buf_size += tfsfpLenv (np->offset);
        /* name_len + name */
        buf_size += tfsfpLenv (np->name_len) + np->name_len;
    }

    /* Allocate the buffer. */
    if(buf_size == 0)
    {
        node->data = NULL;
        node->data_len = 0;
    }
    else if ((node->data = malloc (buf_size)) == NULL)
        node->data_len = 0;
    else
    {
        unsigned char *pp;

        /* Point to the start of the data. */
        pp = node->data;

        /* Iterate over all of the children. */
        for (np = node->child; np != NULL; np = np->sibling)
        {
            unsigned int ii;

            /* Current directory - node */
            pp += bufPutv (pp, np->offset);
            /* Current directory - name_len */
            pp += bufPutv (pp, np->name_len);
            /* Current directory - name */
            for (ii = 0; ii < np->name_len; ii++)
                *pp++ = np->name[ii];
            TFS_DEBUG (("Added node \"%s\" to directory\n", strnil (np->fsname)));
        }

        /* Set the data length. */
        node->data_len = pp - node->data;
        assert (node->data_len != 0);
        assert (node->data_len <= buf_size);
        TFS_DEBUG (("Dir Allocate = %p@%d ?%d\n",
                    node->data, (int)(node->data_len), buf_size));
    }

    /* Completed. */
    return 0;
}

/**
 * Compress a node to a temporary file to get the size.
 *
 * @param [in] node The node to process.
 *
 * @return A non-null file pointer to encode or NULL if the original data will
 * be encoded.
 */
static FILE *
nodeCompress (tfsNode_t *node)
{
#define WORKING_BUFLEN  (128*1024)

    FILE *ufp = NULL;
    FILE *cfp;
    unsigned char uworking[WORKING_BUFLEN];
    unsigned char *ubuf = uworking;
    size_t ulen = 0;
    unsigned char cworking[WORKING_BUFLEN];
    unsigned char *cbuf = cworking;
    size_t clen = 0;
    int rlen;

    /* Assume that we are not compressing. */
    node->encode_type = TFS_ENCODE_NONE;
    /* If this is a file node then try to open the file and compress it. */
    if (node->type == TFS_TYPE_FILE)
    {
        /* Open the file. */
        if ((ufp = fopen (node->fsname, "rb")) != NULL)
        {
            /* Read in a buffer full */
            rlen = fread (ubuf, 1, WORKING_BUFLEN, ufp);
            if (rlen <= 10)
            {
                /* The file is short, return the read file pointer. */
                rewind (ufp);
                fprintf (stderr, "File is short skip compression \"%s\"\n", node->fsname);
                return ufp;
            }
            ulen = rlen;
        }
    }
    else if (node->type == TFS_TYPE_DIR)
    {
        /* This is a directory node. */
        if (node->data_len < 10)
            return ufp;                 /* Not worth compressing. */
        
        /* DO Not compress !! */
        return NULL;
    }
    else
        return NULL;

    /* At this point then we have some data that might be worth compressing.
     * Open a temporary file into which we will compress the data. */
    
#ifdef _WIN32
    remove (TMP_FILE);
    if ((cfp = fopen (TMP_FILE, "wb+")) != NULL)
#else
    if ((cfp = tmpfile ()) != NULL)
#endif
    {
        z_stream c_stream;              /* compression stream */
        int zstatus;                    /* Status of the compression. */
        int zflush;                     /* The flush type. */

        /* Initialise the compression structure */
        zflush = Z_NO_FLUSH;

        /* Initialise the deflate algorithm */
        c_stream.zalloc = Z_NULL;
        c_stream.zfree = Z_NULL;
        c_stream.opaque = Z_NULL;
        zstatus = deflateInit (&c_stream, Z_BEST_COMPRESSION);

        if (zstatus != Z_OK)
        {
            /* Cannot compress return the original file. Return the file
             * pointer to the start and return the file pointer. */
            printf ("Cannot compress \"%s\"\n", strnil (node->fsname));
            fclose (cfp);
            if (ufp != NULL)
                rewind (ufp);
            return ufp;                 /* Fault */
        }

        /* initialise the stream structure */
        c_stream.next_out = cbuf;
        c_stream.avail_out = WORKING_BUFLEN;
        if (node->type == TFS_TYPE_DIR)
        {
            c_stream.next_in = node->data;
            c_stream.avail_in = (int)(node->data_len);
            ulen = node->data_len;
        }
        else
        {
            c_stream.next_in = ubuf;    /* Uncompressed buffer. */
            c_stream.avail_in = (int)(rlen);
        }

        /* Repeat the compression by writing to the output file. */
        do
        {
            /* Deflate the input. */
            zstatus = deflate (&c_stream, zflush);
            if (zstatus == Z_OK)
            {
                /* Run out out input data. */
                if (c_stream.avail_in == 0)
                {
                    if (zflush == Z_NO_FLUSH)
                    {
                        /* For a valid file pointer then try to read some
                         * more data. */
                        if (ufp != NULL)
                            c_stream.avail_in = fread (ubuf, 1, WORKING_BUFLEN, ufp);
                        if (c_stream.avail_in == 0)
                            zflush = Z_FINISH;
                        else
                            ulen += c_stream.avail_in;
                        /* Reset the buffer pointer. */
                        c_stream.next_in = ubuf;
                    }
                }
                /* Check for no output. */
                if (c_stream.avail_out == 0)
                {
                    clen += fwrite (cbuf, 1, WORKING_BUFLEN, cfp);
                    c_stream.next_out  = cbuf;
                    c_stream.avail_out = WORKING_BUFLEN;
                }
            }
            /* Check if we are flushing the output. */
            else if (zstatus == Z_STREAM_END)
            {
                int len;

                len = WORKING_BUFLEN - c_stream.avail_out;
                TFS_DEBUG (("Z_STREAM_END: avail_out = %d, buflen = %d, len=%d\n",
                            c_stream.avail_out, WORKING_BUFLEN, len));

                if (len > 0)
                    clen += fwrite (cbuf, 1, len, cfp);
            }
        }
        while (zstatus == Z_OK);

        /* Check for the end of the stream. */
        if (zstatus == Z_STREAM_END)
        {
            /* End the compression and flush the output. */
            zstatus = deflateEnd (&c_stream);
            fflush (cfp);
        }
        /* Handle the final compression status */
        if (zstatus == Z_OK)
        {
            /* Make a decision as to what we are going to return. */
            TFS_DEBUG (("Compressed file \"%s\" %ld=>%ld (%ld%%)\n",
                        strnil(node->fsname), ulen, clen, (ulen * 100) / clen));
            fprintf (stderr,
                     "Compressed file \"%s\" %ld=>%ld (%ld%%)\n",
                     strnil(node->fsname), ulen, clen, (ulen * 100) / clen);
            
            if (((ulen * 100) / clen) > 110)
            {
                /* We are going to take the compressed version set up the
                 * node to convey zlib compression. */
                assert (c_stream.total_out == clen);
                node->encode_len = c_stream.total_out;
                node->encode_type = TFS_ENCODE_ZLIB;
                /* Close the uncompressed file */
                if (ufp != NULL)
                    fclose (ufp);
                /* Rewind the compressed file pointer back to the start and
                 * return the compressed file pointer. */
                fflush (cfp);
                rewind (cfp);
                return cfp;
            }
            else
            {
                TFS_DEBUG (("Use uncompressed \"%s\"\n", strnil (node->fsname)));
            }
        }
    }
    else
        fprintf (stderr, "Cannot open temporary file for compression on \"%s\"\n", node->fsname);
        

    /* Close the temporary compression stream. */
    if (cfp != NULL)
    {
        fclose (cfp);
#ifdef _WIN32
        remove (TMP_FILE);
#endif
    }
    
    /* Rewind the uncompressed file pointer. */
    if (ufp != NULL)
        rewind (ufp);
    return ufp;
}

/**
 * Write the node data to the file.
 *
 * @param [in] tfsfp The tfsfp file descriptor.
 * @param [in] node The node to write.
 */
static void
nodeWrite (tfsfp_t *tfsfp, tfsNode_t *node)
{
    off_t datum;
    int encoding;
    int aux_flag;
    FILE *fp;

    TFS_DEBUG (("nodeWrite: %s @ %d\n", strnil (node->fsname),
                (int)(node->data_len & 0xffffffff)));

    /* Save the current byte offset */
    node->offset = tfsfp->offset;

    /* Work out the current type; if it is a directory then build it. */
    if (node->type == TFS_TYPE_DIR)
    {
        buildDirectory (node);
        TFS_DEBUG (("nodeWrite[DIR]: %s @ %d\n",
                    strnil (node->fsname), (int)(node->data_len & 0xffffffff)));
    }

    /* Write the node block */
    fp = nodeCompress (node);

    /* leader */
    /* tfsfpPut8 (tfsfp, '@');*/

    /* attrib_flag */
    datum = (node->attribute == NULL) ? 0 : 0x80;
    /* encoding 0=none */
    encoding = (node->encode_type & 0x07) << 4;
    /* auxillary_flag */
    if ((node->type == TFS_TYPE_DIR) ||
        (node->type == TFS_TYPE_MOUNT))
        aux_flag = 0x08;
    else
        aux_flag = 0x00;
    /* type */
    datum |= encoding | aux_flag | (node->type & 0x07);
    tfsfpPut8 (tfsfp, datum);

    /* data_length */
    if (encoding == 0)
        tfsfpPutv (tfsfp, node->data_len);
    else
    {
        tfsfpPutv (tfsfp, node->encode_len);
        /* original_length */
        tfsfpPutv (tfsfp, node->data_len);
    }

    /* Add the auxillary data */
    if ((aux_flag != 0) &&
        ((node->type == TFS_TYPE_DIR) || (node->type == TFS_TYPE_MOUNT)))
    {
        /* Placeholder for the parent node */
        tfsfpPutvn (tfsfp, 0, tfsfp->offset_bytes);
    }

    /* attributes */
    if (node->attribute != NULL)
    {
        /* TODO: Need to handle the attributes. */
    }

    /* Put the data_bytes */
    if (fp != NULL)
    {
        datum = tfsfpPutfp (tfsfp, fp);
        fclose (fp);
    }
    else if ((node->data != NULL) && (node->data_len != 0))
        datum = tfsfpPutbuf (tfsfp, node->data, node->data_len);
    else if ((node->data == NULL) && (node->data_len != 0))
        datum = tfsfpPutc (tfsfp, 0, node->data_len);

    if (node->encode_type == TFS_ENCODE_NONE)
    {
        if (datum != node->data_len)
            printf ("ERROR: \"%s\" size mismatch %d!=%d\n",
                    strnil (node->fsname), (int)(datum), (int)(node->data_len));
    }
    else
    {
        if (datum != node->encode_len)
            printf ("ERROR: \"%s\" compressed size mismatch %d!=%d\n",
                    strnil (node->fsname), (int)(datum), (int)(node->encode_len));
    }

    /* Put the padding. */
    tfsfpPut8 (tfsfp, 0x0f);
    
    /* Clean up the compression file if it exists */
#ifdef _WIN32
    remove (TMP_FILE);
#endif    
}

/**
 * Write the node data to the file.
 *
 * @param [in] tfsfp The tfsfp file descriptor.
 * @param [in] node The node to write.
 */
static int
nodeWriteTree (tfsfp_t *tfsfp, tfsNode_t *node)
{
    tfsNode_t *np;                      /* Temporary node pointer */

    TFS_DEBUG (("In node write tree\n"));

    /* Perform a post-order traversal over the node tree and establish the
     * names as package data. */
    for (np = nodePostOrderFirst (node);
         (np != NULL) && (np->type != TFS_TYPE_MOUNT);
         np = nodePostOrderNext (np))
    {
        nodeWrite (tfsfp, np);
    }

    /* Set up the root node entry point */
    np = node->child;
    if (np != NULL)
        tfsfp->root_node = np->offset;

    /* Add the trailer. */
    tfsfpPut8 (tfsfp, 'T');
    tfsfpPut8 (tfsfp, 'F');
    tfsfpPut8 (tfsfp, 'S');
    tfsfpPutv (tfsfp, tfsfp->written);

    TFS_DEBUG (("Root_node is at offset %d(0x%08x)\n",
                (int)(tfsfp->root_node), (int)(tfsfp->root_node)));
    return 0;
}

/**
 * Build the mount point block.
 *
 * @param [in] tfsfp The tfsfp file descriptor.
 * @param [in] node The root node to write.
 */
static void
nodeWriteHeader (tfsfp_t *tfsfp, tfsNode_t *root)
{
    /* output the header. */
    tfsfpPut8 (tfsfp, 't');
    tfsfpPut8 (tfsfp, 'f');
    tfsfpPut8 (tfsfp, 's');

    /* Allocate the root data */
    if ((root->data = malloc (root->data_len)) != NULL)
    {
        time_t curr_time;               /* The current time. */
        struct tm *curr_gmttime;        /* Time structure */
        unsigned char *p;
        int ii;

        p = root->data;
        /* version_major */
        *p++ = (unsigned char) tfs_version_major;
        /* version_minor */
        *p++ = (unsigned char) tfs_version_minor;
        /* version_micro */
        *p++ = (unsigned char) tfs_version_micro;
        /* reserved */
        *p++ = 0;

        /* Fill in the time. */
        curr_time = time (NULL);
        if ((curr_gmttime = gmtime (&curr_time)) != NULL)
        {
            /* Year MSB 8 bits */
            ii = curr_gmttime->tm_year + 1900;
            *p++ = (ii >> 4) & 0xff;
            /* Year LSB 4 bits and month */
            *p++ = ((ii << 4) & 0xf0) | (curr_gmttime->tm_mon + 1);
            /* day */
            *p++ = (curr_gmttime->tm_mday << 2) & 0xfc;
            /* hour */
            *p++ = curr_gmttime->tm_hour;
            /* minute */
            *p++ = curr_gmttime->tm_min;
            /* second */
            *p++ = curr_gmttime->tm_sec;
        }
        else
        {
            /* Zero fill. */
            for (ii = 0; ii < 6; ii++)
                *p++ = 0;
        }
        /* free_node - none here so reset. */
        *p++ = 0;
        /* length - fill in later */
        for (ii = 0; ii < tfsfp->offset_bytes; ii++)
        {
            if (ii == 0)
                *p++ = ((tfsfp->offset_bytes - 1) & 7) << 5;
            else
                *p++ = 0;
        }
        /* crc32_data */
        for (ii = 0; ii < 4; ii++)
            *p++ = 0xdd;
        /* crc32_header */
        for (ii = 0; ii < 4; ii++)
            *p++ = 0xcc;

        /* Re-evaluate the data length */
        root->data_len = p - root->data;
    }

    /* Add the mount point node */
    nodeWrite (tfsfp, root);
}

/**
 * Fix the directory nodes ".." offset. Perform an in-order traversal of the
 * tree and fix the nodes.
 *
 * @param [in] tfsfp The tfsfp file descriptor.
 * @param [in] node The root node of the tree.
 */
static void
nodeFix (tfsfp_t *tfsfp, tfsNode_t *root)
{
    tfsNode_t *np;
    off_t crc32data_off = 0;
    off_t crc32head_off = 0;

    /* Flush all of the write data out. */
    fflush (tfsfp->fp);
    rewind (tfsfp->fp);

    /* Iterate over the nodes. */
    for (np = root; np != NULL; np = nodePreOrderNext (np))
    {
        /* Ignore files. */
        if (np->type == TFS_TYPE_FILE)
            continue;
        if (np->type == TFS_TYPE_MOUNT)
        {
            /* Seek to the node position. */
            if (fseeko (tfsfp->fp, tfsfp->base_offset + 3, SEEK_SET) == 0)
            {
                int status;
                off_t datalen;
                off_t aux_flag_off;
                off_t arch_len_off;

                /* Get the type */
                status = fgetc (tfsfp->fp);
                /* Data length */
                datalen = tfsfpGetv (tfsfp);
                /* original_length */
                if ((status & 0x70) != 0)
                    datalen = tfsfpGetv (tfsfp);
                /* Auxillary flag */
                if ((status & 0x08) != 0)
                {
                    aux_flag_off = ftello (tfsfp->fp);
                    datalen = tfsfpGetv (tfsfp);
                }
                else
                    aux_flag_off = 0;
                /* attribute flag */
                if ((status & 0x80) != 0)
                {
                    /* Skip the attribute data */
                    datalen = tfsfpGetv (tfsfp);
                    fseeko (tfsfp->fp, datalen, SEEK_CUR);
                }
                /* Within the data - skip version and time */
                fseeko (tfsfp->fp, 10, SEEK_CUR);
                /* Skip the free_node */
                datalen = tfsfpGetv (tfsfp);
                /* Save the archive length offset and then skip */
                arch_len_off = ftello (tfsfp->fp);
                datalen = tfsfpGetv (tfsfp);
                /* Save the crc32 data and header offsets */
                crc32data_off = ftello (tfsfp->fp);
                if (crc32data_off > 0)
                    crc32head_off = crc32data_off + 4;

                /* Re-write the aux flag */
                if (aux_flag_off > 0)
                {
                    /* Get the position of the file. */
                    status = fseeko (tfsfp->fp, aux_flag_off, SEEK_SET);
                    assert (status == 0);
                    tfsfpPutvn (tfsfp, tfsfp->root_node, tfsfp->offset_bytes);
                    fflush (tfsfp->fp);
                }
                /* Re-write the archive length */
                if (arch_len_off > 0)
                {
                    /* Get the position of the file. */
                    status = fseeko (tfsfp->fp, arch_len_off, SEEK_SET);
                    assert (status == 0);
                    tfsfpPutvn (tfsfp, tfsfp->actual_len, tfsfp->offset_bytes);
                    fflush (tfsfp->fp);
                }
            }
        }
        else if (np->type == TFS_TYPE_DIR)
        {
            /* Seek to the node position. */
            if (fseeko (tfsfp->fp, tfsfp->base_offset + np->offset, SEEK_SET) == 0)
            {
                int status;
                off_t datalen;

                /* Get the type */
                status = fgetc (tfsfp->fp);
                /* Data length */
                datalen = tfsfpGetv (tfsfp);
                /* original_length */
                if ((status & 0x70) != 0)
                    datalen = tfsfpGetv (tfsfp);
                /* Re-write the aux flag */
                if ((status & 0x08) != 0)
                {
                    size_t pos;

                    if (np->parent != NULL)
                        datalen = np->parent->offset;
                    else
                        datalen = 0;

                    pos = ftell (tfsfp->fp);
                    assert (pos > 0);
                    status = fseeko (tfsfp->fp, pos, SEEK_SET);
                    assert (status == 0);
                    tfsfpPutvn (tfsfp, datalen, tfsfp->offset_bytes);
                    fflush (tfsfp->fp);
                }
            }
        }
    }

    /* Compute the CRC-32 for the data. */
    if (crc32head_off != 0)
    {
        /* Advance to the byte after the CRC-32 checksum */
        if (fseeko (tfsfp->fp, crc32head_off + 4, SEEK_SET) == 0)
        {
            unsigned char buf [2048];
            tfsuint32_t crc32 = 0xffffffff;
            int nread;
            
            /* Read in all of the data until we are finished */
            do
            {
                nread = fread (buf, 1, sizeof (buf), tfsfp->fp);
                if (nread > 0)
                    crc32 = tfs_crc32 (crc32, buf, nread);
            }
            while (nread > 0);

            /* Fill in the data CRC-32 */
            if (fseeko (tfsfp->fp, crc32data_off, SEEK_SET) == 0)
            {
                tfsfpPutn (tfsfp, crc32, 4);
                fflush (tfsfp->fp);
            }
        }

        /* Compute the CRC-32 for the header. */
        if (fseeko (tfsfp->fp, tfsfp->base_offset, SEEK_SET) == 0)
        {
            unsigned char buf [2048];
            tfsuint32_t crc32 = 0xffffffff;
            int remain;
            int nread;

            /* Compute the number of remaining bytes in the header. */
            remain = crc32head_off - tfsfp->base_offset;

            /* Read in all of the data until we are finished */
            do
            {
                nread = sizeof (buf);
                if (remain < nread)
                    nread = remain;

                nread = fread (buf, 1, nread, tfsfp->fp);
                if (nread > 0)
                {
                    remain -= nread;
                    crc32 = tfs_crc32 (crc32, buf, nread);
                }
            }
            while ((nread > 0) && (remain > 0));

            /* Fill in the data CRC-32 */
            if (fseeko (tfsfp->fp, crc32head_off, SEEK_SET) == 0)
                tfsfpPutn (tfsfp, crc32, 4);
        }
    }
}

/**
 * Construct a TFS archive.
 *
 * @param [in] options  The option mask.
 * @param [in] fp The file pointer to write.
 * @param [in] direname The directory to process.
 *
 * @return Status of the call, 0 is success.
 */
int
tfs_build (int options, FILE *fp, char *dirname)
{
    int status;
    tfsNode_t *root;                    /* The root node. */
    tfsfp_t tfsfp;

    /* Keep user up to date */
    if ((options & TFSUTIL_OPT_QUIET) == 0)
        fprintf (stderr, "Building from directory \"%s\"\n", strnil (dirname));

    /* Create the mount node */
    root = nodeConstruct ();
    if (root != NULL)
    {
        tfsNode_t *np;

        /* "mount" data block size */
        root->data_len = 30;            /* Maximum size */
        root->type = TFS_TYPE_MOUNT;

        /* Create the root directory. */
        if ((np = nodeConstruct ()) != NULL)
        {
            root->child = np;
            np->parent = root;
            np->type = TFS_TYPE_DIR;
        }
    }

    /* Open up the temporary tfsfp file to write. */
    memset ((char *)(&tfsfp), 0, sizeof (tfsfp));

    /* Process the directory. */
    if ((root != NULL) && (root->child != NULL))
    {
        status = processDirectory (&tfsfp, root->child, dirname);
        /* Calculate the offset size estimate */
        tfsfp.offset_bytes = tfsfpLenv (tfsfp.offset << 1);
        TFS_DEBUG (("Size estimate = %d @ %d bytes\n",
                    (int)(tfsfp.offset), (int)(tfsfp.offset_bytes)));
        /* Reset the offset field. */
        tfsfp.offset = 0;
    }

    /* Seek to the end of the file pointer and get the base offset. */
    if (fseeko (fp, 0, SEEK_END) != 0)
    {
        fprintf (stderr, "Error: Cannot seek to end of archive file.\n");
        return (-1);
    }

    /* Get the offset of the start of the archive */
    tfsfp.base_offset = ftello (fp);
    if (tfsfp.base_offset < 0)
    {
        fprintf (stderr, "Error: Cannot get writing offset\n");
        return (-2);
    }
    if (tfsfp.base_offset != 0)
        fprintf (stderr, "Archive commencing from byte offset %d\n",
                 (int)(tfsfp.base_offset));
    tfsfp.fp = fp;

    /* Build the header. */
    nodeWriteHeader (&tfsfp, root);

    /* Build the node name */
    nodeWriteTree (&tfsfp, root);
    tfsfp.actual_len = tfsfp.written;

    /* Go back and fix the tree */
    nodeFix (&tfsfp, root);

    /* Print out the information. */
    if ((options & TFSUTIL_OPT_QUIET) == 0)
        fprintf (stderr, "Archive constructed %d bytes\n", (int)(tfsfp.actual_len));
    if ((options & TFSUTIL_OPT_QUIET) == 0)
        fprintf (stderr, "Completed archive build from \"%s\" [%s]\n",
                 strnil (dirname), strstatus (0));
    return 0;
}
