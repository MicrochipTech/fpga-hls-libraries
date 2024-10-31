source $env(SHLS_ROOT_DIR)/examples/legup.tcl
set_project PolarFire MPF300 hw_only

set_parameter CLOCK_PERIOD 2.5

# This prevents tready to be asserted before the HLS module becomes active 
set_parameter REGISTER_EXTERNAL_INPUT_FIFO SKID
set_synthesis_top_module inplace_fft_wrapper_top