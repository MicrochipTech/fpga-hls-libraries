# Creating SmartDesign PROC_SUBSYSTEM
set sd_name {PROC_SUBSYSTEM}
create_smartdesign -sd_name ${sd_name}

# Disable auto promotion of pins of type 'pad'
auto_promote_pad_pins -promote_all 0

# Create top level Scalar Ports
sd_create_scalar_port -sd_name ${sd_name} -port_name {AXI4mslave0_SLAVE0_ARREADY} -port_direction {IN}
sd_create_scalar_port -sd_name ${sd_name} -port_name {AXI4mslave0_SLAVE0_AWREADY} -port_direction {IN}
sd_create_scalar_port -sd_name ${sd_name} -port_name {AXI4mslave0_SLAVE0_BVALID} -port_direction {IN}
sd_create_scalar_port -sd_name ${sd_name} -port_name {AXI4mslave0_SLAVE0_RLAST} -port_direction {IN}
sd_create_scalar_port -sd_name ${sd_name} -port_name {AXI4mslave0_SLAVE0_RVALID} -port_direction {IN}
sd_create_scalar_port -sd_name ${sd_name} -port_name {AXI4mslave0_SLAVE0_WREADY} -port_direction {IN}
sd_create_scalar_port -sd_name ${sd_name} -port_name {AXI4mslave0_SLAVE0_ARVALID} -port_direction {OUT}
sd_create_scalar_port -sd_name ${sd_name} -port_name {AXI4mslave0_SLAVE0_AWVALID} -port_direction {OUT}
sd_create_scalar_port -sd_name ${sd_name} -port_name {AXI4mslave0_SLAVE0_BREADY} -port_direction {OUT}
sd_create_scalar_port -sd_name ${sd_name} -port_name {AXI4mslave0_SLAVE0_RREADY} -port_direction {OUT}
sd_create_scalar_port -sd_name ${sd_name} -port_name {AXI4mslave0_SLAVE0_WLAST} -port_direction {OUT}
sd_create_scalar_port -sd_name ${sd_name} -port_name {AXI4mslave0_SLAVE0_WVALID} -port_direction {OUT}
sd_create_bus_port -sd_name ${sd_name} -port_name {AXI4mslave0_SLAVE0_BID} -port_direction {IN} -port_range {[1:0]}
sd_create_bus_port -sd_name ${sd_name} -port_name {AXI4mslave0_SLAVE0_BRESP} -port_direction {IN} -port_range {[1:0]}
sd_create_bus_port -sd_name ${sd_name} -port_name {AXI4mslave0_SLAVE0_BUSER} -port_direction {IN} -port_range {[0:0]}
sd_create_bus_port -sd_name ${sd_name} -port_name {AXI4mslave0_SLAVE0_RDATA} -port_direction {IN} -port_range {[63:0]}
sd_create_bus_port -sd_name ${sd_name} -port_name {AXI4mslave0_SLAVE0_RID} -port_direction {IN} -port_range {[1:0]}
sd_create_bus_port -sd_name ${sd_name} -port_name {AXI4mslave0_SLAVE0_RRESP} -port_direction {IN} -port_range {[1:0]}
sd_create_bus_port -sd_name ${sd_name} -port_name {AXI4mslave0_SLAVE0_RUSER} -port_direction {IN} -port_range {[0:0]}
sd_create_bus_port -sd_name ${sd_name} -port_name {AXI4mslave0_SLAVE0_ARADDR} -port_direction {OUT} -port_range {[31:0]}
sd_create_bus_port -sd_name ${sd_name} -port_name {AXI4mslave0_SLAVE0_ARBURST} -port_direction {OUT} -port_range {[1:0]}
sd_create_bus_port -sd_name ${sd_name} -port_name {AXI4mslave0_SLAVE0_ARCACHE} -port_direction {OUT} -port_range {[3:0]}
sd_create_bus_port -sd_name ${sd_name} -port_name {AXI4mslave0_SLAVE0_ARID} -port_direction {OUT} -port_range {[1:0]}
sd_create_bus_port -sd_name ${sd_name} -port_name {AXI4mslave0_SLAVE0_ARLEN} -port_direction {OUT} -port_range {[7:0]}
sd_create_bus_port -sd_name ${sd_name} -port_name {AXI4mslave0_SLAVE0_ARLOCK} -port_direction {OUT} -port_range {[1:0]}
sd_create_bus_port -sd_name ${sd_name} -port_name {AXI4mslave0_SLAVE0_ARPROT} -port_direction {OUT} -port_range {[2:0]}
sd_create_bus_port -sd_name ${sd_name} -port_name {AXI4mslave0_SLAVE0_ARQOS} -port_direction {OUT} -port_range {[3:0]}
sd_create_bus_port -sd_name ${sd_name} -port_name {AXI4mslave0_SLAVE0_ARREGION} -port_direction {OUT} -port_range {[3:0]}
sd_create_bus_port -sd_name ${sd_name} -port_name {AXI4mslave0_SLAVE0_ARSIZE} -port_direction {OUT} -port_range {[2:0]}
sd_create_bus_port -sd_name ${sd_name} -port_name {AXI4mslave0_SLAVE0_ARUSER} -port_direction {OUT} -port_range {[0:0]}
sd_create_bus_port -sd_name ${sd_name} -port_name {AXI4mslave0_SLAVE0_AWADDR} -port_direction {OUT} -port_range {[31:0]}
sd_create_bus_port -sd_name ${sd_name} -port_name {AXI4mslave0_SLAVE0_AWBURST} -port_direction {OUT} -port_range {[1:0]}
sd_create_bus_port -sd_name ${sd_name} -port_name {AXI4mslave0_SLAVE0_AWCACHE} -port_direction {OUT} -port_range {[3:0]}
sd_create_bus_port -sd_name ${sd_name} -port_name {AXI4mslave0_SLAVE0_AWID} -port_direction {OUT} -port_range {[1:0]}
sd_create_bus_port -sd_name ${sd_name} -port_name {AXI4mslave0_SLAVE0_AWLEN} -port_direction {OUT} -port_range {[7:0]}
sd_create_bus_port -sd_name ${sd_name} -port_name {AXI4mslave0_SLAVE0_AWLOCK} -port_direction {OUT} -port_range {[1:0]}
sd_create_bus_port -sd_name ${sd_name} -port_name {AXI4mslave0_SLAVE0_AWPROT} -port_direction {OUT} -port_range {[2:0]}
sd_create_bus_port -sd_name ${sd_name} -port_name {AXI4mslave0_SLAVE0_AWQOS} -port_direction {OUT} -port_range {[3:0]}
sd_create_bus_port -sd_name ${sd_name} -port_name {AXI4mslave0_SLAVE0_AWREGION} -port_direction {OUT} -port_range {[3:0]}
sd_create_bus_port -sd_name ${sd_name} -port_name {AXI4mslave0_SLAVE0_AWSIZE} -port_direction {OUT} -port_range {[2:0]}
sd_create_bus_port -sd_name ${sd_name} -port_name {AXI4mslave0_SLAVE0_AWUSER} -port_direction {OUT} -port_range {[0:0]}
sd_create_bus_port -sd_name ${sd_name} -port_name {AXI4mslave0_SLAVE0_WDATA} -port_direction {OUT} -port_range {[63:0]}
sd_create_bus_port -sd_name ${sd_name} -port_name {AXI4mslave0_SLAVE0_WSTRB} -port_direction {OUT} -port_range {[7:0]}
sd_create_bus_port -sd_name ${sd_name} -port_name {AXI4mslave0_SLAVE0_WUSER} -port_direction {OUT} -port_range {[0:0]}
sd_create_scalar_port -sd_name ${sd_name} -port_name {axi_clk} -port_direction {IN}

sd_create_scalar_port -sd_name ${sd_name} -port_name {PCLK} -port_direction {IN}
sd_create_scalar_port -sd_name ${sd_name} -port_name {PREADYS9} -port_direction {IN}
sd_create_scalar_port -sd_name ${sd_name} -port_name {PSLVERRS9} -port_direction {IN}
sd_create_scalar_port -sd_name ${sd_name} -port_name {TCK} -port_direction {IN}
sd_create_scalar_port -sd_name ${sd_name} -port_name {TDI} -port_direction {IN}
sd_create_scalar_port -sd_name ${sd_name} -port_name {TMS} -port_direction {IN}
sd_create_scalar_port -sd_name ${sd_name} -port_name {TRSTB} -port_direction {IN}
sd_create_scalar_port -sd_name ${sd_name} -port_name {reset} -port_direction {IN}

sd_create_scalar_port -sd_name ${sd_name} -port_name {APB_CLK} -port_direction {OUT}
sd_create_scalar_port -sd_name ${sd_name} -port_name {APB_Reset} -port_direction {OUT}
sd_create_scalar_port -sd_name ${sd_name} -port_name {CAM2_RST} -port_direction {OUT}
sd_create_scalar_port -sd_name ${sd_name} -port_name {AXI_RST} -port_direction {OUT}
sd_create_scalar_port -sd_name ${sd_name} -port_name {CAM2_CLK_EN} -port_direction {OUT}
sd_create_scalar_port -sd_name ${sd_name} -port_name {HDMI_RST} -port_direction {OUT}
sd_create_scalar_port -sd_name ${sd_name} -port_name {PENABLES} -port_direction {OUT}
sd_create_scalar_port -sd_name ${sd_name} -port_name {PSELS9} -port_direction {OUT}
sd_create_scalar_port -sd_name ${sd_name} -port_name {PWRITES} -port_direction {OUT}
sd_create_scalar_port -sd_name ${sd_name} -port_name {TDO} -port_direction {OUT}
sd_create_scalar_port -sd_name ${sd_name} -port_name {TRNG_RST_N} -port_direction {OUT}

sd_create_scalar_port -sd_name ${sd_name} -port_name {CAM2_SCL} -port_direction {INOUT} -port_is_pad {1}
sd_create_scalar_port -sd_name ${sd_name} -port_name {CAM2_SDA} -port_direction {INOUT} -port_is_pad {1}
sd_create_scalar_port -sd_name ${sd_name} -port_name {HDMI_SCL} -port_direction {INOUT} -port_is_pad {1}
sd_create_scalar_port -sd_name ${sd_name} -port_name {HDMI_SDA} -port_direction {INOUT} -port_is_pad {1}

sd_create_scalar_port -sd_name ${sd_name} -port_name {UART_RX} -port_direction {IN}
sd_create_scalar_port -sd_name ${sd_name} -port_name {UART_TX} -port_direction {OUT}

# Create top level Bus Ports

sd_create_bus_port -sd_name ${sd_name} -port_name {GPIO_OUT_0} -port_direction {OUT} -port_range {[3:0]}
sd_create_bus_port -sd_name ${sd_name} -port_name {PRDATAS9} -port_direction {IN} -port_range {[31:0]}
sd_create_bus_port -sd_name ${sd_name} -port_name {PADDRS} -port_direction {OUT} -port_range {[31:0]}
sd_create_bus_port -sd_name ${sd_name} -port_name {PWDATAS} -port_direction {OUT} -port_range {[31:0]}


# Create top level Bus interface Ports
sd_create_bif_port -sd_name ${sd_name} -port_name {APBmslave9} -port_bif_vlnv {AMBA:AMBA2:APB:r0p0} -port_bif_role {mirroredSlave} -port_bif_mapping {\
"PADDR:PADDRS" \
"PSELx:PSELS9" \
"PENABLE:PENABLES" \
"PWRITE:PWRITES" \
"PRDATA:PRDATAS9" \
"PWDATA:PWDATAS" \
"PREADY:PREADYS9" \
"PSLVERR:PSLVERRS9" } 

sd_create_bif_port -sd_name ${sd_name} -port_name {AXI4mslave0} -port_bif_vlnv {AMBA:AMBA4:AXI4:r0p0_0} -port_bif_role {mirroredSlave} -port_bif_mapping {\
"AWID:AXI4mslave0_SLAVE0_AWID" \
"AWADDR:AXI4mslave0_SLAVE0_AWADDR" \
"AWLEN:AXI4mslave0_SLAVE0_AWLEN" \
"AWSIZE:AXI4mslave0_SLAVE0_AWSIZE" \
"AWBURST:AXI4mslave0_SLAVE0_AWBURST" \
"AWLOCK:AXI4mslave0_SLAVE0_AWLOCK" \
"AWCACHE:AXI4mslave0_SLAVE0_AWCACHE" \
"AWPROT:AXI4mslave0_SLAVE0_AWPROT" \
"AWQOS:AXI4mslave0_SLAVE0_AWQOS" \
"AWREGION:AXI4mslave0_SLAVE0_AWREGION" \
"AWVALID:AXI4mslave0_SLAVE0_AWVALID" \
"AWREADY:AXI4mslave0_SLAVE0_AWREADY" \
"WDATA:AXI4mslave0_SLAVE0_WDATA" \
"WSTRB:AXI4mslave0_SLAVE0_WSTRB" \
"WLAST:AXI4mslave0_SLAVE0_WLAST" \
"WVALID:AXI4mslave0_SLAVE0_WVALID" \
"WREADY:AXI4mslave0_SLAVE0_WREADY" \
"BID:AXI4mslave0_SLAVE0_BID" \
"BRESP:AXI4mslave0_SLAVE0_BRESP" \
"BVALID:AXI4mslave0_SLAVE0_BVALID" \
"BREADY:AXI4mslave0_SLAVE0_BREADY" \
"ARID:AXI4mslave0_SLAVE0_ARID" \
"ARADDR:AXI4mslave0_SLAVE0_ARADDR" \
"ARLEN:AXI4mslave0_SLAVE0_ARLEN" \
"ARSIZE:AXI4mslave0_SLAVE0_ARSIZE" \
"ARBURST:AXI4mslave0_SLAVE0_ARBURST" \
"ARLOCK:AXI4mslave0_SLAVE0_ARLOCK" \
"ARCACHE:AXI4mslave0_SLAVE0_ARCACHE" \
"ARPROT:AXI4mslave0_SLAVE0_ARPROT" \
"ARQOS:AXI4mslave0_SLAVE0_ARQOS" \
"ARREGION:AXI4mslave0_SLAVE0_ARREGION" \
"ARVALID:AXI4mslave0_SLAVE0_ARVALID" \
"ARREADY:AXI4mslave0_SLAVE0_ARREADY" \
"RID:AXI4mslave0_SLAVE0_RID" \
"RDATA:AXI4mslave0_SLAVE0_RDATA" \
"RRESP:AXI4mslave0_SLAVE0_RRESP" \
"RLAST:AXI4mslave0_SLAVE0_RLAST" \
"RVALID:AXI4mslave0_SLAVE0_RVALID" \
"RREADY:AXI4mslave0_SLAVE0_RREADY" \
"AWUSER:AXI4mslave0_SLAVE0_AWUSER" \
"WUSER:AXI4mslave0_SLAVE0_WUSER" \
"BUSER:AXI4mslave0_SLAVE0_BUSER" \
"ARUSER:AXI4mslave0_SLAVE0_ARUSER" \
"RUSER:AXI4mslave0_SLAVE0_RUSER" } 

# Add BIBUF_2 instance
sd_instantiate_macro -sd_name ${sd_name} -macro_name {BIBUF} -instance_name {BIBUF_2}
sd_connect_pins_to_constant -sd_name ${sd_name} -pin_names {BIBUF_2:D} -value {GND}



# Add BIBUF_3 instance
sd_instantiate_macro -sd_name ${sd_name} -macro_name {BIBUF} -instance_name {BIBUF_3}
sd_connect_pins_to_constant -sd_name ${sd_name} -pin_names {BIBUF_3:D} -value {GND}



# Add BIBUF_4 instance
sd_instantiate_macro -sd_name ${sd_name} -macro_name {BIBUF} -instance_name {BIBUF_4}
sd_connect_pins_to_constant -sd_name ${sd_name} -pin_names {BIBUF_4:D} -value {GND}



# Add BIBUF_5 instance
sd_instantiate_macro -sd_name ${sd_name} -macro_name {BIBUF} -instance_name {BIBUF_5}
sd_connect_pins_to_constant -sd_name ${sd_name} -pin_names {BIBUF_5:D} -value {GND}


# Add CoreAPB3_0 instance
sd_instantiate_core -sd_name ${sd_name} -core_vlnv "Actel:DirectCore:CoreAPB3:${CoreAPB3_version}" -instance_name {CoreAPB3_0}
# Exporting Parameters of instance CoreAPB3_0
sd_configure_core_instance -sd_name ${sd_name} -instance_name {CoreAPB3_0} -params {\
"APB_DWIDTH:32" \
"APBSLOT0ENABLE:false" \
"APBSLOT1ENABLE:true" \
"APBSLOT2ENABLE:true" \
"APBSLOT3ENABLE:true" \
"APBSLOT4ENABLE:true" \
"APBSLOT5ENABLE:true" \
"APBSLOT6ENABLE:true" \
"APBSLOT7ENABLE:true" \
"APBSLOT8ENABLE:true" \
"APBSLOT9ENABLE:true" \
"APBSLOT10ENABLE:false" \
"APBSLOT11ENABLE:false" \
"APBSLOT12ENABLE:false" \
"APBSLOT13ENABLE:false" \
"APBSLOT14ENABLE:false" \
"APBSLOT15ENABLE:false" \
"FAMILY:15" \
"HDL_license:U" \
"IADDR_OPTION:0" \
"MADDR_BITS:16" \
"SC_0:false" \
"SC_1:false" \
"SC_2:false" \
"SC_3:false" \
"SC_4:false" \
"SC_5:false" \
"SC_6:false" \
"SC_7:false" \
"SC_8:false" \
"SC_9:false" \
"SC_10:false" \
"SC_11:false" \
"SC_12:false" \
"SC_13:false" \
"SC_14:false" \
"SC_15:false" \
"testbench:User" \
"UPR_NIBBLE_POSN:6" }\
-validate_rules 0
sd_save_core_instance_config -sd_name ${sd_name} -instance_name {CoreAPB3_0}



# Add COREAXI4INTERCONNECT_C1_0 instance
sd_instantiate_component -sd_name ${sd_name} -component_name {COREAXI4INTERCONNECT_C1} -instance_name {COREAXI4INTERCONNECT_C1_0}

# Add CoreGPIO_OUT instance
sd_instantiate_core -sd_name ${sd_name} -core_vlnv "Actel:DirectCore:CoreGPIO:${CoreGPIO_version}" -instance_name {CoreGPIO_OUT}
# Exporting Parameters of instance CoreGPIO_OUT
sd_configure_core_instance -sd_name ${sd_name} -instance_name {CoreGPIO_OUT} -params {\
"APB_WIDTH:32" \
"FIXED_CONFIG_0:true" \
"FIXED_CONFIG_1:true" \
"FIXED_CONFIG_2:true" \
"FIXED_CONFIG_3:true" \
"FIXED_CONFIG_4:true" \
"FIXED_CONFIG_5:true" \
"FIXED_CONFIG_6:true" \
"FIXED_CONFIG_7:true" \
"FIXED_CONFIG_8:true" \
"FIXED_CONFIG_9:true" \
"FIXED_CONFIG_10:true" \
"FIXED_CONFIG_11:true" \
"FIXED_CONFIG_12:true" \
"FIXED_CONFIG_13:true" \
"FIXED_CONFIG_14:true" \
"FIXED_CONFIG_15:true" \
"FIXED_CONFIG_16:true" \
"FIXED_CONFIG_17:true" \
"FIXED_CONFIG_18:true" \
"FIXED_CONFIG_19:true" \
"FIXED_CONFIG_20:true" \
"FIXED_CONFIG_21:true" \
"FIXED_CONFIG_22:true" \
"FIXED_CONFIG_23:true" \
"FIXED_CONFIG_24:true" \
"FIXED_CONFIG_25:true" \
"FIXED_CONFIG_26:true" \
"FIXED_CONFIG_27:true" \
"FIXED_CONFIG_28:true" \
"FIXED_CONFIG_29:true" \
"FIXED_CONFIG_30:true" \
"FIXED_CONFIG_31:true" \
"INT_BUS:0" \
"IO_INT_TYPE_0:7" \
"IO_INT_TYPE_1:7" \
"IO_INT_TYPE_2:7" \
"IO_INT_TYPE_3:7" \
"IO_INT_TYPE_4:7" \
"IO_INT_TYPE_5:7" \
"IO_INT_TYPE_6:7" \
"IO_INT_TYPE_7:7" \
"IO_INT_TYPE_8:7" \
"IO_INT_TYPE_9:7" \
"IO_INT_TYPE_10:7" \
"IO_INT_TYPE_11:7" \
"IO_INT_TYPE_12:7" \
"IO_INT_TYPE_13:7" \
"IO_INT_TYPE_14:7" \
"IO_INT_TYPE_15:7" \
"IO_INT_TYPE_16:7" \
"IO_INT_TYPE_17:7" \
"IO_INT_TYPE_18:7" \
"IO_INT_TYPE_19:7" \
"IO_INT_TYPE_20:7" \
"IO_INT_TYPE_21:7" \
"IO_INT_TYPE_22:7" \
"IO_INT_TYPE_23:7" \
"IO_INT_TYPE_24:7" \
"IO_INT_TYPE_25:7" \
"IO_INT_TYPE_26:7" \
"IO_INT_TYPE_27:7" \
"IO_INT_TYPE_28:7" \
"IO_INT_TYPE_29:7" \
"IO_INT_TYPE_30:7" \
"IO_INT_TYPE_31:7" \
"IO_NUM:32" \
"IO_TYPE_0:1" \
"IO_TYPE_1:1" \
"IO_TYPE_2:1" \
"IO_TYPE_3:1" \
"IO_TYPE_4:1" \
"IO_TYPE_5:1" \
"IO_TYPE_6:1" \
"IO_TYPE_7:1" \
"IO_TYPE_8:1" \
"IO_TYPE_9:1" \
"IO_TYPE_10:1" \
"IO_TYPE_11:1" \
"IO_TYPE_12:1" \
"IO_TYPE_13:1" \
"IO_TYPE_14:1" \
"IO_TYPE_15:1" \
"IO_TYPE_16:1" \
"IO_TYPE_17:1" \
"IO_TYPE_18:1" \
"IO_TYPE_19:1" \
"IO_TYPE_20:1" \
"IO_TYPE_21:1" \
"IO_TYPE_22:1" \
"IO_TYPE_23:1" \
"IO_TYPE_24:1" \
"IO_TYPE_25:1" \
"IO_TYPE_26:1" \
"IO_TYPE_27:1" \
"IO_TYPE_28:1" \
"IO_TYPE_29:1" \
"IO_TYPE_30:1" \
"IO_TYPE_31:1" \
"IO_VAL_0:0" \
"IO_VAL_1:0" \
"IO_VAL_2:0" \
"IO_VAL_3:0" \
"IO_VAL_4:0" \
"IO_VAL_5:0" \
"IO_VAL_6:0" \
"IO_VAL_7:0" \
"IO_VAL_8:0" \
"IO_VAL_9:0" \
"IO_VAL_10:0" \
"IO_VAL_11:0" \
"IO_VAL_12:0" \
"IO_VAL_13:0" \
"IO_VAL_14:0" \
"IO_VAL_15:0" \
"IO_VAL_16:0" \
"IO_VAL_17:0" \
"IO_VAL_18:0" \
"IO_VAL_19:0" \
"IO_VAL_20:0" \
"IO_VAL_21:0" \
"IO_VAL_22:0" \
"IO_VAL_23:0" \
"IO_VAL_24:0" \
"IO_VAL_25:0" \
"IO_VAL_26:0" \
"IO_VAL_27:0" \
"IO_VAL_28:0" \
"IO_VAL_29:0" \
"IO_VAL_30:0" \
"IO_VAL_31:0" \
"OE_TYPE:1" \
"testbench:User" }\
-validate_rules 0
sd_save_core_instance_config -sd_name ${sd_name} -instance_name {CoreGPIO_OUT}
sd_create_pin_slices -sd_name ${sd_name} -pin_name {CoreGPIO_OUT:GPIO_OUT} -pin_slices {[31:10]}
sd_mark_pins_unused -sd_name ${sd_name} -pin_names {CoreGPIO_OUT:GPIO_OUT[31:10]}
sd_create_pin_slices -sd_name ${sd_name} -pin_name {CoreGPIO_OUT:GPIO_OUT} -pin_slices {[3:0]}
sd_create_pin_slices -sd_name ${sd_name} -pin_name {CoreGPIO_OUT:GPIO_OUT} -pin_slices {[4:4]}
sd_create_pin_slices -sd_name ${sd_name} -pin_name {CoreGPIO_OUT:GPIO_OUT} -pin_slices {[5:5]}
sd_create_pin_slices -sd_name ${sd_name} -pin_name {CoreGPIO_OUT:GPIO_OUT} -pin_slices {[6:6]}
sd_mark_pins_unused -sd_name ${sd_name} -pin_names {CoreGPIO_OUT:GPIO_OUT[6:6]}
sd_create_pin_slices -sd_name ${sd_name} -pin_name {CoreGPIO_OUT:GPIO_OUT} -pin_slices {[7:7]}
sd_create_pin_slices -sd_name ${sd_name} -pin_name {CoreGPIO_OUT:GPIO_OUT} -pin_slices {[8:8]}
sd_mark_pins_unused -sd_name ${sd_name} -pin_names {CoreGPIO_OUT:GPIO_OUT[8:8]}
sd_create_pin_slices -sd_name ${sd_name} -pin_name {CoreGPIO_OUT:GPIO_OUT} -pin_slices {[9:9]}
sd_mark_pins_unused -sd_name ${sd_name} -pin_names {CoreGPIO_OUT:INT}
sd_connect_pins_to_constant -sd_name ${sd_name} -pin_names {CoreGPIO_OUT:GPIO_IN} -value {GND}



# Add COREI2C_C0_0 instance
sd_instantiate_component -sd_name ${sd_name} -component_name {COREI2C_C0} -instance_name {COREI2C_C0_0}


# Add COREI2C_C0_1 instance
sd_instantiate_component -sd_name ${sd_name} -component_name {COREI2C_C0} -instance_name {COREI2C_C0_1}

# "Add CoreTimer_C0_0 instance"
sd_instantiate_component -sd_name ${sd_name} -component_name {CoreTimer_C0} -instance_name {CoreTimer_C0_0}

# Add COREJTAGDEBUG_0 instance
sd_instantiate_component -sd_name ${sd_name} -component_name {COREJTAGDEBUG_C0} -instance_name {COREJTAGDEBUG_0}

# Add INV_2 instance
sd_instantiate_macro -sd_name ${sd_name} -macro_name {INV} -instance_name {INV_2}

# Add INV_3 instance
sd_instantiate_macro -sd_name ${sd_name} -macro_name {INV} -instance_name {INV_3}

# Add INV_4 instance
sd_instantiate_macro -sd_name ${sd_name} -macro_name {INV} -instance_name {INV_4}

# Add INV_5 instance
sd_instantiate_macro -sd_name ${sd_name} -macro_name {INV} -instance_name {INV_5}

# CPU
sd_instantiate_component -sd_name ${sd_name} -component_name {MIV_RV32_C0} -instance_name {MIV_RV32_C0_0} 
sd_mark_pins_unused -sd_name ${sd_name} -pin_names {MIV_RV32_C0_0:EXT_RESETN}
sd_create_pin_slices -sd_name ${sd_name} -pin_name {MIV_RV32_C0_0:MSYS_EI} -pin_slices {"[3:3]"}
sd_create_pin_slices -sd_name ${sd_name} -pin_name {MIV_RV32_C0_0:MSYS_EI} -pin_slices {"[2:2]"}
sd_create_pin_slices -sd_name ${sd_name} -pin_name {MIV_RV32_C0_0:MSYS_EI} -pin_slices {"[1:1]"}
sd_create_pin_slices -sd_name ${sd_name} -pin_name {MIV_RV32_C0_0:MSYS_EI} -pin_slices {"[0:0]"}
sd_connect_pins -sd_name ${sd_name} -pin_names {"MIV_RV32_C0_0:APB_INITIATOR" "CoreAPB3_0:APB3mmaster"} 
sd_connect_pins -sd_name ${sd_name} -pin_names {"PCLK" "MIV_RV32_C0_0:CLK"} 
sd_connect_pins -sd_name ${sd_name} -pin_names {"reset" "MIV_RV32_C0_0:RESETN"} 
sd_connect_pins -sd_name ${sd_name} -pin_names {"COREJTAGDEBUG_0:TGT_TCK_0"   "MIV_RV32_C0_0:JTAG_TCK"} 
sd_connect_pins -sd_name ${sd_name} -pin_names {"COREJTAGDEBUG_0:TGT_TDI_0"   "MIV_RV32_C0_0:JTAG_TDI"}
sd_connect_pins -sd_name ${sd_name} -pin_names {"COREJTAGDEBUG_0:TGT_TMS_0"   "MIV_RV32_C0_0:JTAG_TMS"}
sd_connect_pins -sd_name ${sd_name} -pin_names {"COREJTAGDEBUG_0:TGT_TRSTN_0" "MIV_RV32_C0_0:JTAG_TRSTN"}
sd_connect_pins -sd_name ${sd_name} -pin_names {"MIV_RV32_C0_0:JTAG_TDO"      "COREJTAGDEBUG_0:TGT_TDO_0"}
sd_mark_pins_unused -sd_name ${sd_name} -pin_names {MIV_RV32_C0_0:JTAG_TDO_DR}
sd_connect_pins -sd_name ${sd_name} -pin_names {"COREI2C_C0_0:INT"            "MIV_RV32_C0_0:MSYS_EI[0:0]"}
sd_connect_pins -sd_name ${sd_name} -pin_names {"COREI2C_C0_1:INT"            "MIV_RV32_C0_0:MSYS_EI[1:1]"}
sd_connect_pins -sd_name ${sd_name} -pin_names {"CoreTimer_C0_0:TIMINT"       "MIV_RV32_C0_0:MSYS_EI[3:3]"}


# sd_mark_pins_unused -sd_name ${sd_name} -pin_names {MIV_RV32_C0_0:AXI4_INITIATOR}


# Add PF_SRAM_AHBL_AXI_C0_0 instance
sd_instantiate_component -sd_name ${sd_name} -component_name {PF_SRAM_AHBL_AXI_C0} -instance_name {PF_SRAM_AHBL_AXI_C0_0}
sd_connect_pins -sd_name ${sd_name} -pin_names {"MIV_RV32_C0_0:AHBL_M_TARGET" "PF_SRAM_AHBL_AXI_C0_0:AHBSlaveInterface" }


puts "Add CoreUARTapb_C0_0 instance"
sd_instantiate_component -sd_name ${sd_name} -component_name {CoreUARTapb_C0} -instance_name {CoreUARTapb_C0_0}
sd_mark_pins_unused -sd_name ${sd_name} -pin_names {CoreUARTapb_C0_0:TXRDY}
sd_mark_pins_unused -sd_name ${sd_name} -pin_names {CoreUARTapb_C0_0:RXRDY}
sd_mark_pins_unused -sd_name ${sd_name} -pin_names {CoreUARTapb_C0_0:PARITY_ERR}
sd_mark_pins_unused -sd_name ${sd_name} -pin_names {CoreUARTapb_C0_0:OVERFLOW}
sd_mark_pins_unused -sd_name ${sd_name} -pin_names {CoreUARTapb_C0_0:FRAMING_ERR}
sd_connect_pins -sd_name ${sd_name} -pin_names {"CoreUARTapb_C0_0:RX" "UART_RX"}
sd_connect_pins -sd_name ${sd_name} -pin_names {"CoreUARTapb_C0_0:TX" "UART_TX"}


# Add scalar net connections
sd_connect_pins -sd_name ${sd_name} -pin_names {"PCLK" \
    "APB_CLK" \
    "COREI2C_C0_0:BCLK" \
    "COREI2C_C0_0:PCLK" \
    "COREI2C_C0_1:BCLK" \
    "COREI2C_C0_1:PCLK" \
    "CoreGPIO_OUT:PCLK" \
    "CoreTimer_C0_0:PCLK" \
    "PF_SRAM_AHBL_AXI_C0_0:HCLK" \
    "CoreUARTapb_C0_0:PCLK" \
    "COREAXI4INTERCONNECT_C1_0:ACLK" \
}
sd_connect_pins -sd_name ${sd_name} -pin_names {"reset" \
    "APB_Reset" \
    "COREI2C_C0_0:PRESETN" \
    "COREI2C_C0_1:PRESETN" \
    "CoreUARTapb_C0_0:PRESETN" \
    "CoreGPIO_OUT:PRESETN" \
    "CoreTimer_C0_0:PRESETN" \
    "HDMI_RST" \
    "PF_SRAM_AHBL_AXI_C0_0:HRESETN" \
    "COREAXI4INTERCONNECT_C1_0:ARESETN" \
}
sd_connect_pins -sd_name ${sd_name} -pin_names {"BIBUF_2:E" "INV_2:Y" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"BIBUF_2:PAD" "HDMI_SCL" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"BIBUF_2:Y" "COREI2C_C0_1:SCLI" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"BIBUF_3:E" "INV_3:Y" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"BIBUF_3:PAD" "HDMI_SDA" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"BIBUF_3:Y" "COREI2C_C0_1:SDAI" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"BIBUF_4:E" "INV_4:Y" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"BIBUF_4:PAD" "CAM2_SCL" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"BIBUF_4:Y" "COREI2C_C0_0:SCLI" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"BIBUF_5:E" "INV_5:Y" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"BIBUF_5:PAD" "CAM2_SDA" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"BIBUF_5:Y" "COREI2C_C0_0:SDAI" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"CAM2_RST" "CoreGPIO_OUT:GPIO_OUT[7:7]" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"AXI_RST" "CoreGPIO_OUT:GPIO_OUT[5:5]" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"COREI2C_C0_0:SCLO" "INV_4:A" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"COREI2C_C0_0:SDAO" "INV_5:A" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"COREI2C_C0_1:SCLO" "INV_2:A" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"COREI2C_C0_1:SDAO" "INV_3:A" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"COREJTAGDEBUG_0:TCK" "TCK" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"COREJTAGDEBUG_0:TDI" "TDI" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"COREJTAGDEBUG_0:TDO" "TDO" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"COREJTAGDEBUG_0:TMS" "TMS" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"COREJTAGDEBUG_0:TRSTB" "TRSTB" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"CoreGPIO_OUT:GPIO_OUT[4:4]" "TRNG_RST_N"}
sd_connect_pins -sd_name ${sd_name} -pin_names {"CoreGPIO_OUT:GPIO_OUT[9:9]" "CAM2_CLK_EN"}

# Add bus net connections
sd_connect_pins -sd_name ${sd_name} -pin_names {"CoreGPIO_OUT:GPIO_OUT[3:0]" "GPIO_OUT_0" }

# Add bus interface net connections
sd_connect_pins -sd_name ${sd_name} -pin_names {"COREAXI4INTERCONNECT_C1_0:AXI4mslave0" "AXI4mslave0"}
sd_connect_pins -sd_name ${sd_name} -pin_names {"MIV_RV32_C0_0:AXI4_INITIATOR" "COREAXI4INTERCONNECT_C1_0:AXI4mmaster0"}
sd_connect_pins -sd_name ${sd_name} -pin_names {"axi_clk" "COREAXI4INTERCONNECT_C1_0:S_CLK0"}

sd_mark_pins_unused -sd_name ${sd_name} -pin_names {CoreAPB3_0:APBmslave1}
sd_mark_pins_unused -sd_name ${sd_name} -pin_names {CoreAPB3_0:APBmslave2}
sd_mark_pins_unused -sd_name ${sd_name} -pin_names {CoreAPB3_0:APBmslave7}
sd_connect_pins -sd_name ${sd_name} -pin_names {"CoreAPB3_0:APBmslave3" "CoreTimer_C0_0:APBslave"}
sd_connect_pins -sd_name ${sd_name} -pin_names {"CoreAPB3_0:APBmslave4" "COREI2C_C0_0:APBslave"}
sd_connect_pins -sd_name ${sd_name} -pin_names {"CoreAPB3_0:APBmslave5" "CoreGPIO_OUT:APB_bif" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"CoreAPB3_0:APBmslave6" "CoreUARTapb_C0_0:APB_bif" }
sd_connect_pins -sd_name ${sd_name} -pin_names {"CoreAPB3_0:APBmslave8" "COREI2C_C0_1:APBslave"}
sd_connect_pins -sd_name ${sd_name} -pin_names {"CoreAPB3_0:APBmslave9" "APBmslave9"}


# Re-enable auto promotion of pins of type 'pad'
auto_promote_pad_pins -promote_all 1
# Save the smartDesign
save_smartdesign -sd_name ${sd_name}
# Generate SmartDesign PROC_SUBSYSTEM
generate_component -component_name ${sd_name}
