

<!doctype html public "-//w3c//dtd html 3.2//en">
<html>
<!-- Copyright (c) 1999-2000 by BEA Systems, Inc. All Rights Reserved.-->

<head>
<title>Employee Update</title>
</head>
<body bgcolor=#FFFFFF>

<font face="Helvetica">

<h1>
<font color=#DB1260>
Employee List
</font>
</h1>

<%@ page import="
weblogic.db.jdbc.*, 
weblogic.html.*, 
java.sql.*
" %>

<p>

<%
  Connection conn = null;
  try {
    Class.forName("weblogic.jdbc.pool.Driver").newInstance();
    conn = DriverManager.getConnection("jdbc:weblogic:pool:demoPool");
    // Fetch all records from the database in a TableDataSet
    DataSet dSet = new TableDataSet(conn, "emp").fetchRecords();
    TableElement tE = new TableElement(dSet);
    tE.setBorder(1);
    out.print(tE);
  } catch (SQLException sqle) {
    out.print("Sorry, the database is not available.");
    out.print("Exception: " + sqle);
  } catch (Exception e) {
    out.print("Exception occured: " + e);
  } finally {
    if(conn != null)
      try { 
        conn.close(); 
      } catch(SQLException sqle) {}
  }
%>

<p>Please call Mary with any updates ASAP!

<p>
<font size=-1>Copyright (c) 1999-2000 by BEA Systems, Inc. All Rights Reserved.
</font>

</font>
</body>
</html>
