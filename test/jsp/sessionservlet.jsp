<!doctype html public "-//w3c/dtd HTML 4.0//en">
<html>
<!-- Copyright (c) 1999-2000 by BEA Systems, Inc. All Rights Reserved.-->
<head>
<title>Session Servlet</title>
</head>

<body bgcolor="#FFFFFF">
<font face="Helvetica">

<h2>
<font color=#DB1260>
Session Servlet
</font>
</h2>

<p>
This servlet demonstrates the use of HTTP sessions. You can add or delete
named values to a session. The same session can be retrieved next time the
user visits the page via a browser cookie. Use this form to see your additions
and deletions at work. Check the code to see how adding and deleting session
name/value pairs works.

</p>


<%
  if (request.getParameter("AddValue") != null) {
    session.setAttribute(addPrefix(request.getParameter("NameField")), 
  	              request.getParameter("ValueField"));
  } else if (request.getParameter("DeleteValue") != null) {
    session.removeAttribute(addPrefix(request.getParameter("NameField")));
  }
%>

<center>
<table border=1 cellspacing=2 cellpadding=5 width=400 bgcolor=#EEEEEE>
<th colspan=2>Session<br>

</th>
<tr>
<td><B>Name</B></td>
<td><B>Value</B></td>
</tr>

<%
  Enumeration sessionNames = session.getAttributeNames();
  String name;
  while (sessionNames.hasMoreElements()) {
    name = (String)sessionNames.nextElement();    
%>

<tr>
<td><%= removePrefix(name) %></td>
<td><%= (String)session.getAttribute(name) %></td>
</tr>

<%
  } // end of while loop for session names
%>

</table>
</center>
<p>

<form method="post" name="SessionServlet" action="SessionServlet.jsp">

<center>
<table border=0 cellspacing=2 cellpadding=5 width=400>
<th>Name to add/delete</th>
<th>Value</th>
<tr>
<td><input type="text" name="NameField"></td>
<td><input type="text" name="ValueField"></td>
</tr>

<tr>
<td colspan=2 align=center><input type="submit" value=" Add " name="AddValue"></td>
</tr>
<tr>
<td colspan=2 align=center><input type="submit" value="Delete" name="DeleteValue"></td>
</tr>
</table>
</center>

</form>
<p>
<font size=-1>Copyright (c) 1999-2000 by BEA Systems, Inc. All Rights Reserved.
</font>

</font>
</body>
</html>

<%! 
static final String PREFIX_LABEL="SessionServlet.";

private String removePrefix(String name) {
  if (name.startsWith(PREFIX_LABEL))
    name = name.substring(PREFIX_LABEL.length());
  return name;
} 

private String addPrefix(String name) {
  if (!name.startsWith(PREFIX_LABEL))
    name = PREFIX_LABEL + name;
  return name;
}

%>

