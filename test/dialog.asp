<!--#INCLUDE FILE=../Localization/StringTable_EDR.bas-->
<!--#INCLUDE FILE=..\constants.bas-->
<!--#INCLUDE FILE=browserspecific.asp-->


<SCRIPT LANGUAGE=VBScript RUNAT=Server>

Function Substr(strng,sta,delim)
  
   p1 = instr(sta,strng,delim)
   if(p1 < sta) then
     p1 = len(strng)+1
   end if  
   Substr = mid(strng,sta,p1-sta)
   sta = p1+1
   
end Function
 
 
Sub ShowFilter

   If Session("edr_qry_keytype") = "" then
      Session("edr_qry_keytype") = "*"
   end if
   If(Session("edr_qry_partnum")) = "" then
      Session("edr_qry_partnum") = "*"
   end if
   If(Session("edr_qry_version")) = "" then
      Session("edr_qry_version") = "*"
   end if
   If(Session("edr_qry_description")) = "" then
      Session("edr_qry_description") = "*"
   end if
response.write "<form  method=""POST"" action=""dialog.asp?selection=actions&action=filter"">"
response.write "<TABLE BGCOLOR=""#FFFFFF""  WIDTH=100% CELLSPACING=0 CELLPADDING=5><TR><TD>"
response.write "<TABLE BGCOLOR=""#FFFFFF""  WIDTH=100% CELLSPACING=0 CELLPADDING=0>"
response.write "<TR VALIGN=""top""><TD><img src=""" & IMAGE_PATH & "nothing.gif"" height=8 width=1 border=0></td></tr>"
response.write "<TR VALIGN=""top""><TD><FONT FACE=""Arial, Sans Serif"" SIZE=""-1"" ><B><small>" & LABEL_KEYTYPE & "</small></B></FONT></TD>"
response.write "<TD COLSPAN=2><FONT FACE=""Arial, Sans Serif"" SIZE=""-1"" ><INPUT TYPE=""text"" NAME=""Keytype"" size=17 VALUE=" & Session("edr_qry_keytype") & " ></FONT></TD></TR>"
response.write "<TR VALIGN=""top""><TD><FONT FACE=""Arial, Sans Serif"" SIZE=""-1"" ><B><small>" & LABEL_DESCRIPTION & "</small></B></FONT></TD>"
response.write "<TD COLSPAN=2><FONT FACE=""Arial, Sans Serif"" SIZE=""-1"" ><INPUT TYPE=""text"" NAME=""Description"" size=17 VALUE=" & Session("edr_qry_description") & " ></FONT></TD></TR>"
response.write "<TR VALIGN=""top""><TD><FONT FACE=""Arial, Sans Serif"" SIZE=""-1"" ><B><small>" & LABEL_PARTNO & "</small></B></FONT></TD>"
response.write "<TD COLSPAN=2><FONT FACE=""Arial, Sans Serif"" SIZE=""-1"" ><INPUT TYPE=""text"" NAME=""PartNum"" size=17 VALUE=" & Session("edr_qry_partnum") & " ></FONT></TD></TR>"
response.write "<TR VALIGN=""top""><TD><FONT FACE=""Arial, Sans Serif"" SIZE=""-1"" ><B><small>" & LABEL_VERSION &"</small></B></FONT></TD>"
response.write "<TD><FONT FACE=""Arial, Sans Serif"" SIZE=""-1""><INPUT TYPE=""text"" NAME=""Version"" size=6 VALUE=" & Session("edr_qry_version") & " ></FONT></TD>"
response.write "<TD><FONT FACE=""Arial, Sans Serif"" SIZE=""-1"" ><INPUT TYPE=""submit"" NAME=""searchbutton"" VALUE=""search""></FONT></TD></TR>"
response.write "</TABLE> </td></tr></table></form>"
end Sub
 
 

</SCRIPT>
 
<%

Set MenuBar=Session("KeepMenuBar")
Set EDRAdmin=Session("KeepEDRAdmin")

Selection=Request.QueryString("selection")
Select Case Selection
    
Case "startup"
  

  MenuBar.Add "File",0,"dialog.asp?selection=menus&menu=file"
  MenuBar.Add "Wizard",1,"dialog.asp?selection=actions&action=wizard"
  MenuBar.Add "Exit",1,"dialog.asp?selection=actions&action=exit"

  MenuBar.Add "Edit",0,"dialog.asp?selection=menus&menu=edit"
  MenuBar.Add "Cut",1,"dialog.asp?selection=actions&action=cut"
  MenuBar.Add "Copy",1,"dialog.asp?selection=actions&action=copy"
  MenuBar.Add "Paste",1,"dialog.asp?selection=actions&action=paste"
  MenuBar.Add "Select All",1,"dialog.asp?selection=actions&action=selectall"
  MenuBar.Add "Invert Selection",1,"dialog.asp?selection=actions&action=invertselection"

  MenuBar.Add "View",0,"dialog.asp?selection=menus&menu=view"
  MenuBar.Add "As List",1,"dialog.asp?selection=actions&action=aslist"
  MenuBar.Add "Details",1,"dialog.asp?selection=actions&action=details"
  MenuBar.Add "Settings...",1,"dialog.asp?selection=dialogs&dialog=settings"
  MenuBar.Add "Configuration",1,"dialog.asp?selection=actions&action=wizard"
  'MenuBar.Add "Recycle Bin",1," "

  MenuBar.Add "Repository",0,"dialog.asp?selection=menus&menu=repository"
  MenuBar.Add "Add...",1,"dialog.asp?selection=dialogs&dialog=add"
  MenuBar.Add "Rename...",1,"dialog.asp?selection=dialogs&dialog=rename"

  MenuBar.Add "Objects",0,"dialog.asp?selection=menus&menu=objects"
  MenuBar.Add "Purge Versions...",1,"dialog.asp?selection=dialogs&dialog=purgeversions"
  MenuBar.Add "Purge By Date",1,"dialog.asp?selection=dialogs&dialog=purgedates"
  MenuBar.Add "Thumbnails...",1,"dialog.asp?selection=dialogs&dialog=thumbnails"
  MenuBar.Add "Edit Details...",1,"dialog.asp?selection=dialogs&dialog=editdetails"
  MenuBar.Add "Report Matrix links",1,"dialog.asp?selection=plugins&action=mqlreport"
  MenuBar.Add "Fix Matrix links",1,"dialog.asp?selection=plugins&action=mqlfix"
  
  MenuBar.Disable("Edit")
  MenuBar.Disable("Copy")
  MenuBar.Disable("Paste")
  MenuBar.Disable("Select All")
  MenuBar.Disable("Invert Selection")
  
  MenuBar.Check("As List")
  MenuBar.Disable("Rename...")
  MenuBar.Disable("Recycle Bin")
  MenuBar.Disable("Purge By Date")
  MenuBar.Disable("Edit Details...")
  Response.write(MenuBar.Show)
  ShowFilter
  Session("DialogUp") = ""
%>

<HTML>
<HEAD>
<TITLE> EDR dialogs </TITLE>
</head>
   <BODY BGCOLOR="#FFFFFF" LEFTMARGIN=0 TOPMARGIN=0 BORDER = "yes">
       
<%Case "plugins"

   if not (EDRAdmin.InitPlugin("MQLAdm.MQLAdmin")) then
      response.write "Error initializing MQLAdmin plugin"
   end if
    
   Action=Request.QueryString("action")
   Select Case Action
     
   Case "mqlreport"
       Set objgrid = Session("KeepGrid")
       If(objgrid.isSelected()) then
         EDRAdmin.CallPlugin objgrid.GetSelected("Repository,Part No,Version"), "MQLReport","c:\TEMP\MQLReport.txt"         
         'EDRAdmin.CallPlugin objgrid.GetSelected("Repository,Part No,Version"), "Batch", "MQLReport","c:\TEMP\MQLReport.txt"
       else
        'EDRAdmin.CallPlugin "*", "Batch", "MQLReport","c:\TEMP\MQLReport.txt"
        EDRAdmin.CallPlugin "*", "MQLReport","c:\TEMP\MQLReport.txt"
       end if
   Case "mqlfix"
       Set objgrid = Session("KeepGrid")
       If(objgrid.isSelected()) then
          EDRAdmin.CallPlugin objgrid.GetSelected("Repository,Part No,Version"), "MQLFix"
          'EDRAdmin.CallPlugin objgrid.GetSelected("Repository,Part No,Version"), "Batch", "MQLFix"
       else
          EDRAdmin.CallPlugin "*", "Batch", "MQLFix"
          'EDRAdmin.CallPlugin "*", "MQLFix"
       end if
   Case else
       
   End Select
   Response.write(MenuBar.Show)
   ShowFilter%>
   
   <TABLE WIDTH=100% BGCOLOR="#FFFFFF">
   <TR><TD>
   <A HREF=dialog.asp?selection=menus><img src="<%=IMAGE_PATH%>nothing.gif" height=300 width=300 border=0></a>
   </td></tr>
   </table>
  

<%Case "menus"
  
   Menu=Request.QueryString("menu")%>
          


   <%Select Case Menu
   Case "file"
      Response.write (MenuBar.ShowMenu ("File") )

   Case "edit"
      Response.write(MenuBar.ShowMenu ("Edit"))

   Case "view"
      Response.write(MenuBar.ShowMenu ("View"))
    
   Case "repository"   
      Response.write(MenuBar.ShowMenu ("Repository"))
    
   Case "objects"  
      Response.write(MenuBar.ShowMenu ("Objects"))

   Case else
      Response.write(MenuBar.Show)
      ShowFilter
   End Select%>
   <TABLE WIDTH=100% BGCOLOR="#FFFFFF">
   <TR><TD>
   <A HREF=dialog.asp?selection=menus><img src="<%=IMAGE_PATH%>nothing.gif" height=300 width=300 border=0></a>
   </td></tr>
   </table>


<%Case "dialogs"
   Dialog=Request.QueryString("dialog")
   if Dialog = "dialogalreadyup" then
     Dialog = Session("DialogUp")
   end if%>
   <%Select Case Dialog%>
   
       <%Case "add"
          node = EDRAdmin.GetDBName(EDRAdmin.LastSelectedID)
          if(node = "*") then %>
            <%Button = Request.Form("button")
            if(Button = "Add") then
               RepName = Request.Form("repname")
               EDRAdmin.AddDatabase(RepName)
               %><SCRIPT Language="JavaScript">
                  parent.frames[NavFrame].location = "navigate.asp"
               </SCRIPT><%
            end if  
            if(Button = "Close") then
               Session("DialogUp")=""
               response.write(MenuBar.show())
               ShowFilter
            else%>
              <%Session("DialogUp") = "add"%>
               <FORM  method="POST" action="dialog.asp?selection=dialogs&dialog=add">
               <TABLE BORDER BGCOLOR="<%=STATUS_BGCOLOR%>"  WIDTH=100% CELLSPACING=0 CELLPADDING=0>
               <TR><TD>
               <TABLE BGCOLOR="<%=STATUS_BGCOLOR%>"  WIDTH=100% CELLSPACING=0 CELLPADDING=<%=DIALOGCELLPADDING%>>
                  <TR VALIGN="top">
                  <TD COLSPAN=3><FONT FACE="<%=FONT_FACE%>"  SIZE="<%=FONT_SIZE%>" >Add Database to <B>EDR</b></font></td></tr>
               <TR>
                  <TD><img src="<%=IMAGE_PATH%>nothing.gif" height=20 width=10 ></td></tr>
               <TR>
                  <TD><FONT FACE="<%=FONT_FACE%>"  SIZE="<%=FONT_SIZE%>" ><B>Database Name:</B></font></td>
                  <TD COLSPAN=2><input type="input" name="repname" size=12></td></tr>
               <TR>
                  <TD><PRE></pre></td>
                  <TD><input type="submit" name="button" value="Add"  ></td>
                  <TD><input type="submit" name="button"  value="Close"></td></tr>
               </table></td></tr></table>
             <%end if%>
          <%else%>
             <%Button = Request.Form("button")
             if(Button = "Add") then
               RepName = Request.Form("repname")
               EDRAdmin.AddProject node,RepName
               %><SCRIPT Language="JavaScript">
                  parent.frames[NavFrame].location = "navigate.asp"
               </SCRIPT><%
             end if  
             if(Button = "Close") then
                Session("DialogUp")=""
                response.write(MenuBar.show())
                ShowFilter
             else%>
               <%Session("DialogUp") = "add"%>
                <FORM  method="POST" action="dialog.asp?selection=dialogs&dialog=add">
                <TABLE BORDER BGCOLOR="<%=STATUS_BGCOLOR%>"  WIDTH=100% CELLSPACING=0 CELLPADDING=0>
                <TR><TD>
                <TABLE BGCOLOR="<%=STATUS_BGCOLOR%>"  WIDTH=100% CELLSPACING=0 CELLPADDING=<%=DIALOGCELLPADDING%>>
                   <TR VALIGN="top">
                   <TD COLSPAN=3><FONT FACE="<%=FONT_FACE%>"  SIZE="<%=FONT_SIZE%>" >Add Project to <B><%=node%></b></font></td></tr>
                <TR>
                   <TD><img src="<%=IMAGE_PATH%>nothing.gif" height=20 width=10 ></td></tr>
                <TR>
                   <TD><FONT FACE="<%=FONT_FACE%>"  SIZE="<%=FONT_SIZE%>" ><B>Project Name:</B></font></td>
                   <TD COLSPAN=2><input type="input" name="repname" size=12></td></tr>
                <TR>
                   <TD><PRE></pre></td>
                   <TD><input type="submit" name="button" value="Add"  ></td>
                   <TD><input type="submit" name="button"  value="Close"></td></tr>
                </table></td></tr></table>
             <%end if%>
          <%end if%>
         
        <%Case "rename"%>
             <%Button = Request.Form("button")
             if(Button = "Rename") then
                EDRAdmin.Rename EDRAdmin.LastSelectedID(), Request.Form("newname")
               %><SCRIPT Language="JavaScript">
                  parent.frames[NavFrame].location = "navigate.asp"
               </SCRIPT><%
             end if  
             if(Button = "Close") then
               Session("DialogUp")=""
               response.write(MenuBar.show())
               ShowFilter
             else%>
               <%Session("DialogUp") = "rename"%>
                <FORM  method="POST" action="dialog.asp?selection=dialogs&dialog=rename">
                <TABLE BORDER BGCOLOR="<%=STATUS_BGCOLOR%>"  WIDTH=100% CELLSPACING=0 CELLPADDING=0>
                <TR><TD>
                <TABLE BGCOLOR="<%=STATUS_BGCOLOR%>"  WIDTH=100% CELLSPACING=0 CELLPADDING=<%=DIALOGCELLPADDING%>>
                   <TR VALIGN="top">
                   <TD COLSPAN=3><FONT FACE="<%=FONT_FACE%>"  SIZE="<%=FONT_SIZE%>" >Rename Repository:</font></td></tr>
                <TR>
                   <TD><img src="<%=IMAGE_PATH%>nothing.gif" height=20 width=10 ></td></tr>
                <TR>
                   <TD><FONT FACE="<%=FONT_FACE%>"  SIZE="<%=FONT_SIZE%>" ><B>Old Name:</B></font></td>
                   <TD><FONT FACE="<%=FONT_FACE%>"  SIZE="<%=FONT_SIZE%>" ><%=EDRAdmin.GetNodeName(EDRAdmin.LastSelectedID())%></font></td>
                <TR>
                   <TD><FONT FACE="<%=FONT_FACE%>"  SIZE="<%=FONT_SIZE%>" ><B>New Name:</B></font></td>
                   <TD COLSPAN=2><input type="input" name="newname" size=12></td></tr>
                <TR>
                   <TD><PRE></pre></td>
                   <TD><input type="submit" name="button" value="Rename"  ></td>
                   <TD><input type="submit" name="button"  value="Close"></td></tr>
                </table></td></tr></table>
             <%end if%>
           
           
        <%Case "settings"%>
           <%Button1 = Request.Form("button1")
           Button2 = Request.Form("button2")
           if(Button1 = "Apply") then
             
              if(Request.Form("repository") = "true") then
                Session("RepositoryChecked") = "checked"
              else
                Session("RepositoryChecked") = ""
              end if
              if(Request.Form("keytype") = "true") then
                Session("KeyTypeChecked") = "checked"
              else
                Session("KeyTypeChecked") = ""
              end if
              if(Request.Form("timestamp") = "true") then
                Session("TimestampChecked") = "checked"
              else
                Session("TimestampChecked") = ""
              end if
              if(Request.Form("file") = "true") then
                Session("FileChecked") = "checked"
              else
                Session("FileChecked") = ""
              end if
              if(Request.Form("folder") = "true") then
                Session("FolderChecked") = "checked"
              else
                Session("FolderChecked") = ""
              end if
              PreThumbnailChecked = Session("ThumbnailChecked")
              if(Request.Form("thumbnail") = "true") then
                Session("ThumbnailChecked") = "checked"
              else
                Session("ThumbnailChecked") = ""
              end if
              if(Session("GridContentDisplayed") = "true") then
                if(PreThumbnailChecked = "" and Session("ThumbnailChecked") = "checked") then
                   thumbnailStatus = EDRAdmin.QueryExists(Session("edr_qry_partnum"),Session("edr_qry_version"),Session("edr_qry_description"),"{thumbnail},")
                   set objgrid = Session("KeepGrid")
                   objgrid.FillColumn "Thumbnail",thumbnailStatus
                end if
              end if
              if(Session("GridDisplay") = "details") then%>
                  <SCRIPT Language="JavaScript">
                    parent.frames[DetFrame].location = "details.asp?selection=details"
                  </SCRIPT>
               <%end if
           elseif(Button2 = "Apply") then
             if(not isNumeric(Request.Form("pagesize"))) then
                Session("PageSetting") = 0
             elseif((Request.Form("paging") = "default") or (Request.Form("pagesize") < 1)) then
                Session("PageSetting") = 0
             else
                Session("PageSetting") = Request.Form("pagesize")
             end if
             Button1 = "Page Size..."
             %><SCRIPT Language="JavaScript">
               parent.frames[DetFrame].location = "details.asp?selection=pagesize"
             </SCRIPT><%
           end if
             
            if(Button1 = "Close") then
               Session("DialogUp")=""
               response.write(MenuBar.show())
               ShowFilter
               
            elseif(Button1 = "Page Size...") then
               Session("DialogUp")="settings"
               if( Session("PageSetting") = 0) then
                 DefaultChecked = "checked"
                 UserChecked = ""
                 PageSize = ""
               else
                 DefaultChecked = ""
                 UserChecked = "checked"
                 PageSize = Session("PageSetting")
               end if%>                
               <FORM  method="POST" action="dialog.asp?selection=dialogs&dialog=settings">
               <TABLE BORDER BGCOLOR="<%=STATUS_BGCOLOR%>"  WIDTH=100% CELLSPACING=0 CELLPADDING=0>
               <TR><TD>
               <TABLE BGCOLOR="<%=STATUS_BGCOLOR%>"  WIDTH=100% CELLSPACING=0 CELLPADDING=<%=DIALOGCELLPADDING%>>
               <TR VALIGN="top">
                  <TD COLSPAN=3><FONT FACE="<%=FONT_FACE%>"  SIZE="<%=FONT_SIZE%>" >Page Size:</font></td></tr>
               <TR>
                  <TD><img src="<%=IMAGE_PATH%>nothing.gif" height=5 width=10 ></td></tr>
               <TR>
               <TD><input type="radio" name="paging" value="default" <%=DefaultChecked%>> Default</td></tr>
               <TR>
               <TD><input type="radio" name="paging" value="user" <%=UserChecked%>> User Defined</td></tr>
               <TR>
               <TD COLSPAN=3>Show <input type="input" name="pagesize" size=4 value=<%=PageSize%>> Parts per page</td></tr>
               <TR>
                <TD><PRE></pre></td>
                   <TD><input type="submit" name="button2" value="Apply"  ></td>
                   <TD><input type="submit" name="button2"  value="Close"></td></tr>
                </table></td></tr></table>
                
            <%else
                Session("DialogUp")="settings"%>
                <FORM  method="POST" action="dialog.asp?selection=dialogs&dialog=settings">
                <TABLE BORDER BGCOLOR="<%=STATUS_BGCOLOR%>"  WIDTH=100% CELLSPACING=0 CELLPADDING=0>
                <TR><TD>
                <TABLE BGCOLOR="<%=STATUS_BGCOLOR%>"  WIDTH=100% CELLSPACING=0 CELLPADDING=<%=DIALOGCELLPADDING%>>
                   <TR VALIGN="top">
                   <TD COLSPAN=4><FONT FACE="<%=FONT_FACE%>"  SIZE="<%=FONT_SIZE%>" >View Settings:</font></td></tr>
                <TR>
                    <TD><img src="<%=IMAGE_PATH%>nothing.gif" height=5 width=10 ></td></tr>
                <TR>
                    <TD colspan=2><input  type="checkbox" name="repository" value="true" <%=Session("RepositoryChecked")%> > Repository</td>
                    <TD colspan=2><input  type="checkbox" name="keytype" value="true" <%=Session("KeyTypeChecked")%> > Key Type</td></tr>
                <TR>
                    <TD colspan=2><input type="checkbox" name="file" value="true" <%=Session("FileChecked")%>> File</td>
                    <TD colspan=2><input type="checkbox" name="folder" value="true" <%=Session("FolderChecked")%>> Folder</td></tr>
                <TR>
                    <TD colspan=2><input type="checkbox" name="timestamp" value="true" <%=Session("TimestampChecked")%>> Timestamp</td>
                    <TD colspan=2><input type="checkbox" name="thumbnail" value="true" <%=Session("ThumbnailChecked")%>> Thumbnail</td></tr>
                <TR>
                   <TD colspan=2><input type="submit" name="button1" value="Page Size..."></td>
                   <TD><input type="submit" name="button1" value="Apply"  ></td>
                   <TD><input type="submit" name="button1"  value="Close"></td></tr>
                </table></td></tr></table>
             <%end if%>
              
           <%Case "purgeversions"%>
            <%Button = Request.Form("button")
            if(Button = "Purge") then
               Session("SortOption") = Request.Form("sortoption")
               Session("KeepNLatest") = Request.Form("keepoption")
               if(Session("SortOption") = 2) then
                 'change this string to point at your class that contains
                 'the method Compare(Version1 as String, Version2 as String) as boolean
                 'returning true when Version1 < Version2
                   SortClass = "AdminExtensions.PurgeComparison"
               else
                   SortClass = Session("SortOption")
               end if
               EDRAdmin.PurgeVersions Session("KeepNLatest"), SortClass
               Session("GridRefresh") = Session("GridRefresh") + 1
               %><SCRIPT Language="JavaScript">
                  parent.frames[DetFrame].location = "details.asp?selection=fillgrid&rnum=<%=Session("GridRefresh")%>"
                  parent.frames[NavFrame].location = "navigate.asp?selection=focustree"
               </SCRIPT><%

            end if  
            if(Button = "Close") then
               Session("DialogUp") = ""
               response.write(MenuBar.show())
               ShowFilter
            else%>
               <%Session("DialogUp") = "purgeversions"%>
               <FORM  method="POST" action="dialog.asp?selection=dialogs&dialog=purgeversions">
               <TABLE BORDER BGCOLOR="<%=STATUS_BGCOLOR%>"  WIDTH=100% CELLSPACING=0 CELLPADDING=0>
               <TR><TD>
               <TABLE BGCOLOR="<%=STATUS_BGCOLOR%>"  WIDTH=100% CELLSPACING=0 CELLPADDING=<%=DIALOGCELLPADDING%>>
               <TR VALIGN="top">              
                  <TD COLSPAN=3><FONT FACE="<%=FONT_FACE%>"  SIZE="<%=FONT_SIZE%>" ><B>Purge Versions:</b></font></td></tr>
               <TR>
                  <TD><img src="<%=IMAGE_PATH%>nothing.gif" height=5 width=10 ></td></tr>
               <TR>
                  <TD><FONT FACE="<%=FONT_FACE%>"  SIZE="<%=FONT_SIZE%>" ><B>Sort:</B></font></td>
                  <TD COLSPAN=2>
                  <SELECT name="sortoption">
                  <%Dim ssel
                  ssel = Array("","","")
                  if(Session("SortOption") <> "") then
                     ssel(clng(Session("SortOption"))) = "selected"
                  else
                     ssel(0) = "selected"
                  end if%> 
                  <OPTION value=0  <%=ssel(0)%>> Numerically </option>      
                  <OPTION value=1  <%=ssel(1)%>> AlphaNumerically </option>      
                  <OPTION value=2  <%=ssel(2)%>> User Supplied </option>      
                  </select>             
                  
                  </td></tr>
               <TR>
                  <TD><FONT FACE="<%=FONT_FACE%>"  SIZE="<%=FONT_SIZE%>" ><B>Delete:</B></font></td></tr>
               <TR>
                  <TD COLSPAN=3>
                  <SELECT name="keepoption">
                  <%Dim sel
                  sel = Array("","","")
                  if(Session("keepNLatest") <> "") then
                     sel(clng(Session("keepNLatest"))) = "selected"
                  else
                     sel(1) = "selected"
                  end if%>
                  <OPTION value=1 <%=sel(1)%>>All Except Current Version </option>      
                  <OPTION value=2 <%=sel(2)%>>All Except Current & Previous Version </option>      
                  <OPTION value=0 <%=sel(0)%>>All Versions </option>      
                  </select>            
                  </td>
               
               <TR>
                  <TD><PRE></pre></td>
                  <TD><input type="submit" name="button" value="Purge"></td>
                  <TD><input type="submit" name="button"  value="Close"></td></tr>
               </table></td></tr></table>
            <%end if%>
        <%Case "editdetails"%>
            <%Button = Request.Form("button")
            set objgrid = Session("KeepGrid")
            if(Button = "Apply") then
               RepName = Request.Form("repname")
               Key = Request.Form("key")
               Subkey = Request.Form("subkey")
               PartNo =  Request.Form("partno")
               Version = Request.Form("version")
               Description = Request.Form("description")
               if(EDRAdmin.EditObj(RepName, Key, Subkey, PartNo, Version, Description)) then
                  findColumns = "Repository,Part No,Version"
                  withValues = RepName & "," & Key & "," & Subkey
                  andChange = "Part No,Version,Description"              
                  toValues = PartNo & "," & Version & "," & Description                
                  objgrid.EditRow findColumns,withValues,andChange,toValues
               end if
               %><SCRIPT Language="JavaScript">
                  parent.frames[DetFrame].location = "details.asp?selection=refresh"
               </SCRIPT><%
            elseif(Button = "Close") then
               Details = objgrid.NextSelected()
               if(Details <> "") then
                 p = 1
                 RepName = Substr(Details,p,",")
                 PartNo = Substr(Details,p,",")
                 Version = Substr(Details,p,",")
                 Description =  Substr(Details,p,",")
                 Key = PartNo
                 Subkey = Version
            end if


            else
               Details = objgrid.NextSelected("Repository,Part No,Version,Description")
               if(Details <> "") then
                 p = 1
                 RepName = Substr(Details,p,",")
                 PartNo = Substr(Details,p,",")
                 Version = Substr(Details,p,",")
                 Description =  Substr(Details,p,",")
                 Key = PartNo
                 Subkey = Version
               end if
               
            end if
             
            if(Key = "") then
               Session("DialogUp")=""
               response.write(MenuBar.show())
               ShowFilter
              
             else
               Session("DialogUp") = "editdetails"%>
               <FORM  method="POST" action="dialog.asp?selection=dialogs&dialog=editdetails">
               <input type = "hidden" name="repname" value="<%=Repname%>">
               <input type = "hidden" name="key" value="<%=Key%>">
               <input type = "hidden" name="subkey" value="<%=Subkey%>">
               <TABLE BORDER BGCOLOR="<%=STATUS_BGCOLOR%>"  WIDTH=100% CELLSPACING=0 CELLPADDING=0>
               <TR><TD>
               <TABLE BGCOLOR="<%=STATUS_BGCOLOR%>"  WIDTH=100% CELLSPACING=0 CELLPADDING=2>
               <TR VALIGN="top">
                  <TD><FONT FACE="<%=FONT_FACE%>"  SIZE="<%=FONT_SIZE%>" >Edit Details:</font></td>
                  <TD COLSPAN =3> <SMALL> <%=Key%>:<%=Subkey%></small></td></tr>
               <TR>
                  <TD><img src="<%=IMAGE_PATH%>nothing.gif" height=10 width=10 ></td></tr>

               <TR>
                  <TD><FONT FACE="<%=FONT_FACE%>"  SIZE="<%=FONT_SIZE%>" >Part No:</font></td>
                  <TD COLSPAN=2><input type="input" name="partno" value="<%=PartNo%>" size=18></td></tr>
               <TR>
                  <TD><FONT FACE="<%=FONT_FACE%>"  SIZE="<%=FONT_SIZE%>" >Version:</font></td>
                  <TD COLSPAN=2><input type="input" name="version" value="<%=Version%>" size=18></td></tr>
               <TR>
                  <TD><FONT FACE="<%=FONT_FACE%>"  SIZE="<%=FONT_SIZE%>" >Description:</font></td></tr>
               <TR>   
                  <TD COLSPAN=4><input type="input" name="description"  value="<%=Description%>" size=32></textarea></td></tr>
               <TR>
                  <TD><PRE></pre></td>
                  <TD><input type="submit" name="button" value="Apply"  ></td>
                  <TD><input type="submit" name="button"  value="Close"></td></tr>
               </table></td></tr></table>
            <%end if%>

        <%Case "thumbnails"
           Button1 = Request.Form("button1")
           Button2 = Request.Form("button2")
           if(Button1 = "Create") then
              if(Request.Form("regen")="all") then
                 Session("AllChecked") = "checked"
                 ForceRegen = "1"
               else
                 Session("AllChecked") = ""
                 ForceRegen = "0"
              end if
              Set objgrid = Session("KeepGrid")
              Timeout = Request.Form("timeout")
              KeepScriptTimeout = server.scripttimeout
              if isNumeric(timeout) then
                 if Timeout > server.scripttimeout then
                    server.scripttimeout = Timeout + 5 
                 end if
                 EDRAdmin.SetParam "Thumbnail","maxjobtime",Timeout
              end if
              If(objgrid.IsSelected()) then
                 EDRAdmin.EnsureThumbnailsForObjs ForceRegen, objgrid.GetSelected("Repository,Part No,Version")
              else       
                 EDRAdmin.EnsureThumbnails ForceRegen 
              end if
              server.scripttimeout = KeepScriptTimeout 
              if(Session("ThumbnailChecked") = "checked") then
                 thumbnailStatus = EDRAdmin.QueryExists(Session("edr_qry_partnum"),Session("edr_qry_version"),Session("edr_qry_description"),"{thumbnail},")
                 objgrid.FillColumn "Thumbnail",thumbnailStatus
                 %><SCRIPT Language="JavaScript">
                    parent.frames[DetFrame].location = "details.asp?selection=refresh"
                 </SCRIPT><%
              end if
              
           elseif(Button2 = "Apply") then
              %>CHANGE SETTINGS<%
              Button1 = "Settings..."
           end if
             
            if(Button1 = "Close") then
               Session("DialogUp")=""
               response.write(MenuBar.show())
               ShowFilter
               
            elseif(Button1 = "Settings...") then
               Session("DialogUp")="thumbnails"
               %>                
               <FORM  method="POST" action="dialog.asp?selection=dialogs&dialog=thumbnails">
               <TABLE BORDER BGCOLOR="<%=STATUS_BGCOLOR%>"  WIDTH=100% CELLSPACING=0 CELLPADDING=0>
               <TR><TD>
               <TABLE BGCOLOR="<%=STATUS_BGCOLOR%>"  WIDTH=100% CELLSPACING=0 CELLPADDING=<%=DIALOGCELLPADDING%>>
               <TR VALIGN="top">
                  <TD COLSPAN=3><FONT FACE="<%=FONT_FACE%>"  SIZE="<%=FONT_SIZE%>" >Thumbnail Settings:</font></td></tr>
               <TR>
                  <TD><img src="<%=IMAGE_PATH%>nothing.gif" height=5 width=10 ></td></tr>
               <TR>
                <TD><PRE></pre></td>
                   <TD><input type="submit" name="button2" value="Apply"  ></td>
                   <TD><input type="submit" name="button2"  value="Close"></td></tr>
                </table></td></tr></table>
                
            <%else
                Session("DialogUp")="thumbnails" 
                if(Session("AllChecked") = "checked") then
                  Session("MissingChecked") = ""
                else
                  Session("MissingChecked") = "checked"
                end if
                %>
                <FORM  method="POST" action="dialog.asp?selection=dialogs&dialog=thumbnails">
                <TABLE BORDER BGCOLOR="<%=STATUS_BGCOLOR%>"  WIDTH=100% CELLSPACING=0 CELLPADDING=0>
                <TR><TD>
                <TABLE BGCOLOR="<%=STATUS_BGCOLOR%>"  WIDTH=100% CELLSPACING=0 CELLPADDING=<%=DIALOGCELLPADDING%>>
                   <TR VALIGN="top">
                   <TD COLSPAN=4><FONT FACE="<%=FONT_FACE%>"  SIZE="<%=FONT_SIZE%>" >Create Thumbnails:</font></td></tr>
                <TR>
                    <TD><img src="<%=IMAGE_PATH%>nothing.gif" height=5 width=10 ></td></tr>
                <TR>
                    <TD COLSPAN=4><input type="radio" name="regen" value="missing" <%=Session("MissingChecked")%>> Missing Thumbnails Only</td></tr>
               <TR>
                    <TD COLSPAN=4><input type="radio" name="regen" value="all" <%=Session("AllChecked")%>> Regenerate All</td></tr>
    
                <TR>
                    <TD COLSPAN=4>Timeout: <input type="input" name="timeout" size=4 value=<%=EDRAdmin.GetParam("Thumbnail","maxjobtime")%>> secs</td></tr>

                <TR>
                   <TD><PRE> </pre></td>
                   <TD><PRE> </pre></td>
                   <TD><input type="submit" name="button1" value="Create"  ></td>
                   <TD><input type="submit" name="button1"  value="Close"></td></tr>
                </table></td></tr></table>
          
            
             <%end if%>
              
        <%Case else%>

           GOT DIALOG SELECTION
         
   <%End Select%>
   </form>

<%Case "actions"
   Action=Request.QueryString("action")%>
 
   <%Select Case Action%>
       <%Case "exit"%>
         <SCRIPT Language="JavaScript">
         parent.location = "/eds/admin/admin.asp"
         </SCRIPT>       
       <%Case "wizard"%>
         <SCRIPT Language="JavaScript">
         parent.location = "/eds/admin/EDRConfig.asp"
         </SCRIPT>
         
       <%Case "cut"        
         MenuBar.Enable("Paste")
         Set objgrid = Session("KeepGrid")
         If(objgrid.IsSelected()) then
           if(EDRAdmin.CutObjs(objgrid.GetSelected("Repository,Part No,Version"))) then
               %><SCRIPT Language="JavaScript">
                  parent.frames[DetFrame].location = "details.asp?selection=cut"
                  parent.frames[NavFrame].location = "navigate.asp?selection=focustree"
                  </SCRIPT><%
           end if
         else
            ID = EDRAdmin.LastSelectedID()            
            EDRAdmin.Cut ID
            if(ID = EDRAdmin.GetDbName(ID)) then
               MenuBar.Disable("Rename...")
               MenuBar.Disable("Cut")
               MenuBar.Disable("Copy")
            end if
            gridID = objgrid.GetID
            if(gridID = ID or EDRAdmin.GetDbName(gridID) = ID or gridID = EDRAdmin.GetDbName(ID) or gridID = "*") then
              objgrid.ClearRows
            end if
            %><SCRIPT Language="JavaScript">
               parent.frames[NavFrame].location = "navigate.asp"
            </SCRIPT><%
         end if
          
       Case "copy"
         MenuBar.Enable("Paste")
         Set objgrid = Session("KeepGrid")
         If(objgrid.IsSelected()) then
           EDRAdmin.CopyObjs(objgrid.GetSelected("Repository,Part No,Version"))
         else
         
            EDRAdmin.Copy EDRAdmin.LastSelectedID() 
            %><SCRIPT Language="JavaScript">
               parent.frames[NavFrame].location = "navigate.asp"
            </SCRIPT><%
         end if
       Case "paste"           
         if(EDRAdmin.Paste(EDRAdmin.LastSelectedID()))then
            Set objgrid = Session("KeepGrid")
            if(objgrid.IsSelected()) then
              Session("GridRefresh") = Session("GridRefresh") + 1
              %><SCRIPT Language="JavaScript">
                  parent.frames[DetFrame].location = "details.asp?selection=fillgrid&rnum=<%=Session("GridRefresh")%>"
                  parent.frames[NavFrame].location = "navigate.asp?selection=focustree"
                </SCRIPT><%             
            else
               %><SCRIPT Language="JavaScript">
                 parent.frames[NavFrame].location = "navigate.asp"
               </SCRIPT><%
            end if
         end if
        
       Case "selectall"
          %><SCRIPT Language="JavaScript">
             parent.frames[DetFrame].location = "details.asp?selection=selectall"
          </SCRIPT><%
       Case "invertselection"
          %><SCRIPT Language="JavaScript">
             parent.frames[DetFrame].location = "details.asp?selection=invertselection"
          </SCRIPT><%
       Case "aslist"
         if(Session("GridDisplay") <> "aslist") then%>
            <SCRIPT Language="JavaScript">
               parent.frames[DetFrame].location = "details.asp?selection=aslist"
            </SCRIPT>
            <%
            MenuBar.UnCheck("Details")
            MenuBar.Check("As List")
            Session("GridDisplay")="aslist"
         end if%>
          
       <%Case "details"
         if(Session("GridDisplay") <> "details") then%>
            <SCRIPT Language="JavaScript">
               parent.frames[DetFrame].location = "details.asp?selection=details"
            </SCRIPT>
            <%
            MenuBar.UnCheck("As List")
            MenuBar.Check("Details")
            Session("GridDisplay")="details"
         end if%>
         
      <%Case "filter"


         If(Request.Form("Keytype") <> "") then 
            Session("edr_qry_keytype")=Request.Form("Keytype")
         end if
         If(Request.Form("PartNum") <> "") then 
            Session("edr_qry_partnum")=Request.Form("PartNum")
         end if
         If(Request.Form("Version") <> "") then
            Session("edr_qry_version")=Request.Form("Version")
         end if  
         If(Request.Form("Description") <> "") then
            Session("edr_qry_description")=Request.Form("Description")
         end if
         %><SCRIPT Language="JavaScript">
            parent.frames[DetFrame].location = "details.asp?selection=fillgrid"
            parent.frames[NavFrame].location = "navigate.asp?selection=focustree"
         </SCRIPT><%
         
      Case else %>
         DIDNT GET ACTION
      <%End Select%>
      <%Response.write(MenuBar.Show)
        ShowFilter%>

<%Case "navigate"
   Action=Request.QueryString("action")
   
   Select Case Action
       Case "edrselected"
          MenuBar.Disable("Rename...")
          MenuBar.Disable("Cut")
          MenuBar.Disable("Copy")
       Case "selected"
          MenuBar.Enable("Edit")
          MenuBar.Enable("Rename...")
          MenuBar.Enable("Cut")
          MenuBar.Enable("Copy")
       Case else
   End Select
   If(Session("DialogUp")<>"")then
         %><SCRIPT Language="JavaScript">
            parent.frames[DlgFrame].location = "dialog.asp?selection=dialogs&dialog=dialogalreadyup"
          </SCRIPT><%
   else
     Response.write(MenuBar.Show)
     ShowFilter
   end if%>
    
<%Case "details"
   Action=Request.QueryString("action")
   Select Case Action
     Case "editable"
          MenuBar.Enable("Edit")
          MenuBar.Enable("Cut")
          MenuBar.Enable("Edit Details...")
          MenuBar.Enable("Select All")
          MenuBar.Enable("Invert Selection")
       Case "noteditable"
          MenuBar.Disable("Edit Details...")
          MenuBar.Disable("Select All")
          MenuBar.Disable("Invert Selection")          
       Case Else
   End Select
   If(Session("DialogUp")<>"")then
         %><SCRIPT Language="JavaScript">
            parent.frames[DlgFrame].location = "dialog.asp?selection=dialogs&dialog=dialogalreadyup"
          </SCRIPT><%
   else
     Response.write(MenuBar.Show)
     ShowFilter
   end if
   
   
Case else%>
   DIDNT GET SELECTION
<%End Select%>
</html>
