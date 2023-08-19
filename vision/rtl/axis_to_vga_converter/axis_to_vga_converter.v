`timescale 1 ns / 1 ns

/*
    This module converts AXI4 stream signals to Video (VGA/HDMI) signals
    (hsync, vsync, etc.) used to display data on monitors. Inside this module
    there is a First-Word Fall-Through (FWFT) FIFO that takes incoming data
    (RGB data, SOF, and EOL) as input and a modified version of the display
    controller IP core (called VGA controller) that generates video timing 
    signals based on the incoming AXI data.

        VGA controller has an alignment phase that checks if incoming data 
    is stable for 2 frames, and goes to a locked state which enables 
    outputting video signals. In case of a broken frame, a new frame 
    beginning at the middle of another one, there is a resync mechanism 
    which handles the video frame being transmitted and fills the remaining
     pixels on the screen before starting to transmit the new frame.   
*/
module AXIS_To_VGA_Converter # (
    parameter C_WIDTH = 8, // component (R, G, B) width
    parameter RESOLUTION = 1,// 0 -> 1280x720 1 -> 1920x1080 2-> 3840x2160 
    //3-> 640x360
    parameter TUSER_WIDTH = 1, //AXIS tuser width. tuser 0 is user for Start Of
    // Frame (SOF). Other bits can be used if needed.
    parameter FIFO_DEPTH = 32,
    parameter DEBUBBLE = 0,  //If there are bubbles (active tready but inactive 
    //tvalid) in the incoming axi stream line, use the DEBUBBLE module at 
    //input to remove bubbles from each line.
    parameter PIXEL_PER_CLK = 1
)
(
    input   reset,//active high
    input   clk,
    // AXIS interfaces
    input   tlast,
    input   tvalid,
    input   [TUSER_WIDTH-1:0] tuser,
    //all RGB components put together. R = C_WIDTH-1:0, G = 2*C_WIDTH-1:C_WIDTH,
    // B: 3*C_WIDTH-1:2*C_WIDTH
    input   [3 * C_WIDTH * PIXEL_PER_CLK-1:0] tdata,
    output  tready_O,
    output  hsync_O,
    output  vsync_O,
    output  data_enable_O,
    output  [PIXEL_PER_CLK * C_WIDTH-1:0] R_O,   
    output  [PIXEL_PER_CLK * C_WIDTH-1:0] G_O,   
    output  [PIXEL_PER_CLK * C_WIDTH-1:0] B_O
);
    wire hsync_counter, hsync_VGA, vsync_counter, vsync_VGA; 
    wire fifo_full;
    wire fifo_almost_empty;
    wire pop_fifo;
    wire [$clog2(FIFO_DEPTH):0] fifo_used_w;
    wire [PIXEL_PER_CLK*3*C_WIDTH+1:0] fifo_o;
    wire display_data;

    wire tready_int_converter, tlast_int_converter, tvalid_int_converter;
    wire data_enable_VGA, data_enable_counter;
    wire [TUSER_WIDTH-1:0] tuser_int_converter;
    wire [PIXEL_PER_CLK*3*C_WIDTH-1:0] tdata_int_converter; 
    wire tready_int_debubble, tlast_int_debubble, tvalid_int_debubble;
    wire [TUSER_WIDTH-1:0] tuser_int_debubble;
    wire [PIXEL_PER_CLK*3*C_WIDTH-1:0] tdata_int_debubble;

    assign tready_int_converter = !fifo_full;
    genvar i;
    generate
        for(i=0; i < PIXEL_PER_CLK; i = i + 1)begin
            // Display yellow if not locked/resync
            assign R_O[(i + 1 ) * C_WIDTH - 1 : i * C_WIDTH] = display_data ? fifo_o [i*3*C_WIDTH+C_WIDTH-1:i*3*C_WIDTH] : {C_WIDTH{1'b1}};
            assign G_O[(i + 1 ) * C_WIDTH - 1 : i * C_WIDTH] = display_data ? fifo_o [i*3*C_WIDTH+2*C_WIDTH-1:i*3*C_WIDTH+C_WIDTH] : {C_WIDTH{1'b1}};
            assign B_O[(i + 1 ) * C_WIDTH - 1 : i * C_WIDTH] = display_data ? fifo_o [i*3*C_WIDTH+3*C_WIDTH-1:i*3*C_WIDTH+2*C_WIDTH] : {C_WIDTH{1'b0}};
        end
    endgenerate

    assign tready_O = (DEBUBBLE) ? tready_int_debubble : tready_int_converter;
    assign tlast_int_converter = (DEBUBBLE) ? tlast_int_debubble : tlast;
    assign tvalid_int_converter = (DEBUBBLE) ? tvalid_int_debubble : tvalid;
    assign tuser_int_converter = (DEBUBBLE) ? tuser_int_debubble : tuser;
    assign tdata_int_converter = (DEBUBBLE) ? tdata_int_debubble : tdata;

    // assign vsync and hsync based on whether VGA controller is in locked mode or not
    assign hsync_O = display_data ? hsync_VGA : hsync_counter;
    assign vsync_O = display_data ? vsync_VGA : vsync_counter;
    assign data_enable_O = display_data ? data_enable_VGA : data_enable_counter;

    AXIS_VGA_fwft_fifo #(
        .width (PIXEL_PER_CLK*3*C_WIDTH+2),
        .widthad ($clog2(FIFO_DEPTH)),
        .depth (FIFO_DEPTH),
        .almost_empty_value (1)
    ) FIFO_A (
        .reset ( reset ),
        .clk ( clk ),
        .clken ( 1'b1 ),
        .full ( fifo_full ),
        .write_en ( tvalid_int_converter ),
        //Combine rgb data, SOF, and EOL and write them inside the FIFO
        .write_data ( {tuser_int_converter[0], tlast_int_converter,
            tdata_int_converter} ),
        .almost_empty ( fifo_almost_empty ),
        .read_en ( pop_fifo ),
        .read_data ( fifo_o ),
        .usedw ( fifo_used_w )
    );
    
    
    VGA_Controller_SHLS #(
        .g_VIDEO_FORMAT(RESOLUTION),
        .g_FIFO_WIDTHAD($clog2(FIFO_DEPTH)),
        .G_FORMAT(0),
        .g_PIXELS_PER_CLK(PIXEL_PER_CLK)
    ) VC_A (
        .SOF(fifo_o[PIXEL_PER_CLK*3*C_WIDTH+1]),
        .EOL(fifo_o[PIXEL_PER_CLK*3*C_WIDTH]),
        .POP_FIFO(pop_fifo),
        .FIFO_FILL_LVL(fifo_used_w),
        .FIFO_AE(fifo_almost_empty),
        .AXI_VALID(tvalid_int_converter),
        .RESETN_I(~reset),
        .SYS_CLK_I(clk),
        .ENABLE_I(1'b1),
        .H_SYNC_O(hsync_VGA),
        .V_SYNC_O(vsync_VGA),
        .DATA_ENABLE_O(data_enable_VGA),
        .DISPLAY_DATA_O(display_data)
    );


    // Added for yellow screen
 VGA_Counter # (
    .RESOLUTION(RESOLUTION),
    .PIXEL_PER_CLK(PIXEL_PER_CLK)
 ) V_Count_A(
    .reset(reset),//active high
    .clk(clk),
    .hsync_O(hsync_counter),
    .vsync_O(vsync_counter),
    .data_enable_O(data_enable_counter)
);


    generate 
        if (DEBUBBLE)
                AXIS_Debubble # (
                    .C_WIDTH(C_WIDTH), // component (R, G, B) width
                    .RESOLUTION(RESOLUTION),// 0 -> 1280x720 1 -> 1920x1080 
                                            //2-> 3840x2160 3-> 640x360
                    .TUSER_WIDTH(TUSER_WIDTH), //AXIS tuser width. tuser 0 is 
                        //user for Start Of Frame (SOF). Other bits can be used
                        // if needed.)
                    .PIXEL_PER_CLK(PIXEL_PER_CLK)
                    ) AXIS_DB_A
                (
                    .reset(reset),//active high
                    .clk(clk),
                    // input AXIS ports
                    .tlast_I(tlast),
                    .tvalid_I(tvalid),
                    .tuser_I(tuser),
                    //all RGB components put together. R = C_WIDTH-1:0, 
                    //G = 2*C_WIDTH-1:C_WIDTH, B: 3*C_WIDTH-1:2*C_WIDTH
                    .tdata_I(tdata),
                    .tready_I(tready_int_debubble), 
                    // output AXIS ports
                    .tlast_O(tlast_int_debubble),
                    .tvalid_O(tvalid_int_debubble),
                    .tuser_O(tuser_int_debubble),
                    .tdata_O(tdata_int_debubble),
                    .tready_O(tready_int_converter)
                );
    endgenerate


endmodule





/*
    This module removes bubbles from an AXIS video line.
    VGA controller has an alignment phase that checks if incoming data is 
    stable for 2 frames, and goes to a locked state which enables outputting 
    video signals. In case of a broken frame, a new frame beginning at the 
    middle of another one, there is a resync mechanism which handles the video 
    frame being transmitted and fills the remaining pixels on the screen before 
    starting to transmit the new frame.   
*/
module AXIS_Debubble # (
    parameter C_WIDTH = 8, // component (R, G, B) width
    parameter RESOLUTION = 1,// 0 -> 1280x720 1 -> 1920x1080 2-> 3840x2160 
            //3-> 640x360
    parameter TUSER_WIDTH = 1, //AXIS tuser width. tuser 0 is user for Start Of 
            //Frame (SOF). Other bits can be used if needed.)
    parameter PIXEL_PER_CLK = 1 
)
(
    input reset,//active high
    input clk,
    // input AXIS ports
    input tlast_I,
    input tvalid_I,
    input   [TUSER_WIDTH-1:0] tuser_I,
    //all RGB components put together. R = C_WIDTH-1:0, G = 2*C_WIDTH-1:C_WIDTH,
    // B: 3*C_WIDTH-1:2*C_WIDTH
    input   [PIXEL_PER_CLK*3*C_WIDTH-1:0] tdata_I,
    output  tready_I, 
    // output AXIS ports
    output tlast_O,
    output tvalid_O,
    output   [TUSER_WIDTH-1:0] tuser_O,
    //all RGB components put together. R = C_WIDTH-1:0, G = 2*C_WIDTH-1:C_WIDTH,
    // B: 3*C_WIDTH-1:2*C_WIDTH
    output   [PIXEL_PER_CLK*3*C_WIDTH-1:0] tdata_O,
    input  tready_O 
);
    //set fifo depth to one line of pixels, based on RESOLUTION
    localparam FIFO_DEPTH = (RESOLUTION ==  0) ? (1280 / PIXEL_PER_CLK) : (RESOLUTION ==  1) ? 
                (1920 / PIXEL_PER_CLK) : (RESOLUTION ==  2) ? (3840 / PIXEL_PER_CLK) : (640 / PIXEL_PER_CLK);

    wire [$clog2(FIFO_DEPTH+2):0] fifo_used_w;
    wire fifo_almost_full, fifo_full;

    //The indices which determine the buffer read/write location
    reg [$clog2(FIFO_DEPTH)-1 : 0] transfer_index, write_index;
    reg transfer, eol_received;

    //backpressure the incloming stream if the fifo gets full while downstream 
    //is backpressuring
    assign tready_I = (fifo_used_w <FIFO_DEPTH+1)  || (transfer && tready_O);
    assign tvalid_O = transfer;
    
    //transfer signal state machine
    always @(posedge clk) begin
        if (reset)
            transfer <= 0;
        else begin
            // if(!transfer && fifo_almost_full && tvalid_I)
            if(!transfer && fifo_almost_full)
            // if(!transfer && fifo_full)
                transfer <= 1'b1;
            else if(transfer && (transfer_index == FIFO_DEPTH-1)) begin
                if (fifo_almost_full && tvalid_I)
                    transfer <= 1'b1;
                else 
                    transfer <= 1'b0;
            end
        end
    end

    always @(posedge clk)begin
        if (reset)
            transfer_index <= 0;
        else begin
            //tready_O is used to account for backpressure from the downstream
            if(transfer && tready_O)begin
                if(transfer_index == FIFO_DEPTH-1)
                    transfer_index <= 0;
                else
                    transfer_index <= transfer_index + 1; 
            end
        end
    end

    AXIS_VGA_fwft_fifo #(
        .width (PIXEL_PER_CLK*3*C_WIDTH+2),
        .widthad ($clog2(FIFO_DEPTH+2)),
        .depth (FIFO_DEPTH+2),
        .almost_empty_value (1),
        .almost_full_value(FIFO_DEPTH)
    ) FIFO_A (
        .reset ( reset ),
        .clk ( clk ),
        .clken ( 1'b1 ),
        .almost_full ( fifo_almost_full ),
        .write_en ( tvalid_I ),
        //concat rgb data, SOF, and EOL and write them inside the FIFO
        .write_data ( {tuser_I[0], tlast_I,tdata_I} ),
        // .empty ( fifo_empty ),
        .read_en ( transfer && tready_O ),
        .full(fifo_full),
        .read_data ( {tuser_O[0], tlast_O,tdata_O} ),
        .usedw ( fifo_used_w )
    );

endmodule












// ©2022 Microchip Technology Inc. and its subsidiaries
//
// Subject to your compliance with these terms, you may use this Microchip
// software and any derivatives exclusively with Microchip products. You are
// responsible for complying with third party license terms applicable to your
// use of third party software (including open source software) that may
// accompany this Microchip software. SOFTWARE IS “AS IS.” NO WARRANTIES,
// WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING
// ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR
// A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY
// INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST
// OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED,
// EVEN IF MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE
// FORESEEABLE.  TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP’S TOTAL
// LIABILITY ON ALL CLAIMS LATED TO THE SOFTWARE WILL NOT EXCEED AMOUNT OF
// FEES, IF ANY, YOU PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE. MICROCHIP
// OFFERS NO SUPPORT FOR THE SOFTWARE. YOU MAY CONTACT MICROCHIP AT
// https://www.microchip.com/en-us/support-and-training/design-help/client-support-services
// TO INQUIRE ABOUT SUPPORT SERVICES AND APPLICABLE FEES, IF AVAILABLE.

module AXIS_VGA_fwft_fifo # (
    parameter width = 32,
    parameter widthad = 3,
    parameter depth = 8,
    parameter almost_empty_value = 2,
    parameter almost_full_value = 2,
    parameter name = "",
    parameter ramstyle = "",
    parameter disable_full_empty_check = 0
) (
    input reset,
    input clk,
    input clken,
    // Interface to source.
    output full,
    output almost_full,
    input write_en,
    input [width-1:0] write_data,
    // Interface to sink.
    output empty,
    output almost_empty,
    input read_en,
    output [width-1:0] read_data,
    // Number of words stored in the FIFO.
    output [widthad:0] usedw
);

generate
if (depth == 0) begin
    assign full = !read_en;
    assign almost_full = 1'b1;
    assign empty = !write_en;
    assign almost_empty = 1'b1;
    assign read_data = write_data;
end else if (ramstyle == "block" || ramstyle == "") begin
    AXIS_VGA_fwft_fifo_bram # (
      .width (width),
      .widthad (widthad),
      .depth (depth),
      .almost_empty_value (almost_empty_value),
      .almost_full_value (almost_full_value),
      .name (name),
      .ramstyle (ramstyle),
      .disable_full_empty_check (disable_full_empty_check)
    ) fwft_fifo_bram_inst (
      .reset (reset),
      .clk (clk),
      .clken (clken),
      .full (full),
      .almost_full (almost_full),
      .write_en (write_en),
      .write_data (write_data),
      .empty (empty),
      .almost_empty (almost_empty),
      .read_en (read_en),
      .read_data (read_data),
      .usedw (usedw)
    );
end else begin // if (ramstyle == distributed || ramstyle == registers)
    AXIS_VGA_fwft_fifo_lutram # (
      .width (width),
      .widthad (widthad),
      .depth (depth),
      .almost_empty_value (almost_empty_value),
      .almost_full_value (almost_full_value),
      .name (name),
      .ramstyle (ramstyle),
      .disable_full_empty_check (disable_full_empty_check)
    ) fwft_fifo_lutram_inst (
      .reset (reset),
      .clk (clk),
      .clken (clken),
      .full (full),
      .almost_full (almost_full),
      .write_en (write_en),
      .write_data (write_data),
      .empty (empty),
      .almost_empty (almost_empty),
      .read_en (read_en),
      .read_data (read_data),
      .usedw (usedw)
    );
end
endgenerate

/* synthesis translate_off */

localparam NUM_CYCLES_BETWEEN_STALL_WARNINGS = 1000000;
integer num_empty_stall_cycles = 0;
integer num_full_stall_cycles = 0;
integer num_full_cycles = 0;

always @ (posedge clk) begin
    if (num_empty_stall_cycles == NUM_CYCLES_BETWEEN_STALL_WARNINGS) begin
        num_empty_stall_cycles = 0;
        if (name == "")
            $display("Warning: fifo_read() has been stalled for %d cycles due to FIFO being empty.", NUM_CYCLES_BETWEEN_STALL_WARNINGS);
        else
            $display("Warning: fifo_read() from %s has been stalled for %d cycles due to FIFO being empty.", name, NUM_CYCLES_BETWEEN_STALL_WARNINGS);
    end else if (empty & read_en)
        num_empty_stall_cycles = num_empty_stall_cycles + 1;
    else
        num_empty_stall_cycles = 0;


    if (num_full_stall_cycles == NUM_CYCLES_BETWEEN_STALL_WARNINGS) begin
        num_full_stall_cycles = 0;
        if (name == "")
            $display("Warning: fifo_write() has been stalled for %d cycles due to FIFO being full.", NUM_CYCLES_BETWEEN_STALL_WARNINGS);
        else
            $display("Warning: fifo_write() to %s has been stalled for %d cycles due to FIFO being full.", name, NUM_CYCLES_BETWEEN_STALL_WARNINGS);
    end else if (full & write_en)
        num_full_stall_cycles = num_full_stall_cycles + 1;
    else
        num_full_stall_cycles = 0;


    if (num_full_cycles == NUM_CYCLES_BETWEEN_STALL_WARNINGS) begin
        num_full_cycles = 0;
        $display("Warning: FIFO %s has been full for %d cycles. The circuit may have been stalled with no progress.", name, NUM_CYCLES_BETWEEN_STALL_WARNINGS);
        $display("         Please examine the simulation waveform and increase the corresponding FIFO depth if necessary.");
    end else if (full)
        num_full_cycles = num_full_cycles + 1;
    else
        num_full_cycles = 0;
end

/* synthesis translate_on */


endmodule

//--------------------------------------------
// Block-RAM-based FWFT FIFO implementation.
//--------------------------------------------

module AXIS_VGA_fwft_fifo_bram # (
    parameter width = 32,
    parameter widthad = 4,
    parameter depth = 16,
    parameter almost_empty_value = 2,
    parameter almost_full_value = 2,
    parameter name = "",
    parameter ramstyle = "block",
    parameter disable_full_empty_check = 0
) (
    input reset,
    input clk,
    input clken,
    // Interface to source.
    output reg full,
    output almost_full,
    input write_en,
    input [width-1:0] write_data,
    // Interface to sink.
    output reg empty,
    output almost_empty,
    input read_en,
    output [width-1:0] read_data,
    // Number of words stored in the FIFO.
    output reg [widthad:0] usedw
);


// The output data from RAM.
wire [width-1:0] ram_data;
// An extra register to either sample fifo output or write_data.
reg [width-1:0] sample_data;
// Use a mealy FSM with 4 states to handle the special cases.
localparam [1:0] EMPTY = 2'd0;
localparam [1:0] FALL_THRU = 2'd1;
localparam [1:0] LEFT_OVER = 2'd2;
localparam [1:0] STEADY = 2'd3;
reg [1:0] state = 2'd0;

always @ (posedge clk) begin
    if (reset) begin
        state <= EMPTY;
        sample_data <= {width{1'b0}};
    end else begin
        case (state)
            EMPTY:
                if (write_en) begin
                    state <= FALL_THRU;
                    sample_data <= write_data;
                end else begin
                    state <= EMPTY;
                    sample_data <= {width{1'bX}};
                end
            FALL_THRU:  // usedw must be 1.
                if (write_en & ~read_en) begin
                    state <= STEADY;
                    sample_data <= {width{1'bX}};
                end else if (~write_en & read_en) begin
                    state <= EMPTY;
                    sample_data <= {width{1'bX}};
                end else if (~write_en & ~read_en) begin
                    state <= STEADY;
                    sample_data <= {width{1'bX}};
                end else begin // write_en & read_en
                    state <= FALL_THRU;
                    sample_data <= write_data;
                end
            LEFT_OVER:  // usedw must be > 1.
                if (usedw == 1 & read_en & ~write_en) begin
                    state <= EMPTY;
                    sample_data <= {width{1'bX}};
                end else if (usedw == 1 & read_en & write_en) begin
                    state <= FALL_THRU;
                    sample_data <= write_data;
                end else if (read_en) begin
                    state <= STEADY;
                    sample_data <= {width{1'bX}};
                end else begin // ~read_en
                    state <= LEFT_OVER;
                    sample_data <= sample_data;
                end
            STEADY:
                if (usedw == 1 & read_en & ~write_en) begin
                    state <= EMPTY;
                    sample_data <= {width{1'bX}};
                end else if (usedw == 1 & read_en & write_en) begin
                    state <= FALL_THRU;
                    sample_data <= write_data;
                end else if (~read_en) begin
                    state <= LEFT_OVER; // Only transition to LEFT_OVER.
                    sample_data <= ram_data;
                end else begin
                    state <= STEADY;
                    sample_data <= {width{1'bX}};
                end
            default: begin
                 state <= EMPTY;
                 sample_data <= {width{1'b0}};
            end
        endcase
    end
end

assign read_data = (state == LEFT_OVER || state == FALL_THRU) ? sample_data
                                                              : ram_data;

wire write_handshake = (write_en & ~full);
wire read_handshake = (read_en & ~empty);

// Full and empty.
generate
if (disable_full_empty_check) begin
    always @ (posedge clk) begin full <= 0; empty <= 0; end
end else begin
    always @ (posedge clk) begin
      if (reset) begin
        full <= 0;
        empty <= 1;
      end else begin
        full <= (full & ~read_handshake) | ((usedw == depth - 1) & (write_handshake & ~read_handshake));
        empty <= (empty & ~write_handshake) | ((usedw == 1) & (read_handshake & ~write_handshake));
      end
    end
end
endgenerate

// FIXME: may want to make almost_full/empty registers too.
assign almost_full = (usedw >= almost_full_value);
assign almost_empty= (usedw <= almost_empty_value);

// Read/Write port addresses.
reg [widthad-1:0] write_address = 0;
reg [widthad-1:0] read_address = 0;

function [widthad-1:0] increment;
    input [widthad-1:0] address;
    input integer depth;
    increment = (address == depth - 1) ? 0 : address + 1;
endfunction

always @ (posedge clk) begin
    if (reset) begin
        write_address <= 0;
        read_address <= 0;
    end else begin
        if (write_en & ~full)
            write_address <= increment(write_address, depth);
        if ((read_en & ~empty & ~(usedw==1)) | (state == FALL_THRU))
            read_address <= increment(read_address, depth);
    end
end

// Usedw.
always @ (posedge clk) begin
    if (reset) begin
        usedw <= 0;
    end else begin
        if (write_handshake & read_handshake)
            usedw <= usedw;
        else if (write_handshake)
            usedw <= usedw + 1;
        else if (read_handshake)
            usedw <= usedw - 1;
        else
            usedw <= usedw;
    end
end

/* synthesis translate_off */
initial
if ( widthad < $clog2(depth) ) begin
    $display("Error: Invalid FIFO parameter, widthad=%d, depth=%d.",
             widthad, depth);
    $finish;
end

always @ (posedge clk) begin
    if ( (state == EMPTY &&
            (usedw != 0 || read_address != write_address)) ||
         (state == FALL_THRU &&
            ((read_address + usedw) % depth != write_address)) ||
         (state == STEADY &&
            ((read_address + usedw - 1) % depth != write_address)) ||
         (state == LEFT_OVER &&
            ((read_address + usedw - 1) % depth != write_address)) ) begin
        $display("Error: FIFO read/write address mismatch with usedw.");
        $display("\t rd_addr=%d, wr_addr=%d, usedw=%d, state=%d.",
                    read_address, write_address, usedw, state);
        $finish;
    end
    if (usedw > depth) begin
        $display("Error: usedw goes out of range.");
        $finish;
    end
end

/* synthesis translate_on */

/// Instantiation of inferred ram.
AXIS_VGA_simple_ram_dual_port_fifo ram_dual_port_inst (
  .clk( clk ),
  // Write port, i.e., interface to source.
  .waddr( write_address ),
  .wr_en( write_en & ~full ),
  .din( write_data ),
  // Read port, i.e., interface to sink.
  .raddr( read_address ),
  .dout( ram_data )
);
defparam ram_dual_port_inst.width = width;
defparam ram_dual_port_inst.widthad = widthad;
defparam ram_dual_port_inst.numwords = depth;

endmodule

//--------------------------------------------
// LUT-RAM-based FWFT FIFO implementation.
//--------------------------------------------

module AXIS_VGA_fwft_fifo_lutram # (
    parameter width = 32,
    parameter widthad = 4,
    parameter depth = 16,
    parameter almost_empty_value = 2,
    parameter almost_full_value = 2,
    parameter name = "",
    parameter ramstyle = "",
    parameter disable_full_empty_check = 0
) (
    input reset,
    input clk,
    input clken,
    // Interface to source.
    output reg full,
    output almost_full,
    input write_en,
    input [width-1:0] write_data,
    // Interface to sink.
    output reg empty,
    output almost_empty,
    input read_en,
    output [width-1:0] read_data,
    // Number of words stored in the FIFO.
    output reg [widthad:0] usedw
);

wire write_handshake = (write_en & ~full);
wire read_handshake = (read_en & ~empty);

// Full and empty.
generate
if (disable_full_empty_check) begin
    always @ (posedge clk) begin full <= 0; empty <= 0; end
end else begin
    always @ (posedge clk) begin
      if (reset) begin
        full <= 0;
        empty <= 1;
      end else begin
        full <= (full & ~read_handshake) | ((usedw == depth - 1) & (write_handshake & ~read_handshake));
        empty <= (empty & ~write_handshake) | ((usedw == 1) & (read_handshake & ~write_handshake));
      end
    end
end
endgenerate

// FIXME: may want to make almost_full/empty registers too.
assign almost_full = (usedw >= almost_full_value);
assign almost_empty= (usedw <= almost_empty_value);

// Read/Write port addresses.
reg [widthad-1:0] write_address = 0;
reg [widthad-1:0] read_address = 0;

function [widthad-1:0] increment;
    input [widthad-1:0] address;
    input integer depth;
    increment = (address == depth - 1) ? 0 : address + 1;
endfunction

always @ (posedge clk) begin
    if (reset) begin
        write_address <= 0;
        read_address <= 0;
    end else begin
        if (write_en & ~full)
            write_address <= increment(write_address, depth);
        if (read_en & ~empty)
            read_address <= increment(read_address, depth);
    end
end

// Usedw.
always @ (posedge clk) begin
    if (reset) begin
        usedw <= 0;
    end else begin
        if (write_handshake & read_handshake)
            usedw <= usedw;
        else if (write_handshake)
            usedw <= usedw + 1;
        else if (read_handshake)
            usedw <= usedw - 1;
        else
            usedw <= usedw;
    end
end

/* synthesis translate_off */
initial
if ( widthad < $clog2(depth) ) begin
    $display("Error: Invalid FIFO parameter, widthad=%d, depth=%d.",
             widthad, depth);
    $finish;
end

always @ (posedge clk) begin
    if ((read_address + usedw) % depth != write_address) begin
        $display("Error: FIFO read/write address mismatch with usedw.");
        $display("\t rd_addr=%d, wr_addr=%d, usedw=%d.",
                    read_address, write_address, usedw);
        $finish;
    end
    if (usedw > depth) begin
        $display("Error: usedw goes out of range.");
        $finish;
    end
end

/* synthesis translate_on */

/// Instantiation of inferred ram.
AXIS_VGA_lutram_dual_port_fifo lutram_dual_port_inst (
    .clk( clk ),
    .clken( clken ),
    // Write port, i.e., interface to source.
    .address_a( write_address ),
    .wren_a( write_en & ~full ),
    .data_a( write_data ),
    // Read port, i.e., interface to sink.
    .address_b( read_address ),
    .q_b( read_data )
);
defparam lutram_dual_port_inst.width = width;
defparam lutram_dual_port_inst.widthad = widthad;
defparam lutram_dual_port_inst.numwords = depth;
defparam lutram_dual_port_inst.ramstyle = ramstyle;

endmodule





`timescale 1 ns / 1 ns
// ©2022 Microchip Technology Inc. and its subsidiaries
//
// Subject to your compliance with these terms, you may use this Microchip
// software and any derivatives exclusively with Microchip products. You are
// responsible for complying with third party license terms applicable to your
// use of third party software (including open source software) that may
// accompany this Microchip software. SOFTWARE IS “AS IS.” NO WARRANTIES,
// WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING
// ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR
// A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY
// INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST
// OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED,
// EVEN IF MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE
// FORESEEABLE.  TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP’S TOTAL
// LIABILITY ON ALL CLAIMS LATED TO THE SOFTWARE WILL NOT EXCEED AMOUNT OF
// FEES, IF ANY, YOU PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE. MICROCHIP
// OFFERS NO SUPPORT FOR THE SOFTWARE. YOU MAY CONTACT MICROCHIP AT
// https://www.microchip.com/en-us/support-and-training/design-help/client-support-services
// TO INQUIRE ABOUT SUPPORT SERVICES AND APPLICABLE FEES, IF AVAILABLE.

// Adapted from Example 5 in:
// Inferring Microchip PolarFire RAM Blocks
// Synopsys® Application Note, April 2021
module AXIS_VGA_simple_ram_dual_port_fifo # (
  parameter  width    = 1'd0,
  parameter  widthad  = 1'd0,
  parameter  numwords = 1'd0
) (
  input clk,
  input [(width-1):0] din,
  input wr_en,
  input [(widthad-1):0] waddr, raddr,
  output [(width-1):0] dout
);
  reg [(widthad-1):0] raddr_reg;
  reg [(width-1):0] mem [(numwords-1):0];

  assign dout = mem[raddr_reg];

  always @ (posedge clk) begin
    raddr_reg <= raddr;
    if (wr_en) begin
      mem[waddr] <= din;
    end
  end

endmodule

// Zero-cycle read latency and One-cycle write latency.
// Port A is for write, Port B is for read.
module AXIS_VGA_lutram_dual_port_fifo # (
    parameter  width = 1'd0,
    parameter  widthad = 1'd0,
    parameter  numwords = 1'd0,
    parameter  ramstyle = ""
) (
    input  clk,
    input  clken,
    input [widthad - 1:0] address_a,
    input  wren_a,
    input [width - 1:0] data_a,
    input [widthad - 1:0] address_b,
    output [width - 1:0] q_b
);

(* ramstyle = ramstyle, ram_style = ramstyle *) reg [width - 1:0] ram [numwords - 1:0];

assign q_b = ram[address_b];

always @ (posedge clk) begin
  if (clken & wren_a) ram[address_a] <= data_a;
end

endmodule


// Added for yellow screen
module VGA_Counter # (
    parameter RESOLUTION = 1,// 0 -> 1280x720 1 -> 1920x1080 2-> 3840x2160 
    //3-> 640x360
    parameter PIXEL_PER_CLK = 1
)
(
    input   reset,//active high
    input   clk,
    output reg hsync_O,
    output reg vsync_O,
    output data_enable_O
);

// Counter values used for the yellow screen
wire [15:0] h_resolution, v_resolution, h_f_porch, h_b_porch, v_f_porch,
            v_b_porch, h_sync_width, v_sync_width;
reg [15:0] h_counter, v_counter;
wire [15:0] h_event1, h_event2, h_event3, v_event1, v_event2, 
            v_event3, v_blank, h_total, v_total;
reg h_active_dly, h_active_fe, h_active, v_active; 




// Assign active/blanking period values based on resolution and pixels per clock
generate
  if (RESOLUTION == 0 && PIXEL_PER_CLK == 1) begin : FORMAT_1280x720
    assign h_resolution = 16'h0500;
    assign v_resolution = 16'h02D0;
    assign h_f_porch = 16'h006E;
    assign h_b_porch = 16'h00DC;
    assign v_f_porch = 16'h0005;
    assign v_b_porch = 16'h0014;
    assign h_sync_width = 16'h0028;
    assign v_sync_width = 16'h0005;
  end

  if (RESOLUTION == 0 && PIXEL_PER_CLK == 4) begin : FORMAT_1280x720_4p
    assign h_resolution = 16'h0140;
    assign v_resolution = 16'h02D0;
    assign h_f_porch = 16'h001B;
    assign h_b_porch = 16'h0037;
    assign v_f_porch = 16'h0005;
    assign v_b_porch = 16'h0014;
    assign h_sync_width = 16'h000A;
    assign v_sync_width = 16'h0005;
  end

  if (RESOLUTION == 1 && PIXEL_PER_CLK == 1) begin : FORMAT_1920x1080
    assign h_resolution = 16'h0780;
    assign v_resolution = 16'h0438;
    assign h_f_porch = 16'h0058;
    assign h_b_porch = 16'h0094;
    assign v_f_porch = 16'h0004;
    assign v_b_porch = 16'h0024;
    assign h_sync_width = 16'h002C;
    assign v_sync_width = 16'h0005;
  end

  if (RESOLUTION == 1 && PIXEL_PER_CLK == 4) begin : FORMAT_1920x1080_4p
    assign h_resolution = 16'h01E0;
    assign v_resolution = 16'h0438;
    assign h_f_porch = 16'h0016;
    assign h_b_porch = 16'h0025;
    assign v_f_porch = 16'h0004;
    assign v_b_porch = 16'h0024;
    assign h_sync_width = 16'h000B;
    assign v_sync_width = 16'h0005;
  end

  if (RESOLUTION == 2 && PIXEL_PER_CLK == 1) begin : FORMAT_3840x2160
    assign h_resolution = 16'h0F00;
    assign v_resolution = 16'h0870;
    assign h_f_porch = 16'h00B0;
    assign h_b_porch = 16'h0128;
    assign v_f_porch = 16'h0008;
    assign v_b_porch = 16'h0048;
    assign h_sync_width = 16'h0058;
    assign v_sync_width = 16'h000A;
  end

  if (RESOLUTION == 2 && PIXEL_PER_CLK == 4) begin : FORMAT_3840x2160_4p
    assign h_resolution = 16'h03C0;
    assign v_resolution = 16'h0870;
    assign h_f_porch = 16'h002C;
    assign h_b_porch = 16'h004A;
    assign v_f_porch = 16'h0008;
    assign v_b_porch = 16'h0048;
    assign h_sync_width = 16'h0016;
    assign v_sync_width = 16'h000A;
  end

  if (RESOLUTION == 3 && PIXEL_PER_CLK == 1) begin : FORMAT_640x360
    assign h_resolution = 16'h0280;
    assign v_resolution = 16'h0168;
    assign h_f_porch = 16'h0058;
    assign h_b_porch = 16'h0094;
    assign v_f_porch = 16'h0004;
    assign v_b_porch = 16'h0024;
    assign h_sync_width = 16'h002C;
    assign v_sync_width = 16'h0005;
  end
endgenerate

assign h_event1 = h_f_porch;
assign h_event2 = h_f_porch + h_sync_width;
assign h_event3 = h_f_porch + h_sync_width + h_b_porch;
assign v_event1 = v_f_porch;
assign v_event2 = v_f_porch + v_sync_width;
assign v_event3 = v_f_porch + v_sync_width + v_b_porch;
assign v_blank  = v_sync_width + v_b_porch;
assign h_total  = h_resolution + h_f_porch + h_b_porch + h_sync_width - 1;
assign v_total  = v_resolution + v_f_porch + v_b_porch + v_sync_width - 1;

assign data_enable_O = h_active & v_active;

// V_vounter incrementation
always @ (posedge clk) begin
  if (reset) begin
    h_active_dly <= 0;
    h_active_fe  <= 0;
    v_counter    <= v_event3;
  end
  else begin
    h_active_dly <= h_active;
    h_active_fe  <= (!h_active & h_active_dly);
    if (h_active_fe == 1) begin
      if (v_counter < v_total) 
        v_counter <= v_counter + 1;
      else
        v_counter <= 0;
    end
  end
end




// H_counter incrementation
always @ (posedge clk) begin
  if (reset)
    h_counter <= 0;
  else begin
    if(h_counter < h_total) 
      h_counter <= h_counter + 1;
    else
      h_counter <= 0;
  end
end


// H timing gen
always @ (posedge clk) begin
  if (reset) begin
    hsync_O    <= 0;
    h_active <= 0;
  end
  else begin
    if(h_counter < h_event1) begin
        hsync_O <= 0;
        h_active <= 0;
    end
    else if(h_counter < h_event2) begin
        hsync_O    <= 1;
        h_active <= 0;
    end
    else if(h_counter < h_event3) begin
        hsync_O    <= 0;
        h_active <= 0;
    end
    else begin
        hsync_O    <= 0;
        h_active <= 1;
    end
  end
end


// V timing gen
always @ (posedge clk) begin
  if (reset) begin
    vsync_O    <= 0;
    v_active <= 0;
  end
  else begin
    if(v_counter < v_event1) begin
        vsync_O    <= 0;
        v_active <= 0;
    end
    else if(v_counter < v_event2) begin
        vsync_O    <= 1;
        v_active <= 0;
    end
    else if(v_counter < v_event3) begin
        vsync_O    <= 0;
        v_active <= 0;
    end
    else begin
        vsync_O <= 0;
        v_active <= 1;
    end
  end
end

endmodule
