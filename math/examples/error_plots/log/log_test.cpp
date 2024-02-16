#include "../../test_utils.hpp"
#include "../../../include/hls_log2.hpp"
#include "widths.hpp"

/* Floating point sin function wrappers.
 *
 * The wrappers are only for converting from fixed point to floating point. */
template <unsigned int W, int IW>
double wrapper_lut_16(double x){
#pragma HLS function top pipeline
  ap_ufixpt<W_16, IW_16> x_fixpt(x);
  return hls::math::log2_lut<W_16, IW_16>(x_fixpt).to_double();
}
template <unsigned int W, int IW>
double wrapper_lut_24(double x){
#pragma HLS function top pipeline
  ap_ufixpt<W_24, IW_24> x_fixpt(x);
  return hls::math::log2_lut<W_24, IW_24>(x_fixpt).to_double();
}
template <unsigned int W, int IW>
double wrapper_lut_28(double x){
#pragma HLS function top pipeline
  ap_ufixpt<W_28, IW_28> x_fixpt(x);
  return hls::math::log2_lut<W_28, IW_28>(x_fixpt).to_double();
}


template <unsigned int W, int IW>
int test(){
//    float delta = (2 * M_PI) / 256 / 1024;
    float delta = 0.01;
    double start = 0.01, limit = M_PI;

    double max_diff_lut_16 = 0, avg_diff_lut_16 = 0;
    double max_diff_lut_24 = 0, avg_diff_lut_24= 0;
    double max_diff_lut_28 = 0, avg_diff_lut_28 = 0;

    unsigned count = 0;

    for (double x = start; x < limit; x += delta, count++) {
        double actual_lut_16 = wrapper_lut_16<W, IW>(x);
        double actual_lut_24 = wrapper_lut_24<W, IW>(x);
        double actual_lut_28 = wrapper_lut_28<W, IW>(x);

        double expect = log2(x);

        double diff_lut_16 = fabs(expect - actual_lut_16);
        double diff_lut_24 = fabs(expect - actual_lut_24);
        double diff_lut_28 = fabs(expect - actual_lut_28);

	
        if (diff_lut_16 > max_diff_lut_16) max_diff_lut_16 = diff_lut_16;
        avg_diff_lut_16 += diff_lut_16;

        if (diff_lut_24 > max_diff_lut_24) max_diff_lut_24 = diff_lut_24;
        avg_diff_lut_24 += diff_lut_24;

        if (diff_lut_28 > max_diff_lut_28) max_diff_lut_28 = diff_lut_28;
        avg_diff_lut_28 += diff_lut_28;


//     printf("%f,%lf,%lf,%lf,%lf\n", x, expect,  actual_lut_16, actual_lut_24, actual_lut_28);
    }

    avg_diff_lut_16 /= count;
    avg_diff_lut_24 /= count;
    avg_diff_lut_28 /= count;

    printf("LUT 16 Count: %d\tMax diff: %.10lf\tAvg diff: %.10lf\n", count, max_diff_lut_16, avg_diff_lut_16);
    printf("LUT 24 Count: %d\tMax diff: %.10lf\tAvg diff: %.10lf\n", count, max_diff_lut_24, avg_diff_lut_24);
    printf("LUT 28 Count: %d\tMax diff: %.10lf\tAvg diff: %.10lf\n", count, max_diff_lut_28, avg_diff_lut_28);

    return 0;
}

int main(){

  // test<W_16, IW_16>();
  // test<W_24, IW_24>();
   test<W_28, IW_28>();

   return 0;
}
