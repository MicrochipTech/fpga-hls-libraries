// ©2025 Microchip Technology Inc. and its subsidiaries
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

#pragma once

#include <type_traits>

#include "../common/common.hpp"
#include "../common/utils.hpp"
#include "hls/ap_common.hpp"
#include "hls/ap_fixpt.hpp"
#include "hls/ap_int.hpp"

namespace hls {
namespace vision {

/***
 * @function ImageEnhance
 * 
 * This module will perform the following transformation to the video frame:
 * 
  * - $OutImg.r = brightness + (r\_factor / 32) * InImg.r$
 * - $OutImg.g = brightness + (g\_factor / 32) * InImg.g$
 * - $OutImg.b = brightness + (b\_factor / 32) * InImg.b$
 * 
 * The multiplying factors (`r_factor, g_factor, b_factor`) are 8-bit unsigned integers.
 * The division by 32 is to allow the effective factor to be less than 1 (factor < 32), 
 * exactly 1 (factor=32), or more than one (factor>32).
 * 
 * @template {PixelType PIXEL_T} Type of the pixel, e.g. HLS_8UC1
 * @template {unsigned H} Height of the image
 * @template {unsigned H} Width of the image
 * @template {StorageType STORAGE_IN} Type of storage for the input image
 * @template {StorageType STORAGE_OUT} Type of storage for the output image
 * @template {NumPixelsPerCycle NPPC} Number of pixels per cycle. This must be 
 *      divisible by the width of the image. 
 * 
 * @param {vision::Img InImg} Input image in RGB format.
 * @param {vision::Img OutImg} Output image in RGB format.
 * @param {ap_uint<8> b_factor} Factor for blue channel.
 * @param {ap_uint<8> g_factor} Factor for green channel.
 * @param {ap_uint<8> r_factor} Factor for red channel.
 * @param {ap_int<10> brightness} Brightness factor common to all channels. This
 * number can be positive or negative to compensate for the per-channel scaling.
 *
 * @example
 * hls::vision::ImageEnhance(ImgIn, ImgOut, 62, 42, 52, 12);
 */


template <
    PixelType PIXEL_T,
    unsigned H,
    unsigned W,
    StorageType STORAGE_IN,
    StorageType STORAGE_OUT, 
    NumPixelsPerCycle NPPC = NPPC_1>
void ImageEnhance(
    vision::Img<PIXEL_T, H, W, STORAGE_IN, NPPC> &InImg,
    vision::Img<PIXEL_T, H, W, STORAGE_OUT, NPPC> &OutImg,
    ap_uint<8> b_factor,
    ap_uint<8> g_factor,
    ap_uint<8> r_factor,
    ap_int<10> brightness
) {
    #pragma HLS memory partition argument(InImg) type(struct_fields)
    #pragma HLS memory partition argument(OutImg) type(struct_fields)

    using PixelWordT = typename DT<PIXEL_T, NPPC>::T;
    const unsigned ChannelWidth = DT<PIXEL_T, NPPC>::PerChannelPixelWidth;
    const unsigned PixelWidth = DT<PIXEL_T, NPPC>::W / NPPC;

    static_assert(DT<PIXEL_T, NPPC>::NumChannels == 3 &&
        (NPPC == NPPC_1 || NPPC == NPPC_4),
        "Image_Enhance only supports number of pixels per cycle of 1 or 4. "
        "Input data should have only be 3 channels.");

    // 
    ap_ufixpt<8,3, AP_RND, AP_SAT> b_mult = b_factor / 32.0;
    ap_ufixpt<8,3, AP_RND, AP_SAT> g_mult = g_factor / 32.0;
    ap_ufixpt<8,3, AP_RND, AP_SAT> r_mult = r_factor / 32.0;

    TransformPixel(InImg, OutImg, [&](ap_int<PixelWidth> in){
        using fixpt_t = ap_ufixpt<ChannelWidth, ChannelWidth, AP_TRN, AP_SAT>;

        fixpt_t b = in.byte(2,ChannelWidth);
        fixpt_t g = in.byte(1,ChannelWidth);
        fixpt_t r = in.byte(0,ChannelWidth);

        fixpt_t rout = brightness + r_mult * r;
        fixpt_t gout = brightness + g_mult * g;
        fixpt_t bout = brightness + b_mult * b;

        // printf("brightness:%f, "
        //     "r_mult:%f, g_mult:%f, b_mult:%f, "
        //     "ri:%f, gi:%f, bi:%f, "
        //     "ro:%f, go:%f, bo:%f\n", 
        //     brightness.to_uint64(), r_mult.to_double(), g_mult.to_double(), b_mult.to_double(),
        //     r.to_double(), g.to_double(), b.to_double(),
        //     rout.to_double(), gout.to_double(), bout.to_double());

        ap_int<PixelWidth> out;
        out.byte(2, ChannelWidth) = bout.raw_bits();
        out.byte(1, ChannelWidth) = gout.raw_bits();
        out.byte(0, ChannelWidth) = rout.raw_bits();
        return out;
    });
}

} // End of namespace vision.
} // End of namespace hls.

