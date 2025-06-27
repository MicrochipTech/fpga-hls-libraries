#include <iostream>
#include "common/opencv_utils.hpp"
#include "common/utils.hpp"
#include <opencv2/opencv.hpp>  // OpenCV for golden model verification
#include <vision.hpp>


using namespace cv;
using namespace std;
using namespace hls;
using namespace hls::vision;


//=====================================================================
//  Macros
//=====================================================================
#define SCALE_X                         0.25
#define SCALE_Y                         0.25
#define WIDTH                           640
#define HEIGHT                          480
#define OUTPUT_WIDTH                    static_cast<int>(WIDTH * SCALE_X)
#define OUTPUT_HEIGHT                   static_cast<int>(HEIGHT * SCALE_Y)
#define PIXEL_TYPE                      vision::PixelType::HLS_8UC1
// Use FRAME_BUFFER for data comming from memory over AXI.
// Otherwise use FIFO but also change the pragma intgerface to type(simple).
// #define STORAGE_TYPE                 vision::StorageType::FRAME_BUFFER
#define STORAGE_TYPE                    vision::StorageType::FIFO


using ImageT            = Img<PIXEL_TYPE, HEIGHT, WIDTH, STORAGE_TYPE, NPPC_1>;
using ScaledImageT      = Img<PIXEL_TYPE, OUTPUT_HEIGHT, OUTPUT_WIDTH, STORAGE_TYPE, NPPC_1>;

//=====================================================================
//  HLS Bilinear Filter Function Definition
//=====================================================================
void bilinearFilterWrapper (ImageT &InImg, ScaledImageT &OutImg) {
    #pragma HLS function top
    #pragma HLS memory partition argument(InImg) type(struct_fields)
    #pragma HLS memory partition argument(OutImg) type(struct_fields)

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
}

//====================================================================
//  OpenCV Reference Method
//====================================================================
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
    Mat OutMat;
    ImageT InImg(HEIGHT, WIDTH);
    ScaledImageT OutImg(OUTPUT_HEIGHT, OUTPUT_WIDTH);
    convertFromCvMat(InMat, InImg);
    bilinearFilterWrapper(InImg, OutImg);
    convertToCvMat(OutImg, OutMat);
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

    return 0;
}