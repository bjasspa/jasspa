<!--#INCLUDE FILE=..\constants.bas--> 
<!--#INCLUDE FILE=..\parsing.bas--> 
<!--#INCLUDE FILE=common_admin.bas--> 
<!--#INCLUDE FILE=NavigateHead.asp--> 

<SCRIPT LANGUAGE=VBScript RUNAT=Server>						 
  
  Sub DisplayClassConfig()
    NumClasses = Ini.GetVal(SECTION_SECURITY, KEY_NUMCLASSES)
    ' Response.Write "Classes Defined: " & NumClasses
    Response.Write "<table cellpadding=5>" & vbcrlf & "<tr><th BGCOLOR=" & InQuotes(STATUS_BGCOLOR) & ">"
  Response.Write "<FONT FACE=" & InQuotes(FONT_FACE) & " SIZE=" & InQuotes(FONT_SIZE) & " > "
  Response.Write "Class"
  Response.Write "</FONT></th>" & VBCRLF
  Response.Write "<th BGCOLOR=" & InQuotes(STATUS_BGCOLOR) & ">"
  Response.Write "<FONT FACE=" & InQuotes(FONT_FACE) & " SIZE=" & InQuotes(FONT_SIZE) & " > "
  Response.Write "Account"
  Response.Write "</FONT></th>" & VBCRLF
  Response.Write "<th BGCOLOR=" & InQuotes(STATUS_BGCOLOR) & ">"
  Response.Write "<FONT FACE=" & InQuotes(FONT_FACE) & " SIZE=" & InQuotes(FONT_SIZE) & " > "
  Response.Write "Users</th></tr>"

  for i = 1 to NumClasses
      class = SECTION_CLASS & i
      Account = Ini.GetStr(class, KEY_ACCOUNT)
      Users = Ini.Getstr(class, KEY_USERS)
      Response.Write "<tr valign=top ><td ALIGN=center >"
    Response.Write "<FONT FACE=" & InQuotes(FONT_FACE) & " SIZE=" & InQuotes(FONT_SIZE) & " > "
    Response.Write i 
    Response.Write "</FONT></td><td>"
    Response.Write "<FONT FACE=" & InQuotes(FONT_FACE) & " SIZE=" & InQuotes(FONT_SIZE) & " > "
    Response.Write Account & "</FONT></td><td>"
    Response.Write "<FONT FACE=" & InQuotes(FONT_FACE) & " SIZE=" & InQuotes(FONT_SIZE) & " > "
    Response.Write Users& "</FONT></td></tr>"      
    next
    Response.Write "</table>"
  End Sub


  Sub DisplayClassList(name, SelectedClass)
    Response.Write "<select name=" & InQuotes(name) & ">"
    NumClasses = Ini.GetVal(SECTION_SECURITY, KEY_NUMCLASSES)    
    for i = 1 to NumClasses      
      class = SECTION_CLASS & i
      if i = clng(SelectedClass) then
        s = " selected"
      else
        s = ""
      end if
      Account = Ini.GetStr(class, KEY_ACCOUNT)      
      Response.Write "<option value=" & i & s & ">(" & i & ") " & Account & "</option>"      
    next  
    Response.Write "</select>"
  End Sub





  '************************
  '* DISPLAY SUMMARY INFO *
  '************************

  Sub DisplaySummary

  transferdir = Ini.GetStr(SECTION_MASTER, KEY_REMOTETRANSFERDIR)
  host = Ini.GetStr(SECTION_MASTER, KEY_HOST)
  username = Ini.GetStr(SECTION_MASTER, KEY_USERNAME)
  password = Ini.GetStr(SECTION_MASTER, KEY_PASSWORD)
  prompt = Ini.GetStr(SECTION_MASTER, KEY_PROMPT)
  UseSecurity = Ini.GetBool(SECTION_SECURITY, KEY_ENABLED)
  UseClasses = Ini.GetBool(SECTION_SECURITY, KEY_USECLASSES)

     if not UseSecurity then
       sec = "Disabled"
     else
       sec = "Enabled"
     end if

     if UseClasses then
       class = "User Class Accounts"
     else
       class = "Individual User Accounts"
     end if



  Response.Write WriteTableStart

  Response.Write "    <TR VALIGN=" & InQuotes("top") & " >"
  DisplayCell "Sherpa Host Name:", 0
  DisplayCell Host, 1
  Response.Write "    </TR>" & vbcrlf

  Response.Write "    <TR VALIGN=" & InQuotes("top") & " >"
  DisplayCell "Unix Username:", 0
  DisplayCell USERNAME, 1
  Response.Write "    </TR>" & vbcrlf

  Response.Write "    <TR VALIGN=" & InQuotes("top") & " >"
  DisplayCell "System Prompt:", 0
  DisplayCell prompt, 1
  Response.Write "    </TR>" & vbcrlf

  Response.Write "    <TR VALIGN=" & InQuotes("top") & " >"
  DisplayCell "Databases:", 0
  DisplayCell SHERPA_DATABASES, 1
  Response.Write "    </TR>" & vbcrlf
  Response.Write "    <TR VALIGN=" & InQuotes("top") & " >"

    Response.Write "    <TR VALIGN=" & InQuotes("top") & " >"
    DisplayCell "Publish Parts Offline:", 0
    DisplayCell SHERPA_POST_PRT, 1
    Response.Write "    </TR>" & vbcrlf

    Response.Write "    <TR VALIGN=" & InQuotes("top") & " >"
    DisplayCell "Publish Drawings Offline:", 0
    DisplayCell SHERPA_POST_DRW, 1
    Response.Write "    </TR>" & vbcrlf

    Response.Write "    <TR VALIGN=" & InQuotes("top") & " >"
    DisplayCell "Publish Assemblies Offline:", 0
    DisplayCell SHERPA_POST_ASM, 1
    Response.Write "    </TR>" & vbcrlf

    'Response.Write "    <TR VALIGN=" & InQuotes("top") & " >"
    'DisplayCell "Store All Files in EDR:", 0
    'DisplayCell SHERPACacheAllFiles, 1
    'Response.Write "    </TR>" & vbcrlf

    Select Case SHERPAInsertMetaData
      case 0 
        s = "None"
      case 1
        s = "During Publish"
      case 2
        s= "On Demand"
      case else
    end select

    Response.Write "    <TR VALIGN=" & InQuotes("top") & " >"
    DisplayCell "Sherpa Metadata Merging:", 0
    DisplayCell s, 1
    Response.Write "    </TR>" & vbcrlf


  Response.Write "    <TR VALIGN=" & InQuotes("top") & " >"
  DisplayCell "Security:", 0
  DisplayCell sec, 1
  Response.Write "    </TR>" & vbcrlf

  if UseSecurity then
    Response.Write "    <TR VALIGN=" & InQuotes("top") & " >"
    DisplayCell "User Access With:", 0
    DisplayCell class, 1
    Response.Write "    </TR>" & vbcrlf
  end if
  
  Response.Write "    </TR>" & vbcrlf
  Response.Write "</TABLE>"

  End Sub

</SCRIPT>

<HTML>
<HEAD>
  <TITLE> <%=PDM_NAME%> Configuration Wizard </TITLE>

<BODY BGCOLOR=#FFFFFF leftmargin=5 topmargin=5 >

    <FONT FACE="<%=FONT_FACE%>" >

<%
  CONST PDM_NAME = "Sherpa"
  
  
  CONST SECTION_MASTER = "master"
    CONST KEY_HOST = "host"
    CONST KEY_USERNAME = "username"
    CONST KEY_PASSWORD = "password"
    CONST KEY_PROMPT = "prompt"
    CONST KEY_REMOTETRANSFERDIR = "remotetransferdir"
  
  CONST SECTION_SECURITY = "security"
    CONST KEY_ENABLED = "enabled"
    CONST KEY_USECLASSES = "useclasses"
    CONST KEY_NUMCLASSES = "numclasses"
  
  CONST SECTION_CLASS = "class"
    CONST KEY_ACCOUNT = "account"    
    CONST KEY_USERS = "users"

  CONST SECTION_PUBLISH = "publish"
    'CONST KEY_CACHEALL = "cacheallfiles"
    CONST KEY_METADATA = "insertmetadata"  

  'SHERPACacheAllFiles = SHERPA_CACHEALLFILES
  SHERPAInsertMetaData = SHERPA_INSERT_METADATA


  AgentIniFile = AddPathToFile(ROOT_DIR, "ObjectAdapters/Sherpa/agent.ini")
  ConstFile = AddPathToFile(ROOT_DIR, "eds\constants.bas")


  Set Admin = Server.CreateObject("olStateManager.Admin")
  Set Ini = Server.CreateObject("olStateManager.IniFile")
  Ini.IniFile = AgentIniFile

  if Request.Form("host").Count Then
    host = Request.Form("host")
    Admin.SetIniFileParam AgentIniFile, SECTION_MASTER, KEY_HOST, host
  end if
  if Request.Form("username").Count Then
    username = Request.Form("username")
    Admin.SetIniFileParam AgentIniFile, SECTION_MASTER, KEY_USERNAME, username
  end if
  if Request.Form("password").Count Then
    password = Request.Form("password")
    Admin.SetIniFileParam AgentIniFile, SECTION_MASTER, KEY_PASSWORD, password
  end if
  if Request.Form("prompt").Count Then
    prompt = Request.Form("prompt")
    Admin.SetIniFileParam AgentIniFile, SECTION_MASTER, KEY_PROMPT, prompt
  end if  

  if Request.Form("usesecurity").Count Then
    UseSecurity = Request.Form("usesecurity")
    Admin.SetIniFileParam AgentIniFile, SECTION_SECURITY, KEY_ENABLED, UseSecurity    
  end if

  if Request.Form("useclasses").Count Then
    UseClasses = Request.Form("useclasses")
    Admin.SetIniFileParam AgentIniFile, SECTION_SECURITY, KEY_USECLASSES, UseClasses
  end if

  if Request.Form("databases").Count Then
    dbs = Request.Form("databases")
    Admin.SetStringConstant ConstFile, "SHERPA_DATABASES", dbs
  end if

  if Request.Form("setpost").Count Then  
    b = (Request.Form("postprt") = "on")
    Admin.SetConstant ConstFile, "SHERPA_POST_PRT", b

    b = (Request.Form("postdrw") = "on")
    Admin.SetConstant ConstFile, "SHERPA_POST_DRW", b

    b = (Request.Form("postasm") = "on")
    Admin.SetConstant ConstFile, "SHERPA_POST_ASM", b
  end if

  if Request.Form("setedr").Count Then    

    b = Request.Form("metadata")
    Admin.SetConstant ConstFile, "SHERPA_INSERT_METADATA", b
    Admin.SetIniFileParam AgentIniFile, SECTION_PUBLISH, KEY_METADATA, b
    SHERPAInsertMetaData = b

	'b = (Request.Form("cacheall") = "on")
	'Admin.SetIniFileParam AgentIniFile, SECTION_PUBLISH, KEY_CACHEALLFILES, b
	'Admin.SetConstant ConstFile, "SHERPA_CACHEALLFILES", b
    'SHERPAInsertMetaData = b

  end if


%>


<!-- *********************** START WIZARD TABLE ************************** -->
<img src="<%=IMAGE_PATH%>nothing.gif" height=10 width=10 ><BR>


<CENTER>
<TABLE BORDER BGCOLOR="<%=STATUS_BGCOLOR%>" WIDTH=95%>
  <TR VALIGN="top">
  <TH BGCOLOR="<%=ADMIN_TITLE_BGCOLOR%>" >
    <FONT FACE="<%=FONT_FACE%>" COLOR="#FFFFFF" >
<%=PDM_NAME%> Configuration
  <% if StepNum > 0 then %>
  Wizard - Step: 
  <% if StepNum > 10 then
     response.write " 10"
     'response.write StepNum
     else
      response.write StepNum
     end if %>
  <%end if%>
    </FONT>
  </TH>
  </TR>

  <TR VALIGN="top"><TD>


  <! **** BEGIN INTERFACE INFO TABLE **** >

  <TABLE CELLPADDING=5 WIDTH=100% >
  <TR VALIGN="top"> <TD WIDTH=220>
    <FONT FACE="<%=FONT_FACE%>" >


<form  method="POST" action="<%=PDM_NAME%>AgentConfig.asp">

<%Select Case StepNum%>


  <%Case 0 'XXXXXXXXXXX  CONFIG INFO  XXXXXXXXXXXXXXXXXXXXXXX%>    

  <FONT FACE="<%=FONT_FACE%>"  SIZE="<%=FONT_SIZE%>" >

     <%
  NextStep = StepNum + 1
     %>

  <H3> Current <%=PDM_NAME%> Master Account Configuration</H3>
  

  <%DisplaySummary%>
   
   
   
   
   
   <%
    SkipBack = True
    SkipNext = True
    SkipFinish = True
    SkipHelp = True
    ShowConfig = True
   %>

  <HR>

  <!--#INCLUDE FILE=NavigateTail.asp--> 

  
  </FONT>
  </TD><TD> 

  <%
  UseSecurity = Ini.GetBool(SECTION_SECURITY, KEY_ENABLED)
  UseClasses = Ini.GetBool(SECTION_SECURITY, KEY_USECLASSES)
  if UseSecurity and UseClasses then%>
    <TABLE BORDER WIDTH=200 HEIGHT=250 BGCOLOR="white" CELLPADDING=5 >
    <TR VALIGN="top" ><TD>
    <%DisplayClassConfig%>
    </TD></TR></TABLE>

  <%else%>
    <img src="<%=IMAGE_PATH%>wiz_server.jpg" 
    WIDTH=200 HEIGHT=250
    VSPACE=3 HSPACE=5 ><BR>
  <%end if%>

  </TD></TR>
  <TR><TD COLSPAN=2>



  <HR>





  <%Case 1 'XXXXXXXXXXX  SERVER  XXXXXXXXXXXXXXXXXXXXXXX%>    

  <img src="<%=IMAGE_PATH%>wiz_server.jpg" 
  WIDTH=200 HEIGHT=250
  VSPACE=3 HSPACE=5 ><BR>
  </TD><TD>
  <FONT FACE="<%=FONT_FACE%>"  SIZE="<%=FONT_SIZE%>" >

     <%
  NextStep = StepNum + 1

  transferdir = Ini.GetStr(SECTION_MASTER, KEY_REMOTETRANSFERDIR)
  host = Ini.GetStr(SECTION_MASTER, KEY_HOST)
  username = Ini.GetStr(SECTION_MASTER, KEY_USERNAME)
  password = Ini.GetStr(SECTION_MASTER, KEY_PASSWORD)
  prompt = Ini.GetStr(SECTION_MASTER, KEY_PROMPT)
     %>

  <H3> Server Settings </H3>
  This section will identify a master account which can be used to access <%=PDM_NAME%>.

     <table>
     <tr>
     <td>
  <FONT FACE="<%=FONT_FACE%>" SIZE="<%=FONT_SIZE%>" >Sherpa Host Name: </FONT>
     </td><td>
  <input type="input" name="host" size="24" value="<%=Host%>">
     </td>
     </tr><tr>
     <td>
  <FONT FACE="<%=FONT_FACE%>" SIZE="<%=FONT_SIZE%>" >Unix Username: </FONT>
     </td><td>
  <input type="input" name="username" size="24" value="<%=username%>"></td>
     </tr><tr>
     <td>
  <FONT FACE="<%=FONT_FACE%>"  SIZE="<%=FONT_SIZE%>" >Unix Password: </FONT>
     </td><td>
  <input type="password" name="password" size="24" value="<%=password%>"></td>
     </tr><tr>
     <td>
  <FONT FACE="<%=FONT_FACE%>"  SIZE="<%=FONT_SIZE%>" >System Prompt: </FONT>
     </td><td>
  <input type="input" name="prompt" size="4" value="<%=prompt%>"></td>
     </tr>
     </table>  
     <%SkipBack = True%>

  <HR>

  <!--#INCLUDE FILE=NavigateTail.asp--> 

<!*************** HELP DISPLAY CONTROL ***************>

<P>
  </FONT>
  </TD></TR>
  <TR VALIGN="top"><TD COLSPAN=2>
  <FONT FACE="<%=FONT_FACE%>"  SIZE="<%=FONT_SIZE%>" >


<HR>

<%If Session("showhelp") Then%>
  <!--#INCLUDE FILE=help_sherpaserver.asp--> 
<%end if%>







 <%Case 2 'XXXXXXXXXXXXXXXXX DATABASES XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX%>
   <%
    NextStep = StepNum + 1      
   %>


  <img src="<%=IMAGE_PATH%>wiz_server.jpg" 
  WIDTH=200 HEIGHT=250
  VSPACE=3 HSPACE=5 ><BR>
  </TD><TD>
  <FONT FACE="<%=FONT_FACE%>"  SIZE="<%=FONT_SIZE%>" >


  <H3> Sherpa Databases </H3>


  You must define the Sherpa Databases which will be accessible
  through the Sherpa ObjectAdapter.  Please enter a comma delimited list.

   <P>
     <table>
     <tr>
     <td>
  <FONT FACE="<%=FONT_FACE%>" SIZE="<%=FONT_SIZE%>" >Sherpa Databases: </FONT>
     </td><td>
  <input type="input" name="databases" size="24" value="<%=SHERPA_DATABASES%>">
     </td>
  </tr></table>

   <P>

  <HR>

  <!--#INCLUDE FILE=NavigateTail.asp--> 


<!*************** HELP DISPLAY CONTROL ***************>

<P>
  </FONT>
  </TD></TR>
  <TR VALIGN="top"><TD COLSPAN=2>
  <FONT FACE="<%=FONT_FACE%>"  SIZE="<%=FONT_SIZE%>" >


<HR>

<%If Session("showhelp") Then%>
  <!--#INCLUDE FILE=help_sherpasecurity.asp--> 
<%end if%>
 <%Case 3 'XXXXXXXXXXXXXXXXX PUBLISH POLICY XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX%>
   <%
    NextStep = StepNum + 1      
   %>


  <img src="<%=IMAGE_PATH%>wiz_server.jpg" 
  WIDTH=200 HEIGHT=250
  VSPACE=3 HSPACE=5 ><BR>
  </TD><TD>
  <FONT FACE="<%=FONT_FACE%>"  SIZE="<%=FONT_SIZE%>" >


  <H3> Publishing Policy </H3>

  For each type of CAD object, please indicate if you want the object published offline or if you want
  interactive publishing.
  
   <P>
     <table>
     <tr>
     <td>
  
  <FONT FACE="<%=FONT_FACE%>" SIZE="<%=FONT_SIZE%>" >Parts are published offline: </FONT>
     </td><td>
  <input type="checkbox" name="postprt" <%if SHERPA_POST_PRT then response.write "checked"%>>
     </td>
     </tr><tr>
     <td>
     
  <FONT FACE="<%=FONT_FACE%>" SIZE="<%=FONT_SIZE%>" >Drawings are published offline: </FONT>
     </td><td>
  <input type="checkbox" name="postdrw" <%if SHERPA_POST_DRW then response.write "checked"%>>
     </td>
     </tr><tr>
     <td>
     
  <FONT FACE="<%=FONT_FACE%>" SIZE="<%=FONT_SIZE%>" >Assemblies are published offline: </FONT>
     </td><td>
  <input type="checkbox" name="postasm" <%if SHERPA_POST_ASM then response.write "checked"%>>
     </td>
     </tr><tr>
     <td>
     
     </tr></table>
   <P>
  
  <input type="hidden" name="setpost" value="1">
  <HR>

  <!--#INCLUDE FILE=NavigateTail.asp--> 


<!*************** HELP DISPLAY CONTROL ***************>

<P>
  </FONT>
  </TD></TR>
  <TR VALIGN="top"><TD COLSPAN=2>
  <FONT FACE="<%=FONT_FACE%>"  SIZE="<%=FONT_SIZE%>" >


<HR>

<%If Session("showhelp") Then%>
  <!--#INCLUDE FILE=help_sherpapublish.asp--> 
<%end if%>



 <%Case 4 'XXXXXXXXXXXXXXXXX EDR POLICY XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX%>
   <%
     NextStep = StepNum + 1      
   %>
   
  <img src="<%=IMAGE_PATH%>wiz_lock.jpg" 
  WIDTH=200 HEIGHT=250
  VSPACE=3 HSPACE=5 ><BR>
  </TD><TD>
  <FONT FACE="<%=FONT_FACE%>"  SIZE="<%=FONT_SIZE%>" >

  <H3> EDR Storage Policy </H3>

  Use these settings to customize the way information is stored in the Engineering Data Repository.
   
   <P>
     <table>
     <tr>
     <td>
    
     <!****  EDR CACHEALLFILES GOES HERE OF NEEDED ****> 
       
     <FONT FACE="<%=FONT_FACE%>" SIZE="<%=FONT_SIZE%>" >Published assemblies contain Sherpa Metadata: </FONT>
       </td><td>
       <select name="metadata">
         <OPTION value="0"<%if SHERPAInsertMetaData = 0 then response.write " SELECTED "%>>
           No         
         <OPTION value="1"<%if SHERPAInsertMetaData = 1 then response.write " SELECTED "%>>
           Yes, during publishing.         

         <OPTION value="2"<%if SHERPAInsertMetaData = 2 then response.write " SELECTED "%>>
           Yes, on demand.         
        
        </select>         
       </td>
       </tr><tr>
       <td>

    </td>
  </tr></table> 
   <P>
  
  <input type="hidden" name="setedr" value="1">
  <HR>

  <!--#INCLUDE FILE=NavigateTail.asp--> 


<!*************** HELP DISPLAY CONTROL ***************>

<P>
  </FONT>
  </TD></TR>
  <TR VALIGN="top"><TD COLSPAN=2>
  <FONT FACE="<%=FONT_FACE%>"  SIZE="<%=FONT_SIZE%>" >

<HR>

<%If Session("showhelp") Then%>
  <!--#INCLUDE FILE=help_Sherpaedr.asp--> 
<%end if%>               

 <%Case 5 'XXXXXXXXXXXXXXXXX SECURITY XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX%>
   <%
   
    UseSecurity = Ini.GetBool(SECTION_SECURITY, KEY_ENABLED)
    if not UseSecurity then
       c1 = " checked"
     else
       c2 = " checked"
     end if
     NextStep = StepNum + 1      
   %>


  <img src="<%=IMAGE_PATH%>wiz_lock.jpg" 
  WIDTH=200 HEIGHT=250
  VSPACE=3 HSPACE=5 ><BR>
  </TD><TD>
  <FONT FACE="<%=FONT_FACE%>"  SIZE="<%=FONT_SIZE%>" >


  <H3> Security Settings </H3>


  Do you want users to provide a username and password before allowing
  them access to <%=PDM_NAME%>? 

   <P>
  <table><tr valign=top>
    <td>
    <input type="radio" name="usesecurity" value="0"<%=c1%>>
    </td><td>
    <FONT FACE="<%=FONT_FACE%>"  SIZE="<%=FONT_SIZE%>" >
    Disable passwords (open access)
    </td>
  </tr><tr>
    <td>
    <input type="radio" name="usesecurity" value="1"<%=c2%>>
    </td><td>
    <FONT FACE="<%=FONT_FACE%>"  SIZE="<%=FONT_SIZE%>" >
    Require passwords for access to <%=PDM_NAME%>.
    </td>
  </tr></table>

   <P>

  <HR>

  <!--#INCLUDE FILE=NavigateTail.asp--> 


<!*************** HELP DISPLAY CONTROL ***************>

<P>
  </FONT>
  </TD></TR>
  <TR VALIGN="top"><TD COLSPAN=2>
  <FONT FACE="<%=FONT_FACE%>"  SIZE="<%=FONT_SIZE%>" >


<HR>

<%If Session("showhelp") Then%>
  <!--#INCLUDE FILE=help_sherpasecurity.asp--> 
<%end if%>


 <%Case 6 'XXXXXXXXXXXXXXXXX WINDOWS NT SECURITY XXXXXXXXXXXXXXXXXXXXXXXXXX%>
   <%UseSecurity = Ini.GetBool(SECTION_SECURITY, KEY_ENABLED)%>


   <%if not UseSecurity then%>

    <img src="<%=IMAGE_PATH%>wiz_finish.gif" 
    WIDTH=200 HEIGHT=250
    VSPACE=3 HSPACE=5 ><BR>
    </TD><TD>
    <FONT FACE="<%=FONT_FACE%>"  SIZE="<%=FONT_SIZE%>" >


    <H3> <%=PDM_NAME%> ObjectAdapter Setup Complete </H3>
   
     Any user will be allowed access to <%=PDM_NAME%>.
     <br>
     <br>
     The <%=PDM_NAME%> ObjectAdapter will use the master <%=PDM_NAME%> account. Regardless of
     the requesting user, all commands will be issued with the same priviledges as the
     master account.
     <br>
     <br> 
     The master <%=PDM_NAME%> account and the security model have now been configured. 
     <%
    SkipNext = True
    SkipHelp = True
    Session("showhelp") = False
   %>



   <%else%>
     <! ******** SECURITY ENABLED ********* >
     <%NextStep = StepNum + 1%>



    <img src="<%=IMAGE_PATH%>wiz_locknt.jpg" 
    WIDTH=200 HEIGHT=250
    VSPACE=3 HSPACE=5 ><BR>
    </TD><TD>
    <FONT FACE="<%=FONT_FACE%>"  SIZE="<%=FONT_SIZE%>" >


    <H3> Enable Windows NT Security </H3>

     All users must provide a username and password before they can access <%=PDM_NAME%>.
   To configure Windows NT you will need to do the following:
   
  <UL>
    <LI>Turn on <B>Basic Authentication</B> on the web server <P> </LI>
    <LI>Create an NT user account for each user</LI>
  </UL> 
  
  <%If Session("showhelp") Then%>
    Follow the step by step guide below to do this.
  <%else%>
    Push the <B>Show Help</B> for a step by step guide to doing this.
  <%end if%>  
   
   <%end if%>

   <HR>

  <!--#INCLUDE FILE=NavigateTail.asp--> 


<!*************** HELP DISPLAY CONTROL ***************>

<P>
  </FONT>
  </TD></TR>
  <TR VALIGN="top"><TD COLSPAN=2>
  <FONT FACE="<%=FONT_FACE%>"  SIZE="<%=FONT_SIZE%>" >


<HR>

<%If Session("showhelp") Then%>
  <!--#INCLUDE FILE=help_winntsecurity.asp--> 
<%end if%>


 <%Case 7 'XXXXXXXXXXXXXXXXXXX USER ACCESS XXXXXXXXXXXXXXXXXXXXXXXXXX%>
   <%
     UseClasses = Ini.GetBool(SECTION_SECURITY, KEY_USECLASSES)
     if UseClasses then
       c1 = " checked"
     else
       c2 = " checked"
     end if
     NextStep = StepNum + 1      
   %>


    <img src="<%=IMAGE_PATH%>wiz_user.jpg" 
    WIDTH=200 HEIGHT=250
    VSPACE=3 HSPACE=5 ><BR>
    </TD><TD>
    <FONT FACE="<%=FONT_FACE%>"  SIZE="<%=FONT_SIZE%>" >


    <H3> User Access Account Setup </H3>

  Do you want users to access <%=PDM_NAME%> via user class
  accounts or with their own private <%=PDM_NAME%> accounts.



   <P>
  <table><tr valign=top>
    <td>
    <input type="radio" name="useclasses" value="1"<%=c1%>>
    </td><td>
    <FONT FACE="<%=FONT_FACE%>"  SIZE="<%=FONT_SIZE%>" >
    Use one or more class accounts
    </td>
  </tr><tr>
    <td>
    <input type="radio" name="useclasses" value="0"<%=c2%>>
    </td><td>
    <FONT FACE="<%=FONT_FACE%>"  SIZE="<%=FONT_SIZE%>" >
    Use private accounts
    </td>
  </tr></table>

   <P>

  <HR>

  <!--#INCLUDE FILE=NavigateTail.asp--> 


<!*************** HELP DISPLAY CONTROL ***************>

<P>
  </FONT>
  </TD></TR>
  <TR VALIGN="top"><TD COLSPAN=2>
  <FONT FACE="<%=FONT_FACE%>"  SIZE="<%=FONT_SIZE%>" >


<HR>

<%If Session("showhelp") Then%>
  <!--#INCLUDE FILE=help_sherpauseraccess.asp--> 
<%end if%>



 <%Case 8 'XXXXXXXXXXXXXXXXXX CLASS INFO XXXXXXXXXXXXXXXXXXXXXXXXXXX%>
   <%UseClasses = Ini.GetBool(SECTION_SECURITY, KEY_USECLASSES)%>   
   <%if UseClasses then%>
     <%NextStep = StepNum + 1%>

    <TABLE BORDER WIDTH=200 HEIGHT=250 BGCOLOR="white" CELLPADDING=5 >
    <TR VALIGN="top" ><TD>
    <%DisplayClassConfig%>

    </TD></TR></TABLE>

    </TD><TD>
    <FONT FACE="<%=FONT_FACE%>"  SIZE="<%=FONT_SIZE%>" >


    <H3> User Class Account Setup </H3>




     Users will share one or more different <%=PDM_NAME%> accounts.  
  <P>
  You must identify one or more <%=PDM_NAME%> accounts that can be used. Each account 
     defines a "user class". For each user that will access the system, you must assign 
     them to one of the user classes.     
   <P>
   
     What would you like to do: 
   <P>
   
   
   <input type="submit" name="sac" value="Add Class" > 
   <input type="submit" name="sec" value="Edit Class" > 
   <input type="submit" name="sdc" value="Delete Class" > 
   
  <P>
  

   <%else%>

    <! *********** NO CLASSES - ALL DONE ************ >

    <img src="<%=IMAGE_PATH%>wiz_finish.gif" 
    WIDTH=200 HEIGHT=250
    VSPACE=3 HSPACE=5 ><BR>
    </TD><TD>
    <FONT FACE="<%=FONT_FACE%>"  SIZE="<%=FONT_SIZE%>" >


    <H3> <%=PDM_NAME%> ObjectAdapter Setup Complete </H3>
   

     Each user will access <%=PDM_NAME%> with their own private <%=PDM_NAME%> account.       
     <P>
   The <%=PDM_NAME%> ObjectAdapter will use the master <%=PDM_NAME%> account. For each request
     the "Set User" command will be used to assume the identity of the requesting user.
   <P>
     The master <%=PDM_NAME%> account and the security model have now been configured. 
     <%
    SkipNext = True
    SkipHelp = True
    Session("showhelp") = False
   %>
   <%end if%>

  <HR>
  <%Finish="true"%>
  <!--#INCLUDE FILE=NavigateTail.asp--> 


<!*************** HELP DISPLAY CONTROL ***************>

<P>

<HR>

<%If Session("showhelp") Then%>
  <!--#INCLUDE FILE=help_classsetup.asp--> 
<%end if%>

 
 <%Case 9 'XXXXXXXXXXXXXXXXXXXXX  CLASS ADMIN  XXXXXXXXXXXXXXXXXXXX%>

   <%
    ClassNum = 0    
    NextStep = StepNum + 1
    
  '****** ADD CLASS *******
  If Request.Form("newclass").Count and Request.Form("newclass") <> "" then
      ClassNum = Ini.GetVal(SECTION_SECURITY, KEY_NUMCLASSES)
      Admin.AddNumberedSection AgentIniFile, SECTION_SECURITY, KEY_NUMCLASSES, SECTION_CLASS, ClassNum
      Class = Request.Form("newclass")    
      ClassNum = ClassNum + 1
      ClassSection = SECTION_CLASS & ClassNum
      Admin.SetIniFileParam AgentIniFile, ClassSection, KEY_ACCOUNT, Class
    End if

    '****** MODIFY CLASS ACCOUNT *******
  If Request.Form("modclass").Count and Request.Form("modclass") <> "" then
      Class = Request.Form("modclass")    
      ClassNum = Request.Form("selclass")  
      ClassSection = SECTION_CLASS & ClassNum
      Admin.SetIniFileParam AgentIniFile, ClassSection, KEY_ACCOUNT, Class
    End if

    '****** REMOVE CLASS *******
  If Request.Form("delclass").Count then
      ClassNum = Request.Form("delclass")    
      Admin.RemoveNumberedSection AgentIniFile, SECTION_SECURITY, KEY_NUMCLASSES, SECTION_CLASS, ClassNum
    End if


    Set Ini = Server.CreateObject("olStateManager.IniFile")
    Ini.IniFile = AgentIniFile
    


  if Request.Form("changeclasses") then
      ClassCommand = Request.Form("changeclasses")
  else
    ClassCommand = changeclasses
  end if


     if ClassCommand <> 0 then %>

    <TABLE BORDER WIDTH=200 HEIGHT=250 BGCOLOR="white" CELLPADDING=5 >
    <TR VALIGN="top" ><TD>
    <%DisplayClassConfig%>

    </TD></TR></TABLE>

    </TD><TD>
    <FONT FACE="<%=FONT_FACE%>"  SIZE="<%=FONT_SIZE%>" >
  
       <%
       NextStep = StepNum
       StopStack = StepNum & " " & StepNum
       if Left(StepPath, len(StopStack)) = StopStack Then
         GetNext StepPath, " "
       end if%>
       <input type="hidden" name="changeclasses" value="<%=ClassCommand%>">


   <%end if%>   
   
   
   
   <!*********** ADMIN USER CLASSES **************>
   
   <%Select case ClassCommand%>
     <%Case 0%>
   
    <img src="<%=IMAGE_PATH%>wiz_finish.gif" 
    WIDTH=200 HEIGHT=250
    VSPACE=3 HSPACE=5 ><BR>
    </TD><TD>
    <FONT FACE="<%=FONT_FACE%>"  SIZE="<%=FONT_SIZE%>" >


    <H3> <%=PDM_NAME%> ObjectAdapter Setup Complete </H3>
   

       Each user will access <%=PDM_NAME%> through an assigned user class.       
     <P>
     The master <%=PDM_NAME%> account and the security model have now been configured. 
     <%
    SkipNext = True
    SkipHelp = True
   %>


     <%Case 1%>
   
    <H3> Add User Class </H3>
    
       To add a new User Class, enter a <%=PDM_NAME%> account name.
     Press the Apply button to create the Class. 
     <P>
     <table>
        <tr><td>
        <FONT FACE="<%=FONT_FACE%>"  SIZE="<%=FONT_SIZE%>" >
      <B><%=PDM_NAME%> Account:</B></FONT>
    </td><td>
      <input type="input" name="newclass" size=12>
    </td></tr>       
       </table>

     

     <%
      SkipNext = True
    SkipHelp = True
    SkipBack = True
    SkipFinish = True
    ShowDone = True
    ShowApply = True
     %>
    
     
     <%Case 2%>
    <H3> Edit User Class </H3>
    Select a Class to modify.  You must press the Apply button for
    changes to the <%=PDM_NAME%> account to be registered.
    If you wish to add or delete users from a Class use the Add User or Delete User Buttons.
    <P>
       <table>       
        <tr><td>
        <FONT FACE="<%=FONT_FACE%>"  SIZE="<%=FONT_SIZE%>" >
      <B>Class:</B></FONT>
    </td><td>
      <%DisplayClassList "selclass", ClassNum%>
    </td></tr>

       <tr><td>
        <FONT FACE="<%=FONT_FACE%>"  SIZE="<%=FONT_SIZE%>" >     
        <%=PDM_NAME%> Account:</FONT>
    </td><td>
      <input type="input" name="modclass" size=12></td></tr>       
       </table>

     <P>

     <input type="submit" name="susr" value="Add User">  
     <input type="submit" name="susr" value="Delete User">  


     <%
      SkipNext = True
    SkipHelp = True
    SkipBack = True
    SkipFinish = True
    ShowDone = True
    ShowApply = True
     %>
     
     <%Case 3%>
    <H3> Delete User Class </H3>
       To delete a User Class, select a <%=PDM_NAME%> account name from the list.
     Press the Delete button to remove the Class.
     <P>
     <table>
        <tr><td>
        <FONT FACE="<%=FONT_FACE%>"  SIZE="<%=FONT_SIZE%>" >
      <B><%=PDM_NAME%> Account:</B></FONT>
    </td><td>
      <%DisplayClassList "delclass", ClassNum %>
    </td></tr>       
       </table>
    
     <%
      SkipNext = True
    SkipHelp = True
    SkipBack = True
    SkipFinish = True
    ShowDone = True
    ShowDelete = True
     %>
     
     <%Case 4%>       
    <H3> Add User to Class </H3>
       <table>
       <tr><td>Add user:</td><td><input type="input" name="newuser" size=12></td></tr>       
       <tr><td>To this class:</td><td><%DisplayClassList "newuserclass", ClassNum%></td></tr>
       </table>
     <%Case 5%>
    <H3> Remove User from Class </H3>
       <table>
       <tr><td>Remove user:</td><td><input type="input" name="deluser" size=12></td></tr>       
       <tr><td>From this class:</td><td><%DisplayClassList "deluserclass", ClassNum%></td></tr>
       </table>
     <%Case Else%>
   <%End Select%>

      <HR>

  <!--#INCLUDE FILE=NavigateTail.asp--> 

 <%Case 10 'XXXXXXXXXXXXXXXXXXXXX  USERS ADMIN  XXXXXXXXXXXXXXXXXXXX%>


<%

ClassNum = Request.Form("selclass")

    '****** ADD USER *******
  If Request.Form("newuser").Count then
      User = Request.Form("newuser")
      ClassNum = Request.Form("newuserclass")
      ClassSection = SECTION_CLASS & ClassNum
      Users = Trim(Ini.GetStr(ClassSection, KEY_USERS))
      NewUsers = ""
      While Users <> ""
        OneUser = GetNext(Users, ",")
        if OneUser <> User then
          NewUsers = NewUsers & ", " & OneUser
        end if
      Wend
      NewUsers = NewUsers & ", " & User
      Users = mid(NewUsers, 2)           
      Admin.SetIniFileParam AgentIniFile, ClassSection, KEY_USERS, Users
    end if

  '****** REMOVE USER *******
    If Request.Form("deluser").Count then
      User = Request.Form("deluser")
      ClassNum = Request.Form("deluserclass")
      ClassSection = SECTION_CLASS & ClassNum
      Users = Trim(Ini.GetStr(ClassSection, KEY_USERS))
      NewUsers = ""
      While Users <> ""
        OneUser = GetNext(Users, ",")
        if OneUser <> User then
          NewUsers = NewUsers & ", " & OneUser
        end if
      Wend
      Users = mid(NewUsers, 2)
      Admin.SetIniFileParam AgentIniFile, ClassSection, KEY_USERS, Users
    end if

    Set Ini = Server.CreateObject("olStateManager.IniFile")
    Ini.IniFile = AgentIniFile
    
%>



    <TABLE BORDER WIDTH=200 HEIGHT=250 BGCOLOR="white" CELLPADDING=5 >
    <TR VALIGN="top" ><TD>
    <%DisplayClassConfig%>

    </TD></TR></TABLE>

    </TD><TD>
    <FONT FACE="<%=FONT_FACE%>"  SIZE="<%=FONT_SIZE%>" >
           <input type="hidden" name="changeclasses" value="<%=changeclasses%>">
         <input type="hidden" name="editcmd" value="<%=editcmd%>">
  
       <%
       NextStep = StepNum
       StopStack = StepNum & " " & StepNum
       if Left(StepPath, len(StopStack)) = StopStack Then
         GetNext StepPath, " "
       end if%>


  <%Select case editcmd%>
     <%Case 1%>

    <H3> Add User to Class </H3>

    Select the Class and type in the name of the user to add
    (or a comma delimited list of users).
    <P>
    Press the <B>Apply</B> button to add the user. <BR>
    Press Done to return the the previous screen, making no more changes.
    <P>

       <table>
        <tr><td>
        <FONT FACE="<%=FONT_FACE%>"  SIZE="<%=FONT_SIZE%>" >
      <B>Class:</B></FONT>
    </td><td>
      <%DisplayClassList "newuserclass", ClassNum%>
    </td></tr>

       <tr><td>
        <FONT FACE="<%=FONT_FACE%>"  SIZE="<%=FONT_SIZE%>" >     
        Username: </FONT>
    </td><td>
      <input type="input" name="newuser" size=12>
    </td></tr>       
       </table>

     <%
      SkipNext = True
    SkipBack = True
    SkipHelp = True
    SkipFinish = True
    ShowDone = True
    ShowApply = True
     %>

   <%Case 2%>
    <H3> Remove User from Class </H3>

    Select the Class and type in the name of the user to remove (one at a time).
    <P>
    Press the <B>Delete</B> button to remove the user. <BR>
    Press Done to return the the previous screen, making no more changes.
    <P>

       <table>
       <table>
        <tr><td>
        <FONT FACE="<%=FONT_FACE%>"  SIZE="<%=FONT_SIZE%>" >
      <B>Class:</B></FONT>
    </td><td>
      <%DisplayClassList "deluserclass", ClassNum%>
    </td></tr>

       <tr><td>
        <FONT FACE="<%=FONT_FACE%>"  SIZE="<%=FONT_SIZE%>" >     
        Username:</FONT>
    </td><td>
      <input type="input" name="deluser" size=12>
    </td></tr>       
       </table>

     <%
      SkipNext = True
    SkipBack = True
    SkipHelp = True
    SkipFinish = True
    ShowDone = True
    ShowDelete = True
     %>

   <%Case else%>
   xxx
    <%End Select%>

     


      <HR>

  <!--#INCLUDE FILE=NavigateTail.asp--> 


 
 <%Case Else%>
 <h1> I don't know </H1>

<%End Select%>


  </FONT>
  </TD></TR>
  </TABLE>

  </FONT>
  </TD></TR>
</TABLE>

</CENTER>



</form>  

</FONT>
</BODY>
</HTML>