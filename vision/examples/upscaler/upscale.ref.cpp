#include "hls/utils.h"
#include "vision.hpp"

using namespace hls;
using vision::Img;
using vision::PixelType;
using vision::StorageType;

// constexpr unsigned IN_WIDTH = 100;
// constexpr unsigned IN_HEIGHT = 56;
// constexpr unsigned OUT_WIDTH = 133;
// constexpr unsigned OUT_HEIGHT = 80;

// constexpr unsigned IN_WIDTH = 100;
// constexpr unsigned IN_HEIGHT = 56;
// constexpr unsigned OUT_WIDTH = (IN_WIDTH);
// constexpr unsigned OUT_HEIGHT = (IN_HEIGHT);
// constexpr unsigned OUT_WIDTH = (2*IN_WIDTH);
// constexpr unsigned OUT_HEIGHT = (2*IN_HEIGHT);


// Input dimensions WUXGA
constexpr unsigned IN_WIDTH = 1920;
constexpr unsigned IN_HEIGHT = 1200;

// Output dimensions WQXGA
constexpr unsigned OUT_WIDTH = 2560;
constexpr unsigned OUT_HEIGHT = 1600;

using InputImgT =
    Img<PixelType::HLS_8UC3, IN_HEIGHT, IN_WIDTH, StorageType::FRAME_BUFFER, vision::NPPC_1>;

using OutputImgT =
    Img<PixelType::HLS_8UC3, OUT_HEIGHT, OUT_WIDTH, StorageType::FRAME_BUFFER, vision::NPPC_1>;


template <
    PixelType PIXEL_T, 
    unsigned H_IN,
    unsigned W_IN,
    unsigned H_OUT,
    unsigned W_OUT,
    StorageType STORAGE,
    vision::NumPixelsPerCycle NPPC>
void bicubicWrapper (
    vision::Img<PIXEL_T, H_IN, W_IN, STORAGE, NPPC> &ImgIn,
    vision::Img<PIXEL_T, H_OUT, W_OUT, STORAGE, NPPC> &ImgOut
) {
    #pragma HLS function top

    using PixelWordT = typename vision::DT<PIXEL_T, NPPC>::T;
    const unsigned ChannelWidth = vision::DT<PIXEL_T, NPPC>::PerChannelPixelWidth;

    // Scale factors
    const float scale_x = static_cast<float>(OUT_WIDTH) / IN_WIDTH;
    const float scale_y = static_cast<float>(OUT_HEIGHT) / IN_HEIGHT;
    // hls_dbg_printf("scale_x: %f, scale_y: %f\n", scale_x, scale_y);

    // Bicubic kernel weights
    auto cubic_weight = [](float x, char c) {
        float abs_x = std::abs(x);
        float ret = 0.0f;
        if (abs_x <= 1.0f) {
            ret = 1.0f - 2.0f * abs_x * abs_x + abs_x * abs_x * abs_x;
        } else if (abs_x < 2.0f) {
            ret = 4.0f - 8.0f * abs_x + 5.0f * abs_x * abs_x - abs_x * abs_x * abs_x;
        } else {
            ret = 0.0f;
        }
        // printf("x:%f, dim: %d, ret: %f, kx: %f, ky: %f\n", x, dim, ret, kx, ky);
        // printf("%c: x:%f, ret: %f\n", c, x, ret); std::fflush(stdout);
        return ret;
    };

    // Process each output pixel
    HLS_BICUBIC_LOOP1:
    for (unsigned y = 0; y < OUT_HEIGHT; y++) {
        HLS_BICUBIC_LOOP2:
        for (unsigned x = 0; x < OUT_WIDTH; x++) {
            // Find corresponding input coordinates
            float in_x = x / scale_x;
            float in_y = y / scale_y;
            
            // Get integer and fractional parts
            int ix = static_cast<int>(in_x);
            int iy = static_cast<int>(in_y);

            float sum_r = 0, sum_g = 0, sum_b = 0;
            float weight_sum = 0;

            // Iterate over 4x4 kernel centered around the input pixel
            // hls_dbg_printf("iy: %d, ix: %d, in_x: %f, in_y: %f\n", iy, ix, in_x, in_y);
            HLS_BICUBIC_LOOP3:
            for (int ky = -1; ky <= 2; ky++) {
                HLS_BICUBIC_LOOP4:
                for (int kx = -1; kx <= 2; kx++) {
                    // Clamp sample coordinates to valid image bounds
                    int sample_y = std::min(std::max(iy + ky, 0), static_cast<int>(IN_HEIGHT - 1));
                    int sample_x = std::min(std::max(ix + kx, 0), static_cast<int>(IN_WIDTH - 1));
                    
                    // Calculate distances from current kernel position to target position
                    float dx = in_x - (ix + kx);
                    float dy = in_y - (iy + ky);
                    // printf("dx: %f, dy: %f\n", dx, dy); std::fflush(stdout);

                    // Calculate bicubic weight based on x and y distances
                    float weight = cubic_weight(dx, 'x') * cubic_weight(dy, 'y');
                    // printf("weight: %f\n", weight);

                    // Read input pixel and accumulate weighted RGB values
                    auto pixel = ImgIn.read(sample_x + IN_WIDTH * sample_y);
                    // hls_dbg_printf("p[y=%d,x=%d]:0x%x\n", sample_y, sample_x, pixel.to_uint64());
                    // hls_dbg_printf("0x%x\n", pixel.to_uint64());
                    sum_r += weight * pixel.byte(0,ChannelWidth);  // Red channel
                    sum_g += weight * pixel.byte(1,ChannelWidth);  // Green channel 
                    sum_b += weight * pixel.byte(2,ChannelWidth);  // Blue channel
                    weight_sum += weight;        // Track total weight for normalization
                    hls_dbg_printf("sum_r: %f, sum_g: %f, sum_b: %f, weight: %f, weight_sum: %f\n", sum_r, sum_g, sum_b, weight, weight_sum); std::fflush(stdout);
                }
            }

            // Normalize and write output
            PixelWordT out_pixel;
            out_pixel.byte(0,ChannelWidth) = static_cast<unsigned char>(std::min(std::max(sum_r / weight_sum, 0.0f), 255.0f));
            out_pixel.byte(1,ChannelWidth) = static_cast<unsigned char>(std::min(std::max(sum_g / weight_sum, 0.0f), 255.0f));
            out_pixel.byte(2,ChannelWidth) = static_cast<unsigned char>(std::min(std::max(sum_b / weight_sum, 0.0f), 255.0f));
            ImgOut.write(out_pixel, y * (OUT_WIDTH / NPPC) + x);
            // hls_dbg_printf("OutPixelWord: 0x%x\n", out_pixel.to_uint64());
            hls_dbg_printf("p: 0x%x, w: %f, r: %f, g: %f, b: %f\n", out_pixel.to_uint64(), weight_sum, sum_r, sum_g, sum_b);
        }
    }
}

int main(int argc, char **argv) {
    std::string INPUT_IMAGE=argv[1];
    cv::Mat BGRInMat = cv::imread(INPUT_IMAGE, cv::IMREAD_COLOR);
    if (BGRInMat.empty()) {
        printf("Error: Could not open file: %s\n.", INPUT_IMAGE.c_str());
        return 1;
    }
    printf("Input image: %s opened successfully.\n", INPUT_IMAGE.c_str());

    // 
    // Use OpenCV to perform bicubic interpolation to upscale the image
    //
    cv::Size target_size(OUT_WIDTH, OUT_HEIGHT);
    cv::Mat cvScaledMat;
    cv::resize(BGRInMat, cvScaledMat, target_size, 0, 0, cv::INTER_CUBIC);
    cv::imwrite("cv_out.png", cvScaledMat);

    //
    // Use SmartHLS
    //
    InputImgT InImg;
    OutputImgT OutImg;
    convertFromCvMat(BGRInMat, InImg);
    bicubicWrapper(InImg, OutImg);
    cv::Mat HlsOutMat;
    convertToCvMat(OutImg, HlsOutMat);
    cv::imwrite("hls_out.png", HlsOutMat);

    //
    // Compare the results
    //
    int threshold = 10;
    float ErrPercent = vision::compareMat(HlsOutMat, cvScaledMat, threshold);
    printf("Percentage of pixels with differences > %d: %0.2lf%%\n", threshold, ErrPercent);
    int error = (ErrPercent > 10);
    printf("%s\n", error ? "FAIL" : "PASS");
    return error;
}