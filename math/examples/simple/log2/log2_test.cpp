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
#include "math/include/hls_log2.hpp"

#define THRESHOLD 1
using namespace hls::math;

template <unsigned int W, int IW>
struct log2_struct {
        ap_fixpt<W, IW> log2;
        int error;
};

// The wrapper must return a struct with the log2 value & the error value, since we can't pass the error by reference: there is a known issue with SHLS cosim, in which
// cosim doesn't support simulating pipelined functions with memory interfaces (e.g. values passed by reference)
log2_struct<M_W, M_IW> log2_lut_M_wrapper(ap_ufixpt<M_W, M_IW>x, int error){
#pragma HLS function pipeline top
  ap_fixpt<M_W, M_IW> temp = log2_lut<M_W, M_IW>(x, error);
  log2_struct<M_W, M_IW> r = {temp, error};
  return (r);
}

log2_struct<S_W, S_IW> log2_lut_S_wrapper(ap_ufixpt<S_W, S_IW> x, int error){
#pragma HLS function pipeline top
  ap_fixpt<S_W, S_IW> temp = log2_lut<S_W, S_IW>(x, error);
  log2_struct<S_W, S_IW> r = {temp, error};
  return (r);	
}

log2_struct<M_W, M_IW> log2_cordic_M_wrapper(ap_ufixpt<M_W, M_IW>x, int error){
#pragma HLS function pipeline top
  ap_fixpt<M_W, M_IW> temp = log2_cordic<M_W, M_IW, M_N_ITER>(x, error);
  log2_struct<M_W, M_IW> r = {temp, error};
  return (r);
}

log2_struct<S_W, S_IW> log2_cordic_S_wrapper(ap_ufixpt<S_W, S_IW> x, int error){
#pragma HLS function pipeline top
  ap_fixpt<S_W, S_IW> temp = log2_cordic<S_W, S_IW, S_N_ITER>(x, error);
  log2_struct<S_W, S_IW> r = {temp, error};
  return (r);
}

double log2_cmath_wrapper(double x){
#pragma HLS function pipeline top
  return log2(x);
}

int test(double start_at, double limit, double delta, unsigned int W, int IW, int N_ITER = -1, int graph = 0, int report = 0){
  double actual_lut = 0, actual_cordic = 0;
  double  max_diff_cordic = 0, avg_diff_cordic = 0, max_diff_lut = 0, avg_diff_lut = 0, diff_cordic = 0, diff_lut = 0;
  int count = 0, error = 0;
  std::string test_name = find_test_name("log2", W, IW, start_at, limit);

  FILE* fp = fopen(test_name.c_str(), "w");
  if (graph) fprintf(fp, "# PLOT ");
  if (report) fprintf(fp, "# REPORT ");
  fprintf(fp, "\n# from %f to %f at intervals of %f, using W: %d, IW: %d\n", start_at, limit, delta, W, IW);  
  fprintf(fp, "# x,expected,log2_lut,log2_cordic,log2_lut diff,log2_cordic diff\n");

  printf("From %f to %f at intervals of %f, using W: %d, IW: %d\n", start_at, limit, delta, W, IW);

  for (double x = start_at; x <= limit; x += delta) {
    double expect = log2_cmath_wrapper(x);

    if (W == M_W && IW == M_IW){
      ap_ufixpt<M_W, M_IW> x_fixpt = x;
      actual_cordic = log2_cordic_M_wrapper(x_fixpt, error).log2.to_double();
      actual_lut = log2_lut_M_wrapper(x_fixpt, error).log2.to_double();
    }
    else if (W == S_W && IW == S_IW){
      ap_ufixpt<S_W, S_IW> x_fixpt = x;
      actual_cordic = log2_cordic_S_wrapper(x_fixpt, error).log2.to_double();
      actual_lut = log2_lut_S_wrapper(x_fixpt, error).log2.to_double();
    }

    diff_cordic = fabs(expect - actual_cordic);
    if (diff_cordic > max_diff_cordic)
      max_diff_cordic = diff_cordic;
    avg_diff_cordic += diff_cordic;

    diff_lut = fabs(expect - actual_lut);
    if (diff_lut > max_diff_lut)
      max_diff_lut = diff_lut;
    avg_diff_lut += diff_lut;

    count++;

    fprintf(fp, "%f,%lf,%lf,%lf,%lf,%lf\n", x, expect, actual_lut, actual_cordic, diff_lut, diff_cordic);
  }
  avg_diff_cordic /= count;
  avg_diff_lut /= count;

  fprintf(fp, "# log2_cmath: Count: %d\tMax error: 0.000000\tAvg error: 0.000000\n", count);
  fprintf(fp, "# log2_lut: Count: %d\tMax error: %lf\tAvg error: %lf\n", count, max_diff_lut, avg_diff_lut);
  fprintf(fp, "# log2_cordic: Count: %d\tMax error: %lf\tAvg error: %lf\n", count, max_diff_cordic, avg_diff_cordic);

  printf("log2_cmath: Count: %d\tMax error: 0.000000\tAvg error: 0.000000\n", count);
  printf("log2_lut: Count: %d\tMax error: %lf\tAvg error: %lf\n", count, max_diff_lut, avg_diff_lut);
  printf("log2_cordic: Count: %d\tMax error: %lf\tAvg error: %lf\n\n", count, max_diff_cordic, avg_diff_cordic);

  fclose(fp);
//  if (avg_diff > 1) return THRESHOLD;
  return 0;
}


int main() {
  double start;
  double limit;
  double delta;
  int RC = 0;

  create_dir("log2_reports");

  // Test invalid
  RC |= test(0, 0, 1, M_W, M_IW);
  RC |= test(-1, -1, 1, M_W, M_IW);

  // Regular Tests

  start = 0.01;  limit = 1;  delta = 0.01;
  RC |= test(start, limit, delta, S_W, S_IW, -1, 1, 1); 
  start = 1;  limit = 100;  delta = 1;
  RC |= test(start, limit, delta, M_W, M_IW, -1, 1, 1);

  if (RC != 0)  printf("Errors may have occurred. Please double-check usage.\n");
  return RC;
}
