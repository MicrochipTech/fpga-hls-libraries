#ifndef __DEFINE_H__
#define __DEFINE_H__

#include "hls/ap_int.hpp"
#include "hls/streaming.hpp"

// This line tests on a smaller image for faster co-simulation
#define FAST_COSIM

#ifdef FAST_COSIM
#define WIDTH 100
#define HEIGHT 56
#define GOLDEN_OUTPUT "pattern_gen_golden_100x56.png"
#else
#define WIDTH 1920
#define HEIGHT 1080
#define GOLDEN_OUTPUT "pattern_gen_golden.png"
#endif
#define SIZE (WIDTH * HEIGHT)

#endif
