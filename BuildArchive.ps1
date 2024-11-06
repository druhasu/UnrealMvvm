Add-Type -AssemblyName System.IO.Compression.FileSystem

$PluginName = "UnrealMvvm"

$PluginDefinition = Get-Content -Raw .\$PluginName\$PluginName.uplugin | ConvertFrom-Json
$PluginVersion = $PluginDefinition.VersionName

$ArchiveName = $PluginName + "_v" + $PluginVersion + ".zip"

if(Test-Path $ArchiveName)
{
    throw ("Archive with version '" + $PluginVersion + "' already exist")
}

$TrackedFiles = git ls-tree -r HEAD --name-only
$FilteredFiles = $TrackedFiles | Where-Object { $_.StartsWith($PluginName + "/") }

# save main plugin files to archive
$ZipArchive = [System.IO.Compression.ZipFile]::Open($ArchiveName, 1)
foreach ($File in $FilteredFiles)
{
    [System.IO.Compression.ZipFileExtensions]::CreateEntryFromFile($ZipArchive, $File, $File.Substring($PluginName.Length + 1)) | Out-Null
}

# add Readme and License files
[System.IO.Compression.ZipFileExtensions]::CreateEntryFromFile($ZipArchive, "LICENSE", "LICENSE") | Out-Null
[System.IO.Compression.ZipFileExtensions]::CreateEntryFromFile($ZipArchive, "README.md", "README.md") | Out-Null

# close archive
$ZipArchive.Dispose()