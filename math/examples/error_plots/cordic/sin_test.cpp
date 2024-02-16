#include "../../test_utils.hpp"
#include "../../../include/hls_sin.hpp"
#include "widths.hpp"

#define N_ITERATIONS 26

/* Floating point sin function wrappers.
 *
 * The wrappers are only for converting from fixed point to floating point. */
template <unsigned int W, int IW>
double wrapper_cordic_16(double x){
#pragma HLS function top pipeline
  ap_fixpt<W_16, IW_16> x_fixpt = x;
  return hls::math::sin_cordic<W_16, IW_16, N_ITERATIONS>(x_fixpt).to_double();
}
template <unsigned int W, int IW>
double wrapper_cordic_24(double x){
#pragma HLS function top pipeline
  ap_fixpt<W_24, IW_24> x_fixpt = x;
  return hls::math::sin_cordic<W_24, IW_24, N_ITERATIONS>(x_fixpt).to_double();
}
template <unsigned int W, int IW>
double wrapper_cordic_28(double x){
#pragma HLS function top pipeline
  ap_fixpt<W_28, IW_28> x_fixpt = x;
  return hls::math::sin_cordic<W_28, IW_28, N_ITERATIONS>(x_fixpt).to_double();
}


template <unsigned int W, int IW>
int test(){
//    float delta = (2 * M_PI) / 256 / 1024;
    float delta = 0.01;
    double start = -2 * M_PI, limit = 2 * M_PI;

    double max_diff_cordic_16 = 0, avg_diff_cordic_16 = 0;
    double max_diff_cordic_24 = 0, avg_diff_cordic_24= 0;
    double max_diff_cordic_28 = 0, avg_diff_cordic_28 = 0;

    unsigned count = 0;

    for (double x = start; x < limit; x += delta, count++) {
        double actual_cordic_16 = wrapper_cordic_16<W, IW>(x);
        double actual_cordic_24 = wrapper_cordic_24<W, IW>(x);
        double actual_cordic_28 = wrapper_cordic_28<W, IW>(x);

        double expect = sin(x);

        double diff_cordic_16 = fabs(expect - actual_cordic_16);
        double diff_cordic_24 = fabs(expect - actual_cordic_24);
        double diff_cordic_28 = fabs(expect - actual_cordic_28);

	
        if (diff_cordic_16 > max_diff_cordic_16) max_diff_cordic_16 = diff_cordic_16;
        avg_diff_cordic_16 += diff_cordic_16;

        if (diff_cordic_24 > max_diff_cordic_24) max_diff_cordic_24 = diff_cordic_24;
        avg_diff_cordic_24 += diff_cordic_24;

        if (diff_cordic_28 > max_diff_cordic_28) max_diff_cordic_28 = diff_cordic_28;
        avg_diff_cordic_28 += diff_cordic_28;


  //    printf("%f,%lf,%lf,%lf,%lf,%lf,%lf\n", x, expect, actual_cordic, actual_taylor, actual_cordic_16, actual_cordic_24, actual_cordic_28);
    }

    avg_diff_cordic_16 /= count;
    avg_diff_cordic_24 /= count;
    avg_diff_cordic_28 /= count;

    printf("CORDIC 16 Count: %d\tMax diff: %.10lf\tAvg diff: %.10lf\n", count, max_diff_cordic_16, avg_diff_cordic_16);
    printf("CORDIC 24 Count: %d\tMax diff: %.10lf\tAvg diff: %.10lf\n", count, max_diff_cordic_24, avg_diff_cordic_24);
    printf("CORDIC 28 Count: %d\tMax diff: %.10lf\tAvg diff: %.10lf\n", count, max_diff_cordic_28, avg_diff_cordic_28);

    return 0;
}

int main(){

  // test<W_16, IW_16>();
  // test<W_24, IW_24>();
   test<W_28, IW_28>();

   return 0;
}
