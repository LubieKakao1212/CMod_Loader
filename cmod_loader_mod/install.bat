@echo off    
rem automatically copies avrt.dll to cosmoteer bin folder

@REM will be 1, if the debug flag is set by the build script
set print_debug=%1

@REM a path to cosmoteer bin dir
@REM set when print_debug is set, since it's that in dev env when the loader mod
@REM is located in saved games dir where finding cosmoteer installation is not viable
set cosmoteer_bin_dir_override=%2

IF /I "%print_debug%" NEQ "1" goto skip_debug_echo

echo [DEBUG] running in debug mode

set cosmoteer_bin_dir=%cosmoteer_bin_dir_override%

goto skip_cosmoteer_bin_dir_search

:skip_debug_echo

@REM ================

rem  take current dir
set "current_dir=%~dp0"

IF /I "%print_debug%" NEQ "1" goto skip_current_dir_echo

echo [DEBUG] current dir: %current_dir%

:skip_current_dir_echo

@REM ================

rem go 4 levels up 
for %%I in ("%current_dir%\..\..\..\..") do set "root=%%~fI"

IF /I "%print_debug%" NEQ "1" goto skip_root_dir_echo

echo [DEBUG] cosmoteer root dir: %root%

:skip_current_dir_echo

@REM ================

set "cosmoteer_bin_dir=%root%\common\Cosmoteer\Bin\"

:skip_cosmoteer_bin_dir_search

echo +----------------------------------------------+
echo Installing to %cosmoteer_bin_dir%
echo +----------------------------------------------+

if exist "%cosmoteer_bin_dir%\Cosmoteer.exe" (
	goto cosmoteer_exe_found
) else (
	goto cosmoteer_exe_notfound
)

@REM ================

:cosmoteer_exe_found
copy /b/v/y "%current_dir%bin\AVRT.dll" "%cosmoteer_bin_dir%AVRT.dll"
if exist "%cosmoteer_bin_dir%bin\AVRT.dll" (
	goto install_success
)
echo Something went wrong while copying AVRT.dll
echo Please install manually by copying AVRT.dll (inside bin dir) to %cosmoteer_bin_dir%
pause
goto exit_error

@REM ================

:install_success
echo +----------------------------------------------+
echo Installation successfull!
echo +----------------------------------------------+
goto exit_success

@REM ================

:cosmoteer_exe_notfound
echo Error: Could not find Cosmoteer Bin Folder!
echo Please install manually by copying AVRT.dll (inside bin dir) to your Cosmoteer Bin folder
pause
goto exit_error

@REM ================

:exit_success
exit /b 0

:exit_error
exit /b 1