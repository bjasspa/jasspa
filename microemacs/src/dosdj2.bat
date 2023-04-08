@echo off
set OPTIONS=
set LOGFILE=
set MECORE=
set MEDEBUG=
:build_option
if "%1." == "."    goto build_cont
if "%1" == "-C"    set  OPTIONS= clean
if "%1" == "-d"    set  MEDEBUG= BCFG=debug
if "%1" == "-h"    goto build_help
if "%1" == "-l"    goto build_logf
if "%1" == "-la"   goto build_logfa
if "%1" == "-ne"   set  MECORE= BCOR=ne
if "%1" == "-S"    set  OPTIONS= spotless
shift
goto build_option

:build_logf
shift
set LOGFILE=redir -o %1 -eo 
shift
goto build_option

:build_logfa
shift
set LOGFILE=redir -oa %1 -eo 
shift
goto build_option

:build_cont

set OPTIONS=%MECORE%%MEDEBUG%%OPTIONS%

echo %LOGFILE% make -f dosdj2.mak %OPTIONS%
%LOGFILE% make -f dosdj2.mak %OPTIONS%

goto build_exit

:build_help

echo Usage: build [options]
echo.
echo Where options can be:-
echo     -C   : Build clean.
echo     -d   : For debug build (output is med* or ned*)
echo     -h   : For this help page
echo     -l {logfile}
echo          : Set the compile log file
echo     -la {logfile}
echo          : Append the compile log to the given file
echo     -ne  : for NanoEmacs build (output is ne).
echo     -S   : Build clean spotless.
echo.

:build_exit

