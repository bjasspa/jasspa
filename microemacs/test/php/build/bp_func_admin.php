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

function bp_useredit($login, $password, $email, $admin, $edit, $post) {
    global $idx;
    $sql = "UPDATE users SET login='$login', passwd='$password', email='$email', admin='$admin', edit='$edit', post='$post' WHERE login='$login'";
    $result= bp_db_query($sql, $idx);
}

function bp_useradd($login, $password, $email, $admin, $edit, $post) {
    global $idx;
    $sql = "INSERT INTO users (login, passwd, email, admin, edit, post) values ('$login', '$password', '$email', '$admin', '$edit', '$post')";
    $result = bp_db_query($sql, $idx);
}

function bp_userdel($login) {
    global $idx;
    $sql = "DELETE FROM users WHERE login='$login'";
    $result = bp_db_query($sql, $idx);
}

function bp_userinfo($login, $type) {
    global $idx;

    // Get the info for the specified user
    $sql = "SELECT * FROM users WHERE login='$login'";
    $result = bp_db_query($sql, $idx);

    $row = bp_db_fetch_array($result, 0);

    return $row["$type"];
}

function bp_listusers($filename) {
    global $idx;

    $sql = "SELECT * FROM users";
    $result = bp_db_query($sql, $idx);

    print "<table border=\"0\">\n";
    print "<tr>";
    print "<td align=\"left\" rowspan=\"2\"><b>Username</b></td>";
    print "<td align=\"left\" rowspan=\"2\"><b>Password</b></td>";
    print "<td align=\"left\" rowspan=\"2\"><b>Email</b></td>";
    print "<td align=\"left\" colspan=\"3\"><b>Permissions</b></td>\n";
    print "<td align=\"left\" rowspan=\"2\" colspan=\"2\">&nbsp;</td></tr>\n";

    print "<tr><td align=\"left\">Admin</td>";
    print "<td align=\"left\">Edit</td>";
    print "<td align=\"left\">Post</td>";

    $num = bp_db_num_rows($result);

    for ($i = 0; $i < $num; ++$i) {
        $row = bp_db_fetch_array($result, $i);
        print "<tr>";

        print "<form method=\"post\" action=\"$filename\" enctype=\"multipart/form-data\">";

        print "<td align=\"left\"><input type=\"text\" value=\"" . $row["login"] . "\" size=\"10\" name=\"login\" class=\"tbox\"></td>";
        print "<td align=\"left\"><input type=\"text\" value=\"" . $row["passwd"] . "\" size=\"10\" name=\"password\" class=\"tbox\"></td>";
        print "<td align=\"left\"><input type=\"text\" value=\"" . $row["email"] . "\" size=\"30\" name=\"email\" class=\"tbox\"></td>";

        // admin dude
        if ($row["admin"] == 1) { $form_admin = "checked"; }
        else { $form_admin = ""; }
        print "<td align=\"left\"><input type=\"checkbox\" name=\"admin\" $form_admin class=\"tbox\"></td>";

        // editing allowed
        if ($row["edit"] == 1) { $form_edit = "checked"; }
        else { $form_edit = ""; }
        print "<td align=\"left\"><input type=\"checkbox\" name=\"edit\" $form_edit class=\"tbox\"></td>";

        // posting allowed
        if ($row["post"] == 1) { $form_post = "checked"; }
        else { $form_post = ""; }
        print "<td align=\"left\"><input type=\"checkbox\" name=\"post\" $form_post class=\"tbox\"></td>";

        print "<td align=\"left\"><input type=\"submit\" name=\"submit_edit\" value=\"Edit\" class=\"button\"></td>";
        print "<td align=\"left\"><input type=\"submit\" name=\"submit_delete\" value=\"Delete\" class=\"button\"></td>";
        print "</form>";

        print "</tr>\n";
    }

    print "</table>\n";
}


// Show form for specified user, and let some changes be made
function bp_admin_showuser($login, $filename) {
    global $idx;

    $sql = "SELECT * FROM users WHERE login='$login'";
    $result = bp_db_query($sql, $idx);

    $row = bp_db_fetch_array($result, 0);

    print "<p>\n";

    print "<h2>User info for $login:</h2>\n";
    print "<form method=\"post\" action=\"$filename\" enctype=\"multipart/form-data\">\n";
    // This line seems like it is a problem - login gets reset later
    //print "<input type=\"hidden\" name=\"login\" value=\"$login\">\n";

    print "<table border=\"0\">";

    print "<tr><td align=\"left\">Username:</td>";
    print "<td align=\"left\" colspan=\"3\"><input type=\"text\" name=\"login\" value=\"$login\" class=\"tbox\"></td></tr>\n";

    print "<tr><td align=\"left\">Password:</td>";
    print "<td align=\"left\" colspan=\"3\"><input type=\"text\" name=\"password\" value=\"" . $row["passwd"] . "\" class=\"tbox\"></td></tr>\n";

    print "<tr><td align=\"left\">Email:</td>";
    print "<td align=\"left\" colspan=\"3\"><input type=\"text\" name=\"email\" value=\"" . $row["email"] . "\" class=\"tbox\"></td></tr>\n";

    print "<tr><td align=\"left\">Permissions:</td>";

    // admin dude
    if ($row["admin"] == 1) { $form_admin = "checked"; }
    else { $form_admin = ""; }
    print "<td align=\"left\"><input type=\"checkbox\" name=\"admin\" $form_admin class=\"tbox\">Admin</td>";

    // editing allowed
    if ($row["edit"] == 1) { $form_edit = "checked"; }
    else { $form_edit = ""; }
    print "<td align=\"left\"><input type=\"checkbox\" name=\"edit\" $form_edit class=\"tbox\">Edit</td>";

    // posting allowed
    if ($row["post"] == 1) { $form_post = "checked"; }
    else { $form_post = ""; }
    print "<td align=\"left\"><input type=\"checkbox\" name=\"post\" $form_post class=\"tbox\">Post</td>";

    print "</tr>\n";

    print "</table>\n";

    print "<input type=\"submit\" name=\"submit_editself\" value=\"Update Info\" class=\"button\">\n";
    print "<input type=\"reset\" value=\"Reset\" class=\"button\">\n";

    print "</form>\n";
}

// Show form for adding a user to the database
function bp_admin_adduser($filename) {

    print "<p>\n";

    // Add a new user
    print "<h2>Add a user:</h2>\n";
    print "<form method=\"post\" action=\"$filename\" enctype=\"multipart/form-data\">\n";
    print "<input type=\"hidden\" name=\"table\" value=\"news\">\n";
    // This line seems like it is a problem - login gets reset later
    //print "<input type=\"hidden\" name=\"login\" value=\"$login\">\n";

    print "<table border=\"0\">";

    print "<tr><td align=\"left\">Username:</td>";
    print "<td colspan=\"3\"><input type=\"text\" name=\"login\" class=\"tbox\"></td></tr>\n";

    print "<tr><td align=\"left\">Password:</td>";
    print "<td colspan=\"3\"><input type=\"password\" name=\"password\" class=\"tbox\"></td></tr>\n";

    print "<tr><td align=\"left\">Confirm password:</td>";
    print "<td colspan=\"3\"><input type=\"password\" name=\"confirm_password\" class=\"tbox\"></td></tr>\n";

    print "<tr><td align=\"left\">Email:</td>";
    print "<td colspan=\"3\"><input type=\"text\" name=\"email\" class=\"tbox\"></td></tr>\n";

    print "<tr><td align=\"left\">Permissions:</td>";
    print "<td align=\"left\"><input type=\"checkbox\" name=\"admin\" class=\"tbox\">Admin</td>";
    print "<td align=\"left\"><input type=\"checkbox\" name=\"edit\" class=\"tbox\">Edit</td>";
    print "<td align=\"left\"><input type=\"checkbox\" name=\"post\" class=\"tbox\">Post</td>";

    print "</tr>\n";

    print "</table>\n";

    print "<input type=\"submit\" name=\"submit_add\" value=\"Add User\" class=\"button\">\n";
    print "<input type=\"reset\" value=\"Reset\" class=\"button\">\n";
    print "</form>\n";
}

// $Id: bp_func_admin.php,v 1.1 2005-12-15 22:36:45 bill Exp $
?>
