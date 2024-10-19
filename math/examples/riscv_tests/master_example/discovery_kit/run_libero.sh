#!/bin/bash
#
# Usage:
#   ./run_libero.sh
# Notes:
#   This script will create a directory in your SHLS project directory, create a working directory called "work", and copy in the Discovery Kit reference design included in SHLS.

set -e

prjDir=soc
HLS_PATH=$(pwd)

#
# Start from a clean state
#
rm -rf \
    $HLS_PATH/hls_output \
    work \
    component

# Copy the reference design files to the hls project directory
mkdir work
cp -r ../../../../../support/discovery_kit/polarfire-soc-discovery-kit-reference-design/ work
cd work/polarfire-soc-discovery-kit-reference-design
libero script:MPFS_DISCOVERY_KIT_REFERENCE_DESIGN.tcl script_args:VERIFYTIMING+PLACEROUTE+SYNTHESIZE+SMARTHLS:$HLS_PATH+EXPORT_FPE:$prjDir+HSS_UPDATE:1 logfile:disco_kit_master_example.log

echo "ALL DONE"
