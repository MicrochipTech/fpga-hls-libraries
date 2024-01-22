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
#include "math/include/hls_exp.hpp"

#define THRESHOLD 1
using namespace hls::math;

/* Wrapper Functions */
ap_fixpt<S_W, S_IW> exp_taylor_S_wrapper(ap_fixpt<S_W, S_IW>x){
#pragma HLS function pipeline top
  return exp_taylor<S_W, S_IW>(x);
}

ap_fixpt<M_W, M_IW> exp_taylor_M_wrapper(ap_fixpt<M_W, M_IW>x){
#pragma HLS function pipeline top
  return exp_taylor<M_W, M_IW>(x);
} 

ap_fixpt<M_W, M_IW> exp_cordic_S_wrapper(ap_fixpt<S_W, S_IW>x){
#pragma HLS function pipeline top
  return exp_cordic<S_W, S_IW, S_N_ITER>(x);
}

ap_fixpt<M_W, M_IW> exp_cordic_M_wrapper(ap_fixpt<M_W, M_IW>x){
#pragma HLS function pipeline top
  return exp_cordic<M_W, M_IW, M_N_ITER>(x);
}

double exp_cmath_wrapper(double x){
#pragma HLS function pipeline top
  return exp(x);
}


int test(double start_at, double limit, double delta, unsigned int W, int IW, int N_ITER = -1, int graph = 0, int report = 0) {
  double max_diff_taylor = 0, avg_diff_taylor = 0;
  double max_diff_cordic = 0, avg_diff_cordic = 0;
  int count = 0;
  std::string test_name = find_test_name("exp", W, IW, start_at, limit, N_ITER);

  FILE* fp = fopen(test_name.c_str(), "w");
  if (graph) fprintf(fp, "# PLOT ");
  if (report) fprintf(fp, "# REPORT ");
  fprintf(fp, "\n# from %f to %f at intervals of %f, using W: %d, IW: %d\n", start_at, limit, delta, W, IW);
  fprintf(fp, "# x,expected,exp_taylor,exp_taylor_diff\n");
  printf("From %f to %f at intervals of %f, using W: %d, IW: %d\n", start_at, limit, delta, W, IW);

  for (double x = start_at; x <= limit; x += delta) {
    ap_fixpt<S_W, S_IW> x_fixpt = x;

    double expect = exp_cmath_wrapper(x);
    double actual_taylor, diff_taylor, actual_cordic, diff_cordic;

    if (W == S_W && IW == S_IW){
      ap_fixpt<S_W, S_IW> x_fixpt = x;
      actual_taylor = exp_taylor_S_wrapper(x_fixpt).to_double();
      actual_cordic = exp_cordic_S_wrapper(x_fixpt).to_double();
    }
    else if (W == M_W && IW == M_IW){
      ap_fixpt<M_W, M_IW> x_fixpt = x;	    
      actual_taylor = exp_taylor_M_wrapper(x_fixpt).to_double();
      actual_cordic = exp_cordic_M_wrapper(x_fixpt).to_double();
    }
    else{ printf("Please write a custom wapper function for this bitwidth.\n"); return 1;}

    diff_taylor = fabs(expect - actual_taylor);
    diff_cordic = fabs(expect - actual_cordic);

    if (diff_cordic > max_diff_cordic)
      max_diff_cordic = diff_cordic;
    avg_diff_cordic += diff_cordic;

    if (diff_taylor > max_diff_taylor)
      max_diff_taylor = diff_taylor;
    avg_diff_taylor += diff_taylor;
    count++;

    fprintf(fp, "%f,%lf,%lf,%lf,%lf,%lf\n", x, expect, actual_taylor, actual_cordic, diff_taylor, diff_cordic); 
  }
  
  avg_diff_taylor /= count;
  avg_diff_cordic /= count;

    fprintf(fp, "# exp_cmath: Count: %d\tMax error: 0.000000\tAvg error: 0.000000\n", count);
    fprintf(fp, "# exp_taylor: Count: %d\tMax error: %lf\tAvg error: %lf\n", count, max_diff_taylor, avg_diff_taylor);
    fprintf(fp, "# exp_cordic: Count: %d\tMax error: %lf\tAvg error: %lf\n", count, max_diff_cordic, avg_diff_cordic);
  
    printf("exp_cmath: Count: %d\tMax error: 0.000000\tAvg error: 0.000000\n", count);
    printf("exp_taylor: Count: %d\tMax error: %lf\tAvg error: %lf\n", count, max_diff_taylor, avg_diff_taylor);
    printf("exp_cordic: Count: %d\tMax error: %lf\tAvg error: %lf\n\n", count, max_diff_cordic, avg_diff_cordic);

    fclose(fp);
//  if (avg_diff_cordic > THRESHOLD || avg_diff_taylor > THRESHOLD) return 1;
  return 0;
}


int main() {
  double start;
  double limit;
  double delta;
  int RC = 0;

  create_dir("exp_reports");
  // Small Test
  start = -10; limit = -2; delta = 0.01;
  RC |= test(start, limit, delta, S_W, S_IW, S_N_ITER, 1, 1); 

  // Medium Test
  start = -2; limit = 3; delta = 0.01;
  RC |= test(start, limit, delta, M_W, M_IW, M_N_ITER, 1, 1);

  // Testing special values
  RC |= test(0, 0, 1, M_W, M_IW); 

  if (RC != 0)  printf("Errors may have occurred. Please double-check usage.\n");
  return RC;
}
