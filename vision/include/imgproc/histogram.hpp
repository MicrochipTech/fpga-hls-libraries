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

#pragma once

#include "../common/common.hpp"
#include "../common/utils.hpp"
#include <hls/ap_fixpt.hpp>

namespace hls {
namespace vision {

template <
    unsigned int HIST_SIZE = 256,
    PixelType PIXEL_T,
    unsigned H, 
    unsigned W, 
    StorageType STORAGE_T, 
    NumPixelsPerCycle NPPC
> void Histogram (
    Img<PIXEL_T, H, W, STORAGE_T, NPPC> &InImg,
    uint32_t histogram[HIST_SIZE]
) {
    static_assert(DT<PIXEL_T, NPPC>::NumChannels == 1,
        "Histogram function only supports one channel");

    static_assert(NPPC == 1,
        "Histogram function only supports one pixel per cycle (NPPC = 1).");

    static_assert(DT<PIXEL_T, NPPC>::W == 8, 
        "Histogram function only supports 8 bits per channel");

    const unsigned ImgHeight = InImg.get_height(), ImgWidth = InImg.get_width();

    // using PixelWordT = typename DT<PIXEL_T, NPPC>::T;
    using PixelWordT = ap_uint<DT<PIXEL_T, NPPC>::W>;

    uint8_t prevPix = 0;
    uint32_t prevVal = 0;

    static uint32_t h_tmp[HIST_SIZE];

    HLS_VISION_HISTOGRAM_LOOP1:
    #pragma HLS loop pipeline
    #pragma HLS loop dependence variable(h_tmp) type(inter) direction(RAW) dependent(false)
    for(int p=0; p < ImgHeight * ImgWidth; p++) {
        PixelWordT curPix = InImg.read(p);
        uint32_t curVal = h_tmp[curPix];
        if (prevPix == curPix) {
            curVal = prevVal;
        }
        curVal++;
        h_tmp[curPix] = curVal;
        prevPix = curPix;
        prevVal = curVal;
    }

    HLS_VISION_HISTOGRAM_LOOP2:
    #pragma HLS loop pipeline II(1)
    for(int p=0; p < HIST_SIZE; p++) {
        // printf("h_tmp[%d]:%d\n", p, h_tmp[p]);
        histogram[p] = h_tmp[p];
        h_tmp[p] = 0;
    }
}

} // End of namespace vision.
} // End of namespace hls.
