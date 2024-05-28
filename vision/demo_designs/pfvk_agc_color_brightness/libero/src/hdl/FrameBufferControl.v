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

