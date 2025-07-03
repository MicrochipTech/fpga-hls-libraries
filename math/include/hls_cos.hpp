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
//
// Taylor Series cos implementation from https://sourceforge.net/projects/fixedptc/
// Released under BSD License. To see full license, see licenses folder in fixptc_license.

#pragma once
#include "hls_common.hpp"
#include "hls_sin.hpp"
#include "hls_sincos.hpp"

/***
 * @title cos
 */

namespace hls{
namespace math{	

// CORDIC implementation adapted from https://www.eit.lth.se/fileadmin/eit/courses/eitf35/2017/CORDIC_For_Dummies.pdf.
// Modified: Translated to C++, & made compilable with SHLS.

/***
 * @function cos_cordic
 * CORDIC implementation of cos.
 *
 * @template {unsigned int} W_OUT width of the output
 * @template {int} IW_OUT width of integer portion of the output
 * @template {int} N_ITERATIONS number of CORDIC iterations
 * @template {unsigned int} W_IN width of the input (automatically inferred)
 * @template {int} IW_IN width of integer portion of the input (automatically inferred)
 *
 * @param {ap_fixpt<unsigned int W_IN, int IW_IN>} x angle (in radians)
 * @return {ap_fixpt<unsigned int W_OUT, int IW_OUT>} cosine of input angle
 * @example
 * hls::ap_fixpt<10, 2> y = 3.1415;
 * auto x = hls::math::cos_cordic<10, 2, 16>(y); //x will be an ap_fixpt w/ the value 1
 */

template <unsigned int W_OUT, int IW_OUT, int N_ITERATIONS, unsigned int W_IN, int IW_IN>	
ap_fixpt<W_OUT, IW_OUT> cos_cordic(ap_fixpt<W_IN, IW_IN> x) {
  ap_fixpt<W_OUT, IW_OUT> sin;
  ap_fixpt<W_OUT, IW_OUT> cos;
  sincos<N_ITERATIONS>(x, sin, cos);
  return cos;
}

// Implementation adapted from https://sourceforge.net/projects/fixedptc/
// Modified: Translated to C++, & made compilable with SHLS.
// License in licenses folder, disclaimer at the top of header file.

/***
 * @function cos_taylor
 * 2-term Taylor Series implementation of cos.
 *
 * @template {unsigned int} W_OUT width of the output
 * @template {int} IW_OUT width of integer portion of the output
 * @template {unsigned int} W_IN width of the input (automatically inferred)
 * @template {int} IW_IN width of integer portion of the input (automatically inferred)
 *
 * @param {ap_fixpt<unsigned int W_IN, int IW_IN>} x angle (in radians)
 * @return {ap_fixpt<unsigned int W_OUT, int IW_OUT>} cosine of input angle
 * @example
 * hls::ap_fixpt<10, 2> y = 3.1415;
 * auto x = hls::math::cos_taylor<10, 2>(y); //x will be an ap_fixpt w/ the value 1
 */
template <unsigned int W_OUT, int IW_OUT, unsigned int W_IN, int IW_IN>
ap_fixpt<W_OUT, IW_OUT> cos_taylor(ap_fixpt<W_IN, IW_IN> x) {
  ap_fixpt<W_OUT, IW_OUT> HALF_PI(M_PI_2);
  return (sin_taylor<W_OUT, IW_OUT>(HALF_PI - x));
}

/***
 * @function cos_lut
 * Lookup Table implementation of cos.
 * Number of decimal bits of input value is recommended to be less than DECIM (defined in utils/generators/generated_tables/sin_lut_table.hpp)
 *
 * @template {unsigned int} W_OUT width of the output
 * @template {int} IW_OUT width of integer portion of the output
 * @template {unsigned int} W_IN width of the input (automatically inferred)
 * @template {int} IW_IN width of integer portion of the input (automatically inferred)
 *
 * See utils/generators/sin_lut_gentable.cpp to generate your own tables.
 * @param {ap_fixpt<unsigned int W_IN, int IW_IN>} x angle (in radians)
 * @return {ap_fixpt<unsigned int W_OUT, int IW_OUT>} cosine of input angle
 * @example
 * hls::ap_fixpt<10, 2> y = 3.1415;
 * auto x = hls::math::cos_lut<10, 2>(y); //x will be an ap_fixpt w/ the value 1
 */
template <unsigned int W_OUT, int IW_OUT, unsigned int W_IN, int IW_IN>
ap_fixpt<W_OUT, IW_OUT> cos_lut(ap_fixpt<W_IN, IW_IN> x) {
  ap_fixpt<W_IN, IW_IN> HALF_PI(M_PI_2);
  return (sin_lut<W_OUT, IW_OUT>(HALF_PI - x));
}
} //namespace math
} // namespace hls

