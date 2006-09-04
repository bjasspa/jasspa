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

<cfinclude template="includes/bbcode.cfm">
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
<head>
  <title>Preview Post</title>
  <meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
  <link rel="stylesheet" href="includes/ipost.css">
</head>

<body>
<div class="heading"><b>Message:</b></div><br /><br />
<table width="90%" cellpadding="0" cellspacing="0" border="0" align="center">
  <tr>
    <td width="100%" class="forumTopic">
      <cfset enterbreak = Replace('#form.message#',chr(13),"<br />","ALL")>
      <cfoutput>
        #enterbreak#
      </cfoutput>
    </td>
  </tr>
</table>
<br />
<div align="center"><input type="button" value="Close Window" onClick="window.close();"></div>
</body>
</html>

