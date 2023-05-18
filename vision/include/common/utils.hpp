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

#ifndef __SHLS_VISION_UTILS_HPP__
#define __SHLS_VISION_UTILS_HPP__

#include "common.hpp"
#include "line_buffer.hpp"

namespace hls {
namespace vision {

/**
 * Iterates over the input image pixel by pixel, and calls the pass-in functor
 * to compute each output pixel based on the corresponding input pixel.
 * The loop iterating a line of pixels is pipelined.  If there are multiple
 * pixels per cycle, all pixels will be processed in parallel.
 */
template <PixelType PIXEL_T_I, PixelType PIXEL_T_O, unsigned H, unsigned W,
          StorageType STORAGE_I = FIFO, StorageType STORAGE_O = FIFO,
          NumPixelsPerCycle NPPC = NPPC_1, typename Func>
void TransformPixel(vision::Img<PIXEL_T_I, H, W, STORAGE_I, NPPC> &ImgIn,
                    vision::Img<PIXEL_T_O, H, W, STORAGE_O, NPPC> &ImgOut,
                    Func Functor) {

    for (unsigned i = 0, idx = 0; i < ImgIn.get_height(); i++) {
#ifdef __SYNTHESIS__
#pragma HLS loop pipeline
#endif
        for (unsigned j = 0; j < ImgIn.get_width() / NPPC; j++, idx++) {
            typename DT<PIXEL_T_I, NPPC>::T ImgdataIn = ImgIn.read(idx);
            typename DT<PIXEL_T_O, NPPC>::T ImgdataOut;

            // There may be multiple pixels per cycle.
            // We will split the pixels.
            for (unsigned k = 0; k < NPPC; k++) {
                const static unsigned kIW = DT<PIXEL_T_I>::W,
                                      kOW = DT<PIXEL_T_O>::W;

                typename DT<PIXEL_T_I>::T InPixel = ImgdataIn.byte(k, kIW);

                typename DT<PIXEL_T_O>::T OutPixel = Functor(InPixel);
                ImgdataOut.byte(k, kOW) = OutPixel;
            }
            ImgOut.write(ImgdataOut, idx);
        }
    }
}

} // End of namespace vision.
} // End of namespace hls.

#endif
