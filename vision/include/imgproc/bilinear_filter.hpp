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
#include "bilinear_table.hpp"
#include <hls/ap_fixpt.hpp>
#include "../common/line_buffer.hpp"

namespace hls {
namespace vision {

/***********************************************************************************
 * @brief  Utility that checks whether the sliding-window kernel (or any
 *         2-D buffer that is scanned left-to-right, top-to-bottom) has
 *         already received the pixel that sits in its *centre* position.
 *
 * Template parameters
 * -------------------
 * @tparam WIDTH   Compile-time constant: total number of pixels in one
 *                 row of the image / line-buffer that feeds the window.
 *
 * Parameters
 * ----------
 * @param kernel_size  Run-time kernel dimension K (for a square K × K window).
 * @param count        Linear write counter that starts at 0 for the very
 *                     first pixel of the image and is incremented after
 *                     every pixel write into the line-buffer / window.
 *
 * Behaviour
 * ---------
 *                              ┌───────────┐
 *                              │   K × K   │
 *  Pixels are streamed like →  │           │
 *  a raster scanner (row-major)├───┬───┬───│
 *                              │...│ C │...│
 *                              └───────────┘
 *
 * The function computes the linear index of the centre pixel **C**
 * inside that raster order:
 *
 *     centre_index = row_of_C * WIDTH + col_of_C
 *                  = (K/2)    * WIDTH + (K/2)  – 1
 *                                └─────────────┘
 *                             zero-based column index
 *
 * and simply returns whether the counter `count` has already
 * passed that index.  In other words, it answers the question:
 * “Has the kernel been filled up to its centre position yet?”
 *
 * @retval true   when the caller can safely read/use the centre pixel
 *                (and therefore the whole valid K × K neighbourhood).
 * @retval false  when the kernel is still being primed with the first
 *                few rows/columns of the image.
 *
 ************************************************************************************/
template <int WIDTH>
bool is_filled(unsigned int kernel_size, unsigned int count) {
    unsigned int center = kernel_size / 2;
    return count > (center * WIDTH + center - 1);
}

/*************************************************************************************
 * @brief  Boundary-check for a centred convolution kernel.
 *
 * For a kernel of size @p kernel_size (e.g. 3 × 3, 5 × 5 …) centred on the
 * image pixel with coordinates (i , j), this helper determines whether the
 * kernel would extend beyond the image borders of an image with compile-time
 * dimensions @p HEIGHT × @p WIDTH.
 *
 * A kernel is considered out-of-bounds when any of the following is true
 * (see illustration below):
 *
 * ```
 *            top border            bottom border
 *          ┌─────────────────────────────────────┐
 *   i < c  │  rows missing above the image       │
 *          │                                     │
 *        … │ …                               …   │  i > HEIGHT-c-1
 *          │                                     │
 *          └─────────────────────────────────────┘
 *             j < c                j > WIDTH-c-1  → columns missing
 * ```
 *
 * where c = kernel_size / 2< is the distance from the kernel centre to
 * its edges.
 *
 * @tparam WIDTH   Compile-time image width  (columns).
 * @tparam HEIGHT  Compile-time image height (rows).
 *
 * @param  kernel_size  Full side length of the square kernel (must be odd).
 * @param  i            Row index   of the kernel centre (0 … HEIGHT-1).
 * @param  j            Column index of the kernel centre (0 … WIDTH-1).
 *
 * @return `true`  if the kernel would touch / cross the image boundary,  
 *         `false` otherwise.
 *
 * The check is performed with four simple comparisons combined by logical *OR*;
 * therefore the function can be inlined and synthesised into a single
 * combinational expression in HLS flows.
 * 
 **************************************************************************************/
template <
    int WIDTH,
    int HEIGHT
>
bool is_out_of_bounds(
    unsigned int kernel_size,
    unsigned int i,
    unsigned int j
) {
    unsigned int center = kernel_size / 2;
    return (i < center) | (i > (HEIGHT - center - 1)) | (j < center) |
            (j > (WIDTH - center - 1));
}

/**************************************************************************************
* @brief  Advance pixel coordinates in raster-scan order.
*
*         The helper walks through an image one pixel at a time in left-to-right,
*         top-to-bottom fashion ( **raster scan** ).  After the last pixel of the
*         lower-right corner is reached, it wraps around to (0, 0) so that the same
*         counter pair can be reused for the next image frame.
*
* @tparam WIDTH   Logical width  of the image in pixels.
* @tparam HEIGHT  Logical height of the image in pixels.
*
* @param[in,out]  i   Row   index ( 0 … HEIGHT – 1 ).  
*                     • On entry  – current y-coordinate.  
*                     • On return – updated y-coordinate.
*
* @param[in,out]  j   Column index ( 0 … WIDTH – 1 ).  
*                     • On entry  – current x-coordinate.  
*                     • On return – updated x-coordinate.
*
* @details
*   ┌──────── Case 1 ────────┐  If the current pixel is not at the end of a row
*   │   ( j < WIDTH-1 )      │  ➜ increment the column counter j only.
*   ├──────── Case 2 ────────┤  If the current pixel is the very last pixel of the
*   │ ( i == HEIGHT-1        │  frame (bottom-right corner)
*   │   && j == WIDTH-1 )    │  ➜ wrap both counters back to (0, 0).
*   ├───────── Case 3 ───────┤  Otherwise we are at the end of a row but not the last
*   │  ( j == WIDTH-1 )      │  row ➜ advance to the first column of the next row:
*   └────────────────────────┘  increment i, reset j to 0.
*
* @note  The function operates purely on the two reference parameters; it keeps no
*        internal static state and therefore is thread-safe and synthesises to simple
*        combinational / FSM logic in HLS.
***************************************************************************************/
template <int WIDTH, int HEIGHT>
void update_image_position(unsigned int &i, unsigned int &j) {
    if (j < WIDTH - 1) {
        // Case 1: Moving to next pixel from left to right across image row.
        j++;
    } else if (i == HEIGHT - 1 && j == WIDTH - 1) {
        // Case 2: End of the image frame, reset to the first pixel coordinates
        // for the next image frame.
        i = 0;
        j = 0;
    } else {
        // Case 3: End of image row. Move to the first pixel of one row down in
        // the image frame.
        i++;
        j = 0;
    }
}

/***************************************************************************//**
 * @brief   Bilinear-resize a whole image that lives in a **frame buffer**.
 *
 * The kernel reads four neighbouring source pixels for every destination
 * coordinate and performs classic bilinear interpolation
 * 
 *  I(x,y) = (1 - x_d) * (1 - y_d) * P1
 *         +  x_d * (1 - y_d)      * P2
 *         + (1 - x_d) * y_d       * P3
 *         +  x_d * y_d            * P4
 * 
 * where P1 … P4 are the top-left, top-right, bottom-left and bottom-right
 * samples returned by the pre-computed Neighbour-Pixel Table (i.e. NPTable).
 *
 * The routine is synthesised only when the image objects are stored as a
 * frame buffer (`StorageType::FRAME_BUFFER`).  
 * For streaming images a different overload is provided elsewhere.
 *
 * Latency is reduced by pipelining the main loop (`#pragma HLS loop pipeline`)
 * and replicating the six ROM look-up tables inside NPTable so every cycle
 * delivers all required neighbours and fractional weights.
 *
 * @tparam STORAGE_TYPE   Image storage back-end (must be
 *                        `StorageType::FRAME_BUFFER` for this overload)
 * @tparam NPPC           Number-of-pixels processed per clock (only `1`
 *                        tested)
 * @tparam PIXEL          Pixel format (e.g. `PixelType::HLS_8UC1`)
 * @tparam H_DST,W_DST    Height/width of destination image
 * @tparam H_SRC,W_SRC    Height/width of source image
 * @param[out] OutImg     Destination image (frame-buffer).  Must be pre-created
 *                        with size @p W_DST × @p H_DST.  Each pixel is overwritten
 *                        exactly once.
 * @param[in]  InImg      Source image (frame-buffer) of size @p W_SRC × @p H_SRC .
 *
 * @details
 * * `NeighborPixelTable` is a compile-time generated LUT that, for every
 *   destination coordinate *i*, supplies:
 *      * indices of the four contributing source pixels (`getTopLeftPixel`
 *        … `getBottomRightPixel`)
 *      * the fixed-point horizontal (`getXDiff`) and vertical (`getYDiff`)
 *        sub-pixel offsets *x_d, y_d* in the range [0,1).
 * * `ufixpt_t` (Q4.12) is sufficient for the weights and for the
 *   intermediate products \( x_d(P_2-P_1) \) etc.  The final value is clamped
 *   to 0…255 before being written.
 *
 * The loop runs once per destination pixel, so the total latency is
 * @p H_DST × @p W_DST cycles minus the pipeline depth.
 ******************************************************************************/
template <
    StorageType STORAGE_TYPE, // Storage Type
    NumPixelsPerCycle NPPC,   // Pixels Per Cycle
    PixelType PIXEL,          // Pixel Type
    int H_DST,
    int W_DST,
    int H_SRC, 
    int W_SRC,
    typename std::enable_if<(STORAGE_TYPE == StorageType::FRAME_BUFFER)>::type * = nullptr
>
static void BilinearFilterProcess(
    Img<PIXEL, H_SRC, W_SRC, STORAGE_TYPE, NPPC> &InImg,
    Img<PIXEL, H_DST, W_DST, STORAGE_TYPE, NPPC> &OutImg
) {

    using ufixpt_t = hls::ap_ufixpt<16,12>;
    constexpr unsigned imageSize = H_DST * W_DST;

    #pragma HLS memory replicate_rom variable(NPTable.TopLeftTable) max_replicas(0)
    #pragma HLS memory replicate_rom variable(NPTable.TopRightTable) max_replicas(0)
    #pragma HLS memory replicate_rom variable(NPTable.BottomLeftTable) max_replicas(0)
    #pragma HLS memory replicate_rom variable(NPTable.BottomRightTable) max_replicas(0)
    #pragma HLS memory replicate_rom variable(NPTable.xDiffTable) max_replicas(0)
    #pragma HLS memory replicate_rom variable(NPTable.yDiffTable) max_replicas(0)
    constexpr auto NPTable = NeighborPixelTable<imageSize, W_SRC, H_SRC, W_DST, H_DST>{};

HLS_VISION_BILINEAR_LOOP:
    #pragma HLS loop pipeline
    for(int i = 0; i < imageSize; ++i) {

        int y = static_cast<int>(i / W_DST);
        int x = static_cast<int>(i % W_DST);

        int top_left_index = NPTable.getTopLeftPixel(i);
        int top_right_index = NPTable.getTopRightPixel(i);
        int bottom_left_index = NPTable.getBottomLeftPixel(i);
        int bottom_right_index = NPTable.getBottomRightPixel(i);

        // Read the four neighboring pixel values
        ap_uint<8> P1 = InImg.read(top_left_index); // Top-left
        ap_uint<8> P2 = InImg.read(top_right_index); // Top-right
        ap_uint<8> P3 = InImg.read(bottom_left_index); // Bottom-left
        ap_uint<8> P4 = InImg.read(bottom_right_index); // Bottom-right

        // Bilinear interpolation calculation
        ufixpt_t x_diff = NPTable.getXDiff(i);
        ufixpt_t y_diff = NPTable.getYDiff(i);
        ufixpt_t R1 = P1 + x_diff * (P2 - P1);
        ufixpt_t R2 = P3 + x_diff * (P4 - P3);
        ufixpt_t interpolated_value = R1 + y_diff * (R2 - R1);

        // Clamping the interpolated value to the valid range [0, 255]
        int value = std::min(255, std::max(0, static_cast<int>(interpolated_value)));

        // Write the interpolated value to the destination image
        OutImg.write(value, y * W_DST + x);
    }
}


#define FIXPT_W     40
#define FIXPT_I     12
using fixpt_t = hls::ap_fixpt<FIXPT_W, FIXPT_I, AP_RND, AP_SAT>;

/**
 * @brief Computes the mathematical floor of a fixed-point value.
 *
 * This function extracts the integer part of the fixed-point number v
 * without invoking any floating-point conversions, then corrects
 * for negative values with nonzero fractional bits to produce the true
 * floor (i.e., rounding toward −∞).
 *
 * @tparam fixpt_t  Arbitrary-precision fixed-point type (e.g. Q16.8).
 * @param v         The input fixed-point value.
 * @return          The largest integer not greater than v.
 *
 * Internally:
 * 1.  Retrieve the raw two’s-complement bit representation of v.
 * 2.  Shift right by the fixed fractional width to truncate toward zero.
 * 3.  Cast to an int (truncation = floor for positive v, ceil for negative v).
 * 4.  If v is negative and had any fractional remainder, subtract one to
 *     obtain the true floor.
 */
int floor_fx(fixpt_t v)
{
    #pragma HLS function replicate
    constexpr int W = FIXPT_W;             // Total bit-width of fixpt_t
    constexpr int I = FIXPT_I;             // Integer bit-width
    constexpr int F = W - I;               // Fractional bit-width

    // 1) Grab the raw bits
    hls::ap_int<W> raw = v.raw_bits();

    // 2) Drop the fraction by shifting right
    hls::ap_int<I> ip  = raw >> F;

    // 3) Truncate toward zero
    int ipart = static_cast<int>(ip);

    // 4) Adjust for negative values with leftover fraction
    return (v < 0 && v != fixpt_t(ipart)) ? ipart - 1 : ipart;
}

/**
 * @brief Computes the fractional part of a fixed-point value.
 *
 * Returns v − floor(v), preserving the same fixed-point type.
 *
 * @param v  The input fixed-point value.
 * @return   A fixed-point value in [0,1) representing the fractional
 *           remainder of v.
 *
 * This is computed by subtracting the integer component (from floor_fx)
 * from the original v.
 */
fixpt_t frac_fx(fixpt_t v)
{
    #pragma HLS function replicate
    return v - fixpt_t(floor_fx(v));
}

/********************************************************************
 * @brief BilinearFilterProcess – Resize and interpolate an image using
 *        bilinear filtering (FIFO-based, banked storage for II=1)
 * ------------------------------------------------------------------
 * This function reads an entire source image from a FIFO-based Img,
 * partitions it into four banks by (row%2, col%2) parity to allow
 * four concurrent reads, and then streams out a resized image via
 * bilinear interpolation at one output pixel per clock cycle.
 *
 * Template Parameters:
 *   STORAGE_TYPE – must be StorageType::FIFO (enforced by SFINAE)
 *   NPPC         – number of pixels processed per cycle (must be 1)
 *   PIXEL        – the HLS pixel data type (e.g. HLS_8UC1)
 *   H_DST, W_DST – height and width of the destination image
 *   H_SRC, W_SRC – height and width of the source image
 *
 * Function Arguments:
 *   InImg  – input Img<PIXEL, H_SRC, W_SRC, STORAGE_TYPE, NPPC>
 *            FIFO from which source pixels are read
 *   OutImg – output Img<PIXEL, H_DST, W_DST, STORAGE_TYPE, NPPC>
 *            FIFO into which interpolated pixels are written
 *
 * Behavior:
 *   1. Read H_SRC×W_SRC pixels from InImg into four RAM banks:
 *        bank00[y/2][x/2] ← pixel where y%2=0, x%2=0
 *        bank01[y/2][x/2] ← y%2=0, x%2=1
 *        bank10[y/2][x/2] ← y%2=1, x%2=0
 *        bank11[y/2][x/2] ← y%2=1, x%2=1
 *   2. Compute scale factors:
 *        X_RATIO = W_SRC / W_DST
 *        Y_RATIO = H_SRC / H_DST
 *   3. Initialize fixed-point accumulators:
 *        src_x = (0.5)*X_RATIO − 0.5
 *        src_y = (0.5)*Y_RATIO − 0.5
 *   4. For each of the DST_PIXELS = H_DST×W_DST output pixels:
 *        a) src_x += X_RATIO
 *           src_y only increments by Y_RATIO when x_dst wraps
 *        b) Extract integer/fractional parts with floor_fx()/frac_fx()
 *        c) Clamp sample coordinates to [0…W_SRC-1]×[0…H_SRC-1]
 *        d) Compute four bank IDs and bank addresses once
 *        e) Perform exactly one RAM read per bank:
 *             b00 = bank00[y0/2][x0/2], …, b11 = bank11[y1/2][x1/2]
 *        f) Select P00…P11 from b00…b11 in registers (no extra RAM)
 *        g) Compute bilinear mix:
 *             v0 = (1−rx)*P00 + rx*P01
 *             v1 = (1−rx)*P10 + rx*P11
 *             out = (1−ry)*v0  + ry*v1
 *        h) Round and write to OutImg
 *
 * Notes:
 *   - Four-bank layout and one-read-per-bank eliminate BRAM conflicts,
 *     enabling II=1.
 *   - Uses fixed-point (hls::ap_fixpt) for all arithmetic.
 ********************************************************************/
template <
    StorageType         STORAGE_TYPE,
    NumPixelsPerCycle   NPPC,
    PixelType           PIXEL,
    int                 H_DST,
    int                 W_DST,
    int                 H_SRC,
    int                 W_SRC,
    typename std::enable_if<(STORAGE_TYPE == StorageType::FIFO)>::type* = nullptr
>
static void BilinearFilterProcess(
    Img<PIXEL, H_SRC, W_SRC, STORAGE_TYPE, NPPC>& InImg,
    Img<PIXEL, H_DST, W_DST, STORAGE_TYPE, NPPC>& OutImg
) {
    using pix_t   = typename DT<PIXEL, NPPC>::T;
    fixpt_t rounding = 0.5;
    const fixpt_t X_RATIO = static_cast<fixpt_t>(W_SRC) / W_DST;
    const fixpt_t Y_RATIO = static_cast<fixpt_t>(H_SRC) / H_DST;

    // Read the entire input image into a buffer
    constexpr int SRC_PIXELS = H_SRC * W_SRC;
    int  x = 0;                 // column   0 … W_SRC-1
    int  y = 0;                 // row      0 … H_SRC-1

    // Storage split into 4 banks based on (row, col) parity
    pix_t bank00[(H_SRC+1)>>1][(W_SRC+1)>>1];   // y even, x even
    pix_t bank01[(H_SRC+1)>>1][(W_SRC   )>>1];  // y even, x odd
    pix_t bank10[(H_SRC   )>>1][(W_SRC+1)>>1];  // y odd, x even
    pix_t bank11[(H_SRC   )>>1][(W_SRC   )>>1]; // y odd, x odd

/* ---------- load-pixel-per-cycle pipeline ------------------------------- */
FILL_INPUT_LOOP:
    #pragma HLS loop pipeline
    for (int i = 0; i < SRC_PIXELS; ++i)
    {
        pix_t p = InImg.read();
        bool x_odd = x & 1;
        bool y_odd = y & 1;
        int xi = x >> 1, yi = y >> 1;

        if (!y_odd && !x_odd) bank00[yi][xi] = p;
        else if (!y_odd && x_odd) bank01[yi][xi] = p;
        else if (y_odd && !x_odd) bank10[yi][xi] = p;
        else bank11[yi][xi] = p;

        if (++x == W_SRC) { x = 0; ++y; }
    }

    /* ---------- constants & first-row initialisation ----------------------- */
    constexpr int   DST_PIXELS = H_DST * W_DST;

    const fixpt_t   src_x0   = (rounding * X_RATIO) - rounding;   // x at (0,0)
    const fixpt_t   src_y0   = (rounding * Y_RATIO) - rounding;   // y at (0,0)

    const fixpt_t   x_step   = X_RATIO;   // advance in source-space per dst-pixel
    const fixpt_t   y_step   = Y_RATIO;   // advance per dst-row

    fixpt_t src_x  = src_x0 - x_step;     // pre-decrement → first + adds x0
    fixpt_t src_y  = src_y0;

    int     x_dst  = 0;                   // 0 … W_DST-1
    int     y_dst  = 0;                   // 0 … H_DST-1

    /* ---------- one-pixel-per-cycle pipeline ------------------------------- */
    SCALE_LOOP:
    #pragma HLS loop pipeline
    for (int i = 0; i < DST_PIXELS; ++i)
    {
        /* --- accumulate source coordinates (no mult/div per pixel) -------- */
        src_x += x_step;

        /* integer / fractional parts – NO double, NO floor() ---------------*/
        int     x_i = floor_fx(src_x);
        fixpt_t rx  = frac_fx(src_x);

        int     y_i = floor_fx(src_y);
        fixpt_t ry  = frac_fx(src_y);

        /* -------------- (clamp) edge-replicated indices --------------------------- */
        int x0 = (x_i   < 0)      ? 0 : (x_i   > W_SRC-1 ? W_SRC-1 : x_i   );
        int x1 = (x_i+1 < 0)      ? 0 : (x_i+1 > W_SRC-1 ? W_SRC-1 : x_i+1);
        int y0 = (y_i   < 0)      ? 0 : (y_i   > H_SRC-1 ? H_SRC-1 : y_i   );
        int y1 = (y_i+1 < 0)      ? 0 : (y_i+1 > H_SRC-1 ? H_SRC-1 : y_i+1);

        /*------------------------------------------------------------------*/
        /*  Edge replication happens?  Make the corresponding weight zero   */
        /*------------------------------------------------------------------*/
        if (x0 == x1)   rx = fixpt_t(0);   // both taps hit same column
        if (y0 == y1)   ry = fixpt_t(0);   // both taps hit same row

        /* -------------- fetch 4 neighbours -------------------------------- */

        /* ───── compute bank id & addresses once ───── */
        auto bankID   = [](int y,int x){ return ((y & 1) << 1) | (x & 1); };
        uint8_t id00 = bankID(y0,x0);   uint8_t id01 = bankID(y0,x1);
        uint8_t id10 = bankID(y1,x0);   uint8_t id11 = bankID(y1,x1);

        /* addresses of all four taps */
        uint16_t r00 = y0>>1, c00 = x0>>1;
        uint16_t r01 = y0>>1, c01 = x1>>1;
        uint16_t r10 = y1>>1, c10 = x0>>1;
        uint16_t r11 = y1>>1, c11 = x1>>1;

        /* ── ONE read per bank – address chosen with a ternary chain ───── */
        pix_t b00 = bank00[
            (id00==0)    ? r00 :
            (id01==0)    ? r01 :
            (id10==0)    ? r10 :
                        r11
        ][
            (id00==0)    ? c00 :
            (id01==0)    ? c01 :
            (id10==0)    ? c10 :
                        c11
        ];

        pix_t b01 = bank01[
            (id00==1)    ? r00 :
            (id01==1)    ? r01 :
            (id10==1)    ? r10 :
                        r11
        ][
            (id00==1)    ? c00 :
            (id01==1)    ? c01 :
            (id10==1)    ? c10 :
                        c11
        ];

        pix_t b10 = bank10[
            (id00==2)    ? r00 :
            (id01==2)    ? r01 :
            (id10==2)    ? r10 :
                        r11
        ][
            (id00==2)    ? c00 :
            (id01==2)    ? c01 :
            (id10==2)    ? c10 :
                        c11
        ];

        pix_t b11 = bank11[
            (id00==3)    ? r00 :
            (id01==3)    ? r01 :
            (id10==3)    ? r10 :
                        r11
        ][
            (id00==3)    ? c00 :
            (id01==3)    ? c01 :
            (id10==3)    ? c10 :
                        c11
        ];

        /* ── register-level selection – no further BRAM reads ─────────────*/
        auto sel = [&](uint8_t id)->pix_t {
            switch(id){ case 0: return b00; case 1: return b01;
                        case 2: return b10; default: return b11; }
        };

        pix_t P00 = sel(id00);
        pix_t P01 = sel(id01);
        pix_t P10 = sel(id10);
        pix_t P11 = sel(id11);

        /* -------------- bilinear weights ---------------------------------- */
        fixpt_t w00 = (fixpt_t(1) - rx) * (fixpt_t(1) - ry);
        fixpt_t w01 =              rx  * (fixpt_t(1) - ry);
        fixpt_t w10 = (fixpt_t(1) - rx) *              ry ;
        fixpt_t w11 =              rx  *              ry ;

        /* -------------- interpolate & write ------------------------------- */
        fixpt_t val = w00*P00 + w01*P01 + w10*P10 + w11*P11;
        OutImg.write( pix_t( val + rounding ) );

        /* -------------- advance dst-pixel counters ------------------------ */
        if (++x_dst == W_DST) {
            /* end of destination row → go to next row */
            x_dst  = 0;
            ++y_dst;

            src_x  = src_x0 - x_step;   // start of new row (pre-decrement trick)
            src_y += y_step;            // exactly +Y_RATIO each row
        }
    }
}

/******************************************************************************
* @brief  Scales an input image to a new resolution using bilinear
*         interpolation and writes the result to an output image object.
*
*         The routine is a thin, type–safe wrapper that performs
*         `static_assert` checks (channel count, word-width, NPPC) and then
*         delegates the actual work to `BilinearFilterProcess`.  
*         All I/O happens through the SmartHLS `Img<>` FIFO interface so the
*         function is synthesizable without extra glue code.
*
* @tparam STORAGE_TYPE   Storage backend used by the SmartHLS image wrapper
*                        (`StorageType::FIFO`, `StorageType::FRAME_BUFFER`).
* @tparam NPPC           *Pixels-per-cycle* the design is compiled for
*                        (only 1 pixel / cycle is accepted).
* @tparam PIXEL          Pixel format (must be 8-bit, one channel).
* @tparam H_DST          Height  of the destination image  (rows).
* @tparam W_DST          Width   of the destination image  (columns).
* @tparam H_SRC          Height  of the source image (rows).
* @tparam W_SRC          Width   of the source image (columns).
*
* @param[out] OutImg     SmartHLS image object that will receive the
*                        @p H_DST × @p W_DST bilinearly-filtered output stream.
* @param[in]  InImg      SmartHLS image object providing the
*                        @p H_SRC × @p W_SRC input pixel stream.
*
* @details
* *   The function performs **compile-time checks** to guarantee that the
*     component is used with a monochrome, 8-bit image stream and a single
*     pixel per clock cycle.
* *   The interpolation weights are derived exactly as in OpenCV
*     (`(x+0.5)/scale − 0.5`), so the output matches software reference
*     results down to the LSB when the downstream pipeline keeps the same
*     precision.
* *   No runtime heap is used; all buffering happens inside `OutImg`,
*     `InImg`, and the data-flowed `BilinearFilterProcess` kernel.
******************************************************************************/
template <
    StorageType STORAGE_TYPE, // Storage Type
    NumPixelsPerCycle NPPC,   // Pixels Per Cycle
    PixelType PIXEL,          // Pixel Type
    int H_DST,
    int W_DST,
    int H_SRC,
    int W_SRC
>
void BilinearFilter(
    Img<PIXEL, H_SRC, W_SRC, STORAGE_TYPE, NPPC> &InImg,
    Img<PIXEL, H_DST, W_DST, STORAGE_TYPE, NPPC> &OutImg
) {

    static_assert(DT<PIXEL, NPPC>::NumChannels == 1,
        "BilinearFilter function only supports one channel");

    static_assert(NPPC == 1,
        "BilinearFilter function only supports one pixel per cycle (NPPC = 1).");

    static_assert(DT<PIXEL, NPPC>::W == 8, 
        "BilinearFilter function only supports 8 bits per channel");

    BilinearFilterProcess
    <
        STORAGE_TYPE,
        NPPC,
        PIXEL,
        H_DST,
        W_DST,
        H_SRC,
        W_SRC
    >
    (InImg, OutImg);
}

} // End of namespace vision.
} // End of namespace hls.