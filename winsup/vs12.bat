@echo off
setlocal
call "%VS120COMNTOOLS%..\..\VC\vcvarsall.bat" amd64
%*
exit /b %ERRORLEVEL%
endlocal

