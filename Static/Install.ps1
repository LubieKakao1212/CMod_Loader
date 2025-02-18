$steam_reg_path = 'HKCU:\Software\Valve\Steam'
try {
    $steam_dir_path = (Get-ItemProperty -Path $steam_reg_path -ErrorAction Stop -Name SteamPath).SteamPath
}
catch {
    Write-Host "Failed to get path to Steam from the Registry. Error details:"
    Write-Host $_
}

echo "Found Steam at: $steam_dir_path"

$cosmoteer_dir_path = "$steam_dir_path\steamapps\common\Cosmoteer"
if (!(test-path -PathType container $cosmoteer_dir_path)) {
    Throw "Cosmoteer folder not found. Expected at path: $cosmoteer_dir_path"
}

echo "Found Cosmoteer at: $cosmoteer_dir_path"

$cosmoteer_bin_dir_path = "$cosmoteer_dir_path\Bin"
if (!(test-path -PathType container $cosmoteer_bin_dir_path)) {
    Throw "Cosmoteer Bin folder not found. Expected at path: $cosmoteer_bin_dir_path"
}

echo "Copying Loader"...
Copy-Item -Path ".\CMod_Loader.dll" -Destination "$cosmoteer_bin_dir_path\AVRT.dll" -force

echo "All Done!"

pause