NAME = sin_performance
SRCS = ../sources/test.cpp  

# Root directory path for the HLS libraries. Modify as necessary. 
SHLS_LIBRARY_DIR ?=  ../../../../..

USER_CXX_FLAG += -I$(SHLS_LIBRARY_DIR)
USER_CXX_FLAG += -D__N_ELEM=6400

LOCAL_CONFIG = -shls-config=config.tcl



