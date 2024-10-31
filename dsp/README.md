# SmartHLS DSP Library

<!-- TOC -->

- [SmartHLS DSP Library](#smarthls-dsp-library)
  - [Overview](#overview)
  - [Directory structure](#directory-structure)
  - [Functions](#functions)

<!-- /TOC -->
## Overview

The SmartHLS DSP includes C++ functions that facilitate the rapid development of applications that require common DSP functionality. Applications developed using the SmartHLS DSP library can be seamlessly ported across various Microchip FPGA platforms, significantly reducing time to market.

By open-sourcing the DSP library, we aim to introduce new features and fixes more rapidly and provide the source code as a reference for developers working on other HLS designs. We encourage all developers to contribute by reporting issues, providing feedback, and submitting feature requests through the "Issues" section. Additionally, we welcome contributions of new algorithms and fixes to enhance this open-source repository.

## Directory structure

Below is the directory structure of the DSP library and each sub-directory's respective content.
Please see the readme files in sub-directories for more details.

- [docs](./docs/): Documentation for the DSP HLS C++ Library.
  - [graphs](./docs/graphs/): Graphs that are used in .md files
  - [md](./docs/md/): folder containing .md files for each DSP function.
- [include](include/): The header files of the DSP HLS C++ library.
- [utils](utils/): Python scripts that are used for various purposes.
- [examples](examples/): Examples showcasing the usage of the DSP HLS C++ library
  - [riscv_tests](./examples/riscv_tests/): Directory containing examples using the RISC-V CPUs and FPGA fabric for different boards.
    - [fft_demo](./examples/riscv_tests/fft_demo/): Directory containing the FFT demo for different boards.
      - [icicle_kit](./examples/riscv_tests/fft_demo/icicle_kit/): Project files for the Icicle Kit
      - [discovery_kit](./examples/riscv_tests/fft_demo/discovery_kit/): Project files for the Discovery kit.
      - [beaglev_fire](./examples/riscv_tests/fft_demo/beaglev_fire/): Project files for the BeagleV-Fire board.
  - [simple](./examples/simple/): Directory containing test programs for each DSP function.
    - [fft](./examples/simple/fft/): Simple test program for FFT

## Functions

| Function                             | Description                                      | 
|--------------------------------------|--------------------------------------------------|
| [fft](docs/md/fft.md)            | Radix 2, 256 samples, in-place FFT               |