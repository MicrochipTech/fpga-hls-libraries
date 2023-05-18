#include "../../../include/vision.hpp"
#include <opencv2/opencv.hpp>
#include <stdio.h>

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

#define NumPixels (WIDTH * HEIGHT)
#define NPPC 1
#define NumPixelWords (NumPixels / NPPC)
#define InPixelWordWidth (24 * NPPC) // Input image is 8UC3
#define InAxiWordWidth 32            // Input AXI memory is 32-bit
#define InNumAxiWords (NumPixelWords * InPixelWordWidth / InAxiWordWidth)
#define OutPixelWordWidth (8 * NPPC) // Output image is 8UC1
#define OutAxiWordWidth 32           // Output AXI memory is 32-bit
#define OutNumAxiWords (NumPixelWords * OutPixelWordWidth / OutAxiWordWidth)

void hlsRGB2GRAY(uint32_t *InAxiMM, uint32_t *OutAxiMM) {
#pragma HLS function top
#pragma HLS function dataflow

#if defined(AXI_INITIATOR_INTERFACE)
#pragma HLS interface argument(InAxiMM) type(axi_initiator)                    \
    ptr_addr_interface(axi_target) num_elements(InNumAxiWords)                 \
        max_burst_len(256)
#pragma HLS interface argument(OutAxiMM) type(axi_initiator)                   \
    ptr_addr_interface(simple) num_elements(OutNumAxiWords) max_burst_len(256)
#elif defined(AXI_TARGET_INTERFACE)
#pragma HLS interface argument(InAxiMM) type(axi_target)                       \
    num_elements(InNumAxiWords)
#pragma HLS interface argument(OutAxiMM) type(axi_target)                      \
    num_elements(OutNumAxiWords)
#endif

    Img<vision::PixelType::HLS_8UC3, HEIGHT, WIDTH, vision::StorageType::FIFO,
        vision::NPPC_1>
        InImg;
    Img<vision::PixelType::HLS_8UC1, HEIGHT, WIDTH, vision::StorageType::FIFO,
        vision::NPPC_1>
        OutImg;

    // 1. AXI Memory to Img conversion
    vision::AxiMM2Img<InAxiWordWidth>(InAxiMM, InImg);
    // 2. Call the processing function i.e. vision::RGB2GRAY()
    vision::RGB2GRAY(InImg, OutImg);
    // 3. Img to AXI Memory conversion
    vision::Img2AxiMM<OutAxiWordWidth>(OutImg, OutAxiMM);
}

//  Use OpenCV's RGB2GRAY as reference.
void cvRGB2GRAY(cv::Mat &InMat, cv::Mat &OutMat) {
    cv::cvtColor(InMat, OutMat, cv::COLOR_RGB2GRAY);
}

int main() {
#if !defined(AXI_INITIATOR_INTERFACE) && !defined(AXI_TARGET_INTERFACE)
    printf("Error: Please select an interface in the project's Makefile.\n");
    return 1;
#endif
    // Load image from file, using OpenCV's imread function.
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
    // Set up the input and output regions of memory.
    // Dynamic allocation since large image size can cause stack overflow.
    uint32_t *InAxiMM = new uint32_t[InNumAxiWords],
             *OutAxiMM = new uint32_t[OutNumAxiWords];
    // Then write the content of RGBInMat to `InAxiMM`.
    memcpy(InAxiMM, RGBInMat.data, NumPixelWords * InPixelWordWidth / 8);
    // Now, call the SmartHLS top-level function.
    hlsRGB2GRAY(InAxiMM, OutAxiMM);
    // Finally, convert the `OutAxiMM` back to OpenCV `Mat`.
    Mat HlsOutMat(HEIGHT, WIDTH, CV_8UC1, OutAxiMM);

    // 2. OpenCV result
    Mat CvOutMat;
    cvRGB2GRAY(RGBInMat, CvOutMat);

    // 3. Print the HlsOutMat and CvOutMat as pictures for reference.
    cv::imwrite("hls_output.png", HlsOutMat);
    cv::imwrite("cv_output.png", CvOutMat);

    // 4. Compare the SmartHLS result and the OpenCV result.
    // Use this commented out line to report location of errors.
    // vision::compareMatAndReport<unsigned char>(HlsOutMat, CvOutMat, 0);
    float ErrPercent = vision::compareMat(HlsOutMat, CvOutMat, 0);
    printf("Percentage of over threshold: %0.2lf%\n", ErrPercent);

    // Clean up
    delete[] InAxiMM;
    delete[] OutAxiMM;

    return ErrPercent;
}
