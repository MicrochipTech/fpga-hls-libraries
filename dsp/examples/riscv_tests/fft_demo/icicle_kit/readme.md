# FFT with RISC-V

The goal of this example is to demonstrate the use of the HLS FFT core on the RISC-V SoC using the SmartHLS SoC Flow. 
The design is compiled using the SoC flow and run on the [Icicle Kit](https://www.microchip.com/en-us/development-tool/mpfs-icicle-kit-es) board.

## Prerequisites
- If you haven't already, follow the [Icicle Kit Setup Instructions](https://onlinedocs.microchip.com/v2/keyword-lookup?keyword=hls_iciclekit&redirect=true&version=latest) to set up the Icicle Kit.
- We assume you have understanding of how FFT works, as this example will focus on using HLS FFT core with the SoC Flow.
- Having completed [Training 4](../Training4) as a primer on how the SoC Flow works will be useful for this example.

## Description

In this example, the HLS FFT library is used in a dataflow application which is compiled using the SoC flow.
This SoC flow automatically integrates the FFT module into the RISC-V subsystem within the MPFS250T FPGA.

Here, the `main()` function is now cross-compiled and executed by the RISC-V processor operating on Linux on the MPFS250T FPGA in the Icicle Kit board.
The RISC-V processor loads test vector files into the DDR memory. 
This allows the SmartHLS module to access, process, and subsequently write the output back directly into the DDR.

To be able to compile the demo using the SoC flow instead of the IP flow, we provide users options to configure the FFT module with the following option to the top-level argument type 
for the transfer between the CPU and fabric:
* [axi_initiator](https://onlinedocs.microchip.com/v2/keyword-lookup?keyword=hls_axi4_initiator&redirect=true&version=latest)
* [axi_target](https://onlinedocs.microchip.com/v2/keyword-lookup?keyword=hls_axi4_target&redirect=true&version=latest)

Note: one advantage of using `axi_initiator` argument type is that it eliminates the need for on-chip buffers, which would be needed if the argument type was set to `axi_target`.  For large data inputs, there may not be enough LSRAM on-chip.


The test vector used in the demo represents signal y = 1000 * sin(2πx), and the input buffer `buf_in` and the output buffer `but_out` are of 256 elements in the primitive `uint32_t` type. This allows direct data transfer between the processor and the fabric without data flow 
mechanism such as fifo pipelining. In addition, `buf_in` and `buf_out` can be configured to be of `axi_target` or `axi_initiator` based on if the macro `AXI_TARGET` is defined or not in Makefile. 

```C
template <unsigned SIZE, fft_mode MODE> 
void fft_wrapper(uint32_t buf_in[SIZE], uint32_t buf_out[SIZE]) {

	#pragma HLS function top dataflow
	#pragma HLS interface default type(axi_target)
  #pragma HLS memory impl argument(buf_in) pack(byte)
  #pragma HLS memory impl argument(buf_out) pack(byte)
  #if defined(AXI_TARGET)
  #pragma HLS interface argument(buf_in) type(axi_target) num_elements(SIZE) dma(true)
  #pragma HLS interface argument(buf_out) type(axi_target) num_elements(SIZE) dma(true)
  #else
	#pragma HLS interface argument(buf_in) type(axi_initiator) num_elements(SIZE) max_burst_len(SIZE)
  #pragma HLS interface argument(buf_out) type(axi_initiator) num_elements(SIZE) max_burst_len(SIZE)
  #endif

  FIFO<fft_data_t> fifo_in(SIZE);
  FIFO<fft_data_t> fifo_out(SIZE);

  fft_data_t *buf_in_ptr = (fft_data_t *)buf_in;
  fft_data_t *buf_out_ptr = (fft_data_t *)buf_out;
  write_fifo(buf_in_ptr, fifo_in, SIZE);

  fft<FFT_SIZE>(fifo_in, fifo_out);

  read_fifo(fifo_out, buf_out_ptr, SIZE);
}
```

This is an image of what the design looks like:

```
                          +--------------+
                          |    RISC-V    |
                          +--------------+
                                  |
                                  |
                                  V
                            +------------+
                            |            |
                            |------------|
                            |  BufOut    |<-----------------+
                            |------------|                  |
                            |            |                  |
                            |------------|                  |
            +---------------|  BufIn     |                  |
            |               |------------|                  |
            |               |            |                  |
            |               +------------+                  |
            |                 DDR Memory                    |
            |                                               |
            V                                               |
    +--------------+         +--------------+         +-------------+
    | write_fifo() |--FIFO-->|    fft()     |--FIFO-->| read_fifo() |
    +--------------+         +--------------+         +-------------+

```



**NOTE:** For the sake of simplicity, all error checks, such as verifying the successful allocation of CPU memory, and comparing against a reference software,
have been omitted from this example. However, it is advisable to include error checking code.

## References

For a more comprehensive look at the AXI Initator interface, see [the SmartHLS User Guide](https://onlinedocs.microchip.com/v2/keyword-lookup?keyword=hls_axi4_initiator&redirect=true&version=latest).

