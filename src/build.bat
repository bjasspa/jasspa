@echo off

set makefile=
set OPTIONS=
:build_option
if "%1." == "."    goto build_cont
if "%1" == "-m"    goto build_mkfl
if "%1" == "-h"    goto build_help
if "%1" == "-C"    set OPTIONS=clean
if "%1" == "-d"    set OPTIONS=med
if "%1" == "-n"    set OPTIONS=men
if "%1" == "-u"    set OPTIONS=meu
if "%1" == "-nu"   set OPTIONS=menu
if "%1" == "-S"    set OPTIONS=spotless
shift
goto build_option

:build_mkfl
shift
set makefile=%1
shift
goto build_option

:build_cont

if "%makefile%." == "." goto build_auto

set make=nmake
if "%makefile%" == "dosdj1.mak"  set make=make
if "%makefile%" == "dosdj2.mak"  set make=make

echo %make% -f %makefile% %OPTIONS%
%make% -f %makefile% %OPTIONS%

goto build_exit

:build_auto

if "%DJGPP%." == "." goto build_win32

:build_dos

echo make -f dosdj1.mak %OPTIONS%
make -f dosdj1.mak %OPTIONS%

goto build_exit

:build_win32

rem test for 4dos - no 4dos == no smarts
if NOT "%@LOWER[A]" == "a" goto build_msvc5

rem try to auto detect the build tools available
set lpath=%@LOWER[%PATH%]
set borland=%@INDEX[%lpath%,borland]
if %borland% == -1 goto build_msvc
set msvc=%@INDEX[%lpath%,\vc]
if %msvc% == -1 goto build_borland

if %borland% GT %msvc% goto build_msvc

:build_borland

echo make -f win32bc.mak %OPTIONS%
make -f win32bc.mak %OPTIONS%

goto build_exit

:build_msvc

if %@INDEX[%lpath%,\vc98] == %msvc% goto build_msvc6

:build_msvc5

echo nmake -f win32v5.mak %OPTIONS%
nmake -f win32v5.mak %OPTIONS%

goto build_exit

:build_msvc6

echo nmake -f win32v6.mak %OPTIONS%
nmake -f win32v6.mak %OPTIONS%

goto build_exit

:build_help

echo Usage: build [options]
echo .
echo Where options can be:-
echo     -C   : Build clean.
echo     -d   : build dos or win32 debug version (produces med.exe or med32.exe)
echo     -h   : Print this help page
echo     -m {makefile}
echo            Sets build makefile to be used where {makefile} can be:-
echo            dosdj1.mak   Dos build using djgpp version 1 
echo            dosdj2.mak   Dos build using djgpp version 2 
echo            win32bc.mak  Win32 build using Borland C
echo            win32v2.mak  Win32 build using MS VC version 2
echo            win32v5.mak  Win32 build using MS VC version 5
echo            win32v6.mak  Win32 build using MS VC version 6 (or 98)
echo            win32sv2.mak Win32s build (for Win 3.xx) using MS VC version 2
echo            win32sv4.mak Win32s build (for Win 3.xx) using MS VC version 4
echo     -n   : Build win32 console version (produces men32.exe)
echo     -nu  : Build win32 console and url version (produces menu32.exe)
echo     -S   : Build clean spotless.
echo     -u   : Build win32 url version (produces meu32.exe)

:build_exit

