#include "vision.hpp"
#include <opencv2/core/matx.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <string>

using namespace std;
using namespace cv;
using namespace hls;
using cv::Mat;
using vision::Img;

#ifdef SMALL_TEST_FRAME
#define WIDTH 100
#define HEIGHT 56
#else
#define WIDTH 1920
#define HEIGHT 1080
#endif
#define SIZE (WIDTH * HEIGHT)

// We have to create a top-level function here even though the function simply
// calls vision::RGB2GRAY. This is required by our CoSim flow.
template <vision::PixelType PIXEL_T_IN, vision::PixelType PIXEL_T_OUT,
          unsigned H, unsigned W, vision::StorageType STORAGE_IN,
          vision::StorageType STORAGE_OUT, vision::NumPixelsPerCycle NPPC>
void hlsRGB2GRAY(Img<PIXEL_T_IN, H, W, STORAGE_IN, NPPC> &InImg,
                 Img<PIXEL_T_OUT, H, W, STORAGE_OUT, NPPC> &OutImg) {
#pragma HLS function top
    vision::RGB2GRAY(InImg, OutImg);
}

//  Use OpenCV's RGB2YUV as reference.
void cvRGB2GRAY(cv::Mat &InMat, cv::Mat &OutMat) {
    cv::cvtColor(InMat, OutMat, cv::COLOR_RGB2GRAY);
}

int main(int argc, char* argv[]) {
    // Load image from file, using OpenCV's imread function.
    std::string INPUT_IMAGE = argv[1];
    Mat BGRInMat = cv::imread(INPUT_IMAGE, cv::IMREAD_COLOR);

    // By default, OpenCV reads and write image in BGR format, so let's convert
    // to the more traditional RGB format.
    Mat RGBInMat;
    cv::cvtColor(BGRInMat, RGBInMat, cv::COLOR_BGR2RGB);

    /*
     * Call the SmartHLS top-level function and the OpenCV reference function,
     * and compare the 2 results.
     */
    // 1. SmartHLS result
    // First, convert the OpenCV `Mat` into `Img`.
    Img<vision::PixelType::HLS_8UC3, HEIGHT, WIDTH, vision::StorageType::FIFO,
        vision::NPPC_1>
        HlsInImg;
    Img<vision::PixelType::HLS_8UC1, HEIGHT, WIDTH, vision::StorageType::FIFO,
        vision::NPPC_1>
        HlsOutImg;
    convertFromCvMat(RGBInMat, HlsInImg);
    // Then, call the SmartHLS top-level function.
    hlsRGB2GRAY(HlsInImg, HlsOutImg);
    // Finally, convert the `Img` back to OpenCV `Mat`.
    Mat HlsOutMat;
    convertToCvMat(HlsOutImg, HlsOutMat);

    // 2. OpenCV result
    Mat CvOutMat;
    cvRGB2GRAY(RGBInMat, CvOutMat);

    // 3. Print the HlsOutMat and CvOutMat as pictures for reference.
    cv::imwrite("hls_output.png", HlsOutMat);
    cv::imwrite("cv_output.png", CvOutMat);

    // 4. Compare the SmartHLS result and the OpenCV result.
    // Use this commented out line to report location of errors.
    // vision::compareMatAndReport<unsigned char>(HlsOutMat, CvOutMat, 0);
    float ErrPercentCV = vision::compareMat(HlsOutMat, CvOutMat, 0);
    printf("Percentage of over threshold against OpenCV: %f\n", ErrPercentCV);
    // Compare the result with golden result
    std::string GOLDEN_IMAGE = argv[2];
    Mat Golden = cv::imread(GOLDEN_IMAGE, cv::IMREAD_GRAYSCALE);
    // Use this commented out line to report location of errors.
    // vision::compareMatAndReport<unsigned char>(HlsOutMat, Golden, 0);
    float ErrPercentGolden = vision::compareMat(HlsOutMat, Golden, 0);
    printf("Percentage of over threshold against Golden output: %f\n",
           ErrPercentGolden);
    bool Pass = (ErrPercentCV < 0.001) && (ErrPercentGolden == 0.0);
    printf("%s\n", Pass ? "PASS" : "FAIL");
    return Pass ? 0 : 1;
}
