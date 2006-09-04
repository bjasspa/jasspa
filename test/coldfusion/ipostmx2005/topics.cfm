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

<cfinclude template="includes/bbcode.cfm">
<cfif isDefined("topic") AND topic GT 0>
  <cfquery name="messages" datasource="#Application.Database#">
    SELECT MESSAGES.*, TOPICS.topictitle, FORUMS.forumname, MEMBERS.*
    FROM ((MESSAGES LEFT JOIN TOPICS ON MESSAGES.TOPICID = TOPICS.TOPICID) 
          LEFT JOIN FORUMS ON MESSAGES.FORUMID = FORUMS.FORUMID) 
    LEFT JOIN MEMBERS ON MESSAGES.MEMBERID = 
    MEMBERS.MEMBERID 
    WHERE MESSAGES.Topicid = <cfqueryparam cfsqltype="cf_sql_integer" value="#url.topic#">
    ORDER BY MESSAGES.messageid ASC
  </cfquery>
  
  <!---TAKE CARE OF VIEWS--->
  <cfset viewed = false>
  <cfloop from="1" to="#ArrayLen(session.a_views)#" index="i">
    <cfif topic EQ session.a_views[i]>
      <cfset viewed = true>
    </cfif>
  </cfloop>
  <cfif NOT viewed>
    <cfset ArrayAppend(session.a_views, topic)>
    <cfquery name="updateviews" datasource="#Application.Database#">
      UPDATE TOPICS
      SET views = views + 1
      WHERE topicid = <cfqueryparam cfsqltype="cf_sql_integer" value="#topic#">
    </cfquery>
  </cfif>
  <!---END TAKING CARE OF VIEWS--->
  
  <cfif NOT isDefined("url.print")>
    
    <cfinclude template="includes/header.cfm">
    <cfinclude template="ipostmx_header.cfm">
    
  </cfif>
  
  <table width="100%" align="center">
    <tr>
      <td>
        <table width="100%">
          <tr>
            <td align="left" colspan="2">
              <p><a href="index.cfm" class="gear"><b>iPostMX 2005</b></a>
              <cfoutput> #Chr(62)# 
                <a href="forums.cfm?forum=#messages.forumid#<cfif isDefined("url.page")>&page=#url.page#</cfif>" class="gear"><b>#messages.forumname#</b></a> 
                #Chr(62)# <b>#messages.topictitle#</b>
              </cfoutput>
            </td>
          </tr>
        </table>
      </td>
    </tr>
    <tr>
      <td>
        <table cellpadding='0' cellspacing='0' border='0' width='100%'>
          <tr>
            <td align="center" valign="top" class="tableColor" colspan="2">
              <cfoutput>
                <div class="heading"><b>#messages.topictitle#</b></div>
              </cfoutput>
            </td>
          </tr>
          
          <cfloop query="messages">
            <cfoutput>
              <tr bgcolor="###IIF(messages.currentrow MOD 2,DE('DEE3E7'),DE('EFEFEF'))#">
                <td width="20%" align="center" valign="top" class="forumTopic">
                  <a href="javascript:void(0);" onClick="whois('#messages.username#');" class="gear"><b>#messages.username#</b></a>
                  <p>Total Posts: #posts#
                  <br />
                  Member Since: #DateFormat(JoinDate, "mmm yyyy")#</p>
                </td>
                <td width="100%" align="left" class="forumTopic" style="padding-left: 5px;">
                  <b>Posted</b>: #DateFormat(messages.messagedate, "mmm dd, yyyy")#
                  <br />
                  <br />
                  <cfif messages.format EQ 1>
                    #smileyDecode(messages.messagecopy)#
                  <cfelseif messages.format EQ 2>
                    #bbDecode(messages.messagecopy)#
                  <cfelseif messages.format EQ 4>
                    #deHTML(messages.messagecopy)#
                  </cfif>
                  <br />
                  <cfif Len("messages.signature") GT 0>
                    <hr width="25%">
                    #messages.signature#
                  <cfelse>
                    <hr width="25%">
                  </cfif>
                  <cfif NOT isDefined("url.print")>
                    <div align="right">
                    <cfif logged AND messages.memberid EQ session.user.memberid>
                      <a href="deletepost.cfm?topic=#topicid#&forum=#forumid#&message=#messages.messageid#&RETURNURL=#URLEncodedFormat(myurl)#" 
                       class="gear"><b>delete</b></a>&nbsp;
                      <a href="messagepost.cfm?action=edit&topic=#topicid#&forum=#forumid#&message=#messageid#" class="gear"><b>edit</b></a>&nbsp;
                    </cfif>
                    <a href="messagepost.cfm?action=reply&topic=#topicid#&forum=#forumid#&message=#messageid#" class="gear"><b>reply</b></a>&nbsp;
                    <cfset mytitle = UrlEncodedFormat(topictitle)>
                    <!---<a href="javascript:void(0);" onClick="printIt('includes/print.cfm?title=#mytitle#&print=#URLEncodedFormat(myurl)#')" 
                    class="gear"><b>print</b></a>&nbsp;--->
                   <a href="#myurl###top" class="gear"><b>top</b></a>&nbsp;
                   <a href="#myurl###bottom" class="gear"><b>bottom</b></a>
                   </div>
                 </cfif>
               </td>
             </tr>
           </cfoutput>
         </cfloop>
         <tr>
           <td class="tableColor" colspan="2">&nbsp;</td>
         </tr>
       </table>
       <a name="bottom">&nbsp;</a>
     </td>
   </tr>
 </table>
 
 
 <br />
 <cfinclude template="includes/footer.cfm">
 
<cfelse>
  <cflocation addtoken="no" url="index.cfm">
</cfif>



