<!--#INCLUDE FILE=..\constants.bas--> 
<%
  If Request.QueryString("op").Count then
    Session("AdminOp") = Request.QueryString("op")
  End if
%>
<html>
  <%if Session("msie") then%>
    <frameset rows="20,*">
  <%else%>
    <frameset rows="32,*">
  <%end if%>
    <frame src="sherpaops.asp" name="ops" marginwidth="0" marginheight="0" scrolling="no" FRAMEBORDER=1 NORESIZE>    
    <%Select Case Session("AdminOp")%>
      <%case else%>        
        <frame src="SherpaAgentConfig.asp" name="wizard" marginwidth="0" marginheight="0" FRAMEBORDER=1>
    <%end select%>
  </frameset>
</html>
