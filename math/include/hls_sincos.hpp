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
#include "hls_cordic.hpp"

/***
 * @title sincos
 */

namespace hls{
namespace math{
// CORDIC implementation adapted from https://www.eit.lth.se/fileadmin/eit/courses/eitf35/2017/CORDIC_For_Dummies.pdf.
// Modified: Translated to C++, & made compilable with SHLS.	

/***
 * @function sincos
 * CORDIC implementation of sincos.
 *
 * @template {unsigned int} W_OUT width of the output
 * @template {int} IW_OUT width of integer portion of the output
 * @template {int} N_ITERATIONS number of CORDIC iterations
 * @template {unsigned int} W_IN width of the input (automatically inferred)
 * @template {int} IW_IN width of integer portion of the input (automatically inferred)
 *
 * @param {ap_fixpt<W_IN, IW_IN>} desired_angle angle (in radians)
 * @param {ap_fixpt<W_OUT, IW_OUT>} sin variable that will hold the value of sine after the function executes
 * @param {ap_fixpt<W_OUT, IW_OUT>} cos variable that will hold the value of cosine after the function executes
 * @example
 * hls::ap_fixpt<16, 2> sin = 0;
 * hls::ap_fixpt<16, 2> cos = 0; 
 * hls::ap_fixpt<16, 2> x = 3.14; 
 * hls::math::sincos<16, 32, 16>(x, sin, cos) 
 */
template <int N_ITERATIONS, unsigned int W_IN, int IW_IN, unsigned int W_OUT, int IW_OUT>
void sincos(ap_fixpt<W_IN, IW_IN> desired_angle, ap_fixpt<W_OUT, IW_OUT>& sin, ap_fixpt<W_OUT, IW_OUT>& cos) {
  typedef ap_fixpt<W_IN - IW_IN + 2, 2> T;
  ap_uint<2> q;
  T x = 0.60725293500888125616;
  T y = 0;
  T ZERO(0);

  bool sign = desired_angle[W_IN - 1];
  // First get angle to be positive:
  auto abs_in = sign ? ap_fixpt<W_IN, IW_IN>(-desired_angle) : (desired_angle);
  
  // Find quadrant & get angle btwn 0 - 90:
  T r;
  const ap_ufixpt<W_IN - IW_IN + 1, 1> pi2(M_PI_2); // pi/2
  const ap_ufixpt<W_IN - IW_IN + 1, 0> pi2_inv(M_2_PI);     // 2/pi
  ap_uint<IW_IN> q_I = abs_in * pi2_inv;
  r = abs_in - ap_fixpt<W_IN, IW_IN>(q_I * pi2);
  q = q_I;

  DBG_CODE{
  printf("quad = %llu\n", q.to_uint64());
  printf("BEFORE ROTATE = %f\n", desired_angle.to_double());
  printf("DESIRED ANGLE = %f\n", r.to_double());
  }

  //  r -> 0
  cordic<N_ITERATIONS, ROTATING>(r, x, y);

  if (q == 0) {
    sin = y;   // sin(t + 0      ) = sin(t)
    cos = x;   // cos(t + 0      ) = cos(t)
  } else if (q == 1) {
    sin = x;   // sin(t + pi/2   ) = cos(t)
    cos = -y;  // cos(t + pi/2   ) = -sin(t)
  } else if (q == 2) {
    sin = -y;  // sin(t + pi     ) = -sin(t)
    cos = -x;  // cos(t + pi     ) = -cos(t)
  } else{
    sin = -x;  // sin(t + 3*pi/2) = -cos(t)
    cos = y;   // cos(t + 3*pi/2) = sin(t);
  }

  if (sign)
    sin = -sin;

//   printf("ANGLE: %f: COS = %f, SIN = %f\n\n", desired_angle.to_double(),
//   x.to_double(), y.to_double());
}
}
}
