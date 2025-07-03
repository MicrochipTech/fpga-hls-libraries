#include <iostream>
#include "common/opencv_utils.hpp"
#include "common/utils.hpp"
#include <opencv2/opencv.hpp>  // OpenCV for golden model verification
#include <vision.hpp>
#include "hls/hls_alloc.h"

using namespace cv;
using namespace std;
using namespace hls;
using namespace hls::vision;

#define SCALE                               2
#define WIDTH                               640
#define HEIGHT                              480
#define PIXEL_TYPE                          PixelType::HLS_8UC1
#define OUTPUT_WIDTH                        static_cast<int>(WIDTH * SCALE)
#define OUTPUT_HEIGHT                       static_cast<int>(HEIGHT * SCALE)
#define NumPixels                           (WIDTH * HEIGHT)
#define NumPixelWords                       (NumPixels / NPPC_1)
#define InPixelWordWidth                    (8 * NPPC_1)      // Input image is 8UC3
#define InAxiWordWidth                      32              // Input AXI memory is 32-bit
#define InNumAxiWords                       (NumPixelWords * InPixelWordWidth / InAxiWordWidth)
#define OutPixelWordWidth                   (8 * NPPC_1)      // Output image is 8UC1
#define OutAxiWordWidth                     32              // Output AXI memory is 32-bit
#define OutNumAxiWords                      (SCALE * SCALE * NumPixelWords * OutPixelWordWidth / OutAxiWordWidth)


// #define STORAGE_TYPE                     StorageType::FRAME_BUFFER
#define STORAGE_TYPE                        StorageType::FIFO

using ImageT            = Img<PIXEL_TYPE, HEIGHT, WIDTH, STORAGE_TYPE, NPPC_1>;
using ScaledImageT      = Img<PIXEL_TYPE, OUTPUT_HEIGHT, OUTPUT_WIDTH, STORAGE_TYPE, NPPC_1>;

//------------------------------------------------------------------------------
void bilinearFilterWrapper(uint32_t *InAxiMM, uint32_t *OutAxiMM) {
    #pragma HLS function top
    #pragma HLS function dataflow
    #pragma HLS interface default type(axi_target)
    #pragma HLS interface argument(InAxiMM) type(axi_initiator)                    \
        num_elements(InNumAxiWords) max_burst_len(256)
    #pragma HLS interface argument(OutAxiMM) type(axi_initiator)                   \
        num_elements(OutNumAxiWords) max_burst_len(256)

    #pragma HLS memory partition variable(InImg) type(struct_fields)
    ImageT InImg;
    #pragma HLS memory partition variable(OutImg) type(struct_fields)
    ScaledImageT OutImg;

    // AXI Memory to Img conversion
    hls::vision::AxiMM2Img
    <InAxiWordWidth>
    (InAxiMM, InImg);
    // Bilinear filtering
    hls::vision::BilinearFilter
    <
        STORAGE_TYPE,
        NPPC_1,
        PIXEL_TYPE,
        OUTPUT_HEIGHT,
        OUTPUT_WIDTH,
        HEIGHT,
        WIDTH
    >
    (InImg, OutImg);
    // Img to AXI Memory conversion
    hls::vision::Img2AxiMM
    <OutAxiWordWidth>
    (OutImg, OutAxiMM);
}

//------------------------------------------------------------------------------
void opencvBilinearFilter(Mat& input, Mat& output) {
    resize(input, output, Size(OUTPUT_WIDTH, OUTPUT_HEIGHT), 0, 0, INTER_LINEAR);
}

//====================================================================
//  Image Quality Evaluation Methods
//====================================================================
int compareMats(const Mat& mat1, const Mat& mat2, int threshold) {
    // Ensure both matrices have the same size and type
    if (mat1.total() != mat2.total() ||
        mat1.size != mat2.size ||
        mat1.type() != mat2.type()
    ) {
        cerr << "Error: Matrices must have the same size and type!" << endl;
        return -1;
    }

    int minDiff = numeric_limits<int>::max();
    int maxDiff = 0;
    int mismatches = 0;

    // **For Grayscale (Single Channel) Mat**
    if (mat1.channels() == 1) {
        for (int i = 0; i < mat1.rows; i++) {
            for (int j = 0; j < mat1.cols; j++) {
                int diff = abs(mat1.at<uint8_t>(i, j) - mat2.at<uint8_t>(i, j));
                if(diff > threshold) {
                    mismatches++;
                }
                minDiff = std::min(minDiff, diff);
                maxDiff = std::max(maxDiff, diff);
            }
        }
    }
    // Unsupported number of channels
    else {
        cerr << "Error: Unsupported " << mat1.channels() << " channels" << endl;
        return -1;
    }

    // Print the Results
    cout << "Min Difference: " << minDiff << endl;
    cout << "Max Difference: " << maxDiff << endl;

    return mismatches;
}

//====================================================================
//  Software Testbench
//====================================================================
int main(int argc, char* argv[]) {

    if(argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file.png>" << std::endl;
        return -1;
    }
    std::string INPUT_IMAGE=argv[1];

    cv::Mat InMat = cv::imread(INPUT_IMAGE, cv::IMREAD_GRAYSCALE);
    if (InMat.empty()) {
        std::cerr << "Unable to open input file" << std::endl;
        return -1;
    }

    // 
    // With HLS
    //
    uint32_t *InAxiMM = (uint32_t *)hls_malloc(sizeof(uint32_t) * InNumAxiWords);
    uint32_t *OutAxiMM = (uint32_t *)hls_malloc(sizeof(uint32_t) * OutNumAxiWords);
 
    // write the content of InMat to `InAxiMM`.
    memcpy(InAxiMM, InMat.data, NumPixelWords * InPixelWordWidth / 8);
    // call the SmartHLS top-level function.
    bilinearFilterWrapper(InAxiMM, OutAxiMM);
    // convert the `OutAxiMM` back to OpenCV `OutMat`.
    Mat OutMat(OUTPUT_HEIGHT, OUTPUT_WIDTH, CV_8UC1, OutAxiMM);
    imwrite("hls_output.png", OutMat);

    // 
    // With OpenCV
    //
    Mat cvOutputMat;
    opencvBilinearFilter(InMat, cvOutputMat);
    imwrite("opencv_output.png", cvOutputMat);

    
    // 
    // Check errors
    //
    double threshold = 0; // difference in pixel value of 0
    float errPercent = compareMats(OutMat, cvOutputMat, threshold);
    const unsigned total_pixels = OUTPUT_WIDTH * OUTPUT_HEIGHT;
    float ErrPercent = errPercent * 100 / total_pixels;
    std::cout << "Percentage of over threshold for opencv image :" << ErrPercent << "%" << std::endl;

    // compute Peak-Signal-to-Noise-Ratio (PSNR)
    double psnr = cv::PSNR(OutMat, cvOutputMat);   // dB   (∞  means identical)
    std::cout << "PSNR : " << psnr << " dB\n";

    // pick a target suitable for CLAHE
    if (psnr < 30.0)
        std::cout << "FAIL\n";
    else
        std::cout << "PASS\n";

    // 
    // free resources
    //
    hls_free(InAxiMM);
    hls_free(OutAxiMM);

    return 0;
}