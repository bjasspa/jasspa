@echo off
set ERRORLEVEL=
mec -p @mesgen -f %*
EXIT /B %ERRORLEVEL%
