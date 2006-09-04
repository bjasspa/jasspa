
<cfdump var="#cache.dumpCache()#" />
<cfset cache.setValue("test", 1, 1) />
<cfset cache.setValue("test2", "mine", 1) />
<cfoutput>
  #cache.getValue("test")#
</cfoutput>
<br><br>
This form also passes a URL variable.<br>
<cfform name="frmTst" action="index.cfm?urlVar=1234">
  Test: 
  <cfinput id="frmVar" name="frmVar" value="#cache.getValue('frmVar', '')#" maxlength="30" />
  <cfinput type="submit" name="btnSubmit" value="Submit" />
</cfform>
