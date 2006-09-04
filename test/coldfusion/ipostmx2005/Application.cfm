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

<cfapplication name="IPOSTMX" clientmanagement="Yes" sessionmanagement="Yes" setclientcookies="yes" sessiontimeout="#CreateTimeSpan(0,0,45,0)#">
<cfset Application.Database="ipostmx">
<cfif isDefined("form.logout")>
  <cflogout>
  <cflocation addtoken="no" url="index.cfm">
</cfif>
<cfif NOT isDefined("cookie.cfauthorization_forum")
   OR (isDefined("cookie.cfauthorization_forum") 
   AND cookie.cfauthorization_forum EQ "") OR 
   NOT isDefined("session.user")>
  <cfset x = StructDelete(session, "user")>
  <cfset logged = false>
<cfelse>
  <cfset logged = true>
</cfif>
<cfset myUrl = "http://" & "#CGI.SERVER_NAME#" & "#CGI.SCRIPT_NAME#">
<cfif CGI.QUERY_STRING IS NOT "">
  <cfset  myUrl = myUrl & "?#CGI.QUERY_STRING#">
</cfif>
<cfif NOT isDefined("session.a_views")>
  <cfset session.a_views = ArrayNew(1)>
  <cfset session.a_views[1] = 0>
</cfif>
<cfset mapping = "http://" &"#cgi.server_name#" & "/">
<!---USER IS GOOD, SET VARIABLES--->
<cfif Len(getAuthUser()) AND NOT isDefined("session.user")>
  <cfquery name="loginQuery" datasource="#Application.Database#">
    SELECT *
    FROM MEMBERS
    WHERE email = '#getAuthUser()#'
  </cfquery>
  <cfset session.user = StructNew()>
  <cfset session.user.memberid = loginQuery.memberid>
  <cfset session.user.username = loginQuery.username>
  <cfset session.user.password = loginQuery.password>
  <cfset session.user.firstname = loginquery.firstname>
  <cfset session.user.lastname = loginquery.lastname>
  <cfset session.user.address = loginquery.address>
  <cfset session.user.address2 = loginquery.address2>
  <cfset session.user.city = loginquery.city>
  <cfset session.user.state = loginquery.state>
  <cfset session.user.zip = loginquery.zip>
  <cfset session.user.age = loginquery.age>
  <cfset session.user.country = loginquery.country>
  <cfset session.user.joindate = loginquery.joindate>
  <cfset session.user.posts = loginquery.posts>
  <cfset session.user.signature = loginquery.signature>
  <cfset session.user.lastvisitdate = loginquery.lastlogin>
  <cfset session.user.viewemail = loginquery.showemail>
  <cfset session.user.email = loginquery.email>
  <cfset session.user.updates = loginquery.noreceiveemail>
  <cfset session.user.profilelastupdated = loginQuery.profilelastupdated>
  <cfif loginQuery.accesslevel NEQ 5>
    <cfset session.user.admin = 0>
  <cfelse>
    <cfset session.user.admin = 1>
  </cfif>
  <cfif isDefined("form.rememberme") AND form.rememberme>
    <cfset session.user.rememberme = true>
  <cfelse>
    <cfset session.user.rememberme = false>
  </cfif>
</cfif>
<cfif Len(getAuthUser())>
  <cfset logged = true>
<cfelseif isDefined("session.user")>
  <cfset logged = true>
</cfif>