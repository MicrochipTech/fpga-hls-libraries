## [`atan`](../../include/hls_atan.hpp)

## Table of Contents:

**Functions**

> [`atan_rational`](#function-atan_rational)

> [`atan_cordic`](#function-atan_cordic)

**Examples**

> [Examples](#examples)

**Quality of Results**

> [Error Graph](#error-graph)

> [Resource Usage](#resource-usage)


### Function `atan_rational`
~~~lua
template <unsigned int W_OUT, int IW_OUT, unsigned int W_IN, int IW_IN>
ap_fixpt<unsigned int W_OUT, int IW_OUT> atan_rational(ap_fixpt<unsigned int W_IN, int IW_IN> num)
~~~

A rational function approximation of arctan.

**Template Parameters:**

- `unsigned int W_OUT`: width of the output
- `int IW_OUT`: width of integer portion of the output
- `unsigned int W_IN`: width of the input (automatically inferred)
- `int IW_IN`: width of integer portion of the input (automatically inferred)

**Function Arguments:**

- `ap_fixpt<unsigned int W_IN, int IW_IN> num`: input

**Returns:**

- `ap_fixpt<unsigned int W_OUT, int IW_OUT>`: arctangent of input value (in radians)
### Function `atan_cordic`
~~~lua
template <unsigned int W_OUT, int IW_OUT, int N_ITERATIONS, unsigned int W_IN, int IW_IN>
ap_fixpt<unsigned int W_OUT, int IW_OUT> atan_cordic(ap_fixpt<unsigned int W_IN, int IW_IN> num)
~~~

CORDIC implementation of arctan.

**Template Parameters:**

- `unsigned int W_OUT`: width of the output
- `int IW_OUT`: width of integer portion of the output
- `int N_ITERATIONS`: number of CORDIC iterations
- `unsigned int W_IN`: width of the input (automatically inferred)
- `int IW_IN`: width of integer portion of the input (automatically inferred)

**Function Arguments:**

- `ap_fixpt<unsigned int W_IN, int IW_IN> num`: input

**Returns:**

- `ap_fixpt<unsigned int W_OUT, int IW_OUT>`: arctangent of input value (in radians)
## Examples

~~~lua
  hls::ap_fixpt<10, 2> y = 1;

  auto x = hls::math::atan_rational<10, 2>(y); // x will be an ap_fixpt number with the value 0.785398163

~~~
~~~lua
  hls::ap_fixpt<10, 2> y = 1;

  auto x = hls::math::atan_cordic<10, 2, 16>(y); // x will be an ap_fixpt number with the value 0.785398163

~~~

The example used to gather the following graph and resource report can be found [here](../../examples/simple/atan).

## Error Graph

![atan_D32_I8_S-2.000000_L2.000000_N16](../graphs/atan_D32_I8_S-2.000000_L2.000000_N16_graph.png)

![atan_D32_I16_S-20.000000_L20.000000_N16](../graphs/atan_D32_I16_S-20.000000_L20.000000_N16_graph.png)

## Resource Usage

Using MPF300

Input Plot Range: [-2.00, 2.00]
Using N_ITER = 16
Using W = 32, IW = 8, Q_M = AP_TRN, O_M = AP_WRAP

| Name          | Latency [cycles] (min/max/avg)   | II [cycles] (min/max/avg)   |   Avg Error |   Max Error |   LUTs |   DFFs |   DSPs |   LSRAM |   uSRAM | Estimated Frequency   |
|---------------|----------------------------------|-----------------------------|-------------|-------------|--------|--------|--------|---------|---------|-----------------------|
| atan_cmath    | 309 / 309 / 309.00               | 18 / 18 / 18.00             |    0        |    0        |  14599 |  23879 |      9 |       0 |       0 | 340.368 MHz           |
| atan_cordic   | 16 / 17 / 16.00                  | 1 / 2 / 1.00                |    0.00012  |    0.000244 |   1107 |   1251 |      0 |       0 |       0 | 340.368 MHz           |
| atan_rational | 79 / 80 / 79.00                  | 1 / 2 / 1.00                |    0.001957 |    0.002828 |   9627 |  13032 |     10 |       0 |       0 | 340.368 MHz           |


Input Plot Range: [-20.00, 20.00]
Using N_ITER = 16
Using W = 32, IW = 16, Q_M = AP_TRN, O_M = AP_WRAP

| Name          | Latency [cycles] (min/max/avg)   | II [cycles] (min/max/avg)   |   Avg Error |   Max Error |   LUTs |   DFFs |   DSPs |   LSRAM |   uSRAM | Estimated Frequency   |
|---------------|----------------------------------|-----------------------------|-------------|-------------|--------|--------|--------|---------|---------|-----------------------|
| atan_cmath    | 309 / 309 / 309.00               | 18 / 18 / 18.00             |    0        |    0        |  14599 |  23879 |      9 |       0 |       0 | 340.368 MHz           |
| atan_cordic   | 14 / 15 / 14.01                  | 1 / 2 / 1.01                |    0.000134 |    0.000348 |   1195 |   1125 |      0 |       0 |       0 | 340.368 MHz           |
| atan_rational | 77 / 78 / 77.01                  | 1 / 2 / 1.01                |    0.002222 |    0.0028   |   9391 |  13025 |     12 |       0 |       0 | 340.368 MHz           |

Notes:
- The standard C math library uses floating point numbers.
- FMax is displayed as reported after RTL synthesis and may change during place and route.
- Targeted FMax was 400MHz.


Back to [top](#).