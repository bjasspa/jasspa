/*
 * cuckoo web authoring
 * cuckoo-gen.js
 * Convenience tool to rebuild a delivery
 * Version 0.1.1
 * Copyright (c) 2000-2001 Alexis Grandemange
 * Mail: alexis.grandemange@pagebox.net
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; version
 * 2.1 of the License.
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 * A copy of the GNU Lesser General Public License lesser.txt should be
 * included in the distribution.
 */
/*
 * Transforms the xml file
 * Parameters:
 * xslname name of the xsl file
 * xmlname name of the xml source file
 * to name of the target file
 */
function process(xslname, xmlname, to, progid) {
    // WScript.Echo("xsl:" + xslname + " xmlname:" + xmlname + " to:" + to);
    var xmldoc = new ActiveXObject(progid);
    var xsldoc = new ActiveXObject(progid);		
    xmldoc.async = false;
    xsldoc.async = false;
    xmldoc.validateOnParse = false;
    xsldoc.validateOnParse = false;
    xmldoc.load(xmlname);
    xsldoc.load(xslname);			
    var out = new ActiveXObject("Scripting.FileSystemObject");
    var o = out.CreateTextFile(to, true, false)
    if((xmldoc.parseError.errorCode == 0) && (xsldoc.parseError.errorCode == 0)) {
	var str = xmldoc.transformNode(xsldoc);
	// MSXML 3 adds <META http-equiv="Content-Type" content="text/html; charset=UTF-16">
	str = str.replace("; charset=UTF-16", "");
        str = str.replace("content=\"text/html", "content=\"text/html; charset=iso-8859-1");
        str = str.replace(
          "</head>", "<script>\r\nif (is_nav) document.write(\'<link rel=\"stylesheet\" href=\"cuckoo-nav6.css\"" +
          " type=\"text/css\">\');\r\n</script>");
	var a0 = /\u00A0/g;
	var cuckoo = /<cuckoo>/g;
	var ecuckoo = /<\/cuckoo>/g;
	var amp = /!#/g;
	var nbsp = /!nbsp;/g;
	str = str.replace(a0, "&nbsp;").replace(cuckoo, "").replace(ecuckoo, "").replace(amp, "&#").replace(nbsp, "&nbsp;");
	var pos = to.lastIndexOf("/");
	if (pos == -1)
	    pos = to.lastIndexOf("\\");
	var fn = to.substring(pos + 1);
	var pos = fn.lastIndexOf("ForPrinters.");
	var href = /href=\"\#this/g;
	str = str.replace(href, "href=\"" + fn.substring(0, pos));
	o.write(str);
    }
    o.close();
}
/*
 * WSH script.
 * Recommended version of WSH: 5.6 and above for WScript.CurrentDirectory.
 * Usage:
 * cuckoo-gen.js /dir:source-directory|/file:file [/toDir:target-directory|/toFile:target-file] [/xsl:xsl-file]
 * Example:
 * cuckoo-gen.js /dir:D:\cuckoo /toDir:D:\cuckoo /xsl:D:\cuckoo\cuckoo.xsl
 * cuckoo-gen.js /file:D:\cuckoo\myfile.xml /toFile:D:\cuckoo\myfile.asp /xsl:D:\cuckoo\cuckoo.xsl
 */
var xslfile = null;
var todir = null;
var argsNamed = WScript.Arguments.Named;
if (argsNamed.Exists("xsl"))
    xslfile = argsNamed.Item("xsl");
else
    xslfile = WScript.CurrentDirectory + "\\cuckoo-printer.xsl";
var bKey = null;
var progid = null;
var WshShell = WScript.CreateObject ("WScript.Shell");
try {
    bKey = WshShell.RegRead("HKEY_CLASSES_ROOT\\Msxml2.DOMDocument.4.0\\");
}
catch(e) {}
if (bKey == null) {
    try {
        bKey = WshShell.RegRead("HKEY_CLASSES_ROOT\\Msxml2.DOMDocument.3.0\\");
    }
    catch(e) {}
    if (bKey != null)
        progid = "Msxml2.DOMDocument.3.0";
} else
    progid = "Msxml2.DOMDocument.4.0";
if (progid == null)
    WScript.Echo("You must install MSXML 3 or MSXML 4");
else {
    // WScript.Echo(progid);
if (argsNamed.Exists("dir")) {
    var fso = new ActiveXObject("Scripting.FileSystemObject");
    var xmldir = argsNamed.Item("dir");
    if (fso.FolderExists(xmldir)) {
        if (argsNamed.Exists("toDir"))
            todir = argsNamed.Item("toDir");
        else
            toDir = xmldir;
        var fold = fso.GetFolder(xmldir);
        var files = new Enumerator(fold.Files);
        for (; !files.atEnd(); files.moveNext()) {
            var n = files.item().Name;
            var l = n.length - "W.xml".length;
            var s = n.substring(0, l);
            if (n.substring(l) == "W.xml")
                process(xslfile, xmldir + "\\" + n, todir + "\\" + s + "ForPrinters.html", progid);
        }
    }
} else {
    if (argsNamed.Exists("file")) {
    	var xmlname = argsNamed.Item("file");
    	if (argsNamed.Exists("toFile"))
            process(xslfile, xmlname, argsNamed.Item("toFile"), progid);
    	else {
    	    if (argsNamed.Exists("toDir")) {
                toDir = argsNamed.Item("toDir");
    	        var pos = xmlname.lastIndexOf("\\");
    	        var f1 = xmlname.substring(pos);
                process(xslfile, xmlname, toDir + f1.replace("W.xml", "ForPrinters.html"), progid);
    	    } else
                process(xslfile, xmlname, xmlname.replace("W.xml", "ForPrinters.html"), progid);
        }
    } else
        WScript.Echo(
            "Usage: cuckoo-gen.js /dir:source-directory|/file:file [/toDir:target-directory|"
            + "/toFile:target-file] [/xsl:xsl-file]");
}    
}
