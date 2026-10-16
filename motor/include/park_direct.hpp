// ©2025 Microchip Technology Inc. and its subsidiaries
//
// Subject to your compliance with these terms, you may use this Microchip
// software and any derivatives exclusively with Microchip products. You are
// responsible for complying with third party license terms applicable to your
// use of third party software (including open source software) that may
// accompany this Microchip software. SOFTWARE IS “AS IS.” NO WARRANTIES,
// WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY
// IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A
// PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT,
// SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE
// OF ANY KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF
// MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.
// TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP’S TOTAL LIABILITY ON ALL
// CLAIMS LATED TO THE SOFTWARE WILL NOT EXCEED AMOUNT OF FEES, IF ANY, YOU PAID
// DIRECTLY TO MICROCHIP FOR THIS SOFTWARE. MICROCHIP OFFERS NO SUPPORT FOR THE
// SOFTWARE. YOU MAY CONTACT MICROCHIP AT
// https://www.microchip.com/en-us/support-and-training/design-help/client-support-services
// TO INQUIRE ABOUT SUPPORT SERVICES AND APPLICABLE FEES, IF AVAILABLE.

#pragma once 

#include <type_traits>
#include "hls/ap_common.hpp"
#include "hls/ap_fixpt.hpp"
#include "hls/ap_int.hpp"
#include "hls_cos.hpp"
#include "hls_sin.hpp"
#include "common.hpp"

namespace hls {
namespace motor {


template <int N_ITERATIONS=16,unsigned int W_OUT, int IW_OUT, unsigned int W_IN, int IW_IN,unsigned int W_TH, int IW_TH>
void park_direct_impl(hls::ap_fixpt<W_IN, IW_IN> alpha,
                hls::ap_fixpt<W_IN, IW_IN> beta,
                hls::ap_fixpt<W_TH, IW_TH> theta,
                hls::ap_fixpt<W_OUT, IW_OUT> &d,
                hls::ap_fixpt<W_OUT, IW_OUT> &q) {
	// While this function can be used alone if your inputs are fixpt, it's also used as the basis for the integer input park_direct.
	// Since the latter case is what the FOC transform IP is, we automatically inline it, but you can remove it if you want  
    #pragma HLS function inline

	// Note that cos & sin range from -1 to 1
	hls::ap_fixpt<W_IN,2> cos_theta;
	hls::ap_fixpt<W_IN,2> sin_theta;

	hls::math::sincos<N_ITERATIONS>(theta, sin_theta, cos_theta);
	d = (alpha * cos_theta) + (beta * sin_theta);
  	q = (beta * cos_theta) - (alpha * sin_theta);
}

/**
 * @function park_direct
 * Park transform (αβ → dq) using CORDIC-generated sin/cos, fixed-point.
 *
 * Converts stationary-frame (alpha, beta) components into rotating-frame
 * (d, q) components for a given electrical angle theta (revolutions).
 *
 *   $$d = alpha * cos(theta) + beta * sin(theta)$$
 *   $$q = beta  * cos(theta) - alpha * sin(theta)$$
 *
 * Conceptually, this is a 2-D rotation of the vector [alpha, beta] by angle
 * +theta, producing the direct (d) and quadrature (q) axes. In exact math, the
 * rotation preserves energy: d^2 + q^2 ≈ alpha^2 + beta^2 (subject to
 * quantization and rounding).
 *
 * Implementation details:
 * - cos(theta) and sin(theta) are computed via CORDIC with N_ITERATIONS.
 *
 * @template {unsigned int} W_OUT width of the output
 * @template {int} IW_OUT width of integer portion of the output
 * @template {int} N_ITERATIONS Number of CORDIC iterations for sin/cos. More iterations means higher accuracy but more latency/area. Default set to 16.
 * @template {unsigned int} W_IN width of the input (automatically inferred)
 * @template {int} IW_IN width of integer portion of the input (automatically inferred)
 *
 * @param {ap_fixpt<unsigned int W_IN, int IW_IN>} I_alpha Stationary-frame α component.
 * @param {ap_fixpt<unsigned int W_IN, int IW_IN>} I_beta Stationary-frame β component.
 * @param {ap_fixpt<unsigned int W_IN, int IW_IN>} theta Electrical angle in revolutions.
 * @param {ap_fixpt<unsigned int W_OUT, int IW_OUT>} d  Direct-axis component (rotating frame).
 * @param {ap_fixpt<unsigned int W_OUT, int IW_OUT>} q  Quadrature-axis component (rotating frame).
 *
 * @example
 * using fixpt_t = hls::ap_fixpt<18,4>;
 * fixpt_t q, d;
 * hls::ap_fixpt<9, 4> theta = 0.25;
 * fixpt_t alpha = 1;
 * fixpt_t beta = 0;
 * hls::motor::park_direct(alpha, beta, theta, d, q);
 */
template <unsigned int W_OUT, int IW_OUT, int N_ITERATIONS=16, unsigned int W_IN, int IW_IN>
void park_direct(hls::ap_fixpt<W_IN, IW_IN> alpha,
                hls::ap_fixpt<W_IN, IW_IN> beta,
                hls::ap_fixpt<W_IN, IW_IN> theta,
                hls::ap_fixpt<W_OUT, IW_OUT> &d,
                hls::ap_fixpt<W_OUT, IW_OUT> &q) {

	// Convert from revolutions to radians 
	hls::ap_fixpt<2 * W_IN+1, W_IN> theta_in_rad = theta_to_rad<2 * W_IN, W_IN>(theta);
	park_direct_impl(alpha, beta, theta_in_rad, d, q);
}


/**
 * @function park_direct
 * Park transform (αβ → dq) using CORDIC-generated sin/cos, integer.
 *
 * Converts stationary-frame (alpha, beta) components into rotating-frame
 * (d, q) components for a given electrical angle theta (revolutions).
 *
 *   $$d = alpha * cos(theta) + beta * sin(theta)$$
 *   $$q = beta  * cos(theta) - alpha * sin(theta)$$
 *
 * Conceptually, this is a 2-D rotation of the vector [alpha, beta] by angle
 * +theta, producing the direct (d) and quadrature (q) axes. In exact math, the
 * rotation preserves energy: d^2 + q^2 ≈ alpha^2 + beta^2 (subject to
 * quantization and rounding).
 *
 * Implementation details:
 * - cos(theta) and sin(theta) are computed via CORDIC with N_ITERATIONS.
 *
 * @template {unsigned int} W_OUT width of the output
 * @template {int} IW_OUT width of integer portion of the output
 * @template {int} N_ITERATIONS Number of CORDIC iterations for sin/cos. More iterations means higher accuracy but more latency/area. Default set to 16.
 * @template {unsigned int} W_IN width of the input (automatically inferred)
 * @template {int} IW_IN width of integer portion of the input (automatically inferred)
 *
 * @param {ap_int<unsigned int W_IN>} I_alpha Stationary-frame α component.
 * @param {ap_int<unsigned int W_IN>} I_beta Stationary-frame β component.
 * @param {ap_uint<unsigned int W_IN>} theta Electrical angle in revolutions.
 * @param {ap_int<unsigned int W_OUT>} d  Direct-axis component (rotating frame).
 * @param {ap_int<unsigned int W_OUT>} q  Quadrature-axis component (rotating frame).
 *
 * @example
 * hls::ap_int<18> q, d;
 * hls::ap_uint<10> theta = 0.25;
 * hls::ap_int<18> alpha = 1;
 * hls::ap_int<18> beta = 0;
 * hls::motor::park_direct(alpha, beta, theta, d, q);
 */
template <unsigned int W_OUT, int N_ITERATIONS=16, unsigned int W_IN>
void park_direct(hls::ap_int<W_IN> alpha,
                hls::ap_int<W_IN> beta,
                hls::ap_uint<W_IN> theta,
                hls::ap_int<W_OUT> &d,
                hls::ap_int<W_OUT> &q) {

	// Convert from revolutions to radians 
	hls::ap_fixpt<2 * W_IN+1, W_IN> theta_in_rad = theta_to_rad<2 * W_IN, W_IN, W_IN>(theta);

	hls::ap_fixpt<2 * W_IN, W_IN> alpha_fix = alpha, beta_fix = beta;
	hls::ap_fixpt<2 * W_OUT, W_OUT> d_fix, q_fix;

	park_direct_impl(alpha_fix, beta_fix, theta_in_rad, d_fix, q_fix);

	d = d_fix;
	q = q_fix;
}
}
}
