@echo off
rem JASSPA MicroEmacs - www.jasspa.com
rem build - JASSPA MicroEmacs build script for MS windows and dos platforms
rem Copyright (C) 2001-2024 JASSPA (www.jasspa.com)
rem See the file main.c for copying and conditions.
set BITSIZE=
set LOGFILE=
set LOGFILEA=
set MECORE=
set MEDEBUG=
set MELSTT=
set MEPROF=
set METYPE=
set MAKEFILE=
set OPTIONS=
:build_option
if "%1." == "."    goto build_cont
if "%1" == "-32"   set  BITSIZE= BIT_SIZE=32
if "%1" == "-64"   set  BITSIZE= BIT_SIZE=64
if "%1" == "-C"    set  OPTIONS= clean
if "%1" == "-d"    set  MEDEBUG= BCFG=debug
if "%1" == "-h"    goto build_help
if "%1" == "-l"    goto build_logf
if "%1" == "-la"   goto build_logfa
if "%1" == "-LS"   set  MELSTT= LSTT=1
if "%1" == "-m"    goto build_mkfl
if "%1" == "-ne"   set  MECORE= BCOR=ne
if "%1" == "-P"    set  MEPROF= BPRF=1
if "%1" == "-S"    set  OPTIONS= spotless
if "%1" == "-t"    goto build_type
shift
goto build_option

:build_logf
shift
set LOGFILE=%1
shift
goto build_option

:build_logfa
shift
set LOGFILEA=%1
shift
goto build_option

:build_mkfl
shift
set MAKEFILE=%1
shift
goto build_option

:build_type
shift
set METYPE= BTYP=%1
shift
goto build_option

:build_cont

set OPTIONS=%MECORE%%MEDEBUG%%METYPE%%MEPROF%%MELSTT%%BITSIZE%%OPTIONS%

if NOT "%MAKEFILE%." == "." goto build_got_makefile
if NOT "%VisualStudioVersion%." == "." set MAKEFILE=winmsvc.mak & goto build_got_makefile
if NOT "%VCINSTALLDIR%." == "." set MAKEFILE=winmsvc.mak & goto build_got_makefile
if NOT "%PATH:\mingw=%." == "%PATH%." set MAKEFILE=winmingw.mak & goto build_got_makefile

echo.
echo ERROR: Failed to identify compiler, use -m to specify the required makefile.
echo.
echo Run 'build -h' for more information.
echo.
goto :build_exit

:build_got_makefile

set MAKE=make
if "%MAKEFILE:~0,7%" == "winmsvc" set MAKE=nmake
if "%MAKEFILE:~0,8%" == "winmingw" set MAKE=mingw32-make

if "%LOGFILE%." == "." goto build_applog

echo %MAKE% -f %MAKEFILE%%OPTIONS% > %LOGFILE% 2>&1
%MAKE% -f %MAKEFILE%%OPTIONS% >> %LOGFILE% 2>&1

goto build_exit

:build_applog

if "%LOGFILEA%." == "." goto build_nolog

echo. >> %LOGFILEA% 2>&1
echo %MAKE% -f %MAKEFILE%%OPTIONS% >> %LOGFILEA% 2>&1
%MAKE% -f %MAKEFILE%%OPTIONS% >> %LOGFILEA% 2>&1

goto build_exit

:build_nolog

echo %MAKE% -f %MAKEFILE%%OPTIONS%
%MAKE% -f %MAKEFILE%%OPTIONS%

goto build_exit

:build_help

echo Usage: build [options]
echo.
echo Where options can be:-
echo     -32  : Build 32bit binary.
echo     -64  : Build 64bit binary.
echo     -C   : Build clean.
echo     -d   : For debug build (output is med* or ned*)
echo     -h   : For this help page
echo     -l {logfile}
echo          : Set the compile log file
echo     -la {logfile}
echo          : Append the compile log to the given file
echo     -LS  : Link with static libraries to reduce dynamic runtime dependents (MSVC)
echo     -m {makefile}
echo            Sets the makefile to use where {makefile} can be:-
echo              winmingw.mak  Windows 32bit build using MinGW GNU GCC
echo              winmsvc.mak   Windows build using MS VC version 8+
echo     -ne  : for NanoEmacs build (output is ne).
echo     -P   : Build with profiling instructions (MinGW)
echo     -S   : Build clean spotless.
echo     -t {type}
echo          : Sets build type:
echo               c   Console support only
echo               w   Wondow support only (default)
echo               cw  Console and window support
echo.

:build_exit
