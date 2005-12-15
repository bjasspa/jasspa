<?php

//to print a form:
//make_form_start($action, &$r_hidden, $method="post");
//make_form_select($caption, $select_name, &$r_selects, $selected="");
//make_submit('small_submit.gif', 'submit', 'bottom');
//make_form_end();

// send a redirect to the specified page
// MS IIS/PWS has a bug in which it does not allow us to send a cookie
// and a redirect in the same HTTP header.
// See the following for more info on the IIS bug:
//   http://www.faqts.com/knowledge_base/view.phtml/aid/9316/fid/4

global $e107;

//define('styleTable',  $e107 ? '' : "style=\"BORDER-RIGHT: #000 1px solid; BORDER-TOP: #000 1px solid; BORDER-LEFT: #000 1px solid; BORDER-BOTTOM: #000 1px solid\"");
define('styleTable',  $e107 ? '' : 'style="BORDER-RIGHT: #000 1px solid; BORDER-TOP: #000 1px solid; BORDER-LEFT: #000 1px solid; BORDER-BOTTOM: #000 1px solid"');
define('styleBottom', $e107 ? '' : "style=\"BORDER-BOTTOM: #000 1px solid\"");
define('styleTop',    $e107 ? '' : "style=\"BORDER-TOP: #000 1px solid\"");
define('styleRow',    $e107 ? '' : "valign=\"baseline\" bgcolor=\"#cccccc\"");
define('styleHead',   $e107 ? '' : "valign=\"middle\" bgcolor=\"#9999cc\"");

function do_redirect ( $url ) {
  if ( substr ( $SERVER_SOFTWARE, 0, 5 ) == "Micro" ) {
    echo "<HTML><HEAD><TITLE>Redirect</TITLE>" .
      "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL=$url\"></HEAD><BODY>" .
      "Redirecting to ... <A HREF=\"" . $url . "\">here</A>.</BODY></HTML>.\n";
  } else {
    Header ( "Location: $url" );
    echo "<HTML><HEAD><TITLE>Redirect</TITLE></HEAD><BODY>" .
      "Redirecting to ... <A HREF=\"" . $url . "\">here</A>.</BODY></HTML>.\n";
  }
  exit;
}

#?php#################################################################
# spacer()
#
# print a IMG tag for a sized spacer GIF
#

function spacer($width=1, $height=1, $align=false, $extras=false) {
    global $URLBASE;
    printf("<img src=\"%s/gifs/spacer.gif\" width=\"%d\" height=\"%d\" border=\"0\" alt=\"\" %s%s>",
        $URLBASE,
        $width,
        $height,
        ($align ? 'align="'.$align.'" ' : ''),
        ($extras ? $extras : '')
    );
}

#?php#################################################################
# resize_image()
#
# tag the output of make_image() and resize it manually
#

function resize_image($img, $width=1, $height=1) {
    $str = preg_replace('/width=\"([0-9]+?)\"/i', '', $img );
    $str = preg_replace('/height=\"([0-9]+?)\"/i', '', $str );
    $str = substr($str,0,-1) . sprintf(' height="%s" width="%s">', $height, $width );
    return $str;
}

#?php#################################################################
# make_image()
#
# return an IMG tag for a given file (relative to the images dir)
#

function make_image($file, $alt=false, $align=false, $extras=false, $dir=false, $border=0) {
    global $HTTP_SERVER_VARS, $URLBASE;
    if (!$dir) {
        $dir = "${URLBASE}/gifs";
    }
    if ($size = @getimagesize($HTTP_SERVER_VARS['DOCUMENT_ROOT'].$dir.'/'.$file)) {
        $image = sprintf('<img src="%s/%s" border="%d" %s title="%s" %s%s>',
            $dir,
            $file,
            $border,
            $size[3],
            ($alt    ? $alt : ''),
            ($align  ? ' align="'.$align.'"'  : ''),
            ($extras ? ' '.$extras            : '')
        );
    } else {
        $image = sprintf('<img src="%s/%s" border="%d" title="%s" %s%s>',
            $dir,
            $file,
            $border,
            ($alt    ? $alt : ''),
            ($align  ? ' ALIGN="'.$align.'"'  : ''),
            ($extras ? ' '.$extras            : '')
        );
    }
    return $image;
}

#?php#################################################################
# print_image()
#
# print an IMG tag for a given file
#

function print_image($file, $alt=false, $align=false, $extras=false, $dir=false, $border=0) {
    print make_image($file, $alt, $align, $extras, $dir);
}

#?php#################################################################
# make_submit()
#
#  - make a submit button image
#
function make_submit($file, $alt=false, $align=false, $extras=false, $dir=false, $border=0) {
    // SUN submit style:
    // '<input type="submit" border="0" class="buttonred" value="&nbsp;&raquo;&nbsp;" onmouseover="this.style.color=\'#fbe249\';" onmouseout="this.style.color=\'#FFF\';" />'
    $return = make_image($file, $alt, $align, $extras, $dir, $border);
    if ($return != "<img>") {
        $return = '<input type="image"'.($extras ? ' '.$extras : '').substr($return,4);
    } else {
        $return = '<input type="submit"'.($extras ? ' '.$extras : '').'>';
    }
    return $return;
}

#?php#################################################################
# print_submit()
#
# print a submit form
#

function print_submit($file, $alt=false, $align=false, $extras=false, $dir=false, $border=0) {
    print make_submit($file, $alt, $align, $extras, $dir, $border);
}

#?php#################################################################
# delim()
#
# print a pipe delimiter
#

function delim($color=false) {
    if (!$color) {
        return ' | ';
    }
    return sprintf('<font color="%s"> | </font>', $color );
}

#?php#################################################################
# hdelim()
#
# print a horizontal delimiter (just a wide line);
#

function hdelim($color="#000000") {
    if (!$color) {
        return '<hr noshade size="1">';
    }
    return sprintf('<hr noshade size="1" color="%s">', $color );
}

#?php#################################################################
# make_link()
#
# return a hyperlink to something, within the site
#

function make_link ($url, $linktext=false, $target=false, $extras=false) {
    return sprintf("<a href=\"%s\"%s%s>%s</a>",
        $url,
        ($target ? ' target="'.$target.'"' : ''),
        ($extras ? ' '.$extras : ''),
        ($linktext ? $linktext : $url)
    );
}

#?php#################################################################
# print_link()
#
# echo a hyperlink to something, within the site
#

function print_link($url, $linktext=false, $target=false, $extras=false) {
    echo make_link($url, $linktext, $target, $extras);
}

#?php#################################################################
# make_popup_link()
#
# return a hyperlink to something, within the site, that pops up a new window
#

function make_popup_link ($url, $linktext=false, $target=false, $windowprops="", $extras=false) {
    return sprintf("<a href=\"%s\" target=\"%s\" onclick=\"window.open('%s','%s','%s');return false;\" %s>%s</a>",
        htmlspecialchars($url),
        ($target ? $target : "_new"),
        htmlspecialchars($url),
        ($target ? $target : "_new"),
        $windowprops,
        ($extras ? ' '.$extras : ''),
        ($linktext ? $linktext : $url)
    );
}

#?php#################################################################
# print_popup_link()
#
# echo a hyperlink to something, within the site, that pops up a new window
#

function print_popup_link($url, $linktext=false, $windowprops="", $target=false, $extras=false) {
    echo make_popup_link($url, $linktext, $windowprops, $target, $extras);
}

#?php#################################################################
# download_link()
#
# print a link for a downloadable file (including filesize)
#

function download_link($file, $title) {
    global $filesizes;

    if ($tmp = strrchr($file, "/")) {
        $local_file = substr($tmp, 1, strlen($tmp));
    } else {
        $local_file = "distributions/$file";
    }

    $size = @filesize($local_file)/1024;
    //echo("<!-- file=$file localfile=$local_file size-$size -->\n");

    print_link("do_download.php?download_file=$file", $title);

    if (!$size && isset($filesizes[$local_file])) { $size = $filesizes[$local_file]; }
    if ($size) { echo " [" . number_format($size, 0, ".", ",") . "Kb]"; }
}

#?php#################################################################
# make_table()
#
# print the beginning of a table
#

function make_table($cellpadding,$cellspacing,$border,$bgcolor,$width,$align) {
    return sprintf("<table cellpadding=%d cellspacing=%d border=%d bgcolor=#%s width=%d%% align=%s>",
                   ($cellpadding ? $cellpadding : ''),
                   ($cellspacing ? $cellspacing : ''),
                   ($border ? $border : ''),
                   ($bgcolor ? $bgcolor : ''),
                   ($width ? $width : '100'),
                   ($align ? $align : '')
                   );
}

#?php#################################################################
# print_table()
#
# print a <TABLE ...>
#

function print_table($cellpadding,$cellspacing,$border,$bgcolor,$width,$align) {
    echo make_table($cellpadding,$cellspacing,$border,$bgcolor,$width,$align);
}

#?php#################################################################
# mkdirs()
#
# *NIX like mkdir
#

function mkdirs($mode, $path)
{
  exec("mkdir -m $mode -p $path");
}

#?php#################################################################
# make_form_select()
#
#
#

function make_form_select($caption, $select_name, $r_selects, $selected="", $submit_onchange=0, $dimension=0) {

    $this_select = "${caption}";
//    $this_select .= "<select name=\"$select_name\" class=\"smallWhite\"";
    $this_select .= "<select name=\"$select_name\"";
    if ($submit_onchange != "") {
        $this_select .= " onchange=\"this.form.submit()\"";
    }
    $this_select .= ">\n";
    if (is_array($r_selects)) {
        //reset($r_selects);  // foreach does perform the reset
        foreach ($r_selects as $select_item => $select_value) {
            $select_item = ($dimension == 1 ? $select_item : $select_value);
            if ($select_item == $selected) {
                $this_select .= "  <option value=\"$select_item\" selected>${select_value}\n";
            } else {
                $this_select .= "  <option value=\"$select_item\">${select_value}\n";
            }
        }
    } else {
        display_errors(array("\nERROR - \$r_selects is not an array\nfunction make_form_select(\$caption, \$select_name, &\$r_selects, \$selected, \$submit_onchange)\n"));
        return null;
    }
    $this_select .= '</select>';

    return $this_select;
}

#?php#################################################################
# print_form_select()
#
#
#

function print_form_select($caption, $select_name, $r_selects, $selected="", $submit_onchange="") {
    print make_form_select($caption, $select_name, $r_selects, $selected, $submit_onchange);
}

#?php#################################################################
# make_form_start()
#
#
#

function make_form_start($action, $r_hidden=false, $method="post", $extras=false) {
    $this_form = "<form method=\"$method\" action=\"$action\"";
    $this_form .= $extras ? ' '.$extras : '';
    $this_form .= ">\n";
    if (is_array($r_hidden)) {
        reset($r_hidden);
        while (list($key,$value) = each($r_hidden)) {
            $this_form .= "  <input type=\"hidden\" name=\"$key\" value=\"$value\">\n";
        }
    } elseif ($r_hidden != false) {
        display_errors(array("\nERROR - \$r_hidden is not an array\nfunction make_form_start(\$action, \$r_hidden=false, \$method=\"post\")\n"));
        return null;
    }
    return $this_form;
}

#?php#################################################################
# print_form_start()
#
# print a <FORM ...>
#

function print_form_start($action, $r_hidden=false, $method="post") {
    print make_form_start($action, $r_hidden, $method);
}

#?php#################################################################
# make_form_end()
#
#
#

function make_form_end() {
    $this_form = '</form>';
    return $this_form;
}

#?php#################################################################
# print_form_end()
#
# print a </FORM>
#

function print_form_end() {
    echo make_form_end();
}

$enclosed = 0;

#?php#################################################################
# commonHeader()
#
#
#

function commonHeader($title="",$dont_enclose=0,$headers_gone=0) {
    global $MYSITE, $URLBASE, $enclosed, $prevsearch, $PHP_SELF;
    global $SIDEBAR_DATA, $CONTEXT_DROPDOWN, $build;

header("Cache-Control: public, max-age=600");
if (!$headers_gone):
?>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<html>
<head>
  <title>visualization<?php if ($title) echo ": $title";?></title>
  <link rel="stylesheet" type="text/css" href="<?php echo "${URLBASE}";?>/styles/style.css" title="Default" />
  <link rel="shortcut icon" href="<?php echo "${URLBASE}";?>/gifs/favicon.ico" />
  <link rel="icon" href="<?php echo "${URLBASE}";?>/gifs/favicon.png" />
  <script language="JavaScript" type="text/javascript">
    if (window != top) top.location.href = location.href;
    self.name = "root"; // names current window as "root"
  </script>
<!--  <script language="JavaScript" type="text/javascript" src="<?php echo ${URLBASE}; ?>/jscripts/infobox.js"></SCRIPT> -->
  <script language="JavaScript" type="text/javascript" src="<?php echo ${URLBASE}; ?>/jscripts/tips.js"></SCRIPT> <!-- MEFIX -->
  <script language="JavaScript" type="text/javascript">
    function jumptolink(what){
      var selectedopt=what.options[what.selectedIndex]
      if (document.getElementById && selectedopt.getAttribute('target')=='newwin')
        window.open(selectedopt.value);
      else
        window.location=selectedopt.value;
    }
  </script>
</head>
<body>
<div id="infodiv" style="position:absolute; visibility:hidden; z-index:20; top:0px; left:0px;"></div>
<?php endif; ?>
<?php echo "\n<!--$MYSITE-->\n"; ?>

<table border="0" cellspacing="0" cellpadding="0" width="100%">
<tr bgcolor="#9999cc" height=24>
  <td align="left" valign="middle" colspan="3">
    <div class="visTitleShadow"><?php echo spacer(23,18); echo $title; ?>
      <div class="visTitle"><?php print_link("${URLBASE}/", make_image('website.png', 'Home', 'bottom')); echo $title; ?></div>
    </div>
  </td>
  <td align='right' valign='middle' colspan=2>
      <table border="0" cellspacing="0" cellpadding="0" width="100%">
          <tr bgcolor="#9999cc" height=24>
              <form name="toolpage">
              <td align='right' valign='middle'>
                  <?php //print_link("${URLBASE}/",'home',false,'class="smallWhite"'); ?>
                  <select name="quicklink" class="tbox" onChange="jumptolink(document.toolpage.quicklink)">
                      <option selected value="<?php print("${URLBASE}"); ?>">quick links to ...</option>
                      <option value="<?php print("${URLBASE}"); ?>">Visualization home</option>
                      <option value="<?php print("${URLBASE}/status.php"); ?>">Latest build&apos;s status</option>
                      <option value='http://hq-nfuse1.ptcnet.ptc.com/citrix/metaframexp/default/frameset.asp' target='newwin'>CaRDS login</option>
                      <option value='http://internal.ptc.com/' target='newwin'>PTConnector</option>
                      <option value="javascript:q=document.getSelection();if(!q)void(q=prompt('Address Book:',''));if(q)location.href='http://internal.ptc.com/cgi/it/addressbook.pl?cn='+escape(q)" target='newwin'>PTC address book</option>
                      <option value='/home/phonelist.htm' target='newwin'>Bristol phone list</option>
                      <option value="<?php echo("${URLBASE}/tools/todolist/"); ?>" target='newwin'>Integration&apos;s todolist</option>
                  </select><?php
                    //print_submit('small_submit.gif', 'go', 'center');
                  ?>
              </td>
              </form>
              <form name='SPR' action='http://rdweb.ptc.com/WebSiebel/report.php' method='get' target='_blank'>
              <td align='right' valign='middle' width='100'>
                  <small><font color="#ffffff"><span title="Keyboard shortcut: Alt+S"><u>S</u>PR:<input type="hidden" name="mode" value="spr_details"><input type="hidden" name="form" value="spr"><input type="hidden" name="do_not_check_ver" value=1><input class="tbox" type="text" size=10 name="spr_no" accesskey="s"></span></font></small>
              </td>
              </form>
          </tr>
      </table>
  </td>
</tr>
<!--
<tr bgcolor="#9999cc" height=12>
  <td align="left" valign="bottom" colspan="2" nowrap>
    <?php
//      print_link("${URLBASE}/", make_image('favicon.png', 'back there...', 'left') );
        echo "&nbsp;";
        print_link("${URLBASE}/",'Visualization home',false,'class="smallWhite"');
//        echo delim();
//        print_link('http://ah-bsp.ptcnet.ptc.com/', 'wnc builds', 'nouvelle', 'class="smallWhite"');
    ?>
  </td>
  <td align="right" valign="bottom" colspan="3" nowrap>
    <?php
      //print_link("${URLBASE}/downloads.php",'downloads',false,'class="smallWhite"');
      //echo delim();
      //if (builds_domain() != "needham.ptc.com") {
      //  print_link("${URLBASE}/status.php",'PV builds',false,'class="smallWhite"');
      //  echo delim();
      //}
      //print_link("${URLBASE}/QA.php", 'QA', false, 'class="smallWhite"');
      //echo delim();
      //print_link ("${URLBASE}/tools/", 'more...', false, 'class="smallWhite"');
    ?>
    &nbsp;
  </td>
</tr>
-->
<tr bgcolor="#333366" height=1>
  <td colspan="5"><?php spacer(1,1);?></td>
</tr>
<tr bgcolor="#666699" height=20>
  <?php
  if (isset($CONTEXT_DROPDOWN)) {
    echo $CONTEXT_DROPDOWN;
  } else{
  ?>
  <td align="right" valign="top" colspan="5">
    <font color="#ffffff"><small>&nbsp;</small></font>
  </td>
  <?php } ?>
</tr>
<tr bgcolor="#333366" height=1>
  <td colspan="5"><?php spacer(1,1);?></td>
</tr>
</table>

<?php
  if (!$dont_enclose):
    $enclosed = 1;
?>

<table border="0" cellpadding="0" cellspacing="0" height="75%" width="100%">
<tr valign="top">
<?php if (isset($SIDEBAR_DATA)):?>
  <td width="180" bgcolor="#f0f0f0">
   <table width="100%" cellpadding="2" cellspacing="0" border="0">
    <tr valign="top"><td class="sidebar"><?php echo $SIDEBAR_DATA?></td></tr>
   </table>
  </td>
  <td bgcolor="#cccccc" background="<?php echo $URLBASE ?>/gifs/checkerboard.gif" width="1"><?php spacer(1,1); ?></td>
<?php endif; // SIDEBAR_DATA ?>
  <td>
   <table cellpadding="10" cellspacing="0" width="100%">
     <tr><td valign="top">
<!-- start content -->
<?php
  endif; // dont_enclose
}


#?php#################################################################
# commonFooter()
#
#
#

function commonFooter() {
    global $enclosed, $RSIDEBAR_DATA;
    global $SCRIPT_NAME, $MYSITE, $URLBASE, $LAST_UPDATED, $PHP_SELF;

    if ($enclosed) {
//        spacer(620);
        echo "</td></tr></table>";
        echo "</td>\n";

        if(isset($RSIDEBAR_DATA)) {
            echo '  <td bgcolor="#cccccc" width="1"' . spacer(1,1) . '</td>';
            echo '  <td width="175" bgcolor="#f0f0f0">';
            echo '    <table width="100%" cellpadding="4" cellspacing="0">';
            echo '      <tr valign="top"><td class="sidebar">' . $RSIDEBAR_DATA . '</td></tr>';
            echo '    </table>';
            spacer(175);
            echo '  </td>';
        }

        echo "</tr></table>\n";

    }
  ?>

<table border="0" cellspacing="0" cellpadding="0" width="100%">
  <tr bgcolor="#333366" height=1><td colspan=5><?php spacer(1,1);?></td></tr>
  <tr bgcolor="#9999cc" height=12>
    <form name="mirrors" method="get" action="mirrors.php" onsubmit="return gotomirror(this);" style="margin-bottom:0;">
    <td align="right" valign="bottom" nowrap colspan=5>

      <script language="javascript">
      <!--
      function gotomirror(form) {
        mirror = form.country.options[form.country.selectedIndex].value;
        if (mirror != 'http://' + window.location.hostname + '/') {
                 window.location.href = mirror +
                 window.location.pathname.substring(1) +
                 window.location.hash +
                 window.location.search;
        }
        return false;
      }
      //-->
      </script>

      <input type="hidden" name="FROM" value="<?php echo htmlspecialchars($PHP_SELF)?>">
      <?php
      print_link('mailto:bsabin@ptc.com','page bugs',false,'class="smallWhite"');
      echo delim();
      print_link("${URLBASE}/source.php?url=".$SCRIPT_NAME, 'show source', false, 'class="smallWhite"');
      echo delim();
      if (have_stats()) {
        print_link("${URLBASE}/stats/", 'stats', false, 'class="smallWhite"');
        echo delim();
      }
      echo "<select class=\"tbox\" name=\"country\" onchange=\"gotomirror(this.form)\">\n";
      show_mirror_options($MYSITE, 1);
      echo "</select>";
      echo make_submit('small_submit.gif', 'go', 'bottom' ) . '&nbsp;';
      //echo '<p class="smallWhite">' .  mirror_provider() . '&nbsp;</p>' ;
      ?>
    </td>
    </form>
  </tr>
  <tr bgcolor="#333366" height=1><td colspan=5><?php spacer(1,1); ?></td></tr>

  <tr valign="top" bgcolor="#cccccc" height=8>
    <td align="left" colspan=3><small class="copy">
      <?php //echo make_link("${URLBASE}/copyright.php", 'Copyright &copy; 2001, 2002 The PHP Group'); ?> <?php //echo "<br />All rights reserved."; ?>
      &nbsp;.::jahtooth
    </small></td>
    <td align="right" colspan=2><small class="copy">
    <!-- This mirror generously provided by: <a href="<?php echo mirror_provider_url();?>"><?php echo mirror_provider();?></a><br /> -->
    (your IP: <?php echo getenv("REMOTE_ADDR");?>)&nbsp;&nbsp;
    Last updated: <url-minder-ignore><?php echo strftime("%c &nbsp;", $LAST_UPDATED); ?></url-minder-ignore>
    </small></td>
  </tr>
</table>

</body>
</html>
<?php
}

#?php#################################################################
# setAutoRefresh()
#
#
#

function setAutoRefresh($autorefresh) {
    if ($autorefresh) {
        return '
        <script language="JavaScript">
        <!--
        // default to 1 min
        setTimeout(\'location.reload()\',60000);
        -->
        </script>
        <a name="EOP"></a>
        ';
    }
}

#?php#################################################################
# sect_to_file()
#
#
#

function sect_to_file($string) {
    $string = strtolower($string);
    $string = str_replace(' ','-',$string);
    $string = str_replace('_','-',$string);
    $func = "function.$string.php";
    $chap = "ref.$string.php";
    $feat = "features.$string.php";
    $struct = "control-structures.$string.php";
    if(@is_file($func)) return $func;
    else if(@is_file($chap)) return $chap;
    else if(@is_file($feat)) return $feat;
    else if(@is_file($struct)) return $struct;
    else return "$string.php";
}

#?php#################################################################
# clean()
#
#
#

function clean($var) {
  return htmlspecialchars(get_magic_quotes_gpc() ? stripslashes($var) : $var);
}

#?php#################################################################
# clean_note()
#
#
#

function clean_note($text) {
    $text = htmlspecialchars(trim($text));

    /* turn urls into links */
    $text = preg_replace("/((mailto|http|ftp|nntp|news):.+?)(&gt;|\\s|\\)|\"|\\.\\s|$)/","<a href=\"\\1\">\\1</a>\\3",$text);

    /* this 'fixing' code will go away eventually. */
    $fixes = array('<br>','<p>','</p>');
    reset($fixes);
    while (list(,$f)=each($fixes)) {
        $text=str_replace(htmlspecialchars($f), $f, $text);
        $text=str_replace(htmlspecialchars(strtoupper($f)), $f, $text);
    }

    /* this will only break long lines */
    if (function_exists("wordwrap")) {
        $text = wordwrap($text);
    }

    $text = "<pre class=\"note\">".$text."</pre>";
    return $text;
}

#?php#################################################################
# make_errors()
#
#
#

function make_errors($errors)
{
    $return = '<div class="errors">';
    if (count($errors) > 1) {
        $return .=  "You need to do the following before your submission will be accepted:<ul>";
        foreach ($errors as $error) {
            $return .=  "<li>$error</li>\n";
        }
        $return .=  "</ul>";
    }
    else {
        $return .=  $errors[0];
    }
    $return .=  '</div>';
    return $return;
}

#?php#################################################################
# display_errors()
#
#
#

function display_errors($errors)
{
    print make_errors($errors);
}

#?php#################################################################
# display_event()
#
#
#

function display_event($event,$include_date=1)
{
  for ($i = 1; $i <= 7; $i++) {
    $days[$i] = strftime('%A',mktime(12,0,0,4,$i,2001));
  }

  $re = array(1=>'First',2=>'Second',3=>'Third',4=>'Fourth',-1=>'Last',-2=>'2nd Last',-3=>'3rd Last');

  $sday = $event['start'] ? strtotime($event['start'])
          : mktime(0,0,1,$event['smonth'],$event['sday'],$event['syear']);
  $eday = $event['end'] ? strtotime($event['end'])
          : mktime(0,0,1,$event['emonth'],$event['eday'],$event['eyear']);
?>
<table border="0" cellspacing="0" cellpadding="3" width="100%">
<tr bgcolor="#dddddd"><td>
<?php
  if ($include_date && ($event['start'] || $event['smonth'])) echo "<b>", date("F j, Y", $sday), "</b>\n";
  if ($event['url']) echo '<a href="', htmlentities($event['url']),'">';
  echo "<b>", htmlentities($event['sdesc']), "</b>";
  if ($event['url']) echo "</a>";
  switch ($event['type']) {
  case 2:
  case 'multi':
    echo " (", date("Y-m-d",$sday), " to ", date("Y-m-d",$eday), ")";
    break;
  case 3:
  case 'recur':
    echo " (Every ", $re[$event['recur']], " ", $days[$event['recur_day']], " of the month)";
    break;
  }
?>
 </td></tr>
<tr bgcolor="#eeeeee"><td><?php echo preg_replace("/\r?\n\r?\n/","<br /><br />",trim($event['ldesc'])); if ($event['url']) {?><br /><br /><b>URL:</b> <?php echo '<a href="',htmlentities($event['url']),'">',htmlentities($event['url']),'</a>'; }?></td></tr>
</table>
<?php
}

#
#  this is a snippet which parses a CSV file
#
# $fp = @fopen("backend/events.csv",'r');
# if($fp) {
#     $cm=0;
#     while(!feof($fp)) {
#         list($d,$m,$y,$url,$desc,$id) = fgetcsv($fp,8192);
#         if($cm!=(int)$m) {
#             if($cm) $RSIDEBAR_DATA.= "<br />\n";
#             else $RSIDEBAR_DATA.='<h3>Upcoming Events<br />&nbsp;&nbsp;&nbsp;&nbsp;<a href="submit-event.php">[add event]</a></h3>';
#             $cm = (int)$m;
#             $RSIDEBAR_DATA .= "<h4>".strftime('%B',mktime(12,0,0,$cm,$d,$y))."</h4>\n";
#             unset($seen);
#         }
#         if(!$seen[$desc]) {
#             $RSIDEBAR_DATA .= "$d. <a href=\"cal.php?id=$id\">$desc</a><br>\n";
#             $seen[$desc] = true;
#         }
#     }
#     fclose($fp);
# }
?>
