<?php

require_once 'prepend.inc';
require_once 'builds.inc';

if (!isset($autorefresh)) { $autorefresh = 0; }
if (!isset($wrap)) { $wrap = 0; }
if (!isset($no_header)) { $no_header = 0; }
if (!isset($smokeTests)) { $smokeTests = 0; }
if (!isset($extralines)) { $extralines = 0; }

if (isset($module)) {
    $CONTEXT_DROPDOWN = preg_replace("/.*\/$build\/build\/(.*)/", "$1", $module);
    $headerTitle = "showing a module...";
} else if (isset($testdir)) {
    $CONTEXT_DROPDOWN = preg_replace("/.*\/$build\/build\/(.*)/", "$1", $testdir);
    $headerTitle = "showing a test directory...";
} else {
    $CONTEXT_DROPDOWN = $fullpath;
    $headerTitle = "showing a logfile...";
}
$CONTEXT_DROPDOWN = "<td colspan=5><font color=\"#ffffff\"><small>" . $CONTEXT_DROPDOWN . "</small></font></td>";

if (!$no_header) {
    commonHeader($headerTitle);
}

if (isset($testdir)) {
    $base_path = builds_base() . "";
    print_directory($testdir, $base_path, false, (isset($module)?"module=${module}&":'').(isset($build)?"build=${build}&":'')."fullpath=${fullpath}");
    print setAutoRefresh($autorefresh);
    commonFooter();
    exit;
}

$fp = @fopen(${fullpath}, "r");
if ( $fp && !feof($fp) ) {

    get_log_patterns($r_logpatterns, $r_logreplacements, "(active = '1' AND (category = 'build' OR category = 'command' OR category = 'cvs'))");
    echo "<a href=\"${PHP_SELF}?".(isset($module)?"module=${module}&":'').(isset($build)?"build=${build}&":'')."fullpath=${fullpath}&smokeTests=${smokeTests}&wrap=".($wrap?0:1)."&autorefresh=${autorefresh}#EOP\"><small>[".($wrap?'no_':'')."wrap]</small></a>";
    echo "<a href=\"${PHP_SELF}?".(isset($module)?"module=${module}&":'').(isset($build)?"build=${build}&":'')."fullpath=${fullpath}&smokeTests=${smokeTests}&wrap=${wrap}&autorefresh=".($autorefresh?0:1)."#EOP\"><small>[".($autorefresh?'no_':'')."autorefresh]</small></a><br>";
    echo $wrap ? "<tt>" : "<pre>";
    if (!isset($module)) {
        while (!feof($fp) ) {
            $line = rtrim(fgets($fp, 4096));
            echo parse_log_line($line) . ($wrap ? "<br>\n" : "\n");
        }
    } else if ($smokeTests) {
        parse_smokeTests($fp);
    } else {
        parse_logSection($fp, $extralines);
    }
    echo $wrap ? "</tt>" : "</pre>";
    fclose($fp);

} else {
        display_errors(array("could not open: \"${fullpath}\" for reading.\n"));
}

if (!$no_header) {
    print setAutoRefresh($autorefresh);
    commonFooter();
}

######################################################################
# parse_logsection()
#
# blah
#
function parse_logsection(&$fp, $extralines) {
    global $module, $wrap;

    // try to locate a BUILDERROR tag; Note the -4 offset taking into account extra header
    $foundMod   = $foundError   = $extraLinesDone = 0;
    $currentLine = -4;
    $parentMod  = preg_replace("/(.*)\/\w+$/", "$1", $module) or $module;
    while (!feof($fp) ) {
        $line = rtrim(fgets($fp, 4096));
        if ( $currentLine == $extralines ) echo "<a name=\"${extralines}\"></a>\n";
        if (!$foundMod) {
            if (preg_match("/^About to cd to \w?:?(\/.+)/", $line, $match)) {
                if ($match[1] == $module) {
                    $foundMod = 1;
                    preg_match("/\/(\w+)$/", $line, $match);
                    echo "<font color=orange>BUILD: Changing directory to ${match[1]}</font>" . ($wrap ? "<br>\n" : "\n");
                    echo parse_log_line($line) . ($wrap ? "<br>\n" : "\n");
                }
            }
        } else if (preg_match("/^BUILD: Returning to directory\s+(.*)/", $line, $match)) {
            if ($match[1] == $parentMod or $foundError) {
                echo parse_log_line($line) . ($wrap ? "<br>\n" : "\n");
                break;
            }
        } else if (preg_match("/^BUILDERROR TARGET:\s+(.*)/", $line, $match)) {
                $foundError = 1;
                echo parse_log_line($line) . ($wrap ? "<br>\n" : "\n");
        } else {
            echo parse_log_line($line) . ($wrap ? "<br>\n" : "\n");
        }
        $currentLine++;
    }
    /*
    while ($extralines && !feof($fp) ) {
        $line = rtrim(fgets($fp, 4096));
        echo parse_log_line($line) . ($wrap ? "<br>\n" : "\n");
        $extralines--;
    }
    */
}

######################################################################
# parse_smokeTests()
#
# blah
#
function parse_smokeTests(&$fp) {
    global $module, $wrap;

    // try to locate the module and then the "regressionRun" tag
    $foundMod = $foundRegr = 0;
    $workdir = "";
    while (!feof($fp) ) {
        $line = rtrim(fgets($fp, 4096));
        if (!$foundMod) {
            if (preg_match("/^About to cd to \w?:?(\/.+)/", $line, $match)) {
                if ($match[1] == $module) { $foundMod = 1; }
            }
        } else if ($foundMod && !$foundRegr) {
            if (preg_match("/^regressionRun/", $line)) {
                $foundRegr = 1;
                echo parse_test_line($line, $workdir) . ($wrap ? "<br>\n" : "\n");
            } else if (preg_match("/^BUILD: Returning to directory/", $line)) {
                display_errors(array("\nERROR - found the module: $module\nbut there does not appear to be a matching \"runRegression\" section for it\n"));
                break;
            }
        } else if ($foundRegr && preg_match("/ DONE\.$/", $line)) {
            echo parse_test_line($line, $workdir) . ($wrap ? "<br>\n" : "\n");
            break;
        } else {
            echo parse_test_line($line, $workdir) . ($wrap ? "<br>\n" : "\n");
        }
    }
}

?>
