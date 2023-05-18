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
// MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.
// TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP’S TOTAL LIABILITY ON ALL
// CLAIMS LATED TO THE SOFTWARE WILL NOT EXCEED AMOUNT OF FEES, IF ANY, YOU PAID
// DIRECTLY TO MICROCHIP FOR THIS SOFTWARE. MICROCHIP OFFERS NO SUPPORT FOR THE
// SOFTWARE. YOU MAY CONTACT MICROCHIP AT
// https://www.microchip.com/en-us/support-and-training/design-help/client-support-services
// TO INQUIRE ABOUT SUPPORT SERVICES AND APPLICABLE FEES, IF AVAILABLE.

#ifndef __SHLS_VISION_PARAMS_HPP__
#define __SHLS_VISION_PARAMS_HPP__

#include <hls/ap_int.hpp>
#include <stdint.h>

namespace hls {
namespace vision {

enum StorageType {
    FIFO,
    FRAME_BUFFER,
    EXTERNAL_FRAME_BUFFER,
    PRIMITIVE_TYPE_EXTERNAL_FRAME_BUFFER
};

// The corresponding types in OpenCV are defined by CvPixelTypeList.
enum PixelType {
    HLS_8UC1 = 0,
    HLS_8UC2 = 1,
    HLS_8UC3 = 2,
    HLS_8UC4 = 3,
    HLS_16UC1 = 4,
    HLS_16UC3 = 5,
    HLS_16UC4 = 6,
    HLS_16SC1 = 7,
    HLS_16SC3 = 8,
    HLS_32SC1 = 9,
    HLS_32SC3 = 10,
    // HLS_32FC1 = 11,
    // HLS_32FC3 = 12,
};

enum NumPixelsPerCycle {
    NPPC_1 = 1,
    NPPC_2 = 2,
    NPPC_4 = 4,
    NPPC_8 = 8,
    NPPC_16 = 16,
    NPPC_32 = 32,
    NPPC_64 = 64
};

/**
 * This struct is used to convert the combination of pixels per clock and pixel
 * type to an ap_uint (or ap_int if using a signed type) of size DT.W bits
 * stored in DT.T. The bitwidth (DT.W) is equal to #pixel components * component
 * size * pixels per clock) stored it DT.T.
 */
template <PixelType PIXEL_T, NumPixelsPerCycle NPPC = NPPC_1> struct DT {
    struct T {};
};

#define __VISION_DT__(NAME, PerChannelWidth_, NumChannels_, APType,            \
                      PixelPrimT_)                                             \
    template <NumPixelsPerCycle NPPC> struct DT<NAME, NPPC> {                  \
        static constexpr unsigned PerChannelPixelWidth = PerChannelWidth_;     \
        static constexpr unsigned NumChannels = NumChannels_;                  \
        static constexpr unsigned W = PerChannelWidth_ * NumChannels_ * NPPC;  \
        using T = APType<W>;                                                   \
        using PixelPrimT = PixelPrimT_;                                        \
    }

__VISION_DT__(HLS_8UC1, 8, 1, ap_uint, uint8_t);
__VISION_DT__(HLS_8UC2, 8, 2, ap_uint, uint8_t);
__VISION_DT__(HLS_8UC3, 8, 3, ap_uint, uint8_t);
__VISION_DT__(HLS_8UC4, 8, 4, ap_uint, uint8_t);
__VISION_DT__(HLS_16UC1, 16, 1, ap_uint, uint16_t);
__VISION_DT__(HLS_16UC3, 16, 3, ap_uint, uint16_t);
__VISION_DT__(HLS_16UC4, 16, 4, ap_uint, uint16_t);
__VISION_DT__(HLS_16SC1, 16, 1, ap_int, int16_t);
__VISION_DT__(HLS_16SC3, 16, 3, ap_int, int16_t);
__VISION_DT__(HLS_32SC1, 32, 1, ap_int, int32_t);
__VISION_DT__(HLS_32SC3, 32, 3, ap_int, int32_t);

// TODO: revisit floating point types (search FC):
// __VISION_DT__(HLS_32FC1, 32, 1, ap_int, float);
// __VISION_DT__(HLS_32FC3, 32, 3, ap_int, float);

#undef __VISION_DT__

#include <stdint.h>

template <unsigned W> struct InvalidWidthToUsePrimitiveType {
    static_assert(false && W, "Invalid width to use primitive type pointer.");
};

template <unsigned W>
using PrimT = typename std::conditional<
    W <= 8, uint8_t,
    typename std::conditional<
        W <= 16, uint16_t,
        typename std::conditional<
            W <= 32, uint32_t,
            typename std::conditional<
                W <= 64, uint64_t,
                InvalidWidthToUsePrimitiveType<W>>::type>::type>::type>::type;

} // End of namespace vision.
} // End of namespace hls.

#endif
