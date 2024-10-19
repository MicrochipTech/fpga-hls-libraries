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

#include <cstdint>
#include <type_traits>
#include "bilateral_table.hpp"
#include "../common/common.hpp"
#include "../common/utils.hpp"
#include "hls/ap_fixpt.hpp"

namespace hls {
namespace vision {

template <
    unsigned FILTER_SIZE = 5,
    PixelType PIXEL_T_IN, 
    PixelType PIXEL_T_OUT,
    unsigned H, 
    unsigned W, 
    StorageType STORAGE_IN,
    StorageType STORAGE_OUT, 
    NumPixelsPerCycle NPPC
> void BilateralProcess (
    vision::Img<PIXEL_T_IN, H, W, STORAGE_IN, NPPC> &InImg,
    vision::Img<PIXEL_T_OUT, H, W, STORAGE_OUT, NPPC> &OutImg,
    LineBuffer<
        typename DT<PIXEL_T_IN, NPPC>::T, 
        W / NPPC, 
        FILTER_SIZE,
        DT<PIXEL_T_IN, 
        NPPC>::W / NPPC, 
        unsigned(NPPC)
    > &LineBuffer,
    unsigned &i, 
    unsigned &j,
    const GaussianIntensityTable<256> &GI,
    const GaussianSpaceTable<FILTER_SIZE> &GS
) {
    using OutPixelWordT = typename DT<PIXEL_T_OUT, NPPC>::T;

    const unsigned InPixelWidth = DT<PIXEL_T_IN, NPPC>::W / NPPC;
    const unsigned OutPixelWidth = DT<PIXEL_T_OUT, NPPC>::W / NPPC;
    const unsigned TmpPixelWidth = InPixelWidth;
    const unsigned ImgHeight = InImg.get_height();
    const unsigned ImgWidth = InImg.get_width();
    const unsigned ImgIdx = i * (ImgWidth / NPPC) + j;

    using TmpPixelT = ap_uint<TmpPixelWidth>;

    OutPixelWordT OutPixelWord;

    const int FilterRadius = (FILTER_SIZE-1) / (FILTER_SIZE/2); // e.g., 2 if FILTER_SIZE is 5.
    TmpPixelT centerPix = TmpPixelT(LineBuffer.AccessWindow(FilterRadius, FilterRadius, 0));

    ap_uint<32> Wp = 0, Sum = 0;
    HLS_VISION_BILATERAL_LOOP2A:
    for (int OffsetY = -FilterRadius; OffsetY <= FilterRadius; OffsetY++) {
        HLS_VISION_BILATERAL_LOOP2B:
        for (int OffsetX = -FilterRadius; OffsetX <= FilterRadius; OffsetX++) {
            int y = i + OffsetY;
            int x = j * NPPC + OffsetX;
            bool WindowOutOfBounds = (y < 0) | (y >= ImgHeight) | (x < 0) | (x >= ImgWidth);
            int ArrayIdxY = OffsetY + FilterRadius;
            int ArrayIdxX = OffsetX + FilterRadius;
            TmpPixelT WindowPix = WindowOutOfBounds ? 
                TmpPixelT(0) : 
                TmpPixelT(LineBuffer.AccessWindow(ArrayIdxY, ArrayIdxX, 0));
            ap_int<9> d = (WindowPix - centerPix);
            if (d<0) d = -d;
            auto gi = GI.getIntensity(d.to_uint64());
            auto gs = GS.getSpace(ArrayIdxY,ArrayIdxX);
            auto w = gs * gi;
            Sum += WindowPix * w;
            Wp += w;
        }
    }

    Sum = Sum / Wp;
    const TmpPixelT OutMaxPixelVal = TmpPixelT(255);

    TmpPixelT PSum = (TmpPixelT(Sum) > OutMaxPixelVal) ? OutMaxPixelVal : TmpPixelT(Sum);
    OutPixelWord.byte(0, OutPixelWidth) = OutPixelWordT((unsigned int)PSum.to_uint64());

    // Now write to OutImg.
    OutImg.write(OutPixelWord, ImgIdx);
    // We're done with this pixel. Now update the coordinate to the next one.
    if (j < (W / NPPC) - 1) {
        j++;
    } else { // j == WIDTH - 1.
        i++;
        j = 0;
    }
}

template <
    unsigned FILTER_SIZE, 
    PixelType PIXEL_T, 
    unsigned H, 
    unsigned W, 
    StorageType STORAGE_TYPE,
    NumPixelsPerCycle NPPC
> 
void BilateralFilter(
    Img<PIXEL_T, H, W, STORAGE_TYPE, NPPC> &InImg,
    Img<PIXEL_T, H, W, STORAGE_TYPE, NPPC> &OutImg,
    const float sigma_color,
    const float sigma_space
) {
    #pragma HLS memory partition argument(InImg) type(struct_fields)
    #pragma HLS memory partition argument(OutImg) type(struct_fields)


    static_assert(FILTER_SIZE == 5,
        "BilateralFilter only supports filter size of 5.");
                  
    static_assert(DT<PIXEL_T, NPPC>::NumChannels == 1,
        "BilateralFilter only supports 1-channel images.");

    static_assert(NPPC == 1,
        "BilateralFilter currenlty only supports NPPC=1.");

    static_assert(W % NPPC == 0,
        "In BilateralFilter, the width of the frame has to be divisible "
        "by the number of pixels per clock.");


    constexpr unsigned RANGE = 1 << (DT<PIXEL_T, NPPC>::W / NPPC);
    #pragma HLS memory replicate_rom variable(GS.Table) max_replicas(0)
    // #pragma HLS memory impl variable(GS) pack(abi) byte_enable(true)
    static const GaussianSpaceTable<FILTER_SIZE> GS(sigma_space);

    #pragma HLS memory replicate_rom variable(GI.Table) max_replicas(0)
    // #pragma HLS memory impl variable(GI) pack(abi) byte_enable(true)
    static const GaussianIntensityTable<RANGE> GI(sigma_color);

    using InPixelWordT = typename DT<PIXEL_T, NPPC>::T;

    const unsigned ImgHeight = InImg.get_height(); 
    const unsigned ImgWidth = InImg.get_width();

    OutImg.set_height(ImgHeight);
    OutImg.set_width(ImgWidth);

    const unsigned FrameSize = (ImgHeight * ImgWidth) / NPPC;
    const unsigned InPixelWidth = DT<PIXEL_T, NPPC>::W / NPPC;


    LineBuffer<InPixelWordT, W / NPPC, FILTER_SIZE, InPixelWidth, NPPC>
        LineBuffer;
    const unsigned FilterRadius = (FILTER_SIZE-1) / (FILTER_SIZE/2);
    // Before we can process the first pixel word, the LineBuffer needs to be
    // filled for a certain number of pixels first. This number of pixels
    // (denoted `LineBufferPixelFillCount`) is:
    //   `FilterRadius` rows, plus `NPPC + FilterRadius - 1` pixels of the next
    //   row.
    // It's best to draw a picture to see why this is the case. The intuition is
    // that, processing the first pixel word is equivalent to processing the
    // first NPPC pixels. So we need to have enough pixels in the LineBuffer to
    // be able to construct a windows of size `FilterRadius` at the NPPC-th
    // pixel of the first pixel word.
    // Some examples assuming FILTER_SIZE=5:
    // +-------------+-------------------+------+--------------------------+
    // | FILTER_SIZE | FilterRadius      | NPPC | LineBufferPixelFillCount |
    // |             | = FILTER_SIZE / 2 |      |                          |
    // +-------------+-------------------+------+--------------------------+
    // | 3           | 1                 | 1    | 1 row  + 1 pixel         |
    // | 3           | 1                 | 4    | 1 row  + 4 pixels        |
    // | 5           | 2                 | 1    | 2 rows + 2 pixels        |
    // | 5           | 2                 | 4    | 2 rows + 5 pixels        |
    // +-------------+-------------------+------+--------------------------+
    //
    // We have:
    //   LineBufferPixelFillCount
    // = FilterRadius * ImgWidth + NPPC + FilterRadius - 1
    // = FilterRadius * (ImgWidth + 1) + NPPC - 1
    //
    //   LineBufferPixelWordFillCount = ceil(LineBufferPixelFillCount / NPPC)
    const unsigned LineBufferPixelFillCount =
        FilterRadius * (ImgWidth + 1) + NPPC - 1;
    unsigned LineBufferPixelWordFillCount = LineBufferPixelFillCount / NPPC;
    // Note that C++ unsigned division rounds down, so we can do this to
    // implement ceil():
    if (LineBufferPixelFillCount % NPPC != 0) {
        LineBufferPixelWordFillCount += 1;
    }

    HLS_VISION_BILATERAL_FILL_BUFFER_LOOP:
    #pragma HLS loop pipeline
    for (unsigned Count = 0; Count < LineBufferPixelWordFillCount; Count++) {
        auto InPixelWord = InImg.read(Count);
        LineBuffer.ShiftInPixel(InPixelWord);
    }

    // i and j are the row and col indices of the current pixel word being
    // processed. They'll be incremented by BilateralProcess().
    unsigned i = 0, j = 0;

    // 2. Fill LineBuffer and process (steady state)
    HLS_VISION_BILATERAL_FILL_AND_PROCESS_LOOP:
    #pragma HLS loop pipeline
    for (unsigned Count = LineBufferPixelWordFillCount; Count < FrameSize;
         Count++) {
        auto InPixelWord = InImg.read(Count);
        LineBuffer.ShiftInPixel(InPixelWord);
        BilateralProcess<FILTER_SIZE>(InImg, OutImg, LineBuffer, i, j, GI, GS);
    }

    // 3. Process only (flush out). The input to LineBuffer is 0.
    HLS_VISION_BILATERAL_FLUSH_LOOP:
    #pragma HLS loop pipeline
    for (unsigned Count = FrameSize;
        Count < FrameSize + LineBufferPixelWordFillCount; Count++) {
        LineBuffer.ShiftInPixel(0);
        BilateralProcess<FILTER_SIZE>(InImg, OutImg, LineBuffer, i, j, GI, GS);
        }
}

} // End of namespace vision.
} // End of namespace hls.
