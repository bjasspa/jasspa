/*
 * @(#)FileInputStream.java	1.1 98/11/24
 *
 * Copyright 1995-1998 by Sun Microsystems, Inc.,
 * 901 San Antonio Road, Palo Alto, California, 94303, U.S.A.
 * All rights reserved.
 * 
 * This software is the confidential and proprietary information
 * of Sun Microsystems, Inc. ("Confidential Information").  You
 * shall not disclose such Confidential Information and shall use
 * it only in accordance with the terms of the license agreement
 * you entered into with Sun.
 */

package java.io;

import sun.io.FDInputStream;

/**
 * A file input stream is an input stream for reading data from a 
 * <code>File</code> or from a <code>FileDescriptor</code>. 
 *
 * @author  Arthur van Hoff
 * @version 1.1, 11/24/98
 * @see     java.io.File
 * @see     java.io.FileDescriptor
 * @see	    java.io.FileOutputStream
 * @since   JDK1.0
 */
public
class FileInputStream extends InputStream 
{
    
    /**
     * The system dependent File Descriptor Stream.
     * @see sun.io.FDInputStream
     */
    private FDInputStream fdStream;
    
    /**
     * Creates an input file stream to read from a file with the 
     * specified name. 
     *
     * @param      name   the system-dependent file name.
     * @exception  FileNotFoundException  if the file is not found.
     * @exception  SecurityException      if a security manager exists, its
     *               <code>checkRead</code> method is called with the name
     *               argument to see if the application is allowed read access
     *               to the file.
     * @see        java.lang.SecurityManager#checkRead(java.lang.String)
     * @since      JDK1.0
     */
    public FileInputStream(String name) throws FileNotFoundException {
	try {
	    fdStream = new FDInputStream(name);
	} catch (IOException e) {
	    throw new FileNotFoundException(name);
	}
    }
    
    /**
     * Creates an input file stream to read from the specified 
     * <code>File</code> object. 
     *
     * @param      file   the file to be opened for reading.
     * @exception  FileNotFoundException  if the file is not found.
     * @exception  SecurityException      if a security manager exists, its
     *               <code>checkRead</code> method is called with the pathname
     *               of this <code>File</code> argument to see if the
     *               application is allowed read access to the file.
     * @see        java.io.File#getPath()
     * @see        java.lang.SecurityManager#checkRead(java.lang.String)
     * @since      JDK1.0
     */
    public FileInputStream(File file) throws FileNotFoundException {
	this(file.getPath());
    }

    /**
     * Creates an input file stream to read from the specified file descriptor.
     *
     * @param      fdObj   the file descriptor to be opened for reading.
     * @exception  SecurityException  if a security manager exists, its
     *               <code>checkRead</code> method is called with the file
     *               descriptor to see if the application is allowed to read
     *               from the specified file descriptor.
     * @see        java.lang.SecurityManager#checkRead(java.io.FileDescriptor)
     * @since      JDK1.0
     */
    public FileInputStream(FileDescriptor fdObj) {
	fdStream = new FDInputStream(fdObj);
    }

    /**
     * Opens the specified file for reading.
     * @param name the name of the file
     */
    private void open(String name) throws IOException {
	fdStream.open(name);
    }

    /**
     * Reads a byte of data from this input stream. This method blocks 
     * if no input is yet available. 
     *
     * @return     the next byte of data, or <code>-1</code> if the end of the
     *             file is reached.
     * @exception  IOException  if an I/O error occurs.
     * @since      JDK1.0
     */
    public int read() throws IOException {
	return fdStream.read();
    }

    /** 
     * Reads a subarray as a sequence of bytes. 
     * @param b the data to be written
     * @param off the start offset in the data
     * @param len the number of bytes that are written
     * @exception IOException If an I/O error has occurred. 
     */ 
    private int readBytes(byte b[], int off, int len) throws IOException {
	return fdStream.readBytes(b, off, len);
    }

    /**
     * Reads up to <code>b.length</code> bytes of data from this input 
     * stream into an array of bytes. This method blocks until some input 
     * is available. 
     *
     * @param      b   the buffer into which the data is read.
     * @return     the total number of bytes read into the buffer, or
     *             <code>-1</code> if there is no more data because the end of
     *             the file has been reached.
     * @exception  IOException  if an I/O error occurs.
     * @since      JDK1.0
     */
    public int read(byte b[]) throws IOException {
	return readBytes(b, 0, b.length);
    }

    /**
     * Reads up to <code>len</code> bytes of data from this input stream 
     * into an array of bytes. This method blocks until some input is 
     * available. 
     *
     * @param      b     the buffer into which the data is read.
     * @param      off   the start offset of the data.
     * @param      len   the maximum number of bytes read.
     * @return     the total number of bytes read into the buffer, or
     *             <code>-1</code> if there is no more data because the end of
     *             the file has been reached.
     * @exception  IOException  if an I/O error occurs.
     * @since      JDK1.0
     */
    public int read(byte b[], int off, int len) throws IOException {
	return readBytes(b, off, len);
    }

    /**
     * Skips over and discards <code>n</code> bytes of data from the 
     * input stream. The <code>skip</code> method may, for a variety of 
     * reasons, end up skipping over some smaller number of bytes, 
     * possibly <code>0</code>. The actual number of bytes skipped is returned.
     *
     * @param      n   the number of bytes to be skipped.
     * @return     the actual number of bytes skipped.
     * @exception  IOException  if an I/O error occurs.
     * @since      JDK1.0
     */
    public long skip(long n) throws IOException {
	return fdStream.skip(n);
    }

    /**
     * Returns the number of bytes that can be read from this file input
     * stream without blocking.
     *
     * @return     the number of bytes that can be read from this file input
     *             stream without blocking.
     * @exception  IOException  if an I/O error occurs.
     * @since      JDK1.0
     */
    public int available() throws IOException {
	return fdStream.available();
    }

    /**
     * Closes this file input stream and releases any system resources 
     * associated with the stream. 
     *
     * @exception  IOException  if an I/O error occurs.
     * @since      JDK1.0
     */
    public void close() throws IOException {
	fdStream.close();
    }

    /**
     * Returns the opaque file descriptor object associated with this stream.
     *
     * @return     the file descriptor object associated with this stream.
     * @exception  IOException  if an I/O error occurs.
     * @see        java.io.FileDescriptor
     * @since      JDK1.0
     */
    public final FileDescriptor getFD() throws IOException {
	return fdStream.getFD();
    }

    /**
     * Ensures that the <code>close</code> method of this file input stream is
     * called when there are no more references to it. 
     *
     * @exception  IOException  if an I/O error occurs.
     * @see        java.io.FileInputStream#close()
     * @since      JDK1.0
     */
    protected void finalize() throws IOException {
	FileDescriptor fd = getFD();
	if (fd != null) {
	    if (fd != fd.in) {
		close();
	    }
	}
    }
}
