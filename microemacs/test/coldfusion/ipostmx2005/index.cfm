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

<cfif NOT isDefined("cookie.ipostmx")>
  <cfset ForumDT = Now() - 1000>
<cfelse>
  <cfset ForumDT = ListGetAt(cookie.ipostmx, 4, ",")>
</cfif>


<!---GET MESSAGE INFORMATION--->
<cfquery name="forum" datasource="#Application.Database#">
  SELECT FORUMS.FORUMID, FORUMS.CATEGORYID, FORUMS.FORUMNAME, 
  FORUMS.DESCRIPTION, CATEGORIES.CATID, CATEGORIES.CATEGORY, COUNT(TOPICS.TOPICID) AS TOPICNUM
  FROM ((FORUMS LEFT JOIN CATEGORIES ON FORUMS.CATEGORYID = CATEGORIES.CATID) 
        LEFT JOIN TOPICS ON FORUMS.FORUMID = TOPICS.FORUMID)
  GROUP BY FORUMS.FORUMID
  ORDER BY FORUMS.CATEGORYID
</cfquery>

<cfquery name="forum2" datasource="#Application.Database#">
  SELECT MAX(MESSAGES.MESSAGEID) AS MESSAGEID, COUNT(MESSAGES.MESSAGEID) AS REPLYNUM, FORUMS.CATEGORYID
  FROM MESSAGES LEFT JOIN FORUMS ON MESSAGES.FORUMID = FORUMS.FORUMID
  GROUP BY MESSAGES.FORUMID
  ORDER BY CATEGORYID, MESSAGES.FORUMID
</cfquery>

<cfset mList = ValueList(forum2.messageid)>
<cfset rList = ValueList(forum2.replynum)>

<cfif forum2.recordcount GT 0>
  <cfquery name="getMembers" datasource="#Application.Database#">
    SELECT MESSAGES.USERNAME, MESSAGES.MESSAGEDATE, MESSAGES.FORUMID, FORUMS.CATEGORYID
    FROM MESSAGES 
    LEFT JOIN FORUMS ON MESSAGES.FORUMID = FORUMS.FORUMID
    WHERE MESSAGEID IN (#MLIST#)
    GROUP BY MESSAGES.FORUMID
    ORDER BY FORUMS.CATEGORYID, MESSAGES.FORUMID
  </cfquery>
</cfif>

<cfset mArray = ArrayNew(1)>
<cfset dArray = ArrayNew(1)>
<cfset ArraySet(mArray, 1, forum.recordcount, 0)>
<cfset ArraySet(dArray, 1, forum.recordcount, 0)>


<cfloop query="getmembers">
  <cfset mArray[getmembers.currentrow] = getMembers.username>
  <cfset dArray[getmembers.currentrow] = getMembers.messagedate>
</cfloop>



<!---END MESSAGE RETRIEVAL--->

<cfset forum_cap = forum.categoryid - 1>

<cfinclude template="includes/header.cfm">
<cfinclude template="ipostmx_header.cfm">

<table width="100%" cellspacing="0" cellpadding="0" align="center">
  <tr>
    <td align="left">
      <table cellpadding="0" cellspacing="0" border="0" width="100%">
        <tr>
          <td width="5%" class="tableColorIndex">&nbsp;</td>
          <td width="50%" class="tableColorIndex"><p><b>Forums</b></p></td>
          <td width="1%" align="center" class="tableColorIndex"><p><b>Topics</b></p></td>
          <td width="1%" align="center" class="tableColorIndex"><p><b>Replies</b></p></td>
          <td width="15%" align="center" class="tableColorIndex"><p><b>Last Post Info</b></p></td>
        </tr>
        
        <!---LET THE FUN BEGIN--->
        <cfset mArray_index = 1>
        <cfloop query="forum" startrow="1" endrow="#forum.recordcount#">
          <cfoutput>
            <cfif forum.categoryid GT forum_cap><cfset forum_cap = forum_cap + 1>
              <tr>
                <td colspan="5" height="15" class="forumCat">
                  <div class="fHeading"><b>#forum.category#</b></div>
                </td>
              </tr>
            </cfif>
            <tr>
              <td align="center" width="5%" class="PreforumTopic">
                <cfif dArray[forum.currentrow] GT 0 AND dArray[forum.currentrow] GT forumdt>
                  <img src="images/active.gif" border="0" alt="">
                <cfelse>
                  <img src="images/inactive.gif" border="0" alt="">
                </cfif>
              </td>
              <td align="left" width='45%' class="PreforumTopic">
                <a href="forums.cfm?forum=#forum.forumid#" class="gear"><b>#forum.forumname#</b></a>
                <p>#forum.description#</p>
              </td>
              <td align='center' valign='middle' width='10%' class="PreforumTopic">
                <cfif forum.topicnum GT 0>
                  #forum.topicnum#
                <cfelse>
                  0
                </cfif>
              </td>
              <td align='center' valign='middle' width='10%' class="PreforumTopic">
                <cfif forum.topicnum GT 0>
                  #ListGetAt(rList, forum.currentrow)-forum.topicnum#
                <cfelse>
                  0
                </cfif>
              </td>
              <td align="center" valign='middle' width='35%' class="PreforumTopic">
                <p>
                <cfif forum.topicnum GT 0>
                  #DateFormat(dArray[forum.currentrow], "mmm dd, yyyy")# #TimeFormat(dArray[forum.currentrow], "hh:mm tt")#
                  <br />
                  By: <a href="javascript: void(0);" onClick="whois('#mArray[forum.currentrow]#');" class="gear">#mArray[forum.currentrow]#</a>
                  <cfset mArray_index = mArray_index + 1>
                <cfelse>
                  &nbsp;
                </cfif>
                </p>
              </td>
            </tr>
          </cfoutput>
        </cfloop>
        <tr>
          <td bgcolor="#C0C0C0" style="border-top:1px outset #000000;border-bottom:1px inset #000000" colspan='5'>&nbsp;</td>
        </tr>
        <tr>
          <td bgcolor="#ffffff" colspan='5' style="padding:3px; font: .6em, verdana, tahoma, arial, helvetica, sans-serif;" valign="top">&nbsp;<img src="images/active.gif" 
             border="0" alt="New Messages" align="absmiddle"> - New Messages &nbsp;&nbsp;<img src="images/inactive.gif" border="0" alt="No New Messages" align="absmiddle"> - 
            No New Messages</td>
        </tr>
      </table>
    </td>
  </tr>
</table>
<br />
<br />
<br />
<cfinclude template="includes/footer.cfm">


<!---SET COOKIE VARIABLES--->
<cfif isDefined("session.user") AND session.user.rememberme>
  <cfcookie expires="never" name="ipostmx" value="#session.user.email#,#SESSION.USER.PASSWORD#,#session.user.email#, #now()#">
<cfelseif isDefined("session.user") AND NOT session.user.rememberme AND isDefined("cookie.ipostmx")>
  <cfcookie expires="never" name="ipostmx" value="0,0,0, #now()#">
<cfelse>
  <cfcookie expires="never" name="ipostmx" value="0,0,0, #now()#">
</cfif>
<!---END COOKIE SETTING--->
