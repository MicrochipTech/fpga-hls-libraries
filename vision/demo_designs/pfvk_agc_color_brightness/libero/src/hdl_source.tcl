import_files -library work -hdl_source hdl/FrameBufferControl.v

# Add verilog testbenches
create_links -stimulus ../../../../rtl/camera_sd_component/IMX334_IF_TOP/testbench/IMX334_IF_TB.v
create_links -stimulus stimulus/PROC_SUBSYSTEM_TB.v
create_links -stimulus ../../../../rtl/display_sd_component/HDMI_2p0/testbench/HDMI_2p0_TB.v
