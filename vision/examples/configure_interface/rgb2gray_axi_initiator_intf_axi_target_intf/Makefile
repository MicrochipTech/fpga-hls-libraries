SRCS=rgb2gray_axi_initiator_intf_axi_target_intf_tb.cpp
LOCAL_CONFIG = -legup-config=config.tcl
LEVEL = $(SHLS_ROOT_DIR)/examples

ifeq ($(shell uname -o), Cygwin)
# Use cygpath to locate the Windows location
OPENCV_PATH := $(shell cygpath -t mixed $(SHLS_ROOT_DIR)/precompiled_sw_libraries/opencv4.5.4-x86_64-cygwin)
else
OPENCV_PATH := $(SHLS_ROOT_DIR)/precompiled_sw_libraries/opencv4.5.4-x86_64-linux
endif

USER_CXX_FLAG += -I$(OPENCV_PATH)/include/opencv4
USER_LINK_FLAG += -L$(OPENCV_PATH)/lib -lopencv_core -lopencv_imgcodecs -lopencv_imgproc

ifeq ($(shell uname -o), Cygwin)
# Use semicolons as PATH separator on Windows
USER_ENV_VARS := PATH=$(OPENCV_PATH)/bin;$$PATH
else
USER_ENV_VARS := LD_LIBRARY_PATH=$(OPENCV_PATH)/lib
endif

include $(LEVEL)/Makefile.common

# Set interface macro here.
# Example: Uncomment the line below will configure the design to run with axi_initiator interface.
# AXI_INITIATOR_INTERFACE = 1

ifdef AXI_INITIATOR_INTERFACE
USER_CXX_FLAG += -DAXI_INITIATOR_INTERFACE
else ifdef AXI_TARGET_INTERFACE
USER_CXX_FLAG += -DAXI_TARGET_INTERFACE
endif