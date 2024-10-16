/*
 * @(#)PermissionCollection.java	1.23 99/10/08
 *
 * Copyright 1997, 1998 by Sun Microsystems, Inc.,
 * 901 San Antonio Road, Palo Alto, California, 94303, U.S.A.
 * All rights reserved.
 *
 * This software is the confidential and proprietary information
 * of Sun Microsystems, Inc. ("Confidential Information").  You
 * shall not disclose such Confidential Information and shall use
 * it only in accordance with the terms of the license agreement
 * you entered into with Sun.
 */

package java.security;

import java.util.*;

/**
 * Abstract class representing a collection of Permission objects.
 *
 * <p>With a PermissionCollection, you can:
 * <UL>
 * <LI> add a permission to the collection using the <code>add</code> method.
 * <LI> check to see if a particular permission is implied in the
 *      collection, using the <code>implies</code> method.
 * <LI> enumerate all the permissions, using the <code>elements</code> method.
 * </UL>
 * <P>
 *
 * <p>When it is desirable to group together a number of Permission objects of the
 * same type, the <code>newPermissionCollection</code> method on that particular
 * type of Permission object should first be called. The default behavior (from the
 * Permission class) is to simply return null. Subclasses of class Permission
 * override the method if they need to store their permissions in a particular
 * PermissionCollection object in order to provide the correct semantics
 * when the <code>PermissionCollection.implies</code> method is called.
 * If a non-null value is returned, that PermissionCollection must be used.
 * If null is returned, then the caller of <code>newPermissionCollection</code>
 * is free to store permissions of the
 * given type in any PermissionCollection they choose (one that uses a Hashtable,
 * one that uses a Vector, etc).
 *
 * <p>The PermissionCollection returned by the
 * <code>Permission.newPermissionCollection</code>
 * method is a homogeneous collection, which stores only Permission objects
 * for a given Permission type.  A PermissionCollection may also be heterogenous.
 * For example, Permissions is a PermissionCollection subclass that represents a
 * collection of PermissionCollections. That is, its members are each a homogeneous
 * PermissionCollection. For example, a Permissions object might have a
 * FilePermissionCollection
 * for all the FilePermission objects, a SocketPermissionCollection for all the
 * SocketPermission objects, and so on. Its <code>add</code> method adds a permission
 * to the appropriate collection.
 *
 * <p>Whenever a permission is added to a heterogeneous PermissionCollection such
 * as Permissions, and the PermissionCollection doesn't yet contain a
 * PermissionCollection of the specified permission's type, the
 * PermissionCollection should call
 * the <code>newPermissionCollection</code> method on the permission's class
 * to see if it requires a special PermissionCollection. If
 * <code>newPermissionCollection</code>
 * returns null, the PermissionCollection
 * is free to store the permission in any type of PermissionCollection it desires
 * (one using a Hastable, one using a Vector, etc.). For example,
 * the Permissions object uses a default PermissionCollection implementation
 * that stores the permission objects in a Hashtable.
 *
 * @see Permission
 * @see Permissions
 *
 * @version 1.23 99/10/08
 *
 * @author Roland Schemers
 */

public abstract class PermissionCollection implements java.io.Serializable {

    // when set, add will throw an exception.
    private boolean readOnly;

    /**
     * Adds a permission object to the current collection of permission objects.
     *
     * @param permission the Permission object to add.
     */

    public abstract void add(Permission permission);

    /**
     * Checks to see if the specified permission is implied by
     * the collection of Permission objects held in this PermissionCollection.
     *
     * @param permission the Permission object to compare.
     *
     * @return true if "permission" is implied by the  permissions in
     * the collection, false if not.
     */
    public abstract boolean implies(Permission permission);

    /**
     * Returns an enumeration of all the Permission objects in the collection.
     *
     * @return an enumeration of all the Permissions.
     */
    public abstract Enumeration elements();

    /**
     * Marks this PermissionCollection object as "readonly". After
     * a PermissionCollection object
     * is marked as readonly, no new Permission objects can be added to it
     * using <code>addPermission</code>.
     */
    public void setReadOnly() {
	readOnly = true;
    }

    /**
     * Returns true if this PermissionCollection object is marked as readonly. If it
     * is readonly, no new Permission objects can be added to it
     * using <code>addPermission</code>.
     *
     * <p>By default, the object is <i>not</i> readonly. It can be set to readonly
     * by a call to <code>setReadOnly</code>.
     *
     * @return true if this PermissionCollection object is marked as readonly, false
     * otherwise.
     */
    public boolean isReadOnly() {
	return readOnly;
    }

    /**
     * Returns a string describing this PermissionCollection object,
     * providing information about all the permissions it contains.
     * The format is:
     * <pre>
     * super.toString() (
     *   // enumerate all the Permission
     *   // objects and call toString() on them,
     *   // one per line..
     * )</pre>
     *
     * <code>super.toString</code> is a call to the <code>toString</code>
     * method of this
     * object's superclass, which is Object. The result is
     * this PermissionCollection's type name followed by this object's
     * hashcode, thus enabling clients to differentiate different
     * PermissionCollections object, even if they contain the same permissions.
     *
     * @return information about this PermissionCollection object,
     *         as described above.
     *
     */
    public String toString() {
	Enumeration enum = elements();
	StringBuffer sb = new StringBuffer();
	sb.append(super.toString()+" (\n");
	while (enum.hasMoreElements()) {
	    try {
		sb.append(" ");
		sb.append(enum.nextElement().toString());
		sb.append("\n");
	    } catch (NoSuchElementException e){
		// ignore
	    }
	}
	sb.append(")\n");
	return sb.toString();
    }
}
