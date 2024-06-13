## sin_performance

This example is meant to show the performance improvements gained when using the sin_lut function over the cmath sin function on the [BeagleV-Fire board](https://www.beagleboard.org/boards/beaglev-fire). To run, visit the [BeagleV-Fire gateware repo](https://openbeagle.org/beaglev-fire/gateware) and see the `sin_performance.yaml` file under the `build-options` directory. 

The source files for this design are located in [../../sources/sin_performance](../../sources/sin_performance). You can configure the [data transfer method](https://onlinedocs.microchip.com/v2/keyword-lookup?keyword=hls_data_transfer&redirect=true&version=latest) by changing the `TRANSFER_TYPE` and `USE_DMA` variables in the [hls/Makefile](hls/Makefile):  

* `TRANSFER_TYPE=AXI_TARGET` and `USE_DMA=true`: use the DMA to transfer data in and out of the accelerator’s on-chip memory buffer. 

* `TRANSFER_TYPE=AXI_TARGET` and `USE_DMA=false`: let the RISC-V CPU perform the data transfers in and out of the accelerator’s on-chip memory buffer. 

* `TRANSFER_TYPE=AXI_INITIATOR`: let the accelerator directly access the DDR memory so it doesn't have to copy the data onto the chip. 

Note that on the BeagleV-Fire board, `hls_malloc` must allocate memory in the non-cached DDR region.

## 2024.1 Patch
When using versions of SmartHLS 2024.1 or older, you must generate the binary executable for the project using the provided [Makefile](Makefile). This is because the binary must be linked with the patch provided in [hls/hls_patch](hls/hls_patch). This issue will be resolved in SmartHLS v2024.2.
