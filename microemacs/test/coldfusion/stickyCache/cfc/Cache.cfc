<cfcomponent displayname="Cache" hint="Cache class">
  
  <cfset variables.container = structNew() />
  <cfset variables.persistList = "" />
  <cfset variables.persistScope = "" />
  <cfset variables.facade = createObject("component", "stickyCache.cfc.Facade") />
  
  <cffunction name="init" output="false" returntype="Cache" access="public" hint="Creates an instance of stickyCache">
    <cfargument name="persistScope" type="string" required="true" default="session" hint="Name of scope in which to persist stickyCache" />
    
    <cfset variables.persistScope = arguments.persistScope />
    <cfset variables.facade.init(variables.persistScope) /> 
    
    <cfreturn this />
  </cffunction>
  
  <!--- public methods --->
  <cffunction name="getValue" output="false" returntype="any" access="public" hint="Returns a value from the cache">
    <cfargument name="variableName" type="string" required="true" hint="Name of the value we want to return" />
    <cfargument name="variableDefault" type="any" required="false" hint="Default value to set and return of value does not exist in the cache" />
    
    <cfset var variableValue = "" />
    
    <cfif NOT structKeyExists(variables.container, arguments.variableName) AND isDefined("arguments.variableDefault")>
      <cfset setValue(variableName, variableDefault) />
    </cfif>
    
    <cftry>
      <cfset variableValue = variables.container[arguments.variableName] />
      <cfcatch>
        <cfthrow type="stickyCacheError" errorcode="cache.getValue.Error" message="Error ocurred in stickyCache:" detail="<strong>DETAIL:</strong><br>#variableName# does not exist and no default value was specified.<br>" />
      </cfcatch>
    </cftry>
    <cfreturn variableValue />
  </cffunction>
  <!--- setValue() --->
  <cffunction name="setValue" output="false" returntype="any" access="public" hint="Sets a value into the cache">
    <cfargument name="variableName" type="string" required="true" hint="Name of the value we want to set" />
    <cfargument name="variableValue" type="any" required="true" hint="Actual value we want to set" />
    <cfargument name="persist" type="boolean" required="true" default="0" hint="Should this value be persisted across requests" />
    
    <cfset variables.container[arguments.variableName] = arguments.variableValue />
    <cfif arguments.persist>
      <cfset variables.persistList = listAppend(variables.persistList, arguments.variableName) />
    </cfif>
  </cffunction>
  <!--- removeValue() --->
  <cffunction name="removeValue" output="false" returntype="any" access="public" hint="Removes a value from the cache">
    <cfargument name="variableName" type="string" required="true" hint="Name of the value we want to remove" />
    
    <cfset structDelete(variables.container, arguments.variableName) />
    <cfif listFind(variables.persistList, arguments.variableName)>
      <cfset variables.persistList = listDeleteAt(variables.persistList, listFind(variables.persistList, arguments.variableName)) />
    </cfif>
    
  </cffunction>
  <!--- exists() --->
  <cffunction name="exists" output="false" returntype="boolean" access="public" hint="Checks if a variable exists in the cache">
    <cfargument name="variableName" type="string" required="true" hint="Name of the value we want to check" />
    
    <cfset var doesExist = 0 />
    
  </cffunction>
  <!--- dumpCache() --->
  <cffunction name="dumpCache" output="false" returntype="struct" access="public" hint="Returns all values in the cache">
    
    <cfreturn variables.container />
  </cffunction>
  <!--- clearCache() --->
  <cffunction name="clearCache" output="false" returntype="void" access="public" hint="Clears the entire Cache">
    
    <cfset structClear(variables.container) />
  </cffunction>
  <!--- persistCache() --->
  <cffunction name="persistCache" output="false" returntype="void" access="public" hint="Persists values in the cache that were set to persist across the request">
    
    <cfset var myCache = structNew() />
    
    <cfloop list="#variables.persistList#" index="i">
      <cfset myCache[i] = variables.container[i] />
    </cfloop>
    
    <cfset variables.facade.setValue("stickyCache", myCache) />
  </cffunction>
  <!--- retrieveCache() --->
  <cffunction name="retrieveCache" output="false" returntype="void" access="public" hint="Returns values that were persisted across the request and sets them back into the cache">
    
    <cfset var myCache = variables.facade.getValue("stickyCache", structNew()) />
    <cfset variables.facade.removeKey("stickyCache") />
    <cfdump var="#myCache#" />---
    <cfloop collection="#myCache#" item="current">
      <cfset variables.container[current] = myCache[current] />
      <cfset variables.persistList = listAppend(variables.persistList, current) />
    </cfloop>
    
  </cffunction>
  <!--- addStructAsVariables() --->
  <cffunction name="addStructAsVariables" output="false" returntype="void" access="public" hint="Adds a struct into the cache as single variable values (used for form and url scopes)">
    <cfargument name="structName" type="struct" required="true" hint="Structure to break apart (generally form or url)" />
    
    <cfloop collection="#arguments.structName#" item="key">
      <cfset variables.container[key] = arguments.structName[key] />
    </cfloop>
    
  </cffunction>
</cfcomponent>