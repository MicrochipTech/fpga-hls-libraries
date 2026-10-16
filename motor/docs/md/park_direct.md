## [`park_direct`](../../include/park_direct.hpp)

## Table of Contents:

**Functions**

> [`park_direct`](#function-park_direct)

**Examples**

> [Examples](#examples)

**Quality of Results**

> [Error Graph](#error-graph)

> [Resource Usage](#resource-usage)

### Function `park_direct`
~~~lua
template <unsigned int W_OUT, int IW_OUT, int N_ITERATIONS, unsigned int W_IN, int IW_IN>
void park_direct(ap_fixpt<unsigned int W_IN, int IW_IN> I_alpha, ap_fixpt<unsigned int W_IN, int IW_IN> I_beta, ap_fixpt<unsigned int W_IN, int IW_IN> theta, ap_fixpt<unsigned int W_OUT, int IW_OUT> d, ap_fixpt<unsigned int W_OUT, int IW_OUT> q)
~~~

Park transform (αβ → dq) using CORDIC-generated sin/cos, fixed-point.

Converts stationary-frame (alpha, beta) components into rotating-frame
(d, q) components for a given electrical angle theta (revolutions).

$$d = alpha * cos(theta) + beta * sin(theta)$$
$$q = beta  * cos(theta) - alpha * sin(theta)$$

Conceptually, this is a 2-D rotation of the vector [alpha, beta] by angle
+theta, producing the direct (d) and quadrature (q) axes. In exact math, the
rotation preserves energy: d^2 + q^2 ≈ alpha^2 + beta^2 (subject to
quantization and rounding).

Implementation details:
- cos(theta) and sin(theta) are computed via CORDIC with N_ITERATIONS.



**Template Parameters:**

* `unsigned int W_OUT`: width of the output<br>
* `int IW_OUT`: width of integer portion of the output<br>
* `int N_ITERATIONS`: Number of CORDIC iterations for sin/cos. More iterations means higher accuracy but more latency/area. Default set to 16.<br>
* `unsigned int W_IN`: width of the input (automatically inferred)<br>
* `int IW_IN`: width of integer portion of the input (automatically inferred)<br> <br>

**Function Arguments:**

* `ap_fixpt<unsigned int W_IN, int IW_IN> I_alpha`: Stationary-frame α component.<br>
* `ap_fixpt<unsigned int W_IN, int IW_IN> I_beta`: Stationary-frame β component.<br>
* `ap_fixpt<unsigned int W_IN, int IW_IN> theta`: Electrical angle in revolutions.<br>
* `ap_fixpt<unsigned int W_OUT, int IW_OUT> d`: Direct-axis component (rotating frame).<br>
* `ap_fixpt<unsigned int W_OUT, int IW_OUT> q`: Quadrature-axis component (rotating frame).<br> <br>

**Limitations:**

No limitations.

**Returns:**

No return.
### Function `park_direct`
~~~lua
template <unsigned int W_OUT, int IW_OUT, int N_ITERATIONS, unsigned int W_IN, int IW_IN>
void park_direct(ap_int<unsigned int W_IN> I_alpha, ap_int<unsigned int W_IN> I_beta, ap_uint<unsigned int W_IN> theta, ap_int<unsigned int W_OUT> d, ap_int<unsigned int W_OUT> q)
~~~

Park transform (αβ → dq) using CORDIC-generated sin/cos, integer.

Converts stationary-frame (alpha, beta) components into rotating-frame
(d, q) components for a given electrical angle theta (revolutions).

$$d = alpha * cos(theta) + beta * sin(theta)$$
$$q = beta  * cos(theta) - alpha * sin(theta)$$

Conceptually, this is a 2-D rotation of the vector [alpha, beta] by angle
+theta, producing the direct (d) and quadrature (q) axes. In exact math, the
rotation preserves energy: d^2 + q^2 ≈ alpha^2 + beta^2 (subject to
quantization and rounding).

Implementation details:
- cos(theta) and sin(theta) are computed via CORDIC with N_ITERATIONS.



**Template Parameters:**

* `unsigned int W_OUT`: width of the output<br>
* `int IW_OUT`: width of integer portion of the output<br>
* `int N_ITERATIONS`: Number of CORDIC iterations for sin/cos. More iterations means higher accuracy but more latency/area. Default set to 16.<br>
* `unsigned int W_IN`: width of the input (automatically inferred)<br>
* `int IW_IN`: width of integer portion of the input (automatically inferred)<br> <br>

**Function Arguments:**

* `ap_int<unsigned int W_IN> I_alpha`: Stationary-frame α component.<br>
* `ap_int<unsigned int W_IN> I_beta`: Stationary-frame β component.<br>
* `ap_uint<unsigned int W_IN> theta`: Electrical angle in revolutions.<br>
* `ap_int<unsigned int W_OUT> d`: Direct-axis component (rotating frame).<br>
* `ap_int<unsigned int W_OUT> q`: Quadrature-axis component (rotating frame).<br> <br>

**Limitations:**

No limitations.

**Returns:**

No return.

## Examples

~~~lua
using fixpt_t = hls::ap_fixpt<18,4>;
fixpt_t q, d;
hls::ap_fixpt<9, 4> theta = 0.25;
fixpt_t alpha = 1;
fixpt_t beta = 0;
hls::motor::park_direct(alpha, beta, theta, d, q);
~~~
~~~lua
hls::ap_int<18> q, d;
hls::ap_uint<10> theta = 0.25;
hls::ap_int<18> alpha = 1;
hls::ap_int<18> beta = 0;
hls::motor::park_direct(alpha, beta, theta, d, q);
~~~

The example used to gather the following graph and resource report can be found [here](../../examples/simple/park_direct).

## Error Graph

![park_direct_D18_I4](<../graphs/park_direct_D18_I4_graph.png>)

## Resource Usage

Using SmartFusion2


Using W = direct, IW = D18, Q_M = AP_TRN, O_M = AP_WRAP



| Name        | Latency [cycles] (min/max/avg)   | II [cycles] (min/max/avg)   |   Avg Error |   Max Error |   LUTs |   DFFs |   DSPs |   LSRAM |   uSRAM |   MACC count |   RAM1K18 count |   RAM64x18 count | Estimated Frequency   |
|-------------|----------------------------------|-----------------------------|-------------|-------------|--------|--------|--------|---------|---------|--------------|-----------------|------------------|-----------------------|
| park_direct | 65 / 66 / 65.74                  | 66 / 67 / 66.73             |    0.000122 |    0.000549 |   2732 |   3495 |      0 |       0 |       0 |           11 |               0 |                0 | 188.644 MHz           |

Notes:
- Targeted FMax was 400MHz.


Back to [top](#).