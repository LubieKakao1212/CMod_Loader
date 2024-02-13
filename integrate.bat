set SolutionDir=%1
set Platform=%2
set Configuration=%3

set loader_dir_src_path=%SolutionDir%%Platform%%Configuration%
echo %loader_dir_src_path%

exit

set helper_dir_src_path="C:\Users\aliser\Desktop\repos\CMod_Loader\CMod_LoaderHelper\bin\Release\net7.0-windows"
set loader_mod_dir_dist_path="C:\Users\aliser\Saved Games\Cosmoteer\76561198068709671\Mods\cmod_loader"
set game_bin_dir_path="C:\Program Files (x86)\Steam\steamapps\common\Cosmoteer\Bin"


@REM =================


@REM check if cosmoteer process is running
tasklist | find "Cosmoteer.exe" >nul: && goto kill_cosmoteer_process

goto skip_cosmoteer_taskkill

:kill_cosmoteer_process

@REM stop cosmoteer process
taskkill /f /im cosmoteer.exe /FI "STATUS eq RUNNING"

@REM sleep for N - 1 seconds, so that the cosmoteer process can properly end
ping 127.0.0.1 -n 3 > nul


@REM =================


@REM copy stuff in
copy %loader_dir_src_path%"\CMod_Loader.dll" %loader_mod_dir_dist_path%"\AVRT.dll"
copy %loader_dir_src_path%"\CMod_Loader.pdb" %loader_mod_dir_dist_path%"\AVRT.pdb"

copy %helper_dir_src_path%"\CMod_LoaderHelper.dll" %loader_mod_dir_dist_path%"\bin"
copy %helper_dir_src_path%"\CMod_LoaderHelper.pdb" %loader_mod_dir_dist_path%"\bin"
copy %helper_dir_src_path%"\CMod_LoaderHelper.runtimeconfig.json" %loader_mod_dir_dist_path%"\bin"

copy %loader_mod_dir_dist_path%"\AVRT.dll" %game_bin_dir_path%
copy %loader_mod_dir_dist_path%"\AVRT.pdb" %game_bin_dir_path%


@REM =================


@REM restart cosmoteer 
start "" "C:\Program Files (x86)\Steam\steamapps\common\Cosmoteer\Bin\Cosmoteer.exe" --devmode