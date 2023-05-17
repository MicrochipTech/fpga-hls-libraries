// ©2022 Microchip Technology Inc. and its subsidiaries
//
// Subject to your compliance with these terms, you may use this
// Microchip software and any derivatives exclusively with Microchip
// products. You are responsible for complying with third party
// license terms applicable to your use of third party software
// (including open source software) that may accompany this
// Microchip software. SOFTWARE IS “AS IS.” NO WARRANTIES, WHETHER
// EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING
// ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, OR
// FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE
// LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR
// CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
// WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF
// MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE
// FORESEEABLE.  TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP’S
// TOTAL LIABILITY ON ALL CLAIMS LATED TO THE SOFTWARE WILL NOT
// EXCEED AMOUNT OF FEES, IF ANY, YOU PAID DIRECTLY TO MICROCHIP FOR
// THIS SOFTWARE. MICROCHIP OFFERS NO SUPPORT FOR THE SOFTWARE. YOU
// MAY CONTACT MICROCHIP AT
// https://www.microchip.com/en-us/support-and-training/design-help/client-support-services
// TO INQUIRE ABOUT SUPPORT SERVICES AND APPLICABLE FEES, IF
// AVAILABLE.

#ifndef __SHLS_VISION_TEST_UTILS_HPP__
#define __SHLS_VISION_TEST_UTILS_HPP__

#include "common.hpp"
#include <hls/ap_int.hpp>
#include <stdio.h>

namespace hls {
namespace vision {

/**
 * Creates RGB image frames containing vertical colored lines
 * Format:  0: vertical lines
 *          1: horizontal lines
 *          2: angled lines over horizontal lines
 *          3: angled lines over vertical lines
 */
template <int Format = 0, PixelType PIXEL_T, unsigned H, unsigned W,
          StorageType STORAGE = FIFO, NumPixelsPerCycle NPPC = NPPC_1>
void PatternGenerator(vision::Img<PIXEL_T, H, W, STORAGE, NPPC> &ImgOut) {
    static_assert(
        (PIXEL_T == HLS_8UC3) && (Format == 0 || Format == 1 || Format == 2 ||
                                  Format == 3 || Format == 4),
        "Pattern generator only supports RGB (8UC3) pixel type and "
        "formats of 0 (vertical lines), 1(horizontal "
        "lines), 2 (angled lines over horizontal lines), 3 "
        "(angled lines over vertical lines). or 4 (checker board pattern).");
    int ImgHeight = ImgOut.get_height();
    int ImgWidth = ImgOut.get_width();
    int ImgColumns = ImgWidth / NPPC;
    int ImgIndex = 0;
    const unsigned NumberOfColors = 8;
    // Width or height of each color column or line
    int ColorColumnWidth = ImgColumns / NumberOfColors;
    int ColorLineHeight = ImgHeight / NumberOfColors;

    // Checker pattern column and row
    int CheckerCol, CheckerRow;
    bool CheckerColor;

    typename DT<PIXEL_T, NPPC>::T ImgData;
    typename DT<PIXEL_T, NPPC_1>::T PixelData;

    for (int i = 0; i < ImgHeight; i++) {
#ifdef __SYNTHESIS__
#pragma HLS loop pipeline
#endif
        for (int j = 0; j < ImgColumns; j++) {
            // Vertical colored lines
            if (Format == 0 || Format == 2) {
                // This binary search method results in a more efficient
                // hardware with less pipeline length.

                // This adds 2 angled orange lines (200 orange pixels per line)
                if (((j > i && j < i + 200) ||
                     (j > ImgHeight - i && j < ImgHeight - i + 200)) &&
                    Format == 2)
                    PixelData = 0x00A5FF;
                // Define the color based on the location.
                // There will be 8 columns and each pixel has its color defined
                // based on the column it goes to.
                // The colors are :
                // black - blue - green - cyan - red - pink - yello - white
                else if (j < 4 * ColorColumnWidth) {
                    if (j < 2 * ColorColumnWidth) {
                        if (j < ColorColumnWidth)
                            // black
                            PixelData = 0x000000;
                        else
                            // blue
                            PixelData = 0xFF0000;
                    } else {
                        if (j < 3 * ColorColumnWidth)
                            // green
                            PixelData = 0x00FF00;
                        else
                            // cyan
                            PixelData = 0xFFFF00;
                    }
                } else {
                    if (j < 6 * ColorColumnWidth) {
                        if (j < 5 * ColorColumnWidth)
                            // red
                            PixelData = 0x0000FF;
                        else
                            // pink
                            PixelData = 0xFF00FF;
                    } else {
                        if (j < 7 * ColorColumnWidth)
                            // yellow
                            PixelData = 0x00FFFF;
                        else
                            // white
                            PixelData = 0xFFFFFF;
                    }
                }

            }
            // Horizontal colored lines

            else if (Format == 1 || Format == 3) {
                // This adds 2 angled orange lines (200 orange pixels per line)
                if (((j > i && j < i + 200) ||
                     (j > ImgHeight - i && j < ImgHeight - i + 200)) &&
                    Format == 3)
                    PixelData = 0x00A5FF;
                // Define the color based on the location.
                // There will be 8 colored lines and each pixel has its color
                // defined based on the column it goes to.
                // The colors are (from top to bottom) :
                // black - blue - green - cyan - red - pink - yello - white
                else if (i < 4 * ColorLineHeight) {
                    if (i < 2 * ColorLineHeight) {
                        if (i < ColorLineHeight)
                            // black
                            PixelData = 0x000000;
                        else
                            // blue
                            PixelData = 0xFF0000;
                    } else {
                        if (i < 3 * ColorLineHeight)
                            // green
                            PixelData = 0x00FF00;
                        else
                            // cyan
                            PixelData = 0xFFFF00;
                    }
                } else {
                    if (i < 6 * ColorLineHeight) {
                        if (i < 5 * ColorLineHeight)
                            // red
                            PixelData = 0x0000FF;
                        else
                            // pink
                            PixelData = 0xFF00FF;
                    } else {
                        if (i < 7 * ColorLineHeight)
                            // yellow
                            PixelData = 0x00FFFF;
                        else
                            // white
                            PixelData = 0xFFFFFF;
                    }
                }

            } else {
                // Checker board pattern
                CheckerCol = j / ColorColumnWidth;
                CheckerRow = i / ColorLineHeight;

                // We assume an 8x8 black and white checker board, with top left
                // being white:
                // | W | B | W | B | W | B | W | B |
                // | B | W | B | W | B | W | B | W |
                // | W | B | W | B | W | B | W | B |
                // | B | W | B | W | B | W | B | W |
                // | W | B | W | B | W | B | W | B |
                // | B | W | B | W | B | W | B | W |
                // | W | B | W | B | W | B | W | B |
                // | B | W | B | W | B | W | B | W |
                //
                // As a result, if we are in an odd row and odd column or even
                // row and even column the color should be white and black
                // otherwise (top left is 0,0).
                // This can be simplified to if the rightmost bits of
                // column and row numbers are different, which means one of them
                // is odd and one is even, the color is white and black
                // otherwise.
                // (CheckerCol & 1) and (CheckerRow & 1) each return the
                // righmost bits of column and row, and the XOR "^" operand
                // returns 1 if they are not equal (indicating one is odd and
                // one is even) and 0 otherwise.

                CheckerColor = (CheckerCol & 1) ^ (CheckerRow & 1);
                if (CheckerColor)
                    // Black
                    PixelData = 0x000000;
                else
                    // White
                    PixelData = 0xFFFFFF;
            }
            ImgData = 0;
            for (int k = 0; k < NPPC; k++) {
                ImgData.byte(k, 24) = PixelData;
            }
            ImgOut.write(ImgData, ImgIndex++);
        }
    }
}

} // end of namespace vision
} // end of namespace hls

#endif
