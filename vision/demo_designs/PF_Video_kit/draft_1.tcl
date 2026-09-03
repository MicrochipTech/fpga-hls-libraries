# Microsemi Tcl Script
# libero
# Date: Tue Jun 16 16:19:31 2026
# Directory C:\Users\c79828\Documents\HLS_Work\fpga-hls-libraries\vision\demo_designs\PF_Video_kit
# File C:\Users\c79828\Documents\HLS_Work\fpga-hls-libraries\vision\demo_designs\PF_Video_kit\draft_1.tcl


new_project -location {./libero/vision_pipeline} -name {vision_pipeline} -project_description {} -block_mode 0 -standalone_peripheral_initialization 0 -instantiate_in_smartdesign 1 -ondemand_build_dh 1 -use_relative_path 0 -linked_files_root_dir_env {} -hdl {VERILOG} -family {PolarFire} -die {MPF300TS} -package {FCG1152} -speed {-1} -die_voltage {1.0} -part_range {IND} -adv_options {IO_DEFT_STD:LVCMOS 1.8V} -adv_options {RESERVEMIGRATIONPINS:1} -adv_options {RESTRICTPROBEPINS:1} -adv_options {RESTRICTSPIPINS:0} -adv_options {TEMPR:IND} -adv_options {UNUSED_MSS_IO_RESISTOR_PULL:None} -adv_options {VCCI_1.2_VOLTR:IND} -adv_options {VCCI_1.5_VOLTR:IND} -adv_options {VCCI_1.8_VOLTR:IND} -adv_options {VCCI_2.5_VOLTR:IND} -adv_options {VCCI_3.3_VOLTR:IND} -adv_options {VOLTR:IND} 
download_core -vlnv {Actel:SystemBuilder:PF_DDR4:2.5.120} -location {www.microchip-ip.com/repositories/SgCore} 
download_core -vlnv {Actel:SystemBuilder:PF_SRAM_AHBL_AXI:1.2.116} -location {www.microchip-ip.com/repositories/SgCore} 
download_core -vlnv {Actel:DirectCore:COREI2C:7.2.101} -location {www.microchip-ip.com/repositories/DirectCore} 
download_core -vlnv {Actel:DirectCore:CoreAPB3:4.2.100} -location {www.microchip-ip.com/repositories/DirectCore} 
download_core -vlnv {Actel:DirectCore:CoreGPIO:3.2.102} -location {www.microchip-ip.com/repositories/DirectCore} 
download_core -vlnv {Actel:DirectCore:COREJTAGDEBUG:4.0.100} -location {www.microchip-ip.com/repositories/DirectCore} 
download_core -vlnv {Actel:DirectCore:CoreAHBLite:5.6.105} -location {www.microchip-ip.com/repositories/DirectCore} 
download_core -vlnv {Actel:SgCore:PF_INIT_MONITOR:2.0.308} -location {www.microchip-ip.com/repositories/SgCore} 
download_core -vlnv {Microsemi:MiV:MIV_RV32IMA_L1_AHB:2.3.100} -location {www.microchip-ip.com/repositories/DirectCore} 
download_core -vlnv {Actel:DirectCore:COREUART:5.6.102} -location {www.microchip-ip.com/repositories/DirectCore} 
download_core -vlnv {Microsemi:SolutionCore:Image_Enhancement:3.0.0} -location {www.microchip-ip.com/repositories/DirectCore} 
download_core -vlnv {Actel:DirectCore:COREAXI4INTERCONNECT:2.8.103} -location {www.microchip-ip.com/repositories/DirectCore} 
download_core -vlnv {Actel:SystemBuilder:PF_DDR4:2.5.120} -location {www.microchip-ip.com/repositories/SgCore} 
import_files \
         -convert_EDN_to_HDL 0 \
         -library {work} \
         -hdl_source {./libero/src/hdl/FrameBufferControl.v} 
build_design_hierarchy 
download_core -vlnv {Actel:SgCore:PF_CCC:2.2.220} -location {www.microchip-ip.com/repositories/SgCore} 
download_core -vlnv {Actel:DirectCore:CORERESET_PF:2.2.107} -location {www.microchip-ip.com/repositories/DirectCore} 
download_core -vlnv {Actel:DirectCore:CORERXIODBITALIGN:2.1.104} -location {www.microchip-ip.com/repositories/DirectCore} 
download_core -vlnv {Actel:SystemBuilder:PF_IOD_GENERIC_RX:2.1.116} -location {www.microchip-ip.com/repositories/SgCore} 
download_core -vlnv {Microsemi:SolutionCore:mipicsi2rxdecoderPF:2.2.5} -location {www.microchip-ip.com/repositories/DirectCore} 
download_core -vlnv {Actel:DirectCore:COREAHBTOAPB3:3.2.101} -location {www.microchip-ip.com/repositories/DirectCore} 
import_files \
         -convert_EDN_to_HDL 0 \
         -library {work} \
         -hdl_source {../../rtl/camera_to_axis_converter/camera_to_axis_converter.v} 
build_design_hierarchy 
create_hdl_core -file {./libero/vision_pipeline/hdl/camera_to_axis_converter.v} -module {Camera_To_AXIS_Converter} -library {work} -package {} 
hdl_core_add_bif -hdl_core_name {Camera_To_AXIS_Converter} -bif_definition {AXI4Stream:AMBA:AMBA4:master} -bif_name {AXIS} -signal_map {"TDATA:o_tdata" "TLAST:o_tlast" "TREADY:i_tready" "TUSER:o_tuser" "TVALID:o_tvalid"} 
build_design_hierarchy 
create_and_configure_core -core_vlnv {Actel:DirectCore:CORERESET_PF:2.2.107} -component_name {CORERESET_PF_C1} -params {} 
create_and_configure_core -core_vlnv {Actel:DirectCore:CORERXIODBITALIGN:2.1.104} -component_name {CORERXIODBITALIGN_C0} -params {"DEM_TAP_WAIT_CNT_WIDTH:3" "HOLD_TRNG:0" "MIPI_TRNG:1" "SKIP_TRNG:0"} 
sd_delete_instances -sd_name {PROC_SUBSYSTEM} -instance_names {"MIV_RV32IMA_L1_AHB_C0_0"} 
create_and_configure_core -core_vlnv {Microsemi:MiV:MIV_RV32:3.1.200} -component_name {MIV_RV32_C0} -params {"AHB_END_ADDR_0:0xffff" "AHB_END_ADDR_1:0x8fff" "AHB_INITIATOR_TYPE:1" "AHB_START_ADDR_0:0x0" "AHB_START_ADDR_1:0x8000" "AHB_TARGET_MIRROR:true" "APB_END_ADDR_0:0xffff" "APB_END_ADDR_1:0x7fff" "APB_INITIATOR_TYPE:1" "APB_START_ADDR_0:0x0" "APB_START_ADDR_1:0x7000" "APB_TARGET_MIRROR:false" "AXI_END_ADDR_0:0xffff" "AXI_END_ADDR_1:0x6fff" "AXI_INITIATOR_TYPE:2" "AXI_START_ADDR_0:0x0" "AXI_START_ADDR_1:0x6000" "AXI_TARGET_MIRROR:false" "BOOTROM_DEST_ADDR_LOWER:0x0" "BOOTROM_DEST_ADDR_UPPER:0x4000" "BOOTROM_PRESENT:false" "BOOTROM_SRC_END_ADDR_LOWER:0x3fff" "BOOTROM_SRC_END_ADDR_UPPER:0x8000" "BOOTROM_SRC_START_ADDR_LOWER:0x0" "BOOTROM_SRC_START_ADDR_UPPER:0x8000" "C_EXT:true" "DEBUGGER:true" "ECC_ENABLE:false" "FWD_REGS:false" "F_EXT:false" "GEN_MUL_TYPE:0" "GPR_REGS:false" "ICACHE_EN:false" "INTERNAL_MTIME:false" "INTERNAL_MTIME_IRQ:false" "I_REGS:false" "I_TRACE:false" "MIV_HART_ID:0x0" "MI_I_MEM:false" "MTIME_PRESCALER:100" "M_EXT:true" "NO_MACC_BLK:false" "NUM_EXT_IRQS:4" "RECONFIG_BOOTROM:false" "RESET_VECTOR_ADDR_0:0x0" "RESET_VECTOR_ADDR_1:0x8000" "TAS_END_ADDR_0:0x3fff" "TAS_END_ADDR_1:0x4000" "TAS_START_ADDR_0:0x0" "TAS_START_ADDR_1:0x4000" "TCM_END_ADDR_0:0x3fff" "TCM_END_ADDR_1:0x4000" "TCM_PRESENT:false" "TCM_REGS:false" "TCM_START_ADDR_0:0x0" "TCM_START_ADDR_1:0x4000" "TCM_TAS_PRESENT:false" "VECTORED_INTERRUPTS:false"} 
sd_instantiate_component -sd_name {PROC_SUBSYSTEM} -component_name {MIV_RV32_C0} -instance_name {} 
sd_mark_pins_unused -sd_name {PROC_SUBSYSTEM} -pin_names {MIV_RV32_C0_0:EXT_RESETN} 
sd_create_pin_slices -sd_name {PROC_SUBSYSTEM} -pin_name {MIV_RV32_C0_0:MSYS_EI} -pin_slices {"[3:3]"} 
sd_create_pin_slices -sd_name {PROC_SUBSYSTEM} -pin_name {MIV_RV32_C0_0:MSYS_EI} -pin_slices {"[2:2]"} 
sd_create_pin_slices -sd_name {PROC_SUBSYSTEM} -pin_name {MIV_RV32_C0_0:MSYS_EI} -pin_slices {"[1:1]"} 
sd_create_pin_slices -sd_name {PROC_SUBSYSTEM} -pin_name {MIV_RV32_C0_0:MSYS_EI} -pin_slices {"[0:0]"} 
sd_delete_nets -sd_name {PROC_SUBSYSTEM} -net_names {COREAHBTOAPB3_0_APBmaster} 
sd_connect_pins -sd_name {PROC_SUBSYSTEM} -pin_names {"CoreAPB3_0:APB3mmaster" "MIV_RV32_C0_0:APB_INITIATOR"} 
sd_connect_pins -sd_name {PROC_SUBSYSTEM} -pin_names {"MIV_RV32_C0_0:CLK" "PCLK"} 
sd_connect_pins -sd_name {PROC_SUBSYSTEM} -pin_names {"MIV_RV32_C0_0:RESETN" "reset"} 
sd_connect_pins -sd_name {PROC_SUBSYSTEM} -pin_names {"COREJTAGDEBUG_0:TGT_TCK_0" "MIV_RV32_C0_0:JTAG_TCK"} 
sd_connect_pins -sd_name {PROC_SUBSYSTEM} -pin_names {"COREJTAGDEBUG_0:TGT_TDI_0" "MIV_RV32_C0_0:JTAG_TDI"} 
sd_connect_pins -sd_name {PROC_SUBSYSTEM} -pin_names {"COREJTAGDEBUG_0:TGT_TDO_0" "MIV_RV32_C0_0:JTAG_TDO"} 
sd_mark_pins_unused -sd_name {PROC_SUBSYSTEM} -pin_names {MIV_RV32_C0_0:JTAG_TDO_DR} 
sd_connect_pins -sd_name {PROC_SUBSYSTEM} -pin_names {"COREJTAGDEBUG_0:TGT_TMS_0" "MIV_RV32_C0_0:JTAG_TMS"} 
sd_connect_pins -sd_name {PROC_SUBSYSTEM} -pin_names {"COREJTAGDEBUG_0:TGT_TRST_0" "MIV_RV32_C0_0:JTAG_TRSTN"} 
sd_connect_pins -sd_name {PROC_SUBSYSTEM} -pin_names {"COREI2C_C0_0:INT" "MIV_RV32_C0_0:MSYS_EI[0:0]"} 
sd_connect_pins -sd_name {PROC_SUBSYSTEM} -pin_names {"COREI2C_C0_1:INT" "MIV_RV32_C0_0:MSYS_EI[1:1]"} 
sd_connect_pins -sd_name {PROC_SUBSYSTEM} -pin_names {"COREI2C_C0_2:INT" "MIV_RV32_C0_0:MSYS_EI[2:2]"} 
sd_delete_nets -sd_name {PROC_SUBSYSTEM} -net_names {CoreAHBLite_C0_0_AHBmslave16} 
sd_connect_pins -sd_name {PROC_SUBSYSTEM} -pin_names {"MIV_RV32_C0_0:AHBL_M_TARGET" "PF_SRAM_AHBL_AXI_C0_0:AHBSlaveInterface"} 
sd_mark_pins_unused -sd_name {PROC_SUBSYSTEM} -pin_names {MIV_RV32_C0_0:AXI4_INITIATOR} 
sd_delete_instances -sd_name {PROC_SUBSYSTEM} -instance_names {"COREAHBTOAPB3_0"} 
sd_delete_instances -sd_name {PROC_SUBSYSTEM} -instance_names {"CoreAHBLite_C0_0"} 
sd_delete_instances -sd_name {PROC_SUBSYSTEM} -instance_names {"CoreAHBLite_0"} 
save_smartdesign -sd_name {PROC_SUBSYSTEM} 
save_project 
