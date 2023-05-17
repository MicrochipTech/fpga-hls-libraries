#Libero project creation
new_project -location {./vision_pipeline} -name {vision_pipeline} -project_description {} -block_mode 0 -hdl Verilog -family {PolarFire} -die {MPF300TS} -package {FCG1152} -speed {-1} -die_voltage {1.0} -part_range {IND} -adv_options {IO_DEFT_STD:LVCMOS 1.8V} -adv_options {RESERVEMIGRATIONPINS:1} -adv_options {RESTRICTPROBEPINS:1} -adv_options {RESTRICTSPIPINS:0} -adv_options {TEMPR:IND} -adv_options {UNUSED_MSS_IO_RESISTOR_PULL:None} -adv_options {VCCI_1.2_VOLTR:IND} -adv_options {VCCI_1.5_VOLTR:IND} -adv_options {VCCI_1.8_VOLTR:IND} -adv_options {VCCI_2.5_VOLTR:IND} -adv_options {VCCI_3.3_VOLTR:IND} -adv_options {VOLTR:IND} 

# Set IP core version variables
source ../../../rtl/ip_core_versions.tcl

#Download all the required cores to the vault (Camera and display components will download their required cores separately)
download_core -vlnv "Microsemi:SolutionCore:Display_Controller:${Display_Controller_version}"  -location {www.microchip-ip.com/repositories/DirectCore}
download_core -vlnv "Actel:SystemBuilder:PF_DDR4:${PF_DDR4_version}" -location {www.microchip-ip.com/repositories/SgCore}
download_core -vlnv "Actel:SystemBuilder:PF_SRAM_AHBL_AXI:${PF_SRAM_AHBL_AXI_version}" -location {www.microchip-ip.com/repositories/SgCore}
download_core -vlnv "Actel:DirectCore:COREI2C:${COREI2C_version}" -location {www.microchip-ip.com/repositories/DirectCore}
download_core -vlnv "Actel:DirectCore:CoreAPB3:${CoreAPB3_version}" -location {www.microchip-ip.com/repositories/DirectCore}
download_core -vlnv "Actel:DirectCore:CoreGPIO:${CoreGPIO_version}" -location {www.microchip-ip.com/repositories/DirectCore}
download_core -vlnv "Actel:DirectCore:COREJTAGDEBUG:${COREJTAGDEBUG_version}" -location {www.microchip-ip.com/repositories/DirectCore}
download_core -vlnv "Actel:DirectCore:CoreAHBLite:${CoreAHBLite_version}" -location {www.microchip-ip.com/repositories/DirectCore}
download_core -vlnv "Actel:SgCore:PF_INIT_MONITOR:${PF_INIT_MONITOR_version}" -location {www.microchip-ip.com/repositories/SgCore}
download_core -vlnv "Microsemi:MiV:MIV_RV32IMA_L1_AHB:${MIV_RV32IMA_L1_AHB_version}" -location {www.microchip-ip.com/repositories/DirectCore}
download_core -vlnv "Actel:DirectCore:COREUART:${COREUART_version}" -location {www.microchip-ip.com/repositories/DirectCore}
download_core -vlnv "Microsemi:SolutionCore:Image_Enhancement:${Image_Enhancement_version}" -location {www.microchip-ip.com/repositories/DirectCore}
download_core -vlnv "Actel:DirectCore:COREAXI4INTERCONNECT:${COREAXI4INTERCONNECT_verion}" -location {www.microchip-ip.com/repositories/DirectCore}

#source the below tcl file to create the top level SmartDesign and generate it
cd src
source ./VIDEO_KIT_TOP_recursive.tcl
cd ..
set_root -module {VIDEO_KIT_TOP::work} 
delete_component -component_name {temp}
build_design_hierarchy 

#Import the constraint files and organize the Synthesize, Place and Route and Verify Timing tools
import_files \
         -io_pdc ./src/constraint/user_io.pdc
import_files \
         -fp_pdc ./src/constraint/user_fp.pdc
import_files \
         -sdc ./src/constraint/user.sdc

organize_tool_files -tool {PLACEROUTE} -file {./vision_pipeline/constraint/io/user_io.pdc} -file {./vision_pipeline/constraint/fp/user_fp.pdc} -file {./vision_pipeline/constraint/user.sdc} -module {VIDEO_KIT_TOP::work} -input_type {constraint} 
organize_tool_files -tool {VERIFYTIMING} -file {./vision_pipeline/constraint/user.sdc} -module {VIDEO_KIT_TOP::work} -input_type {constraint} 

derive_constraints_sdc
save_project

if {[info exists env(SystemDrive)]} {

if {[catch {
    # Remove empty concatenations if there are any
    exec sed -i {/assign COREAXI4INTERCONNECT_C0_0_AXI4mslave0_WSTRB = {  }/d} vision_pipeline/component/work/VIDEO_KIT_TOP/VIDEO_KIT_TOP.v
    exec sed -i {/assign COREAXI4INTERCONNECT_C0_0_AXI4mslave0_WDATA = {  }/d} vision_pipeline/component/work/VIDEO_KIT_TOP/VIDEO_KIT_TOP.v
    # Remove the lines containing wrong assignment to AXI data due to Libero bug, if there are any
    exec sed -i {/assign COREAXI4INTERCONNECT_C0_0_AXI4mslave0_WDATA =.*448.*/d} vision_pipeline/component/work/VIDEO_KIT_TOP/VIDEO_KIT_TOP.v
    exec sed -i {/assign COREAXI4INTERCONNECT_C0_0_AXI4mslave0_WSTRB =.*56.*/d} vision_pipeline/component/work/VIDEO_KIT_TOP/VIDEO_KIT_TOP.v
} result]} {
    puts "****************************************************************
        ****************************************************************
        ERROR executing the sed command. 
        sed is not installed on windows by default
        You can download and install sed for windows at https://gnuwin32.sourceforge.net/downlinks/sed.php
        Once installed, you should update your PATH to include sed installation bin folder.
        By default it should be located at  C:/Program Files (x86)/GnuWin32/bin
        ****************************************************************
        ****************************************************************"
        exit 1
}
} else {
    # Remove empty concatenations if there are any
    exec /bin/sed -i {/assign COREAXI4INTERCONNECT_C0_0_AXI4mslave0_WSTRB = {  }/d} vision_pipeline/component/work/VIDEO_KIT_TOP/VIDEO_KIT_TOP.v
    exec /bin/sed -i {/assign COREAXI4INTERCONNECT_C0_0_AXI4mslave0_WDATA = {  }/d} vision_pipeline/component/work/VIDEO_KIT_TOP/VIDEO_KIT_TOP.v
    # Remove the lines containing wrong assignment to AXI data due to Libero bug, if there are any
    exec /bin/sed -i {/assign COREAXI4INTERCONNECT_C0_0_AXI4mslave0_WDATA =.*448.*/d} vision_pipeline/component/work/VIDEO_KIT_TOP/VIDEO_KIT_TOP.v
    exec /bin/sed -i {/assign COREAXI4INTERCONNECT_C0_0_AXI4mslave0_WSTRB =.*56.*/d} vision_pipeline/component/work/VIDEO_KIT_TOP/VIDEO_KIT_TOP.v
}

#Configure and run the tools Synthesize, Place and Route and Verify Timing
configure_tool -name {SYNTHESIZE} -params {ACTIVE_IMPLEMENTATION:synthesis} -params {AUTO_COMPILE_POINT:false} -params {BLOCK_MODE:false} -params {BLOCK_PLACEMENT_CONFLICTS:ERROR} -params {BLOCK_ROUTING_CONFLICTS:LOCK} -params {CDC_MIN_NUM_SYNC_REGS:2} -params {CDC_REPORT:true} -params {CLOCK_ASYNC:800} -params {CLOCK_DATA:5000} -params {CLOCK_GATE_ENABLE:false} -params {CLOCK_GATE_ENABLE_THRESHOLD_GLOBAL:1000} -params {CLOCK_GATE_ENABLE_THRESHOLD_ROW:100} -params {CLOCK_GLOBAL:2} -params {PA4_GB_COUNT:36} -params {PA4_GB_MAX_RCLKINT_INSERTION:16} -params {PA4_GB_MIN_GB_FANOUT_TO_USE_RCLKINT:1000} -params {RAM_OPTIMIZED_FOR_POWER:0} -params {RETIMING:true} -params {ROM_TO_LOGIC:false} -params {SEQSHIFT_TO_URAM:1} -params {SYNPLIFY_OPTIONS:} -params {SYNPLIFY_TCL_FILE:../src/synplify_options.tcl}  
run_tool -name {SYNTHESIZE}


configure_tool -name {PLACEROUTE} -params {DELAY_ANALYSIS:MAX} -params {EFFORT_LEVEL:false} -params {GB_DEMOTION:true} -params {INCRPLACEANDROUTE:false} -params {IOREG_COMBINING:true} -params {MULTI_PASS_CRITERIA:VIOLATIONS} -params {MULTI_PASS_LAYOUT:true} -params {NUM_MULTI_PASSES:5} -params {PDPR:false} -params {RANDOM_SEED:1} -params {REPAIR_MIN_DELAY:true} -params {REPLICATION:true} -params {SLACK_CRITERIA:WORST_SLACK} -params {SPECIFIC_CLOCK:} -params {START_SEED_INDEX:3} -params {STOP_ON_FIRST_PASS:true} -params {TDPR:true} 
run_tool -name {PLACEROUTE}

configure_tool -name {VERIFYTIMING} -params {CONSTRAINTS_COVERAGE:1} -params {FORMAT:TEXT} -params {MAX_EXPANDED_PATHS_TIMING:1} -params {MAX_EXPANDED_PATHS_VIOLATION:0} -params {MAX_PARALLEL_PATHS_TIMING:1} -params {MAX_PARALLEL_PATHS_VIOLATION:1} -params {MAX_PATHS_INTERACTIVE_REPORT:1} -params {MAX_PATHS_TIMING:5} -params {MAX_PATHS_VIOLATION:20} -params {MAX_TIMING_FAST_HV_LT:1} -params {MAX_TIMING_MULTI_CORNER:1} -params {MAX_TIMING_SLOW_LV_HT:1} -params {MAX_TIMING_SLOW_LV_LT:1} -params {MAX_TIMING_VIOLATIONS_FAST_HV_LT:1} -params {MAX_TIMING_VIOLATIONS_MULTI_CORNER:1} -params {MAX_TIMING_VIOLATIONS_SLOW_LV_HT:1} -params {MAX_TIMING_VIOLATIONS_SLOW_LV_LT:1} -params {MIN_TIMING_FAST_HV_LT:1} -params {MIN_TIMING_MULTI_CORNER:1} -params {MIN_TIMING_SLOW_LV_HT:1} -params {MIN_TIMING_SLOW_LV_LT:1} -params {MIN_TIMING_VIOLATIONS_FAST_HV_LT:1} -params {MIN_TIMING_VIOLATIONS_MULTI_CORNER:1} -params {MIN_TIMING_VIOLATIONS_SLOW_LV_HT:1} -params {MIN_TIMING_VIOLATIONS_SLOW_LV_LT:1} -params {SLACK_THRESHOLD_VIOLATION:0.0} -params {SMART_INTERACTIVE:0} 
run_tool -name {VERIFYTIMING}

run_tool -name {GENERATEPROGRAMMINGDATA} 

#Configure Design Initialization Data and Memories
configure_design_initialization_data -second_stage_start_address {0x00000000} -third_stage_uprom_start_address {0x00000000} -third_stage_snvm_start_address {0x00000000} -third_stage_spi_start_address {0x00000400} -third_stage_spi_type {SPIFLASH_NO_BINDING_PLAINTEXT} -third_stage_spi_clock_divider {2} -init_timeout 128 -auto_calib_timeout {3000} -broadcast_RAMs {1} 
configure_snvm -cfg_file {./src/cfg_and_mem_files/SNVM.cfg}
#The below file sources the *.hex file generated by the SoftConsole
configure_ram -cfg_file {./src/cfg_and_mem_files/RAM.cfg} 
save_project 

# generate_design_initialization_data 

configure_tool \
         -name {GENERATEPROGRAMMINGFILE} \
         -params {program_fabric:true} \
         -params {program_security:false} \
         -params {program_snvm:true} \
         -params {sanitize_snvm:false} 
run_tool -name {GENERATEPROGRAMMINGFILE} 

#Generate the Programming job file
export_prog_job \
         -job_file_name {VIDEO_KIT_TOP} \
         -export_dir {.} \
         -bitstream_file_type {TRUSTED_FACILITY} \
         -bitstream_file_components {FABRIC_SNVM} \
         -zeroization_likenew_action 0 \
         -zeroization_unrecoverable_action 0 \
         -program_design 1 \
         -program_spi_flash 0 \
         -include_plaintext_passkey 0 \
         -design_bitstream_format {PPD} \
         -prog_optional_procedures {} \
         -skip_recommended_procedures {} \
         -sanitize_snvm 0 

save_project
