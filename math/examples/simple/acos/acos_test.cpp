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
#include "math/include/hls_acos.hpp"
#include "math/examples/simple/configs.hpp"

#define THRESHOLD 1
using namespace hls::math;

template <unsigned int W, int IW>
struct acos_struct {
        ap_fixpt<W, IW> acos;
        int error;
};

/* Wrappers */
acos_struct<S_W, S_IW> acos_cordic_S_wrapper(ap_fixpt<S_W, S_IW>x, int error){
#pragma HLS function pipeline top
  ap_fixpt<S_W, S_IW> temp = acos_cordic<S_W, S_IW, S_N_ITER>(x, error);
  acos_struct<S_W, S_IW> r = {temp, error};
  return (r);
}
double acos_cmath_wrapper(double x){
#pragma HLS function top
  return acos(x);
}


int test(double start_at, double limit, double delta, unsigned int W, int IW, int N_ITER = -1, int graph = 0, int report = 0){
  double max_diff = 0, avg_diff = 0, diff;
  int count = 0, error = 0;
  std::string test_name = find_test_name("acos", W, IW, start_at, limit, N_ITER);

  FILE* fp = fopen(test_name.c_str(), "w");
  if (graph) fprintf(fp, "# PLOT ");
  if (report)  fprintf(fp, "# REPORT ");
  fprintf(fp, "\n# from %f to %f at intervals of %f, using W: %d, IW: %d\n", start_at, limit, delta, W, IW);
  fprintf(fp, "# x,expected,acos_cordic,acos_cordic diff\n");

  printf("From %f to %f at intervals of %f, using W: %d, IW: %d\n", start_at, limit, delta, W, IW);

  for (double x = start_at; x <= limit; x += delta) {
    ap_fixpt<S_W, S_IW> x_fixpt = x;

    double actual, expect;
    expect = acos_cmath_wrapper(x);

    actual = acos_cordic_S_wrapper(x_fixpt, error).acos.to_double();
    diff = fabs(expect - actual);
    if (diff > max_diff) max_diff = diff;
    avg_diff += diff;

    count++;

//    if (actual.error != 0) return actual.error;
    fprintf(fp, "%f,%lf,%lf,%lf\n", x, expect, actual, diff);
  }
  avg_diff /= count;

  fprintf(fp, "# acos_cmath: Count: %d\tMax error: 0.000000\tAvg error: 0.000000\n", count);
  fprintf(fp, "# acos_cordic: Count: %d\tMax error: %lf\tAvg error: %lf\n", count, max_diff, avg_diff);

  printf("acos_cmath: Count: %d\tMax error: 0.000000\tAvg error: 0.000000\n", count);
  printf("acos_cordic: Count: %d\tMax error: %lf\tAvg error: %lf\n\n", count, max_diff, avg_diff);

  fclose(fp);

  if (avg_diff > 1) return THRESHOLD;
  return 0;
}


int main() {
  double start;
  double limit;
  double delta;
  int RC = 0;

  create_dir("acos_reports");

  // Test invalid argument
  test(2, 2, 1, S_W, S_IW, S_N_ITER);

  // Normal tests
  start = -1;
  limit = 1;
  delta = 0.01;
  RC |= test(start, limit, delta, S_W, S_IW, S_N_ITER, 1, 1);

  if (RC != 0)  printf("Errors may have occurred. Please double-check usage.\n");
  return RC;
}
