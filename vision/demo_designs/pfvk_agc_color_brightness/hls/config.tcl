source $env(SHLS_ROOT_DIR)/examples/legup.tcl
set_project PolarFire MPF300 MiV_SoC

# 200 MHz
set_parameter CLOCK_PERIOD 5

# Base address of the SmartHLS sub-system. This address range will include
# all top-level functions.
set_parameter SOC_FABRIC_BASE_ADDRESS       0x60000000
set_parameter SOC_FABRIC_SIZE               0x200000
