## [`ln`](../../include/hls_ln.hpp)

## Table of Contents:

**Functions**

> [`ln_lut`](#function-ln_lut)

> [`ln_cordic`](#function-ln_cordic)

**Examples**

> [Examples](#examples)

**Quality of Results**

> [Error Graph](#error-graph)

> [Resource Usage](#resource-usage)

### Function `ln_lut`
~~~lua
template <unsigned int W_OUT, int IW_OUT, unsigned int W_IN, int IW_IN>
ap_fixpt<unsigned int W_OUT, int IW_OUT> ln_lut(ap_fixpt<unsigned int W_IN, int IW_IN> num, int error)
~~~

Lookup Table based implementation of ln. Uses the lookup implementation of log2. If the input is negative, then an error will occur.



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

- `ap_fixpt<unsigned int W_OUT, int IW_OUT>`: natural logarithm of the input value
### Function `ln_cordic`
~~~lua
template <unsigned int W_OUT, int IW_OUT, int N_ITERATIONS, unsigned int W_IN, int IW_IN>
ap_fixpt<unsigned int W_OUT, int IW_OUT> ln_cordic(ap_fixpt<unsigned int W_IN, int IW_IN> num, int error)
~~~

CORDIC based implementation of ln. Uses the lookup implementation of log2. If the input is negative, then an error will occur.



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

- `ap_fixpt<unsigned int W_OUT, int IW_OUT>`: natural logarithm of the input value

## Examples

~~~lua
hls::ap_fixpt<10, 2> y = 2;
auto x = hls::math::ln_lut<10, 2>(y); //x will be an ap_fixpt w/ the value 0.69314718055995
~~~
~~~lua
hls::ap_fixpt<10, 2> y = 2;
auto x = hls::math::ln_cordic<10, 2, 16>(y); //x will be an ap_fixpt w/ the value 0.69314718055995
~~~

The example used to gather the following graph and resource report can be found [here](../../examples/simple/ln).

## Error Graph

![ln_D32_I8_S0.010000_L1.000000](<../graphs/ln_D32_I8_S0.010000_L1.000000_graph.png>)

![ln_D32_I16_S1.000000_L100.000000](<../graphs/ln_D32_I16_S1.000000_L100.000000_graph.png>)

## Resource Usage

Using MPF300


Input Plot Range: [0.01, 1.00]

Using W = 32, IW = 8, Q_M = AP_TRN, O_M = AP_WRAP



| Name      | Latency [cycles] (min/max/avg)   | II [cycles] (min/max/avg)   |   Avg Error |   Max Error |   LUTs |   DFFs |   DSPs |   LSRAM |   uSRAM | Estimated Frequency   |
|-----------|----------------------------------|-----------------------------|-------------|-------------|--------|--------|--------|---------|---------|-----------------------|
| ln_lut    | 105 / 106 / 105.01               | 1 / 2 / 1.01                |     1.9e-05 |     3.5e-05 |   2931 |   5638 |      5 |       1 |       1 | 368.596 MHz           |
| ln_cordic | 58 / 59 / 58.01                  | 1 / 2 / 1.01                |     2.9e-05 |     5.8e-05 |   3168 |   4141 |     11 |       0 |       4 | 339.905 MHz           |
| ln_cmath  | 186 / 186 / 186.00               | 28 / 28 / 28.00             |     0       |     0       |  16248 |  22506 |      9 |       6 |       0 | 300.030 MHz           |


Input Plot Range: [1.00, 100.00]

Using W = 32, IW = 16, Q_M = AP_TRN, O_M = AP_WRAP



| Name      | Latency [cycles] (min/max/avg)   | II [cycles] (min/max/avg)   |   Avg Error |   Max Error |   LUTs |   DFFs |   DSPs |   LSRAM |   uSRAM | Estimated Frequency   |
|-----------|----------------------------------|-----------------------------|-------------|-------------|--------|--------|--------|---------|---------|-----------------------|
| ln_lut    | 105 / 106 / 105.01               | 1 / 2 / 1.01                |    4e-05    |    0.000108 |   2639 |   4833 |      5 |       1 |       2 | 350.631 MHz           |
| ln_cordic | 55 / 56 / 55.01                  | 1 / 2 / 1.01                |    0.000162 |    0.000286 |   2955 |   3636 |     11 |       0 |       4 | 352.237 MHz           |
| ln_cmath  | 186 / 186 / 186.00               | 28 / 28 / 28.00             |    0        |    0        |  16248 |  22506 |      9 |       6 |       0 | 295.858 MHz           |

Notes:
- Targeted FMax was 400MHz.
- The standard C math library uses floating point numbers.


Back to [top](#).