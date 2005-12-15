<?php
 
/* $Id: db.inc,v 1.1 2005-12-15 22:36:46 bill Exp $ */

function make_formDropDown($select_name, $query)
{
  ?>
  <select class="tbox" name=<?php echo $select_name ?>>
  <?php
  $result = dbConnect($query);
  while ($row = mysql_fetch_object($result)) {
    ?>
    <option value=<?php echo preg_replace("/( |-|_)/","",$row->$select_name); ?>><?php echo $row->$select_name ?></option>
    <?php
  }
  dbFreeResult($result);
  ?></select><?php
}

function print_formDropDown($select_name, $query) {
    echo make_formDropDown($select_name, $query);
}

function dbConnect($query)
{
  require_once 'config.inc';
  $db = mysql_connect($cfgServers[1]['host'],$cfgServers[1]['user'],$cfgServers[1]['password']) or die ("could not connect to the server");
  mysql_select_db("VIS",$db) or die ("could not select database");
  $result = mysql_query($query,$db) or die ("could not query database with $query");
  if (!$result) {
    echo mysql_error();
    exit;
  }
  return $result;
}

function dbFreeResult($result)
{
  mysql_free_result($result);
}

?>
