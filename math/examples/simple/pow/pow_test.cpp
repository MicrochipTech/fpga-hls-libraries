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

#include "math/examples/test_utils.hpp"
#include "math/examples/simple/configs.hpp"
#include "math/include/hls_pow.hpp"

#define THRESHOLD 1
using namespace hls::math;

template <unsigned int W, int IW>
struct pow_struct {
        ap_fixpt<W, IW> pow;
        int error;
};

pow_struct<S_W, S_IW> pow_hls_S_wrapper(ap_fixpt<S_W, S_IW> b, ap_fixpt<S_W, S_IW> x, int error){
#pragma HLS function pipeline top
  ap_fixpt<S_W, S_IW> temp = pow<S_W, S_IW>(b, x, error);
  pow_struct<S_W, S_IW> r = {temp, error};
  return (r);
}

pow_struct<M_W, M_IW> pow_hls_M_wrapper(ap_fixpt<M_W, M_IW> b, ap_fixpt<M_W, M_IW> x, int error){
#pragma HLS function pipeline top
  ap_fixpt<M_W, M_IW> temp = pow<M_W, M_IW>(b, x, error);
  pow_struct<M_W, M_IW> r = {temp, error};
  return (r);	
}

pow_struct<XL_W, XL_IW> pow_hls_XL_wrapper(ap_fixpt<XL_W, XL_IW> b, ap_fixpt<XL_W, XL_IW> x, int error){
#pragma HLS function pipeline top
  ap_fixpt<XL_W, XL_IW> temp = pow<XL_W, XL_IW>(b, x, error);
  pow_struct<XL_W, XL_IW> r = {temp, error};
  return (r);	
}

double pow_cmath_wrapper(double b, double x){
#pragma HLS function pipeline top
  return pow(b, x);
}

int test(double start_at, double limit, double delta, double base, unsigned int W, int IW, int N_ITER = -1, int graph = 0, int report = 0) {
  double max_diff = 0, avg_diff = 0;
  int count = 0, error = 0;
  std::string test_name = find_test_name("pow", W, IW, start_at, limit, N_ITER, base);

  FILE* fp = fopen(test_name.c_str(), "w");
  if (graph) fprintf(fp, "# PLOT ");
  if (report) fprintf(fp, "# REPORT ");
  fprintf(fp, "\n# from %f to %f at intervals of %f, using W: %d, IW: %d\n", start_at, limit, delta, W, IW);  
  fprintf(fp, "# x,expected,pow_hls,pow_hls diff\n");

  printf("From %f to %f at intervals of %f, using base %f and W: %d, IW: %d\n", start_at, limit, delta, base, W, IW);

  for (double x = start_at; x <= limit; x += delta) {
    double expect = pow_cmath_wrapper(base, x);
    double actual = 0;

    if (W == XL_W && IW == XL_IW){
      ap_fixpt<XL_W, XL_IW> x_fixpt = x;
      ap_fixpt<XL_W, XL_IW> b_fixpt = base;
      actual = pow_hls_XL_wrapper(b_fixpt, x_fixpt, error).pow.to_double();
    }
    else if (W == M_W && IW == M_IW){
      ap_fixpt<M_W, M_IW> x_fixpt = x;
      ap_fixpt<M_W, M_IW> b_fixpt = base;
      actual = pow_hls_M_wrapper(b_fixpt, x_fixpt, error).pow.to_double();
    }
    else if (W == S_W && IW == S_IW){
      ap_fixpt<S_W, S_IW> x_fixpt = x;
      ap_fixpt<S_W, S_IW> b_fixpt = base;
      actual = pow_hls_S_wrapper(b_fixpt, x_fixpt, error).pow.to_double();
    }

    double diff = fabs(expect - actual);
    if (diff > max_diff)
      max_diff = diff;
    avg_diff += diff;

    count++;

//    if (actual.error != 0) return actual.error;
    fprintf(fp, "%f,%lf,%lf,%lf\n", x, expect, actual, diff);
  }
  avg_diff /= count;

  fprintf(fp, "# pow_cmath: Count: %d\tMax error: 0.000000\tAvg error: 0.000000\n", count);
  fprintf(fp, "# pow_hls: Count: %d\tMax error: %lf\tAvg error: %lf\n", count, max_diff, avg_diff);

  printf("pow_cmath: Count: %d\tMax error: 0.000000\tAvg error: 0.000000\n", count);
  printf("pow_hls: Count: %d\tMax error: %lf\tAvg error: %lf\n\n", count, max_diff, avg_diff);

  fclose(fp);
  if (avg_diff > THRESHOLD) return 1;
  return 0;
}

int main() {
  double start;
  double limit;
  double delta;
  double base;
  int RC = 0;

  create_dir("pow_reports");
  // Special cases:
  RC |= test(0, 0, 1, 3, M_W, M_IW);

  // Negative base cases:
  start = -3;  limit = 3;  delta = 1;  base = -3;
  RC |= test(start, limit, delta, base, M_W, M_IW, -1, 1, 1); 

  // Invalid Argument Error
  test(0.5, 0.5, 1, -3, M_W, M_IW);

  // Small Cases
  start = -10;  limit = -2;  delta = 0.5;  base = 2;
  RC |= test(start, limit, delta, base, S_W, S_IW, -1, 1, 1); 

  // Medium Cases
  start = -2;  limit = 3;  delta = 0.01;  base = 3;
  RC |= test(start, limit, delta, base, M_W, M_IW, -1, 1, 1); 
  start = -2;  limit = 3;  delta = 0.01;  base = 1.4;
  RC |= test(start, limit, delta, base, M_W, M_IW, -1, 1, 1); 
  // Large Cases
  start = 15;  limit = 20;  delta = 1;  base = 2;
  RC |= test(start, limit, delta, base, XL_W, XL_IW, -1, 1, 1); 

  if (RC != 0)  printf("Errors may have occurred. Please double-check usage.\n");
  return RC;
}

