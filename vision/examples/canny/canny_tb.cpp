#include "../../include/vision.hpp"
#include <opencv2/opencv.hpp>

using namespace hls;
using namespace std;
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
// calls vision::Canny().  This is required by our CoSim flow.
template <vision::PixelType PIXEL_T_IN, vision::PixelType PIXEL_T_OUT,
          unsigned H, unsigned W, vision::StorageType STORAGE_IN,
          vision::StorageType STORAGE_OUT, vision::NumPixelsPerCycle NPPC_IN,
          vision::NumPixelsPerCycle NPPC_OUT>
void hlsCanny(Img<PIXEL_T_IN, H, W, STORAGE_IN, NPPC_IN> &InImg,
              Img<PIXEL_T_OUT, H, W, STORAGE_OUT, NPPC_OUT> &OutImg,
              unsigned Thres) {
#pragma HLS function top
#pragma HLS function dataflow
    vision::Canny(InImg, OutImg, Thres);
}

void cvCanny(Mat &InMat, Mat &OutMat, double Thres) {
    // cv::Canny() doesn't apply GaussianBlur() so we have to apply it manually
    // beforehand.
    Mat CvGaussianBlurMat;
    cv::GaussianBlur(InMat, CvGaussianBlurMat, cv::Size(5, 5), 0, 0,
                     cv::BORDER_CONSTANT);

    cv::Canny(CvGaussianBlurMat, OutMat, Thres, Thres);
}

int main() {
    // Load image from file, using OpenCV's imread function.
    Mat InMat = cv::imread(INPUT_IMAGE, cv::IMREAD_GRAYSCALE);
    unsigned Thres = 110;

    /*
     * Call the SmartHLS top-level function and the OpenCV reference function,
     * and compare the 2 results.
     */
    // 1. SmartHLS result
    // First, convert the OpenCV `Mat` into `Img`.
    Img<vision::PixelType::HLS_8UC1, HEIGHT, WIDTH, vision::StorageType::FIFO,
        vision::NPPC_1>
        HlsInImg;
    Img<vision::PixelType::HLS_32SC1, HEIGHT, WIDTH, vision::StorageType::FIFO,
        vision::NPPC_1>
        HlsOutImg;
    convertFromCvMat(InMat, HlsInImg);
    // Then, call the SmartHLS top-level function.
    hlsCanny(HlsInImg, HlsOutImg, Thres);
    // Finally, convert the `Img` back to OpenCV `Mat`.
    Mat HlsOutMat, HlsOutMat_8UC1;
    convertToCvMat(HlsOutImg, HlsOutMat);
    HlsOutMat.convertTo(HlsOutMat_8UC1, CV_8UC1);

    // 2. OpenCV result
    Mat CvOutMat;
    cvCanny(InMat, CvOutMat, Thres);

    // 3. Print the HlsOutMat and CvOutMat as pictures for reference.
    cv::imwrite("hls_output.png", HlsOutMat_8UC1);
    cv::imwrite("cv_output.png", CvOutMat);

    // 4. Compare the SmartHLS result and the OpenCV result.
    // Use this commented out line to report location of errors.
    // vision::compareMatAndReport<unsigned char>(HlsOutMat_8UC1, CvOutMat, 0);
    float ErrPercent = vision::compareMat(HlsOutMat_8UC1, CvOutMat, 0);
    printf("Percentage of over threshold: %0.2lf%\n", ErrPercent);

    // Allow a higher percentage error for SMALL_TEST_FRAME since the total
    // number of pixels is lower.
#ifdef SMALL_TEST_FRAME
    return (ErrPercent < 10.f) ? 0 : ErrPercent;
#else
    return (ErrPercent < 2.5f) ? 0 : ErrPercent;
#endif
}
