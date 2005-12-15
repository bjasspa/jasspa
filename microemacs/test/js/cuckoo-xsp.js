/*
 * cuckoo web authoring
 * cuckoo-xtp.js
 * Convenience tool to build XSP (Cocoon) files.
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
 * xmlname name of the xml source file
 * to name of the target file
 */
function process(xmlname, to) {
    var fso = new ActiveXObject("Scripting.FileSystemObject");
    var o = fso.CreateTextFile(to, true, false);
    var i = fso.OpenTextFile(xmlname, 1 /* For reading. */);
    var pos = to.lastIndexOf("\\");
    if (pos == -1)
        pos = to.lastIndexOf("/");
    var basename = to.substring(pos + 1);
    pos = basename.lastIndexOf(".");
    if (pos != -1)
        basename = basename.substring(0, pos);
    var amp = /&amp;#/g;
    var r = i.ReadLine();
    r = r.replace(amp, "&#");
    while(!i.AtEndOfStream) {
        o.Write(r + "\n");
    	if (r == "<cuckoo>")
    	    o.Write("<page>" + basename + "</page>");
        r = i.ReadLine();
        r = r.replace(amp, "&#");
    }
    o.Write(r + "\n");
    i.Close();
    o.Close();
}
/*
 * WSH script.
 * Recommended version of WSH: 5.6 and above for WScript.CurrentDirectory.
 * Usage:
 * cuckoo-xsp.js /dir:source-directory|/file:file [/toDir:target-directory|/toFile:target-file]
 * Example:
 * cuckoo-xsp.js /dir:D:\cuckoo /toDir:D:\cuckoo
 * cuckoo-gen.js /file:D:\cuckoo\myfile.xml /toFile:D:\cuckoo\myfile.xsp
 */
var xslfile = null;
var todir = null;
var argsNamed = WScript.Arguments.Named;
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
                process(xmldir + "\\" + n, todir + "\\" + s + ".xsp");
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
                process(xmlname, toDir + f1.replace("W.xml", ".xsp"));
    	    } else
                process(xmlname, xmlname.replace("W.xml", ".xsp"));
        }
    } else
        WScript.Echo(
            "Usage: cuckoo-gen.js /dir:source-directory|/file:file [/toDir:target-directory|"
            + "/toFile:target-file]");
}    
