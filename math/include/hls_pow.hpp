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
//
// Taylor Series pow implementation from https://sourceforge.net/projects/fixedptc/
// Released under BSD License. To see full license, see licenses folder in fixptc_license.

#pragma once
#include "hls_common.hpp"
#include "hls_exp.hpp"
#include "hls_log.hpp"

/***
 * @title pow
 */

namespace hls {
namespace math {
// Implementation adapted from https://sourceforge.net/projects/fixedptc/
// Modified: Made compilable with SHLS.
// License in licenses folder, disclaimer at the top of header file.	

/***
 * @function pow
 * Implementation of pow using ln_taylor and exp.
 *
 * @template {unsigned int} W_OUT width of the output
 * @template {int} IW_OUT width of integer portion of the output
 * @template {unsigned int} W_IN width of the input (automatically inferred)
 * @template {int} IW_IN width of integer portion of the input (automatically inferred)
 *
 * @param {ap_fixpt<W_IN, IW_IN>} base base number
 * @param {ap_fixpt<W_IN, IW_IN>} pow power to raise the base to
 * @param {int} error variable to hold error code if error encountered
 * @return {ap_fixpt<W_OUT, IW_OUT>} base number raised to the power of pow
 * @example
 * hls::ap_fixpt<10, 2> y = 2;
 * hls::ap_fixpt<10, 3> base = 2;
 * auto x = hls::math::pow<10, 2>(base, y); //x will be an ap_fixpt w/ the value 4
 */
template <unsigned int W_OUT, int IW_OUT, unsigned int W_IN, int IW_IN>
ap_fixpt<W_OUT, IW_OUT> pow(ap_fixpt<W_IN, IW_IN> base,
                             ap_fixpt<W_IN, IW_IN> pow,
			     int& error = DEFAULT_ERROR) {
  ap_fixpt<W_OUT, IW_OUT> result;
  int n = 0;
  if (pow == 0)
    return 1;
  if (base == 0) return 0;

  // TODO: Fix when complex nums are supported
  //  For now: check if power is int or not
  int in_frac_bits = W_IN - IW_IN;
  ap_fixpt<W_IN, IW_IN> ONE(1);
  ap_ufixpt<W_IN, IW_IN> u_base = base;
  ap_fixpt<W_IN, IW_IN> FIXEDPT_FMASK((ONE) - (ONE >> in_frac_bits));
  if (base < 0) {
    if ((pow & FIXEDPT_FMASK) != 0) {
#ifndef __SYNTHESIS__
     printf("Argument is invalid, the result will be complex.\n");
#endif
     error = NAN_ERROR;
      return 0;
    }
    if (pow & (ONE))
      n = 1;
    u_base = -base;
  }

  // base^exp = result -> e^(exp * ln(base)) = result
  ap_fixpt<W_OUT, IW_OUT> ln_base = ln_lut<W_OUT, IW_OUT>(u_base);
  DBG_CODE {printf("LN_BASE: %f\n", ln_base.to_double());}
  ap_fixpt<W_OUT, IW_OUT> temp = ln_base * pow;
  DBG_CODE {printf("MULT: %f\n", temp.to_double());}
  result = exp_taylor<W_OUT, IW_OUT>(temp);
  DBG_CODE {printf("exp: %f\n\n", result.to_double());}

  if (n)
    result = -result;

  return result;
}
} // namespace math
} // namespace hls
