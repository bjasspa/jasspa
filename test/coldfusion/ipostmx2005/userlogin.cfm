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

<cflogin applicationtoken="ipostmx" idletimeout="1800">
  <cfif NOT IsDefined( "cflogin" )>
    <cfinclude template="login.cfm">
    <cfabort>
  <cfelse>
    <cfif cflogin.name IS "" OR cflogin.password IS "">
      <cfset error = "You must enter text in both the Username and Password fields.">
      <cfinclude template="login.cfm">
      <cfabort>
    <cfelse>
      <cfquery name="loginQuery" datasource="#Application.Database#">
        SELECT email, password
        FROM MEMBERS
        WHERE email = '#cflogin.name#'
        AND password = '#cflogin.password#'
      </cfquery>
      <cfif loginQuery.email NEQ "">
        <cfloginuser name="#cflogin.name#" password="#cflogin.password#" roles="user">
        <cfelse>
          <cfset error = "Invalid Username and or Password. Please try again.">
          <cfinclude template="login.cfm">
          <cfabort>
        </cfif>
      </cfif>
    </cfif>
  </cfif>
</cflogin>

<!---USER IS GOOD, SET VARIABLES--->
<cfif Len(getAuthUser())>
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
    <cfset session.user.rememberme = true />
    <cfcookie name="autolog" value="1" expires="NEVER" />
    <cfcookie name="autolog_name" value="#session.user.email#" expires="NEVER" />
  <cfelse>
    <cfset session.user.rememberme = false />
  </cfif>

  <cflocation addtoken="no" url="#returnurl#">

</cfif>
