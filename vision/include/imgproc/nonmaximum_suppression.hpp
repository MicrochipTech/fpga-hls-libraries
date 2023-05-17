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

#ifndef __SHLS_VISION_NONMAXIMUM_SUPPRESSION_HPP__
#define __SHLS_VISION_NONMAXIMUM_SUPPRESSION_HPP__

#include <type_traits>

#include "../common/common.hpp"
#include "../common/utils.hpp"

namespace hls {
namespace vision {

template <PixelType PIXEL_T_IN, PixelType PIXEL_T_OUT, unsigned H, unsigned W,
          StorageType STORAGE_IN, StorageType STORAGE_OUT,
          NumPixelsPerCycle NPPC_IN, NumPixelsPerCycle NPPC_OUT>
void NonMaxSuppProcess(
    vision::Img<PIXEL_T_IN, H, W, STORAGE_IN, NPPC_IN> &InImg,
    vision::Img<PIXEL_T_IN, H, W, STORAGE_IN, NPPC_IN> &InDirection,
    vision::Img<PIXEL_T_OUT, H, W, STORAGE_OUT, NPPC_OUT> &OutImg,
    LineBuffer<typename DT<PIXEL_T_IN, NPPC_IN>::T, W / NPPC_IN, 3,
               DT<PIXEL_T_IN, NPPC_IN>::W / NPPC_IN, unsigned(NPPC_IN)>
        &LineBuffer,
    unsigned &i, unsigned &j) {

    using InPixelWordT = typename DT<PIXEL_T_IN, NPPC_IN>::T;
    using OutPixelWordT = typename DT<PIXEL_T_OUT, NPPC_OUT>::T;

    // For all intermediate values of calculations, let's use an ap_int that has
    // a slightly larger width than max(InPixelWidth, OutPixelWidth)
    const unsigned InPixelWidth = DT<PIXEL_T_IN, NPPC_IN>::W / NPPC_IN,
                   OutPixelWidth = DT<PIXEL_T_OUT, NPPC_OUT>::W / NPPC_OUT;
    const unsigned TmpPixelWidth =
        (InPixelWidth > OutPixelWidth ? InPixelWidth : OutPixelWidth) + 4;
    using TmpPixelT = ap_int<TmpPixelWidth>;

    const unsigned ImgHeight = InImg.get_height(), ImgWidth = InImg.get_width();
    const unsigned ImgIdx = i * (ImgWidth / NPPC_IN) + j;
    OutPixelWordT OutPixelWord;
    auto InDirectionPixelWord = InDirection.read(ImgIdx);

    // Now do the non-max suppression at the current point:
    for (int k = 0; k < NPPC_IN; k++) {
        // Initialize the input window from LineBuffer.window. If the coordinate
        // is out-of-bound, set the value to 0 (i.e., zero-padding).
        TmpPixelT Window[3][3];
        for (int OffsetY = -1; OffsetY <= 1; OffsetY++) {
            for (int OffsetX = -1; OffsetX <= 1; OffsetX++) {
                int y = i + OffsetY, x = j * NPPC_IN + k + OffsetX;
                bool WindowOutOfBounds =
                    (y < 0) | (y >= ImgHeight) | (x < 0) | (x >= ImgWidth);
                // Array indices start from 0 so we need to "recalibrate" the
                // index by adding `FilterRadius`. E.g., for 3x3 filter, the
                // offset is from -1 to 1. To turn it into 0-based index, we
                // need to add 1.
                int ArrayIdxY = OffsetY + 1, ArrayIdxX = OffsetX + 1;
                Window[ArrayIdxY][ArrayIdxX] =
                    WindowOutOfBounds ? TmpPixelT(0)
                                      : TmpPixelT(LineBuffer.AccessWindow(
                                            ArrayIdxY, ArrayIdxX, k));
            }
        }

        // Non-maximum suppression algorithm:
        // Each pixel has a corresponding gradient direction, which is assumed
        // to have been calculated by vision::Sobel() beforehand and given to
        // this function as an input argument. The direction of the gradient is
        // defined as followed: (Note: The angle is taken from the right (0 at
        // horizontal right), and
        //  goes counter-clockwise. It's also rounded to the nearest 45 degrees)
        //
        //          135     90     45
        //            \     |     /
        //              \   |   /
        //                \ | /
        //          0 ------ ------ 0
        //                / | \
        //              /   |   \
        //            /     |     \
        //          45     90     135
        //
        // Example: If the angle is in:
        //  [-22.5,  22.5] or [157.5, 202.5]: direction =   0 (horizontal)
        //  [ 22.5,  67.5] or [202.5, 247.5]: direction =  45 (NW-SE diagonal)
        //  [ 67.5, 112.5] or [247.5, 292.5]: direction =  90 (vertical)
        //  [112.5, 157.5] or [292.5, 337.5]: direction = 135 (NE-SW diagonal)
        //
        // Note that the direction of the gradient is always orthogonal to the
        // direction of the edge. In other word, the direction of the gradient
        // is the direction of the "thickness" of the edge.
        //
        // Non-maximum suppression aims to make the edges thinner. The algorithm
        // looks at the current pixel, then compare it with the 2 adjacent
        // pixels in the gradient direction. If the current pixel is smaller
        // than either of those 2 adjacent pixels, then it's suppressed (set to
        // 0). The result is a thinner edge.
        //
        // Note that our LineBuffer window (and our image representation) follow
        // the following index conventions:
        // - y-index first, x-index second
        // - x-direction is right-ward, y-direction is downward.
        // For example: Window[-1][1] means the top-right pixel
        //
        // +-------+-------+-------+    -->x
        // | -1,-1 | -1, 0 | -1, 1 |   |
        // +-------+-------+-------+   y
        // |  0,-1 |  0, 0 |  0, 1 |
        // +-------+-------+-------+
        // |  1,-1 |  1, 0 |  1, 1 |
        // +-------+-------+-------+
        //
        // For example, the 2 neighboring pixels in the 45 direction is [-1, 1]
        // and [1, -1].

        unsigned Center = 1;
        TmpPixelT InPixel = Window[Center][Center];
        TmpPixelT AdjPixel1, AdjPixel2;
        switch (InDirectionPixelWord.byte(k, InPixelWidth)) {
        case 0:
            AdjPixel1 = Window[Center + 0][Center - 1];
            AdjPixel2 = Window[Center + 0][Center + 1];
            break;
        case 45:
            AdjPixel1 = Window[Center - 1][Center + 1];
            AdjPixel2 = Window[Center + 1][Center - 1];
            break;
        case 90:
            AdjPixel1 = Window[Center - 1][Center + 0];
            AdjPixel2 = Window[Center + 1][Center + 0];
            break;
        case 135:
            AdjPixel1 = Window[Center - 1][Center - 1];
            AdjPixel2 = Window[Center + 1][Center + 1];
            break;
        default:
            // TODO T: How to crash here?
            break;
        }
        TmpPixelT OutPixel = InPixel;
        OutPixel = (OutPixel >= AdjPixel1) ? OutPixel : TmpPixelT(0);
        OutPixel = (OutPixel >= AdjPixel2) ? OutPixel : TmpPixelT(0);
        OutPixelWord.byte(k, OutPixelWidth) = OutPixel;
        /*********************************************************************/
    }

    // Now write to OutImg.
    OutImg.write(OutPixelWord, ImgIdx);
    // We're done with this pixel. Now update the coordinate to the next one.
    if (j < W / NPPC_IN - 1) {
        j++;
    } else { // j == WIDTH - 1.
        i++;
        j = 0;
    }
}

// TODO T:
// - Add support for multiple channels.
// - Add support for other pixel-per-cycle.
template <PixelType PIXEL_T_IN, PixelType PIXEL_T_OUT, unsigned H, unsigned W,
          StorageType STORAGE_IN = StorageType::FIFO,
          StorageType STORAGE_OUT = StorageType::FIFO,
          NumPixelsPerCycle NPPC_IN = NPPC_1,
          NumPixelsPerCycle NPPC_OUT = NPPC_1>
void NonMaximumSuppression(
    vision::Img<PIXEL_T_IN, H, W, STORAGE_IN, NPPC_IN> &InImg,
    vision::Img<PIXEL_T_IN, H, W, STORAGE_IN, NPPC_IN> &InDirection,
    vision::Img<PIXEL_T_OUT, H, W, STORAGE_OUT, NPPC_OUT> &OutImg) {
#pragma HLS memory partition argument(InImg) type(struct_fields)
#pragma HLS memory partition argument(InDirection) type(struct_fields)
#pragma HLS memory partition argument(OutImg) type(struct_fields)

    static_assert(DT<PIXEL_T_IN, NPPC_IN>::NumChannels == 1 &&
                      DT<PIXEL_T_OUT, NPPC_OUT>::NumChannels == 1,
                  "NonMaximumSuppression only supports 1-channel images.");
    static_assert(NPPC_IN == NPPC_OUT,
                  "NonMaximumSuppression only supports having the same number "
                  "of pixels per clock in input and output data.");
    static_assert(W % NPPC_IN == 0,
                  "In NonMaximumSuppression, the width of the frame has to be "
                  "divisible by the number of pixels per clock.");
    using InPixelWordT = typename DT<PIXEL_T_IN, NPPC_IN>::T;
    using OutPixelWordT = typename DT<PIXEL_T_OUT, NPPC_OUT>::T;

    const unsigned ImgHeight = InImg.get_height(), ImgWidth = InImg.get_width();
    OutImg.set_height(ImgHeight);
    OutImg.set_width(ImgWidth);

    const unsigned FrameSize = (ImgHeight * ImgWidth) / NPPC_IN;
    const unsigned InPixelWidth = DT<PIXEL_T_IN, NPPC_IN>::W / NPPC_IN;
    LineBuffer<InPixelWordT, W / NPPC_IN, 3, InPixelWidth, NPPC_IN> LineBuffer;
    // The LineBuffer needs to be filled for 1 row, plus 1 pixel.
    const unsigned LineBufferPixelWordFillCount = ImgWidth / NPPC_IN + 1;

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
        NonMaxSuppProcess(InImg, InDirection, OutImg, LineBuffer, i, j);
    }

    // 3. Process only (flush out). The input to LineBuffer is 0.
#pragma HLS loop pipeline
    for (unsigned Count = FrameSize;
         Count < FrameSize + LineBufferPixelWordFillCount; Count++) {
        LineBuffer.ShiftInPixel(0);
        NonMaxSuppProcess(InImg, InDirection, OutImg, LineBuffer, i, j);
    }
}

} // End of namespace vision.
} // End of namespace hls.

#endif
