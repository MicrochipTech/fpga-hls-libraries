## [`floor`](../../include/hls_floor.hpp)

## Table of Contents:

**Functions**

> [`floor`](#function-floor)

**Examples**

> [Examples](#examples)

**Quality of Results**

> [Error Graph](#error-graph)

> [Resource Usage](#resource-usage)

### Function `floor`
~~~lua
template <unsigned int W_OUT, int IW_OUT, unsigned int W_IN, int IW_IN>
ap_fixpt<unsigned int W_OUT, int IW_OUT> floor(ap_fixpt<unsigned int W_IN, int IW_IN> input)
~~~

Rounds the input downwards.



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

- `ap_fixpt<unsigned int W_OUT, int IW_OUT>`: largest integer value that is not greater than the input 

## Examples

~~~lua
hls::ap_fixpt<10, 2> x = 12.222;
auto y = hls::math::floor<32, 16>(x) // y will be an ap_fixpt number with a value of 12
~~~

The example used to gather the following graph and resource report can be found [here](../../examples/simple/floor).

## Error Graph

![floor_D32_I16_S-10.000000_L10.000000](<../graphs/floor_D32_I16_S-10.000000_L10.000000_graph.png>)

## Resource Usage

Using MPF300


Input Plot Range: [-10.00, 10.00]

Using W = 32, IW = 16, Q_M = AP_TRN, O_M = AP_WRAP



| Name        | Latency [cycles] (min/max/avg)   | II [cycles] (min/max/avg)   |   Avg Error |   Max Error |   LUTs |   DFFs |   DSPs |   LSRAM |   uSRAM | Estimated Frequency   |
|-------------|----------------------------------|-----------------------------|-------------|-------------|--------|--------|--------|---------|---------|-----------------------|
| floor_hls   | 2 / 3 / 2.01                     | 1 / 2 / 1.01                |           0 |           0 |     54 |     34 |      0 |       0 |       0 | 1594.896 MHz          |
| floor_cmath | 19 / 19 / 19.00                  | 2 / 2 / 2.00                |           0 |           0 |   3047 |   6069 |      0 |       0 |       0 | 398.406 MHz           |

Notes:
- Targeted FMax was 400MHz.
- The standard C math library uses floating point numbers.


Back to [top](#).