/*
 * WSH script.
 * To run after cuckoo-gen.js.
 * Recommended version of WSH: 5.6 and above for WScript.CurrentDirectory.
 * Usage:
 * math-gen.js /dir:source-directory|/file:file [/toDir:target-directory]
 * Example:
 * math-gen.js /dir:D:\cuckoo /toDir:D:\cuckoo
 * math-gen.js /file:D:\cuckoo\myfile.html /toDir:D:\cuckoo
 */
var argsNamed = WScript.Arguments.Named;
if (argsNamed.Exists("dir")) {
  var fso = new ActiveXObject("Scripting.FileSystemObject");
  var dir = argsNamed.Item("dir");
  if (fso.FolderExists(dir)) {
    var todir = null;
    if (argsNamed.Exists("toDir"))
      todir = argsNamed.Item("toDir");
    else
      toDir = dir;
    var fold = fso.GetFolder(dir);
    var files = new Enumerator(fold.Files);
    for (; !files.atEnd(); files.moveNext()) {
      var n = files.item().Name;
      var l = n.length - "-nav6.html".length;
      var s = n.substring(0, l);
      if (n.substring(l) == "-nav6.html")
        process(dir + "\\" + s, todir + "\\" + s);
    }
  }
} else {
  if (argsNamed.Exists("file")) {
    var name = argsNamed.Item("file");
    var l = name.length - "-nav6.html".length;
    var s = name.substring(0, l);
    if (name.substring(l) == "-nav6.html") {
      if (argsNamed.Exists("toDir")) {
        toDir = argsNamed.Item("toDir");
        var pos = s.lastIndexOf("\\");
        var f1 = s.substring(pos);
        process(s, toDir + f1);
      } else
        process(s, s);
    }
  } else
    WScript.Echo(
              "Usage: math-gen.js /dir:source-directory|/file:file [/toDir:target-directory]");
}
/**
 * Generates xxxx.html and xxxx-ie5.html from xxxx-nav6.html.
 * Parameters:
 * Source xxxx
 * Destination xxxx
 */
function process(source, dest) {    
  var out = WScript.CreateObject("Scripting.FileSystemObject");
  var f = out.GetFile(source + "-nav6.html");
  var is = f.OpenAsTextStream( 1, 0 );
  if (is == null)
    return;
  var page = "";
  var count = 0;
  while( !is.AtEndOfStream ) {
    page += (is.ReadLine() + "\r\n");
    count ++;
  }
  if (count == 0)
    return;
  page = processSymbol(page);
  page = processGreek(page);
  page = page.replace('</div>', '</div>\r\n<script>\r\n' +
            'function redirect() {\r\n' +
            'window.location = window.location.toString().replace(".html", "-nav6.html");\r\n}\r\n' +
            'var agt=navigator.userAgent.toLowerCase();\r\n' +
            'var is_major = parseInt(navigator.appVersion);\r\n' +
            'var is_nav  = ((agt.indexOf("mozilla")!=-1) && (agt.indexOf("spoofer")==-1)' +
            '&& (agt.indexOf("compatible") == -1) && (agt.indexOf("opera")==-1)' +
            '&& (agt.indexOf("webtv")==-1) && (agt.indexOf("hotjava")==-1));' +
            'var is_nav6up = (is_nav && (is_major >= 5));\r\n' +
            'var is_gecko = (agt.indexOf("gecko") != -1);\r\n' +
            'var is_opera6 = (agt.indexOf("opera 6") != -1 || agt.indexOf("opera/6") != -1);\r\n' +
            'if (is_nav6up || is_gecko || is_opera6)\r\nsetTimeout("redirect();", 10);\r\n' +
            '</script>\r\n');
  var o = out.CreateTextFile(dest + ".html", true, false);
  o.write(page);
  o.close();
}
/**
 * Converts math symbols from unicode to symbol font.
 * Parameter:
 * str page string
 * Returns converted page string.
 */
function processSymbol(str) {
  var code = /&#8706;/g;
  page = str.replace(code, '&#182;');
  code = /&#8719;/g;
  page = page.replace(code, '&#213;');
  code = /&#8721;/g;
  page = page.replace(code, '&#229;');
  code = /&#8730;/g;
  page = page.replace(code, '&#214;');
  code = /&#8734;/g;
  page = page.replace(code, '&#165;');
  code = /&#8745;/g;
  page = page.replace(code, '&#199;');
  code = /&#8746;/g;
  page = page.replace(code, '&#200;');
  code = /&#8747;/g;
  page = page.replace(code, '&#166;');
  code = /&#8776;/g;
  page = page.replace(code, '&#187;');
  code = /&#8800;/g;
  page = page.replace(code, '&#185;');
  code = /&#8801;/g;
  page = page.replace(code, '&#186;');
  code = /&#8804;/g;
  page = page.replace(code, '&#163;');
  code = /&#8805;/g;
  page = page.replace(code, '&#179;');
  code = /&#8704;/g;
  page = page.replace(code, '&#034;');
  code = /&#8707;/g;
  page = page.replace(code, '&#036;');
  code = /&#8835;/g;
  page = page.replace(code, '&#201;');
  code = /&#8839;/g;
  page = page.replace(code, '&#202;');
  code = /&#8836;/g;
  page = page.replace(code, '&#203;');
  code = /&#8834;/g;
  page = page.replace(code, '&#204;');
  code = /&#8838;/g;
  page = page.replace(code, '&#205;');
  code = /&#8712;/g;
  page = page.replace(code, '&#206;');
  code = /&#8713;/g;
  page = page.replace(code, '&#207;');
  return page;
}
/**
 * Converts greek characters from unicode to symbol font.
 * Parameter:
 * str page string
 * Returns converted page string.
 */
function processGreek(str) {
  // lower case
  var code = /&#945;/g;
  page = str.replace(code, '&#097;');
  var code = /&#946;/g;
  page = page.replace(code, '&#098;');
  var code = /&#947;/g;
  page = page.replace(code, '&#103;');
  var code = /&#948;/g;
  page = page.replace(code, '&#100;');
  var code = /&#949;/g;
  page = page.replace(code, '&#101;');
  var code = /&#950;/g;
  page = page.replace(code, '&#122;');
  var code = /&#951;/g;
  page = page.replace(code, '&#104;');
  var code = /&#952;/g;
  page = page.replace(code, '&#113;');
  var code = /&#953;/g;
  page = page.replace(code, '&#105;');
  var code = /&#954;/g;
  page = page.replace(code, '&#107;');
  var code = /&#955;/g;
  page = page.replace(code, '&#108;');
  var code = /&#956;/g;
  page = page.replace(code, '&#109;');
  var code = /&#957;/g;
  page = page.replace(code, '&#110;');
  var code = /&#958;/g;
  page = page.replace(code, '&#120;');
  var code = /&#959;/g;
  page = page.replace(code, '&#111;');
  var code = /&#960;/g;
  page = page.replace(code, '&#112;');
  var code = /&#961;/g;
  page = page.replace(code, '&#114;');
  var code = /&#962;/g;
  page = page.replace(code, '&#086;');
  var code = /&#963;/g;
  page = page.replace(code, '&#115;');
  var code = /&#964;/g;
  page = page.replace(code, '&#116;');
  var code = /&#965;/g;
  page = page.replace(code, '&#117;');
  var code = /&#966;/g;
  page = page.replace(code, '&#102;');
  var code = /&#967;/g;
  page = page.replace(code, '&#099;');
  var code = /&#968;/g;
  page = page.replace(code, '&#121;');
  var code = /&#969;/g;
  page = page.replace(code, '&#119;');
  // upper case
  var code = /&#913;/g;
  page = page.replace(code, '&#065;');
  var code = /&#914;/g;
  page = page.replace(code, '&#066;');
  var code = /&#915;/g;
  page = page.replace(code, '&#071;');
  var code = /&#916;/g;
  page = page.replace(code, '&#068;');
  var code = /&#917;/g;
  page = page.replace(code, '&#069;');
  var code = /&#918;/g;
  page = page.replace(code, '&#090;');
  var code = /&#919;/g;
  page = page.replace(code, '&#072;');
  var code = /&#920;/g;
  page = page.replace(code, '&#081;');
  var code = /&#921;/g;
  page = page.replace(code, '&#073;');
  var code = /&#922;/g;
  page = page.replace(code, '&#075;');
  var code = /&#923;/g;
  page = page.replace(code, '&#076;');
  var code = /&#924;/g;
  page = page.replace(code, '&#077;');
  var code = /&#925;/g;
  page = page.replace(code, '&#078;');
  var code = /&#926;/g;
  page = page.replace(code, '&#088;');
  var code = /&#927;/g;
  page = page.replace(code, '&#079;');
  var code = /&#928;/g;
  page = page.replace(code, '&#080;');
  var code = /&#929;/g;
  page = page.replace(code, '&#082;');
  var code = /&#931;/g;
  page = page.replace(code, '&#083;');
  var code = /&#932;/g;
  page = page.replace(code, '&#084;');
  var code = /&#933;/g;
  page = page.replace(code, '&#085;');
  var code = /&#934;/g;
  page = page.replace(code, '&#070;');
  var code = /&#935;/g;
  page = page.replace(code, '&#067;');
  var code = /&#936;/g;
  page = page.replace(code, '&#089;');
  var code = /&#937;/g;
  page = page.replace(code, '&#087;');
  return page;
}
