/**
 * This file contains all the numberic constants included in math.h. Some functions (e.g. sin_cordic)
 * require use of math constants. We've included them here so that including math.h in your
 * design is not necessary.
*/

namespace hls{
namespace math{
#ifndef M_PI
const float M_PI = 3.14159265358979323846;
#endif
#ifndef M_PI_2
const float M_PI_2 = 1.57079632679489661923;
#endif
#ifndef M_PI_4
const float M_PI_4 = 0.785398163397448309616;
#endif
#ifndef M_1_PI
const float M_1_PI = 0.318309886183790671538;
#endif
#ifndef M_2_PI
const float M_2_PI = 0.636619772367581343076;
#endif
#ifndef M_2_SQRTPI
const float M_2_SQRTPI = 1.12837916709551257390;
#endif
#ifndef M_SQRT2
const float M_SQRT2 = 1.41421356237309504880;
#endif
#ifndef M_SQRT1_2
const float M_SQRT1_2 = 0.707106781186547524401;
#endif
#ifndef M_E
const float M_E = 2.71828182845904523536;
#endif
#ifndef M_LOG2E
const float M_LOG2E = 1.44269504088896340736;
#endif
#ifndef M_LOG10E
const float M_LOG10E = 0.434294481903251827651;
#endif
#ifndef M_LN2
const float M_LN2 = 0.693147180559945309417;
#endif
#ifndef M_LN10
const float M_LN10 = 2.30258509299404568402;
#endif
}
}
