param ([string]$build_output_abs_path)

# =============
# Variables
# =============

# path to the Cosmoteer executable
$cosmoteer_exe_path = "C:\Program Files (x86)\Steam\steamapps\common\Cosmoteer\Bin\Cosmoteer.exe"

# user mods directory; no trailing slash
$cosmoteer_mods_directory_path = "C:\Users\aliser\Saved Games\Cosmoteer\76561198068709671\Mods"

# mod folder name for Loader
$cmod_loader_mod_folder_name = "CMod_Loader"

# filename of the Loader install script
$install_script_filename = "Install.ps1"

# whether to restart (or start) Cosmoteer after the script is done
$restart_cosmoteer = $false

# whether to start Cosmoteer in devmode. 
# works only if $restart_cosmoteer is enabled.
$restart_cosmoteer_devmode = $true

# =============
# Script
# =============

$mod_dir_path = "$cosmoteer_mods_directory_path\$cmod_loader_mod_folder_name"
$install_script_filepath = "$mod_dir_path\$install_script_filename"

Echo ">>>>>> Running Integration for CMod Loader <<<<<<<"
Echo "     Build path (absolute) : $build_output_abs_path"
Echo "CMod Loader directory path : $mod_dir_path"

# Check if build path exists (ie passed correctly from the build process)
if(!(test-path -PathType container $build_output_abs_path)) {
  Throw "Build path not found. Make sure it is passed correctly (in a post-build action to this script). Path: $build_output_abs_path"
}

# Check if Cosmoteer mods directory path exist (ie valid)
if(!(test-path -PathType container $cosmoteer_mods_directory_path)) {
  Throw "Cosmoteer mods directory not found. Make sure the path exists: $cosmoteer_mods_directory_path"
}

# Check if Cosmoteer executable exists (if restarting is enabled)
if($restart_cosmoteer -And !([System.IO.File]::Exists($cosmoteer_exe_path))) {
	Throw "Cosmoteer executable not found. Path: $cosmoteer_exe_path"
}

# Stop Cosmoteer process if it's running
if(Get-Process -ErrorAction SilentlyContinue -Name "cosmoteer") {
  Echo "Found Cosmoteer process, stopping"
  Get-Process -Name "cosmoteer" -ErrorAction SilentlyContinue | Stop-Process -Force

  Echo "Sleeping for some time after killing the process"
  Start-Sleep -Seconds 3
} else {
  Echo "Cosmoteer process is not running"
}

# Create mod directory if it doesn't exist
# Clear it out if it does.
if(test-path -PathType container $mod_dir_path) {
  Echo "Loader directory found, clearing"

  Get-ChildItem $mod_dir_path | Remove-Item -Recurse -Force
} else {
  Echo "Loader directory not found, creating"

  New-Item -ItemType Directory -Force -Path $mod_dir_path
}

# Copy files from the release directory to the mod directory
Echo "Copying build files into the Loader directory"
Copy-Item -Path "$build_output_abs_path\*" -Destination "$mod_dir_path\" -Recurse -Force

# Copy static files if any
if(test-path -PathType container ".\static") {
	Echo "Copying static files into the Loader directory"
	Copy-Item -Path ".\Static\*" -Destination "$mod_dir_path\" -Recurse -Force
}

# Run the install script
if(test-path -PathType container $install_script_filepath) {
  Throw "Install script not found within the Laoder mod directory"
}

Echo "Running Install script"
Echo "======================"
# temporarily change to the correct folder
Push-Location $mod_dir_path
# run script
Powershell.exe -File $install_script_filepath 
# now back to previous directory
Pop-Location
Echo "======================"

# Restart Cosmoteer if set
if($restart_cosmoteer) {
  Echo "Starting Cosmoteer process"

  if($restart_cosmoteer_devmode) {
	  Start-Process -FilePath $cosmoteer_exe_path -ArgumentList "--devmode"
  } else {
	  Start-Process -FilePath $cosmoteer_exe_path
  }
}

Echo "<<<<<<< CMod Loader Integration finished! >>>>>>"