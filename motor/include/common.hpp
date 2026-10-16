
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
// MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE
// FORESEEABLE.  TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP’S TOTAL
// LIABILITY ON ALL CLAIMS LATED TO THE SOFTWARE WILL NOT EXCEED AMOUNT OF
// FEES, IF ANY, YOU PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE. MICROCHIP
// OFFERS NO SUPPORT FOR THE SOFTWARE. YOU MAY CONTACT MICROCHIP AT
// https://www.microchip.com/en-us/support-and-training/design-help/client-support-services
// TO INQUIRE ABOUT SUPPORT SERVICES AND APPLICABLE FEES, IF AVAILABLE.

#ifndef __SHLS_MOTOR_COMMON_HPP__
#define __SHLS_MOTOR_COMMON_HPP__

#include <cmath>
/**
 * @brief Check if two floating-point values are “close enough”.
 *
 * Uses a combined relative/absolute tolerance:
 * - Let err=∣a−b∣,ε=eps.
 * - Relative bound: err≤ε⋅max(1,∣a∣,∣b∣).
 * - Absolute bound: err<5.
 *
 * The function returns `true` if either the relative criterion or the absolute
 * criterion is satisfied:
 * 
 * err ≤ eps × max(1, |a|, |b|)  or  err < 5.0
 *
 * This design provides stable behavior near zero while also allowing a hard
 * absolute tolerance for large-magnitude comparisons.
 *
 * @param a   First value to compare.
 * @param b   Second value to compare.
 * @param eps Relative tolerance multiplier (unitless). Defaults to @c 1e-6 .
 * @return @c true if @p a and @p b are considered close; otherwise @c false.
 *
 * @note The absolute threshold of @c 5.0f dominates when @c |a| and @c |b|
 *       are small or when a coarse absolute match is acceptable.
 * @see std::fabs, std::max
 */
bool close_enough(float a, float b, float eps = 1e-6) {
    const auto err = std::fabs(a - b);
    return err <= eps * std::max(1.0f,
           std::max(std::fabs(a), std::fabs(b))) || err < 5;
}


/**
 * @brief Convert a fixed-point encoded angle (raw theta) into radians.
 *
 * This function interprets a raw integer phase value, encoded with
 * THETA_BITS of fractional resolution (Q0.THETA_BITS format, representing
 * fractions of a full turn), and converts it into radians in the range
 * (-π, π].
 *
 * The conversion is performed entirely using HLS fixed-point types
 * to ensure synthesizability and to avoid overflow or precision loss
 * during scaling. An internal wide accumulator type (Tacc) is used to
 * guarantee sufficient integer and fractional precision before casting
 * down to the target fixed-point output type.
 *
 * @tparam W_RAD       Total bit-width of the output fixed-point type (radians).
 * @tparam IW_RAD      Number of integer bits in the output fixed-point type.
 * @tparam THETA_BITS  Resolution of the raw input phase, i.e. number of fractional
 *                     bits representing [0, 1) turns.
 *
 * @param raw_theta    The raw input angle in Q0.THETA_BITS turn format.
 *                     Interpreted as an integer in the range [0, 2^THETA_BITS).
 *
 * @return hls::ap_fixpt<W_RAD, IW_RAD>
 *         The converted angle in radians, represented as fixed-point with
 *         W_RAD total bits and IW_RAD integer bits. The value is wrapped into
 *         the interval (-π, π].
 *
 * @note Internally uses a wider accumulator type to prevent overflow when
 *       multiplying by 2π. The result is then cast down to the specified
 *       output type.
 */
template <int W_RAD, int IW_RAD, int THETA_BITS>
hls::ap_fixpt<W_RAD, IW_RAD> theta_to_rad(hls::ap_uint<THETA_BITS> raw_theta) {
    static_assert(W_RAD > IW_RAD, "theta_to_rad needs fractional bits in its return type");

	hls::ap_fixpt<2 * THETA_BITS + 1, THETA_BITS> theta_in_rad = (raw_theta);
	hls::ap_ufixpt<THETA_BITS + 2, 2> pi = M_PI;
	theta_in_rad = (theta_in_rad >> (THETA_BITS-1)) * pi ;
    return hls::ap_fixpt<W_RAD, IW_RAD>(theta_in_rad);
}

template <unsigned int W_OUT, int IW_OUT, unsigned int W_IN, int IW_IN>
hls::ap_fixpt<W_OUT, IW_OUT> theta_to_rad(hls::ap_fixpt<W_IN, IW_IN> raw_theta) {
    static_assert(W_OUT > IW_OUT, "theta_to_rad needs fractional bits in its return type");
    hls::ap_ufixpt<W_OUT, IW_OUT> pi = M_PI;
	hls::ap_fixpt<W_OUT, IW_OUT> theta_in_rad = raw_theta * 2 * pi ;
    return (theta_in_rad);
}


#endif // __SHLS_MOTOR_COMMON_HPP__