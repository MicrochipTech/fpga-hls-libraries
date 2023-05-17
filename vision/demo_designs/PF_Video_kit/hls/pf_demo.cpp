#include "../../../include/vision.hpp"
#include <assert.h>
#include <opencv2/opencv.hpp>

using namespace hls;

#define WIDTH 3840
#define HEIGHT 2160

#define NumPixels (WIDTH * HEIGHT)
#define NPPC 4
#define NumPixelWords (NumPixels / NPPC)
#define PixelWordWidth (8 * NPPC) // Image is 8UC1
#define AxiWordWidth 64           // AXI memory is 64-bit
#define NumAxiWords (NumPixelWords * PixelWordWidth / AxiWordWidth)

using vision::NPPC_4;
using vision::StorageType;
using vision::PixelType::HLS_8UC1;
using vision::PixelType::HLS_8UC3;

// Define data types used in the design.
using BayerAxisVideoT = vision::AxisVideoFIFO<HLS_8UC1, NPPC_4>;
using RgbAxisVideoT = vision::AxisVideoFIFO<HLS_8UC3, NPPC_4>;
using BayerImgT =
    vision::Img<HLS_8UC1, HEIGHT, WIDTH, StorageType::FIFO, NPPC_4>;
using RgbImgT = vision::Img<HLS_8UC3, HEIGHT, WIDTH, StorageType::FIFO, NPPC_4>;

// The following 3 top functions are compiled to RTL and are used on board:
void DDR_Read_wrapper(uint64_t *Buf, BayerAxisVideoT &VideoOut, int HRes,
                      int VRes) {
#pragma HLS function top dataflow
#pragma HLS interface argument(Buf) type(axi_initiator)                        \
    num_elements(NumAxiWords) max_burst_len(256)

    vision::AxiMM2AxisVideo<AxiWordWidth, uint64_t, HEIGHT, WIDTH>(
        Buf, VideoOut, HRes, VRes);
}

void DDR_Write_wrapper(BayerAxisVideoT &VideoIn, uint64_t *Buf, int HRes,
                       int VRes) {
#pragma HLS function top dataflow
#pragma HLS interface argument(Buf) type(axi_initiator)                        \
    num_elements(NumAxiWords) max_burst_len(256)

    vision::AxisVideo2AxiMM<AxiWordWidth, uint64_t, HEIGHT, WIDTH>(VideoIn, Buf,
                                                                   HRes, VRes);
}

void VideoPipelineTop(BayerAxisVideoT &VideoIn, RgbAxisVideoT &VideoOut,
                      ap_uint<2> BayerFormat = 0) {
#pragma HLS function top
#pragma HLS function dataflow
    BayerImgT BayerImg;
    RgbImgT RGBImage;
    vision::AxisVideo2Img(VideoIn, BayerImg);
    vision::DeBayer(BayerImg, RGBImage, BayerFormat);
    vision::Img2AxisVideo(RGBImage, VideoOut);
}

// This main implements a testbench verifying the above top-level functions.
int main() {
    // Step 1: create the input to top-level functions.
    // - generate an RGB image using pattern generator
    // - convert the image to Bayer format
    // - then inject the image into an AXI-S video protocol stream.
    // The resulting pixel stream is similar to the input data coming from the
    // camera module on board.
    RgbImgT PatternImg;
    BayerImgT BayerImgIn;
    BayerAxisVideoT InputStream(NumPixelWords), DDRReadFIFO(NumPixelWords);
    vision::PatternGenerator<3>(PatternImg);
    vision::RGB2Bayer(PatternImg, BayerImgIn);
    vision::Img2AxisVideo(BayerImgIn, InputStream);

    // Step 2: pass the input to the top-level functions and obtain the output.
    // The InputStream is fed to DDR write and read functions, which write
    // incoming data to DDR and read it back.
    // The data read from DDR is then forwarded to the video pipeline which does
    // DeBayer on the incoming data and outputs data in RGB format.

    // Intermediate data between DDR writer and DDR reader. You can think of it
    // as the DDR memory. 'static' so we don't get stack overflow.
    static uint64_t Buf[NumPixelWords];
    // Output from
    RgbAxisVideoT OutputStream(NumPixelWords);
    // Call the three top-level funtions.
    DDR_Write_wrapper(InputStream, Buf, WIDTH, HEIGHT);
    DDR_Read_wrapper(Buf, DDRReadFIFO, WIDTH, HEIGHT);
    VideoPipelineTop(DDRReadFIFO, OutputStream, 0);

    // Step 3: verify the output data by comparing against an expected image.

    // Convert OutputStream to `vision::Img` type and then to `cv::Mat` type.
    RgbImgT OutImg;
    vision::AxisVideo2Img(OutputStream, OutImg);
    cv::Mat HlsOutMat;
    convertToCvMat(OutImg, HlsOutMat);

    // Create the golden output Mat with the same pattern as above input image.
    RgbImgT GoldenImg;
    vision::PatternGenerator<3>(GoldenImg);
    cv::Mat GoldenMat;
    convertToCvMat(GoldenImg, GoldenMat);

    // Now compare the two Mat's, with a threshold of 0 (any pixel with
    // difference in value will be considered as an error).
    // Use this commented out line to report location of errors.
    //   vision::compareMatAndReport<cv::Vec3b>(HlsOutMat, GoldenMat, 0);
    float ErrPercent = vision::compareMat(HlsOutMat, GoldenMat, 0);
    printf("Percentage of pixels with difference over threshold: %0.2lf%\n",
           ErrPercent);

    // Consider the test passes if there is less than 1% of pixels in
    // difference.
    bool Pass = (ErrPercent < 1.f);
    printf("%s\n", Pass ? "PASS" : "FAIL");

    return Pass ? 0 : 1; // Only return 0 on pass.
}
