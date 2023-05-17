# Set IP core version variables
source ../../ip_core_versions.tcl

# uncomment the lines below to add the extra camera top pins
# set Extra_camera_pins 1
# global Extra_camera_pins


#Download all the required cores to the vault
download_core -vlnv "Actel:SgCore:PF_CCC:${PF_CCC_version}" -location {www.microchip-ip.com/repositories/SgCore}
download_core -vlnv "Actel:DirectCore:CORERESET_PF:${CORERESET_PF_version}" -location {www.microchip-ip.com/repositories/DirectCore}
download_core -vlnv "Actel:DirectCore:CORERXIODBITALIGN:${CORERXIODBITALIGN_version}" -location {www.microchip-ip.com/repositories/DirectCore}
download_core -vlnv "Actel:SystemBuilder:PF_IOD_GENERIC_RX:${PF_IOD_GENERIC_RX_version}" -location {www.microchip-ip.com/repositories/SgCore}
download_core -vlnv "Microsemi:SolutionCore:mipicsi2rxdecoderPF:${mipicsi2rxdecoderPF_version}" -location {www.microchip-ip.com/repositories/DirectCore}
download_core -vlnv "Actel:DirectCore:COREAHBTOAPB3:${COREAHBTOAPB3_version}" -location {www.microchip-ip.com/repositories/DirectCore}

#This Tcl file sources other Tcl files to build the design(on which recursive export is run) in a bottom-up fashion

#Sourcing the Tcl file in which all the HDL source files used in the design are imported or linked
source hdl_source.tcl
build_design_hierarchy

#Sourcing the Tcl files in which HDL+ core definitions are created for HDL modules
source components/Camera_To_AXIS_Converter.tcl 
build_design_hierarchy

#Sourcing the Tcl files for creating individual components under the top level
source components/CORERESET_PF_C1.tcl 
source components/CORERXIODBITALIGN_C0.tcl 
source components/CORERXIODBITALIGN_C1.tcl 
source components/CORERXIODBITALIGN_C2.tcl 
source components/CORERXIODBITALIGN_C3.tcl 
source components/PF_IOD_GENERIC_RX_C0.tcl 
source components/CAM_IOD_TIP_TOP.tcl 
source components/CORERESET_PF_C2.tcl 
source components/CORERESET_PF_C3.tcl 
source components/PF_CCC_C2.tcl 
source components/mipicsi2rxdecoderPF_C0.tcl 
source components/IMX334_IF_TOP.tcl 
build_design_hierarchy
