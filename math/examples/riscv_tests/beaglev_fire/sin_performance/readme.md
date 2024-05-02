## sin_performance

This example is meant to show the performance improvements using the sin_lut function over the cmath sin function on the 
[Icicle Kit](https://onlinedocs.microchip.com/oxy/GUID-AFCB5DCC-964F-4BE7-AA46-C756FA87ED7B-en-US-11/GUID-1F9BA312-87A9-43F0-A66E-B83D805E3F02.html). The design should be compiled using the 
[SoC Flow](https://onlinedocs.microchip.com/oxy/GUID-AFCB5DCC-964F-4BE7-AA46-C756FA87ED7B-en-US-11/GUID-7324A022-0DE8-45E9-9FF0-E06D6CC7AD40.html). 

Included in this example are three variants of the design, each using a different [data transfer method](https://onlinedocs.microchip.com/oxy/GUID-AFCB5DCC-964F-4BE7-AA46-C756FA87ED7B-en-US-11/GUID-212067DF-C1B6-4C22-ADDD-3C306CE990E5.html). [test_dma.cpp](test_dma.cpp) uses the DMA to transfer data in and out of the accelerator’s on-chip memory buffer. [test_no_dma.cpp](test_no_dma.cpp) 
lets the RISC-V CPU perform the data transfers in and out of the accelerator’s on-chip memory buffer. [test_initiator.cpp](test_initiator.cpp) has the 
accelerator directly access the DDR memory so it doesn't have to copy the data onto the chip. Which design to compile can be chosen by changing the `SRCS` variable 
in the [Makefile](Makefile) to the desired design.
