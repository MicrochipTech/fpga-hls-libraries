// ©2022 Microchip Technology Inc. and its subsidiaries
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

#include <stdio.h>
#include "math/include/hls_math.hpp"

#define N_FUNCS 27
#define W 32
#define IW 16
#define N_ITER 16

using namespace hls::math;
using namespace hls;


void test_inner(float fp, float vals[N_FUNCS]) {
   hls::ap_fixpt<W, IW> x = fp;
   hls::ap_fixpt<W, IW> base = 3;

   vals[0] = hls::math::sin_lut<W, IW>(x).to_double(); 
   vals[1] = hls::math::sin_taylor<W, IW>(x).to_double(); 
   vals[2] = hls::math::sin_cordic<W, IW, N_ITER>(x).to_double();

   vals[3] = hls::math::cos_lut<W, IW>(x).to_double();
   vals[4] = hls::math::cos_taylor<W, IW>(x).to_double();
   vals[5] = hls::math::cos_cordic<W, IW, N_ITER>(x).to_double();

  vals[6] = tan_taylor<W, IW>(x).to_double();
  vals[7] = tan_lut<W, IW>(x).to_double();
  vals[8] = tan_cordic<W, IW, N_ITER>(x).to_double();

  vals[9] = sqrt<W, IW, N_ITER>((ap_ufixpt<W, IW>)x).to_double();

  vals[10] = atan_rational<W, IW>(x).to_double();
  vals[11] = atan_cordic<W, IW, N_ITER>(x).to_double();

  vals[12] = exp_taylor<W, IW>(x).to_double();
  vals[13] = exp_cordic<W, IW, N_ITER>(x).to_double();

  vals[14] = ln_lut<W, IW>((ap_ufixpt<W, IW>)x).to_double();
  vals[15] = ln_cordic<W, IW, N_ITER>((ap_ufixpt<W, IW>)x).to_double();

  vals[16] = log<W, IW>((ap_ufixpt<W, IW>)x, (ap_ufixpt<W, IW>)base).to_double();
  vals[17] = pow<W, IW>(base, x).to_double();

  vals[18] = ceil<W, IW>(x).to_double();
  vals[19] = floor<W, IW>(x).to_double();
  vals[20] = round<W, IW>(x).to_double();
  vals[21] = abs<W, IW>(x).to_double();
  vals[22] = trunc<W, IW>(x).to_double();

  vals[23] = asin_cordic<W, IW, N_ITER>(x).to_double();
  vals[24] = acos_cordic<W, IW, N_ITER>(x).to_double();

  vals[25] = log2_lut<W, IW>((ap_ufixpt<W, IW>)x).to_double();
  vals[26] = log2_cordic<W, IW, N_ITER>((ap_ufixpt<W, IW>)x).to_double();

}

void test(float fp, float vals[N_FUNCS]){
  #pragma HLS function top dataflow
  #pragma HLS interface default type(axi_target)
   test_inner(fp, vals);
}


int main() {
  float y = M_PI_4;
  float x[N_FUNCS];
  test(y, x);
  for (int i = 0; i < N_FUNCS; i++){
        printf("x[%d]=%f\r\n",i, x[i]);
  }

  return 0;
}
