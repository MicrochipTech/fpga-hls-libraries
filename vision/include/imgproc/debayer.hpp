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

#ifndef __SHLS_VISION_DEBAYER_HPP__
#define __SHLS_VISION_DEBAYER_HPP__

#include "../common/common.hpp"
#include "../common/line_buffer.hpp"
#include <cmath>
#include <type_traits>

namespace hls {
namespace vision {

template <PixelType PIXEL_T_IN, PixelType PIXEL_T_OUT, unsigned H, unsigned W,
          StorageType STORAGE_IN, StorageType STORAGE_OUT,
          NumPixelsPerCycle NPPC>
void DeBayer_3x3(vision::Img<PIXEL_T_IN, H, W, STORAGE_IN, NPPC> &InImg,
                 vision::Img<PIXEL_T_OUT, H, W, STORAGE_OUT, NPPC> &OutImg,
                 ap_uint<2> BayerFormat = 2) {
    using DATA_T_IN = typename DT<PIXEL_T_IN, NPPC>::T;
    using DATA_T_OUT = typename DT<PIXEL_T_OUT, NPPC>::T;
    DATA_T_OUT RgbData;
    const unsigned WindowSize = 3;
    const unsigned ChannelPixelWidth =
        DT<PIXEL_T_IN, NPPC>::PerChannelPixelWidth;

    // This function is based on the Bayer Interpolation IP core and uses the
    // same formulas to calculate pixel values
    // https://www.microchip.com/en-us/products/fpgas-and-plds/ip-core-tools/bayer-interpolation

    LineBuffer<DATA_T_IN, W / NPPC, WindowSize, ChannelPixelWidth, NPPC>
        LineBuffer;

    // There will be no output until enough pixels have been stored in line
    // buffer (LineBufferFillCount)
    const unsigned LineBufferFillCount = InImg.get_width() / NPPC + 1;
    // image frame size (height * columns (width / pixels per clock))
    const unsigned FrameSize = InImg.get_height() * InImg.get_width() / NPPC;

    // used to determine if the current pixel is on an image boundary, and which
    // boundary it is on
    ap_uint<4> BoundaryHcVc;
    // horizontal and vertical counts used to determine pixel color based on
    // location and bayer format
    ap_uint<2> VcHc;
    ap_uint<1> Hc, Vc;
    // i: row index of the current pixel
    // j: index of the current column (1 column can have many pixels based on
    // NPPC)
    // k: index of the pixel within the column l: horizontal location of
    // the current pixel in image line
    ap_uint<13> i = 0, j = 0, l;
    int k = 0;
#pragma HLS loop pipeline
    for (int Count = 0; Count < FrameSize + LineBufferFillCount; Count++) {

        // No more reads after receiving the whole image, the extra iteration is
        // flushing out the final outputs.
        auto InputPixel = 0;
        if (Count < InImg.get_height() * InImg.get_width() / NPPC)
            InputPixel = InImg.read(Count);

        LineBuffer.ShiftInPixel(InputPixel);

        // No output before line buffer is filled with 1 row + 1 pixel. This
        // requirement is based on the way the algorithm works.
        if (Count < LineBufferFillCount)
            continue;

        for (k = 0; k < NPPC; k++) {
            // DeBayer

            // The exact pixel location in image
            l = j * NPPC + k;
            // BoundaryHcVc determines where in the image the pixel we re
            // processing is located at (at the edges or inside).
            //  0 : first row
            //  1 : first column
            //  2 : first column
            //  3 : last row
            //  4 : normal pixel inside the image (not located on any boundary)
            if (i == 0) {
                BoundaryHcVc = 0;
            } else if (l == 0) {
                BoundaryHcVc = 1;
            } else if (l == W - 1) {
                BoundaryHcVc = 2;
            } else if (i == H - 1) {
                BoundaryHcVc = 3;
            } else {
                BoundaryHcVc = 4;
            }

            // BayerFormat:
            //      0: RGGB
            //      1: GRBG
            //      2: GBRG
            //      3: BGGR
            //
            // We use Hc (horizontal cound), Vc (vertical count), and VcHc to
            // determine the color of the pixel we are processing (either red,
            // green or blue in the  bayer image). This depends on the bayer
            // format and location in the image.
            if (BayerFormat == 0 || BayerFormat == 2) {
                Hc = l[0];
            } else {
                Hc = ~l[0];
            }
            if (BayerFormat == 0 || BayerFormat == 1) {
                Vc = i[0];
            } else {
                Vc = ~i[0];
            }

            VcHc = 2 * Vc + Hc;

            switch (BoundaryHcVc) {
            // first row
            case 0:
                if (Hc == 0) {
                    // red value
                    RgbData.byte(3 * k + 0, ChannelPixelWidth) =
                        LineBuffer.AccessWindow(1, 1, k);
                    // green value
                    RgbData.byte(3 * k + 1, ChannelPixelWidth) =
                        (LineBuffer.AccessWindow(1, 0, k) +
                         LineBuffer.AccessWindow(1, 2, k)) >>
                        1;
                    // blue value
                    RgbData.byte(3 * k + 2, ChannelPixelWidth) =
                        (LineBuffer.AccessWindow(2, 0, k) +
                         LineBuffer.AccessWindow(2, 2, k)) >>
                        1;
                } else {
                    // red value
                    RgbData.byte(3 * k + 0, ChannelPixelWidth) =
                        (LineBuffer.AccessWindow(1, 0, k) +
                         LineBuffer.AccessWindow(1, 2, k)) >>
                        1;
                    // green value
                    RgbData.byte(3 * k + 1, ChannelPixelWidth) =
                        LineBuffer.AccessWindow(1, 1, k);
                    // blue value
                    RgbData.byte(3 * k + 2, ChannelPixelWidth) =
                        LineBuffer.AccessWindow(2, 1, k);
                }
                break;
            // first column
            case 1:
                if (Vc == 0) {
                    // red value
                    RgbData.byte(3 * k + 0, ChannelPixelWidth) =
                        LineBuffer.AccessWindow(1, 1, k);
                    // green value
                    RgbData.byte(3 * k + 1, ChannelPixelWidth) =
                        (LineBuffer.AccessWindow(1, 2, k) +
                         LineBuffer.AccessWindow(0, 1, k)) >>
                        1;
                    // blue value
                    RgbData.byte(3 * k + 2, ChannelPixelWidth) =
                        (LineBuffer.AccessWindow(2, 0, k) +
                         LineBuffer.AccessWindow(2, 2, k)) >>
                        1;
                } else {
                    // red value
                    RgbData.byte(3 * k + 0, ChannelPixelWidth) =
                        (LineBuffer.AccessWindow(0, 1, k) +
                         LineBuffer.AccessWindow(2, 1, k)) >>
                        1;
                    // green value
                    RgbData.byte(3 * k + 1, ChannelPixelWidth) =
                        LineBuffer.AccessWindow(1, 1, k);
                    // blue value
                    RgbData.byte(3 * k + 2, ChannelPixelWidth) =
                        LineBuffer.AccessWindow(1, 2, k);
                }
                break;
            // last column
            case 2:
                if (Vc == 1) {
                    // red value
                    RgbData.byte(3 * k + 0, ChannelPixelWidth) =
                        (LineBuffer.AccessWindow(0, 0, k) +
                         LineBuffer.AccessWindow(2, 0, k)) >>
                        1;
                    // green value
                    RgbData.byte(3 * k + 1, ChannelPixelWidth) =
                        (LineBuffer.AccessWindow(0, 1, k) +
                         LineBuffer.AccessWindow(1, 0, k)) >>
                        1;
                    // blue value
                    RgbData.byte(3 * k + 2, ChannelPixelWidth) =
                        LineBuffer.AccessWindow(1, 1, k);
                } else {
                    // red value
                    RgbData.byte(3 * k + 0, ChannelPixelWidth) =
                        LineBuffer.AccessWindow(1, 0, k);
                    // green value
                    RgbData.byte(3 * k + 1, ChannelPixelWidth) =
                        LineBuffer.AccessWindow(1, 1, k);
                    // blue value
                    RgbData.byte(3 * k + 2, ChannelPixelWidth) =
                        (LineBuffer.AccessWindow(2, 1, k) +
                         LineBuffer.AccessWindow(0, 1, k)) >>
                        1;
                }
                break;
            // last row
            case 3:
                if (Hc == 0) {
                    // red value
                    RgbData.byte(3 * k + 0, ChannelPixelWidth) =
                        LineBuffer.AccessWindow(0, 1, k);
                    // green value
                    RgbData.byte(3 * k + 1, ChannelPixelWidth) =
                        LineBuffer.AccessWindow(1, 1, k);
                    // blue value
                    RgbData.byte(3 * k + 2, ChannelPixelWidth) =
                        (LineBuffer.AccessWindow(1, 2, k) +
                         LineBuffer.AccessWindow(1, 0, k)) >>
                        1;
                } else {
                    // red value
                    RgbData.byte(3 * k + 0, ChannelPixelWidth) =
                        (LineBuffer.AccessWindow(0, 0, k) +
                         LineBuffer.AccessWindow(0, 2, k)) >>
                        1;
                    // green value
                    RgbData.byte(3 * k + 1, ChannelPixelWidth) =
                        (LineBuffer.AccessWindow(1, 2, k) +
                         LineBuffer.AccessWindow(1, 0, k)) >>
                        1;
                    // blue value
                    RgbData.byte(3 * k + 2, ChannelPixelWidth) =
                        LineBuffer.AccessWindow(1, 1, k);
                }
                break;
            // normal pixel (not located on any boundary)
            case 4:
                if (VcHc == 3) {
                    // red value
                    RgbData.byte(3 * k + 0, ChannelPixelWidth) =
                        (LineBuffer.AccessWindow(0, 0, k) +
                         LineBuffer.AccessWindow(0, 2, k) +
                         LineBuffer.AccessWindow(2, 0, k) +
                         LineBuffer.AccessWindow(2, 2, k)) >>
                        2;
                    // green value
                    RgbData.byte(3 * k + 1, ChannelPixelWidth) =
                        (LineBuffer.AccessWindow(0, 1, k) +
                         LineBuffer.AccessWindow(1, 0, k) +
                         LineBuffer.AccessWindow(1, 2, k) +
                         LineBuffer.AccessWindow(2, 1, k)) >>
                        2;
                    // blue value
                    RgbData.byte(3 * k + 2, ChannelPixelWidth) =
                        LineBuffer.AccessWindow(1, 1, k);
                } else if (VcHc == 2) {
                    // red value
                    RgbData.byte(3 * k + 0, ChannelPixelWidth) =
                        (LineBuffer.AccessWindow(0, 1, k) +
                         LineBuffer.AccessWindow(2, 1, k)) >>
                        1;
                    // green value
                    RgbData.byte(3 * k + 1, ChannelPixelWidth) =
                        LineBuffer.AccessWindow(1, 1, k);
                    // blue value
                    RgbData.byte(3 * k + 2, ChannelPixelWidth) =
                        (LineBuffer.AccessWindow(1, 0, k) +
                         LineBuffer.AccessWindow(1, 2, k)) >>
                        1;
                } else if (VcHc == 1) {
                    // red value
                    RgbData.byte(3 * k + 0, ChannelPixelWidth) =
                        (LineBuffer.AccessWindow(1, 0, k) +
                         LineBuffer.AccessWindow(1, 2, k)) >>
                        1;
                    // green value
                    RgbData.byte(3 * k + 1, ChannelPixelWidth) =
                        LineBuffer.AccessWindow(1, 1, k);
                    // blue value
                    RgbData.byte(3 * k + 2, ChannelPixelWidth) =
                        (LineBuffer.AccessWindow(0, 1, k) +
                         LineBuffer.AccessWindow(2, 1, k)) >>
                        1;
                } else {
                    // red value
                    RgbData.byte(3 * k + 0, ChannelPixelWidth) =
                        LineBuffer.AccessWindow(1, 1, k);
                    // green value
                    RgbData.byte(3 * k + 1, ChannelPixelWidth) =
                        (LineBuffer.AccessWindow(0, 1, k) +
                         LineBuffer.AccessWindow(1, 0, k) +
                         LineBuffer.AccessWindow(1, 2, k) +
                         LineBuffer.AccessWindow(2, 1, k)) >>
                        2;
                    // blue value
                    RgbData.byte(3 * k + 2, ChannelPixelWidth) =
                        (LineBuffer.AccessWindow(0, 0, k) +
                         LineBuffer.AccessWindow(0, 2, k) +
                         LineBuffer.AccessWindow(2, 0, k) +
                         LineBuffer.AccessWindow(2, 2, k)) >>
                        2;
                }
                break;

            default:
                // red value
                RgbData.byte(3 * k + 0, ChannelPixelWidth) = 0;
                // green value
                RgbData.byte(3 * k + 1, ChannelPixelWidth) = 0;
                // blue value
                RgbData.byte(3 * k + 2, ChannelPixelWidth) = 0;
                break;
            }
        }
        OutImg.write(RgbData, Count - LineBufferFillCount);

        // Keep track of row/column of image.
        if (j < W / NPPC - 1) {
            j++;
        } else {
            i++;
            j = 0;
        }
    }
}

/**
 * This function converts Bayer data (1 channel) to RGB data (3 channels).
 * The user can use BayerFormat argument to determine the incoming stream's
 * bayer format:
 *      0: RGGB
 *      1: GRBG
 *      2: GBRG
 *      3: BGGR
 */
template <PixelType PIXEL_T_IN, PixelType PIXEL_T_OUT, unsigned H, unsigned W,
          StorageType STORAGE_IN, StorageType STORAGE_OUT,
          NumPixelsPerCycle NPPC = NPPC_1>
void DeBayer(vision::Img<PIXEL_T_IN, H, W, STORAGE_IN, NPPC> &InImg,
             vision::Img<PIXEL_T_OUT, H, W, STORAGE_OUT, NPPC> &OutImg,
             ap_uint<2> BayerFormat = 2) {
#pragma HLS memory partition argument(InImg) type(struct_fields)
#pragma HLS memory partition argument(OutImg) type(struct_fields)

    static_assert(DT<PIXEL_T_IN, NPPC>::NumChannels == 1 &&
                      DT<PIXEL_T_OUT, NPPC>::NumChannels == 3 &&
                      (NPPC == NPPC_1 || NPPC == NPPC_4),
                  "DeBayer only supports number of pixels per cycle of 1 or 4. "
                  "Input data should have only 1 channel and output should "
                  "only be 3 channels.");
    DeBayer_3x3<PIXEL_T_IN, PIXEL_T_OUT, H, W, STORAGE_IN, STORAGE_OUT, NPPC>(
        InImg, OutImg, BayerFormat);
}

/**
 * Converts RGB to RGGB bayer format (BayerFormat 0 in DeBayer).
 */
template <PixelType PIXEL_T_IN, PixelType PIXEL_T_OUT, unsigned H, unsigned W,
          StorageType STORAGE_IN = FIFO, StorageType STORAGE_OUT = FIFO,
          NumPixelsPerCycle NPPC = NPPC_1>
void RGB2Bayer(vision::Img<PIXEL_T_IN, H, W, STORAGE_IN, NPPC> &InImg,
               vision::Img<PIXEL_T_OUT, H, W, STORAGE_OUT, NPPC> &OutImg) {
    static_assert(
        DT<PIXEL_T_IN, NPPC>::NumChannels == 3 &&
            DT<PIXEL_T_OUT, NPPC>::NumChannels == 1 &&
            (NPPC == NPPC_1 || NPPC == NPPC_4),
        "RGB2Bayer only supports number of pixels per cycle of 1 or 4"
        "Input data should have 3 channels and output should be only 1"
        "channel.");
    int ImgHeight = OutImg.get_height();
    int ImgWidth = OutImg.get_width();
    int ImgColumns = ImgWidth / NPPC;
    int ImgIndex = 0;
    int ColorColumnWidth =
        ImgColumns / DT<PIXEL_T_IN, NPPC>::PerChannelPixelWidth;

    typename DT<PIXEL_T_IN, NPPC>::T RgbData;
    typename DT<PIXEL_T_OUT, NPPC>::T BayerData;

    for (int i = 0; i < ImgHeight; i++) {
#ifdef __SYNTHESIS__
#pragma HLS loop pipeline
#endif
        for (int j = 0; j < ImgColumns; j++) {
            RgbData = InImg.read(ImgIndex);

            BayerData = 0;
            for (int k = 0; k < NPPC; k++) {
                if (i % 2 == 0) {
                    if ((j * NPPC + k) % 2 == 0) {
                        // red
                        BayerData.byte(k) = RgbData.byte(3 * k);
                    } else {
                        // green
                        BayerData.byte(k) = RgbData.byte(3 * k + 1);
                    }
                } else {
                    if ((j * NPPC + k) % 2 == 0) {
                        // green
                        BayerData.byte(k) = RgbData.byte(3 * k + 1);
                    } else {
                        // blue
                        BayerData.byte(k) = RgbData.byte(3 * k + 2);
                    }
                }
            }
            OutImg.write(BayerData, ImgIndex++);
        }
    }
}

} // End of namespace vision.
} // End of namespace hls.

#endif
