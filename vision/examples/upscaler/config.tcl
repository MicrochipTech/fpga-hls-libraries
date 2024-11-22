source $env(SHLS_ROOT_DIR)/examples/legup.tcl
set_project PolarFireSoC MPFS250T Icicle_SoC

# This prevents tready to be asserted before the HLS module becomes active 
set_parameter REGISTER_EXTERNAL_INPUT_FIFO SKID

set_parameter CLOCK_PERIOD 3.33

set_resource_constraint signed_divide_40 3
set_resource_constraint fp_fptosi_32_32 2
set_resource_constraint fp_truncate_64 2
set_resource_constraint signed_divide_64 4
