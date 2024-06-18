SRCS=bilateral_filter.cpp  
LOCAL_CONFIG = -legup-config=config.tcl
LEVEL = $(SHLS_ROOT_DIR)/examples

SHLS_VISION_LIBRARY ?= ../../
SHLS_MATH_LIBRARY ?= ../../../math

#-------------------------------------------------------------------------------
# For X86_64 compilation
#-------------------------------------------------------------------------------
OPENCV_PATH = $(SHLS_VISION_LIBRARY)/precompiled_sw_libraries/opencv4.5.4-x86_64
USER_CXX_FLAG += -I$(OPENCV_PATH)/include/opencv4 -I$(SHLS_VISION_LIBRARY)/include -I$(SHLS_MATH_LIBRARY)/include -O3
USER_LINK_FLAG += -L$(OPENCV_PATH)/lib -lopencv_core -lopencv_imgcodecs -lopencv_imgproc

# Windows needs libraries on PATH
# USER_ENV_VARS += PATH=$(OPENCV_PATH)/lib$(PATH_SEP)$$PATH
# Linux needs libraries on LD_LIBRARY_PATH
USER_ENV_VARS += LD_LIBRARY_PATH=$(OPENCV_PATH)/lib

#-------------------------------------------------------------------------------
# For RISC-V cross-compilation
#-------------------------------------------------------------------------------
OPENCV_PATH_RISCV = $(SHLS_VISION_LIBRARY)/precompiled_sw_libraries/opencv4.5.4-riscv_64
USER_CXX_FLAG_RISCV += -I$(OPENCV_PATH_RISCV)/include/opencv4 -I$(SHLS_VISION_LIBRARY)/include -I$(SHLS_MATH_LIBRARY)/include -g
USER_LINK_FLAG_RISCV += -L$(OPENCV_PATH_RISCV)/lib -lopencv_core -lopencv_imgcodecs -lopencv_imgproc
