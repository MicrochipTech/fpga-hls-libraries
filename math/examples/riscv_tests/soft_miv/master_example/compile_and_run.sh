#!/bin/bash

set -euxo pipefail


# Generate the bitstream
shls clean
shls -a soc_accel_proj_pnr

# Compile the software
./compile.sh

#The next 3 commands assume the board is connected to the machine the script is running on
# Program the FPGA
shls -s soc_accel_proj_program

# OpenOCD interferes with JTAG programming
killall openocd

# Restart OpenOCD in the background
# If don't want running in bkg:
# openocd - board/microsemi-riscv.cfg
openocd -f board/microsemi-riscv.cfg &

# Run the .elf on the board
riscv64-unknown-elf-gdb ./build/main.elf -x ./gdb.txt
