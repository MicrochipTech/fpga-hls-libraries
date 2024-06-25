# Detect the operating system
ifeq ($(OS),Windows_NT)
    detected_OS := Windows
    RM = powershell rm -ErrorAction SilentlyContinue -Force -Recurse
    MKDIR = powershell New-Item -Type Directory -Force -Path
    CP = powershell cp
    # FIX_PATH = $(subst /,\,$1)
    FIX_PATH = $1
else
    detected_OS := $(shell uname -s)
    RM = rm -rf
    MKDIR = mkdir -p
    CP = cp
    FIX_PATH = $1
endif

# Set the tools based on the detected operating system
ifeq ($(detected_OS),Windows)
    CC = riscv64-unknown-elf-gcc
    CXX = riscv64-unknown-elf-g++
    OBJCOPY = riscv64-unknown-elf-objcopy
    SIZE = riscv64-unknown-elf-size
    SHLS_ROOT_DIR ?= $(patsubst %\bin\shls.bat,%,$(shell powershell "(Get-Command shls).Path"))
else
    # Set to the appropriate tools for Linux or other OS
    CC = riscv64-unknown-elf-gcc
    CXX = riscv64-unknown-elf-g++
    OBJCOPY = riscv64-unknown-elf-objcopy
    SIZE = riscv64-unknown-elf-size
    SHLS_ROOT_DIR ?= $(patsubst %/bin/shls,%,$(shell which shls))
endif

CC = riscv64-unknown-elf-gcc
CXX = riscv64-unknown-elf-g++
OBJCOPY = riscv64-unknown-elf-objcopy
SIZE = riscv64-unknown-elf-size
BUILD_DIR = ./build
MIV_PLATFORM = ../../../../support/miv/platform
HLS_PRJ ?= ../hls

COMMON_FLAGS = -march=rv32imc -mabi=ilp32 -msmall-data-limit=8 -mno-save-restore 
COMMON_FLAGS += -Os -fmessage-length=0 -fno-builtin-printf -fsigned-char 
COMMON_FLAGS += -ffunction-sections -fdata-sections -MMD -MP
# COMMON_FLAGS += -g3

# Comment this to disable the PRINTF macro. Keep in mind the code size will 
# increase.  Approx. 16KB without PRINTF, 64KB with PRINTF
COMMON_FLAGS += -DUSE_UART

CFLAGS = $(COMMON_FLAGS) -std=gnu11
CPPFLAGS = $(COMMON_FLAGS) -std=c++11 -fpermissive
LDFLAGS += $(COMMON_FLAGS) -T ./miv-rv32-ram.ld -nostartfiles -Xlinker
LDFLAGS += --gc-sections -Wl,-Map,$(BUILD_DIR)/ref_sc_miv.map

INCLUDES += -I.
INCLUDES += -I./fpga_design_config
INCLUDES += -I./imx334_corei2c 
INCLUDES += -I$(MIV_PLATFORM) 
INCLUDES += -I$(MIV_PLATFORM)/hal
INCLUDES += -I$(MIV_PLATFORM)/miv_rv32_hal 
INCLUDES += -I$(MIV_PLATFORM)/drivers/fpga_ip/CoreI2C 
INCLUDES += -I$(MIV_PLATFORM)/drivers/fpga_ip/CoreUARTapb
INCLUDES += -I$(MIV_PLATFORM)/drivers/fpga_ip/CoreGPIO 
INCLUDES += -I$(MIV_PLATFORM)/drivers/fpga_ip/CoreTimer 
INCLUDES += -I$(SHLS_ROOT_DIR)/smarthls-library
INCLUDES += -I$(HLS_PRJ)/hls_output/accelerator_drivers

SRC = $(wildcard $(MIV_PLATFORM)/drivers/fpga_ip/CoreGPIO/*.c) \
      $(wildcard $(MIV_PLATFORM)/drivers/fpga_ip/CoreI2C/core_i2c.c) \
      $(wildcard $(MIV_PLATFORM)/drivers/fpga_ip/CoreTimer/*.c) \
      $(wildcard $(MIV_PLATFORM)/drivers/fpga_ip/CoreUARTapb/*.c) \
      $(wildcard $(MIV_PLATFORM)/hal/*.c) \
      $(wildcard $(MIV_PLATFORM)/miv_rv32_hal/*.c) \
      $(wildcard ./imx334_corei2c/*.c) \
      $(wildcard $(HLS_PRJ)/hls_output/accelerator_drivers/*.cpp) \
      ./main.cpp

ASMSRC = $(wildcard $(MIV_PLATFORM)/hal/*.S) \
         $(wildcard $(MIV_PLATFORM)/miv_rv32_hal/*.S)

SRC_OBJ := $(addsuffix .o,$(basename $(SRC))) $(addsuffix .o,$(basename $(ASMSRC)))
BUILD_OBJ = $(foreach obj,$(SRC_OBJ), $(BUILD_DIR)/$(notdir $(obj)))

.PHONY: all clean

all: $(BUILD_DIR)/main.hex

%.o: %.S | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -x assembler-with-cpp -c $< -o $(BUILD_DIR)/$(@F)

%.o: %.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $(BUILD_DIR)/$(@F)

%.o: %.cpp | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(INCLUDES) -c $< -o $(BUILD_DIR)/$(@F)

$(BUILD_DIR)/main.elf: $(SRC_OBJ)
	$(CXX) $(LDFLAGS) $(BUILD_OBJ) -o $(BUILD_DIR)/$(@F)

$(BUILD_DIR)/main.hex: $(BUILD_DIR)/main.elf
	$(OBJCOPY) -O ihex --change-section-lma *-0x80000000 $< $@
	$(SIZE) --format=berkeley $<

# Create build directory
$(BUILD_DIR):
	$(MKDIR) $(call FIX_PATH,$@)

# Copy the generated .hex file to the specific directory from which Libero will
# take it to include it in the bitstream. 
export_hex: $(BUILD_DIR)/main.hex
	$(CP) $(call FIX_PATH,$(BUILD_DIR)/main.hex) $(call FIX_PATH,../libero/src/cfg_and_mem_files/imx334_4lane_raw10_1188Mbps_AE.hex)

clean:
	$(RM) $(call FIX_PATH,$(BUILD_DIR))
