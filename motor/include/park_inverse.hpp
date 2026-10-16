
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


template <unsigned int W_OUT, int IW_OUT, int N_ITERATIONS=16, unsigned int W_IN, int IW_IN, unsigned int W_TH, int IW_TH>
void park_inverse_impl(hls::ap_fixpt<W_IN, IW_IN> d,
                hls::ap_fixpt<W_IN, IW_IN> q,
                hls::ap_fixpt<W_TH, IW_TH> theta,
                hls::ap_fixpt<W_OUT, IW_OUT> &alpha,
                hls::ap_fixpt<W_OUT, IW_OUT> &beta) {
    #pragma HLS function inline

	// Note that cos & sin range from -1 to 1
	hls::ap_fixpt<W_IN,2> cos_theta;
	hls::ap_fixpt<W_IN,2> sin_theta;

	hls::math::sincos<N_ITERATIONS>(theta, sin_theta, cos_theta);
 
	alpha = (d * cos_theta) + (q * sin_theta);
	beta = (q * cos_theta) + (d * sin_theta);
}

/**
 * @function park_inverse
 * Inverse Park transform (dq → αβ) using fixed-point CORDIC sine/cosine.
 *
 * Computes the standard right-handed inverse Park rotation with electrical angle θ:
 * $$V_{alpha} = V_{d} * cos(th) + V_{q} * sin(th)$$
 * $$V_{beta} = V_{q} * cos(th) + V_{d} * sin(th)$$
 * 
 * @template {unsigned int} W_OUT width of the output
 * @template {int} IW_OUT width of integer portion of the output
 * @template {int} N_ITERATIONS Number of CORDIC iterations for sin/cos. More iterations means higher accuracy but more latency/area. Default set to 16.
 * @template {unsigned int} W_IN width of the input (automatically inferred)
 * @template {int} IW_IN width of integer portion of the input (automatically inferred)
 *
 * @param {ap_fixpt<unsigned int W_IN, int IW_IN>} d Rotor-frame d-axis voltage (flux axis).
 * @param {ap_fixpt<unsigned int W_IN, int IW_IN>} q Rotor-frame q-axis voltage (torque axis).
 * @param {ap_fixpt<unsigned int W_IN, int IW_IN>} Electrical angle θ in revolutions for this control tick.
 * @param {ap_fixpt<unsigned int W_OUT, int IW_OUT>} alpha Stationary-frame α voltage component.
 * @param {ap_fixpt<unsigned int W_OUT, int IW_OUT>} beta Stationary-frame α voltage component.
 *
 * @example
 * using fixpt_t = hls::ap_fixpt<18,4>;
 * fixpt_t alpha, beta;
 * hls::ap_fixpt<9, 4> theta = 0.25;
 * fixpt_t d = 1;
 * fixpt_t q = 0;
 * hls::motor::park_inverse(d, q, theta, alpha, beta);
 */
template <unsigned int W_OUT, int IW_OUT, int N_ITERATIONS=16, unsigned int W_IN, int IW_IN>
void park_inverse(
                hls::ap_fixpt<W_IN, IW_IN> d,
                hls::ap_fixpt<W_IN, IW_IN> q,
				hls::ap_fixpt<W_IN, IW_IN> theta,
                hls::ap_fixpt<W_OUT, IW_OUT> &alpha,
                hls::ap_fixpt<W_OUT, IW_OUT> &beta) {

    #pragma HLS function pipeline
	hls::ap_fixpt<2 * W_IN+1, W_IN> theta_in_rad = theta_to_rad<2 * W_IN, W_IN, W_IN>(theta);
    park_inverse_impl(d, q, theta_in_rad, alpha, beta);
}

/**
 * @function park_inverse
 * Inverse Park transform (dq → αβ) using fixed-point CORDIC sine/cosine.
 *
 * Computes the standard right-handed inverse Park rotation with electrical angle θ:
 * $$V_{alpha} = V_{d} * cos(th) + V_{q} * sin(th)$$
 * $$V_{beta} = V_{q} * cos(th) + V_{d} * sin(th)$$
 * 
 * @template {unsigned int} W_OUT width of the output
 * @template {int} N_ITERATIONS Number of CORDIC iterations for sin/cos. More iterations means higher accuracy but more latency/area. Default set to 16.
 * @template {unsigned int} W_IN width of the input (automatically inferred)
 *
 * @param {ap_int<unsigned int W_IN>} d Rotor-frame d-axis voltage (flux axis).
 * @param {ap_int<unsigned int W_IN>} q Rotor-frame q-axis voltage (torque axis).
 * @param {ap_int<unsigned int W_IN>} Electrical angle θ in revolutions for this control tick.
 * @param {ap_int<unsigned int W_OUT>} alpha Stationary-frame α voltage component.
 * @param {ap_int<unsigned int W_OUT>} beta Stationary-frame α voltage component.
 *
 *  @example
 * hls::ap_int<18> alpha, beta;
 * hls::ap_int<18> d = 1;
 * hls::ap_uint<10> theta = 125;
 * hls::ap_int<18> q = 0;
 * hls::motor::park_inverse(d, q, theta, alpha, beta);
 */
template <unsigned int W_OUT, int N_ITERATIONS=16, unsigned int W_IN>
void park_inverse(hls::ap_int<W_IN> d,
                hls::ap_int<W_IN> q,
                hls::ap_uint<W_IN> theta,
                hls::ap_int<W_OUT> &alpha,
                hls::ap_int<W_OUT> &beta) {

	// Convert from revolutions to radians 
	hls::ap_fixpt<2 * W_IN+1, W_IN> theta_in_rad = theta_to_rad<2 * W_IN, W_IN, W_IN>(theta);

	hls::ap_fixpt<2 * W_IN, W_IN> d_fix = d, q_fix = q;
	hls::ap_fixpt<2 * W_OUT, W_OUT> alpha_fix, beta_fix;
	park_inverse_impl(d_fix, q_fix, theta_in_rad, alpha_fix, beta_fix);

	alpha = alpha_fix;
	beta = beta_fix;
}
}
}