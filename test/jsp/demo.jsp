
<%--
  This is an example from Sun for a template modified by me
  
  Copyright 2001 Sun Microsystems, Inc. All Rights Reserved.
  
  This software is the proprietary information of Sun Microsystems, Inc.  
  Use is subject to license terms.
  
--%>

<%@ taglib uri="/tutorial-template" prefix="tt" %>
<%@ page errorPage="errorpage.jsp" %>
<%@ page import="java.util.*" %>
<% ResourceBundle messages = (ResourceBundle)session.getAttribute("messages"); %>
<%@ include file="screendefinitions.jsp" %>
<html>
<head>
<title>
  <tt:insert definition="bookstore" parameter="title"/>
</title>
</head>
<body>
<%= out.println ("Whatever you want to print") %>
<p>Some more html
</p>

<%
String parameter1 = new String ();

if (request.getParameter ("parameter1") != null) {
    parameter1 = request.getParameter ("parameter1");
    if (parameter1.equals ("something else") {
        %>
        <p>Whatever HTML you want to put here, hopefully indented but this is not as important</p>
        <%
    }
}
    
//If you type the loop below you'll see that it indents using the position of the "<"
for (int i = 0; i < 10; i++) {
    out.println (i + "something else");                   
}
  
%>
</body>
</html>

