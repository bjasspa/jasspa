<!--#INCLUDE FILE=../Localization/StringTable_EDR.bas-->
<!--#INCLUDE FILE=..\constants.bas-->
<!--#INCLUDE FILE=browserspecific.asp-->

<%Selection=Request.QueryString("selection")%>

<SCRIPT LANGUAGE=VBScript RUNAT=Server>
</script>


<%Set objgrid=Session("KeepGrid")
Select Case Selection
    Case "startup"
      objgrid.AddColumns "Repository,Part No,Version,KeyType,Description,Timestamp,File,Folder,Thumbnail"
      objgrid.SetRelativeWidths 20,15,5,5,25,5,10,10,5
      objgrid.SetVisible "Part No,Version"
      objgrid.SetPage "detailContent.asp?selection=rowselect"
      
      Session("GridDisplay") = "aslist"
      
      Session("RepositoryChecked") = ""
      Session("DescriptionChecked") = "checked"
      Session("KeyTypeChecked") = "checked"
      Session("TimestampChecked") = "checked"
      Session("FileChecked") = "checked"
      Session("FolderChecked") = "checked"
    
   Case "fillgrid"
      Set EDRAdmin=Session("KeepEDRAdmin")   
      objgrid.ClearRows
      columns = "{repository},{key},{subkey},{keytype},{description},{timestamp},{file},{folder},,"
      objgrid.SetID(EDRAdmin.LastSelectedID())
      objgrid.Fill(EDRAdmin.Query(Session("edr_qry_keytype"),Session("edr_qry_partnum"),Session("edr_qry_version"),Session("edr_qry_description"),columns))
      Session("GridContentDisplayed") = "true"
      rn=Request.QueryString("rnum")
      
      if(Session("ThumbnailChecked") = "checked") then
         thumbnailStatus = EDRAdmin.QueryExists(Session("edr_qry_partnum"),Session("edr_qry_version"),Session("edr_qry_description"),"{thumbnail},")
         objgrid.FillColumn "Thumbnail",thumbnailStatus
      end if
      
    Case "pagesize"
      objgrid.SetPageSize Session("PageSetting")

    Case "aslist"
      objgrid.SetVisible "Part No,Version"
      
    Case "details"
      Details = "Part No,Version"
      
      if(Session("DescriptionChecked") = "checked") then
        Details = Details & ",Description"
      end if
      
      if(Session("RepositoryChecked") = "checked") then
        Details = Details & ",Repository"
      end if   
      
      if(Session("KeyTypeChecked") = "checked") then
        Details = Details & ",KeyType"
      end if
      
      if(Session("FileChecked") = "checked") then
        Details = Details & ",File"
      end if
      
      if(Session("FolderChecked") = "checked") then
        Details = Details & ",Folder"
      end if
      
      if(Session("TimestampChecked") = "checked") then
        Details = Details & ",Timestamp"
      end if
      
      if(Session("ThumbnailChecked") = "checked") then
        Details = Details & ",Thumbnail"
      end if
      objgrid.SetVisible Details


   Case "selectall"
      objgrid.SelectAll
      %><SCRIPT Language="JavaScript">
         parent.frames[NavFrame].location = "navigate.asp?selection=defocustree"
      </SCRIPT><%

   Case "invertselection"
      objgrid.InvertSelection
      if(objgrid.IsSelected) then
       %><SCRIPT Language="JavaScript">
         parent.frames[NavFrame].location = "navigate.asp?selection=defocustree"
       </SCRIPT><%       
      else
       %><SCRIPT Language="JavaScript">
         parent.frames[NavFrame].location = "navigate.asp?selection=focustree"
       </SCRIPT><%       
       
      end if

   Case "show"
      Session("GridContentDisplayed") = "true"
      if(objgrid.Count > 0) then%>
      <SCRIPT Language="JavaScript">
        parent.frames[DlgFrame].location = "dialog.asp?selection=details&action=editable"
      </SCRIPT>
      <%end if

    
   Case "hide"
      objgrid.DeSelectAll
      Session("GridContentDisplayed") = "false"
      %><SCRIPT Language="JavaScript">
        parent.frames[DlgFrame].location = "dialog.asp?selection=details&action=noteditable"
      </SCRIPT><%
      
   Case "cut"
      objgrid.CutSelected()

   Case else
End Select
if(Session("GridContentDisplayed") = "false") then
  Selection = "hide"
end if%>

<HTML>

<HEAD>
<TITLE> EDR dialogs </TITLE>


<FRAMESET ROWS="35,*,35" FRAMEBORDER="no">
 <FRAME NAME="detailheader" SRC="<%=EDR_PATH%>detailHeader.asp" BORDER="no" MARGINWIDTH="0" SCROLLING="no">
 <FRAME NAME="detailcontent" SRC="<%=EDR_PATH%>detailContent.asp?selection=<%=Selection%>&rnum=<%=rn%>" BORDER="no" MARGINWIDTH="0">
 <FRAME NAME="detailsummary" SRC="<%=EDR_PATH%>detailSummary.asp?selection=hide" BORDER="no" SCROLLING="no" MARGINWIDTH="0">
</frameset>




</HTML>

