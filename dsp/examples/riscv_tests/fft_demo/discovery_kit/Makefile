NAME = FFT
LOCAL_CONFIG = -legup-config=config.tcl
SRCS = ../sources/demo.cpp

# Root directory path for the HLS libraries. Modify as necessary. 
SHLS_LIBRARY_DIR = ../../../../..

USER_CXX_FLAG += -I$(SHLS_LIBRARY_DIR)/dsp/include
USER_CXX_FLAG += -O3 -pg -Wall -Wno-strict-aliasing -Wno-unused-label -Wno-unknown-pragmas -Wno-attributes 
USER_CXX_FLAG += -I$(SHLS_ROOT_DIR)/smarthls-library 

# Could be HLS_ALLOC_NONCACHED or HLS_ALLOC_CACHED
USER_CXX_FLAG += -DDDR_REGION=HLS_ALLOC_NONCACHED

# AXI_TYPE: available options are:
# - AXI_TARGET 
# - AXI_INITIATOR (default)
# USER_CXX_FLAG += -DAXI_TARGET
