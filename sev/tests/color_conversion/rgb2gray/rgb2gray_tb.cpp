// This code is an extension of examples/color_conversion/rgb2gray. The extra
// part is running the functions with a 16-bit image.

// TODO Tue: Convert this into an actual test.
// Right now this code runs both the 8 bit and 16 bit width.
// For the test, we want this code to be configurable e.g. it can run either
// 8-bit or 16-bit based on a parameter.
// We also don't really want to use dg.exp because this code is exposed to the
// users.
// The idea so far (by Dec 16 2022) is to have a bash script that runs a loop
// through different pixel types e.g. 8UC1, 16UC1. But we still need to figure
// out more details about how we want to do this.

// TODO Tue: Remove the above comments when that is addressed.

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
    Mat BGRInMat_8bit = cv::imread(INPUT_IMAGE, cv::IMREAD_COLOR);

    // By default, OpenCV reads and write image in BGR format, so let's convert
    // to the more traditional RGB format.
    Mat RGBInMat_8bit;
    cv::cvtColor(BGRInMat_8bit, RGBInMat_8bit, cv::COLOR_BGR2RGB);

    /*
     * Now call SmartHLS SEV function and the OpenCV reference function, and
     * compare the 2 results.
     */

    /*************************************************************************/
    /********************************* 8-bit *********************************/
    /*************************************************************************/
    // 1. SmartHLS SEV result
    // First, convert the OpenCV Mat into the SEV format `Img`.
    Img<sev::PixelType::SEV_8UC3, HEIGHT, WIDTH, sev::StorageType::FIFO,
        sev::NPPC_1> sevInImg_8bit(HEIGHT, WIDTH);
    Img<sev::PixelType::SEV_8UC1, HEIGHT, WIDTH, sev::StorageType::FIFO,
        sev::NPPC_1> sevOutImg_8bit(HEIGHT, WIDTH);
    convertFromCvMat(RGBInMat_8bit, sevInImg_8bit);
    // Then, call the SmartHLS SEV function
    sevRGB2GRAY(sevInImg_8bit, sevOutImg_8bit);
    // Finally, convert the SEV `Img` type back to the OpenCV `Mat` type.
    Mat sevOutMat_8bit;
    convertToCvMat(sevOutImg_8bit, sevOutMat_8bit);

    // 2. OpenCV result
    Mat cvOutMat_8bit;
    cvRGB2GRAY(RGBInMat_8bit, cvOutMat_8bit);

    // 3. Compare the SEV result and the OpenCV result.
    // Use this commented out line to report location of errors.
    //   sev::compareMatAndReport<uint16_t>(sevOutMat, cvOutMat, 0);
    float ErrPercent_8bit = sev::compareMat(sevOutMat_8bit, cvOutMat_8bit, 0);
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
    // 1. SmartHLS SEV result
    Img<sev::PixelType::SEV_16UC3, HEIGHT, WIDTH, sev::StorageType::FIFO,
        sev::NPPC_1> sevInImg_16bit(HEIGHT, WIDTH);
    Img<sev::PixelType::SEV_16UC1, HEIGHT, WIDTH, sev::StorageType::FIFO,
        sev::NPPC_1> sevOutImg_16bit(HEIGHT, WIDTH);
    convertFromCvMat(RGBInMat_16bit, sevInImg_16bit);
    // Then, call the SmartHLS SEV function
    sevRGB2GRAY(sevInImg_16bit, sevOutImg_16bit);
    // Finally, convert the SEV `Img` type back to the OpenCV `Mat` type.
    Mat sevOutMat_16bit;
    convertToCvMat(sevOutImg_16bit, sevOutMat_16bit);

    // 2. OpenCV result
    Mat cvOutMat_16bit;
    cvRGB2GRAY(RGBInMat_16bit, cvOutMat_16bit);

    // 3. Compare the SEV result and the OpenCV result.
    // I observed that using our SEV function, for some pixel values, they don't
    // match exactly with the OpenCV result. However, the difference is only at
    // most 2 units. This might be explainable because on the scale of 2^16,
    // it's hard to get the exact matching value e.g. the underlying formula
    // that OpenCV uses might not use exactly the coefficients that we use.
    // TODO Tue: We might want to look into this.

    // Use a diff threshold of 2 here because of the above
    // float ErrPercent_16bit =
    //   sev::compareMatAndReport<uint16_t>(sevOutMat_16bit, cvOutMat_16bit, 2);
    float ErrPercent_16bit =
        sev::compareMat(sevOutMat_16bit, cvOutMat_16bit, 2);
    printf("Percentage of over threshold: %0.2lf%\n", ErrPercent_16bit);


    /*************************************************************************/
    /********************* Print images to sanity check **********************/
    /*************************************************************************/
    // The original Toronto image is 8-bit, so the result of the imread should
    // also be 8-bit
    cv::imwrite("input.png", BGRInMat_8bit);

    // Output 8-bit
    cv::imwrite("sev_output_8bit.png", sevOutMat_8bit);
    cv::imwrite("cv_output_8bit.png", cvOutMat_8bit);

    // Output 16-bit
    cv::imwrite("sev_output_16bit.png", sevOutMat_16bit);
    cv::imwrite("cv_output_16bit.png", cvOutMat_16bit);

    // To sanity check (manually), we can do a couple things:
    // - Check that sev_output_*bit.png looks the same as cv_output_*bit.png
    // - Check the size of the images:
    //   1) *_output_16bit.png should have around twice the file size of
    //      *_output_8bit.png.
    //   2) input.png should have around 3 times the file size of
    //      *_output_8bit.png since it's 8 bit and has 3 channels compared to 1
    //      of the output.

    // Return 0 if both 8 bit and 16 bits have no errors.
    return ErrPercent_8bit + ErrPercent_16bit == 0;
}
