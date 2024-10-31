## [`sincos`](../../include/hls_sincos.hpp)

## Table of Contents:

**Functions**

> [`sincos_cordic`](#function-sincos_cordic)

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

* `unsigned int W_OUT`: width of the output<br>
* `int IW_OUT`: width of integer portion of the output<br>
* `int N_ITERATIONS`: number of CORDIC iterations<br>
* `unsigned int W_IN`: width of the input (automatically inferred)<br>
* `int IW_IN`: width of integer portion of the input (automatically inferred)<br> <br>

**Function Arguments:**

* `ap_fixpt<W_IN, IW_IN> desired_angle`: angle (in radians)<br>
* `ap_fixpt<W_OUT, IW_OUT> sin`: variable that will hold the value of sine after the function executes<br>
* `ap_fixpt<W_OUT, IW_OUT> cos`: variable that will hold the value of cosine after the function executes<br>

**Limitations:**

No limitations.

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

## Error Graph

## Resource Usage

Using MPF300


Input Plot Range: [-12.57, 12.57]
Using N_ITER = 16

Using W = 32, IW = 16, Q_M = AP_TRN, O_M = AP_WRAP



| Name          | Latency [cycles] (min/max/avg)   | II [cycles] (min/max/avg)   |   Avg Error |   Max Error |   LUTs |   DFFs |   DSPs |   LSRAM |   uSRAM | Estimated Frequency   |
|---------------|----------------------------------|-----------------------------|-------------|-------------|--------|--------|--------|---------|---------|-----------------------|
| sincos_cmath  | 304 / 595 / 538.44               | 304 / 595 / 538.45          |     0       |    0        |  54318 |  28888 |     20 |       0 |      27 | 130.685 MHz           |
| sincos_cordic | 35 / 36 / 35.00                  | 1 / 2 / 1.00                |     8.5e-05 |    0.000302 |   1725 |   3854 |      3 |       0 |       0 | 385.654 MHz           |

Notes:
- Targeted FMax was 400MHz.
- The standard C math library uses floating point numbers.


Back to [top](#).