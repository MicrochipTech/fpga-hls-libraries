#include "../../include/vision.hpp"
#include <opencv2/opencv.hpp>

using namespace hls;
using cv::Mat;
using vision::Img;
using vision::PixelType;
using vision::StorageType;

// This line tests on a smaller image for faster co-simulation.
#define FAST_COSIM

#ifdef FAST_COSIM
#define WIDTH 100
#define HEIGHT 56
#define INPUT_IMAGE "toronto_100x56.bmp"
#define GOLDEN_OUTPUT "toronto_100x56.bmp"

#else
#define WIDTH 1920
#define HEIGHT 1080
#define INPUT_IMAGE "toronto_1080p.bmp"
#define GOLDEN_OUTPUT "toronto_1080p.bmp"
#endif

#define SIZE (WIDTH * HEIGHT)

// RGB and Bayer image types with 4 Pixels Per Clock (4PPC)
// Using 4 pixels per clock would enable using lower clock frequencies in
// hardware. This is particularly useful when dealing with 4K data, but can help
// lower the required Fmax in other resolutions as well, making hardware
// implementation easier.
using RgbImgT4PPC =
    Img<PixelType::HLS_8UC3, HEIGHT, WIDTH, StorageType::FIFO, vision::NPPC_4>;
using BayerImgT4PPC =
    Img<PixelType::HLS_8UC1, HEIGHT, WIDTH, StorageType::FIFO, vision::NPPC_4>;

template <PixelType PIXEL_T_IN, PixelType PIXEL_T_OUT, unsigned H, unsigned W,
          StorageType STORAGE_IN, StorageType STORAGE_OUT,
          vision::NumPixelsPerCycle NPPC = vision::NPPC_1>
void DeBayerWrapper(vision::Img<PIXEL_T_IN, H, W, STORAGE_IN, NPPC> &ImgIn,
                    vision::Img<PIXEL_T_OUT, H, W, STORAGE_OUT, NPPC> &ImgOut,
                    ap_uint<2> BayerFormat = 0) {
#pragma HLS function top
    vision::DeBayer(ImgIn, ImgOut, BayerFormat);
}

template <PixelType PIXEL_T_IN, PixelType PIXEL_T_OUT, unsigned H, unsigned W,
          StorageType STORAGE = vision::FIFO,
          vision::NumPixelsPerCycle NPPC = vision::NPPC_1>
void RGB2BayerWrapper(vision::Img<PIXEL_T_IN, H, W, STORAGE, NPPC> &ImgIn,
                      vision::Img<PIXEL_T_OUT, H, W, STORAGE, NPPC> &ImgOut) {
#pragma HLS function top
    vision::RGB2Bayer(ImgIn, ImgOut);
}

int main() {
    RgbImgT4PPC InImg, OutImg, DeBayerGoldImg;
    BayerImgT4PPC BayerImg;

    // Read input image into a cv Mat and convert it to RGB format since cv
    // reads images in BGR format
    Mat BGRInMat = cv::imread(INPUT_IMAGE, cv::IMREAD_COLOR);
    Mat RGBInMat;
    cv::cvtColor(BGRInMat, RGBInMat, cv::COLOR_BGR2RGB);

    // Convert the cv Mat into Img class
    convertFromCvMat(RGBInMat, InImg);
    // Process the input, converting RGB to bayer and bayer back to RGB
    RGB2BayerWrapper(InImg, BayerImg);
    DeBayerWrapper(BayerImg, OutImg, 0);

    // Convert the output image to cv Mat for comparing with the input image.
    // As we convert the RGB input image to bayer format and convert back to
    // RGB, the input and output images should be similar
    Mat OutMat;
    vision::convertToCvMat(OutImg, OutMat);

    // Compare the output image with the input image
    // Converting RGB to bayer format results in 3x reduction in data size, and
    // converting back from bayer to RGB (using debayer function) will result in
    // mismatch.
    // So we will say pass as long as less than 5% of pixels (each channel) have
    // mismatch greater than 32 (in range of 0-255).
    float ErrPercent = vision::compareMat(RGBInMat, OutMat, 32);
    printf("Percentage of over threshold: %0.2lf%\n", ErrPercent);
    if (ErrPercent < 5) {
        printf("PASS");
        return 0;
    }
    printf("FAIL");
    return 1;
}
