<!doctype html public "-//w3c/dtd HTML 4.0//en">
<html>
<!-- Copyright (c) 1999-2000 by BEA Systems, Inc. All Rights Reserved.-->
<head>
<title>URL Rewriting</title>
</head>

<body bgcolor="#FFFFFF">
<font face="Helvetica">
<h2>
<font color=#DB1260>
HttpSessions and URL rewriting
</font>
</h2>

<p>
This servlet demonstrates how WebLogic deals with session-related
information when cookies are unavailable or disabled in your client's
browser. This is commonly referred to as "URL rewriting," because
instead of tracking the session ID in a cookie, WebLogic appends
the session ID at the end of the URL. <a href=#NOTE><b>NOTE</b></a>

<p>
To use this servlet to show how WebLogic rewrites URLs, you'll need
to set up both WebLogic and your browser.

<h4>
Check WebLogic properties
</h4>

<p>
Make sure that these properties are set as shown in your <font
face="Courier New" size=-1>weblogic.properties</font> file, and then
restart the WebLogic Server:

<table>
<tr>
<td>
<font face="Courier New"
size=-1>weblogic.httpd.session.enable=true</font>
</td>
<td><font face="Helvetica">(Current setting:  <font face="Courier New"
size=-1><b><%= getProp("weblogic.httpd.session.enable") %></b></font>)
</td>
</tr>

<tr>
<td>
<font face="Courier New"
size=-1>weblogic.httpd.session.URLRewriting.enable=true</font>
</td>
<td><font face="Helvetica">(Current setting:  <font face="Courier New"
size=-1><b><%= getProp("weblogic.httpd.session.URLRewriting.enable") %></b></font>)
</td>
</tr>

<tr>
<td>
<font face="Courier New"
size=-1>weblogic.httpd.session.cookies.enable=true</font>
</td>
<td><font face="Helvetica">(Current setting:  <font face="Courier New" 
size=-1><b><%= getProp("weblogic.httpd.session.cookies.enable") %></b></font>)
</td>
</tr>
</table>

<h4>
Set up your browser
</h4>

<p>
You must also <b>disable cookies</b> in your browser:
<dl>
<dt>
In Netscape
<dd>Select Preferences from the Edit menu.
<br>
On the Advanced tab panel, select the radio button beside "Disable Cookies".

<p>
<dt>In Internet Explorer
<dd>Select Internet Options from the View menu.
<br>
On the Advanced tab panel, scroll down to "Security".
<br>
Find the entry for "Cookies" and select the radio button
beside "Disable all cookie use".

</dl>

<h4>
Set some session info
</h4>

<p>
To see how it works, try setting some session name/value
pairs with cookies disabled. Session data will still be
stored on the server but the session ID will be passed to 
the server by rewritten as arguments appended to the URL.
<p>
<b>Note:</b> You'll also need to encode the FORM ACTION
URL.

<p>
<%@ page import="
weblogic.common.T3Services
" %>

<%!
  HttpSession session;
%>

<%
  session = request.getSession(true);
  if (session == null) {
    out.print("\nSession is null!<p>");
  }
  if(session != null){
	String url = "http://" + request.getRemoteAddr() + ":" 
	+ request.getServerPort() + request.getRequestURI(); 
    out.print("The normal non-encoded URL is:<br><u>" + url +"</u><p>");
    url = response.encodeURL(url);
    out.print("The encoded URL is :<br><a href=\"" + url +
              "\">" + url + "</a><p>");
  }

  if (request.getParameter("AddValue") != null) {
    session.putValue("SessionServlet." + request.getParameter("NameField"), 
  	              request.getParameter("ValueField"));
  } else if (request.getParameter("DeleteValue") != null) {
    session.removeValue("SessionServlet." + request.getParameter("NameField"));
  }

%>

<center>
<table border=1 cellspacing=2 cellpadding=5 width=400 bgcolor=#EEEEEE>
<th colspan=2><font face="Helvetica">Session : <%= session.getId() %></font><br>

</th>
<tr>
<th><font face="Helvetica"><B>Name</B></th>
<th><font face="Helvetica"><B>Value</B></th>
</tr>

<%
  String[] sessionNames = session.getValueNames();  
  if (sessionNames != null) {
    for (int index = 0; index < sessionNames.length; index++) {
%>

<tr>
<td><font face="Helvetica"><%= sessionNames[index] %></td>
<td><font face="Helvetica"><%= session.getValue(sessionNames[index]) %></td>
</tr>

<%
    }
  }
%>

<%!
  public String getProp(String toGet){
    try {
      return T3Services.getT3Services().config().getProperty(toGet);
    }
    catch(Exception e){
      // Here, we access the javax.servlet.ServletContext object. Note that the 
      // 'application' implicit object is not available here because we are not
      // in a scriplet or an expression. 
      getServletConfig().
          getServletContext().log("T3Exception thrown getting property.",e);
    }
    return "";
}
 
%>

</table>
</center>
<p>

<form method="post" name="URLEncode" action="<%= response.encodeURL(request.getRequestURI()) %>">

<center>
<table border=1 cellspacing=2 cellpadding=5 width=400>
<th><font face="Helvetica">Name to add/delete</th>
<th><font face="Helvetica">Value</th>
<tr>
<td align=right><font face="Helvetica"><input type="text" name="NameField"></td>
<td align=left><font face="Helvetica"><input type="text" name="ValueField"></td>
</tr>

<tr>
<td colspan=2 align=center><input 
  type="submit" value=" Add " name="AddValue"><input type="submit" value="Delete" name="DeleteValue"></td>
</tr>
</table>
</center>
</form>

<a name="NOTE"></a>
For the very first request to a servlet
that involves a brand new HttpSession, the call
to <font face="Courier New" size=-1>response.encodeURL(url)</font> will always return
an encoded URL.  This is because on the first
request there is no Session ID to be
found in either the Cookie or the URL.  There is no 
way to know if the browser has cookies turned 
off. The course of action is:

<ol>
  <li>Set a cookie.
  <li>Encode URL with SessionCookie.
</ol>
<p>
If a cookie doesn't come back because cookies are 
disabled, then the URL will contine to be
encoded with Session ID.

<p>
<font size=-1>Copyright © 1999-2000 by BEA Systems, Inc. All Rights Reserved.
</font>

</body>
</html>
