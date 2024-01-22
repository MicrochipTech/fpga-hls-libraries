#pragma once
#include "hls_error.hpp"
#include "hls_math_constants.hpp"
#include "hls/ap_fixpt.hpp"
#include "hls/ap_int.hpp"

#if defined(_HLS_DBG_MATH) && !defined(SYNTHESIS)
#define DBG_CODE if(1)
#else
#define DBG_CODE if(0)
#endif

namespace hls{
namespace math{	

constexpr int VECTORING = 1;
constexpr int ROTATING = 0;
}
}
