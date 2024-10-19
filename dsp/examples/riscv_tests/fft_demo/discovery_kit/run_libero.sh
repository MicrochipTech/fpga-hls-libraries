#!/bin/bash
#
# Usage:
#   ./run_libero.sh
# Notes:
#   This script will create a directory in your SHLS project directory, create a 
#   working directory called "work", and copy in the Discovery Kit reference 
#   design included in SHLS.

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

args=SMARTHLS:$HLS_PATH
args+=+SYNTHESIZE
args+=+PLACEROUTE
args+=+VERIFYTIMING
args+=+HSS_UPDATE:1
args+=+EXPORT_FPE:$prjDir
libero script:MPFS_DISCOVERY_KIT_REFERENCE_DESIGN.tcl \
    script_args:$args \
    logfile:disco_kit_sin_performance.log

echo "ALL DONE"
