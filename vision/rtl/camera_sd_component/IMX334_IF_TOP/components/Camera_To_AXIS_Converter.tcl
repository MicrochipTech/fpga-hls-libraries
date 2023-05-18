# Exporting core Camera_To_AXIS_Converter to TCL
# Exporting Create HDL core command for module Camera_To_AXIS_Converter
create_hdl_core -file {hdl/camera_to_axis_converter.v} -module {Camera_To_AXIS_Converter} -library {work} -package {}
# Exporting BIF information of  HDL core command for module Camera_To_AXIS_Converter
hdl_core_add_bif -hdl_core_name {Camera_To_AXIS_Converter} -bif_definition {AXI4Stream:AMBA:AMBA4:master} -bif_name {AXIS} -signal_map {\
"TVALID:o_tvalid" \
"TREADY:i_tready" \
"TDATA:o_tdata" \
"TLAST:o_tlast" \
"TUSER:o_tuser" }
