`timescale 1 ps / 1 ps
 /*
    This module converts Camera signals (Data, frame start, data valid) in Bayer format to 
    AXI4-Stream video signals. This module uses a Dual Clock (DC) FIFO to store incoming data.
 */
module Camera_To_AXIS_Converter # (
    parameter C_WIDTH = 8, // component width
    // AXI o_tuser width. o_tuser 0 is user for Start Of 
    // Frame (SOF) and o_tuser 1 is used for End Of Frame (EOF). Other bits can be used if needed.    
    parameter TUSER_WIDTH = 1, 
    // Async FIFO depth should be a power of 2 per module requirement)            
    parameter FIFO_DEPTH = 4096,
    parameter PIXEL_PER_CLK = 1,
    // If there are bubbles (active tready but inactive 
    // i_tvalid) in the incoming axi stream line, use the DEBUBBLE module at 
    // input to remove bubbles from each line.
    parameter DEBUBBLE = 0,
    parameter LINES_TO_SKIP = 1
) (
    input   i_video_reset,//active high
    input   i_video_clk,
    input   i_frame_start,
    input   i_data_valid,
    input   [PIXEL_PER_CLK * C_WIDTH - 1 : 0] i_Data,
    input   [11:0]   i_vres,
    input   [11:0]   i_hres,
    output  reg [15:0]   o_frames_skipped, /* synthesis syn_preserve=1 */
    output  reg o_pushback_detected, /* synthesis syn_preserve=1 */
    // AXI interfaces
    input   i_axis_reset,//active high
    input   i_axis_clk,
    input   i_tready,
    output  o_tlast,
    output  o_tvalid,
    output  [TUSER_WIDTH-1:0] o_tuser,
    //all RGB components put together. R = C_WIDTH-1:0, G = 2*C_WIDTH-1:C_WIDTH,
    // B: 3*C_WIDTH-1:2*C_WIDTH 
    output  [PIXEL_PER_CLK * C_WIDTH - 1 : 0] o_tdata
);

    wire EOF, EOL, SOF, s, r;
    wire fifo_o_valid;

    reg SOF_delay;
    reg q;
    reg frame_start_delay;
    reg data_valid_fe, data_valid_re, frame_start_re;
    reg data_valid_delay [1:0];
    reg  [PIXEL_PER_CLK * C_WIDTH - 1 : 0] data_i_delay[1:0];
    reg [11:0] v_count;
    reg [12:0] flush_v_count, flush_h_count;
    reg skip_frame;
    reg i_frame_start_reg;
    assign o_tvalid = fifo_o_valid & !skip_frame;
    assign EOL = data_valid_fe;
    assign SOF = data_valid_re & q;


    //SR flip-flop
    assign s = frame_start_re;
    assign r = data_valid_re && (v_count == 1);
    wire axis_tvalid;

    
    always @ (posedge i_video_clk or posedge i_video_reset) begin
        if (i_video_reset)
            // We assume the first pixel after video_reset is the begining of a frame 
            q <= 1;
        else begin
            if (s == 1)
                q <= 1;
            else if (r == 1)
                q <= 0;
            else
                q <= q;
        end
    end


    
    
    
    //v_count, used for generating EOF
    always @ (posedge i_video_clk or posedge i_video_reset) begin
        if (i_video_reset) begin 
            v_count <= 0;
        end
        else begin
            if (i_frame_start_reg == 0 && i_frame_start == 1)
                v_count <= 0;
            else if (data_valid_fe) begin 
                v_count <= v_count + 1;
            end
        end
    end

    integer i;
    always @ (posedge i_video_clk or posedge i_video_reset) begin
        if (i_video_reset) begin
            frame_start_delay <= 0;
            data_valid_delay[0] <= 0;
            data_valid_delay[1] <= 0;
            data_valid_fe <= 0;
            data_valid_re <= 0;
            frame_start_re <= 0;
            data_i_delay[0] <= 0;
            data_i_delay[1] <= 0;
            SOF_delay <= 0;
            i_frame_start_reg <= 0;
        end
        else begin
            SOF_delay <= SOF;
            i_frame_start_reg <= i_frame_start;
            for(i=0; i < PIXEL_PER_CLK; i = i + 1)
                data_i_delay[0][C_WIDTH * i +: C_WIDTH ] <= i_Data[C_WIDTH * i +: C_WIDTH];
            data_i_delay[1] <= data_i_delay[0]; 
            data_valid_delay[0] <= i_data_valid;
            data_valid_delay[1] <= data_valid_delay[0];
            frame_start_delay <= i_frame_start;
            data_valid_fe <= (data_valid_delay[0] & !i_data_valid) ? 1 : 0;
            data_valid_re <= (!data_valid_delay[0] & i_data_valid) ? 1 : 0;
            frame_start_re <= (!frame_start_delay & i_frame_start) ? 1 : 0;
        end
    end

    //skip frame if backpressure detected at SOF
    always @ (posedge i_axis_clk) begin
        if (i_axis_reset) begin
            skip_frame <= 0; 
            o_frames_skipped <= 0;
            o_pushback_detected <= 0;
        end
        else if (fifo_o_valid && (o_tuser[0] == 1) && !i_tready && !skip_frame) begin 
            skip_frame <= 1;
            o_frames_skipped <= o_frames_skipped + 1;
            o_pushback_detected <= 1;
        end
        else if(flush_v_count == i_vres-1 && flush_h_count == i_hres-1)begin
            skip_frame <= 0;

        end
    end


    always @ (posedge i_axis_clk) begin
        if (i_axis_reset) begin
            flush_v_count <= 0;
            flush_h_count <= 0;
        end
        else if (!skip_frame) begin 
            flush_v_count <= 0;
            flush_h_count <= 0;
        end
        else if(fifo_o_valid)begin
            if (flush_h_count < i_hres-1)
                flush_h_count <= flush_h_count+PIXEL_PER_CLK;
            else begin
                flush_h_count <= 0;
                if (flush_v_count < i_vres-1)
                    flush_v_count <= flush_v_count+1;
                else
                    flush_v_count <= 0;
            end
            
        end
    end

    assign axis_tvalid = (LINES_TO_SKIP > 0) ? (data_valid_delay[1] && (v_count > LINES_TO_SKIP - 1)) : data_valid_delay[1];
    axis_async_fifo #(
        .DEPTH          (FIFO_DEPTH),
        .DATA_WIDTH     (PIXEL_PER_CLK * C_WIDTH),
        .KEEP_ENABLE    (0),
        .LAST_ENABLE    (1),
        .USER_ENABLE    (1),
        .USER_WIDTH     (1),
        .FRAME_FIFO     (DEBUBBLE)
    ) FIFO_A(
        .s_clk          (i_video_clk),
        .s_rst          (i_video_reset),
        .s_axis_tdata   (data_i_delay[1]),
        .s_axis_tvalid  (axis_tvalid),
        .s_axis_tlast   (EOL),
        .s_axis_tuser   (SOF_delay),
        .m_clk          (i_axis_clk),
        .m_rst          (i_axis_reset),
        .m_axis_tlast   (o_tlast),
        .m_axis_tuser   (o_tuser),
        .m_axis_tdata   (o_tdata),
        .m_axis_tvalid  (fifo_o_valid),
        .m_axis_tready  (i_tready | skip_frame)
    );
endmodule





/*

Copyright (c) 2014-2021 Alex Forencich

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

// Language: Verilog 2001

// `resetall
// `timescale 1ns / 1ps
// `default_nettype none

/*
 * AXI4-Stream asynchronous FIFO
 */
module axis_async_fifo #
(
    // FIFO depth in words
    // KEEP_WIDTH words per cycle if KEEP_ENABLE set
    // Rounded up to nearest power of 2 cycles
    parameter DEPTH = 4096,
    // Width of AXI stream interfaces in bits
    parameter DATA_WIDTH = 8,
    // Propagate tkeep signal
    // If disabled, tkeep assumed to be 1'b1
    parameter KEEP_ENABLE = (DATA_WIDTH>8),
    // tkeep signal width (words per cycle)
    parameter KEEP_WIDTH = ((DATA_WIDTH+7)/8),
    // Propagate tlast signal
    parameter LAST_ENABLE = 1,
    // Propagate tid signal
    parameter ID_ENABLE = 0,
    // tid signal width
    parameter ID_WIDTH = 8,
    // Propagate tdest signal
    parameter DEST_ENABLE = 0,
    // tdest signal width
    parameter DEST_WIDTH = 8,
    // Propagate tuser signal
    parameter USER_ENABLE = 1,
    // tuser signal width
    parameter USER_WIDTH = 1,
    // number of RAM pipeline registers
    parameter RAM_PIPELINE = 1,
    // use output FIFO
    // When set, the RAM read enable and pipeline clock enables are removed
    parameter OUTPUT_FIFO_ENABLE = 0,
    // Frame FIFO mode - operate on frames instead of cycles
    // When set, m_axis_tvalid will not be deasserted within a frame
    // Requires LAST_ENABLE set
    parameter FRAME_FIFO = 0,
    // tuser value for bad frame marker
    parameter USER_BAD_FRAME_VALUE = 1'b1,
    // tuser mask for bad frame marker
    parameter USER_BAD_FRAME_MASK = 1'b1,
    // Drop frames larger than FIFO
    // Requires FRAME_FIFO set
    parameter DROP_OVERSIZE_FRAME = FRAME_FIFO,
    // Drop frames marked bad
    // Requires FRAME_FIFO and DROP_OVERSIZE_FRAME set
    parameter DROP_BAD_FRAME = 0,
    // Drop incoming frames when full
    // When set, s_axis_tready is always asserted
    // Requires FRAME_FIFO and DROP_OVERSIZE_FRAME set
    parameter DROP_WHEN_FULL = 0
)
(
    /*
     * AXI input
     */
    input  wire                   s_clk,
    input  wire                   s_rst,
    input  wire [DATA_WIDTH-1:0]  s_axis_tdata,
    input  wire [KEEP_WIDTH-1:0]  s_axis_tkeep,
    input  wire                   s_axis_tvalid,
    output wire                   s_axis_tready,
    input  wire                   s_axis_tlast,
    input  wire [ID_WIDTH-1:0]    s_axis_tid,
    input  wire [DEST_WIDTH-1:0]  s_axis_tdest,
    input  wire [USER_WIDTH-1:0]  s_axis_tuser,

    /*
     * AXI output
     */
    input  wire                   m_clk,
    input  wire                   m_rst,
    output wire [DATA_WIDTH-1:0]  m_axis_tdata,
    output wire [KEEP_WIDTH-1:0]  m_axis_tkeep,
    output wire                   m_axis_tvalid,
    input  wire                   m_axis_tready,
    output wire                   m_axis_tlast,
    output wire [ID_WIDTH-1:0]    m_axis_tid,
    output wire [DEST_WIDTH-1:0]  m_axis_tdest,
    output wire [USER_WIDTH-1:0]  m_axis_tuser,

    /*
     * Status
     */
    output wire                   s_status_overflow,
    output wire                   s_status_bad_frame,
    output wire                   s_status_good_frame,
    output wire                   m_status_overflow,
    output wire                   m_status_bad_frame,
    output wire                   m_status_good_frame
);

parameter ADDR_WIDTH = (KEEP_ENABLE && KEEP_WIDTH > 1) ? $clog2(DEPTH/KEEP_WIDTH) : $clog2(DEPTH);

parameter OUTPUT_FIFO_ADDR_WIDTH = RAM_PIPELINE < 2 ? 3 : $clog2(RAM_PIPELINE*2+7);

// check configuration
initial begin
    if (FRAME_FIFO && !LAST_ENABLE) begin
        $error("Error: FRAME_FIFO set requires LAST_ENABLE set (instance %m)");
        $finish;
    end

    if (DROP_OVERSIZE_FRAME && !FRAME_FIFO) begin
        $error("Error: DROP_OVERSIZE_FRAME set requires FRAME_FIFO set (instance %m)");
        $finish;
    end

    if (DROP_BAD_FRAME && !(FRAME_FIFO && DROP_OVERSIZE_FRAME)) begin
        $error("Error: DROP_BAD_FRAME set requires FRAME_FIFO and DROP_OVERSIZE_FRAME set (instance %m)");
        $finish;
    end

    if (DROP_WHEN_FULL && !(FRAME_FIFO && DROP_OVERSIZE_FRAME)) begin
        $error("Error: DROP_WHEN_FULL set requires FRAME_FIFO and DROP_OVERSIZE_FRAME set (instance %m)");
        $finish;
    end

    if (DROP_BAD_FRAME && (USER_BAD_FRAME_MASK & {USER_WIDTH{1'b1}}) == 0) begin
        $error("Error: Invalid USER_BAD_FRAME_MASK value (instance %m)");
        $finish;
    end
end

localparam KEEP_OFFSET = DATA_WIDTH;
localparam LAST_OFFSET = KEEP_OFFSET + (KEEP_ENABLE ? KEEP_WIDTH : 0);
localparam ID_OFFSET   = LAST_OFFSET + (LAST_ENABLE ? 1          : 0);
localparam DEST_OFFSET = ID_OFFSET   + (ID_ENABLE   ? ID_WIDTH   : 0);
localparam USER_OFFSET = DEST_OFFSET + (DEST_ENABLE ? DEST_WIDTH : 0);
localparam WIDTH       = USER_OFFSET + (USER_ENABLE ? USER_WIDTH : 0);

reg [ADDR_WIDTH:0] wr_ptr_reg = {ADDR_WIDTH+1{1'b0}};
reg [ADDR_WIDTH:0] wr_ptr_cur_reg = {ADDR_WIDTH+1{1'b0}};
reg [ADDR_WIDTH:0] wr_ptr_gray_reg = {ADDR_WIDTH+1{1'b0}};
reg [ADDR_WIDTH:0] wr_ptr_sync_gray_reg = {ADDR_WIDTH+1{1'b0}};
reg [ADDR_WIDTH:0] wr_ptr_cur_gray_reg = {ADDR_WIDTH+1{1'b0}};
reg [ADDR_WIDTH:0] rd_ptr_reg = {ADDR_WIDTH+1{1'b0}};
reg [ADDR_WIDTH:0] rd_ptr_gray_reg = {ADDR_WIDTH+1{1'b0}};

reg [ADDR_WIDTH:0] wr_ptr_temp;
reg [ADDR_WIDTH:0] rd_ptr_temp;

(* SHREG_EXTRACT = "NO" *)
reg [ADDR_WIDTH:0] wr_ptr_gray_sync1_reg = {ADDR_WIDTH+1{1'b0}};
(* SHREG_EXTRACT = "NO" *)
reg [ADDR_WIDTH:0] wr_ptr_gray_sync2_reg = {ADDR_WIDTH+1{1'b0}};
(* SHREG_EXTRACT = "NO" *)
reg [ADDR_WIDTH:0] rd_ptr_gray_sync1_reg = {ADDR_WIDTH+1{1'b0}};
(* SHREG_EXTRACT = "NO" *)
reg [ADDR_WIDTH:0] rd_ptr_gray_sync2_reg = {ADDR_WIDTH+1{1'b0}};

reg wr_ptr_update_valid_reg = 1'b0;
reg wr_ptr_update_reg = 1'b0;
(* SHREG_EXTRACT = "NO" *)
reg wr_ptr_update_sync1_reg = 1'b0;
(* SHREG_EXTRACT = "NO" *)
reg wr_ptr_update_sync2_reg = 1'b0;
(* SHREG_EXTRACT = "NO" *)
reg wr_ptr_update_sync3_reg = 1'b0;
(* SHREG_EXTRACT = "NO" *)
reg wr_ptr_update_ack_sync1_reg = 1'b0;
(* SHREG_EXTRACT = "NO" *)
reg wr_ptr_update_ack_sync2_reg = 1'b0;

(* SHREG_EXTRACT = "NO" *)
reg s_rst_sync1_reg = 1'b1;
(* SHREG_EXTRACT = "NO" *)
reg s_rst_sync2_reg = 1'b1;
(* SHREG_EXTRACT = "NO" *)
reg s_rst_sync3_reg = 1'b1;
(* SHREG_EXTRACT = "NO" *)
reg m_rst_sync1_reg = 1'b1;
(* SHREG_EXTRACT = "NO" *)
reg m_rst_sync2_reg = 1'b1;
(* SHREG_EXTRACT = "NO" *)
reg m_rst_sync3_reg = 1'b1;

(* ramstyle = "no_rw_check" *)
reg [WIDTH-1:0] mem[(2**ADDR_WIDTH)-1:0];
reg [WIDTH-1:0] mem_read_data_reg;
reg mem_read_data_valid_reg = 1'b0;

(* shreg_extract = "no" *)
reg [WIDTH-1:0] m_axis_pipe_reg[RAM_PIPELINE+1-1:0];
reg [RAM_PIPELINE+1-1:0] m_axis_tvalid_pipe_reg = 0;

// full when first TWO MSBs do NOT match, but rest matches
// (gray code equivalent of first MSB different but rest same)
wire full = wr_ptr_gray_reg == (rd_ptr_gray_sync2_reg ^ {2'b11, {ADDR_WIDTH-1{1'b0}}});
wire full_cur = wr_ptr_cur_gray_reg == (rd_ptr_gray_sync2_reg ^ {2'b11, {ADDR_WIDTH-1{1'b0}}});
// empty when pointers match exactly
wire empty = rd_ptr_gray_reg == (FRAME_FIFO ? wr_ptr_gray_sync1_reg : wr_ptr_gray_sync2_reg);
// overflow within packet
wire full_wr = wr_ptr_reg == (wr_ptr_cur_reg ^ {1'b1, {ADDR_WIDTH{1'b0}}});

// control signals
reg write;
reg read;
reg store_output;

reg s_frame_reg = 1'b0;
reg m_frame_reg = 1'b0;

reg drop_frame_reg = 1'b0;
reg send_frame_reg = 1'b0;
reg overflow_reg = 1'b0;
reg bad_frame_reg = 1'b0;
reg good_frame_reg = 1'b0;

reg m_drop_frame_reg = 1'b0;
reg m_terminate_frame_reg = 1'b0;

reg overflow_sync1_reg = 1'b0;
reg overflow_sync2_reg = 1'b0;
reg overflow_sync3_reg = 1'b0;
reg overflow_sync4_reg = 1'b0;
reg bad_frame_sync1_reg = 1'b0;
reg bad_frame_sync2_reg = 1'b0;
reg bad_frame_sync3_reg = 1'b0;
reg bad_frame_sync4_reg = 1'b0;
reg good_frame_sync1_reg = 1'b0;
reg good_frame_sync2_reg = 1'b0;
reg good_frame_sync3_reg = 1'b0;
reg good_frame_sync4_reg = 1'b0;

assign s_axis_tready = (FRAME_FIFO ? (!full_cur || (full_wr && DROP_OVERSIZE_FRAME) || DROP_WHEN_FULL) : !full) && !s_rst_sync3_reg;

wire [WIDTH-1:0] s_axis;

generate
    assign s_axis[DATA_WIDTH-1:0] = s_axis_tdata;
    if (KEEP_ENABLE) assign s_axis[KEEP_OFFSET +: KEEP_WIDTH] = s_axis_tkeep;
    if (LAST_ENABLE) assign s_axis[LAST_OFFSET]               = s_axis_tlast;
    if (ID_ENABLE)   assign s_axis[ID_OFFSET   +: ID_WIDTH]   = s_axis_tid;
    if (DEST_ENABLE) assign s_axis[DEST_OFFSET +: DEST_WIDTH] = s_axis_tdest;
    if (USER_ENABLE) assign s_axis[USER_OFFSET +: USER_WIDTH] = s_axis_tuser;
endgenerate

wire [WIDTH-1:0] m_axis = RAM_PIPELINE ? m_axis_pipe_reg[RAM_PIPELINE+1-1] : mem_read_data_reg;

wire                   m_axis_tvalid_pipe = m_axis_tvalid_pipe_reg[RAM_PIPELINE+1-1];

wire [DATA_WIDTH-1:0]  m_axis_tdata_pipe  = m_axis[DATA_WIDTH-1:0];
wire [KEEP_WIDTH-1:0]  m_axis_tkeep_pipe  = KEEP_ENABLE ? m_axis[KEEP_OFFSET +: KEEP_WIDTH] : {KEEP_WIDTH{1'b1}};
wire                   m_axis_tlast_pipe  = LAST_ENABLE ? m_axis[LAST_OFFSET] | m_terminate_frame_reg : 1'b1;
wire [ID_WIDTH-1:0]    m_axis_tid_pipe    = ID_ENABLE   ? m_axis[ID_OFFSET +: ID_WIDTH] : {ID_WIDTH{1'b0}};
wire [DEST_WIDTH-1:0]  m_axis_tdest_pipe  = DEST_ENABLE ? m_axis[DEST_OFFSET +: DEST_WIDTH] : {DEST_WIDTH{1'b0}};
wire [USER_WIDTH-1:0]  m_axis_tuser_pipe  = USER_ENABLE ? (m_terminate_frame_reg ? USER_BAD_FRAME_VALUE : m_axis[USER_OFFSET +: USER_WIDTH]) : {USER_WIDTH{1'b0}};

wire pipe_ready;

assign s_status_overflow = overflow_reg;
assign s_status_bad_frame = bad_frame_reg;
assign s_status_good_frame = good_frame_reg;

assign m_status_overflow = overflow_sync3_reg ^ overflow_sync4_reg;
assign m_status_bad_frame = bad_frame_sync3_reg ^ bad_frame_sync4_reg;
assign m_status_good_frame = good_frame_sync3_reg ^ good_frame_sync4_reg;

// reset synchronization
always @(posedge m_clk or posedge m_rst) begin
    if (m_rst) begin
        s_rst_sync1_reg <= 1'b1;
    end else begin
        s_rst_sync1_reg <= 1'b0;
    end
end

always @(posedge s_clk) begin
    s_rst_sync2_reg <= s_rst_sync1_reg;
    s_rst_sync3_reg <= s_rst_sync2_reg;
end

always @(posedge s_clk or posedge s_rst) begin
    if (s_rst) begin
        m_rst_sync1_reg <= 1'b1;
    end else begin
        m_rst_sync1_reg <= 1'b0;
    end
end

always @(posedge m_clk) begin
    m_rst_sync2_reg <= m_rst_sync1_reg;
    m_rst_sync3_reg <= m_rst_sync2_reg;
end

// Write logic
always @(posedge s_clk) begin
    overflow_reg <= 1'b0;
    bad_frame_reg <= 1'b0;
    good_frame_reg <= 1'b0;

    if (FRAME_FIFO && wr_ptr_update_valid_reg) begin
        // have updated pointer to sync
        if (wr_ptr_update_reg == wr_ptr_update_ack_sync2_reg) begin
            // no sync in progress; sync update
            wr_ptr_update_valid_reg <= 1'b0;
            wr_ptr_sync_gray_reg <= wr_ptr_gray_reg;
            wr_ptr_update_reg <= !wr_ptr_update_ack_sync2_reg;
        end
    end

    if (s_axis_tready && s_axis_tvalid && LAST_ENABLE) begin
        // track input frame status
        s_frame_reg <= !s_axis_tlast;
    end

    if (s_rst_sync3_reg && LAST_ENABLE) begin
        // if sink side is reset during transfer, drop partial frame
        if (s_frame_reg && !(s_axis_tready && s_axis_tvalid && s_axis_tlast)) begin
            drop_frame_reg <= 1'b1;
        end
        if (s_axis_tready && s_axis_tvalid && !s_axis_tlast) begin
            drop_frame_reg <= 1'b1;
        end
    end

    if (s_axis_tready && s_axis_tvalid) begin
        // transfer in
        if (!FRAME_FIFO) begin
            // normal FIFO mode
            mem[wr_ptr_reg[ADDR_WIDTH-1:0]] <= s_axis;
            if (drop_frame_reg && LAST_ENABLE) begin
                // currently dropping frame
                // (only for frame transfers interrupted by sink reset)
                if (s_axis_tlast) begin
                    // end of frame, clear drop flag
                    drop_frame_reg <= 1'b0;
                end
            end else begin
                // update pointers
                wr_ptr_temp = wr_ptr_reg + 1;
                wr_ptr_reg <= wr_ptr_temp;
                wr_ptr_gray_reg <= wr_ptr_temp ^ (wr_ptr_temp >> 1);
            end
        end else if ((full_cur && DROP_WHEN_FULL) || (full_wr && DROP_OVERSIZE_FRAME) || drop_frame_reg) begin
            // full, packet overflow, or currently dropping frame
            // drop frame
            drop_frame_reg <= 1'b1;
            if (s_axis_tlast) begin
                // end of frame, reset write pointer
                wr_ptr_temp = wr_ptr_reg;
                wr_ptr_cur_reg <= wr_ptr_temp;
                wr_ptr_cur_gray_reg <= wr_ptr_temp ^ (wr_ptr_temp >> 1);
                drop_frame_reg <= 1'b0;
                overflow_reg <= 1'b1;
            end
        end else begin
            mem[wr_ptr_cur_reg[ADDR_WIDTH-1:0]] <= s_axis;
            wr_ptr_temp = wr_ptr_cur_reg + 1;
            wr_ptr_cur_reg <= wr_ptr_temp;
            wr_ptr_cur_gray_reg <= wr_ptr_temp ^ (wr_ptr_temp >> 1);
            if (s_axis_tlast || (!DROP_OVERSIZE_FRAME && (full_wr || send_frame_reg))) begin
                // end of frame or send frame
                send_frame_reg <= !s_axis_tlast;
                if (s_axis_tlast && DROP_BAD_FRAME && USER_BAD_FRAME_MASK & ~(s_axis_tuser ^ USER_BAD_FRAME_VALUE)) begin
                    // bad packet, reset write pointer
                    wr_ptr_temp = wr_ptr_reg;
                    wr_ptr_cur_reg <= wr_ptr_temp;
                    wr_ptr_cur_gray_reg <= wr_ptr_temp ^ (wr_ptr_temp >> 1);
                    bad_frame_reg <= 1'b1;
                end else begin
                    // good packet or packet overflow, update write pointer
                    wr_ptr_temp = wr_ptr_cur_reg + 1;
                    wr_ptr_reg <= wr_ptr_temp;
                    wr_ptr_gray_reg <= wr_ptr_temp ^ (wr_ptr_temp >> 1);

                    if (wr_ptr_update_reg == wr_ptr_update_ack_sync2_reg) begin
                        // no sync in progress; sync update
                        wr_ptr_update_valid_reg <= 1'b0;
                        wr_ptr_sync_gray_reg <= wr_ptr_temp ^ (wr_ptr_temp >> 1);
                        wr_ptr_update_reg <= !wr_ptr_update_ack_sync2_reg;
                    end else begin
                        // sync in progress; flag it for later
                        wr_ptr_update_valid_reg <= 1'b1;
                    end

                    good_frame_reg <= s_axis_tlast;
                end
            end
        end
    end else if (s_axis_tvalid && full_wr && FRAME_FIFO && !DROP_OVERSIZE_FRAME) begin
        // data valid with packet overflow
        // update write pointer
        send_frame_reg <= 1'b1;
        wr_ptr_temp = wr_ptr_cur_reg;
        wr_ptr_reg <= wr_ptr_temp;
        wr_ptr_gray_reg <= wr_ptr_temp ^ (wr_ptr_temp >> 1);

        if (wr_ptr_update_reg == wr_ptr_update_ack_sync2_reg) begin
            // no sync in progress; sync update
            wr_ptr_update_valid_reg <= 1'b0;
            wr_ptr_sync_gray_reg <= wr_ptr_temp ^ (wr_ptr_temp >> 1);
            wr_ptr_update_reg <= !wr_ptr_update_ack_sync2_reg;
        end else begin
            // sync in progress; flag it for later
            wr_ptr_update_valid_reg <= 1'b1;
        end
    end

    if (s_rst_sync3_reg) begin
        wr_ptr_reg <= {ADDR_WIDTH+1{1'b0}};
        wr_ptr_cur_reg <= {ADDR_WIDTH+1{1'b0}};
        wr_ptr_gray_reg <= {ADDR_WIDTH+1{1'b0}};
        wr_ptr_sync_gray_reg <= {ADDR_WIDTH+1{1'b0}};
        wr_ptr_cur_gray_reg <= {ADDR_WIDTH+1{1'b0}};

        wr_ptr_update_valid_reg <= 1'b0;
        wr_ptr_update_reg <= 1'b0;
    end

    if (s_rst) begin
        wr_ptr_reg <= {ADDR_WIDTH+1{1'b0}};
        wr_ptr_cur_reg <= {ADDR_WIDTH+1{1'b0}};
        wr_ptr_gray_reg <= {ADDR_WIDTH+1{1'b0}};
        wr_ptr_sync_gray_reg <= {ADDR_WIDTH+1{1'b0}};
        wr_ptr_cur_gray_reg <= {ADDR_WIDTH+1{1'b0}};

        wr_ptr_update_valid_reg <= 1'b0;
        wr_ptr_update_reg <= 1'b0;

        s_frame_reg <= 1'b0;

        drop_frame_reg <= 1'b0;
        send_frame_reg <= 1'b0;
        overflow_reg <= 1'b0;
        bad_frame_reg <= 1'b0;
        good_frame_reg <= 1'b0;
    end
end

// pointer synchronization
always @(posedge s_clk) begin
    rd_ptr_gray_sync1_reg <= rd_ptr_gray_reg;
    rd_ptr_gray_sync2_reg <= rd_ptr_gray_sync1_reg;
    wr_ptr_update_ack_sync1_reg <= wr_ptr_update_sync3_reg;
    wr_ptr_update_ack_sync2_reg <= wr_ptr_update_ack_sync1_reg;

    if (s_rst) begin
        rd_ptr_gray_sync1_reg <= {ADDR_WIDTH+1{1'b0}};
        rd_ptr_gray_sync2_reg <= {ADDR_WIDTH+1{1'b0}};
        wr_ptr_update_ack_sync1_reg <= 1'b0;
        wr_ptr_update_ack_sync2_reg <= 1'b0;
    end
end

always @(posedge m_clk) begin
    if (!FRAME_FIFO) begin
        wr_ptr_gray_sync1_reg <= wr_ptr_gray_reg;
    end else if (wr_ptr_update_sync2_reg ^ wr_ptr_update_sync3_reg) begin
        wr_ptr_gray_sync1_reg <= wr_ptr_sync_gray_reg;
    end
    wr_ptr_gray_sync2_reg <= wr_ptr_gray_sync1_reg;
    wr_ptr_update_sync1_reg <= wr_ptr_update_reg;
    wr_ptr_update_sync2_reg <= wr_ptr_update_sync1_reg;
    wr_ptr_update_sync3_reg <= wr_ptr_update_sync2_reg;

    if (FRAME_FIFO && m_rst_sync3_reg) begin
        wr_ptr_gray_sync1_reg <= {ADDR_WIDTH+1{1'b0}};
    end

    if (m_rst) begin
        wr_ptr_gray_sync1_reg <= {ADDR_WIDTH+1{1'b0}};
        wr_ptr_gray_sync2_reg <= {ADDR_WIDTH+1{1'b0}};
        wr_ptr_update_sync1_reg <= 1'b0;
        wr_ptr_update_sync2_reg <= 1'b0;
        wr_ptr_update_sync3_reg <= 1'b0;
    end
end

// status synchronization
always @(posedge s_clk) begin
    overflow_sync1_reg <= overflow_sync1_reg ^ overflow_reg;
    bad_frame_sync1_reg <= bad_frame_sync1_reg ^ bad_frame_reg;
    good_frame_sync1_reg <= good_frame_sync1_reg ^ good_frame_reg;

    if (s_rst) begin
        overflow_sync1_reg <= 1'b0;
        bad_frame_sync1_reg <= 1'b0;
        good_frame_sync1_reg <= 1'b0;
    end
end

always @(posedge m_clk) begin
    overflow_sync2_reg <= overflow_sync1_reg;
    overflow_sync3_reg <= overflow_sync2_reg;
    overflow_sync4_reg <= overflow_sync3_reg;
    bad_frame_sync2_reg <= bad_frame_sync1_reg;
    bad_frame_sync3_reg <= bad_frame_sync2_reg;
    bad_frame_sync4_reg <= bad_frame_sync3_reg;
    good_frame_sync2_reg <= good_frame_sync1_reg;
    good_frame_sync3_reg <= good_frame_sync2_reg;
    good_frame_sync4_reg <= good_frame_sync3_reg;

    if (m_rst) begin
        overflow_sync2_reg <= 1'b0;
        overflow_sync3_reg <= 1'b0;
        overflow_sync4_reg <= 1'b0;
        bad_frame_sync2_reg <= 1'b0;
        bad_frame_sync3_reg <= 1'b0;
        bad_frame_sync4_reg <= 1'b0;
        good_frame_sync2_reg <= 1'b0;
        good_frame_sync3_reg <= 1'b0;
        good_frame_sync4_reg <= 1'b0;
    end
end

// Read logic
integer j;

always @(posedge m_clk) begin
    if (OUTPUT_FIFO_ENABLE || m_axis_tready) begin
        // output ready; invalidate stage
        m_axis_tvalid_pipe_reg[RAM_PIPELINE+1-1] <= 1'b0;
        m_terminate_frame_reg <= 1'b0;
    end

    for (j = RAM_PIPELINE+1-1; j > 0; j = j - 1) begin
        if (OUTPUT_FIFO_ENABLE || m_axis_tready || ((~m_axis_tvalid_pipe_reg) >> j)) begin
            // output ready or bubble in pipeline; transfer down pipeline
            m_axis_tvalid_pipe_reg[j] <= m_axis_tvalid_pipe_reg[j-1];
            m_axis_pipe_reg[j] <= j == 1 ? mem_read_data_reg : m_axis_pipe_reg[j-1];
            m_axis_tvalid_pipe_reg[j-1] <= 1'b0;
        end
    end

    if (OUTPUT_FIFO_ENABLE || m_axis_tready || ~m_axis_tvalid_pipe_reg) begin
        // output ready or bubble in pipeline; read new data from FIFO
        m_axis_tvalid_pipe_reg[0] <= 1'b0;
        mem_read_data_reg <= mem[rd_ptr_reg[ADDR_WIDTH-1:0]];
        if (!empty && !m_rst_sync3_reg && !m_drop_frame_reg && pipe_ready) begin
            // not empty, increment pointer
            m_axis_tvalid_pipe_reg[0] <= 1'b1;
            rd_ptr_temp = rd_ptr_reg + 1;
            rd_ptr_reg <= rd_ptr_temp;
            rd_ptr_gray_reg <= rd_ptr_temp ^ (rd_ptr_temp >> 1);
        end
    end

    if (m_axis_tvalid_pipe && LAST_ENABLE) begin
        // track output frame status
        if (m_axis_tlast_pipe && (OUTPUT_FIFO_ENABLE || m_axis_tready)) begin
            m_frame_reg <= 1'b0;
        end else begin
            m_frame_reg <= 1'b1;
        end
    end

    if (m_drop_frame_reg && (OUTPUT_FIFO_ENABLE ? pipe_ready : m_axis_tready || !m_axis_tvalid_pipe) && LAST_ENABLE) begin
        // terminate frame
        // (only for frame transfers interrupted by source reset)
        m_axis_tvalid_pipe_reg[RAM_PIPELINE+1-1] <= 1'b1;
        m_terminate_frame_reg <= 1'b1;
        m_drop_frame_reg <= 1'b0;
    end

    if (m_rst_sync3_reg && LAST_ENABLE) begin
        // if source side is reset during transfer, drop partial frame

        // empty output pipeline, except for last stage
        if (RAM_PIPELINE > 0) begin
            m_axis_tvalid_pipe_reg[RAM_PIPELINE+1-2:0] <= 0;
        end

        if (m_frame_reg && (!m_axis_tvalid_pipe || (m_axis_tvalid_pipe && !m_axis_tlast_pipe)) &&
                !(m_drop_frame_reg || m_terminate_frame_reg)) begin
            // terminate frame
            m_drop_frame_reg <= 1'b1;
        end
    end

    if (m_rst_sync3_reg) begin
        rd_ptr_reg <= {ADDR_WIDTH+1{1'b0}};
        rd_ptr_gray_reg <= {ADDR_WIDTH+1{1'b0}};
    end

    if (m_rst) begin
        rd_ptr_reg <= {ADDR_WIDTH+1{1'b0}};
        rd_ptr_gray_reg <= {ADDR_WIDTH+1{1'b0}};
        m_axis_tvalid_pipe_reg <= 0;
        m_frame_reg <= 1'b0;
        m_drop_frame_reg <= 1'b0;
        m_terminate_frame_reg <= 1'b0;
    end
end

generate

if (!OUTPUT_FIFO_ENABLE) begin

    assign pipe_ready = 1'b1;

    assign m_axis_tvalid = m_axis_tvalid_pipe;

    assign m_axis_tdata = m_axis_tdata_pipe;
    assign m_axis_tkeep = m_axis_tkeep_pipe;
    assign m_axis_tlast = m_axis_tlast_pipe;
    assign m_axis_tid   = m_axis_tid_pipe;
    assign m_axis_tdest = m_axis_tdest_pipe;
    assign m_axis_tuser = m_axis_tuser_pipe;

end else begin

    // output datapath logic
    reg [DATA_WIDTH-1:0] m_axis_tdata_reg  = {DATA_WIDTH{1'b0}};
    reg [KEEP_WIDTH-1:0] m_axis_tkeep_reg  = {KEEP_WIDTH{1'b0}};
    reg                  m_axis_tvalid_reg = 1'b0;
    reg                  m_axis_tlast_reg  = 1'b0;
    reg [ID_WIDTH-1:0]   m_axis_tid_reg    = {ID_WIDTH{1'b0}};
    reg [DEST_WIDTH-1:0] m_axis_tdest_reg  = {DEST_WIDTH{1'b0}};
    reg [USER_WIDTH-1:0] m_axis_tuser_reg  = {USER_WIDTH{1'b0}};

    reg [OUTPUT_FIFO_ADDR_WIDTH+1-1:0] out_fifo_wr_ptr_reg = 0;
    reg [OUTPUT_FIFO_ADDR_WIDTH+1-1:0] out_fifo_rd_ptr_reg = 0;
    reg out_fifo_half_full_reg = 1'b0;

    wire out_fifo_full = out_fifo_wr_ptr_reg == (out_fifo_rd_ptr_reg ^ {1'b1, {OUTPUT_FIFO_ADDR_WIDTH{1'b0}}});
    wire out_fifo_empty = out_fifo_wr_ptr_reg == out_fifo_rd_ptr_reg;

    (* ram_style = "distributed", ramstyle = "no_rw_check, mlab" *)
    reg [DATA_WIDTH-1:0] out_fifo_tdata[2**OUTPUT_FIFO_ADDR_WIDTH-1:0];
    (* ram_style = "distributed", ramstyle = "no_rw_check, mlab" *)
    reg [KEEP_WIDTH-1:0] out_fifo_tkeep[2**OUTPUT_FIFO_ADDR_WIDTH-1:0];
    (* ram_style = "distributed", ramstyle = "no_rw_check, mlab" *)
    reg                  out_fifo_tlast[2**OUTPUT_FIFO_ADDR_WIDTH-1:0];
    (* ram_style = "distributed", ramstyle = "no_rw_check, mlab" *)
    reg [ID_WIDTH-1:0]   out_fifo_tid[2**OUTPUT_FIFO_ADDR_WIDTH-1:0];
    (* ram_style = "distributed", ramstyle = "no_rw_check, mlab" *)
    reg [DEST_WIDTH-1:0] out_fifo_tdest[2**OUTPUT_FIFO_ADDR_WIDTH-1:0];
    (* ram_style = "distributed", ramstyle = "no_rw_check, mlab" *)
    reg [USER_WIDTH-1:0] out_fifo_tuser[2**OUTPUT_FIFO_ADDR_WIDTH-1:0];

    assign pipe_ready = !out_fifo_half_full_reg;

    assign m_axis_tdata  = m_axis_tdata_reg;
    assign m_axis_tkeep  = KEEP_ENABLE ? m_axis_tkeep_reg : {KEEP_WIDTH{1'b1}};
    assign m_axis_tvalid = m_axis_tvalid_reg;
    assign m_axis_tlast  = LAST_ENABLE ? m_axis_tlast_reg : 1'b1;
    assign m_axis_tid    = ID_ENABLE   ? m_axis_tid_reg   : {ID_WIDTH{1'b0}};
    assign m_axis_tdest  = DEST_ENABLE ? m_axis_tdest_reg : {DEST_WIDTH{1'b0}};
    assign m_axis_tuser  = USER_ENABLE ? m_axis_tuser_reg : {USER_WIDTH{1'b0}};

    always @(posedge m_clk) begin
        m_axis_tvalid_reg <= m_axis_tvalid_reg && !m_axis_tready;

        out_fifo_half_full_reg <= $unsigned(out_fifo_wr_ptr_reg - out_fifo_rd_ptr_reg) >= 2**(OUTPUT_FIFO_ADDR_WIDTH-1);

        if (!out_fifo_full && m_axis_tvalid_pipe) begin
            out_fifo_tdata[out_fifo_wr_ptr_reg[OUTPUT_FIFO_ADDR_WIDTH-1:0]] <= m_axis_tdata_pipe;
            out_fifo_tkeep[out_fifo_wr_ptr_reg[OUTPUT_FIFO_ADDR_WIDTH-1:0]] <= m_axis_tkeep_pipe;
            out_fifo_tlast[out_fifo_wr_ptr_reg[OUTPUT_FIFO_ADDR_WIDTH-1:0]] <= m_axis_tlast_pipe;
            out_fifo_tid[out_fifo_wr_ptr_reg[OUTPUT_FIFO_ADDR_WIDTH-1:0]] <= m_axis_tid_pipe;
            out_fifo_tdest[out_fifo_wr_ptr_reg[OUTPUT_FIFO_ADDR_WIDTH-1:0]] <= m_axis_tdest_pipe;
            out_fifo_tuser[out_fifo_wr_ptr_reg[OUTPUT_FIFO_ADDR_WIDTH-1:0]] <= m_axis_tuser_pipe;
            out_fifo_wr_ptr_reg <= out_fifo_wr_ptr_reg + 1;
        end

        if (!out_fifo_empty && (!m_axis_tvalid_reg || m_axis_tready)) begin
            m_axis_tdata_reg <= out_fifo_tdata[out_fifo_rd_ptr_reg[OUTPUT_FIFO_ADDR_WIDTH-1:0]];
            m_axis_tkeep_reg <= out_fifo_tkeep[out_fifo_rd_ptr_reg[OUTPUT_FIFO_ADDR_WIDTH-1:0]];
            m_axis_tvalid_reg <= 1'b1;
            m_axis_tlast_reg <= out_fifo_tlast[out_fifo_rd_ptr_reg[OUTPUT_FIFO_ADDR_WIDTH-1:0]];
            m_axis_tid_reg <= out_fifo_tid[out_fifo_rd_ptr_reg[OUTPUT_FIFO_ADDR_WIDTH-1:0]];
            m_axis_tdest_reg <= out_fifo_tdest[out_fifo_rd_ptr_reg[OUTPUT_FIFO_ADDR_WIDTH-1:0]];
            m_axis_tuser_reg <= out_fifo_tuser[out_fifo_rd_ptr_reg[OUTPUT_FIFO_ADDR_WIDTH-1:0]];
            out_fifo_rd_ptr_reg <= out_fifo_rd_ptr_reg + 1;
        end

        if (m_rst) begin
            out_fifo_wr_ptr_reg <= 0;
            out_fifo_rd_ptr_reg <= 0;
            m_axis_tvalid_reg <= 1'b0;
        end
    end

end

endgenerate

endmodule

`resetall

