<?php

require_once 'prepend.inc';
require_once 'builds.inc';

$locltime =  strftime("%m/%d/%Y %H:%M:%S");

if (!isset($autorefresh)) {
    $autorefresh = false;
}
$r_builds = builds_list(true);
if (!isset($build)) {
    $autorefresh = true;
    $build = $LATEST_BUILD;
    if (!isset($build)) {
        commonHeader("build status");
        display_errors(array("\nERROR - Cannot find a build to display\n"));
        commonFooter();
        exit;
    }
}

$r_logdirs = logdirs_list($build);
if (is_null($r_logdirs)) {
    $r_logdirs = array('invalid build!?!');
}

if (!isset($logdir)) $logdir = $r_logdirs[0];
if (!isset($view)) $view = 'summary';
if (!isset($type)) $type = 'opt';
if (!isset($logfile)) $logfile = '';
if ($view == "preview") {
    $r_views = array('summary', 'admin_logs', 'downloads', 'docs', 'preview', 'showcase');
} else {
    $r_views = array('summary', 'admin_logs', 'downloads', 'docs', 'showcase');
}
$r_hidden_fields = array(   'build'     => $build,
                            'logdir'    => $logdir,
                            'view'      => $view,
                            'type'      => $type,
                            'logfile'   => $logfile);
$r_types = array('opt'    => 'optimized',
                 'deb'    => 'debug');

if ($build != "admin") {
    $logPath       = unix_home() . "/buildlogs/${build}/${logdir}/" . builds_domain();
    $logTimeStamp  = @getFormattedDateTime("${logPath}/..");
    $julianDate    = getJulianDateForBuild($logPath);
} else {
    $logPath       = unix_home() . "/buildlogs/${build}/${logdir}";
    $logTimeStamp  = @getFormattedDateTime("${logPath}");
    $julianDate    = getJulianDateForBuild($logPath);
}
$logTimeStamp = "<p align=\"center\"><small>(".builds_domain()." - ${logTimeStamp} - ${julianDate})&nbsp;";
if (!isset($no_form)) {
    $logTimeStamp .= "<a href=\"${PHP_SELF}?logdir=${logdir}&build=${build}&view=${view}&type=${type}&logfile=${logfile}&autorefresh=${autorefresh}&no_form=1#EOP\">[no_form]</a>";
}
$logTimeStamp .= "<a href=\"${PHP_SELF}?logdir=${logdir}&build=${build}&view=${view}&type=${type}&logfile=${logfile}&no_form=1&autorefresh=".($autorefresh?0:1)."#EOP\">[".($autorefresh?'no_':'')."autorefresh]</a></small></p>";

$CONTEXT_DROPDOWN  = make_form_start($PHP_SELF, array('build' => $build,'view' => $view,'type' => $type,'logfile' => $logfile), "post");
$CONTEXT_DROPDOWN .= "    <td align=\"right\" valign=\"top\" nowrap colspan=2><font color=\"#ffffff\">\n";
$CONTEXT_DROPDOWN .= make_form_select("<small>build </small>", 'build" class="tbox', $r_builds, $build, true);
$CONTEXT_DROPDOWN .= make_submit('small_submit_white.gif', 'submit', 'bottom');
$CONTEXT_DROPDOWN .= "&nbsp;&nbsp;&nbsp;    </font></td>";
$CONTEXT_DROPDOWN .= make_form_end();
$CONTEXT_DROPDOWN .= make_form_start($PHP_SELF, $r_hidden_fields, "post");
$CONTEXT_DROPDOWN .= "    <td align=\"center\" valign=\"top\" nowrap><font color=\"#ffffff\">\n";
$CONTEXT_DROPDOWN .= make_form_select("<small> day </small>", 'logdir" class="tbox', $r_logdirs, $logdir, true);
$CONTEXT_DROPDOWN .= make_submit('small_submit_white.gif', 'submit', 'bottom');
$CONTEXT_DROPDOWN .= "&nbsp;    </font></td>";
$CONTEXT_DROPDOWN .= make_form_end();
if ($build != "admin") {
    $CONTEXT_DROPDOWN .= make_form_start($PHP_SELF, $r_hidden_fields, "post");
    $CONTEXT_DROPDOWN .= "    <td align=\"center\" valign=\"top\" nowrap><font color=\"#ffffff\">\n";
    $CONTEXT_DROPDOWN .= make_form_select("<small> type </small>", 'type" class="tbox', $r_types, $type, true, 1);
    $CONTEXT_DROPDOWN .= make_submit('small_submit_white.gif', 'submit', 'bottom');
    $CONTEXT_DROPDOWN .= "&nbsp;    </font></td>";
    $CONTEXT_DROPDOWN .= make_form_end();
    $CONTEXT_DROPDOWN .= make_form_start($PHP_SELF, $r_hidden_fields, "post");
    $CONTEXT_DROPDOWN .= "    <td align=\"right\" valign=\"top\" nowrap><font color=\"#ffffff\">\n";
    $CONTEXT_DROPDOWN .= make_form_select("<small>view </small>", 'view" class="tbox', $r_views, $view, true);
    $CONTEXT_DROPDOWN .= make_submit('small_submit_white.gif', 'submit', 'bottom');
    $CONTEXT_DROPDOWN .= "&nbsp;    </font></td>";
    $CONTEXT_DROPDOWN .= make_form_end();
} else {
    $CONTEXT_DROPDOWN .= "    <td colspan=\"2\">&nbsp;</td>\n";
}

if (preg_match("/invalid build/", $r_logdirs[0])) {
    commonHeader("build status");
    display_errors(array("\nERROR - Cannot get the list of log directories for build: ${build}\n"));
    commonFooter();
    exit;
}

if ($build == "admin") {
    commonHeader("build status: cron outputs");
    echo $logTimeStamp;
    // header("Location: ${URLBASE}/showmodule.php?build=${build}&fullpath=${logPath}");
    //echo "<p align=\"center\">show: " . make_link("${URLBASE}/showmodule.php?build=${build}&fullpath=${logPath}", $logPath, false, false);
    $fullpath = $logPath;
    $no_header = 1;
    require("showmodule.php");
    print setAutoRefresh($autorefresh);
    commonFooter();
    exit;
}

if ($view == 'preview') {
    commonHeader("build status: ${logfile}");
    if (!isset($logfile)) {
        display_errors(array("\nERROR - Called in preview mode without a logfile argument\n"));
        commonFooter();
        exit;
    }
    if (!isset($no_cache)) $no_cache = 0;
    echo $logTimeStamp;
    quick_status("${logPath}/${logfile}", $no_cache);
    print_error_table("${logPath}/${logfile}");
    echo "  <center><small>last accessed: $locltime&nbsp;<a href=\"${PHP_SELF}?logdir=${logdir}&build=${build}&view=${view}&type=${type}&logfile=${logfile}&autorefresh=${autorefresh}".(isset($no_form)?"&no_form=${no_form}":"")."&no_cache=1#EOP\">[no_cache]</a></small></center><br>";
    print setAutoRefresh($autorefresh);
    commonFooter();
    exit;
}

if ($view == 'admin_logs') {
    commonHeader("build status: admin logs");
    echo $logTimeStamp;
    print_admin_logs($logPath);
    print setAutoRefresh($autorefresh);
    commonFooter();
    exit;
}

if ($view == 'downloads') {
    commonHeader("build status: downloads");
    echo $logTimeStamp;
    print_download_links($logPath);
    print setAutoRefresh($autorefresh);
    commonFooter();
    exit;
}

if ($view == 'showcase') {
//    header("Location: http://zion.ptcnet.ptc.com/demo/index.jsp");
    commonHeader("build status: showcase");
    echo '
  <table border="0" cellpadding="0" align="center" height="600" width="100%">
    <tr><td align="center">
        <iframe name="showcase"
                width="100%"
                height="100%"
                frameborder="0"
                scrolling="auto"
                src="/demo/index.jsp">
        </iframe>
';
    echo "
    </td></tr>
  </table>
";
    commonFooter();
    exit;
}

if ($view == 'docs') {
    commonHeader("build status: source code documentation");
    $docs_base = builds_base() . "/${build}/install/documentation";
    if (!isset($path)) {
        $path = $docs_base;
    }
    echo "<p align=\"center\"><font size=-2>${path}</font></p>";
    print_docs_directory($path, $docs_base, 'docs');
    print setAutoRefresh($autorefresh);
    commonFooter();
    exit;
}

commonHeader("build status: summary");
  
echo $logTimeStamp;

if (!isset($no_cache)) $no_cache = 0;
$logBuildTypes = array();
$logPlatforms = array();
foreach ($BUILD_TYPES as $bt_short => $bt_long) {
    if (preg_match("/^$type/", $bt_short)) {
        foreach ($PLATFORMS as $pf_short => $pf_long) {
            $buildLogFile = "overnight_${pf_short}_${bt_short}.lgg";
            $status = quick_status("${logPath}/${buildLogFile}", $no_cache);
            if (!preg_match("/N\/A/", $status)) {
                $logfile_name   =  preg_replace("/^(.+)\/([^\/]+)\.lgg$/", "$1/html/$2_status.html", $buildLogFile);
                $r_status = preg_split('/(<br>)/', $status, -1, PREG_SPLIT_NO_EMPTY|PREG_SPLIT_DELIM_CAPTURE);
                $logBuildTypes[$bt_short][$pf_short]  = make_link("$PHP_SELF?logdir=${logdir}&build=${build}&view=preview&type=${type}&logfile=${logfile_name}&autorefresh=${autorefresh}#EOP", $r_status[0], false, false) . "\n";
                array_shift($r_status);
                $logBuildTypes[$bt_short][$pf_short] .= implode('',$r_status) . "\n";
                $logPlatforms[$pf_short] = $pf_long;
            }
        }
    }
}
//print_r($logBuildTypes);

if (count($logBuildTypes)) {
    echo '
<table border="0" cellpadding="3" cellspacing="1" bgcolor="#000000" align="center">
  <tr valign="middle" bgcolor="#ccccff">
    <td bgcolor="#9999cc" width="150">datecode <b>'.$julianDate.'</b></td>
';

    foreach ($logBuildTypes as $key => $value) {
        echo "    <td bgcolor=\"#ccccff\" align=\"center\" valign=\"top\"><b>${BUILD_TYPES[$key]}</b></td>\n";
    }
    echo '  </tr>';

    foreach ($logPlatforms as $pf_short => $pf_long) {
        echo "  <tr valign=\"baseline\" bgcolor=\"#cccccc\">\n";
        echo "    <td align=\"left\" valign=\"top\"><b>${PLATFORMS[$pf_short]}</b></td>\n";
        foreach ($logBuildTypes as $bt_short => $bt_long) {
            if (array_key_exists($pf_short, $logBuildTypes[$bt_short])) {
                echo "    <td align=\"center\" valign=\"top\">\n".$logBuildTypes[$bt_short][$pf_short]."\n    </td>\n";
            } else {
                echo "    <td align=\"center\" valign=\"top\">\nN/A<br>&nbsp;\n";
            }
        }
        echo "  </tr>\n";
    }
    $buildLogFile = "overnight_cdimage.lgg";
    $status = quick_status("${logPath}/${buildLogFile}", $no_cache);
    if (!preg_match("/N\/A/", $status)) {
        echo '  <tr valign="baseline" bgcolor="#cccccc">';
        echo '    <td align="left" valign="top"><b>CD images</b></td>';
        echo '    <td align="center" valign="top" colspan="3">';
        $logfile_name   =  preg_replace("/^(.+)\/([^\/]+)\.lgg$/", "$1/html/$2_status.html", $buildLogFile);
        $r_status = preg_split('/(<br>)/', $status, -1, PREG_SPLIT_NO_EMPTY|PREG_SPLIT_DELIM_CAPTURE);
        echo "      " . make_link("$PHP_SELF?logdir=${logdir}&build=${build}&view=preview&type=${type}&logfile=${logfile_name}&autorefresh=${autorefresh}#EOP", $r_status[0], false, false) . "\n";
        array_shift($r_status);
        echo implode('',$r_status) . "</td>\n";
        echo '  </tr>';
    }
    echo "\n  </table>\n";
} else {
    echo '<p align="center">None of the usual build log files were found.<br>
    You could try to select &quot;admin logs&quot; or &quot;downloads&quot; from the &quot;view&quot; menu above</p>';
}
echo "
  <center><small>last accessed: $locltime&nbsp;
  <a href=\"${PHP_SELF}?logdir=${logdir}&build=${build}&view=${view}&type=${type}&logfile=${logfile}&autorefresh=${autorefresh}".(isset($no_form)?"&no_form=${no_form}":"")."&no_cache=1#EOP\">[no_cache]</a>
  </small></center><br>
";

print setAutoRefresh($autorefresh);
commonFooter();
?>
