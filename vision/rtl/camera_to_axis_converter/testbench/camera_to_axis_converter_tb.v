`timescale 1 ps / 1 ps
/*
    This testbench tests the Video_AXIS module and writes the output frame to a
    file
*/

module Camera_To_AXIS_Converter_tb();

    parameter C_WIDTH = 8; // component (R, G, B) width
    parameter RESOLUTION = 1;// 0 -> 1280x720 1 -> 1920x1080 2-> 3840x2160 
			//3-> 640x360
    parameter TUSER_WIDTH = 1; //AXIS tuser width. tuser 0 is user for Start Of
			// Frame (SOF). Other bits can be used if needed.
    parameter FIFO_DEPTH = 32 ;
    parameter CLOCK_PERIOD = 10;
    parameter LINES_TO_SKIP = 0;

    reg clk, reset; 

    //A-V out
    wire [3*C_WIDTH-1:0] tdata_O;
    wire tready_O, tlast_O, tvalid_O;
    wire [TUSER_WIDTH-1:0] tuser_O;
    //A-V in
    wire tlast_I, tvalid_I;
    wire [TUSER_WIDTH-1:0] tuser_I;
    wire [3*C_WIDTH-1:0] tdata_I;
    wire [C_WIDTH-1:0] R_int, G_int, B_int;
    wire tready_I, hsync_int, vsync_int, data_enable_int;

    integer j, k, log, c;
    initial clk = 0;
    always @(clk) clk <= #(CLOCK_PERIOD/2) ~clk;

    //reset the module
    initial begin
        reset <= 1;
        for (c=0; c<5; c=c+1)
            @(posedge clk) ;
        reset <= 0;
    end

    //Store the output frame in a file
    initial begin
        $display($sformatf("**printing to output.ppm"));
        log = $fopen ($sformatf("output.ppm"), "w");
        $fwrite (log, "P3\n");
        $fwrite (log, "%3d %3d\n", 1920, 1080);
        $fwrite (log, "255\n");
        for (j=0; j<1080; j=j+1) begin
            for (k=0; k<1920; ) begin
                @(posedge clk);
                if(tvalid_O == 1)begin
                    $fwrite (log, "\t%1d %1d %1d", tdata_O[C_WIDTH-1:0], 
		tdata_O[2*C_WIDTH-1:C_WIDTH], tdata_O[3*C_WIDTH-1:2*C_WIDTH]);
                    k = k+1;
                end
            end
            $fwrite (log, "\n");
        end
        $fclose(log);
                $display($sformatf("**DONE printing output"));
        $finish;
    end


    //axis-video module instantiation
    //active high reset
    AXIS_To_VGA_Converter # (
    .C_WIDTH(C_WIDTH), 
    .RESOLUTION(RESOLUTION),// 0 -> 1280x720 1 -> 1920x1080 2-> 3840x2160 
			//3-> 640x360
    .TUSER_WIDTH(TUSER_WIDTH),
    .FIFO_DEPTH(FIFO_DEPTH)
    ) axis_vga_dut_in (
    .reset(reset),
    .clk(clk),
    .tlast(tlast_I),
    .tvalid(tvalid_I),
    .tuser(tuser_I),
    .tdata(tdata_I),
    .tready_O(tready_I),
    .hsync_O(hsync_int),
    .vsync_O(vsync_int),
    .data_enable_O(data_enable_int),
    .R_O(R_int),   
    .G_O(G_int),   
    .B_O(B_int)
    );

    
    Camera_To_AXIS_Converter # (
    .C_WIDTH(C_WIDTH*3), 
    .TUSER_WIDTH(TUSER_WIDTH)
    ) camera_axis_dut (
    .i_video_reset(reset),
    .i_video_clk(clk),
    .i_axis_reset(reset),
    .i_axis_clk(clk),
    .o_tlast(tlast_O),
    .o_tvalid(tvalid_O),
    .o_tuser(tuser_O),
    .o_tdata(tdata_O),
    .i_tready(1'b1),
    .i_frame_start(vsync_int),
    .i_data_valid(data_enable_int),
    .i_Data({B_int, G_int, R_int}),
    .i_vres(1920),
    .i_hres(1080)
);

    //pattern generator module instantiation
    //active low reset
    pattern_generator_verilog_pattern #(
        .g_VIDEO_FORMAT(RESOLUTION)
    )pattern_dut(
        .tready(tready_I),
        .tlast(tlast_I),
        .tvalid(tvalid_I),
        .tuser(tuser_I),
        .tdata(tdata_I),
        .sys_clk_i(clk),
        .reset_i(~reset),
        .data_en_i(1'b1),
        .pattern_sel_i(0)
    );



endmodule
