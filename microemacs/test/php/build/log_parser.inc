<?php

$r_logpatterns = array();
$r_logreplacements = array();
$r_logpatterns_extra = array();
$r_logreplacements_extra = array();

######################################################################
# get_log_patterns();
#
# retrieve log parsing patterns from the main database defined in site.inc $DB
#
function get_log_patterns(&$r_logpatterns, &$r_logreplacements, $whereString) {
    global $DB;
    $i = 0;
    if (!is_array($r_logpatterns) or !is_array($r_logreplacements)) {
        display_errors(array("get_log_patterns(): the arguments passed were not arrays!\n"));
    }
    $dbquery =  "select * from ".$DB['dbname'].".".$DB['dbparse']." where ".$whereString." order by category,pattern";
    mysql_connect($DB['dbhost'],$DB['dbuser'],$DB['dbpwd'])
        or die("<br>parse_log_line(): Unable to connect to database:<br>\n".mysql_error()."<br>\n");
    mysql_select_db($DB['dbname'])
        or die("<br>parse_log_line(): Unable to select database:<br>\n".mysql_error()."<br>\n");
    $result = mysql_query($dbquery)
        or die("<br>parse_log_line(): Invalid query:<br>\n".mysql_error()."<br>\n");
    while($row = mysql_fetch_array($result)) {
        $r_logpatterns[$i] = stripslashes($row['pattern']);
        $r_logreplacements[$i] = stripslashes($row['replacement']);
        $i++;
    }
    mysql_free_result($result);
    mysql_close();
    unset ($dbquery, $result, $i);
}

######################################################################
# process_line()
#
# lookup the given string for all sorts if patterns and sets a bunch of
# variables accordingly
#
function process_line(&$thisLogFile, &$r_status, $line) {
    global $build, $URLBASE;
    chop($line);
    $line = preg_replace("/<[^>]+>/", "", $line);
    if ( preg_match("/(^\d+)\:(.*)/", $line, $match) ) {
        $line_number = $match[1];
        $line = $match[2];
    }
    if ($line == ""): return; endif;

    if ( preg_match("/^FINISHED/", $line) ) {
        $r_status[BUILD_FINISHED] = "<font size=-2 color=green>$line</font>";
        return;
    }

    if ( preg_match("/^Directory .*\/build\/([^\/]+)\//", $line, $match) ) {
        $r_status[BUILD_ST_DIR] = $match[1];
        return;
    }

    if ( preg_match("/^About to cd to \w?:?(.*\/build\/)(.+)/", $line, $match) ) {
        $fp = $match[1];
        $fm = rtrim($match[2]);
        $r_status[BUILD_LAST] = $r_status[BUILD_CURRENT];
        $r_status[BUILD_CURRENT] = "${fp}${fm}";
        if ($r_status[BUILD_ST_CUR] == 0) {
            return;
        } else {
            $fm = preg_replace("/\"/", "", $r_status[BUILD_LAST]);
            preg_match("/.*\/${build}\/build\/([^\/]+)\/(.*$)/", $fm, $match);
            if ( isset($match[1]) && isset($match[2]) ) {
                $finfo = "<font color=brown>test:</font> <font color=black>[$match[1]]</font> $match[2]<font color=black>: ${r_status[BUILD_ST_CUR]} failure(s)</font>";
            } else {
                preg_match("/.*\/${build}\/build\/([^\/]+$)/", $fm, $match);
                $finfo = "<font color=brown>test:</font> <font color=black>[$match[1]]</font> $match[1]<font color=black>: ${r_status[BUILD_ST_CUR]} failure(s)</font>";
            }
            $colon = split(':',$fm);
            $fm = $colon[(count($colon)-1)];
            $r_status[BUILD_ST_CUR] = 0;
            return "<a href=${URLBASE}/showmodule.php?module=${fm}&build=${build}&fullpath=${thisLogFile}&smokeTests=1>${finfo}</a>";
        }
    }

    if ( preg_match("/^build completed successfully::\s*\(([\w]+)\)/", $line, $match) ) {
        $fm = rtrim($match[1]);
        $r_status[BUILD_LAST] = $r_status[BUILD_CURRENT];
        $r_status[BUILD_CURRENT] = $fm;
        if ($r_status[BUILD_ST_CUR] == 0) {
            return;
        } else {
            $finfo = "<font color=brown>test:</font> <font color=black>[${fm}]</font> ${fm}<font color=black>: ${r_status[BUILD_ST_CUR]} failure(s)</font>";
            $r_status[BUILD_ST_CUR] = 0;
            return "<a href=${URLBASE}/showmodule.php?module=${fm}&build=${build}&fullpath=${thisLogFile}>${finfo}</a>";
        }
    }
        
    //    if ( preg_match("/does not exist$/", $line) or preg_match("/^Cannot open/", $line) ) {
    if ( preg_match("/does not exist$/", $line)
        or preg_match("/No such file or directory$/i", $line)
        or preg_match("/^\tzip warning:/", $line)
        ) {
        $r_status[BUILD_ERRNO]++;
        if ( preg_match("/^Ignoring Directory ([\w]+)/", $line, $match) ) {
            $fm = preg_replace("/\/[\w]+$/", "", $r_status[BUILD_CURRENT]);
            $missingDir = $match[1];
        } else {
            $fm = $r_status[BUILD_CURRENT];
            $missingFile = "...".substr(preg_replace(array("/(No such file or directory)/","/(does not exist)/","/:?\s+$/"), array("","",""), $line), -25);;
        }
        preg_match("/.*\/${build}\/build\/([^\/]+)\/(.*$)/", $fm, $match);
        if ( isset($match[1]) && isset($match[2]) ) {
            $finfo = "<font color=darkcyan>filesys:</font> <font color=black>[$match[1]]</font> $match[2] <font color=black>(".(isset($missingDir) ? "dir: ${missingDir}":"file: ${missingFile}").")</font>";
        } else {
            preg_match("/.*\/${build}\/build\/([^\/]+$)/", $fm, $match);
            $finfo = "<font color=purple>filesys:</font> <font color=black>[$match[1]]</font> $match[1] <font color=black>(".(isset($missingDir) ? "dir: ${missingDir}":"file: ${missingFile}").")</font>";
        }
        $colon = split(':',$fm);
        $fm = $colon[(count($colon)-1)];
        return "<a href=${URLBASE}/showmodule.php?module=${fm}&build=${build}&fullpath=${thisLogFile}&extralines=${line_number}#${line_number} onMouseOver=\"status='${line}'; return true;\">${finfo}</a>";
//        return "<a href=${URLBASE}/showmodule.php?fullpath=${thisLogFile}&build=${build}>${line}</a>";
    }

    if ( preg_match("/^\t\s[fd]\s/", $line) ) {
        $r_status[BUILD_ERRNO]++;
        $fm = $r_status[BUILD_CURRENT];
        preg_match("/.*\/${build}\/build\/([^\/]+)\/(.*$)/", $fm, $match);
        $finfo = "<font color=darkorange>IA build:</font> <font color=black>[$match[1]]</font> $match[2] <font color=black>(...".substr("${line}", -25).")</font>";
//        return "<a href=${URLBASE}/showmodule.php?fullpath=${thisLogFile}&build=${build}>${line}</a>";
        return "<a href=${URLBASE}/showmodule.php?module=${fm}&build=${build}&fullpath=${thisLogFile}&extralines=${line_number}#${line_number} onMouseOver=\"status='${line}'; return true;\">${finfo}</a>";
    }

    if ( preg_match("/^FAILED BEFORE THIS POINT/", $line) ) {
        $r_status[BUILD_STATUS] = 1;
        return;
    }

    if (    preg_match("/^ABORTED/", $line)
         or preg_match("/:\s+Connection \w+ed[ out]?\.?$/", $line)
         or preg_match("/Permission is denied\.$/", $line) ) {
        $r_status[BUILD_ERRNO]++;
        $r_status[BUILD_STATUS] = 2;
        return;
    }

    if ( preg_match("/^BUILDERROR/", $line) ) {
        $r_status[BUILD_ERRNO]++;
        $word = split(' ', $line);
        $fm   = $word[(count($word)-2)];
        $tool = $word[(count($word)-4)];
        $fm = preg_replace("/\"/", "", $fm);
        preg_match("/.*\/${build}\/build\/([^\/]+)\/(.*$)/", $fm, $match);
        if ( isset($match[1]) && isset($match[2]) ) {
            $finfo = "<font color=purple>${tool}:</font> <font color=black>[$match[1]]</font> $match[2] <font color=black>(" . $word[(count($word)-1)] . ")</font>";
        } else {
            preg_match("/.*\/${build}\/build\/([^\/]+$)/", $fm, $match);
            $finfo = "<font color=purple>${tool}:</font> <font color=black>[$match[1]]</font> $match[1] <font color=black>(" . $word[(count($word)-1)] . ")</font>";
        }
        $colon = split(':',$fm);
        $fm = $colon[(count($colon)-1)];

        return "<a href=${URLBASE}/showmodule.php?module=${fm}&build=${build}&fullpath=${thisLogFile}>${finfo}</a>";
    }

    if ( preg_match("/(^\d+)\s+(FAIL)\s+(\S+)\s+(.*)/", $line, $matchTest) ) {
        $r_status[BUILD_ST_FAIL]++;
        $r_status[BUILD_ST_CUR]++;
        return;
    }

    if ( preg_match("/(^\d+)\s+(PASS)/", $line) ) {
        $r_status[BUILD_ST_PASS]++;
        return;
    }

    if ( preg_match("/^ERROR::/", $line) ) {
        $r_status[BUILD_STATUS] = 1;
        $r_status[BUILD_ERRNO]++;
        preg_match("/^(ERROR:: Unable to delete:: ([^<]*))/", $line, $match);
        if ( isset($match[2]) ) {
            $err_msg = $match[1];
            $err_mod = $match[2];
        } else {
            preg_match("/^ERROR:: (.*)! (((See)|(Target file))\s*\(([^\)]+)\)).*(:: \(([^<]*)\))/", $line, $match);
            if ( isset($match[8]) ) {
                $err_mod = $match[8];
                $err_msg = "[$match[8]] $match[1]";
            } else {
                if (count($match)) {
                    $err_msg = $match[1];
                    $err_msg =~ preg_replace("/^(ERROR:: )(Dependency \()([^\)]+)(\).*)/", "$match[2]$match[3]$match[4]", $line);
                    $err_mod = $match[3];
                } else {
                    $err_mod = "";
                    $err_msg = "<font color=red>Hmmmm... got weirdo: $line</font>";
                }
            }
        }
        // ERROR:: component build failed! See (P:\builds\pview\pview_7_0\build\[pview\]logfiles\PVLite-win32_opt.lgg) for details!:: (PVLite)
        preg_match("/\\\build\\\([a-z]+)?\\\?logfiles\\\/", $line, $match);
        $logBase = isset($match[1]) ? "${match[1]}/" : "";
        // overnight_win32_p_opt_pview.lgg
        preg_match("/overnight_([^_]+)_([a-z]_)?([^_]+)(?:_(.*))*\.lgg/", $thisLogFile, $match);
        $modlogpath = builds_base() . "/${build}/build/${logBase}logfiles/${err_mod}-$match[1]_$match[3].lgg";
        return "<a href=${URLBASE}/showmodule.php?fullpath=${modlogpath}&build=${build}>${err_msg}</a>";
    }

    // this gets returned only if the "if" statement above don't return first
    // useful to default to a link to the logfile
    return "<a href=${URLBASE}/showmodule.php?fullpath=${thisLogFile}&build=${build}>${line}</a>";
}

######################################################################
# parse_log_line()
#
# blah
#
function parse_log_line($line) {
    global $r_logpatterns, $r_logreplacements;
    // strip all HTML cack but not on <.*\.h> lines 
    $line = htmlspecialchars(preg_replace("/<[\/\!]?[^<>]*[^\.h]>/si", "", $line));

    $line = preg_replace($r_logpatterns, $r_logreplacements, $line);

    return $line;
}

######################################################################
# parse_test_line()
#
# blah
#
function parse_test_line($line, &$workdir) {
        global $build;
        $buildBase = builds_base();
        $buildBase = str_replace('/', '\/', $buildBase);   // need to quote the "/" for the coming regexp

        if ( preg_match("/^(\d+)\s+(PASS|FAIL)\s+(\S+)\s+(.*)/", $line, $match) ) {
            $testResult = "<font color=" . ($match[2] == "FAIL" ? "red" : "green") . ">" .$match[2] . "</font>";
            $thisSPR = $match[3];
            if ( $thisSPR != "NONE" ) {
                $thisSPR = "<a href=\"http://rdweb.ptc.com/WebSiebel/report.php?spr_no=${match[3]}&mode=spr_details&form=spr&do_not_check_ver=1\" target=\"sprdetails\"><font color=brown>${match[3]}</font></a>";
            }            
            $line = "<tr><td><a href=/logs_win/${workdir}/Test${match[1]}/>${match[1]}</a></td><td><a href=/logs_win/${workdir}/Test${match[1]}/>${testResult}</a></td><td>${thisSPR}</td><td>${match[4]}</td></tr>";
        } else if ( preg_match("/^Directory\s+:- \"\w?:?$buildBase\/(.*)\"\.$/", $line, $match) ) {
            $workdir = $match[1];
        } else if ( preg_match("/^(Test)\s+(Result)\s+(SPR Ref)\s+(Description)/", $line, $match) ) {
            $line = "<br><br><table>\n<tr><td>${match[1]}</td><td>${match[2]}</td><td>${match[3]}</td><td>${match[4]}</td></tr>";
        } else if ( preg_match("/ DONE\.$/", $line) ) {
            $line = "</table>\n<br>${line}";
        } else {
            $line = parse_log_line($line);
        }

    return $line."\n";
}

?>
