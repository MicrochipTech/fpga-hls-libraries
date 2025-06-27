## [`round`](../../include/hls_round.hpp)

## Table of Contents:

**Functions**

> [`round`](#function-round)

**Examples**

> [Examples](#examples)

**Quality of Results**

> [Error Graph](#error-graph)

> [Resource Usage](#resource-usage)

### Function `round`
~~~lua
template <unsigned int W_OUT, int IW_OUT, unsigned int W_IN, int IW_IN>
ap_fixpt<unsigned int W_OUT, int IW_OUT> round(ap_fixpt<unsigned int W_IN, int IW_IN> input)
~~~

Rounds the input value to the nearest integer.



**Template Parameters:**

* `unsigned int W_OUT`: width of the output<br>
* `int IW_OUT`: width of integer portion of the output<br>
* `unsigned int W_IN`: width of the input (automatically inferred)<br>
* `int IW_IN`: width of integer portion of the input (automatically inferred)<br> <br>

**Function Arguments:**

* `ap_fixpt<unsigned int W_IN, int IW_IN> input`: None<br>

**Limitations:**

No limitations.

**Returns:**

- `ap_fixpt<unsigned int W_OUT, int IW_OUT>`: closest integer to the input

## Examples

~~~lua
hls::ap_fixpt<10, 2> x = 12.222;
auto y = hls::math::round<32, 16>(x) // y will be an ap_fixpt number with a value of 12
~~~

The example used to gather the following graph and resource report can be found [here](../../examples/simple/round).

## Error Graph

![round_D32_I16_S-5.000000_L5.000000](<../graphs/round_D32_I16_S-5.000000_L5.000000_graph.png>)

## Resource Usage

Using MPF300


Input Plot Range: [-5.00, 5.00]

Using W = 32, IW = 16, Q_M = AP_TRN, O_M = AP_WRAP



| Name        | Latency [cycles] (min/max/avg)   | II [cycles] (min/max/avg)   |   Avg Error |   Max Error |   LUTs |   DFFs |   DSPs |   LSRAM |   uSRAM | Estimated Frequency   |
|-------------|----------------------------------|-----------------------------|-------------|-------------|--------|--------|--------|---------|---------|-----------------------|
| round_hls   | 2 / 3 / 2.02                     | 1 / 2 / 1.02                |           0 |           0 |     72 |     35 |      0 |       0 |       0 | 991.080 MHz           |
| round_cmath | 15 / 16 / 15.02                  | 1 / 2 / 1.02                |           0 |           0 |   2675 |   4656 |      0 |       0 |       0 | 377.501 MHz           |

Notes:
- Targeted FMax was 400MHz.
- The standard C math library uses floating point numbers.


Back to [top](#).