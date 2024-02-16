#include "vision.hpp"
#include <opencv2/opencv.hpp>

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
// calls vision::GaussianBlur().  This is required by our CoSim flow.
template <vision::PixelType PIXEL_T_IN, vision::PixelType PIXEL_T_OUT,
          unsigned H, unsigned W, vision::StorageType STORAGE_IN,
          vision::StorageType STORAGE_OUT, vision::NumPixelsPerCycle NPPC>
void hlsGaussianBlur(Img<PIXEL_T_IN, H, W, STORAGE_IN, NPPC> &InImg,
                     Img<PIXEL_T_OUT, H, W, STORAGE_OUT, NPPC> &OutImg) {
#pragma HLS function top
    vision::GaussianBlur<5>(InImg, OutImg);
}

//  Use OpenCV's GaussianBlur as reference.
void cvGaussianBlur(cv::Mat &InMat, cv::Mat &OutMat) {
    cv::GaussianBlur(InMat, OutMat, cv::Size(5, 5), 0, 0, cv::BORDER_CONSTANT);
}

int main(int argc, char* argv[]) {
    // Load image from file, using OpenCV's imread function.
    std::string INPUT_IMAGE=argv[1];
    Mat InMat = cv::imread(INPUT_IMAGE, cv::IMREAD_GRAYSCALE);

    /*
     * Call the SmartHLS top-level function and the OpenCV reference function,
     * and compare the 2 results.
     */
    // 1. SmartHLS result
    // First, convert the OpenCV `Mat` into `Img`.
    Img<vision::PixelType::HLS_8UC1, HEIGHT, WIDTH, vision::StorageType::FIFO,
        vision::NPPC_1>
        InImg, OutImg;
    convertFromCvMat(InMat, InImg);
    // Then, call the SmartHLS top-level function.
    hlsGaussianBlur(InImg, OutImg);
    // Finally, convert the `Img` back to OpenCV `Mat`.
    Mat HlsOutMat;
    convertToCvMat(OutImg, HlsOutMat);

    // 2. OpenCV result
    Mat CvOutMat;
    cvGaussianBlur(InMat, CvOutMat);
    cv::convertScaleAbs(CvOutMat, CvOutMat);

    // 3. Print the HlsOutMat_8UC1 and CvOutMat as pictures for reference.
    cv::imwrite("hls_output.bmp", HlsOutMat);
    cv::imwrite("cv_output.bmp", CvOutMat);

    // 4. Compare the SmartHLS result and the OpenCV result.
    // Use this commented out line to report location of errors.
    //  vision::compareMatAndReport<unsigned char>(HlsOutMat_8UC1, CvOutMat, 0);
    float ErrPercent = vision::compareMat(HlsOutMat, CvOutMat, 0);
    printf("Percentage of over threshold: %0.2lf%\n", ErrPercent);
    if (ErrPercent == 0.0) {
        printf("PASS");
        return 0;
    }
    return 1;
}
