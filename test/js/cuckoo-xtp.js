/*
 * cuckoo web authoring
 * cuckoo-xtp.js
 * Convenience tool to build XTP (Resin) files.
 * Version 0.01
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
function process(xslname, xmlname, to) {
    // WScript.Echo("xsl:" + xslname + " xmlname:" + xmlname + " to:" + to);
    var fso = new ActiveXObject("Scripting.FileSystemObject");
    var o = fso.CreateTextFile(to, true, false);
    o.Write("<?xml-stylesheet href='" + xslname + "'?>\r\n");
    var i = fso.OpenTextFile(xmlname, 1 /* For reading. */);
    var r = i.ReadLine();
    var cdata = /<!\[CDATA\[/g;
    var ecdata = /\]\]>/g;
    var amp = /&amp;#/g;
    while(!i.AtEndOfStream) {
    	r = r.replace(cdata, "").replace(ecdata, "");
    	r = r.replace(amp, "&#");
        o.Write(r + "\n");
        r = i.ReadLine();
    }
    r = r.replace(cdata, "").replace(ecdata, "");
    r = r.replace(amp, "&#");
    o.Write(r + "\n");
    i.Close();
    o.Close();
}
/*
 * WSH script.
 * Recommended version of WSH: 5.6 and above for WScript.CurrentDirectory.
 * Usage:
 * cuckoo-gen.js /dir:source-directory|/file:file [/toDir:target-directory|/toFile:target-file] [/xsl:xsl-file]
 * Example:
 * cuckoo-gen.js /dir:D:\cuckoo /toDir:D:\cuckoo /xsl:cuckoo.xsl
 * cuckoo-gen.js /file:D:\cuckoo\myfile.xml /toFile:D:\cuckoo\myfile.xtp /xsl:cuckoo.xsl
 * Note:
 * The xsl file should the path of the xsl file relative to the xtp file on Resin.
 * For instance cuckoo.xsl if cuckoo.xsl is in the same directory as the xtp or
 * ../xsl/cuckoo.xsl if cuckoo.xsl is stored in a xsl directory at the same level as the xtp file or
 * /xsl/cuckoo.xsl. 
 */
var xslfile = null;
var todir = null;
var argsNamed = WScript.Arguments.Named;
if (argsNamed.Exists("xsl"))
    xslfile = argsNamed.Item("xsl");
else
    xslfile = "cuckoo-xtp.xsl";
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
                process(xslfile, xmldir + "\\" + n, todir + "\\" + s + ".xtp");
        }
    }
} else {
    if (argsNamed.Exists("file")) {
    	var xmlname = argsNamed.Item("file");
    	if (argsNamed.Exists("toFile"))
            process(xslfile, xmlname, argsNamed.Item("toFile"));
    	else {
    	    if (argsNamed.Exists("toDir")) {
                toDir = argsNamed.Item("toDir");
    	        var pos = xmlname.lastIndexOf("\\");
    	        var f1 = xmlname.substring(pos);
                process(xslfile, xmlname, toDir + f1.replace("W.xml", ".xtp"));
    	    } else
                process(xslfile, xmlname, xmlname.replace("W.xml", ".xtp"));
        }
    } else
        WScript.Echo(
            "Usage: cuckoo-gen.js /dir:source-directory|/file:file [/toDir:target-directory|"
            + "/toFile:target-file] [/xsl:xsl-file]");
}    
