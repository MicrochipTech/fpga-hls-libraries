#include "math/examples/test_utils.hpp"
#include "math/include/hls_math.hpp"
#include "hls/utils.hpp"
#include "hls/streaming.hpp"
#include "hls/hls_alloc.h"

#define W 32
#define IW 16
#define N_ITER 16
using namespace hls::math;

#define N 64000
#define THRESHOLD 0.1

void hls_test(float n[N], float vals[N]){
#pragma HLS function top
#pragma HLS interface default type(axi_target)
#pragma HLS interface argument(vals) type(axi_target) num_elements(N) dma(false)
#pragma HLS interface argument(n) type(axi_target) num_elements(N) dma(false)

ap_fixpt<W, IW> x_temp[N];
#pragma HLS loop pipeline
  for (int i = 0; i < N; i++){
    x_temp[i] = n[i];
  }

ap_fixpt<W, IW> temp[N];
#pragma HLS loop pipeline
  for (int i = 0; i < N; i++){
    vals[i] = sin_lut<W, IW>(x_temp[i]).to_double();
  }
}	

void cmath_test(float n[N], float vals[N]){
  for (int i = 0; i < N; i++){
    float x = n[i];
    vals[i] = sin(x);
  }
}


int main(){
  float* vals = (float*)hls_malloc(sizeof(float) * N);
  float* vals_fixpt  = (float*)hls_malloc(sizeof(float) * N);

  float* x  = (float*)hls_malloc(sizeof(float) * N);
  for (int i = 0; i < N; i++){
    x[i] = i/1000;
  }

  double cmath0 = timestamp();
  cmath_test(x, vals);
  double cmath1 = timestamp();

  double hls_math0 = timestamp();
  hls_test(x, vals_fixpt);
  double hls_math1 = timestamp();

 for (int i = 0; i < N; i++){
    if (fabs(vals[i] - vals_fixpt[i]) > THRESHOLD){
      printf("failed: x: %f, expected: %f, actual: %f\r\n", x[i], vals[i], vals_fixpt[i]);
      return 1;
    }  
  }

  printf("Passed! Times: cmath: %lf s, hls_math: %lf s\n", (cmath1 - cmath0), (hls_math1 - hls_math0));
  hls_free(vals_fixpt);
  hls_free(vals);
  hls_free(x);
  return 0;
}
