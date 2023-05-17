This directory contains examples that demonstrate how the C++ Vision library can be used.
Each sub-directory is a SmartHLS project folder.

Each example project has the following files:
- Makefile
- C++ source files
- Image files
- config.tcl (add link to UG)

Typically, you will run `shls sw` to verify the software passes testbench.
Then, you will run `shls hw` to generate the Verilog RTL for the top-level function.
Then, run `shls cosim` to verify the generated RTL using SmartHLS [Software/Hardware CoSimulation](https://microchiptech.github.io/fpga-hls-docs/userguide.html#sw-hw-co-simulation).
