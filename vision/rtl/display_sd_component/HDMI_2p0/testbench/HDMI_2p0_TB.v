`timescale 1ns / 100ps

module testbench;

parameter real CLK_HALF     = 3.37;  // 148.5 MHz

reg         ref_p   = 0;
reg         ref_n   = 1;
reg         reset_n = 0;

reg  [95:0] tdata   = 0;
reg         tvalid  = 0;
reg         tlast   = 0;
reg  [0:0]  tuser   = 0;

wire        tready;
wire        lane0_txd_p, lane0_txd_n;
wire        lane1_txd_p, lane1_txd_n;
wire        lane2_txd_p, lane2_txd_n;
wire        lane3_txd_p, lane3_txd_n;
wire        lane0_tx_clk_r;

always #(CLK_HALF) ref_p = ~ref_p;
always #(CLK_HALF) ref_n = ~ref_n;

HDMI_2p0 DUT (
    .AXIS_tdata     (tdata),
    .AXIS_tlast     (tlast),
    .AXIS_tuser     (tuser),
    .AXIS_tvalid    (tvalid),
    .LANE0_RXD_N    (1'b0), .LANE0_RXD_P (1'b1),
    .LANE1_RXD_N    (1'b0), .LANE1_RXD_P (1'b1),
    .LANE2_RXD_N    (1'b0), .LANE2_RXD_P (1'b1),
    .LANE3_RXD_N    (1'b0), .LANE3_RXD_P (1'b1),
    .REF_CLK_PAD_N  (ref_n),
    .REF_CLK_PAD_P  (ref_p),
    .RESET_N_I_0    (reset_n),
    .AXIS_tready_O  (tready),
    .LANE0_TXD_N    (lane0_txd_n), .LANE0_TXD_P (lane0_txd_p),
    .LANE0_TX_CLK_R (lane0_tx_clk_r),
    .LANE1_TXD_N    (lane1_txd_n), .LANE1_TXD_P (lane1_txd_p),
    .LANE2_TXD_N    (lane2_txd_n), .LANE2_TXD_P (lane2_txd_p),
    .LANE3_TXD_N    (lane3_txd_n), .LANE3_TXD_P (lane3_txd_p),
    .SDA            ()
);

parameter integer H_BEATS  = 960;   // 3840 pixels / 4 ppc
parameter integer V_LINES  = 2160;
parameter integer NUM_FRAMES = 2;   // need ≥2 frames for VGA controller to lock

integer frame, line, beat;
integer pixel_val;

initial begin
    reset_n = 0;
    #500;
    reset_n = 1;
    // Wait for lane0_tx_clk_r to start (SerDes PLL lock)
    wait (lane0_tx_clk_r === 1'b0 || lane0_tx_clk_r === 1'b1);
    repeat (200) @(posedge lane0_tx_clk_r);

    // Skip Training
    force DUT.AXIS_To_VGA_Converter_0.display_data = 1'b1;

    pixel_val = 0;
    for (frame = 0; frame < NUM_FRAMES; frame = frame + 1) begin
        for (line = 0; line < V_LINES; line = line + 1) begin
            for (beat = 0; beat < H_BEATS; beat = beat + 1) begin
                tdata  = { 8'(pixel_val+3), 8'(pixel_val+3), 8'(pixel_val+3),
                           8'(pixel_val+2), 8'(pixel_val+2), 8'(pixel_val+2),
                           8'(pixel_val+1), 8'(pixel_val+1), 8'(pixel_val+1),
                           8'(pixel_val+0), 8'(pixel_val+0), 8'(pixel_val+0) };
                tvalid = 1;
                tuser  = (frame == 0 && line == 0 && beat == 0);  // SOF: once per frame
                tlast  = (beat == H_BEATS - 1);                   // EOL

                @(posedge lane0_tx_clk_r);
                while (tready !== 1'b1) @(posedge lane0_tx_clk_r);

                pixel_val = pixel_val + 4;
            end
        end
    end

    @(posedge lane0_tx_clk_r);
    tvalid = 0; tlast = 0; tuser = 0;
    repeat (20) @(posedge lane0_tx_clk_r);
    $finish;
end
endmodule
