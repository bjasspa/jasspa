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

<cfif isDefined("url.topic") AND topic GT 0>
  
  <cfif isDefined("form.bYes")>
    <!---IF URL MESSAGE AND TOPIC DO NOT MATCH FORM MESSAGE AND TOPIC IDS THEN URL TAMPERING HAS OCCURRED REDIRECT TO INDEX PAGE--->
    <cfif url.message NEQ form.f_messageid OR url.topic NEQ form.f_topicid>
      <cflocation url="index.cfm" addtoken="no">
    </cfif>
    <!---IF MESSAGE_NUMBER = 1 DELETE MESSAGE AND TOPIC OTHERWISE JUST DELETE MESSAGE--->
    <cfif form.message_number NEQ 1>
      <cfquery name="deleteMessage" datasource="#Application.Database#">
        DELETE
        FROM MESSAGES
        WHERE MessageID = <cfqueryparam cfsqltype="cf_sql_integer" value="#url.message#">
      </cfquery>
      <!---TAKE THEM BACK TO TOPICS PAGE--->
      <cflocation url="topics.cfm?forum=#url.forum#&topic=#url.topic#" addtoken="no">
    <cfelse>
      <!---DELETE MESSAGE--->
      <cfquery name="deleteMessage" datasource="#Application.Database#">
        DELETE
        FROM MESSAGES
        WHERE MessageID = <cfqueryparam cfsqltype="cf_sql_integer" value="#url.message#">
      </cfquery>
      <!---DELETE TOPIC--->
      <cfquery name="deleteTopic" datasource="#Application.Database#">
        DELETE
        FROM TOPICS
        WHERE TopicID = <cfqueryparam cfsqltype="cf_sql_integer" value="#url.topic#">
      </cfquery>
      <!---TAKE THEM BACK TO FORUMS PAGE--->
      <cflocation url="forums.cfm?forum=#url.forum#" addtoken="no">
    </cfif>
  <cfelse>
    
    <cfquery name="messages" datasource="#Application.Database#">
      SELECT MESSAGES.*, TOPICS.topictitle, FORUMS.forumname, FORUMS.forumid, MEMBERS.*
      FROM ((MESSAGES LEFT JOIN TOPICS ON MESSAGES.TOPICID = TOPICS.TOPICID) 
            LEFT JOIN FORUMS ON MESSAGES.FORUMID = FORUMS.FORUMID) 
      LEFT JOIN MEMBERS ON MESSAGES.MEMBERID = MEMBERS.MEMBERID
      WHERE MESSAGES.topicid = <cfqueryparam cfsqltype="cf_sql_integer" value="#topic#">
      ORDER BY MESSAGES.messageid asc
    </cfquery>
    
    
    <cfinclude template="includes/header.cfm">
    <cfinclude template="ipostmx_header.cfm">
    
    <table width="60%" align="center">
      <tr>
        <td>
          <cfif logged>
            <cfoutput>
              <cfloop query="messages">
		<cfif messages.messageid EQ url.message>
                  <tr>
                    <td colspan="2">
                      <table cellpadding='0' cellspacing='0' border='0' width='100%'>
                        <form style="padding:0px;margin0px;width:0px;" method="post" action="#myurl#">
                        <tr>
                          <td colspan="2" nowrap>
                            <p>
                            <strong>Delete the following message?</strong>&nbsp;&nbsp;
                            <input type="submit" name="bYes" value="YES" />&nbsp;&nbsp;
                            <input type="button" value="NO" onClick="location.href = '#url.returnurl#';"/>
                            <input type="hidden" name="message_number" value="#messages.recordcount#">
                            <input type="hidden" name="f_messageid" value="#url.message#">
                            <input type="hidden" name="f_topicid" value="#url.topic#">
                            </p>
                          </td>
                        </tr>
                        </form>
                        <tr>
                          <td>&nbsp;</td>
                        </tr>
                        <tr>
                          <td width="25%" bgcolor="##EFEFEF" align="center" valign="top" style="border:1px solid ##cccccc;padding-top:15px">
                            <p><strong>#messages.username#</strong>
                            <br />
                            Total Posts: #messages.posts#<br />
                            Member Since: #DateFormat(messages.JoinDate, "mmm yyyy")#</p>
                          </td>
                          <td width="75%" bgcolor="##EFEFEF" align="left" style="padding:10px;border:1px solid ##cccccc;border-left:0px">
                            <p><strong>Posted</strong>: #DateFormat(messages.messagedate, "mmm dd, yyyy")#
                            <br />
                            <br />
                            #ParagraphFormat(messages.messagecopy)#<br /></p>
                          </td>
                        </tr>
                      </cfif>
                    </cfloop>
                  </cfoutput>
                <cfelse>
                  <tr>
                    <td colspan="2">
                      <table cellpadding='4' cellspacing='1' border='0' width='100%'>
                        <tr>
                          <td width="100%" align="center"><br /><br /><p><strong>Sorry, you must be logged in to reply to topics.</strong><br /><br />
                            If you alread have an account, you may log in <a href="userlogin.cfm?RETURNURL=<cfoutput>#URLEncodedFormat(myurl)#</cfoutput>" 
                             class="gear"><strong>here</strong></a>.<br /><br />
                            If you don't have an account, you may create one <a href="account.cfm" class="gear"><strong>here</strong></a>.
                          </td>
                        </tr>
                      </cfif>
                      
                    </table>
                  </td>
                </tr>
              </table>
              
              
              
            </cfif>
            <br />
            <br />
            <cfinclude template="includes/footer.cfm">
          <cfelse>
            <cflocation addtoken="no" url="index.cfm">
          </cfif>
