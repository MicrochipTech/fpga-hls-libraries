## sin_performance

This example is meant to show two things:
1. The performance improvements gained when using the hls::math::sin_lut() function implemented in the FPGA fabric over the cmath sin() function excuted by the CPU.
2. How the same source code example can be compiled to target two different boards:
   
  - [BeagleV-Fire board](https://www.beagleboard.org/boards/beaglev-fire)
  - [Icicle Kit board](https://www.microchip.com/en-us/development-tool/mpfs-icicle-kit-es)

For the BeagleF-Fire, see the `sin_performance.yaml` file under the `build-options` directory in the [BeagleV-Fire gateware repo](https://openbeagle.org/beaglev-fire/gateware) 

This directory contains the source files for the sin_performance design. You can configure the [data transfer method](https://onlinedocs.microchip.com/oxy/GUID-AFCB5DCC-964F-4BE7-AA46-C756FA87ED7B-en-US-11/GUID-212067DF-C1B6-4C22-ADDD-3C306CE990E5.html) by changing the `TRANSFER_TYPE` and `USE_DMA` variables in the [hls/Makefile](hls/Makefile):  

* `TRANSFER_TYPE=AXI_TARGET` and `USE_DMA=true`: use the DMA engine to transfer data in and out of the accelerator’s on-chip memory buffer. 

* `TRANSFER_TYPE=AXI_TARGET` and `USE_DMA=false`: let the RISC-V CPU perform the data transfers in and out of the accelerator’s on-chip memory buffer. 

* `TRANSFER_TYPE=AXI_INITIATOR`: let the accelerator directly access the DDR memory so it doesn't have an extra copy of the data in the chip. 

To change the memory region `hls_malloc` allocates memory to, you can set the `CACHED` variable:

* `CACHED=HLS_ALLOC_CACHED`: Cached DDR. Default if region unspecified. Recommended for best overall transfer times. 
   
* `CACHED=HLS_ALLOC_NONCACHED_WCB`: Non-cached DDR with write-combine buffer. Slightly better performance than Cached DDR for writes, but worse for reads.

* `CACHED=HLS_ALLOC_NONCACHED`: Non-cached DDR. Not recommended (lower performance than other options).

**Note**: The BeagleV-Fire board only supports HLS_ALLOC_NONCACHED option.

After running the example on-board, you should see something like this in the terminal:
```
Passed! Times: cmath: 0.004770 s, hls_math: 0.000616 s
```
The exact times it takes to run will vary.


## BeagleV-Fire 2024.1 Patch
If you are running this example on the BeagleV-Fire board, and you are using SmartHLS 2024.1 or older, you must generate the binary executable for the project using the provided [Makefile](../../beaglev-fire/sin_performance/Makefile). This is because the binary must be linked with the patch provided in [hls/hls_patch](../../beaglev-fire/sin_performance/hls/hls_patch). This issue will be resolved in SmartHLS v2024.2.
