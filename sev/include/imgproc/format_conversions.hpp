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

#include "../common.hpp"
#include "../utils.hpp"
#include <hls/ap_fixpt.hpp>
#include <hls/ap_int.hpp>

namespace hls {
namespace sev {

/**
 * Convert sev::Img in RGB format to Grayscale.
 * Template parameters:
 *  - NTSC: if true, we use NTSC formula (0.299 * R + 0.587 * G + 0.114 * B),
 *          otherwise we use (R + G + B) / 3
 * CAVEAT: This function assumes 8 bit components (r, g, b), hence each (rgb)
 *         pixel having 3 * 8 = 24 bits of data.
 */
template <bool NTSC = true, PixelType PIXEL_T_I, PixelType PIXEL_T_O,
          unsigned H, unsigned W, StorageType STORAGE_I = FIFO,
          StorageType STORAGE_O = FIFO, NumPixelsPerCycle NPPC = NPPC_1>
void RGB2GS(sev::Img<PIXEL_T_I, H, W, STORAGE_I, NPPC> &ImgIn,
            sev::Img<PIXEL_T_O, H, W, STORAGE_O, NPPC> &ImgOut) {

    struct RGB2GSFunctor {
        typename DT<PIXEL_T_O>::T
        operator()(typename DT<PIXEL_T_I>::T in) const {

            using fixpt_t = ap_fixpt<18, 10>;
            ap_uint<8> r = in.range(7, 0), g = in.range(15, 8),
                       b = in.range(23, 16);

            if (!NTSC)
                return (r + g + b) / 3;

            // Use NTSC formula.
            auto sum = fixpt_t(76.544) * r + fixpt_t(150.272) * g +
                       fixpt_t(29.184) * b;
            return (sum >> 8) + fixpt_t(0.5);
        }
    };

    TransformPixel(ImgIn, ImgOut, RGB2GSFunctor());
}

/**
 * Converts sev::Img in Grayscale format to RGB.
 * Note that the resulting image is still visually Grayscale but represented in
 * RGB format.
 * CAVEAT: This function assumes GS is 8 bits per grayscale pixel. We can have a
 *         separate template parameter for pixel width.
 *         This function assumes 8 bit components (r, g, b), hence each (rgb)
 *         pixel having 3 * 8 = 24 bits of data
 */
template <PixelType PIXEL_T_I, PixelType PIXEL_T_O, unsigned H, unsigned W,
          StorageType STORAGE_I = FIFO, StorageType STORAGE_O = FIFO,
          NumPixelsPerCycle NPPC = NPPC_1>
void GS2RGB(sev::Img<PIXEL_T_I, H, W, STORAGE_I, NPPC> &ImgIn,
            sev::Img<PIXEL_T_O, H, W, STORAGE_O, NPPC> &ImgOut) {
    struct GS2RGBFunctor {
        typename DT<PIXEL_T_O>::T
        operator()(typename DT<PIXEL_T_I>::T in) const {
            // Set all three R,G,B channels to grayscale in.
            return (in(7, 0), in(7, 0), in(7, 0));
        }
    };
    TransformPixel(ImgIn, ImgOut, GS2RGBFunctor());
}

} // end of namespace sev
} // end of namespace hls
#endif

