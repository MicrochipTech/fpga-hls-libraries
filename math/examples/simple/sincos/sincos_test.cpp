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
#include "math/include/hls_sincos.hpp"


#define THRESHOLD 1
using namespace hls::math;

/* This is necessary because cosim doesn't allow returning values by reference. To get around this,
 * we simply just use a struct to return sin & cos values. */

struct sincos_struct_M {
	ap_fixpt<M_W, M_IW> sin;
	ap_fixpt<M_W, M_IW> cos;
} typedef sincos_struct_M;	


// The wrapper must return a struct with sin/cos values. There is a known issue with SHLS cosim, in which
// cosim doesn't support simulating pipelined functions with memory interfaces (e.g. values passed by reference)
sincos_struct_M sincos_cordic_M_wrapper(ap_fixpt<M_W, M_IW> x, ap_fixpt<M_W, M_IW> s, ap_fixpt<M_W, M_IW> c){
#pragma HLS function pipeline top
  sincos<M_N_ITER>(x, s, c);
  sincos_struct_M r = {s, c};
  return (r);
}

void sincos_cmath_wrapper(double x, double* s, double* c){
#pragma HLS interface argument(s) type(memory) num_elements(1)	
#pragma HLS interface argument(c) type(memory) num_elements(1)	
#pragma HLS function top
  sincos(x, s, c);
}

int test(double start_at, double limit, double delta, unsigned int W, int IW, int N_ITER = -1, int graph = 0, int report = 0) {
  double max_diff_sin = 0, avg_diff_sin = 0, max_diff_cos = 0, avg_diff_cos = 0;
  double diff_cos = 0, diff_sin = 0;
  ap_fixpt<M_W, M_IW> actual_sin = 0, actual_cos=0;
  int count = 0;
  std::string test_name = find_test_name("sincos", W, IW, start_at, limit, N_ITER);

  FILE* fp = fopen(test_name.c_str(), "w");
  if (graph) fprintf(fp, "# PLOT ");
  if (report) fprintf(fp, "# REPORT ");
  fprintf(fp, "\n# from %f to %f at intervals of %f, using W: %d, IW: %d\n", start_at, limit, delta, W, IW);
  fprintf(fp, "# x,expected sin,expected cos,actual_sin,actual_sin,diff sin,diff cos\n");

  printf("From %f to %f at intervals of %f, using W: %d, IW: %d\n", start_at, limit, delta, W, IW);

  for (double x = start_at; x <= limit; x += delta) {
    ap_fixpt<M_W, M_IW> x_fixpt = x;

    double* expect_sin = (double*)malloc(sizeof(double));
    double* expect_cos = (double*)malloc(sizeof(double));
    sincos_cmath_wrapper(x, expect_sin, expect_cos);
    sincos_struct_M r = sincos_cordic_M_wrapper(x_fixpt, actual_sin, actual_cos);
    
    diff_cos = fabs(*expect_cos - r.cos.to_double());
    diff_sin = fabs(*expect_sin - r.sin.to_double());

    if (diff_sin > max_diff_sin) max_diff_sin = diff_sin;
    if (diff_cos > max_diff_cos) max_diff_cos = diff_cos;

    avg_diff_cos += diff_cos;
    avg_diff_sin += diff_sin;

    count++;

    fprintf(fp, "%f,%lf,%lf,%lf,%lf,%lf,%lf\n", x, *expect_sin, *expect_cos, r.sin.to_double(),r.cos.to_double(), diff_sin, diff_cos);
  }
  avg_diff_cos /= count;
  avg_diff_sin /= count;

  double avg_diff = (avg_diff_sin + avg_diff_cos)/2;
  double max_diff = (max_diff_sin > max_diff_cos) ? max_diff_sin : max_diff_cos;

  fprintf(fp, "# sincos_cmath: Count: %d\tMax error: 0.000000\tAvg error: 0.000000\n", count);
  fprintf(fp, "# sincos_cordic: Count: %d\tMax error: %lf\tAvg error: %lf\n", count, max_diff, avg_diff);

  printf("sincos_cmath: Count: %d\tMax error: 0.000000\tAvg error: 0.000000\n", count);
  printf("sincos_cordic: Count: %d\tMax error: %lf\tAvg error: %lf\n", count, max_diff, avg_diff);
  fclose(fp);

//  if (avg_diff > THRESHOLD )
//    return 1;
//  else
    return 0;
}

int main() {

  create_dir("sincos_reports");

  double start;
  double limit;
  double delta;
  int RC = 0;

  // Medium Test
  start = -4 * M_PI; limit = 4 * M_PI;  delta = (0.1);
  RC |= test(start, limit, delta, M_W, M_IW, M_N_ITER, 0, 1); 

  if (RC != 0)  printf("Errors may have occurred. Please double-check usage.\n");
  return RC;
}

