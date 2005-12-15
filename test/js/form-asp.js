/*
 * cuckoo web authoring
 * form-asp.js
 * Convenience tool to generate an ASP processing the request of a Cuckoo document
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
	str = str.replace('<?xml version="1.0" encoding="UTF-16"?>', "");
	var a0 = /\u00A0/g;	// For Opera
	var lt = /&lt;/g;
	var gt = /&gt;/g;
	var amp = /&amp;#/g;
	str = str.replace(a0, "&nbsp;").replace(lt, "<").replace(gt, ">").replace(amp, "&#");
	o.write(str);
    }
    o.close();
}
/*
 * WSH script.
 * Recommended version of WSH: 5.6 and above for WScript.CurrentDirectory.
 * Usage:
 * form-asp.js /dir:source-directory|/file:file [/toDir:target-directory|/toFile:target-file] [/xsl:xsl-file]
 * Example:
 * form-asp.js /dir:D:\cuckoo /toDir:D:\cuckoo /xsl:D:\cuckoo\cuckoo.xsl
 * form-asp.js /file:D:\cuckoo\myfile-aspF.xml /toFile:D:\cuckoo\myfile.asp /xsl:D:\cuckoo\form-asp.xsl
 */
var xslfile = null;
var todir = null;
var argsNamed = WScript.Arguments.Named;
if (argsNamed.Exists("xsl"))
    xslfile = argsNamed.Item("xsl");
else
    xslfile = WScript.CurrentDirectory + "\\form-asp.xsl";
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
            var l = n.length - "-aspF.xml".length;
            var s = n.substring(0, l);
            if (n.substring(l) == "-aspF.xml")
                process(xslfile, xmldir + "\\" + n, todir + "\\" + s + ".asp", progid);
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
                process(xslfile, xmlname, toDir + f1.replace("-aspF.xml", ".asp"), progid);
    	    } else
                process(xslfile, xmlname, xmlname.replace("-aspF.xml", ".asp"), progid);
        }
    } else
        WScript.Echo(
            "Usage: form-asp.js /dir:source-directory|/file:file [/toDir:target-directory|"
            + "/toFile:target-file] [/xsl:xsl-file]");
}    
}
