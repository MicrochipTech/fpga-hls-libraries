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
// Taylor Series exp implementation adapted from https://sourceforge.net/projects/fixedptc/
// Released under BSD license. To see full license, see licenses folder in fixptc_license.

#pragma once
#include "hls_common.hpp"
#include "hls_cordic.hpp"

/***
 * @title exp
 */

namespace hls {
namespace math {

// Implementation adapted from https://sourceforge.net/projects/fixedptc/
// Modified: Made compilable with SHLS.
// License in licenses folder, disclaimer at the top of header file.	

/***
 * @function exp_taylor
 * 6-term Taylor Series implementation of exp.
 *
 * @template {unsigned int} W_OUT width of the output
 * @template {int} IW_OUT width of integer portion of the output
 * @template {unsigned int} W_IN width of the input (automatically inferred)
 * @template {int} IW_IN width of integer portion of the input (automatically inferred)
 *
 * @param {ap_fixpt<unsigned int W_IN, int IW_IN>} fp input
 * @return {ap_fixpt<unsigned int W_OUT, int IW_OUT>} e raised to the power of the input
 * @example
 * hls::ap_fixpt<10, 2> y = 2;
 * auto x = hls::math::exp_taylor<10, 2>(y); //x will be an ap_fixpt w/ the value 7.3890560989
 */
template <unsigned int W_OUT, int IW_OUT, unsigned int W_IN, int IW_IN>
ap_fixpt<W_OUT, IW_OUT> exp_taylor(ap_fixpt<W_IN, IW_IN> fp) {
  const ap_ufixpt<W_IN + 1, 0> ln2(M_LN2); // ln2 = 0.6931471805599453094172
  const ap_ufixpt<W_IN + 2, 1> ln2_inv(1/M_LN2);     // 1/ln2 = 1.4426950408889634073

  // Taylor series take a long time to converge, so we break up e^fp into e^(q_I + rem)
  auto abs_in = fp[W_IN - 1] ? ap_ufixpt<W_IN, IW_IN>(-fp) : ap_ufixpt<W_IN, IW_IN>(fp);  
  ap_uint<W_IN> q_I = abs_in * ln2_inv;
  DBG_CODE{printf("q_I: %d\n", q_I.to_uint64());}
  ap_fixpt<W_IN + 2, 2> rem = abs_in - q_I * ln2;
  if (fp < 0) rem = -rem;

  // Term 6: x/6 -> 2.6667x/16:
  const ap_fixpt<W_IN, IW_IN> div_by_six(0.166666666667);
  ap_fixpt<W_IN, IW_IN> temp = rem * div_by_six;
  ap_fixpt<W_IN, IW_IN> frac = temp;
  ap_fixpt<W_OUT, IW_OUT> result = frac + 1;

  DBG_CODE{
    double x = rem.to_double();
    double d = 1 + x/6;
    printf("START: %f\n", x);
    printf("Term 6: actual %f, expected %f\n", result.to_double(), d);
  }

  // Term 5: x/5 -> 1.6x/8
  const ap_fixpt<W_IN, IW_IN> div_by_five(0.2);
  frac = (rem) * div_by_five;
  result = result * frac;
  result = result + 1;
  DBG_CODE{
    double x = rem.to_double();
    double d = (x/5);
    printf("Term 5: actual %f, expected %f\n", frac.to_double(), d);
  }

  // Term 4: x/4 -> x/4
  frac = rem >> 2;
  result = result * frac;
  result = result + 1;
  DBG_CODE{
    double x = rem.to_double();
    double d = (x/4);
    printf("Term 4: actual %f, expected %f\n", frac.to_double(), d);
  }

  // Term 3: x/3 -> 2.6667/8
  frac = temp << 1;
  result = result * frac;
  result = result + 1;
  DBG_CODE{
    double x = rem.to_double();
    double d = (x/3);
    printf("Term 3: actual %f, expected %f\n", frac.to_double(), d);
  }

  // Term 2: x/2 -> x/2:
  frac = rem >> 1;
  result = result * frac;
  result = result + 1;

  DBG_CODE{
    double x = rem.to_double();
    double d = (x/2);
    printf("Term 2: actual %f, expected %f\n", frac.to_double(), d);
  }

  // Term 1: x
  result = result * rem;
  result = result + 1;

  ap_fixpt<W_OUT, IW_OUT> e_to_q_I;
  ap_fixpt<W_OUT, IW_OUT> ONE(1);
  
  if (fp < 0) e_to_q_I = ONE >> q_I;
  else e_to_q_I = ONE << q_I;
  DBG_CODE{
    printf("SCALING %f\n", e_to_q_I.to_double());
  }

  result = result * e_to_q_I;
  return result;
	
}

// Implementation adapted from https://www.researchgate.net/publication/224614338_Implementation_of_hyperbolic_functions_using_CORDIC_algorithm
// And see: https://web.cs.ucla.edu/digital_arithmetic/files/ch11.pdf
// Modified: Made compilable with SHLS.
// License in licenses folder, disclaimer at the top of header file.

/***
 * @function exp_cordic
 *
 * CORDIC implementation of exp.
 * Takes in x as a fixed point number, returns exp(x).
 *
 * @template {unsigned int} W_OUT width of the output
 * @template {int} IW_OUT width of integer portion of the output
 * @template {int} N_ITERATIONS number of CORDIC iterations
 * @template {unsigned int} W_IN width of the input (automatically inferred)
 * @template {int} IW_IN width of integer portion of the input (automatically inferred)
 *
 * @param {ap_fixpt<unsigned int W_IN, int IW_IN>} fp Input
 * @return {ap_fixpt<unsigned int W_OUT, int IW_OUT>} e raised to the power of the input
 * @example
 * hls::ap_fixpt<10, 2> y = 2;
 * auto x = hls::math::exp_cordic<10, 2, 16>(y); //x will be an ap_fixpt w/ the value 7.3890560989
 */
template <unsigned int W_OUT, int IW_OUT, unsigned int N_ITERATIONS, unsigned int W_IN, int IW_IN>
ap_fixpt<W_OUT, IW_OUT> exp_cordic(ap_fixpt<W_IN, IW_IN> fp) {
 // Limitations on CORDIC angle recommended here as 1.11817, ln2 less than 1.11817, so we're safe
  const ap_ufixpt<W_IN + 1, 0> ln2(M_LN2); // ln2 = 0.6931471805599453094172
  const ap_ufixpt<W_IN + 2, 1> ln2_inv(1/M_LN2);     // 1/ln2 = 1.4426950408889634073

  auto abs_in = fp[W_IN - 1] ? ap_ufixpt<W_IN, IW_IN>(-fp) : ap_ufixpt<W_IN, IW_IN>(fp);
  ap_uint<W_IN> q_I = abs_in * ln2_inv;
  ap_fixpt<W_IN, IW_IN> rem = abs_in - q_I * ln2;
  if (fp < 0) rem = -rem; 

  ap_fixpt<W_IN, IW_IN> Z(0);
  ap_fixpt<W_IN, IW_IN> x(1.20749613601);
  ap_fixpt<W_IN, IW_IN> y(1.20749613601);
 
  DBG_CODE{ printf("START %f\n", rem.to_double()); }
  cordic_hyp<N_ITERATIONS, ROTATING>(rem, x, y);

  // Hold results of small part first
  ap_fixpt<W_OUT, IW_OUT> result = x;

  DBG_CODE { printf("before mult = %f\n", result.to_double()); }

  ap_fixpt<W_OUT, IW_OUT> e_to_q_I;
  ap_fixpt<W_OUT, IW_OUT> ONE(1);
  if (fp < 0) e_to_q_I = ONE >> q_I;
  else e_to_q_I = ONE << q_I;

  result = result * e_to_q_I;

  DBG_CODE { printf("cordic result = %f\n\n", result.to_double()); }
 return result;
}
} // namespace math
} // namespace hls
