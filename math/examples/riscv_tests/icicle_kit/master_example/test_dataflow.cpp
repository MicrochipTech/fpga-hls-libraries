#include "math/examples/test_utils.hpp"
#include "math/include/hls_math.hpp"
#include "hls/utils.hpp"
#include "input_vals.hpp"
//#include "hls/hls_alloc.h"

#define W 32
#define IW 16
#define N_ITER 16
using namespace hls::math;

#define N_FUNCS 27
#define THRESHOLD 0.1

void test_inner(float n, float vals[N_FUNCS]){
  ap_fixpt<W, IW> x(n);
  ap_fixpt<W, IW> base(3);

  vals[0] = sin_taylor<W, IW>(x).to_double();
  vals[1] = sin_lut<W, IW>(x).to_double();
  vals[2] = sin_cordic<W, IW, N_ITER>(x).to_double();
  
  vals[3] = cos_taylor<W, IW>(x).to_double();
  vals[4] = cos_lut<W, IW>(x).to_double();
  vals[5] = cos_cordic<W, IW, N_ITER>(x).to_double();

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

void hls_test(float fp, float vals[N_FUNCS]){
  #pragma HLS function top dataflow
  #pragma HLS interface default type(axi_target)
   test_inner(fp, vals);
}

void cmath_test(float x, float vals[N_FUNCS]){
  float num, base = 3;

  vals[0] = sin(x);
  vals[1] = sin(x);
  vals[2] = sin(x);

  vals[3] = cos(x);
  vals[4] = cos(x);
  vals[5] = cos(x);

  vals[6] = tan(x);
  vals[7] = tan(x);
  vals[8] = tan(x);

  vals[9] = sqrt(x);

  vals[10] = atan(x);
  vals[11] = atan(x);

  vals[12] = exp(x);
  vals[13] = exp(x);

  vals[14] = log(x);
  vals[15] = log(x);

  vals[16] = log(x)/log(base);
  vals[17] = pow(base, x);

  vals[18] = ceil(x);
  vals[19] = floor(x);
  vals[20] = round(x);
  vals[21] = fabs(x);
  vals[22] = trunc(x);

  vals[23] = asin(x);
  vals[24] = acos(x);

  vals[25] = log2(x);
  vals[26] = log2(x);

}

int main(){
  float vals[N_FUNCS];
  float vals_fixpt[N_FUNCS];
  float x;
  
  double cmath0 = timestamp();
  for (int i = 0; i < 90; i++){
    x = input_vals[i];
    cmath_test(x, vals);
  }
  double cmath1 = timestamp();

  double hls_math0 = timestamp();
  for (int i = 0; i < 90 ; i++){
    x = input_vals[i];
    hls_test(x, vals_fixpt);
  }
  double hls_math1 = timestamp();

  float diff, actual;
  int i, failing = 0;

  for (i = 0; i < N_FUNCS; i++){
    actual = vals_fixpt[i];
    diff = fabs(vals[i] - actual);

    if (diff > THRESHOLD){
      failing ++;
      printf("Test number %d is failing, please double check!\n", i);
    }

    printf("i: %d\texpected: %f\tactual: %f\t diff: %f\n", i, vals[i], actual, diff);
  }

  if (failing == 0) printf("PASS\n");
  else printf("FAIL\n");

  printf("cmath: %lf s, hls_math: %lf s\n", (cmath1 - cmath0), (hls_math1 - hls_math0));
  return failing;
}
