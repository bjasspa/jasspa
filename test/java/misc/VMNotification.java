/*
 * @(#)VMNotification.java	1.4 98/10/29
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

public interface VMNotification {

    /**
     * This interface is used by parties who are interested in being notified
     * when the VM changes the memory state.
     *
     * @param oldState The old memory state, one of VM.STATE_GREEN,
     *			VM.STATE_YELLOW or VM.STATE_RED
     * @param newState The new memory state
     *
     * @see sun.misc.VM.registerVMNotification()
     */
    void newAllocState(int oldState, int newState, boolean notUsed);
}

