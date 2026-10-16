#include <stdio.h>
#include <stdint.h>
#include "motor.hpp"
#include <iostream>
#include <cmath>
#include "test_utils.hpp"


using namespace hls;
using namespace hls::motor;


/**
 * @brief Park (direct) transform: rotate stationary-frame (α,β) into rotor-frame (d,q).
 *
 * Computes the standard right-handed Park transform using the electrical angle θ:
 * 
 * d = alpha*cos + beta*sin
 * q = -alpha*sin + beta*cos
 *
 * @param alpha   Stationary-frame α component (typically from Clarke).
 * @param beta    Stationary-frame β component (typically from Clarke).
 * @param theta   Electrical angle θ (radians).
 * @param[out] d  Rotor-frame d-axis component (flux axis).
 * @param[out] q  Rotor-frame q-axis component (torque axis).
 *
 */
void park_direct_cmath(float alpha,
                float beta,
                float theta,
                float& d,
                float& q) {

    float cos_t = std::cos(theta);
    float sin_t = std::sin(theta);
    d = (alpha * cos_t) + (beta * sin_t);
    q = (beta * cos_t) - (alpha * sin_t); // q = -alpha*sin + beta*cos
}

//------------------------------------------------------------------------------
template <unsigned int W_OUT, int IW_OUT, int N_ITERATIONS=16, unsigned int W_IN, int IW_IN>
void park_direct_wrapper(
    hls::ap_fixpt<W_IN, IW_IN> alpha,
    hls::ap_fixpt<W_IN, IW_IN> beta,
    hls::ap_fixpt<W_IN, IW_IN> theta,
    hls::ap_fixpt<W_OUT, IW_OUT> &d,
    hls::ap_fixpt<W_OUT, IW_OUT> &q) {

    #pragma HLS function top
    hls::motor::park_direct(alpha, beta, theta, d, q);
}

//------------------------------------------------------------------------------
int main(int argc, char* argv[]) {

    constexpr int W = 18;
    constexpr int IW = 4;
    using fixpt_t = hls::ap_fixpt<W,IW>;
    constexpr int N_ITERATIONS = 11;
    constexpr double EPS = 1e-3;
    int err = 0, count = 0;
    float th, c, d, q, s, alpha, beta, d_diff, q_diff, max_diff, avg_diff;
    fixpt_t hls_q, hls_d;

    create_dir("park_direct_reports");

    FILE* fp = fopen(find_test_name("park_direct", W, IW).c_str(), "w");
    fprintf(fp, "# REPORT PLOT");


    // This testcase is from the Libero FOC transform IP testbench.
    alpha = 2046;
    beta = 31;
    // 655 is 000000001010001111 in decimal, which shifted 18 bits is 0.002498626708984375 revolutions 
    float theta_fl = 0.002498626708984375 * 2 * M_PI;
    park_direct_cmath(alpha, beta, theta_fl, d, q);
    park_direct(fixpt_t(2046), fixpt_t(31), fixpt_t(655), hls_d, hls_q);
    std::cout << "(PARK TESTCASE 1) Expected d = " << d << " Actual d = " << hls_d.to_double() << " Expected q = " << q << " Actual q = " << hls_q.to_double() << std::endl;


    // Zero vector stays zero
    park_direct_cmath(0.0, 0.0, 0.0, d, q);
    park_direct_wrapper<W,IW>(fixpt_t(0), fixpt_t(0), fixpt_t(0), hls_d, hls_q);
    err += !close_enough(d, hls_d.to_double(), EPS);
    err += !close_enough(q, hls_q.to_double(), EPS);
    err += !close_enough(hls_d.to_double(), 0, EPS);
    err += !close_enough(hls_q.to_double(), 0, EPS);
    std::cout << "(PARK TESTCASE 2) Expected d = " << d << " Actual d = " << hls_d.to_double() << " Expected q = " << q << " Actual q = " << hls_q.to_double() << std::endl;

    // Unit α axis @ θ=0: d=1, q=0
    park_direct_cmath(1.0, 0.0, 0.0, d, q);
    park_direct_wrapper<W,IW>(fixpt_t(1), fixpt_t(0), fixpt_t(0), hls_d, hls_q);
    err += !close_enough(d, hls_d.to_double(), EPS);
    err += !close_enough(q, hls_q.to_double(), EPS);
    err += !close_enough(hls_d.to_double(), 1, EPS);
    err += !close_enough(hls_q.to_double(), 0, EPS);
    std::cout << "(PARK TESTCASE 3) Expected d = " << d << " Actual d = " << hls_d.to_double() << " Expected q = " << q << " Actual q = " << hls_q.to_double() << std::endl;

    std::cout << "(PARK TESTCASE: ANGLE SWEEP)";
    fprintf(fp, "\n# Expecting d = 1, q = 0. Using W: %d, IW: %d\n", W, IW);
    fprintf(fp, "#alpha input,beta input,d expected,q expected,d actual,q actual,d diff,q diff\n");

    for (float th_in_revs = 0; th_in_revs <= 1; th_in_revs += 0.01) {
        th = th_in_revs * (2 * M_PI);
        printf("\nAngle (rads, revolutions): %f, %f\n", th, th_in_revs);
        c = std::cos(th);
        s = std::sin(th);

        // Case A: vector aligned with θ  → α=cosθ, β=sinθ → d=1, q=0
        park_direct_cmath(c, s, th, d, q);
        park_direct_wrapper<W,IW>(fixpt_t(c), fixpt_t(s), fixpt_t(th_in_revs), hls_d, hls_q);
        err += !close_enough(d, hls_d.to_double(), EPS);
        err += !close_enough(q, hls_q.to_double(), EPS);
        err += !close_enough(hls_d.to_double(), 1, EPS);
        err += !close_enough(hls_q.to_double(), 0, EPS);

        d_diff = fabs(d - hls_d.to_double());
	    q_diff = fabs(q - hls_q.to_double());
        
	    count+=2;
    	
        if (d_diff > max_diff) max_diff = d_diff;
    	avg_diff += d_diff;

    	if (q_diff > max_diff) max_diff = q_diff;
    	avg_diff += q_diff;

        // Only making one plot to prove the point
        printf("Expected d = %f, Actual d %f, Expected q = %f, Actual q = %f\n", d, hls_d.to_double(), q, hls_q.to_double());
        fprintf(fp, "%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf\n", c, s, d, q, (double)hls_d, (double)hls_q, d_diff, q_diff);

        
        // Case B: vector 90° ahead → α=-sinθ, β=cosθ → d=0, q=1
        park_direct_cmath(-s, c, th, d, q);
        park_direct_wrapper<W,IW>(fixpt_t(-s), fixpt_t(c), fixpt_t(th_in_revs), hls_d, hls_q);
        err += !close_enough(d, hls_d.to_double(), EPS);
        err += !close_enough(q, hls_q.to_double(), EPS);
        err += !close_enough(hls_d.to_double(), 0, EPS);
        err += !close_enough(hls_q.to_double(), 1, EPS);

    }
    avg_diff = avg_diff / count;
    fprintf(fp, "# park_direct: Count: %d\tMax error: %lf\tAvg error: %lf\n", count, max_diff, avg_diff);    
    if(err) {
        printf("FAIL\n");
    } else {
        printf("PASS\n");
    }

    fclose(fp);
    return err;
}
