# Usage:
#   .\run_libero.ps1
# Notes:
#   This script will create a directory in your SHLS project directory, create a working directory called "work", and copy in the Discovery Kit reference design included in SHLS.

$prjDir = "soc"
$HLS_PATH = Get-Location

#
# Start from a clean state
#

Remove-Item $HLS_PATH/hls_output, "work", "component" -Recurse -Force -ErrorAction SilentlyContinue

# # Copy the reference design files to the hls project directory
mkdir work -ErrorAction SilentlyContinue
Copy-Item -Recurse ../../../../../support/discovery_kit/polarfire-soc-discovery-kit-reference-design work/libero

try {
    pushd work/libero
    libero script:MPFS_DISCOVERY_KIT_REFERENCE_DESIGN.tcl `
        script_args:VERIFYTIMING+PLACEROUTE+SYNTHESIZE+SMARTHLS:$HLS_PATH+EXPORT_FPE:$prjDir+HSS_UPDATE:1 `
        logfile:disco_kit_sin_performance.log | Write-Output
} finally {
    popd
}
    
Write-Output "ALL DONE"