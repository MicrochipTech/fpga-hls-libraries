///////////////////////////////////////////////////////////////////////////////////////////////////
// File: IMX334_IF_TB.v
// Description:
//   MIPI CSI-2 D-PHY testbench for IMX334_IF_TOP. Sends an HS training burst to align
//   the camera clock and data signals, then transmits a test frame consisting of a Frame
//   Start short packet, RAW10 line packets, and a Frame End short packet. The AXI4-Stream
//   output is then verified against the transmitted pixel data.
//
//   See https://www.mouser.com/pdfDocs/Microsemi_MPF300-VIDEO-KIT_UG.pdf?srsltid=AfmBOoojb5anx-qLa0CZXDx2UlEIDuzevzo7QsOGZ79IddjvfjGI8P9f
//   for more information.      
//
// Targeted device: PolarFire MPF300TS FCG1152
//
// Author: Samuel Ho
///////////////////////////////////////////////////////////////////////////////////////////////////

`timescale 1ns/100ps

module testbench;

// ============================================================================
// Parameters
// ============================================================================
// D-PHY timing
parameter real DPHY_CLK_HALF = 0.84;  // DPHY HS clock half-period: 1/(2*594MHz)
parameter real DPHY_BIT_HALF = 0.42;   // half bit-period
parameter real AXIS_CLK_HALF = 3.37;  // AXI-S clock half-period: 1/(2*148.5MHz)

// MIPI D-PHY protocol timings
parameter real TLPX        = 50.0;   // LP pulse width (ns)
parameter real THS_PREPARE = 40.0;   // data lane prepare time (ns)
parameter real THS_ZERO    = 160.0;  // HS-zero hold time (ns)
parameter real HS_EXIT_T   = 100.0;  // HS exit to LP11 (ns)

// Frame dimensions
parameter integer FRAME_WIDTH  = 32;   // pixels per line
parameter integer FRAME_HEIGHT = 32;    // lines per frame
parameter integer TRAINING_TIME = 50000;

// ============================================================================
// Signal declarations
// ============================================================================
reg i_axis_clk  = 1'b0;
reg arst_n      = 1'b0;
reg trng_rst_n  = 1'b0;
reg init_done   = 1'b0;
reg axis_resetn = 1'b0;

// D-PHY data lanes
reg [3:0] cam_rxd   = 4'b1111; // Set to high in idle mode (LP11)
reg [3:0] cam_rxd_n = 4'b1111;

// D-PHY clock lane
reg cam_clk_p = 1'b1;
reg cam_clk_n = 1'b1;

// AXI-Stream outputs from DUT
wire [31:0] axis_tdata;
wire        axis_tlast;
wire  [0:0] axis_tuser;
wire        axis_tvalid;

// Image (RAW8: 8 bits per pixel)
reg [FRAME_WIDTH*8-1:0] image [0:FRAME_HEIGHT-1];

// Initialize Image to pass in
integer row, col, val;
initial begin
    val = 1;
    for (row = 0; row < FRAME_HEIGHT; row = row + 1) begin
        for (col = 0; col < FRAME_WIDTH; col = col + 1) begin
            image[row][col*8 +: 8] = val[7:0];
            val = val + 1;
        end
    end
end

// ============================================================================
// IOD LP glitch filter (simulation artifact workaround - see DS60001727 §7.2.5.3)
// Prevents false LP_DATA_N toggling during HS mode in simulation.
// ============================================================================
parameter MIPI_LP     = 1'b1;
parameter LP_FLTR_VAL = 100;

defparam DUT.PF_IOD_GENERIC_RX_C0_1.PF_IOD_0.PF_IOD_RX.I_INBUF_DIFF_MIPI_3.MIPI_LP = MIPI_LP;
defparam DUT.PF_IOD_GENERIC_RX_C0_1.PF_IOD_0.PF_IOD_RX.I_INBUF_DIFF_MIPI_2.MIPI_LP = MIPI_LP;
defparam DUT.PF_IOD_GENERIC_RX_C0_1.PF_IOD_0.PF_IOD_RX.I_INBUF_DIFF_MIPI_1.MIPI_LP = MIPI_LP;
defparam DUT.PF_IOD_GENERIC_RX_C0_1.PF_IOD_0.PF_IOD_RX.I_INBUF_DIFF_MIPI_0.MIPI_LP = MIPI_LP;

defparam DUT.PF_IOD_GENERIC_RX_C0_1.PF_IOD_0.PF_IOD_RX.I_INBUF_DIFF_MIPI_3.LP_FLTR_VAL = LP_FLTR_VAL;
defparam DUT.PF_IOD_GENERIC_RX_C0_1.PF_IOD_0.PF_IOD_RX.I_INBUF_DIFF_MIPI_2.LP_FLTR_VAL = LP_FLTR_VAL;
defparam DUT.PF_IOD_GENERIC_RX_C0_1.PF_IOD_0.PF_IOD_RX.I_INBUF_DIFF_MIPI_1.LP_FLTR_VAL = LP_FLTR_VAL;
defparam DUT.PF_IOD_GENERIC_RX_C0_1.PF_IOD_0.PF_IOD_RX.I_INBUF_DIFF_MIPI_0.LP_FLTR_VAL = LP_FLTR_VAL;

defparam DUT.PF_IOD_GENERIC_RX_C0_1.PF_IOD_0.PF_IOD_RX.I_IOD_3.MIPI_LP = MIPI_LP;
defparam DUT.PF_IOD_GENERIC_RX_C0_1.PF_IOD_0.PF_IOD_RX.I_IOD_2.MIPI_LP = MIPI_LP;
defparam DUT.PF_IOD_GENERIC_RX_C0_1.PF_IOD_0.PF_IOD_RX.I_IOD_1.MIPI_LP = MIPI_LP;
defparam DUT.PF_IOD_GENERIC_RX_C0_1.PF_IOD_0.PF_IOD_RX.I_IOD_0.MIPI_LP = MIPI_LP;

defparam DUT.PF_IOD_GENERIC_RX_C0_1.PF_IOD_0.PF_IOD_RX.I_IOD_3.LP_FLTR_VAL = LP_FLTR_VAL;
defparam DUT.PF_IOD_GENERIC_RX_C0_1.PF_IOD_0.PF_IOD_RX.I_IOD_2.LP_FLTR_VAL = LP_FLTR_VAL;
defparam DUT.PF_IOD_GENERIC_RX_C0_1.PF_IOD_0.PF_IOD_RX.I_IOD_1.LP_FLTR_VAL = LP_FLTR_VAL;
defparam DUT.PF_IOD_GENERIC_RX_C0_1.PF_IOD_0.PF_IOD_RX.I_IOD_0.LP_FLTR_VAL = LP_FLTR_VAL;

// ============================================================================
// DUT Instantiation
// ============================================================================
IMX334_IF_TOP DUT (
    .ARST_N        (arst_n),
    .AXIS_i_tready (1'b1),
    .CAM2_RXD      (cam_rxd),
    .CAM2_RXD_N    (cam_rxd_n),
    .CAM2_RX_CLK_P (cam_clk_p),
    .CAM2_RX_CLK_N (cam_clk_n),
    .INIT_DONE     (init_done),
    .TRNG_RST_N    (trng_rst_n),
    .i_axis_clk    (i_axis_clk),
    .i_axis_resetn (axis_resetn),
    .AXIS_o_tdata  (axis_tdata),
    .AXIS_o_tlast  (axis_tlast),
    .AXIS_o_tuser  (axis_tuser),
    .AXIS_o_tvalid (axis_tvalid)
);

// ============================================================================
// Clocks
// ============================================================================

// AXI-S Clock: 148.5 MHz
always #(AXIS_CLK_HALF) i_axis_clk = ~i_axis_clk;

// D-PHY HS Clock driver
// For simplicity we keep these clocks runnning all the time and never switch to low power mode.
always begin
    cam_clk_p = 1'b1; cam_clk_n = 1'b0; #(DPHY_CLK_HALF);
    cam_clk_p = 1'b0; cam_clk_n = 1'b1; #(DPHY_CLK_HALF);
end

// ============================================================================
// Task: send_beat
//   Transmit one 4-lane beat (one byte per lane, LSB first) in HS mode.
//   b0->Lane0, b1->Lane1, b2->Lane2, b3->Lane3.
// ============================================================================
task send_beat;
    input [7:0] b0, b1, b2, b3;
    integer i;
    begin
        for (i = 0; i < 8; i = i+1) begin
            cam_rxd   = {b3[i], b2[i], b1[i], b0[i]};
            cam_rxd_n = ~{b3[i], b2[i], b1[i], b0[i]};
            #(DPHY_BIT_HALF * 2.0);
        end
    end
endtask

// ============================================================================
// Task: start_of_transmission
// ============================================================================
task start_of_transmission;
    begin
        // Allow clock lane LP01->LP00->HS to complete before starting data
        #(TLPX + THS_PREPARE + 5.0);

        // LP01: P=0, N=1
        cam_rxd = 4'b0000; cam_rxd_n = 4'b1111;
        #(TLPX);
        // LP00: P=0, N=0
        cam_rxd_n = 4'b0000;
        #(THS_PREPARE);
        // HS-zero: differential 0 (P=0, N=1) for THS_ZERO duration
        cam_rxd = 4'b0000; cam_rxd_n = 4'b1111;
        #(THS_ZERO);
        // Align to clock: wait for next rising edge then offset by half a bit period
        // so data transitions land in the center of each clock half-period.
        @(posedge cam_clk_p);
        #(DPHY_BIT_HALF);
        // SOT sync byte 0xB8 on all lanes (starts HS data stream)
        send_beat(8'hB8, 8'hB8, 8'hB8, 8'hB8);
    end
endtask

// ============================================================================
// Task: end_of_transmission
// ============================================================================
task end_of_transmission;
    begin
        cam_rxd   = 4'b1111;
        cam_rxd_n = 4'b1111;
        #(HS_EXIT_T);
        #(200.0); // inter-packet LP gap
    end
endtask


// ============================================================================
// CSI-2 Error Correction Code: 6-bit Hamming code over the 3-byte packet header.
// Input d[23:0] = {Data ID, Word Count}
// Output [7:0]  = {VXC[1:0],P5,P4,P3,P2,P1,P0}
// ============================================================================
function [7:0] csi2_ecc;
    input [23:0] d;
    reg [5:0] p;
    begin
        p[0] = d[0]^d[1]^d[2]^d[4]^d[5]^d[7]^d[10]^d[11]^d[13]^d[16]^d[20]^d[21]^d[22]^d[23];
        p[1] = d[0]^d[1]^d[3]^d[4]^d[6]^d[8]^d[10]^d[12]^d[14]^d[17]^d[20]^d[21]^d[22]^d[23];
        p[2] = d[0]^d[2]^d[3]^d[5]^d[6]^d[9]^d[11]^d[12]^d[15]^d[18]^d[20]^d[21]^d[22];
        p[3] = d[1]^d[2]^d[3]^d[7]^d[8]^d[9]^d[13]^d[14]^d[15]^d[19]^d[20]^d[21]^d[23];
        p[4] = d[4]^d[5]^d[6]^d[7]^d[8]^d[9]^d[16]^d[17]^d[18]^d[19]^d[20]^d[22]^d[23];
        p[5] = d[10]^d[11]^d[12]^d[13]^d[14]^d[15]^d[16]^d[17]^d[18]^d[19]^d[21]^d[22]^d[23];
        csi2_ecc = {2'b00, p[5], p[4], p[3], p[2], p[1], p[0]};
    end
endfunction

// ============================================================================
// CSI-2 CRC-16 (Cyclic Redundancy Check) byte update
// Polynomial: x^16 + x^12 + x^5 + 1
// Reflected polynomial: 16'h8408
// ============================================================================
function [15:0] csi2_crc16_update;
    input [15:0] crc_in;
    input [7:0]  data_in;

    reg [15:0] crc;
    reg [7:0]  data;
    reg        feedback;
    integer    bit_index;

    begin
        crc  = crc_in;
        data = data_in;

        for (bit_index = 0; bit_index < 8; bit_index = bit_index + 1) begin
            feedback = crc[0] ^ data[0];

            crc  = crc >> 1;
            data = data >> 1;

            if (feedback)
                crc = crc ^ 16'h8408;
        end

        csi2_crc16_update = crc;
    end
endfunction

// ============================================================================
// Task: send_long_packet
//   Sends one CSI-2 long packet carrying a single RAW10 video line.
//   Accepts 8-bit pixels and packs them as RAW10 with the 2 LSBs zeroed,
//   matching what the camera outputs (DUT strips those LSBs on the way out).
// ============================================================================
task send_long_packet;
    input [FRAME_WIDTH*8-1:0] line_data;
    reg [7:0]  b0, b1, b2, b3;
    reg [23:0] header_data;
    reg [15:0] crc;
    integer    beat_idx, num_data_beats, group;
    reg [7:0]  stream [0:FRAME_WIDTH*10/8-1];

    localparam [15:0] WORD_COUNT = FRAME_WIDTH * 10 / 8; // RAW10: 40 bytes for 32 pixels
    localparam [7:0]  DATA_ID   = {2'b0, 6'h2B};         // RAW10 data type
    begin

        // Pack 4 pixels per 5-byte group; pixel value in upper 8 bits, LSBs zeroed
        for (group = 0; group < FRAME_WIDTH/4; group = group + 1) begin
            stream[5*group+0] = line_data[(4*group+0)*8 +: 8];
            stream[5*group+1] = line_data[(4*group+1)*8 +: 8];
            stream[5*group+2] = line_data[(4*group+2)*8 +: 8];
            stream[5*group+3] = line_data[(4*group+3)*8 +: 8];
            stream[5*group+4] = 8'h00;
        end

        start_of_transmission;

        // Header: DI, WC_L, WC_H, ECC
        header_data = {WORD_COUNT, DATA_ID};
        b0 = header_data[7:0];
        b1 = header_data[15:8];
        b2 = header_data[23:16];
        b3 = csi2_ecc(header_data);
        send_beat(b0, b1, b2, b3);

        crc = 16'hFFFF;

        // Data beats (10 beats × 4 bytes = 40 bytes for 32 RAW10 pixels)
        num_data_beats = WORD_COUNT / 4;
        for (beat_idx = 0; beat_idx < num_data_beats; beat_idx = beat_idx + 1) begin
            b0 = stream[beat_idx*4+0];
            b1 = stream[beat_idx*4+1];
            b2 = stream[beat_idx*4+2];
            b3 = stream[beat_idx*4+3];
            send_beat(b0, b1, b2, b3);
            crc = csi2_crc16_update(crc, b0);
            crc = csi2_crc16_update(crc, b1);
            crc = csi2_crc16_update(crc, b2);
            crc = csi2_crc16_update(crc, b3);
        end

        // Footer: CRC_L, CRC_H, pad, pad
        send_beat(crc[7:0], crc[15:8], 8'h00, 8'h00);

        end_of_transmission;
    end
endtask




// ============================================================================
// Task: send_short_packet
//   Sends a CSI-2 short packet (4 bytes, one per lane in a single beat).
//   dt[5:0]: CSI-2 Data Type  (0x00=FS, 0x01=FE)
//   data_field[15:0]: 16-bit short data field
// ============================================================================
task send_short_packet;
    input [7:0]  data_type;
    input [15:0] data_field;
    reg [7:0] b0, b1, b2, b3;
    begin
        b0 = data_type;
        b1 = data_field[7:0];
        b2 = data_field[15:8];
        b3 = csi2_ecc({b2, b1, b0});
        start_of_transmission;
        send_beat(b0, b1, b2, b3);   // L0=DI, L1=word_count_L, L2=word_count_H, L3=ECC
        end_of_transmission;
    end
endtask



// ============================================================================
// Task: send_training_burst
//   Sends a D-PHY HS training burst so CORERXIODBITALIGN can bit-align,
//   then polls training_done_o until the IOD signals it is ready.
//   Must be called once after reset, before the first send_frame.
// ============================================================================
task send_training_burst;
    integer training_wait_cnt;
    begin

        $display("[%0t] Training preamble burst...", $time);
        start_of_transmission;
        begin : training_preamble_loop

            training_wait_cnt = 0;
            while ((!DUT.PF_IOD_GENERIC_RX_C0_1.training_done_o ||
                    !DUT.PF_IOD_GENERIC_RX_C0_1.CLK_TRAIN_DONE) &&
                   training_wait_cnt < TRAINING_TIME) begin
                if (training_wait_cnt[0])
                    send_beat(8'h55, 8'h55, 8'h55, 8'h55);
                else
                    send_beat(8'hAA, 8'hAA, 8'hAA, 8'hAA);
                training_wait_cnt = training_wait_cnt + 1;
            end
        end_of_transmission;
        #(200.0);  // extra LP gap after training burst

        end
        if (training_wait_cnt == TRAINING_TIME)
            $display("[%0t] WARNING: training timed out - proceeding anyway", $time);
        else
            $display("[%0t] training_done_o and CLK_TRAIN_DONE both asserted", $time);
    end
endtask

// ============================================================================
// Task: send_frame
//   Sends one complete frame
//   frame_num : frame counter written into short-packet data field
//   width     : pixels per line (must be a multiple of 4)
//   height    : number of lines
// ============================================================================
task send_frame;
    input integer frame_num;
    input [FRAME_WIDTH*8-1:0] img [0:FRAME_HEIGHT-1];
    integer line;
    begin
        // Frame Start short packet (DT=0x00)
        $display("[%0t] Frame Start (frame=%0d)", $time, frame_num);
        send_short_packet(6'h00, frame_num[15:0]);

        // Video lines
        for (line = 0; line < FRAME_HEIGHT; line = line + 1) begin
            $display("[%0t]   Line %0d", $time, line);
            send_long_packet(img[line]);
        end

        // Frame End short packet (DT=0x01)
        $display("[%0t] Frame End (frame=%0d)", $time, frame_num);
        send_short_packet(6'h01, frame_num[15:0]);

        $display("[%0t] Frame %0d complete.", $time, frame_num);
    end
endtask

// ============================================================================
// Output monitor: print and verify each valid AXI-S beat against input image.
// DUT outputs 8 bits per pixel, 4 pixels per 32-bit beat (8 beats per line).
// ============================================================================
integer monitor_frame  = 0;
integer monitor_line   = 0;
integer monitor_beat   = 0;
integer monitor_errors = 0;

always @(posedge i_axis_clk) begin
    if (axis_tvalid) begin
        if (axis_tuser[0]) begin
            monitor_frame = monitor_frame + 1;
            monitor_line  = 1; // The first line contain camera data and are not a part of the actual image
            monitor_beat  = 0;
            $display("[%0t] AXIS >>> START-OF-FRAME (frame %0d) <<<", $time, monitor_frame);
        end

        $display("[%0t] AXIS data=0x%08h  tlast=%b", $time, axis_tdata, axis_tlast);

        if (axis_tdata[ 7: 0] !== image[monitor_line][(monitor_beat*4+0)*8 +: 8]) begin
            $display("[%0t] MISMATCH frame=%0d line=%0d pixel=%0d  got=0x%02h expected=0x%02h",
                     $time, monitor_frame, monitor_line, monitor_beat*4+0,
                     axis_tdata[7:0], image[monitor_line][(monitor_beat*4+0)*8 +: 8]);
            monitor_errors = monitor_errors + 1;
        end
        if (axis_tdata[15: 8] !== image[monitor_line][(monitor_beat*4+1)*8 +: 8]) begin
            $display("[%0t] MISMATCH frame=%0d line=%0d pixel=%0d  got=0x%02h expected=0x%02h",
                     $time, monitor_frame, monitor_line, monitor_beat*4+1,
                     axis_tdata[15:8], image[monitor_line][(monitor_beat*4+1)*8 +: 8]);
            monitor_errors = monitor_errors + 1;
        end
        if (axis_tdata[23:16] !== image[monitor_line][(monitor_beat*4+2)*8 +: 8]) begin
            $display("[%0t] MISMATCH frame=%0d line=%0d pixel=%0d  got=0x%02h expected=0x%02h",
                     $time, monitor_frame, monitor_line, monitor_beat*4+2,
                     axis_tdata[23:16], image[monitor_line][(monitor_beat*4+2)*8 +: 8]);
            monitor_errors = monitor_errors + 1;
        end
        if (axis_tdata[31:24] !== image[monitor_line][(monitor_beat*4+3)*8 +: 8]) begin
            $display("[%0t] MISMATCH frame=%0d line=%0d pixel=%0d  got=0x%02h expected=0x%02h",
                     $time, monitor_frame, monitor_line, monitor_beat*4+3,
                     axis_tdata[31:24], image[monitor_line][(monitor_beat*4+3)*8 +: 8]);
            monitor_errors = monitor_errors + 1;
        end

        monitor_beat = monitor_beat + 1;
        if (axis_tlast) begin
            monitor_line = monitor_line + 1;
            monitor_beat = 0;
        end
    end
end

// ============================================================================
// Main stimulus
// ============================================================================
initial begin
    $display("=================================================");
    $display("IMX334_IF testbench: %0d x %0d", FRAME_WIDTH, FRAME_HEIGHT);
    $display("=================================================");

    // Reset sequence
    arst_n     = 1'b0;
    trng_rst_n = 1'b0;
    init_done  = 1'b0;
    axis_resetn = 1'b0;
    #200.0;
    arst_n     = 1'b1;
    #100.0;
    trng_rst_n = 1'b1;
    #100.0;
    init_done  = 1'b1;
    #100.0;
    axis_resetn = 1'b1;
    #500.0;               

    // Training burst
    send_training_burst;

    // Not sure if this is a bug
    // bit_align_done does not yet stabilize on the first frame, 
    // so we need to send a dummy frame first
    send_short_packet(6'h00, 16'b0);
    send_short_packet(6'h01, 16'b0);

    // Send test frame
    send_frame(1, image);

    // End of simulation
    #2000.0;
    if (monitor_errors == 0)
        $display("[%0t] Simulation complete. All pixels match.", $time);
    else
        $display("[%0t] Simulation complete. %0d pixel mismatches detected.", $time, monitor_errors);
    $finish;
end

endmodule
