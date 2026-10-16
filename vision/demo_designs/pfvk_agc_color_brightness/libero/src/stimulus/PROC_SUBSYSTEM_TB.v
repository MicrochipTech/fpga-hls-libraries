///////////////////////////////////////////////////////////////////////////////////////////////////
// File: PROC_SUBSYSTEM_TB.v
// Description:
//   Simulation testbench for PROC_SUBSYSTEM. 
//   Logs I2C, Reset, and AXI4 Slave transactions.
//
// Targeted device: PolarFire MPF300TS FCG1152
//
// Author: Samuel Ho
///////////////////////////////////////////////////////////////////////////////////////////////////

`timescale 1ns / 100ps

module testbench;

// -----------------------------------------------------------------------
// Clocks and Resets
// -----------------------------------------------------------------------
reg axi_clk  = 0;
reg PCLK     = 0;
reg reset_n    = 0;

always #3.367 axi_clk = ~axi_clk;  // 148.5 MHz  (period = 6.734 ns)
always #3.333 PCLK    = ~PCLK;     // 150.0 MHz  (period = 6.667 ns)

// -----------------------------------------------------------------------
// JTAG (not used)
// -----------------------------------------------------------------------
reg TCK   = 0;
reg TDI   = 0;
reg TMS   = 1;
reg TRSTB = 0;
wire TDO;

// -----------------------------------------------------------------------
// UART RX: idle high
// -----------------------------------------------------------------------
reg UART_RX = 1;

// -----------------------------------------------------------------------
// AXI4 Slave
// -----------------------------------------------------------------------
reg        AXI4mslave0_SLAVE0_ARREADY = 1;
reg        AXI4mslave0_SLAVE0_AWREADY = 1;
reg [1:0]  AXI4mslave0_SLAVE0_BID     = 0;
reg [1:0]  AXI4mslave0_SLAVE0_BRESP   = 0;
reg [0:0]  AXI4mslave0_SLAVE0_BUSER   = 0;
reg        AXI4mslave0_SLAVE0_BVALID  = 0;
reg [63:0] AXI4mslave0_SLAVE0_RDATA   = 0;
reg [1:0]  AXI4mslave0_SLAVE0_RID     = 0;
reg        AXI4mslave0_SLAVE0_RLAST   = 0;
reg [1:0]  AXI4mslave0_SLAVE0_RRESP   = 0;
reg [0:0]  AXI4mslave0_SLAVE0_RUSER   = 0;
reg        AXI4mslave0_SLAVE0_RVALID  = 0;
reg        AXI4mslave0_SLAVE0_WREADY  = 1;
wire [31:0] AXI4mslave0_SLAVE0_ARADDR;
wire [1:0]  AXI4mslave0_SLAVE0_ARBURST;
wire [3:0]  AXI4mslave0_SLAVE0_ARCACHE;
wire [1:0]  AXI4mslave0_SLAVE0_ARID;
wire [7:0]  AXI4mslave0_SLAVE0_ARLEN;
wire [1:0]  AXI4mslave0_SLAVE0_ARLOCK;
wire [2:0]  AXI4mslave0_SLAVE0_ARPROT;
wire [3:0]  AXI4mslave0_SLAVE0_ARQOS;
wire [3:0]  AXI4mslave0_SLAVE0_ARREGION;
wire [2:0]  AXI4mslave0_SLAVE0_ARSIZE;
wire [0:0]  AXI4mslave0_SLAVE0_ARUSER;
wire        AXI4mslave0_SLAVE0_ARVALID;
wire [31:0] AXI4mslave0_SLAVE0_AWADDR;
wire [1:0]  AXI4mslave0_SLAVE0_AWBURST;
wire [3:0]  AXI4mslave0_SLAVE0_AWCACHE;
wire [1:0]  AXI4mslave0_SLAVE0_AWID;
wire [7:0]  AXI4mslave0_SLAVE0_AWLEN;
wire [1:0]  AXI4mslave0_SLAVE0_AWLOCK;
wire [2:0]  AXI4mslave0_SLAVE0_AWPROT;
wire [3:0]  AXI4mslave0_SLAVE0_AWQOS;
wire [3:0]  AXI4mslave0_SLAVE0_AWREGION;
wire [2:0]  AXI4mslave0_SLAVE0_AWSIZE;
wire [0:0]  AXI4mslave0_SLAVE0_AWUSER;
wire        AXI4mslave0_SLAVE0_AWVALID;
wire        AXI4mslave0_SLAVE0_BREADY;
wire        AXI4mslave0_SLAVE0_RREADY;
wire [63:0] AXI4mslave0_SLAVE0_WDATA;
wire        AXI4mslave0_SLAVE0_WLAST;
wire [7:0]  AXI4mslave0_SLAVE0_WSTRB;
wire [0:0]  AXI4mslave0_SLAVE0_WUSER;
wire        AXI4mslave0_SLAVE0_WVALID;

// -----------------------------------------------------------------------
// Output Wires
// -----------------------------------------------------------------------
wire        CAM2_CLK_EN;
wire        CAM2_RST;
wire        TRNG_RST_N;
wire        UART_TX;

// -----------------------------------------------------------------------
// I2C
// -----------------------------------------------------------------------
wire CAM2_SCL;
wire CAM2_SDA;

pullup (CAM2_SCL);
pullup (CAM2_SDA);

// -----------------------------------------------------------------------
// DUT
// -----------------------------------------------------------------------
PROC_SUBSYSTEM dut (
    .AXI4mslave0_SLAVE0_ARREADY (AXI4mslave0_SLAVE0_ARREADY),
    .AXI4mslave0_SLAVE0_AWREADY (AXI4mslave0_SLAVE0_AWREADY),
    .AXI4mslave0_SLAVE0_BID     (AXI4mslave0_SLAVE0_BID),
    .AXI4mslave0_SLAVE0_BRESP   (AXI4mslave0_SLAVE0_BRESP),
    .AXI4mslave0_SLAVE0_BUSER   (AXI4mslave0_SLAVE0_BUSER),
    .AXI4mslave0_SLAVE0_BVALID  (AXI4mslave0_SLAVE0_BVALID),
    .AXI4mslave0_SLAVE0_RDATA   (AXI4mslave0_SLAVE0_RDATA),
    .AXI4mslave0_SLAVE0_RID     (AXI4mslave0_SLAVE0_RID),
    .AXI4mslave0_SLAVE0_RLAST   (AXI4mslave0_SLAVE0_RLAST),
    .AXI4mslave0_SLAVE0_RRESP   (AXI4mslave0_SLAVE0_RRESP),
    .AXI4mslave0_SLAVE0_RUSER   (AXI4mslave0_SLAVE0_RUSER),
    .AXI4mslave0_SLAVE0_RVALID  (AXI4mslave0_SLAVE0_RVALID),
    .AXI4mslave0_SLAVE0_WREADY  (AXI4mslave0_SLAVE0_WREADY),
    .PCLK                       (PCLK),
    .TCK                        (TCK),
    .TDI                        (TDI),
    .TMS                        (TMS),
    .TRSTB                      (TRSTB),
    .UART_RX                    (UART_RX),
    .axi_clk                    (axi_clk),
    .reset                      (reset_n),
    .AXI4mslave0_SLAVE0_ARADDR  (AXI4mslave0_SLAVE0_ARADDR),
    .AXI4mslave0_SLAVE0_ARBURST (AXI4mslave0_SLAVE0_ARBURST),
    .AXI4mslave0_SLAVE0_ARCACHE (AXI4mslave0_SLAVE0_ARCACHE),
    .AXI4mslave0_SLAVE0_ARID    (AXI4mslave0_SLAVE0_ARID),
    .AXI4mslave0_SLAVE0_ARLEN   (AXI4mslave0_SLAVE0_ARLEN),
    .AXI4mslave0_SLAVE0_ARLOCK  (AXI4mslave0_SLAVE0_ARLOCK),
    .AXI4mslave0_SLAVE0_ARPROT  (AXI4mslave0_SLAVE0_ARPROT),
    .AXI4mslave0_SLAVE0_ARQOS   (AXI4mslave0_SLAVE0_ARQOS),
    .AXI4mslave0_SLAVE0_ARREGION(AXI4mslave0_SLAVE0_ARREGION),
    .AXI4mslave0_SLAVE0_ARSIZE  (AXI4mslave0_SLAVE0_ARSIZE),
    .AXI4mslave0_SLAVE0_ARUSER  (AXI4mslave0_SLAVE0_ARUSER),
    .AXI4mslave0_SLAVE0_ARVALID (AXI4mslave0_SLAVE0_ARVALID),
    .AXI4mslave0_SLAVE0_AWADDR  (AXI4mslave0_SLAVE0_AWADDR),
    .AXI4mslave0_SLAVE0_AWBURST (AXI4mslave0_SLAVE0_AWBURST),
    .AXI4mslave0_SLAVE0_AWCACHE (AXI4mslave0_SLAVE0_AWCACHE),
    .AXI4mslave0_SLAVE0_AWID    (AXI4mslave0_SLAVE0_AWID),
    .AXI4mslave0_SLAVE0_AWLEN   (AXI4mslave0_SLAVE0_AWLEN),
    .AXI4mslave0_SLAVE0_AWLOCK  (AXI4mslave0_SLAVE0_AWLOCK),
    .AXI4mslave0_SLAVE0_AWPROT  (AXI4mslave0_SLAVE0_AWPROT),
    .AXI4mslave0_SLAVE0_AWQOS   (AXI4mslave0_SLAVE0_AWQOS),
    .AXI4mslave0_SLAVE0_AWREGION(AXI4mslave0_SLAVE0_AWREGION),
    .AXI4mslave0_SLAVE0_AWSIZE  (AXI4mslave0_SLAVE0_AWSIZE),
    .AXI4mslave0_SLAVE0_AWUSER  (AXI4mslave0_SLAVE0_AWUSER),
    .AXI4mslave0_SLAVE0_AWVALID (AXI4mslave0_SLAVE0_AWVALID),
    .AXI4mslave0_SLAVE0_BREADY  (AXI4mslave0_SLAVE0_BREADY),
    .AXI4mslave0_SLAVE0_RREADY  (AXI4mslave0_SLAVE0_RREADY),
    .AXI4mslave0_SLAVE0_WDATA   (AXI4mslave0_SLAVE0_WDATA),
    .AXI4mslave0_SLAVE0_WLAST   (AXI4mslave0_SLAVE0_WLAST),
    .AXI4mslave0_SLAVE0_WSTRB   (AXI4mslave0_SLAVE0_WSTRB),
    .AXI4mslave0_SLAVE0_WUSER   (AXI4mslave0_SLAVE0_WUSER),
    .AXI4mslave0_SLAVE0_WVALID  (AXI4mslave0_SLAVE0_WVALID),
    .CAM2_CLK_EN                (CAM2_CLK_EN),
    .CAM2_RST                   (CAM2_RST),
    .TDO                        (TDO),
    .TRNG_RST_N                 (TRNG_RST_N),
    .UART_TX                    (UART_TX),
    .CAM2_SCL                   (CAM2_SCL),
    .CAM2_SDA                   (CAM2_SDA)
);

// -----------------------------------------------------------------------
// Stimulus
// -----------------------------------------------------------------------
initial begin
    // Assert reset for 10 PCLK cycles, then release
    reset_n = 0;
    repeat (10) @(posedge PCLK);
    reset_n = 1;
    $display("[%0t] reset released", $time);
end

// -----------------------------------------------------------------------
// Dummy AXI4 Slave Write and Read Response 
// -----------------------------------------------------------------------
always @(posedge axi_clk) begin
    // Write response:
    AXI4mslave0_SLAVE0_BVALID <= AXI4mslave0_SLAVE0_WVALID;

    // Read response: Send 0 when read is requested
    if (!AXI4mslave0_SLAVE0_RVALID) begin
        AXI4mslave0_SLAVE0_RVALID <= AXI4mslave0_SLAVE0_ARVALID;
        AXI4mslave0_SLAVE0_RLAST  <= AXI4mslave0_SLAVE0_ARVALID;
        AXI4mslave0_SLAVE0_RDATA  <= 64'd0;
    end else if (AXI4mslave0_SLAVE0_RREADY) begin
        AXI4mslave0_SLAVE0_RVALID <= 0;
        AXI4mslave0_SLAVE0_RLAST  <= 0;
    end
end

// -----------------------------------------------------------------------
// Monitors
// -----------------------------------------------------------------------
initial begin
    $monitor("[%0t] CAM2_RST=%b  CAM2_CLK_EN=%b  TRNG_RST_N=%b  UART_TX=%b",
             $time,
             CAM2_RST, CAM2_CLK_EN, TRNG_RST_N, UART_TX);
end

// Log any AXI master activity from the DUT
always @(posedge axi_clk) begin
    if (AXI4mslave0_SLAVE0_AWVALID)
        $display("[%0t] AXI AW  addr=%h len=%0d", $time,
                 AXI4mslave0_SLAVE0_AWADDR, AXI4mslave0_SLAVE0_AWLEN);
    if (AXI4mslave0_SLAVE0_WVALID)
        $display("[%0t] AXI W   data=%h strb=%h last=%b", $time,
                 AXI4mslave0_SLAVE0_WDATA, AXI4mslave0_SLAVE0_WSTRB,
                 AXI4mslave0_SLAVE0_WLAST);
    if (AXI4mslave0_SLAVE0_ARVALID)
        $display("[%0t] AXI AR  addr=%h len=%0d", $time,
                 AXI4mslave0_SLAVE0_ARADDR, AXI4mslave0_SLAVE0_ARLEN);
end

// -----------------------------------------------------------------------
// I2C slave model for IMX334 camera
// -----------------------------------------------------------------------
i2c_slave_model #(.SLAVE_ADDR(7'h10)) cam2_i2c_slave (
    .scl (CAM2_SCL),
    .sda (CAM2_SDA)
);

endmodule

// -----------------------------------------------------------------------
// I2C slave model: Simulates I2C slave transactions, and monitors the state. 
// -----------------------------------------------------------------------
module i2c_slave_model #(
    parameter [6:0] SLAVE_ADDR = 7'h1A
) (
    inout scl,
    inout sda
);

// Open-drain driver - only pull low, never drive high
reg sda_oe = 0;
assign sda = sda_oe ? 1'b0 : 1'bz;

// Internal state
reg [7:0] rx_byte  = 0;
reg [2:0] bit_cnt  = 7;   // counts down 7..0 for each bit
reg       byte_done = 0;  // pulsed when 8th bit received
reg       in_ack   = 0;   // currently driving ACK clock
reg       in_xfer  = 0;   // between START and STOP
reg       is_read  = 0;   // R/W bit from address byte
reg [7:0] byte_num = 0;   // 0=addr, 1=first data byte, ...
reg [7:0] rd_data  = 8'h00; // data returned on reads

// Detect START and STOP
reg sda_prev = 1;
always @(sda or scl) begin
    if (scl === 1'b1) begin
        if (sda_prev === 1'b1 && sda === 1'b0) begin
            // Start (SDA falls while SCL high)
            $display("[%0t] I2C: START", $time);
            in_xfer   = 1;
            bit_cnt   = 7;
            byte_num  = 0;
            rx_byte   = 0;
            byte_done = 0;
            in_ack    = 0;
            sda_oe    = 0;
        end else if (sda_prev === 1'b0 && sda === 1'b1) begin
            // STOP (SDA rises while SCL high)
            $display("[%0t] I2C: STOP", $time);
            in_xfer = 0;
            sda_oe  = 0;
        end
    end
    sda_prev = sda;
end

// Sample Data bits
always @(posedge scl) begin
    if (in_xfer && !in_ack) begin
        if (is_read && byte_num > 0) begin
            // Read transaction: nothing to sample
        end else begin
            rx_byte[bit_cnt] = sda;
            if (bit_cnt == 0)
                byte_done = 1;
            else
                bit_cnt = bit_cnt - 1;
        end
    end
end

// Drive ACK / data on falling SCL
always @(negedge scl) begin
    if (in_xfer) begin
        if (byte_done) begin
            // Byte just completed. Handle ACK.
            byte_done = 0;
            in_ack    = 1;

            if (byte_num == 0) begin
                // This is the Address byte                
                is_read = rx_byte[0];
                if (rx_byte[7:1] == SLAVE_ADDR) begin
                    $display("[%0t] I2C: ADDR=0x%02h (%s) - ACK",
                             $time, rx_byte[7:1],
                             rx_byte[0] ? "READ" : "WRITE");
                    sda_oe = 1; // ACK
                end else begin
                    $display("[%0t] I2C: ADDR=0x%02h - NACK",
                             $time, rx_byte[7:1]);
                    sda_oe = 0; // NACK
                    in_ack = 0;
                    in_xfer = 0;
                end
            end else begin
                // Data byte (write)
                $display("[%0t] I2C: DATA[%0d]=0x%02h - ACK",
                         $time, byte_num - 1, rx_byte);
                sda_oe = 1; // ACK
            end

        end else if (in_ack) begin
            // ACK clock has passed - release SDA, prepare for next byte
            sda_oe  = 0;
            in_ack  = 0;
            bit_cnt = 7;
            rx_byte = 0;
            byte_num = byte_num + 1;

            // If read transaction, start driving first bit of read data
            if (is_read && byte_num > 0) begin
                rd_data = 8'h00;
                sda_oe  = ~rd_data[7];
                bit_cnt = 6; // already driving bit 7
            end
        end else if (is_read && byte_num > 0 && !in_ack) begin
            // Continue driving read data bits
            sda_oe  = ~rd_data[bit_cnt];
            if (bit_cnt == 0)
                byte_done = 1; // will check master ACK next
            else
                bit_cnt = bit_cnt - 1;
        end
    end
end

endmodule
