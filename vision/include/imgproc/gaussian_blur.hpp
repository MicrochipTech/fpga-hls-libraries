// ©2024 Microchip Technology Inc. and its subsidiaries
//
// Subject to your compliance with these terms, you may use this Microchip
// software and any derivatives exclusively with Microchip products. You are
// responsible for complying with third party license terms applicable to your
// use of third party software (including open source software) that may
// accompany this Microchip software. SOFTWARE IS “AS IS.” NO WARRANTIES,
// WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY
// IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A
// PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT,
// SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE
// OF ANY KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF
// MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.
// TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP’S TOTAL LIABILITY ON ALL
// CLAIMS LATED TO THE SOFTWARE WILL NOT EXCEED AMOUNT OF FEES, IF ANY, YOU PAID
// DIRECTLY TO MICROCHIP FOR THIS SOFTWARE. MICROCHIP OFFERS NO SUPPORT FOR THE
// SOFTWARE. YOU MAY CONTACT MICROCHIP AT
// https://www.microchip.com/en-us/support-and-training/design-help/client-support-services
// TO INQUIRE ABOUT SUPPORT SERVICES AND APPLICABLE FEES, IF AVAILABLE.

#ifndef __SHLS_VISION_GAUSSIAN_BLUR_HPP__
#define __SHLS_VISION_GAUSSIAN_BLUR_HPP__

#include <type_traits>

#include "../common/common.hpp"
#include "../common/utils.hpp"
#include "convolution_2d.hpp"

namespace hls {
namespace vision {

// KERNEL FUNCTION
constexpr int GaussianBlurKernel(int i, int j, int input) {
    return (i == 0 && j == 0)   ? 1
           : (i == 0 && j == 1) ? 4
           : (i == 0 && j == 2) ? 6
           : (i == 0 && j == 3) ? 4
           : (i == 0 && j == 4) ? 1
           : (i == 1 && j == 0) ? 4
           : (i == 1 && j == 1) ? 16
           : (i == 1 && j == 2) ? 24
           : (i == 1 && j == 3) ? 16
           : (i == 1 && j == 4) ? 4
           : (i == 2 && j == 0) ? 6
           : (i == 2 && j == 1) ? 24
           : (i == 2 && j == 2) ? 36
           : (i == 2 && j == 3) ? 24
           : (i == 2 && j == 4) ? 6
           : (i == 3 && j == 0) ? 4
           : (i == 3 && j == 1) ? 16
           : (i == 3 && j == 2) ? 24
           : (i == 3 && j == 3) ? 16
           : (i == 3 && j == 4) ? 4
           : (i == 4 && j == 0) ? 1
           : (i == 4 && j == 1) ? 4
           : (i == 4 && j == 2) ? 6
           : (i == 4 && j == 3) ? 4
           : (i == 4 && j == 4) ? 1
                                : 0;
}

// - Add support for multiple channels.
// - Add support for clamp logic. Right now we assume [0, 255] for all pixel
//   types which is not correct, but we need to be really careful with the clamp
//   logic since this function can be called both by itself or by Canny().
template <unsigned FILTER_SIZE = 5, PixelType PIXEL_T_IN, PixelType PIXEL_T_OUT,
          unsigned H, unsigned W, StorageType STORAGE_IN = StorageType::FIFO,
          StorageType STORAGE_OUT = StorageType::FIFO,
          NumPixelsPerCycle NPPC = NPPC_1>
void GaussianBlur(vision::Img<PIXEL_T_IN, H, W, STORAGE_IN, NPPC> &InImg,
                    vision::Img<PIXEL_T_OUT, H, W, STORAGE_OUT, NPPC> &OutImg) {
    #pragma HLS memory partition argument(InImg) type(struct_fields)
    #pragma HLS memory partition argument(OutImg) type(struct_fields)

    static_assert(FILTER_SIZE == 5,
                  "Gaussian Blur only supports filter size of 5.");
    static_assert(DT<PIXEL_T_IN, NPPC>::NumChannels == 1 &&
                      DT<PIXEL_T_OUT, NPPC>::NumChannels == 1,
                  "Gaussian Blur only supports 1-channel images.");
    static_assert(W % NPPC == 0,
                  "In GaussianBlur, the width of the frame has to be divisible "
                  "by the number of pixels per clock.");

    vision::Convolution_2d<FILTER_SIZE>(InImg, OutImg, GaussianBlurKernel, 0);
}

} // End of namespace vision.
} // End of namespace hls.

#endif
