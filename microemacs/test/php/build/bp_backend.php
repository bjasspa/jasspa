<?php

// IMPORTANT: set this to the full path of where
//   backend.php, template.php, and config.php exist,
//   with a trailing /
define("BASE", "");
//define("BASE", "bplog/");

include(BASE."bp_config.php");      // Configuration stuff
include(BASE."bp_func_db.php");     // Database functions
include(BASE."bp_func_admin.php");  // Admin functions
include(BASE."bp_func_edit.php");   // Editing functions

$idx = bp_db_connect();

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

function bp_display($sql, $template) {
    include(BASE.$template);

    global $idx;

    $result = bp_db_query($sql, $idx);

    bp_display_tablehead();

    // Set last used date to 0 so we will always get at least one date displayed
    $ts_last = 0;

    $num = bp_db_num_rows($result);

    for ($i = 0; $i < $num; ++$i) {
        $row = bp_db_fetch_array($result, $i);

        // Convert the date column to UNIX time format
        // $row["date"] -> 2001-12-25 14:25:00
        $ts = strtotime($row["date"]);

        bp_display_entry($ts_last, $ts, $row["id"], $row["subject"], $row["login"], $row["entry"]);

        // Store the date to compare with the next entry
        $ts_last = date("mdY", $ts);
    }

    bp_display_tablefoot();
}

function bp_last($table, $num, $template) {
    $sql = "SELECT * FROM $table ORDER BY id DESC LIMIT $num";
    bp_display($sql, $template);
}

function bp_archived($table, $year, $month, $template) {
    $sql = "SELECT * FROM $table WHERE date LIKE '$year-$month%' ORDER BY id DESC";
    bp_display($sql, $template);
}

function bp_add_entry($table, $subject, $entry, $login, $year, $month, $day, $hour, $min) {

    // Do any error checking in the addnews part
    // That way, you can set your own defaults for missing variables

    global $idx;

    $subject = myAddSlashes($subject);
    $entry = myAddSlashes($entry);

    $ts = bp_datecheck($year, $month, $day, $hour, $min);

    $sql = "INSERT INTO $table (date, subject, entry, login) VALUES ('$ts', '$subject', '$entry', '$login')";

    extract($GLOBALS);
    $result = bp_db_query($sql, $idx);
}

function bp_entry_edit($table, $id, $subject, $entry, $login, $year, $month, $day, $hour, $min) {
    global $idx;

    $subject = myAddSlashes($subject);
    $entry = myAddSlashes($entry);

    // Check for invalid dates
    $ts = bp_datecheck($year, $month, $day, $hour, $min);

    $sql = "UPDATE $table SET date='$ts', subject='$subject', entry='$entry', login='$login' WHERE id='$id'";

    $result = bp_db_query($sql, $idx);
}

function bp_auth($user, $pass) {
    global $idx;

    $sql = "SELECT * FROM users WHERE login='$user' AND passwd='$pass'";
    $result = bp_db_query($sql, $idx);

    $num = bp_db_num_rows($result);
    if ($num != 0) {
        global $auth, $auth_admin, $auth_edit, $auth_post;
        $auth = true;

        // Do checking for user permissions (admin/edit/post)
        $row = bp_db_fetch_array($result, 0);

        if ($row["admin"]) { $auth_admin = 1; }
        if ($row["edit"]) { $auth_edit = 1; }
        if ($row["post"]) { $auth_post = 1; }
    }
}

function bp_html_header($title) {
    extract($GLOBALS); // For $bp_ver variable

    print <<< EOT
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
<head>
    <title>$title</title>
    <meta name="Generator" content="bplog $bp_ver">
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8">
    <link rel="stylesheet" href="bplog.css" type="text/css" title="Default">
</head>
<body>
EOT
;
}

function bp_html_footer() {
    extract($GLOBALS); // For $bp_ver variable

    print <<< EOT
<hr>

<table width="100%">
<tr>
<td class="bpfooter" align="left">&nbsp;</td>
<td class="bpfooter" align="right">
Powered by: <a href="http://bplog.blackplasma.net/">bplog $bp_ver</a></td>
</tr>
</table>

</body>
</html>
EOT
;
}

function bp_numcomments($id) {
    // Gets the number of comments for a particular news item
    global $idx;

    $sql = "SELECT * FROM comments WHERE replyto='$id'";
    $result = bp_db_query($sql, $idx);

    $num = bp_db_num_rows($result); // Number of comments

    return $num;
}

function bp_datecheck($year, $month, $day, $hour, $min) {
    $ts_check = 0;  // Check for invalid dates (set to 1 if a date is invalid)

    // Check to see if a valid date was submitted
    if (($month < 1) || ($month > 12)) {
        $ts_check = 1;
    }
    if (($day < 1) || ($day > 31)) {
        $ts_check = 1;
    }
    if (($hour < 0) || ($hour > 23)) {
        $ts_check = 1;
    }
    if (($min < 0) || ($min > 59)) {
        $ts_check = 1;
    }

    if ($ts_check == 1) {
        // Invalid time
        $ts = date("Y-m-d H:i:s");
    } else {
        // Valid time
        $ts = "$year-$month-$day $hour:$min:00";
    }

    return $ts;
}

function bp_getmonths($table="news") {
    /* ====================================================
     * Configuration:
     *
     * $bp_conf_textmonth
     *   0 - Display months as 01, 02, 03, etc
     *   1 - Display months as Jan, Feb, Mar, etc
     * ====================================================
     */
    $bp_conf_textmonth = 1;

    // Generate links for months with entries
    global $idx;

    // Find out what the earliest entry is
    $sql = "SELECT * FROM $table ORDER BY date ASC LIMIT 1";
    $result = bp_db_query($sql, $idx);

    $row = bp_db_fetch_array($result, 0);
    $first = $row["date"];
    $ts = @strtotime($first);
    $year = strftime("%Y", $ts);
    $month = strftime("%m", $ts);

    // Figure out how to display the link
    if ($bp_conf_textmonth == 1) {
        $linktext = bp_textmonth($month) . " " . $year;
    }
    else {
        $linktext = "$year-$month";
    }

    print "<b>Archive:</b><br>";
    print "&nbsp;&nbsp;<a href=\"?year=$year&amp;month=$month\">$linktext</a>";

    // Now generate links for months up to the current one
    $sql = "SELECT * FROM $table ORDER BY date ASC";
    $result = bp_db_query($sql, $idx);

    $dec = 0; // check for December
    $lastmonth = $month;
    $lastyear = $year;
    if ($lastmonth == 12) {
        $dec = 1;
    }

    $num = bp_db_num_rows($result);
    for ($i = 0; $i < $num; ++$i) {
        $row = bp_db_fetch_array($result, $i);
        $ts = strtotime($row["date"]);
        $year = strftime("%Y", $ts);
        $month = strftime("%m", $ts);

        // Figure out how to display the link
        if ($bp_conf_textmonth == 1) {
            $linktext = bp_textmonth($month) . " " . $year;
        }
        else {
            $linktext = "$year-$month";
        }

        if ($year > $lastyear) { // if it's a new year
            print "<br>";
            print "&nbsp;&nbsp;<a href=\"?year=$year&amp;month=$month\">$linktext</a>";

            $lastmonth = $month;
            $lastyear = $year;

            if ($month == 12) {
                $dec = 1;
                $year++;
            } else {
                $dec = 0;
            }

            $month++;

        } elseif ($month > $lastmonth) {
            print " | <a href=\"?year=$year&amp;month=$month\">$linktext</a>";

            $lastmonth = $month;
            $lastyear = $year;

            if ($month == 12) {
                $dec = 1;
                $year++;
            } else {
                $dec = 0;
            }

            $month++;
        }

    }
}

function bp_textmonth($month) {
    // Converts a numeric month to a text month (mainly for the archive links)
    $textmonth = array(
        '01' => 'Jan',
        '02' => 'Feb',
        '03' => 'Mar',
        '04' => 'Apr',
        '05' => 'May',
        '06' => 'Jun',
        '07' => 'Jul',
        '08' => 'Aug',
        '09' => 'Sep',
        '10' => 'Oct',
        '11' => 'Nov',
        '12' => 'Dec'
    );

    return $textmonth[$month];
}

// $Id: bp_backend.php,v 1.1 2005-12-15 22:36:44 bill Exp $

?>
