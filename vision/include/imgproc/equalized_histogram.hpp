// ©2025 Microchip Technology Inc. and its subsidiaries
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

#pragma once

#include "../common/common.hpp"
#include "../common/utils.hpp"
#include <hls/ap_fixpt.hpp>

namespace hls {
namespace vision {

/***************************************************************************//**
 * @brief  Histogram equalisation for a single–channel,
 *         8-bit image stored in a `Img`.
 *
 *         The routine:
 *         1.  Streams the image once to build a histogram.
 *         2.  Integrates that histogram to form the cumulative distribution
 *             function (CDF) and normalises it to the range [0, 255].
 *         3.  Streams the image a second time, rewriting every pixel through
 *             the LUT obtained from the CDF.
 *
 *         The design is fully pipelined (II = 1) and needs only a single
 *         on-chip frame buffer for the input image plus two 256-word RAMs
 *         (histogram + CDF/LUT).
 *
 * @tparam HIST_SIZE   Number of histogram bins – normally 256.
 * @tparam PIXEL_T     SmartHLS pixel type (only HLS_8UC1 supported).
 * @tparam H           Image height  (pixels).
 * @tparam W           Image width   (pixels).
 * @tparam STORAGE_TYPE
 *                     SmartHLS storage enum – can be FIFO or FRAME_BUFFER
 * @tparam NPPC        Pixels processed per clock – must be NPPC_1.
 *
 * @param[in]  InImg   Source image.
 * @param[out] OutImg  Destination image; same geometry as @p InImg.
 *
 * @note  Compile-time checks enforce all restrictions (
 *        greyscale, 8-bit depth, NPPC = 1, --etc.).
 *
 * @details
 * * Two-pass approach – the first pass gathers statistics, the second
 *   pass performs the LUT look-up.
 * * RAW hazard avoidance – the inner histogram loop uses a private
 *   `h_tmp` buffer and last-pixel re-use trick to achieve an initiation
 *   interval of 1.
 * * Fixed-point arithmetic – CDF normalisation is carried out with
 *   `ap_ufixpt` to keep the hardware small while preserving accuracy.
 ******************************************************************************/
template <
    int HIST_SIZE = 256,
    PixelType     PIXEL_T,
    int           H,
    int           W,
    StorageType   STORAGE_TYPE,
    NumPixelsPerCycle NPPC
>
void EqualizedHistogram(
    Img<PIXEL_T, H, W, STORAGE_TYPE, NPPC> &InImg,
    Img<PIXEL_T, H, W, STORAGE_TYPE, NPPC> &OutImg)
{

    static_assert(DT<PIXEL_T, NPPC>::NumChannels == 1,
                  "Supports single-channel images only");
    static_assert(NPPC == 1,          "NPPC must be 1");
    static_assert(DT<PIXEL_T, NPPC>::W == 8,
                  "Equalization is fixed to 8-bit pixels");

    using PixelWordT = ap_uint<DT<PIXEL_T, NPPC>::W>;
    /* ------------------------------------------------------------------ */
    /* 1.  Local storage                                                  */
    /* ------------------------------------------------------------------ */
    constexpr int TOTAL_PIXELS = H * W;

    #pragma HLS memory partition variable(histogram) type(complete)
    uint32_t histogram         [HIST_SIZE];
    #pragma HLS memory partition variable(cdf_lut) type(complete)
    uint32_t cdf_lut           [HIST_SIZE];

    PixelWordT img_buffer[TOTAL_PIXELS];   // keeps original order for write-back

    using PixelWordT = ap_uint<DT<PIXEL_T, NPPC>::W>;
    static uint32_t h_tmp[HIST_SIZE];
    uint32_t PrevPix = 0;
    uint32_t PrevVal = 0;

    /* ------------------------------------------------------------------ */
    /* 2.  Build the histogram                                            */
    /* ------------------------------------------------------------------ */
    #pragma HLS loop pipeline
    for (int i = 0; i < HIST_SIZE; ++i) {
        histogram[i] = 0;
        h_tmp[i] = 0;
    }

    // The use of h_tmp array is to resovle cross-iteration dependency
    // due to the RAW issue on histogram array if the following
    // expression is used inside a loop body:
    // histogram[curPix] += curVal;
    //
    // We use h_tmp to buffer histogram data and proactively check
    // the read address and the write address so that the RAW does not
    // occur.
    HLS_VISION_HISTOGRAM_LOOP1:
    #pragma HLS loop pipeline
    #pragma HLS loop dependence variable(h_tmp) type(inter) direction(RAW) dependent(false)
    for(int p=0; p < H * W; p++) {
        PixelWordT curPix = InImg.read(p);  // Read the current pixel
        img_buffer[p] = curPix;
        uint32_t curVal = h_tmp[curPix];
        if (curPix == PrevPix)
            curVal = PrevVal;
        curVal++;
        h_tmp[curPix] = curVal;
        PrevPix = curPix;
        PrevVal = curVal;
    }

    HLS_HE_HIST_LOOP:
    #pragma HLS loop pipeline II(1)
    for(int p=0; p < HIST_SIZE; p++) {
        histogram[p] = h_tmp[p];
        h_tmp[p] = 0;
    }

    /* ------------------------------------------------------------------ */
    /* 3.  Build CDF and LUT in one pass (integer math)                   */
    /* ------------------------------------------------------------------ */
    uint32_t cdf       = 0;
    uint32_t min_cdf   = 0;
    bool     min_seen  = false;
    const uint32_t NUMERATOR_SCALE = HIST_SIZE - 1;

CDF_LUT_BUILD:
    #pragma HLS loop pipeline
    for (int i = 0; i < HIST_SIZE; ++i) {
        cdf += histogram[i];

        if (!min_seen && cdf != 0) {
            min_cdf  = cdf;
            min_seen = true;
        }

        /*-----------------------------------------------------------------------------
        * Histogram–equalisation LUT entry
        *
        *   g(i) = round( ( CDF(i) - CDF_min ) * (L - 1)
        *                -------------------------------- )
        *                (   N        - CDF_min )
        *
        * where
        *   i          : current grey level (0‥255)
        *   CDF(i)     : cumulative histogram up to level i
        *   CDF_min    : first non-zero CDF value
        *   N          : total number of pixels  (H × W)
        *   L          : number of grey levels   (256 → L − 1 = 255)
        *
        * Integer implementation details
        * --------------------------------
        *   num = max(CDF(i) - CDF_min, 0)                     // numerator
        *   val = num * (L - 1)                                // 64-bit product
        *   den = N - CDF_min                                  // denominator
        *   g(i) = (den == 0) ? 0                              // degenerate image
        *          : floor( (val + den/2) / den )              // rounded division
        *---------------------------------------------------------------------------*/
        uint32_t num = (cdf > min_cdf) ? (cdf - min_cdf) : 0;
        // A 64-bit accumulator (val) guarantees the product cannot overflow when num ≈ H * W.
        uint64_t val = (uint64_t)num * NUMERATOR_SCALE;
        uint32_t den = TOTAL_PIXELS - min_cdf;
        // Integer division with rounding to the nearest integer
        cdf_lut[i]   = (den ? (val + (den >> 1)) / den : 0); // +½ for rounding
    }

    /* ------------------------------------------------------------------ */
    /* 4.  Map every input pixel through the LUT                          */
    /* ------------------------------------------------------------------ */
WRITE_BACK:
    #pragma HLS loop pipeline
    for (int p = 0; p < TOTAL_PIXELS; ++p) {
        OutImg.write(cdf_lut[img_buffer[p]], p);
    }
}

} // End of namespace vision.
} // End of namespace hls.
