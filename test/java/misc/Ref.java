/*
 * @(#)Ref.java	1.22 98/07/01
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

package sun.misc;

/**
 * A "Ref" is an indirect reference to an object that the garbage collector
 * knows about.  An application should override the reconstitute() method with one
 * that will construct the object based on information in the Ref, often by
 * reading from a file.  The get() method retains a cache of the result of the last call to
 * reconstitute() in the Ref.  When space gets tight, the garbage collector
 * will clear old Ref cache entries when there are no other pointers to the
 * object.  In normal usage, Ref will always be subclassed.  The subclass will add the 
 * instance variables necessary for the reconstitute() method to work.  It will also add a 
 * constructor to set them up, and write a version of reconstitute().
 * @version     1.22, 07/01/98
 */

public abstract class Ref {
    static  long    lruclock;
    private Object  thing;
    private long    priority;

    /**
     * Returns a pointer to the object referenced by this Ref.  If the object
     * has been thrown away by the garbage collector, it will be
     * reconstituted. This method does everything necessary to ensure that the garbage
     * collector throws things away in Least Recently Used(LRU) order.  Applications should 
     * never override this method. The get() method effectively caches calls to
     * reconstitute().
     */
    public Object get() {
	Object p = thing;
	if (p == null) {
	    /* synchronize if thing is null, but then check again
	       in case somebody else beat us to it */
	    synchronized (this) {
		if ((p = thing) == null) {
		    // thing has not been reconstituted.
		    p = reconstitute();
		    thing = p;
		} 
	    }
	}
	priority = ++lruclock;
	return p;	/* Return local copy to avoid potential race */
    }

    /**
     * Returns a pointer to the object referenced by this Ref by 
     * reconstituting it from some external source (such as a file).  This method should not 
     * bother with caching since the method get() will deal with that.
     * <p>
     * In normal usage, Ref will always be subclassed.  The subclass will add
     * the instance variables necessary for reconstitute() to work.  It will
     * also add a constructor to set them up, and write a version of
     * reconstitute().
     */
    public abstract Object reconstitute();

    /**
     * Flushes the cached object.  Forces the next invocation of get() to
     * invoke reconstitute().
     */
    public void flush() {
	thing = null;
    }
   
    /**
     * Sets the thing to the specified object.
     * @param thing the specified object
     */
    public void setThing(Object thing) {
	this.thing = thing;
    }

    /**
     * Checks to see what object is being pointed at by this Ref and returns it.
     */
    public Object check() {
	return thing;
    }

    /**
     * Constructs a new Ref.
     */
    public Ref() {
	priority = ++lruclock;
    }
}
