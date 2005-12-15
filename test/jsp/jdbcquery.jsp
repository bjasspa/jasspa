
<!doctype html public "-//w3c/dtd HTML 4.0//en">
<html>
<!-- Copyright (c) 1999-2000 by BEA Systems, Inc. All Rights Reserved.-->
<head>
<title>JDBC Table Servlet</title>
</head>

<body bgcolor="#FFFFFF">
<font face="Helvetica">

<h2><font color=#DB1260>Using JSP to retrieve database data with JDBC</font></h2>

<h4>Make a selection</h4>

<p>

Choose a JDBC driver and a database name from the drop down
lists below. 

Note that to use the 'demoPool' connection pool option, you must configure
your <font face="Courier New" size=-1>weblogic.properties</font> file. The
'demoPool' connection pool properties are provided by default, but they may be
commented out.

<p> 
<form method="post" name="JdbcTable" action="JdbcTable.jsp">

<table border=0 cellspacing=2 cellpadding=2 width=80%>
<tr> 
<td width=30%><font face="Helvetica"><b>JDBC driver :</b></td>
<td><font face="Helvetica"><select name="jdbcDriver">
  <option value="COM.cloudscape.core.JDBCDriver">COM.cloudscape.core.JDBCDriver</option>
  <option value="weblogic.jdbc.pool.Driver">weblogic.jdbc.pool.Driver</option>
</select></td>
</tr>

<tr> 
<td width=30%><font face="Helvetica"><b>Database URL / Connection Pool :</b></td>
<td><font face="Helvetica"><select name="dbURL">
  <option value="jdbc:cloudscape:demo">jdbc:cloudscape:demo</option>
  <option value="jdbc:weblogic:pool:demoPool">jdbc:weblogic:pool:demoPool</option>
</select></font></td>
</tr>

<tr>
<td width=30%><font face="Helvetica"><b>Username :</b></td>
<td><font face="Helvetica"><input type="text" name="username" size=30></font></td>
</tr>

<tr>
<td width=30%><font face="Helvetica"><b>Password :</b></td>
<td><font face="Helvetica"><input type="password" name="passwd" size=30></font></td>
</tr>

<tr> 
<td width=30%><font face="Helvetica"><b>SQL Query :</b></td>
<td><font face="Helvetica"><input type="text" name="sqlQuery" size=50 value="Select * from emp"></td>
</tr>

<tr> 
<td><font face="Helvetica"><input type="Submit" value="Submit Query" name="Submit"></td>
</tr>
</table>

</form>

<hr width=80%>

<%@ page import="
javax.naming.*,
java.util.*,
java.sql.*,
weblogic.common.*
" %>

<%
  if ("POST".equals(request.getMethod())) {

    String jdbcDriver = (String) request.getParameter("jdbcDriver");
    String dbURL = (String) request.getParameter("dbURL");
    String sqlQuery = (String) request.getParameter("sqlQuery");
    String username = (String) request.getParameter("username");
    if (username != null && username.equals(""))
        username=null;
    String passwd = (String) request.getParameter("passwd");
    if (passwd != null && passwd.equals(""))
        passwd = null;
%>

<h2>Results from previous query:</h2>

Here are the results from the previous SQL query using the these parameters:

<ul>
<li> JDBC Driver: <%= jdbcDriver==null?"No driver specified.":jdbcDriver %>
<li> Database URL: <%= dbURL==null?"No URL specified":dbURL %>
<li> SQL query: <%= sqlQuery==null?"No SQL query":sqlQuery %>
<li> Username: <%= username==null?"<i>No username supplied</i>":username %>
<li> Password: <%= passwd==null?"<i>No password supplied</i>":passwd %>
</ul>
<p>
<%

    Connection conn = null;
    Statement stmt = null;
    ResultSet rs = null;

    try {
      Class.forName(jdbcDriver).newInstance();
      if ((username != null) && (passwd != null))
        conn = DriverManager.getConnection(dbURL, username, passwd);
      else
        conn = DriverManager.getConnection(dbURL, null);
      stmt = conn.createStatement();
      rs = stmt.executeQuery(sqlQuery);

      ResultSetMetaData rsmd = rs.getMetaData();
      int numCols = rsmd.getColumnCount();
%>

<p>
<center>
<table border=1 cellspacing=2 cellpadding=0 width=400>
<tr>

<%
    for (int i = 1; i <= numCols; i++) {
%>

<td><font face="Helvetica"><b><%= rsmd.getColumnLabel(i) %></b></td>

<%
    }
%>

</tr>

<%
    while (rs.next()) {
%>

<tr> 

<%
      for (int i = 1; i <= numCols; i++) {
%>

<td><font face="Helvetica"><%= rs.getString(i) %></td>

<%
      } 
%>

</tr>

<%
    } 
  } 
  catch (Exception e) {
%>

<p><b>There was an error executing or processing the query:</b>
<br>
Exception: <%= e %>	
<pre><%= getStackTraceAsString(e) %></pre>

<%
  } 

  finally {
    try {
      rs.close();
      stmt.close();
      conn.close();
    } 
    catch (Exception e) {
      out.print("<b>There was an error closing the database connection</b>");
      out.print("<br>Exception: " +e);	
      out.print("<br><pre>"+getStackTraceAsString(e)+"</pre>");
    } 
  } 
}
%>

</table>
</center>
<p>
<font size=-1>Copyright (c) 1999-2000 by BEA Systems, Inc. All Rights Reserved.
</font>

</font>
</body>
</html>

<%!

  String getStackTraceAsString(Exception e)
  {
    // Dump the stack trace to a buffered stream, then return it's contents
    // as a String. This is useful for printing the stack to 'out'. 
    ByteArrayOutputStream ostr = new ByteArrayOutputStream();
    e.printStackTrace(new PrintStream(ostr));
    return(ostr.toString());
  }

%>

