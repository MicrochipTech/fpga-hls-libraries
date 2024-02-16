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

- `unsigned int W_OUT`: width of the output
- `int IW_OUT`: width of integer portion of the output
- `unsigned int W_IN`: width of the input (automatically inferred)
- `int IW_IN`: width of integer portion of the input (automatically inferred)

**Function Arguments:**

- `ap_fixpt<unsigned int W_IN, int IW_IN> input`: None

**Returns:**

- `ap_fixpt<unsigned int W_OUT, int IW_OUT>`: closest integer to the input
## Examples

~~~lua
  hls::ap_fixpt<10, 2> x = 12.222;

  auto y = hls::math::round<32, 16>(x) // y will be an ap_fixpt number with a value of 12

~~~

The example used to gather the following graph and resource report can be found [here](../../examples/simple/round).

## Error Graph

![round_D32_I16_S-5.000000_L5.000000](../graphs/round_D32_I16_S-5.000000_L5.000000_graph.png)

## Resource Usage

Using MPF300

Input Plot Range: [-5.00, 5.00]
Using W = 32, IW = 16, Q_M = AP_TRN, O_M = AP_WRAP

| Name        | Latency [cycles] (min/max/avg)   | II [cycles] (min/max/avg)   |   Avg Error |   Max Error |   LUTs |   DFFs |   DSPs |   LSRAM |   uSRAM | Estimated Frequency   |
|-------------|----------------------------------|-----------------------------|-------------|-------------|--------|--------|--------|---------|---------|-----------------------|
| round_cmath | 14 / 15 / 14.02                  | 1 / 2 / 1.02                |           0 |           0 |   2620 |   5193 |      0 |       0 |       0 | 881.834 MHz           |
| round_hls   | 4 / 5 / 4.02                     | 1 / 2 / 1.02                |           0 |           0 |     74 |     71 |      0 |       0 |       0 | 881.834 MHz           |

Notes:
- The standard C math library uses floating point numbers.
- FMax is displayed as reported after RTL synthesis and may change during place and route.
- Targeted FMax was 400MHz.


Back to [top](#).