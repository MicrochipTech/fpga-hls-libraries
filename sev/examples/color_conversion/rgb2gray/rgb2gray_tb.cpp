#include <hls_lib/sev/include/common/common.hpp>
#include <hls_lib/sev/include/common/opencv_utils.hpp>
#include <hls_lib/sev/include/imgproc/format_conversions.hpp>
#include <opencv2/core/matx.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <string>

using namespace std;
using namespace cv;
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
// calls sev::RGB2GRAY. This is required by our CoSim flow.
template <sev::PixelType PIXEL_T_IN, sev::PixelType PIXEL_T_OUT, unsigned H,
          unsigned W, sev::StorageType STORAGE_IN, sev::StorageType STORAGE_OUT>
void sevRGB2GRAY(Img<PIXEL_T_IN, H, W, STORAGE_IN, sev::NPPC_1> &img_in,
                 Img<PIXEL_T_OUT, H, W, STORAGE_OUT, sev::NPPC_1> &img_out) {
#pragma HLS function top
    sev::RGB2GRAY(img_in, img_out);
}

//  Use OpenCV's RGB2YUV as reference.
void cvRGB2GRAY(cv::Mat &InMat, cv::Mat &OutMat) {
    cv::cvtColor(InMat, OutMat, cv::COLOR_RGB2GRAY);
}

int main() {
    // Load image from file, using OpenCV's imread function.
    Mat BGRInMat = cv::imread(INPUT_IMAGE, cv::IMREAD_COLOR);

    // By default, OpenCV reads and write image in BGR format, so let's convert
    // to the more traditional RGB format.
    Mat RGBInMat;
    cv::cvtColor(BGRInMat, RGBInMat, cv::COLOR_BGR2RGB);

    /*
     * Now call SmartHLS SEV function and the OpenCV reference function, and
     * compare the 2 results.
     */
    // 1. SmartHLS SEV result
    // First, convert the OpenCV Mat into the SEV format `Img`.
    Img<sev::PixelType::SEV_8UC3, HEIGHT, WIDTH, sev::StorageType::FIFO,
        sev::NPPC_1> sevInImg(HEIGHT, WIDTH);
    Img<sev::PixelType::SEV_8UC1, HEIGHT, WIDTH, sev::StorageType::FIFO,
        sev::NPPC_1> sevOutImg(HEIGHT, WIDTH);
    convertFromCvMat(RGBInMat, sevInImg);
    // Then, call the SmartHLS SEV function
    sevRGB2GRAY(sevInImg, sevOutImg);
    // Finally, convert the SEV `Img` type back to the OpenCV `Mat` type.
    Mat sevOutMat;
    convertToCvMat(sevOutImg, sevOutMat);

    // 2. OpenCV result
    Mat cvOutMat;
    cvRGB2GRAY(RGBInMat, cvOutMat);

    // 3. Print the sevOutMat and cvOutMat as pictures for reference.
    cv::imwrite("sev_output.png", sevOutMat);
    cv::imwrite("cv_output.png", cvOutMat);

    // 4. Compare the SEV result and the OpenCV result.
    // Use this commented out line to report location of errors.
    //   sev::compareMatAndReport<uint16_t>(sevOutMat, cvOutMat, 0);
    float ErrPercent = sev::compareMat(sevOutMat, cvOutMat, 0);
    printf("Percentage of over threshold: %0.2lf%\n", ErrPercent);

    return ErrPercent;
}
