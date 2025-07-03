#include <iostream>
#include "vision.hpp"
#include <opencv2/opencv.hpp>

using namespace hls;
using namespace hls::vision;


//=====================================================================
//  Macros
//=====================================================================
#define WIDTH               320
#define HEIGHT              240
#define PIXEL_TYPE          PixelType::HLS_8UC1
#define HIST_SIZE           256
// Use FRAME_BUFFER for data comming from memory over AXI.  Otherwise use FIFO 
// but also change the pragma interface to type(simple).
#define STORAGE_TYPE        StorageType::FIFO
// #define STORAGE_TYPE     StorageType::FRAME_BUFFER

using ImageT = Img<PIXEL_TYPE, HEIGHT, WIDTH, STORAGE_TYPE, NPPC_1>;


//=====================================================================
//  HLS Histogram Equalization Function Definition
//=====================================================================
void equalizedHistogramWrapper(ImageT &InImg, ImageT &OutImg) {
    #pragma HLS function top
    #pragma HLS interface argument(InImg) type(simple)
    #pragma HLS interface argument(OutImg) type(simple)

    // compute equalized histogram without clip limiting
    hls::vision::EqualizedHistogram
    <
        HIST_SIZE,
        PIXEL_TYPE,
        HEIGHT,
        WIDTH,
        STORAGE_TYPE,
        NPPC_1
    >
    (InImg, OutImg);
}

//====================================================================
//  Software Testbench
//====================================================================
int main(int argc, char* argv[]) {

    if(argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file.png>" << std::endl;
        return -1;
    }
    std::string INPUT_IMAGE=argv[1];

    cv::Mat InMat = cv::imread(INPUT_IMAGE, cv::IMREAD_GRAYSCALE);
    if (InMat.empty()) {
        std::cerr << "Unable to open input file" << std::endl;
        return -1;
    }

    // 
    // With HLS
    //
    ImageT InImg;
    cv::Mat hlsMat(HEIGHT, WIDTH, CV_8UC1, cv::Scalar(0));
    convertFromCvMat(InMat, InImg);
    ImageT OutImg;
    equalizedHistogramWrapper(InImg, OutImg);
    convertToCvMat(OutImg, hlsMat);
    cv::imwrite("hls_output.png", hlsMat);

    // 
    // With OpenCV
    //
    cv::Mat cvHistMat(HEIGHT, WIDTH, CV_8UC1, cv::Scalar(0));
    cv::equalizeHist(InMat, cvHistMat);
    cv::imwrite("opencv_output.png", cvHistMat);

    // 
    // check the errors
    //
    float ErrPercent = compareMat(hlsMat, cvHistMat, 0);
    std::cout << "Percentage of over threshold for opencv image :" << ErrPercent  << "%" << std::endl;
    int error = (ErrPercent != 0.0);
    if(error) {
        std::cout << "FAIL\n";
    } else {
        std::cout << "PASS\n";
    }

    return error;
}
