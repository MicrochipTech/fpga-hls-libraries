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
#include "math/include/hls_floor.hpp"

#define THRESHOLD 1
using namespace hls::math;

ap_fixpt<M_W, M_IW> floor_hls_M_wrapper(ap_fixpt<M_W, M_IW>x){
#pragma HLS function pipeline top
  return floor<M_W, M_IW>(x);
}

double floor_cmath_wrapper(double x){
#pragma HLS function pipeline top
  return floor(x);
}

int test(double start_at, double limit, double delta, unsigned int W, int IW, int graph = 0, int report = 0){
  double max_diff = 0, avg_diff = 0;
  int count = 0;
  std::string test_name = find_test_name("floor", W, IW, start_at, limit);

  FILE* fp = fopen(test_name.c_str(), "w");
  if (graph) fprintf(fp, "# PLOT ");
  if (report) fprintf(fp, "# REPORT ");
  fprintf(fp, "\n# from %f to %f at intervals of %f, using W: %d, IW: %d\n", start_at, limit, delta, W, IW);  
  fprintf(fp, "# x,expected,floor_hls,floor_hls diff\n");

  printf("From %f to %f at intervals of %f, using W: %d, IW: %d\n", start_at, limit, delta, W, IW);

  for (double x = start_at; x <= limit; x += delta) {
    ap_fixpt<M_W, M_IW> x_fixpt = x;

    double expect, actual;
    expect = floor_cmath_wrapper(x);
    actual = floor_hls_M_wrapper(x_fixpt).to_double();
    double diff = fabs(expect - actual);
    if (diff > max_diff)
      max_diff = diff;
    avg_diff += diff;

    count++;

    fprintf(fp, "%f,%lf,%lf,%lf\n", x, expect, actual, diff);
  }

  avg_diff /= count;

  fprintf(fp, "# floor_cmath: Count: %d\tMax error: 0.000000\tAvg error: 0.000000\n", count);
  fprintf(fp, "# floor_hls: Count: %d\tMax error: %lf\tAvg error: %lf\n", count, max_diff, avg_diff);

  printf("floor_cmath: Count: %d\tMax error: 0.000000\tAvg error: 0.000000\n", count);
  printf("floor_hls: Count: %d\tMax error: %lf\tAvg error: %lf\n\n", count, max_diff, avg_diff);

    fclose(fp);

  if (avg_diff > THRESHOLD) return 1;
  return 0;
}

int main() {
  double start;
  double limit;
  double delta;
  int RC = 0;
  create_dir("floor_reports");

  start = -10;  limit = 10;  delta = 0.2;
  RC |= test(start, limit, delta, M_W, M_IW, 1, 1); 

  if (RC != 0)  printf("Errors may have occurred. Please double-check usage.\n");
  return RC;
}
