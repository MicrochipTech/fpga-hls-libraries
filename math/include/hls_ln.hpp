// ©2025 Microchip Technology Inc. and its subsidiaries
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
#include "hls_log2.hpp"

/***
 * @title ln
 */

namespace hls {
namespace math {

// Algorithm adapted from https://www.researchgate.net/profile/Salvador-Tropea/publication/4254293_FPGA_implementation_of_base-N_logarithm/links/564cb22d08aedda4c134385a/FPGA-implementation-of-base-N-logarithm.pdf
// Modified: Made compilable with SHLS.

/***
 * @function ln_lut
 * Lookup Table based implementation of ln. Uses the lookup implementation of log2. If the input is negative, then an error will occur.
 *
 * @template {unsigned int} W_OUT width of the output
 * @template {int} IW_OUT width of integer portion of the output
 * @template {unsigned int} W_IN width of the input (automatically inferred)
 * @template {int} IW_IN width of integer portion of the input (automatically inferred)
 *
 * @param {ap_fixpt<unsigned int W_IN, int IW_IN>} num input
 * @param {int} error variable to hold error code value if an error occurs
 * @return {ap_fixpt<unsigned int W_OUT, int IW_OUT>} natural logarithm of the input value
 * @example
 * hls::ap_fixpt<10, 2> y = 2;
 * auto x = hls::math::ln_lut<10, 2>(y); //x will be an ap_fixpt w/ the value 0.69314718055995
 */
template <unsigned int W_OUT, int IW_OUT, unsigned int W_IN, int IW_IN>
ap_fixpt<W_OUT, IW_OUT> ln_lut(ap_ufixpt<W_IN, IW_IN> x, int& error = DEFAULT_ERROR) {
  // CONV_FACTOR = 1/log_2(e)
  const ap_ufixpt<W_IN + 1, 0> CONV_FACTOR(1/M_LOG2E);
  ap_fixpt<W_OUT, IW_OUT> log2_x = log2_lut<W_OUT, IW_OUT>(x);
  ap_fixpt<W_OUT, IW_OUT, AP_RND> result = CONV_FACTOR * log2_x;
  return result;
}

/***
 * @function ln_cordic
 * CORDIC based implementation of ln. Uses the lookup implementation of log2. If the input is negative, then an error will occur.
 *
 * @template {unsigned int} W_OUT width of the output
 * @template {int} IW_OUT width of integer portion of the output
 * @template {int} N_ITERATIONS number of CORDIC iterations
 * @template {unsigned int} W_IN width of the input (automatically inferred)
 * @template {int} IW_IN width of integer portion of the input (automatically inferred)
 *
 * @param {ap_fixpt<unsigned int W_IN, int IW_IN>} num input
 * @param {int} error variable to hold error code value if an error occurs
 * @return {ap_fixpt<unsigned int W_OUT, int IW_OUT>} natural logarithm of the input value
 * @example
 * hls::ap_fixpt<10, 2> y = 2;
 * auto x = hls::math::ln_cordic<10, 2, 16>(y); //x will be an ap_fixpt w/ the value 0.69314718055995
 */
template <unsigned int W_OUT, int IW_OUT, int N_ITERATIONS, unsigned int W_IN, int IW_IN>
ap_fixpt<W_OUT, IW_OUT> ln_cordic(ap_ufixpt<W_IN, IW_IN> x, int& error = DEFAULT_ERROR) {
  // CONV_FACTOR = 1/log_2(e)
  const ap_ufixpt<W_IN + 1, 0> CONV_FACTOR(1/M_LOG2E);
  ap_fixpt<W_OUT, IW_OUT> log2_x = log2_cordic<W_OUT, IW_OUT, N_ITERATIONS>(x);
  ap_fixpt<W_OUT, IW_OUT, AP_RND> result = CONV_FACTOR * log2_x;
  return result;
}
} // namespace math
} // namespace hls
