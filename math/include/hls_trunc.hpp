// ©2024 Microchip Technology Inc. and its subsidiaries
//
// Subject to your compliance with these terms, you may use this Microchip
// software and any derivatives exclusively with Microchip products. You are
// responsible for complying with third party license terms applicable to your
// use of third party software (including open source software) that may
// accompany this Microchip software. SOFTWARE IS “AS IS.” NO WARRANTIES,
// WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING
// ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR
// A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY
// INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST
// OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED,
// EVEN IF MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE
// FORESEEABLE.  TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP’S TOTAL
// LIABILITY ON ALL CLAIMS LATED TO THE SOFTWARE WILL NOT EXCEED AMOUNT OF
// FEES, IF ANY, YOU PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE. MICROCHIP
// OFFERS NO SUPPORT FOR THE SOFTWARE. YOU MAY CONTACT MICROCHIP AT
// https://www.microchip.com/en-us/support-and-training/design-help/client-support-services
// TO INQUIRE ABOUT SUPPORT SERVICES AND APPLICABLE FEES, IF AVAILABLE.

#pragma once
#include "hls_common.hpp"
#include "hls_ceil.hpp"
#include "hls_floor.hpp"

/***
 * @title trunc
 */

namespace hls {
namespace math {

/***
 * @function trunc
 * Rounds the input towards 0.
 *
 * @template {unsigned int} W_OUT width of the output
 * @template {int} IW_OUT width of integer portion of the output
 * @template {unsigned int} W_IN width of the input (automatically inferred)
 * @template {int} IW_IN width of integer portion of the input (automatically inferred)
 *
 * @param {ap_fixpt<unsigned int W_IN, int IW_IN>} input
 * @return {ap_fixpt<unsigned int W_OUT, int IW_OUT>} the nearest integer that is not larger in magnitute to the input
 * @example
 * hls::ap_fixpt<10, 2> x = 12.222;
 * auto y = hls::math::trunc<32, 16>(x) // y will be an ap_fixpt number with a value of 12.
*/
template <unsigned int W_OUT, int IW_OUT, unsigned int W_IN, int IW_IN>
ap_fixpt<W_OUT, IW_OUT> trunc(ap_fixpt<W_IN, IW_IN> fp) {
  ap_fixpt<W_OUT, IW_OUT> result;
  if (fp <= 0)
    return (ceil<W_OUT, IW_OUT>(fp));
  else
    return (floor<W_OUT, IW_OUT>(fp));
}
} // namespace math
} // namespace hls
