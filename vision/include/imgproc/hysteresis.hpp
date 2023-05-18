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

#ifndef __SHLS_VISION_HYSTERESIS_HPP__
#define __SHLS_VISION_HYSTERESIS_HPP__

#include <type_traits>

#include "../common/common.hpp"
#include "../common/utils.hpp"

namespace hls {
namespace vision {

// TODO T:
// - Add support for double thresholding algorithm.
// - Add support for other pixel-per-cycle.
// - Add support for clamp logic. Right now we assume [0, 255] for all pixel
//   types which is not correct, but we need to be really careful with the clamp
//   logic since this function can be called both by itself or by Canny().
template <PixelType PIXEL_T_IN, PixelType PIXEL_T_OUT, unsigned H, unsigned W,
          StorageType STORAGE_IN = StorageType::FIFO,
          StorageType STORAGE_OUT = StorageType::FIFO,
          NumPixelsPerCycle NPPC_IN = NPPC_1,
          NumPixelsPerCycle NPPC_OUT = NPPC_1>
void Hysteresis(vision::Img<PIXEL_T_IN, H, W, STORAGE_IN, NPPC_IN> &InImg,
                vision::Img<PIXEL_T_OUT, H, W, STORAGE_OUT, NPPC_OUT> &OutImg,
                unsigned Thres) {
#pragma HLS memory partition argument(InImg) type(struct_fields)
#pragma HLS memory partition argument(OutImg) type(struct_fields)

    static_assert(DT<PIXEL_T_IN, NPPC_IN>::NumChannels == 1 &&
                      DT<PIXEL_T_OUT, NPPC_OUT>::NumChannels == 1,
                  "Hysteresis Thresholding only supports 1-channel images.");
    static_assert(NPPC_IN == NPPC_OUT,
                  "Hysteresis Thresholding only supports having the same "
                  "number of pixels per clock in input and output data.");
    static_assert(W % NPPC_IN == 0,
                  "In Hysteresis, the width of the frame has to be divisible "
                  "by the number of pixels per clock.");
    using InPixelWordT = typename DT<PIXEL_T_IN, NPPC_IN>::T;
    using OutPixelWordT = typename DT<PIXEL_T_OUT, NPPC_OUT>::T;

    const unsigned ImgHeight = InImg.get_height(), ImgWidth = InImg.get_width();
    OutImg.set_height(ImgHeight);
    OutImg.set_width(ImgWidth);

    // For all intermediate values of calculations, let's use an ap_int that has
    // a slightly larger width than max(InPixelWidth, OutPixelWidth)
    const unsigned InPixelWidth = DT<PIXEL_T_IN, NPPC_IN>::W / NPPC_IN,
                   OutPixelWidth = DT<PIXEL_T_OUT, NPPC_OUT>::W / NPPC_OUT;
    const unsigned TmpPixelWidth =
        (InPixelWidth > OutPixelWidth ? InPixelWidth : OutPixelWidth) + 2;
    using TmpPixelT = ap_int<TmpPixelWidth>;

    const unsigned FrameSize = (ImgHeight * ImgWidth) / NPPC_IN;
    // Hysteresis with 1 threshold:
    // - If the pixel is greater than `Thres`, set it to the maximum value.
    // - Otherwise, set it to 0.
    // TODO T: Need to figure out how to do clamp the value e.g. to support
    // signed vs. unsigned, what's the min and max value to clamp.
    // Assume [0, 255] for now.
    // const TmpPixelT OutMaxPixelVal = (TmpPixelT(1) << OutPixelWidth) - 1;
    const TmpPixelT OutMaxPixelVal = 255;
#pragma HLS loop pipeline
    for (unsigned ImgIdx = 0; ImgIdx < FrameSize; ImgIdx++) {
        InPixelWordT InPixelWord = InImg.read(ImgIdx);
        OutPixelWordT OutPixelWord;
        for (int k = 0; k < NPPC_IN; k++) {
            TmpPixelT InPixel = InPixelWord.byte(k, InPixelWidth);
            OutPixelWord.byte(k, OutPixelWidth) =
                (InPixel > TmpPixelT(Thres)) ? OutMaxPixelVal : TmpPixelT(0);
        }
        OutImg.write(OutPixelWord, ImgIdx);
    }
}

} // End of namespace vision.
} // End of namespace hls.

#endif
