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
#define GOLDEN_IMAGE "canny_golden_100x56.png"
#else
#define WIDTH 1920
#define HEIGHT 1080
#define INPUT_IMAGE "toronto_1080p.bmp"
#define GOLDEN_IMAGE "canny_golden_1080p.png"
#endif
#define SIZE (WIDTH * HEIGHT)

// We have to create a top-level function here even though the function simply
// calls vision::Canny().  This is required by our CoSim flow.
template <vision::PixelType PIXEL_T_IN, vision::PixelType PIXEL_T_OUT,
          unsigned H, unsigned W, vision::StorageType STORAGE_IN,
          vision::StorageType STORAGE_OUT, vision::NumPixelsPerCycle NPPC>
void hlsCanny(Img<PIXEL_T_IN, H, W, STORAGE_IN, NPPC> &InImg,
              Img<PIXEL_T_OUT, H, W, STORAGE_OUT, NPPC> &OutImg,
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
        vision::NPPC_4>
        HlsInImg, HlsInImgSW;
    Img<vision::PixelType::HLS_8UC1, HEIGHT, WIDTH, vision::StorageType::FIFO,
        vision::NPPC_4>
        HlsOutImg, HlsOutImgSW;
    convertFromCvMat(InMat, HlsInImg);
    // Then, call the SmartHLS top-level function.
    hlsCanny(HlsInImg, HlsOutImg, Thres);
    // Finally, convert the `Img` back to OpenCV `Mat`.
    Mat HlsOutMat;
    convertToCvMat(HlsOutImg, HlsOutMat);

    // 2. OpenCV result
    Mat CvOutMat;
    cvCanny(InMat, CvOutMat, Thres);

    // 3. Print the HlsOutMat and CvOutMat as pictures for reference.
    cv::imwrite("hls_output.png", HlsOutMat);
    cv::imwrite("cv_output.png", CvOutMat);

    // 4. Compare the SmartHLS result and the OpenCV result.
    // Use this commented out line to report location of errors.

    // Uncomment the line below to print the places where there is difference
    // between images
    // vision::compareMatAndReport<unsigned char>(HlsOutMat, CvOutMat,0);
    float ErrPercentCV = vision::compareMat(HlsOutMat, CvOutMat, 0);

    // Compare the output with golden output
    Mat GoldenMat = cv::imread(GOLDEN_IMAGE, cv::IMREAD_GRAYSCALE);

    // Uncomment the line below to print the places where there is difference
    // between images
    // vision::compareMatAndReport<unsigned char>(HlsOutMat, GoldenMat,0);
    float ErrPercentGolden = vision::compareMat(HlsOutMat, GoldenMat, 0);

    printf("Percentage of over threshold against OpenCV: %0.2lf%\n",
           ErrPercentCV);
    printf("Percentage of over threshold against golden output: %0.2lf%\n",
           ErrPercentGolden);

    // Allow a higher percentage error for SMALL_TEST_FRAME since the total
    // number of pixels is lower.
    bool MatchGolden = (ErrPercentGolden == 0.0);
    bool Pass;
#ifdef SMALL_TEST_FRAME
    Pass = (ErrPercentCV < 10.f) && MatchGolden;
#else
    Pass = (ErrPercentCV < 2.5f) && MatchGolden;
#endif
    printf("%s\n", Pass ? "PASS" : "FAIL");

    return Pass ? 0 : 1; // Only return 0 on pass.
}
