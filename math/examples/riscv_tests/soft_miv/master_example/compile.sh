#!/bin/bash

set -euxo pipefail
# set -euo pipefail

rm -rf ./build
mkdir ./build

MIV_PLATFORM=../../../../../support/miv/platform

COMMON_FLAGS=""
COMMON_FLAGS+=" -march=rv32imc"
COMMON_FLAGS+=" -mabi=ilp32"
COMMON_FLAGS+=" -msmall-data-limit=8"
COMMON_FLAGS+=" -mno-save-restore"
# COMMON_FLAGS+=" -O0"
COMMON_FLAGS+=" -O1"
COMMON_FLAGS+=" -fmessage-length=0"
COMMON_FLAGS+=" -fno-builtin-printf"
COMMON_FLAGS+=" -fsigned-char"
COMMON_FLAGS+=" -ffunction-sections"
COMMON_FLAGS+=" -fdata-sections"
# COMMON_FLAGS+=" -g3"
COMMON_FLAGS+=" -g"
COMMON_FLAGS+=" -DMSCC_STDIO_THRU_CORE_UART_APB"

COMMON_FLAGS+=" -MMD"
COMMON_FLAGS+=" -MP"

CFLAGS+=$COMMON_FLAGS
CFLAGS+=" -std=gnu11"

CPPFLAGS+=$COMMON_FLAGS
CPPFLAGS+=" -std=c++11"
CPPFLAGS+=" -fpermissive"

LDFLAGS+=$COMMON_FLAGS
LDFLAGS+=" -T $SHLS_ROOT_DIR/reference_designs/MiV_SoC/miv-rv32-ram.ld"
LDFLAGS+=" -nostartfiles"
LDFLAGS+=" -Xlinker"
LDFLAGS+=" --gc-sections"
LDFLAGS+=" -Wl,-Map,ref_sc_miv.map"

INCLUDES=""
INCLUDES+=" -I$SHLS_ROOT_DIR/reference_designs/MiV_SoC"
INCLUDES+=" -I$MIV_PLATFORM"
INCLUDES+=" -I$MIV_PLATFORM/drivers/fpga_ip/CoreUARTapb"
INCLUDES+=" -I$MIV_PLATFORM/miv_rv32_hal/"
INCLUDES+=" -I$MIV_PLATFORM/miv_rv32_hal/hal"
INCLUDES+=" -I$SHLS_ROOT_DIR/smarthls-library"
INCLUDES+=" -I./hls_output/accelerator_drivers"
INCLUDES+=" -I./"

echo "----------------"
echo "Compile the drivers & HAL"
riscv64-unknown-elf-gcc $CFLAGS $INCLUDES -c $MIV_PLATFORM/drivers/fpga_ip/CoreUARTapb/core_uart_apb.c -o ./build/core_uart_apb.o
riscv64-unknown-elf-gcc $CFLAGS $INCLUDES -c $MIV_PLATFORM/hal/hal_irq.c -o ./build/hal_irq.o
riscv64-unknown-elf-gcc $CFLAGS $INCLUDES -c $MIV_PLATFORM/miv_rv32_hal/miv_rv32_hal.c -o ./build/miv_rv32_hal.o
riscv64-unknown-elf-gcc $CFLAGS $INCLUDES -c $MIV_PLATFORM/miv_rv32_hal/miv_rv32_init.c -o ./build/miv_rv32_init.o
riscv64-unknown-elf-gcc $CFLAGS $INCLUDES -c $MIV_PLATFORM/miv_rv32_hal/miv_rv32_stubs.c -o ./build/miv_rv32_stubs.o
riscv64-unknown-elf-gcc $CFLAGS $INCLUDES -c $MIV_PLATFORM/miv_rv32_hal/miv_rv32_syscall.c -o ./build/miv_rv32_syscall.o
riscv64-unknown-elf-gcc $CFLAGS -x assembler-with-cpp -c $MIV_PLATFORM/hal/hw_reg_access.S -o ./build/hw_reg_access.o
riscv64-unknown-elf-gcc $CFLAGS -x assembler-with-cpp -c $MIV_PLATFORM/miv_rv32_hal/miv_rv32_entry.S -o ./build/miv_rv32_entry.o

echo "----------------"
echo "Compile accelerator drivers"
riscv64-unknown-elf-g++ $CPPFLAGS $INCLUDES \
    -c ./hls_output/accelerator_drivers/test_accelerator_driver.cpp \
    -o ./build/test_accelerator_driver.o

echo "----------------"
echo "Compile main()"
riscv64-unknown-elf-g++ $CPPFLAGS $INCLUDES \
    -MF"./build/main.d" -MT"./build/main.o" -c main.cpp -o ./build/main.o

echo "----------------"
echo "Linking everything"
riscv64-unknown-elf-g++ $LDFLAGS \
    ./build/miv_rv32_entry.o \
    ./build/miv_rv32_hal.o \
    ./build/miv_rv32_init.o \
    ./build/miv_rv32_stubs.o \
    ./build/miv_rv32_syscall.o  \
    ./build/hal_irq.o \
    ./build/hw_reg_access.o  \
    ./build/core_uart_apb.o  \
    ./build/test_accelerator_driver.o \
    ./build/main.o \
    -o ./build/main.elf


echo "----------------"
echo ".elf to .hex"
riscv64-unknown-elf-objcopy -O ihex --change-section-lma *-0x80000000 ./build/main.elf ./build/main.hex
riscv64-unknown-elf-size --format=berkeley ./build/main.elf
