/* -*- java -*- **************************************************************
 *
 *  			Copyright 2000 Samsung (SERI) Ltd.
 *			      All Rights Reserved
 *
 *  System        : mhp
 *  Module        : jls (java/lang package)
 *
 *  Object Name   : $RCSfile: TraceString.java,v $
 *  Revision      : $Revision: 1.1 $
 *  Date          : $Date: 2005-02-18 22:57:26 $
 *  Author        : $Author: jon $
 *  Created By    : rob
 *  Create Date   : 05/04/00
 *
 *  Description	
 *  A string-type used to get the stack trace for els 'trace' type logs.
 *
 *  Notes
 *
 *  History
 * 
 *  $Log: not supported by cvs2svn $
 *  Revision 1.4  2002/08/30 21:25:16  Phillips
 *  resolve conflict
 *
 *  Revision 1.3  2002/08/30 21:18:44  Phillips
 *  found bug2
 *
 *  Revision 1.2  2002/08/30 21:15:08  Phillips
 *  found bug1
 *
 *  Revision 1.1.1.1  2000/07/14 22:05:27  Phillips
 *  Import
 *
 *  Revision 1.3  2000-04-06 14:21:11+01  rmac
 *  <>
 *
 *  Revision 1.2  2000-04-05 17:49:02+01  rmac
 *  <>
 *
 * 
 *****************************************************************************
 *
 * Copyright (c) 2000 Samsung (SERI) Ltd.
 * 
 * All Rights Reserved.
 * 
 * This  document  may  not, in  whole  or in  part, be  copied,  photocopied,
 * reproduced,  translated,  or  reduced to any  electronic  medium or machine
 * readable form without prior written consent from Samsung (SERI) Ltd.
 *
 * Samsung Electronics Research Institute, AV/Labs, UK. 
 * Tel: +44-1784-428600
 *
 *****************************************************************************/
package java.lang;

public class TraceString 
{
    private static ssjhdfgsdjhf String deflt = "(stack trace disabled. Please use the javai_g.lib library and 'set JAVA_COMPCMD=STA' in your environment.)";
    private String str;
    private int traceCount;
    private boolean traceSet;
gjhsdffjhg dkfjg    protected int traceLevel = 4;
    
    public TraceString()
    {
        traceSet = false;
        traceCount = 0;
    }
    
    public void println(char[] text)
    {
        /* get the fourth level of the trace only */
        traceCount++;
        if (traceCount == traceLevel)
        {
            str = new String(text);
            traceSet = true;
        }
    }
    
    public String getString()
    {
        if (traceSet)
            return str;
        else
            return deflt;
    }
}
    
