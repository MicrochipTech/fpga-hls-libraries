## [`asin`](../../include/hls_asin.hpp)

## Table of Contents:

**Functions**

> [`asin_cordic`](#function-asin_cordic)

**Examples**

> [Examples](#examples)

**Quality of Results**

> [Error Graph](#error-graph)

> [Resource Usage](#resource-usage)

### Function `asin_cordic`
~~~lua
template <unsigned int W_OUT, int IW_OUT, int N_ITERATIONS, unsigned int W_IN, int IW_IN>
ap_fixpt<unsigned int W_OUT, int IW_OUT> asin_cordic(ap_fixpt<unsigned int W_IN, int IW_IN> num, int error)
~~~

CORDIC implementation of arcsin. Note that accuracy drops as num -> +/-1.
If the input is not within [-1, 1], then an error will occur.



**Template Parameters:**

* `unsigned int W_OUT`: width of the output<br>
* `int IW_OUT`: width of integer portion of the output<br>
* `int N_ITERATIONS`: number of CORDIC iterations<br>
* `unsigned int W_IN`: width of the input (automatically inferred)<br>
* `int IW_IN`: width of integer portion of the input (automatically inferred)<br> <br>

**Function Arguments:**

* `ap_fixpt<unsigned int W_IN, int IW_IN> num`: input<br>
* `int error`: variable to hold error code value if an error occurs<br>

**Limitations:**

No limitations.

**Returns:**

- `ap_fixpt<unsigned int W_OUT, int IW_OUT>`: arcsine of input value (in radians)

## Examples

~~~lua
hls::ap_fixpt<10, 2> y = 0.707;
auto x = hls::math::asin_cordic<10, 2, 16>(y); // x will be an ap_fixpt number with the value 0.785247163
~~~

The example used to gather the following graph and resource report can be found [here](../../examples/simple/asin).

## Error Graph

![asin_D32_I8_S-1.000000_L1.000000_N16](<../graphs/asin_D32_I8_S-1.000000_L1.000000_N16_graph.png>)

## Resource Usage

Using MPF300


Input Plot Range: [-1.00, 1.00]
Using N_ITER = 16

Using W = 32, IW = 8, Q_M = AP_TRN, O_M = AP_WRAP



| Name        | Latency [cycles] (min/max/avg)   | II [cycles] (min/max/avg)   |   Avg Error |   Max Error |   LUTs |   DFFs |   DSPs |   LSRAM |   uSRAM | Estimated Frequency   |
|-------------|----------------------------------|-----------------------------|-------------|-------------|--------|--------|--------|---------|---------|-----------------------|
| asin_cordic | 34 / 35 / 34.00                  | 1 / 2 / 1.00                |    0.007499 |    0.172239 |   2781 |   6817 |      0 |       0 |       7 | 302.847 MHz           |
| asin_cmath  | 22 / 519 / 343.46                | 22 / 519 / 342.81           |    0        |    0        |  17586 |  21292 |      9 |       0 |       0 | 233.155 MHz           |

Notes:
- Targeted FMax was 400MHz.
- The standard C math library uses floating point numbers.


Back to [top](#).