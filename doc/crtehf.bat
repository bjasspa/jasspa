@echo off

cd c:\me\nroff
nmake meehf.hts
if %errorlevel% == 1 then goto exit

cd c:\me\ehf
del -y c:\me\ehf\me\*.*
hts2html -l .htm ..\nroff\meehf.hts
if %errorlevel% == 1 then goto exit

copy me.htm me\1.htm
cd .\me
me32 "@ehftools" *.htm
cd ..

:exit
