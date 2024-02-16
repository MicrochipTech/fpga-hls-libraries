#include "vision.hpp"
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
using RGBAxisVideoT = vision::AxisVideoFIFO<HLS_8UC3, NPPC_4>;
using BayerImgT =
    vision::Img<HLS_8UC1, HEIGHT, WIDTH, StorageType::FIFO, NPPC_4>;
using RGBImgT = vision::Img<HLS_8UC3, HEIGHT, WIDTH, StorageType::FIFO, NPPC_4>;
using GrayImgT =
    vision::Img<HLS_8UC1, HEIGHT, WIDTH, StorageType::FIFO, NPPC_4>;

// The following 2 top functions are compiled to RTL and are used on board:
void DDR_Write_wrapper(BayerAxisVideoT &VideoIn, uint64_t *Buf, int HRes,
                       int VRes) {
#pragma HLS function top
#pragma HLS interface argument(Buf) type(axi_initiator)                        \
    num_elements(NumAxiWords) max_burst_len(256)

    vision::AxisVideo2AxiMM<AxiWordWidth, uint64_t, HEIGHT, WIDTH>(VideoIn, Buf,
                                                                   HRes, VRes);
}

void VideoPipelineTop(uint64_t *Buf, RGBAxisVideoT &VideoOut,
                      ap_uint<2> BayerFormat = 0) {
#pragma HLS function top
#pragma HLS function dataflow
#pragma HLS interface argument(Buf) type(axi_initiator)                        \
    num_elements(NumAxiWords) max_burst_len(256)
    BayerImgT BayerImg;
    RGBImgT RGBInImg, RGBOutImg;
    GrayImgT GSImg, CannyImg;
    unsigned Thres = 110;

    vision::AxiMM2Img<AxiWordWidth>(Buf, BayerImg);
    vision::DeBayer(BayerImg, RGBInImg, BayerFormat);
    vision::RGB2GRAY(RGBInImg, GSImg);
    vision::Canny(GSImg, CannyImg, Thres);
    vision::GRAY2RGB(CannyImg, RGBOutImg);
    vision::Img2AxisVideo(RGBOutImg, VideoOut);
}

// This main implements a testbench verifying the above top-level functions.
int main() {
    // Step 1: create the input to top-level functions.
    // - Read an RGB image
    // - Convert the image to Bayer format
    // - Then inject the image into an AXI-S video protocol stream.
    // The resulting pixel stream is similar to the input data coming from the
    // camera module on board.
    RGBImgT InImg;
    BayerImgT BayerInImg;
    BayerAxisVideoT InStream(NumPixelWords), DDRReadFIFO(NumPixelWords);
    Mat BGRInMat = cv::imread("../../../media_files/toronto_4k.jpg", cv::IMREAD_COLOR);
    Mat RGBInMat;
    cv::cvtColor(BGRInMat, RGBInMat, cv::COLOR_BGR2RGB);

    // Convert the cv Mat into Img class
    convertFromCvMat(RGBInMat, InImg);
    vision::RGB2Bayer(InImg, BayerInImg);
    vision::Img2AxisVideo(BayerInImg, InStream);

    // Step 2: pass the input to the top-level functions and obtain the output.
    // The InStream is fed to DDR write function, which writes incoming data
    // to DDR.
    // The video pipeline reads data from DDR and does DeBayer, RGB2Gray, Canny,
    // Gray2RGB, and outputs data in RGB format.

    // Intermediate data between DDR writer and video pipeline. You can think of
    // it as the DDR memory. 'static' so we don't get stack overflow.
    static uint64_t Buf[NumPixelWords];
    // Output from
    RGBAxisVideoT OutputStream(NumPixelWords);
    // Call the two top-level functions.
    DDR_Write_wrapper(InStream, Buf, WIDTH, HEIGHT);
    VideoPipelineTop(Buf, OutputStream, 0);
    // Step 3: verify the output data by comparing against an expected image.

    // Convert OutputStream to `vision::Img` type and then to `cv::Mat` type.
    RGBImgT OutImg;
    vision::AxisVideo2Img(OutputStream, OutImg);
    cv::Mat HlsOutMat;
    convertToCvMat(OutImg, HlsOutMat);
    cv::imwrite("hls_out.png", HlsOutMat);

    // Read the golden image
    Mat GoldenMat = cv::imread("../../../media_files/test_images/demo_golden.png", cv::IMREAD_COLOR);
    Mat GoldenRGBMat;
    cv::cvtColor(GoldenMat, GoldenRGBMat, cv::COLOR_BGR2RGB);

    // Now compare the two Mat's, with a threshold of 0 (any pixel with
    // difference in value will be considered as an error).

    // Use this commented out line to report location of errors.
    // vision::compareMatAndReport<cv::Vec3b>(HlsOutMat, GoldenRGBMat, 0);
    float ErrPercent = vision::compareMat(HlsOutMat, GoldenRGBMat, 0);
    bool Pass = (ErrPercent == 0.0);
    printf("%s\n", Pass ? "Pass" : "Fail");
    return Pass ? 0 : 1; // Only return 0 on pass.
}
