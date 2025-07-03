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

/***
 * @title sqrt
 */

namespace hls {
namespace math {

inline unsigned long long nextPowerOf2(unsigned long long n) {
  n--;
  n |= n >> 1;
  n |= n >> 2;
  n |= n >> 4;
  n |= n >> 8;
  n |= n >> 16;
  n |= n >> 32ull;
  return ++n;
}

template <unsigned int IW> inline unsigned int log2Pow(unsigned long long n) {
  unsigned int l = 1;
  for (unsigned int i = 0; i < IW; i++) {
    if (n > 1) {
      l++;
      n >>= 1;
    }
  }
  return l;
}

template <unsigned int W, int IW>
inline ap_ufixpt<W, IW> fixpt_ceil(ap_ufixpt<W, IW> x) {
  unsigned short intPart = x(W - 1, W - IW);
  unsigned short fracPart = x(W - IW - 1, 0);
  if (fracPart > 0)
    intPart++;
  return ap_ufixpt<W, IW>(intPart);
}

//This is an experimental implementation based on the following article. Here is the link for more information about the algorithm.
// https://iopscience.iop.org/article/10.1088/1742-6596/1314/1/012008.

/***
 * @function sqrt
 * Iterative implementation of sqrt.
 *
 * Takes in x as a fixed point number, returns sqrt(x).
 *
 * @template {unsigned int} W_OUT width of the output
 * @template {int} IW_OUT width of integer portion of the output
 * @template {int} N_ITERATIONS number of iterations
 * @template {unsigned int} W_IN width of the input (automatically inferred)
 * @template {int} IW_IN width of integer portion of the input (automatically inferred)
 *
 * @param {ap_fixpt<unsigned int W_IN, int IW_IN>} num input
 * @return {ap_fixpt<unsigned int W_OUT, int IW_OUT>} square root of input value 
 * @example
 * hls::ap_fixpt<10, 2> y = 4;
 * auto x = hls::math::sqrt<10, 2, 20>(y); //x will be an ap_fixpt w/ the value 2
 */
template <unsigned int W, int IW, unsigned N_ITERATIONS, unsigned int W_IN, int IW_IN>
ap_ufixpt<W, IW> sqrt(ap_ufixpt<W_IN, IW_IN> val) {

  if (val == 0)
    return 0;

  unsigned long long intPart = val(W_IN - 1, W_IN - IW_IN);
  unsigned long long npow2 = nextPowerOf2(intPart);
  unsigned long lg2 = log2Pow<IW_IN>(npow2);
  ap_ufixpt<W_IN, IW_IN> m = fixpt_ceil<W_IN, IW_IN>(lg2 / 2.0);

  DBG_CODE{
	printf("x = %f\nintPart = %d\nnpow2 = %d\nlg2 = %d\nm = %f\n", val.to_double(), intPart, npow2, lg2, m.to_double());
  }

  // Variable "A" will be normalized < 1, therefore we're only keeping 1 bit
  // for the integer part. Most bits are set for the fractional part.
  ap_ufixpt<W_IN, 1> A;
  if (val > 1) {
    //  Increased width in tmp variable to avoid loosing information when
    // shifting. This is specially important for large numbers.
    ap_ufixpt<2 * W_IN, W_IN> tmp = val;
    A = tmp >> (2 * m);
  } else {
    A = val;
  }

  ap_ufixpt<W + 1, 1> x = 0, c = 0;
  DBG_CODE {
    std::cout << "A:" << A.to_double() << ", val:" << val.to_double()
              << ", m:" << m.to_double() << std::endl;
  }

  // #pragma HLS loop pipeline
  for (unsigned short k = 1; k <= N_ITERATIONS; k++) {
    ap_ufixpt<W + 1, 1> c0 = x >> (k - 1);
    ap_ufixpt<W + 1, 1> c1 = ap_ufixpt<W + 1, 1>(1.0) >> (2 * k);
    ap_ufixpt<W + 1, 1> x0 = ap_ufixpt<W + 1, 1>(1.0) >> k;
    if (A > c) {
      c += c0;
      x += x0;
    } else {
      c -= c0;
      x -= x0;
    }
    c += c1;
    DBG_CODE {
      std::cout << "k:" << k << "\tc:" << c.to_double()
                << "\tx:" << x.to_double() << std::endl;
    }
  }

  // Increased width temp. variable to avoid loosing information
  ap_ufixpt<2 * W, W> Qtmp = x;
  if (val > 1) {
    Qtmp <<= m;
  }
  ap_ufixpt<W, IW, hls::AP_RND> Q = Qtmp;
  return Q;
}
} // namespace math
} // namespace hls
