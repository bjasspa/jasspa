/**
 * Xlet.java
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
 * The Xlet interface is used by the Xlet Intreface Thread
 * to control the state of the Xlet.
 *<p>
 *@see javax.tv.xlet.XletContext
 *@see javax.tv.xlet.XletStateChangeException
 */
public abstract interface Xlet {
    
    /**
     * Signals the Xlet to terminate and enter the Destroyed 
     * state. In the destroyed state the Xlet must release all 
     * resources and save any persistent state. This method 
     * may be called from the Loaded, Paused or Active states.
     *<p>
     * Xlets should perform any operations required before being 
     * terminated, such as releasing resources or saving 
     * preferences or state.
     *<p>
     * NOTE: The Xlet can request that it not enter the Destroyed 
     * state by throwing an XletStateChangeException. This is 
     * only a valid response if the unconditional flag is set 
     * to false. If it is true the Xlet is assumed to be in the 
     * Destroyed state regardless of how this method terminates. 
     * If it is not an unconditional request, the Xlet can signify 
     * that it wishes to stay in its current state by throwing 
     * the Exception. This request may be honored and the destroy() 
     * method called again at a later time.
     *<p>
     *@param    unconditional If done is true when this method 
     *          is called, requests by the Xlet to not enter 
     *          the destroyed state will be ignored.
     *
     *@throws    XletStateChangeException - is thrown if the Xlet 
     *          wishes to continue to execute (Not enter the 
     *          Destroyed state). This exception is ignored if 
     *          unconditional is equal to true.
     */
    public void destroyXlet(boolean unconditional)
	    throws XletStateChangeException;
	
	/**
	 *
     * Signals the Xlet to initialize itself and enter the Paused 
     * state. The Xlet shall initialize itself in preparation for 
     * providing service. It should not hold shared resources but 
     * should be prepared to provide service in a reasonable 
     * amount of time.
     *<p>
     * An XletContext is used by the Xlet to access properties 
     * associated with it's runtime environment. After this method 
     * returns successfully, the Xlet is in the Paused state and 
     * should be quiescent.
     *<p>
     * Note: This method shall only be called once.
     *<p>
     *@throws javax.tv.XletStateChangeException
     *
     *@see XletContext	
     */
    public void initXlet(XletContext ctx) 
        throws XletStateChangeException;

    /**
     * Signals the Xlet to stop providing service and enter the 
     * Paused state. In the Paused state the Xlet must stop 
     * providing service, and might release all shared resources 
     * and become quiescent. This method will only be called 
     * when the Xlet is in the Active state.
     */
    public void pauseXlet();
	
	/**
	 * Signals the Xlet to start providing service and enter the 
	 * Active state. In the Active state the Xlet may hold shared 
	 * resources. The method will only be called when the Xlet 
	 * is in the paused state.
	 *<p>
	 * Two kinds of failures can prevent the service from starting, 
	 * transient and non-transient. For transient failures the
	 * XletStateChangeException exception should be thrown. For 
	 * non-transient failures the XletContext.done method should
	 * be called with an error indication (TBD).
	 *<p>
	 *@throws    XletStateChangeException - is thrown if the Xlet 
	 *          cannot start providing service.
     */
     public void startXlet()
	    throws XletStateChangeException;

}