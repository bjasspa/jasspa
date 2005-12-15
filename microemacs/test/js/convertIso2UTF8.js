/*
 * cuckoo web authoring
 * convertIso2UTF8.js
 * Convenience tool to change the charset
 * Version 0.1
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
 * WSH script.
 * Usage:
 * convertIso2UTF8.js /ifile:file /ofile:target-file
 */
var ifile = null;
var ofile = null;
var argsNamed = WScript.Arguments.Named;
if (argsNamed.Exists("ifile"))
    ifile = argsNamed.Item("ifile");
if (argsNamed.Exists("ofile"))
    ofile = argsNamed.Item("ofile");
if ((ifile == null) || (ofile == null))
    WScript.Echo("convertIso2UTF8.js /ifile:file /ofile:target-file");
else {
    var fso = new ActiveXObject("Scripting.FileSystemObject");
    var f = fso.GetFile(ifile);
    var is = f.OpenAsTextStream( 1, 0 );
    if (is != null) {
        var page = "";
        var count = 0;
        while( !is.AtEndOfStream ) {
            page += (is.ReadLine() + "\r\n");
            count ++;
        }
    	if (count != 0) {
            is.close();
            page = page.replace("charset=iso-8859-1", "charset=UTF-8")
            var o = fso.CreateTextFile(ofile, true, false)
            o.write(page);
            o.close();
        }
    }
}