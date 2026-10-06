puts "TCL_BEGIN: [info script]"

# set scriptPath [file normalize [file dirname [info script]]]

set prjFile ./SEVPFSOC_TOP_syn.prj
set top SEVPFSOC_TOP

project open $prjFile

impl -active synthesis

device jtagport builtin

# --[ IICE for VideoPipelineTop]--------------------------------------------
iice new {IICE} -type regular -mode {none}
iice sampler -iice {IICE} -depth 2048 -qualified_sampling 1 -always_armed 1 -compression 0 {behavioral}
iice clock -iice {IICE}  -edge positive {/VideoPipelineTop_top_0/clk}
iice controller -iice {IICE}  none

signals add -iice {IICE} -silent -sample -trigger {/VideoPipelineTop_top_0/axi4initiator_r_last}\
{/VideoPipelineTop_top_0/axi4initiator_r_resp}\
{/VideoPipelineTop_top_0/axi4initiator_r_valid}\
{/VideoPipelineTop_top_0/axi4initiator_r_ready}\
{/VideoPipelineTop_top_0/axi4initiator_r_data}\
{/VideoPipelineTop_top_0/axi4initiator_ar_len}\
{/VideoPipelineTop_top_0/axi4initiator_ar_size}\
{/VideoPipelineTop_top_0/axi4initiator_ar_burst}\
{/VideoPipelineTop_top_0/axi4initiator_ar_valid}\
{/VideoPipelineTop_top_0/axi4initiator_ar_ready}\
{/VideoPipelineTop_top_0/axi4initiator_ar_addr}\
{/VideoPipelineTop_top_0/VideoOut_user}\
{/VideoPipelineTop_top_0/VideoOut_last}\
{/VideoPipelineTop_top_0/VideoOut_valid}\
{/VideoPipelineTop_top_0/VideoOut_ready}\
{/VideoPipelineTop_top_0/VideoOut_data}\
{/VideoPipelineTop_top_0/BayerFormat}\
{/VideoPipelineTop_top_0/enable_invert}\
{/VideoPipelineTop_top_0/Buf}\
{/VideoPipelineTop_top_0/finish}\
{/VideoPipelineTop_top_0/ready}\
{/VideoPipelineTop_top_0/start}\
{/VideoPipelineTop_top_0/axi4target_buser}\
{/VideoPipelineTop_top_0/axi4target_bresp}\
{/VideoPipelineTop_top_0/axi4target_bid}\
{/VideoPipelineTop_top_0/axi4target_bready}\
{/VideoPipelineTop_top_0/axi4target_bvalid}\
{/VideoPipelineTop_top_0/axi4target_wuser}\
{/VideoPipelineTop_top_0/axi4target_wstrb}\
{/VideoPipelineTop_top_0/axi4target_wlast}\
{/VideoPipelineTop_top_0/axi4target_wdata}\
{/VideoPipelineTop_top_0/axi4target_wvalid}\
{/VideoPipelineTop_top_0/axi4target_wready}\
{/VideoPipelineTop_top_0/axi4target_awuser}\
{/VideoPipelineTop_top_0/axi4target_awregion}\
{/VideoPipelineTop_top_0/axi4target_awqos}\
{/VideoPipelineTop_top_0/axi4target_awprot}\
{/VideoPipelineTop_top_0/axi4target_awlock}\
{/VideoPipelineTop_top_0/axi4target_awcache}\
{/VideoPipelineTop_top_0/axi4target_awsize}\
{/VideoPipelineTop_top_0/axi4target_awlen}\
{/VideoPipelineTop_top_0/axi4target_awburst}\
{/VideoPipelineTop_top_0/axi4target_awid}\
{/VideoPipelineTop_top_0/axi4target_awaddr}\
{/VideoPipelineTop_top_0/axi4target_awvalid}\
{/VideoPipelineTop_top_0/axi4target_awready}\
{/VideoPipelineTop_top_0/axi4target_ruser}\
{/VideoPipelineTop_top_0/axi4target_rresp}\
{/VideoPipelineTop_top_0/axi4target_rlast}\
{/VideoPipelineTop_top_0/axi4target_rid}\
{/VideoPipelineTop_top_0/axi4target_rdata}\
{/VideoPipelineTop_top_0/axi4target_rvalid}\
{/VideoPipelineTop_top_0/axi4target_rready}\
{/VideoPipelineTop_top_0/axi4target_aruser}\
{/VideoPipelineTop_top_0/axi4target_arregion}\
{/VideoPipelineTop_top_0/axi4target_arqos}\
{/VideoPipelineTop_top_0/axi4target_arprot}\
{/VideoPipelineTop_top_0/axi4target_arlock}\
{/VideoPipelineTop_top_0/axi4target_arcache}\
{/VideoPipelineTop_top_0/axi4target_arsize}\
{/VideoPipelineTop_top_0/axi4target_arlen}\
{/VideoPipelineTop_top_0/axi4target_arburst}\
{/VideoPipelineTop_top_0/axi4target_arid}\
{/VideoPipelineTop_top_0/axi4target_araddr}\
{/VideoPipelineTop_top_0/axi4target_arvalid}\
{/VideoPipelineTop_top_0/axi4target_arready}\
{/VideoPipelineTop_top_0/reset}

# --[ IICE for DDRWriteWrapper]--------------------------------------------
iice new {IICE_0} -type regular
iice controller -iice {IICE_0} none
iice sampler -iice {IICE_0} -ram {type URAM}
iice sampler -iice {IICE_0} -depth 2048
iice clock -iice {IICE_0} -edge positive  {/DDR_Write_wrapper_top_0/clk}
signals add -iice {IICE_0} -silent -sample -trigger {/DDR_Write_wrapper_top_0/axi4initiator_w_last}\
{/DDR_Write_wrapper_top_0/axi4initiator_w_strb}\
{/DDR_Write_wrapper_top_0/axi4initiator_w_valid}\
{/DDR_Write_wrapper_top_0/axi4initiator_w_ready}\
{/DDR_Write_wrapper_top_0/axi4initiator_w_data}\
{/DDR_Write_wrapper_top_0/axi4initiator_aw_len}\
{/DDR_Write_wrapper_top_0/axi4initiator_aw_size}\
{/DDR_Write_wrapper_top_0/axi4initiator_aw_burst}\
{/DDR_Write_wrapper_top_0/axi4initiator_aw_valid}\
{/DDR_Write_wrapper_top_0/axi4initiator_aw_ready}\
{/DDR_Write_wrapper_top_0/axi4initiator_aw_addr}\
{/DDR_Write_wrapper_top_0/axi4initiator_b_resp_valid}\
{/DDR_Write_wrapper_top_0/axi4initiator_b_resp_ready}\
{/DDR_Write_wrapper_top_0/axi4initiator_b_resp}\
{/DDR_Write_wrapper_top_0/VideoIn_user}\
{/DDR_Write_wrapper_top_0/VideoIn_last}\
{/DDR_Write_wrapper_top_0/VideoIn_valid}\
{/DDR_Write_wrapper_top_0/VideoIn_ready}\
{/DDR_Write_wrapper_top_0/VideoIn_data}\
{/DDR_Write_wrapper_top_0/VRes}\
{/DDR_Write_wrapper_top_0/HRes}\
{/DDR_Write_wrapper_top_0/Buf}\
{/DDR_Write_wrapper_top_0/finish}\
{/DDR_Write_wrapper_top_0/ready}\
{/DDR_Write_wrapper_top_0/start}\
{/DDR_Write_wrapper_top_0/reset}

write instrumentation -idc_loc ./identify.idc

# Command to estimate the resources required for instrumentation
# device  estimate -resources -iice IICE
project -save $prjFile

puts "TCL_END: [info script]"