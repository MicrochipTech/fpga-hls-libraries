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

#define NumPixels (WIDTH * HEIGHT)
#define NPPC 4
#define NumPixelWords (NumPixels / NPPC)
#define InPixelWordWidth (8 * NPPC) // Input image is 8UC1
#define InAxiWordWidth 32           // Input AXI memory is 32-bit
#define InNumAxiWords (NumPixelWords * InPixelWordWidth / InAxiWordWidth)
#define OutPixelWordWidth (16 * NPPC) // Output image is 16SC1
#define OutAxiWordWidth 32            // Output AXI memory is 32-bit
#define OutNumAxiWords (NumPixelWords * OutPixelWordWidth / OutAxiWordWidth)

// InAxiMM: AXI-Initiator
// OutAxiMM: AXI-Initiator
// OutDirection: AXI-Stream
template <vision::PixelType PIXEL_T_IN, vision::PixelType PIXEL_T_OUT,
          unsigned H, unsigned W, vision::StorageType STORAGE_IN,
          vision::StorageType STORAGE_OUT, vision::NumPixelsPerCycle _NPPC>
void hlsSobel(uint32_t *InAxiMM, uint32_t *OutAxiMM,
              Img<PIXEL_T_OUT, H, W, STORAGE_OUT, _NPPC> &OutDirection) {
#pragma HLS function top
#pragma HLS function dataflow
#pragma HLS interface argument(InAxiMM) type(axi_initiator)                    \
    num_elements(InNumAxiWords) max_burst_len(256)
#pragma HLS interface argument(OutAxiMM) type(axi_initiator)                   \
    num_elements(OutNumAxiWords) max_burst_len(256)
#pragma HLS memory partition argument(OutDirection) type(struct_fields)

    Img<PIXEL_T_IN, H, W, STORAGE_IN, _NPPC> InImg;
    Img<PIXEL_T_OUT, H, W, STORAGE_OUT, _NPPC> OutImg;

    // 1. AXI Memory to Img conversion
    vision::AxiMM2Img<InAxiWordWidth>(InAxiMM, InImg);
    // 2. Call the processing function i.e. vision::Sobel()
    vision::Sobel<3>(InImg, OutImg, OutDirection);
    // 3. Img to AXI Memory conversion
    vision::Img2AxiMM<OutAxiWordWidth>(OutImg, OutAxiMM);
}

//  Use OpenCV's Sobel as reference.
void cvSobel(cv::Mat &InMat, cv::Mat &OutMat) {
    // Compute x- and y-direction gradiations first.
    Mat CvGx, CvGy;
    cv::Sobel(InMat, CvGx, CV_16S, 1, 0, 3, 1, 0, cv::BORDER_CONSTANT);
    cv::Sobel(InMat, CvGy, CV_16S, 0, 1, 3, 1, 0, cv::BORDER_CONSTANT);

    // Then combine the absolute values of the X/Y gradients.
    Mat CvGxAbs, CvGyAbs;
    CvGxAbs = cv::abs(CvGx);
    CvGyAbs = cv::abs(CvGy);
    cv::add(CvGxAbs, CvGyAbs, OutMat);
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
    // Set up the input and output regions of AXI memory.
    // Dynamic allocation since large image size can cause stack overflow.
    uint32_t *InAxiMM = new uint32_t[InNumAxiWords],
             *OutAxiMM = new uint32_t[OutNumAxiWords];
    // Then write the content of Mat to `InAxiMM`.
    memcpy(InAxiMM, InMat.data, NumPixelWords * InPixelWordWidth / 8);
    // Declare OutDirection which is an Img.
    Img<vision::PixelType::HLS_8UC1, HEIGHT, WIDTH, vision::StorageType::FIFO,
        vision::NPPC_4>
        OutDirection, OutDirectionSw;
    // Now, call the SmartHLS top-level function.
    hlsSobel</*PIXEL_T_IN=*/vision::PixelType::HLS_8UC1,
             /*PIXEL_T_OUT=*/vision::PixelType::HLS_8UC1, /*H=*/HEIGHT,
             /*W=*/WIDTH, /*STORAGE_IN=*/vision::StorageType::FIFO,
             /*STORAGE_OUT=*/vision::StorageType::FIFO,
             /*_NPPC=*/vision::NPPC_4>(InAxiMM, OutAxiMM, OutDirection);
    // Finally, convert the `OutAxiMM` back to OpenCV `Mat`.
    Mat HlsOutMat(HEIGHT, WIDTH, CV_8UC1, OutAxiMM);

    // 2. OpenCV result
    Mat CvOutMat;
    cvSobel(InMat, CvOutMat);
    cv::convertScaleAbs(CvOutMat, CvOutMat);

    // 3. Print the HlsOutMat and CvOutMat as pictures for reference.
    cv::imwrite("hls_output.bmp", HlsOutMat);
    cv::imwrite("cv_output.bmp", CvOutMat);

    // 4. Compare the SmartHLS result and the OpenCV result.
    // Use this commented out line to report location of errors.
    // vision::compareMatAndReport<unsigned char>(HlsOutMat, CvOutMat, 0);
    float ErrPercent = vision::compareMat(HlsOutMat, CvOutMat, 0);
    printf("Percentage of over threshold: %0.2lf%\n", ErrPercent);

    // Clean up
    delete[] InAxiMM;
    delete[] OutAxiMM;

    if (ErrPercent == 0.0) {
        printf("PASS");
        return 0;
    }
    printf("FAIL");
    return 1;
}
