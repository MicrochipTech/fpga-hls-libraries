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

#define BICUBIC_FP_WIDTH 32
#define BICUBIC_FP_INT_WIDTH 16

#define WIN_SZ 4
// x <= 1.0        : 1.0 - 2.0*x^2 + x^3
// 1.0 < x < 2.0   : 4.0 - 8.0*x + 5.0*x^2 - x^3
// x >= 2.0        : 0.0

using fp_t = hls::ap_fixpt<BICUBIC_FP_WIDTH, BICUBIC_FP_INT_WIDTH>;

fp_t cubic_weight(fp_t x) {
    fp_t abs_x = hls::math::abs<BICUBIC_FP_WIDTH, BICUBIC_FP_INT_WIDTH>(x);
    fp_t ret = 0.0;
    if (abs_x <= fp_t(1.0)) {
        ret = fp_t(1.0) - fp_t(2.0) * abs_x * abs_x + abs_x * abs_x * abs_x;
    } else if (abs_x < fp_t(2.0)) {
        ret = fp_t(4.0) - fp_t(8.0) * abs_x + fp_t(5.0) * abs_x * abs_x - abs_x * abs_x * abs_x;
    } else {
        ret = fp_t(0.0);
    }
    return ret;
};


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
> void BicubicProcess (
    vision::Img<PIXEL_T_IN, IN_H, IN_W, STORAGE_IN, NPPC> &InImg,
    vision::Img<PIXEL_T_OUT, OUT_H, OUT_W, STORAGE_OUT, NPPC> &OutImg,
    typename DT<PIXEL_T_IN, NPPC>::T win[WIN_SZ][WIN_SZ],
    unsigned &x,
    unsigned &y
) {
    using OutPixelWordT = typename DT<PIXEL_T_OUT, NPPC>::T;

    const unsigned PixelWidth = DT<PIXEL_T_IN, NPPC>::W / NPPC;
    const unsigned InChannelWidth = vision::DT<PIXEL_T_IN, NPPC>::PerChannelPixelWidth;
    const unsigned OutChannelWidth = vision::DT<PIXEL_T_OUT, NPPC>::PerChannelPixelWidth;
    const unsigned InImgHeight = InImg.get_height();
    const unsigned InImgWidth = InImg.get_width();

    using TmpPixelT = ap_uint<PixelWidth>;

    // Scale factors
    const fp_t scale_x = static_cast<float>(OUT_W) / IN_W;
    const fp_t scale_y = static_cast<float>(OUT_H) / IN_H;
    // printf("scale_x: %f, scale_y: %f\n", scale_x.to_double(), scale_y.to_double());

    // Find corresponding input coordinates
    fp_t in_x = fp_t(x) / scale_x;
    fp_t in_y = fp_t(y) / scale_y;

    // Get integer part
    int ix = static_cast<int>((float)in_x.to_double());
    int iy = static_cast<int>((float)in_y.to_double());

    // hls_dbg_printf("iy: %d, ix: %d, prev_y: %d, in_x: %f, in_y: %f\n", iy, ix, prev_y, in_x.to_double(), in_y.to_double());

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
            
            // hls_dbg_printf("p[y=%d,x=%d]:", sample_y, sample_x); std::fflush(stdout);
            auto pixel = win[ky+1][kx+1];
            // hls_dbg_printf("0x%x\n", pixel.to_uint64()); std::fflush(stdout);

            sum_r = sum_r + (weight * (unsigned)pixel.byte(0,InChannelWidth));
            sum_g = sum_g + (weight * (unsigned)pixel.byte(1,InChannelWidth));
            sum_b = sum_b + (weight * (unsigned)pixel.byte(2,InChannelWidth));
            weight_sum = weight_sum + weight; // Track total weight for normalization
            // printf("sum_r: %f, sum_g: %f, sum_b: %f, weight: %f, weight_sum: %f\n", 
            //     sum_r.to_double(), sum_g.to_double(), sum_b.to_double(), 
            //     weight.to_double(), weight_sum.to_double()); 
            // std::fflush(stdout);
        }
    }

    // Normalize and write output
    OutPixelWordT OutPixelWord;
    OutPixelWord.byte(0,OutChannelWidth) = static_cast<unsigned char>(HLS_MIN(HLS_MAX(fp_t(sum_r / weight_sum), fp_t(0.0)), fp_t(255.0)));
    OutPixelWord.byte(1,OutChannelWidth) = static_cast<unsigned char>(HLS_MIN(HLS_MAX(fp_t(sum_g / weight_sum), fp_t(0.0)), fp_t(255.0)));
    OutPixelWord.byte(2,OutChannelWidth) = static_cast<unsigned char>(HLS_MIN(HLS_MAX(fp_t(sum_b / weight_sum), fp_t(0.0)), fp_t(255.0)));
    OutImg.write(OutPixelWord, y * (OUT_W / NPPC) + x);
    hls_dbg_printf("p: 0x%x, w: %f, r: %f, g: %f, b: %f\n", OutPixelWord.to_uint64(), weight_sum.to_double(), sum_r.to_double(), sum_g.to_double(), sum_b.to_double()); // std::fflush(stdout);
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
    
    #pragma HLS memory partition variable(win) type(complete)
    InPixelWordT win[WIN_SZ][WIN_SZ];

    unsigned LineBufferPixelWordFillCount = 3*W_IN;

    unsigned y = 0, x = 0;
    unsigned wy = 0, wx = 0;


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

    // Scale factors
    const fp_t scale_x = static_cast<float>(OutWidth) / InWidth;
    const fp_t scale_y = static_cast<float>(OutHeight) / InHeight;
    // printf("scale_x: %f, scale_y: %f\n", scale_x.to_double(), scale_y.to_double());

    HLS_VISION_BICUBIC_UPSCALER_FILL_BUFFER_LOOP1:
    for (wy = 0; wy < 3; wy++) {
        HLS_VISION_BICUBIC_UPSCALER_FILL_BUFFER_LOOP2:
        #pragma HLS loop pipeline
        for (wx = 0; wx < W_IN; wx++) {
            auto InPixelWord = InImg.read();
            LB[wy][wx] = InPixelWord;
            // if (wx < WIN_SZ) {
            //     win[wy][wx] = InPixelWord;
            // }
        }
    }
    wx = 0;

    unsigned Count = LineBufferPixelWordFillCount;

    int prev_y = -1;

    for (y=0; y < H_OUT; y++) {
        #pragma HLS loop pipeline
        for (x=0; x < W_OUT; x++) {

            // Find corresponding input coordinates
            fp_t in_x = fp_t(x) / scale_x;
            fp_t in_y = fp_t(y) / scale_y;

            // Get integer part
            int ix = static_cast<int>((float)in_x.to_double());
            int iy = static_cast<int>((float)in_y.to_double());

            // Shift the window to the left
            for (int i = 0; i < WIN_SZ; ++i) {
                for (int j = 0; j < WIN_SZ-1; ++j) {
                    win[i][j] = win[i][j+1];
                }
            }

            // concatenated selected row { LB[FILTER_SIZE-1][row], ... LB[1][row], LB[0][row] }
            ap_uint<InPixelWidth * FILTER_SIZE> LB_row;

            unsigned sample_x = HLS_MIN(ix + 3, (W_IN - 1));
            for ( int _i=0; _i < FILTER_SIZE; _i++ )
                LB_row.byte(_i, InPixelWidth) = LB[_i][sample_x];

            // Fill the rightmost column of the window
            LOOP_LB:
            for (int ky = 0; ky < WIN_SZ; ky++) {
                unsigned sample_y = iy + ky - 1;

                // Clip the index to 0 or the height of the image
                if (sample_y < 0) {
                    sample_y = 0;
                } else if (sample_y >= H_IN) {
                    sample_y = H_IN-1;
                }

                win[ky][3] = LB_row.byte(sample_y % FILTER_SIZE, InPixelWidth);

            }

            // Process the window
            BicubicProcess<FILTER_SIZE>(InImg, OutImg, win, x, y);

            // Do we need to read another row ?
            int RdNextRow = (prev_y != iy) ? 1 : 0;

            // Fill the line buffer
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

            // Update the previous row index if we're done with the current one
            if (x == OutWidth - 1) {
                prev_y = iy;
            }
        }
        wx = 0;

        // Shift the window down
        for (int i = 0; i < WIN_SZ-1; i++) {
            #pragma HLS loop pipeline
            for (int j = 0; j < WIN_SZ; j++) {
                win[i][j] = win[i+1][j];
            }
        }
    }
}

} // End of namespace vision.
} // End of namespace hls.

