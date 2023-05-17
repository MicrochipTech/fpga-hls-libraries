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

#ifndef __SHLS_VISION_AXIS_VIDEO_HPP__
#define __SHLS_VISION_AXIS_VIDEO_HPP__

#include "../common/common.hpp"
#include <hls/ap_int.hpp>

namespace hls {
namespace vision {

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
 * The function converts an AXI stream video stream (FIFO) to a vision::Img and
 * returns the following status code:
 *  0: OK, no error
 *  1: early start of frame (SOF is received in the middle of a previous frame)
 *  2: early end of line (EOL is received in the middle of a previous line)
 *  4: late end of line (EOL is missing at the expected last pixel of a line)
 * The return status can include more than 1 error, for example, we can have
 * both early start of frame and late end of line happening, which would result
 * in an error code of 5 (1 | 4 = 5)
 */
template <PixelType PIXEL_T, unsigned H, unsigned W, StorageType STORAGE = FIFO,
          NumPixelsPerCycle NPPC = NPPC_1>
int AxisVideo2Img(AxisVideoFIFO<PIXEL_T, NPPC> &InVideo,
                  vision::Img<PIXEL_T, H, W, STORAGE, NPPC> &OutImg) {
    AxisVideoT<PIXEL_T, NPPC> VideoData;
    bool SOF_Received = false;
    unsigned WriteIdx = 0;
    unsigned ImgColumns = OutImg.get_width() / NPPC;

    int Ret = 0;

#ifdef __SYNTHESIS__
#pragma HLS loop pipeline
#endif
    // Waiting until we receive SOF.
    while (!SOF_Received) {
        VideoData = InVideo.read();
        SOF_Received = VideoData.user;
    }

    for (unsigned i = 0; i < OutImg.get_height(); i++) {
        bool EOL_Received = false;
#ifdef __SYNTHESIS__
#pragma HLS loop pipeline
#endif
        for (int j = 0; j < ImgColumns; j++) {
            if (SOF_Received || EOL_Received) {
                SOF_Received = 0;
            } else {
                VideoData = InVideo.read();
                if (VideoData.user) {
                    Ret |= 1; // early start of frame
                }
            }
            EOL_Received = VideoData.last;
            if (EOL_Received && (j != ImgColumns - 1)) {
                Ret |= 2; // received EOL_Received before end of the row.
            }

            OutImg.write(VideoData.data, WriteIdx++);
        }

#ifdef __SYNTHESIS__
#pragma HLS loop pipeline
#endif
        // Skip the pixels until we get the EOL.
        while (!EOL_Received) {
            VideoData = InVideo.read();
            EOL_Received = VideoData.last;
            // We have received all expected pixels for the line from the for
            // loop above but were missing EOL.
            Ret |= 4;
        }
    }
    return Ret;
}

/**
 * The function converts a vision::Img to an AXI stream video stream (FIFO).
 * If the input vision::Img uses FIFO storage, the input Img will become empty
 * after calling this function (or 1 frame less of pixels).
 */
template <PixelType PIXEL_T, unsigned H, unsigned W, StorageType STORAGE = FIFO,
          NumPixelsPerCycle NPPC = NPPC_1>
void Img2AxisVideo(vision::Img<PIXEL_T, H, W, STORAGE, NPPC> &InImg,
                   AxisVideoFIFO<PIXEL_T, NPPC> &OutVideo) {
    const unsigned ImgColumns = InImg.get_width() / NPPC;
    const unsigned TripCount = InImg.get_height() * ImgColumns;
#pragma HLS loop pipeline
    for (unsigned i = 0, j = 0, ReadIdx = 0; i < TripCount; i++, ReadIdx++) {
        // for (int j = 0; j < ImgColumns; j++, ReadIdx++) {
        AxisVideoT<PIXEL_T, NPPC> VideoData;
        VideoData.user = (ReadIdx == 0);
        VideoData.last = (j == ImgColumns - 1);
        VideoData.data = InImg.read(ReadIdx);
        OutVideo.write(VideoData);
        if (j < ImgColumns - 1)
            j++;
        else
            j = 0;
        // }
    }
}

} // end of namespace vision.
} // end of namespace hls.

#endif
