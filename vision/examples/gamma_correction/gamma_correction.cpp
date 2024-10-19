#include "vision.hpp"

using namespace hls;
using vision::Img;
using vision::PixelType;
using vision::StorageType;

#ifdef FAST_COSIM
    #define WIDTH 100
    #define HEIGHT 56
#else
    #define WIDTH 1920
    #define HEIGHT 1080
#endif

#ifndef GAMMA
#define GAMMA 0.5
#endif

using RGBImgT =
    Img<PixelType::HLS_8UC3, HEIGHT, WIDTH, StorageType::FIFO, vision::NPPC_2>;

template <
    PixelType PIXEL_T, 
    unsigned H, 
    unsigned W,
    StorageType STORAGE,
    vision::NumPixelsPerCycle NPPC>
void GammaCorrectionWrapper (
    vision::Img<PIXEL_T, H, W, STORAGE, NPPC> &ImgIn,
    vision::Img<PIXEL_T, H, W, STORAGE, NPPC> &ImgOut
) {
    #pragma HLS function top
    unsigned enable = 1;
    double gamma = GAMMA;
    vision::GammaCorrection(ImgIn, ImgOut, gamma, enable);
}

int main(int argc, char **argv) {
    std::string INPUT_IMAGE=argv[1];
    Mat BGRInMat = cv::imread(INPUT_IMAGE, cv::IMREAD_COLOR);
    if (BGRInMat.empty()) {
        printf("Error: Could not open file: %s\n.", INPUT_IMAGE.c_str());
        return 1;
    }

    // 
    // Use OpenCV to compute the gamma correction as a golden reference
    // Create a look-up table for gamma correction
    //
    double gamma = GAMMA;
    Mat lut(1, 256, CV_8UC1);
    for (int i = 0; i < 256; i++) {
        lut.at<uchar>(i) = cv::saturate_cast<uchar>(pow(i / 255.0, 1.0/gamma) * 255.0);
    }
    Mat corrected_image;
    cv::LUT(BGRInMat, lut, corrected_image);
    cv::imwrite("cv_out.png", corrected_image);

    //
    // Use SmartHLS to compute the Gamma correction
    //
    RGBImgT InImg, OutImg;
    convertFromCvMat(BGRInMat, InImg);
    GammaCorrectionWrapper(InImg, OutImg);

    cv::Mat HlsOutMat;
    convertToCvMat(OutImg, HlsOutMat);
    cv::imwrite("hls_out.png", HlsOutMat);

    //
    // Compare the results
    //
    int threshold = 5;
    float ErrPercent = vision::compareMat(corrected_image, HlsOutMat, threshold);
    printf("Percentage of pixels with differences > %d: %0.2lf%\n", threshold, ErrPercent);
    int error = (ErrPercent > 1);
    printf("%s\n", error ? "FAIL" : "PASS");
    return error;
}