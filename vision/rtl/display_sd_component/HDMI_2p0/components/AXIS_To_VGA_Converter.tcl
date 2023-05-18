# Exporting core AXIS_To_VGA_Converter to TCL
# Exporting Create HDL core command for module AXIS_To_VGA_Converter
create_hdl_core -file {hdl/axis_to_vga_converter.v} -module {AXIS_To_VGA_Converter} -library {work} -package {}
# Exporting BIF information of  HDL core command for module AXIS_To_VGA_Converter
hdl_core_add_bif -hdl_core_name {AXIS_To_VGA_Converter} -bif_definition {AXI4Stream:AMBA:AMBA4:slave} -bif_name {AXIS} -signal_map {\
"TVALID:tvalid" \
"TDATA:tdata" \
"TLAST:tlast" \
"TUSER:tuser" \
"TREADY:tready_O" }
