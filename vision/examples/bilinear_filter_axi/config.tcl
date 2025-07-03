source $env(SHLS_ROOT_DIR)/examples/legup.tcl
set_project PolarFireSoC MPFS250T Icicle_SoC

# This prevents tready to be asserted before the HLS module becomes active 
# set_parameter REGISTER_EXTERNAL_INPUT_FIFO SKID

set_parameter CLOCK_PERIOD 10
