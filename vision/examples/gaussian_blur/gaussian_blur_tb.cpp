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

using ImgT = Img<vision::PixelType::HLS_8UC1, HEIGHT, WIDTH, vision::StorageType::FIFO,
        vision::NPPC_2>;

//
// Top-level wrapper function 
// 
template <vision::PixelType PIXEL_T_IN, vision::PixelType PIXEL_T_OUT,
          unsigned H, unsigned W, vision::StorageType STORAGE_IN,
          vision::StorageType STORAGE_OUT, vision::NumPixelsPerCycle NPPC>
void GaussianBlurWrapper(Img<PIXEL_T_IN, H, W, STORAGE_IN, NPPC> &InImg,
                     Img<PIXEL_T_OUT, H, W, STORAGE_OUT, NPPC> &OutImg) {
    #pragma HLS function top
    const unsigned KERNEL_SIZE = 5;
    vision::GaussianBlur<KERNEL_SIZE>(InImg, OutImg);
}

int main(int argc, char* argv[]) {
    // 
    // Load image from file using OpenCV's imread function and convert ImgT type
    //
    std::string INPUT_IMAGE=argv[1];
    Mat InMat = cv::imread(INPUT_IMAGE, cv::IMREAD_GRAYSCALE);
    ImgT InImg, OutImg;
    convertFromCvMat(InMat, InImg);

    // 
    // Call the SmartHLS top-level function
    // 
    GaussianBlurWrapper(InImg, OutImg);

    // 
    // OpenCV result as golden reference
    // 
    Mat CvOutMat;
    cv::GaussianBlur(InMat, CvOutMat, cv::Size(5, 5), 0, 0, cv::BORDER_CONSTANT);
    cv::convertScaleAbs(CvOutMat, CvOutMat);

    // 
    // Print the HlsOutMat_8UC1 and CvOutMat as pictures for reference.
    // 
    Mat HlsOutMat;
    convertToCvMat(OutImg, HlsOutMat);
    cv::imwrite("hls_output.bmp", HlsOutMat);
    cv::imwrite("cv_output.bmp", CvOutMat);

    // 
    // Compare the SmartHLS result and the OpenCV results.
    // 
    // Use this commented out line to report location of errors.
    //  vision::compareMatAndReport<unsigned char>(HlsOutMat_8UC1, CvOutMat, 0);
    float ErrPercent = vision::compareMat(HlsOutMat, CvOutMat, 0);
    printf("Percentage of pixels over the threshold: %0.2lf%\n", ErrPercent);
        int error = (ErrPercent != 0.0);
    printf("%s\n", error ? "FAILED" : "PASSED");
    return error;
}
