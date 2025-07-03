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

#include "hls/ap_fixpt.hpp"
#include "hls/ap_int.hpp"

namespace hls {
namespace vision {

/******************************************************************************
* @brief Compile-time lookup table that maps every destination pixel to the
*        four source-image neighbours required for bilinear interpolation.
*
*        The table is generated in the constexpr constructor, therefore it
*        occupies **no run-time cycles** and is baked into the ELF/bit-stream.
*
* ──────────────────────────────────────────────────────────────────────────
* Template parameters
* ──────────────────────────────────────────────────────────────────────────
* SIZE     : Number of pixels in the destination image (W_DST × H_DST).
* W_SRC    : Width  of the source image.
* H_SRC    : Height of the source image.
* W_DST    : Width  of the destination image.
* H_DST    : Height of the destination image.
*
* ──────────────────────────────────────────────────────────────────────────
* Public data members (all fixed-size @ compile time)
* ──────────────────────────────────────────────────────────────────────────
* TopLeftTable     [SIZE] – linear index of ⌈src_x⌉,⌈src_y⌉
* TopRightTable    [SIZE] – linear index of ⌈src_x⌉+1,⌈src_y⌉
* BottomLeftTable  [SIZE] – linear index of ⌈src_x⌉,⌈src_y⌉+1
* BottomRightTable [SIZE] – linear index of ⌈src_x⌉+1,⌈src_y⌉+1
* xDiffTable       [SIZE] – fractional distance in X (src_x – x1)   ∈[0,1]
* yDiffTable       [SIZE] – fractional distance in Y (src_y – y1)   ∈[0,1]
*
* The linear index returned by @p computeIndex is
*      index = y * W_SRC + x.
*
* ──────────────────────────────────────────────────────────────────────────
* Public member functions
* ──────────────────────────────────────────────────────────────────────────
* getTopLeftPixel(i)     – constexpr accessor for TopLeftTable[i]
* getTopRightPixel(i)    – constexpr accessor for TopRightTable[i]
* getBottomLeftPixel(i)  – constexpr accessor for BottomLeftTable[i]
* getBottomRightPixel(i) – constexpr accessor for BottomRightTable[i]
* getXDiff(i)            – fractional x-offset for pixel i
* getYDiff(i)            – fractional y-offset for pixel i
*
* print{Top|Bottom}{Left|Right}Pixels() – helper debug dump (run-time only)
*
* ──────────────────────────────────────────────────────────────────────────
* Usage example
* ──────────────────────────────────────────────────────────────────────────
* using Tbl = NeighborPixelTable<
*                640*480,  // SIZE
*                640,480,  // source
*                320,240   // destination
*            >;
*
* static constexpr Tbl tbl;          // created entirely at compile time
* uint8_t  g11 = src[tbl.getTopLeftPixel(p)    ];
* uint8_t  g12 = src[tbl.getTopRightPixel(p)   ];
* uint8_t  g21 = src[tbl.getBottomLeftPixel(p) ];
* uint8_t  g22 = src[tbl.getBottomRightPixel(p)];
* float    fx  = tbl.getXDiff(p);
* float    fy  = tbl.getYDiff(p);
*
******************************************************************************/
template <
    int SIZE,   /// Destination-image pixel count (W_DST × H_DST)
    int W_SRC,  /// Source-image width  (pixels)
    int H_SRC,  /// Source-image height (pixels)
    int W_DST,  /// Destination-image width  (pixels)
    int H_DST   /// Destination-image height (pixels)
>
struct NeighborPixelTable
{
    /* ------------------------------------------------------------------ */
    /*  Pre-computed lookup tables                                        */
    /* ------------------------------------------------------------------ */
    int   TopLeftTable    [SIZE];   /// index of (x1 , y1)
    int   TopRightTable   [SIZE];   /// index of (x2 , y1)
    int   BottomLeftTable [SIZE];   /// index of (x1 , y2)
    int   BottomRightTable[SIZE];   /// index of (x2 , y2)
    float xDiffTable      [SIZE];   /// src_x − x1  (0…1)
    float yDiffTable      [SIZE];   /// src_y − y1  (0…1)

    /* ------------------------------------------------------------------ */
    /*  Compile-time constants                                            */
    /* ------------------------------------------------------------------ */
    static constexpr int   WIDTH_SRC  = W_SRC;
    static constexpr int   HEIGHT_SRC = H_SRC;
    static constexpr int   WIDTH_DST  = W_DST;
    static constexpr int   HEIGHT_DST = H_DST;
    static constexpr float x_ratio    = float(W_SRC) / float(W_DST);
    static constexpr float y_ratio    = float(H_SRC) / float(H_DST);

    /// Convert 2-D (x,y) to linear index in the *source* image
    static constexpr int computeIndex(int x, int y, int width) {
        return y * width + x;
    }

    /* ------------------------------------------------------------------ */
    /*  constexpr constructor – executed entirely at compile time         */
    /* ------------------------------------------------------------------ */
    constexpr NeighborPixelTable() :
        TopLeftTable    {},
        TopRightTable   {},
        BottomLeftTable {},
        BottomRightTable{},
        xDiffTable      {},
        yDiffTable      {}
    {
        for (int i = 0; i < SIZE; ++i)
        {
            /* -------------------------------- coordinates in dst image */
            const int y = i / W_DST;          // integer division
            const int x = i - y * W_DST;      // avoid % for constexpr

            /* -------------- exact floating-point position in src image */
            const float src_x = x * x_ratio;
            const float src_y = y * y_ratio;

            /* -------- clamp & convert to top-left integer coordinate */
            const int x1 = src_x < 0               ? 0           :
                           src_x >= W_SRC - 1      ? W_SRC - 1   :
                                                   int(src_x);
            const int y1 = src_y < 0               ? 0           :
                           src_y >= H_SRC - 1      ? H_SRC - 1   :
                                                   int(src_y);

            /* -------- neighbouring pixel (with edge-clamp)          */
            const int x2 = (x1 + 1 < W_SRC) ? x1 + 1 : W_SRC - 1;
            const int y2 = (y1 + 1 < H_SRC) ? y1 + 1 : H_SRC - 1;

            /* -------- store linear indices                           */
            TopLeftTable    [i] = computeIndex(x1, y1, W_SRC);
            TopRightTable   [i] = computeIndex(x2, y1, W_SRC);
            BottomLeftTable [i] = computeIndex(x1, y2, W_SRC);
            BottomRightTable[i] = computeIndex(x2, y2, W_SRC);

            /* -------- fractional parts                               */
            xDiffTable[i] = src_x - x1;
            yDiffTable[i] = src_y - y1;
        }
    }

    /* ------------------------------------------------------------------ */
    /*  constexpr accessors (usable inside further constexpr code)        */
    /* ------------------------------------------------------------------ */
    constexpr int   getTopLeftPixel   (int i) const { return TopLeftTable[i];    }
    constexpr int   getTopRightPixel  (int i) const { return TopRightTable[i];   }
    constexpr int   getBottomLeftPixel(int i) const { return BottomLeftTable[i]; }
    constexpr int   getBottomRightPixel(int i)const { return BottomRightTable[i];}
    constexpr float getXDiff          (int i) const { return xDiffTable[i];      }
    constexpr float getYDiff          (int i) const { return yDiffTable[i];      }

    /* Debug helpers (run-time only) */
    void printTopLeftPixels() const    { dump("Top-Left",    TopLeftTable   ); }
    void printTopRightPixels() const   { dump("Top-Right",   TopRightTable  ); }
    void printBottomLeftPixels() const { dump("Bottom-Left", BottomLeftTable); }
    void printBottomRightPixels()const { dump("Bottom-Right",BottomRightTable); }

private:
    /* small helper to avoid duplicate code in dumps */
    void dump(const char* lbl,const int* tbl)const{
        printf("\n---- %s neighbours ----\n",lbl);
        for(int i=0;i<SIZE;++i) printf("[%d] = %d\n",i,tbl[i]);
    }
};

}
}
