#include "../../../include/vision.hpp"
#include <opencv2/core/matx.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <string>

using namespace std;
using namespace cv;
using namespace hls;
using cv::Mat;
using vision::Img;

// #define SMALL_TEST_FRAME // for faster simulation.
#ifdef SMALL_TEST_FRAME
#define WIDTH 100
#define HEIGHT 56
#define INPUT_IMAGE "toronto_100x56.bmp"
#else
#define WIDTH 1920
#define HEIGHT 1080
#define INPUT_IMAGE "toronto_1080p.bmp"
#endif
#define SIZE (WIDTH * HEIGHT)

// We have to create a top-level function here even though the function simply
// calls vision::RGB2GRAY. This is required by our CoSim flow.
template <vision::PixelType PIXEL_T_IN, vision::PixelType PIXEL_T_OUT,
          unsigned H, unsigned W, vision::StorageType STORAGE_IN,
          vision::StorageType STORAGE_OUT, vision::NumPixelsPerCycle NPPC>
void hlsGRAY2RGB(Img<PIXEL_T_IN, H, W, STORAGE_IN, NPPC> &InImg,
                 Img<PIXEL_T_OUT, H, W, STORAGE_OUT, NPPC> &OutImg) {
#pragma HLS function top
    vision::GRAY2RGB(InImg, OutImg);
}

//  Use OpenCV's RGB2YUV as reference.
void cvGRAY2RGB(cv::Mat &InMat, cv::Mat &OutMat) {
    cv::cvtColor(InMat, OutMat, cv::COLOR_GRAY2RGB);
}

int main() {
    // Load image from file, using OpenCV's imread function.
    Mat BGRInMat = cv::imread(INPUT_IMAGE, cv::IMREAD_COLOR);

    // Convert to grayscale
    Mat GRAYInMat;
    cv::cvtColor(BGRInMat, GRAYInMat, cv::COLOR_BGR2GRAY);

    /*
     * Call the SmartHLS top-level function and the OpenCV reference function,
     * and compare the 2 results.
     */
    // 1. SmartHLS result
    // First, convert the OpenCV `Mat` into `Img`.
    Img<vision::PixelType::HLS_8UC1, HEIGHT, WIDTH, vision::StorageType::FIFO,
        vision::NPPC_1>
        HlsInImg;
    Img<vision::PixelType::HLS_8UC3, HEIGHT, WIDTH, vision::StorageType::FIFO,
        vision::NPPC_1>
        HlsOutImg;
    convertFromCvMat(GRAYInMat, HlsInImg);
    // Then, call the SmartHLS top-level function.
    hlsGRAY2RGB(HlsInImg, HlsOutImg);
    // Finally, convert the `Img` back to OpenCV `Mat`.
    Mat HlsOutMat;
    convertToCvMat(HlsOutImg, HlsOutMat);

    // 2. OpenCV result
    Mat CvOutMat;
    cvGRAY2RGB(GRAYInMat, CvOutMat);

    // 3. Print the HlsOutMat and CvOutMat as pictures for reference.
    cv::imwrite("hls_output.bmp", HlsOutMat);
    cv::imwrite("cv_output.bmp", CvOutMat);

    // 4. Compare the SmartHLS result and the OpenCV result.
    // Use this commented out line to report location of errors.
    //   vision::compareMatAndReport<Vec3b>(HlsOutMat, CvOutMat, 0);
    float ErrPercent = vision::compareMat(HlsOutMat, CvOutMat, 0);
    printf("Percentage of over threshold: %0.2lf%\n", ErrPercent);

    bool Pass = (ErrPercent == 0.0);
    printf("%s\n", Pass ? "PASS" : "FAIL");

    return Pass ? 0 : 1; // Only return 0 on pass.
}
