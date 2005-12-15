<?php

######################################################################
# getFormattedDateTime()
#
# returns the formatted modification date + time of a given file
#
function getFormattedDateTime($file) {
    $filemod = filemtime($file);
    $filemodtime = date("l j F Y h:i:s A", $filemod);
    return $filemodtime;
}


######################################################################
# generateJulianDateForFile()
#
# returns the formatted modification date + time of a given file
# in Julian Date format (YYYYJJJ)
#
function generateJulianDateForFile($file) {
    $filemod = @filemtime($file);
    $filemodtime = date("Yz", $filemod);
    return $filemodtime;
}


######################################################################
# getJulianDateForBuild()
#
# returns the Juliandate associated with a given build (identified by
# the logPath parammeter the return format is the
# Julian Date format (YYYYJJJ) or "no_datecode"
#
function getJulianDateForBuild($logPath) {
    $julianDate = "no_datecode";
    if ( @file_exists("${logPath}/overnight_checkout.lgg") ) {
        $fp = @fopen("${logPath}/overnight_checkout.lgg", "r");
    } elseif ( @file_exists("${logPath}/cleanup.lgg") ) {
        $fp = @fopen("${logPath}/cleanup.lgg", "r");
    } else {
        $julianDate = generateJulianDateForFile("${logPath}/cleanup.lgg")
                   || generateJulianDateForFile("${logPath}/overnight_checkout.lgg")
                   || "no_datecode";
    }
    if ( isset($fp) && !feof($fp) ) {
        preg_match("/<html><head><title>([\w-_]+)/", chop(fgets($fp, 4096)), $match);
        $julianDate = $match[1];
        fclose($fp);
    } else {
        $julianDate = "failed to open file";
    }
    return $julianDate;
}


######################################################################
# getDatecodeInfo()
#
# returns the Juliandate associated with a given build (identified by
# the logPath parammeter the return format is the
# Julian Date format (YYYYJJJ) or "no_datecode"
#
function getDatecodeInfo($logPath, $infoStr) {
    $datecodeFile  = builds_base()
                . "/"
                . preg_replace("/.*\/buildlogs\/(\w+)\/.*/", "$1", $logPath)
                . "/build/pview/pvresenu/datecode.h";
    $buildNo = "undefined";
    if ( @file_exists("${datecodeFile}") ) {
        $fp = @fopen("${datecodeFile}", "r");
        while ( isset($fp) && !feof($fp) ) {
            $tmpStr = chop(fgets($fp, 4096));
            if ( preg_match("/#define\s+${infoStr}\s+(\d+)$/", $tmpStr, $match) ) {
                $buildNo = $match[1];
                break;
            }
        }
        if (isset($fp)) fclose($fp);
    }
    return $buildNo;
}


######################################################################
# unixTm()
#
# returns the unix formatted time
#
function unixTm($strDT) {
    $arrDT = explode(" ", $strDT);
    $arrD  = explode("-", $arrDT[0]);
    $arrT  = explode(":", $arrDT[1]);
    return mktime($arrT[0], $arrT[1], $arrT[2], $arrD[1], $arrD[2], $arrD[0]);
}


######################################################################
# dateDiff()
#
# returns the difference between two date stamps
#
function dateDiff ($date1,$date2) {
    $dt2 = unixTm($date2) ;
    $dt1 = unixTm($date1) ;
    $r   = ($dt2 - $dt1);
    $dd  = floor($r / 86400) ;
    if ($dd<=9) $dd = "0".$dd ;
    $r   = $r % 86400 ;
    $hh  = floor($r / 3600) ;
    if ($hh<=9) $hh = "0".$hh ;
    $r   = $r % 3600 ;
    $mm  = floor($r / 60) ;
    if ($mm<=9) $mm = "0".$mm ;
    $r   = $r % 60 ;
    $ss  = $r ;
    if ($ss<=9) $ss = "0".$ss ;
    return "$hh:$mm:$ss" ;
}


function date_diff($date1, $date2) {
    $s = (strtotime($date2)-strtotime($date1));
    $d = intval($s/86400);
    $s -= ($d*86400);
    $h = intval($s/3600);
    $s -= ($h*3600);
    $m = intval($s/60);
    $s -= ($m*60);
    return array("d"=>$d,"h"=>$h,"m"=>$m,"s"=>$s);
}


// To get the monday for a week (german Kalenderwoche) of the current year use
// this function:
function kwday($kw) {
    return mktime(0, 0, 0, 1, 1 + ((8 - date("w", mktime(0,0,0,1,1,date("Y")))) % 7) + ($kw - 2) * 7, date("Y"));
}


// We have meetings once a month.  For instance, one might be on the third
// Thursday of each month ... or the first Monday, etc.  I wanted a function to
// return the date of next month's meeting given the date of this month's
// meeting.

// Here are a couple of functions that do that:
// <h1>Find the next meeting date</h1>

/* Convert date in YYYYMMDD format to unix format */
function date_to_epoch ($d) {
    $yesterday = date("ymd",time()-24*3600);
    // return mktime(3, 0, 0, substr($d, 4, 2), substr($d, 6, 2), substr($d, 0, 4));
    return mktime(3, 0, 0, substr($d, 4, 2), substr($d, 6, 2), substr($d, 0, 4));
}

/* Find the next meeting day, assuming you want to
  meet one month later on the same day of the week
  and the same week of the month. */
function next_month($d) {
    // Find where we are in the month
    $day_of_week = date("w", $d);
    $week_of_month = ceil(date("d", $d)/7);
    // Find first day of next month
    $d2 = date_to_epoch ( date("Ymd",mktime (0,0,0,date("m",$d)+1,1,date("Y",$d))));
    // Find week adjustment factor
    $day_of_week2 = date("w", $d2);
    $week_adjustment = $week_of_month;
    if ($day_of_week >= $day_of_week2) { $week_adjustment -= 1; }
    // Find day adjustment factor
    $day_adjustment = $day_of_week - $day_of_week2;
    // Find the target day
    $dy2 = (($week_adjustment) * 7) + $day_adjustment + 1;
    // Get month and year to assemble new date variable to return
    $mo2 = date("m", $d2);
    $yr2 = date("Y", $d2);

    // Show calendar to enable a visual check of this function
    // $cal = shell_exec("cal -3 $mo2 $yr2");
    // echo "<pre>$cal</pre>";
    // Return date in YYYYMMDD format
    return date("Ymd", mktime (0,0,0,$mo2,$dy2,$yr2));
}

/* Main rountine */
//if ($start_date) {
//    echo "Current Meeting is: $start_date<br/>\n";
//    $start = date_to_epoch($start_date);
//    $next_meeting = next_month($start);
//    echo "Next Meeting is: $next_meeting<br/>\n";
//}

// <form action=$PHP_SELF method="get">
// <input type="text" name="start_date"\ value="$next_meeting"> Format = YYYYMMDD<br/>
// <input type="submit" name="submit" value="Submit"\>
// </form>

######################################################################
# make_directory()
#
# makes an array of the given directory
#
function make_directory($path, $base_path, $target=false, $title=false, $extras='')
{
    global $PHP_SELF;
    $return = '';
    $at_base = false;
    if ($base_path && $path == $base_path ) {
        $at_base = true;
    }

    if (!$title) { $title = $path; }

    if ($dir = @opendir($path)) {
        $return .= "<p><table ".styleTable." cellpadding=\"3\" cellspacing=\"0\" align=\"center\" width=\"60%\">\n";
        $return .= "<tr valign=\"middle\" bgcolor=\"#9999cc\"><td ".styleBottom." bgcolor=\"#ccccff\" align=\"center\"><b>${title}</b></td></tr>\n";
        while (($file = readdir($dir)) !== false) {
          if (! (preg_match("/\.+$/", $file) && $at_base) ) { // skip "dotted" files and directories
            if ($file != "." && $file != "TODO") {
              $return .= "<tr valign=\"baseline\" bgcolor=\"#cccccc\"><td align=left>\n";
            }
            if (is_file("${path}/${file}")) {
              $file_url = $path . "/" . rawurlencode($file);
              $return .= make_link($file_url, $file, $target, false);
            } elseif ( preg_match("/\.+$/", $file) ) {
              if ($file == "..") {
                $true_path = preg_replace("/([\w\/]+)\/(\w+)$/", "$1", $path);
                $return .= make_link("$PHP_SELF?path=${true_path}${extras}", $file, $target, false);
              }
            } else {
              if ($file != "TODO") {
                $return .= make_link("$PHP_SELF?path=${path}/${file}${extras}", $file, $target, false);
              }
            }
            $return .= "</td></tr>\n";
          }
        }
        $return .= "</table><p>&nbsp;</p>\n";
        closedir($dir);
    } else {
        $return .= make_errors(array("\nERROR - could not open the directory for reading: ${path}\n"));
    }
    return $return;
}

######################################################################
# print_directory()
#
# echo an array of the given directory
#
function print_directory($path, $base_path, $target=false, $title=false, $extras='') {
    echo make_directory($path, $base_path, $target=false, $title, $extras);
}

######################################################################
# ob_file_get_contents()
#
# shut output buffering and gathers the content of the given file
# into a variable
#
function ob_file_get_contents($filename) {
    ob_start();
    $retval = @readfile($filename);
    if (false !== $retval) { // no readfile error
        $retval = ob_get_contents();
    }
    ob_end_clean();
    return $retval;
}
?>
