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
<cfif (isDefined("topic") AND topic GT 0) OR (isDefined("url.action") AND url.action EQ "topic")>
  
  <cfif isDefined("url.action") AND url.action EQ "edit">
    <cfset editMode = "edit">
    <cfquery name="messages" datasource="#Application.Database#" maxrows="1">
      SELECT MESSAGES.*, TOPICS.topictitle, FORUMS.forumname, FORUMS.forumid, MEMBERS.*
      FROM ((MESSAGES LEFT JOIN TOPICS ON MESSAGES.TOPICID = TOPICS.TOPICID) 
            LEFT JOIN FORUMS ON MESSAGES.FORUMID = FORUMS.FORUMID) 
      LEFT JOIN MEMBERS ON MESSAGES.MEMBERID = MEMBERS.MEMBERID
      WHERE MESSAGES.messageid = <cfqueryparam cfsqltype="cf_sql_integer" value="#message#">
    </cfquery>
    <cfset msg = messages.messagecopy>
    <cfif FindNoCase("This message was edited by: ", msg)>
      <cfset i = FindNoCase(Chr(13) & Chr(13) & "This message was edited by: ", msg,1)>
      <cfset msg = Left(msg, i-1)>
    </cfif>
    
  <cfelseif isDefined("url.action") AND url.action EQ "reply">
    <cfset editMode = "post">
    <cfset msg = "">
    <cfquery name="messages" datasource="#Application.Database#">
      SELECT MESSAGES.*, TOPICS.topictitle, TOPICS.notifyauthor, TOPICS.notifyemail, FORUMS.forumname, FORUMS.forumid, MEMBERS.*
      FROM ((MESSAGES LEFT JOIN TOPICS ON MESSAGES.TOPICID = TOPICS.TOPICID) 
            LEFT JOIN FORUMS ON MESSAGES.FORUMID = FORUMS.FORUMID) 		      
      LEFT JOIN MEMBERS ON MESSAGES.MEMBERID = MEMBERS.MEMBERID
      WHERE MESSAGES.Topicid = <cfqueryparam cfsqltype="cf_sql_integer" value="#topic#">
      ORDER BY MESSAGES.messageid ASC
    </cfquery>
  <cfelseif isDefined("url.action") AND url.action EQ "topic">
    <cfset editMode = "topic">
    <cfset msg = "">
    <cfquery name="messages" datasource="#Application.Database#">
      SELECT FORUMS.forumname, FORUMS.forumid
      FROM FORUMS
      WHERE forumid = <cfqueryparam cfsqltype="cf_sql_integer" value="#forum#">
    </cfquery>
  </cfif>
</cfif>
  
  
  <cfinclude template="includes/header.cfm">
  <cfinclude template="ipostmx_header.cfm">
  
  <table width="100%" align="center">
    
    
    <cfif isDefined("form.bsubmit") AND EditMode EQ "post">
      <!---INSERT NEW MESSAGE INTO DATABASE--->
      <cfset msg = form.message>
      <cfset enterbreak = Replace('#form.message#',chr(13),"<br />","ALL")>
      <cfquery name="insertMessage" datasource="#Application.Database#">
        INSERT INTO MESSAGES (TopicID, ForumID, MemberID, UserName, UserEmail, MessageDate, MessageCopy, Format)
        VALUES (#url.topic#, #url.forum#, #session.user.memberid#, '#session.user.username#', '#session.user.email#', NOW(), '#msg#', '#form.encoding#')
      </cfquery>
      <cfquery name="insertMessage" datasource="#Application.Database#">
        UPDATE MEMBERS Set Posts = Posts+1 where memberid = #session.user.memberid#
      </cfquery>
      
      <cfquery name="updateDate" datasource="#Application.Database#">
        UPDATE TOPICS set lastmessagedate = NOW() where topicid = #url.topic#
      </cfquery>
      
      <!---<cfif form.notifyauthor EQ 1>--->
      <cfquery name="emailAuthor" datasource="#Application.Database#">
        SELECT * 
        FROM TOPICS
        WHERE topicid = #URL.topic#
        AND notifyauthor = 1
      </cfquery>
      
      <cfif emailAuthor.RecordCount GT 0>
        <cfmail to="#emailAuthor.notifyemail#" from="ipostmx@ipostmx.com" subject="Forum reply">
          #session.user.username# has posted a reply to the topic:
          
          http://locahost/forum/topics.cfm?forum=#URL.forum#&topic=#URL.topic#
          </cfmail>
        </cfif>
        
        <!---</cfif>--->
        <cflocation addtoken="no" url="topics.cfm?forum=#messages.forumid#&topic=#messages.topicid#">
      <cfelseif isDefined("form.bsubmit") AND EditMode EQ "edit">
        <!---INSERT UPDATED MESSAGE INTO DATABASE--->
        <cfset msg = form.message>
        <cfquery name="updateMessage" datasource="#Application.Database#">
          UPDATE MESSAGES
          SET topicid = #url.topic#, 
          forumid = #url.forum#, 
          memberid = #session.user.memberid#, 
          username = '#session.user.username#',
          useremail = '#session.user.email#', 
          messagedate= NOW(), 
          messagecopy= '#msg#',
          format='#form.encoding#'
          WHERE MessageID = #url.message#
        </cfquery>
        <cflocation addtoken="no" url="topics.cfm?forum=#messages.forumid#&topic=#messages.topicid#">
      <cfelseif isDefined("form.bSubmit") AND EditMode EQ "topic">
        <!---INSERT NEW MESSAGE AND TOPIC INTO DATABASE--->
        <cfset msg = form.message>
        <cfif isDefined("form.subscribe")><cfset notify=1><cfelse><cfset notify = 0></cfif>
        <cfquery name="insertTopic" datasource="#Application.Database#">
          INSERT INTO TOPICS (ForumID, TopicTitle, Views, MemberName, LastMessageDate, notifyauthor, notifyemail,memberid)
          VALUES (#url.forum#, '#form.topictitle#', 0, '#session.user.username#', NOW(), #notify#, '#session.user.email#',#session.user.memberid#)
        </cfquery>
        <cfquery name="Maxtopic" datasource="#Application.Database#">
          SELECT Max(Topicid) AS maxid
          FROM TOPICS
        </cfquery>
        <cfquery name="insertMessage" datasource="#Application.Database#">
          INSERT INTO MESSAGES (TopicID, ForumID, MemberID, UserName, UserEmail, MessageDate, MessageCopy)
          VALUES (#maxtopic.maxid#, #url.forum#, #session.user.memberid#, '#session.user.username#', '#session.user.email#', NOW(), '#msg#')
        </cfquery>
        <cfquery name="insertMessage" datasource="#Application.Database#">
          UPDATE MEMBERS Set Posts = Posts+1 where memberid = #session.user.memberid#
        </cfquery>
        <cflocation addtoken="no" url="topics.cfm?forum=#messages.forumid#&topic=#maxtopic.maxid#">
      </cfif>
      <!---END MESSAGE INSERTION--->
      
      
      <tr>
        <td align="left" colspan="2">
          <p>
          <cfif isDefined("url.action") AND url.action NEQ "topic">
            <a href="index.cfm" class="gear"><b>iPostMX</b></a>
            <cfoutput> 
              &gt;
              <a href="forums.cfm?forum=#messages.forumid#" class="gear"><b>#messages.forumname#</b></a> &gt;
              <a href="topics.cfm?topic=#messages.topicid#" class="gear"><b>#messages.topictitle#</b></a>
            </cfoutput>
          <cfelse>
            <a href="index.cfm" class="gear"><b>iPostMX</b></a><cfoutput> #Chr(62)# <a href="forums.cfm?forum=#messages.forumid#" 
               class="gear"><b>#messages.forumname#</b></a> </a></cfoutput>
          </cfif>
          </p>
        </td>
      </tr>
      <cfif logged>
        
        <cfoutput>
          <tr>
            <td align="left">
              <table cellpadding='0' cellspacing='0' border='0' width='100%'>
                <cfif isDefined("url.action") AND url.action NEQ "topic">
                  <tr>
                    <td height="15" class="tableColor" colspan="2" align="left">
                      <p><b>Replying to:</b> #messages.topictitle# 
                      <br />
                      <b>Started by:</b> #messages.username#</p>
                    </td>
                  </tr>
                  <tr><td>&nbsp;</td></tr>
                  <tr>
                    <td colspan="2" style="padding-left:10px;">
                      <form style="padding:0px;margin:0px;" name="postForm" method="post" onSubmit="return validMessage(this);">
                    <cfelse>
                      <tr>
                        <td colspan="2" style="padding-left:10px;">
                          <form style="padding:0px;margin:0px;" name="postForm" method="post" onSubmit="return validMessage(this);">
                          <br /><p><b>Topic Title:</b>&nbsp;
                          <input name="topictitle" type="text" value="" style="width:240px;background-color:##eeeeee;border:1px solid ##200057;" maxlength="200"/><br /><br /><br 
                           />
                        </cfif>
                        
                        <p><b>Message Text:</b><br /><br />
                        <input type="button" name="bBold" value="bold" onClick="makeBold();" alt="Use this button to create bold text." title="Use this button to create bold 
                         text.">
                        <input type="button" name="bItalic" value="italic" onClick="makeItalic();" alt="Use this button to create italicized text." title="Use this button to 
                         create italicized text.">
                        <input type="button" name="bUnderline" value="underline" onClick="makeUnderline();" alt="Use this button to create text that is underlined." title="Use 
                         this button to create text that is underlined.">
                        <input type="button" name="bHTTP" value="http://" onClick="makeHTTP();" alt="Use this button to insert a web link." title="Use this button to insert a 
                         web link.">
                        <br /><br />
                        <a href="javascript:void(0);" onClick="emotion('happy');"><img src="images/icon_happy.gif" border=0 alt="insert happy"></a>&nbsp;&nbsp;
                        <a href="javascript:void(0);" onClick="emotion('sad');"><img src="images/icon_sad.gif" border=0 alt="insert sad"></a>&nbsp;&nbsp;
                        <a href="javascript:void(0);" onClick="emotion('mad');"><img src="images/icon_mad.gif" border=0 alt="insert mad"></a>&nbsp;&nbsp;
                        <a href="javascript:void(0);" onClick="emotion('shock');"><img src="images/icon_shock.gif" border=0 alt="insert shock"></a>&nbsp;&nbsp;
                        <a href="javascript:void(0);" onClick="emotion('confused');"><img src="images/icon_confused.gif" border=0 alt="insert confused"></a>&nbsp;&nbsp;
                        <a href="javascript:void(0);" onClick="emotion('wink');"><img src="images/wink.gif" border=0 alt="insert wink"></a>&nbsp;&nbsp;
                        <a href="javascript:void(0);" onClick="emotion('evil');"><img src="images/icon_evil.gif" border=0 alt="insert evil"></a>&nbsp;&nbsp;
                        <a href="javascript:void(0);" onClick="emotion('cool');"><img src="images/icon_cool.gif" border=0 alt="insert cool"></a>&nbsp;&nbsp;
                        <a href="javascript:void(0);" onClick="emotion('sigh');"><img src="images/icon-sigh.gif" border=0 alt="insert sigh"></a>&nbsp;&nbsp;
                        <br />
                        
                        <textarea cols="60" rows="6" wrap="virtual" name="message" style="background-color:##eeeeee;border:1px solid ##200057;">#msg#</textarea>
                        <br /><br />
                        <cfif isDefined("url.action") AND url.action EQ "topic">
                          Email me when somebody replies to this topic: <input type="checkbox" name="subscribe" checked>Yes<br /><br />
                          <input type="button" value="cancel" name="bCancel" onClick="location.href='forums.cfm?forum=#messages.forumid#'">&nbsp;&nbsp;
                          <input type="button" value="preview" name="bPreview" onClick="showPreview();">&nbsp;&nbsp;
                          <input type="submit" value="submit topic" name="bSubmit"> <br /><br />
                        <cfelse>
                          <input type="hidden" name="notifyauthor" value="<cfif isDefined("messages.notifyauthor") AND messages.notifyauthor EQ 1>1<cfelse>0</cfif>">
                          <input type="hidden" name="notifyemail" value="<cfif isDefined("messages.notifyemail") AND Len(messages.notifyemail) GT 
                              0>#messages.notifyemail#<cfelse></cfif>">
                          <input type="button" value="cancel" name="bCancel" 
                           onClick="location.href='topics.cfm?forum=#messages.forumid#&topic=#messages.topicid#'">&nbsp;&nbsp;
                          <input type="button" value="preview" name="bPreview" onClick="showPreview();">&nbsp;&nbsp;
                          <cfif EditMode EQ "edit">
                            <input type="submit" value="make changes" name="bSubmit">&nbsp;&nbsp;
                          <cfelse>
                            <input type="submit" value="send reply" name="bSubmit">&nbsp;&nbsp;
                          </cfif>
                          
                          Formatting: <select name="encoding">
                            <option value="2">BBCode</option>
                            <option value="1">HTML</option>
                            <option value="4">None</option>
                          </select>
                          <br /><br />
                        </cfif>
                        
                        </form>
                      </td>
                    </tr>
                  </cfoutput>
                  <div name="preview"></div>
                  <cfif editMode EQ "post">
                    <cfloop query="messages" startrow="1" endrow="#messages.recordcount#">
                      <cfoutput>
                        <tr>
                          <td width="25%" bgcolor="##EFEFEF" align="center" valign="top" style="border-top:1px solid ##cccccc;border-left:1px solid ##cccccc;border-right:1px solid 
                             ##cccccc;padding-top:15px;<cfif messages.currentrow EQ messages.recordcount>border-bottom:1px solid ##cccccc</cfif>">
                            <p><b>#messages.username#</b><br />
                            Total Posts: #posts#<br />
                            Member Since: #DateFormat(JoinDate, "mmm yyyy")#</p>
                          </td>
                          <td width="75%" bgcolor="##EFEFEF" align="left" style="padding:10px;border-top:1px solid ##cccccc;border-right:1px solid ##cccccc;padding-top:15px;<cfif 
                                messages.currentrow EQ messages.recordcount>border-bottom:1px solid ##cccccc</cfif>" class="forumTopic">
                            <b>Posted</b>: #DateFormat(messages.messagedate, "mmm dd, yyyy")#<br /><br />
                            <cfif messages.format EQ 1>
                              #smileyDecode(messages.messagecopy)#
                            <cfelseif messages.format EQ 2>
                              #bbDecode(messages.messagecopy)#
                            <cfelseif messages.format EQ 4>
                              #deHTML(messages.messagecopy)#
                            </cfif>
                          </td>
                        </tr>
                      </cfoutput>
                    </cfloop>
                  </cfif>
                  
                <cfelse>
                  <tr bgcolor="#ffffff">
                    <td>
                      <table cellpadding='4' cellspacing='1' border='0' width='100%' height="225">
                        <tr>
                          <td width="100%" align="center" valign="middle"><p><b>Sorry, you must be logged in to reply to topics.</b><br /><br />
                            If you alread have an account, you may log in <a href="userlogin.cfm?RETURNURL=<cfoutput>#URLEncodedFormat(myurl)#</cfoutput>" class="gear"><b>here</b></a>.<br 
                             /><br />
                            If you don't have an account, you may create one <a href="account.cfm" class="gear"><b>here</b></a>.
                          </td>
                        </tr>
                        
                      </cfif>
                      
                    </table>
                    
                    <tr bgcolor="#ffffff"><td>&nbsp;</td></tr>
                    
                  </table>
                  <cfinclude template="includes/footer.cfm">
                <cfelse>
                  <cflocation addtoken="no" url="index.cfm">
                </cfif>
