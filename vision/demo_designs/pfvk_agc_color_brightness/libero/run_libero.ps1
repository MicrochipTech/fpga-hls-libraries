# Three easy steps:
#
#   1. Generate the SmartHLS Verilog & driver API
#   2. Compile the software for the Soft-MiV CPU
#   3. Compile the Libero project

Write-Output @"

Compiling SmartHLS AGC + Color + Brightness + Gamma Correction demo!

"@

# Stop the script as soon as there's an error in any command. 
$ErrorActionPreference = "Stop"

# Get the current location to come back after the compilation process.
$cwd = Get-Location

# Get the directory path of the this script.
$scriptPath = Split-Path -Parent $MyInvocation.MyCommand.Definition

try {
    # [ Step 1]---------------------------------------------------------------------
    # Generate the Verilog and the C++ driver API
    Write-Output "Compiling SmartHLS module."
    cd $scriptPath/../hls 
    shls clean
    shls hw 

    # [ Step 2]---------------------------------------------------------------------
    # This will include the generated API driver and compile the sw/main.cpp file.
    # The .hex file will be copied into the libero\src\cfg_and_mem_files directory. 
    # memory from it. 
    Write-Output "Compiling Software for MiV."
    cd $scriptPath/../sw
    mingw32-make.exe

    # [ Step 3]---------------------------------------------------------------------
    # Integrate the SmartHLS verilog and compiled .hex into the Libero project and 
    # generate the bitstream. Also, delete the previous libero run.
    Write-Output "Compiling Libero project."
    cd $scriptPath    
    rm vision_pipeline -ErrorAction SilentlyContinue -Force -Recurse
    libero.exe script:libero_flow.tcl logfile:libero_flow.log  |  Write-Output
} catch {
    Write-Output "`nERROR. Something went wrong!`n"
    $Error[0].Exception.Message
} finally {
    # Return to the original directory
    Write-Output "`nDone`n"
    cd $cwd
}