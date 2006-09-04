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

<cfinclude template="includes/header.cfm">
<cfinclude template="ipostmx_header.cfm">

<cfif isDefined("url.id") AND isNumeric(url.id)>
<cfelse>
  
  <table cellpadding="4" cellspacing="1" border="0" width="100%" height="220">
    <tr>
      <td width="100%" colspan="2" valign="top">
        
        <p><b>Thanks for joining ColdForums!</b><br /><br />
        Please click <a href="userlogin.cfm?RETURNURL=<cfoutput>#URLEncodedFormat(index.cfm)#</cfoutput>" class="gear"><b>here</b></a> to login now!
        </p>
      </td>
    </tr>
    
  </table>
  
</cfif>
<br />
<br />
<cfinclude template="includes/footer.cfm">
