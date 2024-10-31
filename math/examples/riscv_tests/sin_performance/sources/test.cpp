#include "math/examples/test_utils.hpp"
#include "math/include/hls_math.hpp"
#include "hls/utils.hpp"
#include "hls/streaming.hpp"
#include "hls/hls_alloc.h"

#define W 32
#define IW 16
using namespace hls::math;

#define THRESHOLD 0.1 // Error Threshold

void hls_test(float n[__N_ELEM], float vals[__N_ELEM]){
  #pragma HLS function top dataflow
  #pragma HLS interface default type(axi_target)
  #pragma HLS interface argument(vals) type(axi_initiator) num_elements(__N_ELEM)
  #pragma HLS interface argument(n) type(axi_initiator) num_elements(__N_ELEM) 

  #ifndef __SYNTHESIS__
  // When compiling for software we need deep FIFOs because the loops are not 
  // executing in parallel (i.e. no dataflow in software) and it would be needed
  // to store all the vector completely. 
  hls::FIFO<ap_fixpt<W, IW> > ff0(__N_ELEM), ff1(__N_ELEM);
  #else
  hls::FIFO<ap_fixpt<W, IW> > ff0, ff1;
  #endif
  
  #pragma HLS loop pipeline
  for(int i=0; i<__N_ELEM; i++)
    ff0.write(ap_fixpt<W, IW>(n[i]));

  #pragma HLS loop pipeline
  for(int i=0; i<__N_ELEM; i++)
    ff1.write(sin_lut<W, IW>(ff0.read()));

  #pragma HLS loop pipeline
  for(int i=0; i<__N_ELEM; i++)
    vals[i] = ff1.read().to_double();
}

void cmath_test(float n[__N_ELEM], float vals[__N_ELEM]){
  for (int i = 0; i < __N_ELEM; i++){
    float x = n[i];
    vals[i] = sin(x);
  }
}


int main(){
  float* vals = (float*)hls_malloc(sizeof(float) * __N_ELEM, HLS_ALLOC_NONCACHED);
  float* vals_fixpt  = (float*)hls_malloc(sizeof(float) * __N_ELEM, HLS_ALLOC_NONCACHED);

  float* x  = (float*)hls_malloc(sizeof(float) * __N_ELEM, HLS_ALLOC_NONCACHED);
  for (int i = 0; i < __N_ELEM; i++){
    x[i] = i/1000;
  }

  double cmath0 = timestamp();
  cmath_test(x, vals);
  double cmath1 = timestamp();

  double hls_math0 = timestamp();
  hls_test(x, vals_fixpt);
  double hls_math1 = timestamp();

  // Test accuracy of results:
 for (int i = 0; i < __N_ELEM; i++){
    if (fabs(vals[i] - vals_fixpt[i]) > THRESHOLD){
      printf("failed: x: %f, expected: %f, actual: %f\r\n", x[i], vals[i], vals_fixpt[i]);
      return 1;
    }  
  }
  hls_free(vals_fixpt);
  hls_free(vals);
  hls_free(x);
  
  float cmath_diff = cmath1 - cmath0;
  float hls_math_diff = hls_math1 - hls_math0;

  printf("Times: cmath: %lf s, hls_math: %lf s\n", cmath_diff, hls_math_diff);

  // #ifndef COSIM_EARLY_EXIT
  //   if (cmath_diff < hls_math_diff) {
  //       printf("failed: cmath was faster than hls_math...? ");
  //       return 1;
  //   }
  // #endif

  printf("Passed!");
  return 0;
}
