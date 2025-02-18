# Uninstall the CMod Loader.

echo "=== CMod Loader Uninstall ==="

# source: https://stackoverflow.com/a/22362868
Function Pause ($Message = "Press any key to continue . . . ") {
    if ((Test-Path variable:psISE) -and $psISE) {
        $Shell = New-Object -ComObject "WScript.Shell"
        $Button = $Shell.Popup("Click OK to continue.", 0, "Script Paused", 0)
    }
    else {     
        Write-Host -NoNewline $Message
        [void][System.Console]::ReadKey($true)
        Write-Host
    }
}

Function Delete-Paths-If-Exists-Verbose ($Paths) {
    foreach ($filePath in $Paths) {
        if (Test-Path $filePath) {
            Remove-Item $filePath -Verbose
        }
        else {
            Write-Host -ForegroundColor Yellow "Path does not exist: $filePath"
        }
    }
}

$failed_try_manual_msg = "=== CMod Loader uninstall failed === `nPlease, uninstall the Loader manually: `n1. Open Cosmoteer Bin folder. `n2. Remove 'AVRT.dll' file."

$steam_reg_path = 'HKCU:\Software\Valve\Steam'
try {
    $steam_dir_path = (Get-ItemProperty -Path $steam_reg_path -ErrorAction Stop -Name SteamPath).SteamPath
}
catch {
    echo "Failed to get path to Steam from the Registry. Error details:"
    echo $_
    echo $failed_try_manual_msg
    Pause
    exit
}

echo "Found Steam at: $steam_dir_path"


$cosmoteer_bin_dir_path = "$steam_dir_path\steamapps\common\Cosmoteer\Bin"
if (!(test-path -PathType container $cosmoteer_bin_dir_path)) {
    echo "Cosmoteer Bin folder not found. Expected at path: $cosmoteer_bin_dir_path" 
    echo $failed_try_manual_msg
    Pause
    exit
}

echo "Found Cosmoteer Bin at: $cosmoteer_bin_dir_path"


# remove loader and logfile
echo "Uninstalling Loader..."
Delete-Paths-If-Exists-Verbose ("$cosmoteer_bin_dir_path\AVRT.dll", "$cosmoteer_bin_dir_path\CMod_Loader.log")


echo "All Done!"

Pause