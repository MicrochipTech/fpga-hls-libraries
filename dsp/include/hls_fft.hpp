// ©2025 Microchip Technology Inc. and its subsidiaries
//
// Subject to your compliance with these terms, you may use this Microchip
// software and any derivatives exclusively with Microchip products. You are
// responsible for complying with third party license terms applicable to your
// use of third party software (including open source software) that may
// accompany this Microchip software. SOFTWARE IS “AS IS.” NO WARRANTIES,
// WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING
// ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR
// A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY
// INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST
// OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED,
// EVEN IF MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE
// FORESEEABLE.  TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP’S TOTAL
// LIABILITY ON ALL CLAIMS LATED TO THE SOFTWARE WILL NOT EXCEED AMOUNT OF
// FEES, IF ANY, YOU PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE. MICROCHIP
// OFFERS NO SUPPORT FOR THE SOFTWARE. YOU MAY CONTACT MICROCHIP AT
// https://www.microchip.com/en-us/support-and-training/design-help/client-support-services
// TO INQUIRE ABOUT SUPPORT SERVICES AND APPLICABLE FEES, IF AVAILABLE.
//
// In-place FFT implementation is referenced from https://github.com/KastnerRG/pp4fpgas/tree/master
// Released under CC-BY-4.0 license. To see full license, see licenses folder in fft_license.

#pragma once


#include "common.hpp"



namespace hls {
namespace dsp {

// define the type of FFT data points
struct fft_data_t {
  ap_int<16> im;
  ap_int<16> re;
};

#include "twiddle.h"

template <unsigned int SIZE> 
void fft_in_place(hls::FIFO<fft_data_t> &in, hls::FIFO<fft_data_t> &out) {

	FLT_TYPE temp_R; // temporary storage complex variable
	FLT_TYPE temp_I; // temporary storage complex variable
  ap_fixpt<17,2> c, s;
  FLT_TYPE x_r_lower, x_i_lower, x_r, x_i, t00, t11;
	INT_TYPE i, j, k;	// loop indexes
	INT_TYPE i_lower;	// Index of lower point in butterfly
	INT_TYPE stage, DFTpts;
	INT_TYPE numBF;			// Butterfly Width
	INT_TYPE step = SIZE >> 1; // step=N>>1
  INT_TYPE cnt;

  // compute the number of stages needed based on the FFT size
  constexpr int NUMBER_OF_STAGES = log2(SIZE);

  // memory layout of Stage:
  // 0 - 255: Re part
  // 256 - 511: Im part
  hls::DoubleBuffer<FLT_TYPE[SIZE << 1]> Stage;
  auto Stage_P = Stage.producer();
  auto Stage_C = Stage.consumer();

  auto butterfly_loop_body = [&](int i, int k) {
        
        c = twiddleRe[k];
        s = twiddleIm[k];
        i_lower = i + numBF; // index of lower point in butterfly
        x_r_lower = Stage_C[i_lower] >> 1;
        x_i_lower = Stage_C[i_lower + SIZE] >> 1;
        x_r = Stage_C[i] >> 1;
        x_i = Stage_C[i + SIZE] >> 1;
        
        // apply the the Karatsuba pattern
        t00 = x_r_lower * c;
        t11 = x_i_lower * s;
        temp_R = t00 - t11;
        temp_I = (x_r_lower - x_i_lower) * (s - c) + t00 + t11;
        Stage_P[i_lower] = x_r - int(temp_R);
        Stage_P[i_lower + SIZE] = x_i - int(temp_I);
        Stage_P[i] = x_r + int(temp_R);
        Stage_P[i + SIZE] = x_i + int(temp_I);

  };

  Stage.producer_acquire();
  #pragma HLS loop pipeline
  for (unsigned i = 0; i < SIZE; i++) {
      // Decimation in Time, reverse bits to obtain new indices and swap elements
      int j = new_index<SIZE>(i);
      auto data = in.read();
      Stage_P[j] = data.re; // Re part
      Stage_P[j + SIZE] = data.im; // Im part
  }
  Stage.producer_release();

stage_loop:
	for (stage = 1; stage <= NUMBER_OF_STAGES; stage++) { // Do M stages of butterflies
      Stage.consumer_acquire();
      Stage.producer_acquire();

      DFTpts = 1 << stage;								 // DFT = 2^stage = points in sub DFT
      numBF = DFTpts / 2;									 // Butterfly WIDTHS in sub-DFT
      k = 0;
      j = numBF;
      i = 0;
      cnt = 0;
      
	// Perform butterflies for j-th stage
	butterfly_loop:
  #pragma HLS loop pipeline
  for(unsigned z = 0; z < SIZE/2; z++) {
      if(j == 0) {
        break;
      }
      butterfly_loop_body(i, k);
      i += DFTpts;

      if(i >= SIZE) {
        j -= 1;
        i = ++cnt;
        k += step;
      }
  }

		step = step / 2;
    Stage.consumer_release();
    Stage.producer_release();
	}

  Stage.consumer_acquire();
 #pragma HLS loop pipeline
  for (unsigned i = 0; i < SIZE; i++) {
    fft_data_t data;
    data.re = Stage_C[i];
    data.im = Stage_C[i + SIZE];
    out.write(data);
  }
  Stage.consumer_release();

}

/***
 * @function fft
 * Compute the FFT. Note that fft_data_t type is defined as:
 * ```cpp
 * struct fft_data_t {
 * ap_int<16> im;
 * ap_int<16> re;
 * };
 * ```
 * ![timing_diagram](../graphs/in-place-fft-timing.PNG)
 * @param {hls::FIFO<fft_data_t>&} fifo_in reference to the input fifo, where the depth must match the FFT size
 * @param {hls::FIFO<fft_data_t>&} fifo_out reference to the output fifo, where the depth must match the FFT size
 * @template {unsigned} SIZE the FFT transform size
 * @limitation The fft function currently only supports the radix-2, 256-point, forward inplace FFT implementation.
 * @example
 * hls::dsp::fft<SIZE>(fifo_in, fifo_out);
 */
template <unsigned SIZE> 
void fft(hls::FIFO<fft_data_t> &fifo_in, hls::FIFO<fft_data_t> &fifo_out) {
    static_assert(SIZE == 256, "FFT size is not 256!");
    fft_in_place<SIZE>(fifo_in, fifo_out);
    return;
}

} // namespace dsp
} // namespace hls
