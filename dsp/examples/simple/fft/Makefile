
NAME = fft
LOCAL_CONFIG = -legup-config=config.tcl
SRCS = test.cpp
LEVEL = $(SHLS_ROOT_DIR)/examples


USER_CXX_FLAG += -O3 -pg -Wall -Wno-strict-aliasing -Wno-unused-label -Wno-unknown-pragmas -Wno-attributes -I$(SHLS_ROOT_DIR)/smarthls-library
SHLS_FFT_LIBRARY=../../../
USER_CXX_FLAG += -I$(SHLS_FFT_LIBRARY)/include -I$(SHLS_FFT_LIBRARY)/examples

include $(LEVEL)/Makefile.common

