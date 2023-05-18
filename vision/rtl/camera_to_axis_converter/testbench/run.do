vlib work
vcom -2008 ../../axis_to_vga_converter/VGA_Controller_SHLS.vhd
vlog \
  ../../axis_to_vga_converter/axis_to_vga_converter.v \
  ./pattern_generator_verilog_pattern.v \
  ../camera_to_axis_converter.v \
  ./camera_to_axis_converter_tb.v


vsim Camera_To_AXIS_Converter_tb 

#uncomment the lines below to show signals

# add wave -position insertpoint  sim:/Camera_To_AXIS_Converter_tb/*
# add wave -position insertpoint  \
# sim:/Camera_To_AXIS_Converter_tb/axi_video_dut_in/DC_A/DC_Native_FORMAT/Display_Controller_Native_INST/locked \
# sim:/Camera_To_AXIS_Converter_tb/axi_video_dut_in/DC_A/DC_Native_FORMAT/Display_Controller_Native_INST/in_alignment
# add wave -position insertpoint  \
# sim:/Camera_To_AXIS_Converter_tb/Camera_To_AXIS_Converter_tb/DC_A/DC_Native_FORMAT/Display_Controller_Native_INST/locked \
# sim:/Camera_To_AXIS_Converter_tb/Camera_To_AXIS_Converter_tb/DC_A/DC_Native_FORMAT/Display_Controller_Native_INST/in_alignment
# add wave -position insertpoint  \
# sim:/Camera_To_AXIS_Converter_tb/Camera_To_AXIS_Converter_tb/DC_A/FIFO_AE
# add wave -position insertpoint  \
# sim:/Camera_To_AXIS_Converter_tb/Camera_To_AXIS_Converter_tb/DC_A/DC_Native_FORMAT/Display_Controller_Native_INST/s_h_counterx \
# sim:/Camera_To_AXIS_Converter_tb/Camera_To_AXIS_Converter_tb/DC_A/DC_Native_FORMAT/Display_Controller_Native_INST/s_v_counterx
# add wave -position insertpoint  sim:/Camera_To_AXIS_Converter_tb/Camera_To_AXIS_Converter_tb/FIFO_A/usedw
# add wave -position insertpoint  sim:/Camera_To_AXIS_Converter_tb/axi_video_dut_in/DC_A/DC_Native_FORMAT/Display_Controller_Native_INST/s_h_counterx
# add wave -position insertpoint  sim:/Camera_To_AXIS_Converter_tb/axi_video_dut_in/DC_A/DC_Native_FORMAT/Display_Controller_Native_INST/s_v_counterx
# add wave -position insertpoint  sim:/Camera_To_AXIS_Converter_tb/Camera_To_AXIS_Converter_tb/DC_A/DC_Native_FORMAT/Display_Controller_Native_INST/s_h_activex
# add wave -position insertpoint  sim:/Camera_To_AXIS_Converter_tb/Camera_To_AXIS_Converter_tb/DC_A/DC_Native_FORMAT/Display_Controller_Native_INST/s_v_activex

#config wave -signalnamewidth 1
run 800000000 ns
quit
