## Some notes on the Soft MiV Master Example:

### Before running...
This example only works on Linux. It is recommended you read and are able to follow the instructions for setting up the PolarFire Video Kit in the SHLS User Guide linked [here](https://onlinedocs.microchip.com/oxy/GUID-AFCB5DCC-964F-4BE7-AA46-C756FA87ED7B-en-US-11/GUID-B68DFB7D-343C-4739-A67E-E40F685CB435.html). 

### Reference Design
We have included the Mi-V soft processor platform source code in [support/miv/platform](../../../../../support/miv/platform). This folder contains the 
drivers necessary for running on the Mi-V soft processor.

We have also included the [fpga_config dir/fpga_hls_config.hpp](fpga_config_dir/fpga_hls_config.hpp) file, which specifies the base addresses we use in our MiV reference design.
If you use are using a different reference design, make sure to change [fpga_config dir/fpga_hls_config.hpp](fpga_config_dir/fpga_hls_config.hpp) accordingly.

### Compilation Flow
To compile, we use the scripts [compile_and_run.sh](compile_and_run.sh) and [compile.sh](compile.sh) included in the example. 

We can't use the usual flow (the one described in the User Guide) because the board we target with this example, the MPF300 ([PolarFire Video Kit](https://onlinedocs.microchip.com/oxy/GUID-AFCB5DCC-964F-4BE7-AA46-C756FA87ED7B-en-US-11/GUID-B68DFB7D-343C-4739-A67E-E40F685CB435.html)), only has 64KB of memory, and runs baremetal (e.g. the processor is implemented in the fabric of the FPGA). 
This means we need to be careful to make sure what we include in our design does not exceed the memory capacities of the board.
When we use the regular compilation flow, it tries to include the ap_fixpt library, which brings in data unnecessary for our example,
thus using up more memory than it needs. The custom compilation flow fixes that, by only including the driver files and HLS libraries that are strictly necessary.

### How to Compile
Note these instructions assume the PolarFire Video Kit is connected to the machine you are compiling on. To compile, simply run `bash compile_and_run.sh`.
This should compile the design, program it to the board, open an OpenOCD instance, and allow you to run gdb as you would normally. 


