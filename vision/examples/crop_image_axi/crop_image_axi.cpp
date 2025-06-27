#include "common/opencv_utils.hpp"
#include "common/utils.hpp"
#include <opencv2/opencv.hpp>
#include <iostream>
#include "hls_math.hpp"
#include "hls/hls_alloc.h"
#include <vision.hpp>

using namespace cv;
using namespace std;
using namespace hls;
using namespace hls::vision;

//=====================================================================
//  Macros
//=====================================================================
#define WIDTH                               160
#define HEIGHT                              120
#define TILE_SIZE                           64
#define PIXEL_TYPE                          PixelType::HLS_8UC1
#define NumPixels                           (WIDTH * HEIGHT)
#define NumPixelWords                       (NumPixels / NPPC_1)
#define InPixelWordWidth                    (8 * NPPC_1)      // Input image is 8UC3
#define InAxiWordWidth                      32              // Input AXI memory is 32-bit
#define InNumAxiWords                       (NumPixelWords * InPixelWordWidth / InAxiWordWidth)
#define OutPixelWordWidth                   (8 * NPPC_1)      // Output image is 8UC1
#define OutAxiWordWidth                     32              // Output AXI memory is 32-bit
#define OutNumAxiWords                      (NumPixelWords * OutPixelWordWidth / OutAxiWordWidth)

// Use FRAME_BUFFER for data comming from memory over AXI.  Otherwise use FIFO 
// but also change the pragma intgerface to type(simple).
// #define STORAGE_TYPE                     StorageType::FRAME_BUFFER
#define STORAGE_TYPE                        StorageType::FIFO


using ImageT            = Img<PIXEL_TYPE, HEIGHT, WIDTH, STORAGE_TYPE, NPPC_1>;
using TileImageT        = Img<PIXEL_TYPE, TILE_SIZE, TILE_SIZE, STORAGE_TYPE, NPPC_1>;

//=====================================================================
//  HLS Image Cropper Function Definition
//=====================================================================
void cropImgWrapper(uint32_t *InAxiMM, uint32_t *OutAxiMM,
    int rowBound, int colBound, int srcStart
) {
    #pragma HLS function top
    #pragma HLS function dataflow
    #pragma HLS interface default type(axi_target)
    #pragma HLS interface argument(rowBound) type(axi_target)
    #pragma HLS interface argument(colBound) type(axi_target)
    #pragma HLS interface argument(srcStart) type(axi_target)
    #pragma HLS interface argument(InAxiMM) type(axi_initiator)                    \
        num_elements(InNumAxiWords) max_burst_len(256)
    #pragma HLS interface argument(OutAxiMM) type(axi_initiator)                   \
        num_elements(OutNumAxiWords) max_burst_len(256)

    #pragma HLS memory partition variable(InImg) type(struct_fields)
    ImageT InImg;
    #pragma HLS memory partition variable(OutImg) type(struct_fields)
    TileImageT OutImg;

    // AXI Memory to Img conversion
    hls::vision::AxiMM2Img
    <InAxiWordWidth>
    (InAxiMM, InImg);
    // Img to img process
    hls::vision::CropImg
    <
        PIXEL_TYPE,
        HEIGHT,
        WIDTH,
        TILE_SIZE,
        TILE_SIZE,
        STORAGE_TYPE,
        NPPC_1
    >
    (InImg, OutImg, rowBound, colBound, srcStart);
    // Img to AXI Memory conversion
    hls::vision::Img2AxiMM
    <OutAxiWordWidth>
    (OutImg, OutAxiMM);
}

//====================================================================
//  Image Quality Evaluation Methods
//====================================================================
int compareMatRegions(const cv::Mat& mat1, const cv::Mat& mat2, 
    int mat1_startRow, int mat1_startCol, 
    int mat2_startRow, int mat2_startCol, 
    int numCols, int numRows) {

    // Ensure valid bounds for both matrices
    if (mat1_startRow < 0 || mat1_startRow + numRows > mat1.rows ||
        mat1_startCol < 0 || mat1_startCol + numCols > mat1.cols ||
        mat2_startRow < 0 || mat2_startRow + numRows > mat2.rows ||
        mat2_startCol < 0 || mat2_startCol + numCols > mat2.cols) {
        std::cerr << "Error: Comparison region is out of bounds." << std::endl;
        return 1;
    }

    int mismatchCount = 0;
    int maxDiff = 0;

    // Iterate over the specified region in both matrices
    for (int i = 0; i < numRows; i++) {
        for (int j = 0; j < numCols; j++) {
            int val1 = mat1.at<uchar>(mat1_startRow + i, mat1_startCol + j);
            int val2 = mat2.at<uchar>(mat2_startRow + i, mat2_startCol + j);
            int diff = std::abs(val1 - val2);

            if (diff > 0) {
                mismatchCount++;
                maxDiff = std::max(maxDiff, diff);
            }
        }
    }

    // Output results
    std::cout << "Total mismatches: " << mismatchCount << "\n";
    std::cout << "Maximum pixel difference: " << maxDiff << "\n";

    return mismatchCount;
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

    uint32_t *InAxiMM = (uint32_t *)hls_malloc(sizeof(uint32_t) * InNumAxiWords);
    uint32_t *OutAxiMM = (uint32_t *)hls_malloc(sizeof(uint32_t) * OutNumAxiWords);
    int srcImgStart = 0;
    // write the content of InMat to `InAxiMM`.
    memcpy(InAxiMM, InMat.data, NumPixelWords * InPixelWordWidth / 8);
    // call the SmartHLS top-level function.
    cropImgWrapper(InAxiMM, OutAxiMM, TILE_SIZE, TILE_SIZE, srcImgStart);
    // convert the `OutAxiMM` back to OpenCV `OutMat`.
    Mat OutMat(TILE_SIZE, TILE_SIZE, CV_8UC1, OutAxiMM);
    imwrite("hls_output.png", OutMat);

    // 
    // free resources
    //
    hls_free(InAxiMM);
    hls_free(OutAxiMM);

    // 
    // check the errors
    //
    int startRow = srcImgStart / WIDTH;
    int startCol = srcImgStart % WIDTH;
    int error = compareMatRegions(InMat, OutMat, startRow, startCol, 0, 0, TILE_SIZE, TILE_SIZE);
    if (error)
        std::cout << "FAIL\n";
    else
        std::cout << "PASS\n";

    return error;
}