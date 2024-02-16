# SmartHLS Math Library

<!-- TOC -->

  - [Overview](#overview)
  - [Library Components](#library-components)

<!-- /TOC -->
## Overview

The SmartHLS math library provides HLS C++ library functions for algorithm prototyping of fixed point math applications.

By open sourcing the math library, we hope to deliver new features and fixes under a faster release cycle,
and share the source code to serve as examples for developers to implement other HLS designs.
We welcome all developers to contribute by reporting issues, feedback, and feature requests (by raising "Issues");
and even better, contribute back new algorithms and fixes into this open-source repository.

## Library Components

Below is the directory structure of the math library and each sub-directory's respective content.
Please see the readme files in sub-directories for more details.

- [docs](docs/): Documentation for the HLS C++ Library. See [documentation notes](#documentation-notes).
- [include](include/): The HLS C++ library.
- [examples](examples/): Examples showcasing the usage of the HLS C++ library. See [examples notes](#examples-notes).
- [utils](utils/): Utilities to make using the library easier.
    
## Functions

| Function                             | Description                                      | 
|--------------------------------------|--------------------------------------------------|
| [sin_lut](docs/md/sin.md)            | Lookup Table implementation of sin.              |
| [sin_cordic](docs/md/sin.md)         | CORDIC implementation of sin.                    |
| [sin_taylor](docs/md/sin.md)         | Taylor Series implementation of sin.             |
| [tan_lut](docs/md/tan.md)            | Lookup Table implementation of tan.              |
| [tan_cordic](docs/md/tan.md)         | CORDIC implementation of tan.                    |
| [tan_taylor](docs/md/tan.md)         | Taylor Series implementation of tan.             |
| [cos_lut](docs/md/cos.md)            | Lookup Table implementation of cos.              |
| [cos_cordic](docs/md/cos.md)         | CORDIC implementation of cos.                    |
| [cos_taylor](docs/md/cos.md)         | Taylor Series implementation of cos.             |
| [sqrt](docs/md/sqrt.md)              | Iterative implementation of sqrt.                |
| [atan_cordic](docs/md/atan.md)       | CORDIC implementation of atan.                   |
| [atan_rational](docs/md/atan.md)     | Rational function approximation of atan.         |
| [exp_taylor](docs/md/exp.md)         | Taylor Series implementation of exp.             |
| [exp_cordic](docs/md/exp.md)         | CORDIC implementation of exp.                    |
| [ln](docs/md/ln.md)                  | Lookup Table based implementation of ln.         |
| [log](docs/md/log.md)                | Lookup Table based implementation of log.        |
| [pow](docs/md/pow.md)                | Implementation of pow based on ln and exp.       |
| [ceil](docs/md/ceil.md)              | Rounds the input upwards.                        |
| [floor](docs/md/floor.md)            | Rounds the input downwards.                      |
| [round](docs/md/round.md)            | Rounds the input to the closest integer.         |
| [trunc](docs/md/trunc.md)            | Rounds the input towards zero.                   |
| [abs](docs/md/abs.md)                | Returns the absolute value of the input.         |
| [asin_cordic](docs/md/asin.md)       | CORDIC implementation of asin.                   |
| [acos_cordic](docs/md/acos.md)       | CORDIC implementation of acos.                   |
| [sincos](docs/md/sincos.md)          | CORDIC implementation of sincos.                 |
| [log2_lut](docs/md/log2.md)          | Lookup Table implementation of log2.             |
| [log2_cordic](docs/md/log2.md)       | CORDIC implementation of log2.                   |
| [cordic](docs/md/cordic.md)          | CORDIC algorithm.                                |
    
## gdb-pretty-printers
The SmartHLS Fixed Point Math Library also includes a [gdb pretty-printer](utils/gdb-pretty-printers/) that simplifies the debugging of applications that use ap_[u]fixpt datatypes. The pretty-printer will display the real value, the hexadecimal representation, 
the word width (W) and the width of the integer part (IW) of ap_[u]fixpt types:

```
(gdb) print x
$2 = 0.49999999906867743 [0xfffffff8] <W:34,IW:1> 
```

## Notes

For more information on SHLS ap_fixpt, see the [User Guide](https://onlinedocs.microchip.com/oxy/GUID-AFCB5DCC-964F-4BE7-AA46-C756FA87ED7B-en-US-11/GUID-61CF52C5-A40E-436D-9E38-AD885C0EF16D.html).

Fixed point math functions internal calculations use the default Quantization (Q_M) and Overflow (O_M) modes (i.e. AP_TRN and AP_WRAP respectively.)

### Documentation Notes

Documentation for each function in the HLS Math Library can be found in [docs/md](docs/). The resource usage displayed in the documentation was generated
using a specific set of parameters, which are listed at the top of the resource report table and in the title of the graph. W is the width of the input/output in bits,
IW is how far the most significant bit is above the decimal, W_M is the Quantization (rounding) mode used when a result has precision below the least significant bit,
and O_M is the Overflow mode used when a result exceeds the maximum or minimum representable value.
Your resource usage may change depending on your parameters.

Note that FMax in the resource reports is an estimation made during synthesis. Your FMax may be different depending on your design.

### Examples Notes

#### Simple Examples
The wrappers for the HLS Math Library functions in the [examples](examples/) include "XS", "S", "M", "L", "XL" in their names. These represent fixed point 
configurations used to test different datapoints. These are by no means the only configurations the HLS Math Library can support, but were arbitrarly chosen
to represent a good range of precisions one might use. See [examples/simple/configs.hpp](examples/simple/configs.hpp).

#### RISC-V Examples
The HLS Math Library also includes RISC-V SoC examples for the [Icicle Kit](examples/riscv_tests/icicle_kit) (MPFS250T) running Linux, and the [PolarFire Video Kit](examples/riscv_tests/soft_miv/master_example) (MPF300) for the Soft-MiV processor running baremetal.

