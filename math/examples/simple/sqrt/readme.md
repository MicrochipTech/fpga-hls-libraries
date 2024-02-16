# Fixed point square root example

This is an experimental implementation based on the following article. Here is the
 link for more information about the algorithm. 

https://iopscience.iop.org/article/10.1088/1742-6596/1314/1/012008.

**The implementation has been roughly tested, not all possible combinations of the
algortihm's parameters have been tested. So, please make sure to check the results
based on your requirements and configuration.**

The `hls_sqrt` function has template parameters to define the fixed point 
configuration of the output (W, IW) and input (W_IN, IW_IN) arguments, the later
can be automatically inferred from the input argument itself. Only the first three 
arguments are mandatory.

```
template <
    unsigned int W,
    int IW, 
    unsigned N_ITER, 
    unsigned int W_IN, 
    int IW_IN>
hls::ap_ufixpt<W,IW> hls_sqrt(hls::ap_fixpt<W_IN,IW_IN> val);
```

The testbench defines the following configuration, but it can be changed as needed:

```
#define DW      32  // Width of the data word
#define IW      16  // Width of the integer part
#define N_ITER  10  // Number of iterations
```

## Resource usage example for a specific configuration

There are two mutually exclusive pragma options:

1.  Pipeline the entire function to have better performance at the expense
    of a few more resources. This will unroll the loop, so be mindful of N_ITER.
    This will have better Fmax and Initiation Interval II=1
    
2.  Pipeline only the loop, not the entire function. This will reduce resources 
    but II will not be 1. 


```
Data word width     : 32b
Integer Width       : 16b
Num. of iterations  : 10

Fmax requested      : 250MHz

# With function pileine:

LUTs    : 1500
FF      : 2356
LSRAMs  : 0
DSPs    : 0
Latency : 37 cycles
II      : 1

# With loop pipeline:
LUTs    : 1426
FF      : 926
LSRAMs  : 0
DSPs    : 0
Latency : 18 cycles
II      : 18
```

*NOTE*:  Those numbers were obtained using the same fixed point configuration 
        DW & IW for the input argument and the return value, but they could 
        also be different. For example, the integr part for the return value
        could be smaller than the integer part of the input argument by the
        nature of the square root operation.

## Example of a sofware run:

```
$> shls -a sw

DW:32, IW:16, N_ITER:10
Count: 32768    Max diff: 0.250000      Avg diff: 0.089312
Count: 10000    Max diff: 0.001211      Avg diff: 0.000488
Count: 1        Max diff: 0.007812      Avg diff: 0.007812
Count: 1        Max diff: 0.007812      Avg diff: 0.007812
Count: 1        Max diff: 0.015625      Avg diff: 0.015625
```

## Future work:

* Add parameter to remove normalization if not needed, i.e. input argument is 
  less than 1. This should help reduce resources usage.

