#include "../../include/vision.hpp"
#include <iostream>
#include <opencv2/opencv.hpp>

using namespace hls;
using cv::Mat;
using vision::Img;

// #define SMALL_TEST_FRAME // for faster simulation.
// #define UHD_TEST_FRAME // for 4K simulation.
#ifdef SMALL_TEST_FRAME
#define WIDTH 100
#define HEIGHT 56
#define INPUT_IMAGE "toronto_100x56.bmp"
#else
#ifdef UHD_TEST_FRAME
#define WIDTH 3840
#define HEIGHT 2160
#define INPUT_IMAGE "ddr_4k_golden.png"
#else
#define WIDTH 1920
#define HEIGHT 1080
#define INPUT_IMAGE "toronto_1080p.bmp"
#endif
#endif

#define NumPixels (WIDTH * HEIGHT)
#define PPC 4
#define NumPixelWords (NumPixels / PPC)
#define PixelWordWidth (8 * PPC) // Image is 8UC1
#define AxiWordWidth 64          // AXI memory is 64-bit
#define NumAxiWords (NumPixelWords * PixelWordWidth / AxiWordWidth)

template <vision::PixelType PIXEL_I_T,
          vision::NumPixelsPerCycle NPPC = vision::NPPC_1>
void DDR_Read_Wrapper(uint64_t *Buf,
                      vision::AxisVideoFIFO<PIXEL_I_T, NPPC> &VideoOut,
                      int HRes, int VRes) {
#pragma HLS function top
#pragma HLS interface argument(Buf) type(axi_initiator)                        \
    num_elements(NumAxiWords) max_burst_len(256)

    vision::AxiMM2AxisVideo<AxiWordWidth, uint64_t, HEIGHT, WIDTH>(
        Buf, VideoOut, HRes, VRes);
}

template <vision::PixelType PIXEL_I_T,
          vision::NumPixelsPerCycle NPPC = vision::NPPC_1>
void DDR_Write_Wrapper(vision::AxisVideoFIFO<PIXEL_I_T, NPPC> &VideoIn,
                       uint64_t *Buf, int HRes, int VRes) {
#pragma HLS function top
#pragma HLS interface argument(Buf) type(axi_initiator)                        \
    num_elements(NumAxiWords) max_burst_len(256)

    vision::AxisVideo2AxiMM<AxiWordWidth, uint64_t, HEIGHT, WIDTH>(VideoIn, Buf,
                                                                   HRes, VRes);
}

int main() {
    // Load image from file, using OpenCV's imread function.
    Mat InMat = cv::imread(INPUT_IMAGE, cv::IMREAD_GRAYSCALE);

    Img<vision::PixelType::HLS_8UC1, HEIGHT, WIDTH, vision::StorageType::FIFO,
        vision::NPPC_4>
        InImg, OutImg;
    vision::AxisVideoFIFO<vision::PixelType::HLS_8UC1, vision::NPPC_4>
        InAxisVideo(NumPixelWords), OutAxisVideo(NumPixelWords);

    // 1. Set up the input.
    // Set up the AXI memory map.
    // Dynamic allocation since large image size can cause stack overflow.
    uint64_t *Buf = new uint64_t[NumAxiWords];
    // Read the input image into `InAxisVideo`
    convertFromCvMat(InMat, InImg);
    vision::Img2AxisVideo(InImg, InAxisVideo);

    // 2. Call the SmartHLS top-level functions.
    // Generate output by doing DDR write and reading the same data back
    // We expect the output data is identical to the input data at the end.
    DDR_Write_Wrapper(InAxisVideo, Buf, WIDTH, HEIGHT);
    DDR_Read_Wrapper(Buf, OutAxisVideo, WIDTH, HEIGHT);

    // 3. Verify the output
    vision::AxisVideo2Img(OutAxisVideo, OutImg);
    // Convert the output image to cv Mat for comparing with the input image.
    Mat OutMat;
    vision::convertToCvMat(OutImg, OutMat);
    // Compare the output with the input, we should read the same data we write
    // to DDR
    float ErrPercent = vision::compareMat(InMat, OutMat, 0);
    printf("Percentage of over threshold: %0.2lf%\n", ErrPercent);
    // Consider the test passes if there is less than 1% of pixels in
    // difference.
    bool Pass = (ErrPercent == 0.0);
    printf("%s\n", Pass ? "PASS" : "FAIL");

    // Clean up
    delete[] Buf;
    return Pass ? 0 : 1; // Only return 0 on pass.
}
