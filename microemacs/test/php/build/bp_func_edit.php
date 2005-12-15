<?php
/*
    bplog-0.5.8 - minimal news/weblog system
    Copyright (C) 2001 Chris Gushue <chris@blackplasma.net>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

function bp_edit_listall($table, $filename) {
    $sql = "SELECT * FROM $table ORDER BY id DESC";
    bp_edit_list($table, $sql, $filename);
}

function bp_edit_listown($table, $login, $filename) {
    $sql = "SELECT * FROM $table WHERE login='$login' ORDER BY id DESC";
    bp_edit_list($table, $sql, $filename);
}

function bp_edit_list($table, $sql, $filename) {
    global $idx;

    $result = bp_db_query($sql, $idx);

    print "<table border=\"0\" width=\"100%\">\n";

    print "<tr>\n";
    print "<td width=\"5%\"><b>Id</b></td>\n";
    print "<td width=\"75%\"><b>Subject/Entry</b></td>\n";
    print "<td><b>Date/Username</b></td>\n";
    print "<td><b>Action</b></td>\n";
    print "</tr>\n";
    print "<tr><td colspan=\"4\">&nbsp;</td></tr>";

    $num = bp_db_num_rows($result);

    for ($i = 0; $i < $num; ++$i) {
        $row = bp_db_fetch_array($result, $i);
        $subject = myStripSlashes($row["subject"]);
        $entry   = myStripSlashes($row["entry"]);

        $ts = strtotime($row["date"]);

        print "<form method=\"post\" action=\"$filename\" enctype=\"multipart/form-data\">";
        print "<input type=\"hidden\" name=\"id\" value=\"" . $row["id"]. "\">\n";
        print "<input type=\"hidden\" name=\"table\" value=\"$table\">\n";

        print "<tr valign=\"top\">\n";
        print "<td><b>" . $row["id"] . "</b></td>\n";
        print "<td><em>${subject}</em></td>\n";
        print "<td><nobr>" . date("Y-m-d H:i", $ts) . "</nobr></td>\n";
        print "<td><input type=\"submit\" name=\"submit_edit\" value=\"Edit\" class=\"button\"></td>\n";
        print "</tr>\n";

        print "<tr valign=\"top\">\n";
        print "<td colspan=\"2\">" . substr($entry, 0, 250) . " <b>...</b></td>\n"; // TODO: This can cause a problem if a html tag is cut off with substr - way to work around this perhaps?
        print "<td>" . $row["login"] . "</td>\n";
        print "<td><input type=\"submit\" name=\"submit_delete\" value=\"Delete\" class=\"button\"></td>\n";
        print "</tr>\n";

        print "</form>\n";

        print "<tr><td colspan=\"4\">&nbsp</td></tr>\n";
    }

    print "</table>\n";
}

function bp_edit_edit($table, $id, $filename) {
    global $idx;

    // Get info for selected id
    $sql = "SELECT * FROM $table WHERE id='$id'";
    $result = bp_db_query($sql, $idx);

    $row = bp_db_fetch_array($result);
    $subject = myStripSlashes($row["subject"]);
    $entry   = myStripSlashes($row["entry"]);

    $ts = strtotime($row["date"]);

    // Convert date to seperate things
    $year  = date("Y", $ts);
    $month = date("m", $ts);
    $day   = date("d", $ts);
    $hour  = date("H", $ts);
    $min   = date("i", $ts);

    print "<h1>Edit this news item</h1>\n";
    print "<form method=\"post\" action=\"$filename\" enctype=\"multipart/form-data\">\n";
    print "<input type=\"hidden\" name=\"table\" value=\"$table\">\n";
    print "<input type=\"hidden\" name=\"login\" value=\"" . $row["login"] . "\">\n";
    print "<input type=\"hidden\" name=\"date\" value=\"$ts\">\n";
    print "<input type=\"hidden\" name=\"id\" value=\"$id\">\n";

    print "<table>\n";

    print "<tr>\n";
    print "\t<td align=\"left\">Date:</td>";
    print "\t<td align=\"left\">";
    print "<input type=\"text\" name=\"year\" value=\"$year\" size=\"4\" maxlength=\"4\" class=\"tbox\"> - ";
    print "<input type=\"text\" name=\"month\" value=\"$month\" size=\"2\" maxlength=\"2\" class=\"tbox\"> - ";
    print "<input type=\"text\" name=\"day\" value=\"$day\" size=\"2\" maxlength=\"2\" class=\"tbox\">";
    print " (yyyy-mm-dd)";
    print "</td>\n";
    print "</tr>\n";

    print "<tr>\n";
    print "\t<td align=\"left\">Time:</td>\n";
    print "\t<td align=\"left\">";
    print "<input type=\"text\" name=\"hour\" value=\"$hour\" size=\"2\" maxlength=\"2\" class=\"tbox\"> : ";
    print "<input type=\"text\" name=\"min\" value=\"$min\" size=\"2\" maxlength=\"2\" class=\"tbox\">";
    print " (24 hour time)";
    print "</td>\n";
    print "</tr>\n";

    print "<tr>\n";
    print "\t<td align=\"left\">Subject:</td>\n";
    print "\t<td align=\"left\"><input type=\"text\" name=\"subject\" value=\"${subject}\" size=\"60\" maxlength=\"255\" class=\"tbox\"></td></tr>\n";

    print "<tr>\n";
    print "\t<td align=\"left\" valign=\"top\">Entry:</td>\n";
    print "\t<td align=\"left\"><textarea name=\"entry\" value=\"\" cols=\"60\" rows=\"10\" wrap=\"soft\" class=\"tbox\">${entry}</textarea></td>\n";
    print "</tr>\n";

    print "<tr>\n";
    print "\t<td colspan=\"2\" align=\"center\">";
    print "<input type=\"submit\" name=\"entry_edit\" value=\"Submit\" class=\"button\"></td>\n";
    print "</tr>\n";
    print "</table>\n";
    print "</form>\n";
}

function bp_edit_delete($table, $id, $filename) {
    global $idx;

    // Delete the news item
    $sql = "DELETE FROM $table WHERE id='$id'";
    $result = bp_db_query($sql, $idx);

    // Delete the associated comments
    $sql = "DELETE FROM comments WHERE replyto='$id'";
    $result = bp_db_query($sql, $idx);
    
    print "<h1>News item deleted</h1>\n";
    print "Click <a href=\"$filename\">here</a> to return.<br>\n";
}

// $Id: bp_func_edit.php,v 1.1 2005-12-15 22:36:45 bill Exp $

?>
