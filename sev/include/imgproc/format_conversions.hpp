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

#ifndef __SEV_FORMAT_CONVERSIONS_HPP__
#define __SEV_FORMAT_CONVERSIONS_HPP__

#include "../common/common.hpp"
#include "../common/utils.hpp"
#include <hls/ap_fixpt.hpp>
#include <hls/ap_int.hpp>

namespace hls {
namespace sev {

/**
 * Convert sev::Img in RGB format to Grayscale.
 * Template parameters:
 *  - NTSC: if true, we use NTSC formula (0.299 * R + 0.587 * G + 0.114 * B),
 *          otherwise we use (R + G + B) / 3
 */
template <bool NTSC = true, PixelType PIXEL_T_I, PixelType PIXEL_T_O,
          unsigned H, unsigned W, StorageType STORAGE_I = FIFO,
          StorageType STORAGE_O = FIFO, NumPixelsPerCycle NPPC = NPPC_1>
void RGB2GRAY(sev::Img<PIXEL_T_I, H, W, STORAGE_I, NPPC> &ImgIn,
              sev::Img<PIXEL_T_O, H, W, STORAGE_O, NPPC> &ImgOut) {
    /* Assert that the formats of the input and output make sense */
    static_assert(DT<PIXEL_T_I, NPPC>::NumChannels == 3 &&
                      DT<PIXEL_T_O, NPPC>::NumChannels == 1,
                  "The input (RGB) must have 3 channels, and the output (GRAY) "
                  "must have 1 channel");
    // Assert pixel width of in and out are the same e.g. both have to be 8 bit,
    // or both have to be 16 bit, etc.
    static_assert(
        DT<PIXEL_T_I, NPPC>::PerChannelPixelWidth ==
            DT<PIXEL_T_I, NPPC>::PerChannelPixelWidth,
        "The per channel pixel width of the input and output must be the same");
    // TODO Tue: Assert the sign information as well e.g. 16UC vs 16SC?

    /* Define a functor to transform ImgIn pixel-by-pixel to get ImgOut */
    struct RGB2GRAYFunctor {
        typename DT<PIXEL_T_O>::T
        operator()(typename DT<PIXEL_T_I>::T in) const {
            using fixpt_t = ap_ufixpt<18, 10>;
            const unsigned PerChannelPixelWidth =
                DT<PIXEL_T_I, NPPC>::PerChannelPixelWidth;
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

    TransformPixel(ImgIn, ImgOut, RGB2GRAYFunctor());
}

/**
 * Converts sev::Img in Grayscale format to RGB.
 * Note that the resulting image is still visually Grayscale but represented in
 * RGB format.
 */
template <PixelType PIXEL_T_I, PixelType PIXEL_T_O, unsigned H, unsigned W,
          StorageType STORAGE_I = FIFO, StorageType STORAGE_O = FIFO,
          NumPixelsPerCycle NPPC = NPPC_1>
void GRAY2RGB(sev::Img<PIXEL_T_I, H, W, STORAGE_I, NPPC> &ImgIn,
              sev::Img<PIXEL_T_O, H, W, STORAGE_O, NPPC> &ImgOut) {
    /* Assert that the formats of the input and output make sense */
    static_assert(DT<PIXEL_T_I, NPPC>::NumChannels == 1 &&
                      DT<PIXEL_T_O, NPPC>::NumChannels == 3,
                  "The input (GRAY) must have 1 channel, and the output (RGB) "
                  "must have 3 channels");
    // Assert pixel width of in and out are the same e.g. both have to be 8 bit,
    // or both have to be 16 bit, etc.
    static_assert(
        DT<PIXEL_T_I, NPPC>::PerChannelPixelWidth ==
            DT<PIXEL_T_I, NPPC>::PerChannelPixelWidth,
        "The per channel pixel width of the input and output must be the same");
    // TODO Tue: Assert the sign information as well e.g. 16UC vs 16SC?

    /* Define a functor to transform ImgIn pixel-by-pixel to get ImgOut */
    struct GRAY2RGBFunctor {
        typename DT<PIXEL_T_O>::T
        operator()(typename DT<PIXEL_T_I>::T in) const {
            // Set all three R,G,B channels to grayscale in.
            const unsigned PerChannelPixelWidth =
                DT<PIXEL_T_O, NPPC>::PerChannelPixelWidth;
            typename DT<PIXEL_T_O>::T out;
            out.byte(0, PerChannelPixelWidth) = in;
            out.byte(1, PerChannelPixelWidth) = in;
            out.byte(2, PerChannelPixelWidth) = in;
            return out;
        }
    };

    TransformPixel(ImgIn, ImgOut, GRAY2RGBFunctor());
}

/**
 * Convert sev::Img in RGB format to YUV444 format.
 * Formula:
 *   Y =  0.299 * R + 0.587 * G + 0.114 * B
 *   U = -0.147 * R - 0.289 * G + 0.436 * B + 128
 *   V =  0.615 * R - 0.515 * G - 0.100 * B + 128
 * Note: The "+ 128" offset for U and V is because conceptually the U and V can
 * be negative. The formula OpenCV uses adds 128 to make the value positive.
 */
template <PixelType PIXEL_T_I, PixelType PIXEL_T_O, unsigned H, unsigned W,
          StorageType STORAGE_I = FIFO, StorageType STORAGE_O = FIFO,
          NumPixelsPerCycle NPPC = NPPC_1>
void RGB2YUV(sev::Img<PIXEL_T_I, H, W, STORAGE_I, NPPC> &ImgIn,
             sev::Img<PIXEL_T_O, H, W, STORAGE_O, NPPC> &ImgOut) {
    /* Assert that the formats of the input and output make sense */
    static_assert(DT<PIXEL_T_I, NPPC>::NumChannels == 3 &&
                      DT<PIXEL_T_O, NPPC>::NumChannels == 3,
                  "The input (RGB) must have 3 channels, and the output (YUV) "
                  "must have 3 channels");
   // Assert pixel width of in and out are the same e.g. both have to be 8 bit,
    // or both have to be 16 bit, etc.
    static_assert(
        DT<PIXEL_T_I, NPPC>::PerChannelPixelWidth ==
            DT<PIXEL_T_I, NPPC>::PerChannelPixelWidth,
        "The per channel pixel width of the input and output must be the same");
    // TODO Tue: Assert the sign information as well e.g. 16UC vs 16SC?

    /* Define a functor to transform ImgIn pixel-by-pixel to get ImgOut */
    struct RGB2YUVFunctor {
        typename DT<PIXEL_T_O>::T
        operator()(typename DT<PIXEL_T_I>::T in) const {
            // Use ap_fixpt instead of ap_ufixpt here
            // because the formula needs to deal with
            // negative / substract.
            using fixpt_t = ap_fixpt<18, 10>;
            const unsigned PerChannelPixelWidth =
                DT<PIXEL_T_I, NPPC>::PerChannelPixelWidth;
            auto r = in.byte(0, PerChannelPixelWidth),
                 g = in.byte(1, PerChannelPixelWidth),
                 b = in.byte(2, PerChannelPixelWidth);

            // Y =  0.299 * R + 0.587 * G + 0.114 * B
            // U = -0.147 * R - 0.289 * G + 0.436 * B + 128
            // V =  0.615 * R - 0.515 * G - 0.100 * B + 128
            // To not lose precision, for all multiply operations, we'll
            // multiply by coefficients that's 256 times larger, then shift
            // right by 8 to divide by 256.
            ap_ufixpt<PerChannelPixelWidth, PerChannelPixelWidth> r_fixpt = r,
                                                                  g_fixpt = g,
                                                                  b_fixpt = b;
            auto y_ = fixpt_t(76.544) * r_fixpt + fixpt_t(150.272) * g_fixpt +
                      fixpt_t(29.184) * b_fixpt;
            auto u_ = -fixpt_t(37.632) * r_fixpt - fixpt_t(73.984) * g_fixpt +
                      fixpt_t(111.616) * b_fixpt;
            auto v_ = fixpt_t(157.44) * r_fixpt - fixpt_t(-131.84) * g_fixpt -
                      fixpt_t(25.6) * b_fixpt;

            ap_uint<PerChannelPixelWidth> y = y_ >> 8 + fixpt_t(0.5),
                                          u = u_ >> 8 + fixpt_t(128.5),
                                          v = v_ >> 8 + fixpt_t(128.5);

            typename DT<PIXEL_T_O>::T out;
            out.byte(0, PerChannelPixelWidth) = y;
            out.byte(1, PerChannelPixelWidth) = u;
            out.byte(2, PerChannelPixelWidth) = v;

            return out;
        }
    };

    TransformPixel(ImgIn, ImgOut, RGB2YUVFunctor());
}

/**
 * Convert sev::Img in YUV444 format to RGB format.
 * Formula:
 *   U' = U - 128
 *   V' = V - 128
 *   R =  Y + 1.140 * V'
 *   G =  Y - 0.395 * U' - 0.581 * V'
 *   B =  Y + 2.032 * U'
 */
template <PixelType PIXEL_T_I, PixelType PIXEL_T_O, unsigned H, unsigned W,
          StorageType STORAGE_I = FIFO, StorageType STORAGE_O = FIFO,
          NumPixelsPerCycle NPPC = NPPC_1>
void YUV2RGB(sev::Img<PIXEL_T_I, H, W, STORAGE_I, NPPC> &ImgIn,
             sev::Img<PIXEL_T_O, H, W, STORAGE_O, NPPC> &ImgOut) {
    /* Assert that the formats of the input and output make sense */
    static_assert(DT<PIXEL_T_I, NPPC>::NumChannels == 3 &&
                      DT<PIXEL_T_O, NPPC>::NumChannels == 3,
                  "The input (YUV) must have 3 channels, and the output (RGB) "
                  "must have 3 channels");
    // Assert pixel width of in and out are the same e.g. both have to be 8 bit,
    // or both have to be 16 bit, etc.
    static_assert(
        DT<PIXEL_T_I, NPPC>::PerChannelPixelWidth ==
            DT<PIXEL_T_I, NPPC>::PerChannelPixelWidth,
        "The per channel pixel width of the input and output must be the same");
    // TODO Tue: Assert the sign information as well e.g. 16UC vs 16SC?

    /* Define a functor to transform ImgIn pixel-by-pixel to get ImgOut */
    struct YUV2RGBFunctor {
        typename DT<PIXEL_T_O>::T
        operator()(typename DT<PIXEL_T_I>::T in) const {
            // Use ap_fixpt instead of ap_ufixpt here
            // because the formula needs to deal with
            // negative / substract.
            using fixpt_t = ap_fixpt<18, 10>;
            const unsigned PerChannelPixelWidth =
                DT<PIXEL_T_I, NPPC>::PerChannelPixelWidth;
            auto y = in.byte(0, PerChannelPixelWidth),
                 u = in.byte(1, PerChannelPixelWidth),
                 v = in.byte(2, PerChannelPixelWidth);

            // U' = U - 128
            // V' = V - 128
            // R =  Y + 1.140 * V'
            // G =  Y - 0.395 * U' - 0.581 * V'
            // B =  Y + 2.032 * U'
            // To not lose precision, for all multiply operations, we'll
            // multiply by coefficients that's 256 times larger, then shift
            // right by 8 to divide by 256.
            ap_fixpt<PerChannelPixelWidth, PerChannelPixelWidth> y_fixpt = y,
                                                                 u_fixpt = u,
                                                                 v_fixpt = v;
            auto u_ = u - fixpt_t(128);
            auto v_ = v - fixpt_t(128);

            auto r_ = fixpt_t(256) * y + fixpt_t(291.84) * v_;
            auto g_ =
                fixpt_t(256) * y - fixpt_t(101.12) * u_ - fixpt_t(148.736) * v_;
            auto b_ = fixpt_t(256) * y + fixpt_t(520.192) * u_;

            ap_uint<PerChannelPixelWidth> r = r_ >> 8 + fixpt_t(0.5),
                                          g = g_ >> 8 + fixpt_t(0.5),
                                          b = b_ >> 8 + fixpt_t(0.5);

            typename DT<PIXEL_T_O>::T out;
            out.byte(0, PerChannelPixelWidth) = r;
            out.byte(1, PerChannelPixelWidth) = g;
            out.byte(2, PerChannelPixelWidth) = b;

            return out;
        }
    };

    TransformPixel(ImgIn, ImgOut, YUV2RGBFunctor());
}

} // end of namespace sev
} // end of namespace hls
#endif

