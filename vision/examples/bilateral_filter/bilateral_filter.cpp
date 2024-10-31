#include "common/opencv_utils.hpp"
#include "common/utils.hpp"
#include <iostream>
#include <stdio.h>
#include "hls_math.hpp"
#include <vision.hpp>

using namespace hls;
using namespace hls::vision;

#define FILTER_SIZE 5

#define SIGMA_COLOR 12.0
#define SIGMA_SPACE 16.0

constexpr int H=480;
constexpr int W=640;

using ImgT = vision::Img<
        vision::PixelType::HLS_8UC1,
        H, W, 
        vision::StorageType::FIFO, 
        vision::NPPC_1>;

void BilateralFilterWrapper(
    ImgT &ImgIn,
    ImgT &ImgOut
) {
  #pragma HLS function top
  const float sigmaColor = SIGMA_COLOR;
  const float sigmaSpace = SIGMA_SPACE;
  BilateralFilter<FILTER_SIZE>(ImgIn, ImgOut, sigmaColor, sigmaSpace);
}

//------------------------------------------------------------------------------
int main(int argc, char **argv) {
  Mat src = cv::imread("bilateral-filter-test.png", 0);
  if (!src.data) {
    printf("No image data \n");
    return -1;
  }

  Mat filteredImageOpenCV;
  cv::bilateralFilter(src, filteredImageOpenCV, FILTER_SIZE, SIGMA_COLOR, SIGMA_SPACE);
  cv::imwrite("output_opencv.png", filteredImageOpenCV);

  ImgT ImgIn, ImgOut;
  convertFromCvMat(src, ImgIn);
  BilateralFilterWrapper(ImgIn, ImgOut);
  
  Mat dst;
  convertToCvMat(ImgOut, dst);
  cv::imwrite("output_hls.png", dst);

  double threshold = 5; // difference in pixel value of 5 or more
  float ErrPercent = vision::compareMat(dst, filteredImageOpenCV, threshold);
  printf("ErrPercent:%f \% (threshold:%d)\n", ErrPercent, (unsigned int)threshold);
  int error = (ErrPercent > 0.1);
  printf("%s\n", error ? "FAIL" : "PASS");
  return error;
}