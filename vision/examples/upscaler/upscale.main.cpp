#include "vision.hpp"
#include <cstdio>

using namespace hls;
using vision::Img;
using vision::PixelType;
using vision::StorageType;

// constexpr unsigned IN_WIDTH = 100;
// constexpr unsigned IN_HEIGHT = 56;
// constexpr unsigned OUT_WIDTH = 133;
// constexpr unsigned OUT_HEIGHT = 80;

// constexpr unsigned IN_WIDTH = 12;
// constexpr unsigned IN_HEIGHT = 8;
// constexpr unsigned OUT_WIDTH = (IN_WIDTH);
// constexpr unsigned OUT_HEIGHT = (IN_HEIGHT);
// constexpr unsigned OUT_WIDTH = (2*IN_WIDTH);
// constexpr unsigned OUT_HEIGHT = (2*IN_HEIGHT);


// Input dimensions WUXGA
constexpr unsigned IN_WIDTH = 1920;
constexpr unsigned IN_HEIGHT = 1200;

// Output dimensions WQXGA
constexpr unsigned OUT_WIDTH = 2560;
constexpr unsigned OUT_HEIGHT = 1600;

// constexpr unsigned OUT_WIDTH = IN_WIDTH;
// constexpr unsigned OUT_HEIGHT = IN_HEIGHT;

using InputImgT =
    Img<PixelType::HLS_8UC3, IN_HEIGHT, IN_WIDTH, StorageType::FIFO, vision::NPPC_1>;

using OutputImgT =
    Img<PixelType::HLS_8UC3, OUT_HEIGHT, OUT_WIDTH, StorageType::FIFO, vision::NPPC_1>;


template <
    PixelType PIXEL_T, 
    unsigned H_IN,
    unsigned W_IN,
    unsigned H_OUT,
    unsigned W_OUT,
    StorageType STORAGE,
    vision::NumPixelsPerCycle NPPC>
void bicubicWrapper (
    vision::Img<PIXEL_T, H_IN, W_IN, STORAGE, NPPC> &ImgIn,
    vision::Img<PIXEL_T, H_OUT, W_OUT, STORAGE, NPPC> &ImgOut
) {
    #pragma HLS function top
    vision::BicubicUpscaler(ImgIn, ImgOut);
}

int main(int argc, char **argv) {

    // using fp_t = hls::ap_fixpt<32,16>;
    // fp_t x(1.5);
    // fp_t weight = hls::vision::cubic_weight(x, 'x');
    // printf("weight: %f\n", weight.to_double()); std::fflush(stdout);
    // return 0;

    std::string INPUT_IMAGE=argv[1];
    cv::Mat BGRInMat = cv::imread(INPUT_IMAGE, cv::IMREAD_COLOR);
    if (BGRInMat.empty()) {
        printf("Error: Could not open file: %s\n.", INPUT_IMAGE.c_str());
        return 1;
    }
    printf("Input image: %s opened successfully.\n", INPUT_IMAGE.c_str());

    // 
    // Use OpenCV to perform bicubic interpolation to upscale the image
    //
    cv::Size target_size(OUT_WIDTH, OUT_HEIGHT);
    cv::Mat cvScaledMat;
    cv::resize(BGRInMat, cvScaledMat, target_size, 0, 0, cv::INTER_CUBIC);
    cv::imwrite("cv_out.png", cvScaledMat);

    
    //
    // Use SmartHLS
    //
    InputImgT InImg;
    OutputImgT OutImg;
    convertFromCvMat(BGRInMat, InImg);
    bicubicWrapper(InImg, OutImg);
    cv::Mat HlsOutMat;
    convertToCvMat(OutImg, HlsOutMat);
    cv::imwrite("hls_out.png", HlsOutMat);
    printf("HLS output image written to hls_out.png\n");

    //
    // Compare the results
    //
    int threshold = 10;
    float ErrPercent = vision::compareMat(HlsOutMat, cvScaledMat, threshold);
    printf("Percentage of pixels with differences > %d: %0.2lf%%\n", threshold, ErrPercent);
    int error = (ErrPercent > 10);
    printf("%s\n", error ? "FAIL" : "PASS");
    return error;
}