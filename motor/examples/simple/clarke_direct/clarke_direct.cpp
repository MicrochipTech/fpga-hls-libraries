#include <stdio.h>
#include <stdint.h>
#include "motor.hpp"
#include <iostream>
#include <cmath>
#include "test_utils.hpp"

using namespace hls;
using namespace hls::motor;


/**
 * @brief Clarke (direct) transform, balanced & amplitude-invariant (3φ → αβ).
 *
 * Converts two measured phase components to stationary-frame components assuming a
 * balanced three-phase system where Ic = −(Ia + Ib). In this balanced form
 * Ic is not needed and is ignored.
 *
 * Equations (amplitude-invariant variant):
 *   alpha = a
 *   beta  = (a + 2*b) / sqrt(3)
 *
 * This variant is commonly paired with the right-handed Park/Inverse-Park so that
 * vector magnitude is preserved across αβ ↔ dq (with consistent scaling).
 *
 * @param a             Phase-A component.
 * @param b             Phase-B component.
 * @param[out] alpha    Stationary-frame α component.
 * @param[out] beta     Stationary-frame β component.
 *
 * @pre Balanced system: a + b + c ≈ 0 (or c ≈ −(a + b)).
 * @note This implementation uses a precomputed 1/sqrt(3) constant for efficiency.
 *
 */
void clarke_direct(float a,
                    float b,
                    float& alpha,
                    float& beta) {

    constexpr float kInvSqrt3 = 0.5773502691896258f; // 1/sqrt(3)
    alpha = a;
    beta  = kInvSqrt3 * std::fma(2.0f, b, a); // (a + 2*b)/sqrt(3)
}

//------------------------------------------------------------------------------
template <
unsigned int W_OUT,
int IW_OUT,
unsigned int W_IN,
int IW_IN
>
void clarke_direct_wrapper(
    hls::ap_fixpt<W_IN, IW_IN> a,
    hls::ap_fixpt<W_IN, IW_IN> b,
    hls::ap_fixpt<W_OUT, IW_OUT> &alpha,
    hls::ap_fixpt<W_OUT, IW_OUT> &beta) {

    #pragma HLS function top
    hls::motor::clarke_direct(a, b, alpha, beta);
}

//------------------------------------------------------------------------------
int main(int argc, char* argv[]) {

    constexpr int W = 18;
    constexpr int IW = 4;
    using fixpt_t = hls::ap_fixpt<W,IW>;
    constexpr float EPS   = 1e-3;
    int err = 0;
    float a,b;
    fixpt_t hls_a, hls_b;

    create_dir("clarke_direct_reports");
    std::string test_name = find_test_name("clarke_direct", W, IW);

    FILE* fp = fopen(test_name.c_str(), "w");
    fprintf(fp, "# REPORT PLOT");
    // ----------------------------------------------------------------
    // Zero vector stays zero: (Ia,Ib)=(0,0) -> (alpha,beta)=(0,0)
    // ----------------------------------------------------------------
    clarke_direct(0.0, 0.0, a, b);
    clarke_direct_wrapper<W,IW>(fixpt_t(0), fixpt_t(0), hls_a, hls_b);

    err += !close_enough(a, hls_a.to_double(), EPS);
    err += !close_enough(b, hls_b.to_double(), EPS);
    err += !close_enough(0, hls_a.to_double(), EPS);
    err += !close_enough(0, hls_b.to_double(), EPS);

    // -----------------------------------------------------------------------
    // Unit phase axes (balanced DC points)
    // For a balanced set: Ib = Ic = -0.5 when Ia = 1, etc.
    // Expected (balanced amplitude-invariant):
    //   alpha = Ia
    //   beta  = (Ia + 2*Ib)/sqrt(3)
    // -----------------------------------------------------------------------

    // 1) Unit A-axis: Ia=1, Ib=Ic=-0.5 => alpha=1, beta=0
    clarke_direct(1.0, -0.5, a, b);
    clarke_direct_wrapper<W,IW>(fixpt_t(1.0), fixpt_t(-0.5), hls_a, hls_b);

    err += !close_enough(a, 1.0, EPS);
    err += !close_enough(b, 0.0, EPS);
    err += !close_enough(a, hls_a.to_double(), EPS);
    err += !close_enough(b, hls_b.to_double(), EPS);
    printf("Expected a = %f, Actual a %f, Expected b = %f, Actual b = %f\n", a, hls_a.to_double(), b, hls_b.to_double());

    
    // 2) Unit B-axis: Ia=-0.5, Ib=1, Ic=-0.5 => alpha=-0.5, beta=+sqrt(3)/2 ≈ +0.8660
    clarke_direct(-0.5, 1.0, a, b);
    clarke_direct_wrapper<W,IW>(fixpt_t(-0.5), fixpt_t(1.0), hls_a, hls_b);

    err += !close_enough(a, -0.5, EPS);
    err += !close_enough(b, 0.866, EPS);
    err += !close_enough(a, hls_a.to_double(), EPS);
    err += !close_enough(b, hls_b.to_double(), EPS);
    printf("Expected a = %f, Actual a %f, Expected b = %f, Actual b = %f\n", a, hls_a.to_double(), b, hls_b.to_double());
   
    // 3) Unit C-axis: Ia=-0.5, Ib=-0.5, Ic=1 => alpha=-0.5, beta=-sqrt(3)/2 ≈ -0.8660
    clarke_direct(-0.5, -0.5, a, b);
    clarke_direct_wrapper<W,IW>(fixpt_t(-0.5), fixpt_t(-0.5), hls_a, hls_b);

    err += !close_enough(a, -0.5, EPS);
    err += !close_enough(b, -0.866, EPS);
    err += !close_enough(a, hls_a.to_double(), EPS);
    err += !close_enough(b, hls_b.to_double(), EPS);
    printf("Expected a = %f, Actual a %f, Expected b = %f, Actual b = %f\n", a, hls_a.to_double(), b, hls_b.to_double());

    // -----------------------------------------------------------------------
    // Angle sweep for a balanced sinusoidal 3φ set
    // Ia = A*cos(φ)
    // Ib = A*cos(φ - 2π/3)
    // (Ic would be A*cos(φ + 2π/3), but the balanced Clarke uses only Ia, Ib)
    // Expected (balanced amplitude-invariant):
    //   alpha = A*cos(φ)
    //   beta  = A*sin(φ)
    // -----------------------------------------------------------------------
    int delta = 3;

    fprintf(fp, "\n# using W: %d, IW: %d\n", W, IW);
    fprintf(fp, "#a input,b input,alpha expected,beta expected,alpha actual,beta actual,alpha diff,beta diff\n");

    const double A = 0.8;
    int count = 0;
    float max_diff = 0, avg_diff = 0;
    for (int k = 0; k <= 360; k += 3) {
        const float phi = k * M_PI / 180.0;
        const float Ia  = A * std::cos(phi);
        const float Ib  = A * std::cos(phi - 2.0*M_PI/3.0);

        clarke_direct((float)Ia, (float)Ib, a, b);
        clarke_direct_wrapper<W,IW>(fixpt_t(Ia), fixpt_t(Ib), hls_a, hls_b);

        const float a_ref = A * std::cos(phi);
        const float b_ref = A * std::sin(phi);

        err += !close_enough(a, a_ref, EPS);
        err += !close_enough(b, b_ref, EPS);
        err += !close_enough(hls_a.to_double(), a_ref, EPS);
        err += !close_enough(hls_b.to_double(), b_ref, EPS);

	    float a_diff = fabs(a - hls_a.to_double());
	    float b_diff = fabs(b - hls_b.to_double());

	    count+=2;
    	
        if (a_diff > max_diff) max_diff = a_diff;
    	avg_diff += a_diff;

    	if (b_diff > max_diff) max_diff = b_diff;
    	avg_diff += b_diff;

        printf("Expected a = %f, Actual a %f, Expected b = %f, Actual b = %f\n", a, hls_a.to_double(), b, hls_b.to_double());
	    fprintf(fp, "%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf\n", Ia, Ib, a, b, hls_a.to_double(), hls_b.to_double(), a_diff, b_diff);

    }
  
    avg_diff = avg_diff / count;
    fprintf(fp, "# clarke_direct: Count: %d\tMax error: %lf\tAvg error: %lf\n", count, max_diff, avg_diff);    
    if(err) {
        printf("FAIL\n");
    } else {
        printf("PASS\n");
    }
    fclose(fp);

    return err;
}
