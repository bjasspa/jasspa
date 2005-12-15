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

// Function to connect to the database
function bp_db_connect() {
    extract($GLOBALS);
    global $idx;

    // MySQL
    if ($bp_db == "mysql") {
        mysql_connect($bp_server, $bp_dbuser, $bp_dbpass)
            or die("Unable to connect to server.");
        mysql_select_db($bp_database)
            or die("Unable to select database.");
    }
    // PostgreSQL
    elseif ($bp_db == "pgsql") {
        $idx = pg_connect("host=$bp_server dbname=$bp_database user=$bp_dbuser password=$bp_dbpass");
    }
}

// Function to query the databse with the supplied SQL statement
function bp_db_query($sql, $idx="") {
    extract($GLOBALS);

    // MySQL
    if ($bp_db == "mysql") {
        $output = mysql_query($sql)
            or die("Unable to execute query.");
    }
    // PostgreSQL
    elseif ($bp_db == "pgsql") {
        $output = pg_exec($idx, $sql)
            or die("Unable to execute query.");
    }

    return $output;
}

function bp_db_fetch_array($result, $i=0) {
    extract($GLOBALS);

    // MySQL
    if ($bp_db == "mysql") {
        $output = mysql_fetch_array($result);
    }
    // PostgreSQL
    elseif ($bp_db == "pgsql") {
        $output = pg_fetch_array($result, $i);
    }

    return $output;
}

function bp_db_num_rows($stuff) {
    extract($GLOBALS);

    // MySQL
    if ($bp_db == "mysql") {
        $rows = mysql_num_rows($stuff);
    }
    // PostgreSQL
    elseif ($bp_db == "pgsql") {
        $rows = pg_numrows($stuff);
    }

    return $rows;
}

function bp_db_cleanup($result) {
    mysql_free_result($result);
    mysql_close();
}
    
function bp_db_die() {
    die("bp_db_die()<br>: Invalid query:<br>\n".mysql_error()."<br>\n");
}
            
// $Id: bp_func_db.php,v 1.1 2005-12-15 22:36:45 bill Exp $

?>
