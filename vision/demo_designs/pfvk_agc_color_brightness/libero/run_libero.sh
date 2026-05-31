
echo "Compiling SmartHLS AGC + Color + Brightness + Gamma Correction demo!"

# Exit on error (-e), on unset vars (-u), and fail pipelines if any command fails (pipefail).
set -euo pipefail

# Save the current directory so we can return to it at the end.
cwd="$(pwd)"

#get directory path of script
script_path=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" && pwd )

cleanup() {
# Define a cleanup function that always runs at script exit.
    echo
    echo "Done"
    cd "$cwd"
}

on_error() {
# Define an error handler for any command failure.
    echo
    echo "ERROR. Something went wrong!"
    echo "Failed command: $BASH_COMMAND"
    echo "At line: $1"
}

trap cleanup EXIT
# Always run cleanup when the script exits (success or failure).
trap 'on_error "$LINENO"' ERR
# Run the error handler when any command fails.

# [ Step 1]---------------------------------------------------------------------
# Generate the Verilog and the C++ driver API.
echo "Compiling SmartHLS module."
cd "$script_path/../hls"
shls clean
shls hw

# [ Step 2]---------------------------------------------------------------------
# Include generated API driver and compile sw/main.cpp.
# The .hex file should be copied into libero/src/cfg_and_mem_files.
echo "Compiling Software for MiV."
cd "$script_path/../sw"
make

# [ Step 3]---------------------------------------------------------------------
# Integrate generated SmartHLS Verilog and compiled .hex into Libero.
# Generate the bitstream and remove the previous Libero run directory.
echo "Compiling Libero project."
cd "$script_path"
rm -rf vision_pipeline
libero script:libero_flow.tcl logfile:libero_flow.log
