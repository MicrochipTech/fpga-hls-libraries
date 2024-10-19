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
#include "math/include/hls_sqrt.hpp"

#define THRESHOLD 2
#define PARSE 1
using namespace hls::math;

ap_ufixpt<L_W, L_IW> sqrt_hls_L_wrapper(ap_ufixpt<L_W, L_IW> x){
#pragma HLS function pipeline top
  return sqrt<L_W, L_IW, L_N_ITER>(x);
}

ap_ufixpt<M_W, M_IW> sqrt_hls_M_wrapper(ap_ufixpt<M_W, M_IW> x){
#pragma HLS function pipeline top
  return sqrt<M_W, M_IW, M_N_ITER>(x);
}

ap_ufixpt<S_W, S_IW> sqrt_hls_S_wrapper(ap_ufixpt<S_W, S_IW> x){
#pragma HLS function pipeline top
  return sqrt<S_W, S_IW, S_N_ITER>(x);
}

double sqrt_cmath_wrapper(double x){
#pragma HLS function top
  return sqrt(x);
}

int test(double start_at, double limit, double delta, unsigned int W, int IW, int N_ITER = -1, int graph = 0, int report = 0){
  double max_diff = 0, avg_diff = 0;
  int count = 0;
  std::string test_name = find_test_name("sqrt", W, IW, start_at, limit, N_ITER);

  FILE* fp = fopen(test_name.c_str(), "w");
  if (graph) fprintf(fp, "# PLOT ");
  if (report) fprintf(fp, "# REPORT ");
  fprintf(fp, "\n# from %f to %f at intervals of %f, using W: %d, IW: %d\n", start_at, limit, delta, W, IW);  
  fprintf(fp, "# x,expected,sqrt_hls,sqrt_hls diff\t\n");

  printf("From %f to %f at intervals of %f, using W: %d, IW: %d\n", start_at, limit, delta, W, IW);

  for (double x = start_at; x <= limit; x += delta) {
    double expect = sqrt_cmath_wrapper(x);
    double actual = 0;

    if (W == L_W && IW == L_IW){
      ap_ufixpt<L_W, L_IW> x_fixpt = x;
      actual = sqrt_hls_L_wrapper(x_fixpt).to_double();
    }
    else if (W == M_W && IW == M_IW){
      ap_ufixpt<M_W, M_IW> x_fixpt = x;
      actual = sqrt_hls_M_wrapper(x_fixpt).to_double();
    }
    else if (W == S_W && IW == S_IW){
      ap_ufixpt<S_W, S_IW> x_fixpt = x;
      actual = sqrt_hls_S_wrapper(x_fixpt).to_double();
    }    

    double diff = fabs(expect - actual);

    if (diff > max_diff)
      max_diff = diff;
    avg_diff += diff;
    count++;

    fprintf(fp, "%f,%lf,%lf,%lf\n", x, expect, actual, diff);
  }
  avg_diff /= count;

  fprintf(fp, "# sqrt_cmath: Count: %d\tMax error: 0.000000\tAvg error: 0.000000\n", count);
  fprintf(fp, "# sqrt_hls: Count: %d\tMax error: %lf\tAvg error: %lf\n", count, max_diff, avg_diff);

  printf("sqrt_cmath: Count: %d\tMax error: 0.000000\tAvg error: 0.000000\n", count);
  printf("sqrt_hls: Count: %d\tMax error: %lf\tAvg error: %lf\n\n", count, max_diff, avg_diff);

  fclose(fp);
  if (avg_diff > THRESHOLD) return 1;
  return 0;
}

int main() {
  double start;
  double limit;
  double delta;
  int RC = 0;
  create_dir("sqrt_reports");

#ifdef COSIM_EARLY_EXIT
  // ---------------------------------------------------------------------
  // This test will be used automatically when running cosimulation, as
  // SmartHLS defines COSIM_EARLY_EXIT when running cosim.
  // This test is quick, good to validate the generated verilog and see
  // the wave form
  // ---------------------------------------------------------------------
  start=0; limit = 10; delta = 0.1;
  RC |= test(start, limit, delta, M_W, M_IW, M_N_ITER, 1, 1);
											  
  start=0.0; limit = 1.0; delta = 0.01;
  RC |= test(start, limit, delta, S_W, S_IW, S_N_ITER, 1, 1); 

  start = 0.0; limit = pow(2, 32) - 1;  delta = pow(2, 32) / 4;
  RC |= test(start, limit, delta, L_W, L_IW, L_N_ITER, 1, 1); 
  RC |= test(pow(2,32)-1, pow(2,32)-1, 1, L_W, L_IW, L_N_ITER); 
  RC |= test(100.0, 100.0, 1, M_W, M_IW, M_N_ITER); 
  //test(-1, -1, 1, M_W, M_IW); //Test invalid argument

#else
  // ---------------------------------------------------------------------
  // These tests are for software run as they call the HLS module
  // thousands of times. Good to validate the algorithm and its parameters.
  // ---------------------------------------------------------------------

  // 256 test of large numbers starting from 0 with increments of 1.
  start = 0.0; limit = pow(2, 32) - 1;  delta = pow(2, 32) / 256.0;
  RC |= test(start, limit, delta, L_W, L_IW, L_N_ITER, 1, 1);

  // test small numbers < 1
  start=0.0; limit = 1.0; delta = 0.0001;
  RC |= test(start, limit, delta, S_W, S_IW, S_N_ITER, 1, 1);

  // Normal tests
  start=1.0; limit = 100; delta = 0.1;
  RC |= test(start, limit, delta, M_W, M_IW, M_N_ITER, 1, 1);

  // Single number tests

  test(4.0, 4.0, 1.0, M_W, M_IW, M_N_ITER);
  test(9.0, 9.0, 1.0, M_W, M_IW, M_N_ITER);
  test(25.0, 25.0, 1, M_W, M_IW, M_N_ITER);
  test(36.0, 36.0, 1, M_W, M_IW, M_N_ITER);
  test(64.0, 64.0, 1, M_W, M_IW, M_N_ITER);
  test(100.0, 100.0, 1, M_W, M_IW, M_N_ITER);
  test(pow(2,32)-1, pow(2,32)-1, 1, L_W, L_IW, L_N_ITER);
  test(pow(2,32)-2, pow(2,32)-2, 1, L_W, L_IW, L_N_ITER);
  test (-1, -1, 1, M_W, M_IW, M_N_ITER); //Test invalid argument

#endif

  if (RC != 0)  printf("Errors may have occurred. Please double-check usage.\n");
  return RC;
}
