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

<cfinclude template="includes/header.cfm">
<cfinclude template="ipostmx_header.cfm">

<table width="100%">
  
  <tr>
    <td>&nbsp;</td>
  </tr>
  <tr>
    <td>
      <div style="font: .9em,verdana;"><b>Search</div>
    </td>
  </tr>
  <tr>
    <td>&nbsp;</td>
  </tr>
  <form style="padding:0px;margin:0px" method="post" action="search.cfm">
  <tr>
    <td colspan=2 align="center">
      <p>Search Criteria:&nbsp;&nbsp;
      <input type="text" name="phrase1" maxlength="50" size="10">
      <select name="mod1">
        <option value="AND">AND</option>
        <option value="OR">OR</option>
      </select>
      <input type="text" name="phrase2" maxlength="50" size="10">
      <select name="mod2">
        <option value="AND">AND</option>
        <option value="OR">OR</option>
      </select>
      <input type="text" name="phrase3" maxlength="50" size="10">
      <br /><br />
      Author:&nbsp;&nbsp;
      <input type="text" name="author" maxlength="50">
      &nbsp;&nbsp;&nbsp;
      Filter by Date within last:&nbsp;&nbsp;
      <select name="days">
        <option value="10000"></option>
        <option value="1">1 day</option>
        <option value="7">7 days</option>
        <option value="14">2 weeks</option>
        <option value="31">1 month</option>
        <option value="90">3 months</option>
        <option value="180">6 months</option>
        <option value="365">1 year</option>
      </select>
      <br /><br />
      <input type="submit" name="submit" value="submit">
      
    </td>
  </tr>
  </form>
  
  
  <cfif isDefined("form.submit") and Len(form.phrase1) GT 0>
    <cfset sql ="SELECT TOPICS.*, FORUMS.forumname FROM TOPICS LEFT JOIN FORUMS ON TOPICS.forumid = FORUMS.forumid WHERE ">
    <cfoutput>
      <cfif Len(form.phrase1) NEQ 0>
        <cfset sql = sql & "topictitle LIKE '%" & form.phrase1 & "%'">
      </cfif>
      <cfif Len(form.phrase1) NEQ 0 AND Len(form.phrase2) NEQ 0>
        <cfset sql = sql & " " & form.mod1 & " ">
      </cfif>
      <cfif Len(form.phrase2) NEQ 0>
        <cfset sql = sql & "topictitle LIKE '%" & form.phrase2 & "%'">
      </cfif>
      <cfif Len(form.phrase2) NEQ 0 AND Len(form.phrase3) NEQ 0>
        <cfset sql = sql & " " & form.mod2 & " ">
      </cfif>
      <cfif Len(form.phrase3) NEQ 0>
        <cfset sql = sql & " " & form.mod2 & " ">
        <cfset sql = sql & "topictitle LIKE '%" & form.phrase3>
      </cfif>
      <cfif (Len(form.phrase1) NEQ 0 OR Len(form.phrase2) OR 
         Len(form.phrase3) NEQ 0) AND 
         (isDefined("form.author") AND Len(form.author) GT 0)>
        <cfset sql = sql & " AND membername = '" & form.author & "'">
      <cfelseif isDefined("form.author") AND Len(form.author) GT 0>
        <cfset sql = sql & "membername = '" & form.author & "'">
      </cfif>
      
      <cfif isDefined("form.days") AND form.days NEQ 0>
        <cfset sql = sql & " AND lastmessagedate > " & #DateAdd('d', -form.days, NOW() )#>
     </cfif>
     
     <cfquery name="getTopics" datasource="#Application.Database#">
       #PreserveSingleQuotes(sql)# ORDER BY lastmessagedate DESC
     </cfquery>
     
     <cfif getTopics.recordcount GT 0>
       <tr>
         <td colspan=2>&nbsp;</td>
       </tr>
       <tr>
         <td colspan=2><p><b>Results: ( #gettopics.recordcount# )</b></p>
         </td>
       </tr>
       <tr>
         <td colspan=2 align="center">
           <table width="100%" cellspacing=0 cellpadding=3 border=0>
             <tr>
               <td width="13%" bgcolor="##f6723b" align="center" style="color:white;border-top:2px outset ##f4470f;border-bottom:2px inset ##f4470f">
                 <strong>Post Date</strong>
               </td>
               <td width="22%" bgcolor="##f6723b" align="center" style="color:white;border-top:2px outset ##f4470f;border-bottom:2px inset ##f4470f">
                 <strong>Forum</strong>
               </td>
               <td width="50%" bgcolor="##f6723b" style="color:white;border-top:2px outset ##f4470f;border-bottom:2px inset ##f4470f">
                 <strong>Topic</strong>
               </td>
               <td width="15%" bgcolor="##f6723b" align="center" style="color:white;border-top:2px outset ##f4470f;border-bottom:2px inset ##f4470f">
                 <strong>Started By</strong>
               </td>
             </tr>
             <cfloop query="getTopics">
               <tr>
                 <td width="13%" bgcolor="##eeeeff" align="center" 
                    style="border-bottom:1px solid ##cccccc;border-left:1px solid ##cccccc;padding-top:4px;padding-bottom:4px">
                   #DateFormat(gettopics.lastmessagedate, "dd/mm/yy")#
                 </td>
                 <td width="22%" bgcolor="##eeeeff" align="left" style="border-bottom:1px solid ##cccccc;">
                   #gettopics.forumname#
                 </td>
                 <td width="50%" bgcolor="##eeeeff" align="left" style="border-bottom:1px solid ##cccccc;">
                   <a href="topics.cfm?forum=#gettopics.forumid#&topic=#gettopics.topicid#" 
                    onMouseOut="this.style.backgroundColor = '##eeeeff';" 
                    onMouseOver="this.style.backgroundColor = '##cccccc';" 
                    style="text-decoration:none;color:##000000">
                   <strong>#gettopics.topictitle#</strong></a>
                 </td>
                 <td width="15%" bgcolor="##eeeeff" align="center" 
                    style="border-right:1px solid ##cccccc;border-bottom:1px solid ##cccccc;">
                   #gettopics.membername#
                 </td>
               </tr>
             </cfloop>
             <tr>
               <td bgcolor="##f6723b" style="color:white;border-top:2px outset ##f4470f;border-bottom:2px inset ##f4470f" colspan=4>
                 &nbsp;
               </td>
             </tr>
           </table>
         </td>
       </tr>
     <cfelse>
       <tr>
         <td colspan=2>
           &nbsp;
         </td>
       </tr>
       <tr>
         <td colspan=2>
           <p><b>Results: ( #gettopics.recordcount# )</b></p>
         </td>
       </tr>
       <tr>
         <td colspan=2>
           <p>Your search did not produce any results.</p>
         </td>
       </tr>
     </cfif>
     
   </cfoutput>
 </cfif>
 
 
</table>
<br />
<br />
<cfinclude template="includes/footer.cfm">