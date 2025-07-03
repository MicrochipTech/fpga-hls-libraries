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
#include "hls/ap_fixpt.hpp"
#include <cmath>

namespace hls {
namespace vision {

//------------------------------------------------------------------------------
// Generate the Gamma Lookup Table
template <unsigned WIDTH = 8>
struct GammaTable {
    static constexpr unsigned SIZE = 1 << WIDTH;
    ap_int<WIDTH> Table[SIZE];
    GammaTable(const float gamma = 2.2) {
        #pragma unroll
        for(int i=0; i < SIZE; i++) {
            const float fv = pow((float)i / (SIZE-1), 1.0 / gamma) * (SIZE-1);
            ap_ufixpt<WIDTH,WIDTH, AP_TRN, AP_SAT> val = fv;
            Table[i] = ap_uint<WIDTH>(val.raw_bits());
        }
        // printTable();
    };
    ap_int<WIDTH> getGamma(int i) const { 
        // printf("%d:%lld\n", i, Table[i].to_uint64());
        return Table[i]; 
    }

    // For debugging purposes
    void printTable() {
        printf("\n----------[Gamma Table]-------------\n");
        for(int i=0; i<SIZE; i++) {
            printf("%d:%lld\n", i, getGamma(i).to_uint64());
        }
    }
};

//------------------------------------------------------------------------------
template <
    PixelType PIXEL_T,
    unsigned H,
    unsigned W,
    StorageType STORAGE_IN,
    StorageType STORAGE_OUT,
    NumPixelsPerCycle NPPC = NPPC_1
> void GammaCorrection (
    vision::Img<PIXEL_T, H, W, STORAGE_IN, NPPC> &InImg,
    vision::Img<PIXEL_T, H, W, STORAGE_OUT, NPPC> &OutImg,
    const float gamma,
    const ap_uint<1> enable = 1 
) {
    #pragma HLS memory partition argument(InImg) type(struct_fields)
    #pragma HLS memory partition argument(OutImg) type(struct_fields)

    static_assert(DT<PIXEL_T, NPPC>::PerChannelPixelWidth == 8,
        "GammaCorrection only supports ChannelWidth = 8");

    const unsigned ChannelWidth = DT<PIXEL_T, NPPC>::PerChannelPixelWidth;
    const unsigned PixelWidth = DT<PIXEL_T, NPPC>::W / NPPC;
    const unsigned NumChannels = DT<PIXEL_T, NPPC>::NumChannels;
    const unsigned NumPixelWords = InImg.get_height() * InImg.get_width() / NPPC;

    #pragma HLS memory replicate_rom variable(GAMMA_CORRECTION_TABLE.Table) max_replicas(0)
    static const GammaTable<ChannelWidth> GAMMA_CORRECTION_TABLE(gamma);
    // GAMMA_CORRECTION_TABLE.printTable();

    static_assert(W % NPPC == 0,
                  "In GammaCorrection, the width of the frame has to be divisible "
                  "by the number of pixels per clock.");

    OutImg.set_height(InImg.get_height());
    OutImg.set_width(InImg.get_width());

    TransformPixel_enable(InImg, OutImg, enable, [](ap_uint<DT<PIXEL_T, NPPC>::W / NPPC> in){
        decltype(in) out;

        #pragma HLS loop unroll
        for(int c=0; c<NumChannels; c++) {
            out.byte(c, ChannelWidth) = GAMMA_CORRECTION_TABLE.getGamma(in.byte(c,ChannelWidth));
        }
        return out;
    });
}

} // End of namespace vision.
} // End of namespace hls.

