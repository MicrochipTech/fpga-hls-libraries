// This code is an extension of examples/color_conversion/rgb2gray. The extra
// part is running the functions with a 16-bit image.

// TODO T: Convert this into an actual test.
// Right now this code runs both the 8 bit and 16 bit width.
// For the test, we want this code to be configurable e.g. it can run either
// 8-bit or 16-bit based on a parameter.
// We also don't really want to use dg.exp because this code is exposed to the
// users.
// The idea so far (by Dec 16 2022) is to have a bash script that runs a loop
// through different pixel types e.g. 8UC1, 16UC1. But we still need to figure
// out more details about how we want to do this.

// TODO T: Remove the above comments when that is addressed.

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
          vision::StorageType STORAGE_OUT>
void hlsRGB2GRAY(Img<PIXEL_T_IN, H, W, STORAGE_IN, vision::NPPC_1> &InImg,
                 Img<PIXEL_T_OUT, H, W, STORAGE_OUT, vision::NPPC_1> &OutImg) {
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
    Mat BGRInMat_8bit = cv::imread(INPUT_IMAGE, cv::IMREAD_COLOR);

    // By default, OpenCV reads and write image in BGR format, so let's convert
    // to the more traditional RGB format.
    Mat RGBInMat_8bit;
    cv::cvtColor(BGRInMat_8bit, RGBInMat_8bit, cv::COLOR_BGR2RGB);

    /*
     * Call the SmartHLS top-level function and the OpenCV reference function,
     * and compare the 2 results.
     */

    /*************************************************************************/
    /********************************* 8-bit *********************************/
    /*************************************************************************/
    // 1. SmartHLS result
    // First, convert the OpenCV `Mat` into `Img`.
    Img<vision::PixelType::HLS_8UC3, HEIGHT, WIDTH, vision::StorageType::FIFO,
        vision::NPPC_1>
        HlsInImg_8bit(HEIGHT, WIDTH);
    Img<vision::PixelType::HLS_8UC1, HEIGHT, WIDTH, vision::StorageType::FIFO,
        vision::NPPC_1>
        HlsOutImg_8bit(HEIGHT, WIDTH);
    convertFromCvMat(RGBInMat_8bit, HlsInImg_8bit);
    // Then, call the SmartHLS top-level function.
    hlsRGB2GRAY(HlsInImg_8bit, HlsOutImg_8bit);
    // Finally, convert the `Img` back to OpenCV `Mat`.
    Mat HlsOutMat_8bit;
    convertToCvMat(HlsOutImg_8bit, HlsOutMat_8bit);

    // 2. OpenCV result
    Mat CvOutMat_8bit;
    cvRGB2GRAY(RGBInMat_8bit, CvOutMat_8bit);

    // 3. Compare the SmartHLS result and the OpenCV result.
    // Use this commented out line to report location of errors.
    //   vision::compareMatAndReport<unsigned char>(HlsOutMat, CvOutMat, 0);
    float ErrPercent_8bit =
        vision::compareMat(HlsOutMat_8bit, CvOutMat_8bit, 0);
    printf("Percentage of over threshold: %0.2lf%\n", ErrPercent_8bit);

    /*************************************************************************/
    /******************************** 16-bit *********************************/
    /*************************************************************************/
    // Instrument a 16-bit Mat that represents the Toronto image since we don't
    // have a 16-bit image on hand.
    // Start from RGBInMat. To convert from 8UC3 to 16UC3, just use a scale
    // factor of 2^8 = 256.
    Mat RGBInMat_16bit;
    RGBInMat_8bit.convertTo(RGBInMat_16bit, CV_16UC3, 256);
    // 1. SmartHLS result
    // First, convert the OpenCV `Mat` into `Img`.
    Img<vision::PixelType::HLS_16UC3, HEIGHT, WIDTH, vision::StorageType::FIFO,
        vision::NPPC_1>
        HlsInImg_16bit(HEIGHT, WIDTH);
    Img<vision::PixelType::HLS_16UC1, HEIGHT, WIDTH, vision::StorageType::FIFO,
        vision::NPPC_1>
        HlsOutImg_16bit(HEIGHT, WIDTH);
    convertFromCvMat(RGBInMat_16bit, HlsInImg_16bit);
    // Then, call the SmartHLS top-level function.
    hlsRGB2GRAY(HlsInImg_16bit, HlsOutImg_16bit);
    // Finally, convert the `Img` back to OpenCV `Mat`.
    Mat HlsOutMat_16bit;
    convertToCvMat(HlsOutImg_16bit, HlsOutMat_16bit);

    // 2. OpenCV result
    Mat CvOutMat_16bit;
    cvRGB2GRAY(RGBInMat_16bit, CvOutMat_16bit);

    // 3. Compare the SmartHLS result and the OpenCV result.
    // I observed that using SmartHLS function, for some pixel values, they
    // don't match exactly with the OpenCV result. However, the difference is
    // only at most 2 units. This might be explainable because on the scale of
    // 2^16, it's hard to get the exact matching value e.g. the underlying
    // formula that OpenCV uses might not use exactly the coefficients that we
    // use.
    // TODO T: We might want to look into this.

    // Use a diff threshold of 2 here because of the above.
    // Use this commented out line to report location of errors.
    // vision::compareMatAndReport<uint16_t>(HlsOutMat_16bit, CvOutMat_16bit, 2);
    float ErrPercent_16bit =
        vision::compareMat(HlsOutMat_16bit, CvOutMat_16bit, 2);
    printf("Percentage of over threshold: %0.2lf%\n", ErrPercent_16bit);

    /*************************************************************************/
    /********************* Print images to sanity check **********************/
    /*************************************************************************/
    // The original Toronto image is 8-bit, so the result of the imread should
    // also be 8-bit
    cv::imwrite("input.png", BGRInMat_8bit);

    // Output 8-bit
    cv::imwrite("hls_output_8bit.png", HlsOutMat_8bit);
    cv::imwrite("cv_output_8bit.png", CvOutMat_8bit);

    // Output 16-bit
    cv::imwrite("hls_output_16bit.png", HlsOutMat_16bit);
    cv::imwrite("cv_output_16bit.png", CvOutMat_16bit);

    // To sanity check (manually), we can do a couple things:
    // - Check that hls_output_*bit.png looks the same as cv_output_*bit.png
    // - Check the size of the images:
    //   1) *_output_16bit.png should have around twice the file size of
    //      *_output_8bit.png.
    //   2) input.png should have around 3 times the file size of
    //      *_output_8bit.png since it's 8 bit and has 3 channels compared to 1
    //      of the output.

    // Return 0 if both 8 bit and 16 bits have no errors.
    return ErrPercent_8bit + ErrPercent_16bit == 0;
}
