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

<cfif isDefined("url.forum") AND forum GT 0>
  <cfquery name="topics" datasource="#Application.Database#">
    SELECT TOPICS.*, COUNT(MESSAGES.MESSAGEID) AS MESSAGECOUNT, FORUMS.FORUMNAME
    FROM (TOPICS LEFT JOIN MESSAGES ON TOPICS.TOPICID = MESSAGES.TOPICID) 
    LEFT JOIN FORUMS ON TOPICS.FORUMID = FORUMS.FORUMID
    WHERE TOPICS.FORUMID = <cfqueryparam cfsqltype="cf_sql_integer" value="#url.forum#">
    GROUP BY TOPICS.TOPICID
    ORDER BY TOPICS.LASTMESSAGEDATE DESC,TOPICS.TOPICID DESC
  </cfquery>
  
  
  <cfif topics.recordcount EQ 0>
    <cfquery name="getForum" datasource="#Application.Database#">
      SELECT FORUMNAME FROM FORUMS
      WHERE FORUMS.FORUMID = <cfqueryparam cfsqltype="cf_sql_integer" value="#url.forum#">
    </cfquery>
  </cfif>
  
  
  <cfif NOT isDefined("page")>
    <cfset page = 1>
  </cfif>
  <cfset perpage = 20>
  <cfset start_row = ((page-1)*perpage) + 1>
  <cfset end_row = start_row + perpage>
  <cfset min_pages = 1>
  <cfset max_pages = Ceiling(topics.recordcount/ perpage)>
  
  
  <cfinclude template="includes/header.cfm">
  <cfinclude template="ipostmx_header.cfm">
  
  <table width="100%" align="center">
    <tr>
      <td>
        <table width="100%">
          <tr>
            <td width="50%" align="left">
              <p><a href="/ipostmx2005/" class="gear"><b>iPostMX 2005</b></a>&#62;
              <cfoutput>
                <cfif topics.recordcount NEQ 0>
                  #topics.forumname#
                <cfelse>
                  #getForum.forumname#
                </cfif>
              </cfoutput>
              </p>
            </td>
            <td width="38%" align="right" >
              <cfif max_pages GT 1>
                <p>Pages : [
                <cfoutput>
                  <cfif page GT min_pages>
                    &nbsp;<a href="forums.cfm?forum=#url.forum#&page=#page-1#" class="gear">&##60;&##60;</a>
                    &nbsp;<a href="forums.cfm?forum=#url.forum#&page=#page-1#" class="gear">previous</a>&nbsp;
                  </cfif>
                </cfoutput>
                <cfloop from="#min_pages#" to="#max_pages#" index="p">
                  <cfoutput>
                    <cfif page EQ p>
                      &nbsp;<a href="forums.cfm?forum=#url.forum#&page=#p#" class="gear">#p#</a>&nbsp;
                    <cfelse>
                      &nbsp;<a href="forums.cfm?forum=#url.forum#&page=#p#" class="gear"><b>#p#</b></a>&nbsp;
                    </cfif>
                  </cfoutput>
                </cfloop>
                <cfoutput>
                  <cfif page LT max_pages>
                    &nbsp;<a href="forums.cfm?forum=#url.forum#&page=#page+1#" class="gear">next</a>
                    &nbsp;<a href="forums.cfm?forum=#url.forum#&page=#page+1#" class="gear">&##62;&##62;</a>&nbsp;
                  </cfif>
                </cfoutput>
                ]
              <cfelse>
                &nbsp;
              </cfif>
              </p>
            </td>
          </tr>
        </table>
      </td>
    </tr>
    <tr>
      <td>
        <table cellpadding='0' cellspacing='0' border='0' width='100%'>
          <tr>
            <td colspan='2' class="tableColorIndex">&nbsp;</td>
            <td width='45%' align='left' valign='middle' class="tableColorIndex"><p><b>Topic Title</b></p></td>
            <td width='14%' align='center' valign='middle' class="tableColorIndex"><p><b>Topic Starter</b></p></td>
            <td align='center' width='10%' valign='middle' class="tableColorIndex"><p><b>Replies</b></p></td>
            <td align='center' width='10%' valign='middle' class="tableColorIndex"><p><b>Views</b></p></td>
            <td align='center' width='15%' valign='middle' class="tableColorIndex"><p><b>Last Action</b></p></td>
          </tr>
          
          <cfloop query="topics" startrow="#start_row#" endrow="#end_row#">
            <cfoutput>
              <tr>
                <td height="20" align="center" colspan="2" class="PreforumTopic">
                  <img src="images/inactive.gif" border="0" alt="">
                </td>
                <td height="20" align="left" width="45%" class="PreforumTopic">
                  <a href="topics.cfm?forum=#url.forum#&topic=#topics.topicid#<cfif isDefined("url.page")>&page=#url.page#</cfif>" class="gear"><b>#topics.topictitle#</b></a>
                </td>
                <td height="20" align="center" width='14%' class="PreforumTopic">
                  <a href="javascript: void(0);" onClick="whois('#topics.membername#');" class="gear">#topics.membername#</a>
                </td>
                <td height="20" align="center" valign='middle' width='10%' class="PreforumTopic">
                  #topics.messagecount-1#
                </td>
                <td height="20" align="center" valign='middle' width='10%' class="PreforumTopic">
                  #topics.views#
                </td>
                <td height="20" align="center" valign='middle' width='15%' class="PreforumTopic">
                  #DateFormat(lastmessagedate, "mmm dd, yyyy")#
                </td>
              </tr>
            </cfoutput>
          </cfloop>
          
          <cfif topics.recordcount EQ 0>
            <tr bgcolor="#EFEFEF">
              <td colspan=7 height="200" class="PreforumTopic">
                <p><b>No topics exists for this forum yet.</b></p>
              </td>
            </tr>
          <cfelseif topics.recordcount LT 10>
            <cfloop from="#10-topics.recordcount#" to="1" step="-1" index="i">
              <tr bgcolor="#eeeeee">
                <td colspan=7 style="border-left:1px solid #cccccc;border-right:1px solid #cccccc">&nbsp;</td>
              </tr>
            </cfloop>
          </cfif>
          <tr>
            <td colspan="7" class="tableColorIndex">&nbsp;</td></tr>
          
          <cfif max_pages GT 1>
            <tr bgcolor="#ffffff">
              <td align="right" colspan="7" style="padding:4px"><p>Pages : [ </b>
                <cfoutput>
                  <cfif page GT min_pages>
                    <p>
                    &nbsp;<a href="forums.cfm?forum=#url.forum#&page=#page-1#" class="gear">&##60;&##60;</a>
                    &nbsp;<a href="forums.cfm?forum=#url.forum#&page=#page-1#" class="gear">previous</a>
                    &nbsp;
                    </p>
                  </cfif>
                </cfoutput>
                <cfloop from="#min_pages#" to="#max_pages#" index="p">
                  <cfoutput>
                    <cfif page EQ p>
                      <p>
                      &nbsp;<a href="forums.cfm?forum=#url.forum#&page=#p#" class="gear">#p#</a>&nbsp;
                    <cfelse>
                      &nbsp;<a href="forums.cfm?forum=#url.forum#&page=#p#" class="gear"><b>#p#</b></a>&nbsp;
                    </cfif>
                  </cfoutput>
                </cfloop>
                <cfoutput>
                  <cfif page LT max_pages>
                    <p>
                    &nbsp;<a href="forums.cfm?forum=#url.forum#&page=#page+1#" class="gear">next</a>
                    &nbsp;<a href="forums.cfm?forum=#url.forum#&page=#page+1#" class="gear">&##62;&##62;</a>
                    &nbsp;
                  </cfif>
                </cfoutput>
                ]</b>
                </p>
              </td>
            </tr>
          </cfif>
          
	</table>
      </td>
    </tr>
  </table>
  <br />
  <cfinclude template="includes/footer.cfm">
  
<cfelse>
  <cflocation addtoken="no" url="index.cfm">
</cfif>
