/**
 * XletStateChangeException.java
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
 * An Xlet state change exception.
 *<p>
 *@see javax.tv.xlet.Xlet
 *@see javax.tv.xlet.XletContext
 */
public class XletStateChangeException extends java.lang.Exception
{
    private String Text = null;
    private Object msgobj = null;

    public XletStateChangeException(String Text) 
    {
        super(Text);
        this.Text = Text;
        
    }
    public String getLocalizedMessage() 
    {
             return Text;
    }
}
