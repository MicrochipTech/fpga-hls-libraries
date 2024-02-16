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
#include "hls_asin.hpp"

/***
 * @title acos
 */

namespace hls {
namespace math {

// CORDIC implementation adapted from http://www.andraka.com/files/crdcsrvy.pdf.	

/***
 * @function acos_cordic 
 * CORDIC implementation of arccos. Note that accuracy drops as input -> +/-1.
 * If the input is not within [-1, 1], then an error will occur.
 *
 * @template {unsigned int} W_OUT width of the output
 * @template {int} IW_OUT width of integer portion of the output
 * @template {int} N_ITERATIONS number of CORDIC iterations
 * @template {unsigned int} W_IN width of the input (automatically inferred)
 * @template {int} IW_IN width of integer portion of the input (automatically inferred)
 *
 * @param {ap_fixpt<unsigned int W_IN, int IW_IN>} num input
 * @param {int} error variable to hold error code value if an error occurs
 * @return {ap_fixpt<unsigned int W_OUT, int IW_OUT>} arccosine of input value (in radians)
 * @example
 * hls::ap_fixpt<10, 2> y = 0.707;
 * auto x = hls::math::acos_cordic<10, 2, 16>(y); // x will be an ap_fixpt number with the value 0.785549163 
 */	
template <unsigned int W_OUT, int IW_OUT, int N_ITERATIONS, unsigned int W_IN, int IW_IN>
ap_fixpt<W_OUT, IW_OUT> acos_cordic(ap_fixpt<W_IN, IW_IN> num, int& error = DEFAULT_ERROR) {
// Check for Out of Domain error:
  if (num < -1 || num > 1){
#ifndef __SYNTHESIS__
    printf("Argument is invalid. Must be within [-1, 1].\n");
#endif
    error = NAN_ERROR;
    return 0;
  }
  ap_fixpt<W_OUT, IW_OUT> result;
  ap_fixpt<W_OUT, IW_OUT> HALF_PI(M_PI_2);
  result = HALF_PI - asin_cordic<W_OUT, IW_OUT, N_ITERATIONS>(num);

  return result;
}
} // namespace math
} // namespace hls
