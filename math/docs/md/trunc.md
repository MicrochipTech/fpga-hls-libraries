## [`trunc`](../../include/hls_trunc.hpp)

## Table of Contents:

**Functions**

> [`trunc`](#function-trunc)

**Examples**

> [Examples](#examples)

**Quality of Results**

> [Error Graph](#error-graph)

> [Resource Usage](#resource-usage)


### Function `trunc`
~~~lua
template <unsigned int W_OUT, int IW_OUT, unsigned int W_IN, int IW_IN>
ap_fixpt<unsigned int W_OUT, int IW_OUT> trunc(ap_fixpt<unsigned int W_IN, int IW_IN> input)
~~~

Rounds the input towards 0.

**Template Parameters:**

- `unsigned int W_OUT`: width of the output
- `int IW_OUT`: width of integer portion of the output
- `unsigned int W_IN`: width of the input (automatically inferred)
- `int IW_IN`: width of integer portion of the input (automatically inferred)

**Function Arguments:**

- `ap_fixpt<unsigned int W_IN, int IW_IN> input`: None

**Returns:**

- `ap_fixpt<unsigned int W_OUT, int IW_OUT>`: the nearest integer that is not larger in magnitute to the input
## Examples

~~~lua
  hls::ap_fixpt<10, 2> x = 12.222;

  auto y = hls::math::trunc<32, 16>(x) // y will be an ap_fixpt number with a value of 12.

~~~

The example used to gather the following graph and resource report can be found [here](../../examples/simple/trunc).

## Error Graph

![trunc_D32_I16_S-5.000000_L5.000000](../graphs/trunc_D32_I16_S-5.000000_L5.000000_graph.png)

## Resource Usage

Using MPF300

Input Plot Range: [-5.00, 5.00]
Using W = 32, IW = 16, Q_M = AP_TRN, O_M = AP_WRAP

| Name        | Latency [cycles] (min/max/avg)   | II [cycles] (min/max/avg)   |   Avg Error |   Max Error |   LUTs |   DFFs |   DSPs |   LSRAM |   uSRAM | Estimated Frequency   |
|-------------|----------------------------------|-----------------------------|-------------|-------------|--------|--------|--------|---------|---------|-----------------------|
| trunc_cmath | 14 / 15 / 14.06                  | 1 / 2 / 1.06                |           0 |           0 |   2337 |   5017 |      0 |       0 |       0 | 377.644 MHz           |
| trunc_hls   | 3 / 4 / 3.06                     | 1 / 2 / 1.06                |           0 |           0 |     99 |     69 |      0 |       0 |       0 | 760.456 MHz           |

Notes:
- The standard C math library uses floating point numbers.
- Targeted FMax was 400MHz.


Back to [top](#).