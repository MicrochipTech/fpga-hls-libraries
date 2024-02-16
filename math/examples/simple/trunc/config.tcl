source $env(SHLS_ROOT_DIR)/examples/legup.tcl
set_project PolarFire MPF300 hw_only

# Set other parameters and constraints here
# Refer to the user guide for more information: https://onlinedocs.microchip.com/oxy/GUID-AFCB5DCC-964F-4BE7-AA46-C756FA87ED7B-en-US-11/GUID-3636C6BE-3977-4267-A5DF-A514D1A46BE3.html
set_parameter CLOCK_PERIOD 2.5 
set_synthesis_top_module trunc_hls_M_wrapper_top