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
void clarke_direct_impl(
                hls::ap_fixpt<W_IN, IW_IN>  a,
                hls::ap_fixpt<W_IN, IW_IN>  b,
                hls::ap_fixpt<W_OUT, IW_OUT> &alpha,
                hls::ap_fixpt<W_OUT, IW_OUT> &beta) {

    #pragma HLS function inline
    
    const hls::ap_fixpt<W_OUT, 1> Kd = 0.57735026918962576451;

    alpha = a;
    beta = Kd * (a + 2 * b);
}


/**
 * @function clarke_direct
 * Clarke (direct) transform (3φ → αβ), amplitude-invariant, fixed-point.
 *
 * Converts phase currents to the stationary (α, β) frame assuming a balanced
 * three-phase system where Ic = −(Ia + Ib). With that assumption, the
 * amplitude-invariant Clarke reduces to:
 *
 *   $$alpha = I_{a}$$
 *   $$beta  = \frac{(I_{a} + 2*I_{b})}{\sqrt{3}}$$
 * 
 * @template {unsigned int} W_OUT width of the output
 * @template {int} IW_OUT width of integer portion of the output
 * @template {unsigned int} W_IN width of the input (automatically inferred)
 * @template {int} IW_IN width of integer portion of the input (automatically inferred)
 *
 * @param {ap_fixpt<unsigned int W_IN, int IW_IN>} I_a Phase-A current.
 * @param {ap_fixpt<unsigned int W_IN, int IW_IN>} I_b Phase-B current.
 * @param {ap_fixpt<unsigned int W_OUT, int IW_OUT>} I_alpha Stationary-frame α component.
 * @param {ap_fixpt<unsigned int W_OUT, int IW_OUT>} I_beta Stationary-frame β component.
 *
 * @example
 * using fixpt_t = hls::ap_fixpt<18,4>;
 * fixpt_t alpha, beta;
 * fixpt_t a = -0.5;
 * fixpt_t b = 0.5 * 0.8660254; 
 * hls::motor::clarke_direct(alpha, beta, a, b)
 */
template <unsigned int W_OUT, int IW_OUT, unsigned int W_IN, int IW_IN>
void clarke_direct(
                hls::ap_fixpt<W_IN, IW_IN>  a,
                hls::ap_fixpt<W_IN, IW_IN>  b,
                hls::ap_fixpt<W_OUT, IW_OUT> &alpha,
                hls::ap_fixpt<W_OUT, IW_OUT> &beta) {

    #pragma HLS function pipeline
    clarke_direct_impl<W_OUT, IW_OUT, W_IN, IW_IN>(a, b, alpha, beta);
}

/**
 * @function clarke_direct
 * Clarke (direct) transform (3φ → αβ), amplitude-invariant, ap_int type.
 *
 * Converts phase currents to the stationary (α, β) frame assuming a balanced
 * three-phase system where Ic = −(Ia + Ib). With that assumption, the
 * amplitude-invariant Clarke reduces to:
 *
 *   $$alpha = I_{a}$$
 *   $$beta  = \frac{(I_{a} + 2*I_{b})}{\sqrt{3}}$$
 * 
 * @template {unsigned int} W_OUT width of the output
 * @template {int} IW_OUT width of integer portion of the output
 * @template {unsigned int} W_IN width of the input (automatically inferred)
 * @template {int} IW_IN width of integer portion of the input (automatically inferred)
 *
 * @param {ap_int<unsigned int W_IN>} I_a Phase-A current.
 * @param I_b {ap_int<unsigned int W_IN>} I_b Phase-B current.
 * @param I_alpha {ap_int<unsigned int W_OUT>} Stationary-frame α component.
 * @param I_beta {ap_int<unsigned int W_OUT>} Stationary-frame β component.
 *
 * @example
 * hls::ap_int<18> alpha, beta;
 * hls::ap_int<18> a = 14555;
 * hls::ap_int<18> b = 45; 
 * hls::motor::clarke_direct(alpha, beta, a, b)
 */
template <unsigned int W_OUT, unsigned int W_IN>
void clarke_direct(hls::ap_int<W_IN>  a,
                    hls::ap_int<W_IN>  b,
                    hls::ap_int<W_OUT> &alpha,
                    hls::ap_int<W_OUT> &beta) {

    #pragma HLS function pipeline
    hls::ap_fixpt<W_IN, W_IN> a_fix, b_fix;
    hls::ap_fixpt<W_OUT, W_OUT> alpha_fix, beta_fix;
    a_fix = a;
    b_fix = b;

    clarke_direct_impl <W_OUT, W_OUT, W_IN, W_IN>(a_fix, b_fix, alpha_fix, beta_fix);

    alpha = hls::ap_int<W_OUT>(alpha_fix);
    beta = hls::ap_int<W_OUT>(beta_fix);
}

}; // End of namespace motor
}; // End of namespace hls
