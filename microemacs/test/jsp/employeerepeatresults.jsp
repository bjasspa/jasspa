<HTML>
<HEAD>
<TITLE>JSP Employee Results</TITLE>
</HEAD>
<H1><CENTER>EMPLOYEE RESULTS</CENTER></H1>
<BODY>

<tsx:dbconnect id="conn" url="jdbc:db2:sample" driver="COM.ibm.db2.jdbc.app.DB2Driver">
<userid><tsx:getProperty name="request" property=request.getParameter("USERID") /></userid>
<passwd><tsx:getProperty name="request" property=request.getParameter("PASSWD") /></passwd>
</tsx:dbconnect>

<% if  ( ( request.getParameter("Submit")).equals("Update") ) { %>
<tsx:dbmodify connection="conn" >
INSERT INTO  EMPLOYEE
(EMPNO,FIRSTNME,MIDINIT,LASTNAME,WORKDEPT,EDLEVEL)
VALUES
( '<tsx:getProperty name="request" property=request.getParameter("EMPNO") />',
'<tsx:getProperty name="request" property=request.getParameter("FIRSTNME") />',
'<tsx:getProperty name="request" property=request.getParameter("MIDINIT") />',
'<tsx:getProperty name="request" property=request.getParameter("LASTNAME") />',
'<tsx:getProperty name="request" property=request.getParameter("WORKDEPT") />',
<tsx:getProperty name="request" property=request.getParameter("EDLEVEL") />)
</tsx:dbmodify>
<B><UL>UPDATE SUCCESSFUL</UL></B>
<BR><BR>
<tsx:dbquery id="qs" connection="conn" >
select * from Employee  where  WORKDEPT= '<tsx:getProperty name="request" property=request.getParameter("WORKDEPT") />'
</tsx:dbquery>


<B><CENTER><U>EMPLOYEE LIST</U></CENTER></B><BR><BR>
<HR>
<TABLE>
<TR VALIGN=BOTTOM>
<TD><B>EMPLOYEE
<BR>
<U>NUMBER</U></B></TD>
<TD><B><U>NAME</U></B></TD>
<TD><B><U>DEPARTMENT</U></B></TD>
<TD><B><U>EDUCATION</U></B></TD>
</TR>

<tsx:repeat>
<TR>
<TD><B><I><tsx:getProperty name="qs" property="EMPNO" /></I></B></TD>
<TD><B><I><tsx:getProperty name="qs" property="FIRSTNME" /></I></B></TD>
<TD><B><I><tsx:getProperty name="qs" property="WORKDEPT" /></I></B></TD>
<TD><B><I><tsx:getProperty name="qs" property="EDLEVEL" /></I></B></TD>
</TR>
</tsx:repeat>

</TABLE>


<HR>
<BR>
<% } %>

<% if  ( ( request.getParameter("Submit")).equals("Query") ) { %>

<tsx:dbquery id="qs2" connection="conn" >
select * from Employee  where  WORKDEPT= '<tsx:getProperty name="request" property=request.getParameter("WORKDEPT") />'
</tsx:dbquery>


<B><CENTER><U>EMPLOYEE LIST</U></CENTER></B><BR><BR>
<HR>
<TABLE>
<TR>
<TR VALIGN=BOTTOM>
<TD><B>EMPLOYEE
<BR>
<U>NUMBER</U></B></TD>
<TD><B><U>NAME</U></B></TD>
<TD><B><U>DEPARTMENT</U></B></TD>
<TD><B><U>EDUCATION</U></B></TD>
</TR>

<tsx:repeat>
<TR>
<TD><B><I><tsx:getProperty name="qs2" property="EMPNO" /></I></B></TD>
<TD><B><I><tsx:getProperty name="qs2" property="FIRSTNME" /></I></B></TD>
<TD><B><I><tsx:getProperty name="qs2" property="WORKDEPT" /></I></B></TD>
<TD><B><I><tsx:getProperty name="qs2" property="EDLEVEL" /></I></B></TD>
</TR>
</tsx:repeat>
</TABLE>
<HR>
<BR>
<% } %>

</BODY>
</HTML>
