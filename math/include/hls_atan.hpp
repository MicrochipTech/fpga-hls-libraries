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
#include "hls_cordic.hpp"
#include "hls_abs.hpp"

/***
 * @title atan
 */

namespace hls {
namespace math {
	
// Algorithm from https://ieeexplore.ieee.org/document/6375931 

/***
 * @function atan_rational
 * A rational function approximation of arctan. 
 *
 * @template {unsigned int} W_OUT width of the output
 * @template {int} IW_OUT width of integer portion of the output
 * @template {unsigned int} W_IN width of the input (automatically inferred)
 * @template {int} IW_IN width of integer portion of the input (automatically inferred)
 *
 * @param {ap_fixpt<unsigned int W_IN, int IW_IN>} num input
 * @return {ap_fixpt<unsigned int W_OUT, int IW_OUT>} arctangent of input value (in radians)
 * @example
 * hls::ap_fixpt<10, 2> y = 1;
 * auto x = hls::math::atan_rational<10, 2>(y); // x will be an ap_fixpt number with the value 0.785398163
 */
template <unsigned int W_OUT, int IW_OUT, unsigned int W_IN, int IW_IN>
ap_fixpt<W_OUT, IW_OUT> atan_rational(ap_fixpt<W_IN, IW_IN> num) {
  typedef ap_fixpt<W_IN, IW_IN> T;
  int sign = 1;
  if (num < 0){
     num = -num;
     sign = -1;
  }

  T B(0.596227);
  T ONE(1);
  T HALF_PI(M_PI_2);

  //(Bx + x^2)/(1 + 2Bx + x^2)
  
  T bx_a(abs<W_IN, IW_IN>(B * num));
  T n(num * num);
  n += bx_a;
  T denom = (ONE + bx_a + n);
  T atan_1q(n / denom);
  atan_1q = atan_1q * HALF_PI;
  if (sign < 0)
    return (-(atan_1q));

  return (atan_1q);	  
}

// Algorithm from https://www.eit.lth.se/fileadmin/eit/courses/eitf35/2017/CORDIC_For_Dummies.pdf
// Modified: Translated to C++, & made compilable with SHLS.

/***
 * @function atan_cordic
 * CORDIC implementation of arctan.
 *
 * @template {unsigned int} W_OUT width of the output
 * @template {int} IW_OUT width of integer portion of the output
 * @template {int} N_ITERATIONS number of CORDIC iterations
 * @template {unsigned int} W_IN width of the input (automatically inferred)
 * @template {int} IW_IN width of integer portion of the input (automatically inferred)
 *
 * @param {ap_fixpt<unsigned int W_IN, int IW_IN>} num input
 * @return {ap_fixpt<unsigned int W_OUT, int IW_OUT>} arctangent of input value (in radians)
 * @example
 * hls::ap_fixpt<10, 2> y = 1;
 * auto x = hls::math::atan_cordic<10, 2, 16>(y); // x will be an ap_fixpt number with the value 0.785398163
 */
template <unsigned int W_OUT, int IW_OUT, int N_ITERATIONS, unsigned int W_IN, int IW_IN>
ap_fixpt<W_OUT, IW_OUT> atan_cordic(ap_fixpt<W_IN, IW_IN> num) {
  typedef ap_fixpt<W_IN, IW_IN> T;

  T x(1);
  T y(num);
  T angle(0);
  T HALF_PI(M_PI_2);

  cordic<N_ITERATIONS, VECTORING>(angle, x, y);

  return angle;
}
} // namespace math
} // namespace hls
