## [`park_inverse`](../../include/park_inverse.hpp)

## Table of Contents:

**Functions**

> [`park_inverse`](#function-park_inverse)

**Examples**

> [Examples](#examples)

**Quality of Results**

> [Error Graph](#error-graph)

> [Resource Usage](#resource-usage)

### Function `park_inverse`
~~~lua
template <unsigned int W_OUT, int IW_OUT, int N_ITERATIONS, unsigned int W_IN, int IW_IN>
void park_inverse(ap_fixpt<unsigned int W_IN, int IW_IN> d, ap_fixpt<unsigned int W_IN, int IW_IN> q, ap_fixpt<unsigned int W_IN, int IW_IN> Electrical, ap_fixpt<unsigned int W_OUT, int IW_OUT> alpha, ap_fixpt<unsigned int W_OUT, int IW_OUT> beta)
~~~

Inverse Park transform (dq → αβ) using fixed-point CORDIC sine/cosine.

Computes the standard right-handed inverse Park rotation with electrical angle θ:
$$V_{alpha} = V_{d} * cos(th) + V_{q} * sin(th)$$
$$V_{beta} = V_{q} * cos(th) + V_{d} * sin(th)$$



**Template Parameters:**

* `unsigned int W_OUT`: width of the output<br>
* `int IW_OUT`: width of integer portion of the output<br>
* `int N_ITERATIONS`: Number of CORDIC iterations for sin/cos. More iterations means higher accuracy but more latency/area. Default set to 16.<br>
* `unsigned int W_IN`: width of the input (automatically inferred)<br>
* `int IW_IN`: width of integer portion of the input (automatically inferred)<br> <br>

**Function Arguments:**

* `ap_fixpt<unsigned int W_IN, int IW_IN> d`: Rotor-frame d-axis voltage (flux axis).<br>
* `ap_fixpt<unsigned int W_IN, int IW_IN> q`: Rotor-frame q-axis voltage (torque axis).<br>
* `ap_fixpt<unsigned int W_IN, int IW_IN> Electrical`: angle θ in revolutions for this control tick.<br>
* `ap_fixpt<unsigned int W_OUT, int IW_OUT> alpha`: Stationary-frame α voltage component.<br>
* `ap_fixpt<unsigned int W_OUT, int IW_OUT> beta`: Stationary-frame α voltage component.<br> <br>

**Limitations:**

No limitations.

**Returns:**

No return.
### Function `park_inverse`
~~~lua
template <unsigned int W_OUT, int N_ITERATIONS, unsigned int W_IN>
void park_inverse(ap_int<unsigned int W_IN> d, ap_int<unsigned int W_IN> q, ap_int<unsigned int W_IN> Electrical, ap_int<unsigned int W_OUT> alpha, ap_int<unsigned int W_OUT> beta)
~~~

Inverse Park transform (dq → αβ) using fixed-point CORDIC sine/cosine.

Computes the standard right-handed inverse Park rotation with electrical angle θ:
$$V_{alpha} = V_{d} * cos(th) + V_{q} * sin(th)$$
$$V_{beta} = V_{q} * cos(th) + V_{d} * sin(th)$$



**Template Parameters:**

* `unsigned int W_OUT`: width of the output<br>
* `int N_ITERATIONS`: Number of CORDIC iterations for sin/cos. More iterations means higher accuracy but more latency/area. Default set to 16.<br>
* `unsigned int W_IN`: width of the input (automatically inferred)<br> <br>

**Function Arguments:**

* `ap_int<unsigned int W_IN> d`: Rotor-frame d-axis voltage (flux axis).<br>
* `ap_int<unsigned int W_IN> q`: Rotor-frame q-axis voltage (torque axis).<br>
* `ap_int<unsigned int W_IN> Electrical`: angle θ in revolutions for this control tick.<br>
* `ap_int<unsigned int W_OUT> alpha`: Stationary-frame α voltage component.<br>
* `ap_int<unsigned int W_OUT> beta`: Stationary-frame α voltage component.<br> <br>

**Limitations:**

No limitations.

**Returns:**

No return.

## Examples

~~~lua
using fixpt_t = hls::ap_fixpt<18,4>;
fixpt_t alpha, beta;
hls::ap_fixpt<9, 4> theta = 0.25;
fixpt_t d = 1;
fixpt_t q = 0;
hls::motor::park_inverse(d, q, theta, alpha, beta);
~~~
~~~lua
hls::ap_int<18> alpha, beta;
hls::ap_int<18> d = 1;
hls::ap_uint<10> theta = 125;
hls::ap_int<18> q = 0;
hls::motor::park_inverse(d, q, theta, alpha, beta);
~~~

The example used to gather the following graph and resource report can be found [here](../../examples/simple/park_inverse).

## Error Graph

![park_inverse_D18_I4](<../graphs/park_inverse_D18_I4_graph.png>)

## Resource Usage

Using SmartFusion2


Using W = inverse, IW = D18, Q_M = AP_TRN, O_M = AP_WRAP



| Name         | Latency [cycles] (min/max/avg)   | II [cycles] (min/max/avg)   |   Avg Error |   Max Error |   LUTs |   DFFs |   DSPs |   LSRAM |   uSRAM |   MACC count |   RAM1K18 count |   RAM64x18 count | Estimated Frequency   |
|--------------|----------------------------------|-----------------------------|-------------|-------------|--------|--------|--------|---------|---------|--------------|-----------------|------------------|-----------------------|
| park_inverse | 72 / 72 / 72.00                  | 73 / 73 / 73.00             |    0.000145 |    0.000574 |   3568 |   8189 |      0 |       0 |       0 |           11 |               0 |                2 | 198.452 MHz           |

Notes:
- Targeted FMax was 400MHz.


Back to [top](#).