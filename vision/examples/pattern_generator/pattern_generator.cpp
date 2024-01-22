#include "vision.hpp"
#include <assert.h>
#include <iostream>
#include <opencv2/opencv.hpp>

using namespace hls;
using cv::Mat;
using vision::Img;
using vision::PixelType;
using vision::StorageType;

#ifdef SMALL_TEST_FRAME
#define WIDTH 100
#define HEIGHT 56
#else
#define WIDTH 1920
#define HEIGHT 1080
#endif
#define SIZE (WIDTH * HEIGHT)

// RGB image type with 4 Pixels Per Clock (PPC)
using RgbImgT4PPC =
    Img<PixelType::HLS_8UC3, HEIGHT, WIDTH, StorageType::FIFO, vision::NPPC_4>;

// Pattern generator top function wrapper
template <int format, PixelType PIXEL_T, unsigned H, unsigned W,
          StorageType STORAGE, vision::NumPixelsPerCycle NPPC>
void PatternGeneratorWrapper(
    vision::Img<PIXEL_T, H, W, STORAGE, NPPC> &ImgOut) {
#pragma HLS function top
    vision::PatternGenerator<format>(ImgOut);
}

// Testbench generates uses pattern generator to create an image and
// then compares it with a golden image file.
int main(int argc, char* argv[]) {
    RgbImgT4PPC ImgOut, GoldImg;
    // Pattern generator format (the generated image changes based on the
    // format)
    const int format = 0;
    // Generate output
    PatternGeneratorWrapper<format>(ImgOut);
    // Convert the output to cv::Mat for comparison with the golden output
    Mat OutMat;
    vision::convertToCvMat(ImgOut, OutMat);
    // Read golden image file
    std::string GOLDEN_OUTPUT=argv[1];
    Mat BGRGoldenMat = cv::imread(GOLDEN_OUTPUT, cv::IMREAD_COLOR);
    Mat RGBGoldenMat;
    // Since cv::imread reads the image in BGR format, we convert to RGB
    cv::cvtColor(BGRGoldenMat, RGBGoldenMat, cv::COLOR_BGR2RGB);
    vision::convertFromCvMat(RGBGoldenMat, GoldImg);
    // Compare the output image with the golden image
    float ErrPercent = vision::compareMat(RGBGoldenMat, OutMat, 0);
    printf("Percentage of over threshold: %0.2lf%\n", ErrPercent);
    if (ErrPercent == 0.0) {
        printf("PASS");
        return 0;
    }
    printf("FAIL");
    return 1;
}
