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

#ifndef __SHLS_VISION_LINE_BUFFER_H__
#define __SHLS_VISION_LINE_BUFFER_H__

#include <math.h>
namespace hls {
namespace sev {

// Example usage:
// - Instantiate the line buffer in your C++ implementation.  If you are
//   instantiating the line buffer inside a pipelined function (accepting a new
//   pixel in every function call), you will need to add 'static' to make the
//   line buffer static.   The window maintained by the line buffer assumes a
//   square WindowSize x WindowSize window.
//   > static hls::LineBuffer<unsigned char, ImageWidth, WindowSize>
//   line_buffer;
//
// - Shift in a new pixel by calling the ShiftInPixel method:
//   > line_buffer.ShiftInPixel(input_pixel);
//
// - Then your filter can access the window by "line_buffer.window[i][j]".

template <typename PixelType, unsigned ImageWidth, unsigned WindowSize,
          unsigned BitWidth = 8, unsigned NPPC = 1, bool LowRamUsage = false>
class LineBuffer {};

template <typename PixelType, unsigned ImageWidth, unsigned WindowSize,
          unsigned BitWidth, unsigned NPPC>
class LineBuffer<PixelType, ImageWidth, WindowSize, BitWidth, NPPC, false> {
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

    // The line buffer maintains NPPC number of WindowSize x WindowSize windows.
    // For example, say NPPC is 4 and WindowSize is 3, the windows array will
    // have the following 9 words, each word contains 4 pixels.
    //
    // a0.a1.a2.a3  b0.b1.b2.b3  c0.c1.c2.c3
    // d0.d1.d2.d3  e0.e1.e2.e3  f0.f1.f2.f3
    // g0.g1.g2.g3  h0.h1.h2.h3  i0.i1.i2.i3
    //
    // The user algorithm should be processing the "receptive" fields for the 4
    // pixels in word e.  So calling
    //   AccessWindow(x, y, 0) returns pixels in the receptive field for e0.
    //   - AccessWindow(0, 0, 0) gives a3
    //   - AccessWindow(1, 1, 0) gives e0
    //   - AccessWindow(2, 2, 0) gives h1
    //
    //   AccessWindow(x, y, 2) returns pixels in the receptive field for e2.
    //   - AccessWindow(2, 0, 2) gives b3
    //
    // x, y coordinates in a window where the centroid is at the k-th pixel.
    ap_uint<BitWidth> AccessWindow(unsigned x, unsigned y, unsigned k) {
        // The radius of 3x3 is 1, radius of 5x5 is 2, radius of 7x7 is 3.
        const unsigned WindowRadius = (WindowSize - 1) / 2;
        // Map k into j & pixel_offset
        // - k ranges from 0 to NPPC - 1
        // - j ranges from 0 to WindowSize, to index window
        // - pixel_offset range from 0 to NPPC - 1, to index a window element
        //
        // Say the window has WindowSize * NPPC columns, the column index of
        // the centroid pixel is:
        unsigned centroid_col = WindowRadius * NPPC + k;
        // Now the corresponding pixel at y = 0 is (WindowSize - 1) / 2 pixels
        // to the left of the centroid.
        unsigned col_idx = centroid_col - WindowRadius + y;

        unsigned j = col_idx / NPPC;
        unsigned pixel_offset = col_idx - j * NPPC;

        return window[x][j].byte(pixel_offset, BitWidth);
    }

    void print_window(unsigned k) {
        for (int i = 0; i < WindowSize; i++) {
            for (int j = 0; j < WindowSize; j++) {
                printf("%x ", int(AccessWindow(i, j, k)));
            }
            printf("\n");
        }
    }

  private:
    unsigned prev_row_index = 0;
    PixelType prev_row[WindowSize - 1][ImageWidth];
};

template <typename PixelType, unsigned ImageWidth, unsigned WindowSize,
          unsigned BitWidth, unsigned NPPC>
class LineBuffer<PixelType, ImageWidth, WindowSize, BitWidth, NPPC, true> {
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
    unsigned prev_row_index{0};
    PixelType prev_row[(WindowSize - 1) * ImageWidth];
};

// Specialization when the WindowSize is 1.
template <typename PixelType, unsigned ImageWidth, unsigned BitWidth,
          unsigned NPPC>
class LineBuffer<PixelType, ImageWidth, 1, BitWidth, NPPC, true> {
  public:
    PixelType window[1][1];
    void ShiftInPixel(PixelType input_pixel) { window[0][0] = input_pixel; }
};
template <typename PixelType, unsigned ImageWidth, unsigned BitWidth,
          unsigned NPPC>
class LineBuffer<PixelType, ImageWidth, 1, BitWidth, NPPC, false> {
  public:
    PixelType window[1][1];
    void ShiftInPixel(PixelType input_pixel) { window[0][0] = input_pixel; }
};

} // End of namespace vision.

namespace vision {
template <typename PixelType, unsigned ImageWidth, unsigned WindowSize,
          unsigned BitWidth = 8, unsigned NPPC = 1, bool LowRamUsage = false>
using LineBuffer = sev::LineBuffer<PixelType, ImageWidth, WindowSize, BitWidth,
                                   NPPC, LowRamUsage>;
}

} // End of namespace hls.

#endif
