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

#ifndef __SEV_UTILS_HPP__
#define __SEV_UTILS_HPP__

#include "common.hpp"

namespace hls {
namespace sev {

/**
 * Iterates over the input image pixel by pixel, and calls the pass-in functor
 * to compute each output pixel based on the corresponding input pixel.
 * The loop iterating a line of pixels is pipelined.  If there are multiple
 * pixels per cycle, all pixels will be processed in parallel.
 */
template <PixelType PIXEL_T_I, PixelType PIXEL_T_O, unsigned H, unsigned W,
          StorageType STORAGE_I = FIFO, StorageType STORAGE_O = FIFO,
          NumPixelsPerCycle NPPC = NPPC_1, typename Func>
void TransformPixel(sev::Img<PIXEL_T_I, H, W, STORAGE_I, NPPC> &ImgIn,
                    sev::Img<PIXEL_T_O, H, W, STORAGE_O, NPPC> &ImgOut,
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

                typename DT<PIXEL_T_I>::T InPixel =
                    ImgdataIn.byte(k, kIW);

                typename DT<PIXEL_T_O>::T OutPixel = Functor(InPixel);
                ImgdataOut.byte(k, kOW) = OutPixel;
            }
            ImgOut.write(idx, ImgdataOut);
        }
    }
}

/**
 * The LineBuffer class implements the line buffer that can be used to buffer a
 * few lines of pixels and provide a window of pixels as a stencil for applying
 * filters.
 * Example usage:
 * - Instantiate the line buffer in your C++ implementation.  If you are
 *   instantiating the line buffer inside a pipelined function (accepting a new
 *   pixel in every function call), you will need to add 'static' to make the
 *   line buffer static.   The window maintained by the line buffer assumes a
 *   square WindowSize x WindowSize window.
 *   > static hls::LineBuffer<unsigned char, ImageWidth, WindowSize> line_buffer;
 *
 * - Shift in a new pixel by calling the ShiftInPixel method:
 *   > line_buffer.ShiftInPixel(input_pixel);
 *
 * - Then your filter can access the window by "line_buffer.window[i][j]".
 */
template <typename PixelType, unsigned ImageWidth, unsigned WindowSize,
          bool LowRamUsage = false>
class LineBuffer {};

template <typename PixelType, unsigned ImageWidth, unsigned WindowSize>
class LineBuffer<PixelType, ImageWidth, WindowSize, false> {
  public:
    PixelType window[WindowSize][WindowSize];

    void ShiftInPixel(PixelType input_pixel) {
        // Shift existing window to the left by one.
        for (unsigned i = 0; i < WindowSize; i++) {
            for (unsigned j = 0; j < WindowSize - 1; j++) {
                window[i][j] = window[i][j + 1];
            }
        }

        // Load data into a simpler array prev_row_loads which can be more
        // easily partitioned by HLS.
        PixelType prev_row_loads[WindowSize - 1];
        for (unsigned i = 0; i < WindowSize - 1; i++)
            prev_row_loads[i] = prev_row[i][prev_row_index];

        // Grab next column (the rightmost column of the sliding window).
        for (unsigned i = 0; i < WindowSize; i++) {
            window[i][WindowSize - 1] =
                (i == WindowSize - 1) ? input_pixel
                                      : prev_row_loads[WindowSize - 2 - i];
        }

        for (int i = WindowSize - 2; i >= 0; i--) {
            prev_row[i][prev_row_index] =
                (i == 0) ? input_pixel : prev_row_loads[i - 1];
        }

        prev_row_index =
            (prev_row_index == ImageWidth - 1) ? 0 : prev_row_index + 1;
    }

  private:
    // Index that points to the current column of prev_row.
    unsigned prev_row_index = 0;
    PixelType prev_row[WindowSize - 1][ImageWidth];
};

template <typename PixelType, unsigned ImageWidth, unsigned WindowSize>
class LineBuffer<PixelType, ImageWidth, WindowSize, true> {
  public:
    PixelType window[WindowSize][WindowSize];

    void ShiftInPixel(PixelType input_pixel) {
        // Shift existing window to the left by one.
        for (unsigned i = 0; i < WindowSize; i++) {
            for (unsigned j = 0; j < WindowSize - 1; j++) {
                window[i][j] = window[i][j + 1];
            }
        }

        // Grab next column (the rightmost column of the sliding window).
        for (unsigned i = 0; i < WindowSize; i++) {
            window[i][WindowSize - 1] =
                (i == WindowSize - 1)
                    ? input_pixel
                    : prev_row[(WindowSize - 2 - i) * ImageWidth +
                               prev_row_index];
        }

        for (int i = WindowSize - 2; i >= 0; i--) {
            prev_row[i * ImageWidth + prev_row_index] =
                (i == 0) ? input_pixel
                         : prev_row[(i - 1) * ImageWidth + prev_row_index];
        }

        prev_row_index =
            (prev_row_index == ImageWidth - 1) ? 0 : prev_row_index + 1;
    }

  private:
    // Index that points to the current column of prev_row.
    unsigned prev_row_index{0};
    PixelType prev_row[(WindowSize - 1) * ImageWidth];
};

// Specialization when the WindowSize is 1.
template <typename PixelType, unsigned ImageWidth>
class LineBuffer<PixelType, ImageWidth, 1, true> {
  public:
    PixelType window[1][1];
    void ShiftInPixel(PixelType input_pixel) { window[0][0] = input_pixel; }
};
template <typename PixelType, unsigned ImageWidth>
class LineBuffer<PixelType, ImageWidth, 1, false> {
  public:
    PixelType window[1][1];
    void ShiftInPixel(PixelType input_pixel) { window[0][0] = input_pixel; }
};

} // End of namespace sev.
} // End of namespace hls.

#endif

