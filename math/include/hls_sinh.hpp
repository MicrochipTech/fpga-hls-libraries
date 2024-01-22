#pragma once
#include "hls/ap_fixpt.hpp"
#include "hls_common.hpp"
#include "hls/ap_int.hpp"
#include "hls_cordic.hpp"
#include <cstdio>
#include <cmath>
using hls::ap_fixpt;
using hls::ap_ufixpt;
using hls::ap_uint;
namespace hls{
namespace math{

template <unsigned int W_IN, int IW_IN, int N, unsigned int W_OUT, int IW_OUT>
void sinh(ap_fixpt<W_IN, IW_IN> desired_angle, ap_fixpt<W_OUT, IW_OUT>& sin, ap_fixpt<W_OUT, IW_OUT>& cos) {
  typedef hls::ap_fixpt<W_IN, IW_IN> T;
  T x = 1.20749613601;
  T y = 0;
  T angle(0);
DBG_CODE{printf("abs_in: %lf\n\n", desired_angle.to_double());}
  // First get angle to be positive:
  auto abs_in = desired_angle[W_IN - 1] ? T(-desired_angle) : (desired_angle);

  DBG_CODE{printf("abs_in: %lf\n\n", abs_in.to_double());}
  cordic_hyp<N>(angle, abs_in, angle, x, y);

  sin = y;
  cos = x;
  if (desired_angle > 0)
    sin = -sin;

//   printf("ANGLE: %f: COS = %f, SIN = %f\n\n", desired_angle.to_double(),
//   x.to_double(), y.to_double());
}
}
}
