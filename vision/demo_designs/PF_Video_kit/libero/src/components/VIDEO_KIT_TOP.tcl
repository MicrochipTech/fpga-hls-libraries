# Creating SmartDesign VIDEO_KIT_TOP
set sd_name {VIDEO_KIT_TOP}
create_smartdesign -sd_name ${sd_name}

# Disable auto promotion of pins of type 'pad'
auto_promote_pad_pins -promote_all 0

# Create top level Scalar Ports
sd_create_scalar_port -sd_name ${sd_name} -port_name {CAM1_RX_CLK_N} -port_direction {IN} -port_is_pad {1}
sd_create_scalar_port -sd_name ${sd_name} -port_name {CAM1_RX_CLK_P} -port_direction {IN} -port_is_pad {1}
sd_create_scalar_port -sd_name ${sd_name} -port_name {CAM2_RX_CLK_N} -port_direction {IN} -port_is_pad {1}
sd_create_scalar_port -sd_name ${sd_name} -port_name {CAM2_RX_CLK_P} -port_direction {IN} -port_is_pad {1}
sd_create_scalar_port -sd_name ${sd_name} -port_name {CLK_IN} -port_direction {IN}
sd_create_scalar_port -sd_name ${sd_name} -port_name {LANE0_RXD_N} -port_direction {IN} -port_is_pad {1}
sd_create_scalar_port -sd_name ${sd_name} -port_name {LANE0_RXD_P} -port_direction {IN} -port_is_pad {1}
sd_create_scalar_port -sd_name ${sd_name} -port_name {LANE1_RXD_N} -port_direction {IN} -port_is_pad {1}
sd_create_scalar_port -sd_name ${sd_name} -port_name {LANE1_RXD_P} -port_direction {IN} -port_is_pad {1}
sd_create_scalar_port -sd_name ${sd_name} -port_name {LANE2_RXD_N} -port_direction {IN} -port_is_pad {1}
sd_create_scalar_port -sd_name ${sd_name} -port_name {LANE2_RXD_P} -port_direction {IN} -port_is_pad {1}
sd_create_scalar_port -sd_name ${sd_name} -port_name {LANE3_RXD_N} -port_direction {IN} -port_is_pad {1}
sd_create_scalar_port -sd_name ${sd_name} -port_name {LANE3_RXD_P} -port_direction {IN} -port_is_pad {1}
sd_create_scalar_port -sd_name ${sd_name} -port_name {REF_CLK_PAD_N} -port_direction {IN} -port_is_pad {1}
sd_create_scalar_port -sd_name ${sd_name} -port_name {REF_CLK_PAD_P} -port_direction {IN} -port_is_pad {1}
sd_create_scalar_port -sd_name ${sd_name} -port_name {TCK} -port_direction {IN}
sd_create_scalar_port -sd_name ${sd_name} -port_name {TDI} -port_direction {IN}
sd_create_scalar_port -sd_name ${sd_name} -port_name {TMS} -port_direction {IN}
sd_create_scalar_port -sd_name ${sd_name} -port_name {TRSTB} -port_direction {IN}

sd_create_scalar_port -sd_name ${sd_name} -port_name {ACT_N} -port_direction {OUT} -port_is_pad {1}
sd_create_scalar_port -sd_name ${sd_name} -port_name {BG} -port_direction {OUT} -port_is_pad {1}
sd_create_scalar_port -sd_name ${sd_name} -port_name {CAM1_RST} -port_direction {OUT}
sd_create_scalar_port -sd_name ${sd_name} -port_name {CAM1_XMASTER} -port_direction {OUT}
sd_create_scalar_port -sd_name ${sd_name} -port_name {CAM2_RST} -port_direction {OUT}
sd_create_scalar_port -sd_name ${sd_name} -port_name {CAM2_XMASTER} -port_direction {OUT}
sd_create_scalar_port -sd_name ${sd_name} -port_name {CAMERA_CLK} -port_direction {OUT}
sd_create_scalar_port -sd_name ${sd_name} -port_name {CAM_CLK_EN} -port_direction {OUT}
sd_create_scalar_port -sd_name ${sd_name} -port_name {CAM_INCK} -port_direction {OUT}
sd_create_scalar_port -sd_name ${sd_name} -port_name {CAS_N} -port_direction {OUT} -port_is_pad {1}
sd_create_scalar_port -sd_name ${sd_name} -port_name {CK0_N} -port_direction {OUT} -port_is_pad {1}
sd_create_scalar_port -sd_name ${sd_name} -port_name {CK0} -port_direction {OUT} -port_is_pad {1}
sd_create_scalar_port -sd_name ${sd_name} -port_name {CKE} -port_direction {OUT} -port_is_pad {1}
sd_create_scalar_port -sd_name ${sd_name} -port_name {CS_N} -port_direction {OUT} -port_is_pad {1}
sd_create_scalar_port -sd_name ${sd_name} -port_name {HDMIO_PD} -port_direction {OUT}
sd_create_scalar_port -sd_name ${sd_name} -port_name {LANE0_TXD_N} -port_direction {OUT} -port_is_pad {1}
sd_create_scalar_port -sd_name ${sd_name} -port_name {LANE0_TXD_P} -port_direction {OUT} -port_is_pad {1}
sd_create_scalar_port -sd_name ${sd_name} -port_name {LANE1_TXD_N} -port_direction {OUT} -port_is_pad {1}
sd_create_scalar_port -sd_name ${sd_name} -port_name {LANE1_TXD_P} -port_direction {OUT} -port_is_pad {1}
sd_create_scalar_port -sd_name ${sd_name} -port_name {LANE2_TXD_N} -port_direction {OUT} -port_is_pad {1}
sd_create_scalar_port -sd_name ${sd_name} -port_name {LANE2_TXD_P} -port_direction {OUT} -port_is_pad {1}
sd_create_scalar_port -sd_name ${sd_name} -port_name {LANE3_TXD_N} -port_direction {OUT} -port_is_pad {1}
sd_create_scalar_port -sd_name ${sd_name} -port_name {LANE3_TXD_P} -port_direction {OUT} -port_is_pad {1}
sd_create_scalar_port -sd_name ${sd_name} -port_name {ODT} -port_direction {OUT} -port_is_pad {1}
sd_create_scalar_port -sd_name ${sd_name} -port_name {RAS_N} -port_direction {OUT} -port_is_pad {1}
sd_create_scalar_port -sd_name ${sd_name} -port_name {RESET_N} -port_direction {OUT} -port_is_pad {1}
sd_create_scalar_port -sd_name ${sd_name} -port_name {SHIELD0} -port_direction {OUT} -port_is_pad {1}
sd_create_scalar_port -sd_name ${sd_name} -port_name {SHIELD1} -port_direction {OUT} -port_is_pad {1}
sd_create_scalar_port -sd_name ${sd_name} -port_name {SHIELD2} -port_direction {OUT} -port_is_pad {1}
sd_create_scalar_port -sd_name ${sd_name} -port_name {SHIELD3} -port_direction {OUT} -port_is_pad {1}
sd_create_scalar_port -sd_name ${sd_name} -port_name {SHIELD4} -port_direction {OUT} -port_is_pad {1}
sd_create_scalar_port -sd_name ${sd_name} -port_name {SHIELD5} -port_direction {OUT} -port_is_pad {1}
sd_create_scalar_port -sd_name ${sd_name} -port_name {SHIELD6} -port_direction {OUT} -port_is_pad {1}
sd_create_scalar_port -sd_name ${sd_name} -port_name {SHIELD7} -port_direction {OUT} -port_is_pad {1}
sd_create_scalar_port -sd_name ${sd_name} -port_name {TDO} -port_direction {OUT}
sd_create_scalar_port -sd_name ${sd_name} -port_name {WE_N} -port_direction {OUT} -port_is_pad {1}
sd_create_scalar_port -sd_name ${sd_name} -port_name {hdmi_clk} -port_direction {OUT}
sd_create_scalar_port -sd_name ${sd_name} -port_name {line_valid_o} -port_direction {OUT}

sd_create_scalar_port -sd_name ${sd_name} -port_name {CAM1_SCL} -port_direction {INOUT} -port_is_pad {1}
sd_create_scalar_port -sd_name ${sd_name} -port_name {CAM1_SDA} -port_direction {INOUT} -port_is_pad {1}
sd_create_scalar_port -sd_name ${sd_name} -port_name {CAM2_SCL} -port_direction {INOUT} -port_is_pad {1}
sd_create_scalar_port -sd_name ${sd_name} -port_name {CAM2_SDA} -port_direction {INOUT} -port_is_pad {1}
sd_create_scalar_port -sd_name ${sd_name} -port_name {HDMI_SCL} -port_direction {INOUT} -port_is_pad {1}
sd_create_scalar_port -sd_name ${sd_name} -port_name {HDMI_SDA} -port_direction {INOUT} -port_is_pad {1}
sd_create_scalar_port -sd_name ${sd_name} -port_name {SDA} -port_direction {INOUT} -port_is_pad {1}

# Create top level Bus Ports
sd_create_bus_port -sd_name ${sd_name} -port_name {CAM1_RXD_N} -port_direction {IN} -port_range {[3:0]} -port_is_pad {1}
sd_create_bus_port -sd_name ${sd_name} -port_name {CAM1_RXD} -port_direction {IN} -port_range {[3:0]} -port_is_pad {1}
sd_create_bus_port -sd_name ${sd_name} -port_name {CAM2_RXD_N} -port_direction {IN} -port_range {[3:0]} -port_is_pad {1}
sd_create_bus_port -sd_name ${sd_name} -port_name {CAM2_RXD} -port_direction {IN} -port_range {[3:0]} -port_is_pad {1}

sd_create_bus_port -sd_name ${sd_name} -port_name {A} -port_direction {OUT} -port_range {[13:0]} -port_is_pad {1}
sd_create_bus_port -sd_name ${sd_name} -port_name {BA} -port_direction {OUT} -port_range {[1:0]} -port_is_pad {1}
sd_create_bus_port -sd_name ${sd_name} -port_name {DM_N} -port_direction {OUT} -port_range {[7:0]} -port_is_pad {1}

sd_create_bus_port -sd_name ${sd_name} -port_name {DQS_N} -port_direction {INOUT} -port_range {[7:0]} -port_is_pad {1}
sd_create_bus_port -sd_name ${sd_name} -port_name {DQS} -port_direction {INOUT} -port_range {[7:0]} -port_is_pad {1}
sd_create_bus_port -sd_name ${sd_name} -port_name {DQ} -port_direction {INOUT} -port_range {[63:0]} -port_is_pad {1}

sd_connect_pins_to_constant -sd_name ${sd_name} -pin_names {CAM1_XMASTER} -value {GND}
sd_connect_pins_to_constant -sd_name ${sd_name} -pin_names {CAM2_XMASTER} -value {GND}
sd_connect_pins_to_constant -sd_name ${sd_name} -pin_names {CAM_INCK} -value {GND}
sd_connect_pins_to_constant -sd_name ${sd_name} -pin_names {HDMIO_PD} -value {GND}
# Add AND2_0 instance
sd_instantiate_macro -sd_name ${sd_name} -macro_name {AND2} -instance_name {AND2_0}



# Add AND4_0 instance
sd_instantiate_macro -sd_name ${sd_name} -macro_name {AND4} -instance_name {AND4_0}



# Add CCC_0 instance
sd_instantiate_component -sd_name ${sd_name} -component_name {CCC} -instance_name {CCC_0}



# Add COREAXI4INTERCONNECT_C0_0 instance
sd_instantiate_component -sd_name ${sd_name} -component_name {COREAXI4INTERCONNECT_C0} -instance_name {COREAXI4INTERCONNECT_C0_0}



# Add DDR_Access_wrapper_top_0 instance
sd_instantiate_hdl_core -sd_name ${sd_name} -hdl_core_name {DDR_Access_wrapper_top} -instance_name {DDR_Access_wrapper_top_0}
# Exporting Parameters of instance DDR_Access_wrapper_top_0
sd_configure_core_instance -sd_name ${sd_name} -instance_name {DDR_Access_wrapper_top_0} -params {\
"ADDR_WIDTH:32" \
"AXI_DATA_WIDTH:64" \
"AXI_ID_WIDTH:1" }\
-validate_rules 0
sd_save_core_instance_config -sd_name ${sd_name} -instance_name {DDR_Access_wrapper_top_0}
sd_update_instance -sd_name ${sd_name} -instance_name {DDR_Access_wrapper_top_0}
sd_invert_pins -sd_name ${sd_name} -pin_names {DDR_Access_wrapper_top_0:reset}
sd_connect_pins_to_constant -sd_name ${sd_name} -pin_names {DDR_Access_wrapper_top_0:start_write} -value {VCC}
sd_connect_pins_to_constant -sd_name ${sd_name} -pin_names {DDR_Access_wrapper_top_0:start_read} -value {VCC}
sd_connect_pins_to_constant -sd_name ${sd_name} -pin_names {DDR_Access_wrapper_top_0:buf_var} -value {01100000000000000000000000000000}
sd_connect_pins_to_constant -sd_name ${sd_name} -pin_names {DDR_Access_wrapper_top_0:hres} -value {00000000000000000000111100000000}
sd_connect_pins_to_constant -sd_name ${sd_name} -pin_names {DDR_Access_wrapper_top_0:vres} -value {00000000000000000000100001110000}



# Add HDMI instance
sd_instantiate_component -sd_name ${sd_name} -component_name {HDMI_2p0} -instance_name {HDMI}
sd_create_pin_group -sd_name ${sd_name} -group_name {PADs_IN} -instance_name {HDMI} -pin_names {"LANE0_RXD_P" "LANE0_RXD_N" "LANE1_RXD_P" "LANE1_RXD_N" "LANE2_RXD_P" "LANE2_RXD_N" "LANE3_RXD_P" "LANE3_RXD_N" }
sd_create_pin_group -sd_name ${sd_name} -group_name {PADs_OUT} -instance_name {HDMI} -pin_names {"LANE0_TXD_P" "LANE0_TXD_N" "LANE1_TXD_P" "LANE1_TXD_N" "LANE2_TXD_P" "LANE2_TXD_N" "LANE3_TXD_P" "LANE3_TXD_N" }



# Add IMX334_IF_TOP_0 instance
sd_instantiate_component -sd_name ${sd_name} -component_name {IMX334_IF_TOP} -instance_name {IMX334_IF_TOP_0}



# Add PF_DDR4_C0_0 instance
sd_instantiate_component -sd_name ${sd_name} -component_name {PF_DDR4_C0} -instance_name {PF_DDR4_C0_0}
sd_create_pin_slices -sd_name ${sd_name} -pin_name {PF_DDR4_C0_0:axi0_wdata} -pin_slices {[511:64]}
sd_connect_pins_to_constant -sd_name ${sd_name} -pin_names {PF_DDR4_C0_0:axi0_wdata[511:64]} -value {GND}
sd_create_pin_slices -sd_name ${sd_name} -pin_name {PF_DDR4_C0_0:axi0_wdata} -pin_slices {[63:0]}
sd_create_pin_slices -sd_name ${sd_name} -pin_name {PF_DDR4_C0_0:axi0_wstrb} -pin_slices {[63:8]}
sd_connect_pins_to_constant -sd_name ${sd_name} -pin_names {PF_DDR4_C0_0:axi0_wstrb[63:8]} -value {GND}
sd_create_pin_slices -sd_name ${sd_name} -pin_name {PF_DDR4_C0_0:axi0_wstrb} -pin_slices {[7:0]}



# Add PROC_SUBSYSTEM_0 instance
sd_instantiate_component -sd_name ${sd_name} -component_name {PROC_SUBSYSTEM} -instance_name {PROC_SUBSYSTEM_0}
sd_create_pin_slices -sd_name ${sd_name} -pin_name {PROC_SUBSYSTEM_0:GPIO_OUT_0} -pin_slices {[0:0]}
sd_create_pin_slices -sd_name ${sd_name} -pin_name {PROC_SUBSYSTEM_0:GPIO_OUT_0} -pin_slices {[3:1]}
sd_mark_pins_unused -sd_name ${sd_name} -pin_names {PROC_SUBSYSTEM_0:GPIO_OUT_0[3:1]}
sd_mark_pins_unused -sd_name ${sd_name} -pin_names {PROC_SUBSYSTEM_0:HDMI_RST}



# Add Reset_0 instance
sd_instantiate_component -sd_name ${sd_name} -component_name {Reset} -instance_name {Reset_0}



# Add VideoPipelineTop_top_0 instance
sd_instantiate_hdl_core -sd_name ${sd_name} -hdl_core_name {VideoPipelineTop_top} -instance_name {VideoPipelineTop_top_0}
sd_invert_pins -sd_name ${sd_name} -pin_names {VideoPipelineTop_top_0:reset}
sd_connect_pins_to_constant -sd_name ${sd_name} -pin_names {VideoPipelineTop_top_0:start} -value {VCC}
sd_connect_pins_to_constant -sd_name ${sd_name} -pin_names {VideoPipelineTop_top_0:BayerFormat} -value {00}



# Add scalar net connections
sd_connect_pins -sd_name ${sd_name} -pin_names {"ACT_N" "PF_DDR4_C0_0:ACT_N" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"AND2_0:A" "Reset_0:DEVICE_INIT_DONE" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"AND2_0:B" "AND4_0:A" "CCC_0:PLL_LOCK_0" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"AND2_0:Y" "Reset_0:PLL_LOCK_2" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"AND4_0:B" "CCC_0:PLL_LOCK_0_0" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"AND4_0:C" "PF_DDR4_C0_0:PLL_LOCK" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"AND4_0:D" "PF_DDR4_C0_0:CTRLR_READY" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"AND4_0:Y" "Reset_0:PLL_LOCK_0" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"BG" "PF_DDR4_C0_0:BG" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"CAM1_RST" "PROC_SUBSYSTEM_0:CAM1_RST" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"CAM1_RX_CLK_N" "IMX334_IF_TOP_0:CAM1_RX_CLK_N" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"CAM1_RX_CLK_P" "IMX334_IF_TOP_0:CAM1_RX_CLK_P" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"CAM1_SCL" "PROC_SUBSYSTEM_0:CAM1_SCL" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"CAM1_SDA" "PROC_SUBSYSTEM_0:CAM1_SDA" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"CAM2_RST" "PROC_SUBSYSTEM_0:CAM2_RST" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"CAM2_RX_CLK_N" "IMX334_IF_TOP_0:CAM2_RX_CLK_N" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"CAM2_RX_CLK_P" "IMX334_IF_TOP_0:CAM2_RX_CLK_P" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"CAM2_SCL" "PROC_SUBSYSTEM_0:CAM2_SCL" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"CAM2_SDA" "PROC_SUBSYSTEM_0:CAM2_SDA" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"CAMERA_CLK" "IMX334_IF_TOP_0:CAMERA_CLK" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"CAM_CLK_EN" "PROC_SUBSYSTEM_0:GPIO_OUT" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"CAS_N" "PF_DDR4_C0_0:CAS_N" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"CCC_0:OUT0_FABCLK_0_0" "Reset_0:CLK_1" "hdmi_clk" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"CCC_0:OUT1_FABCLK_0_0" "PF_DDR4_C0_0:PLL_REF_CLK" "PROC_SUBSYSTEM_0:PCLK" "Reset_0:CLK_2" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"CCC_0:REF_CLK_0" "CLK_IN" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"CCC_0:REF_CLK_0_0" "COREAXI4INTERCONNECT_C0_0:ACLK" "PF_DDR4_C0_0:SYS_CLK" "Reset_0:CLK_0" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"CK0" "PF_DDR4_C0_0:CK0" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"CK0_N" "PF_DDR4_C0_0:CK0_N" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"CKE" "PF_DDR4_C0_0:CKE" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"COREAXI4INTERCONNECT_C0_0:ARESETN" "Reset_0:FABRIC_RESET_N" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"COREAXI4INTERCONNECT_C0_0:M_CLK0" "DDR_Access_wrapper_top_0:clk" "HDMI:LANE0_TX_CLK_R" "IMX334_IF_TOP_0:i_axis_clk" "Reset_0:CLK" "VideoPipelineTop_top_0:clk" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"CS_N" "PF_DDR4_C0_0:CS_N" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"DDR_Access_wrapper_top_0:reset" "HDMI:RESET_N_I_0" "IMX334_IF_TOP_0:i_axis_reset" "Reset_0:FABRIC_RESET_N_0" "VideoPipelineTop_top_0:reset" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"HDMI:LANE0_RXD_N" "LANE0_RXD_N" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"HDMI:LANE0_RXD_P" "LANE0_RXD_P" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"HDMI:LANE0_TXD_N" "LANE0_TXD_N" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"HDMI:LANE0_TXD_P" "LANE0_TXD_P" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"HDMI:LANE1_RXD_N" "LANE1_RXD_N" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"HDMI:LANE1_RXD_P" "LANE1_RXD_P" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"HDMI:LANE1_TXD_N" "LANE1_TXD_N" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"HDMI:LANE1_TXD_P" "LANE1_TXD_P" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"HDMI:LANE2_RXD_N" "LANE2_RXD_N" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"HDMI:LANE2_RXD_P" "LANE2_RXD_P" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"HDMI:LANE2_TXD_N" "LANE2_TXD_N" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"HDMI:LANE2_TXD_P" "LANE2_TXD_P" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"HDMI:LANE3_RXD_N" "LANE3_RXD_N" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"HDMI:LANE3_RXD_P" "LANE3_RXD_P" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"HDMI:LANE3_TXD_N" "LANE3_TXD_N" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"HDMI:LANE3_TXD_P" "LANE3_TXD_P" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"HDMI:REF_CLK_PAD_N" "REF_CLK_PAD_N" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"HDMI:REF_CLK_PAD_P" "REF_CLK_PAD_P" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"HDMI:SDA" "SDA" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"HDMI_SCL" "PROC_SUBSYSTEM_0:HDMI_SCL" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"HDMI_SDA" "PROC_SUBSYSTEM_0:HDMI_SDA" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"IMX334_IF_TOP_0:ARST_N" "IMX334_IF_TOP_0:INIT_DONE" "Reset_0:AUTOCALIB_DONE" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"IMX334_IF_TOP_0:TRNG_RST_N" "PROC_SUBSYSTEM_0:TRNG_RST_N" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"IMX334_IF_TOP_0:c1_line_valid_o" "line_valid_o" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"ODT" "PF_DDR4_C0_0:ODT" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"PF_DDR4_C0_0:RAS_N" "RAS_N" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"PF_DDR4_C0_0:RESET_N" "RESET_N" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"PF_DDR4_C0_0:SHIELD0" "SHIELD0" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"PF_DDR4_C0_0:SHIELD1" "SHIELD1" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"PF_DDR4_C0_0:SHIELD2" "SHIELD2" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"PF_DDR4_C0_0:SHIELD3" "SHIELD3" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"PF_DDR4_C0_0:SHIELD4" "SHIELD4" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"PF_DDR4_C0_0:SHIELD5" "SHIELD5" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"PF_DDR4_C0_0:SHIELD6" "SHIELD6" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"PF_DDR4_C0_0:SHIELD7" "SHIELD7" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"PF_DDR4_C0_0:SYS_RESET_N" "PROC_SUBSYSTEM_0:reset" "Reset_0:FABRIC_RESET_N_2" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"PF_DDR4_C0_0:WE_N" "WE_N" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"PROC_SUBSYSTEM_0:TCK" "TCK" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"PROC_SUBSYSTEM_0:TDI" "TDI" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"PROC_SUBSYSTEM_0:TDO" "TDO" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"PROC_SUBSYSTEM_0:TMS" "TMS" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"PROC_SUBSYSTEM_0:TRSTB" "TRSTB" }

# Add bus net connections
sd_connect_pins -sd_name ${sd_name} -pin_names {"A" "PF_DDR4_C0_0:A" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"BA" "PF_DDR4_C0_0:BA" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"CAM1_RXD" "IMX334_IF_TOP_0:CAM1_RXD" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"CAM1_RXD_N" "IMX334_IF_TOP_0:CAM1_RXD_N" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"CAM2_RXD" "IMX334_IF_TOP_0:CAM2_RXD" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"CAM2_RXD_N" "IMX334_IF_TOP_0:CAM2_RXD_N" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"DM_N" "PF_DDR4_C0_0:DM_N" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"DQ" "PF_DDR4_C0_0:DQ" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"DQS" "PF_DDR4_C0_0:DQS" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"DQS_N" "PF_DDR4_C0_0:DQS_N" }

# Add bus interface net connections
sd_connect_pins -sd_name ${sd_name} -pin_names {"COREAXI4INTERCONNECT_C0_0:AXI4mmaster0" "DDR_Access_wrapper_top_0:axi4initiator" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"COREAXI4INTERCONNECT_C0_0:AXI4mslave0" "PF_DDR4_C0_0:AXI4slave0" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"DDR_Access_wrapper_top_0:VideoIn_axi4stream" "IMX334_IF_TOP_0:AXIS" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"DDR_Access_wrapper_top_0:VideoOut_axi4stream" "VideoPipelineTop_top_0:VideoIn_axi4stream" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"HDMI:AXIS" "VideoPipelineTop_top_0:VideoOut_axi4stream" }

# Re-enable auto promotion of pins of type 'pad'
auto_promote_pad_pins -promote_all 1
# Save the smartDesign
save_smartdesign -sd_name ${sd_name}
# Generate SmartDesign VIDEO_KIT_TOP
generate_component -component_name ${sd_name}
