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

<!DOCTYPE html
PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
<head>
  <link rel="StyleSheet" href="includes/ipostmx.css" />
  <title>iPostMX 2005 Database Installer</title>
</head>
<body>

<cfif isdefined("form.installdb")>
  
  <p>Please wait as the database is installed.</p>
  <cftry>
    <cfinclude template="ipostmx2005.cfm" />
    <cfcatch>
      <cfoutput>
        <p>#cfcatch.message#<br />#cfcatch.detail#</p>
      </cfoutput>
    </cfcatch>		
  </cftry>
  
  <script language="JavaScript">
    location.href='index.cfm';
  </script>	
  
<cfelse>
  
  <div align="center">
  <img src="images/ipostmx2005_logo.gif" border="0">
  <br />
  <form action="installdb.cfm" method="post">
  <input type="submit" value="Install Database" name="installdb" />
  </form>
  </div>
  
</cfif>

</body>
</html>