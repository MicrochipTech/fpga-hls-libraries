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

#ifndef __SHLS_VISION_IMG_CROP_HPP__
#define __SHLS_VISION_IMG_CROP_HPP__

#include "common/common.hpp"
#include "common/params.hpp"
#include "common/utils.hpp"

namespace hls {
namespace vision {

/********************************************************************************************
 * @brief   Copy (crop) an axis-aligned sub-window from a source image into a
 *          destination image.
 *
 * The routine is a very thin wrapper around
 * `hls::vision::ImgToImgProcess<>`.
 * 
 * It forwards every template / run-time argument unchanged, so that the
 * behaviour is identical to a direct call to `ImgToImgProcess(...)` – the
 * only purpose of this helper is to avoid repeating the verbose template
 * list at each call-site.
 *
 * A rectangular region that starts at (start + rowBound , start + colBound)
 * inside InImg and has a size of H_DST × W_DST pixels is copied
 * into OutImg.
 * 
 * Pixels are transferred NPPC ( number of pixels per cycle ) at a time,
 * making the function suitable for use in a data-flow pipeline.
 *
 * @tparam PIXEL_T   Pixel data type (e.g. `hls::vision::PixelType::HLS_8UC1`).
 * @tparam H_SRC     Height  of the source image  (compile-time constant).
 * @tparam W_SRC     Width   of the source image  (compile-time constant).
 * @tparam H_DST     Height  of the destination image  (compile-time constant).
 * @tparam W_DST     Width   of the destination image  (compile-time constant).
 * @tparam STORAGE   Storage model used by the `Img` wrapper
 *                   (`StorageType::FIFO` and `StorageType::FRAME_BUFFER`).
 * @tparam NPPC      Number of pixels processed per clock cycle (1, 2, 4, …).
 *
 * @param[in]  InImg   Source image object to crop from.
 * @param[out] OutImg   Destination image object that receives the cropped region.
 * @param[in]  rowBound Row offset (relative to @p start) of the top-left corner
 *                      of the crop window in the source image.
 * @param[in]  colBound Column offset (relative to @p start) of the top-left corner
 *                      of the crop window in the source image.
 * @param[in]  start    Additional absolute offset applied to both row and column
 *                      (allows chaining of crops without recalculating coordinates).
 *
 * @note   • All bounds are inclusive and zero-based.  
 *         • It is the caller’s responsibility to ensure that the requested
 *           region lies completely inside InImg; the run-time clipping
 *           is performed, however.
 *         • Because this function is intended for HLS synthesis, it is
 *           declared `static` and `inline`-able; there is deliberately no
 *           return value.
 ********************************************************************************************/
template <
    PixelType PIXEL_T,              // Pixel Type
    int H_SRC,                      // Input Image Height
    int W_SRC,                      // Input Image Width
    int H_DST,                      // Output Image Height
    int W_DST,                      // Output Image Width
    StorageType STORAGE,            // Storage Type
    NumPixelsPerCycle NPPC          // Pixels Per Cycle
>
void CropImg(
    Img<PIXEL_T, H_SRC, W_SRC, STORAGE, NPPC> &InImg,
    Img<PIXEL_T, H_DST, W_DST, STORAGE, NPPC> &OutImg,
    int rowBound,
    int colBound,
    int start
) {

    static_assert(DT<PIXEL_T, NPPC>::NumChannels == 1,
        "CropImg function only supports one channel");

    static_assert(NPPC == 1,
        "CropImg function only supports one pixel per cycle (NPPC = 1).");

    static_assert(DT<PIXEL_T, NPPC>::W == 8,
        "CropImg function only supports 8 bits per channel");

    hls::vision::ImgToImg
    <   
        PIXEL_T,
        H_SRC,
        W_SRC,
        H_DST,
        W_DST,
        STORAGE,
        NPPC
    >
    (InImg, OutImg, rowBound, colBound, start, 0);
}

} // End of namespace vision.
} // End of namespace hls.

#endif
