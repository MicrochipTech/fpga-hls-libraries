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
#include "../utils/generators/generated_tables/log_table.hpp"

/***
 * @title log2
 */

namespace hls {
namespace math {

// Algorithm adapted from https://www.researchgate.net/profile/Salvador-Tropea/publication/4254293_FPGA_implementation_of_base-N_logarithm/links/564cb22d08aedda4c134385a/FPGA-implementation-of-base-N-logarithm.pdf
// Modified: Made compilable with SHLS.

/***
 * @function log2_lut
 * Lookup Table based implementation of log base 2. If input is negative, then an error will occur.
 *
 * @template {unsigned int} W_OUT width of the output
 * @template {int} IW_OUT width of integer portion of the output
 * @template {unsigned int} W_IN width of the input (automatically inferred)
 * @template {int} IW_IN width of integer portion of the input (automatically inferred)
 *
 * @param {ap_fixpt<unsigned int W_IN, int IW_IN>} num input
 * @param {int} error variable to hold error code value if an error occurs
 * @return {ap_fixpt<unsigned int W_OUT, int IW_OUT>} log2 of input value
 * @example
 * hls::ap_fixpt<10, 2> y = 4;
 * auto x = hls::math::log2_lut<10, 2>(y); //x will be an ap_fixpt w/ the value 2
 */
template <unsigned int W_OUT, int IW_OUT, unsigned int W_IN, int IW_IN>
ap_fixpt<W_OUT, IW_OUT> log2_lut(ap_ufixpt<W_IN, IW_IN> x, int& error = DEFAULT_ERROR) {

  if (x == 0) {
#ifndef __SYNTHESIS__
    printf("Math Error: result is -inf.\n");
#endif
    error = INF_ERROR;
    return (0);
  }
  else if (x == 1) return 0;
  else if (x == 2) return 1;
// IW_IN MUST BE LARGER THAN 2
  // Find j:
  int n = W_IN - 1;
  int j = IW_IN, done = 0;
  DBG_CODE{ printf("x = %f\n", x.to_double());}

   auto xi = x;
  for (unsigned int i = 0; i < n; i ++){
        if (xi[W_IN - 1] == 1) done = 1;
        if (done == 0){
                xi <<= 1;
                j --;
        }
 DBG_CODE{ printf("x = %f, j = %d\n", xi.to_double(), j);}
  }

  ap_fixpt<W_IN, 1> x_normal;
  if (j > 0) x_normal = x >> j;
  else x_normal = x << -(j);
  ap_fixpt<W_OUT, IW_OUT> y = 0;
  ap_fixpt<W_IN, IW_IN> x2;

  // See paper for why W_OUT - 3
  const unsigned int n2 = (W_OUT - 3 > LOG_TABLE_SIZE) ? LOG_TABLE_SIZE : W_OUT - 3;
  DBG_CODE{printf("n = %d\n", n2);}
  for (unsigned int i = 1; i < n2; i++){
        x2 = x_normal + (x_normal >> i);
        if (x2 < 1){
           x_normal = x2;
           y = y - logTable[i];
        }
   DBG_CODE{printf("tab_i = %f, x = %f, y = %f\n", ((ap_fixpt<31,0>)logTable[i]).to_double(), x_normal.to_double(), y.to_double());}
  }
  y = y + j;
  DBG_CODE{printf("y + j = %f\n", y.to_double());}
  return y;
}

// Adapted from http://www.ijsps.com/uploadfile/2014/1210/20141210051242629.pdf
/***
 * @function log2_cordic
 * CORDIC based implementation of log base 2. If input is negative, then an error will occur.
 *
 * @template {unsigned int} W_OUT width of the output
 * @template {int} IW_OUT width of integer portion of the output
 * @template {int} N_ITERATIONS number of CORDIC iterations
 * @template {unsigned int} W_IN width of the input (automatically inferred)
 * @template {int} IW_IN width of integer portion of the input (automatically inferred)
 *
 * @param {ap_fixpt<unsigned int W_IN, int IW_IN>} num input
 * @param {int} error variable to hold error code value if an error occurs
 * @return {ap_fixpt<unsigned int W_OUT, int IW_OUT>} log2 of input value
 * @example
 * hls::ap_fixpt<10, 2> y = 4;
 * auto x = hls::math::log2_cordic<10, 2, 16>(y); //x will be an ap_fixpt w/ the value 2
 */
template <unsigned int W_OUT, int IW_OUT, unsigned int N_ITERATIONS, unsigned int W_IN, int IW_IN>
ap_fixpt<W_OUT, IW_OUT> log2_cordic(ap_ufixpt<W_IN, IW_IN> x, int& error = DEFAULT_ERROR) {

  if (x == 0) {
#ifndef __SYNTHESIS__
    printf("Math Error: result is -inf.\n");
#endif
    error = INF_ERROR;
    return (0);
  }
  else if (x == 1) return 0;
  else if (x == 2) return 1;
// IW_IN MUST BE LARGER THAN 2
  const ap_fixpt<W_OUT, 2> CONV(1/M_LN2);
  // Find j:
  const int n = W_IN - 1;
  int j = IW_IN, done = 0;
  DBG_CODE{ printf("x = %f\n", x.to_double());}

  auto xi = x;
#pragma HLS loop unroll factor(W_IN - 1)
  for (unsigned int i = 0; i < n; i ++){
        if (xi[W_IN - 1] == 1) done = 1;
        if (done == 0){
                xi <<= 1;
                j --;
        }
 DBG_CODE{ printf("x = %f, j = %d\n", xi.to_double(), j);}
  }

  ap_fixpt<W_IN, 1> x_normal;
  if (j > 0) x_normal = x >> j;
  else x_normal = x << -(j);

  ap_fixpt<W_OUT, IW_OUT> x2(x_normal+1);
  ap_fixpt<W_OUT, IW_OUT> y(x_normal-1);
  ap_fixpt<W_OUT, IW_OUT> z(0);
  ap_fixpt<W_OUT, IW_OUT> zero(0);
  cordic_hyp<N_ITERATIONS, VECTORING>(z, x2, y);

  z <<= 1;
  z = z * CONV;

  z = z + j;
  DBG_CODE{printf("z + j = %f\n", z.to_double());}
  return z;
}
} // namespace math
} // namespace hls
