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

// 
// RGB and Bayer image types with 4 Pixels Per Clock (4PPC)
// Using 4 pixels per clock would enable using lower clock frequencies in
// hardware. This is particularly useful when dealing with 4K data, but can help
// lower the required Fmax in other resolutions as well, making hardware
// implementation easier.
//
using RgbImgT4PPC =
  Img<PixelType::HLS_8UC3, HEIGHT, WIDTH, StorageType::FIFO, vision::NPPC_4>;
using BayerImgT4PPC =
  Img<PixelType::HLS_8UC1, HEIGHT, WIDTH, StorageType::FIFO, vision::NPPC_4>;
using RgbImgT4PPC8b =
  Img<PixelType::HLS_8UC3, HEIGHT, WIDTH, StorageType::FIFO, vision::NPPC_4>;

//------------------------------------------------------------------------------
//  This function converts the input image to Bayer and back.  This is done 
//  mainly to test the DeBayer function, which is what would be most commonly 
//  used ina real design.  
//
//      3 Channel BGR -> 1 Channel Bayer BGGR -> 3 Channel BGR
//
template <
    PixelType PIXEL_T_IN, 
    PixelType PIXEL_T_OUT, 
    unsigned H, 
    unsigned W,
    StorageType STORAGE_IN, 
    StorageType STORAGE_OUT,
    vision::NumPixelsPerCycle NPPC
> 
void BayerDeBayerWrapper (
    vision::Img<PIXEL_T_IN, H, W, STORAGE_IN, NPPC> &InImg,
    vision::Img<PIXEL_T_OUT, H, W, STORAGE_OUT, NPPC> &OutImg,
    vision::BayerFormat Format) {
    #pragma HLS function top dataflow
    BayerImgT4PPC BayerImg;
    vision::BGR2Bayer(InImg, BayerImg, Format);
    vision::DeBayer(BayerImg, OutImg, Format);
}

//------------------------------------------------------------------------------
int main(int argc, char* argv[]) {
    RgbImgT4PPC InImg, OutImg;
    BayerImgT4PPC BayerImg;

    std::string INPUT_IMAGE=argv[1];

    Mat BGRInMat = cv::imread(INPUT_IMAGE, cv::IMREAD_COLOR);
    if (BGRInMat.empty()) {
        printf("Error: Could not open file: %s\n.", INPUT_IMAGE.c_str());
        return 1;
    }

    // 
    // Convert the cvMat to SmartHLS Image and process it
    //
    vision::convertFromCvMat(BGRInMat, InImg);
    BayerDeBayerWrapper(InImg, OutImg, vision::BayerFormat::BGGR);

    //
    // Convert the output image to cv Mat for comparing with the input image.
    // Also, write the result to a file for visual inspection.
    //
    Mat OutMat;
    vision::convertToCvMat(OutImg, OutMat);
    cv::imwrite("hls_out.png", OutMat);

    //
    // Compare SmartHLS debayer output image with the golden reference of the 
    // debayered image. They should completely match.
    //
    std::string GOLDEN_OUTPUT=argv[2];
    Mat BGRGoldenMat = cv::imread(GOLDEN_OUTPUT, cv::IMREAD_COLOR);
    float ErrPercentGolden = vision::compareMat(BGRGoldenMat, OutMat, 0);
    printf("ErrPercentGolden: %0.2lf%\n", ErrPercentGolden);

    //
    // Compare the SmartHLS debayred output image with the input image. 
    // Converting to bayer format results in 3x reduction in data size, and 
    // converting back from bayer (using debayer function) will result in mismatch.
    // So we will say PASS as long as less than 10% of pixels (each channel) have
    // mismatch greater than 32 (in range of 0-255).
    //
    float ErrPercent = vision::compareMat(BGRInMat, OutMat, 32);
    printf("Percentage of over threshold In vs Out: %0.2lf%\n", ErrPercent);

    int error =  (ErrPercent > 10 || ErrPercentGolden != 0.0);
    printf("%s\n", error ? "FAIL" : "PASS");
    return error;
}
