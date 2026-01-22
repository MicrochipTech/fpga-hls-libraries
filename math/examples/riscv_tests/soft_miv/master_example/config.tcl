source $env(SHLS_ROOT_DIR)/examples/shls.tcl
set_project PolarFire MPF300 MiV_SoC

# Set other parameters and constraints here
# Refer to the user guide for more information: https://onlinedocs.microchip.com/oxy/GUID-AFCB5DCC-964F-4BE7-AA46-C756FA87ED7B-en-US-17/Chunk1566049232.html#Chunk1566049232
set_resource_constraint signed_divide_64 3
set_parameter MINIMIZE_MATH_BLOCKS 1
