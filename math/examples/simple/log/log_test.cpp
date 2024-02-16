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
#include "math/include/hls_log.hpp"

#define THRESHOLD 1
using namespace hls::math;

/* This is necessary because cosim doesn't allow returning values by reference. */

struct log_struct_S {
        ap_fixpt<S_W, S_IW> log;
        int error;
};

struct log_struct_M {
        ap_fixpt<M_W, M_IW> log;
        int error;
};

log_struct_M log_hls_M_wrapper(ap_ufixpt<M_W, M_IW>x, ap_ufixpt<M_W, M_IW>b, int error){
#pragma HLS function pipeline top
  ap_fixpt<M_W, M_IW> temp = log<M_W, M_IW>(x, b, error);
  log_struct_M r = {temp, error};
  return (r);
}

log_struct_S log_hls_S_wrapper(ap_ufixpt<S_W, S_IW>x, ap_ufixpt<S_W, S_IW>b, int error){
#pragma HLS function pipeline top
  ap_fixpt<S_W, S_IW> temp = log<S_W, S_IW>(x, b, error);
  log_struct_S r = {temp, error};
  return (r);
}

double log_cmath_wrapper(double x, double b, int& error){
#pragma HLS function pipeline top
  return log(x)/log(b);
}

int test(double start_at, double limit, double delta, double base, unsigned int W, int IW, int N_ITER = -1, int graph = 0, int report = 0) {
  double max_diff = 0, avg_diff = 0, diff;
  int count = 0, error = 0;
  std::string test_name = find_test_name("log", W, IW, start_at, limit, N_ITER, base);

  FILE* fp = fopen(test_name.c_str(), "w");
  if (graph) fprintf(fp, "# PLOT ");
  if (report) fprintf(fp, "# REPORT ");
  fprintf(fp, "\n# from %f to %f at intervals of %f, using W: %d, IW: %d\n", start_at, limit, delta, W, IW);  
  fprintf(fp, "# x,expected,log_hls,log_hls_diff\n");

  printf("From %f to %f at intervals of %f, using W: %d, IW: %d\n", start_at, limit, delta, W, IW);

  for (double x = start_at; x <= limit; x += delta) {
    double expect = log_cmath_wrapper(x, base, error);
    double actual = 0;

    if (W == M_W && IW == M_IW){
      ap_ufixpt<M_W, M_IW> x_fixpt = x;
      ap_ufixpt<M_W, M_IW> b_fixpt = base;
      actual = log_hls_M_wrapper(x_fixpt, b_fixpt, error).log.to_double();
    }
    else if (W == S_W && IW == S_IW){
      ap_ufixpt<S_W, S_IW> x_fixpt = x;
      ap_ufixpt<S_W, S_IW> b_fixpt = base;
      actual = log_hls_S_wrapper(x_fixpt, b_fixpt, error).log.to_double();	    
    }
    double diff = fabs(expect - actual);
      max_diff = diff;
    avg_diff += diff;

    count++;

    fprintf(fp, "%f,%lf,%lf,%lf\n", x, expect, actual, diff);
  }

  avg_diff /= count;

  fprintf(fp, "# log_cmath: Count: %d\tMax error: 0.000000\tAvg error: 0.000000\n", count);
  fprintf(fp, "# log_hls: Count: %d\tMax error: %lf\tAvg error: %lf\n", count, max_diff, avg_diff);

  printf("log_cmath: Count: %d\tMax error: 0.000000\tAvg error: 0.000000\n", count);
  printf("log_hls: Count: %d\tMax error: %lf\tAvg error: %lf\n\n", count, max_diff, avg_diff);

  fclose(fp);
//  if (avg_diff > THRESHOLD) return 1;
  return 0;
}


int main() {
  double start;
  double limit;
  double delta;
  double base;
  int RC = 0;

  create_dir("log_reports");

  // Testing invalid arguments
  test(0, 0, 1, 3, S_W, S_IW);
  test(-1, -1, 1, 3, S_W, S_IW);
  test(1, 1, 1, -3, S_W, S_IW);
  test(1, 1, 1, 0, S_W, S_IW);

  // Small Test
  start = 0.01; limit = 1;  delta = 0.01; base = 2;
  RC |= test(start, limit, delta, base, S_W, S_IW, -1, 1, 1); 

  // Regular Tests
  start = 1; limit = 64;  delta = 0.1; base = 2;
  RC |= test(start, limit, delta, base, M_W, M_IW, -1, 1, 1); 

  start = 1; limit = 64;  delta = 0.1; base = 3;
  RC |= test(start, limit, delta, base, M_W, M_IW, -1, 1, 1);

  if (RC != 0)  printf("Errors may have occurred. Please double-check usage.\n");
  return RC;
}
