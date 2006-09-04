<cfcomponent name="facade">
  
  <cffunction name="init" access="public" returntype="facade" output="false">
    <cfargument name="scope" type="string" required="true" />
    <cfset var oScopes = getPageContext().getBuiltinScopes()>
    <cfset setScope(oScopes[arguments.scope])>
    <cfreturn this />
  </cffunction>
  
  <cffunction name="setValue" access="public" returntype="void" output="false">
    <cfargument name="name" type="string" required="true" />
    <cfargument name="value" type="any" required="true" />
    <cfset variables.scope[arguments.name] = arguments.value>
  </cffunction>
  
  <cffunction name="removeKey" access="public" returntype="void" output="false">
    <cfargument name="name" type="string" required="true" />
    <cfset structDelete(getScope(),arguments.name,false)>
  </cffunction>
  
  <cffunction name="getValue" access="public" returntype="any" output="false">
    <cfargument name="name" type="string" required="true" />
    <cfargument name="defaultValue" type="any" required="false" />
    
    <cfset var oScope = getScope()>
    <cfset var variableValue = "" />
    
    <cfif NOT structKeyExists(oScope, arguments.name) 
       AND isDefined("arguments.defaultValue")>
      <cfset oScope[arguments.name] = arguments.defaultValue />
    </cfif>
    
    <cftry>
      <cfset variableValue = oScope[arguments.name] />
      <cfcatch>
        <cfthrow type="FacadeError" errorcode="facade.getValue.Error" 
         message="Error ocurred in facade:" 
         detail="<strong>DETAIL:</strong><br>#name# does not exist and no default value was specified.<br>" />
      </cfcatch>
    </cftry>
    
    <cfreturn variableValue />
  </cffunction>
  
  <cffunction name="exists" access="public" returntype="boolean" output="false">
    <cfargument name="name" type="string" required="true" />
    <cfreturn structKeyExists(getScope(),arguments.name)>
  </cffunction>
  
  <cffunction name="setScope" access="private" returntype="void" output="false">
    <cfargument name="scope" type="struct" required="true" />
    <cfset variables.scope = arguments.scope>
  </cffunction>
  
  <cffunction name="getScope" access="public" returntype="struct" output="false">
    <cfreturn variables.scope />
  </cffunction>
  
</cfcomponent>