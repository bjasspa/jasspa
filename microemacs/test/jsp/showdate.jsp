<!doctype html public "-//w3c/dtd/HTML 4.0//en">
<html>
<!-- Copyright (c) 1999-2000 by BEA Systems, Inc. All Rights Reserved. -->

<head>
	<title>JSP DATE EXAMPLE</title>
	<%@ import="java.util.Date" %>
</head>
<body bgcolor=#ffffff>
<font color=#DB1260>
<h1>JSP DATE</h1>
<h2>
<% response.setHeader("Refresh", "5"); %>
The current date is <%= new Date() %>.
</h2>
<font size=-1>
Copyright © 1999-2000 by BEA Systems, Inc. All Rights Reserved.
</font>
</BODY>
</HTML>
