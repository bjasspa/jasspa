/*
 * @(#)VM.java	1.11 98/10/29
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

import java.util.Vector;

/**
 * Interface to the low level vm allocator state (green-yellow-red).
 * When state = green, we are running as normal.  When state = yellow,
 * we attempt to invoke higher-level cleanups.  When state = red,
 * the higher-level routines should try more last-ditch cleanups.
 */
public class VM implements Runnable {

    public static final int STATE_GREEN = 1;
    public static final int STATE_YELLOW = 2;
    public static final int STATE_RED = 3;

    private byte memoryAdvice = STATE_GREEN;	// Updated by GC
    private Object memoryAdviceLock = new Object();

    private Vector callbacks = new Vector(1);
    private Object[] callbacksCopy = new Object[1];
    private Thread callbackThread;

    private static VM singleton = null;


    /**
     * Compute the current memory state.  This may be an expensive operation,
     * because a GC might be triggered to produce an accurate result.
     * @return the current memory state
     *
     */
    public static final native int getState();

    /**
     * Report what the memory state was at the end of the last GC.  This will
     * be a quick operation that should give a useful approximation of the
     * real state of the system.
     *
     * @return the current memory state
     */
    public static final int getStateQuick() {
	return getSingleton().memoryAdvice;
    }

    public static void registerVMNotification(VMNotification n) {
	getSingleton().registerCallback(n);
    }
    

    //
    // Don't let anyone else instantiate this class
    //
    private VM() { 
	registerWithGC();
    }

    private static VM getSingleton() {
	if (singleton == null) {
	    createSingleton();
	}
	return singleton;
    }

    private static synchronized void createSingleton() {
	if (singleton == null) {  // We must re-test this within synchronized
	    singleton = new VM();
	}
    }

    private native void registerWithGC();

    private void registerCallback(VMNotification n) {
	synchronized(this) {
	    if (callbackThread == null) {
		callbackThread = new Thread(singleton, "RYG Callback Thread");
		callbackThread.setPriority(Thread.MAX_PRIORITY - 1);
		callbackThread.start();
	    }
            // we update callbacksCopy first in case the memory allocation
            // triggers a state change.  The gc will attempt to acquire
            // the lock that we already own, which is ok since it's
            // only going to update fields we're not playing with.
	    synchronized(callbacks) {
		if (callbacks.size() + 1 > callbacksCopy.length) {
		    callbacksCopy = new Object[callbacks.size() + 1];
		}
		callbacks.addElement(n);
	    }
	}
    }

    /**
     * User code is not meant to call this.
     */
    // It's public because that's a requirement of runnable.  This isn't 
    // a security hole, because untrusted code can't call into sun.*.
    public void run() {

	int previousState = STATE_GREEN;
	int currentState;

	while (true) {
	    try {
		while (true) {
		    synchronized(memoryAdviceLock) {
			currentState = memoryAdvice;
			if (currentState != previousState) {
			    break;
			}
			memoryAdviceLock.wait();
		    }
		}
		// We go to some effort to avoid doing callbacks while holding
		// locks.  Doing otherwise is just asking for trouble.
		int copySize;
		Object[] cbc;
		synchronized(callbacks) {
		    // guaranteed to fit by registerCallback
		    callbacks.copyInto(callbacksCopy);
		    copySize = callbacks.size();

		    // callbacksCopy (the variable, not the object)
		    // may get stored into from registerCallback, so
		    // make a copy of the reference.
		    cbc = callbacksCopy;
		}

		// do the callbacks outside of the synchronized block to
		// avoid locking problems
		for (int i = 0; i < copySize; i++) {
		    VMNotification n = (VMNotification)cbc[i];
		    try {
			n.newAllocState(previousState, currentState, true);
		    } catch (Throwable t) {
			try {
			    t.printStackTrace();
			} catch (Throwable ignored) {
			}
		    }
		}

		previousState = currentState;
	    } catch (Throwable t) {
		try {
		    // don't let the RYG Callback thread exit for any reason.
		    t.printStackTrace();
		} catch (Throwable e) {
		}
	    }
	}
    }
}
