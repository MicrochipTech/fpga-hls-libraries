//  ©2026 Microchip Technology Inc. and its subsidiaries
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

   vals[0] = (double)sin_lut<W, IW>(x); 
   vals[1] = (double)sin_taylor<W, IW>(x); 
   vals[2] = (double)sin_cordic<W, IW, N_ITER>(x);

   vals[3] = (double)cos_lut<W, IW>(x);
   vals[4] = (double)cos_taylor<W, IW>(x);
   vals[5] = (double)cos_cordic<W, IW, N_ITER>(x);

  vals[6] = (double)tan_taylor<W, IW>(x);
  vals[7] = (double)tan_lut<W, IW>(x);
  vals[8] = (double)tan_cordic<W, IW, N_ITER>(x);

  vals[9] = (double)sqrt<W, IW, N_ITER>((ap_ufixpt<W, IW>)x);

  vals[10] = (double)atan_rational<W, IW>(x);
  vals[11] = (double)atan_cordic<W, IW, N_ITER>(x);

  vals[12] = (double)exp_taylor<W, IW>(x);
  vals[13] = (double)exp_cordic<W, IW, N_ITER>(x);

  vals[14] = (double)ln_lut<W, IW>((ap_ufixpt<W, IW>)x);
  vals[15] = (double)ln_cordic<W, IW, N_ITER>((ap_ufixpt<W, IW>)x);

  vals[16] = (double)log<W, IW>((ap_ufixpt<W, IW>)x, (ap_ufixpt<W, IW>)base);
  vals[17] = (double)pow<W, IW>(base, x);

  vals[18] = (double)ceil<W, IW>(x);
  vals[19] = (double)floor<W, IW>(x);
  vals[20] = (double)round<W, IW>(x);
  vals[21] = (double)abs<W, IW>(x);
  vals[22] = (double)trunc<W, IW>(x);

  vals[23] = (double)asin_cordic<W, IW, N_ITER>(x);
  vals[24] = (double)acos_cordic<W, IW, N_ITER>(x);

  vals[25] = (double)log2_lut<W, IW>((ap_ufixpt<W, IW>)x);
  vals[26] = (double)log2_cordic<W, IW, N_ITER>((ap_ufixpt<W, IW>)x);

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
