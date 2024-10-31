## [`exp`](../../include/hls_exp.hpp)

## Table of Contents:

**Functions**

> [`exp_taylor`](#function-exp_taylor)

> [`exp_cordic`](#function-exp_cordic)

**Examples**

> [Examples](#examples)

**Quality of Results**

> [Error Graph](#error-graph)

> [Resource Usage](#resource-usage)

### Function `exp_taylor`
~~~lua
template <unsigned int W_OUT, int IW_OUT, unsigned int W_IN, int IW_IN>
ap_fixpt<unsigned int W_OUT, int IW_OUT> exp_taylor(ap_fixpt<unsigned int W_IN, int IW_IN> fp)
~~~

6-term Taylor Series implementation of exp.



**Template Parameters:**

* `unsigned int W_OUT`: width of the output<br>
* `int IW_OUT`: width of integer portion of the output<br>
* `unsigned int W_IN`: width of the input (automatically inferred)<br>
* `int IW_IN`: width of integer portion of the input (automatically inferred)<br> <br>

**Function Arguments:**

* `ap_fixpt<unsigned int W_IN, int IW_IN> fp`: input<br>

**Limitations:**

No limitations.

**Returns:**

- `ap_fixpt<unsigned int W_OUT, int IW_OUT>`: e raised to the power of the input
### Function `exp_cordic`
~~~lua
template <unsigned int W_OUT, int IW_OUT, int N_ITERATIONS, unsigned int W_IN, int IW_IN>
ap_fixpt<unsigned int W_OUT, int IW_OUT> exp_cordic(ap_fixpt<unsigned int W_IN, int IW_IN> fp)
~~~


CORDIC implementation of exp.
Takes in x as a fixed point number, returns exp(x).



**Template Parameters:**

* `unsigned int W_OUT`: width of the output<br>
* `int IW_OUT`: width of integer portion of the output<br>
* `int N_ITERATIONS`: number of CORDIC iterations<br>
* `unsigned int W_IN`: width of the input (automatically inferred)<br>
* `int IW_IN`: width of integer portion of the input (automatically inferred)<br> <br>

**Function Arguments:**

* `ap_fixpt<unsigned int W_IN, int IW_IN> fp`: Input<br>

**Limitations:**

No limitations.

**Returns:**

- `ap_fixpt<unsigned int W_OUT, int IW_OUT>`: e raised to the power of the input

## Examples

~~~lua
hls::ap_fixpt<10, 2> y = 2;
auto x = hls::math::exp_taylor<10, 2>(y); //x will be an ap_fixpt w/ the value 7.3890560989
~~~
~~~lua
hls::ap_fixpt<10, 2> y = 2;
auto x = hls::math::exp_cordic<10, 2, 16>(y); //x will be an ap_fixpt w/ the value 7.3890560989
~~~

The example used to gather the following graph and resource report can be found [here](../../examples/simple/exp).

## Error Graph

![exp_D32_I8_S-10.000000_L-2.000000_N16](<../graphs/exp_D32_I8_S-10.000000_L-2.000000_N16_graph.png>)

![exp_D32_I16_S-2.000000_L3.000000_N16](<../graphs/exp_D32_I16_S-2.000000_L3.000000_N16_graph.png>)

## Resource Usage

Using MPF300


Input Plot Range: [-10.00, -2.00]
Using N_ITER = 16

Using W = 32, IW = 8, Q_M = AP_TRN, O_M = AP_WRAP



| Name       | Latency [cycles] (min/max/avg)   | II [cycles] (min/max/avg)   |   Avg Error |   Max Error |   LUTs |   DFFs |   DSPs |   LSRAM |   uSRAM | Estimated Frequency   |
|------------|----------------------------------|-----------------------------|-------------|-------------|--------|--------|--------|---------|---------|-----------------------|
| exp_taylor | 38 / 39 / 38.00                  | 1 / 2 / 1.00                |       0     |     3e-06   |   2906 |   4215 |     35 |       0 |      12 | 322.789 MHz           |
| exp_cordic | 55 / 56 / 55.00                  | 1 / 2 / 1.00                |       8e-06 |     1.7e-05 |   3360 |   7290 |     10 |       0 |       8 | 386.548 MHz           |
| exp_cmath  | 253 / 253 / 253.00               | 19 / 19 / 19.00             |       0     |     0       |   8230 |  14574 |      9 |       4 |       0 | 338.868 MHz           |


Input Plot Range: [-2.00, 3.00]
Using N_ITER = 16

Using W = 32, IW = 16, Q_M = AP_TRN, O_M = AP_WRAP



| Name       | Latency [cycles] (min/max/avg)   | II [cycles] (min/max/avg)   |   Avg Error |   Max Error |   LUTs |   DFFs |   DSPs |   LSRAM |   uSRAM | Estimated Frequency   |
|------------|----------------------------------|-----------------------------|-------------|-------------|--------|--------|--------|---------|---------|-----------------------|
| exp_taylor | 36 / 37 / 36.00                  | 1 / 2 / 1.00                |    7.9e-05  |    0.000664 |   2309 |   3290 |     26 |       0 |       8 | 365.230 MHz           |
| exp_cordic | 55 / 56 / 55.00                  | 1 / 2 / 1.00                |    0.000339 |    0.003017 |   3221 |   7169 |     10 |       0 |       8 | 378.072 MHz           |
| exp_cmath  | 253 / 253 / 253.00               | 19 / 19 / 19.00             |    0        |    0        |   8230 |  14574 |      9 |       4 |       0 | 317.158 MHz           |

Notes:
- Targeted FMax was 400MHz.
- The standard C math library uses floating point numbers.


Back to [top](#).