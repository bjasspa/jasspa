/**
 * XletContext.java
 *
 * Copyright 1999-2000 by Samsung Electronics Research Institute,
 * The Communications Centre, South St, Staines, Middlesex, TW18 4QE, U.K.
 *
 * All rights reserved.
 * 
 * This software is the confidential and proprietary information
 * of Samsung Electronics, Co. ("Confidential Information").
 */
  
package javax.tv.xlet;

/**
 * The XletContext interface is used by the Xlet to signal
 * state changes and to obtain properties associated with
 * this instance of the Xlet.
 *<p>
 *@see javax.tv.xlet.Xlet
 *@see javax.tv.xlet.XletStateChangeException
 */
public abstract interface XletContext {
    /**
     * Signals that the Xlet has entered itself into the 
     * Destroyed state. The application manager should update 
     * the state to Destroyed without calling the Xlet's destroy
     * method. The Xlet must perform the same operations (clean up,
     * releasing of resources etc.) it would have if the destroy()
     * was called.
     */
    public void notifydestroyed();
    
    /**
     * Provides an Xlet with a mechanism to retrieve named properties 
     * from the XletContext.
     *<p>
     *@return   A reference to an object representing the property. 
     *          <code>null</code> is returned if no value is 
     *          available for key.
     */
    public java.lang.Object getXletProperty(java.lang.String key);
    
    /**
     * Signals that the Xlet does not want to be active and has 
     * entered the Paused state. This method can only be invoked 
     * when the Xlet is in the Active state.
     *<p>
     * If an Xlet calls paused, in the future it may be asked to 
     * enter Destroyed state or the Active state again.
     */
    public void notifypaused();
    
    /**
     * Provides the Xlet with a mechanism to indicate that it is 
     * interested in entering the Active state. Calls to this method 
     * can be used by an application manager to determine which 
     * Xlets to move to Active state.
     */
    public void resumeRequest();
}