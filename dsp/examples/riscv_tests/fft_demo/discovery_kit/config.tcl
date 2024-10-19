source $env(SHLS_ROOT_DIR)/examples/legup.tcl
set_project PolarFireSoC MPFS095T-1FCSG325E Icicle_SoC
set_parameter SOC_POLL_DELAY 1 

# Set other parameters and constraints here
# Refer to the user guide for more information: https://onlinedocs.microchip.com/oxy/GUID-AFCB5DCC-964F-4BE7-AA46-C756FA87ED7B-en-US-11/GUID-3636C6BE-3977-4267-A5DF-A514D1A46BE3.html
set_parameter CLOCK_PERIOD 5
set_parameter MINIMIZE_MATH_BLOCKS 1
#set_resource_constraint fp_extend 27 

#
# Parameters used for SoC integration
#
## The SOC BD NAME is hard coded in the pre integration script /shrug
set_parameter SOC_BD_NAME                 FIC_0_PERIPHERALS
set_parameter SOC_DMA_ENGINE              HARD_DMA
set_parameter SOC_AXI_INITIATOR           AXI2AXI_TO_HLS:AXI4_MASTER
set_parameter SOC_AXI_TARGET              AXI2AXI_FROM_HLS:AXI4_SLAVE
set_parameter SOC_RESET                   ARESETN
set_parameter SOC_CLOCK                   ACLK
set_parameter SOC_FABRIC_BASE_ADDRESS     0x70000000
set_parameter SOC_FABRIC_SIZE             0x400000
set_parameter SOC_CPU_MEM_BASE_ADDRESS    0x80000000
set_parameter SOC_CPU_MEM_SIZE            0x60000000

