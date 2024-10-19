source $env(SHLS_ROOT_DIR)/examples/legup.tcl
set_project PolarFireSoC MPFS095T-1FCSG325E Icicle_SoC
set_parameter POINTSTO_ANALYZE_HW_ONLY 1

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

set_parameter CLOCK_PERIOD 5
