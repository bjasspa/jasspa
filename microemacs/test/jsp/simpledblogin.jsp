<!doctype html public "-//w3c/dtd HTML 4.0//en">
<html>

<!-- Illustrates a simple database login through an HTML interface. -->
<!-- Copyright (c) 1999-2000 by BEA Systems, Inc. All Rights Reserved. -->

<head>
<title>Simple Database Login</title>
</head>
<body bgcolor=#FFFFFF>

<font face="Helvetica">

<h2>
<font color=#DB1260>
Simple database login
</font>
</h2>

<%@ import="
import java.sql.*
" %>

<%!
String jdbcClass = "COM.cloudscape.core.JDBCDriver";
String jdbcURL   = "jdbc:cloudscape:demo";
String user      = "scott";
String password  = "tiger";
%>

<%
try {
  password = "tiger";
  
  Class.forName(jdbcClass).newInstance();
  Connection conn =
    DriverManager.getConnection(jdbcURL, user, password);
  out.print("<p>First login attempt was successful for ");
  out.print(user + "/" + password);

  password = "tigger";
  Connection conn2 =
    DriverManager.getConnection(jdbcURL, user, password);
  out.print("<p>Second login attempt was successful for ");
  out.print(user + "/" + password);
  
  }
catch (Exception e) {

  out.print("<p>Login failed for " + user + "/" + password);
  out.print("<p>");
  out.print("<pre>");
  e.printStackTrace(new PrintStream(out));
  out.print("</pre>");

  }  

%>

<p>
<font size=-1>
Copyright © 1999-2000 by BEA Systems, Inc. All Rights Reserved.
</font>

</font>
</body>
</html>

