#This Tcl file sources other Tcl files to build the design(on which recursive export is run) in a bottom-up fashion

#Sourcing the Tcl file in which all the HDL source files used in the design are imported or linked
source hdl_source.tcl
build_design_hierarchy


#Create camera and display modules
set cwd [pwd]
cd ../../../../rtl/camera_sd_component/IMX334_IF_TOP
source IMX334_IF_TOP_recursive.tcl
cd ../../display_sd_component/HDMI_2p0
source HDMI_2p0_recursive.tcl 
build_design_hierarchy
cd $cwd
create_smartdesign -sd_name temp
set_root -module {temp::work} 


#Sourcing the Tcl files in which HDL+ core definitions are created for HDL modules
#Create hls top Smart Design Modules
source ../../hls/hls_output/scripts/create_hdl_plus.tcl
source components/DDR_Access_wrapper_top.tcl 

#Sourcing the Tcl files for creating individual components under the top level
source components/PF_CCC_C0.tcl 
source components/PF_CCC_C1.tcl 
source components/CCC.tcl 
source components/COREAXI4INTERCONNECT_C0.tcl 
source components/PF_DDR4_C0.tcl 
source components/COREI2C_C0.tcl 
source components/CoreAHBLite_C0.tcl 
source components/MIV_RV32IMA_L1_AHB_C0.tcl 
source components/PF_SRAM_AHBL_AXI_C0.tcl 
source components/PROC_SUBSYSTEM.tcl 
source components/CORERESET_PF_C0.tcl 
source components/PF_INIT_MONITOR_C0.tcl 
source components/Reset.tcl 
source components/VIDEO_KIT_TOP.tcl 
build_design_hierarchy
