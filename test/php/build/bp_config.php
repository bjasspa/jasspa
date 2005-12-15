<?php

// IMPORTANT: edit backend.php and set the BASE define at the top

global $bp_server, $bp_dbuser, $bp_dbpass, $bp_database, $bp_db, $bp_ver;

$bp_server   = "vast.ptcnet.ptc.com";   // MySQL server to connect to
$bp_dbuser   = "root";                  // Username on the MySQL server
$bp_dbpass   = "";                      // Password on the MySQL server
$bp_database = "VIS";                   // Database to use (uses the "news" table)

$bp_db = "mysql";                       // choose one of: mysql, pgsql

$bp_ver = "0.5.8";                      // bplog version, no need to change

// $Id: bp_config.php,v 1.1 2005-12-15 22:36:45 bill Exp $

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

?>
