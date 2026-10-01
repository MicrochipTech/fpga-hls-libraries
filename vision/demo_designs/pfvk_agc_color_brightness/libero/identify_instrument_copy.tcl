puts "TCL_BEGIN: [info script]"

# set scriptPath [file normalize [file dirname [info script]]]

set prjFile ./SEVPFSOC_TOP_syn.prj
set top SEVPFSOC_TOP

project open $prjFile

impl -active synthesis

device jtagport builtin

# --[ IICE for AXIS_To_VGA_Converter_0]--------------------------------------------
iice new {IICE} -type regular
iice controller -iice {IICE} none
iice sampler -iice {IICE_0} -ram {type URAM}
iice sampler -iice {IICE} -depth 2048
iice clock -iice {IICE} -edge positive  {/HDMI/AXIS_To_VGA_Converter_0/clk}
signals add -iice {IICE} -silent -sample {/HDMI/AXIS_To_VGA_Converter_0/tdata_int_debubble}\
{/HDMI/AXIS_To_VGA_Converter_0/tuser_int_debubble}\
{/HDMI/AXIS_To_VGA_Converter_0/tvalid_int_debubble}\
{/HDMI/AXIS_To_VGA_Converter_0/tlast_int_debubble}\
{/HDMI/AXIS_To_VGA_Converter_0/tready_int_debubble}\
{/HDMI/AXIS_To_VGA_Converter_0/tdata_int_converter}\
{/HDMI/AXIS_To_VGA_Converter_0/tuser_int_converter}\
{/HDMI/AXIS_To_VGA_Converter_0/data_enable_counter}\
{/HDMI/AXIS_To_VGA_Converter_0/data_enable_VGA}\
{/HDMI/AXIS_To_VGA_Converter_0/tvalid_int_converter}\
{/HDMI/AXIS_To_VGA_Converter_0/tlast_int_converter}\
{/HDMI/AXIS_To_VGA_Converter_0/tready_int_converter}\
{/HDMI/AXIS_To_VGA_Converter_0/display_data}\
{/HDMI/AXIS_To_VGA_Converter_0/fifo_o}\
{/HDMI/AXIS_To_VGA_Converter_0/fifo_used_w}\
{/HDMI/AXIS_To_VGA_Converter_0/pop_fifo}\
{/HDMI/AXIS_To_VGA_Converter_0/fifo_almost_empty}\
{/HDMI/AXIS_To_VGA_Converter_0/fifo_full}\
{/HDMI/AXIS_To_VGA_Converter_0/vsync_VGA}\
{/HDMI/AXIS_To_VGA_Converter_0/vsync_counter}\
{/HDMI/AXIS_To_VGA_Converter_0/hsync_VGA}\
{/HDMI/AXIS_To_VGA_Converter_0/hsync_counter}\
{/HDMI/AXIS_To_VGA_Converter_0/B_O}\
{/HDMI/AXIS_To_VGA_Converter_0/G_O}\
{/HDMI/AXIS_To_VGA_Converter_0/R_O}\
{/HDMI/AXIS_To_VGA_Converter_0/vsync_O}\
{/HDMI/AXIS_To_VGA_Converter_0/hsync_O}\
{/HDMI/AXIS_To_VGA_Converter_0/tready_O}\
{/HDMI/AXIS_To_VGA_Converter_0/tdata}\
{/HDMI/AXIS_To_VGA_Converter_0/tuser}\
{/HDMI/AXIS_To_VGA_Converter_0/tlast}\
{/IMX334_IF_TOP_0/Camera_To_AXIS_Converter_0/o_tdata}\
{/IMX334_IF_TOP_0/Camera_To_AXIS_Converter_0/o_tuser}\
{/IMX334_IF_TOP_0/Camera_To_AXIS_Converter_0/o_tvalid}\
{/IMX334_IF_TOP_0/Camera_To_AXIS_Converter_0/o_tlast}\
{/IMX334_IF_TOP_0/Camera_To_AXIS_Converter_0/i_tready}
signals add -iice {IICE} -silent -sample -trigger {/HDMI/AXIS_To_VGA_Converter_0/data_enable_O}\
{/HDMI/AXIS_To_VGA_Converter_0/tvalid}\
{/HDMI/AXIS_To_VGA_Converter_0/reset}\
{/IMX334_IF_TOP_0/Camera_To_AXIS_Converter_0/i_axis_reset}\
{/IMX334_IF_TOP_0/Camera_To_AXIS_Converter_0/o_pushback_detected}\
{/IMX334_IF_TOP_0/Camera_To_AXIS_Converter_0/o_frames_skipped}




# --[ IICE for video_clock]--------------------------------------------
iice new {IICE_0} -type regular
iice controller -iice {IICE_0} none
iice sampler -iice {IICE_0} -ram {type URAM}
iice sampler -iice {IICE_0} -depth 2048
iice clock -iice {IICE_0} -edge positive  {/IMX334_IF_TOP_0/Camera_To_AXIS_Converter_0/i_video_clk}
signals add -iice {IICE_0} -silent -sample {/IMX334_IF_TOP_0/Camera_To_AXIS_Converter_0/v_count}\
{/IMX334_IF_TOP_0/Camera_To_AXIS_Converter_0/data_i_delay}\
{/IMX334_IF_TOP_0/Camera_To_AXIS_Converter_0/data_valid_delay}\
{/IMX334_IF_TOP_0/Camera_To_AXIS_Converter_0/frame_start_re}\
{/IMX334_IF_TOP_0/Camera_To_AXIS_Converter_0/data_valid_re}\
{/IMX334_IF_TOP_0/Camera_To_AXIS_Converter_0/data_valid_fe}\
{/IMX334_IF_TOP_0/Camera_To_AXIS_Converter_0/frame_start_delay}\
{/IMX334_IF_TOP_0/Camera_To_AXIS_Converter_0/i_hres}\
{/IMX334_IF_TOP_0/Camera_To_AXIS_Converter_0/i_vres}\
{/IMX334_IF_TOP_0/Camera_To_AXIS_Converter_0/i_Data}\
{/IMX334_IF_TOP_0/Camera_To_AXIS_Converter_0/i_data_valid}\
{/IMX334_IF_TOP_0/Camera_To_AXIS_Converter_0/i_frame_start}
signals add -iice {IICE_0} -silent -sample -trigger {/IMX334_IF_TOP_0/Camera_To_AXIS_Converter_0/skip_frame}\
{/IMX334_IF_TOP_0/Camera_To_AXIS_Converter_0/flush_h_count}\
{/IMX334_IF_TOP_0/Camera_To_AXIS_Converter_0/flush_v_count}\
{/IMX334_IF_TOP_0/Camera_To_AXIS_Converter_0/i_video_reset}


write instrumentation -idc_loc ./identify.idc

# Command to estimate the resources required for instrumentation
# device  estimate -resources -iice IICE
project -save $prjFile

puts "TCL_END: [info script]"