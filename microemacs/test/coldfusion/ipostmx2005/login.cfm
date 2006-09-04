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

<cfif isDefined("COOKIE.ipostmx") AND ListGetAt(Cookie.ipostmx, 1) NEQ 0>
  <cfset uname = ListGetAt(cookie.ipostmx, 1, ",")>
  <cfset pwd = ListGetAt(cookie.ipostmx, 2, ",")>
<cfelse>
  <cfset uname = "">
  <cfset pwd = "">
</cfif>

<script language="JavaScript">
function subForm(form) {
  if (form.j_username.value == null || form.j_username.value == '') {
    alert("Username required.");
    return false;
  } else {
    if (form.j_password.value == null || form.j_password.value == '') {
      alert("Password required.");
      return false;
    } else {
      return true;
    }
  }
}

function forgotPwd() {
  window.open('password.cfm', 'pwwin', 'width=390, height=200, scrollbar=0, menubar=0');
}


function forgotEmail() {
  window.open('username.cfm', 'unwin', 'width=400, height=250, scrollbar=0, menubar=0');
}
</script>
<cfinclude template="includes/header.cfm">
<cfinclude template="ipostmx_header.cfm">
<table width="100%">
  <tr>
    <td>
      <form name="loginForm" method="post" action="userlogin.cfm" onSubmit="return subForm(this);">
      <table border="0" align="center">
        <tr><td>&nbsp;</td></tr>
        <tr>
          <td colspan="2">
            <p>If you are a returning forum member, please sign in. If you do not have a forum member account you can create one
            <a href="account.cfm?ReturnURL=<cfoutput>#returnURL#</cfoutput>" class="gear"><strong>here</strong></a><br /><br />
          </td>
        </tr>
        <tr>
          <td nowrap width="25%" align="right"><p>E-MAIL ADDRESS</p></td>
          <td align="left"><input name="j_username" type="text" value="<cfif 
                isdefined('cookie.autolog_name')><cfoutput>#cookie.autolog_name#</cfoutput></cfif><cfoutput>#uname#</cfoutput>" 
             style="width:240px;background-color:#eeeeee;border:1px solid #200057;"/> <span style="color:#200057;">*</span></td>
        </tr>
        
        <tr><td colspan="2">&nbsp;</td></tr>
        <tr>
          <td nowrap width="25%" align="right"><p>PASSWORD</p></td>
          <td align="left"><p><input name="j_password" type="password" style="width:240px;background-color:#eeeeee;border:1px solid #200057;" 
             value="<cfoutput>#pwd#</cfoutput>"/> <span style="color:#200057;">*</span>
            <input name="rememberMe" type="checkbox" value="true" <cfif isDefined("cookie.autolog") AND ListGetAt(cookie.autolog, 1) NEQ 0>checked</cfif>/> 
            Remember me</p></td>
        </tr>
        
        <tr>
          <td>&nbsp;</td>
          <td align="left">
            <p><a href="javascript:forgotPwd();" class="gear">Did you forget your password?</a></p>
          </td>
        </tr>
        <tr>
          <td>&nbsp;</td>
          <td align="left">
            <p><a href="javascript:forgotEmail();" class="gear">Forget what email address you used to register?</a></p>
          </td>
        </tr>
        
        <cfif isDefined("error")>
          <tr><td colspan="2">&nbsp;</td></tr>
          <tr>
            
            <td style="color:#dd0000;" colspan=2 align="center"><cfoutput><p><strong>**#error#**</strong></p></cfoutput></td>
          </tr>
        </cfif>
        
        <tr><td colspan="2">&nbsp;</td></tr>
        
        <tr>
          <td>&nbsp;</td>
          <td align="center">
            <input type="button" name="cancel" value="Cancel" onClick="location.href='<cfoutput>#returnURL#</cfoutput>'"/>&nbsp;&nbsp;&nbsp;
            <input type="submit" name="submit" value="Log In"/></td></tr>
        
        <tr><td colspan="2">&nbsp;</td></tr>
        
      </table>
      <input name="returnURL" type="hidden" value="<cfoutput><cfif isDefined("url.returnurl")>#url.returnurl#<cfelse>index.cfm</cfif></cfoutput>" />
      </form>
      
      
    </td></tr>
</table>

<cfinclude template="includes/footer.cfm">
