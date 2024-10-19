
NAME = FFT
LOCAL_CONFIG = -legup-config=config.tcl
SRCS = ../sources/demo.cpp
LEVEL = $(SHLS_ROOT_DIR)/examples


USER_CXX_FLAG += -O3 -pg -Wall -Wno-strict-aliasing -Wno-unused-label -Wno-unknown-pragmas -Wno-attributes -I$(SHLS_ROOT_DIR)/smarthls-library


# AXI_TYPE: available options are:
# - AXI_TARGET (default)
# - AXI_INITIATOR
# USER_CXX_FLAG += -DAXI_TARGET


# SHLS_DSP_LIBRARY should be the path to the dsp directory: 
SHLS_DSP_LIBRARY=../../../../
USER_CXX_FLAG += -I$(SHLS_DSP_LIBRARY)/include

include $(LEVEL)/Makefile.common

