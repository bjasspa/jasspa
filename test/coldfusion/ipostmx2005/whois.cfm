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

<cfquery name="whois" datasource="#Application.Database#">
  SELECT MEMBERS.username, MEMBERS.joindate, MEMBERS.posts, MEMBERS.showemail, MEMBERS.email
  FROM MEMBERS
  WHERE username= <cfqueryparam cfsqltype="sql_varchar" value="#url.member#">
</cfquery>

<!DOCTYPE html
PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
<head>
  <link rel="StyleSheet" href="includes/ipostmx.css">
  <title>Member Lookup</title>
</head>
<body leftmargin="0">
<br />
<table width="100%" cellspacing="0" cellpadding="0">
  <tr>
    <td align="center" class="tableColor"  background="images/menu.jpg">
      <p><b>Member Info : <cfoutput>#URL.member#</cfoutput></b></p>
    </td>
  </tr>
  <tr>
    <td class="PreforumTopic">
      <cfif whois.recordcount GT 0>
        <cfoutput>
          Member Name: #whois.username#
          <br />
          Member Since: #DateFormat(whois.joindate, "mmmm dd, yyyy")#<br />
          Number of Posts: #whois.posts#
          <cfif whois.showemail>
            <br />Email: <a href="mailto:#whois.email#" class="gear">#whois.email#</a>
          </cfif>
        </cfoutput>
      <cfelse>
        <p><b>Sorry, no members were found by that username.</b>
      </cfif>
    </td>
  </tr>
  <tr>
    <td align="center">
      <p><a href="javascript: window.close();" class="gear">Close Window</a></p>
    </td>
  </tr>
</table>
</body>
</html>