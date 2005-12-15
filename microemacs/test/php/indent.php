<? // pages.inc - the main bulk of the output from the todolist script
   /* vim: set si tw=110 ts=4 sw=4: */

/**
 * Print the todo-table
 *
 * @param $xquery		string	the SQL-Query that should fill the table
 * @param $xis_search	boolean	Is this a search?
 * @param $page			int		the current page-context
 *
 * @return HTML
 */
function printTodoTable($xquery, $xis_search, $page) {
	global $db, $HTTP_GET_VARS, $HTTP_SERVER_VARS, $GLOBALS, $HTTP_SESSION_VARS, $stati;

	$str = "";

	switch ($page)
	{
		case SEARCH:
			// here's a little nuisance I haven't fixed yet - will only pass one user name
			// if you change the sort order. Can probably work a solution to this using
			// urlencode(serialize()) but it's a headache.
			if (is_array($HTTP_GET_VARS['responsible_person'])){
				$responsible = $HTTP_GET_VARS['responsible_person'][0];
			} else {
				$responsible = $HTTP_GET_VARS['responsible_person'];
			}
			$xREFRESH_URL = $HTTP_SERVER_VARS['PHP_SELF'].'?route='.SEARCH.'abfrage='.$HTTP_GET_VARS['abfrage'];
			if (isset($HTTP_GET_VARS['prioritaet'])) {
				$xREFRESH_URL .="&prioritaet=".$HTTP_GET_VARS['prioritaet'];
			}
			$xREFRESH_URL .="&responsible_person=$responsible&wildcards=".$HTTP_GET_VARS['wildcards'].'&';
			break;

		default:
			$xREFRESH_URL = $HTTP_SERVER_VARS['PHP_SELF']."?";
			break;
	}

	// Respect printlayout when sorting table.
	if (isset($HTTP_GET_VARS['printlayout'])) {
		$xREFRESH_URL .= 'printlayout=' . $HTTP_GET_VARS['printlayout'] . '&';
	}

	$query = "SELECT distinct(rp.todo_id), first_name, last_name, login_name
		FROM todo_users u, todo_responsible_persons rp, todo_todos t, todo_project_members pm";

	if ($HTTP_SESSION_VARS['usr']->selected_project != "all") {
		$query .= ' WHERE t.project_id='.$HTTP_SESSION_VARS['usr']->selected_project;
	} else {
		$query .= ' WHERE t.project_id=pm.project_id
			AND pm.member_id =' . $HTTP_SESSION_VARS['usr']->userid;
	}
	$query .= ' 
		AND t.todo_id=rp.todo_id
		AND u.usernr = rp.user_id';

	$db->query($query);
	while ($db->next_record()) {
		// give login-name as "last_name" if first and last name are not set.
		if ( $db->f('last_name') == '' && $db->f('first_name') == '' ) {
			$responsible_users[] = array ($db->f("todo_id"), $db->f("login_name"), "");
		} else {
			$responsible_users[] = array ($db->f("todo_id"), $db->f("first_name"), $db->f("last_name"));
		}
	}

	// Get all the project-names in an array
	$query = 'SELECT id, project_name FROM todo_projects';

	$db->query($query);
	while ($db->next_record()) {
		$projects[$db->f('id')] = stripslashes($db->f('project_name'));
	}

	$db->query("$xquery");

	/*
	//if there are no open but resolved tasks they are not displayed if returning here.... ;(
	if ($db->num_rows() == 0 ){
		return ($GLOBALS['TEXT_NO_DATA_FOUND']);
	}
	*/


	$i = 0;
	$str .= '<table summary="Table of Tasks" border="0" cellspacing="1" cellpadding="0" rules="cols" width="100%">';

	$row[COLUMN_ID]			= $GLOBALS['TEXT_ID'];
	$row[COLUMN_PRIORITY]	= $GLOBALS['TEXT_PRIORITY'];
	$row[COLUMN_PROJECT]	= $GLOBALS['TEXT_PROJECT'];
	$row[COLUMN_STATUS]		= $GLOBALS['TEXT_STATUS'];
	$row[COLUMN_PERCENTAGE]	= '%';
	$row[COLUMN_TEXT]		= $GLOBALS['TEXT_TEXT'];
	$row[COLUMN_RESPONSIBLE]= $GLOBALS['TEXT_RESPONSIBLE'];
	$row[COLUMN_DUE]		= $GLOBALS['TEXT_DUE'];
	$row[COLUMN_CHANGED_ON]	= $GLOBALS['TEXT_CHANGED_ON'];
	$row[COLUMN_DETAILS]	= $GLOBALS['TEXT_DETAILS'];
	$row[COLUMN_NR_NOTES]	= $GLOBALS['TEXT_NR_NOTES'];

	$str .= printTodoRow($row,$xREFRESH_URL);

	while ($db->next_record()) {
		$status		= $db->f("status");
		$due_date	= $db->f("due_date");
		$id			= $db->f("todo_id");
		$priority	= $db->f("todo_priority");

		if ($due_date < $GLOBALS['very_important_date'] && $due_date != "0000-00-00" && $status != 10 &&
				$GLOBALS['VERY_IMPORTANT_DAYS']!= 0){
			$ROW_COLOR = $GLOBALS['VERY_IMPORTANT_COLOR'];
		} elseif ($priority == 1){
			$ROW_COLOR = $GLOBALS['HIGH_COLOR'];
		} elseif ($priority == 2){
			$ROW_COLOR = $GLOBALS['MED_COLOR'];
		} elseif ($priority == 3){
			$ROW_COLOR = $GLOBALS['LOW_COLOR'];
		} elseif ($status > 5){
			$ROW_COLOR = $GLOBALS['DONE_COLOR'];
		}

		$row['COLOR']			= $ROW_COLOR;
		if ($xis_search) {
			$row['NR_NOTES']	= 0;
		} else {
			$row['NR_NOTES']	= $db->f("nr_notes");
		}
		$row[COLUMN_ID]				= $id;
		$row['PARENT']			= $db->f('parent');
		$row['LEVEL']			= $db->f("level");
		$row['COUNTER']			= $i+1;
		$row[COLUMN_PRIORITY]	= switchPriority($priority);
		$row[COLUMN_PROJECT]	= $projects[$db->f('project_id')];
		$row[COLUMN_STATUS]		= $stati[$status];
		$row['STATUS_ID']		= $db->f("status");
		$row[COLUMN_PERCENTAGE]	= $db->f("percentage_completed");
		$row[COLUMN_TEXT]		= $db->f("todo_text");
		reset ($responsible_users);
		$respstr = "";
		while (@list($k,$v) = @each($responsible_users)){
			if ($v[0] == $id)
				$respstr .= $v[1] . " " . $v[2]. ", ";
		}
		$row[COLUMN_RESPONSIBLE]= substr($respstr,0,-2);
		$row[COLUMN_DUE]		= $due_date;
		$row[COLUMN_CHANGED_ON]	= $db->f('date_changed');
		$row[COLUMN_DETAILS]	= $GLOBALS['TEXT_DETAILS'];

		$str .= printTodoRow($row);

		$i++;
	}

	$nr_datasets = ++$i;

//?php
///////////////////// done tasks ///////////////////////////////////////////////

	if ($page == FRONTPAGE || $page == ACTIONS) { // list completed entries also
		$query="SELECT todo_todos.*, count(distinct(todo_notes.note_id)) AS nr_notes
			FROM todo_todos, todo_responsible_persons, todo_project_members
			LEFT JOIN todo_notes ON todo_todos.todo_id=todo_notes.todo_id";

		if ($HTTP_SESSION_VARS['usr']->selected_project != "all") {
			$query .= ' WHERE todo_todos.project_id='.$HTTP_SESSION_VARS['usr']->selected_project.
			' AND todo_todos.project_id=todo_project_members.project_id';
		} else {
			$query .= ' WHERE todo_todos.project_id=todo_project_members.project_id
				AND '. $HTTP_SESSION_VARS['usr']->userid. ' = todo_project_members.member_id';
		}

		// list all tasks with status >5 as this are end-status.
		$query .= "	AND status>5";

		// List done tasks or not
		if ($HTTP_SESSION_VARS['usr']->show_done != 1) {
		  $query .= ' AND status <> 10 AND status <> 8';
		}

		if ($HTTP_SESSION_VARS['usr']->showMyTasks()) {
			// show only tasks where I'm responsible for
			$query .= ' 
				AND todo_responsible_persons.user_id = '. $HTTP_SESSION_VARS['usr']->userid.'
				AND todo_todos.todo_id = todo_responsible_persons.todo_id';
              } else {
                $query .= '
                              AND todo_todos.todo_id = todo_responsible_persons.todo_id';
		}

		$query .= '  GROUP BY todo_id
			ORDER BY date_changed DESC
			LIMIT '.$GLOBALS['MAX_DONE']; 

		$db->query("$query");

		if ($db->num_rows() > 0 ){
			// Abstand vor den erledigten Einträgen. --> bessere Übersicht. 
			$str .= '<tr><td height="15"></td></tr>';
		}

		$i = 0;

		while ($db->next_record()) {
			$id = $db->f("todo_id");


			$row['COLOR']			= $GLOBALS['DONE_COLOR'];
			if ($xis_search) {
				$row['NR_NOTES']	= 0;
			} else {
				$row['NR_NOTES']	= $db->f("nr_notes");
			}
			$row[COLUMN_ID]			= $id;
			$row['PARENT']			= $db->f('parent');
			$row['LEVEL']			= $db->f("level");
			$row['COUNTER']			= $i+$nr_datasets;
			$row[COLUMN_PRIORITY]	= switchPriority($db->f("todo_priority"));
			$row[COLUMN_PROJECT]	= $projects[$db->f('project_id')];
			$row[COLUMN_STATUS]		= $stati[$db->f("status")];
			$row['STATUS_ID']		= $db->f("status");
			$row[COLUMN_PERCENTAGE]	= $db->f("percentage_completed");
			$row[COLUMN_TEXT]		= $db->f("todo_text");
			reset ($responsible_users);
			$respstr = "";
			while (@list($k,$v) = @each($responsible_users)){
				if ($v[0] == $id)
					$respstr .= $v[1] . " " . $v[2]. ", ";
			}
			$row[COLUMN_RESPONSIBLE]= substr($respstr,0,-2);
			$row[COLUMN_DUE]		= $db->f("due_date");
			$row[COLUMN_CHANGED_ON]	= $db->f('date_changed');
			$row[COLUMN_DETAILS]	= $GLOBALS['TEXT_DETAILS'];

			$str .= printTodoRow($row);
			$i++;
		}
	}

	$str .= '</table>';

	return $str;
} // end of printTodoTable()


/**
 * Prints one row of the todo-table
 *
 * @param $row			Array	The values of the columns
 * @param $refresh_url	boolean	The refresh-url for the table-headers
 *
 * @return HTML
 */
function printTodoRow($row, $refresh_url=false){
	global $HTTP_SESSION_VARS, $HTTP_SERVER_VARS, $HTTP_GET_VARS, $GLOBALS;

	$str = '';
	if (!isset($HTTP_GET_VARS['order_by'])){
		$HTTP_GET_VARS['order_by'] = '';
	}


	if ($refresh_url) { // only set for table-header.
		$str .= '<tr>';

		if ($HTTP_SESSION_VARS['usr']->showColumn(COLUMN_COUNTER)){
			$str .= "<th>#</th>";
		}

		if ($HTTP_SESSION_VARS['usr']->showColumn(COLUMN_ID)){
			if ($HTTP_GET_VARS['order_by']=='id_desc'){
				$str .= '<th>
					<a href="'.$refresh_url.'order_by=id_asc">'.$row[COLUMN_ID].'</a></th>';
			} else {
				$str .= '<th>
					<a href="'.$refresh_url.'order_by=id_desc">'.$row[COLUMN_ID].'</a></th>';
			}
		}

		if ($HTTP_SESSION_VARS['usr']->showColumn(COLUMN_PROJECT)){
			if ($HTTP_GET_VARS['order_by']=='project_desc'){
				$str .= '<th>
					<a href="'.$refresh_url.'order_by=project_asc">'.$row[COLUMN_PROJECT].'</a></th>';
			} else {
				$str .= '<th>
					<a href="'.$refresh_url.'order_by=project_desc">'.$row[COLUMN_PROJECT].'</a></th>';
			}
		}

		if ($HTTP_SESSION_VARS['usr']->showColumn(COLUMN_PRIORITY)){
			if ($HTTP_GET_VARS['order_by']=="prio_desc"){
				$str .= '<th width="60" align="left">
					<a href="'.$refresh_url.'order_by=prio_asc">'.$row[COLUMN_PRIORITY].'</a></th>';
			} else { 
				$str .= '<th width="60" align="left">
					<a href="'.$refresh_url.'order_by=prio_desc">'.$row[COLUMN_PRIORITY].'</a></th>';
			}
		}

		if ($HTTP_SESSION_VARS['usr']->showColumn(COLUMN_STATUS)) {
			if ($HTTP_GET_VARS['order_by']=="status_desc"){
				$str .= '<th align="left">
					<a href="'.$refresh_url.'order_by=status_asc">'.$row[COLUMN_STATUS].'</a></th>';
			} else { 
				$str .= '<th align="left">
					<a href="'.$refresh_url.'order_by=status_desc">'.$row[COLUMN_STATUS].'</a></th>';
			}
		}

		if ($HTTP_SESSION_VARS['usr']->showColumn(COLUMN_PERCENTAGE)){
			$str .= '<th>'.$row[COLUMN_PERCENTAGE].'</th>';
		}

		if ($HTTP_SESSION_VARS['usr']->showColumn(COLUMN_TEXT)){
			$str .= '<th align="left">'.$row[COLUMN_TEXT].'</th>';
		}

		if ($HTTP_SESSION_VARS['usr']->showColumn(COLUMN_RESPONSIBLE)){
			$str .= '<th align="left">'.$row[COLUMN_RESPONSIBLE].'</th>';
		}

		if ($HTTP_SESSION_VARS['usr']->showColumn(COLUMN_DUE)){
			if ($HTTP_GET_VARS['order_by']=="due_asc"){
				$str .= '<th width="60" align="left"><a href="'.$refresh_url.'order_by=due_desc">'.$row[COLUMN_DUE].'</a></th>';
			} else { 
				$str .= '<th width="60" align="left"><a href="'.$refresh_url.'order_by=due_asc">'.$row[COLUMN_DUE].'</a></th>';
			}
		}

		if ($HTTP_SESSION_VARS['usr']->showColumn(COLUMN_CHANGED_ON)){
			if ($HTTP_GET_VARS['order_by']=="changed_on_asc") {
				$str .= '<th width="60" align="left"><a href="'.$refresh_url.'order_by=changed_on_desc">'.$row[COLUMN_CHANGED_ON].'</a></th>';
			} else { 
				$str .= '<th width="60" align="left"><a href="'.$refresh_url.'order_by=changed_on_asc">'.$row[COLUMN_CHANGED_ON].'</a></th>';
			}
		}

		if ($HTTP_SESSION_VARS['usr']->showColumn(COLUMN_DETAILS)){
			$str .= '<th width="100" align="left">'.$row[COLUMN_DETAILS].'</th>';
		}
		$str .= "</tr>\n\n";
	} else {
		$str .= '<tr bgcolor="'.$row['COLOR'].'">';
		if ($HTTP_SESSION_VARS['usr']->showColumn(COLUMN_COUNTER)){
			$str .= '<td class="tdt" align="right">';
			if ($GLOBALS['SHOW_EXTRA_ASTERISK']== 1 && $row['NR_NOTES'] > 0 ){
				$str .= "<b>*</b> ";
			}
			$str .= $row['COUNTER'] . ".</td>";
		}

		if ($HTTP_SESSION_VARS['usr']->showColumn(COLUMN_ID)){
			$str .= '<td class="tdt">'.$row[COLUMN_ID].'</td>';
		}

		if ($HTTP_SESSION_VARS['usr']->showColumn(COLUMN_PROJECT)){
			$str .= '<td class="tdt">'.$row[COLUMN_PROJECT].'</td>';
		}

		if ($HTTP_SESSION_VARS['usr']->showColumn(COLUMN_PRIORITY)){
			if ($GLOBALS['SHOW_EXTRA_ASTERISK']== 2 && $row['NR_NOTES'] > 0 ){
				$str .= '<td class="tdt">'.$row[COLUMN_PRIORITY].'<b>*</b></td>';
			} else {
				$str .= '<td class="tdt">'.$row[COLUMN_PRIORITY].'</td>';
			}
		}

		if ($HTTP_SESSION_VARS['usr']->showColumn(COLUMN_STATUS)) {
			$str .= '<td class="tdt">'.$row[COLUMN_STATUS].'</td>';
		}

		if ($HTTP_SESSION_VARS['usr']->showColumn(COLUMN_PERCENTAGE)) {
			$str .= '<td class="tdt" align="center">';
			if ($GLOBALS['SHOW_EXTRA_ASTERISK']== 3 && $row['NR_NOTES'] > 0){
				$str .= "<b>*</b> ";
			}
			$str .= $row[COLUMN_PERCENTAGE].'</td>';
		}

/*
		if ($HTTP_SESSION_VARS['usr']->showColumn(COLUMN_TEXT)){
			if ($GLOBALS['SHOW_EXTRA_ASTERISK']== 4 && $row['NR_NOTES'] > 0){

				$str .= '<td class="tdt"><b>*</b> <a href="'.$HTTP_SERVER_VARS['PHP_SELF'].'?route='.DETAILS.'&amp;id='.$row[COLUMN_ID].'"><img
					src="include/img/arrow.gif"></a>'.stripslashes($row[COLUMN_TEXT]).'</td>';
			} else {
				$str .= '<td class="tdt"><a
					href="'.$HTTP_SERVER_VARS['PHP_SELF'].'?route='.DETAILS.'&amp;id='.$row[COLUMN_ID].'"><img src="include/img/new.png"></a>
					'.stripslashes($row[COLUMN_TEXT]).'</td>';
			}
		}
*/

		if ($HTTP_SESSION_VARS['usr']->showColumn(COLUMN_TEXT)){
			/*
			if ($row['PARENT'] != null) {
				for ($i=1; $i < $row['LEVEL'] ; $i++) {
					$dash .= '&nbsp;&nbsp;';
				}
				$dash .= '--&gt;&nbsp;';
				// This is a subtask.
				$row[COLUMN_TEXT] = $dash . $row[COLUMN_TEXT];
			}
			*/
			// FIXME: Show something if no text was entered. The form should be validated via Javascript,
			// so this is not possible.
			if ( ! strlen($row[COLUMN_TEXT]) ) {
				$row[COLUMN_TEXT] = '<b>' . $GLOBALS['TEXT_NOTEXT'] . '</b>';
			}
			if ($GLOBALS['SHOW_EXTRA_ASTERISK']== 4 && $row['NR_NOTES'] > 0){
				$str .= '<td class="tdt"><b>*</b>
				<a
				href="'.$HTTP_SERVER_VARS['PHP_SELF'].'?route='.DETAILS.'&amp;id='.$row[COLUMN_ID].'">'.
				nl2br(stripslashes($row[COLUMN_TEXT])).
				'</a></td>';
			} else {
				$str .= '<td class="tdt">
				<a href="'.$HTTP_SERVER_VARS['PHP_SELF'].'?route='.DETAILS.'&amp;id='.$row[COLUMN_ID].'">'.
				nl2br(htmlentities(stripslashes($row[COLUMN_TEXT]))).
				'</a></td>';
			}
		}

		if ($HTTP_SESSION_VARS['usr']->showColumn(COLUMN_RESPONSIBLE)){
			$str .= '<td class="tdt">'.$row[COLUMN_RESPONSIBLE].'</td>';
		}

		if ($HTTP_SESSION_VARS['usr']->showColumn(COLUMN_DUE)){
			// change the color if due_date already in the past and not done.
			if ($row[COLUMN_DUE] < $GLOBALS['most_important_date'] && $row[COLUMN_DUE] != "0000-00-00" &&
					$GLOBALS['MOST_IMPORTANT_DAYS']!= 0 && $row['STATUS_ID'] < 6) {
				$str .= '<td class="tdt" nowrap="nowrap"><font color="'.$GLOBALS['MOST_IMPORTANT_COLOR'].'">'.convDate($row[COLUMN_DUE]) . "</font></td>";
			} else {
				$str .= '<td class="tdt" nowrap="nowrap">' . convDate($row[COLUMN_DUE]) . "</td>";
			}
		}

		if ($HTTP_SESSION_VARS['usr']->showColumn(COLUMN_CHANGED_ON)){
			$str .= '<td class="tdt">' . convDate($row[COLUMN_CHANGED_ON]) . '</td>';
		}

		if ($HTTP_SESSION_VARS['usr']->showColumn(COLUMN_DETAILS)){
			$str .= '<td class="tdt"><a href="'.$HTTP_SERVER_VARS['PHP_SELF'].'?route='.DETAILS.'&amp;id='.$row[COLUMN_ID].'">'.$row[COLUMN_DETAILS].'</a></td>'; 
		}

		if ($HTTP_SESSION_VARS['usr']->showColumn(COLUMN_NR_NOTES)){
			// Anzahl der Notes anzeigen. Wenn mehr als 5 vorhanden sind, dann soll
			// die Zahl angezeigt werden, sonnst die entsprechende Anzahl Sternchen.
			if ($row['NR_NOTES'] > 0) {
				$str .= '<td class="tdt"><b><a href="'.$HTTP_SERVER_VARS['PHP_SELF'].'?route='.DETAILS.'&amp;id='.$row[COLUMN_ID].'">'; 
				if ($row['NR_NOTES'] < 5) {
					for ($zaehler=0;($zaehler < $row['NR_NOTES']) && ($zaehler < 5) && ($row['NR_NOTES'] < 5) ; $zaehler++) {
						$str .= "*";
					}
				} else {
					$str .= $row['NR_NOTES'];
				}
				$str .= "</a></b></td>";
			} else {
				$str .= '<td class="tdt"></td>'; 
			}
		}
		$str .= '</tr>
			';
	}

	return $str;
}



/**
 * form to add a new entry
 *
 * @param $project		int	The project of the parent-task (set if a subtask should be created.)
 * @param $parent_task	int	The parent-task (set if this a subtask should be created.)
 * @param $level		int	The level of this task (starting at 0)
 * @return HTML
 */
function add_box($project=null,$parent_task=null, $level=0){
global $TEXT_TEXT, $TEXT_DUE, $TEXT_PRIORITY, $TEXT_SUBMIT, $TEXT_RESPONSIBLE, $TEXT_DATEFORMAT,
$TEXT_PRIORITY_HIGH, $TEXT_PRIORITY_MEDIUM, $TEXT_PRIORITY_LOW, $TEXT_SHOW_CAL,
$TEXT_PROJECT, $HTTP_SESSION_VARS;

global $HTTP_SERVER_VARS, $HTTP_GET_VARS;

if (($project !=null && $parent_task == null ) || ($project == null && $parent_task != null )) {
	// something's wrong!
	return ;
}

if (!isset($HTTP_GET_VARS['add_project']) || $parent_task != null) {
	if ($HTTP_SESSION_VARS['usr']->selected_project == "all") {
		$add_project = $HTTP_SESSION_VARS['usr']->getFirstProject();
	} else {
		$add_project = $HTTP_SESSION_VARS['usr']->selected_project;
	}
} else {
	$add_project = $HTTP_GET_VARS['add_project'];
}

if (isset($HTTP_GET_VARS['add_text'])) {
	$text=str_replace(";RAUTE;", "#", $HTTP_GET_VARS['add_text']);;
} else {
	$text='';
}

// FIXME: This is not used at the moment because of the switchPrio-mess... :(
if (isset($HTTP_GET_VARS['add_prio'])) {
	$prio=$HTTP_GET_VARS['add_prio'];
} else {
	$prio='';
}

$str = "";
$str .= '
<a name="todoAddForm"/><hr noshade="noshade"/>
<form method="post" name="addform" action="'.$HTTP_SERVER_VARS['PHP_SELF'].'">
<input type="hidden" name="route" value="'.ACTIONS.'"/>';
if (isset($HTTP_GET_VARS['route']) && $HTTP_GET_VARS['route']==ADDFORM) {
	$str .= '<input type="hidden" name="page" value="'.ADDTASK_SINGLE_PAGE.'"/>';
} else {
	$str .= '<input type="hidden" name="page" value="'.ADDTASK_PAGE.'"/>';
}
if ($parent_task != null) {
	$str .= '<input type="hidden" name="action" value="subtodo-add"/>';
	$str .= '<input type="hidden" name="project" value="'.$project.'"/>';
	$str .= '<input type="hidden" name="parent_task" value="'.$parent_task.'"/>';
	$str .= '<input type="hidden" name="level" value="'.$level.'"/>';
} else {
	$str .= '<input type="hidden" name="action" value="todo-add"/>';
}
$str .='<table summary="add-form" width="100%">
';
$str .= "<tr><th align=\"left\">$TEXT_TEXT</th>";
$str .= "<th align=\"left\">$TEXT_PRIORITY</th>";
if ($parent_task == null) {
	$str .= "<th align=\"left\">$TEXT_PROJECT</th>";
}
$str .= "<th align=\"left\">$TEXT_DUE</th>";
$str .= "<th align=\"left\">$TEXT_RESPONSIBLE</th></tr>";
$str .='
<tr align="left" valign="top"><td>
<textarea cols="40" rows="5" name="text">'.$text.'</textarea>
</td>';
$str .='<td>
<select name="priority" size="1">
';

// Prio got from form-element is one less than the DB-value!
if (isset($HTTP_GET_VARS['add_prio'])) {
	$prio=$HTTP_GET_VARS['add_prio'];
} else {
	$prio='';
}

$str .= '<option';
if ($prio==0) {
	$str .= ' selected="selected"';
}
$str .= ">$TEXT_PRIORITY_HIGH</option>";
$str .= '<option';
if ($prio==1) {
	$str .= ' selected="selected"';
}
$str .=">$TEXT_PRIORITY_MEDIUM</option>";
$str .= '<option';
if ($prio==2) {
	$str .= ' selected="selected"';
}
$str .=">$TEXT_PRIORITY_LOW</option>";
$str .='
</select>
</td>';
if ($parent_task == null) {
	$str .='<td>'. makeProjectDropdown("project",$add_project, false, "updateaddbox()"). '</td>';
}
$str .= '
<td>
<input type="text" name="due_date" size="10" value="'.convDate(date("Y-m-d")).'" maxlength="10"/>
<pre>'.convDate($TEXT_DATEFORMAT).'</pre>';
$str .= '<a href="javascript:showCalendar(\'addform\',\'due_date\')">'.$TEXT_SHOW_CAL.'</a>';
$str .='
</td>
<td>
';
  $str .=  makeUserDropdown("responsible_person", array($HTTP_SESSION_VARS['usr']->userid), $add_project ,0,true); //empty array to keep it clear
$str .='
<p /><div align="right">
<input type="submit" value="'.$TEXT_SUBMIT.'" /></div>
</td></tr>
</table>
</form>
';
return $str;
}
// end add_box


/**
 * a box for searching, believe it or not 
 *
 * @param $priority
 * @param $search_status
 * @param $search_project
 * @param $responsible_person
 * @param $date_min
 * @param $date_max
 *
 * @return HTML
 */
function search_box($priority='',$search_status='', $search_project='',$responsible_person='',$date_min='',$date_max='') {
	global $TEXT_PRIORITY,$TEXT_RESPONSIBLE,$TEXT_THE_SEARCH,$TEXT_SEARCH,$TEXT_PRIORITY_HIGH,$TEXT_PRIORITY_MEDIUM,
	$TEXT_PRIORITY_LOW, $TEXT_DATE_FROM, $TEXT_DATE_TO, $TEXT_PROJECT, $TEXT_DATEFORMAT;
	global $TEXT_ALL, $route, $DATEFORMAT, $TEXT_STATUS, $TEXT_SHOW_CAL;
	global $HTTP_GET_VARS, $HTTP_SERVER_VARS, $GLOBALS;

	if (!isset($HTTP_GET_VARS['abfrage'])) {
		$HTTP_GET_VARS['abfrage'] = '';
	}

	if ($search_project == '') {
		$search_project = $HTTP_GET_VARS['search_project'];
	}

	$str = '
		<a name="todoSearchForm"/>';

	if ($route!=SEARCH) {
		$str .= '<hr noshade="noshade"/>';
	}
	$str .= '<form method="get" action="'.$HTTP_SERVER_VARS['PHP_SELF'].'" name="searchform">
		<input type="hidden" name="route" value="'.SEARCH.'" />';
	$str.='<input type="hidden" name="page" value="'.SEARCHPAGE.'" />';
	$str .='<table summary="searchform" width="100%">
		<tr>
		';
	$str .= "<th align=\"left\">$TEXT_THE_SEARCH:</th>";
	$str .= "<th align=\"left\">$TEXT_PRIORITY:</th>";
	$str .= "<th align=\"left\">$TEXT_STATUS:</th>";
	$str .= "<th align=\"left\">$TEXT_PROJECT:</th>";
	$str .= "<th align=\"left\">$TEXT_RESPONSIBLE:</th>";
	$str .= "<th align=\"left\">$TEXT_DATE_FROM:</th>";
	$str .= "<th align=\"left\">$TEXT_DATE_TO:</th>";
	$str .='
		</tr><tr valign="top"><td>
		';
	$str .= "<input type=\"text\" size=\"20\" maxlength=\"45\" name=\"abfrage\" value=\"".$HTTP_GET_VARS['abfrage'].'"/>';
	$str .= '<br />
		<input type="checkbox" name="wildcards"';
	if ( (isset($HTTP_GET_VARS['wildcards']) && $HTTP_GET_VARS['wildcards']==1) || ($route==FRONTPAGE) || ($route==ACTIONS)) {
		$str .= ' checked="checked"';
	}
	$str .= 'value="1"/> '.$GLOBALS['TEXT_WILDCARDS'].'<br />';
	$str .='
		</td>
		<td>
		<select name="priority" size="1">
		';
	if ($priority == ""){
		$str .= '<option selected="selected" value="">'.$TEXT_ALL.'</option>';
	}else{
		$str .= '<option value="">'.$TEXT_ALL.'</option>';
	}
	if ($priority == "1"){
		$str .= '<option value="1" selected="selected">'.$TEXT_PRIORITY_HIGH.'</option>';
	}else{
		$str .= '<option value="1">'.$TEXT_PRIORITY_HIGH.'</option>';
	}
	if ($priority == "2"){
		$str .= '<option value="2" selected="selected">'.$TEXT_PRIORITY_MEDIUM.'</option>';
	}else{
		$str .= '<option value="2">'.$TEXT_PRIORITY_MEDIUM.'</option>';
	}
	if ($priority == "3"){
		$str .= '<option value="3" selected="selected">'.$TEXT_PRIORITY_LOW.'</option>';
	}else{
		$str .= '<option value="3">'.$TEXT_PRIORITY_LOW.'</option>';
	}
	/*
	if ($priority == "4"){
		$str .= '<option value="4" selected="selected">'.$TEXT_PRIORITY_DONE.'</option>';
	}else{
		$str .= '<option value="4">'.$TEXT_PRIORITY_DONE.'</option>';
	}
	*/
	$str .= '</select></td>
		';
	$str .= '<td>'.makeStatusDropdown("search_status",$search_status, true).'</td>';
	//$str .= '<td>'.makeProjectDropdown("search_project",$search_project, true, "updatesearchbox()");
	$str .= '<td>'.makeProjectDropdown("search_project",$search_project, false, "updatesearchbox()");
	$str .= '</td><td>';
	$str .= makeUserDropdown("responsible_person", $responsible_person, $search_project , 0, true);

	if ($date_min=="") {
		$date_min="0000-00-00";
	} else {
		if ($DATEFORMAT != "1" ) {
			$date_min=convDateToUS($date_min);
		}
	}

	if ($date_max=="") {
		$date_max=date("Y-m-d");
	} else {
		if ($DATEFORMAT != "1" ) {
			$date_max=convDateToUS($date_max);
		}
	}

	$str .='
		</td><td>
		<input type="text" name="date_min" size="10" value="' . convDate($date_min) . '" maxlength="10"/><pre>'.convDate($TEXT_DATEFORMAT).'</pre>';
	$str .= '<a href="javascript:showCalendar(\'searchform\',\'date_min\')">'.$TEXT_SHOW_CAL.'</a>';
	$str .= '
		</td><td>
		<input type="text" name="date_max" size="10" value="' . convDate($date_max) . '" maxlength="10"/><pre>'.convDate($TEXT_DATEFORMAT).'</pre>';
	$str .= '<a href="javascript:showCalendar(\'searchform\',\'date_max\')">'.$TEXT_SHOW_CAL.'</a>';
	$str .= '
		<p/>
		<div align="right">
		<input type="submit" value="'.$TEXT_SEARCH.'"/>
		</div>
		</td>
		</tr>
		</table>
		</form>
		';
	return $str;
} 

// END FRONT_PAGE


/**
 * details page
 *
 * generates the detail-page for a task
 *
 * @param $id	int	The ID of the task that should be shown
 *
 * @return HTML
 */
function details_page($id){
	global $db, $DEBUG, $TEXT_ADMIN, $TEXT_ADMIN_MODULES, $TEXT_BACK, $TEXT_CHANGE,
	$TEXT_CHANGED_BY, $TEXT_CHANGED_ON, $TEXT_CHOOSE_EXISTING_USER, $TEXT_CHOOSE_MODULE, $TEXT_CREATED_BY,
	$TEXT_CREATED_ON, $TEXT_CREATE_NEW_USER, $TEXT_DATASET_CHANGED, $TEXT_DATASET_DELETED, $TEXT_DATE,
	$TEXT_DATEFORMAT, $TEXT_DELETE, $TEXT_DETAILS, $TEXT_DUE, $TEXT_GROUPS, $TEXT_ID, $TEXT_LOGIN_NAME,
	$TEXT_MONTH_NAMES, $TEXT_NAME_FIRST, $TEXT_NAME_LAST, $TEXT_NEW, $TEXT_NOTE, $TEXT_PASSWORD,
	$TEXT_PERCENTAGE, $TEXT_PRINTLAYOUT, $TEXT_PRIORITY, $TEXT_PRIORITY_HIGH, $TEXT_PRIORITY_LOW,
	$TEXT_PRIORITY_MEDIUM, $TEXT_PROJECT, $TEXT_RELOAD, $TEXT_RESPONSIBLE, $TEXT_SEARCH,
	$TEXT_SHORT_DAY_NAMES, $TEXT_SHOW_CAL, $TEXT_STATUS, $TEXT_SUBMIT, $TEXT_SUBMIT, $TEXT_TEXT,
	$TEXT_THE_SEARCH, $TEXT_TODO, $TEXT_TODO_ADDED, $TEXT_TOP, $TEXT_USER, $TEXT_USERS,
	$TEXT_SUBTASKS;
	global $HTTP_GET_VARS, $HTTP_SERVER_VARS, $HTTP_SESSION_VARS, $GLOBALS;

	$str = "";

	// Check if the user is really a member of that project
	$db->query("select tt.todo_id
			from todo_todos tt, todo_project_members tpm
			where tt.todo_id=$id
			and tt.project_id=tpm.project_Id
			and tpm.member_id=".$HTTP_SESSION_VARS['usr']->getUserid());

	if (!$db->num_rows() > 0 ) {
		return '<p class="feedback">'.$GLOBALS['TEXT_ERROR_NOT_MEMBER'].'</p>';
	}

	$db->free();

	if (isset ($HTTP_GET_VARS['detail_project'])){
		$db->query("SELECT user_id FROM todo_responsible_persons WHERE todo_id=$id");

		if ($db->num_rows() > 0 ) {
			while ($db->next_record()) {
				$responsible_users[] = $db->f("user_id");
			}

			$db->free();

			$db->query('SELECT member_id FROM todo_project_members WHERE project_id='.$HTTP_GET_VARS['detail_project']);

			while ($db->next_record()){
				$project_members[] = $db->f("member_id");
			}

			$db->free();

			$db->query("DELETE FROM todo_responsible_persons WHERE todo_id=$id");

			$query = 'INSERT INTO todo_responsible_persons VALUES ';
			$query2 = '';

			while (list ($key, $val) = each ($responsible_users)) {
				if (in_array($val, $project_members)) {
					$query2 .= "($id, $val)," ;
				}
			}

			// if there are new responsible persons that are also in the new project we don't need to update.
			// TODO: But of course there are _no_ responsible users for that task now!
			if (strlen($query2) > 0) {
				$query .= $query2;
				$query = substr($query, 0 , -1);
				$db->query($query);
				$db->free();
			}
		}
		$db->query('UPDATE todo_todos SET project_id='.$HTTP_GET_VARS['detail_project']." WHERE todo_id=$id");
		$db->free();
	}

	if (!$db->query("SELECT t.*,p.project_name FROM todo_todos t, todo_projects p
				WHERE todo_id=$id
				AND t.project_id=p.id"))
		return false;

	$db->next_record();

	$id						= $db->f("todo_id");
	$parent					= $db->f("parent");
	$level					= $db->f("level");
	$project				= $db->f("project_id");
	$project_name			= stripslashes($db->f("project_name"));
	$text					= $db->f("todo_text");
	$priority				= $db->f("todo_priority");
	$percentage_completed	= $db->f("percentage_completed");
	$due_date				= $db->f("due_date");
	$datum_erstellt			= $db->f("date_created");
	$date_changed			= $db->f("date_changed");
	$created_by				= $db->f("created_by");
	$changed_by				= $db->f("changed_by");
	$status					= $db->f("status");

	$db->free();

	$result = $db->query("SELECT user_id FROM todo_responsible_persons WHERE todo_id=$id");

	while ($db->next_record()){
		$responsible_users[] = $db->f("user_id");
	}

	$db->free();

	if (!$db->query("SELECT * FROM todo_users WHERE usernr IN ($created_by, $changed_by)"))
		return false;

	while ($db->next_record()){
		$usernames[$db->f("usernr")] = $db->f("last_name") . ", " . $db->f("first_name");
	}

	$str .=  '<form method="post" name="detailform" action="'.$HTTP_SERVER_VARS['PHP_SELF'].'">
		<input type="hidden" name="route" value="'.ACTIONS.'" />
		<table summary="task-details" border="0">';

	if ($DEBUG) {
		$str .='
			<tr>
			<td class="fmu">'.$TEXT_ID.'</td>
			<td>'.$id.'</td>
			</tr>';
	}
	$str .= '
		<tr>
		<td class="fmu"><input type="hidden" name="id" value="'.$id. '" readonly="readonly" />'.$TEXT_PROJECT.'</td>
		<td>' . makeProjectDropdown("project",$project,false,"updatedetails()") . '</td>
		</tr>';
	$str .= '
		<tr>
		<td class="fmu">'.$TEXT_PRIORITY.'</td>';

	$priority = switchPriority($priority);

	$str .= "<td><select name=\"priority\" size=\"1\">";
	if ($priority == "$TEXT_PRIORITY_HIGH") {
		$str .= "<option selected=\"selected\">$TEXT_PRIORITY_HIGH</option>";
	} else {
		$str .= "<option>$TEXT_PRIORITY_HIGH</option>";
	}

	if ($priority == "$TEXT_PRIORITY_MEDIUM"){
		$str .= "<option selected>$TEXT_PRIORITY_MEDIUM</option>";
	} else {
		$str .= "<option>$TEXT_PRIORITY_MEDIUM</option>";
	}

	if ($priority == "$TEXT_PRIORITY_LOW"){
		$str .= "<option selected=\"selected\">$TEXT_PRIORITY_LOW</option>";
	} else {
		$str .= "<option>$TEXT_PRIORITY_LOW</option>";
	}
	$str .= '
		</select></td>
		</tr>';
	$str .= '
		<tr><td class="fmu">'.$TEXT_STATUS.'</td><td>'.makeStatusDropdown("status",$status,false,'updatepercentage()').'</td></tr>';
	$str .='
		<tr>
		<td class="fmu">'.$TEXT_PERCENTAGE.'</td><td>
		<select name="percentage_completed" size="1" onchange="updatestatus()">
		';
	for ($i = 0 ; $i <= 100 ; $i += $GLOBALS['PERCENTAGE_STEPSIZE'])
	{
		if ($percentage_completed == $i)
			$str .= '<option value="'.$i.'" selected="selected">'.$i.'</option>';
		else
			$str .= '<option value="'.$i.'">'.$i.'</option>';
	}
	$str .= '
		</select>
		</td>
		</tr>
		<tr>';
	$str .= '<td class="fmu">'.$TEXT_TEXT.'</td>';
	$str .= '<td><textarea cols="50" rows="5" name="text">'.stripslashes($text).'</textarea></td>';
	$str .= "</tr>";
	$str .= "<tr>";
	$str .= '<td class="fmu">'.$TEXT_RESPONSIBLE.'</td><td>';
	$str .= makeUserDropdown("responsible_persons", $responsible_users, $project ,0,true);
	$str .= "</td></tr>";
	$str .= "<tr>";
	$str .= '<td class="fmu">'.$TEXT_DUE.'</td>';
	$str .= '<td><input type="text" name="due_date" value="' . convDate($due_date) . '" /><br/><pre>'.convDate($TEXT_DATEFORMAT).'</pre>';
	$str .= '<a href="javascript:showCalendar(\'detailform\',\'due_date\')">'.$TEXT_SHOW_CAL.'</a>';
	$str .= '</td></tr>';
	$str .= "<tr>";
	$str .= '<td class="fmu">'.$TEXT_CREATED_ON.'</td>';
	$str .= '<td><input type="hidden" name="datum_erstellt" value="'.$datum_erstellt.'" />';
	$str .= convDate($datum_erstellt) . "</td>";
	$str .= "</tr>";
	$str .= "<tr>";
	$str .= '<td class="fmu">'.$TEXT_CREATED_BY.'</td>';
	$str .= "<td>".$usernames["$created_by"]."</td>";
	$str .= "</tr>";
	$str .= "<tr>";
	$str .= '<td class="fmu">'.$TEXT_CHANGED_BY.'</td>';
	$str .= "<td>".$usernames["$changed_by"]."</td>";
	$str .= "</tr>";
	$str .= "<tr>";
	$str .= '<td class="fmu">'.$TEXT_CHANGED_ON.'</td>';
	$str .= "<td>";
	$str .= convDate($date_changed).'</td></tr>';

	//////////////////////////////////////////////////

	$str .= "</table>";
	$str .= '<select name="action" size="1">';
	$str .= '<option value="todo_change">'.$TEXT_CHANGE."</option>";
	$str .= '<option value="todo_delete">'.$TEXT_DELETE."</option>";
	$str .= "</select>";
	$str .= "&nbsp;&nbsp;<input type=\"submit\" value=\"$TEXT_SUBMIT\" />";


	$str .= "<br /><br />";

	$db->query("SELECT n.note_id,n.text,n.date,u.first_name,u.last_name
			FROM todo_notes n, todo_users u
			WHERE todo_id=$id AND n.usernr=u.usernr
			ORDER BY n.date");
	$anzahl = $db->num_rows();

	$i = 0;

	if ($anzahl > 0){
		$str .= '<table summary="notes for task" border="1"><tr>';
		$str .= "<th align=\"left\">$TEXT_NOTE</th>";
		$str .= "<th align=\"left\">$TEXT_USER</th>";
		$str .= "<th align=\"left\">$TEXT_DATE</th>";
		$str .= "</tr>";
	}

	$todo_id=$id;

	while ($db->next_record()) {
		$note_id	= $db->f("note_id");
		$note_text= stripslashes($db->f("text"));
		$datum	= $db->f("date");
		$lastname	= $db->f("last_name");
		$firstname= $db->f("first_name");

		$str .= '<tr><td class="note">'.nl2br($note_text)."</td><td align=\"center\">$lastname, $firstname</td><td>$datum</td></tr>";
	}

	if ($anzahl > 0){
		$str .= "</table>";
	}
	$str .= '<hr noshade="noshade"/>';

	$str .= "<h3>$TEXT_NOTE</h3>";
	$str .= '<table summary="note-add-form">';
	$str .= '<tr><td><textarea cols="50" rows="4" name="note_text"></textarea></td>';
	$str .= "<td>";
	$str .= "&nbsp;&nbsp;<input type=\"submit\" value=\"$TEXT_SUBMIT\" /></td>";
	$str .= "</tr></table>";
	$str .= "</form>";

	if (!($level >= $GLOBALS['MAX_SUBTASKS_LEVEL'])) {
		$str .= '<hr noshade="noshade"/>';
		$str .= "<h3>$TEXT_SUBTASKS</h3>";
		if (isset($HTTP_GET_VARS['order_by'])) {
			$order_by = $HTTP_GET_VARS['order_by'];
		} else {
			$order_by = '';
		}
		$query = makeFrontQuery($order_by, $project, $id);
		$str .= printTodoTable($query,$order_by,DETPAGE);
		$str .= add_box($project, $id, $level+1);
	}
	return $str;
} 
//END DETAILS_PAGE
?>
