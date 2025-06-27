## [`sin`](../../include/hls_sin.hpp)

## Table of Contents:

**Functions**

> [`sin_cordic`](#function-sin_cordic)

> [`sin_taylor`](#function-sin_taylor)

> [`sin_lut`](#function-sin_lut)

**Examples**

> [Examples](#examples)

**Quality of Results**

> [Error Graph](#error-graph)

> [Resource Usage](#resource-usage)

### Function `sin_cordic`
~~~lua
template <unsigned int W_OUT, int IW_OUT, int N_ITERATIONS, unsigned int W_IN, int IW_IN>
ap_fixpt<unsigned int W_OUT, int IW_OUT> sin_cordic(ap_fixpt<unsigned int W_IN, int IW_IN> x)
~~~

CORDIC implementation of sin.



**Template Parameters:**

* `unsigned int W_OUT`: width of the output<br>
* `int IW_OUT`: width of integer portion of the output<br>
* `int N_ITERATIONS`: number of CORDIC iterations<br>
* `unsigned int W_IN`: width of the input (automatically inferred)<br>
* `int IW_IN`: width of integer portion of the input (automatically inferred)<br> <br>

**Function Arguments:**

* `ap_fixpt<unsigned int W_IN, int IW_IN> x`: angle (in radians)<br>

**Limitations:**

No limitations.

**Returns:**

- `ap_fixpt<unsigned int W_OUT, int IW_OUT>`: sine of input angle
### Function `sin_taylor`
~~~lua
template <unsigned int W_OUT, int IW_OUT, unsigned int W_IN, int IW_IN>
ap_fixpt<unsigned int W_OUT, int IW_OUT> sin_taylor(ap_fixpt<unsigned int W_IN, int IW_IN> x)
~~~

2-term Taylor Series implementation of sin.



**Template Parameters:**

* `unsigned int W_OUT`: width of the output<br>
* `int IW_OUT`: width of integer portion of the output<br>
* `unsigned int W_IN`: width of the input (automatically inferred)<br>
* `int IW_IN`: width of integer portion of the input (automatically inferred)<br> <br>

**Function Arguments:**

* `ap_fixpt<unsigned int W_IN, int IW_IN> x`: angle (in radians)<br>

**Limitations:**

No limitations.

**Returns:**

- `ap_fixpt<unsigned int W_OUT, int IW_OUT>`: sine of input angle
### Function `sin_lut`
~~~lua
template <unsigned int W_OUT, int IW_OUT, unsigned int W_IN, int IW_IN>
ap_fixpt<unsigned int W_OUT, int IW_OUT> sin_lut(ap_fixpt<unsigned int W_IN, int IW_IN> x)
~~~

Lookup Table implementation of sin.
Number of decimal bits of input value is recommended to be less than DECIM (defined in utils/generators/generated_tables/sin_lut_table.hpp)



**Template Parameters:**

* `unsigned int W_OUT`: width of the output<br>
* `int IW_OUT`: width of integer portion of the output<br>
* `unsigned int W_IN`: width of the input (automatically inferred)<br>
* `int IW_IN`: width of integer portion of the input (automatically inferred)<br> <br>See utils/generators/sin_lut_gentable.cpp to generate your own tables. <br>

**Function Arguments:**

* `ap_fixpt<unsigned int W_IN, int IW_IN> x`: angle (in radians)<br>

**Limitations:**

No limitations.

**Returns:**

- `ap_fixpt<unsigned int W_OUT, int IW_OUT>`: sine of input angle

## Examples

~~~lua
hls::ap_fixpt<10, 2> y = 3.1415;
auto x = hls::math::sin_cordic<10, 2, 16>(y); //x will be an ap_fixpt w/ the value 0
~~~
~~~lua
hls::ap_fixpt<10, 2> y = 3.1415;
auto x = hls::math::sin_taylor<10, 2>(y); //x will be an ap_fixpt w/ the value 0
~~~
~~~lua
hls::ap_fixpt<10, 2> y = 3.1415;
auto x = hls::math::sin_lut<10, 2>(y); //x will be an ap_fixpt w/ the value 0
~~~

The example used to gather the following graph and resource report can be found [here](../../examples/simple/sin).

## Error Graph

![sin_D32_I16_S-6.283185_L6.283185_N16](<../graphs/sin_D32_I16_S-6.283185_L6.283185_N16_graph.png>)

## Resource Usage

Using MPF300


Input Plot Range: [-12.57, 12.57]
Using N_ITER = 16

Using W = 32, IW = 16, Q_M = AP_TRN, O_M = AP_WRAP



| Name       | Latency [cycles] (min/max/avg)   | II [cycles] (min/max/avg)   |   Avg Error |   Max Error |   LUTs |   DFFs |   DSPs |   LSRAM |   uSRAM | Estimated Frequency   |
|------------|----------------------------------|-----------------------------|-------------|-------------|--------|--------|--------|---------|---------|-----------------------|
| sin_cordic | 45 / 46 / 45.00                  | 1 / 2 / 1.00                |    8.5e-05  |    0.000292 |   1853 |   4442 |      0 |       0 |       0 | 540.249 MHz           |
| sin_taylor | 38 / 39 / 38.00                  | 1 / 2 / 1.00                |    0.000564 |    0.004389 |   2610 |   4556 |      9 |       0 |       0 | 423.729 MHz           |
| sin_lut    | 40 / 41 / 40.00                  | 1 / 2 / 1.00                |    0.002583 |    0.007718 |   1875 |   3952 |      4 |       0 |       0 | 394.011 MHz           |
| sin_cmath  | 138 / 295 / 256.94               | 138 / 295 / 256.94          |    0        |    0        |  22740 |  19571 |      9 |       1 |      27 | 194.212 MHz           |

Notes:
- Targeted FMax was 400MHz.
- The standard C math library uses floating point numbers.


Back to [top](#).