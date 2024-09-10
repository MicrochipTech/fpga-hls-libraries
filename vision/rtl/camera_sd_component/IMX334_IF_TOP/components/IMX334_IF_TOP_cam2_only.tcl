# Creating SmartDesign IMX334_IF_TOP
set sd_name {IMX334_IF_TOP}
create_smartdesign -sd_name ${sd_name}

# Disable auto promotion of pins of type 'pad'
auto_promote_pad_pins -promote_all 0

# Create top level Scalar Ports
sd_create_scalar_port -sd_name ${sd_name} -port_name {ARST_N} -port_direction {IN}
sd_create_scalar_port -sd_name ${sd_name} -port_name {AXIS_i_tready} -port_direction {IN}
# sd_create_scalar_port -sd_name ${sd_name} -port_name {CAM1_RX_CLK_N} -port_direction {IN} -port_is_pad {1}
# sd_create_scalar_port -sd_name ${sd_name} -port_name {CAM1_RX_CLK_P} -port_direction {IN} -port_is_pad {1}
sd_create_scalar_port -sd_name ${sd_name} -port_name {CAM2_RX_CLK_N} -port_direction {IN} -port_is_pad {1}
sd_create_scalar_port -sd_name ${sd_name} -port_name {CAM2_RX_CLK_P} -port_direction {IN} -port_is_pad {1}
sd_create_scalar_port -sd_name ${sd_name} -port_name {INIT_DONE} -port_direction {IN}
sd_create_scalar_port -sd_name ${sd_name} -port_name {TRNG_RST_N} -port_direction {IN}
sd_create_scalar_port -sd_name ${sd_name} -port_name {i_axis_clk} -port_direction {IN}
sd_create_scalar_port -sd_name ${sd_name} -port_name {i_axis_resetn} -port_direction {IN}

sd_create_scalar_port -sd_name ${sd_name} -port_name {AXIS_o_tlast} -port_direction {OUT}
sd_create_scalar_port -sd_name ${sd_name} -port_name {AXIS_o_tvalid} -port_direction {OUT}

# Create top level Bus Ports
sd_create_bus_port -sd_name ${sd_name} -port_name {CAM2_RXD_N} -port_direction {IN} -port_range {[3:0]} -port_is_pad {1}
sd_create_bus_port -sd_name ${sd_name} -port_name {CAM2_RXD} -port_direction {IN} -port_range {[3:0]} -port_is_pad {1}

sd_create_bus_port -sd_name ${sd_name} -port_name {AXIS_o_tdata} -port_direction {OUT} -port_range {[31:0]}
sd_create_bus_port -sd_name ${sd_name} -port_name {AXIS_o_tuser} -port_direction {OUT} -port_range {[0:0]}


# Create top level Bus interface Ports
sd_create_bif_port -sd_name ${sd_name} -port_name {AXIS} -port_bif_vlnv {AMBA:AMBA4:AXI4Stream:r0p0_1} -port_bif_role {master} -port_bif_mapping {\
"TVALID:AXIS_o_tvalid" \
"TREADY:AXIS_i_tready" \
"TDATA:AXIS_o_tdata" \
"TLAST:AXIS_o_tlast" \
"TUSER:AXIS_o_tuser" } 

# Add AND2_0 instance
sd_instantiate_macro -sd_name ${sd_name} -macro_name {AND2} -instance_name {AND2_0}

# Add Camera_To_AXIS_Converter_0 instance
sd_instantiate_hdl_core -sd_name ${sd_name} -hdl_core_name {Camera_To_AXIS_Converter} -instance_name {Camera_To_AXIS_Converter_0}
# Exporting Parameters of instance Camera_To_AXIS_Converter_0
sd_configure_core_instance -sd_name ${sd_name} -instance_name {Camera_To_AXIS_Converter_0} -params {\
"C_WIDTH:8" \
"DEBUBBLE:0" \
"FIFO_DEPTH:4096" \
"LINES_TO_SKIP:1" \
"PIXEL_PER_CLK:4" \
"TUSER_WIDTH:1" }\
-validate_rules 0
sd_save_core_instance_config -sd_name ${sd_name} -instance_name {Camera_To_AXIS_Converter_0}
sd_update_instance -sd_name ${sd_name} -instance_name {Camera_To_AXIS_Converter_0}
sd_create_pin_slices -sd_name ${sd_name} -pin_name {Camera_To_AXIS_Converter_0:i_Data} -pin_slices {[15:8]}
sd_create_pin_slices -sd_name ${sd_name} -pin_name {Camera_To_AXIS_Converter_0:i_Data} -pin_slices {[23:16]}
sd_create_pin_slices -sd_name ${sd_name} -pin_name {Camera_To_AXIS_Converter_0:i_Data} -pin_slices {[31:24]}
sd_create_pin_slices -sd_name ${sd_name} -pin_name {Camera_To_AXIS_Converter_0:i_Data} -pin_slices {[7:0]}
sd_invert_pins -sd_name ${sd_name} -pin_names {Camera_To_AXIS_Converter_0:i_video_reset}
sd_invert_pins -sd_name ${sd_name} -pin_names {Camera_To_AXIS_Converter_0:i_axis_reset}
sd_connect_pins_to_constant -sd_name ${sd_name} -pin_names {Camera_To_AXIS_Converter_0:i_vres} -value {100001110000}
sd_connect_pins_to_constant -sd_name ${sd_name} -pin_names {Camera_To_AXIS_Converter_0:i_hres} -value {111100000000}



# Add CORERESET_PF_C1_0 instance
sd_instantiate_component -sd_name ${sd_name} -component_name {CORERESET_PF_C1} -instance_name {CORERESET_PF_C1_0}
sd_connect_pins_to_constant -sd_name ${sd_name} -pin_names {CORERESET_PF_C1_0:BANK_x_VDDI_STATUS} -value {VCC}
sd_connect_pins_to_constant -sd_name ${sd_name} -pin_names {CORERESET_PF_C1_0:BANK_y_VDDI_STATUS} -value {GND}
sd_connect_pins_to_constant -sd_name ${sd_name} -pin_names {CORERESET_PF_C1_0:SS_BUSY} -value {GND}
sd_connect_pins_to_constant -sd_name ${sd_name} -pin_names {CORERESET_PF_C1_0:FF_US_RESTORE} -value {GND}
sd_connect_pins_to_constant -sd_name ${sd_name} -pin_names {CORERESET_PF_C1_0:FPGA_POR_N} -value {VCC}
sd_mark_pins_unused -sd_name ${sd_name} -pin_names {CORERESET_PF_C1_0:PLL_POWERDOWN_B}

# Add CORERESET_PF_C3_0 instance
sd_instantiate_component -sd_name ${sd_name} -component_name {CORERESET_PF_C3} -instance_name {CORERESET_PF_C3_0}
sd_connect_pins_to_constant -sd_name ${sd_name} -pin_names {CORERESET_PF_C3_0:BANK_x_VDDI_STATUS} -value {VCC}
sd_connect_pins_to_constant -sd_name ${sd_name} -pin_names {CORERESET_PF_C3_0:BANK_y_VDDI_STATUS} -value {GND}
sd_connect_pins_to_constant -sd_name ${sd_name} -pin_names {CORERESET_PF_C3_0:SS_BUSY} -value {GND}
sd_connect_pins_to_constant -sd_name ${sd_name} -pin_names {CORERESET_PF_C3_0:FF_US_RESTORE} -value {GND}
sd_connect_pins_to_constant -sd_name ${sd_name} -pin_names {CORERESET_PF_C3_0:FPGA_POR_N} -value {VCC}
sd_mark_pins_unused -sd_name ${sd_name} -pin_names {CORERESET_PF_C3_0:PLL_POWERDOWN_B}

# Add CSI2_RXDecoder_1 instance
sd_instantiate_component -sd_name ${sd_name} -component_name {mipicsi2rxdecoderPF_C0} -instance_name {CSI2_RXDecoder_1}
sd_create_pin_slices -sd_name ${sd_name} -pin_name {CSI2_RXDecoder_1:data_out_o} -pin_slices {[19:12]}
sd_create_pin_slices -sd_name ${sd_name} -pin_name {CSI2_RXDecoder_1:data_out_o} -pin_slices {[29:22]}
sd_create_pin_slices -sd_name ${sd_name} -pin_name {CSI2_RXDecoder_1:data_out_o} -pin_slices {[39:32]}
sd_create_pin_slices -sd_name ${sd_name} -pin_name {CSI2_RXDecoder_1:data_out_o} -pin_slices {[9:2]}
sd_mark_pins_unused -sd_name ${sd_name} -pin_names {CSI2_RXDecoder_1:frame_end_o}
sd_mark_pins_unused -sd_name ${sd_name} -pin_names {CSI2_RXDecoder_1:line_end_o}
sd_mark_pins_unused -sd_name ${sd_name} -pin_names {CSI2_RXDecoder_1:line_start_o}
sd_mark_pins_unused -sd_name ${sd_name} -pin_names {CSI2_RXDecoder_1:word_count_o}



# Add PF_CCC_C2_0 instance
sd_instantiate_component -sd_name ${sd_name} -component_name {PF_CCC_C2} -instance_name {PF_CCC_C2_0}

# Add PF_IOD_GENERIC_RX_C0_1 instance
sd_instantiate_component -sd_name ${sd_name} -component_name {CAM_IOD_TIP_TOP} -instance_name {PF_IOD_GENERIC_RX_C0_1}
sd_connect_pins_to_constant -sd_name ${sd_name} -pin_names {PF_IOD_GENERIC_RX_C0_1:HS_IO_CLK_PAUSE} -value {GND}
sd_connect_pins_to_constant -sd_name ${sd_name} -pin_names {PF_IOD_GENERIC_RX_C0_1:HS_SEL} -value {VCC}
sd_connect_pins_to_constant -sd_name ${sd_name} -pin_names {PF_IOD_GENERIC_RX_C0_1:RESTART_TRNG} -value {GND}
sd_connect_pins_to_constant -sd_name ${sd_name} -pin_names {PF_IOD_GENERIC_RX_C0_1:SKIP_TRNG} -value {GND}
sd_mark_pins_unused -sd_name ${sd_name} -pin_names {PF_IOD_GENERIC_RX_C0_1:CLK_TRAIN_DONE}
sd_mark_pins_unused -sd_name ${sd_name} -pin_names {PF_IOD_GENERIC_RX_C0_1:CLK_TRAIN_ERROR}



# Add scalar net connections
sd_connect_pins -sd_name ${sd_name} -pin_names {"AND2_0:A" "TRNG_RST_N" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"AND2_0:B" "CORERESET_PF_C1_0:PLL_LOCK" "CORERESET_PF_C3_0:PLL_LOCK" "PF_CCC_C2_0:PLL_LOCK_0" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"AND2_0:Y" "PF_IOD_GENERIC_RX_C0_1:TRAINING_RESETN" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"PF_IOD_GENERIC_RX_C0_1:training_done_o" "CORERESET_PF_C1_0:EXT_RST_N" "CORERESET_PF_C3_0:EXT_RST_N" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"ARST_N" "PF_IOD_GENERIC_RX_C0_1:ARST_N" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"CAM2_RX_CLK_N" "PF_IOD_GENERIC_RX_C0_1:RX_CLK_N" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"CAM2_RX_CLK_P" "PF_IOD_GENERIC_RX_C0_1:RX_CLK_P" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"CORERESET_PF_C1_0:FABRIC_RESET_N" "Camera_To_AXIS_Converter_0:i_video_reset" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"CORERESET_PF_C1_0:CLK" "CSI2_RXDecoder_1:PARALLEL_CLOCK_I" "Camera_To_AXIS_Converter_0:i_video_clk" "PF_CCC_C2_0:OUT0_FABCLK_0" }

sd_connect_pins -sd_name ${sd_name} -pin_names {"CORERESET_PF_C1_0:INIT_DONE" "CORERESET_PF_C3_0:INIT_DONE" "INIT_DONE" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"PF_IOD_GENERIC_RX_C0_1:RX_CLK_G" \
    "CORERESET_PF_C3_0:CLK" "CSI2_RXDecoder_1:CAM_CLOCK_I" "PF_CCC_C2_0:REF_CLK_0"}
sd_connect_pins -sd_name ${sd_name} -pin_names {"CORERESET_PF_C3_0:FABRIC_RESET_N" "CSI2_RXDecoder_1:RESET_n_I" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"CSI2_RXDecoder_1:L0_LP_DATA_I" "PF_IOD_GENERIC_RX_C0_1:L0_LP_DATA" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"CSI2_RXDecoder_1:L0_LP_DATA_N_I" "PF_IOD_GENERIC_RX_C0_1:L0_LP_DATA_N" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"CSI2_RXDecoder_1:L1_LP_DATA_I" "PF_IOD_GENERIC_RX_C0_1:L1_LP_DATA" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"CSI2_RXDecoder_1:L1_LP_DATA_N_I" "PF_IOD_GENERIC_RX_C0_1:L1_LP_DATA_N" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"CSI2_RXDecoder_1:L2_LP_DATA_I" "PF_IOD_GENERIC_RX_C0_1:L2_LP_DATA" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"CSI2_RXDecoder_1:L2_LP_DATA_N_I" "PF_IOD_GENERIC_RX_C0_1:L2_LP_DATA_N" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"CSI2_RXDecoder_1:L3_LP_DATA_I" "PF_IOD_GENERIC_RX_C0_1:L3_LP_DATA" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"CSI2_RXDecoder_1:L3_LP_DATA_N_I" "PF_IOD_GENERIC_RX_C0_1:L3_LP_DATA_N" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"CSI2_RXDecoder_1:data_out_o[19:12]" "Camera_To_AXIS_Converter_0:i_Data[15:8]" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"CSI2_RXDecoder_1:data_out_o[29:22]" "Camera_To_AXIS_Converter_0:i_Data[23:16]" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"CSI2_RXDecoder_1:data_out_o[39:32]" "Camera_To_AXIS_Converter_0:i_Data[31:24]" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"CSI2_RXDecoder_1:data_out_o[9:2]" "Camera_To_AXIS_Converter_0:i_Data[7:0]" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"CSI2_RXDecoder_1:line_valid_o" "Camera_To_AXIS_Converter_0:i_data_valid" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"CSI2_RXDecoder_1:frame_start_o" "Camera_To_AXIS_Converter_0:i_frame_start" }

sd_connect_pins -sd_name ${sd_name} -pin_names {"Camera_To_AXIS_Converter_0:i_axis_clk" "i_axis_clk" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"i_axis_resetn" "Camera_To_AXIS_Converter_0:i_axis_reset"}

# Add bus net connections
sd_connect_pins -sd_name ${sd_name} -pin_names {"CAM2_RXD" "PF_IOD_GENERIC_RX_C0_1:RXD" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"CAM2_RXD_N" "PF_IOD_GENERIC_RX_C0_1:RXD_N" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"CSI2_RXDecoder_1:L0_HS_DATA_I" "PF_IOD_GENERIC_RX_C0_1:L0_RXD_DATA" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"CSI2_RXDecoder_1:L1_HS_DATA_I" "PF_IOD_GENERIC_RX_C0_1:L1_RXD_DATA" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"CSI2_RXDecoder_1:L2_HS_DATA_I" "PF_IOD_GENERIC_RX_C0_1:L2_RXD_DATA" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"CSI2_RXDecoder_1:L3_HS_DATA_I" "PF_IOD_GENERIC_RX_C0_1:L3_RXD_DATA" }

# Add bus interface net connections
sd_connect_pins -sd_name ${sd_name} -pin_names {"AXIS" "Camera_To_AXIS_Converter_0:AXIS" }

# Re-enable auto promotion of pins of type 'pad'
auto_promote_pad_pins -promote_all 1
# Save the smartDesign
save_smartdesign -sd_name ${sd_name}
# Generate SmartDesign IMX334_IF_TOP
generate_component -component_name ${sd_name}
