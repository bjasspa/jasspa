<?
/**
 * Funktionssammlung
 */

include_once 'include/constants.php';
include_once 'include/history.inc.php';
include_once 'include/config.php';

/**
 * updates a todo-entry
 *
 * Updates a Todo-Entry into database and generates a mail-notify to subscribed users.
 *
 * @param $due_date		string	the due date
 * @param $priority
 * @param $status
 * @param $percentage_completed
 * @param $text
 * @param $responsible_persons
 * @param $id
 * @param $note_text
 * @param $selected_project
 */
function update_todo($due_date, $priority, $status, $percentage_completed, 
		$text, $responsible_persons, $id, $note_text, $selected_project) {
	global $db,$DATEFORMAT, $HTTP_SESSION_VARS, $DEBUG;
	/* Das Datum muß evtl. wieder nach US-Format konvertiert werden... */
	if ($DATEFORMAT != "1") {
		$due_date=convDateToUS($due_date);
	}
	$priority = switchPriority($priority);

	$query = "UPDATE todo_todos SET todo_priority=$priority, status=$status, project_id=$selected_project, "
		."percentage_completed=$percentage_completed, todo_text='".addslashes($text)."', "
		."due_date='$due_date', date_changed=NOW(), "
		."changed_by=" . $HTTP_SESSION_VARS['usr']->userid . " where todo_id='$id'";

	$querystatus = $db->query($query);

	// update responsible_persons
	if ((count($responsible_persons) > 0) && ($querystatus == true)) {

		// delete old responsibilities
		if (!$db->query("DELETE from todo_responsible_persons WHERE todo_id=$id")) {
			return false;
		}

		$query = "INSERT INTO todo_responsible_persons VALUES ";

		$anzahl = count($responsible_persons);
		for ($i=0; $i < $anzahl ; $i++) {
			$query .= "($id, $responsible_persons[$i])";
			if (($i+1) < $anzahl)
				$query .= ", ";
		}

		$querystatus = $db->query($query);
	}

	// if new note entered insert it into DB
	if (($note_text != "") && ($querystatus == true)) {
		// not needed.
		// $db->query("UPDATE todo_todos set date_changed=NOW() where todo_id='$id'");
		$querystatus = $db->query("INSERT INTO todo_notes (todo_id, text, usernr, date) VALUES
				('$id', '" .addslashes($note_text)."', ".$HTTP_SESSION_VARS['usr']->userid ." , NOW())");
	}

	$userfile = $_FILES['userfile'];
	$filename = $userfile['name'];
	if(($userfile != "") && ($filename != "") && ($querystatus == true)) {
		$filetype = $userfile['type'];
		$filesize = $userfile['size'];
		$addfile = $userfile['tmp_name']; 

		if($filesize < 786432) {
			$addQuery = 'insert into todo_files (todo_id, name, type, size, ' .
				"time) values($id, '" . addslashes($filename) . "', '" . 
				addslashes($filetype) .	"' ," . addslashes($filesize) . 
				",  NOW())";

			$querystatus = $db->query($addQuery);
			if($querystatus == true) {
				$filehandle = fopen($addfile, "r");
				$buffer = addslashes(fread($filehandle, 906240)); 
				$fileid = mysql_insert_id($db->link_id());
				
				$addQuery = "update todo_files set data = concat('" . $buffer . 
					"') where id = $fileid";

				$querystatus = $db->query($addQuery);
				fclose($filehandle);
			}
		}
		else 
			print "<br>File to largei, 768K limit<br>";
	}

	if ($DEBUG) {
		echo "-------------------- update_todo finish --------------------";
	}

	generateMail($id, "todo_change", $HTTP_SESSION_VARS['usr']->userid);
	return ($querystatus);
} 

/**
 * add a todo-entry
 *
 * Inserts a Todo-Entry into database and generates a mail-notify to subscribed users.
 *
 * @param $due_date		string	the due date
 * @param $priority
 * @param $project
 * @param $percentage_completed
 * @param $text
 * @param $responsible_persons
 * @param $note_text
 * @param $parent_task	int	The parent-task (set if this a subtask should be created.)
 */
function add_todo($due_date,$priority,$project,$text,$responsible_person,$parent_task=0, $level=0){
	global $db,$DATEFORMAT, $HTTP_SESSION_VARS;
	if ($DATEFORMAT != "1") {
		// datum is the due date
		$due_date = convDateToUS($due_date);
	}
	$priority = switchPriority($priority);

	if ($HTTP_SESSION_VARS['usr']->userid) {
		$query  = 'INSERT INTO todo_todos';
		$query .= "(project_id, todo_text,todo_priority, percentage_completed, created_by, due_date,";
		$query .= 'date_created, date_changed, changed_by';

		if ($parent_task != 0) {
			$query .= ', parent, level';
		}

		$query .=") VALUES (".$project.',"'.addslashes($text).'", '.$priority.', 0, '.$HTTP_SESSION_VARS['usr']->userid.',
		"'.$due_date.'",NOW(),NOW(), '.$HTTP_SESSION_VARS['usr']->userid;

		if ($parent_task != 0) {
			$query .= ','.$parent_task.','.$level;
		}

		$query .=')';

		//echo $query;
		if ($db->query($query)){
			$insert_success = true;
		} else {
			$insert_success = false;
		}
		//echo "responsible_person: " . count($responsible_person);
		if ((count($responsible_person) > 0) && ($insert_success == true)) {
			$query = "INSERT INTO todo_responsible_persons VALUES ";

			$anzahl = count($responsible_person);
			for ($i=0; $i < $anzahl ; $i++) {
				$query .= "(LAST_INSERT_ID(), $responsible_person[$i])";
				if (($i+1) < $anzahl)
					$query .= ", ";
			}

			// echo "<br> 2. Query: $query";
			if ($db->query($query)){
				$insert_success = true;
			} else {
				$insert_success = false;
			}
			generateMail("LAST_INSERT_ID()", "todo_add",$HTTP_SESSION_VARS['usr']->userid);
		}

	} else {
		// not authenticated - don't allow them to add it.
		echo 'Authentication Failure!';
		return false;
	}

	return $insert_success;
} // end of add_todo

/**
 * deletes a todo-entry
 *
 * Deletes a Todo-Entry, and all associated notes. A mail-notify is not generated!
 *
 * @param $todo_id	int	the primary key of the todo-entry
 */
function delete_todo($todo_id) {
	global $db, $HTTP_SESSION_VARS;

	// Check if the user is allowed to delete the task. This is a hack/workaround until a true
	// ACL/Security-system exists.
	if (! $HTTP_SESSION_VARS['usr']->isAdmin()) { 
		// You are not admin ... 
		$db->query("SELECT project_leader FROM todo_projects tp,todo_todos tt
				WHERE tt.todo_id=$todo_id
				AND tt.project_id=tp.id");
		if ($db->next_record()){ 
			if ($db->f("project_leader") != $userid) { 
				// ... nor the project leader 
				$db->query("SELECT user_id from 
						todo_responsible_persons
						WHERE todo_id=$todo_id
						AND user_id=". $HTTP_SESSION_VARS['usr']->getUserid()); 
					if (! $db->next_record()){ 
						// ... nether the responsible person for that task!
						// you can not delete it. 
						return false; 
					} // else ...You are the responsible person for that task.
				//	you can proceed and delete it. 
				return false; 
			} // else You're the project-leader. Carry on.
		} else { 
			// no project-leader assigned, this is a DB inconsistency. 
				echo "DB inconsistency todo_responsible_persons for todo_id: $todo_id!"; 
				return false; 
		} 
	} 

	if ($HTTP_SESSION_VARS['usr']->userid) {
		// Delete the subtasks recursively
		$db->query("SELECT todo_id from todo_todos where parent=$todo_id");
		while ($db->next_record()){
			delete_todo($db->f("todo_id"));
		}

		// TODO: Add mail-notification
		generateMail($todo_id, "todo_delete",$HTTP_SESSION_VARS['usr']->userid);
		$db->query("DELETE FROM todo_notes WHERE todo_id=$todo_id");
		$db->query("DELETE FROM todo_responsible_persons WHERE todo_id=$todo_id");
		$db->query("DELETE FROM todo_responsible_groups WHERE todo_id=$todo_id");
		$db->query("DELETE FROM todo_todos WHERE todo_id=$todo_id");
	} else {
		// not authenticated - don't allow them to delete it.
		echo 'Authentication Failure!';
		return false;
	}

	return true;
} // end of delete_todo


/**
 * deletes a project
 *
 * Deletes a whole project. A mail-notify is not generated!
 *
 * @param $project_id	int	the primary key of the project
 */
function delete_project($project_id) {
	// TODO: We have to make user only ADMIN and Project-Admins can do that!
	global $db, $HTTP_SESSION_VARS;
	if ($HTTP_SESSION_VARS['usr']->userid) {
		$db->query("DELETE FROM todo_projects WHERE id=$project_id");
		$db->query("DELETE FROM todo_project_members WHERE project_id=$project_id");
		// get the todo_ids that have notes attached and construct a query
		$db->query("SELECT DISTINCT n.todo_id
				FROM todo_todos t, todo_notes n
				WHERE t.todo_id=n.todo_id AND t.project_id=$project_id");
		while ($db->next_record()){
			$notes[] = $db->f("todo_id");
		}

		// If notes found -> delete
		if (sizeof($notes) > 0) {
			$query="DELETE from todo_notes WHERE todo_id in (";

			while ($neu=array_pop($notes)){
				$query .= $neu;
				if (sizeof($notes) > 0) {
					$query .= ',';
				} else {
					$query .= ')';
				}
			}
			$db->query("$query");
		}
		// delete all tasks assigned to the project.
		$db->query("DELETE FROM todo_todos WHERE project_id=$project_id");
	} else {
		// not authenticated - don't allow them to delete it.
		echo 'Authentication Failure!';
		return false;
	}

	return true;
} // end of delete_project


/**
 * deletes a group
 *
 * Deletes a whole group. A mail-notify is not generated!
 *
 * @param $group_id	int	the primary key of the group
 */
function delete_group($group_id) {
	// TODO: We have to make user only ADMIN and group-leader can do that!
	global $db, $HTTP_SESSION_VARS;
	if ($HTTP_SESSION_VARS['usr']->userid) {
		$db->query("DELETE FROM todo_groups WHERE id=$group_id");
		$db->query("DELETE FROM todo_group_members WHERE group_id=$group_id");
		$db->query("DELETE FROM todo_responsible_groups WHERE group_id=$group_id");
	} else {
		// not authenticated - don't allow them to delete it.
		echo 'Authentication Failure!';
		return false;
	}

	return true;
} // end of delete_group


/**
 * deletes a user
 *
 * Deletes a user.
 *
 * @param $user_id	int	the primary key of the group
 */
function delete_user($user_id) {
	global $db, $HTTP_SESSION_VARS;
	if ($HTTP_SESSION_VARS['usr']->userid) {
		$db->query("DELETE FROM todo_users WHERE usernr=$user_id");
		$db->query("DELETE FROM todo_responsible_persons WHERE user_id=$user_id");
		$db->query("DELETE FROM todo_group_members WHERE member_id=$user_id");
		$db->query("DELETE FROM todo_project_members WHERE member_id=$user_id");
		$db->query("DELETE FROM todo_notes WHERE usernr=$user_id");
	} else {
		// not authenticated - don't allow them to delete it.
		echo 'Authentication Failure!';
		return false;
	}

	return true;
} // end of delete_user


/**
 * creates the SQL-Query to retrieve the main todo-table
 *
 * @param $user_id			int		the primary key of the group
 * @param $order_by			string	How should the table be ordered?
 * @param $selected_project	int		which project should be shown
 * @param $parent			int		should this be subtasks?
 *
 * @returns string the Query
 */
function makeFrontQuery($order_by, $selected_project, $parent=0){
	global $db, $HTTP_SESSION_VARS;

	$query='SELECT todo_todos.*, count(distinct(todo_notes.note_id)) AS nr_notes 
		FROM todo_todos, todo_responsible_persons, todo_project_members
		LEFT JOIN todo_notes ON todo_todos.todo_id=todo_notes.todo_id
		LEFT JOIN todo_projects on todo_todos.project_id=todo_projects.id
		WHERE status < 6
		AND todo_todos.todo_id=todo_responsible_persons.todo_id';

	if ($parent==0) {
		/* Don't show subtasks in the main-table
		 * TODO: Make this possible! */
		 if (($HTTP_SESSION_VARS['usr']->showMyTasks()) &&
			 ($HTTP_SESSION_VARS['usr']->showChildren())) {
			 	$query .= '';
		 } else {
			$query .= ' AND parent=0';
		 }
	} else {
		$query .= ' AND parent='.$parent;
	}

	if ($selected_project != "all") {
		$query .= ' AND todo_todos.project_id='.$selected_project;
	} else {
		$db->query('SELECT project_id FROM todo_project_members WHERE member_id='.$HTTP_SESSION_VARS['usr']->userid);


		while ($db->next_record()){
			$projects[] = $db->f("project_id");
		}
		if ($projects[0]!="") {
			$query.=' AND todo_todos.project_id in (';

					while ($neu=array_pop($projects)){
					$query .= $neu;
					if (sizeof($projects) > 0)
					$query .= ',';
					else
					$query .= ') ';
					}
		}

	}

	if ($HTTP_SESSION_VARS['usr']->showMyTasks()) {
		// show only tasks where I'm responsible for
		$query .= ' 
			AND todo_responsible_persons.user_id = '. $HTTP_SESSION_VARS['usr']->userid.'
			AND todo_todos.todo_id = todo_responsible_persons.todo_id';
	}

	$query .= ' GROUP BY todo_id ';
	$query .= orderBy($order_by);

	return $query;
}
//end makeFrontQuery


/**
 * creates the SQL-Query for the search-form
 *
 * @param $wildcards					the primary key of the group
 * @param $priority						priority to search for
 * @param $status						the status to search for
 * @param $project
 * @param $responsible_persons
 * @param $order_by				string	How should the table be ordered?
 * @param $date_min
 * @param $date_max
 */
function makeSearchQuery($wildcards,$priority, $status, $project, $responsible_persons,$order_by,$date_min,$date_max){
	global $DATEFORMAT, $HTTP_GET_VARS, $db, $HTTP_SESSION_VARS;

	/* Generate the SQL-Statement */
	$query='SELECT todo_todos.*, count(todo_notes.todo_id) AS nr_notes
		FROM todo_todos, todo_responsible_persons rp
		LEFT JOIN todo_notes ON todo_todos.todo_id=todo_notes.todo_id
		WHERE todo_todos.todo_text LIKE ';

	if ($wildcards) {
		$query=$query . "'%".$HTTP_GET_VARS['abfrage']."%' "; 
	} else {
		$query=$query . "'".$HTTP_GET_VARS['abfrage']."' "; 
	}

	if ($priority != ""){
		$query=$query . "AND todo_PRIORITY=$priority "; 
	}

	if ($status != "" && $status != "-1"){
		$query=$query . "AND status=$status "; 
	}

	if ($project != "") {
		if ($project != "all") {
			$query=$query . "AND project_id=$project "; 
		} else {
			//MARC work to do here.  Ensure we see everything
			$db->query('SELECT project_id FROM todo_project_members WHERE member_id='.$HTTP_SESSION_VARS['usr']->userid);


			while ($db->next_record()){
				$projects[] = $db->f("project_id");
			}
			if ($projects[0]!="") {
				$query.=' AND project_id in (';

						while ($neu=array_pop($projects)){
						$query .= $neu;
						if (sizeof($projects) > 0)
						$query .= ',';
						else
						$query .= ') ';
						}
			}

		}
	}

	if ($DATEFORMAT != "1" ) {
		$date_min=convDateToUS($date_min);
		$date_max=convDateToUS($date_max);
	}

	if ($date_min && $date_min != "0000-00-00") {
		$query .= "AND date_changed >= '$date_min' ";
	}
	if ($date_max && $date_max != "0000-00-00") {
		$query .= "AND date_changed <= '$date_max 23:59:59' ";
	}

	print $responsible_persons[1] . "<br>";

	if ($responsible_persons[0]!="") {
		$query.=' AND rp.user_id in (';

				while ($neu=array_pop($responsible_persons)){
				$query .= $neu;
				if (sizeof($responsible_persons) > 0)
				$query .= ',';
				else
				$query .= ') ';
				}
	}
	$query .= 'AND rp.todo_id=todo_todos.todo_id';

	$query=$query . " GROUP BY todo_id ";

	// How should the table be ordered?
	$query .= orderBy($order_by);
	return $query;
}
// end makeSearchQuery

function createUserMap() {
	global $db;

   //MARC you can optimize this better later
   $query = "select distinct(rp.todo_id), first_name, last_name, login_name " .
      "from todo_users u, todo_responsible_persons rp, todo_todos t " .
      "WHERE t.todo_id = rp.todo_id AND u.usernr = rp.user_id ";
   $db->query($query);
   while ($db->next_record()) {
      // give login-name as "last_name" if first and last name are not set.
	   if ( $db->f('last_name') == '' && $db->f('first_name') == '' ) {
	      $responsible_users[] = array ($db->f("todo_id"),
		      $db->f("login_name"), "");
	   } else {
         $responsible_users[] = array ($db->f("todo_id"),
            $db->f("first_name"), $db->f("last_name"));
      }
   }

	return $responsible_users;
}

function createProjectMap() {
   global $db;

   // Get all the project-names in an array
   $db->query('select id, project_name from todo_projects');
   while ($db->next_record()) {
      $projects[$db->f('id')] = stripslashes($db->f('project_name'));
   }
	return $projects;						
}

function buildNameString($id, $responsible_users_arr) {
	reset ($responsible_users_arr);
	$respstr = "";
   while (@list($k,$v) = @each($responsible_users_arr)) {
	   if ($v[0] == $id)
	   	$respstr .= $v[1] . " " . $v[2]. ", ";
	}
	return substr($respstr,0,-2);
}

function createMainTable($usr, $parent = 0) {
   // list completed entries also
   $query = "SELECT todo_todos.*, " .
      "count(distinct(todo_notes.note_id)) AS nr_notes " .
      "FROM todo_todos, todo_projects, " .
      "todo_responsible_persons, todo_project_members " .
      "LEFT JOIN todo_notes ON " .
      "todo_todos.todo_id=todo_notes.todo_id ";

   $query .= "WHERE todo_todos.project_id = " .
      "todo_project_members.project_id " .
      "AND todo_todos.project_id = todo_projects.id ";

	if ($usr->selected_project != "all") {
      $query .= ' AND todo_todos.project_id = ' . $usr->selected_project;
   }

   // List done tasks or not
	switch($usr->show_done) {
	case 0:
      $query .= ' AND status not in (10, 8)';
	   break;
   case 2:
      $query .= ' AND status in (3, 6, 8, 10)';
	   break;
   case 3:
      $query .= ' AND status in (1, 0, 4, 9, 2, 7)';
      break;
	case 4:
      $query .= ' AND status in (8, 10)';
		break;
	default:
		break;
   }
															  
   if ($usr->show_children == 0) {
      // Don't show subtasks in the main-table
      // TODO: Make this possible! 
      //$query .= ' AND parent IS null';
      $query .= ' AND todo_todos.parent = 0';
   }

   if($usr->showMyTasks()) {
	   // show only tasks where I'm responsible for
	   $query .= ' AND todo_responsible_persons.user_id = ' .
	   $usr->userid;
	}
	
	$query .= ' AND todo_todos.todo_id = todo_responsible_persons.todo_id' .
		' GROUP BY todo_id ' . orderBy($usr->order_by);

//	print "Query : $query";

	return $query;
}

/**
 * generates the login-form-box
 */
function authenticate() {
	global $TEXT_SUBMIT, $TEXT_LOGIN_NAME, $TEXT_PASSWORD, $TEXT_LOGIN_REMEMBER_USER, $TODO_HEADING;
	global $HTTP_COOKIE_VARS, $HTTP_SERVER_VARS, $HTTP_GET_VARS;

	echo page_top("$TODO_HEADING <br/>Login", LOGIN);

	if (!isset($HTTP_COOKIE_VARS['userkeks'])) {
		$HTTP_COOKIE_VARS['userkeks'] = '';
	}
	/*
	if (isset($HTTP_GET_VARS['action']) && $HTTP_GET_VARS['action']=='logout') {
		$target=$HTTP_SERVER_VARS['PHP_SELF'];
	} else {
		$target=$HTTP_SERVER_VARS['REQUEST_URI'];
	}
		
		<form method="post" name="loginForm" action="<?php echo $target; ?>">
		*/

	?>
		<form method="post" name="loginForm" action="<?php echo $HTTP_SERVER_VARS['PHP_SELF']; ?>">
		<table width="100%" cellpadding="0" cellspacing="0" summary="login-form">
		<tr><td height="40"></td></tr>
		<tr>
		<td class="fmu" width="47%" align="right"><?php echo $TEXT_LOGIN_NAME; ?></td>
		<td width="10"/>
		<td><input type="text" name="todouser" value="<? echo $HTTP_COOKIE_VARS['userkeks']; ?>" /></td>
		</tr>
		<tr><td height="10"></td></tr>
		<tr>
		<td class="fmu" width="47%" align="right"><?php echo $TEXT_PASSWORD; ?></td>
		<td width="10"/>
		<td><input type="password" name="todopass" /></td>
		</tr>
		<tr><td height="10"></td></tr>
		<td class="fmu" width="47%" align="right"><?php echo $TEXT_LOGIN_REMEMBER_USER; ?></td>
		<td width="10"/>
		<td><input type="checkbox" name="remember_me" <? if (isset($HTTP_COOKIE_VARS['userkeks'])) { echo 'checked="checked"'; } ?>/></td>
		</tr>
		<tr><td height="10"></td></tr>
		<tr>
		<td width="47%">&nbsp;</td>
		<td width="10"/>
		<td><input type="submit" name="button" value="<? echo $TEXT_SUBMIT; ?>" /></td>
		</tr>
		<tr><td height="40"></td></tr>
		</table>
		</form>
		<?
		page_foot(LOGIN);
	exit;
}

/**
 * switches priority-notation from string to int and vice-versa
 *
 * TODO: get rid of this somehow...
 *
 * @param $priority	mixed	The prio represented as string or int
 * @return mixed The switched value
 */
function switchPriority($priority) {
	global $TEXT_PRIORITY_HIGH, $TEXT_PRIORITY_MEDIUM, $TEXT_PRIORITY_LOW;

	switch ($priority)
	{
		case "$TEXT_PRIORITY_HIGH":
			$priority = 1;
		break;

		case "$TEXT_PRIORITY_MEDIUM":
			$priority = 2;
		break;

		case "$TEXT_PRIORITY_LOW":
			$priority = 3;
		break;

		/*
		   case "$TEXT_PRIORITY_DONE":
		   $priority = 4;
		   break;
		 */

		case "1":
			$priority = "$TEXT_PRIORITY_HIGH";
		break;

		case "2":
			$priority = "$TEXT_PRIORITY_MEDIUM";
		break;

		case "3":
			$priority = "$TEXT_PRIORITY_LOW";
		break;

		/*
		   case "4":
		   $priority = "$TEXT_PRIORITY_DONE";
		   break;
		 */
	} //switch 
	return $priority;
}

/**
 * generates a mail to all responsible persons and shows up what happened to a task.
 * @param id of the task
 * @param the performed action
 */
function generateMail($id,$action,$user_id)
{
	global $HTTP_SERVER_VARS,$SEND_MAILS, $GLOBALS, $db;

	if (!$SEND_MAILS)
		return;

	include_once("include/class.phpmailer.php");

	$mail = new phpmailer();
	$mail->Mailer="sendmail";
	$mail->Sendmail="/usr/lib/sendmail";
	$mail->From=$GLOBALS['SEND_MAILS_SENDER'];
	$mail->FromName='Todolist.php / '.$HTTP_SERVER_VARS['HTTP_HOST'];
	$mail->WordWrap=60;

	// Get all responsible users first (needed to get the details of the users (email!) later as MySQL
	// doesn't support subselects... :-(
	$result = $db->query("SELECT user_id
	FROM todo_responsible_persons trp, todo_users tu
	WHERE todo_id=$id
	AND tu.usernr=trp.user_id
	AND tu.email_notify=1
	AND trp.user_id = $user_id");

	while ($db->next_record()){
		$responsible_users[] = $db->f("user_id");
	}

	// Task creator is responsible too
	$db->free();
	$result = $db->query("SELECT tt.created_by
	FROM todo_todos tt, todo_users tu
	WHERE tt.todo_id=$id
	AND tu.usernr=tt.created_by
	AND tu.email_notify=1 AND tu.usernr <> $user_id");

	while ($db->next_record()){
		$responsible_users[] = $db->f("created_by");
	}


	if (isset($responsible_users) && $responsible_users[0]!="") {
		$message_html = '<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
			<HTML><HEAD>
			<META http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
			</HEAD>
			<BODY>';
		// responsible users found: generate mail-text
		$db->free();

		if (!$db->query('SELECT todo_todos.*, todo_projects.project_name
					FROM todo_todos, todo_projects
					WHERE project_id=todo_projects.id AND todo_id='.$id)) {
			return false;
		}

		$db->next_record();

		$id						= $db->f("todo_id");
		$project				= $db->f("project_id")." - ".stripslashes($db->f("project_name"));
		$text					= stripslashes($db->f("todo_text"));
		$priority				= $db->f("todo_priority");
		$percentage_completed	= $db->f("percentage_completed");
		$due_date				= $db->f("due_date");
		$date_created			= $db->f("date_created");
		$date_changed			= $db->f("date_changed");
		$created_by				= $db->f("created_by");
		$changed_by				= $db->f("changed_by");

		$message_text= "\n".$GLOBALS['TEXT_PROJECT']	.": $project";
		$message_text.="\n".$GLOBALS['TEXT_PRIORITY']	.": " . switchPriority($priority);
		$message_text.="\n".$GLOBALS['TEXT_PERCENTAGE']	.": $percentage_completed";
		$message_text.="\n".$GLOBALS['TEXT_DUE']		.": $due_date";
		$message_text.="\n".$GLOBALS['TEXT_CREATED_ON']	.": $date_created";
		$message_text.="\n".$GLOBALS['TEXT_CHANGED_ON']	.": $date_changed";
		$message_text.="\n".$GLOBALS['TEXT_TEXT']		.": $text";

		$message_html .='<table>
			<tr><td>'.$GLOBALS['TEXT_PROJECT'].'</td><td>' . $project . '</td></tr>
			<tr><td>'.$GLOBALS['TEXT_PRIORITY'].'</td><td>' . switchPriority($priority) . '</td></tr>
			<tr><td>'.$GLOBALS['TEXT_PERCENTAGE'].'</td><td>' . $percentage_completed . '</td></tr>
			<tr><td>'.$GLOBALS['TEXT_DUE'].'</td><td>' . $due_date . '</td></tr>
			<tr><td>'.$GLOBALS['TEXT_CREATED_ON'].'</td><td>' . $date_created . '</td></tr>
			<tr><td>'.$GLOBALS['TEXT_CHANGED_ON'].'</td><td>' . $date_changed . '</td></tr>
			<tr><td>'.$GLOBALS['TEXT_TEXT'].'</td><td>' . $text . '</td></tr>
			</table>
			';

		if ($action != 'todo_delete') {
			// FIXME: What if https?!?
			$url = 'http://'.$HTTP_SERVER_VARS['HTTP_HOST'].':'.$HTTP_SERVER_VARS['SERVER_PORT'];
			$url .= $HTTP_SERVER_VARS['PHP_SELF'].'?route='.DETAILS.'&amp;id='.$id;
			$message_html .= str_replace("%URL%", $url, $GLOBALS['TEXT_EMAIL_CLICK_HERE_HTML']);
			$message_text .= str_replace("%URL%", $url, $GLOBALS['TEXT_EMAIL_CLICK_HERE_TEXT']);
		}
		// get the notes:
		$db->query("SELECT n.text,n.date,u.first_name,u.last_name
				FROM todo_notes n, todo_users u
				WHERE todo_id=$id AND n.usernr=u.usernr
				ORDER BY n.date");

		if ($db->num_rows() > 0 ) {
			$message_text .= "\n\n".$GLOBALS['TEXT_NOTE'].":\n";
			$message_html .='<br><b>'.$GLOBALS['TEXT_NOTE'].':</b><table border="1">';
			$message_html .='<tr><th>'.$GLOBALS['TEXT_TEXT'].'</th><th>'.$GLOBALS['TEXT_USER'].'</th>
			<th>'.$GLOBALS['TEXT_DATE'].'</th></tr>';

			while ($db->next_record()) {
				$text		= stripslashes($db->f("text"));
				$last_name	= stripslashes($db->f("last_name"));
				$first_name = stripslashes($db->f("first_name"));
				$date		= $db->f("date");

				$message_text .= "\n\"$text\"";
				$message_text .= "\n$last_name, $first_name $date";
				$message_html .= "<tr><td><pre>$text</pre></td><td>$last_name, $first_name</td><td>$date</td><tr>\n";
			}
			$message_html .="</table>";
		}

		// get the receipients
		$query ="SELECT first_name, email FROM todo_users WHERE usernr in ("; 
		while ($resp=array_pop($responsible_users)) {
			$query .= $resp;
			if (sizeof($responsible_users) > 0) {
				$query .= ',';
			} else {
				$query .= ') ';
			}
		}
		$query .= " AND email IS NOT NULL
			AND email != ''
			AND email_notify != 0";

		$db->query($query);

		$message_text .="\n";
		$message_html .="</BODY></HTML>\n";

		// it would be nice to kill html-formating here and save the creation of the text-version...
		// but the HTML-code must be indented nicely then...

		switch ($action) {
			case "todo_delete":
				$subject = $GLOBALS['TEXT_TODO_DELETED'];
			break;

			case "todo_change":
				$subject = $GLOBALS['TEXT_TODO_CHANGED'];
			break;

			case "todo_add":
				$subject = $GLOBALS['TEXT_TODO_ADDED'];
			break;

			default:
				$subject = $GLOBALS['TEXT_TODO_STATUS'];
			break;
		}
		$subject .= ' ' . $project . ': ' . $text . '; ' . $changed_by;
		// Kill Linefeeds in subjects.
		$subject = str_replace("\n","_",$subject);

		while ($db->next_record()){
			$mail->AddAddress($db->f("email"));
		}

		$mail->IsHTML(true);
		$mail->Subject=$subject;
		$mail->Body=$message_html;
		$mail->AltBody=$message_text;

		if(!$mail->Send())
		{
			echo "Message could not be sent. <p>";
			echo "Mailer Error: " . $mail->ErrorInfo;
//			exit;
		}

	} else {
		// no responsible users found so no mail hast to be generated.
		return;
	}
} // end of generateMail();


/**
 * creates a HTML-dropdownbox with the availible Users
 *
 * @param $myname			string	Name of the form-variable
 * @param $selected_names	Array	Array containing the usernr
 * @param $emty_choice		Boolean	Should an emty-entry be created? [1,0,true,false]
 * @param $multiple			Boolean	Allow multiple selects? [1,0,true,false]
 * @return HTML containing the dropdownbox
 */
function makeUserDropdown($myname,$selected_names,$selected_project, $emty_choice, $multiple) {
	global $db, $route, $HTTP_GET_VARS, $HTTP_SESSION_VARS;
	$str = "";

/*
	if (($route!=ADMIN) && isset($HTTP_GET_VARS['page']) && ($HTTP_GET_VARS['page'] != ADMINPAGE)) {
		$query="SELECT distinct(usernr), first_name, last_name, login_name
			FROM todo_users u, todo_project_members pm";

		if ($selected_project != "all") {
			$query .= ' WHERE u.usernr = pm.member_id
				AND pm.project_id='.$selected_project;
		} else {
			$query.=' WHERE pm.project_id in ' . $HTTP_SESSION_VARS['usr']->my_projects;

		}
		$query .= ' 
			ORDER BY last_name, first_name';

	} else {
		$query = "SELECT usernr, first_name, last_name, login_name
			FROM todo_users
			ORDER BY last_name, first_name";
	}
	*/

	$query="SELECT distinct(usernr), first_name, last_name, login_name
		FROM todo_users u, todo_project_members pm";

	if ($selected_project != "all" && $selected_project != '') {
		$query .= ' WHERE u.usernr = pm.member_id
			AND pm.project_id='.$selected_project;
	} else {
		$query.=' WHERE pm.project_id in ' . $HTTP_SESSION_VARS['usr']->my_projects;

	}
	$query .= ' 
		ORDER BY last_name, first_name';

	$db->query($query);

	if ($multiple) {
		if ($db->num_rows()> 100) {
			$size=15;
		} elseif ($db->num_rows() > 50) {
			$size=10;
		} elseif ($db->num_rows() > 25) {
			$size=7;
		} elseif ($db->num_rows() > 5) {
			$size=6;
		} elseif ($db->num_rows() <= 5) {
			$size=$db->num_rows();
		}

		$myname=$myname . "[]";
		$str .= '<select multiple="multiple" name="'.$myname.'" size="'.$size.'">
			';
	} else  {
		$str .= '<select name="'.$myname.'" size="1">
			';
	}

	if ($emty_choice) {
		if ("$selected_names[0]" == "")  {
			$str .= '<option selected="selected" VALUE="">';
		} else {
			$str .= '<option value="">';
		}
	} 
	$i = 0;
	if ($db->num_rows() > 0)
	{
		while ($db->next_record())
		{
			$last_name  = stripslashes($db->f("last_name"));
			$first_name = stripslashes($db->f("first_name"));
			$usernr = $db->f("usernr");

			if (inArray("$usernr", $selected_names)) {
				$str .= '<option selected="selected" value="'.$usernr.'">';
				if ( $db->f('last_name') == '' && $db->f('first_name') == '' ) {
					$str .= $db->f('login_name');
				} else {
					$str .= $last_name;
					if ("$first_name" != ""){
						$str .= ", $first_name";
					}
				}
			} else {
				$str .= '<option value="'.$usernr.'">';
				if ( $db->f('last_name') == '' && $db->f('first_name') == '' ) {
					$str .= $db->f('login_name');
				} else {
					$str .= $last_name;
					if ("$first_name" != ""){
						$str .= ", $first_name";
					}
				}
			}
			$str .= "</option>\n";
			$i++;
		}
	}
	$str .= '
		</select>
		';
	return $str;
}


/**
 * creates a HTML-dropdownbox with the availible Columns for main-table
 *
 * @param $myname			string	Name of the form-variable
 * @param $selected_columns	Array	Array containing the Column-Numbers to show.
 * @return HTML containing the dropdownbox
 */
function makeColumnDropdown($myname,$selected_columns) {
	global $GLOBALS;

	$columns = array (
			COLUMN_CHANGED_ON	=> $GLOBALS['TEXT_CHANGED_ON'],
			COLUMN_COUNTER		=> '#',
			COLUMN_DETAILS		=> $GLOBALS['TEXT_DETAILS'],
			COLUMN_DUE			=> $GLOBALS['TEXT_DUE'],
			COLUMN_NR_NOTES		=> $GLOBALS['TEXT_NR_NOTES'],
			COLUMN_PERCENTAGE	=> $GLOBALS['TEXT_PERCENTAGE'],
			COLUMN_PRIORITY		=> $GLOBALS['TEXT_PRIORITY'],
			COLUMN_RESPONSIBLE	=> $GLOBALS['TEXT_RESPONSIBLE'],
			COLUMN_STATUS		=> $GLOBALS['TEXT_STATUS'],
			COLUMN_TEXT			=> $GLOBALS['TEXT_TEXT'],
			COLUMN_PROJECT		=> $GLOBALS['TEXT_PROJECT'],
			COLUMN_ID			=> $GLOBALS['TEXT_ID']
			);

	asort($columns);
	reset($columns);

	$str = '<select multiple="multiple" name="'.$myname.'[]" size="4">';

	foreach($columns as $k => $v) {
		if (in_array($k, $selected_columns)){
			$str .= '<option selected="selected" value="'.$k.'">'.$v;
		} else {
			$str .= '<option value="'.$k.'">'.$v;
		}
		$str .= "</option>\n";
	}

	$str .='</select>';
	return $str;
}


/**
 * Creates an HTML checkbox for show done tasks or not
 */
function makeShowDone($show_done)
{
  $str = "";

  $str .= '<select name=show_done onchange=showdone()>';

  if ($show_done == 1) {
    $str .= '<option value=0>Hide Finished Tasks
<option value=1 selected>Show All Tasks
<option value=2>Show Dev Complete Tasks
<option value=3>Show Development Tasks
<option value=4>Show Finished Tasks
';
  } else if($show_done == 0) {
    $str .= '<option value=0 selected>Hide Finished Tasks
<option value=1>Show All Tasks
<option value=2>Show Dev Complete Tasks
<option value=3>Show Development Tasks
<option value=4>Show Finished Tasks
';
  } else if($show_done == 2) {
    $str .= '<option value=0 >Hide Finished Tasks
<option value=1>Show All Tasks
<option value=2 selected>Show Dev Complete Tasks
<option value=3>Show Development Tasks
<option value=4>Show Finished Tasks
';
  } else if($show_done == 3) {
    $str .= '<option value=0>Hide Finished Tasks
<option value=1>Show All Tasks
<option value=2>Show Dev Complete Tasks
<option value=3 selected>Show Development Tasks
<option value=4>Show Finished Tasks
';
  } else if($show_done == 4) {
    $str .= '<option value=0>Hide Finished Tasks
<option value=1>Show All Tasks
<option value=2>Show Dev Complete Tasks
<option value=3 selected>Show Development Tasks
<option value=4 selected>Show Finished Tasks
';
  }

  $str .= '</select>';

  return $str;
}

/**
 * creates a HTML-Dropdownbox with the availible projects
 *
 * @param $myname			String	Name of the form-variable
 * @param $selected_project	int		the project currently selected
 * @param $all				Boolean	should the all-entry be there?
 * @param $java				String	which JavaScript-function schould be called?
 * @param $admin			Boolean	Show the projects the user is admin off.
 * @return String containing the HTML
 */
function makeProjectDropdown($myname,$selected_project,$all=false, $java=false, $admin=false) {
	global $db, $TEXT_ALL, $HTTP_GET_VARS, $HTTP_POST_VARS, $HTTP_SESSION_VARS;

	// FIXME: This is quite a hack....
	if (isset ($HTTP_POST_VARS['route'])){
		$route = $HTTP_POST_VARS['route'];
	} else if (isset ($HTTP_GET_VARS['route'])){
		$route = $HTTP_GET_VARS['route'];
	} else {
		$route = '';
	}

	if (isset ($HTTP_POST_VARS['page'])){
		$page = $HTTP_POST_VARS['page'];
	} else if (isset ($HTTP_GET_VARS['page'])){
		$page = $HTTP_GET_VARS['page'];
	} else {
		$page = '';
	}

	$str = "";

	if ($admin){
		// This a user who is project leader... Show only those projects
		$db->query('SELECT * FROM todo_projects where project_leader='.$HTTP_SESSION_VARS['usr']->userid . " order by project_name asc");
	} else if ($HTTP_SESSION_VARS['usr']->isAdmin() &&
			($route == ADMIN) ||
			($route == ACTIONS && $page == NEWUSER ) ||
			($route == ACTIONS && $page == UPDATEUSER ) ||
			($route == ACTIONS && $page == DELUSER ) ||
			($route == ACTIONS && $page == ADMINPAGE ) ||
			($route == ACTIONS && $page == NEWPROJECT ) ||
			($route == ACTIONS && $page == UPDATEPROJECT ) ||
			($route == ACTIONS && $page == DELETEPROJECT )) {
		// Admin needs to see all projects.
		$db->query("SELECT * FROM todo_projects order by project_name asc");
	} else {
		// If we are not the admin do only get the projects we're member of.
		$db->query('SELECT * FROM todo_projects p, todo_project_members m
				WHERE p.id = m.project_id and m.member_id = '.$HTTP_SESSION_VARS['usr']->userid .'
				ORDER BY project_name ASC');
	}

	$str .= '<select name="'.$myname.'" size="1"';
	if ($java) {
		$str .= ' onchange="'.$java.'"';
	}
	$str .= '>';

	// all means all projects the user is member off
	if ($all) {
		if ($selected_project == "all" ) {
			$str .= '<option selected="selected" value="all">'.$TEXT_ALL;
		} else {
			$str .= '<option value="all">'.$TEXT_ALL;
		}
		$str.='</option>';
	}

	while ($db->next_record())
	{
		$project_id   = $db->f("id");
		$project_name = stripslashes($db->f("project_name"));

		if ($project_id == $selected_project) {
			$str .= '<option selected="selected" value="'.$project_id.'">'.stripslashes($project_name);
		} else {
			$str .= '<option value="'.$project_id.'">'.stripslashes($project_name);
		}
		$str .= "</option>\n";
	}

	$str .= '</select>';
	return $str;
}


/**
 * creates a HTML-Dropdownbox with the availible groups
 *
 * @param $myname			String	Name of the form-variable
 * @param $selected_project	int		the project currently selected
 * @param $emty_choice		Boolean	Should an emty-entry be created? [1,0,true,false]
 * @param $multiple			Boolean	Allow multiple selects? [1,0,true,false]
 * @return String HTML
 */
function makeGroupDropdown($myname,$selected_group, $emty_choice, $multiple) {
	global $db;

	$str = "";

	$db->query("SELECT * FROM todo_groups ORDER BY group_name");

	$str .= '<select name="'.$myname.'" size="1">';

	while ($db->next_record())
	{
		$group_id   = $db->f("id");
		$group_name = stripslashes($db->f("group_name"));

		if ($group_id == $selected_group) {
			$str .= '<option selected="selected" value="'.$group_id.'">'.$group_name;
		} else {
			$str .= '<option value="'.$group_id.'">'.$group_name;
		}
		$str .= "</option>\n";
	}

	$str .= '</select>';
	return $str;
}


/**
 * creates a HTML-Dropdownbox with the task-stati
 *
 * @param	$myname				string	Name of the form-variable
 * @param	$selected_status	int		the status currently selected
 * @param	$all				Boolean	should the all-entry be there?
 * @param	$java				string	which JavaScript-function schould be called?
 */
function makeStatusDropdown($myname, $selected_status, $all=false, $java=false) {
	global $TEXT_STATUS_DONE, $TEXT_STATUS_IN_PROGRESS, $TEXT_STATUS_OPEN, $TEXT_STATUS_OBSOLETE, $TEXT_ALL, $stati;

	$str = '<select name="'.$myname.'" size="1"';
	if ($java) {
		$str .= ' onchange="'.$java.'"';
	}
	$str .= '>';

	// for search-boxes
	if ($all) {
		if ($selected_status < 0) {
			$str .= '<option selected="selected" value="-1">'.$TEXT_ALL;
		} else {
			$str .= '<option value="-1">'.$TEXT_ALL;
		}
		$str.='</option>';
	}

	foreach($stati as $k => $v) {
		if ($k == $selected_status){
			$str .= '<option selected="selected" value="'.$k.'">'.$v;
		} else {
			$str .= '<option value="'.$k.'">'.$v;
		}
		$str .= "</option>\n";
	}

	$str .= '</select>';
	return $str;
}


/**
 * converts date to US-dateformat
 *
 * converts date in the format specified via config.php to US-Dateformat (so that MySQL understands it):
 * 01.12.1999  --->  1999-12-01
 * 01-12-1999  --->  1999-12-01
 * 01121999    --->  1999-12-01
 * 011299	    --->  99-12-01
 * mmddyy
 * etc.
 *
 * @param $datestr String the date in the local format
 * @return String date in US Format
 */
function convDateToUS($datestr)
{
	global $DATEFORMAT;
	$xTemp = explode(" ", trim($datestr));
	if (isset($xTemp[1])) {
		$xTime = "$xTemp[1]"; // Time ist not changed...
	} else {
		$xTime = ""; // Time ist not changed...
	}

	if ( ereg( "([0-9]{1,2})([.-\/]{0,1})([0-9]{1,2})([.-\/]{0,1})([0-9]{2,4})", $xTemp[0], $regs ) ) {
		if ($DATEFORMAT == "2") {
			$date="$regs[5]-$regs[3]-$regs[1]";
		} elseif ($DATEFORMAT == "3") {
			$date="$regs[5]-$regs[1]-$regs[3]";
		}
	}
	return $date;
}


/**
 * converts US-dateformat to EU format
 *
 * convert 1999-12-01 ---> 01.12.1999
 *
 * @param $datestring String date in US Format
 * @return String date in EU Format
 */
function convDateToEU($datestr)
{
	$xTemp = explode(' ', trim($datestr)); // dont convert the time...
	if ( eregi( "([0-9yja]{4})([.-]{0,1})([0-9m]{1,2})([.-]{0,1})([0-9td]{1,2})",  $xTemp[0], $regs )) {
		$date="$regs[5].$regs[3].$regs[1]";
	}
	if (!isset($xTemp[1])) {
		return $date;
	} else {
		return $date . " " . $xTemp[1];
	}
}                                                                               

/**
 * convert 1999-12-01 ---> 12/01/1999
 *
 * @param $datestring String date in US Format
 * @return String date in mm/dd/yyyy Format
 */
function convDateToMMDDYY($datestr)
{
	$xTemp = explode(' ', trim($datestr)); // dont convert the time...
	if ( eregi( "([0-9yja]{4})([.-]{0,1})([0-9m]{1,2})([.-]{0,1})([0-9td]{1,2})",  $xTemp[0], $regs )) {
		$date="$regs[3]/$regs[5]/$regs[1]";
	}
	if (!isset($xTemp[1])) {
		return $date;
	} else {
		return $date . " " . $xTemp[1];
	}
}                                                                               

/**
 * Date-conversion depending on $DATEFORMAT
 * 
 * @param $datestring String date in US Format
 * @return String date in format specified in $DATEFORMAT
 */
function convDate($datestr)
{
	global $DATEFORMAT, $DEBUG;
	/*
	   if ($DEBUG)
	   {
	   echo "Before: " . $datestr;
	   }
	 */
	if ($DATEFORMAT == "2" ) {
		$xdate=convDateToEU("$datestr");
	} elseif ($DATEFORMAT == "3") {
		$xdate=convDateToMMDDYY("$datestr");
	} else {
		$xdate=$datestr;
	}

	return $xdate;
}

/**
 * Return true if a value exists in an array
 * @param $needle	string	string to search for
 * @param $haystack	array	array to search in
 */
function inArray ($needle, $haystack)
{
	@reset($haystack);
	while (@list(, $value) = @each ($haystack)) {
		if ($value == $needle){
			return true;
		}
	}
	return false;
}


/**
 * How should the table be ordered?
 *
 * @param string order-indicator
 */
function orderBy($order_by) {
	//$str = ' ORDER BY todo_priority, root, parent, level,  ';
	$str = ' ORDER BY ';
	switch ($order_by) {
		case "id_asc":
			$str .= "todo_id ASC"; 
		break;

		case "id_desc":
			$str .= "todo_id DESC"; 
		break;

		case "status_asc":
			$str .= "status ASC"; 
		break;

		case "status_desc":
			$str .= "status DESC"; 
		break;

		case "prio_asc":
			$str .= "todo_priority ASC"; 
		break;
		case "prio_desc":
			$str .= "todo_priority DESC";
		break;
		case "due_asc":
			$str .= "due_date ASC"; 
		break;
		case "due_desc":
			$str .= "due_date DESC"; 
		break;
		/*
		   case "responsible_asc":
		   $str .= " responsible_person ASC"; 
		   break;
		   case "responsible_desc":
		   $str .= " responsible_person DESC"; 
		   break;
		 */
		case "project_asc":
			$str .= " project_name ASC"; 
		break;
		case "project_desc":
			$str .= " project_name DESC"; 
		break;
		case "changed_on_asc":
			$str .= " date_changed ASC"; 
		break;
		case "changed_on_desc":
			$str .= " date_changed DESC"; 
		break;
		case "":
			$str .= " todo_priority ASC, due_date ASC"; 
		break;
	} // end of switch
	return $str;
}

/* vim: set si tw=110 ts=4 sw=4: */
?>
