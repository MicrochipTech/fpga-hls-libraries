# Set IP core version variables
source ../../ip_core_versions.tcl

#Download all the required cores to the vault
download_core -vlnv "Microsemi:SolutionCore:HDMI_TX:${HDMI_TX_version}" -location {www.microchip-ip.com/repositories/DirectCore}
download_core -vlnv "Actel:SgCore:PF_TX_PLL:${PF_TX_PLL_version}" -location {www.microchip-ip.com/repositories/SgCore}
download_core -vlnv "Actel:SystemBuilder:PF_XCVR_ERM:${PF_XCVR_ERM_version}" -location {www.microchip-ip.com/repositories/SgCore}
download_core -vlnv "Actel:SgCore:PF_XCVR_REF_CLK:${PF_XCVR_REF_CLK_version}" -location {www.microchip-ip.com/repositories/SgCore}


#This Tcl file sources other Tcl files to build the design(on which recursive export is run) in a bottom-up fashion

#Sourcing the Tcl file in which all the HDL source files used in the design are imported or linked
source hdl_source.tcl
build_design_hierarchy

#Sourcing the Tcl files in which HDL+ core definitions are created for HDL modules
source components/AXIS_To_VGA_Converter.tcl 
build_design_hierarchy

#Sourcing the Tcl files for creating individual components under the top level
source components/HDMI_TX_C0.tcl 
source components/PF_TX_PLL_C0.tcl 
source components/PF_XCVR_ERM_C0.tcl 
source components/PF_XCVR_REF_CLK_C0.tcl 
source components/HDMI_2p0.tcl 
build_design_hierarchy
