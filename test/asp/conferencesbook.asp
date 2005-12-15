<%
  Response.Buffer=True
  userID=Request.Cookies("du")("userid")
%>
<html>
  <head>
    <title> Dental Update Conferences </title>
    <!-- #include file="./cgi-bin/adovbs.inc" -->
    <!-- #include file="./cgi-bin/connection.asp" -->
    <!-- #include file="./cgi-bin/scripts.asp" -->
    <!-- #include file="./cgi-bin/javascript.asp" -->
    <link rel="stylesheet" href="cgi-bin/style.asp" type="text/css">
  </head>
  <body>
  <%
    'INSTALL THE PALATE INFORMATION
    pallate=Request.Cookies("du")("pallate")
    If pallate=Empty Then
    colorPallate()
    End If
    
    pallate=Request.Cookies("du")("pallate")
    darkColor=Request.Cookies("du")("darkColor")
    middleColor=Request.Cookies("du")("middleColor")
    lightColor=Request.Cookies("du")("lightColor")
    darkGrey=Request.Cookies("du")("darkGrey")
    lightGrey=Request.Cookies("du")("lightGrey")
    
    
    'OPEN THE CONNECTION TO THE DATABASE
    DIM objConn, objRec, objCmd, sql
    Set objConn = Server.CreateObject ("ADODB.Connection")
    objConn.Open strDuDatabase
    
    'GET THE CPDISSUE VALUE
    cpdissue=Request.Cookies("du")("cpdissue")
    If cpdissue=EMPTY Then
    cpdissue=getcpdissue()
    End If
    
    'GET CONF RATE
    Set objRec = Server.CreateObject("ADODB.RecordSet")
    sql = "SELECT confrate, confratedis "
    sql=sql & "FROM subrate "
    objRec.Open sql, objConn, adOpenKeySet, Adlockreadonly, adCmdText
    On Error Resume Next
    objRec.moveFirst
    userExists=objRec.RecordCount
    If userExists>0 Then
confrate=Server.HTMLEncode(objRec.Fields("confrate").Value)
confratedis=Server.HTMLEncode(objRec.Fields("confratedis").Value)
End If
objRec.Close
Set objRec=Nothing

If LEN(userID)<2 Then
               signtext="signin"
               conferenceamount=confrate
               Else
               signtext="signout"
               conferenceamount=confratedis
               End If
               
               
             %>
             <!-- Top TABLE LOGO AND LEADING TEXT AND MENU-->
             <table width=100% cellpadding=0 cellspacing=0 border=0>
               <tr>
                 <td width=169>
                   <img src="./images/<%=pallate%>/du.gif" height="77" width="169" border="0"><br>
                 </td>
                 
                 <td valign="bottom">
                   <SPAN CLASS="smallbodytext">Dental Update is the leading journal for postgraduate and continuing professional development<br>
                   <img src="./images/<%=pallate%>/transparent.gif" height="8" width="1" border="0"><BR>
                 </td>
               </tr>
             </table>
             <table width=100% cellpadding=0 cellspacing=0 border=0>
               <tr>
                 <td bgcolor=#<%=darkColor%> valign="center" height="18">
<span class="smallbodytextwhite">
<A CLASS="smallbodytextwhite" href="./default.asp">HOME</a> |&nbsp;
<a CLASS="smallbodytextwhite" href="./questionscpd.asp">CPD</a> |&nbsp;
<a CLASS="smallbodytextwhite" href="./questionssa.asp">SELF ASSESSMENT</a>&nbsp;&nbsp;|&nbsp;&nbsp;
<a CLASS="smallbodytextwhite" href="./articles.asp">ARTICLES</a> |&nbsp;
<a CLASS="smallbodytextwhite" href="./reviews.asp">REVIEWS</a> |&nbsp;
<a CLASS="smallbodytextwhite" href="./cpdlog.asp">CPD LOG</a> |&nbsp;
<a CLASS="smallbodytextwhite" href="./messages.asp">MESSAGES</a> |&nbsp;
<a CLASS="smallbodytextwhite" href="./certificates.asp">CERTIFICATES</a> |&nbsp;
<a CLASS="smallbodytextwhite" href="./conferences.asp">CONFERENCES</a> |&nbsp;
<a CLASS="smallbodytextwhite" href="./bookshop.asp">BOOKSHOP</a>
</span>
</td>
</tr>
</table>
<!-- START OF BODY TABLE -->
<table width=100% cellpadding=0 cellspacing=0 border=0>
  <tr>
    <td width=120 bgcolor=#<%=lightcolor%> valign=top>
<!-- START OF LEFT HAND MENU TABLE-->
<table width=100% cellpadding=0 cellspacing=0 border=0>
  <tr>
    <td bgcolor=#FFFFFF valign=top>
      <img src="./images/<%=pallate%>/transparent.gif" height="3"  width="1" border="0"><br>
      <a href="<%=signtext%>.asp"><img src="./images/<%=pallate%>/<%=signtext%>.gif" height="23" width="120" border="0"></a><br>
      <img src="./images/<%=pallate%>/transparent.gif" height="3" width="1" border="0"><br>
    </td>
  </tr>
  <tr>
    <td bgcolor=#<%=darkColor%>>&nbsp;
</span>
</td>
</tr>
<tr>
  <td bgcolor=#<%=middleColor%>>
<img src="./images/<%=pallate%>/transparent.gif" height="15" width="1" border="0"><br>
</td>
</tr>
<tr>
  <td bgcolor=#<%=lightColor%>>
<img src="./images/<%=pallate%>/transparent.gif" height="15" width="1" border="0"><br>
<span class="smallbodytext">
<%
  If signtext="signout" Then
  Response.Write"&nbsp;<a CLASS=""black"" href=""./contact.asp"">CONTACTS</a><br>&nbsp;<BR>"
  Response.Write"</span>"
  Response.Write"<span class=""smallbodytext"">"
  Response.Write"&nbsp;EDIT:<BR>"
  Response.Write"&nbsp;&nbsp;- <a CLASS=""black"" href=""editdetails.asp"">PERSONAL DETAILS</a><BR>"
  Response.Write"&nbsp;&nbsp;- <a CLASS=""black"" href=""editprefs.asp"">PREFERENCES</a><BR>&nbsp;<br>"
  Response.Write"</span>"
  Response.Write"<span class=""smallbodytext"">"
  Response.Write"&nbsp;<a CLASS=""black"" href=""subscribe.asp"">RESUBSCRIBE</a><br>"
  Else
  Response.Write"&nbsp;<a CLASS=""black"" href=""./subscribe.asp"">SUBSCRIBE</a><br>"
  Response.Write"&nbsp;<a CLASS=""black"" href=""./sample.asp"">SAMPLE ISSUE</a><br>"
  Response.Write"&nbsp;<a CLASS=""black"" href=""./contact.asp"">CONTACT</a><br>"
  End If
%>
&nbsp;<a CLASS="black" href="./editorial.asp">EDITORIAL TEAM</a><br>
&nbsp;<a CLASS="black" href="./production.asp">PRODUCTION TEAM</a><br></B>
&nbsp;<a CLASS="black" href="./submit.asp">SUBMITTING PAPERS</a><br></B>
<img src="./images/<%=pallate%>/transparent.gif" height="15" width="1" border="0"><br>
</td>
</tr>
<tr>
  <td bgcolor=#<%=middleColor%>>
<img src="./images/<%=pallate%>/transparent.gif" height="15" width="1" border="0"><br>
</td>
</tr>
<tr>
  <td bgcolor=#<%=darkColor%>>
<img src="./images/<%=pallate%>/transparent.gif" height="15" width="1" border="0"><br>
</td>
</table>
</td>
<td bgcolor=#FFFFFF valign=top>
  <!-- MAIN BODY TABLE (TOP DATE)-->
  <table width=100% cellpadding=0 cellspacing=0 border=0>
    <tr>
      <td width=20>&nbsp;</td>
      <td valign=middle width=100% height=30>
        <table width=100% cellpadding=0 cellspacing=0 border=0>
          <tr>
            <td valign=middle height=30>
              <span class="normalbodytext">
              <SCRIPT>
              document.write('<b>' + theDay + ',</b> ' + theDate + ' ' + theMonth + ' ' + theYear + '. ');
              </SCRIPT>
              </span>
              <span class="normalbodytextB">Volume:</span><span class="normalbodytext"> <%=LEFT(cpdIssue, 2)%> </span><span class="normalbodytextB">Issue:</span><span class="normalbodytext"> <%=RIGHT(cpdIssue, 2)%> (<%=issueIs(INT(RIGHT(cpdIssue, 2)))%> edition)</span>
            </td>
            <td width=100 valign=bottom>
              <img src="./images/<%=pallate%>/spectacles.gif" height="10" usemap="#themap" width="100" border="0" ><br>
            </td>
          </tr>
        </table>
      </td>
    </tr>
    <tr>
      <td width=20 height=1>
        <img src="./images/<%=pallate%>/transparent.gif" height="1" width="1" border="0" ><BR>
      </td>
      <td valign=middle width=100% bgcolor=#000000 height=1>
        <img src="./images/<%=pallate%>/black.gif" height="1" width="1" border="0" ><BR>
      </td>
    </tr>
    <tr>
      <td width=20>
        &nbsp;
      </td>
      <td valign=top width=100% bgcolor=#FFFFFF>
        <span class="verylargebodytextBI">Education Live from Dental Update</span>
        <!-- BODY OF PAGE -->
        <table width="90%" bgcolor=#FFFFFF border="0" cellspacing="0" cellpadding="0">
          <tr>
            <td width="10%" valign=top>
              <a href="conference.asp"><img src="./images/communal/education.jpg" height="100" width="80" border="0"></a>
            </td>
            <td width="90%" colspan="3" valign="top">
              <!-- GOES HERE -->
              <p align=center>&nbsp;<BR>
              <table width=90% cellspacing=0 cellpadding=4 border=0 bgcolor=#<%=lightcolor%>>
<tr>
  <td valign=top width=50% class="normalbodytext">
    <span class="normalbodytextB">London - February 28</span>. FULL<BR>
    <span class="normalbodytextB">Bristol - March 21</span>. FULL<BR>
    <span class="normalbodytextB">Manchester - April 4</span>. FULL<BR>
    <span class="normalbodytextB">Leeds - May 16</span>. FULL<BR>
    <span class="normalbodytextB">Norwich - May 23</span>. FULL  </td>
  <td valign=top width=50% class="normalbodytext">
    <span class="normalbodytextB">Edinburgh - July 4</span>. FULL<BR>
    <span class="normalbodytextB">Birmingham - October 24</span>. National Motorcycle Museum<BR>
    <span class="normalbodytextB">Belfast - November 28</span>. Hastings Stormont Hotel
  </td>
</tr>
</table>&nbsp;<BR></p>
<FORM NAME="education" ACTION="https://select.worldpay.com/wcc/purchase" METHOD="POST">
<span class="largebodytext">
Choose the <span class="largebodytextBI">Education Live Event</span> you would like to go on. You will then be forwarded to the <span class="largebodytextB">WorldPay.com</span> server to complete your secure transaction.<BR>&nbsp;<BR>
<Select name="cartId">
  <option value="confbirmingham">Birmingham - October 24</option>
  <option value="confbelfast">Belfast November 28</option>
  <INPUT TYPE="HIDDEN" NAME="instId" VALUE="63768">
  <INPUT TYPE="HIDDEN" NAME="currency" VALUE="GBP">
  <INPUT TYPE="HIDDEN" NAME="M_userid" VALUE="<%=userid%>">
  <INPUT TYPE="HIDDEN" NAME="desc" VALUE="Dental Update Education Live Event">
  <INPUT TYPE="HIDDEN" NAME="amount" VALUE="<%=LEFT(((conferenceamount*0.9)*1.175), 6)%>">
  <input type=hidden name="testMode" value="0">
  
</select><BR>&nbsp;<BR>
<a href="javascript:document.education.submit()"><img src="./images/<%=pallate%>/submit.gif"  border=0></a><BR>&nbsp;<BR>

<span class="largebodytextB">This event is £<%=conferenceamount%>.00</span>. With an online discount of 10%, you will be charged £<%=LEFT(((conferenceamount*0.9)*1.175), 6)%> (Inc. VAT). If you are a Dental Update subscriber please <span class="largebodytextB">Sign-In</span> and complete to this page to guarantee your £15.00 discount.<BR>&nbsp;<BR>

</span>
</td>
</tr>
</table>

</td>
</tr>
<tr>
  <td width=20 height=1>
    <img src="./images/<%=pallate%>/transparent.gif" height="1" width="1" border="0" ><BR>
  </td>
  <td valign=middle width=100% bgcolor=#000000 height=1>
    <img src="./images/<%=pallate%>/black.gif" height="1" width="1" border="0" ><BR>
  </td>
</tr>
</table>
</td>
</tr>
<TR>
  <td bgcolor=#<%=lightcolor%>>&nbsp;</td>
<td  align=center>
  <span class="smallbodytext">
  &copy; George Warman Publications (UK) Ltd. All Rights Reserved.<BR>Code &copy; &amp; Site by Paul Bucknell. All Rights Reserved. <a class="ul" href="tandc.asp">T &amp; C's and Privacy</a>.</span>
  
</td>
</TR>
</table>
<map name="themap">
<area shape="rect" coords="0,0,20,10" href="./cgi-bin/spectacles.asp?value=4&page=defaultlogged" Alt="Purple">
<area shape="rect" coords="20,0,40,10" href="./cgi-bin/spectacles.asp?value=1&page=defaultlogged" Alt="Blue">
<area shape="rect" coords="40,0,60,10" href="./cgi-bin/spectacles.asp?value=3&page=defaultlogged" Alt="Green">
<area shape="rect" coords="60,0,80,10" href="./cgi-bin/spectacles.asp?value=2&page=defaultlogged" Alt="Brown">
<area shape="rect" coords="80,0,100,10" href="./cgi-bin/spectacles.asp?value=5&page=defaultlogged" Alt="Red">
</map>
<%
  'CLOSE THE CONNECTION TO THE DATABASE
  objConn.Close
  Set objConn=Nothing
%>
</body>
</html>
