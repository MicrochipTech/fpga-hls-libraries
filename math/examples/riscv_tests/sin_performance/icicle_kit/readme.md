## sin_performance

This example is meant to show the performance improvements gained when using the sin_lut function over the cmath sin function on the 
[Icicle Kit](https://onlinedocs.microchip.com/oxy/GUID-AFCB5DCC-964F-4BE7-AA46-C756FA87ED7B-en-US-11/GUID-1F9BA312-87A9-43F0-A66E-B83D805E3F02.html). The design should be compiled using the 
[SoC Flow](https://onlinedocs.microchip.com/oxy/GUID-AFCB5DCC-964F-4BE7-AA46-C756FA87ED7B-en-US-11/GUID-7324A022-0DE8-45E9-9FF0-E06D6CC7AD40.html). 


The source files for this design are located in [../sources/sin_performance](../sources/sin_performance). You can configure the [data transfer method](https://onlinedocs.microchip.com/oxy/GUID-AFCB5DCC-964F-4BE7-AA46-C756FA87ED7B-en-US-11/GUID-212067DF-C1B6-4C22-ADDD-3C306CE990E5.html) by changing the `TRANSFER_TYPE` and `USE_DMA` variables in the [Makefile](Makefile):  

* `TRANSFER_TYPE=AXI_TARGET` and `USE_DMA=true`: use the DMA to transfer data in and out of the accelerator’s on-chip memory buffer. 

* `TRANSFER_TYPE=AXI_TARGET` and `USE_DMA=false`: let the RISC-V CPU perform the data transfers in and out of the accelerator’s on-chip memory buffer. 

* `TRANSFER_TYPE=AXI_INITIATOR`: let the accelerator directly access the DDR memory so it doesn't have to copy the data onto the chip. 

The memory region `hls_malloc` will allocate memory to can be controlled by changing the `CACHED` variable in the [Makefile](Makefile):

* `CACHED=HLS_ALLOC_CACHED`: Use cached DDR.

* `CACHED=HLS_ALLOC_NONCACHED`: Use non-cached DDR.

## Prerequisites:
You should install the following software:
1. The latest version of Libero & SmartHLS

You will need the following hardware:
1. A [PolarFire SoC Icicle Kit](https://www.microchip.com/en-us/development-tool/mpfs-icicle-kit-es)

## Board Setup

Before running this SmartHLS example, the board needs to be prepared by performing the setup tasks in the [user guide](https://onlinedocs.microchip.com/oxy/GUID-AFCB5DCC-964F-4BE7-AA46-C756FA87ED7B-en-US-11/GUID-1F9BA312-87A9-43F0-A66E-B83D805E3F02.html).

If you have already set up the Discovery Kit, skip to [Compiling the SmartHLS Module and Libero Project](#compiling-the-smarthls-module-and-libero-project).

## Compiling the SmartHLS Module and Libero Project:

1. First, set the `BOARD_IP` environment variable to the IP of the Icicle Kit. In Linux, you can do this by running:

```
export BOARD_IP=<YOUR BOARD IP HERE>
```

Or in Windows by running:

```
$Env:BOARD_IP="<YOUR BOARD IP HERE>"
```

2. To compile the example from the command line, make sure Libero and SmartHLS are
in the PATH in your terminal. On Linux, you can confirm this by typing the following commands:

```
which shls
```

```
which libero 
```

On Windows, you can confirm this by typing the following commands:
```
Get-Command shls
```

```
Get-Command libero
```

3. Now that the tools' paths are set, we can compile! Run `shls -a soc_accel_proj_program`. This command will generate the bitstream for the example and program it to the board connected to your host PC.

4. Once the previous command finished, run `shls -a soc_accel_proj_run` to compile the RISC-V binary, copy it to BOARD_IP on the network, and then run the binary on the Icicle kit.

For more information on the SmartHLS CLI, see [the user guide](https://onlinedocs.microchip.com/oxy/GUID-AFCB5DCC-964F-4BE7-AA46-C756FA87ED7B-en-US-12/GUID-9355FB9A-5134-49FB-8F37-525A043B736E.html?hl=command%2Cline%2Cinterface#GUID-9355FB9A-5134-49FB-8F37-525A043B736E__GUID-365A16D5-A05F-44BB-A940-4FE8D9EC74A8).


