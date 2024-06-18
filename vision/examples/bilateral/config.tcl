source $env(SHLS_ROOT_DIR)/examples/legup.tcl
set_project PolarFire MPF300 hw_only

# Set other parameters and constraints here
# Refer to the user guide for more information: https://microchiptech.github.io/fpga-hls-docs/constraintsmanual.html

# This crashes
# set_parameter CLOCK_PERIOD 3.3

# This works
set_parameter CLOCK_PERIOD 10

set_parameter MINIMIZE_MATH_BLOCKS 1
