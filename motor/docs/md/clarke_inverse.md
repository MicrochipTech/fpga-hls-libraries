## [`clarke_inverse`](../../include/clarke_inverse.hpp)

## Table of Contents:

**Functions**

> [`clarke_inverse`](#function-clarke_inverse)

**Examples**

> [Examples](#examples)

**Quality of Results**

> [Error Graph](#error-graph)

> [Resource Usage](#resource-usage)

### Function `clarke_inverse`
~~~lua
template <unsigned int W_OUT, int IW_OUT, unsigned int W_IN, int IW_IN>
void clarke_inverse(ap_fixpt<unsigned int W_IN, int IW_IN> alpha, ap_fixpt<unsigned int W_IN, int IW_IN> beta, ap_fixpt<unsigned int W_OUT, int IW_OUT> a, ap_fixpt<unsigned int W_OUT, int IW_OUT> b, ap_fixpt<unsigned int W_OUT, int IW_OUT> c)
~~~


Inverse Clarke transform (αβ → abc) with constant √3/2 factor, fixed-point.

Computes the three-phase components from stationary-frame inputs:
$$a = alpha$$
$$b = \frac{-alpha + \sqrt{3} * beta} {2}$$
$$c = \frac{-alpha - \sqrt{3} * beta}{2}$$
which implies the zero-sum property a + b + c = 0.



**Template Parameters:**

* `unsigned int W_OUT`: width of the output<br>
* `int IW_OUT`: width of integer portion of the output<br>
* `unsigned int W_IN`: width of the input (automatically inferred)<br>
* `int IW_IN`: width of integer portion of the input (automatically inferred)<br> <br>

**Function Arguments:**

* `ap_fixpt<unsigned int W_IN, int IW_IN> alpha`: Clarke α component.<br>
* `ap_fixpt<unsigned int W_IN, int IW_IN> beta`: Clarke β component.<br>
* `ap_fixpt<unsigned int W_OUT, int IW_OUT> a`: Phase-A result.<br>
* `ap_fixpt<unsigned int W_OUT, int IW_OUT> b`: Phase-B result.<br>
* `ap_fixpt<unsigned int W_OUT, int IW_OUT> c`: Phase-C result.<br> <br>

**Limitations:**

No limitations.

**Returns:**

No return.
### Function `clarke_inverse`
~~~lua
template <unsigned int W_OUT, int IW_OUT, unsigned int W_IN, int IW_IN>
void clarke_inverse(ap_int<unsigned int W_IN> alpha, ap_int<unsigned int W_IN> beta, ap_int<unsigned int W_OUT> a, ap_int<unsigned int W_OUT> b, ap_int<unsigned int W_OUT> c)
~~~


Inverse Clarke transform (αβ → abc) with constant √3/2 factor, integer.

Computes the three-phase components from stationary-frame inputs:
$$a = alpha$$
$$b = \frac{-alpha + \sqrt{3} * beta} {2}$$
$$c = \frac{-alpha - \sqrt{3} * beta}{2}$$
which implies the zero-sum property a + b + c = 0.



**Template Parameters:**

* `unsigned int W_OUT`: width of the output<br>
* `int IW_OUT`: width of integer portion of the output<br>
* `unsigned int W_IN`: width of the input (automatically inferred)<br>
* `int IW_IN`: width of integer portion of the input (automatically inferred)<br> <br>

**Function Arguments:**

* `ap_int<unsigned int W_IN> alpha`: Clarke α component.<br>
* `ap_int<unsigned int W_IN> beta`: Clarke β component.<br>
* `ap_int<unsigned int W_OUT> a`: Phase-A result.<br>
* `ap_int<unsigned int W_OUT> b`: Phase-B result.<br>
* `ap_int<unsigned int W_OUT> c`: Phase-C result.<br> <br>

**Limitations:**

No limitations.

**Returns:**

No return.

## Examples

~~~lua
using fixpt_t = hls::ap_fixpt<18,4>;
fixpt_t a, b, c;
fixpt_t alpha = -0.5;
fixpt_t beta = 0.5 * 0.8660254;
hls::motor::clarke_inverse(alpha, beta, a, b, c)
~~~
~~~lua
hls::ap_int<18> a, b, c;
hls::ap_int<18> alpha = 1047;
hls::ap_int<18> beta = 6779;
hls::motor::clarke_inverse(alpha, beta, a, b, c)
~~~

The example used to gather the following graph and resource report can be found [here](../../examples/simple/clarke_inverse).

## Error Graph

![clarke_inverse_D18_I4](<../graphs/clarke_inverse_D18_I4_graph.png>)

## Resource Usage

Using SmartFusion2


Using W = inverse, IW = D18, Q_M = AP_TRN, O_M = AP_WRAP



| Name           | Latency [cycles] (min/max/avg)   | II [cycles] (min/max/avg)   |   Avg Error |   Max Error |   LUTs |   DFFs |   DSPs |   LSRAM |   uSRAM |   MACC count |   RAM1K18 count |   RAM64x18 count | Estimated Frequency   |
|----------------|----------------------------------|-----------------------------|-------------|-------------|--------|--------|--------|---------|---------|--------------|-----------------|------------------|-----------------------|
| clarke_inverse | 11 / 11 / 11.00                  | 12 / 12 / 12.00             |     7.6e-05 |    0.000148 |    245 |    211 |      0 |       0 |       0 |            1 |               0 |                0 | 272.926 MHz           |

Notes:
- Targeted FMax was 400MHz.


Back to [top](#).