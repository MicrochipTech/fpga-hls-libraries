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
// MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE
// FORESEEABLE.  TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP’S TOTAL
// LIABILITY ON ALL CLAIMS LATED TO THE SOFTWARE WILL NOT EXCEED AMOUNT OF
// FEES, IF ANY, YOU PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE. MICROCHIP
// OFFERS NO SUPPORT FOR THE SOFTWARE. YOU MAY CONTACT MICROCHIP AT
// https://www.microchip.com/en-us/support-and-training/design-help/client-support-services
// TO INQUIRE ABOUT SUPPORT SERVICES AND APPLICABLE FEES, IF AVAILABLE.

#pragma once 

#include "common.hpp"
#include "line_buffer.hpp"
#include "hls/ap_fixpt.hpp"
#include "params.hpp"
#include <type_traits> // Needed for std::enable_if

namespace hls {
namespace vision {

constexpr unsigned FloorLog2(unsigned long long x) {
    return x == 1 ? 0 : 1 + FloorLog2(x >> 1);
}

template <unsigned long long N> 
struct CeilLog2 {
    static constexpr unsigned int value = 1 + CeilLog2<(N + 1) / 2>::value;
};

template <>
struct CeilLog2<1> {
    static constexpr unsigned int value = 0;
};


/****************************************************************************************
 * ImgToImgProcess
 * --------------------------------------------------------------------------------------
 * Copy/route a rectangular Region-Of-Interest (ROI) from a streamed source image
 * (InImg) into a destination image (OutImg).
 *
 * The function is intended for AXI-FIFO based designs:
 *   • Both images use FIFO storage (`StorageType::FIFO`).
 *   • One pixel is transferred per clock (`NPPC == 1` is enforced elsewhere).
 *
 *  ⌈rowBound × colBound⌉ pixels are copied:
 *      – The ROI’s top-left pixel in the source starts at linear index
 *        `srcStart`  (0-based, row-major).
 *      – Those pixels are written into the destination starting at index
 *        `dstStart`.
 *
 *  All pixels that do not belong to the ROI are simply read and discarded,
 *  so the read pointer of InImg still advances monotonically.  The function
 *  therefore preserves throughput and FIFO ordering while avoiding any random
 *  access to the source.
 *
 * Template parameters
 * -------------------
 *  PIXEL_T     : Pixel data type (e.g. `PixelType::HLS_8UC1`).
 *  H_SRC/W_SRC : Height and width of the source image.
 *  H_DST/W_DST : Height and width of the destination image.
 *  STORAGE     : Must be `StorageType::FIFO`.
 *  NPPC        : Pixels processed per cycle (design assumes 1).
 *
 * Function arguments
 * ------------------
 *  InImg  (in)   : FIFO-backed source image.
 *  OutImg (out)  : FIFO-backed destination image that receives the ROI.
 *  rowBound      : Number of rows to copy.
 *  colBound      : Number of columns to copy.
 *  srcStart      : Linear start index of the ROI inside InImg
 *                  ( = y_src*W_SRC + x_src ).
 *  dstStart      : Linear start index inside OutImg where the ROI is written.
 *
 * Notes
 * -----
 *  • The template is SFINAE-guarded so that instantiation is only possible for
 *    FIFO-based images.
 *  • Indices are linear row-major offsets, not (row,col) pairs.
 *  • Internal variables:
 *       - `srcEnd`   : last source index of the ROI.
 *       - `dropBound`: number of pixels to skip at the end of each ROI row.
 *       - `dropCount`: run-time counter implementing the skip logic.
 *       - `currStart`: first source index of the current ROI row.
 ****************************************************************************************/
template <
    PixelType PIXEL_T,                  // Pixel Type
    int H_SRC,                          // Input Image Height
    int W_SRC,                          // Input Image Width
    int H_DST,                          // Output Image Height
    int W_DST,                          // Output Image Width
    StorageType STORAGE,                // Storage Type
    NumPixelsPerCycle NPPC,             // Pixels Per Cycle
    typename std::enable_if<(STORAGE == StorageType::FIFO)>::type * = nullptr
>
void ImgToImgProcess(
    Img<PIXEL_T, H_SRC, W_SRC, STORAGE, NPPC> &InImg,
    Img<PIXEL_T, H_DST, W_DST, STORAGE, NPPC> &OutImg,
    int rowBound,
    int colBound,
    int srcStart,
    int dstStart
) {
    constexpr int imgSize = H_SRC * W_SRC / NPPC;
    // calculate the last image index to be copied
    int srcEnd = srcStart + (rowBound - 1) * W_SRC + colBound - 1;
    const int dropBound = W_SRC - rowBound;
    int dropCount = dropBound;
    int currStart = srcStart;
    
    /**
     * Use dropCount to skip pixels that are off interest. 
     * Use currStart to keep track of the latest starting
     * index of a given row. When dropCount reaches 0 it means
     * the next row is reached then currStart is updated.
     */

    #pragma HLS loop pipeline
    for(int i = 0; i < imgSize; ++i) {
        auto val = InImg.read(i);
        if(i >= currStart && i <= srcEnd) {
            if(i - currStart < colBound) {
                OutImg.write(val, i);
            } else {
                dropCount--;
                if(dropCount == 0) {
                    dropCount = dropBound;
                    currStart = i + 1;
                }
            }
        }
    }
}

/******************************************************************************
 * ImgToImgProcess
 * ----------------------------------------------------------------------------
 *  Copies a rectangular region of pixels from one frame–buffer image
 *  (`OutImg`) into another (`OutImg`).  Both images use the same pixel format
 *  (`PIXEL_T`) and are stored in on-chip frame buffers
 *  (`StorageType::FRAME_BUFFER`) that can be accessed at random addresses.
 *
 *  The copy is purely spatial: no colour conversion, filtering, or scaling is
 *  performed.  Pixels are transferred *row-by-row* beginning at the source
 *  index `srcStart` and written to the destination beginning at
 *  `dstStart`.  The region size is defined by `rowBound × colBound`.
 *
 *  Template parameters
 *  -------------------
 *  PIXEL_T   : SmartHLS pixel-format tag (e.g. PixelType::HLS_8UC1).
 *  H_SRC     : Height  (rows) of the source image.
 *  W_SRC     : Width   (columns) of the source image.
 *  H_DST     : Height  (rows) of the destination image.
 *  W_DST     : Width   (columns) of the destination image.
 *  ST        : Must be StorageType::FRAME_BUFFER (compile-time guard).
 *  NPPC      : Number of pixels processed per clock cycle (only NPPC = 1
 *              is sensible for a random-access frame buffer).
 *
 *  Function arguments
 *  ------------------
 *  InImg  [in]   : Source image object from which the region is copied.
 *  OutImg  [out] : Destination image object into which the region is copied.
 *  rowBound      : Number of rows to copy (height of the region).
 *  colBound      : Number of columns to copy (width  of the region).
 *  srcStart      : Linear index in `OutImg` of the first pixel to copy
 *                  (top-left corner of the region) —
 *                  calculated as  `src_y * W_SRC + src_x`.
 *  dstStart      : Linear index in `OutImg` of the destination position for
 *                  the first pixel — calculated as
 *                  `dst_y * W_DST + dst_x`.
 *
 *  Behaviour
 *  ---------
 *  For every pixel position (i,j) inside the region:
 *
 *     • `srcIndex = srcStart + i * W_SRC + j`
 *     • `dstIndex = dstStart + i * W_DST + j`
 *     • The pixel read from   `InImg[srcIndex]`
 *       is written to         `OutImg[dstIndex]`
 *
 *  Notes
 *  -----
 *  * The function is enabled only when the storage type is FRAME_BUFFER
 *    via `std::enable_if`.
 ******************************************************************************/
template <
    PixelType PIXEL_T,              // Pixel Type
    int H_SRC,                      // Input Image Height
    int W_SRC,                      // Input Image Width
    int H_DST,                      // Output Image Height
    int W_DST,                      // Output Image Width
    StorageType STORAGE,            // Storage Type
    NumPixelsPerCycle NPPC,         // Pixels Per Cycle
    typename std::enable_if<(STORAGE == StorageType::FRAME_BUFFER)>::type * = nullptr
>
void ImgToImgProcess(
    Img<PIXEL_T, H_SRC, W_SRC, STORAGE, NPPC> &InImg,
    Img<PIXEL_T, H_DST, W_DST, STORAGE, NPPC> &OutImg,
    int rowBound,
    int colBound,
    int srcStart,
    int dstStart
) {
    /* ---------------------------------------------------------------
     *  Main copy loop – row by row
     * ------------------------------------------------------------- */
    int srcIndex;
    int dstIndex;
    for(int i = 0; i < rowBound; i++) {
        for(int j = 0; j < colBound; j++) {
            srcIndex = srcStart + i * W_SRC + j;
            dstIndex = dstStart + i * W_DST + j;
            auto curPix = InImg.read(srcIndex);
            OutImg.write(curPix, dstIndex);
        }
    }
}

/******************************************************************************
 * ImgToImg – Copy / paste a rectangular Region-Of-Interest (ROI) from one
 *             image into another image of (possibly) different dimensions.
 *
 *   •  The function clips the requested ROI automatically so that:
 *        – it never extends beyond the source-image boundaries, and
 *        – it never extends beyond the destination-image boundaries.
 *   •  Coordinates are given in flattened 1-D “pixel index” notation
 *      (row major, 0 … rows*cols-1).  Internally they are converted to
 *      (row , col) pairs.
 *   •  All compile-time image properties (pixel type, resolution,
 *      storage, pixels-per-cycle) are expressed through template
 *      parameters so the implementation remains fully static and
 *      synthesiser-friendly.
 *
 * ---------------------------------------------------------------------------
 *  Template parameters
 *  -------------------
 *  PIXEL_T      Pixel format used by both images (e.g. HLS_8UC1, HLS_8UC3 …)
 *  H_SRC ,W_SRC Height and width of the source image  (compile-time)
 *  H_DST ,W_DST Height and width of the destination image (compile-time)
 *  STORAGE      Memory interface (only FIFO/AXI-stream is supported here)
 *  NPPC         Number of parallel pixels processed per clock cycle
 *
 * ---------------------------------------------------------------------------
 *  Function arguments
 *  ------------------
 *  InImg   [in ] Source image object from which the rectangle is read.
 *  OutImg  [out] Destination image object.  After the call the selected
 *                rectangle is written to this image (all other pixels remain
 *                unchanged).
 *
 *  rowBound       Desired rectangle height  (in rows).             run-time
 *  colBound       Desired rectangle width   (in columns).          run-time
 *  srcStart       Linear start index inside InImg (0 … H_SRC*W_SRC-1).
 *                 Defines the upper-left corner of the ROI within the
 *                 *source*  image.                                 run-time
 *
 *  dstStart       Linear start index inside OutImg (0 … H_DST*W_DST-1).
 *                 Defines where the ROI shall be inserted inside the
 *                 *destination* image.                             run-time
 *
 * ---------------------------------------------------------------------------
 *  Behaviour
 *  ---------
 *  1. All run-time parameters are first clamped so that the requested
 *     copy rectangle fits inside both pictures.
 *  2. The resulting (rowBound × colBound) rectangle is copied pixel-wise
 *     from `InImg` → `OutImg`.
 *  3. Copying is finally delegated to the lower-level helper
 *     `ImgToImgProcess<…>` which performs the actual loop-nest; this
 *     wrapper only takes care of bound checking and address translation.
 *
 ******************************************************************************/
template <
    PixelType PIXEL_T,              // Pixel Type
    int H_SRC,                      // Input Image Height
    int W_SRC,                      // Input Image Width
    int H_DST,                      // Output Image Height
    int W_DST,                      // Output Image Width
    StorageType STORAGE,            // Storage Type
    NumPixelsPerCycle NPPC>         // Pixels Per Cycle
void ImgToImg(
    Img<PIXEL_T, H_SRC, W_SRC, STORAGE, NPPC> &InImg,
    Img<PIXEL_T, H_DST, W_DST, STORAGE, NPPC> &OutImg,
    int rowBound, int colBound,
    int srcStart, int dstStart
) {
    /* ----------------------- Sanity / clipping -------------------- */
    const int SRC_PIXELS =  H_SRC * W_SRC;     // total source pixels
    const int DST_PIXELS =  H_DST * W_DST;     // total destination pixels

    /* Clamp ROI size so it never exceeds the image dimensions        */
    rowBound = std::max(1,
                std::min(rowBound, std::min(H_SRC, H_DST)));
    colBound = std::max(1,
                std::min(colBound, std::min(W_SRC, W_DST)));

    /* Make sure starting indices are within their respective images  */
    srcStart = std::max(0, std::min(srcStart, SRC_PIXELS-1));
    dstStart = std::max(0, std::min(dstStart, DST_PIXELS-1));

    /* Derive (row , col) of the starting points                      */
    const int srcRow0 = srcStart / W_SRC;
    const int srcCol0 = srcStart % W_SRC;
    const int dstRow0 = dstStart / W_DST;
    const int dstCol0 = dstStart % W_DST;

    /* Trim ROI if it would run over the right / bottom edges          */
    rowBound = std::min(rowBound, H_SRC - srcRow0);
    rowBound = std::min(rowBound, H_DST - dstRow0);
    colBound = std::min(colBound, W_SRC - srcCol0);
    colBound = std::min(colBound, W_DST - dstCol0);

    /* Perform image copy */
    ImgToImgProcess
    <   PIXEL_T,
        H_SRC,
        W_SRC,
        H_DST,
        W_DST,
        STORAGE,
        NPPC
    >
    (InImg, OutImg, rowBound, colBound, srcStart, dstStart);
}

/**
 * Iterates over the input image pixel by pixel, and calls the pass-in functor
 * to compute each output pixel based on the corresponding input pixel.
 * The loop iterating a line of pixels is pipelined.  If there are multiple
 * pixels per cycle, all pixels will be processed in parallel.
 */
template <
    PixelType PIXEL_T_I, 
    PixelType PIXEL_T_O, 
    unsigned H, 
    unsigned W,
    StorageType STORAGE_I = FIFO, 
    StorageType STORAGE_O = FIFO,
    NumPixelsPerCycle NPPC = NPPC_1, 
    typename Func>
void TransformPixel(
    vision::Img<PIXEL_T_I, H, W, STORAGE_I, NPPC> &ImgIn,
    vision::Img<PIXEL_T_O, H, W, STORAGE_O, NPPC> &ImgOut,
    Func Functor) {

    const unsigned InPixelWidth = DT<PIXEL_T_I>::W;
    const unsigned OutPixelWidth = DT<PIXEL_T_O>::W;
    const unsigned NumPixelWords = ImgIn.get_height() * ImgIn.get_width() / NPPC;

    HLS_VISION_TRANSFORMPIXEL_LOOP:
    #pragma HLS loop pipeline
    for (unsigned i=0, j=0, k = 0; k < NumPixelWords; k++) {
        typename DT<PIXEL_T_I, NPPC>::T ImgdataIn = ImgIn.read(k);
        typename DT<PIXEL_T_O, NPPC>::T ImgdataOut;
        // There may be multiple pixels per cycle.
        HLS_VISION_TRANSFORMPIXEL_NPPC_LOOP:
        for (unsigned p = 0; p < NPPC; p++) {
            typename DT<PIXEL_T_I>::T InPixel = ImgdataIn.byte(p, InPixelWidth);
            typename DT<PIXEL_T_O>::T OutPixel = Functor(InPixel);
            ImgdataOut.byte(p, OutPixelWidth) = OutPixel;
        }
        ImgOut.write(ImgdataOut, k);
    }
}

/**
Same as Transform() function but it passes the "i" (height index) & "j" 
(width index) of the pixel being transformed. That way the functor can use the 
coordinates to decide what transformation to apply to the input pixel. 
*/
template <
    PixelType PIXEL_T_I, 
    PixelType PIXEL_T_O, 
    unsigned H, 
    unsigned W,
    StorageType STORAGE_I = FIFO, 
    StorageType STORAGE_O = FIFO,
    NumPixelsPerCycle NPPC = NPPC_1, 
    typename Func>
void TransformPixel_ij(
    vision::Img<PIXEL_T_I, H, W, STORAGE_I, NPPC> &ImgIn,
    vision::Img<PIXEL_T_O, H, W, STORAGE_O, NPPC> &ImgOut,
    Func Functor) {

    const unsigned InPixelWidth = DT<PIXEL_T_I>::W;
    const unsigned OutPixelWidth = DT<PIXEL_T_O>::W;
    const unsigned NumPixelWords = ImgIn.get_height() * ImgIn.get_width() / NPPC;

    HLS_VISION_TRANSFORMPIXEL_LOOP:
    #pragma HLS loop pipeline
    for (unsigned i=0, j=0, k = 0; k < NumPixelWords; k++) {
        typename DT<PIXEL_T_I, NPPC>::T ImgdataIn = ImgIn.read(k);
        typename DT<PIXEL_T_O, NPPC>::T ImgdataOut;
        for (unsigned p = 0; p < NPPC; p++, j++) {
            typename DT<PIXEL_T_I>::T InPixel = ImgdataIn.byte(p, InPixelWidth);
            typename DT<PIXEL_T_O>::T OutPixel = Functor(InPixel, i, j);
            ImgdataOut.byte(p, OutPixelWidth) = OutPixel;
        }
        ImgOut.write(ImgdataOut, k);
        if (j == ImgIn.get_width()) {
            j = 0;
            i++;
        }
    }
}

/**
Same as Transform() function but it passes the "enable" argument to control
if the pixel is being transformed or not. The use case is that where a CPU can 
control (enable) the transformation at runtime.
*/
template <
    PixelType PIXEL_T_I, 
    PixelType PIXEL_T_O, 
    unsigned H, 
    unsigned W,
    StorageType STORAGE_I = FIFO, 
    StorageType STORAGE_O = FIFO,
    NumPixelsPerCycle NPPC = NPPC_1, 
    typename Func>
void TransformPixel_enable(
    vision::Img<PIXEL_T_I, H, W, STORAGE_I, NPPC> &ImgIn,
    vision::Img<PIXEL_T_O, H, W, STORAGE_O, NPPC> &ImgOut,
    ap_uint<1> enable,
    Func Functor) {

    const unsigned InPixelWidth = DT<PIXEL_T_I>::W;
    const unsigned OutPixelWidth = DT<PIXEL_T_O>::W;
    const unsigned NumPixelWords = ImgIn.get_height() * ImgIn.get_width() / NPPC;

    HLS_VISION_TRANSFORMPIXEL_LOOP:
    #pragma HLS loop pipeline
    for (unsigned k = 0; k < NumPixelWords; k++) {
        typename DT<PIXEL_T_I, NPPC>::T ImgdataIn = ImgIn.read(k);
        typename DT<PIXEL_T_O, NPPC>::T ImgdataOut;
        for (unsigned p = 0; p < NPPC; p++) {
            typename DT<PIXEL_T_I>::T InPixel = ImgdataIn.byte(p, InPixelWidth);
            typename DT<PIXEL_T_O>::T OutPixel = Functor(InPixel);
            ImgdataOut.byte(p, OutPixelWidth) = enable ? OutPixel : InPixel;
        }
        ImgOut.write(ImgdataOut, k);
    }
}

/**
Transform() function but it passes an argument by reference.
*/
template <
    typename ARG_T,
    PixelType PIXEL_T_I, 
    PixelType PIXEL_T_O, 
    unsigned H, 
    unsigned W,
    StorageType STORAGE_I = FIFO, 
    StorageType STORAGE_O = FIFO,
    NumPixelsPerCycle NPPC = NPPC_1, 
    typename Func>
void TransformPixel_ref(
    vision::Img<PIXEL_T_I, H, W, STORAGE_I, NPPC> &ImgIn,
    vision::Img<PIXEL_T_O, H, W, STORAGE_O, NPPC> &ImgOut,
    ARG_T &arg,
    Func Functor) {

    const unsigned InPixelWidth = DT<PIXEL_T_I>::W;
    const unsigned OutPixelWidth = DT<PIXEL_T_O>::W;
    const unsigned NumPixelWords = ImgIn.get_height() * ImgIn.get_width() / NPPC;

    HLS_VISION_TRANSFORMPIXEL_LOOP:
    #pragma HLS loop pipeline
    for (unsigned k = 0; k < NumPixelWords; k++) {
        typename DT<PIXEL_T_I, NPPC>::T ImgdataIn = ImgIn.read(k);
        typename DT<PIXEL_T_O, NPPC>::T ImgdataOut;
        for (unsigned p = 0; p < NPPC; p++) {
            typename DT<PIXEL_T_I>::T InPixel = ImgdataIn.byte(p, InPixelWidth);
            typename DT<PIXEL_T_O>::T OutPixel = Functor(InPixel, arg);
            ImgdataOut.byte(p, OutPixelWidth) = OutPixel;
        }
        ImgOut.write(ImgdataOut, k);
    }
}


// Create an image to plot the histogram
template <int HIST_SIZE, int W, int H>
void plotHistogram(cv::Mat &histMat, std::string filename) {
    cv::normalize(histMat, histMat, 0, H - 15, cv::NORM_MINMAX);

    cv::Mat histImage(H, W, CV_8UC3, cv::Scalar(255, 255, 255));

    // Plot histogram
    int binWidth = cvRound((double)W / HIST_SIZE);

    // printf("-------[ %s ]-------\n", filename.c_str());
        // printf("%d:%d\n", i, h);
    for (int i = 0; i < HIST_SIZE; ++i) {
        unsigned int h = static_cast<unsigned int>(histMat.at<unsigned int>(i));
        cv::line(
            histImage,
            cv::Point(binWidth * i, H - 15),
            cv::Point(binWidth * i, H - h - 15),
            cv::Scalar(0, 0, 0), 2, 8, 0
        );
    }
    cv::putText(
        histImage, 
        "Pixel Intensity", 
        cv::Point(W/2 - 64, H - 4),
        cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(0, 0, 0), 1
    );
    
    cv::imwrite(filename, histImage);
}

} // End of namespace vision.
} // End of namespace hls.
