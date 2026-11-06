#include <stdio.h>
#include <stdint.h>
#include "motor.hpp"
#include <iostream>
#include <cmath>
#include "test_utils.hpp"

using namespace hls;
using namespace hls::motor;


/**
 * @brief Inverse Park transform: rotate rotor-frame (d,q) back to stationary-frame (α,β).
 *
 * Computes the standard right-handed inverse Park using electrical angle θ:
 * α = dcosθ − qsinθ,
 * β = dsinθ + qcosθ.
 *
 * @param d      Rotor-frame d-axis component (flux axis).
 * @param q      Rotor-frame q-axis component (torque axis).
 * @param theta  Electrical angle θ (radians) for the same control tick as @p d and @p q.
 * @param[out] alpha  Stationary-frame α component.
 * @param[out] beta   Stationary-frame β component.
 *
 * @pre @p theta corresponds to the same sampling instant as @p d and @p q.
 * @note Implementation uses `std::fma` (fused multiply–add) to reduce rounding error
 *       and typically map to a single FMA instruction where available.
 */
void park_inverse_cmath(float d,
                float q,
                float theta,
                float& alpha,
                float& beta) {

    const float cos_t = std::cos(theta);
    const float sin_t = std::sin(theta);

    alpha = (d * cos_t) - (q * sin_t); // α = d*cos - q*sin
    beta  = (d * sin_t)  + (q * cos_t); // β = d*sin + q*cos
}


//------------------------------------------------------------------------------
template <unsigned int W_OUT, int IW_OUT, int N_ITERATIONS=16, unsigned int W_IN, int IW_IN>
void park_inverse_wrapper(hls::ap_fixpt<W_IN, IW_IN> theta,
    hls::ap_fixpt<W_IN, IW_IN> d,
    hls::ap_fixpt<W_IN, IW_IN> q,
    hls::ap_fixpt<W_OUT, IW_OUT> &alpha,
    hls::ap_fixpt<W_OUT, IW_OUT> &beta) {

    #pragma HLS function top
    hls::motor::park_inverse(d, q, theta, alpha, beta);
}

//------------------------------------------------------------------------------
int main(int argc, char* argv[]) {

    constexpr int W = 18;
    constexpr int IW = 4;
    using fixpt_t = hls::ap_fixpt<W,IW>;
    constexpr double EPS   = 1e-3;
    int err = 0, count = 0;
    float alpha, beta, th, th_in_revs, alpha_diff, beta_diff, max_diff, avg_diff;
    fixpt_t hls_alpha, hls_beta, hls_d, hls_q;

    create_dir("park_inverse_reports");
    std::string test_name = find_test_name("park_inverse", W, IW);

    FILE* fp = fopen(test_name.c_str(), "w");
    fprintf(fp, "# REPORT PLOT");

    // --------------------------------------------------------------------
    // Zero vector stays zero: (d, q) = (0, 0) -> (alpha, beta) = (0, 0)
    // --------------------------------------------------------------------
    for (int k = 0; k <= 360; k += 15) {
        th = k * M_PI / 180;
        th_in_revs = th / (2 * M_PI);
        park_inverse_cmath(/*d=*/0.0, /*q=*/0.0, /*theta=*/th, alpha, beta);
        park_inverse_wrapper(fixpt_t(th_in_revs), fixpt_t(0), fixpt_t(0), hls_alpha, hls_beta);
        err += !close_enough(alpha, hls_alpha.to_double(), EPS);
        err += !close_enough(beta,  hls_beta.to_double(), EPS);
        err += !close_enough(alpha, 0.0, EPS);
        err += !close_enough(beta,  0.0, EPS);
    }

    // -------------------------------------------------------------------
    // Unit α axis @ θ = 0: with d=1, q=0 → alpha=1, beta=0 (by definition)
    // -------------------------------------------------------------------
    park_inverse_cmath(/*d=*/1.0, /*q=*/0.0, /*theta=*/0.0, alpha, beta);
    park_inverse_wrapper(fixpt_t(0), fixpt_t(1), fixpt_t(0), hls_alpha, hls_beta);
    err += !close_enough(alpha, hls_alpha.to_double(), EPS);
    err += !close_enough(beta,  hls_beta.to_double(), EPS);
    err += !close_enough(alpha, 1.0, EPS);
    err += !close_enough(beta,  0.0, EPS);

    fprintf(fp, "\n# Using W: %d, IW: %d\n", W, IW);
    fprintf(fp, "#d input,q input,alpha expected,beta expected,alpha actual,beta actual,alpha diff,beta diff\n");
    // ---------------------------------------------------------------------
    // Angle sweep for two canonical unit vectors:
    //  A) (d,q) = (1,0)  → (alpha,beta) = (cos θ, sin θ)
    //  B) (d,q) = (0,1)  → (alpha,beta) = (-sin θ, cos θ)
    // ---------------------------------------------------------------------
    for (int k = 0; k <= 360; k += 3) {
        th = k * M_PI / 180;
        th_in_revs = th / (2 * M_PI);
        float c = std::cos(th);
        float s = std::sin(th);

        printf("\n\nAngle (rad) %f: \n", th);


        // Case A
        park_inverse_cmath(/*d=*/1.0, /*q=*/0.0, th, alpha, beta);
        park_inverse_wrapper(fixpt_t(th_in_revs), fixpt_t(1), fixpt_t(0), hls_alpha, hls_beta);
        err += !close_enough(alpha, hls_alpha.to_double(), EPS);
        err += !close_enough(beta,  hls_beta.to_double(), EPS);
        err += !close_enough(alpha, c, EPS);
        err += !close_enough(beta,  s, EPS);

        alpha_diff = fabs(alpha - hls_alpha.to_double());
	    beta_diff = fabs(beta - hls_beta.to_double());
        
	    count+=2;
    	
        if (alpha_diff > max_diff) max_diff = alpha_diff;
    	avg_diff += alpha_diff;

    	if (beta_diff > max_diff) max_diff = beta_diff;
    	avg_diff += beta_diff;

        printf("Expected alpha = %f, Actual alpha %f, Expected beta = %f, Actual beta = %f\n", alpha, hls_alpha.to_double(), beta, hls_beta.to_double());
        fprintf(fp, "1,0,%lf,%lf,%lf,%lf,%lf,%lf\n", alpha, beta, (double)hls_alpha, (double)hls_beta, alpha_diff, beta_diff);


        // Case B
        park_inverse_cmath(/*d=*/0.0, /*q=*/1.0, th, alpha, beta);
        park_inverse_wrapper(fixpt_t(th_in_revs), fixpt_t(0), fixpt_t(1), hls_alpha, hls_beta);
        err += !close_enough(alpha, hls_alpha.to_double(), EPS);
        err += !close_enough(beta,  hls_beta.to_double(), EPS);
        err += !close_enough(alpha, -s, EPS);
        err += !close_enough(beta,   c, EPS);  
        printf("Expected alpha = %f, Actual alpha %f, Expected beta = %f, Actual beta = %f\n", alpha, hls_alpha.to_double(), beta, hls_beta.to_double());
 
    }
    avg_diff = avg_diff / count;
    fprintf(fp, "# park_inverse: Count: %d\tMax error: %lf\tAvg error: %lf\n", count, max_diff, avg_diff); 
    if(err) {
        printf("FAIL\n");
    } else {
        printf("PASS\n");
    }

    fclose(fp);
    return err;
}