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

<cfparam name="emailDupe" default="0">
<cfparam name="aliasDupe" default="0">
<cfparam name="f_signature" default="">

<cfinclude template="includes/header.cfm">
<cfinclude template="ipostmx_header.cfm">

<table width="100%">
	<tr>
		<td>

			<table cellpadding='0' cellspacing='0' border='0' width='100%' bgcolor='#ffffff' align='center'>
				<tr>
					<td>
						<table cellpadding="4" cellspacing="1" border="0" width="100%">
							<tr>

<!---FORM HAS BEEN SUBMITTED--->
<cfif isDefined("form.bSubmit")>

	<!---CHECK FOR DUPLICATE EMAIL ADDRESSES--->
	<cfif (NOT logged) OR (logged AND session.user.email NEQ form.email)>
		<cfquery name="emailDupe" datasource="#Application.Database#">
		SELECT Email
		FROM MEMBERS
		WHERE Email = '#form.email#'
		</cfquery>

		<cfif emailDupe.recordCount EQ 0>
			<cfset emailDupe = false>
		<cfelse>
			<cfset emailDupe = true>
		</cfif>
	</cfif>

	<!---CHECK FOR DUPLICATE ALIAS--->
	<cfif (NOT logged) OR (logged AND session.user.username NEQ form.alias)>
		<cfquery name="aliasDupe" datasource="#Application.Database#">
		SELECT username
		FROM MEMBERS
		WHERE username = '#form.alias#'
		</cfquery>
		<cfif aliasDupe.recordCount EQ 0>
			<cfset aliasDupe = false>
		<cfelse>
			<cfset aliasDupe = true>
		</cfif>
	</cfif>

	<!---LOGGED IN USER HAS UPDATED ACCOUNT INFORMATION AND DID NOT DUPLICATE UNIQUE INFORMATION--->
	<cfif logged AND NOT emailDupe AND NOT aliasDupe>
	<!---UPDATE INFORMATION IN DATABASE--->
	<!---TRIM PASSWORD BECAUSE OF PADDING ADDED--->
	<cfif isDefined("form.updates")>
		<cfset noEmail = 1>
	<cfelse>
		<cfset noEmail = 0>
	</cfif>
	<cfif isDefined("form.viewEmail") AND form.viewEmail EQ "Yes">
		<cfset showEmail = 1>
	<cfelse>
		<cfset showEmail = 0>
	</cfif>
	<cfquery name="updateInfo" datasource="#Application.Database#">
	UPDATE MEMBERS
	SET FirstName = '#form.firstname#',
		LastName = '#form.lastname#',
		Address = '#form.address#',
		Address2 = '#form.address2#',
		City = '#form.city#',
		State = '#form.state#',
		Zip = '#form.zip#',
		Age = '#form.age#',
		Country = '#form.country#',
		UserName = '#form.alias#',
		Password = '#Trim(form.password_a)#',
		Email = '#form.email#',
		NoReceiveEmail = #noEmail#,
		showEmail = #showEmail#,
		signature = '#form.signature#',
		profilelastupdated = NOW()
	WHERE memberid = #session.user.memberid#
	</cfquery>

	<cfquery name="updateMessages" datasource="#Application.Database#">
	UPDATE MESSAGES
	SET username = '#Form.alias#'
	WHERE memberid = #session.user.memberid#
	</cfquery>
	
	<cfquery name="updateTopics" datasource="#Application.Database#">
	UPDATE TOPICS
	SET membername = '#Form.alias#'
	WHERE memberid = #session.user.memberid#
	</cfquery>

	<!---UPDATE SESSION VARIABLES--->
	<cfset session.user.username = form.alias>
	<cfset session.user.password = form.password_a>
	<cfset session.user.firstname = form.firstname>
	<cfset session.user.lastname = form.lastname>
	<cfset session.user.address = form.address>
	<cfset session.user.address2 = form.address2>
	<cfset session.user.city = form.city>
	<cfset session.user.state = form.state>
	<cfset session.user.zip = form.zip>
	<cfset session.user.age = form.age>
	<cfset session.user.country = form.country>
	<cfset session.user.signature = form.signature>
	<cfset session.user.viewemail = showEmail>
	<cfset session.user.email = form.email>
	<cfset session.user.updates = noEmail>
	<cfset session.user.profilelastupdated = now()>


	<!---DISPLAY CONFIRMATION UPDATE SCREEN--->
	<td>
		<h3>Account Center</h3>
		<p>Your account information has been updated.
		<br />
		<a href="/forum/" class="gear">iPostMX Home</a></p>
		<br />
		<br />
	</td>
	</tr>
	</table>
	</td>
	</tr>



	<!---NEW ACCOUNT TO BE CREATED UNIQUE INFORMATION HAS BEEN SUBMITTED--->
	<cfelseif NOT logged AND NOT emailDupe AND NOT aliasDupe>
		<cfif isDefineD("form.updates")>
			<cfset noEmail = 1>
		<cfelse>
			<cfset noEmail = 0>
		</cfif>
		<cfif isDefined("form.viewEmail") AND form.viewEmail EQ "Yes">
			<cfset showEmail = 1>
		<cfelse>
			<cfset showEmail = 0>
		</cfif>
		<!---INSERT DATA INTO DATABASE--->
		<cfquery name="insertMember" datasource="#Application.Database#">
		INSERT INTO MEMBERS(FirstName, LastName, Address, Address2, City, State, Zip, Age, Country, username, password, email, noreceiveemail, showemail, profilelastupdated, 
joindate, accesslevel, posts)
		VALUES ('#form.firstname#', '#form.lastname#', '#form.address#', '#form.address2#', '#form.city#', '#form.state#', '#form.zip#', '#form.age#', '#form.country#', 
'#form.alias#','#form.password_a#', '#form.email#', #noEmail#, #showEmail#, NOW(), NOW(), 1, 0)
		</cfquery>

		<!--- SEND EMAIL TO THE NEW USER WITH LOGIN INFO --->
		<cfmail to="#Form.email#" from="donotreply@ipostmx.com" subject="Your new iPostMX account">
		***********************DO NOT REPLY TO THIS EMAIL***********************
		Hello #Form.firstname#(#Form.alias#),

		Your new account with iPostMX 2005 is active. Below is your login information.

		username/login: #Form.email#
		password: #Form.password_a#

		Please keep this email in a safe place for future use. It is also recommended that you do not share this information with anyone.

		iPostMX 2005 Crew
		http://www.ipostmx.com
		</cfmail>

		<!---REDIRECT TO ACTIVATE SCREEN WITH NEW USER FLAG SET, THIS REDIRECT ALSO PREVENTS A USER FROM INSERTING THEMSELF UPON REFRESHES--->
		<cflocation addtoken="no" url="activate.cfm">


	<!---WE HAVE A DUPLICATE EMAIL--->
	<cfelseif emailDupe OR aliasDupe>
		<cfif isDefined("form.updates")>
			<cfset noEmail = 1>
		<cfelse>
			<cfset noEmail = 0>
		</cfif>
		<cfif isDefined("form.viewEmail") AND form.viewEmail EQ "Yes">
			<cfset showEmail = 1>
		<cfelse>
			<cfset showEmail = 0>
		</cfif>
		<!---DUPLICATE EMAIL, SET FORM VALUES TO PREVIOUS VALUES--->
			<cfset f_firstName = form.firstName>
			<cfset f_lastName = form.lastName>
			<cfset f_address = form.address>
			<cfset f_address2 = form.address2>
			<cfset f_city = form.city>
			<cfset f_state = form.state>
			<cfset f_zip = form.zip>
			<cfset f_country = form.country>
			<cfset f_age = form.age>
			<cfset f_alias = form.alias>
			<cfset f_email = form.email>
			<cfset f_password = form.password_a>
			<cfset f_viewEmail = showEmail>
			<cfset f_updates = noEmail>
				<cfoutput>
				<table border="0" width="90%" align="center">
					<tr><td colspan="2" align="left"><br /><p>
					<cfif logged><strong>Your account was last updated on <cfif session.user.profilelastupdated EQ "">#dateFormat(session.user.joindate, "mmm dd, 
yyyy")#<cfelse>#dateFormat(session.user.profilelastupdated, "mmm dd, yyyy")#</cfif>.</strong></p></cfif>
					</td></tr>

					<cfif emailDupe>
					<tr><td colspan="2" style="color:##dd0000;"><p><strong>ERROR:</strong> Sorry, this email address (#form.email#) has already been used to register a 
ColdForum account. If this is your email address, please click here, and your account information will be sent to #form.email#, otherwise please try a 
different email.</p></td></tr>
					</cfif>

					<cfif aliasDupe>
					<tr><td colspan="2" style="color:##dd0000;"><p><strong>ERROR:</strong> Sorry, the alias you entered (#form.alias#) is already in use. Please select a 
different alias to continue.</p></td></tr>
					</cfif>

				</cfoutput>
					<cfinclude template="includes/ipostreg.cfm">
				</table>

	</cfif>

	<!---FORM HAS NOT BEEN SUBMITTED--->
	<cfelse>



		<tr>
		<cfif logged>
			<!---preset form values if user is logged in and updating information--->
			<cfset f_firstName = session.user.firstname>
			<cfset f_lastName = session.user.lastname>
			<cfset f_address = session.user.address>
			<cfset f_address2 = session.user.address2>
			<cfset f_city = session.user.city>
			<cfset f_state = session.user.state>
			<cfset f_zip = session.user.zip>
			<cfset f_country = session.user.country>
			<cfset f_age = session.user.age>
			<cfset f_alias = session.user.username>
			<cfset f_email = session.user.email>
			<cfset f_password = session.user.password & "        ">
			<cfset f_viewEmail = session.user.viewEmail>
			<cfset f_updates = session.user.updates>
			<cfset f_signature = session.user.signature>
		<cfelse>
			<!---empty form values--->
			<cfset f_firstName = "">
			<cfset f_lastName = "">
			<cfset f_address = "">
			<cfset f_address2 = "">
			<cfset f_city = "">
			<cfset f_state = "">
			<cfset f_zip = "">
			<cfset f_country = "">
			<cfset f_age = "">
			<cfset f_alias = "">
			<cfset f_email = "">
			<cfset f_password = "">
			<cfset f_viewEmail = false>
			<cfset f_updates = false>
		</cfif>
		<cfoutput>
				<table border="0" width="90%" align="center">
					<tr>
						<td><p><cfif logged><strong>Your account was last updated on <cfif session.user.profilelastupdated EQ "">#dateFormat(session.user.joindate, "mmm dd, 
yyyy")#<cfelse>#dateFormat(session.user.profilelastupdated, "mmm dd, yyyy")#</cfif>.</strong></cfif></p>
					<br /><br /></td></tr>
		</cfoutput>
				<cfinclude template="includes/ipostreg.cfm">
				</table>

				</cfif>
			</table>
		</td>
	</tr>
</table>
<cfinclude template="includes/footer.cfm">