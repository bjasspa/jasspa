<!--#INCLUDE FILE=..\constants.bas-->
<!--#INCLUDE FILE=..\parsing.bas-->
<!--#INCLUDE FILE=../Localization/StringTable.bas-->
<!--#INCLUDE FILE=..\WaitingDlg.bas-->

<html>
<HEAD>
<TITLE>EDS Administrator </TITLE>

<%
'********************** ADMIN SETUP *********************
  if Request.QueryString("start").Count then
    TaskID = Request.QueryString("start")
    Set o = Server.CreateObject("OlObjectConverter.edsMonitor")
    o.MaxJobTime = 15
    Response.Write "Start Task: " & o.StartTask(TaskID, ROOT_DIR)

  elseif Request.QueryString("stop").Count then
    proxy = Request.QueryString("stop")
    if proxy = "" then
      Response.Write "That process cannot be stopped by the admin wizard."
    else
      Set o = Server.CreateObject(proxy)
      o.MaxJobTime = 10
      o.Terminate
    end if

  elseif Request.QueryString("shutdown").Count then
    Set o = Server.CreateObject("OlObjectConverter.edsMonitor")
    o.MaxJobTime = 60
    WriteWaitingDlg 60
    o.StopAll
    response.write "<" + "SCRIPT LANGUAGE='JAVASCRIPT'>" + vbcrlf
    response.write "  location = 'Admin.asp' ;" + vbcrlf
    response.write "</" + "SCRIPT>" + vbcrlf
    response.end
  elseif Request.QueryString("sync").Count then
    Set o = Server.CreateObject("OlObjectConverter.edsMonitor")
    o.MaxJobTime = 60
    WriteWaitingDlg 60
    Response.Write o.Sync(ROOT_DIR)
    response.write "<" + "SCRIPT LANGUAGE='JAVASCRIPT'>" + vbcrlf
    response.write "  location = 'Admin.asp' ;" + vbcrlf
    response.write "</" + "SCRIPT>" + vbcrlf
    response.end
  end if
%>



</HEAD>

<BODY BGCOLOR=#FFFFFF leftmargin=5 >
<FONT SIZE=<%=FONT_SIZE%> FACE="<%=FONT_FACE%>">



<!-- *********************** START WIZARD TABLE ************************** -->
<img src="<%=IMAGE_PATH%>nothing.gif" height=10 width=10 ><BR>


<CENTER>
<TABLE BORDER BGCOLOR="<%=STATUS_BGCOLOR%>" WIDTH=95%>
  <TR VALIGN="top">
  <TH BGCOLOR="<%=ADMIN_TITLE_BGCOLOR%>" >
    <FONT FACE="<%=FONT_FACE%>" COLOR="#FFFFFF" >
Current EDS Server Status
  <% if StepNum > 0 then %>
  Wizard - Step: <%=StepNum%>
  <%end if%>
    </FONT>
  </TH>
  </TR>

  <TR VALIGN="top"><TD>
  <! **** BEGIN INTERFACE INFO TABLE **** >

  <TABLE CELLPADDING=5 WIDTH=100% >
  <TR VALIGN="top"> <TD WIDTH=320>
    <FONT FACE="<%=FONT_FACE%>" >







    <!-- ******************** STATUS INFO **************** -->
    <TABLE BGCOLOR="#FFFFFF" BORDER>
    <TR><TD>

      <TABLE CELLSPACING=2 CELLPADDING=3 >
      <TR VALIGN="top">
        <TH BGCOLOR="<%=TABLE_BGCOLOR%>" >
        <FONT FACE="<%=FONT_FACE%>" >
      Process
      </FONT>
        </th>
        <TH BGCOLOR="<%=TABLE_BGCOLOR%>" >
        <FONT FACE="<%=FONT_FACE%>" >
      Status
      </FONT>
        </th>
        <TH BGCOLOR="<%=TABLE_BGCOLOR%>" >
        <FONT FACE="<%=FONT_FACE%>" >
      <a href="Admin.Asp?sync=1">Start</a>
      </FONT>
        </th>
        <TH BGCOLOR="<%=TABLE_BGCOLOR%>" >
        <FONT FACE="<%=FONT_FACE%>" >
      <a href="Admin.Asp?shutdown=1">Stop</a>
      </FONT>
        </th>
      </tr>
    <%
      Set o = Server.CreateObject("OlObjectConverter.edsMonitor")
      o.MaxJobTime = 10

      Output = "<tr valign=top ><td> <FONT FACE=" & InQuotes(FONT_FACE) & " SIZE=" & InQuotes(FONT_SIZE) & " > "
      Output = Output & "{title}</FONT></td>"
      Output = Output & "<td align=" + InQuotes("center") + ">"
      Output = Output & "<FONT FACE=" & InQuotes(FONT_FACE) & " SIZE=" & InQuotes(FONT_SIZE) & " > "
      Output = Output & "<B>{status}</B></FONT></td>"
      Output = Output & "<td align=" + InQuotes("center") + "><a href=" & InQuotes("Admin.Asp?start={id}") & "><img src=start.gif></a></td>"
      Output = Output & "<td align=" + InQuotes("center") + "><a href=" & InQuotes("Admin.Asp?stop={proxy}") & "><img src=stop.gif></a></td></tr>"
      Response.Write o.GetStatus(OutPut)
    %>

      </table>
    </TD></TR>
    </TABLE>

    <!-- ******************** END STATUS INFO **************** -->


    </TD><TD>
    <FONT FACE="<%=FONT_FACE%>"  SIZE="<%=FONT_SIZE%>" >


    <!--#INCLUDE FILE=help_admin.asp-->


  </FONT>
  </TD></TR>
  </TABLE>

  </FONT>
  </TD></TR>
</TABLE>

</CENTER>




</FONT>
</BODY>

</html>