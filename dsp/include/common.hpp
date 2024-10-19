#pragma once


#include "hls/ap_int.hpp"
#include "hls/ap_fixpt.hpp"
#include <hls/streaming.hpp>
#include <hls/data_buffer.hpp>


namespace hls {
namespace dsp {

constexpr int W = 22;
constexpr int W_I = 16;

typedef ap_int<W_I> INT_TYPE;
typedef ap_fixpt<W,W_I> FLT_TYPE;


/***
 * @function log2
 * log2 implementation to produce the result at compile time.
 *
 * @param {unsigned long long} n: the number which the log2() will be computed
 * @param {int} p: the current power of 2, initialized to 0
 * @return {int} : the power of 2 for the input number n
 */
constexpr int log2(unsigned long long n, int p = 0) {
    return (n <= 1) ? p : log2(n / 2, p + 1);
}


/***
 * @function new_index
 * Reverses bits to obtain new index (for time decimation)
 *
 * @template {unsigned int} SIZE: the FFT transform size
 * @param {short} initial: the initial index value 
 * @return {short} : the final index value, which should be bit-reversed version of the initial index value
 */
template <unsigned int SIZE> 
short new_index(short initial) {
  if (initial == 0 || initial == SIZE - 1)
    return initial;
  constexpr int NUMBER_OF_SHIFTS = log2(SIZE);
  short i, final = 0;
  #pragma HLS loop pipeline
  for(i=NUMBER_OF_SHIFTS-1;i>=0;i--) {
    final += (initial & 1) << i;
    initial = initial >> 1;
  }
  return final;
}

} //namespace dsp
} //namespace hls

