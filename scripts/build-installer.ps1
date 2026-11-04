param(
    [string]$AppVersion = "1.0.0",
    [string]$SourceDir = "build/install",
    [string]$OutputDir = "dist"
)

$ErrorActionPreference = "Stop"

Write-Host "==> Packaging OBS Audio Router Installer v$AppVersion..."

if (-not (Test-Path $OutputDir)) {
    New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null
}

# Locate ISCC
$iscc = (Get-Command iscc -ErrorAction SilentlyContinue).Source
if (-not $iscc) {
    $searchPaths = @(
        "C:\Program Files (x86)\Inno Setup 6\iscc.exe",
        "C:\Program Files\Inno Setup 6\iscc.exe"
    )
    foreach ($p in $searchPaths) {
        if (Test-Path $p) {
            $iscc = $p
            break
        }
    }
}

if (-not $iscc) {
    Write-Warning "Inno Setup Compiler (iscc.exe) not found on host. The setup.iss script is ready for CI or machines with Inno Setup installed."
    return
}

Write-Host "Running ISCC: $iscc"
& $iscc installer/setup.iss "/DAppVersion=$AppVersion" "/DSourceDir=$SourceDir" "/DOutputDir=$OutputDir"
if ($LASTEXITCODE -ne 0) {
    throw "ISCC compilation failed with code $LASTEXITCODE"
}

Write-Host "==> Installer created successfully in $OutputDir"
