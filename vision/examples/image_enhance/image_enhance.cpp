#include "opencv2/imgproc.hpp"
#include "vision.hpp"

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
    ap_uint<8> b_const,
    ap_uint<8> g_const,
    ap_uint<8> r_const,
    ap_int<10> brightness
) {
    #pragma HLS function top
    vision::ImageEnhance(ImgIn, ImgOut, b_const, g_const, r_const, brightness);
}

int main() {
    Mat BGRInMat = cv::imread("../../media_files/toronto_100x56.bmp", cv::IMREAD_COLOR);
    Mat RGBMat;
    // ImageEnhance uses RGB format, but OpenCV uses BGR to read from files
    cv::cvtColor(BGRInMat, RGBMat, cv::COLOR_BGR2RGB);

    using RGBImgT = Img<PixelType::HLS_8UC3, HEIGHT, WIDTH, StorageType::FIFO, vision::NPPC_1>;
    RGBImgT InImg, OutImg;
    convertFromCvMat(RGBMat, InImg);

    // These numbers are intentionally random to try different factors and mess 
    // around with the original color of the image. 
    ap_uint<8> r_const = 255;   // red factor = 256/32 = 8
    ap_uint<8> g_const = 32;    // green factor = 32/32 = 1
    ap_uint<8> b_const = 1;     // blue factor = 256/32 = 0.031
    ap_int<10> brightness = -60; // reduce the brightness

    ImageEnhanceWrapper(InImg, OutImg, b_const, g_const, r_const, brightness);

    cv::Mat HlsOutMat, HlsOutMatBGR;
    convertToCvMat(OutImg, HlsOutMat);
    // ImageEnhance uses RGB format, but OpenCV uses BGR to write to files
    cv::cvtColor(HlsOutMat, HlsOutMatBGR, cv::COLOR_RGB2BGR);
    cv::imwrite("hls_out.png", HlsOutMatBGR);

    printf("Done\n");
}