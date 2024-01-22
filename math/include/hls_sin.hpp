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
// Taylor Series cos implementation from https://sourceforge.net/projects/fixedptc/
// Released under BSD License. To see full license, see licenses folder in fixptc_license.

#pragma once
#include "hls_common.hpp"
#include "hls_sincos.hpp"
#include "../utils/generators/generated_tables/sin_lut_table.hpp"

/***
 * @title sin
 */

namespace hls {
namespace math {

// CORDIC implementation adapted from https://www.eit.lth.se/fileadmin/eit/courses/eitf35/2017/CORDIC_For_Dummies.pdf.
// Modified: Translated to C++, & made compilable with SHLS.

/***
 * @function sin_cordic
 * CORDIC implementation of sin.
 *
 * @template {unsigned int} W_OUT width of the output
 * @template {int} IW_OUT width of integer portion of the output
 * @template {int} N_ITERATIONS number of CORDIC iterations
 * @template {unsigned int} W_IN width of the input (automatically inferred)
 * @template {int} IW_IN width of integer portion of the input (automatically inferred)
 *
 * @param {ap_fixpt<unsigned int W_IN, int IW_IN>} x angle (in radians)
 * @return {ap_fixpt<unsigned int W_OUT, int IW_OUT>} sine of input angle
 * @example
 * hls::ap_fixpt<10, 2> y = 3.1415;
 * auto x = hls::math::sin_cordic<10, 2, 16>(y); //x will be an ap_fixpt w/ the value 0
 */	
template <unsigned int W_OUT, int IW_OUT, int N_ITERATIONS, unsigned int W_IN, int IW_IN>
ap_fixpt<W_OUT, IW_OUT> sin_cordic(ap_fixpt<W_IN, IW_IN> x) {
  ap_fixpt<W_OUT, IW_OUT> sin;
  ap_fixpt<W_OUT, IW_OUT> cos;
  sincos<N_ITERATIONS>(x, sin, cos);
  return sin;
}

// Implementation adapted from https://sourceforge.net/projects/fixedptc/
// Modified: Made compilable with SHLS.
// License in licenses folder, disclaimer at the top of header file.

/***
 * @function sin_taylor
 * 2-term Taylor Series implementation of sin.
 *
 * @template {unsigned int} W_OUT width of the output
 * @template {int} IW_OUT width of integer portion of the output
 * @template {unsigned int} W_IN width of the input (automatically inferred)
 * @template {int} IW_IN width of integer portion of the input (automatically inferred)
 *
 * @param {ap_fixpt<unsigned int W_IN, int IW_IN>} x angle (in radians)
 * @return {ap_fixpt<unsigned int W_OUT, int IW_OUT>} sine of input angle
 * @example
 * hls::ap_fixpt<10, 2> y = 3.1415;
 * auto x = hls::math::sin_taylor<10, 2>(y); //x will be an ap_fixpt w/ the value 0
 */
template <unsigned int W_OUT, int IW_OUT, unsigned int W_IN, int IW_IN>
ap_fixpt<W_OUT, IW_OUT> sin_taylor(ap_fixpt<W_IN, IW_IN> x) {

  int sign = 1;
  ap_fixpt<W_OUT, IW_OUT> sqr, result;
  const ap_fixpt<W_OUT, IW_OUT> SK[2] = {0.008333333333333333333,
                                          0.166666666666666666666};
  const ap_fixpt<W_OUT, IW_OUT> PI(M_PI);
  const ap_fixpt<W_OUT, IW_OUT> TWO_PI(2 * M_PI);
  const ap_fixpt<W_OUT, IW_OUT> HALF_PI(M_PI_2);

DBG_CODE{
  printf("x = %f\n", x.to_double());
}

  // x %= 2PI
DBG_CODE{
  printf("x in rad = %f\n", x.to_double());
}
  ap_fixpt<W_IN, W_IN> div = x / TWO_PI;
  ap_fixpt<W_IN, IW_IN> div2 = TWO_PI * div;
  x = x - div2;

DBG_CODE{
  printf("principle x = %f\n", x.to_double());
}

  if (x < 0)
    x = PI * 2 + x;
  if ((x > HALF_PI) && (x <= PI))
    x = PI - x;
  else if ((x > PI) && (x <= (PI + HALF_PI))) {
    x = x - PI;
    sign = -1;
  } else if (x > (PI + HALF_PI)) {
    x = (PI << 1) - x;
    sign = -1;
  }
  sqr = x * x;
  result = SK[0];
  result = result * sqr;
  result -= SK[1];
  result = result * sqr;
  result = result + 1;
  result = result * x;
  result = sign * result;
  return result;
}

/***
 * @function sin_lut
 * Lookup Table implementation of sin. 
 * Number of decimal bits of input value is recommended to be less than DECIM (defined in utils/generators/generated_tables/sin_lut_table.hpp)
 *
 * @template {unsigned int} W_OUT width of the output
 * @template {int} IW_OUT width of integer portion of the output
 * @template {unsigned int} W_IN width of the input (automatically inferred)
 * @template {int} IW_IN width of integer portion of the input (automatically inferred)
 *
 * See utils/generators/sin_lut_gentable.cpp to generate your own tables.
 * @param {ap_fixpt<unsigned int W_IN, int IW_IN>} x angle (in radians)
 * @return {ap_fixpt<unsigned int W_OUT, int IW_OUT>} sine of input angle
 * @example
 * hls::ap_fixpt<10, 2> y = 3.1415;
 * auto x = hls::math::sin_lut<10, 2>(y); //x will be an ap_fixpt w/ the value 0
 */

template <unsigned int W_OUT, int IW_OUT, unsigned int W_IN, int IW_IN>
ap_fixpt<W_OUT, IW_OUT> sin_lut(ap_fixpt<W_IN, IW_IN> x) {

  typedef ap_fixpt<2 + W_OUT - IW_OUT, 2> T;
  const T HALF_PI(M_PI_2);

  const ap_fixpt<3 + W_OUT - IW_OUT, 3> PI(M_PI);
  const ap_fixpt<4 + W_OUT - IW_OUT, 4> TWO_PI(2 * M_PI);
  const ap_fixpt<4 + W_OUT - IW_OUT, 4> THREE_HALVES_PI(3 * M_PI / 2);

  const int DECIM = _HLS_SIN_LUT_DECIM;
  int sign = 1;
  while (x < 0)
    x = x + TWO_PI;
  while (x > TWO_PI)
    x = x - TWO_PI;

  DBG_CODE{ printf("x = %f\n", x.to_double()); }

  DBG_CODE {
	  if (DECIM < (W_IN - IW_IN)) printf("Because DECIM is less than input fractional bits (i.e. DECIM < W_IN - IW_IN), precision will be lost.\nPlease either re-generate sin_lut tables with a larger DECIM, or decrease number of input fractional bits.\n");
  }
  ap_ufixpt<DECIM + 1, 1> first_quad_x;

  // Depending on the quadrant, we wanna find sin(A - B), where (A - B) will
  // be within [0, PI / 2).
  if (x < HALF_PI) {
      first_quad_x = x;
  } else if (x < PI) {
      // For 2nd quadrant, sin(x) = sin(PI - x).
      first_quad_x = PI - x;
  } else if (x < THREE_HALVES_PI) {
      // For 3rd quadrant, sin(x) = -sin(x - PI).
      sign = -1;
      first_quad_x = x - PI;
  } else {
      // For 4th quadrant, sin(x) = -sin(2 * PI - x).
      sign = -1;
      first_quad_x = TWO_PI - x;
  }

  DBG_CODE{
    printf("first_quad_x = %f\n", first_quad_x.to_double());
  }


    // Say X = fa + fb, sin(X) = sin(fa + fb)
    //                         = sin(fa) * cos(fb) + cos(fa) * sin(fb)
    // fa will be the upper bits of X and fb wil be the lower bits of X.
    //
    // We need four tables for sin(fa), cos(fa), sin(fb), and cos(fb), each with
    // 2^(width of fa/b) entries.

  unsigned int EXT_W, fa_width, fb_width;
  EXT_W = DECIM + 1;
  fa_width = EXT_W >> 1;
  fb_width = EXT_W - fa_width;

  ap_ufixpt<2 * DECIM + 1, 2 + DECIM> x_ext(first_quad_x);
  ap_uint<1 + DECIM> my_angle(x_ext << DECIM);
  unsigned trans_x = (unsigned)(my_angle.to_uint64());
  unsigned fa = trans_x >> fb_width;
  unsigned fb = trans_x & ((1 << fb_width) - 1);

  DBG_CODE{
    printf("decim = %d, W = %d\n", DECIM, EXT_W);
    printf("fa_width = %d, fb_width = %d\n", fa_width, fb_width);
    printf("trans_x = %x\n", trans_x);
    printf("fa = %d, fb = %d\n", fa, fb);
  }

  T sin_fa = sin_fa_lut[fa];
  T sin_fb = sin_fb_lut[fb];
  T cos_fa = cos_fa_lut[fa];
  T cos_fb = cos_fb_lut[fb];

  DBG_CODE{
    printf("sin_fa = %f, sin_fb = %f\n", sin_fa.to_double(), sin_fb.to_double());
    printf("cos_fa = %f, cos_fb = %f\n", cos_fa.to_double(), cos_fb.to_double());
  }

  T sin_result = sin_fa * cos_fb;
  sin_result += (cos_fa * sin_fb);
  sin_result = sign * sin_result;

  DBG_CODE{
    printf("result = %f\n", sin_result.to_double());
  }
  return(sin_result) ;

}
}
}
