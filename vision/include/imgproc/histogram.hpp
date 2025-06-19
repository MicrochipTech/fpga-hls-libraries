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

#pragma once

#include "../common/common.hpp"
#include "../common/utils.hpp"
#include <hls/ap_fixpt.hpp>

namespace hls {
namespace vision {

//------------------------------------------------------------------------------
template<typename T>
constexpr bool isPowerOfTwo(T n) {
    return (n > 0) && ((n & (n - 1)) == 0);
}

/*******************************************************************************
 * @brief Evenly re-injects any “excess” pixels that were clipped from a
 *        histogram bin during CLAHE processing.
 *
 *  The function receives a clipped 1-D histogram and the total number of
 *  pixels that were removed (`excess`).  
 *  - Step 1 – base redistribution 
 *    Every bin gets ⌊excess / HIST_SIZE⌋ extra counts (`step`).  
 *  - Step 2 – remainder redistribution
 *    The leftover pixels (`remainder = excess mod HIST_SIZE`) are added
 *    one-by-one using a Bresenham-style walk with stride 37
 *    to avoid concentrating the remainder in any local region.
 *
 *  All arithmetic is division-free except for the initial compile-time
 *  constant division that derives `step` and `remainder`.
 *
 *  @tparam HIST_SIZE  Histogram length (must be a power of two – enforced
 *                     at compile time).
 *  @param[in,out] histogram  Histogram to be updated in-place (length
 *                     `HIST_SIZE`).
 *  @param[in]      excess    Number of clipped pixels that need to be
 *                     re-distributed.
 *
 *  @note The compile-time power-of-two restriction lets us replace `%`
 *        with a cheap bit-wise AND.
 ******************************************************************************/
template <unsigned int HIST_SIZE = 256>
void redistributeExcessEvenly(uint32_t histogram[HIST_SIZE], uint32_t excess) {
    static_assert(isPowerOfTwo(HIST_SIZE) == true, 
    "The redistributeExcessEvenly function only supports histogram size being power of two.");

    /* ---------- base redistribution (no runtime division) ---------- */
    uint8_t step = excess / HIST_SIZE;        // Base excess per bin
    uint8_t remainder = excess & (HIST_SIZE - 1);   // Remaining excess pixels

    #pragma HLS loop pipeline
    for (int i = 0; i < HIST_SIZE; i++) {
        histogram[i] += step;
    }

    /* ---------- remainder redistribution (pseudo-random walk) ------- */

    // Distribute Remainder Using Bresenham’s algorithm
    // (Avoids Division by cycling through histogram bins using modular arithmetic.)
    // 37 is a prime number chosen to spread excess pixels evenly.
    // Using a fixed prime number avoids clustering in one region, leading to
    // better contrast adjustment in histogram equalization. 37 spreads excess
    // pixels more uniformly compared to smaller steps.
    #pragma HLS loop pipeline
    for (uint8_t i = 0, count = 0; count < remainder; count++) {
        histogram[i]++;
        i = (i + 37) & (HIST_SIZE - 1);
    }
}

/***************************************************************************************
 * @brief Clips a histogram to a specified limit and redistributes excess counts evenly.
 *
 * For each bin in the input histogram, if the bin count exceeds @p clipLimit,
 * the count is truncated to @p clipLimit and the excess is accumulated.
 * Once all bins have been processed, the total excess is redistributed
 * evenly across all bins using @c redistributeExcessEvenly().
 *
 * This function is typically used in CLAHE (Contrast-Limited Adaptive Histogram Equalization)
 * to prevent over-amplification of noise by capping the maximum bin count and spreading
 * the clipped counts, thereby preserving overall histogram mass.
 *
 * The function is fully pipelined for efficient hardware synthesis.
 *
 * @tparam HIST_SIZE   Number of bins in the histogram (default: 256).
 *
 * @param[in,out] hist      Histogram array to be clipped and redistributed.
 *                          On input, contains raw bin counts; on output, contains
 *                          the clipped and redistributed counts.
 * @param[in]     clipLimit Maximum allowed count for any bin. Must be ≥ 1.
 *
 * @note
 * - The excess is redistributed using the @c redistributeExcessEvenly function,
 *   which aims for a uniform spread of the clipped counts.
 * - This routine is typically called per-tile in CLAHE pipelines.
 * - The function does not check for negative @p clipLimit; the caller must ensure validity.
 **************************************************************************************/
template<int HIST_SIZE = 256>
void clipHistogram(uint32_t (&hist)[HIST_SIZE], int clipLimit)
{
    uint32_t excess = 0;
    // Clip the histogram & compute total excess
    CLIP_HISTOGRAM_LOOP:
    #pragma HLS loop pipeline
    for (int i = 0; i < HIST_SIZE; i++) {
        int diff = hist[i] - clipLimit;
        hls::ap_uint<1> diff_mask = (diff > 0);
        excess += diff_mask * diff;
        hist[i] = diff_mask * clipLimit + (1 - diff_mask) * hist[i];
    }

    // Redistribute excess pixels evenly
    redistributeExcessEvenly<HIST_SIZE>(hist, excess);
}

/***************************************************************************
 *  @brief  Computes an 8-bit grey-scale histogram of an image stream.
 *
 *  The image is supplied through a Img wrapper that models a
 *  FIFO or FRAME_BUFFER interface ( `StorageType::FIFO` or `StorageType::FRAME_BUFFER`).  
 *  Pixels are consumed one per clock-cycle (`NPPC==1`).  
 *  All histogram bookkeeping is performed on-chip; the final
 *  256-bin histogram is written to the caller-provided
 *  `histogram[HIST_SIZE]` array.
 *
 *  Internally a ping-pong scratch buffer (`h_tmp`) is used to break the
 *  read-after-write dependency that would otherwise prevent the loop from
 *  pipelining.  Each pixel value is counted exactly once, so
 *  `∑ histogram[i]  ==  H × W`.
 *
 *  Throughput:  Initiation Interval = 1  
 *  Restrictions:  
 *  * One image channel only (greyscale).  
 *  * 8-bit pixels ( `uint8_t` ).  
 *  * One pixel per cycle (`NPPC == 1`).
 *
 *  @tparam  HIST_SIZE   Number of bins (defaults to 256 for 8-bit data).
 *  @tparam  PIXEL_T     SmartHLS pixel type (e.g. `PixelType::HLS_8UC1`).
 *  @tparam  H           Image height  (compile-time constant).
 *  @tparam  W           Image width   (compile-time constant).
 *  @tparam  STORAGE_T   Storage model of the @p InImg wrapper
 *                       (can be `StorageType::FIFO` or `StorageType::FRAME_BUFFER` 
 *                       when synthesised).
 *  @tparam  NPPC        Pixels read per clock (only 1 is supported).
 *
 *  @param[in]  InImg    Streaming input image; delivers `H×W` pixels.
 *  @param[out] histogram
 *                       Caller-allocated array that receives the histogram
 *                       counts; index i corresponds to grey value i.
 *
 *  @note  All arguments are passed by reference; no dynamic memory is
 *         allocated inside this routine.
 *  @note  The two inner loops carry the pragmas required for an II = 1 pipeline.
 *****************************************************************************/
template <
    unsigned int HIST_SIZE = 256,
    PixelType PIXEL_T,
    unsigned H,
    unsigned W,
    StorageType STORAGE_T,
    NumPixelsPerCycle NPPC
> void Histogram (
    Img<PIXEL_T, H, W, STORAGE_T, NPPC> &InImg,
    uint32_t histogram[HIST_SIZE]
) {
    static_assert(DT<PIXEL_T, NPPC>::NumChannels == 1,
        "Histogram function only supports one channel");

    static_assert(NPPC == 1,
        "Histogram function only supports one pixel per cycle (NPPC = 1).");

    static_assert(DT<PIXEL_T, NPPC>::W == 8,
        "Histogram function only supports 8 bits per channel");

    const unsigned ImgHeight = InImg.get_height(), ImgWidth = InImg.get_width();

    using PixelWordT = ap_uint<DT<PIXEL_T, NPPC>::W>;

    uint8_t prevPix = 0;
    uint32_t prevVal = 0;

    static uint32_t h_tmp[HIST_SIZE];

    HLS_VISION_HISTOGRAM_LOOP1:
    #pragma HLS loop pipeline
    #pragma HLS loop dependence variable(h_tmp) type(inter) direction(RAW) dependent(false)
    for(int p=0; p < ImgHeight * ImgWidth; p++) {
        PixelWordT curPix = InImg.read(p);
        uint32_t curVal = h_tmp[curPix];
        if (prevPix == curPix) {
            curVal = prevVal;
        }
        curVal++;
        h_tmp[curPix] = curVal;
        prevPix = curPix;
        prevVal = curVal;
    }

    HLS_VISION_HISTOGRAM_LOOP2:
    #pragma HLS loop pipeline II(1)
    for(int p=0; p < HIST_SIZE; p++) {
        // printf("h_tmp[%d]:%d\n", p, h_tmp[p]);
        histogram[p] = h_tmp[p];
        h_tmp[p] = 0;
    }
}

} // End of namespace vision.
} // End of namespace hls.