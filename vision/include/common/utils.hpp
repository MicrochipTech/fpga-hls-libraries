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
// MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE
// FORESEEABLE.  TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP’S TOTAL
// LIABILITY ON ALL CLAIMS LATED TO THE SOFTWARE WILL NOT EXCEED AMOUNT OF
// FEES, IF ANY, YOU PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE. MICROCHIP
// OFFERS NO SUPPORT FOR THE SOFTWARE. YOU MAY CONTACT MICROCHIP AT
// https://www.microchip.com/en-us/support-and-training/design-help/client-support-services
// TO INQUIRE ABOUT SUPPORT SERVICES AND APPLICABLE FEES, IF AVAILABLE.

#pragma once 

#include "common.hpp"
#include "line_buffer.hpp"
#include "hls/ap_fixpt.hpp"
#include "params.hpp"

namespace hls {
namespace vision {

constexpr unsigned FloorLog2(unsigned long long x) {
    return x == 1 ? 0 : 1 + FloorLog2(x >> 1);
}

template <unsigned long long N> 
struct CeilLog2 {
    static constexpr unsigned int value = 1 + CeilLog2<(N + 1) / 2>::value;
};

template <>
struct CeilLog2<1> {
    static constexpr unsigned int value = 0;
};

/**
 * Iterates over the input image pixel by pixel, and calls the pass-in functor
 * to compute each output pixel based on the corresponding input pixel.
 * The loop iterating a line of pixels is pipelined.  If there are multiple
 * pixels per cycle, all pixels will be processed in parallel.
 */
template <
    PixelType PIXEL_T_I, 
    PixelType PIXEL_T_O, 
    unsigned H, 
    unsigned W,
    StorageType STORAGE_I = FIFO, 
    StorageType STORAGE_O = FIFO,
    NumPixelsPerCycle NPPC = NPPC_1, 
    typename Func>
void TransformPixel(
    vision::Img<PIXEL_T_I, H, W, STORAGE_I, NPPC> &ImgIn,
    vision::Img<PIXEL_T_O, H, W, STORAGE_O, NPPC> &ImgOut,
    Func Functor) {

    const unsigned InPixelWidth = DT<PIXEL_T_I>::W;
    const unsigned OutPixelWidth = DT<PIXEL_T_O>::W;
    const unsigned NumPixelWords = ImgIn.get_height() * ImgIn.get_width() / NPPC;

    HLS_VISION_TRANSFORMPIXEL_LOOP:
    #pragma HLS loop pipeline
    for (unsigned k = 0; k < NumPixelWords; k++) {
        typename DT<PIXEL_T_I, NPPC>::T ImgdataIn = ImgIn.read(k);
        typename DT<PIXEL_T_O, NPPC>::T ImgdataOut;
        // There may be multiple pixels per cycle.
        HLS_VISION_TRANSFORMPIXEL_NPPC_LOOP:
        for (unsigned p = 0; p < NPPC; p++) {
            typename DT<PIXEL_T_I>::T InPixel = ImgdataIn.byte(p, InPixelWidth);
            typename DT<PIXEL_T_O>::T OutPixel = Functor(InPixel);
            ImgdataOut.byte(p, OutPixelWidth) = OutPixel;
        }
        ImgOut.write(ImgdataOut, k);
    }
}

/**
Same as Transform() function but it passes the "i" (height index) & "j" 
(width index) of the pixel being transformed. That way the functor can use the 
coordinates to decide what transformation to apply to the input pixel. 
*/
template <
    PixelType PIXEL_T_I, 
    PixelType PIXEL_T_O, 
    unsigned H, 
    unsigned W,
    StorageType STORAGE_I = FIFO, 
    StorageType STORAGE_O = FIFO,
    NumPixelsPerCycle NPPC = NPPC_1, 
    typename Func>
void TransformPixel_ij(
    vision::Img<PIXEL_T_I, H, W, STORAGE_I, NPPC> &ImgIn,
    vision::Img<PIXEL_T_O, H, W, STORAGE_O, NPPC> &ImgOut,
    Func Functor) {

    const unsigned InPixelWidth = DT<PIXEL_T_I>::W;
    const unsigned OutPixelWidth = DT<PIXEL_T_O>::W;
    const unsigned NumPixelWords = ImgIn.get_height() * ImgIn.get_width() / NPPC;

    HLS_VISION_TRANSFORMPIXEL_LOOP:
    #pragma HLS loop pipeline
    for (unsigned i=0, j=0, k = 0; k < NumPixelWords; k++) {
        typename DT<PIXEL_T_I, NPPC>::T ImgdataIn = ImgIn.read(k);
        typename DT<PIXEL_T_O, NPPC>::T ImgdataOut;
        for (unsigned p = 0; p < NPPC; p++, j++) {
            typename DT<PIXEL_T_I>::T InPixel = ImgdataIn.byte(p, InPixelWidth);
            typename DT<PIXEL_T_O>::T OutPixel = Functor(InPixel, i, j);
            ImgdataOut.byte(p, OutPixelWidth) = OutPixel;
        }
        ImgOut.write(ImgdataOut, k);
        if (j == ImgIn.get_width()) {
            j = 0;
            i++;
        }
    }
}

/**
Same as Transform() function but it passes the "enable" argument to control
if the pixel is being transformed or not. The use case is that where a CPU can 
control (enable) the transformation at runtime.
*/
template <
    PixelType PIXEL_T_I, 
    PixelType PIXEL_T_O, 
    unsigned H, 
    unsigned W,
    StorageType STORAGE_I = FIFO, 
    StorageType STORAGE_O = FIFO,
    NumPixelsPerCycle NPPC = NPPC_1, 
    typename Func>
void TransformPixel_enable(
    vision::Img<PIXEL_T_I, H, W, STORAGE_I, NPPC> &ImgIn,
    vision::Img<PIXEL_T_O, H, W, STORAGE_O, NPPC> &ImgOut,
    ap_uint<1> enable,
    Func Functor) {

    const unsigned InPixelWidth = DT<PIXEL_T_I>::W;
    const unsigned OutPixelWidth = DT<PIXEL_T_O>::W;
    const unsigned NumPixelWords = ImgIn.get_height() * ImgIn.get_width() / NPPC;

    HLS_VISION_TRANSFORMPIXEL_LOOP:
    #pragma HLS loop pipeline
    for (unsigned k = 0; k < NumPixelWords; k++) {
        typename DT<PIXEL_T_I, NPPC>::T ImgdataIn = ImgIn.read(k);
        typename DT<PIXEL_T_O, NPPC>::T ImgdataOut;
        for (unsigned p = 0; p < NPPC; p++) {
            typename DT<PIXEL_T_I>::T InPixel = ImgdataIn.byte(p, InPixelWidth);
            typename DT<PIXEL_T_O>::T OutPixel = Functor(InPixel);
            ImgdataOut.byte(p, OutPixelWidth) = enable ? OutPixel : InPixel;
        }
        ImgOut.write(ImgdataOut, k);
    }
}

/**
Transform() function but it passes an argument by reference.
*/
template <
    typename ARG_T,
    PixelType PIXEL_T_I, 
    PixelType PIXEL_T_O, 
    unsigned H, 
    unsigned W,
    StorageType STORAGE_I = FIFO, 
    StorageType STORAGE_O = FIFO,
    NumPixelsPerCycle NPPC = NPPC_1, 
    typename Func>
void TransformPixel_ref(
    vision::Img<PIXEL_T_I, H, W, STORAGE_I, NPPC> &ImgIn,
    vision::Img<PIXEL_T_O, H, W, STORAGE_O, NPPC> &ImgOut,
    ARG_T &arg,
    Func Functor) {

    const unsigned InPixelWidth = DT<PIXEL_T_I>::W;
    const unsigned OutPixelWidth = DT<PIXEL_T_O>::W;
    const unsigned NumPixelWords = ImgIn.get_height() * ImgIn.get_width() / NPPC;

    HLS_VISION_TRANSFORMPIXEL_LOOP:
    #pragma HLS loop pipeline
    for (unsigned k = 0; k < NumPixelWords; k++) {
        typename DT<PIXEL_T_I, NPPC>::T ImgdataIn = ImgIn.read(k);
        typename DT<PIXEL_T_O, NPPC>::T ImgdataOut;
        for (unsigned p = 0; p < NPPC; p++) {
            typename DT<PIXEL_T_I>::T InPixel = ImgdataIn.byte(p, InPixelWidth);
            typename DT<PIXEL_T_O>::T OutPixel = Functor(InPixel, arg);
            ImgdataOut.byte(p, OutPixelWidth) = OutPixel;
        }
        ImgOut.write(ImgdataOut, k);
    }
}


} // End of namespace vision.
} // End of namespace hls.
