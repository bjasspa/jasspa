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

<cfquery name="topics" datasource="#Application.Database#">
  SELECT TOPICS.*, count(MESSAGES.messageid) AS messagecount, FORUMS.forumname
  FROM (TOPICS LEFT JOIN MESSAGES ON TOPICS.topicid = MESSAGES.topicid) LEFT JOIN FORUMS on TOPICS.forumid = FORUMS.forumid
  WHERE TOPICS.lastmessagedate >= '#DateFormat(DateAdd("d", -7, now()), "yyyy-mm-dd")#'
  GROUP BY TOPICS.topicid
  ORDER BY TOPICS.lastmessagedate desc,TOPICS.topicid desc
</cfquery>

<cfif NOT isDefined("page")><cfset page = 1></cfif>
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
      <table width="100%" cellpadding=0 cellspacing=0>
        <tr>
          <td width="50%" align="left">
            <p><a href="index.cfm" class="gear"><b>iPostMX</b></a></p>
          </td>
          <td width="38%" align="right" >
            <div class="pages">
            <b>Pages : [ </b>
            <cfoutput>
              <cfif page GT min_pages>
                &nbsp;<a href="active.cfm?page=#page-1#" class="gear">&##60;&##60;</a>
                &nbsp;<a href="active.cfm?page=#page-1#" class="gear">previous</a>&nbsp;
              </cfif>
            </cfoutput>
            <cfloop from="#min_pages#" to="#max_pages#" index="p">
              <cfoutput>
                <cfif page EQ p>
                  &nbsp;<a href="active.cfm?page=#p#" class="gear">#p#</a>&nbsp;
                <cfelse>
                  &nbsp;<a href="active.cfm?page=#p#" class="gear"><b>#p#</b></a>&nbsp;
                </cfif>
              </cfoutput>
            </cfloop>
            <cfoutput>
              <cfif page LT max_pages>
                &nbsp;<a href="active.cfm?page=#page+1#" class="gear">next</a>
                &nbsp;<a href="active.cfm?page=#page+1#" class="gear">&##62;&##62;</a>&nbsp;
              </cfif>
            </cfoutput>
            <b>]</b></div>
          </td>
        </tr>
      </table>
    </td>
  </tr>
  <tr>
    <td>
      <table cellpadding='0' cellspacing='0' border='0' width='100%'>
        <tr>
          <td colspan='2' class="tableColor"  background="images/menu.jpg">&nbsp;</td>
          <td width='45%' align='left' valign='middle' class="tableColor"  background="images/menu.jpg"><p><b>Topic Title</b></p></td>
          <td width='14%' align='center' valign='middle' class="tableColor"  background="images/menu.jpg"><p><b>Topic Starter</b></p></td>
          <td align='center' width='10%' valign='middle' class="tableColor"  background="images/menu.jpg"><p><b>Replies</b></p></td>
          <td align='center' width='10%' valign='middle' class="tableColor"  background="images/menu.jpg"><p><b>Views</b></p></td>
          <td align='center' width='15%' valign='middle' class="tableColor"  background="images/menu.jpg"><p><b>Last Action</b></p></td>
        </tr>
        
        <cfif topics.recordcount GT 0>
          <cfloop query="topics" startrow="#start_row#" endrow="#end_row#">
            <cfoutput>
              <tr bgcolor="###IIF(topics.currentrow MOD 2,DE('DEE3E7'),DE('EFEFEF'))#">
                <td height=20 class="forumTopic" align="center" colspan="2">
                  <img src="images/inactive.gif" border="0" alt="">
                </td>
                <td height=20 align="left" width="45%" class="forumTopic">
                  <a href="topics.cfm?forum=#topics.forumid#&topic=#topics.topicid#<cfif isDefined("url.page")>&page=#url.page#</cfif>" class="gear"><b>#topics.topictitle#</b></a>
                </td>
                <td height="20" align="center" width='14%' class="forumTopic">
                  <a href="javascript: void(0);" onClick="whois('#topics.membername#');" class="gear">#topics.membername#</a>
                </td>
                <td height=20 align="center" valign='middle' width='10%' class="forumTopic">
                  <p>#topics.messagecount-1#</p>
                </td>
                <td height=20 align="center" valign='middle' width='10%' class="forumTopic">
                  <p>#topics.views#</p>
                </td>
                <td height=20 align="center" valign='middle' width='15%' class="forumTopic">
                  <p>#DateFormat(lastmessagedate, "mmm dd, yyyy")#</p>
                </td>
              </tr>
            </cfoutput>
          </cfloop>
          <tr><td colspan="7" class="tableColor"  background="images/menu.jpg">&nbsp;</td></tr>
          <tr>
            <td align="right" colspan=8 style="padding:3px">
              <div class="pages"><b>Pages : [ </b>
              <cfoutput>
                <cfif page GT min_pages>
                  &nbsp;<a href="active.cfm?page=#page-1#" class="gear">&##60;&##60;</a>
                  &nbsp;<a href="active.cfm?page=#page-1#" class="gear">previous</a>&nbsp;
                </cfif>
              </cfoutput>
              <cfloop from="#min_pages#" to="#max_pages#" index="p">
                <cfoutput>
                  <cfif page EQ p>
                    &nbsp;<a href="active.cfm?page=#p#" class="gear">#p#</a>&nbsp;
                  <cfelse>
                    &nbsp;<a href="active.cfm?page=#p#" class="gear"><b>#p#</b></a>&nbsp;
                  </cfif>
                </cfoutput>
              </cfloop>
              <cfoutput>
                <cfif page LT max_pages>
                  &nbsp;<a href="active.cfm?page=#page+1#" class="gear">next</a>
                  &nbsp;<a href="active.cfm?page=#page+1#" class="gear">&##62;&##62;</a>&nbsp;
                </cfif>
              </cfoutput>
              <b>]</b></div>
            </td>
          </tr>
          <cfif topics.recordcount LT 10>
            <cfloop from="#10-topics.recordcount#" to="1" step="-1" index="i">
              <tr><td>&nbsp;</td></tr>
            </cfloop>
          </cfif>
        <cfelse>
          <tr>
            <td colspan="7" class="forumTopic" height="20" bgcolor="#eeeeee">
              &nbsp;<p>No active discussions found in the past three days.</p>
            </td>
          </tr>
          <tr><td colspan="7" class="tableColor">&nbsp;</td></tr>
        </cfif>
      </table>
    </td>
  </tr>
</table>
<br />
<br />
<cfinclude template="includes/footer.cfm">
