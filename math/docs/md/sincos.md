## [`sincos`](../../include/hls_sincos.hpp)

## Table of Contents:

**Functions**

> [`sincos`](#function-sincos)

**Examples**

> [Examples](#examples)

**Quality of Results**

> [Resource Usage](#resource-usage)


### Function `sincos`
~~~lua
template <unsigned int W_OUT, int IW_OUT, int N_ITERATIONS, unsigned int W_IN, int IW_IN>
void sincos(ap_fixpt<W_IN, IW_IN> desired_angle, ap_fixpt<W_OUT, IW_OUT> sin, ap_fixpt<W_OUT, IW_OUT> cos)
~~~

CORDIC implementation of sincos.

**Template Parameters:**

- `unsigned int W_OUT`: width of the output
- `int IW_OUT`: width of integer portion of the output
- `int N_ITERATIONS`: number of CORDIC iterations
- `unsigned int W_IN`: width of the input (automatically inferred)
- `int IW_IN`: width of integer portion of the input (automatically inferred)

**Function Arguments:**

- `ap_fixpt<W_IN, IW_IN> desired_angle`: angle (in radians)
- `ap_fixpt<W_OUT, IW_OUT> sin`: variable that will hold the value of sine after the function executes
- `ap_fixpt<W_OUT, IW_OUT> cos`: variable that will hold the value of cosine after the function executes

**Returns:**

No return.
## Examples

~~~lua
  hls::ap_fixpt<16, 2> sin = 0;

  hls::ap_fixpt<16, 2> cos = 0; 

  hls::ap_fixpt<16, 2> x = 3.14; 

  hls::math::sincos<16, 32, 16>(x, sin, cos) 

~~~

The example used to gather the following graph and resource report can be found [here](../../examples/simple/sincos).

## Resource Usage

Using MPF300

Input Plot Range: [-12.57, 12.57]
Using N_ITER = 16
Using W = 32, IW = 16, Q_M = AP_TRN, O_M = AP_WRAP

| Name          | Latency [cycles] (min/max/avg)   | II [cycles] (min/max/avg)   |   Avg Error |   Max Error |   LUTs |   DFFs |   DSPs |   LSRAM |   uSRAM | Estimated Frequency   |
|---------------|----------------------------------|-----------------------------|-------------|-------------|--------|--------|--------|---------|---------|-----------------------|
| sincos_cmath  | 305 / 597 / 540.06               | 305 / 597 / 540.06          |     0       |    0        |  50930 |  34536 |     29 |       5 |      21 | 538.503 MHz           |
| sincos_cordic | 22 / 23 / 22.00                  | 1 / 2 / 1.00                |     8.5e-05 |    0.000302 |   1093 |   1133 |      3 |       0 |       0 | 538.503 MHz           |

Notes:
- The standard C math library uses floating point numbers.
- FMax is displayed as reported after RTL synthesis and may change during place and route.
- Targeted FMax was 400MHz.


Back to [top](#).