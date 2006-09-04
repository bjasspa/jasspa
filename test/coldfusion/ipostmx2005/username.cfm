<!---
// iPostMX 2005 - Coldfusion Community and Bulletin Board
// Copyright (C) 2004 - 2005  Cole Barksdale Applications 
// http://www.colebarksdale.com http://www.ipostmx.com
//
// This program is free software; you can redistribute it and/or modify 
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
--->

<cfparam name="form.email" default="">
<cfparam name="error" default="false">

<cfif isDefined("bSubmit")>
  <!---QUERY FOR USER INFORMATION--->
  <cfquery name="retrieve" datasource="#Application.Database#">
    SELECT email,password
    FROM MEMBERS
    WHERE username = '#form.uname#' AND password = '#form.pw#'
  </cfquery>
  
  <!---IF NO RECORDS ARE RETURNED, USER DOES NOT EXIST, OTHERWISE EMAIL ACCOUNT INFORMATION--->
  <cfif retrieve.recordcount GT 0>
    <cfset error = false>
  <cfelse>
    <cfset error = true>
  </cfif>
</cfif>

<html>
  <head>
    <title>iPostMX: Retrieve Account Info</title>
    <meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
    <link rel="stylesheet" href="includes/ipost.css">
    <script language="JavaScript1.2" src="includes/js/global.js"></script>
  </head>
  <body <cfif NOT isDefined("form.bSubmit")> onLoad="document.pw.uname.focus();"</cfif>>
  
  <cfif error OR NOT isDefined("bSubmit")>
    
    
    <p>Please enter your nickname/alias and password in the form below and we will retrieve you account information.</p>
    
    <cfif error>
      <p><b>ERROR:</b> No user with that account information exists in our
      system. <br />Please be sure that the information that you have entered
      is spelled correctly. <br />If you would like to register for a new
      forum account you can do so 
      <a href="javascript:window.opener.location.href='account.cfm';window.self.close();">here</a>.
    </cfif>
    </p>
    
    <form action="<cfoutput>#myurl#</cfoutput>" method="post" onSubmit="return submitForgotEmail( this );" name="pw">
    <p>Nickname/Alias: 
    <input name="uname" type="text" 
     value="<cfif isDefined("form.uname")><cfoutput>#uname#</cfoutput></cfif>" 
     style="width:240px;background-color:#eeeeee;border:1px solid #200057;"/> <span style="color:#200057;">*</span><br /><br />
    &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Password: <input name="pw" type="password" value="" style="width:240px;background-color:#eeeeee;border:1px solid 
     #200057;"/> <span style="color:#200057;">*</span><br /><br />
    <br />
    <div align="center">
    <input type="button" name="bCancel" value="cancel" onClick="window.self.close();">&nbsp;&nbsp;
    <input type="submit" value="submit" name="bSubmit">
    </div>
    </form>
    
  <cfelse>
    
    <cfoutput>
      <p>The Email Address you have in our system is: 
      <strong style="color:red">#retrieve.email#</strong>
    </cfoutput>
    <br /><br /><br />
    <a href="#" onClick="window.self.close();" class="gear">CLOSE WINDOW</a>
    <form action="post" name="pw" style="padding:0px;margin:0px;">
    <input type="hidden" name="uname">
    </form>
    
  </cfif>
    
  </body>
</html>