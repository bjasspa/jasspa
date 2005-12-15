
<!-- Copyright (c) 1999-2000 by BEA Systems, Inc. All Rights Reserved.-->

<%@ page isErrorPage="true" %>

<!-- This error page demonstrates a JSP ErrorPage used for
     catching exceptions thrown in other JSP pages, it is
     used by the ThrowException.jsp page -->

<html>
<head><title>JSP Error Page</title></head>

<body bgcolor=#ffffff>

<font face="Helvetica">

<h2><font color=#DB1260>JSP Error Page</font></h2>

<p> An exception was thrown: <b> <%= exception %>

<p> With the following stack trace:
<pre>

<%
    ByteArrayOutputStream ostr = new ByteArrayOutputStream();
    exception.printStackTrace(new PrintStream(ostr));
    out.print(ostr);
%>
</pre>

<p>
<hr width=80%>

</body>
</html>

