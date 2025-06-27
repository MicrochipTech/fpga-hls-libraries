#include "common/opencv_utils.hpp"
#include "common/utils.hpp"
#include "hls_math.hpp"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vision.hpp>

using namespace cv;
using namespace std;
using namespace hls;
using namespace hls::vision;

//=====================================================================
//  Macros
//=====================================================================
#define WIDTH               160
#define HEIGHT              120
#define TILE_SIZE           64
#define PIXEL_TYPE          PixelType::HLS_8UC1

// Use FRAME_BUFFER for data comming from memory over AXI.  Otherwise use FIFO 
// but also change the pragma intgerface to type(simple).
// #define STORAGE_TYPE     StorageType::FRAME_BUFFER
#define STORAGE_TYPE        StorageType::FIFO

using ImageT        = Img<PIXEL_TYPE, HEIGHT, WIDTH, STORAGE_TYPE, NPPC_1>;
using TileImageT    = Img<PIXEL_TYPE, TILE_SIZE, TILE_SIZE, STORAGE_TYPE, NPPC_1>;

//=====================================================================
//  HLS Image Cropper Function Definition
//=====================================================================
template <
    PixelType PIXEL_T,              // Pixel Type
    int H_SRC,                      // Input Image Height
    int W_SRC,                      // Input Image Width
    int H_DST,                      // Output Image Height
    int W_DST,                      // Output Image Width
    StorageType ST,                 // Storage Type
    NumPixelsPerCycle NPPC          // Pixels Per Cycle
>
void CropImgWrapper(
    Img<PIXEL_T, H_SRC, W_SRC, ST, NPPC> &InImg,
    Img<PIXEL_T, H_DST, W_DST, ST, NPPC> &OutImg,
    int rowBound,
    int colBound,
    int srcStart
) {
    #pragma HLS function top
    #pragma HLS memory partition argument(InImg) type(struct_fields)
    #pragma HLS memory partition argument(OutImg) type(struct_fields)

    OutImg.set_height(rowBound);
    OutImg.set_width(colBound);
    hls::vision::CropImg
    <
        PIXEL_T,
        H_SRC,
        W_SRC,
        H_DST,
        W_DST,
        STORAGE_TYPE,
        NPPC
    >
    (InImg, OutImg, rowBound, colBound, srcStart);
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

    //
    // With HLS
    //
    ImageT InImg;
    cv::Mat OutMat;
    convertFromCvMat(InMat, InImg);
    TileImageT OutImg;
    int srcImgStart = 0;

    CropImgWrapper
    <
        PIXEL_TYPE,
        HEIGHT,
        WIDTH,
        TILE_SIZE,
        TILE_SIZE,
        STORAGE_TYPE,
        NPPC_1
    >
    (InImg, OutImg, TILE_SIZE, TILE_SIZE, srcImgStart);
    convertToCvMat(OutImg, OutMat);
    imwrite("hls_output.png", OutMat);

    // 
    // check the errors
    //
    int startRow = srcImgStart / WIDTH;
    int startCol = srcImgStart % WIDTH;
    int error = compareMatRegions(InMat, OutMat, startRow, startCol, 0, 0, TILE_SIZE, TILE_SIZE);
    if(error)
        std::cout << "FAIL" << std::endl;
    else
        std::cout << "PASS" << std::endl;

    return error;
}