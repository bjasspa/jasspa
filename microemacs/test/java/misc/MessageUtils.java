/*
 * @(#)MessageUtils.java	1.8 98/07/01
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
 * MessageUtils: miscellaneous utilities for handling error and status
 * properties and messages.
 *
 * @version 1.8, 07/01/98
 * @author Herb Jellinek
 */

public class MessageUtils {
    // can instantiate it for to allow less verbose use - via instance
    // instead of classname
    
    public MessageUtils() { }

    public static String subst(String patt, String arg) {
	String args[] = { arg };
	return subst(patt, args);
    }

    public static String subst(String patt, String arg1, String arg2) {
	String args[] = { arg1, arg2 };
	return subst(patt, args);
    }

    public static String subst(String patt, String arg1, String arg2,
			       String arg3) {
	String args[] = { arg1, arg2, arg3 };
	return subst(patt, args);
    }

    public static String subst(String patt, String args[]) {
	StringBuffer result = new StringBuffer();
	int len = patt.length();
	for (int i = 0; i >= 0 && i < len; i++) {
	    char ch = patt.charAt(i);
	    if (ch == '%') {
		if (i != len) {
		    int index = Character.digit(patt.charAt(i + 1), 10);
		    if (index == -1) {
			result.append(patt.charAt(i + 1));
			i++;
		    } else if (index < args.length) {
			result.append(args[index]);
			i++;
		    }
		}
	    } else {
		result.append(ch);
	    }
	}
	return result.toString();
    }

    public static String substProp(String propName, String arg) {
	return subst(System.getProperty(propName), arg);
    }

    public static String substProp(String propName, String arg1, String arg2) {
	return subst(System.getProperty(propName), arg1, arg2);
    }

    public static String substProp(String propName, String arg1, String arg2,
				   String arg3) {
	return subst(System.getProperty(propName), arg1, arg2, arg3);
    }

    public static void main(String args[]) {
	String stringArgs[] = new String[args.length - 1];
	System.arraycopy(args, 1, stringArgs, 0, args.length - 1);
	System.out.println("> result = "+subst(args[0], stringArgs));
    }
}
