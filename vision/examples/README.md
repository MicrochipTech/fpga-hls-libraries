This directory contains examples that demonstrate how the C++ Vision library can be used.
Each sub-directory is a SmartHLS project folder.

Each example project has the following files:
- Makefile
- C++ source files
- Image files
- config.tcl 

Typically, you will run `shls sw` to verify the software passes testbench.
Then, you will run `shls hw` to generate the Verilog RTL for the top-level function.
Then, run `shls cosim` to verify the generated RTL using SmartHLS [Software/Hardware CoSimulation](https://onlinedocs.microchip.com/oxy/GUID-AFCB5DCC-964F-4BE7-AA46-C756FA87ED7B-en-US-11/GUID-1E5B6475-7959-41AD-A3B0-0F4629416576.html).
