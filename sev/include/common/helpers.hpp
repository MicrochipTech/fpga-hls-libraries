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

#ifndef __SHLS_SEV_HELPERS_HPP__
#define __SHLS_SEV_HELPERS_HPP__

#include "params.hpp"
#include <hls/ap_int.hpp>

namespace hls {
namespace sev {

/**
 * This struct is used to convert the combination of pixels per clock and pixel
 * type to an ap_uint (or ap_int if using a signed type) of size DT.W bits
 * stored in DT.T. The bitwidth (DT.W) is equal to #pixel components * component
 * size * pixels per clock) stored it DT.T.
 */
template <PixelType PIXEL_T, NumPixelsPerCycle NPPC = NPPC_1> struct DT {
    struct T {
        ;
    };
};

// TODO: revisit floating point types (search FC):
template <NumPixelsPerCycle NPPC> struct DT<SEV_8UC1, NPPC> {
    static constexpr unsigned W = 8 * 1 * NPPC;
    using T = ap_uint<W>;
};
template <NumPixelsPerCycle NPPC> struct DT<SEV_16UC1, NPPC> {
    static constexpr unsigned W = 16 * 1 * NPPC;
    using T = ap_uint<W>;
};
template <NumPixelsPerCycle NPPC> struct DT<SEV_16SC1, NPPC> {
    static constexpr unsigned W = 16 * 1 * NPPC;
    using T = ap_int<W>;
};
template <NumPixelsPerCycle NPPC> struct DT<SEV_32UC1, NPPC> {
    static constexpr unsigned W = 32 * 1 * NPPC;
    using T = ap_uint<W>;
};
template <NumPixelsPerCycle NPPC> struct DT<SEV_32FC1, NPPC> {
    static constexpr unsigned W = 32 * 1 * NPPC;
    using T = ap_uint<W>;
};
template <NumPixelsPerCycle NPPC> struct DT<SEV_32SC1, NPPC> {
    static constexpr unsigned W = 32 * 1 * NPPC;
    using T = ap_int<W>;
};
template <NumPixelsPerCycle NPPC> struct DT<SEV_8UC2, NPPC> {
    static constexpr unsigned W = 8 * 2 * NPPC;
    using T = ap_uint<W>;
};
template <NumPixelsPerCycle NPPC> struct DT<SEV_8UC4, NPPC> {
    static constexpr unsigned W = 8 * 4 * NPPC;
    using T = ap_uint<W>;
};
template <NumPixelsPerCycle NPPC> struct DT<SEV_2UC1, NPPC> {
    static constexpr unsigned W = 2 * 1 * NPPC;
    using T = ap_uint<W>;
};
template <NumPixelsPerCycle NPPC> struct DT<SEV_8UC3, NPPC> {
    static constexpr unsigned W = 8 * 3 * NPPC;
    using T = ap_uint<W>;
};
template <NumPixelsPerCycle NPPC> struct DT<SEV_16UC3, NPPC> {
    static constexpr unsigned W = 16 * 3 * NPPC;
    using T = ap_uint<W>;
};
template <NumPixelsPerCycle NPPC> struct DT<SEV_16SC3, NPPC> {
    static constexpr unsigned W = 16 * 3 * NPPC;
    using T = ap_int<W>;
};
template <NumPixelsPerCycle NPPC> struct DT<SEV_16UC4, NPPC> {
    static constexpr unsigned W = 16 * 4 * NPPC;
    using T = ap_uint<W>;
};
template <NumPixelsPerCycle NPPC> struct DT<SEV_10UC1, NPPC> {
    static constexpr unsigned W = 10 * 1 * NPPC;
    using T = ap_uint<W>;
};
template <NumPixelsPerCycle NPPC> struct DT<SEV_10UC4, NPPC> {
    static constexpr unsigned W = 10 * 4 * NPPC;
    using T = ap_uint<W>;
};
template <NumPixelsPerCycle NPPC> struct DT<SEV_12UC1, NPPC> {
    static constexpr unsigned W = 12 * 1 * NPPC;
    using T = ap_uint<W>;
};
template <NumPixelsPerCycle NPPC> struct DT<SEV_12UC4, NPPC> {
    static constexpr unsigned W = 12 * 4 * NPPC;
    using T = ap_uint<W>;
};
template <NumPixelsPerCycle NPPC> struct DT<SEV_10UC3, NPPC> {
    static constexpr unsigned W = 10 * 3 * NPPC;
    using T = ap_uint<W>;
};
template <NumPixelsPerCycle NPPC> struct DT<SEV_12UC3, NPPC> {
    static constexpr unsigned W = 12 * 3 * NPPC;
    using T = ap_uint<W>;
};
template <NumPixelsPerCycle NPPC> struct DT<SEV_32FC3, NPPC> {
    static constexpr unsigned W = 32 * 3 * NPPC;
    using T = ap_uint<W>;
};

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

} // End of namespace sev.
} // End of namespace hls.

#endif

