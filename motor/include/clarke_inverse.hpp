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
#include "hls_sqrt.hpp"

namespace hls {
namespace motor {

template <unsigned int W_OUT, int IW_OUT, unsigned int W_IN, int IW_IN>
void clarke_inverse_impl(hls::ap_fixpt<W_IN, IW_IN> alpha,
                hls::ap_fixpt<W_IN, IW_IN> beta,
                hls::ap_fixpt<W_OUT, IW_OUT> &a,
                hls::ap_fixpt<W_OUT, IW_OUT> &b,
                hls::ap_fixpt<W_OUT, IW_OUT> &c) {

    #pragma HLS function inline

    const hls::ap_fixpt<W_OUT,1> Kd = 0.86602540378443864676; // sqrt(3)/2
    hls::ap_fixpt<W_OUT, IW_OUT> tmp = Kd * beta;
    hls::ap_fixpt<W_OUT, IW_OUT> alpha_half = alpha >> 1;
    b =  tmp - alpha_half;
    c = - tmp - alpha_half;
    a = alpha;
}

/**
 * @function clarke_inverse
 * 
 * Inverse Clarke transform (αβ → abc) with constant √3/2 factor, fixed-point.
 *
 * Computes the three-phase components from stationary-frame inputs:
 *   $$a = alpha$$
 *   $$b = \frac{-alpha + \sqrt{3} * beta} {2}$$
 *   $$c = \frac{-alpha - \sqrt{3} * beta}{2}$$
 * which implies the zero-sum property a + b + c = 0.
 *
 * @template {unsigned int} W_OUT width of the output
 * @template {int} IW_OUT width of integer portion of the output
 * @template {unsigned int} W_IN width of the input (automatically inferred)
 * @template {int} IW_IN width of integer portion of the input (automatically inferred)
 *
 * @param {ap_fixpt<unsigned int W_IN, int IW_IN>} alpha   Clarke α component.
 * @param {ap_fixpt<unsigned int W_IN, int IW_IN>} beta    Clarke β component.
 * @param {ap_fixpt<unsigned int W_OUT, int IW_OUT>} a   Phase-A result.
 * @param {ap_fixpt<unsigned int W_OUT, int IW_OUT>} b   Phase-B result.
 * @param {ap_fixpt<unsigned int W_OUT, int IW_OUT>} c   Phase-C result.
 *
 * @example
 * using fixpt_t = hls::ap_fixpt<18,4>;
 * fixpt_t a, b, c;
 * fixpt_t alpha = -0.5;
 * fixpt_t beta = 0.5 * 0.8660254; 
 * hls::motor::clarke_inverse(alpha, beta, a, b, c)
 */

template <unsigned int W_OUT, int IW_OUT, unsigned int W_IN, int IW_IN>
void clarke_inverse(hls::ap_fixpt<W_IN, IW_IN> alpha,
                hls::ap_fixpt<W_IN, IW_IN> beta,
                hls::ap_fixpt<W_OUT, IW_OUT> &a,
                hls::ap_fixpt<W_OUT, IW_OUT> &b,
                hls::ap_fixpt<W_OUT, IW_OUT> &c) {

    #pragma HLS function pipeline
    clarke_inverse_impl<W_OUT, IW_OUT, W_IN, IW_IN>(alpha, beta, a, b, c);
}

/**
 * @function clarke_inverse
 * 
 * Inverse Clarke transform (αβ → abc) with constant √3/2 factor, integer.
 *
 * Computes the three-phase components from stationary-frame inputs:
 *   $$a = alpha$$
 *   $$b = \frac{-alpha + \sqrt{3} * beta} {2}$$
 *   $$c = \frac{-alpha - \sqrt{3} * beta}{2}$$
 * which implies the zero-sum property a + b + c = 0.
 *
 * @template {unsigned int} W_OUT width of the output
 * @template {int} IW_OUT width of integer portion of the output
 * @template {unsigned int} W_IN width of the input (automatically inferred)
 * @template {int} IW_IN width of integer portion of the input (automatically inferred)
 *
 * @param {ap_int<unsigned int W_IN>} alpha   Clarke α component.
 * @param {ap_int<unsigned int W_IN>} beta    Clarke β component.
 * @param {ap_int<unsigned int W_OUT>} a   Phase-A result.
 * @param {ap_int<unsigned int W_OUT>} b   Phase-B result.
 * @param {ap_int<unsigned int W_OUT>} c   Phase-C result.
 *
 * @example
 * hls::ap_int<18> a, b, c;
 * hls::ap_int<18> alpha = 1047;
 * hls::ap_int<18> beta = 6779; 
 * hls::motor::clarke_inverse(alpha, beta, a, b, c)
 */
template <unsigned int W_OUT, unsigned int W_IN>
void clarke_inverse(hls::ap_int<W_IN> alpha,
                    hls::ap_int<W_IN> beta,
                    hls::ap_int<W_OUT> &a,
                    hls::ap_int<W_OUT> &b,
                    hls::ap_int<W_OUT> &c) {

    hls::ap_fixpt<W_IN*2, W_IN> alpha_fix = alpha;
    hls::ap_fixpt<W_IN*2, W_IN> beta_fix = beta;

    hls::ap_fixpt<W_OUT*2, W_OUT> a_fix, b_fix, c_fix;
    
    clarke_inverse_impl<W_OUT, W_OUT, W_IN, W_IN>(alpha_fix, beta_fix, a_fix, b_fix, c_fix);

    a = hls::ap_int<W_OUT>(a_fix);
    b = hls::ap_int<W_OUT>(b_fix);
    c = hls::ap_int<W_OUT>(c_fix);
}

}; // End of namespace motor
}; // End of namespace hls