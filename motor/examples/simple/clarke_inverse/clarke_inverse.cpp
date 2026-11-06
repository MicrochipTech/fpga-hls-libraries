#include <stdio.h>
#include <stdint.h>
#include "motor.hpp"
#include <iostream>
#include <cmath>
#include "test_utils.hpp"

using namespace hls;
using namespace hls::motor;


/**
 * @brief Clarke (inverse) transform, balanced & amplitude-invariant (αβ → 3φ).
 *
 * Reconstructs three phase components from stationary-frame components assuming a
 * balanced three-phase system (a + b + c = 0). This is the inverse of the
 * balanced, amplitude-invariant Clarke used in the forward direction:
 *
 *   a = alpha
 *   b = ( -alpha + sqrt(3)*beta ) / 2
 *   c = ( -alpha - sqrt(3)*beta ) / 2
 *
 * With this scaling, the αβ ↔ dq pair can be chosen orthonormal so vector
 * magnitudes are preserved across αβ ↔ dq (given consistent Park/Inverse-Park).
 *
 * @param alpha  Stationary-frame α component.
 * @param beta   Stationary-frame β component.
 * @param[out] a  Reconstructed phase-A component.
 * @param[out] b  Reconstructed phase-B component.
 * @param[out] c  Reconstructed phase-C component.
 *
 * @pre The system is treated as balanced: a + b + c = 0.
 * @note Use together with the balanced, amplitude-invariant clarke_direct()
 * for a true inverse pair.
 */
void clarke_inverse(float alpha,
                    float beta,
                    float& a,
                    float& b,
                    float& c) {

    constexpr float kSqrt3 = 1.7320508075688772f;
    a = alpha;
    b = 0.5 * std::fma(kSqrt3, beta, -alpha);  // (√3*beta - alpha)/2
    c = -0.5 * std::fma(kSqrt3, beta,  alpha); // -(alpha + √3*beta)/2
}

//------------------------------------------------------------------------------
template <
unsigned int W_OUT,
int IW_OUT,
int N_ITERATIONS=16,
unsigned int W_IN,
int IW_IN
>
void clarke_inverse_wrapper(hls::ap_fixpt<W_IN, IW_IN> alpha,
                            hls::ap_fixpt<W_IN, IW_IN> beta,
                            hls::ap_fixpt<W_OUT, IW_OUT> &a,
                            hls::ap_fixpt<W_OUT, IW_OUT> &b,
                            hls::ap_fixpt<W_OUT, IW_OUT> &c) {

    #pragma HLS function top
    hls::motor::clarke_inverse(alpha, beta, a, b, c);
}

//------------------------------------------------------------------------------
int main(int argc, char* argv[]) {

    constexpr int W = 18;
    constexpr int IW = 4;
    using fixpt_t = hls::ap_fixpt<W,IW>;
    constexpr double EPS   = 1e-3;
    constexpr double SQRT3 = 1.7320508075688772;
    int err = 0;
    float alpha, beta;
    float a,b,c;
    fixpt_t hls_a, hls_b, hls_c;
 
    // Store the results into a report file
    
    create_dir("clarke_inverse_reports");
    std::string test_name = find_test_name("clarke_inverse", W, IW);

    FILE* fp = fopen(test_name.c_str(), "w");
    if (fp == NULL) {
        perror("Error: ");
        return -1;
    }
    fprintf(fp, "# REPORT PLOT");

    // ----------------------------------------------------------------
    // Zero vector stays zero: (alpha,beta)=(0,0) -> (Ia,Ib,Ic)=(0,0,0)
    // ----------------------------------------------------------------
    clarke_inverse(0.0, 0.0, a, b, c);
    clarke_inverse_wrapper<W,IW>(fixpt_t(0), fixpt_t(0), hls_a, hls_b, hls_c);

    err += !close_enough(a, hls_a.to_double(), EPS);
    err += !close_enough(b, hls_b.to_double(), EPS);
    err += !close_enough(c, hls_c.to_double(), EPS);

    err += !close_enough(0.0, hls_a.to_double(), EPS);
    err += !close_enough(0.0, hls_b.to_double(), EPS);
    err += !close_enough(0.0, hls_c.to_double(), EPS);
	
    printf("Expected a = %f, Actual a %f, Expected b = %f, Actual b = %f, Expected c = %f, Actual c %f, \n", a, hls_a.to_double(), b, hls_b.to_double(), c, hls_c.to_double());

    
    // -----------------------------------------------------------------------
    // Unit phase axes (balanced DC points), via αβ "basis" directions
    //
    //    Expected (balanced amplitude-invariant):
    //      Ia = alpha
    //      Ib = (-alpha + sqrt(3)*beta)/2
    //      Ic = (-alpha - sqrt(3)*beta)/2
    //
    //    Cases:
    //      A) α=1,  β=0          -> Ia= 1,   Ib= -0.5, Ic= -0.5
    //      B) α=-0.5, β= +√3/2   -> Ia=-0.5, Ib=  1,   Ic= -0.5
    //      C) α=-0.5, β= -√3/2   -> Ia=-0.5, Ib= -0.5, Ic=  1
    // -----------------------------------------------------------------------
    
    // A) α=1, β=0
    alpha = 1.0;
    beta = 0.0;

    clarke_inverse(alpha, beta, a, b, c);
    clarke_inverse_wrapper<W,IW>(fixpt_t(alpha), fixpt_t(beta), hls_a, hls_b, hls_c);

    err += !close_enough(hls_a.to_double(),  1.0,  EPS);
    err += !close_enough(hls_b.to_double(), -0.5,  EPS);
    err += !close_enough(hls_c.to_double(), -0.5,  EPS);

    err += !close_enough(a, hls_a.to_double(), EPS);
    err += !close_enough(b, hls_b.to_double(), EPS);
    err += !close_enough(c, hls_c.to_double(), EPS);
    printf("Expected a = %f, Actual a %f, Expected b = %f, Actual b = %f, Expected c = %f, Actual c %f, \n", a, hls_a.to_double(), b, hls_b.to_double(), c, hls_c.to_double());

    
    // B) α=-0.5, β= +√3/2
    alpha = -0.5;
    beta  = 0.5 * SQRT3; // √3/2 ≈ 0.8660254

    clarke_inverse(alpha, beta, a, b, c);
    clarke_inverse_wrapper<W,IW>(fixpt_t(alpha), fixpt_t(beta), hls_a, hls_b, hls_c);

    err += !close_enough(hls_a.to_double(), -0.5,  EPS);
    err += !close_enough(hls_b.to_double(), 1.0,  EPS);
    err += !close_enough(hls_c.to_double(), -0.5,  EPS);

    err += !close_enough(a, hls_a.to_double(), EPS);
    err += !close_enough(b, hls_b.to_double(), EPS);
    err += !close_enough(c, hls_c.to_double(), EPS);
    printf("Expected a = %f, Actual a %f, Expected b = %f, Actual b = %f, Expected c = %f, Actual c %f, \n", a, hls_a.to_double(), b, hls_b.to_double(), c, hls_c.to_double());

    
    // C) α=-0.5, β= -√3/2
    alpha = -0.5;
    beta  = -0.5 * SQRT3; // -√3/2

    clarke_inverse(alpha, beta, a, b, c);
    clarke_inverse_wrapper<W,IW>(fixpt_t(alpha), fixpt_t(beta), hls_a, hls_b, hls_c);

    err += !close_enough(hls_a.to_double(), -0.5,  EPS);
    err += !close_enough(hls_b.to_double(), -0.5,  EPS);
    err += !close_enough(hls_c.to_double(), 1.0,  EPS);

    err += !close_enough(a, hls_a.to_double(), EPS);
    err += !close_enough(b, hls_b.to_double(), EPS);
    err += !close_enough(c, hls_c.to_double(), EPS);
    printf("Expected a = %f, Actual a %f, Expected b = %f, Actual b = %f, Expected c = %f, Actual c %f, \n", a, hls_a.to_double(), b, hls_b.to_double(), c, hls_c.to_double());

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
    fprintf(fp, "#alpha input,beta input,a expected,b expected,c expected,a actual,b actual,c actual,a diff,b diff,c diff\n");

    const double A = 0.8;
    int count = 0;
    float max_diff = 0, avg_diff = 0;
    for (int k = 0; k <= 360; k += 3) {
        const float phi = k * M_PI / 180.0;
        const float alpha  = A * std::cos(phi);
        const float beta = A * std::cos(phi - M_PI/2);

        clarke_inverse(alpha, beta, a, b, c);
        clarke_inverse_wrapper<W,IW>(fixpt_t(alpha), fixpt_t(beta), hls_a, hls_b, hls_c);

        err += !close_enough(hls_a.to_double(), -0.5,  EPS);
        err += !close_enough(hls_b.to_double(), -0.5,  EPS);
        err += !close_enough(hls_c.to_double(), 1.0,  EPS);

        err += !close_enough(a, hls_a.to_double(), EPS);
        err += !close_enough(b, hls_b.to_double(), EPS);
        err += !close_enough(c, hls_c.to_double(), EPS);

	    float a_diff = fabs(a - hls_a.to_double());
	    float b_diff = fabs(b - hls_b.to_double());
        float c_diff = fabs(c - hls_c.to_double());

	    count+=2;
    	
        if (a_diff > max_diff) max_diff = a_diff;
    	avg_diff += a_diff;

    	if (b_diff > max_diff) max_diff = b_diff;
    	avg_diff += b_diff;

        if (c_diff > max_diff) max_diff = c_diff;
    	avg_diff += c_diff;

        printf("Expected a = %f, Actual a %f, Expected b = %f, Actual b = %f, Expected c = %f, Actual c %f, \n", a, hls_a.to_double(), b, hls_b.to_double(), c, hls_c.to_double());
	    fprintf(fp, "%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf\n", alpha, beta, a, b, c, (double)hls_a, (double)hls_b, (double)hls_c, a_diff, b_diff, c_diff);

    }
  
    avg_diff = avg_diff / count;
    fprintf(fp, "# clarke_inverse: Count: %d\tMax error: %lf\tAvg error: %lf\n", count, max_diff, avg_diff);    
    if(err) {
        printf("FAIL\n");
    } else {
        printf("PASS\n");
    }
    fclose(fp);

    return err;
}
