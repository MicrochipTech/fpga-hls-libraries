#include "examples/test_utils.hpp"
#include "include/hls_math.hpp"
#include "hls/utils.hpp"
#include "hls/streaming.hpp"
#include "hls/hls_alloc.h"

#define W 32
#define IW 16
#define N_ITER 16
using namespace hls::math;

#define THRESHOLD 0.1

void hls_test(float n[__N_ELEM], float vals[__N_ELEM]){
#pragma HLS function top
#pragma HLS interface default type(axi_target)
#ifdef AXI_TARGET
#pragma HLS interface argument(vals) type(axi_target) num_elements(__N_ELEM) dma(__USE_DMA)
#pragma HLS interface argument(n) type(axi_target) num_elements(__N_ELEM) dma(__USE_DMA)
#else
#pragma HLS interface argument(vals) type(axi_initiator) num_elements(__N_ELEM)
#pragma HLS interface argument(n) type(axi_initiator) num_elements(__N_ELEM) 
#endif

ap_fixpt<W, IW> x_temp[__N_ELEM];
#pragma HLS loop pipeline
  for (int i = 0; i < __N_ELEM; i++){
    x_temp[i] = n[i];
  }
 
ap_fixpt<W, IW> temp[__N_ELEM];
#pragma HLS loop pipeline
  for (int i = 0; i < __N_ELEM; i++){
    vals[i] = sin_lut<W, IW>(x_temp[i]).to_double();
  }
}


void cmath_test(float n[__N_ELEM], float vals[__N_ELEM]){
  for (int i = 0; i < __N_ELEM; i++){
    float x = n[i];
    vals[i] = sin(x);
  }
}


int main(){
  float* vals = (float*)hls_malloc(sizeof(float) * __N_ELEM, __CACHED);
  float* vals_fixpt  = (float*)hls_malloc(sizeof(float) * __N_ELEM, __CACHED);

  float* x  = (float*)hls_malloc(sizeof(float) * __N_ELEM, __CACHED);
  for (int i = 0; i < __N_ELEM; i++){
    x[i] = i/1000;
  }

  double cmath0 = timestamp();
  cmath_test(x, vals);
  double cmath1 = timestamp();

  double hls_math0 = timestamp();
  hls_test(x, vals_fixpt);
  double hls_math1 = timestamp();

 for (int i = 0; i < __N_ELEM; i++){
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
