@echo off
rem JASSPA MicroEmacs - www.jasspa.com
rem build - JASSPA MicroEmacs build script for MS windows and dos platforms
rem Copyright (C) 2001-2009 JASSPA (www.jasspa.com)
rem See the file main.c for copying and conditions.
set OPTIONS=
set LOGFILE=
set LOGFILEA=
set MECORE=
set MEDEBUG=
set MELSTT=
set MEPROF=
set METYPE=
set MAKEFILE=
:build_option
if "%1." == "."    goto build_cont
if "%1" == "-C"    set  OPTIONS= clean
if "%1" == "-d"    set  MEDEBUG= BCFG=debug
if "%1" == "-h"    goto build_help
if "%1" == "-l"    goto build_logf
if "%1" == "-la"   goto build_logfa
if "%1" == "-LS"   set  MELSTT= LSTT=1
if "%1" == "-m"    goto build_mkfl
if "%1" == "-ne"   set  MECORE= BCOR=ne
if "%1" == "-p"    set  MEPROF= BPRF=1
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

set OPTIONS=%MECORE%%MEDEBUG%%METYPE%%MEPROF%%MELSTT%%OPTIONS%

if NOT "%MAKEFILE%." == "." goto build_got_makefile


if NOT "%PATH:Microsoft Visual Studio\2019=%." == "%PATH%." set MAKEFILE=win32vc16.mak & goto build_got_makefile
if NOT "%PATH:Microsoft Visual Studio\2017=%." == "%PATH%." set MAKEFILE=win32vc15.mak & goto build_got_makefile
if NOT "%PATH:Microsoft Visual Studio 10.0=%." == "%PATH%." set MAKEFILE=win32vc10.mak & goto build_got_makefile
if NOT "%PATH:Microsoft Visual Studio 9.0=%." == "%PATH%." set MAKEFILE=win32vc9.mak & goto build_got_makefile
if NOT "%PATH:Microsoft Visual Studio 8.0=%." == "%PATH%." set MAKEFILE=win32vc8.mak & goto build_got_makefile
if NOT "%PATH:\mingw=%." == "%PATH%." set MAKEFILE=win32mingw.mak & goto build_got_makefile
if NOT "%VS160COMNTOOLS%." == "." set MAKEFILE=win32vc16.mak & goto build_got_makefile
if NOT "%VS150COMNTOOLS%." == "." set MAKEFILE=win32vc15.mak & goto build_got_makefile
if NOT "%VS100COMNTOOLS%." == "." set MAKEFILE=win32vc10.mak & goto build_got_makefile
if NOT "%VS90COMNTOOLS%." == "." set MAKEFILE=win32vc9.mak & goto build_got_makefile
if NOT "%VS80COMNTOOLS%." == "." set MAKEFILE=win32vc8.mak & goto build_got_makefile
if NOT "%MSVCDir%." == "." set MAKEFILE=win32vc6.mak & goto build_got_makefile

echo.
echo ERROR: Failed to identify compiler, use -m to specify the required makefile.
echo.
echo Run 'build -h' for more information.
echo.
goto :build_exit

:build_got_makefile

set MAKE=make
if "%MAKEFILE:~0,10%" == "win32mingw" set MAKE=mingw32-make
if "%MAKEFILE:~0,7%" == "win32vc" set MAKE=nmake

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
echo              win32mingw.mak  Win32 build using MinGW GNU GCC
echo              win32vc6.mak  Win32 build using MS VC version 6 (or 98)
echo              win32vc8.mak  Win32 build using MS VC version 8 (or 2005)
echo              win32vc9.mak  Win32 build using MS VC version 9 (or 2008)
echo              win32vc10.mak  Win32 build using MS VC version 10 (or 2010)
echo              win32vc15.mak  Win32 build using MS VC version 15 (or 2017)
echo              win32vc16.mak  Win32 build using MS VC version 16 (or 2019)
echo     -ne  : for NanoEmacs build (output is ne).
echo     -p   : Build with profiling instructions (MinGW)
echo     -S   : Build clean spotless.
echo     -t {type}
echo          : Sets build type:
echo               c   Console support only
echo               w   Wondow support only (default)
echo               cw  Console and window support
echo.

:build_exit
