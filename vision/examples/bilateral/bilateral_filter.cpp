#include "common/opencv_utils.hpp"
#include "common/utils.hpp"
#include <iostream>
#include <stdio.h>
#include "hls_math.hpp"
#include <vision.hpp>

using namespace hls;
using namespace hls::vision;

#define FILTER_SIZE 5

constexpr int H=434;
constexpr int W=640;
// constexpr int H=1920;
// constexpr int W=1080;

using ImgT = vision::Img<
        vision::PixelType::HLS_8UC1,
        H, W, 
        vision::StorageType::FIFO, 
        vision::NPPC_1>;

void BilateralFilterWrapper(
    ImgT &ImgIn,
    ImgT &ImgOut,
    const float sigmaColor,
    const float sigmaSpace,
    int border_type
) {
  #pragma HLS function top
  BilateralFilter<FILTER_SIZE>(ImgIn, ImgOut, sigmaColor, sigmaSpace, border_type);
}

//------------------------------------------------------------------------------
int main(int argc, char **argv) {
  Mat src = cv::imread("input_grayscale.png", 0);
  if (!src.data) {
    printf("No image data \n");
    return -1;
  }

  const float sigmaColor = 12.0;
  const float sigmaSpace = 16.0;
  Mat filteredImageOpenCV;
  cv::bilateralFilter(src, filteredImageOpenCV, FILTER_SIZE, sigmaColor, sigmaSpace);
  cv::imwrite("output_opencv.png", filteredImageOpenCV);

  ImgT ImgIn, ImgOut;
  convertFromCvMat(src, ImgIn);
  BilateralFilterWrapper(ImgIn, ImgOut, sigmaColor, sigmaSpace, 0);
  
  Mat dst;
  convertToCvMat(ImgOut, dst);
  cv::imwrite("output_hls.png", dst);

  // double threshold = 12; // pixel value difference 12 = ~5% of 255
  double threshold = 6; // pixel value difference 6 = ~2.3% of 255
  float ErrPercent = vision::compareMat(dst, filteredImageOpenCV, threshold);
  printf("ErrPercent:%f \% (threshold:%d)\n", ErrPercent, (unsigned int)threshold);

  return 0;
}