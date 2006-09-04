<!--- *****************************************************************************************************************
			
		Email validation written by Cole Barksdale. Not copyrighted. Can be downloaded, modified and distributed by 
		anyone to anyone.

		HOW TO USE IT:
		It's very simple. Those of you who are use to CustomTags and UDFs (User Defined Functions) shouldn't have
		any problem importing this into your applications.

		For those of you who aren't use to UDFs and CustomTags, this is a very simple piece of code to implement.

		Of course you can place this file anywhere you feel like calling it from (preferably in your web root or in
		the CFMX CustomTag directory).

		The syntax when validating an email address from a form will look like this:

			Include the file:
			<cfinclude template="<directory>/validemail.cfm">
			
			If the email is valid, the function will return a 1. If not, it will return a 0

			<cfif validEmail(form.email) EQ 1>
				[your good email message]
			<cfelse>
				[your bad email message]
			</cfif>

		Yep. It's that simple. Enjoy. If you have any questions, email me at cole@colebarksdale.com
				
		
****************************************************************************************************************** --->
<cfscript>
Function validEmail(email)
{
	email =  REFind("^[A-Za-z0-9](([_\.\-]?[a-zA-Z0-9]+)*)@([A-Za-z0-9]+)(([\.\-]?[a-zA-Z0-9]+)*)\.([A-Za-z]{2,})$", email);
	return email;
}
</cfscript>