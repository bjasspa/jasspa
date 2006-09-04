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

<script language="javascript">


function makeBold() {
  var txt = window.prompt("Enter the text you wish to be bold.", "Enter Text Here");
  if (txt != null) {
    document.postForm.message.value += "<b>" + txt + "</b>";
  }
  document.postForm.message.focus();
}

function makeItalic() {
  var txt = window.prompt("Enter the text you wish to be in italics.", "Enter Text Here");
  if (txt != null) {
    document.postForm.message.value += "<i>" + txt + "</i>";
  }
  document.postForm.message.focus();
}

function makeUnderline() {
  var txt = window.prompt("Enter the text you wish to be underlined.", "Enter Text Here");
  if (txt != null) {
    document.postForm.message.value += "<u>" + txt + "</u>";
  }
  document.postForm.message.focus();
}

function emotion(icon) {
  document.postForm.message.value += " ::" + icon + ":: ";
  document.postForm.message.focus();
}

function makeHTTP() {
  myHTTP = window.prompt("Please enter your link.", "http://www.ipostmx.com");
  if ( myHTTP != null) {
    document.postForm.message.value += "<a href='" + myHTTP + "'>" + myHTTP + "</a>";
  }
  document.postForm.message.focus();
}

function showPreview() {
  origAction = document.postForm.action;
  origTarget = document.postForm.target;
  document.postForm.action = 'previewpost.cfm';
  document.postForm.target = 'PreviewWin';
  window.open('about:blank', 'PreviewWin', 'height=300, width=450, toolbar=0, scrollable=1, resizeable=0, scrollbars=1, titlebar=0,status=0');
  document.postForm.submit();
  document.postForm.action = origAction;
  document.postForm.target = origTarget;
}

function validMessage(form) {
  if(form.topictitle != null &&
     (form.topictitle.value == "" || form.topictitle.value == null)) {
  alert("Please enter a topic title.");
  return false;
}
if(form.message.value == "" || form.message.value == null) {
  alert("Cannot submit empty message!");
  return false;
}
return true;
}


function printIt( url )
{
  window.open( url, 'printwindow', 'width=700, height=450, menubar=0, status=0, scrollbars=1');
}


function whois( member ) {
  window.open( 'whois.cfm?member='+member, 'whois', 'width=350,height=200, menubar=0, status=0, scrollbars=1');
}

</script>

<table cellpadding="4" cellspacing="1" border="0" width="100%" align="center">
  <tr>
    <td width="100%" align="right" nowrap>
      <!---BEGIN FORUM NAVIGATION--->
      <form name="logout" style="padding:0px;margin:0px;" method="post">
      <p>
      <a href="active.cfm" class="gear"><b>active discussions</b></a>&nbsp;|&nbsp; 
      <!---<cfif NOT FindNoCase("search.cfm", CGI.SCRIPT_NAME)>--->
      <!---<a href="search.cfm" class="gear"><b>search</b></a>--->
      <!---<cfelse>--->
      <!---<a href="index.cfm" class="gear"><b>forum home</b></a>--->
      <!---</cfif>--->
      <!---&nbsp;|&nbsp;--->
      
      <cfif ((FindNoCase("forums.cfm", CGI.SCRIPT_NAME)) 
         OR (FindNoCase("topics.cfm", CGI.SCRIPT_NAME))) 
         AND NOT (isDefined("url.forum") 
         AND NOT ((isDefined("session.user") AND session.user.admin EQ 1) OR (url.forum NEQ 1))) 
         AND isDefined("forum")>
        <a href="messagepost.cfm?action=topic&forum=<cfoutput>#forum#</cfoutput>" class="gear"><b>new topic</b></a>
        &nbsp;|&nbsp;
      </cfif>
      <a href="account.cfm?RETURNURL=<cfoutput>#URLEncodedFormat(myurl)#</cfoutput>" class="gear">
      <b>
      <cfif logged>
        modify account
      <cfelse>
        create account
      </cfif></b></a>
      &nbsp;|&nbsp;
      <cfif logged>
        <input type="submit" name="logout" value="logout" 
         style="padding:0px;margin:0px;border:0px;background-color:#FFFFFF;cursor:hand;color:#1476A7;font-family: Tahoma,Arial;font-size: 8pt;font-weight:900;">
      <cfelse>
        <a href="userlogin.cfm?RETURNURL=<cfoutput>#URLEncodedFormat(myurl)#</cfoutput>" class="gear"><b>login</b></a>
      </cfif>
      </form>
      <!---END FORUM NAVIGATION--->
    </td>
  </tr>
</table>