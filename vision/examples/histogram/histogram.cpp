#include <stdio.h>
#include <stdint.h>
#include "vision.hpp"
#include <iostream>

using namespace hls;
using namespace hls::vision;

#define WIDTH 100
#define HEIGHT 56

// Range of pixel values (PixelType::HLS_8UC1). Also the number of bins in the 
// histogram. 
constexpr int HIST_SIZE = 256;  

// Use FRAME_BUFFER for data comming from memory over AXI.  Otherwise use FIFO 
// but also change the pragma intgerface to type(simple).
#define STORAGE_TYPE StorageType::FRAME_BUFFER
// #define STORAGE_TYPE StorageType::FIFO

using ImageT = Img<PixelType::HLS_8UC1, HEIGHT, WIDTH, STORAGE_TYPE, NPPC_1>;

//------------------------------------------------------------------------------
// Create an image to plot the histogram
void plot_histogram(cv::Mat &histMat, std::string filename) {
    int histWidth = 512, histHeight = 400;
    cv::normalize(histMat, histMat, 0, histHeight - 15, cv::NORM_MINMAX);

    cv::Mat histImage(histHeight, histWidth, CV_8UC3, cv::Scalar(255, 255, 255));

    // Plot histogram
    int binWidth = cvRound((double)histWidth / HIST_SIZE);

    // printf("-------[ %s ]-------\n", filename.c_str());
        // printf("%d:%d\n", i, h);
    for (int i = 0; i < HIST_SIZE; ++i) {
        unsigned int h = static_cast<unsigned int>(histMat.at<unsigned int>(i));
        cv::line(
            histImage,
            cv::Point(binWidth * i, histHeight - 15),
            cv::Point(binWidth * i, histHeight - h - 15),
            cv::Scalar(0, 0, 0), 2, 8, 0
        );

        cv::putText(
            histImage, 
            "Pixel Intensity", 
            cv::Point(histWidth/2 - 64, histHeight - 4),
            cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(0, 0, 0), 1
        );
    }
    cv::imwrite(filename, histImage);
}

//------------------------------------------------------------------------------
void histogramWrapper (ImageT &InImg, uint32_t hist[HIST_SIZE]) {
    #pragma HLS function top
    #pragma HLS interface default type(axi_target)
    #pragma HLS interface argument(hist) type(axi_target) dma(false)
    #pragma HLS interface argument(InImg) type(axi_target) dma(false)
    // #pragma HLS interface argument(InImg) type(simple)

    hls::vision::Histogram(InImg, hist);
}

//------------------------------------------------------------------------------
int main(int argc, char* argv[]) {

    if(argc != 2) {
        printf("Usage: %s <input_file.png>\n", argv[0]);
        return -1;
    }
    std::string INPUT_IMAGE=argv[1];

    cv::Mat InMat = cv::imread(INPUT_IMAGE, cv::IMREAD_GRAYSCALE);
    if (InMat.empty()) {
        std::perror("Unable to open input file\n");
        return -1;
    }

    // 
    // With HLS
    //
    ImageT InImg;
    convertFromCvMat(InMat, InImg);
    uint32_t hlsHist[HIST_SIZE] = {0};
    histogramWrapper(InImg, hlsHist);
    cv::Mat histMatHLS = cv::Mat(1, HIST_SIZE, CV_32SC1, hlsHist);
    plot_histogram(histMatHLS, "output_hist_hls.png");

    // 
    // With OpenCV
    //
    int histSize = HIST_SIZE;
    float range[] = {0, HIST_SIZE};  // Range of pixel values
    const float* histRange = {range};
    cv::Mat cvHistMat;
    cv::calcHist(&InMat, 1, 0, cv::Mat(), cvHistMat, 1, &histSize, &histRange);
    cvHistMat = cvHistMat.t(); //transpose
    cvHistMat.convertTo(cvHistMat, CV_32SC1, 255.0); // From float to int
    plot_histogram(cvHistMat, "output_hist_opencv.png");
    
    // 
    // check the errors
    //
    float ErrPercent = compareMat(histMatHLS, cvHistMat, 0);
    printf("Percentage of over threshold: %0.2lf%\n", ErrPercent);
    int error = (ErrPercent != 0.0);
    printf("%s\n", error ? "FAIL" : "PASS");
    return error;
}

