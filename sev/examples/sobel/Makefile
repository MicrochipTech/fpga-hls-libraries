SRCS=sobel_tb.cpp
LOCAL_CONFIG = -legup-config=config.tcl
LEVEL = $(LEGUP_ROOT_DIR)/examples

ifeq ($(shell uname -o), Cygwin)
OPENCV_PATH = $(LEGUP_ROOT_DIR)/precompiled_sw_libraries/opencv4.5.4-x86_64-cygwin
FFMPEG_PATH = $(LEGUP_ROOT_DIR)/precompiled_sw_libraries/ffmpeg4.4-x86_64-cygwin
else
OPENCV_PATH = $(LEGUP_ROOT_DIR)/precompiled_sw_libraries/opencv4.5.4-x86_64-linux
FFMPEG_PATH = $(LEGUP_ROOT_DIR)/precompiled_sw_libraries/ffmpeg4.4-x86_64-linux
endif

USER_CXX_FLAG += -I$(OPENCV_PATH)/include/opencv4
USER_LINK_FLAG += -L$(OPENCV_PATH)/lib -lopencv_core -lopencv_imgcodecs -lopencv_imgproc
USER_ENV_VARS = LD_LIBRARY_PATH=$(OPENCV_PATH)/lib:$(FFMPEG_PATH)/lib

include $(LEVEL)/Makefile.common
