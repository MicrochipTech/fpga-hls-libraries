SRCS=../../../sources/sin_performance/test.cpp  

TRANSFER_TYPE?=AXI_TARGET
#TRANSFER_TYPE?=AXI_INITIATOR
USE_DMA=true
N_ELEM=6400
SHLS_MATH_LIBRARY=../../../../../

USER_CXX_FLAG += -I$(SHLS_MATH_LIBRARY) -I$(SHLS_ROOT_DIR)/smarthls-library 
USER_CXX_FLAG += -D__N_ELEM=$(N_ELEM) -D$(TRANSFER_TYPE) -D__USE_DMA=$(USE_DMA)
## Note: The BeagleV-Fire doesn't have a cached DDR region
USER_CXX_FLAG += -D__CACHED=HLS_ALLOC_NONCACHED 

LOCAL_CONFIG = -legup-config=config.tcl
LEVEL = $(SHLS_ROOT_DIR)/examples


