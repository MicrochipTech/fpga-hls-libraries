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

template <PixelType PIXEL_T, NumPixelsPerCycle NPPC>
using ImgDT = typename DT<PIXEL_T, NPPC>::T;


/**
 * @brief  Compute the CLAHE histogram equalisation LUT for a single tile.
 *
 * The routine builds a (clipped) 256-entry histogram for the
 * @p TILE_H × @p TILE_W slice given in @p window, converts it to a cumulative
 * distribution function (CDF), normalises that CDF and stores the resulting
 * look-up-table in @p lut.  
 *  
 * A small on-chip scratch array @c h_tmp is used to break the *RAW*
 * (read-after-write) dependency that would otherwise prevent the inner loop
 * from being fully pipelined.  When @p CLIP_LIMIT > 0 the histogram is first
 * clipped and the excess is re-distributed (`clipHistogram()`).
 *
 * ────────────────────────────────────────────────────────────────────
 * Template parameters
 * ────────────────────────────────────────────────────────────────────
 * @tparam HIST_SIZE  Size of the histogram & LUT (default 256).
 * @tparam CLIP_LIMIT Histogram clip limit  
 *                    – > 0 → clip & redistribute,  
 *                    – 0   → plain histogram (no clipping).
 * @tparam PIXEL_T    HLS pixel type (must be mono, 8 bit).
 * @tparam H,W        Tile height / width in pixels.
 * @tparam NPPC       Pixels processed per clock (only 1 is supported here).
 *
 * ────────────────────────────────────────────────────────────────────
 * Function arguments
 * ────────────────────────────────────────────────────────────────────
 * @param window  Flattened @p H * @p W buffer holding the tile’s pixels in
 *                row-major order.  Each entry is an 8-bit greyscale value.
 * @param lut     Output. Receives the equalisation LUT  
 *                (array of @p HIST_SIZE unsigned bytes).
 *
 * ────────────────────────────────────────────────────────────────────
 * Internal operation
 * ────────────────────────────────────────────────────────────────────
 * 1.  Clear the local @p histogram.  
 * 2.  Histogram pass.
 *     – The pixel stream is traversed once; a RAW-free scratch buffer
 *       (@c h_tmp) allows `II=1`.  
 * 3.  Optional clipping and excess redistribution.  
 * 4.  Build CDF, capture the first non-zero entry (`min_cdf`).  
 * 5.  Normalise the CDF to `[0 … HIST_SIZE-1]` and write it to @p lut,
 *     rounding and clamping.
 *
 * The function is fully pipelined; and the
 * implementation is re-entrant from the HLS point of view.
 */
template <
    int HIST_SIZE = 256,
    int CLIP_LIMIT,
    vision::PixelType PIXEL_T,
    int H,
    int W,
    vision::NumPixelsPerCycle NPPC
>
static void histEqu (
    ImgDT<PIXEL_T, NPPC> window[H * W],
    uint8_t lut[HIST_SIZE]
) {
    static_assert(DT<PIXEL_T, NPPC>::NumChannels == 1,
        "histEqu function only supports one channel");
    static_assert(NPPC == 1,
        "histEqu function only supports one pixel per cycle (NPPC = 1).");
    static_assert(DT<PIXEL_T, NPPC>::W == 8,
        "histEqu function only supports 8 bits per channel");

    uint32_t cdf[HIST_SIZE];
    uint32_t histogram[HIST_SIZE];
    int min_cdf = 0;

    using PixelWordT = ap_uint<DT<PIXEL_T, NPPC>::W>;
    static uint32_t h_tmp[HIST_SIZE];
    uint32_t PrevPix = 0;
    uint32_t PrevVal = 0;

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
        PixelWordT curPix = window[p];
        uint32_t curVal = h_tmp[curPix];
        if (curPix == PrevPix)
            curVal = PrevVal;
        curVal++;
        h_tmp[curPix] = curVal;
        PrevPix = curPix;
        PrevVal = curVal;
    }

    HLS_VISION_HISTOGRAM_LOOP2:
    #pragma HLS loop pipeline II(1)
    for(int p=0; p < HIST_SIZE; p++) {
        histogram[p] = h_tmp[p];
        h_tmp[p] = 0;
    }

    // clip and redistribute historgram if CLIP_LIMIT > 0
    // In OpenCV CLAHE, the clipLimit parameter is not a direct per-bin limit.
    // OpenCV scales it by the tile area and divides by the number of bins
    constexpr int tileArea = H * W;
    constexpr int clipLimit = std::max((CLIP_LIMIT > 0) ? (CLIP_LIMIT * tileArea / HIST_SIZE) : INT_MAX, CLIP_LIMIT);
    hls::vision::clipHistogram<HIST_SIZE>(histogram, clipLimit);

    // Compute cumulative distribution function (CDF)
    cdf[0]   = histogram[0];
    min_cdf = (cdf[0] != 0) ? cdf[0] : -1;

    HLS_VISION_CDF_LOOP:
    #pragma HLS loop pipeline
    for (int i = 1; i < HIST_SIZE; ++i)
    {
        cdf[i] = cdf[i-1] + histogram[i];
        if (min_cdf < 0 && cdf[i] != 0)            // pick first non-zero
            min_cdf = cdf[i];
    }

    if (min_cdf < 0)  min_cdf = 0;                // completely empty tile


    // Normalize CDF
    hls::ap_fixpt<32,16> dividend = (tileArea > min_cdf) ? (tileArea - min_cdf) : 1;
    hls::ap_fixpt<2,1> offset = 0.5;
    hls::ap_fixpt<32,16> factor = (HIST_SIZE - 1) / dividend;

    HLS_VISION_LUT_LOOP:
    #pragma HLS loop pipeline
    for (int i = 0; i < HIST_SIZE; ++i)
    {
        // Compute equalized histogram directly
        hls::ap_fixpt<40, 32, AP_TRN, AP_SAT> divisor = cdf[i] - min_cdf;
        hls::ap_fixpt<17, 9, AP_TRN, AP_SAT> tmp = divisor * factor;
        lut[i] = tmp + offset;
        // Ensure values remain in the valid range [0, HIST_SIZE-1]
        if (lut[i] > HIST_SIZE-1) lut[i] = HIST_SIZE - 1;
        if (lut[i] < 0) lut[i] = 0;
    }
}

/**
 * @brief  Bilinear-interpolate an entire tile-row using the two most
 *         recently received LUT rows (*ping–pong buffer*).
 *
 * For every tile column @p t in the current tile-row the routine:
 * 1.  Copies the four 256-entry corner LUTs {row0,row1} × {col0,col1}
 *     into fast on-chip arrays @c lut11 … @c lut22.  
 *     - `row0` = the previous tile-row (index 0 or 1 inside the
 *       2-row circular buffer)  
 *     - `row1` = the current tile-row, except on the bottom edge where
 *       `row0` is replicated.  
 *     - `col0` = tile column @p t  
 *     - `col1` = @p t + @p 1 or @p t when we are already at the rightmost tile.
 * 2.  Walks through all @p TILE_H * @p TILE_W pixels of that tile, computes
 *     sub-pixel bilinear weights, looks up the four LUT values, and writes
 *     the blended result to @p outData.
 *
 * Pixels on the image border are blended against replicated tiles so that no
 * out-of-bounds access occurs and no artificial dark/bright frame appears.
 *
 * ────────────────────────────────────────────────────────────────────────────
 * Template parameters
 * ────────────────────────────────────────────────────────────────────────────
 * @tparam HIST_SIZE Size of each LUT (default 256 for 8-bit images).
 * @tparam TILE_H,W  Tile height / width in pixels.
 * @tparam ROWS      Number of tile-rows in the whole image ( = H / TILE_H ).
 * @tparam COLS      Number of tile-columns             ( = W / TILE_W ).
 * @tparam BUFF_R    Depth of the ping-pong LUT buffer  
 *                   (must be 2 – one “previous” and one “current” row).
 * @tparam TILE_PIX  Convenience = @p TILE_H * @p TILE_W.
 *
 * ────────────────────────────────────────────────────────────────────────────
 * Function arguments
 * ────────────────────────────────────────────────────────────────────────────
 * @param lutBuff  Two-row circular buffer of LUTs.  
 *                 Layout: <tt>[ row (0/1) ][ tile-col ][ 256 ]</tt>.
 * @param inData   Raw tile pixels of the current tile-row.  
 *                 Layout: <tt>[ tile-col ][ TILE_PIX ]</tt>, row-major.
 * @param outData  Output buffer receiving the interpolated pixels in the
 *                 same layout as @p inData.
 * @param lutY     0 → use LUT rows {0,1};  
 *                 1 → use LUT rows {1,0}.  
 *                 (Pass the tile-row index & 1 to alternate automatically.)
 *
 * ────────────────────────────────────────────────────────────────────────────
 * Precision / numeric notes
 * ────────────────────────────────────────────────────────────────────────────
 * *Weights* (`w_t`) are unsigned fixed-point ( ≥12 bits ) covering 0…1.  
 * Each LUT entry is promoted to `p_t` before multiplication so no overflow
 * occurs. The four products are accumulated in `acc_t`, rounded, and
 * clamped to 8-bit before being written back to @p outData.
 *
 * Changing the three typedefs `w_t`, `p_t`, `acc_t` is the recommended
 * way to trade off accuracy vs. FPGA resources.
 */
template <
    vision::PixelType PIXEL_T,
    vision::NumPixelsPerCycle NPPC,
    int HIST_SIZE = 256,
    int TILE_H,
    int TILE_W,
    int ROWS,
    int COLS,
    int BUFF_R
>
static void bilinearFilterWithLUTBuff(
    uint8_t lutBuff[BUFF_R][COLS][HIST_SIZE],
    ImgDT<PIXEL_T, NPPC> inData [COLS][TILE_H * TILE_W],
    ImgDT<PIXEL_T, NPPC> outData[COLS][TILE_H * TILE_W],
    int     lutY)          // 0 / 1  → which pair  (previous / current)
{
    // on-chip RAM to cache lut buffer
    uint8_t lut11[HIST_SIZE];
    uint8_t lut12[HIST_SIZE];
    uint8_t lut21[HIST_SIZE];
    uint8_t lut22[HIST_SIZE];

    /* ------------------------------------------------------------------ */
    /* Fixed-point helper types                                            */
    /* ------------------------------------------------------------------ */
    // For all tile sizes
    #if TILE_SIZE == 8
    using w_t   = hls::ap_ufixpt<12,2, AP_RND_CONV, AP_SAT>;    // weights 0…1
    #else
    using w_t   = hls::ap_ufixpt<18,8, AP_RND_CONV, AP_SAT>;    // weights 0…1
    #endif
    using p_t   = hls::ap_ufixpt<30, 9, AP_RND_CONV, AP_SAT>;   // weight·value
    using acc_t = hls::ap_ufixpt<32, 10, AP_RND_CONV, AP_SAT>;  // sum of four

    const bool bottom_edge = (lutY == ROWS-1);
    const w_t rnd = w_t(0.5);

    /* Top / bottom row inside circular buffer */
    const int row0 =  lutY      & 1;                 // previous row  (0 or 1)
    const int row1 = bottom_edge ? row0 : row0 ^ 1;  // row0⊕1, but keep row0
                                                     // when we are already at
                                                     // the last tile-row
    TILE_COL_LOOP:
    for (int t = 0; t < COLS; ++t){

        const bool right_edge  = (t == COLS-1);
        const int col0 = t;
        const int col1 = right_edge  ? col0 : col0 + 1; // clamp at right edge

        // buffer lutBuff for the neighnour tiles
        INIT_LUT11_LOOP:
        #pragma HLS loop pipeline
        for(int i = 0; i < HIST_SIZE; ++i) lut11[i] = lutBuff[row0][col0][i];
        INIT_LUT12_LOOP:
        #pragma HLS loop pipeline
        for(int i = 0; i < HIST_SIZE; ++i) lut12[i] = lutBuff[row0][col1][i];
        INIT_LUT21_LOOP:
        #pragma HLS loop pipeline
        for(int i = 0; i < HIST_SIZE; ++i) lut21[i] = lutBuff[row1][col0][i];
        INIT_LUT22_LOOP:
        #pragma HLS loop pipeline
        for(int i = 0; i < HIST_SIZE; ++i) lut22[i] = lutBuff[row1][col1][i];

        // interplate a tile of pixels
        PIX_ROW_LOOP:
        for (int dy = 0; dy < TILE_H; ++dy){
            // precise pixel-centre weights
            const w_t fy  = w_t(dy) / TILE_H;
            const w_t fy1 = w_t(1) - fy;

            const w_t fy_eff = bottom_edge ? w_t(0) : fy;
            const w_t fy1_eff = bottom_edge ? w_t(1) : fy1;

            PIX_COL_LOOP:
            #pragma HLS loop pipeline
            for (int dx = 0; dx < TILE_W; ++dx){
                // precise pixel-centre weights
                const w_t fx  = w_t(dx) / TILE_W;
                const w_t fx1 = w_t(1) - fx;

                /* If we had to clamp => force weight to zero for the missing neighbour */
                const w_t fx_eff = right_edge  ? w_t(0) : fx;
                const w_t fx1_eff= right_edge  ? w_t(1) : fx1;

                const int  pIdx = dy*TILE_W + dx;
                ImgDT<PIXEL_T, NPPC> g    = inData[t][pIdx];

                /* lift LUT entry to p_t _once_ (no overflow) */
                const p_t v11 = lut11[g];
                const p_t v12 = lut12[g];
                const p_t v21 = lut21[g];
                const p_t v22 = lut22[g];

                acc_t interp =
                    (fx1_eff * fy1_eff) * v11 +
                    (fx_eff  * fy1_eff) * v12 +
                    (fx1_eff *  fy_eff) * v21 +
                    (fx_eff  *  fy_eff) * v22;

                outData[t][pIdx] = static_cast<ImgDT<PIXEL_T, NPPC>>(interp + rnd);
            }
        }
    }
}

/**
 * @brief  Produces a stream of CLAHE tiles and their LUTs from an input
 *         image.  
 *         For every  @p TILE_H * @p TILE_W  patch (i.e. tile) the function:
 *         - collects its 256 pixels into an on-chip buffer  
 *         - builds the 256-entry equalisation LUT via @c histEqu  
 *         - pushes first the raw tile-pixels, then the LUT to two separate
 *           FIFOs.
 *
 * The data order on the FIFOs is therefore:
 * ```
 *  [ tile 0  pixels ] → [ tile 0 LUT ] →
 *  [ tile 1  pixels ] → [ tile 1 LUT ] → …   (left-to-right, top-to-bottom)
 * ```
 * A consumer (e.g. `ClaheFilterStream`) can read the two FIFOs in lock-step
 * to reconstruct the full, contrast-limited image.
 *
 * ────────────────────────────────────────────────────────────────────────────
 * Template parameters
 * ────────────────────────────────────────────────────────────────────────────
 * @tparam PIXEL_T    HLS pixel type (must be 8-bit, 1-channel).
 * @tparam H, W       Image height & width in pixels.
 * @tparam STORAGE    Memory layout enum used by `Img<>`.
 * @tparam NPPC       Pixels processed per clock (must match port width).
 * @tparam HIST_SIZE  Histogram / LUT size (default 256).
 * @tparam TILE_H,W   Tile dimensions (e.g. 8, 16, 32). *W* must divide @p W,
 *                    *H* must divide @p H.
 * @tparam COLS       Number of tiles in one image row ( @p W / @p TILE_W ).
 * @tparam CLIP_LIM   CLAHE clip-limit (per tile); ≤0 disables clipping.
 *
 * ────────────────────────────────────────────────────────────────────────────
 * Function arguments
 * ────────────────────────────────────────────────────────────────────────────
 * @param InImg       Input image object; pixels are read sequentially
 *                    (row-major order).
 * @param pixel_fifo  **Output** FIFO carrying raw tile pixels  
 *                    – depth ≥ @p H * @p W.
 * @param lut_fifo    **Output** FIFO carrying the LUT that follows each tile  
 *                    – depth ≥ @p HIST_SIZE * @p COLS * ( @p H / @p TILE_H ).
 *
 * ────────────────────────────────────────────────────────────────────────────
 * Notes / Requirements
 * ────────────────────────────────────────────────────────────────────────────
 * * The function is synthesised for streaming (no random image access).
 * * Should be run in a `dataflow` region together with its consumer.
 * * Internal storage: one row of @p COLS circular tile-buffers
 *   ( @p COLS x @p TILE_PIX bytes ).
 * * The compile-time check inside `histEqu` enforces 8-bit grayscale input.
 */
template <
    vision::PixelType PIXEL_T,
    int H,
    int W,
    vision::StorageType STORAGE,
    vision::NumPixelsPerCycle NPPC,
    int HIST_SIZE = 256,
    int TILE_H,
    int TILE_W,
    int COLS
>
static void ClahePixelLoader(
    Img<PIXEL_T, H, W, STORAGE, NPPC> &InImg,
    ImgDT<PIXEL_T, NPPC> tile_pix [COLS][TILE_H * TILE_W],
    uint8_t tile_ready[COLS]
) {

    constexpr int PIXELS = H * W;
    constexpr int TILE_PIX = TILE_H * TILE_W;
    /* ----------------------------------------------------------------
        COLS circular tile buffers (e.g. COLS = 40, 40 × 256 B = 10 240 B)
    ------------------------------------------------------------------*/
    // ImgDT<PIXEL_T, NPPC> tile_pix [COLS][TILE_PIX];
    uint8_t tile_lut [HIST_SIZE];         // scratch for one LUT

    /* counters inside image stream */
    uint16_t col_in_img  = 0;                   // 0 … 639

    /* per–tile write pointers */
    /* wr_ptr[c] indicates how many pixels have already
        been written into the current tile that belongs
        to column c */
    #pragma HLS memory partition variable(wr_ptr) type(complete)
    uint32_t  wr_ptr[COLS] = {0};                // 0 … 255 inside tile

//-------------------------------- pixel stream -------------------------------
PIX_LOOP:
    for (int p = 0; p < PIXELS; ++p)
    {
        //--------------------------------------------------------------
        /* 1 ‑‑ read next pixel */
        uint8_t pix = InImg.read(p);

        //--------------------------------------------------------------
        /* 2 ‑‑ decode its tile coordinates                           */
        const uint16_t tile_col  = col_in_img / TILE_W; // 0…COLS-1
        uint32_t wr_index  = wr_ptr[tile_col]++;        // 0…255

        //--------------------------------------------------------------
        /* 3 ‑‑ store pixel into its 256‑byte tile                     */
        tile_pix[tile_col][wr_index] = pix;

        //--------------------------------------------------------------
        /* 4 ‑‑ when we have written 256 px into this tile‑buffer      */
        bool is_tile_ready = wr_index == TILE_PIX-1;
        wr_ptr[tile_col] = is_tile_ready * 0 + (1 - is_tile_ready) * wr_ptr[tile_col];
        tile_ready[tile_col] = is_tile_ready;

        // if (wr_index == TILE_PIX-1)      /* tile ready for lut compute */
        // {
        //     // printf("reached here 1, p = %d\n", p);
        //     /* (ii) stream TILE and LUT */
        //     #pragma HLS loop pipeline
        //     for (int i=0;i<TILE_PIX;++i) pixel_fifo.write(tile_pix[tile_col][i]);
        //     wr_ptr[tile_col] = 0;              // reset pointer
        // }

        //--------------------------------------------------------------
        /* 5 ‑‑ advance image‑position counters                        */
        unsigned next = col_in_img + 1u;        // 0 … W
        // -(next == W) is 0xFFFF… when next == W, otherwise 0.
        col_in_img   = next - (W & -(next == W));
    }
}

template <
    vision::PixelType PIXEL_T,
    int H,
    int W,
    vision::StorageType STORAGE,
    vision::NumPixelsPerCycle NPPC,
    int HIST_SIZE = 256,
    int TILE_H,
    int TILE_W,
    int COLS,
    int ROWS,
    int CLIP_LIM = 2
>
static void ClaheLutStream(
    ImgDT<PIXEL_T, NPPC> tile_pix [COLS][TILE_H * TILE_W],
    uint8_t tile_ready[COLS],
    hls::FIFO<ImgDT<PIXEL_T, NPPC>> &pixel_fifo,
    hls::FIFO<uint8_t> &lut_fifo
) {

    constexpr int TILE_PIX = TILE_H * TILE_W;
    uint8_t tile_lut[HIST_SIZE];

//-------------------------------- lut stream -------------------------------
LUT_TILE_LOOP:
    for (int tileRow = 0; tileRow < ROWS; ++tileRow)
    {
        /* ---------------- read one tile‑row from stream -------------*/
        for (int t = 0; t < COLS; ++t){
            if(tile_ready[t]) {

                histEqu
                <
                    HIST_SIZE,
                    CLIP_LIM,
                    PIXEL_T,
                    TILE_H,
                    TILE_W,
                    NPPC_1
                >
                ( tile_pix[t], tile_lut );

                #pragma HLS loop pipeline
                for (int i=0;i<HIST_SIZE;++i) lut_fifo.write(tile_lut[i]);
                #pragma HLS loop pipeline
                for (int i=0;i<TILE_PIX;++i) pixel_fifo.write(tile_pix[t][i]);

            }
        }
    }
}

/**
 * @brief  Consumes a stream of **CLAHE-tile pixels** + their **LUTs** and writes
 *         the fully-interpolated image into OutImg.
 *
 * The producer (i.e. `ClaheLutStream`) sends data in the sequence  
 * **[ 1 tile ( @p TILE_H * @p TILE_W )  →  its TILE_PIX-entry LUT ]** for every tile
 * in a scan-line order.  
 * `ClaheFilterStream` keeps two successive tile-rows in on-chip buffers
 * (`BUFF_R` = 2) so that it can bilinearly interpolate each tile from the four
 * surrounding LUTs and emit the final contrast-enhanced pixels.
 *
 * ────────────────────────────────────────────────────────────────────────────
 * Template parameters
 * ────────────────────────────────────────────────────────────────────────────
 * @tparam PIXEL_T   HLS pixel type (e.g. `HLS_8UC1`).
 * @tparam H         Image height  in pixels.
 * @tparam W         Image width   in pixels.
 * @tparam STORAGE   Memory layout enum used by `Img<>`.
 * @tparam NPPC      Number-of-Pixels-Per-Cycle (must match the stream width).
 * @tparam HIST_SIZE Size of every LUT (normally 256 for 8-bit gray).
 * @tparam TILE_H    Tile height   (e.g. 16 or 8).
 * @tparam TILE_W    Tile width    (same as TILE_H for square tiles).
 * @tparam COLS      Number of tiles in a row  = @p W / @p TILE_W.
 *
 * ────────────────────────────────────────────────────────────────────────────
 * Function arguments
 * ────────────────────────────────────────────────────────────────────────────
 * @param pixel_fifo  Input FIFO that delivers tile pixels in the order
 *                    described above.  Depth = @p H * @p W
 * @param lut_fifo    Input FIFO that delivers LUT following
 *                    every tile. Depth = ( @p HIST_SIZE * @p COLS * @p ROWS )
 * @param OutImg      Output image handle that is written pixel-by-pixel once
 *                    the surrounding 2 × 2 LUT neighbourhood is available.
 *
 * ────────────────────────────────────────────────────────────────────────────
 * Requirements & Notes
 * ────────────────────────────────────────────────────────────────────────────
 * * The caller must instantiate both FIFOs with sufficient depth.
 * * TILE_H and TILE_W must evenly divide H and W respectively.
 * * `pixel_fifo` and `lut_fifo` must be driven at the same rate so that the
 *   consumer never under- or over-flows.
 * * The function is intended for HLS data-flow contexts and should be
 *   marked  `#pragma HLS function dataflow`  in the caller if run in parallel
 *   with its producer.
 */
template <
    vision::PixelType PIXEL_T,
    int H,
    int W,
    vision::StorageType STORAGE,
    vision::NumPixelsPerCycle NPPC,
    int HIST_SIZE = 256,
    int TILE_H,
    int TILE_W,
    int COLS,
    int ROWS
>
static void ClaheFilterStream(
    hls::FIFO<ImgDT<PIXEL_T, NPPC>> &pixel_fifo,
    hls::FIFO<uint8_t> &lut_fifo,
    Img<PIXEL_T, H, W, STORAGE, NPPC> &OutImg
){

    constexpr int PIXELS = H * W;
    constexpr int TILE_PIX = TILE_H * TILE_W;
    constexpr int BUFF_R = 2;

    /* E.g. COLS = 40 and TILE_SIZE = 16,
    tile_buf and lut_buf each consumes 2 rows × 40 tiles × 256B  = 20 480 B */
    ImgDT<PIXEL_T, NPPC> tile_buf[BUFF_R][COLS][TILE_PIX];
    uint8_t lut_buf [BUFF_R][COLS][HIST_SIZE];

    /* define a lambda function that filters one tile using bilinear */
    auto processTileWithBilinearFilter = 
        [&](int rd, int tileRow) {

            bilinearFilterWithLUTBuff
                <
                PIXEL_T,
                NPPC,
                HIST_SIZE,
                TILE_H,
                TILE_W,
                ROWS,
                COLS,
                BUFF_R
                >
                (lut_buf,
                tile_buf[rd],            // in
                tile_buf[rd],            // out (in‑place ok)
                tileRow);                // lut_y
    };

    /* define a lambda function that dumps one tile row to OutImg.
     * The flatten loop implementation is derived from the following
     * triple nested loop: 
     * 
     * for (int i = 0; i < TILE_H; ++i)
     *  for (int t = 0; t < COLS; ++t)
     *      for (int j = 0; j < TILE_W; ++j)
     *          OutImg.write(tile_buf[rd][t][i * TILE_H + j]);
     * 
    */
    auto writeTileRowToOutImg = [&](int rd, int tileRow) {
        FLUSH_TILE_LOOP:
        #pragma HLS loop pipeline
        for (int idx = 0; idx < TILE_H * COLS * TILE_W; ++idx) {
            int i = idx / (COLS * TILE_W); // row index for a tile row
            int rem = idx % (COLS * TILE_W); // position inside a tile image
            int t = rem / TILE_W; // col
            int j = rem % TILE_W; // row
        
            OutImg.write(tile_buf[rd][t][i * TILE_H + j], 
                tileRow * TILE_H * W + t * TILE_W + i * TILE_H + j);
        }
    };

    //----------------------------------------------------------------
    for (int tileRow = 0; tileRow < ROWS; ++tileRow)
    {
        const int wr = tileRow & 1;           // 0 or 1 (ping‑pong)

        /* ---------------- read one tile‑row from stream -------------*/
        for (int t = 0; t < COLS; ++t){
            #pragma HLS loop pipeline
            for (int i=0;i<TILE_PIX;++i) tile_buf[wr][t][i] = pixel_fifo.read();
            #pragma HLS loop pipeline
            for (int i=0;i<HIST_SIZE;++i) lut_buf[wr][t][i] = lut_fifo.read();
        }

        /* nothing to output on the first tile‑row */
        if (tileRow == 0) continue;

        /* ---------------- process *previous* tile‑row ---------------*/
        const int rd = wr ^ 1;                // row written last time
        processTileWithBilinearFilter(rd, tileRow - 1);

        /* --------------- dump processed row to OutImg ---------------*/
        writeTileRowToOutImg(rd, tileRow - 1);
    }

    /* -------- process & dump the final (bottom) tile‑row -----------*/
    const int rd = (ROWS-1) & 1;
    processTileWithBilinearFilter(rd, ROWS - 1);
    writeTileRowToOutImg(rd, ROWS - 1);
}


/**
 * @brief Contrast-Limited Adaptive Histogram Equalisation (CLAHE) –  
 *        top-level, fully-streamed implementation.
 *
 * This wrapper instantiates the two streaming kernels:
 * - `ClaheLutStream()` (tile histogram + LUT producer) and
 * - `ClaheFilterStream()` (bilinear filter consumer) 
 * in a dataflow region so that they run concurrently on the FPGA fabric.
 * Only a pair of lightweight `hls::FIFO<uint8_t>` channels is used
 * between the stages; no frame buffers are allocated.
 *
 * ────────────────────────────────────────────────────────────────────
 * Template parameters
 * ────────────────────────────────────────────────────────────────────
 * @tparam HIST_SIZE   Size of histogram / LUT (usually 256).
 * @tparam PIXEL_T     Vivado-HLS pixel type (single-channel, 8-bit).
 * @tparam H, W        Frame height / width in pixels.
 * @tparam STORAGE     Must be `StorageType::FIFO` (streamed images).
 * @tparam NPPC        Pixels per clock – only 1 is supported.
 * @tparam TILE_H/W    Tile height / width in pixels (power-of-two).
 * @tparam CLIP_LIMIT  CLAHE clip limit (≤0 disables clipping).
 *
 * ────────────────────────────────────────────────────────────────────
 * Function arguments
 * ────────────────────────────────────────────────────────────────────
 * @param[in]  InImg   Continuous input image stream (`FIFO` storage
 *                     interface) containing @p H * @p W pixels.
 * @param[out] OutImg  Output stream; receives the CLAHE-enhanced frame
 *                     in the same order & format as @p InImg.
 *
 * ────────────────────────────────────────────────────────────────────
 * Internal operation
 * ────────────────────────────────────────────────────────────────────
 * 1. Parameter checks – static assertions guarantee a single
 *    greyscale channel, 8-bit depth and NPPC = 1.
 * 2. Constant derivation – number of tiles per row/column and the
 *    ping-pong buffer depth.
 * 3. FIFO creation –  
 *    • `pixel_FIFO` streams 1 pixel per beat  
 *    • `lut_FIFO`   streams the corresponding 256-entry LUT per tile  
 *    Depths are chosen for synthesis (`FIFO_DEPTH = 2`) and for the
 *    C/RTL co-simulation model (depth ≥ frame size).
 * 4. Dataflow region – the producer and consumer are launched in
 *    parallel; HLS schedules them so that back-pressure is handled
 *    automatically.
 *
 * The function is `static`-inlined by the compiler and introduces no
 * extra latency beyond the two kernel stages.  All line-buffering and
 * on-chip memories are contained inside the called sub-modules.
 */
template <
    int HIST_SIZE = 256,
    vision::PixelType PIXEL_T,
    int H,
    int W,
    vision::StorageType STORAGE,
    vision::NumPixelsPerCycle NPPC,
    int TILE_H,
    int TILE_W,
    int CLIP_LIMIT
>
void clahe(
    Img<PIXEL_T, H, W, STORAGE, NPPC> &InImg,
    Img<PIXEL_T, H, W, STORAGE, NPPC> &OutImg
) {
    #pragma HLS function dataflow
    #pragma HLS memory partition argument(InImg) type(struct_fields)
    #pragma HLS memory partition argument(OutImg) type(struct_fields)

    static_assert(DT<PIXEL_T, NPPC>::NumChannels == 1,
        "clahe function only supports one channel");
    static_assert(NPPC == 1,
        "clahe function only supports one pixel per cycle (NPPC = 1).");
    static_assert(DT<PIXEL_T, NPPC>::W == 8,
        "clahe function only supports 8 bits per channel");
    static_assert(STORAGE == vision::StorageType::FIFO,
        "clahe function only supports FIFO type");

    constexpr int TILE_PIX = TILE_W * TILE_H; // e.g. TILE_SIZE = 16, 256
    constexpr int COLS     = W  / TILE_W;     // e.g. TILE_SIZE = 16, 640/16 = 40
    constexpr int ROWS     = H / TILE_H;      // e.g. TILE_SIZE = 16, 480/16 = 30
    constexpr int BUFF_R   = 2;               // ping‑pong (upper / lower)
    constexpr int PIXELS   =  W * H;          // e.g. for 640x480 image, 307 200

#ifndef __SYNTHESIS__ 
// For software, the fifo depth has to be larger than 2
#define FIFO_DEPTH (PIXELS)
#else
#define FIFO_DEPTH 2
#endif

    hls::FIFO<uint8_t>              lut_FIFO                    ;
    hls::FIFO<ImgDT<PIXEL_T, NPPC>> pixel_FIFO                  ;
    ImgDT<PIXEL_T, NPPC>            tile_pix [COLS][TILE_PIX]   ;
    uint8_t                         tile_ready[COLS]            ;

    lut_FIFO.setDepth(HIST_SIZE * COLS * ROWS);
    pixel_FIFO.setDepth(FIFO_DEPTH);

    ClahePixelLoader
    <
        PIXEL_T,
        H,
        W,
        STORAGE,
        NPPC,
        HIST_SIZE,
        TILE_H,
        TILE_W,
        COLS
    >
    (InImg, tile_pix, tile_ready);

    printf("reached 1\n");

    ClaheLutStream
    <
        PIXEL_T,
        H,
        W,
        STORAGE,
        NPPC,
        HIST_SIZE,
        TILE_H,
        TILE_W,
        COLS,
        ROWS,
        CLIP_LIMIT
    >
    (tile_pix, tile_ready, pixel_FIFO, lut_FIFO);
    

    printf("reached 2\n");

    ClaheFilterStream
    <
        PIXEL_T,
        H,
        W,
        STORAGE,
        NPPC,
        HIST_SIZE,
        TILE_H,
        TILE_W,
        COLS,
        ROWS
    >
    (pixel_FIFO, lut_FIFO, OutImg);
}

} // End of namespace vision.
} // End of namespace hls.