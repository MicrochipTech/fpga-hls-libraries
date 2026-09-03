────────────────────────────────────────────────────────────
 # Query Log - 2026-06-25 16:13:23 

## Query
what do shls hw and shls sw commands do

## shls_mcp response

````verbatim
# What do `shls hw` and `shls sw` Commands Do

Based on the provided sources, here is a comprehensive explanation of these two SmartHLS command-line commands:

## `shls sw` - Software Compilation and Execution

The `shls sw` command **compiles and runs the program in software on your host machine**.

**Detailed functionality:**
- Compiles the C++ source code using GCC compiler
- Generates an executable file (typically located at `hls_output/.hls/<NAME>.sw_binary`)
- Runs the compiled executable on your development machine
- The compilation step is skipped if no changes are detected in source/header files, Makefile, and config.tcl
- Used for verifying that the design is **functionally correct in software before compiling to hardware**

**Example usage:**
```bash
shls sw
```

**Purpose:** This is typically the first step in the SmartHLS design flow. Developers use this to test and debug their C++ code in software, ensuring the algorithm works correctly before attempting hardware synthesis. As shown in one example, a software test would output results like:

```
DW:32, IW:16, N_ITER:10
Count: 32768    Max diff: 0.250000      Avg diff: 0.089312
Count: 10000    Max diff: 0.001211      Avg diff: 0.000488
PASS
```

## `shls hw` - Hardware Compilation (Default Command)

The `shls hw` command **compiles the C++ software to hardware** by generating Verilog RTL files.

**Detailed functionality:**
- Compiles functions designated with `#pragma HLS function top` into hardware accelerators
- Generates Verilog HDL files that describe the hardware implementation
- Output files are located in `hls_output/rtl/` directory:
  - Main module: `<Project_Name>_<Top_Level_Name>.v`
  - Sub-modules: `<Project_Name>_<Top_Level_Name>_*.v`
- Also generates supporting files:
  - VHDL wrapper files
  - Report files (in `hls_output/reports/`)
  - C++ accelerator driver code (in `hls_output/accelerator_drivers/`)
  - Tcl integration scripts
- This is the **default command** if no target is specified

**Example usage:**
```bash
shls hw
```

## Design Flow Context

These commands are part of a standard HLS design workflow:

1. **`shls sw`** - Compile and run software to verify correctness
2. **`shls hw`** - Compile software to hardware (generate Verilog)
3. **`shls cosim`** - Run software/hardware co-simulation to verify the generated hardware matches the software behavior
4. **`shls fpga`** - Synthesize to FPGA for resource and timing results

## Related Convenience Commands

The sources also mention these related commands:
- **`shls -a sw`** - Software compilation with additional processing (used in some training examples)
- **`shls -s soc_base_proj_run`** - Run SW-only version
- **`shls -s soc_accel_proj_run`** - Run with HW module acceleration

These commands are essential for the typical SmartHLS workflow, allowing developers to iteratively test their C++ implementations in software before committing to hardware synthesis.
````


