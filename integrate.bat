set solution_dir=%1
set platform=%2
set configuration=%3
@REM without end slash
set cosmoteer_bin_dir_path=%4
@REM without end slash
set cmod_loader_mod_dir_path=%5

set release_dir_path=%solution_dir%%platform%"\"%configuration%"\"
set helper_release_dir_path=%solution_dir%"CMod_LoaderHelper\bin\"%configuration%"\net7.0-windows\"
set mod_installer_script_filename="install.bat"

@REM =================


@REM check if cosmoteer process is running
tasklist | find "Cosmoteer.exe" >nul: && goto kill_cosmoteer_process

goto skip_cosmoteer_taskkill

:kill_cosmoteer_process
echo [info]: killing cosmoteer process

@REM stop cosmoteer process
taskkill /f /im cosmoteer.exe /FI "STATUS eq RUNNING"

@REM sleep for N - 1 seconds, so that the cosmoteer process can properly end
ping 127.0.0.1 -n 3 > nul

:skip_cosmoteer_taskkill
@REM =================

@REM copy the loader dll and files, and rename them to a dll that the game gonna load
echo [info]: copying cmod loader dll and files

set from=%release_dir_path%"CMod_Loader.dll"
set to=%cmod_loader_mod_dir_path%"\AVRT.dll"
echo [info]: from %from%
echo [info]: to %to%
copy %from% %to%

set from=%cmod_loader_mod_dir_path%"\AVRT.pdb"
set to=%release_dir_path%"CMod_Loader.pdb"
echo [info]: from %from%
echo [info]: to %to%
copy %from% %to%


@REM get the helper dll and files
echo [info]: copying cmod loader helper dll and files

set from=%helper_release_dir_path%"CMod_LoaderHelper.dll"
set to=%cmod_loader_mod_dir_path%"\bin"
echo [info]: from %from%
echo [info]: to %to%
copy %from% %to%

set from=%helper_release_dir_path%"CMod_LoaderHelper.pdb"
set to=%cmod_loader_mod_dir_path%"\bin"
echo [info]: from %from%
echo [info]: to %to%
copy %from% %to%

set from=%helper_release_dir_path%"CMod_LoaderHelper.runtimeconfig.json"
set to=%cmod_loader_mod_dir_path%"\bin"
echo [info]: from %from%
echo [info]: to %to%
copy %from% %to%


@REM run the installer and echo a string to skip the pause
echo [info]: running cmod loader install
echo | call %cmod_loader_mod_dir_path%%mod_installer_script_filename%
@echo pause skip

@REM =================


@REM start cosmoteer 
echo [info]: starting cosmoteer
start "" %cosmoteer_bin_dir_path%"\Cosmoteer.exe" --devmode