## [`log2`](../../include/hls_log2.hpp)

## Table of Contents:

**Functions**

> [`log2_lut`](#function-log2_lut)

> [`log2_cordic`](#function-log2_cordic)

**Examples**

> [Examples](#examples)

**Quality of Results**

> [Error Graph](#error-graph)

> [Resource Usage](#resource-usage)

### Function `log2_lut`
~~~lua
template <unsigned int W_OUT, int IW_OUT, unsigned int W_IN, int IW_IN>
ap_fixpt<unsigned int W_OUT, int IW_OUT> log2_lut(ap_fixpt<unsigned int W_IN, int IW_IN> num, int error)
~~~

Lookup Table based implementation of log base 2. If input is negative, then an error will occur.



**Template Parameters:**

* `unsigned int W_OUT`: width of the output<br>
* `int IW_OUT`: width of integer portion of the output<br>
* `unsigned int W_IN`: width of the input (automatically inferred)<br>
* `int IW_IN`: width of integer portion of the input (automatically inferred)<br> <br>

**Function Arguments:**

* `ap_fixpt<unsigned int W_IN, int IW_IN> num`: input<br>
* `int error`: variable to hold error code value if an error occurs<br>

**Limitations:**

No limitations.

**Returns:**

- `ap_fixpt<unsigned int W_OUT, int IW_OUT>`: log2 of input value
### Function `log2_cordic`
~~~lua
template <unsigned int W_OUT, int IW_OUT, int N_ITERATIONS, unsigned int W_IN, int IW_IN>
ap_fixpt<unsigned int W_OUT, int IW_OUT> log2_cordic(ap_fixpt<unsigned int W_IN, int IW_IN> num, int error)
~~~

CORDIC based implementation of log base 2. If input is negative, then an error will occur.



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

- `ap_fixpt<unsigned int W_OUT, int IW_OUT>`: log2 of input value

## Examples

~~~lua
hls::ap_fixpt<10, 2> y = 4;
auto x = hls::math::log2_lut<10, 2>(y); //x will be an ap_fixpt w/ the value 2
~~~
~~~lua
hls::ap_fixpt<10, 2> y = 4;
auto x = hls::math::log2_cordic<10, 2, 16>(y); //x will be an ap_fixpt w/ the value 2
~~~

The example used to gather the following graph and resource report can be found [here](../../examples/simple/log2).

## Error Graph

![log2_D32_I8_S0.010000_L1.000000](<../graphs/log2_D32_I8_S0.010000_L1.000000_graph.png>)

![log2_D32_I16_S1.000000_L100.000000](<../graphs/log2_D32_I16_S1.000000_L100.000000_graph.png>)

## Resource Usage

Using MPF300


Input Plot Range: [0.01, 1.00]

Using W = 32, IW = 8, Q_M = AP_TRN, O_M = AP_WRAP



| Name        | Latency [cycles] (min/max/avg)   | II [cycles] (min/max/avg)   |   Avg Error |   Max Error |   LUTs |   DFFs |   DSPs |   LSRAM |   uSRAM | Estimated Frequency   |
|-------------|----------------------------------|-----------------------------|-------------|-------------|--------|--------|--------|---------|---------|-----------------------|
| log2_lut    | 101 / 102 / 101.01               | 1 / 2 / 1.01                |     2.7e-05 |     5.1e-05 |   2797 |   5157 |      0 |       2 |       1 | 386.100 MHz           |
| log2_cordic | 54 / 55 / 54.01                  | 1 / 2 / 1.01                |     4.3e-05 |     8.3e-05 |   2986 |   3922 |      6 |       0 |       7 | 334.784 MHz           |
| log2_cmath  | 199 / 199 / 199.00               | 28 / 28 / 28.00             |     0       |     0       |  16455 |  22439 |      9 |       0 |      18 | 273.000 MHz           |


Input Plot Range: [1.00, 100.00]

Using W = 32, IW = 16, Q_M = AP_TRN, O_M = AP_WRAP



| Name        | Latency [cycles] (min/max/avg)   | II [cycles] (min/max/avg)   |   Avg Error |   Max Error |   LUTs |   DFFs |   DSPs |   LSRAM |   uSRAM | Estimated Frequency   |
|-------------|----------------------------------|-----------------------------|-------------|-------------|--------|--------|--------|---------|---------|-----------------------|
| log2_lut    | 101 / 102 / 101.01               | 1 / 2 / 1.01                |    5.8e-05  |    0.000163 |   2471 |   4649 |      0 |       2 |       2 | 384.320 MHz           |
| log2_cordic | 51 / 52 / 51.01                  | 1 / 2 / 1.01                |    0.000232 |    0.000405 |   2773 |   3400 |      6 |       0 |       7 | 360.360 MHz           |
| log2_cmath  | 199 / 199 / 199.00               | 28 / 28 / 28.00             |    0        |    0        |  16455 |  22439 |      9 |       0 |      18 | 282.326 MHz           |

Notes:
- Targeted FMax was 400MHz.
- The standard C math library uses floating point numbers.


Back to [top](#).