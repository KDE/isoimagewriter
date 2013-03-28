@echo off

set SRCDIR=%~dp0
set DSTDIR=%1
if "%DSTDIR%" == "" (
	set DSTDIR=%SRCDIR%
) else (
	set DSTDIR=%DSTDIR%\lang
)
mkdir %DSTDIR% 2>nul
for %%t in ("%SRCDIR%\*.ts") do lrelease "%%t" -qm "%DSTDIR%\%%~nt.qm"
