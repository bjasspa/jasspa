<html>
<head>
<!--
This file retrieved from the JS-Examples archives
http://www.js-examples.com
1000s of free ready to use scripts, tutorials, forums.
Author: Helge Larsen - 0
-->

<SCRIPT LANGUAGE="JavaScript">

<!--










function Cookie(document,name,hours,path,domain,secure) {
// any VAR in "this" that does not start with a "$" will
// be written into the cookie (read from also)
  this.$doc  = document
  this.$name = name
  if (hours)  this.$expiration=new Date((new Date()).getTime()+hours*3600000); else this.$expiration = null
  if (path)   this.$path   = path;                                             else this.$path       = null
  if (domain) this.$domain = domain;                                           else this.$domain     = null
  if (secure) this.$secure = true;                                             else this.$secure     = false
}

function CookieWrite() {
  var cookieval=""
  for(var prop in this) {
    if ((prop.charAt(0) == '$') || ((typeof this[prop]) == 'function') || prop == '') continue
if (cookieval != "") cookieval += '&'
cookieval+=prop+":"+escape(this[prop])
  }
  var cookie=this.$name+"="+cookieval
  if (this.$expiration) cookie+='; expires=' + this.$expiration.toGMTString()
  if (this.$path)       cookie+='; path='    + this.$path
  if (this.$domain)     cookie+='; domain='  + this.$domain
  if (this.$secure)     cookie+='; secure'
//alert("writting cookie="+cookie)
  this.$doc.cookie=cookie
}

function CookieRead() {
  var allcookies=this.$doc.cookie
  if (allcookies=="") {
    return false
  }
  var start= allcookies.indexOf(this.$name+'=')
  if (start== -1) {
    return false
  }
  start += this.$name.length+1
  var end=allcookies.indexOf(';',start)
  if (end == -1) end=allcookies.length
  var cookieval = allcookies.substring(start,end)
  var a = cookieval.split('&')
  for (var i=0;i<a.length;i++) a[i]=a[i].split(':')
  for (var i=0;i<a.length;i++) this[a[i][0]]=unescape(a[i][1])
  return true
}

function CookieDelete() {
  var cookie = this.$name+'='
  if (this.$path)   cookie+='; path='+this.$path
  if (this.$domain) cookie+='; domain='+this.$domain
  cookie+='; expires=Fri, 02-Jan-1970 00:00:00 GMT'// MAKE IT EXPIRE!
  this.$doc.cookie=cookie
}

new Cookie()
Cookie.prototype.write = CookieWrite
Cookie.prototype.del   = CookieDelete
Cookie.prototype.read  = CookieRead
























window.status = "Press START, or hit 'S'";




var FullBrwNmV = navigator.userAgent;
var brwNmV = navigator.userAgent.substring(0,11);
var lastinvader = "";
var gunner = "_]===";
var space = " ";
var shot = "-";
var digitInvadersWidth = 40;
var i = 0;
var wait = 0;
var buttonPresses = -1;
var score = 0;
var invader = "";
var pause = 61;
var myHighScoreCookie = new Cookie(document,"myHighScore",3000);

if (!myHighScoreCookie.read() || !myHighScoreCookie.text) {
  myHighScoreCookie.text = 0;
}

myHighScoreCookie.write();



function startTheGame() {
   if (brwNmV == "Mozilla/2.0") {
       alert('Sorry, this digitInvaders requires a newer browser-version!');
   }
   else { 
       var lastinvader = "";
       var i = 0;
       var score = 0;
       var invader = "";
       var pause = 61;
       document.digitInvadersForm.statusButton.value = "Go!";
       digitInvaders();
   }
}



function digitInvaders() {
  if (document.digitInvadersForm.statusButton.value == "Go!"){
    var showed = "";
    var outSpaces = "";
    var outShots = "";
    var rndNumber=Math.floor(Math.random() * 10);
    var invaLen = invader.length;
    if (wait == 10) {
        invader += rndNumber.toString(10);
        wait = 0;
    }
    else {
        wait += 1;
    }
    if (invader.charAt(0) == buttonPresses.toString(10)) {
        score += 1;
        lastinvader = invader;
        invader = "";
        for (i=1; i<invaLen; i++) {
           invader += lastinvader.charAt(i);
        }
        if (score%10 == 0 & pause > 10)
           pause -= 5;      
    }
    outSpaces = "";
    outShots = "";
    for (i=1; i<=(digitInvadersWidth-invaLen); i++) {
       outSpaces += space;
       outShots += shot;
    }
    if (buttonPresses == -1)
      showed = gunner + outSpaces + invader;
    else
      showed = gunner + outShots + invader;
    window.status = showed;
    document.digitInvadersForm.digitInvadersField.value = showed;
    document.digitInvadersForm.scoreField.value = score.toString(10);
    buttonPresses = -1;
    if (invaLen<digitInvadersWidth) {
        timeToNextStep = setTimeout("digitInvaders()",pause);
    }
    else {
        invader = "";
        pause = 61;
        document.digitInvadersForm.statusButton.value = "Game Over";
        window.status = "Game Over";
        if (score >= document.digitInvadersForm.highScoreField.value){
           document.digitInvadersForm.highScoreField.value = score;
           myHighScoreCookie.text = score;
           myHighScoreCookie.write();
        }
        score = 0;
    }
  }
  else{
    invader = "";
    pause = 61;
    score = 0;
    document.digitInvadersForm.digitInvadersField.value = gunner;
    window.status = gunner;
  }
}









var isIE5=document.all;
var isNS4=document.layers;
var isIE6=document.getElementById && isIE5;
var isNS6=document.getElementById && ! isIE6;



function KeyPressed(keyStroke) { 
  var eventChooser = (isNS4||isNS6) ? keyStroke.which : event.keyCode; 
  buttonPresses=String.fromCharCode(eventChooser).toLowerCase();

  if (buttonPresses == "s"){
     document.digitInvadersForm.statusButton.value='Ready?';
     setTimeout('startTheGame()',2000);
  }
}




document.onkeypress = KeyPressed;





function resetGame(){

document.digitInvadersForm.digitInvadersField.value = gunner;
document.digitInvadersForm.scoreField.value = "0";

if (myHighScoreCookie.read()){
  document.digitInvadersForm.highScoreField.value = myHighScoreCookie.text;
}

}









-->
</script>



</head>
<body>

<body onLoad="javascript:resetGame();">
<DIV ALIGN="CENTER">
<B>
<FONT STYLE="font-family:'Impact';font-size:25px;">
DigitInvaders!
</FONT>
<BR>
Blast out the digits, before they reach your gun!<BR>
Use your keyboard, and press the number of the first digit.
<BR>
Press 'S' to start...
<BR>
</B>

<FORM NAME="digitInvadersForm">
<INPUT TYPE="button" VALUE="   Start   " ONCLICK="document.digitInvadersForm.statusButton.value='Ready?';setTimeout('startTheGame()',2000);">
<INPUT TYPE="button" VALUE="   Stop    " ONCLICK="document.digitInvadersForm.statusButton.value='Stopped!'">
<BR>
<BR>
<BR>
</DIV>
<DIV ALIGN="CENTER">


<INPUT NAME="statusButton" TYPE="button" VALUE="Press START" ONCLICK="javacript:void(0);">
<BR>
<INPUT STYLE="font-family:'Courier';" TYPE="text" NAME="digitInvadersField" SIZE="55" VALUE="_]===">

<BR>&nbsp;


<TABLE BORDER=0>

<TR><TD>
Score:
</TD>

<TD>
<INPUT STYLE="font-family:'Courier';" TYPE="text" NAME="scoreField" SIZE="6" VALUE="0">
</TD></TR>

<TR><TD>
Highscore:
</TD>

<TD>
<INPUT STYLE="font-family:'Courier';" TYPE="text" NAME="highScoreField" SIZE="6" VALUE="0">
</TD>
</TR>

</TABLE>


</DIV>
</FORM>

<BR><center><a href='http://www.js-examples.com'>JS-Examples.com</a></center> 
</body>
</html>
