## [`cordic`](../../include/hls_cordic.hpp)

## Table of Contents:

**Functions**

> [`cordic`](#function-cordic)

> [`cordic`](#function-cordic)

> [`hyp_cordic`](#function-hyp_cordic)

**Examples**

> [Examples](#examples)


### Function `cordic`
~~~lua
template <int SIZE, int MODE>
void cordic(ap_fixpt<unsigned int W_IN, int IW_IN> angle, ap_fixpt<unsigned int W_IN, int IW_IN> x, ap_fixpt<unsigned int W_IN, int IW_IN> y)
~~~

Runs the CORDIC algorithm using the circular coordinate system. See hls_sincos and hls_atan for examples of usage.

**Template Parameters:**

- `int SIZE`: number of desired CORDIC iterations. If SIZE is larger than the depth of the angle table, the depth of the angle table will be used for number of iterations instead.
- `int MODE`: variable to indicate vectoring mode (e.g. y -> 0) or rotating mode (e.g. angle -> 0). See hls_common.hpp for modes.

**Function Arguments:**

- `ap_fixpt<unsigned int W_IN, int IW_IN> angle`: input argument in radians
- `ap_fixpt<unsigned int W_IN, int IW_IN> x`: x-coordinate, will hold the resulting x-coordinate after CORDIC executes
- `ap_fixpt<unsigned int W_IN, int IW_IN> y`: y-coordinate, will hold the resulting y-coordinate after CORDIC executes

**Returns:**

No return.
### Function `cordic`
~~~lua
template <int SIZE>
void cordic(ap_fixpt<unsigned int W_IN, int IW_IN> angle, ap_fixpt<unsigned int W_IN, int IW_IN> x, ap_fixpt<unsigned int W_IN, int IW_IN> y, ap_fixpt<unsigned int W_IN, int IW_IN> desired_val)
~~~

Runs the CORDIC algorithm using the circular coordinate system. This version of CORDIC drives y to the fourth parameter, desired_val.See hls_asin for an example of usage.

**Template Parameters:**

- `int SIZE`: number of desired CORDIC iterations. If SIZE is larger than the depth of the angle table, the depth of the angle table will be used for number of iterations instead.

**Function Arguments:**

- `ap_fixpt<unsigned int W_IN, int IW_IN> angle`: input argument in radians
- `ap_fixpt<unsigned int W_IN, int IW_IN> x`: x-coordinate, will hold the resulting x-coordinate after CORDIC executes
- `ap_fixpt<unsigned int W_IN, int IW_IN> y`: y-coordinate, will hold the resulting y-coordinate after CORDIC executes
- `ap_fixpt<unsigned int W_IN, int IW_IN> desired_val`: value to drive y to

**Returns:**

No return.
### Function `hyp_cordic`
~~~lua
template <int SIZE, int MODE>
void hyp_cordic(ap_fixpt<unsigned int W_IN, int IW_IN> angle, ap_fixpt<unsigned int W_IN, int IW_IN> x, ap_fixpt<unsigned int W_IN, int IW_IN> y)
~~~

Runs the CORDIC algorithm. See hls_exp for an example of usage.NOTE: We use a gain of 0.82816, and a max angle of 1.11817.

**Template Parameters:**

- `int SIZE`: number of desired CORDIC iterations. If SIZE is larger than the depth of the angle table, the depth of the angle table will be used for number of iterations instead.
- `int MODE`: variable to indicate vectoring mode (e.g. y -> 0) or rotating mode (e.g. angle -> 0). See hls_common.hpp for modes.

**Function Arguments:**

- `ap_fixpt<unsigned int W_IN, int IW_IN> angle`: input argument in radians
- `ap_fixpt<unsigned int W_IN, int IW_IN> x`: x-coordinate, will hold the resulting x-coordinate after CORDIC executes
- `ap_fixpt<unsigned int W_IN, int IW_IN> y`: y-coordinate, will hold the resulting y-coordinate after CORDIC executes

**Returns:**

No return.
## Examples

~~~lua
  hls::ap_fixpt<32, 16> x = 0.60725293500888125616;

  hls::ap_fixpt<32, 16> y = 0;

  hls::ap_fixpt<32, 16> r = 3.14;

  hls::math::cordic<16, ROTATING>(r, x, y); 

  hls::math::cordic<16, VECTORING>(r, x, y);

~~~
~~~lua
  hls::ap_fixpt<32, 16> x = 0.60725293500888125616;

  hls::ap_fixpt<32, 16> y = 0;

  hls::ap_fixpt<32, 16> r = 3.14;

  hls::ap_fixpt<32, 16> dv = 3.14;

  hls::math::cordic<16>(r, x, y, dv);

~~~
~~~lua
  hls::ap_fixpt<32, 16> x = 0.82816;

  hls::ap_fixpt<32, 16> y = 0;

  hls::ap_fixpt<32, 16> r = 3.14;

  hls::math::cordic_hyp<16, ROTATING>(r, x, y);

  hls::math::cordic_hyp<16, VECTORING>(r, x, y);

~~~

