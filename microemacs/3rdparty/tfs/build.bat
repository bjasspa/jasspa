@echo off
rem JASSPA MicroEmacs - www.jasspa.com
rem build - JASSPA MicroEmacs build script for MS windows and dos platforms
rem Copyright (C) 2001-2009 JASSPA (www.jasspa.com)
rem See the file main.c for copying and conditions.
set OPTIONS=
set LOGFILE=
set LOGFILEA=
set BCFG=release
set MAKEFILE=
:build_option
if "%1." == "."    goto build_cont
if "%1" == "-C"    set  OPTIONS=clean
if "%1" == "-d"    set  BCFG=debug
if "%1" == "-h"    goto build_help
if "%1" == "-l"    goto build_logf
if "%1" == "-la"   goto build_logfa
if "%1" == "-m"    goto build_mkfl
if "%1" == "-S"    set  OPTIONS=spotless
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

:build_cont

set MAKE=nmake
if "%MAKEFILE%." == "." goto build_auto
if "%MAKEFILE%" == "dosdj1.mak"  set MAKE=make
if "%MAKEFILE%" == "dosdj2.mak"  set MAKE=make
if "%MAKEFILE%" == "mingw32.gmk" set MAKE=mingw32-make

goto build

:build_auto

if "%DJGPP%." == "." goto build_win32

:build_dos

set MAKE=make
set MAKEFILE=dosdj1.mak

goto build

:build_win32

rem test for 4dos - no 4dos == no smarts
if NOT "%@LOWER[A]" == "a" goto build_msvs5

rem try to auto detect the build tools available
set lpath=%@LOWER[%PATH%]
set borland=%@INDEX[%lpath%,borland]
set msvs=%@INDEX[%lpath%,\vc]
if %borland% == -1 goto build_msvs
if %msvs% == -1 goto build_borland

if %borland% GT %msvs% goto build_msvs

:build_borland

set MAKE=make

if %@INDEX[%lpath%,\bcc55] == -1 goto build_borland_bc

set MAKEFILE=win32b55.mak
goto build

:build_borland_bc

set MAKEFILE=win32bc.mak
goto build

:build_msvs

set MAKE=nmake
if %@INDEX[%lpath%,\vc98] == %msvs% goto build_msvs6

:build_msvs5

set MAKEFILE=win32vs5.mak
goto build

:build_msvs6

set MAKEFILE=win32vs6.mak

:build

cd ..\zlib

if "%LOGFILE%." == "." goto build_zapplog

echo %MAKE% -f %MAKEFILE% BCFG=%BCFG% %OPTIONS% > %LOGFILE% 2>&1
%MAKE% -f %MAKEFILE% BCFG=%BCFG% %OPTIONS% > %LOGFILE% 2>&1
cd ..\tfs

goto build_tfs

:build_zapplog

if "%LOGFILEA%." == "." goto build_znolog

echo %MAKE% -f %MAKEFILE% BCFG=%BCFG% %OPTIONS% >> %LOGFILEA% 2>&1
%MAKE% -f %MAKEFILE% BCFG=%BCFG% %OPTIONS% >> %LOGFILEA% 2>&1
cd ..\tfs

goto build_tfs

:build_znolog

echo %MAKE% -f %MAKEFILE% BCFG=%BCFG% %OPTIONS%
%MAKE% -f %MAKEFILE% BCFG=%BCFG% %OPTIONS%
cd ..\tfs

:build_tfs

if "%LOGFILE%." == "." goto build_applog

echo %MAKE% -f %MAKEFILE% BCFG=%BCFG% %OPTIONS% > %LOGFILE% 2>&1
%MAKE% -f %MAKEFILE% BCFG=%BCFG% %OPTIONS% > %LOGFILE% 2>&1

goto build_exit

:build_applog

if "%LOGFILEA%." == "." goto build_nolog

echo %MAKE% -f %MAKEFILE% BCFG=%BCFG% %OPTIONS% >> %LOGFILEA% 2>&1
%MAKE% -f %MAKEFILE% BCFG=%BCFG% %OPTIONS% >> %LOGFILEA% 2>&1

goto build_exit

:build_nolog

echo %MAKE% -f %MAKEFILE% BCFG=%BCFG% %OPTIONS%
%MAKE% -f %MAKEFILE% BCFG=%BCFG% %OPTIONS%

goto build_exit

:build_help

echo Usage: build [options]
echo .
echo Where options can be:-
echo     -C   : Build clean.
echo     -d   : For debug build (output is med* or ned*)
echo     -h   : For this help page
echo     -l {logfile}
echo          : Set the compile log file
echo     -la {logfile}
echo          : Append the compile log to the given file
echo     -m {makefile}
echo            Sets the makefile to use where {makefile} can be:-
echo              dosdj1.mak   Dos build using djgpp version 1 
echo              dosdj2.mak   Dos build using djgpp version 2 
echo              mingw32.gmk  Win32 build using MinGW GNU GCC
echo              win32bc.mak  Win32 build using Borland C
echo              win32vs2.mak Win32 build using MS VS version 2
echo              win32vs5.mak Win32 build using MS VS version 5
echo              win32vs6.mak Win32 build using MS VS version 6 (or 98)
echo              win32sv2.mak Win32s build (for Win 3.xx) using MS VC version 2
echo              win32sv4.mak Win32s build (for Win 3.xx) using MS VC version 4
echo     -S   : Build clean spotless.

:build_exit

