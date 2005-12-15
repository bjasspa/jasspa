<!doctype html public "-//w3c/dtd HTML 4.0//en">
<html>

<!-- Creates an HTML page that can be used interactively to take
input from a client and then structure a query with it.
Copyright (c) 1999-2000 by BEA Systems, Inc. All Rights Reserved.-->

<head>
<title>Interactive Query</title>
</head>
<body bgcolor=#FFFFFF>
<font face="Helvetica">

<h2>
<font color=#DB1260>
Interactive Query
</font>
</h2>

<%@ import="
weblogic.db.jdbc.*, 
java.sql.*
" %>

<%!
Connection conn  = null;
String jdbcClass = "COM.cloudscape.core.JDBCDriver";
String jdbcURL   = "jdbc:cloudscape:demo";

public Connection getCon() {
  try {
    Class.forName(jdbcClass).newInstance();
    conn = DriverManager.getConnection(jdbcURL);
  } catch (Exception e) {}
  return conn;
}
%>

<%
try {
  conn = getCon();
  if (conn != null) {
    Statement stmt = conn.createStatement();
    stmt.execute("select * from emp");
    ResultSet ds = stmt.getResultSet();
      
    String myURL  = request.getRequestURI();
    String person = request.getParameter("person");
      
    if (person == null) {
%>

      <table border=1 cellpadding=5>
        <th>Employee no</th>
        <th>Select a name</th>
        <th>Job</th>
        <th>Manager</th>
        <th>Date of hire</th>
        <th>Salary</th>
        <th>Commission</th>
        <th>Dept no</th>

<%
      while (ds.next()) {
        String ename = "<a href=" + myURL + "?person=" +
	                  ds.getString("ename") + ">" +
		             ds.getString("ename") + "</a>";
%>

        <tr>
          <td><%= ds.getString("empno") != null ? ds.getString("empno") : " " %></td>
          <td><%= ename %></td>
          <td><%= ds.getString("job") != null ? ds.getString("job") : " " %></td>
          <td><%= ds.getString("mgr") != null ? ds.getString("mgr") : " " %></td>
          <td><%= ds.getString("hiredate") != null ? ds.getString("hiredate") : " " %></td>
          <td><%= ds.getString("sal") != null ? ds.getString("sal") : " " %></td>
          <td><%= ds.getString("comm") != null ? ds.getString("comm") : " " %></td>
          <td><%= ds.getString("deptno") != null ? ds.getString("deptno") : " " %></td>
        </tr>

<%
      }
%>      

      </table>

<%
      ds.close();
      conn.close();
      out.flush();
    }
    else {
%>    

      <p>
      <b>Results of your interactive query</b>:
      <p>
      You chose person: <%= person %>

<%
    }
  }
  else {
    out.print("Sorry. Database is not available.");
  }
} catch (Exception e) {
  out.print("Exception: " + e);
}
%>

<p>
<font size=-1>Copyright (c) 1999-2000 by BEA Systems, Inc. All Rights Reserved.
</font>

</font>
</body>
</html>

