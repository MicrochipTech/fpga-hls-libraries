// ©2022 Microchip Technology Inc. and its subsidiaries
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

#ifndef __SHLS_VISION_CANNY_HPP__
#define __SHLS_VISION_CANNY_HPP__

#include <type_traits>

#include "../common/common.hpp"
#include "../common/utils.hpp"

#include "gaussian_blur.hpp"
#include "sobel_direction.hpp"
#include "nonmaximum_suppression.hpp"
#include "hysteresis.hpp"

namespace hls {
namespace vision {

// TODO T: Canny has all current limitations of the sub-blocks (e.g. assume
// [0, 255] pixel range, gaussian size = 5, sobel size = 3).
template <unsigned GAUSSIAN_SIZE = 5, unsigned SOBEL_SIZE = 3,
          vision::PixelType PIXEL_T_IN, vision::PixelType PIXEL_T_OUT,
          unsigned H, unsigned W, StorageType STORAGE_IN = StorageType::FIFO,
          StorageType STORAGE_OUT = StorageType::FIFO,
          NumPixelsPerCycle NPPC_IN = NPPC_1,
          NumPixelsPerCycle NPPC_OUT = NPPC_1>
void Canny(Img<PIXEL_T_IN, H, W, STORAGE_IN, NPPC_IN> &InImg,
           Img<PIXEL_T_OUT, H, W, STORAGE_OUT, NPPC_OUT> &OutImg,
           unsigned Thres) {
#pragma HLS function dataflow

    // 1. Gaussian Blur
    Img<PIXEL_T_OUT, H, W, STORAGE_OUT, NPPC_OUT> GaussianBlurImg;
    vision::GaussianBlur<GAUSSIAN_SIZE>(InImg, GaussianBlurImg);

    // 2. Sobel
    Img<PIXEL_T_OUT, H, W, STORAGE_OUT, NPPC_OUT> SobelImg;
    Img<PIXEL_T_OUT, H, W, STORAGE_OUT, NPPC_OUT> SobelDirection;
    // When SobelDirection is consumed by NonMaximumSuppression, the data is
    // only consumed after the LineBuffer is filled for 1 line + 1 pixel. So it
    // needs to have sufficient FIFO depth for this.
    // TODO T: Fix the line below not working.
    // SobelDirection.set_fifo_depth(InImg.get_width() / NPPC_OUT + 2);
    SobelDirection.set_fifo_depth(W / NPPC_OUT + 2);
    vision::Sobel<SOBEL_SIZE>(GaussianBlurImg, SobelImg, SobelDirection);

    // 3. Non-maximum Suppression
    Img<PIXEL_T_OUT, H, W, STORAGE_OUT, NPPC_OUT> NMSImg;
    vision::NonMaximumSuppression(SobelImg, SobelDirection, NMSImg);

    // 4. Hysteresis Thresholding
    vision::Hysteresis(NMSImg, OutImg, Thres);
}

} // End of namespace vision.
} // End of namespace hls.

#endif
