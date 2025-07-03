## [`pow`](../../include/hls_pow.hpp)

## Table of Contents:

**Functions**

> [`pow`](#function-pow)

**Examples**

> [Examples](#examples)

**Quality of Results**

> [Error Graph](#error-graph)

> [Resource Usage](#resource-usage)

### Function `pow`
~~~lua
template <unsigned int W_OUT, int IW_OUT, unsigned int W_IN, int IW_IN>
ap_fixpt<W_OUT, IW_OUT> pow(ap_fixpt<W_IN, IW_IN> base, ap_fixpt<W_IN, IW_IN> pow, int error)
~~~

Implementation of pow using ln_taylor and exp.



**Template Parameters:**

* `unsigned int W_OUT`: width of the output<br>
* `int IW_OUT`: width of integer portion of the output<br>
* `unsigned int W_IN`: width of the input (automatically inferred)<br>
* `int IW_IN`: width of integer portion of the input (automatically inferred)<br> <br>

**Function Arguments:**

* `ap_fixpt<W_IN, IW_IN> base`: base number<br>
* `ap_fixpt<W_IN, IW_IN> pow`: power to raise the base to<br>
* `int error`: variable to hold error code if error encountered<br>

**Limitations:**

No limitations.

**Returns:**

- `ap_fixpt<W_OUT, IW_OUT>`: base number raised to the power of pow

## Examples

~~~lua
hls::ap_fixpt<10, 2> y = 2;
hls::ap_fixpt<10, 3> base = 2;
auto x = hls::math::pow<10, 2>(base, y); //x will be an ap_fixpt w/ the value 4
~~~

The example used to gather the following graph and resource report can be found [here](../../examples/simple/pow).

## Error Graph

![pow_D64_I32_S15.000000_L20.000000_B2.000000](<../graphs/pow_D64_I32_S15.000000_L20.000000_B2.000000_graph.png>)

![pow_D32_I8_S-10.000000_L-2.000000_B2.000000](<../graphs/pow_D32_I8_S-10.000000_L-2.000000_B2.000000_graph.png>)

![pow_D32_I16_S-2.000000_L3.000000_B3.000000](<../graphs/pow_D32_I16_S-2.000000_L3.000000_B3.000000_graph.png>)

![pow_D32_I16_S-2.000000_L3.000000_B1.400000](<../graphs/pow_D32_I16_S-2.000000_L3.000000_B1.400000_graph.png>)

![pow_D32_I16_S-3.000000_L3.000000_B-3.000000](<../graphs/pow_D32_I16_S-3.000000_L3.000000_B-3.000000_graph.png>)

## Resource Usage

Using MPF300


Input Plot Range: [15.00, 20.00]
Using base = 2.00

Using W = 64, IW = 32, Q_M = AP_TRN, O_M = AP_WRAP



| Name      | Latency [cycles] (min/max/avg)   | II [cycles] (min/max/avg)   |   Avg Error |   Max Error |   LUTs |   DFFs |   DSPs |   LSRAM |   uSRAM | Estimated Frequency   |
|-----------|----------------------------------|-----------------------------|-------------|-------------|--------|--------|--------|---------|---------|-----------------------|
| pow_hls   | 235 / 236 / 235.17               | 1 / 2 / 1.20                |    0.000237 |    0.000732 |  21169 |  30415 |    134 |      13 |      42 | 311.139 MHz           |
| pow_cmath | 477 / 477 / 477.00               | 43 / 43 / 43.00             |    0        |    0        |  19590 |  27563 |      9 |       8 |       0 | 284.252 MHz           |


Input Plot Range: [-10.00, -2.00]
Using base = 2.00

Using W = 32, IW = 8, Q_M = AP_TRN, O_M = AP_WRAP



| Name      | Latency [cycles] (min/max/avg)   | II [cycles] (min/max/avg)   |   Avg Error |   Max Error |   LUTs |   DFFs |   DSPs |   LSRAM |   uSRAM | Estimated Frequency   |
|-----------|----------------------------------|-----------------------------|-------------|-------------|--------|--------|--------|---------|---------|-----------------------|
| pow_hls   | 149 / 150 / 149.06               | 1 / 2 / 1.06                |           0 |           0 |   6730 |  10455 |     44 |       7 |      13 | 343.289 MHz           |
| pow_cmath | 477 / 477 / 477.00               | 43 / 43 / 43.00             |           0 |           0 |  19590 |  27563 |      9 |       8 |       0 | 278.552 MHz           |


Input Plot Range: [-2.00, 3.00]
Using base = 3.00

Using W = 32, IW = 16, Q_M = AP_TRN, O_M = AP_WRAP



| Name      | Latency [cycles] (min/max/avg)   | II [cycles] (min/max/avg)   |   Avg Error |   Max Error |   LUTs |   DFFs |   DSPs |   LSRAM |   uSRAM | Estimated Frequency   |
|-----------|----------------------------------|-----------------------------|-------------|-------------|--------|--------|--------|---------|---------|-----------------------|
| pow_hls   | 147 / 148 / 147.00               | 1 / 2 / 1.00                |     8.7e-05 |    0.000732 |   5781 |   9301 |     35 |       7 |      10 | 338.066 MHz           |
| pow_cmath | 477 / 477 / 477.00               | 43 / 43 / 43.00             |     0       |    0        |  19590 |  27563 |      9 |       8 |       0 | 263.922 MHz           |


Input Plot Range: [-2.00, 3.00]
Using base = 1.40

Using W = 32, IW = 16, Q_M = AP_TRN, O_M = AP_WRAP



| Name      | Latency [cycles] (min/max/avg)   | II [cycles] (min/max/avg)   |   Avg Error |   Max Error |   LUTs |   DFFs |   DSPs |   LSRAM |   uSRAM | Estimated Frequency   |
|-----------|----------------------------------|-----------------------------|-------------|-------------|--------|--------|--------|---------|---------|-----------------------|
| pow_hls   | 147 / 148 / 147.00               | 1 / 2 / 1.00                |    0.000179 |    0.000714 |   5781 |   9301 |     35 |       7 |      10 | 334.001 MHz           |
| pow_cmath | 477 / 477 / 477.00               | 43 / 43 / 43.00             |    0        |    0        |  19590 |  27563 |      9 |       8 |       0 | 271.739 MHz           |


Input Plot Range: [-3.00, 3.00]
Using base = -3.00

Using W = 32, IW = 16, Q_M = AP_TRN, O_M = AP_WRAP



| Name      | Latency [cycles] (min/max/avg)   | II [cycles] (min/max/avg)   |   Avg Error |   Max Error |   LUTs |   DFFs |   DSPs |   LSRAM |   uSRAM | Estimated Frequency   |
|-----------|----------------------------------|-----------------------------|-------------|-------------|--------|--------|--------|---------|---------|-----------------------|
| pow_hls   | 147 / 148 / 147.00               | 1 / 2 / 1.00                |     3.8e-05 |    0.000244 |   5781 |   9301 |     35 |       7 |      10 | 344.471 MHz           |
| pow_cmath | 477 / 477 / 477.00               | 43 / 43 / 43.00             |     0       |    0        |  19590 |  27563 |      9 |       8 |       0 | 259.202 MHz           |

Notes:
- Targeted FMax was 400MHz.
- The standard C math library uses floating point numbers.


Back to [top](#).