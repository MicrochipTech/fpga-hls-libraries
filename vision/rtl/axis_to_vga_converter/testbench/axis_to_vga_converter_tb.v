`timescale 1 ns / 1 ns
 /*
    This module implements a testbehnch for the AXI-Video converter module. It
    uses the pixel generator module to generate images and feed the to the
    AXI-Video converter in AXI 4 stream format. After 2 frames of initial
    alignemnt, AXI-Video converter starts outputting data in video format, which
     is written to .ppm image files.
 */

module AXIS_To_VGA_Converter_tb();

    parameter C_WIDTH = 8; // component (R, G, B) width
    parameter RESOLUTION = 1; // 0 -> 1280x720 1 -> 1920x1080
                              // 2-> 3840x2160 3-> 640x360
    parameter TUSER_WIDTH = 1; // AXI tuser width. tuser 0 is user for Start Of
                               // Frame (SOF). Other bits can be used if needed.
    parameter FIFO_DEPTH = 32;
    parameter CLOCK_PERIOD = 10;

    reg [2:0] pattern_sel;
    reg clk, reset;
    wire tlast, tvalid;
    wire [TUSER_WIDTH-1:0] tuser;
    wire [3*C_WIDTH-1:0] tdata;
    wire tready_O, hsync_O, vsync_O, data_enable_O;
    wire [C_WIDTH-1:0] R_O, G_O, B_O;


    integer j,k, log, c;

    initial clk = 0;

    always @(clk) clk <= #(CLOCK_PERIOD/2) ~clk;

    //reset the module
    initial begin
        reset <= 1;
        for (c=0; c<5; c=c+1)
            @(posedge clk) ;
        reset <= 0;
    end


    initial begin
        pattern_sel <= 0;
        //write the output frame into a ppm image
        $display($sformatf("**printing to output.ppm"));
        log = $fopen ($sformatf("output.ppm"), "w");
    
        $fwrite (log, "P3\n");
        $fwrite (log, "%3d %3d\n", 1920, 1080);
        $fwrite (log, "255\n");
        for (j=0; j<1080; j=j+1) begin
            for (k=0; k<1920; ) begin
                @(posedge clk) ;
                if(data_enable_O == 1)begin
                    $fwrite (log, "\t%1d %1d %1d", R_O, G_O, B_O);
                    k = k+1;
                end
            end
            $fwrite (log, "\n");
        end
        $fclose(log);
        $display($sformatf("**DONE printing output"));
        $finish;

    end


    // axi-video module instantiation
    // active high reset
    AXIS_To_VGA_Converter # (
        .C_WIDTH (C_WIDTH),
        .RESOLUTION (RESOLUTION), // 0 -> 1280x720   1 -> 1920x1080
                                  // 2-> 3840x2160   3-> 640x360
        .TUSER_WIDTH (TUSER_WIDTH),
        .FIFO_DEPTH (FIFO_DEPTH),
        .DEBUBBLE (1)
    ) axis_vga_dut (
        .reset (reset),
        .clk (clk),
        .tlast (tlast),
        .tvalid (tvalid),
        .tuser (tuser),
        .tdata (tdata),
        .tready_O (tready_O),
        .hsync_O (hsync_O),
        .vsync_O (vsync_O),
        .data_enable_O (data_enable_O),
        .R_O (R_O),
        .G_O (G_O),
        .B_O (B_O)
    );


    // pattern generator module instantiation
    // active low reset
    pattern_generator_verilog_pattern # (
        .g_VIDEO_FORMAT (RESOLUTION),
        .g_ADD_BUBBLES (1)
    ) pattern_dut (
        .tready (tready_O),
        .tlast (tlast),
        .tvalid (tvalid),
        .tuser (tuser),
        .tdata (tdata),
        .sys_clk_i (clk),
        .reset_i (~reset),
        .data_en_i (1),
        .pattern_sel_i (pattern_sel)
    );


endmodule
