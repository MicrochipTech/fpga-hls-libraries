#include <hls_lib/sev/include/common/common.hpp>
#include <hls_lib/sev/include/common/opencv_utils.hpp>
#include <hls_lib/sev/include/imgproc/sobel.hpp>
#include <opencv2/opencv.hpp>

using namespace hls;
using sev::Img;
using cv::Mat;

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
// calls sev::Sobel.  This is required by our CoSim flow.
template <sev::PixelType PIXEL_T_IN, sev::PixelType PIXEL_T_OUT, unsigned H,
          unsigned W, sev::StorageType STORAGE_IN, sev::StorageType STORAGE_OUT>
void MySobelTop(Img<PIXEL_T_IN, H, W, STORAGE_IN, sev::NPPC_1> &img_in,
                Img<PIXEL_T_OUT, H, W, STORAGE_OUT, sev::NPPC_1> &img_out) {
#pragma HLS function top
    sev::Sobel<3>(img_in, img_out);
}

//  Use OpenCV's Sobel as reference.
void cvSobelReference(cv::Mat &InMat, cv::Mat &OutMat) {
    // Compute X and Y direction gradiations first.
    Mat CvGx, CvGy;
    cv::Sobel(InMat, CvGx, CV_16S, 1, 0, 3, 1, 0, cv::BORDER_CONSTANT);
    cv::Sobel(InMat, CvGy, CV_16S, 0, 1, 3, 1, 0, cv::BORDER_CONSTANT);

    // Then combine the absolute values of the X/Y gradients.
    Mat CvGxAbs, CvGyAbs;
    CvGxAbs = cv::abs(CvGx);
    CvGyAbs = cv::abs(CvGy);
    cv::add(CvGxAbs, CvGyAbs, OutMat);
}

int main() {
    Img<sev::PixelType::SEV_8UC1, HEIGHT, WIDTH, sev::StorageType::FIFO,
        sev::NPPC_1> InImg(HEIGHT, WIDTH);
    Img<sev::PixelType::SEV_16SC1, HEIGHT, WIDTH, sev::StorageType::FIFO,
        sev::NPPC_1> OutImg(HEIGHT, WIDTH);

    // Load image from file, using OpenCV's imread function.
    Mat InMat = cv::imread(INPUT_IMAGE, cv::IMREAD_GRAYSCALE);
    convertFromCvMat(InMat, InImg);

    // Call the top function.
    MySobelTop(InImg, OutImg);

    // Use OpenCV's Sobel function as reference.
    Mat OutMat;
    cvSobelReference(InMat, OutMat);
    cv::convertScaleAbs(OutMat, OutMat);

    cv::imwrite("cv_output.bmp", OutMat);

    Mat HlsMat, HlsMatConverted;
    convertToCvMat(OutImg, HlsMat);
    HlsMat.convertTo(HlsMatConverted, CV_8UC1);
    cv::imwrite("hls_output.bmp", HlsMatConverted);

    // Use this commented out line to report location of errors.
    //   sev::compareMatAndReport<uint16_t>(HlsMatConverted, OutMat, 0);
    float ErrPercent = sev::compareMat(HlsMatConverted, OutMat, 0);
    printf("Percentage of over threshold: %0.2lf%\n", ErrPercent);

    return ErrPercent;
}

