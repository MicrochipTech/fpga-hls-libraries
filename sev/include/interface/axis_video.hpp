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

#ifndef __SHLS_SEV_AXIS_VIDEO_HPP__
#define __SHLS_SEV_AXIS_VIDEO_HPP__

#include "common.hpp"
#include <hls/ap_int.hpp>

namespace hls {
namespace sev {

/**
 * AXI stream video protocol type -- the underlying data type inside the
 * hls::FIFO template.
 */
template <PixelType PIXEL_T, NumPixelsPerCycle NPPC = NPPC_1>
struct AxisVideoT {
    typename DT<PIXEL_T, NPPC>::T data;
    ap_uint<1> last, user;
};

/**
 * AXI stream video protocol FIFO type.  The FIFO type will have data, last
 * user, plus valid and ready.
 */
template <PixelType PIXEL_T, NumPixelsPerCycle NPPC = NPPC_1>
using AxisVideoFIFO = hls::FIFO<AxisVideoT<PIXEL_T, NPPC>>;

/**
 * The function converts an AXI stream video stream (FIFO) to a sev::Img and
 * returns the following status code:
 *  0: OK, no error
 *  1: early start of frame (SOF is received in the middle of a previous frame)
 *  2: early end of line (EOL is received in the middle of a previous line)
 *  4: late end of line (EOL is missing at the expected last pixel of a line)
 * The return status can include more than 1 error, for example, we can have
 * both early start of frame and late end of line happening, which would result
 * in an error code of 5 (1 | 4 = 5)
 */
template <PixelType PIXEL_I_T, PixelType PIXEL_O_T, unsigned H, unsigned W,
          StorageType STORAGE = FIFO, NumPixelsPerCycle NPPC = NPPC_1>
int AxisVideo2Img(AxisVideoFIFO<PIXEL_I_T, NPPC> &VideoIn,
                  sev::Img<PIXEL_O_T, H, W, STORAGE, NPPC> &ImgOut) {
    AxisVideoT<PIXEL_I_T, NPPC> video_data;
    bool sof_received = false;
    unsigned write_index = 0;
    unsigned img_columns = ImgOut.get_width() / NPPC;

    int ret = 0;

#ifdef __SYNTHESIS__
#pragma HLS loop pipeline
#endif
    // Waiting until we receive SOF.
    while (!sof_received) {
        video_data = VideoIn.read();
        sof_received = video_data.user;
    }

    for (unsigned i = 0; i < ImgOut.get_height(); i++) {
        bool eol_received = false;
#ifdef __SYNTHESIS__
#pragma HLS loop pipeline
#endif
        for (int j = 0; j < img_columns; j++) {
            if (sof_received || eol_received) {
                sof_received = 0;
            } else {
                video_data = VideoIn.read();
                if (video_data.user) {
                    ret |= 1; // early start of frame
                }
            }
            eol_received = video_data.last;
            if (eol_received && (j != img_columns - 1)) {
                ret |= 2; // received eol_received before end of the row.
            }

            ImgOut.write(write_index++, video_data.data);
        }

#ifdef __SYNTHESIS__
#pragma HLS loop pipeline
#endif
        // Skip the pixels until we get the EOL.
        while (!eol_received) {
            video_data = VideoIn.read();
            eol_received = video_data.last;
            // We have received all expected pixels for the line from the for
            // loop above but were missing EOL.
            ret |= 4;
        }
    }
    return ret;
}

/**
 * The function converts a sev::Img to an AXI stream video stream (FIFO).
 * If the input sev::Img uses FIFO storage, the input Img will become empty
 * after calling this function (or 1 frame less of pixels).
 */
template <PixelType PIXEL_I_T, PixelType PIXEL_O_T, unsigned H, unsigned W,
          StorageType STORAGE = FIFO, NumPixelsPerCycle NPPC = NPPC_1>
void Img2AxisVideo(sev::Img<PIXEL_I_T, H, W, STORAGE, NPPC> &ImgIn,
                   AxisVideoFIFO<PIXEL_O_T, NPPC> &VideoOut) {
    for (unsigned i = 0, read_index = 0; i < ImgIn.get_height(); i++) {

        const unsigned img_columns = ImgIn.get_width() / NPPC;
#ifdef __SYNTHESIS__
#pragma HLS loop pipeline
#endif
        for (int j = 0; j < img_columns; j++, read_index++) {
            AxisVideoT<PIXEL_O_T, NPPC> video_data;
            video_data.user = (read_index == 0);
            video_data.last = (j == img_columns - 1);
            video_data.data = ImgIn.read(read_index);
            VideoOut.write(video_data);
        }
    }
}

} // end of namespace sev.
} // end of namespace hls.

#endif
