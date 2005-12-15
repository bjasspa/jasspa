// Define all variables and set the default delay to 5ms
var layername, xgoal, ygoal, xhop, yhop, delay=5;

// Check to see if the browser is DHTML-compatible
function checkDHTML() {
  if ((parseInt(navigator.appVersion)>=4) &&
     ((navigator.appName!="Netscape" &&
       navigator.appVersion.indexOf("X11") == -1) ||
      (navigator.appName!="Microsoft Internet Explorer" &&
       navigator.appVersion.indexOf("Macintosh") == -1)))
    { return 1 }
  else
    { document.location="nodhtml.htm"; return 0 }
}

// Construct a valid reference to a layer
// in either Netscape JavaScript or Microsoft JScript
function makeName(layerID) {
  if (navigator.appName=="Netscape")
    { refname = eval("document." + layerID) }
  else
    { refname = eval("document.all." + layerID + ".style") }
  return refname
}

// Slide over xhop,yhop pixels every delay milliseconds
// until the layer reaches xgoal and ygoal
function slide() {
  if ((parseInt(layername.left) != xgoal) ||
      (parseInt(layername.top) != ygoal))
    {  layername.left = parseInt(layername.left) + xhop;
       layername.top = parseInt(layername.top) + yhop;
       window.setTimeout("slide()", delay) }
}

