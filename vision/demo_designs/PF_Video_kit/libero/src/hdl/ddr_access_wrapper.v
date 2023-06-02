`timescale 1 ns / 1 ns
`define MEMORY_CONTROLLER_ADDR_SIZE 32

module DDR_Access_wrapper_top # (
  parameter ADDR_WIDTH = 32,
  parameter AXI_DATA_WIDTH = 64,
  parameter AXI_ID_WIDTH = 1
) (
    input  clk,
    input  reset,
    input  start_write,
    input  start_read,
    input [`MEMORY_CONTROLLER_ADDR_SIZE-1:0] buf_var, //DDR address which we write to and read from
    input [31:0] hres,
    input [31:0] vres,
    output [31:0] axi4initiator_aw_addr,
    input  axi4initiator_aw_ready,
    output  axi4initiator_aw_valid,
    output [1:0] axi4initiator_aw_burst,
    output [2:0] axi4initiator_aw_size,
    output [7:0] axi4initiator_aw_len,
    input [31:0] VideoIn_data,
    output  VideoIn_ready,
    input  VideoIn_valid,
    input  VideoIn_last,
    input  VideoIn_user,
    output [63:0] axi4initiator_w_data,
    input  axi4initiator_w_ready,
    output  axi4initiator_w_valid,
    output [7:0] axi4initiator_w_strb,
    output  axi4initiator_w_last,
    input [1:0] axi4initiator_b_resp,
    output  axi4initiator_b_resp_ready,
    input  axi4initiator_b_resp_valid,
    output [31:0] axi4initiator_ar_addr,
    input  axi4initiator_ar_ready,
    output  axi4initiator_ar_valid,
    output [1:0] axi4initiator_ar_burst,
    output [2:0] axi4initiator_ar_size,
    output [7:0] axi4initiator_ar_len,
    input [63:0] axi4initiator_r_data,
    output  axi4initiator_r_ready,
    input  axi4initiator_r_valid,
    input [1:0] axi4initiator_r_resp,
    input  axi4initiator_r_last,
    output [31:0] VideoOut_data,
    input  VideoOut_ready,
    output  VideoOut_valid,
    output  VideoOut_last,
    output  VideoOut_user);

wire writer_finish;
wire [`MEMORY_CONTROLLER_ADDR_SIZE-1:0] wr_address;
wire [`MEMORY_CONTROLLER_ADDR_SIZE-1:0] rd_address;

FrameBufferControl (
    .clk(clk),
    .reset(reset),
    .wr_finish(writer_finish),
    .base_address(buf_var),
    .wr_address(wr_address),
    .rd_address(rd_address)
);

DDR_Write_wrapper_top # (
    .ADDR_WIDTH(ADDR_WIDTH),
    .AXI_DATA_WIDTH(AXI_DATA_WIDTH),
    .AXI_ID_WIDTH(AXI_ID_WIDTH)
) DDR_Write_Inst(
    .clk(clk),
    .reset(reset),
    .start(start_write),
    .finish(writer_finish),
    .Buf(wr_address),
    .HRes(hres),
    .VRes(vres),
    .axi4initiator_aw_addr(axi4initiator_aw_addr),
    .axi4initiator_aw_ready(axi4initiator_aw_ready),
    .axi4initiator_aw_valid(axi4initiator_aw_valid),
    .axi4initiator_aw_burst(axi4initiator_aw_burst),
    .axi4initiator_aw_size(axi4initiator_aw_size),
    .axi4initiator_aw_len(axi4initiator_aw_len),
    .VideoIn_data(VideoIn_data),
    .VideoIn_ready(VideoIn_ready),
    .VideoIn_valid(VideoIn_valid),
    .VideoIn_last(VideoIn_last),
    .VideoIn_user(VideoIn_user),
    .axi4initiator_w_data(axi4initiator_w_data),
    .axi4initiator_w_ready(axi4initiator_w_ready),
    .axi4initiator_w_valid(axi4initiator_w_valid),
    .axi4initiator_w_strb(axi4initiator_w_strb),
    .axi4initiator_w_last(axi4initiator_w_last),
    .axi4initiator_b_resp(axi4initiator_b_resp),
    .axi4initiator_b_resp_ready(axi4initiator_b_resp_ready),
    .axi4initiator_b_resp_valid(axi4initiator_b_resp_valid)
);



DDR_Read_wrapper_top # (
    .ADDR_WIDTH(ADDR_WIDTH),
    .AXI_DATA_WIDTH(AXI_DATA_WIDTH),
    .AXI_ID_WIDTH(AXI_ID_WIDTH)
) (
  .clk(clk),
  .reset(reset),
  .start(start_read),
  .Buf(rd_address),
  .HRes(hres),
  .VRes(vres),
  .axi4initiator_ar_addr(axi4initiator_ar_addr),
  .axi4initiator_ar_ready(axi4initiator_ar_ready),
  .axi4initiator_ar_valid(axi4initiator_ar_valid),
  .axi4initiator_ar_burst(axi4initiator_ar_burst),
  .axi4initiator_ar_size(axi4initiator_ar_size),
  .axi4initiator_ar_len(axi4initiator_ar_len),
  .axi4initiator_r_data(axi4initiator_r_data),
  .axi4initiator_r_ready(axi4initiator_r_ready),
  .axi4initiator_r_valid(axi4initiator_r_valid),
  .axi4initiator_r_resp(axi4initiator_r_resp),
  .axi4initiator_r_last(axi4initiator_r_last),
  .VideoOut_data(VideoOut_data),
  .VideoOut_ready(VideoOut_ready),
  .VideoOut_valid(VideoOut_valid),
  .VideoOut_last(VideoOut_last),
  .VideoOut_user(VideoOut_user)
);

endmodule

// Each time the DDR writer finishes writing a frame to DDR, FrameBufferControl
// provides a new address for the DDR writer to write the next frame, and
// provides to the DDR reader with the address of latest frame (the frame that
// just got written to DDR by DDR writer).
// The number of frames and each frame's byte size can be parameterized.
module FrameBufferControl # (
    parameter NUM_BUFFERS = 4,
    parameter BUFFER_SIZE = (1 << 23)  // 8 Mbytes.
) (
    input clk,
    input reset,
    input wr_finish,
    input [31:0] base_address,
    output reg [31:0] wr_address,
    output reg [31:0] rd_address
);

reg [7:0] wr_idx_count = 0;

always @ (posedge clk) begin
    if (reset) begin
        wr_idx_count <= 0;
        wr_address <= base_address;
        rd_address <= base_address;
    end else if (wr_finish) begin
        rd_address <= wr_address;
        if (wr_idx_count == NUM_BUFFERS - 1) begin
            wr_idx_count <=  0;
            wr_address <= base_address;
        end else begin
            wr_idx_count <= wr_idx_count + 1;
            wr_address <= wr_address + BUFFER_SIZE;
        end
    end
end

endmodule
