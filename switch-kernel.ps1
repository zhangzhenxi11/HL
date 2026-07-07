param(
    [string]$Mode,
    [switch]$Force
)

$ErrorActionPreference = 'Stop'

$root = Split-Path -Parent $MyInvocation.MyCommand.Path
$kernelPath = Join-Path $root 'kernel.xml'
$backupPath = Join-Path $root 'kernel.xmlbak'
$backupDir = Join-Path $root 'kernel_backups'
$simulateKeyword = ([char]0x6A21) + ([char]0x62DF)
$siteKeyword = ([char]0x73B0) + ([char]0x573A)

function Normalize-Mode([string]$InputMode) {
    if (-not $InputMode) {
        return $null
    }

    $m = $InputMode.Trim().ToLowerInvariant()
    switch ($m) {
        '1' { return 'simulate' }
        'simulate' { return 'simulate' }
        '2' { return 'site' }
        'site' { return 'site' }
        '3' { return 'status' }
        'status' { return 'status' }
        default { return $null }
    }
}

function Get-SceneFile([string]$Scene) {
    $simulateExplicit = Join-Path $root ("kernel_{0}.xml" -f $simulateKeyword)
    $siteExplicit = Join-Path $root ("kernel_{0}.xml" -f $siteKeyword)

    if ($Scene -eq 'simulate' -and (Test-Path -LiteralPath $simulateExplicit)) {
        return Get-Item -LiteralPath $simulateExplicit
    }

    if ($Scene -eq 'site' -and (Test-Path -LiteralPath $siteExplicit)) {
        return Get-Item -LiteralPath $siteExplicit
    }

    $candidates = Get-ChildItem -LiteralPath $root -File -Filter 'kernel_*.xml'

    if ($Scene -eq 'simulate') {
        $preferred = $candidates | Where-Object { $_.Name -like ("*{0}*" -f $simulateKeyword) } | Select-Object -First 1
        if ($preferred) { return $preferred }

        return $candidates |
            Where-Object { $_.Name -notlike ("*{0}*" -f $siteKeyword) -and $_.BaseName -notlike '*_old' } |
            Select-Object -First 1
    }

    if ($Scene -eq 'site') {
        $preferred = $candidates | Where-Object { $_.Name -like ("*{0}*" -f $siteKeyword) } | Select-Object -First 1
        if ($preferred) { return $preferred }

        return $candidates |
            Where-Object { $_.BaseName -like '*_old' } |
            Select-Object -First 1
    }

    throw "Unknown scene: $Scene"
}

function Get-FileSha256([string]$Path) {
    if (-not (Test-Path -LiteralPath $Path)) {
        return $null
    }

    return (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash
}

function Get-CurrentScene {
    $simulateFile = Get-SceneFile 'simulate'
    $siteFile = Get-SceneFile 'site'
    $kernelHash = Get-FileSha256 $kernelPath
    if (-not $kernelHash) {
        return 'missing'
    }

    if ($simulateFile -and $kernelHash -eq (Get-FileSha256 $simulateFile.FullName)) {
        return 'simulate'
    }

    if ($siteFile -and $kernelHash -eq (Get-FileSha256 $siteFile.FullName)) {
        return 'site'
    }

    return 'custom'
}

function Ensure-BackupDirectory {
    if (-not (Test-Path -LiteralPath $backupDir)) {
        New-Item -ItemType Directory -Path $backupDir | Out-Null
    }
}

function New-BackupFilePath([string]$Prefix) {
    Ensure-BackupDirectory
    $timestamp = Get-Date -Format 'yyyyMMdd_HHmmss'
    return Join-Path $backupDir ("{0}_{1}.xml" -f $Prefix, $timestamp)
}

function Save-CurrentKernelBackup([string]$Prefix) {
    if (-not (Test-Path -LiteralPath $kernelPath)) {
        return $null
    }

    $timestampBackupPath = New-BackupFilePath $Prefix
    Copy-Item -LiteralPath $kernelPath -Destination $backupPath -Force
    Copy-Item -LiteralPath $kernelPath -Destination $timestampBackupPath -Force
    return $timestampBackupPath
}

function Show-Status {
    $scene = Get-CurrentScene
    $simulateFile = Get-SceneFile 'simulate'
    $siteFile = Get-SceneFile 'site'
    Write-Host "Current kernel.xml scene: $scene"
    Write-Host "kernel.xml      : $(Test-Path -LiteralPath $kernelPath)"
    Write-Host "kernel.xmlbak   : $(Test-Path -LiteralPath $backupPath)"
    Write-Host "backup dir      : $backupDir"
    Write-Host "simulate source : $($simulateFile.FullName)"
    Write-Host "site source     : $($siteFile.FullName)"
}

$Mode = Normalize-Mode $Mode

if (-not $Mode) {
    Write-Host 'Select mode:'
    Write-Host '  1) simulate'
    Write-Host '  2) site'
    Write-Host '  3) status'
    $Mode = Normalize-Mode (Read-Host 'Input 1/2/3')
}

if (-not $Mode) {
    throw 'Invalid mode.'
}

if ($Mode -eq 'status') {
    Show-Status
    exit 0
}

$sourceFile = Get-SceneFile $Mode
$sourcePath = if ($sourceFile) { $sourceFile.FullName } else { $null }

if (-not $sourceFile -or -not (Test-Path -LiteralPath $sourcePath)) {
    $available = Get-ChildItem -LiteralPath $root -File -Filter 'kernel*.xml' | Select-Object -ExpandProperty Name
    Write-Host "Available kernel files: $($available -join ', ')"
    throw "Source file not found for scene: $Mode"
}

if (Test-Path -LiteralPath $kernelPath) {
    $currentScene = Get-CurrentScene
    if ($currentScene -eq 'custom' -and -not $Force) {
        $protectedBackupPath = Save-CurrentKernelBackup 'kernel_custom_protected'
        Write-Host "Detected custom kernel.xml, protected backup: $protectedBackupPath"
        throw 'kernel.xml contains manual changes and does not match simulate/site templates. Refusing to overwrite. Use -Force only after confirming the backup is correct.'
    }

    if ($currentScene -eq $Mode) {
        Write-Host "kernel.xml is already in scene: $Mode"
        Show-Status
        exit 0
    }

    $switchBackupPath = Save-CurrentKernelBackup 'kernel_before_switch'
    Write-Host "Backup updated: $backupPath"
    Write-Host "Timestamp backup: $switchBackupPath"
}
else {
    Write-Host 'kernel.xml not found, skip backup.'
}

Copy-Item -LiteralPath $sourcePath -Destination $kernelPath -Force

$sourceHash = Get-FileSha256 $sourcePath
$kernelHash = Get-FileSha256 $kernelPath

if ($sourceHash -ne $kernelHash) {
    throw 'Verification failed: kernel.xml does not match source file after copy.'
}

Write-Host "Switched to: $Mode"
Write-Host "Source      : $sourcePath"
Write-Host "Target      : $kernelPath"
Show-Status
