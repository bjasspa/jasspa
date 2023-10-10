@echo off
set ERRORLEVEL=
mecw32 -p -n @mesgen -f %*
EXIT /B %ERRORLEVEL%
