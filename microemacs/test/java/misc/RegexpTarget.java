/*
 * @(#)RegexpTarget.java	1.6 98/07/01
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
 * A class to define actions to be performed when a regular expression match
 *  occurs.
 * @author  James Gosling
 */

public interface RegexpTarget {
    /** Gets called when a pattern in a RegexpPool matches.
      * This method is called by RegexpPool.match() who passes the return
      * value from found() back to its caller.
      * @param remainder the string that matched the * in the pattern.
      */
    Object found(String remainder);
}
