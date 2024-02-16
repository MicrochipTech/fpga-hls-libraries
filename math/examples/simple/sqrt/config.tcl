source $env(SHLS_ROOT_DIR)/examples/legup.tcl
set_project PolarFire MPF300 hw_only

#set_parameter CLOCK_PERIOD 4
#set_parameter SYNTHESIS_CLOCK_PERIOD 4
set_parameter CLOCK_PERIOD 2.5
set_synthesis_top_module sqrt_hls_M_wrapper_top