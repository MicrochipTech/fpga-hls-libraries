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
#include "hls_ln.hpp"

/***
 * @title log
 */

namespace hls {
namespace math {
// Algorithm adapted from https://www.researchgate.net/profile/Salvador-Tropea/publication/4254293_FPGA_implementation_of_base-N_logarithm/links/564cb22d08aedda4c134385a/FPGA-implementation-of-base-N-logarithm.pdf
// Modified: Made compilable with SHLS.

/***
 * @function log
 * Lookup Table based implementation of log based on the lookup table implementation of log2. 
 * If input is negative, then an error will occur.
 * Negative bases are not supported yet, and will result in a NaN error.
 *
 * @template {unsigned int} W_OUT width of the output
 * @template {int} IW_OUT width of integer portion of the output
 * @template {unsigned int} W_IN width of the input (automatically inferred)
 * @template {int} IW_IN width of integer portion of the input (automatically inferred)
 *
 * @param {ap_fixpt<unsigned int W_IN, int IW_IN>} x input
 * @param {ap_fixpt<unsigned int W_IN, int IW_IN>} base log base
 * @param {int} error variable to hold error code value if an error occurs
 * @return {ap_fixpt<unsigned int W_OUT, int IW_OUT>} log of input value with base
 * @example
 * hls::ap_fixpt<10, 2> y = 4;
 * hls::ap_fixpt<10, 3> base = 2;
 * auto x = hls::math::log<10, 2>(y, base); //x will be an ap_fixpt w/ the value 2
 */
template <unsigned int W_OUT, int IW_OUT, unsigned int W_IN, int IW_IN>
ap_fixpt<W_OUT, IW_OUT> log(ap_ufixpt<W_IN, IW_IN> x,
                             ap_ufixpt<W_IN, IW_IN> base,
			     int& error = DEFAULT_ERROR) {

  if (x < 0 || base <= 0) {
#ifndef __SYNTHESIS__
    printf("Math Error: argument cannot be negative/base cannot be 0.\n");
#endif
    error = NAN_ERROR;
    return (0);
  }

  if (x == 0) {
#ifndef __SYNTHESIS__
    printf("Math Error: result is -inf.\n");
#endif
    error = INF_ERROR;
    return (0);
  }

  if (x == 1)
    return 0;
  return (log2_lut<W_OUT, IW_OUT>(x) /
          log2_lut<W_OUT, IW_OUT>(base));
}
} // namespace math
} // namespace hls
