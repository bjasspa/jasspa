<html>
<head>
<!--
This file retrieved from the JS-Examples archives
http://www.js-examples.com
1000s of free ready to use scripts, tutorials, forums.
Author: Nanda Kishore - 0
-->

<style>
<!--
#slidemenubar, #slidemenubar2{
position:absolute;
border:1.5px solid black;
background-color:#F2F2F2;
layer-background-color:#F2F2F2;
font:bold 12px Verdana;
line-height:20px;
}
-->
</style>


</head>
<body>

<script language="JavaScript1.2">
var slidemenu_width=160 //specify width of menu (in pixels)
var slidemenu_reveal=12 //specify amount that menu should protrude initially
var slidemenu_top=170 //specify vertical offset of menu on page

var ns4=document.layers?1:0
var ie4=document.all&&navigator.userAgent.indexOf("Opera")==-1
var ns6=document.getElementById&&!document.all?1:0

if (ie4||ns6)
document.write('<div id="slidemenubar2" style="left:'+((slidemenu_width-slidemenu_reveal)*-1)+'; top:'+slidemenu_top+'; width:'+slidemenu_width+'" onMouseover="pull()" onMouseout="draw()">')
else if (ns4){
document.write('<style>\n#slidemenubar{\nwidth:'+slidemenu_width+';}\n<\/style>\n')
document.write('<layer id="slidemenubar" left=0 top='+slidemenu_top+' width='+slidemenu_width+' onMouseover="pull()" onMouseout="draw()" visibility=hide>')
}

var sitems=new Array()

///////////Edit below/////////////////////////////////

//siteitems[x]=["Item Text", "Optional URL associated with text"]

sitems[0]=["<big><font face='Arial'>Site Menu</font></big>", ""]
sitems[1]=["Menus And Navigation", "/"]
sitems[2]=["Document Effects", "/"]
sitems[3]=["Scrollers", "/"]
sitems[4]=["Image Effects", "/"]
sitems[5]=["Links And Buttons", "/"]
sitems[6]=["Dynamic Clocks & Dates", "/"]
sitems[7]=["Text Animations", "/"]
sitems[8]=["Browser Window", "/"]
sitems[9]=["User System Information", "/"]
sitems[10]=["Other", "/"]

//If you want the links to load in another frame/window, specify name of target (ie: target="_new")
var target=""

/////////////////////////////////////////////////////////

if (ie4||ns4||ns6){
for (i=0;i<sitems.length;i++){
if (sitems[i][1])
document.write('<a href="'+sitems[i][1]+'" target="'+target+'">')
document.write(sitems[i][0])
if (sitems[i][1])
document.write('</a>')
document.write('<br>\n')
}
}

function regenerate(){
window.location.reload()
}
function regenerate2(){
if (ns4){
document.slidemenubar.left=((slidemenu_width-slidemenu_reveal)*-1)
document.slidemenubar.visibility="show"
setTimeout("window.onresize=regenerate",400)
}
}
window.onload=regenerate2
rightboundary=0
leftboundary=(slidemenu_width-slidemenu_reveal)*-1
if (ie4||ns6){
document.write('</div>')
themenu=(ns6)? document.getElementById("slidemenubar2").style : document.all.slidemenubar2.style
}
else if (ns4){
document.write('</layer>')
themenu=document.layers.slidemenubar
}

function pull(){
if (window.drawit)
clearInterval(drawit)
pullit=setInterval("pullengine()",10)
}
function draw(){
clearInterval(pullit)
drawit=setInterval("drawengine()",10)
}
function pullengine(){
if ((ie4||ns6)&&parseInt(themenu.left)<rightboundary)
themenu.left=parseInt(themenu.left)+10
else if(ns4&&themenu.left<rightboundary)
themenu.left+=10
else if (window.pullit){
themenu.left=0
clearInterval(pullit)
}
}

function drawengine(){
if ((ie4||ns6)&&parseInt(themenu.left)>leftboundary)
themenu.left=parseInt(themenu.left)-10
else if(ns4&&themenu.left>leftboundary)
themenu.left-=10
else if (window.drawit){
themenu.left=leftboundary
clearInterval(drawit)
}
}
</script>
<BR><center><a href='http://www.js-examples.com'>JS-Examples.com</a></center> 
</body>
</html>
