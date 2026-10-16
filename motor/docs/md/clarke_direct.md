## [`clarke_direct`](../../include/clarke_direct.hpp)

## Table of Contents:

**Functions**

> [`clarke_direct`](#function-clarke_direct)

**Examples**

> [Examples](#examples)

**Quality of Results**

> [Error Graph](#error-graph)

> [Resource Usage](#resource-usage)

### Function `clarke_direct`
~~~lua
template <unsigned int W_OUT, int IW_OUT, unsigned int W_IN, int IW_IN>
void clarke_direct(ap_fixpt<unsigned int W_IN, int IW_IN> I_a, ap_fixpt<unsigned int W_IN, int IW_IN> I_b, ap_fixpt<unsigned int W_OUT, int IW_OUT> I_alpha, ap_fixpt<unsigned int W_OUT, int IW_OUT> I_beta)
~~~

Clarke (direct) transform (3φ → αβ), amplitude-invariant, fixed-point.

Converts phase currents to the stationary (α, β) frame assuming a balanced
three-phase system where Ic = −(Ia + Ib). With that assumption, the
amplitude-invariant Clarke reduces to:

$$alpha = I_{a}$$
$$beta  = \frac{(I_{a} + 2*I_{b})}{\sqrt{3}}$$



**Template Parameters:**

* `unsigned int W_OUT`: width of the output<br>
* `int IW_OUT`: width of integer portion of the output<br>
* `unsigned int W_IN`: width of the input (automatically inferred)<br>
* `int IW_IN`: width of integer portion of the input (automatically inferred)<br> <br>

**Function Arguments:**

* `ap_fixpt<unsigned int W_IN, int IW_IN> I_a`: Phase-A current.<br>
* `ap_fixpt<unsigned int W_IN, int IW_IN> I_b`: Phase-B current.<br>
* `ap_fixpt<unsigned int W_OUT, int IW_OUT> I_alpha`: Stationary-frame α component.<br>
* `ap_fixpt<unsigned int W_OUT, int IW_OUT> I_beta`: Stationary-frame β component.<br> <br>

**Limitations:**

No limitations.

**Returns:**

No return.
### Function `clarke_direct`
~~~lua
template <unsigned int W_OUT, int IW_OUT, unsigned int W_IN, int IW_IN>
void clarke_direct(ap_int<unsigned int W_IN> I_a, None I_b, None I_alpha, None I_beta)
~~~

Clarke (direct) transform (3φ → αβ), amplitude-invariant, ap_int type.

Converts phase currents to the stationary (α, β) frame assuming a balanced
three-phase system where Ic = −(Ia + Ib). With that assumption, the
amplitude-invariant Clarke reduces to:

$$alpha = I_{a}$$
$$beta  = \frac{(I_{a} + 2*I_{b})}{\sqrt{3}}$$



**Template Parameters:**

* `unsigned int W_OUT`: width of the output<br>
* `int IW_OUT`: width of integer portion of the output<br>
* `unsigned int W_IN`: width of the input (automatically inferred)<br>
* `int IW_IN`: width of integer portion of the input (automatically inferred)<br> <br>

**Function Arguments:**

* `ap_int<unsigned int W_IN> I_a`: Phase-A current.<br>
* `None I_b`: {ap_int<unsigned int W_IN>} I_b Phase-B current.<br>
* `None I_alpha`: {ap_int<unsigned int W_OUT>} Stationary-frame α component.<br>
* `None I_beta`: {ap_int<unsigned int W_OUT>} Stationary-frame β component.<br> <br>

**Limitations:**

No limitations.

**Returns:**

No return.

## Examples

~~~lua
using fixpt_t = hls::ap_fixpt<18,4>;
fixpt_t alpha, beta;
fixpt_t a = -0.5;
fixpt_t b = 0.5 * 0.8660254;
hls::motor::clarke_direct(alpha, beta, a, b)
~~~
~~~lua
hls::ap_int<18> alpha, beta;
hls::ap_int<18> a = 14555;
hls::ap_int<18> b = 45;
hls::motor::clarke_direct(alpha, beta, a, b)
~~~

The example used to gather the following graph and resource report can be found [here](../../examples/simple/clarke_direct).

## Error Graph

![clarke_direct_D18_I4](<../graphs/clarke_direct_D18_I4_graph.png>)

## Resource Usage

Using SmartFusion2


Using W = direct, IW = D18, Q_M = AP_TRN, O_M = AP_WRAP



| Name          | Latency [cycles] (min/max/avg)   | II [cycles] (min/max/avg)   |   Avg Error |   Max Error |   LUTs |   DFFs |   DSPs |   LSRAM |   uSRAM |   MACC count |   RAM1K18 count |   RAM64x18 count | Estimated Frequency   |
|---------------|----------------------------------|-----------------------------|-------------|-------------|--------|--------|--------|---------|---------|--------------|-----------------|------------------|-----------------------|
| clarke_direct | 10 / 10 / 10.00                  | 11 / 11 / 11.00             |     5.7e-05 |    0.000143 |    243 |    198 |      0 |       0 |       0 |            2 |               0 |                0 | 264.971 MHz           |

Notes:
- Targeted FMax was 400MHz.


Back to [top](#).