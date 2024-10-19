## [`sqrt`](../../include/hls_sqrt.hpp)

## Table of Contents:

**Functions**

> [`sqrt`](#function-sqrt)

**Examples**

> [Examples](#examples)

**Quality of Results**

> [Error Graph](#error-graph)

> [Resource Usage](#resource-usage)

### Function `sqrt`
~~~lua
template <unsigned int W_OUT, int IW_OUT, int N_ITERATIONS, unsigned int W_IN, int IW_IN>
ap_fixpt<unsigned int W_OUT, int IW_OUT> sqrt(ap_fixpt<unsigned int W_IN, int IW_IN> num)
~~~

Iterative implementation of sqrt.

Takes in x as a fixed point number, returns sqrt(x).



**Template Parameters:**

* `unsigned int W_OUT`: width of the output<br>
* `int IW_OUT`: width of integer portion of the output<br>
* `int N_ITERATIONS`: number of iterations<br>
* `unsigned int W_IN`: width of the input (automatically inferred)<br>
* `int IW_IN`: width of integer portion of the input (automatically inferred)<br> <br>

**Function Arguments:**

* `ap_fixpt<unsigned int W_IN, int IW_IN> num`: input<br>

**Limitations:**

No limitations.

**Returns:**

- `ap_fixpt<unsigned int W_OUT, int IW_OUT>`: square root of input value 

## Examples

~~~lua
hls::ap_fixpt<10, 2> y = 4;
auto x = hls::math::sqrt<10, 2, 20>(y); //x will be an ap_fixpt w/ the value 2
~~~

The example used to gather the following graph and resource report can be found [here](../../examples/simple/sqrt).

## Error Graph

![sqrt_D32_I16_S0.000000_L10.000000_N16](<../graphs/sqrt_D32_I16_S0.000000_L10.000000_N16_graph.png>)

![sqrt_D33_I32_S0.000000_L4294967295.000000_N30](<../graphs/sqrt_D33_I32_S0.000000_L4294967295.000000_N30_graph.png>)

![sqrt_D32_I16_S1.000000_L100.000000_N16](<../graphs/sqrt_D32_I16_S1.000000_L100.000000_N16_graph.png>)

![sqrt_D32_I8_S0.000000_L1.000000_N16](<../graphs/sqrt_D32_I8_S0.000000_L1.000000_N16_graph.png>)

## Resource Usage

Using MPF300


Input Plot Range: [0.00, 10.00]
Using N_ITER = 16

Using W = 32, IW = 16, Q_M = AP_TRN, O_M = AP_WRAP



| Name       | Latency [cycles] (min/max/avg)   | II [cycles] (min/max/avg)   |   Avg Error |   Max Error |   LUTs |   DFFs |   DSPs |   LSRAM |   uSRAM | Estimated Frequency   |
|------------|----------------------------------|-----------------------------|-------------|-------------|--------|--------|--------|---------|---------|-----------------------|
| sqrt_hls   | 102 / 103 / 102.01               | 1 / 2 / 1.01                |     2.9e-05 |     0.00012 |   4003 |   6623 |      0 |       0 |       8 | 424.628 MHz           |
| sqrt_cmath | 6 / 239 / 206.24                 | 6 / 239 / 206.39            |     0       |     0       |  12691 |  16379 |      6 |       0 |       0 | 288.850 MHz           |


Input Plot Range: [0.00, 4294967295.00]
Using N_ITER = 30

Using W = 33, IW = 32, Q_M = AP_TRN, O_M = AP_WRAP



| Name       | Latency [cycles] (min/max/avg)   | II [cycles] (min/max/avg)   |   Avg Error |   Max Error |   LUTs |   DFFs |   DSPs |   LSRAM |   uSRAM | Estimated Frequency   |
|------------|----------------------------------|-----------------------------|-------------|-------------|--------|--------|--------|---------|---------|-----------------------|
| sqrt_hls   | 185 / 186 / 185.20               | 1 / 2 / 1.25                |    0.052281 |    0.159138 |   8810 |  21135 |      0 |       1 |      28 | 346.380 MHz           |
| sqrt_cmath | 6 / 239 / 206.24                 | 6 / 239 / 206.39            |    0        |    0        |  12691 |  16379 |      6 |       0 |       0 | 296.472 MHz           |


Input Plot Range: [1.00, 100.00]
Using N_ITER = 16

Using W = 32, IW = 16, Q_M = AP_TRN, O_M = AP_WRAP



| Name       | Latency [cycles] (min/max/avg)   | II [cycles] (min/max/avg)   |   Avg Error |   Max Error |   LUTs |   DFFs |   DSPs |   LSRAM |   uSRAM | Estimated Frequency   |
|------------|----------------------------------|-----------------------------|-------------|-------------|--------|--------|--------|---------|---------|-----------------------|
| sqrt_hls   | 102 / 103 / 102.01               | 1 / 2 / 1.01                |     9.9e-05 |    0.000244 |   4003 |   6623 |      0 |       0 |       8 | 426.439 MHz           |
| sqrt_cmath | 6 / 239 / 206.24                 | 6 / 239 / 206.39            |     0       |    0        |  12691 |  16379 |      6 |       0 |       0 | 278.396 MHz           |


Input Plot Range: [0.00, 1.00]
Using N_ITER = 16

Using W = 32, IW = 8, Q_M = AP_TRN, O_M = AP_WRAP



| Name       | Latency [cycles] (min/max/avg)   | II [cycles] (min/max/avg)   |   Avg Error |   Max Error |   LUTs |   DFFs |   DSPs |   LSRAM |   uSRAM | Estimated Frequency   |
|------------|----------------------------------|-----------------------------|-------------|-------------|--------|--------|--------|---------|---------|-----------------------|
| sqrt_hls   | 86 / 87 / 86.01                  | 1 / 2 / 1.01                |       7e-06 |     1.5e-05 |   2866 |   5025 |      0 |       0 |      10 | 470.146 MHz           |
| sqrt_cmath | 6 / 239 / 206.24                 | 6 / 239 / 206.39            |       0     |     0       |  12691 |  16379 |      6 |       0 |       0 | 305.530 MHz           |

Notes:
- Targeted FMax was 400MHz.
- The standard C math library uses floating point numbers.


Back to [top](#).