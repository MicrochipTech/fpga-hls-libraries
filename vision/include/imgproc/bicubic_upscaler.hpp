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

#include <cstdio>
#include <type_traits>

#include "../common/common.hpp"
#include "../common/utils.hpp"
#include "hls/utils.h"
#include "hls_math.hpp"


namespace hls {
namespace vision {


#define HLS_MAX(a, b) ((a) > (b) ? (a) : (b))
#define HLS_MIN(a, b) ((a) < (b) ? (a) : (b))

#define FP_WIDTH 20
#define FP_INT_WIDTH 10

// x <= 1.0        : 1.0 - 2.0*x^2 + x^3
// 1.0 < x < 2.0   : 4.0 - 8.0*x + 5.0*x^2 - x^3
// x >= 2.0        : 0.0
// float cubic_weight(float x) {
//     float abs_x = std::abs(x);
//     float ret = 0.0f;
//     if (abs_x <= 1.0f) {
//         ret = 1.0f - 2.0f * abs_x * abs_x + abs_x * abs_x * abs_x;
//     } else if (abs_x < 2.0f) {
//         ret = 4.0f - 8.0f * abs_x + 5.0f * abs_x * abs_x - abs_x * abs_x * abs_x;
//     } else {
//         ret = 0.0f;
//     }
//     // printf("x:%f, dim: %d, ret: %f, kx: %f, ky: %f\n", x, dim, ret, kx, ky);
//     printf("x:%f, ret: %f\n", x, ret);
//     return ret;
// };

using fp_t = hls::ap_fixpt<FP_WIDTH, FP_INT_WIDTH>;
// using fp_t = float;

fp_t cubic_weight(fp_t x) {
    // fp_t x_fixed(x);
    fp_t abs_x = hls::math::abs<FP_WIDTH, FP_INT_WIDTH>(x);
    // fp_t abs_x = std::abs(x);
    fp_t ret = 0.0;
    if (abs_x <= fp_t(1.0)) {
        ret = fp_t(1.0) - fp_t(2.0) * abs_x * abs_x + abs_x * abs_x * abs_x;
    } else if (abs_x < fp_t(2.0)) {
        ret = fp_t(4.0) - fp_t(8.0) * abs_x + fp_t(5.0) * abs_x * abs_x - abs_x * abs_x * abs_x;
    } else {
        ret = fp_t(0.0);
    }
    // printf("x:%f, dim: %d, ret: %f, kx: %f, ky: %f\n", x, dim, ret, kx, ky);
    // printf("%c: x:%f, abs_x: %f, ret: %f\n", c, x.to_double(), abs_x.to_double(), ret.to_double()); std::fflush(stdout);
    return ret;
};


// struct BicubicTable {
//     float Table[16];
//     BicubicTable() {
//         HLS_BICUBIC_TABLE_LOOP:
//         for (int i = 0; i < 16; i++) {
//             Table[i] = cubic_weight(i);
//         }
//     };


//     float getWeight(int i) const { return Table[i]; }

//     void printTables() const {
//         printf("\n----------[Weights]-------------\n");
//         for (int i = 0; i < 16; i++) {
//             printf("[%d]:%f\n", i, getWeight(i));
//         }
//     }
// };

//------------------------------------------------------------------------------
template <
    unsigned FILTER_SIZE,
    PixelType PIXEL_T_IN, 
    PixelType PIXEL_T_OUT,
    unsigned IN_H, 
    unsigned IN_W, 
    unsigned OUT_H, 
    unsigned OUT_W, 
    StorageType STORAGE_IN,
    StorageType STORAGE_OUT, 
    NumPixelsPerCycle NPPC
> int BicubicProcess (
    vision::Img<PIXEL_T_IN, IN_H, IN_W, STORAGE_IN, NPPC> &InImg,
    vision::Img<PIXEL_T_OUT, OUT_H, OUT_W, STORAGE_OUT, NPPC> &OutImg,
    typename DT<PIXEL_T_IN, NPPC>::T LB[FILTER_SIZE][IN_W],
    unsigned &x,
    unsigned &y,
    int &prev_y
) {
    using OutPixelWordT = typename DT<PIXEL_T_OUT, NPPC>::T;

    const unsigned PixelWidth = DT<PIXEL_T_IN, NPPC>::W / NPPC;
    const unsigned InChannelWidth = vision::DT<PIXEL_T_IN, NPPC>::PerChannelPixelWidth;
    const unsigned OutChannelWidth = vision::DT<PIXEL_T_OUT, NPPC>::PerChannelPixelWidth;
    const unsigned InImgHeight = InImg.get_height();
    const unsigned InImgWidth = InImg.get_width();

    using TmpPixelT = ap_uint<PixelWidth>;

    int ret = 0;    

    // Scale factors
    const fp_t scale_x = static_cast<float>(OUT_W) / IN_W;
    const fp_t scale_y = static_cast<float>(OUT_H) / IN_H;
    // printf("scale_x: %f, scale_y: %f\n", scale_x.to_double(), scale_y.to_double());

    // Find corresponding input coordinates
    fp_t in_x = fp_t(x) / scale_x;
    fp_t in_y = fp_t(y) / scale_y;

    // Get integer part
    // int ix = static_cast<int>(in_x);
    // int iy = static_cast<int>(in_y);

    int ix = static_cast<int>((float)in_x.to_double());
    int iy = static_cast<int>((float)in_y.to_double());

    // hls_dbg_printf("iy: %d, ix: %d, prev_y: %d, in_x: %f, in_y: %f\n", iy, ix, prev_y, in_x.to_double(), in_y.to_double());

    if(prev_y != iy) {
        ret = 1;
    } else {
        ret = 0;
    }

    fp_t sum_r = 0.0, sum_g = 0.0, sum_b = 0.0;
    fp_t weight_sum = 0.0;

    // Iterate over 4x4 kernel centered around the input pixel
    HLS_BICUBIC_PROCESS_LOOP1:
    for (int ky = -1; ky <= 2; ky++) {
        HLS_BICUBIC_PROCESS_LOOP2:
        for (int kx = -1; kx <= 2; kx++) {
                
            // Calculate distances from current kernel position to target position
            // and calculate bicubic weight based on x and y distances
            fp_t dx = in_x - (ix + kx);
            fp_t dy = in_y - (iy + ky);
            // printf("dx: %f, dy: %f\n", dx.to_double(), dy.to_double()); std::fflush(stdout);
            fp_t weight = cubic_weight(dx) * cubic_weight(dy);
            // printf("weight: %f\n", weight.to_double()); std::fflush(stdout);
            
            unsigned int sample_y = HLS_MIN(HLS_MAX(iy + ky, 0), HLS_MIN(iy + ky, (int)(InImgHeight-1))) % FILTER_SIZE;
            unsigned int sample_x = HLS_MIN(HLS_MAX(ix + kx, 0), (int)(InImgWidth - 1));
            
            // hls_dbg_printf("p[y=%d,x=%d]:", sample_y, sample_x); std::fflush(stdout);
            auto pixel = LB[sample_y][sample_x];
            // hls_dbg_printf("0x%x\n", pixel.to_uint64()); std::fflush(stdout);

            sum_r = sum_r + (weight * (unsigned)pixel.byte(0,InChannelWidth));
            sum_g = sum_g + (weight * (unsigned)pixel.byte(1,InChannelWidth));
            sum_b = sum_b + (weight * (unsigned)pixel.byte(2,InChannelWidth));
            weight_sum = weight_sum + weight; // Track total weight for normalization
            // printf("sum_r: %f, sum_g: %f, sum_b: %f, weight: %f, weight_sum: %f\n", sum_r.to_double(), sum_g.to_double(), sum_b.to_double(), weight.to_double(), weight_sum.to_double()); std::fflush(stdout);
        }
    }

    // Normalize and write output
    OutPixelWordT OutPixelWord;
    OutPixelWord.byte(0,OutChannelWidth) = static_cast<unsigned char>(HLS_MIN(HLS_MAX(fp_t(sum_r / weight_sum), fp_t(0.0)), fp_t(255.0)));
    OutPixelWord.byte(1,OutChannelWidth) = static_cast<unsigned char>(HLS_MIN(HLS_MAX(fp_t(sum_g / weight_sum), fp_t(0.0)), fp_t(255.0)));
    OutPixelWord.byte(2,OutChannelWidth) = static_cast<unsigned char>(HLS_MIN(HLS_MAX(fp_t(sum_b / weight_sum), fp_t(0.0)), fp_t(255.0)));
    OutImg.write(OutPixelWord, y * (OUT_W / NPPC) + x);
    hls_dbg_printf("p: 0x%x, w: %f, r: %f, g: %f, b: %f\n", OutPixelWord.to_uint64(), weight_sum.to_double(), sum_r.to_double(), sum_g.to_double(), sum_b.to_double()); // std::fflush(stdout);

    if (x == OUT_W - 1) {
        prev_y = iy;
    }

    // We're done with this pixel. Now update the coordinate to the next one.
    // if (x < (OUT_W / NPPC) - 1) {
    //     x++;

    // } else { // x == OUT_W - 1.
    //     y++;
    //     x = 0;
    // }
    return ret;
}

template <
    PixelType PIXEL_T, 
    unsigned H_IN,
    unsigned W_IN,
    unsigned H_OUT,
    unsigned W_OUT,
    StorageType STORAGE_TYPE,
    NumPixelsPerCycle NPPC
> 
void BicubicUpscaler(
    Img<PIXEL_T, H_IN, W_IN, STORAGE_TYPE, NPPC> &InImg,
    Img<PIXEL_T, H_OUT, W_OUT, STORAGE_TYPE, NPPC> &OutImg
) {
    #pragma HLS memory partition argument(InImg) type(struct_fields)
    #pragma HLS memory partition argument(OutImg) type(struct_fields)

    const unsigned FILTER_SIZE = 5; 

    static_assert(NPPC == 1,
        "BicubicUpscaler currenlty only supports NPPC=1.");

    static_assert(W_IN % NPPC == 0,
        "In BicubicUpscaler, the input width of the frame has to be divisible "
        "by the number of pixels per clock.");

    static_assert(W_OUT % NPPC == 0,
        "In BicubicUpscaler, the output width of the frame has to be divisible "
        "by the number of pixels per clock.");

    // #pragma HLS memory replicate_rom variable(BICUBIC_TABLE) max_replicas(0)
    // static const BicubicTable BICUBIC_TABLE;

    using InPixelWordT = typename DT<PIXEL_T, NPPC>::T;

    const unsigned InHeight = InImg.get_height(); 
    const unsigned InWidth = InImg.get_width();
    const unsigned OutHeight = OutImg.get_height(); 
    const unsigned OutWidth = OutImg.get_width();
    const unsigned InFrameSize = (InHeight * InWidth) / NPPC;
    const unsigned OutFrameSize = (OutHeight * OutWidth) / NPPC;
    // printf("InHeight: %d, InWidth: %d, InFrameSize: %d, OutHeight: %d, OutWidth: %d, OutFrameSize: %d\n", InHeight, InWidth, InFrameSize, OutHeight, OutWidth, OutFrameSize); std::fflush(stdout);

    const unsigned InPixelWidth = DT<PIXEL_T, NPPC>::W / NPPC;

    #pragma HLS memory partition variable(LB) type(complete) dim(1)
    InPixelWordT LB[FILTER_SIZE][W_IN];

    unsigned LineBufferPixelWordFillCount = 3*W_IN;

    unsigned y = 0, x = 0;
    int wy = 0, wx = 0;


/*
    // 8x12 table with pattern ax where x is consecutive
    {
        0 {"0x80", "0x81", "0x82", "0x83", "0x84", "0x85", "0x86", "0x87", "0x88", "0x89", "0x8a", "0x8b"},
        1 {"0x8c", "0x8d", "0x8e", "0x8f", "0x90", "0x91", "0x92", "0x93", "0x94", "0x95", "0x96", "0x97"},
        2 {"0x98", "0x99", "0x9a", "0x9b", "0x9c", "0x9d", "0x9e", "0x9f", "0xa0", "0xa1", "0xa2", "0xa3"},
        3 {"0xa4", "0xa5", "0xa6", "0xa7", "0xa8", "0xa9", "0xaa", "0xab", "0xac", "0xad", "0xae", "0xaf"},
        4 {"0xb0", "0xb1", "0xb2", "0xb3", "0xb4", "0xb5", "0xb6", "0xb7", "0xb8", "0xb9", "0xba", "0xbb"},
        0 {"0xbc", "0xbd", "0xbe", "0xbf", "0xc0", "0xc1", "0xc2", "0xc3", "0xc4", "0xc5", "0xc6", "0xc7"},
        1 {"0xc8", "0xc9", "0xca", "0xcb", "0xcc", "0xcd", "0xce", "0xcf", "0xd0", "0xd1", "0xd2", "0xd3"},
        2 {"0xd4", "0xd5", "0xd6", "0xd7", "0xd8", "0xd9", "0xda", "0xdb", "0xdc", "0xdd", "0xde", "0xdf"}
    };

*/

    HLS_VISION_BICUBIC_UPSCALER_FILL_BUFFER_LOOP1:
    for (wy = 0; wy < 3; wy++) {
        HLS_VISION_BICUBIC_UPSCALER_FILL_BUFFER_LOOP2:
        #pragma HLS loop pipeline
        for (wx = 0; wx < W_IN; wx++) {
            auto InPixelWord = InImg.read();
            LB[wy][wx] = InPixelWord;
        }
    }
    wx = 0;

    unsigned Count = LineBufferPixelWordFillCount;

    int prev_y = -1;

    HLS_VISION_BICUBIC_UPSCALER_LOOPY:
    for (y=0; y<OutHeight; y++) {
        HLS_VISION_BICUBIC_UPSCALER_LOOPX:
        #pragma HLS loop pipeline
        for (x=0; x<OutWidth; x++) {

            auto RdNextRow = BicubicProcess<FILTER_SIZE>(InImg, OutImg, LB, x, y, prev_y);

            if (wx < InWidth && Count < InFrameSize && RdNextRow) {
                auto p = InImg.read();
                Count++;
                // hls_dbg_printf("LB[wy=%d,wx=%d]<-0x%x (Count=%d)\n", wy, wx, p.to_uint64(), Count); std::fflush(stdout);
                LB[wy][wx] = p;
                wx++;
                if (wx == W_IN) {
                    wy = (wy + 1) % FILTER_SIZE;
                }
            }
        }
        wx = 0;
    }
}

} // End of namespace vision.
} // End of namespace hls.

