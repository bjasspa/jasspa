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
  <cfquery name="retrieve" datasource="#DSN#">
    SELECT email,password
    FROM MEMBERS
    WHERE email = '#form.email#'
  </cfquery>
  
  <!---IF NO RECORDS ARE RETURNED, USER DOES NOT EXIST, OTHERWISE EMAIL ACCOUNT INFORMATION--->
  <cfif retrieve.recordcount GT 0>
    <cfmail subject="iPostMX Username/Password" from="no-reply@ipostmx.com" to="#retrieve.email#" timeout="900">
      Your requested account information.
      
      Username: #retrieve.email#
      Password: #retrieve.password#
    </cfmail>
    <cfset error = false>
  <cfelse>
    <cfset error = true>
  </cfif>
</cfif>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
<head>
  <title>iPostMX: Retrieve Password</title>
  <meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
  <link rel="stylesheet" href="includes/ipost.css">
  <script language="JavaScript1.2" src="includes/js/global.js">
  </script>
</head>
<body>

<cfif error OR NOT isDefined("bSubmit")>
  
  
  <p>Please enter your email address in the form below and your account information will be emailed to you.</p>
  
  <cfif error>
    <p>ERROR:</strong> No user with that email address exists in system. If email address is typed correctly, you may create a new forum account with it <a 
     href="javascript:window.opener.location.href='account.cfm';window.self.close();">here</a>.</p>
  </cfif>
  
  <form action="<cfoutput>#myurl#</cfoutput>" method="post" onSubmit="return submitForgotPwd(this);" name="pw">
  <p>EMAIL ADDRESS: <input name="email" type="text" value="<cfoutput>#email#</cfoutput>" style="width:240px;background-color:#eeeeee;border:1px solid #200057;"/> <span 
     style="color:#dd0000;">*</span><br /><br /><br />
  <div align="center"><input type="button" name="bCancel" value="cancel" onClick="window.self.close();">&nbsp;&nbsp;<input type="submit" value="submit" 
   name="bSubmit"></div>
  </form>
  
<cfelse>
  
  Your account information has succesffully been emailed.<br /><br />
  <a href="#" onClick="window.self.close();" class="gear">CLOSE WINDOW</a>
  
  
</cfif>

</p>
</body>
</html>
