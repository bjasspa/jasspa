<?php

require_once 'log_parser.inc';
require_once 'filesystem.inc';

define('BUILD_ERRNO',    0);
define('BUILD_WARN',     1);
define('BUILD_STATUS',   2);
define('BUILD_FINISHED', 3);
define('BUILD_CURRENT',  4);
define('BUILD_LAST',     5);
define('BUILD_ST_FAIL',  6);
define('BUILD_ST_PASS',  7);
define('BUILD_ST_DIR',   8);
define('BUILD_ST_CUR',   9);

######################################################################
# builds_list()
#
# builds up a list of all the build directories available
#
function builds_list($timeSort=false) {
    global $LATEST_BUILD;
    $home_directory = unix_home();
    if ($timeSort) {
        $directory_list = `cd ${home_directory}/buildlogs/ ; ls -dt * 2> /dev/null`;
    } else {
        $directory_list = `cd ${home_directory}/buildlogs/ ; ls -dr * 2> /dev/null`;
    }
    if ($directory_list == "") return NULL;
    $directory_list = split ( ' ', ereg_replace ('[[:space:]]+', ' ', rtrim ($directory_list) ) );
    reset($directory_list);
    while (list($key,$value) = each($directory_list)) {
        if ($value == "admin" or $value == "ARCHIVE") {
            array_splice($directory_list,$key,1);
            reset($directory_list);
        }
    }
    array_push($directory_list,"admin");
    $LATEST_BUILD = $directory_list[0];
    return $directory_list;
}


######################################################################
# logdirs_list()
#
# builds up a list of all the log directories pertaining to a
# specific build (or the latest by default) and returns an array.
# The values are offset from ${build_path}
#
function logdirs_list($build) {

    if (!isset($build)) return NULL;
    $checkFiles = array('overnight_checkout.lgg',
                        'overnight_check.lgg',
                        'cleanup.lgg');
    $build_path = unix_home() . "/buildlogs/" . $build;
    if ( $build == "admin" ) {
        $directory_list = `cd ${build_path}/ ; ls -t * 2> /dev/null`;
    } else {
        while (list($key, $value) = each($checkFiles)) {
            $directory_list = `cd ${build_path}/ ; ls -dt log_*/*/${value} 2> /dev/null`;
            if ( $directory_list != "" ) break;
        }
    }
    if ( !$directory_list ) {
        return NULL;
    } else {
        $directory_arr = split ( ' ', ereg_replace ('[[:space:]]+', ' ', rtrim ($directory_list) ) );
    }
    while (list($key,$value) = each($directory_arr)) {
        if ( $build != "admin" ) {
            $directory_arr[$key] = preg_replace("/^(\w+)\/([^\/]+)\/\w+\.lgg$/", "$1", $directory_arr[$key]);
        }
    }
    return $directory_arr;
}


######################################################################
# make_docs_directory()
#
# makes a nicely HTML formatted output of the given directory
#
function make_docs_directory($path, $base_path=false, $target=false)
{
    $return = "";
    global $build, $logdir, $view;
    if ($base_path && $path == $base_path ) $at_base = true;
    if (is_file($path)) {
        readfile($path);
    } elseif ($dir = @opendir($path)) {
        $tmp_path = strstr($path, "${build}/install/");
        if (!$at_base) {
            readfile("${path}/index.html");
        } else {
            $return .= "<table ".styleTable." cellpadding=\"3\" cellspacing=\"0\" align=\"center\">\n";
            $return .= "<tr valign=\"middle\" bgcolor=\"#9999cc\"><td ".styleBottom." bgcolor=\"#ccccff\" align=\"center\"><b>doxygen documentation</b></td></tr>\n";
            while (($file = readdir($dir)) !== false) {
                if (!preg_match("/\.+$/", $file) or !$at_base) {
                    $return .= "<tr valign=\"baseline\" bgcolor=\"#cccccc\"><td align=left>\n";
                    if (is_dir("$path/$file")) {
                        $return .= make_link("/logs_win/${tmp_path}/${file}/index.html", $file, $target, false);
                    } else {
                        $return .= make_link("/logs_win/${tmp_path}/${file}", $file, $target, false);
                    }
                    $return .= "</td></tr>\n";
                }
            }
            $return .= "</table><p>&nbsp;</p>\n";
            closedir($dir);
        }
    } else {
        return make_errors(array("\nERROR - could not open the directory for reading: ${path}\n"));
    }
    return $return;
}

######################################################################
# print_docs_directory()
#
# prints a nicely HTML formatted output of the given directory
#
function print_docs_directory($path, $base_path=false, $target=false)
{
    print make_docs_directory($path, $base_path, $target);
}


######################################################################
# make_error_table()
#
# works out the name of the status_file for a given log and
# displays the BUILDERROR occurences in a nifty table
#
function make_error_table($thisLogFile) {
    global $build, $URLBASE;

    $return = "";
    $thisLogFileSize = filesize($thisLogFile);
    $return .= "<table border=\"0\" cellpadding=\"0\" cellspacing=\"0\" align=\"center\" width=\"50%\"><tr><td align=\"center\" style=\"border: dashed orange; margin: 1em; padding: 1em;\">\n";
    preg_match("/^(.+)\/([^\/]+)\.lgg$/", $thisLogFile, $match);
//    $return .= make_link("${URLBASE}/mod_perl/showlgg.cgi?f=${thisLogFile}&build=${build}", "<font size=\"+1\">$match[2]</font><br>(full log size: ${thisLogFileSize} bytes)", false, false);
    $return .= make_link("${URLBASE}/showmodule.php?fullpath=${thisLogFile}&build=${build}", "<font size=\"+1\">$match[2]</font>", false, false);
    $return .= "<br><font size=\"-1\">(full log size: ${thisLogFileSize} bytes)</font>";
    $return .= "</td></tr></table><br>\n";
    $thisLogFile = preg_replace("/^(.+)\/([^\/]+)\.lgg$/", "$1/html_tmp/$2_status.html", $thisLogFile);
    $fp = @fopen($thisLogFile, "r");
    if ( $fp && !feof($fp) ) {
        // drop the first line which contains the quick_status
        fgets($fp, 4096);
        $return .= "<table ".styleTable." cellpadding=\"3\" cellspacing=\"0\" align=\"center\">\n";
        $return .= "<tr valign=\"middle\" bgcolor=\"#9999cc\"><td ".styleBottom." bgcolor=\"#ccccff\" align=\"center\"><b>quick links to BUILDERROR sections</b></td></tr>\n";
        while (!feof($fp) ) {
            $line = chop(fgets($fp, 4096));
            $return .= "<tr valign=\"baseline\" bgcolor=\"#cccccc\"><td align=left>${line}</td></tr>\n";
        }
        $return .= "</table>\n";
        fclose($fp);
    } else {
        return make_errors(array("could not open: \"${thisLogFile}\" for reading.<br>Make sure the \"summary\" for this build already exists (simply change view to \"summary\" at the top of the page).<br>if still unlucky, click on the \"page bugs\" link at the top of the page\n"));
    }
    return $return;
}


######################################################################
# print_error_table()
#
# works out the name of the status_file for a given log and
# displays the BUILDERROR occurences in a nifty table
#
function print_error_table($thisLogFile) {
    print make_error_table($thisLogFile);
}


######################################################################
# make_admin_logs()
#
# finds all the build related admin logs in a table.
# these are namely the files not appearing in the quick summary
#
function make_admin_logs($logPath) {
    global $build, $URLBASE, $ADMIN_LOGS;

    $return = "";
    preg_match("/^.+\/([^\/]+)\/([^\/]+)$/", $logPath, $match);

    if ($dir = @opendir($logPath)) {
        $return .= "<table ".styleTable." cellpadding=\"3\" cellspacing=\"0\" align=\"center\" width=\"60%\">\n";
        $return .= "<tr valign=\"middle\" bgcolor=\"#9999cc\"><td ".styleBottom." bgcolor=\"#ccccff\" align=\"center\"><b>build harness logfiles</b></td></tr>\n";
        while (($file = readdir($dir)) !== false) {
            if (    preg_match("/^overnight_.*_(deb|opt).*\.lgg$/", $file)
                ||  preg_match("/^cronoutput_.*\.lgg$/", $file)
                ||  preg_match("/^cronoutput_.*_restart$/", $file)
                ||  preg_match("/^\.+$/", $file)
                ||  ($file == 'html_tmp')) {
                continue;
            } else {
                $return .= "<tr valign=\"baseline\" bgcolor=\"#cccccc\"><td align=left>";
                if (preg_match("/overnight.*_checkout\.lgg\.([\w\.]+)\.html$/", $file, $machine)) {
//                    $return .= make_link ("${URLBASE}/mod_perl/showlgg.cgi?f=${logPath}/${file}&build=${build}", "checkout on $machine[1]", false, false);
                    $return .= make_link ("${URLBASE}/showmodule.php?fullpath=${logPath}/${file}&build=${build}", "checkout on $machine[1]", false, false);
                } elseif (@$ADMIN_LOGS[$file]) {
//                    $return .= make_link ("${URLBASE}/mod_perl/showlgg.cgi?f=${logPath}/${file}&build=${build}", $ADMIN_LOGS[$file], false, false);
                    $return .= make_link ("${URLBASE}/showmodule.php?fullpath=${logPath}/${file}&build=${build}", $ADMIN_LOGS[$file], false, false);
                } else {
//                    $return .= make_link ("${URLBASE}/mod_perl/showlgg.cgi?f=${logPath}/${file}&build=${build}", $file, false, false);
                    $return .= make_link ("${URLBASE}/showmodule.php?fullpath=${logPath}/${file}&build=${build}", $file, false, false);
                }
                $return .= "</td></tr>\n";
            }
        }
        $return .= "<tr valign=\"baseline\" bgcolor=\"#cccccc\"><td align=left>";
        $return .= make_link ("/logs_win/${build}/logfiles/$match[1]/$match[2]", 'log directory browsing', false, false);
        $return .= "</td></tr>\n";
        $return .= "</table><p>&nbsp;</p>\n";
        closedir($dir);
    } else {
        return make_errors(array("\nERROR - could not open the directory for reading: ${logPath}\n"));
    }
    return $return;
}


######################################################################
# print_admin_logs()
#
# prints all the build related admin logs in a table.
# these are namely the files not appearing in the quick summary
#
function print_admin_logs($logPath) {
    print make_admin_logs($logPath);
}


######################################################################
# quick_status()
#
# builds up a list of all the log directories pertaining to a
# specific build (or the latest by default) and returns an array.
# The values are offset from ${build_path}
#
function quick_status($thisLogFile, $no_cache=false) {
    global $build, $URLBASE,
           $r_logpatterns, $r_logreplacements,
           $r_logpatterns_extra, $r_logreplacements_extra;
    $r_status = array(  BUILD_ERRNO     => 0,
                        BUILD_WARN      => 0,
                        BUILD_STATUS    => 0,
                        BUILD_FINISHED  => "",
                        BUILD_CURRENT   => "",
                        BUILD_LAST      => "",
                        BUILD_ST_FAIL   => 0,
                        BUILD_ST_PASS   => 0,
                        BUILD_ST_DIR    => "",
                        BUILD_ST_CUR    => 0);

    // let's not waste any time in that case ...
    if ( !file_exists($thisLogFile) ) {
        return "<font color=\"purple\">N/A</font>";
    }

    preg_match("/^(.+)\/([^\/]+)\.lgg$/", $thisLogFile, $match);
    $status_file_name = "$match[1]/html_tmp/$match[2]_status.html";
    $html_dir = "$match[1]/html_tmp";
    if ( !$no_cache && file_exists($status_file_name) && (filemtime("$thisLogFile") < filemtime($status_file_name)) ) {
        $fp = @fopen($status_file_name, "r");
        if ( $fp && !feof($fp) ) {
            // the return_status is always stored on the first line of the file
            $return_status = chop(fgets($fp, 4096));
            fclose($fp);
        } else {
            return make_errors(array("could not open \"${status_file_name}\" for reading. Check permissions\n\t"));
        }
        return $return_status;
    }

    // bodge for Solaris machine. not necessary on Linux
    if ( is_executable("/usr/xpg4/bin/grep") ) {
        $grep_cmd = "/usr/xpg4/bin/grep";
    } else {
        $grep_cmd = "grep";
    }

    $r_status[BUILD_WARN] = trim(`${grep_cmd} -E -i -c warning ${thisLogFile}`);

    $grep_string = "";
    $testLogDetected = 0;
    if (!count($r_logpatterns)) {
        get_log_patterns($r_logpatterns, $r_logreplacements, "(active = '1' AND (category = 'grep'))");
    }
    if (!count($r_logpatterns_extra)) {
        get_log_patterns($r_logpatterns_extra, $r_logreplacements_extra, "(active = '1' AND (category = 'grep_extra'))");
    }
    foreach ($r_logpatterns as $pattern) $grep_string .= "-e \"".$pattern."\" ";

    if (preg_match("/_test\.lgg$/", $thisLogFile)) {
        $testLogDetected = 1;
        foreach ($r_logpatterns_extra as $pattern) $grep_string .= "-e \"".$pattern."\" ";
    } elseif (preg_match("/_cdimage\.lgg$/", $thisLogFile)) {
        foreach ($r_logpatterns_extra as $pattern) $grep_string .= "-e \"".$pattern."\" ";
    } elseif (preg_match("/_win32_.*\.lgg$/", $thisLogFile)) {
        foreach ($r_logpatterns_extra as $pattern) $grep_string .= "-e \"".$pattern."\" ";
    }

    $grep_results = split ("\n",`${grep_cmd} -n -E ${grep_string} ${thisLogFile}`);
//    echo "<b>grep_results for: ${grep_cmd} -n -E ${grep_string} ${thisLogFile}</b><br>\n";
//    print_r($grep_results);
    $text = $add = "";
    while (list($key,$value) = each($grep_results)) {
        $add = process_line($thisLogFile, $r_status, $value);
        if ( $add != "" ) {
            $text .= $add . "<br>\n";
        }
    }

//    $buildResults = "(E${r_status[BUILD_ERRNO]}-W${r_status[BUILD_WARN]})";
//    $testResults  = "(P${r_status[BUILD_ST_PASS]}-F${r_status[BUILD_ST_FAIL]})";
    $buildResults = "${r_status[BUILD_ERRNO]} err(s) &amp; ${r_status[BUILD_WARN]} warn(s)";
    $testResults  = "<font color=black>tests: </font><font color=green>${r_status[BUILD_ST_PASS]} pass</font> &amp; <font color=red>${r_status[BUILD_ST_FAIL]} fail</font>";

    if ( $r_status[BUILD_STATUS] == 2 ) {
        $text .= "<font color=red><b>BUILD ABORTED!</b></font>";
        $text = "<font color=red><b>ABORTED</b></font>\n" . $text;
        $return_status = "<font color=red><b>ABORTED</b></font>";
    } elseif ( strlen($r_status[BUILD_FINISHED]) == 0 ) {
        preg_match("/(.*\/build\/)(.+)/", $r_status[BUILD_CURRENT], $match);
        $fp = @$match[1];
        $fm = rtrim(@$match[2]);
        preg_match("/([^\/]+)\/(.*$)/", $fm, $match);
        if ( isset($match[2]) ) {
            $finfo = "<font color=black>[$match[1]]</font> $match[2]";
        } elseif ( isset($match[1]) ) {
            $finfo = "<font color=black>[$fm]</font>";
        } else {
            $fm = $r_status[BUILD_CURRENT];
            $finfo = "<font color=black>[$fm]</font>";
        }
//        $text .= "<font color=orange>currently in: </font><a href=${URLBASE}/mod_perl/showmodule.cgi?module=" . $r_status[BUILD_CURRENT] . "&build=${build}&fp=${thisLogFile}&no_error=1><font size=-2>${finfo}</font></a>\n";
        if ( isset($match[1]) ) {
            $text .= "<font color=orange>currently in: </font><a href=${URLBASE}/showmodule.php?module=" . $r_status[BUILD_CURRENT] . "&build=${build}&fullpath=${thisLogFile}><font size=-2>${finfo}</font></a>\n";
        } else {
            $text .= "<font color=orange>last module successfully built: </font><a href=${URLBASE}/showmodule.php?build=${build}&fullpath=${thisLogFile}#EOP><font size=-2>${finfo}</font></a>\n";
        }
        $text .= "<font color=orange><b>BUILD STILL IN PROGRESS</b> <small>(or quite broken)</small></font> with ${buildResults}...";
        if (!$testLogDetected) {
            $text = "<font color=orange>IN PROGRESS</font><br>$buildResults\n" . $text;
            $return_status = "<font color=orange>IN PROGRESS</font><br>$buildResults";
        } else {
            $text = "<font color=orange>IN PROGRESS: </font>$buildResults<br>$testResults\n" . $text;
            $return_status = "<font color=orange>IN PROGRESS: </font>$buildResults<br>$testResults";
        }
    } elseif ( $r_status[BUILD_STATUS] == 1 ) {
        $text .= $r_status[BUILD_FINISHED] . "\n";
        $text .= "<font color=red><b>BUILD FAILED</b></font> with at least ${buildResults}...";
        if (!$testLogDetected) {
            $text = "<font color=red>FAILED</font><br>$buildResults\n" . $text;
            $return_status = "<font color=red>FAILED</font><br>$buildResults";
        } else {
            $text = "<font color=red>build:</font> $buildResults<br>$testResults\n" . $text;
            $return_status = "<font color=red>build:</font> $buildResults<br>$testResults";
        }
    } else {
        if ( $r_status[BUILD_ERRNO] == 0 ) {
            $text .= $r_status[BUILD_FINISHED] . "\n";
            $text .= "<font color=green><b>BUILD SUCCEEDED!!!</b></font> with ${buildResults}";
            if (!$testLogDetected) {
                $text = "<font color=green>SUCCEEDED</font><br>$buildResults\n" . $text;
                $return_status = "<font color=green>SUCCEEDED</font><br>$buildResults";
            } else {
                $text = "<font color=green>build:</font> $buildResults<br>$testResults\n" . $text;
                $return_status = "<font color=green>build:</font> $buildResults<br>$testResults";
            }
        } else {
            $text .= "<font color=red><b>BUILD FAILED</b></font> with ${buildResults}!";
            if (!$testLogDetected) {
                $text = "<font color=red>FAILED (STRANGE)</font><br>***$buildResults***\n" . $text;
                $return_status = "<font color=red>FAILED (STRANGE)</font><br>***$buildResults***";
            } else {
                $text = "<font color=red>build: </font>***$buildResults***<br>$testResults\n" . $text;
                $return_status = "<font color=red>build:</font> ***$buildResults***<br>$testResults";
            }
        }
    }

    if ( !file_exists($html_dir) ) {
        mkdir($html_dir, 0777);
        chmod ($html_dir, 0777);
    }
    $fp = @fopen($status_file_name, "w+");
    if ($fp) {
        fwrite($fp,$text);
        fclose($fp);
        chmod ($status_file_name, 0777);
    } else {
        return make_errors(array("could not open \"${status_file_name}\" for writing. Check permissions\n\t"));
    }
    return $return_status;
}


######################################################################
# make_download_links()
#
# makes a list of download links if the target file is available
#
function make_download_links($logPath) {
    global $build, $FTP_FILES;

    $return = '';
    ### assume that the build stream is pview_6_4 and then work out exceptions
    ###
    $buildStream = preg_replace("/^(pview_\d+_\d+).*/", "$1",$build);
    if (preg_match("/^tyr/", $build) )          $buildStream = 'pview_6_3';
    if (preg_match("/^odin/", $build) )         $buildStream = 'pview_6_4';
    if (preg_match("/^loki/", $build) )         $buildStream = 'pview_7_0';
    ### special build names
    ###
    if (preg_match("/^pview_6_0/", $build) )    $buildStream = '6.0';
    if (preg_match("/^tyr_phase_9$/", $build) ) $buildStream = '620';
    if (preg_match("/^pview_6_2_MOR1$/", $build) ) $buildStream = '621';
    ### default directory under which we automatically place downloads
    ###
    $releaseDir = 'development';
    if ($buildStream == '6.0')                  $releaseDir     = 'DSU';

    $julianDate = getJulianDateForBuild($logPath);
    if ( !preg_match("/^DEV/", $julianDate) ) {
        $verBuild = getDatecodeInfo($logPath, "VER_BUILD");
    } else {
        $verBuild = "DEV";
    }
    
    if (builds_domain() == "division.co.uk") {
        $ftpServer    = "vast.ptcnet.ptc.com";
        $ftpPathBase  = "/builds/pview/downloads";
        $ftpUserName  = "builds";
        $ftpUserPass  = "waiwurr1e";
        $ftpPrefix    = "ftp://${ftpUserName}:${ftpUserPass}@${ftpServer}";
    } elseif (builds_domain() == "needham.ptc.com") {
        $ftpServer    = "orange.ptcnet.ptc.com";
        $ftpPathBase  = "/builds/pview/downloads";
        $ftpUserName  = "builds";
        $ftpUserPass  = "waiwurr1e";
        $ftpPrefix    = "ftp://${ftpUserName}:${ftpUserPass}@${ftpServer}";
    } else {
        $ftpServer    = "zion.ptcnet.ptc.com";
        $ftpPathBase  = "/builds";
        $ftpUserName  = "anonymous";
        $ftpUserPass  = "email@ptc.com";
        $ftpPrefix    = "ftp://${ftpServer}";
    }

    $connID = ftp_connect($ftpServer);
    $loginResult = ftp_login($connID, $ftpUserName, $ftpUserPass);
    if ((!$connID) || (!$loginResult)) {
        $return .= "<p align=\"center\">FTP connection has failed!";
        $return .= "<br>Attempted to connect to $ftpServer as user $ftpUserName";
        return;
    } else {
        $return .= "<p align=\"center\"><small>FTP: user ${ftpUserName} successfully connected to ${ftpServer}</small>";
    }

    $ftpPath = "${ftpPathBase}/${buildStream}/${verBuild}_${julianDate}";
    if (!@ftp_chdir($connID, $ftpPath)) {
        $ftpPath = "${ftpPathBase}/${buildStream}/${julianDate}";
        if (!@ftp_chdir($connID, $ftpPath)) {
            $ftpPath = "${ftpPathBase}/${buildStream}/${releaseDir}/${verBuild}_${julianDate}";
            if (!@ftp_chdir($connID, $ftpPath)) {
                $ftpPath = "${ftpPathBase}/${buildStream}/${releaseDir}/${julianDate}";
                if (!@ftp_chdir($connID, $ftpPath)) {
                    $return .= "<p align=\"center\">no FTP directory found for this build!";
                    return $return;
                }
            }
        }
    }

    $return .= "<p><table ".styleTable." cellpadding=\"3\" cellspacing=\"0\" align=\"center\" width=\"60%\">\n";
    $return .= "<tr valign=\"middle\" bgcolor=\"#9999cc\"><td ".styleBottom." bgcolor=\"#ccccff\" align=\"center\"><b>downloads for ${build}</b><br>(datecode: ${julianDate})</td></tr>\n";
    $return .= "<tr valign=\"baseline\" bgcolor=\"#cccccc\"><td align=\"center\">&nbsp;&nbsp;</td></tr>\n";
    while (list($key,$value) = each($FTP_FILES)) {
        $ftpSize = ftp_mdtm($connID, $value);
        if ($ftpSize > 0) {
            preg_match("/^(.+)\//", $value, $match);
            $return .= "<tr valign=\"baseline\" bgcolor=\"#cccccc\"><td align=\"center\">&nbsp;&nbsp;";
//            $return .= "<img src=/images/bullet.gif align=middle>&nbsp;";
            $return .= make_popup_link ("${ftpPrefix}${ftpPath}/$match[1]", $key, 'FTP', "menubar=no,toolbar=no,scrollbars,resizable,height=400,width=600,left=10,top=25", false);
//            $return .= "&nbsp;<img src=/images/bullet.gif align=middle>";
            $return .= "</td></tr>\n";
        }
    }
    $return .= "</table>\n";

    $dateEndEpoch = (filectime("${logPath}/.."));
    $dateEnd = date("Y-m-d", $dateEndEpoch);
    $dateStartEpoch = ($dateEndEpoch - (7*86400));
    $dateStart = date("Y-m-d", $dateStartEpoch);
    $return .= "<p align=\"center\">weekly submissions for ${buildStream}: ";
    if ($buildStream == "pview_7_0") { $buildStream = 'HEAD'; }
    $return .= make_link("http://zion.ptcnet.ptc.com/builds/mod_perl/showcommitlog.cgi?branch_name=${buildStream}&bug_number=%5Cd%2B&start_date=${dateStart}&end_date=${dateEnd}&submitted_range=1", "[spr]", 'commit');
    $return .= make_link("http://zion.ptcnet.ptc.com/builds/mod_perl/showcommitlog.cgi?branch_name=${buildStream}&start_date=${dateStart}&end_date=${dateEnd}&submitted_range=1", "[all]", 'commit');
    $return .= "</p>";

    $ftpSize = ftp_mdtm($connID, "cdimages/README.txt");
    if ($ftpSize > 0) {
        $return .= "<h2 align=\"center\">README for this particular set of CD images</h2><pre>\n";
        $return .= htmlspecialchars(implode("", file("${ftpPrefix}${ftpPath}/cdimages/README.txt")));
        $return .= "</pre>\n";
    } else {
        $ftpList = ftp_nlist($connID, "cdimages/*.txt");
        if ( count($ftpList) && ftp_size($connID, $ftpList[0]) != -1 ) {
            $return .= "<h2 align=\"center\">README(s) for this particular set of CD images</h2>\n<p align=\"center\">";
            foreach($ftpList as $readmeFile) {
                $return .= make_popup_link ("${ftpPrefix}${ftpPath}/$readmeFile", preg_replace("/cdimages\//", "", $readmeFile), 'README', "menubar=no,toolbar=no,scrollbars,resizable,left=20,top=30", false);
                $return .= "<br>\n";
            }
            $return .= "</p>\n";
        }
    }

    ftp_quit($connID);
    return $return;
}


######################################################################
# print_download_links()
#
# prints download links if the target file is available
#
function print_download_links($logPath) {
    print make_download_links($logPath);
}

?>
