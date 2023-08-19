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
// MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.
// TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP’S TOTAL LIABILITY ON ALL
// CLAIMS LATED TO THE SOFTWARE WILL NOT EXCEED AMOUNT OF FEES, IF ANY, YOU PAID
// DIRECTLY TO MICROCHIP FOR THIS SOFTWARE. MICROCHIP OFFERS NO SUPPORT FOR THE
// SOFTWARE. YOU MAY CONTACT MICROCHIP AT
// https://www.microchip.com/en-us/support-and-training/design-help/client-support-services
// TO INQUIRE ABOUT SUPPORT SERVICES AND APPLICABLE FEES, IF AVAILABLE.

#ifndef __SHLS_VISION_SOBEL_HPP__
#define __SHLS_VISION_SOBEL_HPP__

#include <type_traits>

#include "../common/common.hpp"
#include "../common/utils.hpp"
#include <hls/ap_fixpt.hpp>

namespace hls {
namespace vision {

// Only support 3x3 for now for quick prototyping.
// Also assume [0, 255] pixel range.
template <unsigned FILTER_SIZE, PixelType PIXEL_T_IN, PixelType PIXEL_T_OUT,
          unsigned H, unsigned W, StorageType STORAGE_IN,
          StorageType STORAGE_OUT, NumPixelsPerCycle NPPC>
void SobelProcess(
    vision::Img<PIXEL_T_IN, H, W, STORAGE_IN, NPPC> &InImg,
    vision::Img<PIXEL_T_OUT, H, W, STORAGE_OUT, NPPC> &OutImg,
    LineBuffer<typename DT<PIXEL_T_IN, NPPC>::T, W / NPPC, FILTER_SIZE,
               DT<PIXEL_T_IN, NPPC>::W / NPPC, unsigned(NPPC)> &LineBuffer,
    unsigned &i, unsigned &j) {

    using InPixelWordT = typename DT<PIXEL_T_IN, NPPC>::T;
    using OutPixelWordT = typename DT<PIXEL_T_OUT, NPPC>::T;

    // For all intermediate values of calculations, let's use an ap_int that has
    // a slightly larger width than max(InPixelWidth, OutPixelWidth)
    const unsigned InPixelWidth = DT<PIXEL_T_IN, NPPC>::W / NPPC,
                   OutPixelWidth = DT<PIXEL_T_OUT, NPPC>::W / NPPC;
    const unsigned TmpPixelWidth = InPixelWidth + 9;
    using TmpPixelT = ap_int<TmpPixelWidth>;

    const unsigned ImgHeight = InImg.get_height(), ImgWidth = InImg.get_width();
    const unsigned ImgIdx = i * (ImgWidth / NPPC) + j;
    static const TmpPixelT KernelX[FILTER_SIZE][FILTER_SIZE] = {
        {-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
    static const TmpPixelT KernelY[FILTER_SIZE][FILTER_SIZE] = {
        {1, 2, 1}, {0, 0, 0}, {-1, -2, -1}};
    OutPixelWordT OutPixelWord;

    // Now do the convolution at the current point:
    // - OutGx = dot_product(KernelX, Window)
    // - OutGy = dot_product(KernelY, Window)
    const int FilterRadius = FILTER_SIZE / 2; // e.g., 1 if FILTER_SIZE is 3.
    for (int k = 0; k < NPPC; k++) {
        // Initialize the input window from LineBuffer.window. If the coordinate
        // is out-of-bound, set the value to 0 (i.e., zero-padding).
        TmpPixelT Window[FILTER_SIZE][FILTER_SIZE];
        for (int OffsetY = -FilterRadius; OffsetY <= FilterRadius; OffsetY++) {
            for (int OffsetX = -FilterRadius; OffsetX <= FilterRadius;
                 OffsetX++) {
                int y = i + OffsetY, x = j * NPPC + k + OffsetX;
                bool WindowOutOfBounds =
                    (y < 0) | (y >= ImgHeight) | (x < 0) | (x >= ImgWidth);
                // Array indices start from 0 so we need to "recalibrate" the
                // index by adding `FilterRadius`. E.g., for 3x3 filter, the
                // offset is from -1 to 1. To turn it into 0-based index, we
                // need to add FilterRadius = 1.
                int ArrayIdxY = OffsetY + FilterRadius,
                    ArrayIdxX = OffsetX + FilterRadius;
                Window[ArrayIdxY][ArrayIdxX] =
                    WindowOutOfBounds ? TmpPixelT(0)
                                      : TmpPixelT(LineBuffer.AccessWindow(
                                            ArrayIdxY, ArrayIdxX, k));
            }
        }

        // Apply the convolution.
        TmpPixelT GxSum = 0, GySum = 0;
        for (int OffsetY = -FilterRadius; OffsetY <= FilterRadius; OffsetY++) {
            for (int OffsetX = -FilterRadius; OffsetX <= FilterRadius;
                 OffsetX++) {
                // "Recalibrate" the array index to be 0-based.
                int ArrayIdxY = OffsetY + FilterRadius,
                    ArrayIdxX = OffsetX + FilterRadius;
                // Get the pixel in "receptive field" from LineBuffer's window.
                auto Pixel = Window[ArrayIdxY][ArrayIdxX];
                GxSum += Pixel * KernelX[ArrayIdxY][ArrayIdxX];
                GySum += Pixel * KernelY[ArrayIdxY][ArrayIdxX];
            }
        }

        /**************************** OutImg *********************************/
        // OutImg = |OutGx| + |OutGy|
        TmpPixelT GxSumAbs = (GxSum < 0) ? -GxSum : GxSum,
                  GySumAbs = (GySum < 0) ? -GySum : GySum;
        TmpPixelT Sum = GxSumAbs + GySumAbs;
        // TODO T: Need to figure out how to do clamp the value e.g., to support
        // signed vs. unsigned, what's the min and max value to clamp.
        // Assume [0, 255] for now.
        // const TmpPixelT OutMaxPixelVal = (TmpPixelT(1) << OutPixelWidth) - 1;
        const TmpPixelT OutMaxPixelVal = TmpPixelT(255);
        Sum = (Sum > OutMaxPixelVal) ? OutMaxPixelVal : Sum;
        OutPixelWord.byte(k, OutPixelWidth) = Sum;
        /*********************************************************************/
    }

    // Now write to OutImg.
    OutImg.write(OutPixelWord, ImgIdx);
    // We're done with this pixel. Now update the coordinate to the next one.
    if (j < W / NPPC - 1) {
        j++;
    } else { // j == WIDTH - 1.
        i++;
        j = 0;
    }
}

// TODO T:
// - Add support for FILTER_SIZE other than 3.
// - Add support for multiple channels.
// - Add support for clamp logic. Right now we assume [0, 255] for all pixel
//   types which is not correct, but we need to be really careful with the clamp
//   logic since this function can be called both by itself or by Canny().
template <unsigned FILTER_SIZE = 3, PixelType PIXEL_T_IN, PixelType PIXEL_T_OUT,
          unsigned H, unsigned W, StorageType STORAGE_IN = StorageType::FIFO,
          StorageType STORAGE_OUT = StorageType::FIFO,
          NumPixelsPerCycle NPPC = NPPC_1>
void Sobel(vision::Img<PIXEL_T_IN, H, W, STORAGE_IN, NPPC> &InImg,
           vision::Img<PIXEL_T_OUT, H, W, STORAGE_OUT, NPPC> &OutImg) {
#pragma HLS memory partition argument(InImg) type(struct_fields)
#pragma HLS memory partition argument(OutImg) type(struct_fields)

    static_assert(FILTER_SIZE == 3, "Sobel only supports filter size of 3.");
    static_assert(DT<PIXEL_T_IN, NPPC>::NumChannels == 1 &&
                      DT<PIXEL_T_OUT, NPPC>::NumChannels == 1,
                  "Sobel only supports 1-channel images.");
    static_assert(W % NPPC == 0,
                  "In Sobel, the width of the frame has to be divisible by the "
                  "number of pixels per clock.");
    using InPixelWordT = typename DT<PIXEL_T_IN, NPPC>::T;
    using OutPixelWordT = typename DT<PIXEL_T_OUT, NPPC>::T;

    const unsigned ImgHeight = InImg.get_height(), ImgWidth = InImg.get_width();
    OutImg.set_height(ImgHeight);
    OutImg.set_width(ImgWidth);

    const unsigned FrameSize = (ImgHeight * ImgWidth) / NPPC;
    const unsigned InPixelWidth = DT<PIXEL_T_IN, NPPC>::W / NPPC;
    LineBuffer<InPixelWordT, W / NPPC, FILTER_SIZE, InPixelWidth, NPPC>
        LineBuffer;
    const unsigned FilterRadius = FILTER_SIZE / 2;
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
    // Some examples:
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

// 1. Fill LineBuffer only
#pragma HLS loop pipeline
    for (unsigned Count = 0; Count < LineBufferPixelWordFillCount; Count++) {
        auto InPixelWord = InImg.read(Count);
        LineBuffer.ShiftInPixel(InPixelWord);
    }

    // i and j are the row and col indices of the current pixel word being
    // processed. They'll be incremented by SobelProcess().
    unsigned i = 0, j = 0;
// 2. Fill LineBuffer and process (steady state)
#pragma HLS loop pipeline
    for (unsigned Count = LineBufferPixelWordFillCount; Count < FrameSize;
         Count++) {
        auto InPixelWord = InImg.read(Count);
        LineBuffer.ShiftInPixel(InPixelWord);
        SobelProcess<FILTER_SIZE>(InImg, OutImg, LineBuffer, i, j);
    }

// 3. Process only (flush out). The input to LineBuffer is 0.
#pragma HLS loop pipeline
    for (unsigned Count = FrameSize;
         Count < FrameSize + LineBufferPixelWordFillCount; Count++) {
        LineBuffer.ShiftInPixel(0);
        SobelProcess<FILTER_SIZE>(InImg, OutImg, LineBuffer, i, j);
    }
}

} // End of namespace vision.
} // End of namespace hls.

#endif
