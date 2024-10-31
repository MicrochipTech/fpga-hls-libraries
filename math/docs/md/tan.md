## [`tan`](../../include/hls_tan.hpp)

## Table of Contents:

**Functions**

> [`tan_cordic`](#function-tan_cordic)

> [`tan_taylor`](#function-tan_taylor)

> [`tan_lut`](#function-tan_lut)

**Examples**

> [Examples](#examples)

**Quality of Results**

> [Error Graph](#error-graph)

> [Resource Usage](#resource-usage)

### Function `tan_cordic`
~~~lua
template <unsigned int W_OUT, int IW_OUT, int N_ITERATIONS, unsigned int W_IN, int IW_IN>
ap_fixpt<unsigned int W_OUT, int IW_OUT> tan_cordic(ap_fixpt<unsigned int W_IN, int IW_IN> x, int error)
~~~

CORDIC implementation of tan. Uses sin_cordic and cos_cordic.



**Template Parameters:**

* `unsigned int W_OUT`: width of the output<br>
* `int IW_OUT`: width of integer portion of the output<br>
* `int N_ITERATIONS`: number of CORDIC iterations<br>
* `unsigned int W_IN`: width of the input (automatically inferred)<br>
* `int IW_IN`: width of integer portion of the input (automatically inferred)<br> <br>

**Function Arguments:**

* `ap_fixpt<unsigned int W_IN, int IW_IN> x`: angle (in radians)<br>
* `int error`: variable to hold error code value if an error occurs<br>

**Limitations:**

No limitations.

**Returns:**

- `ap_fixpt<unsigned int W_OUT, int IW_OUT>`: tangent of input angle
### Function `tan_taylor`
~~~lua
template <unsigned int W_OUT, int IW_OUT, unsigned int W_IN, int IW_IN>
ap_fixpt<unsigned int W_OUT, int IW_OUT> tan_taylor(ap_fixpt<unsigned int W_IN, int IW_IN> x, int error)
~~~

2-term Taylor Series implementation of tan based off sin_taylor and cos_taylor.



**Template Parameters:**

* `unsigned int W_OUT`: width of the output<br>
* `int IW_OUT`: width of integer portion of the output<br>
* `unsigned int W_IN`: width of the input (automatically inferred)<br>
* `int IW_IN`: width of integer portion of the input (automatically inferred)<br> <br>

**Function Arguments:**

* `ap_fixpt<unsigned int W_IN, int IW_IN> x`: angle (in radians)<br>
* `int error`: variable to hold error code value if an error occurs<br>

**Limitations:**

No limitations.

**Returns:**

- `ap_fixpt<unsigned int W_OUT, int IW_OUT>`: tangent of input angle
### Function `tan_lut`
~~~lua
template <unsigned int W_OUT, int IW_OUT, unsigned int W_IN, int IW_IN>
ap_fixpt<unsigned int W_OUT, int IW_OUT> tan_lut(ap_fixpt<unsigned int W_IN, int IW_IN> x)
~~~

Lookup Table implementation of tan based on sin_lut and cos_lut.
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

- `ap_fixpt<unsigned int W_OUT, int IW_OUT>`: tangent of input angle

## Examples

~~~lua
hls::ap_fixpt<10, 2> y = 3.1415;
auto x = hls::math::tan_cordic<10, 2, 16>(y); //x will be an ap_fixpt w/ the value 0
~~~
~~~lua
hls::ap_fixpt<10, 2> y = 3.1415;
auto x = hls::math::tan_taylor<10, 2>(y); //x will be an ap_fixpt w/ the value 0
~~~
~~~lua
hls::ap_fixpt<10, 2> y = 3.1415;
auto x = hls::math::tan_lut<10, 2>(y); //x will be an ap_fixpt w/ the value 0
~~~

The example used to gather the following graph and resource report can be found [here](../../examples/simple/tan).

## Error Graph

![tan_D32_I16_S-1.470796_L1.470796_N16](<../graphs/tan_D32_I16_S-1.470796_L1.470796_N16_graph.png>)

## Resource Usage

Using MPF300


Input Plot Range: [-1.47, 1.47]
Using N_ITER = 16

Using W = 32, IW = 16, Q_M = AP_TRN, O_M = AP_WRAP



| Name       | Latency [cycles] (min/max/avg)   | II [cycles] (min/max/avg)   |   Avg Error |   Max Error |   LUTs |   DFFs |   DSPs |   LSRAM |   uSRAM | Estimated Frequency   |
|------------|----------------------------------|-----------------------------|-------------|-------------|--------|--------|--------|---------|---------|-----------------------|
| tan_cordic | 100 / 101 / 100.02               | 1 / 2 / 1.02                |    0.001054 |    0.015253 |  10539 |  14952 |      3 |       1 |       3 | 321.647 MHz           |
| tan_taylor | 95 / 96 / 95.02                  | 1 / 2 / 1.02                |    0.002537 |    0.029693 |  13255 |  18826 |     51 |       1 |       3 | 323.834 MHz           |
| tan_lut    | 96 / 97 / 96.02                  | 1 / 2 / 1.02                |    0.037227 |    0.664322 |  11338 |  16377 |     22 |       1 |       0 | 288.268 MHz           |
| tan_cmath  | 211 / 474 / 334.15               | 211 / 474 / 332.69          |    0        |    0        |  32129 |  26039 |     11 |       1 |      27 | 130.839 MHz           |

Notes:
- Targeted FMax was 400MHz.
- The standard C math library uses floating point numbers.


Back to [top](#).