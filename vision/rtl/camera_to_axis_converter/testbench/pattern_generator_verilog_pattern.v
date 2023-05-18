/*
  This is a modified version of the pattern generator used by the IP team to test the display controller
  on board. This version outputs data in AXI4 stream format to be used by our converters.

  This module generates pixel patterns (like colored boxes or horizontal or vertical lines) based on the
  input pattern select, which is connected to the switches on the board. 

*/

module pattern_generator_verilog_pattern
#( // parameter list
   
    // video format selection
    parameter g_VIDEO_FORMAT = 3,
	
	// Bayer format selection
	parameter g_BAYER_FORMAT = 0
)(
    //AXI-Video: modified ports to test AXI-Video converter
    input tready,
    output tlast, 
    output tvalid,
    output reg tuser,
    output wire [23:0] tdata,
    // input and output Declaration 
  
    // system clock
  input                    sys_clk_i,
	
	// system reset
    input                    reset_i,
	
	// enable
    input                    data_en_i,
	
	//pattern select
	input        [2:0]       pattern_sel_i,
	
	// data enable 
    output wire              data_valid_o,
	
	// End of frame
    output wire              frame_end_o,
	
	//End of line
    output wire              line_end_o,
	
	//Red pixel
  //   output wire [7:0]        red_o,
	
	// // Green Pixel
  //   output wire [7:0]        green_o,
	
	// // Blue Pixel 
  //   output wire [7:0]        blue_o,
	
	// Bayer output 
    output wire [7:0]        bayer_o
);

  wire [7:0] blue_o;
  wire [7:0] green_o;
  wire [7:0] red_o;
  // localparam IDLE    = 3'd0;
  // localparam H_STATE = 3'd1;
  // localparam V_STATE = 3'd2;
  
  // reg  [2:0]               r_state;
  reg                      r_data_en_dly1;   
  reg                      r_data_en_dly2;  
  reg                      r_data_en_re;     
  reg                      r_data_en_fe;  
  //modified   
  wire                      r_h_counting;     
  reg                      r_v_active;       
  reg                      r_vact_dly;      
  wire                     w_vact_fe;        
  wire [15:0]              r_h_resolution;   
  wire [15:0]              w_h_resolution_1; 
  wire [15:0]              r_v_resolution;   
  wire [15:0]              w_v_event3;       
  wire [15:0]              r_h_f_porch;      
  wire [15:0]              r_h_b_porch;     
  wire [15:0]              r_v_f_porch;      
  wire [15:0]              r_v_b_porch;      
  wire [15:0]              r_h_sync_width;   
  wire [15:0]              r_v_sync_width;  
  reg  [15:0]              r_h_counter;      
  reg  [15:0]              r_v_counter;      
  reg  [15:0]              r_frame_counter;  
  wire [15:0]              w_h_count_active; 
  wire [15:0]              w_v_count_active;
  wire [15:0]              w_h_shift;        
  wire [15:0]              w_h_bar_ref;      
  wire [15:0]              w_bar_width;      
  wire [15:0]              w_bar_width2;     
  wire [15:0]              w_bar_width3;     
  wire [15:0]              w_bar_width4;     
  wire [15:0]              w_bar_width5;     
  wire [15:0]              w_bar_width6;     
  wire [15:0]              w_bar_width7;     
  wire [3:0]               w_bar_select;     
  wire [15:0]              w_box_width;      
  wire [15:0]              w_box_width2;     
  wire [15:0]              w_box_width3;     
  wire [15:0]              w_box_width4;     
  wire [15:0]              w_box_width5;     
  wire [15:0]              w_box_width6;     
  wire [15:0]              w_box_width7;    
  wire [7:0]               w_red_p_red;          
  wire [7:0]               w_green_p_red;        
  wire [7:0]               w_blue_p_red; 
  wire [7:0]               w_red_p_green;          
  wire [7:0]               w_green_p_green;        
  wire [7:0]               w_blue_p_green;
  wire [7:0]               w_red_p_blue;          
  wire [7:0]               w_green_p_blue;        
  wire [7:0]               w_blue_p_blue;
  wire [7:0]               w_red_p_vertical;          
  wire [7:0]               w_green_p_vertical;        
  wire [7:0]               w_blue_p_vertical;
  wire [7:0]               w_red_p_horizontal;          
  wire [7:0]               w_green_p_horizontal;        
  wire [7:0]               w_blue_p_horizontal; 
  wire [7:0]               w_red_p_boxes;
  wire [7:0]               w_green_p_boxes;
  wire [7:0]               w_blue_p_boxes;
  reg  [7:0]               r_red;
  reg  [7:0]               r_green;
  reg  [7:0]               r_blue;
  reg  [7:0]               r_bayer;          
  wire [7:0]               w_vh;  
  reg  [7:0]               r_v_bw_count;
  reg  [7:0]               r_h_bw_count;
  reg  [2:0]               r_pattern_sel_temp;
  reg  [2:0]               r_pattern_sel_delay;
  reg                      reset_counters;
  

//AXI-Video: modified for new AXI-Video ports
assign tdata = {blue_o, green_o, red_o};
assign tvalid = 1'b1;
assign tlast = (r_h_counter == w_h_resolution_1) ? 1'b1 : 1'b0;
assign r_h_counting = tready;
assign  data_valid_o = r_h_counting;
assign  red_o        = (r_h_counting) ? r_red   : 'd0;
assign  green_o      = (r_h_counting) ? r_green : 'd0;
assign  blue_o       = (r_h_counting) ? r_blue  : 'd0;
assign  bayer_o      = r_bayer;
assign  line_end_o   = r_data_en_fe;
assign  frame_end_o  = w_vact_fe;

//modified for AXI-Video
always@* 
begin
  if (r_v_counter == 0 && r_h_counter == 0 && tready == 1 )
    tuser <= 1;
  else
    tuser <= 0;
end

always @ (posedge sys_clk_i )begin
  r_pattern_sel_delay <= pattern_sel_i;
  if (r_pattern_sel_delay != pattern_sel_i)
    reset_counters <= 1;
  else
    reset_counters <= 0;
end

generate
if (g_VIDEO_FORMAT == 'd0)
begin
assign r_h_resolution = 'h0500;
assign r_v_resolution = 'h02CF;
assign r_h_f_porch    = 'h006E;
assign r_h_b_porch    = 'h00DC;
assign r_v_f_porch    = 'h0005;
assign r_v_b_porch    = 'h0014;
assign r_h_sync_width = 'h0028;
assign r_v_sync_width = 'h0005;
end
else if(g_VIDEO_FORMAT == 'd1)
begin
assign r_h_resolution = 'h0780;
assign r_v_resolution = 'h0437;
assign r_h_f_porch    = 'h0058;
assign r_h_b_porch    = 'h0094;
assign r_v_f_porch    = 'h0004;
assign r_v_b_porch    = 'h0024;
assign r_h_sync_width = 'h002C;
assign r_v_sync_width = 'h0005;
end
else if(g_VIDEO_FORMAT == 'd2)
begin
assign	r_h_resolution	= 'h0F00;
assign	r_v_resolution	= 'h0869;
assign	r_h_f_porch   	= 'h00B0;
assign	r_h_b_porch     = 'h0128;
assign  r_v_f_porch     = 'h0008;
assign  r_v_b_porch     = 'h0048;
assign	r_h_sync_width  = 'h0058;
assign  r_v_sync_width  = 'h000A;
end
else if(g_VIDEO_FORMAT == 'd3)
begin
assign r_h_resolution	= 'h03C0;
assign r_v_resolution	= 'h0870;
assign r_h_f_porch   	= 'h002C;
assign r_h_b_porch      = 'h004A;
assign r_v_f_porch      = 'h0008;
assign r_v_b_porch      = 'h0048;
assign r_h_sync_width   = 'h0016;
assign r_v_sync_width   = 'h000A;
end
endgenerate


assign w_v_event3       = r_v_f_porch + r_v_sync_width + r_v_b_porch;
assign w_vact_fe        = r_vact_dly & (!r_v_active);
assign w_h_resolution_1 = r_h_resolution - 'd1;
assign w_vh             = {r_v_counter[0],r_h_counter[0]};

generate
always@*
begin
  if(g_BAYER_FORMAT == 'd0)
    r_bayer <= (w_vh == 'b11) ? r_green : (w_vh == 'b10) ? r_red : (w_vh == 'b01) ? r_blue : r_green;
  else if(g_BAYER_FORMAT == 'd1)
    r_bayer <= (w_vh == 'b11) ? r_red : (w_vh == 'b10) ? r_green : (w_vh == 'b01) ? r_green : r_blue;
  else if(g_BAYER_FORMAT == 'd2)
    r_bayer <= (w_vh == 'b11) ? r_green : (w_vh == 'b10) ? r_blue : (w_vh == 'b01) ? r_red : r_green;
  else if(g_BAYER_FORMAT == 3)
    r_bayer <= (w_vh == 'b11) ? r_blue : (w_vh == 'b10) ? r_green : (w_vh == 'b01) ? r_green : r_red;
end  
endgenerate

assign w_bar_width      = r_h_resolution >>> 3;
assign w_box_width      = r_v_resolution >>> 3;
assign w_h_count_active = (pattern_sel_i == 'd0) ? r_h_counter + w_h_shift : r_h_counter;
assign w_h_bar_ref      = (w_h_count_active < r_h_resolution) ? w_h_count_active : (w_h_count_active - r_h_resolution);
assign w_v_count_active = r_v_counter;
assign w_bar_width2     = {w_bar_width[14:0],1'd0};
assign w_bar_width3     = {w_bar_width[14:0],1'd0} + w_bar_width;
assign w_bar_width4     = {w_bar_width[13:0],2'b00};
assign w_bar_width5     = {w_bar_width[13:0],2'b00} + w_bar_width;
assign w_bar_width6     = w_bar_width4 + w_bar_width2;
assign w_bar_width7     = w_bar_width4 + w_bar_width2 + w_bar_width;

assign w_bar_select = (w_h_bar_ref < w_bar_width) ? 'h0  : (w_h_bar_ref < w_bar_width2) ? 'h1 :
                      (w_h_bar_ref < w_bar_width3) ? 'h2 : (w_h_bar_ref < w_bar_width4) ? 'h3 :
					  (w_h_bar_ref < w_bar_width5) ? 'h4 : (w_h_bar_ref < w_bar_width6) ? 'h5 :
					  (w_h_bar_ref < w_bar_width7) ? 'h6 : 'h7;

assign w_box_width2 = {w_box_width[14:0],1'd0};
assign w_box_width3 = {w_box_width[14:0],1'd0} + w_box_width;
assign w_box_width4 = {w_box_width[13:0],2'b00};
assign w_box_width5 = {w_box_width[13:0],2'b00} + w_box_width;
assign w_box_width6 = w_box_width4 + w_box_width2;
assign w_box_width7 = w_box_width4 + w_box_width2 + w_box_width;


assign w_h_shift = (w_v_count_active < w_box_width) ? 'h0 : (w_v_count_active < w_box_width2) ? w_bar_width :
                   (w_v_count_active < w_box_width3) ? w_bar_width2 : (w_v_count_active < w_box_width4) ? w_bar_width3 :
				   (w_v_count_active < w_box_width5) ? w_bar_width4 : (w_v_count_active < w_box_width6) ? w_bar_width5 :
				   (w_v_count_active < w_box_width7) ? w_bar_width6 : w_bar_width7;
				   
// 0 -- shifted 8 colors boxes
// 1 -- red
// 2 -- green
// 3 -- blue
// 4 -- vertical 8 colors
// 5 -- horizontal 8 colors
// 6 -- horizontal BW
// 7 -- vertical BW
// assign  w_red        = (pattern_sel_i == 'd1) ? w_red_p_red   : (pattern_sel_i == 'd2) ? w_red_p_green   : 
                       // (pattern_sel_i == 'd3) ? w_red_p_blue   : (pattern_sel_i == 'd4) ? w_red_p_vertical   :
					   // (pattern_sel_i == 'd5) ? w_red_p_horizontal : (pattern_sel_i == 'd6) ? r_v_bw_count :
					   // (pattern_sel_i == 'd7) ? r_h_bw_count : (pattern_sel_i == 'd0) ? w_red_p_boxes : 'd0;
// assign  w_green      = (pattern_sel_i == 'd1) ? w_green_p_red : (pattern_sel_i == 'd2) ? w_green_p_green : 
                       // (pattern_sel_i == 'd3) ? w_green_p_blue : (pattern_sel_i == 'd4) ? w_green_p_vertical :
					   // (pattern_sel_i == 'd5) ? w_green_p_horizontal : (pattern_sel_i == 'd6) ? r_v_bw_count :
					   // (pattern_sel_i == 'd7) ? r_h_bw_count : (pattern_sel_i == 'd0) ? w_green_p_boxes : 'd0;
// assign  w_blue       = (pattern_sel_i == 'd1) ? w_green_p_red : (pattern_sel_i == 'd2) ? w_blue_p_green  : 
                       // (pattern_sel_i == 'd3) ? w_blue_p_blue  : (pattern_sel_i == 'd4) ? w_blue_p_vertical  :
					   // (pattern_sel_i == 'd5) ? w_blue_p_horizontal  : (pattern_sel_i == 'd6) ? r_v_bw_count :
					   // (pattern_sel_i == 'd7) ? r_h_bw_count : (pattern_sel_i == 'd0) ? w_blue_p_boxes : 'd0;

always@(posedge sys_clk_i or negedge reset_i)
begin
  if(!reset_i)
  begin
    r_pattern_sel_temp <= 'd0;
	r_red              <= 'd0;
	r_green            <= 'd0;
	r_blue             <= 'd0;
  end
  else
  begin
    r_pattern_sel_temp <= (frame_end_o) ? pattern_sel_i : r_pattern_sel_temp;
	case(r_pattern_sel_temp)
	  3'd0 : // shifted 8 color boxes
	  begin
	    r_red    <= w_red_p_boxes;
	    r_green  <= w_green_p_boxes;
	    r_blue   <= w_blue_p_boxes;
	  end
	  3'd1 : // only red
	  begin
	    r_red    <= w_red_p_red;
	    r_green  <= w_green_p_red;
	    r_blue   <= w_blue_p_red;
	  end
	  3'd2 : // only green
	  begin
	    r_red    <= w_red_p_green;
	    r_green  <= w_green_p_green;
	    r_blue   <= w_blue_p_green;
	  end
	  3'd3 : // only blue
	  begin
	    r_red    <= w_red_p_blue;
	    r_green  <= w_green_p_blue;
	    r_blue   <= w_blue_p_blue;
	  end
	  3'd4 : // vertical color
	  begin
	    r_red    <= w_red_p_vertical;
	    r_green  <= w_green_p_vertical;
	    r_blue   <= w_blue_p_vertical;
	  end
	  3'd5 : // horizontal color
	  begin
	    r_red    <= w_red_p_horizontal;
	    r_green  <= w_green_p_horizontal;
	    r_blue   <= w_blue_p_horizontal;
	  end
	  3'd6 :  // horizontal BW
	  begin
	    r_red    <= r_v_bw_count;
	    r_green  <= r_v_bw_count;
	    r_blue   <= r_v_bw_count;
	  end
	  3'd7 : // vertical BW
	  begin
	    r_red    <= r_h_bw_count;
	    r_green  <= r_h_bw_count;
	    r_blue   <= r_h_bw_count;
	  end
	  default :
	  begin
	    r_red    <= 'd0;
	    r_green  <= 'd0;
	    r_blue   <= 'd0;
	  end
	endcase
  end
end


assign w_red_p_red   = (r_h_counting) ? 'd255 : 'd0;
assign w_green_p_red = 'd0;
assign w_blue_p_red  = 'd0;

assign w_red_p_green   = 'd0;
assign w_green_p_green = (r_h_counting) ? 'd255 : 'd0;
assign w_blue_p_green  = 'd0;

assign w_red_p_blue   = 'd0;
assign w_green_p_blue = 'd0;
assign w_blue_p_blue  = (r_h_counting) ? 'd255 : 'd0;

assign w_red_p_vertical   = (w_bar_select == 'd4 | w_bar_select == 'd5 | w_bar_select == 'd6 | w_bar_select == 'd7) ? 'd255 : 'd0;
assign w_green_p_vertical = (w_bar_select == 'd2 | w_bar_select == 'd3 | w_bar_select == 'd6 | w_bar_select == 'd7) ? 'd255 : 'd0;
assign w_blue_p_vertical  = (w_bar_select == 'd1 | w_bar_select == 'd3 | w_bar_select == 'd5 | w_bar_select == 'd7) ? 'd255 : 'd0;

assign w_red_p_horizontal   = (r_v_counter < w_box_width) ? 'd0 :    (r_v_counter < w_box_width2) ? 'd0 :
                              (r_v_counter < w_box_width3) ? 'd0 :   (r_v_counter < w_box_width4) ? 'd0 :
			                  (r_v_counter < w_box_width5) ? 'd255 : (r_v_counter < w_box_width6) ? 'd255 :
			                  (r_v_counter < w_box_width7) ? 'd255 : 'd255;
assign w_green_p_horizontal = (r_v_counter < w_box_width) ? 'd0 :    (r_v_counter < w_box_width2) ? 'd0 :
                              (r_v_counter < w_box_width3) ? 'd255 : (r_v_counter < w_box_width4) ? 'd255 :
			                  (r_v_counter < w_box_width5) ? 'd0 :   (r_v_counter < w_box_width6) ? 'd0 :
			                  (r_v_counter < w_box_width7) ? 'd255 : 'd255;
assign w_blue_p_horizontal  = (r_v_counter < w_box_width) ? 'd0 :  (r_v_counter < w_box_width2) ? 'd255  :
                              (r_v_counter < w_box_width3) ? 'd0 : (r_v_counter < w_box_width4)  ? 'd255 :
			                  (r_v_counter < w_box_width5) ? 'd0 : (r_v_counter < w_box_width6) ? 'd255 :
			                  (r_v_counter < w_box_width7) ? 'd0 : 'd255;

assign w_red_p_boxes   = (w_bar_select == 'd4 | w_bar_select == 'd5 | w_bar_select == 'd6 | w_bar_select == 'd7) ? 'd255 : 'd0;
assign w_green_p_boxes = (w_bar_select == 'd2 | w_bar_select == 'd3 | w_bar_select == 'd6 | w_bar_select == 'd7) ? 'd255 : 'd0;
assign w_blue_p_boxes  = (w_bar_select == 'd1 | w_bar_select == 'd3 | w_bar_select == 'd5 | w_bar_select == 'd7) ? 'd255 : 'd0;





  always@(posedge sys_clk_i or negedge reset_i)
  begin
    if(!reset_i)
	begin
	  r_v_bw_count <= 'd0;
	  r_h_bw_count <= 'd0;
	end
	else
	begin
	  r_h_bw_count <= (r_h_counter == w_h_resolution_1) ? 'd0 : (r_h_bw_count == 'd255) ? 'd0 : (r_h_counting) ? r_h_bw_count + 'd1 : 'd0;
	  r_v_bw_count <= (w_vact_fe) ? 'd0 : (r_v_bw_count == 'd255) ? 'd0 : (r_data_en_re) ? r_v_bw_count + 'd1 : r_v_bw_count;
	end
  end



  always@(posedge sys_clk_i or negedge reset_i)
  begin
    if (!reset_i)
	begin
      r_h_counter    <= 'd0;
      r_data_en_dly1 <= 'd0;
      r_data_en_dly2 <= 'd0;
      r_data_en_re   <= 'd0;
      // r_h_counting   <= 'd0;
	end
    else
	begin
      r_data_en_dly1 <= data_en_i;
      r_data_en_dly2 <= r_data_en_dly1;
      r_data_en_re   <= r_data_en_dly1 & !r_data_en_dly2;
      r_data_en_fe   <= !r_data_en_dly1 & r_data_en_dly2;
      // r_h_counting   <= (r_data_en_re) ? 'd1 : (r_h_counter == w_h_resolution_1) ? 'd0 : r_h_counting;
      if (reset_counters == 1)
        r_h_counter <= 'd0;	
        
      else if (r_h_counting == 1'b1)begin
        if (r_h_counter < w_h_resolution_1)
          r_h_counter <= r_h_counter + 'd1;
        else
          r_h_counter <= 'd0;
      end
      else
        r_h_counter <= r_h_counter;
      // r_h_counter    <= (r_h_counting) ? ((r_h_counter < w_h_resolution_1)r_h_counter + 'd1 : 'd0 ): r_h_counter;
    end
  end


  always@(posedge sys_clk_i or negedge reset_i)
  begin
    if(!reset_i)
      r_v_counter <= 'd0;
	else
	begin
    if(r_h_counter == w_h_resolution_1)
        r_v_counter <= (r_v_counter < r_v_resolution) ? r_v_counter + 'd1 : 'd0;
    else if (reset_counters == 1)
        r_v_counter <= 'd0;
	  else
	    r_v_counter <= r_v_counter;
    end
  end


  always@(posedge sys_clk_i or negedge reset_i)
  begin
    if(!reset_i)
	begin
      r_v_active <= 'd0;
      r_vact_dly <= 'd0;
	end
	else 
	begin
      r_v_active <= (r_v_counter < w_v_event3) ? 'd0 : 'd1;
      r_vact_dly <= r_v_active;
    end
  end



  always@(posedge sys_clk_i or negedge reset_i)
  begin
    if(!reset_i) 
      r_frame_counter <= 'd0;
    else
      r_frame_counter <= (w_vact_fe) ? r_frame_counter + 'd1 : r_frame_counter;
  end

endmodule