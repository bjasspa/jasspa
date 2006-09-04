<cfcomponent output="false">
  <cfset this.name = "stickyCache">
  <cfset this.applicationTimeout = createTimeSpan(0,2,0,0)>
  <cfset this.sessionManagement = true>
  <cfset this.sessionTimeout = createTimeSpan(0,0,30,0)>
  <cfset this.setClientCookies = true>
  <!--- instantiate the class  --->
  <cfset variables.cache = createObject("component", "stickyCache.cfc.Cache") />
  <!--- what scope do we want to persist specified values in --->
  <cfset variables.defaultCacheScope = "session" />
  
  <cffunction name="onApplicationStart" returnType="boolean" output="false">
    <cfreturn true>
  </cffunction>
  <cffunction name="onApplicationEnd" returnType="void" output="false">
    <cfargument name="applicationScope" required="true">
  </cffunction>
  <cffunction name="onRequestStart" returnType="void" output="true">
    <!--- initialize the cache --->
    <cfset variables.cache.init(defaultCacheScope) />
    <!--- get persisted values --->
    <cfset variables.cache.retrieveCache() />
    <!--- set form and url variables into the cache --->
    <cfset variables.cache.addStructAsVariables(form) />
    <cfset variables.cache.addStructAsVariables(url) />
  </cffunction>
  <cffunction name="onRequest" returnType="void">
    <cfargument name="thePage" type="string" required="true">
    <cfinclude template="#arguments.thePage#">
  </cffunction>
  <cffunction name="onRequestEnd" returnType="void" output="false">
    <!--- set the values to persist into the persist scope --->
    <cfset variables.cache.persistCache() />
  </cffunction>
  <cffunction name="onError" returnType="void" output="true">
    <cfargument name="exception" required="true">
    <cfargument name="eventname" type="string" required="true">
    <cfdump var="#arguments.exception#" />
  </cffunction>
  <cffunction name="onSessionStart" returnType="void" output="false">
    
  </cffunction>
  <cffunction name="onSessionEnd" returnType="void" output="false">
    
  </cffunction>
</cfcomponent>