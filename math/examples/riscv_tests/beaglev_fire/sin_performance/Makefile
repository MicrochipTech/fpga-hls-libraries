# Detect the operating system
ifeq ($(OS),Windows_NT)
    detected_OS := Windows
    RM = del /Q /F
    MKDIR = mkdir
    CP = copy
    FIX_PATH = $(subst /,\,$1)
    SHLS_CMD = $(shell where shls)
else
    detected_OS := $(shell uname -s)
    RM = rm -rf
    MKDIR = mkdir -p
    CP = cp
    FIX_PATH = $1
    SHLS_CMD = $(shell which shls)
endif


CC = riscv64-unknown-linux-gnu-gcc
BUILD_DIR = ./build
HLS_PRJ_DIR = ./hls
SHLS_ROOT_DIR ?= $(patsubst %\bin\$(notdir $(SHLS_CMD)),%,$(SHLS_CMD))
TRANSFER_TYPE?=AXI_TARGET
USE_DMA=true
N_ELEM=6400

SRCS += $(HLS_PRJ_DIR)/hls_output/.hls/sw/hls_accelerator_driver.cpp 
SRCS += $(HLS_PRJ_DIR)/hls_output/.hls/sw/test.cpp

FLAGS = -g -std=c++11 -mcmodel=medany -march=rv64gc -mabi=lp64d -pthread -lm -lrt -lstdc++ -fpermissive -O3

DEFINES = DN_ELEM=$(N_ELEM) -D__CACHED=HLS_ALLOC_NONCACHED 
DEFINES += -D$(TRANSFER_TYPE) -D__USE_DMA=$(USE_DMA)  

INCLUDES = -I$(HLS_PRJ_DIR)/hls_output/.hls/sw -I../../../.. -I$(SHLS_ROOT_DIR)/smarthls-library 

LDFLAGS = -L$(HLS_PRJ_DIR)/hls_patch 
LDFLAGS +=  -Wl,-Map,$(BUILD_DIR)/hls.map  -lhls_riscv64_accel 

OUTFILE = $(BUILD_DIR)/hls.accel.elf 

.PHONY: all

all: clean make_accel make_sw_binary

clean:
	cd $(call FIX_PATH,$(HLS_PRJ_DIR)); $(SHLS_CMD) clean
	$(RM) $(call FIX_PATH,$(BUILD_DIR))

make_accel:
	## Run soc_sw_transform first
	cd $(call FIX_PATH,$(HLS_PRJ_DIR)); $(SHLS_CMD) -a hw; $(SHLS_CMD) -a soc_source_transform

make_sw_binary:
	$(RM) $(call FIX_PATH,$(BUILD_DIR))
	$(MKDIR) $(call FIX_PATH,$(BUILD_DIR))
	$(CC) $(SRCS) $(FLAGS) $(DEFINES) $(INCLUDES) $(LDFLAGS) -o $(OUTFILE) 
