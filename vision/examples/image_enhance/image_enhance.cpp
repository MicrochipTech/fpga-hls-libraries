#include "vision.hpp"
#include <opencv2/opencv.hpp>

using namespace hls;
using cv::Mat;
using vision::Img;
using vision::PixelType;
using vision::StorageType;

#define WIDTH 100
#define HEIGHT 56


template <
    PixelType PIXEL_T, 
    unsigned H,
    unsigned W,
    StorageType STORAGE,
    vision::NumPixelsPerCycle NPPC = vision::NPPC_1>
void ImageEnhanceWrapper(
    vision::Img<PIXEL_T, H, W, STORAGE, NPPC> &ImgIn,
    vision::Img<PIXEL_T, H, W, STORAGE, NPPC> &ImgOut,
    ap_uint<8> b_factor,
    ap_uint<8> g_factor,
    ap_uint<8> r_factor,
    ap_int<10> brightness
) {
    #pragma HLS function top
    vision::ImageEnhance(ImgIn, ImgOut, b_factor, g_factor, r_factor, brightness);
}

int main(int argc, char* argv[]) {

    std::string INPUT_IMAGE=argv[1];

    Mat BGRInMat = cv::imread(INPUT_IMAGE, cv::IMREAD_COLOR);
    if (BGRInMat.empty()) {
        printf("Error loading the file: %s\n", INPUT_IMAGE.c_str());
        exit(0);
    }
    
    Mat RGBMat;
    // ImageEnhance uses RGB format, but OpenCV uses BGR to read from files
    cv::cvtColor(BGRInMat, RGBMat, cv::COLOR_BGR2RGB);

    using RGBImgT = Img<PixelType::HLS_8UC3, HEIGHT, WIDTH, StorageType::FIFO, vision::NPPC_1>;
    RGBImgT InImg, OutImg;
    convertFromCvMat(RGBMat, InImg);

    // These numbers are intentionally random to try different factors and mess 
    // around with the original color of the image. 
    ap_uint<8> r_factor = 1;        // red factor = 256/32 = 8
    ap_uint<8> g_factor = 32;       // green factor = 32/32 = 1
    ap_uint<8> b_factor = 255;      // blue factor = 256/32 = 0.031
    ap_int<10> brightness = -60;    // reduce the brightness

    ImageEnhanceWrapper(InImg, OutImg, b_factor, g_factor, r_factor, brightness);

    cv::Mat HlsOutMat, HlsOutMatBGR;
    convertToCvMat(OutImg, HlsOutMat);
    
    // ImageEnhance uses RGB format, but OpenCV uses BGR to write to files
    cv::cvtColor(HlsOutMat, HlsOutMatBGR, cv::COLOR_RGB2BGR);
    cv::imwrite("hls_out.png", HlsOutMatBGR);

    // Check if the output image is identical to the reference.
    auto similarity = cv::norm(
        cv::imread("hls_out.png"), cv::imread("hls_out_golden.png"), cv::NORM_INF);
    int error = (similarity != 0);
    printf("%s\n", error ? "FAIL" : "PASS");
    return error;
}
