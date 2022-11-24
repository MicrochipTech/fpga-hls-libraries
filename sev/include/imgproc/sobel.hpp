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

#ifndef __SHLS_SEV_SOBEL_HPP__
#define __SHLS_SEV_SOBEL_HPP__

#include <type_traits>

#include "../common/common.hpp"
// FIXME: move LineBuffer to within SEV library.
#include <hls/image_processing.hpp>

namespace hls {
namespace sev {

// Only support 3x3 and 1 pixel-per-cycle for now for quick prototyping.
template <PixelType PIXEL_T_IN, PixelType PIXEL_T_OUT, unsigned H, unsigned W,
          StorageType STORAGE_IN, StorageType STORAGE_OUT>
void Sobel_3x3(sev::Img<PIXEL_T_IN, H, W, STORAGE_IN, NPPC_1> &img_in,
               sev::Img<PIXEL_T_OUT, H, W, STORAGE_OUT, NPPC_1> &img_out) {
    using DATA_T_IN = typename DT<PIXEL_T_IN, NPPC_1>::T;
    using DATA_T_OUT = typename DT<PIXEL_T_OUT, NPPC_1>::T;

    const int GX[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
    const int GY[3][3] = {{1, 2, 1}, {0, 0, 0}, {-1, -2, -1}};
    LineBuffer<DATA_T_IN, W, 3> line_buffer;

    const unsigned line_buffer_fill_count = img_in.get_width() + 1;
    const unsigned frame_size = img_in.get_height() * img_in.get_width();

#pragma HLS loop pipeline
    for (int count = 0, i = 0, j = 0;
         count < frame_size + line_buffer_fill_count; count++) {

        // No more reads after receiving the whole image, the extra iteration is
        // flushing out the final outputs.
        auto input_pixel = 0;
        if (count < img_in.get_height() * img_in.get_width())
            input_pixel = img_in.read(count);
        line_buffer.ShiftInPixel(input_pixel);

        // No output before line buffer is filled with 1 row + 1 pixel.
        if (count < line_buffer_fill_count)
            continue;

        // Indicates whether the current 3x3 receptive field is out of bound.
        bool outofbounds = (i < 1) | (i > img_in.get_height() - 2) | //
                           (j < 1) | (j > img_in.get_width() - 2);

        DATA_T_IN Window[3][3];
        for (int m = -1; m <= 1; m++) {
            for (int n = -1; n <= 1; n++) {
                int y = i + m, x = j + n;
                bool window_oob = (y < 0) | (y >= img_in.get_height()) |
                                  (x < 0) | (x >= img_in.get_width());
                Window[m + 1][n + 1] = window_oob
                                           ? DATA_T_IN(0)
                                           : line_buffer.window[m + 1][n + 1];
            }
        }

        // Apply the sobel filter at the current "receptive field".
        DATA_T_OUT gx_sum = 0, gy_sum = 0;
        for (int m = -1; m <= 1; m++) {
            for (int n = -1; n <= 1; n++) {
                // Get the pixel in "receptive field" from LineBuffer's window.
                DATA_T_IN pixel = Window[m + 1][n + 1];
                gx_sum += pixel * GX[m + 1][n + 1];
                gy_sum += pixel * GY[m + 1][n + 1];
            }
        }

        gx_sum = (gx_sum < 0) ? DATA_T_OUT(0 - gx_sum) : gx_sum;
        gy_sum = (gy_sum < 0) ? DATA_T_OUT(0 - gy_sum) : gy_sum;

        DATA_T_OUT sum = gx_sum + gy_sum;
        sum = (sum > 255) ? DATA_T_OUT(255) : sum;

        // Set output to 0 if the current 3x3 receptive field is out of bound.
        img_out.write(count - line_buffer_fill_count, sum);

        // Keep track of row/column of image.
        if (j < W - 1) {
            j++;
        } else { // j == WIDTH - 1.
            i++;
            j = 0;
        }
    }
}

template <unsigned FILTER_SIZE = 3, PixelType PIXEL_T_IN, PixelType PIXEL_T_OUT,
          unsigned H, unsigned W, StorageType STORAGE_IN,
          StorageType STORAGE_OUT, NumPixelsPerCycle NPPC = NPPC_1>
void Sobel(sev::Img<PIXEL_T_IN, H, W, STORAGE_IN, NPPC> &img_in,
           sev::Img<PIXEL_T_OUT, H, W, STORAGE_OUT, NPPC> &img_out) {
#pragma HLS memory partition argument(img_in) type(struct_fields)
#pragma HLS memory partition argument(img_out) type(struct_fields)

    static_assert(FILTER_SIZE == 3 && NPPC == NPPC_1,
                  "Sobel only supports filter size of 3 and number of "
                  "pixels per cycle of 1.");
    Sobel_3x3<PIXEL_T_IN, PIXEL_T_OUT, H, W, STORAGE_IN, STORAGE_OUT>(img_in,
                                                                      img_out);
}

} // End of namespace sev.
} // End of namespace hls.

#endif

