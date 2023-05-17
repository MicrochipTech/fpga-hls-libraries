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
// MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE
// FORESEEABLE.  TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP’S TOTAL
// LIABILITY ON ALL CLAIMS LATED TO THE SOFTWARE WILL NOT EXCEED AMOUNT OF
// FEES, IF ANY, YOU PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE. MICROCHIP
// OFFERS NO SUPPORT FOR THE SOFTWARE. YOU MAY CONTACT MICROCHIP AT
// https://www.microchip.com/en-us/support-and-training/design-help/client-support-services
// TO INQUIRE ABOUT SUPPORT SERVICES AND APPLICABLE FEES, IF AVAILABLE.

#ifndef __SHLS_VISION_AXI_MM_INTF_HPP__
#define __SHLS_VISION_AXI_MM_INTF_HPP__

#include "../common/common.hpp"
#include "../common/utils.hpp"
#include "./axis_video.hpp"
#include <hls/ap_fixpt.hpp>
#include <hls/ap_int.hpp>

namespace hls {
namespace vision {

// This file provides the following conversion functions:
// 1. AXI Memory Map <-> Img conversion:
// Convert between an AXI memory map (AxiMM) of power-of-2-byte width and an
// image of arbitrary pixel type.
// Example usage:
//   uint64_t InAxiMM[NumAxiWords];
//   Img<...> OutImg;
//   AxiMM2Img<64>(InAxiMM, OutImg)
template <unsigned AxiWordWidth, typename AxiDT, vision::PixelType PIXEL_T,
          unsigned H, unsigned W, vision::StorageType STORAGE,
          vision::NumPixelsPerCycle NPPC>
void AxiMM2Img(AxiDT *InAxiMM, Img<PIXEL_T, H, W, STORAGE, NPPC> &OutImg);

template <unsigned AxiWordWidth, typename AxiDT, vision::PixelType PIXEL_T,
          unsigned H, unsigned W, vision::StorageType STORAGE,
          vision::NumPixelsPerCycle NPPC>
void Img2AxiMM(Img<PIXEL_T, H, W, STORAGE, NPPC> &InImg, AxiDT *OutAxiMM);

// 2. Axi Memory Map <-> AxisVideo conversion:
// Convert between an AXI memory map (AxiMM) of power-of-2-byte width and an AXI
// stream video protocol FIFO type (see `axis_video.hpp`).
// Example usage:
//   uint64_t InAxiMM[NumAxiWords];
//   vision::AxisVideoFIFO<...> OutVideo;
// The HRes and VRes of `OutVideo`, which might be different (smaller or equal
// to) the maximum frame size `W` and `H`.
//   unsigned HRes = ..., VRes = ...;
//   AxiMM2AxisVideo<64, uint64_t, H, W>(InAxiMM, OutVideo, HRes, VRes);
template <unsigned AxiWordWidth, typename AxiDT, unsigned H, unsigned W,
          vision::PixelType PIXEL_T, vision::NumPixelsPerCycle NPPC>
void AxiMM2AxisVideo(AxiDT *InAxiMM, AxisVideoFIFO<PIXEL_T, NPPC> &OutVideo,
                     unsigned HRes, unsigned VRes);

template <unsigned AxiWordWidth, typename AxiDT, unsigned H, unsigned W,
          vision::PixelType PIXEL_T, vision::NumPixelsPerCycle NPPC>
void AxisVideo2AxiMM(AxisVideoFIFO<PIXEL_T, NPPC> &InVideo, AxiDT *OutAxiMM,
                     unsigned HRes, unsigned VRes);

/*****************************************************************************/
/******************************* AxiMM <-> Img *******************************/
/*****************************************************************************/
// This section provides the helper functions to convert between an AXI memory
// map (AxiMM) of power-of-2-byte width and an image of arbitrary pixel type.
// Both directions of the conversion are supported:
// - InAxiMM -> OutImg:
//     AxiMM2Img(&InAxiMM<AxiWordWidth>, &OutImg<PixelWordWidth>)
// - InImg -> OutAxiMM:
//     Img2AxiMM(&InImg<PixelWordWidth>, &OutAxiMM<AxiWordWidth>)
//
// TODO T: Once our AXI interface can support ap_int memory, we need to remove
// all reference of `AxiDT` and change the type for the AxiMM argument to
// `ap_uint<AxiWordWidth>` in this file.
//
// Some definitions used in this file:
// - Pixel word: a group of NPPC pixels. For example, for NPPC = 4, a pixel word
//   is a group of 4 pixels.
// - AXI word: a word in AXI Memory.
// - PixelWordWidth: Number of bits in a pixel word.
//   E.g, for 8UC3 with NPPC = 4:
//     PixelWordWidth = 8 (bits per channel) * 3 (channels) * 4 (NPPC) = 96.
// - AxiWordWidth: Number of bits in an AXI word. Has to be a power-of-2 bytes,
//   e.g., 32.
// - NumPixelWords: Number of pixel words in the Img, where each pixel word is
//   NPPC pixels:
//     NumPixels = Img.height * Img.width
//     NumPixelWords = NumPixels / NPPC.
// - NumAxiWords: How many words in the AXI memory to hold the Img.
// Basically: NumPixelWords * PixelWordWidth = NumAxiWords * AxiWordWidth.
//
//
// Example of a RGB2GRAY() dataflow pipeline that uses the width conversion
// helper functions. Assumptions:
// - Input image is 8UC3, NPPC = 1 (24b per pixel):
//     InImg.PixelWordWidth = 24
// - Output image is 8UC1, NPPC = 1 (8b per pixel):
//     OutImg.PixelWordWidth = 8
// - InAxiMM and OutAxiMM are 32-bits:
//     InAxiMM.AxiWordWidth = OutAxiMM.AxiWordWidth = 32.
//
//           +--------------------------------------------------------------------------+
//           |                              RGB2GRAYTop()                               |
//           |                                                                          |
//           |  +-------------+             +------------+              +-------------+ |
//           |  |             |             |            |              |             | |
// InAxiMM ---->| AxiMM2Img() |--> InImg -->| RGB2GRAY() |--> OutImg -->| Img2AxiMM() |----> OutAxiMM
//   <32>    |  |             |    <24>     |            |     <8>      |             | |      <32>
//           |  +-------------+             +------------+              +-------------+ |
//           +--------------------------------------------------------------------------+


/******************************** AxiMM2Img() ********************************/
// The AxiMM2Img() pipeline is shown using the following example.
// Assumption of the example:
// - InAxiMM is 32-bit:        InAxiMM.AxiWordWidth  = 32
// - OutImg is 8UC3, NPPC = 1: OutImg.PixelWordWidth = 24
//           +------------------------------------------------------------+
//           |                        AxiMM2Img()                         |
//           |                                                            |
//           |  +--------------+               +------------------------+ |
//           |  |              |               |                        | |
// InAxiMM ---->| AxiMM2FIFO() |--> TmpFIFO -->| widthConvertFIFO2Img() |----> OutImg
//   <32>    |  |              |     <32>      |                        | |     <24>
//           |  +--------------+               +------------------------+ |
//           +------------------------------------------------------------+
// - AxiMM2FIFO() simply takes data from InAxiMM and write to TmpFIFO as-is.
// - widthConvertFIFO2Img() is when the width conversion happens.

/**
 * Helper functions: AxiMM2FIFO()
 */
template <unsigned AxiWordWidth, typename AxiDT>
void AxiMM2FIFO(AxiDT *InAxiMM, hls::FIFO<ap_uint<AxiWordWidth>> &OutFIFO,
                unsigned NumAxiWords) {
#pragma HLS loop pipeline
    for (unsigned AxiWordIdx = 0; AxiWordIdx < NumAxiWords; AxiWordIdx++) {
        OutFIFO.write(InAxiMM[AxiWordIdx]);
    }
}

/**
 * Helper function: widthConvertFIFO2Img():
 */
// Idea: Have a shift register that takes in data from InFIFO, and output data
//   to OutImg.
// In each iteration, depending on whether the current amount of data in the
//   shift register (let's call this `LeftOverBits`) is enough to output a
//   pixel, we will be able to either read, or write, or both.
// Example: If AxiWordWidth = 32, PixelWordWidth = 24 (for 8UC3, NPPC = 1).
//   Since AxiWordWidth > PixelWordWidth, in each iteration, we are not
//   guaranteed to be able to read an input AXI word, but we are guaranteed to
//   be able to write an output pixel word.
// Let's go through some iterations. Note that each read adds 32b to
//   `LeftOverBits`, and each write substracts 24b from `LeftOverBits`.
//   Initially, LeftOverBits = 0.
// +------+------------------------+------------------------+
// | Iter |   Read from InFIFO?    |    Write to OutImg?    |
// |      |  (LeftOverBits += 32)  |  (LeftOverBits -= 24)  |
// +------+------------------------+------------------------+
// |  0   | Yes. LeftOverBits = 32 | Yes. LeftOverBits = 8  |
// |  1   | Yes. LeftOverBits = 40 | Yes. LeftOverBits = 16 |
// |  2   | Yes. LeftOverBits = 48 | Yes. LeftOverBits = 24 |
// |  3   | No . LeftOverBits = 24 | Yes. LeftOverBits = 0  |
// |                      <Repeat...>                       |
// +------+------------------------+------------------------+
// Observe that after iter 2, LeftOverBits = 24 which is enough to output a
// pixel word. So in iter 3, we don't need to read an input AXI word.
template <unsigned AxiWordWidth, vision::PixelType PIXEL_T, unsigned H,
          unsigned W, vision::StorageType STORAGE,
          vision::NumPixelsPerCycle NPPC>
void widthConvertFIFO2Img(hls::FIFO<ap_uint<AxiWordWidth>> &InFIFO,
                          Img<PIXEL_T, H, W, STORAGE, NPPC> &OutImg,
                          unsigned NumIters) {
    const unsigned PixelWordWidth = DT<PIXEL_T, NPPC>::W;

    ap_uint<AxiWordWidth + PixelWordWidth> LeftOverData;
    unsigned LeftOverBits = 0;
    unsigned ImgIdx = 0;

    // Loop to read from `InFIFO` and write to `OutImg`.
#pragma HLS loop pipeline
    for (unsigned i = 0; i < NumIters; i++) {
        // Read from InFIFO if we don't have enough data to output a pixel.
        if (LeftOverBits < PixelWordWidth) {
            ap_uint<AxiWordWidth> InData = InFIFO.read();
            LeftOverData(LeftOverBits + AxiWordWidth - 1, LeftOverBits) =
                InData;
            LeftOverBits += AxiWordWidth;
        }
        // Write to OutImg if we have enough data to output a pixel.
        if (LeftOverBits >= PixelWordWidth) {
            ap_uint<PixelWordWidth> OutData = LeftOverData(PixelWordWidth - 1, 0);
            LeftOverData >>= PixelWordWidth;
            LeftOverBits -= PixelWordWidth;
            OutImg.write(OutData, ImgIdx);
            ImgIdx++;
        }
    }
}

/**
 * Main function: AxiMM2Img()
 */
template <unsigned AxiWordWidth, typename AxiDT, vision::PixelType PIXEL_T,
          unsigned H, unsigned W, vision::StorageType STORAGE,
          vision::NumPixelsPerCycle NPPC>
void AxiMM2Img(AxiDT *InAxiMM, Img<PIXEL_T, H, W, STORAGE, NPPC> &OutImg) {
#pragma HLS function dataflow
    static_assert(AxiWordWidth == sizeof(AxiDT) * 8,
                  "AxiMM2Img(): The data type of the Axi memory map has to be "
                  "the same as 'AxiWordWidth'.");
    // Assert `AxiWordWidth` is a power-of-2 bytes, i.e.,
    //   (AxiWordWidth / 8) is a power-of-2.
    // Use the following expression to check if a value is a power-of-2:
    //   (val & (val - 1)) == 0
    static_assert(AxiWordWidth % 8 == 0 &&
                      ((AxiWordWidth / 8) & (AxiWordWidth / 8 - 1)) == 0,
                  "AxiMM2Img(): 'AxiWordWidth' has to be a power-of-2 byte.");
    // TODO T: Ideally should use `get_height()` & `get_width()` for the check
    // below, but they aren't constants and can't be used in a `static_assert()`.
    static_assert(H * W * DT<PIXEL_T, NPPC>::W / NPPC % AxiWordWidth == 0,
                  "AxiMM2Img(): The image size (in bits) has to be divisible "
                  "by 'AxiWordWidth'.");

    const unsigned PixelWordWidth = DT<PIXEL_T, NPPC>::W;
    // TODO T: Fix the line below not working.
    // const unsigned NumPixelWords = OutImg.get_height() * OutImg.get_width() / NPPC;
    const unsigned NumPixelWords = H * W / NPPC;
    const unsigned NumAxiWords = NumPixelWords * PixelWordWidth / AxiWordWidth;

    hls::FIFO<ap_uint<AxiWordWidth>> TmpFIFO;
#ifndef __SYNTHESIS__
    TmpFIFO.setDepth(NumAxiWords);
#else
    // The FIFO needs to be able to hold enough data for at least 1 pixel word.
    // TODO T: Only + 1 if it's not perfectly divisible?
    const unsigned NumAxiWordsPerPixelWord = PixelWordWidth / AxiWordWidth + 1;
    // The SmartHLS User Guide recommends a minimum FIFO depth of 2 to avoid
    // excessive stalls.
    if (NumAxiWordsPerPixelWord > 2)
        TmpFIFO.setDepth(NumAxiWordsPerPixelWord);
#endif
    const unsigned NumIters = NumAxiWords > NumPixelWords ? NumAxiWords : NumPixelWords;
    AxiMM2FIFO(InAxiMM, TmpFIFO, NumAxiWords);
    widthConvertFIFO2Img(TmpFIFO, OutImg, NumIters);
}

/******************************** Img2AxiMM() ********************************/
// The Img2AxiMM() pipeline is shown using the following example.
// Assumption of the example:
// - InImg is 8UC3, NPPC = 1: InImg.PixelWordWidth  = 24
// - OutAxiMM is 32-bit:      OutAxiMM.AxiWordWidth = 32
//         +------------------------------------------------------------+
//         |                        Img2AxiMM()                         |
//         |                                                            |
//         |  +------------------------+               +--------------+ |
//         |  |                        |               |              | |
// InImg ---->| widthConvertImg2FIFO() |--> TmpFIFO -->| FIFO2AxiMM() |----> OutAxiMM
// <24>    |  |                        |     <32>      |              | |      <32>
//         |  +------------------------+               +--------------+ |
//         +------------------------------------------------------------+
// - widthConvertImg2FIFO() is when the width conversion happens.
// - FIFO2AxiMM() simply takes data from TmpFIFO and write to OutAxiMM as-is.

/**
 * Helper function: widthConvertImg2FIFO():
 */
// Idea: Have a shift register that takes in data from InImg, and output data
//   to OutFIFO.
// In each iteration, depending on whether the current amount of data in the
//   shift register (let's call this `LeftOverBits`) is enough to output a
//   word, we will be able to either read, or write, or both.
// Example: If PixelWordWidth = 24 (for 8UC3, NPPC = 1), AxiWordWidth = 32.
//   Since PixelWordWidth < AxiWordWidth, in each iteration, we are guaranteed
//   to be able to read an input pixel word, but we are not guaranteed to be
//   able to write an output AXI word.
// Let's go through some iterations. Note that each read adds 24b to
//   `LeftOverBits`, and each write substracts 32b from `LeftOverBits`.
//   Initially, LeftOverBits = 0.
// +------+------------------------+------------------------+
// | Iter |    Read from InImg?    |   Write to OutFIFO?    |
// |      |  (LeftOverBits += 24)  |  (LeftOverBits -= 32)  |
// +------+------------------------+------------------------+
// |  0   | Yes. LeftOverBits = 24 | No . LeftOverBits = 24 |
// |  1   | Yes. LeftOverBits = 48 | Yes. LeftOverBits = 16 |
// |  2   | Yes. LeftOverBits = 40 | Yes. LeftOverBits = 8  |
// |  3   | Yes. LeftOverBits = 32 | Yes. LeftOverBits = 0  |
// |                      <Repeat...>                       |
// +------+------------------------+------------------------+
// Observe that in iter 0, after the read, LeftOverBits = 24 which is not enough
// to output an AXI word. So in iter 0, we are not able to write an output AXI
// word.
template <unsigned AxiWordWidth, vision::PixelType PIXEL_T, unsigned H,
          unsigned W, vision::StorageType STORAGE,
          vision::NumPixelsPerCycle NPPC>
void widthConvertImg2FIFO(Img<PIXEL_T, H, W, STORAGE, NPPC> &InImg,
                          hls::FIFO<ap_uint<AxiWordWidth>> &OutFIFO,
                          unsigned NumIters) {
    const unsigned PixelWordWidth = DT<PIXEL_T, NPPC>::W;

    ap_uint<PixelWordWidth + AxiWordWidth> LeftOverData;
    unsigned LeftOverBits = 0;
    unsigned ImgIdx = 0;

    // Loop to read from `InImg` and write to `OutFIFO`.
#pragma HLS loop pipeline
    for (unsigned i = 0; i < NumIters; i++) {
        // Read from InImg if we don't have enough data to output a word.
        if (LeftOverBits < AxiWordWidth) {
            ap_uint<PixelWordWidth> InData = InImg.read(ImgIdx);
            LeftOverData(LeftOverBits + PixelWordWidth - 1, LeftOverBits) =
                InData;
            LeftOverBits += PixelWordWidth;
            ImgIdx++;
        }
        // Write to OutFIFO if we have enough data to output a word.
        if (LeftOverBits >= AxiWordWidth) {
            ap_uint<AxiWordWidth> OutData =
                LeftOverData(AxiWordWidth - 1, 0);
            LeftOverData >>= AxiWordWidth;
            LeftOverBits -= AxiWordWidth;
            OutFIFO.write(OutData);
        }
    }
}

/**
 * Helper function: FIFO2AxiMM()
 */
template <unsigned AxiWordWidth, typename AxiDT>
void FIFO2AxiMM(hls::FIFO<ap_uint<AxiWordWidth>> &InFIFO, AxiDT *OutAxiMM,
                unsigned NumAxiWords) {
#pragma HLS loop pipeline
    for (unsigned AxiWordIdx = 0; AxiWordIdx < NumAxiWords; AxiWordIdx++) {
        OutAxiMM[AxiWordIdx] = InFIFO.read();
    }
}

/**
 * Main function: Img2AxiMM()
 */
template <unsigned AxiWordWidth, typename AxiDT, vision::PixelType PIXEL_T,
          unsigned H, unsigned W, vision::StorageType STORAGE,
          vision::NumPixelsPerCycle NPPC>
void Img2AxiMM(Img<PIXEL_T, H, W, STORAGE, NPPC> &InImg, AxiDT *OutAxiMM) {
#pragma HLS function dataflow
    static_assert(AxiWordWidth == sizeof(AxiDT) * 8,
                  "Img2AxiMM(): The data type of the Axi memory map has to be "
                  "the same as 'AxiWordWidth'.");
    // Assert `AxiWordWidth` is a power-of-2 bytes, i.e.,
    //   (AxiWordWidth / 8) is a power-of-2.
    // Use the following expression to check if a value is a power-of-2:
    //   (val & (val - 1)) == 0
    static_assert(AxiWordWidth % 8 == 0 &&
                      ((AxiWordWidth / 8) & (AxiWordWidth / 8 - 1)) == 0,
                  "Img2AxiMM(): 'AxiWordWidth' has to be a power-of-2 byte.");
    // TODO T: Ideally should use `get_height()` & `get_width()` for the check
    // below, but they aren't constants and can't be used in a `static_assert()`.
    static_assert(H * W * DT<PIXEL_T, NPPC>::W / NPPC % AxiWordWidth == 0,
                  "Img2AxiMM(): The image size (in bits) has to be divisible "
                  "by 'AxiWordWidth'.");

    const unsigned PixelWordWidth = DT<PIXEL_T, NPPC>::W;
    // TODO T: Fix the line below not working.
    // const unsigned NumPixelWords = InImg.get_height() * InImg.get_width() / NPPC;
    const unsigned NumPixelWords = H * W / NPPC;
    const unsigned NumAxiWords = NumPixelWords * PixelWordWidth / AxiWordWidth;

    hls::FIFO<ap_uint<AxiWordWidth>> TmpFIFO;
#ifndef __SYNTHESIS__
    TmpFIFO.setDepth(NumAxiWords);
#else
    // The FIFO needs to be able to hold enough data for at least 1 pixel word.
    // TODO T: Only + 1 if it's not perfectly divisible?
    const unsigned NumAxiWordsPerPixelWord = PixelWordWidth / AxiWordWidth + 1;
    // The SmartHLS User Guide recommends a minimum FIFO depth of 2 to avoid
    // excessive stalls.
    if (NumAxiWordsPerPixelWord > 2)
        TmpFIFO.setDepth(NumAxiWordsPerPixelWord);
#endif
    const unsigned NumIters = NumPixelWords > NumAxiWords ? NumPixelWords : NumAxiWords;
    widthConvertImg2FIFO(InImg, TmpFIFO, NumIters);
    FIFO2AxiMM(TmpFIFO, OutAxiMM, NumAxiWords);
}

/*****************************************************************************/
/**************************** AxiMM <-> AxisVideo ****************************/
/*****************************************************************************/
// This section provides the helper functions to convert between an AXI memory
// map (AxiMM) of power-of-2-byte width and an AXI stream video protocol FIFO
// type (see `axis_video.hpp`)

template <unsigned AxiWordWidth, typename AxiDT, unsigned H, unsigned W,
          vision::PixelType PIXEL_T, vision::NumPixelsPerCycle NPPC>
void AxiMM2AxisVideo(AxiDT *InAxiMM, AxisVideoFIFO<PIXEL_T, NPPC> &OutVideo,
                     int HRes, int VRes) {
#pragma HLS function dataflow
    static_assert(AxiWordWidth == sizeof(AxiDT) * 8,
                  "AxiMM2AxisVideo(): The data type of the Axi memory map has "
                  "to be the same as 'AxiWordWidth'.");
    // Assert `AxiWordWidth` is a power-of-2 bytes, i.e.,
    //   (AxiWordWidth / 8) is a power-of-2.
    // Use the following expression to check if a value is a power-of-2:
    //   (val & (val - 1)) == 0
    static_assert(
        AxiWordWidth % 8 == 0 &&
            ((AxiWordWidth / 8) & (AxiWordWidth / 8 - 1)) == 0,
        "AxiMM2AxisVideo(): 'AxiWordWidth' has to be a power-of-2 byte.");
    // TODO T: Ideally should use `HRes` & `VRes` for the check below, but they
    // aren't constants and can't be used in a `static_assert()`.
    static_assert(H * W * DT<PIXEL_T, NPPC>::W / NPPC % AxiWordWidth == 0,
                  "AxiMM2AxisVideo(): The image size (in bits) has to be "
                  "divisible by 'AxiWordWidth'.");
    Img<PIXEL_T, H, W, vision::StorageType::FIFO, NPPC> TmpImg(VRes, HRes);

    // 1. AxiMM2Img()
    // TODO T: Have to inline because of a current limitation.
    // AxiMM2Img<AxiWordWidth>(InAxiMM, TmpImg);
    const unsigned PixelWordWidth = DT<PIXEL_T, NPPC>::W;
    const unsigned NumPixelWords = H * W / NPPC;
    const unsigned NumAxiWords = NumPixelWords * PixelWordWidth / AxiWordWidth;
    hls::FIFO<ap_uint<AxiWordWidth>> TmpFIFO;
#ifndef __SYNTHESIS__
    TmpFIFO.setDepth(NumAxiWords);
#else
    const unsigned NumAxiWordsPerPixelWord = PixelWordWidth / AxiWordWidth + 1;
    if (NumAxiWordsPerPixelWord > 2)
        TmpFIFO.setDepth(NumAxiWordsPerPixelWord);
#endif
    const unsigned NumIters = NumAxiWords > NumPixelWords ? NumAxiWords : NumPixelWords;
    AxiMM2FIFO(InAxiMM, TmpFIFO, NumAxiWords);
    widthConvertFIFO2Img(TmpFIFO, TmpImg, NumIters);

    // 2. Img2AxisVideo()
    Img2AxisVideo(TmpImg, OutVideo);
}

template <unsigned AxiWordWidth, typename AxiDT, unsigned H, unsigned W,
          vision::PixelType PIXEL_T, vision::NumPixelsPerCycle NPPC>
void AxisVideo2AxiMM(AxisVideoFIFO<PIXEL_T, NPPC> &InVideo, AxiDT *OutAxiMM,
                     int HRes, int VRes) {
#pragma HLS function dataflow
    static_assert(AxiWordWidth == sizeof(AxiDT) * 8,
                  "AxisVideo2AxiMM(): The data type of the Axi memory map has "
                  "to be the same as 'AxiWordWidth'.");
    // Assert `AxiWordWidth` is a power-of-2 bytes, i.e.,
    //   (AxiWordWidth / 8) is a power-of-2.
    // Use the following expression to check if a value is a power-of-2:
    //   (val & (val - 1)) == 0
    static_assert(
        AxiWordWidth % 8 == 0 &&
            ((AxiWordWidth / 8) & (AxiWordWidth / 8 - 1)) == 0,
        "AxisVideo2AxiMM(): 'AxiWordWidth' has to be a power-of-2 byte.");
    // TODO T: Ideally should use `HRes` & `VRes` for the check below, but they
    // aren't constants and can't be used in a `static_assert()`.
    static_assert(H * W * DT<PIXEL_T, NPPC>::W / NPPC % AxiWordWidth == 0,
                  "AxisVideo2AxiMM(): The image size (in bits) has to be "
                  "divisible by 'AxiWordWidth'.");
    Img<PIXEL_T, H, W, vision::StorageType::FIFO, NPPC> TmpImg(VRes, HRes);

    // 1. AxisVideo2Img()
    AxisVideo2Img(InVideo, TmpImg);

    // 2. Img2AxiMM()
    // TODO T: Have to inline because of a current limitation.
    // Img2AxiMM<AxiWordWidth>(TmpImg, OutAxiMM);
    const unsigned PixelWordWidth = DT<PIXEL_T, NPPC>::W;
    const unsigned NumPixelWords = H * W / NPPC;
    const unsigned NumAxiWords = NumPixelWords * PixelWordWidth / AxiWordWidth;
    hls::FIFO<ap_uint<AxiWordWidth>> TmpFIFO;
#ifndef __SYNTHESIS__
    TmpFIFO.setDepth(NumAxiWords);
#else
    const unsigned NumAxiWordsPerPixelWord = PixelWordWidth / AxiWordWidth + 1;
    if (NumAxiWordsPerPixelWord > 2)
        TmpFIFO.setDepth(NumAxiWordsPerPixelWord);
#endif
    const unsigned NumIters = NumPixelWords > NumAxiWords ? NumPixelWords : NumAxiWords;
    widthConvertImg2FIFO(TmpImg, TmpFIFO, NumIters);
    FIFO2AxiMM(TmpFIFO, OutAxiMM, NumAxiWords);
}

} // end of namespace vision
} // end of namespace hls
#endif