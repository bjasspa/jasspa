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

function bp_display_tablehead() {
    print "<table width=\"100%\">\n";
}

function bp_display_tablefoot() {
    print "</table>\n";
}

function bp_display_entry($ts_last, $ts, $id, $subject, $login, $entry) {
    global $URLBASE;
    $bp_conf_singledate = 1; // Set to 0 if you want the date to be shown for every entry
    $bp_conf_date = "l, F j, Y";

    // Blank line
    print "<tr><td align=\"center\" colspan=\"2\">&nbsp;</td></tr>\n";

    print "<tr><td align=\"left\" colspan=\"2\">";
    // Subject
    print "<font class=\"newsHead\">";
    printf("%s", myStripSlashes($subject));
    print "</font>\n";
    // Date
    if (($ts_last != date("mdY", $ts)) && ($bp_conf_singledate == 1)) {
        print "<br><font class=\"newsDate\">[";
        printf("%s", date($bp_conf_date, $ts)); // Format the date according to config.php
        print "]</font>\n";
    }
    print "</td></tr>";
    
    // Blank line
    print "<tr><td align=\"center\" colspan=\"2\">&nbsp;</td></tr>\n";

    $bp_conf_time = "%H:%M:%S %Z";
    $bp_conf_showuser = 1; // Set to 0 if you don't want to show the poster's login

    // Entry
    print "<tr><td align=\"left\" colspan=\"2\" class=\"bpentry\">";
    printf("%s", nl2br(myStripSlashes($entry)));
    print "</td></tr>\n";

    // Blank line
    print "<tr><td align=\"center\" colspan=\"2\">&nbsp;</td></tr>\n";

    print  "<tr><td align=\"left\"><font class=\"copy\">";

    print(strftime("Posted @ $bp_conf_time", $ts));
    if (($login) && ($bp_conf_showuser)) {
        $email = bp_userinfo($login, 'email');
        if ($email) {
            print " by <a href=\"mailto:$email\">$login</a>";
        } else {
            print " by $login";
        }
    }

    print  "</font></td>\n";
    print  "<td align=\"right\"><font class=\"copy\">";

    // Comment link
    $numcomments = bp_numcomments($id);     // Set once so we don't have to query the db as much

    print " ( ";

    // read comments link
    if ($numcomments != 0) {
        // There are comments, generate a "read comments" link, depending on the number
        print "<a href=\"${URLBASE}/tools/bp_comment.php?display=$id\">";
        if ($numcomments == 1) {
            print "1 comment";
        } else {
            print "$numcomments comments";
        }
        print "</a> | ";
    }

    // post comments link
    print "<a href=\"${URLBASE}/tools/bp_comment.php?replyto=$id\">post comment</a> )\n";

    print  "</font></td></tr>\n";

    // Blank line
    print "<tr><td align=\"center\" colspan=\"2\">".hdelim()."</td></tr>\n";
}

// $Id: bp_template.php,v 1.1 2005-12-15 22:36:45 bill Exp $

?>
