/*
 * @(#)Regexp.java	1.7 98/07/01
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
 * A class to represent a regular expression.  Only handles '*'s.
 * @author  James Gosling
 */

public class Regexp {
    /** if true then the matching process ignores case. */
    public boolean ignoreCase;

    /*
     * regular expressions are carved into three regions: a constant string
     * prefix, a constant string suffix, and a series of floating strings in
     * between.  In the input regular expression, they are seperated by *s
     */
    public String exp;
    public String prefix, suffix;
    public boolean exact;
    public int prefixLen, suffixLen, totalLen;
    public String mids[];

    /** Create a new regular expression object.  The regular expression
        is a series of constant strings seperated by *s.  For example:
        <dl>
        <dt>*.gif       <dd>Matches any string that ends in ".gif".
        <dt>/tmp/*      <dd>Matches any string that starts with "/tmp/".
        <dt>/tmp/*.gif  <dd>Matches any string that starts with "/tmp/" and ends
                        with ".gif".
        <dt>/tmp/*new*.gif <dd>Matches any string that starts with "/tmp/"
                        and ends with ".gif" and has "new" somewhere in between.
        </dl>
     */
    public Regexp (String s) {
        exp = s;
	int firstst = s.indexOf('*');
	int lastst = s.lastIndexOf('*');
	if (firstst < 0) {
	    totalLen = s.length();
	    exact = true;	// no * s
	} else {
	    prefixLen = firstst;
	    if (firstst == 0)
		prefix = null;
	    else
		prefix = s.substring(0, firstst);
	    suffixLen = s.length() - lastst - 1;
	    if (suffixLen == 0)
		suffix = null;
	    else
		suffix = s.substring(lastst + 1);
	    int nmids = 0;
	    int pos = firstst;
	    while (pos < lastst && pos >= 0) {
		nmids++;
		pos = s.indexOf('*', pos + 1);
	    }
	    totalLen = prefixLen + suffixLen;
	    if (nmids > 0) {
		mids = new String[nmids];
		pos = firstst;
		for (int i = 0; i < nmids; i++) {
		    pos++;
		    int npos = s.indexOf('*', pos);
		    if (pos < npos) {
			mids[i] = s.substring(pos, npos);
			totalLen += mids[i].length();
		    }
		    pos = npos;
		}
	    }
	}
    }

    /** Returns true iff the String s matches this regular expression. */
    final boolean matches(String s) {
	return matches(s, 0, s.length());
    }

    /** Returns true iff the substring of s from offset for len characters
        matches this regular expression. */
    boolean matches(String s, int offset, int len) {
	if (exact)
	    return len == totalLen &&
		exp.regionMatches(ignoreCase, 0, s, offset, len);
	if (len < totalLen)
	    return false;
	if (prefixLen > 0 &&
		!prefix.regionMatches(ignoreCase,
				      0, s, offset, prefixLen)
		||
		suffixLen > 0 &&
		!suffix.regionMatches(ignoreCase,
				      0, s, offset + len - suffixLen,
				      suffixLen))
	    return false;
        if (mids == null)
            return true;
        int nmids = mids.length;
	int spos = offset + prefixLen;
        int limit = offset+len-suffixLen;
        for (int i = 0; i<nmids; i++) {
            String ms = mids[i];
            int ml = ms.length();
            while (spos+ml<=limit &&
                   !ms.regionMatches(ignoreCase,
                                     0, s, spos, ml))
                spos++;
            if (spos+ml>limit)
                return false;
            spos+=ml;
        }
        return true;
    }

    static public void main(String argv[]) {
        Regexp r = new Regexp(argv[0]);
        System.out.print("re="+r+"\n");
        r.ignoreCase = true;
        for (int i = 1; i<argv.length; i++)
            System.out.print("<"+argv[i]+"> "+r.matches(argv[i])+"\n");
    }
}
