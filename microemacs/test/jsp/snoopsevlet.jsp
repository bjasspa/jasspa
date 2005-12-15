<html>
<!-- Copyright (c) 1999-2000 by BEA Systems, Inc. All Rights Reserved.-->
<head>
<title>Snoop Servlet</title>
</head>

<body bgcolor=#FFFFFF>
<font face="Helvetica">

<h2>
<font color=#DB1260>
Snoop Servlet
</font>
</h2>

<p>
This servlet returns information about the HTTP request
itself. You can modify this servlet to take this information
and store it elsewhere for your HTTP server records. This
servlet is also useful for debugging.

<h3>
Requested URL
</h3>

<pre>
<%= HttpUtils.getRequestURL(request) %>
</pre>

<h3>
Init parameters
</h3>

<pre>
<%
Enumeration e = getServletConfig().getInitParameterNames();
while (e.hasMoreElements()) {
  String name = (String)e.nextElement();
  out.println(name + ": " + getInitParameter(name));
}
%>
</pre>

<h3>
Request information
</h3>

<pre>
Request Method: <%= request.getMethod() %>
Request URI: <%= request.getRequestURI() %>
Request Protocol: <%= request.getProtocol() %>
Servlet Path: <%= request.getServletPath() %>
Path Info: <%= request.getPathInfo() %>
Path Translated: <%= request.getPathTranslated() %>
Query String: <%= request.getQueryString() %>
Content Length: <%= request.getContentLength() %>
Content Type: <%= request.getContentType() %>
Server Name: <%= request.getServerName() %>
Server Port: <%= request.getServerPort() %>
Remote User: <%= request.getRemoteUser() %>
Remote Address: <%= request.getRemoteAddr() %>
Remote Host: <%= request.getRemoteHost() %>
Authorization Scheme: <%= request.getAuthType() %>
</pre>

<h3>
Request headers
</h3>

<pre>
<%
e = request.getHeaderNames();
while (e.hasMoreElements()) {
  String name = (String)e.nextElement();
  out.println(name + ": " + request.getHeader(name));
}
%>
</pre>

<p>
<font size=-1>Copyright © 1999-2000 by BEA Systems, Inc. All Rights Reserved.
</font>

</font>
</body>
</html>

