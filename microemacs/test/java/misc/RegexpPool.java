/*
 * @(#)RegexpPool.java	1.7 98/07/01
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
import java.io.*;

/**
 * A class to represent a pool of regular expressions.  A string
 * can be matched against the whole pool all at once.  It is much
 * faster than doing individual regular expression matches one-by-one.
 *
 * @see java.misc.RegexpTarget
 * @author  James Gosling
 */

public class RegexpPool {
    private RegexpNode prefixMachine = new RegexpNode();
    private RegexpNode suffixMachine = new RegexpNode();
    private static final int BIG = 0x7FFFFFFF;
    private int lastDepth = BIG;

    public RegexpPool () {
    }

    /** 
     * Add a regular expression to the pool of regular expressions.
     * @param	re  The regular expression to add to the pool.
	    For now, only handles strings that either begin or end with
	    a '*'.
     * @param	ret The object to be returned when this regular expression is
	    matched.  If ret is an instance of the RegexpTarget class, ret.found
	    is called with the string fragment that matched the '*' as its 
            parameter.
     * @exception REException error
     */
    public void add(String re, Object ret) throws REException {
	add(re, ret, false);
    }

    /**
     * Replace the target for the regular expression with a different
     * target.
     *
     * @param	re  The regular expression to be replaced in the pool.
     *	    For now, only handles strings that either begin or end with
     *	    a '*'.
     * @param	ret The object to be returned when this regular expression is
     *	    matched.  If ret is an instance of the RegexpTarget class, ret.found
     *	    is called with the string fragment that matched the '*' as its 
     *      parameter.
     */
    public void replace(String re, Object ret) {
	try {
	    add(re, ret, true);
	} catch(Exception e) {
	    // should never occur if replace is true
	}
    }

    /**
     * Delete the regular expression and its target. 
     * @param re The regular expression to be deleted from the pool.
     *           must begin or end with a '*'
     * @return target - the old target.
     */ 
    public Object delete(String re) {
	Object o = null;
	RegexpNode p = prefixMachine;
	RegexpNode best = p;
	int len = re.length() - 1;
	int i;
	boolean prefix = true;
	
	if (!re.startsWith("*") ||
	    !re.endsWith("*"))
	    len++;

	if (len <= 0)
	    return null;

	/* March forward through the prefix machine */
	for (i = 0; p != null; i++) {
	    if (p.result != null && p.depth < BIG
		&& (!p.exact || i == len)) {
		best = p;
	    }
	    if (i >= len)
		break;
	    p = p.find(re.charAt(i));
	}

	/* march backward through the suffix machine */
	p = suffixMachine;
	for (i = len; --i >= 0 && p != null;) {
	    if (p.result != null && p.depth < BIG) {
		prefix = false;
		best = p;
	    }
	    p = p.find(re.charAt(i));
	}
	
	// delete only if there is an exact match
	if (prefix) {
	    if (re.equals(best.re)) {
		p("Deleting " + best.re);
		o = best.result;
		best.result = null;

	    }
	} else {
	    if (re.equals(best.re)) {
		o = best.result;
		best.result = null;
	    }
	}
	return o;
    }

    /** Search for a match to a string & return the object associated
	with it with the match.  When multiple regular expressions
	would match the string, the best match is returned first. 
	The next best match is returned the next time matchNext is 
        called.
	@param s    The string to match against the regular expressions
		    in the pool.
	@return	    null on failure, otherwise the object associated with
		    the regular expression when it was added to the pool.
		    If the object is an instance of RegexpTarget, then
		    the return value is the result from calling
		    return.found(string_that_matched_wildcard).
    */
    public Object match(String s) {
	return matchAfter(s, BIG);
    }

    /** Identical to match except that it will only find matches to
	regular expressions that were added to the pool <i>after</i>
	the last regular expression that matched in the last call
	to match() or matchNext() */
    public Object matchNext(String s) {
	return matchAfter(s, lastDepth);
    }

    private void add(String re, Object ret, boolean replace) throws REException {
	int len = re.length();
	RegexpNode p;
	if (re.charAt(0) == '*') {
	    p = suffixMachine;
	    while (len > 1)
		p = p.add(re.charAt(--len));
	} else {
	    boolean exact = false;
	    if (re.charAt(len - 1) == '*')
		len--;
	    else
		exact = true;
	    p = prefixMachine;
	    for (int i = 0; i < len; i++)
		p = p.add(re.charAt(i));
	    p.exact = exact;
	}

	if (p.result != null && !replace)
	    throw new REException(re + " is a duplicate");

	p.re = re;
	p.result = ret;
    }

    private Object matchAfter(String s, int lastMatchDepth) {
	RegexpNode p = prefixMachine;
	RegexpNode best = p;
	int bst = 0;
	int bend = 0;
	int len = s.length();
	int i;
	if (len <= 0)
	    return null;
	/* March forward through the prefix machine */
	for (i = 0; p != null; i++) {
	    if (p.result != null && p.depth < lastMatchDepth
		&& (!p.exact || i == len)) {
		lastDepth = p.depth;
		best = p;
		bst = i;
		bend = len;
	    }
	    if (i >= len)
		break;
	    p = p.find(s.charAt(i));
	}
	/* march backward through the suffix machine */
	p = suffixMachine;
	for (i = len; --i >= 0 && p != null;) {
	    if (p.result != null && p.depth < lastMatchDepth) {
		lastDepth = p.depth;
		best = p;
		bst = 0;
		bend = i+1;
	    }
	    p = p.find(s.charAt(i));
	}
	Object o = best.result;
	if (o != null && o instanceof RegexpTarget)
	    o = ((RegexpTarget) o).found(s.substring(bst, bend));
	return o;
    }

    /** Resets the pool so that the next call to matchNext looks
	at all regular expressions in the pool.  match(s); is equivalent
	to reset(); matchNext(s);
	<p><b>Multithreading note:</b> reset/nextMatch leave state in the
	regular expression pool.  If multiple threads could be using this
	pool this way, they should be syncronized to avoid race hazards.
	match() was done in such a way that there are no such race
	hazards: multiple threads can be matching in the same pool
	simultaneously. */
    public void reset() {
	lastDepth = BIG;
    }

    static public void main(String argv[]) {
	if (argv.length < 5) {
	    p("need 5 params");
	    System.exit(1);
	}
	RegexpPool rp = new RegexpPool ();
	try {
	    for (int i = 1; i < argv.length; i++)
		rp.add(argv[i], argv[i]);

	    // delete the 3rd addition
	    p("Trying to delete " + argv[3]);
	    rp.delete(argv[3]);
	} catch (Exception e) {
	}	
	p((String)rp.match(argv[0]));
    }

    /** Print this pool to standard output */
    public void print(PrintStream out) {
	out.print("Regexp pool:\n");
	if (suffixMachine.firstchild != null) {
	    out.print(" Suffix machine: ");
	    suffixMachine.firstchild.print(out);
	    out.print("\n");
	}
	if (prefixMachine.firstchild != null) {
	    out.print(" Prefix machine: ");
	    prefixMachine.firstchild.print(out);
	    out.print("\n");
	}
    }
    
    private static void p(String s) {
//	System.out.println(s);
    }
}

/* A node in a regular expression finite state machine. */
class RegexpNode {
    char c;
    RegexpNode firstchild;
    RegexpNode nextsibling;
    int depth;
    boolean exact;
    Object result;
    String re = null;
    
    RegexpNode () {
	c = '#';
	depth = 0;
    }
    RegexpNode (char C, int depth) {
	c = C;
	this.depth = depth;
    }
    RegexpNode add(char C) {
	RegexpNode p = firstchild;
	if (p == null)
	    p = new RegexpNode (C, depth+1);
	else {
	    while (p != null)
		if (p.c == C)
		    return p;
		else
		    p = p.nextsibling;
	    p = new RegexpNode (C, depth+1);
	    p.nextsibling = firstchild;
	}
	firstchild = p;
	return p;
    }
    RegexpNode find(char C) {
	for (RegexpNode p = firstchild;
		p != null;
		p = p.nextsibling)
	    if (p.c == C)
		return p;
	return null;
    }
    void print(PrintStream out) {
	if (nextsibling != null) {
	    RegexpNode p = this;
	    out.print("(");
	    while (p != null) {
		out.write(p.c);
		if (p.firstchild != null)
		    p.firstchild.print(out);
		p = p.nextsibling;
		out.write(p != null ? '|' : ')');
	    }
	} else {
	    out.write(c);
	    if (firstchild != null)
		firstchild.print(out);
	}
    }
}
