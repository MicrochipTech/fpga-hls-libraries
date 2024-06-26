# Exporting core DDR_Access_wrapper_top to TCL
# Exporting Create HDL core command for module DDR_Access_wrapper_top
create_hdl_core -file {hdl/ddr_access_wrapper.v} -module {DDR_Access_wrapper_top} -library {work} -package {}
# Exporting BIF information of  HDL core command for module DDR_Access_wrapper_top
hdl_core_add_bif -hdl_core_name {DDR_Access_wrapper_top} -bif_definition {AXI4Stream:AMBA:AMBA4:slave} -bif_name {VideoIn_axi4stream} -signal_map {\
"TDATA:VideoIn_data" \
"TLAST:VideoIn_last" \
"TREADY:VideoIn_ready" \
"TUSER:VideoIn_user" \
"TVALID:VideoIn_valid" }
hdl_core_add_bif -hdl_core_name {DDR_Access_wrapper_top} -bif_definition {AXI4Stream:AMBA:AMBA4:master} -bif_name {VideoOut_axi4stream} -signal_map {\
"TDATA:VideoOut_data" \
"TLAST:VideoOut_last" \
"TREADY:VideoOut_ready" \
"TUSER:VideoOut_user" \
"TVALID:VideoOut_valid" }
hdl_core_add_bif -hdl_core_name {DDR_Access_wrapper_top} -bif_definition {AXI4:AMBA:AMBA4:master} -bif_name {axi4initiator} -signal_map {\
"AWADDR:axi4initiator_aw_addr" \
"AWBURST:axi4initiator_aw_burst" \
"AWLEN:axi4initiator_aw_len" \
"AWREADY:axi4initiator_aw_ready" \
"AWSIZE:axi4initiator_aw_size" \
"AWVALID:axi4initiator_aw_valid" \
"BRESP:axi4initiator_b_resp" \
"BREADY:axi4initiator_b_resp_ready" \
"BVALID:axi4initiator_b_resp_valid" \
"WDATA:axi4initiator_w_data" \
"WLAST:axi4initiator_w_last" \
"WREADY:axi4initiator_w_ready" \
"WSTRB:axi4initiator_w_strb" \
"WVALID:axi4initiator_w_valid" \
"ARADDR:axi4initiator_ar_addr" \
"ARBURST:axi4initiator_ar_burst" \
"ARLEN:axi4initiator_ar_len" \
"ARREADY:axi4initiator_ar_ready" \
"ARSIZE:axi4initiator_ar_size" \
"ARVALID:axi4initiator_ar_valid" \
"RDATA:axi4initiator_r_data" \
"RLAST:axi4initiator_r_last" \
"RREADY:axi4initiator_r_ready" \
"RRESP:axi4initiator_r_resp" \
"RVALID:axi4initiator_r_valid" }
