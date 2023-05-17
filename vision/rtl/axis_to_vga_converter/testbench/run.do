vlib work
vcom -2008 ../VGA_Controller_SHLS.vhd
vlog \
  ../axis_to_vga_converter.v \
  ./pattern_generator_verilog_pattern.v \
  ./axis_to_vga_converter_tb.v

vsim AXIS_To_VGA_Converter_tb 
#uncomment the lines below to view waves

#add wave -position insertpoint  sim:/AXIS_To_VGA_Converter_tb/*
#add wave -position end  sim:/AXIS_To_VGA_Converter_tb/pattern_dut/r_h_counter
#add wave -position end  sim:/AXIS_To_VGA_Converter_tb/pattern_dut/r_v_counter
#add wave -position end  sim:/AXIS_To_VGA_Converter_tb/pattern_dut/data_valid_o
#add wave -position end  sim:/AXIS_To_VGA_Converter_tb/pattern_dut/r_pattern_sel_temp
#add wave -position end  sim:/AXIS_To_VGA_Converter_tb/axi_vga_dut/DC_A/DC_Native_FORMAT/Display_Controller_Native_INST/locked
#add wave -position end  sim:/AXIS_To_VGA_Converter_tb/axi_vga_dut/DC_A/DC_Native_FORMAT/Display_Controller_Native_INST/in_alignment
#add wave -position end  sim:/AXIS_To_VGA_Converter_tb/pattern_dut/r_red
#add wave -position end  sim:/AXIS_To_VGA_Converter_tb/pattern_dut/r_green
#add wave -position end  sim:/AXIS_To_VGA_Converter_tb/pattern_dut/r_blue
#add wave -position end  sim:/AXIS_To_VGA_Converter_tb/axi_vga_dut/DC_A/POP_FIFO
#add wave -position end  sim:/AXIS_To_VGA_Converter_tb/axi_vga_dut/DC_A/DC_Native_FORMAT/Display_Controller_Native_INST/s_h_activex
#add wave -position end  sim:/AXIS_To_VGA_Converter_tb/axi_vga_dut/DC_A/DC_Native_FORMAT/Display_Controller_Native_INST/s_v_activex
#add wave -position end  sim:/AXIS_To_VGA_Converter_tb/axi_vga_dut/DC_A/DC_Native_FORMAT/Display_Controller_Native_INST/s_h_counterx
#add wave -position end  sim:/AXIS_To_VGA_Converter_tb/axi_vga_dut/DC_A/DC_Native_FORMAT/Display_Controller_Native_INST/s_v_counterx
#add wave -position end  sim:/AXIS_To_VGA_Converter_tb/axi_vga_dut/DC_A/SOF

run 800000000 ns
quit

