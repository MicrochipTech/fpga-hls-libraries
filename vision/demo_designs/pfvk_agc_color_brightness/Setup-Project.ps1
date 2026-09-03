# Project setup for pfvk_agc_color_brightness
# Run this once per session: . .\Setup-Project.ps1

# Map Z: to the repo root (shorten paths for Synplify Pro on Windows)
$repoRoot = "C:\Users\c79828\Documents\HLS_Work\fpga-hls-libraries"
$drive = "Z:"

$existing = (subst) | Where-Object { $_ -match "^$drive" }
if (-not $existing) {
    subst $drive $repoRoot
    Write-Host "Mapped $drive => $repoRoot"
} else {
    Write-Host "$drive already mapped: $existing"
}

# Set SHLS_ROOT_DIR for make
$env:SHLS_ROOT_DIR = "C:\Microchip\Libero_SoC_2025.2\SmartHLS\SmartHLS"
Write-Host "SHLS_ROOT_DIR = $env:SHLS_ROOT_DIR"

# Switch to the libero directory on the short drive
Set-Location "Z:\vision\demo_designs\pfvk_agc_color_brightness\libero"
Write-Host "Working directory: $(Get-Location)"
Write-Host ""
Write-Host "Ready. Run: .\run_libero.ps1"
