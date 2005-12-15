<?php

// things to work in debug mode:
// change the $URLBASE value below to your home dir
// add/comment out your web server in the $MIRRORS array below

$URLBASE = '/~bsabin';

// trick to override the php include location
//    $document_dir = getcwd();
//    preg_match("/^\/([^\/]+)[\/]*/", $PHP_SELF, $match);
//    $obj = apache_lookup_uri("/$match[1]");
//    chdir($obj->filename);
//    require_once 'include/prepend.inc';
//    chdir($document_dir);

/*
  Structure of MIRRORS array:
  0    "country code",
  1    "Mirror Name",
  2    flag for whether local stats work (1) or not (0) on this mirror
  3    "url for hosting company",
  4    flag for whether site is a full mirror (1) or just a download site (0), or just a placeholder (2),
  5    flag for whether search engine works (1) or not (0) on the site
  6    default language code
  7    newbuild home directory
  8    builds base directory
*/

$MIRRORS = array(
"http://vast.ptcnet.ptc.com/" => array("GBR","Bristol - vast",0,"http://vast.ptcnet.ptc.com/",1,0,"en","/user/newbuild","/builds/pview","division.co.uk"),
"http://zion.ptcnet.ptc.com/" => array("USA","San Diego - zion",0,"http://zion.ptcnet.ptc.com/",1,0,"en","/home/newbuild","/builds/pview","sd.ptc.com"),
"http://orange.ptcnet.ptc.com/" => array("USA","Needham - orange",0,"http://orange.ptcnet.ptc.com/",1,0,"en","/user/newbuild","/builds/pview","needham.ptc.com"),
"http://roots.ptcnet.ptc.com/" => array("USA","San Diego Debug - roots",0,"http://roots.ptcnet.ptc.com/",1,0,"en","/home/newbuild","/builds/pview","sd.ptc.com"),
0 => array("xx", "Unknown", 0, "/", 2, 0, "en")
);

$COUNTRIES = array(
'GBR'           => 'United Kingdom',
'USA'           => 'United States',
);

$BUILD_TYPES = array(
'opt'           => 'optimized (pvk)',
'opt_pview'     => 'optimized (pview)',
'opt_test'      => 'smoke tests',
'deb'           => 'debug (pvk)',
'deb_pview'     => 'debug (pview)',
'deb_test'      => 'smoke test',
);

$PLATFORMS = array(
'aix4'          => 'AIX 4.x',
'hpux10'        => 'HP-UX 10.20',
'hpux11'        => 'HP-UX 11.x',
'hpux11_64_64'  => 'HP-UX 11.x 64bit',
'irix6'         => 'IRIX 6.x',
'irix6_32_o32'  => 'IRIX 6.x o32',
'irix6_64_64'   => 'IRIX 6.x 64bit',
'osx10'         => 'Mac OSX 10.x',
'sunos55'       => 'SunOS 5.x',
'sunos55_64_64' => 'SunOS 5.x 64bit',
'win32'         => 'win32',
'win32_s'       => 'win32 static',
'win32_p'       => 'win32 pview',
);

$QA_SITE = array(
'welcome'       => 'Welcome Page',
'temp'          => 'Templates',
'archives'      => 'Test Data Archives',
'upload'        => 'CAD Data Upload',
'worker'        => 'Worker Config.',
);

$FTP_FILES = array(
'Full CD images'            => 'cdimages/00_INDEX',
'individual DSU archives'   => 'client/00_INDEX',
'ProductView client [win32]'=> 'productview/ProductView_en.exe',
'Windchill client [win32]'  => 'windchill/ProductView_en.exe',
'Windchill Graphics Server' => 'windchill/WindchillGS.exe',
'DIVISION client [win32]'   => 'division/ProductView_en.exe',
'DIVISION Graphics Server'  => 'division/GraphicsServer.exe',
'DIVISION Graphics Server 7.x'          => 'division/GS.exe',
'Client Add-Ons'            => 'common/00_INDEX',
'ProjectLink Vizualization files'       => 'projectlink/00_INDEX',
'DDL development files'     => 'DDL/00_INDEX',
'dAPI ToolKit archives'     => 'adapters/00_INDEX',
'Misc. development files'   => 'development/pview.hm',
'Localization files'        => 'localization/localization_pview.zip',
);

$ADMIN_LOGS = array(
'overnight_check.lgg'                   => 'CVS locks check',
'overnight_checkout.lgg'                => 'build environment setup',
'overnight_mirroring.lgg'               => 'CVS mirrorring',
'cleanup.lgg'                           => 'files backup/removal',
'overnight_gs_compile_division.lgg'     => 'DGS compiletool',
'overnight_gs_compile_windchill.lgg'    => 'WGS compiletool',
'overnight_gs_compile_modelserver.lgg'  => 'ModelServer compiletool',
'overnight_gs_compile_wvsECADworker.lgg'=> 'WVS ECAD worker compiletool',
'overnight_gs_build_division.lgg'       => 'DGS buildproduct',
'overnight_gs_build_windchill.lgg'      => 'WGS buildproduct',
'overnight_gs_build_modelserver.lgg'    => 'ModelServer buildproduct',
'overnight_gs_build_wvsECADworker.lgg'  => 'WVS ECAD worker buildproduct',
'overnight_gs_build_remoteEDR.lgg'      => 'RemoteEDR buildproduct',
'overnight_win32_packaging.lgg'         => 'win32 packaging',
);

# http://www.unicode.org/unicode/onlinedat/languages.html
$LANGUAGES = array(
'en' => 'English',
'fr' => 'French',
'ja' => 'Japanese',
);

$DB = array(
'dbhost'  => 'vast.ptcnet.ptc.com',
'dbuser'  => 'root',
'dbpwd'   => '',
'dbname'  => 'VIS',
'dbparse' => 'log_parsing',
);

$MYHOST = getenv("SERVER_NAME");
$MYSITE = 'http://' . $MYHOST . '/';
if (!isset($MIRRORS[$MYSITE])) {
    $MYSITE='http://' . ereg_replace("^www\\.","",$HTTP_HOST) . '/';
}
if (!isset($MIRRORS[$MYSITE])) {
    $MIRRORS[$MYSITE] = array("xx", $MYSITE, "none", $MYSITE, 2, 0, "en");
}

function is_primary_site() {
  global $MYSITE;
  return $MYSITE == "http://vast.ptcnet.ptc.com/";
}

function is_backup_primary() {
  global $MYSITE;
  return $MYSITE == "http://zion.ptcnet.ptc.com/";
}

function mirror_provider() {
  global $MIRRORS,$MYSITE;
  return $MIRRORS[$MYSITE][1];
}

function have_stats() {
  global $MIRRORS,$MYSITE;
  return $MIRRORS[$MYSITE][2];
}

function mirror_provider_url() {
  global $MIRRORS,$MYSITE;
  return $MIRRORS[$MYSITE][3];
}

function have_search() {
  global $MIRRORS,$MYSITE;
  return $MIRRORS[$MYSITE][5];
}

function default_language() {
  global $MIRRORS,$MYSITE;
  return $MIRRORS[$MYSITE][6];
}

function unix_home() {
  global $MIRRORS,$MYSITE;
  return $MIRRORS[$MYSITE][7];
}

function builds_base() {
  global $MIRRORS,$MYSITE;
  return $MIRRORS[$MYSITE][8];
}

function builds_domain() {
  global $MIRRORS,$MYSITE;
  return $MIRRORS[$MYSITE][9];
}

function show_mirror_options ($current, $short=false) {
  global $MIRRORS, $COUNTRIES;
  foreach ($MIRRORS as $url=>$mirror) {
    if ($mirror[4] == 1) { /* only list full mirrors here */
      $countries = $short ? '' : $COUNTRIES[$mirror[0]];
      if ($url==$current) {
        echo '<option value="' . $url . '" selected>' . $countries .
          ' (' . $mirror[1] . ") *</option>\n";
      } else {
        echo '<option value="' . $url . '">' . $countries .
          ' (' . $mirror[1] . ")</option>\n";
      }
    }
  }
}

function myAddSlashes($st) {
    if (get_magic_quotes_gpc()) {
        return $st;
    } else {
        return addslashes($st);
    }
}

function myStripSlashes($st) {
    if (get_magic_quotes_gpc()) {
        return stripslashes($st);
    } else {
        return $st;
    }
}

?>
