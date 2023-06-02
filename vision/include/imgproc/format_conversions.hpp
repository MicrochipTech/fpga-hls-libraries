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
// MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE
// FORESEEABLE.  TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP’S TOTAL
// LIABILITY ON ALL CLAIMS LATED TO THE SOFTWARE WILL NOT EXCEED AMOUNT OF
// FEES, IF ANY, YOU PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE. MICROCHIP
// OFFERS NO SUPPORT FOR THE SOFTWARE. YOU MAY CONTACT MICROCHIP AT
// https://www.microchip.com/en-us/support-and-training/design-help/client-support-services
// TO INQUIRE ABOUT SUPPORT SERVICES AND APPLICABLE FEES, IF AVAILABLE.

#ifndef __SHLS_VISION_FORMAT_CONVERSIONS_HPP__
#define __SHLS_VISION_FORMAT_CONVERSIONS_HPP__

#include "../common/common.hpp"
#include "../common/utils.hpp"
#include <hls/ap_fixpt.hpp>
#include <hls/ap_int.hpp>

namespace hls {
namespace vision {

/**
 * Convert vision::Img in RGB format to Grayscale.
 * Template parameters:
 *  - NTSC: if true, we use NTSC formula (0.299 * R + 0.587 * G + 0.114 * B),
 *          otherwise we use (R + G + B) / 3
 */
template <bool NTSC = true, PixelType PIXEL_T_IN, PixelType PIXEL_T_OUT,
          unsigned H, unsigned W, StorageType STORAGE_IN = FIFO,
          StorageType STORAGE_OUT = FIFO, NumPixelsPerCycle NPPC = NPPC_1>
void RGB2GRAY(vision::Img<PIXEL_T_IN, H, W, STORAGE_IN, NPPC> &InImg,
              vision::Img<PIXEL_T_OUT, H, W, STORAGE_OUT, NPPC> &OutImg) {
    /* Assert that the formats of the input and output make sense */
    static_assert(DT<PIXEL_T_IN, NPPC>::NumChannels == 3 &&
                      DT<PIXEL_T_OUT, NPPC>::NumChannels == 1,
                  "The input (RGB) must have 3 channels, and the output (GRAY) "
                  "must have 1 channel");
    // Assert pixel width of in and out are the same e.g. both have to be 8 bit,
    // or both have to be 16 bit, etc.
    static_assert(
        DT<PIXEL_T_IN, NPPC>::PerChannelPixelWidth ==
            DT<PIXEL_T_OUT, NPPC>::PerChannelPixelWidth,
        "The per channel pixel width of the input and output must be the same");
    // TODO T: Assert the sign information as well e.g. 16UC vs 16SC?

    /* Define a functor to transform InImg pixel-by-pixel to get OutImg */
    struct RGB2GRAYFunctor {
        typename DT<PIXEL_T_OUT>::T
        operator()(typename DT<PIXEL_T_IN>::T in) const {
            using fixpt_t = ap_ufixpt<18, 10>;
            const unsigned PerChannelPixelWidth =
                DT<PIXEL_T_IN, NPPC>::PerChannelPixelWidth;
            auto r = in.byte(0, PerChannelPixelWidth),
                 g = in.byte(1, PerChannelPixelWidth),
                 b = in.byte(2, PerChannelPixelWidth);

            ap_ufixpt<PerChannelPixelWidth, PerChannelPixelWidth> r_fixpt = r,
                                                                  g_fixpt = g,
                                                                  b_fixpt = b;

            if (!NTSC)
                return (r_fixpt + g_fixpt + b_fixpt) / 3 + fixpt_t(0.5);

            // Use NTSC formula: (0.299 * R + 0.587 * G + 0.114 * B)
            // To not lose precision, for all multiply operations, we'll
            // multiply by coefficients that's 256 times larger, then shift
            // right by 8 to divide by 256.
            auto sum = fixpt_t(76.544) * r_fixpt + fixpt_t(150.272) * g_fixpt +
                       fixpt_t(29.184) * b_fixpt;
            return (sum >> 8) + fixpt_t(0.5);
        }
    };

    TransformPixel(InImg, OutImg, RGB2GRAYFunctor());
}

/**
 * Converts vision::Img in Grayscale format to RGB.
 * Note that the resulting image is still visually Grayscale but represented in
 * RGB format.
 */
template <PixelType PIXEL_T_IN, PixelType PIXEL_T_OUT, unsigned H, unsigned W,
          StorageType STORAGE_IN = FIFO, StorageType STORAGE_OUT = FIFO,
          NumPixelsPerCycle NPPC = NPPC_1>
void GRAY2RGB(vision::Img<PIXEL_T_IN, H, W, STORAGE_IN, NPPC> &InImg,
              vision::Img<PIXEL_T_OUT, H, W, STORAGE_OUT, NPPC> &OutImg) {
    /* Assert that the formats of the input and output make sense */
    static_assert(DT<PIXEL_T_IN, NPPC>::NumChannels == 1 &&
                      DT<PIXEL_T_OUT, NPPC>::NumChannels == 3,
                  "The input (GRAY) must have 1 channel, and the output (RGB) "
                  "must have 3 channels");
    // Assert pixel width of in and out are the same e.g. both have to be 8 bit,
    // or both have to be 16 bit, etc.
    static_assert(
        DT<PIXEL_T_IN, NPPC>::PerChannelPixelWidth ==
            DT<PIXEL_T_OUT, NPPC>::PerChannelPixelWidth,
        "The per channel pixel width of the input and output must be the same");
    // TODO T: Assert the sign information as well e.g. 16UC vs 16SC?

    /* Define a functor to transform InImg pixel-by-pixel to get OutImg */
    struct GRAY2RGBFunctor {
        typename DT<PIXEL_T_OUT>::T
        operator()(typename DT<PIXEL_T_IN>::T in) const {
            // Set all three R,G,B channels to grayscale in.
            const unsigned PerChannelPixelWidth =
                DT<PIXEL_T_OUT, NPPC>::PerChannelPixelWidth;
            typename DT<PIXEL_T_OUT>::T out;
            out.byte(0, PerChannelPixelWidth) = in;
            out.byte(1, PerChannelPixelWidth) = in;
            out.byte(2, PerChannelPixelWidth) = in;
            return out;
        }
    };

    TransformPixel(InImg, OutImg, GRAY2RGBFunctor());
}

} // end of namespace vision
} // end of namespace hls
#endif
