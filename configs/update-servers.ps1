# Files to replace.
$files_to_copy = "a2serv.exe", "srvmgr.dll"

# Path to folder containing new files
$source = 'C:\Users\Admin\Desktop\'

# Path to folder containing Servers subfolders
$destination = 'C:\Allods2\'

# Fancy output
Write-Output "`r`n##############"
Write-Output "Files To Copy:"
Write-Output "##############`r`n"
foreach ($file in $files_to_copy)
{
    Write-Output $source$file
}

# Get all subfolders matching "*Server*" pattern
$destinationSubFolders = Get-ChildItem -Path $destination -Directory -Filter "*Server*" | Select FullName

# Fancy output
Write-Output "`r`n####################"
Write-Output "Destination Folders:"
Write-Output "####################"
Write-Output $destinationSubFolders | ft -HideTableHeaders

$counter = 0
# Copy all files
$destinationSubFolders | ForEach-Object { Get-Childitem $source -Recurse -Include $files_to_copy | Copy-Item -Destination $_.FullName}
# Rename a2serv.exe -> a2servN.exe
$destinationSubFolders | ForEach-Object { Get-Childitem $_.FullName -Recurse -Include "a2serv.exe" | Move-Item -Destination (Join-Path $_.FullName "\a2serv$((++$counter)).exe") -Force }
Write-Output "`r`nAll files have been copied.`r`n"

# Wait for user input
Write-Output "Press any key..."
$waitForInput = [Console]::ReadKey()
