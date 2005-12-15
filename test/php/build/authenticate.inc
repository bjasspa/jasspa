<?php

function auth() {
    global $PHP_SELF, $sent, $mysession, $login, $passwd, $bye, $DB;
    $login_ok = 0;                              // access denied by default

    if ($bye) {                                 // user requested logout
        session_start();
        session_unregister("mysession");
        session_destroy();
        return 0;
    }

    mysql_connect($DB['dbhost'],$DB['dbuser'],$DB['dbpwd'])
        or die("<br>authenticate(): Unable to connect to database:<br>\n".mysql_error()."<br>\n");
    mysql_select_db($DB['dbname'])
        or die("<br>authenticate(): Unable to select database:<br>\n".mysql_error()."<br>\n");
    $sql = "SELECT login, passwd FROM users";
    $res = mysql_query($sql) or die("Login query failed");

    if(isset($sent)) {                          // arrive from login form
        if (isset($login) and isset($passwd)) {
            while ( $row = mysql_fetch_array($res, MYSQL_ASSOC) ) {
                if ( $row["login"] == $login && $row["passwd"] == $passwd ) {
                    session_start();            // authentication succeeded
                    $login_ok = 1;
                    // create the session array
                    $mysession = array ("login" => $login, "passwd" => $passwd, "ID" => session_id(), "loginForm"=>FALSE, "valid" => 1);
                    session_register("mysession");
                    break;
                }
            }
         }
    } else {                                    // arrive from session var
        session_start();
        foreach($mysession as $elem) {          // retrieve session array
              $ses_tmp[] = $elem;
        }
        $login = $ses_tmp[0];
        $passwd = $ses_tmp[1];
        while ( $row = mysql_fetch_array($res, MYSQL_ASSOC) ) {
            if ( $row["login"] == $ses_tmp[0] && $row["passwd"] == $ses_tmp[1] ) {
                session_start();
                $login_ok = 1;                  // authentication succeeded
                // create the session array
                $mysession = array ("login" => $login, "passwd" => $passwd, "ID" => session_id(), "loginForm"=>FALSE, "valid" => 1);
                session_register("mysession");
                break;
            }
        }
    }
    return $login_ok;
}

// --------------------------------------------------------------------------------------
// main
// --------------------------------------------------------------------------------------

//init vars;
$mysession = array ("login"=>FALSE,
                    "passwd"=>FALSE,
                    "ID"=>FALSE,
                    "loginForm"=>FALSE,
                    "valid"=>FALSE);
$uri = basename($PHP_SELF);
$stamp = md5(srand(5));

if(!auth()) {                           // authentication failed
    $mysession["loginForm"] = '
    <form method="post" action="'.$uri.'" name=loginform>
      login: <input class="tbox" type="text" name="login" maxlength=50 size=15 />
      password: <input class="tbox" type="password" name="passwd" maxlength=50 size=15 />
      <input type=submit class="button" value="login" name="sent" />
    </form>
    <script type="text/javascript"> <!--
    if (document.loginform) {
       document.loginform.login.focus();
    }
    // --> </script>
';
} else {                                // authentication was successful
    $tmp = session_id();                // session is already started
    if (isset($sent)) {                 // logged in for first time
        $mysession['loginForm'] = "
        logged in as ".$mysession['login']."&nbsp;&nbsp;&nbsp;
        [<a class='smallWhite' href='$uri?SID=$tmp'>load</a>]&nbsp;
        [<a class='smallWhite' href='$uri?bye=true'>logout</a>]&nbsp;
        ";
    } else {                            // logged in via session var
        $mysession['loginForm'] = "
        logged in as ".$mysession['login']."&nbsp;&nbsp;&nbsp;
        [<a class='smallWhite' href='$uri?SID=$tmp&stamp=$stamp'>load</a>]&nbsp;
        [<a class='smallWhite' href='$uri?bye=true'>logout</a>]&nbsp;
        ";
    }
}

?>
