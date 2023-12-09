@echo off

for %%a in (%*) do set dstpth=%%a

:CpNxt
if "%1." == "." goto ext
if "%1" == "%dstpth%" goto ext
copy /Y "%1" "%dstpth%"
shift
goto CpNxt

:ext
